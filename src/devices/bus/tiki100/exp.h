// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 expansion bus emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_TIKI100_EXP_H
#define MAME_BUS_TIKI100_EXP_H

#pragma once

#include "machine/z80daisy.h"



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
	downcast<tiki100_bus_device &>(*device).set_irq_wr_callback(DEVCB_##_write);

#define MCFG_TIKI100_BUS_NMI_CALLBACK(_write) \
	downcast<tiki100_bus_device &>(*device).set_nmi_wr_callback(DEVCB_##_write);

#define MCFG_TIKI100_BUS_BUSRQ_CALLBACK(_write) \
	downcast<tiki100_bus_device &>(*device).set_busrq_wr_callback(DEVCB_##_write);

#define MCFG_TIKI100_BUS_IN_MREQ_CALLBACK(_read) \
	downcast<tiki100_bus_device &>(*device).set_mrq_rd_callback(DEVCB_##_read);

#define MCFG_TIKI100_BUS_OUT_MREQ_CALLBACK(_write) \
	downcast<tiki100_bus_device &>(*device).set_mrq_wr_callback(DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tiki100_bus_slot_device

class tiki100_bus_device;
class tiki100_bus_slot_device;


// ======================> device_tiki100bus_card_interface

class device_tiki100bus_card_interface : public device_slot_card_interface
{
	friend class tiki100_bus_device;
	template <class ElementType> friend class simple_list;

public:
	device_tiki100bus_card_interface *next() const { return m_next; }

	// memory access
	virtual uint8_t mrq_r(address_space &space, offs_t offset, uint8_t data, bool &mdis) { mdis = 1; return data; }
	virtual void mrq_w(address_space &space, offs_t offset, uint8_t data) { }

	// I/O access
	virtual uint8_t iorq_r(address_space &space, offs_t offset, uint8_t data) { return data; }
	virtual void iorq_w(address_space &space, offs_t offset, uint8_t data) { }

	virtual void busak_w(int state) { m_busak = state; }

	// Z80 daisy chain
	virtual int z80daisy_irq_state() { return 0; }
	virtual int z80daisy_irq_ack() { return 0; }
	virtual void z80daisy_irq_reti() { }

protected:
	// construction/destruction
	device_tiki100bus_card_interface(const machine_config &mconfig, device_t &device);

	tiki100_bus_device  *m_bus;
	tiki100_bus_slot_device *m_slot;
	int m_busak;

private:
	device_tiki100bus_card_interface *m_next;
};


// ======================> tiki100_bus_slot_device

class tiki100_bus_slot_device : public device_t,
							public device_slot_interface,
							public device_z80daisy_interface
{
public:
	// construction/destruction
	tiki100_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

protected:
	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override { return get_card_device() ? m_card->z80daisy_irq_state() : 0; }
	virtual int z80daisy_irq_ack() override { return get_card_device() ? m_card->z80daisy_irq_ack() : 0; }
	virtual void z80daisy_irq_reti() override { if (get_card_device()) m_card->z80daisy_irq_reti(); }

private:
	// configuration
	tiki100_bus_device  *m_bus;
	device_tiki100bus_card_interface *m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(TIKI100_BUS_SLOT, tiki100_bus_slot_device)


// ======================> tiki100_bus_device

class tiki100_bus_device : public device_t
{
public:
	// construction/destruction
	tiki100_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tiki100_bus_device() { m_device_list.detach_all(); }

	template <class Object> devcb_base &set_irq_wr_callback(Object &&cb) { return m_irq_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_nmi_wr_callback(Object &&cb) { return m_nmi_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_busrq_wr_callback(Object &&cb) { return m_busrq_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_mrq_rd_callback(Object &&cb) { return m_in_mrq_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_mrq_wr_callback(Object &&cb) { return m_out_mrq_cb.set_callback(std::forward<Object>(cb)); }

	void add_card(device_tiki100bus_card_interface *card);

	// computer interface
	uint8_t mrq_r(address_space &space, offs_t offset, uint8_t data, bool &mdis);
	DECLARE_WRITE8_MEMBER( mrq_w );

	uint8_t iorq_r(address_space &space, offs_t offset, uint8_t data);
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
	virtual void device_start() override;

private:
	devcb_write_line   m_irq_cb;
	devcb_write_line   m_nmi_cb;
	devcb_write_line   m_busrq_cb;
	devcb_read8        m_in_mrq_cb;
	devcb_write8       m_out_mrq_cb;

	simple_list<device_tiki100bus_card_interface> m_device_list;
};


// device type definition
DECLARE_DEVICE_TYPE(TIKI100_BUS, tiki100_bus_device)



void tiki100_cards(device_slot_interface &device);

#endif // MAME_BUS_TIKI100_EXP_H
