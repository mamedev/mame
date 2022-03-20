// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Kaiser PCBs


 Here we emulate the following Kaiser bootleg PCBs

 * Kaiser KS106C
 * Kaiser KS202
 * Kaiser KS7010
 * Kaiser KS7012
 * Kaiser KS7013B
 * Kaiser KS7016
 * Kaiser KS7016B
 * Kaiser KS7017
 * Kaiser KS7021A
 * Kaiser KS7022
 * Kaiser KS7030
 * Kaiser KS7031
 * Kaiser KS7032
 * Kaiser KS7037
 * Kaiser KS7057
 * Kaiser KS7058

 ***********************************************************************************************************/


#include "emu.h"
#include "kaiser.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_KS106C,  nes_ks106c_device,  "nes_ks106c",  "NES Cart Kaiser KS-106C PCB")
DEFINE_DEVICE_TYPE(NES_KS202,   nes_ks202_device,   "nes_ks202",   "NES Cart Kaiser KS-202 PCB")
DEFINE_DEVICE_TYPE(NES_KS7010,  nes_ks7010_device,  "nes_ks7010",  "NES Cart Kaiser KS-7010 PCB")
DEFINE_DEVICE_TYPE(NES_KS7012,  nes_ks7012_device,  "nes_ks7012",  "NES Cart Kaiser KS-7012 PCB")
DEFINE_DEVICE_TYPE(NES_KS7013B, nes_ks7013b_device, "nes_ks7013b", "NES Cart Kaiser KS-7013B PCB")
DEFINE_DEVICE_TYPE(NES_KS7016,  nes_ks7016_device,  "nes_ks7016",  "NES Cart Kaiser KS-7016 PCB")
DEFINE_DEVICE_TYPE(NES_KS7016B, nes_ks7016b_device, "nes_ks7016b", "NES Cart Kaiser KS-7016B PCB")
DEFINE_DEVICE_TYPE(NES_KS7017,  nes_ks7017_device,  "nes_ks7017",  "NES Cart Kaiser KS-7017 PCB")
DEFINE_DEVICE_TYPE(NES_KS7021A, nes_ks7021a_device, "nes_ks7021a", "NES Cart Kaiser KS-7021A PCB")
DEFINE_DEVICE_TYPE(NES_KS7022,  nes_ks7022_device,  "nes_ks7022",  "NES Cart Kaiser KS-7022 PCB")
DEFINE_DEVICE_TYPE(NES_KS7030,  nes_ks7030_device,  "nes_ks7030",  "NES Cart Kaiser KS-7030 PCB")
DEFINE_DEVICE_TYPE(NES_KS7031,  nes_ks7031_device,  "nes_ks7031",  "NES Cart Kaiser KS-7031 PCB")
DEFINE_DEVICE_TYPE(NES_KS7032,  nes_ks7032_device,  "nes_ks7032",  "NES Cart Kaiser KS-7032 PCB")
DEFINE_DEVICE_TYPE(NES_KS7037,  nes_ks7037_device,  "nes_ks7037",  "NES Cart Kaiser KS-7037 PCB")
DEFINE_DEVICE_TYPE(NES_KS7057,  nes_ks7057_device,  "nes_ks7057",  "NES Cart Kaiser KS-7057 PCB")
DEFINE_DEVICE_TYPE(NES_KS7058,  nes_ks7058_device,  "nes_ks7058",  "NES Cart Kaiser KS-7058 PCB")


nes_ks106c_device::nes_ks106c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_KS106C, tag, owner, clock), m_latch(0)
{
}

nes_ks7058_device::nes_ks7058_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7058, tag, owner, clock)
{
}

nes_ks7022_device::nes_ks7022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7022, tag, owner, clock), m_latch(0)
{
}

nes_ks7032_device::nes_ks7032_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_latch(0), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), irq_timer(nullptr)
{
}

nes_ks7032_device::nes_ks7032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_ks7032_device(mconfig, NES_KS7032, tag, owner, clock)
{
}

nes_ks202_device::nes_ks202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_ks7032_device(mconfig, NES_KS202, tag, owner, clock)
{
}

nes_ks7016_device::nes_ks7016_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 a15_flip)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_latch(0), m_a15_flip(a15_flip)
{
}

