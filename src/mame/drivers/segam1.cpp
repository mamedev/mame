// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Sega M1 hardware (837-7571) (PCB)

Sega Bingo Multicart (837-10675) (Sticker on PCB)

used for redemption / gambling style machines in a satellite setup

based on Caribbean Boule the following hardware setup is used

One X-Board (segaxbd.cpp) drives a large rear-projection monitor which all players view to see the main game progress.

Multiple M1 boards ("satellite" board) for each player for them to view information privately.

One 'link' board which connects everything together.  The link board has audio hardware, a 68K, and a Z80 as
well as a huge bank of UARTS and toslink connectors, but no video. It's possible the main game logic runs
on the 'link' board.


Unfortunately we don't have any dumps of anything other than an M1 board right now.

---

is this related to (or a component of?) bingoc.cpp, the EPR numbers are much lower there tho
so it's probably an earlier version of the same thing or one of the 'link' boards?

uses s24 style tilemaps (ram based?)


*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/315_5296.h"
#include "machine/gen_latch.h"
#include "machine/i8251.h"
#include "machine/mb8421.h"
#include "sound/ymopn.h"
#include "video/segaic24.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"



class segam1_state : public driver_device
{
public:
	segam1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_m1comm(*this, "m1comm")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_paletteram(*this, "paletteram")
		, m_tile(*this, "tile")
		, m_mixer(*this, "mixer")
		, m_ymsnd(*this, "ymsnd")
	{ }

	void unkm1(machine_config &config);
	void segam1(machine_config &config);

private:
	void paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_a0_bank_w(uint8_t data);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<z80_device> m_m1comm;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<u16> m_paletteram;
	required_device<segas24_tile_device> m_tile;
	required_device<segas24_mixer_device> m_mixer;
	required_device<ym3438_device> m_ymsnd;
	void segam1_comms_map(address_map &map);
	void segam1_map(address_map &map);
	void segam1_sound_io_map(address_map &map);
	void segam1_sound_map(address_map &map);
	void unkm1_sound_map(address_map &map);
};

void segam1_state::machine_start()
{
	membank("soundbank")->configure_entries(0x00, 0x10, memregion("audiocpu")->base(), 0x2000);
}

void segam1_state::video_start()
{
}

void segam1_state::sound_a0_bank_w(uint8_t data)
{
	membank("soundbank")->set_entry(data & 0x0f);
}

// 315-5242

void segam1_state::paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	data = m_paletteram[offset];

	u16 r = (data & 0x00f) << 4;
	if(data & 0x1000)
		r |= 8;

	u16 g = data & 0x0f0;
	if(data & 0x2000)
		g |= 8;

	u16 b = (data & 0xf00) >> 4;
	if(data & 0x4000)
		b |= 8;

	r |= r >> 5;
	g |= g >> 5;
	b |= b >> 5;

	m_palette->set_pen_color(offset, rgb_t(r, g, b));

	if(data & 0x8000) {
		r = 255-0.6*(255-r);
		g = 255-0.6*(255-g);
		b = 255-0.6*(255-b);
	} else {
		r = 0.6*r;
		g = 0.6*g;
		b = 0.6*b;
	}
	m_palette->set_pen_color(offset+m_palette->entries()/2, rgb_t(r, g, b));
}


// copied from segas24.cpp
namespace {
	struct layer_sort {
		layer_sort(segas24_mixer_device *_mixer) { mixer = _mixer; }

		bool operator()(int l1, int l2) {
			static const int default_pri[12] = { 0, 1, 2, 3, 4, 5, 6, 7, -4, -3, -2, -1 };
			int p1 = mixer->get_reg(l1) & 7;
			int p2 = mixer->get_reg(l2) & 7;
			if(p1 != p2)
				return p1 - p2 < 0;
			return default_pri[l2] - default_pri[l1] < 0;
		}

		segas24_mixer_device *mixer;
	};
}

