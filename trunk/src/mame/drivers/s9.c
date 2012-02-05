/*
    Williams System 9
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class williams_s9_state : public driver_device
{
public:
	williams_s9_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( williams_s9_map, AS_PROGRAM, 8, williams_s9_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( williams_s9 )
INPUT_PORTS_END

void williams_s9_state::machine_reset()
{
}

static DRIVER_INIT( williams_s9 )
{
}

static MACHINE_CONFIG_START( williams_s9, williams_s9_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(williams_s9_map)
MACHINE_CONFIG_END

/*--------------------
/ Comet (S9) 06/85
/--------------------*/
ROM_START(comet_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u20.128", 0x8000, 0x4000, CRC(36193600) SHA1(efdc44ef26c2def8f860a0296e27b2c3dac55ec8))
	ROM_RELOAD( 0xc000, 0x4000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("cpu_u49.128", 0xc000, 0x4000, CRC(f1db0cbe) SHA1(59b7f36fb2003b90b288abeff56df62ce50f10c6))
	ROM_LOAD("spch_u7.732", 0x8000, 0x1000, CRC(36545b22) SHA1(f4a026f3fa58dce81b439d76120a6769f4632955))
	ROM_LOAD("spch_u5.732", 0x9000, 0x1000, CRC(89f7ede5) SHA1(bbfbd991c9e005c2fa36d8458803b121f4933618))
	ROM_LOAD("spch_u6.732", 0xa000, 0x1000, CRC(6ba2aba6) SHA1(783b4e9b38db8677d91f86cb4805f0fa1ae8f856))
	ROM_LOAD("spch_u4.732", 0xb000, 0x1000, CRC(d0215c49) SHA1(4f0925a826199b6e8baa5e7fbff5cde9e31d505b))
ROM_END

ROM_START(comet_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u20.l5", 0x8000, 0x4000, CRC(d153d9ab) SHA1(0b97591b8ba35207b1427900486d69078ae122bc))
	ROM_RELOAD( 0xc000, 0x4000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("cpu_u49.128", 0xc000, 0x4000, CRC(f1db0cbe) SHA1(59b7f36fb2003b90b288abeff56df62ce50f10c6))
	ROM_LOAD("spch_u7.732", 0x8000, 0x1000, CRC(36545b22) SHA1(f4a026f3fa58dce81b439d76120a6769f4632955))
	ROM_LOAD("spch_u5.732", 0x9000, 0x1000, CRC(89f7ede5) SHA1(bbfbd991c9e005c2fa36d8458803b121f4933618))
	ROM_LOAD("spch_u6.732", 0xa000, 0x1000, CRC(6ba2aba6) SHA1(783b4e9b38db8677d91f86cb4805f0fa1ae8f856))
	ROM_LOAD("spch_u4.732", 0xb000, 0x1000, CRC(d0215c49) SHA1(4f0925a826199b6e8baa5e7fbff5cde9e31d505b))
ROM_END

/*--------------------
/ Sorcerer (S9) 03/85
/--------------------*/
ROM_START(sorcr_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u19.732", 0x5000, 0x1000, CRC(88b6837d) SHA1(d26b06342741443406a72ba48a70e82df62bb26e))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("cpu_u20.764", 0x6000, 0x2000, CRC(c235b692) SHA1(d3b97fad2d501c894570601b387933c7644f64e6))
	ROM_RELOAD( 0xe000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("cpu_u49.128", 0xc000, 0x4000, CRC(a0bae1e4) SHA1(dc5172aa1d59191d4119da20757cb2c2469f8fe3))
	ROM_LOAD("spch_u7.732", 0x8000, 0x1000, CRC(bba9ed18) SHA1(8e37ba8cb6bbc1e0afeef230088beda4513adddb))
	ROM_LOAD("spch_u5.732", 0x9000, 0x1000, CRC(d48c68ad) SHA1(b1391b87519ad47be3dcce7f8581f871e6a3669f))
	ROM_LOAD("spch_u6.732", 0xa000, 0x1000, CRC(a5c54d47) SHA1(4e1206412ecf52ae61c9df2055e0715749a6325d))
	ROM_LOAD("spch_u4.732", 0xb000, 0x1000, CRC(0c81902d) SHA1(6d8f703327e5c73a321fc4aa3a67ce68fff82d70))
ROM_END

ROM_START(sorcr_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u19.l2", 0x5000, 0x1000, CRC(faf738db) SHA1(a3b3f4160dc837ddf5379e1edb0eafeefcc11e3d))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("cpu_u20.l2", 0x6000, 0x2000, CRC(74fc8117) SHA1(c228c76ade670603f77bb324e6794ec6dd358285))
	ROM_RELOAD( 0xe000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("cpu_u49.128", 0xc000, 0x4000, CRC(a0bae1e4) SHA1(dc5172aa1d59191d4119da20757cb2c2469f8fe3))
	ROM_LOAD("spch_u7.732", 0x8000, 0x1000, CRC(bba9ed18) SHA1(8e37ba8cb6bbc1e0afeef230088beda4513adddb))
	ROM_LOAD("spch_u5.732", 0x9000, 0x1000, CRC(d48c68ad) SHA1(b1391b87519ad47be3dcce7f8581f871e6a3669f))
	ROM_LOAD("spch_u6.732", 0xa000, 0x1000, CRC(a5c54d47) SHA1(4e1206412ecf52ae61c9df2055e0715749a6325d))
	ROM_LOAD("spch_u4.732", 0xb000, 0x1000, CRC(0c81902d) SHA1(6d8f703327e5c73a321fc4aa3a67ce68fff82d70))
ROM_END

/*--------------------
/ Space Shuttle (S9) 12/84
/--------------------*/
ROM_START(sshtl_l7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u20.128", 0x8000, 0x4000, CRC(848ad54c) SHA1(4e4ce5fb970da37706472f94a27fd912e1ecb1a0))
	ROM_RELOAD( 0xc000, 0x4000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("cpu_u49.128", 0xc000, 0x4000, CRC(8050ae27) SHA1(e3f5e9398f61b075620ecd075617a8dac3c07d0e))
	ROM_LOAD("spch_u5.732", 0x9000, 0x1000, CRC(13edd4e5) SHA1(46c4052c31ddc20bb87445636f8fe3b6f7bff856))
	ROM_LOAD("spch_u6.732", 0xa000, 0x1000, CRC(cf48b2e7) SHA1(fe55419a5d40b3a4e8c02a92746b25a075b8efd3))
	ROM_LOAD("spch_u4.732", 0xb000, 0x1000, CRC(b0d03c5e) SHA1(46b952f71a7ecc03e22e427875f6e16a9d124067))
ROM_END

/*--------------------
/ Strike Zone (Shuffle) (#916)
/--------------------*/
ROM_START(szone_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("sz_u19r5.732", 0x5000, 0x1000, CRC(c79c46cb) SHA1(422ba74ae67bebbe02f85a9a8df0e3072f3cebc0))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("sz_u20r5.764", 0x6000, 0x2000, CRC(9b5b3be2) SHA1(fce051a60b6eecd9bc07273892b14046b251b372))
	ROM_RELOAD( 0xe000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("szs_u49.128", 0xc000, 0x4000, CRC(144c3c07) SHA1(57be6f336f200079cd698b13f8fa4755cf694274))
	ROM_RELOAD(0x8000, 0x4000)
ROM_END

ROM_START(szone_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("sz_u19r2.732", 0x5000, 0x1000, CRC(c0e4238b) SHA1(eae60ccd5b5001671cd6d2685fd588494d052d1e))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("sz_u20r2.764", 0x6000, 0x2000, CRC(91c08137) SHA1(86da08f346f85810fceceaa7b9824ab76a68da54))
	ROM_RELOAD( 0xe000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("szs_u49.128", 0xc000, 0x4000, CRC(144c3c07) SHA1(57be6f336f200079cd698b13f8fa4755cf694274))
	ROM_RELOAD(0x8000, 0x4000)
ROM_END

/*--------------------
/ Alley Cats (Shuffle) (#918)
/--------------------*/
ROM_START(alcat_l7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26_rev7.rom", 0x5000, 0x1000, CRC(4d274dd3) SHA1(80d72bd0f85ce2cac04f6d9f59dc1fcccc86d402))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("u27_rev7.rom", 0x6000, 0x2000, CRC(9c7faf8a) SHA1(dc1a561948b9a303f7924d7bebcd972db766827b))
	ROM_RELOAD( 0xe000, 0x2000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("acs_u21.bin", 0x0000, 0x8000, CRC(c54cd329) SHA1(4b86b10e60a30c4de5d97129074f5657447be676))
	ROM_LOAD("acs_u22.bin", 0x8000, 0x8000, CRC(56c1011a) SHA1(c817a3410c643617f3643897b8f529ae78546b0d))
ROM_END

GAME(1985,	comet_l4,		comet_l5,	williams_s9,	williams_s9,	williams_s9,	ROT0,	"Williams",	"Comet (L-4)",					GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	comet_l5,		0,			williams_s9,	williams_s9,	williams_s9,	ROT0,	"Williams",	"Comet (L-5)",					GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	sorcr_l1,		sorcr_l2,	williams_s9,	williams_s9,	williams_s9,	ROT0,	"Williams",	"Sorcerer (L-1)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	sorcr_l2,		0,			williams_s9,	williams_s9,	williams_s9,	ROT0,	"Williams",	"Sorcerer (L-2)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1984,	sshtl_l7,		0,			williams_s9,	williams_s9,	williams_s9,	ROT0,	"Williams",	"Space Shuttle (L-7)",			GAME_IS_SKELETON_MECHANICAL)
GAME(1985,	alcat_l7,		0,			williams_s9,	williams_s9,	williams_s9,	ROT0,	"Williams",	"Alley Cats (Shuffle) (L-7)",	GAME_IS_SKELETON_MECHANICAL)
GAME(1984,	szone_l5,		0,			williams_s9,	williams_s9,	williams_s9,	ROT0,	"Williams",	"Strike Zone (Shuffle) (L-5)",	GAME_IS_SKELETON_MECHANICAL)
GAME(1984,	szone_l2,		szone_l5,	williams_s9,	williams_s9,	williams_s9,	ROT0,	"Williams",	"Strike Zone (Shuffle) (L-2)",	GAME_IS_SKELETON_MECHANICAL)
