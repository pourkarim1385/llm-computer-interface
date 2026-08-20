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
        Observation/Models/WorldState.cpp
        Observation/Models/WorldState.h
        Observation/Public.h
        Observation/Services/WorldStateBuilderService.cpp
        Observation/Services/WorldStateBuilderService.h
        Observation/Models/ScreenMetricsState.cpp
        Observation/Models/ScreenMetricsState.h
        Observation/Services/ScreenMetricsService.cpp
        Observation/Services/ScreenMetricsService.h
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
    pkg_check_modules(LINUX_DEPS REQUIRED atspi-2 gtk+-3.0 x11 xfixes)

    target_include_directories(AccessibilityService PRIVATE ${LINUX_DEPS_INCLUDE_DIRS})
    target_link_libraries(AccessibilityService PRIVATE ${LINUX_DEPS_LIBRARIES})
    target_compile_options(AccessibilityService PRIVATE ${LINUX_DEPS_CFLAGS_OTHER})
endif()