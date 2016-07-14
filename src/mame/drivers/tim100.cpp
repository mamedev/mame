// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/********************************************************************************************************

TIM-100 Terminal
Mihajlo Pupin Institute

2012/12/21 Skeleton driver.
2016/07/14 Fixed display etc [Robbbert]

Notes:
- Serial terminals appear to need 8 bits, 2 stop bits, odd parity @ 9600
- Unable to set these settings as default, because std::bad_cast fatal error occurs at start
- Unable to type anything as it seems uarts want BRKDET activated all the time, which we cannot do.
- Unable to find any technical info at all, so it's all guesswork.

*******************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "video/i8275.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"

class tim100_state : public driver_device
{
public:
	tim100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_crtc(*this, "crtc")
		, m_uart17(*this, "uart_u17")
		, m_uart18(*this, "uart_u18")
		{ }

	WRITE_LINE_MEMBER(drq_w);
	WRITE_LINE_MEMBER(irq_w);
	WRITE_LINE_MEMBER(clock_w);
	I8275_DRAW_CHARACTER_MEMBER( crtc_display_pixels );

private:
	virtual void machine_start() override;
	UINT8 *m_charmap;
	UINT16 m_dma_adr;
	required_shared_ptr<UINT8> m_p_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<i8275_device> m_crtc;
	required_device<i8251_device> m_uart17;
	required_device<i8251_device> m_uart18;
};

