// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  a2eauxslot.h - Apple IIe auxiliary slot and card emulation

  by R. Belmont

***************************************************************************/

#pragma once

#ifndef __A2EAUXSLOT_H__
#define __A2EAUXSLOT_H__

#include "emu.h"
#include "a2bus.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_A2EAUXSLOT_CPU(_cputag) \
	a2eauxslot_device::static_set_cputag(*device, _cputag);

#define MCFG_A2EAUXSLOT_OUT_IRQ_CB(_devcb) \
	devcb = &a2eauxslot_device::set_out_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_A2EAUXSLOT_OUT_NMI_CB(_devcb) \
	devcb = &a2eauxslot_device::set_out_nmi_callback(*device, DEVCB_##_devcb);

#define MCFG_A2EAUXSLOT_SLOT_ADD(_nbtag, _tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, A2EAUXSLOT_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	a2eauxslot_slot_device::static_set_a2eauxslot_slot(*device, _nbtag, _tag);
#define MCFG_A2EAUXSLOT_SLOT_REMOVE(_tag)    \
	MCFG_DEVICE_REMOVE(_tag)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2eauxslot_device;

class a2eauxslot_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	a2eauxslot_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2eauxslot_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start();

	// inline configuration
	static void static_set_a2eauxslot_slot(device_t &device, const char *tag, const char *slottag);
protected:
	// configuration
	const char *m_a2eauxslot_tag, *m_a2eauxslot_slottag;
};

// device type definition
extern const device_type A2EAUXSLOT_SLOT;


class device_a2eauxslot_card_interface;

// ======================> a2eauxslot_device
class a2eauxslot_device : public device_t
{
public:
	// construction/destruction
	a2eauxslot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2eauxslot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);
	template<class _Object> static devcb_base &set_out_irq_callback(device_t &device, _Object object) { return downcast<a2eauxslot_device &>(device).m_out_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_nmi_callback(device_t &device, _Object object) { return downcast<a2eauxslot_device &>(device).m_out_nmi_cb.set_callback(object); }

	void add_a2eauxslot_card(device_a2eauxslot_card_interface *card);
	device_a2eauxslot_card_interface *get_a2eauxslot_card();

	void set_irq_line(int state);
	void set_nmi_line(int state);

	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// internal state
	cpu_device   *m_maincpu;

	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;

	device_a2eauxslot_card_interface *m_device;
	const char *m_cputag;
};


// device type definition
extern const device_type A2EAUXSLOT;

// ======================> device_a2eauxslot_card_interface

// class representing interface-specific live a2eauxslot card
class device_a2eauxslot_card_interface : public device_slot_card_interface
{
	friend class a2eauxslot_device;
public:
	// construction/destruction
	device_a2eauxslot_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_a2eauxslot_card_interface();

	virtual UINT8 read_auxram(UINT16 offset) { printf("a2eauxslot: unhandled auxram read @ %04x\n", offset); return 0xff; }
	virtual void write_auxram(UINT16 offset, UINT8 data) { printf("a2eauxslot: unhandled auxram write %02x @ %04x\n", data, offset); }
	virtual void write_c07x(address_space &space, UINT8 offset, UINT8 data) {}
	virtual UINT8 *get_vram_ptr() = 0;
	virtual UINT8 *get_auxbank_ptr() = 0;
	virtual bool allow_dhr() { return true; }

	device_a2eauxslot_card_interface *next() const { return m_next; }

	void set_a2eauxslot_device();

	void raise_slot_irq() { m_a2eauxslot->set_irq_line(ASSERT_LINE); }
	void lower_slot_irq() { m_a2eauxslot->set_irq_line(CLEAR_LINE); }
	void raise_slot_nmi() { m_a2eauxslot->set_nmi_line(ASSERT_LINE); }
	void lower_slot_nmi() { m_a2eauxslot->set_nmi_line(CLEAR_LINE); }

	// inline configuration
	static void static_set_a2eauxslot_tag(device_t &device, const char *tag, const char *slottag);
public:
	a2eauxslot_device  *m_a2eauxslot;
	const char *m_a2eauxslot_tag, *m_a2eauxslot_slottag;
	int m_slot;
	device_a2eauxslot_card_interface *m_next;
};

#endif  /* __A2EAUXSLOT_H__ */
