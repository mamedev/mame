// license:BSD-3-Clause
// copyright-holders:kmg
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Konami VRC clone PCBs


 Here we emulate several pirate PCBs based on VRC2/4 boards

 ***********************************************************************************************************/


#include "emu.h"
#include "vrc_clones.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_2YUDB,       nes_2yudb_device,       "nes_2yudb",       "NES Cart Yu Yu Hakusho - Dragon Ball Z 2 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_900218,      nes_900218_device,      "nes_900218",      "NES Cart 900218 PCB")
DEFINE_DEVICE_TYPE(NES_AX40G,       nes_ax40g_device,       "nes_ax40g",       "NES Cart UNL-AX-40G PCB")
DEFINE_DEVICE_TYPE(NES_AX5705,      nes_ax5705_device,      "nes_ax5705",      "NES Cart AX5705 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_830506C, nes_bmc_830506c_device, "nes_bmc_830506c", "NES Cart BMC 830506C PCB")
DEFINE_DEVICE_TYPE(NES_CITYFIGHT,   nes_cityfight_device,   "nes_cityfight",   "NES Cart City Fighter PCB")
DEFINE_DEVICE_TYPE(NES_SHUIGUAN,    nes_shuiguan_device,    "nes_shuiguan",    "NES Cart Shui Guan Pipe Pirate PCB")
DEFINE_DEVICE_TYPE(NES_T230,        nes_t230_device,        "nes_t230",        "NES Cart T-230 PCB")
DEFINE_DEVICE_TYPE(NES_TF1201,      nes_tf1201_device,      "nes_tf1201",      "NES Cart UNL-TF1201 PCB")
DEFINE_DEVICE_TYPE(NES_TH21311,     nes_th21311_device,     "nes_th21311",     "NES Cart UNL-TH2131-1 PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_SGZ, nes_waixing_sgz_device, "nes_waixing_sgz", "NES Cart Waixing San Guo Zhi PCB")
DEFINE_DEVICE_TYPE(NES_HENGG_SHJY3, nes_hengg_shjy3_device, "nes_hengg_shjy3", "NES Cart Henggedianzi Shen Hua Jian Yun III PCB")


nes_2yudb_device::nes_2yudb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_2YUDB, tag, owner, clock), m_outer(0)
{
}

nes_900218_device::nes_900218_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc2_device(mconfig, NES_900218, tag, owner, clock), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
{
}

nes_ax40g_device::nes_ax40g_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc2_device(mconfig, NES_AX40G, tag, owner, clock)
{
}

nes_ax5705_device::nes_ax5705_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_AX5705, tag, owner, clock)
{
}

nes_bmc_830506c_device::nes_bmc_830506c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_BMC_830506C, tag, owner, clock), m_outer(0)
{
}

nes_cityfight_device::nes_cityfight_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_CITYFIGHT, tag, owner, clock)
{
}

nes_shuiguan_device::nes_shuiguan_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_SHUIGUAN, tag, owner, clock)
{
}

nes_t230_device::nes_t230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_T230, tag, owner, clock)
{
}

nes_tf1201_device::nes_tf1201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_TF1201, tag, owner, clock)
{
}

nes_th21311_device::nes_th21311_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc2_device(mconfig, NES_TH21311, tag, owner, clock), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
{
}

nes_waixing_sgz_device::nes_waixing_sgz_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 chr_match)
	: nes_konami_vrc4_device(mconfig, type, tag, owner, clock), m_chr_match(chr_match)
{
}

nes_waixing_sgz_device::nes_waixing_sgz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_waixing_sgz_device(mconfig, NES_WAIXING_SGZ, tag, owner, clock, 0x06)
{
}

nes_hengg_shjy3_device::nes_hengg_shjy3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_waixing_sgz_device(mconfig, NES_HENGG_SHJY3, tag, owner, clock, 0x04)
{
}



void nes_2yudb_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_outer));

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 3;  // A3
	m_vrc_ls_prg_b = 2;  // A2
}

void nes_2yudb_device::pcb_reset()
{
	m_outer = 0;
	nes_konami_vrc4_device::pcb_reset();
}

void nes_900218_device::device_start()
{
	nes_konami_vrc2_device::device_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));

	// VRC2 pins 3 and 4
	m_vrc_ls_prg_a = 1;  // A1
	m_vrc_ls_prg_b = 0;  // A0
}

