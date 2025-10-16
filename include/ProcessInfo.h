#ifndef PROCESSINFO_H
#define PROCESSINFO_H

#include <string>
#include <cstdint>
#include <sys/types.h>

class ProcessInfo {
public:
    ProcessInfo();
    ProcessInfo(pid_t pid);

    // Getters
    pid_t getPid() const { return m_pid; }
    std::string getName() const { return m_name; }
    std::string getPath() const { return m_path; }
    uint64_t getResidentSize() const { return m_residentSize; }
    uint64_t getVirtualSize() const { return m_virtualSize; }
    double getMemoryUsageGB() const;
    double getMemoryPercentage(uint64_t totalPhysicalRAM) const;

    // Update process information from system
    bool update();

    // Validation
    bool isValid() const { return m_valid; }

private:
    pid_t m_pid;
    std::string m_name;
    std::string m_path;
    uint64_t m_residentSize;  // Resident memory in bytes
    uint64_t m_virtualSize;   // Virtual memory in bytes
    bool m_valid;

    bool collectProcessInfo();
};

#endif // PROCESSINFO_H
