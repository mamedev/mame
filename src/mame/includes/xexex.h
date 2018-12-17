// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    Xexex

*************************************************************************/
#ifndef MAME_INCLUDES_XEXEX_H
#define MAME_INCLUDES_XEXEX_H

#pragma once

#include "video/k053250.h"
#include "sound/flt_vol.h"
#include "sound/k054539.h"
#include "machine/k053252.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k054338.h"
#include "video/k053251.h"
#include "video/konami_helper.h"
#include "machine/k054321.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"

class xexex_state : public driver_device
{
public:
	xexex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_workram(*this, "workram")
		, m_spriteram(*this, "spriteram")
		, m_z80bank(*this, "z80bank")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_k054539(*this, "k054539")
		, m_filter_l(*this, "filter%u_l", 1)
		, m_filter_r(*this, "filter%u_r", 1)
		, m_k056832(*this, "k056832")
		, m_k053246(*this, "k053246")
		, m_k053250(*this, "k053250")
		, m_k053251(*this, "k053251")
		, m_k053252(*this, "k053252")
		, m_k054338(*this, "k054338")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_k054321(*this, "k054321")
	{
	}

	void xexex(machine_config &config);

	void init_xexex();

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_workram;
	required_shared_ptr<uint16_t> m_spriteram;

	/* memory regions */
	required_memory_bank m_z80bank;

	/* video-related */
	int        m_layer_colorbase[4];
	int        m_sprite_colorbase;
	int        m_layerpri[4];
	int        m_cur_alpha;

	/* misc */
	uint16_t     m_cur_control2;
	int32_t      m_strip_0x1a;
	int        m_suspension_active;
	int        m_resume_trigger;
	emu_timer  *m_dmadelay_timer;
	int        m_frame;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054539_device> m_k054539;
	required_device_array<filter_volume_device, 2> m_filter_l;
	required_device_array<filter_volume_device, 2> m_filter_r;
	required_device<k056832_device> m_k056832;
	required_device<k053247_device> m_k053246;
	required_device<k053250_device> m_k053250;
	required_device<k053251_device> m_k053251;
	required_device<k053252_device> m_k053252;
	required_device<k054338_device> m_k054338;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<k054321_device> m_k054321;

	DECLARE_READ16_MEMBER(spriteram_mirror_r);
	DECLARE_WRITE16_MEMBER(spriteram_mirror_w);
	DECLARE_READ16_MEMBER(xexex_waitskip_r);
	DECLARE_READ16_MEMBER(control2_r);
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_xexex(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(dmaend_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(xexex_interrupt);
	void xexex_postload();
	void xexex_objdma(int limiter);
	void parse_control2();
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
	K054539_CB_MEMBER(ym_set_mixing);

	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_XEXEX_H