void nes_900218_device::pcb_reset()
{
	nes_konami_vrc2_device::pcb_reset();
	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_ax40g_device::device_start()
{
	nes_konami_vrc2_device::device_start();

	// VRC2 pins 3 and 4
	m_vrc_ls_prg_a = 1;  // A1
	m_vrc_ls_prg_b = 0;  // A0
}

void nes_ax5705_device::device_start()
{
	nes_konami_vrc4_device::device_start();

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 1;  // A1
	m_vrc_ls_prg_b = 0;  // A0
}

void nes_bmc_830506c_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_outer));

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 1;  // A1
	m_vrc_ls_prg_b = 0;  // A0
}

void nes_bmc_830506c_device::pcb_reset()
{
	m_outer = 0;
	nes_konami_vrc4_device::pcb_reset();
}

void nes_cityfight_device::device_start()
{
	nes_konami_vrc4_device::device_start();

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 3;  // A3
	m_vrc_ls_prg_b = 2;  // A2
}

void nes_shuiguan_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_reg));

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 3;  // A3
	m_vrc_ls_prg_b = 2;  // A2
}

void nes_shuiguan_device::pcb_reset()
{
	nes_konami_vrc4_device::pcb_reset();
	m_reg = 0;
}

void nes_t230_device::device_start()
{
	nes_konami_vrc4_device::device_start();

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 3;  // A3
	m_vrc_ls_prg_b = 2;  // A2
}

void nes_tf1201_device::device_start()
{
	nes_konami_vrc4_device::device_start();

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 0;  // A0
	m_vrc_ls_prg_b = 1;  // A1
}

void nes_th21311_device::device_start()
{
	nes_konami_vrc2_device::device_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_latch));

	// VRC2 pins 3 and 4
	m_vrc_ls_prg_a = 1;  // A1
	m_vrc_ls_prg_b = 0;  // A0
}

void nes_th21311_device::pcb_reset()
{
	nes_konami_vrc2_device::pcb_reset();
	m_irq_enable = 0;
	m_irq_count = 0;
	m_irq_latch = 0;
}

void nes_waixing_sgz_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_chr_mask));
	save_item(NAME(m_chr_match));

	m_chr_mask = 0xfe;

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 3;  // A3
	m_vrc_ls_prg_b = 2;  // A2
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Board BTL-900218

 Games: Lord of King (Astyanax pirate)

 This board has a VRC2 clone that has been altered to
 add a simple IRQ counter fixed to 1024 CPU cycles.

 NES 2.0: mapper 524

 In MAME: Supported.

 -------------------------------------------------*/

void nes_900218_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			m_irq_count++;
			set_irq_line(BIT(m_irq_count, 10) ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}

void nes_900218_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("900218 write_h, offset: %04x, data: %02x\n", offset, data));

	if ((offset & 0x7000) == 0x7000)
	{
		switch (offset & 0xc)
		{
			case 0x8:
				m_irq_enable = 1;
				break;
			case 0xc:
				m_irq_enable = 0;
				m_irq_count = 0;
				set_irq_line(CLEAR_LINE);
				break;
		}
	}
	else
		nes_konami_vrc2_device::write_h(offset, data);
}

/*-------------------------------------------------

 Board UNL-AX-40G

 Games: Fudou Myouou Den pirate

 VRC2 clone that supports extra single screen mirroring
 modes seen in the original Taito X1-005 board.

 NES 2.0: mapper 527

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ax40g_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ax40g write_h, offset: %04x, data: %02x\n", offset, data));

	nes_konami_vrc2_device::write_h(offset, data);

	if ((offset & 0x7001) == 0x3001)
	{
		set_nt_page(offset & 0x02, CIRAM, BIT(data, 3), 1);
		set_nt_page((offset & 0x02) + 1, CIRAM, BIT(data, 3), 1);
	}
}

/*-------------------------------------------------

 Board UNL-AX5705

 Games: Super Mario Bros. Pocker Mali (Crayon Shin-chan pirate hack)

 VRC4 clone with a few PRG/CHR address lines swapped.

 NES 2.0: mapper 530

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ax5705_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ax5705 write_h, offset: %04x, data: %02x\n", offset, data));

	offset = (offset & ~0x1000) | BIT(offset, 3) << 12;
	switch (offset & 0x7001)
	{
		case 0x0000: case 0x0001: case 0x2000: case 0x2001:
			data = bitswap<4>(data, 1, 2, 3, 0);
			break;
		case 0x3001: case 0x4001: case 0x5001: case 0x6001:
			data = bitswap<3>(data, 1, 2, 0);
			break;
	}
	nes_konami_vrc4_device::write_h(offset, data);
}

/*-------------------------------------------------

 UNL-CITYFIGHT

 Games: City Fighter IV

 VRC4 clone with simple 32K PRG banking (it's a minimal
 hack of Master Fighter II banking). More interestingly
 it adds a 4-bit PCM audio register, not emulated yet.

 NES 2.0: mapper 266

 In MAME: Partially supported.

 -------------------------------------------------*/

