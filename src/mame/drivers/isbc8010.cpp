// license:BSD-3-Clause
// copyright-holders:Nigel Barnes,Ryan Holtz
/***************************************************************************

        Intel SBC 80/10 and SBC 80/10A Single Board Computers

The difference between the SBC 80/10 and SBC 80/10A is in the type and
quantity of memory available on each board.

There is no speaker or storage facility in the standard kit.

Download the User Manual to get the operating procedures.

Monitor Commands:
D  Display memory command
G  Program execute command
I  Insert instructions into memory
M  Move memory command
R  Read hexadecimal file
S  Substitute memory command
W  Write hexadecimal file
X  Examine and modify CPU registers command

No known manual or schematic of the video board.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"
//#include "machine/ay31015.h"

#define I8255A_1_TAG    "ppi8255_1"
#define I8255A_2_TAG    "ppi8255_2"
#define I8251A_TAG      "usart"
#define I8251A_BAUD_TAG "usart_baud"
#define RS232_TAG       "rs232"

class isbc8010_state : public driver_device
{
public:
	isbc8010_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_usart(*this, I8251A_TAG)
		, m_ppi_0(*this, I8255A_1_TAG)
		, m_ppi_1(*this, I8255A_2_TAG)
		, m_rs232(*this, RS232_TAG)
		, m_usart_baud_rate(*this, I8251A_BAUD_TAG)
		, m_usart_divide_counter(0)
		, m_usart_clock_state(0)
	{ }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER( usart_clock_tick );

private:
	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_usart;
	required_device<i8255_device> m_ppi_0;
	required_device<i8255_device> m_ppi_1;
	required_device<rs232_port_device> m_rs232;
	required_ioport m_usart_baud_rate;

	UINT8 m_usart_divide_counter;
	UINT8 m_usart_clock_state;
};

static ADDRESS_MAP_START(isbc8010_mem, AS_PROGRAM, 8, isbc8010_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x3c00, 0x3fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(isbc8010a_mem, AS_PROGRAM, 8, isbc8010_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x3c00, 0x3fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(isbc8010b_mem, AS_PROGRAM, 8, isbc8010_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x3c00, 0x3fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(isbc8010_io, AS_IO, 8, isbc8010_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xe4, 0xe7) AM_DEVREADWRITE(I8255A_1_TAG, i8255_device, read, write)
	AM_RANGE(0xe8, 0xeb) AM_DEVREADWRITE(I8255A_2_TAG, i8255_device, read, write)
	AM_RANGE(0xec, 0xec) AM_MIRROR(0x02) AM_DEVREADWRITE(I8251A_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0xed, 0xed) AM_MIRROR(0x02) AM_DEVREADWRITE(I8251A_TAG, i8251_device, status_r, control_w)
	//AM_RANGE(0xf0, 0xf7) MCS0 - iSBX Multimodule
	//AM_RANGE(0xf8, 0xff) MCS1 - iSBX Multimodule
ADDRESS_MAP_END

static INPUT_PORTS_START( isbc8010 )
	PORT_START(I8251A_BAUD_TAG)
	PORT_DIPNAME( 0x3f, 0x01, "i8251 Baud Rate" )
	PORT_DIPSETTING(    0x01, "4800")
	PORT_DIPSETTING(    0x02, "2400")
	PORT_DIPSETTING(    0x04, "1200")
	PORT_DIPSETTING(    0x08, "600")
	PORT_DIPSETTING(    0x10, "300")
	PORT_DIPSETTING(    0x20, "150")
	PORT_DIPSETTING(    0x40, "75")
INPUT_PORTS_END

#if 0
/* Graphics Output */
const gfx_layout sdk80_charlayout =
{
	7, 8,               /* character cell is 7 pixels wide by 8 pixels high */
	64,                 /* 64 characters in 2513 character generator ROM */
	1,                  /* 1 bitplane */
	{ 0 },
	/* 5 visible pixels per row, starting at bit 3, with MSB being 0: */
	{ 3, 4, 5, 6, 7 },
	/* pixel rows stored from top to bottom: */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8 * 8               /* 8 8-bit pixel rows per character */
};

static GFXDECODE_START( isbc8010 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, sdk80_charlayout, 0, 1 )
GFXDECODE_END
#endif

UINT32 isbc8010_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

WRITE_LINE_MEMBER( isbc8010_state::usart_clock_tick )
{
	UINT8 old_counter = m_usart_divide_counter;
	m_usart_divide_counter++;

	UINT8 transition = (old_counter ^ m_usart_divide_counter) & m_usart_baud_rate->read();
	if (transition)
	{
		m_usart->write_txc(m_usart_clock_state);
		m_usart->write_rxc(m_usart_clock_state);
		m_usart_clock_state ^= 1;
	}
}

static DEVICE_INPUT_DEFAULTS_START( terminal ) // set up terminal to default to 4800
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_START( isbc8010, isbc8010_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080A, XTAL_18_432MHz/9)
	MCFG_CPU_PROGRAM_MAP(isbc8010_mem)
	MCFG_CPU_IO_MAP(isbc8010_io)

	MCFG_DEVICE_ADD(I8251A_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))

	MCFG_DEVICE_ADD(I8255A_1_TAG, I8255A, 0)
	MCFG_DEVICE_ADD(I8255A_2_TAG, I8255A, 0)

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251A_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251A_TAG, i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(I8251A_TAG, i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	MCFG_DEVICE_ADD("usart_clock", CLOCK, XTAL_18_432MHz/60)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(isbc8010_state, usart_clock_tick))

	/* video hardware */
	// 96364 crt controller

