# CST209 Final Project Report
# Hospital Patient and Appointment Management System
# XMUM General Hospital

---

## Abstract

This report documents the design, architecture, and implementation of a Hospital Patient and Appointment Management System developed in C++ for the CST209 Object-Oriented Programming course. The application manages patient registrations, doctor assignments, medical service catalogues, appointment scheduling, billing, and persistent file storage. The system was architected using a multi-file project structure with seven header files and seven source files, employing advanced object-oriented principles including abstract base classes, multi-level inheritance, runtime and compile-time polymorphism, composition and aggregation relationships, custom template libraries, scoped enumerations, smart pointers for automatic memory management, and a three-level custom exception hierarchy. This report explains the rationale behind the choice of Standard Template Library (STL) containers, the inheritance and polymorphism architecture, and the role of static members and additional advanced components integrated into the system.

---

## 1. Introduction

Modern hospital management demands software systems that can handle complex relationships between patients, doctors, medical services, and appointment records while maintaining data integrity and providing a robust user experience. This project applies core C++ object-oriented programming concepts to model these real-world healthcare entities and their interactions.

The system is structured as a multi-file project comprising seven classes distributed across separate header (.h) and implementation (.cpp) files: Person, Patient, Doctor, MedicalService, AppointmentRecord, HospitalSystem, and InputValidator. The main.cpp file serves as the entry point, providing an 18-option interactive command-line interface with dynamic selection menus, input validation, and comprehensive error handling. The project compiles under the C++17 standard using a cross-platform Makefile.

The following sections address the three documentation requirements specified in Part II of the assignment: STL explanation, inheritance and polymorphism rationale, and static functions with additional advanced components.

---

## 2. STL Explanation

The Standard Template Library (STL) was used extensively throughout this project, with multiple container types selected based on the specific structural and performance requirements of each data relationship. Rather than defaulting to a single container type, each STL choice was made deliberately to match the access patterns and ownership semantics of the data it holds.

### 2.1 std::vector

The std::vector container was used in several distinct roles throughout the system. In the HospitalSystem class, a vector of smart pointers (std::vector<std::unique_ptr<Person>>) serves as the primary polymorphic container for storing both Patient and Doctor objects through their shared base class Person. The vector was chosen here because the system requires sequential iteration over all users when performing polymorphic display operations such as "View All Users," where the displayRole() virtual function is called on each element. The contiguous memory layout of std::vector makes this iteration cache-efficient, which is important when the system scales to hundreds of registered users. Additionally, the vector supports dynamic resizing as patients and doctors are continuously added or removed during runtime, abstracting away the manual memory reallocation that would be required with raw arrays.

Within the Patient class, a std::vector<AppointmentRecord> stores the patient's appointment history through composition. This vector owns the AppointmentRecord objects directly by value, meaning that when a Patient object is destroyed, all of its appointment records are automatically destroyed as well. This ownership model correctly reflects the real-world relationship: a patient's appointment history belongs to that patient and should not exist independently. The vector also provides index-based access for the interactive selection menus, where the system dynamically generates numbered lists of appointments for the user to choose from.

The Doctor class uses std::vector<std::string> for both qualifications and assigned patient IDs. The qualifications vector stores a variable-length list of professional credentials, while the assignedPatientIds vector tracks which patients currently have active appointments with the doctor. The erase-remove idiom (std::remove combined with std::vector::erase) is used in the Doctor class to efficiently remove patient IDs when appointments are completed or cancelled, following the standard C++ pattern for removing elements from a vector by value.

### 2.2 std::unordered_map

The medical service catalogue is stored in a std::unordered_map<std::string, MedicalService> within the HospitalSystem class. This container was chosen specifically because the service code (such as "XRAY01" or "CONS01") serves as a natural unique key, and the primary access pattern involves looking up a service by its code when scheduling appointments or calculating billing. Unlike std::map, which uses a balanced binary search tree with O(log N) lookup complexity, std::unordered_map uses a hash table internally, providing O(1) average-case lookup time. In a hospital environment where services are looked up frequently during appointment scheduling and billing calculations, this constant-time performance ensures that the system remains responsive regardless of how many services are registered.

The unordered_map also provides the count() method for O(1) existence checks (used before adding a service to prevent duplicates via DuplicateIdException) and the erase() method for O(1) removal (used when removing a medical service from the catalogue).

### 2.3 std::unique_ptr and Smart Pointer Usage

The system uses std::unique_ptr<Person> rather than raw Person* pointers for the polymorphic user container. This is a deliberate design decision that provides automatic memory management: when a unique_ptr is destroyed (either through vector::erase during user removal or when the HospitalSystem destructor clears the vector), the pointed-to object is automatically deallocated. This eliminates the risk of memory leaks that would be present with manual new/delete management. The std::move function is used throughout the codebase to transfer ownership of unique_ptr objects, following modern C++ move semantics.

### 2.4 File I/O with STL Streams

The system implements full data persistence using std::ofstream for writing and std::ifstream for reading. The save/load functionality uses a pipe-delimited format with section markers ([SERVICES], [USERS], [APPOINTMENTS]) to serialize the entire system state to a text file and reconstruct it on load. String splitting during file loading is performed using a lambda function that wraps std::istringstream with std::getline using a custom delimiter, which is the idiomatic C++ approach to tokenizing strings. The load function also includes a static ID counter synchronization step that scans all loaded IDs to determine the highest used number, preventing ID collisions when new entities are created after loading.

---

## 3. Inheritance and Polymorphism Rationale

### 3.1 Inheritance Hierarchy

The inheritance architecture of this system is built around the principle that different hospital entities share fundamental demographic attributes but possess entirely different operational roles and data requirements. The Person class serves as an Abstract Base Class (ABC) that encapsulates the common attributes shared by all hospital users: id, name, contactNumber, email, dateOfBirth, and gender. By declaring the pure virtual function virtual void displayRole() const = 0, the Person class cannot be instantiated directly, enforcing the architectural constraint that every user in the system must be either a Patient or a Doctor with their own specific implementation of how their role is displayed.

The Patient class inherits publicly from Person (class Patient : public Person), establishing a strict "IS-A" relationship: a Patient is a Person. This inheritance eliminates the need to duplicate the six common demographic fields and their associated getter functions. The Patient class then extends the base with specialised attributes including insuranceProvider, insurancePolicyNumber, an InsuranceTier enumeration, bloodType, a vector of allergies, and most importantly, a vector of AppointmentRecord objects that represents the composition relationship between a patient and their medical history. The Patient constructor uses member initializer lists to delegate the common fields to the Person base constructor, following the C++ best practice of initialising base class members before derived class members.

The Doctor class similarly inherits from Person and extends it with specializationDepartment, assignedClinicCode, licenseNumber, yearsOfExperience, consultationFee, a vector of qualifications, and a vector of assigned patient IDs. The Doctor class also tracks a patientsSeen counter that is incremented when appointments are marked as completed, providing performance metrics at the doctor level.

### 3.2 Runtime Polymorphism

Runtime polymorphism is the central architectural mechanism that allows the system to treat all users uniformly through a single container while preserving their type-specific behaviour. This is achieved through three interconnected mechanisms.

First, the Person class declares two pure virtual functions: virtual void displayRole() const = 0 and virtual std::string getType() const = 0. Both Patient and Doctor provide their own override implementations. The Patient's displayRole() prints the patient's demographic data, insurance information, blood type, allergies, and a formatted table of their appointment history with billing totals. The Doctor's displayRole() prints their specialisation, license, clinic assignment, qualifications, and patient workload statistics.

Second, the HospitalSystem stores all users in a single std::vector<std::unique_ptr<Person>>. When the "View All Users" function iterates through this container and calls p->displayRole() on each element, the C++ runtime uses the virtual table (vtable) mechanism to determine at runtime whether the object is a Patient or a Doctor, and dispatches the call to the correct overridden method. This is dynamic dispatch, or late binding, and it is the defining characteristic of runtime polymorphism in C++.

Third, the Person class declares a virtual destructor (virtual ~Person()), which is mandatory for any class that serves as a polymorphic base. Without a virtual destructor, deleting a derived object through a base class pointer would result in undefined behaviour, as only the base class destructor would be called, potentially leaking the derived class's resources. Both Patient and Doctor explicitly mark their destructors with the override keyword to enforce correctness at compile time.

The system also uses dynamic_cast<Patient*> and dynamic_cast<Doctor*> extensively for safe downcasting when type-specific operations are needed, such as accessing a patient's appointment list or a doctor's specialisation. The dynamic_cast operator uses Runtime Type Information (RTTI) to verify the cast at runtime, returning nullptr if the object is not of the requested type, which prevents undefined behaviour from incorrect type assumptions.

### 3.3 Compile-Time Polymorphism

In addition to runtime polymorphism, the system demonstrates compile-time polymorphism through constructor overloading and operator overloading. The MedicalService class provides three overloaded constructors: a default constructor for composition default-initialisation, a two-parameter constructor that defaults the base fee to a standard consultation rate of RM 50.00, and a five-parameter constructor that accepts all fields including category and referral requirements. This allows the system to create MedicalService objects with varying levels of detail depending on the context. The MedicalService class also overloads the == operator to compare services by their service code, and implements the Rule of Three (copy constructor, copy assignment operator, and destructor) to properly track the static totalServices counter across copies and assignments.

---

## 4. Static Functions, Members, and Additional Components

### 4.1 Static Members and Functions

Static members and functions play a critical role in this system for managing class-level state that exists independently of any specific object instance. Each of the core classes uses static members for different purposes.

The Patient class maintains a static integer patientCount that is incremented in every constructor and decremented in the destructor. This counter provides a global, always-accurate count of how many Patient objects currently exist in memory, which is displayed in the System Summary. The static function getPatientCount() provides read-only access to this counter. A separate static integer nextPatientNumber is used by the static function generatePatientId() to produce auto-incremented IDs in the format "PAT1001", "PAT1002", etc. By making ID generation a static class-level operation rather than an instance method, the system guarantees that no two patients can ever receive the same ID, even if multiple patients are created in rapid succession. The static function setNextPatientNumber() allows the file loading system to synchronise this counter after restoring saved data, preventing ID collisions.

The Doctor class follows the same pattern with doctorCount, nextDoctorNumber, getDoctorCount(), generateDoctorId() (producing IDs like "DOC101", "DOC102"), and setNextDoctorNumber(). The AppointmentRecord class uses a static idCounter to generate unique appointment IDs in the format "APT1001", "APT1002", with a corresponding setIdCounter() for file load synchronisation. The MedicalService class uses a public static totalServices counter that tracks how many service instances exist.

The Person base class provides two static utility functions, printDivider() and printCentred(), that are used throughout the entire system for consistent console formatting. These are appropriately static because they perform formatting operations that do not depend on any specific Person instance.

### 4.2 Custom Exception Hierarchy

A three-level custom exception hierarchy was implemented to provide robust, informative error handling throughout the system. The base class HospitalException inherits from std::exception and overrides the what() method with the noexcept specifier, returning a custom error message string. Two derived exception classes, DuplicateIdException and NotFoundException, inherit from HospitalException and provide specialised error messages for the two most common error conditions in the system.

The main event loop wraps every menu operation in a try-catch block that first catches HospitalException (for application-level errors like duplicate IDs or missing records) and then catches std::exception (for unexpected system-level errors). This architecture ensures that no error condition can crash the program. Instead, every error is caught, a descriptive message is displayed to the user, and the system returns safely to the main menu. This is significantly more robust than simply printing error messages and exiting, as it allows the hospital receptionist to correct their input and continue working without restarting the application.

### 4.3 Custom Template Library (InputValidator)

