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
	dmvcart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~dmvcart_slot_device();

	template <class Object> devcb_base &set_prog_read_callback(Object &&cb) { return m_prog_read_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_prog_write_callback(Object &&cb) { return m_prog_write_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_int_callback(Object &&cb) { return m_out_int_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irq_callback(Object &&cb) { return m_out_irq_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_thold_callback(Object &&cb) { return m_out_thold_cb.set_callback(std::forward<Object>(cb)); }

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
	device_dmvslot_interface*       m_cart;
};


// device type definition
DECLARE_DEVICE_TYPE(DMVCART_SLOT, dmvcart_slot_device)


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DMVCART_SLOT_PROGRAM_READWRITE_CB(_read_devcb, _write_devcb) \
	downcast<dmvcart_slot_device &>(*device).set_prog_read_callback(DEVCB_##_read_devcb); \
	downcast<dmvcart_slot_device &>(*device).set_prog_write_callback(DEVCB_##_write_devcb);

#define MCFG_DMVCART_SLOT_OUT_INT_CB(_devcb) \
	downcast<dmvcart_slot_device &>(*device).set_out_int_callback(DEVCB_##_devcb);

#define MCFG_DMVCART_SLOT_OUT_IRQ_CB(_devcb) \
	downcast<dmvcart_slot_device &>(*device).set_out_irq_callback(DEVCB_##_devcb);

#define MCFG_DMVCART_SLOT_OUT_THOLD_CB(_devcb) \
	downcast<dmvcart_slot_device &>(*device).set_out_thold_callback(DEVCB_##_devcb);

#endif // MAME_BUS_DMV_DMVBUS_H
