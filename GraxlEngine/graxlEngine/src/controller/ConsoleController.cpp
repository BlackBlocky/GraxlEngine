#include "graxlEngine/controller/ConsoleController.h"

#include <memory>
#include <format>

#include <imgui.h>
#include <spdlog/spdlog.h>

#include "graxlEngine/event/GuiSwitchSceneElementDebugWindow.h"
#include "graxlEngine/event/GameStateSwitchEvent.h"

#include "graxlEngine/controller/CoreController.h"
#include <graxlEngine/GraxlConstants.h>

namespace graxl {

ConsoleController::ConsoleController(CoreController& coreControllerRef) : _coreControllerRef(coreControllerRef)
{
    // Just add a "header"
    _consoleLogs.push_back("++++ Graxl Console ++++");
    _consoleLogs.push_back("Console initialized. Type 'help' for commands.");
}

void ConsoleController::consoleImGuiUpdate(const DoubleEventBuffer& doubleEventBuffer)
{
    ImGui::Begin("Graxl Debug Console");

    // Console scroll view
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
    for (const auto& log : _consoleLogs)
        ImGui::TextUnformatted(log.c_str());
    ImGui::PopStyleColor();
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);  // Auto-scroll to bottom
    ImGui::EndChild();

    ImGui::Separator();

