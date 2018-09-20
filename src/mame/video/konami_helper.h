// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Konami video helper functions */
#ifndef MAME_VIDEO_KONAMI_HELPER_H
#define MAME_VIDEO_KONAMI_HELPER_H

#pragma once

void konami_decode_gfx(device_gfx_interface &gfxdecode, int gfx_index, uint8_t *data, uint32_t total, const gfx_layout *layout, int bpp);

// helper function to sort three tile layers by priority order
void konami_sortlayers3(int *layer, int *pri);
// helper function to sort four tile layers by priority order
void konami_sortlayers4(int *layer, int *pri);
// helper function to sort five tile layers by priority order
void konami_sortlayers5(int *layer, int *pri);

#endif // MAME_VIDEO_KONAMI_HELPER_H
