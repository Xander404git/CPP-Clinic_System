# XIAMEN UNIVERSITY MALAYSIA
### Faculty of Engineering and Technology

**CST209 – Object-Oriented Programming C++**
**FINAL PROJECT REPORT**
**Hospital Patient & Appointment Management System**

---

## 1. Project Overview

This report documents the design and implementation of a Hospital Patient and Appointment Management System developed in C++ using Object-Oriented Programming (OOP) principles. The system allows hospital staff to manage patient registrations, doctor profiles, medical service catalogues, appointment scheduling, and billing records through an interactive console interface.

The project demonstrates the following core OOP concepts:

- **Abstraction and Encapsulation** – data hidden inside classes with well-defined public interfaces
- **Inheritance** – Patient and Doctor derive from the abstract base class Person
- **Composition** – AppointmentRecord contains a MedicalService object
- **Runtime Polymorphism** – virtual dispatch via `std::vector<Person*>`
- **Compile-time Polymorphism** – overloaded constructors and `operator==`
- **STL containers** – `std::vector` and `std::map` for efficient storage and retrieval
- **Custom exception hierarchy** – HospitalException, DuplicateIdException, NotFoundException
- **Static members** – auto-generated IDs and population counters
- **Templates** – custom InputValidator library for type-safe CLI handling
- **File I/O** – robust persistent state serialisation and deserialisation to plain text

### 1.1 File Structure

The project uses separate header and implementation files for each class, following professional multi-file project conventions:

```
hospital_project/
├── Makefile
├── include/
│   ├── MedicalService.h
│   ├── AppointmentRecord.h
│   ├── Person.h
│   ├── Patient.h
│   ├── Doctor.h
│   ├── HospitalSystem.h
│   └── InputValidator.h
├── src/
│   ├── MedicalService.cpp
│   ├── AppointmentRecord.cpp
│   ├── Person.cpp
│   ├── Patient.cpp
│   ├── Doctor.cpp
│   ├── HospitalSystem.cpp
│   └── main.cpp
```

---

## 2. Class Design

### 2.1 Class Hierarchy

The system is built around three primary entity types: Users, Medical Services, and Appointment Records. The following table summarises all classes and their OOP roles:

### 2.2 Composition vs Aggregation Relationships

The system rigorously distinguishes between composition (strong ownership) and aggregation (weak reference) to ensure safe memory management and entity lifecycles.

**Composition:** AppointmentRecord demonstrates composition. The MedicalService object is embedded directly (by value) inside each AppointmentRecord instance. This means the lifetime of the MedicalService is strictly tied to the AppointmentRecord that contains it — when the record is destroyed, the embedded service copy is also destroyed. Similarly, Patient owns a `std::vector<AppointmentRecord>` by composition; deleting a Patient automatically destroys their appointment history.

**Aggregation:** The relationship between Doctor and Patient is aggregation. A Doctor holds a vector of assignedPatientIds (strings), and an AppointmentRecord holds a doctorId (string). These are weak references. If a Patient is deleted from the system, they are not strictly "owned" by the Doctor.

To prevent dangling references, the `HospitalSystem::removeUser()` method explicitly cleans up aggregation relationships:

- If a Patient is removed, their ID is scrubbed from every Doctor's assigned list.
- If a Doctor is removed, their ID is cleared from all Patient appointments, but the appointment record itself (owned by the Patient via composition) survives intact.

This design prevents memory leaks and system crashes caused by referencing deleted objects.

---

## 3. Part II Documentation – Q1: STL Explanation

The C++ Standard Template Library (STL) provides a collection of generic containers, algorithms, and iterators. Rather than implementing our own data structures from scratch — which would introduce bugs, reduce readability, and duplicate already-tested functionality — we leverage two STL containers that are structurally well-suited to this hospital context.

### 3.1 `std::vector<Person*>` – Dynamic Polymorphic Container

**Why a vector?** The hospital system must store both Patient and Doctor objects in a single unified list — for example, when displaying all registered users or counting total occupancy. Because both types inherit from Person, they can be stored as `Person*` pointers in a single `std::vector<Person*>`. This design directly enables runtime polymorphism: when we call `p->displayRole()` for each element, C++ resolves the correct override at runtime via the virtual table (vtable).

**Real-life analogy:** A hospital's central staff registry does not maintain two separate, completely disconnected databases for doctors and patients — it maintains a single registry of "persons" in the system, differentiated by role. `std::vector` mirrors this directly. A `std::list` would provide faster insertions in the middle, but random access (e.g. finding the third user) would be O(n). A `std::set` would avoid duplicates but would require a comparator and does not naturally model ordered registration. For a system where users are registered in sequence and primarily accessed by iterating the full list (to display all users) or searched by ID, `std::vector` is the most structurally appropriate choice.

