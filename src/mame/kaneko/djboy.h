// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

    DJ Boy

*************************************************************************/
#ifndef MAME_KANEKO_DJBOY_H
#define MAME_KANEKO_DJBOY_H

#pragma once

#include "kan_pand.h"

#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"

#include "emupal.h"
#include "tilemap.h"


class djboy_state : public driver_device
{
public:
	djboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_paletteram(*this, "paletteram")
		, m_masterbank(*this, "master_bank")
		, m_slavebank(*this, "slave_bank")
		, m_soundbank(*this, "sound_bank")
		, m_masterbank_l(*this, "master_bank_l")
		, m_port_in(*this, "IN%u", 0)
		, m_port_dsw(*this, "DSW%u", 1)
		, m_mastercpu(*this, "mastercpu")
		, m_slavecpu(*this, "slavecpu")
		, m_soundcpu(*this, "soundcpu")
		, m_beast(*this, "beast")
		, m_pandora(*this, "pandora")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_slavelatch(*this, "slavelatch")
		, m_beastlatch(*this, "beastlatch")
	{
	}

	void djboy(machine_config &config);

	void init_djboy();
	void init_djboyj();

private:
	static constexpr unsigned PROT_OUTPUT_BUFFER_SIZE = 8;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_paletteram;

	/* ROM banking */
	uint8_t       m_bankxor;

	required_memory_bank m_masterbank;
	required_memory_bank m_slavebank;
	required_memory_bank m_soundbank;
	required_memory_bank m_masterbank_l;

	required_ioport_array<3> m_port_in;
	required_ioport_array<2> m_port_dsw;

	/* video-related */
	tilemap_t   *m_background = nullptr;
	uint8_t       m_videoreg = 0;
	uint8_t       m_scrollx = 0;
	uint8_t       m_scrolly = 0;

	/* Kaneko BEAST state */
	uint8_t       m_beast_p0 = 0;
	uint8_t       m_beast_p1 = 0;
	uint8_t       m_beast_p2 = 0;
	uint8_t       m_beast_p3 = 0;

	/* devices */
	required_device<cpu_device> m_mastercpu;
	required_device<cpu_device> m_slavecpu;
	required_device<cpu_device> m_soundcpu;
	required_device<i80c51_device> m_beast;
	required_device<kaneko_pandora_device> m_pandora;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_slavelatch;
	required_device<generic_latch_8_device> m_beastlatch;

	uint8_t beast_status_r();
	void trigger_nmi_on_mastercpu(uint8_t data);
	void mastercpu_bankswitch_w(uint8_t data);
	void slavecpu_bankswitch_w(uint8_t data);
	void coin_count_w(uint8_t data);
	void soundcpu_bankswitch_w(uint8_t data);
	uint8_t beast_p0_r();
	void beast_p0_w(uint8_t data);
	uint8_t beast_p1_r();
	void beast_p1_w(uint8_t data);
	uint8_t beast_p2_r();
	void beast_p2_w(uint8_t data);
	uint8_t beast_p3_r();
	void beast_p3_w(uint8_t data);
	void djboy_scrollx_w(uint8_t data);
	void djboy_scrolly_w(uint8_t data);
	void djboy_videoram_w(offs_t offset, uint8_t data);
	void djboy_paletteram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_djboy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_djboy);
	TIMER_DEVICE_CALLBACK_MEMBER(djboy_scanline);
	void mastercpu_am(address_map &map);
	void mastercpu_port_am(address_map &map);
	void slavecpu_am(address_map &map);
	void slavecpu_port_am(address_map &map);
	void soundcpu_am(address_map &map);
	void soundcpu_port_am(address_map &map);
};

#endif // MAME_KANEKO_DJBOY_H
