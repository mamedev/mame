// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Data East Pinball CPU boards
 *
 *  Created on: 10/07/2013
 */

#ifndef MAME_PINBALL_DECOPINCPU_H
#define MAME_PINBALL_DECOPINCPU_H

#pragma once

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/nvram.h"

// 6808 CPU's input clock is 4MHz
// but because it has an internal /4 divider, its E clock runs at 1/4 that frequency
#define E_CLOCK (XTAL(4'000'000)/4)

// Length of time in cycles between IRQs on the main 6808 CPU
// This length is determined by the settings of the W14 and W15 jumpers
// It can be 0x300, 0x380, 0x700 or 0x780 cycles long.
// IRQ length is always 32 cycles
#define S11_IRQ_CYCLES 0x380

class decocpu_type1_device : public device_t
{
public:
	template <typename T>
	decocpu_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpuregion_tag)
		: decocpu_type1_device(mconfig, tag, owner, clock)
	{
		set_cpuregion(std::forward<T>(cpuregion_tag));
	}

	decocpu_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callbacks
	auto display_read_callback() { return m_read_display.bind(); }
	auto display_write_callback() { return m_write_display.bind(); }
	auto dmdstatus_read_callback() { return m_read_dmdstatus.bind(); }
	auto soundlatch_write_callback() { return m_write_soundlatch.bind(); }
	auto switch_read_callback() { return m_read_switch.bind(); }
	auto switch_write_callback() { return m_write_switch.bind(); }
	auto lamp_write_callback() { return m_write_lamp.bind(); }
	auto solenoid_write_callback() { return m_write_solenoid.bind(); }

	INPUT_CHANGED_MEMBER(main_nmi);

	template <typename T> void set_cpuregion(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); } // region for cpu board code and data

protected:
	void solenoid0_w(u8 data);

	decocpu_type1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_trigger);

	void decocpu1_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_cpu;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia2c;
	required_device<pia6821_device> m_pia30;
	required_device<pia6821_device> m_pia34;
	required_region_ptr<u8> m_rom;
	required_ioport m_diags;

private:
	emu_timer* m_irq_timer = 0;
	bool m_irq_active = 0;
	u8 m_lamp_data = 0U;

	// callbacks
	devcb_read8 m_read_display;
	devcb_write8 m_write_display;
	devcb_read8 m_read_dmdstatus;
	devcb_write8 m_write_soundlatch;
	devcb_read8 m_read_switch;
	devcb_write8 m_write_switch;
	devcb_write8 m_write_lamp;
	devcb_write8 m_write_solenoid;
	output_finder<86> m_io_outputs; // 22 solenoids + 64 lamps

	void cpu_pia_irq(int state);
	void pia21_cb2_w(int state) { }   // flipper enable
	void pia24_ca2_w(int state) { m_io_outputs[18] = state; }
	void pia24_cb2_w(int state) { m_io_outputs[20] = state; }
	void pia2c_ca2_w(int state) { m_io_outputs[21] = state; }
	void pia2c_cb2_w(int state) { m_io_outputs[17] = state; }
	void pia30_ca2_w(int state) { m_io_outputs[19] = state; }
	void pia30_cb2_w(int state) { m_io_outputs[16] = state; }
	void lamp0_w(u8 data);
	void lamp1_w(u8 data);
	u8 display_strobe_r();
	void display_strobe_w(u8 data);
	void display_out1_w(u8 data);
	void display_out2_w(u8 data);
	void display_out3_w(u8 data);
	void display_out4_w(u8 data);
	u8 display_in3_r();
	void switch_w(u8 data);
	u8 switch_r();
	u8 dmdstatus_r();
	void sound_w(u8 data);
	void solenoid1_w(u8 data);
};

class decocpu_type2_device : public decocpu_type1_device
{
public:
	template <typename T>
	decocpu_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpuregion_tag)
		: decocpu_type2_device(mconfig, tag, owner, clock)
	{
		set_cpuregion(std::forward<T>(cpuregion_tag));
	}

	decocpu_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void decocpu2_map(address_map &map) ATTR_COLD;
protected:
	decocpu_type2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

class decocpu_type3_device : public decocpu_type2_device
{
public:
	template <typename T>
	decocpu_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpuregion_tag)
		: decocpu_type3_device(mconfig, tag, owner, clock)
	{
		set_cpuregion(std::forward<T>(cpuregion_tag));
	}

	decocpu_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	decocpu_type3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// overrides
	virtual void device_start() override ATTR_COLD;
};

class decocpu_type3b_device : public decocpu_type3_device
{
public:
	template <typename T>
	decocpu_type3b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpuregion_tag)
		: decocpu_type3b_device(mconfig, tag, owner, clock)
	{
		set_cpuregion(std::forward<T>(cpuregion_tag));
	}

	decocpu_type3b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// overrides
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(DECOCPU1,  decocpu_type1_device)
DECLARE_DEVICE_TYPE(DECOCPU2,  decocpu_type2_device)
DECLARE_DEVICE_TYPE(DECOCPU3,  decocpu_type3_device)
DECLARE_DEVICE_TYPE(DECOCPU3B, decocpu_type3b_device)

#endif // MAME_PINBALL_DECOPINCPU_H
