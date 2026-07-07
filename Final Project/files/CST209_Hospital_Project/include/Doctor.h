#ifndef DOCTOR_H
#define DOCTOR_H

#include "Person.h"
#include <vector>
#include <string>

// ============================================================
// Doctor.h  –  Derived from Person
// Manages doctor specialisation, clinic assignment, schedule.
// Demonstrates: inheritance, override of pure virtual.
// ============================================================

class Doctor : public Person {
private:
    std::string specializationDepartment;
    std::string assignedClinicCode;
    std::string licenseNumber;
    int         yearsOfExperience;
    double      consultationFee;    // Doctor's personal consultation surcharge
    std::vector<std::string> qualifications;
    std::vector<std::string> assignedPatientIds; // tracks patients seen

    // Static counter
    static int doctorCount;
    static int nextDoctorNumber;

public:
    // ----- Constructors -----
    Doctor();
    Doctor(const std::string& id,
           const std::string& name,
           const std::string& contact,
           const std::string& email,
           const std::string& dob,
           const std::string& gender,
           const std::string& specialization,
           const std::string& licenseNum,
           int yearsExp,
           double consultFee);

    // ----- Destructor -----
    ~Doctor() override;

    // ----- Override pure virtual -----
    void displayRole() const override;

    // ----- Override type identifier -----
    std::string getType() const override;

    // ----- Clinic management -----
    void assignToClinic(const std::string& clinicCode);
    void addQualification(const std::string& qual);
    void addAssignedPatient(const std::string& patientId);
    void removeAssignedPatient(const std::string& patientId);

    // ----- Getters -----
    std::string getSpecialization()    const;
    std::string getAssignedClinicCode()const;
    std::string getLicenseNumber()     const;
    int         getYearsOfExperience() const;
    double      getConsultationFee()   const;
    const std::vector<std::string>& getQualifications() const;
    const std::vector<std::string>& getAssignedPatients() const;

    // ----- Static -----
    static int  getDoctorCount();
    static std::string generateDoctorId();
};

#endif // DOCTOR_H
