// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo MMC-1 PCBs


 Here we emulate the Nintendo SxROM / MMC-1 PCBs + older variants without WRAM protect bit

 Known issues on specific mappers:

 * 001 Yoshi flashes in-game.
 * 001 Back to the Future have heavily corrupted graphics (since forever).

 TODO:
 - Combine 2 versions of set_prg in SxROM base class. This means dealing with
   variant boards SNROM, SUROM, etc which repurpose bits in the MMC1 regs.

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc1.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_SXROM,   nes_sxrom_device,   "nes_sxrom",   "NES Cart SxROM (MMC-1) PCB")
DEFINE_DEVICE_TYPE(NES_SOROM,   nes_sorom_device,   "nes_sorom",   "NES Cart SOROM (MMC-1) PCB")
DEFINE_DEVICE_TYPE(NES_SXROM_A, nes_sxrom_a_device, "nes_sxrom_a", "NES Cart SxROM (MMC-1A) PCB")
DEFINE_DEVICE_TYPE(NES_SOROM_A, nes_sorom_a_device, "nes_sorom_a", "NES Cart SOROM (MMC-1A) PCB")



nes_sxrom_device::nes_sxrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_reg_write_enable(0), m_latch(0), m_count(0)
{
}

nes_sxrom_device::nes_sxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sxrom_device(mconfig, NES_SXROM, tag, owner, clock)
{
}

nes_sorom_device::nes_sorom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sxrom_device(mconfig, NES_SOROM, tag, owner, clock)
{
}

nes_sxrom_a_device::nes_sxrom_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sxrom_device(mconfig, NES_SXROM_A, tag, owner, clock)
{
}

nes_sorom_a_device::nes_sorom_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sxrom_device(mconfig, NES_SOROM_A, tag, owner, clock)
{
}



void nes_sxrom_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_count));
	save_item(NAME(m_reg));
	save_item(NAME(m_reg_write_enable));
}

void nes_sxrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_latch = 0;
	m_count = 0;
	m_reg[0] = 0x0f;
	m_reg[1] = m_reg[2] = m_reg[3] = 0;
	m_reg_write_enable = 1;

	set_nt_mirroring(PPU_MIRROR_HORZ);
	set_chr();
	set_prg();
}

void nes_sorom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_latch = 0;
	m_count = 0;
	m_reg[0] = 0x0f;
	m_reg[1] = m_reg[2] = m_reg[3] = 0;
	m_reg_write_enable = 1;

	set_nt_mirroring(PPU_MIRROR_HORZ);
	set_chr();
	set_prg();
}

void nes_sorom_a_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_latch = 0;
	m_count = 0;
	m_reg[0] = 0x0f;
	m_reg[1] = m_reg[2] = m_reg[3] = 0;
	m_reg_write_enable = 1;

	set_nt_mirroring(PPU_MIRROR_HORZ);
	set_chr();
	set_prg();
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/


