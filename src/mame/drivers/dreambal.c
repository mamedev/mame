/* Dream Ball

PCB DE-0386-2
(also has DEC-22V0 like many Data East PCBS)

Customs
104 (I/O, Protection)
59 (68000 CPU)
141 (Tilemap GFX)
71 (usually sprites? or mixer?)

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/decocrpt.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decospr.h"

class dreambal_state : public driver_device
{
public:
	dreambal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)/* ,

		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_spriteram(*this, "spriteram"),
		m_sprgen(*this, "spritegen")
*/
	{ }

	/* memory pointers */
	/*
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	required_shared_ptr<UINT16> m_pf2_rowscroll;
	required_shared_ptr<UINT16> m_spriteram;
	optional_device<decospr_device> m_sprgen;
	*/

	/* devices */
	cpu_device *m_maincpu;
	device_t *m_deco_tilegen1;

	DECLARE_DRIVER_INIT(dreambal);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_dreambal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

#if 0
UINT16 dreambal_pri_callback(UINT16 x)
{
	UINT16 pri = (x & 0xc000); // 2 bits or 1?
	switch (pri & 0xc000)
	{
		case 0x0000: pri = 0; break;
		case 0x4000: pri = 0xf0; break;
		case 0x8000: pri = 0xf0 | 0xcc; break;
		case 0xc000: pri = 0xf0 | 0xcc; break; /*  or 0xf0|0xcc|0xaa ? */
	}

	return pri;
}
#endif


UINT32 dreambal_state::screen_update_dreambal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#if 0
	address_space &space = generic_space();
	UINT16 flip = deco16ic_pf_control_r(m_deco_tilegen1, space, 0, 0xffff);

	flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(m_deco_tilegen1, m_pf1_rowscroll, m_pf2_rowscroll);

	bitmap.fill(0, cliprect); /* not Confirmed */
	machine().priority_bitmap.fill(0);

	deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, 0, 2);
	deco16ic_tilemap_1_draw(m_deco_tilegen1, bitmap, cliprect, 0, 4);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
#endif
	return 0;
}


static ADDRESS_MAP_START( dreambal_map, AS_PROGRAM, 16, dreambal_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
ADDRESS_MAP_END


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

/*
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 24,8,16,0 },
	{ 512,513,514,515,516,517,518,519, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32,10*32,11*32,12*32,13*32,14*32,15*32},
	32*32
};
*/

static GFXDECODE_START( dreambal )
	GFXDECODE_ENTRY( "gfx1", 0, tile_8x8_layout,     0x000, 32 )    /* Tiles (8x8) */
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x16_layout,   0x000, 32 )    /* Tiles (16x16) */
//	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,        0x200, 32 )    /* Sprites (16x16) */
GFXDECODE_END

static INPUT_PORTS_START( dreambal )
	PORT_START("UNK")
	PORT_DIPNAME( 0x0001, 0x0001, "2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
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
INPUT_PORTS_END

static int dreambal_bank_callback( const int bank )
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

static const deco16ic_interface dreambal_deco16ic_tilegen1_intf =
{
	"screen",
	0, 1,
	0x0f, 0x0f,     /* trans masks (default values) */
	0, 16, /* color base (default values) */
	0x0f, 0x0f, /* color masks (default values) */
	dreambal_bank_callback,
	dreambal_bank_callback,
	0,1,
};

void dreambal_state::machine_start()
{
	m_maincpu = machine().device<cpu_device>("maincpu");
	m_deco_tilegen1 = machine().device("tilegen1");


}

void dreambal_state::machine_reset()
{

}

// xtals = 28.000, 9.8304
static MACHINE_CONFIG_START( dreambal, dreambal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 28000000/2)
	MCFG_CPU_PROGRAM_MAP(dreambal_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", dreambal_state,  irq6_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(dreambal_state, screen_update_dreambal)

	MCFG_PALETTE_LENGTH(4096)
	MCFG_GFXDECODE(dreambal)

	MCFG_DECO16IC_ADD("tilegen1", dreambal_deco16ic_tilegen1_intf)
/*
	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 2);
	decospr_device::set_pri_callback(*device, dreambal_pri_callback);
*/
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 9830400/8, OKIM6295_PIN7_HIGH)
	                          
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END



ROM_START( dreambal )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mm_00-2.1c",    0x000001, 0x020000, CRC(257f6ad1) SHA1(7b232ce2d503e6f21286176974f6b74052f76d07) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "mm_01-1.12b",    0x00000, 0x80000, CRC(dc9cc708) SHA1(03b8e6aa37e0107514a2498849208d2bd51a4163) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "mm_01-1.12f",    0x00000, 0x20000, CRC(4f134be7) SHA1(b83230cc62bde55be736fd604af23f927706a770) )

	ROM_REGION( 0x80000, "eeprom", 0 ) /* EEPROM */
	ROM_LOAD( "93lc46b.8f",    0x00000, 0x80, CRC(5ba5403f) SHA1(cad63d704d81db5c45826d485c5e3a0679fba152) )
	
ROM_END

DRIVER_INIT_MEMBER(dreambal_state,dreambal)
{
	deco56_decrypt_gfx(machine(), "gfx1");
}


GAME( 199?, dreambal, 0,     dreambal, dreambal, dreambal_state,  dreambal,  ROT0, "Data East", "Dream Ball", GAME_NOT_WORKING )
