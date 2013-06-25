/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef ROCKETCOREINPUT_H
#define ROCKETCOREINPUT_H

namespace Rocket {
namespace Core {

/*
	Enumerants for sending input events into Rocket.

	@author Peter Curry
 */

namespace Input
{
	enum KeyIdentifier
	{
		KI_UNKNOWN = 0,

		KI_SPACE = 1,

		KI_0 = 2,
		KI_1 = 3,
		KI_2 = 4,
		KI_3 = 5,
		KI_4 = 6,
		KI_5 = 7,
		KI_6 = 8,
		KI_7 = 9,
		KI_8 = 10,
		KI_9 = 11,

		KI_A = 12,
		KI_B = 13,
		KI_C = 14,
		KI_D = 15,
		KI_E = 16,
		KI_F = 17,
		KI_G = 18,
		KI_H = 19,
		KI_I = 20,
		KI_J = 21,
		KI_K = 22,
		KI_L = 23,
		KI_M = 24,
		KI_N = 25,
		KI_O = 26,
		KI_P = 27,
		KI_Q = 28,
		KI_R = 29,
		KI_S = 30,
		KI_T = 31,
		KI_U = 32,
		KI_V = 33,
		KI_W = 34,
		KI_X = 35,
		KI_Y = 36,
		KI_Z = 37,

		KI_OEM_1 = 38,				// US standard keyboard; the ';:' key.
		KI_OEM_PLUS = 39,			// Any region; the '=+' key.
		KI_OEM_COMMA = 40,			// Any region; the ',<' key.
		KI_OEM_MINUS = 41,			// Any region; the '-_' key.
		KI_OEM_PERIOD = 42,			// Any region; the '.>' key.
		KI_OEM_2 = 43,				// Any region; the '/?' key.
		KI_OEM_3 = 44,				// Any region; the '`~' key.

		KI_OEM_4 = 45,				// US standard keyboard; the '[{' key.
		KI_OEM_5 = 46,				// US standard keyboard; the '\|' key.
		KI_OEM_6 = 47,				// US standard keyboard; the ']}' key.
		KI_OEM_7 = 48,				// US standard keyboard; the ''"' key.
		KI_OEM_8 = 49,

		KI_OEM_102 = 50,			// RT 102-key keyboard; the '<>' or '\|' key.

		KI_NUMPAD0 = 51,
		KI_NUMPAD1 = 52,
		KI_NUMPAD2 = 53,
		KI_NUMPAD3 = 54,
		KI_NUMPAD4 = 55,
		KI_NUMPAD5 = 56,
		KI_NUMPAD6 = 57,
		KI_NUMPAD7 = 58,
		KI_NUMPAD8 = 59,
		KI_NUMPAD9 = 60,
		KI_NUMPADENTER = 61,
		KI_MULTIPLY = 62,			// Asterisk on the numeric keypad.
		KI_ADD = 63,				// Plus on the numeric keypad.
		KI_SEPARATOR = 64,
		KI_SUBTRACT = 65,			// Minus on the numeric keypad.
		KI_DECIMAL = 66,			// Period on the numeric keypad.
		KI_DIVIDE = 67,				// Forward Slash on the numeric keypad.

		/*
		 * NEC PC-9800 kbd definitions
		 */
		KI_OEM_NEC_EQUAL = 68,		// Equals key on the numeric keypad.

		KI_BACK = 69,				// Backspace key.
		KI_TAB = 70,				// Tab key.

		KI_CLEAR = 71,
		KI_RETURN = 72,

		KI_PAUSE = 73,
		KI_CAPITAL = 74,			// Capslock key.

		KI_KANA = 75,				// IME Kana mode.
		KI_HANGUL = 76,				// IME Hangul mode.
		KI_JUNJA = 77,				// IME Junja mode.
		KI_FINAL = 78,				// IME final mode.
		KI_HANJA = 79,				// IME Hanja mode.
		KI_KANJI = 80,				// IME Kanji mode.

		KI_ESCAPE = 81,				// Escape key.

		KI_CONVERT = 82,			// IME convert.
		KI_NONCONVERT = 83,			// IME nonconvert.
		KI_ACCEPT = 84,				// IME accept.
		KI_MODECHANGE = 85,			// IME mode change request.

