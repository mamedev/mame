/*

    Gottlieb System 80B

*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6502/m6502.h"

class gts80b_state : public driver_device
{
public:
	gts80b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};

static ADDRESS_MAP_START( gts80b_map, AS_PROGRAM, 8, gts80b_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	AM_RANGE(0x1000, 0x17ff) AM_MIRROR(0xc000) AM_ROM	/* PROM */
	AM_RANGE(0x2000, 0x2fff) AM_MIRROR(0xc000) AM_ROM	/* u2 ROM */
	AM_RANGE(0x3000, 0x3fff) AM_MIRROR(0xc000) AM_ROM	/* u3 ROM */
ADDRESS_MAP_END


static INPUT_PORTS_START( gts80b )
INPUT_PORTS_END

void gts80b_state::machine_reset()
{
}

static DRIVER_INIT( gts80b )
{
}

/* with Old Style Sound Board */
static MACHINE_CONFIG_START( gts80b_s, gts80b_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 850000)
	MCFG_CPU_PROGRAM_MAP(gts80b_map)

	/* related to src/mame/audio/gottlieb.c */
//  MCFG_IMPORT_FROM(gts80s_s)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gts80b_s1, gts80b_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 850000)
	MCFG_CPU_PROGRAM_MAP(gts80b_map)

	/* related to src/mame/audio/gottlieb.c? */
//  MCFG_IMPORT_FROM(gts80s_b1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gts80b_s2, gts80b_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 850000)
	MCFG_CPU_PROGRAM_MAP(gts80b_map)

	/* related to src/mame/audio/gottlieb.c? */
//  MCFG_IMPORT_FROM(gts80s_b2)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gts80b_s3, gts80b_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 850000)
	MCFG_CPU_PROGRAM_MAP(gts80b_map)

	/* related to src/mame/audio/gottlieb.c? */
//  MCFG_IMPORT_FROM(gts80s_b3)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bonebstr, gts80b_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 850000)
	MCFG_CPU_PROGRAM_MAP(gts80b_map)

	/* related to src/mame/audio/gottlieb.c? */
//  MCFG_IMPORT_FROM(gts80s_b3a)
MACHINE_CONFIG_END


/*-------------------------------------------------------------------
/ Amazon Hunt II 05/85
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Arena
/-------------------------------------------------------------------*/
ROM_START(arena)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(4783b689) SHA1(d10d4cbf8d00c9d0db57cdac32ef96498275eea6))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(8c9f8ee9) SHA1(840505d08e387c3f7de105305e183f8ed3a6d5c6))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(78e6cbf1) SHA1(7b66a0cb211a93cf475172aa0465a952009e1a59))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(f7a951c2) SHA1(12d7a6119d9033ae02c6312c9af888bfc7c63ad1))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(cc2aef4e) SHA1(a6e243de99f6a76eb527e879f4441c036dd379b6))
ROM_END

/*-------------------------------------------------------------------
/ Bad Girls
/-------------------------------------------------------------------*/
ROM_START(badgirls)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(583933ec) SHA1(89da6750d779d68db578715b058f9321695b79b0))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(956aeae0) SHA1(24d9d514fc83aba1ab310bfe4ed80605df399417))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(452dec20) SHA1(a9c41dfb2d83c5671ab96e946f13df774b567976))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(ab3b8e2d) SHA1(b57a0b804b42b923bb102d295e3b8a69b1033d27))
ROM_END

/*-------------------------------------------------------------------
/ Big House
/-------------------------------------------------------------------*/
ROM_START(bighouse)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(047c8ef5) SHA1(3afa2a0011b724836b69b2ef386597e0953dfadf))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(0ecef900) SHA1(78e4ed6e40fdb45dde2d0f2cf60d4c8a7ea2e39e))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(f330fd04) SHA1(1288c47f636d9d5b826a2b870b81788a630e489e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(0b1ba1cb) SHA1(26327689992018837b1c9957c515ab67248623eb))
ROM_END

/*-------------------------------------------------------------------
/ Bone Busters Inc.
/-------------------------------------------------------------------*/
ROM_START(bonebstr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(681643df) SHA1(76af6951e4403b4951298d35a9058bcebfa6bc43))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(052f97be) SHA1(0ee108e79c4196dffedc64d7f7a576e0394427c1))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom2.snd", 0x8000, 0x8000, CRC(d147d78d) SHA1(f8f6d6a1921685b883b224a9ea85ead52a32a4c3))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(ec43f4e9) SHA1(77b0988700be7a597dca7e5f06ac5d3c6834ce21))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(a95eedfc) SHA1(5ced2d6869a9895f8ff26d830b21d3c9364b32e7))
ROM_END

