// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    language.h

    Multi-language support.

***************************************************************************/
#ifndef MAME_LIB_UTIL_LANGUAGE_H
#define MAME_LIB_UTIL_LANGUAGE_H

#pragma once

#include "utilfwd.h"

#include <string_view>


//**************************************************************************
//  LOCALIZATION SUPPORT
//**************************************************************************

#define _(...) (::util::lang_translate(__VA_ARGS__))

#define N_(msg) (msg)
#define N_p(ctx, msg) (msg)

namespace util {

void unload_translation();
void load_translation(random_read &file);

char const *lang_translate(char const *message);
std::string_view lang_translate(std::string_view message);

char const *lang_translate(char const *context, char const *message);
std::string_view lang_translate(char const *context, std::string_view message);
std::string_view lang_translate(std::string_view context, std::string_view message);

// allow UTF-8 messages to be used as keys transparently
inline char const *lang_translate(char8_t const *message)
{ return lang_translate(reinterpret_cast<char const *>(message)); }
inline std::string_view lang_translate(std::u8string_view message)
{ return lang_translate(std::string_view(reinterpret_cast<char const *>(message.data()), message.size())); }

// allow UTF-8 messages to be used as keys transparently (with context)
inline char const *lang_translate(char const *context, char8_t const *message)
{ return lang_translate(context, reinterpret_cast<char const *>(message)); }
inline std::string_view lang_translate(char const *context, std::u8string_view message)
{ return lang_translate(context, std::string_view(reinterpret_cast<char const *>(message.data()), message.size())); }
inline std::string_view lang_translate(std::string_view context, std::u8string_view message)
{ return lang_translate(context, std::string_view(reinterpret_cast<char const *>(message.data()), message.size())); }

} // namespace util

#endif // MAME_LIB_UTIL_LANGUAGE_H
