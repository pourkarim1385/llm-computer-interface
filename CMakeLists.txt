cmake_minimum_required(VERSION 3.20)
project(AccessibilityService LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_SOURCES
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
)


if(WIN32)
    set(PLATFORM_SOURCES
            Observation/Services/CaptureServices/CaptureWin.hpp
            Observation/Services/CaptureServices/CaptureWin.cpp
    )
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_SOURCES
            Observation/Services/CaptureServices/CaptureLinux.hpp
            Observation/Services/CaptureServices/CaptureLinux.cpp
    )
endif()

add_executable(AccessibilityService ${COMMON_SOURCES} ${PLATFORM_SOURCES})

if(WIN32)
    target_link_libraries(AccessibilityService PRIVATE
            user32
            ole32
            oleaut32
    )
endif()