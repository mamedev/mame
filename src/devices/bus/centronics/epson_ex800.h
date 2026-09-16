// license:BSD-3-Clause
// copyright-holders:Dirk Best
/**********************************************************************

    Epson EX-800 dot matrix printer emulation

**********************************************************************/

#ifndef MAME_BUS_CENTRONICS_EPSON_EX800_H
#define MAME_BUS_CENTRONICS_EPSON_EX800_H

#pragma once

#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "sound/beep.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> epson_ex800_device

class epson_ex800_device :  public device_t, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_ex800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(online_switch);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual bool supports_pin35_5v() override { return true; }

private:
	uint8_t porta_r();
	uint8_t portb_r();
	uint8_t portc_r();
	void porta_w(uint8_t data);
	void portb_w(uint8_t data);
	void portc_w(uint8_t data);
	uint8_t devsel_r(offs_t offset);
	void devsel_w(offs_t offset, uint8_t data);
	uint8_t gate5a_r(offs_t offset);
	void gate5a_w(offs_t offset, uint8_t data);
	uint8_t iosel_r(offs_t offset);
	void iosel_w(offs_t offset, uint8_t data);
	uint8_t gate7a_r(offs_t offset);
	void gate7a_w(offs_t offset, uint8_t data);

	void ex800_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beeper;

	int m_irq_state;
};



// device type definition
DECLARE_DEVICE_TYPE(EPSON_EX800, epson_ex800_device)



#endif // MAME_BUS_CENTRONICS_EPSON_EX800_H
