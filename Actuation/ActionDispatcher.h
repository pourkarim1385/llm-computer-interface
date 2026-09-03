#pragma once
#include "Actions.h"
#include "ActionExecutorServices/SystemService.h"

class ActionDispatcher {
public:
    // Prevent instantiation since this is a pure static utility class
    ActionDispatcher() = delete;

    // Top-level static router
    static ActionStatus dispatch(const Actions::Action& action);

private:
    // Exhaustive domain-level static routers
    static ActionStatus dispatchInput(const Actions::InputData& input);
    static ActionStatus dispatchFile(const Actions::FileData& file);
    static ActionStatus dispatchSystem(const Actions::SystemData& system);
    static ActionStatus dispatchControl(const Actions::ControlData& control);
};