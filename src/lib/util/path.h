// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Aaron Giles
/***************************************************************************

    path.h

    Filesystem path utilties

***************************************************************************/
#ifndef MAME_LIB_UTIL_PATH_H
#define MAME_LIB_UTIL_PATH_H

#include "osdfile.h" // for PATH_SEPARATOR

#include <string>
#include <string_view>
#include <utility>


namespace util {

/// \defgroup pathutils Filesystem path utilities
/// \{

/// \brief Is a character a directory separator?
///
/// Determine whether a character is used to separate components within
/// a filesystem path.
/// \param [in] c A character to test.
/// \return True if the character is used to separate components in
///   filesystem paths.
constexpr bool is_directory_separator(char c)
{
#if defined(_WIN32)
	return ('\\' == c) || ('/' == c) || (':' == c);
#else
	return '/' == c;
#endif
}


/// \brief Append components to a filesystem path
///
/// Appends directory components to a filesystem path.
/// \param [in,out] path The path to append to.
/// \param [in] next The first directory component to append to the
///   path.
/// \param [in] more Additional directory components to append to the
///   path.
/// \return A reference to the modified path.
template <typename T, typename... U>
inline std::string &path_append(std::string &path, T &&next, U &&... more)
{
	if (!path.empty() && !is_directory_separator(path.back()))
		path.append(PATH_SEPARATOR);
	path.append(std::forward<T>(next));
	if constexpr (sizeof...(U) > 0U)
		return path_append(path, std::forward<U>(more)...);
	else
		return path;
}


/// \brief Concatenate filsystem paths
///
/// Concatenates multiple filesystem paths.
/// \param [in] first Initial filesystem path.
/// \param [in] more Additional directory components to append to the
///   intial path.
/// \return The concatenated filesystem path.
template <typename T, typename... U>
inline std::string path_concat(T &&first, U &&... more)
{
	std::string result(std::forward<T>(first));
	if constexpr (sizeof...(U) > 0U)
		path_append(result, std::forward<U>(more)...);
	return result;
}

/// \}

} // namespace util


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- filename utilities ----- */

// extract the base part of a filename (remove extensions and paths)
std::string_view core_filename_extract_base(std::string_view name, bool strip_extension = false) noexcept;

// extracts the file extension from a filename
std::string_view core_filename_extract_extension(std::string_view filename, bool strip_period = false) noexcept;

// true if the given filename ends with a particular extension
bool core_filename_ends_with(std::string_view filename, std::string_view extension) noexcept;

#endif // MAME_LIB_UTIL_PATH_H
