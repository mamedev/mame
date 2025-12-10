// license:BSD-3-Clause
// copyright-holders:David Haywood

// Thanks to SynaMax & Dutchman2000

// The hardware design is cloned from Donkey Kong

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ymopm.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class jammin_state : public driver_device
{
public:
	jammin_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_ym2151(*this, "ym2151"),
		m_bgram(*this, "bgram"),
		m_spram(*this, "spram"),
		m_proms(*this, "proms"),
		m_proms2(*this, "proms_lookup")
	{ }

	void jammin(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ym2151_device> m_ym2151;
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_spram;
	required_region_ptr<uint8_t> m_proms;
	required_region_ptr<uint8_t> m_proms2;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_dma_param0;
	uint16_t m_dma_param1;
	uint16_t m_dma_param2;
	uint16_t m_dma_param3;
	uint16_t m_dma_param4;
	uint8_t m_dma_go;
	uint8_t m_pal1;
	uint8_t m_pal2;

	void pal_init(palette_device &palette) const;

	void bgram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void dma_param0_w(uint8_t data);
	void dma_param1_w(uint8_t data);
	void dma_param2_w(uint8_t data);
	void dma_param3_w(uint8_t data);
	void dma_param4_w(uint8_t data);
	void dma_go_w(uint8_t data);
	void pal1_w(uint8_t data);
	void pal2_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
};


static const res_net_decode_info dkong_decode_info =
{
	2,      /*  there may be two proms needed to construct color */
	0,      /*  start at 0 */
	255,    /*  end at 255 */
	/*  R,   G,   B,   R,   G,   B */
	{ 256, 256,   0,   0,   0,   0},        /*  offsets */
	{   1,  -2,   0,   0,   2,   0},        /*  shifts */
	{0x07,0x04,0x03,0x00,0x03,0x00}         /*  masks */
};

static const res_net_info dkong_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 2, {  470, 220,   0 } }  /*  dkong */
	}
};

static const res_net_info dkong_net_bck_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 0, { 0 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 0, { 0 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 0, { 0 } }
	}
};


void jammin_state::pal_init(palette_device &palette) const
{
	const uint8_t *color_prom = m_proms;

	std::vector<rgb_t> rgb;
	compute_res_net_all(rgb, color_prom, dkong_decode_info, dkong_net_info);
	palette.set_pen_colors(0, rgb);

	// Now treat tri-state black background generation
	for (int i = 0; i < 256; i++)
		if ((i & 0x03) == 0x00)  // NOR => CS=1 => Tristate => real black
		{
			int const r = compute_res_net(1, 0, dkong_net_bck_info);
			int const g = compute_res_net(1, 1, dkong_net_bck_info);
			int const b = compute_res_net(1, 2, dkong_net_bck_info);
			palette.set_pen_color(i, r, g, b);
		}

	palette.palette()->normalize_range(0, 255);
}


TILE_GET_INFO_MEMBER(jammin_state::get_bg_tile_info)
{
	int code = m_bgram[tile_index];

	int col = tile_index & 0x01f;
	int row = (tile_index & 0x3e0) >> 5;

	int lookup = col + ((row / 4) * 0x20);

	lookup += (m_pal2 & 1) * 0x100;

	int colour = m_proms2[lookup];

	colour += (m_pal1 & 1) * 0x10;

	tileinfo.set(0,
		code,
		colour,
		0);
}

void jammin_state::bgram_w(offs_t offset, uint8_t data)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void jammin_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jammin_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t jammin_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for (int i = 0; i < 0x200; i += 4)
	{
		uint8_t y = m_spram[i + 0];
		uint8_t tile = m_spram[i + 1];
		uint8_t pal = m_spram[i + 2];
		uint8_t x = m_spram[i + 3];

		y = 255 - y;

		bool flipx = pal & 0x80;
		bool flipy = tile & 0x80;

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, tile, pal & 0xf, flipx, flipy, x-8, y-8, 0);
	}

	return 0;
}

void jammin_state::dma_param0_w(uint8_t data) { m_dma_param0 = data; }
void jammin_state::dma_param1_w(uint8_t data) { m_dma_param1 = (m_dma_param1 >> 8) | (data << 8); }
void jammin_state::dma_param2_w(uint8_t data) { m_dma_param2 = (m_dma_param2 >> 8) | (data << 8); }
void jammin_state::dma_param3_w(uint8_t data) { m_dma_param3 = (m_dma_param3 >> 8) | (data << 8); }
void jammin_state::dma_param4_w(uint8_t data) { m_dma_param4 = (m_dma_param4 >> 8) | (data << 8); }

