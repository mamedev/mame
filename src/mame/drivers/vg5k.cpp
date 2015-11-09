// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Philips VG-5000mu

    Driver by Sandro Ronco with help from Daniel Coulom

    05/2010 (Sandro Ronco)
     - EF9345 video controller
     - keyboard input ports
    05/2009 Skeleton driver.

    Known issues:
    - 1200 bauds cassette don't works
    - BASIC games hangs in default BIOS but works in alternative version

    Informations ( see the very informative http://vg5k.free.fr/ ):
     - Variants: Radiola VG5000 and Schneider VG5000
     - CPU: Zilog Z80 running at 4MHz
     - ROM: 18KB (16 KB BASIC + 2 KB charset )
     - RAM: 24 KB
     - Video: SGS Thomson EF9345 processor
            - Text mode: 25 rows x 40 columns
            - Character matrix: 8 x 10
            - ASCII characters set, 128 graphics mode characters, 192 user characters.
            - Graphics mode: not available within basic, only semi graphic is available.
            - Colors: 8
     - Sound: Synthesizer, 4 Octaves
     - Keyboard: 63 keys AZERTY, Caps Lock, CTRL key to access 33 BASIC instructions
     - I/O: Tape recorder connector (1200/2400 bauds), Scart connector to TV (RGB),
       External PSU (VU0022) connector, Bus connector (2x25 pins)
     - There are 2 versions of the VG5000 ROM, one with Basic v1.0,
       contained in two 8 KB ROMs, and one with Basic 1.1, contained in
       a single 16 KB ROM.
     - RAM: 24 KB (3 x 8 KB) type SRAM D4168C, more precisely:
         2 x 8 KB, used by the system
         1 x 8 KB, used by the video processor
     - Memory Map:
         $0000 - $3fff  BASIC + monitor
         $4000 - $47cf  Screen
         $47d0 - $7fff  reserved area for BASIC, variables, etc
         $8000 - $bfff  Memory Expansion 16K or ROM cart
         $c000 - $ffff  Memory Expansion 32K or ROM cart
     - This computer was NOT MSX-compatible!


****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "imagedev/printer.h"
#include "video/ef9345.h"
#include "sound/dac.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "formats/vg5k_cas.h"
#include "softlist.h"

class vg5k_state : public driver_device
{
public:
	vg5k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ef9345(*this, "ef9345"),
			m_dac(*this, "dac"),
			m_printer(*this, "printer"),
			m_cassette(*this, "cassette"),
			m_ram(*this, RAM_TAG)
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<ef9345_device> m_ef9345;
	required_device<dac_device> m_dac;
	required_device<printer_image_device> m_printer;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;

	offs_t m_ef9345_offset;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ8_MEMBER( printer_r );
	DECLARE_WRITE8_MEMBER( printer_w );
	DECLARE_WRITE8_MEMBER ( ef9345_offset_w );
	DECLARE_READ8_MEMBER ( ef9345_io_r );
	DECLARE_WRITE8_MEMBER ( ef9345_io_w );
	DECLARE_READ8_MEMBER ( cassette_r );
	DECLARE_WRITE8_MEMBER ( cassette_w );
	DECLARE_DRIVER_INIT(vg5k);
	TIMER_CALLBACK_MEMBER(z80_irq_clear);
	TIMER_DEVICE_CALLBACK_MEMBER(z80_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(vg5k_scanline);
};


READ8_MEMBER( vg5k_state::printer_r )
{
	return (m_printer->is_ready() ? 0x00 : 0xff);
}


WRITE8_MEMBER( vg5k_state::printer_w )
{
	m_printer->output(data);
}


WRITE8_MEMBER ( vg5k_state::ef9345_offset_w )
{
	m_ef9345_offset = data;
}


READ8_MEMBER ( vg5k_state::ef9345_io_r )
{
	return m_ef9345->data_r(space, m_ef9345_offset, 0xff);
}


WRITE8_MEMBER ( vg5k_state::ef9345_io_w )
{
	m_ef9345->data_w(space, m_ef9345_offset, data, 0xff);
}


READ8_MEMBER ( vg5k_state::cassette_r )
{
	double level = m_cassette->input();

	return (level > 0.03) ? 0xff : 0x00;
}


WRITE8_MEMBER ( vg5k_state::cassette_w )
{
	m_dac->write_unsigned8(data <<2);

	if (data == 0x03)
		m_cassette->output(+1);
	else if (data == 0x02)
		m_cassette->output(-1);
	else
		m_cassette->output(0);
}


static ADDRESS_MAP_START( vg5k_mem, AS_PROGRAM, 8, vg5k_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x3fff ) AM_ROM
	AM_RANGE( 0x4000, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0xffff ) AM_NOP /* messram expansion memory */
ADDRESS_MAP_END

static ADDRESS_MAP_START( vg5k_io , AS_IO, 8, vg5k_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK (0xff)

	/* joystick */
	AM_RANGE( 0x07, 0x07 ) AM_READ_PORT("JOY0")
	AM_RANGE( 0x08, 0x08 ) AM_READ_PORT("JOY1")

	/* printer */
	AM_RANGE( 0x10, 0x10 ) AM_READ(printer_r)
	AM_RANGE( 0x11, 0x11 ) AM_WRITE(printer_w)

	/* keyboard */
	AM_RANGE( 0x80, 0x80 ) AM_READ_PORT("ROW1")
	AM_RANGE( 0x81, 0x81 ) AM_READ_PORT("ROW2")
	AM_RANGE( 0x82, 0x82 ) AM_READ_PORT("ROW3")
	AM_RANGE( 0x83, 0x83 ) AM_READ_PORT("ROW4")
	AM_RANGE( 0x84, 0x84 ) AM_READ_PORT("ROW5")
	AM_RANGE( 0x85, 0x85 ) AM_READ_PORT("ROW6")
	AM_RANGE( 0x86, 0x86 ) AM_READ_PORT("ROW7")
	AM_RANGE( 0x87, 0x87 ) AM_READ_PORT("ROW8")

	/* EF9345 */
	AM_RANGE( 0x8f, 0x8f ) AM_WRITE(ef9345_offset_w)
	AM_RANGE( 0xcf, 0xcf ) AM_READWRITE(ef9345_io_r, ef9345_io_w)

	/* cassette */
	AM_RANGE( 0xaf,0xaf ) AM_READWRITE(cassette_r, cassette_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vg5k )
	PORT_START("ROW1")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("HOME")
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_UNUSED
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("SHIFT")
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("LEFT")
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("RIGHT")
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("DOWN")
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL)  PORT_NAME("CTRL")
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT) PORT_NAME("INS")
	PORT_START("ROW2")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("RUN")
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("CAPSLOCK")
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_UNUSED
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("ENTER")
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_NAME("UP")
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_START("ROW3")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR(':')
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_START("ROW4")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_START("ROW5")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_START("ROW6")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_UNUSED
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("..")
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_START("ROW7")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP) PORT_CHAR('<')
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("PRINT")
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_START("ROW8")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("<--")
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_START("JOY0")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1)
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_UNUSED
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_UNUSED
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_UNUSED
	PORT_START("JOY1")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2)
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_UNUSED
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_UNUSED
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_UNUSED
INPUT_PORTS_END


