// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __VIC10__
#define __VIC10__

#include "emu.h"
#include "bus/pet/cass.h"
#include "bus/vic10/exp.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m6510.h"
#include "machine/mos6526.h"
#include "machine/ram.h"
#include "sound/dac.h"
#include "sound/mos6581.h"
#include "video/mos6566.h"

#define M6510_TAG       "u3"
#define MOS6566_TAG     "u2"
#define MOS6581_TAG     "u6"
#define MOS6526_TAG     "u9"
#define SCREEN_TAG      "screen"
#define TIMER_C1531_TAG "c1531"
#define CONTROL1_TAG    "joy1"
#define CONTROL2_TAG    "joy2"

class vic10_state : public driver_device
{
public:
	vic10_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M6510_TAG),
			m_vic(*this, MOS6566_TAG),
			m_sid(*this, MOS6581_TAG),
			m_cia(*this, MOS6526_TAG),
			m_joy1(*this, CONTROL1_TAG),
			m_joy2(*this, CONTROL2_TAG),
			m_exp(*this, VIC10_EXPANSION_SLOT_TAG),
			m_ram(*this, RAM_TAG),
			m_cassette(*this, PET_DATASSETTE_PORT_TAG),
			m_color_ram(*this, "color_ram"),
			m_row0(*this, "ROW0"),
			m_row1(*this, "ROW1"),
			m_row2(*this, "ROW2"),
			m_row3(*this, "ROW3"),
			m_row4(*this, "ROW4"),
			m_row5(*this, "ROW5"),
			m_row6(*this, "ROW6"),
			m_row7(*this, "ROW7"),
			m_restore(*this, "RESTORE"),
			m_lock(*this, "LOCK"),
			m_cia_irq(CLEAR_LINE),
			m_vic_irq(CLEAR_LINE),
			m_exp_irq(CLEAR_LINE)
	{ }

	required_device<m6510_device> m_maincpu;
	required_device<mos6566_device> m_vic;
	required_device<mos6581_device> m_sid;
	required_device<mos6526_device> m_cia;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<vic10_expansion_slot_device> m_exp;
	required_device<ram_device> m_ram;
	optional_device<pet_datassette_port_device> m_cassette;
	optional_shared_ptr<UINT8> m_color_ram;
	required_ioport m_row0;
	required_ioport m_row1;
	required_ioport m_row2;
	required_ioport m_row3;
	required_ioport m_row4;
	required_ioport m_row5;
	required_ioport m_row6;
	required_ioport m_row7;
	required_ioport m_restore;
	required_ioport m_lock;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void check_interrupts();

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( vic_irq_w );
	DECLARE_READ8_MEMBER( vic_videoram_r );
	DECLARE_READ8_MEMBER( vic_colorram_r );

	DECLARE_READ8_MEMBER( sid_potx_r );
	DECLARE_READ8_MEMBER( sid_poty_r );

	DECLARE_WRITE_LINE_MEMBER( cia_irq_w );
	DECLARE_READ8_MEMBER( cia_pa_r );
	DECLARE_READ8_MEMBER( cia_pb_r );
	DECLARE_WRITE8_MEMBER( cia_pb_w );

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );

	DECLARE_WRITE_LINE_MEMBER( exp_irq_w );
	DECLARE_WRITE_LINE_MEMBER( exp_reset_w );

	// interrupt state
	int m_cia_irq;
	int m_vic_irq;
	int m_exp_irq;
};



#endif