/*-------------------------------------------------------------------
/ Bounty Hunter (#694)
/-------------------------------------------------------------------*/
ROM_START(bountyh)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(e8190df7) SHA1(5304918d35e379da17ab19d8879a7ace5c864326))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("694-s.snd", 0x0800, 0x0800, CRC(a0383e41) SHA1(156514d2b52fcd89b608b85991c5066780949979))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Chicago Cubs' Triple Play (#696)
/-------------------------------------------------------------------*/
ROM_START(triplay)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(42b29b01) SHA1(58145ce10939d00faff49972ada669005a223792))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("696-s.snd", 0x0800, 0x0800, CRC(deedea61) SHA1(6aec221397f250d5dd99faefa313e8028c8818f7))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Diamond Lady
/-------------------------------------------------------------------*/
ROM_START(diamondp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(862951dc) SHA1(b15899ecf7ec869e3722cef3f5c16b0dadd2514e))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(7a011757) SHA1(cc49ec7451feae035670ea9d70cc8f6b32747c90))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(c216d1e4) SHA1(aa38db5ad36d1d1d35e727ab27c1f1c05a9627cd))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(0a18d626) SHA1(6b367668be55ca04c69c4c4c5a4a524ae8f790f8))
ROM_END

/*-------------------------------------------------------------------
/ Excalibur
/-------------------------------------------------------------------*/
ROM_START(excalibr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(499e2e41) SHA1(1e3fcba18882bd7df30a43843916aa5d7968eecc))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(ed1083d7) SHA1(3ff829ecfaba7d20c75268d3ee5224cb3cac3507))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(a4368cd0) SHA1(c48513e56899938dc83a3545d8ee9def3dc1491f))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(9f194744) SHA1(dbd73b546071c3d4f0dcfe21e3e646da716c5b71))
ROM_END

/*-------------------------------------------------------------------
/ Genesis (#705)
/-------------------------------------------------------------------*/
ROM_START(genesisp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(ac9f3a0f) SHA1(0e44888dc046121794e824d128628f991245c1cb))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(4a2f185c) SHA1(b45982b1ce9777292731ad523516c76cde4ddfa4))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(758e1743) SHA1(6df3011c044796afcd88e52d1ca69692cb489ff4))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(4869b0ec) SHA1(b8a56753257205af56e06105515b8a700bb1935b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0528c024) SHA1(d24ff7e088b08c1f35b54be3c806f8a8757d96c7))
ROM_END

/*-------------------------------------------------------------------
/ Gold Wings (#707)
/-------------------------------------------------------------------*/
ROM_START(goldwing)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(a5318c20) SHA1(8b4dcf45b13657ff753237a2e7d0352fda7755ef))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(bf242185) SHA1(0bf231050aa29f8bba5cb478a815b3d83bad93b3))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(892dbb21) SHA1(e24611544693e95dd2b9c0f2532c4d1f0b8ac10c))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(e17e9b1f) SHA1(ada9a6139a13ef31173801d560ec732d5a285140))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(4e482023) SHA1(62e97d229eb28ff67f0ebc4ee04c1b4918a4affe))
ROM_END

/*-------------------------------------------------------------------
/ Hollywood Heat
/-------------------------------------------------------------------*/
ROM_START(hlywoodh)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(a465e5f3) SHA1(56afa2f67aebcd17345bba76ecb814653719ee7b))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(0493e27a) SHA1(72c603cda3cc43ed0f841a9fcc6f40d020475e74))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(a698ec33) SHA1(e7c1d28279ec4f12095c3a106c6cefcc2a84b31e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(9232591e) SHA1(72883e0c542c572226c6c654bea14749cc9e351f))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(51709c2f) SHA1(5834d7b72bd36e30c87377dc7c3ad0cf26ff303a))
ROM_END

/*-------------------------------------------------------------------
/ Hot Shots
/-------------------------------------------------------------------*/
ROM_START(hotshots)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(7695c7db) SHA1(90188ff83b888262ba849e5af9d99145c5bc1c30))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(122ff4a8) SHA1(195392b9f2050b52392a123831bb7a9428087c1b))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(42c3cc3d) SHA1(26ca7f3a71b83df18ac6be1d1eb28da20120285e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(2933a80e) SHA1(5982b9ed361d90f8ea47047fc29770ef142acbec))
ROM_END

/*-------------------------------------------------------------------
/ Monte Carlo
/-------------------------------------------------------------------*/
ROM_START(mntecrlo)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(6860e315) SHA1(cecb1815334506dfebf29efe3e4e2a838010e8db))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(0fbf15a3) SHA1(0155b39c2c38224301857313ab784c1d39f1183b))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(1a53ac15) SHA1(f2751664a09431e908873580ddf4f44df9b4eda7))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(6e234c49) SHA1(fdb4126ecdaac378d144e9dd3c29b4e79290da2a))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(a95d1a6b) SHA1(91946ef7af0e4dd96db6d2d6f4f2e9a3a7279b81))
ROM_END

