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
