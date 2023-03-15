// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    language.h

    Multi-language support.

***************************************************************************/
#ifndef MAME_FRONTEND_MAME_LANGUAGE_H
#define MAME_FRONTEND_MAME_LANGUAGE_H

#pragma once

#include "util/language.h"


void load_translation(emu_options &options);

using util::lang_translate;

#endif // MAME_FRONTEND_MAME_LANGUAGE_H
