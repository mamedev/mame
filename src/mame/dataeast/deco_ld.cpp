// license:BSD-3-Clause
// copyright-holders:Angelo Salese, bugjosh
/***************************************************************************

Bega's Battle (c) 1983 Data East Corporation

preliminary driver by Angelo Salese

In begas, cobra and rblaster, the main CPU controls a Sony LDP-1000 or
LDP-1000A player through the 6850 ACIA, whose receive interrupt is its only
IRQ (the coins are on NMI). The player is emulated as the LDP-1000A.

TODO:
- Bega's Battle VBLANK hack;
- clean-ups;

***************************************************************************

There are three hardware versions of Cobra Command (LD):

Data East single PCB, Pioneer LD-V1000
Data East 3-boardset, same hardware as Bega's Battle & Road Blaster, Sony
LDP-1000
MACH3 conversion kit (ROMs, disc, decals), Pioneer PR-8210

There are four versions of the laserdisc.

Pioneer (08359)
Data East (Japan), LDS-301 with an orange label
Data East (USA), LDS-301A with a green label
Sony (a1090731704132a)

The Data East labelled discs were released with the DE 3-boardset version
and MACH3 conversion.
The Pioneer Labelled disc was released with the DE single PCB version.
Not sure what version the Sony disc came from. It was given to me by the
copyright owner of Road Blaster, who also gave me a Road Blaster disc/kit
which has a similar Sony label.

I peeled the Data East labels off an orange and a green labelled disc and
the labels underneath were identical to the Sony labelled disc (Sony Japan,
disc No.a1090731704132a).

Physical appearances aside, the Sony and Pioneer pressed discs have
identical content.

===========================================================================


---------------------------------
Bega's Battle by DATA EAST (1983)
---------------------------------
malcor




Location   Device    File ID    Checksum
----------------------------------------
TB 14F      2764      AN00        E929   [ main program ] [ Rev.1 ]
TB 12F      2764      AN01        7B4D   [ main program ] [ Rev.1 ]
TB 11F      2764      AN02        3390   [ main program ] [ Rev.1 ]
TB 9F       2764      AN03        A9E5   [ main program ] [ Rev.1 ]
TB 8F       2764      AN04        303E   [ main program ] [ Rev.1 ]
TB 6F       2764      AN05        3A89   [ main program ] [ Rev.1 ]

TB 14F      2764      AN00-3      E983   [ main program ] [ Rev.3 ]
TB 11F      2764      AN02-3      46DA   [ main program ] [ Rev.3 ]
TB 9F       2764      AN03-3      B99B   [ main program ] [ Rev.3 ]
TB 8F       2764      AN04-3      3A57   [ main program ] [ Rev.3 ]
TB 6F       2764      AN05-3      3A9D   [ main program ] [ Rev.3 ]

TB 15C      2764      AN06        916B   [ snd  program ]
TB 3A       2764      AN07        944B
TB 4A       2764      AN08        798F
TB 6A       2764      AN09        DF57
TB 12A      2764      AN0A        5B95
TB 14A      2764      AN0B        F2C7
TB 15A      2764      AN0C        1605
LB 2F     82S123     AF-8.bpr     00FC   [ DSP select  ]
LB 14K   PAL10L8     LP1-1.pld    6A1A
LB 7C    PAL10L8     LP1-2.pld    6A16
LB 8C    PAL12L6     LP1-3.pld    76B3
LB 11E   PAL12L6     LP1-4.pld    769F
LB 6C    PAL10L8     LP1-5.pld    6A99
LB 12C   PAL10L8     LP1-5.pld    6A99
TB 10H   PAL10L8     LP2-1.pld    6A36
TB C10   PAL10L8     LP2-4.pld    6A05


Note: TB - Top board      VDO-2 DE-0139-1
      LB - Lower board    VDO-1 DE-0138-1

           Laserdisc video game


Brief hardware overview
-----------------------

Main processor  - 6502
                - EF68B50P   communications interface
                - AM2950DC   I/O port to sound processor

Sound processor - 6502
             2x - AY-3-8910

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/gen_latch.h"
#include "machine/ldp1000.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class deco_ld_state : public driver_device
{
public:
	deco_ld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_laserdisc(*this, "laserdisc")
		, m_acia(*this, "acia")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch2(*this, "soundlatch2")
		, m_vram(*this, "vram%u", 0U)
		, m_attr(*this, "attr%u", 0U)
		, m_coin2(*this, "COIN2")
	{ }

	void rblaster(machine_config &config) ATTR_COLD;
	void cobraa(machine_config &config) ATTR_COLD;

	int begas_vblank_r();
	int coin2_r() { return m_coin2->read() ? 1 : 0; }
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<sony_ldp1000_device> m_laserdisc;
	required_device<acia6850_device> m_acia;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_shared_ptr_array<uint8_t, 2> m_vram;
	required_shared_ptr_array<uint8_t, 2> m_attr;
	required_ioport m_coin2;

	uint8_t m_sound_nmi_enable = 0;
	uint8_t m_video_control = 0;
	emu_timer *m_sound_timer = nullptr;
	tilemap_t *m_tilemap[2];

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	uint8_t sound_status_r();
	void video_control_w(uint8_t data);
	bool laserdisc_shown() const { return BIT(m_video_control, 3) && !BIT(m_video_control, 0); }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void sound_nmi_enable_w(uint8_t data);
	void nmi_ack_w(uint8_t data);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint8_t *spriteram, uint16_t tile_bank, uint32_t color);

	template <unsigned N> void vram_w(offs_t offset, u8 data);
	template <unsigned N> void attr_w(offs_t offset, u8 data);
	// the lower half of each layer's range accesses the same RAM by column (the low address bits select the row);
	// cobra draws the left and right sides of a frame and its cockpit struts through it
	template <unsigned N> u8 vram_column_r(offs_t offset) { return m_vram[N][column_offset(offset)]; }
	template <unsigned N> void vram_column_w(offs_t offset, u8 data) { vram_w<N>(column_offset(offset), data); }
	template <unsigned N> u8 attr_column_r(offs_t offset) { return m_attr[N][column_offset(offset)]; }
	template <unsigned N> void attr_column_w(offs_t offset, u8 data) { attr_w<N>(column_offset(offset), data); }
	static offs_t column_offset(offs_t offset) { return BIT(offset, 0, 5) << 5 | BIT(offset, 5, 5); }
	template <unsigned N> TILE_GET_INFO_MEMBER(get_tile_info);

	TIMER_CALLBACK_MEMBER(sound_timing);
};

template <unsigned N> void deco_ld_state::vram_w(offs_t offset, u8 data)
{
	m_vram[N][offset] = data;
	m_tilemap[N]->mark_tile_dirty(offset);
}

template <unsigned N> void deco_ld_state::attr_w(offs_t offset, u8 data)
{
	m_attr[N][offset] = data;
	m_tilemap[N]->mark_tile_dirty(offset);
}

// each layer has its own character set: layer 0 uses the second half of the graphics ROMs (which
// starts with solid blocks the games fill it with) like its sprites, layer 1 the first half (the font)
// the palette is the priority encoder's code for the layer's tiles (see screen_update): 2 for layer 0 (group A),
// 5 for layer 1 (group B)
template <unsigned N> TILE_GET_INFO_MEMBER(deco_ld_state::get_tile_info)
{
	u8 attr = m_attr[N][tile_index];
	u16 tile = m_vram[N][tile_index] | BIT(attr, 0, 2) << 8 | (N ? 0x000 : 0x400);
	tileinfo.set(0, tile, N ? 5 : 2, 0);
}


void deco_ld_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deco_ld_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deco_ld_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);

	// flush palette to all black: ends up using a very similar pattern than the MAME default one ...
	for (int i = 0; i < 0x40; i++)
		m_palette->set_pen_color(i, 0, 0, 0);
}

/*
[+0] ---- -x-- flip X
[+0] ---- --x- flip Y
[+0] ---- ---x enable this sprite
[+1] tile number
[+2] y coord
[+3] x coord
*/
void deco_ld_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint8_t *spriteram, uint16_t tile_bank, uint32_t color)
{
	gfx_element *gfx = m_gfxdecode->gfx(1);

	// the first and last rows of tile RAM
	for (int list : { 0x000, 0x3e0 })
	{
		for (int i = list; i < list + 0x20; i += 4)
		{
			if (BIT(spriteram[i], 0))
				gfx->transpen(bitmap, cliprect, spriteram[i + 1] | tile_bank, color, BIT(spriteram[i], 2), BIT(spriteram[i], 1), spriteram[i + 3], spriteram[i + 2], 0);
		}
	}
}

