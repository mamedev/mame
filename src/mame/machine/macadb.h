// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_MACADB_H
#define MAME_MACHINE_MACADB_H

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
	macadb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_mcu_mode(bool bMCUMode) { m_bIsMCUMode = bMCUMode; }

	auto via_clock_callback() { return write_via_clock.bind(); }
	auto via_data_callback() { return write_via_data.bind(); }
	auto adb_data_callback() { return write_adb_data.bind(); }
	auto adb_irq_callback() { return write_adb_irq.bind(); }

	required_ioport m_mouse0, m_mouse1, m_mouse2;
	required_ioport_array<6> m_keys;
	devcb_write_line write_via_clock, write_via_data, write_adb_data, write_adb_irq;

	DECLARE_WRITE_LINE_MEMBER(adb_data_w);
	DECLARE_WRITE_LINE_MEMBER(adb_linechange_w);

	void adb_vblank();
	void mac_adb_newaction(int state);
	int32_t get_adb_state(void) { return m_adb_state; }

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	bool m_bIsMCUMode;

	uint64_t m_last_adb_time;

	emu_timer *m_adb_timer;

	/* keyboard matrix to detect transition */
	int m_key_matrix[7];

	// ADB HLE state
	int32_t m_adb_state, m_adb_waiting_cmd, m_adb_datasize, m_adb_buffer[257];
	int32_t m_adb_command, m_adb_send, m_adb_timer_ticks, m_adb_extclock, m_adb_direction;
	int32_t m_adb_listenreg, m_adb_listenaddr, m_adb_last_talk, m_adb_srq_switch;
	int32_t m_adb_stream_ptr;
	int32_t m_adb_linestate;
	bool  m_adb_srqflag;
	int m_adb_linein;

	#define kADBKeyBufSize 32
	uint8_t m_adb_keybuf[kADBKeyBufSize];
	uint8_t m_adb_keybuf_start;
	uint8_t m_adb_keybuf_end;

	// ADB mouse state
	int m_adb_mouseaddr;
	int m_adb_lastmousex, m_adb_lastmousey, m_adb_lastbutton, m_adb_mouse_initialized;

	// ADB keyboard state
	int m_adb_keybaddr;
	int m_adb_keybinitialized, m_adb_currentkeys[2], m_adb_modifiers;

	int adb_pollkbd(int update);
	int adb_pollmouse();
	void adb_accummouse( uint8_t *MouseX, uint8_t *MouseY );
	void adb_talk();

	inline void set_adb_line(int linestate) { write_adb_data(linestate); }

	TIMER_CALLBACK_MEMBER(mac_adb_tick);
};

// device type definition
DECLARE_DEVICE_TYPE(MACADB, macadb_device)

#endif // MAME_MACHINE_MACADB_H
