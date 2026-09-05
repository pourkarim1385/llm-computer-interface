#pragma once

#include <string>
#include <variant>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

    json BuildToolsSchema() {
    json tools = json::array({

        //INPUT ACTIONS
        {
            {"type", "function"},
            {"function", {
                {"name", "MoveMouse"},
                {"description", "Move the mouse cursor to an absolute screen coordinate."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"x", {{"type", "integer"}, {"description", "Target X coordinate in pixels"}}},
                        {"y", {{"type", "integer"}, {"description", "Target Y coordinate in pixels"}}}
                    }},
                    {"required", json::array({"x", "y"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "Click"},
                {"description", "Click a mouse button at the current cursor position."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"button", {
                            {"type", "string"},
                            {"enum", json::array({"left", "right", "middle"})},
                            {"description", "Mouse button to click"}
                        }}
                    }},
                    {"required", json::array({"button"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "DoubleClick"},
                {"description", "Double-click a mouse button at the current cursor position."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"button", {
                            {"type", "string"},
                            {"enum", json::array({"left", "right", "middle"})},
                            {"description", "Mouse button to double-click"}
                        }}
                    }},
                    {"required", json::array({"button"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "Type"},
                {"description", "Type a string of text using the keyboard."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"text", {{"type", "string"}, {"description", "Text to type"}}}
                    }},
                    {"required", json::array({"text"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "KeyPress"},
                {"description", "Press a single keyboard key (e.g. Enter, Escape, Tab, F5)."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"key", {{"type", "string"}, {"description", "Key name to press"}}}
                    }},
                    {"required", json::array({"key"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "Scroll"},
                {"description", "Scroll the mouse wheel. Positive scrolls down, negative scrolls up."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"amount", {{"type", "integer"}, {"description", "Scroll amount (positive=down, negative=up)"}}}
                    }},
                    {"required", json::array({"amount"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "Hotkey"},
                {"description", "Press a keyboard shortcut by holding multiple keys simultaneously."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"keys", {
                            {"type", "array"},
                            {"items", {{"type", "string"}}},
                            {"description", "Ordered list of keys to hold, e.g. [ctrl, c]"}
                        }}
                    }},
                    {"required", json::array({"keys"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "MouseDown"},
                {"description", "Press and hold a mouse button without releasing it."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"button", {
                            {"type", "string"},
                            {"enum", json::array({"left", "right", "middle"})},
                            {"description", "Mouse button to hold down"}
                        }}
                    }},
                    {"required", json::array({"button"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "MouseUp"},
                {"description", "Release a previously held mouse button."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"button", {
                            {"type", "string"},
                            {"enum", json::array({"left", "right", "middle"})},
                            {"description", "Mouse button to release"}
                        }}
                    }},
                    {"required", json::array({"button"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "DragMouse"},
                {"description", "Click and drag the mouse from a start coordinate to an end coordinate."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"start_x", {{"type", "integer"}, {"description", "Start X coordinate"}}},
                        {"start_y", {{"type", "integer"}, {"description", "Start Y coordinate"}}},
                        {"end_x",   {{"type", "integer"}, {"description", "End X coordinate"}}},
                        {"end_y",   {{"type", "integer"}, {"description", "End Y coordinate"}}}
                    }},
                    {"required", json::array({"start_x", "start_y", "end_x", "end_y"})}
                }}
            }}
        },

        //FILE ACTIONS
        {
            {"type", "function"},
            {"function", {
                {"name", "CreateFile"},
                {"description", "Create a new file at the given path with optional initial content."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path", {
                            {"type", "string"},
                            {"description", "The file path to create."}
                        }},
                        {"content", {
                            {"type", "string"},
                            {"description", "Initial content to write into the file."}
                        }}
                    }},
                    {"required", {"path"}}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "WriteFile"},
                {"description", "Create a new file at the given path with optional initial content."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path",    {{"type", "string"}, {"description", "File path to create"}}},
                        {"content", {{"type", "string"}, {"description", "Optional initial content for the file"}}}
                    }},
                    {"required", json::array({"path"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "AppendFile"},
                {"description", "Append text to the end of an existing file without modifying current content."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path", {{"type", "string"}, {"description", "File path"}}},
                        {"text", {{"type", "string"}, {"description", "Text to append"}}}
                    }},
                    {"required", json::array({"path", "text"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "InsertFile"},
                {"description", "Insert text at a specific character position inside a file."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path",     {{"type", "string"},  {"description", "File path"}}},
                        {"position", {{"type", "integer"}, {"description", "Zero-based character offset where text is inserted"}}},
                        {"text",     {{"type", "string"},  {"description", "Text to insert"}}}
                    }},
                    {"required", json::array({"path", "position", "text"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "DeleteFile"},
                {"description", "Permanently delete a file from the filesystem."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path", {{"type", "string"}, {"description", "File path to delete"}}}
                    }},
                    {"required", json::array({"path"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "RenameFile"},
                {"description", "Rename a file or directory."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path",     {{"type", "string"}, {"description", "Current file path"}}},
                        {"new_path", {{"type", "string"}, {"description", "New file path or name"}}}
                    }},
                    {"required", json::array({"path", "new_path"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "CopyFile"},
                {"description", "Copy a file to a destination path."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path",        {{"type", "string"}, {"description", "Source file path"}}},
                        {"destination", {{"type", "string"}, {"description", "Destination path"}}}
                    }},
                    {"required", json::array({"path", "destination"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "MoveFile"},
                {"description", "Move a file to a different path (rename across directories)."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path",        {{"type", "string"}, {"description", "Source file path"}}},
                        {"destination", {{"type", "string"}, {"description", "Destination path"}}}
                    }},
                    {"required", json::array({"path", "destination"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "edit_file"},
                {"description", "Edit an existing file by replacing a target string with new content."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path", {
                            {"type", "string"},
                            {"description", "Path to the file to edit."}
                        }},
                        {"old_content", {
                            {"type", "string"},
                            {"description", "The exact string to be replaced."}
                        }},
                        {"new_content", {
                            {"type", "string"},
                            {"description", "The replacement string."}
                        }}
                    }},
                    {"required", {"path", "old_content", "new_content"}}
                }}
            }}
        },
        //SYSTEM ACTIONS
        {
            {"type", "function"},
            {"function", {
                {"name", "RunCmd"},
                {"description", "Execute a bash shell command and return its stdout/stderr output."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"command", {{"type", "string"}, {"description", "Shell command to execute"}}}
                    }},
                    {"required", json::array({"command"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "RunPowerShell"},
                {"description", "Execute a PowerShell command and return its output (Windows only)."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"command", {{"type", "string"}, {"description", "PowerShell command to run"}}}
                    }},
                    {"required", json::array({"command"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "OpenApp"},
                {"description", "Launch an application by name."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"name", {{"type", "string"}, {"description", "Application name or executable"}}}
                    }},
                    {"required", json::array({"name"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "CloseApp"},
                {"description", "Close or terminate a running application by name."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"name", {{"type", "string"}, {"description", "Application name"}}}
                    }},
                    {"required", json::array({"name"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "FocusWindow"},
                {"description", "Bring a window to the foreground and give it input focus."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"name", {{"type", "string"}, {"description", "Window title or app name"}}}
                    }},
                    {"required", json::array({"name"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "MinimizeWindow"},
                {"description", "Minimize a window to the taskbar."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"name", {{"type", "string"}, {"description", "Window title or app name"}}}
                    }},
                    {"required", json::array({"name"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "MaximizeWindow"},
                {"description", "Maximize a window to fill the screen."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"name", {{"type", "string"}, {"description", "Window title or app name"}}}
                    }},
                    {"required", json::array({"name"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "RestoreWindow"},
                {"description", "Restore a minimized or maximized window to its previous size."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"name", {{"type", "string"}, {"description", "Window title or app name"}}}
                    }},
                    {"required", json::array({"name"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "SetVolume"},
                {"description", "Set the system volume to a specific level (0-100)."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"value", {{"type", "integer"}, {"description", "Volume level 0-100"}}}
                    }},
                    {"required", json::array({"value"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "MuteVolume"},
                {"description", "Mute system audio output."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", json::object()},
                    {"required", json::array()}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "UnmuteVolume"},
                {"description", "Unmute system audio output."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", json::object()},
                    {"required", json::array()}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "Sleep"},
                {"description", "Put the system into sleep or suspend mode."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", json::object()},
                    {"required", json::array()}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "Shutdown"},
                {"description", "Shut down the operating system."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", json::object()},
                    {"required", json::array()}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "Restart"},
                {"description", "Restart the operating system."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", json::object()},
                    {"required", json::array()}
                }}
            }}
        },

        //CONTROL ACTIONS
        {
            {"type", "function"},
            {"function", {
                {"name", "Msg"},
                {"description", "Send a plain-text message or status update back to the orchestrator."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"content", {{"type", "string"}, {"description", "Message text"}}}
                    }},
                    {"required", json::array({"content"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "Observe"},
                {"description", "Request an observation snapshot of the current screen or environment state."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", json::object()},
                    {"required", json::array()}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "Wait"},
                {"description", "Pause execution for the specified number of milliseconds."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"value", {{"type", "integer"}, {"description", "Wait duration in milliseconds"}}}
                    }},
                    {"required", json::array({"value"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "FAR"},
                {"description", "File Access Request - request read access to a file before operating on it."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path", {{"type", "string"}, {"description", "Path of the file to request access to"}}}
                    }},
                    {"required", json::array({"path"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "IsVerified"},
                {"description", "Signal whether the last action was verified as successful or not."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"value", {{"type", "boolean"}, {"description", "true = verified success, false = verification failed"}}}
                    }},
                    {"required", json::array({"value"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "ClearStack"},
                {"description", "Clear the current action stack and reset orchestrator state."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", json::object()},
                    {"required", json::array()}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "SearchWeb"},
                {"description", "Search the web for current information on a given query."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"query", {
                            {"type", "string"},
                            {"description", "The search query string."}
                        }},
                        {"max_results", {
                            {"type", "integer"},
                            {"description", "Maximum number of results to return."}
                        }}
                    }},
                    {"required", {"query"}}
                }}
            }}
        }
    });

    return tools;
}
}