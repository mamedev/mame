// license:BSD-3-Clause
// copyright-holders:Dirk Best
/**********************************************************************

    Epson LX-800 dot matrix printer emulation

**********************************************************************/

#ifndef MAME_BUS_CENTRONICS_EPSON_LX800_H
#define MAME_BUS_CENTRONICS_EPSON_LX800_H

#pragma once

#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/e05a03.h"
#include "sound/beep.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> epson_lx800_device

class epson_lx800_device :  public device_t, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_lx800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	epson_lx800_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual bool supports_pin35_5v() override { return true; }

private:
	uint8_t porta_r(offs_t offset);
	void porta_w(offs_t offset, uint8_t data);
	uint8_t portc_r(offs_t offset);
	void portc_w(offs_t offset, uint8_t data);

	int an0_r();
	int an1_r();
	int an2_r();
	int an3_r();
	int an4_r();
	int an5_r();

	uint8_t centronics_data_r();
	void centronics_pe_w(int state);
	void reset_w(int state);

	void lx800_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
	output_finder<> m_online_led;
};



// device type definition
DECLARE_DEVICE_TYPE(EPSON_LX800, epson_lx800_device)

#endif // MAME_BUS_CENTRONICS_EPSON_LX800_H
