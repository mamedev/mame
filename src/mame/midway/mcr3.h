// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Midway MCR-3 system

**************************************************************************/
#ifndef MAME_MIDWAY_MCR3_H
#define MAME_MIDWAY_MCR3_H

#pragma once

#include "mcr.h"

#include "machine/74259.h"
#include "machine/adc0804.h"
#include "machine/adc0844.h"

#include "screen.h"
#include "tilemap.h"


class mcr3_state : public mcr_state
{
public:
	mcr3_state(const machine_config &mconfig, device_type type, const char *tag)
		: mcr_state(mconfig, type, tag)
		, m_spyhunt_alpharam(*this, "spyhunt_alpha")
		, m_screen(*this, "screen")
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void mcrmono(machine_config &config);
	void mono_tcs(machine_config &config);
	void mcrscroll(machine_config &config);
	void mono_sg(machine_config &config);

	void init_crater();
	void init_demoderm();
	void init_powerdrv();
	void init_stargrds();
	void init_rampage();
	void init_sarge();

protected:
	void mcr3_videoram_w(offs_t offset, uint8_t data);
	void spyhunt_videoram_w(offs_t offset, uint8_t data);
	void spyhunt_alpharam_w(offs_t offset, uint8_t data);
	void spyhunt_scroll_value_w(offs_t offset, uint8_t data);
	void mcrmono_control_port_w(uint8_t data);
	uint8_t demoderm_ip1_r();
	uint8_t demoderm_ip2_r();
	void demoderm_op6_w(uint8_t data);
	uint8_t rampage_ip4_r();
	void rampage_op6_w(uint8_t data);
	uint8_t powerdrv_ip2_r();
	void powerdrv_op5_w(uint8_t data);
	void powerdrv_op6_w(uint8_t data);
	uint8_t stargrds_ip0_r();
	void stargrds_op5_w(uint8_t data);
	void stargrds_op6_w(uint8_t data);

	DECLARE_VIDEO_START(spyhunt);
	void spyhunt_palette(palette_device &palette) const;

	uint32_t screen_update_mcr3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spyhunt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mcrmono_map(address_map &map) ATTR_COLD;
	void mcrmono_portmap(address_map &map) ATTR_COLD;
	void spyhunt_map(address_map &map) ATTR_COLD;
	void spyhunt_portmap(address_map &map) ATTR_COLD;

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

	optional_shared_ptr<uint8_t> m_spyhunt_alpharam;
	required_device<screen_device> m_screen;
	output_finder<3> m_lamps;

	uint8_t m_latched_input = 0;
	uint8_t m_spyhunt_sprite_color_mask = 0;
	int16_t m_spyhunt_scroll_offset = 0;
	int16_t m_spyhunt_scrollx = 0;
	int16_t m_spyhunt_scrolly = 0;
	tilemap_t *m_alpha_tilemap = nullptr;

	[[maybe_unused]] TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(mcrmono_get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(spyhunt_bg_scan);
	TILE_GET_INFO_MEMBER(spyhunt_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(spyhunt_get_alpha_tile_info);
	void mcr3_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int code_xor, int dx, int dy, int interlaced);
	void mcr_common_init();
};

class maxrpm_state : public mcr3_state
{
public:
	maxrpm_state(const machine_config &mconfig, device_type type, const char *tag)
		: mcr3_state(mconfig, type, tag)
		, m_maxrpm_adc(*this, "adc")
	{ }

	void maxrpm(machine_config &config);

	void init_maxrpm();

private:
	uint8_t maxrpm_ip1_r();
	uint8_t maxrpm_ip2_r();
	void maxrpm_op5_w(uint8_t data);
	void maxrpm_op6_w(uint8_t data);

	required_device<adc0844_device> m_maxrpm_adc;

	uint8_t m_maxrpm_adc_control = 0;
	uint8_t m_maxrpm_last_shift = 0;
	int8_t m_maxrpm_p1_shift = 0;
	int8_t m_maxrpm_p2_shift = 0;
};

class mcrsc_csd_state : public mcr3_state
{
public:
	mcrsc_csd_state(const machine_config &mconfig, device_type type, const char *tag)
		: mcr3_state(mconfig, type, tag)
		, m_adc(*this, "adc")
		, m_lamplatch(*this, "lamplatch")
		, m_analog_inputs(*this, {"ssio:IP2", "ssio:IP2.ALT"})
	{ }

	void mcrsc_csd(machine_config &config);
	void spyhunt(machine_config &config);
	void turbotag(machine_config &config);

	void init_spyhunt();
	void init_turbotag();

private:
	uint8_t spyhunt_ip1_r();
	uint8_t spyhunt_ip2_r();
	void spyhunt_op4_w(uint8_t data);
	uint8_t turbotag_ip2_r();
	uint8_t turbotag_kludge_r();

	required_device<adc0804_device> m_adc;
	optional_device<cd4099_device> m_lamplatch;
	required_ioport_array<2> m_analog_inputs;
};

#endif // MAME_MIDWAY_MCR3_H
