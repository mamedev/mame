// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Tangerine TANEX (MT002 Iss2)

**********************************************************************/


#ifndef MAME_BUS_TANBUS_TANEX_H
#define MAME_BUS_TANBUS_TANEX_H

#pragma once

#include "bus/tanbus/tanbus.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/rs232/rs232.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "machine/input_merger.h"
#include "machine/timer.h"
#include "imagedev/cassette.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_tanex_device :
	public device_t,
	public device_tanbus_interface
{
public:
	// construction/destruction
	tanbus_tanex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	enum { IRQ_VIA_0, IRQ_VIA_1, IRQ_ACIA };

	void bus_irq_w(int state);
	void bus_so_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(read_cassette);
	uint8_t via_0_in_a();
	void via_0_out_a(uint8_t data);
	void via_0_out_b(uint8_t data);
	void via_0_out_ca2(int state);
	void via_0_out_cb2(int state);
	void via_1_out_a(uint8_t data);
	void via_1_out_b(uint8_t data);
	void via_1_out_ca2(int state);
	void via_1_out_cb2(int state);

	required_memory_region m_rom_tanex;
	required_memory_region m_rom_h2;
	required_memory_region m_rom_e2;
	required_device_array<generic_slot_device, 5> m_rom;
	required_device<cassette_image_device> m_cassette;
	required_device<mos6551_device> m_acia;
	required_device<rs232_port_device> m_rs232;
	required_device_array<via6522_device, 2> m_via6522;
	required_device<input_merger_device> m_irq_line;
	required_ioport m_config;

	std::unique_ptr<uint8_t[]> m_ram;
	//emu_timer *m_read_cassette_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_TANEX, tanbus_tanex_device)


#endif // MAME_BUS_TANBUS_TANEX_H
