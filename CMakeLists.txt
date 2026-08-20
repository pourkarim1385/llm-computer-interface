cmake_minimum_required(VERSION 3.20)
project(AccessibilityService LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
        main.cpp
        Observation/Models/AccessibilityState.cpp
        Observation/Models/AccessibilityState.h
        Observation/Services/AccessibilityService.cpp
        Observation/Services/AccessibilityService.h
        Observation/Models/ClipboardState.cpp
        Observation/Models/ClipboardState.h
        Observation/Services/CaptureClipboardService.cpp
        Observation/Services/CaptureClipboardService.h
        Observation/Models/FileContextState.cpp
        Observation/Models/FileContextState.h
        Observation/Services/FileContextExtractorService.cpp
        Observation/Services/FileContextExtractorService.h
        Observation/Services/CaptureService.hpp
        Observation/Services/CaptureService.cpp
        Observation/Models/WorldState.cpp
        Observation/Models/WorldState.h
        Observation/Public.h
        Observation/Services/WorldStateBuilderService.cpp
        Observation/Services/WorldStateBuilderService.h
        Observation/Models/ScreenMetricsState.cpp
        Observation/Models/ScreenMetricsState.h
        Observation/Services/ScreenMetricsService.cpp
        Observation/Services/ScreenMetricsService.h
        Observation/Models/DesktopState.hpp
        Observation/Models/DesktopState.cpp
        Observation/Services/DesktopService/BaseOsInfo.cpp
        Observation/Services/DesktopService/IdleTime.cpp
        Observation/Services/CaptureDesktopService.cpp
        Observation/Services/CaptureDesktopService.hpp
        Observation/Services/DesktopService/Connection.cpp
        Observation/Services/DesktopService/Connection.hpp
        Observation/Models/VisionState.hpp
        Observation/Models/VisionState.cpp
)

if(WIN32)
    list(APPEND SOURCES
        Observation/Services/CaptureServices/CaptureWin.hpp
        Observation/Services/CaptureServices/CaptureWin.cpp
        Observation/Services/DesktopService/ActiveAppWin.cpp
        Observation/Services/DesktopService/ActiveAppWin.hpp
        Observation/Services/DesktopService/CPURAMWin.cpp
        Observation/Services/DesktopService/CPURAMWin.hpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND SOURCES
        Observation/Services/CaptureServices/CaptureLinux.hpp
        Observation/Services/CaptureServices/CaptureLinux.cpp
        Observation/Services/DesktopService/ActiveAppLinux.cpp
        Observation/Services/DesktopService/ActiveAppLinux.hpp
        Observation/Services/DesktopService/CPURAMLinux.cpp
        Observation/Services/DesktopService/CPURAMLinux.hpp
    )
endif()

add_executable(AccessibilityService ${SOURCES})

# This line fixes the Windows MSVC include path errors by making the root folder globally accessible 
target_include_directories(AccessibilityService PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    target_link_libraries(AccessibilityService PRIVATE
            user32
            ole32
            oleaut32
    )
elseif(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)

    # Restored LINUX_DEPS because it is required for your ATSPI agent architecture
    # ADDED 'xrandr' to the end of this list to fix the XRRFreeOutputInfo linker error
    pkg_check_modules(LINUX_DEPS REQUIRED atspi-2 gtk+-3.0 x11 xfixes xrandr)
    pkg_check_modules(XSCRNSAVER REQUIRED xscrnsaver)

    target_link_libraries(AccessibilityService PRIVATE
            ${LINUX_DEPS_LIBRARIES}
            ${XSCRNSAVER_LIBRARIES}
    )

    target_include_directories(AccessibilityService PRIVATE
            ${LINUX_DEPS_INCLUDE_DIRS}
            ${XSCRNSAVER_INCLUDE_DIRS}
    )
    target_compile_options(AccessibilityService PRIVATE
            ${LINUX_DEPS_CFLAGS_OTHER}
    )
endif()