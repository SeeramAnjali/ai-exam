#include "GarageMonitor.h"
#include <cassert>
#include <sstream>
#include <iostream>
#include <cmath>
#include <vector>

static bool approx(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) < eps;
}

int main() {
    // 1) Severe Engine Stress scenario
    {
        GarageMonitor gm;
        gm.addDiagnostic("CarX", DiagnosticType::RPM, 6500);
        gm.addDiagnostic("CarX", DiagnosticType::EngineLoad, 95);
        gm.addDiagnostic("CarX", DiagnosticType::CoolantTemp, 120);
        auto st = gm.statusOf("CarX");
        assert(st.hasAll);
        assert(st.score.has_value());
        assert(*st.score < 40.0);
        assert(st.alert == "Severe Engine Stress");
    }

    // 2) Missing coolant temp -> Sensor Failure
    {
        GarageMonitor gm;
        gm.addDiagnostic("CarY", DiagnosticType::RPM, 3000);
        gm.addDiagnostic("CarY", DiagnosticType::EngineLoad, 50);
        auto st = gm.statusOf("CarY");
        assert(!st.hasAll);
        assert(st.alert == "Sensor Failure Detected");
        assert(!st.score.has_value());
    }

    // 3) Average score 70 and 30 -> 50
    {
        GarageMonitor gm;
        auto tempForScore = [](double s){ return 90.0 + (100.0 - s)/2.0; };
        gm.addDiagnostic("A", DiagnosticType::RPM, 0);
        gm.addDiagnostic("A", DiagnosticType::EngineLoad, 0);
        gm.addDiagnostic("A", DiagnosticType::CoolantTemp, tempForScore(70));
        gm.addDiagnostic("B", DiagnosticType::RPM, 0);
        gm.addDiagnostic("B", DiagnosticType::EngineLoad, 0);
        gm.addDiagnostic("B", DiagnosticType::CoolantTemp, tempForScore(30));
        auto avg = gm.averageScore();
        assert(avg.has_value());
        assert(approx(*avg, 50.0));
    }

    // 4) Boundary score = 40 -> no alert
    {
        GarageMonitor gm;
        gm.addDiagnostic("Bnd", DiagnosticType::RPM, 0);
        gm.addDiagnostic("Bnd", DiagnosticType::EngineLoad, 0);
        gm.addDiagnostic("Bnd", DiagnosticType::CoolantTemp, 120);
        auto st = gm.statusOf("Bnd");
        assert(st.hasAll);
        assert(st.score.has_value());
        assert(approx(*st.score, 40.0));
        assert(st.alert.empty());
    }

    // 5) Empty CSV must throw
    {
        GarageMonitor gm;
        std::stringstream emptyCSV("");
        std::vector<std::string> errors;
        bool threw = false;
        try {
            gm.loadCSV(emptyCSV, errors);
        } catch (...) {
            threw = true;
        }
        assert(threw);
    }

    // 6) Invalid type row should be ignored, as long as at least one valid row exists (no throw)
    {
        GarageMonitor gm;
        std::stringstream csv(
            "Car1, RPM, 1000\n"
            "Car1, BadType, 10\n"   // invalid type -> error list, ignored
            "Car1, CoolantTemp, 110\n"
        );
        std::vector<std::string> errors;
        size_t n = gm.loadCSV(csv, errors);
        assert(n == 2);
        assert(!errors.empty());
        auto st = gm.statusOf("Car1");
        assert(!st.hasAll); // missing EngineLoad
        assert(st.alert == "Sensor Failure Detected");
    }

    // 7) Non-numeric value should be ignored (error recorded)
    {
        GarageMonitor gm;
        std::stringstream csv(
            "Car1, RPM, abc\n"
            "Car1, RPM, 900\n"
        );
        std::vector<std::string> errors;
        size_t n = gm.loadCSV(csv, errors);
        assert(n == 1);
        assert(!errors.empty());
        auto st = gm.statusOf("Car1");
        assert(!st.hasAll);
    }

    // 8) averageScore() returns nullopt when no car has complete diagnostics
    {
        GarageMonitor gm;
        gm.addDiagnostic("P", DiagnosticType::RPM, 1000);
        auto avg = gm.averageScore();
        assert(!avg.has_value());
    }

    // 9) CSV with extra spaces and case-insensitive types should parse
    {
        GarageMonitor gm;
        std::stringstream csv(
            "  CarZ ,   rpm , 2000  \n"
            "CarZ, ENGINELOAD , 10\n"
            "CarZ, CoolantTemp, 90\n"
        );
        std::vector<std::string> errors;
        size_t n = gm.loadCSV(csv, errors);
        assert(n == 3);
        auto st = gm.statusOf("CarZ");
        assert(st.hasAll);
        assert(st.score.has_value());
        // rpm/100 + load*0.5 + (temp-90)*2 => 20 + 5 + 0 = 25 => score 75
        assert(approx(*st.score, 75.0));
        assert(st.alert.empty());
    }

    // 10) Concurrency simulateRealTimeUpdates should run and produce non-negative elapsed
    {
        GarageMonitor gm;
        // seed cars via a quick CSV
        std::stringstream csv("A,RPM,1000\nA,EngineLoad,10\nA,CoolantTemp,90\n");
        std::vector<std::string> errors;
        gm.loadCSV(csv, errors);
        auto t1 = gm.simulateRealTimeUpdates(100, 1, false);
        auto t2 = gm.simulateRealTimeUpdates(100, 2, true);
        assert(t1 >= 0 && t2 >= 0);
        assert(gm.hasCar("A"));
        auto avg = gm.averageScore();
        assert(avg.has_value());
    }

    std::cout << "All tests passed.\n";
    return 0;
}