Beyond the standard requirements, the system includes a custom template library in InputValidator.h that demonstrates advanced C++ template usage. The generic function template Input::getValidInput<T>() accepts a prompt string, a std::function<bool(const T&)> validator callback, and an error message. It repeatedly prompts the user until valid input is provided, using the validator lambda to enforce custom constraints. An explicit template specialisation for std::string (template<> getValidInput<std::string>) overrides the default behaviour to use std::getline instead of cin >> for string input, allowing the system to handle strings containing spaces. This template is instantiated in main.cpp for double and string types with custom validation lambdas for date format checking, gender validation, and numeric range enforcement.

### 4.4 Scoped Enumerations

The system uses enum class (scoped enumerations) for InsuranceTier (NONE, BASIC, STANDARD, PREMIUM) and AppointmentType (REGULAR, EMERGENCY, FOLLOW_UP). Unlike traditional C-style enums, scoped enumerations provide type safety by preventing implicit conversions to integers and avoiding name collisions. The billing calculation in AppointmentRecord::calculateTotalBilling() uses switch statements on these enums to apply modifiers: emergency appointments receive a 30% surcharge, follow-up appointments receive a 30% discount, and insurance tiers provide 15%, 30%, or 50% coverage respectively.

### 4.5 Composition and Aggregation Relationships

The system carefully distinguishes between composition (strong ownership) and aggregation (weak reference) relationships. The AppointmentRecord class contains a MedicalService object by value (composition), meaning the service data is embedded directly within the appointment and destroyed with it. The Patient class contains a vector of AppointmentRecord objects (also composition). In contrast, the AppointmentRecord stores only a doctorId string (aggregation), creating a weak reference to the Doctor. When a Doctor is removed from the system, the removeUser() method in HospitalSystem iterates through all patients and calls clearDoctorFromAppointments() to null out the dangling doctor references, while preserving the appointment data itself. This cleanup logic is explicitly commented in the source code to demonstrate understanding of the difference between composition and aggregation ownership semantics.

### 4.6 Dynamic Interactive Selection Menus

Going beyond a standard text-input system, the application implements dynamic, context-aware selection menus. When scheduling an appointment, the system presents numbered lists of available patients, services, and doctors rather than requiring the user to memorise and type raw IDs. When removing a medical service, the system uses the checkServiceRemovable() method to pre-scan all active appointments and displays [BLOCKED] or SAFE TO REMOVE directly on the menu. When removing a doctor with active appointments, the system presents the blocking appointments with patient contact information and offers an option to reassign all appointments to a different doctor before proceeding with deletion. These features elevate the application from a basic academic exercise to an industry-grade user experience.

---

## 5. References

1. cppreference.com, "std::vector," C++ Reference. Available: https://en.cppreference.com/w/cpp/container/vector

2. cppreference.com, "std::unordered_map," C++ Reference. Available: https://en.cppreference.com/w/cpp/container/unordered_map

3. cppreference.com, "std::unique_ptr," C++ Reference. Available: https://en.cppreference.com/w/cpp/memory/unique_ptr

4. cppreference.com, "Abstract class (pure virtual functions)," C++ Reference. Available: https://en.cppreference.com/w/cpp/language/abstract_class

5. cppreference.com, "Virtual destructors," C++ Reference. Available: https://en.cppreference.com/w/cpp/language/destructor

6. cppreference.com, "dynamic_cast," C++ Reference. Available: https://en.cppreference.com/w/cpp/language/dynamic_cast

7. cppreference.com, "std::exception," C++ Reference. Available: https://en.cppreference.com/w/cpp/error/exception

8. cppreference.com, "Function templates," C++ Reference. Available: https://en.cppreference.com/w/cpp/language/function_template

9. cppreference.com, "Rule of three/five/zero," C++ Reference. Available: https://en.cppreference.com/w/cpp/language/rule_of_three

10. cppreference.com, "enum class (scoped enumerations)," C++ Reference. Available: https://en.cppreference.com/w/cpp/language/enum

11. cppreference.com, "std::ofstream / std::ifstream," C++ Reference. Available: https://en.cppreference.com/w/cpp/header/fstream

12. cppreference.com, "std::numeric_limits," C++ Reference. Available: https://en.cppreference.com/w/cpp/types/numeric_limits

13. Stack Overflow, "How do I iterate over the words of a string?" Available: https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string

14. cppreference.com, "std::istringstream," C++ Reference. Available: https://en.cppreference.com/w/cpp/io/basic_istringstream

15. cppreference.com, "std::find_if," C++ Reference. Available: https://en.cppreference.com/w/cpp/algorithm/find


---

## 6. Appendix: Full Source Code

This appendix contains the complete, fully commented source code for the project, including citations for patterns adapted from online resources.

### include/Person.h

```cpp
#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <iostream>

// ============================================================
// Person.h  –  Abstract Base Class (ABC)
// Defines the common interface for all hospital system users.
// Contains a pure virtual function displayRole() = 0,
// making direct instantiation impossible.
// ============================================================

class Person {
protected:
    std::string id;
    std::string name;
    std::string contactNumber;
    std::string email;
    std::string dateOfBirth;
    std::string gender;

    // Static counter shared across all derived types
    static int totalPersonCount;

public:
    // ----- Constructors -----
    Person();
    Person(const std::string& id,
           const std::string& name,
           const std::string& contact,
           const std::string& email,
           const std::string& dob,
           const std::string& gender);

    // ----- Virtual Destructor (mandatory for polymorphic base) -----
    // Reference: Virtual destructors for polymorphic base classes
    // https://en.cppreference.com/w/cpp/language/destructor#Virtual_destructors
    virtual ~Person();

    // ----- Pure Virtual Function (makes this class abstract) -----
    // Reference: Pure virtual functions and abstract classes
    // https://en.cppreference.com/w/cpp/language/abstract_class
    virtual void displayRole() const = 0;

    // ----- Virtual functions with default implementation -----
    virtual std::string getType() const = 0;
    virtual void displaySummary() const;

    // ----- Getters -----
    std::string getId()            const;
    std::string getName()          const;
    std::string getContactNumber() const;
    std::string getEmail()         const;
    std::string getDateOfBirth()   const;
    std::string getGender()        const;

    // ----- Static -----
    static int getTotalPersonCount();

    // ----- Utility -----
    static void printDivider(char c = '=', int width = 55);
    static void printCentred(const std::string& text, int width = 55);
};

#endif // PERSON_H

```

### include/Patient.h

```cpp
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

```

### include/Doctor.h

```cpp
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
    std::vector<std::string> assignedPatientIds; // tracks active patient assignments
    int patientsSeen;  // completed appointment counter

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
    void incrementPatientsSeen();

    // ----- Getters -----
    std::string getSpecialization()    const;
    std::string getAssignedClinicCode()const;
    std::string getLicenseNumber()     const;
    int         getYearsOfExperience() const;
    double      getConsultationFee()   const;
    const std::vector<std::string>& getQualifications() const;
    const std::vector<std::string>& getAssignedPatients() const;
    int getPatientsSeen() const;
    void setPatientsSeen(int n);

    // ----- Static -----
    static int  getDoctorCount();
    static std::string generateDoctorId();
    static void setNextDoctorNumber(int n);
};

#endif // DOCTOR_H

```

### include/MedicalService.h

```cpp
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

```

### include/AppointmentRecord.h

```cpp
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

```

### include/HospitalSystem.h

```cpp
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

```

### include/InputValidator.h

```cpp
#ifndef INPUTVALIDATOR_H
#define INPUTVALIDATOR_H

#include <iostream>
#include <string>
#include <limits>
#include <functional>
#include <sstream>

// ============================================================
// InputValidator.h – Custom Template Library
// Demonstrates advanced C++ template usage for type-safe,
// robust CLI input handling with custom validation rules.
// ============================================================

namespace Input {

    // Clears the input buffer to prevent infinite loops on bad input
    inline void clearBuffer() {
        std::cin.clear();
        // Reference: std::numeric_limits for clearing input buffer
        // https://en.cppreference.com/w/cpp/types/numeric_limits
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Generic template function for validated input
    // Reference: Function templates and template specialization
    // https://en.cppreference.com/w/cpp/language/function_template
    template <typename T>
    T getValidInput(const std::string& prompt, 
                    std::function<bool(const T&)> validator = [](const T&) { return true; }, 
                    const std::string& errorMsg = "  [!] Invalid input. Please try again.\n") 
    {
        T value;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value) {
                if (validator(value)) {
                    clearBuffer();
                    return value;
                }
            }
            std::cout << errorMsg;
            clearBuffer();
        }
    }

    // Specialization for std::string to handle spaces (getline)
    template <>
    inline std::string getValidInput<std::string>(const std::string& prompt, 
                                           std::function<bool(const std::string&)> validator, 
                                           const std::string& errorMsg) 
    {
        std::string value;
        while (true) {
            std::cout << prompt;
            std::getline(std::cin, value);
            if (validator(value)) {
                return value;
            }
            std::cout << errorMsg;
        }
    }
    
    // Convenience helper for non-empty strings
    inline std::string getNonEmptyString(const std::string& prompt) {
        return getValidInput<std::string>(
            prompt,
            [](const std::string& s) { return !s.empty(); },
            "  [!] Input cannot be empty.\n"
        );
    }

} // namespace Input

#endif // INPUTVALIDATOR_H

```

### src/Person.cpp

```cpp
// Person.cpp
#include "../include/Person.h"
#include <iostream>
#include <iomanip>

// ----- Static member -----
int Person::totalPersonCount = 0;

// ----- Constructors -----
Person::Person()
    : id(""), name(""), contactNumber(""), email(""), dateOfBirth(""), gender("")
{
    ++totalPersonCount;
}

Person::Person(const std::string& id,
               const std::string& name,
               const std::string& contact,
               const std::string& email,
               const std::string& dob,
               const std::string& gender)
    : id(id), name(name), contactNumber(contact),
      email(email), dateOfBirth(dob), gender(gender)
{
    ++totalPersonCount;
}

// ----- Virtual destructor -----
Person::~Person() {
    --totalPersonCount;
}

// ----- Getters -----
std::string Person::getId()            const { return id; }
std::string Person::getName()          const { return name; }
std::string Person::getContactNumber() const { return contactNumber; }
std::string Person::getEmail()         const { return email; }
std::string Person::getDateOfBirth()   const { return dateOfBirth; }
std::string Person::getGender()        const { return gender; }

int Person::getTotalPersonCount() { return totalPersonCount; }

// ----- Default virtual implementation -----
void Person::displaySummary() const {
    std::cout << "  ID      : " << id            << "\n"
              << "  Name    : " << name          << "\n"
              << "  Contact : " << contactNumber << "\n"
              << "  Email   : " << email         << "\n"
              << "  DOB     : " << dateOfBirth   << "\n"
              << "  Gender  : " << gender        << "\n";
}

// ----- Utility statics -----
void Person::printDivider(char c, int width) {
    std::cout << std::string(width, c) << "\n";
}

void Person::printCentred(const std::string& text, int width) {
    int pad = (width - static_cast<int>(text.size())) / 2;
    if (pad > 0) std::cout << std::string(pad, ' ');
    std::cout << text << "\n";
}

```

### src/Patient.cpp

