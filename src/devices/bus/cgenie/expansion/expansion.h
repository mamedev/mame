// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Expansion Slot

    50-pin slot

     1  GND        26  /MREQ
     2  A8         27  /WR
     3  A7         28  /C4
     4  A6         29  (not used)
     5  A9         30  /C1
     6  A5         31  BD3
     7  A4         32  /C3
     8  A3         33  (not used)
     9  A10        34  /C2
    10  A2         35  DB6
    11  A11        36  /RD
    12  A1         37  BD4
    13  A0         38  (not used)
    14  A12        39  BD7
    15  A14        40  (not used)
    16  A13        41  BD5
    17  /RFSH      42  (not useD)
    18  A15        43  BD0
    19  /INT       44  (not used)
    20  /BUSRQ     45  BD2
    21  /NMI       46  /RESET
    22  /WAIT      47  /M1
    23  /HALT      48  /IORQ
    24  /BUSAK     49  BD1
    25  /ROMDIS    50  +5V

***************************************************************************/

#ifndef MAME_BUS_CGENIE_EXPANSION_EXPANSION_H
#define MAME_BUS_CGENIE_EXPANSION_EXPANSION_H

#pragma once

// include here so drivers don't need to
#include "carts.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_cg_exp_interface;

class cg_exp_slot_device : public device_t, public device_single_card_slot_interface<device_cg_exp_interface>
{
public:
	// construction/destruction
	cg_exp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner)
		: cg_exp_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		cg_exp_slot_carts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}
	cg_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~cg_exp_slot_device();

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }
	auto reset_handler() { return m_reset_handler.bind(); }

	// called from cart device
	void int_w(int state) { m_int_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }
	void reset_w(int state) { m_reset_handler(state); }

	required_address_space m_program;
	required_address_space m_io;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_cg_exp_interface *m_cart;

private:
	devcb_write_line m_int_handler;
	devcb_write_line m_nmi_handler;
	devcb_write_line m_reset_handler;
};

// class representing interface-specific live expansion device
class device_cg_exp_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_cg_exp_interface();

protected:
	device_cg_exp_interface(const machine_config &mconfig, device_t &device);

	cg_exp_slot_device *m_slot;
};

// device type definition
DECLARE_DEVICE_TYPE(CG_EXP_SLOT, cg_exp_slot_device)

#endif // MAME_BUS_CGENIE_EXPANSION_EXPANSION_H
