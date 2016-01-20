// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Super Nintendo Entertainment System - Miracle Piano Keyboard

**********************************************************************/

#pragma once

#ifndef __SNES_MIRACLE__
#define __SNES_MIRACLE__


#include "emu.h"
#include "ctrl.h"
#include "bus/midi/midi.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> snes_miracle_device

class snes_miracle_device : public device_t,
							public device_serial_interface,
							public device_snes_control_port_interface
{
public:
	static const int XMIT_RING_SIZE = 64;
	static const int RECV_RING_SIZE = 64;

	// construction/destruction
	snes_miracle_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	void xmit_char(UINT8 data);

	required_device<midi_port_device> m_midiin, m_midiout;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT8 read_pin4() override;
	void write_pin6(UINT8 data) override;
	void write_strobe(UINT8 data) override;

	static const device_timer_id TIMER_STROBE_ON = 0;
	emu_timer *strobe_timer;

	int m_strobe_on, m_midi_mode, m_sent_bits;
	UINT32 m_strobe_clock;
	UINT8 m_data_sent;
	UINT8 m_xmitring[XMIT_RING_SIZE], m_recvring[RECV_RING_SIZE];
	int m_xmit_read, m_xmit_write;
	int m_recv_read, m_recv_write;
	bool m_tx_busy, m_read_status, m_status_bit;
};

// device type definition
extern const device_type SNES_MIRACLE;

#endif
