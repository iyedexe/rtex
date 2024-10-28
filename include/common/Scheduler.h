#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <chrono>
#include <string>
#include <ctime>

class Scheduler {
public:
    explicit Scheduler(const std::string& date); // Constructor

    bool isFuture() const;    // Check if date is in the future
    bool isToday() const;     // Check if date is today
    std::chrono::seconds timeUntil(const std::chrono::system_clock::time_point& time_point) const; // Get time until the specified time_point
    std::chrono::system_clock::time_point startTime() const;  // Start of the target day at 00:00:00 GMT
    std::chrono::system_clock::time_point endTime() const;    // End of the target day at 23:59:59 GMT
    std::string humanReadable(std::chrono::seconds duration) const; // Human-readable duration
    std::string formattedTime(const std::chrono::system_clock::time_point& time_point, bool local = false) const; // Formatted time (GMT or local)

private:
    std::tm target_date_;  // Target date in std::tm format
    std::time_t toTimeT(const std::tm& time_struct) const;  // Helper to convert tm to time_t
};

#endif // SCHEDULER_H
