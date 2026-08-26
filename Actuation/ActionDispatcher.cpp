#include "ActionDispatcher.h"
// #include "Services/InputService.h"
// #include "Services/FileService.h"
// #include "Services/SystemService.h"
// #include "Services/ControlService.h"
#include "ActionExecutorServices/MouseService.hpp"
#include "Observation/Services/WorldStateBuilderService.h"
#include "Actuation/ActionExecutorServices/FileService.h"
#include "Actuation/WebSearchServices/SearchTypes.h"
#include "Actuation/WebSearchServices/SearchService.h"

ActionStatus ActionDispatcher::dispatch(const Actions::Action& action) {
    return std::visit(Actions::Overloaded{
            [](const Actions::InputData& input)     { return dispatchInput(input); },
            [](const Actions::FileData& file)       { return dispatchFile(file); },
            [](const Actions::SystemData& system)   { return dispatchSystem(system); },
            [](const Actions::ControlData& control) { return dispatchControl(control); }
    }, action);
}

ActionStatus ActionDispatcher::dispatchInput(const Actions::InputData& input) {
    return std::visit(Actions::Overloaded{
            [](const Actions::MoveMouse& m)   {
                try{
                    MouseService::getInstance().moveMouse(m.x, m.y);
                    return ActionStatus::Success;
                }
                catch(...){
                    return ActionStatus::Failed;
                }
            },
            [](const Actions::Click& c)       {
                try{
                    MouseService::getInstance().clickMouse(c.button);
                    return ActionStatus::Success;
                }
                catch(...){
                    return ActionStatus::Failed;
                }
                },
            [](const Actions::DoubleClick& d) { /* return InputService::getInstance().doubleClick(d.button); */ return ActionStatus::Success; },
            [](const Actions::Type& t)        { /* return InputService::getInstance().typeText(t.text); */ return ActionStatus::Success; },
            [](const Actions::KeyPress& k)    { /* return InputService::getInstance().keyPress(k.key); */ return ActionStatus::Success; },
            [](const Actions::Scroll& s)      { /* return InputService::getInstance().scroll(s.amount); */ return ActionStatus::Success; },
            [](const Actions::Hotkey& h)      { /* return InputService::getInstance().hotkey(h.keys); */ return ActionStatus::Success; },
            [](const Actions::MouseDown& md)  { /* return InputService::getInstance().mouseDown(md.button); */ return ActionStatus::Success; },
            [](const Actions::MouseUp& mu)    { /* return InputService::getInstance().mouseUp(mu.button); */ return ActionStatus::Success; },
            [](const Actions::DragMouse& dm)  { /* return InputService::getInstance().dragMouse(dm.start_x, dm.start_y, dm.end_x, dm.end_y); */ return ActionStatus::Success; }
    }, input);
}

ActionStatus ActionDispatcher::dispatchFile(const Actions::FileData& file) {
    auto& fs = FileService::getInstance();

    auto toPathStr = [](const auto& p) -> std::string {
        if constexpr (requires { p.string(); }) {
            return p.string(); //std::filesystem::path
        } else {
            return std::string(p);
        }
    };

    auto execute = [&](std::string_view actionName, const auto& targetPath, auto&& operation) -> ActionStatus {
        try {
            operation();
            return ActionStatus::Success;
        }
        catch (const FileServiceException& e) {
            WorldStateBuilderService::getInstance().pushActionResult(
                    "[" + std::string(actionName) + " Failed] Target: " + toPathStr(targetPath) + " | Error: " + e.what()
            );
            return ActionStatus::Failed;
        }
        catch (const std::exception& e) {
            WorldStateBuilderService::getInstance().pushActionResult(
                    "[" + std::string(actionName) + " Failed] Target: " + toPathStr(targetPath) + " | System Error: " + e.what()
            );
            return ActionStatus::Failed;
        }
        catch (...) {
            WorldStateBuilderService::getInstance().pushActionResult(
                    "[" + std::string(actionName) + " Failed] Target: " + toPathStr(targetPath) + " | Fatal: Unknown Error"
            );
            return ActionStatus::Failed;
        }
    };

    return std::visit(Actions::Overloaded{
            [&](const Actions::CreateFile& c) {
                return execute("CreateFile", c.path, [&] { fs.createFile(c.path, c.text); });
            },
            [&](const Actions::WriteFile& w) {
                return execute("WriteFile", w.path, [&] { fs.writeFile(w.path, w.text); });
            },
            [&](const Actions::AppendFile& a) {
                return execute("AppendFile", a.path, [&] { fs.appendFile(a.path, a.text); });
            },
            [&](const Actions::InsertFile& i) {
                return execute("InsertFile", i.path, [&] { fs.insertFile(i.path, i.position, i.text); });
            },
            [&](const Actions::DeleteFile& d) {
                return execute("DeleteFile", d.path, [&] { fs.deleteFile(d.path); });
            },
            [&](const Actions::RenameFile& r) {
                return execute("RenameFile", r.path, [&] { fs.renameFile(r.path, r.new_path); });
            },
            [&](const Actions::CopyFile& c) {
                return execute("CopyFile", c.path, [&] { fs.copyFile(c.path, c.destination); });
            },
            [&](const Actions::MoveFile& m) {
                return execute("MoveFile", m.path, [&] { fs.moveFile(m.path, m.destination); });
            },
            [&](const Actions::ApplyBlockDiff& abd) {
                return execute("ApplyBlockDiff", abd.path, [&] { fs.applyDiff(abd.path, abd.edits); });
            },
            [&](const Actions::EditFile& ef) {
                return execute("EditFile", ef.path, [&] { fs.editFile(ef.path, ef.edits); });
            }
    }, file);
}