nes_ks7016_device::nes_ks7016_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_ks7016_device(mconfig, NES_KS7016, tag, owner, clock, 0x00)
{
}

nes_ks7016b_device::nes_ks7016b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_ks7016_device(mconfig, NES_KS7016B, tag, owner, clock, 0x04)
{
}

nes_ks7017_device::nes_ks7017_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7017, tag, owner, clock), m_latch(0), m_irq_count(0), m_irq_status(0), m_irq_enable(0), irq_timer(nullptr)
{
}

nes_ks7021a_device::nes_ks7021a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_KS7021A, tag, owner, clock)
{
}

nes_ks7010_device::nes_ks7010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_KS7010, tag, owner, clock), m_latch(0)
{
}

nes_ks7012_device::nes_ks7012_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7012, tag, owner, clock)
{
}

nes_ks7013b_device::nes_ks7013b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7013B, tag, owner, clock)
{
}

nes_ks7030_device::nes_ks7030_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_KS7030, tag, owner, clock)
{
}

nes_ks7031_device::nes_ks7031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7031, tag, owner, clock)
{
}

nes_ks7037_device::nes_ks7037_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7037, tag, owner, clock)
{
}

nes_ks7057_device::nes_ks7057_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_KS7057, tag, owner, clock)
{
}




void nes_ks106c_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_ks106c_device::pcb_reset()
{
	prg32(m_latch);
	chr8(m_latch, CHRROM);
	set_nt_mirroring(BIT(m_latch, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	m_latch = (m_latch + 1) & 0x03;
}

void nes_ks7058_device::device_start()
{
	common_start();
}

void nes_ks7058_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_ks7022_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_ks7022_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_ks7032_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_latch));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_reg));
}

void nes_ks7032_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	chr8(0, m_chr_source);

	m_latch = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	prg_update();
	prg8_ef((m_prg_chunks << 1) - 1);
}

void nes_ks7016_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_ks7016_device::pcb_reset()
{
	prg8_89(0x0c ^ m_a15_flip);
	prg8_ab(0x0d ^ m_a15_flip);
	prg8_cd(0x0e ^ m_a15_flip);
	prg8_ef(0x0f ^ m_a15_flip);
	chr8(0, CHRRAM);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_latch = 0;
}

void nes_ks7017_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_latch));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_status));
}

void nes_ks7017_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(2);
	chr8(0, m_chr_source);

	m_latch = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
	m_irq_status = 0;
}

void nes_ks7021a_device::device_start()
{
	common_start();
}

void nes_ks7021a_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRROM);
}

void nes_ks7010_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_ks7010_device::pcb_reset()
{
	prg16_89ab(0x05);    // all upper banks are fixed
	prg16_cdef(0x03);
	chr8(0, CHRROM);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_latch = 0;
}

void nes_ks7012_device::device_start()
{
	common_start();
}

void nes_ks7012_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0xff);
	chr8(0, m_chr_source);
}

void nes_ks7013b_device::device_start()
{
	common_start();
}

void nes_ks7013b_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_ks7030_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_ks7030_device::pcb_reset()
{
	prg32((m_prg_chunks >> 1) - 1);    // not really used...
	chr8(0, CHRRAM);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_reg[0] = m_reg[1] = 0;
}

void nes_ks7031_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_ks7031_device::pcb_reset()
{
	prg32(0);   // not really used...
	chr8(0, CHRRAM);

	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
	m_reg[3] = 0;
}

void nes_ks7037_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_ks7037_device::pcb_reset()
{
	prg8_89(0);
	prg8_ab(0x0e);
	prg8_cd(0);
	prg8_ef(0x0f);
	chr8(0, CHRRAM);

	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	m_latch = 0;
}

