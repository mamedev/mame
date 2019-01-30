// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  nubus.h - NuBus bus and card emulation

  by R. Belmont, based heavily on Miodrag Milanovic's ISA8/16 implementation

***************************************************************************/

#ifndef MAME_BUS_NUBUS_NUBUS_H
#define MAME_BUS_NUBUS_NUBUS_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NUBUS_CPU(_cputag) \
	downcast<nubus_device &>(*device).set_cputag(_cputag);

#define MCFG_NUBUS_OUT_IRQ9_CB(_devcb) \
	downcast<nubus_device &>(*device).set_out_irq9_callback(DEVCB_##_devcb);

#define MCFG_NUBUS_OUT_IRQA_CB(_devcb) \
	downcast<nubus_device &>(*device).set_out_irqa_callback(DEVCB_##_devcb);

#define MCFG_NUBUS_OUT_IRQB_CB(_devcb) \
	downcast<nubus_device &>(*device).set_out_irqb_callback(DEVCB_##_devcb);

#define MCFG_NUBUS_OUT_IRQC_CB(_devcb) \
	downcast<nubus_device &>(*device).set_out_irqc_callback(DEVCB_##_devcb);

#define MCFG_NUBUS_OUT_IRQD_CB(_devcb) \
	downcast<nubus_device &>(*device).set_out_irqd_callback(DEVCB_##_devcb);

#define MCFG_NUBUS_OUT_IRQE_CB(_devcb) \
	downcast<nubus_device &>(*device).set_out_irqe_callback(DEVCB_##_devcb);

#define MCFG_NUBUS_SLOT_ADD(_nbtag, _tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, NUBUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	downcast<nubus_slot_device &>(*device).set_nubus_slot(_nbtag, _tag);

#define MCFG_NUBUS_ONBOARD_ADD(_nbtag, _tag, _dev_type, _def_inp) \
	MCFG_DEVICE_ADD(_tag, _dev_type, 0) \
	MCFG_DEVICE_INPUT_DEFAULTS(_def_inp) \
	downcast<device_nubus_card_interface &>(*device).set_nubus_tag(_nbtag, _tag);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nubus_device;

class nubus_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	nubus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *nbtag, T &&opts, const char *dflt)
		: nubus_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_nubus_slot(nbtag, tag);
	}

	nubus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	void set_nubus_slot(const char *tag, const char *slottag) { m_nubus_tag = tag; m_nubus_slottag = slottag; }
protected:
	nubus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// configuration
	const char *m_nubus_tag, *m_nubus_slottag;
};

// device type definition
DECLARE_DEVICE_TYPE(NUBUS_SLOT, nubus_slot_device)


class device_nubus_card_interface;
// ======================> nubus_device
class nubus_device : public device_t
{
public:
	// construction/destruction
	nubus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~nubus_device() { m_device_list.detach_all(); }

	// inline configuration
	void set_cputag(const char *tag) { m_cputag = tag; }
	template <class Object> devcb_base &set_out_irq9_callback(Object &&cb) { return m_out_irq9_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irqa_callback(Object &&cb) { return m_out_irqa_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irqb_callback(Object &&cb) { return m_out_irqb_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irqc_callback(Object &&cb) { return m_out_irqc_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irqd_callback(Object &&cb) { return m_out_irqd_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irqe_callback(Object &&cb) { return m_out_irqe_cb.set_callback(std::forward<Object>(cb)); }
	auto out_irq9_callback() { return m_out_irq9_cb.bind(); }
	auto out_irqa_callback() { return m_out_irqa_cb.bind(); }
	auto out_irqb_callback() { return m_out_irqb_cb.bind(); }
	auto out_irqc_callback() { return m_out_irqc_cb.bind(); }
	auto out_irqd_callback() { return m_out_irqd_cb.bind(); }
	auto out_irqe_callback() { return m_out_irqe_cb.bind(); }

	void add_nubus_card(device_nubus_card_interface *card);
	void install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler, uint32_t mask=0xffffffff);
	void install_device(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler, uint32_t mask=0xffffffff);
	void install_device(offs_t start, offs_t end, read32_delegate rhandler, write32_delegate whandler, uint32_t mask=0xffffffff);
	void install_readonly_device(offs_t start, offs_t end, read32_delegate rhandler, uint32_t mask=0xffffffff);
	void install_writeonly_device(offs_t start, offs_t end, write32_delegate whandler, uint32_t mask=0xffffffff);
	void install_bank(offs_t start, offs_t end, const char *tag, uint8_t *data);
	void set_irq_line(int slot, int state);

	DECLARE_WRITE_LINE_MEMBER( irq9_w );
	DECLARE_WRITE_LINE_MEMBER( irqa_w );
	DECLARE_WRITE_LINE_MEMBER( irqb_w );
	DECLARE_WRITE_LINE_MEMBER( irqc_w );
	DECLARE_WRITE_LINE_MEMBER( irqd_w );
	DECLARE_WRITE_LINE_MEMBER( irqe_w );

protected:
	nubus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

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
	const char *m_cputag;
};


// device type definition
DECLARE_DEVICE_TYPE(NUBUS, nubus_device)

// ======================> device_nubus_card_interface

// class representing interface-specific live nubus card
class device_nubus_card_interface : public device_slot_card_interface
{
	friend class nubus_device;
	template <class ElementType> friend class simple_list;
public:
	// construction/destruction
	virtual ~device_nubus_card_interface();

	device_nubus_card_interface *next() const { return m_next; }

	void set_nubus_device();

	// helper functions for card devices
	void install_declaration_rom(device_t *dev, const char *romregion, bool mirror_all_mb = false, bool reverse_rom = false);
	void install_bank(offs_t start, offs_t end, const char *tag, uint8_t *data);

	uint32_t get_slotspace() { return 0xf0000000 | (m_slot<<24); }
	uint32_t get_super_slotspace() { return m_slot<<28; }

	void raise_slot_irq() { nubus().set_irq_line(m_slot, ASSERT_LINE); }
	void lower_slot_irq() { nubus().set_irq_line(m_slot, CLEAR_LINE); }

	// inline configuration
	void set_nubus_tag(const char *tag, const char *slottag) { m_nubus_tag = tag; m_nubus_slottag = slottag; }

protected:
	device_nubus_card_interface(const machine_config &mconfig, device_t &device);
	virtual void interface_pre_start() override;

	int slotno() const { assert(m_nubus); return m_slot; }
	nubus_device &nubus() { assert(m_nubus); return *m_nubus; }

private:
	nubus_device  *m_nubus;
	const char *m_nubus_tag, *m_nubus_slottag;
	int m_slot;
	std::vector<uint8_t> m_declaration_rom;
	device_nubus_card_interface *m_next;
};

#endif  // MAME_BUS_NUBUS_NUBUS_H