/*-------------------------------------------------------------------
/ Night Moves C-103
/-------------------------------------------------------------------*/
ROM_START(nmoves)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("nmovsp2.732", 0x1000, 0x0800, CRC(a2bc00e4) SHA1(5c3e9033f5c72b87058b2f70a0ff0811cc6770fa))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("nmovsp1.764", 0x2000, 0x2000, CRC(36837146) SHA1(88312ae1d1fe76defc4aa2d0a0570c5bb56253e9))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("nmovdrom.256", 0x8000, 0x8000, CRC(90929841) SHA1(e203ccd3552c9843c91fc49a437f60ae2dd49142))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("nmovyrom.256", 0x8000, 0x8000, CRC(cb74a687) SHA1(af8275807491eb35643cdeb6c898025fde47ceac))
ROM_END

/*-------------------------------------------------------------------
/ Raven
/-------------------------------------------------------------------*/
ROM_START(raven)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(481f3fb8) SHA1(22ffa55ed362219ebedbc40edcf866ff152a01b9))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(edc88561) SHA1(101878527307c6f04d141dd74e04102c4ea53105))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(a04bf7d0) SHA1(5be5d445b199e7dc9d42e7ee5e9b31c18dec3881))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(ee5f868b) SHA1(23ef4112b94109ad4d4a6b9bb5215acec20e5e55))
ROM_END

/*-------------------------------------------------------------------
/ Robo-War
/-------------------------------------------------------------------*/
ROM_START(robowars)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(893177ed) SHA1(791540a64d498979e5b0c8baf4ceb2fd5ff7f047))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(cd1587d8) SHA1(77e8e02dc03d052e9e4ce19c9431439e4211a29f))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(ea59b6a1) SHA1(6a4cdd37ba85f94f703afd1c5d3f102f51fedf46))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(7ecd8b67) SHA1(c5167b0acc64e535d389ba70be92a65672e119f6))
ROM_END

/*-------------------------------------------------------------------
/ Rock (#697)
/-------------------------------------------------------------------*/
ROM_START(rock)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(1146c1d3) SHA1(1e838756017cdc51239c082f8d491cd2824d273d))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(03830e81) SHA1(786f85eba5a8f5e9cc659305623e1d178b5410f6))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(effba2ad) SHA1(2288a4f655376e0aa18f8ecd9a3818ed4d6c6891))
ROM_END

/*-------------------------------------------------------------------
/ Rock Encore (#704)
/-------------------------------------------------------------------*/
ROM_START(rock_enc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(1146c1d3) SHA1(1e838756017cdc51239c082f8d491cd2824d273d))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1a.snd",0xe000,0x2000, CRC(b8aa8912) SHA1(abff690256c0030807b2d4dfa0516496516384e8))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1a.snd",0xe000,0x2000, CRC(a62e3b94) SHA1(59636c2ac7ebbd116a0eb39479c97299ba391906))
	ROM_LOAD("yrom2a.snd",0xc000,0x2000, CRC(66645a3f) SHA1(f06261af81e6b1829d639933297d2461a8c993fc))
ROM_END

/*-------------------------------------------------------------------
/ Spring Break
/-------------------------------------------------------------------*/
ROM_START(sprbreak)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(47171062) SHA1(0d2e7777f695ab22170be861019c05ddeade5f85))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(53ed608b) SHA1(555a6c02d637ea03e8265bb2b0fba95f2e2584b3))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(97d3f9ba) SHA1(1b34c7e51373c26d29d757c57a2b0333fe38d19e))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(5ea89df9) SHA1(98ce7661a4d862fd02c77e69b0f6e9372c3ade2b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0fb0128e) SHA1(3bdc5ed11b8e062f71f2a78b955830bd985e80a3))
ROM_END

ROM_START(sprbreaks)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.rv2", 0x1000, 0x0800, CRC(911cd14f) SHA1(2bc3ff6a3889da69b97f8ec318f93208e3d42cfe))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.rv2", 0x2000, 0x2000, CRC(d67d9d2f) SHA1(ebb82f0a1b7d6a2ec2607d4000e58fb6bfa73fe7))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(97d3f9ba) SHA1(1b34c7e51373c26d29d757c57a2b0333fe38d19e))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(5ea89df9) SHA1(98ce7661a4d862fd02c77e69b0f6e9372c3ade2b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0fb0128e) SHA1(3bdc5ed11b8e062f71f2a78b955830bd985e80a3))
ROM_END

