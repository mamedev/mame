// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, R.Belmont
/***************************************************************************

        HP DIO and DIO-II bus devices

        DIO is 16-bit, essentially the MC68000 bus
        DIO-II extends to 32-bit for 68020/030/040 machines

        16-bit DIO cards fit and work in either 16 or 32 bit systems, much like 8-bit ISA.
        32-bit DIO-II cards only work in 32 bit DIO-II systems.

***************************************************************************/

#ifndef MAME_BUS_HPDIO_HPDIO_H
#define MAME_BUS_HPDIO_HPDIO_H

#pragma once

namespace bus::hp_dio {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_dio16_card_interface;
class dio16_device;

class dio16_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T, typename U>
	dio16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&dio_tag, U &&opts, const char *dflt, bool fixed) :
		dio16_slot_device(mconfig, tag, owner, clock)
	{
		set_dio(std::forward<T>(dio_tag));
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}
	dio16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_dio(T &&dio_tag) { m_dio.set_tag(std::forward<T>(dio_tag)); }

protected:
	dio16_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;

	// configuration
	required_device<dio16_device> m_dio;
};

// ======================> dio16_device
class dio16_device : public device_t
{
public:
	// construction/destruction
	dio16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	// inline configuration
	template <typename T> void set_program_space(T &&tag, int spacenum) { m_prgspace.set_tag(std::forward<T>(tag), spacenum); }

	// callback configuration
	auto dmar0_out_cb() { return m_dmar0_out_cb.bind(); }
	auto dmar1_out_cb() { return m_dmar1_out_cb.bind(); }
	auto irq1_out_cb() { return m_irq1_out_cb.bind(); }
	auto irq2_out_cb() { return m_irq2_out_cb.bind(); }
	auto irq3_out_cb() { return m_irq3_out_cb.bind(); }
	auto irq4_out_cb() { return m_irq4_out_cb.bind(); }
	auto irq5_out_cb() { return m_irq5_out_cb.bind(); }
	auto irq6_out_cb() { return m_irq6_out_cb.bind(); }
	auto irq7_out_cb() { return m_irq7_out_cb.bind(); }

	template<typename R, typename W> void install_memory(offs_t start, offs_t end, R rhandler, W whandler);

	// DANGER: these will currently produce different results for a DIO-I card on DIO-I and DIO-II systems
	//         due to the varying bus widths.  Using all install_memory() shields you from this problem.
	//         Either know what you're doing (m_prgwidth is available to cards for this purpose) or
	//         only use these for 32-bit DIO-II cards.
	void install_bank(offs_t start, offs_t end, uint8_t *data);
	void install_rom(offs_t start, offs_t end, uint8_t *data);

	void unmap_bank(offs_t start, offs_t end);
	void unmap_rom(offs_t start, offs_t end);
	address_space &program_space() { return *m_prgspace; }

	// IRQs 1, 2, and 7 are reserved for non-bus usage.

	// input lines
	DECLARE_WRITE_LINE_MEMBER(dmar0_in) { set_dmar(m_bus_index, 0, state); }
	DECLARE_WRITE_LINE_MEMBER(dmar1_in) { set_dmar(m_bus_index, 1, state); }
	DECLARE_WRITE_LINE_MEMBER(irq1_in) { set_irq(m_bus_index, 0, state); }
	DECLARE_WRITE_LINE_MEMBER(irq2_in) { set_irq(m_bus_index, 1, state); }
	DECLARE_WRITE_LINE_MEMBER(irq3_in) { set_irq(m_bus_index, 2, state); }
	DECLARE_WRITE_LINE_MEMBER(irq4_in) { set_irq(m_bus_index, 3, state); }
	DECLARE_WRITE_LINE_MEMBER(irq5_in) { set_irq(m_bus_index, 4, state); }
	DECLARE_WRITE_LINE_MEMBER(irq6_in) { set_irq(m_bus_index, 5, state); }
	DECLARE_WRITE_LINE_MEMBER(irq7_in) { set_irq(m_bus_index, 6, state); }
	DECLARE_WRITE_LINE_MEMBER(reset_in);

