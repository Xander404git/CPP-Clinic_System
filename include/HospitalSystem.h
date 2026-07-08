#ifndef HOSPITALSYSTEM_H
#define HOSPITALSYSTEM_H

#include "Person.h"
#include "Patient.h"
#include "Doctor.h"
#include "MedicalService.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <memory>

// ============================================================
// HospitalSystem.h
// Central manager: owns all runtime objects.
// Demonstrates: STL vector<unique_ptr<Person>> for polymorphic storage,
//               STL unordered_map<string,MedicalService> for O(1) lookup,
//               exception handling for invalid operations.
// ============================================================

// Custom exceptions
// Reference: Custom exception classes inheriting from std::exception
// https://en.cppreference.com/w/cpp/error/exception
class HospitalException : public std::exception {
    std::string msg;
public:
    explicit HospitalException(const std::string& m) : msg(m) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

class DuplicateIdException : public HospitalException {
public:
    explicit DuplicateIdException(const std::string& id)
        : HospitalException("Duplicate ID: '" + id + "' already exists in the system.") {}
};

class NotFoundException : public HospitalException {
public:
    explicit NotFoundException(const std::string& id)
        : HospitalException("Not found: '" + id + "' does not exist in the system.") {}
};

// ============================================================

// Info about a blocking appointment when trying to remove a doctor
struct DoctorAppointmentInfo {
    std::string patientName;
    std::string patientId;
    std::string patientContact;
    std::string appointmentId;
};

// ============================================================

class HospitalSystem {
private:
    // Polymorphic container – stores both Patient and Doctor via base pointer
    // Reference: Using std::unique_ptr for polymorphic containers
    // https://en.cppreference.com/w/cpp/memory/unique_ptr
    std::vector<std::unique_ptr<Person>> users;

    // O(1) lookup for medical services by code
    // Reference: std::unordered_map for O(1) average lookup
    // https://en.cppreference.com/w/cpp/container/unordered_map
    std::unordered_map<std::string, MedicalService> servicesMap;

    std::string hospitalName;

    // ----- Internal lookup helpers -----
    Patient* findPatient(const std::string& id);
    Doctor*  findDoctor (const std::string& id);
    bool     idExists   (const std::string& id) const;

public:
    explicit HospitalSystem(const std::string& name = "XMUM General Hospital");
    ~HospitalSystem(); // releases all dynamically allocated Person objects

    // ----- User management -----
    void addPatient(std::unique_ptr<Patient> p);
    void addDoctor (std::unique_ptr<Doctor>  d);
    void removeUser(const std::string& id);

    // ----- Service management -----
    void addMedicalService(const MedicalService& svc);
    void removeMedicalService(const std::string& serviceCode);
    void listMedicalServices() const;

    // ----- Appointment operations -----
    void scheduleAppointment(const std::string& patientId,
                             const std::string& serviceCode,
                             const std::string& doctorId,
                             const std::string& dateTime,
                             AppointmentType type = AppointmentType::REGULAR,
                             const std::string& notes = "");

    void setAppointmentStatus(const std::string& patientId,
                              const std::string& apptId,
                              const std::string& status);

    void assignDoctorToClinic(const std::string& doctorId,
                              const std::string& clinicCode);

    // ----- Polymorphic display -----
    void viewAllUsers()     const;  // runtime polymorphism via displayRole()
    void viewAllPatients()  const;
    void viewAllDoctors()   const;

    // ----- Reports -----
    void generateBillingReport(const std::string& patientId) const;
    void generateSystemSummary() const;

    // ----- File persistence (STL file I/O) -----
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);

    // ----- Getters -----
    const std::vector<std::unique_ptr<Person>>& getUsers()  const;
    const std::unordered_map<std::string,MedicalService>& getServices() const;
    int getUserCount()    const;
    int getServiceCount() const;

    // ----- UI Helpers -----
    std::string checkServiceRemovable(const std::string& serviceCode) const;
    std::vector<DoctorAppointmentInfo> checkDoctorRemovable(const std::string& doctorId) const;
    void reassignAppointmentDoctor(const std::string& patientId,
                                   const std::string& apptId,
                                   const std::string& newDoctorId);
    bool hasDoctorConflict(const std::string& doctorId, const std::string& dateTime) const;
};

#endif // HOSPITALSYSTEM_H
