// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Double Wings
Mitchell 1993

This game runs on Data East hardware.

PCB Layout
----------

S-NK-3220
DEC-22VO
|---------------------------------------------|
|MB3730 C3403    32.22MHz           MBE-01.16A|
|  Y3014B  KP_03-.16H       77                |
|           M6295                   MBE-00.14A|
|  YM2151                            |------| |
|          Z80      CXK5864          |      | |
| VOL                       VG-02.11B|  52  | |
|        LH5168     CXK5864          |      | |
|                                    |------| |
|                  |------|              28MHz|
|J       KP_02-.10H|      |                   |
|A                 | 141  |         CXK5814   |
|M       MBE-02.8H |      |                   |
|M                 |      |         CXK5814   |
|A                 |------|                   |
|                                   CXK5814   |
|                 KP_01-.5D                   |
|                                   CXK5814   |
|                 CXK5864                     |
| |----|          KP_00-.3D         |------|  |
| |104 |                            | 102  |  |
| |    |          CXK5864           |      |  |
| |----|                            |      |  |
|SW2 SW1 VG-01.1H VG-00.1F          |------|  |
|---------------------------------------------|
Notes:
       102     - Custom encrypted 68000 CPU. Clock 14.000MHz [28/2]
       Z80     - Toshiba TMPZ84C000AP-6 Z80 CPU. Clock 3.58MHz [32.22/9]
       YM2151  - Yamaha YM2151 FM Operator Type-M (OPM) sound chip. Clock 3.58MHz [32.22/9]
       M6295   - Oki M6295 4-channel mixing ADPCM LSI. Clock 1.000MHz [28/28]. Pin 7 HIGH
       LH6168  - Sharp LH6168 8kx8 SRAM (DIP28)
       CXK5814 - Sony CXK5816 2kx8 SRAM (DIP24)
       CXK5864 - Sony CXK5864 8kx8 SRAM (DIP28)
       VG-*    - MMI PAL16L8 (DIP20)
       SW1/SW2 - 8-position DIP switch
       HSync   - 15.6250kHz
       VSync   - 58.4443Hz

       Other DATA EAST Chips
       --------------------------------------
       DATA EAST 52  9235EV 205941 VC5259-0001 JAPAN   (Sprite Generator IC, 128 pin PQFP)
       DATA EAST 102 (M) DATA EAST 250 JAPAN           (Encrypted 68000 CPU, 128 Pin PQFP)
       DATA EAST 141 24220F008                         (Tile Generator IC, 160 pin PQFP)
       DATA EAST 104 L7A0717 9143 (M) DATA EAST        (IO/Protection, 100 pin PQFP)
       Small surface-mounted chip with number scratched off (28 pin SOP), but has number 9303K9200
       A similar chip exists on Capt. America PCB and has the number 77 on it. Possibly the same chip?



 - Main program writes two commands to soundlatch without pause in some places. Should the 104 custom
   chip be handling this through an internal FIFO?
 - should sprites be buffered, is the Deco '77' a '71' or similar?

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "deco102.h"
#include "deco104.h"
#include "decocrpt.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "deco16ic.h"
#include "decospr.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class dblewing_state : public driver_device
{
public:
	dblewing_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_pf_rowscroll(*this, "pf%u_rowscroll", 1U),
		m_spriteram(*this, "spriteram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_deco_tilegen(*this, "tilegen"),
		m_deco104(*this, "ioprot"),
		m_sprgen(*this, "spritegen"),
		m_soundlatch_pending(false)
	{ }

	void dblewing(machine_config &config);

	void init_dblewing();

private:
	/* memory pointers */
	required_shared_ptr_array<uint16_t, 2> m_pf_rowscroll;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_decrypted_opcodes;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco16ic_device> m_deco_tilegen;
	required_device<deco104_device> m_deco104;
	required_device<decospr_device> m_sprgen;

	uint8_t irq_latch_r();
	void soundlatch_irq_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);

	uint16_t ioprot_r(offs_t offset);
	void ioprot_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void dblewing_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void sound_io(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	bool m_soundlatch_pending;
};


uint32_t dblewing_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen->pf_control_r(0);

	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(BIT(flip, 7));
	m_deco_tilegen->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);

	bitmap.fill(0, cliprect); /* not Confirmed */
	screen.priority().fill(0);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}

uint16_t dblewing_state::ioprot_r(offs_t offset)
{
	int const real_address = 0 + (offset *2);
	int const deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t const data = m_deco104->read_data( deco146_addr, cs );
	return data;
}

