import os

files_to_include = [
    "../include/Person.h",
    "../include/Patient.h",
    "../include/Doctor.h",
    "../include/MedicalService.h",
    "../include/AppointmentRecord.h",
    "../include/HospitalSystem.h",
    "../include/InputValidator.h",
    "../src/Person.cpp",
    "../src/Patient.cpp",
    "../src/Doctor.cpp",
    "../src/MedicalService.cpp",
    "../src/AppointmentRecord.cpp",
    "../src/HospitalSystem.cpp",
    "../src/main.cpp"
]

report_path = "report_final.md"

with open(report_path, "a", encoding="utf-8") as f:
    f.write("\n\n---\n\n## 6. Appendix: Full Source Code\n\n")
    f.write("This appendix contains the complete, fully commented source code for the project, including citations for patterns adapted from online resources.\n\n")
    
    for relative_path in files_to_include:
        if os.path.exists(relative_path):
            f.write(f"### {relative_path.replace('../', '')}\n\n```cpp\n")
            with open(relative_path, "r", encoding="utf-8") as src_file:
                f.write(src_file.read())
            f.write("\n```\n\n")
        else:
            print(f"Warning: {relative_path} not found")

print("Appended source code to report_final.md successfully.")
