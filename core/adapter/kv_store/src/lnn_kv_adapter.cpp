/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <cinttypes>
#include <mutex>
#include <unistd.h>
#include <vector>

#include "lnn_kv_adapter.h"
#include "anonymizer.h"
#include "lnn_log.h"
#include "softbus_errcode.h"

#include "datetime_ex.h"
namespace OHOS {
using namespace OHOS::DistributedKv;
namespace {
constexpr int32_t MAX_STRING_LEN = 4096;
constexpr int32_t MAX_INIT_RETRY_TIMES = 30;
constexpr int32_t INIT_RETRY_SLEEP_INTERVAL = 500 * 1000; // 500ms
constexpr int32_t MAX_MAP_SIZE = 10000;
const std::string DATABASE_DIR = "/data/service/el1/public/database/dsoftbus";
}

KVAdapter::KVAdapter(const std::string &appId, const std::string &storeId,
                     const std::shared_ptr<DistributedKv::KvStoreObserver> &dataChangeListener)
{
    this->appId_.appId = appId;
    this->storeId_.storeId = storeId;
    this->dataChangeListener_ = dataChangeListener;
    LNN_LOGI(LNN_LEDGER, "KVAdapter Constructor Success, appId: %{public}s, storeId: %{public}s",
        appId.c_str(), storeId.c_str());
}

KVAdapter::~KVAdapter()
{
    LNN_LOGI(LNN_LEDGER, "KVAdapter Destruction!");
}


int32_t KVAdapter::Init()
{
    LNN_LOGI(LNN_LEDGER, "Init kvAdapter, storeId: %s", storeId_.storeId.c_str());
    int32_t tryTimes = MAX_INIT_RETRY_TIMES;
    int64_t beginTime = GetTickCount();
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStorePtr();
        if (kvStorePtr_ && status == DistributedKv::Status::SUCCESS) {
            int64_t endTime = GetTickCount();
            LNN_LOGI(LNN_LEDGER, "Init KvStorePtr Success, spend %{public}" PRId64 " ms", endTime - beginTime);
            RegisterDataChangeListener();
            return SOFTBUS_OK;
        }
        LNN_LOGI(LNN_LEDGER, "CheckKvStore, left times: %{public}d, status: %{public}d", tryTimes, status);
        if (status == DistributedKv::Status::SECURITY_LEVEL_ERROR) {
            DeleteKvStore();
        }
        usleep(INIT_RETRY_SLEEP_INTERVAL);
        tryTimes--;
    }
    return SOFTBUS_KV_DB_INIT_FAIL;
}

int32_t KVAdapter::DeInit()
{
    LNN_LOGI(LNN_LEDGER, "DBAdapter DeInit");
    UnRegisterDataChangeListener();
    DeleteDataChangeListener();
    DeleteKvStorePtr();
    return SOFTBUS_OK;
}

int32_t KVAdapter::RegisterDataChangeListener()
{
    LNN_LOGI(LNN_LEDGER, "Register db data change listener");
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        if (kvStorePtr_ == nullptr) {
            LNN_LOGE(LNN_LEDGER, "kvStoragePtr_ is null");
            return SOFTBUS_KV_DB_PTR_NULL;
        }
        DistributedKv::Status status =
                kvStorePtr_->SubscribeKvStore(DistributedKv::SubscribeType::SUBSCRIBE_TYPE_CLOUD, dataChangeListener_);
        if (status != DistributedKv::Status::SUCCESS) {
            LNN_LOGE(LNN_LEDGER, "Register db data change listener failed, ret: %{public}d", status);
            return SOFTBUS_KV_REGISTER_DATA_LISTENER_FAILED;
        }
    }
    return SOFTBUS_OK;
}

int32_t KVAdapter::UnRegisterDataChangeListener()
{
    LNN_LOGI(LNN_LEDGER, "UnRegister db data change listener");
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        if (kvStorePtr_ == nullptr) {
            LNN_LOGE(LNN_LEDGER, "kvStoragePtr_ is null");
            return SOFTBUS_KV_DB_PTR_NULL;
        }
        DistributedKv::Status status =
            kvStorePtr_->UnSubscribeKvStore(DistributedKv::SubscribeType::SUBSCRIBE_TYPE_CLOUD, dataChangeListener_);
        if (status != DistributedKv::Status::SUCCESS) {
            LNN_LOGE(LNN_LEDGER, "UnRegister db data change listener failed, ret: %{public}d", status);
            return SOFTBUS_KV_UNREGISTER_DATA_LISTENER_FAILED;
        }
    }
    return SOFTBUS_OK;
}

