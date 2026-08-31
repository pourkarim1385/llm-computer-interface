cmake_minimum_required(VERSION 3.20)

set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "" FORCE)

project(AccessibilityService LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

#Disable tests and documentation for dependencies to speed up builds
set(CPR_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)

# CPR (C++ Requests)
FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 1.10.5
)
FetchContent_MakeAvailable(cpr)

# nlohmann/json
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# sqlite_orm - DOWNLOAD ONLY (Bypasses its internal find_package(SQLite3))
FetchContent_Declare(
        sqlite_orm
        GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
        GIT_TAG v1.9
)
FetchContent_GetProperties(sqlite_orm)
if(NOT sqlite_orm_POPULATED)
    FetchContent_Populate(sqlite_orm)
endif()

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
        Observation/Models/fileIncludeFilter.cpp
        Observation/Models/fileIncludeFilter.h
        Actuation/ActionExecutorServices/AsyncCommand.cpp
        Actuation/ActionExecutorServices/AsyncCommand.h
        Actuation/ActionExecutorServices/SystemService.cpp
        Actuation/ActionExecutorServices/SystemService.h
        Actuation/ActionDispatcher.cpp
        Actuation/ActionDispatcher.h
        Actuation/Actions.h
        Actuation/ExecutionCallStack.cpp
        Actuation/ExecutionCallStack.h
        Actuation/ActionExecutorServices/FileService.cpp
        Actuation/ActionExecutorServices/FileService.h
        Actuation/ActionExecutorServices/MouseService.hpp
        Actuation/ActionExecutorServices/MouseService.cpp
        Actuation/WebSearchServices/SearchProvider.cpp
        Actuation/WebSearchServices/SearchProvider.h
        Actuation/WebSearchServices/SearchExceptions.cpp
        Actuation/WebSearchServices/SearchExceptions.h
        Actuation/WebSearchServices/SearchService.cpp
        Actuation/WebSearchServices/SearchService.h
        Actuation/WebSearchServices/SearchTypes.h
        Actuation/WebSearchServices/SearchTypes.cpp
        Config/LLM/LlmProviderConfig.cpp
        Config/LLM/LlmProviderConfig.h
        Config/UserSettings.cpp
        Config/UserSettings.h
        Context/Message.cpp
        Context/Message.h
        Context/ChatHistory.cpp
        Context/ChatHistory.h
        Actuation/PermissionValidator.cpp
        Actuation/PermissionValidator.h
        Repository/DbMappers.cpp
        Repository/DbMappers.h
        Repository/SecretVault.cpp
        Repository/SecretVault.h
        Repository/DatabaseManager.cpp
        Repository/DatabaseManager.h
        externalLibs/sqlite/shell.c
        externalLibs/sqlite/sqlite3.c
        externalLibs/sqlite/sqlite3.h
        externalLibs/sqlite/sqlite3ext.h
        Repository/SqlInterfaces/SettingsRepository.cpp
        Repository/SqlInterfaces/SettingsRepository.h
        Repository/SqlInterfaces/ChatRepository.cpp
        Repository/SqlInterfaces/ChatRepository.h
)

if(WIN32)
    list(APPEND SOURCES
            Observation/Services/CaptureServices/CaptureWin.hpp
            Observation/Services/CaptureServices/CaptureWin.cpp
            Observation/Services/DesktopService/ActiveAppWin.cpp
            Observation/Services/DesktopService/ActiveAppWin.hpp
            Observation/Services/DesktopService/CPURAMWin.cpp
            Observation/Services/DesktopService/CPURAMWin.hpp
            Actuation/ActionExecutorServices/MouseSource/MouseClickWin.cpp
            Actuation/ActionExecutorServices/MouseSource/MouseClickWin.hpp
            Actuation/ActionExecutorServices/MouseSource/MoveMouseWin.cpp
            Actuation/ActionExecutorServices/MouseSource/MoveMouseWin.hpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND SOURCES
            Observation/Services/CaptureServices/CaptureLinux.hpp
            Observation/Services/CaptureServices/CaptureLinux.cpp
            Observation/Services/DesktopService/ActiveAppLinux.cpp
            Observation/Services/DesktopService/ActiveAppLinux.hpp
            Observation/Services/DesktopService/CPURAMLinux.cpp
            Observation/Services/DesktopService/CPURAMLinux.hpp
            Actuation/ActionExecutorServices/MouseSource/MouseClickLinux.cpp
            Actuation/ActionExecutorServices/MouseSource/MouseClickLinux.hpp
            Actuation/ActionExecutorServices/MouseSource/MoveMouseLinux.cpp
            Actuation/ActionExecutorServices/MouseSource/MoveMouseLinux.hpp
    )
endif()

add_executable(AccessibilityService ${SOURCES})

#Python Sidecar Bundling

#Locate the Python 3 interpreter
find_package(Python3 REQUIRED COMPONENTS Interpreter)

#Define the input python script path
set(DDG_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/Actuation/WebSearchServices/ddg_server.py")

#Tell CMake to bundle using the found Python executable
add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ddg_server${CMAKE_EXECUTABLE_SUFFIX}"
        COMMAND ${Python3_EXECUTABLE} -m PyInstaller
        --noconfirm
        --onefile
        --distpath "${CMAKE_CURRENT_BINARY_DIR}"
        --name "ddg_server"
        "${DDG_SCRIPT}"
        DEPENDS "${DDG_SCRIPT}"
        COMMENT "Bundling Python sidecar with PyInstaller..."
)

#Create a target for the bundling step
add_custom_target(bundle_ddg_server ALL
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ddg_server${CMAKE_EXECUTABLE_SUFFIX}"
)

#Make the main C++ project wait for the bundle to finish
add_dependencies(AccessibilityService bundle_ddg_server)

#End Python Sidecar Bundling

add_custom_command(TARGET AccessibilityService POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:AccessibilityService>
        $<TARGET_FILE_DIR:AccessibilityService>
        COMMAND_EXPAND_LISTS
)

# Project root include path, sqlite directory, AND sqlite_orm includes
target_include_directories(AccessibilityService PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/externalLibs/sqlite
        ${sqlite_orm_SOURCE_DIR}/include
)

#Link cross-platform third-party libraries common to both OS targets
target_link_libraries(AccessibilityService PRIVATE
        cpr::cpr
        nlohmann_json::nlohmann_json
)

#Platform-specific linking
if(WIN32)
    target_link_libraries(AccessibilityService PRIVATE
            user32
            ole32
            oleaut32
            wininet
    )
elseif(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)

    #Use IMPORTED_TARGET to automatically bundle include dirs, compile options, and link libs
    pkg_check_modules(LINUX_DEPS REQUIRED IMPORTED_TARGET atspi-2 gtk+-3.0 x11 xfixes xrandr)
    pkg_check_modules(XSCRNSAVER REQUIRED IMPORTED_TARGET xscrnsaver)

    target_link_libraries(AccessibilityService PRIVATE
            PkgConfig::LINUX_DEPS
            PkgConfig::XSCRNSAVER
            pthread
            dl
    )
endif()