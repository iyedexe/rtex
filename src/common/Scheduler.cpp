#include "common/Scheduler.h"
#include <sstream>
#include <iomanip>
#include <thread>
#include <stdexcept>

Scheduler::Scheduler(const std::string& date) {
    currentDate_ = date;
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        throw std::runtime_error("Failed to parse date string");
    }
    {
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        std::time_t time = timegm(&tm);
        startTime_ = std::chrono::system_clock::from_time_t(time);
    }
    {
        tm.tm_hour = 23;
        tm.tm_min = 59;
        tm.tm_sec = 59;
        std::time_t time = timegm(&tm);
        stopTime_ = std::chrono::system_clock::from_time_t(time);
    }
}

bool Scheduler::waitStart() {
    auto timeToStart = timeUntil(startTime_);
    auto timeToStop = timeUntil(stopTime_);
    if (timeToStop.count()<0) {
        LOG_ERROR("[SCHEDULER] Date {} is in the past. Exiting.", currentDate_);
        return false;
    }
    if (timeToStart.count()>0) {
        LOG_INFO("[SCHEDULER] Date {} is in the future. Scheduling start at {} ({}), which is in {}.",
                currentDate_, printTime(startTime_), printTime(startTime_, true), printDuration(timeToStart));

        std::this_thread::sleep_for(timeToStart);
    }    
    LOG_INFO("[SCHEDULER] Date {} is today, starting ...", currentDate_);
    return true;
}

std::chrono::seconds Scheduler::timeUntil(const std::chrono::system_clock::time_point& time_point) const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(time_point - now);
}

std::string Scheduler::printDuration(std::chrono::seconds duration) const {
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes;

    std::stringstream ss;
    if (hours.count() > 0) ss << hours.count() << "h ";
    if (minutes.count() > 0) ss << minutes.count() << "min ";
    ss << duration.count() << "s";
    return ss.str();
}

std::string Scheduler::printTime(const std::chrono::system_clock::time_point& time_point, bool local) const {
    std::time_t time_t_value = std::chrono::system_clock::to_time_t(time_point);
    std::tm* time_tm = local ? std::localtime(&time_t_value) : std::gmtime(&time_t_value);

    std::stringstream ss;
    ss << std::put_time(time_tm, "%Y-%m-%d %H:%M:%S");
    ss << (local ? " Local" : " GMT");
    return ss.str();
}