void nes_ks7057_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_ks7057_device::pcb_reset()
{
	prg8_ab(0x0d);
	prg16_cdef(0x07);
	chr8(0, CHRRAM);

	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Kaiser Board KS106C

 Games: 4 in 1

 No need to use handlers. At reset the banks change
 and so does the game.

 NES 2.0: mapper 352

 In MAME: Supported.

 -------------------------------------------------*/

/*-------------------------------------------------

 Kaiser Board KS7058

 Games: Tui Do Woo Ma Jeung

 Writes to 0xf000-0xffff set 4k chr banks. Namely, if
 offset&0x80 is 0 the lower 4k are set, if it is 1 the
 upper 4k are set.

 iNES: mapper 171

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ks7058_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7058 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7080)
	{
		case 0x7000:
			chr4_0(data, CHRROM);
			break;
		case 0x7080:
			chr4_4(data, CHRROM);
			break;
	}
}

/*-------------------------------------------------

 Kaiser Board KS7022

 Games: 15 in 1

 iNES: mapper 175

 In MAME: Supported?

 -------------------------------------------------*/

void nes_ks7022_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7022 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0)
		set_nt_mirroring(BIT(data, 2) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (offset == 0x2000)
		m_latch = data & 0x0f;
}

uint8_t nes_ks7022_device::read_h(offs_t offset)
{
	LOG_MMC(("ks7022 read_h, offset: %04x\n", offset));

	if (offset == 0x7ffc)
	{
		chr8(m_latch, CHRROM);
		prg16_89ab(m_latch);
		prg16_cdef(m_latch);
	}

	return hi_access_rom(offset);
}

/*-------------------------------------------------

 Kaiser Board KS7032

 Games: FDS conversions of Bubble Bobble, SMB2,
 and Exciting Soccer

 These boards have a KS202 ASIC that provide both
 banking and a cycle-based IRQ similar to the VRC3.

 iNES: mapper 142

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ks7032_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (++m_irq_count == 0)
			{
				set_irq_line(ASSERT_LINE);
				m_irq_enable = 0;
				m_irq_count = m_irq_count_latch;
			}
		}
	}
}

void nes_ks7032_device::prg_update()
{
	prg8_89(m_reg[1]);
	prg8_ab(m_reg[2]);
	prg8_cd(m_reg[3]);
}

void nes_ks7032_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ks7032_write, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
		{
			int shift = 4 * BIT(offset, 12, 2);
			m_irq_count_latch &= ~(0x000f << shift);
			m_irq_count_latch |= (data & 0x0f) << shift;
			break;
		}
		case 0x4000:
			m_irq_enable = BIT(data, 1);
			if (m_irq_enable)
				m_irq_count = m_irq_count_latch;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x5000:
			set_irq_line(CLEAR_LINE);
			break;
		case 0x6000:
			m_latch = data & 0x07;
			break;
		case 0x7000:
			m_reg[m_latch] = (m_reg[m_latch] & 0xf0) | (data & 0x0f);
			prg_update();
			break;
	}
}

u8 nes_ks7032_device::read_m(offs_t offset)
{
	LOG_MMC(("ks7032 read_m, offset: %04x\n", offset));
	return m_prg[(m_reg[4] * 0x2000 + offset) & (m_prg_size - 1)];
}

/*-------------------------------------------------

 Kaiser SMB3 Board with KS202

 Games: Super Mario Bros. 3 (Pirate, Alt)

 A chip, PAL16L8ANC, provides the extra bits overlaid
 at 0xf000-0xffff. Writes go to both it and the KS202.

 iNES: mapper 56

 In MAME: Supported.

 TODO: This device needs renaming of some sort. KS202
 is a chip found on some Kaiser PCBs that actually
 provides the functionality implemented above in the
 KS7032 PCB. It's not clear what SMB3's PCB is.

 -------------------------------------------------*/

void nes_ks202_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ks202 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x7000)
	{
		switch (offset & 0xc00)
		{
			case 0x000:
				if ((offset & 3) == 3)
					prg8_ef((data & 0x10) | 0x0f);
				else
				{
					int reg = (offset & 3) + 1;
					m_reg[reg] = (m_reg[reg] & 0x0f) | (data & 0x10);
				}
				break;
			case 0x800:
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
				break;
			case 0xc00:
				chr1_x(offset & 0x07, data & 0x7f, CHRROM);
				break;
		}
	}

	nes_ks7032_device::write_h(offset, data);
}

u8 nes_ks202_device::read_m(offs_t offset)
{
	return device_nes_cart_interface::read_m(offset);
}

