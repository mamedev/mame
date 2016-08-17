// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for JY Company


 Here we emulate multiple PCBs by JY Company with weird IRQ mechanisms [mappers 90, 209, 211]

 TODO: long list...
 - add dipswitches
 - revamp IRQ system
   * scanline/hblank irq should fire 8 times per line (currently not possible)
   * implement CPU write IRQ (used by any games?)
   * possibly implementing 'funky' IRQ mode (unused?)

 ***********************************************************************************************************/


#include "emu.h"
#include "jy.h"

#include "cpu/m6502/m6502.h"
#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access PPU_BOTTOM_VISIBLE_SCANLINE

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
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_latch(0), m_extra_chr_bank(0), m_extra_chr_mask(0), m_bank_6000(0),
	m_irq_mode(0), m_irq_count(0), m_irq_prescale(0), m_irq_prescale_mask(0), m_irq_flip(0), m_irq_enable(0), m_irq_up(0), m_irq_down(0), irq_timer(nullptr)
				{
}

nes_jy_typea_device::nes_jy_typea_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_JY_TYPEA, "NES Cart JY Company Type A PCB", tag, owner, clock, "nes_jya", __FILE__), m_latch(0), m_extra_chr_bank(0),
	m_extra_chr_mask(0), m_bank_6000(0), m_irq_mode(0), m_irq_count(0), m_irq_prescale(0), m_irq_prescale_mask(0), m_irq_flip(0), m_irq_enable(0), m_irq_up(0),
	m_irq_down(0), irq_timer(nullptr)
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
	timer_freq = machine().device<cpu_device>("maincpu")->cycles_to_attotime(1);

	save_item(NAME(m_mul));
	save_item(NAME(m_latch));
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_nt_bank));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_reg));
	save_item(NAME(m_chr_latch));
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
	m_bank_6000 = 0;

	update_prg();
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


READ8_MEMBER(nes_jy_typea_device::nt_r)
{
	int page = ((offset & 0xc00) >> 10);
	irq_clock(0, 2);
	return m_nt_access[page][offset & 0x3ff];
}

READ8_MEMBER(nes_jy_typea_device::chr_r)
{
	int bank = offset >> 10;
	irq_clock(0, 2);
	return m_chr_access[bank][offset & 0x3ff];
}

void nes_jy_typea_device::irq_clock(int mode, int blanked)
{
	bool clock = FALSE, fire = FALSE;

	if (m_irq_mode != mode)
		return;

	// no counter changes if both Up/Down are set or clear
	if ((m_irq_down && m_irq_up) || (!m_irq_down && !m_irq_up))
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
		if ((m_irq_prescale & m_irq_prescale_mask) == m_irq_prescale_mask)
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
			m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);

	}
}

void nes_jy_typea_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		irq_clock(0, 0);
	}
}

void nes_jy_typea_device::scanline_irq(int scanline, int vblank, int blanked)
{
	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
		irq_clock(blanked, 1);
}


// 0x5000-0x5fff : sort of protection?
READ8_MEMBER(nes_jy_typea_device::read_l)
{
	LOG_MMC(("JY Company write_m, offset: %04x\n", offset));
	offset += 0x100;

	if (offset >= 0x1000 && offset < 0x1800)
	{
		// bit6/bit7 DSW read
		return m_open_bus & 0x3f;
	}

	if (offset >= 0x1800)
	{
		if ((offset & 7) == 0)
			return (m_mul[0] * m_mul[1]) & 0xff;
		if ((offset & 7) == 1)
			return ((m_mul[0] * m_mul[1]) >> 8) & 0xff;
		if ((offset & 7) == 3)
			return m_latch;
	}

	return m_open_bus;   // open bus
}

