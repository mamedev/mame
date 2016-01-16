// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __PET__
#define __PET__

#include "emu.h"
#include "bus/ieee488/c8050.h"
#include "bus/ieee488/ieee488.h"
#include "bus/pet/cass.h"
#include "bus/pet/exp.h"
#include "bus/pet/user.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "imagedev/snapquik.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "sound/speaker.h"
#include "video/mc6845.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define M6502_TAG       "f3"
#define M6522_TAG       "a5"
#define M6520_1_TAG     "g8"
#define M6520_2_TAG     "b8"
#define MC6845_TAG      "ub13"
#define SCREEN_TAG      "screen"
#define PLA1_TAG        "ue6"
#define PLA2_TAG        "ue5"
#define PET_USER_PORT_TAG "user"

class pet_state : public driver_device
{
public:
	pet_state(const machine_config &mconfig, device_type type, std::string tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, M6502_TAG),
		m_via(*this, M6522_TAG),
		m_pia1(*this, M6520_1_TAG),
		m_pia2(*this, M6520_2_TAG),
		m_crtc(*this, MC6845_TAG),
		m_ieee(*this, IEEE488_TAG),
		m_palette(*this, "palette"),
		m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		m_cassette2(*this, PET_DATASSETTE_PORT2_TAG),
		m_exp(*this, PET_EXPANSION_SLOT_TAG),
		m_user(*this, PET_USER_PORT_TAG),
		m_speaker(*this, "speaker"),
		m_cart_9000(*this, "cart_9000"),
		m_cart_a000(*this, "cart_a000"),
		m_cart_b000(*this, "cart_b000"),
		m_ram(*this, RAM_TAG),
		m_rom(*this, M6502_TAG),
		m_char_rom(*this, "charom"),
		m_video_ram(*this, "video_ram"),
		m_row0(*this, "ROW0"),
		m_row1(*this, "ROW1"),
		m_row2(*this, "ROW2"),
		m_row3(*this, "ROW3"),
		m_row4(*this, "ROW4"),
		m_row5(*this, "ROW5"),
		m_row6(*this, "ROW6"),
		m_row7(*this, "ROW7"),
		m_row8(*this, "ROW8"),
		m_row9(*this, "ROW9"),
		m_lock(*this, "LOCK"),
		m_key(0),
		m_sync(0),
		m_graphic(0),
		m_blanktv(0),
		m_via_irq(CLEAR_LINE),
		m_pia1a_irq(CLEAR_LINE),
		m_pia1b_irq(CLEAR_LINE),
		m_pia2a_irq(CLEAR_LINE),
		m_pia2b_irq(CLEAR_LINE),
		m_exp_irq(CLEAR_LINE),
		m_user_diag(1)
	{ }

	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	optional_device<mc6845_device> m_crtc;
	required_device<ieee488_device> m_ieee;
	required_device<palette_device> m_palette;
	required_device<pet_datassette_port_device> m_cassette;
	required_device<pet_datassette_port_device> m_cassette2;
	required_device<pet_expansion_slot_device> m_exp;
	required_device<pet_user_port_device> m_user;
	optional_device<speaker_sound_device> m_speaker;
	optional_device<generic_slot_device> m_cart_9000;
	optional_device<generic_slot_device> m_cart_a000;
	optional_device<generic_slot_device> m_cart_b000;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	optional_shared_ptr<UINT8> m_video_ram;
	required_ioport m_row0;
	required_ioport m_row1;
	required_ioport m_row2;
	required_ioport m_row3;
	required_ioport m_row4;
	required_ioport m_row5;
	required_ioport m_row6;
	required_ioport m_row7;
	required_ioport m_row8;
	required_ioport m_row9;
	required_ioport m_lock;

	DECLARE_MACHINE_START( pet );
	DECLARE_MACHINE_START( pet2001 );
	DECLARE_MACHINE_RESET( pet );
	DECLARE_MACHINE_START( pet40 );
	DECLARE_MACHINE_RESET( pet40 );

	MC6845_BEGIN_UPDATE( pet_begin_update );
	MC6845_UPDATE_ROW( pet40_update_row );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void check_interrupts();
	void update_speaker();

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( via_irq_w );
	DECLARE_WRITE8_MEMBER( via_pa_w );
	DECLARE_READ8_MEMBER( via_pb_r );
	DECLARE_WRITE8_MEMBER( via_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via_ca2_w );
	DECLARE_WRITE_LINE_MEMBER( via_cb2_w );

