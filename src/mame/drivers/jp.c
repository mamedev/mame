#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/z80/z80.h"

class jp_state : public driver_device
{
public:
	jp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( jp_map, AS_PROGRAM, 8, jp_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( jp )
INPUT_PORTS_END

void jp_state::machine_reset()
{
}

static DRIVER_INIT( jp )
{
}

static MACHINE_CONFIG_START( jp, jp_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 5000000)
	MCFG_CPU_PROGRAM_MAP(jp_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ America 1492
/-------------------------------------------------------------------*/
ROM_START(america)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpvi1492.dat", 0x0000, 0x2000, CRC(e1d3bd57) SHA1(049c17cd717404e58339100ab8efd4d6bf8ee791))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sbvi1492.dat", 0x00000, 0x4000, CRC(38934e06) SHA1(eef850a5096a7436b728921aed22fe5f3d85b4ee))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("b1vi1492.dat", 0x0000, 0x8000, CRC(e93083ed) SHA1(6a44675d8cc8b8af40091646f589b833245bf092))
	ROM_LOAD("b2vi1492.dat", 0x8000, 0x8000, CRC(88be85a0) SHA1(ebf9d88847d6fd787892f0a34258f38e48445014))
	ROM_LOAD("b3vi1492.dat", 0x10000, 0x8000, CRC(1304c87b) SHA1(f84eb3116dd9841892f46106f9443c09cc094675))
	ROM_LOAD("b4vi1492.dat", 0x18000, 0x8000, CRC(831e4033) SHA1(f51f3f5a226692caed59e4aac0843cdb40f0667d))
	ROM_LOAD("b5vi1492.dat", 0x20000, 0x8000, CRC(46ee29a5) SHA1(08d756f5a0430aca723f842951dd8520024859b0))
	ROM_LOAD("b6vi1492.dat", 0x28000, 0x8000, CRC(5180d751) SHA1(6c2e8edf606d24d86f4ab6da4adaf1d1095e9b19))
	ROM_LOAD("b7vi1492.dat", 0x30000, 0x8000, CRC(ba98138f) SHA1(2c8ef3b17972b7022afdf89c6280d02038b65501))
ROM_END

/*-------------------------------------------------------------------
/ Aqualand
/-------------------------------------------------------------------*/
ROM_START(aqualand)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jpaqcpu", 0x0000, 0x2000, CRC(53230fab) SHA1(0b049f3be412be598982537e7fa7abf9b2766a16))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("jpaqsds", 0x00000, 0x4000, CRC(ff1e0cd2) SHA1(ef58d2b59929c7250dd30c413a3ba31ebfd7e09d))
	ROM_REGION(0x80000, "sound1", 0)
	ROM_LOAD("jpaq-1sd", 0x0000, 0x8000, CRC(7cdf2f7a) SHA1(e00482a6accd11e96fd0d444b3167b7d36332f7b))
	ROM_LOAD("jpaq-2sd", 0x8000, 0x8000, CRC(db05c774) SHA1(2d40410b70de6ab0de57e94c6d8ada6e8a4a2050))
	ROM_LOAD("jpaq-3sd", 0x10000, 0x8000, CRC(df38304e) SHA1(ec6f0c99764e3c3fe7e1de09b2d9b59d85d168d5))
	ROM_LOAD("jpaq-4sd", 0x18000, 0x8000, CRC(8065c03e) SHA1(0731cb76d3be117a82c4ad5b7e23b53e05b3a95a))
	ROM_LOAD("jpaq-5sd", 0x20000, 0x8000, CRC(a387a1a6) SHA1(20abee033a33e388a5f2ed3896a650766b62cfa2))
	ROM_LOAD("jpaq-6sd", 0x28000, 0x8000, CRC(55076afb) SHA1(68b86e6855b2a80e37d2fb172bb0c4fa107d4aba))
	ROM_LOAD("jpaq-7sd", 0x30000, 0x8000, CRC(67675b5b) SHA1(52b7cb310ddeff0bde7f0dfd37f61ab09964a75d))
	ROM_LOAD("jpaq-8sd", 0x38000, 0x8000, CRC(c9d2d30e) SHA1(ee504b0e2aa69f541c3f4d245cc6525a7c920fa7))
	ROM_LOAD("jpaq-9sd", 0x40000, 0x8000, CRC(3bc45f9f) SHA1(6d838b1ba94087f9a29af016b68125400dcf1fe5))
	ROM_LOAD("jpaq10sd", 0x48000, 0x8000, CRC(239cb7f3) SHA1(1abc59bc73cf84ee3b73d500bf57a2a202291fcb))
	ROM_LOAD("jpaq11sd", 0x50000, 0x8000, CRC(e5b9e70f) SHA1(7db0a13166120fe20bb76072475b092e942629cf))
	ROM_LOAD("jpaq12sd", 0x58000, 0x8000, CRC(9aa37260) SHA1(6eec14f0d7152bf0cfadabe5b3017b9b6b7aa2d3))
	ROM_LOAD("jpaq13sd", 0x60000, 0x8000, CRC(5599792e) SHA1(9d844d9f155f299bbe2d512f8ed84410e7a9cfb3))
	ROM_LOAD("jpaq14sd", 0x68000, 0x8000, CRC(0bdcbbbd) SHA1(555d8ed846079894cfc60041fb724deeaddc4e89))
ROM_END

/*-------------------------------------------------------------------
/ Faeton
/-------------------------------------------------------------------*/
ROM_START(faeton)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("faeton.cpu", 0x0000, 0x2000, CRC(ef7e6915) SHA1(5d3d86549606b3d9134bb3f6d3026d6f3e07d4cd))
ROM_END

/*-------------------------------------------------------------------
/ Halley Comet
/-------------------------------------------------------------------*/
ROM_START(halley)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("halley.cpu", 0x0000, 0x2000, CRC(b158a0d7) SHA1(ad071ac3d06a99a8fbd4df461071fe03dc1e1a26))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("hcsnd0.dat", 0x00000, 0x4000, CRC(39b9107a) SHA1(8a11fa0c1558d0b1d309446b8a6f97e761b6559d))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("hcsnd1.dat", 0x0000, 0x8000, NO_DUMP)
	ROM_LOAD("hcsnd2.dat", 0x8000, 0x8000, NO_DUMP)
	ROM_LOAD("hcsnd3.dat", 0x10000, 0x8000, NO_DUMP)
	ROM_LOAD("hcsnd4.dat", 0x18000, 0x8000, NO_DUMP)
	ROM_LOAD("hcsnd5.dat", 0x20000, 0x8000, NO_DUMP)
	ROM_LOAD("hcsnd6.dat", 0x28000, 0x8000, NO_DUMP)
	ROM_LOAD("hcsnd7.dat", 0x30000, 0x8000, NO_DUMP)
ROM_END

/*-------------------------------------------------------------------
/ Lortium
/-------------------------------------------------------------------*/
ROM_START(lortium)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpulort1.dat", 0x0000, 0x2000, NO_DUMP)
	ROM_LOAD("cpulort2.dat", 0x2000, 0x2000, CRC(71eebb26) SHA1(9d49c1012555bda24ac7287499bcb93828cbb57f))
ROM_END

/*-------------------------------------------------------------------
/ Pimbal
/-------------------------------------------------------------------*/
ROM_START(pimbal)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("p3000.r1", 0x0000, 0x2000, CRC(57fb5958) SHA1(536d6564c184f214edf821b83a27aa7f75c7ad00))
	ROM_LOAD("p3000.r2", 0x2000, 0x2000, CRC(b8aae5ad) SHA1(8639b132aa69281f4460f80e84e0d30a5dc298d0))
ROM_END

/*-------------------------------------------------------------------
/ Olympus
/-------------------------------------------------------------------*/
ROM_START(olympus)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("olympus.dat", 0x0000, 0x2000, CRC(08b021e8) SHA1(9662d37ccef94b6e6bc3c8c81dea0c0a34c8052d))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("cs.128", 0x00000, 0x4000, CRC(39b9107a) SHA1(8a11fa0c1558d0b1d309446b8a6f97e761b6559d))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("c1.256", 0x0000, 0x8000, CRC(93ceefbf) SHA1(be50b3d4485d4e8291047a52ca60656b55729555))
	ROM_LOAD("c2.256", 0x8000, 0x8000, NO_DUMP)
	ROM_LOAD("c3.256", 0x10000, 0x8000, CRC(266eb5dd) SHA1(0eb7c098ddb7f257daf625e5209a54c306d365bf))
	ROM_LOAD("c4.256", 0x18000, 0x8000, CRC(082a052d) SHA1(f316fbe6ff63433861a8856e297c953ce29a8901))
	ROM_LOAD("c5.256", 0x20000, 0x8000, CRC(402a3fb2) SHA1(1c078ca519271bf2bcbe0bc10e33078861085fcf))
	ROM_LOAD("c6.256", 0x28000, 0x8000, CRC(d113add1) SHA1(c0258226994af162ef766d5e8d27f809dac4ef7f))
	ROM_LOAD("c7.256", 0x30000, 0x8000, CRC(13f5fcad) SHA1(e7a8b76527067f16aa62d0f22eccde5b55eba972))
ROM_END

/*-------------------------------------------------------------------
/ Petaco
/-------------------------------------------------------------------*/
ROM_START(petaco)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("petaco1.cpu", 0x0000, 0x2000, CRC(f4e09939) SHA1(dcc4220b269d271eb0b6ad0a5d3c1a240587a01b))
	ROM_LOAD("petaco2.cpu", 0x2000, 0x2000, CRC(d29a59ea) SHA1(bb7891e9597bbf5ae6a3276abf2b1247e082d828))
ROM_END

/*-------------------------------------------------------------------
/ Petaco 2
/-------------------------------------------------------------------*/
ROM_START(petaco2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("petaco2.dat", 0x0000, 0x2000, NO_DUMP)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("jpsonid0.dat", 0x00000, 0x4000, CRC(1bdbdd60) SHA1(903012e58cdb4041e5546a377f5c9df83dc93737))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("jpsonid1.dat", 0x0000, 0x8000, CRC(e39da92a) SHA1(79eb60710bdf6b826349e02ae909426cb81e131e))
	ROM_LOAD("jpsonid2.dat", 0x8000, 0x8000, CRC(88456f1e) SHA1(168fe88ae9da5114d0ef6427df0503ca2eea9089))
	ROM_LOAD("jpsonid3.dat", 0x10000, 0x8000, CRC(c7597d29) SHA1(45abe1b28ad14610ac8e2bc3a70af46bbe6277f4))
	ROM_LOAD("jpsonid4.dat", 0x18000, 0x8000, CRC(0d29a028) SHA1(636cc9a1f6128c820b18db4bf764e0be10a46119))
	ROM_LOAD("jpsonid5.dat", 0x20000, 0x8000, CRC(76790393) SHA1(23df394ecd11205d83073dca160f8f9a98aaa169))
	ROM_LOAD("jpsonid6.dat", 0x28000, 0x8000, CRC(53c3f0b4) SHA1(dcf4c63636e2b7ff5cd2db99d949db9e33b78fc7))
	ROM_LOAD("jpsonid7.dat", 0x30000, 0x8000, CRC(ff430b1b) SHA1(423592a40eba174108dfc6817e549c643bb3c80f))
ROM_END

GAME(1986,	america,	0,		jp,	jp,	jp,	ROT0,	"Juegos Populares",				"America 1492",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1986,	aqualand,	0,		jp,	jp,	jp,	ROT0,	"Juegos Populares",				"Aqualand",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1985,	faeton,		0,		jp,	jp,	jp,	ROT0,	"Juegos Populares",				"Faeton",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1987,	lortium,	0,		jp,	jp,	jp,	ROT0,	"Juegos Populares",				"Lortium",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(19??,	pimbal,		0,		jp,	jp,	jp,	ROT0,	"Juegos Populares",				"Pimbal (Pinball 3000)",GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1984,	petaco,		0,		jp,	jp,	jp,	ROT0,	"Juegos Populares",				"Petaco",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1985,	petaco2,	0,		jp,	jp,	jp,	ROT0,	"Juegos Populares",				"Petaco 2",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1986,	halley,		0,		jp,	jp,	jp,	ROT0,	"Juegos Populares",				"Halley Comet",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1986,	olympus,	0,		jp,	jp,	jp,	ROT0,	"Juegos Populares",				"Olympus",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
