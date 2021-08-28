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

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

// is a given character a directory separator?

constexpr bool is_directory_separator(char c)
{
#if defined(WIN32)
	return ('\\' == c) || ('/' == c) || (':' == c);
#else
	return '/' == c;
#endif
}


// append to a path

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


// concatenate paths

template <typename T, typename... U>
inline std::string path_concat(T &&first, U &&... more)
{
	std::string result(std::forward<T>(first));
	if constexpr (sizeof...(U))
		path_append(result, std::forward<U>(more)...);
	return result;
}

} // namespace util

#endif // MAME_LIB_UTIL_PATH_H
