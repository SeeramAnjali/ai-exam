#ifndef CAR_H
#define CAR_H

#include "Diagnostic.h"
#include <optional>
#include <string>

class Car {
public:
    explicit Car(std::string id);

    const std::string& getId() const;
    void addDiagnostic(const Diagnostic& d);

    std::optional<double> rpm() const { return rpm_; }
    std::optional<double> engineLoad() const { return engineLoad_; }
    std::optional<double> coolantTemp() const { return coolantTemp_; }

    bool hasAllRequired() const;
    std::optional<double> computePerformanceScore() const;

private:
    std::string id_;
    std::optional<double> rpm_;
    std::optional<double> engineLoad_;
    std::optional<double> coolantTemp_;
};

#endif // CAR_H
