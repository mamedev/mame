// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debughlp.h

    Debugger help engine.

*********************************************************************/

#ifndef MAME_EMU_DEBUG_DEBUGHLP_H
#define MAME_EMU_DEBUG_DEBUGHLP_H

#pragma once

#include <string_view>


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// help management
const char *debug_get_help(std::string_view tag);

#endif // MAME_EMU_DEBUG_DEBUGHLP_H
