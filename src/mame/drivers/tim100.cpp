// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/********************************************************************************************************

TIM-100 Terminal
Mihajlo Pupin Institute

2012/12/21 Skeleton driver.
2016/07/14 Fixed display etc [Robbbert]

Notes:
- Serial terminals appear to need 8 bits, 2 stop bits, odd parity @ 9600
- Unable to type anything as it seems uarts want BRKDET activated all the time, which we cannot do.
- Unable to find any technical info at all, so it's all guesswork.

*******************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "video/i8275.h"
#include "emupal.h"
#include "screen.h"

class tim100_state : public driver_device
{
public:
	tim100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_charmap(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_crtc(*this, "crtc")
	{ }

	void tim100(machine_config &config);

private:
	DECLARE_WRITE_LINE_MEMBER(drq_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	I8275_DRAW_CHARACTER_MEMBER( crtc_display_pixels );

	void tim100_io(address_map &map);
	void tim100_mem(address_map &map);

	virtual void machine_start() override;

	uint16_t m_dma_adr;
	required_region_ptr<uint8_t> m_charmap;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<i8276_device> m_crtc;
};

void tim100_state::tim100_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom(); // 2764 at U16
	map(0x2000, 0x27ff).ram().share("videoram"); // 2KB static ram CDM6116A at U15
	map(0x6000, 0x6001).rw("uart_u17", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x8000, 0x8001).rw("uart_u18", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xa000, 0xa000).nopw();   // continuously writes 00 here
	map(0xc000, 0xc001).rw(m_crtc, FUNC(i8276_device::read), FUNC(i8276_device::write)); // i8276
}

void tim100_state::tim100_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
}


/* Input ports */
static INPUT_PORTS_START( tim100 )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( tim100 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

static const rgb_t tim100_palette[3] = {
	rgb_t(0x00, 0x00, 0x00), // black
	rgb_t(0xa0, 0xa0, 0xa0), // white
	rgb_t(0xff, 0xff, 0xff)  // highlight
};

void tim100_state::machine_start()
{
	m_palette->set_pen_colors(0, tim100_palette);
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

static GFXDECODE_START( gfx_tim100 )
	GFXDECODE_ENTRY( "chargen", 0x0000, tim100_charlayout, 0, 1 )
GFXDECODE_END


I8275_DRAW_CHARACTER_MEMBER( tim100_state::crtc_display_pixels )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint8_t pixels;
	for (uint8_t i = 0; i < 2; i++)
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


void tim100_state::tim100(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(4'915'200)); // divider unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &tim100_state::tim100_mem);
	m_maincpu->set_addrmap(AS_IO, &tim100_state::tim100_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update("crtc", FUNC(i8276_device::screen_update));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(600, 352);
	screen.set_visarea(0, 40*12-1, 0, 16*16-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_tim100);

	I8276(config, m_crtc, XTAL(4'915'200));
	m_crtc->set_character_width(12);
	m_crtc->set_display_callback(FUNC(tim100_state::crtc_display_pixels), this);
	m_crtc->drq_wr_callback().set(FUNC(tim100_state::drq_w));
	m_crtc->irq_wr_callback().set(FUNC(tim100_state::irq_w));
	m_crtc->set_screen("screen");

	PALETTE(config, m_palette).set_entries(3);

	i8251_device &uart_u17(I8251(config, "uart_u17", 0));
	uart_u17.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart_u17.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart_u17.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.rxd_handler().set("uart_u17", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("uart_u17", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("uart_u17", FUNC(i8251_device::write_cts));
	rs232.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(tim100));

	i8251_device &uart_u18(I8251(config, "uart_u18", 0));
	uart_u18.txd_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	uart_u18.dtr_handler().set("rs232a", FUNC(rs232_port_device::write_dtr));
	uart_u18.rts_handler().set("rs232a", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal")); //"keyboard"));
	rs232a.rxd_handler().set("uart_u18", FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set("uart_u18", FUNC(i8251_device::write_dsr));
	rs232a.cts_handler().set("uart_u18", FUNC(i8251_device::write_cts));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(tim100));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 153'600));
	uart_clock.signal_handler().set("uart_u17", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart_u17", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart_u18", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart_u18", FUNC(i8251_device::write_rxc));
}

/* ROM definition */
ROM_START( tim100 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "tim 100 v.3.2.0.u16",   0x0000, 0x2000, CRC(4de9c8ad) SHA1(b0914d6e8d618e92a87b4b39c35391541251e8cc))

	// The first and 2nd halves of these roms are identical, confirmed ok
	ROM_REGION( 0x2000, "chargen", ROMREGION_INVERT )
	ROM_SYSTEM_BIOS( 0, "212", "v 2.1.2" )
	ROMX_LOAD( "tim 100kg v.2.1.2.u12", 0x0000, 0x0800, CRC(faf5743c) SHA1(310b662e9535878210f8aaab3e2b846fade60642), ROM_BIOS(0))
	ROM_CONTINUE (0x1000, 0x0800)
	ROM_CONTINUE (0x0800, 0x0800)
	ROM_CONTINUE (0x1800, 0x0800)
	ROM_SYSTEM_BIOS( 1, "220", "v 2.2.0" )
	ROMX_LOAD( "tim 100kg v.2.2.0.u12", 0x0000, 0x0800, CRC(358dbbd3) SHA1(14b7d6ee41b19bedf2f070f5b28b03aaff2cac4f), ROM_BIOS(1))
	ROM_CONTINUE (0x1000, 0x0800)
	ROM_CONTINUE (0x0800, 0x0800)
	ROM_CONTINUE (0x1800, 0x0800)
ROM_END

/* Driver */
COMP( 1985, tim100, 0, 0, tim100, tim100, tim100_state, empty_init, "Mihajlo Pupin Institute", "TIM-100", MACHINE_IS_SKELETON)
