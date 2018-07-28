// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TAITO_HELPER_H
#define MAME_VIDEO_TAITO_HELPER_H

#pragma once


// These scanline drawing routines, currently used by the pc080sn, tc0080vco, tc0150rod and tc0480scp devices, were lifted from Taito F3: optimise/merge?

void taitoic_drawscanline(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, const uint16_t *src, int transparent, uint32_t orient, bitmap_ind8 &priority, int pri);

#endif // MAME_VIDEO_TAITO_HELPER_H
