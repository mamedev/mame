// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*******************************************************************************

Primo driver by Krzysztof Strzecha

What's new:
-----------
2009.07.19 -    Removed hacked and duplicated ROMs (Krzysztof Strzecha)
2008.08.31 -    Added new ROMs including B32 and B48 [Miodrag Milanovic]
2005.05.19 -    Primo B-32 and B-48 testdrivers added.
2005.05.15 -    EPROM+RAM expansion.
        Support for .pp files improved.
        Some cleanups.
2005.05.12 -    Memory fixed for A-48 model what make it fully working.
        Fixed address of second video memory area.

To do:
    1. Disk
    2. V.24 / tape control
    3. CDOS autoboot
    4. Joystick
    5. Printer
    6. .PRI format

Primo variants:
    A-32 - 16kB RAM + 16KB ROM
    A-48 - 32kB RAM + 16KB ROM
    A-64 - 48kB RAM + 16KB ROM
    B-32 - 16kB RAM + 16KB ROM
    B-48 - 32kB RAM + 16KB ROM
    B-64 - 48kB RAM + 16KB ROM

CPU:
    U880D (DDR clone of Z80), 2.5 MHz

Memory map:
    A-32, B-32
    0000-3fff ROM
    4000-7fff RAM
    8000-bfff not mapped
    c000-ffff not mapped

    A-48, B-48
    0000-3fff ROM
    4000-7fff RAM
    8000-bfff RAM
    c000-ffff not mapped

    A-64, B-64
    0000-3fff ROM
    4000-7fff RAM
    8000-bfff RAM
    c000-ffff RAM

I/O Ports:
    Write:
    00h-3fh
        bit 7 - NMI
                0 - disable
                1 - enable
        bit 6 - joystick register shift
        bit 5 - V.24 (2) / tape control
        bit 4 - speaker
        bit 3 - display page
                0 - secondary
                1 - primary
        bit 2 - V.24 (1) / tape control
        bit 1 - cassette output
        bit 0 /
                00 - -110mV
                01 - 0V
                10 - 0V
                11 - 110mV
    40h-ffh (B models only)
        bit 7 - not used
        bit 6 - not used
        bit 5 - SCLK
        bit 4 - SDATA
        bit 3 - not used
        bit 2 - SRQ
        bit 1 - ATN
        bit 0 - not used
    Read:
    00h-3fh
        bit 7 - not used
        bit 6 - not used
        bit 5 - VBLANK
        bit 4 - I4
        bit 3 - I3
        bit 2 - cassette input
        bit 1 - reset
        bit 0 - keyboard input
    40h-ffh (B models only)
        bit 7 - not used
        bit 6 - not used
        bit 5 - SCLK
        bit 4 - SDATA
        bit 3 - SRQ
        bit 2 - joystick 2
        bit 1 - ATN
        bit 0 - joystick 1

Interrupts:
    NMI - 20ms (50HZ), can be disbled/enabled by I/O write

TODO:
- primoc64 works, but it seems it should be used with a commodore 64
  disk drive. This isn't emulated.

*******************************************************************************/

#include "emu.h"
#include "primo.h"

#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/primoptp.h"


void primo_state::primoa_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).rw(FUNC(primo_state::be_1_r), FUNC(primo_state::ki_1_w));
	map(0xfd, 0xfd).w(FUNC(primo_state::fd_w));
}

void primo_state::primob_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).rw(FUNC(primo_state::be_1_r), FUNC(primo_state::ki_1_w));
	map(0x40, 0x7f).rw(FUNC(primo_state::be_2_r), FUNC(primo_state::ki_2_w));
	map(0xfd, 0xfd).w(FUNC(primo_state::fd_w));
}

void primo_state::primo32_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x7fff).ram().share("videoram").mirror(0x8000);
}

void primo_state::primo48_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x7fff).ram();
	map(0x8000, 0xbfff).ram().share("videoram").mirror(0x4000);
}

