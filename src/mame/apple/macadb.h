// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_APPLE_MACADB_H
#define MAME_APPLE_MACADB_H

#pragma once

#include "emu.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> macadb_device

class macadb_device :  public device_t
{
public:
	// construction/destruction
	macadb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto adb_data_callback() { return write_adb_data.bind(); }
	auto adb_irq_callback() { return write_adb_irq.bind(); }

	required_ioport m_mouse0, m_mouse1, m_mouse2;
	required_ioport_array<8> m_keys;
	devcb_write_line write_adb_data, write_adb_irq;

	void adb_linechange_w(int state);

	void adb_vblank() {}

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u64 m_last_adb_time;
	emu_timer *m_timer;

	/* keyboard matrix to detect transition */
	u16 m_key_matrix[9];

	// ADB HLE state
	bool m_waiting_cmd;
	s32 m_datasize;
	s32 m_command, m_direction;
	u8 m_listenreg, m_listenaddr;
	s32 m_srq_switch, m_stream_ptr, m_linestate;
	u8 m_buffer[16], m_last_kbd[2], m_last_mouse[2];
	u8 m_keyboard_handler, m_mouse_handler;
	bool m_srqflag;
	s32 m_linein;

	static constexpr int kADBKeyBufSize = 32;
	u8 m_keybuf[kADBKeyBufSize];
	u8 m_keybuf_start;
	u8 m_keybuf_end;

	// ADB mouse state
	u8 m_mouseaddr;
	s32 m_lastmousex, m_lastmousey, m_lastbutton;

	// ADB keyboard state
	u8 m_keybaddr;
	s32 m_currentkeys[2], m_modifiers;

	bool adb_pollkbd(int update);
	bool adb_pollmouse();
	void adb_accummouse(u8 *MouseX, u8 *MouseY );
	void adb_talk();

	TIMER_CALLBACK_MEMBER(timer_tick);
};

// device type definition
DECLARE_DEVICE_TYPE(MACADB, macadb_device)

#endif // MAME_APPLE_MACADB_H
