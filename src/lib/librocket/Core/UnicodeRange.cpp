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

#include "precompiled.h"
#include "UnicodeRange.h"

namespace Rocket {
namespace Core {

UnicodeRange::UnicodeRange()
{
	min_codepoint = UINT_MAX;
	max_codepoint = UINT_MAX;
}

UnicodeRange::UnicodeRange(int _min_codepoint, int _max_codepoint)
{
	min_codepoint = _min_codepoint;
	max_codepoint = _max_codepoint;

	ROCKET_ASSERT(min_codepoint <= max_codepoint);
}

// Initialises the range from a unicode range in string form.
bool UnicodeRange::Initialise(const String& unicode_range)
{
	// Check for a 'U+' at the start.
	if (unicode_range.Length() < 2 ||
		unicode_range[0] != 'U' ||
		unicode_range[1] != '+')
		return false;

	// Check if there's a '-' sign; if so, we've got a range.
	String::size_type separator_index = unicode_range.Find("-", 2);
	if (separator_index != String::npos)
	{
		const char* end = unicode_range.CString() + separator_index;
		min_codepoint = strtoul(unicode_range.CString() + 2, (char **) &end, 16);

		end = unicode_range.CString() + unicode_range.Length();
		max_codepoint = strtoul(unicode_range.CString() + separator_index + 1, (char **) &end, 16);

		return min_codepoint <= max_codepoint;
	}

	// No range! Check if we have any wildcards.
	String::size_type wildcard_index = unicode_range.Find("?", 2);
	if (wildcard_index != String::npos)
	{
		String range_min(unicode_range.CString() + 2, unicode_range.CString() + wildcard_index);
		String range_max(range_min);

		for (String::size_type i = 0; i < unicode_range.Length() - wildcard_index; ++i)
		{
			range_min += "0";
			range_max += "F";
		}

		const char* end = range_min.CString() + range_min.Length();
		min_codepoint = strtoul(range_min.CString(), (char**) &end, 16);
		end = range_max.CString() + range_max.Length();
		max_codepoint = strtoul(range_max.CString(), (char**) &end, 16);

		return true;
	}

	const char* end = unicode_range.CString() + unicode_range.Length();
	min_codepoint = strtoul(unicode_range.CString() + 2, (char**) &end, 16);
	max_codepoint = min_codepoint;

	return true;
}

// Builds up a list of unicode ranges from a comma-separated list of unicode ranges in string form.
bool UnicodeRange::BuildList(UnicodeRangeList& list, const String& unicode_range)
{
	StringList unicode_ranges;
	StringUtilities::ExpandString(unicode_ranges, unicode_range);

	for (size_t i = 0; i < unicode_ranges.size(); ++i)
	{
		UnicodeRange range;
		if (!range.Initialise(unicode_ranges[i]))
			return false;

		list.push_back(range);
	}

	// Collapse contiguous ranges.
	for (size_t i = 0; i < list.size(); ++i)
	{
		size_t j = i + 1;
		while (j < list.size())
		{
			if (list[i].IsContiguous(list[j]))
			{
				list[i] = list[i].Join(list[j]);
				list.erase(list.begin() + j);
			}
			else
				++j;
		}
	}

	return !list.empty();
}

// Returns true if this range is wholly contained within another range.
bool UnicodeRange::IsContained(const UnicodeRange& rhs)
{
	return rhs.min_codepoint <= min_codepoint &&
		   rhs.max_codepoint >= max_codepoint;
}

// Returns true if this range is wholly contained within another range list.
bool UnicodeRange::IsContained(const UnicodeRangeList& rhs)
{
	for (size_t i = 0; i < rhs.size(); ++i)
	{
		if (IsContained(rhs[i]))
			return true;
	}

	return false;
}

// Returns true if this range is contained or contiguous with another range.
bool UnicodeRange::IsContiguous(const UnicodeRange& rhs)
{
	return (min_codepoint >= rhs.min_codepoint && min_codepoint <= ((rhs.max_codepoint == 0xFFFFFFFF) ? rhs.max_codepoint : rhs.max_codepoint + 1)) ||
		   (max_codepoint >= ((rhs.min_codepoint == 0) ? 0 : rhs.min_codepoint - 1) && max_codepoint <= rhs.max_codepoint);
}

// Joins this range with another that it is contiguous with.
UnicodeRange UnicodeRange::Join(const UnicodeRange& rhs)
{
	return UnicodeRange(Math::Min(min_codepoint, rhs.min_codepoint),
						   Math::Max(max_codepoint, rhs.max_codepoint));
}

}
}
