// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************************************

    PINBALL
    Sleic made a number of pinball machines (Pinball, Bike Race, Dona Elvira 2,
    Super Pang, Io Moon) but seems only Pinball is dumped. The only manual I could
    find is in Spanish and has no schematics.

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
	sleic_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
public:
	DECLARE_DRIVER_INIT(sleic);
};


static ADDRESS_MAP_START( sleic_map, AS_PROGRAM, 8, sleic_state )
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0xe0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( sleic )
INPUT_PORTS_END

void sleic_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(sleic_state,sleic)
{
}

static MACHINE_CONFIG_START( sleic, sleic_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, 8000000)
	MCFG_CPU_PROGRAM_MAP(sleic_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Bike Race (1992)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Dona Elvira 2 (1996)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Io Moon (1994)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Sleic Pin Ball (1994)
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

GAME(1993,  sleicpin,  0,  sleic,  sleic, sleic_state,  sleic,  ROT0,  "Sleic",    "Sleic Pin Ball",     MACHINE_IS_SKELETON_MECHANICAL)
