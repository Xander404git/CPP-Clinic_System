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
Doctor::Doctor()
    : Person(), specializationDepartment("General"), assignedClinicCode(""),
      licenseNumber(""), yearsOfExperience(0), consultationFee(0.0)
{
    ++doctorCount;
}

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
      consultationFee(consultFee)
{
    ++doctorCount;
}

Doctor::~Doctor() {
    --doctorCount;
}

// ----- Pure virtual override -----
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
    std::cout << "  Patients Seen  : " << assignedPatientIds.size() << "\n";
    Person::printDivider('-', 55);
}

std::string Doctor::getType() const { return "Doctor"; }

// ----- Clinic management -----
void Doctor::assignToClinic(const std::string& clinicCode) {
    assignedClinicCode = clinicCode;
    std::cout << "  [OK] Dr. " << name << " assigned to clinic [" << clinicCode << "].\n";
}

void Doctor::addQualification(const std::string& qual) {
    qualifications.push_back(qual);
}

void Doctor::addAssignedPatient(const std::string& patientId) {
    assignedPatientIds.push_back(patientId);
}

void Doctor::removeAssignedPatient(const std::string& patientId) {
    assignedPatientIds.erase(
        std::remove(assignedPatientIds.begin(), assignedPatientIds.end(), patientId),
        assignedPatientIds.end()
    );
}

// ----- Getters -----
std::string Doctor::getSpecialization()     const { return specializationDepartment; }
std::string Doctor::getAssignedClinicCode() const { return assignedClinicCode; }
std::string Doctor::getLicenseNumber()      const { return licenseNumber; }
int         Doctor::getYearsOfExperience()  const { return yearsOfExperience; }
double      Doctor::getConsultationFee()    const { return consultationFee; }
const std::vector<std::string>& Doctor::getQualifications() const { return qualifications; }
const std::vector<std::string>& Doctor::getAssignedPatients() const { return assignedPatientIds; }

// ----- Static -----
int Doctor::getDoctorCount() { return doctorCount; }

std::string Doctor::generateDoctorId() {
    std::ostringstream oss;
    oss << "DOC" << (++nextDoctorNumber);
    return oss.str();
}
