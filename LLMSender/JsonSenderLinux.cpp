#include "JsonSenderLinux.hpp"

std::string JsonSender::IntegratedDetail(const std::vector<std::string>& detail) {
    std::string integratedDetail;
    integratedDetail.reserve(detail.size() * 100);
    for (const auto& str : detail) {
        integratedDetail += str;
    }
    return integratedDetail;
}

size_t JsonSender::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    if (userp) userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

json JsonSender::BuildToolsSchema() {
    json tools = json::array({

        // ─── INPUT ACTIONS ────────────────────────────────────────────
        {
            {"type", "function"},
            {"function", {
                {"name", "move_mouse"},
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
                {"name", "click"},
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
                {"name", "double_click"},
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
                {"name", "type_text"},
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
                {"name", "key_press"},
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
                {"name", "scroll"},
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
                {"name", "hotkey"},
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
                {"name", "mouse_down"},
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
                {"name", "mouse_up"},
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
                {"name", "drag_mouse"},
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

        // ─── FILE ACTIONS ─────────────────────────────────────────────
        {
            {"type", "function"},
            {"function", {
                {"name", "create_file"},
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
                {"name", "create_file"},
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
                {"name", "append_file"},
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
                {"name", "insert_file"},
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
                {"name", "delete_file"},
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
                {"name", "rename_file"},
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
                {"name", "copy_file"},
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
                {"name", "move_file"},
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
        // ─── SYSTEM ACTIONS ───────────────────────────────────────────
        {
            {"type", "function"},
            {"function", {
                {"name", "run_cmd"},
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
                {"name", "run_powershell"},
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
                {"name", "open_app"},
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
                {"name", "close_app"},
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
                {"name", "focus_window"},
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
                {"name", "minimize_window"},
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
                {"name", "maximize_window"},
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
                {"name", "restore_window"},
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
                {"name", "set_volume"},
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
                {"name", "mute_volume"},
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
                {"name", "unmute_volume"},
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
                {"name", "sleep"},
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
                {"name", "shutdown"},
                {"description", "Shut down the operating system."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", json::object()},
                    {"required", json::array()}
                }}
            }}
        },

        // ─── CONTROL ACTIONS ──────────────────────────────────────────
        {
            {"type", "function"},
            {"function", {
                {"name", "msg"},
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
                {"name", "observe"},
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
                {"name", "wait"},
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
                {"name", "far"},
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
                {"name", "is_verified"},
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
                {"name", "clear_stack"},
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
                {"name", "search_web"},
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

std::string JsonSender::SendDataToLLM(
    const std::string& apiKey,
    const std::string& endpoint,
    const std::string& user_prompt,
    const std::string& sysData,
    const json& tools,
    const std::string& image,
    const std::string& file,
    const std::string& model,
    double temperature
) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[cURL Error] Failed to initialize cURL." << std::endl;
        return "";
    }

    std::string response_string;

    json user_content = json::array();

    if (!user_prompt.empty()) {
        user_content.push_back({{"type", "text"}, {"text", user_prompt}});
    }
    if (!file.empty()) {
        user_content.push_back({
            {"type", "text"},
            {"text", "\n\n--- Attached File Content ---\n" + file}
        });
    }
    if (!image.empty()) {
        user_content.push_back({
            {"type", "image_url"},
            {"image_url", {{"url", "data:image/jpeg;base64," + image}}}
        });
    }

    json messages = json::array();
    if (!sysData.empty()) {
        messages.push_back({{"role", "system"}, {"content", sysData}});
    }
    messages.push_back({{"role", "user"}, {"content", user_content}});

    json payload = {
        {"model",       model},
        {"messages",    messages},
        {"temperature", temperature}
    };

    if (!tools.is_null() && !tools.empty()) {
        payload["tools"]       = tools;
        payload["tool_choice"] = "auto";
    }

    std::string json_payload = payload.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth_header = "Authorization: Bearer " + apiKey;
    headers = curl_slist_append(headers, auth_header.c_str());

    curl_easy_setopt(curl, CURLOPT_URL,           endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    json_payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,       60L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "[cURL Error] " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response_string;
}
