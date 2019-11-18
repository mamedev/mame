// license:BSD-3-Clause
// copyright-holders:Kaz
/***********************************************************************************************************

 NES/Famicom cartridge emulation for Zemina PCBs

 ***********************************************************************************************************/

#include "emu.h"
#include "zemina.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(...) do { if (VERBOSE) logerror(__VA_ARGS__); } while (0)

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_ZEMINA, nes_zemina_device, "nes_zemina", "NES Cart Zemina PCB")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_zemina_device - constructor
//-------------------------------------------------

nes_zemina_device::nes_zemina_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_ZEMINA, tag, owner, clock)
{
}

/*-------------------------------------------------
    device_start
-------------------------------------------------*/

void nes_zemina_device::device_start()
{
	common_start();
}

/*-------------------------------------------------
    pcb_reset
-------------------------------------------------*/

void nes_zemina_device::pcb_reset()
{
	set_nt_mirroring(PPU_MIRROR_VERT);
	chr2_0(0, CHRROM);
	chr2_2(0, CHRROM);
	chr2_4(0, CHRROM);
	chr2_6(0, CHRROM);
	prg16_89ab(0);
	prg16_cdef(0);
}

/*-------------------------------------------------
    mapper specific handlers
-------------------------------------------------*/

/*-------------------------------------------------

 Zemina board emulation

 Currently, this board is only known to be used
 by one game: Magic Kid GooGoo.

 Info from kevtris at NESDev, who dumped the game:
 wiki.nesdev.com/w/index.php/INES_Mapper_190

-------------------------------------------------*/

/*-------------------------------------------------
    write
-------------------------------------------------*/

void nes_zemina_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC("zemina write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x0000 && offset <= 0x1FFF)
	{
		prg16_89ab(data & 0x07);
	}
	else if (offset >= 0x4000 && offset <= 0x5FFF)
	{
		prg16_89ab((data & 0x07) | 0x08);
	}
	else if ((offset & 0x2000) == 0x2000) // 2K CHR banks
	{
		switch (offset & 0x03) // only A0, A1, A13, A14, and A15 are used to select the CHR bank
		{
			case 0x00:
				chr2_0(data, CHRROM);
				break;
			case 0x01:
				chr2_2(data, CHRROM);
				break;
			case 0x02:
				chr2_4(data, CHRROM);
				break;
			case 0x03:
				chr2_6(data, CHRROM);
				break;
		}
	}
}
