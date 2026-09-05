#include "LLMReciever.hpp"

Actions::MouseButton LLMReciever::parseMouseButton(const std::string& btn) {
    if (btn == "right")  return Actions::MouseButton::Right;
    if (btn == "middle") return Actions::MouseButton::Middle;
    return Actions::MouseButton::Left;
}

Actions::Action LLMReciever::parseAction(const std::string& tool, const json& args) {

    //InputData
    if (tool == "functions.MoveMouse")
        return Actions::InputData{ Actions::MoveMouse{
            args.at("x").get<int>(),
            args.at("y").get<int>()
        }};

    if (tool == "functions.Click")
        return Actions::InputData{ Actions::Click{
            parseMouseButton(args.value("button", "left"))
        }};

    if (tool == "functions.DoubleClick")
        return Actions::InputData{ Actions::DoubleClick{
            parseMouseButton(args.value("button", "left"))
        }};

    if (tool == "functions.Type")
        return Actions::InputData{ Actions::Type{
            args.at("text").get<std::string>()
        }};

    if (tool == "functions.KeyPress")
        return Actions::InputData{ Actions::KeyPress{
            args.at("key").get<std::string>()
        }};

    if (tool == "functions.Scroll")
        return Actions::InputData{ Actions::Scroll{
            args.at("amount").get<int>()
        }};

    if (tool == "functions.Hotkey") {
        std::vector<std::string> keys =
            args.at("keys").get<std::vector<std::string>>();
        return Actions::InputData{ Actions::Hotkey{ std::move(keys) }};
    }

    if (tool == "functions.MouseDown")
        return Actions::InputData{ Actions::MouseDown{
            parseMouseButton(args.value("button", "left"))
        }};

    if (tool == "functions.MouseUp")
        return Actions::InputData{ Actions::MouseUp{
            parseMouseButton(args.value("button", "left"))
        }};

    if (tool == "functions.DragMouse")
        return Actions::InputData{ Actions::DragMouse{
            args.at("start_x").get<int>(),
            args.at("start_y").get<int>(),
            args.at("end_x").get<int>(),
            args.at("end_y").get<int>()
        }};

    //FileData

    if (tool == "functions.CreateFile")
        return Actions::FileData{ Actions::CreateFile{
            args.at("path").get<std::string>(),
            args.value("text", "")
        }};

    if (tool == "functions.WriteFile")
        return Actions::FileData{ Actions::WriteFile{
            args.at("path").get<std::string>(),
            args.at("text").get<std::string>()
        }};

    if (tool == "functions.AppendFile")
        return Actions::FileData{ Actions::AppendFile{
            args.at("path").get<std::string>(),
            args.at("text").get<std::string>()
        }};

    if (tool == "functions.InsertFile")
        return Actions::FileData{ Actions::InsertFile{
            args.at("path").get<std::string>(),
            args.at("position").get<int>(),
            args.at("text").get<std::string>()
        }};

    if (tool == "functions.DeleteFile")
        return Actions::FileData{ Actions::DeleteFile{
            args.at("path").get<std::string>()
        }};

    if (tool == "functions.RenameFile")
        return Actions::FileData{ Actions::RenameFile{
            args.at("path").get<std::string>(),
            args.at("new_path").get<std::string>()
        }};

    if (tool == "functions.CopyFile")
        return Actions::FileData{ Actions::CopyFile{
            args.at("path").get<std::string>(),
            args.at("destination").get<std::string>()
        }};

    if (tool == "functions.MoveFile")
        return Actions::FileData{ Actions::MoveFile{
            args.at("path").get<std::string>(),
            args.at("destination").get<std::string>()
        }};

    if (tool == "functions.EditFile") {
        std::vector<Actions::TextEdit> edits;
        for (const auto& e : args.at("edits")) {
            edits.push_back(Actions::TextEdit{
                e.at("start_index").get<size_t>(),
                e.at("end_index").get<size_t>(),
                e.at("text").get<std::string>()
            });
        }
        return Actions::FileData{ Actions::EditFile{
            args.at("path").get<std::string>(),
            std::move(edits)
        }};
    }

    if (tool == "functions.ApplyBlockDiff") {
        std::vector<Actions::ReplaceBlock> edits;
        for (const auto& e : args.at("edits")) {
            edits.push_back(Actions::ReplaceBlock{
                e.at("search_text").get<std::string>(),
                e.at("replace_text").get<std::string>()
            });
        }
        return Actions::FileData{ Actions::ApplyBlockDiff{
            args.at("path").get<std::string>(),
            std::move(edits)
        }};
    }

    //SystemData

    if (tool == "functions.RunCmd")
        return Actions::SystemData{ Actions::RunCmd{
            args.at("command").get<std::string>(), "", 0
        }};

    if (tool == "functions.RunPowerShell")
        return Actions::SystemData{ Actions::RunPowerShell{
            args.at("command").get<std::string>(), "", 0
        }};

    if (tool == "functions.OpenApp")
        return Actions::SystemData{ Actions::OpenApp{
            args.at("name").get<std::string>()
        }};

    if (tool == "functions.CloseApp")
        return Actions::SystemData{ Actions::CloseApp{
            args.at("name").get<std::string>()
        }};

    if (tool == "functions.FocusWindow")
        return Actions::SystemData{ Actions::FocusWindow{
            args.at("name").get<std::string>()
        }};

    if (tool == "functions.MinimizeWindow")
        return Actions::SystemData{ Actions::MinimizeWindow{
            args.at("name").get<std::string>()
        }};

    if (tool == "functions.MaximizeWindow")
        return Actions::SystemData{ Actions::MaximizeWindow{
            args.at("name").get<std::string>()
        }};

    if (tool == "functions.RestoreWindow")
        return Actions::SystemData{ Actions::RestoreWindow{
            args.at("name").get<std::string>()
        }};

    if (tool == "functions.SetVolume")
        return Actions::SystemData{ Actions::SetVolume{
            args.at("value").get<int>()
        }};

    if (tool == "functions.MuteVolume")
        return Actions::SystemData{ Actions::MuteVolume{} };

    if (tool == "functions.UnmuteVolume")
        return Actions::SystemData{ Actions::UnmuteVolume{} };

    if (tool == "functions.Sleep")
        return Actions::SystemData{ Actions::Sleep{} };

    if (tool == "functions.Shutdown")
        return Actions::SystemData{ Actions::Shutdown{} };

    if (tool == "functions.Restart")
        return Actions::SystemData{ Actions::Restart{} };

    //ControlData

    if (tool == "functions.Msg")
        return Actions::ControlData{ Actions::Msg{
            args.at("content").get<std::string>()
        }};

    if (tool == "functions.Observe")
        return Actions::ControlData{ Actions::Observe{} };

    if (tool == "functions.Wait")
        return Actions::ControlData{ Actions::Wait{
            args.at("value").get<int>()
        }};

    if (tool == "functions.FAR")
        return Actions::ControlData{ Actions::FAR{
            args.at("path").get<std::string>()
        }};

    if (tool == "functions.IsVerified")
        return Actions::ControlData{ Actions::IsVerified{
            args.at("value").get<bool>()
        }};

    if (tool == "functions.ClearStack")
        return Actions::ControlData{ Actions::ClearStack{} };

    if (tool == "functions.SearchWeb")
        return Actions::ControlData{ Actions::SearchWeb{
            args.at("query").get<std::string>(),
            args.value("max_result", 1)
        }};

    //Fallback
    return Actions::ControlData{ Actions::Msg{
        "[unknown tool]: " + tool
    }};
}

