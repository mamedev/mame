// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Nicola Salmoria
/*************************************************************************

    Volfied

*************************************************************************/
#ifndef MAME_INCLUDES_VOLFIED_H
#define MAME_INCLUDES_VOLFIED_H

#pragma once

#include "machine/taitocchip.h"
#include "video/pc090oj.h"
#include "screen.h"

class volfied_state : public driver_device
{
public:
	enum
	{
		TIMER_VOLFIED
	};

	volfied_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_cchip(*this, "cchip"),
		m_pc090oj(*this, "pc090oj"),
		m_screen(*this, "screen") { }

	/* memory pointers */
	std::unique_ptr<uint16_t[]>    m_video_ram;
	std::unique_ptr<uint8_t[]>    m_cchip_ram;

	/* video-related */
	uint16_t      m_video_ctrl;
	uint16_t      m_video_mask;

	/* c-chip */
	uint8_t       m_current_bank;
	uint8_t       m_current_flag;
	uint8_t       m_cc_port;
	uint8_t       m_current_cmd;
	emu_timer     *m_cchip_timer;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<taito_cchip_device> m_cchip;
	required_device<pc090oj_device> m_pc090oj;
	required_device<screen_device> m_screen;

	DECLARE_WRITE16_MEMBER(cchip_ctrl_w);
	DECLARE_WRITE16_MEMBER(cchip_bank_w);
	DECLARE_WRITE16_MEMBER(cchip_ram_w);
	DECLARE_READ16_MEMBER(cchip_ctrl_r);
	DECLARE_READ16_MEMBER(cchip_ram_r);
	DECLARE_READ16_MEMBER(video_ram_r);
	DECLARE_WRITE16_MEMBER(video_ram_w);
	DECLARE_WRITE16_MEMBER(video_ctrl_w);
	DECLARE_READ16_MEMBER(video_ctrl_r);
	DECLARE_WRITE16_MEMBER(video_mask_w);
	DECLARE_WRITE16_MEMBER(sprite_ctrl_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(timer_callback);
	void refresh_pixel_layer( bitmap_ind16 &bitmap );
	void cchip_init();
	void cchip_reset();

	void volfied(machine_config &config);
	void main_map(address_map &map);
	void z80_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_VOLFIED_H