ActionStatus ActionDispatcher::dispatchSystem(const Actions::SystemData& system) {
    return std::visit(Actions::Overloaded{
            [](const Actions::RunCmd& r){
                    ActionStatus status = SystemService::getInstance().runCommand(r);
                    WorldStateBuilderService::getInstance().pushActionResult("[CMD: " + r.command +"] Result: " + r.output);
                    return status;
                },
            [](const Actions::RunPowerShell& r){
                ActionStatus status = SystemService::getInstance().runPowerShell(r);
                WorldStateBuilderService::getInstance().pushActionResult("[PowerShell: " + r.command +"] Result: " + r.output);
                return status;
                },
            [](const Actions::OpenApp& o)        { /* return SystemService::getInstance().openApp(o.name); */ return ActionStatus::Success; },
            [](const Actions::CloseApp& c)       { /* return SystemService::getInstance().closeApp(c.name); */ return ActionStatus::Success; },
            [](const Actions::FocusWindow& f)    { /* return SystemService::getInstance().focusWindow(f.name); */ return ActionStatus::Success; },
            [](const Actions::MinimizeWindow& m) { /* return SystemService::getInstance().minimizeWindow(m.name); */ return ActionStatus::Success; },
            [](const Actions::MaximizeWindow& m) { /* return SystemService::getInstance().maximizeWindow(m.name); */ return ActionStatus::Success; },
            [](const Actions::RestoreWindow& r)  { /* return SystemService::getInstance().restoreWindow(r.name); */ return ActionStatus::Success; },
            [](const Actions::SetVolume& s)      { /* return SystemService::getInstance().setVolume(s.value); */ return ActionStatus::Success; },
            [](const Actions::MuteVolume& m)     { /* return SystemService::getInstance().muteVolume(); */ return ActionStatus::Success; },
            [](const Actions::UnmuteVolume& u)   { /* return SystemService::getInstance().unmuteVolume(); */ return ActionStatus::Success; },
            [](const Actions::Sleep& s)          { /* return SystemService::getInstance().sleep(); */ return ActionStatus::Success; },
            [](const Actions::Shutdown& s)       { /* return SystemService::getInstance().shutdown(); */ return ActionStatus::Success; },
            [](const Actions::Restart& r)        { /* return SystemService::getInstance().restart(); */ return ActionStatus::Success; }
    }, system);
}

ActionStatus ActionDispatcher::dispatchControl(const Actions::ControlData& control) {
    return std::visit(Actions::Overloaded{
            [](const Actions::Msg& m)        { /* return ControlService::getInstance().showMessage(m.content); */ return ActionStatus::Success; },

            // SPECIAL CASE: The Observe action returns a specific status to break the orchestrator loop
            [](const Actions::Observe& o)    {
                /* ControlService::getInstance().triggerObservePrep(); */
                return ActionStatus::TriggerObserve;
            },

            [](const Actions::Wait& w)       { /* return ControlService::getInstance().wait(w.value); */ return ActionStatus::Success; },
            [](const Actions::FAR& r)        {
                bool status = WorldStateBuilderService::getInstance().fileAnalyzeRequest(r.path);
                return (status) ? ActionStatus::Success : ActionStatus::Failed;
                },
            [](const Actions::IsVerified& i) { /* return ControlService::getInstance().setVerified(i.value); */ return ActionStatus::Success; },
            [](const Actions::ClearStack& c) { /* return ControlService::getInstance().clearStack(); */ return ActionStatus::Success; },
            [](const Actions::SearchWeb& sw) {
                //TODO: use getApiKey after implementing user settings
                const std::string myApiKey = "tvly-";
                const int creditLimit = 3;
                WebSearch::SearchConfig config{myApiKey, creditLimit};
                WebSearch::SearchService service(config);
                try {
                    WebSearch::SearchResponse response = service.search(sw.query, sw.max_result);
                    WorldStateBuilderService::getInstance().pushActionResult(response.to_llm_context());
                    return ActionStatus::Success;
                }
                catch (const WebSearch::AllProvidersFailedException& e) {
                    WorldStateBuilderService::getInstance().pushActionResult("[Web Search Failed] " + e.getDetailedReport());
                    return ActionStatus::Failed;
                }
                catch (const std::exception& e) {
                    WorldStateBuilderService::getInstance().pushActionResult("[Web Search Failed] " + std::string(e.what()));
                    return ActionStatus::Failed;
                }
                catch (...) {
                    WorldStateBuilderService::getInstance().pushActionResult("[Web Search Failed] Unknown Error");
                    return ActionStatus::Failed;
                }
            }
    }, control);
}