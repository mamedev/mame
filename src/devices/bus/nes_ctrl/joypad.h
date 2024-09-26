// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Joypads

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_JOYPAD_H
#define MAME_BUS_NES_CTRL_JOYPAD_H

#pragma once

#include "ctrl.h"

INPUT_PORTS_EXTERN( nes_joypad );

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_joypad_device

class nes_joypad_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_joypad_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_bit0() override;
	virtual void write(u8 data) override;

protected:
	nes_joypad_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 latch_fill = 0x80);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void set_latch() { m_latch = m_joypad->read(); }

	required_ioport m_joypad;
	u32 m_latch;  // wider than standard joypad's 8-bit latch to accomodate subclass devices
	const u32 m_latch_fill;  // the new MSB as a joypad's shift register shifts
};


// ======================> nes_fcpadexp_device

class nes_fcpadexp_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_fcpadexp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_bit0() override { return 0; }
	virtual u8 read_exp(offs_t offset) override;

protected:
	nes_fcpadexp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 latch_fill = 0x80);
};


// ======================> nes_fcpad2_device

class nes_fcpad2_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_fcpad2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_bit2() override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_mic;
};


// ======================> nes_ccpadl_device

class nes_ccpadl_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_ccpadl_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> nes_ccpadr_device

class nes_ccpadr_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_ccpadr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> nes_arcstick_device

class nes_arcstick_device : public nes_fcpadexp_device
{
public:
	// construction/destruction
	nes_arcstick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<nes_control_port_device> m_daisychain;
	required_ioport m_cfg;
};


// ======================> nes_vboyctrl_device

class nes_vboyctrl_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_vboyctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_JOYPAD,         nes_joypad_device)
DECLARE_DEVICE_TYPE(NES_FCPAD_EXP,      nes_fcpadexp_device)
DECLARE_DEVICE_TYPE(NES_FCPAD_P2,       nes_fcpad2_device)
DECLARE_DEVICE_TYPE(NES_CCPAD_LEFT,     nes_ccpadl_device)
DECLARE_DEVICE_TYPE(NES_CCPAD_RIGHT,    nes_ccpadr_device)
DECLARE_DEVICE_TYPE(NES_ARCSTICK,       nes_arcstick_device)
DECLARE_DEVICE_TYPE(NES_VBOYCTRL,       nes_vboyctrl_device)

#endif // MAME_BUS_NES_CTRL_JOYPAD_H
