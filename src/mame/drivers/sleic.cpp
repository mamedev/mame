// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************************************

    PINBALL
    Sleic made a number of pinball machines (Pinball, Bike Race, Dona Elvira 2,
    Super Pang, Io Moon). The only manual I could find is in Spanish and has no schematics.

    Principal components:
    80C188-10
    80C39-11
    27C64
    27C040
    27C010
    28C64A
    6376 (Voice Synthesiser by OKI)
    YM3812 (Sound Generator by Yamaha)
    YM3014 (DAC Sounds by Yamaha)
    X9103 NVRAM
    Z80A
    27C256
    PinMAME also has a PIC8259.

    The only real source of info is PinMAME, but the game only partially works there.

****************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"

class sleic_state : public driver_device
{
public:
	sleic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void sleic(machine_config &config);
	void sleic_map(address_map &map);

	void init_sleic();

private:
	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
};


void sleic_state::sleic_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0xe0000, 0xfffff).rom();
}

static INPUT_PORTS_START( sleic )
INPUT_PORTS_END

void sleic_state::machine_reset()
{
}

void sleic_state::init_sleic()
{
}

void sleic_state::sleic(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sleic_state::sleic_map);
}

/*-------------------------------------------------------------------
/ Bike Race (1992)
/-------------------------------------------------------------------*/
ROM_START(bikerace)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("bkcpu04.bin", 0xe0000, 0x20000, CRC(ce745e89) SHA1(04ba97a9ef1e60a7609c87cf6d8fcae2d0e32621))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bkio07.bin", 0x0000, 0x8000, CRC(b52a9d4f) SHA1(726a4d9b354729d7390d2a4f877dc480701ec795))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("bkdsp01.bin", 0x0000, 0x2000, CRC(9b220fcb) SHA1(54e82705d8ce8a26d9e1b5f0fe382ded1f2070c3))

	ROM_REGION(0x100000, "user1", 0)
	ROM_LOAD("bksnd02.bin", 0x00000, 0x80000, CRC(d67b3883) SHA1(712022b9b24c6ab559d020ab8e2106f68b4d7896))
	ROM_LOAD("bksnd03.bin", 0x80000, 0x80000, CRC(b6d00245) SHA1(f7da6f2ca681fbe62ea9cab7f92d3e501b7e867d))

	ROM_REGION(0x100000, "user2", 0)
	ROM_LOAD("bkcpu05.bin", 0x00000, 0x20000, CRC(072ce879) SHA1(4f6fb044592feb4c72bbdcbe5f19e063c0e49d0d))
	ROM_LOAD("bkcpu06.bin", 0x20000, 0x20000, CRC(9db436d4) SHA1(3869524c0490e0a019d2f8ab46546ff42727665e))
ROM_END

ROM_START(bikerace2)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("04.bin", 0xe0000, 0x20000, CRC(aaaa4a8a) SHA1(ff579041575da4060615da2ff634f3aa91537751))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("07.bin", 0x0000, 0x8000, CRC(0b763a89) SHA1(8952d7b13674e1599e53cce96e57c2783899a90a))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("bkdsp01.bin", 0x0000, 0x2000, CRC(9b220fcb) SHA1(54e82705d8ce8a26d9e1b5f0fe382ded1f2070c3))

	ROM_REGION(0x100000, "user1", 0)
	ROM_LOAD("bksnd02.bin", 0x00000, 0x80000, CRC(d67b3883) SHA1(712022b9b24c6ab559d020ab8e2106f68b4d7896))
	ROM_LOAD("bksnd03.bin", 0x80000, 0x80000, CRC(b6d00245) SHA1(f7da6f2ca681fbe62ea9cab7f92d3e501b7e867d))

	ROM_REGION(0x100000, "user2", 0)
	ROM_LOAD("bkcpu05.bin", 0x00000, 0x20000, CRC(072ce879) SHA1(4f6fb044592feb4c72bbdcbe5f19e063c0e49d0d))
	ROM_LOAD("bkcpu06.bin", 0x20000, 0x20000, CRC(9db436d4) SHA1(3869524c0490e0a019d2f8ab46546ff42727665e))
ROM_END
/*-------------------------------------------------------------------
/ Dona Elvira 2 (1996)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Io Moon (1994)
/-------------------------------------------------------------------*/
ROM_START(iomoon)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("v1_3_01.bin", 0x80000, 0x80000, CRC(df80bf4f) SHA1(29547b444cad116c9dc925d6b3112f584df37250))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("v1_3_05.bin", 0x0000, 0x8000, CRC(6bb5e101) SHA1(125412953bbee7ee171c0bd34f7848fde37ace67))

	ROM_REGION(0x100000, "user1", 0)
	ROM_LOAD("v1_3_03.bin", 0x00000, 0x80000, CRC(334d0e20) SHA1(06b38cc7fcee633c45a9000187fcde8d7e03a51f))
	ROM_LOAD("v1_3_04.bin", 0x80000, 0x80000, CRC(f3a950bf) SHA1(e0410f8fe9b4efe7d21052c0a19894a563f90a27))

	ROM_REGION(0x100000, "user2", 0)
	ROM_LOAD("v1_3_02.bin", 0x00000, 0x80000, CRC(2bd589cd) SHA1(87354c76cbef8185d563266230c72a618ce6fcd7))
ROM_END
/*-------------------------------------------------------------------
/ Sleic Pin Ball (1993)
/-------------------------------------------------------------------*/
ROM_START(sleicpin)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("sp03-1_1.rom", 0xe0000, 0x20000, CRC(261b0ae4) SHA1(e7d9d1c2cab7776afb732701b0b8697b62a8d990))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sp01-1_1.rom", 0x0000, 0x2000, CRC(240015bb) SHA1(0e647718173ad59dafbf3b5bc84bef3c33886e23))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("sp04-1_1.rom", 0x0000, 0x8000, CRC(84514cfa) SHA1(6aa87b86892afa534cf963821f08286c126b4245))

	ROM_REGION(0x100000, "user1", 0)
	ROM_LOAD("sp02-1_1.rom", 0x00000, 0x80000, CRC(0e4851a0) SHA1(0692ee2df0b560e2013db9c03fd27c6eb12e618d))
ROM_END

GAME(1992,  bikerace,  0,         sleic,  sleic, sleic_state, init_sleic, ROT0, "Sleic", "Bike Race",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  bikerace2, bikerace,  sleic,  sleic, sleic_state, init_sleic, ROT0, "Sleic", "Bike Race (2-ball play)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  sleicpin,  0,         sleic,  sleic, sleic_state, init_sleic, ROT0, "Sleic", "Sleic Pin Ball",          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  iomoon,    0,         sleic,  sleic, sleic_state, init_sleic, ROT0, "Sleic", "Io Moon",                 MACHINE_IS_SKELETON_MECHANICAL)
