// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        Z1013 driver by Miodrag Milanovic

        22/04/2009 Preliminary driver.

The 8x4 keyboard contains letters A-W, enter, cursor right/left, space, and
4 shift keys.
S1 - X-Z, numbers
S2 - a-w
S3 - x-z, punctuation
S4 - control keys

Control Keys:
G - Graphics
A - Alpha
T - clear screen
U - enter
P - cursor left
Q - cursor right

Monitor commands (debug)
R - registers
B - breakpoint
E - execute
G - resume after breakpoint
N - single-step

Monitor commands (general)
H - switch to hex keyboard (H to Q become 0 to 9)
A - switch back to normal alpha keyboard
C - Compare memory blocks
D - Dump memory
F - Find bytes in memory
T - Copy memory block
M - Modify bytes (; to exit)
I - reboot
J - Jump to address
K - Fill memory (K by itself fills all of memory)
L - Load Cassette
S - Save Cassette
W - window (example: W EF00 EFFF)

Due to no input checking, misuse of commands can crash the system.



****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"


class z1013_state : public driver_device
{
public:
	z1013_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_cass(*this, "cassette"),
	m_p_videoram(*this, "videoram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	DECLARE_WRITE8_MEMBER(z1013_keyboard_w);
	DECLARE_READ8_MEMBER(port_b_r);
	DECLARE_WRITE8_MEMBER(port_b_w);
	DECLARE_READ8_MEMBER(k7659_port_b_r);
	required_shared_ptr<UINT8> m_p_videoram;
	const UINT8 *m_p_chargen;
	UINT8 m_keyboard_line;
	bool m_keyboard_part;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_z1013(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_SNAPSHOT_LOAD_MEMBER( z1013 );
};


/* Address maps */
static ADDRESS_MAP_START(z1013_mem, AS_PROGRAM, 8, z1013_state)
	AM_RANGE( 0x0000, 0xebff ) AM_RAM
	AM_RANGE( 0xec00, 0xefff ) AM_RAM AM_SHARE("videoram")
	AM_RANGE( 0xf000, 0xffff ) AM_ROM //  ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(z1013_io, AS_IO, 8, z1013_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x03 ) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
	AM_RANGE( 0x08, 0x08 ) AM_WRITE(z1013_keyboard_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( z1013_8x4 )
	PORT_START("X0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S1") PORT_CODE(KEYCODE_LSHIFT)
	PORT_START("X1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S2") PORT_CODE(KEYCODE_LCONTROL)
	PORT_START("X2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S3") PORT_CODE(KEYCODE_RSHIFT)
	PORT_START("X3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S4") PORT_CODE(KEYCODE_RCONTROL)
	PORT_START("X4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_START("X5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_START("X6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_START("X7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ent") PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END

static INPUT_PORTS_START( z1013_8x8 )
	PORT_START("X0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_START("X1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_START("X2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_START("X3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_START("X4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
	PORT_START("X5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_MINUS)
	PORT_START("X6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ent") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_START("X7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK)
INPUT_PORTS_END

static INPUT_PORTS_START( z1013 )
INPUT_PORTS_END


void z1013_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 z1013_state::screen_update_z1013(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 32; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma+32; x++)
			{
				chr = m_p_videoram[x];

				/* get pattern of pixels for that character scanline */
				gfx = m_p_chargen[(chr<<3) | ra];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=32;
	}
	return 0;
}

void z1013_state::machine_reset()
{
	m_maincpu->set_state_int(Z80_PC, 0xF000);
	m_keyboard_part = 0;
	m_keyboard_line = 0;
}

WRITE8_MEMBER( z1013_state::z1013_keyboard_w )
{
	m_keyboard_line = data;
}

READ8_MEMBER( z1013_state::port_b_r )
{
	char kbdrow[6];
	sprintf(kbdrow,"X%d", m_keyboard_line & 7);
	UINT8 data = ioport(kbdrow)->read();

	if (m_keyboard_part)
		data >>= 4;

	data &= 0x0f;

	if (m_cass->input() > 0.03)
		data |= 0x40;

	return data;
}

WRITE8_MEMBER( z1013_state::port_b_w )
{
	m_keyboard_part = BIT(data, 4); // for z1013a2 only
	m_cass->output(BIT(data, 7) ? -1.0 : +1.0);
}

READ8_MEMBER( z1013_state::k7659_port_b_r )
{
	return 0xff;
}

SNAPSHOT_LOAD_MEMBER( z1013_state, z1013 )
{
/* header layout
0000,0001 - load address
0002,0003 - end address
0004,0005 - exec address
0006-000B - unknown
000C      - Filetype (appears B=Basic, C=Machine Language, I=?, could be more)
000D-000F - bytes D3, D3, D3
0010-001F - Filename
0020 up   - Program to load
*/

	UINT8* data= auto_alloc_array(machine(), UINT8, snapshot_size);
	UINT16 startaddr,endaddr,runaddr;

	image.fread( data, snapshot_size);

	startaddr = data[0] + data[1]*256;
	endaddr   = data[2] + data[3]*256;
	runaddr   = data[4] + data[5]*256;

	if ((data[13]==data[14]) && (data[14]==data[15]))
	{ }
	else
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Not a Z1013 image");
		image.message(" Not a Z1013 image");
		return IMAGE_INIT_FAIL;
	}

	memcpy (m_maincpu->space(AS_PROGRAM).get_read_ptr(startaddr),
			data+0x20, endaddr - startaddr + 1);

	if (runaddr)
		m_maincpu->set_state_int(Z80_PC, runaddr);
	else
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Loaded but cannot run");
		image.message(" Loaded but cannot run");
	}

	return IMAGE_INIT_PASS;
}

/* F4 Character Displayer */
static const gfx_layout z1013_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	512,                    /* 2 x 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( z1013 )
	GFXDECODE_ENTRY( "chargen", 0x0000, z1013_charlayout, 0, 1 )
GFXDECODE_END

/* Machine driver */
static MACHINE_CONFIG_START( z1013, z1013_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_1MHz )
	MCFG_CPU_PROGRAM_MAP(z1013_mem)
	MCFG_CPU_IO_MAP(z1013_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(z1013_state, screen_update_z1013)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", z1013)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_1MHz)
	MCFG_Z80PIO_IN_PB_CB(READ8(z1013_state, port_b_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(z1013_state, port_b_w))

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_SNAPSHOT_ADD("snapshot", z1013_state, z1013, "z80", 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( z1013k76, z1013 )
	MCFG_DEVICE_REMOVE("z80pio")
	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_1MHz)
	MCFG_Z80PIO_IN_PB_CB(READ8(z1013_state, k7659_port_b_r))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( z1013 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "202", "Original" )
	ROMX_LOAD( "mon_202.bin", 0xf000, 0x0800, CRC(5884edab) SHA1(c3a45ea5cc4da2b7c270068ba1e2d75916960709), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 1, "jm", "Jens Muller version" )
	ROMX_LOAD( "mon_jm_1992.bin", 0xf000, 0x0800, CRC(186d2888) SHA1(b52ccb557c41c96bace7db4c4f5031a0cd736168), ROM_BIOS(2))

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD ("z1013font.bin",   0x0000, 0x0800, CRC(7023088f) SHA1(8b197a51c070efeba173d10be197bd41e764358c))
	ROM_LOAD ("altfont.bin",     0x0800, 0x0800, CRC(2dc96f9c) SHA1(d0b9b0751cc1e91be731547f6442c649b6dd6979))
ROM_END

ROM_START( z1013a2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mon_a2.bin", 0xf000, 0x0800, CRC(98b19b10) SHA1(97e158f589198cb96aae1567ee0aa6e47824027e))

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD ("z1013font.bin",   0x0000, 0x0800, CRC(7023088f) SHA1(8b197a51c070efeba173d10be197bd41e764358c))
	ROM_LOAD ("altfont.bin",     0x0800, 0x0800, CRC(2dc96f9c) SHA1(d0b9b0751cc1e91be731547f6442c649b6dd6979))
ROM_END

ROM_START( z1013k76 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mon_rb_k7659.bin", 0xf000, 0x1000, CRC(b3d88c45) SHA1(0bcd20338cf0706b384f40901b7f8498c6f6c320))

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD ("z1013font.bin",   0x0000, 0x0800, CRC(7023088f) SHA1(8b197a51c070efeba173d10be197bd41e764358c))
	ROM_LOAD ("altfont.bin",     0x0800, 0x0800, CRC(2dc96f9c) SHA1(d0b9b0751cc1e91be731547f6442c649b6dd6979))

	ROM_REGION(0x1000, "k7659",0)
	ROM_LOAD ("k7659n.bin", 0x0000, 0x0800, CRC(7454bf0a) SHA1(b97e7df93778fa371b96b6f4fb1a5b1c8b89d7ba) )
ROM_END

ROM_START( z1013s60 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD( "mon_rb_s6009.bin", 0xf000, 0x1000, CRC(b37faeed) SHA1(ce2e69af5378d39284e8b3be23da50416a0b0fbe), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD( "4k-moni-k7652.bin", 0xf000, 0x1000, CRC(a1625fce) SHA1(f0847399502b38a73ad26b38ee2d85ba04ab85ec), ROM_BIOS(2))

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD ("z1013font.bin",   0x0000, 0x0800, CRC(7023088f) SHA1(8b197a51c070efeba173d10be197bd41e764358c))
	ROM_LOAD ("altfont.bin",     0x0800, 0x0800, CRC(2dc96f9c) SHA1(d0b9b0751cc1e91be731547f6442c649b6dd6979))
ROM_END

ROM_START( z1013k69 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "4k-moni-k7669.bin", 0xf000, 0x1000, CRC(09cd2a7a) SHA1(0b8500320d464469868a6b48db31105f34710c41))

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD ("z1013font.bin",   0x0000, 0x0800, CRC(7023088f) SHA1(8b197a51c070efeba173d10be197bd41e764358c))
	ROM_LOAD ("altfont.bin",     0x0800, 0x0800, CRC(2dc96f9c) SHA1(d0b9b0751cc1e91be731547f6442c649b6dd6979))
ROM_END
/* Driver */

/*    YEAR  NAME       PARENT  COMPAT  MACHINE      INPUT       INIT             COMPANY                       FULLNAME        FLAGS */
COMP( 1985, z1013,     0,      0,      z1013,       z1013_8x4, driver_device,  0, "VEB Robotron Electronics Riesa", "Z1013 (matrix 8x4)", 0 )
COMP( 1985, z1013a2,   z1013,  0,      z1013,       z1013_8x8, driver_device,  0, "VEB Robotron Electronics Riesa", "Z1013 (matrix 8x8)", 0 )
COMP( 1985, z1013k76,  z1013,  0,      z1013k76,    z1013, driver_device,      0, "VEB Robotron Electronics Riesa", "Z1013 (K7659)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1985, z1013s60,  z1013,  0,      z1013k76,    z1013_8x8, driver_device,  0, "VEB Robotron Electronics Riesa", "Z1013 (K7652/S6009)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1985, z1013k69,  z1013,  0,      z1013k76,    z1013, driver_device,      0, "VEB Robotron Electronics Riesa", "Z1013 (K7669)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
