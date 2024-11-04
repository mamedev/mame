// license:BSD-3-Clause
// copyright-holders: Phil Stroffolino
// thanks-to: Enrique Sanchez

/***************************************************************************

    Yie Ar Kung-Fu memory map (preliminary)

CPU:    Motorola 6809

Normal 6809 IRQs must be generated each video frame (60 fps).
The 6809 NMI is used for sound timing.


0000        R   VLM5030 status ???
4000         W  control port
                    bit 0 - flip screen
                    bit 1 - NMI enable
                    bit 2 - IRQ enable
                    bit 3 - coin counter A
                    bit 4 - coin counter B
4800         W  sound latch write
4900         W  copy sound latch to SN76496
4a00         W  VLM5030 control
4b00         W  VLM5030 data
4c00        R   DSW #0
4d00        R   DSW #1
4e00        R   IN #0
4e01        R   IN #1
4e02        R   IN #2
4e03        R   DSW #2
4f00         W  watchdog
5000-502f    W  sprite RAM 1 (18 sprites)
                    byte 0 - bit 0 - sprite code MSB
                             bit 6 - flip X
                             bit 7 - flip Y
                    byte 1 - Y position
5030-53ff   RW  RAM
5400-542f    W  sprite RAM 2
                    byte 0 - X position
                    byte 1 - sprite code LSB
5430-57ff   RW  RAM
5800-5fff   RW  video RAM
                    byte 0 - bit 4 - character code MSB
                             bit 6 - flip Y
                             bit 7 - flip X
                    byte 1 - character code LSB
8000-ffff   R   ROM


Additional Information
----------------------

While the arcade game itself never had them, nor are they really
mentioned in the manual, the board supports a third button for both
players.  This button enhances jump height on straight up jumps among
other different stances in combination with movements.  It is now in
the emulation, but default mapping is "n/a" and must be manually mapped
to be used.

This is a chart for the edge connector showing the location of pins for
"Button 3" for each player.  The manual shows these without a function
or not used.

(Posted by Matt Osborne @ KLOV Forums)

             Solder Side | Parts Side
_________________________|___________________________
 NC                  | A | 1 |   +12V
 Speaker             | B | 2 |   Speaker
 2P Button 2 (Punch) | C | 3 |   2P Button 1 (Kick)
 2P Left             | D | 4 |   2P Right
 1P Start            | E | 5 |   2P Start
 1P Button 1 (Kick)  | F | 6 |   2P Up
 1P Button 2 (Punch) | H | 7 |   Service
 1P Right            | J | 8 |   1P Left
 1P Up               | K | 9 |   2P Down
 Coin 1              | L | 10|   Coin 2
 1P Down             | M | 11|   Coin Counter 1
 1P Button 3 (Boost?)| N | 12|   Coin Counter 2
 Video Green         | P | 13|   Video Blue
 Video Red           | R | 14|   Video Sync
 NC                  | S | 15|   2P Button 3 (Boost?)
 GND                 | T | 16|   GND
 GND                 | U | 17|   GND
 +5V                 | V | 18|   +5V

PCB is silkscreened as:
GX407           PWB 200211A

  CPU: 68B09EP at 6D
Sound: VLM5030 at 7B
  OSC: 18.432MHz & 3.579545MHz
  DSW: SW1 at 3G 8-position
       SW2 at 3E 8-position
       SW3 at 3F 4-position

***************************************************************************/

#include "emu.h"

#include "konamipt.h"
#include "trackfld_a.h"

#include "cpu/m6809/m6809.h"
#include "machine/watchdog.h"
#include "sound/sn76496.h"
#include "sound/vlm5030.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class yiear_state : public driver_device
{
public:
	yiear_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram%u", 1U),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audio(*this, "trackfld_audio"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void yiear(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr_array<uint8_t, 2> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<trackfld_audio_device> m_audio;
	required_device<sn76489a_device> m_sn;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t m_nmi_enable = 0;
	uint8_t m_irq_enable = 0;

	uint8_t m_sn76496_latch = 0;

	void videoram_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);
	uint8_t speech_r();
	void vlm5030_control_w(uint8_t data);
	void konami_sn76496_latch_w(uint8_t data) { m_sn76496_latch = data; }
	void konami_sn76496_w(uint8_t data) { m_sn->write(m_sn76496_latch); }
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	INTERRUPT_GEN_MEMBER(nmi_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void vlm_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Yie Ar Kung-Fu has one 32x8 palette PROM, connected to the RGB output this
  way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void yiear_state::palette(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(*color_prom, 6);
		bit2 = BIT(*color_prom, 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r,g,b));
		color_prom++;
	}
}

void yiear_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void yiear_state::control_w(uint8_t data)
{
	// bit 0 flips screen
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}

	// bit 1 is NMI enable
	m_nmi_enable = data & 0x02;

	// bit 2 is IRQ enable
	m_irq_enable = data & 0x04;

	// bits 3 and 4 are coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x08);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);
}