	DECLARE_WRITE_LINE_MEMBER( pia1_irqa_w );
	DECLARE_WRITE_LINE_MEMBER( pia1_irqb_w );
	DECLARE_READ8_MEMBER( pia1_pa_r );
	DECLARE_READ8_MEMBER( pia1_pb_r );
	DECLARE_WRITE8_MEMBER( pia1_pa_w );
	DECLARE_WRITE_LINE_MEMBER( pia1_ca2_w );

	DECLARE_WRITE_LINE_MEMBER( pia2_irqa_w );
	DECLARE_WRITE_LINE_MEMBER( pia2_irqb_w );

	DECLARE_WRITE_LINE_MEMBER( user_diag_w );

	TIMER_DEVICE_CALLBACK_MEMBER( sync_tick );

	DECLARE_QUICKLOAD_LOAD_MEMBER( cbm_pet );

	enum
	{
		SEL0 = 0,
		SEL1,
		SEL2,
		SEL3,
		SEL4,
		SEL5,
		SEL6,
		SEL7,
		SEL8,
		SEL9,
		SELA,
		SELB,
		SELC,
		SELD,
		SELE,
		SELF
	};

	// keyboard state
	UINT8 m_key;

	// video state
	int m_sync;
	int m_graphic;
	int m_blanktv;
	int m_video_ram_size;

	// sound state
	int m_via_cb2;
	int m_pia1_pa7;

	UINT8 m_via_pa;

	// interrupt state
	int m_via_irq;
	int m_pia1a_irq;
	int m_pia1b_irq;
	int m_pia2a_irq;
	int m_pia2b_irq;
	int m_exp_irq;
	int m_user_diag;
};


class pet2001b_state : public pet_state
{
public:
	pet2001b_state(const machine_config &mconfig, device_type type, std::string tag) :
		pet_state(mconfig, type, tag)
	{ }

	DECLARE_READ8_MEMBER( pia1_pb_r );
};


class pet80_state : public pet2001b_state
{
public:
	pet80_state(const machine_config &mconfig, device_type type, std::string tag) :
		pet2001b_state(mconfig, type, tag)
	{ }

	DECLARE_MACHINE_START( pet80 );
	DECLARE_MACHINE_RESET( pet80 );

	MC6845_UPDATE_ROW( pet80_update_row );
	MC6845_UPDATE_ROW( cbm8296_update_row );
};


class superpet_state : public pet80_state
{
public:
	superpet_state(const machine_config &mconfig, device_type type, std::string tag)
		: pet80_state(mconfig, type, tag)
	{ }
};


class cbm8096_state : public pet80_state
{
public:
	cbm8096_state(const machine_config &mconfig, device_type type, std::string tag) :
		pet80_state(mconfig, type, tag)
	{ }
};


class cbm8296_state : public pet80_state
{
public:
	cbm8296_state(const machine_config &mconfig, device_type type, std::string tag) :
		pet80_state(mconfig, type, tag),
		m_basic_rom(*this, "basic"),
		m_editor_rom(*this, "editor"),
		m_ue5_rom(*this, "ue5_eprom"),
		m_ue6_rom(*this, "ue6_eprom"),
		m_pla1(*this, PLA1_TAG),
		m_pla2(*this, PLA2_TAG)
	{ }

	required_memory_region m_basic_rom;
	required_memory_region m_editor_rom;
	required_memory_region m_ue5_rom;
	required_memory_region m_ue6_rom;
	required_device<pla_device> m_pla1;
	required_device<pla_device> m_pla2;

	DECLARE_MACHINE_START( cbm8296 );
	DECLARE_MACHINE_RESET( cbm8296 );

	void read_pla1(offs_t offset, int phi2, int brw, int noscreen, int noio, int ramsela, int ramsel9, int ramon, int norom,
		int &cswff, int &cs9, int &csa, int &csio, int &cse, int &cskb, int &fa12, int &casena1);
	void read_pla2(offs_t offset, int phi2, int brw, int casena1, int &endra, int &noscreen, int &casena2, int &fa15);

	void read_pla1_eprom(offs_t offset, int phi2, int brw, int noscreen, int noio, int ramsela, int ramsel9, int ramon, int norom,
		int &cswff, int &cs9, int &csa, int &csio, int &cse, int &cskb, int &fa12, int &casena1);
	void read_pla2_eprom(offs_t offset, int phi2, int brw, int casena1, int &endra, int &noscreen, int &casena2, int &fa15);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 m_cr;
};



#endif
