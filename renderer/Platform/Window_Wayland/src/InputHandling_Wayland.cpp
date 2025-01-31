//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Window_Wayland/InputHandling_Wayland.h"
#include "Utils/ThreadLocalLogForced.h"
#include "Utils/Warnings.h"
#include <linux/input.h>

namespace ramses_internal
{
    InputHandling_Wayland::InputHandling_Wayland(IWindowEventHandler& windowEventHandler)
    : m_windowEventHandler(windowEventHandler)
    {
    }

    void InputHandling_Wayland::registerSeat(wl_registry* wl_registry, uint32_t name)
    {
        LOG_INFO(CONTEXT_RENDERER, "InputHandling_Wayland::registerSeat");

        if (m_seat == nullptr)
        {
            const uint32_t seatInterfaceVersion = 1;
            m_seat = static_cast<wl_seat*>(wl_registry_bind(wl_registry, name, &wl_seat_interface, seatInterfaceVersion));
            wl_seat_add_listener(m_seat, &m_seatListener, this);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "InputHandling_Wayland::registerSeat Only a single seat is supported!");
        }
    }

    void InputHandling_Wayland::deinit()
    {
        if (m_pointer != nullptr)
        {
            wl_pointer_destroy(m_pointer);
            m_pointer = nullptr;
        }

        if (m_keyboard != nullptr)
        {
            wl_keyboard_destroy(m_keyboard);
            m_keyboard = nullptr;
        }

        if (m_seat != nullptr)
        {
            wl_seat_destroy(m_seat);
            m_seat = nullptr;
        }
    }

    EKeyCode InputHandling_Wayland::RamsesKeyCodeFromWaylandKey(uint32_t waylandKey)
    {
        static const EKeyCode waylandKeyToRamsesKeyCodeTable[] =
        {
            /*   0-9 */   EKeyCode_Unknown,    EKeyCode_Escape,     EKeyCode_1,              EKeyCode_2,               EKeyCode_3,               EKeyCode_4,                EKeyCode_5,            EKeyCode_6,            EKeyCode_7,             EKeyCode_8,
            /*  10-19 */  EKeyCode_9,          EKeyCode_0,          EKeyCode_Minus,          EKeyCode_Equals,          EKeyCode_Backspace,       EKeyCode_Tab,              EKeyCode_Q,            EKeyCode_W,            EKeyCode_E,             EKeyCode_R,
            /*  20-29 */  EKeyCode_T,          EKeyCode_Z,          EKeyCode_U,              EKeyCode_I,               EKeyCode_O,               EKeyCode_P,                EKeyCode_LeftBracket,  EKeyCode_RightBracket, EKeyCode_Return,        EKeyCode_ControlLeft,
            /*  30-39 */  EKeyCode_A,          EKeyCode_S,          EKeyCode_D,              EKeyCode_F,               EKeyCode_G,               EKeyCode_H,                EKeyCode_J,            EKeyCode_K,            EKeyCode_L,             EKeyCode_Semicolon,
            /*  40-49 */  EKeyCode_Apostrophe, EKeyCode_Grave,      EKeyCode_ShiftLeft,      EKeyCode_NumberSign,      EKeyCode_Y,               EKeyCode_X,                EKeyCode_C,            EKeyCode_V,            EKeyCode_B,             EKeyCode_N,
            /*  50-59 */  EKeyCode_M,          EKeyCode_Comma,      EKeyCode_Period,         EKeyCode_Slash,           EKeyCode_ShiftRight,      EKeyCode_Numpad_Multiply,  EKeyCode_AltLeft,      EKeyCode_Space,        EKeyCode_CapsLock,      EKeyCode_F1,
            /*  60-69 */  EKeyCode_F2,         EKeyCode_F3,         EKeyCode_F4,             EKeyCode_F5,              EKeyCode_F6,              EKeyCode_F7,               EKeyCode_F8,           EKeyCode_F9,           EKeyCode_F10,           EKeyCode_NumLock,
            /*  70-79 */  EKeyCode_ScrollLock, EKeyCode_Numpad_7,   EKeyCode_Numpad_8,       EKeyCode_Numpad_9,        EKeyCode_Numpad_Subtract, EKeyCode_Numpad_4,         EKeyCode_Numpad_5,     EKeyCode_Numpad_6,     EKeyCode_Numpad_Add,    EKeyCode_Numpad_1,
            /*  80-89 */  EKeyCode_Numpad_2,   EKeyCode_Numpad_3,   EKeyCode_Numpad_0,       EKeyCode_Numpad_Decimal,  EKeyCode_Unknown,         EKeyCode_Numpad_4,         EKeyCode_Backslash,    EKeyCode_F11,          EKeyCode_F12,           EKeyCode_Unknown,
            /*  90-99 */  EKeyCode_Unknown,    EKeyCode_Unknown,    EKeyCode_Unknown,        EKeyCode_Unknown,         EKeyCode_Unknown,         EKeyCode_Unknown,          EKeyCode_Numpad_Enter, EKeyCode_ControlRight, EKeyCode_Numpad_Divide, EKeyCode_PrintScreen,
            /* 100-109 */ EKeyCode_AltRight,   EKeyCode_Unknown,    EKeyCode_Home,           EKeyCode_Up,              EKeyCode_PageUp,          EKeyCode_Left,             EKeyCode_Right,        EKeyCode_End,          EKeyCode_Down,          EKeyCode_PageDown,
            /* 110-119 */ EKeyCode_Insert,     EKeyCode_Delete,     EKeyCode_Unknown,        EKeyCode_Unknown,         EKeyCode_Unknown,         EKeyCode_Unknown,          EKeyCode_Unknown,      EKeyCode_Unknown,      EKeyCode_Unknown,       EKeyCode_Pause,
            /* 120-129 */ EKeyCode_Unknown,    EKeyCode_Unknown,    EKeyCode_Unknown,        EKeyCode_Unknown,         EKeyCode_Unknown,         EKeyCode_WindowsLeft,      EKeyCode_WindowsRight, EKeyCode_Menu,         EKeyCode_Unknown,       EKeyCode_Unknown
        };

        static const uint32_t numKeys = sizeof(waylandKeyToRamsesKeyCodeTable)/sizeof(waylandKeyToRamsesKeyCodeTable[0]);
        if (waylandKey >= numKeys)
        {
            return EKeyCode_Unknown;
        }
        return waylandKeyToRamsesKeyCodeTable[waylandKey];
    }


    void InputHandling_Wayland::PointerHandleEnter(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy)
    {
        UNUSED(data);
        UNUSED(pointer);
        UNUSED(serial);
        UNUSED(surface);
        UNUSED(sx);
        UNUSED(sy);
    }

    void InputHandling_Wayland::PointerHandleLeave(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface)
    {
        UNUSED(data);
        UNUSED(pointer);
        UNUSED(serial);
        UNUSED(surface);
    }

    void InputHandling_Wayland::PointerHandleMotion(void *data, wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
    {
        UNUSED(pointer);
        UNUSED(time);

        InputHandling_Wayland& inputHandling = *static_cast<InputHandling_Wayland*>(data);
        inputHandling.m_cursorPosX = wl_fixed_to_double(sx);
        inputHandling.m_cursorPosY = wl_fixed_to_double(sy);
        inputHandling.m_windowEventHandler.onMouseEvent(EMouseEventType_Move, inputHandling.m_cursorPosX, inputHandling.m_cursorPosY);
    }

    void InputHandling_Wayland::PointerHandleButton(void *data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
    {
        UNUSED(pointer);
        UNUSED(serial);
        UNUSED(time);

        InputHandling_Wayland& inputHandling = *static_cast<InputHandling_Wayland*>(data);
        const bool buttonPressed = (state == WL_POINTER_BUTTON_STATE_PRESSED);

        switch (button)
        {
        case BTN_LEFT:
        {
            inputHandling.m_leftMouseButtonDown = buttonPressed;
            inputHandling.m_windowEventHandler.onMouseEvent(
                buttonPressed ? EMouseEventType_LeftButtonDown : EMouseEventType_LeftButtonUp,
                inputHandling.m_cursorPosX, inputHandling.m_cursorPosY);
        }
        break;
        case BTN_MIDDLE:
        {
            inputHandling.m_middleMouseButtonDown = buttonPressed;
            inputHandling.m_windowEventHandler.onMouseEvent(
                buttonPressed ? EMouseEventType_MiddleButtonDown : EMouseEventType_MiddleButtonUp,
                inputHandling.m_cursorPosX, inputHandling.m_cursorPosY);

        }
        break;
        case BTN_RIGHT:
        {
            inputHandling.m_rightMouseButtonDown = buttonPressed;
            inputHandling.m_windowEventHandler.onMouseEvent(
                buttonPressed ? EMouseEventType_RightButtonDown : EMouseEventType_RightButtonUp,
                inputHandling.m_cursorPosX, inputHandling.m_cursorPosY);
        }
        break;

        }
    }

    void InputHandling_Wayland::PointerHandleAxis(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
    {
        UNUSED(pointer);
        UNUSED(time);

        InputHandling_Wayland& inputHandling = *static_cast<InputHandling_Wayland*>(data);

        switch (axis)
        {
        case WL_POINTER_AXIS_HORIZONTAL_SCROLL:
            LOG_WARN(CONTEXT_RENDERER, "Detected horizontal scroll event which is not yet supported by RAMSES.");
            break;
        case WL_POINTER_AXIS_VERTICAL_SCROLL:
        {
            const double delta = wl_fixed_to_double(value);
            const EMouseEventType event = (delta < 0) ? EMouseEventType_WheelUp : EMouseEventType_WheelDown;
            inputHandling.m_windowEventHandler.onMouseEvent(event, inputHandling.m_cursorPosX, inputHandling.m_cursorPosY);
        }
        break;
        default:
            LOG_WARN(CONTEXT_RENDERER, "unhandled wayland axis event " << axis);
        }
    }

    void InputHandling_Wayland::KeyboardHandleKeymap(void *data, wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size)
    {
        UNUSED(data);
        UNUSED(keyboard);
        UNUSED(format);
        UNUSED(fd);
        UNUSED(size);
    }

    void InputHandling_Wayland::KeyboardHandleEnter(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys)
    {
        LOG_TRACE(CONTEXT_RENDERER, "keyboard handle enter");
        UNUSED(data);
        UNUSED(keyboard);
        UNUSED(serial);
        UNUSED(surface);
        UNUSED(keys);
    }

    void InputHandling_Wayland::KeyboardHandleLeave(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface)
    {
        LOG_TRACE(CONTEXT_RENDERER, "keyboard handle leave");
        UNUSED(data);
        UNUSED(keyboard);
        UNUSED(serial);
        UNUSED(surface);
    }

    void InputHandling_Wayland::KeyboardHandleKey(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state_w)
    {
        UNUSED(data);
        UNUSED(keyboard);
        UNUSED(serial);
        UNUSED(time);

        const EKeyCode ramsesKeyCode = RamsesKeyCodeFromWaylandKey(key);

        EKeyModifier keyModifier = EKeyModifier_NoModifier;
        switch (ramsesKeyCode)
        {
        case EKeyCode_ShiftLeft:
        case EKeyCode_ShiftRight:
            keyModifier = EKeyModifier_Shift;
            break;
        case EKeyCode_ControlLeft:
        case EKeyCode_ControlRight:
            keyModifier = EKeyModifier_Ctrl;
            break;
        case EKeyCode_AltRight:
            keyModifier = EKeyModifier_Alt;
            break;
        default:
            break;
        }

        InputHandling_Wayland& inputHandling = *static_cast<InputHandling_Wayland*>(data);

        switch (state_w)
        {
        case WL_KEYBOARD_KEY_STATE_PRESSED:
            inputHandling.m_keyModifiers |= keyModifier;
            inputHandling.m_windowEventHandler.onKeyEvent(EKeyEventType_Pressed, inputHandling.m_keyModifiers, ramsesKeyCode);
            break;
        case WL_KEYBOARD_KEY_STATE_RELEASED:
            inputHandling.m_keyModifiers &= ~keyModifier;
            inputHandling.m_windowEventHandler.onKeyEvent(EKeyEventType_Released, inputHandling.m_keyModifiers, ramsesKeyCode);
            break;
        case 2://WL_KEYBOARD_KEY_STATE_REPEAT: // not availabe in current wayland header but send by winston compositor
            inputHandling.m_windowEventHandler.onKeyEvent(EKeyEventType_Pressed, inputHandling.m_keyModifiers, ramsesKeyCode);
            break;
        default:
            LOG_WARN(CONTEXT_RENDERER, "unhandled wayland keyboard handle key event: " << state_w);
        }
    }

    void InputHandling_Wayland::KeyboardHandleModifiers(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
    {
        UNUSED(data);
        UNUSED(keyboard);
        UNUSED(serial);
        UNUSED(mods_depressed);
        UNUSED(mods_latched);
        UNUSED(mods_locked);
        UNUSED(group);
    }

    void InputHandling_Wayland::SeatHandleCapabilities(void *data, wl_seat *seat, unsigned int caps)
    {
        InputHandling_Wayland& inputHandling = *static_cast<InputHandling_Wayland*>(data);

        const Bool hasPointerCapability = (caps & WL_SEAT_CAPABILITY_POINTER) != 0;

        if (hasPointerCapability)
        {
            if (inputHandling.m_pointer == nullptr)
            {
                LOG_INFO(CONTEXT_RENDERER, "InputHandling_Wayland::SeatHandleCapabilities Pointer available");
                inputHandling.m_pointer = wl_seat_get_pointer(seat);
                wl_pointer_add_listener(inputHandling.m_pointer, &inputHandling.m_pointerListener, data);
            }
        }
        else
        {
            if (inputHandling.m_pointer != nullptr)
            {
                LOG_INFO(CONTEXT_RENDERER, "InputHandling_Wayland::SeatHandleCapabilities Pointer not available anymore");
                wl_pointer_destroy(inputHandling.m_pointer);
                inputHandling.m_pointer = nullptr;
            }
        }

        const Bool hasKeyboardCapability = (caps & WL_SEAT_CAPABILITY_KEYBOARD) != 0;

        if (hasKeyboardCapability)
        {
            if (inputHandling.m_keyboard == nullptr)
            {
                LOG_INFO(CONTEXT_RENDERER, "InputHandling_Wayland::SeatHandleCapabilities Keyboard available");
                inputHandling.m_keyboard = wl_seat_get_keyboard(seat);
                wl_keyboard_add_listener(inputHandling.m_keyboard, &inputHandling.m_keyboardListener, data);
            }
        }
        else
        {
            if (inputHandling.m_keyboard != nullptr)
            {
                LOG_INFO(CONTEXT_RENDERER, "InputHandling_Wayland::SeatHandleCapabilities Keyboard not available anymore");
                wl_keyboard_destroy(inputHandling.m_keyboard);
                inputHandling.m_keyboard = nullptr;
            }
        }
    }
}
