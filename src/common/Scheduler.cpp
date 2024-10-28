#include "common/Scheduler.h"
#include <sstream>
#include <iomanip>
#include <thread>
#include <stdexcept>

// Helper to convert tm to time_t in UTC (using timegm)
std::time_t Scheduler::toTimeT(const std::tm& time_struct) const {
    return timegm(const_cast<std::tm*>(&time_struct));  // Uses timegm for GMT conversion
}

Scheduler::Scheduler(const std::string& date) {
    // Parse date string (YYYY-MM-DD) into tm structure
    strptime(date.c_str(), "%Y-%m-%d", &target_date_);
    // Set time to 00:00:00 to ensure the start is midnight
    target_date_.tm_hour = 0;
    target_date_.tm_min = 0;
    target_date_.tm_sec = 0;
}

bool Scheduler::isFuture() const {
    return std::difftime(toTimeT(target_date_), std::time(nullptr)) > 0;
}

bool Scheduler::isToday() const {
    std::time_t now = std::time(nullptr);
    std::tm* now_tm = std::gmtime(&now);  // Use GMT for comparison
    return target_date_.tm_year == now_tm->tm_year &&
           target_date_.tm_mon == now_tm->tm_mon &&
           target_date_.tm_mday == now_tm->tm_mday;
}

std::chrono::seconds Scheduler::timeUntil(const std::chrono::system_clock::time_point& time_point) const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(time_point - now);
}

std::chrono::system_clock::time_point Scheduler::startTime() const {
    // Create a time_point for 00:00:00 GMT on the target day
    return std::chrono::system_clock::from_time_t(toTimeT(target_date_));
}

std::chrono::system_clock::time_point Scheduler::endTime() const {
    std::tm end_day = target_date_;
    end_day.tm_hour = 23;
    end_day.tm_min = 59;
    end_day.tm_sec = 59;
    return std::chrono::system_clock::from_time_t(toTimeT(end_day));
}

std::string Scheduler::humanReadable(std::chrono::seconds duration) const {
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

std::string Scheduler::formattedTime(const std::chrono::system_clock::time_point& time_point, bool local) const {
    std::time_t time_t_value = std::chrono::system_clock::to_time_t(time_point);
    std::tm* time_tm = local ? std::localtime(&time_t_value) : std::gmtime(&time_t_value);

    std::stringstream ss;
    ss << std::put_time(time_tm, "%Y-%m-%d %H:%M:%S");
    ss << (local ? " Local" : " GMT");
    return ss.str();
}
