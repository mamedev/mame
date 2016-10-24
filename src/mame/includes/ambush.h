// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Ambush

*************************************************************************/

class ambush_state : public driver_device
{
public:
	ambush_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_scrollram(*this, "scrollram"),
		m_colorbank(*this, "colorbank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_colorbank;

	void ambush_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_init_ambush(palette_device &palette);
	uint32_t screen_update_ambush(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_chars( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
