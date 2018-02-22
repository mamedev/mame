// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Universal 8106-A2 + 8106-B PCB set

*************************************************************************/
#ifndef MAME_INCLUDES_LADYBUG_H
#define MAME_INCLUDES_LADYBUG_H

#pragma once

class ladybug_state : public driver_device
{
public:
	ladybug_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_colorram(*this, "colorram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_port_dsw0(*this, "DSW0"),
		m_p1_control(*this, "CONTP1"),
		m_p2_control(*this, "CONTP2")
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(ladybug_p1_control_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ladybug_p2_control_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin1_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(coin2_inserted);
	DECLARE_DRIVER_INIT(dorodon);
	void dorodon(machine_config &config);
	void ladybug(machine_config &config);

protected:
	DECLARE_WRITE8_MEMBER(ladybug_videoram_w);
	DECLARE_WRITE8_MEMBER(ladybug_colorram_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_grid_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(ladybug);
	uint32_t screen_update_ladybug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void palette_init_common( palette_device &palette, const uint8_t *color_prom,
								int r_bit0, int r_bit1, int g_bit0, int g_bit1, int b_bit0, int b_bit1 );

	void decrypted_opcodes_map(address_map &map);
	void ladybug_map(address_map &map);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

private:
	optional_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	optional_ioport m_port_dsw0;
	optional_ioport m_p1_control;
	optional_ioport m_p2_control;
};

#endif // MAME_INCLUDES_LADYBUG_H
