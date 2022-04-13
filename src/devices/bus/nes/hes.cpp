// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for HES PCBs


 Here we emulate the HES PCBs (both the one with hardwired mirroring and the one with mapper-controlled
 mirroring used by HES 6 in 1) [mapper 113]


 ***********************************************************************************************************/


#include "emu.h"
#include "hes.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_HES, nes_hes_device, "nes_hes", "NES Cart HES PCB")


nes_hes_device::nes_hes_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_HES, tag, owner, clock)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board by HES (also used by others)

 Games: AV Hanafuda Club, AV Soccer, Papillon, Sidewinder,
 Total Funpack

 Actually, two variant: one for HES 6-in-1 with mirroring control
 and one for AV Soccer and others with hardwired mirroring

 iNES: mapper 113

 In MAME: Supported.

 -------------------------------------------------*/

void nes_hes_device::write_l(offs_t offset, u8 data)
{
	LOG_MMC(("hes write_l, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;
	if (BIT(offset, 8)) // $41xx, $43xx, ... $5fxx
	{
		prg32(BIT(data, 3, 3));
		chr8(bitswap<4>(data, 6, 2, 1, 0), CHRROM);
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	}
}
