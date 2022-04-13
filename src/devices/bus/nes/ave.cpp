// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for AVE PCBs


 Here we emulate the following PCBs

   * AVE Nina-001 [mapper 34]
   * AVE Nina-006/Nina-003/MB-91 [mapper 79]
   * AVE Maxi 15 [mapper 234]


 ***********************************************************************************************************/


#include "emu.h"
#include "ave.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_NINA001, nes_nina001_device, "nes_nina001", "NES Cart AVE Nina-001 PCB")
DEFINE_DEVICE_TYPE(NES_NINA006, nes_nina006_device, "nes_nina006", "NES Cart AVE Nina-006 PCB")
DEFINE_DEVICE_TYPE(NES_MAXI15,  nes_maxi15_device,  "nes_maxi15",  "NES Cart AVE Maxi 15 PCB")


nes_nina001_device::nes_nina001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_NINA001, tag, owner, clock)
{
}

nes_nina006_device::nes_nina006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_NINA006, tag, owner, clock)
{
}

nes_maxi15_device::nes_maxi15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_MAXI15, tag, owner, clock)
{
}




void nes_maxi15_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_maxi15_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRROM);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_reg[0] = m_reg[1] = 0;
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 AVE NINA-001 board emulation

 iNES: mapper 34

 Notice that in this board the bankswitch regs
 overlaps WRAM, so that writes to the regs are
 then readable back in WRAM (WRAM is tested by
 Impossible Mission II at start)

 In MAME: Supported.

 -------------------------------------------------*/

void nes_nina001_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("nina-001 write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1ffd:
			prg32(data);
			break;
		case 0x1ffe:
			chr4_0(data, CHRROM);
			break;
		case 0x1fff:
			chr4_4(data, CHRROM);
			break;
	}

	m_prgram[offset] = data;
}

/*-------------------------------------------------

 AVE NINA-003, NINA-006 and MB-91 boards emulation

 Games: Krazy Kreatures, Poke Block, Puzzle, Pyramid,
 Solitaire, Ultimate League Soccer

 iNES: mapper 79

 In MAME: Supported.

 -------------------------------------------------*/

void nes_nina006_device::write_l(offs_t offset, uint8_t data)
{
	LOG_MMC(("nina-006 write_l, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;
	if (BIT(offset, 8)) // $41xx, $43xx, ... $5fxx
	{
		prg32(data >> 3);
		chr8(data & 7, CHRROM);
	}
}

/*-------------------------------------------------

 AVE Maxi 15 boards emulation

 Games: Maxi 15

 iNES: mapper 234

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_maxi15_device::read_h(offs_t offset)
{
	LOG_MMC(("Maxi 15 read_h, offset: %04x\n", offset));

	u8 temp = hi_access_rom(offset);

	if ((offset >= 0x7f80 && offset < 0x7fa0) || (offset >= 0x7fe8 && offset < 0x7ff8))
	{
		int reg = BIT(offset, 6);
		if (reg || !(m_reg[0] & 0x3f))    // inner banks always modifiable, outer banks locked once set
		{
			m_reg[reg] = temp;

			u8 mode = !BIT(m_reg[0], 6);
			u8 outer = m_reg[0] & (0x0e | mode);
			prg32(outer | (m_reg[1] & !mode));
			chr8(outer << 2 | ((m_reg[1] >> 4) & (7 >> mode)), CHRROM);
			set_nt_mirroring(BIT(m_reg[0], 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		}
	}

	return temp;
}
