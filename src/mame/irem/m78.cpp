// license:BSD-3-Clause
// copyright-holders:

/*
Irem M78

Black Jack (1992)
Flyers show this game has a main screen and 5 user screens.
Is this the main unit or the satellite one?

2-PCB stack with IREM markings

Main components:

on main PCB:
2x Z0840004PSC
2x 27512 CPU ROM
2x HM6264 RAM
3.579545 MHz XTAL
YM2151 (sanded off)
bank of 8 DIP switches
battery

on sub PCB:
16 MHz XTAL
6x 27512 GFX ROM
6x TMM2064P-10 RAM

TODO:
- I/O section;
- video registers;
- Undumped sound ROMs. The sound program is almost identical to the one of irem/shisen.cpp games.
  Copied over sound handling from there for now;
- comms;

*/

#include "emu.h"

#include "m72_a.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/rstbuf.h"
#include "machine/timer.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class m78_state : public driver_device
{
public:
	m78_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tileram(*this, "tileram%u", 0),
		m_attrram(*this, "attrram%u", 0),
		m_colorram(*this, "colorram%u", 0)
	{}

	void bj92(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr_array<u8, 2> m_tileram;
	required_shared_ptr_array<u8, 2> m_attrram;
	required_shared_ptr_array<u8, 2> m_colorram;

	tilemap_t *m_tilemap[2] = {};

	void palette_init(palette_device &palette) const;

	TIMER_DEVICE_CALLBACK_MEMBER(sound_nmi) { m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero); }

	template <unsigned N> TILE_GET_INFO_MEMBER(get_tile_info) ATTR_COLD;
	template <unsigned N> void tileram_w(offs_t offset, uint8_t data);
	template <unsigned N> void attrram_w(offs_t offset, uint8_t data);
	template <unsigned N> void colorram_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void audio_program_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
};

// BBRRGGII, almost likely wrong
void m78_state::palette_init(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();

	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1;

		const u8 data = (color_prom[i] << 4) | (color_prom[i | 0x100]);
		int const br_bit0 = BIT(data, 6);
		int const br_bit1 = BIT(data, 7);

		bit0 = BIT(data, 0);
		bit1 = BIT(data, 1);
		int const b = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;

		bit0 = BIT(data, 2);
		bit1 = BIT(data, 3);
		int const r = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;

		bit0 = BIT(data, 4);
		bit1 = BIT(data, 5);
		int const g = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void m78_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m78_state::get_tile_info<0>)), TILEMAP_SCAN_COLS, 8, 8, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m78_state::get_tile_info<1>)), TILEMAP_SCAN_COLS, 8, 8, 64, 64);

	m_tilemap[0]->set_transparent_pen(0);
	//m_tilemap[1]->set_transparent_pen(0);
}

template <unsigned N> TILE_GET_INFO_MEMBER(m78_state::get_tile_info)
{
	int const attr = m_attrram[N][tile_index];
	int const code = m_tileram[N][tile_index] | ((attr & 0x1f) << 8);
	const u8 color = m_colorram[N][tile_index];
	u8 flags = 0;

	if (BIT(color, 4))
		flags |= TILE_FLIPX;
	if (BIT(color, 5))
		flags |= TILE_FLIPY;

	tileinfo.set(0, code, color & 0xf, flags);
}

template <unsigned N> void m78_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[N][offset] = data;
	m_tilemap[N]->mark_tile_dirty(offset);
}

template <unsigned N> void m78_state::attrram_w(offs_t offset, uint8_t data)
{
	m_attrram[N][offset] = data;
	m_tilemap[N]->mark_tile_dirty(offset);
}

template <unsigned N> void m78_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[N][offset] = data;
	m_tilemap[N]->mark_tile_dirty(offset);
}

uint32_t m78_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//bitmap.fill(0, cliprect);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void m78_state::main_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0xe000, 0xe7ff).ram();
}

