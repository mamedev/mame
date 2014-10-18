// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __VIC20__
#define __VIC20__

#include "emu.h"
#include "bus/cbmiec/cbmiec.h"
#include "bus/pet/cass.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "bus/vic20/exp.h"
#include "bus/vic20/user.h"
#include "cpu/m6502/m6510.h"
#include "machine/6522via.h"
#include "machine/cbm_snqk.h"
#include "machine/ram.h"
#include "sound/dac.h"
#include "sound/mos6560.h"

#define M6502_TAG       "ue10"
#define M6522_1_TAG     "uab3"
#define M6522_2_TAG     "uab1"
#define M6560_TAG       "ub7"
#define M6561_TAG       "ub7"
#define IEC_TAG         "iec"
#define SCREEN_TAG      "screen"
#define CONTROL1_TAG    "joy1"
#define PET_USER_PORT_TAG     "user"

class vic20_state : public driver_device
{
public:
	vic20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M6502_TAG),
			m_via1(*this, M6522_1_TAG),
			m_via2(*this, M6522_2_TAG),
			m_vic(*this, M6560_TAG),
			m_iec(*this, CBM_IEC_TAG),
			m_joy(*this, CONTROL1_TAG),
			m_exp(*this, VIC20_EXPANSION_SLOT_TAG),
			m_user(*this, PET_USER_PORT_TAG),
			m_cassette(*this, PET_DATASSETTE_PORT_TAG),
			m_ram(*this, RAM_TAG),
			m_basic(*this, "basic"),
			m_kernal(*this, "kernal"),
			m_charom(*this, "charom"),
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
			m_lock(*this, "LOCK")
	{ }

	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
	required_device<mos6560_device> m_vic;
	required_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy;
	required_device<vic20_expansion_slot_device> m_exp;
	required_device<pet_user_port_device> m_user;
	required_device<pet_datassette_port_device> m_cassette;
	required_device<ram_device> m_ram;
	required_region_ptr<UINT8> m_basic;
	required_region_ptr<UINT8> m_kernal;
	required_region_ptr<UINT8> m_charom;
	required_shared_ptr<UINT8> m_color_ram;
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

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( vic_videoram_r );
	DECLARE_READ8_MEMBER( vic_lightx_cb );
	DECLARE_READ8_MEMBER( vic_lighty_cb );
	DECLARE_READ8_MEMBER( vic_lightbut_cb );

	DECLARE_WRITE_LINE_MEMBER( write_light_pen );
	DECLARE_WRITE_LINE_MEMBER( write_user_joy0 );
	DECLARE_WRITE_LINE_MEMBER( write_user_joy1 );
	DECLARE_WRITE_LINE_MEMBER( write_user_joy2 );
	DECLARE_WRITE_LINE_MEMBER( write_user_light_pen );
	DECLARE_WRITE_LINE_MEMBER( write_user_cassette_switch );

	DECLARE_READ8_MEMBER( via1_pa_r );
	DECLARE_WRITE8_MEMBER( via1_pa_w );
	DECLARE_WRITE8_MEMBER( via1_pb_w );

	DECLARE_READ8_MEMBER( via2_pa_r );
	DECLARE_READ8_MEMBER( via2_pb_r );
	DECLARE_WRITE8_MEMBER( via2_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via2_ca2_w );
	DECLARE_WRITE_LINE_MEMBER( via2_cb2_w );

	DECLARE_WRITE_LINE_MEMBER( exp_reset_w );

	DECLARE_QUICKLOAD_LOAD_MEMBER( cbm_vc20 );
	// keyboard state
	int m_key_col;
	int m_light_pen;
	int m_user_joy0;
	int m_user_joy1;
	int m_user_joy2;
	int m_user_light_pen;
	int m_user_cassette_switch;

	enum
	{
		BLK0 = 0,
		BLK1,
		BLK2,
		BLK3,
		BLK4,
		BLK5,
		BLK6,
		BLK7
	};


	enum
	{
		RAM0 = 0,
		RAM1,
		RAM2,
		RAM3,
		RAM4,
		RAM5,
		RAM6,
		RAM7
	};


	enum
	{
		IO0 = 4,
		COLOR = 5,
		IO2 = 6,
		IO3 = 7
	};
};

#endif
