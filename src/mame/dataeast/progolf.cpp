// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Zandona'
/**************************************************************************************************

18 Holes Pro Golf (c) 1981 Data East

driver by Angelo Salese and Roberto Zandona',
based on early work by Pierpaolo Prazzoli and David Haywood

TODO:
- 6845 resolution/refresh rate calculation is incorrect (256x256),
  is it actually trying to fit 256 columns into 240?
- Flip screen support;

===================================================================================================

18 Holes Pro Golf (PCB version)
Data East 1981
Hardware info by Guru


Top Board (Sound/Inputs)
------------------------

GGM-Ø2  DE-0087C-0   MADE IN JAPAN
DE-0087B-0 (top side)
DE-0087A-0 (bottom side)
  |---------------------------------------|
  |             G6-M.1B                   |
 |-|                            6502      |
 | |  G4-M.2A   MK4118                    |
 | |                            AY3-8910  |
 | |  G3-M.4A                             |
 | |                            AY3-8910  |
 | |  G2-M.6A                             |
 |-|            DSW1                      |
  |   G1-M.8A                             |
  |                                       |
  |   G0-M.9A   DSW2                      |
|-|                                       |
|1 A18                                    |
|8                                        |
|W                                        |
|A                       555      555     |
|Y                                        |
|-|A1      TA75558                        |
  |  C1181H  VOL                          |
  |---------------------------------------|
Notes:
      6502 Clock - 441.458333kHz [10.595/24]
  AY3-8910 Clock - 1.324375MHz [10.595/8] (both)
          MK4118 - MOSTEK MK4118 1kx8 SRAM (also seen 6116 and M58725 on other
                   PCBs which are both 2kx8 SRAM)
          DSW1/2 - 8-position DIP Switch
         TA75558 - Toshiba TA75558 Dual Operational Amplifier
          C1181H - NEC uPC1181H Power AMP
             555 - 555 Timer
             VOL - 1k Volume Pot
              G* - 2732 4kx8-bit EPROM

        TOP BOARD PINOUT
    -------------------------
    Parts Side    Solder Side
    ------+---------+--------
      GND | A1   B1 | GND
      GND | A2   B2 | GND
      GND | A3   B3 | GND
     SPKR+| A4   B4 | SPKR-
     -----+---------+----- (KEY)
 RIGHT P1 | A5   B5 | RIGHT P2
  LEFT P1 | A6   B6 | LEFT P2
    UP P1 | A7   B7 | UP P2
  DOWN P1 | A8   B8 | DOWN P2
BUTTON P1 | A9   B9 | BUTTON P2
       NC | A10  B10| NC
 START P1 | A11  B11| START P2
       NC | A12  B12| NC
    COIN 1| A13  B13| COIN 2
       NC | A14  B14| NC
       NC | A15  B15| NC
      GND | A16  B16| GND
      +5V | A17  B17| +5V
      +12V| A18  B18| +12V
   -------+---------+-------


Bottom Board (CPU/Video)
------------------------

GGM-Ø1  DE-0086   MADE IN JAPAN
DE-0086B-0 (top side)
DE-0086A-0 (bottom side)
  |-------------------------------------------------------------------|
  |                                                                   |
 |-| |----------|                                                     |
 | | | DECO     |                                                     |
 | | | CPU-6    | 6845P                                               |
 | | | CUSTOM   |                                                     |
 | | | MODULE   |                           M58725              GBM.4K|
 | | |----------|                                                     |
 |-|     G7-M.7A                            M58725                    |
  |                                                                   |
  |      G8-M.9A                                                      |
  |                                                                   |
|-|      G9-M.10A  555                                                |
|1 A10                                                         GAM.11K|
|0                       8116   8116   8116   8116                    |
|W                       8116   8116   8116   8116                    |
|A                       8116   8116   8116   8116                    |
|Y       GCM.14A         8116   8116   8116   8116                    |
|-|A1                    8116   8116   8116   8116                    |
  |                      8116   8116   8116   8116          10.595MHz |
  |-------------------------------------------------------------------|
Notes:
    MC6845P - Motorola MC6845P CRT Controller (CRTC). Clock 662.1875kHz [10.595/16]
      CPU-6 - The input base clock on several pins is 1.324375MHz [10.595/8].
              There are no other different clocks going into the CPU module.
              Note the logic inside the CPU-6 module may be dividing the clock so the
              actual CPU clock is unknown but a 6502 running at 1.32437MHz is highly likely.
       8116 - Fujitsu MB8116E 16kx1 DRAM (24 chips = 48kB)
     M58725 - Mitsubishi M58725 2kx8 SRAM (compatible with 6116)
      Gx-M* - 2732 4kx8-bit EPROM
GCM/GBM/GAM - Harris M3 7603-5 Bipolar PROM (compatible with 82S123)
        555 - 555 Timer
      HSync - 15.45kHz
      VSync - 57Hz

    BOTTOM BOARD PINOUT
 -------------------------
 Parts Side    Solder Side
 ------+---------+--------
   +5V | A1   B1 | +5V
   +5V | A2   B2 | +5V
   +5V | A3   B3 | +5V
   +12V| A4   B4 | +12V
   -5V | A5   B5 | -5V
   RED | A6   B6 | GREEN
   BLUE| A7   B7 | SYNC
   GND | A8   B8 | GND
  -----+---------+----- (KEY)
   GND | A9   B9 | GND
   GND | A10  B10| GND
-------+---------+-------
**************************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "deco222.h"
#include "decocpu6.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class progolf_state : public driver_device
{
public:
	progolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_video_view0(*this, "video_view0")
		, m_video_view1(*this, "video_view1")
	{ }

	void progolfa(machine_config &config);
	void progolf(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	memory_view m_video_view0;
	memory_view m_video_view1;

	uint8_t m_plane_select = 0;
	std::unique_ptr<uint8_t[]> m_fbram{};
	uint8_t m_scrollx_hi = 0;
	uint8_t m_scrollx_lo = 0;

	u8 charram_r(offs_t offset);
	void charram_w(offs_t offset, uint8_t data);
	void video_bank_w(uint8_t data);
	void scrollx_lo_w(uint8_t data);
	void scrollx_hi_w(uint8_t data);
	void flip_screen_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);

	void palette_init(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void audio_map(address_map &map) ATTR_COLD;
};

void progolf_state::palette_init(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void progolf_state::video_start()
{
	m_scrollx_hi = 0;
	m_scrollx_lo = 0;

	m_fbram = std::make_unique<uint8_t[]>(0x2000 * 3);

	save_item(NAME(m_plane_select));
	save_pointer(NAME(m_fbram), 0x2000 * 3);
	save_item(NAME(m_scrollx_hi));
	save_item(NAME(m_scrollx_lo));
}


uint32_t progolf_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	{
		int const scroll = (m_scrollx_lo | ((m_scrollx_hi & 0x03) << 8));

		int count = 0;

		// TODO: rewrite using standard tilemap
		for (int x = 0; x < 128; x++)
		{
			for (int y = 0; y < 32; y++)
			{
				int const tile = m_videoram[count];

				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, tile, 1, 0, 0, (256 - x * 8) + scroll, y * 8);
				/* wrap-around */
				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, tile, 1, 0, 0, (256 - x * 8) + scroll -1024, y * 8);

				count++;
			}
		}
	}

	// framebuffer is 8x8 chars arranged like a bitmap
	{
		const int pitch = 32;
		for (int y = 0; y < 256; y+= 8)
		{
			for (int x = 0; x < 256; x+= 8)
			{
				const u32 fb_offset = ((y >> 3) + (x >> 3) * pitch) << 3;

				for (int xi = 0; xi < 8; xi ++)
				{
					for (int yi = 0; yi < 8; yi ++)
					{
						const int res_x = 256 - x + xi;
						const int res_y = y + yi;
						if (!cliprect.contains(res_x, res_y))
							continue;

						const u8 pen =
								(BIT(m_fbram[fb_offset + (yi | 0x0000)], 7 - xi) << 0) |
								(BIT(m_fbram[fb_offset + (yi | 0x2000)], 7 - xi) << 1) |
								(BIT(m_fbram[fb_offset + (yi | 0x4000)], 7 - xi) << 2);

						if (pen)
							bitmap.pix(res_y, res_x) = m_palette->pen(pen);
					}
				}
			}
		}
	}

	return 0;
}

