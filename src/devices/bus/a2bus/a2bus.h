// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  a2bus.h - Apple II slot bus and card emulation

  by R. Belmont

***************************************************************************/

#ifndef MAME_BUS_A2BUS_A2BUS_H
#define MAME_BUS_A2BUS_A2BUS_H

#pragma once


// /INH special addresses
#define INH_START_INVALID   0xffff;
#define INH_END_INVALID     0x0000;

// /INH types
#define INH_NONE            0x00
#define INH_READ            0x01
#define INH_WRITE           0x02

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_A2BUS_CPU(_cputag) \
	a2bus_device::static_set_cputag(*device, _cputag);

#define MCFG_A2BUS_OUT_IRQ_CB(_devcb) \
	devcb = &a2bus_device::set_out_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_A2BUS_OUT_NMI_CB(_devcb) \
	devcb = &a2bus_device::set_out_nmi_callback(*device, DEVCB_##_devcb);

#define MCFG_A2BUS_OUT_INH_CB(_devcb) \
	devcb = &a2bus_device::set_out_inh_callback(*device, DEVCB_##_devcb);

#define MCFG_A2BUS_SLOT_ADD(_nbtag, _tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, A2BUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	a2bus_slot_device::static_set_a2bus_slot(*device, _nbtag, _tag);
#define MCFG_A2BUS_SLOT_REMOVE(_tag)    \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_A2BUS_ONBOARD_ADD(_nbtag, _tag, _dev_type, _def_inp) \
	MCFG_DEVICE_ADD(_tag, _dev_type, 0) \
	MCFG_DEVICE_INPUT_DEFAULTS(_def_inp) \
	device_a2bus_card_interface::static_set_a2bus_tag(*device, _nbtag, _tag);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_device;

class a2bus_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	a2bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	static void static_set_a2bus_slot(device_t &device, const char *tag, const char *slottag);
protected:
	a2bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	const char *m_a2bus_tag, *m_a2bus_slottag;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_SLOT, a2bus_slot_device)


class device_a2bus_card_interface;
// ======================> a2bus_device
class a2bus_device : public device_t
{
	// multi-card devices need to access m_device_list, so they get friended here.
	friend class a2bus_mcms2_device;
public:
	// construction/destruction
	a2bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);
	template <class Object> static devcb_base &set_out_irq_callback(device_t &device, Object &&cb) { return downcast<a2bus_device &>(device).m_out_irq_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_out_nmi_callback(device_t &device, Object &&cb) { return downcast<a2bus_device &>(device).m_out_nmi_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_out_inh_callback(device_t &device, Object &&cb) { return downcast<a2bus_device &>(device).m_out_inh_cb.set_callback(std::forward<Object>(cb)); }

	void add_a2bus_card(int slot, device_a2bus_card_interface *card);
	device_a2bus_card_interface *get_a2bus_card(int slot);
	uint8_t get_a2bus_irq_mask();
	uint8_t get_a2bus_nmi_mask();

	void set_irq_line(int state, int slot);
	void set_nmi_line(int state, int slot);
	void set_maincpu_halt(int state);
	void recalc_inh(int slot);
	uint8_t dma_r(address_space &space, uint16_t offset);
	void dma_w(address_space &space, uint16_t offset, uint8_t data);
	uint8_t dma_nospace_r(uint16_t offset);
	void dma_nospace_w(uint16_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );

protected:
	a2bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	cpu_device   *m_maincpu;
	address_space *m_maincpu_space;

	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;
	devcb_write8        m_out_inh_cb;

	device_a2bus_card_interface *m_device_list[8];
	const char *m_cputag;

	uint8_t m_slot_irq_mask;
	uint8_t m_slot_nmi_mask;
};


// device type definition
DECLARE_DEVICE_TYPE(A2BUS, a2bus_device)

// ======================> device_a2bus_card_interface

// class representing interface-specific live a2bus card
class device_a2bus_card_interface : public device_slot_card_interface
{
	friend class a2bus_device;
public:
	// construction/destruction
	virtual ~device_a2bus_card_interface();

	virtual uint8_t read_c0nx(address_space &space, uint8_t offset) { m_device.logerror("a2bus: unhandled read at C0n%x\n", offset); return 0; }       // C0nX - /DEVSEL
	virtual void write_c0nx(address_space &space, uint8_t offset, uint8_t data) { m_device.logerror("a2bus: unhandled write %02x to C0n%x\n", data, offset); }
	virtual uint8_t read_cnxx(address_space &space, uint8_t offset) { return 0; }       // CnXX - /IOSEL
	virtual void write_cnxx(address_space &space, uint8_t offset, uint8_t data) { m_device.logerror("a2bus: unhandled write %02x to Cn%02x\n", data, offset); }
	virtual uint8_t read_c800(address_space &space, uint16_t offset) { return 0; }      // C800 - /IOSTB
	virtual void write_c800(address_space &space, uint16_t offset, uint8_t data) {m_device.logerror("a2bus: unhandled write %02x to %04x\n", data, offset + 0xc800); }
	virtual bool take_c800() { return true; }   // override and return false if your card doesn't take over the c800 space
	virtual uint8_t read_inh_rom(address_space &space, uint16_t offset) { return 0; }
	virtual void write_inh_rom(address_space &space, uint16_t offset, uint8_t data) { }
	virtual uint16_t inh_start() { return INH_START_INVALID; }
	virtual uint16_t inh_end() { return INH_END_INVALID; }
	virtual int inh_type() { return INH_NONE; }

	device_a2bus_card_interface *next() const { return m_next; }

	void set_a2bus_device();

	uint32_t get_slotromspace() { return 0xc000 | (m_slot<<8); }      // return Cn00 address for this slot
	uint32_t get_slotiospace() { return 0xc080 + (m_slot<<4); }       // return C0n0 address for this slot

	void raise_slot_irq() { m_a2bus->set_irq_line(ASSERT_LINE, m_slot); }
	void lower_slot_irq() { m_a2bus->set_irq_line(CLEAR_LINE, m_slot); }
	void raise_slot_nmi() { m_a2bus->set_nmi_line(ASSERT_LINE, m_slot); }
	void lower_slot_nmi() { m_a2bus->set_nmi_line(CLEAR_LINE, m_slot); }
	void recalc_slot_inh() { m_a2bus->recalc_inh(m_slot); }
	void set_maincpu_halt(int state) { m_a2bus->set_maincpu_halt(state); }

	// pass through the original address space if any for debugger protection
	// when debugging e.g. coprocessor cards (Z80 SoftCard etc).
	uint8_t slot_dma_read(address_space &space, uint16_t offset) { return m_a2bus->dma_r(space, offset); }
	void slot_dma_write(address_space &space, uint16_t offset, uint8_t data) { m_a2bus->dma_w(space, offset, data); }

	// these versions forego that protection for when the DMA isn't coming from a debuggable CPU device
	uint8_t slot_dma_read_no_space(uint16_t offset) { return m_a2bus->dma_nospace_r(offset); }
	void slot_dma_write_no_space(uint16_t offset, uint8_t data) { m_a2bus->dma_nospace_w(offset, data); }

	// inline configuration
	static void static_set_a2bus_tag(device_t &device, const char *tag, const char *slottag);

protected:
	device_a2bus_card_interface(const machine_config &mconfig, device_t &device);

	a2bus_device  *m_a2bus;
	const char *m_a2bus_tag, *m_a2bus_slottag;
	int m_slot;
	device_a2bus_card_interface *m_next;
};

#endif  // MAME_BUS_A2BUS_A2BUS_H
