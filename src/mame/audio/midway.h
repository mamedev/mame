// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    midway.h

    Functions to emulate general the various Midway sound cards.

***************************************************************************/

#pragma once

#ifndef __MIDWAY_AUDIO__
#define __MIDWAY_AUDIO__

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

extern const device_type MIDWAY_SSIO;
extern const device_type MIDWAY_CHIP_SQUEAK_DELUXE;
extern const device_type MIDWAY_SOUNDS_GOOD;
extern const device_type MIDWAY_TURBO_CHIP_SQUEAK;
extern const device_type MIDWAY_SQUAWK_N_TALK;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> midway_ssio_device

class midway_ssio_device :  public device_t,
							public device_mixer_interface
{
public:
	// construction/destruction
	midway_ssio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reset_write(int state);
	uint8_t ioport_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ioport_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// configuration
	void set_custom_input(int which, uint8_t mask, read8_delegate handler);
	void set_custom_output(int which, uint8_t mask, write8_delegate handler);

	// internal communications
	void clock_14024(device_t &device);
	uint8_t irq_clear(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void porta0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void porta1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
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
	read8_delegate m_custom_input[5];
	uint8_t m_custom_output_mask[2];
	write8_delegate m_custom_output[2];
};


// ======================> midway_chip_squeak_deluxe_device

class midway_chip_squeak_deluxe_device :    public device_t,
											public device_mixer_interface
{
public:
	// construction/destruction
	midway_chip_squeak_deluxe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reset_write(int state);

	// internal communications
	void porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_w(int state);
	uint16_t pia_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pia_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
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
};


// ======================> midway_sounds_good_device

class midway_sounds_good_device :   public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	midway_sounds_good_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reset_write(int state);

	// internal communications
	void porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_w(int state);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
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
};


// ======================> midway_turbo_chip_squeak_device

class midway_turbo_chip_squeak_device : public device_t,
										public device_mixer_interface
{
public:
	// construction/destruction
	midway_turbo_chip_squeak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reset_write(int state);

	// internal communications
	void porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_w(int state);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// devices
	required_device<m6809e_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<dac_word_interface> m_dac;

	// internal state
	uint8_t m_status;
	uint16_t m_dacval;
};


// ======================> midway_squawk_n_talk_device

class midway_squawk_n_talk_device : public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	midway_squawk_n_talk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reset_write(int state);

	// internal communications
	void porta1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void porta2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_w(int state);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// devices
	required_device<m6802_cpu_device> m_cpu;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	optional_device<tms5200_device> m_tms5200;

	// internal state
	uint8_t m_tms_command;
	uint8_t m_tms_strobes;
};


/************ SSIO input ports ***************/

#define SSIO_INPUT_PORTS(ssio) \
	AM_RANGE(0x00, 0x04) AM_MIRROR(0x18) AM_DEVREAD(ssio, midway_ssio_device, ioport_read) \
	AM_RANGE(0x07, 0x07) AM_MIRROR(0x18) AM_DEVREAD(ssio, midway_ssio_device, read) \
	AM_RANGE(0x00, 0x07) AM_DEVWRITE(ssio, midway_ssio_device, ioport_write) \
	AM_RANGE(0x1c, 0x1f) AM_DEVWRITE(ssio, midway_ssio_device, write)


#endif /* __MIDWAY_AUDIO__ */