TIMER_CALLBACK_MEMBER(vg5k_state::z80_irq_clear)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(vg5k_state::z80_irq)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);

	machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(vg5k_state::z80_irq_clear),this));
}

TIMER_DEVICE_CALLBACK_MEMBER(vg5k_state::vg5k_scanline)
{
	m_ef9345->update_scanline((UINT16)param);
}


void vg5k_state::machine_start()
{
	save_item(NAME(m_ef9345_offset));
}

void vg5k_state::machine_reset()
{
	m_ef9345_offset = 0;
}

/* F4 Character Displayer */
static const gfx_layout vg5k_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( vg5k )
	GFXDECODE_ENTRY( "ef9345", 0x2000, vg5k_charlayout, 0, 4 )
GFXDECODE_END

DRIVER_INIT_MEMBER(vg5k_state,vg5k)
{
	UINT8 *FNT = memregion("ef9345")->base();
	UINT16 a,b,c,d,dest=0x2000;

	/* Unscramble the chargen rom as the format is too complex for gfxdecode to handle unaided */
	for (a = 0; a < 8192; a+=4096)
		for (b = 0; b < 2048; b+=64)
			for (c = 0; c < 4; c++)
				for (d = 0; d < 64; d+=4)
					FNT[dest++]=FNT[a|b|c|d];


	/* install expansion memory*/
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();
	UINT16 ram_size = m_ram->size();

	if (ram_size > 0x4000)
		program.install_ram(0x8000, 0x3fff + ram_size, ram);
}


static MACHINE_CONFIG_START( vg5k, vg5k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(vg5k_mem)
	MCFG_CPU_IO_MAP(vg5k_io)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("vg5k_scanline", vg5k_state, vg5k_scanline, "screen", 0, 10)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer", vg5k_state, z80_irq, attotime::from_msec(20))

	MCFG_DEVICE_ADD("ef9345", EF9345, 0)
	MCFG_EF9345_PALETTE("palette")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("ef9345", ef9345_device, screen_update)
	MCFG_SCREEN_SIZE(336, 300)
	MCFG_SCREEN_VISIBLE_AREA(00, 336-1, 00, 270-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", vg5k)
	MCFG_PALETTE_ADD("palette", 8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* cassette */
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(0, "mono", 0.25)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(vg5k_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MASK_SPEAKER)
	MCFG_CASSETTE_INTERFACE("vg5k_cass")

	/* printer */
	MCFG_DEVICE_ADD("printer", PRINTER, 0)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K,48k")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cass_list", "vg5k")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( vg5k )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v11", "BASIC v1.1")
	ROMX_LOAD( "vg5k11.bin",  0x0000, 0x4000, CRC(a6998ff8) SHA1(881ba594be0a721a999378312aea0c3c1c7b2b58), ROM_BIOS(1) )           // dumped from a Radiola VG-5000
	ROM_SYSTEM_BIOS(1, "v11a", "BASIC v1.1 (alt)")
	ROMX_LOAD( "vg5k11a.bin", 0x0000, 0x4000, BAD_DUMP CRC(a6f4a0ea) SHA1(58eccce33cc21fc17bc83921018f531b8001eda3), ROM_BIOS(2) )  // from dcvg5k
	ROM_SYSTEM_BIOS(2, "v10", "BASIC v1.0")
	ROMX_LOAD( "vg5k10.bin", 0x0000, 0x4000, BAD_DUMP CRC(57983260) SHA1(5ad1787a6a597b5c3eedb7c3704b649faa9be4ca), ROM_BIOS(3) )

	ROM_REGION( 0x4000, "ef9345", 0 )
	ROM_LOAD( "charset.rom", 0x0000, 0x2000, BAD_DUMP CRC(b2f49eb3) SHA1(d0ef530be33bfc296314e7152302d95fdf9520fc) )            // from dcvg5k
ROM_END

/* Driver */
/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  INIT   COMPANY     FULLNAME   FLAGS */
COMP( 1984, vg5k,   0,      0,      vg5k,    vg5k, vg5k_state,  vg5k, "Philips",  "VG-5000", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
