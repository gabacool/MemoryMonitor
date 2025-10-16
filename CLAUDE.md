# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MemoryMonitor is a native macOS application built with C++17 and Qt 6 that monitors system memory usage in real-time. It displays process memory information in an interactive table and pie chart visualization.

## Build & Run Commands

### First-Time Setup
```bash
# Install dependencies
brew install cmake qt@6

# Add Qt to PATH (required for CMake to find Qt)
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
export CMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6
```

### Build
```bash
# Configure build (from project root)
cmake -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6

# Compile
cmake --build build

# Clean build (when needed)
rm -rf build && cmake -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 && cmake --build build
```

### Run
```bash
# Run directly from build directory
./build/MemoryMonitor.app/Contents/MacOS/MemoryMonitor

# Or open as macOS app
open build/MemoryMonitor.app
```

### Install
```bash
# Copy to Applications folder
cp -r build/MemoryMonitor.app /Applications/
open /Applications/MemoryMonitor.app
```

## Architecture

### Core Components

**SystemMonitor** (`src/SystemMonitor.cpp`, `include/SystemMonitor.h`)
- Collects system-wide memory statistics using macOS Mach APIs
- Uses `vm_statistics64` and `host_statistics64` for system memory info
- Retrieves total physical RAM via `sysctl` with `CTL_HW, HW_MEMSIZE`
- Should run in background thread to avoid blocking UI

**ProcessInfo** (`src/ProcessInfo.cpp`, `include/ProcessInfo.h`)
- Represents individual process memory data
- Collects per-process information using macOS libproc APIs:
  - `proc_listallpids()` - enumerate all running processes
  - `proc_pidinfo()` with `PROC_PIDTASKINFO` - get memory stats (resident size, virtual size)
  - `proc_pidpath()` - get full executable path
  - `proc_name()` - get process name
- Key metric: `pti_resident_size` from `proc_taskinfo` struct

**MainWindow** (`src/MainWindow.cpp`, `include/MainWindow.h`)
- Primary Qt GUI container
- Manages QTableWidget for process list (sortable, 4 columns)
- Manages Qt Charts QPieSeries for pie chart visualization
- Implements interactive features:
  - Click table row → highlight pie slice
  - Click pie slice → highlight table row
- Uses QTimer for auto-refresh (default 5 seconds)
- All UI updates must happen on main thread

### Threading Model

Data collection (SystemMonitor + ProcessInfo) must run in background thread:
```cpp
QThread *workerThread = new QThread;
monitor->moveToThread(workerThread);
connect(workerThread, &QThread::started, monitor, &SystemMonitor::collectData);
connect(monitor, &SystemMonitor::dataReady, this, &MainWindow::updateUI);
```

UI updates only on main thread via Qt signals/slots.

### Memory Calculation

```
RAM Usage (GB) = resident_size / (1024³)
Percentage = (resident_size / total_physical_ram) × 100
```

Use `pti_resident_size` from `proc_taskinfo` struct, NOT virtual memory.

## macOS-Specific APIs

### Required Headers
```cpp
#include <sys/sysctl.h>      // sysctl for total RAM
#include <libproc.h>          // proc_* functions
#include <mach/mach.h>        // Mach VM APIs
```

### Permission Considerations
- Some system processes may require elevated permissions
- Handle `EPERM` errors gracefully when querying process info
- Test on actual macOS (11.0+), not just in simulator

## Development Workflow

```bash
# 1. Make code changes in src/ or include/
# 2. Rebuild incrementally
cmake --build build

# 3. Test immediately
open build/MemoryMonitor.app

# 4. Check for build warnings (verbose mode)
cmake --build build --verbose
```

## Performance Requirements

- CPU usage < 1% when idle
- App memory footprint < 50 MB
- No UI lag during 5-second refresh cycles
- Filter processes using < 10 MB to reduce table size
- Cache process paths (they rarely change)

## Qt-Specific Patterns

### Table Setup
```cpp
tableWidget->setColumnCount(4);
tableWidget->setHorizontalHeaderLabels({"Process Name", "Path", "RAM Usage", "Percentage"});
tableWidget->setSortingEnabled(true);  // Allow column sorting
```

### Pie Chart Setup
```cpp
QPieSeries *series = new QPieSeries();
QPieSlice *slice = series->append(processName, memoryPercent);
slice->setLabelVisible(true);
```

### Auto-refresh Timer
```cpp
QTimer *refreshTimer = new QTimer(this);
connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshData);
refreshTimer->start(5000);  // milliseconds
```

## Common Issues

**Qt not found during CMake configuration**
- Ensure `CMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6` is set
- Check Qt installation: `brew list qt@6`

**Zombie processes or terminated PIDs**
- Wrap `proc_pidinfo()` calls in checks: `if (result == sizeof(ti))`
- Handle cases where process exits between enumeration and query

**UI freezing during refresh**
- Verify data collection is in background thread
- Never call blocking APIs on main/UI thread

**Memory leak detection**
- Use: `leaks MemoryMonitor` while app is running
- Follow RAII principles with smart pointers (`std::unique_ptr`, `std::shared_ptr`)

## Target Platform

- **macOS:** 11.0+ (Big Sur and later)
- **Architecture:** Universal binary (ARM64 + x86_64)
- **C++ Standard:** C++17
- **Qt Version:** Qt 6.x (requires Qt6::Core, Qt6::Widgets, Qt6::Charts)

## File Organization

Currently empty directories - implement files as needed:
- `src/` - Implementation files (.cpp)
- `include/` - Header files (.h)
- `resources/` - App icons, Info.plist template
- `build/` - Generated build artifacts (gitignored)

CMakeLists.txt expects these source files:
- `src/main.cpp`
- `src/MainWindow.cpp`
- `src/SystemMonitor.cpp`
- `src/ProcessInfo.cpp`
- `include/MainWindow.h`
- `include/SystemMonitor.h`
- `include/ProcessInfo.h`

## Testing

```bash
# Check memory stats with system tools
top -l 1 | grep -E "^(Processes|PhysMem)"
vm_stat

# Simulate high memory load (if stress tool available)
stress --vm 1 --vm-bytes 2G --vm-hang 60

# Monitor app's own memory usage
leaks MemoryMonitor
```

Test on multiple macOS versions (11, 12, 13, 14, 15) with varying system loads.