static ADDRESS_MAP_START(tim100_mem, AS_PROGRAM, 8, tim100_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM // 2764 at U16
	AM_RANGE(0x2000, 0x27ff) AM_RAM AM_SHARE("videoram") // 2KB static ram CDM6116A at U15
	AM_RANGE(0x6000, 0x6000) AM_DEVREADWRITE("uart_u17", i8251_device, data_r, data_w)
	AM_RANGE(0x6001, 0x6001) AM_DEVREADWRITE("uart_u17", i8251_device, status_r, control_w)
	AM_RANGE(0x8000, 0x8000) AM_DEVREADWRITE("uart_u18", i8251_device, data_r, data_w)
	AM_RANGE(0x8001, 0x8001) AM_DEVREADWRITE("uart_u18", i8251_device, status_r, control_w)
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP   // continuously writes 00 here
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("crtc", i8275_device, read, write) // i8276
ADDRESS_MAP_END

static ADDRESS_MAP_START(tim100_io, AS_IO, 8, tim100_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( tim100 )
INPUT_PORTS_END

DEVICE_INPUT_DEFAULTS_START( tim100 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static const rgb_t tim100_palette[3] = {
	rgb_t(0x00, 0x00, 0x00), // black
	rgb_t(0xa0, 0xa0, 0xa0), // white
	rgb_t(0xff, 0xff, 0xff)  // highlight
};

void tim100_state::machine_start()
{
	m_charmap = memregion("chargen")->base();
	m_palette->set_pen_colors(0, tim100_palette, ARRAY_LENGTH(tim100_palette));
}

const gfx_layout tim100_charlayout =
{
	12, 16,             /* 8x16 characters */
	256,                /* 128 characters */
	1,              /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{0,1,2,3,4,5,0x8000,0x8001,0x8002,0x8003,0x8004,0x8005},
	{0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8,
		8 * 8, 9 * 8, 10 * 8, 11 * 8, 12 * 8, 13 * 8, 14 * 8, 15 * 8},
	8*16                /* space between characters */
};

static GFXDECODE_START( tim100 )
	GFXDECODE_ENTRY( "chargen", 0x0000, tim100_charlayout, 0, 1 )
GFXDECODE_END


I8275_DRAW_CHARACTER_MEMBER( tim100_state::crtc_display_pixels )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 pixels;
	for (UINT8 i = 0; i < 2; i++)
	{
		pixels = m_charmap[(i * 0x1000) | (linecount & 15) | (charcode << 4)];
		if (vsp)
			pixels = 0;

		if (lten)
			pixels = 0xff;

		if (rvv)
			pixels ^= 0xff;

		bitmap.pix32(y, x++) = palette[BIT(pixels, 7) ? (hlgt ? 2 : 1) : 0];
		bitmap.pix32(y, x++) = palette[BIT(pixels, 6) ? (hlgt ? 2 : 1) : 0];
		bitmap.pix32(y, x++) = palette[BIT(pixels, 5) ? (hlgt ? 2 : 1) : 0];
		bitmap.pix32(y, x++) = palette[BIT(pixels, 4) ? (hlgt ? 2 : 1) : 0];
		bitmap.pix32(y, x++) = palette[BIT(pixels, 3) ? (hlgt ? 2 : 1) : 0];
		bitmap.pix32(y, x++) = palette[BIT(pixels, 2) ? (hlgt ? 2 : 1) : 0];
	}
}

WRITE_LINE_MEMBER( tim100_state::drq_w )
{
	if (state)
	{
		address_space& mem = m_maincpu->space(AS_PROGRAM);
		m_crtc->dack_w(mem, 0, m_p_videoram[m_dma_adr++]);
	}
}

WRITE_LINE_MEMBER( tim100_state::irq_w )
{
	if (state)
	{
		m_dma_adr = 2;
		m_maincpu->set_input_line(I8085_RST65_LINE, HOLD_LINE);
	}
	else
		m_maincpu->set_input_line(I8085_RST65_LINE, CLEAR_LINE);
}

DECLARE_WRITE_LINE_MEMBER( tim100_state::clock_w )
{
	m_uart17->write_txc(state);
	m_uart17->write_rxc(state);
	m_uart18->write_txc(state);
	m_uart18->write_rxc(state);
}


static MACHINE_CONFIG_START( tim100, tim100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A, XTAL_4_9152MHz) // divider unknown
	MCFG_CPU_PROGRAM_MAP(tim100_mem)
	MCFG_CPU_IO_MAP(tim100_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", i8275_device, screen_update)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(40*12, 16*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 40*12-1, 0, 16*16-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tim100 )

	MCFG_DEVICE_ADD("crtc", I8275, XTAL_4_9152MHz)
	MCFG_I8275_CHARACTER_WIDTH(12)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(tim100_state, crtc_display_pixels)
	MCFG_I8275_DRQ_CALLBACK(WRITELINE(tim100_state, drq_w))
	MCFG_I8275_IRQ_CALLBACK(WRITELINE(tim100_state, irq_w))
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_PALETTE_ADD("palette", 3)

	MCFG_DEVICE_ADD("uart_u17", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart_u17", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart_u17", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart_u17", i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("keyboard", tim100 )

	MCFG_DEVICE_ADD("uart_u18", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, "terminal") //"keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart_u18", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart_u18", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart_u18", i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", tim100 )

	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(tim100_state, clock_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tim100 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "tim 100 v.3.2.0.u16",   0x0000, 0x2000, CRC(4de9c8ad) SHA1(b0914d6e8d618e92a87b4b39c35391541251e8cc))

	// The first and 2nd halves of these roms are identical, confirmed ok
	ROM_REGION( 0x2000, "chargen", ROMREGION_INVERT )
	ROM_SYSTEM_BIOS( 0, "212", "v 2.1.2" )
	ROMX_LOAD( "tim 100kg v.2.1.2.u12", 0x0000, 0x0800, CRC(faf5743c) SHA1(310b662e9535878210f8aaab3e2b846fade60642),ROM_BIOS(1))
	ROM_CONTINUE (0x1000, 0x0800)
	ROM_CONTINUE (0x0800, 0x0800)
	ROM_CONTINUE (0x1800, 0x0800)
	ROM_SYSTEM_BIOS( 1, "220", "v 2.2.0" )
	ROMX_LOAD( "tim 100kg v.2.2.0.u12", 0x0000, 0x0800, CRC(358dbbd3) SHA1(14b7d6ee41b19bedf2f070f5b28b03aaff2cac4f),ROM_BIOS(2))
	ROM_CONTINUE (0x1000, 0x0800)
	ROM_CONTINUE (0x0800, 0x0800)
	ROM_CONTINUE (0x1800, 0x0800)
ROM_END

/* Driver */
COMP( 1985, tim100, 0, 0, tim100, tim100, driver_device, 0, "Mihajlo Pupin Institute", "TIM-100", MACHINE_IS_SKELETON)
