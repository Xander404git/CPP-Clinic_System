// HospitalSystem.cpp
#include "../include/HospitalSystem.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>

// ----- Constructor / Destructor -----
HospitalSystem::HospitalSystem(const std::string& name) : hospitalName(name) {}

HospitalSystem::~HospitalSystem() {
    // Memory management: delete all dynamically allocated Person objects
    for (Person* p : users) delete p;
    users.clear();
}

// ----- Internal helpers -----
bool HospitalSystem::idExists(const std::string& id) const {
    for (const Person* p : users)
        if (p->getId() == id) return true;
    return false;
}

Patient* HospitalSystem::findPatient(const std::string& id) {
    for (Person* p : users)
        if (p->getId() == id && p->getType() == "Patient")
            return dynamic_cast<Patient*>(p);
    return nullptr;
}

Doctor* HospitalSystem::findDoctor(const std::string& id) {
    for (Person* p : users)
        if (p->getId() == id && p->getType() == "Doctor")
            return dynamic_cast<Doctor*>(p);
    return nullptr;
}

// ----- User management -----
void HospitalSystem::addPatient(Patient* p) {
    if (!p) throw HospitalException("Null patient pointer.");
    if (idExists(p->getId())) throw DuplicateIdException(p->getId());
    users.push_back(p);
    std::cout << "  [OK] Patient '" << p->getName() << "' registered. ID: " << p->getId() << "\n";
}

void HospitalSystem::addDoctor(Doctor* d) {
    if (!d) throw HospitalException("Null doctor pointer.");
    if (idExists(d->getId())) throw DuplicateIdException(d->getId());
    users.push_back(d);
    std::cout << "  [OK] Doctor '" << d->getName() << "' registered. ID: " << d->getId() << "\n";
}

void HospitalSystem::removeUser(const std::string& id) {
    auto it = std::find_if(users.begin(), users.end(),
        [&](Person* p){ return p->getId() == id; });
    if (it == users.end()) throw NotFoundException(id);

    Person* target = *it;

    // ── Aggregation cleanup ──────────────────────────────────────
    // When we destroy one entity, we must clean up any weak (aggregation)
    // references that OTHER entities hold to it.
    // Composition relationships (embedded objects) are destroyed
    // automatically by the destructor — no cleanup needed for those.

    if (target->getType() == "Patient") {
        // A Patient is being removed.
        // Doctors hold the patient's ID in their assignedPatientIds list
        // (aggregation). We must remove it so doctors don't reference
        // a patient that no longer exists.
        for (Person* p : users) {
            if (p->getType() == "Doctor") {
                Doctor* doc = dynamic_cast<Doctor*>(p);
                doc->removeAssignedPatient(id);
            }
        }
        // The Patient's own AppointmentRecords (composition) and their
        // embedded MedicalService objects (nested composition) will be
        // destroyed automatically when `delete target` runs below.

    } else if (target->getType() == "Doctor") {
        // A Doctor is being removed.
        // Patients' AppointmentRecords hold the doctor's ID as a string
        // (aggregation). We must clear it so appointments don't reference
        // a doctor that no longer exists.
        // The appointment ITSELF is preserved — it is owned by the
        // Patient (composition), not by the Doctor.
        for (Person* p : users) {
            if (p->getType() == "Patient") {
                Patient* pat = dynamic_cast<Patient*>(p);
                pat->clearDoctorFromAppointments(id);
            }
        }
    }

    // ── Destroy the object (composition: owned data is freed) ────
    delete target;
    users.erase(it);
    std::cout << "  [OK] User " << id << " removed from system.\n";
}

// ----- Service management -----
void HospitalSystem::addMedicalService(const MedicalService& svc) {
    if (servicesMap.count(svc.getServiceCode()))
        throw DuplicateIdException(svc.getServiceCode());
    servicesMap[svc.getServiceCode()] = svc;
    std::cout << "  [OK] Medical service '" << svc.getServiceName()
              << "' added. Code: " << svc.getServiceCode() << "\n";
}

