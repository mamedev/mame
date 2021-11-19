// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    path.h

    Filesystem path utilties

***************************************************************************/
#ifndef MAME_LIB_UTIL_PATH_H
#define MAME_LIB_UTIL_PATH_H

#include "osdfile.h" // for PATH_SEPARATOR

#include <string>
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
	if constexpr (sizeof...(U))
		return path_append(std::forward<U>(more)...);
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
	if constexpr (sizeof...(U))
		path_append(result, std::forward<U>(more)...);
	return result;
}

/// \}

} // namespace util

#endif // MAME_LIB_UTIL_PATH_H
