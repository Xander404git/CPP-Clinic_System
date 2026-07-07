// AppointmentRecord.cpp
#include "../include/AppointmentRecord.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

// ----- Static member -----
int AppointmentRecord::idCounter = 1000;

// ----- ID generator -----
std::string AppointmentRecord::generateId() {
    std::ostringstream oss;
    oss << "APT" << (++idCounter);
    return oss.str();
}

// Helper: current date-time string
static std::string currentDateTime() {
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", std::localtime(&now));
    return std::string(buf);
}

// ----- Constructors -----
AppointmentRecord::AppointmentRecord()
    : service(), appointmentStatus("Scheduled"),
      apptType(AppointmentType::REGULAR), insuranceTier(InsuranceTier::NONE)
{
    appointmentId = generateId();
    dateTime      = currentDateTime();
}

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

    // Cancelled or Paid appointments – no charge
    if (appointmentStatus == "Cancelled" || appointmentStatus == "Paid") return 0.0;

    return total;
}

// ----- String helpers -----
std::string AppointmentRecord::tierToString(InsuranceTier t) {
    switch (t) {
        case InsuranceTier::NONE:     return "None";
        case InsuranceTier::BASIC:    return "Basic (15%)";
        case InsuranceTier::STANDARD: return "Standard (30%)";
        case InsuranceTier::PREMIUM:  return "Premium (50%)";
        default: return "Unknown";
    }
}

std::string AppointmentRecord::typeToString(AppointmentType t) {
    switch (t) {
        case AppointmentType::REGULAR:    return "Regular";
        case AppointmentType::EMERGENCY:  return "Emergency";
        case AppointmentType::FOLLOW_UP:  return "Follow-Up";
        default: return "Unknown";
    }
}

// ----- Display -----
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
