// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Kaiser PCBs


 Here we emulate the following Kaiser bootleg PCBs

 * Kaiser KS202
 * Kaiser KS7012
 * Kaiser KS7013B
 * Kaiser KS7017
 * Kaiser KS7022
 * Kaiser KS7032
 * Kaiser KS7058
 * Kaiser KS7016
 * Kaiser KS7037

 The Kaiser KS7057 bootleg board is emulated in nes_mmc3_clones.cpp


 TODO:
 - FCEUmm lists more Kaiser PCBs:
   * KS7030 (for Yume Koujou Doki Doki Panic by Kaiser?)
   but there seem to be no available dumps...

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

DEFINE_DEVICE_TYPE(NES_KS7058,  nes_ks7058_device,  "nes_ks7058",  "NES Cart Kaiser KS-7058 PCB")
DEFINE_DEVICE_TYPE(NES_KS7022,  nes_ks7022_device,  "nes_ks7022",  "NES Cart Kaiser KS-7022 PCB")
DEFINE_DEVICE_TYPE(NES_KS7032,  nes_ks7032_device,  "nes_ks7032",  "NES Cart Kaiser KS-7032 PCB")
DEFINE_DEVICE_TYPE(NES_KS202,   nes_ks202_device,   "nes_ks202",   "NES Cart Kaiser KS-202 PCB")
DEFINE_DEVICE_TYPE(NES_KS7017,  nes_ks7017_device,  "nes_ks7017",  "NES Cart Kaiser KS-7017 PCB")
DEFINE_DEVICE_TYPE(NES_KS7012,  nes_ks7012_device,  "nes_ks7012",  "NES Cart Kaiser KS-7012 PCB")
DEFINE_DEVICE_TYPE(NES_KS7013B, nes_ks7013b_device, "nes_ks7013b", "NES Cart Kaiser KS-7013B PCB")
DEFINE_DEVICE_TYPE(NES_KS7031,  nes_ks7031_device,  "nes_ks7031",  "NES Cart Kaiser KS-7031 PCB")
DEFINE_DEVICE_TYPE(NES_KS7016,  nes_ks7016_device,  "nes_ks7016",  "NES Cart Kaiser KS-7016 PCB")
DEFINE_DEVICE_TYPE(NES_KS7037,  nes_ks7037_device,  "nes_ks7037",  "NES Cart Kaiser KS-7037 PCB")


nes_ks7058_device::nes_ks7058_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7058, tag, owner, clock)
{
}

nes_ks7022_device::nes_ks7022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7022, tag, owner, clock), m_latch(0)
{
}

nes_ks7032_device::nes_ks7032_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_latch(0), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
{
}

nes_ks7032_device::nes_ks7032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_ks7032_device(mconfig, NES_KS7032, tag, owner, clock)
{
}

nes_ks202_device::nes_ks202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_ks7032_device(mconfig, NES_KS202, tag, owner, clock)
{
}

nes_ks7017_device::nes_ks7017_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7017, tag, owner, clock), m_latch(0), m_irq_count(0), m_irq_status(0), m_irq_enable(0), irq_timer(nullptr)
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

nes_ks7031_device::nes_ks7031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7031, tag, owner, clock)
{
}

nes_ks7016_device::nes_ks7016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7016, tag, owner, clock)
{
}

nes_ks7037_device::nes_ks7037_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KS7037, tag, owner, clock)
{
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
	save_item(NAME(m_reg));
}

void nes_ks7032_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
	memset(m_reg, 0, sizeof(m_reg));
	prg_update();
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

void nes_ks7016_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_ks7016_device::pcb_reset()
{
	prg8_89(0xc);
	prg8_ab(0xd);
	prg8_cd(0xe);
	prg8_ef(0xf);
	chr8(0, CHRRAM);

	m_reg = 4;
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
	prg8_ab(0x1e);
	prg8_cd(0);
	prg8_ef(0x1f);
	chr8(0, CHRRAM);

	memset(m_reg, 0, sizeof(m_reg));
	m_latch = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Kaiser Board KS7058

 Games: Tui Do Woo Ma Jeung

 Writes to 0xf000-0xffff set 4k chr banks. Namely, if
 offset&0x80 is 0 the lower 4k are set, if it is 1 the
 upper 4k are set.

 iNES: mapper 171

 In MESS: Supported.

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

 In MESS: Supported?

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

 Games: A few FDS conversions like Bubble Bobble
        or SMB2

 iNES:

 TODO: available dumps do not seem to use WRAM...
 yet m_reg[4] should switch WRAM bank... investigate!

 In MESS: Supported?

 -------------------------------------------------*/

void nes_ks7032_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (m_irq_count == 0xffff)
			{
				m_irq_enable = 0;
				m_irq_count = 0;
				hold_irq_line();
			}
			else
				m_irq_count++;
		}
	}
}

void nes_ks7032_device::prg_update()
{
	prg8_89(m_reg[1]);
	prg8_ab(m_reg[2]);
	prg8_cd(m_reg[3]);
}

