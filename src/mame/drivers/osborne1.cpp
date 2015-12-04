// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Vas Crabb
/***************************************************************************

    Osborne-1 driver file

The Osborne-1 memory is divided into 3 "banks".

Bank 1 simply consists of 64KB of RAM. The upper 4KB is used for the lower 8
bit of video RAM entries.

Bank 2 holds the BIOS ROM and I/O area. Only addresses 0000-3FFF are used
by bank 2 (4000-FFFF mirrors bank 1). Bank 2 is divided as follows:
3000-3FFF Nominally unused but acts as mirror of 2000-2FFF
2C00-2C03 Video PIA
2A00-2A01 Serial interface
2900-2903 488 PIA
2400-2400 SCREEN-PAC (if present)
2201-2280 Keyboard
2100-2103 Floppy
1000-1FFF Nominally unused but acts as read mirror of BIOS ROM
0000-0FFF BIOS ROM

The logic is actually quite sloppy, and will cause bus fighting under many
circumstances since it doesn't actually check all four bits, just that two
are in the desired state.

Bank 3 has the ninth bit needed to complete the full Video RAM. These bits
are stored at F000-FFFF. Only the highest bit is used.

On bootup bank 2 is active.

Banking is controlled by writes to I/O space.  Only two low address bits are
used, and the value on the data bus is completley ignored.
00 - Activate bank 2 (also triggered by CPU reset)
01 - Activate bank 1
02 - Set BIT 9 signal (map bank 3 into F000-FFFF)
03 - Clear BIT 9 signal (map bank 1/2 into F000-FFFF)

Selecting between bank 1 and bank 2 is also affected by M1 and IRQACK
conditions using a set of three flipflops.

The serial speed configuration implements wiring changes recommended in the
Osborne 1 Technical Manual.  There's no way for software to read the
selected baud rates, so it will always call the low speed "300" and the high
speed "1200".  You as the user have to keep this in mind using the system.

Serial communications can be flaky when 600/2400 is selected.  This is not a
bug in MAME.  I've checked and double-checked the schematics to confirm it's
an original bug.  The division ratio from the master clock to the baud rates
in this mode is effectively 16*24*64 or 16*24*16 giving actual data rates of
650 baud or 2600 baud, about 8.3% too fast (16*26*64 and 16*26*16 would give
the correct rates).  MAME's bitbanger seems to be able to accept the ACIA
output at this rate, but the ACIA screws up when consuming data from MAME's
bitbanger.

Schematics specify a WD1793 floppy controller, but we're using the Fujitsu
equivalent MB8877 here.  Is it known that the original machines used one or
the other exclusively?  In any case MAME emulates them identically.


TODO:

* Hook up the port direction control bits in the IEEE488 interface properly
  and test it with some emulated peripheral.  Also the BIOS can speak
  Centronics parallel over the same physical interface, so this should be
  tested, too.

***************************************************************************/

#include "includes/osborne1.h"
#include "bus/rs232/rs232.h"
#include "softlist.h"

#define MAIN_CLOCK  15974400


