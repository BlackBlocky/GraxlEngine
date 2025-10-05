#include "graxlEngine/InputSystem.h"

#include <raylib.h>
#include <spdlog/spdlog.h>

namespace graxl {

void InputSystem::checkForInputs(const DoubleEventBuffer& dEventBuffer)
{
    int pressedKey = GetKeyPressed();
    if (pressedKey != KeyboardKey::KEY_NULL) {
        dEventBuffer.pushEvent(std::make_shared<const GameKeyEvent>(GameKeyEvent("InputSystem.cpp", pressedKey, true)));
    }

}

std::string InputSystem::getPressedKeyName(int keyboardKey)
{
    switch (keyboardKey) {
    case KEY_NULL: return "KEY_NULL";
    case KEY_APOSTROPHE: return "KEY_APOSTROPHE";
    case KEY_COMMA: return "KEY_COMMA";
    case KEY_MINUS: return "KEY_MINUS";
    case KEY_PERIOD: return "KEY_PERIOD";
    case KEY_SLASH: return "KEY_SLASH";
    case KEY_ZERO: return "KEY_ZERO";
    case KEY_ONE: return "KEY_ONE";
    case KEY_TWO: return "KEY_TWO";
    case KEY_THREE: return "KEY_THREE";
    case KEY_FOUR: return "KEY_FOUR";
    case KEY_FIVE: return "KEY_FIVE";
    case KEY_SIX: return "KEY_SIX";
    case KEY_SEVEN: return "KEY_SEVEN";
    case KEY_EIGHT: return "KEY_EIGHT";
    case KEY_NINE: return "KEY_NINE";
    case KEY_SEMICOLON: return "KEY_SEMICOLON";
    case KEY_EQUAL: return "KEY_EQUAL";
    case KEY_A: return "KEY_A";
    case KEY_B: return "KEY_B";
    case KEY_C: return "KEY_C";
    case KEY_D: return "KEY_D";
    case KEY_E: return "KEY_E";
    case KEY_F: return "KEY_F";
    case KEY_G: return "KEY_G";
    case KEY_H: return "KEY_H";
    case KEY_I: return "KEY_I";
    case KEY_J: return "KEY_J";
    case KEY_K: return "KEY_K";
    case KEY_L: return "KEY_L";
    case KEY_M: return "KEY_M";
    case KEY_N: return "KEY_N";
    case KEY_O: return "KEY_O";
    case KEY_P: return "KEY_P";
    case KEY_Q: return "KEY_Q";
    case KEY_R: return "KEY_R";
    case KEY_S: return "KEY_S";
    case KEY_T: return "KEY_T";
    case KEY_U: return "KEY_U";
    case KEY_V: return "KEY_V";
    case KEY_W: return "KEY_W";
    case KEY_X: return "KEY_X";
    case KEY_Y: return "KEY_Y";
    case KEY_Z: return "KEY_Z";
    case KEY_LEFT_BRACKET: return "KEY_LEFT_BRACKET";
    case KEY_BACKSLASH: return "KEY_BACKSLASH";
    case KEY_RIGHT_BRACKET: return "KEY_RIGHT_BRACKET";
    case KEY_GRAVE: return "KEY_GRAVE";
    case KEY_SPACE: return "KEY_SPACE";
    case KEY_ESCAPE: return "KEY_ESCAPE";
    case KEY_ENTER: return "KEY_ENTER";
    case KEY_TAB: return "KEY_TAB";
    case KEY_BACKSPACE: return "KEY_BACKSPACE";
    case KEY_INSERT: return "KEY_INSERT";
    case KEY_DELETE: return "KEY_DELETE";
    case KEY_RIGHT: return "KEY_RIGHT";
    case KEY_LEFT: return "KEY_LEFT";
    case KEY_DOWN: return "KEY_DOWN";
    case KEY_UP: return "KEY_UP";
    case KEY_PAGE_UP: return "KEY_PAGE_UP";
    case KEY_PAGE_DOWN: return "KEY_PAGE_DOWN";
    case KEY_HOME: return "KEY_HOME";
    case KEY_END: return "KEY_END";
    case KEY_CAPS_LOCK: return "KEY_CAPS_LOCK";
    case KEY_SCROLL_LOCK: return "KEY_SCROLL_LOCK";
    case KEY_NUM_LOCK: return "KEY_NUM_LOCK";
    case KEY_PRINT_SCREEN: return "KEY_PRINT_SCREEN";
    case KEY_PAUSE: return "KEY_PAUSE";
    case KEY_F1: return "KEY_F1";
    case KEY_F2: return "KEY_F2";
    case KEY_F3: return "KEY_F3";
    case KEY_F4: return "KEY_F4";
    case KEY_F5: return "KEY_F5";
    case KEY_F6: return "KEY_F6";
    case KEY_F7: return "KEY_F7";
    case KEY_F8: return "KEY_F8";
    case KEY_F9: return "KEY_F9";
    case KEY_F10: return "KEY_F10";
    case KEY_F11: return "KEY_F11";
    case KEY_F12: return "KEY_F12";
    case KEY_LEFT_SHIFT: return "KEY_LEFT_SHIFT";
    case KEY_LEFT_CONTROL: return "KEY_LEFT_CONTROL";
    case KEY_LEFT_ALT: return "KEY_LEFT_ALT";
    case KEY_LEFT_SUPER: return "KEY_LEFT_SUPER";
    case KEY_RIGHT_SHIFT: return "KEY_RIGHT_SHIFT";
    case KEY_RIGHT_CONTROL: return "KEY_RIGHT_CONTROL";
    case KEY_RIGHT_ALT: return "KEY_RIGHT_ALT";
    case KEY_RIGHT_SUPER: return "KEY_RIGHT_SUPER";
    case KEY_KB_MENU: return "KEY_KB_MENU";
    case KEY_KP_0: return "KEY_KP_0";
    case KEY_KP_1: return "KEY_KP_1";
    case KEY_KP_2: return "KEY_KP_2";
    case KEY_KP_3: return "KEY_KP_3";
    case KEY_KP_4: return "KEY_KP_4";
    case KEY_KP_5: return "KEY_KP_5";
    case KEY_KP_6: return "KEY_KP_6";
    case KEY_KP_7: return "KEY_KP_7";
    case KEY_KP_8: return "KEY_KP_8";
    case KEY_KP_9: return "KEY_KP_9";
    case KEY_KP_DECIMAL: return "KEY_KP_DECIMAL";
    case KEY_KP_DIVIDE: return "KEY_KP_DIVIDE";
    case KEY_KP_MULTIPLY: return "KEY_KP_MULTIPLY";
    case KEY_KP_SUBTRACT: return "KEY_KP_SUBTRACT";
    case KEY_KP_ADD: return "KEY_KP_ADD";
    case KEY_KP_ENTER: return "KEY_KP_ENTER";
    case KEY_KP_EQUAL: return "KEY_KP_EQUAL";
    case KEY_BACK: return "KEY_BACK";
    case KEY_MENU: return "KEY_MENU";
    case KEY_VOLUME_UP: return "KEY_VOLUME_UP";
    case KEY_VOLUME_DOWN: return "KEY_VOLUME_DOWN";
    default: return "UNKNOWN_KEY";
    }
}

};
