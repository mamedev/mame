// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina

/************************************
 Card Line
 driver by Tomasz Slanina
 analog[at]op[dot]pl


 SIEMENS 80C32 (main cpu)
 MC6845P
 GM76C88 x3 (8K x 8 RAM)
 K-665 9546 (OKI 6295)
 STARS B2072 9629 (qfp ASIC)
 XTAL 12 MHz
 XTAL  4 MHz

 TODO:
     Really understand ASIC chip

***********************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/okim6295.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "cardline.lh"


namespace {

#define MASTER_CLOCK XTAL(12'000'000)

class cardline_state : public driver_device
{
public:
	cardline_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void cardline(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void vram_w(offs_t offset, uint8_t data);
	void attr_w(offs_t offset, uint8_t data);
	void video_w(uint8_t data);
	uint8_t hsync_r();
	void lamps_w(uint8_t data);

	uint8_t asic_r(offs_t offset);
	void asic_w(offs_t offset, uint8_t data);
	void a3003_w(uint8_t data);

	void cardline_palette(palette_device &palette) const;

	void hsync_changed(int state);
	void vsync_changed(int state);
	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);

	void mem_io(address_map &map) ATTR_COLD;
	void mem_prg(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	uint8_t m_video = 0;
	uint8_t m_hsync_q = 0;

	required_device<i80c32_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	output_finder<8> m_lamps;
};

void cardline_state::machine_start()
{
	m_lamps.resolve();
	m_video = 0;
	m_hsync_q = 1;

	for (int i = 0; i < 0x2000; i++)
		m_maincpu.target()->space(AS_IO).write_byte(i, 0x73);

	save_item(NAME(m_video));
	save_item(NAME(m_hsync_q));
}

MC6845_BEGIN_UPDATE( cardline_state::crtc_begin_update )
{
}


MC6845_UPDATE_ROW( cardline_state::crtc_update_row )
{
	uint16_t x = 0;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	int gfx_ofs = 0;

	// bits 0 and 1 seem to be chip select lines. None of those selected
	// most likely would put the output lines into a floating (threestate)
	// state. The next statement doesn't add functionality but documents
	// how this works.

	if(m_video & 1)
		gfx_ofs = 0;

	if(m_video & 2)
		gfx_ofs = 0x1000;

	uint8_t *gfx = memregion("gfx1")->base();

	for (uint8_t cx = 0; cx < x_count; cx++)
	{
		int bg_tile = (m_videoram[ma + gfx_ofs] | (m_colorram[ma + gfx_ofs]<<8)) & 0x3fff;
		int bg_pal_ofs = ((m_colorram[ma + gfx_ofs] & 0x80) ? 256 : 0);
		int fg_tile = (m_videoram[ma + gfx_ofs + 0x800] | (m_colorram[ma + gfx_ofs + 0x800]<<8)) & 0x3fff;
		int fg_pal_ofs = ((m_colorram[ma + gfx_ofs + 0x800] & 0x80) ? 256 : 0);
		for (int i = 0; i < 8; i++)
		{
			int bg_col = gfx[bg_tile * 64 + ra * 8 + i];
			int fg_col = gfx[fg_tile * 64 + ra * 8 + i];

			if (fg_col == 1)
				bitmap.pix(y, x) = palette[bg_pal_ofs + bg_col];
			else
				bitmap.pix(y, x) = palette[fg_pal_ofs + fg_col];

			x++;
		}
		ma++;
	}
}


void cardline_state::hsync_changed(int state)
{
	/* update any video up to the current scanline */
	m_hsync_q = (state ? 0x00 : 0x10);
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
}

