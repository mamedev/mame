// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Super Nintendo Entertainment System - Miracle Piano Keyboard

**********************************************************************/

#ifndef MAME_BUS_SNES_CTRL_MIRACLE_H
#define MAME_BUS_SNES_CTRL_MIRACLE_H

#pragma once

#include "ctrl.h"
#include "bus/midi/midi.h"
#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> snes_miracle_device

class snes_miracle_device : public device_t,
							public device_serial_interface,
							public device_snes_control_port_interface
{
public:
	// construction/destruction
	snes_miracle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	static constexpr int XMIT_RING_SIZE = 64;
	static constexpr int RECV_RING_SIZE = 64;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	uint8_t read_pin4() override;
	void write_pin6(uint8_t data) override;
	void write_strobe(uint8_t data) override;

	void xmit_char(uint8_t data);

	TIMER_CALLBACK_MEMBER(strobe_tick);

	required_device<midi_port_device> m_midiin, m_midiout;

	emu_timer *strobe_timer;

	int m_strobe_on, m_midi_mode, m_sent_bits;
	uint32_t m_strobe_clock;
	uint8_t m_data_sent;
	uint8_t m_xmitring[XMIT_RING_SIZE], m_recvring[RECV_RING_SIZE];
	int m_xmit_read, m_xmit_write;
	int m_recv_read, m_recv_write;
	bool m_tx_busy, m_read_status, m_status_bit;
};

// device type definition
DECLARE_DEVICE_TYPE(SNES_MIRACLE, snes_miracle_device)

#endif // MAME_BUS_SNES_CTRL_MIRACLE_H
