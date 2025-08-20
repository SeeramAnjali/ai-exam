#include "GarageMonitor.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>

static void printUsage(const char* prog) {
    std::cerr << "Usage:\n"
              << "  " << prog << " <diagnostics.csv>\n"
              << "  " << prog << " <diagnostics.csv> --simulate [iterations] [threads]\n"
              << "    iterations = loop iterations to simulate work (default 1000)\n"
              << "    threads    = number of threads when multi-threading (default 4)\n"
              << "  Add -DDEBUG_LOGGING at compile-time to enable debug logs.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    const char* path = argv[1];
    std::ifstream in(path);
    if (!in) {
        std::cerr << "Error: cannot open file: " << path << "\n";
        return 1;
    }

    GarageMonitor gm;
    std::vector<std::string> errors;
    try {
        size_t loaded = gm.loadCSV(in, errors);
        for (const auto& e : errors) std::cerr << "CSV Warning: " << e << "\n";
        std::cerr << "Loaded " << loaded << " row(s).\n";
    } catch (const std::exception& ex) {
        std::cerr << "CSV Error: " << ex.what() << "\n";
        return 1;
    }

    gm.printStatus(std::cout);

    if (argc >= 3 && std::string(argv[2]) == "--simulate") {
        int iters = (argc >= 4) ? std::stoi(argv[3]) : 1000;
        int threads = (argc >= 5) ? std::stoi(argv[4]) : 4;

        std::cout << "\n--- Real-time Simulation (" << iters << " iterations, " << threads
                  << " thread(s) in MT mode) ---\n";

        auto t1 = gm.simulateRealTimeUpdates(iters, threads, false);
        auto avg1 = gm.averageScore();
        std::cout << "Single-thread elapsed: " << t1 << " ms";
        if (avg1) std::cout << " | avg score: " << std::fixed << std::setprecision(2) << *avg1;
        std::cout << "\n";

        auto t2 = gm.simulateRealTimeUpdates(iters, threads, true);
        auto avg2 = gm.averageScore();
        std::cout << "Multi-thread elapsed:  " << t2 << " ms";
        if (avg2) std::cout << " | avg score: " << std::fixed << std::setprecision(2) << *avg2;
        std::cout << "\n";
    }

    return 0;
}
