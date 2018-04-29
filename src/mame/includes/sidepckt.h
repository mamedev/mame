// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/******************************************************************************

    Data East Side Pocket hardware

******************************************************************************/

#include "machine/gen_latch.h"

class sidepckt_state : public driver_device
{
public:
	sidepckt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_bg_tilemap;
	const uint8_t* m_prot_table[3];
	uint8_t m_i8751_return;
	uint8_t m_current_ptr;
	uint8_t m_current_table;
	uint8_t m_in_math;
	uint8_t m_math_param;
	uint8_t m_scroll_y;

	DECLARE_READ8_MEMBER(i8751_r);
	DECLARE_WRITE8_MEMBER(i8751_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_READ8_MEMBER(scroll_y_r);
	DECLARE_WRITE8_MEMBER(scroll_y_w);

	void init_sidepckt();
	void init_sidepcktj();

	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(sidepckt);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void sidepcktb(machine_config &config);
	void sidepckt(machine_config &config);
	void sidepckt_map(address_map &map);
	void sidepcktb_map(address_map &map);
	void sound_map(address_map &map);
};
