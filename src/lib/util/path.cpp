// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Nathan Woods
/***************************************************************************

    path.cpp

    Path and filename utilities.

***************************************************************************/

#include "path.h"

#include <algorithm>
#include <cctype>
#include <iterator>


/***************************************************************************
    FILENAME UTILITIES
***************************************************************************/

// -------------------------------------------------
// core_filename_extract_base - extract the base
// name from a filename; note that this makes
// assumptions about path separators
// -------------------------------------------------

std::string_view core_filename_extract_base(std::string_view name, bool strip_extension) noexcept
{
	// find the start of the basename
	auto const start = std::find_if(name.rbegin(), name.rend(), &util::is_directory_separator);
	if (start == name.rbegin())
		return std::string_view();

	// find the end of the basename
	auto const chop_position = strip_extension
		? std::find(name.rbegin(), start, '.')
		: start;
	auto const end = ((chop_position != start) && (std::next(chop_position) != start))
		? std::next(chop_position)
		: name.rbegin();

	return std::string_view(&*start.base(), end.base() - start.base());
}


// -------------------------------------------------
// core_filename_extract_extension
// -------------------------------------------------

std::string_view core_filename_extract_extension(std::string_view filename, bool strip_period) noexcept
{
	auto loc = filename.find_last_of('.');
	if (loc != std::string_view::npos)
		return filename.substr(loc + (strip_period ? 1 : 0));
	else
		return std::string_view();
}


// -------------------------------------------------
// core_filename_ends_with - does the given
// filename end with the specified extension?
// -------------------------------------------------

bool core_filename_ends_with(std::string_view filename, std::string_view extension) noexcept
{
	auto namelen = filename.length();
	auto extlen = extension.length();

	// first if the extension is bigger than the name, we definitely don't match
	bool matches = namelen >= extlen;

	// work backwards checking for a match
	while (matches && extlen > 0 && namelen > 0)
	{
		if (std::tolower(uint8_t(filename[--namelen])) != std::tolower(uint8_t(extension[--extlen])))
			matches = false;
	}

	return matches;
}
