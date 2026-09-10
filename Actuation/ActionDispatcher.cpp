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
            [](const Actions::InputData& input)     { return ActionDispatcher::dispatchInput(input); },
            [](const Actions::FileData& file)       { return ActionDispatcher::dispatchFile(file); },
            [](const Actions::SystemData& system)   { return ActionDispatcher::dispatchSystem(system); },
            [](const Actions::ControlData& control) { return ActionDispatcher::dispatchControl(control); }
    }, action);
}

ActionStatus ActionDispatcher::dispatchInput(const Actions::InputData& input) {
    // Wrapper lambda to handle all the repetitive try/catch and error logging logic
    auto execute = [](const char* actionName, auto&& actionFunc) {
        try {
            actionFunc();
            return ActionStatus::Ok;
        }
        catch (...) {
            WorldStateBuilderService::getInstance().pushActionResult(
                std::string("[ ") + actionName + " Failed] | Fatal: Unknown Error"
            );
            return ActionStatus::Failed;
        }
        };

    return std::visit(Actions::Overloaded{
        [&](const Actions::MoveMouse& m) { return execute("MoveMouse", [&] { MouseService::getInstance().moveMouse(m.x, m.y); }); },
        [&](const Actions::Click& c) { return execute("ClickMouse", [&] { MouseService::getInstance().clickMouse(c.button); }); },
        [&](const Actions::DoubleClick& d) { return execute("DoubleClickMouse", [&] { MouseService::getInstance().clickMouse(d.button); }); },
        [&](const Actions::Type& t) { return execute("TypeClipboardService", [&] { ClipboardService::getInstance().type(t.text); }); },
        [&](const Actions::PressKey& k) { return execute("PressKeyClipboardService", [&] { ClipboardService::getInstance().keyPress(k.key); }); },
        [&](const Actions::Scroll& s) { return execute("ScrollMouse", [&] { MouseService::getInstance().scrollMouse(s.direction, s.amount); }); },
        [&](const Actions::Hotkey& h) { return execute("HotkeyClipboardService", [&] { ClipboardService::getInstance().hotKey(h.keys); }); },
        [&](const Actions::MouseDown& md) { return execute("MouseDown", [&] { MouseService::getInstance().mouseClickHold(md.button); }); },
        [&](const Actions::MouseUp& mu) { return execute("MouseUp", [&] { MouseService::getInstance().mouseClickRelease(mu.button); }); },
        [&](const Actions::DragMouse& dm) { return execute("DragMouse", [&] { MouseService::getInstance().dragMouse(dm.start_x, dm.start_y, dm.duration, dm.step); }); }
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
            return ActionStatus::Ok;
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
            [](const Actions::OpenApp& o)        { /* return SystemService::getInstance().openApp(o.name); */ return ActionStatus::Ok; },
            [](const Actions::CloseApp& c)       { /* return SystemService::getInstance().closeApp(c.name); */ return ActionStatus::Ok; },
            [](const Actions::FocusWindow& f)    { /* return SystemService::getInstance().focusWindow(f.name); */ return ActionStatus::Ok; },
            [](const Actions::MinimizeWindow& m) { /* return SystemService::getInstance().minimizeWindow(m.name); */ return ActionStatus::Ok; },
            [](const Actions::MaximizeWindow& m) { /* return SystemService::getInstance().maximizeWindow(m.name); */ return ActionStatus::Ok; },
            [](const Actions::RestoreWindow& r)  { /* return SystemService::getInstance().restoreWindow(r.name); */ return ActionStatus::Ok; },
            [](const Actions::SetVolume& s)      { /* return SystemService::getInstance().setVolume(s.value); */ return ActionStatus::Ok; },
            [](const Actions::MuteVolume& m)     { /* return SystemService::getInstance().muteVolume(); */ return ActionStatus::Ok; },
            [](const Actions::UnmuteVolume& u)   { /* return SystemService::getInstance().unmuteVolume(); */ return ActionStatus::Ok; },
            [](const Actions::Sleep& s)          { /* return SystemService::getInstance().sleep(); */ return ActionStatus::Ok; },
            [](const Actions::Shutdown& s)       { /* return SystemService::getInstance().shutdown(); */ return ActionStatus::Ok; },
            [](const Actions::Restart& r)        { /* return SystemService::getInstance().restart(); */ return ActionStatus::Ok; }
    }, system);
}

ActionStatus ActionDispatcher::dispatchControl(const Actions::ControlData& control) {
    return std::visit(Actions::Overloaded{
            [](const Actions::Observe& o)    {
                return ActionStatus::TriggerObserve;
            },
            [](const Actions::Wait& w)       { /* return ControlService::getInstance().wait(w.value); */ return ActionStatus::Ok; },
            [](const Actions::FAR& r)        {
                bool status = WorldStateBuilderService::getInstance().fileAnalyzeRequest(r.path);
                return (status) ? ActionStatus::Ok : ActionStatus::Failed;
                },
            [](const Actions::ClearStack& c) { return ActionStatus::Ok; },
            [](const Actions::SearchWeb& sw) {
                WebSearch::SearchService service(sw.config);
                try {
                    WebSearch::SearchResponse response = service.search(sw.query, sw.max_result);
                    WorldStateBuilderService::getInstance().pushActionResult(response.to_llm_context());
                    return ActionStatus::Ok;
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