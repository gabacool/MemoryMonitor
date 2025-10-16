# Memory Monitor

A native macOS application for monitoring system memory usage in real-time.

## Features
- 📊 Interactive table showing all processes with memory usage
- 🥧 Pie chart visualization of top memory consumers
- 🔄 Auto-refresh every 5 seconds (adjustable)
- 🎯 Click chart slice to highlight process in table
- 🎨 Native macOS appearance with dark mode support

## Quick Start

### Prerequisites
```bash
# Install Homebrew if not installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake qt@6
```

### Build
```bash
cd ~/Git/MemoryMonitor
cmake -B build
cmake --build build
```

### Run
```bash
./build/MemoryMonitor.app/Contents/MacOS/MemoryMonitor
```

### Install to Applications
```bash
cp -r build/MemoryMonitor.app /Applications/
```

## Project Structure
```
MemoryMonitor/
├── src/               # Source files
├── include/           # Header files
├── resources/         # Icons, assets
├── CMakeLists.txt     # Build configuration
├── REQUIREMENTS.md    # Detailed specifications
└── README.md          # This file
```

## Technology Stack
- **Language:** C++17
- **GUI:** Qt 6 (recommended) or wxWidgets
- **Build:** CMake
- **Platform:** macOS 11.0+

## Development

See `REQUIREMENTS.md` for complete technical specifications and implementation details.

## License
MIT

## Author
Created for monitoring macOS system memory usage
