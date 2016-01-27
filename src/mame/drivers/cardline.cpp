// license:LGPL-2.1+
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

#include "cardline.lh"

#define MASTER_CLOCK XTAL_12MHz

class cardline_state : public driver_device
{
public:
	cardline_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	UINT8 m_video;
	UINT8 m_hsync_q;

	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(attr_w);
	DECLARE_WRITE8_MEMBER(video_w);
	DECLARE_READ8_MEMBER(hsync_r);
	DECLARE_WRITE8_MEMBER(lamps_w);

	DECLARE_READ8_MEMBER(asic_r);
	DECLARE_WRITE8_MEMBER(asic_w);
	DECLARE_WRITE8_MEMBER(a3003_w);

	DECLARE_PALETTE_INIT(cardline);

	virtual void machine_start() override;

	DECLARE_WRITE_LINE_MEMBER(hsync_changed);
	DECLARE_WRITE_LINE_MEMBER(vsync_changed);
	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

void cardline_state::machine_start()
{
	m_video = 0;
	m_hsync_q = 1;
	for (int i=0; i < 0x2000; i++)
		m_maincpu.target()->space(AS_IO).write_byte(i, 0x73);
	save_item(NAME(m_video));
	save_item(NAME(m_hsync_q));
}


MC6845_BEGIN_UPDATE( cardline_state::crtc_begin_update )
{
}


MC6845_UPDATE_ROW( cardline_state::crtc_update_row )
{
	UINT8 *gfx;
	UINT16 x = 0;
	int gfx_ofs;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	gfx_ofs = 0;

	// bits 0 and 1 seem to be chip select lines. None of those selected
	// most likely would put the output lines into a floating (threestate)
	// state. The next statement doesn't add functioality but documents
	// how this works.

	if(m_video & 1)
		gfx_ofs = 0;

	if(m_video & 2)
		gfx_ofs = 0x1000;

	gfx = memregion("gfx1")->base();

	for (UINT8 cx = 0; cx < x_count; cx++)
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
				bitmap.pix32(y, x) = palette[bg_pal_ofs + bg_col];
			else
				bitmap.pix32(y, x) = palette[fg_pal_ofs + fg_col];

			x++;
		}
		ma++;
	}
}


WRITE_LINE_MEMBER(cardline_state::hsync_changed)
{
	/* update any video up to the current scanline */
	m_hsync_q = (state ? 0x00 : 0x10);
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
}

