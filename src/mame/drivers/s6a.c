/*
    Williams System 6a
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class williams_s6a_state : public driver_device
{
public:
	williams_s6a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( williams_s6a_map, AS_PROGRAM, 8, williams_s6a_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( williams_s6a )
INPUT_PORTS_END

void williams_s6a_state::machine_reset()
{
}

static DRIVER_INIT( williams_s6a )
{
}

static MACHINE_CONFIG_START( williams_s6a, williams_s6a_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(williams_s6a_map)
MACHINE_CONFIG_END

/*--------------------------
/ Algar - Sys.6 (Game #499)
/-------------------------*/
ROM_START(algar_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(6711da23) SHA1(80a46f5a2630977bc1c6e17466e8865083eb9a18))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound4.716", 0x7800, 0x0800, CRC(67ea12e7) SHA1(f81e97183442736d5766a7e5e074bc6539e8ced0))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------
/ Alien Poker - Sys.6 (Game #501)
/-------------------------------*/
ROM_START(alpok_l6)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom6.716", 0x6000, 0x0800, CRC(20538a4a) SHA1(6cdd6b7ded76b3cbd954d371e126e1bbd95a6219))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound3.716", 0x7800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("v_ic7.532", 0x3000, 0x1000, CRC(a66c7ca6) SHA1(6e90081f853fcf66bfeac0a8ee1c762b3760b90b))
	ROM_RELOAD( 0xb000, 0x1000)
	ROM_LOAD("v_ic5.532", 0x4000, 0x1000, CRC(f16a237a) SHA1(a904138fad5cbc19946bcf0de824e27537dcd621))
	ROM_RELOAD( 0xc000, 0x1000)
	ROM_LOAD("v_ic6.532", 0x5000, 0x1000, CRC(15a3cc85) SHA1(86002ac78189415ae912e8bc23c92b3b67610d87))
	ROM_RELOAD( 0xd000, 0x1000)
ROM_END

ROM_START(alpok_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(79c07603) SHA1(526a45b139394e475fc052636e98d880a8908168))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound3.716", 0x7800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("v_ic7.532", 0x3000, 0x1000, CRC(a66c7ca6) SHA1(6e90081f853fcf66bfeac0a8ee1c762b3760b90b))
	ROM_RELOAD( 0xb000, 0x1000)
	ROM_LOAD("v_ic5.532", 0x4000, 0x1000, CRC(f16a237a) SHA1(a904138fad5cbc19946bcf0de824e27537dcd621))
	ROM_RELOAD( 0xc000, 0x1000)
	ROM_LOAD("v_ic6.532", 0x5000, 0x1000, CRC(15a3cc85) SHA1(86002ac78189415ae912e8bc23c92b3b67610d87))
	ROM_RELOAD( 0xd000, 0x1000)
ROM_END

ROM_START(alpok_f6)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom6.716", 0x6000, 0x0800, CRC(20538a4a) SHA1(6cdd6b7ded76b3cbd954d371e126e1bbd95a6219))
	ROM_RELOAD( 0xe000, 0x0800)
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_RELOAD( 0xf000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound3.716", 0x7800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("5t5014fr.dat", 0x3000, 0x1000, CRC(1d961517) SHA1(c71ee324becfc8cdbecabd1e64b11b5a39ff2483))
	ROM_RELOAD( 0xb000, 0x1000)
	ROM_LOAD("5t5015fr.dat", 0x4000, 0x1000, CRC(8d065f80) SHA1(0ab22c9b20ab6fe41abab620435ad03652db7a8e))
	ROM_RELOAD( 0xc000, 0x1000)
	ROM_LOAD("5t5016fr.dat", 0x5000, 0x1000, CRC(0ddf91e9) SHA1(48f5fdfc0c5a66dd318fecb7c90e5f5a684a3876))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("5t5017fr.dat", 0x6000, 0x1000, CRC(7e546dc1) SHA1(58f8286403978b0d929987189089881d754a9a83))
	ROM_RELOAD( 0xe000, 0x1000)
ROM_END


GAME(1980,	algar_l1,		0,			williams_s6a,	williams_s6a,	williams_s6a,	ROT0,	"Williams",				"Algar (L-1)",								GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	alpok_l6,		0,			williams_s6a,	williams_s6a,	williams_s6a,	ROT0,	"Williams",				"Alien Poker (L-6)",						GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	alpok_l2,		alpok_l6,	williams_s6a,	williams_s6a,	williams_s6a,	ROT0,	"Williams",				"Alien Poker (L-2)",						GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	alpok_f6,		alpok_l6,	williams_s6a,	williams_s6a,	williams_s6a,	ROT0,	"Williams",				"Alien Poker (L-6 French speech)",			GAME_IS_SKELETON_MECHANICAL)