std::string HospitalSystem::checkServiceRemovable(const std::string& serviceCode) const {
    if (!servicesMap.count(serviceCode)) return "";

    for (const Person* p : users) {
        if (p->getType() == "Patient") {
            const Patient* pat = dynamic_cast<const Patient*>(p);
            for (const auto& rec : pat->getAppointments()) {
                if (rec.getServiceCode() == serviceCode &&
                    rec.getAppointmentStatus() != "Cancelled") {
                    return rec.getAppointmentId(); // Returns the blocking appointment
                }
            }
        }
    }
    return ""; // Empty string means safe to remove
}

void HospitalSystem::removeMedicalService(const std::string& serviceCode) {
    if (!servicesMap.count(serviceCode))
        throw NotFoundException(serviceCode);

    // Safety check: refuse removal if any active appointment references this service.
    std::string blocker = checkServiceRemovable(serviceCode);
    if (!blocker.empty()) {
        throw HospitalException(
            "Cannot remove service '" + serviceCode +
            "' — it is referenced by active appointment " + blocker + ".");
    }

    std::string name = servicesMap[serviceCode].getServiceName();
    servicesMap.erase(serviceCode);
    std::cout << "  [OK] Medical service '" << name << "' (" << serviceCode << ") removed.\n";
}

void HospitalSystem::listMedicalServices() const {
    Person::printDivider('=', 55);
    Person::printCentred("MEDICAL SERVICES CATALOGUE", 55);
    Person::printDivider('=', 55);
    if (servicesMap.empty()) {
        std::cout << "  No services registered.\n";
    } else {
        int i = 1;
        for (const auto& kv : servicesMap) {
            std::cout << "  [" << i++ << "]\n";
            kv.second.displayServiceDetails();
        }
    }
    Person::printDivider('=', 55);
}

// ----- Appointment operations -----
void HospitalSystem::scheduleAppointment(const std::string& patientId,
                                         const std::string& serviceCode,
                                         const std::string& doctorId,
                                         AppointmentType type,
                                         const std::string& notes) {
    Patient* patient = findPatient(patientId);
    if (!patient) throw NotFoundException(patientId);

    auto it = servicesMap.find(serviceCode);
    if (it == servicesMap.end()) throw NotFoundException(serviceCode);

    Doctor* doc = findDoctor(doctorId);
    if (!doc) throw NotFoundException(doctorId);

    patient->scheduleAppointment(it->second, doctorId, type, notes);
    doc->addAssignedPatient(patientId);
}

void HospitalSystem::setAppointmentStatus(const std::string& patientId,
                                           const std::string& apptId,
                                           const std::string& status) {
    Patient* patient = findPatient(patientId);
    if (!patient) throw NotFoundException(patientId);
    if (!patient->setAppointmentStatus(apptId, status))
        throw NotFoundException(apptId);
    std::cout << "  [OK] Appointment " << apptId << " status set to '" << status << "'.\n";
}

void HospitalSystem::assignDoctorToClinic(const std::string& doctorId,
                                           const std::string& clinicCode) {
    Doctor* doc = findDoctor(doctorId);
    if (!doc) throw NotFoundException(doctorId);
    doc->assignToClinic(clinicCode);
}

// ----- Polymorphic display (Runtime Polymorphism) -----
void HospitalSystem::viewAllUsers() const {
    Person::printDivider('=', 55);
    Person::printCentred("ALL REGISTERED USERS", 55);
    std::cout << "  Total: " << users.size() << " user(s)\n";
    Person::printDivider('=', 55);
    for (const Person* p : users)
        p->displayRole();   // <-- runtime polymorphism: correct override called at runtime
}

void HospitalSystem::viewAllPatients() const {
    Person::printDivider('=', 55);
    Person::printCentred("ALL PATIENTS", 55);
    Person::printDivider('=', 55);
    int count = 0;
    for (const Person* p : users)
        if (p->getType() == "Patient") { p->displayRole(); ++count; }
    if (count == 0) std::cout << "  No patients registered.\n";
}