void cardline_state::vsync_changed(int state)
{
	//m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

void cardline_state::a3003_w(uint8_t data)
{
	/* seems to generate a signal when address is written to */
}

void cardline_state::vram_w(offs_t offset, uint8_t data)
{
	offset+=0x1000*((m_video&2)>>1);
	m_videoram[offset]=data;
}

uint8_t cardline_state::asic_r(offs_t offset)
{
	static int t=0;
	//printf("asic read %02x\n", offset);
	// change "if (0)" to "if (1)" to get "ASIC is ERROR!" message
	if (0)
		return t++;
	else
		return 0xaa;
}

void cardline_state::asic_w(offs_t offset, uint8_t data)
{
	//printf("asic write %02x %02x\n", offset, data);
}


void cardline_state::attr_w(offs_t offset, uint8_t data)
{
	offset+=0x1000*((m_video&2)>>1);
	m_colorram[offset]=data;
}

void cardline_state::video_w(uint8_t data)
{
	m_video=data;
	//printf("m_video %x\n", m_video);
}

uint8_t cardline_state::hsync_r()
{
	return m_hsync_q;
}

void cardline_state::lamps_w(uint8_t data)
{
	/* button lamps 1-8 (collect, card 1-5, bet, start) */
	m_lamps[5] = BIT(data, 0);
	m_lamps[0] = BIT(data, 1);
	m_lamps[1] = BIT(data, 2);
	m_lamps[2] = BIT(data, 3);
	m_lamps[3] = BIT(data, 4);
	m_lamps[4] = BIT(data, 5);
	m_lamps[6] = BIT(data, 6);
	m_lamps[7] = BIT(data, 7);
}

void cardline_state::mem_prg(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void cardline_state::mem_io(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2003, 0x2003).portr("IN0");
	map(0x2005, 0x2005).portr("IN1");
	map(0x2006, 0x2006).portr("DSW");
	map(0x2007, 0x2007).w(FUNC(cardline_state::lamps_w));
	map(0x2008, 0x2008).noprw(); // set to 1 during coin input
	//map(0x2080, 0x213f).noprw(); // ????
	map(0x2100, 0x213f).rw(FUNC(cardline_state::asic_r), FUNC(cardline_state::asic_w));
	map(0x2400, 0x2400).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x2800, 0x2800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x2801, 0x2801).w("crtc", FUNC(mc6845_device::register_w));
	//map(0x2840, 0x2840).noprw(); // ???
	//map(0x2880, 0x2880).noprw(); // ???
	map(0x3003, 0x3003).w(FUNC(cardline_state::a3003_w));
	map(0xc000, 0xdfff).w(FUNC(cardline_state::vram_w)).share("videoram");
	map(0xe000, 0xffff).w(FUNC(cardline_state::attr_w)).share("colorram");
}


static INPUT_PORTS_START( cardline )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Card 1 / Double-Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Card 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Card 4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Card 5 / Winning Plan")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Bet")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Unknown1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bookkeeping Info") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_L) PORT_NAME("Payout 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Unknown2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Payout")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Unknown3")

	PORT_START("DSW")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf5, IP_ACTIVE_HIGH, IPT_CUSTOM ) // h/w status bits

	PORT_START("VBLANK")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank)) // VBLANK_Q
	PORT_BIT( 0xef, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static GFXDECODE_START( gfx_cardline )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x8_raw,     0, 2 )
GFXDECODE_END

void cardline_state::cardline_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int const data = color_prom[i];
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(data, 5);
		bit1 = BIT(data, 6);
		bit2 = BIT(data, 7);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(data, 2);
		bit1 = BIT(data, 3);
		bit2 = BIT(data, 4);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(data, 0);
		bit1 = BIT(data, 1);
		int const b = 0x55 * bit0 + 0xaa * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void cardline_state::cardline(machine_config &config)
{
	/* basic machine hardware */
	I80C32(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_port_forced_input(1, 0x10);
	m_maincpu->set_addrmap(AS_PROGRAM, &cardline_state::mem_prg);
	m_maincpu->set_addrmap(AS_IO, &cardline_state::mem_io);
	m_maincpu->port_in_cb<1>().set(FUNC(cardline_state::hsync_r));
	m_maincpu->port_out_cb<1>().set(FUNC(cardline_state::video_w));
	//m_maincpu->set_vblank_int("screen", FUNC(cardline_state::irq1_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 35*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	//screen.set_screen_update(FUNC(cardline_state::screen_update_cardline));
	//screen.set_palette(m_palette);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cardline);
	PALETTE(config, m_palette, FUNC(cardline_state::cardline_palette), 512);

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK/8));   /* divisor guessed - result is 56 Hz */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_begin_update_callback(FUNC(cardline_state::crtc_begin_update));
	crtc.set_update_row_callback(FUNC(cardline_state::crtc_update_row));
	crtc.out_hsync_callback().set(FUNC(cardline_state::hsync_changed));
	crtc.out_vsync_callback().set(FUNC(cardline_state::vsync_changed));

	config.set_default_layout(layout_cardline);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	okim6295_device &oki(OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cardline )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dns0401.u23",   0x0000, 0x10000, CRC(5bbaf5c1) SHA1(70972a744c5981b01a46799a7fd1b0a600489264) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "u38cll01.u38",   0x000001, 0x80000, CRC(12f62496) SHA1(b89eaf09e76c5c42588bf9c8c23190347635cc83) )
	ROM_LOAD16_BYTE( "u39cll01.u39",   0x000000, 0x80000, CRC(fcfa703e) SHA1(9230ad9df02140f3a6c38b24558548a888b23412) )

	ROM_REGION( 0x40000,  "oki", 0 ) // OKI samples
	ROM_LOAD( "3a.u3",   0x0000, 0x40000, CRC(9fa543c5) SHA1(a22396cb341ca4a3f0dd23719620a219c91e0e9d) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "82s147.u33",   0x0000, 0x0200, CRC(a3b95911) SHA1(46850ea38950cdccbc2ad91d968218ac964c0eb5) )

ROM_END

} // anonymous namespace


GAME( 199?, cardline, 0, cardline, cardline, cardline_state, empty_init, ROT0, "Veltmeijer", "Card Line" , MACHINE_SUPPORTS_SAVE)