void nes_cityfight_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("unl_cityfight write_h, offset: %04x, data: %02x\n", offset, data));

	offset = (offset & ~0x6000) | bitswap<2>(offset, 13, 14) << 13;
	switch (offset & 0x7800)
	{
		case 0x0000:
		case 0x0800:
			break;  // PRG banking at $8000 ignored
		case 0x1000:
			prg32((data >> 2) & 0x03);
			if (!(offset & 3))  // $9000 is also VRC4 mirroring
				nes_konami_vrc4_device::write_h(offset, data);
			break;
		case 0x1800:
			LOG_MMC(("Extended Audio write, data %x!", data & 0x0f)); // pcmwrite(0x4011, (V & 0xf) << 3); (etabeta's original comment, FCEUX code)
			break;
		default:
			nes_konami_vrc4_device::write_h(offset, data);
			break;
	}
}

/*-------------------------------------------------

 BTL-SHUIGUANPIPE

 Games: Shui Guan Pipe (Gimmick Pirate)

 VRC4 clone that differs in its PRG banking only.

 iNES: mapper 183

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_shuiguan_device::read_m(offs_t offset)
{
//  LOG_MMC(("shuiguan read_m, offset: %04x\n", offset));
	return m_prg[(m_reg * 0x2000 + offset) & (m_prg_size - 1)];
}

void nes_shuiguan_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("shuiguan write_m, offset: %04x, data: %02x\n", offset, data));
	if ((offset & 0x1800) == 0x800)
		m_reg = offset & 0x1f;
}

void nes_shuiguan_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("shuiguan write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7800)
	{
		case 0x0000:
		case 0x1000:
			break;  // writes to $8000 and $9000 ignored
		case 0x0800:
			prg8_89(data);
			break;
		case 0x2000:
			prg8_cd(data);
			break;
		case 0x2800:
			prg8_ab(data);
			break;
		default:
			nes_konami_vrc4_device::write_h(offset, data);
			break;
	}
}

/*-------------------------------------------------

 Board UNL-T-230

 Games: Dragon Ball Z IV (Unl)

 VRC4 clone that uses CHRRAM instead of CHRROM and
 has nonstandard PRG banking.

 NES 2.0: mapper 529

 In MAME: Supported.

 TODO: This cart and its appearance on the 2yudb set
 have IRQ timing issues that cause a bouncing status
 bar in fights. Other emulators also have issues
 on the split line so perhaps some noise is expected?

 -------------------------------------------------*/

void nes_t230_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("t230 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			break;  // PRG banking at $8000 ignored
		case 0x2000:
			prg16_89ab(data);
			break;
		default:
			nes_konami_vrc4_device::write_h(offset, data);
			break;
	}
}

/*-------------------------------------------------

 UNL-TF1201

 Games: Leathal Weapon (Leathal Enforcers clone)

 VRC4 clone where the only known difference is in the
 IRQ behavior. This clone does not copy the IRQ reload
 bit to the enable bit on writes to IRQ acknowledge.

 NES 2.0: mapper 298

 In MAME: Supported.

 -------------------------------------------------*/

void nes_tf1201_device::irq_ack_w()
{
	set_irq_line(CLEAR_LINE);
}

/*-------------------------------------------------

 Board UNL-TH2131-1

 Games: Batman pirate (Dynacon)

 VRC2 clone with an added IRQ timer. A selectable 4-bit
 latch controls the duration as it is decremented
 relative to another internal 12-bit counter.

 NES 2.0: mapper 308

 In MAME: Supported.

 -------------------------------------------------*/

