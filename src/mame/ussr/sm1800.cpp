// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        SM1800 (original name is CM1800 in cyrilic letters)

        10/12/2010 Skeleton driver.

        On board hardware :
            KR580VM80A central processing unit (i8080)
            KR580VG75  programmable CRT video display controller (i8275)
            KR580VV55  programmable parallel interface (i8255)
            KR580IK51  programmable serial interface/communication controller (i8251)
            KR580VV79  programmable peripheral device, keyboard and display controller (i8279)

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "video/i8275.h"
#include "emupal.h"
#include "screen.h"


namespace {

class sm1800_state : public driver_device
{
public:
	sm1800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_ppi(*this, "ppi")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
	{ }

	void sm1800(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<i8255_device> m_ppi;
	required_device<i8275_device> m_crtc;
	required_device<palette_device> m_palette;
	void portb_w(uint8_t data);
	void portc_w(uint8_t data);
	uint8_t porta_r();
	uint8_t portc_r();
	uint8_t m_irq_state = 0U;
	void machine_start() override ATTR_COLD;
	void sm1800_palette(palette_device &palette) const;
	INTERRUPT_GEN_MEMBER(vblank_interrupt);
	IRQ_CALLBACK_MEMBER(irq_callback);
	I8275_DRAW_CHARACTER_MEMBER( crtc_display_pixels );
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
};

void sm1800_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom();
	//map(0x0fb0, 0x0fff).w("crtc", FUNC(i8275_device::dack_w));
	map(0x1000, 0x17ff).ram(); // videoram looks like 1080-17FF, normal ascii
}

void sm1800_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x3c, 0x3d).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x5c, 0x5d).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x6c, 0x6f).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	//map(0x74, 0x74).rw("i8279", FUNC(i8279_device::status_r), FUNC(i8279_device::cmd_w));
	//map(0x75, 0x75).rw("i8279", FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));
}

/* Input ports */
static INPUT_PORTS_START( sm1800 )
INPUT_PORTS_END

IRQ_CALLBACK_MEMBER(sm1800_state::irq_callback)
{
	return 0xff;
}

void sm1800_state::machine_start()
{
	save_item(NAME(m_irq_state));
}

INTERRUPT_GEN_MEMBER(sm1800_state::vblank_interrupt)
{
	m_maincpu->set_input_line(0, m_irq_state ?  HOLD_LINE : CLEAR_LINE);
	m_irq_state ^= 1;
}

I8275_DRAW_CHARACTER_MEMBER( sm1800_state::crtc_display_pixels )
{
	using namespace i8275_attributes;

	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint8_t const *const charmap = memregion("chargen")->base();
	uint8_t pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if (BIT(attrcode, VSP))
		pixels = 0;

	if (BIT(attrcode, LTEN))
		pixels = 0xff;

	if (BIT(attrcode, RVV))
		pixels ^= 0xff;

	bool hlgt = BIT(attrcode, HLGT);
	for(int i=0;i<8;i++)
		bitmap.pix(y, x + i) = palette[(pixels >> (7-i)) & 1 ? (hlgt ? 2 : 1) : 0];
}

void sm1800_state::portb_w(uint8_t data)
{
}

void sm1800_state::portc_w(uint8_t data)
{
}

uint8_t sm1800_state::porta_r()
{
	return 0xff;
}

uint8_t sm1800_state::portc_r()
{
	return 0;
}

void sm1800_state::sm1800_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t::black()); // black
	palette.set_pen_color(1, 0xa0, 0xa0, 0xa0); // white
	palette.set_pen_color(2, rgb_t::white()); // highlight
}


/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_sm1800 )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END


void sm1800_state::sm1800(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, XTAL(2'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &sm1800_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sm1800_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(sm1800_state::vblank_interrupt));
	m_maincpu->set_irq_acknowledge_callback(FUNC(sm1800_state::irq_callback));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	PALETTE(config, m_palette, FUNC(sm1800_state::sm1800_palette), 3);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_sm1800);

	/* Devices */
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(sm1800_state::porta_r));
	m_ppi->out_pb_callback().set(FUNC(sm1800_state::portb_w));
	m_ppi->in_pc_callback().set(FUNC(sm1800_state::portc_r));
	m_ppi->out_pc_callback().set(FUNC(sm1800_state::portc_w));

	I8275(config, m_crtc, 2000000);
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(sm1800_state::crtc_display_pixels));

	I8251(config, m_uart, 0);
}

/* ROM definition */
ROM_START( sm1800 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "prog.bin", 0x0000, 0x0800, CRC(55736ad5) SHA1(b77f720f1b64b208dd6a5d4f9c9521d1284028e9))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "font.bin", 0x0000, 0x0800, CRC(28ed9ebc) SHA1(f561136962a06a5dcb5a0436931d29e940155d24))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   STATE         INIT        COMPANY      FULLNAME   FLAGS */
COMP( ????, sm1800, 0,      0,      sm1800,  sm1800, sm1800_state, empty_init, "<unknown>", "SM1800",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