static ADDRESS_MAP_START( osborne1_mem, AS_PROGRAM, 8, osborne1_state )
	AM_RANGE( 0x0000, 0x0FFF ) AM_READ_BANK("bank_0xxx") AM_WRITE(bank_0xxx_w)
	AM_RANGE( 0x1000, 0x1FFF ) AM_READ_BANK("bank_1xxx") AM_WRITE(bank_1xxx_w)
	AM_RANGE( 0x2000, 0x3FFF ) AM_READWRITE(bank_2xxx_3xxx_r, bank_2xxx_3xxx_w)
	AM_RANGE( 0x4000, 0xEFFF ) AM_RAM
	AM_RANGE( 0xF000, 0xFFFF ) AM_READ_BANK("bank_fxxx") AM_WRITE(videoram_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( osborne1_op, AS_DECRYPTED_OPCODES, 8, osborne1_state )
	AM_RANGE( 0x0000, 0xFFFF ) AM_READ(opcode_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( osborne1_io, AS_IO, 8, osborne1_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0xff ) AM_WRITE(bankswitch_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( osborne1 )
	PORT_START("ROW0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)       PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)            PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)            PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)            PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("ROW3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)            PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)            PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)            PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)            PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)            PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)            PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)            PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)            PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("ROW4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)            PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)            PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)            PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)            PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)            PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)            PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)            PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)            PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("ROW5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)            PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)            PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)            PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)            PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)            PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)            PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)            PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("ROW6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)            PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("ROW7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME( 0x08, 0, "Alpha Lock" ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("RESET")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RESET") PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("CNF")
	PORT_BIT(0xF8, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x06, 0x00, "Serial Speed")
	PORT_CONFSETTING(0x00, "300/1200")
	PORT_CONFSETTING(0x02, "600/2400")
	PORT_CONFSETTING(0x04, "1200/4800")
	PORT_CONFSETTING(0x06, "2400/9600")
	PORT_CONFNAME(0x01, 0x00, "Video Output")
	PORT_CONFSETTING(0x00, "Standard")
	PORT_CONFSETTING(0x01, "SCREEN-PAC")
INPUT_PORTS_END


/*
 * The Osborne-1 supports the following disc formats:
 * - Osborne single density: 40 tracks, 10 sectors per track, 256-byte sectors (100 KByte)
 * - Osborne double density: 40 tracks, 5 sectors per track, 1024-byte sectors (200 KByte)
 * - IBM Personal Computer: 40 tracks, 8 sectors per track, 512-byte sectors (160 KByte)
 * - Xerox 820 Computer: 40 tracks, 18 sectors per track, 128-byte sectors (90 KByte)
 * - DEC 1820 double density: 40 tracks, 9 sectors per track, 512-byte sectors (180 KByte)
 *
 */

static SLOT_INTERFACE_START( osborne1_floppies )
	SLOT_INTERFACE("525sssd", FLOPPY_525_SSSD) // Siemens FDD 100-5, custom Osborne electronics
	SLOT_INTERFACE("525ssdd", FLOPPY_525_SSDD) // MPI 52(?), custom Osborne electronics
SLOT_INTERFACE_END


/* F4 Character Displayer */
static const gfx_layout osborne1_charlayout =
{
	8, 10,              // 8 x 10 characters
	128,                // 128 characters
	1,                  // 1 bits per pixel
	{ 0 },              // no bitplanes
	// x offsets
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	// y offsets
	{ 0*128*8, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8, 8*128*8, 9*128*8 },
	8                   // every char takes 16 x 1 bytes
};

static GFXDECODE_START( osborne1 )
	GFXDECODE_ENTRY("chargen", 0x0000, osborne1_charlayout, 0, 1)
GFXDECODE_END


static MACHINE_CONFIG_START( osborne1, osborne1_state )
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(osborne1_mem)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(osborne1_op)
	MCFG_CPU_IO_MAP(osborne1_io)
	MCFG_Z80_SET_IRQACK_CALLBACK(WRITELINE(osborne1_state, irqack_w))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(osborne1_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS( MAIN_CLOCK, 1024, 0, 104*8, 260, 0, 24*10 )
	MCFG_SCREEN_PALETTE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", osborne1)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN_HIGHLIGHT("palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_DEVICE_ADD("pia_0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(DEVREAD8(IEEE488_TAG, ieee488_device, dio_r))
	MCFG_PIA_READPB_HANDLER(READ8(osborne1_state, ieee_pia_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8(IEEE488_TAG, ieee488_device, dio_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(osborne1_state, ieee_pia_pb_w))
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE(IEEE488_TAG, ieee488_device, ifc_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE(IEEE488_TAG, ieee488_device, ren_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(osborne1_state, ieee_pia_irq_a_func))

	MCFG_IEEE488_BUS_ADD()
	MCFG_IEEE488_SRQ_CALLBACK(DEVWRITELINE("pia_0", pia6821_device, ca2_w))

	MCFG_DEVICE_ADD("pia_1", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(osborne1_state, video_pia_port_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(osborne1_state, video_pia_port_b_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(osborne1_state, video_pia_out_cb2_dummy))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(osborne1_state, video_pia_irq_a_func))

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(osborne1_state, serial_acia_irq_func))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_dcd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("pia_1", pia6821_device, ca2_w))

	MCFG_DEVICE_ADD("mb8877", MB8877, MAIN_CLOCK/16)
	MCFG_WD_FDC_FORCE_READY
	MCFG_FLOPPY_DRIVE_ADD("mb8877:0", osborne1_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("mb8877:1", osborne1_floppies, "525ssdd", floppy_image_device::default_floppy_formats)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("68K")    // 64kB main RAM and 4kbit video attribute RAM

	MCFG_SOFTWARE_LIST_ADD("flop_list","osborne1")
MACHINE_CONFIG_END


ROM_START( osborne1 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "ver144", "BIOS version 1.44" )
	ROMX_LOAD( "3a10082-00rev-e.ud11", 0x0000, 0x1000, CRC(c0596b14) SHA1(ee6a9cc9be3ddc5949d3379351c1d58a175ce9ac), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "verA", "BIOS version A" )
	ROMX_LOAD( "osba.bin", 0x0000, 0x1000, NO_DUMP, ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "ver12", "BIOS version 1.2" )
	ROMX_LOAD( "osb12.bin", 0x0000, 0x1000, NO_DUMP, ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "ver121", "BIOS version 1.2.1" )
	ROMX_LOAD( "osb121.bin", 0x0000, 0x1000, NO_DUMP, ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "ver13", "BIOS version 1.3" )
	ROMX_LOAD( "osb13.bin", 0x0000, 0x1000, NO_DUMP, ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 5, "ver14", "BISO version 1.4" )
	ROMX_LOAD( "rev1.40.ud11", 0x0000, 0x1000, CRC(3d966335) SHA1(0c60b97a3154a75868efc6370d26995eadc7d927), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 6, "ver143", "BIOS version 1.43" )
	ROMX_LOAD( "rev1.43.ud11", 0x0000, 0x1000, CRC(91a48e3c) SHA1(c37b83f278d21e6e92d80f9c057b11f7f22d88d4), ROM_BIOS(7) )
	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "7a3007-00.ud15", 0x0000, 0x800, CRC(6c1eab0d) SHA1(b04459d377a70abc9155a5486003cb795342c801) )
	//ROM_LOAD( "char.ua15", 0x0000, 0x800, CRC(5297C109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594) ) // this is CHRROM from v1.4 BIOS MB. I don't know how to hook up diff CHR to ROM_BIOS(6)

ROM_END

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT        COMPANY     FULLNAME        FLAGS */
COMP( 1981, osborne1,   0,      0,      osborne1,   osborne1, osborne1_state,   osborne1,   "Osborne",  "Osborne-1",    MACHINE_SUPPORTS_SAVE )