// Japanese RMW style framebuffer, where individual planes are joined together
// to form bitplanes thru rudimentary OPs
u8 progolf_state::charram_r(offs_t offset)
{
	u8 res = 0;

	u8 plane = m_plane_select ^ 7;
	for (int i = 0; i < 3; i++)
		if (!BIT(plane, i))
			res |= m_fbram[offset | (i * 0x2000)];
	return res;
}

void progolf_state::charram_w(offs_t offset, uint8_t data)
{
	u8 plane = m_plane_select ^ 7;

	for (int i = 0; i < 3; i++)
	{
		const u16 fb_offset = offset | (i * 0x2000);
		if (m_plane_select == 0)
			m_fbram[fb_offset] = 0;
		else if (!BIT(plane, i))
			m_fbram[fb_offset] = data;
	}
}

/*
 * -xxx ---- view select reading for videoram ROM bank
 * -x00 ---- \- disabled (reading goes to videoram)
 * -S-- ---- \- selected half $000 / $800 (other half to videoram?)
 * --NN ---- \- read gfx ROM bank
 * ---- -xxx plane select for RMW
 */
void progolf_state::video_bank_w(uint8_t data)
{
	m_plane_select = data & 0x07;
	const bool bank_select = bool(BIT(data, 6));
	const u8 bank_number = (data >> 4) & 3;

	m_video_view0.select(bank_select ? 0 : bank_number);
	m_video_view1.select(bank_select ? bank_number : 0);

	if (data & 0x88)
		logerror("$9000: %02x\n", data);
}