```cpp
std::vector<Person*> users;
// Runtime polymorphism – correct displayRole() called for each type:
for (const Person* p : users)
    p->displayRole();   // Doctor::displayRole() or Patient::displayRole()
```

### 3.2 `std::map<std::string, MedicalService>` – Ordered Key-Value Lookup

**Why a map?** Medical services need to be looked up frequently and efficiently by their unique service code (e.g. "CONS01", "SURG01"). `std::map` provides O(log n) lookup using a balanced binary search tree internally. Every time a patient schedules an appointment, the system retrieves the matching MedicalService using its code — this must be fast and unambiguous.

**Real-life analogy:** Think of a hospital's service billing code catalogue (similar to ICD-10 or CPT medical billing codes used in real hospitals). A doctor selects a billing code, and the system instantly retrieves the procedure name and base fee. A sequential search through a vector would be O(n) — acceptable for a dozen services but unacceptable in a real hospital system with hundreds or thousands of service codes. `std::map` also automatically prevents duplicate service codes (inserting the same key again is a no-op), which enforces data integrity.

```cpp
std::map<std::string, MedicalService> servicesMap;
// O(log n) lookup by service code:
auto it = servicesMap.find("CONS01");
if (it != servicesMap.end()) { /* service found */ }
```

### 3.3 `std::vector<AppointmentRecord>` – Per-Patient History

Each Patient object maintains its own `std::vector<AppointmentRecord>` to store that patient's appointment history. Because AppointmentRecord is a value type (not a pointer), the vector manages the lifetime of all records automatically — no manual memory allocation is needed. Iterating over all appointments to calculate the total bill is O(n) with a range-based for loop, which is idiomatic and clear. Using `std::vector` here is appropriate because: (a) appointments are added at the end (amortised O(1) `push_back`), (b) they are iterated in order, and (c) the number of appointments per patient is typically small (single digits to tens).

### 3.4 Custom Template Library – InputValidator.h

In addition to the Standard Template Library, the system implements a self-developed generic template library enclosed in the `Input` namespace: `InputValidator.h`. This library contains the primary `template <typename T> getValidInput(prompt, validator, errorMsg)` function which accepts:

- A prompt string.
- A `std::function<bool(const T&)>` lambda validator for custom validation rules.
- A custom error message string.

The library includes a full template specialisation for `std::string` which uses `std::getline()` instead of `std::cin >>`, correctly capturing inputs that contain spaces. A convenience wrapper `getNonEmptyString()` further simplifies the most common validation requirement.

Why custom templates? Command-line interfaces in C++ are notorious for entering infinite loops when `std::cin` encounters mismatched data types (e.g. typing a string into an integer prompt). By developing this custom generic template, the system handles any primitive type (int, double, string) with custom validation logic and automatic buffer clearing on invalid input — all without duplicating input-handling code for every prompt.

---

## 4. Part II Documentation – Q2: Inheritance & Polymorphism

### 4.1 Why Person Should Be an Abstract Base Class

In a real hospital, every entity that interacts with the system — whether a patient receiving treatment or a doctor delivering it — is fundamentally a "person" with shared attributes: a name, an ID number, contact number, email, date of birth, and gender. Defining a Person abstract base class captures this shared identity once, in one place, and forces all derived classes to implement a `displayRole()` function appropriate to their specific role in the system.

The critical design decision is making `displayRole()` a pure virtual function (`= 0`). In fact, Person declares two pure virtual functions:

- `virtual void displayRole() const = 0;`
- `virtual std::string getType() const = 0;`

This achieves three goals:

1. It makes Person an Abstract Base Class (ABC), preventing any code from accidentally creating a generic "Person" object — a person in a hospital is always specifically a patient or a doctor, never an undefined entity.
2. It enforces a contract: any concrete class that inherits from Person must provide its own implementation of both pure virtuals. If it does not, the compiler will refuse to compile, catching design errors early.
3. It enables runtime polymorphism — a pointer of type `Person*` can point to either a Patient or a Doctor, and the correct `displayRole()` is dispatched at runtime based on the actual type of the object, not the type of the pointer.

```cpp
// Person.h – Two pure virtual functions make this class abstract
virtual void displayRole() const = 0;
virtual std::string getType() const = 0;
```

**Real-life parallel:** In a Malaysian hospital's information system (such as those used at Hospital Kuala Lumpur or private hospitals running systems like iHIS), the underlying patient-doctor record distinction is exactly this pattern. A "Person" record holds MyKad/passport number, date of birth, and contact details, while a "PatientVisit" record and a "ClinicalStaff" record each extend it with domain-specific fields. The OOP concept directly models this administrative reality.