int32_t KVAdapter::DeleteDataChangeListener()
{
    LNN_LOGI(LNN_LEDGER, "Delete DataChangeListener!");
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        dataChangeListener_ = nullptr;
    }
    return SOFTBUS_OK;
}

int32_t KVAdapter::Put(const std::string& key, const std::string& value)
{
    if (key.empty() || key.size() > MAX_STRING_LEN || value.empty() || value.size() > MAX_STRING_LEN) {
        LNN_LOGE(LNN_LEDGER, "Param is invalid!");
        return SOFTBUS_INVALID_PARAM;
    }
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        if (kvStorePtr_ == nullptr) {
            LNN_LOGE(LNN_LEDGER, "kvDBPtr is null!");
            return SOFTBUS_KV_DB_PTR_NULL;
        }
        DistributedKv::Key kvKey(key);
        DistributedKv::Value oldV;
        if (kvStorePtr_->Get(kvKey, oldV) == DistributedKv::Status::SUCCESS && oldV.ToString() == value) {
            char *anonyKey = nullptr;
            char *anonyValue = nullptr;
            Anonymize(key.c_str(), &anonyKey);
            Anonymize(value.c_str(), &anonyValue);
            LNN_LOGI(LNN_LEDGER, "The key-value pair already exists. key=%{public}s, value=%{public}s",
                anonyKey, anonyValue);
            AnonymizeFree(anonyKey);
            AnonymizeFree(anonyValue);
            return SOFTBUS_OK;
        }
        DistributedKv::Value kvValue(value);
        status = kvStorePtr_->Put(kvKey, kvValue);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        LNN_LOGE(LNN_LEDGER, "Put kv to db failed, ret: %{public}d", status);
        return SOFTBUS_KV_PUT_DB_FAIL;
    }
    LNN_LOGI(LNN_LEDGER, "KVAdapter Put succeed");
    return SOFTBUS_OK;
}

int32_t KVAdapter::PutBatch(const std::map<std::string, std::string>& values)
{
    if (values.empty() || values.size() > MAX_MAP_SIZE) {
        LNN_LOGE(LNN_LEDGER, "Param is invalid!");
        return SOFTBUS_INVALID_PARAM;
    }
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        if (kvStorePtr_ == nullptr) {
            LNN_LOGE(LNN_LEDGER, "kvDBPtr is null!");
            return SOFTBUS_KV_DB_PTR_NULL;
        }
        std::vector<DistributedKv::Entry> entries;
        DistributedKv::Value oldV;
        DistributedKv::Key kvKey;
        for (auto item : values) {
            kvKey = item.first;
            if (kvStorePtr_->Get(kvKey, oldV) == DistributedKv::Status::SUCCESS && oldV.ToString() == item.second) {
                char *anonyKey = nullptr;
                char *anonyValue = nullptr;
                Anonymize(item.first.c_str(), &anonyKey);
                Anonymize(item.second.c_str(), &anonyValue);
                LNN_LOGI(LNN_LEDGER, "The key-value pair already exists. key=%{public}s, value=%{public}s", anonyKey,
                    anonyValue);
                AnonymizeFree(anonyKey);
                AnonymizeFree(anonyValue);
                continue;
            }
            Entry entry;
            entry.key = kvKey;
            entry.value = item.second;
            entries.emplace_back(entry);
        }
        if (entries.empty()) {
            LNN_LOGI(LNN_LEDGER, "All key-value pair already exists.");
            return SOFTBUS_OK;
        }
        status = kvStorePtr_->PutBatch(entries);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        LNN_LOGE(LNN_LEDGER, "PutBatch kv to db failed, ret: %d", status);
        return SOFTBUS_KV_PUT_DB_FAIL;
    }
    LNN_LOGI(LNN_LEDGER, "KVAdapter PutBatch succeed");
    return SOFTBUS_OK;
}

int32_t KVAdapter::Delete(const std::string& key)
{
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        if (kvStorePtr_ == nullptr) {
            LNN_LOGE(LNN_LEDGER, "kvDBPtr is null!");
            return SOFTBUS_KV_DB_PTR_NULL;
        }
        DistributedKv::Key kvKey(key);
        status = kvStorePtr_->Delete(kvKey);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        LNN_LOGE(LNN_LEDGER, "Delete kv by key failed!");
        return SOFTBUS_KV_DEL_DB_FAIL;
    }
    LNN_LOGI(LNN_LEDGER, "Delete kv by key success!");
    return SOFTBUS_OK;
}

