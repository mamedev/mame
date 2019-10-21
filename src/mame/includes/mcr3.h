// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Midway MCR-3 system

**************************************************************************/
#ifndef MAME_INCLUDES_MCR3_H
#define MAME_INCLUDES_MCR3_H

#pragma once

#include "includes/mcr.h"

#include "machine/74259.h"
#include "machine/adc0844.h"
#include "screen.h"
#include "tilemap.h"

class mcr3_state : public mcr_state
{
public:
	mcr3_state(const machine_config &mconfig, device_type type, const char *tag)
		: mcr_state(mconfig, type, tag)
		, m_spyhunt_alpharam(*this, "spyhunt_alpha")
		, m_maxrpm_adc(*this, "adc")
		, m_lamplatch(*this, "lamplatch")
		, m_screen(*this, "screen")
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	DECLARE_WRITE8_MEMBER(mcr3_videoram_w);
	DECLARE_WRITE8_MEMBER(spyhunt_videoram_w);
	DECLARE_WRITE8_MEMBER(spyhunt_alpharam_w);
	DECLARE_WRITE8_MEMBER(spyhunt_scroll_value_w);
	DECLARE_WRITE8_MEMBER(mcrmono_control_port_w);
	DECLARE_READ8_MEMBER(demoderm_ip1_r);
	DECLARE_READ8_MEMBER(demoderm_ip2_r);
	DECLARE_WRITE8_MEMBER(demoderm_op6_w);
	DECLARE_READ8_MEMBER(maxrpm_ip1_r);
	DECLARE_READ8_MEMBER(maxrpm_ip2_r);
	DECLARE_WRITE8_MEMBER(maxrpm_op5_w);
	DECLARE_WRITE8_MEMBER(maxrpm_op6_w);
	DECLARE_READ8_MEMBER(rampage_ip4_r);
	DECLARE_WRITE8_MEMBER(rampage_op6_w);
	DECLARE_READ8_MEMBER(powerdrv_ip2_r);
	DECLARE_WRITE8_MEMBER(powerdrv_op5_w);
	DECLARE_WRITE8_MEMBER(powerdrv_op6_w);
	DECLARE_READ8_MEMBER(stargrds_ip0_r);
	DECLARE_WRITE8_MEMBER(stargrds_op5_w);
	DECLARE_WRITE8_MEMBER(stargrds_op6_w);
	DECLARE_READ8_MEMBER(spyhunt_ip1_r);
	DECLARE_READ8_MEMBER(spyhunt_ip2_r);
	DECLARE_WRITE8_MEMBER(spyhunt_op4_w);
	DECLARE_READ8_MEMBER(turbotag_ip2_r);
	DECLARE_READ8_MEMBER(turbotag_kludge_r);
	void init_crater();
	void init_demoderm();
	void init_turbotag();
	void init_powerdrv();
	void init_stargrds();
	void init_maxrpm();
	void init_rampage();
	void init_spyhunt();
	void init_sarge();
	DECLARE_VIDEO_START(spyhunt);
	void spyhunt_palette(palette_device &palette) const;

	uint32_t screen_update_mcr3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spyhunt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mcrmono(machine_config &config);
	void maxrpm(machine_config &config);
	void mcrsc_csd(machine_config &config);
	void mono_tcs(machine_config &config);
	void mcrscroll(machine_config &config);
	void mono_sg(machine_config &config);
	void mcrmono_map(address_map &map);
	void mcrmono_portmap(address_map &map);
	void spyhunt_map(address_map &map);
	void spyhunt_portmap(address_map &map);
protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override;

private:
	optional_shared_ptr<uint8_t> m_spyhunt_alpharam;
	optional_device<adc0844_device> m_maxrpm_adc;
	optional_device<cd4099_device> m_lamplatch;
	required_device<screen_device> m_screen;
	output_finder<3> m_lamps;

	uint8_t m_latched_input;
	uint8_t m_maxrpm_adc_control;
	uint8_t m_maxrpm_last_shift;
	int8_t m_maxrpm_p1_shift;
	int8_t m_maxrpm_p2_shift;
	uint8_t m_spyhunt_sprite_color_mask;
	int16_t m_spyhunt_scroll_offset;
	int16_t m_spyhunt_scrollx;
	int16_t m_spyhunt_scrolly;
	tilemap_t *m_alpha_tilemap;

	TILE_GET_INFO_MEMBER(mcrmono_get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(spyhunt_bg_scan);
	TILE_GET_INFO_MEMBER(spyhunt_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(spyhunt_get_alpha_tile_info);
	void mcr3_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int code_xor, int dx, int dy, int interlaced);
	void mcr_common_init();
};

#endif // MAME_INCLUDES_MCR3_H
