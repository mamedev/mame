// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for RacerMate PCBs


 Here we emulate the UNL-RACERMATE PCB [mapper 168]

 TODO:
 - save VRAM
 - emulate the bike controller?

 ***********************************************************************************************************/


#include "emu.h"
#include "racermate.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_RACERMATE = &device_creator<nes_racermate_device>;


nes_racermate_device::nes_racermate_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_RACERMATE, "NES Cart Racermate PCB", tag, owner, clock, "nes_racermate", __FILE__)
{
}



void nes_racermate_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_racermate_device::pcb_reset()
{
//  m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr4_0(0, CHRRAM);
	chr4_4(0, CHRRAM);

	m_latch = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Board UNL-RACERMATE

 In MESS: *VERY* preliminary support. Also, it seems that this
 board saves to battery the CHRRAM!!!

 -------------------------------------------------*/

void nes_racermate_device::update_banks()
{
	chr4_4(m_latch & 0x0f, m_chr_source);
	prg16_89ab(m_latch >> 6);
}

WRITE8_MEMBER(nes_racermate_device::write_h)
{
	LOG_MMC(("racermate write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x3000)
	{
		m_latch = data;
		update_banks();
	}
}
