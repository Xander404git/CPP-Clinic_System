#ifndef MEDICALSERVICE_H
#define MEDICALSERVICE_H

#include <string>
#include <iostream>
#include <iomanip>

// ============================================================
// MedicalService.h
// Represents a clinic/department service offered by the hospital.
// Demonstrates: overloaded constructors (compile-time polymorphism).
// ============================================================

class MedicalService {
private:
    std::string serviceCode;
    std::string serviceName;
    std::string serviceCategory;   // e.g. "Consultation", "Surgery", "Lab"
    double      baseFee;
    bool        requiresReferral;

public:
    // ----- Static counter (tracks total services registered) -----
    static int totalServices;

    // ----- Constructors (overloaded) -----
    // Default – used for composition default-init
    MedicalService();

    // Minimal – defaults baseFee to standard consultation rate (50.0)
    MedicalService(const std::string& code, const std::string& name);

    // Standard – caller supplies all fields
    MedicalService(const std::string& code,
                   const std::string& name,
                   const std::string& category,
                   double fee,
                   bool refRequired = false);

    // Rule of Three to properly track instances
    // Reference: Rule of Three - copy constructor, copy assignment, destructor
    // https://en.cppreference.com/w/cpp/language/rule_of_three
    MedicalService(const MedicalService& other);
    MedicalService& operator=(const MedicalService& other);
    ~MedicalService();

    // ----- Getters -----
    std::string getServiceCode()     const;
    std::string getServiceName()     const;
    std::string getServiceCategory() const;
    double      getBaseFee()         const;
    bool        getRequiresReferral()const;

    // ----- Static helper -----
    static int getTotalServices();

    // ----- Display -----
    void displayServiceDetails() const;

    // ----- Operator overload (bonus polymorphism) -----
    bool operator==(const MedicalService& other) const;
};

#endif // MEDICALSERVICE_H