void progolf_state::scrollx_lo_w(uint8_t data)
{
	m_scrollx_lo = data;
}

void progolf_state::scrollx_hi_w(uint8_t data)
{
	m_scrollx_hi = data;
}

void progolf_state::flip_screen_w(uint8_t data)
{
	flip_screen_set(data & 1);
	if(data & 0xfe)
		logerror("$9600: with data = %02x used\n",data);
}

void progolf_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
}

void progolf_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).ram();
	map(0x6000, 0x7fff).rw(FUNC(progolf_state::charram_r), FUNC(progolf_state::charram_w));
	map(0x8000, 0x8fff).ram().w(FUNC(progolf_state::videoram_w)).share("videoram");
	map(0x8000, 0x87ff).view(m_video_view0);
	m_video_view0[0]; // falls through to RAM read
	m_video_view0[1](0x8000, 0x87ff).rom().region("gfx1", 0x0000);
	m_video_view0[2](0x8000, 0x87ff).rom().region("gfx1", 0x1000);
	m_video_view0[3](0x8000, 0x87ff).rom().region("gfx1", 0x2000);
	map(0x8800, 0x8fff).view(m_video_view1);
	m_video_view1[0]; // falls through to RAM read
	m_video_view1[1](0x8800, 0x8fff).rom().region("gfx1", 0x0800);
	m_video_view1[2](0x8800, 0x8fff).rom().region("gfx1", 0x1800);
	m_video_view1[3](0x8800, 0x8fff).rom().region("gfx1", 0x2800);
	map(0x9000, 0x9000).portr("COINS").w(FUNC(progolf_state::video_bank_w));
	map(0x9200, 0x9200).portr("P1").w(FUNC(progolf_state::scrollx_hi_w));
	map(0x9400, 0x9400).portr("P2").w(FUNC(progolf_state::scrollx_lo_w));
	map(0x9600, 0x9600).portr("VBLANK").w(FUNC(progolf_state::flip_screen_w));
	map(0x9800, 0x9800).portr("DSW1");
	map(0x9800, 0x9800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x9801, 0x9801).w("crtc", FUNC(mc6845_device::register_w));
	map(0x9a00, 0x9a00).portr("DSW2").w("soundlatch", FUNC(generic_latch_8_device::write));
//  map(0x9e00, 0x9e00).nopw(); // strobe for NMI ack?
	map(0xb000, 0xffff).rom();
}

void progolf_state::audio_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x4000, 0x4fff).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x5000, 0x5fff).w("ay1", FUNC(ay8910_device::address_w));
	map(0x6000, 0x6fff).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x7000, 0x7fff).w("ay2", FUNC(ay8910_device::address_w));
	map(0x8000, 0x8fff).rw("soundlatch", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0xf000, 0xffff).rom();
}


