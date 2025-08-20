#include "Car.h"

Car::Car(std::string id) : id_(std::move(id)) {}

const std::string& Car::getId() const { return id_; }

void Car::addDiagnostic(const Diagnostic& d) {
    switch (d.getType()) {
        case DiagnosticType::RPM:        rpm_ = d.getValue(); break;
        case DiagnosticType::EngineLoad: engineLoad_ = d.getValue(); break;
        case DiagnosticType::CoolantTemp:coolantTemp_ = d.getValue(); break;
        default: break;
    }
}

bool Car::hasAllRequired() const {
    return rpm_.has_value() && engineLoad_.has_value() && coolantTemp_.has_value();
}

std::optional<double> Car::computePerformanceScore() const {
    if (!hasAllRequired()) return std::nullopt;
    double rpmVal  = *rpm_;
    double loadVal = *engineLoad_;
    double tempVal = *coolantTemp_;
    return 100.0 - (rpmVal / 100.0 + loadVal * 0.5 + (tempVal - 90.0) * 2.0);
}