		KI_PRIOR = 86,				// Page Up key.
		KI_NEXT = 87,				// Page Down key.
		KI_END = 88,
		KI_HOME = 89,
		KI_LEFT = 90,				// Left Arrow key.
		KI_UP = 91,					// Up Arrow key.
		KI_RIGHT = 92,				// Right Arrow key.
		KI_DOWN = 93,				// Down Arrow key.
		KI_SELECT = 94,
		KI_PRINT = 95,
		KI_EXECUTE = 96,
		KI_SNAPSHOT = 97,			// Print Screen key.
		KI_INSERT = 98,
		KI_DELETE = 99,
		KI_HELP = 100,

		KI_LWIN = 101,				// Left Windows key.
		KI_RWIN = 102,				// Right Windows key.
		KI_APPS = 103,				// Applications key.

		KI_POWER = 104,
		KI_SLEEP = 105,
		KI_WAKE = 106,

		KI_F1 = 107,
		KI_F2 = 108,
		KI_F3 = 109,
		KI_F4 = 110,
		KI_F5 = 111,
		KI_F6 = 112,
		KI_F7 = 113,
		KI_F8 = 114,
		KI_F9 = 115,
		KI_F10 = 116,
		KI_F11 = 117,
		KI_F12 = 118,
		KI_F13 = 119,
		KI_F14 = 120,
		KI_F15 = 121,
		KI_F16 = 122,
		KI_F17 = 123,
		KI_F18 = 124,
		KI_F19 = 125,
		KI_F20 = 126,
		KI_F21 = 127,
		KI_F22 = 128,
		KI_F23 = 129,
		KI_F24 = 130,

		KI_NUMLOCK = 131,			// Numlock key.
		KI_SCROLL = 132,			// Scroll Lock key.

		/*
		 * Fujitsu/OASYS kbd definitions
		 */
		KI_OEM_FJ_JISHO = 133,		// 'Dictionary' key.
		KI_OEM_FJ_MASSHOU = 134,	// 'Unregister word' key.
		KI_OEM_FJ_TOUROKU = 135,	// 'Register word' key.
		KI_OEM_FJ_LOYA = 136,		// 'Left OYAYUBI' key.
		KI_OEM_FJ_ROYA = 137,		// 'Right OYAYUBI' key.

		KI_LSHIFT = 138,
		KI_RSHIFT = 139,
		KI_LCONTROL = 140,
		KI_RCONTROL = 141,
		KI_LMENU = 142,
		KI_RMENU = 143,

		KI_BROWSER_BACK = 144,
		KI_BROWSER_FORWARD = 145,
		KI_BROWSER_REFRESH = 146,
		KI_BROWSER_STOP = 147,
		KI_BROWSER_SEARCH = 148,
		KI_BROWSER_FAVORITES = 149,
		KI_BROWSER_HOME = 150,

		KI_VOLUME_MUTE = 151,
		KI_VOLUME_DOWN = 152,
		KI_VOLUME_UP = 153,
		KI_MEDIA_NEXT_TRACK = 154,
		KI_MEDIA_PREV_TRACK = 155,
		KI_MEDIA_STOP = 156,
		KI_MEDIA_PLAY_PAUSE = 157,
		KI_LAUNCH_MAIL = 158,
		KI_LAUNCH_MEDIA_SELECT = 159,
		KI_LAUNCH_APP1 = 160,
		KI_LAUNCH_APP2 = 161,

		/*
		 * Various extended or enhanced keyboards
		 */
		KI_OEM_AX = 162,
		KI_ICO_HELP = 163,
		KI_ICO_00 = 164,

		KI_PROCESSKEY = 165,		// IME Process key.

		KI_ICO_CLEAR = 166,

		KI_ATTN = 167,
		KI_CRSEL = 168,
		KI_EXSEL = 169,
		KI_EREOF = 170,
		KI_PLAY = 171,
		KI_ZOOM = 172,
		KI_PA1 = 173,
		KI_OEM_CLEAR = 174,

		KI_LMETA = 175,
		KI_RMETA = 176
	};

	enum KeyModifier
	{
		KM_CTRL = 1 << 0,		// Set if at least one Ctrl key is depressed.
		KM_SHIFT = 1 << 1,		// Set if at least one Shift key is depressed.
		KM_ALT = 1 << 2,		// Set if at least one Alt key is depressed.
		KM_META = 1 << 3,		// Set if at least one Meta key (the command key) is depressed.
		KM_CAPSLOCK = 1 << 4,	// Set if caps lock is enabled.
		KM_NUMLOCK = 1 << 5,	// Set if num lock is enabled.
		KM_SCROLLLOCK = 1 << 6	// Set if scroll lock is enabled.
	};
}

}
}

#endif
