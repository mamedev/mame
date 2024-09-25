// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Grull Osgo, Peter Ferrie
#ifndef MAME_FUNWORLD_FUNWORLD_H
#define MAME_FUNWORLD_FUNWORLD_H

#pragma once

#include "emupal.h"
#include "machine/i2cmem.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "tilemap.h"

class funworld_state : public driver_device
{
public:
	funworld_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_palette(*this, "palette"),
		m_i2cmem(*this, "i2cmem"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void royalcd1(machine_config &config);
	void royalcd2(machine_config &config);
	void fw1stpal(machine_config &config);
	void fw2ndpal(machine_config &config);
	void saloon(machine_config &config);
	void clubcard(machine_config &config);
	void cuoreuno(machine_config &config);
	void funquiz(machine_config &config);
	void witchryl(machine_config &config);
	void fw_brick_1(machine_config &config);
	void fw_brick_2(machine_config &config);
	void gratispk(machine_config &config);

	void init_saloon();
	void init_mongolnw();
	void init_soccernw();
	void init_tabblue();
	void init_dino4();
	void init_ctunk();
	void init_jolycdig();
	void init_impera16();

protected:
	void funworld_videoram_w(offs_t offset, uint8_t data);
	void funworld_colorram_w(offs_t offset, uint8_t data);
	void funworld_lamp_a_w(uint8_t data);
	void funworld_lamp_b_w(uint8_t data);

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;
	void funworld_palette(palette_device &palette) const;
	uint32_t screen_update_funworld(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void magicrd2_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	tilemap_t *m_bg_tilemap = nullptr;

private:
	uint8_t questions_r(offs_t offset);
	void question_bank_w(uint8_t data);
	void pia1_ca2_w(int state);
	uint8_t funquiz_ay8910_a_r();
	uint8_t funquiz_ay8910_b_r();

	void clubcard_map(address_map &map) ATTR_COLD;
	void cuoreuno_map(address_map &map) ATTR_COLD;
	void funquiz_map(address_map &map) ATTR_COLD;
	void funworld_map(address_map &map) ATTR_COLD;
	void fw_brick_map(address_map &map) ATTR_COLD;
	void gratispk_map(address_map &map) ATTR_COLD;
	void saloon_map(address_map &map) ATTR_COLD;
	void witchryl_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_device<palette_device> m_palette;
	optional_device<i2cmem_device> m_i2cmem;
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
	virtual void video_start() override ATTR_COLD;
};


class lunapark_state : public funworld_state
{
public:
	using funworld_state::funworld_state;

	void lunapark(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void lunapark_map(address_map &map) ATTR_COLD;
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
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t chinatow_r_32f0(offs_t offset);

	void chinatow_map(address_map &map) ATTR_COLD;
};


class multiwin_state : public funworld_state
{
public:

	using funworld_state::funworld_state;

	void multiwin(machine_config& config);

	void driver_init();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t multiwin_opcode_r(offs_t offset);

	void multiwin_opcodes_map(address_map &map) ATTR_COLD;
};

class royalcrdf_state : public funworld_state
{
public:
	using funworld_state::funworld_state;

	void royalcrdf(machine_config& config);

	void driver_init();

private:
	uint8_t royalcrdf_opcode_r(offs_t offset);

	void royalcrdf_map(address_map &map) ATTR_COLD;
	void royalcrdf_opcodes_map(address_map &map) ATTR_COLD;
};

class intergames_state : public funworld_state
{
public:
	intergames_state(const machine_config &mconfig, device_type type, const char *tag) :
		funworld_state(mconfig, type, tag),
		m_crtc(*this, "crtc"),
		m_ay8910(*this, "ay8910"),
		m_crtc_selected(false)
	{ }

	void intrgmes(machine_config &config);

	void init_novop_a();
	void init_novop_b();
	void init_intgms();

protected:

	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t crtc_or_psg_r(offs_t offset);
	void crtc_or_psg_w(offs_t offset, uint8_t data);
	uint8_t prot_r(offs_t offset);
	void prot_w(offs_t offset, uint8_t data);

	void intergames_map(address_map &map) ATTR_COLD;

	required_device<mc6845_device> m_crtc;
	required_device<ay8910_device> m_ay8910;

	bool m_crtc_selected;
};

#endif // MAME_FUNWORLD_FUNWORLD_H
