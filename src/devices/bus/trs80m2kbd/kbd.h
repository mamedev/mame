// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II/12/16/6000 keyboard connector emulation

    DATA, CLOCK and BUSY* between the keyboard MCU and the
    video/keyboard board.

**********************************************************************/

#ifndef MAME_BUS_TRS80M2KBD_KBD_H
#define MAME_BUS_TRS80M2KBD_KBD_H

#pragma once


class device_trs80m2_keyboard_interface;

class trs80m2_keyboard_port_device : public device_t, public device_single_card_slot_interface<device_trs80m2_keyboard_interface>
{
public:
	template <typename T>
	trs80m2_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt) :
		trs80m2_keyboard_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	trs80m2_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto clock_wr_callback() { return m_write_clock.bind(); }

	int data_r();
	void busy_w(int state);

	void clock_w(int state) { m_write_clock(state); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_write_clock;

	device_trs80m2_keyboard_interface *m_keyboard;
};


class device_trs80m2_keyboard_interface : public device_interface
{
public:
	virtual int data_r() = 0;
	virtual void busy_w(int state) = 0;

protected:
	device_trs80m2_keyboard_interface(const machine_config &mconfig, device_t &device);

	trs80m2_keyboard_port_device *m_port;
};


DECLARE_DEVICE_TYPE(TRS80M2_KEYBOARD_PORT, trs80m2_keyboard_port_device)

void trs80m2_keyboards(device_slot_interface &device);

#endif // MAME_BUS_TRS80M2KBD_KBD_H
