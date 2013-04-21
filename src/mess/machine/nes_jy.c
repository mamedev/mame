/***********************************************************************************************************


 NES/Famicom cartridge emulation for JY Company

 Copyright MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.


 Here we emulate multiple PCBs by JY Company with weird IRQ mechanisms [mappers 90, 209, 211]

 TODO: long list...
 - add dipswitches
 - revamp IRQ system
   * scanline/hblank irq should fire 8 times per line (currently not possible)
   * implement CPU write IRQ (used by any games?)
   * possibly implementing 'funky' IRQ mode (unused?)

 ***********************************************************************************************************/


#include "emu.h"
#include "machine/nes_jy.h"

#include "cpu/m6502/m6502.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_JY_TYPEA = &device_creator<nes_jy_typea_device>;
const device_type NES_JY_TYPEB = &device_creator<nes_jy_typeb_device>;
const device_type NES_JY_TYPEC = &device_creator<nes_jy_typec_device>;


nes_jy_typea_device::nes_jy_typea_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_jy_typea_device::nes_jy_typea_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_JY_TYPEA, "NES Cart JY Company Type A PCB", tag, owner, clock, "nes_jya", __FILE__)
{
}

nes_jy_typeb_device::nes_jy_typeb_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_jy_typea_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_jy_typeb_device::nes_jy_typeb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_jy_typea_device(mconfig, NES_JY_TYPEB, "NES Cart JY Company Type B PCB", tag, owner, clock, "nes_jyb", __FILE__)
{
}

nes_jy_typec_device::nes_jy_typec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_jy_typeb_device(mconfig, NES_JY_TYPEC, "NES Cart JY Company Type C PCB", tag, owner, clock, "nes_jyc", __FILE__)
{
}




void nes_jy_typea_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->reset();

	save_item(NAME(m_mul));
	save_item(NAME(m_latch));
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_nt_bank));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_reg));
	save_item(NAME(m_chr_latch));
	save_item(NAME(m_extra_chr_bank));
	save_item(NAME(m_extra_chr_mask));
	save_item(NAME(m_bank_6000));

	save_item(NAME(m_irq_prescale));
	save_item(NAME(m_irq_prescale_mask));
	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_flip));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_up));
	save_item(NAME(m_irq_down));
}

void nes_jy_typea_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	// 0x5000-0x5fff
	m_mul[0] = 0;
	m_mul[1] = 0;
	m_latch = 0;

	// 0x8000-0xffff
	memset(m_mmc_prg_bank, 0xff, sizeof(m_mmc_prg_bank));
	memset(m_mmc_nt_bank, 0, sizeof(m_mmc_nt_bank));
	memset(m_mmc_vrom_bank, 0xffff, sizeof(m_mmc_vrom_bank));
	memset(m_reg, 0, sizeof(m_reg));
	m_chr_latch[0] = 0;
	m_chr_latch[1] = 4;
	m_extra_chr_bank = 0;
	m_extra_chr_mask = 0;
	m_bank_6000 = 0;

	update_prg();
	update_extra_chr();
	update_chr();
	update_mirror();

	m_irq_mode = 0;
	m_irq_count = 0;
	m_irq_prescale = 0;
	m_irq_prescale_mask = 0xff;
	m_irq_flip = 0;
	m_irq_enable = 0;
	m_irq_up = 0;
	m_irq_down = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 JY Company Type A board emulation

 iNES: mapper 90

 -------------------------------------------------*/

