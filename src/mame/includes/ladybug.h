// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Universal 8106-A2 + 8106-B PCB set

*************************************************************************/
#ifndef MAME_INCLUDES_LADYBUG_H
#define MAME_INCLUDES_LADYBUG_H

#pragma once

#include "video/ladybug.h"
#include "emupal.h"
#include "tilemap.h"


class ladybug_base_state : public driver_device
{
protected:
	using driver_device::driver_device;

	void palette_init_common(
			palette_device &palette, const uint8_t *color_prom,
			int r_bit0, int r_bit1,
			int g_bit0, int g_bit1,
			int b_bit0, int b_bit1) const;
};


// ladybug platform
class ladybug_state : public ladybug_base_state
{
public:
	ladybug_state(const machine_config &mconfig, device_type type, const char *tag)
		: ladybug_base_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_video(*this, "video")
		, m_port_dsw0(*this, "DSW0")
		, m_p1_control(*this, "CONTP1")
		, m_p2_control(*this, "CONTP2")
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(ladybug_p1_control_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ladybug_p2_control_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin1_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(coin2_inserted);
	void ladybug(machine_config &config);

protected:
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	void ladybug_palette(palette_device &palette) const;
	uint32_t screen_update_ladybug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ladybug_map(address_map &map);

	required_device<cpu_device> m_maincpu;

private:
	required_device<ladybug_video_device> m_video;

	required_ioport m_port_dsw0;
	optional_ioport m_p1_control;
	optional_ioport m_p2_control;
};


// ladybug plus program decryption
class dorodon_state : public ladybug_state
{
public:
	dorodon_state(const machine_config &mconfig, device_type type, const char *tag)
		: ladybug_state(mconfig, type, tag)
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void init_dorodon();
	void dorodon(machine_config &config);

protected:
	void decrypted_opcodes_map(address_map &map);

private:
	required_shared_ptr<uint8_t> m_decrypted_opcodes;
};


// graphics from ladybug, stars from zerohour, plus grid layer
class sraider_state : public ladybug_base_state
{
public:
	sraider_state(const machine_config &mconfig, device_type type, const char *tag)
		: ladybug_base_state(mconfig, type, tag)
		, m_grid_data(*this, "grid_data")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_video(*this, "video")
		, m_stars(*this, "stars")
	{ }

	void sraider(machine_config &config);

protected:
	DECLARE_READ8_MEMBER(sraider_8005_r);
	DECLARE_WRITE8_MEMBER(sraider_misc_w);
	DECLARE_WRITE8_MEMBER(sraider_io_w);
	void sraider_palette(palette_device &palette) const;
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_sraider);
	TILE_GET_INFO_MEMBER(get_grid_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_sraider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sraider_cpu1_map(address_map &map);
	void sraider_cpu2_io_map(address_map &map);
	void sraider_cpu2_map(address_map &map);

private:
	required_shared_ptr<uint8_t> m_grid_data;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ladybug_video_device> m_video;
	required_device<zerohour_stars_device> m_stars;

	tilemap_t   *m_grid_tilemap;

	uint8_t m_grid_color;
	uint8_t m_sraider_0x30;
	uint8_t m_sraider_0x38;
	uint8_t m_weird_value[8];
};

#endif // MAME_INCLUDES_LADYBUG_H
