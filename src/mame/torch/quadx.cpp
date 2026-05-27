// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Torch Quad X

    UNIX workstation with VME bus

    Hardware:
    - MC68020RC16B CPU
    - MC68881RC20A FPU
    - XC68851RC16A MMU
    - Torch OpenChip DMA controller
    - ROM 64k + 4x8k PROM with microcode for the DMA controller
    - INMOS IMSG170S50 (video)
    - WD33C93JM (SCSI)
    - SEEQ DQ8003 + DQ8020 (ethernet)
    - SCN68562 (DUART)
    - RTC (unknown model)
    - X2444P NOVRAM
    - XTALs: 14.7456 MHz, 16.6667 MHz, 20 MHz, 32 MHz

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

#include "cpu/m68000/m68020.h"
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
	void quadxp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<m68000_musashi_device> m_maincpu;
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
	M68020(config, m_maincpu, 32_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadx_state::mem_map);
}

void quadx_state::quadxp(machine_config &config)
{
	M68030(config, m_maincpu, 50_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadx_state::mem_map);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( quadx )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("torch_quadx.bin", 0x00000, 0x10000, CRC(02f8d5e8) SHA1(c9312fb87156eae1e05a2332e7a4fbbd72cb1f40)) // M27C512

	// microcode for the dma controller "(C) Copyright 1988 TORCH Computers Ltd. v1.08m16.7"
	ROM_REGION(0x8000, "microcode", 0)
	ROM_LOAD32_BYTE("micro0.bin", 0x0000, 0x2000, CRC(0b66995f) SHA1(f1d1f437f8d523946352baa93848e875644eb32d)) // 1st half empty
	ROM_LOAD32_BYTE("micro1.bin", 0x0001, 0x2000, CRC(eea8b592) SHA1(26ad2374cce8c25841f9c4708d21e190756838ee)) // 1st half empty
	ROM_LOAD32_BYTE("micro2.bin", 0x0002, 0x2000, CRC(4d91698f) SHA1(ce15c37f55c3f9fae3ef249442c9e115ef61f588)) // 1st half empty
	ROM_LOAD32_BYTE("micro3.bin", 0x0003, 0x2000, CRC(9b754db8) SHA1(9b44dd1ae3806969ddceb615954237e2cafed065)) // 1st half empty
ROM_END

ROM_START( quadxp )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("torch_quadx_proto.bin", 0x00000, 0x10000, CRC(2d71bc67) SHA1(999f478cda5dc6b9da845ae8580789da6292fc75)) // M27C512

	// microcode for the dma controller "(C) Copyright 1990 CUBE i.t. Ltd. v2.00t"
	ROM_REGION(0x8000, "microcode", 0)
	ROM_LOAD32_BYTE("qy-0.bin", 0x0000, 0x2000, CRC(8ca1709a) SHA1(5b08e20b59c5b513a2710771a53576a9b3052a1c)) // 2nd half empty
	ROM_LOAD32_BYTE("qy-1.bin", 0x0001, 0x2000, CRC(dfccd7c3) SHA1(18182053ba817c3f72a319dbeb0ebc497250d636)) // 2nd half empty
	ROM_LOAD32_BYTE("qy-2.bin", 0x0002, 0x2000, CRC(114d6c9a) SHA1(611cb4a349f3a0686e9d66547492b3d5d76f4a3b)) // 2nd half empty
	ROM_LOAD32_BYTE("qy-3.bin", 0x0003, 0x2000, CRC(5d4999f6) SHA1(b3c5aee014bfbbe67afb970d10eb87a81b3733ef)) // 2nd half empty
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT  CLASS        INIT        COMPANY  FULLNAME                              FLAGS
COMP( 1988, quadx,  0,      0,      quadx,    quadx, quadx_state, empty_init, "Torch Computers", "Quad X",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1990, quadxp, 0,      0,      quadxp,   quadx, quadx_state, empty_init, "Torch Computers", "Quad X (68030 prototype)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
