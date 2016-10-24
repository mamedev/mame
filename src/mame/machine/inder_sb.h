// license:BSD-3-Clause
// copyright-holders:David Haywood
/* */


#pragma once

#ifndef __INDER_AUDIO__
#define __INDER_AUDIO__

#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "cpu/z80/z80daisy.h"
#include "sound/dac.h"


extern const device_type INDER_AUDIO;

#define MCFG_INDER_AUDIO_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, INDER_AUDIO, 0)


class inder_sb_device :  public device_t,
							public device_mixer_interface
{
public:
	// construction/destruction
	inder_sb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<cpu_device> m_audiocpu;
	required_device<z80ctc_device> m_ctc;

	uint8_t megaphx_sound_sent_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t megaphx_sound_cmd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void megaphx_sound_to_68k_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void dac0_rombank_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dac1_rombank_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dac2_rombank_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dac3_rombank_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void z80ctc_ch0(int state);
	void z80ctc_ch1(int state);
	void z80ctc_ch2(int state);
	void z80ctc_ch3(int state);

	uint8_t megaphx_02cc_hack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t megaphx_02e6_hack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t megaphx_0309_hack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t megaphx_0323_hack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);


	uint16_t megaphx_0x050002_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void megaphx_0x050000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint8_t m_soundbank[4];

	int m_soundsent;
	uint8_t m_sounddata;
	uint8_t m_soundback;

	void install_sound_hacks(void);
	void update_sound_irqs(void);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;


	int m_soundirq;


private:


};

#endif