```cpp
// Patient.cpp
#include "../include/Patient.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <stdexcept>

// ----- Static members -----
int Patient::patientCount      = 0;
int Patient::nextPatientNumber = 1000;

// ----- Constructors -----
// Initialize default patient attributes and increment counter
Patient::Patient()
    : Person(), insuranceProvider("None"), insurancePolicyNumber(""),
      insuranceTier(InsuranceTier::NONE), bloodType("Unknown")
{
    ++patientCount;
}

// Parameterized constructor initializing base Person and specific Patient attributes
Patient::Patient(const std::string& id,
                 const std::string& name,
                 const std::string& contact,
                 const std::string& email,
                 const std::string& dob,
                 const std::string& gender,
                 const std::string& insProvider,
                 const std::string& policyNum,
                 InsuranceTier tier,
                 const std::string& blood)
    : Person(id, name, contact, email, dob, gender),
      insuranceProvider(insProvider), insurancePolicyNumber(policyNum),
      insuranceTier(tier), bloodType(blood)
{
    ++patientCount;
}

Patient::~Patient() {
    --patientCount;
}

// ----- Pure virtual override -----
// Display detailed patient record and billing summary
void Patient::displayRole() const {
    Person::printDivider('-', 55);
    std::cout << "  [PATIENT RECORD]\n";
    Person::printDivider('-', 55);
    displaySummary();
    std::cout << "  Blood Type  : " << bloodType << "\n";
    std::cout << "  Insurance   : " << insuranceProvider
              << " (" << AppointmentRecord::tierToString(insuranceTier) << ")\n";
    std::cout << "  Policy No.  : " << insurancePolicyNumber << "\n";
    // Print a comma-separated list of allergies if any exist
    if (!allergies.empty()) {
        std::cout << "  Allergies   : ";
        for (size_t i = 0; i < allergies.size(); ++i)
            std::cout << allergies[i] << (i + 1 < allergies.size() ? ", " : "");
        std::cout << "\n";
    }
    int activeAppts = 0;
    for (const auto& rec : appointments) {
        if (rec.getAppointmentStatus() == "Scheduled") {
            activeAppts++;
        }
    }
    std::cout << "  Active Appts: " << activeAppts << " (Total: " << appointments.size() << ")\n";
    std::cout << std::fixed << std::setprecision(2)
              << "  Total Bill  : RM " << calculateTotalMedicalExpenses() << "\n";
    Person::printDivider('-', 55);
}

std::string Patient::getType() const { return "Patient"; }

// ----- Appointment management -----
// Create a new appointment and append it to the patient's record
void Patient::scheduleAppointment(const MedicalService& s,
                                  const std::string& doctorId,
                                  const std::string& dateTime,
                                  AppointmentType type,
                                  const std::string& notes) {
    AppointmentRecord rec(s, doctorId, type, insuranceTier, notes);
    rec.setDateTime(dateTime);  // Use user-specified date/time
    appointments.push_back(rec);
    std::cout << "  [OK] Appointment " << rec.getAppointmentId()
              << " scheduled for patient " << name << ".\n";
}

void Patient::addExistingAppointment(const AppointmentRecord& rec) {
    appointments.push_back(rec);
}

// Find specific appointment by ID and update its current status
bool Patient::setAppointmentStatus(const std::string& apptId, const std::string& status) {
    for (auto& rec : appointments) {
        if (rec.getAppointmentId() == apptId) {
            rec.setStatus(status);
            return true;
        }
    }
    return false;
}

bool Patient::cancelAppointment(const std::string& apptId) {
    return setAppointmentStatus(apptId, "Cancelled");
}

// ----- Aggregation cleanup -----
// Clear doctor references from all appointments without deleting the records
void Patient::clearDoctorFromAppointments(const std::string& doctorId) {
    for (auto& rec : appointments) {
        if (rec.getDoctorId() == doctorId) {
            rec.setDoctorId("");  // clear the aggregation reference
            // Note: the appointment data (service, billing, status) is
            // preserved – it is owned by composition, not by the doctor.
        }
    }
}

void Patient::addAllergy(const std::string& allergy) {
    allergies.push_back(allergy);
}

// ----- Financial -----
// Sum up total expenses for all associated appointments
double Patient::calculateTotalMedicalExpenses() const {
    double total = 0.0;
    for (const auto& rec : appointments)
        total += rec.calculateTotalBilling();
    return total;
}

// Calculate total expenses specifically for completed appointments
double Patient::calculateOutstandingBalance() const {
    double total = 0.0;
    for (const auto& rec : appointments)
        if (rec.getAppointmentStatus() == "Completed")
            total += rec.calculateTotalBilling();
    return total;
}

// ----- Getters -----
std::string   Patient::getInsuranceProvider()     const { return insuranceProvider; }
std::string   Patient::getInsurancePolicyNumber() const { return insurancePolicyNumber; }
InsuranceTier Patient::getInsuranceTier()         const { return insuranceTier; }
std::string   Patient::getBloodType()             const { return bloodType; }
const std::vector<AppointmentRecord>& Patient::getAppointments() const { return appointments; }
const std::vector<std::string>& Patient::getAllergies() const { return allergies; }

// ----- Static -----
int Patient::getPatientCount() { return patientCount; }

// Generate a unique patient ID string using an auto-incrementing counter
std::string Patient::generatePatientId() {
    std::ostringstream oss;
    oss << "PAT" << (++nextPatientNumber);
    return oss.str();
}

void Patient::setNextPatientNumber(int n) { nextPatientNumber = n; }

// ----- Appointment history display -----
void Patient::displayAppointmentHistory() const {
    Person::printDivider('=', 55);
    std::cout << "  Appointment History - " << name << " (" << id << ")\n";
    Person::printDivider('=', 55);
    if (appointments.empty()) {
        std::cout << "  No appointments on record.\n";
    } else {
        for (const auto& rec : appointments)
            rec.displayRecord();
    }
    std::cout << std::fixed << std::setprecision(2)
              << "  TOTAL ESTIMATED BILL: RM " << calculateTotalMedicalExpenses() << "\n";
    Person::printDivider('=', 55);
}

```

### src/Doctor.cpp

```cpp
// Doctor.cpp
#include "../include/Doctor.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

// ----- Static members -----
int Doctor::doctorCount      = 0;
int Doctor::nextDoctorNumber = 100;

// ----- Constructors -----
// Initialize default doctor attributes and increment overall counter
Doctor::Doctor()
    : Person(), specializationDepartment("General"), assignedClinicCode(""),
      licenseNumber(""), yearsOfExperience(0), consultationFee(0.0), patientsSeen(0)
{
    ++doctorCount;
}

// Parameterized constructor initializing both Person and specific Doctor attributes
Doctor::Doctor(const std::string& id,
               const std::string& name,
               const std::string& contact,
               const std::string& email,
               const std::string& dob,
               const std::string& gender,
               const std::string& specialization,
               const std::string& licenseNum,
               int yearsExp,
               double consultFee)
    : Person(id, name, contact, email, dob, gender),
      specializationDepartment(specialization), assignedClinicCode(""),
      licenseNumber(licenseNum), yearsOfExperience(yearsExp),
      consultationFee(consultFee), patientsSeen(0)
{
    ++doctorCount;
}

Doctor::~Doctor() {
    --doctorCount;
}

// ----- Pure virtual override -----
// Print detailed doctor information, qualifications, and patient stats
void Doctor::displayRole() const {
    Person::printDivider('-', 55);
    std::cout << "  [DOCTOR RECORD]\n";
    Person::printDivider('-', 55);
    displaySummary();
    std::cout << "  Specialization : " << specializationDepartment << "\n"
              << "  License No.    : " << licenseNumber            << "\n"
              << "  Experience     : " << yearsOfExperience << " years\n"
              << std::fixed << std::setprecision(2)
              << "  Consult. Fee   : RM " << consultationFee       << "\n"
              << "  Assigned Clinic: " << (assignedClinicCode.empty() ? "None" : assignedClinicCode) << "\n";
    if (!qualifications.empty()) {
        std::cout << "  Qualifications : ";
        for (size_t i = 0; i < qualifications.size(); ++i)
            std::cout << qualifications[i] << (i + 1 < qualifications.size() ? ", " : "");
        std::cout << "\n";
    }
    std::cout << "  Patients Seen  : " << patientsSeen << "\n"
              << "  Active Patients: " << assignedPatientIds.size() << "\n";
    Person::printDivider('-', 55);
}

std::string Doctor::getType() const { return "Doctor"; }

// ----- Clinic management -----
// Assign this doctor to a specific clinic by code
void Doctor::assignToClinic(const std::string& clinicCode) {
    assignedClinicCode = clinicCode;
    std::cout << "  [OK] " << name << " assigned to clinic [" << clinicCode << "].\n";
}

void Doctor::addQualification(const std::string& qual) {
    qualifications.push_back(qual);
}

// Add patient ID to assigned list, avoiding duplicate entries
void Doctor::addAssignedPatient(const std::string& patientId) {
    // Prevent duplicate entries – a patient should appear at most once
    if (std::find(assignedPatientIds.begin(), assignedPatientIds.end(), patientId)
        == assignedPatientIds.end()) {
        assignedPatientIds.push_back(patientId);
    }
}

// Remove specific patient ID from the assigned list if it exists
void Doctor::removeAssignedPatient(const std::string& patientId) {
    assignedPatientIds.erase(
        std::remove(assignedPatientIds.begin(), assignedPatientIds.end(), patientId),
        assignedPatientIds.end()
    );
}

void Doctor::incrementPatientsSeen() {
    ++patientsSeen;
}

// ----- Getters -----
std::string Doctor::getSpecialization()     const { return specializationDepartment; }
std::string Doctor::getAssignedClinicCode() const { return assignedClinicCode; }
std::string Doctor::getLicenseNumber()      const { return licenseNumber; }
int         Doctor::getYearsOfExperience()  const { return yearsOfExperience; }
double      Doctor::getConsultationFee()    const { return consultationFee; }
const std::vector<std::string>& Doctor::getQualifications() const { return qualifications; }
const std::vector<std::string>& Doctor::getAssignedPatients() const { return assignedPatientIds; }
int Doctor::getPatientsSeen() const { return patientsSeen; }
void Doctor::setPatientsSeen(int n) { patientsSeen = n; }

// ----- Static -----
int Doctor::getDoctorCount() { return doctorCount; }

// Generate a sequential unique doctor ID
std::string Doctor::generateDoctorId() {
    std::ostringstream oss;
    oss << "DOC" << (++nextDoctorNumber);
    return oss.str();
}

void Doctor::setNextDoctorNumber(int n) { nextDoctorNumber = n; }

```

### src/MedicalService.cpp

```cpp
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

```

### src/AppointmentRecord.cpp

