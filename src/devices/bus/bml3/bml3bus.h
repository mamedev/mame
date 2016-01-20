// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/***************************************************************************

  bml3bus.h - Hitachi MB-6890 slot bus and card emulation

  Adapted from a2bus by Jonathan Edwards

***************************************************************************/

#pragma once

#ifndef __BML3BUS_H__
#define __BML3BUS_H__

#include "emu.h"

#define BML3BUS_MAX_SLOTS 6

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BML3BUS_CPU(_cputag) \
	bml3bus_device::static_set_cputag(*device, _cputag);

#define MCFG_BML3BUS_OUT_NMI_CB(_devcb) \
	devcb = &bml3bus_device::set_out_nmi_callback(*device, DEVCB_##_devcb);

#define MCFG_BML3BUS_OUT_IRQ_CB(_devcb) \
	devcb = &bml3bus_device::set_out_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_BML3BUS_OUT_FIRQ_CB(_devcb) \
	devcb = &bml3bus_device::set_out_firq_callback(*device, DEVCB_##_devcb);

#define MCFG_BML3BUS_SLOT_ADD(_nbtag, _tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, BML3BUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	bml3bus_slot_device::static_set_bml3bus_slot(*device, _nbtag, _tag);
#define MCFG_BML3BUS_SLOT_REMOVE(_tag)    \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_BML3BUS_ONBOARD_ADD(_nbtag, _tag, _dev_type) \
	MCFG_DEVICE_ADD(_tag, _dev_type, 0) \
	device_bml3bus_card_interface::static_set_bml3bus_tag(*device, _nbtag, _tag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bml3bus_device;

class bml3bus_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	bml3bus_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	bml3bus_slot_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	static void static_set_bml3bus_slot(device_t &device, std::string tag, std::string slottag);
protected:
	// configuration
	std::string m_bml3bus_tag;
	std::string m_bml3bus_slottag;
};

// device type definition
extern const device_type BML3BUS_SLOT;


class device_bml3bus_card_interface;
// ======================> bml3bus_device
class bml3bus_device : public device_t
{
public:
	// construction/destruction
	bml3bus_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	bml3bus_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// inline configuration
	static void static_set_cputag(device_t &device, std::string tag);
	template<class _Object> static devcb_base &set_out_nmi_callback(device_t &device, _Object object) { return downcast<bml3bus_device &>(device).m_out_nmi_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_irq_callback(device_t &device, _Object object) { return downcast<bml3bus_device &>(device).m_out_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_firq_callback(device_t &device, _Object object) { return downcast<bml3bus_device &>(device).m_out_firq_cb.set_callback(object); }

	void add_bml3bus_card(int slot, device_bml3bus_card_interface *card);
	device_bml3bus_card_interface *get_bml3bus_card(int slot);

	void set_nmi_line(int state);
	void set_irq_line(int state);
	void set_firq_line(int state);

	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( firq_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	cpu_device   *m_maincpu;

	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_firq_cb;

	device_bml3bus_card_interface *m_device_list[BML3BUS_MAX_SLOTS];
	std::string m_cputag;
};


// device type definition
extern const device_type BML3BUS;

// ======================> device_bml3bus_card_interface

// class representing interface-specific live bml3bus card
class device_bml3bus_card_interface : public device_slot_card_interface
{
	friend class bml3bus_device;
public:
	// construction/destruction
	device_bml3bus_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_bml3bus_card_interface();

	device_bml3bus_card_interface *next() const { return m_next; }

	void set_bml3bus_device();

	void raise_slot_nmi() { m_bml3bus->set_nmi_line(ASSERT_LINE); }
	void lower_slot_nmi() { m_bml3bus->set_nmi_line(CLEAR_LINE); }
	void raise_slot_irq() { m_bml3bus->set_irq_line(ASSERT_LINE); }
	void lower_slot_irq() { m_bml3bus->set_irq_line(CLEAR_LINE); }
	void raise_slot_firq() { m_bml3bus->set_firq_line(ASSERT_LINE); }
	void lower_slot_firq() { m_bml3bus->set_firq_line(CLEAR_LINE); }

	// inline configuration
	static void static_set_bml3bus_tag(device_t &device, std::string tag, std::string slottag);
public:
	bml3bus_device  *m_bml3bus;
	std::string m_bml3bus_tag;
	std::string m_bml3bus_slottag;
	int m_slot;
	device_bml3bus_card_interface *m_next;
};

#endif  /* __BML3BUS_H__ */