TILE_GET_INFO_MEMBER(yiear_state::get_bg_tile_info)
{
	int const offs = tile_index * 2;
	int const attr = m_videoram[offs];
	int const code = m_videoram[offs + 1] | ((attr & 0x10) << 4);
//  int const color = (attr & 0xf0) >> 4;
	int const flags = ((attr & 0x80) ? TILE_FLIPX : 0) | ((attr & 0x40) ? TILE_FLIPY : 0);

	tileinfo.set(0, code, 0, flags);
}

void yiear_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(yiear_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void yiear_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram[0].bytes() - 2; offs >= 0; offs -= 2)
	{
		int const attr = m_spriteram[0][offs];
		int const code = m_spriteram[1][offs + 1] + 256 * (attr & 0x01);
		int const color = 0;
		int const flipx = ~attr & 0x40;
		int flipy = attr & 0x80;
		int sy = 240 - m_spriteram[0][offs + 1];
		int const sx = m_spriteram[1][offs];

		if (flip_screen())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		if (offs < 0x26)
		{
			sy++;   // fix title screen & garbage at the bottom of the screen
		}


			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

uint32_t yiear_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


uint8_t yiear_state::speech_r()
{
	if (m_vlm->bsy())
		return 1;
	else
		return 0;
}

void yiear_state::vlm5030_control_w(uint8_t data)
{
	// bit 0 is latch direction
	m_vlm->st((data >> 1) & 1);
	m_vlm->rst((data >> 2) & 1);
}

void yiear_state::vblank_irq(int state)
{
	if (state && m_irq_enable)
		m_maincpu->set_input_line(0, HOLD_LINE);
}


INTERRUPT_GEN_MEMBER(yiear_state::nmi_interrupt)
{
	if (m_nmi_enable)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void yiear_state::main_map(address_map &map)
{
	map(0x0000, 0x0000).r(FUNC(yiear_state::speech_r));
	map(0x4000, 0x4000).w(FUNC(yiear_state::control_w));
	map(0x4800, 0x4800).w(FUNC(yiear_state::konami_sn76496_latch_w));
	map(0x4900, 0x4900).w(FUNC(yiear_state::konami_sn76496_w));
	map(0x4a00, 0x4a00).w(FUNC(yiear_state::vlm5030_control_w));
	map(0x4b00, 0x4b00).w(m_vlm, FUNC(vlm5030_device::data_w));
	map(0x4c00, 0x4c00).portr("DSW2");
	map(0x4d00, 0x4d00).portr("DSW3");
	map(0x4e00, 0x4e00).portr("SYSTEM");
	map(0x4e01, 0x4e01).portr("P1");
	map(0x4e02, 0x4e02).portr("P2");
	map(0x4e03, 0x4e03).portr("DSW1");
	map(0x4f00, 0x4f00).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x502f).ram().share(m_spriteram[0]);
	map(0x5030, 0x53ff).ram();
	map(0x5400, 0x542f).ram().share(m_spriteram[1]);
	map(0x5430, 0x57ff).ram();
	map(0x5800, 0x5fff).ram().w(FUNC(yiear_state::videoram_w)).share(m_videoram);
	map(0x8000, 0xffff).rom();
}


void yiear_state::vlm_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x1fff).rom();
}


static INPUT_PORTS_START( yiear )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // WTF? PORT_CODE("NONE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL // *WTF? PORT_CODE("NONE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	// "Invalid" = both coin slots disabled

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "30000 80000" )
	PORT_DIPSETTING(    0x00, "40000 90000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW3:4" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0 },
	{ 0*8*8+0, 0*8*8+1, 0*8*8+2, 0*8*8+3, 1*8*8+0, 1*8*8+1, 1*8*8+2, 1*8*8+3,
		2*8*8+0, 2*8*8+1, 2*8*8+2, 2*8*8+3, 3*8*8+0, 3*8*8+1, 3*8*8+2, 3*8*8+3 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static GFXDECODE_START( gfx_yiear )
	GFXDECODE_ENTRY( "tiles",   0, charlayout,   16, 1 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,  0, 1 )
GFXDECODE_END



void yiear_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
}

void yiear_state::machine_reset()
{
	m_nmi_enable = 0;
}

