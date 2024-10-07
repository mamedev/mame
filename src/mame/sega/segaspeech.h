// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega speech board

*************************************************************************/
#ifndef MAME_SEGA_SEGASPEECH_H
#define MAME_SEGA_SEGASPEECH_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/netlist.h"
#include "machine/timer.h"

class sega_speech_device : public device_t, public device_mixer_interface
{
public:
	sega_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

//  auto int_cb() { return m_int_cb.bind(); }

	void data_w(uint8_t data);
	void control_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	int t0_r();
	int t1_r();
	uint8_t p1_r();
	uint8_t rom_r(offs_t offset);
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);

	void drq_w(int state);

private:
	void speech_map(address_map &map) ATTR_COLD;
	void speech_portmap(address_map &map) ATTR_COLD;

	required_memory_region m_speech;
	required_device<cpu_device> m_cpu;

	// internal state
	u8 m_drq;
	u8 m_latch;
	u8 m_t0;
	u8 m_p2;

	TIMER_CALLBACK_MEMBER( delayed_speech_w );
};

DECLARE_DEVICE_TYPE(SEGA_SPEECH_BOARD, sega_speech_device)

#endif // MAME_SEGA_SEGASPEECH_H