```cpp
// AppointmentRecord.cpp
#include "../include/AppointmentRecord.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

// ----- Static member -----
int AppointmentRecord::idCounter = 1000;

// ----- ID generator -----
// Generate unique appointment ID by incrementing a static counter
std::string AppointmentRecord::generateId() {
    std::ostringstream oss;
    oss << "APT" << (++idCounter);
    return oss.str();
}

// Helper: current date-time string
// Return the current system date and time formatted as a string
static std::string currentDateTime() {
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", std::localtime(&now));
    return std::string(buf);
}

// ----- Constructors -----
// Default constructor configuring initial state for an empty appointment
AppointmentRecord::AppointmentRecord()
    : service(), appointmentStatus("Scheduled"),
      apptType(AppointmentType::REGULAR), insuranceTier(InsuranceTier::NONE)
{
    appointmentId = generateId();
    dateTime      = currentDateTime();
}

// Parameterized constructor populating details and setting scheduled status
AppointmentRecord::AppointmentRecord(const MedicalService& svc,
                                     const std::string& docId,
                                     AppointmentType type,
                                     InsuranceTier   tier,
                                     const std::string& n)
    : service(svc), appointmentStatus("Scheduled"),
      apptType(type), insuranceTier(tier), doctorId(docId), notes(n)
{
    appointmentId = generateId();
    dateTime      = currentDateTime();
}

// ----- Getters -----
std::string     AppointmentRecord::getAppointmentId()     const { return appointmentId; }
std::string     AppointmentRecord::getServiceCode()       const { return service.getServiceCode(); }
std::string     AppointmentRecord::getServiceName()       const { return service.getServiceName(); }
std::string     AppointmentRecord::getAppointmentStatus() const { return appointmentStatus; }
std::string     AppointmentRecord::getDoctorId()          const { return doctorId; }
std::string     AppointmentRecord::getDateTime()          const { return dateTime; }
AppointmentType AppointmentRecord::getAppointmentType()   const { return apptType; }
InsuranceTier   AppointmentRecord::getInsuranceTier()     const { return insuranceTier; }
std::string     AppointmentRecord::getNotes()             const { return notes; }

// ----- Setters -----
void AppointmentRecord::setAppointmentId(const std::string& id) { appointmentId = id; }
void AppointmentRecord::setDateTime(const std::string& dt)      { dateTime = dt; }
void AppointmentRecord::setDoctorId(const std::string& docId)   { doctorId = docId; }
void AppointmentRecord::setStatus(const std::string& status)  { appointmentStatus = status; }
void AppointmentRecord::setInsuranceTier(InsuranceTier tier)  { insuranceTier = tier; }
void AppointmentRecord::setNotes(const std::string& n)        { notes = n; }
void AppointmentRecord::setIdCounter(int n)                    { idCounter = n; }

// ----- Billing calculation -----
// Base fee → apply type modifier → apply insurance discount
// Calculate total cost considering base fee, modifiers, and insurance discounts
double AppointmentRecord::calculateTotalBilling() const {
    double total = service.getBaseFee();

    // Type modifier
    switch (apptType) {
        case AppointmentType::EMERGENCY:  total *= 1.30; break;  // +30% emergency surcharge
        case AppointmentType::FOLLOW_UP:  total *= 0.70; break;  // -30% follow-up discount
        default: break;
    }

    // Insurance coverage
    switch (insuranceTier) {
        case InsuranceTier::BASIC:    total *= 0.85; break;  // 15% coverage
        case InsuranceTier::STANDARD: total *= 0.70; break;  // 30% coverage
        case InsuranceTier::PREMIUM:  total *= 0.50; break;  // 50% coverage
        default: break;
    }

    // Do not charge for cancelled or fully paid appointments
    if (appointmentStatus == "Cancelled" || appointmentStatus == "Paid") return 0.0;

    return total;
}

// ----- String helpers -----
// Convert insurance tier enum value to human-readable string
std::string AppointmentRecord::tierToString(InsuranceTier t) {
    switch (t) {
        case InsuranceTier::NONE:     return "None";
        case InsuranceTier::BASIC:    return "Basic (15%)";
        case InsuranceTier::STANDARD: return "Standard (30%)";
        case InsuranceTier::PREMIUM:  return "Premium (50%)";
        default: return "Unknown";
    }
}

// Convert appointment type enum value to human-readable string
std::string AppointmentRecord::typeToString(AppointmentType t) {
    switch (t) {
        case AppointmentType::REGULAR:    return "Regular";
        case AppointmentType::EMERGENCY:  return "Emergency";
        case AppointmentType::FOLLOW_UP:  return "Follow-Up";
        default: return "Unknown";
    }
}

// ----- Display -----
// Print formatted box containing all appointment details
void AppointmentRecord::displayRecord() const {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  +---------------------------------------------+\n";
    std::cout << "  | Appt ID  : " << std::left << std::setw(33) << appointmentId << "|\n";
    std::cout << "  | Service  : " << std::setw(33) << service.getServiceName()   << "|\n";
    std::cout << "  | Type     : " << std::setw(33) << typeToString(apptType)     << "|\n";
    std::cout << "  | Status   : " << std::setw(33) << appointmentStatus          << "|\n";
    std::cout << "  | Doctor ID: " << std::setw(33) << doctorId                  << "|\n";
    std::cout << "  | Insurance: " << std::setw(33) << tierToString(insuranceTier)<< "|\n";
    std::cout << "  | Date/Time: " << std::setw(33) << dateTime                  << "|\n";
    std::cout << "  | Total Fee: RM " << std::setw(30) << calculateTotalBilling() << "|\n";
    if (!notes.empty())
    std::cout << "  | Notes    : " << std::setw(33) << notes                     << "|\n";
    std::cout << "  +---------------------------------------------+\n";
}

```

### src/HospitalSystem.cpp

