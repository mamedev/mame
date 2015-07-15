// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

        Intel SDK-80

This is an evaluation kit for the 8080 cpu.

There is no speaker or storage facility in the standard kit.

Download the User Manual to get the operating procedures.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/pit8253.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "machine/ay31015.h"
#include "bus/rs232/rs232.h"


class sdk80_state : public driver_device
{
public:
	sdk80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_WRITE8_MEMBER(scanlines_w);
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_READ8_MEMBER(kbd_r);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	UINT8 m_digit;
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(sdk80_mem, AS_PROGRAM, 8, sdk80_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x3000, 0x3fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(sdk80_io, AS_IO, 8, sdk80_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static INPUT_PORTS_START( sdk80 )
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

static GFXDECODE_START( sdk80 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, sdk80_charlayout, 0, 1 )
GFXDECODE_END
#endif

UINT32 sdk80_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

WRITE8_MEMBER( sdk80_state::scanlines_w )
{
	m_digit = data;
}

WRITE8_MEMBER( sdk80_state::digit_w )
{
	if (m_digit < 6)
		output_set_digit_value(m_digit, BITSWAP8(data, 3, 2, 1, 0, 7, 6, 5, 4)^0xff);
}

READ8_MEMBER( sdk80_state::kbd_r )
{
	UINT8 data = 0xff;

	if (m_digit < 3)
	{
		char kbdrow[6];
		sprintf(kbdrow,"X%X",m_digit);
		data = ioport(kbdrow)->read();
	}
	return data;
}

static MACHINE_CONFIG_START( sdk80, sdk80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080A, 500000)
	MCFG_CPU_PROGRAM_MAP(sdk80_mem)
	MCFG_CPU_IO_MAP(sdk80_io)

	MCFG_DEVICE_ADD("uart", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart", i8251_device, write_dsr))

// old references to other drivers have been left in
//	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
//	MCFG_PIT8253_CLK0(MAIN_CLOCK_X1) /* heartbeat IRQ */
//	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259_master", pic8259_device, ir0_w))
//	MCFG_PIT8253_CLK1(MAIN_CLOCK_X1) /* Memory Refresh */
//	MCFG_PIT8253_CLK2(MAIN_CLOCK_X1) /* RS-232c */
//	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(pc9801_state, write_uart_clock))

//	MCFG_DEVICE_ADD("ppi8255_sys", I8255, 0)
//	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW2"))
//	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW1"))
//	MCFG_I8255_IN_PORTC_CB(CONSTANT(0xa0)) // 0x80 cpu triple fault reset flag?
//	MCFG_I8255_OUT_PORTC_CB(WRITE8(pc9801_state, ppi_sys_portc_w))

//	MCFG_DEVICE_ADD("ppi8255_prn", I8255, 0)
	/* TODO: check this one */
//	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW5"))

	/* video hardware */
	// 96364 crt controller

//	MCFG_SCREEN_ADD("screen", RASTER)
//	MCFG_SCREEN_REFRESH_RATE(60)
	/* Video is blanked for 70 out of 262 scanlines per refresh cycle.
	   Each scanline is composed of 65 character times, 40 of which
	   are visible, and each character time is 7 dot times; a dot time
	   is 2 cycles of the fundamental 14.31818 MHz oscillator.  The
	   total blanking time is about 4450 microseconds. */
//	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC((int) (70 * 65 * 7 * 2 / 14.31818)))
	/* It would be nice if we could implement some sort of display
	   overscan here. */
//	MCFG_SCREEN_SIZE(40 * 7, 24 * 8)
//	MCFG_SCREEN_VISIBLE_AREA(0, 40 * 7 - 1, 0, 24 * 8 - 1)
//	MCFG_SCREEN_UPDATE_DRIVER(sdk80_state, screen_update)
//	MCFG_SCREEN_PALETTE("palette")

//	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sdk80)

//	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	// uart
//	MCFG_DEVICE_ADD( "hd6402", AY31015, 0 )
//	MCFG_AY31015_TX_CLOCK(( XTAL_16MHz / 16 ) / 256)
//	MCFG_AY31015_RX_CLOCK(( XTAL_16MHz / 16 ) / 256)
//	MCFG_AY51013_READ_SI_CB(READ8(sdk80_state, nascom1_hd6402_si))
//	MCFG_AY51013_WRITE_SO_CB(WRITE8(sdk80_state, nascom1_hd6402_so))

	/* Devices */
//	MCFG_DEVICE_ADD("i8279", I8279, 3100000) // based on divider
//	MCFG_I8279_OUT_IRQ_CB(INPUTLINE("maincpu", I8085_RST55_LINE))   // irq
//	MCFG_I8279_OUT_SL_CB(WRITE8(sdk80_state, scanlines_w))          // scan SL lines
//	MCFG_I8279_OUT_DISP_CB(WRITE8(sdk80_state, digit_w))            // display A&B
//	MCFG_I8279_IN_RL_CB(READ8(sdk80_state, kbd_r))                  // kbd RL lines
//	MCFG_I8279_IN_SHIFT_CB(VCC)                                     // Shift key
//	MCFG_I8279_IN_CTRL_CB(VCC)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sdk80 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basic_blc_1.a14", 0x0000, 0x0400, CRC(b5e75aee) SHA1(6bd1eb9586d72544e8afb4ae43ecedcefa14da33))
	ROM_LOAD( "basic_blc_2.a15", 0x0400, 0x0400, CRC(0a9ad1ed) SHA1(92c47eadcf8b18eeedcccaa3deb9f1518aaceeae))
	ROM_LOAD( "basic_blc_3.a16", 0x0800, 0x0400, CRC(bc898e4b) SHA1(adc000534db0f736a75fbceed360dc220e02c30d))
	ROM_LOAD( "basic_blc_4.a17", 0x0c00, 0x0400, CRC(568e8b6d) SHA1(22960193d3b0ae1b5d876d8c3b3f3b40db01358c))

	/* 512-byte Signetics 2513 character generator ROM at location D2-D3 */
	ROM_REGION(0x0200, "gfx1",0)
	ROM_LOAD("s2513.d2", 0x0000, 0x0200, CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee))
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE    INPUT   CLASS           INIT   COMPANY   FULLNAME  FLAGS */
COMP( 1975, sdk80,  0,       0,     sdk80,     sdk80,  driver_device,  0,     "Intel",  "SDK-80", GAME_NO_SOUND_HW)