int32_t KVAdapter::DeleteByPrefix(const std::string& keyPrefix)
{
    LNN_LOGI(LNN_LEDGER, "call");
    if (keyPrefix.empty() || keyPrefix.size() > MAX_STRING_LEN) {
        LNN_LOGE(LNN_LEDGER, "Param is invalid!");
        return SOFTBUS_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> lock(kvAdapterMutex_);
    if (kvStorePtr_ == nullptr) {
        LNN_LOGE(LNN_LEDGER, "kvStoragePtr_ is null");
        return SOFTBUS_KV_DB_PTR_NULL;
    }
    // if prefix is empty, get all entries.
    DistributedKv::Key allEntryKeyPrefix(keyPrefix);
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(allEntryKeyPrefix, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        LNN_LOGE(LNN_LEDGER, "GetEntries failed, ret: %{public}d", status);
        return SOFTBUS_KV_DEL_DB_FAIL;
    }
    std::vector<DistributedKv::Key> keys;
    for (auto item : allEntries) {
        keys.push_back(item.key);
    }
    status = kvStorePtr_->DeleteBatch(keys);
    if (status != DistributedKv::Status::SUCCESS) {
        LNN_LOGE(LNN_LEDGER, "DeleteBatch failed, ret: %{public}d", status);
        return SOFTBUS_KV_DEL_DB_FAIL;
    }
    LNN_LOGI(LNN_LEDGER, "DeleteByPrefix succeed");
    return SOFTBUS_OK;
}

int32_t KVAdapter::Get(const std::string& key, std::string& value)
{
    char *anonyKey = nullptr;
    Anonymize(key.c_str(), &anonyKey);
    LNN_LOGI(LNN_LEDGER, "Get data by key: %{public}s", anonyKey);
    AnonymizeFree(anonyKey);
    DistributedKv::Key kvKey(key);
    DistributedKv::Value kvValue;
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        if (kvStorePtr_ == nullptr) {
            LNN_LOGE(LNN_LEDGER, "kvStoragePtr_ is null");
            return SOFTBUS_KV_DB_PTR_NULL;
        }
        status = kvStorePtr_->Get(kvKey, kvValue);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        anonyKey = nullptr;
        Anonymize(key.c_str(), &anonyKey);
        LNN_LOGE(LNN_LEDGER, "Get data from kv failed, key: %{public}s", anonyKey);
        AnonymizeFree(anonyKey);
        return SOFTBUS_KV_GET_DB_FAIL;
    }
    value = kvValue.ToString();
    LNN_LOGI(LNN_LEDGER, "Get succeed");
    return SOFTBUS_OK;
}

DistributedKv::Status KVAdapter::GetKvStorePtr()
{
    LNN_LOGI(LNN_LEDGER, "called");
    DistributedKv::Options options = {
        .encrypt = true,
        .autoSync = false,
        .securityLevel = DistributedKv::SecurityLevel::S1,
        .area = 1,
        .kvStoreType = KvStoreType::SINGLE_VERSION,
        .baseDir = DATABASE_DIR,
        .isPublic = true
    };
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        status = kvDataMgr_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    }
    return status;
}

int32_t KVAdapter::DeleteKvStore()
{
    LNN_LOGI(LNN_LEDGER, "Delete KvStore!");
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        kvDataMgr_.CloseKvStore(appId_, storeId_);
        kvDataMgr_.DeleteKvStore(appId_, storeId_, DATABASE_DIR);
    }
    return SOFTBUS_OK;
}

int32_t KVAdapter::DeleteKvStorePtr()
{
    LNN_LOGI(LNN_LEDGER, "Delete KvStore Ptr!");
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        kvStorePtr_ = nullptr;
    }
    return SOFTBUS_OK;
}

int32_t KVAdapter::CloudSync()
{
    LNN_LOGI(LNN_LEDGER, "call!");
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        if (kvStorePtr_ == nullptr) {
            LNN_LOGE(LNN_LEDGER, "kvDBPtr is null!");
            return SOFTBUS_KV_DB_PTR_NULL;
        }
        status = kvStorePtr_->CloudSync(nullptr);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        LNN_LOGE(LNN_LEDGER, "cloud sync failed, ret: %{public}d", status);
        return SOFTBUS_KV_CLOUD_SYNC_FAIL;
    }
    LNN_LOGI(LNN_LEDGER, "cloud sync ok, ret: %{public}d", status);
    return SOFTBUS_OK;
}

} // namespace OHOS