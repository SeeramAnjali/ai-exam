#ifndef GARAGE_MONITOR_H
#define GARAGE_MONITOR_H

#include "Car.h"
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include <optional>
#include <chrono>

// Simple debug macro: enable with -DDEBUG_LOGGING
#ifdef DEBUG_LOGGING
#include <iostream>
#define DEBUG_LOG(x) do { std::cerr << "[DEBUG] " << x << "\n"; } while(0)
#else
#define DEBUG_LOG(x) do {} while(0)
#endif

struct CarStatus {
    bool hasAll = false;
    std::optional<double> score;
    std::string alert; // "" | "Sensor Failure Detected" | "Severe Engine Stress"
};

class GarageMonitor {
public:
    void addDiagnostic(const std::string& carId, DiagnosticType type, double value);
    size_t loadCSV(std::istream& in, std::vector<std::string>& errors); // throws on empty CSV

    CarStatus statusOf(const std::string& carId) const;
    void printStatus(std::ostream& out) const;
    std::optional<double> averageScore() const;
    long long simulateRealTimeUpdates(int durationIterations, int threadsPerRun, bool multithread);
    bool hasCar(const std::string& id) const;

private:
    CarStatus statusOfUnlocked(const Car& car) const;
    mutable std::mutex mtx_;
    std::map<std::string, Car> cars_;
};

#endif // GARAGE_MONITOR_H
