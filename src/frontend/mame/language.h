// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    language.h

    Multi-language support.

***************************************************************************/
#ifndef MAME_FRONTEND_MAME_LANGUAGE_H
#define MAME_FRONTEND_MAME_LANGUAGE_H

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

//**************************************************************************
//  LOCALIZATION SUPPORT
//**************************************************************************

#define _(param)    lang_translate(param)
// Fake one to make possible using it in static text definitions, on those
// lang_translate must be called afterwards
#define __(param)   param

void load_translation(emu_options &option);
const char *lang_translate(const char *word);

#endif // MAME_FRONTEND_MAME_LANGUAGE_H