//  MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
	/* Video is blanked for 70 out of 262 scanlines per refresh cycle.
	   Each scanline is composed of 65 character times, 40 of which
	   are visible, and each character time is 7 dot times; a dot time
	   is 2 cycles of the fundamental 14.31818 MHz oscillator.  The
	   total blanking time is about 4450 microseconds. */
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC((int) (70 * 65 * 7 * 2 / 14.31818)))
	/* It would be nice if we could implement some sort of display
	   overscan here. */
//  MCFG_SCREEN_SIZE(40 * 7, 24 * 8)
//  MCFG_SCREEN_VISIBLE_AREA(0, 40 * 7 - 1, 0, 24 * 8 - 1)
//  MCFG_SCREEN_UPDATE_DRIVER(sdk80_state, screen_update)
//  MCFG_SCREEN_PALETTE("palette")

//  MCFG_GFXDECODE_ADD("gfxdecode", "palette", sdk80)

//  MCFG_PALETTE_ADD_MONOCHROME("palette")

	// Video board UART
//  MCFG_DEVICE_ADD( "hd6402", AY31015, 0 )
//  MCFG_AY31015_TX_CLOCK(( XTAL_16MHz / 16 ) / 256)
//  MCFG_AY31015_RX_CLOCK(( XTAL_16MHz / 16 ) / 256)
//  MCFG_AY51013_READ_SI_CB(READ8(sdk80_state, nascom1_hd6402_si))
//  MCFG_AY51013_WRITE_SO_CB(WRITE8(sdk80_state, nascom1_hd6402_so))

	/* Devices */
//  MCFG_DEVICE_ADD("i8279", I8279, 3100000) // based on divider
//  MCFG_I8279_OUT_IRQ_CB(INPUTLINE("maincpu", I8085_RST55_LINE))   // irq
//  MCFG_I8279_OUT_SL_CB(WRITE8(sdk80_state, scanlines_w))          // scan SL lines
//  MCFG_I8279_OUT_DISP_CB(WRITE8(sdk80_state, digit_w))            // display A&B
//  MCFG_I8279_IN_RL_CB(READ8(sdk80_state, kbd_r))                  // kbd RL lines
//  MCFG_I8279_IN_SHIFT_CB(VCC)                                     // Shift key
//  MCFG_I8279_IN_CTRL_CB(VCC)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( isbc8010a, isbc8010 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(isbc8010a_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( isbc8010b, isbc8010 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(isbc8010b_mem)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( isbc8010 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("mon11")
	ROM_SYSTEM_BIOS( 0, "mon11", "80/10 Monitor v1.1" )
	ROMX_LOAD( "sbc80p.a23", 0x0000, 0x0400, CRC(bd49a7d6) SHA1(3b2f18abf35efe05f38eb08bf0c6d0f45fa7ae0a), ROM_BIOS(1))
	ROMX_LOAD( "sbc80p.a24", 0x0400, 0x0400, CRC(18131631) SHA1(6fb29df38e056c966dcc95885bc59c2a3caf4baf), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 1, "bas80", "BASIC-80" )
	ROMX_LOAD( "basic_blc_1.a24", 0x0000, 0x0400, CRC(b5e75aee) SHA1(6bd1eb9586d72544e8afb4ae43ecedcefa14da33), ROM_BIOS(2))
	ROMX_LOAD( "basic_blc_2.a25", 0x0400, 0x0400, CRC(0a9ad1ed) SHA1(92c47eadcf8b18eeedcccaa3deb9f1518aaceeae), ROM_BIOS(2))
	ROMX_LOAD( "basic_blc_3.a26", 0x0800, 0x0400, CRC(bc898e4b) SHA1(adc000534db0f736a75fbceed360dc220e02c30d), ROM_BIOS(2))
	ROMX_LOAD( "basic_blc_4.a27", 0x0c00, 0x0400, CRC(568e8b6d) SHA1(22960193d3b0ae1b5d876d8c3b3f3b40db01358c), ROM_BIOS(2))

	/* 512-byte Signetics 2513 character generator ROM at location D2-D3 */
	ROM_REGION(0x0200, "gfx1",0)
	ROM_LOAD("s2513.d2", 0x0000, 0x0200, CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee))

	/* 256x4 PROM located on the video board, schematic location P7, to be moved into separate device later */
	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6300__d7.p7",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )
ROM_END

#define rom_isbc8010a rom_isbc8010
#define rom_isbc8010b rom_isbc8010

/*    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT      CLASS           INIT   COMPANY   FULLNAME       FLAGS */
COMP( 1975, isbc8010,  0,        0,      isbc8010,  isbc8010,  driver_device,  0,     "Intel",  "iSBC 80/10",  MACHINE_NO_SOUND_HW )
COMP( 1977, isbc8010a, isbc8010, 0,      isbc8010a, isbc8010,  driver_device,  0,     "Intel",  "iSBC 80/10A", MACHINE_NO_SOUND_HW )
COMP( 1979, isbc8010b, isbc8010, 0,      isbc8010b, isbc8010,  driver_device,  0,     "Intel",  "iSBC 80/10B", MACHINE_NO_SOUND_HW )
