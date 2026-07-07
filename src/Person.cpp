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
