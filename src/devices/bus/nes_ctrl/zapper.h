// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Zapper Lightgun
    Nintendo Family Computer Bandai Hyper Shot Lightgun
    Nintendo R.O.B.

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_ZAPPER
#define MAME_BUS_NES_CTRL_ZAPPER

#pragma once


#include "ctrl.h"
#include "cpu/sm510/sm590.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_zapper_device

class nes_zapper_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_zapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	// construction/destruction
	nes_zapper_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;

	virtual u8 read_bit34() override;
	virtual u8 read_exp(offs_t offset) override;

private:
	required_ioport m_lightx;
	required_ioport m_lighty;
	required_ioport m_trigger;
};


// ======================> nes_bandaihs_device

class nes_bandaihs_device : public nes_zapper_device
{
public:
	// construction/destruction
	nes_bandaihs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

private:
	required_ioport m_joypad;
	u8 m_latch;
};


// ======================> nes_rob_device

class nes_rob_device : public nes_zapper_device
{
public:
	// construction/destruction
	nes_rob_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual u8 read_exp(offs_t offset) override { return 0; }

private:
	required_device<sm590_device> m_maincpu;
	output_finder<6> m_motor_out;
	output_finder<> m_led_out;

	u8 input_r();
	void output_w(offs_t offset, u8 data);
};


// device type definition
DECLARE_DEVICE_TYPE(NES_ZAPPER, nes_zapper_device)
DECLARE_DEVICE_TYPE(NES_BANDAIHS, nes_bandaihs_device)
DECLARE_DEVICE_TYPE(NES_ROB, nes_rob_device)

#endif // MAME_BUS_NES_CTRL_ZAPPER
