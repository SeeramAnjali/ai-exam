#include "Diagnostic.h"
#include <algorithm>
#include <cctype>

static std::string trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end-1]))) --end;
    return s.substr(start, end - start);
}

static std::string upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
    return s;
}

DiagnosticType diagnosticTypeFromString(const std::string& sIn) {
    std::string s = upper(trim(sIn));
    if (s == "RPM") return DiagnosticType::RPM;
    if (s == "ENGINELOAD") return DiagnosticType::EngineLoad;
    if (s == "COOLANTTEMP") return DiagnosticType::CoolantTemp;
    return DiagnosticType::Unknown;
}

std::string diagnosticTypeToString(DiagnosticType t) {
    switch (t) {
        case DiagnosticType::RPM: return "RPM";
        case DiagnosticType::EngineLoad: return "EngineLoad";
        case DiagnosticType::CoolantTemp: return "CoolantTemp";
        default: return "Unknown";
    }
}

Diagnostic::Diagnostic(std::string id, DiagnosticType type, double value)
: id_(std::move(id)), type_(type), value_(value) {}

const std::string& Diagnostic::getId() const { return id_; }
DiagnosticType Diagnostic::getType() const { return type_; }
double Diagnostic::getValue() const { return value_; }
