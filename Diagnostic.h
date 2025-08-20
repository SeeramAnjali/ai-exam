#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <string>

enum class DiagnosticType {
    RPM,
    EngineLoad,
    CoolantTemp,
    Unknown
};

DiagnosticType diagnosticTypeFromString(const std::string& s);
std::string diagnosticTypeToString(DiagnosticType t);

class Diagnostic {
public:
    Diagnostic(std::string id, DiagnosticType type, double value);

    const std::string& getId() const;
    DiagnosticType getType() const;
    double getValue() const;

private:
    std::string id_;
    DiagnosticType type_;
    double value_;
};

#endif // DIAGNOSTIC_H