/*-------------------------------------------------------------------
/ Tag-Team Wrestling (#698)
/-------------------------------------------------------------------*/
ROM_START(tagteamp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(fd1615ce) SHA1(3a6c3525552286b86e5340af2bf196f12adc9b35))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(65931038) SHA1(6d2f1a9fb1b3ce4610074fd3f2ac37ad6af70a44))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("698-s.snd", 0x0800, 0x0800, CRC(9c8191b7) SHA1(12b017692f078dcdc8e4bbf1ffcea1c5d0293d06))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

ROM_START(tagteamp2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2a.cpu", 0x1000, 0x0800, CRC(6d56b636) SHA1(8f50f2742be727835e7343307787b4b5daa1623a))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1a.cpu", 0x2000, 0x2000, CRC(92766607) SHA1(29744dd3c447cc51fb123750ae1456329122e986))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("698-s.snd", 0x0800, 0x0800, CRC(9c8191b7) SHA1(12b017692f078dcdc8e4bbf1ffcea1c5d0293d06))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ TX-Sector
/-------------------------------------------------------------------*/
ROM_START(txsector)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(f12514e6) SHA1(80bca17c33df99ed1a7acc21f7f70ea90e7c0463))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(e51d39da) SHA1(b6e4d573b62cc441a153cc4d8b647ee46b4dd2a7))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(61d66ca1) SHA1(59b1705b13d46b29f45257c566274f3cdce15ec2))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(469ef444) SHA1(faa16f34357a53c3fc61b59251fabdc44c605000))
ROM_END

/*-------------------------------------------------------------------
/ Victory
/-------------------------------------------------------------------*/
ROM_START(victoryp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(6a42eaf4) SHA1(3e28b01473266db463986a4283e1be85f2410fb1))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(e724db90) SHA1(10e760e129ce89f11372c6dd3616216d45f2c926))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(4ab6dab7) SHA1(7e21e69029e60052112ddd5c7481582ea6684dc1))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(921a100e) SHA1(0c3c7eae4ceeb5a1a8150bac52203d3f1e8f917e))
ROM_END


GAME(1987,	arena,	0,		gts80b_s1,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Arena",				GAME_IS_SKELETON_MECHANICAL)
GAME(1988,	badgirls,	0,		gts80b_s3,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Bad Girls",			GAME_IS_SKELETON_MECHANICAL)
GAME(1989,	bighouse,	0,		gts80b_s3,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Big House",			GAME_IS_SKELETON_MECHANICAL)
GAME(1989,	bonebstr,	0,		bonebstr,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Bone Busters Inc.",		GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	bountyh,	0,		gts80b_s,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Bounty Hunter",			GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	triplay,	0,		gts80b_s,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Triple Play",			GAME_IS_SKELETON_MECHANICAL)
GAME(1988,	diamondp,	0,		gts80b_s2,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Diamond Lady",			GAME_IS_SKELETON_MECHANICAL)
GAME(1988,	excalibr,	0,		gts80b_s3,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Excalibur",			GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	genesisp,	0,		gts80b_s1,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Genesis",				GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	goldwing,	0,		gts80b_s1,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Gold Wings",			GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	hlywoodh,	0,		gts80b_s1,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Hollywood Heat",			GAME_IS_SKELETON_MECHANICAL)
GAME(1989,	hotshots,	0,		gts80b_s2,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Hot Shots",			GAME_IS_SKELETON_MECHANICAL)
GAME(1987,	mntecrlo,	0,		gts80b_s1,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Monte Carlo (Pinball)",			GAME_IS_SKELETON_MECHANICAL)
GAME(1989,	nmoves,	0,		gts80b_s2,	gts80b,	gts80b,	ROT0,	"International Concepts",	"Night Moves",			GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	raven,	0,		gts80b_s1,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Raven",				GAME_IS_SKELETON_MECHANICAL)
GAME(1988,	robowars,	0,		gts80b_s2,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Robo-War",				GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	rock,		0,		gts80b_s1,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Rock",				GAME_IS_SKELETON_MECHANICAL)
GAME(1986,	rock_enc,	rock,		gts80b_s1,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Rock Encore",			GAME_IS_SKELETON_MECHANICAL)
GAME(1987,	sprbreak,	0,		gts80b_s1,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Spring Break",			GAME_IS_SKELETON_MECHANICAL)
GAME(19??,	sprbreaks,	sprbreak,gts80b_s1,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Spring Break (single ball game)",			GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	tagteamp,	0,		gts80b_s,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Tag-Team Wrestling",		GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	tagteamp2,	tagteamp,	gts80b_s,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Tag-Team Wrestling (rev.2)",	GAME_IS_SKELETON_MECHANICAL)
GAME(1988,	txsector,	0,		gts80b_s2,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"TX-Sector",			GAME_IS_SKELETON_MECHANICAL)
GAME(1987,	victoryp,	0,		gts80b_s2,	gts80b,	gts80b,	ROT0,	"Gottlieb",				"Victory (Pinball)",				GAME_IS_SKELETON_MECHANICAL)
