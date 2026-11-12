// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Sergey Svishchev
#ifndef MAME_GRIDCOMP_GRIDKEYB_H
#define MAME_GRIDCOMP_GRIDKEYB_H

#pragma once

#include "cpu/mcs48/mcs48.h"


/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

DECLARE_DEVICE_TYPE(GRID_KEYBOARD, grid_keyboard_device)



/***************************************************************************
    REUSABLE I/O PORTS
***************************************************************************/

INPUT_PORTS_EXTERN( grid_keyboard );



/***************************************************************************
    TYPE DECLARATIONS
***************************************************************************/


class grid_keyboard_device : public device_t
{
public:
	grid_keyboard_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			u32 clock = 0);

	auto irq_callback() { return m_irq_cb.bind(); }
	auto dma_callback() { return m_dma_cb.bind(); }
	auto nmi_callback() { return m_nmi_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void p2_w(u8 data);
	u8 p1_r();
	int t0_r();
	int t1_r();

	required_device<i8741a_device> m_mcu;
	required_ioport_array<8> m_columns;
	required_ioport m_modifiers;

private:
	devcb_write_line m_irq_cb;
	devcb_write_line m_dma_cb;
	devcb_write_line m_nmi_cb;
};

#endif // MAME_GRIDCOMP_GRIDKEYB_H
