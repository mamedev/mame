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

namespace bus {
	namespace hp_dio {
//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DIO16_CPU(_cputag) \
	downcast<bus::hp_dio::dio16_device &>(*device).set_cputag(_cputag);
#define MCFG_DIO32_CPU(_cputag) \
	downcast<bus::hp_dio::dio32_device &>(*device).set_cputag(_cputag);

#define MCFG_ISA_OUT_IRQ3_CB(_devcb) \
	downcast<bus::hp_dio::dio16_device &>(*device).set_out_irq3_callback(DEVCB_##_devcb);

#define MCFG_ISA_OUT_IRQ4_CB(_devcb) \
	downcast<bus::hp_dio::dio16_device &>(*device).set_out_irq4_callback(DEVCB_##_devcb);

#define MCFG_ISA_OUT_IRQ5_CB(_devcb) \
	downcast<bus::hp_dio::dio16_device &>(*device).set_out_irq5_callback(DEVCB_##_devcb);

#define MCFG_ISA_OUT_IRQ6_CB(_devcb) \
	downcast<bus::hp_dio::dio16_device &>(*device).set_out_irq6_callback(DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

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


class device_dio16_card_interface;
// ======================> dio16_device
class dio16_device : public device_t
{
public:
	// construction/destruction
	dio16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	// inline configuration
	template <typename T> void set_cputag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	template <class Object> devcb_base &set_out_irq3_callback(Object &&cb) { return m_out_irq3_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irq4_callback(Object &&cb) { return m_out_irq4_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irq5_callback(Object &&cb) { return m_out_irq5_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irq6_callback(Object &&cb) { return m_out_irq6_cb.set_callback(std::forward<Object>(cb)); }

	void install_memory(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler);

	// DANGER: these will currently produce different results for a DIO-I card on DIO-I and DIO-II systems
	//         due to the varying bus widths.  Using all install_memory() shields you from this problem.
	//         either know what you're doing (m_prgwidth is available to cards for this purpose) or
	//         only use these for 32-bit DIO-II cards.
	void install_bank(offs_t start, offs_t end, const char *tag, uint8_t *data);
	void install_rom(offs_t start, offs_t end, const char *tag, uint8_t *data);

	void unmap_bank(offs_t start, offs_t end);
	void unmap_rom(offs_t start, offs_t end);

	// IRQs 1, 2, and 7 are reserved for non-bus usage.
	DECLARE_WRITE_LINE_MEMBER( irq3_w ) { m_out_irq3_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( irq4_w ) { m_out_irq4_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( irq5_w ) { m_out_irq5_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( irq6_w ) { m_out_irq6_cb(state); }

	int m_prgwidth;

protected:
	dio16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void install_space(int spacenum, offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	required_device<cpu_device> m_maincpu;

	// address spaces
	address_space *m_prgspace;

	devcb_write_line    m_out_irq3_cb;
	devcb_write_line    m_out_irq4_cb;
	devcb_write_line    m_out_irq5_cb;
	devcb_write_line    m_out_irq6_cb;
};

// ======================> device_dio16_card_interface

// class representing interface-specific live dio16 card
class device_dio16_card_interface : public device_slot_card_interface
{
	friend class dio16_device;
	template <class ElementType> friend class simple_list;
public:
	// construction/destruction
	virtual ~device_dio16_card_interface();

	device_dio16_card_interface *next() const { return m_next; }

	// inline configuration
	void set_diobus(dio16_device &dio_device) { m_dio_dev = &dio_device; }

protected:
	device_dio16_card_interface(const machine_config &mconfig, device_t &device);
	dio16_device &dio() { assert(m_dio_dev); return *m_dio_dev; }

	virtual void interface_pre_start() override;

	dio16_device *m_dio_dev;

private:
	device_dio16_card_interface *m_next;
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
} // namespace bus

// device type definition
DECLARE_DEVICE_TYPE_NS(DIO16_SLOT, bus::hp_dio, dio16_slot_device)
DECLARE_DEVICE_TYPE_NS(DIO32, bus::hp_dio, dio32_device)
DECLARE_DEVICE_TYPE_NS(DIO32_SLOT, bus::hp_dio, dio32_slot_device)
DECLARE_DEVICE_TYPE_NS(DIO16, bus::hp_dio, dio16_device)

#endif // MAME_BUS_HPDIO_HPDIO_H
