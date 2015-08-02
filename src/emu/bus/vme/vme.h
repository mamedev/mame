// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edström

/*
 * vme.h
 *
 * VME bus system
 *
 * Pinout: (from http://pinouts.ru/Slots/vmebus_pinout.shtml)

     P1/J1                                   P2/J2 (optional for 32 bit)
    +-A-B-C--+  A        B         C        +-A-B-C--+  A     B        C
 01 | [][][] | D00      BBSY*     D08       | [][][] | n/a   +5v      n/a
 02 | [][][] | D01      BCLR*     D09       | [][][] | n/a   GROUND   n/a
 03 | [][][] | D02      ACFAIL*   D10       | [][][] | n/a   RESERVED n/a
 04 | [][][] | D03      BG0IN*    D11       | [][][] | n/a   A24      n/a
 05 | [][][] | D04      BG0OUT*   D12       | [][][] | n/a   A25      n/a
 06 | [][][] | D05      BG1IN*    D13       | [][][] | n/a   A26      n/a
 07 | [][][] | D06      BG1OUT*   D14       | [][][] | n/a   A27      n/a
 08 | [][][] | D07      BG2IN*    D15       | [][][] | n/a   A28      n/a
 09 | [][][] | GROUND   BG2OUT*   GROUND    | [][][] | n/a   A29      n/a
 10 | [][][] | SYSCLK   BG3IN*    SYSFAIL*  | [][][] | n/a   A30      n/a
 11 | [][][] | GROUND   BG3OUT*   BERR*     | [][][] | n/a   A31      n/a
 12 | [][][] | DS1*     BR0*      SYSRESET* | [][][] | n/a   GROUND   n/a
 13 | [][][] | DS0*     BR1*      LWORD*    | [][][] | n/a   +5v      n/a
 14 | [][][] | WRITE*   BR2*      AM5       | [][][] | n/a   D16      n/a
 15 | [][][] | GROUND   BR3*      A23       | [][][] | n/a   D17      n/a
 16 | [][][] | DTACK*   AM0       A22       | [][][] | n/a   D18      n/a
 17 | [][][] | GROUND   AM1       A21       | [][][] | n/a   D19      n/a
 18 | [][][] | AS*      AM2       A20       | [][][] | n/a   D20      n/a
 19 | [][][] | GROUND   AM3       A19       | [][][] | n/a   D21      n/a
 20 | [][][] | IACK*    GROUND    A18       | [][][] | n/a   D22      n/a
 21 | [][][] | IACKIN*  SERCLK*   A17       | [][][] | n/a   D23      n/a
 22 | [][][] | IACKOUT* SERDAT*   A16       | [][][] | n/a   GROUND   n/a
 23 | [][][] | AM4      GROUND    A15       | [][][] | n/a   D24      n/a
 24 | [][][] | A07      IRQ7*     A14       | [][][] | n/a   D25      n/a
 25 | [][][] | A06      IRQ6*     A13       | [][][] | n/a   D26      n/a
 26 | [][][] | A05      IRQ5*     A12       | [][][] | n/a   D27      n/a
 27 | [][][] | A04      IRQ4*     A11       | [][][] | n/a   D28      n/a
 28 | [][][] | A03      IRQ3*     A10       | [][][] | n/a   D29      n/a
 29 | [][][] | A02      IRQ2*     A09       | [][][] | n/a   D30      n/a
 30 | [][][] | A01      IRQ1*     A08       | [][][] | n/a   D31      n/a
 31 | [][][] | -12v     +5v STDBY +12v      | [][][] | n/a   GROUND   n/a
 32 | [][][] | +5v      +5v       +5v       | [][][] | n/a   +5v      n/a

 */

#ifndef VME_H_
#define VME_H_

#include "emu.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VME_EXP_SLOT_TAG       "vmeexp"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VME_EXPANSION_SLOT_OUT_IRQ2_CB(_devcb) \
	devcb = &vme_expansion_slot_device::set_out_irq2_callback(*device, DEVCB_##_devcb);

#define MCFG_VME_EXPANSION_SLOT_OUT_IRQ4_CB(_devcb) \
	devcb = &vme_expansion_slot_device::set_out_irq4_callback(*device, DEVCB_##_devcb);

#define MCFG_VME_EXPANSION_SLOT_OUT_NMI_CB(_devcb) \
	devcb = &vme_expansion_slot_device::set_out_nmi_callback(*device, DEVCB_##_devcb);

#define MCFG_VME_EXPANSION_SLOT_OUT_RESET_CB(_devcb) \
	devcb = &vme_expansion_slot_device::set_out_reset_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_vme_expansion_card_interface

// class representing interface-specific live vme_expansion card
class device_vme_expansion_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_vme_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_vme_expansion_card_interface();

	// reset
	virtual void vme_reset_w() { };
};


// ======================> vme_expansion_slot_device

class vme_expansion_slot_device : public device_t,
									public device_slot_interface
{
public:
	// construction/destruction
	vme_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~vme_expansion_slot_device();

	template<class _Object> static devcb_base &set_out_irq2_callback(device_t &device, _Object object) { return downcast<vme_expansion_slot_device &>(device).m_out_irq2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_irq4_callback(device_t &device, _Object object) { return downcast<vme_expansion_slot_device &>(device).m_out_irq4_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_nmi_callback(device_t &device, _Object object) { return downcast<vme_expansion_slot_device &>(device).m_out_nmi_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_reset_callback(device_t &device, _Object object) { return downcast<vme_expansion_slot_device &>(device).m_out_reset_cb.set_callback(object); }


	DECLARE_WRITE_LINE_MEMBER( irq2_w );
	DECLARE_WRITE_LINE_MEMBER( irq4_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	devcb_write_line    m_out_irq2_cb;
	devcb_write_line    m_out_irq4_cb;
	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_reset_cb;

	device_vme_expansion_card_interface *m_card;
};


// device type definition
extern const device_type VME_EXPANSION_SLOT;

#endif /* VME_H_ */
