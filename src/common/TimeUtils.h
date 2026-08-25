// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DAP_TIME_UTILS_H
#define DAP_TIME_UTILS_H

#include <chrono>
#include <memory>
#include <sstream>
#include <iomanip>
#include <functional>
#include <map>
#include <ctime>

namespace Dap {
using namespace std::chrono;

struct Time {
    std::int32_t year = 0;
    std::int32_t month = 0;
    std::int32_t day = 0;
    std::int32_t hour = 0;
    std::int32_t minute = 0;
    std::int32_t second = 0;
    std::int32_t millisecond = 0;
};

enum TimeStyle : int {
    // YYYY-mm-DD HH:MM:SS.sss
    DATE_TIME_1 = 0,
    // YYYY-mm-DD HH:MM:SS
    DATE_TIME_2,
    // YYYYmmDDHHMMSS
    DATE_TIME_3,
    // mm/DD HH:MM:SS.sss
    DATE_TIME_4,
    // YYYY-mm-DD
    DATE_1,
    // HH:MM:SS
    TIME_1
};

class TimeUtils {
public:
    static TimeUtils &Instance()
    {
        static TimeUtils instance;
        return instance;
    }

    uint64_t NowUnixTimeMs()
    {
        system_clock::time_point now = system_clock::now();
        auto unixTimeMs = duration_cast<milliseconds>(now.time_since_epoch());
        return unixTimeMs.count();
    }

    std::unique_ptr<Time> Now()
    {
        std::unique_ptr<Time> time = std::make_unique<Time>();
        system_clock::time_point now = system_clock::now();
        auto durationInMs = duration_cast<milliseconds>(now.time_since_epoch());
        time_t tt = system_clock::to_time_t(now);
        constexpr std::int16_t sinceYear = 1900;
        constexpr std::int16_t sinceMonth = 1;
        constexpr std::int16_t msDuration = 1000;
        struct tm nowTm{};
#ifdef _WIN32
        if (localtime_s(&nowTm, &tt) != 0) {
            return nullptr;
        }
#else
        if (localtime_r(&tt, &nowTm) == nullptr) {
            return nullptr;
        }
#endif
        time->year = nowTm.tm_year + sinceYear;
        time->month = nowTm.tm_mon + sinceMonth;
        time->day = nowTm.tm_mday;
        time->hour = nowTm.tm_hour;
        time->minute = nowTm.tm_min;
        time->second = nowTm.tm_sec;
        time->millisecond = durationInMs.count() % msDuration;
        return time;
    }

    std::string NowStr(const TimeStyle &style = TimeStyle::DATE_TIME_1)
    {
        using TimeStyleFunc = std::function<std::string(const Time &time)>;
        static std::map<TimeStyle, TimeStyleFunc> funcMap = { { TimeStyle::DATE_TIME_1, GetTimeStyle1 },
                                                              { TimeStyle::DATE_TIME_2, GetTimeStyle2 },
                                                              { TimeStyle::DATE_TIME_3, GetTimeStyle3 },
                                                              { TimeStyle::DATE_TIME_4, GetTimeStyle4 },
                                                              { TimeStyle::DATE_1, GetTimeStyle5 },
                                                              { TimeStyle::TIME_1, GetTimeStyle6 } };
        if (funcMap.count(style) == 0) {
            return "";
        }
        std::unique_ptr<Time> nowPtr = Now();
        if (nowPtr == nullptr) {
            return "";
        }
        return funcMap[style](*nowPtr);
    }

private:
    TimeUtils() = default;

    ~TimeUtils() = default;

    static const int TWO_ALIGN = 2;
    static const int THREE_ALIGN = 3;
    static const int FOUR_ALIGN = 4;

    static std::string GetTimeStyle1(const Time &time)
    {
        std::stringstream ss;
        ss.fill('0');
        ss << std::setw(FOUR_ALIGN) << time.year << "-" << std::setw(TWO_ALIGN) << time.month << "-" <<
            std::setw(TWO_ALIGN) << time.day << " " << std::setw(TWO_ALIGN) << time.hour << ":" <<
            std::setw(TWO_ALIGN) << time.minute << ":" << std::setw(TWO_ALIGN) << time.second << "." <<
            std::setw(THREE_ALIGN) << time.millisecond;
        return ss.str();
    }

    static std::string GetTimeStyle2(const Time &time)
    {
        std::stringstream ss;
        ss.fill('0');
        ss << std::setw(FOUR_ALIGN) << time.year << "-" << std::setw(TWO_ALIGN) << time.month << "-" <<
            std::setw(TWO_ALIGN) << time.day << " " << std::setw(TWO_ALIGN) << time.hour << ":" <<
            std::setw(TWO_ALIGN) << time.minute << ":" << std::setw(TWO_ALIGN) << time.second;
        return ss.str();
    }

    static std::string GetTimeStyle3(const Time &time)
    {
        std::stringstream ss;
        ss.fill('0');
        ss << std::setw(FOUR_ALIGN) << time.year << std::setw(TWO_ALIGN) << time.month << std::setw(TWO_ALIGN) <<
            time.day << std::setw(TWO_ALIGN) << time.hour << std::setw(TWO_ALIGN) << time.minute <<
            std::setw(TWO_ALIGN) << time.second;
        return ss.str();
    }

    static std::string GetTimeStyle4(const Time &time)
    {
        std::stringstream ss;
        ss.fill('0');
        ss << std::setw(TWO_ALIGN) << time.month << "/" <<
           std::setw(TWO_ALIGN) << time.day << " " << std::setw(TWO_ALIGN) << time.hour << ":" <<
           std::setw(TWO_ALIGN) << time.minute << ":" << std::setw(TWO_ALIGN) << time.second << "." <<
           std::setw(THREE_ALIGN) << time.millisecond;
        return ss.str();
    }

    static std::string GetTimeStyle5(const Time &time)
    {
        std::stringstream ss;
        ss.fill('0');
        ss << std::setw(FOUR_ALIGN) << time.year << "-" << std::setw(TWO_ALIGN) << time.month << "-" <<
            std::setw(TWO_ALIGN) << time.day;
        return ss.str();
    }

    static std::string GetTimeStyle6(const Time &time)
    {
        std::stringstream ss;
        ss.fill('0');
        ss << std::setw(TWO_ALIGN) << time.hour << ":" << std::setw(TWO_ALIGN) << time.minute << ":" <<
            std::setw(TWO_ALIGN) << time.second;
        return ss.str();
    }
};
} // end of namespace Dap

#endif // DAP_TIME_UTILS_H
