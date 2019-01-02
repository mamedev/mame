// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Joypads

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_JOYPAD_H
#define MAME_BUS_NES_CTRL_JOYPAD_H

#pragma once

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
	nes_joypad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	nes_joypad_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

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

// ======================> nes_vt_majesco_ddr_device

class nes_vt_majesco_ddr_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_vt_majesco_ddr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;
};

// ======================> nes_arcstick_device

class nes_arcstick_device : public nes_joypad_device
{
public:
	// construction/destruction
	nes_arcstick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t read_bit0() override { return 0; }
	virtual uint8_t read_exp(offs_t offset) override;
	virtual void write(uint8_t data) override;

	required_device<nes_control_port_device> m_daisychain;
	required_ioport m_cfg;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_JOYPAD,         nes_joypad_device)
DECLARE_DEVICE_TYPE(NES_FCPAD_P2,       nes_fcpad2_device)
DECLARE_DEVICE_TYPE(NES_CCPAD_LEFT,     nes_ccpadl_device)
DECLARE_DEVICE_TYPE(NES_CCPAD_RIGHT,    nes_ccpadr_device)
DECLARE_DEVICE_TYPE(NES_ARCSTICK,       nes_arcstick_device)
DECLARE_DEVICE_TYPE(NES_VT_MAJESCO_DDR, nes_vt_majesco_ddr_device)

#endif // MAME_BUS_NES_CTRL_JOYPAD_H
