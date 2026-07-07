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
Patient::Patient()
    : Person(), insuranceProvider("None"), insurancePolicyNumber(""),
      insuranceTier(InsuranceTier::NONE), bloodType("Unknown")
{
    ++patientCount;
}

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
void Patient::displayRole() const {
    Person::printDivider('-', 55);
    std::cout << "  [PATIENT RECORD]\n";
    Person::printDivider('-', 55);
    displaySummary();
    std::cout << "  Blood Type  : " << bloodType << "\n";
    std::cout << "  Insurance   : " << insuranceProvider
              << " (" << AppointmentRecord::tierToString(insuranceTier) << ")\n";
    std::cout << "  Policy No.  : " << insurancePolicyNumber << "\n";
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
double Patient::calculateTotalMedicalExpenses() const {
    double total = 0.0;
    for (const auto& rec : appointments)
        total += rec.calculateTotalBilling();
    return total;
}

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