/*-------------------------------------------------

 SxROM (MMC1 based) board emulation

 iNES: mapper 1 (and 155 for the MMC1A variant which does not
 have WRAM disable bit)

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER( nes_sxrom_device::resync_callback )
{
	m_reg_write_enable = 1;
}


// Standard MMC1 PRG banking with base and mask (to support multicarts, etc)
void nes_sxrom_device::set_prg(int prg_base, int prg_mask)
{
	u8 bank = prg_base | (m_reg[3] & prg_mask);

	switch ((m_reg[0] >> 2) & 3)
	{
		case 0:
		case 1:
			prg32(bank >> 1);
			break;
		case 2:
			prg16_89ab(prg_base);
			prg16_cdef(bank);
			break;
		case 3:
			prg16_89ab(bank);
			prg16_cdef(prg_base | prg_mask);
			break;
	}
}

void nes_sxrom_device::set_prg()
{
	uint8_t prg_mode, prg_offset;

	prg_mode = m_reg[0] & 0x0c;
	/* prg_mode&0x8 determines bank size: 32k (if 0) or 16k (if 1)? when in 16k mode,
	 prg_mode&0x4 determines which half of the PRG space we can swap: if it is 4,
	 m_reg[3] sets banks at 0x8000; if it is 0, m_reg[3] sets banks at 0xc000. */

	prg_offset = m_reg[1] & 0x10;
	/* In principle, m_reg[2]&0x10 might affect "extended" banks as well, when chr_mode=1.
	 However, quoting Disch's docs: When in 4k CHR mode, 0x10 in both $A000 and $C000 *must* be
	 set to the same value, or else pages will constantly be swapped as graphics render!
	 Hence, we use only m_reg[1]&0x10 for prg_offset */

	switch (prg_mode)
	{
		case 0x00:
		case 0x04:
//          printf("PRG 32 bank %d \n", (prg_offset + m_reg[3]) >> 1);
			prg32((prg_offset + m_reg[3]) >> 1);
			break;
		case 0x08:
//          printf("PRG 16 bank %d (high) \n", prg_offset + m_reg[3]);
			prg16_89ab(prg_offset + 0);
			prg16_cdef(prg_offset + m_reg[3]);
			break;
		case 0x0c:
//          printf("PRG 16 bank %d (low) \n", prg_offset + m_reg[3]);
			prg16_89ab(prg_offset + m_reg[3]);
			prg16_cdef(prg_offset + 0x0f);
			break;
	}
}

// Standard MMC1 CHR banking with base and mask (to support multicarts, etc)
void nes_sxrom_device::set_chr(int chr_base, int chr_mask)
{
	if (BIT(m_reg[0], 4))
	{
		chr4_0(chr_base | (m_reg[1] & chr_mask), m_chr_source);
		chr4_4(chr_base | (m_reg[2] & chr_mask), m_chr_source);
	}
	else
		chr8((chr_base | (m_reg[1] & chr_mask)) >> 1, m_chr_source);
}

// this allows for easier implementation of the NES-EVENT board used for Nintento World Championships
void nes_sxrom_device::update_regs(int reg)
{
	switch (reg)
	{
		case 0:
			switch (m_reg[0] & 0x03)
			{
				case 0: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 1: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				case 2: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 3: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			}
			set_chr();
			set_prg();
			break;
		case 1:
			set_chr();
			set_prg();
			break;
		case 2:
			set_chr();
			break;
		case 3:
			set_prg();
			break;
	}
}

void nes_sxrom_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("sxrom write_h, offset: %04x, data: %02x\n", offset, data));

	// There is only one latch and shift counter, shared amongst the 4 regs (testcase: Space Shuttle)

	/* here we would need to add an if(cpu_cycles_passed>1) test, and
	 if requirement is not met simply return without writing anything.
	 Some games (AD&D Hillsfar, Bill & Ted Excellent Adventure, Cosmic
	 Wars, Rocket Ranger, Sesame Street 123 and Snow Brothers) rely on
	 this behavior!! */
	if (!m_reg_write_enable)
		return;
	else
	{
		m_reg_write_enable = 0;
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(nes_sxrom_device::resync_callback),this));
	}

	if (data & 0x80)
	{
		m_count = 0;
		m_latch = 0;

		// Set reg at 0x8000 to size 16k and lower half swap - needed for Robocop 3, Dynowars
		m_reg[0] |= 0x0c;
		set_prg();
		return;
	}

	if (m_count < 5)
	{
		if (m_count == 0) m_latch = 0;
		m_latch >>= 1;
		m_latch |= (data & 0x01) ? 0x10 : 0x00;
		m_count++;
	}

	if (m_count == 5)
	{
		m_reg[(offset & 0x6000) >> 13] = m_latch;
		update_regs((offset & 0x6000) >> 13);
		m_count = 0;
	}
}