void dblewing_state::ioprot_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const real_address = 0 + (offset *2);
	int const deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_deco104->write_data( deco146_addr, data, mem_mask, cs );
}

void dblewing_state::soundlatch_irq_w(int state)
{
	m_soundlatch_pending = bool(state);
}

void dblewing_state::dblewing_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x100000, 0x100fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x102000, 0x102fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x104000, 0x104fff).ram().share(m_pf_rowscroll[0]);
	map(0x106000, 0x106fff).ram().share(m_pf_rowscroll[1]);

//  map(0x280000, 0x2807ff).rw("ioprot104", FUNC(deco104_device::dblewing_prot_r), FUNC(deco104_device::dblewing_prot_w)).share("prot16ram");
	map(0x280000, 0x283fff).rw(FUNC(dblewing_state::ioprot_r), FUNC(dblewing_state::ioprot_w)).share("prot16ram"); /* Protection device */

	map(0x284000, 0x284001).ram();
	map(0x288000, 0x288001).ram();
	map(0x28c000, 0x28c00f).ram().w(m_deco_tilegen, FUNC(deco16ic_device::pf_control_w));
	map(0x300000, 0x3007ff).ram().share("spriteram");
	map(0x320000, 0x3207ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xff0000, 0xff3fff).mirror(0xc000).ram();
}

void dblewing_state::decrypted_opcodes_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().share("decrypted_opcodes");
}

uint8_t dblewing_state::irq_latch_r()
{
	// bit 0: irq type (0 = latch, 1 = ym)
	return m_soundlatch_pending ? 0 : 1;
}

void dblewing_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa001).rw("ymsnd", FUNC(ym2151_device::status_r), FUNC(ym2151_device::write));
	map(0xb000, 0xb000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc000, 0xc000).r(m_deco104, FUNC(deco104_device::soundlatch_r));
	map(0xd000, 0xd000).r(FUNC(dblewing_state::irq_latch_r)); //timing? sound latch?
	map(0xf000, 0xf000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void dblewing_state::sound_io(address_map &map)
{
	map(0x0000, 0xffff).rom().region("audiocpu", 0);
}


static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*16
};

static const gfx_layout tile_16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ STEP8(8*2*16,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	32*16
};


static GFXDECODE_START( gfx_dblewing )
	GFXDECODE_ENTRY( "tiles", 0, tile_8x8_layout,   0x000, 32 )    /* Tiles (8x8) */
	GFXDECODE_ENTRY( "tiles", 0, tile_16x16_layout, 0x000, 32 )    /* Tiles (16x16) */
GFXDECODE_END

static GFXDECODE_START( gfx_dblewing_spr )
	GFXDECODE_ENTRY( "sprites", 0, tile_16x16_layout, 0x200, 32 )    /* Sprites (16x16) */
GFXDECODE_END

