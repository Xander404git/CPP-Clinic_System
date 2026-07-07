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
#include "../include/InputValidator.h"

// ─────────────────────────── Helpers ───────────────────────────

static int getIntInput(const std::string& prompt, int lo = 0, int hi = 9999) {
    return Input::getValidInput<int>(
        prompt,
        [lo, hi](const int& v) { return v >= lo && v <= hi; },
        "  [!] Invalid input. Please enter a number between " + std::to_string(lo) + " and " + std::to_string(hi) + ".\n"
    );
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

static InsuranceTier selectInsuranceTier() {
    std::cout << "  Insurance Tier:\n"
              << "    0 – None\n"
              << "    1 – Basic    (15% coverage)\n"
              << "    2 – Standard (30% coverage)\n"
              << "    3 – Premium  (50% coverage)\n";
    int ch = getIntInput("  Select tier [0-3]: ", 0, 3);
    return static_cast<InsuranceTier>(ch);
}

static AppointmentType selectAppointmentType() {
    std::cout << "  Appointment Type:\n"
              << "    1 – Regular\n"
              << "    2 – Emergency (+30% surcharge)\n"
              << "    3 – Follow-Up (-30% discount)\n";
    int ch = getIntInput("  Select type [1-3]: ", 1, 3);
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
    std::cout
        << "  1.  Add New Patient\n"
        << "  2.  Add New Doctor\n"
        << "  3.  Add New Medical Service\n"
        << "  4.  Schedule Appointment for Patient\n"
        << "  5.  Set Appointment Status\n"
        << "  6.  View All Users\n"
        << "  7.  View All Patients\n"
        << "  8.  View All Doctors\n"
        << "  9.  View Medical Services Catalogue\n"
        << "  10. Assign Doctor to Clinic\n"
        << "  11. Generate Patient Billing Report\n"
        << "  12. System Summary\n"
        << "  13. Save Data to File\n"
        << "  14. Load Data from File\n"
        << "  15. Remove User (Patient/Doctor)\n"
        << "  16. Remove Medical Service\n"
        << "  0.  Exit System\n";
    Person::printDivider('-', 55);
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
    std::string dob     = getStringInput("  Date of Birth   : ");
    std::string gender  = getStringInput("  Gender (M/F)    : ");
    std::string ins     = getStringInput("  Insurance Co.   : ");
    std::string policy  = getStringInput("  Policy Number   : ");
    InsuranceTier tier  = selectInsuranceTier();
    std::string blood   = getStringInput("  Blood Type      : ");

    Patient* p = new Patient(id, name, contact, email, dob, gender, ins, policy, tier, blood);

    // Optional allergies
    int numAllergies = getIntInput("  Number of known allergies [0-10]: ", 0, 10);
    for (int i = 0; i < numAllergies; ++i) {
        std::string a = getStringInput("  Allergy " + std::to_string(i + 1) + "      : ");
        p->addAllergy(a);
    }

    hs.addPatient(p);
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
    std::string dob     = getStringInput("  Date of Birth      : ");
    std::string gender  = getStringInput("  Gender (M/F)       : ");
    std::string spec    = getStringInput("  Specialization     : ");
    std::string license = getStringInput("  License Number     : ");
    int yrs             = getIntInput("  Years of Experience: ", 0, 60);
    double fee          = getDoubleInput("  Consultation Fee (RM): ");

    Doctor* d = new Doctor(id, name, contact, email, dob, gender, spec, license, yrs, fee);

    int numQual = getIntInput("  Number of qualifications [0-5]: ", 0, 5);
    for (int i = 0; i < numQual; ++i) {
        std::string q = getStringInput("  Qualification " + std::to_string(i + 1) + " : ");
        d->addQualification(q);
    }

    hs.addDoctor(d);
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
    for (const Person* p : users) {
        if (typeFilter.empty() || p->getType() == typeFilter) {
            filtered.push_back(p);
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
    int choice = getIntInput("  Select service [1-" + std::to_string(codes.size()) + "] (0 to cancel): ", 0, codes.size());
    if (choice == 0) throw HospitalException("Operation cancelled.");
    return codes[choice - 1];
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
    std::string notes    = getStringInput("  Notes (optional) : ");

    hs.scheduleAppointment(pid, code, did, type, notes);
}

static void handleSetStatus(HospitalSystem& hs) {
    Person::printDivider('-', 55);
    Person::printCentred("SET APPOINTMENT STATUS", 55);
    Person::printDivider('-', 55);

    std::string pid   = getStringInput("  Patient ID       : ");
    std::string apptId= getStringInput("  Appointment ID   : ");
    
    std::cout << "  Status options:\n"
              << "    1 - Scheduled\n"
              << "    2 - Completed\n"
              << "    3 - Cancelled\n";
    int sChoice = getIntInput("  Select New Status [1-3]: ", 1, 3);
    std::string status = "Scheduled";
    if (sChoice == 2) status = "Completed";
    else if (sChoice == 3) status = "Cancelled";

    hs.setAppointmentStatus(pid, apptId, status);
}

static void handleAssignClinic(HospitalSystem& hs) {
    Person::printDivider('-', 55);
    Person::printCentred("ASSIGN DOCTOR TO CLINIC", 55);
    Person::printDivider('-', 55);

    std::string did   = getStringInput("  Doctor ID        : ");
    std::string clinic= getStringInput("  Clinic Code      : ");
    hs.assignDoctorToClinic(did, clinic);
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
    Doctor* d1 = new Doctor(Doctor::generateDoctorId(),
                            "Dr. Aisha Rahman", "012-3456789",
                            "aisha@xmumhosp.my", "1985-03-14", "F",
                            "Cardiology", "MMC-12345", 15, 120.0);
    d1->addQualification("MBBS (Malaya)");
    d1->addQualification("MD Cardiology (NUS)");
    hs.addDoctor(d1);

    Doctor* d2 = new Doctor(Doctor::generateDoctorId(),
                            "Dr. Tan Wei Ming", "011-9876543",
                            "tanwm@xmumhosp.my", "1978-11-22", "M",
                            "General Surgery", "MMC-67890", 20, 150.0);
    d2->addQualification("MBBS (UM)");
    d2->addQualification("MRCS (Edinburgh)");
    hs.addDoctor(d2);

    Doctor* d3 = new Doctor(Doctor::generateDoctorId(),
                            "Dr. Priya Nair", "016-1122334",
                            "priya@xmumhosp.my", "1990-07-05", "F",
                            "Orthopaedics", "MMC-11223", 8, 100.0);
    hs.addDoctor(d3);

    // Assign clinics
    hs.assignDoctorToClinic(d1->getId(), "CLN-CARDIO");
    hs.assignDoctorToClinic(d2->getId(), "CLN-SURG");
    hs.assignDoctorToClinic(d3->getId(), "CLN-ORTHO");

    // Patients
    Patient* p1 = new Patient(Patient::generatePatientId(),
                              "Ahmad Faris bin Zulkifli",
                              "017-5556666", "ahmadfarisz@gmail.com",
                              "1992-06-20", "M",
                              "Great Eastern Life", "GE-884432",
                              InsuranceTier::STANDARD, "O+");
    p1->addAllergy("Penicillin");
    hs.addPatient(p1);

    Patient* p2 = new Patient(Patient::generatePatientId(),
                              "Lim Shu Ying",
                              "013-7778888", "limshuying@hotmail.com",
                              "1988-12-01", "F",
                              "AXA Affin", "AXA-993311",
                              InsuranceTier::PREMIUM, "A-");
    hs.addPatient(p2);

    Patient* p3 = new Patient(Patient::generatePatientId(),
                              "Krishnamurthy s/o Rajan",
                              "019-4443333", "krishna.rajan@yahoo.com",
                              "1975-04-17", "M",
                              "Prudential", "PRU-221188",
                              InsuranceTier::BASIC, "B+");
    p3->addAllergy("Aspirin");
    p3->addAllergy("Sulfa drugs");
    hs.addPatient(p3);

    // Appointments
    // Retrieve service from system

    hs.scheduleAppointment(p1->getId(), "CONS01", d1->getId(),
                           AppointmentType::REGULAR, "Routine check-up");
    hs.scheduleAppointment(p1->getId(), "CARD01", d1->getId(),
                           AppointmentType::REGULAR, "Follow cardiac risk");
    hs.scheduleAppointment(p2->getId(), "XRAY01", d2->getId(),
                           AppointmentType::EMERGENCY, "Acute chest pain");
    hs.scheduleAppointment(p3->getId(), "ORTH01", d3->getId(),
                           AppointmentType::FOLLOW_UP, "Post-op review");
    hs.scheduleAppointment(p3->getId(), "LAB001", d2->getId(),
                           AppointmentType::REGULAR, "Pre-surgical blood panel");

    // Mark some as completed
    const auto& p1appts = p1->getAppointments();
    if (!p1appts.empty())
        hs.setAppointmentStatus(p1->getId(), p1appts[0].getAppointmentId(), "Completed");

    std::cout << "  [INFO] Demo data loaded successfully.\n\n";
}

// ─────────────────────────── Main ───────────────────────────

int main() {
    HospitalSystem hs("XMUM General Hospital");

    // Seed demo data so system is immediately usable
    seedDemoData(hs);

    bool running = true;
    while (running) {
        printMainMenu();
        int choice = getIntInput("  Enter choice: ", 0, 16);

        try {
            switch (choice) {
                case 1:  handleAddPatient(hs);           break;
                case 2:  handleAddDoctor(hs);            break;
                case 3:  handleAddService(hs);           break;
                case 4:  handleScheduleAppointment(hs);  break;
                case 5:  handleSetStatus(hs);            break;
                case 6:  hs.viewAllUsers();              break;
                case 7:  hs.viewAllPatients();           break;
                case 8:  hs.viewAllDoctors();            break;
                case 9:  hs.listMedicalServices();       break;
                case 10: handleAssignClinic(hs);         break;
                case 11: {
                    std::string pid = getStringInput("  Patient ID: ");
                    hs.generateBillingReport(pid);
                    break;
                }
                case 12: hs.generateSystemSummary();     break;
                case 13: {
                    std::string fname = getStringInput("  Filename (e.g. hospital_data.txt): ");
                    hs.saveToFile(fname);
                    break;
                }
                case 14: {
                    std::string fname = getStringInput("  Filename to load (e.g. hospital_data.txt): ");
                    hs.loadFromFile(fname);
                    break;
                }
                case 15: {
                    Person::printDivider('-', 55);
                    Person::printCentred("REMOVE USER", 55);
                    Person::printDivider('-', 55);
                    std::string uid = selectUser(hs);
                    hs.removeUser(uid);
                    break;
                }
                case 16: {
                    Person::printDivider('-', 55);
                    Person::printCentred("REMOVE MEDICAL SERVICE", 55);
                    Person::printDivider('-', 55);
                    std::string sCode = selectService(hs, true);
                    hs.removeMedicalService(sCode);
                    break;
                }
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
