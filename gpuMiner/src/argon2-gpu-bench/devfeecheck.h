#ifndef DEVFEE_H
#define DEVFEE_H

#include <chrono>

bool is_devfee_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);
    tm *timeinfo = std::localtime(&time_now);
    int minutes = timeinfo->tm_min;
    int seconds = timeinfo->tm_sec;
    return (minutes == 47 && seconds >= 30) || (minutes == 56 && seconds < 30) || (minutes >= 48 && minutes <= 55);
}

#endif //DEVFEE_H