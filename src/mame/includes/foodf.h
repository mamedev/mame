// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Food Fight hardware

*************************************************************************/
#ifndef MAME_INCLUDES_FOODF_H
#define MAME_INCLUDES_FOODF_H

#pragma once

#include "machine/atarigen.h"
#include "machine/timer.h"
#include "machine/x2212.h"

class foodf_state : public atarigen_state
{
public:
	foodf_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
		m_nvram(*this, "nvram"),
		m_playfield_tilemap(*this, "playfield"),
		m_scan_timer(*this, "scan_timer"),
		m_spriteram(*this, "spriteram"),
		m_leds(*this, "led%u", 0U)
	{ }

	void foodf(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void update_interrupts() override;
	DECLARE_WRITE16_MEMBER(nvram_recall_w);
	DECLARE_WRITE8_MEMBER(digital_w);
	DECLARE_WRITE16_MEMBER(foodf_paletteram_w);
	void foodf_set_flip(int flip);
	DECLARE_READ8_MEMBER(pot_r);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_foodf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update_timer);

	void main_map(address_map &map);

	required_device<x2212_device> m_nvram;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<timer_device> m_scan_timer;

	double          m_rweights[3];
	double          m_gweights[3];
	double          m_bweights[2];
	uint8_t           m_playfield_flip;

	required_shared_ptr<uint16_t> m_spriteram;
	output_finder<2> m_leds;
};

#endif // MAME_INCLUDES_FOODF_H
