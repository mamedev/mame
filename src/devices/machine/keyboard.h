// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_DEVICES_MACHINE_KEYBOARD_H
#define MAME_DEVICES_MACHINE_KEYBOARD_H

#pragma once

#include "emu.h"

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_GENERIC_KEYBOARD_CB(cb) \
		devcb = &generic_keyboard_device::set_keyboard_callback(*device, DEVCB_##cb);



/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

extern const device_type GENERIC_KEYBOARD;



/***************************************************************************
    REUSABLE I/O PORTS
***************************************************************************/

INPUT_PORTS_EXTERN( generic_keyboard );



/***************************************************************************
    TYPE DECLARATIONS
***************************************************************************/

template <UINT8 ROW_COUNT>
class device_matrix_keyboard_interface : public device_interface
{
protected:
	template <typename... T>
	device_matrix_keyboard_interface(machine_config const &mconfig, device_t &device, T &&... tags);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void start_processing(const attotime &period);
	void stop_processing();
	void reset_key_state();

	void typematic_start(UINT8 row, UINT8 column, attotime const &delay, attotime const &interval);
	void typematic_restart(attotime const &delay, attotime const &interval);
	void typematic_stop();
	bool typematic_is(UINT8 row, UINT8 column) const { return (m_typematic_row == row) && (m_typematic_column == column); }

	virtual void key_make(UINT8 row, UINT8 column) = 0;
	virtual void key_repeat(UINT8 row, UINT8 column);
	virtual void key_break(UINT8 row, UINT8 column);
	virtual void will_scan_row(UINT8 row);

	bool are_all_keys_up();

private:
	// device_serial_interface uses 10'000 range
	enum {
		TIMER_ID_SCAN = 20'000,
		TIMER_ID_TYPEMATIC
	};

	void scan_row();

	emu_timer       *m_scan_timer;
	emu_timer       *m_typematic_timer;
	required_ioport m_key_rows[ROW_COUNT];
	ioport_value    m_key_states[ROW_COUNT];
	UINT8           m_next_row;
	UINT8           m_processing;
	UINT8           m_typematic_row;
	UINT8           m_typematic_column;
};


class generic_keyboard_device : public device_t, protected device_matrix_keyboard_interface<4U>
{
public:
	generic_keyboard_device(
			const machine_config &mconfig,
			device_type type,
			char const *name,
			char const *tag,
			device_t *owner,
			UINT32 clock,
			char const *shortname,
			char const *source);
	generic_keyboard_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			UINT32 clock);

	template <class Object> static devcb_base &set_keyboard_callback(device_t &device, Object object) { return downcast<generic_keyboard_device &>(device).m_keyboard_cb.set_callback(object); }

	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void key_make(UINT8 row, UINT8 column) override;
	virtual void key_repeat(UINT8 row, UINT8 column) override;
	virtual void send_key(UINT8 code);
	virtual bool translate(UINT8 code, UINT8 &translated) const;

	required_ioport m_config;
	required_ioport m_modifiers;

private:
	virtual void will_scan_row(UINT8 row) override;

	void typematic();
	void send_translated(UINT8 code);
	attotime typematic_delay() const;
	attotime typematic_period() const;

	UINT16          m_last_modifiers;
	devcb_write8    m_keyboard_cb;
};

#endif // MAME_DEVICES_MACHINE_KEYBOARD_H
