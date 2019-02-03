// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    midway.h

    Functions to emulate general the various Midway sound cards.

***************************************************************************/
#ifndef MAME_AUDIO_MIDWAY_H
#define MAME_AUDIO_MIDWAY_H

#pragma once


#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "sound/tms5220.h"
#include "sound/ay8910.h"
#include "sound/dac.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(MIDWAY_SSIO,               midway_ssio_device)
DECLARE_DEVICE_TYPE(MIDWAY_SOUNDS_GOOD,        midway_sounds_good_device)
DECLARE_DEVICE_TYPE(MIDWAY_TURBO_CHEAP_SQUEAK, midway_turbo_cheap_squeak_device)
DECLARE_DEVICE_TYPE(MIDWAY_SQUAWK_N_TALK,      midway_squawk_n_talk_device)



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
	u8 read();
	void write(offs_t offset, u8 data);
	DECLARE_WRITE_LINE_MEMBER(reset_write);
	DECLARE_READ8_MEMBER(ioport_read);
	DECLARE_WRITE8_MEMBER(ioport_write);

	// configuration
	void set_custom_input(int which, uint8_t mask, read8_delegate handler);
	void set_custom_output(int which, uint8_t mask, write8_delegate handler);

	// internal communications
	u8 irq_clear();
	void status_w(u8 data);
	u8 data_r(offs_t offset);

	void ssio_map(address_map &map);
	static void ssio_input_ports(address_map &map, const char *ssio);

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// internal helpers
	void compute_ay8910_modulation();
	void update_volumes();

	// devices
	required_device<z80_device> m_cpu;
	required_device_array<ay8910_device, 2> m_ay;

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
	read8_delegate m_custom_input[5];
	uint8_t m_custom_output_mask[2];
	write8_delegate m_custom_output[2];

	INTERRUPT_GEN_MEMBER(clock_14024);
	void porta0_w(u8 data);
	void portb0_w(u8 data);
	void porta1_w(u8 data);
	void portb1_w(u8 data);

};


// ======================> midway_sounds_good_device

class midway_sounds_good_device :   public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	midway_sounds_good_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 16'000'000);

	// read/write
	u8 read();
	void write(u8 data);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	void soundsgood_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// devices
	required_device<m68000_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<dac_word_interface> m_dac;

	// internal state
	uint8_t m_status;
	uint16_t m_dacval;

	// internal communications
	void porta_w(u8 data);
	void portb_w(u8 data);
};


// ======================> midway_turbo_cheap_squeak_device

class midway_turbo_cheap_squeak_device : public device_t,
										public device_mixer_interface
{
public:
	// construction/destruction
	midway_turbo_cheap_squeak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 8'000'000);

	// read/write
	u8 read();
	void write(u8 data);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	void turbocs_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// devices
	required_device<mc6809e_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<dac_word_interface> m_dac;

	// internal state
	uint8_t m_status;
	uint16_t m_dacval;

	// internal communications
	void porta_w(u8 data);
	void portb_w(u8 data);
};


// ======================> midway_squawk_n_talk_device

class midway_squawk_n_talk_device : public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	midway_squawk_n_talk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 3'579'545);

	// read/write
	void write(u8 data);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	// internal communications
	void dac_w(u8 data);

	void squawkntalk_alt_map(address_map &map);
	void squawkntalk_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// devices
	required_device<m6802_cpu_device> m_cpu;
	required_device_array<pia6821_device, 2> m_pia;
	optional_device<tms5200_device> m_tms5200;

	// internal state
	uint8_t m_tms_command;
	uint8_t m_tms_strobes;

	// internal communications
	void porta1_w(u8 data);
	void porta2_w(u8 data);
	void portb2_w(u8 data);
};

#endif // MAME_AUDIO_MIDWAY_H
