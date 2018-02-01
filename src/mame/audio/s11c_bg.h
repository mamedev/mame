// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * s11c_bg.h - Williams System 11C background sound (M68B09E + YM2151 + HC55516 + DAC)
 *
 *  Created on: 2/10/2013
 */

#ifndef MAME_AUDIO_S11C_BG_H
#define MAME_AUDIO_S11C_BG_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "sound/hc55516.h"
#include "sound/ym2151.h"


#define MCFG_S11C_BG_ROM_REGION(_region) \
	s11c_bg_device::static_set_romregion(*device, _region);


class s11c_bg_device : public device_t,
	public device_mixer_interface
{
public:
	// construction/destruction
	s11c_bg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<cpu_device> m_cpu;
	required_device<ym2151_device> m_ym2151;
	required_device<hc55516_device> m_hc55516;
	required_device<pia6821_device> m_pia40;
	required_memory_bank m_cpubank;
	memory_region* m_rom;


	DECLARE_WRITE8_MEMBER(bg_speech_clock_w);
	DECLARE_WRITE8_MEMBER(bg_speech_digit_w);
	DECLARE_WRITE8_MEMBER(bgbank_w);
	void ctrl_w(uint8_t data);
	void data_w(uint8_t data);

	static void static_set_romregion(device_t &device, const char *tag);

	void s11c_bg_map(address_map &map);
protected:
	// overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	const char* m_regiontag;

	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_WRITE8_MEMBER(pia40_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia40_cb2_w);
};

DECLARE_DEVICE_TYPE(S11C_BG, s11c_bg_device)

#endif // MAME_AUDIO_S11C_BG_H