void nes_th21311_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			m_irq_count = (m_irq_count + 1) & 0xfff; // 12-bit counter
			if (m_irq_count == 0x800)
				m_irq_latch--;
			if (!m_irq_latch && m_irq_count < 0x800)
				set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_th21311_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("th21311 write_h, offset: %04x, data: %02x\n", offset, data));

	if ((offset & 0x7000) == 0x7000)
	{
		switch (offset & 3)
		{
			case 0:
				m_irq_enable = 0;
				m_irq_count = 0;
				set_irq_line(CLEAR_LINE);
				break;
			case 1:
				m_irq_enable = 1;
				break;
			case 3:
				m_irq_latch = data >> 4;
				break;
		}
	}
	else
		nes_konami_vrc2_device::write_h(offset, data);
}

/*-------------------------------------------------

 Waixing San Guo Zhi Board, Henggedianzi UNL-SHJY3

 Games: San Guo Zhi (mapper 252), several Chinese
 hacks of Dragon Ball games (mapper 253)

 VRC4 clones that substitute 2K of CHRRAM in place for
 two of the standard 1K CHRROM pages. Unusually this is
 done by a GAL looking at certain address bits of the
 PPU's address space, as the PPU writes pattern tables.

 iNES: mapper 252 and mapper 253

 In MAME: Supported.

 TODO: Why does Sanguozhi flicker every second or two?

 -------------------------------------------------*/

void nes_waixing_sgz_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("waixing_sgz write_h, offset: %04x, data: %02x\n", offset, data));

	nes_konami_vrc4_device::write_h(offset, data);

	for (int bank = 0; bank < 8; bank++)
		if ((m_mmc_vrom_bank[bank] & m_chr_mask) == m_chr_match)
			chr1_x(bank, m_mmc_vrom_bank[bank] & 1, CHRRAM);
}

void nes_waixing_sgz_device::chr_w(offs_t offset, u8 data)
{
	device_nes_cart_interface::chr_w(offset, data);

	switch (m_mmc_vrom_bank[BIT(offset, 10, 3)])
	{
		case 0x88:
			m_chr_mask = 0xfc;
			m_chr_match = 0x4c;
			break;
		case 0xc2:
			m_chr_mask = 0xfe;
			m_chr_match = 0x7c;
			break;
		case 0xc8:
			m_chr_mask = 0xfe;
			m_chr_match = 0x04;
			break;
	}
}

/*-------------------------------------------------

 MULTIGAME CARTS BASED ON VRC

 -------------------------------------------------*/

/*-------------------------------------------------

 Board BTL-2YUDB

 Games: 2 in 1 Datach Yu Yu Hakusho + Dragon Ball Z

 VRC4 clone that uses CHRRAM instead of the usual
 CHRROM and has a bit for selecting the outer PRG bank.

 NES 2.0: mapper 520

 In MAME: Supported.

 -------------------------------------------------*/

void nes_2yudb_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("2yudb write_h, offset: %04x, data: %02x\n", offset, data));

	nes_konami_vrc4_device::write_h(offset, data);

	if (offset >= 0x3000 && offset < 0x7000 && !BIT(offset, m_vrc_ls_prg_b))
	{
		u8 bank = ((offset >> 12) - 3) * 2 + BIT(offset, m_vrc_ls_prg_a);
		m_outer = (m_mmc_vrom_bank[bank] & 0x08) << 2;
		set_prg();
	}
}

/*-------------------------------------------------

 Board BMC-830506C

 Games: 1995 Super HiK 4 in 1 (JY-005)

 VRC4 clone with banking for multicart menu.

 NES 2.0: mapper 362

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_830506c_device::irq_ack_w()
{
	set_irq_line(CLEAR_LINE);
}

void nes_bmc_830506c_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_830506c write_h, offset: %04x, data: %02x\n", offset, data));

	nes_konami_vrc4_device::write_h(offset, data);

	if (offset >= 0x3000 && offset < 0x7000 && BIT(offset, m_vrc_ls_prg_b))
	{
		u8 bank = ((offset >> 12) - 3) * 2 + BIT(offset, m_vrc_ls_prg_a);
		m_outer = (m_mmc_vrom_bank[bank] & 0x180) >> 3;
		set_prg();
	}
}
