#pragma once

#include <string>
#include <variant>
#include "Actions.h"

enum class PermissionLevel {
    Allowed,
    RequiresConfirmation,
    Denied
};

class PermissionValidator {
public:
    static PermissionLevel validate(const Actions::Action& action) {
        return std::visit(Actions::Overloaded{
            [](const Actions::InputData& input) {
                return validateInput(input);
            },
            [](const Actions::FileData& file) {
                return validateFile(file);
            },
            [](const Actions::SystemData& sys) {
                return validateSystem(sys);
            },
            [](const Actions::ControlData& ctrl) {
                return validateControl(ctrl);
            }
        }, action);
    }

private:
    static PermissionLevel validateInput(const Actions::InputData&) {
        return PermissionLevel::Allowed;
    }

    static PermissionLevel validateFile(const Actions::FileData& file) {
        return std::visit(Actions::Overloaded{
            [](const Actions::DeleteFile&) {
                return PermissionLevel::RequiresConfirmation;
            },
            [](const Actions::EditFile&) {
                return PermissionLevel::RequiresConfirmation;
            },
            [](const Actions::ApplyBlockDiff&) {
                return PermissionLevel::RequiresConfirmation;
            },
            [](const Actions::AppendFile&) {
                return PermissionLevel::RequiresConfirmation;
            },
            [](const auto&) {
                return PermissionLevel::Allowed;
            }
        }, file);
    }

    static PermissionLevel validateSystem(const Actions::SystemData& sys) {
        return std::visit(Actions::Overloaded{
            [](const Actions::RunCmd&) {
                return PermissionLevel::RequiresConfirmation;
            },
            [](const Actions::RunPowerShell&) {
                return PermissionLevel::RequiresConfirmation;
            },
            [](const Actions::Shutdown&) {
                return PermissionLevel::RequiresConfirmation;
            },
            [](const Actions::Restart&) {
                return PermissionLevel::RequiresConfirmation;
            },
            [](const Actions::CloseApp&) {
                return PermissionLevel::RequiresConfirmation;
            },
            [](const auto&) {
                return PermissionLevel::Allowed;
            }
        }, sys);
    }

    static PermissionLevel validateControl(const Actions::ControlData&) {
        return PermissionLevel::Allowed;
    }
};