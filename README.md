# Garage Monitor (C++) — v2

## Introduction
Loads per-car diagnostics from CSV, computes a **performance score**, and triggers alerts. Includes **10 unit tests**, a simple **debug logging** macro, and a **concurrency** demo that compares single-thread to multi-thread.

### Score Formula
```
score = 100 - (rpm/100 + engineLoad*0.5 + (coolantTemp - 90) * 2)
```

### Alerts
- Missing required diagnostic → `Sensor Failure Detected`
- `score < 40` → `Severe Engine Stress`
- Exactly `score = 40` → no alert

## Build & Run

### Using CMake (Linux/Mac/WSL/Windows)
```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```

**Windows PowerShell run commands:**
```powershell
.\garage.exe ..\diagnostics.csv
.\tests.exe
```

**Linux/macOS run commands:**
```bash
./garage ../diagnostics.csv
./tests
```

### Enable Debug Logging
```bash
# CMake configure step with flag
cmake -DDEBUG_LOGGING=ON ..
cmake --build .
```

### Concurrency Demo
```bash
# Linux/macOS
./garage ../diagnostics.csv --simulate 1000 4

# Windows PowerShell
.\garage.exe ..\diagnostics.csv --simulate 1000 4
```

## Files
- `Diagnostic.h/.cpp` – Diagnostic class & type helpers
- `Car.h/.cpp` – Holds diagnostics and computes score
- `GarageMonitor.h/.cpp` – Thread-safe manager, CSV loading, status/alerts, average score, concurrency
- `main.cpp` – CLI (CSV + optional simulation)
- `tests.cpp` – **10 unit & integration tests** with `cassert`
- `CMakeLists.txt` – Build config with optional `DEBUG_LOGGING`
- `diagnostics.csv` – Example data

## Sample CSV
```
Car1, RPM, 6500
Car1, CoolantTemp, 120
Car1, EngineLoad, 95
Car2, EngineLoad, 95
Car2, RPM, 4500
Car2, CoolantTemp, 88
```

## Sample Output
```
Loaded 6 row(s).
Car: Car1 | Score: -10.00 | Alert: Severe Engine Stress
Car: Car2 | Score: 54.00
```
