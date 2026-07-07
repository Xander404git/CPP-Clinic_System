#ifndef PATIENT_H
#define PATIENT_H

#include "Person.h"
#include "AppointmentRecord.h"
#include "MedicalService.h"
#include <vector>
#include <string>

// ============================================================
// Patient.h  –  Derived from Person
// Holds a patient's insurance info and appointment history.
// Demonstrates: inheritance, override of pure virtual, composition via vector<AppointmentRecord>.
// ============================================================

class Patient : public Person {
private:
    std::string insuranceProvider;
    std::string insurancePolicyNumber;
    InsuranceTier insuranceTier;
    std::string bloodType;
    std::vector<std::string> allergies;
    std::vector<AppointmentRecord> appointments; // composition via STL vector

    // Static counter – tracks total patients (separate from base class total)
    static int patientCount;
    static int nextPatientNumber;  // for auto-ID generation

public:
    // ----- Constructors -----
    Patient();
    Patient(const std::string& id,
            const std::string& name,
            const std::string& contact,
            const std::string& email,
            const std::string& dob,
            const std::string& gender,
            const std::string& insuranceProvider,
            const std::string& policyNum,
            InsuranceTier tier,
            const std::string& bloodType);

    // ----- Destructor -----
    ~Patient() override;

    // ----- Override pure virtual -----
    void displayRole() const override;

    // ----- Override type identifier -----
    std::string getType() const override;

    // ----- Appointment management -----
    void scheduleAppointment(const MedicalService& s,
                             const std::string& doctorId,
                             const std::string& dateTime,
                             AppointmentType type = AppointmentType::REGULAR,
                             const std::string& notes = "");

    void addExistingAppointment(const AppointmentRecord& rec);

    bool setAppointmentStatus(const std::string& apptId, const std::string& status);

    bool cancelAppointment(const std::string& apptId);

    // ----- Aggregation cleanup -----
    // When a Doctor is removed from the system, this clears the dangling
    // doctorId reference from any appointment that referenced them.
    // The appointment data itself (service, billing, status) is preserved
    // because it is owned by composition, NOT by the doctor.
    void clearDoctorFromAppointments(const std::string& doctorId);

    void addAllergy(const std::string& allergy);

    // ----- Financial -----
    double calculateTotalMedicalExpenses() const;
    double calculateOutstandingBalance() const;

    // ----- Getters -----
    std::string   getInsuranceProvider()     const;
    std::string   getInsurancePolicyNumber() const;
    InsuranceTier getInsuranceTier()         const;
    std::string   getBloodType()             const;
    const std::vector<AppointmentRecord>& getAppointments() const;
    const std::vector<std::string>& getAllergies() const;

    // ----- Static -----
    static int  getPatientCount();
    static std::string generatePatientId();
    static void setNextPatientNumber(int n);

    // ----- Display -----
    void displayAppointmentHistory() const;
};

#endif // PATIENT_H
