// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, Jonathan Gevaryahu
/*
 * s11c_bg.h - Williams System 11C background sound (M68B09E + YM2151 + HC55516 + DAC)
 *
 *  Created on: 2/10/2013
 */

#ifndef MAME_SHARED_S11C_BG_H
#define MAME_SHARED_S11C_BG_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/rescap.h"
#include "sound/dac.h"
#include "sound/flt_biquad.h"
#include "sound/hc55516.h"
#include "sound/ymopm.h"


class s11c_bg_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	s11c_bg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// base config
	void s11_bg_base(machine_config &config);
	// add ym2151
	void s11_bg_ym(machine_config &config);
	// add cvsd
	void s11_bg_cvsd(machine_config &config);

	// note to keep synchronization working, the host machine should have synchronization timer expired delegates
	// before writing to the following 3 things:
	void extra_w(int state); // external write to board CB2 (J4 pin 12), does anything actually do this?
	void ctrl_w(int state); // external write to board CB1 (J4 pin 13)
	void data_w(uint8_t data); // external write to board data bus (J4 pins 3 thru 10 for D0-D7)
	virtual void device_reset() override ATTR_COLD; // power up reset
	void resetq_w(int state); // external write to board /RESET (J4 pin 18)

	// callbacks
	auto cb2_cb() { return m_cb2_cb.bind(); }
	auto pb_cb() { return m_pb_cb.bind(); }

	void s11c_bg_map(address_map &map) ATTR_COLD;
	void s11c_bgm_map(address_map &map) ATTR_COLD;
	void s11c_bgs_map(address_map &map) ATTR_COLD;

	//mc6809e_device *get_cpu() { return m_cpu; }
protected:
	// constructor with overridable type for subclass
	s11c_bg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(deferred_cb2_w);
	TIMER_CALLBACK_MEMBER(deferred_pb_w);

	required_device<mc6809e_device> m_cpu;
	required_device<mc1408_device> m_dac;
	optional_device<ym2151_device> m_ym2151;
	optional_device<hc55516_device> m_cvsd;
	optional_device<filter_biquad_device> m_cvsd_filter;
	optional_device<filter_biquad_device> m_cvsd_filter2;
	required_device<pia6821_device> m_pia40;
	required_memory_bank m_cpubank;

private:
	devcb_write_line m_cb2_cb;
	devcb_write8 m_pb_cb;

	void common_reset(); // common reset function used by both internal and external reset
	uint8_t m_old_resetq_state;
	void pia40_cb2_w(int state);
	void pia40_pb_w(uint8_t data);

	void bg_cvsd_clock_set_w(uint8_t data);
	void bg_cvsd_digit_clock_clear_w(uint8_t data);
	void bgbank_w(uint8_t data);
};

class s11_bg_device : public s11c_bg_device
{
public:
	s11_bg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class s11_obg_device : public s11c_bg_device
{
public:
	s11_obg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class s11_bgm_device : public s11c_bg_device
{
public:
	s11_bgm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class s11_bgs_device : public s11c_bg_device
{
public:
	s11_bgs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(S11C_BG, s11c_bg_device)
DECLARE_DEVICE_TYPE(S11_BG, s11_bg_device)
DECLARE_DEVICE_TYPE(S11_OBG, s11_obg_device)
DECLARE_DEVICE_TYPE(S11_BGM, s11_bgm_device)
DECLARE_DEVICE_TYPE(S11_BGS, s11_bgs_device)

#endif // MAME_SHARED_S11C_BG_H
