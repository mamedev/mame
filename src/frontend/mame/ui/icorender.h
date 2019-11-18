// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/icorender.h

    Windows icon file parser.

    File handles passed to these functions must support read and seek
    operations.

***************************************************************************/
#ifndef MAME_FRONTEND_MAME_UI_ICORENDER_H
#define MAME_FRONTEND_MAME_UI_ICORENDER_H

#pragma once

namespace ui {

// get number of images in icon file (-1 on error)
int images_in_ico(util::core_file &fp);

// load specified icon from file (zero-based)
void render_load_ico(util::core_file &fp, unsigned index, bitmap_argb32 &bitmap);

// load first supported icon from file
void render_load_ico_first(util::core_file &fp, bitmap_argb32 &bitmap);

// load highest detail supported icon from file
void render_load_ico_highest_detail(util::core_file &fp, bitmap_argb32 &bitmap);

} // namespace ui

#endif // MAME_FRONTEND_MAME_UI_ICORENDER_H
