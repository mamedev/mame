// license:GPL2+
// copyright-holders:Felipe Sanches
/********************************************************************

   Scopus Sagitta 150 / 180
   Serial terminal

   driver by: Felipe Correa da Silva Sanches <juca@members.fsf.org>

   Thanks to Werner Moecke for the help in inspecting the circuits and
     debugging this driver.

   TODO:
   - figure out how the additional ROMs are mapped into memory
   - figure out how/if the ROMs of the two models (150 and 180) can
     be switched (is that via a dipswitch setting?) The model 180
     contains a Model 150 ROM as well.
   - emulate the printer port
   - document the keyboard layout and emulate it

*********************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/i8257.h"
#include "machine/i8212.h"
#include "video/i8275.h"
#include "emupal.h"
#include "screen.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"

class sagitta180_state : public driver_device
{
public:
	sagitta180_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_palette(*this, "palette"),
		m_crtc(*this, "crtc"),
		m_dma8257(*this, "dma"),
		m_maincpu(*this, "maincpu")
	{ }

	void sagitta180(machine_config &config);

	void init_sagitta180();

private:
	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	I8275_DRAW_CHARACTER_MEMBER(crtc_display_pixels);

	void maincpu_io_map(address_map &map);
	void maincpu_map(address_map &map);

	/* devices */
	required_device<palette_device> m_palette;
	required_device<i8275_device> m_crtc;
	required_device<i8257_device> m_dma8257;
	required_device<cpu_device> m_maincpu;

	// Character generator
	const uint8_t *m_chargen;

	virtual void machine_start() override;
	virtual void machine_reset() override;
};

void sagitta180_state::machine_start()
{
	m_palette->set_pen_color(0, rgb_t(0x00,0x00,0x00)); // black
	m_palette->set_pen_color(1, rgb_t(0x00,0xa0,0x00)); // normal
	m_palette->set_pen_color(2, rgb_t(0x00,0xff,0x00)); // highlight

	m_chargen = memregion("chargen")->base();
}


const gfx_layout sagitta180_charlayout =
{
	8, 8,             /* 8x16 characters - the last 8 lines are always blank */
	128,                /* 128 characters */
	1,              /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{0,1,2,3,4,5,6,7},
	{0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16                /* space between characters */
};

static GFXDECODE_START( gfx_sagitta180 )
	GFXDECODE_ENTRY( "chargen", 0x0000, sagitta180_charlayout, 0, 1 )
GFXDECODE_END

I8275_DRAW_CHARACTER_MEMBER(sagitta180_state::crtc_display_pixels)
{
	unsigned i;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint8_t chargen_byte = m_chargen[ (linecount & 7) | ((unsigned)charcode << 3) ];
	uint8_t pixels;

	if (lten) {
		pixels = ~0;
	} else if (vsp != 0 || (linecount & 8) != 0) {
		pixels = 0;
	} else {
		pixels = chargen_byte;
	}

	if (rvv) {
		pixels = ~pixels;
	}

	for (i = 0; i < 7; i++) {
		bitmap.pix32(y, x + i) = palette[ (pixels & (1U << (7 - i))) != 0 ];
	}
}


void sagitta180_state::machine_reset()
{
}

void sagitta180_state::maincpu_map(address_map &map)
{
	map.global_mask(0xffff);
	map(0x0000, 0x07ff).rom();
//  AM_RANGE(0x0800, 0x17ff) AM_ROM
	map(0x1800, 0xffff).ram();
}


void sagitta180_state::maincpu_io_map(address_map &map)
{
	map(0x00, 0x00).portr("DSW");
	map(0x20, 0x21).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x30, 0x31).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x40, 0x48).rw(m_dma8257, FUNC(i8257_device::read), FUNC(i8257_device::write));
}


