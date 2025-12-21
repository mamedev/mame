// license:BSD-3-Clause
// copyright-holders:Rob Justice, R. Belmont
/*********************************************************************

    q68.h

    Implementation of the Stellation Q-68 68008 card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_Q68_H
#define MAME_BUS_A2BUS_Q68_H

#pragma once

#include "a2bus.h"


//**************************************************************************
//  DEVICE TYPE DECLARATION
//**************************************************************************

DECLARE_DEVICE_TYPE(A2BUS_Q68, a2bus_q68_device)
DECLARE_DEVICE_TYPE(A2BUS_Q68PLUS, a2bus_q68plus_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_68k_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	a2bus_68k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override { return false; }

	uint8_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint8_t data);

	required_device<cpu_device> m_m68008;

private:
	bool m_bEnabled;
};

class a2bus_q68_device : public a2bus_68k_device
{
public:
	a2bus_q68_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void m68008_mem(address_map &map) ATTR_COLD;
};

class a2bus_q68plus_device : public a2bus_68k_device
{
public:
	a2bus_q68plus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static auto parent_rom_device_type() { return &A2BUS_Q68; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void m68008_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_BUS_A2BUS_Q68_H
