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

#ifndef ROCKETCOREUNICODERANGE_H
#define ROCKETCOREUNICODERANGE_H

namespace Rocket {
namespace Core {

class UnicodeRange;
typedef std::vector< UnicodeRange > UnicodeRangeList;

/**
 */

class UnicodeRange
{
public:
	UnicodeRange();
	UnicodeRange(int min_codepoint, int max_codepoint);

	/// Initialises the range from a unicode range in string form.
	/// @param[in] unicode_range The string specified the unicode range.
	/// @return True if the range is valid, false otherwise.
	bool Initialise(const String& unicode_range);

	/// Builds up a list of unicode ranges from a comma-separated list of unicode ranges in string form.
	/// @param[out] list The returned list.
	/// @param[in] unicode_range The comma-separated list of unicode ranges.
	/// @return True if all values were parsed successfully and at least one value is returned in the list, false otherwise.
	static bool BuildList(UnicodeRangeList& list, const String& unicode_range);

	/// Returns true if this range is wholly contained within another range.
	/// @param[in] rhs The range to check against.
	bool IsContained(const UnicodeRange& rhs);
	/// Returns true if this range is wholly contained within another range list.
	/// @param[in] rhs The range list to check against.
	bool IsContained(const UnicodeRangeList& rhs);

	/// Returns true if this range is contained or contiguous with another range.
	/// @param[in] rhs The range to check against.
	/// @return True if the ranges are contiguous, false otherwise.
	bool IsContiguous(const UnicodeRange& rhs);
	/// Joins this range with another that it is contiguous with.
	/// @param[in] rhs The range to join with this range.
	/// @return The new, joined, range containing both the original ranges.
	UnicodeRange Join(const UnicodeRange& rhs);

	unsigned int min_codepoint;
	unsigned int max_codepoint;
};

}
}

#endif