INPUT_CHANGED_MEMBER(progolf_state::coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

// verified from M6502 code
static INPUT_PORTS_START( progolf )
	PORT_START("VBLANK")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("COINS")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, progolf_state, coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, progolf_state, coin_inserted, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_HIGH ) PORT_DIPLOCATION("DSW1:6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )  PORT_DIPLOCATION("DSW1:7") PORT_CHANGED_MEMBER(DEVICE_SELF, progolf_state, coin_inserted, 0) // same coinage as COIN1
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH ) PORT_DIPLOCATION("DSW1:8")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("DSW2:2,3") // table at 0xd16e (4 * 3 bytes, LSB first) - no multiple bonus lives
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x06, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW2:4") // code at 0xd188
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, "Display Strength and Position" ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Force Coinage = A 1C/3C - B 1C/8C" ) PORT_DIPLOCATION("DSW2:6") // SERVICE1 = 2C/1C
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Coin Mode" ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, "Mode 1" )
	PORT_DIPSETTING(    0x40, "Mode 2" )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH ) PORT_DIPLOCATION("DSW2:8")
INPUT_PORTS_END


static GFXDECODE_START( gfx_progolf )
	GFXDECODE_ENTRY( "gfx1", 0x0000, gfx_8x8x3_planar, 0, 8 )
GFXDECODE_END

void progolf_state::machine_reset()
{
	m_video_view0.select(0);
	m_video_view1.select(0);
}

void progolf_state::progolf(machine_config &config)
{
	// basic machine hardware
	DECO_222(config, m_maincpu, 10.595_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &progolf_state::main_map);

	M6502(config, m_audiocpu, 10.595_MHz_XTAL / 24);
	m_audiocpu->set_addrmap(AS_PROGRAM, &progolf_state::audio_map);

	config.set_perfect_quantum(m_maincpu);

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set_inputline(m_audiocpu, 0);
	soundlatch.set_separate_acknowledge(true);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(3072));
	screen.set_size(256, 256);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(progolf_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_progolf);
	PALETTE(config, m_palette, FUNC(progolf_state::palette_init), 32 * 3);

	// TODO: gives a 256x256 screen with 52 Hz as refresh rate, should be 57
	mc6845_device &crtc(MC6845(config, "crtc", 10.595_MHz_XTAL / 16));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", 10.595_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.23);
	AY8910(config, "ay2", 10.595_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.23);
}

void progolf_state::progolfa(machine_config &config)
{
	progolf(config);
	// different encrypted cpu to progolf
	DECO_CPU6(config.replace(), m_maincpu, 10.595_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &progolf_state::main_map);
}


