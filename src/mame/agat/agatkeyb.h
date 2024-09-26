// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_AGAT_AGATKEYB_H
#define MAME_AGAT_AGATKEYB_H

#pragma once

#include "machine/keyboard.h"


/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

DECLARE_DEVICE_TYPE(AGAT_KEYBOARD, agat_keyboard_device)


/***************************************************************************
    TYPE DECLARATIONS
***************************************************************************/


class agat_keyboard_device : public device_t, protected device_matrix_keyboard_interface<6U>
{
public:
	agat_keyboard_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	auto out_callback() { return m_keyboard_cb.bind(); }
	auto out_meta_callback() { return m_out_meta_cb.bind(); }
	auto out_reset_callback() { return m_out_reset_cb.bind(); }

	DECLARE_INPUT_CHANGED_MEMBER(meta_changed);
	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);

protected:
	agat_keyboard_device(
		const machine_config &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_repeat(u8 row, u8 column) override;

	required_ioport m_modifiers;

private:
	virtual void will_scan_row(u8 row) override;

	bool translate(u8 code, u16 &translated) const;
	void send_translated(u8 code);
	attotime typematic_delay() const;
	attotime typematic_period() const;

	bool             m_meta = false;
	u16              m_last_modifiers;
	devcb_write8     m_keyboard_cb;
	devcb_write_line m_out_meta_cb;
	devcb_write_line m_out_reset_cb;
};

#endif // MAME_AGAT_AGATKEYB_H
