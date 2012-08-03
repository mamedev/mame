/* MPU4 'Plasma' addition */

/* the Plasma was an oversized DMD, but was rarely used, Big Chief might be the only game with it, at least it's the only dump we have? */
// http://www.youtube.com/watch?v=PAs8p48u0Jc

#include "emu.h"
#include "includes/mpu4.h"
#include "cpu/m68000/m68000.h"
#include "mpu4plasma.lh"


class mpu4plasma_state : public mpu4_state
{
public:
	mpu4plasma_state(const machine_config &mconfig, device_type type, const char *tag)
		: mpu4_state(mconfig, type, tag),
		m_plasmaram(*this, "plasmaram")
	{

	}

	required_shared_ptr<UINT16> m_plasmaram;

	DECLARE_READ16_MEMBER( mpu4plasma_unk_r )
	{
		return machine().rand();
	}
	
	DECLARE_WRITE16_MEMBER( mpu4plasma_unk_w )
	{

	}
};




MACHINE_CONFIG_EXTERN( mod2 );
INPUT_PORTS_EXTERN( mpu4 );
extern DRIVER_INIT( m4default );

static ADDRESS_MAP_START( mpu4plasma_map, AS_PROGRAM, 16, mpu4plasma_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	
	// why does it test this much ram, just sloppy code expecting mirroring?
	AM_RANGE(0x400000, 0x4fffff) AM_RAM AM_SHARE("plasmaram")
	// comms?
	AM_RANGE(0xffff00, 0xffff01) AM_READ( mpu4plasma_unk_r ) 
	AM_RANGE(0xffff04, 0xffff05) AM_WRITE( mpu4plasma_unk_w )
ADDRESS_MAP_END

SCREEN_UPDATE_IND16( mpu4plasma )
{
	// don't know if this really gets drawn straight from ram..
	mpu4plasma_state *state = screen.machine().driver_data<mpu4plasma_state>();
	int base = 0x1600 / 2;

	UINT16* rambase = state->m_plasmaram;
	UINT16* dst_bitmap;

	int i,y,x,p;
	i = 0;

	for (y=0;y<40;y++)
	{
		dst_bitmap = &bitmap.pix16(y);

		for (x=0;x<128/16;x++)
		{
			UINT16 pix = rambase[base+i];
			
			for (p=0;p<16;p++)
			{
				UINT16 bit = (pix << p)&0x8000;
				if (bit) dst_bitmap[x*16 + p] = 1;
				else dst_bitmap[x*16 + p] = 0;
			}

			i++;

		}
	}
	
	return 0;
}


MACHINE_CONFIG_DERIVED_CLASS( mpu4plasma    , mod2, mpu4plasma_state )
	MCFG_CPU_ADD("plasmacpu", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(mpu4plasma_map)
	MCFG_CPU_VBLANK_INT("screen", irq4_line_hold)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 128-1, 0*8, 40-1)
	MCFG_SCREEN_UPDATE_STATIC(mpu4plasma)

	MCFG_PALETTE_LENGTH(0x200)
MACHINE_CONFIG_END


#define M4BIGCHF_PLASMA \
	ROM_REGION( 0x40000, "plasmacpu", 0 ) \
	ROM_LOAD16_BYTE( "b6cpl.p0", 0x00000, 0x020000, CRC(7fbb2efb) SHA1(c21136bf10407f1685f3933d426ef53925aca8d8) ) \
	ROM_LOAD16_BYTE( "b6cpl.p1", 0x00001, 0x020000, CRC(a9f67f3e) SHA1(1309cc2dc8565ee79ac8cdc754187c8db6ddb3ea) ) \

ROM_START( m4bigchf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b6cs.p1", 0x0000, 0x010000, CRC(4f45086b) SHA1(e1d639b068951df8f25b9c77d4fb86336ad19933) )
	M4BIGCHF_PLASMA
ROM_END

ROM_START( m4bigchfa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bchf20-6", 0x0000, 0x010000, CRC(7940eb01) SHA1(b23537e91842a0d9b25b9c76b245d2be3d9af57f) )
	M4BIGCHF_PLASMA
ROM_END

ROM_START( m4bigchfb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b6cc.p1", 0x0000, 0x010000, CRC(8d3916b4) SHA1(1818137da9d53000053a8023c4994c6539459df0) )
	M4BIGCHF_PLASMA
ROM_END

ROM_START( m4bigchfc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big chief 6.bin", 0x0000, 0x010000, CRC(edee08b7) SHA1(8de6160a4a4e5cd57f64c49d913f763aa87dc69a) )
	M4BIGCHF_PLASMA
ROM_END

#define GAME_FLAGS (GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK)

GAMEL(199?, m4bigchf	,0			,mpu4plasma   	,mpu4				,m4default			,ROT0,   "Barcrest","Big Chief (Barcrest) (MPU4 w/ Plasma DMD) (set 1)",						GAME_FLAGS|GAME_NO_SOUND,layout_mpu4plasma )
GAMEL(199?, m4bigchfa	,m4bigchf	,mpu4plasma   	,mpu4				,m4default			,ROT0,   "Barcrest","Big Chief (Barcrest) (MPU4 w/ Plasma DMD) (set 2)",						GAME_FLAGS|GAME_NO_SOUND,layout_mpu4plasma )
GAMEL(199?, m4bigchfb	,m4bigchf	,mpu4plasma   	,mpu4				,m4default			,ROT0,   "Barcrest","Big Chief (Barcrest) (MPU4 w/ Plasma DMD) (set 3)",						GAME_FLAGS|GAME_NO_SOUND,layout_mpu4plasma )
GAMEL(199?, m4bigchfc	,m4bigchf	,mpu4plasma   	,mpu4				,m4default			,ROT0,   "Barcrest","Big Chief (Barcrest) (MPU4 w/ Plasma DMD) (set 4)",						GAME_FLAGS|GAME_NO_SOUND,layout_mpu4plasma )
