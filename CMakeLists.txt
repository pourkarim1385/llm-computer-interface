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
        Observation/Services/ImageCompare.hpp
        Observation/Services/ImageCompare.cpp
)

if(WIN32)
    list(APPEND SOURCES
            Observation/Services/CaptureServices/CaptureWin.hpp
            Observation/Services/CaptureServices/CaptureWin.cpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND SOURCES
            Observation/Services/CaptureServices/CaptureLinux.hpp
            Observation/Services/CaptureServices/CaptureLinux.cpp
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
    pkg_check_modules(ATSPI REQUIRED atspi-2)

    target_include_directories(AccessibilityService PRIVATE ${ATSPI_INCLUDE_DIRS})
    target_link_libraries(AccessibilityService PRIVATE ${ATSPI_LIBRARIES})
    target_compile_options(AccessibilityService PRIVATE ${ATSPI_CFLAGS_OTHER})
endif()