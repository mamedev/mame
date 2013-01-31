#pragma once

#ifndef __PET2001__
#define __PET2001__

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "machine/6522via.h"
#include "machine/cbmipt.h"
#include "machine/ieee488.h"
#include "machine/petcass.h"
#include "machine/ram.h"
#include "video/mc6845.h"

#define M6502_TAG 		"f3"
#define M6522_TAG 		"a5"
#define M6520_1_TAG 	"g8"
#define M6520_2_TAG 	"b8"
#define SCREEN_TAG		"screen"

class pet2001_state : public driver_device
{
public:
	pet2001_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M6502_TAG),
			m_via(*this, M6522_TAG),
			m_pia1(*this, M6520_1_TAG),
			m_pia2(*this, M6520_2_TAG),
			m_ieee(*this, IEEE488_TAG),
			m_cassette(*this, PET_DATASSETTE_PORT_TAG),
			m_cassette2(*this, PET_DATASSETTE_PORT2_TAG),
			//m_exp(*this, PET_EXPANSION_SLOT_TAG),
			//m_user(*this, PET_USER_PORT_TAG),
			m_ram(*this, RAM_TAG),
			m_rom(*this, M6502_TAG),
			m_spare_rom(*this, "spare"),
			m_char_rom(*this, "gfx1"),
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
			m_exp_irq(CLEAR_LINE)
	{ }

	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	required_device<ieee488_device> m_ieee;
	required_device<pet_datassette_port_device> m_cassette;
	required_device<pet_datassette_port_device> m_cassette2;
	//required_device<pet_expansion_slot_device> m_exp;
	//required_device<pet_user_port_device> m_user;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	optional_memory_region m_spare_rom;
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

	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void check_interrupts();

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( via_irq_w );
	DECLARE_READ8_MEMBER( via_pb_r );
	DECLARE_WRITE8_MEMBER( via_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via_ca2_w );

	DECLARE_WRITE_LINE_MEMBER( pia1_irqa_w );
	DECLARE_WRITE_LINE_MEMBER( pia1_irqb_w );
	DECLARE_READ8_MEMBER( pia1_pa_r );
	DECLARE_READ8_MEMBER( pia1_pb_r );
	DECLARE_WRITE8_MEMBER( pia1_pa_w );
	DECLARE_WRITE_LINE_MEMBER( pia1_ca2_w );
	DECLARE_READ_LINE_MEMBER( pia1_cb1_r );

	DECLARE_WRITE_LINE_MEMBER( pia2_irqa_w );
	DECLARE_WRITE_LINE_MEMBER( pia2_irqb_w );

	TIMER_DEVICE_CALLBACK_MEMBER( sync_tick );

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

	// interrupt state
	int m_via_irq;
	int m_pia1a_irq;
	int m_pia1b_irq;
	int m_pia2a_irq;
	int m_pia2b_irq;
	int m_exp_irq;
};


class pet2001b_state : public pet2001_state
{
public:
	pet2001b_state(const machine_config &mconfig, device_type type, const char *tag)
		: pet2001_state(mconfig, type, tag)
	{ }

	DECLARE_READ8_MEMBER( pia1_pb_r );
};



#endif