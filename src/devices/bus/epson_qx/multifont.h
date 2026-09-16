// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * Q10MF Multifont card
 *
 *******************************************************************/

#ifndef MAME_BUS_EPSON_QX_MULTIFONT_H
#define MAME_BUS_EPSON_QX_MULTIFONT_H

#pragma once

#include "bus/epson_qx/option.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"

namespace bus::epson_qx {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* Epson QX-10 Multifont Device */

class multifont_device : public device_t, public bus::epson_qx::device_option_expansion_interface
{
public:
	// construction/destruction
	multifont_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void rom_map(address_map &map) ATTR_COLD;
	void map(address_map &map) ATTR_COLD;

	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	template<int Shift> void rom_addr_w(uint8_t data);
	void rom_select_w(uint8_t data);
	uint8_t t0_r();
	uint8_t t1_r();

	void write_bus(uint8_t data);
	uint8_t read_bus();
private:
	void update_bank_select();

	required_device<i8039_device> m_mcu;
	required_device<i8243_device> m_i8243;
	required_memory_region_array<8> m_fonts;
	required_ioport m_ioport;

	uint8_t m_status;

	uint8_t m_data_out;
	uint8_t m_data_in;
	uint8_t m_p1;
	uint8_t m_p2;
	uint8_t m_rom_sel;
	uint16_t m_address;
	uint8_t m_rom_bank;

	bool m_bus_reset;
	bool m_hard_reset;
};

} // namespace bus::epson_qx

// device type definition
DECLARE_DEVICE_TYPE_NS(EPSON_QX_OPTION_MULTIFONT, bus::epson_qx, multifont_device)


#endif // MAME_BUS_EPSON_QX_MULTIFONT_H


