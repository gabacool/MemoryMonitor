# Memory Monitor - macOS Application Requirements

## Project Overview
A native macOS application built with C++ that monitors system memory usage in real-time with an interactive GUI.

## Core Features

### 1. Process List Table
- **Columns:**
  - Process Name
  - Full Path
  - RAM Usage (in GB/MB)
  - RAM Usage Percentage (%)
- **Sorting:** Default sort by RAM usage (high to low)
- **Auto-refresh:** Every 5 seconds (user adjustable)
- **Interactive:** Click on row to highlight corresponding pie chart slice

### 2. Pie Chart Visualization
- **Display:** Visual representation of top memory consumers
- **Interactive:** Click on slice highlights corresponding row in table
- **Color-coded:** Different colors for each process
- **Labels:** Show process name and percentage on hover/click
- **Top N processes:** Show top 10-15 processes, group rest as "Others"

### 3. Refresh Controls
- **Default refresh rate:** 5 seconds
- **Adjustable:** User can set custom refresh interval (1-60 seconds)
- **Manual refresh:** Button to force immediate refresh
- **Pause/Resume:** Toggle to stop/start auto-refresh

### 4. Additional UI Elements
- **Total RAM:** Display total system RAM
- **Used RAM:** Display total used RAM
- **Free RAM:** Display available RAM
- **Memory pressure indicator:** Visual indicator (green/yellow/red)

## Technical Specifications

### Language & Framework
- **Primary Language:** C++17 or later
- **GUI Framework Options:**
  1. Qt (recommended) - cross-platform, excellent widgets
  2. wxWidgets - native macOS look and feel
  3. Cocoa with Objective-C++ bridge - fully native

### System Integration
- **Process Information:** Use macOS system calls
  - `proc_listallpids()` - list all process IDs
  - `proc_pidinfo()` - get process memory info
  - `task_info()` - detailed memory statistics
- **Memory Info:** Use `vm_statistics64` and `host_statistics64`

### Data Collection
```cpp
// Key APIs to use:
- kern_return_t host_statistics64()
- kern_return_t task_info()
- int proc_listallpids()
- int proc_pidinfo()
- int proc_pidpath()
```

### Memory Calculation
```
Memory % = (Process RSS / Total Physical RAM) × 100
```

## File Structure
```
MemoryMonitor/
├── src/
│   ├── main.cpp
│   ├── MemoryMonitor.h
│   ├── MemoryMonitor.cpp
│   ├── ProcessInfo.h
│   ├── ProcessInfo.cpp
│   ├── SystemMonitor.h
│   ├── SystemMonitor.cpp
│   ├── MainWindow.h
│   └── MainWindow.cpp
├── include/
│   └── (header files)
├── resources/
│   └── (app icons, etc.)
├── CMakeLists.txt
├── REQUIREMENTS.md
└── README.md
```

## Build System
- **CMake** for build configuration
- **Target:** macOS 11.0+ (Big Sur and later)
- **Architecture:** Universal binary (ARM64 + x86_64)

## Installation
- **Deploy to:** `/Applications/MemoryMonitor.app`
- **Bundle:** Create proper macOS .app bundle
- **Icon:** Include custom app icon
- **Launch:** Can be launched from Applications folder

## User Experience Requirements

### Performance
- Minimal CPU usage (< 1% when idle)
- Low memory footprint (< 50 MB)
- Smooth animations and transitions
- No UI lag during refresh

### Accessibility
- Resizable window (minimum 800x600)
- Dark mode support (follow system theme)
- Keyboard shortcuts for common actions
- Clear visual feedback for interactions

### Error Handling
- Graceful handling of permission errors
- Handle process termination during monitoring
- Recover from system API failures

## Keyboard Shortcuts
- `Cmd+R` - Manual refresh
- `Cmd+P` - Pause/Resume auto-refresh
- `Cmd+Q` - Quit application
- `Cmd+,` - Preferences/Settings

## Configuration
- Settings file: `~/Library/Preferences/com.memorymonitor.plist`
- Configurable options:
  - Refresh interval (seconds)
  - Number of processes in pie chart
  - Theme preference (light/dark/auto)
  - Launch at login option

## Optional Enhancements (Future)
- [ ] Export data to CSV
- [ ] Historical memory usage graphs
- [ ] Process filtering by name
- [ ] Alert notifications for high memory usage
- [ ] Menu bar widget showing top consumer
- [ ] Kill process functionality
- [ ] Memory usage trends over time

## Dependencies
- **Required:**
  - CMake 3.20+
  - C++17 compliant compiler (Clang on macOS)
  - macOS SDK 11.0+
- **GUI Framework:** Qt 6.x or wxWidgets 3.2+

## Testing Requirements
- Test on multiple macOS versions (11, 12, 13, 14, 15)
- Test with varying system loads
- Test memory leak prevention
- Verify accurate memory reporting

## Deliverables
1. Source code with proper documentation
2. CMakeLists.txt for building
3. Build instructions in README.md
4. macOS .app bundle
5. Optional: DMG installer

## Success Criteria
- ✅ Displays accurate memory usage
- ✅ Refreshes smoothly without lag
- ✅ Interactive chart-table synchronization
- ✅ Clean, native macOS appearance
- ✅ Stable and crash-free operation
- ✅ Easy installation to /Applications

## Notes for Implementation
- Use smart pointers (std::unique_ptr, std::shared_ptr)
- Follow RAII principles for resource management
- Thread-safe data collection in background thread
- Update UI only on main thread
- Implement proper memory management to avoid leaks
- Use modern C++ features (auto, lambdas, range-for)

## Developer Setup
1. Clone repository
2. Install dependencies (Qt/wxWidgets via Homebrew)
3. Run `cmake -B build`
4. Run `cmake --build build`
5. Run `./build/MemoryMonitor.app/Contents/MacOS/MemoryMonitor`

---

**Target Completion:** Full implementation with all core features
**Priority:** High performance, accurate data, intuitive UI
