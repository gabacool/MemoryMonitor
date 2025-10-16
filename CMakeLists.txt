cmake_minimum_required(VERSION 3.20)
project(MemoryMonitor VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS specific settings
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
# Build for native architecture only (Qt from Homebrew is architecture-specific)
set(CMAKE_OSX_ARCHITECTURES "${CMAKE_SYSTEM_PROCESSOR}")

# Qt Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SystemMonitor.cpp
    src/ProcessInfo.cpp
)

# Header files
set(HEADERS
    include/MainWindow.h
    include/SystemMonitor.h
    include/ProcessInfo.h
)

# Resources
set(RESOURCES
    resources/resources.qrc
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Create executable
add_executable(${PROJECT_NAME} MACOSX_BUNDLE
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
)

# macOS Bundle properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    BUNDLE True
    MACOSX_BUNDLE_GUI_IDENTIFIER com.memorymonitor.app
    MACOSX_BUNDLE_BUNDLE_NAME "Memory Monitor"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/Info.plist.in
)

# Copy resources to bundle
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/resources/MemoryMonitor.icns
    $<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/MemoryMonitor.icns
)
