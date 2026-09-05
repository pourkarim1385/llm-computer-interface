#pragma once

#include <string>
#include <variant>
#include <vector>
#include <filesystem>

enum class ActionStatus {
    Success,
    Failed,
    TriggerObserve // Special signal for the Orchestrator
};

namespace Actions {
    enum class MouseButton {
        Left,
        Right,
        Middle
    };

    struct MoveMouse    { int x; int y; };
    struct Click        { MouseButton button; };
    struct DoubleClick  { MouseButton button; };
    struct Type         { std::string text; };
    struct KeyPress     { std::string key; };
    struct Scroll       { int amount; };
    struct Hotkey       { std::vector<std::string> keys; };
    struct MouseDown  { MouseButton button; };
    struct MouseUp    { MouseButton button; };
    struct DragMouse { int start_x, start_y, end_x, end_y; };

    using InputData = std::variant<
            MoveMouse,
            Click,
            DoubleClick,
            Type,
            KeyPress,
            Scroll,
            Hotkey,
            MouseDown,
            MouseUp,
            DragMouse
    >;

    struct CreateFile { std::filesystem::path path; std::string text; };
    struct WriteFile  { std::filesystem::path path; std::string text; };
    struct AppendFile { std::filesystem::path path; std::string text; };
    struct InsertFile { std::filesystem::path path; int position; std::string text; };
    struct DeleteFile { std::filesystem::path path; };
    struct RenameFile { std::filesystem::path path; std::filesystem::path new_path; };
    struct CopyFile   { std::filesystem::path path; std::filesystem::path destination; };
    struct MoveFile   { std::filesystem::path path; std::filesystem::path destination; };

    struct ReplaceBlock {
        std::string search_text;
        std::string replace_text;
    };

    struct TextEdit {
        size_t start_index;
        size_t end_index;
        std::string text; //replacing text
    };

    struct EditFile {
        std::filesystem::path path;
        std::vector<TextEdit> edits;
    };

    struct ApplyBlockDiff{
        std::filesystem::path path;
        std::vector<ReplaceBlock> edits;
    };
    //NOTE: about edit file service:
    //do changes reverse order using sort
    /*
     std::sort(editFile.edits.begin(), editFile.edits.end(),
        [](const TextEdit& a, const TextEdit& b) {
            return a.start_index > b.start_index;
        }
    );
     * */
    //or do offset tracking if you dont want to do reverse order

    using FileData = std::variant<
            CreateFile,
            WriteFile,
            AppendFile,
            InsertFile,
            DeleteFile,
            RenameFile,
            CopyFile,
            MoveFile,
            EditFile,
            ApplyBlockDiff
    >;


    struct RunCmd         { std::string command; mutable std::string output; mutable size_t cycle;};
    struct RunPowerShell  { std::string command; mutable std::string output; mutable size_t cycle;};
    struct OpenApp        { std::string name; };
    struct CloseApp       { std::string name; };
    struct FocusWindow    { std::string name; };
    struct MinimizeWindow { std::string name; };
    struct MaximizeWindow { std::string name; };
    struct RestoreWindow  { std::string name; };
    struct SetVolume      { int value; };
    struct MuteVolume     {}; // Empty struct (no params)
    struct UnmuteVolume   {};
    struct Sleep          {};
    struct Shutdown       {};
    struct Restart        {};

    using SystemData = std::variant<
            RunCmd,
            RunPowerShell,
            OpenApp,
            CloseApp,
            FocusWindow,
            MinimizeWindow,
            MaximizeWindow,
            RestoreWindow,
            SetVolume,
            MuteVolume,
            UnmuteVolume,
            Sleep,
            Shutdown,
            Restart
    >;

    struct Msg        { std::string content; };
    struct Observe    {
        //ObservationFlags flags;
    };
    struct Wait       { int value; }; // milliseconds
    struct FAR        { std::string path; };
    struct IsVerified { bool value; };
    struct ClearStack {};
    struct SearchWeb{
        std::string query;
        int max_result;
    };

    using ControlData = std::variant<
            Msg,
            Observe,
            Wait,
            FAR,
            IsVerified,
            ClearStack,
            SearchWeb
    >;

    using Action = std::variant<
            InputData,
            FileData,
            SystemData,
            ControlData
    >;

    template<class... Ts>
    struct Overloaded : Ts... { using Ts::operator()...; };

    template<class... Ts>
    Overloaded(Ts...) -> Overloaded<Ts...>;
}