ROM_START( progolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g4-m.2a",      0xb000, 0x1000, CRC(8f06ebc0) SHA1(c012dcaf06cbd9e49f3ae819d9cbed4df8751cec) )
	ROM_LOAD( "g3-m.4a",      0xc000, 0x1000, CRC(8101b231) SHA1(d933992c93b3cd9a052ac40ec1fa92a181b28691) )
	ROM_LOAD( "g2-m.6a",      0xd000, 0x1000, CRC(a4a0d8dc) SHA1(04db60d5cfca4834ac2cc7661f772704489cb329) )
	ROM_LOAD( "g1-m.8a",      0xe000, 0x1000, CRC(749032eb) SHA1(daa356b2c70bcd8cdd0c4df4268b6158bc8aae8e) )
	ROM_LOAD( "g0-m.9a",      0xf000, 0x1000, CRC(8f8b1e8e) SHA1(fc877a8f2b26ea48c5ba2324678d6077f3432a79) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "g6-m.1b",      0xf000, 0x1000, CRC(0c6fadf5) SHA1(9af2c2152b339cadab7aff0b0164d4431d2558bd) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "g7-m.7a",      0x0000, 0x1000, CRC(16b42975) SHA1(29268a8a660781ff0de77b3b1bfc16edff7be134) )
	ROM_LOAD( "g8-m.9a",      0x1000, 0x1000, CRC(cf3f35da) SHA1(06acc29a5e282b5a9960eabebdb1a529910286b6) )
	ROM_LOAD( "g9-m.10a",     0x2000, 0x1000, CRC(7712e248) SHA1(4e7dd12d323cf8378adb1e32a763a1799e2b4bdc) )

	ROM_REGION( 0x60, "proms", 0 )
	ROM_LOAD( "gcm.a14",      0x0000, 0x0020, CRC(8259e7db) SHA1(f98db5ebf8182eb0359fa372fa664cb6d3b09437) )
	ROM_LOAD( "gbm.k4",       0x0020, 0x0020, CRC(1ea3319f) SHA1(809af38e73fa1f30410e7d6b4504fe360ee9b091) )
	ROM_LOAD( "gam.k11",      0x0040, 0x0020, CRC(b9665de3) SHA1(4c5aba5f6589f4bce4692c0d5bb2811ab8e14aed) )
ROM_END

// top board: GGM-Ø2  DE-0087C-0
// bottom board: GGM-Ø1  DE-0086B-0
ROM_START( progolfa )
	ROM_REGION( 0x10000, "maincpu", 0 ) // custom DECO CPU-6 module
	ROM_LOAD( "g4-m.a3",      0xb000, 0x1000, CRC(015a08d9) SHA1(671d5cd708e098dbda3e495a8b4ce3393c6971da) )
	ROM_LOAD( "g3-m.a4",      0xc000, 0x1000, CRC(c1339da5) SHA1(e9728dcc5f67fbe79eea818ba48421c46d9e63e9) )
	ROM_LOAD( "g2-m.a6",      0xd000, 0x1000, CRC(fafec36e) SHA1(70880d6f9b11505d466f36c12a43361ee2639fed) )
	ROM_LOAD( "g1-m.a8",      0xe000, 0x1000, CRC(749032eb) SHA1(daa356b2c70bcd8cdd0c4df4268b6158bc8aae8e) )
	ROM_LOAD( "g0-m.a9",      0xf000, 0x1000, CRC(a03c533f) SHA1(2e0006be40e32b64b1490bd339d9fc9302eee7c4) )
	// the following single byte patch gets the ball position to be correct like in the parent. g3-m.a4 dump has been verified on multiple PCBs
	// ROM_FILL( 0xc14b, 0x01, 0xf0) // from: EB 07    sbc #$07 to: F0 07    beq $c154

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "g6-m.b1",      0xf000, 0x1000, CRC(0c6fadf5) SHA1(9af2c2152b339cadab7aff0b0164d4431d2558bd) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "g7-m.a8",      0x0000, 0x1000, CRC(16b42975) SHA1(29268a8a660781ff0de77b3b1bfc16edff7be134) )
	ROM_LOAD( "g8-m.a9",      0x1000, 0x1000, CRC(cf3f35da) SHA1(06acc29a5e282b5a9960eabebdb1a529910286b6) )
	ROM_LOAD( "g9-m.a10",     0x2000, 0x1000, CRC(7712e248) SHA1(4e7dd12d323cf8378adb1e32a763a1799e2b4bdc) )

	ROM_REGION( 0x60, "proms", 0 )
	ROM_LOAD( "gcm.a14",      0x0000, 0x0020, CRC(8259e7db) SHA1(f98db5ebf8182eb0359fa372fa664cb6d3b09437) )
	ROM_LOAD( "gbm.k4",       0x0020, 0x0020, CRC(1ea3319f) SHA1(809af38e73fa1f30410e7d6b4504fe360ee9b091) )
	ROM_LOAD( "gam.k11",      0x0040, 0x0020, CRC(b9665de3) SHA1(4c5aba5f6589f4bce4692c0d5bb2811ab8e14aed) )
ROM_END

} // anonymous namespace


// this uses DECO222 style encryption
GAME( 1981, progolf,  0,       progolf,  progolf, progolf_state, empty_init, ROT270, "Data East Corporation", "18 Holes Pro Golf (set 1)", MACHINE_IMPERFECT_TIMING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// this uses DECO CPU-6 as custom module CPU
GAME( 1981, progolfa, progolf, progolfa, progolf, progolf_state, empty_init, ROT270, "Data East Corporation", "18 Holes Pro Golf (set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
