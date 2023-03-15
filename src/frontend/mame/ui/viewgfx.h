// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/viewgfx.h

    Internal graphics viewer.

***************************************************************************/

#ifndef MAME_FRONTEND_MAME_UI_VIEWGFX_H
#define MAME_FRONTEND_MAME_UI_VIEWGFX_H

#pragma once

#include "ui/ui.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// master handler
uint32_t ui_gfx_ui_handler(render_container &container, mame_ui_manager &mui, bool uistate);


#endif // MAME_FRONTEND_MAME_UI_VIEWGFX_H
