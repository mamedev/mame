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
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "hd66421")
		, m_speaker(*this, "speaker")
		, m_rtc(*this, "rtc")
		, m_ram(*this, RAM_TAG)
		, m_flash1(*this, "flash1")
		, m_nvram(*this, "nvram")
		, m_input(*this, "A.%u", 0)
	{ }

	void serflash_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t clock_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void clock_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t xtclock_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void xtclock_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t xtpower_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t adc1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t adc2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t port0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	uint16_t cybiko_lcd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cybiko_lcd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t cybikov1_key_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t cybikov2_key_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t cybikoxt_key_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cybiko_usb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	int cybiko_key_r( offs_t offset, int mem_mask);

	required_device<cpu_device> m_maincpu;
	required_device<hd66421_device> m_crtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<pcf8593_device> m_rtc;
	required_device<ram_device> m_ram;
	optional_device<at45db041_device> m_flash1;
	required_device<nvram_device>   m_nvram;
	optional_ioport_array<15> m_input;
	void init_cybikoxt();
	void init_cybiko();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_QUICKLOAD_LOAD_MEMBER( cybiko );
	DECLARE_QUICKLOAD_LOAD_MEMBER( cybikoxt );
};

#endif /* CYBIKO_H_ */
