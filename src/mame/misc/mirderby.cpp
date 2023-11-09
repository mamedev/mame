// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Miracle Derby - Ascot

- has the same GX61A01 custom (blitter?) as homedata.cpp and a 'similar' CPU setup (this has more CPUs)
 and similar board / rom numbering (X**-)

The drivers can probably be merged later, although the current per-game handling of the blitter in
homedata.cpp should be looked at.
Update: GX61A01 looks more of a CRTC, also note that "blitter" ROM spot at 12e on homedata PCBs is
actually a CPU. Is this a bootleg of an Home Data original?

===================================================================================================

Notes from Stefan Lindberg:

Eprom "x70_a04.5g" had wires attached to it, pin 2 and 16 was joined and pin 1,32,31,30 was joined, 
i removed them and read the eprom as the type it was (D27c1000D).

Measured frequencies:
MBL68B09E = 2mhz
MBL68B09E = 2mhz
z80 = 4mhz
YM2203 = 2mhz

See included PCB pics.



Roms:

Name              Size     CRC32         Chip Type
---------------------------------------------------------------------------------
x70a07.8l         256      0x7d4c9712    82s129
x70a08.7l         256      0xc4e77174    82s129
x70a09.6l         256      0xd0187957    82s129
x70_a03.8g        32768    0x4e298b2d    27c256
x70_a04.5g        131072   0x14392fdb    D27c1000D
x70_a11.1g        32768    0xb394eef7    27c256
x70_b02.12e       32768    0x76c9bb6f    27c256
x70_c01.14e       65536    0xd79d072d    27c512


**************************************************************************************************/

#include "emu.h"
//#include "homedata.h"

#include "cpu/m6809/m6809.h"
#include "cpu/upd7810/upd7810.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/dac.h"
#include "sound/sn76496.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

class mirderby_state : public driver_device
{
public:
	mirderby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ymsnd(*this, "ymsnd")
//		, m_vreg(*this, "vreg")
		, m_videoram(*this, "videoram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_subbank(*this, "subbank")
		, m_soundlatch(*this, "soundlatch")
//		, m_mainlatch(*this, "mainlatch")
//		, m_sn(*this, "snsnd")
		, m_keys(*this, "KEY%u", 0U)
	{
	}

	void mirderby(machine_config &config);

private:
//	optional_region_ptr<uint8_t> m_blit_rom;

	required_device<mc6809e_device> m_maincpu;
	required_device<mc6809e_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<ym2203_device> m_ymsnd;
//	optional_shared_ptr<uint8_t> m_vreg;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_memory_bank m_subbank;
	optional_device<generic_latch_8_device> m_soundlatch;
//	optional_device<generic_latch_8_device> m_mainlatch; // pteacher
//	optional_device<sn76489a_device> m_sn; // mrokumei and pteacher

	optional_ioport_array<12> m_keys;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void screen_vblank(int state);
	virtual void video_start() override;


	uint8_t prot_r();
	void prot_w(uint8_t data);
	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void videoram_w(offs_t offset, u8 data);

	void main_map(address_map &map);
	void sub_map(address_map &map);

	void shared_map(address_map &map);

	void audio_map(address_map &map);
	void audio_io(address_map &map);

	tilemap_t *m_bg_tilemap{};
//	int m_visible_page = 0;
//	int m_priority = 0;
//	[[maybe_unused]] int m_flipscreen = 0;
	u8 m_prot_data = 0;
	u8 m_latch = 0;
	u16 m_gfx_flip = 0;
};

void mirderby_state::palette_init(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x100; i++)
	{
		int const r = color_prom[0x000 + i];
		int const g = color_prom[0x100 + i];
		int const b = color_prom[0x200 + i];

		palette.set_pen_color(i, pal4bit(r), pal4bit(g), pal4bit(b));
	}
}

void mirderby_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty((offset & 0xffe) >> 1);
}



TILE_GET_INFO_MEMBER(mirderby_state::get_bg_tile_info)
{
	int const addr  = tile_index * 2;
	int const attr  = m_videoram[addr];
	int const code  = m_videoram[addr + 1] + ((attr & 0x03) << 8) + 0x400 + m_gfx_flip;
	int const color = (attr >> 4) & 0xf;

	tileinfo.set(1, code, color, 0 );
}