static INPUT_PORTS_START( sagitta180 )
	/*
	  Serial settings:
	   * async
	   * 1 stop-bit
	   * baud-rate factor = 16x
	   * byte width and parity are specified by dipswitch seetings below.
	*/
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "Serial settings" ) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, "7 bits, even parity" )
	PORT_DIPSETTING(    0x01, "8 bits, parity disabled" )
	PORT_DIPSETTING(    0x02, "7 bits, odd parity" )
	//PORT_DIPSETTING(  0x03, "8 bits, parity disabled" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:1" )
INPUT_PORTS_END

WRITE_LINE_MEMBER(sagitta180_state::hrq_w)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dma8257->hlda_w(state);
}

READ8_MEMBER(sagitta180_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

MACHINE_CONFIG_START(sagitta180_state::sagitta180)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I8080, XTAL(10'000'000)) /* guessed ! */
	MCFG_DEVICE_PROGRAM_MAP(maincpu_map)
	MCFG_DEVICE_IO_MAP(maincpu_io_map)
//        MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE("intlatch", i8212_device, inta_cb)

	I8257(config, m_dma8257, XTAL(14'745'600)); /* guessed xtal */
	m_dma8257->out_iow_cb<2>().set("crtc", FUNC(i8275_device::dack_w));
	m_dma8257->out_hrq_cb().set(FUNC(sagitta180_state::hrq_w));
	m_dma8257->in_memr_cb().set(FUNC(sagitta180_state::memory_read_byte));

	i8251_device &uart(I8251(config, "uart", 0));
	uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("uart", FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set("uart", FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set("uart", FUNC(i8251_device::write_dsr));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 19218)); // 19218 / 19222 ? guesses...
	uart_clock.signal_handler().set("uart", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart", FUNC(i8251_device::write_rxc));

//  MCFG_DEVICE_ADD("intlatch", I8212, 0)
//  MCFG_I8212_MD_CALLBACK(GND) // guessed !
//  MCFG_I8212_DI_CALLBACK(READ8("picu", i8214_device, vector_r))
//  MCFG_I8212_INT_CALLBACK(INPUTLINE("maincpu", I8085_INTR_LINE)) // guessed !

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", i8275_device, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(80*5, 25*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*5-1, 0, 25*8-1)
	GFXDECODE(config, "gfxdecode", m_palette, gfx_sagitta180);

	I8275(config, m_crtc, 12480000 / 8); /* guessed xtal */
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(sagitta180_state::crtc_display_pixels), this);
	m_crtc->drq_wr_callback().set(m_dma8257, FUNC(i8257_device::dreq2_w));
	m_crtc->irq_wr_callback().set_inputline(m_maincpu, I8085_INTR_LINE);
	m_crtc->set_screen("screen");

	MCFG_PALETTE_ADD("palette", 3)
MACHINE_CONFIG_END


ROM_START( sagitta180 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sagitta180.u25",   0x0000, 0x0800, CRC(be21a153) SHA1(c1351cda839e0873e8d969e45502530bc26105fd) )
	ROM_LOAD( "180_2763_1.u40",   0x1000, 0x1000, CRC(45839a05) SHA1(875ec4db37bcacdc9c1d62ed7c0757b6e9c45b34) )
	ROM_LOAD( "180_2763_2.u46",   0x2000, 0x1000, CRC(10093151) SHA1(a474207c8f8505e41e4a3c0429fc4a308a282b26) )
	ROM_LOAD( "180_2763_0.u60",   0x3000, 0x1000, CRC(6be85799) SHA1(b1b76740b418d7bd1efd4d405b6fe770797b072a) )

	ROM_REGION( 0x1000, "chargen", 0 ) /* data copied from ibm-pc-jr driver */
	ROM_LOAD("cga.chr",  0x00000, 0x01000, BAD_DUMP CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd)) // from an unknown clone cga card (Actual IC is a 2708 that I was not able to dump yet)
ROM_END

//    YEAR    NAME       PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY   FULLNAME       FLAGS */
COMP( 1979?, sagitta180, 0,      0,      sagitta180, sagitta180, sagitta180_state, empty_init, "Scopus", "Sagitta 180", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
