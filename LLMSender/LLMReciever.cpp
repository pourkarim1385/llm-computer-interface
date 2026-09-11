#include "LLMReciever.hpp"
#include <iostream>
#include <stdexcept>

Actions::MouseButton LLMReciever::parseMouseButton(const std::string& btn) {
    if (btn == "right")  return Actions::MouseButton::Right;
    if (btn == "middle") return Actions::MouseButton::Middle;
    if (btn == "left")   return Actions::MouseButton::Left;
    throw std::runtime_error("Invalid Mouse Button: " + btn);
}

Actions::Action LLMReciever::parseAction(const std::string& rawTool, const json& args) {
    std::string tool = rawTool;
    if (tool.rfind("functions.", 0) == 0) {
        tool = tool.substr(10);
    }

    // --- Input Actions ---
    if (tool == "MoveMouse") {
        return Actions::InputData{ Actions::MoveMouse{
            args.at("x").get<int>(),
            args.at("y").get<int>()
        }};
    }

    if (tool == "Click") {
        return Actions::InputData{ Actions::Click{
            parseMouseButton(args.value("button", "left"))
        }};
    }

    if (tool == "DoubleClick") {
        return Actions::InputData{ Actions::DoubleClick{
            parseMouseButton(args.value("button", "left"))
        }};
    }

    if (tool == "Type") {
        return Actions::InputData{ Actions::Type{
            args.at("text").get<std::string>()
        }};
    }

    if (tool == "PressKey" || tool == "KeyPress") {
        return Actions::InputData{ Actions::PressKey{
            args.at("key").get<std::string>()
        }};
    }

    if (tool == "Scroll") {
        return Actions::InputData{ Actions::Scroll{
            args.at("amount").get<int>()
        }};
    }

    if (tool == "Hotkey") {
        std::vector<std::string> keys = args.at("keys").get<std::vector<std::string>>();
        return Actions::InputData{ Actions::Hotkey{ std::move(keys) }};
    }

    if (tool == "MouseDown") {
        return Actions::InputData{ Actions::MouseDown{
            parseMouseButton(args.value("button", "left"))
        }};
    }

    if (tool == "MouseUp") {
        return Actions::InputData{ Actions::MouseUp{
            parseMouseButton(args.value("button", "left"))
        }};
    }

    if (tool == "DragMouse") {
        return Actions::InputData{ Actions::DragMouse{
            args.at("target_x").get<int>(),
            args.at("target_y").get<int>(),
            args.at("duration").get<int>(),
            args.at("step").get<int>()
        }};
    }

    // --- File Actions ---
    if (tool == "CreateFile") {
        std::string fileContent = args.contains("content") ? args["content"].get<std::string>() : args.value("text", "");
        return Actions::FileData{ Actions::CreateFile{
            args.at("path").get<std::string>(),
            fileContent
        }};
    }

    if (tool == "WriteFile") {
        std::string fileContent = args.contains("content") ? args["content"].get<std::string>() : args.value("text", "");
        return Actions::FileData{ Actions::WriteFile{
            args.at("path").get<std::string>(),
            fileContent
        }};
    }

    if (tool == "AppendFile") {
        std::string fileContent = args.contains("content") ? args["content"].get<std::string>() : args.at("text").get<std::string>();
        return Actions::FileData{ Actions::AppendFile{
            args.at("path").get<std::string>(),
            fileContent
        }};
    }

    if (tool == "InsertFile") {
        std::string fileContent = args.contains("content") ? args["content"].get<std::string>() : args.at("text").get<std::string>();
        return Actions::FileData{ Actions::InsertFile{
            args.at("path").get<std::string>(),
            args.at("position").get<int>(),
            fileContent
        }};
    }

    if (tool == "DeleteFile") {
        return Actions::FileData{ Actions::DeleteFile{
            args.at("path").get<std::string>()
        }};
    }

    if (tool == "RenameFile") {
        return Actions::FileData{ Actions::RenameFile{
            args.at("path").get<std::string>(),
            args.at("new_path").get<std::string>()
        }};
    }

    if (tool == "CopyFile") {
        return Actions::FileData{ Actions::CopyFile{
            args.at("path").get<std::string>(),
            args.at("destination").get<std::string>()
        }};
    }

    if (tool == "MoveFile") {
        return Actions::FileData{ Actions::MoveFile{
            args.at("path").get<std::string>(),
            args.at("destination").get<std::string>()
        }};
    }

    if (tool == "EditFile" || tool == "edit_file") {
        if (args.contains("edits") && args["edits"].is_array()) {
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
        } else if (args.contains("old_content") && args.contains("new_content")) {
            std::vector<Actions::ReplaceBlock> edits;
            edits.push_back(Actions::ReplaceBlock{
                args.at("old_content").get<std::string>(),
                args.at("new_content").get<std::string>()
            });
            return Actions::FileData{ Actions::ApplyBlockDiff{
                args.at("path").get<std::string>(),
                std::move(edits)
            }};
        }
    }

    if (tool == "ApplyBlockDiff") {
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

    // --- System Actions ---
    if (tool == "RunCmd") {
        return Actions::SystemData{ Actions::RunCmd{
            args.at("command").get<std::string>(), "", 0
        }};
    }

    if (tool == "RunPowerShell") {
        return Actions::SystemData{ Actions::RunPowerShell{
            args.at("command").get<std::string>(), "", 0
        }};
    }

    if (tool == "OpenApp") {
        return Actions::SystemData{ Actions::OpenApp{
            args.at("name").get<std::string>()
        }};
    }

    if (tool == "CloseApp") {
        return Actions::SystemData{ Actions::CloseApp{
            args.at("name").get<std::string>()
        }};
    }

    if (tool == "FocusWindow") {
        return Actions::SystemData{ Actions::FocusWindow{
            args.at("name").get<std::string>()
        }};
    }

    if (tool == "MinimizeWindow") {
        return Actions::SystemData{ Actions::MinimizeWindow{
            args.at("name").get<std::string>()
        }};
    }

    if (tool == "MaximizeWindow") {
        return Actions::SystemData{ Actions::MaximizeWindow{
            args.at("name").get<std::string>()
        }};
    }

    if (tool == "RestoreWindow") {
        return Actions::SystemData{ Actions::RestoreWindow{
            args.at("name").get<std::string>()
        }};
    }

    if (tool == "SetVolume") {
        return Actions::SystemData{ Actions::SetVolume{
            args.at("value").get<int>()
        }};
    }

    if (tool == "MuteVolume") {
        return Actions::SystemData{ Actions::MuteVolume{} };
    }

    if (tool == "UnmuteVolume") {
        return Actions::SystemData{ Actions::UnmuteVolume{} };
    }

    if (tool == "Sleep") {
        return Actions::SystemData{ Actions::Sleep{} };
    }

    if (tool == "Shutdown") {
        return Actions::SystemData{ Actions::Shutdown{} };
    }

    if (tool == "Restart") {
        return Actions::SystemData{ Actions::Restart{} };
    }

    // --- Control Actions ---
    if (tool == "Observe") {
        return Actions::ControlData{ Actions::Observe{} };
    }

    if (tool == "Wait") {
        return Actions::ControlData{ Actions::Wait{
            args.at("value").get<int>()
        }};
    }

    if (tool == "FAR") {
        return Actions::ControlData{ Actions::FAR{
            args.at("path").get<std::string>()
        }};
    }

    if (tool == "ClearStack") {
        return Actions::ControlData{ Actions::ClearStack{} };
    }

    if (tool == "SearchWeb") {
        int maxResult = 1;
        if (args.contains("max_results")) {
            maxResult = args["max_results"].get<int>();
        } else if (args.contains("max_result")) {
            maxResult = args["max_result"].get<int>();
        }

        return Actions::ControlData{ Actions::SearchWeb{
            args.at("query").get<std::string>(),
            maxResult
        }};
    }

    throw std::runtime_error("Unsupported or unknown tool: " + rawTool);
}

void LLMReciever::parse(const std::string& rawJson, ExecutionCallStack& callStack, Plan& userPlan, std::string& messageToUser) {
    json response;
    std::string validationError;

    if (!validateRawResponse(rawJson, response, validationError)) {
        throw std::runtime_error(validationError);
    }

    std::string content_str = response["choices"][0]["message"]["content"];
    std::string pureJsonStr = extractPureJson(content_str);

    json content;
    try {
        content = json::parse(pureJsonStr);
    } catch (const json::parse_error&) {
        userPlan.name = "Conversational Response";
        userPlan.description = "";
        messageToUser = content_str;
        return;
    }

    userPlan.name = content.value("task_name", "");
    userPlan.description = content.value("task_description", "");
    messageToUser = content.value("message_to_user", "");

    if (!content.contains("steps") || content["steps"].is_null() || !content["steps"].is_object()) {
        return;
    }

    for (const auto& [seq_key, step] : content["steps"].items()) {
        if (!step.is_object()) {
            continue;
        }

        json tool_json = step.value("tool", json());
        if (!tool_json.is_string()) {
            continue;
        }
        const std::string tool = tool_json.get<std::string>();

        json args = step.value("arguments", json::object());
        if (args.is_null() || !args.is_object()) {
            args = json::object();
        }

        json id_json = step.value("id", json());
        const std::string stepId = id_json.is_string() ? id_json.get<std::string>() : seq_key;

        const std::string title = step.value("title", "");
        const std::string stepContent = step.value("content", "");

        try {
            Actions::Action parsedAction = parseAction(tool, args);
            callStack.push_back(ActionItem{
                stepId,
                std::to_string(sequenceId),
                parsedAction
            });
            userPlan.steps.push_back(Step{ title, stepContent, false });
        } catch (const std::exception& e) {
            throw;
            //std::cerr << "[Parser Warning] Skipping invalid step (" << stepId << "): " << e.what() << std::endl;
        }
    }
    sequenceId++;
}

void LLMReciever::Testparse(const std::string& rawJson, Plan& userPlan, std::string& messageToUser) {
    json response = json::parse(rawJson);

    std::string content_str = response["choices"][0]["message"]["content"];
    json content = json::parse(content_str);

    if (!content.contains("steps")) {
        throw std::runtime_error("'steps' key not found in LLM content");
    }

    userPlan.name = content.value("task_name", "");
    userPlan.description = content.value("task_description", "");
    messageToUser = content.value("message_to_user", "");

    for (const auto& [seq_key, step] : content["steps"].items()) {
        const std::string tool = step.at("tool").get<std::string>();
        const json args = step.value("arguments", json::object());
        const std::string stepId = step.at("id").get<std::string>();
        const std::string title = step.value("title", "");
        const std::string stepContent = step.value("content", "");

        std::cout << "StepId : " << stepId << std::endl
                  << "SequenceId : " << sequenceId << std::endl
                  << "Tool : "  << tool << std::endl;

        userPlan.steps.push_back(Step{
            title,
            stepContent,
            false
        });
    }
    sequenceId++;
}

std::string LLMReciever::extractPureJson(const std::string& rawContent) {
    std::string trimmed = rawContent;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

    if (trimmed.rfind("```", 0) == 0) {
        size_t firstNewline = trimmed.find('\n');
        size_t lastBlock = trimmed.rfind("```");
        if (firstNewline != std::string::npos && lastBlock != std::string::npos && lastBlock > firstNewline) {
            trimmed = trimmed.substr(firstNewline + 1, lastBlock - firstNewline - 1);
        }
    }
    return trimmed;
}

bool LLMReciever::validateRawResponse(const std::string& rawJson, json& outResponse, std::string& outErrorMessage) {
    if (rawJson.empty()) {
        outErrorMessage = "Empty response received from LLM gateway/server.";
        return false;
    }

    try {
        outResponse = json::parse(rawJson);
    } catch (const json::parse_error& e) {
        outErrorMessage = "Failed to parse API response as JSON (Possible HTML/Proxy error): " + std::string(e.what());
        return false;
    }

    if (outResponse.contains("error")) {
        if (outResponse["error"].is_object()) {
            std::string msg = outResponse["error"].value("message", "Unknown API error");
            std::string code = outResponse["error"].value("code", "no_code");
            outErrorMessage = "[API Error] " + msg + " (Code: " + code + ")";
        } else if (outResponse["error"].is_string()) {
            outErrorMessage = "[API Error] " + outResponse["error"].get<std::string>();
        } else {
            outErrorMessage = "[API Error] Unknown error format received from endpoint.";
        }
        return false;
    }

    if (!outResponse.contains("choices") || !outResponse["choices"].is_array() || outResponse["choices"].empty()) {
        outErrorMessage = "Malformed LLM response: 'choices' array is missing or empty.";
        return false;
    }

    const auto& firstChoice = outResponse["choices"][0];
    if (!firstChoice.contains("message") || !firstChoice["message"].contains("content") || firstChoice["message"]["content"].is_null()) {
        outErrorMessage = "Malformed LLM response: 'message.content' is null or missing.";
        return false;
    }

    return true;
}