// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_VIDEO_CGAPAL_H
#define MAME_VIDEO_CGAPAL_H

#pragma once

static constexpr unsigned CGA_PALETTE_SETS = 83; // one for colour, one for mono, 81 for colour composite
extern const unsigned char cga_palette[16 * CGA_PALETTE_SETS][3];

#endif // MAME_VIDEO_CGAPAL_H
