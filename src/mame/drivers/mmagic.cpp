// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    "Monkey Magic" ?? 1979 Nintendo


    Dumping info provided by Andrew Welburn:

    TZF-MP  - Main Board
    TZF-SOU - Sound Board

    #   device  Label   PCB     filename
    -------------------------------------------
    4   i2708   1AI*    2A      1AI.2A
    5   i2708   2AI*    3A      2AI.3A
    6   i2708   3AI*    4A      3AI.4A
    7   i2708   4AI*    4/5A    4AI.45A
    8   i2708   5AI*    5A      5AI.5A

    22  H7641   6H      6HI     6H.6HI
    23  ?? **   7H      7HI     7H.7HI
    24  H7641   6J      6JK     6J.6JK
    25  H7641   6H***   7JK

    * Note that there is a Kana character 'I' in romaji on the end of the labels, not an I.

    ** Note this device was plastic and not ceramic, but it was dumped as a Harris 7641 as
    it is logical that its compatible with the 7641. I can see the other devices all have
    similar/same Harris markings in the bottom left of the IC obscured by the labels.

    *** Note that the label for the 7643 PROM at IC25 was almost scraped off, but by its position
    in the sequence, it has to be 6H. I removed a little more of the label in order to
    work out what the inking was on it below, turned out to be 'D-2'. the prom at IC22 also
    looks like it has an inked number under the paper label, just peeking through on one side.
    Without removing the paper labels entirely, these markings wont be fully known, but were
    covered for some reason.

    SPECS:

    - CPU is an NEC D8085A
    - Crystal is marked 6.1440, but this looks to have been replaced.
    - X1/X2 clock frequency measured at pins 1 + 2 is 6.14330 mhz
    - Test point with stable readings is :
    - TP4 (HS) = 15.9982 khz (Horizontal sync)
    - TP5 (VS) = 60.5992 hz  (Vertical Sync)

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/samples.h"
#include "speaker.h"
#include "screen.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define LOG_AUDIO 1


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mmagic_state : public driver_device
{
public:
	mmagic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_samples(*this,"samples"),
		m_vram(*this, "vram"),
		m_tiles(*this, "tiles"),
		m_colors(*this, "colors"),
		m_ball_x(0x00),
		m_ball_y(0x00),
		m_color(0x00),
		m_audio(0x00)
	{}

	DECLARE_READ8_MEMBER(vblank_r);
	DECLARE_WRITE8_MEMBER(ball_x_w);
	DECLARE_WRITE8_MEMBER(ball_y_w);
	DECLARE_WRITE8_MEMBER(color_w);
	DECLARE_WRITE8_MEMBER(audio_w);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mmagic(machine_config &config);
	void mmagic_io(address_map &map);
	void mmagic_mem(address_map &map);
protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<samples_device> m_samples;
	required_shared_ptr<uint8_t> m_vram;
	required_region_ptr<uint8_t> m_tiles;
	required_region_ptr<uint8_t> m_colors;

	uint8_t m_ball_x;
	uint8_t m_ball_y;
	uint8_t m_color;
	uint8_t m_audio;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

ADDRESS_MAP_START(mmagic_state::mmagic_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0x2000, 0x21ff) AM_RAM
	AM_RANGE(0x3000, 0x31ff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x8002, 0x8002) AM_WRITE(ball_x_w)
	AM_RANGE(0x8003, 0x8003) AM_WRITE(ball_y_w)
	AM_RANGE(0x8004, 0x8004) AM_READ(vblank_r)
ADDRESS_MAP_END

ADDRESS_MAP_START(mmagic_state::mmagic_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_WRITE(color_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(audio_w)
	AM_RANGE(0x85, 0x85) AM_READ_PORT("paddle")
	AM_RANGE(0x86, 0x86) AM_READ_PORT("buttons")
	AM_RANGE(0x87, 0x87) AM_READ_PORT("dipswitch")
ADDRESS_MAP_END


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( mmagic )
	PORT_START("dipswitch")
	PORT_SERVICE_DIPLOC(0x01, IP_ACTIVE_LOW, "DSW:1")
	PORT_DIPNAME(0x06, 0x06, DEF_STR(Bonus_Life)) PORT_DIPLOCATION ("DSW:2,3")
	PORT_DIPSETTING(0x00, "30000")
	PORT_DIPSETTING(0x02, "20000")
	PORT_DIPSETTING(0x04, "15000")
	PORT_DIPSETTING(0x06, "10000")
	PORT_DIPNAME(0x18, 0x18, DEF_STR(Lives)) PORT_DIPLOCATION ("DSW:4,5")
	PORT_DIPSETTING(0x00, "6")
	PORT_DIPSETTING(0x08, "5")
	PORT_DIPSETTING(0x10, "4")
	PORT_DIPSETTING(0x18, "3")
	PORT_DIPUNUSED_DIPLOC(0x20, IP_ACTIVE_LOW, "DSW:6" )
	PORT_DIPUNUSED_DIPLOC(0x40, IP_ACTIVE_LOW, "DSW:7" )
	PORT_DIPUNUSED_DIPLOC(0x80, IP_ACTIVE_LOW, "DSW:8" )

	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Debug?") // checked once at startup

	PORT_START("paddle")
	PORT_BIT(0xff, 0x80, IPT_PADDLE) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_CENTERDELTA(0)
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

READ8_MEMBER( mmagic_state::vblank_r )
{
	uint8_t data = 0;

	// bit 0 = vblank
	data |= m_screen->vblank() << 0;

	// other bits unused
	data |= 0xfe;

	return data;
}

WRITE8_MEMBER( mmagic_state::ball_x_w )
{
	m_ball_x = data;
}

WRITE8_MEMBER( mmagic_state::ball_y_w )
{
	m_ball_y = data;
}

WRITE8_MEMBER( mmagic_state::color_w )
{
	// bit 3 is always set
	// bit 6 switches the palette (actually there is only a single differently colored tile)
	// other bits are always 0
	m_color = data;
}

uint32_t mmagic_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// draw playfield
	for (int y = 0; y < 192 / 12; y++)
	{
		for (int x = 0; x < 256 / 8; x++)
		{
			uint8_t code = m_vram[(y * 32) + x] & 0x7f;

			// normal palette 00..7f, alternate palette 80..ff
			uint8_t color = m_colors[code | (BIT(m_color, 6) << 7)];

			// draw one tile
			for (int tx = 0; tx < 12; tx++)
			{
				uint8_t gfx = m_tiles[(code << 4) + tx];

				bitmap.pix32(y * 12 + tx, x * 8 + 0) = BIT(gfx, 4) ? rgb_t::black() : m_palette->pen_color(color);
				bitmap.pix32(y * 12 + tx, x * 8 + 1) = BIT(gfx, 5) ? rgb_t::black() : m_palette->pen_color(color);
				bitmap.pix32(y * 12 + tx, x * 8 + 2) = BIT(gfx, 6) ? rgb_t::black() : m_palette->pen_color(color);
				bitmap.pix32(y * 12 + tx, x * 8 + 3) = BIT(gfx, 7) ? rgb_t::black() : m_palette->pen_color(color);

				bitmap.pix32(y * 12 + tx, x * 8 + 4) = BIT(gfx, 0) ? rgb_t::black() : m_palette->pen_color(color);
				bitmap.pix32(y * 12 + tx, x * 8 + 5) = BIT(gfx, 1) ? rgb_t::black() : m_palette->pen_color(color);
				bitmap.pix32(y * 12 + tx, x * 8 + 6) = BIT(gfx, 2) ? rgb_t::black() : m_palette->pen_color(color);
				bitmap.pix32(y * 12 + tx, x * 8 + 7) = BIT(gfx, 3) ? rgb_t::black() : m_palette->pen_color(color);
			}
		}
	}

	// draw ball (if not disabled)
	if (m_ball_x != 0xff)
	{
		static const int BALL_SIZE = 4;
		int ball_y = (m_ball_y >> 4) * 12 + (m_ball_y & 0x0f);
		bitmap.plot_box(m_ball_x - BALL_SIZE + 1, ball_y - BALL_SIZE + 1, BALL_SIZE, BALL_SIZE, rgb_t::white());
	}

	return 0;
}


//**************************************************************************
//  AUDIO EMULATION
//**************************************************************************

static const char *const mmagic_sample_names[] =
{
	"*mmagic",
	"4",
	"3",
	"5",
	"2",
	"2-2",
	"6",
	"6-2",
	"1",
	nullptr
};

WRITE8_MEMBER( mmagic_state::audio_w )
{
	if (LOG_AUDIO)
		logerror("audio_w: %02x\n", data);

	data ^= 0xff;
	if (data != m_audio)
	{
		if (BIT(data, 7))
			m_samples->start(0, m_audio & 7);

		m_audio = data;
	}
}


//**************************************************************************
//  DRIVER INIT
//**************************************************************************

void mmagic_state::machine_start()
{
	// register for save states
	save_item(NAME(m_ball_x));
	save_item(NAME(m_ball_y));
	save_item(NAME(m_color));
	save_item(NAME(m_audio));
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

MACHINE_CONFIG_START(mmagic_state::mmagic)
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I8085A, 6.144_MHz_XTAL) // NEC D8085A
	MCFG_CPU_PROGRAM_MAP(mmagic_mem)
	MCFG_CPU_IO_MAP(mmagic_io)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(6.144_MHz_XTAL, 384, 0, 256, 264, 0, 192)
	MCFG_SCREEN_UPDATE_DRIVER(mmagic_state, screen_update)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(mmagic_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
	// TODO: replace samples with SN76477 + discrete sound
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( mmagic )
	ROM_REGION(0x1800, "maincpu", 0)
	ROM_LOAD("1ai.2a",  0x0000, 0x0400, CRC(ec772e2e) SHA1(7efc1bbb24b2ed73c518aea1c4ef4b9a93034e31))
	ROM_LOAD("2ai.3a",  0x0400, 0x0400, CRC(e5d482ca) SHA1(208b808e9208bb6f5f5f89ffbeb5a885be33733a))
	ROM_LOAD("3ai.4a",  0x0800, 0x0400, CRC(e8d38deb) SHA1(d7384234fb47e4b1d0421f58571fa748662b05f5))
	ROM_LOAD("4ai.45a", 0x0c00, 0x0400, CRC(3048bd6c) SHA1(740051589f6ba44b2ee68edf76a3177bb973d78e))
	ROM_LOAD("5ai.5a",  0x1000, 0x0400, CRC(2cab8f04) SHA1(203a3c005f18f968cd14c972bbb9fd7e0fc3b670))
	// location 6a is unpopulated, if the "debug" switch is activated on bootup it would jump here

	ROM_REGION(0x800, "tiles", 0)
	ROM_LOAD("6h.6hi", 0x000, 0x200, CRC(b6321b6f) SHA1(06611f7419d2982e006a3e81b79677e59e194f38))
	ROM_LOAD("7h.7hi", 0x200, 0x200, CRC(9ec0e82c) SHA1(29983f690a1b6134bb1983921f42c14898788095))
	ROM_LOAD("6j.6jk", 0x400, 0x200, CRC(7ce83302) SHA1(1870610ff07ab11622e183e04e3fce29328ff291))

	ROM_REGION(0x200, "colors", ROMREGION_INVERT)
	ROM_LOAD("7j.7jk", 0x000, 0x200, CRC(b7eb8e1c) SHA1(b65a8efb88668dcf1c1d00e31a9b15a67c2972c8))
ROM_END


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  MACHINE INPUT   CLASS         INIT  ROT     COMPANY     FULLNAME        FLAGS
GAME( 1979, mmagic, 0,      mmagic, mmagic, mmagic_state, 0,    ROT270, "Nintendo", "Monkey Magic", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