void nes_sxrom_device::write_m(offs_t offset, uint8_t data)
{
	uint8_t bank = (m_reg[1] >> 2) & 3;
	LOG_MMC(("sxrom write_m, offset: %04x, data: %02x\n", offset, data));

	if (!BIT(m_reg[3], 4))  // WRAM enabled
	{
		if (!m_battery.empty())
			m_battery[((bank * 0x2000) + offset) & (m_battery.size() - 1)] = data;
		if (!m_prgram.empty())
			m_prgram[((bank * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
	}
}

uint8_t nes_sxrom_device::read_m(offs_t offset)
{
	uint8_t bank = (m_reg[1] >> 2) & 3;
	LOG_MMC(("sxrom read_m, offset: %04x\n", offset));

	if (!BIT(m_reg[3], 4))  // WRAM enabled
	{
		if (!m_battery.empty())
			return m_battery[((bank * 0x2000) + offset) & (m_battery.size() - 1)];
		if (!m_prgram.empty())
			return m_prgram[((bank * 0x2000) + offset) & (m_prgram.size() - 1)];
	}

	return get_open_bus();   // open bus
}

// SOROM has two RAM banks, the first is not battery backed up, the second is.
void nes_sorom_device::write_m(offs_t offset, uint8_t data)
{
	uint8_t type = BIT(m_reg[0], 4) ? BIT(m_reg[1], 4) : BIT(m_reg[1], 3);
	LOG_MMC(("sorom write_m, offset: %04x, data: %02x\n", offset, data));

	if (!BIT(m_reg[3], 4))  // WRAM enabled
	{
		if (type)
			m_battery[offset & (m_battery.size() - 1)] = data;
		else
			m_prgram[offset & (m_prgram.size() - 1)] = data;
	}
}

uint8_t nes_sorom_device::read_m(offs_t offset)
{
	uint8_t type = BIT(m_reg[0], 4) ? BIT(m_reg[1], 4) : BIT(m_reg[1], 3);
	LOG_MMC(("sorom read_m, offset: %04x\n", offset));

	if (!BIT(m_reg[3], 4))  // WRAM enabled
	{
		if (type)
			return m_battery[offset & (m_battery.size() - 1)];
		else
			return m_prgram[offset & (m_prgram.size() - 1)];
	}

	return get_open_bus();   // open bus
}

// MMC1A boards have no wram enable/disable bit
void nes_sxrom_a_device::write_m(offs_t offset, uint8_t data)
{
	uint8_t bank = (m_reg[1] >> 2) & 3;
	LOG_MMC(("sxrom_a write_m, offset: %04x, data: %02x\n", offset, data));

	if (!m_battery.empty())
		m_battery[((bank * 0x2000) + offset) & (m_battery.size() - 1)] = data;
	if (!m_prgram.empty())
		m_prgram[((bank * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
}

uint8_t nes_sxrom_a_device::read_m(offs_t offset)
{
	uint8_t bank = (m_reg[1] >> 2) & 3;
	LOG_MMC(("sxrom_a read_m, offset: %04x\n", offset));

	if (!m_battery.empty())
		return m_battery[((bank * 0x2000) + offset) & (m_battery.size() - 1)];
	if (!m_prgram.empty())
		return m_prgram[((bank * 0x2000) + offset) & (m_prgram.size() - 1)];

	return get_open_bus();   // open bus
}

void nes_sorom_a_device::write_m(offs_t offset, uint8_t data)
{
	uint8_t type = BIT(m_reg[0], 4) ? BIT(m_reg[1], 4) : BIT(m_reg[1], 3);
	LOG_MMC(("sorom_a write_m, offset: %04x, data: %02x\n", offset, data));

	if (type)
		m_battery[offset & (m_battery.size() - 1)] = data;
	else
		m_prgram[offset & (m_prgram.size() - 1)] = data;
}

uint8_t nes_sorom_a_device::read_m(offs_t offset)
{
	uint8_t type = BIT(m_reg[0], 4) ? BIT(m_reg[1], 4) : BIT(m_reg[1], 3);
	LOG_MMC(("sorom_a read_m, offset: %04x\n", offset));

	if (type)
		return m_battery[offset & (m_battery.size() - 1)];
	else
		return m_prgram[offset & (m_prgram.size() - 1)];
}