json LLMReciever::BuildToolsSchema() {
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

std::vector<ActionItem> LLMReciever::parseLLMResponse(
    const std::string& rawJson,
    std::vector<ActionItem>& actionItems,
    Plan& userPlan)
{
    json response = json::parse(rawJson);

    std::string content_str = response["choices"][0]["message"]["content"];
    json content = json::parse(content_str);

    if (!content.contains("steps")) {
        throw std::runtime_error("'steps' key not found in LLM content");
    }

    userPlan.name = content.value("task_name", "");
    userPlan.description = content.value("task_description", "");

    for (const auto& [seq_key, step] : content["steps"].items()) {
        const std::string tool = step.at("tool").get<std::string>();
        const json args = step.value("arguments", json::object());
        const std::string stepId = step.at("id").get<std::string>();
        const std::string title = step.value("title", "");
        const std::string stepContent = step.value("content", "");

        actionItems.push_back(ActionItem{
            stepId,
            std::to_string(sequenceId),
            parseAction(tool, args)
        });

        userPlan.steps.push_back(Step{
            title,
            stepContent,
            false
        });

        std::cout << stepId << " : " << tool << " -> " << title << std::endl;
    }
    
    sequenceId += 1;
    return actionItems;
}

void LLMReciever::parse(const std::string& rawJson, ExecutionCallStack& callStack) {
    json response = json::parse(rawJson);
    std::string content_str = response["choices"][0]["message"]["content"];
    json content = json::parse(content_str);

    for (const auto& [seq_key, step] : content["steps"].items()) {
        const std::string tool = step.at("tool").get<std::string>();
        const json args = step.value("arguments", json::object());

        callStack.push_back(ActionItem{
            step.at("id").get<std::string>(),
            to_string(sequenceId),
            parseAction(tool, args)
        });

        //if (step.contains("description")) {
        //    plan.steps.push_back(Step{
        //        step.at("id").get<std::string>(),
        //        step.at("description").get<std::string>()
        //    });
        //}
    }
}