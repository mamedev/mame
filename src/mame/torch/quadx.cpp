// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Torch Quad X

    UNIX workstation with VME bus

    Hardware (68030 prototype):
    - MC68030RC25B CPU
    - MC68881RC25B FPU
    - Torch OpenChip DMA controller
    - ROM 64k + 4x8k PROM with microcode for the DMA controller
    - VTC VIC068PG
    - INMOS IMSG300G (video)
    - WD33C93A (SCSI)
    - SEEQ NQ8005A (ethernet)
    - SCN68562 (DUART)
    - DS1216 (RTC)
    - X2444P NOVRAM
    - XTALs: 50 MHz (next to DUART), 64 MHz (next do DMA controller)
    - 16 position rotary switch

    TODO:
    - Everything

    Notes:
    - Currently dumped is a 68030 based prototype. The standard Quad X
      uses a 68020.

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68030.h"
#include "machine/ds1215.h"
#include "machine/eepromser.h"
#include "machine/scnxx562.h"
#include "machine/wd33c9x.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class quadx_state : public driver_device
{
public:
	quadx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void quadx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<m68030_device> m_maincpu;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void quadx_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom().region("maincpu", 0);
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( quadx )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void quadx_state::machine_start()
{
}

void quadx_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void quadx_state::quadx(machine_config &config)
{
	M68030(config, m_maincpu, 50_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadx_state::mem_map);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( quadx )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("torch_quadx_proto.bin", 0x00000, 0x10000, CRC(2d71bc67) SHA1(999f478cda5dc6b9da845ae8580789da6292fc75)) // M27C512
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
COMP( 1990, quadx, 0,      0,      quadx,   quadx, quadx_state, empty_init, "Torch Computers", "Quad X (68030 prototype)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
