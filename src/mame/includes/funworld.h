// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Peter Ferrie
#ifndef MAME_INCLUDES_FUNWORLD_H
#define MAME_INCLUDES_FUNWORLD_H

#pragma once

#include "emupal.h"

class funworld_state : public driver_device
{
public:
	funworld_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void royalcd1(machine_config &config);
	void royalcd2(machine_config &config);
	void fw1stpal(machine_config &config);
	void fw2ndpal(machine_config &config);
	void saloon(machine_config &config);
	void cuoreuno(machine_config &config);
	void funquiz(machine_config &config);
	void witchryl(machine_config &config);
	void intrgmes(machine_config &config);
	void fw_brick_1(machine_config &config);
	void fw_brick_2(machine_config &config);

	void init_saloon();
	void init_royalcdc();
	void init_multiwin();
	void init_mongolnw();
	void init_soccernw();
	void init_tabblue();
	void init_dino4();
	void init_ctunk();

protected:
	DECLARE_WRITE8_MEMBER(funworld_videoram_w);
	DECLARE_WRITE8_MEMBER(funworld_colorram_w);
	DECLARE_WRITE8_MEMBER(funworld_lamp_a_w);
	DECLARE_WRITE8_MEMBER(funworld_lamp_b_w);

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override;
	void funworld_palette(palette_device &palette) const;
	uint32_t screen_update_funworld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void magicrd2_map(address_map &map);

	required_device<gfxdecode_device> m_gfxdecode;
	tilemap_t *m_bg_tilemap = nullptr;

private:
	DECLARE_READ8_MEMBER(questions_r);
	DECLARE_WRITE8_MEMBER(question_bank_w);
	DECLARE_WRITE_LINE_MEMBER(pia1_ca2_w);
	DECLARE_READ8_MEMBER(funquiz_ay8910_a_r);
	DECLARE_READ8_MEMBER(funquiz_ay8910_b_r);

	void cuoreuno_map(address_map &map);
	void funquiz_map(address_map &map);
	void funworld_map(address_map &map);
	void fw_a7_11_map(address_map &map);
	void intergames_map(address_map &map);
	void saloon_map(address_map &map);
	void witchryl_map(address_map &map);

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	output_finder<8> m_lamps;
};


class magicrd2_state : public funworld_state
{
public:
	using funworld_state::funworld_state;

	void magicrd2(machine_config &config);

	void init_magicd2b();
	void init_magicd2c();

protected:
	virtual void video_start() override;
};


class lunapark_state : public funworld_state
{
public:
	using funworld_state::funworld_state;

	void lunapark(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void lunapark_map(address_map &map);
};


class chinatow_state : public funworld_state
{
public:
	using funworld_state::funworld_state;

	void chinatow(machine_config &config);
	void rcdino4(machine_config &config);

	void init_rcdino4();
	void init_rcdinch();

protected:
	virtual void video_start() override;

private:
	DECLARE_READ8_MEMBER(chinatow_r_32f0);

	void chinatow_map(address_map &map);
};

#endif // MAME_INCLUDES_FUNWORLD_H