void HospitalSystem::viewAllDoctors() const {
    Person::printDivider('=', 55);
    Person::printCentred("ALL DOCTORS", 55);
    Person::printDivider('=', 55);
    int count = 0;
    for (const Person* p : users)
        if (p->getType() == "Doctor") { p->displayRole(); ++count; }
    if (count == 0) std::cout << "  No doctors registered.\n";
}

// ----- Billing report -----
void HospitalSystem::generateBillingReport(const std::string& patientId) const {
    // const version – use const_cast to call findPatient via loop
    for (const Person* p : users) {
        if (p->getId() == patientId && p->getType() == "Patient") {
            const Patient* pat = dynamic_cast<const Patient*>(p);
            pat->displayAppointmentHistory();
            return;
        }
    }
    throw NotFoundException(patientId);
}

// ----- System summary -----
void HospitalSystem::generateSystemSummary() const {
    Person::printDivider('=', 55);
    Person::printCentred(hospitalName, 55);
    Person::printCentred("SYSTEM SUMMARY", 55);
    Person::printDivider('=', 55);
    std::cout << "  Total Users    : " << users.size() << "\n"
              << "  Total Patients : " << Patient::getPatientCount() << "\n"
              << "  Total Doctors  : " << Doctor::getDoctorCount()   << "\n"
              << "  Total Services : " << servicesMap.size()         << "\n";
    Person::printDivider('=', 55);
}

// ----- File persistence -----
void HospitalSystem::saveToFile(const std::string& filename) const {
    std::ofstream ofs(filename);
    if (!ofs.is_open())
        throw HospitalException("Cannot open file for writing: " + filename);

    // Write services
    ofs << "[SERVICES]\n";
    for (const auto& kv : servicesMap) {
        const MedicalService& s = kv.second;
        ofs << s.getServiceCode() << "|"
            << s.getServiceName() << "|"
            << s.getServiceCategory() << "|"
            << s.getBaseFee() << "|"
            << s.getRequiresReferral() << "\n";
    }

    // Write users
    ofs << "[USERS]\n";
    for (const Person* p : users) {
        ofs << p->getType() << "|"
            << p->getId()   << "|"
            << p->getName() << "|"
            << p->getContactNumber() << "|"
            << p->getEmail()         << "|"
            << p->getDateOfBirth()   << "|"
            << p->getGender();

        if (p->getType() == "Patient") {
            const Patient* pat = dynamic_cast<const Patient*>(p);
            ofs << "|" << pat->getInsuranceProvider()
                << "|" << pat->getInsurancePolicyNumber()
                << "|" << static_cast<int>(pat->getInsuranceTier())
                << "|" << pat->getBloodType();
            
            ofs << "|";
            const auto& allergies = pat->getAllergies();
            for (size_t i = 0; i < allergies.size(); ++i) {
                ofs << allergies[i] << (i + 1 < allergies.size() ? "," : "");
            }
        } else if (p->getType() == "Doctor") {
            const Doctor* doc = dynamic_cast<const Doctor*>(p);
            ofs << "|" << doc->getSpecialization()
                << "|" << doc->getLicenseNumber()
                << "|" << doc->getYearsOfExperience()
                << "|" << doc->getConsultationFee()
                << "|" << doc->getAssignedClinicCode();
            
            ofs << "|";
            const auto& quals = doc->getQualifications();
            for (size_t i = 0; i < quals.size(); ++i) {
                ofs << quals[i] << (i + 1 < quals.size() ? "," : "");
            }
            
            ofs << "|";
            const auto& pats = doc->getAssignedPatients();
            for (size_t i = 0; i < pats.size(); ++i) {
                ofs << pats[i] << (i + 1 < pats.size() ? "," : "");
            }
        }
        ofs << "\n";
    }

    // Write appointments
    ofs << "[APPOINTMENTS]\n";
    for (const Person* p : users) {
        if (p->getType() == "Patient") {
            const Patient* pat = dynamic_cast<const Patient*>(p);
            for (const auto& rec : pat->getAppointments()) {
                ofs << pat->getId() << "|"
                    << rec.getAppointmentId() << "|"
                    << rec.getServiceCode() << "|"
                    << rec.getDoctorId() << "|"
                    << static_cast<int>(rec.getAppointmentType()) << "|"
                    << static_cast<int>(rec.getInsuranceTier()) << "|"
                    << rec.getAppointmentStatus() << "|"
                    << rec.getDateTime() << "|"
                    << rec.getNotes() << "\n";
            }
        }
    }

    ofs.close();
    std::cout << "  [OK] Data saved to '" << filename << "'.\n";
}

