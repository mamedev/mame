// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __PLUS4__
#define __PLUS4__

#include "emu.h"
#include "bus/cbmiec/cbmiec.h"
#include "bus/pet/cass.h"
#include "bus/plus4/exp.h"
#include "bus/plus4/user.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m7501.h"
#include "imagedev/snapquik.h"
#include "machine/mos6529.h"
#include "machine/mos6551.h"
#include "machine/mos8706.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "sound/mos7360.h"

#define MOS7501_TAG         "u2"
#define MOS7360_TAG         "u1"
#define MOS6551_TAG         "u3"
#define MOS6529_USER_TAG    "u5"
#define MOS6529_KB_TAG      "u27"
#define T6721A_TAG          "t6721a"
#define MOS8706_TAG         "mos8706"
#define PLA_TAG             "u19"
#define SCREEN_TAG          "screen"
#define CONTROL1_TAG        "joy1"
#define CONTROL2_TAG        "joy2"
#define PET_USER_PORT_TAG   "user"

class plus4_state : public driver_device
{
public:
	plus4_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, MOS7501_TAG),
			m_pla(*this, PLA_TAG),
			m_ted(*this, MOS7360_TAG),
			m_acia(*this, MOS6551_TAG),
			m_spi_user(*this, MOS6529_USER_TAG),
			m_spi_kb(*this, MOS6529_KB_TAG),
			m_vslsi(*this, MOS8706_TAG),
			m_iec(*this, CBM_IEC_TAG),
			m_joy1(*this, CONTROL1_TAG),
			m_joy2(*this, CONTROL2_TAG),
			m_exp(*this, PLUS4_EXPANSION_SLOT_TAG),
			m_user(*this, PET_USER_PORT_TAG),
			m_ram(*this, RAM_TAG),
			m_cassette(*this, PET_DATASSETTE_PORT_TAG),
			m_kernal(*this, "kernal"),
			m_function(*this, "function"),
			m_c2(*this, "c2"),
			m_row0(*this, "ROW0"),
			m_row1(*this, "ROW1"),
			m_row2(*this, "ROW2"),
			m_row3(*this, "ROW3"),
			m_row4(*this, "ROW4"),
			m_row5(*this, "ROW5"),
			m_row6(*this, "ROW6"),
			m_row7(*this, "ROW7"),
			m_lock(*this, "LOCK"),
			m_addr(0),
			m_ted_irq(CLEAR_LINE),
			m_acia_irq(CLEAR_LINE),
			m_exp_irq(CLEAR_LINE)
	{ }

	required_device<m7501_device> m_maincpu;
	required_device<pla_device> m_pla;
	required_device<mos7360_device> m_ted;
	optional_device<mos6551_device> m_acia;
	optional_device<mos6529_device> m_spi_user;
	required_device<mos6529_device> m_spi_kb;
	optional_device<mos8706_device> m_vslsi;
	required_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<plus4_expansion_slot_device> m_exp;
	optional_device<pet_user_port_device> m_user;
	required_device<ram_device> m_ram;
	required_device<pet_datassette_port_device> m_cassette;
	required_memory_region m_kernal;
	optional_memory_region m_function;
	optional_memory_region m_c2;
	required_ioport m_row0;
	required_ioport m_row1;
	required_ioport m_row2;
	required_ioport m_row3;
	required_ioport m_row4;
	required_ioport m_row5;
	required_ioport m_row6;
	required_ioport m_row7;
	required_ioport m_lock;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void check_interrupts();
	void bankswitch(offs_t offset, int phi0, int mux, int ras, int *scs, int *phi2, int *user, int *_6551, int *addr_clk, int *keyport, int *kernal);
	UINT8 read_memory(address_space &space, offs_t offset, int ba, int scs, int phi2, int user, int _6551, int addr_clk, int keyport, int kernal);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( ted_videoram_r );

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );

	DECLARE_WRITE_LINE_MEMBER( ted_irq_w );
	DECLARE_READ8_MEMBER( ted_k_r );

	DECLARE_WRITE_LINE_MEMBER( write_kb0 ) { if (state) m_kb |= 1; else m_kb &= ~1; }
	DECLARE_WRITE_LINE_MEMBER( write_kb1 ) { if (state) m_kb |= 2; else m_kb &= ~2; }
	DECLARE_WRITE_LINE_MEMBER( write_kb2 ) { if (state) m_kb |= 4; else m_kb &= ~4; }
	DECLARE_WRITE_LINE_MEMBER( write_kb3 ) { if (state) m_kb |= 8; else m_kb &= ~8; }
	DECLARE_WRITE_LINE_MEMBER( write_kb4 ) { if (state) m_kb |= 16; else m_kb &= ~16; }
	DECLARE_WRITE_LINE_MEMBER( write_kb5 ) { if (state) m_kb |= 32; else m_kb &= ~32; }
	DECLARE_WRITE_LINE_MEMBER( write_kb6 ) { if (state) m_kb |= 64; else m_kb &= ~64; }
	DECLARE_WRITE_LINE_MEMBER( write_kb7 ) { if (state) m_kb |= 128; else m_kb &= ~128; }

	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );

	DECLARE_WRITE_LINE_MEMBER( exp_irq_w );

	DECLARE_QUICKLOAD_LOAD_MEMBER( cbm_c16 );

	enum
	{
		CS0_BASIC = 0,
		CS0_FUNCTION_LO,
		CS0_C1_LOW,
		CS0_C2_LOW
	};

	enum
	{
		CS1_KERNAL = 0,
		CS1_FUNCTION_HI,
		CS1_C1_HIGH,
		CS1_C2_HIGH
	};

	// memory state
	UINT8 m_addr;

	// interrupt state
	int m_ted_irq;
	int m_acia_irq;
	int m_exp_irq;

	// keyboard state
	UINT8 m_kb;
};


class c16_state : public plus4_state
{
public:
	c16_state(const machine_config &mconfig, device_type type, std::string tag)
		: plus4_state(mconfig, type, tag)
	{ }

	DECLARE_READ8_MEMBER( cpu_r );
};



#endif