void primo_state::primo64_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0xbfff).ram();
	map(0xc000, 0xffff).ram().share("videoram");
}

static INPUT_PORTS_START( primo )
	PORT_START( "IN0" )
		PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)       PORT_CHAR('y') PORT_CHAR('Y')
		PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)      PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)       PORT_CHAR('s') PORT_CHAR('S')
		PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)       PORT_CHAR('e') PORT_CHAR('E')
		PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UPPER") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
		PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)       PORT_CHAR('w') PORT_CHAR('W')
		PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTR")   PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
		PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)       PORT_CHAR('d') PORT_CHAR('D')
		PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)       PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)       PORT_CHAR('x') PORT_CHAR('X')
		PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)       PORT_CHAR('2') PORT_CHAR('\"')
		PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)       PORT_CHAR('q') PORT_CHAR('Q')
		PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)       PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)       PORT_CHAR('a') PORT_CHAR('A')
		PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START( "IN1" )
		PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)       PORT_CHAR('c') PORT_CHAR('C')
		PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
		PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)       PORT_CHAR('f') PORT_CHAR('F')
		PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
		PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)       PORT_CHAR('r') PORT_CHAR('R')
		PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
		PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)       PORT_CHAR('t') PORT_CHAR('T')
		PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)       PORT_CHAR('7') PORT_CHAR('/')
		PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)       PORT_CHAR('h') PORT_CHAR('H')
		PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)   PORT_CHAR(' ')
		PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)       PORT_CHAR('b') PORT_CHAR('B')
		PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)       PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)       PORT_CHAR('g') PORT_CHAR('G')
		PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)       PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)       PORT_CHAR('v') PORT_CHAR('V')
		PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)       PORT_CHAR('4') PORT_CHAR('$')

	PORT_START( "IN2" )
		PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)       PORT_CHAR('n') PORT_CHAR('N')
		PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)       PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)       PORT_CHAR('z') PORT_CHAR('Z')
		PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)   PORT_CHAR('+') PORT_CHAR('?')
		PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)       PORT_CHAR('u') PORT_CHAR('U')
		PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)       PORT_CHAR('0') PORT_CHAR('=')
		PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)       PORT_CHAR('j') PORT_CHAR('J')
		PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME)    PORT_CHAR('>') PORT_CHAR('<')
		PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)       PORT_CHAR('l') PORT_CHAR('L')
		PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)   PORT_CHAR('-') PORT_CHAR(U'í')
		PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)       PORT_CHAR('k') PORT_CHAR('K')
		PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)    PORT_CHAR('.') PORT_CHAR(':')
		PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)       PORT_CHAR('m') PORT_CHAR('M')
		PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)       PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)       PORT_CHAR('i') PORT_CHAR('I')
		PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)   PORT_CHAR(',') PORT_CHAR(';')

	PORT_START( "IN3" )
		PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                      PORT_CHAR(U'ü') PORT_CHAR(U'Ü')
		PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                   PORT_CHAR('*') PORT_CHAR('\'')
		PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)                           PORT_CHAR('p') PORT_CHAR('P')
		PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END)                         PORT_CHAR(U'ú') PORT_CHAR(U'ű')
		PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)                           PORT_CHAR('o') PORT_CHAR('O')
		PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CLS") PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
		PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
		PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER)   PORT_CHAR(13)
		PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
		PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)                        PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)                       PORT_CHAR(U'é') PORT_CHAR(U'É')
		PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                  PORT_CHAR(U'ó') PORT_CHAR(U'ő')
		PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                       PORT_CHAR(U'á') PORT_CHAR(U'Á')
		PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)                       PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)                   PORT_CHAR(U'ö') PORT_CHAR(U'Ö')
		PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BRK") PORT_CODE(KEYCODE_ESC)        PORT_CHAR(27)

	PORT_START( "RESET" )
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
		PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START( "CPU_CLOCK" )
		PORT_CONFNAME(0x01, 0x00, "CPU clock" )
		PORT_CONFSETTING(   0x00, "2.50 MHz" )
		PORT_CONFSETTING(   0x01, "3.75 MHz" )

	PORT_START( "MEMORY_EXPANSION" )
		PORT_CONFNAME(0x01, 0x00, "EPROM+RAM Expansion" )
		PORT_CONFSETTING(   0x00, DEF_STR( On ) )
		PORT_CONFSETTING(   0x01, DEF_STR( Off ) )
