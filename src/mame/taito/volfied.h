// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Nicola Salmoria
/*************************************************************************

    Volfied

*************************************************************************/
#ifndef MAME_INCLUDES_VOLFIED_H
#define MAME_INCLUDES_VOLFIED_H

#pragma once

#include "taitocchip.h"
#include "pc090oj.h"
#include "screen.h"
#include "machine/timer.h"

class volfied_state : public driver_device
{
public:
	volfied_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_cchip(*this, "cchip"),
		m_pc090oj(*this, "pc090oj"),
		m_screen(*this, "screen"),
		m_cchip_irq_clear(*this, "cchip_irq_clear")
	{ }

	void volfied(machine_config &config);

protected:
	enum
	{
		TIMER_VOLFIED
	};

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	uint16_t video_ram_r(offs_t offset);
	void video_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void video_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t video_ctrl_r();
	void video_mask_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void counters_w(uint8_t data);
	void volfied_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(cchip_irq_clear_cb);

	void refresh_pixel_layer( bitmap_ind16 &bitmap );

	void main_map(address_map &map);
	void z80_map(address_map &map);

	/* memory pointers */
	std::unique_ptr<uint16_t[]>    m_video_ram;

	/* video-related */
	uint16_t      m_video_ctrl = 0;
	uint16_t      m_video_mask = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<taito_cchip_device> m_cchip;
	required_device<pc090oj_device> m_pc090oj;
	required_device<screen_device> m_screen;

	required_device<timer_device> m_cchip_irq_clear;
};

#endif // MAME_INCLUDES_VOLFIED_H
