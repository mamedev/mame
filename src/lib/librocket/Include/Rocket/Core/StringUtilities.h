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

#ifndef ROCKETCORESTRINGUTILITIES_H
#define ROCKETCORESTRINGUTILITIES_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Types.h>
#include <Rocket/Core/String.h>
#include <stdarg.h>

namespace Rocket {
namespace Core {

/**
	Helper functions for string manipulation.
	@author Lloyd Weehuizen
 */

class ROCKETCORE_API StringUtilities
{
public:
	/// Expands character-delimited list of values in a single string to a whitespace-trimmed list
	/// of values.
	/// @param[out] string_list Resulting list of values.
	/// @param[in] string String to expand.
	/// @param[in] delimiter Delimiter found between entries in the string list.
	static void ExpandString(StringList& string_list, const String& string, const char delimiter = ',');
	/// Joins a list of string values into a single string separated by a character delimiter.
	/// @param[out] string Resulting concatenated string.
	/// @param[in] string_list Input list of string values.
	/// @param[in] delimiter Delimiter to insert between the individual values.
	static void JoinString(String& string, const StringList& string_list, const char delimiter = ',');

	/// Hashes a string of data to an integer value using the FNV algorithm.
	/// @param[in] data Data to hash.
	/// @param[in] length Length of the string to hash. If this is -1, the data will be interpreted as a C string.
	/// @return Integer hash of the data.
	static Hash FNVHash(const char* data, int length = -1);

	/// Converts a character array in UTF-8 encoding to a vector of words. The UCS-2 words will be encoded as
	/// either big- or little-endian, depending on the host processor.
	/// @param[in] input Input string in UTF-8 encoding.
	/// @param[out] output Output vector of UCS-2 characters.
	/// @return True if the conversion went successfully, false if any characters had to be skipped (this will occur if they can't fit into UCS-2).
	static bool UTF8toUCS2(const String& input, std::vector< word >& output);
	/// Converts a vector of words in UCS-2 encoding into a character array in UTF-8 encoding. This
	/// function assumes the endianness of the input words to be the same as the host processor.
	/// @param[in] input Input vector in UCS-2 encoding.
	/// @param[out] output Output string in UTF-8 encoding.
	/// @return True if the conversion went successfully, false if not.
	static bool UCS2toUTF8(const std::vector< word >& input, String& output);
	/// Converts an array of words in UCS-2 encoding into a character array in UTF-8 encoding. This
	/// function assumes the endianness of the input words to be the same as the host processor.
	/// @param[in] input Input array of words in UCS-2 encoding.
	/// @param[in] input_size Length of the input array.
	/// @param[out] output Output string in UTF-8 encoding.
	/// @return True if the conversion went successfully, false if not.
	static bool UCS2toUTF8(const word* input, size_t input_size, String& output);

	/// Checks if a given value is a whitespace character.
	/// @param[in] x The character to evaluate.
	/// @return True if the character is whitespace, false otherwise.
	template < typename CharacterType >
	static bool IsWhitespace(CharacterType x)
	{
		return (x == '\r' || x == '\n' || x == ' ' || x == '\t');
	}

	/// Strip whitespace characters from the beginning and end of a string.
	/// @param[in] string The string to trim.
	/// @return The stripped string.
	static String StripWhitespace(const String& string);

	/// Operator for STL containers using strings.
	struct ROCKETCORE_API StringComparei
	{
		bool operator()(const String& lhs, const String& rhs) const;
	};
};

}
}

#endif
