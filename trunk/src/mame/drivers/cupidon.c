/* Cupidon - Russian Fruit Machines? */

/*
 seems to be Kupidon in the ROMs?

*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m68000/m68000.h"

class cupidon_state : public driver_device
{
public:
	cupidon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( cupidon_map, AS_PROGRAM, 32, cupidon_state )
	AM_RANGE(0x0000000, 0x07fffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START(  cupidon )
INPUT_PORTS_END


static MACHINE_CONFIG_START( cupidon, cupidon_state )
	MCFG_CPU_ADD("maincpu", M68340, 16000000)	 // The access to 3FF00 at the start would suggest this is a 68340
	MCFG_CPU_PROGRAM_MAP(cupidon_map)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	/* unknown sound */
MACHINE_CONFIG_END



ROM_START( tsarevna )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ts_1_29_u2_32m.bin", 0x000000, 0x400000, CRC(e7798a5d) SHA1(5ad876a693c93df79ea5e5672c0a5f3952b2cb36) )
	ROM_LOAD16_WORD_SWAP( "ts_1_29_u1_32m.bin", 0x400000, 0x400000, CRC(5a35ca2a) SHA1(b7beac148190b508469f832d370af082f479527c) )
ROM_END

ROM_START( tsarevnaa )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "v0131-2.bin", 0x000000, 0x400000, CRC(36349e13) SHA1(d82c93b7f19e8b75b0d56653aaaf5da44bb302f5) )
	ROM_LOAD16_WORD_SWAP( "v0131-1.bin", 0x400000, 0x400000, CRC(f502e677) SHA1(84f89f214aeff8544d526c44634672d972714bf6) )
ROM_END

ROM_START( gangrose )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "gangv470m322sec.bin", 0x000000, 0x400000, CRC(c916a292) SHA1(ceac54b06722874f21431834403e49aa2c9c1ded) )
ROM_END


ROM_START( funnyfm )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ff_1_17_u2_32m.bin", 0x000000, 0x400000, CRC(cdd616a7) SHA1(69a9bd73f6f9abb306522071316e1dd770b4ac12) )
	ROM_LOAD16_WORD_SWAP( "ff_1_17_u1_32m.bin", 0x400000, 0x400000, CRC(2073345c) SHA1(33803ebd7720c3436486a383383e99722c2554f4) )
ROM_END

ROM_START( funnyfma )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ff_1_26_u2_32m.bin", 0x000000, 0x400000, CRC(d813da5c) SHA1(ef82f2c7d0aa21921a25d08555c727a967b1a235) )
	ROM_LOAD16_WORD_SWAP( "ff_1_26_u1_32m.bin", 0x400000, 0x400000, CRC(e3c4f483) SHA1(cc78eadadc13a8f295658b493e47eff3bf719c7e) )
ROM_END

ROM_START( funnyfmb )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "u2.bin", 0x000000, 0x400000, CRC(c8fdc338) SHA1(cd3372988c7a4b35069d6e56e786cecb32e0996e) )
	ROM_LOAD16_WORD_SWAP( "u1.bin", 0x400000, 0x400000, CRC(ca2a5345) SHA1(be7c68fca0534b2d817ac78377f98cda2021c5fa) )
ROM_END

ROM_START( cashtrn )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "cash_train_1_10_u2_32.bin", 0x000000, 0x400000, CRC(ee81a918) SHA1(116e14e8f23517c943f8867498b6105221974ce3) )
	ROM_LOAD16_WORD_SWAP( "cash_train_1_10_u1_32.bin", 0x400000, 0x400000, CRC(4a1704e7) SHA1(18cc87cf54277e61a37cfe9c77164bef9688acf6) )
ROM_END



DRIVER_INIT( cupidon )
{

}

/* (c) date is from string in ROM, revision date is noted next to sets - Spellings are as found in ROM */
GAME( 2004, tsarevna		,0,			cupidon, cupidon, cupidon, ROT0, "Kupidon","Tsarevna (v1.29)", GAME_IS_SKELETON ) // 12 Oct 2005
GAME( 2004, tsarevnaa		,tsarevna,	cupidon, cupidon, cupidon, ROT0, "Kupidon","Tsarevna (v1.31)", GAME_IS_SKELETON ) // 17 Jan 2007

GAME( 2004, gangrose		,0,			cupidon, cupidon, cupidon, ROT0, "Kupidon","Gangster's Roses (v4.70)", GAME_IS_SKELETON ) // 01 Sep 2004

GAME( 2004, funnyfm			,0,			cupidon, cupidon, cupidon, ROT0, "Kupidon","Funny Farm (v1.17)", GAME_IS_SKELETON ) // 02 Mar 2005
GAME( 2004, funnyfma		,funnyfm,	cupidon, cupidon, cupidon, ROT0, "Kupidon","Funny Farm (v1.26)", GAME_IS_SKELETON ) // 08 Aug 2005
GAME( 2004, funnyfmb		,funnyfm,	cupidon, cupidon, cupidon, ROT0, "Kupidon","Funny Farm (v1.30)", GAME_IS_SKELETON ) // 16 May 2006

GAME( 2005, cashtrn			,0,			cupidon, cupidon, cupidon, ROT0, "Kupidon","Cash Train (v1.10)", GAME_IS_SKELETON ) // 09 Jan 2006