// copied from segas24.cpp
uint32_t segam1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_mixer->get_reg(13) & 1)
	{
		bitmap.fill(m_palette->black_pen());
		return 0;
	}

	screen.priority().fill(0);
	bitmap.fill(0, cliprect);

	std::vector<int> order;
	order.resize(12);
	for(int i=0; i<12; i++)
		order[i] = i;

	std::sort(order.begin(), order.end(), layer_sort(m_mixer.target()));

	int level = 0;
	for(int i=0; i<12; i++)
		if(order[i] < 8)
			m_tile->draw(screen, bitmap, cliprect, order[i], level, 0);
		else
			level++;

	return 0;
}



void segam1_state::segam1_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x340000, 0x340fff).rw("dpram", FUNC(mb8421_device::right_r), FUNC(mb8421_device::right_w)).umask16(0x00ff);
	map(0xb00000, 0xb0ffff).rw(m_tile, FUNC(segas24_tile_device::tile_r), FUNC(segas24_tile_device::tile_w));
	map(0xb20000, 0xb20001).nopw();        /* Horizontal split position (ABSEL) */
	map(0xb40000, 0xb40001).nopw();        /* Scanline trigger position (XHOUT) */
	map(0xb60000, 0xb60001).nopw();        /* Frame trigger position (XVOUT) */
	map(0xb70000, 0xb70001).nopw();        /* Synchronization mode */
	map(0xb80000, 0xbfffff).rw(m_tile, FUNC(segas24_tile_device::char_r), FUNC(segas24_tile_device::char_w));
	map(0xc00000, 0xc03fff).ram().w(FUNC(segam1_state::paletteram_w)).share("paletteram");
	map(0xc04000, 0xc0401f).rw(m_mixer, FUNC(segas24_mixer_device::read), FUNC(segas24_mixer_device::write));
	map(0xe00000, 0xe0001f).rw("io1", FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0x00ff);
	map(0xe40000, 0xe40001).portr("INX");
	map(0xe40002, 0xe40003).portr("INY");
	map(0xe40005, 0xe40005).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xe40006, 0xe40007).nopw();
	map(0xe40008, 0xe40009).portr("INZ");
	map(0xe80000, 0xe8001f).rw("io2", FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0x00ff);
	map(0xf00000, 0xf03fff).mirror(0x0fc000).ram(); // NVRAM?
}

void segam1_state::segam1_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xbfff).bankr("soundbank");
	map(0xf000, 0xffff).ram();
}

void segam1_state::unkm1_sound_map(address_map &map)
{
	segam1_sound_map(map);
	map(0xe000, 0xefff).ram();
}

void segam1_state::segam1_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x83).rw(m_ymsnd, FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0xa0, 0xa0).w(FUNC(segam1_state::sound_a0_bank_w));
	map(0xc0, 0xc0).r("soundlatch", FUNC(generic_latch_8_device::read)).nopw();
}

void segam1_state::segam1_comms_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa7ff).rw("dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
	map(0xc000, 0xc001).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xe003, 0xe003).nopw(); // ???
}


static INPUT_PORTS_START( segam1 )
	PORT_START("INA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IND")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INF")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("ING")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INX")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INZ")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END




