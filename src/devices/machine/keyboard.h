// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_MACHINE_KEYBOARD_H
#define MAME_MACHINE_KEYBOARD_H

#pragma once


/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

DECLARE_DEVICE_TYPE(GENERIC_KEYBOARD, generic_keyboard_device)



/***************************************************************************
    REUSABLE I/O PORTS
***************************************************************************/

INPUT_PORTS_EXTERN( generic_keyboard );



/***************************************************************************
    TYPE DECLARATIONS
***************************************************************************/

template <uint8_t ROW_COUNT>
class device_matrix_keyboard_interface : public device_interface
{
protected:
	template <typename... T>
	device_matrix_keyboard_interface(machine_config const &mconfig, device_t &device, T &&... tags);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;

	void start_processing(const attotime &period);
	void stop_processing();
	void reset_key_state();

	void typematic_start(u8 row, u8 column, attotime const &delay, attotime const &interval);
	void typematic_restart(attotime const &delay, attotime const &interval);
	void typematic_stop();
	bool typematic_is(u8 row, u8 column) const { return (m_typematic_row == row) && (m_typematic_column == column); }

	virtual void key_make(u8 row, u8 column) = 0;
	virtual void key_repeat(u8 row, u8 column);
	virtual void key_break(u8 row, u8 column);
	virtual void will_scan_row(u8 row);
	virtual void scan_complete();

	bool are_all_keys_up();

private:
	TIMER_CALLBACK_MEMBER(scan_row);
	TIMER_CALLBACK_MEMBER(typematic);

	emu_timer       *m_scan_timer;
	emu_timer       *m_typematic_timer;
	required_ioport m_key_rows[ROW_COUNT];
	ioport_value    m_key_states[ROW_COUNT];
	u8              m_next_row;
	u8              m_processing;
	u8              m_typematic_row;
	u8              m_typematic_column;
};


class generic_keyboard_device : public device_t, protected device_matrix_keyboard_interface<4U>
{
public:
	typedef device_delegate<void (u8 character)> output_delegate;

	generic_keyboard_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			u32 clock);
	virtual ~generic_keyboard_device();

	template <typename... T>
	void set_keyboard_callback(T &&... args)
	{
		m_keyboard_cb.set(std::forward<T>(args)...);
	}

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	generic_keyboard_device(
			const machine_config &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_repeat(u8 row, u8 column) override;
	virtual void send_key(u8 code);
	virtual bool translate(u8 code, u8 &translated) const;

	required_ioport m_config;
	required_ioport m_modifiers;

private:
	virtual void will_scan_row(u8 row) override;

	void typematic();
	void send_translated(u8 code);
	attotime typematic_delay() const;
	attotime typematic_period() const;

	u16             m_last_modifiers;
	output_delegate m_keyboard_cb;
};

#endif // MAME_MACHINE_KEYBOARD_H
