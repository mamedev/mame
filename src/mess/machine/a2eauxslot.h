/***************************************************************************

  a2eauxslot.h - Apple IIe auxiliary slot and card emulation

  by R. Belmont

***************************************************************************/

#pragma once

#ifndef __A2EAUXSLOT_H__
#define __A2EAUXSLOT_H__

#include "emu.h"
#include "machine/a2bus.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_A2EAUXSLOT_BUS_ADD(_tag, _cputag, _config) \
	MCFG_DEVICE_ADD(_tag, A2EAUXSLOT, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	a2eauxslot_device::static_set_cputag(*device, _cputag);
#define MCFG_A2EAUXSLOT_SLOT_ADD(_nbtag, _tag, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, A2EAUXSLOT_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	a2eauxslot_slot_device::static_set_a2eauxslot_slot(*device, _nbtag, _tag);
#define MCFG_A2EAUXSLOT_SLOT_REMOVE(_tag)    \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_A2EAUXSLOT_BUS_REMOVE(_tag) \
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
	a2eauxslot_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

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

// ======================> a2eauxslot_interface

struct a2eauxslot_interface
{
	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;
};

class device_a2eauxslot_card_interface;
// ======================> a2eauxslot_device
class a2eauxslot_device : public device_t,
					public a2eauxslot_interface
{
public:
	// construction/destruction
	a2eauxslot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2eauxslot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);

	void add_a2eauxslot_card(device_a2eauxslot_card_interface *card);
	device_a2eauxslot_card_interface *get_a2eauxslot_card(int slot);

	void set_irq_line(int state);
	void set_nmi_line(int state);

	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	// internal state
	cpu_device   *m_maincpu;

	devcb_resolved_write_line   m_out_irq_func;
	devcb_resolved_write_line   m_out_nmi_func;

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

	virtual void write_c07x(address_space &space, UINT8 offset, UINT8 data) { printf("a2eauxslot: unhandled write %02x to C07%x\n", data, offset); }

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