void segam1_state::segam1(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(20'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &segam1_state::segam1_map);
	m_maincpu->set_vblank_int("screen", FUNC(segam1_state::irq4_line_hold));

	Z80(config, m_audiocpu, 4000000); // unknown clock
	m_audiocpu->set_addrmap(AS_PROGRAM, &segam1_state::segam1_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &segam1_state::segam1_sound_io_map);

	Z80(config, m_m1comm, 4000000); // unknown clock
	m_m1comm->set_addrmap(AS_PROGRAM, &segam1_state::segam1_comms_map);

	sega_315_5296_device &io1(SEGA_315_5296(config, "io1", 0)); // unknown clock
	io1.in_pa_callback().set_ioport("INA");
	io1.in_pb_callback().set_ioport("INB");
	io1.in_pc_callback().set_ioport("INC");
	io1.in_pd_callback().set_ioport("IND");
	io1.in_pe_callback().set_ioport("INE");
	io1.in_pf_callback().set_ioport("INF");

	sega_315_5296_device &io2(SEGA_315_5296(config, "io2", 0)); // unknown clock
	io2.in_pg_callback().set_ioport("ING");

	I8251(config, "uart", 4000000); // unknown clock

	mb8421_device &dpram(MB8421(config, "dpram"));
	dpram.intl_callback().set_inputline("m1comm", 0);

	S24TILE(config, m_tile, 0, 0x3fff).set_palette(m_palette);
	S24MIXER(config, m_mixer, 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	m_screen->set_raw(16000000, 656, 0, 496, 424, 0, 384); // copied from segas24.cpp; may not be accurate
	m_screen->set_screen_update(FUNC(segam1_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(8192*2);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline("audiocpu", INPUT_LINE_NMI);

	YM3438(config, m_ymsnd, 8000000);
	m_ymsnd->add_route(ALL_OUTPUTS, "mono", 0.40);
	//m_ymsnd->irq_handler().set(FUNC(segam1_state::ym3438_irq_handler));
}

void segam1_state::unkm1(machine_config &config)
{
	segam1(config);
	m_audiocpu->set_addrmap(AS_PROGRAM, &segam1_state::unkm1_sound_map);

	m_m1comm->set_disable(); // not dumped yet
}


ROM_START( bingpty ) // 1994/05/01 string
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "epr-16648b.bin", 0x00000, 0x20000, CRC(e4fceb4c) SHA1(0a248bb328d2f6d72d540baefbe62838f4b76585) )
	ROM_LOAD16_BYTE( "epr-16649b.bin", 0x00001, 0x20000, CRC(736d8bbd) SHA1(c359ad513d4a7693cbb1a27ce26f89849e894d05) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "epr-14845.bin", 0x00000, 0x20000, CRC(90d47101) SHA1(7bc002c104e3dbde1986aaec54112d5658eab523) )

	ROM_REGION( 0x8000, "m1comm", 0 ) /* Z80 Code */
	ROM_LOAD( "epr-14221a.bin", 0x00000, 0x8000, CRC(a13e67a4) SHA1(4cd269c7f04a64ae7806c8784f86bf6553a25d85) )

	// dumps of the X-Board part, and the LINK PCB are missing.
ROM_END

ROM_START( unkm1 ) // 1992.01.31 string
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "epr-14427.ic8", 0x00000, 0x40000, CRC(2d904fc6) SHA1(7062f47d77d09906420118c85e1cb565bec345a7) )
	ROM_LOAD16_BYTE( "epr-14428.ic7", 0x00001, 0x40000, CRC(97a317f4) SHA1(19bc4cf6b6c580caa44f36c929b445ed94b2d9eb) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "epr-14429.ic104", 0x00000, 0x20000, CRC(1ff8262d) SHA1(fb90bd877b2dc65eb3e5495d6e21dee1f871fb44) )

	ROM_REGION( 0x8000, "m1comm", 0 )
	ROM_LOAD( "epr-14426.ic2", 0x0000, 0x8000, NO_DUMP ) // on "SYSTEM M1 COM" board with Z80, MB8421 and TMP82C51AP

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "315-5472-01.ic22", 0x000, 0x0eb, CRC(828ee6e2) SHA1(f32dd0f6297cc8bd3049be4bca502c0f8ec738cf) )
	// dumps of the X-Board part, and the LINK PCB are missing.
ROM_END

GAME( 1994, bingpty,    0,        segam1,    segam1, segam1_state, empty_init, ROT0,  "Sega", "Bingo Party Multicart (Rev B) (M1 Satellite board)", MACHINE_NOT_WORKING )
GAME( 1992, unkm1,      0,        unkm1,     segam1, segam1_state, empty_init, ROT0,  "Sega", "unknown Sega gambling game (M1 Satellite board)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