INPUT_PORTS_END

static const cassette_image::Options primo_cassette_options =
{
	1,      /* channels */
	16,     /* bits per sample */
	22050   /* sample frequency */
};

void primo_state::primoa32(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2500000);
	m_maincpu->set_addrmap(AS_PROGRAM, &primo_state::primo32_mem);
	m_maincpu->set_addrmap(AS_IO, &primo_state::primoa_io);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(256, 192);
	m_screen->set_visarea(0, 256-1, 0, 192-1);
	m_screen->set_screen_update(FUNC(primo_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set(FUNC(primo_state::vblank_irq));

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* snapshot/quickload */
	SNAPSHOT(config, "snapshot", "pss").set_load_callback(FUNC(primo_state::snapshot_cb));
	QUICKLOAD(config, "quickload", "pp").set_load_callback(FUNC(primo_state::quickload_cb));

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(primo_ptp_format);
	m_cassette->set_create_opts(&primo_cassette_options);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* floppy from serial bus */
	cbm_iec_slot_device::add(config, m_iec, nullptr);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart1, generic_plain_slot, nullptr, "bin,rom");
	GENERIC_CARTSLOT(config, m_cart2, generic_plain_slot, nullptr, "bin,rom");
}

void primo_state::primoa48(machine_config &config)
{
	primoa32(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &primo_state::primo48_mem);
}

void primo_state::primoa64(machine_config &config)
{
	primoa32(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &primo_state::primo64_mem);
}

void primo_state::primob32(machine_config &config)
{
	primoa32(config);
	m_maincpu->set_addrmap(AS_IO, &primo_state::primob_io);
}

void primo_state::primob48(machine_config &config)
{
	primoa48(config);
	m_maincpu->set_addrmap(AS_IO, &primo_state::primob_io);
}

void primo_state::primob64(machine_config &config)
{
	primoa64(config);
	m_maincpu->set_addrmap(AS_IO, &primo_state::primob_io);
}

void primo_state::primoc64(machine_config &config)
{
	primoa64(config);
	m_maincpu->set_addrmap(AS_IO, &primo_state::primob_io);
}

ROM_START( primoa32 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "a32_1.rom", 0x0000, 0x1000, CRC(4e91c1a4) SHA1(bf6e41b6b36a2556a50065e9acfd8cd57968f039) )
	ROM_LOAD( "a32_2.rom", 0x1000, 0x1000, CRC(81a8a0fb) SHA1(df75bd7774969cabb062e50da6004f2efbde489e) )
	ROM_LOAD( "a32_3.rom", 0x2000, 0x1000, CRC(a97de2f5) SHA1(743c76121f5b9e1eab35c8c00797311f58da5243) )
	ROM_LOAD( "a32_4.rom", 0x3000, 0x1000, CRC(70f84bc8) SHA1(9ae1c06531edf20c14ba47e78c0747dd2a92612a) )
ROM_END

ROM_START( primoa48 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "a48_1.rom", 0x0000, 0x1000, CRC(7de6ad6f) SHA1(f2fd6fac4f9bc57c646efe40281758bb7c3f56e1) )
	ROM_LOAD( "a48_2.rom", 0x1000, 0x1000, CRC(81a8a0fb) SHA1(df75bd7774969cabb062e50da6004f2efbde489e) )
	ROM_LOAD( "a48_3.rom", 0x2000, 0x1000, CRC(a97de2f5) SHA1(743c76121f5b9e1eab35c8c00797311f58da5243) )
	ROM_LOAD( "a48_4.rom", 0x3000, 0x1000, CRC(e4d0c452) SHA1(4a98ff7502f1236445250d6b4e1c34850734350e) )
