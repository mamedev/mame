// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
  Experimental ti99/2 driver

TODO :
  * find a TI99/2 ROM dump (some TI99/2 ARE in private hands)
  * test the driver !
  * understand the "viden" pin
  * implement cassette
  * implement Hex-Bus

  Raphael Nabet (who really has too much time to waste), december 1999, 2000
*/

/*
  TI99/2 facts :

References :
* TI99/2 main logic board schematics, 83/03/28-30 (on ftp.whtech.com, or just ask me)
  (Thanks to Charles Good for posting this)

general :
* prototypes in 1983
* uses a 10.7MHz TMS9995 CPU, with the following features :
  - 8-bit data bus
  - 256 bytes 16-bit RAM (0xff00-0xff0b & 0xfffc-0xffff)
  - only available int lines are INT4 (used by vdp), INT1*, and NMI* (both free for extension)
  - on-chip decrementer (0xfffa-0xfffb)
  - Unlike tms9900, CRU address range is full 0x0000-0xFFFE (A0 is not used as address).
    This is possible because tms9995 uses d0-d2 instead of the address MSBits to support external
    opcodes.
  - quite more efficient than tms9900, and a few additionnal instructions and features
* 24 or 32kb ROM (16kb plain (1kb of which used by vdp), 16kb split into 2 8kb pages)
* 4kb 8-bit RAM, 256 bytes 16-bit RAM
* custom vdp shares CPU RAM/ROM.  The display is quite alike to tms9928 graphics mode, except
  that colors are a static B&W, and no sprite is displayed. The config (particularily the
  table base addresses) cannot be changed.  Since TI located the pattern generator table in
  ROM, we cannot even redefine the char patterns (unless you insert a custom cartidge which
  overrides the system ROMs).  VBL int triggers int4 on tms9995.
* CRU is handled by one single custom chip, so the schematics don't show many details :-( .
* I/O :
  - 48-key keyboard.  Same as TI99/4a, without alpha lock, and with an additional break key.
    Note that the hardware can make the difference between the two shift keys.
  - cassette I/O (one unit)
  - ALC bus (must be another name for Hex-Bus)
* 60-pin expansion/cartidge port on the back

memory map :
* 0x0000-0x4000 : system ROM (0x1C00-0x2000 (?) : char ROM used by vdp)
* 0x4000-0x6000 : system ROM, mapped to either of two distinct 8kb pages according to the S0
  bit from the keyboard interface (!), which is used to select the first key row.
  [only on second-generation TI99/2 protos, first generation protos only had 24kb of ROM]
* 0x6000-0xE000 : free for expansion
* 0xE000-0xF000 : 8-bit "system" RAM (0xEC00-0xEF00 used by vdp)
* 0xF000-0xF0FB : 16-bit processor RAM (on tms9995)
* 0xF0FC-0xFFF9 : free for expansion
* 0xFFFA-0xFFFB : tms9995 internal decrementer
* 0xFFFC-0xFFFF : 16-bit processor RAM (provides the NMI vector)

CRU map :
* 0x0000-0x1EFC : reserved
* 0x1EE0-0x1EFE : tms9995 flag register
* 0x1F00-0x1FD8 : reserved
* 0x1FDA : tms9995 MID flag
* 0x1FDC-0x1FFF : reserved
* 0x2000-0xE000 : unaffected
* 0xE400-0xE40E : keyboard I/O (8 bits input, either 3 or 6 bit output)
* 0xE80C : cassette I/O
* 0xE80A : ALC BAV
* 0xE808 : ALC HSK
* 0xE800-0xE808 : ALC data (I/O)
* 0xE80E : video enable (probably input - seems to come from the VDP, and is present on the
  expansion connector)
* 0xF000-0xFFFE : reserved
Note that only A15-A11 and A3-A1 (A15 = MSB, A0 = LSB) are decoded in the console, so the keyboard
is actually mapped to 0xE000-0xE7FE, and other I/O bits to 0xE800-0xEFFE.
Also, ti99/2 does not support external instructions better than ti99/4(a).  This is crazy, it
would just have taken three extra tracks on the main board and a OR gate in an ASIC.
*/

#include "emu.h"
#include "machine/tms9901.h"
#include "cpu/tms9900/tms9995.h"

