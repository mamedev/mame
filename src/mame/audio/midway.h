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
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MIDWAY_SSIO_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MIDWAY_SSIO, 0)
#define MCFG_MIDWAY_CHIP_SQUEAK_DELUXE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MIDWAY_CHIP_SQUEAK_DELUXE, 0)
#define MCFG_MIDWAY_SOUNDS_GOOD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MIDWAY_SOUNDS_GOOD, 0)
#define MCFG_MIDWAY_TURBO_CHIP_SQUEAK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MIDWAY_TURBO_CHIP_SQUEAK, 0)
#define MCFG_MIDWAY_SQUAWK_N_TALK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MIDWAY_SQUAWK_N_TALK, 0)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> midway_ssio_device

class midway_ssio_device :  public device_t,
							public device_mixer_interface
{
public:
	// construction/destruction
	midway_ssio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read/write
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);
	DECLARE_READ8_MEMBER(ioport_read);
	DECLARE_WRITE8_MEMBER(ioport_write);

	// configuration
	void set_custom_input(int which, UINT8 mask, read8_delegate handler);
	void set_custom_output(int which, UINT8 mask, write8_delegate handler);

	// internal communications
	INTERRUPT_GEN_MEMBER(clock_14024);
	DECLARE_READ8_MEMBER(irq_clear);
	DECLARE_WRITE8_MEMBER(status_w);
	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(porta0_w);
	DECLARE_WRITE8_MEMBER(portb0_w);
	DECLARE_WRITE8_MEMBER(porta1_w);
	DECLARE_WRITE8_MEMBER(portb1_w);

protected:
	// device-level overrides
	virtual const rom_entry *device_rom_region() const override;
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

	// internal state
	UINT8 m_data[4];
	UINT8 m_status;
	UINT8 m_14024_count;
	UINT8 m_mute;
	UINT8 m_overall[2];
	UINT8 m_duty_cycle[2][3];
	UINT8 m_ayvolume_lookup[16];

	// I/O port overrides
	UINT8 m_custom_input_mask[5];
	read8_delegate m_custom_input[5];
	UINT8 m_custom_output_mask[2];
	write8_delegate m_custom_output[2];
};


// ======================> midway_chip_squeak_deluxe_device

class midway_chip_squeak_deluxe_device :    public device_t,
											public device_mixer_interface
{
public:
	// construction/destruction
	midway_chip_squeak_deluxe_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read/write
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	// internal communications
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_READ16_MEMBER(pia_r);
	DECLARE_WRITE16_MEMBER(pia_w);

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
	required_device<dac_device> m_dac;

	// internal state
	UINT8 m_status;
	UINT16 m_dacval;
};


// ======================> midway_sounds_good_device

class midway_sounds_good_device :   public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	midway_sounds_good_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read/write
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	// internal communications
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

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
	required_device<dac_device> m_dac;

	// internal state
	UINT8 m_status;
	UINT16 m_dacval;
};


// ======================> midway_turbo_chip_squeak_device

class midway_turbo_chip_squeak_device : public device_t,
										public device_mixer_interface
{
public:
	// construction/destruction
	midway_turbo_chip_squeak_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read/write
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	// internal communications
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

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
	required_device<dac_device> m_dac;

	// internal state
	UINT8 m_status;
	UINT16 m_dacval;
};


// ======================> midway_squawk_n_talk_device

class midway_squawk_n_talk_device : public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	midway_squawk_n_talk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read/write
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	// internal communications
	DECLARE_WRITE8_MEMBER(porta1_w);
	DECLARE_WRITE8_MEMBER(dac_w);
	DECLARE_WRITE8_MEMBER(porta2_w);
	DECLARE_WRITE8_MEMBER(portb2_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

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
	UINT8 m_tms_command;
	UINT8 m_tms_strobes;
};


/************ SSIO input ports ***************/

#define SSIO_INPUT_PORTS(ssio) \
	AM_RANGE(0x00, 0x04) AM_MIRROR(0x18) AM_DEVREAD(ssio, midway_ssio_device, ioport_read) \
	AM_RANGE(0x07, 0x07) AM_MIRROR(0x18) AM_DEVREAD(ssio, midway_ssio_device, read) \
	AM_RANGE(0x00, 0x07) AM_MIRROR(0x03) AM_DEVWRITE(ssio, midway_ssio_device, ioport_write) \
	AM_RANGE(0x1c, 0x1f) AM_DEVWRITE(ssio, midway_ssio_device, write)


#endif /* __MIDWAY_AUDIO__ */