```cpp
// HospitalSystem.cpp
#include "../include/HospitalSystem.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

// Constructor / Destructor
HospitalSystem::HospitalSystem(const std::string &name) : hospitalName(name) {}

HospitalSystem::~HospitalSystem() {
  // Memory management: delete all dynamically allocated Person objects
  users.clear();
}

// Internal helpers
bool HospitalSystem::idExists(const std::string &id) const {
  // Check if a user with the given ID already exists
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    if (p->getId() == id)
      return true;
  }
  return false;
}

Patient *HospitalSystem::findPatient(const std::string &id) {
  // Search for a patient by ID and return a pointer if found
  for (const auto &ptr : users) {
    Person *p = ptr.get();
    if (p->getId() == id && p->getType() == "Patient")
      // Reference: dynamic_cast for safe downcasting in polymorphic hierarchies
      // https://en.cppreference.com/w/cpp/language/dynamic_cast
      return dynamic_cast<Patient *>(p);
  }
  return nullptr;
}

Doctor *HospitalSystem::findDoctor(const std::string &id) {
  // Search for a doctor by ID and return a pointer if found
  for (const auto &ptr : users) {
    Person *p = ptr.get();
    if (p->getId() == id && p->getType() == "Doctor")
      return dynamic_cast<Doctor *>(p);
  }
  return nullptr;
}

// ----- User management -----
void HospitalSystem::addPatient(std::unique_ptr<Patient> p) {
  if (!p)
    throw HospitalException("Null patient pointer.");
  if (idExists(p->getId()))
    throw DuplicateIdException(p->getId());
  std::cout << "  [OK] Patient '" << p->getName()
            << "' registered. ID: " << p->getId() << "\n";
  users.push_back(std::move(p));
}

void HospitalSystem::addDoctor(std::unique_ptr<Doctor> d) {
  if (!d)
    throw HospitalException("Null doctor pointer.");
  if (idExists(d->getId()))
    throw DuplicateIdException(d->getId());
  std::cout << "  [OK] Doctor '" << d->getName()
            << "' registered. ID: " << d->getId() << "\n";
  users.push_back(std::move(d));
}

void HospitalSystem::removeUser(const std::string &id) {
  auto it = std::find_if(
      users.begin(), users.end(),
      [&](const std::unique_ptr<Person> &p) { return p->getId() == id; });
  if (it == users.end())
    throw NotFoundException(id);

  Person *target = it->get();

  // - Aggregation cleanup
  // After destroying one entity, clean up any weak (aggregation)
  // Composition relationships (embedded objects) are destroyed
  // automatically by the destructor, thus no cleanup needed for those.

  if (target->getType() == "Patient") {
    // A Patient is being removed.
    // Doctors hold the patient's ID in their assignedPatientIds list
    // (aggregation). In here we clear it so appointments don't reference
    // a doctor that no longer exists.
    for (const auto &ptr : users) {
      Person *p = ptr.get();
      if (p->getType() == "Doctor") {
        Doctor *doc = dynamic_cast<Doctor *>(p);
        doc->removeAssignedPatient(id);
      }
    }
    // The Patient's own AppointmentRecords (composition) and their
    // embedded MedicalService objects (nested composition) will be
    // destroyed automatically when `delete target` runs below.

  } else if (target->getType() == "Doctor") {
    // A Doctor is being removed.
    // Patients' AppointmentRecords hold the doctor's ID as a string
    // (aggregation). In here we clear it so appointments don't reference
    // a doctor that no longer exists.
    // The appointment ITSELF is preserved, thus it is owned by the
    // Patient (composition), not by the Doctor.
    for (const auto &ptr : users) {
      Person *p = ptr.get();
      if (p->getType() == "Patient") {
        Patient *pat = dynamic_cast<Patient *>(p);
        pat->clearDoctorFromAppointments(id);
      }
    }
  }

  // Destroy the object (composition: owned data is freed)
  users.erase(it);
  std::cout << "  [OK] User " << id << " removed from system.\n";
}

// Service management
void HospitalSystem::addMedicalService(const MedicalService &svc) {
  if (servicesMap.count(svc.getServiceCode()))
    throw DuplicateIdException(svc.getServiceCode());
  servicesMap[svc.getServiceCode()] = svc;
  std::cout << "  [OK] Medical service '" << svc.getServiceName()
            << "' added. Code: " << svc.getServiceCode() << "\n";
}

std::string
HospitalSystem::checkServiceRemovable(const std::string &serviceCode) const {
  if (!servicesMap.count(serviceCode))
    return "";

  // Verify if any active patient appointments are using this service
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    if (p->getType() == "Patient") {
      const Patient *pat = dynamic_cast<const Patient *>(p);
      for (const auto &rec : pat->getAppointments()) {
        if (rec.getServiceCode() == serviceCode &&
            rec.getAppointmentStatus() != "Cancelled") {
          return rec.getAppointmentId(); // Returns the blocking appointment
        }
      }
    }
  }
  return ""; // Empty string means safe to remove
}

void HospitalSystem::removeMedicalService(const std::string &serviceCode) {
  if (!servicesMap.count(serviceCode))
    throw NotFoundException(serviceCode);

  // Safety check: refuse removal if any active appointment references this
  // service.
  std::string blocker = checkServiceRemovable(serviceCode);
  if (!blocker.empty()) {
    throw HospitalException("Cannot remove service '" + serviceCode +
                            "' - it is referenced by active appointment " +
                            blocker + ".");
  }

  std::string name = servicesMap[serviceCode].getServiceName();
  servicesMap.erase(serviceCode);
  std::cout << "  [OK] Medical service '" << name << "' (" << serviceCode
            << ") removed.\n";
}

void HospitalSystem::listMedicalServices() const {
  Person::printDivider('=', 55);
  Person::printCentred("MEDICAL SERVICES CATALOGUE", 55);
  Person::printDivider('=', 55);
  if (servicesMap.empty()) {
    std::cout << "  No services registered.\n";
  } else {
    int i = 1;
    for (const auto &kv : servicesMap) {
      std::cout << "  [" << i++ << "]\n";
      kv.second.displayServiceDetails();
    }
  }
  Person::printDivider('=', 55);
}

// Appointment operations
void HospitalSystem::scheduleAppointment(const std::string &patientId,
                                         const std::string &serviceCode,
                                         const std::string &doctorId,
                                         const std::string &dateTime,
                                         AppointmentType type,
                                         const std::string &notes) {
  Patient *patient = findPatient(patientId);
  if (!patient)
    throw NotFoundException(patientId);

  auto it = servicesMap.find(serviceCode);
  if (it == servicesMap.end())
    throw NotFoundException(serviceCode);

  Doctor *doc = findDoctor(doctorId);
  if (!doc)
    throw NotFoundException(doctorId);

  if (hasDoctorConflict(doctorId, dateTime)) {
    throw HospitalException("Scheduling conflict: " + doc->getName() +
                            " already has an appointment at " + dateTime + ".");
  }

  patient->scheduleAppointment(it->second, doctorId, dateTime, type, notes);
  doc->addAssignedPatient(patientId);
}

void HospitalSystem::setAppointmentStatus(const std::string &patientId,
                                          const std::string &apptId,
                                          const std::string &status) {
  Patient *patient = findPatient(patientId);
  if (!patient)
    throw NotFoundException(patientId);

  // Find the appointment's doctor ID before changing status
  std::string doctorId;
  std::string oldStatus;
  // Iterate through patient appointments to find current doctor and status
  for (const auto &rec : patient->getAppointments()) {
    if (rec.getAppointmentId() == apptId) {
      doctorId = rec.getDoctorId();
      oldStatus = rec.getAppointmentStatus();
      break;
    }
  }

  // --- Transition Validation ---
  if (oldStatus == status)
    return;

  if (oldStatus == "Paid") {
    throw HospitalException("Cannot change status of a Paid appointment.");
  }
  if (oldStatus == "Cancelled") {
    throw HospitalException("Cannot change status of a Cancelled appointment.");
  }
  if (oldStatus == "Completed" && status != "Paid") {
    throw HospitalException(
        "A Completed appointment can only be marked as Paid.");
  }
  if (oldStatus == "Scheduled" && status == "Paid") {
    throw HospitalException(
        "Appointment must be Completed before it can be Paid.");
  }

  if (!patient->setAppointmentStatus(apptId, status))
    throw NotFoundException(apptId);

  std::cout << "  [OK] Appointment " << apptId << " status set to '" << status
            << "'.\n";

  // --- Data Cascading Logic ---

  Doctor *doc = (!doctorId.empty()) ? findDoctor(doctorId) : nullptr;

  if (status == "Completed" && doc) {
    // Increment the doctor's completed-patients counter
    doc->incrementPatientsSeen();
    // Remove patient from the doctor's active assignment list
    // (only if no OTHER active appointments remain with this doctor)
    bool hasOtherActive = false;
    for (const auto &rec : patient->getAppointments()) {
      if (rec.getDoctorId() == doctorId && rec.getAppointmentId() != apptId &&
          rec.getAppointmentStatus() == "Scheduled") {
        hasOtherActive = true;
        break;
      }
    }
    if (!hasOtherActive) {
      doc->removeAssignedPatient(patientId);
    }
  } else if (status == "Cancelled" && doc) {
    // Remove patient from the doctor's active assignment list
    // (only if no OTHER active appointments remain with this doctor)
    bool hasOtherActive = false;
    for (const auto &rec : patient->getAppointments()) {
      if (rec.getDoctorId() == doctorId && rec.getAppointmentId() != apptId &&
          rec.getAppointmentStatus() == "Scheduled") {
        hasOtherActive = true;
        break;
      }
    }
    if (!hasOtherActive) {
      doc->removeAssignedPatient(patientId);
    }
  }
}

void HospitalSystem::assignDoctorToClinic(const std::string &doctorId,
                                          const std::string &clinicCode) {
  Doctor *doc = findDoctor(doctorId);
  if (!doc)
    throw NotFoundException(doctorId);
  doc->assignToClinic(clinicCode);
}

// Polymorphic display (Runtime Polymorphism)
void HospitalSystem::viewAllUsers() const {
  Person::printDivider('=', 55);
  Person::printCentred("ALL REGISTERED USERS", 55);
  std::cout << "  Total: " << users.size() << " user(s)\n";
  Person::printDivider('=', 55);
  // Iterate through all users and call their respective display role functions
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    p->displayRole();
  } // runtime polymorphism: correct override called at
    // runtime
}

void HospitalSystem::viewAllPatients() const {
  Person::printDivider('=', 55);
  Person::printCentred("ALL PATIENTS", 55);
  Person::printDivider('=', 55);
  int count = 0;
  // Iterate through all users to find and display those who are patients
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    if (p->getType() == "Patient") {
      p->displayRole();
      ++count;
    }
  }
  if (count == 0)
    std::cout << "  No patients registered.\n";
}

void HospitalSystem::viewAllDoctors() const {
  Person::printDivider('=', 55);
  Person::printCentred("ALL DOCTORS", 55);
  Person::printDivider('=', 55);
  int count = 0;
  // Iterate through all users to find and display those who are doctors
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    if (p->getType() == "Doctor") {
      p->displayRole();
      ++count;
    }
  }
  if (count == 0)
    std::cout << "  No doctors registered.\n";
}

// Billing report
void HospitalSystem::generateBillingReport(const std::string &patientId) const {
  // const version: use const_cast to call findPatient via loop
  // Search for the specific patient and display their billing history
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    if (p->getId() == patientId && p->getType() == "Patient") {
      const Patient *pat = dynamic_cast<const Patient *>(p);
      pat->displayAppointmentHistory();
      return;
    }
  }
  throw NotFoundException(patientId);
}

// System summary
void HospitalSystem::generateSystemSummary() const {
  Person::printDivider('=', 55);
  Person::printCentred(hospitalName, 55);
  Person::printCentred("SYSTEM SUMMARY", 55);
  Person::printDivider('=', 55);
  std::cout << "  Total Users    : " << users.size() << "\n"
            << "  Total Patients : " << Patient::getPatientCount() << "\n"
            << "  Total Doctors  : " << Doctor::getDoctorCount() << "\n"
            << "  Total Services : " << servicesMap.size() << "\n";
  Person::printDivider('=', 55);
}

// File persistence
// Reference: File I/O using std::ofstream and std::ifstream
// https://en.cppreference.com/w/cpp/header/fstream
void HospitalSystem::saveToFile(const std::string &filename) const {
  std::ofstream ofs(filename);
  if (!ofs.is_open())
    throw HospitalException("Cannot open file for writing: " + filename);

  // Write services
  ofs << "[SERVICES]\n";
  // Iterate over all medical services to save their details
  for (const auto &kv : servicesMap) {
    const MedicalService &s = kv.second;
    ofs << s.getServiceCode() << "|" << s.getServiceName() << "|"
        << s.getServiceCategory() << "|" << s.getBaseFee() << "|"
        << s.getRequiresReferral() << "\n";
  }

  // Write users
  ofs << "[USERS]\n";
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    ofs << p->getType() << "|" << p->getId() << "|" << p->getName() << "|"
        << p->getContactNumber() << "|" << p->getEmail() << "|"
        << p->getDateOfBirth() << "|" << p->getGender();

    if (p->getType() == "Patient") {
      const Patient *pat = dynamic_cast<const Patient *>(p);
      ofs << "|" << pat->getInsuranceProvider() << "|"
          << pat->getInsurancePolicyNumber() << "|"
          << static_cast<int>(pat->getInsuranceTier()) << "|"
          << pat->getBloodType();

      ofs << "|";
      const auto &allergies = pat->getAllergies();
      for (size_t i = 0; i < allergies.size(); ++i) {
        ofs << allergies[i] << (i + 1 < allergies.size() ? "," : "");
      }
    } else if (p->getType() == "Doctor") {
      const Doctor *doc = dynamic_cast<const Doctor *>(p);
      ofs << "|" << doc->getSpecialization() << "|" << doc->getLicenseNumber()
          << "|" << doc->getYearsOfExperience() << "|"
          << doc->getConsultationFee() << "|" << doc->getAssignedClinicCode();

      ofs << "|";
      const auto &quals = doc->getQualifications();
      for (size_t i = 0; i < quals.size(); ++i) {
        ofs << quals[i] << (i + 1 < quals.size() ? "," : "");
      }

      ofs << "|";
      const auto &pats = doc->getAssignedPatients();
      for (size_t i = 0; i < pats.size(); ++i) {
        ofs << pats[i] << (i + 1 < pats.size() ? "," : "");
      }

      ofs << "|" << doc->getPatientsSeen();
    }
    ofs << "\n";
  }

  // Write appointments
  ofs << "[APPOINTMENTS]\n";
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    if (p->getType() == "Patient") {
      const Patient *pat = dynamic_cast<const Patient *>(p);
      for (const auto &rec : pat->getAppointments()) {
        ofs << pat->getId() << "|" << rec.getAppointmentId() << "|"
            << rec.getServiceCode() << "|" << rec.getDoctorId() << "|"
            << static_cast<int>(rec.getAppointmentType()) << "|"
            << static_cast<int>(rec.getInsuranceTier()) << "|"
            << rec.getAppointmentStatus() << "|" << rec.getDateTime() << "|"
            << rec.getNotes() << "\n";
      }
    }
  }

  ofs.close();
  std::cout << "  [OK] Data saved to '" << filename << "'.\n";
}

// Reference: File I/O using std::ofstream and std::ifstream
// https://en.cppreference.com/w/cpp/header/fstream
void HospitalSystem::loadFromFile(const std::string &filename) {
  std::ifstream ifs(filename);
  if (!ifs.is_open())
    throw HospitalException("Cannot open file for reading: " + filename);

  // Clear current state
  users.clear();
  servicesMap.clear();

  std::string line;
  std::string section = "";

  // Helper to split string by delimiter
  // Reference: Splitting strings using std::istringstream and std::getline
  // https://en.cppreference.com/w/cpp/io/basic_istringstream
  // https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
  auto split = [](const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
      tokens.push_back(token);
    }
    if (!s.empty() && s.back() == delimiter)
      tokens.push_back("");
    return tokens;
  };

  // Read file line by line and parse data into corresponding objects
  while (std::getline(ifs, line)) {
    if (line.empty())
      continue;
    if (line[0] == '[') {
      section = line;
      continue;
    }

    auto fields = split(line, '|');

    if (section == "[SERVICES]" && fields.size() >= 5) {
      MedicalService svc(fields[0], fields[1], fields[2], std::stod(fields[3]),
                         fields[4] == "1");
      servicesMap[fields[0]] = svc;
    } else if (section == "[USERS]" && fields.size() >= 7) {
      std::string type = fields[0];
      std::string id = fields[1], name = fields[2], contact = fields[3],
                  email = fields[4], dob = fields[5], gender = fields[6];

      if (type == "Patient" && fields.size() >= 12) {
        InsuranceTier tier = static_cast<InsuranceTier>(std::stoi(fields[9]));
        auto pat =
            std::make_unique<Patient>(id, name, contact, email, dob, gender,
                                      fields[7], fields[8], tier, fields[10]);
        auto allergies = split(fields[11], ',');
        for (const auto &a : allergies) {
          if (!a.empty())
            pat->addAllergy(a);
        }
        users.push_back(std::move(pat));
      } else if (type == "Doctor" && fields.size() >= 14) {
        auto doc = std::make_unique<Doctor>(
            id, name, contact, email, dob, gender, fields[7], fields[8],
            std::stoi(fields[9]), std::stod(fields[10]));
        if (!fields[11].empty())
          doc->assignToClinic(fields[11]);

        auto quals = split(fields[12], ',');
        for (const auto &q : quals) {
          if (!q.empty())
            doc->addQualification(q);
        }

        auto pats = split(fields[13], ',');
        for (const auto &p_id : pats) {
          if (!p_id.empty())
            doc->addAssignedPatient(p_id);
        }
        // Load patientsSeen counter (field 14, backward compatible)
        if (fields.size() >= 15 && !fields[14].empty()) {
          doc->setPatientsSeen(std::stoi(fields[14]));
        }
        users.push_back(std::move(doc));
      }
    } else if (section == "[APPOINTMENTS]" && fields.size() >= 9) {
      std::string patId = fields[0];
      std::string apptId = fields[1];
      std::string sCode = fields[2];
      std::string docId = fields[3];
      AppointmentType aType =
          static_cast<AppointmentType>(std::stoi(fields[4]));
      InsuranceTier iTier = static_cast<InsuranceTier>(std::stoi(fields[5]));
      std::string status = fields[6];
      std::string dt = fields[7];
      std::string notes = fields[8];

      Patient *pat = findPatient(patId);
      if (pat && servicesMap.count(sCode)) {
        AppointmentRecord rec(servicesMap[sCode], docId, aType, iTier, notes);
        rec.setAppointmentId(apptId);
        rec.setDateTime(dt);
        rec.setStatus(status);
        pat->addExistingAppointment(rec);
      }
    }
  }
  ifs.close();

  // Sync static ID counters to avoid collisions
  int maxPatNum = 1000, maxDocNum = 100, maxApptNum = 1000;
  // Loop through all users to find maximum IDs and synchronize counters
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    std::string uid = p->getId();
    if (uid.size() > 3 && uid.substr(0, 3) == "PAT") {
      try {
        int num = std::stoi(uid.substr(3));
        if (num > maxPatNum)
          maxPatNum = num;
      } catch (...) {
      }
    } else if (uid.size() > 3 && uid.substr(0, 3) == "DOC") {
      try {
        int num = std::stoi(uid.substr(3));
        if (num > maxDocNum)
          maxDocNum = num;
      } catch (...) {
      }
    }
    if (p->getType() == "Patient") {
      const Patient *pat = dynamic_cast<const Patient *>(p);
      for (const auto &rec : pat->getAppointments()) {
        std::string aid = rec.getAppointmentId();
        if (aid.size() > 3 && aid.substr(0, 3) == "APT") {
          try {
            int num = std::stoi(aid.substr(3));
            if (num > maxApptNum)
              maxApptNum = num;
          } catch (...) {
          }
        }
      }
    }
  }
  Patient::setNextPatientNumber(maxPatNum);
  Doctor::setNextDoctorNumber(maxDocNum);
  AppointmentRecord::setIdCounter(maxApptNum);

  std::cout << "  [OK] Data loaded from '" << filename << "'.\n";
}

// Getters
const std::vector<std::unique_ptr<Person>> &HospitalSystem::getUsers() const {
  return users;
}
const std::unordered_map<std::string, MedicalService> &
HospitalSystem::getServices() const {
  return servicesMap;
}
int HospitalSystem::getUserCount() const {
  return static_cast<int>(users.size());
}
int HospitalSystem::getServiceCount() const {
  return static_cast<int>(servicesMap.size());
}

// Doctor time-conflict check
bool HospitalSystem::hasDoctorConflict(const std::string &doctorId,
                                       const std::string &dateTime) const {
  // Check if the doctor already has an active appointment at the requested time
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    if (p->getType() == "Patient") {
      const Patient *pat = dynamic_cast<const Patient *>(p);
      for (const auto &rec : pat->getAppointments()) {
        if (rec.getDoctorId() == doctorId && rec.getDateTime() == dateTime &&
            rec.getAppointmentStatus() != "Cancelled" &&
            rec.getAppointmentStatus() != "Completed" &&
            rec.getAppointmentStatus() != "Paid") {
          return true;
        }
      }
    }
  }
  return false;
}

// Check if a doctor can be removed (no active appointments)
std::vector<DoctorAppointmentInfo>
HospitalSystem::checkDoctorRemovable(const std::string &doctorId) const {
  std::vector<DoctorAppointmentInfo> blockers;
  // Collect all active appointments that block this doctor from being removed
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    if (p->getType() == "Patient") {
      const Patient *pat = dynamic_cast<const Patient *>(p);
      for (const auto &rec : pat->getAppointments()) {
        if (rec.getDoctorId() == doctorId &&
            rec.getAppointmentStatus() != "Cancelled" &&
            rec.getAppointmentStatus() != "Completed" &&
            rec.getAppointmentStatus() != "Paid") {
          DoctorAppointmentInfo info;
          info.patientName = pat->getName();
          info.patientId = pat->getId();
          info.patientContact = pat->getContactNumber();
          info.appointmentId = rec.getAppointmentId();
          blockers.push_back(info);
        }
      }
    }
  }
  return blockers;
}

// Reassign a specific appointment to a different doctor
void HospitalSystem::reassignAppointmentDoctor(const std::string &patientId,
                                               const std::string &apptId,
                                               const std::string &newDoctorId) {
  Patient *pat = findPatient(patientId);
  if (!pat)
    throw NotFoundException(patientId);

  Doctor *newDoc = findDoctor(newDoctorId);
  if (!newDoc)
    throw NotFoundException(newDoctorId);

  // Find the appointment and get the old doctor ID before changing it
  bool found = false;
  std::string oldDoctorId;
  // We need non-const access to appointments, so use setAppointmentStatus-style
  // loop But we need to change doctorID to use clearDoctorFromAppointments
  // selectively iterate via the Patient's methods
  // Find the target appointment and update its assigned doctor
  for (auto &rec :
       const_cast<std::vector<AppointmentRecord> &>(pat->getAppointments())) {
    if (rec.getAppointmentId() == apptId) {
      oldDoctorId = rec.getDoctorId();
      rec.setDoctorId(newDoctorId);
      found = true;
      break;
    }
  }
  if (!found)
    throw NotFoundException(apptId);

  // Update doctor patient lists
  newDoc->addAssignedPatient(patientId);

  // Remove patient from old doctor if no other active appointments remain
  if (!oldDoctorId.empty()) {
    Doctor *oldDoc = findDoctor(oldDoctorId);
    if (oldDoc) {
      bool hasOtherAppts = false;
      // Check if the previous doctor still has other active appointments for this patient
      for (const auto &rec : pat->getAppointments()) {
        if (rec.getDoctorId() == oldDoctorId &&
            rec.getAppointmentId() != apptId &&
            rec.getAppointmentStatus() != "Cancelled" &&
            rec.getAppointmentStatus() != "Paid") {
          hasOtherAppts = true;
          break;
        }
      }
      if (!hasOtherAppts) {
        oldDoc->removeAssignedPatient(patientId);
      }
    }
  }

  std::cout << "  [OK] Appointment " << apptId << " reassigned to "
            << newDoc->getName() << ".\n";
}

```

