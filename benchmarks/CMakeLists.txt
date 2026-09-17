# Collect all Observation sources
set(OBSERVATION_CORE_SOURCES
        ${CMAKE_SOURCE_DIR}/Observation/Models/AccessibilityState.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Models/ClipboardState.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Models/DesktopState.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Models/FileContextState.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Models/fileIncludeFilter.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Models/ScreenMetricsState.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Models/VisionState.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Models/WorldState.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Services/AccessibilityService.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Services/CaptureClipboardService.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Services/CaptureDesktopService.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Services/CaptureService.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Services/FileContextExtractorService.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Services/ScreenMetricsService.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Services/WorldStateBuilderService.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Services/DesktopService/BaseOsInfo.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Services/DesktopService/Connection.cpp
        ${CMAKE_SOURCE_DIR}/Observation/Services/DesktopService/IdleTime.cpp
)

if(WIN32)
    list(APPEND OBSERVATION_CORE_SOURCES
            ${CMAKE_SOURCE_DIR}/Observation/Services/CaptureServices/CaptureWin.cpp
            ${CMAKE_SOURCE_DIR}/Observation/Services/DesktopService/ActiveAppWin.cpp
            ${CMAKE_SOURCE_DIR}/Observation/Services/DesktopService/CPURAMWin.cpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND OBSERVATION_CORE_SOURCES
            ${CMAKE_SOURCE_DIR}/Observation/Services/CaptureServices/CaptureLinux.cpp
            ${CMAKE_SOURCE_DIR}/Observation/Services/DesktopService/ActiveAppLinux.cpp
            ${CMAKE_SOURCE_DIR}/Observation/Services/DesktopService/CPURAMLinux.cpp
    )
endif()

# ==========================================
# Target: observation_benchmark
# ==========================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/observation_benchmark.cpp")
    add_executable(observation_benchmark
            observation_benchmark.cpp
            ${OBSERVATION_CORE_SOURCES}
    )

    target_include_directories(observation_benchmark PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/Observation
            ${CMAKE_SOURCE_DIR}/Observation/Models
            ${CMAKE_SOURCE_DIR}/Observation/Services
    )

    target_link_libraries(observation_benchmark PRIVATE
            nlohmann_json::nlohmann_json
    )

    if(WIN32)
        target_link_libraries(observation_benchmark PRIVATE
                user32
                shell32
                ole32
                oleaut32
                wininet
                ws2_32
                crypt32
                gdi32
                iphlpapi
        )
        if(MINGW)
            target_compile_options(observation_benchmark PRIVATE -Wa,-mbig-obj)
        endif()
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(observation_benchmark PRIVATE
                PkgConfig::LINUX_DEPS
                PkgConfig::XSCRNSAVER
                pthread
                dl
        )
    endif()

    set_target_properties(observation_benchmark PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
    )
endif()