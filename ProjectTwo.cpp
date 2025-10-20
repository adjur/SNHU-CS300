// ProjectTwo.cpp
// Andriana Djurdjevic
// CS 300 Analysis and Design
// 10/15/2024


#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct Course {
    std::string number;
    std::string title;
    std::vector<std::string> prereqs;
};

static inline void trimInPlace(std::string& s) {
    if (!s.empty() && s.back() == '\r') s.pop_back();
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    if (a != 0 || b != s.size()) s = s.substr(a, b - a);
}

static std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> parts;
    std::string item;
    std::stringstream ss(line);
    while (std::getline(ss, item, ',')) {
        trimInPlace(item);
        parts.push_back(item);
    }
    return parts;
}

// Return index of course by number, or -1 if not found
static int findCourseIndex(const std::vector<Course>& catalog, const std::string& num) {
    for (size_t i = 0; i < catalog.size(); ++i) {
        if (catalog[i].number == num) return static_cast<int>(i);
    }
    return -1;
}

static int readIntChoice() {
    std::string line;
    std::getline(std::cin, line);
    trimInPlace(line);
    if (line.empty()) return -1;
    for (char c : line) if (!std::isdigit(static_cast<unsigned char>(c))) return -1;
    try { return std::stoi(line); } catch (...) { return -1; }
}

class Catalog {
public:
    bool loadFromFile(const std::string& filename, std::string& msg) {
        std::ifstream in(filename);
        if (!in.is_open()) {
            msg = "Error: Could not open file \"" + filename + "\".";
            return false;
        }
        courses.clear();

        std::string line;
        size_t lineNo = 0;
        while (std::getline(in, line)) {
            ++lineNo;
            trimInPlace(line);
            if (line.empty()) continue;

            auto parts = splitCSV(line);
            if (parts.size() < 2) {
                std::cerr << "Warning (line " << lineNo << "): Expected number and title. Skipping.\n";
                continue;
            }

            Course c;
            c.number = parts[0];
            c.title  = parts[1];
            for (size_t i = 2; i < parts.size(); ++i) {
                if (!parts[i].empty()) c.prereqs.push_back(parts[i]);
            }
            if (c.number.empty()) {
                std::cerr << "Warning (line " << lineNo << "): Empty course number. Skipping.\n";
                continue;
            }

            int idx = findCourseIndex(courses, c.number);
            if (idx >= 0) {
                courses[idx] = std::move(c);
            } else {
                courses.push_back(std::move(c));
            }
        }

        std::sort(courses.begin(), courses.end(),
                  [](const Course& a, const Course& b) { return a.number < b.number; });

        size_t missing = 0;
        for (const auto& c : courses) {
            for (const auto& p : c.prereqs) {
                if (findCourseIndex(courses, p) < 0) {
                    ++missing;
                    std::cerr << "Warning: \"" << c.number << "\" lists missing prerequisite \"" << p << "\".\n";
                }
            }
        }

        msg = "Loaded " + std::to_string(courses.size()) + " course(s)"
            + (missing ? (" with " + std::to_string(missing) + " missing prerequisite reference(s)") : "")
            + ".";
        return true;
    }

    bool empty() const { return courses.empty(); }

    void printCourseList() const {
        if (courses.empty()) {
            std::cout << "(No courses loaded.)\n";
            return;
        }
        std::cout << "\n=== Course List (Alphanumeric) ===\n";
        for (const auto& c : courses) {
            std::cout << c.number << ", " << c.title << '\n';
        }
        std::cout << "==================================\n\n";
    }

    bool printCourseInfo(const std::string& number) const {
        int idx = findCourseIndex(courses, number);
        if (idx < 0) return false;

        const Course& c = courses[idx];
        std::cout << "\n" << c.number << ", " << c.title << "\n";
        if (c.prereqs.empty()) {
            std::cout << "Prerequisites: None\n\n";
        } else {
            std::cout << "Prerequisites:\n";
            for (const auto& pre : c.prereqs) {
                int pidx = findCourseIndex(courses, pre);
                if (pidx >= 0) {
                    std::cout << "  - " << pre << " (" << courses[pidx].title << ")\n";
                } else {
                    std::cout << "  - " << pre << " (title not found in file)\n";
                }
            }
            std::cout << '\n';
        }
        return true;
    }

private:
    std::vector<Course> courses;
};

int main() {
    Catalog catalog;
    bool running = true;

    std::cout << "ABCU Advising Assistance Program (CS 300)\n";
    std::cout << "-----------------------------------------\n";

    while (running) {
        std::cout << "Menu:\n"
                  << "  1. Load Data Structure\n"
                  << "  2. Print Course List\n"
                  << "  3. Print Course Information\n"
                  << "  9. Exit\n"
                  << "Enter your choice: ";

        int choice = readIntChoice();
        switch (choice) {
            case 1: {
                std::cout << "Enter the course data filename: ";
                std::string filename;
                std::getline(std::cin, filename);
                trimInPlace(filename);
                if (filename.empty()) {
                    std::cout << "Error: filename cannot be empty.\n\n";
                    break;
                }
                std::string msg;
                if (catalog.loadFromFile(filename, msg)) std::cout << msg << "\n\n";
                else                                      std::cout << msg << "\n\n";
                break;
            }
            case 2: {
                if (catalog.empty()) {
                    std::cout << "Error: Load the data structure first (Option 1).\n\n";
                    break;
                }
                catalog.printCourseList();
                break;
            }
            case 3: {
                if (catalog.empty()) {
                    std::cout << "Error: Load the data structure first (Option 1).\n\n";
                    break;
                }
                std::cout << "Enter a course number (e.g., CS200): ";
                std::string num;
                std::getline(std::cin, num);
                trimInPlace(num);
                if (num.empty()) {
                    std::cout << "Error: course number cannot be empty.\n\n";
                    break;
                }
                if (!catalog.printCourseInfo(num)) {
                    std::cout << "Error: course \"" << num << "\" not found.\n\n";
                }
                break;
            }
            case 9:
                std::cout << "Goodbye!\n";
                running = false;
                break;
            default:
                std::cout << "Invalid option. Please choose 1, 2, 3, or 9.\n\n";
                break;
        }
    }
    return 0;
}
