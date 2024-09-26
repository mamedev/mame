// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  a2bus.h - Apple II slot bus and card emulation

  by R. Belmont

***************************************************************************/

#ifndef MAME_BUS_A2BUS_A2BUS_H
#define MAME_BUS_A2BUS_A2BUS_H

#pragma once

#include <utility>


// /INH special addresses
#define INH_START_INVALID   0xffff
#define INH_END_INVALID     0x0000

// /INH types
#define INH_NONE            0x00
#define INH_READ            0x01
#define INH_WRITE           0x02

// 7M = XTAL(14'318'181) / 2 or XTAL(28'636'363) / 4 (for IIgs)
static constexpr uint32_t A2BUS_7M_CLOCK = 7159090;

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_device;
class device_a2bus_card_interface;

class a2bus_slot_device : public device_t, public device_single_card_slot_interface<device_a2bus_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	a2bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&a2bus_tag, U &&opts, const char *dflt)
		: a2bus_slot_device(mconfig, tag, owner, A2BUS_7M_CLOCK, std::forward<T>(a2bus_tag), std::forward<U>(opts), dflt)
	{
	}
	template <typename T, typename U>
	a2bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&a2bus_tag, U &&opts, const char *dflt)
		: a2bus_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_a2bus.set_tag(std::forward<T>(a2bus_tag));
	}
	a2bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = A2BUS_7M_CLOCK);

protected:
	a2bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<a2bus_device> m_a2bus;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_SLOT, a2bus_slot_device)


// ======================> a2bus_device
class a2bus_device : public device_t
{
	// multi-card devices need to access m_device_list, so they get friended here.
	friend class a2bus_mcms2_device;
public:
	// construction/destruction
	a2bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_space(T &&tag, int spacenum) { m_maincpu_space.set_tag(std::forward<T>(tag), spacenum); }
	auto irq_w() { return m_out_irq_cb.bind(); }
	auto nmi_w() { return m_out_nmi_cb.bind(); }
	auto inh_w() { return m_out_inh_cb.bind(); }
	auto dma_w() { return m_out_dma_cb.bind(); }

	void add_a2bus_card(int slot, device_a2bus_card_interface *card);
	device_a2bus_card_interface *get_a2bus_card(int slot);
	uint8_t get_a2bus_irq_mask();
	uint8_t get_a2bus_nmi_mask();

	void set_irq_line(int state, int slot);
	void set_nmi_line(int state, int slot);
	void set_dma_line(int state);
	void recalc_inh(int slot);
	uint8_t dma_r(uint16_t offset);
	void dma_w(uint16_t offset, uint8_t data);
	void reset_bus();

	void irq_w(int state);
	void nmi_w(int state);

protected:
	a2bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal state
	required_address_space m_maincpu_space;

	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;
	devcb_write8        m_out_inh_cb;
	devcb_write_line    m_out_dma_cb;

	device_a2bus_card_interface *m_device_list[8];

	uint8_t m_slot_irq_mask;
	uint8_t m_slot_nmi_mask;
};


// device type definition
DECLARE_DEVICE_TYPE(A2BUS, a2bus_device)

// ======================> device_a2bus_card_interface

// class representing interface-specific live a2bus card
class device_a2bus_card_interface : public device_interface
{
	friend class a2bus_device;
public:
	// construction/destruction
	virtual ~device_a2bus_card_interface();

	virtual uint8_t read_c0nx(uint8_t offset) { device().logerror("a2bus: unhandled read at C0n%x\n", offset); return 0; }       // C0nX - /DEVSEL
	virtual void write_c0nx(uint8_t offset, uint8_t data) { device().logerror("a2bus: unhandled write %02x to C0n%x\n", data, offset); }
	virtual uint8_t read_cnxx(uint8_t offset) { return 0; }       // CnXX - /IOSEL
	virtual void write_cnxx(uint8_t offset, uint8_t data) { device().logerror("a2bus: unhandled write %02x to Cn%02x\n", data, offset); }
	virtual uint8_t read_c800(uint16_t offset) { return 0; }      // C800 - /IOSTB
	virtual void write_c800(uint16_t offset, uint8_t data) {device().logerror("a2bus: unhandled write %02x to %04x\n", data, offset + 0xc800); }
	virtual bool take_c800() { return true; }   // override and return false if your card doesn't take over the c800 space
	virtual uint8_t read_inh_rom(uint16_t offset) { return 0; }
	virtual void write_inh_rom(uint16_t offset, uint8_t data) { }
	virtual uint16_t inh_start() { return INH_START_INVALID; }
	virtual uint16_t inh_end() { return INH_END_INVALID; }
	virtual bool inh_check(uint16_t offset, bool bIsWrite) { return false; }
	virtual int inh_type() { return INH_NONE; }
	virtual void bus_reset() { }

	device_a2bus_card_interface *next() const { return m_next; }

	// inline configuration
	void set_a2bus(a2bus_device *a2bus, const char *slottag) { m_a2bus = a2bus; m_a2bus_slottag = slottag; }
	template <typename T> void set_onboard(T &&a2bus) { m_a2bus_finder.set_tag(std::forward<T>(a2bus)); m_a2bus_slottag = device().tag(); }

	uint8_t slot_dma_read(uint16_t offset) { return m_a2bus->dma_r(offset); }
	void slot_dma_write(uint16_t offset, uint8_t data) { m_a2bus->dma_w(offset, data); }

protected:
	uint32_t get_slotromspace() { return 0xc000 | (m_slot<<8); }      // return Cn00 address for this slot
	uint32_t get_slotiospace() { return 0xc080 + (m_slot<<4); }       // return C0n0 address for this slot

	void raise_slot_irq() { m_a2bus->set_irq_line(ASSERT_LINE, m_slot); }
	void lower_slot_irq() { m_a2bus->set_irq_line(CLEAR_LINE, m_slot); }
	void raise_slot_nmi() { m_a2bus->set_nmi_line(ASSERT_LINE, m_slot); }
	void lower_slot_nmi() { m_a2bus->set_nmi_line(CLEAR_LINE, m_slot); }
	void recalc_slot_inh() { m_a2bus->recalc_inh(m_slot); }
	void raise_slot_dma() { m_a2bus->set_dma_line(ASSERT_LINE); }
	void lower_slot_dma() { m_a2bus->set_dma_line(CLEAR_LINE); }

	device_a2bus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;

	int slotno() const { assert(m_a2bus); return m_slot; }
	a2bus_device &a2bus() { assert(m_a2bus); return *m_a2bus; }

private:
	optional_device<a2bus_device> m_a2bus_finder;
	a2bus_device *m_a2bus;
	const char *m_a2bus_slottag;
	int m_slot;
	device_a2bus_card_interface *m_next;
};

#endif  // MAME_BUS_A2BUS_A2BUS_H
