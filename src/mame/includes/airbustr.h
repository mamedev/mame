// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    Air Buster

*************************************************************************/
#ifndef MAME_INCLUDES_AIRBUSTR_H
#define MAME_INCLUDES_AIRBUSTR_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "video/kan_pand.h"
#include "emupal.h"
#include "screen.h"

class airbustr_state : public driver_device
{
public:
	airbustr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_devram(*this, "devram")
		, m_videoram(*this, "videoram%u", 1)
		, m_colorram(*this, "colorram%u", 1)
		, m_masterbank(*this, "masterbank")
		, m_slavebank(*this, "slavebank")
		, m_audiobank(*this, "audiobank")
		, m_master(*this, "master")
		, m_slave(*this, "slave")
		, m_audiocpu(*this, "audiocpu")
		, m_pandora(*this, "pandora")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_watchdog(*this, "watchdog")
		, m_soundlatch(*this, "soundlatch%u", 1)
	{
	}

	/* memory pointers */
	required_shared_ptr<uint8_t> m_devram;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_colorram;

	required_memory_bank m_masterbank;
	required_memory_bank m_slavebank;
	required_memory_bank m_audiobank;

	/* video-related */
	tilemap_t    *m_tilemap[2];
	int        m_scrollx[2];
	int        m_scrolly[2];
	int        m_highbits;

	/* devices */
	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_device<cpu_device> m_audiocpu;
	required_device<kaneko_pandora_device> m_pandora;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<watchdog_timer_device> m_watchdog;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;

	DECLARE_READ8_MEMBER(devram_r);
	DECLARE_WRITE8_MEMBER(master_nmi_trigger_w);
	DECLARE_WRITE8_MEMBER(master_bankswitch_w);
	DECLARE_WRITE8_MEMBER(slave_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_READ8_MEMBER(soundcommand_status_r);
	DECLARE_WRITE8_MEMBER(coin_counter_w);
	template<int Layer> DECLARE_WRITE8_MEMBER(videoram_w);
	template<int Layer> DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(scrollregs_w);
	void init_airbustr();
	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	INTERRUPT_GEN_MEMBER(slave_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(airbustr_scanline);
	void airbustr(machine_config &config);
	void airbustrb(machine_config &config);
	void master_io_map(address_map &map);
	void master_map(address_map &map);
	void slave_io_map(address_map &map);
	void slave_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_AIRBUSTR_H