class ti99_2_state : public driver_device
{
public:
	ti99_2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_videoram;
	int m_ROM_paged;
	int m_irq_state;
	int m_KeyRow;
	DECLARE_WRITE8_MEMBER(ti99_2_write_kbd);
	DECLARE_WRITE8_MEMBER(ti99_2_write_misc_cru);
	DECLARE_READ8_MEMBER(ti99_2_read_kbd);
	DECLARE_READ8_MEMBER(ti99_2_read_misc_cru);
	DECLARE_DRIVER_INIT(ti99_2_24);
	DECLARE_DRIVER_INIT(ti99_2_32);
	virtual void machine_reset() override;
	UINT32 screen_update_ti99_2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(ti99_2_vblank_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};



DRIVER_INIT_MEMBER(ti99_2_state,ti99_2_24)
{
	/* no ROM paging */
	m_ROM_paged = 0;
}

DRIVER_INIT_MEMBER(ti99_2_state,ti99_2_32)
{
	/* ROM paging enabled */
	m_ROM_paged = 1;
}

#define TI99_2_32_ROMPAGE0 (memregion("maincpu")->base()+0x4000)
#define TI99_2_32_ROMPAGE1 (memregion("maincpu")->base()+0x10000)

void ti99_2_state::machine_reset()
{
	m_irq_state = ASSERT_LINE;
	if (! m_ROM_paged)
		membank("bank1")->set_base(memregion("maincpu")->base()+0x4000);
	else
		membank("bank1")->set_base((memregion("maincpu")->base()+0x4000));

	// Configure CPU to insert 1 wait state for each external memory access
	// by lowering the READY line on reset
	// TODO: Check with specs
	static_cast<tms9995_device*>(machine().device("maincpu"))->set_ready(CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(ti99_2_state::ti99_2_vblank_interrupt)
{
	m_maincpu->set_input_line(INT_9995_INT1, m_irq_state);
	m_irq_state = (m_irq_state == ASSERT_LINE) ? CLEAR_LINE : ASSERT_LINE;
}


/*
  TI99/2 vdp emulation.

  Things could not be simpler.
  We display 24 rows and 32 columns of characters.  Each 8*8 pixel character pattern is defined
  in a 128-entry table located in ROM.  Character code for each screen position are stored
  sequentially in RAM.  Colors are a fixed Black on White.

    There is an EOL character that blanks the end of the current line, so that
    the CPU can get more bus time.
*/



UINT32 ti99_2_state::screen_update_ti99_2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	int i, sx, sy;


	sx = sy = 0;

	for (i = 0; i < 768; i++)
	{
		/* Is the char code masked or not ??? */
		m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, videoram[i] & 0x7F, 0,
			0, 0, sx, sy);

		sx += 8;
		if (sx == 256)
		{
			sx = 0;
			sy += 8;
		}
	}

	return 0;
}

static const gfx_layout ti99_2_charlayout =
{
	8,8,        /* 8 x 8 characters */
	128,        /* 128 characters */
	1,          /* 1 bits per pixel */
	{ 0 },      /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,1,2,3,4,5,6,7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, },
	8*8         /* every char takes 8 bytes */
};

static GFXDECODE_START( ti99_2 )
	GFXDECODE_ENTRY( "maincpu", 0x1c00,  ti99_2_charlayout, 0, 1 )
GFXDECODE_END


/*
  Memory map - see description above
*/

