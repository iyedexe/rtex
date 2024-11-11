#pragma once

#include <chrono>
#include <string>
#include <ctime>
#include "logger.hpp"

class Scheduler {
public:
    explicit Scheduler(const std::string& date); // Constructor
    bool waitStart();
    std::chrono::system_clock::time_point getStartTime() const {return startTime_;}
    std::chrono::system_clock::time_point getStopTime() const {return stopTime_;}
    std::chrono::seconds timeUntil(const std::chrono::system_clock::time_point& time_point) const;

private:
    std::string currentDate_;
    std::chrono::system_clock::time_point startTime_;
    std::chrono::system_clock::time_point stopTime_;

    std::string printDuration(std::chrono::seconds duration) const;
    std::string printTime(const std::chrono::system_clock::time_point& time_point, bool local = false) const; // Formatted time (GMT or local)
};