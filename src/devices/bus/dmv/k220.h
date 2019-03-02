// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_DMV_K220_H
#define MAME_BUS_DMV_K220_H

#pragma once

#include "dmvbus.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_k220_device

class dmv_k220_device :
		public device_t,
		public device_dmvslot_interface
{
public:
	// construction/destruction
	dmv_k220_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// dmvcart_interface overrides
	virtual bool read(offs_t offset, uint8_t &data) override;
	virtual bool write(offs_t offset, uint8_t data) override;

private:
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_WRITE_LINE_MEMBER(write_out0);
	DECLARE_WRITE_LINE_MEMBER(write_out1);
	DECLARE_WRITE_LINE_MEMBER(write_out2);

	required_device<dmvcart_slot_device> m_bus;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_memory_region m_ram;
	required_memory_region m_rom;

	output_finder<2> m_digits;

	uint8_t   m_portc;
};


// device type definition
DECLARE_DEVICE_TYPE(DMV_K220, dmv_k220_device)

#endif // MAME_BUS_DMV_K220_H
