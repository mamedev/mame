// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9845_io.h

    I/O bus of HP9845 systems

*********************************************************************/

#ifndef MAME_BUS_HP9845_IO_HP9845_IO_H
#define MAME_BUS_HP9845_IO_HP9845_IO_H

#pragma once


#define MCFG_HP9845_IO_SLOT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, HP9845_IO_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(hp9845_io_slot_devices, nullptr, false)

#define MCFG_HP9845_IO_IRQ_CB(_devcb) \
	devcb = &downcast<hp9845_io_slot_device &>(*device).set_irq_cb_func(DEVCB_##_devcb);

#define MCFG_HP9845_IO_STS_CB(_devcb) \
	devcb = &downcast<hp9845_io_slot_device &>(*device).set_sts_cb_func(DEVCB_##_devcb);

#define MCFG_HP9845_IO_FLG_CB(_devcb) \
	devcb = &downcast<hp9845_io_slot_device &>(*device).set_flg_cb_func(DEVCB_##_devcb);

#define HP9845_IO_FIRST_SC  1   // Lowest SC used by I/O cards

#define MCFG_HP9845_IO_SC(_default_sc)              \
	PORT_START("SC") \
	PORT_CONFNAME(0xf , (_default_sc) - HP9845_IO_FIRST_SC , "Select Code") \
	PORT_CONFSETTING(0 , "1")\
	PORT_CONFSETTING(1 , "2")\
	PORT_CONFSETTING(2 , "3")\
	PORT_CONFSETTING(3 , "4")\
	PORT_CONFSETTING(4 , "5")\
	PORT_CONFSETTING(5 , "6")\
	PORT_CONFSETTING(6 , "7")\
	PORT_CONFSETTING(7 , "8")\
	PORT_CONFSETTING(8 , "9")\
	PORT_CONFSETTING(9 , "10")\
	PORT_CONFSETTING(10 , "11")\
	PORT_CONFSETTING(11 , "12")

class hp9845_io_slot_device : public device_t,
							  public device_slot_interface
{
public:
	// construction/destruction
	hp9845_io_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp9845_io_slot_device();

	// device-level overrides
	virtual void device_start() override;

	// Callback setups
	template <class Object> devcb_base &set_irq_cb_func(Object &&cb) { return m_irq_cb_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_sts_cb_func(Object &&cb) { return m_sts_cb_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_flg_cb_func(Object &&cb) { return m_flg_cb_func.set_callback(std::forward<Object>(cb)); }

	// irq/sts/flg signal handlers
	void irq_w(uint8_t sc , int state);
	void sts_w(uint8_t sc , int state);
	void flg_w(uint8_t sc , int state);

	// getter for r/w handlers
	// return value is SC (negative if no card is attached to slot)
	int get_rw_handlers(read16_delegate& rhandler , write16_delegate& whandler);

private:
	devcb_write8 m_irq_cb_func;
	devcb_write8 m_sts_cb_func;
	devcb_write8 m_flg_cb_func;
};

class hp9845_io_card_device : public device_t,
							  public device_slot_card_interface
{
public:
	void set_slot_device(hp9845_io_slot_device* dev);

	virtual DECLARE_READ16_MEMBER(reg_r) = 0;
	virtual DECLARE_WRITE16_MEMBER(reg_w) = 0;

	// SC getter
	uint8_t get_sc(void);

protected:
	// construction/destruction
	hp9845_io_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp9845_io_card_device();

	// device-level overrides
	virtual void device_reset() override;

	hp9845_io_slot_device *m_slot_dev;
	required_ioport m_select_code_port;
	uint8_t m_my_sc;

	// card device handling
	void irq_w(int state);
	void sts_w(int state);
	void flg_w(int state);
};

// device type definition
DECLARE_DEVICE_TYPE(HP9845_IO_SLOT, hp9845_io_slot_device)

void hp9845_io_slot_devices(device_slot_interface &device);

#endif // MAME_BUS_HP9845_IO_HP9845_IO_H
