// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

	language.h

	Multi-language support.

***************************************************************************/
#pragma once

#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include "emu.h"

void load_translation(emu_options &option);
const char *lang_translate(const char *word);

#endif /*__LANGUAGE_H__*/

