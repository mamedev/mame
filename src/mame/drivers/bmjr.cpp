// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Basic Master Jr (MB-6885) (c) 1982? Hitachi

    preliminary driver by Angelo Salese

    TODO:
    - for whatever reason, BASIC won't work if you try to use it directly,
      it does if you enter into MON first then exit (with E)
    - tape hook-up doesn't work yet (shouldn't be hard to fix)
    - Break key is unemulated (tied with the NMI)

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/beep.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"


class bmjr_state : public driver_device
{
public:
	bmjr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_cass(*this, "cassette"),
	m_beep(*this, "beeper")
	,
		m_p_wram(*this, "p_wram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<beep_device> m_beep;
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_WRITE8_MEMBER(key_w);
	DECLARE_READ8_MEMBER(ff_r);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_READ8_MEMBER(tape_r);
	DECLARE_WRITE8_MEMBER(tape_w);
	DECLARE_READ8_MEMBER(tape_stop_r);
	DECLARE_READ8_MEMBER(tape_start_r);
	DECLARE_WRITE8_MEMBER(xor_display_w);
	bool m_tape_switch;
	required_shared_ptr<UINT8> m_p_wram;
	UINT8 *m_p_chargen;
	UINT8 m_xor_display;
	UINT8 m_key_mux;
	DECLARE_DRIVER_INIT(bmjr);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_bmjr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



void bmjr_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 bmjr_state::screen_update_bmjr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx,fg=4;
	UINT16 sy=0,ma=0x100,x;
	UINT8 inv = (m_xor_display) ? 0xff : 0;

	for(y = 0; y < 24; y++ )
	{
		for (ra = 0; ra < 8; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 32; x++)
			{
				chr = m_p_wram[x];
				gfx = m_p_chargen[(chr<<3) | ra] ^ inv;

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? fg : 0;
				*p++ = BIT(gfx, 6) ? fg : 0;
				*p++ = BIT(gfx, 5) ? fg : 0;
				*p++ = BIT(gfx, 4) ? fg : 0;
				*p++ = BIT(gfx, 3) ? fg : 0;
				*p++ = BIT(gfx, 2) ? fg : 0;
				*p++ = BIT(gfx, 1) ? fg : 0;
				*p++ = BIT(gfx, 0) ? fg : 0;
			}
		}
		ma+=32;
	}

	return 0;
}

READ8_MEMBER( bmjr_state::key_r )
{
	char kbdrow[6];
	sprintf(kbdrow,"KEY%X", m_key_mux);
	return (ioport(kbdrow)->read() & 15) | (ioport("KEYMOD")->read() << 4);
}

WRITE8_MEMBER( bmjr_state::key_w )
{
	m_key_mux = data & 0xf;

//  if(data & 0xf0)
//      printf("%02x",data & 0xf0);
}


READ8_MEMBER( bmjr_state::ff_r )
{
	return 0xff;
}

READ8_MEMBER( bmjr_state::unk_r )
{
	return 0x30;
}

READ8_MEMBER( bmjr_state::tape_r )
{
	//m_cass->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);

	return ((m_cass->input()) > 0.03) ? 0xff : 0x00;
}

WRITE8_MEMBER( bmjr_state::tape_w )
{
	if(!m_tape_switch)
	{
		m_beep->set_state(!BIT(data, 7));
	}
	else
	{
		//m_cass->change_state(CASSETTE_RECORD,CASSETTE_MASK_UISTATE);
		m_cass->output(BIT(data, 0) ? -1.0 : +1.0);
	}
}

READ8_MEMBER( bmjr_state::tape_stop_r )
{
	m_tape_switch = 0;
	//m_cass->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
	m_cass->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
	return 0x01;
}

READ8_MEMBER( bmjr_state::tape_start_r )
{
	m_tape_switch = 1;
	m_cass->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
	return 0x01;
}

WRITE8_MEMBER( bmjr_state::xor_display_w )
{
	m_xor_display = data;
}

