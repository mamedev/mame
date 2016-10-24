// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "includes/kaneko16.h"

class galpanic_state : public driver_device
{
public:
	galpanic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_bgvideoram(*this, "bgvideoram"),
			m_fgvideoram(*this, "fgvideoram"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_pandora(*this, "pandora")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_bgvideoram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<kaneko_pandora_device> m_pandora;

	bitmap_ind16 m_bitmap;

	void m6295_bankswitch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_galpanic(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void scanline(timer_device &timer, void *ptr, int32_t param);
	void draw_fgbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