static INPUT_PORTS_START( dblewing )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW")
		/* 16bit - These values are for Dip Switch #1 */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Region ) ) PORT_DIPLOCATION("SW1:8") /*Manual says "don't change this" */
	PORT_DIPSETTING(      0x0080, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Korea ) )
	/* 16bit - These values are for Dip Switch #2 */
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING( 0x0200, "1" )
	PORT_DIPSETTING( 0x0100, "2" )
	PORT_DIPSETTING( 0x0300, "3" )
	PORT_DIPSETTING( 0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, "Every 100,000" )
	PORT_DIPSETTING(      0x3000, "Every 150,000" )
	PORT_DIPSETTING(      0x1000, "Every 300,000" )
	PORT_DIPSETTING(      0x0000, "250,000 Only" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

DECO16IC_BANK_CB_MEMBER(dblewing_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

DECOSPR_PRIORITY_CB_MEMBER(dblewing_state::pri_callback)
{
	return 0; // sprites always on top?
}

void dblewing_state::dblewing(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(28'000'000)/2);   /* DE102 */
	m_maincpu->set_addrmap(AS_PROGRAM, &dblewing_state::dblewing_map);
	m_maincpu->set_addrmap(AS_OPCODES, &dblewing_state::decrypted_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(dblewing_state::irq6_line_hold));

	Z80(config, m_audiocpu, XTAL(32'220'000)/9);
	m_audiocpu->set_addrmap(AS_PROGRAM, &dblewing_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &dblewing_state::sound_io);

	INPUT_MERGER_ANY_HIGH(config, "soundirq").output_handler().set_inputline(m_audiocpu, 0);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58.443);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(dblewing_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 2048/2);
	GFXDECODE(config, "gfxdecode", "palette", gfx_dblewing);

	DECO16IC(config, m_deco_tilegen, 0);
	m_deco_tilegen->set_pf1_size(DECO_64x32);
	m_deco_tilegen->set_pf2_size(DECO_64x32);
	m_deco_tilegen->set_pf1_col_bank(0x00);
	m_deco_tilegen->set_pf2_col_bank(0x10);
	m_deco_tilegen->set_pf1_col_mask(0x0f);
	m_deco_tilegen->set_pf2_col_mask(0x0f);
	m_deco_tilegen->set_bank1_callback(FUNC(dblewing_state::bank_callback));
	m_deco_tilegen->set_bank2_callback(FUNC(dblewing_state::bank_callback));
	m_deco_tilegen->set_pf12_8x8_bank(0);
	m_deco_tilegen->set_pf12_16x16_bank(1);
	m_deco_tilegen->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen, 0, "palette", gfx_dblewing_spr);
	m_sprgen->set_pri_callback(FUNC(dblewing_state::pri_callback));

	DECO104PROT(config, m_deco104, 0);
	m_deco104->port_a_cb().set_ioport("INPUTS");
	m_deco104->port_b_cb().set_ioport("SYSTEM");
	m_deco104->port_c_cb().set_ioport("DSW");
	m_deco104->set_interface_scramble_interleave();
	m_deco104->set_use_magic_read_address_xor(true);
	m_deco104->soundlatch_irq_cb().set(FUNC(dblewing_state::soundlatch_irq_w));
	m_deco104->soundlatch_irq_cb().append("soundirq", FUNC(input_merger_device::in_w<0>));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(32'220'000)/9));
	ymsnd.irq_handler().set("soundirq", FUNC(input_merger_device::in_w<1>));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki", XTAL(28'000'000)/28, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.00);
}


