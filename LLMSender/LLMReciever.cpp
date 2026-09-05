#include "LLMReciever.hpp"



// ─── Helper ───────────────────────────────────────────────────────────────────

Actions::MouseButton LLMReciever::parseMouseButton(const std::string& btn) {
    if (btn == "right")  return Actions::MouseButton::Right;
    if (btn == "middle") return Actions::MouseButton::Middle;
    return Actions::MouseButton::Left;
}

// ─── Main dispatcher ──────────────────────────────────────────────────────────

Actions::Action LLMReciever::parseAction(const std::string& tool, const json& args) {

    // ── InputData ─────────────────────────────────────────────────────────────

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

    // ── FileData ──────────────────────────────────────────────────────────────

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

    // ── SystemData ────────────────────────────────────────────────────────────

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

    // ── ControlData ───────────────────────────────────────────────────────────

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

    // ── Fallback ──────────────────────────────────────────────────────────────
    return Actions::ControlData{ Actions::Msg{
        "[unknown tool]: " + tool
    }};
}

// ─── Top-level parser ─────────────────────────────────────────────────────────

std::vector<ActionItem> LLMReciever::parseLLMResponse(
    const std::string& rawJson,
    std::vector<ActionItem>& actionItems,
    std::vector<Description>& descriptions)
{
    json response = json::parse(rawJson);
    
    std::string content_str = response["choices"][0]["message"]["content"];
    json content = json::parse(content_str);
    
    if (!content.contains("steps")) {
        throw std::runtime_error("'steps' key not found in LLM content");
    }

    for (const auto& [seq_key, step] : content["steps"].items()) {
        const std::string tool = step.at("tool").get<std::string>();
        const json args = step.value("arguments", json::object());

        actionItems.push_back(ActionItem{
            step.at("id").get<std::string>(),
            to_string(sequenceId),
            parseAction(tool, args)
        });

        std::cout << step.at("id").get<std::string>() << " : " << tool << std::endl;

        if (step.contains("description")) {
            descriptions.push_back(Description{
                to_string(sequenceId),
                step.at("id").get<std::string>(),
                step.at("description").get<std::string>()
            });
            std::cout << step.at("id").get<std::string>() << " : "
                      << step.at("description").get<std::string>() << std::endl;
        }
    }
    
    sequenceId += 1;
    return actionItems;
}