uint32_t deco_ld_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// with the colour output latch held clear, only the laserdisc picture (or black) is left
	if (BIT(m_video_control, 1))
	{
		bitmap.fill(rgb_t::transparent(), cliprect);
		return 0;
	}

	// with the laserdisc picture off, the board's background colour (palette 0, colour 0) shows instead
	bitmap.fill(laserdisc_shown() ? rgb_t::transparent() : rgb_t(m_palette->pen(0)), cliprect);

	// a priority encoder (LS148 at 16H on VDO-1) picks the topmost source for each pixel, and its code is also
	// the palette number: group B "MIX" (6), group B tiles (5), group B "SOFT2" (4), group A "MIX" (3),
	// group A tiles (2), group A "SOFT2" (1); the games fill palettes 1, 2 and 3 with the same colours, and
	// 4, 5 and 6 likewise
	// TODO: tiles and "MIX" come out of one line buffer on VDO-1 (93422 at 11H for group A, through PAL LP1-6)
	// and "SOFT2" out of another (7F, through PAL LP1-7); neither PAL is dumped, so it isn't known which sprite
	// list, if either, is "MIX". Both lists stay below their group's tiles: with either above them, garbage
	// shows when rblaster and cobra fill the whole tile RAM (rows 0 and 31 too) with one tile
	draw_sprites(bitmap, cliprect, m_vram[0], 0x100, 3);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, m_vram[1], 0x000, 6);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


