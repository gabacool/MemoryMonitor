# Quick Start Guide

## Setup (One-time)

```bash
# Navigate to project
cd ~/Git/MemoryMonitor

# Install dependencies
brew install cmake qt@6

# Add Qt to PATH (add this to ~/.zshrc for persistence)
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
```

## Build

```bash
# Configure
cmake -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6

# Compile
cmake --build build
```

## Run

```bash
# Run directly
./build/MemoryMonitor.app/Contents/MacOS/MemoryMonitor

# Or open as macOS app
open build/MemoryMonitor.app
```

## Install to Applications

```bash
# Copy to Applications folder
cp -r build/MemoryMonitor.app /Applications/

# Run from Applications
open /Applications/MemoryMonitor.app
```

## Clean Build

```bash
rm -rf build
cmake -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6
cmake --build build
```

## Development Workflow

```bash
# 1. Make code changes
# 2. Rebuild
cmake --build build

# 3. Test
open build/MemoryMonitor.app
```

## Troubleshooting

### Qt not found
```bash
# Check Qt installation
brew list qt@6

# Set Qt path explicitly
export CMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6
```

### Build errors
```bash
# Clean and rebuild
rm -rf build
cmake -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6
cmake --build build --verbose
```

### Permission issues
```bash
# Some memory APIs may require additional permissions
# Check Console.app for any security/permission logs
```

## Next Steps

1. Read `REQUIREMENTS.md` for full specifications
2. Check `NOTES.md` for implementation examples
3. Start implementing in `src/` directory
4. Build and test frequently
