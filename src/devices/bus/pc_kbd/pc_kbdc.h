// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  PC Keyboard connector interface

The data line is usually sampled on changes of the clock line. If you have
a device that changes both the data and clock lines at the same time, first
set the data line and then set the clock line.

***************************************************************************/

#ifndef MAME_BUS_PC_KBD_PC_KBDC_H
#define MAME_BUS_PC_KBD_PC_KBDC_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_pc_kbd_interface;


class pc_kbdc_device : public device_t, public device_single_card_slot_interface<device_pc_kbd_interface>
{
public:
	// construction/destruction
	template <typename T>
	pc_kbdc_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: pc_kbdc_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	pc_kbdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_clock_cb() { return m_out_clock_cb.bind(); }
	auto out_data_cb() { return m_out_data_cb.bind(); }

	void set_keyboard(device_pc_kbd_interface *keyboard);

	int clock_signal() { return m_clock_state; }
	int data_signal() { return m_data_state; }

	void clock_write_from_mb(int state);
	void data_write_from_mb(int state);
	void clock_write_from_kb(int state);
	void data_write_from_kb(int state);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void update_clock_state(bool fromkb);
	void update_data_state(bool fromkb);

	devcb_write_line    m_out_clock_cb;
	devcb_write_line    m_out_data_cb;

	int8_t m_clock_state;
	int8_t m_data_state;

	uint8_t m_mb_clock_state;
	uint8_t m_mb_data_state;
	uint8_t m_kb_clock_state;
	uint8_t m_kb_data_state;

	device_pc_kbd_interface     *m_keyboard;
};


// device type definition
DECLARE_DEVICE_TYPE(PC_KBDC, pc_kbdc_device)


// ======================> device_pc_pbd_interface

class device_pc_kbd_interface : public device_interface
{
	friend class pc_kbdc_device;
public:
	virtual ~device_pc_kbd_interface();

	void set_pc_kbdc_device();

	//
	// Override the clock_write and data_write methods in a keyboard implementation
	//
	virtual void clock_write(int state);
	virtual void data_write(int state);

	// inline configuration
	void set_pc_kbdc(device_t *kbdc_device) { m_pc_kbdc = dynamic_cast<pc_kbdc_device *>(kbdc_device); }

protected:
	device_pc_kbd_interface(const machine_config &mconfig, device_t &device);

	int clock_signal() const { return m_pc_kbdc ? m_pc_kbdc->clock_signal() : 1; }
	int data_signal() const { return m_pc_kbdc ? m_pc_kbdc->data_signal() : 1; }

	pc_kbdc_device          *m_pc_kbdc;
	const char              *m_pc_kbdc_tag;
};


#endif // MAME_BUS_PC_KBD_PC_KBDC_H
