// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __C64__
#define __C64__

#include "emu.h"
#include "bus/cbmiec/cbmiec.h"
#include "bus/c64/exp.h"
#include "bus/vic20/user.h"
#include "bus/pet/cass.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m6510.h"
#include "imagedev/snapquik.h"
#include "machine/mos6526.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "sound/mos6581.h"
#include "video/mos6566.h"

#define M6510_TAG       "u7"
#define MOS6567_TAG     "u19"
#define MOS6569_TAG     "u19"
#define MOS6581_TAG     "u18"
#define MOS6526_1_TAG   "u1"
#define MOS6526_2_TAG   "u2"
#define PLA_TAG         "u17"
#define SCREEN_TAG      "screen"
#define CONTROL1_TAG    "joy1"
#define CONTROL2_TAG    "joy2"
#define PET_USER_PORT_TAG     "user"

class c64_state : public driver_device
{
public:
	c64_state(const machine_config &mconfig, device_type type, std::string tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, M6510_TAG),
		m_pla(*this, PLA_TAG),
		m_vic(*this, MOS6569_TAG),
		m_sid(*this, MOS6581_TAG),
		m_cia1(*this, MOS6526_1_TAG),
		m_cia2(*this, MOS6526_2_TAG),
		m_iec(*this, CBM_IEC_TAG),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG),
		m_exp(*this, C64_EXPANSION_SLOT_TAG),
		m_user(*this, PET_USER_PORT_TAG),
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
		m_lock(*this, "LOCK"),
		m_loram(1),
		m_hiram(1),
		m_charen(1),
		m_va14(1),
		m_va15(1),
		m_restore(1),
		m_cia1_irq(CLEAR_LINE),
		m_cia2_irq(CLEAR_LINE),
		m_vic_irq(CLEAR_LINE),
		m_exp_irq(CLEAR_LINE),
		m_exp_nmi(CLEAR_LINE)
	{ }

	// ROM
	UINT8 *m_basic;
	UINT8 *m_kernal;
	UINT8 *m_charom;

	required_device<m6510_device> m_maincpu;
	required_device<pla_device> m_pla;
	required_device<mos6566_device> m_vic;
	required_device<mos6581_device> m_sid;
	required_device<mos6526_device> m_cia1;
	required_device<mos6526_device> m_cia2;
	optional_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<c64_expansion_slot_device> m_exp;
	required_device<pet_user_port_device> m_user;
	required_device<ram_device> m_ram;
	optional_device<pet_datassette_port_device> m_cassette;
	optional_shared_ptr<UINT8> m_color_ram;
	optional_ioport m_row0;
	optional_ioport m_row1;
	optional_ioport m_row2;
	optional_ioport m_row3;
	optional_ioport m_row4;
	optional_ioport m_row5;
	optional_ioport m_row6;
	optional_ioport m_row7;
	optional_ioport m_lock;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void check_interrupts();
	int read_pla(offs_t offset, offs_t va, int rw, int aec, int ba);
	UINT8 read_memory(address_space &space, offs_t offset, offs_t va, int aec, int ba);
	void write_memory(address_space &space, offs_t offset, UINT8 data, int aec, int ba);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( vic_videoram_r );
	DECLARE_READ8_MEMBER( vic_colorram_r );
	DECLARE_WRITE_LINE_MEMBER( vic_irq_w );

	DECLARE_READ8_MEMBER( sid_potx_r );
	DECLARE_READ8_MEMBER( sid_poty_r );

	DECLARE_WRITE_LINE_MEMBER( cia1_irq_w );
	DECLARE_READ8_MEMBER( cia1_pa_r );
	DECLARE_WRITE8_MEMBER( cia1_pa_w );
	DECLARE_READ8_MEMBER( cia1_pb_r );
	DECLARE_WRITE8_MEMBER( cia1_pb_w );

	DECLARE_WRITE_LINE_MEMBER( cia2_irq_w );
	DECLARE_READ8_MEMBER( cia2_pa_r );
	DECLARE_WRITE8_MEMBER( cia2_pa_w );

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );

	DECLARE_WRITE_LINE_MEMBER( write_restore );
	DECLARE_WRITE_LINE_MEMBER( exp_irq_w );
	DECLARE_WRITE_LINE_MEMBER( exp_nmi_w );
	DECLARE_WRITE_LINE_MEMBER( exp_dma_w );
	DECLARE_WRITE_LINE_MEMBER( exp_reset_w );

	DECLARE_QUICKLOAD_LOAD_MEMBER( cbm_c64 );

	DECLARE_READ8_MEMBER( cia2_pb_r );
	DECLARE_WRITE8_MEMBER( cia2_pb_w );

	DECLARE_WRITE_LINE_MEMBER( write_user_pa2 ) { m_user_pa2 = state; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb0 ) { if (state) m_user_pb |= 1; else m_user_pb &= ~1; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb1 ) { if (state) m_user_pb |= 2; else m_user_pb &= ~2; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb2 ) { if (state) m_user_pb |= 4; else m_user_pb &= ~4; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb3 ) { if (state) m_user_pb |= 8; else m_user_pb &= ~8; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb4 ) { if (state) m_user_pb |= 16; else m_user_pb &= ~16; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb5 ) { if (state) m_user_pb |= 32; else m_user_pb &= ~32; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb6 ) { if (state) m_user_pb |= 64; else m_user_pb &= ~64; }
	DECLARE_WRITE_LINE_MEMBER( write_user_pb7 ) { if (state) m_user_pb |= 128; else m_user_pb &= ~128; }

	// memory state
	int m_loram;
	int m_hiram;
	int m_charen;

	// video state
	int m_va14;
	int m_va15;

	// interrupt state
	int m_restore;
	int m_cia1_irq;
	int m_cia2_irq;
	int m_vic_irq;
	int m_exp_irq;
	int m_exp_nmi;
	int m_exp_dma;

	int m_user_pa2;
	int m_user_pb;
};


class sx64_state : public c64_state
{
public:
	sx64_state(const machine_config &mconfig, device_type type, std::string tag)
		: c64_state(mconfig, type, tag)
	{ }

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );
};


class c64c_state : public c64_state
{
public:
	c64c_state(const machine_config &mconfig, device_type type, std::string tag)
		: c64_state(mconfig, type, tag)
	{ }
};


class c64gs_state : public c64c_state
{
public:
	c64gs_state(const machine_config &mconfig, device_type type, std::string tag)
		: c64c_state(mconfig, type, tag)
	{ }

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );

	DECLARE_READ8_MEMBER( cia1_pa_r );
	DECLARE_READ8_MEMBER( cia1_pb_r );
};



#endif