void nes_ks7032_device::ks7032_write(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7032_write, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			m_irq_count = (m_irq_count & 0xfff0) | (data & 0x0f);
			break;
		case 0x1000:
			m_irq_count = (m_irq_count & 0xff0f) | ((data & 0x0f) << 4);
			break;
		case 0x2000:
			m_irq_count = (m_irq_count & 0xf0ff) | ((data & 0x0f) << 8);
			break;
		case 0x3000:
			m_irq_count = (m_irq_count & 0x0fff) | ((data & 0x0f) << 12);
			break;
		case 0x4000:
			m_irq_enable = 1;
			break;
		case 0x6000:
			m_latch = data & 0x07;
			break;
		case 0x7000:
			m_reg[m_latch] = data;
			prg_update();
			break;
	}
}

uint8_t nes_ks7032_device::read_m(offs_t offset)
{
	LOG_MMC(("ks7032 read_m, offset: %04x\n", offset));
	return m_prg[((m_reg[4] * 0x2000) + (offset & 0x1fff)) & (m_prg_size - 1)];
}

/*-------------------------------------------------

 Kaiser Board KS202

 Games: Super Mario Bros. 3 (Pirate, Alt)

 iNES:

 In MESS: Supported?

 -------------------------------------------------*/


void nes_ks202_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks202 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x7000:
			m_reg[m_latch] = data;
			prg_update();
			switch (offset & 0xc00)
			{
				case 0x800:
					set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
					break;
				case 0xc00:
					chr1_x(offset & 0x07, data, CHRROM);
					break;
			}
			break;
		default:
			ks7032_write(offset, data);
			break;
	}
}

uint8_t nes_ks202_device::read_m(offs_t offset)
{
	LOG_MMC(("ks202 read_m, offset: %04x\n", offset));
	return m_prgram[offset & 0x1fff];
}

/*-------------------------------------------------

 Kaiser Board KS7017

 Games: Almana no Kiseki FDS conversion

 iNES:

 In MESS: Supported.

 -------------------------------------------------*/

void nes_ks7017_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (!m_irq_count)
			{
				hold_irq_line();
				m_irq_enable = 0;
				m_irq_status |= 0x01;
			}
			else
				m_irq_count--;
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
		return temp;
	}

	return get_open_bus();   // open bus
}

/*-------------------------------------------------

 Kaiser Board KS7012

 Games: Zanac FDS Conversion

 iNES:

 In MESS: Not working

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

 Games: Highway Star FDS Conversion

 iNES:

 In MESS: Supported.

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

 Kaiser Board KS7031

 Games: Dracula II FDS Conversion

 This board is quite weird. It handles 2K PRG chunks
 and the chip contains chunks in reverse order, so
 that the first 2K are actually loaded at the top
 of the 0x8000-0xffff region. Main bank is fixed, while
 the 8K mapped at 0x6000-0x7fff varies with reg writes.

 TODO: understand how SRAM is handled...

 iNES:

 In MESS: Supported.

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

 Kaiser Board KS7016

 Games: Exciting Basket FDS Conversion

 iNES:

 In MESS: Unsupported.

 -------------------------------------------------*/

uint8_t nes_ks7016_device::read_m(offs_t offset)
{
//  LOG_MMC(("ks7016 read_m, offset: %04x\n", offset));
	return m_prg[((m_reg * 0x2000) + (offset & 0x1fff)) & (m_prg_size - 1)];
}

void nes_ks7016_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7016 write_h, offset: %04x, data: %02x\n", offset, data));
	uint8_t mask = offset & 0x30;
	if ((offset & 0x5943) == 0x5943)
		m_reg = (mask == 0x30) ? 0xb : (((offset >> 2) & 0x0f) << 1);
	if ((offset & 0x5943) == 0x5903)
		m_reg = (mask != 0x30) ? 0xb : (((offset >> 2) & 0x0f) << 1);
}


/*-------------------------------------------------

 Kaiser Board KS7037

 Games: Metroid (FDS conversion)

 This PCB maps PRG in 0x7000-0x7fff in a very
 similar fashion to LH10 (see bootleg.cpp)
 but with WRAM split between 0x6000-0x6fff
 and 0xb000-0xbfff.

 iNES:

 In MESS: Unsupported.

 -------------------------------------------------*/

void nes_ks7037_device::update_prg()
{
	prg8_89(m_reg[6]);
	prg8_ab(0xfe);
	prg8_cd(m_reg[7]);
	prg8_ef(0xff);
	set_nt_page(0, CIRAM, m_reg[2] & 1, 1);
	set_nt_page(1, CIRAM, m_reg[3] & 1, 1);
	set_nt_page(2, CIRAM, m_reg[4] & 1, 1);
	set_nt_page(3, CIRAM, m_reg[5] & 1, 1);
}

uint8_t nes_ks7037_device::read_m(offs_t offset)
{
//  LOG_MMC(("ks7037 read_m, offset: %04x\n", offset));
	if (offset < 0x1000)
		return m_prgram[offset & 0x0fff];
	else
		return m_prg[(0x1e * 0x1000) + (offset & 0x0fff)];
}

void nes_ks7037_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("ks7037 write_m, offset: %04x, data: %02x\n", offset, data));
	if (offset < 0x1000)
		m_prgram[offset & 0x0fff] = data;
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
