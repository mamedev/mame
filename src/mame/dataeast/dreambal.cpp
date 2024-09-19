// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Dream Ball

PCB DE-0386-2
(also has DEC-22V0 like many Data East PCBS)

Customs
104 (I/O, Protection)
59 (68000 CPU)
141 (Tilemap GFX)
71 (usually sprites? or mixer?)
HD63B50P in I/O section (maybe hopper comms?)

--
todo:
emulate hopper
lamps?


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "deco104.h"
#include "decocrpt.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "deco16ic.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class dreambal_state : public driver_device
{
public:
	dreambal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_deco104(*this, "ioprot104"),
		m_deco_tilegen(*this, "tilegen"),
		m_eeprom(*this, "eeprom")
	{ }

	void dreambal(machine_config &config);

	void init_dreambal();

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<deco104_device> m_deco104;
	required_device<deco16ic_device> m_deco_tilegen;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update_dreambal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);

	uint16_t dreambal_protection_region_0_104_r(offs_t offset);
	void dreambal_protection_region_0_104_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void dreambal_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0)
	{
		if (data&0xfff8)
		{
			logerror("dreambal_eeprom_w unhandled data %04x %04x\n",data&0x0fff8, mem_mask);
		}

		if (ACCESSING_BITS_0_7)
		{
			m_eeprom->clk_write(data &0x2 ? ASSERT_LINE : CLEAR_LINE);
			m_eeprom->di_write(data &0x1);
			m_eeprom->cs_write(data&0x4 ? ASSERT_LINE : CLEAR_LINE);
		}
	}
	void dreambal_map(address_map &map) ATTR_COLD;
};


uint32_t dreambal_state::screen_update_dreambal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen->pf_control_r(0);

	flip_screen_set(BIT(flip, 7));
	m_deco_tilegen->pf_update(nullptr, nullptr);

	bitmap.fill(0, cliprect); /* not Confirmed */
	screen.priority().fill(0);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
	return 0;
}


uint16_t dreambal_state::dreambal_protection_region_0_104_r(offs_t offset)
{
	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t data = m_deco104->read_data( deco146_addr, cs );
	return data;
}

void dreambal_state::dreambal_protection_region_0_104_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_deco104->write_data( deco146_addr, data, mem_mask, cs );
}

void dreambal_state::dreambal_map(address_map &map)
{
//map.unmap_value_high();
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x101000, 0x101fff).ram();
	map(0x102000, 0x102fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x103000, 0x103fff).ram();

	map(0x120000, 0x123fff).ram();
	map(0x140000, 0x1403ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");

	map(0x160000, 0x163fff).rw(FUNC(dreambal_state::dreambal_protection_region_0_104_r), FUNC(dreambal_state::dreambal_protection_region_0_104_w)).share("prot16ram"); /* Protection device */

	map(0x161000, 0x16100f).w(m_deco_tilegen, FUNC(deco16ic_device::pf_control_w));


	map(0x180001, 0x180001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x162000, 0x162001).nopw(); // writes 0003 on startup
	map(0x163000, 0x163001).nopw(); // something on bit 1
	map(0x164000, 0x164001).nopw(); // something on bit 1

	map(0x165000, 0x165001).w(FUNC(dreambal_state::dreambal_eeprom_w)); // EEP Write?

	map(0x16c002, 0x16c00d).nopw(); // writes 0000 to 0005 on startup
	map(0x1a0000, 0x1a0003).nopw(); // RS-232C status / data ports (byte access)
}


static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout tile_16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ 256,257,258,259,260,261,262,263,0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	32*16
};


static GFXDECODE_START( gfx_dreambal )
	GFXDECODE_ENTRY( "gfx1", 0, tile_8x8_layout,     0x000, 32 )    /* Tiles (8x8) */
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x16_layout,   0x000, 32 )    /* Tiles (16x16) */
GFXDECODE_END

static INPUT_PORTS_START( dreambal )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // currently causes hopper error
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) // fold?
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Double Up")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Small")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5")
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Start / Proceed")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Big")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4")
	PORT_SERVICE( 0x2000, IP_ACTIVE_LOW ) // only works if there are 0 credits
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0004, 0x0004, "3" ) // freeze / vbl?
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

DECO16IC_BANK_CB_MEMBER(dreambal_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

void dreambal_state::machine_start()
{
}

void dreambal_state::machine_reset()
{
}

// xtals = 28.000, 9.8304
void dreambal_state::dreambal(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 28000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dreambal_state::dreambal_map);
	m_maincpu->set_vblank_int("screen", FUNC(dreambal_state::irq6_line_hold)); // 5 valid too?

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(dreambal_state::screen_update_dreambal));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 0x400/2);
	GFXDECODE(config, "gfxdecode", "palette", gfx_dreambal);

	EEPROM_93C46_16BIT(config, "eeprom");  // 93lc46b

	DECO104PROT(config, m_deco104, 0);
	m_deco104->port_a_cb().set_ioport("INPUTS");
	m_deco104->port_b_cb().set_ioport("SYSTEM");
	m_deco104->port_c_cb().set_ioport("DSW");

	DECO16IC(config, m_deco_tilegen, 0);
	m_deco_tilegen->set_pf1_size(DECO_64x32);
	m_deco_tilegen->set_pf2_size(DECO_64x32);
	m_deco_tilegen->set_pf1_col_bank(0x00);
	m_deco_tilegen->set_pf2_col_bank(0x10);
	m_deco_tilegen->set_pf1_col_mask(0x0f);
	m_deco_tilegen->set_pf2_col_mask(0x0f);
	m_deco_tilegen->set_bank1_callback(FUNC(dreambal_state::bank_callback));
	m_deco_tilegen->set_bank2_callback(FUNC(dreambal_state::bank_callback));
	m_deco_tilegen->set_pf12_8x8_bank(0);
	m_deco_tilegen->set_pf12_16x16_bank(1);
	m_deco_tilegen->set_gfxdecode_tag("gfxdecode");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 9830400/8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.00);
}



ROM_START( dreambal )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mm_00-2.1c",    0x000000, 0x020000, CRC(257f6ad1) SHA1(7b232ce2d503e6f21286176974f6b74052f76d07) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "mm_01-1.12b",    0x00000, 0x80000, CRC(dc9cc708) SHA1(03b8e6aa37e0107514a2498849208d2bd51a4163) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "mm_01-1.12f",    0x00000, 0x20000, CRC(4f134be7) SHA1(b83230cc62bde55be736fd604af23f927706a770) )
ROM_END

void dreambal_state::init_dreambal()
{
	deco56_decrypt_gfx(machine(), "gfx1");
}

} // anonymous namespace


// Ver 2.4 JPN 93.12.02
GAME( 1993, dreambal, 0, dreambal, dreambal, dreambal_state, init_dreambal, ROT0, "NDK / Data East", "Dream Ball (Japan V2.4)", MACHINE_SUPPORTS_SAVE ) // copyright shows NDK, board is Data East, code seems Data East-like too
