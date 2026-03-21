// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, O. Galibert

#include "emu.h"
#include "luna_68k_bm.h"

luna_68k_bm_device::luna_68k_bm_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LUNA_68K_BM, tag, owner, clock)
	, device_luna_68k_video_interface(mconfig, *this)
	, m_acrtc(*this, "acrtc")
	, m_dac(*this, "dac")
	, m_screen(*this, "screen")
	, m_fb(*this, "fb")
{
}

void luna_68k_bm_device::device_start()
{
}

void luna_68k_bm_device::device_reset()
{
}

void luna_68k_bm_device::acrtc_display(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, uint16_t data)
{
}

u32 luna_68k_bm_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_acrtc->update_screen(screen, bitmap, cliprect);

	for(u32 y = 0; y != 1024; y++) {
		u16 *dest = &bitmap.pix(y);
		for(u16 x = 0; x != 80; x ++) {
			u16 v = m_fb[y * 128 + x];
			for(u16 xx = 0; xx != 16; xx ++)
				*dest++ = BIT(v, xx) ? 256 : 257;
		}
	}
	return 0;
}


void luna_68k_bm_device::vme_map(address_map &map)
{
	// d* is bitmap range, the text framebuffer may be visible there, who knows.
	map(0xd0000000, 0xd01fffff).unmaprw();
	map(0xd01f8000, 0xd01f8007).rw(m_acrtc, FUNC(hd63484_device::read16), FUNC(hd63484_device::write16)).umask32(0x0000ffff);
	map(0xd01ff000, 0xd01ff00f).m(m_dac, FUNC(bt458_device::map)).umask32(0x000000ff);
}

void luna_68k_bm_device::hd63484_map(address_map &map)
{
	map(0x00000, 0xfffff).ram().share(m_fb);
}


void luna_68k_bm_device::device_add_mconfig(machine_config &config)
{
	HD63484(config, m_acrtc, 108_MHz_XTAL/32);
	m_acrtc->set_screen(m_screen);
	m_acrtc->set_addrmap(0, &luna_68k_bm_device::hd63484_map);
	m_acrtc->set_display_callback(FUNC(luna_68k_bm_device::acrtc_display));

	BT458(config, m_dac, 108_MHz_XTAL);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(108_MHz_XTAL, 1728, 0, 1280, 1056, 0, 1024);
	m_screen->set_screen_update(FUNC(luna_68k_bm_device::screen_update));
	m_screen->set_palette(m_dac);
}

DEFINE_DEVICE_TYPE(LUNA_68K_BM, luna_68k_bm_device, "luna_68k_bm", "Omron Luna 68k BM VME board")
