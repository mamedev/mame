// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Parallel Slot

    20-pin slot

     1  GND        11  IOB2
     2  +12V       12  IOB1
     3  IOA3       13  IOB0
     4  IOA4       14  IOB3
     5  IOA0       15  IOB4
     6  IOA5       16  IOB5
     7  IOA1       17  IOB7
     8  IOA2       18  IOB6
     9  IOA7       19  +5V
    10  IOA6       20  -12V

***************************************************************************/

#ifndef MAME_BUS_CGENIE_PARALLEL_PARALLEL_H
#define MAME_BUS_CGENIE_PARALLEL_PARALLEL_H

#pragma once

// include here so drivers don't need to
#include "carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_cg_parallel_interface;

class cg_parallel_slot_device : public device_t, public device_single_card_slot_interface<device_cg_parallel_interface>
{
public:
	// construction/destruction
	cg_parallel_slot_device(machine_config const &mconfig, char const *tag, device_t *owner)
		: cg_parallel_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		cg_parallel_slot_carts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}
	cg_parallel_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~cg_parallel_slot_device();

	// IOA
	uint8_t pa_r();
	void pa_w(uint8_t data);

	// IOB
	uint8_t pb_r();
	void pb_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_cg_parallel_interface *m_cart;
};

// class representing interface-specific live parallel device
class device_cg_parallel_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_cg_parallel_interface();

	virtual uint8_t pa_r() { return 0xff; }
	virtual void pa_w(uint8_t data) { }

	virtual uint8_t pb_r() { return 0xff; }
	virtual void pb_w(uint8_t data) { }

protected:
	device_cg_parallel_interface(const machine_config &mconfig, device_t &device);

	cg_parallel_slot_device *m_slot;
};

// device type definition
DECLARE_DEVICE_TYPE(CG_PARALLEL_SLOT, cg_parallel_slot_device)

#endif // MAME_BUS_CGENIE_PARALLEL_PARALLEL_H
