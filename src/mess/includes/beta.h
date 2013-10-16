// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __BETA__
#define __BETA__


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cartslot.h"
#include "machine/6532riot.h"
#include "sound/speaker.h"
#include "machine/ram.h"

#define SCREEN_TAG      "screen"
#define M6502_TAG       "m6502"
#define M6532_TAG       "m6532"
#define EPROM_TAG       "eprom"
//#define SPEAKER_TAG       "b237"

class beta_state : public driver_device
{
public:
	beta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M6502_TAG),
			m_speaker(*this, "speaker"),
			m_eprom(*this, EPROM_TAG),
			m_q6(*this, "Q6"),
			m_q7(*this, "Q7"),
			m_q8(*this, "Q8"),
			m_q9(*this, "Q9")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_memory_region m_eprom;
	required_ioport m_q6;
	required_ioport m_q7;
	required_ioport m_q8;
	required_ioport m_q9;

	virtual void machine_start();

	DECLARE_READ8_MEMBER( riot_pa_r );
	DECLARE_WRITE8_MEMBER( riot_pa_w );
	DECLARE_READ8_MEMBER( riot_pb_r );
	DECLARE_WRITE8_MEMBER( riot_pb_w );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );

	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER( beta_eprom );

	/* EPROM state */
	int m_eprom_oe;
	int m_eprom_ce;
	UINT16 m_eprom_addr;
	UINT8 m_eprom_data;
	UINT8 m_old_data;

	/* display state */
	UINT8 m_ls145_p;
	UINT8 m_segment;

	emu_timer *m_led_refresh_timer;
	TIMER_CALLBACK_MEMBER(led_refresh);
};

#endif