### src/main.cpp

```cpp
// main.cpp
// Hospital Patient & Appointment Management System
// CST209 Final Project – Academic Session 2026/04
// ============================================================
// Key OOP concepts demonstrated:
//   - Abstract Base Class (Person) with pure virtual function
//   - Inheritance (Patient, Doctor from Person)
//   - Composition (AppointmentRecord has-a MedicalService)
//   - Runtime Polymorphism (vector<Person*>, displayRole())
//   - Compile-time Polymorphism (overloaded constructors, operator==)
//   - STL: vector, map
//   - File I/O (save/load system state)
//   - Static members (ID generation, counts)
//   - Exception handling (custom exception hierarchy)
// ============================================================

#include "../include/HospitalSystem.h"
#include "../include/Patient.h"
#include "../include/Doctor.h"
#include "../include/MedicalService.h"
#include <iostream>
#include <limits>
#include <string>
#include <stdexcept>
#include <cctype>
#include "../include/InputValidator.h"

// ─────────────────────────── Helpers ───────────────────────────

static int getIntInput(const std::string& prompt, int lo = 0, int hi = 9999) {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        // Reject empty input
        if (line.empty()) {
            std::cout << "  [!] Invalid input. Please enter a whole number between "
                      << lo << " and " << hi << ".\n";
            continue;
        }
        // Check every character: only digits allowed (with optional leading minus)
        bool valid = true;
        size_t start = 0;
        if (line[0] == '-') start = 1;
        if (start >= line.size()) valid = false;
        // Iterate through all characters to verify they are digits
        for (size_t i = start; i < line.size() && valid; ++i) {
            if (!std::isdigit(static_cast<unsigned char>(line[i]))) {
                valid = false;
            }
        }
        if (!valid) {
            std::cout << "  [!] Invalid input. Please enter a whole number between "
                      << lo << " and " << hi << ".\n";
            continue;
        }
        try {
            int value = std::stoi(line);
            if (value >= lo && value <= hi) {
                return value;
            }
        } catch (...) {
            // stoi overflow or other error
        }
        std::cout << "  [!] Invalid input. Please enter a whole number between "
                  << lo << " and " << hi << ".\n";
    }
}

static double getDoubleInput(const std::string& prompt) {
    return Input::getValidInput<double>(
        prompt,
        [](const double& v) { return v >= 0; },
        "  [!] Invalid input. Please enter a positive number.\n"
    );
}

static std::string getStringInput(const std::string& prompt) {
    return Input::getNonEmptyString(prompt);
}

static std::string getDateInput(const std::string& prompt) {
    return Input::getValidInput<std::string>(
        prompt,
        [](const std::string& s) {
            // Basic length and slash check for DD/MM/YYYY
            if (s.length() != 10) return false;
            if (s[2] != '/' || s[5] != '/') return false;
            for (int i : {0,1, 3,4, 6,7,8,9}) {
                if (!isdigit(s[i])) return false;
            }
            return true;
        },
        "  [!] Invalid date format. Please use DD/MM/YYYY.\n"
    );
}

static std::string getGenderInput(const std::string& prompt) {
    return Input::getValidInput<std::string>(
        prompt,
        [](const std::string& s) {
            return s == "M" || s == "F" || s == "m" || s == "f";
        },
        "  [!] Invalid gender. Please enter M or F.\n"
    );
}

static InsuranceTier selectInsuranceTier() {
    std::cout << "  Insurance Tier:\n"
              << "    0 - None\n"
              << "    1 - Basic    (15% coverage)\n"
              << "    2 - Standard (30% coverage)\n"
              << "    3 - Premium  (50% coverage)\n";
    int ch = getIntInput("  Select tier [0-3]: ", 0, 3);
    return static_cast<InsuranceTier>(ch);
}

static AppointmentType selectAppointmentType() {
    std::cout << "  Appointment Type:\n"
              << "    1 - Regular\n"
              << "    2 - Emergency (+30% surcharge)\n"
              << "    3 - Follow-Up (-30% discount)\n";
    int ch = getIntInput("  Select type [1-3]: ", 1, 3);
    // Map user choice to the corresponding AppointmentType enum
    switch (ch) {
        case 2: return AppointmentType::EMERGENCY;
        case 3: return AppointmentType::FOLLOW_UP;
        default: return AppointmentType::REGULAR;
    }
}

// ─────────────────────────── Menu ───────────────────────────

static void printMainMenu() {
    Person::printDivider('=', 55);
    Person::printCentred("HOSPITAL MANAGEMENT SYSTEM", 55);
    Person::printCentred("XMUM General Hospital", 55);
    Person::printDivider('=', 55);
    
    std::cout << "  -- Data Entry (Registration) --\n"
              << "  1.  Add New Patient\n"
              << "  2.  Add New Doctor\n"
              << "  3.  Add New Medical Service\n";
    Person::printDivider('-', 55);
    
    std::cout << "  -- Appointment & Clinic Management --\n"
              << "  4.  Schedule Appointment for Patient\n"
              << "  5.  Set Appointment Status\n"
              << "  6.  View Upcoming Appointments\n"
              << "  7.  Assign Doctor to Clinic\n";
    Person::printDivider('-', 55);
    
    std::cout << "  -- Record Access & Reports --\n"
              << "  8.  View All Users\n"
              << "  9.  View All Patients\n"
              << "  10. View All Doctors\n"
              << "  11. View Medical Services Catalogue\n"
              << "  12. Generate Patient Billing Report\n"
              << "  13. System Summary\n";
    Person::printDivider('-', 55);
    
    std::cout << "  -- System & Maintenance --\n"
              << "  14. Save Data to File\n"
              << "  15. Load Data from File\n"
              << "  16. Remove User (Patient/Doctor)\n"
              << "  17. Remove Medical Service\n"
              << "  18. Help Guide (How to Use)\n"
              << "  0.  Exit System\n";
    Person::printDivider('-', 55);
}

static void displayHelpGuide() {
    Person::printDivider('=', 57);
    Person::printCentred("CLINIC SYSTEM: HOW TO USE THE APP", 57);
    Person::printDivider('=', 57);
    std::cout
        << "Welcome to the Help Guide! Follow the arrows to understand\n"
        << "the standard workflow of the system.\n\n"
        << "  [1] REGISTER ENTITIES\n"
        << "   |  Create your baseline data first.\n"
        << "   |--> Add Doctors (Creates Doctor ID)\n"
        << "   |--> Add Patients (Creates Patient ID)\n"
        << "   |\n"
        << "   V\n"
        << "  [2] ASSIGNMENTS (Choice 7)\n"
        << "   |  Link your doctors to their workplaces.\n"
        << "   |--> Assign Doctor to Clinic (Requires Doctor ID)\n"
        << "   |\n"
        << "   V\n"
        << "  [3] BOOKING (Choice 4)\n"
        << "   |  Schedule a visit.\n"
        << "   |--> Create Appointment (Requires Doctor ID,\n"
        << "   |    Patient ID, Service Code)\n"
        << "   |\n"
        << "   V\n"
        << "  [4] VIEWING (Choice 6 - Upcoming Appointments)\n"
        << "   |  Check the schedule.\n"
        << "   |--> View Summary List (Shows brief details)\n"
        << "   |      |--> Enter Appointment Number\n"
        << "   |      |    --> View Full Details\n"
        << "   |      |--> Exit --> Back to Summary List\n"
        << "   |\n"
        << "   V\n"
        << "  [5] RESOLUTION (Choice 5 - Set Status)\n"
        << "   |  Close the loop.\n"
        << "   |--> Set Appointment Status\n"
        << "        |--> 'Completed' (Auto-updates Doctor/\n"
        << "        |    Patient records)\n"
        << "        |--> 'Cancelled' (Frees up the schedule)\n\n"
        << "Tip: Whenever you see a prompt for an ID or Code,\n"
        << "use the dropdown selection lists provided!\n";
    Person::printDivider('=', 57);
}

// ─────────────────────────── Menu Handlers ───────────────────────────

static void handleAddPatient(HospitalSystem& hs) {
    Person::printDivider('-', 55);
    Person::printCentred("ADD NEW PATIENT", 55);
    Person::printDivider('-', 55);

    std::string id   = Patient::generatePatientId();
    std::cout << "  Auto-generated Patient ID: " << id << "\n";

    std::string name    = getStringInput("  Full Name       : ");
    std::string contact = getStringInput("  Contact Number  : ");
    std::string email   = getStringInput("  Email           : ");
    std::string dob     = getDateInput("  Date of Birth (DD/MM/YYYY): ");
    std::string gender  = getGenderInput("  Gender (M/F)    : ");
    std::string ins     = getStringInput("  Insurance Co.   : ");
    std::string policy  = getStringInput("  Policy Number   : ");
    InsuranceTier tier  = selectInsuranceTier();
    std::string blood   = getStringInput("  Blood Type      : ");
    std::string allergies = getStringInput("  Allergies (comma separated) : ");

    auto p = std::make_unique<Patient>(id, name, contact, email, dob, gender,
                                       ins, policy, tier, blood);
    
    // Add allergies if any
    if (!allergies.empty()) {
        std::stringstream ss(allergies);
        std::string item;
        while (std::getline(ss, item, ',')) {
            p->addAllergy(item);
        }
    }

    hs.addPatient(std::move(p));
}

static void handleAddDoctor(HospitalSystem& hs) {
    Person::printDivider('-', 55);
    Person::printCentred("ADD NEW DOCTOR", 55);
    Person::printDivider('-', 55);

    std::string id = Doctor::generateDoctorId();
    std::cout << "  Auto-generated Doctor ID: " << id << "\n";

    std::string name    = getStringInput("  Full Name          : ");
    std::string contact = getStringInput("  Contact Number     : ");
    std::string email   = getStringInput("  Email              : ");
    std::string dob     = getDateInput("  Date of Birth (DD/MM/YYYY): ");
    std::string gender  = getGenderInput("  Gender (M/F)       : ");
    std::string spec    = getStringInput("  Specialization     : ");
    std::string license = getStringInput("  License Number     : ");
    int yrs             = getIntInput("  Years of Experience: ", 0, 60);
    double fee          = getDoubleInput("  Consultation Fee (RM): ");
    std::string quals   = getStringInput("  Qualifications (comma separated) : ");

    auto d = std::make_unique<Doctor>(id, name, contact, email, dob, gender,
                                      spec, license, yrs, fee);
    
    if (!quals.empty()) {
        std::stringstream ss(quals);
        std::string item;
        while (std::getline(ss, item, ',')) {
            d->addQualification(item);
        }
    }

    hs.addDoctor(std::move(d));
}

static void handleAddService(HospitalSystem& hs) {
    Person::printDivider('-', 55);
    Person::printCentred("ADD NEW MEDICAL SERVICE", 55);
    Person::printDivider('-', 55);

    std::string code = getStringInput("  Service Code    : ");
    std::string name = getStringInput("  Service Name    : ");
    std::string cat  = getStringInput("  Category        : ");
    double fee       = getDoubleInput("  Base Fee (RM)   : ");

    int ref = getIntInput("  Referral Required? (1=Yes, 0=No): ", 0, 1);

    // Uses full 5-parameter constructor
    MedicalService svc(code, name, cat, fee, ref == 1);
    hs.addMedicalService(svc);
}

static std::string selectUser(const HospitalSystem& hs, const std::string& typeFilter = "") {
    const auto& users = hs.getUsers();
    std::vector<const Person*> filtered;
    // Filter users based on type if a type filter is provided
    for (const auto& u : users) {
        if (typeFilter.empty() || u->getType() == typeFilter) {
            filtered.push_back(u.get());
        }
    }
    if (filtered.empty()) {
        throw HospitalException("No " + (typeFilter.empty() ? "users" : typeFilter + "s") + " available.");
    }
    for (size_t i = 0; i < filtered.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << filtered[i]->getId() << " - " << filtered[i]->getName() << "\n";
    }
    int choice = getIntInput("  Select user [1-" + std::to_string(filtered.size()) + "] (0 to cancel): ", 0, filtered.size());
    if (choice == 0) throw HospitalException("Operation cancelled.");
    return filtered[choice - 1]->getId();
}

static std::string selectService(const HospitalSystem& hs, bool checkRemovable = false) {
    const auto& services = hs.getServices();
    if (services.empty()) throw HospitalException("No medical services available.");

    std::vector<std::string> codes;
    int idx = 1;
    // Display all available services and optionally check if they can be removed
    for (const auto& kv : services) {
        codes.push_back(kv.first);
        std::cout << "  [" << idx++ << "] " << kv.first << " (" << kv.second.getServiceName() << ")";
        if (checkRemovable) {
            std::string blocker = hs.checkServiceRemovable(kv.first);
            if (blocker.empty()) std::cout << " - SAFE TO REMOVE";
            else std::cout << " - [BLOCKED] Active appointment: " << blocker;
        }
        std::cout << "\n";
    }
    int choice = getIntInput("  Select Service Code (Example: SRV-123) [1-" + std::to_string(codes.size()) + "] (0 to cancel): ", 0, codes.size());
    if (choice == 0) throw HospitalException("Operation cancelled.");
    return codes[choice - 1];
}

static std::string selectPatientAppointment(const Patient* p) {
    const auto& appts = p->getAppointments();
    if (appts.empty()) throw HospitalException("Patient has no appointments.");

    std::vector<std::string> ids;
    int idx = 1;
    // Iterate through all patient appointments to display as options
    for (const auto& rec : appts) {
        ids.push_back(rec.getAppointmentId());
        std::cout << "  [" << idx++ << "] " << rec.getAppointmentId()
                  << " - " << rec.getDateTime() << " (Status: " << rec.getAppointmentStatus() << ")\n";
    }
    int choice = getIntInput("  Select appointment [1-" + std::to_string(ids.size()) + "] (0 to cancel): ", 0, ids.size());
    if (choice == 0) throw HospitalException("Operation cancelled.");
    return ids[choice - 1];
}

static std::string selectClinic() {
    std::cout << "  -- Select Clinic --\n"
              << "    1 - CLN-CARDIO (Cardiology)\n"
              << "    2 - CLN-SURG (Surgery)\n"
              << "    3 - CLN-ORTHO (Orthopaedics)\n"
              << "    4 - CLN-GENERAL (General Practice)\n";
    int choice = getIntInput("  Select clinic [1-4] (0 to cancel): ", 0, 4);
    if (choice == 0) throw HospitalException("Operation cancelled.");
    switch(choice) {
        case 1: return "CLN-CARDIO";
        case 2: return "CLN-SURG";
        case 3: return "CLN-ORTHO";
        case 4: return "CLN-GENERAL";
        default: return "";
    }
}

static void handleScheduleAppointment(HospitalSystem& hs) {
    Person::printDivider('-', 55);
    Person::printCentred("SCHEDULE APPOINTMENT", 55);
    Person::printDivider('-', 55);

    std::cout << "  -- Select Patient --\n";
    std::string pid = selectUser(hs, "Patient");
    
    std::cout << "\n  -- Select Service --\n";
    std::string code = selectService(hs, false);
    
    std::cout << "\n  -- Select Doctor --\n";
    std::string did = selectUser(hs, "Doctor");

    AppointmentType type = selectAppointmentType();

    std::string date = getDateInput("  Appointment Date (DD/MM/YYYY) : ");
    std::string time = getStringInput("  Appointment Time (HH:MM)      : ");
    std::string dateTime = date + " " + time;

    std::string notes = getStringInput("  Notes (optional) : ");

    hs.scheduleAppointment(pid, code, did, dateTime, type, notes);
}

static void handleSetStatus(HospitalSystem& hs) {
    Person::printDivider('-', 55);
    Person::printCentred("SET APPOINTMENT STATUS", 55);
    Person::printDivider('-', 55);

    std::cout << "  -- Select Patient --\n";
    std::string pid = selectUser(hs, "Patient");

    std::cout << "\n  -- Select Appointment --\n";
    std::string apptId;
    const auto& users = hs.getUsers();
    // Find the selected patient to get their appointments
    for (const auto& ptr : users) {
        const Person* p = ptr.get();
        if (p->getId() == pid) {
            apptId = selectPatientAppointment(dynamic_cast<const Patient*>(p));
            break;
        }
    }
    
    std::cout << "  Status options:\n"
              << "    1 - Scheduled\n"
              << "    2 - Completed\n"
              << "    3 - Cancelled\n"
              << "    4 - Paid\n";
    int sChoice = getIntInput("  Select New Status [1-4]: ", 1, 4);
    std::string status = "Scheduled";
    if (sChoice == 2) status = "Completed";
    else if (sChoice == 3) status = "Cancelled";
    else if (sChoice == 4) status = "Paid";

    hs.setAppointmentStatus(pid, apptId, status);
}

static void handleAssignClinic(HospitalSystem& hs) {
    Person::printDivider('-', 55);
    Person::printCentred("ASSIGN DOCTOR TO CLINIC", 55);
    Person::printDivider('-', 55);

    std::cout << "  -- Select Doctor --\n";
    std::string did = selectUser(hs, "Doctor");
    std::cout << "\n";
    std::string clinic = selectClinic();
    hs.assignDoctorToClinic(did, clinic);
}

struct UpcomingApptInfo {
    int index;
    std::string doctorId;
    std::string doctorName;
    std::string patientId;
    std::string patientName;
    AppointmentRecord record;
};

static void handleUpcomingAppointments(HospitalSystem& hs) {
    bool viewingList = true;
    while (viewingList) {
        std::vector<UpcomingApptInfo> upcoming;
        int count = 1;
        
        const auto& users = hs.getUsers();
        // Loop through all users to find patients and their scheduled appointments
        for (const auto& ptr : users) {
            const Person* p = ptr.get();
            if (p->getType() == "Patient") {
                const Patient* pat = dynamic_cast<const Patient*>(p);
                for (const auto& rec : pat->getAppointments()) {
                    if (rec.getAppointmentStatus() == "Scheduled") {
                        UpcomingApptInfo info;
                        info.index = count++;
                        info.patientId = pat->getId();
                        info.patientName = pat->getName();
                        info.record = rec;
                        // Find doctor name
                        info.doctorId = rec.getDoctorId();
                        info.doctorName = "Unknown Doctor";
                        for (const auto& dptr : users) {
                            const Person* dp = dptr.get();
                            if (dp->getId() == info.doctorId) {
                                info.doctorName = dp->getName();
                                break;
                            }
                        }
                        upcoming.push_back(info);
                    }
                }
            }
        }
        
        Person::printDivider('-', 55);
        Person::printCentred("UPCOMING APPOINTMENTS", 55);
        Person::printDivider('-', 55);
        
        if (upcoming.empty()) {
            std::cout << "  No upcoming scheduled appointments.\n";
            return;
        }
        
        for (const auto& info : upcoming) {
            std::cout << "  [" << info.index << "] Date: " << info.record.getDateTime() 
                      << " | Doc: " << info.doctorName 
                      << " | Pat: " << info.patientName << "\n";
        }
        
        int choice = getIntInput("\n  Select appointment for details (0 to exit back to menu): ", 0, upcoming.size());
        if (choice == 0) {
            viewingList = false;
        } else {
            const auto& selected = upcoming[choice - 1];
            Person::printDivider('=', 55);
            Person::printCentred("APPOINTMENT DETAILS", 55);
            Person::printDivider('=', 55);
            std::cout << "  Appointment ID: " << selected.record.getAppointmentId() << "\n"
                      << "  Date & Time   : " << selected.record.getDateTime() << "\n"
                      << "  Status        : " << selected.record.getAppointmentStatus() << "\n"
                      << "  Doctor        : " << selected.doctorName << " (" << selected.doctorId << ")\n"
                      << "  Patient       : " << selected.patientName << " (" << selected.patientId << ")\n"
                      << "  Service       : " << selected.record.getServiceName() << "\n"
                      << "  Notes         : " << (selected.record.getNotes().empty() ? "None" : selected.record.getNotes()) << "\n";
            Person::printDivider('=', 55);
            
            std::cout << "\n  Press Enter to go back to the list...";
            std::string dummy;
            std::getline(std::cin, dummy);
        }
    }
}

// ─────────────────────────── Seed data ───────────────────────────

static void seedDemoData(HospitalSystem& hs) {
    std::cout << "\n  [INFO] Loading demo data...\n";

    // Services – demonstrates all 3 overloaded constructors
    hs.addMedicalService(MedicalService("CONS01", "General Consultation"));                         // 2-param
    hs.addMedicalService(MedicalService("XRAY01", "Chest X-Ray",   "Radiology",   120.0, false));  // 5-param
    hs.addMedicalService(MedicalService("SURG01", "Appendectomy",  "Surgery",    3500.0, true));
    hs.addMedicalService(MedicalService("LAB001", "Full Blood Count","Laboratory",  80.0, false));
    hs.addMedicalService(MedicalService("CARD01", "ECG Screening", "Cardiology",  200.0, false));
    hs.addMedicalService(MedicalService("ORTH01", "Bone Density Scan","Orthopaedics",250.0, true));

    // Doctors
    auto d1 = std::make_unique<Doctor>(Doctor::generateDoctorId(),
                            "Dr. Aisha Rahman", "012-3456789",
                            "aisha@xmumhosp.my", "1985-03-14", "F",
                            "Cardiology", "MMC-12345", 15, 120.0);
    d1->addQualification("MBBS (Malaya)");
    d1->addQualification("MD Cardiology (NUS)");
    std::string id_d1 = d1->getId();
    d1->assignToClinic("CLN-CARDIO");
    hs.addDoctor(std::move(d1));

    auto d2 = std::make_unique<Doctor>(Doctor::generateDoctorId(),
                            "Dr. Tan Wei Ming", "011-9876543",
                            "tanwm@xmumhosp.my", "1978-11-22", "M",
                            "General Surgery", "MMC-67890", 20, 150.0);
    d2->addQualification("MBBS (UM)");
    d2->addQualification("MRCS (Edinburgh)");
    std::string id_d2 = d2->getId();
    d2->assignToClinic("CLN-SURG");
    hs.addDoctor(std::move(d2));

    auto d3 = std::make_unique<Doctor>(Doctor::generateDoctorId(),
                            "Dr. Priya Nair", "016-1122334",
                            "priya@xmumhosp.my", "1990-07-05", "F",
                            "Orthopaedics", "MMC-11223", 8, 100.0);
    std::string id_d3 = d3->getId();
    d3->assignToClinic("CLN-ORTHO");
    hs.addDoctor(std::move(d3));

    // Patients
    auto p1 = std::make_unique<Patient>(Patient::generatePatientId(),
                              "Ahmad Faris bin Zulkifli",
                              "017-5556666", "ahmadfarisz@gmail.com",
                              "1992-06-20", "M",
                              "Great Eastern Life", "GE-884432",
                              InsuranceTier::STANDARD, "O+");
    p1->addAllergy("Penicillin");
    std::string id_p1 = p1->getId();
    hs.addPatient(std::move(p1));

    auto p2 = std::make_unique<Patient>(Patient::generatePatientId(),
                              "Lim Shu Ying",
                              "013-7778888", "limshuying@hotmail.com",
                              "1988-12-01", "F",
                              "AXA Affin", "AXA-993311",
                              InsuranceTier::PREMIUM, "A-");
    std::string id_p2 = p2->getId();
    hs.addPatient(std::move(p2));

    auto p3 = std::make_unique<Patient>(Patient::generatePatientId(),
                              "Krishnamurthy s/o Rajan",
                              "019-4443333", "krishna.rajan@yahoo.com",
                              "1975-04-17", "M",
                              "Prudential", "PRU-221188",
                              InsuranceTier::BASIC, "B+");
    p3->addAllergy("Aspirin");
    p3->addAllergy("Sulfa drugs");
    std::string id_p3 = p3->getId();
    hs.addPatient(std::move(p3));

    // Appointments
    // Retrieve service from system

    hs.scheduleAppointment(id_p1, "CONS01", id_d1,
                           "07/07/2026 09:00",
                           AppointmentType::REGULAR, "Routine check-up");
    hs.scheduleAppointment(id_p1, "CARD01", id_d1,
                           "07/07/2026 10:30",
                           AppointmentType::REGULAR, "Follow cardiac risk");
    hs.scheduleAppointment(id_p2, "XRAY01", id_d2,
                           "07/07/2026 11:00",
                           AppointmentType::EMERGENCY, "Acute chest pain");
    hs.scheduleAppointment(id_p3, "ORTH01", id_d3,
                           "08/07/2026 14:00",
                           AppointmentType::FOLLOW_UP, "Post-op review");
    hs.scheduleAppointment(id_p3, "LAB001", id_d2,
                           "08/07/2026 09:00",
                           AppointmentType::REGULAR, "Pre-surgical blood panel");

    // Mark some as completed
    const auto& users = hs.getUsers();
    // Find the first patient and mark their first appointment as completed
    for (const auto& ptr : users) {
        if (ptr->getId() == id_p1) {
            const Patient* pat = dynamic_cast<const Patient*>(ptr.get());
            const auto& appts = pat->getAppointments();
            if (!appts.empty()) {
                hs.setAppointmentStatus(id_p1, appts[0].getAppointmentId(), "Completed");
            }
            break;
        }
    }

    std::cout << "  [INFO] Demo data loaded successfully.\n\n";
}

// ─────────────────────────── Main ───────────────────────────

int main() {
    HospitalSystem hs("XMUM General Hospital");

    // Seed demo data so system is immediately usable
    seedDemoData(hs);

    // Show help guide once at startup
    displayHelpGuide();
    std::cout << "\n  Press Enter to continue to the main menu...";
    std::string dummy;
    std::getline(std::cin, dummy);

    bool running = true;
    while (running) {
        printMainMenu();
        int choice = getIntInput("  Enter choice: ", 0, 18);

        try {
            switch (choice) {
                // Data Entry
                case 1:  handleAddPatient(hs);           break;
                case 2:  handleAddDoctor(hs);            break;
                case 3:  handleAddService(hs);           break;
                // Appointments & Clinics
                case 4:  handleScheduleAppointment(hs);  break;
                case 5:  handleSetStatus(hs);            break;
                case 6:  handleUpcomingAppointments(hs); break;
                case 7:  handleAssignClinic(hs);         break;
                // Record Access & Reports
                case 8:  hs.viewAllUsers();              break;
                case 9:  hs.viewAllPatients();           break;
                case 10: hs.viewAllDoctors();            break;
                case 11: hs.listMedicalServices();       break;
                case 12: {
                    Person::printDivider('-', 55);
                    Person::printCentred("PATIENT BILLING REPORT", 55);
                    Person::printDivider('-', 55);
                    std::string pid = selectUser(hs, "Patient");
                    hs.generateBillingReport(pid);
                    break;
                }
                case 13: hs.generateSystemSummary();     break;
                // System & Maintenance
                case 14: {
                    std::string fname = getStringInput("  Filename (e.g. hospital_data.txt): ");
                    hs.saveToFile(fname);
                    break;
                }
                case 15: {
                    std::string fname = getStringInput("  Filename to load (e.g. hospital_data.txt): ");
                    hs.loadFromFile(fname);
                    break;
                }
                case 16: {
                    Person::printDivider('-', 55);
                    Person::printCentred("REMOVE USER", 55);
                    Person::printDivider('-', 55);
                    std::cout << "  NOTE: IF YOU REMOVE A PATIENT FROM THE SYSTEM, ALL\n"
                              << "  THEIR APPOINTMENTS ARE CANCELLED AUTOMATICALLY.\n";
                    Person::printDivider('-', 55);
                    std::string uid = selectUser(hs);

                    // Determine if this user is a Doctor with active appointments
                    const auto& allUsers = hs.getUsers();
                    std::string userType, userName;
                    // Find the user to determine their type and name
                    for (const auto& ptr : allUsers) {
                        const Person* p = ptr.get();
                        if (p->getId() == uid) {
                            userType = p->getType();
                            userName = p->getName();
                            break;
                        }
                    }

                    if (userType == "Doctor") {
                        auto blockers = hs.checkDoctorRemovable(uid);
                        if (!blockers.empty()) {
                            // Display blocking notice for each active appointment
                            Person::printDivider('!', 55);
                            for (const auto& info : blockers) {
                                std::cout << "\n  [!] USER " << userName
                                          << " CANNOT BE DELETED FROM THE SYSTEM\n"
                                          << "      DUE TO AN EXISTING APPOINTMENT MADE WITH\n"
                                          << "      [NAME: " << info.patientName
                                          << ", ID: " << info.patientId << "]\n";
                            }
                            Person::printDivider('!', 55);

                            std::cout << "\n  Options:\n"
                                      << "    1 - Reassign all appointments to a different doctor\n"
                                      << "    2 - Cancel deletion\n";
                            int delChoice = getIntInput("  Select option [1-2]: ", 1, 2);

                            if (delChoice == 2) {
                                std::cout << "  [OK] Deletion cancelled.\n";
                                break;
                            }

                            // Reassign each blocking appointment
                            bool aborted = false;
                            // Reassign each blocking appointment to a new doctor
                            for (const auto& info : blockers) {
                                Person::printDivider('-', 55);
                                std::cout << "  CALL " << info.patientName
                                          << " at " << info.patientContact
                                          << " to confirm reassignment\n";
                                Person::printDivider('-', 55);

                                std::cout << "\n  -- Select Replacement Doctor for "
                                          << info.appointmentId << " --\n";

                                // Let user pick a new doctor (loop until valid)
                                std::string newDocId;
                                while (true) {
                                    newDocId = selectUser(hs, "Doctor");
                                    if (newDocId == uid) {
                                        std::cout << "  [!] Cannot reassign to the same doctor being deleted.\n"
                                                  << "      Please select a different doctor.\n\n";
                                        continue;
                                    }
                                    break;
                                }

                                hs.reassignAppointmentDoctor(info.patientId,
                                                             info.appointmentId,
                                                             newDocId);
                            }

                            if (aborted) break;
                            // All appointments reassigned — proceed with removal
                        }
                    }

                    hs.removeUser(uid);
                    break;
                }
                case 17: {
                    Person::printDivider('-', 55);
                    Person::printCentred("REMOVE MEDICAL SERVICE", 55);
                    Person::printDivider('-', 55);
                    std::string sCode = selectService(hs, true);
                    hs.removeMedicalService(sCode);
                    break;
                }
                case 18:
                    displayHelpGuide();
                    break;
                case 0:
                    Person::printDivider('=', 55);
                    Person::printCentred("Thank you. System exiting.", 55);
                    Person::printCentred("All dynamically allocated memory freed.", 55);
                    Person::printDivider('=', 55);
                    running = false;
                    break;
            }
        }
        catch (const HospitalException& e) {
            std::cout << "\n  [ERROR] " << e.what() << "\n";
        }
        catch (const std::exception& e) {
            std::cout << "\n  [SYSTEM ERROR] " << e.what() << "\n";
        }

        if (running) {
            std::cout << "\n  Press Enter to return to menu...";
            std::string dummy;
            std::getline(std::cin, dummy);
        }
    }

    // All Person* objects deleted via HospitalSystem destructor
    return 0;
}

```

