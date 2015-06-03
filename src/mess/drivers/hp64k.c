// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// ***************************************
// Driver for HP 64000 development system
// ***************************************
//

#include "emu.h"
#include "cpu/hphybrid/hphybrid.h"

class hp64k_state : public driver_device
{
public:
    hp64k_state(const machine_config &mconfig, device_type type, const char *tag);

private:
    required_device<hp_5061_3011_cpu_device> m_cpu;
};

static ADDRESS_MAP_START(cpu_mem_map , AS_PROGRAM , 16 , hp64k_state)
    AM_RANGE(0x0000 , 0x3fff) AM_ROM
    AM_RANGE(0x8000 , 0xffff) AM_RAM
ADDRESS_MAP_END

hp64k_state::hp64k_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig , type , tag),
            m_cpu(*this , "cpu")
{
}

static MACHINE_CONFIG_START(hp64k , hp64k_state)
    MCFG_CPU_ADD("cpu" , HP_5061_3011 , 6250000)
    MCFG_CPU_PROGRAM_MAP(cpu_mem_map)
    MCFG_QUANTUM_TIME(attotime::from_hz(100))
MACHINE_CONFIG_END

ROM_START(hp64k)
    ROM_REGION(0x8000 , "cpu" , ROMREGION_16BIT | ROMREGION_BE | ROMREGION_INVERT)
    ROM_LOAD16_BYTE("64100_80022.bin" , 0x0000 , 0x1000 , CRC(38b2aae5))
    ROM_LOAD16_BYTE("64100_80020.bin" , 0x0001 , 0x1000 , CRC(ac01b436))
    ROM_LOAD16_BYTE("64100_80023.bin" , 0x2000 , 0x1000 , CRC(6b4bc2ce))
    ROM_LOAD16_BYTE("64100_80021.bin" , 0x2001 , 0x1000 , CRC(74f9d33c))
    ROM_LOAD16_BYTE("64100_80026.bin" , 0x4000 , 0x1000 , CRC(a74e834b))
    ROM_LOAD16_BYTE("64100_80024.bin" , 0x4001 , 0x1000 , CRC(2e15a1d2))
    ROM_LOAD16_BYTE("64100_80027.bin" , 0x6000 , 0x1000 , CRC(b93c0e7a))
    ROM_LOAD16_BYTE("64100_80025.bin" , 0x6001 , 0x1000 , CRC(e6353085))
ROM_END

/*    YEAR  NAME       PARENT    COMPAT MACHINE INPUT     INIT              COMPANY       FULLNAME */
COMP( 1979, hp64k,     0,        0,     hp64k,  0,    driver_device, 0, "HP",      "HP 64000" , GAME_NO_SOUND)
