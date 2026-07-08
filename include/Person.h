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
