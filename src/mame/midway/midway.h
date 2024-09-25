// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    midway.h

    Functions to emulate general the various Midway sound cards.

***************************************************************************/
#ifndef MAME_MIDWAY_MIDWAY_H
#define MAME_MIDWAY_MIDWAY_H

#pragma once


#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/flt_biquad.h"
#include "sound/hc55516.h"
#include "sound/tms5220.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(MIDWAY_SSIO,               midway_ssio_device)
DECLARE_DEVICE_TYPE(MIDWAY_SOUNDS_GOOD,        midway_sounds_good_device)
DECLARE_DEVICE_TYPE(MIDWAY_TURBO_CHEAP_SQUEAK, midway_turbo_cheap_squeak_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> midway_ssio_device

class midway_ssio_device :  public device_t,
							public device_mixer_interface
{
public:
	// construction/destruction
	midway_ssio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 16'000'000);

	// helpers
	void suspend_cpu();

	// read/write
	uint8_t read();
	void write(offs_t offset, uint8_t data);
	void reset_write(int state);
	uint8_t ioport_read(offs_t offset);
	void ioport_write(offs_t offset, uint8_t data);

	// configuration
	template <typename... T> void set_custom_input(int which, uint8_t mask, T &&... args)
	{
		m_custom_input_mask[which] = mask;
		m_custom_input[which].set(std::forward<T>(args)...);
	}
	template <typename... T> void set_custom_output(int which, uint8_t mask, T &&... args)
	{
		m_custom_output_mask[which / 4] = mask;
		m_custom_output[which / 4].set(std::forward<T>(args)...);
	}

	// internal communications
	uint8_t irq_clear();
	void status_w(uint8_t data);
	uint8_t data_r(offs_t offset);

	void ssio_map(address_map &map) ATTR_COLD;
	static void ssio_input_ports(address_map &map, const char *ssio) ATTR_COLD;

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(synced_write);

private:
	// internal helpers
	void compute_ay8910_modulation();
	void update_volumes();

	// devices
	required_device<z80_device> m_cpu;
	required_device<ay8910_device> m_ay0;
	required_device<ay8910_device> m_ay1;

	// I/O ports
	optional_ioport_array<5> m_ports;

	// internal state
	uint8_t m_data[4];
	uint8_t m_status;
	uint8_t m_14024_count;
	uint8_t m_mute;
	uint8_t m_overall[2];
	uint8_t m_duty_cycle[2][3];
	uint8_t m_ayvolume_lookup[16];

	// I/O port overrides
	uint8_t m_custom_input_mask[5];
	read8smo_delegate::array<5> m_custom_input;
	uint8_t m_custom_output_mask[2];
	write8smo_delegate::array<2> m_custom_output;

	INTERRUPT_GEN_MEMBER(clock_14024);
	void porta0_w(uint8_t data);
	void portb0_w(uint8_t data);
	void porta1_w(uint8_t data);
	void portb1_w(uint8_t data);
};


// ======================> midway_sounds_good_device

class midway_sounds_good_device :   public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	midway_sounds_good_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 16'000'000);

	// read/write
	uint8_t read();
	void write(uint8_t data);
	void reset_write(int state);

	void soundsgood_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(synced_write);

private:
	// devices
	required_device<m68000_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<ad7533_device> m_dac;
	required_device_array<filter_biquad_device, 3> m_dac_filter;

	// internal state
	uint8_t m_status;
	uint16_t m_dacval;

	// internal communications
	void porta_w(uint8_t data);
	void portb_w(uint8_t data);
	void irq_w(int state);
};


// ======================> midway_turbo_cheap_squeak_device

class midway_turbo_cheap_squeak_device : public device_t,
										public device_mixer_interface
{
public:
	// construction/destruction
	midway_turbo_cheap_squeak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 8'000'000);

	// read/write
	uint8_t read();
	void write(uint8_t data);
	void reset_write(int state);

	void turbocs_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(synced_write);

private:
	// devices
	required_device<mc6809e_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<ad7533_device> m_dac;
	required_device_array<filter_biquad_device, 3> m_dac_filter;

	// internal state
	uint8_t m_status;
	uint16_t m_dacval;

	// internal communications
	void porta_w(uint8_t data);
	void portb_w(uint8_t data);
	void irq_w(int state);
};

#endif // MAME_MIDWAY_MIDWAY_H
