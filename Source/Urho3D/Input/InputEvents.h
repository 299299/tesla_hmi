//
// Copyright (c) 2008-2018 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Core/Object.h"

#include <SDL/SDL_joystick.h>
#include <SDL/SDL_gamecontroller.h>
#include <SDL/SDL_keycode.h>
#include <SDL/SDL_mouse.h>

namespace Urho3D
{

/// Mouse button pressed.
URHO3D_EVENT(E_MOUSEBUTTONDOWN, MouseButtonDown)
{
    URHO3D_PARAM(P_BUTTON, Button);                // int
    URHO3D_PARAM(P_BUTTONS, Buttons);              // int
    URHO3D_PARAM(P_QUALIFIERS, Qualifiers);        // int
}

/// Mouse button released.
URHO3D_EVENT(E_MOUSEBUTTONUP, MouseButtonUp)
{
    URHO3D_PARAM(P_BUTTON, Button);                // int
    URHO3D_PARAM(P_BUTTONS, Buttons);              // int
    URHO3D_PARAM(P_QUALIFIERS, Qualifiers);        // int
}

/// Mouse moved.
URHO3D_EVENT(E_MOUSEMOVE, MouseMove)
{
    URHO3D_PARAM(P_X, X);                          // int (only when mouse visible)
    URHO3D_PARAM(P_Y, Y);                          // int (only when mouse visible)
    URHO3D_PARAM(P_DX, DX);                        // int
    URHO3D_PARAM(P_DY, DY);                        // int
    URHO3D_PARAM(P_BUTTONS, Buttons);              // int
    URHO3D_PARAM(P_QUALIFIERS, Qualifiers);        // int
}

/// Mouse wheel moved.
URHO3D_EVENT(E_MOUSEWHEEL, MouseWheel)
{
    URHO3D_PARAM(P_WHEEL, Wheel);                  // int
    URHO3D_PARAM(P_BUTTONS, Buttons);              // int
    URHO3D_PARAM(P_QUALIFIERS, Qualifiers);        // int
}

/// Key pressed.
URHO3D_EVENT(E_KEYDOWN, KeyDown)
{
    URHO3D_PARAM(P_KEY, Key);                      // int
    URHO3D_PARAM(P_SCANCODE, Scancode);            // int
    URHO3D_PARAM(P_BUTTONS, Buttons);              // int
    URHO3D_PARAM(P_QUALIFIERS, Qualifiers);        // int
    URHO3D_PARAM(P_REPEAT, Repeat);                // bool
}

/// Key released.
URHO3D_EVENT(E_KEYUP, KeyUp)
{
    URHO3D_PARAM(P_KEY, Key);                      // int
    URHO3D_PARAM(P_SCANCODE, Scancode);            // int
    URHO3D_PARAM(P_BUTTONS, Buttons);              // int
    URHO3D_PARAM(P_QUALIFIERS, Qualifiers);        // int
}

/// Text input event.
URHO3D_EVENT(E_TEXTINPUT, TextInput)
{
    URHO3D_PARAM(P_TEXT, Text);                    // String
}

/// Text editing event.
URHO3D_EVENT(E_TEXTEDITING, TextEditing)
{
    URHO3D_PARAM(P_COMPOSITION, Composition);      // String
    URHO3D_PARAM(P_CURSOR, Cursor);                // int
    URHO3D_PARAM(P_SELECTION_LENGTH, SelectionLength);  // int
}

/// Joystick connected.
URHO3D_EVENT(E_JOYSTICKCONNECTED, JoystickConnected)
{
    URHO3D_PARAM(P_JOYSTICKID, JoystickID);        // int
}

/// Joystick disconnected.
URHO3D_EVENT(E_JOYSTICKDISCONNECTED, JoystickDisconnected)
{
    URHO3D_PARAM(P_JOYSTICKID, JoystickID);        // int
}

/// Joystick button pressed.
URHO3D_EVENT(E_JOYSTICKBUTTONDOWN, JoystickButtonDown)
{
    URHO3D_PARAM(P_JOYSTICKID, JoystickID);        // int
    URHO3D_PARAM(P_BUTTON, Button);                // int
}

/// Joystick button released.
URHO3D_EVENT(E_JOYSTICKBUTTONUP, JoystickButtonUp)
{
    URHO3D_PARAM(P_JOYSTICKID, JoystickID);        // int
    URHO3D_PARAM(P_BUTTON, Button);                // int
}

/// Joystick axis moved.
URHO3D_EVENT(E_JOYSTICKAXISMOVE, JoystickAxisMove)
{
    URHO3D_PARAM(P_JOYSTICKID, JoystickID);        // int
    URHO3D_PARAM(P_AXIS, Button);                  // int
    URHO3D_PARAM(P_POSITION, Position);            // float
}

/// Joystick POV hat moved.
URHO3D_EVENT(E_JOYSTICKHATMOVE, JoystickHatMove)
{
    URHO3D_PARAM(P_JOYSTICKID, JoystickID);        // int
    URHO3D_PARAM(P_HAT, Button);                   // int
    URHO3D_PARAM(P_POSITION, Position);            // int
}

/// Finger pressed on the screen.
URHO3D_EVENT(E_TOUCHBEGIN, TouchBegin)
{
    URHO3D_PARAM(P_TOUCHID, TouchID);              // int
    URHO3D_PARAM(P_X, X);                          // int
    URHO3D_PARAM(P_Y, Y);                          // int
    URHO3D_PARAM(P_PRESSURE, Pressure);            // float
}

/// Finger released from the screen.
URHO3D_EVENT(E_TOUCHEND, TouchEnd)
{
    URHO3D_PARAM(P_TOUCHID, TouchID);              // int
    URHO3D_PARAM(P_X, X);                          // int
    URHO3D_PARAM(P_Y, Y);                          // int
}

/// Finger moved on the screen.
URHO3D_EVENT(E_TOUCHMOVE, TouchMove)
{
    URHO3D_PARAM(P_TOUCHID, TouchID);              // int
    URHO3D_PARAM(P_X, X);                          // int
    URHO3D_PARAM(P_Y, Y);                          // int
    URHO3D_PARAM(P_DX, DX);                        // int
    URHO3D_PARAM(P_DY, DY);                        // int
    URHO3D_PARAM(P_PRESSURE, Pressure);            // float
}

/// A touch gesture finished recording.
URHO3D_EVENT(E_GESTURERECORDED, GestureRecorded)
{
    URHO3D_PARAM(P_GESTUREID, GestureID);          // unsigned
}

/// A recognized touch gesture was input by the user.
URHO3D_EVENT(E_GESTUREINPUT, GestureInput)
{
    URHO3D_PARAM(P_GESTUREID, GestureID);          // unsigned
    URHO3D_PARAM(P_CENTERX, CenterX);              // int
    URHO3D_PARAM(P_CENTERY, CenterY);              // int
    URHO3D_PARAM(P_NUMFINGERS, NumFingers);        // int
    URHO3D_PARAM(P_ERROR, Error);                  // float
}

/// Pinch/rotate multi-finger touch gesture motion update.
URHO3D_EVENT(E_MULTIGESTURE, MultiGesture)
{
    URHO3D_PARAM(P_CENTERX, CenterX);              // int
    URHO3D_PARAM(P_CENTERY, CenterY);              // int
    URHO3D_PARAM(P_NUMFINGERS, NumFingers);        // int
    URHO3D_PARAM(P_DTHETA, DTheta);                // float (degrees)
    URHO3D_PARAM(P_DDIST, DDist);                  // float
}

/// A file was drag-dropped into the application window.
URHO3D_EVENT(E_DROPFILE, DropFile)
{
    URHO3D_PARAM(P_FILENAME, FileName);            // String
}

/// Application input focus or minimization changed.
URHO3D_EVENT(E_INPUTFOCUS, InputFocus)
{
    URHO3D_PARAM(P_FOCUS, Focus);                  // bool
    URHO3D_PARAM(P_MINIMIZED, Minimized);          // bool
}

/// OS mouse cursor visibility changed.
URHO3D_EVENT(E_MOUSEVISIBLECHANGED, MouseVisibleChanged)
{
    URHO3D_PARAM(P_VISIBLE, Visible);              // bool
}

/// Mouse mode changed.
URHO3D_EVENT(E_MOUSEMODECHANGED, MouseModeChanged)
{
    URHO3D_PARAM(P_MODE, Mode);                    // MouseMode
    URHO3D_PARAM(P_MOUSELOCKED, MouseLocked);      // bool
}

/// Application exit requested.
URHO3D_EVENT(E_EXITREQUESTED, ExitRequested)
{
}

/// Raw SDL input event.
URHO3D_EVENT(E_SDLRAWINPUT, SDLRawInput)
{
    URHO3D_PARAM(P_SDLEVENT, SDLEvent);           // SDL_Event*
    URHO3D_PARAM(P_CONSUMED, Consumed);            // bool
}

/// Input handling begins.
URHO3D_EVENT(E_INPUTBEGIN, InputBegin)
{
}

/// Input handling ends.
URHO3D_EVENT(E_INPUTEND, InputEnd)
{
}

static const int MOUSEB_LEFT = SDL_BUTTON_LMASK;
static const int MOUSEB_MIDDLE = SDL_BUTTON_MMASK;
static const int MOUSEB_RIGHT = SDL_BUTTON_RMASK;
static const int MOUSEB_X1 = SDL_BUTTON_X1MASK;
static const int MOUSEB_X2 = SDL_BUTTON_X2MASK;

static const int QUAL_SHIFT = 1;
static const int QUAL_CTRL = 2;
static const int QUAL_ALT = 4;
static const int QUAL_ANY = 8;

static const int KEY_UNKNOWN = SDLK_UNKNOWN;
static const int KEY_A = SDLK_a;
static const int KEY_B = SDLK_b;
static const int KEY_C = SDLK_c;
static const int KEY_D = SDLK_d;
static const int KEY_E = SDLK_e;
static const int KEY_F = SDLK_f;
static const int KEY_G = SDLK_g;
static const int KEY_H = SDLK_h;
static const int KEY_I = SDLK_i;
static const int KEY_J = SDLK_j;
static const int KEY_K = SDLK_k;
static const int KEY_L = SDLK_l;
static const int KEY_M = SDLK_m;
static const int KEY_N = SDLK_n;
static const int KEY_O = SDLK_o;
static const int KEY_P = SDLK_p;
static const int KEY_Q = SDLK_q;
static const int KEY_R = SDLK_r;
static const int KEY_S = SDLK_s;
static const int KEY_T = SDLK_t;
static const int KEY_U = SDLK_u;
static const int KEY_V = SDLK_v;
static const int KEY_W = SDLK_w;
static const int KEY_X = SDLK_x;
static const int KEY_Y = SDLK_y;
static const int KEY_Z = SDLK_z;
static const int KEY_0 = SDLK_0;
static const int KEY_1 = SDLK_1;
static const int KEY_2 = SDLK_2;
static const int KEY_3 = SDLK_3;
static const int KEY_4 = SDLK_4;
static const int KEY_5 = SDLK_5;
static const int KEY_6 = SDLK_6;
static const int KEY_7 = SDLK_7;
static const int KEY_8 = SDLK_8;
static const int KEY_9 = SDLK_9;
static const int KEY_BACKSPACE = SDLK_BACKSPACE;
static const int KEY_TAB = SDLK_TAB;
static const int KEY_RETURN = SDLK_RETURN;
static const int KEY_RETURN2 = SDLK_RETURN2;
static const int KEY_KP_ENTER = SDLK_KP_ENTER;
static const int KEY_SHIFT = SDLK_LSHIFT;
static const int KEY_CTRL = SDLK_LCTRL;
static const int KEY_ALT = SDLK_LALT;
static const int KEY_GUI = SDLK_LGUI;
static const int KEY_PAUSE = SDLK_PAUSE;
static const int KEY_CAPSLOCK = SDLK_CAPSLOCK;
static const int KEY_ESCAPE = SDLK_ESCAPE;
static const int KEY_SPACE = SDLK_SPACE;
static const int KEY_PAGEUP = SDLK_PAGEUP;
static const int KEY_PAGEDOWN = SDLK_PAGEDOWN;
static const int KEY_END = SDLK_END;
static const int KEY_HOME = SDLK_HOME;
static const int KEY_LEFT = SDLK_LEFT;
static const int KEY_UP = SDLK_UP;
static const int KEY_RIGHT = SDLK_RIGHT;
static const int KEY_DOWN = SDLK_DOWN;
static const int KEY_SELECT = SDLK_SELECT;
static const int KEY_PRINTSCREEN = SDLK_PRINTSCREEN;
static const int KEY_INSERT = SDLK_INSERT;
static const int KEY_DELETE = SDLK_DELETE;
static const int KEY_LGUI = SDLK_LGUI;
static const int KEY_RGUI = SDLK_RGUI;
static const int KEY_APPLICATION = SDLK_APPLICATION;
static const int KEY_KP_0 = SDLK_KP_0;
static const int KEY_KP_1 = SDLK_KP_1;
static const int KEY_KP_2 = SDLK_KP_2;
static const int KEY_KP_3 = SDLK_KP_3;
static const int KEY_KP_4 = SDLK_KP_4;
static const int KEY_KP_5 = SDLK_KP_5;
static const int KEY_KP_6 = SDLK_KP_6;
static const int KEY_KP_7 = SDLK_KP_7;
static const int KEY_KP_8 = SDLK_KP_8;
static const int KEY_KP_9 = SDLK_KP_9;
static const int KEY_KP_MULTIPLY = SDLK_KP_MULTIPLY;
static const int KEY_KP_PLUS = SDLK_KP_PLUS;
static const int KEY_KP_MINUS = SDLK_KP_MINUS;
static const int KEY_KP_PERIOD = SDLK_KP_PERIOD;
static const int KEY_KP_DIVIDE = SDLK_KP_DIVIDE;
static const int KEY_F1 = SDLK_F1;
static const int KEY_F2 = SDLK_F2;
static const int KEY_F3 = SDLK_F3;
static const int KEY_F4 = SDLK_F4;
static const int KEY_F5 = SDLK_F5;
static const int KEY_F6 = SDLK_F6;
static const int KEY_F7 = SDLK_F7;
static const int KEY_F8 = SDLK_F8;
static const int KEY_F9 = SDLK_F9;
static const int KEY_F10 = SDLK_F10;
static const int KEY_F11 = SDLK_F11;
static const int KEY_F12 = SDLK_F12;
static const int KEY_F13 = SDLK_F13;
static const int KEY_F14 = SDLK_F14;
static const int KEY_F15 = SDLK_F15;
static const int KEY_F16 = SDLK_F16;
static const int KEY_F17 = SDLK_F17;
static const int KEY_F18 = SDLK_F18;
static const int KEY_F19 = SDLK_F19;
static const int KEY_F20 = SDLK_F20;
static const int KEY_F21 = SDLK_F21;
static const int KEY_F22 = SDLK_F22;
static const int KEY_F23 = SDLK_F23;
static const int KEY_F24 = SDLK_F24;
static const int KEY_NUMLOCKCLEAR = SDLK_NUMLOCKCLEAR;
static const int KEY_SCROLLLOCK = SDLK_SCROLLLOCK;
static const int KEY_LSHIFT = SDLK_LSHIFT;
static const int KEY_RSHIFT = SDLK_RSHIFT;
static const int KEY_LCTRL = SDLK_LCTRL;
static const int KEY_RCTRL = SDLK_RCTRL;
static const int KEY_LALT = SDLK_LALT;
static const int KEY_RALT = SDLK_RALT;
static const int KEY_AC_BACK = SDLK_AC_BACK;
static const int KEY_AC_BOOKMARKS = SDLK_AC_BOOKMARKS;
static const int KEY_AC_FORWARD = SDLK_AC_FORWARD;
static const int KEY_AC_HOME = SDLK_AC_HOME;
static const int KEY_AC_REFRESH = SDLK_AC_REFRESH;
static const int KEY_AC_SEARCH = SDLK_AC_SEARCH;
static const int KEY_AC_STOP = SDLK_AC_STOP;
static const int KEY_AGAIN = SDLK_AGAIN;
static const int KEY_ALTERASE = SDLK_ALTERASE;
static const int KEY_AMPERSAND = SDLK_AMPERSAND;
static const int KEY_ASTERISK = SDLK_ASTERISK;
static const int KEY_AT = SDLK_AT;
static const int KEY_AUDIOMUTE = SDLK_AUDIOMUTE;
static const int KEY_AUDIONEXT = SDLK_AUDIONEXT;
static const int KEY_AUDIOPLAY = SDLK_AUDIOPLAY;
static const int KEY_AUDIOPREV = SDLK_AUDIOPREV;
static const int KEY_AUDIOSTOP = SDLK_AUDIOSTOP;
static const int KEY_BACKQUOTE = SDLK_BACKQUOTE;
static const int KEY_BACKSLASH = SDLK_BACKSLASH;
static const int KEY_BRIGHTNESSDOWN = SDLK_BRIGHTNESSDOWN;
static const int KEY_BRIGHTNESSUP = SDLK_BRIGHTNESSUP;
static const int KEY_CALCULATOR = SDLK_CALCULATOR;
static const int KEY_CANCEL = SDLK_CANCEL;
static const int KEY_CARET = SDLK_CARET;
static const int KEY_CLEAR = SDLK_CLEAR;
static const int KEY_CLEARAGAIN = SDLK_CLEARAGAIN;
static const int KEY_COLON = SDLK_COLON;
static const int KEY_COMMA = SDLK_COMMA;
static const int KEY_COMPUTER = SDLK_COMPUTER;
static const int KEY_COPY = SDLK_COPY;
static const int KEY_CRSEL = SDLK_CRSEL;
static const int KEY_CURRENCYSUBUNIT = SDLK_CURRENCYSUBUNIT;
static const int KEY_CURRENCYUNIT = SDLK_CURRENCYUNIT;
static const int KEY_CUT = SDLK_CUT;
static const int KEY_DECIMALSEPARATOR = SDLK_DECIMALSEPARATOR;
static const int KEY_DISPLAYSWITCH = SDLK_DISPLAYSWITCH;
static const int KEY_DOLLAR = SDLK_DOLLAR;
static const int KEY_EJECT = SDLK_EJECT;
static const int KEY_EQUALS = SDLK_EQUALS;
static const int KEY_EXCLAIM = SDLK_EXCLAIM;
static const int KEY_EXSEL = SDLK_EXSEL;
static const int KEY_FIND = SDLK_FIND;
static const int KEY_GREATER = SDLK_GREATER;
static const int KEY_HASH = SDLK_HASH;
static const int KEY_HELP = SDLK_HELP;
static const int KEY_KBDILLUMDOWN = SDLK_KBDILLUMDOWN;
static const int KEY_KBDILLUMTOGGLE = SDLK_KBDILLUMTOGGLE;
static const int KEY_KBDILLUMUP = SDLK_KBDILLUMUP;
static const int KEY_KP_00 = SDLK_KP_00;
static const int KEY_KP_000 = SDLK_KP_000;
static const int KEY_KP_A = SDLK_KP_A;
static const int KEY_KP_AMPERSAND = SDLK_KP_AMPERSAND;
static const int KEY_KP_AT = SDLK_KP_AT;
static const int KEY_KP_B = SDLK_KP_B;
static const int KEY_KP_BACKSPACE = SDLK_KP_BACKSPACE;
static const int KEY_KP_BINARY = SDLK_KP_BINARY;
static const int KEY_KP_C = SDLK_KP_C;
static const int KEY_KP_CLEAR = SDLK_KP_CLEAR;
static const int KEY_KP_CLEARENTRY = SDLK_KP_CLEARENTRY;
static const int KEY_KP_COLON = SDLK_KP_COLON;
static const int KEY_KP_COMMA = SDLK_KP_COMMA;
static const int KEY_KP_D = SDLK_KP_D;
static const int KEY_KP_DBLAMPERSAND = SDLK_KP_DBLAMPERSAND;
static const int KEY_KP_DBLVERTICALBAR = SDLK_KP_DBLVERTICALBAR;
static const int KEY_KP_DECIMAL = SDLK_KP_DECIMAL;
static const int KEY_KP_E = SDLK_KP_E;
static const int KEY_KP_EQUALS = SDLK_KP_EQUALS;
static const int KEY_KP_EQUALSAS400 = SDLK_KP_EQUALSAS400;
static const int KEY_KP_EXCLAM = SDLK_KP_EXCLAM;
static const int KEY_KP_F = SDLK_KP_F;
static const int KEY_KP_GREATER = SDLK_KP_GREATER;
static const int KEY_KP_HASH = SDLK_KP_HASH;
static const int KEY_KP_HEXADECIMAL = SDLK_KP_HEXADECIMAL;
static const int KEY_KP_LEFTBRACE = SDLK_KP_LEFTBRACE;
static const int KEY_KP_LEFTPAREN = SDLK_KP_LEFTPAREN;
static const int KEY_KP_LESS = SDLK_KP_LESS;
static const int KEY_KP_MEMADD = SDLK_KP_MEMADD;
static const int KEY_KP_MEMCLEAR = SDLK_KP_MEMCLEAR;
static const int KEY_KP_MEMDIVIDE = SDLK_KP_MEMDIVIDE;
static const int KEY_KP_MEMMULTIPLY = SDLK_KP_MEMMULTIPLY;
static const int KEY_KP_MEMRECALL = SDLK_KP_MEMRECALL;
static const int KEY_KP_MEMSTORE = SDLK_KP_MEMSTORE;
static const int KEY_KP_MEMSUBTRACT = SDLK_KP_MEMSUBTRACT;
static const int KEY_KP_OCTAL = SDLK_KP_OCTAL;
static const int KEY_KP_PERCENT = SDLK_KP_PERCENT;
static const int KEY_KP_PLUSMINUS = SDLK_KP_PLUSMINUS;
static const int KEY_KP_POWER = SDLK_KP_POWER;
static const int KEY_KP_RIGHTBRACE = SDLK_KP_RIGHTBRACE;
static const int KEY_KP_RIGHTPAREN = SDLK_KP_RIGHTPAREN;
static const int KEY_KP_SPACE = SDLK_KP_SPACE;
static const int KEY_KP_TAB = SDLK_KP_TAB;
static const int KEY_KP_VERTICALBAR = SDLK_KP_VERTICALBAR;
static const int KEY_KP_XOR = SDLK_KP_XOR;
static const int KEY_LEFTBRACKET = SDLK_LEFTBRACKET;
static const int KEY_LEFTPAREN = SDLK_LEFTPAREN;
static const int KEY_LESS = SDLK_LESS;
static const int KEY_MAIL = SDLK_MAIL;
static const int KEY_MEDIASELECT = SDLK_MEDIASELECT;
static const int KEY_MENU = SDLK_MENU;
static const int KEY_MINUS = SDLK_MINUS;
static const int KEY_MODE = SDLK_MODE;
static const int KEY_MUTE = SDLK_MUTE;
static const int KEY_OPER = SDLK_OPER;
static const int KEY_OUT = SDLK_OUT;
static const int KEY_PASTE = SDLK_PASTE;
static const int KEY_PERCENT = SDLK_PERCENT;
static const int KEY_PERIOD = SDLK_PERIOD;
static const int KEY_PLUS = SDLK_PLUS;
static const int KEY_POWER = SDLK_POWER;
static const int KEY_PRIOR = SDLK_PRIOR;
static const int KEY_QUESTION = SDLK_QUESTION;
static const int KEY_QUOTE = SDLK_QUOTE;
static const int KEY_QUOTEDBL = SDLK_QUOTEDBL;
static const int KEY_RIGHTBRACKET = SDLK_RIGHTBRACKET;
static const int KEY_RIGHTPAREN = SDLK_RIGHTPAREN;
static const int KEY_SEMICOLON = SDLK_SEMICOLON;
static const int KEY_SEPARATOR = SDLK_SEPARATOR;
static const int KEY_SLASH = SDLK_SLASH;
static const int KEY_SLEEP = SDLK_SLEEP;
static const int KEY_STOP = SDLK_STOP;
static const int KEY_SYSREQ = SDLK_SYSREQ;
static const int KEY_THOUSANDSSEPARATOR = SDLK_THOUSANDSSEPARATOR;
static const int KEY_UNDERSCORE = SDLK_UNDERSCORE;
static const int KEY_UNDO = SDLK_UNDO;
static const int KEY_VOLUMEDOWN = SDLK_VOLUMEDOWN;
static const int KEY_VOLUMEUP = SDLK_VOLUMEUP;
static const int KEY_WWW = SDLK_WWW;

static const int SCANCODE_UNKNOWN = SDL_SCANCODE_UNKNOWN;
static const int SCANCODE_CTRL = SDL_SCANCODE_LCTRL;
static const int SCANCODE_SHIFT = SDL_SCANCODE_LSHIFT;
static const int SCANCODE_ALT = SDL_SCANCODE_LALT;
static const int SCANCODE_GUI = SDL_SCANCODE_LGUI;
static const int SCANCODE_A = SDL_SCANCODE_A;
static const int SCANCODE_B = SDL_SCANCODE_B;
static const int SCANCODE_C = SDL_SCANCODE_C;
static const int SCANCODE_D = SDL_SCANCODE_D;
static const int SCANCODE_E = SDL_SCANCODE_E;
static const int SCANCODE_F = SDL_SCANCODE_F;
static const int SCANCODE_G = SDL_SCANCODE_G;
static const int SCANCODE_H = SDL_SCANCODE_H;
static const int SCANCODE_I = SDL_SCANCODE_I;
static const int SCANCODE_J = SDL_SCANCODE_J;
static const int SCANCODE_K = SDL_SCANCODE_K;
static const int SCANCODE_L = SDL_SCANCODE_L;
static const int SCANCODE_M = SDL_SCANCODE_M;
static const int SCANCODE_N = SDL_SCANCODE_N;
static const int SCANCODE_O = SDL_SCANCODE_O;
static const int SCANCODE_P = SDL_SCANCODE_P;
static const int SCANCODE_Q = SDL_SCANCODE_Q;
static const int SCANCODE_R = SDL_SCANCODE_R;
static const int SCANCODE_S = SDL_SCANCODE_S;
static const int SCANCODE_T = SDL_SCANCODE_T;
static const int SCANCODE_U = SDL_SCANCODE_U;
static const int SCANCODE_V = SDL_SCANCODE_V;
static const int SCANCODE_W = SDL_SCANCODE_W;
static const int SCANCODE_X = SDL_SCANCODE_X;
static const int SCANCODE_Y = SDL_SCANCODE_Y;
static const int SCANCODE_Z = SDL_SCANCODE_Z;
static const int SCANCODE_1 = SDL_SCANCODE_1;
static const int SCANCODE_2 = SDL_SCANCODE_2;
static const int SCANCODE_3 = SDL_SCANCODE_3;
static const int SCANCODE_4 = SDL_SCANCODE_4;
static const int SCANCODE_5 = SDL_SCANCODE_5;
static const int SCANCODE_6 = SDL_SCANCODE_6;
static const int SCANCODE_7 = SDL_SCANCODE_7;
static const int SCANCODE_8 = SDL_SCANCODE_8;
static const int SCANCODE_9 = SDL_SCANCODE_9;
static const int SCANCODE_0 = SDL_SCANCODE_0;
static const int SCANCODE_RETURN = SDL_SCANCODE_RETURN;
static const int SCANCODE_ESCAPE = SDL_SCANCODE_ESCAPE;
static const int SCANCODE_BACKSPACE = SDL_SCANCODE_BACKSPACE;
static const int SCANCODE_TAB = SDL_SCANCODE_TAB;
static const int SCANCODE_SPACE = SDL_SCANCODE_SPACE;
static const int SCANCODE_MINUS = SDL_SCANCODE_MINUS;
static const int SCANCODE_EQUALS = SDL_SCANCODE_EQUALS;
static const int SCANCODE_LEFTBRACKET = SDL_SCANCODE_LEFTBRACKET;
static const int SCANCODE_RIGHTBRACKET = SDL_SCANCODE_RIGHTBRACKET;
static const int SCANCODE_BACKSLASH = SDL_SCANCODE_BACKSLASH;
static const int SCANCODE_NONUSHASH = SDL_SCANCODE_NONUSHASH;
static const int SCANCODE_SEMICOLON = SDL_SCANCODE_SEMICOLON;
static const int SCANCODE_APOSTROPHE = SDL_SCANCODE_APOSTROPHE;
static const int SCANCODE_GRAVE = SDL_SCANCODE_GRAVE;
static const int SCANCODE_COMMA = SDL_SCANCODE_COMMA;
static const int SCANCODE_PERIOD = SDL_SCANCODE_PERIOD;
static const int SCANCODE_SLASH = SDL_SCANCODE_SLASH;
static const int SCANCODE_CAPSLOCK = SDL_SCANCODE_CAPSLOCK;
static const int SCANCODE_F1 = SDL_SCANCODE_F1;
static const int SCANCODE_F2 = SDL_SCANCODE_F2;
static const int SCANCODE_F3 = SDL_SCANCODE_F3;
static const int SCANCODE_F4 = SDL_SCANCODE_F4;
static const int SCANCODE_F5 = SDL_SCANCODE_F5;
static const int SCANCODE_F6 = SDL_SCANCODE_F6;
static const int SCANCODE_F7 = SDL_SCANCODE_F7;
static const int SCANCODE_F8 = SDL_SCANCODE_F8;
static const int SCANCODE_F9 = SDL_SCANCODE_F9;
static const int SCANCODE_F10 = SDL_SCANCODE_F10;
static const int SCANCODE_F11 = SDL_SCANCODE_F11;
static const int SCANCODE_F12 = SDL_SCANCODE_F12;
static const int SCANCODE_PRINTSCREEN = SDL_SCANCODE_PRINTSCREEN;
static const int SCANCODE_SCROLLLOCK = SDL_SCANCODE_SCROLLLOCK;
static const int SCANCODE_PAUSE = SDL_SCANCODE_PAUSE;
static const int SCANCODE_INSERT = SDL_SCANCODE_INSERT;
static const int SCANCODE_HOME = SDL_SCANCODE_HOME;
static const int SCANCODE_PAGEUP = SDL_SCANCODE_PAGEUP;
static const int SCANCODE_DELETE = SDL_SCANCODE_DELETE;
static const int SCANCODE_END = SDL_SCANCODE_END;
static const int SCANCODE_PAGEDOWN = SDL_SCANCODE_PAGEDOWN;
static const int SCANCODE_RIGHT = SDL_SCANCODE_RIGHT;
static const int SCANCODE_LEFT = SDL_SCANCODE_LEFT;
static const int SCANCODE_DOWN = SDL_SCANCODE_DOWN;
static const int SCANCODE_UP = SDL_SCANCODE_UP;
static const int SCANCODE_NUMLOCKCLEAR = SDL_SCANCODE_NUMLOCKCLEAR;
static const int SCANCODE_KP_DIVIDE = SDL_SCANCODE_KP_DIVIDE;
static const int SCANCODE_KP_MULTIPLY = SDL_SCANCODE_KP_MULTIPLY;
static const int SCANCODE_KP_MINUS = SDL_SCANCODE_KP_MINUS;
static const int SCANCODE_KP_PLUS = SDL_SCANCODE_KP_PLUS;
static const int SCANCODE_KP_ENTER = SDL_SCANCODE_KP_ENTER;
static const int SCANCODE_KP_1 = SDL_SCANCODE_KP_1;
static const int SCANCODE_KP_2 = SDL_SCANCODE_KP_2;
static const int SCANCODE_KP_3 = SDL_SCANCODE_KP_3;
static const int SCANCODE_KP_4 = SDL_SCANCODE_KP_4;
static const int SCANCODE_KP_5 = SDL_SCANCODE_KP_5;
static const int SCANCODE_KP_6 = SDL_SCANCODE_KP_6;
static const int SCANCODE_KP_7 = SDL_SCANCODE_KP_7;
static const int SCANCODE_KP_8 = SDL_SCANCODE_KP_8;
static const int SCANCODE_KP_9 = SDL_SCANCODE_KP_9;
static const int SCANCODE_KP_0 = SDL_SCANCODE_KP_0;
static const int SCANCODE_KP_PERIOD = SDL_SCANCODE_KP_PERIOD;
static const int SCANCODE_NONUSBACKSLASH = SDL_SCANCODE_NONUSBACKSLASH;
static const int SCANCODE_APPLICATION = SDL_SCANCODE_APPLICATION;
static const int SCANCODE_POWER = SDL_SCANCODE_POWER;
static const int SCANCODE_KP_EQUALS = SDL_SCANCODE_KP_EQUALS;
static const int SCANCODE_F13 = SDL_SCANCODE_F13;
static const int SCANCODE_F14 = SDL_SCANCODE_F14;
static const int SCANCODE_F15 = SDL_SCANCODE_F15;
static const int SCANCODE_F16 = SDL_SCANCODE_F16;
static const int SCANCODE_F17 = SDL_SCANCODE_F17;
static const int SCANCODE_F18 = SDL_SCANCODE_F18;
static const int SCANCODE_F19 = SDL_SCANCODE_F19;
static const int SCANCODE_F20 = SDL_SCANCODE_F20;
static const int SCANCODE_F21 = SDL_SCANCODE_F21;
static const int SCANCODE_F22 = SDL_SCANCODE_F22;
static const int SCANCODE_F23 = SDL_SCANCODE_F23;
static const int SCANCODE_F24 = SDL_SCANCODE_F24;
static const int SCANCODE_EXECUTE = SDL_SCANCODE_EXECUTE;
static const int SCANCODE_HELP = SDL_SCANCODE_HELP;
static const int SCANCODE_MENU = SDL_SCANCODE_MENU;
static const int SCANCODE_SELECT = SDL_SCANCODE_SELECT;
static const int SCANCODE_STOP = SDL_SCANCODE_STOP;
static const int SCANCODE_AGAIN = SDL_SCANCODE_AGAIN;
static const int SCANCODE_UNDO = SDL_SCANCODE_UNDO;
static const int SCANCODE_CUT = SDL_SCANCODE_CUT;
static const int SCANCODE_COPY = SDL_SCANCODE_COPY;
static const int SCANCODE_PASTE = SDL_SCANCODE_PASTE;
static const int SCANCODE_FIND = SDL_SCANCODE_FIND;
static const int SCANCODE_MUTE = SDL_SCANCODE_MUTE;
static const int SCANCODE_VOLUMEUP = SDL_SCANCODE_VOLUMEUP;
static const int SCANCODE_VOLUMEDOWN = SDL_SCANCODE_VOLUMEDOWN;
static const int SCANCODE_KP_COMMA = SDL_SCANCODE_KP_COMMA;
static const int SCANCODE_KP_EQUALSAS400 = SDL_SCANCODE_KP_EQUALSAS400;
static const int SCANCODE_INTERNATIONAL1 = SDL_SCANCODE_INTERNATIONAL1;
static const int SCANCODE_INTERNATIONAL2 = SDL_SCANCODE_INTERNATIONAL2;
static const int SCANCODE_INTERNATIONAL3 = SDL_SCANCODE_INTERNATIONAL3;
static const int SCANCODE_INTERNATIONAL4 = SDL_SCANCODE_INTERNATIONAL4;
static const int SCANCODE_INTERNATIONAL5 = SDL_SCANCODE_INTERNATIONAL5;
static const int SCANCODE_INTERNATIONAL6 = SDL_SCANCODE_INTERNATIONAL6;
static const int SCANCODE_INTERNATIONAL7 = SDL_SCANCODE_INTERNATIONAL7;
static const int SCANCODE_INTERNATIONAL8 = SDL_SCANCODE_INTERNATIONAL8;
static const int SCANCODE_INTERNATIONAL9 = SDL_SCANCODE_INTERNATIONAL9;
static const int SCANCODE_LANG1 = SDL_SCANCODE_LANG1;
static const int SCANCODE_LANG2 = SDL_SCANCODE_LANG2;
static const int SCANCODE_LANG3 = SDL_SCANCODE_LANG3;
static const int SCANCODE_LANG4 = SDL_SCANCODE_LANG4;
static const int SCANCODE_LANG5 = SDL_SCANCODE_LANG5;
static const int SCANCODE_LANG6 = SDL_SCANCODE_LANG6;
static const int SCANCODE_LANG7 = SDL_SCANCODE_LANG7;
static const int SCANCODE_LANG8 = SDL_SCANCODE_LANG8;
static const int SCANCODE_LANG9 = SDL_SCANCODE_LANG9;
static const int SCANCODE_ALTERASE = SDL_SCANCODE_ALTERASE;
static const int SCANCODE_SYSREQ = SDL_SCANCODE_SYSREQ;
static const int SCANCODE_CANCEL = SDL_SCANCODE_CANCEL;
static const int SCANCODE_CLEAR = SDL_SCANCODE_CLEAR;
static const int SCANCODE_PRIOR = SDL_SCANCODE_PRIOR;
static const int SCANCODE_RETURN2 = SDL_SCANCODE_RETURN2;
static const int SCANCODE_SEPARATOR = SDL_SCANCODE_SEPARATOR;
static const int SCANCODE_OUT = SDL_SCANCODE_OUT;
static const int SCANCODE_OPER = SDL_SCANCODE_OPER;
static const int SCANCODE_CLEARAGAIN = SDL_SCANCODE_CLEARAGAIN;
static const int SCANCODE_CRSEL = SDL_SCANCODE_CRSEL;
static const int SCANCODE_EXSEL = SDL_SCANCODE_EXSEL;
static const int SCANCODE_KP_00 = SDL_SCANCODE_KP_00;
static const int SCANCODE_KP_000 = SDL_SCANCODE_KP_000;
static const int SCANCODE_THOUSANDSSEPARATOR = SDL_SCANCODE_THOUSANDSSEPARATOR;
static const int SCANCODE_DECIMALSEPARATOR = SDL_SCANCODE_DECIMALSEPARATOR;
static const int SCANCODE_CURRENCYUNIT = SDL_SCANCODE_CURRENCYUNIT;
static const int SCANCODE_CURRENCYSUBUNIT = SDL_SCANCODE_CURRENCYSUBUNIT;
static const int SCANCODE_KP_LEFTPAREN = SDL_SCANCODE_KP_LEFTPAREN;
static const int SCANCODE_KP_RIGHTPAREN = SDL_SCANCODE_KP_RIGHTPAREN;
static const int SCANCODE_KP_LEFTBRACE = SDL_SCANCODE_KP_LEFTBRACE;
static const int SCANCODE_KP_RIGHTBRACE = SDL_SCANCODE_KP_RIGHTBRACE;
static const int SCANCODE_KP_TAB = SDL_SCANCODE_KP_TAB;
static const int SCANCODE_KP_BACKSPACE = SDL_SCANCODE_KP_BACKSPACE;
static const int SCANCODE_KP_A = SDL_SCANCODE_KP_A;
static const int SCANCODE_KP_B = SDL_SCANCODE_KP_B;
static const int SCANCODE_KP_C = SDL_SCANCODE_KP_C;
static const int SCANCODE_KP_D = SDL_SCANCODE_KP_D;
static const int SCANCODE_KP_E = SDL_SCANCODE_KP_E;
static const int SCANCODE_KP_F = SDL_SCANCODE_KP_F;
static const int SCANCODE_KP_XOR = SDL_SCANCODE_KP_XOR;
static const int SCANCODE_KP_POWER = SDL_SCANCODE_KP_POWER;
static const int SCANCODE_KP_PERCENT = SDL_SCANCODE_KP_PERCENT;
static const int SCANCODE_KP_LESS = SDL_SCANCODE_KP_LESS;
static const int SCANCODE_KP_GREATER = SDL_SCANCODE_KP_GREATER;
static const int SCANCODE_KP_AMPERSAND = SDL_SCANCODE_KP_AMPERSAND;
static const int SCANCODE_KP_DBLAMPERSAND = SDL_SCANCODE_KP_DBLAMPERSAND;
static const int SCANCODE_KP_VERTICALBAR = SDL_SCANCODE_KP_VERTICALBAR;
static const int SCANCODE_KP_DBLVERTICALBAR = SDL_SCANCODE_KP_DBLVERTICALBAR;
static const int SCANCODE_KP_COLON = SDL_SCANCODE_KP_COLON;
static const int SCANCODE_KP_HASH = SDL_SCANCODE_KP_HASH;
static const int SCANCODE_KP_SPACE = SDL_SCANCODE_KP_SPACE;
static const int SCANCODE_KP_AT = SDL_SCANCODE_KP_AT;
static const int SCANCODE_KP_EXCLAM = SDL_SCANCODE_KP_EXCLAM;
static const int SCANCODE_KP_MEMSTORE = SDL_SCANCODE_KP_MEMSTORE;
static const int SCANCODE_KP_MEMRECALL = SDL_SCANCODE_KP_MEMRECALL;
static const int SCANCODE_KP_MEMCLEAR = SDL_SCANCODE_KP_MEMCLEAR;
static const int SCANCODE_KP_MEMADD = SDL_SCANCODE_KP_MEMADD;
static const int SCANCODE_KP_MEMSUBTRACT = SDL_SCANCODE_KP_MEMSUBTRACT;
static const int SCANCODE_KP_MEMMULTIPLY = SDL_SCANCODE_KP_MEMMULTIPLY;
static const int SCANCODE_KP_MEMDIVIDE = SDL_SCANCODE_KP_MEMDIVIDE;
static const int SCANCODE_KP_PLUSMINUS = SDL_SCANCODE_KP_PLUSMINUS;
static const int SCANCODE_KP_CLEAR = SDL_SCANCODE_KP_CLEAR;
static const int SCANCODE_KP_CLEARENTRY = SDL_SCANCODE_KP_CLEARENTRY;
static const int SCANCODE_KP_BINARY = SDL_SCANCODE_KP_BINARY;
static const int SCANCODE_KP_OCTAL = SDL_SCANCODE_KP_OCTAL;
static const int SCANCODE_KP_DECIMAL = SDL_SCANCODE_KP_DECIMAL;
static const int SCANCODE_KP_HEXADECIMAL = SDL_SCANCODE_KP_HEXADECIMAL;
static const int SCANCODE_LCTRL = SDL_SCANCODE_LCTRL;
static const int SCANCODE_LSHIFT = SDL_SCANCODE_LSHIFT;
static const int SCANCODE_LALT = SDL_SCANCODE_LALT;
static const int SCANCODE_LGUI = SDL_SCANCODE_LGUI;
static const int SCANCODE_RCTRL = SDL_SCANCODE_RCTRL;
static const int SCANCODE_RSHIFT = SDL_SCANCODE_RSHIFT;
static const int SCANCODE_RALT = SDL_SCANCODE_RALT;
static const int SCANCODE_RGUI = SDL_SCANCODE_RGUI;
static const int SCANCODE_MODE = SDL_SCANCODE_MODE;
static const int SCANCODE_AUDIONEXT = SDL_SCANCODE_AUDIONEXT;
static const int SCANCODE_AUDIOPREV = SDL_SCANCODE_AUDIOPREV;
static const int SCANCODE_AUDIOSTOP = SDL_SCANCODE_AUDIOSTOP;
static const int SCANCODE_AUDIOPLAY = SDL_SCANCODE_AUDIOPLAY;
static const int SCANCODE_AUDIOMUTE = SDL_SCANCODE_AUDIOMUTE;
static const int SCANCODE_MEDIASELECT = SDL_SCANCODE_MEDIASELECT;
static const int SCANCODE_WWW = SDL_SCANCODE_WWW;
static const int SCANCODE_MAIL = SDL_SCANCODE_MAIL;
static const int SCANCODE_CALCULATOR = SDL_SCANCODE_CALCULATOR;
static const int SCANCODE_COMPUTER = SDL_SCANCODE_COMPUTER;
static const int SCANCODE_AC_SEARCH = SDL_SCANCODE_AC_SEARCH;
static const int SCANCODE_AC_HOME = SDL_SCANCODE_AC_HOME;
static const int SCANCODE_AC_BACK = SDL_SCANCODE_AC_BACK;
static const int SCANCODE_AC_FORWARD = SDL_SCANCODE_AC_FORWARD;
static const int SCANCODE_AC_STOP = SDL_SCANCODE_AC_STOP;
static const int SCANCODE_AC_REFRESH = SDL_SCANCODE_AC_REFRESH;
static const int SCANCODE_AC_BOOKMARKS = SDL_SCANCODE_AC_BOOKMARKS;
static const int SCANCODE_BRIGHTNESSDOWN = SDL_SCANCODE_BRIGHTNESSDOWN;
static const int SCANCODE_BRIGHTNESSUP = SDL_SCANCODE_BRIGHTNESSUP;
static const int SCANCODE_DISPLAYSWITCH = SDL_SCANCODE_DISPLAYSWITCH;
static const int SCANCODE_KBDILLUMTOGGLE = SDL_SCANCODE_KBDILLUMTOGGLE;
static const int SCANCODE_KBDILLUMDOWN = SDL_SCANCODE_KBDILLUMDOWN;
static const int SCANCODE_KBDILLUMUP = SDL_SCANCODE_KBDILLUMUP;
static const int SCANCODE_EJECT = SDL_SCANCODE_EJECT;
static const int SCANCODE_SLEEP = SDL_SCANCODE_SLEEP;
static const int SCANCODE_APP1 = SDL_SCANCODE_APP1;
static const int SCANCODE_APP2 = SDL_SCANCODE_APP2;

static const int HAT_CENTER = SDL_HAT_CENTERED;
static const int HAT_UP = SDL_HAT_UP;
static const int HAT_RIGHT = SDL_HAT_RIGHT;
static const int HAT_DOWN = SDL_HAT_DOWN;
static const int HAT_LEFT = SDL_HAT_LEFT;

static const int CONTROLLER_BUTTON_A = SDL_CONTROLLER_BUTTON_A;
static const int CONTROLLER_BUTTON_B = SDL_CONTROLLER_BUTTON_B;
static const int CONTROLLER_BUTTON_X = SDL_CONTROLLER_BUTTON_X;
static const int CONTROLLER_BUTTON_Y = SDL_CONTROLLER_BUTTON_Y;
static const int CONTROLLER_BUTTON_BACK = SDL_CONTROLLER_BUTTON_BACK;
static const int CONTROLLER_BUTTON_GUIDE = SDL_CONTROLLER_BUTTON_GUIDE;
static const int CONTROLLER_BUTTON_START = SDL_CONTROLLER_BUTTON_START;
static const int CONTROLLER_BUTTON_LEFTSTICK = SDL_CONTROLLER_BUTTON_LEFTSTICK;
static const int CONTROLLER_BUTTON_RIGHTSTICK = SDL_CONTROLLER_BUTTON_RIGHTSTICK;
static const int CONTROLLER_BUTTON_LEFTSHOULDER = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
static const int CONTROLLER_BUTTON_RIGHTSHOULDER = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
static const int CONTROLLER_BUTTON_DPAD_UP = SDL_CONTROLLER_BUTTON_DPAD_UP;
static const int CONTROLLER_BUTTON_DPAD_DOWN = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
static const int CONTROLLER_BUTTON_DPAD_LEFT = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
static const int CONTROLLER_BUTTON_DPAD_RIGHT = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

static const int CONTROLLER_AXIS_LEFTX = SDL_CONTROLLER_AXIS_LEFTX;
static const int CONTROLLER_AXIS_LEFTY = SDL_CONTROLLER_AXIS_LEFTY;
static const int CONTROLLER_AXIS_RIGHTX = SDL_CONTROLLER_AXIS_RIGHTX;
static const int CONTROLLER_AXIS_RIGHTY = SDL_CONTROLLER_AXIS_RIGHTY;
static const int CONTROLLER_AXIS_TRIGGERLEFT = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
static const int CONTROLLER_AXIS_TRIGGERRIGHT = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;

}
