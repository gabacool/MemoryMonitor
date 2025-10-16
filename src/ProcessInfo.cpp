#include "ProcessInfo.h"
#include <libproc.h>
#include <sys/sysctl.h>
#include <cstring>

ProcessInfo::ProcessInfo()
    : m_pid(0)
    , m_name("")
    , m_path("")
    , m_residentSize(0)
    , m_virtualSize(0)
    , m_valid(false)
{
}

ProcessInfo::ProcessInfo(pid_t pid)
    : m_pid(pid)
    , m_name("")
    , m_path("")
    , m_residentSize(0)
    , m_virtualSize(0)
    , m_valid(false)
{
    update();
}

bool ProcessInfo::update() {
    if (m_pid <= 0) {
        m_valid = false;
        return false;
    }

    return collectProcessInfo();
}

bool ProcessInfo::collectProcessInfo() {
    // Get process task info (memory statistics)
    struct proc_taskinfo ti;
    int result = proc_pidinfo(m_pid, PROC_PIDTASKINFO, 0, &ti, sizeof(ti));

    if (result != sizeof(ti)) {
        // Process might have terminated or we don't have permission
        m_valid = false;
        return false;
    }

    m_residentSize = ti.pti_resident_size;
    m_virtualSize = ti.pti_virtual_size;

    // Get process path
    char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
    if (proc_pidpath(m_pid, pathBuffer, sizeof(pathBuffer)) > 0) {
        m_path = pathBuffer;

        // Extract process name from path
        const char *lastSlash = strrchr(pathBuffer, '/');
        if (lastSlash) {
            m_name = lastSlash + 1;
        } else {
            m_name = pathBuffer;
        }
    } else {
        // Fallback: try proc_name
        char nameBuffer[256];
        if (proc_name(m_pid, nameBuffer, sizeof(nameBuffer)) > 0) {
            m_name = nameBuffer;
        } else {
            m_name = "Unknown";
        }
        m_path = "";
    }

    m_valid = true;
    return true;
}

double ProcessInfo::getMemoryUsageGB() const {
    return static_cast<double>(m_residentSize) / (1024.0 * 1024.0 * 1024.0);
}

double ProcessInfo::getMemoryPercentage(uint64_t totalPhysicalRAM) const {
    if (totalPhysicalRAM == 0) {
        return 0.0;
    }
    return (static_cast<double>(m_residentSize) / static_cast<double>(totalPhysicalRAM)) * 100.0;
}
