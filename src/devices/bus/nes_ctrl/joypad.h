// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Joypads

**********************************************************************/

#pragma once

#ifndef __NES_JOYPAD__
#define __NES_JOYPAD__


#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_joypad_device

class nes_joypad_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_joypad_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_joypad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_bit0() override;
	virtual void write(uint8_t data) override;

	required_ioport m_joypad;
	uint32_t m_latch;
};

// ======================> nes_fcpad2_device

class nes_fcpad2_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_fcpad2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual uint8_t read_exp(offs_t offset) override;
	virtual void write(uint8_t data) override;
};

// ======================> nes_ccpadl_device

class nes_ccpadl_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_ccpadl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;
};

// ======================> nes_ccpadr_device

class nes_ccpadr_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_ccpadr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;
};

// ======================> nes_arcstick_device

class nes_arcstick_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_arcstick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	virtual uint8_t read_bit0() override { return 0; }
	virtual uint8_t read_exp(offs_t offset) override;
	virtual void write(uint8_t data) override;

	required_device<nes_control_port_device> m_daisychain;
	required_ioport m_cfg;
};


// device type definition
extern const device_type NES_JOYPAD;
extern const device_type NES_FCPAD_P2;
extern const device_type NES_CCPAD_LEFT;
extern const device_type NES_CCPAD_RIGHT;
extern const device_type NES_ARCSTICK;

#endif
