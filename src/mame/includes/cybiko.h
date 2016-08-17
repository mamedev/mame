// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*****************************************************************************
 *
 * includes/cybiko.h
 *
 * Cybiko Wireless Inter-tainment System
 *
 * (c) 2001-2007 Tim Schuerewegen
 *
 * Cybiko Classic (V1)
 * Cybiko Classic (V2)
 * Cybiko Xtreme
 *
 ****************************************************************************/

#ifndef CYBIKO_H_
#define CYBIKO_H_

/* Core includes */
#include "emu.h"
#include "sound/speaker.h"

/* Components */
#include "cpu/h8/h8s2320.h"
#include "cpu/h8/h8s2245.h"
#include "video/hd66421.h"
#include "machine/pcf8593.h"
#include "machine/at45dbxx.h"
#include "machine/intelfsh.h"
#include "machine/ram.h"
#include "imagedev/snapquik.h"
#include "machine/nvram.h"

class cybiko_state : public driver_device
{
public:
	cybiko_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "hd66421"),
		m_speaker(*this, "speaker"),
		m_rtc(*this, "rtc"),
		m_ram(*this, RAM_TAG),
		m_flash1(*this, "flash1"),
		m_nvram(*this, "nvram"),
		m_input(*this, "A")
	{ }

	DECLARE_WRITE16_MEMBER(serflash_w);
	DECLARE_READ16_MEMBER(clock_r);
	DECLARE_WRITE16_MEMBER(clock_w);
	DECLARE_READ16_MEMBER(xtclock_r);
	DECLARE_WRITE16_MEMBER(xtclock_w);
	DECLARE_READ16_MEMBER(xtpower_r);
	DECLARE_READ16_MEMBER(adc1_r);
	DECLARE_READ16_MEMBER(adc2_r);
	DECLARE_READ16_MEMBER(port0_r);

	DECLARE_READ16_MEMBER(cybiko_lcd_r);
	DECLARE_WRITE16_MEMBER(cybiko_lcd_w);
	DECLARE_READ16_MEMBER(cybikov1_key_r);
	DECLARE_READ16_MEMBER(cybikov2_key_r);
	DECLARE_READ16_MEMBER(cybikoxt_key_r);
	DECLARE_WRITE16_MEMBER(cybiko_usb_w);
	int cybiko_key_r( offs_t offset, int mem_mask);

	required_device<cpu_device> m_maincpu;
	required_device<hd66421_device> m_crtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<pcf8593_device> m_rtc;
	required_device<ram_device> m_ram;
	optional_device<at45db041_device> m_flash1;
	required_device<nvram_device>   m_nvram;
	optional_ioport_array<15> m_input;
	DECLARE_DRIVER_INIT(cybikoxt);
	DECLARE_DRIVER_INIT(cybiko);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_QUICKLOAD_LOAD_MEMBER( cybiko );
	DECLARE_QUICKLOAD_LOAD_MEMBER( cybikoxt );
};

#endif /* CYBIKO_H_ */