void mirderby_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mirderby_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

//	m_bg_tilemap->set_transparent_pen(0);
}

uint32_t mirderby_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void mirderby_state::screen_vblank(int state)
{
	if (state)
	{
		// TODO: each irq routine pings a bit of $8000 for masking/acknowledge
		// TODO: study FIRQ for main CPU
		m_maincpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
		// FIRQ and IRQ same for sub CPU
		m_subcpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	}
}



uint8_t mirderby_state::prot_r()
{
	m_prot_data&=0x7f;
	return m_prot_data++;
}

void mirderby_state::prot_w(uint8_t data)
{
	m_prot_data = data;
}


void mirderby_state::shared_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().w(FUNC(mirderby_state::videoram_w)).share(m_videoram);
	map(0x2000, 0x2fff).ram().share("share3");
	map(0x3000, 0x3fff).ram();
	map(0x4000, 0x5fff).ram().share("share1");
	map(0x6000, 0x6fff).ram(); /* work ram */
	map(0x7000, 0x77ff).ram().share("share2");
	map(0x7800, 0x7800).rw(FUNC(mirderby_state::prot_r), FUNC(mirderby_state::prot_w)); // protection check? (or sound comms?)
	//0x7ff0 onward seems CRTC
//	map(0x7ff0, 0x7ff?).writeonly().share("vreg");
	map(0x7ff2, 0x7ff2).portr("IN0");
	map(0x7ff9, 0x7ffa).lr8(
		NAME([] (offs_t offset) {
			return 0;
		})
	);
	map(0x7ffe, 0x7ffe).nopr(); //watchdog?
	map(0x7ffe, 0x7ffe).lw8(
		NAME([this] (u8 data) {
			m_latch = data;
			//logerror("%02x latch write\n", data);
		})
	);
	map(0x7fff, 0x7fff).lrw8(
		NAME([this] () {
			//	0x7fff $e / $f writes -> DSW reads
			return m_ymsnd->read(1);
		}),
		NAME([this] (u8 data) {
			//logerror("%s -> %02x\n", BIT(m_latch, 2) ? "address" : "data", data);
			m_ymsnd->write(BIT(m_latch, 2) ? 0 : 1, data);
		})
	);
}

void mirderby_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).m(*this, FUNC(mirderby_state::shared_map));
	map(0x7ffd, 0x7ffd).lw8(
		NAME([this] (u8 data) {
			m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			//logerror("%02x latch write\n", data);
		})
	);
	map(0x8000, 0xffff).rom().region("main_rom", 0);
//	map(0x8000, 0x8000).lw8(
//		NAME([this] (offs_t offset, u8 data) {
//			
//		})
//	);
}

void mirderby_state::sub_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).m(*this, FUNC(mirderby_state::shared_map));
	map(0x7ffd, 0x7ffd).lw8(
		NAME([this] (u8 data) {
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			//logerror("%02x latch write\n", data);
		})
	);
	map(0x7ffe, 0x7ffe).nopr(); //watchdog?
	map(0x8000, 0xffff).bankr(m_subbank);
	map(0x8000, 0x8000).lw8(
		NAME([this] (u8 data) {
			m_subbank->set_entry(BIT(data, 7) ? 1 : 0);
			// TODO: other bits used
			const u16 new_gfx_flip = BIT(data, 5) ? 0x800 : 0;
			if (new_gfx_flip != m_gfx_flip)
			{
				m_gfx_flip = new_gfx_flip;
				m_bg_tilemap->mark_all_dirty();
			}
		})
	);
}

void mirderby_state::audio_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("audio_rom", 0);
	map(0x8000, 0x87ff).ram();
}

void mirderby_state::audio_io(address_map &map)
{
	map.global_mask(0xff);
//	map(0x00, 0x00) read in NMI, likely soundlatch
//	Is this just a DAC player?
}

static GFXDECODE_START( gfx_mirderby )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb, 0x0000, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_msb, 0x0000, 0x10 )
GFXDECODE_END


static INPUT_PORTS_START( mirderby )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
INPUT_PORTS_END

void mirderby_state::machine_start()
{
	m_subbank->configure_entries(0, 2, memregion("sub_rom")->base(), 0x8000);
}

