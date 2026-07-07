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
  for (const auto &ptr : users) {
    const Person *p = ptr.get();
    if (p->getId() == id)
      return true;
  }
  return false;
}

Patient *HospitalSystem::findPatient(const std::string &id) {
  for (const auto &ptr : users) {
    Person *p = ptr.get();
    if (p->getId() == id && p->getType() == "Patient")
      return dynamic_cast<Patient *>(p);
  }
  return nullptr;
}

Doctor *HospitalSystem::findDoctor(const std::string &id) {
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
void HospitalSystem::saveToFile(const std::string &filename) const {
  std::ofstream ofs(filename);
  if (!ofs.is_open())
    throw HospitalException("Cannot open file for writing: " + filename);

  // Write services
  ofs << "[SERVICES]\n";
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
