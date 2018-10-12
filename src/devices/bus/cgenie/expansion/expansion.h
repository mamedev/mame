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



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CG_EXP_SLOT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CG_EXP_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(cg_exp_slot_carts, nullptr, false)

#define MCFG_CG_EXP_SLOT_INT_HANDLER(_devcb) \
	downcast<cg_exp_slot_device &>(*device).set_int_handler(DEVCB_##_devcb);

#define MCFG_CG_EXP_SLOT_NMI_HANDLER(_devcb) \
	downcast<cg_exp_slot_device &>(*device).set_nmi_handler(DEVCB_##_devcb);

#define MCFG_CG_EXP_SLOT_RESET_HANDLER(_devcb) \
	downcast<cg_exp_slot_device &>(*device).set_reset_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_cg_exp_interface;

class cg_exp_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	cg_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~cg_exp_slot_device();

	void set_program_space(address_space *program);
	void set_io_space(address_space *io);

	// callbacks
	template <class Object> devcb_base &set_int_handler(Object &&cb) { return m_int_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_nmi_handler(Object &&cb) { return m_nmi_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_reset_handler(Object &&cb) { return m_reset_handler.set_callback(std::forward<Object>(cb)); }

	// called from cart device
	DECLARE_WRITE_LINE_MEMBER( int_w ) { m_int_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_nmi_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( reset_w ) { m_reset_handler(state); }

	address_space *m_program;
	address_space *m_io;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	device_cg_exp_interface *m_cart;

private:
	devcb_write_line m_int_handler;
	devcb_write_line m_nmi_handler;
	devcb_write_line m_reset_handler;
};

// class representing interface-specific live expansion device
class device_cg_exp_interface : public device_slot_card_interface
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

// include here so drivers don't need to
#include "carts.h"

#endif // MAME_BUS_CGENIE_EXPANSION_EXPANSION_H
