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


class s11c_bg_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	s11c_bg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// note to keep synchronization working, the host machine should have synchronization timer expired delegates
	// before writing to the following 3 things:
	DECLARE_WRITE_LINE_MEMBER(extra_w); // external write to board CB2 (J4 pin 12), does anything actually do this?
	DECLARE_WRITE_LINE_MEMBER(ctrl_w); // external write to board CB1 (J4 pin 13)
	void data_w(uint8_t data); // external write to board data bus (J4 pins 3 thru 10 for D0-D7)
	virtual void device_reset() override; // external write to board /RESET (J4 pin 18)

	// callbacks
	auto cb2_cb() { return m_cb2_cb.bind(); }
	auto pb_cb() { return m_pb_cb.bind(); }

	template <typename T> void set_romregion(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); }

	void s11c_bg_map(address_map &map);
protected:
	// overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	TIMER_CALLBACK_MEMBER(deferred_cb2_w);
	TIMER_CALLBACK_MEMBER(deferred_pb_w);

private:
	required_device<cpu_device> m_cpu;
	required_device<ym2151_device> m_ym2151;
	required_device<hc55516_device> m_hc55516;
	required_device<pia6821_device> m_pia40;
	required_memory_bank m_cpubank;
	required_region_ptr<uint8_t> m_rom;

	devcb_write_line m_cb2_cb;
	devcb_write8 m_pb_cb;

	DECLARE_WRITE_LINE_MEMBER(pia40_cb2_w);
	void pia40_pb_w(uint8_t data);

	void bg_cvsd_clock_set_w(uint8_t data);
	void bg_cvsd_digit_clock_clear_w(uint8_t data);
	void bgbank_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(S11C_BG, s11c_bg_device)

#endif // MAME_AUDIO_S11C_BG_H
