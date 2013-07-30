/*


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

class casanova_state : public driver_device
{
public:
	casanova_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> m_maincpu;

	virtual void video_start()
	{

	}

	UINT32 screen_update_casanova(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
	{
		return 0;
	}


	DECLARE_READ16_MEMBER( unk_casanova_28x_r ) { return 0x0000; }

	DECLARE_WRITE16_MEMBER( unk_casanova_40x_w ) { }
	DECLARE_WRITE16_MEMBER( unk_casanova_48x_w ) { }


	DECLARE_READ16_MEMBER( unk_casanova_50x_r ) { return 0x0000; }
	DECLARE_READ16_MEMBER( unk_casanova_58x_r ) { return 0x0000; }
	DECLARE_READ16_MEMBER( unk_casanova_60x_r ) { return 0x0000; }

	DECLARE_WRITE16_MEMBER( unk_casanova_80x_w ) { }
};



static ADDRESS_MAP_START( casanova_map, AS_PROGRAM, 16, casanova_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x200000, 0x202fff) AM_RAM // tilemaps

	AM_RANGE(0x280000, 0x280001) AM_READ(unk_casanova_28x_r)

	AM_RANGE(0x300000, 0x3005ff) AM_RAM_WRITE(paletteram_xRRRRRGGGGGBBBBB_word_w) AM_SHARE("paletteram")

	AM_RANGE(0x400000, 0x400001) AM_WRITE(unk_casanova_40x_w)
	AM_RANGE(0x480000, 0x480001) AM_WRITE(unk_casanova_48x_w)

	AM_RANGE(0x500000, 0x500001) AM_READ(unk_casanova_50x_r)
	AM_RANGE(0x580000, 0x580001) AM_READ(unk_casanova_58x_r)
	AM_RANGE(0x600000, 0x600001) AM_READ(unk_casanova_60x_r)

	AM_RANGE(0x800000, 0x800001) AM_WRITE(unk_casanova_80x_w)

ADDRESS_MAP_END





static INPUT_PORTS_START( casanova )
INPUT_PORTS_END


static const gfx_layout casanova_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};

static const gfx_layout casanova16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	16*128
};

static GFXDECODE_START( casanova )
	GFXDECODE_ENTRY( "gfx1", 0, casanova16_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "gfx2", 0, casanova_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "gfx3", 0, casanova_layout,   0x0, 2  )
GFXDECODE_END



static MACHINE_CONFIG_START( casanova, casanova_state )

	MCFG_CPU_ADD("maincpu", M68000, 10000000 )
	MCFG_CPU_PROGRAM_MAP(casanova_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", casanova_state,  irq4_line_hold)

	MCFG_GFXDECODE(casanova)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(casanova_state, screen_update_casanova)

	MCFG_PALETTE_LENGTH(0x300)


//	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

//	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
//	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.47)
//	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.47)
MACHINE_CONFIG_END


ROM_START( casanova )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "casanova.u7", 0x00001, 0x40000, CRC(869c2bf2) SHA1(58d349fa92880b20e9a58c576002972e46cd7eb2) )
	ROM_LOAD16_BYTE( "casanova.u8", 0x00000, 0x40000, CRC(9df77f4b) SHA1(e2da1440406be715b349c9bf5263cb7bd8ef69d9) )

	ROM_REGION( 0x0c0000, "oki", 0 ) /* Samples */
	ROM_LOAD( "casanova.su2", 0x00000, 0x80000, CRC(84a8320e) SHA1(4d0b4120174b2aa726db8e324d5614e3f0cefe95) )
	ROM_LOAD( "casanova.su3", 0x80000, 0x40000, CRC(334a2d1a) SHA1(d3eb5627a711a78c52a1fdd573a7f91442ccfa49) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "casanova.u23", 0x000000, 0x80000, CRC(4bd4e5b1) SHA1(13759d086ef2dba26129022bade12be11b81258e) )
	ROM_LOAD32_BYTE( "casanova.u25", 0x000001, 0x80000, CRC(5461811b) SHA1(03301c836ba378e527867de25ee15abd3a0434ac))
	ROM_LOAD32_BYTE( "casanova.u27", 0x000002, 0x80000, CRC(dd178379) SHA1(990109db9d0ce693cf7371109cb0d4745b8dde59))
	ROM_LOAD32_BYTE( "casanova.u29", 0x000003, 0x80000, CRC(36469f9e) SHA1(d4603bf99aef953e2eb49c1862d66961246e88c2) )
	ROM_LOAD32_BYTE( "casanova.u81", 0x200000, 0x80000, CRC(9eafd37d) SHA1(bc9e7a035849f23da48c9d923188c61188d93c43) )
	ROM_LOAD32_BYTE( "casanova.u83", 0x200001, 0x80000, CRC(9d4ce407) SHA1(949c7f329bd348beff4f14ac7b506c8aef212ad8) )
	ROM_LOAD32_BYTE( "casanova.u85", 0x200002, 0x80000, CRC(113c6e3a) SHA1(e90d78c4415d244004734a481501f8040f8aa468) )
	ROM_LOAD32_BYTE( "casanova.u87", 0x200003, 0x80000, CRC(61bd80f8) SHA1(13b93f2638c37a5dec5b4016c058f486f9cbadae) )	

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "casanova.u39", 0x000000, 0x80000, CRC(97d4095a) SHA1(4b1fde984025fae240bf64f812d67bc9cbf3a60c) )
	ROM_LOAD32_BYTE( "casanova.u41", 0x000001, 0x80000, CRC(95f67e82) SHA1(34b4350efbe22eb57871b009016adc2660842030) )
	ROM_LOAD32_BYTE( "casanova.u43", 0x000002, 0x80000, CRC(1462d7d6) SHA1(5637c2d0df5866b72d0c8804f23694fa5a025c8d) )
	ROM_LOAD32_BYTE( "casanova.u45", 0x000003, 0x80000, CRC(530d78bc) SHA1(56d6f593da9211d4785f35a9796d593beeb6b224) )

	ROM_REGION( 0x200000, "gfx3", 0 )
	ROM_LOAD32_BYTE( "casanova.u48", 0x000000, 0x80000, CRC(af9f59c5) SHA1(8620579045632ec6a4cd8fc4bff48428c94c8139) )
	ROM_LOAD32_BYTE( "casanova.u50", 0x000001, 0x80000, CRC(c73b5e98) SHA1(07d0be244aba084bd1ef099b547fe1c8e813cbeb) )
	ROM_LOAD32_BYTE( "casanova.u52", 0x000002, 0x80000, CRC(708f779c) SHA1(2272be3971d8983695f9fa7c840d94bdc0e4b0e6) )
	ROM_LOAD32_BYTE( "casanova.u54", 0x000003, 0x80000, CRC(e60bf0db) SHA1(503738b3b83a37ff812beed6c71e915072e5b10f) )
ROM_END

GAME( 199?, casanova,    0,        casanova,    casanova, driver_device,    0, ROT0,  "<unknown>", "Casanova", GAME_NOT_WORKING | GAME_NO_SOUND )