static ADDRESS_MAP_START( ti99_2_memmap, AS_PROGRAM, 8, ti99_2_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM         /* system ROM */
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank1")    /* system ROM, banked on 32kb ROMs protos */
	AM_RANGE(0x6000, 0xdfff) AM_NOP         /* free for expansion */
	AM_RANGE(0xe000, 0xebff) AM_RAM         /* system RAM */
	AM_RANGE(0xec00, 0xeeff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xef00, 0xefff) AM_RAM         /* system RAM */
	AM_RANGE(0xf000, 0xffff) AM_NOP         /* free for expansion (and internal processor RAM) */
ADDRESS_MAP_END


/*
  CRU map - see description above
*/

/* current keyboard row */

/* write the current keyboard row */
WRITE8_MEMBER(ti99_2_state::ti99_2_write_kbd)
{
	offset &= 0x7;  /* other address lines are not decoded */

	if (offset <= 2)
	{
		/* this implementation is just a guess */
		if (data)
			m_KeyRow |= 1 << offset;
		else
			m_KeyRow &= ~ (1 << offset);
	}
	/* now, we handle ROM paging */
	if (m_ROM_paged)
	{   /* if we have paged ROMs, page according to S0 keyboard interface line */
		membank("bank1")->set_base((m_KeyRow == 0) ? TI99_2_32_ROMPAGE1 : TI99_2_32_ROMPAGE0);
	}
}

WRITE8_MEMBER(ti99_2_state::ti99_2_write_misc_cru)
{
	offset &= 0x7;  /* other address lines are not decoded */

	switch (offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		/* ALC I/O */
		break;
	case 4:
		/* ALC HSK */
		break;
	case 5:
		/* ALC BAV */
		break;
	case 6:
		/* cassette output */
		break;
	case 7:
		/* video enable */
		break;
	}
}

/* read keys in the current row */
READ8_MEMBER(ti99_2_state::ti99_2_read_kbd)
{
	static const char *const keynames[] = { "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7" };

	return ioport(keynames[m_KeyRow])->read();
}

READ8_MEMBER(ti99_2_state::ti99_2_read_misc_cru)
{
	return 0;
}

static ADDRESS_MAP_START(ti99_2_io, AS_IO, 8, ti99_2_state )
	AM_RANGE(0x0E00, 0x0E7f) AM_READ(ti99_2_read_kbd)
	AM_RANGE(0x0E80, 0x0Eff) AM_READ(ti99_2_read_misc_cru)
	AM_RANGE(0x7000, 0x73ff) AM_WRITE(ti99_2_write_kbd)
	AM_RANGE(0x7400, 0x77ff) AM_WRITE(ti99_2_write_misc_cru)
ADDRESS_MAP_END


/* ti99/2 : 54-key keyboard */
static INPUT_PORTS_START(ti99_2)

	PORT_START("LINE0")    /* col 0 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 ! DEL") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 @ INS") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $ CLEAR") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 % BEGIN") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^ PROC'D") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 & AID") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 * REDO") PORT_CODE(KEYCODE_8)

	PORT_START("LINE1")    /* col 1 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w W ~") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e E (UP)") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r R [") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t T ]") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("y Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("i I ?") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 ( BACK") PORT_CODE(KEYCODE_9)

	PORT_START("LINE2")    /* col 2 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s S (LEFT)") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d D (RIGHT)") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f F {") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("h H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("u U _") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o O '") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0)

	PORT_START("LINE3")    /* col 3 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z Z \\") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x X (DOWN)") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c C `") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g G }") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("p P \"") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= + QUIT") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("LINE4")    /* col 4 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT/*KEYCODE_CAPSLOCK*/)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("n N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("l L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ -") PORT_CODE(KEYCODE_SLASH)

	PORT_START("LINE5")    /* col 5 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(SPACE)") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("m M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FCTN") PORT_CODE(KEYCODE_LALT)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)

	PORT_START("LINE6")    /* col 6 */
		PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE7")    /* col 7 */
		PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END

static MACHINE_CONFIG_START( ti99_2, ti99_2_state )
	// basic machine hardware
	// TMS9995, standard variant
	// We have no lines connected yet
	MCFG_TMS99xx_ADD("maincpu", TMS9995, 10700000, ti99_2_memmap, ti99_2_io)

	MCFG_CPU_VBLANK_INT_DRIVER("screen", ti99_2_state,  ti99_2_vblank_interrupt)

	/* video hardware */
	/*MCFG_TMS9928A( &tms9918_interface )*/
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256, 192)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 192-1)
	MCFG_SCREEN_UPDATE_DRIVER(ti99_2_state, screen_update_ti99_2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ti99_2)
	MCFG_PALETTE_ADD_WHITE_AND_BLACK("palette")
MACHINE_CONFIG_END



/*
  ROM loading
*/
ROM_START(ti99_224)
	/*CPU memory space*/
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("992rom.bin", 0x0000, 0x6000, NO_DUMP)      /* system ROMs */
ROM_END

ROM_START(ti99_232)
	/*64kb CPU memory space + 8kb to read the extra ROM page*/
	ROM_REGION(0x12000,"maincpu",0)
	ROM_LOAD("992rom32.bin", 0x0000, 0x6000, NO_DUMP)    /* system ROM - 32kb */
	ROM_CONTINUE(0x10000,0x2000)
ROM_END

/* one expansion/cartridge port on the back */
/* one cassette unit port */
/* Hex-bus disk controller: supports up to 4 floppy disk drives */
/* None of these is supported (tape should be easy to emulate) */

/*      YEAR    NAME        PARENT      COMPAT  MACHINE     INPUT   INIT        COMPANY                 FULLNAME */
COMP(   1983,   ti99_224,   0,          0,  ti99_2, ti99_2, ti99_2_state,   ti99_2_24,          "Texas Instruments",    "TI-99/2 BASIC Computer (24kb ROMs)" , MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP(   1983,   ti99_232,   ti99_224,   0,  ti99_2, ti99_2, ti99_2_state,   ti99_2_32,          "Texas Instruments",    "TI-99/2 BASIC Computer (32kb ROMs)" , MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
