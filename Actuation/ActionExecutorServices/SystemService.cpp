#include "SystemService.h"

SystemService& SystemService::getInstance() {
    static SystemService instance;
    return instance;
}

ActionStatus SystemService::runCommand(const Actions::RunCmd& action) {
    CommandResult result = cmd.execute(action.command, terminalType::cmd);

    action.output = result.output;
    action.cycle = result.elapsedCycles;
    return (result.exitCode == 0 && !result.timedOut) ? ActionStatus::Success : ActionStatus::Failed;
}

ActionStatus SystemService::runPowerShell(const Actions::RunPowerShell& action) {
    CommandResult result = cmd.execute(action.command, terminalType::shell);

    action.output = result.output;
    action.cycle = result.elapsedCycles;

    return (result.exitCode == 0 && !result.timedOut) ? ActionStatus::Success : ActionStatus::Failed;
}