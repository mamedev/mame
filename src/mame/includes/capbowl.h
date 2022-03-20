// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

*************************************************************************/
#ifndef MAME_INCLUDES_CAPBOWL_H
#define MAME_INCLUDES_CAPBOWL_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "video/tms34061.h"
#include "screen.h"

class capbowl_state : public driver_device
{
public:
	enum
	{
		TIMER_UPDATE
	};

	capbowl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_audiocpu(*this, "audiocpu"),
		m_tms34061(*this, "tms34061"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_rowaddress(*this, "rowaddress")
	{ }

	void init_capbowl();
	void bowlrama(machine_config &config);
	void capbowl(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<cpu_device> m_audiocpu;
	required_device<tms34061_device> m_tms34061;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_rowaddress;

	/* video-related */
	offs_t m_blitter_addr = 0U;

	/* input-related */
	uint8_t m_last_trackball_val[2]{};

	emu_timer *m_update_timer = nullptr;

	// common
	uint8_t track_0_r();
	uint8_t track_1_r();
	void track_reset_w(uint8_t data);
	void sndcmd_w(uint8_t data);
	void tms34061_w(offs_t offset, uint8_t data);
	uint8_t tms34061_r(offs_t offset);

	// capbowl specific
	void capbowl_rom_select_w(uint8_t data);

	// bowlrama specific
	void bowlrama_blitter_w(offs_t offset, uint8_t data);
	uint8_t bowlrama_blitter_r(offs_t offset);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_CALLBACK_MEMBER(update);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline rgb_t pen_for_pixel( uint8_t const *src, uint8_t pix );

	void bowlrama_map(address_map &map);
	void capbowl_map(address_map &map);
	void sound_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
};

#endif // MAME_INCLUDES_CAPBOWL_H
