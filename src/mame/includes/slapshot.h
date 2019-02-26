// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    Slapshot / Operation Wolf 3

*************************************************************************/
#ifndef MAME_INCLUDES_SLAPSHOT_H
#define MAME_INCLUDES_SLAPSHOT_H

#pragma once

#include "audio/taitosnd.h"
#include "machine/taitoio.h"
#include "video/tc0360pri.h"
#include "video/tc0480scp.h"
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
		m_spriteext(*this,"spriteext")
	{ }

	void opwolf3(machine_config &config);
	void slapshot(machine_config &config);

	void init_slapshot();

protected:
	enum
	{
		TIMER_SLAPSHOT_INTERRUPT6
	};

	virtual void machine_start() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	struct slapshot_tempsprite
	{
		int gfx;
		int code,color;
		int flipx,flipy;
		int x,y;
		int zoomx,zoomy;
		int primask;
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
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_spriteext;
	std::unique_ptr<uint16_t[]>    m_spriteram_buffered;
	std::unique_ptr<uint16_t[]>    m_spriteram_delayed;

	/* video-related */
	slapshot_tempsprite *m_spritelist;
	int32_t       m_sprites_disabled;
	int32_t       m_sprites_active_area;
	int32_t       m_sprites_master_scrollx;
	int32_t       m_sprites_master_scrolly;
	int         m_sprites_flipscreen;
	int         m_prepare_sprites;
	int         m_dislayer[5];

	emu_timer *m_int6_timer;

	// generic
	DECLARE_READ16_MEMBER(service_input_r);
	void sound_bankswitch_w(u8 data);
	void coin_control_w(u8 data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_taito_no_buffer);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int *primasks, int y_offset );
	void taito_handle_sprite_buffering();
	void taito_update_sprites_active_area();

	INTERRUPT_GEN_MEMBER(interrupt);

	void opwolf3_map(address_map &map);
	void opwolf3_z80_sound_map(address_map &map);
	void slapshot_map(address_map &map);
};

#endif // MAME_INCLUDES_SLAPSHOT_H