void jammin_state::dma_go_w(uint8_t data)
{
	if (m_dma_go != data)
	{
		if (data)
		{
			logerror("do DMA with params %02x %04x %04x %04x %04x\n", m_dma_param0, m_dma_param1, m_dma_param2, m_dma_param3, m_dma_param4 );
			// always triggered with 53 6900 41fc 7000 81fc
			// (a copy from 6900 to 7000?)
			for (int i = 0; i < 0x200; i++)
			{
				uint8_t dat = m_maincpu->space(AS_PROGRAM).read_byte(m_dma_param1 + i);
				m_maincpu->space(AS_PROGRAM).write_byte(m_dma_param3 + i, dat);
			}
		}
	}
	m_dma_go = data;
}

void jammin_state::pal1_w(uint8_t data)
{
	// palette banking
	logerror("pal1_w %02x\n", data );
	m_pal1 = data;
	m_bg_tilemap->mark_all_dirty();
}

void jammin_state::pal2_w(uint8_t data)
{
	// palette map select?
	logerror("pal2_w %02x\n", data );
	m_pal2 = data;
	m_bg_tilemap->mark_all_dirty();
}

void jammin_state::prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();

	map(0x4000, 0x4001).ram().rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));

	map(0x5000, 0x50ff).ram();

	map(0x6000, 0x6bff).ram();

	map(0x7000, 0x73ff).ram().share(m_spram); // sprites?
	map(0x7400, 0x77ff).ram().w(FUNC(jammin_state::bgram_w)).share(m_bgram);

	map(0x7800, 0x7800).w(FUNC(jammin_state::dma_param1_w)); // DMACTL
	map(0x7801, 0x7801).w(FUNC(jammin_state::dma_param2_w));
	map(0x7802, 0x7802).w(FUNC(jammin_state::dma_param3_w));
	map(0x7803, 0x7803).w(FUNC(jammin_state::dma_param4_w));

	map(0x7808, 0x7808).w(FUNC(jammin_state::dma_param0_w));

	map(0x7c00, 0x7c00).portr("IN0"); // player inputs
	map(0x7c80, 0x7c80).portr("IN1"); // player inputs
	map(0x7d07, 0x7d07).portr("IN2"); // Coin is in here
	map(0x7d80, 0x7d80).portr("IN3"); // unknown

	map(0x7d82, 0x7d82).nopw(); // VFLIP
	map(0x7d83, 0x7d83).nopw(); // MOCNTL
	map(0x7d84, 0x7d84).nopw(); // NMILAT
	map(0x7d85, 0x7d85).nopw().w(FUNC(jammin_state::dma_go_w));
	map(0x7d86, 0x7d86).nopw().w(FUNC(jammin_state::pal1_w));
	map(0x7d87, 0x7d87).nopw().w(FUNC(jammin_state::pal2_w));

	map(0x8000, 0xbfff).rom();
}

static INPUT_PORTS_START( jammin )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )  // acts as a drum, also start 2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // acts as a drum
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // acts as a drum
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) // acts as a drum
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) // acts as a drum, advances selection in menu
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) // acts as a drum
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) // acts as a drum
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) // acts as a drum
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )  // acts as a drum, also start 1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout tile_layout2 =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 8, 0 },
	{ 0,1,2,3,4,5,6,7, 16,17,18,19,20,21,22,23 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32 },
	16*32
};


static const gfx_layout tile_layout3 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60 },
	{ 0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};

static GFXDECODE_START( gfx_jammin )
	GFXDECODE_ENTRY( "tiles", 0, tile_layout, 0, 64 )
	GFXDECODE_ENTRY( "tiles2", 0, tile_layout2, 0, 64 )
	GFXDECODE_ENTRY( "tiles3", 0, tile_layout3, 0, 64 )
GFXDECODE_END

void jammin_state::machine_start()
{
	save_item(NAME(m_dma_param0));
	save_item(NAME(m_dma_param1));
	save_item(NAME(m_dma_param2));
	save_item(NAME(m_dma_param3));
	save_item(NAME(m_dma_param4));
	save_item(NAME(m_dma_go));
	save_item(NAME(m_pal1));
	save_item(NAME(m_pal2));
}