	// output lines
	DECLARE_READ_LINE_MEMBER(irq1_out) const { return (m_irq[0] & ~m_bus_index) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER(irq2_out) const { return (m_irq[1] & ~m_bus_index) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER(irq3_out) const { return (m_irq[2] & ~m_bus_index) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER(irq4_out) const { return (m_irq[3] & ~m_bus_index) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER(irq5_out) const { return (m_irq[4] & ~m_bus_index) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER(irq6_out) const { return (m_irq[5] & ~m_bus_index) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER(irq7_out) const { return (m_irq[6] & ~m_bus_index) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER(dmar0_out) const { return dmar0_r(); }
	DECLARE_READ_LINE_MEMBER(dmar1_out) const { return dmar1_r(); }

	bool dmar0_r() const { return (m_dmar[0] & ~m_bus_index) ? 1 : 0; }
	bool dmar1_r() const { return (m_dmar[1] & ~m_bus_index) ? 1 : 0; }

	uint8_t dmack_r_out(int index, int channel);
	void dmack_w_out(int index, int channel, uint8_t data);

	int m_prgwidth;

	unsigned add_card(device_dio16_card_interface & card);
	void set_irq(unsigned int index, unsigned int num, int state);
	void set_dmar(unsigned int index, unsigned int num, int state);

protected:
	dio16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	void install_space(int spacenum, offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	std::list<device_dio16_card_interface *> m_cards;

	// address spaces
	required_address_space m_prgspace;
	int m_bus_index;

	// packed line states
	u16 m_irq[7];
	u16 m_dmar[2];

	devcb_write_line m_irq1_out_cb;
	devcb_write_line m_irq2_out_cb;
	devcb_write_line m_irq3_out_cb;
	devcb_write_line m_irq4_out_cb;
	devcb_write_line m_irq5_out_cb;
	devcb_write_line m_irq6_out_cb;
	devcb_write_line m_irq7_out_cb;

	devcb_write_line m_dmar0_out_cb;
	devcb_write_line m_dmar1_out_cb;
};

// ======================> device_dio16_card_interface

// class representing interface-specific live dio16 card
class device_dio16_card_interface : public device_interface
{
	friend class dio16_device;
	template <class ElementType> friend class simple_list;
public:
	// construction/destruction
	virtual ~device_dio16_card_interface();

	device_dio16_card_interface *next() const { return m_next; }
	// inline configuration
	void set_diobus(dio16_device &dio_device) {
		m_dio_dev = &dio_device;
		m_index = m_dio_dev->add_card(*this);
	}
protected:
	device_dio16_card_interface(const machine_config &mconfig, device_t &device);
	dio16_device &dio() {
		assert(m_dio_dev);
		return *m_dio_dev;
	}

	virtual void interface_pre_start() override;

	int get_index() { return m_index; }
	address_space &program_space() { return m_dio_dev->program_space(); }

	DECLARE_WRITE_LINE_MEMBER(irq1_out) { m_dio_dev->set_irq(m_index, 0, state); }
	DECLARE_WRITE_LINE_MEMBER(irq2_out) { m_dio_dev->set_irq(m_index, 1, state); }
	DECLARE_WRITE_LINE_MEMBER(irq3_out) { m_dio_dev->set_irq(m_index, 2, state); }
	DECLARE_WRITE_LINE_MEMBER(irq4_out) { m_dio_dev->set_irq(m_index, 3, state); }
	DECLARE_WRITE_LINE_MEMBER(irq5_out) { m_dio_dev->set_irq(m_index, 4, state); }
	DECLARE_WRITE_LINE_MEMBER(irq6_out) { m_dio_dev->set_irq(m_index, 5, state); }
	DECLARE_WRITE_LINE_MEMBER(irq7_out) { m_dio_dev->set_irq(m_index, 6, state); }
	DECLARE_WRITE_LINE_MEMBER(dmar0_out) { m_dio_dev->set_dmar(m_index, 0, state); }
	DECLARE_WRITE_LINE_MEMBER(dmar1_out) { m_dio_dev->set_dmar(m_index, 1, state); }

	virtual DECLARE_WRITE_LINE_MEMBER(irq1_in) {}
	virtual DECLARE_WRITE_LINE_MEMBER(irq2_in) {}
	virtual DECLARE_WRITE_LINE_MEMBER(irq3_in) {}
	virtual DECLARE_WRITE_LINE_MEMBER(irq4_in) {}
	virtual DECLARE_WRITE_LINE_MEMBER(irq5_in) {}
	virtual DECLARE_WRITE_LINE_MEMBER(irq6_in) {}
	virtual DECLARE_WRITE_LINE_MEMBER(irq7_in) {}
	virtual DECLARE_WRITE_LINE_MEMBER(dmar0_in) {}
	virtual DECLARE_WRITE_LINE_MEMBER(dmar1_in) {}

	virtual uint8_t dmack_r_out(int channel) { return m_dio_dev->dmack_r_out(m_index, channel); }
	virtual void dmack_w_out(int channel, uint8_t data) { m_dio_dev->dmack_w_out(m_index, channel, data); }
	virtual uint8_t dmack_r_in(int channel) { return 0xff; }
	virtual void dmack_w_in(int channel, uint8_t data) {}

	virtual DECLARE_WRITE_LINE_MEMBER(reset_in) {}

	bool dmar0_r() const { return m_dio_dev->dmar0_r(); }
	bool dmar1_r() const { return m_dio_dev->dmar1_r(); }

	dio16_device *m_dio_dev;

private:
	void set_bus(dio16_device & bus);
	device_dio16_card_interface *m_next;
	unsigned int m_index;
};

class dio32_device;

class dio32_slot_device : public dio16_slot_device
{
public:
	// construction/destruction
	template <typename T, typename U>
	dio32_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&dio_tag, U &&opts, const char *dflt, bool fixed) :
		dio32_slot_device(mconfig, tag, owner, clock)
	{
		set_dio(std::forward<T>(dio_tag));
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}
	dio32_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
};

// ======================> dio32_device
class dio32_device : public dio16_device
{
public:
	// construction/destruction
	dio32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void install16_device(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler);

protected:
	// device-level overrides
	virtual void device_start() override;
};

// ======================> device_dio32_card_interface

// class representing interface-specific live dio32 card
class device_dio32_card_interface : public device_dio16_card_interface
{
	friend class dio32_device;
public:
	// construction/destruction
	virtual ~device_dio32_card_interface();

protected:
	device_dio32_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	dio32_device &dio() { assert(m_dio_dev); return downcast<dio32_device &>(*m_dio_dev); }
};

} // namespace bus::hp_dio

// device type definition
DECLARE_DEVICE_TYPE_NS(DIO16_SLOT, bus::hp_dio, dio16_slot_device)
DECLARE_DEVICE_TYPE_NS(DIO32, bus::hp_dio, dio32_device)
DECLARE_DEVICE_TYPE_NS(DIO32_SLOT, bus::hp_dio, dio32_slot_device)
DECLARE_DEVICE_TYPE_NS(DIO16, bus::hp_dio, dio16_device)

void dio16_cards(device_slot_interface &device);
void dio32_cards(device_slot_interface &device);

#endif // MAME_BUS_HPDIO_HPDIO_H