/*-------------------------------------------------

 Kaiser Boards KS7016, KS7016B

 Games: Exciting Basket, Meikyuu Jiin Dababa FDS Conversions

 These two variants have fixed upper 32K PRG and switchable
 8K PRG at 0x6000-0x7fff. The only difference appears to be
 a flipped bit in the bank numbers. KS7016 puts banks 0x0c
 through 0x0f in the upper PRG; KS7016B uses 0x08 to 0x0b.
 For the switchable 8K PRG the latched bank # patterns are:

   KS7016:  0 1 2 3 4 5 6 7 8 9 A B 8 9 A B
   KS7016B: 0 1 2 3 4 5 6 7 C D E F C D E F

 (NB: only KS7016B has been verified against PCB, KS7016 is a surmise.)

 NES 2.0: mapper 306, mapper 549

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_ks7016_device::read_m(offs_t offset)
{
//  LOG_MMC(("ks7016 read_m, offset: %04x\n", offset));
	return m_prg[(m_latch * 0x2000 + offset) & (m_prg_size - 1)];
}

void nes_ks7016_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ks7016 write_h, offset: %04x, data: %02x\n", offset, data));

	m_latch = BIT(offset, 2, 4);
	if (m_latch & 0x08)
		m_latch = (m_latch & 0x0b) | m_a15_flip;
}

/*-------------------------------------------------

 Kaiser Board KS7017

 Games: Almana no Kiseki FDS conversion

 NES 2.0: mapper 303

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ks7017_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (--m_irq_count == 0)
			{
				set_irq_line(ASSERT_LINE);
				m_irq_enable = 0;
				m_irq_status |= 0x01;
			}
		}
	}
}

void nes_ks7017_device::write_l(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7017 write_l, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;

	if (offset >= 0xa00 && offset < 0xb00)
		m_latch = ((offset >> 2) & 0x03) | ((offset >> 4) & 0x04);

	if (offset >= 0x1100 && offset < 0x1200)
		prg16_89ab(m_latch);
}

void nes_ks7017_device::write_ex(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7017 write_ex, offset: %04x, data: %02x\n", offset, data));
	offset += 0x20;

	if (offset == 0x0020) // 0x4020
		m_irq_count = (m_irq_count & 0xff00) | data;

	if (offset == 0x0021) // 0x4021
	{
		m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
		m_irq_enable = 1;
	}

	if (offset == 0x0025) // 0x4025
		set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

uint8_t nes_ks7017_device::read_ex(offs_t offset)
{
	LOG_MMC(("ks7017 read_ex, offset: %04x\n", offset));
	offset += 0x20;

	if (offset == 0x0030) // 0x4030
	{
		int temp = m_irq_status;
		m_irq_status &= ~0x01;
		set_irq_line(CLEAR_LINE);
		return temp;
	}

	return get_open_bus();   // open bus
}

/*-------------------------------------------------

 Kaiser Board KS7021A

 Games: GetsuFumaDen

 This board has a 16K fixed PRG bank at 0xc000 and
 a swappable 16K PRG bank at 0x8000. CHR banks are
 selectable by 1K page.

 NES 2.0: mapper 525

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ks7021a_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ks7021a write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			prg16_89ab((data >> 1) & 0x07);
			break;
		case 0x1000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x3000:
			chr1_x(offset & 0x07, data & 0x7f, CHRROM);
			break;
	}
}

/*-------------------------------------------------

 Kaiser Board KS7010

 Games: Akumajo Dracula FDS Conversion

 This board has fixed PRG banks in 0x8000-0xffff.
 0x6000-0x7fff is an 8K swappable PRG bank. This bank
 and the CHR bank are BOTH set by the same latch.
 Moreover, the latch is set by READING certain
 addresses and the exact mask is still unknown...

 NES 2.0: mapper 554

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_ks7010_device::read_m(offs_t offset)
{
//  LOG_MMC(("ks7010 read_m, offset: %04x, data: %02x\n", offset, data));
	return m_prg[m_latch * 0x2000 + offset];
}

u8 nes_ks7010_device::read_h(offs_t offset)
{
//  LOG_MMC(("ks7010 read_h, offset: %04x, data: %02x\n", offset, data));
	if ((offset >= 0x4ab6 && offset <= 0x4ad6) || offset == 0x6be2 || offset == 0x6be3 || offset == 0x6e32 || offset == 0x7ffc) // HACK! FIXME
	{
		m_latch = (offset >> 2) & 0x0f;
		chr8(m_latch, CHRROM);
	}

	return hi_access_rom(offset);
}

/*-------------------------------------------------

 Kaiser Board KS7012

 Games: Zanac FDS Conversion

 NES 2.0: mapper 346

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ks7012_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7012 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x60a0)
		prg32(0);
	if (offset == 0x6e36)
		prg32(1);
}

/*-------------------------------------------------

 Kaiser Board KS7013B

 Games: Highway Star bootleg

 NES 2.0: mapper 312

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ks7013b_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7013b write_l, offset: %04x, data: %02x\n", offset, data));
	prg16_89ab(data);
}

void nes_ks7013b_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7013b write_h, offset: %04x, data: %02x\n", offset, data));
	set_nt_mirroring((data & 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Kaiser Board KS7030

 Games: Doki Doki Panic (FDS conversion)

 This board has a complicated memory layout. The last
 32k of the mask ROM is fixed into 0x8000-0xffff, but
 8k of WRAM and 8k of bankable PRG (in two 4k banks)
 are adjacent and overlaid. The ranges are as follows.

   WRAM:                   PRG:
     - 3k, 0x6000-0x6bff     - 1k, 0x6c00-0x6fff (reg 1, last 1k)
     - 2k, 0xb800-0xbfff     - 4k, 0x7000-0x7fff (reg 0)
     - 3k, 0xcc00-0xd7ff     - 3k, 0xc000-0xcc00 (reg 1, initial 3k)

 The two registers latch part of the address in writes
 to 0x8000-0x8fff and 0x9000-0x9fff respectively.

 NES 2.0: mapper 347

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_ks7030_device::read_m(offs_t offset)
{
//  LOG_MMC(("ks7030 read_m, offset: %04x\n", offset));
	if (offset < 0x0c00)         // first of 3k WRAM
		return m_prgram[offset];
	else if (offset < 0x1000)    // last 1k of 4k PRG bank
		return m_prg[m_reg[1] * 0x1000 + offset];
	else                         // 4k PRG
		return m_prg[0x10000 + m_reg[0] * 0x1000 + (offset & 0x0fff)];
}

void nes_ks7030_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("ks7030 write_m, offset: %04x\n", offset));
	if (offset < 0x0c00)         // first 3k of WRAM
		m_prgram[offset] = data;
}

u8 nes_ks7030_device::read_h(offs_t offset)
{
//  LOG_MMC(("ks7030 read_h, offset: %04x\n", offset));
	if (offset < 0x3800 || offset >= 0x5800)    // fixed 32k PRG, split 14k and 10k windows
		return m_prg[0x18000 + (offset & 0x7fff)];
	else if (offset < 0x4000)                   // middle 2k of WRAM
		return m_prgram[offset - 0x2c00];
	else if (offset < 0x4c00)                   // first 3k of 4k PRG bank
		return m_prg[m_reg[1] * 0x1000 + (offset & 0x0fff)];
	else                                        // last 3k of WRAM
		return m_prgram[offset - 0x3800];
}

void nes_ks7030_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ks7030 write_h, offset: %04x\n", offset));
	if (offset < 0x1000)
	{
		set_nt_mirroring(BIT(offset, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		m_reg[0] = offset & 0x07;
	}
	else if (offset < 0x2000)
		m_reg[1] = offset & 0x0f;
	else if (offset >= 0x3800 && offset < 0x4000)    // middle 2k of WRAM
		m_prgram[offset - 0x2c00] = data;
	else if (offset >= 0x4c00 && offset < 0x5800)    // last 3k of WRAM
		m_prgram[offset - 0x3800] = data;
}

/*-------------------------------------------------

 Kaiser Board KS7031

 Games: Dracula II FDS Conversion

 This board is quite weird. It handles 2K PRG chunks
 and the chip contains chunks in reverse order, so
 that the first 2K are actually loaded at the top
 of the 0x8000-0xffff region. Main bank is fixed, while
 the 8K mapped at 0x6000-0x7fff varies with reg writes.

 NES 2.0: mapper 305

 In MAME: Supported.

 -------------------------------------------------*/

