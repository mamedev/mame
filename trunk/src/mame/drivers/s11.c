/*
    Williams System 11
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class williams_s11_state : public driver_device
{
public:
	williams_s11_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( williams_s11_map, AS_PROGRAM, 8, williams_s11_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( williams_s11 )
INPUT_PORTS_END

void williams_s11_state::machine_reset()
{
}

static DRIVER_INIT( williams_s11 )
{
}

static MACHINE_CONFIG_START( williams_s11, williams_s11_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(williams_s11_map)
MACHINE_CONFIG_END

/*--------------------
/ Gold Mine (#920)
/--------------------*/
ROM_START(gmine_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u27.128", 0x8000, 0x4000, CRC(99c6e049) SHA1(356faec0598a54892050a28857e9eb5cdbf35833))
	ROM_RELOAD( 0xc000, 0x4000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("u21.256", 0x0000, 0x8000, CRC(3b801570) SHA1(50b50ff826dcb031a30940fa3099bd3a8d773831))
	ROM_LOAD("u22.256", 0x8000, 0x8000, CRC(08352101) SHA1(a7437847a71cf037a80686292f9616b1e08922df))
ROM_END

/*--------------------
/ Grand Lizard 04/86
/--------------------*/
ROM_START(grand_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lzrd_u26.l4", 0x4000, 0x2000, CRC(5fe50db6) SHA1(7e2adfefce5c33ad605606574dbdfb2642aa0e85))
	ROM_RELOAD( 0x6000, 0x2000)
	ROM_LOAD("lzrd_u27.l4", 0x8000, 0x8000, CRC(6462ca55) SHA1(0ebfa998d3cefc213ada9ed815d44977120e5d6d))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("lzrd_u21.l1", 0x4000, 0x4000, CRC(98859d37) SHA1(08429b9e6a3b3007815373dc280b985e3441aa9f))
	ROM_LOAD("lzrd_u22.l1", 0xc000, 0x4000, CRC(4e782eba) SHA1(b44ab499128300175bdb57f07ffe2992c82e47e4))
	ROM_RELOAD( 0x8000, 0x4000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("lzrd_u4.l1", 0x00000, 0x8000, CRC(4baafc11) SHA1(3507f5f37e02688fa56cf5bb303eaccdcedede06))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
ROM_END

/*--------------------
/ High Speed 01/86
/--------------------*/
ROM_START(hs_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hs_u26.l4", 0x4000, 0x2000, CRC(38b73830) SHA1(df89670f3df2b657dcf1f8ee08e506e54e016028))
	ROM_RELOAD( 0x6000, 0x2000)
	ROM_LOAD("hs_u27.l4", 0x8000, 0x8000, CRC(24c6f7f0) SHA1(bb0058650ec0908f88b6a202df79e971b46f8594))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("hs_u21.l2", 0x0000, 0x8000, CRC(c0580037) SHA1(675ca65a6a20f8607232c532b4d127641f77d837))
	ROM_LOAD("hs_u22.l2", 0x8000, 0x8000, CRC(c03be631) SHA1(53823e0f55377a45aa181882c310dd307cf368f5))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("hs_u4.l1", 0x00000, 0x8000, CRC(0f96e094) SHA1(58650705a02a71ced85f5c2a243722a35282cbf7))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
ROM_END

ROM_START(hs_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l3.rom", 0x4000, 0x2000, CRC(fd587959) SHA1(20fe6d7bd617b1fa886362ce520393a25be9a632))
	ROM_RELOAD( 0x6000, 0x2000)
	ROM_LOAD("hs_u27.l4", 0x8000, 0x8000, CRC(24c6f7f0) SHA1(bb0058650ec0908f88b6a202df79e971b46f8594))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("hs_u21.l2", 0x0000, 0x8000, CRC(c0580037) SHA1(675ca65a6a20f8607232c532b4d127641f77d837))
	ROM_LOAD("hs_u22.l2", 0x8000, 0x8000, CRC(c03be631) SHA1(53823e0f55377a45aa181882c310dd307cf368f5))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("hs_u4.l1", 0x00000, 0x8000, CRC(0f96e094) SHA1(58650705a02a71ced85f5c2a243722a35282cbf7))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
ROM_END

/*--------------------
/ Road Kings 07/86
/--------------------*/
ROM_START(rdkng_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("road_u26.l4", 0x4000, 0x4000, CRC(4ea27d67) SHA1(cf46e8c5e417999150403d6d40adf8c36b1c0347))
	ROM_LOAD("road_u27.l4", 0x8000, 0x8000, CRC(5b88e755) SHA1(6438505bb335f670e0892126764819a48eec9b88))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("road_u21.l1", 0x0000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x8000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("road_u4.l1", 0x00000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
ROM_END

ROM_START(rdkng_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("road_u26.l1", 0x4000, 0x4000, CRC(19abe96b) SHA1(d6c3b6dab328f23cc4506e4f56cd0beeb06fb3cb))
	ROM_LOAD("road_u27.l1", 0x8000, 0x8000, CRC(3dcad794) SHA1(0cf06f8e16d738f0bc0111e2e12351a26e2f02c6))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("road_u21.l1", 0x0000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x8000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("road_u4.l1", 0x00000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
ROM_END

ROM_START(rdkng_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("road_u26.l1", 0x4000, 0x4000, CRC(19abe96b) SHA1(d6c3b6dab328f23cc4506e4f56cd0beeb06fb3cb))
	ROM_LOAD("road_u27.l2", 0x8000, 0x8000, CRC(aff45e2b) SHA1(c52aca20639f519a940951ef04c2bd179a596b30))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("road_u21.l1", 0x0000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x8000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("road_u4.l1", 0x00000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
ROM_END

ROM_START(rdkng_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("road_u26.l3", 0x4000, 0x4000, CRC(9bade45d) SHA1(c1791724761cdd1d863e12b02655c5fed8936162))
	ROM_LOAD("road_u27.l3", 0x8000, 0x8000, CRC(97b599dc) SHA1(18524d22a75b0569bb480d847cef8047ee51f91e))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("road_u21.l1", 0x0000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x8000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("road_u4.l1", 0x00000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
ROM_END

/*--------------------
/ Top Dawg (Shuffle) (#921)
/--------------------*/
ROM_START(tdawg_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tdu27r1.128", 0x8000, 0x4000, CRC(0b4bb586) SHA1(a927ebf7167609cc84b38c22aa35d0c4d259dd8b))
	ROM_RELOAD( 0xc000, 0x4000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("tdsu21r1.256", 0x0000, 0x8000, CRC(6a323227) SHA1(7c7263754e5672c654a2ee9582f0b278e637a909))
	ROM_LOAD("tdsu22r1.256", 0x8000, 0x8000, CRC(58407eb4) SHA1(6bd9b304c88d9470eae5afb6621187f4a8313573))
ROM_END

/*--------------------
/ Shuffle Inn (Shuffle) (#922)
/--------------------*/
ROM_START(shfin_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u27rom-1.rv1", 0x8000, 0x4000, CRC(40cfb74a) SHA1(8cee4212ea8bb6b360060391df3208e1e129d7e5))
	ROM_RELOAD( 0xc000, 0x4000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("u21snd-2.rv1", 0x0000, 0x8000, CRC(80ddce05) SHA1(9498260e5ccd2fe0eb03ff321dd34eb945b0213a))
	ROM_LOAD("u22snd-2.rv1", 0x8000, 0x8000, CRC(6894abaf) SHA1(2d661765fbfce33a73a20778c41233c0bd9933e9))
ROM_END

/*--------------------
/ Tic-Tac-Strike (#919)
/--------------------*/
ROM_START(tts_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u27_l2.128", 0x8000, 0x4000, CRC(edbcab92) SHA1(0f6b2dc01874984f9a17ee873f2fa0b6c9bba5be))
	ROM_RELOAD( 0xc000, 0x4000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("tts_u21.256", 0x0000, 0x8000, NO_DUMP)
	ROM_LOAD("tts_u22.256", 0x8000, 0x8000, NO_DUMP)
ROM_END

ROM_START(tts_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tts_u27.128", 0x8000, 0x4000, CRC(f540c53c) SHA1(1c7a318278ad1afdcbe6aaf81f9b774882b069d6))
	ROM_RELOAD( 0xc000, 0x4000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("tts_u21.256", 0x0000, 0x8000, NO_DUMP)
	ROM_LOAD("tts_u22.256", 0x8000, 0x8000, NO_DUMP)
ROM_END

GAME(1987,	gmine_l2,	0,			williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"Gold Mine (Shuffle) (L-2)",	GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	grand_l4,	0,			williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"Grand Lizard (L-4)",			GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	hs_l4,		0,			williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"High Speed (L-4)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	hs_l3,		hs_l4,		williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"High Speed (L-3)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	rdkng_l4,	0,			williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"Road Kings (L-4)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	rdkng_l1,	rdkng_l4,	williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"Road Kings (L-1)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	rdkng_l2,	rdkng_l4,	williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"Road Kings (L-2)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	rdkng_l3,	rdkng_l4,	williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"Road Kings (L-3)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1987,	tdawg_l1,	0,			williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"Top Dawg (Shuffle) (L-1)",		GAME_IS_SKELETON_MECHANICAL)
GAME(1987,	shfin_l1,	0,			williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"Shuffle Inn (Shuffle) (L-1)",	GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	tts_l2,		0,			williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"Tic-Tac-Strike (Shuffle) (L-2)",	GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	tts_l1,		tts_l2,		williams_s11,	williams_s11,	williams_s11,	ROT0,	"Williams",				"Tic-Tac-Strike (Shuffle) (L-1)",	GAME_IS_SKELETON_MECHANICAL)
