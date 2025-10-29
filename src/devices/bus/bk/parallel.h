// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    BK parallel slot with byte addressing mod

    Original bk0010 optionally routed bitbanger to B2 (rx) and B3 (tx).

    A1  INIT       B1  IRQ2
    A2             B2  IRQ3
    A3             B3
    A4             B4
    A5             B5  out 04
    A6             B6  out 06
    A7  out 07     B7  out 05
    A8  +5V        B8  +5V
    A9  +5V        B9  +5V
    A10            B10 out 03
    A11 VCC        B11 VCC
    A12            B12 out 02
    A13 out 01     B13
    A14            B14
    A15            B15
    A16 out 00     B16
    A17            B17 in 03
    A18 VCC        B18 VCC
    A19 VCC        B19 VCC
    A20 in 05      B20 in 04
    A21            B21
    A22            B22 in 06
    A23 in 07      B23 in 02
    A24 in 01      B24 in 00
    A25 out 14     B25 out 15
    A26 out 12     B26 out 13
    A27 out 10     B27 out 11
    A28 out 08     B28 out 09
    A29 in 13      B29 in 14
    A30 in 15      B30 in 12
    A31 in 09      B31 in 08
    A32 in 11      B32 in 10

***************************************************************************/

#ifndef MAME_BUS_BK_PARALLEL_H
#define MAME_BUS_BK_PARALLEL_H

#pragma once

// include here so drivers don't need to
#include "carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_bk_parallel_interface;

class bk_parallel_slot_device : public device_t, public device_single_card_slot_interface<device_bk_parallel_interface>
{
public:
	// construction/destruction
	template <typename T>
	bk_parallel_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&slot_options, const char *default_option)
		: bk_parallel_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		bk_parallel_devices(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}
	bk_parallel_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~bk_parallel_slot_device();

	// callbacks
	auto irq2_handler() { return m_irq2_handler.bind(); }
	auto irq3_handler() { return m_irq3_handler.bind(); }

	uint16_t read();
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void init_w();
	void irq2_w(int state) { m_irq2_handler(state); }
	void irq3_w(int state) { m_irq3_handler(state); }

protected:
	// device-level overrides
	virtual void device_start() override;

	device_bk_parallel_interface *m_cart;

private:
	devcb_write_line m_irq2_handler;
	devcb_write_line m_irq3_handler;
};

// class representing interface-specific live parallel device
class device_bk_parallel_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_bk_parallel_interface();

	virtual uint16_t io_r() { return 0xffff; }
	virtual void io_w(uint16_t data, bool word) { }

	virtual void init_w() { device_reset(); }

protected:
	device_bk_parallel_interface(const machine_config &mconfig, device_t &device);

	virtual void device_reset() { }

	bk_parallel_slot_device *m_slot;
};

// device type definition
DECLARE_DEVICE_TYPE(BK_PARALLEL_SLOT, bk_parallel_slot_device)

#endif
