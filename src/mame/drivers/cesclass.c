/*

    CES Classic wall games

	driver by Angelo Salese

	TODO:
	- custom layout for dual LCDs
	- artwork needed to make progresses
    - U43 and U44 bad in Trap Shoot Classic
    - games are incredibly slow by now

*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "rendlay.h"

class cesclassic_state : public driver_device
{
public:
	cesclassic_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ16_MEMBER(_600000_r);
	DECLARE_READ16_MEMBER(_670000_r);
	UINT16 *m_vram;

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void video_start();
	virtual bool screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);

};


void cesclassic_state::video_start()
{
}

bool cesclassic_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	int x,y,xi;

	bitmap_fill(&bitmap,&cliprect,get_black_pen(machine()));

	{
		for(y=0;y<64;y++)
		{
			for(x=0;x<16;x++)
			{
				for(xi=0;xi<16;xi++)
				{
					UINT8 color;

					color = (((m_vram[x+y*16+0x400])>>(15-xi)) & 1);
					color |= (((m_vram[x+y*16])>>(15-xi)) & 1)<<1;

					if((x*16+xi)<256 && ((y)+0)<256)
						*BITMAP_ADDR32(&bitmap, y, x*16+xi) = machine().pens[color];
				}
			}
		}
	}

	return 0;
}

READ16_MEMBER( cesclassic_state::_600000_r )
{
	return 0x100 | 0x200;
}

READ16_MEMBER( cesclassic_state::_670000_r )
{
	return -1;
}

static ADDRESS_MAP_START( cesclassic_map, AS_PROGRAM, 16, cesclassic_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x400000, 0x40cfff) AM_RAM
	AM_RANGE(0x40d000, 0x40ffff) AM_RAM AM_BASE(m_vram)
	AM_RANGE(0x600000, 0x600001) AM_READ(_600000_r)
//  AM_RANGE(0x640040, 0x640041) AM_WRITENOP
	AM_RANGE(0x670000, 0x670001) AM_READ(_670000_r)
	AM_RANGE(0x900100, 0x900101) AM_WRITENOP
//  AM_RANGE(0x900100, 0x900101) AM_DEVREADWRITE8("oki", okim6295_device, read, write,0x00ff)
//  AM_RANGE(0x904000, 0x904001) AM_WRITENOP
ADDRESS_MAP_END

static INPUT_PORTS_START( cesclassic )
INPUT_PORTS_END

static PALETTE_INIT( cesclassic )
{
	int i;

	for (i = 0; i < 4; i++)
		palette_set_color_rgb(machine, i, pal2bit(i), 0, 0);
}

static MACHINE_CONFIG_START( cesclassic, cesclassic_state )

	MCFG_CPU_ADD("maincpu", M68000, 24000000/2 )
	MCFG_CPU_PROGRAM_MAP(cesclassic_map)
	MCFG_CPU_VBLANK_INT("l_lcd",irq3_line_hold)
	MCFG_CPU_PERIODIC_INT(irq2_line_hold,60)

	/* video hardware */
	MCFG_SCREEN_ADD("l_lcd", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(8*16*2, 8*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 8*16*2-1, 0*8, 8*8-1)

	MCFG_PALETTE_LENGTH(4)
	MCFG_PALETTE_INIT(cesclassic)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 24000000/8, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


ROM_START(hrclass)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("hrclassic-v121-u44.bin", 0x000000, 0x80000, CRC(cbbbbbdb) SHA1(d406a27a823f5e530a9cf7615c396fe52df1c387) )
	ROM_LOAD16_BYTE("hrclassic-v121-u43.bin", 0x000001, 0x80000, CRC(f136aec3) SHA1(7823e81eb79c7575c1d6c2ae0848c2b9943ee6ef) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "hrclassic-v100-u28.bin", 0x00000, 0x80000, CRC(45d15b3a) SHA1(a11ce27a77ea353034c5f498cb46ef5ed787b0f9) )
ROM_END

ROM_START(ccclass)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("ccclassic-v110-u44.bin", 0x000000, 0x80000, CRC(63b63f3a) SHA1(d4b6f401815b05ac0c6c259bb066663d4c2ee132) )
	ROM_LOAD16_BYTE("ccclassic-v110-u43.bin", 0x000001, 0x80000, CRC(c1b420df) SHA1(9f1f22e6b27abcede6880a1a8ad5399ce582dab1) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "ccclassic-v100-u28.bin", 0x00000, 0x80000, CRC(94190a55) SHA1(fb219401431747fc3840da02c4e933d4c23049b7) )
ROM_END

ROM_START(tsclass)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("tsclassic-v100-u43u44.bin", 0x000000, 0x100000, CRC(a820ec9a) SHA1(84e38c7e54bb9e80142ed4e7763c9e36df560f42) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "tsclassic-v100-u28.bin", 0x00000, 0x80000, CRC(5bf53ca3) SHA1(5767391175fa9488ba0fb17a16de6d5013712a01) )
ROM_END


GAME(1997, hrclass, 0, cesclassic, cesclassic, 0, ROT0, "Creative Electronics And Software", "Home Run Classic (v1.21 12-feb-1997)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME(1997, ccclass, 0, cesclassic, cesclassic, 0, ROT0, "Creative Electronics And Software", "Country Club Classic (v1.10 03-apr-1997)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME(1997, tsclass, 0, cesclassic, cesclassic, 0, ROT0, "Creative Electronics And Software", "Trap Shoot Classic (v1.0 21-mar-1997)", GAME_NOT_WORKING | GAME_NO_SOUND )