uint8_t deco_ld_state::sound_status_r()
{
	// bit 7: the sound CPU has written a reply, bit 6: the sound CPU hasn't read the last command yet
	// (cobra's sound test waits for these and checks that the sound CPU echoes the command)
	return (m_soundlatch2->pending_r() << 7) | (m_soundlatch->pending_r() << 6) | 0x3f;
}

void deco_ld_state::video_control_w(uint8_t data)
{
	// LS175 at 11K on VDO-1, cleared at reset
	// bit 0: hide the laserdisc picture (forces the DSP SEL output low); the games set it while showing TILT
	//        and the power-on countdown, and rblaster while it sends a search to the player
	// bit 1: blank the board's own picture (holds the colour output latch, LS273 at 2J, clear) and make DSP SEL
	//        ignore the overlay, so only the laserdisc picture is left if bits 0 and 3 let it through; the games
	//        only set it with bit 0, which leaves the screen black
	// bit 2: drives *MAIG ENB low (through the latch's inverted output), which takes group A's tiles out of the
	//        priority encoder and lets them cut holes in the laserdisc picture instead
	// bit 3: EXT SYNC, sync the screen to the player (LS157 at 8K) and allow DSP SEL high; the games set it
	//        in normal operation and clear it in the service mode's RAM tests
	// TODO: bit 2 (the games only set it together with bit 1)
	if (BIT(data, 2) && !BIT(data, 1))
		logerror("%s: video control %02x\n", machine().describe_context(), data);
	m_video_control = data & 0x0f;
	m_laserdisc->video_enable(laserdisc_shown());
}

void deco_ld_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	// writes to 0x1000 latch bits 0 and 1 into the LS273 at 3H on VDO-2, which drives the coin blockers; begas and
	// cobra set them once booted and clear them at 9 credits, while rblaster's one store there (0xe484) only clears
	// them at 9 credits, so no single polarity of the blockers fits all three games and they aren't emulated
	map(0x1000, 0x1000).portr("IN0").nopw();
	map(0x1001, 0x1001).portr("DSW1").w(FUNC(deco_ld_state::nmi_ack_w));
	map(0x1002, 0x1002).portr("DSW2");
	map(0x1003, 0x1003).portr("IN1").w(FUNC(deco_ld_state::video_control_w));
	map(0x1004, 0x1004).r(m_soundlatch2, FUNC(generic_latch_8_device::read)).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x1005, 0x1005).r(FUNC(deco_ld_state::sound_status_r));
	map(0x1006, 0x1007).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x1800, 0x183f).mirror(0x7c0).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // 93419 64x9 RAM at 5J on VDO-1
	map(0x2000, 0x23ff).rw(FUNC(deco_ld_state::vram_column_r<0>), FUNC(deco_ld_state::vram_column_w<0>));
	map(0x2400, 0x27ff).rw(FUNC(deco_ld_state::attr_column_r<0>), FUNC(deco_ld_state::attr_column_w<0>));
	map(0x2800, 0x2bff).ram().w(FUNC(deco_ld_state::vram_w<0>)).share(m_vram[0]);
	map(0x2c00, 0x2fff).ram().w(FUNC(deco_ld_state::attr_w<0>)).share(m_attr[0]);
	map(0x3000, 0x33ff).rw(FUNC(deco_ld_state::vram_column_r<1>), FUNC(deco_ld_state::vram_column_w<1>));
	map(0x3400, 0x37ff).rw(FUNC(deco_ld_state::attr_column_r<1>), FUNC(deco_ld_state::attr_column_w<1>));
	map(0x3800, 0x3bff).ram().w(FUNC(deco_ld_state::vram_w<1>)).share(m_vram[1]);
	map(0x3c00, 0x3fff).ram().w(FUNC(deco_ld_state::attr_w<1>)).share(m_attr[1]);
	map(0x4000, 0xffff).rom();
}


/* sound arrangement is practically identical to Zero Target. */

