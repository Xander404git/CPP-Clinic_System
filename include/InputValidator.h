#ifndef INPUTVALIDATOR_H
#define INPUTVALIDATOR_H

#include <iostream>
#include <string>
#include <limits>
#include <functional>
#include <sstream>

// ============================================================
// InputValidator.h – Custom Template Library
// Demonstrates advanced C++ template usage for type-safe,
// robust CLI input handling with custom validation rules.
// ============================================================

namespace Input {

    // Clears the input buffer to prevent infinite loops on bad input
    inline void clearBuffer() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Generic template function for validated input
    template <typename T>
    T getValidInput(const std::string& prompt, 
                    std::function<bool(const T&)> validator = [](const T&) { return true; }, 
                    const std::string& errorMsg = "  [!] Invalid input. Please try again.\n") 
    {
        T value;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value) {
                if (validator(value)) {
                    clearBuffer();
                    return value;
                }
            }
            std::cout << errorMsg;
            clearBuffer();
        }
    }

    // Specialization for std::string to handle spaces (getline)
    template <>
    inline std::string getValidInput<std::string>(const std::string& prompt, 
                                           std::function<bool(const std::string&)> validator, 
                                           const std::string& errorMsg) 
    {
        std::string value;
        while (true) {
            std::cout << prompt;
            std::getline(std::cin, value);
            if (validator(value)) {
                return value;
            }
            std::cout << errorMsg;
        }
    }
    
    // Convenience helper for non-empty strings
    inline std::string getNonEmptyString(const std::string& prompt) {
        return getValidInput<std::string>(
            prompt,
            [](const std::string& s) { return !s.empty(); },
            "  [!] Input cannot be empty.\n"
        );
    }

} // namespace Input

#endif // INPUTVALIDATOR_H
