// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 expansion bus emulation

**********************************************************************


**********************************************************************/

#pragma once

#ifndef __TIKI100_BUS__
#define __TIKI100_BUS__

#include "emu.h"
#include "cpu/z80/z80daisy.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define TIKI100_BUS_TAG      "tiki100bus"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TIKI100_BUS_ADD() \
	MCFG_DEVICE_ADD(TIKI100_BUS_TAG, TIKI100_BUS, 0)

#define MCFG_TIKI100_BUS_SLOT_ADD(_tag, _def_slot) \
	MCFG_DEVICE_ADD(_tag, TIKI100_BUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(tiki100_cards, _def_slot, false)


#define MCFG_TIKI100_BUS_IRQ_CALLBACK(_write) \
	devcb = &tiki100_bus_t::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_TIKI100_BUS_NMI_CALLBACK(_write) \
	devcb = &tiki100_bus_t::set_nmi_wr_callback(*device, DEVCB_##_write);

#define MCFG_TIKI100_BUS_BUSRQ_CALLBACK(_write) \
	devcb = &tiki100_bus_t::set_busrq_wr_callback(*device, DEVCB_##_write);

#define MCFG_TIKI100_BUS_IN_MREQ_CALLBACK(_read) \
	devcb = &tiki100_bus_t::set_mrq_rd_callback(*device, DEVCB_##_read);

#define MCFG_TIKI100_BUS_OUT_MREQ_CALLBACK(_write) \
	devcb = &tiki100_bus_t::set_mrq_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tiki100_bus_slot_t

class tiki100_bus_t;
class tiki100_bus_slot_t;


// ======================> device_tiki100bus_card_interface

class device_tiki100bus_card_interface : public device_slot_card_interface
{
	friend class tiki100_bus_t;

public:
	// construction/destruction
	device_tiki100bus_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_tiki100bus_card_interface() { }

	device_tiki100bus_card_interface *next() const { return m_next; }

	// memory access
	virtual UINT8 mrq_r(address_space &space, offs_t offset, UINT8 data, bool &mdis) { mdis = 1; return data; };
	virtual void mrq_w(address_space &space, offs_t offset, UINT8 data) { };

	// I/O access
	virtual UINT8 iorq_r(address_space &space, offs_t offset, UINT8 data) { return data; };
	virtual void iorq_w(address_space &space, offs_t offset, UINT8 data) { };

	virtual void busak_w(int state) { m_busak = state; };

	// Z80 daisy chain
	virtual int z80daisy_irq_state() { return 0; }
	virtual int z80daisy_irq_ack() { return 0; }
	virtual void z80daisy_irq_reti() { }

	tiki100_bus_t  *m_bus;
	tiki100_bus_slot_t *m_slot;
	int m_busak;

	device_tiki100bus_card_interface *m_next;
};


// ======================> tiki100_bus_slot_t

class tiki100_bus_slot_t : public device_t,
							public device_slot_interface,
							public device_z80daisy_interface
{
public:
	// construction/destruction
	tiki100_bus_slot_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

protected:
	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() { return get_card_device() ? m_card->z80daisy_irq_state() : 0; }
	virtual int z80daisy_irq_ack() { return get_card_device() ? m_card->z80daisy_irq_ack() : 0; }
	virtual void z80daisy_irq_reti() { if (get_card_device()) m_card->z80daisy_irq_reti(); }

private:
	// configuration
	tiki100_bus_t  *m_bus;
	device_tiki100bus_card_interface *m_card;
};


// device type definition
extern const device_type TIKI100_BUS_SLOT;


// ======================> tiki100_bus_t

class tiki100_bus_t : public device_t
{
public:
	// construction/destruction
	tiki100_bus_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tiki100_bus_t() { m_device_list.detach_all(); }

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<tiki100_bus_t &>(device).m_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_nmi_wr_callback(device_t &device, _Object object) { return downcast<tiki100_bus_t &>(device).m_nmi_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_busrq_wr_callback(device_t &device, _Object object) { return downcast<tiki100_bus_t &>(device).m_busrq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_mrq_rd_callback(device_t &device, _Object object) { return downcast<tiki100_bus_t &>(device).m_in_mrq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_mrq_wr_callback(device_t &device, _Object object) { return downcast<tiki100_bus_t &>(device).m_out_mrq_cb.set_callback(object); }

	void add_card(device_tiki100bus_card_interface *card);

	// computer interface
	UINT8 mrq_r(address_space &space, offs_t offset, UINT8 data, bool &mdis);
	DECLARE_WRITE8_MEMBER( mrq_w );

	UINT8 iorq_r(address_space &space, offs_t offset, UINT8 data);
	DECLARE_WRITE8_MEMBER( iorq_w );

	DECLARE_WRITE_LINE_MEMBER( busak_w );

	// peripheral interface
	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_irq_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_nmi_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( busrq_w ) { m_busrq_cb(state); }
	DECLARE_READ8_MEMBER( exin_mrq_r ) { return m_in_mrq_cb(offset); }
	DECLARE_WRITE8_MEMBER( exin_mrq_w ) { m_out_mrq_cb(offset, data); }

protected:
	// device-level overrides
	virtual void device_start();

private:
	devcb_write_line   m_irq_cb;
	devcb_write_line   m_nmi_cb;
	devcb_write_line   m_busrq_cb;
	devcb_read8        m_in_mrq_cb;
	devcb_write8       m_out_mrq_cb;

	simple_list<device_tiki100bus_card_interface> m_device_list;
};


// device type definition
extern const device_type TIKI100_BUS;




SLOT_INTERFACE_EXTERN( tiki100_cards );



#endif