ROM_END

ROM_START( primoa64 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "a64_1.rom", 0x0000, 0x1000, CRC(6a7a9b9b) SHA1(e9ce16f90d9a799a26a9cef09d9ee6a6d7749484) )
	ROM_LOAD( "a64_2.rom", 0x1000, 0x1000, CRC(81a8a0fb) SHA1(df75bd7774969cabb062e50da6004f2efbde489e) )
	ROM_LOAD( "a64_3.rom", 0x2000, 0x1000, CRC(a97de2f5) SHA1(743c76121f5b9e1eab35c8c00797311f58da5243) )
	ROM_LOAD( "a64_4.rom", 0x3000, 0x1000, CRC(e4d0c452) SHA1(4a98ff7502f1236445250d6b4e1c34850734350e) )
ROM_END

ROM_START( primob32 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "b32.rom",   0x0000, 0x4000, CRC(f594d2bb) SHA1(b74961dba008a1a6f15a22ddbd1b89acd7e286c2) )
ROM_END

ROM_START( primob48 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "b48.rom",   0x0000, 0x4000, CRC(df3d2a57) SHA1(ab9413aa9d7749d30a486da00bc8c6d178a2172c) )
ROM_END

ROM_START( primob64 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "std", "Standard")
	ROMX_LOAD( "b64.rom",     0x0000, 0x4000, CRC(cea28188) SHA1(a77e42e97402e601b78ab3751eac1e85d0bbb4a0), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "cdos", "CDOS")
	ROMX_LOAD( "b64cdos.rom", 0x0000, 0x4000, CRC(73305e4d) SHA1(c090c3430cdf19eed8363377b981e1c21a4ed169), ROM_BIOS(1) )
ROM_END

ROM_START( primoc64 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "c64_1.rom", 0x0000, 0x1000, CRC(c22290ea) SHA1(af5c73f6d0f7a987c4f082a5cb69e3f016127d57) )
	ROM_LOAD( "c64_2.rom", 0x1000, 0x1000, CRC(0756b56e) SHA1(589dbdb7c43ca7ca29ed1e56e080adf8c069e407) )
	ROM_LOAD( "c64_3.rom", 0x2000, 0x1000, CRC(fc56e0af) SHA1(b547fd270d3413400bc800f5b7ea9153b7a59bff) )
	ROM_LOAD( "c64_4.rom", 0x3000, 0x1000, CRC(3770e3e6) SHA1(792cc71d8f89eb447f94aded5afc70d626a26030) )
ROM_END

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT  CLASS        INIT        COMPANY     FULLNAME      FLAGS
COMP( 1984, primoa32, 0,        0,      primoa32, primo, primo_state, empty_init, "Microkey", "Primo A-32", MACHINE_SUPPORTS_SAVE )
COMP( 1984, primoa48, primoa32, 0,      primoa48, primo, primo_state, empty_init, "Microkey", "Primo A-48", MACHINE_SUPPORTS_SAVE )
COMP( 1984, primoa64, primoa32, 0,      primoa64, primo, primo_state, empty_init, "Microkey", "Primo A-64", MACHINE_SUPPORTS_SAVE )
COMP( 1984, primob32, primoa32, 0,      primob32, primo, primo_state, empty_init, "Microkey", "Primo B-32", MACHINE_SUPPORTS_SAVE )
COMP( 1984, primob48, primoa32, 0,      primob48, primo, primo_state, empty_init, "Microkey", "Primo B-48", MACHINE_SUPPORTS_SAVE )
COMP( 1984, primob64, primoa32, 0,      primob64, primo, primo_state, empty_init, "Microkey", "Primo B-64", MACHINE_SUPPORTS_SAVE )
COMP( 1984, primoc64, primoa32, 0,      primoc64, primo, primo_state, empty_init, "Microkey", "Primo C-64", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