uint8_t nes_ks7031_device::read_m(offs_t offset)
{
//  LOG_MMC(("ks7031 read_m, offset: %04x\n", offset));
	return m_prg[(m_reg[(offset >> 11) & 3] * 0x0800) + (offset & 0x7ff)];
}

uint8_t nes_ks7031_device::read_h(offs_t offset)
{
	// here the first 32K are accessed, but in 16x2K blocks loaded in reverse order
	int accessed_2k = (offset >> 11) & 0x0f;
	return m_prg[((0x0f - accessed_2k) * 0x0800) + (offset & 0x7ff)];
}

void nes_ks7031_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7031 write_h, offset: %04x, data: %02x\n", offset, data));
	m_reg[(offset >> 11) & 3] = data & 0x3f;
}

/*-------------------------------------------------

 Kaiser Board KS7037

 Games: Metroid (FDS conversion)

 This PCB maps PRG in 0x7000-0x7fff in a very
 similar fashion to LH10 (see bootleg.cpp)
 but with WRAM split between 0x6000-0x6fff
 and 0xb000-0xbfff.

 NES 2.0: mapper 307

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ks7037_device::update_prg()
{
	prg8_89(m_reg[6]);
	prg8_cd(m_reg[7]);
	set_nt_page(0, CIRAM, m_reg[2] & 1, 1);
	set_nt_page(2, CIRAM, m_reg[3] & 1, 1);
	set_nt_page(1, CIRAM, m_reg[4] & 1, 1);
	set_nt_page(3, CIRAM, m_reg[5] & 1, 1);
}

uint8_t nes_ks7037_device::read_m(offs_t offset)
{
//  LOG_MMC(("ks7037 read_m, offset: %04x\n", offset));
	if (offset < 0x1000)
		return m_prgram[offset];
	else
		return m_prg[0x0f * 0x1000 + (offset & 0x0fff)]; // 4k PRG bank 15 is fixed
}

void nes_ks7037_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7037 write_m, offset: %04x, data: %02x\n", offset, data));
	if (offset < 0x1000)
		m_prgram[offset] = data;
}

uint8_t nes_ks7037_device::read_h(offs_t offset)
{
//  LOG_MMC(("ks7037 read_h, offset: %04x\n", offset));

	if (offset >= 0x3000 && offset < 0x4000)
		return m_prgram[0x1000 + (offset & 0x0fff)];

	return hi_access_rom(offset);
}

void nes_ks7037_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7037 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x3000 && offset < 0x4000)
		m_prgram[0x1000 + (offset & 0x0fff)] = data;
	else
	{
		switch (offset & 0x6001)
		{
			case 0x0000:
				m_latch = data & 7;
				break;
			case 0x0001:
				m_reg[m_latch] = data;
				update_prg();
				break;
		}
	}
}

/*-------------------------------------------------

 Kaiser Board KS7057

 Games: Gyruss

 This PCB has fixed banks after 0xa000 and 8x2K
 swappable banks in 0x6000-0x9fff. Pairs of nibble
 registers (we store joined as bytes) control the banks,
 with $B000/$B001, $B002/$B003, $C000/$C001, $C002/$C003
 selecting 4 banks in 0x8000-0x9fff, and $D000/$D001,
 $D002/$D003, $E000/$E001, $E002/$E003 selecting the
 remaining 4 banks in 0x6000-0x7fff.

 NES 2.0: mapper 302

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_ks7057_device::read_m(offs_t offset)
{
//  LOG_MMC(("ks7057 read_m, offset: %04x\n", offset));
	return m_prg[0x800 * m_reg[((offset >> 11) & 0x03) + 4] + (offset & 0x7ff)];
}

u8 nes_ks7057_device::read_h(offs_t offset)
{
//  LOG_MMC(("ks7057 read_h, offset: %04x\n", offset));
	if (offset < 0x2000)
		return m_prg[0x800 * m_reg[(offset >> 11) & 0x03] + (offset & 0x7ff)];

	return hi_access_rom(offset);
}

void nes_ks7057_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ks7057 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x2000)
		set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	else if (offset >= 0x3000 && offset < 0x6004)
	{
		u8 reg = (((offset >> 11) & 0x0e) | BIT(offset, 1)) - 6;
		if (BIT(offset, 0))
			m_reg[reg] = (m_reg[reg] & 0x0f) | ((data & 0x03) << 4);
		else
			m_reg[reg] = (m_reg[reg] & 0xf0) | (data & 0x0f);
	}
}
