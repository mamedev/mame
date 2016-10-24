// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __POLY880__
#define __POLY880__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/ram.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "i1"
#define Z80CTC_TAG      "i4"
#define Z80PIO1_TAG     "i2"
#define Z80PIO2_TAG     "i3"

class poly880_state : public driver_device
{
public:
	poly880_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_cassette(*this, "cassette"),
			m_ki1(*this, "KI1"),
			m_ki2(*this, "KI2"),
			m_ki3(*this, "KI3")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_ki1;
	required_ioport m_ki2;
	required_ioport m_ki3;

	virtual void machine_start() override;

	void cldig_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void pio1_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pio1_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio1_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void trigger_reset(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void trigger_nmi(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	void update_display();

	/* display state */
	uint8_t m_digit;
	uint8_t m_segment;
};

#endif
