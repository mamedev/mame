// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_MACHINE_AGATKEYB_H
#define MAME_MACHINE_AGATKEYB_H

#pragma once

#include "machine/keyboard.h"


/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

DECLARE_DEVICE_TYPE(AGAT_KEYBOARD, agat_keyboard_device)


/***************************************************************************
    REUSABLE I/O PORTS
***************************************************************************/

INPUT_PORTS_EXTERN( agat_keyboard );


/***************************************************************************
    TYPE DECLARATIONS
***************************************************************************/


class agat_keyboard_device : public device_t, protected device_matrix_keyboard_interface<6U>
{
public:
	typedef device_delegate<void (u16 character)> output_delegate;

	agat_keyboard_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock);

	template <typename... T>
	void set_keyboard_callback(T &&... args)
	{
		m_keyboard_cb.set(std::forward<T>(args)...);
	}

	virtual ioport_constructor device_input_ports() const override;

	auto out_meta_callback() { return m_out_meta_cb.bind(); }

	DECLARE_INPUT_CHANGED_MEMBER(meta_changed);

protected:
	agat_keyboard_device(
		const machine_config &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock);
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_repeat(u8 row, u8 column) override;
	virtual void send_key(u16 code);
	virtual bool translate(u8 code, u16 &translated) const;

	required_ioport m_modifiers;

private:
	virtual void will_scan_row(u8 row) override;

	void send_translated(u8 code);
	attotime typematic_delay() const;
	attotime typematic_period() const;

	bool			 m_meta;
	u16              m_last_modifiers;
	output_delegate  m_keyboard_cb;
	devcb_write_line m_out_meta_cb;
};

#endif // MAME_MACHINE_AGATKEYB_H
