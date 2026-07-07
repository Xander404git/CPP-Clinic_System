// MedicalService.cpp
#include "../include/MedicalService.h"
#include <iomanip>
#include <iostream>

// ----- Static member definition -----
int MedicalService::totalServices = 0;

// ----- Constructors -----

MedicalService::MedicalService()
    : serviceCode("UNKNOWN"), serviceName("Unknown Service"),
      serviceCategory("General"), baseFee(50.0), requiresReferral(false)
{
    ++totalServices;
}

// Minimal overload – baseFee defaults to standard consultation rate
MedicalService::MedicalService(const std::string& code, const std::string& name)
    : serviceCode(code), serviceName(name),
      serviceCategory("Consultation"), baseFee(50.0), requiresReferral(false)
{
    ++totalServices;
}

// Full overload
MedicalService::MedicalService(const std::string& code,
                               const std::string& name,
                               const std::string& category,
                               double fee,
                               bool refRequired)
    : serviceCode(code), serviceName(name),
      serviceCategory(category), baseFee(fee), requiresReferral(refRequired)
{
    ++totalServices;
}

MedicalService::MedicalService(const MedicalService& other)
    : serviceCode(other.serviceCode), serviceName(other.serviceName),
      serviceCategory(other.serviceCategory), baseFee(other.baseFee), requiresReferral(other.requiresReferral)
{
    ++totalServices;
}

MedicalService& MedicalService::operator=(const MedicalService& other) {
    if (this != &other) {
        serviceCode = other.serviceCode;
        serviceName = other.serviceName;
        serviceCategory = other.serviceCategory;
        baseFee = other.baseFee;
        requiresReferral = other.requiresReferral;
    }
    return *this;
}

MedicalService::~MedicalService() {
    --totalServices;
}

// ----- Getters -----
std::string MedicalService::getServiceCode()      const { return serviceCode; }
std::string MedicalService::getServiceName()      const { return serviceName; }
std::string MedicalService::getServiceCategory()  const { return serviceCategory; }
double      MedicalService::getBaseFee()          const { return baseFee; }
bool        MedicalService::getRequiresReferral() const { return requiresReferral; }

int MedicalService::getTotalServices() { return totalServices; }

// ----- Display -----
void MedicalService::displayServiceDetails() const {
    std::cout << std::left
              << "  Code     : " << serviceCode      << "\n"
              << "  Name     : " << serviceName      << "\n"
              << "  Category : " << serviceCategory  << "\n"
              << std::fixed << std::setprecision(2)
              << "  Base Fee : RM " << baseFee        << "\n"
              << "  Referral : " << (requiresReferral ? "Required" : "Not Required") << "\n";
}

// ----- Operator overload -----
bool MedicalService::operator==(const MedicalService& other) const {
    return serviceCode == other.serviceCode;
}
