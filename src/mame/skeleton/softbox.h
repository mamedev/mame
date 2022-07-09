// license:BSD-3-Clause
// copyright-holders:Curt Coder, Mike Naberezny
#ifndef MAME_INCLUDES_SOFTBOX_H
#define MAME_INCLUDES_SOFTBOX_H

#pragma once

#include "bus/ieee488/ieee488.h"
#include "bus/imi7000/imi7000.h"
#include "cpu/z80/z80.h"
#include "imagedev/harddriv.h"
#include "machine/corvushd.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "machine/i8255.h"

#define Z80_TAG         "z80"
#define I8251_TAG       "ic15"
#define I8255_0_TAG     "ic17"
#define I8255_1_TAG     "ic16"
#define COM8116_TAG     "ic14"
#define RS232_TAG       "rs232"
#define CORVUS_HDC_TAG  "corvus"

class softbox_state : public driver_device
{
public:
	softbox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_ieee(*this, IEEE488_TAG)
		, m_hdc(*this, CORVUS_HDC_TAG)
		, m_leds(*this, "led%u", 0U)
	{ }

	void softbox(machine_config &config);

private:
	// device_ieee488_interface overrides
	virtual void ieee488_ifc(int state);

	uint8_t ppi0_pa_r();
	void ppi0_pb_w(uint8_t data);

	uint8_t ppi1_pa_r();
	void ppi1_pb_w(uint8_t data);
	uint8_t ppi1_pc_r();
	void ppi1_pc_w(uint8_t data);

	enum
	{
		LED_A = 0,
		LED_B,
		LED_READY
	};

	void softbox_io(address_map &map);
	void softbox_mem(address_map &map);
	int m_ifc = 0;  // Tracks previous state of IEEE-488 IFC line

	virtual void machine_start() override;
	virtual void device_reset_after_children() override;

	required_device<cpu_device> m_maincpu;
	required_device<ieee488_device> m_ieee;
	required_device<corvus_hdc_device> m_hdc;
	output_finder<3> m_leds;
};

#endif