### 4.2 Why Patient and Doctor Inherit from Person

Both Patient and Doctor share identity attributes (name, ID, contact, email, DOB, gender) — duplicating these fields in two separate, unrelated classes would violate the DRY (Don't Repeat Yourself) principle, and any change to how names are stored (e.g. adding a title field) would have to be made in two places, risking inconsistency. Inheritance solves this cleanly.

Beyond code reuse, inheritance models the real-world semantic relationship: a patient IS-A person, and a doctor IS-A person. This is not just a coding convenience — it reflects domain truth. The relationship also makes the polymorphic container (`vector<Person*>`) possible: because both types share the same base, they can be stored together and treated uniformly when displaying the full user list.

### 4.3 How Runtime Polymorphism Is Achieved

Runtime polymorphism (dynamic dispatch) is achieved through three cooperating mechanisms in C++:

1. **Virtual functions:** the base class Person declares `displayRole()` as virtual. This tells the compiler to resolve calls to `displayRole()` through the object's vtable at runtime, not at compile time.
2. **Pure virtual (`= 0`):** forces derived classes to override. Without this, a derived class could forget to implement `displayRole()` and silently inherit a no-op base version.
3. **Base-class pointers:** by storing `Person*` pointers (not Patient or Doctor values) in the vector, we preserve the runtime type information. When `p->displayRole()` is called, C++ looks up the actual type of the object `*p` (via the vtable pointer) and dispatches to the correct override.

```cpp
// HospitalSystem.cpp – viewAllUsers()
for (const Person* p : users)
    p->displayRole();   // dispatches to Patient::displayRole() or Doctor::displayRole()
```

Compile-time polymorphism is also demonstrated through overloaded constructors in MedicalService. Three constructors accept different argument combinations: a default constructor, a two-parameter constructor (defaulting baseFee to 50.0, the standard consultation rate), and a five-parameter full constructor. The compiler selects the correct constructor at compile time based on the argument types and count — no virtual table is needed.

Additionally, the `==` operator is overloaded on MedicalService to allow natural equality comparisons by service code, further demonstrating compile-time polymorphism through operator overloading.

---

## 5. Part II Documentation – Q3: Static Functions & Additional Features

### 5.1 Static Members – Rationale and Implementation

Static member functions and variables belong to the class itself, not to any individual object instance. This makes them the correct tool for tracking system-wide state — information that is inherently shared across all instances of a class.

In this system, static members serve three purposes:

#### 5.1.1 Auto-incremented ID Generation

Each class (Patient, Doctor, AppointmentRecord) uses a private static counter to generate unique, auto-incremented IDs. This mirrors how real hospital information systems work: a Malaysian hospital's HIS (Hospital Information System) assigns a unique MRN (Medical Record Number) to each patient on registration — the number is generated centrally, not by the patient themselves.

```cpp
// Patient.h / Patient.cpp
static int nextPatientNumber;   // shared across all Patient instances

static std::string generatePatientId() {
    std::ostringstream oss;
    oss << "PAT" << (++nextPatientNumber);
    return oss.str();   // e.g. PAT1001, PAT1002, ...
}
```

The benefit of using a static function here is that `generatePatientId()` can be called without creating a Patient object first — you call it to get an ID before constructing the patient. This is idiomatic in C++ for factory-style ID generation.

#### 5.1.2 Population Counting

Static int counters (patientCount, doctorCount, totalPersonCount) track how many objects of each type are alive at any moment. The constructor increments the counter; the destructor decrements it. This allows the system to accurately report how many patients and doctors are currently registered, even as users are added or removed during runtime.

```cpp
// Person.cpp
int Person::totalPersonCount = 0;
// Only the parameterised constructor increments the count;
// the default constructor does not, as it is used for internal initialisation only.
Person::Person(id, name, ...)  { ++totalPersonCount; }
Person::~Person()              { --totalPersonCount; }
```

**Real-life value:** In a hospital with a rolling census (number of in-patients currently admitted), tracking this count statically is exactly how the system would work — a single authoritative counter, not computed by querying and counting all records each time.

### 5.2 Additional Advanced Features

#### 5.2.1 Custom Exception Hierarchy

Rather than using generic `std::runtime_error` messages, the system defines a three-level exception hierarchy: HospitalException (base), DuplicateIdException (thrown when a patient/doctor with an existing ID is added), and NotFoundException (thrown when a lookup fails). This mirrors professional exception design in enterprise software.

**Real-life motivation:** A hospital management system crashing with an unhandled exception when a nurse accidentally types an existing patient ID is unacceptable. By catching specific exception types at the menu level (in `main()`), the system can display a clear, human-readable error message — "Duplicate ID: 'PAT1001' already exists in the system." — and return the user to the menu instead of terminating. This is exactly how production hospital software handles data entry errors.

```cpp
try {
    hs.addPatient(p);   // throws DuplicateIdException if ID exists
} catch (const HospitalException& e) {
    std::cout << "  [ERROR] " << e.what() << "\n";
}
```

#### 5.2.2 Billing Modifier Engine

The `calculateTotalBilling()` method in AppointmentRecord implements a layered billing modifier system: the base fee from MedicalService is first adjusted by appointment type (Emergency adds 30%, Follow-Up deducts 30%), then adjusted by insurance tier (Basic covers 15%, Standard 30%, Premium 50%). Finally, if the appointment status is "Cancelled", the method returns 0.0 regardless of calculated modifiers. This directly models how Malaysian private hospital billing works — Malaysian Medical Council guidelines allow emergency surcharges, and insurance panels negotiate coverage percentages.

#### 5.2.3 Enum Classes for Type Safety

Instead of using magic integers or strings to represent appointment status, type, or insurance tier, the system uses `enum class` (scoped enumerations). This provides compile-time type safety — a function that expects an InsuranceTier cannot accidentally receive an AppointmentType — and improves readability. Static helper methods `AppointmentRecord::tierToString()` and `AppointmentRecord::typeToString()` convert enum values to display-friendly strings.

```cpp
enum class InsuranceTier { NONE, BASIC, STANDARD, PREMIUM };
enum class AppointmentType { REGULAR, EMERGENCY, FOLLOW_UP };
```

#### 5.2.4 Memory Management and Virtual Destructor

All Person objects are allocated on the heap with `new` and stored as `Person*` pointers in the vector. When HospitalSystem is destroyed, its destructor iterates the vector and calls `delete` for each pointer. Because Person declares a virtual destructor, `delete p` will correctly call the destructor of the actual derived type (Patient or Doctor) — not just Person's destructor. Without the virtual destructor, derived-class destructors would never run, causing a memory leak. This is a critical and commonly overlooked requirement in C++ polymorphism.

```cpp
// Person.h
virtual ~Person();   // virtual destructor – required for polymorphic base classes

// HospitalSystem.cpp – destructor
HospitalSystem::~HospitalSystem() {
    for (Person* p : users) delete p;   // correct derived destructor called
    users.clear();
}
```

#### 5.2.5 File Persistence

The `saveToFile()` and `loadFromFile()` methods provide robust data persistence. They serialise the entire system state — including nested composition objects (like a patient's appointment history) and complex vector data (like a doctor's qualifications or a patient's allergies) — into a pipe-delimited plain text file using `std::ofstream`. The `loadFromFile()` method safely parses this custom format using a string-splitting lambda and `std::ifstream`, reconstructing the exact object hierarchy and re-linking aggregation IDs. This demonstrates advanced STL file I/O and ensures the system state can survive between program runs.

#### 5.2.6 Standardised Build System

To ensure "Quality of Implementation" for a multi-file source project, the system includes a standard GNU Makefile. This abstracts away the complexity of manual g++ compilation flags, allowing the entire hospital system to be compiled efficiently by running a single `make` command. It intelligently compiles only modified `.cpp` files into object files before linking, which is an industry-standard best practice for C++ projects.

---

## 6. References

The following references informed the design and implementation of this project:

- Stroustrup, B. (2013). *The C++ Programming Language* (4th ed.). Addison-Wesley. – Core C++ OOP concepts, virtual functions, abstract classes, and memory management.
- Lippman, S. B., Lajoie, J., & Moo, B. E. (2012). *C++ Primer* (5th ed.). Addison-Wesley. – Inheritance, polymorphism, STL containers (Chapter 13–15).
- cppreference.com. (2024). *std::vector – C++ Reference.* https://en.cppreference.com/w/cpp/container/vector
- cppreference.com. (2024). *std::map – C++ Reference.* https://en.cppreference.com/w/cpp/container/map
- cppreference.com. (2024). *std::exception hierarchy.* https://en.cppreference.com/w/cpp/error/exception
- Meyers, S. (2005). *Effective C++* (3rd ed.). Addison-Wesley. – Item 7: Declare destructors virtual in polymorphic base classes.
- Malaysian Medical Council. (2022). *Guidelines on Fees for Private Medical Practice.* https://www.mmc.gov.my
- Ministry of Health Malaysia. (2023). *Hospital Information System (HIS) Standards.* https://www.moh.gov.my
