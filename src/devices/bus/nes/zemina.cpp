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
//  constructor
//-------------------------------------------------

nes_zemina_device::nes_zemina_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_ZEMINA, tag, owner, clock)
{
}




void nes_zemina_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);    // fixed bank

	for (int i = 0; i < 4; i++)
		chr2_x(i << 1, 0, CHRROM);
}



/*-------------------------------------------------
    mapper specific handlers
-------------------------------------------------*/

/*-------------------------------------------------

 Zemina board emulation

 Games: Magic Kid GooGoo

 iNES: mapper 190

 In MAME: Supported.

-------------------------------------------------*/

void nes_zemina_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC("zemina write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6000)
	{
		case 0x0000:
		case 0x4000:
			prg16_89ab(BIT(offset, 14) << 3 | (data & 0x07));
			break;
		case 0x2000:
			chr2_x((offset & 0x03) << 1, data, CHRROM);
			break;
	}
}
