#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <QObject>
#include <vector>
#include <cstdint>
#include "ProcessInfo.h"

class SystemMonitor : public QObject {
    Q_OBJECT

public:
    explicit SystemMonitor(QObject *parent = nullptr);

    // System memory statistics
    uint64_t getTotalPhysicalRAM() const { return m_totalPhysicalRAM; }
    uint64_t getFreeMemory() const { return m_freeMemory; }
    uint64_t getActiveMemory() const { return m_activeMemory; }
    uint64_t getInactiveMemory() const { return m_inactiveMemory; }
    uint64_t getWiredMemory() const { return m_wiredMemory; }
    uint64_t getUsedMemory() const;

    // Process information
    const std::vector<ProcessInfo>& getProcesses() const { return m_processes; }
    std::vector<ProcessInfo> getTopProcessesByMemory(size_t count) const;

public slots:
    void collectData();

signals:
    void dataReady();
    void errorOccurred(const QString& error);

private:
    uint64_t m_totalPhysicalRAM;
    uint64_t m_freeMemory;
    uint64_t m_activeMemory;
    uint64_t m_inactiveMemory;
    uint64_t m_wiredMemory;
    std::vector<ProcessInfo> m_processes;

    bool collectSystemMemoryInfo();
    bool collectAllProcesses();
};

#endif // SYSTEMMONITOR_H
