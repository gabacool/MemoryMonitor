#include "SystemMonitor.h"
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <libproc.h>
#include <algorithm>
#include <QDebug>

SystemMonitor::SystemMonitor(QObject *parent)
    : QObject(parent)
    , m_totalPhysicalRAM(0)
    , m_freeMemory(0)
    , m_activeMemory(0)
    , m_inactiveMemory(0)
    , m_wiredMemory(0)
{
    // Get total physical RAM (this doesn't change)
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    size_t length = sizeof(m_totalPhysicalRAM);
    sysctl(mib, 2, &m_totalPhysicalRAM, &length, nullptr, 0);
}

void SystemMonitor::collectData() {
    bool success = collectSystemMemoryInfo();
    if (!success) {
        emit errorOccurred("Failed to collect system memory information");
        return;
    }

    success = collectAllProcesses();
    if (!success) {
        emit errorOccurred("Failed to collect process information");
        return;
    }

    emit dataReady();
}

bool SystemMonitor::collectSystemMemoryInfo() {
    vm_size_t page_size;
    host_page_size(mach_host_self(), &page_size);

    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

    kern_return_t result = host_statistics64(
        mach_host_self(),
        HOST_VM_INFO64,
        (host_info64_t)&vm_stats,
        &count
    );

    if (result != KERN_SUCCESS) {
        return false;
    }

    m_freeMemory = vm_stats.free_count * page_size;
    m_activeMemory = vm_stats.active_count * page_size;
    m_inactiveMemory = vm_stats.inactive_count * page_size;
    m_wiredMemory = vm_stats.wire_count * page_size;

    return true;
}

bool SystemMonitor::collectAllProcesses() {
    // Get number of processes
    int numProcs = proc_listallpids(nullptr, 0);
    if (numProcs <= 0) {
        return false;
    }

    // Get all process IDs
    std::vector<pid_t> pids(numProcs);
    numProcs = proc_listallpids(pids.data(), static_cast<int>(pids.size() * sizeof(pid_t)));

    if (numProcs <= 0) {
        return false;
    }

    // Resize to actual number of processes
    pids.resize(numProcs);

    // Clear previous process list
    m_processes.clear();
    m_processes.reserve(numProcs);

    // Collect information for each process
    for (pid_t pid : pids) {
        ProcessInfo procInfo(pid);
        if (procInfo.isValid()) {
            // Show ALL processes (no filtering)
            m_processes.push_back(std::move(procInfo));
        }
    }

    // Sort by resident memory size (descending)
    std::sort(m_processes.begin(), m_processes.end(),
        [](const ProcessInfo& a, const ProcessInfo& b) {
            return a.getResidentSize() > b.getResidentSize();
        });

    return true;
}

uint64_t SystemMonitor::getUsedMemory() const {
    // Used memory = Active + Wired + Inactive
    // (Inactive is cached but still occupies RAM)
    return m_activeMemory + m_wiredMemory + m_inactiveMemory;
}

std::vector<ProcessInfo> SystemMonitor::getTopProcessesByMemory(size_t count) const {
    std::vector<ProcessInfo> topProcesses;

    size_t numToReturn = std::min(count, m_processes.size());
    topProcesses.reserve(numToReturn);

    for (size_t i = 0; i < numToReturn; ++i) {
        topProcesses.push_back(m_processes[i]);
    }

    return topProcesses;
}
