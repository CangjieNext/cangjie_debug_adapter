// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_SERVER_DATA_ID_H
#define DAP_SERVER_DATA_ID_H

namespace Dap {
class DataId {
public:
    static DataId &GetInstance()
    {
        static DataId dataIdInstance;
        return dataIdInstance;
    }
    int GetDataId()
    {
        std::lock_guard<std::mutex> lock(mutex);
        dataId %= ID_MAX;
        return dataId++;
    }

    void AddDataId(const int &id, const std::string &name)
    {
        std::lock_guard<std::mutex> lock(mutex);
        dataIdMap.emplace(id, name);
    }

    std::string GetNameByData(const int &id)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (dataIdMap.find(id) == dataIdMap.end()) {
            return "";
        }
        return dataIdMap[id];
    }

private:
    const static int ID_MAX = std::numeric_limits<int>::max();
    std::mutex mutex;
    int dataId = 0;
    std::map<int, std::string> dataIdMap;
};
};

#endif // DAP_SERVER_DATA_ID_H
