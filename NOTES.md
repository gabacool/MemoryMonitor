# Implementation Notes for Memory Monitor

## macOS System APIs for Memory Monitoring

### Getting Process List
```cpp
#include <sys/sysctl.h>
#include <libproc.h>

// Get all PIDs
int num_procs = proc_listallpids(nullptr, 0);
std::vector<pid_t> pids(num_procs);
proc_listallpids(pids.data(), num_procs * sizeof(pid_t));
```

### Getting Process Memory Info
```cpp
#include <libproc.h>

struct proc_taskinfo ti;
int result = proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &ti, sizeof(ti));

if (result == sizeof(ti)) {
    uint64_t rss_bytes = ti.pti_resident_size;  // Resident memory
    uint64_t vsize_bytes = ti.pti_virtual_size; // Virtual memory
}
```

### Getting Process Name and Path
```cpp
char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
proc_pidpath(pid, pathbuf, sizeof(pathbuf));

char namebuf[256];
proc_name(pid, namebuf, sizeof(namebuf));
```

### Getting System Memory Stats
```cpp
#include <mach/mach.h>

vm_size_t page_size;
host_page_size(mach_host_self(), &page_size);

vm_statistics64_data_t vm_stats;
mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
host_statistics64(mach_host_self(), HOST_VM_INFO64, 
                  (host_info64_t)&vm_stats, &count);

uint64_t free_mem = vm_stats.free_count * page_size;
uint64_t active_mem = vm_stats.active_count * page_size;
uint64_t inactive_mem = vm_stats.inactive_count * page_size;
uint64_t wired_mem = vm_stats.wire_count * page_size;
```

### Getting Total Physical RAM
```cpp
#include <sys/sysctl.h>

int mib[2] = {CTL_HW, HW_MEMSIZE};
uint64_t total_memory;
size_t length = sizeof(total_memory);
sysctl(mib, 2, &total_memory, &length, nullptr, 0);
```

## Qt Implementation Tips

### Table Widget Setup
```cpp
tableWidget->setColumnCount(4);
tableWidget->setHorizontalHeaderLabels(
    {"Process Name", "Path", "RAM Usage", "Percentage"});
tableWidget->setSortingEnabled(true);
tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
```

### Pie Chart Setup
```cpp
QPieSeries *series = new QPieSeries();
for (const auto& proc : topProcesses) {
    QPieSlice *slice = series->append(proc.name, proc.memoryPercent);
    slice->setLabelVisible(true);
}
QChart *chart = new QChart();
chart->addSeries(series);
```

### Timer for Auto-Refresh
```cpp
QTimer *refreshTimer = new QTimer(this);
connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshData);
refreshTimer->start(5000); // 5 seconds
```

## Threading Considerations

Data collection should be done in a background thread to avoid UI freezing:

```cpp
QThread *workerThread = new QThread;
SystemMonitor *monitor = new SystemMonitor;
monitor->moveToThread(workerThread);

connect(workerThread, &QThread::started, monitor, &SystemMonitor::collectData);
connect(monitor, &SystemMonitor::dataReady, this, &MainWindow::updateUI);

workerThread->start();
```

## Memory Calculation Formulas

```
RAM Usage (GB) = resident_size / (1024³)
Percentage = (resident_size / total_physical_ram) × 100
```

## Performance Optimization

1. **Cache process paths** - they rarely change
2. **Filter processes** - only show processes using > 10MB
3. **Limit table rows** - show top 100 processes max
4. **Throttle updates** - don't update more than once per second
5. **Use move semantics** - avoid copying large data structures

## Common Pitfalls

- **Permission errors**: Some system processes require root access
- **Zombie processes**: Handle cases where process terminates during query
- **Memory units**: Be consistent (bytes vs KB vs MB vs GB)
- **UI thread blocking**: Always collect data in background thread

## Testing Commands

```bash
# Simulate high memory usage
stress --vm 1 --vm-bytes 2G --vm-hang 60

# Check actual memory with system tools
top -l 1 | grep -E "^(Processes|PhysMem)"
vm_stat

# Monitor your app's memory
leaks MemoryMonitor
```

## Helpful Links

- [Apple Process Info API](https://developer.apple.com/library/archive/qa/qa1123/_index.html)
- [Mach Virtual Memory](https://developer.apple.com/documentation/kernel/1537998-host_statistics64)
- [Qt Charts Documentation](https://doc.qt.io/qt-6/qtcharts-index.html)