static ADDRESS_MAP_START(bmjr_mem, AS_PROGRAM, 8, bmjr_state)
	ADDRESS_MAP_UNMAP_HIGH
	//0x0100, 0x03ff basic vram
	//0x0900, 0x20ff vram, modes 0x40 / 0xc0
	//0x2100, 0x38ff vram, modes 0x44 / 0xcc
	AM_RANGE(0x0000, 0xafff) AM_RAM AM_SHARE("p_wram")
	AM_RANGE(0xb000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_ROM
//  AM_RANGE(0xe890, 0xe890) W MP-1710 tile color
//  AM_RANGE(0xe891, 0xe891) W MP-1710 background color
//  AM_RANGE(0xe892, 0xe892) W MP-1710 monochrome / color setting
	AM_RANGE(0xee00, 0xee00) AM_READ(tape_stop_r) //R stop tape
	AM_RANGE(0xee20, 0xee20) AM_READ(tape_start_r) //R start tape
	AM_RANGE(0xee40, 0xee40) AM_WRITE(xor_display_w) //W Picture reverse
	AM_RANGE(0xee80, 0xee80) AM_READWRITE(tape_r,tape_w)//RW tape input / output
	AM_RANGE(0xeec0, 0xeec0) AM_READWRITE(key_r,key_w)//RW keyboard
	AM_RANGE(0xef00, 0xef00) AM_READ(ff_r) //R timer
	AM_RANGE(0xef40, 0xef40) AM_READ(ff_r) //R unknown
	AM_RANGE(0xef80, 0xef80) AM_READ(unk_r) //R unknown
//  AM_RANGE(0xefe0, 0xefe0) W screen mode
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( bmjr )
	PORT_START("KEY0")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("7 \'") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEY9")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEYA")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_UNUSED )
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("@ Up") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('@')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEYB")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("[ Down") PORT_CODE(KEYCODE_OPENBRACE) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('[')
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("^ Right") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('^')
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEYC")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("\xC2\xA5 / Left") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("KEYD")
	PORT_DIPNAME( 0x01, 0x01, "D" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEYE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEYF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEYMOD") /* Note: you should press Normal to return from a Kana state and vice-versa */
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME(DEF_STR( Normal )) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Kana Shift") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("Kana") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0xf0,IP_ACTIVE_LOW,IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout bmjr_charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( bmjr )
	GFXDECODE_ENTRY( "chargen", 0x0000, bmjr_charlayout, 0, 4 )
GFXDECODE_END

void bmjr_state::machine_start()
{
	m_beep->set_frequency(1200); //guesswork
	m_beep->set_state(0);
}

void bmjr_state::machine_reset()
{
	m_tape_switch = 0;
	m_cass->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
}

static MACHINE_CONFIG_START( bmjr, bmjr_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6800, XTAL_4MHz/4) //unknown clock / divider
	MCFG_CPU_PROGRAM_MAP(bmjr_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bmjr_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256, 192)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 192-1)
	MCFG_SCREEN_UPDATE_DRIVER(bmjr_state, screen_update_bmjr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_3BIT_BRG("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bmjr)

	/* Audio */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Devices */
	MCFG_CASSETTE_ADD( "cassette" )
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( bmjr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bas.rom", 0xb000, 0x3000, BAD_DUMP CRC(2318e04e) SHA1(cdb3535663090f5bcaba20b1dbf1f34724ef6a5f)) //12k ROMs doesn't exist ...
	ROM_LOAD( "mon.rom", 0xf000, 0x1000, CRC(776cfa3a) SHA1(be747bc40fdca66b040e0f792b05fcd43a1565ce))
	ROM_LOAD( "prt.rom", 0xe000, 0x0800, CRC(b9aea867) SHA1(b8dd5348790d76961b6bdef41cfea371fdbcd93d))

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x0000, 0x0800, BAD_DUMP CRC(258c6fd7) SHA1(d7c7dd57d6fc3b3d44f14c32182717a48e24587f)) //taken from a JP emulator
ROM_END

/* Driver */
DRIVER_INIT_MEMBER(bmjr_state,bmjr)
{
}

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY        FULLNAME       FLAGS */
COMP( 1982, bmjr,   0,      0,       bmjr,      bmjr, bmjr_state,    bmjr,  "Hitachi", "Basic Master Jr", MACHINE_NOT_WORKING)
