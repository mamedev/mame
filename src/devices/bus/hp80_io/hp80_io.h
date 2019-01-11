// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp80_io.h

    I/O bus of HP80 systems

*********************************************************************/

#ifndef MAME_BUS_HP80_IO_HP80_IO_H
#define MAME_BUS_HP80_IO_HP80_IO_H

#pragma once

#define MCFG_HP80_IO_SLOT_ADD(_tag , _idx)                              \
	MCFG_DEVICE_ADD(_tag, HP80_IO_SLOT, 0)                              \
	MCFG_DEVICE_SLOT_INTERFACE(hp80_io_slot_devices, nullptr, false)    \
	downcast<hp80_io_slot_device &>(*device).set_slot_no(_idx);

#define MCFG_HP80_IO_IRL_CB(_devcb) \
	devcb = &downcast<hp80_io_slot_device &>(*device).set_irl_cb_func(DEVCB_##_devcb);

#define MCFG_HP80_IO_HALT_CB(_devcb) \
	devcb = &downcast<hp80_io_slot_device &>(*device).set_halt_cb_func(DEVCB_##_devcb);

#define HP80_IO_FIRST_SC  3   // Lowest SC used by I/O cards

#define MCFG_HP80_IO_SC(_default_sc)              \
	PORT_START("SC") \
	PORT_CONFNAME(0xf , (_default_sc) - HP80_IO_FIRST_SC , "Select Code") \
	PORT_CONFSETTING(0 , "3")\
	PORT_CONFSETTING(1 , "4")\
	PORT_CONFSETTING(2 , "5")\
	PORT_CONFSETTING(3 , "6")\
	PORT_CONFSETTING(4 , "7")\
	PORT_CONFSETTING(5 , "8")\
	PORT_CONFSETTING(6 , "9")\
	PORT_CONFSETTING(7 , "10")

class hp80_io_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	hp80_io_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp80_io_slot_device();

	// configuration helpers
	void set_slot_no(unsigned slot_no) { m_slot_no = slot_no; }

	// device-level overrides
	virtual void device_start() override;

	// Callback setups
	template <class Object> devcb_base &set_irl_cb_func(Object &&cb) { return m_irl_cb_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_halt_cb_func(Object &&cb) { return m_halt_cb_func.set_callback(std::forward<Object>(cb)); }

	// SC getter
	uint8_t get_sc() const;

	uint16_t get_base_addr() const;

	void install_read_write_handlers(address_space& space);

	DECLARE_WRITE_LINE_MEMBER(irl_w);
	DECLARE_WRITE_LINE_MEMBER(halt_w);

	void inten();
	void clear_service();

private:
	devcb_write8 m_irl_cb_func;
	devcb_write8 m_halt_cb_func;
	unsigned m_slot_no;
};

class hp80_io_card_device : public device_t,
							public device_slot_card_interface
{
public:
	// SC getter
	uint8_t get_sc() const;

	virtual void install_read_write_handlers(address_space& space , uint16_t base_addr) = 0;

	virtual void inten();
	virtual void clear_service();

protected:
	// construction/destruction
	hp80_io_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp80_io_card_device();

	required_ioport m_select_code_port;

	// card device handling
	DECLARE_WRITE_LINE_MEMBER(irl_w);
	DECLARE_WRITE_LINE_MEMBER(halt_w);
};

// device type definition
DECLARE_DEVICE_TYPE(HP80_IO_SLOT, hp80_io_slot_device)

void hp80_io_slot_devices(device_slot_interface &device);

#endif // MAME_BUS_HP80_IO_HP80_IO_H