void jammin_state::machine_reset()
{
	m_dma_param0 = 0;
	m_dma_param1 = 0;
	m_dma_param2 = 0;
	m_dma_param3 = 0;
	m_dma_param4 = 0;
	m_dma_go = 0;
	m_pal1 = 0;
	m_pal2 = 0;
}

void jammin_state::jammin(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4'000'000);         // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &jammin_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(jammin_state::nmi_line_pulse));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(jammin_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_jammin);

	// wrong
	PALETTE(config, "palette").set_init(FUNC(jammin_state::pal_init)).set_entries(0x200);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, 14'318'181 / 4); // ?? used sys2 clock
	m_ym2151->add_route(0, "speaker", 0.60, 0);
	m_ym2151->add_route(1, "speaker", 0.60, 1);
}

ROM_START( jammin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// rebuilt from sources, the bytes for a table at 9b6c-9b87 were missing and may need to be reconstructed
	ROM_LOAD( "jammin.bin", 0x00000, 0xb853, CRC(449ce727) SHA1(83a96284072cd5fc3aae5ad327fc95ad90346954) )

	ROM_REGION( 0x1000, "tiles", 0 ) // backgrounds?
	ROM_LOAD( "jambak.pl0", 0x0000, 0x0800, CRC(af808d29) SHA1(cad060ee4e529f9a2ffa9675682b9e17bed4dffe) )
	ROM_LOAD( "jambak.pl1", 0x0800, 0x0800, CRC(43eaccef) SHA1(37e032b60c0bbaea81b55b4d137cb2d9d047e521) )

	// do these sprite regions need combining into 6bpp gfx?
	// the layouts are very different
	ROM_REGION( 0x2000, "tiles2", 0 ) // 2bpp of sprite data
	ROM_LOAD32_BYTE( "jammin.7c", 0x0000, 0x0800, CRC(82361b24) SHA1(ed070586296c329cb88e3dfc4741d591ec59fb8d) )
	ROM_LOAD32_BYTE( "jammin.7e", 0x0001, 0x0800, CRC(9563c301) SHA1(3947af64f3becf36afaacdb8c962bfccc236525d) )
	ROM_LOAD32_BYTE( "jammin.7d", 0x0002, 0x0800, CRC(0f02eb55) SHA1(b97ee07dbd71bdc698ac775a721f220afd6bb7cc) )
	ROM_LOAD32_BYTE( "jammin.7f", 0x0003, 0x0800, CRC(26c7f3b8) SHA1(a0a13ce692bcf40104099a4d9bb2c36aca885350) )

	ROM_REGION( 0x4000, "tiles3", 0 ) // 4bpp of sprite data
	ROM_LOAD16_WORD_SWAP( "jammin.int", 0x00000, 0x4000, CRC(0f9022de) SHA1(40f33dd7fcdc310c0eb93c3072b24f290247e974) )

	ROM_REGION( 0x200, "proms_lookup", 0 ) // lookup?
	ROM_LOAD( "mac2n.bin", 0x000, 0x0100, CRC(e8198448) SHA1(20fc8da7858daa56be758148e5e80f5de30533f9) )
	ROM_LOAD( "col2n.bin", 0x100, 0x0100, CRC(c5ded6e3) SHA1(21d172952f5befafec6fa93be5023f1df0eceb7d) )

	ROM_REGION( 0x400, "proms", 0 ) // colours?
	ROM_LOAD( "mac2e.bin", 0x000, 0x0100, CRC(65f57bc6) SHA1(8645c8291c7479ed093d64d3f9b19240d5cf8b4e) )
	ROM_LOAD( "mac2f.bin", 0x100, 0x0100, CRC(938955e5) SHA1(96accf365326e499898fb4d937d716df5792fade) )

	ROM_REGION( 0x400, "promsx", 0 ) // more colours, what are these for?
	ROM_LOAD( "col2e.bin", 0x000, 0x0100, CRC(d22fd797) SHA1(a21be0d280eb376dc600b28a15ece0f9d1cb6d42) )
	ROM_LOAD( "col2f.bin", 0x100, 0x0100, CRC(bf115ba7) SHA1(ecd12079c23ed73eed2056cad2c23e6bb19d803e) )
ROM_END

} // anonymous namespace


GAME( 1985, jammin,  0,    jammin, jammin,  jammin_state, empty_init, ROT90, "Atari", "Jammin' (prototype)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
