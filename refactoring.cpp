#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <iomanip>

// ---------------------- Diagnostic ----------------------
class Diagnostic {
public:
    enum class Type { RPM, EngineLoad, CoolantTemp, Unknown };

private:
    Type type;
    double value;

public:
    Diagnostic(Type t, double v) : type(t), value(v) {}

    Type getType() const { return type; }
    double getValue() const { return value; }

    static Type fromString(const std::string &s) {
        if (s == "RPM") return Type::RPM;
        if (s == "EngineLoad") return Type::EngineLoad;
        if (s == "CoolantTemp") return Type::CoolantTemp;
        return Type::Unknown;
    }
};

// ---------------------- Car ----------------------
class Car {
private:
    std::string id;
    std::map<Diagnostic::Type, double> diagnostics;

public:
    Car(const std::string &carId) : id(carId) {}

    void addDiagnostic(const Diagnostic &diag) {
        diagnostics[diag.getType()] = diag.getValue();
    }

    std::string getId() const { return id; }

    double computeScore() const {
        // Ensure required sensors are present
        if (diagnostics.count(Diagnostic::Type::RPM) == 0 ||
            diagnostics.count(Diagnostic::Type::EngineLoad) == 0 ||
            diagnostics.count(Diagnostic::Type::CoolantTemp) == 0) {
            throw std::runtime_error("Sensor Failure Detected");
        }

        double rpm = diagnostics.at(Diagnostic::Type::RPM);
        double load = diagnostics.at(Diagnostic::Type::EngineLoad);
        double temp = diagnostics.at(Diagnostic::Type::CoolantTemp);

        // Use constants instead of magic numbers
        const double RPM_FACTOR = 1.0 / 100;
        const double LOAD_FACTOR = 0.5;
        const double TEMP_BASELINE = 90.0;
        const double TEMP_FACTOR = 2.0;

        return 100 - (rpm * RPM_FACTOR + load * LOAD_FACTOR + (temp - TEMP_BASELINE) * TEMP_FACTOR);
    }

    std::string getAlert() const {
        try {
            double score = computeScore();
            if (score < 40) return "Severe Engine Stress";
            return "None";
        } catch (const std::exception &e) {
            return e.what();
        }
    }
};

// ---------------------- GarageMonitor ----------------------
class GarageMonitor {
private:
    std::map<std::string, Car> cars;

public:
    void loadFromCSV(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) throw std::runtime_error("Could not open CSV file");

        std::string line;
        bool empty = true;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            empty = false;

            std::istringstream ss(line);
            std::string carId, typeStr, valueStr;

            if (!std::getline(ss, carId, ',') ||
                !std::getline(ss, typeStr, ',') ||
                !std::getline(ss, valueStr, ',')) {
                throw std::runtime_error("Malformed CSV line: " + line);
            }

            Diagnostic::Type type = Diagnostic::fromString(typeStr);
            if (type == Diagnostic::Type::Unknown) {
                throw std::runtime_error("Unknown diagnostic type: " + typeStr);
            }

            double value = std::stod(valueStr);

            if (cars.find(carId) == cars.end()) {
                cars.emplace(carId, Car(carId));
            }
            cars.at(carId).addDiagnostic(Diagnostic(type, value));
        }

        if (empty) throw std::runtime_error("Empty CSV file");
    }

    void printStatus() const {
        for (const auto &pair : cars) {
            const Car &car = pair.second;
            std::cout << "Car: " << car.getId();

            try {
                double score = car.computeScore();
                std::cout << " | Score: " << std::fixed << std::setprecision(2) << score;
            } catch (...) {
                std::cout << " | Score: N/A";
            }

            std::cout << " | Alert: " << car.getAlert() << "\n";
        }
    }
};

// ---------------------- Main ----------------------
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: garage <diagnostics.csv>\n";
        return 1;
    }

    try {
        GarageMonitor gm;
        gm.loadFromCSV(argv[1]);
        gm.printStatus();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