void deco_ld_state::sound_nmi_enable_w(uint8_t data)
{
	// LS74 at 10D on VDO-2 latches data bit 0 (cobra keeps it clear while it initialises)
	m_sound_nmi_enable = BIT(data, 0);
}

TIMER_CALLBACK_MEMBER(deco_ld_state::sound_timing)
{
	// "SOUND TIMING" is bit 4 of the vertical counter (inverted by 11C on VDO-1), gated by the enable: eight
	// pulses per field (the screen counts every line twice; the counter's phase in the field isn't verified)
	if (m_sound_nmi_enable)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	const int next = (param + 64) % 512;
	m_sound_timer->adjust(m_screen->time_until_pos(next), next);
}


void deco_ld_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram(); // 58725 at 14C on VDO-2
	map(0x2000, 0x2000).w("ay1", FUNC(ay8910_device::data_w));
	map(0x4000, 0x4000).w("ay1", FUNC(ay8910_device::address_w));
	map(0x6000, 0x6000).w("ay2", FUNC(ay8910_device::data_w));
	map(0x8000, 0x8000).w("ay2", FUNC(ay8910_device::address_w));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w(m_soundlatch2, FUNC(generic_latch_8_device::write));
	map(0xc000, 0xc000).w(FUNC(deco_ld_state::sound_nmi_enable_w));
	// TODO: 0xb000 reads a status port (LP2-4), unused by the games
	map(0xe000, 0xffff).rom().nopw(); // rblaster writes to 0xe600 once, which doesn't select anything
}

int deco_ld_state::begas_vblank_r()
{
	return m_screen->vpos() >= 240*2;
}

INPUT_CHANGED_MEMBER(deco_ld_state::coin_inserted)
{
	// the coin counters are driven by the coin switches; a coin also sets its flip-flop (one of the two in the
	// LS74 at 8J on VDO-2), which pulls NMI until the game acknowledges it
	if (param < 2)
		machine().bookkeeping().coin_counter_w(param, newval);
	if (newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void deco_ld_state::nmi_ack_w(uint8_t data)
{
	// the NMI handlers end with a write here
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

static INPUT_PORTS_START( deco_ld )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(deco_ld_state::begas_vblank_r)) // TODO: IPT_VBLANK a.k.a. screen_device::vblank doesn't seem to work fine?

	// switches 5-8 aren't connected, the start buttons and coins are read in their place
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(deco_ld_state::coin_inserted), 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(deco_ld_state::coin2_r))

	// the service switch is wired to the second coin's input, past the coin counter
	PORT_START("COIN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(deco_ld_state::coin_inserted), 1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(deco_ld_state::coin_inserted), 2)
INPUT_PORTS_END

static INPUT_PORTS_START( begas )
	PORT_INCLUDE( deco_ld )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x10, "50000" )
	PORT_DIPSETTING(    0x00, "None (continuous fire)" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	// also selects the diagnostics: the factory ones (with the monitor adjustment patterns) for cockpit
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "Cockpit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( cobra )
	PORT_INCLUDE( deco_ld )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	// the conversion manual has lives, bonus, difficulty and region on switches 1-5; of those, this program only
	// reads difficulty in play, from switches 4 and 5, and its coinage (DSW1) follows the Bega's Battle manual
	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" ) // only read by the disc test
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	// must be off for the diagnostics
	PORT_DIPNAME( 0x40, 0x40, "Random First Mission" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( rblaster )
	PORT_INCLUDE( deco_ld )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	// the coin handler (0xe444) uses switches 3 and 4 instead of 1 and 2 when the carry is set after shifting
	// the coin bits right twice, but the coins are bits 6 and 7, so the carry is always clear and both coins
	// use switches 1 and 2
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0c, 0x0c, "SW1:3,4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "20000 only" )
	PORT_DIPSETTING(    0x02, "15000, every 30000" )
	PORT_DIPSETTING(    0x04, "20000, every 40000" )
	PORT_DIPSETTING(    0x00, "30000, every 50000" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	// skips the checks of the player's moves, and turns on the player's frame number display (INDEX ON)
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0, 16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	16*16
};

static GFXDECODE_START( gfx_rblaster )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 8 )
GFXDECODE_END

void deco_ld_state::machine_start()
{
	m_sound_timer = timer_alloc(FUNC(deco_ld_state::sound_timing), this);

	save_item(NAME(m_sound_nmi_enable));
	save_item(NAME(m_video_control));
}

void deco_ld_state::machine_reset()
{
	// reset clears the coin flip-flops (8J) and the latches at 10D on VDO-2 and 11K on VDO-1
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_sound_nmi_enable = 0;
	m_video_control = 0;
	m_laserdisc->video_enable(false);
	m_sound_timer->adjust(m_screen->time_until_pos(32), 32);
}

void deco_ld_state::rblaster(machine_config &config)
{
	/* basic machine hardware */
	// the AF-8 PROM at 2F on VDO-1, which also times DSP SEL, shapes the CPU clock from the 12 MHz clock
	M6502(config, m_maincpu, 12_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &deco_ld_state::main_map);

	// 3 MHz from VDO-1 divided by 2 by the LS161 at 8D on VDO-2 (jumpers J1 and J2)
	M6502(config, m_audiocpu, 12_MHz_XTAL / 4 / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &deco_ld_state::sound_map);

//  config.set_maximum_quantum(attotime::from_hz(6000));

	// the games talk to the player through the ACIA, whose receive interrupt is the only IRQ source
	ACIA6850(config, m_acia);
	m_acia->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);
	m_acia->txd_handler().set(m_laserdisc, FUNC(sony_ldp1000_device::rx_w));

	// 9.8304 MHz divided by 128 by the LS163s at 15D and 16D on VDO-2; the games program the ACIA for
	// divide-by-64, giving the player's factory setting of 1200 baud
	clock_device &acia_clock(CLOCK(config, "acia_clock", 9.8304_MHz_XTAL / 128));
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	SONY_LDP1000(config, m_laserdisc);
	m_laserdisc->set_baud(1200);
	m_laserdisc->serial_tx().set(m_acia, FUNC(acia6850_device::write_rxd));
	m_laserdisc->set_overlay(256, 256, FUNC(deco_ld_state::screen_update));
	// the first and last rows of tile RAM hold the sprite lists
	m_laserdisc->set_overlay_clip(0, 256-1, 8, 248-1);
	m_laserdisc->add_route(0, "speaker", 1.0, 0);
	m_laserdisc->add_route(1, "speaker", 1.0, 1);

	/* video hardware */
	m_laserdisc->add_ntsc_screen(config, "screen");
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rblaster);
	PALETTE(config, m_palette).set_format(palette_device::BGR_233_inverted, 0x40);

	/* sound hardware */
	// TODO: mixing with laserdisc
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	GENERIC_LATCH_8(config, m_soundlatch2);

	AY8910(config, "ay1", 12_MHz_XTAL / 4 / 2).add_route(ALL_OUTPUTS, "speaker", 0.25, 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 1);

	AY8910(config, "ay2", 12_MHz_XTAL / 4 / 2).add_route(ALL_OUTPUTS, "speaker", 0.25, 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 1);
}

