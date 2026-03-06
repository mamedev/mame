// license:BSD-3-Clause
// copyright-holders:

/*
Fruit Dream 2

PCB DE-0426

Z84C0008PEC CPU
LC3664BML-70 SRAM
52 Data East custom chip (Sprites)
77 Data East custom chip (?)
141 Data East custom chip (Tilemaps)
153 Data East custom chip (Alpha blending)
223 Data East custom chip (GFX)
T82C55AM-10 PPI
MAX695CPE Microprocessor Supervisory IC
YM2203C sound chip
28.0000 MHz XTAL
bank of 8 switches
*/

#include "emu.h"

#include "deco16ic.h"
#include "decocrpt.h"
#include "decospr.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class dfruit2_state : public driver_device
{
public:
	dfruit2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_deco_tilegen(*this, "tilegen"),
		m_sprgen(*this, "sprgen"),
		m_spriteram(*this, "spriteram", 0x1000, ENDIANNESS_BIG)
	{ }

	void dfruit2(machine_config &config) ATTR_COLD;

	void init_dfruit2() ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen;
	required_device<decospr_device> m_sprgen;

	memory_share_creator<u16> m_spriteram;

	uint16_t m_pf_control = 0;
	uint16_t m_pf1_data = 0;
	uint16_t m_pf2_data = 0;

	uint8_t spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, uint8_t data);
	uint8_t tilegen_pf1_data_r(offs_t offset);
	void tilegen_pf1_data_w(offs_t offset, uint8_t data);
	uint8_t tilegen_pf2_data_r(offs_t offset);
	void tilegen_pf2_data_w(offs_t offset, uint8_t data);
	void tilegen_control_w(offs_t offset, uint8_t data);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t dfruit2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_sprgen->set_flip_screen(true); // sprites are flipped relative to tilemaps

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x1000);

	return 0;
}

uint8_t dfruit2_state::spriteram_r(offs_t offset)
{
	int const offs = offset >> 1;

	if (!BIT(offset, 0))
		return(m_spriteram[offs] & 0xff);
	else
		return(m_spriteram[offs] >> 8);
}

void dfruit2_state::spriteram_w(offs_t offset, uint8_t data)
{
	int const offs = offset >> 1;

	if (!BIT(offset, 0))
		m_spriteram[offs] = (m_spriteram[offs] & 0xff00) | data;
	else
		m_spriteram[offs] = (m_spriteram[offs] & 0x00ff) | (data << 8);
}

uint8_t dfruit2_state::tilegen_pf1_data_r(offs_t offset)
{
	int const offs = offset >> 1;

	if (!BIT(offset, 0))
		return m_deco_tilegen->pf1_data_r(offs) & 0xff;
	else
		return m_deco_tilegen->pf1_data_r(offs) >> 8;
}

void dfruit2_state::tilegen_pf1_data_w(offs_t offset, uint8_t data)
{
	int const offs = offset >> 1;

	if (!BIT(offset, 0))
		m_pf1_data = (m_pf1_data & 0xff00) | data;
	else
		m_pf1_data = (m_pf1_data & 0x00ff) | (data << 8);

	m_deco_tilegen->pf1_data_w(offs, m_pf1_data, 0xffff);
}

uint8_t dfruit2_state::tilegen_pf2_data_r(offs_t offset)
{
	int const offs = offset >> 1;

	if (!BIT(offset, 0))
		return m_deco_tilegen->pf2_data_r(offs) & 0xff;
	else
		return m_deco_tilegen->pf2_data_r(offs) >> 8;
}

void dfruit2_state::tilegen_pf2_data_w(offs_t offset, uint8_t data)
{
	int const offs = offset >> 1;

	if (!BIT(offset, 0))
		m_pf2_data = (m_pf2_data & 0xff00) | data;
	else
		m_pf2_data = (m_pf2_data & 0x00ff) | (data << 8);

	m_deco_tilegen->pf2_data_w(offs, m_pf2_data, 0xffff);
}

void dfruit2_state::tilegen_control_w(offs_t offset, uint8_t data)
{
	int const offs = offset >> 1;

	if (!BIT(offset, 0))
		m_pf_control = (m_pf_control & 0xff00) | data;
	else
		m_pf_control = (m_pf_control & 0x00ff) | (data << 8);

	m_deco_tilegen->pf_control_w(offs, m_pf_control, 0xffff);
}

DECO16IC_BANK_CB_MEMBER(dfruit2_state::bank_callback)
{
	return (bank & 0xf0) << 8; // TODO: not verified
}


