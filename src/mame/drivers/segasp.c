/*

Sega System SP (Spider)
skeleton driver

this is another 'Naomi-like' system

todo: make this actually readable, we don't support unicode source files
convert CF card based sets to CHD?

 Title                                       PCB ID	 REV	CF ID		Dumped	Region	PIC             MAIN BD Serial
Battle Police                               ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx
???????????
Beetle DASH!!                               ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx
??DASH!!                                    						
Bingo Galaxy                                ???-?????				no		???-????-????   AAFE-01E10924916, AAFE-01D67304905, Medal
?????????                                   						
Bingo Parade                                ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx, Medal
???????                                     						
Brick People / Block People                 834-14881				ROM	ALL	253-5508-0558   AAFE-01F67905202, AAFE-01F68275202
????????                                    						
Dinosaur King                               834-14493-01 D			ROM	US	253-5508-0408   AAFE-01D1132xxxx, AAFE-01D15924816
?????                                       						
Dinosaur King - Operation: Dinosaur Rescue  837-14434-91	MDA-C0021?	ROM	US/EXP	253-5508-0408   AAFE-01A30164715, AAFE-01B92094811
-                                           834-14662-01
Dinosaur King 2                             ???-?????				no		253-5508-0408   AAFE-xxxxxxxxxxx
?????2007                                   						
Dinosaur King 2 Ver 2.5                     834-14792-02 F	MDA-C0047	CF	EXP	253-5508-0408   AAFE-01D73384904
????? 2008                                  						
Disney: Magical Dream Dance on Stage        ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx
Disney??????? ?? ????????                   						
Future Police Patrol Chase                  ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx
???????????                                 						
Issyouni Turbo Drive                        ???-?????				no		???-????-????   AAFE-01E91305101
????????????                                						  
Issyouni Wan Wan                            ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx
?????????                                   						
King of Beetle: Battle Terminal             ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx
????????? ????????????                      						
Love & Berry Ver 1.003                      834-14661-02			ROM	EXP	253-5508-0446   AAFE-01D84934906
?????????and???                             						
Love & Berry Ver 2.000                      834-14661-02			ROM	EXP	253-5508-0446   AAFE-01D8493xxxx
?????????and???                             						
Love & Berry 3 EXP Ver 1.002                834-14661-01	MDA-C0042	CF	US/EXP	253-5508-0446   AAFE-01D64704904
?????????and???                             						
Marine & Marine                             ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx
-                                           						
Mirage World                                ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx, Medal
?????????                                   						
Monopoly: The Medal                         ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx, Medal
?????·?·???                                 						
Monopoly: The Medal 2nd Edition             ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx, Medal
?????·?·???                                 						
Mushiking 2K6 2ND                           ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx
                                            
Mushiking 2K7 1ST                           ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx
                                            
Tetris Giant / Tetris Dekaris               834-14970	 G	MDA-C0076	CF	ALL	253-5508-0604   AAFE-01G03025212
????·????                                   						
Tetris Giant / Tetris Dekaris Ver.2.000     834-14970	 G			ROM	ALL	253-5508-0604   AAFE-xxxxxxxxxxx
????·????                                   						
Thomas: The Tank Engine                     ???-?????				no		???-????-????   AAFE-xxxxxxxxxxx
-                                           						

REV PCB       IC6s      Flash       AU1500
D  171-8278D  315-6370  8x 128Mbit  AMD
F  171-8278F  315-6416  8x 512Mbit  AMD
G  171-8278G  315-6416  2x 512Mbit  RMI

*/

#include "emu.h"
#include "cpu/sh4/sh4.h"
#include "debugger.h"


class segasp_state : public driver_device
{
public:
	segasp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{

	}
	
	virtual void video_start();
	UINT32 screen_update_segasp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<sh4_device> m_maincpu;
protected:
};


void segasp_state::video_start()
{
}

UINT32 segasp_state::screen_update_segasp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}



static ADDRESS_MAP_START( main_map, AS_PROGRAM, 64, segasp_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_MIRROR(0xa2000000) AM_ROM AM_REGION("maincpu", 0) // BIOS
ADDRESS_MAP_END

static INPUT_PORTS_START( segasp )
INPUT_PORTS_END

#define SP_CPU_CLOCK 200000000

static MACHINE_CONFIG_START( segasp, segasp_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH4LE, SP_CPU_CLOCK)
	MCFG_SH4_MD0(1)
	MCFG_SH4_MD1(0)
	MCFG_SH4_MD2(1)
	MCFG_SH4_MD3(0)
	MCFG_SH4_MD4(0)
	MCFG_SH4_MD5(1)
	MCFG_SH4_MD6(0)
	MCFG_SH4_MD7(1)
	MCFG_SH4_MD8(0)
	MCFG_SH4_CLOCK(SP_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(segasp_state, screen_update_segasp)

	MCFG_PALETTE_ADD("palette", 0x1000)
MACHINE_CONFIG_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */

#define SEGASP_BIOS \
	ROM_REGION( 0x200000, "maincpu", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "bios0" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "epr-24236a.ic50", 0x000000, 0x200000, CRC(ca7df0de) SHA1(504c74d5fc96c53ef9f7753e9e37fb8b39cb628c) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "bios1" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "epr-24328.ic50", 0x000000, 0x200000, CRC(25f2ef00) SHA1(e58dec9f171e52b3ded213b3fcd9a0de8a438076) ) \
	ROM_SYSTEM_BIOS( 2, "bios2", "bios2" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "epr-24328a.ic50", 0x000000, 0x200000, CRC(03ec3805) SHA1(a8fbaea826ca257be0b2b86952f247254929e046) ) \


ROM_START( segasp )
	SEGASP_BIOS
ROM_END

ROM_START( brickppl )
	SEGASP_BIOS

	ROM_REGION( 0x10000000, "rom", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x0000000, 0x4000000, CRC(d79afdb6) SHA1(328e535980624d9173164b756ebbdc1ca4cb6f18) )
	ROM_LOAD( "ic63",  0x4000000, 0x4000000, CRC(4f3c0937) SHA1(72d68b66c57ff539b8058f80f1a15ffa44095460) )
	ROM_LOAD( "ic64",  0x8000000, 0x4000000, CRC(383e90d9) SHA1(eeca4b1bd0cd1fed7b85f045d71e0c7258d4350b) )
	ROM_LOAD( "ic65",  0xc000000, 0x4000000, CRC(4c29b5ac) SHA1(9e6a79ad2d2498eed5b2590c8764222e7d6c0229) )
ROM_END


#define GAME_FLAGS (GAME_NO_SOUND|GAME_NOT_WORKING)

GAME( 200?, segasp,  0,          segasp,    segasp, driver_device,    0, ROT0, "Sega", "Sega System SP (Spider) BIOS", GAME_FLAGS | GAME_IS_BIOS_ROOT )

GAME( 200?, brickppl,segasp,     segasp,    segasp, driver_device,    0, ROT0, "Sega", "Brick People / Block People", GAME_FLAGS )

