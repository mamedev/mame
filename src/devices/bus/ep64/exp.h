// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Enterprise Sixty Four / One Two Eight Expansion Bus emulation

**********************************************************************

             LH SOUND IN    B1      A1      RH SOUND IN
                     _WR    B2      A2      _RFSH
                   _IORQ    B3      A3      _RD
                   +17V*    B4      A4      +17V*
                    _NMI    B5      A5      _MREQ
                      A9    B6      A6      A8
                     A11    B7      A7      A10
                     A13    B8      A8      A12
                     A15    B9      A9      A14
                      A1    B10     A10     A0
                      A3    B11     A11     A2
                      A5    B12     A12     A4
                      A7    B13     A13     A6
                      D1    B14     A14     D0
                      D3    B15     A15     D2
                      D5    B16     A16     D4
                      D7    B17     A17     D6
                    _INT    B18     A18     _RESET
                     GND    B19     A19     _WAIT
                     GND    B20     A20     _M1
                     GND    B21     A21     1M
                     GND    B22     A22     phi
                     GND    B23     A23     8M
                     EC1    B24     A24     EC0
                     EC3    B25     A25     EC2
                     A16    B26     A26     _EXTC
                     A18    B27     A27     A17
                     A20    B28     A28     A19
                     14M    B29     A29     A21
                   VSYNC    B30     A30     _LOCATE
                    _EXP    B31     A31     GND
                     GND    B32     A32     HSYNC
                     +9V    B33     A33     +9V

**********************************************************************/

#pragma once

#ifndef __EP64_EXPANSION_BUS__
#define __EP64_EXPANSION_BUS__

#include "emu.h"
#include "audio/dave.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define EP64_EXPANSION_BUS_TAG  "exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EP64_EXPANSION_BUS_SLOT_ADD(_tag, _def_slot) \
	MCFG_DEVICE_ADD(_tag, EP64_EXPANSION_BUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(ep64_expansion_bus_cards, _def_slot, false)

#define MCFG_EP64_EXPANSION_BUS_SLOT_DAVE(_tag) \
	ep64_expansion_bus_slot_device::static_set_dave_tag(*device, "^" _tag);

#define MCFG_EP64_EXPANSION_BUS_SLOT_IRQ_CALLBACK(_write) \
	devcb = &ep64_expansion_bus_slot_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_EP64_EXPANSION_BUS_SLOT_NMI_CALLBACK(_write) \
	devcb = &ep64_expansion_bus_slot_device::set_nmi_wr_callback(*device, DEVCB_##_write);

#define MCFG_EP64_EXPANSION_BUS_SLOT_WAIT_CALLBACK(_write) \
	devcb = &ep64_expansion_bus_slot_device::set_wait_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ep64_expansion_bus_slot_device

class device_ep64_expansion_bus_card_interface;

class ep64_expansion_bus_slot_device : public device_t,
										public device_slot_interface
{
	friend class device_ep64_expansion_bus_card_interface;

public:
	// construction/destruction
	ep64_expansion_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_dave_tag(device_t &device, const char* tag) { downcast<ep64_expansion_bus_slot_device &>(device).m_dave.set_tag(tag); }
	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<ep64_expansion_bus_slot_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_nmi_wr_callback(device_t &device, _Object object) { return downcast<ep64_expansion_bus_slot_device &>(device).m_write_nmi.set_callback(object); }
	template<class _Object> static devcb_base &set_wait_wr_callback(device_t &device, _Object object) { return downcast<ep64_expansion_bus_slot_device &>(device).m_write_wait.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_write_nmi(state); }
	DECLARE_WRITE_LINE_MEMBER( wait_w ) { m_write_wait(state); }

	address_space& program() { return m_dave->space(AS_PROGRAM); }
	address_space& io() { return m_dave->space(AS_IO); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_write_irq;
	devcb_write_line m_write_nmi;
	devcb_write_line m_write_wait;

	required_device<dave_device> m_dave;

	device_ep64_expansion_bus_card_interface *m_card;
};


// ======================> device_ep64_expansion_bus_card_interface

class device_ep64_expansion_bus_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_ep64_expansion_bus_card_interface(const machine_config &mconfig, device_t &device);

protected:
	ep64_expansion_bus_slot_device  *m_slot;
};


// device type definition
extern const device_type EP64_EXPANSION_BUS_SLOT;


SLOT_INTERFACE_EXTERN( ep64_expansion_bus_cards );



#endif
