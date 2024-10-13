// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    ramcard16k.h

    Implemention of the Apple II 16K RAM card (aka "language card")

*********************************************************************/

#ifndef MAME_BUS_A2BUS_RAMCARD16K_H
#define MAME_BUS_A2BUS_RAMCARD16K_H

#pragma once

#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ramcard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ramcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_ramcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_inh_rom(uint16_t offset) override;
	virtual void write_inh_rom(uint16_t offset, uint8_t data) override;
	virtual uint16_t inh_start() override { return 0xd000; }
	virtual uint16_t inh_end() override { return 0xffff; }
	virtual int inh_type() override;

private:
	void do_io(int offset, bool writing);

	int m_inh_state;
	bool m_prewrite;
	int m_dxxx_bank;
	uint8_t m_ram[16*1024];
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_RAMCARD16K, a2bus_ramcard_device)

#endif // MAME_BUS_A2BUS_RAMCARD16K_H
