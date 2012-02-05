#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/mcs51/mcs51.h"

class spinb_state : public driver_device
{
public:
	spinb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( spinb_map, AS_PROGRAM, 8, spinb_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( spinb )
INPUT_PORTS_END

void spinb_state::machine_reset()
{
}

static DRIVER_INIT( spinb )
{
}

static MACHINE_CONFIG_START( spinb, spinb_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8051, 16000000)
	MCFG_CPU_PROGRAM_MAP(spinb_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Bushido (1993) - ( Last game by Inder - before becoming Spinball - but same hardware)
/-------------------------------------------------------------------*/
ROM_START(bushido)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("0-z80.bin", 0x0000, 0x2000, CRC(3ea1eb1d) SHA1(cceb6c68e481f36a5646ff4f38d3dfc4275b0c79))
	ROM_LOAD("1-z80.old", 0x2000, 0x2000, CRC(648da72b) SHA1(1005a13b4746e302d979c8b1da300e943cdcab3d))
	ROM_REGION(0x32001, "cpu2", 0)
	ROM_LOAD("g-disply.bin", 0x00000, 0x10000, CRC(9a1df82f) SHA1(4ad6a12ae36ec898b8ac5243da6dec3abcd9dc33))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("a-sonido.bin", 0x00000, 0x2000, CRC(cf7d5399) SHA1(c79145826cfa6be2487e3add477d9b452c553762))
	ROM_REGION(0x180000, "user1", 0)
	ROM_LOAD("b-sonido.bin", 0x0000, 0x80000, CRC(cb4fc885) SHA1(569f389fa8f91f886b58f44f701d2752ef01f3fa))
	ROM_LOAD("c-sonido.bin", 0x80000, 0x80000, CRC(35a43dd8) SHA1(f2b1994f67f749c65a88c95d970b655990d85b96))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("d-musica.bin", 0x00000, 0x2000, CRC(2cb9697c) SHA1(d5c66d616ccd5e299832704e494743429dafd569))
	ROM_REGION(0x180000, "user2", 0)
	ROM_LOAD("e-musica.bin", 0x0000, 0x80000, CRC(1414b921) SHA1(5df9e538ee109df28953ec8f162c60cb8c6e4d96))
	ROM_LOAD("f-musica.bin", 0x80000, 0x80000, CRC(80f3a6df) SHA1(e09ad4660e511779c6e55559fa0c2c0b0c6600c8))
ROM_END

ROM_START(bushidoa)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("0-cpu.bin", 0x0000, 0x2000, CRC(7f7e6642) SHA1(6872397eed7525f384b79cdea13531d273d8cf14))
	ROM_LOAD("1-cpu.bin", 0x2000, 0x2000, CRC(a538d37f) SHA1(d2878ad0d31b4221b823812485c7faaf666ce185))
	ROM_REGION(0x32001, "cpu2", 0)
	ROM_LOAD("g-disply.bin", 0x00000, 0x10000, CRC(9a1df82f) SHA1(4ad6a12ae36ec898b8ac5243da6dec3abcd9dc33))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("a-sonido.bin", 0x00000, 0x2000, CRC(cf7d5399) SHA1(c79145826cfa6be2487e3add477d9b452c553762))
	ROM_REGION(0x180000, "user1", 0)
	ROM_LOAD("b-sonido.bin", 0x0000, 0x80000, CRC(cb4fc885) SHA1(569f389fa8f91f886b58f44f701d2752ef01f3fa))
	ROM_LOAD("c-sonido.bin", 0x80000, 0x80000, CRC(35a43dd8) SHA1(f2b1994f67f749c65a88c95d970b655990d85b96))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("d-musica.bin", 0x00000, 0x2000, CRC(2cb9697c) SHA1(d5c66d616ccd5e299832704e494743429dafd569))
	ROM_REGION(0x180000, "user2", 0)
	ROM_LOAD("e-musica.bin", 0x0000, 0x80000, CRC(1414b921) SHA1(5df9e538ee109df28953ec8f162c60cb8c6e4d96))
	ROM_LOAD("f-musica.bin", 0x80000, 0x80000, CRC(80f3a6df) SHA1(e09ad4660e511779c6e55559fa0c2c0b0c6600c8))
ROM_END

/*-------------------------------------------------------------------
/ Jolly Park (1996)
/-------------------------------------------------------------------*/
ROM_START(jolypark)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jpcpu0.rom", 0x0000, 0x2000, CRC(061967af) SHA1(45048e1d9f17efa3382460fd474a5aeb4191d617))
	ROM_LOAD("jpcpu1.rom", 0x2000, 0x2000, CRC(ea99202f) SHA1(e04825e73fd25f6469b3315f063f598ea1ab44c7))
	ROM_REGION(0x32001, "cpu2", 0)
	ROM_LOAD("jpdmd0.rom", 0x00000, 0x10000, CRC(b57565cb) SHA1(3fef66d298893029de78fdb6ecdb562c33d76180))
	ROM_LOAD("jpdmd1.rom", 0x12000, 0x20000, CRC(40d1563f) SHA1(90dbea742202340da6fa950eedc2bceec5a2af7e))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("jpsndc1.rom", 0x00000, 0x2000, CRC(0475318f) SHA1(7154bd5ca5b28019eb0ff598ec99bbe49260932b))
	ROM_REGION(0x180000, "user1", 0)
	ROM_LOAD("jpsndm4.rom", 0x0000, 0x80000, CRC(735f3db7) SHA1(81dc893f5194d6ac1af54b262555a40c5c3e0292))
	ROM_LOAD("jpsndm5.rom", 0x80000, 0x80000, CRC(769374bd) SHA1(8121369714c55cc06c493b15e5c2ca79b13aff52))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("jpsndc0.rom", 0x00000, 0x2000, CRC(a97259dc) SHA1(58dea3f36b760112cfc32d306077da8cf6cdec5a))
	ROM_REGION(0x180000, "user2", 0)
	ROM_LOAD("jpsndm1.rom", 0x0000, 0x80000, CRC(fc91d2f1) SHA1(c838a0b31bbec9dbc96b46d692c8d6f1286fe46a))
	ROM_LOAD("jpsndm2.rom", 0x80000, 0x80000, CRC(fb2d1882) SHA1(fb0ef9def54d9163a46354a0df0757fac6cbd57c))
	ROM_LOAD("jpsndm3.rom", 0x100000, 0x80000, CRC(77e515ba) SHA1(17b635d107c437bfc809f8cc1a6cd063cef12691))
ROM_END

/*-------------------------------------------------------------------
/ Mach 2 (1995)
/-------------------------------------------------------------------*/
ROM_START(mach2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("m2cpu0.19", 0x0000, 0x2000, CRC(274c8040) SHA1(6b039b79b7e08f2bf2045bc4f1cbba790c999fed))
	ROM_LOAD("m2cpu1.19", 0x2000, 0x2000, CRC(c445df0b) SHA1(1f346c1df8df0a3c4e8cb1186280d2f34959b3f8))
	ROM_REGION(0x32001, "cpu2", 0)
	ROM_LOAD("m2dmdf.01", 0x00000, 0x10000, CRC(c45ccc74) SHA1(8362e799a76536a16dd2d5dde500ad3db273180f))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("m2sndd.01", 0x00000, 0x2000, CRC(e789f22d) SHA1(36aa7eac1dd37a02c982d109462dddbd85a305cc))
	ROM_REGION(0x180000, "user1", 0)
	ROM_LOAD("m2snde.01", 0x0000, 0x80000, CRC(f5721119) SHA1(9082198e8d875b67323266c4bf8c2c378b63dfbb))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("m2musa.01", 0x00000, 0x2000, CRC(2d92a882) SHA1(cead22e434445e5c25414646b1e9ae2b9457439d))
	ROM_REGION(0x180000, "user2", 0)
	ROM_LOAD("m2musb.01", 0x0000, 0x80000, CRC(6689cd19) SHA1(430092d51704dfda8bd8264875f1c1f4461c56e5))
	ROM_LOAD("m2musc.01", 0x80000, 0x80000, CRC(88851b82) SHA1(d0c9fa391ca213a69b7c8ae7ca52063503b5656e))
ROM_END

/*-------------------------------------------------------------------
/ Verne's World (1996)
/-------------------------------------------------------------------*/

GAME(1993,  bushido,   0,       spinb,  spinb,  spinb,  ROT0,  "Inder/Spinball",    "Bushido",					GAME_IS_SKELETON_MECHANICAL)
GAME(1993,  bushidoa,  bushido, spinb,  spinb,  spinb,  ROT0,  "Inder/Spinball",    "Bushido (alternate set)",	GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  jolypark,  0,       spinb,  spinb,  spinb,  ROT0,  "Spinball",          "Jolly Park",				GAME_IS_SKELETON_MECHANICAL)
GAME(1995,  mach2,     0,       spinb,  spinb,  spinb,  ROT0,  "Spinball",          "Mach 2",					GAME_IS_SKELETON_MECHANICAL)