ROM_START( dblewing )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE102 code (encrypted) */
	ROM_LOAD16_BYTE( "kp_00-.3d",    0x000001, 0x040000, CRC(547dc83e) SHA1(f6f96bd4338d366f06df718093f035afabc073d1) )
	ROM_LOAD16_BYTE( "kp_01-.5d",    0x000000, 0x040000, CRC(7a210c33) SHA1(ced89140af6d6a1bc0ffb7728afca428ed007165) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // sound cpu
	ROM_LOAD( "kp_02-.10h",   0x000000, 0x010000, CRC(def035fa) SHA1(fd50314e5c94c25df109ee52c0ce701b0ff2140c) )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "mbe-02.8h",    0x000000, 0x100000, CRC(5a6d3ac5) SHA1(738bb833e2c5d929ac75fe4e69ee0af88197d8a6) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mbe-00.14a",   0x000000, 0x100000, CRC(e33f5c93) SHA1(720904b54d02dace2310ac6bd07d5ed4bc4fd69c) )
	ROM_LOAD( "mbe-01.16a",   0x100000, 0x100000, CRC(ef452ad7) SHA1(7fe49123b5c2778e46104eaa3a2104ce09e05705) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "kp_03-.16h",   0x000000, 0x020000, CRC(5d7f930d) SHA1(ad23aa804ea3ccbd7630ade9b53fc3ea2718a6ec) )
	ROM_RELOAD(               0x020000, 0x020000 )

	ROM_REGION( 0x600, "plds", 0 )
	ROM_LOAD( "pal16l8-vg-00.1f",  0x000, 0x117, CRC(8c2849e5) SHA1(72c5142dd78ea7d009229bad9f1a5651eec1e858) )
	ROM_LOAD( "pal16l8-vg-01.1h",  0x200, 0x117, CRC(04b0bab6) SHA1(6b1ad69506b385eeeac6cb952e166304bf6fbb40) )
	ROM_LOAD( "pal16r8-vg-02.11b", 0x400, 0x117, NO_DUMP )
ROM_END

ROM_START( dblewinga )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE102 code (encrypted) */
	ROM_LOAD16_BYTE( "2.3d",    0x000001, 0x040000, CRC(1e6b0653) SHA1(000fbf8bb07688fe1dd16fe8f33c376871f1860f) )
	ROM_LOAD16_BYTE( "1.5d",    0x000000, 0x040000, CRC(4d537dc9) SHA1(9149e6de11ed9aa1db58c72e4ba6b2b309c6978b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // sound cpu
	ROM_LOAD( "kp_02-.10h",   0x000000, 0x010000, CRC(def035fa) SHA1(fd50314e5c94c25df109ee52c0ce701b0ff2140c) )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "mbe-02.8h",    0x000000, 0x100000, CRC(5a6d3ac5) SHA1(738bb833e2c5d929ac75fe4e69ee0af88197d8a6) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mbe-00.14a",   0x000000, 0x100000, CRC(e33f5c93) SHA1(720904b54d02dace2310ac6bd07d5ed4bc4fd69c) )
	ROM_LOAD( "mbe-01.16a",   0x100000, 0x100000, CRC(ef452ad7) SHA1(7fe49123b5c2778e46104eaa3a2104ce09e05705) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "kp_03-.16h",   0x000000, 0x020000, CRC(5d7f930d) SHA1(ad23aa804ea3ccbd7630ade9b53fc3ea2718a6ec) )
	ROM_RELOAD(               0x020000, 0x020000 )

	ROM_REGION( 0x600, "plds", 0 )
	ROM_LOAD( "pal16l8-vg-00.1f",  0x000, 0x117, CRC(8c2849e5) SHA1(72c5142dd78ea7d009229bad9f1a5651eec1e858) )
	ROM_LOAD( "pal16l8-vg-01.1h",  0x200, 0x117, CRC(04b0bab6) SHA1(6b1ad69506b385eeeac6cb952e166304bf6fbb40) )
	ROM_LOAD( "pal16r8-vg-02.11b", 0x400, 0x117, NO_DUMP )
ROM_END

/*
The most noticeable difference with the set below is that it doesn't use checkpoints, but respawns you when you die.
Checkpoints were more common in Japan, so this is likely to be an export version.
*/
ROM_START( dblewingb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE102 code (encrypted) */
	ROM_LOAD16_BYTE( "17.3d",    0x000001, 0x040000, CRC(3a7ba822) SHA1(726db048ae3ab45cca45f631ad1f04b5cbc7f741) )
	ROM_LOAD16_BYTE( "18.5d",    0x000000, 0x040000, CRC(e5f5f004) SHA1(4bd40ef88027554a0328df1cf6f1c9c975a7a73f) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // sound cpu
	ROM_LOAD( "kp_02-.10h",   0x000000, 0x010000, CRC(def035fa) SHA1(fd50314e5c94c25df109ee52c0ce701b0ff2140c) )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "mbe-02.8h",    0x000000, 0x100000, CRC(5a6d3ac5) SHA1(738bb833e2c5d929ac75fe4e69ee0af88197d8a6) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mbe-00.14a",   0x000000, 0x100000, CRC(e33f5c93) SHA1(720904b54d02dace2310ac6bd07d5ed4bc4fd69c) )
	ROM_LOAD( "mbe-01.16a",   0x100000, 0x100000, CRC(ef452ad7) SHA1(7fe49123b5c2778e46104eaa3a2104ce09e05705) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "kp_03-.16h",   0x000000, 0x020000, CRC(5d7f930d) SHA1(ad23aa804ea3ccbd7630ade9b53fc3ea2718a6ec) )
	ROM_RELOAD(               0x020000, 0x020000 )

	ROM_REGION( 0x600, "plds", 0 )
	ROM_LOAD( "pal16l8-vg-00.1f",  0x000, 0x117, CRC(8c2849e5) SHA1(72c5142dd78ea7d009229bad9f1a5651eec1e858) )
	ROM_LOAD( "pal16l8-vg-01.1h",  0x200, 0x117, CRC(04b0bab6) SHA1(6b1ad69506b385eeeac6cb952e166304bf6fbb40) )
	ROM_LOAD( "pal16r8-vg-02.11b", 0x400, 0x117, NO_DUMP )
ROM_END

void dblewing_state::init_dblewing()
{
	deco56_decrypt_gfx(machine(), "tiles");
	deco102_decrypt_cpu((uint16_t *)memregion("maincpu")->base(), m_decrypted_opcodes, 0x80000, 0x399d, 0x25, 0x3d);

	save_item(NAME(m_soundlatch_pending));
}

} // anonymous namespace


GAME( 1993, dblewing,  0,        dblewing, dblewing, dblewing_state, init_dblewing, ROT90, "Mitchell", "Double Wings (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, dblewinga, dblewing, dblewing, dblewing, dblewing_state, init_dblewing, ROT90, "Mitchell", "Double Wings (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, dblewingb, dblewing, dblewing, dblewing, dblewing_state, init_dblewing, ROT90, "Mitchell", "Double Wings (Asia)",  MACHINE_SUPPORTS_SAVE )