void nes_jy_typea_device::update_chr_latches()
{
	update_extra_chr();
	chr4_0((m_mmc_vrom_bank[m_chr_latch[0]] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
	chr4_4((m_mmc_vrom_bank[m_chr_latch[1]] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
}

READ8_MEMBER(nes_jy_typea_device::chr_r)
{
	int bank = offset >> 10;
	UINT8 val = m_chr_access[bank][offset & 0x3ff]; // this would be usual return value

	switch (offset & 0xff8)
	{
		case 0xfd0:
		case 0xfe8:
//          m_chr_latch[BIT(offset, 12)] = ((bank & 0x4) | 0x2) & (offset >> 4);
			break;
	}

	if ((m_reg[0] & 0x18) == 0x08)      // 4KB mode is the only one using these latches!
		update_chr_latches();

	return val;
}

void nes_jy_typea_device::irq_clock(int mode, int blanked)
{
	bool clock = FALSE, fire = FALSE;

	if (m_irq_mode != mode)
		return;

	// no counter changes if both Up/Down are set or clear
	if ((m_irq_down && m_irq_up) || (!m_irq_down && !m_irq_down))
		return;

	// update prescaler
	if (m_irq_down)
	{
		if ((m_irq_prescale & m_irq_prescale_mask) == 0)
		{
			clock = TRUE;
			m_irq_prescale = (m_irq_prescale_mask == 7) ? ((m_irq_prescale & 0xf8) | 7) : 0xff;
		}
		else
			m_irq_prescale = (m_irq_prescale_mask == 7) ? ((m_irq_prescale & 0xf8) | ((m_irq_prescale - 1) & m_irq_prescale_mask)) : (m_irq_prescale - 1);
	}

	if (m_irq_up)
	{
		if ((m_irq_prescale & m_irq_prescale_mask) == m_irq_prescale)
		{
			clock = TRUE;
			m_irq_prescale = (m_irq_prescale_mask == 7) ? (m_irq_prescale & 0xf8) : 0;
		}
		else
			m_irq_prescale = (m_irq_prescale_mask == 7) ? ((m_irq_prescale & 0xf8) | ((m_irq_prescale + 1) & m_irq_prescale_mask)) : (m_irq_prescale + 1);
	}

	// if prescaler wraps, update count
	if (clock)
	{
		if (m_irq_down)
		{
			if (m_irq_count == 0)
			{
				fire = TRUE;
				m_irq_count = 0xff;
			}
			else
				m_irq_count--;
		}

		if (m_irq_up)
		{
			if (m_irq_count == 0xff)
			{
				fire = TRUE;
				m_irq_count = 0;
			}
			else
				m_irq_count++;
		}


		// if count wraps, check if IRQ is enabled
		if (fire && m_irq_enable && !blanked)
			machine().device("maincpu")->execute().set_input_line(M6502_IRQ_LINE, HOLD_LINE);

	}
}

void nes_jy_typea_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		irq_clock(0, 0);
	}
}

void nes_jy_typea_device::hblank_irq(int scanline, int vblank, int blanked)
{
	irq_clock(blanked, 2);
}

void nes_jy_typea_device::scanline_irq(int scanline, int vblank, int blanked)
{
	irq_clock(blanked, 1);
}


// 0x5000-0x5fff : sort of protection?
READ8_MEMBER(nes_jy_typea_device::read_l)
{
	LOG_MMC(("JY Company write_m, offset: %04x\n", offset));
	offset += 0x100;

	if (offset >= 0x1000 && offset < 0x1800)
	{
		// DSW read
	}
	if (offset >= 0x1800)
	{
		if ((offset & 3) & 3 == 0)
			return (m_mul[0] * m_mul[1]) & 0xff;
		if ((offset & 3) & 3 == 1)
			return ((m_mul[0] * m_mul[1]) >> 8) & 0xff;
		if ((offset & 3) & 3 == 3)
			return m_latch;
	}
	return ((offset + 0x4000) & 0xff00) >> 8;   // open bus
}

WRITE8_MEMBER(nes_jy_typea_device::write_l)
{
	LOG_MMC(("JY Company write_m, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset >= 0x1800)
	{
		if ((offset & 3) == 0)
			m_mul[0] = data;
		if ((offset & 3) & 3 == 1)
			m_mul[1] = data;
		if ((offset & 3) & 3 == 3)
			m_latch = data;
	}
}

// 0x6000-0x7fff : WRAM or open bus
READ8_MEMBER(nes_jy_typea_device::read_m)
{
	LOG_MMC(("JY Company write_m, offset: %04x\n", offset));

	if (m_reg[0] & 0x80)
		return m_prg[(m_bank_6000 & m_prg_mask) * 0x2000 + (offset & 0x1fff)];

	return ((offset + 0x6000) & 0xff00) >> 8;   // open bus
}


inline UINT8 nes_jy_typea_device::unscramble(UINT8 bank)
{
	return BITSWAP8(bank & 0x7f,7,0,1,2,3,4,5,6);
}

void nes_jy_typea_device::update_prg()
{
	UINT8 exPrg = (m_reg[3] & 0x06) << 5;
	UINT8 last = (m_reg[0] & 0x04) ? m_mmc_prg_bank[3] : 0x3f;

	switch (m_reg[0] & 0x03)
	{
		case 0: // 32KB
			prg32((last & 0xF) | (exPrg >> 2));
			m_bank_6000 = (((m_mmc_prg_bank[3] * 4) + 3) & 0x3f) | (exPrg >> 2);
			break;

		case 1: // 16KB
			prg16_89ab((m_mmc_prg_bank[1] & 0x1f) | (exPrg >> 1));
			prg16_cdef((last & 0x1f) | (exPrg >> 1));
			m_bank_6000 = (((m_mmc_prg_bank[3] * 2) + 1) & 0x3f) | (exPrg >> 1);
			break;

		case 2: // 8KB
			prg8_89((m_mmc_prg_bank[0] & 0x3f) | exPrg);
			prg8_ab((m_mmc_prg_bank[1] & 0x3f) | exPrg);
			prg8_cd((m_mmc_prg_bank[2] & 0x3f) | exPrg);
			prg8_ef((last & 0x3f) | exPrg);
			m_bank_6000 = (m_mmc_prg_bank[3] & 0x3f) | exPrg;
			break;

		case 3: // 8KB Alt
			prg8_89((unscramble(m_mmc_prg_bank[0]) & 0x3f) | exPrg);
			prg8_ab((unscramble(m_mmc_prg_bank[1]) & 0x3f) | exPrg);
			prg8_cd((unscramble(m_mmc_prg_bank[2]) & 0x3f) | exPrg);
			prg8_ef((unscramble(last) & 0x3f) | exPrg);
			m_bank_6000 = (unscramble(m_mmc_prg_bank[3]) & 0x3f) | exPrg;
			break;
	}
}

void nes_jy_typea_device::update_extra_chr()
{
	if (m_reg[3] & 0x20)
	{
		//Block mode disabled
		m_extra_chr_mask = 0xffff;
		m_extra_chr_bank = 0;
	}
	else
	{
		// Block mode enabled: in this case lower bits select a 256KB page inside CHRROM
		// and the low bytes of m_mmc_vrom_bank select the banks inside such a page
		int mode = (m_reg[0] & 0x18) >> 3;
		m_extra_chr_mask = 0x00ff >> (mode ^ 0x3);
		m_extra_chr_bank = ((m_reg[3] & 1) | ((m_reg[3] & 0x18) >> 2)) << (mode + 5);
	}
}

void nes_jy_typea_device::update_chr()
{
	UINT8 chr_mirror_mode = BIT(m_reg[3], 7) << 1;  // in 1KB & 2KB mode, PPU 0x800-0xfff always mirrors 0x000-0x7ff (0x1800-0x1fff not affected)

	switch (m_reg[0] & 0x18)
	{
		case 0x00:  // 8KB
			chr8((m_mmc_vrom_bank[0] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			break;

		case 0x08:  // 4KB
//          chr4_0((m_mmc_vrom_bank[m_chr_latch[0]] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
//          chr4_4((m_mmc_vrom_bank[m_chr_latch[1]] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			update_chr_latches();
			break;

		case 0x10:  // 2KB
			chr2_0((m_mmc_vrom_bank[0] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			chr2_2((m_mmc_vrom_bank[2 ^ chr_mirror_mode] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			chr2_4((m_mmc_vrom_bank[4] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			chr2_6((m_mmc_vrom_bank[6] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			break;

		case 0x18:  // 1KB
			chr1_0((m_mmc_vrom_bank[0] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			chr1_0((m_mmc_vrom_bank[1] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			chr1_0((m_mmc_vrom_bank[2 ^ chr_mirror_mode] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			chr1_0((m_mmc_vrom_bank[3 ^ chr_mirror_mode] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			chr1_0((m_mmc_vrom_bank[4] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			chr1_0((m_mmc_vrom_bank[5] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			chr1_0((m_mmc_vrom_bank[6] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			chr1_0((m_mmc_vrom_bank[7] & m_extra_chr_mask) | m_extra_chr_bank, m_chr_source);
			break;
	}
}

void nes_jy_typea_device::update_mirror_typea()
{
	switch (m_reg[1] & 3)
	{
		case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
		case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
		case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
	}
}

void nes_jy_typea_device::update_banks(int reg)
{
	switch (reg & 3)
	{
		case 0:
			update_prg();
			update_extra_chr();
			update_chr();
			update_mirror();
			break;
		case 1:
			update_mirror();
			break;
		case 2:
			update_mirror();
			break;
		case 3:
			update_prg();
			update_extra_chr();
			update_chr();
			break;
	}
}


WRITE8_MEMBER(nes_jy_typea_device::write_h)
{
	LOG_MMC(("JY Company write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			offset &= 3;
			data &= 0x3f;
			if (m_mmc_prg_bank[offset] != data)
			{
				m_mmc_prg_bank[offset] = data;
				update_prg();
			}
			break;
		case 0x1000:
			offset &= 7;
			if ((m_mmc_vrom_bank[offset] & 0xff) != data)
			{
				m_mmc_vrom_bank[offset] = (m_mmc_vrom_bank[offset] & 0xff00) | data;
				update_extra_chr();
				update_chr();
			}
			break;
		case 0x2000:
			offset &= 7;
			if ((m_mmc_vrom_bank[offset] & 0xff00) != (data << 8))
			{
				m_mmc_vrom_bank[offset] = (m_mmc_vrom_bank[offset] & 0x00ff) | (data << 8);
				update_extra_chr();
				update_chr();
			}
			break;
		case 0x3000:
			if ((offset & 7) < 4)
			{
				offset &= 3;
				m_mmc_nt_bank[offset] = (m_mmc_nt_bank[offset] & 0xff00) | data;
				update_mirror();
			}
			else
			{
				offset &= 3;
				m_mmc_nt_bank[offset] = (m_mmc_nt_bank[offset] & 0x00ff) | data << 8;
				update_mirror();
			}
			break;
		case 0x4000:
			switch (offset & 7)
			{
				case 0:
					m_irq_enable = BIT(data, 0);
					break;
				case 1:
					m_irq_mode = data & 3;
					m_irq_prescale_mask = (data & 4) ? 0x07 : 0xff;
					m_irq_down = data & 0x80;
					m_irq_up = data & 0x40;
					if (m_irq_mode == 0)
						irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));
					else
						irq_timer->adjust(attotime::never);
					break;
				case 2:
					m_irq_enable = 0;
						break;
				case 3:
					m_irq_enable = 1;
					break;
				case 4:
					m_irq_prescale = data ^ m_irq_flip;
					break;
				case 5:
					m_irq_count = data ^ m_irq_flip;
					break;
				case 6:
					m_irq_flip = data;
					break;
				case 7:
					// this is used for the 'funky' IRQ mode, not implemented yet
					break;
			}
			break;
		case 0x5000:
			if (m_reg[offset & 3] != data)
			{
				m_reg[offset & 3] = data;
				update_banks(offset & 3);
			}
			break;
	}
}

/*-------------------------------------------------

 JY Company Type B board emulation

 iNES: mapper 211

 The mirroring system is a lot more complex in this
 board

 -------------------------------------------------*/

void nes_jy_typeb_device::update_mirror_typeb()
{
	for (int i = 0; i < 4; i++)
	{
		if (m_reg[0] & 0x40)    // CHRROM
			set_nt_page(i, VROM, m_mmc_nt_bank[i], 0);
		else    // might be either CHRROM or CIRAM
		{
			// CHRROM is only used if bit 7 of the NT Reg does not match bit7 of reg[2].
			if ((m_mmc_nt_bank[i] ^ m_reg[2]) & 0x80)
				set_nt_page(i, VROM, m_mmc_nt_bank[i], 0);
			else
				set_nt_page(i, CIRAM, m_mmc_nt_bank[i] & 1, 1);
		}
	}
}

/*-------------------------------------------------

 JY Company Type C board emulation

 iNES: mapper 209

 These board can switch between the Type A and the
 Type B mirroring

 -------------------------------------------------*/

void nes_jy_typec_device::update_mirror_typec()
{
	if (BIT(m_reg[0], 5))
		update_mirror_typeb();
	else
		update_mirror_typea();
}
