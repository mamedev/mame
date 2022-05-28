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
 - Determine if "MMC1" marked chips, the earliest version, ignores WRAM
   enable/disable bit like its first revision, MMC1A. Also determine if MMC1C
   really exists. It's described by kevtris, but it's not in BootGod's DB.

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

DEFINE_DEVICE_TYPE(NES_SXROM, nes_sxrom_device, "nes_sxrom", "NES Cart SxROM (MMC-1) PCB")
DEFINE_DEVICE_TYPE(NES_SOROM, nes_sorom_device, "nes_sorom", "NES Cart SOROM (MMC-1) PCB")
DEFINE_DEVICE_TYPE(NES_SZROM, nes_szrom_device, "nes_szrom", "NES Cart SZROM (MMC-1) PCB")



nes_sxrom_device::nes_sxrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_reg_write_enable(0), m_latch(0), m_count(0)
{
}

nes_sxrom_device::nes_sxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_SXROM, tag, owner, clock)
{
}

nes_sorom_device::nes_sorom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_SOROM, tag, owner, clock)
{
}

nes_szrom_device::nes_szrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_SZROM, tag, owner, clock)
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
	m_latch = 0;
	m_count = 0;
	m_reg[0] = 0x0f;
	m_reg[1] = 0;
	m_reg[2] = 0;
	m_reg[3] = 0;
	m_reg_write_enable = 1;

	set_chr();
	set_prg();
	set_mirror();
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

	switch (BIT(m_reg[0], 2, 2))
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
	u8 prg_mode, prg_offset;

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

void nes_sxrom_device::set_mirror()
{
	static constexpr u8 mirr[4] = { PPU_MIRROR_LOW, PPU_MIRROR_HIGH, PPU_MIRROR_VERT, PPU_MIRROR_HORZ };

	set_nt_mirroring(mirr[m_reg[0] & 0x03]);
}

// this allows for easier implementation of MMC1 subclasses
void nes_sxrom_device::update_regs(int reg)
{
	switch (reg)
	{
		case 0:
			set_chr();
			set_prg();
			set_mirror();
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

void nes_sxrom_device::write_h(offs_t offset, u8 data)
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

		// Set reg at 0x8000 to size 16k and lower half swap - needed for Robocop 3, Dynowars
		m_reg[0] |= 0x0c;
		set_prg();
		return;
	}

	m_latch >>= 1;
	m_latch |= (data & 1) << 4;
	m_count = (m_count + 1) % 5;

	if (!m_count)
	{
		int reg = BIT(offset, 13, 2);
		m_reg[reg] = m_latch;
		update_regs(reg);
	}
}

void nes_sxrom_device::write_m(offs_t offset, u8 data)
{
	u8 bank = BIT(m_reg[1], 2, 2);
	LOG_MMC(("sxrom write_m, offset: %04x, data: %02x\n", offset, data));

	if (!BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A)  // WRAM enabled
	{
		if (!m_battery.empty())
			m_battery[((bank * 0x2000) + offset) & (m_battery.size() - 1)] = data;
		if (!m_prgram.empty())
			m_prgram[((bank * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
	}
}

u8 nes_sxrom_device::read_m(offs_t offset)
{
	u8 bank = BIT(m_reg[1], 2, 2);
	LOG_MMC(("sxrom read_m, offset: %04x\n", offset));

	if (!BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A)  // WRAM enabled
	{
		if (!m_battery.empty())
			return m_battery[((bank * 0x2000) + offset) & (m_battery.size() - 1)];
		if (!m_prgram.empty())
			return m_prgram[((bank * 0x2000) + offset) & (m_prgram.size() - 1)];
	}

	return get_open_bus();
}

// SOROM has two RAM banks, the first is not battery backed up, the second is.
void nes_sorom_device::write_m(offs_t offset, u8 data)
{
	u8 type = BIT(m_reg[0], 4) ? BIT(m_reg[1], 4) : BIT(m_reg[1], 3);
	LOG_MMC(("sorom write_m, offset: %04x, data: %02x\n", offset, data));

	if (!BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A)  // WRAM enabled
	{
		if (type)
			m_battery[offset & (m_battery.size() - 1)] = data;
		else
			m_prgram[offset & (m_prgram.size() - 1)] = data;
	}
}

u8 nes_sorom_device::read_m(offs_t offset)
{
	u8 type = BIT(m_reg[0], 4) ? BIT(m_reg[1], 4) : BIT(m_reg[1], 3);
	LOG_MMC(("sorom read_m, offset: %04x\n", offset));

	if (!BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A)  // WRAM enabled
	{
		if (type)
			return m_battery[offset & (m_battery.size() - 1)];
		else
			return m_prgram[offset & (m_prgram.size() - 1)];
	}

	return get_open_bus();
}

// SZROM has two RAM banks, the first is not battery backed up, the second is.
void nes_szrom_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("szrom write_m, offset: %04x, data: %02x\n", offset, data));

	if (!BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A)  // WRAM enabled
	{
		if (BIT(m_reg[BIT(m_reg[0], 4) + 1], 4))
			m_battery[offset & (m_battery.size() - 1)] = data;
		else
			m_prgram[offset & (m_prgram.size() - 1)] = data;
	}
}

u8 nes_szrom_device::read_m(offs_t offset)
{
	LOG_MMC(("szrom read_m, offset: %04x\n", offset));

	if (!BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A)  // WRAM enabled
	{
		if (BIT(m_reg[BIT(m_reg[0], 4) + 1], 4))
			return m_battery[offset & (m_battery.size() - 1)];
		else
			return m_prgram[offset & (m_prgram.size() - 1)];
	}

	return get_open_bus();
}