void m78_state::main_io_map(address_map &map)
{
	//map(0x0000, 0x0000).r
	//map(0x0000, 0x0000).w // observed values: 0xfb, 0xfd, 0xfe
	//map(0x1000, 0x1000).w // hopper output?
	map(0x2000, 0x2000).portr("DSW1").w("soundlatch", FUNC(generic_latch_8_device::write)); // ??
	map(0x3000, 0x3000).portr("IN1");
	//map(0x3000, 0x3000).w // lamps for IN1?
	//map(0x4000, 0x4000).r
	//map(0x4000, 0x4000).w // observed values: 0x00
	map(0x5000, 0x5000).portr("IN0"); // ??
	map(0x8000, 0x8fff).ram().w(FUNC(m78_state::tileram_w<1>)).share(m_tileram[1]);
	map(0x9000, 0x9fff).ram().w(FUNC(m78_state::attrram_w<1>)).share(m_attrram[1]);
	map(0xa000, 0xafff).ram().w(FUNC(m78_state::colorram_w<1>)).share(m_colorram[1]);
	//map(0xb000, 0xbfff).ram(); // vregs, $b000, $b400, $b800, $bc00
	map(0xc000, 0xcfff).ram().w(FUNC(m78_state::tileram_w<0>)).share(m_tileram[0]);
	map(0xd000, 0xdfff).ram().w(FUNC(m78_state::attrram_w<0>)).share(m_attrram[0]);
	map(0xe000, 0xefff).ram().w(FUNC(m78_state::colorram_w<0>)).share(m_colorram[0]);
	//map(0xf000, 0xffff).ram(); // layer control at $f000?
}

void m78_state::audio_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("audiocpu", 0);
	map(0xfd00, 0xffff).ram();
}

void m78_state::audio_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x80, 0x80).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x80, 0x81).w("m72_audio", FUNC(m72_audio_device::shisen_sample_addr_w));
	map(0x82, 0x82).w("m72_audio", FUNC(m72_audio_device::sample_w));
	map(0x83, 0x83).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w));
	map(0x84, 0x84).r("m72_audio", FUNC(m72_audio_device::sample_r));
}


static INPUT_PORTS_START( bj92 )
	// TODO: Doesn't look PIO input, bit 2-5 high causes hang
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE ) PORT_NAME("Stand")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL ) PORT_NAME("Hit")
	PORT_DIPNAME( 0x04, 0x00, "IN1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	// TODO: these looks switches
	PORT_START("DSW1") // only 1 8-DIP bank
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5") // data clear
	PORT_DIPNAME( 0xe0, 0x20, "ID number" ) PORT_DIPLOCATION("DSW1:6,7,8")
	PORT_DIPSETTING(      0x0000, "Service Mode?" ) // press stand on "set my number"
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0060, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPSETTING(      0x00a0, "5" )
	PORT_DIPSETTING(      0x00c0, "0xc0 (invalid)" )
	PORT_DIPSETTING(      0x00e0, "0xe0 (invalid)" )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout tilelayout =
 {
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 , 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
};

static GFXDECODE_START( gfx_blackjack )
	GFXDECODE_ENTRY( "tiles",  0, charlayout, 0, 32 )
	GFXDECODE_ENTRY( "tiles2", 0, charlayout, 0, 32 )
	GFXDECODE_ENTRY( "tiles",  0, tilelayout, 0, 32 )
	GFXDECODE_ENTRY( "tiles2", 0, tilelayout, 0, 32 )
GFXDECODE_END


void m78_state::bj92(machine_config &config)
{
	Z80(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &m78_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &m78_state::main_io_map);

	z80_device &audiocpu(Z80(config, "audiocpu", 3.579545_MHz_XTAL));
	audiocpu.set_addrmap(AS_PROGRAM, &m78_state::audio_program_map);
	audiocpu.set_addrmap(AS_IO, &m78_state::audio_io_map);
	// IRQs are generated by main Z80 and YM2151
	audiocpu.set_irq_acknowledge_callback("soundirq", FUNC(rst_neg_buffer_device::inta_cb));

	TIMER(config, "v1").configure_scanline(FUNC(m78_state::sound_nmi), "screen", 1, 2);  // clocked by V1? (Vigilante)

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(55);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(512, 384);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(m78_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE);

	GFXDECODE(config, "gfxdecode", "palette", gfx_blackjack);
	PALETTE(config, "palette", FUNC(m78_state::palette_init), 256);

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set("soundirq", FUNC(rst_neg_buffer_device::rst18_w));
	soundlatch.set_separate_acknowledge(true);

	RST_NEG_BUFFER(config, "soundirq").int_callback().set_inputline("audiocpu", 0);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	m72_audio_device &m72_audio(IREM_M72_AUDIO(config, "m72_audio"));
	m72_audio.set_dac_tag("dac");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3.579545_MHz_XTAL ));   // Verified on PCB
	ymsnd.irq_handler().set("soundirq", FUNC(rst_neg_buffer_device::rst28_w));
	ymsnd.add_route(0, "lspeaker", 0.5);
	ymsnd.add_route(1, "rspeaker", 0.5);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "lspeaker", 0.25).add_route(ALL_OUTPUTS, "rspeaker", 0.25); // unknown DAC
}