WRITE_LINE_MEMBER(cardline_state::vsync_changed)
{
	//m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(cardline_state::a3003_w)
{
	/* seems a generate a signal when address is written to */
}

WRITE8_MEMBER(cardline_state::vram_w)
{
	offset+=0x1000*((m_video&2)>>1);
	m_videoram[offset]=data;
}

READ8_MEMBER(cardline_state::asic_r)
{
	static int t=0;
	//printf("asic read %02x\n", offset);
	// change "if (0)" to "if (1)" to get "ASIC is ERROR!" message
	if (0)
		return t++;
	else
		return 0xaa;
}

WRITE8_MEMBER(cardline_state::asic_w)
{
	//printf("asic write %02x %02x\n", offset, data);
}


WRITE8_MEMBER(cardline_state::attr_w)
{
	offset+=0x1000*((m_video&2)>>1);
	m_colorram[offset]=data;
}

WRITE8_MEMBER(cardline_state::video_w)
{
	m_video=data;
	//printf("m_video %x\n", m_video);
}

READ8_MEMBER(cardline_state::hsync_r)
{
	return m_hsync_q;
}

WRITE8_MEMBER(cardline_state::lamps_w)
{
	/* button lamps 1-8 (collect, card 1-5, bet, start) */
	output().set_lamp_value(5,(data >> 0) & 1);
	output().set_lamp_value(0,(data >> 1) & 1);
	output().set_lamp_value(1,(data >> 2) & 1);
	output().set_lamp_value(2,(data >> 3) & 1);
	output().set_lamp_value(3,(data >> 4) & 1);
	output().set_lamp_value(4,(data >> 5) & 1);
	output().set_lamp_value(6,(data >> 6) & 1);
	output().set_lamp_value(7,(data >> 7) & 1);
}

static ADDRESS_MAP_START( mem_prg, AS_PROGRAM, 8, cardline_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mem_io, AS_IO, 8, cardline_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2003, 0x2003) AM_READ_PORT("IN0")
	AM_RANGE(0x2005, 0x2005) AM_READ_PORT("IN1")
	AM_RANGE(0x2006, 0x2006) AM_READ_PORT("DSW")
	AM_RANGE(0x2007, 0x2007) AM_WRITE(lamps_w)
	AM_RANGE(0x2008, 0x2008) AM_NOP // set to 1 during coin input
	//AM_RANGE(0x2080, 0x213f) AM_NOP // ????
	AM_RANGE(0x2100, 0x213f) AM_READWRITE(asic_r, asic_w)
	AM_RANGE(0x2400, 0x2400) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x2800, 0x2800) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x2801, 0x2801) AM_DEVWRITE("crtc", mc6845_device, register_w)
	//AM_RANGE(0x2840, 0x2840) AM_NOP // ???
	//AM_RANGE(0x2880, 0x2880) AM_NOP // ???
	AM_RANGE(0x3003, 0x3003) AM_WRITE(a3003_w)
	AM_RANGE(0xc000, 0xdfff) AM_WRITE(vram_w) AM_SHARE("videoram")
	AM_RANGE(0xe000, 0xffff) AM_WRITE(attr_w) AM_SHARE("colorram")
	/* Ports */
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READWRITE(hsync_r, video_w)
ADDRESS_MAP_END


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
	PORT_BIT( 0xf5, IP_ACTIVE_HIGH, IPT_SPECIAL ) // h/w status bits

	PORT_START("VBLANK")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // VBLANK_Q
	PORT_BIT( 0xef, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7  },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*8*8
};

static GFXDECODE_START( cardline )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 2 )
GFXDECODE_END

PALETTE_INIT_MEMBER(cardline_state, cardline)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i,r,g,b,data;
	int bit0,bit1,bit2;
	for (i = 0;i < palette.entries();i++)
	{
		data=color_prom[i];

		/* red component */
		bit0 = (data >> 5) & 0x01;
		bit1 = (data >> 6) & 0x01;
		bit2 = (data >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (data >> 2) & 0x01;
		bit1 = (data >> 3) & 0x01;
		bit2 = (data >> 4) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (data >> 0) & 0x01;
		bit1 = (data >> 1) & 0x01;
		b = 0x55 * bit0 + 0xaa * bit1;
		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}

static MACHINE_CONFIG_START( cardline, cardline_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80C32, MASTER_CLOCK)
	MCFG_MCS51_PORT1_CONFIG(0x10)
	MCFG_CPU_PROGRAM_MAP(mem_prg)
	MCFG_CPU_IO_MAP(mem_io)
	//MCFG_CPU_VBLANK_INT_DRIVER("screen", cardline_state,  irq1_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 35*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	//MCFG_SCREEN_UPDATE_DRIVER(cardline_state, screen_update_cardline)
	//MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cardline)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INIT_OWNER(cardline_state, cardline)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", MASTER_CLOCK/8)   /* divisor guessed - result is 56 Hz */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_BEGIN_UPDATE_CB(cardline_state, crtc_begin_update)
	MCFG_MC6845_UPDATE_ROW_CB(cardline_state, crtc_update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(cardline_state, hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(cardline_state, vsync_changed))

	MCFG_DEFAULT_LAYOUT(layout_cardline)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

MACHINE_CONFIG_END

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

GAME( 199?, cardline,  0,       cardline,  cardline, driver_device,  0, ROT0, "Veltmeijer", "Card Line" , MACHINE_SUPPORTS_SAVE)