WRITE8_MEMBER(nes_jy_typea_device::write_l)
{
	LOG_MMC(("JY Company write_m, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset >= 0x1800)
	{
		if ((offset & 7) == 0)
			m_mul[0] = data;
		if ((offset & 7) == 1)
			m_mul[1] = data;
		if ((offset & 7) == 3)
			m_latch = data;
	}
}

// 0x6000-0x7fff : WRAM or open bus
READ8_MEMBER(nes_jy_typea_device::read_m)
{
	LOG_MMC(("JY Company write_m, offset: %04x\n", offset));

	if (m_reg[0] & 0x80)
		return m_prg[(m_bank_6000 & m_prg_mask) * 0x2000 + (offset & 0x1fff)];

	return m_open_bus;   // open bus
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
			prg32((last & 0x0f) | (exPrg >> 2));
			m_bank_6000 = (((m_mmc_prg_bank[3] * 4) + 3) & 0x3f) | (exPrg >> 2);
			break;

		case 1: // 16KB
			prg16_89ab((m_mmc_prg_bank[1] & 0x1f) | (exPrg >> 1));
			prg16_cdef((last & 0x1f) | (exPrg >> 1));
			m_bank_6000 = (((m_mmc_prg_bank[3] * 2) + 1) & 0x1f) | (exPrg >> 1);
			break;

		case 2: // 8KB
			prg8_89(m_mmc_prg_bank[0] | exPrg);
			prg8_ab(m_mmc_prg_bank[1] | exPrg);
			prg8_cd(m_mmc_prg_bank[2] | exPrg);
			prg8_ef(last | exPrg);
			m_bank_6000 = m_mmc_prg_bank[3] | exPrg;
			break;

		case 3: // 8KB Alt
			prg8_89((unscramble(m_mmc_prg_bank[0]) & 0x3f) | exPrg);
			prg8_ab((unscramble(m_mmc_prg_bank[1]) & 0x3f) | exPrg);
			prg8_cd((unscramble(m_mmc_prg_bank[2]) & 0x3f) | exPrg);
			if (m_reg[0] & 0x04)
				prg8_ef((unscramble(m_mmc_prg_bank[3]) & 0x3f) | exPrg);
			else
				prg8_ef((unscramble(last) & 0x3f) | exPrg);
			m_bank_6000 = (unscramble(m_mmc_prg_bank[3]) & 0x3f) | exPrg;
			break;
	}
}

void nes_jy_typea_device::update_chr()
{
	// in 1KB & 2KB mode, PPU 0x800-0xfff always mirrors 0x000-0x7ff (0x1800-0x1fff not affected)
	int chr_mirror_mode = BIT(m_reg[3], 7) << 1;

	// Case (m_reg[3] & 0x20 == 0)
	// Block mode enabled: in this case lower bits select a 256KB page inside CHRROM
	// and the low bytes of m_mmc_vrom_bank select the banks inside such a page

	// docs suggest m_reg[3] & 0x1f for chr_page below,
	// but 45 in 1 (JY-120A) menu requires to use this (from NEStopia)
	UINT8 chr_page = (m_reg[3] & 1) | ((m_reg[3] & 0x18) >> 2);
	UINT32 extra_chr_base = BIT(m_reg[3], 5) ? 0 : (chr_page * 0x100);
	UINT32 extra_chr_mask = BIT(m_reg[3], 5) ? 0xffffff : 0xff;

	switch (m_reg[0] & 0x18)
	{
		case 0x00:  // 8KB
			extra_chr_base >>= 3;
			extra_chr_mask >>= 3;
			chr8(extra_chr_base | (m_mmc_vrom_bank[0] & extra_chr_mask), m_chr_source);
			break;

		case 0x08:  // 4KB
			extra_chr_base >>= 2;
			extra_chr_mask >>= 2;
			// Type A & B games have fixed m_chr_latch[0] = 0 and m_chr_latch[1] = 4
			// Type C games can change them at each CHR access!
			chr4_0(extra_chr_base | (m_mmc_vrom_bank[m_chr_latch[0]] & extra_chr_mask), m_chr_source);
			chr4_4(extra_chr_base | (m_mmc_vrom_bank[m_chr_latch[1]] & extra_chr_mask), m_chr_source);
			break;

		case 0x10:  // 2KB
			extra_chr_base >>= 1;
			extra_chr_mask >>= 1;
			chr2_0(extra_chr_base | (m_mmc_vrom_bank[0] & extra_chr_mask), m_chr_source);
			chr2_2(extra_chr_base | (m_mmc_vrom_bank[2 ^ chr_mirror_mode] & extra_chr_mask), m_chr_source);
			chr2_4(extra_chr_base | (m_mmc_vrom_bank[4] & extra_chr_mask), m_chr_source);
			chr2_6(extra_chr_base | (m_mmc_vrom_bank[6] & extra_chr_mask), m_chr_source);
			break;

		case 0x18:  // 1KB
			chr1_0(extra_chr_base | (m_mmc_vrom_bank[0] & extra_chr_mask), m_chr_source);
			chr1_1(extra_chr_base | (m_mmc_vrom_bank[1] & extra_chr_mask), m_chr_source);
			chr1_2(extra_chr_base | (m_mmc_vrom_bank[2 ^ chr_mirror_mode] & extra_chr_mask), m_chr_source);
			chr1_3(extra_chr_base | (m_mmc_vrom_bank[3 ^ chr_mirror_mode] & extra_chr_mask), m_chr_source);
			chr1_4(extra_chr_base | (m_mmc_vrom_bank[4] & extra_chr_mask), m_chr_source);
			chr1_5(extra_chr_base | (m_mmc_vrom_bank[5] & extra_chr_mask), m_chr_source);
			chr1_6(extra_chr_base | (m_mmc_vrom_bank[6] & extra_chr_mask), m_chr_source);
			chr1_7(extra_chr_base | (m_mmc_vrom_bank[7] & extra_chr_mask), m_chr_source);
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
				update_chr();
			}
			break;
		case 0x2000:
			offset &= 7;
			if ((m_mmc_vrom_bank[offset] & 0xff00) != (data << 8))
			{
				m_mmc_vrom_bank[offset] = (m_mmc_vrom_bank[offset] & 0x00ff) | (data << 8);
				update_chr();
			}
			break;
		case 0x3000:
			if (!(offset & 4))
			{
				offset &= 3;
				m_mmc_nt_bank[offset] = (m_mmc_nt_bank[offset] & 0xff00) | data;
			}
			else
			{
				offset &= 3;
				m_mmc_nt_bank[offset] = (m_mmc_nt_bank[offset] & 0x00ff) | data << 8;
			}
			update_mirror();
			break;
		case 0x4000:
			switch (offset & 7)
			{
				case 0:
					if (BIT(data, 0))
						m_irq_enable = 1;
					else
					{
						m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
						m_irq_enable = 0;
					}
					break;
				case 1:
					m_irq_mode = data & 3;
					m_irq_prescale_mask = (data & 4) ? 0x07 : 0xff;
					m_irq_down = data & 0x80;
					m_irq_up = data & 0x40;
					if (m_irq_mode == 0)
						irq_timer->adjust(attotime::zero, 0, timer_freq);
					else
						irq_timer->adjust(attotime::never);
					break;
				case 2:
					m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
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
		if (BIT(m_reg[0], 6))    // CHRROM
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

READ8_MEMBER(nes_jy_typec_device::chr_r)
{
	int bank = offset >> 10;

	irq_clock(0, 2);
	switch (offset & 0xff0)
	{
		case 0xfd0:
			m_chr_latch[BIT(offset, 12)] = (bank & 0x4);
			update_chr();
			break;
		case 0xfe0:
			m_chr_latch[BIT(offset, 12)] = (bank & 0x4) | 0x2;
			update_chr();
			break;
	}
	return m_chr_access[bank][offset & 0x3ff];
}
