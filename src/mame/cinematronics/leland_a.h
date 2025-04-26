// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Cinematronics / Leland Cinemat System driver

    Leland sound hardware

*************************************************************************/
#ifndef MAME_CINEMATRONICS_LELAND_A_H
#define MAME_CINEMATRONICS_LELAND_A_H

#pragma once

#include "cpu/i86/i186.h"
#include "machine/gen_latch.h"
#include "machine/pit8253.h"
#include "sound/dac.h"
#include "sound/ymopm.h"


class leland_80186_sound_device : public device_t
{
public:
	leland_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template<class T> void set_master_cpu_tag(T &&tag) { m_master.set_tag(std::forward<T>(tag)); }

	void peripheral_ctrl(offs_t offset, u16 data);
	void leland_80186_control_w(u8 data);
	void ataxx_80186_control_w(u8 data);
	u16 peripheral_r(offs_t offset, u16 mem_mask = ~0);
	void peripheral_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void command_lo_w(u8 data);
	void command_hi_w(u8 data);
	u8 response_r();
	void dac_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void ataxx_dac_control(offs_t offset, u16 data, u16 mem_mask = ~0);
	void i80186_tmr0_w(int state);
	void i80186_tmr1_w(int state);

	void pit0_2_w(int state);
	void pit1_0_w(int state);
	void pit1_1_w(int state);
	void pit1_2_w(int state);

protected:
	leland_80186_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	int m_type;

	enum
	{
		TYPE_LELAND,
		TYPE_REDLINE,
		TYPE_ATAXX,
		TYPE_WSF
	};

	required_device<generic_latch_16_device> m_soundlatch;
	optional_device_array<dac_byte_interface, 8> m_dac;
	optional_device<dac_word_interface> m_dac9;
	optional_device_array<dac_8bit_binary_weighted_device, 8> m_dacvol;
	optional_device_array<pit8254_device, 3> m_pit;
	optional_device<i80186_cpu_device> m_audiocpu;
	optional_device<ym2151_device> m_ymsnd;

	void ataxx_80186_map_io(address_map &map) ATTR_COLD;
	void leland_80186_map_io(address_map &map) ATTR_COLD;
	void leland_80186_map_program(address_map &map) ATTR_COLD;

private:
	void set_clock_line(int which, int state) { m_clock_active = state ? (m_clock_active | (1<<which)) : (m_clock_active & ~(1<<which)); }

	// internal state
	u16 m_peripheral;
	u8 m_last_control;
	u8 m_clock_active;
	u8 m_clock_tick;
	u16 m_sound_command;
	u16 m_sound_response;
	bool m_response_sync;
	u32 m_ext_start;
	u32 m_ext_stop;
	u8 m_ext_active;

	required_device<cpu_device> m_master;

	optional_region_ptr<u8> m_ext_base;
};


class redline_80186_sound_device : public leland_80186_sound_device
{
public:
	redline_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	void redline_dac_w(offs_t offset, u16 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
private:
	void redline_80186_map_io(address_map &map) ATTR_COLD;
};


class ataxx_80186_sound_device : public leland_80186_sound_device
{
public:
	ataxx_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class wsf_80186_sound_device : public leland_80186_sound_device
{
public:
	wsf_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(LELAND_80186, leland_80186_sound_device)
DECLARE_DEVICE_TYPE(REDLINE_80186, redline_80186_sound_device)
DECLARE_DEVICE_TYPE(ATAXX_80186, ataxx_80186_sound_device)
DECLARE_DEVICE_TYPE(WSF_80186, wsf_80186_sound_device)

#endif // MAME_CINEMATRONICS_LELAND_A_H