void HospitalSystem::loadFromFile(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open())
        throw HospitalException("Cannot open file for reading: " + filename);

    // Clear current state
    for (Person* p : users) delete p;
    users.clear();
    servicesMap.clear();

    std::string line;
    std::string section = "";
    
    // Helper to split string by delimiter
    auto split = [](const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        if (!s.empty() && s.back() == delimiter) tokens.push_back("");
        return tokens;
    };

    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        if (line[0] == '[') { section = line; continue; }

        auto fields = split(line, '|');

        if (section == "[SERVICES]" && fields.size() >= 5) {
            MedicalService svc(fields[0], fields[1], fields[2],
                              std::stod(fields[3]), fields[4] == "1");
            servicesMap[fields[0]] = svc;
        }
        else if (section == "[USERS]" && fields.size() >= 7) {
            std::string type = fields[0];
            std::string id = fields[1], name = fields[2], contact = fields[3], 
                        email = fields[4], dob = fields[5], gender = fields[6];
            
            if (type == "Patient" && fields.size() >= 12) {
                InsuranceTier tier = static_cast<InsuranceTier>(std::stoi(fields[9]));
                Patient* pat = new Patient(id, name, contact, email, dob, gender,
                                           fields[7], fields[8], tier, fields[10]);
                auto allergies = split(fields[11], ',');
                for (const auto& a : allergies) {
                    if (!a.empty()) pat->addAllergy(a);
                }
                users.push_back(pat);
            }
            else if (type == "Doctor" && fields.size() >= 14) {
                Doctor* doc = new Doctor(id, name, contact, email, dob, gender,
                                         fields[7], fields[8], std::stoi(fields[9]), std::stod(fields[10]));
                if (!fields[11].empty()) doc->assignToClinic(fields[11]);
                
                auto quals = split(fields[12], ',');
                for (const auto& q : quals) {
                    if (!q.empty()) doc->addQualification(q);
                }
                
                auto pats = split(fields[13], ',');
                for (const auto& p_id : pats) {
                    if (!p_id.empty()) doc->addAssignedPatient(p_id);
                }
                users.push_back(doc);
            }
        }
        else if (section == "[APPOINTMENTS]" && fields.size() >= 9) {
            std::string patId = fields[0];
            std::string apptId = fields[1];
            std::string sCode = fields[2];
            std::string docId = fields[3];
            AppointmentType aType = static_cast<AppointmentType>(std::stoi(fields[4]));
            InsuranceTier iTier = static_cast<InsuranceTier>(std::stoi(fields[5]));
            std::string status = fields[6];
            std::string dt = fields[7];
            std::string notes = fields[8];

            Patient* pat = findPatient(patId);
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
    std::cout << "  [OK] Data loaded from '" << filename << "'.\n";
}

// ----- Getters -----
const std::vector<Person*>& HospitalSystem::getUsers()    const { return users; }
const std::map<std::string,MedicalService>& HospitalSystem::getServices() const { return servicesMap; }
int HospitalSystem::getUserCount()    const { return static_cast<int>(users.size()); }
int HospitalSystem::getServiceCount() const { return static_cast<int>(servicesMap.size()); }