    // Console input
    if (ImGui::InputText("Input", _inputBuffer, sizeof(_inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string enteredComamnd(_inputBuffer);
        if (!enteredComamnd.empty()) {
            _consoleLogs.push_back(">> " + enteredComamnd);
            _inputBuffer[0] = '\0'; // Clear input buffer after processing
            processCommand(enteredComamnd, doubleEventBuffer);
        }
    }

    ImGui::End();
}

void ConsoleController::processCommand(std::string command, const DoubleEventBuffer& doubleEventBuffer)
{
    // Please dont judge me for that "else if" hell, i'll improve it later trustttt o.O

    spdlog::info("ConsoleController: Command entered to console: {}", command);

    if (command.starts_with("help")) {
        _consoleLogs.push_back("Available commands:");
        _consoleLogs.push_back(" - help: Show this help message");
        _consoleLogs.push_back(" - clear: Clear the console");
        _consoleLogs.push_back(" - about: Show information about GrashCpp");
        _consoleLogs.push_back(" - gui (open/close) (shader) [shadername]");
        _consoleLogs.push_back(" - state");
        _consoleLogs.push_back(" - \\-> list: lists every defined state");
        _consoleLogs.push_back(" - \\-> set [id]: changes the state to given id");

        spdlog::info("ConsoleController: Console command 'help' executed.");
    }

    else if (command.starts_with("clear")) {
        _consoleLogs.clear();
        _consoleLogs.push_back("Console cleared.");

        spdlog::info("ConsoleController: Console command 'help' executed.");
    }

    else if (command.starts_with("about")) {
        _consoleLogs.push_back(std::format("Graxl. A game engine written by BlackBlocky.\n- Current version: {}", info::GRAXL_ENGINE_VERSION));

        spdlog::info("ConsoleController: Console command 'about' executed.");
    }

    else if (command.starts_with("gui ")) {
		std::string guiCommand = command.substr(4); // Get the part after "gui "

        if (guiCommand.starts_with("open ")) {
			guiCommand = guiCommand.substr(5); // Get the part after "open "

			if (guiCommand.length() >= 7+1 && guiCommand.starts_with("shader ")) {
				guiCommand = guiCommand.substr(7); // Get the part after "shader "
                doubleEventBuffer.pushEvent(
                    std::make_shared<GuiSwitchSceneElementDebugWindow>(
                        GuiSwitchSceneElementDebugWindow("ConsoleController.cpp", GuiSwitchSceneElementDebugWindow::DebugWindowElement::Shader, guiCommand, false)
                    )
                );

                _consoleLogs.push_back("Open shader GUI: " + guiCommand);
				spdlog::info("ConsoleController: Open shader GUI command executed for shader: {}", guiCommand);
			}
            else if (guiCommand.length() >= 8+1 && guiCommand.starts_with("texture ")) {
				guiCommand = guiCommand.substr(8); // Get the part after "texture "
                doubleEventBuffer.pushEvent(
                    std::make_shared<GuiSwitchSceneElementDebugWindow>(
                        GuiSwitchSceneElementDebugWindow("ConsoleController.cpp", GuiSwitchSceneElementDebugWindow::DebugWindowElement::Texture, guiCommand, false)
                    )
                );
				_consoleLogs.push_back("Open texture GUI: " + guiCommand);
            }
            else {
				_consoleLogs.push_back("Unkown gui shader command!");
				spdlog::info("ConsoleController: Unknown gui command entered in console: {}", command);
            }      
        }
		else if (guiCommand.starts_with("close ")) {
			guiCommand = guiCommand.substr(6); // Get the part after "close "

			if (guiCommand.length() >= 7+1 && guiCommand.starts_with("shader ")) {
				guiCommand = guiCommand.substr(7); // Get the part after "shader "
                doubleEventBuffer.pushEvent(
                    std::make_shared<GuiSwitchSceneElementDebugWindow>(
                        GuiSwitchSceneElementDebugWindow("ConsoleController.cpp", GuiSwitchSceneElementDebugWindow::DebugWindowElement::Shader, guiCommand, true)
                    )
                );

                _consoleLogs.push_back("Close shader GUI: " + guiCommand);
				spdlog::info("ConsoleController: Close shader GUI command executed for shader: {}", guiCommand);
			}
            else if (guiCommand.length() >= 8 + 1 && guiCommand.starts_with("texture ")) {
                guiCommand = guiCommand.substr(8); // Get the part after "texture "
                doubleEventBuffer.pushEvent(
                    std::make_shared<GuiSwitchSceneElementDebugWindow>(
                        GuiSwitchSceneElementDebugWindow("ConsoleController.cpp", GuiSwitchSceneElementDebugWindow::DebugWindowElement::Texture, guiCommand, true)
                    )
                );
                _consoleLogs.push_back("Open texture GUI: " + guiCommand);
            }
            else {
				_consoleLogs.push_back("Unkown gui shader command!");
				spdlog::info("ConsoleController: Unknown gui command entered in console: {}", command);
            }     
        }
        else {
			_consoleLogs.push_back("Unkown gui command!");
			spdlog::info("ConsoleController: Unknown gui command entered in console: {}", command);
        }
    }

    else if (command.starts_with("state ")) {
        std::string stateCommand = command.substr(6);

        if (stateCommand.starts_with("list")) {
            for (const auto nState : _coreControllerRef.getDefinedGameStates()) {
                _consoleLogs.push_back(".." + nState.second->id + ", doRender=" + std::to_string(nState.second->doRender) +
                    ", doInput=" + std::to_string(nState.second->doInput) + ", haveUpdateFunc=" + std::to_string(nState.second->updateFunction != nullptr));
            }

            _consoleLogs.push_back("Done Listing.");
            spdlog::info("ConsoleController: Listed all GameStates");
        }
        else if (stateCommand.starts_with("set ")) {
            stateCommand = stateCommand.substr(4);

            doubleEventBuffer.pushEvent(std::make_shared<GameStateSwitchEvent>(GameStateSwitchEvent("ConsoleController.cpp", stateCommand)));

            _consoleLogs.push_back("Switching to state " + stateCommand);
            spdlog::info("ConsoleController: Swtiching Game State to {}", stateCommand);
        }
        else {
            _consoleLogs.push_back("Unknown state command: " + stateCommand);
            spdlog::info("ConsoleController: Unknown state command {}", stateCommand);
        }
    }

    else {
        _consoleLogs.push_back("Unknown command: " + command);

        spdlog::info("ConsoleController: Unknown command entered in console: {}", command);
    }
}

};