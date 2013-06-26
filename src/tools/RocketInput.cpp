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

#include <RocketInput.h>

/**
	This map contains 4 different mappings from key identifiers to character codes. Each entry represents a different
	combination of shift and capslock state.
 */

char ascii_map[4][51] =
{
    // shift off and capslock off
    {
		0,
		' ',
		'0',
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'a',
		'b',
		'c',
		'd',
		'e',
		'f',
		'g',
		'h',
		'i',
		'j',
		'k',
		'l',
		'm',
		'n',
		'o',
		'p',
		'q',
		'r',
		's',
		't',
		'u',
		'v',
		'w',
		'x',
		'y',
		'z',
		';',
		'=',
		',',
		'-',
		'.',
		'/',
		'`',
		'[',
		'\\',
		']',
		'\'',
		0,
		0
	},

	// shift on and capslock off
    {
		0,
		' ',
		')',
		'!',
		'@',
		'#',
		'$',
		'%',
		'^',
		'&',
		'*',
		'(',
		'A',
		'B',
		'C',
		'D',
		'E',
		'F',
		'G',
		'H',
		'I',
		'J',
		'K',
		'L',
		'M',
		'N',
		'O',
		'P',
		'Q',
		'R',
		'S',
		'T',
		'U',
		'V',
		'W',
		'X',
		'Y',
		'Z',
		':',
		'+',
		'<',
		'_',
		'>',
		'?',
		'~',
		'{',
		'|',
		'}',
		'"',
		0,
		0
	},

	// shift on and capslock on
    {
		0,
		' ',
		')',
		'!',
		'@',
		'#',
		'$',
		'%',
		'^',
		'&',
		'*',
		'(',
		'a',
		'b',
		'c',
		'd',
		'e',
		'f',
		'g',
		'h',
		'i',
		'j',
		'k',
		'l',
		'm',
		'n',
		'o',
		'p',
		'q',
		'r',
		's',
		't',
		'u',
		'v',
		'w',
		'x',
		'y',
		'z',
		':',
		'+',
		'<',
		'_',
		'>',
		'?',
		'~',
		'{',
		'|',
		'}',
		'"',
		0,
		0
	},

	// shift off and capslock on
    {
		0,
		' ',
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'0',
		'A',
		'B',
		'C',
		'D',
		'E',
		'F',
		'G',
		'H',
		'I',
		'J',
		'K',
		'L',
		'M',
		'N',
		'O',
		'P',
		'Q',
		'R',
		'S',
		'T',
		'U',
		'V',
		'W',
		'X',
		'Y',
		'Z',
		';',
		'=',
		',',
		'-',
		'.',
		'/',
		'`',
		'[',
		'\\',
		']',
		'\'',
		0,
		0
	}      
};

char keypad_map[2][18] = 
{
	{
		'0',
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'\n',
		'*',
		'+',
		0,
		'-',
		'.',
		'/',
		'='
	},

	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		'\n',
		'*',
		'+',
		0,
		'-',
		0,
		'/',
		'='
	}
};



Rocket::Core::Context* Input::context = NULL;



// Sets the context to send input events to.
void Input::SetContext(Rocket::Core::Context* _context)
{
	context = _context;
}



// Returns the character code for a key identifer / key modifier combination.
Rocket::Core::word Input::GetCharacterCode(Rocket::Core::Input::KeyIdentifier key_identifier, int key_modifier_state)
{
	// Check if we have a keycode capable of generating characters on the main keyboard (ie, not on the numeric
	// keypad; that is dealt with below).
	if (key_identifier <= Rocket::Core::Input::KI_OEM_102)
	{
		// Get modifier states
		bool shift = (key_modifier_state & Rocket::Core::Input::KM_SHIFT) > 0;
		bool capslock = (key_modifier_state & Rocket::Core::Input::KM_CAPSLOCK) > 0;

		// Return character code based on identifier and modifiers
		if (shift && !capslock)
			return ascii_map[1][key_identifier];

		if (shift && capslock)
			return ascii_map[2][key_identifier];	

		if (!shift && capslock)
			return ascii_map[3][key_identifier];

		return ascii_map[0][key_identifier];
	}

	// Check if we have a keycode from the numeric keypad.
	else if (key_identifier <= Rocket::Core::Input::KI_OEM_NEC_EQUAL)
	{
		if (key_modifier_state & Rocket::Core::Input::KM_NUMLOCK)
			return keypad_map[0][key_identifier - Rocket::Core::Input::KI_NUMPAD0];
		else
			return keypad_map[1][key_identifier - Rocket::Core::Input::KI_NUMPAD0];
	}

	else if (key_identifier == Rocket::Core::Input::KI_RETURN)
		return '\n';

	return 0;
}
