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

const device_type NES_HES = &device_creator<nes_hes_device>;


nes_hes_device::nes_hes_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_HES, "NES Cart HES PCB", tag, owner, clock, "nes_hes", __FILE__)
{
}


void nes_hes_device::device_start()
{
	common_start();
}

void nes_hes_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
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

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_hes_device::write_l)
{
	LOG_MMC(("hes write_l, offset: %04x, data: %02x\n", offset, data));

	if (!(offset & 0x100))
	{
		prg32((data & 0x38) >> 3);
		chr8((data & 0x07) | ((data & 0x40) >> 3), CHRROM);
		if (m_pcb_ctrl_mirror)
			set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	}
}
