// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  nubus.h - NuBus bus and card emulation

  by R. Belmont, based heavily on Miodrag Milanovic's ISA8/16 implementation

***************************************************************************/

#ifndef MAME_BUS_NUBUS_NUBUS_H
#define MAME_BUS_NUBUS_NUBUS_H

#pragma once

#include <functional>
#include <utility>
#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nubus_device;

// ======================> device_nubus_card_interface

// class representing interface-specific live nubus card
class device_nubus_card_interface : public device_interface
{
	friend class nubus_device;
public:
	// construction/destruction
	virtual ~device_nubus_card_interface();

	// helper functions for card devices
	void install_declaration_rom(const char *romregion, bool mirror_all_mb = false, bool reverse_rom = false);
	void install_bank(offs_t start, offs_t end, void *data);
	void install_view(offs_t start, offs_t end, memory_view &view);

	uint32_t get_slotspace() { return 0xf0000000 | (m_slot<<24); }
	uint32_t get_super_slotspace() { return m_slot<<28; }

	void raise_slot_irq();
	void lower_slot_irq();

	void set_pds_slot(int slot) { m_slot = slot; }

	// inline configuration
	void set_nubus_tag(nubus_device *nubus, const char *slottag) { m_nubus = nubus; m_nubus_slottag = slottag; }

protected:
	device_nubus_card_interface(const machine_config &mconfig, device_t &device);
	virtual void interface_pre_start() override;

	int slotno() const { assert(m_nubus); return m_slot; }
	nubus_device &nubus() { assert(m_nubus); return *m_nubus; }

private:
	nubus_device *m_nubus;
	const char *m_nubus_slottag;
	int m_slot;
	std::vector<uint8_t> m_declaration_rom;
};

class nubus_slot_device : public device_t, public device_single_card_slot_interface<device_nubus_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	nubus_slot_device(const machine_config &mconfig, T &&tag, device_t *owner, const char *nbtag, U &&opts, const char *dflt)
		: nubus_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_nubus_slot(std::forward<T>(nbtag), tag);
	}

	nubus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T>
	void set_nubus_slot(T &&tag, const char *slottag)
	{
		m_nubus.set_tag(std::forward<T>(tag));
		m_nubus_slottag = slottag;
	}

protected:
	nubus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<nubus_device> m_nubus;
	const char *m_nubus_slottag;
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
	~nubus_device();

	// inline configuration
	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }
	auto out_irq9_callback() { return m_out_irq9_cb.bind(); }
	auto out_irqa_callback() { return m_out_irqa_cb.bind(); }
	auto out_irqb_callback() { return m_out_irqb_cb.bind(); }
	auto out_irqc_callback() { return m_out_irqc_cb.bind(); }
	auto out_irqd_callback() { return m_out_irqd_cb.bind(); }
	auto out_irqe_callback() { return m_out_irqe_cb.bind(); }

	void add_nubus_card(device_nubus_card_interface &card);
	template <typename R, typename W> void install_device(offs_t start, offs_t end, R rhandler, W whandler, uint32_t mask=0xffffffff);
	template <typename R> void install_readonly_device(offs_t start, offs_t end, R rhandler, uint32_t mask=0xffffffff);
	template <typename W> void install_writeonly_device(offs_t start, offs_t end, W whandler, uint32_t mask=0xffffffff);
	void install_bank(offs_t start, offs_t end, void *data);
	void install_view(offs_t start, offs_t end, memory_view &view);
	void set_irq_line(int slot, int state);
	void set_address_mask(uint32_t mask) { m_addr_mask = mask; }

	void irq9_w(int state);
	void irqa_w(int state);
	void irqb_w(int state);
	void irqc_w(int state);
	void irqd_w(int state);
	void irqe_w(int state);

protected:
	nubus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// internal state
	required_address_space m_space;

	devcb_write_line    m_out_irq9_cb;
	devcb_write_line    m_out_irqa_cb;
	devcb_write_line    m_out_irqb_cb;
	devcb_write_line    m_out_irqc_cb;
	devcb_write_line    m_out_irqd_cb;
	devcb_write_line    m_out_irqe_cb;

	std::vector<std::reference_wrapper<device_nubus_card_interface> > m_device_list;

	uint32_t m_addr_mask;
};

inline void device_nubus_card_interface::raise_slot_irq()
{
	nubus().set_irq_line(m_slot, ASSERT_LINE);
}

inline void device_nubus_card_interface::lower_slot_irq()
{
	nubus().set_irq_line(m_slot, CLEAR_LINE);
}


// device type definition
DECLARE_DEVICE_TYPE(NUBUS, nubus_device)

#endif  // MAME_BUS_NUBUS_NUBUS_H
