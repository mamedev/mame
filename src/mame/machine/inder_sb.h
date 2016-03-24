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
	inder_sb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	required_device<cpu_device> m_audiocpu;
	required_device<z80ctc_device> m_ctc;
	required_device<dac_device> m_dac0;
	required_device<dac_device> m_dac1;
	required_device<dac_device> m_dac2;
	required_device<dac_device> m_dac3;

	DECLARE_READ8_MEMBER(megaphx_sound_sent_r);
	DECLARE_READ8_MEMBER(megaphx_sound_cmd_r);
	DECLARE_WRITE8_MEMBER(megaphx_sound_to_68k_w);


	DECLARE_WRITE8_MEMBER(dac0_value_write);
	DECLARE_WRITE8_MEMBER(dac0_gain_write);
	DECLARE_WRITE8_MEMBER(dac1_value_write);
	DECLARE_WRITE8_MEMBER(dac1_gain_write);
	DECLARE_WRITE8_MEMBER(dac2_value_write);
	DECLARE_WRITE8_MEMBER(dac2_gain_write);
	DECLARE_WRITE8_MEMBER(dac3_value_write);
	DECLARE_WRITE8_MEMBER(dac3_gain_write);

	DECLARE_WRITE8_MEMBER(dac0_rombank_write);
	DECLARE_WRITE8_MEMBER(dac1_rombank_write);
	DECLARE_WRITE8_MEMBER(dac2_rombank_write);
	DECLARE_WRITE8_MEMBER(dac3_rombank_write);

	DECLARE_WRITE_LINE_MEMBER(z80ctc_ch0);
	DECLARE_WRITE_LINE_MEMBER(z80ctc_ch1);
	DECLARE_WRITE_LINE_MEMBER(z80ctc_ch2);
	DECLARE_WRITE_LINE_MEMBER(z80ctc_ch3);

	DECLARE_READ8_MEMBER(megaphx_02cc_hack_r);
	DECLARE_READ8_MEMBER(megaphx_02e6_hack_r);
	DECLARE_READ8_MEMBER(megaphx_0309_hack_r);
	DECLARE_READ8_MEMBER(megaphx_0323_hack_r);


	DECLARE_READ16_MEMBER(megaphx_0x050002_r);
	DECLARE_WRITE16_MEMBER(megaphx_0x050000_w);

	UINT8 dac_gain[4];
	UINT8 m_soundbank[4];

	int m_soundsent;
	UINT8 m_sounddata;
	UINT8 m_soundback;

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
