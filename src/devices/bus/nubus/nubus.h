// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  nubus.h - NuBus bus and card emulation

  by R. Belmont, based heavily on Miodrag Milanovic's ISA8/16 implementation

***************************************************************************/

#pragma once

#ifndef __NUBUS_H__
#define __NUBUS_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NUBUS_CPU(_cputag) \
	nubus_device::static_set_cputag(*device, _cputag);

#define MCFG_NUBUS_OUT_IRQ9_CB(_devcb) \
	devcb = &nubus_device::set_out_irq9_callback(*device, DEVCB_##_devcb);

#define MCFG_NUBUS_OUT_IRQA_CB(_devcb) \
	devcb = &nubus_device::set_out_irqa_callback(*device, DEVCB_##_devcb);

#define MCFG_NUBUS_OUT_IRQB_CB(_devcb) \
	devcb = &nubus_device::set_out_irqb_callback(*device, DEVCB_##_devcb);

#define MCFG_NUBUS_OUT_IRQC_CB(_devcb) \
	devcb = &nubus_device::set_out_irqc_callback(*device, DEVCB_##_devcb);

#define MCFG_NUBUS_OUT_IRQD_CB(_devcb) \
	devcb = &nubus_device::set_out_irqd_callback(*device, DEVCB_##_devcb);

#define MCFG_NUBUS_OUT_IRQE_CB(_devcb) \
	devcb = &nubus_device::set_out_irqe_callback(*device, DEVCB_##_devcb);

#define MCFG_NUBUS_SLOT_ADD(_nbtag, _tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, NUBUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	nubus_slot_device::static_set_nubus_slot(*device, _nbtag, _tag);
#define MCFG_NUBUS_SLOT_REMOVE(_tag)    \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_NUBUS_ONBOARD_ADD(_nbtag, _tag, _dev_type, _def_inp) \
	MCFG_DEVICE_ADD(_tag, _dev_type, 0) \
	MCFG_DEVICE_INPUT_DEFAULTS(_def_inp) \
	device_nubus_card_interface::static_set_nubus_tag(*device, _nbtag, _tag);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nubus_device;

class nubus_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	nubus_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	nubus_slot_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	static void static_set_nubus_slot(device_t &device, std::string tag, std::string slottag);
protected:
	// configuration
	std::string m_nubus_tag;
	std::string m_nubus_slottag;
};

// device type definition
extern const device_type NUBUS_SLOT;


class device_nubus_card_interface;
// ======================> nubus_device
class nubus_device : public device_t
{
public:
	// construction/destruction
	nubus_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	nubus_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	~nubus_device() { m_device_list.detach_all(); }

	// inline configuration
	static void static_set_cputag(device_t &device, std::string tag);
	template<class _Object> static devcb_base &set_out_irq9_callback(device_t &device, _Object object) { return downcast<nubus_device &>(device).m_out_irq9_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_irqa_callback(device_t &device, _Object object) { return downcast<nubus_device &>(device).m_out_irqa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_irqb_callback(device_t &device, _Object object) { return downcast<nubus_device &>(device).m_out_irqb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_irqc_callback(device_t &device, _Object object) { return downcast<nubus_device &>(device).m_out_irqc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_irqd_callback(device_t &device, _Object object) { return downcast<nubus_device &>(device).m_out_irqd_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_irqe_callback(device_t &device, _Object object) { return downcast<nubus_device &>(device).m_out_irqe_cb.set_callback(object); }

	void add_nubus_card(device_nubus_card_interface *card);
	void install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler, UINT32 mask=0xffffffff);
	void install_device(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler, UINT32 mask=0xffffffff);
	void install_device(offs_t start, offs_t end, read32_delegate rhandler, write32_delegate whandler, UINT32 mask=0xffffffff);
	void install_readonly_device(offs_t start, offs_t end, read32_delegate rhandler, UINT32 mask=0xffffffff);
	void install_writeonly_device(offs_t start, offs_t end, write32_delegate whandler, UINT32 mask=0xffffffff);
	void install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, std::string tag, UINT8 *data);
	void set_irq_line(int slot, int state);

	DECLARE_WRITE_LINE_MEMBER( irq9_w );
	DECLARE_WRITE_LINE_MEMBER( irqa_w );
	DECLARE_WRITE_LINE_MEMBER( irqb_w );
	DECLARE_WRITE_LINE_MEMBER( irqc_w );
	DECLARE_WRITE_LINE_MEMBER( irqd_w );
	DECLARE_WRITE_LINE_MEMBER( irqe_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	cpu_device   *m_maincpu;

	devcb_write_line    m_out_irq9_cb;
	devcb_write_line    m_out_irqa_cb;
	devcb_write_line    m_out_irqb_cb;
	devcb_write_line    m_out_irqc_cb;
	devcb_write_line    m_out_irqd_cb;
	devcb_write_line    m_out_irqe_cb;

	simple_list<device_nubus_card_interface> m_device_list;
	std::string m_cputag;
};


// device type definition
extern const device_type NUBUS;

// ======================> device_nubus_card_interface

// class representing interface-specific live nubus card
class device_nubus_card_interface : public device_slot_card_interface
{
	friend class nubus_device;
public:
	// construction/destruction
	device_nubus_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_nubus_card_interface();

	device_nubus_card_interface *next() const { return m_next; }

	void set_nubus_device();

	// helper functions for card devices
	void install_declaration_rom(device_t *dev, const char *romregion, bool mirror_all_mb = false, bool reverse_rom = false);
	void install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, std::string tag, UINT8 *data);

	UINT32 get_slotspace() { return 0xf0000000 | (m_slot<<24); }
	UINT32 get_super_slotspace() { return m_slot<<28; }

	void raise_slot_irq() { m_nubus->set_irq_line(m_slot, ASSERT_LINE); }
	void lower_slot_irq() { m_nubus->set_irq_line(m_slot, CLEAR_LINE); }

	// inline configuration
	static void static_set_nubus_tag(device_t &device, std::string tag, std::string slottag);
public:
	nubus_device  *m_nubus;
	std::string m_nubus_tag;
	std::string m_nubus_slottag;
	int m_slot;
	device_nubus_card_interface *m_next;
	dynamic_buffer m_declaration_rom;
};

#endif  /* __NUBUS_H__ */
