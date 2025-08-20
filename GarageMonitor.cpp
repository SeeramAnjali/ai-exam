#include "GarageMonitor.h"
#include "Diagnostic.h"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <thread>
#include <random>
#include <cerrno>
#include <cstdlib>

static std::string trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end-1]))) --end;
    return s.substr(start, end - start);
}

void GarageMonitor::addDiagnostic(const std::string& carId, DiagnosticType type, double value) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = cars_.find(carId);
    if (it == cars_.end()) {
        it = cars_.emplace(carId, Car(carId)).first;
    }
    it->second.addDiagnostic(Diagnostic(carId, type, value));
    DEBUG_LOG("Add " << carId << " " << diagnosticTypeToString(type) << "=" << value);
}

size_t GarageMonitor::loadCSV(std::istream& in, std::vector<std::string>& errors) {
    size_t count = 0;
    std::string line;
    size_t lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        std::string raw = trim(line);
        if (raw.empty()) continue;
        if (!raw.empty() && raw[0] == '#') continue;

        std::stringstream ss(raw);
        std::string carId, typeStr, valueStr;

        if (!std::getline(ss, carId, ',')) {
            errors.push_back("Line " + std::to_string(lineNo) + ": missing CarId");
            continue;
        }
        if (!std::getline(ss, typeStr, ',')) {
            errors.push_back("Line " + std::to_string(lineNo) + ": missing Type");
            continue;
        }
        if (!std::getline(ss, valueStr, ',')) {
            valueStr = "";
        }

        carId    = trim(carId);
        typeStr  = trim(typeStr);
        valueStr = trim(valueStr);

        DiagnosticType type = diagnosticTypeFromString(typeStr);
        if (type == DiagnosticType::Unknown) {
            errors.push_back("Line " + std::to_string(lineNo) + ": unknown Type '" + typeStr + "'");
            continue;
        }

        char* endp = nullptr;
        errno = 0;
        double val = std::strtod(valueStr.c_str(), &endp);
        if (valueStr.empty() || endp == valueStr.c_str() || errno == ERANGE) {
            errors.push_back("Line " + std::to_string(lineNo) + ": invalid Value '" + valueStr + "'");
            continue;
        }

        addDiagnostic(carId, type, val);
        ++count;
    }
    if (count == 0) {
        throw std::runtime_error("Empty CSV: no valid data rows.");
    }
    return count;
}

CarStatus GarageMonitor::statusOfUnlocked(const Car& car) const {
    CarStatus st{};
    st.hasAll = car.hasAllRequired();
    if (!st.hasAll) {
        st.alert = "Sensor Failure Detected";
        return st;
    }
    st.score = car.computePerformanceScore();
    if (st.score && *st.score < 40.0) {
        st.alert = "Severe Engine Stress";
    } else {
        st.alert.clear();
    }
    return st;
}

CarStatus GarageMonitor::statusOf(const std::string& carId) const {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = cars_.find(carId);
    if (it == cars_.end()) return CarStatus{};
    return statusOfUnlocked(it->second);
}

void GarageMonitor::printStatus(std::ostream& out) const {
    std::lock_guard<std::mutex> lock(mtx_);
    out << std::fixed << std::setprecision(2);
    for (const auto& [id, car] : cars_) {
        CarStatus st = statusOfUnlocked(car);
        out << "Car: " << id;
        if (!st.hasAll) {
            out << " | Status: " << st.alert << "\n";
            continue;
        }
        out << " | Score: " << *st.score;
        if (!st.alert.empty()) out << " | Alert: " << st.alert;
        out << "\n";
    }
}

std::optional<double> GarageMonitor::averageScore() const {
    std::lock_guard<std::mutex> lock(mtx_);
    double sum = 0.0;
    int n = 0;
    for (const auto& [id, car] : cars_) {
        if (car.hasAllRequired()) {
            auto s = car.computePerformanceScore();
            if (s) { sum += *s; ++n; }
        }
    }
    if (n == 0) return std::nullopt;
    return sum / n;
}

bool GarageMonitor::hasCar(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mtx_);
    return cars_.find(id) != cars_.end();
}

// durationIterations: loop iterations per thread to simulate work
// threadsPerRun: thread count when multithread==true, else ignored
long long GarageMonitor::simulateRealTimeUpdates(
    int durationIterations,
    int threadsPerRun,
    bool multithread
) {
    using namespace std::chrono;

    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (cars_.empty()) {
            cars_.emplace("Car1", Car("Car1"));
            cars_.emplace("Car2", Car("Car2"));
            cars_.emplace("Car3", Car("Car3"));
        }
    }

    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> rpmDist(600.0, 7000.0);
    std::uniform_real_distribution<double> loadDist(0.0, 100.0);
    std::uniform_real_distribution<double> tempDist(70.0, 130.0);

    auto updateOne = [&](const std::string& id){
        addDiagnostic(id, DiagnosticType::RPM, rpmDist(rng));
        addDiagnostic(id, DiagnosticType::EngineLoad, loadDist(rng));
        addDiagnostic(id, DiagnosticType::CoolantTemp, tempDist(rng));
        (void)statusOf(id);
    };

    auto start = steady_clock::now();

    if (multithread) {
        int threadsN = std::max(1, threadsPerRun);
        std::vector<std::thread> threads;
        threads.reserve(threadsN);
        for (int t = 0; t < threadsN; ++t) {
            threads.emplace_back([&, t](){
                for (int i = 0; i < durationIterations; ++i) {
                    std::vector<std::string> ids;
                    { std::lock_guard<std::mutex> lock(mtx_);
                      for (auto& kv : cars_) ids.push_back(kv.first); }
                    for (auto& id : ids) updateOne(id);
                }
            });
        }
        for (auto& th : threads) th.join();
    } else {
        for (int i = 0; i < durationIterations; ++i) {
            std::vector<std::string> ids;
            { std::lock_guard<std::mutex> lock(mtx_);
              for (auto& kv : cars_) ids.push_back(kv.first); }
            for (auto& id : ids) updateOne(id);
        }
    }

    auto end = steady_clock::now();
    return duration_cast<milliseconds>(end - start).count();
}