void yiear_state::yiear(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, XTAL(18'432'000)/12);   // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &yiear_state::main_map);
	m_maincpu->set_periodic_int(FUNC(yiear_state::nmi_interrupt), attotime::from_hz(480)); // music tempo (correct frequency unknown)

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.58);   // verified on PCB
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(yiear_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(yiear_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_yiear);
	PALETTE(config, m_palette, FUNC(yiear_state::palette), 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	TRACKFLD_AUDIO(config, m_audio, 0, finder_base::DUMMY_TAG, m_vlm);

	SN76489A(config, m_sn, XTAL(18'432'000)/12).add_route(ALL_OUTPUTS, "mono", 1.0);   // verified on PCB

	VLM5030(config, m_vlm, XTAL(3'579'545));   // verified on PCB
	m_vlm->set_addrmap(0, &yiear_state::vlm_map);
	m_vlm->add_route(ALL_OUTPUTS, "mono", 1.0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( yiear )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "407_i08.10d", 0x08000, 0x4000, CRC(e2d7458b) SHA1(1b192130b5cd879ab686a21aa2b518c90edd89aa) )
	ROM_LOAD( "407_i07.8d",  0x0c000, 0x4000, CRC(7db7442e) SHA1(d604a995a5505251904447ad697fc9e7f94bf241) )

	ROM_REGION( 0x04000, "tiles", 0 )
	ROM_LOAD( "407_c01.6h", 0x00000, 0x2000, CRC(b68fd91d) SHA1(c267931d69794c292b7ebae5bc35ad842194683a) ) // was listed as g16_1
	ROM_LOAD( "407_c02.7h", 0x02000, 0x2000, CRC(d9b167c6) SHA1(a2fd10bddfa4e95e9d49892737ace146209bfa2b) ) // was listed as g15_2

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "407_d05.16h", 0x00000, 0x4000, CRC(45109b29) SHA1(0794935b490497b21b99045c90231b7bac151d42) ) // was listed as g04_5
	ROM_LOAD( "407_d06.17h", 0x04000, 0x4000, CRC(1d650790) SHA1(5f2a4983b20251c712358547a7c62c0331c6cb6f) ) // was listed as g03_6
	ROM_LOAD( "407_d03.14h", 0x08000, 0x4000, CRC(e6aa945b) SHA1(c5757d16c28f5966fd04675c0c640ef9b6b76ca5) ) // was listed as g06_3
	ROM_LOAD( "407_d04.15h", 0x0c000, 0x4000, CRC(cc187c22) SHA1(555ba18a9648681e5140b3fd84af16959ee5296d) ) // was listed as g05_4

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "407c10.1g", 0x00000, 0x0020, CRC(c283d71f) SHA1(10cd39f4e951ba6ca5610081c8c1fcd9d68b34d2) )  // Color BPROM type is TBP18S030N or compatible

	ROM_REGION( 0x2000, "vlm", 0 )
	ROM_LOAD( "407_c09.8b", 0x00000, 0x2000, CRC(f75a1539) SHA1(f139f6cb41351eb81ee47d777db03012aa5fadb1) ) // was listed as a12_9
ROM_END

ROM_START( yiear2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "407_g08.10d", 0x08000, 0x4000, CRC(49ecd9dd) SHA1(15692029351e87837cc5a251947ff315fd723aa4) ) // was listed as d12_8
	ROM_LOAD( "407_g07.8d",  0x0c000, 0x4000, CRC(bc2e1208) SHA1(a5a0c78ff4e02bd7da3eab3842dfe99956e74155) ) // was listed as d14_7

	ROM_REGION( 0x04000, "tiles", 0 )
	ROM_LOAD( "407_c01.6h", 0x00000, 0x2000, CRC(b68fd91d) SHA1(c267931d69794c292b7ebae5bc35ad842194683a) ) // was listed as g16_1
	ROM_LOAD( "407_c02.7h", 0x02000, 0x2000, CRC(d9b167c6) SHA1(a2fd10bddfa4e95e9d49892737ace146209bfa2b) ) // was listed as g15_2

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "407_d05.16h", 0x00000, 0x4000, CRC(45109b29) SHA1(0794935b490497b21b99045c90231b7bac151d42) ) // was listed as g04_5
	ROM_LOAD( "407_d06.17h", 0x04000, 0x4000, CRC(1d650790) SHA1(5f2a4983b20251c712358547a7c62c0331c6cb6f) ) // was listed as g03_6
	ROM_LOAD( "407_d03.14h", 0x08000, 0x4000, CRC(e6aa945b) SHA1(c5757d16c28f5966fd04675c0c640ef9b6b76ca5) ) // was listed as g06_3
	ROM_LOAD( "407_d04.15h", 0x0c000, 0x4000, CRC(cc187c22) SHA1(555ba18a9648681e5140b3fd84af16959ee5296d) ) // was listed as g05_4

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "407c10.1g", 0x00000, 0x0020, CRC(c283d71f) SHA1(10cd39f4e951ba6ca5610081c8c1fcd9d68b34d2) )  // Color BPROM type is TBP18S030N or compatible

	ROM_REGION( 0x2000, "vlm", 0 )
	ROM_LOAD( "407_c09.8b", 0x00000, 0x2000, CRC(f75a1539) SHA1(f139f6cb41351eb81ee47d777db03012aa5fadb1) ) // was listed as a12_9
ROM_END

} // anonymous namespace


GAME( 1985, yiear,  0,     yiear, yiear, yiear_state, empty_init, ROT0, "Konami", "Yie Ar Kung-Fu (program code I)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, yiear2, yiear, yiear, yiear, yiear_state, empty_init, ROT0, "Konami", "Yie Ar Kung-Fu (program code G)", MACHINE_SUPPORTS_SAVE )
