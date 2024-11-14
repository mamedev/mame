// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    Slapshot / Operation Wolf 3

*************************************************************************/
#ifndef MAME_TAITO_SLAPSHOT_H
#define MAME_TAITO_SLAPSHOT_H

#pragma once

#include "taitosnd.h"
#include "taitoio.h"
#include "tc0360pri.h"
#include "tc0480scp.h"
#include "emupal.h"

class slapshot_state : public driver_device
{
public:
	slapshot_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tc0140syt(*this, "tc0140syt"),
		m_tc0480scp(*this, "tc0480scp"),
		m_tc0360pri(*this, "tc0360pri"),
		m_tc0640fio(*this, "tc0640fio"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this,"spriteram"),
		m_spriteext(*this,"spriteext"),
		m_z80bank(*this,"z80bank"),
		m_io_system(*this,"SYSTEM"),
		m_io_service(*this,"SERVICE")
	{ }

	void opwolf3(machine_config &config);
	void slapshot(machine_config &config);

	void driver_init();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(trigger_int6);

private:
	struct slapshot_tempsprite
	{
		u8 gfx = 0;
		u32 code = 0;
		u32 color = 0;
		bool flipx = false;
		bool flipy = false;
		int x = 0;
		int y = 0;
		int zoomx = 0;
		int zoomy = 0;
		u32 primask = 0;
	};

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<tc0360pri_device> m_tc0360pri;
	required_device<tc0640fio_device> m_tc0640fio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_spriteext;
	std::unique_ptr<u16[]>   m_spriteram_buffered;
	std::unique_ptr<u16[]>   m_spriteram_delayed;

	required_memory_bank m_z80bank;
	optional_ioport m_io_system;
	optional_ioport m_io_service;

	/* video-related */
	std::unique_ptr<slapshot_tempsprite[]> m_spritelist;
	bool      m_sprites_disabled = false;
	s32       m_sprites_active_area = 0;
	s32       m_sprites_master_scrollx = 0;
	s32       m_sprites_master_scrolly = 0;
	bool      m_sprites_flipscreen = false;
	bool      m_prepare_sprites = false;
#ifdef MAME_DEBUG
	int       m_dislayer[5] = { 0, 0, 0, 0, 0 };
#endif

	emu_timer *m_int6_timer = nullptr;
	std::unique_ptr<u8[]> m_decoded_gfx;

	// generic
	u16 service_input_r(offs_t offset);
	void sound_bankswitch_w(u8 data);
	void coin_control_w(u8 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_no_buffer(int state);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u32 *primasks, int y_offset);
	void handle_sprite_buffering();
	void update_sprites_active_area();

	INTERRUPT_GEN_MEMBER(interrupt);

	void opwolf3_map(address_map &map) ATTR_COLD;
	void slapshot_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TAITO_SLAPSHOT_H
