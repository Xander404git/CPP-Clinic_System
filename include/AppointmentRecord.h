#ifndef APPOINTMENTRECORD_H
#define APPOINTMENTRECORD_H

#include "MedicalService.h"
#include <string>
#include <ctime>

// ============================================================
// AppointmentRecord.h
// Demonstrates COMPOSITION: AppointmentRecord "has-a" MedicalService.
// Manages billing modifiers (insurance tier, emergency surcharge, etc.)
// ============================================================

// Reference: Scoped enumerations (enum class) for type safety
// https://en.cppreference.com/w/cpp/language/enum
enum class InsuranceTier { NONE, BASIC, STANDARD, PREMIUM };
enum class AppointmentType { REGULAR, EMERGENCY, FOLLOW_UP };

class AppointmentRecord {
private:
    // -- Composition: embedded object (not a pointer) --
    MedicalService  service;

    std::string     appointmentId;
    std::string     appointmentStatus;   // "Scheduled" | "Completed" | "Cancelled" | "Paid"
    AppointmentType apptType;
    InsuranceTier   insuranceTier;
    std::string     dateTime;            // stored as formatted string
    std::string     doctorId;
    std::string     notes;

    // -- Static ID generator --
    static int idCounter;

    std::string generateId();

public:
    // ----- Constructors -----
    AppointmentRecord(); // default
    AppointmentRecord(const MedicalService& svc,
                      const std::string& docId,
                      AppointmentType type     = AppointmentType::REGULAR,
                      InsuranceTier   tier     = InsuranceTier::NONE,
                      const std::string& notes = "");

    // ----- Getters -----
    std::string     getAppointmentId()     const;
    std::string     getServiceCode()       const;
    std::string     getServiceName()       const;
    std::string     getAppointmentStatus() const;
    std::string     getDoctorId()          const;
    std::string     getDateTime()          const;
    AppointmentType getAppointmentType()   const;
    InsuranceTier   getInsuranceTier()     const;
    std::string     getNotes()             const;

    // ----- Setters -----
    void setAppointmentId(const std::string& id);
    void setDateTime(const std::string& dt);
    void setDoctorId(const std::string& docId);
    void setStatus(const std::string& status);
    void setInsuranceTier(InsuranceTier tier);
    void setNotes(const std::string& n);

    // ----- Billing -----
    // Applies emergency surcharge (+30%), insurance discounts, follow-up discount
    double calculateTotalBilling() const;

    // ----- Helpers -----
    static std::string tierToString(InsuranceTier t);
    static std::string typeToString(AppointmentType t);
    static void setIdCounter(int n);

    // ----- Display -----
    void displayRecord() const;
};

#endif // APPOINTMENTRECORD_H
