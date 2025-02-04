// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Konami Video Helper functions */

#include "emu.h"
#include "konami_helper.h"

void konami_decode_gfx(device_gfx_interface &gfxdecode, int gfx_index, uint8_t *data, uint32_t total, const gfx_layout *layout, int bpp)
{
	gfx_layout gl;
	device_palette_interface &palette = gfxdecode.palette();

	memcpy(&gl, layout, sizeof(gl));
	gl.total = total;
	gfxdecode.set_gfx(gfx_index, std::make_unique<gfx_element>(&palette, gl, data, 0, palette.entries() >> bpp, 0));
}
