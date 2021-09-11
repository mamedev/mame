// license:BSD-3-Clause
// copyright-holders: kmg, Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for MMC-1 clone PCBs


 Here we emulate several pirate PCBs based on MMC-1 boards

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc1_clones.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_NINJARYU,   nes_ninjaryu_device,   "nes_ninjaryu",   "NES Cart Ninja Ryukenden Chinese PCB")
DEFINE_DEVICE_TYPE(NES_RESETSXROM, nes_resetsxrom_device, "nes_resetsxrom", "NES Cart BMC RESET-SXROM PCB")
DEFINE_DEVICE_TYPE(NES_TXC_22110,  nes_txc_22110_device,  "nes_txc_22110",  "NES Cart TXC 01-22110-000 PCB")


nes_ninjaryu_device::nes_ninjaryu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_NINJARYU, tag, owner, clock)
{
}

nes_resetsxrom_device::nes_resetsxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_RESETSXROM, tag, owner, clock), m_reset_count(-1)
{
}

nes_txc_22110_device::nes_txc_22110_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_TXC_22110, tag, owner, clock), m_latch0(0), m_mode(0)
{
}



void nes_resetsxrom_device::device_start()
{
	nes_sxrom_device::device_start();
	save_item(NAME(m_reset_count));
}

void nes_resetsxrom_device::pcb_reset()
{
	m_reset_count = (m_reset_count + 1) & 3;
	nes_sxrom_device::pcb_reset();
}

void nes_txc_22110_device::device_start()
{
	nes_sxrom_device::device_start();
	save_item(NAME(m_latch0));
	save_item(NAME(m_mode));
}

void nes_txc_22110_device::pcb_reset()
{
	nes_sxrom_device::pcb_reset();
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_latch0 = 0;
	m_mode = 0;
	update_banks();
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 UNL-NINJARYU

 Games: Ninja Ryukenden Chinese

 This board was previously assigned to mapper 111. It has
 registers akin to MMC1 but without the need to write to
 them serially. The one existing game has 256K CHR, so this
 must have at least 1 more bit for CHR banking. Other differences?

 In MAME: Preliminary supported.

 -------------------------------------------------*/

void nes_ninjaryu_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("unl_ninjaryu write_h, offset: %04x, data: %02x\n", offset, data));
	u8 reg = (offset >> 13) & 0x03;
	m_reg[reg] = data;
	update_regs(reg);
}


/*-------------------------------------------------

 MULTIGAME CARTS BASED ON MMC1

 -------------------------------------------------*/

/*-------------------------------------------------

 BMC-RESET-SXROM

 Games: 4 in 1 (JY-021, JY-022, JY-051)

 MMC1 clone with outer banks switched by resetting.

 NES 2.0: mapper 374

 In MAME: Supported.

 -------------------------------------------------*/

/*-------------------------------------------------

 TXC 01-22110-000 Board

 Games: 2 in 1 Uzi Lightgun (MGC-002)

 This board has an MMC1 clone for Operation Wolf and
 otherwise is mostly compatible with mapper 70 for
 Bandai's Space Shadow.

 NES 2.0: mapper 297

 In MAME: Supported.

 -------------------------------------------------*/

void nes_txc_22110_device::update_banks()    // used by menu and Space Shadow
{
	u8 outer = (m_mode & 0x02) << 1;
	prg16_89ab(outer | (m_latch0 & 0x30) >> 4);
	prg16_cdef(outer | 3);
	chr8(m_latch0 & 0x0f, CHRROM);
}

void nes_txc_22110_device::write_l(offs_t offset, u8 data)
{
	LOG_MMC(("TXC 22110 write_l, offset: %04x, data: %02x\n", offset, data));
	if (offset < 0x100)        // $4100 - $41ff
	{
		m_mode = data;
		if (m_mode & 1)    // MMC1 mode
		{
			set_prg();
			set_chr();
		}
		else
			update_banks();
	}
}

void nes_txc_22110_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("TXC 22110 write_h, offset: %04x, data: %02x\n", offset, data));

	if (m_mode & 1)
		nes_sxrom_device::write_h(offset, data);
	else
	{
		m_latch0 = data;
		update_banks();
	}
}
