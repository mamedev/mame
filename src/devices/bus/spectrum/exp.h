// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        ZX Spectrum Expansion Port emulation

**********************************************************************

    Pinout:

        A15   1  A14
        A13   2  A12
         D7   3  +5v
        ¬OE   4  NC
       SLOT   5  SLOT
         D0   6  0v
         D1   7  0v
         D2   8  ¬CK
         D6   9  A0
         D5  10  A1
         D3  11  A2
         D4  12  A3
       ¬INT  13  NC
       ¬NMI  14  0v
      ¬HALT  15  ¬OE
      ¬MREQ  16  NC
      ¬IORQ  17  NC
        ¬RD  18  NC
        ¬WR  19  ¬BUSRQ
         NC  20  ¬RESET
      ¬WAIT  21  A7
       +12v  22  A6
         NC  23  A5
        ¬M1  24  A4
      ¬RFSH  25  ¬ROMCS
         A8  26  ¬BUSACK
        A10  27  A9
         NC  28  A11

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_EXP_H
#define MAME_BUS_SPECTRUM_EXP_H

#pragma once


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define SPECTRUM_EXPANSION_SLOT_TAG      "exp"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SPECTRUM_EXPANSION_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, SPECTRUM_EXPANSION_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_SPECTRUM_PASSTHRU_EXPANSION_SLOT_ADD() \
	MCFG_SPECTRUM_EXPANSION_SLOT_ADD(SPECTRUM_EXPANSION_SLOT_TAG, spectrum_expansion_devices, nullptr) \
	MCFG_SPECTRUM_EXPANSION_SLOT_IRQ_HANDLER(WRITELINE(DEVICE_SELF_OWNER, spectrum_expansion_slot_device, irq_w)) \
	MCFG_SPECTRUM_EXPANSION_SLOT_NMI_HANDLER(WRITELINE(DEVICE_SELF_OWNER, spectrum_expansion_slot_device, nmi_w))

#define MCFG_SPECTRUM_EXPANSION_SLOT_IRQ_HANDLER(_devcb) \
	devcb = &downcast<spectrum_expansion_slot_device &>(*device).set_irq_handler(DEVCB_##_devcb);

#define MCFG_SPECTRUM_EXPANSION_SLOT_NMI_HANDLER(_devcb) \
	devcb = &downcast<spectrum_expansion_slot_device &>(*device).set_nmi_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_expansion_slot_device

class device_spectrum_expansion_interface;

class spectrum_expansion_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	spectrum_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~spectrum_expansion_slot_device();

	// callbacks
	template <class Object> devcb_base &set_irq_handler(Object &&cb) { return m_irq_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_nmi_handler(Object &&cb) { return m_nmi_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( mreq_r );
	DECLARE_WRITE8_MEMBER( mreq_w );
	DECLARE_READ8_MEMBER( port_fe_r );
	DECLARE_READ_LINE_MEMBER( romcs );

	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_irq_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_nmi_handler(state); }

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	device_spectrum_expansion_interface *m_card;

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;
};


// ======================> device_spectrum_expansion_interface

class device_spectrum_expansion_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_spectrum_expansion_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(mreq_r) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(mreq_w) { }
	virtual DECLARE_READ8_MEMBER(port_fe_r) { return 0xff; }
	virtual DECLARE_READ_LINE_MEMBER(romcs) { return 0; }

protected:
	device_spectrum_expansion_interface(const machine_config &mconfig, device_t &device);

	spectrum_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_EXPANSION_SLOT, spectrum_expansion_slot_device)

void spectrum_expansion_devices(device_slot_interface &device);
void spec128_expansion_devices(device_slot_interface &device);
void specpls3_expansion_devices(device_slot_interface &device);


#endif // MAME_BUS_SPECTRUM_EXP_H
