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
- "ERROR 14", keeps checking location $e400 (irq or shared comms), "SET MY NUMBER" if bypassed.
- inputs;
- verify sound. The sound program is almost identical to the one of irem/shisen.cpp games.
  Copied over sound handling from there for now.
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
		m_tileram(*this, "tileram"),
		m_attrram(*this, "attrram"),
		m_colorram(*this, "colorram")
	{}

	void bj92(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_tileram;
	required_shared_ptr<uint8_t> m_attrram;
	required_shared_ptr<uint8_t> m_colorram;

	tilemap_t *m_tilemap = nullptr;

	void palette_init(palette_device &palette) const;

	TIMER_DEVICE_CALLBACK_MEMBER(sound_nmi) { m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero); }

	TILE_GET_INFO_MEMBER(get_tile_info) ATTR_COLD;
	void tileram_w(offs_t offset, uint8_t data);
	void attrram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void audio_program_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
};

void m78_state::palette_init(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();

	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2;

		const u8 data = (color_prom[i] << 4) | (color_prom[i | 0x100]);

		bit0 = 0;
		bit1 = BIT(data, 6);
		bit2 = BIT(data, 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(data, 3);
		bit1 = BIT(data, 4);
		bit2 = BIT(data, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(data, 0);
		bit1 = BIT(data, 1);
		bit2 = BIT(data, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void m78_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m78_state::get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 64);
}

// TODO: theorical mapping, to be verified
// NOTE: conceals ROM/RAM messages unless a NG is returned, seems intentional
TILE_GET_INFO_MEMBER(m78_state::get_tile_info)
{
	int const attr = m_attrram[tile_index];
	int const code = m_tileram[tile_index] | ((attr & 0x1f) << 8);
	const u8 color = m_colorram[tile_index] & 0x1f;

	tileinfo.set(0, code, color, 0);
}

void m78_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void m78_state::attrram_w(offs_t offset, uint8_t data)
{
	m_attrram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void m78_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

uint32_t m78_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

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
	//map(0x1000, 0x1000).w // observed values: 0x00, 0x40
	map(0x2000, 0x2000).portr("DSW1").w("soundlatch", FUNC(generic_latch_8_device::write)); // ??
	//map(0x3000, 0x3000).r
	//map(0x3000, 0x3000).w // observed values: 0x00
	//map(0x4000, 0x4000).r
	//map(0x4000, 0x4000).w // observed values: 0x00
	map(0x5000, 0x5000).portr("IN0"); // ??
	map(0x8000, 0x8fff).ram().w(FUNC(m78_state::tileram_w)).share(m_tileram);
	map(0x9000, 0x9fff).ram().w(FUNC(m78_state::attrram_w)).share(m_attrram);
	map(0xa000, 0xafff).ram().w(FUNC(m78_state::colorram_w)).share(m_colorram);
	map(0xb000, 0xbfff).ram();
	map(0xc000, 0xcfff).ram(); // writes here
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xefff).ram(); // writes at the same offsets of 0xc000-0xcfff
	map(0xf000, 0xffff).ram();
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
	// TODO: mapped for testing, fix this
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1)

	PORT_START("DSW1") // only 1 8-DIP bank
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW1:8")
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout tilelayout =
 {
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
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
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
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
	// there are 2 empty sockets near the scratched off sound chip. ROMs removed or never populated?

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.ic67", 0x000, 0x100, CRC(3e2128b6) SHA1(71e74999c18a2a4e59a7c8388b6deb0918afd669) )
	ROM_LOAD( "82s129.ic69", 0x100, 0x100, CRC(6fdff4a5) SHA1(71fda10c4bf830787218e9be1223259face1cd8e) )
ROM_END

} // anonymous namespace


GAME( 1992, bj92, 0, bj92, bj92, m78_state, empty_init, ROT90, "Irem", "Black Jack (Irem)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
