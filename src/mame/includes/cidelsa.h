// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __CIDELSA__
#define __CIDELSA__


#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "cpu/cop400/cop400.h"
#include "sound/cdp1869.h"
#include "sound/ay8910.h"
#include "machine/cdp1852.h"
#include "machine/nvram.h"

#define SCREEN_TAG  "screen"
#define CDP1802_TAG "cdp1802"
#define CDP1869_TAG "cdp1869"
#define COP402N_TAG "cop402n"
#define AY8910_TAG  "ay8910"

#define DESTRYER_CHR1   3579000.0 // unverified
#define DESTRYER_CHR2   XTAL_5_7143MHz
#define ALTAIR_CHR1     3579000.0 // unverified
#define ALTAIR_CHR2     CDP1869_DOT_CLK_PAL // unverified
#define DRACO_CHR1      XTAL_4_43361MHz
#define DRACO_CHR2      CDP1869_DOT_CLK_PAL // unverified
#define DRACO_SND_CHR1  XTAL_2_01216MHz

#define CIDELSA_PAGERAM_SIZE    0x400
#define DRACO_PAGERAM_SIZE      0x800
#define CIDELSA_CHARRAM_SIZE    0x800

#define CIDELSA_PAGERAM_MASK    0x3ff
#define DRACO_PAGERAM_MASK      0x7ff
#define CIDELSA_CHARRAM_MASK    0x7ff

class cidelsa_state : public driver_device
{
public:
	enum
	{
		TIMER_SET_CPU_MODE
	};

	cidelsa_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, CDP1802_TAG),
			m_vis(*this, CDP1869_TAG)
	{ }

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1869_device> m_vis;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	DECLARE_WRITE8_MEMBER( cdp1869_w );
	DECLARE_WRITE8_MEMBER( destryer_out1_w );
	DECLARE_WRITE8_MEMBER( altair_out1_w );

	DECLARE_READ_LINE_MEMBER( clear_r );

	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_WRITE_LINE_MEMBER( prd_w );
	DECLARE_READ_LINE_MEMBER( cdp1869_pcb_r );

	CDP1869_CHAR_RAM_READ_MEMBER(cidelsa_charram_r);
	CDP1869_CHAR_RAM_WRITE_MEMBER(cidelsa_charram_w);
	CDP1869_PCB_READ_MEMBER(cidelsa_pcb_r);

	// cpu state
	int m_reset;

	// video state
	int m_cdp1802_q;
	int m_cdp1869_pcb;

	UINT8 *m_pageram;
	UINT8 *m_pcbram;
	UINT8 *m_charram;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

class draco_state : public cidelsa_state
{
public:
	draco_state(const machine_config &mconfig, device_type type, const char *tag)
		: cidelsa_state(mconfig, type, tag),
			m_psg(*this, AY8910_TAG)
	{ }

	required_device<ay8910_device> m_psg;

	virtual void machine_start() override;

	DECLARE_READ8_MEMBER( sound_in_r );
	DECLARE_READ8_MEMBER( psg_r );
	DECLARE_WRITE8_MEMBER( sound_bankswitch_w );
	DECLARE_WRITE8_MEMBER( sound_g_w );
	DECLARE_WRITE8_MEMBER( psg_w );
	DECLARE_WRITE8_MEMBER( out1_w );
	DECLARE_WRITE8_MEMBER( psg_pb_w );

	CDP1869_CHAR_RAM_READ_MEMBER(draco_charram_r);
	CDP1869_CHAR_RAM_WRITE_MEMBER(draco_charram_w);
	CDP1869_PCB_READ_MEMBER(draco_pcb_r);

	// sound state
	int m_sound;
	int m_psg_latch;
};

/*----------- defined in video/cidelsa.c -----------*/

MACHINE_CONFIG_EXTERN( destryer_video );
MACHINE_CONFIG_EXTERN( altair_video );
MACHINE_CONFIG_EXTERN( draco_video );

#endif
