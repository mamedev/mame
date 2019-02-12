// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    Decision Mate V expansion slot

*********************************************************************/

#ifndef MAME_BUS_DMV_DMVBUS_H
#define MAME_BUS_DMV_DMVBUS_H

#pragma once

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> device_dmvslot_interface

class device_dmvslot_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_dmvslot_interface();

	virtual bool read(offs_t offset, uint8_t &data) { return false; }
	virtual bool write(offs_t offset, uint8_t data) { return false; }
	virtual void io_read(address_space &space, int ifsel, offs_t offset, uint8_t &data) { }
	virtual void io_write(address_space &space, int ifsel, offs_t offset, uint8_t data) { }

	// slot 1
	virtual void ram_read(uint8_t cas, offs_t offset, uint8_t &data) { }
	virtual void ram_write(uint8_t cas, offs_t offset, uint8_t data) { }

	// slot 7 and 7A
	virtual bool av16bit() { return false; }
	virtual void hold_w(int state) { }
	virtual void switch16_w(int state) { }
	virtual void timint_w(int state) { }
	virtual void keyint_w(int state) { }
	virtual void busint_w(int state) { }
	virtual void flexint_w(int state) { }
	virtual void irq2_w(int state) { }
	virtual void irq2a_w(int state) { }
	virtual void irq3_w(int state) { }
	virtual void irq4_w(int state) { }
	virtual void irq5_w(int state) { }
	virtual void irq6_w(int state) { }

protected:
	device_dmvslot_interface(const machine_config &mconfig, device_t &device);
};


// ======================> dmvcart_slot_device

class dmvcart_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	dmvcart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: dmvcart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	dmvcart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~dmvcart_slot_device();

	auto prog_read() { return m_prog_read_cb.bind(); }
	auto prog_write() { return m_prog_write_cb.bind(); }
	auto out_int() { return m_out_int_cb.bind(); }
	auto out_irq() { return m_out_irq_cb.bind(); }
	auto out_thold() { return m_out_thold_cb.bind(); }
	template <typename T> void set_memspace(T &&tag, int spacenum) { m_memspace.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }

	// device-level overrides
	virtual void device_start() override;

	// reading and writing
	virtual bool read(offs_t offset, uint8_t &data);
	virtual bool write(offs_t offset, uint8_t data);
	virtual void ram_read(uint8_t cas, offs_t offset, uint8_t &data);
	virtual void ram_write(uint8_t cas, offs_t offset, uint8_t data);
	virtual void io_read(address_space &space, int ifsel, offs_t offset, uint8_t &data);
	virtual void io_write(address_space &space, int ifsel, offs_t offset, uint8_t data);
	virtual void hold_w(int state);
	virtual void switch16_w(int state);
	virtual void timint_w(int state);
	virtual void keyint_w(int state);
	virtual void busint_w(int state);
	virtual void flexint_w(int state);
	virtual void irq2_w(int state);
	virtual void irq2a_w(int state);
	virtual void irq3_w(int state);
	virtual void irq4_w(int state);
	virtual void irq5_w(int state);
	virtual void irq6_w(int state);
	virtual bool av16bit();

	// internal state
	devcb_read8                     m_prog_read_cb;
	devcb_write8                    m_prog_write_cb;
	devcb_write_line                m_out_int_cb;
	devcb_write_line                m_out_irq_cb;
	devcb_write_line                m_out_thold_cb;

	required_address_space          m_memspace;
	required_address_space          m_iospace;

	device_dmvslot_interface*       m_cart;
};


// device type definition
DECLARE_DEVICE_TYPE(DMVCART_SLOT, dmvcart_slot_device)

#endif // MAME_BUS_DMV_DMVBUS_H
