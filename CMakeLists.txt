cmake_minimum_required(VERSION 4.0)
project(AccessibilityService)

set(CMAKE_CXX_STANDARD 20)

add_executable(AccessibilityService main.cpp
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
        Observation/Services/FileContextExtractorService.h)