void deco_ld_state::cobraa(machine_config &config)
{
	rblaster(config);

	// TODO: cobraa drives a Pioneer LD-V1000 and has its own PROMs and PALs (VD0-C, VD0-T, LP4-1, LP4-2): it
	// never touches the ACIA, sends LD-V1000 commands to 0x1004 with a handshake on bits 5 and 6 of 0x1001
	// (0xe50b-0xe527), and checks for LD-V1000 status codes (0xe53d-0xe5f1); its 0x1003 writes (0xff, 0x00,
	// and an inverted value at 0xf63f) don't fit the VDO-1 video latch either
	// keep the vblank IRQ it had, as its IRQ handler doesn't use the ACIA
	m_acia->irq_handler().set_nop();
	m_maincpu->set_vblank_int("screen", FUNC(deco_ld_state::irq0_line_hold));
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( begas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "an05-3",   0x4000, 0x2000, CRC(c917a283) SHA1(b91f8cd18b8cc1189e4b69d6932d6f01d4ccfb81) )
	ROM_LOAD( "an04-3",   0x6000, 0x2000, CRC(935b2b0a) SHA1(e7c09960607569bd88e9af396aa70661f4352efb) )
	ROM_LOAD( "an03-3",   0x8000, 0x2000, CRC(79438d80) SHA1(e641336f23c6b84d84313ef3e94871ac9aa8b612) )
	ROM_LOAD( "an02-3",   0xa000, 0x2000, CRC(98ce4ca0) SHA1(e7db66b1f0f06b0a21e7450962ba70f460a24847) )
	ROM_LOAD( "an01",     0xc000, 0x2000, CRC(15f8921d) SHA1(32f945bee8f30e5896da38ac6184a11c0a8194bb) )
	ROM_LOAD( "an00-3",   0xe000, 0x2000, CRC(124a3a36) SHA1(e2f7110196cb46fcda429c613388285b46ec1a9e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "an06",   0xe000, 0x2000, CRC(cbbcd730) SHA1(2f2e78fcf2eba71044bec60d27d8756d9b5af551) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "an0a",   0x0000, 0x2000, CRC(e429305d) SHA1(9a05ab7916235d028b6b05270703516581825660) )
	ROM_LOAD( "an0b",   0x4000, 0x2000, CRC(09e4b780) SHA1(0735420b8529017e507feecf8f74fecd80fbf7d5) )
	ROM_LOAD( "an0c",   0x8000, 0x2000, CRC(0c127207) SHA1(b8372b2fa20ffe5ac278f558c07fd761c86e514b) )

	ROM_LOAD( "an07",   0x2000, 0x2000, CRC(6b8ad735) SHA1(a703523202d40e409e2345a6626b9e29b7a59cd3) )
	ROM_LOAD( "an08",   0x6000, 0x2000, CRC(b5518391) SHA1(57f6407491cff075f76a8b459cc33e8b9a91e7de) )
	ROM_LOAD( "an09",   0xa000, 0x2000, CRC(b7375fd7) SHA1(93a59e99e375bdba77199a705b5e304ece221617) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "af-8.bpr",    0x00, 0x20, CRC(20006a72) SHA1(6d0e1c6de45079f9e128186478a7e0ed3fd471d0) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "lp1-1.pld",   0x0000, 44, CRC(cc84cb09) SHA1(61620ef30dfd6c81cc517f95ab6358c619ca3298) )
	ROM_LOAD( "lp1-2.pld",   0x0100, 44, CRC(60e16fc4) SHA1(1df735f393ed0fcf1272fceada9764084ff11e07) )
	ROM_LOAD( "lp1-3.pld",   0x0200, 52, CRC(976a7c57) SHA1(202c55a236799fb44a977c074c231ed54c71a872) )
	ROM_LOAD( "lp1-4.pld",   0x0300, 52, CRC(cc9a442f) SHA1(5d08873b204b15f888d02d79e049119e05e41b45) )
	ROM_LOAD( "lp1-5.pld",   0x0400, 44, CRC(2d9f3118) SHA1(02e40a99f131bb47562d5b90fdfb11ca8cd90da6) )
	ROM_LOAD( "lp2-1.pld",   0x0500, 44, CRC(dbb05313) SHA1(fc37db24f12c4f5170945c9ec9a333e4583c1712) )
	ROM_LOAD( "lp2-4.pld",   0x0600, 44, CRC(4c72736c) SHA1(6f7521284a5d960ff05c4361095c3e89a79f7475) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "begas", 0, NO_DUMP )
ROM_END

ROM_START( begas1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "an05",   0x4000, 0x2000, CRC(91a05549) SHA1(425668ee0dcf44bc011ee3649aa82df6ad3180eb) )
	ROM_LOAD( "an04",   0x6000, 0x2000, CRC(670966fe) SHA1(c179e3045ed0e46c5829fce5297ada475141e662) )
	ROM_LOAD( "an03",   0x8000, 0x2000, CRC(d2d85cdf) SHA1(da557ce5c3252297d2c073a0242e1989b0b7388b) )
	ROM_LOAD( "an02",   0xa000, 0x2000, CRC(84d13c20) SHA1(6474d90b84bca88c35cdb1d4c117ce431d6addf7) )
	ROM_LOAD( "an01",   0xc000, 0x2000, CRC(15f8921d) SHA1(32f945bee8f30e5896da38ac6184a11c0a8194bb) )
	ROM_LOAD( "an00",   0xe000, 0x2000, CRC(184297f3) SHA1(6813f076fde3eb583929506b2e65d9cd988b1b75) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "an06",   0xe000, 0x2000, CRC(cbbcd730) SHA1(2f2e78fcf2eba71044bec60d27d8756d9b5af551) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "an0a",   0x0000, 0x2000, CRC(e429305d) SHA1(9a05ab7916235d028b6b05270703516581825660) )
	ROM_LOAD( "an0b",   0x4000, 0x2000, CRC(09e4b780) SHA1(0735420b8529017e507feecf8f74fecd80fbf7d5) )
	ROM_LOAD( "an0c",   0x8000, 0x2000, CRC(0c127207) SHA1(b8372b2fa20ffe5ac278f558c07fd761c86e514b) )

	ROM_LOAD( "an07",   0x2000, 0x2000, CRC(6b8ad735) SHA1(a703523202d40e409e2345a6626b9e29b7a59cd3) )
	ROM_LOAD( "an08",   0x6000, 0x2000, CRC(b5518391) SHA1(57f6407491cff075f76a8b459cc33e8b9a91e7de) )
	ROM_LOAD( "an09",   0xa000, 0x2000, CRC(b7375fd7) SHA1(93a59e99e375bdba77199a705b5e304ece221617) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "af-8.bpr",    0x00, 0x20, CRC(20006a72) SHA1(6d0e1c6de45079f9e128186478a7e0ed3fd471d0) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "lp1-1.pld",   0x0000, 44, CRC(cc84cb09) SHA1(61620ef30dfd6c81cc517f95ab6358c619ca3298) )
	ROM_LOAD( "lp1-2.pld",   0x0100, 44, CRC(60e16fc4) SHA1(1df735f393ed0fcf1272fceada9764084ff11e07) )
	ROM_LOAD( "lp1-3.pld",   0x0200, 52, CRC(976a7c57) SHA1(202c55a236799fb44a977c074c231ed54c71a872) )
	ROM_LOAD( "lp1-4.pld",   0x0300, 52, CRC(cc9a442f) SHA1(5d08873b204b15f888d02d79e049119e05e41b45) )
	ROM_LOAD( "lp1-5.pld",   0x0400, 44, CRC(2d9f3118) SHA1(02e40a99f131bb47562d5b90fdfb11ca8cd90da6) )
	ROM_LOAD( "lp2-1.pld",   0x0500, 44, CRC(dbb05313) SHA1(fc37db24f12c4f5170945c9ec9a333e4583c1712) )
	ROM_LOAD( "lp2-4.pld",   0x0600, 44, CRC(4c72736c) SHA1(6f7521284a5d960ff05c4361095c3e89a79f7475) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "begas", 0, NO_DUMP )
ROM_END

ROM_START( rblaster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "01.bin",   0xc000, 0x2000, CRC(e4733c49) SHA1(357f46a80273f8a365d16cddf5e2caaeeacaf4ad) )
	ROM_LOAD( "00.bin",   0xe000, 0x2000, CRC(084d6ae2) SHA1(f49eb2d53bad5af88a12535ba628c9decce690ff) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "02.bin",   0xe000, 0x2000, CRC(6c20335d) SHA1(b28e80f112553af8e3fba9ebbfc10d1f56396ac1) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "03.bin",   0x0000, 0x2000, CRC(d1ff5ffb) SHA1(29df207e225e3b0477d5566d256198310d6ae526) )
	ROM_LOAD( "06.bin",   0x2000, 0x2000, CRC(d1ff5ffb) SHA1(29df207e225e3b0477d5566d256198310d6ae526) )
	ROM_LOAD( "04.bin",   0x4000, 0x2000, CRC(da2c84d9) SHA1(3452b0e2a45fa771e226c3a3668afbf3ceb0ec11) )
	ROM_LOAD( "07.bin",   0x6000, 0x2000, CRC(da2c84d9) SHA1(3452b0e2a45fa771e226c3a3668afbf3ceb0ec11) )
	ROM_LOAD( "05.bin",   0x8000, 0x2000, CRC(4608b516) SHA1(44af4be84a0b807ea0813ce86376a4b6fd927e5a) )
	ROM_LOAD( "08.bin",   0xa000, 0x2000, CRC(4608b516) SHA1(44af4be84a0b807ea0813ce86376a4b6fd927e5a) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "rblaster", 0, SHA1(1563ea907d461592e17646848fdf2dce904bba32) )
ROM_END

ROM_START( cobra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "au03-2",   0x8000, 0x2000, CRC(8f0a8fba) SHA1(8e11d2bd665a5ca6b3bb11aa2b707458c1534327) )
	ROM_LOAD( "au02-2",   0xa000, 0x2000, CRC(7db11acf) SHA1(1eebae0741f5735bc8966f3c31a9c07dac2e3916) )
	ROM_LOAD( "au01-2",   0xc000, 0x2000, CRC(523dd8f6) SHA1(47bd4c9b2272e9a710e6e97f2505075df68101ed) )
	ROM_LOAD( "au00-2",   0xe000, 0x2000, CRC(6c0f1f16) SHA1(ed05d3eaa24e84b1dfb4e1eb5f69b23e4a1494ba) )
	ROM_COPY( "maincpu",  0x8000, 0x4000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "au06",   0xe000, 0x2000, CRC(ccc94eb0) SHA1(354a933ddc6a1e1118c2bf176faaab5d01fc92d3) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "au0a",   0x0000, 0x2000, CRC(6aaedcf3) SHA1(52dc913eecf8a159784d500217cffd7a6d8eb45c) )
	ROM_LOAD( "au0b",   0x4000, 0x2000, CRC(92247877) SHA1(f9bb0c20212ab13caabfb5beb9b6afc807bc9555) )
	ROM_LOAD( "au0c",   0x8000, 0x2000, CRC(d00a2762) SHA1(84d4329b39b9fd30682b7efa5cb2744934c5ee5c) )
	ROM_LOAD( "au07",   0x2000, 0x2000, CRC(d4bf12a5) SHA1(e172f69ae02ac2670b70af0cfcf3887dd99c2761) )
	ROM_LOAD( "au08",   0x6000, 0x2000, CRC(63158274) SHA1(c728e8ba0a11ea67cf508877ad74a3aab9ef26fc) )
	ROM_LOAD( "au09",   0xa000, 0x2000, CRC(74e93394) SHA1(7a1470cf2008b1bef8d950939b758707297b3655) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "cobra", 0, SHA1(8390498294aca97a5d1769032e7b115d1a42f5d3) )
ROM_END

ROM_START( cobraa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bd03-a4.bin",      0x8000, 0x2000, CRC(f1b9df77) SHA1(7fcc613463441f69e4336c12252a616508c296d1) )
	ROM_LOAD( "bd02-a3.bin",      0xa000, 0x2000, CRC(3d802707) SHA1(89033b2f9b295b74b6b5034c837509377f5aba62) )
	ROM_LOAD( "bd01-a2.bin",      0xc000, 0x2000, CRC(1b4db507) SHA1(c58021cb7dcbcb159d9bb24d231755b3d07aa74a) )
	ROM_LOAD( "bd000-1-a1.bin",   0xe000, 0x2000, CRC(8d9ad777) SHA1(10914251350988a82c0cfa1dc6e22587b885cc71) )
	ROM_COPY( "maincpu",          0x8000, 0x4000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd07-b11.bin",   0xe000, 0x2000, CRC(584d714a) SHA1(e3df3c42367d879c5f12db78d2797049ba18e6f8) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "bd04-.a10",     0x0000, 0x2000, CRC(33013cc2) SHA1(380ca35a891805e2aa1d35000a3770483aaedbfc) )
	ROM_LOAD( "bd06-a8.bin",   0x4000, 0x2000, CRC(b1340125) SHA1(eef325c56d3a441432c7d31ea1dadfaa0c871c10) )
	ROM_LOAD( "bd05-a9.bin",   0x8000, 0x2000, CRC(98412178) SHA1(20e93f377a0d572a6752d7435aa4d0a3feec0e92) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "cobra", 0, SHA1(8390498294aca97a5d1769032e7b115d1a42f5d3) ) // might be wrong for this set

	ROM_REGION( 0x40000, "misc", 0 )
	ROM_LOAD( "lp4-1.pal16l8cn.bin",        0x0000, 0x40000, CRC(4aeb2c7e) SHA1(3c962656cffc8d927047c64a15afccab767d776f) ) // dumped with cgfm's tool
	ROM_LOAD( "lp4-1.pal16l8cn.pld",        0x0000, 0x00f71, CRC(ac1f1177) SHA1(ab721a840207354916c96e0ae83220fed12c6352) )
//  ROM_LOAD( "lp4-2-pal10l8.d6.jed",       0x0000, 0x00249, CRC(309b3ce5) SHA1(04f185911d33730004c7cd44a693dd1b69b82032) )
	ROM_LOAD( "lp4-2-pal10l8.d6.bin",       0x0000, 0x0002c, CRC(e594fd13) SHA1(4bb8a9b7cf8f8eaa3c9f290b6e5085a10c927e20) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "vd0-c.h15",        0x0000, 0x00020, CRC(02c27aa0) SHA1(e7b814aabbfbcd992f78254b29b6ab6fa8115429) )
	ROM_LOAD( "vd0-t.f6",         0x0020, 0x00020, CRC(78449942) SHA1(584e25f7bffccd943c4db1edf05552f7989e08a4) )
ROM_END

} // anonymous namespace


GAME( 1983, begas,    0,     rblaster, begas,    deco_ld_state, empty_init, ROT0, "Data East", "Bega's Battle (Revision 3)", MACHINE_NOT_WORKING )
GAME( 1983, begas1,   begas, rblaster, begas,    deco_ld_state, empty_init, ROT0, "Data East", "Bega's Battle (Revision 1)", MACHINE_NOT_WORKING )
GAME( 1984, cobra,    0,     rblaster, cobra,    deco_ld_state, empty_init, ROT0, "Data East", "Cobra Command (Data East LD, set 1)", MACHINE_NOT_WORKING )
GAME( 1984, cobraa,   cobra, cobraa,   cobra,    deco_ld_state, empty_init, ROT0, "Data East", "Cobra Command (Data East LD, set 2)", MACHINE_NOT_WORKING ) // might be a prototype
// Thunder Storm (Cobra Command Japanese version)
GAME( 1985, rblaster, 0,     rblaster, rblaster, deco_ld_state, empty_init, ROT0, "Data East", "Road Blaster (Data East LD)", MACHINE_NOT_WORKING )