void mirderby_state::machine_reset()
{
	m_subbank->set_entry(0);
	m_gfx_flip = 0;
}

/* clocks are 16mhz and 9mhz */
void mirderby_state::mirderby(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, 16000000/8);  /* 2 Mhz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mirderby_state::main_map);
//	m_maincpu->set_vblank_int("screen", FUNC(mirderby_state::homedata_irq));

	MC6809E(config, m_subcpu, 16000000/8); /* 2 Mhz */
	m_subcpu->set_addrmap(AS_PROGRAM, &mirderby_state::sub_map);
//	m_subcpu->set_vblank_int("screen", FUNC(mirderby_state::homedata_irq));

	// im 0, doesn't bother in setting a vector table,
	// should just require a NMI from either CPUs
	Z80(config, m_audiocpu, 16000000/4);   /* 4 Mhz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &mirderby_state::audio_map);
	m_audiocpu->set_addrmap(AS_IO, &mirderby_state::audio_io);

//	config.set_maximum_quantum(attotime::from_hz(6000));
	config.set_perfect_quantum("maincpu");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(1*8, 49*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(mirderby_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(mirderby_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mirderby);
	PALETTE(config, m_palette, FUNC(mirderby_state::palette_init), 0x100);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	YM2203(config, m_ymsnd, 2'000'000);
	m_ymsnd->port_a_read_callback().set_ioport("DSW1");
	m_ymsnd->port_b_read_callback().set_ioport("DSW2");
	m_ymsnd->add_route(0, "speaker", 0.25);
	m_ymsnd->add_route(1, "speaker", 0.25);
	m_ymsnd->add_route(2, "speaker", 0.25);
	m_ymsnd->add_route(3, "speaker", 1.0);
}

ROM_START( mirderby )
	ROM_REGION( 0x8000, "main_rom", 0 ) // M6809 code
	ROM_LOAD( "x70_b02.12e", 0x0000, 0x8000, CRC(76c9bb6f) SHA1(dd8893f3082d33d366247295e9531f8879c219c5) )

	ROM_REGION( 0x10000, "sub_rom", 0 ) // M6809 code
	ROM_LOAD( "x70_c01.14e", 0x00000, 0x10000, CRC(d79d072d) SHA1(8e189931de9c4eb520c1ec2d0898d8eaba0f01b5) )

	ROM_REGION( 0x2000, "audio_rom", 0 ) // Z80 Code
	ROM_LOAD( "x70_a11.1g", 0x0000, 0x2000, CRC(b394eef7) SHA1(a646596d09b90eda44aaf8ccbf8f3fccfd3d5dad) ) // first 0x6000 bytes are blank!
	ROM_CONTINUE(0x0000, 0x2000)
	ROM_CONTINUE(0x0000, 0x2000)
	ROM_CONTINUE(0x0000, 0x2000) // main z80 code is here

	ROM_REGION( 0x8000, "gfx1", 0 ) // horse gfx
	ROM_LOAD( "x70_a03.8g", 0x0000, 0x8000, CRC(4e298b2d) SHA1(ae78327d1f30c8d19ef772b82803dab4d6b7b919))

	ROM_REGION( 0x20000, "gfx2", 0 ) // fonts etc.
	ROM_LOAD( "x70_a04.5g", 0x0000, 0x20000, CRC(14392fdb) SHA1(dafdce473b2d2ebbdbf49fbd12f85c1ad69b2877) )

	ROM_REGION( 0x300, "proms", 0 ) // colours
	ROM_LOAD( "x70a07.8l", 0x000, 0x100, CRC(7d4c9712) SHA1(fe2a89841fdf5e4fd6cd41478ad2f29d28bed54d) )
	ROM_LOAD( "x70a08.7l", 0x100, 0x100, CRC(c4e77174) SHA1(ada238ded69f01b4daeb0159a2c5c422977bb95e) )
	ROM_LOAD( "x70a09.6l", 0x200, 0x100, CRC(d0187957) SHA1(6b36c1bccad24708cfa2fc78da08313f9bcfdbc0) )
ROM_END

GAME( 1988, mirderby,  0, mirderby, mirderby, mirderby_state, empty_init, ROT0, "Home Data? / Ascot", "Miracle Derby (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
