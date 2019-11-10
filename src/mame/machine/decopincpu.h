// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Data East Pinball CPU boards
 *
 *  Created on: 10/07/2013
 */

#ifndef MAME_MACHINE_DECOPINCPU_H
#define MAME_MACHINE_DECOPINCPU_H

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
	decocpu_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpuregion_tag)
		: decocpu_type1_device(mconfig, tag, owner, clock)
	{
		set_cpuregion(std::forward<T>(cpuregion_tag));
	}

	decocpu_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto display_read_callback() { return m_read_display.bind(); }
	auto display_write_callback() { return m_write_display.bind(); }
	auto dmdstatus_read_callback() { return m_read_dmdstatus.bind(); }
	auto soundlatch_write_callback() { return m_write_soundlatch.bind(); }
	auto switch_read_callback() { return m_read_switch.bind(); }
	auto switch_write_callback() { return m_write_switch.bind(); }
	auto lamp_write_callback() { return m_write_lamp.bind(); }
	auto solenoid_write_callback() { return m_write_solenoid.bind(); }

	DECLARE_WRITE8_MEMBER(solenoid2_w);
	INPUT_CHANGED_MEMBER(main_nmi);
	INPUT_CHANGED_MEMBER(audio_nmi);

	template <typename T> void set_cpuregion(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); } // region for cpu board code and data

protected:
	static constexpr device_timer_id TIMER_IRQ = 0;

	decocpu_type1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void decocpu1_map(address_map &map);

	required_device<cpu_device> m_cpu;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia2c;
	required_device<pia6821_device> m_pia30;
	required_device<pia6821_device> m_pia34;
	required_region_ptr<uint8_t> m_rom;

private:
	emu_timer* m_irq_timer;
	bool m_irq_active;
	bool m_ca2;

	// callbacks
	devcb_read8 m_read_display;
	devcb_write8 m_write_display;
	devcb_read8 m_read_dmdstatus;
	devcb_write8 m_write_soundlatch;
	devcb_read8 m_read_switch;
	devcb_write8 m_write_switch;
	devcb_write8 m_write_lamp;
	devcb_write8 m_write_solenoid;

	DECLARE_WRITE_LINE_MEMBER(cpu_pia_irq);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w);
	DECLARE_READ8_MEMBER(display_strobe_r);
	DECLARE_WRITE8_MEMBER(display_strobe_w);
	DECLARE_WRITE8_MEMBER(display_out1_w);
	DECLARE_WRITE8_MEMBER(display_out2_w);
	DECLARE_WRITE8_MEMBER(display_out3_w);
	DECLARE_WRITE8_MEMBER(display_out4_w);
	DECLARE_READ8_MEMBER(display_in3_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_READ8_MEMBER(dmdstatus_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(solenoid1_w);
};

class decocpu_type2_device : public decocpu_type1_device
{
public:
	template <typename T>
	decocpu_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpuregion_tag)
		: decocpu_type2_device(mconfig, tag, owner, clock)
	{
		set_cpuregion(std::forward<T>(cpuregion_tag));
	}

	decocpu_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void decocpu2_map(address_map &map);
protected:
	decocpu_type2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
};

class decocpu_type3_device : public decocpu_type2_device
{
public:
	template <typename T>
	decocpu_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpuregion_tag)
		: decocpu_type3_device(mconfig, tag, owner, clock)
	{
		set_cpuregion(std::forward<T>(cpuregion_tag));
	}

	decocpu_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	decocpu_type3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// overrides
	virtual void device_start() override;
};

class decocpu_type3b_device : public decocpu_type3_device
{
public:
	template <typename T>
	decocpu_type3b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpuregion_tag)
		: decocpu_type3b_device(mconfig, tag, owner, clock)
	{
		set_cpuregion(std::forward<T>(cpuregion_tag));
	}

	decocpu_type3b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// overrides
	virtual void device_start() override;
};

DECLARE_DEVICE_TYPE(DECOCPU1,  decocpu_type1_device)
DECLARE_DEVICE_TYPE(DECOCPU2,  decocpu_type2_device)
DECLARE_DEVICE_TYPE(DECOCPU3,  decocpu_type3_device)
DECLARE_DEVICE_TYPE(DECOCPU3B, decocpu_type3b_device)

#endif // MAME_MACHINE_DECOPINCPU_H
