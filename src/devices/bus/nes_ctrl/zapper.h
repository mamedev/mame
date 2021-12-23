// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Zapper Lightgun
    Nintendo Family Computer Bandai Hyper Shot Lightgun

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_ZAPPER
#define MAME_BUS_NES_CTRL_ZAPPER

#pragma once


#include "ctrl.h"

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


// device type definition
DECLARE_DEVICE_TYPE(NES_ZAPPER,   nes_zapper_device)
DECLARE_DEVICE_TYPE(NES_BANDAIHS, nes_bandaihs_device)

#endif // MAME_BUS_NES_CTRL_ZAPPER
