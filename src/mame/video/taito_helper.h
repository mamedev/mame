// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#pragma once

#include "emu.h"

#ifndef __TAITO_HELPER_H__
#define __TAITO_HELPER_H__

/* These scanline drawing routines, currently used by the pc080sn, tc0080vco, tc0150rod and tc0480scp devices, were lifted from Taito F3: optimise / merge ? */

void taitoic_drawscanline( bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, const UINT16 *src, int transparent, UINT32 orient, bitmap_ind8 &priority, int pri);

#endif
