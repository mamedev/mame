// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Ambush

*************************************************************************/

class ambush_state : public driver_device
{
public:
	ambush_state(const machine_config &mconfig, device_type type, std::string tag)
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
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_scrollram;
	required_shared_ptr<UINT8> m_colorbank;

	DECLARE_WRITE8_MEMBER(ambush_coin_counter_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_PALETTE_INIT(ambush);
	UINT32 screen_update_ambush(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_chars( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