ROM_START( bj92 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2.mp.ic29", 0x00000, 0x10000, CRC(783a9b77) SHA1(ff8d7b69879308f74137a250fb1f96688dc5d213) ) // 1xxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1.sp.ic23", 0x00000, 0x10000, CRC(860fbd4c) SHA1(c021b487f4c27ef907e59cb3e48fd587234957e1) ) // 11xxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x30000, "tiles", 0 )
	ROM_LOAD( "7.c0.ic77", 0x00000, 0x10000, CRC(5f82fb7c) SHA1(27b4a40cd2824bd639c1bef7e2988f71bcc54dfd) )
	ROM_LOAD( "6.c1.ic76", 0x10000, 0x10000, CRC(fb2116a9) SHA1(e6887cab4f6fb17072bb513735d566845dd4491f) )
	ROM_LOAD( "5.c2.ic75", 0x20000, 0x10000, CRC(c2a2ae52) SHA1(2f91722725b03250e58242fd772a83441789dbce) )

	ROM_REGION( 0x30000, "tiles2", 0 ) // identical copies of the above. Does this drive 2 screens?
	ROM_LOAD( "8.c0.ic83",  0x00000, 0x10000, CRC(5f82fb7c) SHA1(27b4a40cd2824bd639c1bef7e2988f71bcc54dfd) )
	ROM_LOAD( "9.c1.ic84",  0x10000, 0x10000, CRC(fb2116a9) SHA1(e6887cab4f6fb17072bb513735d566845dd4491f) )
	ROM_LOAD( "10.c2.ic85", 0x20000, 0x10000, CRC(c2a2ae52) SHA1(2f91722725b03250e58242fd772a83441789dbce) )

	ROM_REGION( 0x40000, "m72_audio", ROMREGION_ERASE00 )
	// there are 2 empty sockets near the scratched off sound chip.
	// Outputs DAC sound if populated
	ROM_LOAD("3.v0.ic46", 0x00000, 0x20000, NO_DUMP )
	ROM_LOAD("4.v1.ic47", 0x20000, 0x20000, NO_DUMP )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.ic67", 0x000, 0x100, CRC(3e2128b6) SHA1(71e74999c18a2a4e59a7c8388b6deb0918afd669) )
	ROM_LOAD( "82s129.ic69", 0x100, 0x100, CRC(6fdff4a5) SHA1(71fda10c4bf830787218e9be1223259face1cd8e) )
ROM_END

} // anonymous namespace


// revision under Dip-Switch Test
GAME( 1992, bj92, 0, bj92, bj92, m78_state, empty_init, ROT90, "Irem", "Black Jack (Irem, satellite unit, rev. T)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS )
