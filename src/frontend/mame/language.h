// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    language.h

    Multi-language support.

***************************************************************************/
#ifndef MAME_FRONTEND_MAME_LANGUAGE_H
#define MAME_FRONTEND_MAME_LANGUAGE_H

#pragma once

#include <string_view>


//**************************************************************************
//  LOCALIZATION SUPPORT
//**************************************************************************

#define _(...) lang_translate(__VA_ARGS__)

#define N_(msg) (msg)
#define N_p(ctx, msg) (msg)

void load_translation(emu_options &option);

char const *lang_translate(char const *message);
std::string_view lang_translate(std::string_view message);

char const *lang_translate(char const *context, char const *message);
std::string_view lang_translate(char const *context, std::string_view message);
std::string_view lang_translate(std::string_view context, std::string_view message);

#endif // MAME_FRONTEND_MAME_LANGUAGE_H
