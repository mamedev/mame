#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6809/m6809.h"

class wpc_flip1_state : public driver_device
{
public:
	wpc_flip1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( wpc_flip1_map, AS_PROGRAM, 8, wpc_flip1_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( wpc_flip1 )
INPUT_PORTS_END

void wpc_flip1_state::machine_reset()
{
}

static DRIVER_INIT( wpc_flip1 )
{
}

static MACHINE_CONFIG_START( wpc_flip1, wpc_flip1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 2000000)
	MCFG_CPU_PROGRAM_MAP(wpc_flip1_map)
MACHINE_CONFIG_END

/*-----------------
/  The Addams Family
/------------------*/
ROM_START(taf_p2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "user1", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "user2", 0)
	ROM_LOAD("addam_p2.rom", 0x00000, 0x40000, CRC(eabf0e72) SHA1(5b84d0315702b39b90beb6a92fb7ad9aba7e620c))
	ROM_REGION(0x010000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("afsnd_p2.rom", 0x000000, 0x80000, CRC(73d19698) SHA1(d14a6ea36a93db185a599a7810dfbef2deb0adc0))
ROM_END

ROM_START(taf_l1)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "user1", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "user2", 0)
	ROM_LOAD("addam_l1.rom", 0x00000, 0x40000, CRC(db287bf7) SHA1(51574c7c04d85aa816a0bc6e9db74f2d2b407525))
	ROM_REGION(0x010000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x000000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "user1", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "user2", 0)
	ROM_LOAD("addam_l2.rom", 0x00000, 0x40000, CRC(952bfc92) SHA1(d95b4b9e6c496a9ce4ceb1aa368c862b2beeffd9))
	ROM_REGION(0x010000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x000000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l3)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "user1", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "user2", 0)
	ROM_LOAD("addam_l3.rom", 0x00000, 0x40000, CRC(d428a760) SHA1(29afee7b1ae64d7a41faf813cdfa1ab7cef1f247))
	ROM_REGION(0x010000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x000000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l4)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "user1", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "user2", 0)
	ROM_LOAD("addam_l4.rom", 0x00000, 0x40000, CRC(ea29935f) SHA1(9f711396728026546c8bd1f69a0833d15e02c192))
	ROM_REGION(0x010000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x000000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l7)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "user1", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user2", 0)
	ROM_LOAD("addam_l7.rom", 0x00000, 0x80000, CRC(4401b43a) SHA1(64e9678334cc900d1f44b95d25bb90c1fff566f8))
	ROM_REGION(0x010000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x000000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l5)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "user1", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user2", 0)
	ROM_LOAD("addam_l5.rom", 0x00000, 0x80000, CRC(4c071564) SHA1(d643506db1b3ba1ea20f34ddb38837df379fb5ab))
	ROM_REGION(0x010000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x000000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l6)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "user1", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user2", 0)
	ROM_LOAD("taf_l6.u6", 0x00000, 0x80000, CRC(06b37e65) SHA1(ce6f9cc45df08f50f5ece2a4c9376ecf67b0466a))
	ROM_REGION(0x010000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x000000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_h4)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "user1", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user2", 0)
	ROM_LOAD("addam_h4.rom", 0x00000, 0x80000, CRC(d0bbd679) SHA1(ebd8c4981dd68a4f8e2dea90144486cb3cbd6b84))
	ROM_REGION(0x010000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x000000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

/*--------------
/  Game drivers
/---------------*/
GAME(1992,  taf_l5,  0,       wpc_flip1,  wpc_flip1,  wpc_flip1,  ROT0,  "Bally",    "The Addams Family (L-5)",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,  taf_p2,  taf_l5,  wpc_flip1,  wpc_flip1,  wpc_flip1,  ROT0,  "Bally",    "The Addams Family (Prototype) (P-2)",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,  taf_l1,  taf_l5,  wpc_flip1,  wpc_flip1,  wpc_flip1,  ROT0,  "Bally",    "The Addams Family (L-1)",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,  taf_l2,  taf_l5,  wpc_flip1,  wpc_flip1,  wpc_flip1,  ROT0,  "Bally",    "The Addams Family (L-2)",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,  taf_l3,  taf_l5,  wpc_flip1,  wpc_flip1,  wpc_flip1,  ROT0,  "Bally",    "The Addams Family (L-3)",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,  taf_l4,  taf_l5,  wpc_flip1,  wpc_flip1,  wpc_flip1,  ROT0,  "Bally",    "The Addams Family (L-4)",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,  taf_l7,  taf_l5,  wpc_flip1,  wpc_flip1,  wpc_flip1,  ROT0,  "Bally",    "The Addams Family (Prototype L-5) (L-7)",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,  taf_l6,  taf_l5,  wpc_flip1,  wpc_flip1,  wpc_flip1,  ROT0,  "Bally",    "The Addams Family (L-6)",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,  taf_h4,  taf_l5,  wpc_flip1,  wpc_flip1,  wpc_flip1,  ROT0,  "Bally",    "The Addams Family (H-4)",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