void dfruit2_state::program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram(); // WORK RAM CHECK NG if this range isn't mapped. TODO: probably battery backed by the MAX695CPE
	map(0xa000, 0xa003).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write)); // probably
	map(0xa004, 0xa005).rw("ym", FUNC(ym2203_device::read), FUNC(ym2203_device::write)); // probably
	// map(0xa008, 0xa008).r(); // watchdog?
	// map(0xa028, 0xa028).w(); // MAX695CPE??
	// map(0xa029, 0xa029).w(); // MAX695CPE??
	map(0xa02b, 0xa02b).lr8(NAME([]( ) -> uint8_t { return 0x00; })); // MAX695CPE?? VBLANK / IRQ?
	map(0xb000, 0xb00f).w(FUNC(dfruit2_state::tilegen_control_w)); // probably
	// map(0xb100, 0xb100).w();
	// map(0xb101, 0xb101).w();
	// map(0xb109, 0xb109).w();
	map(0xc000, 0xc1ff).ram(); // COLOR RAM CHECK NG if this range isn't mapped
	map(0xd000, 0xdfff).rw(FUNC(dfruit2_state::tilegen_pf1_data_r), FUNC(dfruit2_state::tilegen_pf1_data_w)); // SCREEN RAM CHECK NG if this range isn't mapped
	map(0xe000, 0xefff).rw(FUNC(dfruit2_state::tilegen_pf2_data_r), FUNC(dfruit2_state::tilegen_pf2_data_w));
	map(0xf000, 0xffff).rw(FUNC(dfruit2_state::spriteram_r), FUNC(dfruit2_state::spriteram_w)); // maybe not the whole range
}


static INPUT_PORTS_START( dfruit2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW:8" )
INPUT_PORTS_END


static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8
};

// TODO: wrong? see missing parts of the disclaimer text at 0x700-0x752 in the GFX viewer
static const gfx_layout _16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static GFXDECODE_START( gfx_dfruit2 ) // TODO
	GFXDECODE_ENTRY( "tiles", 0, tile_8x8_layout, 0, 32 )
	GFXDECODE_ENTRY( "tiles", 0, _16x16_layout, 0, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_dfruit2_spr ) // TODO
	GFXDECODE_ENTRY( "sprites", 0, _16x16_layout, 0, 32 )
GFXDECODE_END


void dfruit2_state::dfruit2(machine_config &config)
{
	Z80(config, m_maincpu, 28_MHz_XTAL / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &dfruit2_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(dfruit2_state::irq0_line_hold));

	i8255_device &ppi(I8255A(config, "ppi")); // initialized with 0x9b (mode 0 all reads). TODO: ports are just guessed
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.in_pb_callback().set_ioport("IN1");
	ppi.in_pc_callback().set_ioport("DSW");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(dfruit2_state::screen_update));

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 0x200 / 2); // TODO

	GFXDECODE(config, "gfxdecode", "palette", gfx_dfruit2);

	DECO16IC(config, m_deco_tilegen, 0); // TODO: everything
	m_deco_tilegen->set_pf1_size(DECO_64x32);
	m_deco_tilegen->set_pf2_size(DECO_64x32);
	m_deco_tilegen->set_pf1_col_bank(0x00);
	m_deco_tilegen->set_pf2_col_bank(0x10);
	m_deco_tilegen->set_pf1_col_mask(0x0f);
	m_deco_tilegen->set_pf2_col_mask(0x0f);
	m_deco_tilegen->set_bank1_callback(FUNC(dfruit2_state::bank_callback));
	m_deco_tilegen->set_bank2_callback(FUNC(dfruit2_state::bank_callback));
	m_deco_tilegen->set_pf12_8x8_bank(0);
	m_deco_tilegen->set_pf12_16x16_bank(1);
	m_deco_tilegen->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, "sprgen", 0, "palette", gfx_dfruit2_spr);

	SPEAKER(config, "mono").front_center();

	ym2203_device &ym(YM2203(config, "ym", 28_MHz_XTAL / 8));
	ym.add_route(ALL_OUTPUTS, "mono", 1.00); // divider not verified
}


ROM_START( dfruit2 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "nn_002.b4", 0x00000, 0x40000, CRC(bab4fa47) SHA1(d9b4ecaf72ced54a1a7f1de7036efe3d939306cf) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "nn_010.b10", 0x00000, 0x40000, CRC(1ae3c63f) SHA1(c9940c50bfdacf9bbaf823e48abe249b992f74d9) )
	ROM_LOAD( "nn_020.b11", 0x40000, 0x40000, CRC(8e5ebdb8) SHA1(a44c055cd33921561f4b2e1906186d16bfc64783) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "nn_030.h2", 0x00000, 0x80000, CRC(8f9183af) SHA1(e756306755aed15f40cffcdeedea030c5da9ffb3) )
ROM_END


void dfruit2_state::init_dfruit2()
{
	deco56_decrypt_gfx(machine(), "tiles"); // DECO 141
}

} // anonymous namespace


GAME( 1994, dfruit2, 0, dfruit2, dfruit2, dfruit2_state, init_dfruit2, ROT0, "Nippon Data Kiki / Star Fish", "Fruit Dream II", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
