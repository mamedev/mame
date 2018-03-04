// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Mannesmann Kienzle System 9002 Terminal

        2017-08-17 Skeleton driver.

        Chips used:
            Siemens SAB8085A-P
            NEC D8251AFC * 2
            NEC D4016C-3 * 4 + 2
            ST M2764A-4F1 * 4
            HD6845P

Seems a chargen is missing.
If write to the 6845 is enabled, MAME freezes after 1 or 2 seconds.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "video/mc6845.h"
#include "screen.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"

class sys9002_state : public driver_device
{
public:
	sys9002_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "videoram")
		, m_palette(*this, "palette")
	{ }

	MC6845_UPDATE_ROW(crtc_update_row);

	void sys9002(machine_config &config);
	void sys9002_io(address_map &map);
	void sys9002_mem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_device<palette_device> m_palette;
};


ADDRESS_MAP_START(sys9002_state::sys9002_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM // 4 * 4K ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM // 4 * 2k RAM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("videoram") // 2k RAM
	AM_RANGE(0xc000, 0xc07f) AM_RAM // ??
ADDRESS_MAP_END

ADDRESS_MAP_START(sys9002_state::sys9002_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x04, 0x04) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)  // left commented out as mame freezes after about 2 seconds
	//AM_RANGE(0x05, 0x05) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x08, 0x08) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	AM_RANGE(0x09, 0x09) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w) // 7 bits even parity, x64
	AM_RANGE(0x11, 0x11) AM_READNOP  // continuous read
	AM_RANGE(0x1c, 0x1c) AM_DEVREADWRITE("uart2", i8251_device, data_r, data_w)
	AM_RANGE(0x1d, 0x1d) AM_DEVREADWRITE("uart2", i8251_device, status_r, control_w) // enabled for transmit only, 8 bits odd parity, x64
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sys9002 )
INPUT_PORTS_END

MC6845_UPDATE_ROW( sys9002_state::crtc_update_row )
{
	const rgb_t *pens = m_palette->palette()->entry_list_raw();
	uint8_t chr,gfx;
	uint16_t mem,x;
	uint32_t *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];

		/* get pattern of pixels for that character scanline */
		gfx = chr; //gfx = m_p_chargen[(chr<<4) | ra] ^ ((x == cursor_x) ? 0xff : 0);

		/* Display a scanline of a character (8 pixels) */
		*p++ = pens[BIT(gfx, 7)];
		*p++ = pens[BIT(gfx, 6)];
		*p++ = pens[BIT(gfx, 5)];
		*p++ = pens[BIT(gfx, 4)];
		*p++ = pens[BIT(gfx, 3)];
		*p++ = pens[BIT(gfx, 2)];
		*p++ = pens[BIT(gfx, 1)];
		*p++ = pens[BIT(gfx, 0)];
	}
}


static DEVICE_INPUT_DEFAULTS_START( uart1 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( uart2 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

MACHINE_CONFIG_START(sys9002_state::sys9002)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A, XTAL(2'000'000)) // XTAL not visible on images
	MCFG_CPU_PROGRAM_MAP(sys9002_mem)
	MCFG_CPU_IO_MAP(sys9002_io)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not correct
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	//MCFG_GFXDECODE_ADD("gfxdecode", "palette", mx2178)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* Devices */
	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL(2'000'000)) // clk unknown
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(sys9002_state, crtc_update_row)

	MCFG_DEVICE_ADD("uart_clock", CLOCK, 614400)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("uart1", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart1", i8251_device, write_rxc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart2", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart2", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("uart1", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart1", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart1", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart1", i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", uart1)

	MCFG_DEVICE_ADD("uart2", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232b", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart2", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart2", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart2", i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", uart2)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sys9002 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "55-040.bin", 0x0000, 0x2000, CRC(781eaca9) SHA1(1bdae2bcc43deaef2eb1d6ec302fbadbb779fd48))
	ROM_LOAD( "55-041.bin", 0x2000, 0x2000, CRC(0f89fe81) SHA1(2dc8de7dabaf11a150cfd34460c5b47612cf5e61))
	ROM_LOAD( "55-042.bin", 0x4000, 0x2000, CRC(e6fbc837) SHA1(fc11f6a6927709552bedf06b9eb0dc66e9a81264))
	ROM_LOAD( "55-048.bin", 0x6000, 0x2000, CRC(879ef945) SHA1(a54fc01ac26a3cd05f6d1e1139d6d99198556575))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    STATE          INIT  COMPANY                FULLNAME                FLAGS
COMP( 198?, sys9002, 0,      0,       sys9002,   sys9002, sys9002_state, 0,    "Mannesmann Kienzle",  "System 9002 Terminal", MACHINE_IS_SKELETON )
