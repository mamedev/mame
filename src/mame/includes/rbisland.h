// license:BSD-3-Clause
// copyright-holders:Mike Coates
/*************************************************************************

    Rainbow Islands

*************************************************************************/
#ifndef MAME_INCLUDES_RBISLAND_H
#define MAME_INCLUDES_RBISLAND_H

#pragma once


#include "machine/taitocchip.h"

#include "video/pc080sn.h"
#include "video/pc090oj.h"


class rbisland_state : public driver_device
{
public:
	rbisland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_cchip(*this, "cchip"),
		m_pc080sn(*this, "pc080sn"),
		m_pc090oj(*this, "pc090oj"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	uint16_t      m_sprite_ctrl;
	uint16_t      m_sprites_flipscreen;

	/* misc */
	uint8_t       m_jumping_latch;

	/* c-chip */
	std::unique_ptr<uint8_t[]>    m_CRAM[8];
	int         m_extra_version;
	uint8_t       m_current_bank;
	emu_timer *m_cchip_timer;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito_cchip_device> m_cchip;
	required_device<pc080sn_device> m_pc080sn;
	optional_device<pc090oj_device> m_pc090oj;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE16_MEMBER(jumping_sound_w);
	DECLARE_READ8_MEMBER(jumping_latch_r);
	DECLARE_WRITE16_MEMBER(rbisland_cchip_ctrl_w);
	DECLARE_WRITE16_MEMBER(rbisland_cchip_bank_w);
	DECLARE_WRITE16_MEMBER(rbisland_cchip_ram_w);
	DECLARE_READ16_MEMBER(rbisland_cchip_ctrl_r);
	DECLARE_READ16_MEMBER(rbisland_cchip_ram_r);
	DECLARE_WRITE16_MEMBER(rbisland_spritectrl_w);
	DECLARE_WRITE16_MEMBER(jumping_spritectrl_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_DRIVER_INIT(jumping);
	DECLARE_DRIVER_INIT(rbislande);
	DECLARE_DRIVER_INIT(rbisland);
	virtual void machine_start() override;
	DECLARE_VIDEO_START(jumping);
	uint32_t screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jumping(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(cchip_timer);
	void request_round_data(  );
	void request_world_data(  );
	void request_goalin_data(  );
	void rbisland_cchip_init( int version );
	void jumping(machine_config &config);
	void rbisland(machine_config &config);
	void jumpingi(machine_config &config);
	void jumping_map(address_map &map);
	void jumping_sound_map(address_map &map);
	void rbisland_map(address_map &map);
	void rbisland_sound_map(address_map &map);
};


#endif // MAME_INCLUDES_RBISLAND_H
