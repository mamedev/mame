// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9845_io.h

    I/O bus of HP9825/HP9845 systems

*********************************************************************/

#ifndef MAME_BUS_HP9845_IO_HP9845_IO_H
#define MAME_BUS_HP9845_IO_HP9845_IO_H

#pragma once

#define HP9845_IO_FIRST_SC  1   // Lowest SC used by I/O cards

#define PORT_HP9845_IO_SC(_default_sc)              \
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

class device_hp9845_io_interface;

class hp9845_io_slot_device : public device_t,
							  public device_single_card_slot_interface<device_hp9845_io_interface>
{
public:
	// construction/destruction
	hp9845_io_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp9845_io_slot_device();

	// Callback setups
	auto irq() { return m_irq_cb_func.bind(); }
	auto sts() { return m_sts_cb_func.bind(); }
	auto flg() { return m_flg_cb_func.bind(); }
	auto irq_nextsc() { return m_irq_nextsc_cb_func.bind(); }
	auto sts_nextsc() { return m_sts_nextsc_cb_func.bind(); }
	auto flg_nextsc() { return m_flg_nextsc_cb_func.bind(); }
	auto dmar() { return m_dmar_cb_func.bind(); }

	// irq/sts/flg/dmar signal handlers for card devices
	void irq_w(int state);
	void sts_w(int state);
	void flg_w(int state);
	void irq_nextsc_w(int state);
	void sts_nextsc_w(int state);
	void flg_nextsc_w(int state);
	void dmar_w(int state);

	// getter for r/w handlers
	// return value is SC (negative if no card is attached to slot)
	int get_rw_handlers(read16m_delegate& rhandler , write16m_delegate& whandler);

	bool has_dual_sc() const;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_irq_cb_func;
	devcb_write_line m_sts_cb_func;
	devcb_write_line m_flg_cb_func;
	devcb_write_line m_irq_nextsc_cb_func;
	devcb_write_line m_sts_nextsc_cb_func;
	devcb_write_line m_flg_nextsc_cb_func;
	devcb_write_line m_dmar_cb_func;
};

class device_hp9845_io_interface : public device_interface
{
public:
	void set_slot_device(hp9845_io_slot_device &dev);

	virtual uint16_t reg_r(address_space &space, offs_t offset) = 0;
	virtual void reg_w(address_space &space, offs_t offset, uint16_t data) = 0;

	// SC getter
	uint8_t get_sc();

	virtual bool has_dual_sc() const;

protected:
	// construction/destruction
	device_hp9845_io_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_hp9845_io_interface();

	virtual void interface_pre_start() override;

	hp9845_io_slot_device *m_slot_dev;
	required_ioport m_select_code_port;

	// card device handling
	void irq_w(int state);
	void sts_w(int state);
	void flg_w(int state);
	void irq_nextsc_w(int state);
	void sts_nextsc_w(int state);
	void flg_nextsc_w(int state);
	void dmar_w(int state);
};

// device type definition
DECLARE_DEVICE_TYPE(HP9845_IO_SLOT, hp9845_io_slot_device)

#endif // MAME_BUS_HP9845_IO_HP9845_IO_H
