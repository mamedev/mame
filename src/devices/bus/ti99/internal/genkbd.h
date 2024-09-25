// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/**********************************************************************

    Geneve 9640 101-key XT/AT keyboard (High-level emulation)

*********************************************************************/

#ifndef MAME_BUS_TI99_INTERNAL_GENKBD_H
#define MAME_BUS_TI99_INTERNAL_GENKBD_H

#pragma once

#include "bus/pc_kbd/pc_kbdc.h"

#define STR_KBD_GENEVE_XT_101_HLE           "geneve_kb_101"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> geneve_xt_101_hle_keyboard_device

class geneve_xt_101_hle_keyboard_device : public device_t, public device_pc_kbd_interface
{
public:
	// construction/destruction
	geneve_xt_101_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void reset_line(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_pc_kbd_interface overrides
	virtual void clock_write(int state) override;
	virtual void data_write(int state) override;

	TIMER_CALLBACK_MEMBER(poll_tick);
	TIMER_CALLBACK_MEMBER(send_tick);

private:
	emu_timer   *m_poll_timer;
	emu_timer   *m_send_timer;

	static constexpr unsigned KEYQUEUESIZE = 256;
	static constexpr unsigned MAXKEYMSGLENGTH = 10;
	static constexpr unsigned KEYAUTOREPEATDELAY = 30;
	static constexpr unsigned KEYAUTOREPEATRATE = 6;

	void poll();
	void send_key();
	void post_in_key_queue(int key);

	required_ioport_array<8> m_keys;

	int m_queue_length;
	int     m_queue_head;
	uint8_t   m_queue[KEYQUEUESIZE];
	uint32_t  m_key_state_save[4];
	int m_autorepeat_code;
	int m_autorepeat_timer;
	bool m_fake_shift_state;
	bool m_fake_unshift_state;

	bool m_left_shift;
	bool m_right_shift;
	bool m_left_ctrl;
	bool m_right_ctrl;
	bool m_left_alt;
	bool m_altgr;
	bool m_numlock;

	bool m_resetting;

	line_state m_clock_line;
	line_state m_data_line;
	int m_reset_timer;

	int m_shift_reg;
	int m_shift_count;
};


// device type definition
DECLARE_DEVICE_TYPE(KBD_GENEVE_XT_101_HLE, geneve_xt_101_hle_keyboard_device)

#endif // MAME_BUS_TI99_INTERNAL_GENKBD_H
