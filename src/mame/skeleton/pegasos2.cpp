// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    PEGASOS II

    Hardware:
    - PowerPC 750CXe (600 MHz) or MPC7447 (1 GHz)
    - Marvell Discovery II MV64361 northbridge
    - VIA 8231 southbridge
    - W83194 clock generator
    - DDR SDRAM memory, up to 2 GB
    - 1x AGP, 3x PCI slots
    - 2xLAN (Gigabit from northbridge, 100 MBit from southbridge)
    - VIA VT6306 (firewire)
    - AC97 sound (Sigmatel STAC 9766 Codec)
    - Floppy
    - PS/2 keyboard/mouse
    - Joystick

    TODO:
    - Everything

    Notes:
    - Designed by bplan GmbH

***************************************************************************/

#include "emu.h"
#include "cpu/powerpc/ppc.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pegasos2_state : public driver_device
{
public:
	pegasos2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void pegasos2(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;

	void mem_map(address_map &map);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void pegasos2_state::mem_map(address_map &map)
{
	map(0xfff00000, 0xfff7ffff).rom().region("maincpu", 0);
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( pegasos2 )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void pegasos2_state::machine_start()
{
}

void pegasos2_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void pegasos2_state::pegasos2(machine_config &config)
{
	PPC604(config, m_maincpu, 100000000); // wrong cpu/clock
	m_maincpu->set_addrmap(AS_PROGRAM, &pegasos2_state::mem_map);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pegasos2 )
	ROM_REGION(0x80000, "maincpu", 0)
	// extracted from 'up050404'
	ROM_LOAD("pegasos2.rom", 0x00000, 0x80000, CRC(7e992266) SHA1(08dc28afb3d10fb223376a28eebfd07c9f8df9fa))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME      FLAGS
COMP( 2003, pegasos2, 0,      0,      pegasos2, pegasos2, pegasos2_state, empty_init, "Genesi", "PEGASOS II", MACHINE_IS_SKELETON )
