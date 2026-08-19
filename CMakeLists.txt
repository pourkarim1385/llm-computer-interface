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
        Observation/Models/DesktopState.hpp
        Observation/Models/DesktopState.cpp
        Observation/Services/DesktopService/BaseOsInfo.cpp
        Observation/Services/DesktopService/BaseOsInfo.cpp
        Observation/Services/DesktopService/IdleTime.cpp
        Observation/Services/DesktopService/IdleTime.cpp
        Observation/Services/CaptureDesktopService.cpp
        Observation/Services/CaptureDesktopService.hpp
        Observation/Public.h
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

if(WIN32)
    target_link_libraries(AccessibilityService PRIVATE
            user32
            ole32
            oleaut32
    )
elseif(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XSCRNSAVER REQUIRED xscrnsaver)

    target_link_libraries(AccessibilityService PRIVATE
        ${LINUX_DEPS_LIBRARIES}
        ${XSCRNSAVER_LIBRARIES}
    )
    target_include_directories(AccessibilityService PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${LINUX_DEPS_INCLUDE_DIRS}
        ${XSCRNSAVER_INCLUDE_DIRS}
    )
endif()
