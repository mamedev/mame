// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Stefan Jokisch
/*************************************************************************

    Atari Ultra Tank hardware

*************************************************************************/
#ifndef MAME_INCLUDES_ULTRATNK_H
#define MAME_INCLUDES_ULTRATNK_H

#pragma once

#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class ultratnk_state : public driver_device
{
public:
	ultratnk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_joy(*this, "JOY-%c", 'W')
	{ }

	template <int N> DECLARE_READ_LINE_MEMBER(collision_flipflop_r);
	template <int N> DECLARE_READ_LINE_MEMBER(joystick_r);
	void ultratnk(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	enum
	{
		TIMER_NMI
	};

	uint8_t wram_r(offs_t offset);
	uint8_t analog_r(offs_t offset);
	uint8_t coin_r(offs_t offset);
	uint8_t collision_r(offs_t offset);
	uint8_t options_r(offs_t offset);
	void wram_w(offs_t offset, uint8_t data);
	void collision_reset_w(offs_t offset, uint8_t data);
	void da_latch_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(lockout_w);
	void video_ram_w(offs_t offset, uint8_t data);
	void attract_w(uint8_t data);
	void explosion_w(uint8_t data);

	void ultratnk_palette(palette_device &palette) const;

	TILE_GET_INFO_MEMBER(tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	TIMER_CALLBACK_MEMBER(nmi_callback);

	void ultratnk_cpu_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;

	required_ioport_array<4> m_joy;

	int m_da_latch = 0;
	int m_collision[4]{};
	tilemap_t* m_playfield = nullptr;
	bitmap_ind16 m_helper;
	emu_timer *m_nmi_timer = nullptr;
};

#endif // MAME_INCLUDES_ULTRATNK_H
