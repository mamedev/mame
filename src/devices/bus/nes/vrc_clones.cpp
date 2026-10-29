// license:BSD-3-Clause
// copyright-holders:kmg,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Konami VRC clone PCBs

 iNES mappers: 183, 252, 253, 266, 272, 298, 308, 362, 447, 520, 524, 527, 528, 529, 530, 544


 Here we emulate several pirate PCBs based on VRC2/4 boards

 ***********************************************************************************************************/


#include "emu.h"
#include "vrc_clones.h"

#define LOG_HIFREQ (1U << 1)

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_2YUDB,       nes_2yudb_device,       "nes_2yudb",       "NES Cart Yu Yu Hakusho - Dragon Ball Z 2 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_900218,      nes_900218_device,      "nes_900218",      "NES Cart 900218 PCB")
DEFINE_DEVICE_TYPE(NES_AX40G,       nes_ax40g_device,       "nes_ax40g",       "NES Cart UNL-AX-40G PCB")
DEFINE_DEVICE_TYPE(NES_AX5705,      nes_ax5705_device,      "nes_ax5705",      "NES Cart AX5705 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_830506C, nes_bmc_830506c_device, "nes_bmc_830506c", "NES Cart BMC 830506C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_831128C, nes_bmc_831128c_device, "nes_bmc_831128c", "NES Cart BMC 831128C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_KL06,    nes_bmc_kl06_device,    "nes_bmc_kl06",    "NES Cart BMC KL-06 PCB")
DEFINE_DEVICE_TYPE(NES_CITYFIGHT,   nes_cityfight_device,   "nes_cityfight",   "NES Cart City Fighter PCB")
DEFINE_DEVICE_TYPE(NES_SHUIGUAN,    nes_shuiguan_device,    "nes_shuiguan",    "NES Cart Shui Guan Pipe Pirate PCB")
DEFINE_DEVICE_TYPE(NES_T230,        nes_t230_device,        "nes_t230",        "NES Cart T-230 PCB")
DEFINE_DEVICE_TYPE(NES_TF1201,      nes_tf1201_device,      "nes_tf1201",      "NES Cart UNL-TF1201 PCB")
DEFINE_DEVICE_TYPE(NES_TH21311,     nes_th21311_device,     "nes_th21311",     "NES Cart UNL-TH2131-1 PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_SGZ, nes_waixing_sgz_device, "nes_waixing_sgz", "NES Cart Waixing San Guo Zhi PCB")
DEFINE_DEVICE_TYPE(NES_HENGG_SHJY3, nes_hengg_shjy3_device, "nes_hengg_shjy3", "NES Cart Henggedianzi Shen Hua Jian Yun III PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_FS306, nes_waixing_fs306_device, "nes_waixing_fs306", "NES Cart Waixing FS306 PCB")
DEFINE_DEVICE_TYPE(NES_JY_YY840820C, nes_jy_yy840820c_device, "nes_jy_yy840820c", "NES Cart J.Y. Company YY840820C PCB")
DEFINE_DEVICE_TYPE(NES_TMNT2_563, nes_tmnt2_563_device, "nes_tmnt2_563", "NES Cart TMNT II pirate (NES 2.0 mapper 563) PCB")
DEFINE_DEVICE_TYPE(NES_SANGO2_559, nes_sango2_559_device, "nes_sango2_559", "NES Cart Sangokushi II pirate (NES 2.0 mapper 559) PCB")
DEFINE_DEVICE_TYPE(NES_AKUMAJO_272, nes_akumajo_272_device, "nes_akumajo_272", "NES Cart Akumajou Special pirate (NES 2.0 mapper 272) PCB")


nes_2yudb_device::nes_2yudb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_2YUDB, tag, owner, clock), m_outer(0)
{
}

nes_900218_device::nes_900218_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc2_device(mconfig, NES_900218, tag, owner, clock), m_irq_count(0), m_irq_enable(0)
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

nes_bmc_831128c_device::nes_bmc_831128c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_BMC_831128C, tag, owner, clock), m_reg(0)
{
}

nes_bmc_kl06_device::nes_bmc_kl06_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_BMC_KL06, tag, owner, clock), m_reg(0)
{
}

nes_cityfight_device::nes_cityfight_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_CITYFIGHT, tag, owner, clock), m_dac(*this, "dac")
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
	: nes_konami_vrc2_device(mconfig, NES_TH21311, tag, owner, clock), m_irq_count(0), m_irq_enable(0)
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

void nes_bmc_831128c_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_reg));
}

void nes_bmc_831128c_device::pcb_reset()
{
	nes_konami_vrc4_device::pcb_reset();

	m_reg = 0;
	prg32((m_prg_chunks >> 1) - 1);
}

void nes_bmc_kl06_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_reg));

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 3;  // A1
	m_vrc_ls_prg_b = 2;  // A0
}

void nes_bmc_kl06_device::pcb_reset()
{
	m_reg = 0;
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

TIMER_CALLBACK_MEMBER(nes_900218_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		m_irq_count++;
		set_irq_line(BIT(m_irq_count, 10) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void nes_900218_device::write_h(offs_t offset, u8 data)
{
	LOG("900218 write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("ax40g write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("ax5705 write_h, offset: %04x, data: %02x\n", offset, data);

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

 VRC4 clone (A2/A3) with CPU A13/A14 swapped, its PRG banking replaced by one 32 KiB bank select at $900C,
 and a 4-bit DAC at $980C for PCM speech (NESdev NES 2.0 Mapper 266).

 NES 2.0: mapper 266

 In MAME: Supported.

 Tested: City Fighter IV (Asia) (En) (Pirate) plays; its speech is written to the DAC in three bursts in the first
 25 seconds and is heard in the recorded sound at the same times. The DAC's circuit and level are not documented.

 -------------------------------------------------*/

void nes_cityfight_device::device_add_mconfig(machine_config &config)
{
	// the board's 4-bit DAC, mixed into the console audio through the cartridge connector. How the DAC is built and its
	// output level are not documented (the NESdev talk page asks); a 4-bit R-2R ladder at half the slot's level is used
	DAC_4BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.5);
}

void nes_cityfight_device::write_h(offs_t offset, u8 data)
{
	LOG("unl_cityfight write_h, offset: %04x, data: %02x\n", offset, data);

	offset = (offset & ~0x6000) | bitswap<2>(offset, 13, 14) << 13;
	switch (offset & 0x7800)
	{
		case 0x0000:
		case 0x0800:
			break;  // PRG banking at $8000 ignored
		case 0x1000:
			// NESdev NES 2.0 Mapper 266: $9000 (mask $F00C) is the VRC4's mirroring register; $900C (mask $F80C), a
			// write the VRC4 signals on its /WR9003 pin, selects the 32 KiB PRG bank (PRG A16-A15)
			if ((offset & 0x0c) == 0x0c)
				prg32(BIT(data, 2, 2));
			else if (!(offset & 0x0c))
				nes_konami_vrc4_device::write_h(offset, data);
			break;
		case 0x1800:
			// $980C (mask $F80C): 4-bit unsigned PCM data for the board's own DAC
			if ((offset & 0x0c) == 0x0c)
				m_dac->write(data & 0x0f);
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
	LOGMASKED(LOG_HIFREQ, "shuiguan read_m, offset: %04x\n", offset);
	return m_prg[(m_reg * 0x2000 + offset) & (m_prg_size - 1)];
}

void nes_shuiguan_device::write_m(offs_t offset, u8 data)
{
	LOG("shuiguan write_m, offset: %04x, data: %02x\n", offset, data);
	if ((offset & 0x1800) == 0x800)
		m_reg = offset & 0x1f;
}

void nes_shuiguan_device::write_h(offs_t offset, u8 data)
{
	LOG("shuiguan write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("t230 write_h, offset: %04x, data: %02x\n", offset, data);

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

TIMER_CALLBACK_MEMBER(nes_th21311_device::irq_timer_tick)
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

void nes_th21311_device::write_h(offs_t offset, u8 data)
{
	LOG("th21311 write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("waixing_sgz write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("2yudb write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_830506c write_h, offset: %04x, data: %02x\n", offset, data);

	nes_konami_vrc4_device::write_h(offset, data);

	if (offset >= 0x3000 && offset < 0x7000 && BIT(offset, m_vrc_ls_prg_b))
	{
		u8 bank = ((offset >> 12) - 3) * 2 + BIT(offset, m_vrc_ls_prg_a);
		m_outer = (m_mmc_vrom_bank[bank] & 0x180) >> 3;
		set_prg();
	}
}

/*-------------------------------------------------

 Board BMC-831128C

 Games: 1995 New Series Super 2 in 1

 Weird clone that seems to be part Konami VRC4 (IRQ),
 and part Sunsoft FME-7 (banking/mirroring).

 NES 2.0: mapper 528

 In MAME: Supported.

 -------------------------------------------------*/


u8 nes_bmc_831128c_device::read_m(offs_t offset)
{
	u8 data = 0;
	if (m_reg == 1)
		data = device_nes_cart_interface::read_m(offset);
	else
		data = m_prg[(m_reg * 0x2000 + offset) & (m_prg_size - 1)];
	LOGMASKED(LOG_HIFREQ, "bmc_831128c read_m, offset: %04x, data: %02x\n", offset, data);
	return data;
}

void nes_bmc_831128c_device::write_m(offs_t offset, u8 data)
{
	LOGMASKED(LOG_HIFREQ, "bmc_831128c write_m, offset: %04x, data: %02x\n", offset, data);

	if (m_reg == 1)
		device_nes_cart_interface::write_m(offset, data);
}

void nes_bmc_831128c_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_831128c write_h, offset: %04x, data: %02x\n", offset, data);

	u8 page = offset >> 12;
	if (page != 2 && page != 4)  // ignore writes except $Axxx and $Cxxx
		return;

	int outer = BIT(offset, 14) << 4;
	prg16_cdef(outer ? m_prg_chunks - 1 : 7);

	u8 reg = offset & 0x0f;
	switch (reg)
	{
		case 0x0: case 0x1: case 0x2: case 0x3:
		case 0x4: case 0x5: case 0x6: case 0x7:
			chr1_x(reg, (outer << 4) | data, m_chr_source);
			break;
		case 0x8:
			m_reg = data;
			break;
		case 0x9:
		case 0xa:
			prg8_x(reg - 9, outer + (data & (outer | 0x0f)));
			break;
		// case 0xb: do nothing
		case 0xc:
			set_mirror(data);
			break;
		case 0xd:
			irq_ctrl_w(data);
			break;
		case 0xe:
			irq_ack_w();
			break;
		case 0xf:
			m_irq_count_latch = data;
			break;
	}
}

/*-------------------------------------------------

 Board BMC-KL-06

 Games: 1993 New 860 in 1 Over-Valued Golden Version Games

 VRC4 clone with banking for multicart menu.

 NES 2.0: mapper 447

 In MAME: Supported.

 TODO: There must be jumper settings since this is
 showing up as a 1650 in 1.

 -------------------------------------------------*/

void nes_bmc_kl06_device::set_prg()
{
	u8 outer = (m_reg & 0x03) << 4;

	if (BIT(m_reg, 2))
	{
		u8 bank_lo = outer | m_mmc_prg_bank[0];
		u8 bank_hi = outer | m_mmc_prg_bank[1];
		u8 mode = ~m_reg & 0x02;
		prg8_89(bank_lo & ~mode);
		prg8_ab(bank_hi & ~mode);
		prg8_cd(bank_lo | mode);
		prg8_ef(bank_hi | mode);
	}
	else
		nes_konami_vrc4_device::set_prg(outer, 0x0f);
}

void nes_bmc_kl06_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_kl06 write_m, offset: %04x, data: %02x\n", offset, data);

	// 2K RAM for Crisis Force overlays control register
	nes_konami_vrc4_device::write_m(offset, data);

	if (!(m_reg & 1))  // lock bit
	{
		m_reg = offset;
		set_prg();
		set_chr();
	}
}


/*-------------------------------------------------

 Waixing FS306 board

 Games: Bawang de Dalu (Waixing's translation of Namco's
 Sangokushi II: Haou no Tairiku)

 NESdev NES 2.0 Mapper 544: a VRC4 clone, with its A0/A1
 inputs on CPU A10/A11 (subaddresses $000/$400/$800/$C00),
 and a GAL that replaces two or four CHR banks with 2 KiB of
 CHR-RAM, as on INES Mapper 253:
 - VRC4 PRG select #0/#1, CHR select ($B000-$EFFF) and IRQ
   ($F000-$FFFF) as on a VRC4; 512 KiB of CHR-ROM
 - $9C00 (mask $9C04): 8 KiB PRG-ROM bank at $C000-$DFFF
   (the VRC4's external select)
 - $9C0C-$9C0F (mask $9C07): 1 KiB CIRAM bank for each of the
   four nametables
 - the GAL takes as its command the CHR bank number (CHR A10-
   A17) of a PPU write while bit 7 of it is set, as on INES
   Mapper 195; power-on value $80:
     $80 = banks $028-$02B   $C0 = banks $046-$047
     $82 = banks $000-$003   $C2 = banks $07C-$07D
     $88 = banks $04C-$04F   $C8 = banks $004-$005 (not
     $8A = banks $064-$067         $00A as on mapper 195)
     $CA = no CHR-RAM; bit 4 set: no CHR-RAM
   in both 256 KiB halves of the CHR-ROM; the 2 KiB of CHR-RAM
   are mirrored once in the four-bank settings, and every
   setting maps the same 2 KiB
 - 8 KiB of battery-backed PRG-RAM at $6000-$7FFF

 - the IRQ occurs one M2 cycle later than on a VRC4 (NESdev:
   it "seems" to): the VRC4 core's assertion reaches /IRQ at
   the next M2 rise; an acknowledge in between cancels it, and
   an acknowledge clears /IRQ at once, as on a VRC4 (NESdev
   documents only the assertion)

 Not documented, so not emulated: the effect of the VRC4's PRG
 swap bit on the third bank (it swaps $8000 and $C000 as on a
 VRC4 here), and the power-on CIRAM banks (0, 1, 0, 1 here:
 vertical mirroring).

 NES 2.0: mapper 544

 In MAME: Supported.

 -------------------------------------------------*/

nes_waixing_fs306_device::nes_waixing_fs306_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_WAIXING_FS306, tag, owner, clock), m_prg2(0), m_gal(0x80), m_irq_delay(0)
{
	std::fill(std::begin(m_ciram), std::end(m_ciram), 0);
	m_vrc_ls_prg_a = 11;  // A11
	m_vrc_ls_prg_b = 10;  // A10
}

void nes_waixing_fs306_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_prg2));
	save_item(NAME(m_ciram));
	save_item(NAME(m_gal));
	save_item(NAME(m_irq_delay));
}

void nes_waixing_fs306_device::pcb_reset()
{
	m_prg2 = 0;
	m_gal = 0x80;
	m_irq_delay = 0;
	m_ciram[0] = m_ciram[2] = 0;
	m_ciram[1] = m_ciram[3] = 1;
	nes_konami_vrc4_device::pcb_reset();
	update_ciram();
}

void nes_waixing_fs306_device::set_prg()
{
	prg8_x(0 ^ m_prg_flip, m_mmc_prg_bank[0] & 0x1f);
	prg8_x(1, m_mmc_prg_bank[1] & 0x1f);
	prg8_x(2 ^ m_prg_flip, m_prg2 & 0x1f);
	prg8_x(3, 0x1f);
}

bool nes_waixing_fs306_device::gal_chr_is_ram(u16 bank) const
{
	static const u8 first_bank[2][4] =
	{
		{ 0x28, 0x00, 0x4c, 0x64 },     // four banks
		{ 0x46, 0x7c, 0x04, 0xff }      // two banks ($CA: none)
	};
	if (BIT(m_gal, 4))
		return false;
	const int two = BIT(m_gal, 6);
	const u8 first = first_bank[two][(BIT(m_gal, 3) << 1) | BIT(m_gal, 1)];
	if (first == 0xff)
		return false;
	const u8 b = bank & 0xff;      // the same banks in both 256 KiB halves
	return (b >= first) && (b < first + (two ? 2 : 4));
}

void nes_waixing_fs306_device::set_chr()
{
	for (int i = 0; i < 8; i++)
	{
		const u16 bank = m_mmc_vrom_bank[i] & 0x1ff;
		if (gal_chr_is_ram(bank))
			chr1_x(i, bank, CHRRAM);   // 2 KiB of CHR-RAM: the bank wraps to it
		else
			chr1_x(i, bank, CHRROM);
	}
}

void nes_waixing_fs306_device::chr_w(offs_t offset, u8 data)
{
	// the GAL watches the CHR bank number on PPU writes
	const u16 bank = m_mmc_vrom_bank[(offset >> 10) & 7] & 0x1ff;
	device_nes_cart_interface::chr_w(offset, data);
	if (BIT(bank, 7) && (m_gal != (bank & 0xff)))
	{
		m_gal = bank & 0xff;
		set_chr();
	}
}

void nes_waixing_fs306_device::m2_w(int state, offs_t address, bool write)
{
	if (!state)
		return;
	// an assertion from the previous M2 rise reaches the edge now, before this cycle's count
	if (m_irq_delay)
	{
		m_irq_delay = 0;
		set_irq_line(ASSERT_LINE);
	}
	irq_clock();
}

void nes_waixing_fs306_device::vrc_irq_w(int state)
{
	if (state == ASSERT_LINE)
		m_irq_delay = 1;
	else
	{
		m_irq_delay = 0;
		set_irq_line(CLEAR_LINE);
	}
}

void nes_waixing_fs306_device::update_ciram()
{
	for (int i = 0; i < 4; i++)
		set_nt_page(i, CIRAM, m_ciram[i] & 1, 1);
}

void nes_waixing_fs306_device::write_h(offs_t offset, u8 data)
{
	LOG("waixing fs306 write_h, offset: %04x, data: %02x\n", offset, data);

	if ((offset & 0x7c00) == 0x1c00)   // $9C00-$9FFF: the VRC4's external select
	{
		if (BIT(offset, 2))
		{
			m_ciram[offset & 3] = data;
			update_ciram();
		}
		else
		{
			m_prg2 = data;
			set_prg();
		}
		return;
	}
	nes_konami_vrc4_device::write_h(offset, data);
}


/*-------------------------------------------------

 J.Y. Company YY840820C board

 Games: 1995 Super HiK 5-in-1 (JY-048)

 NESdev NES 2.0 Mapper 398: a VRC4 clone wired A0/A1 (VRC4f)
 with an outer bank register in the same address range and a
 GNROM-like mode. The outer register is the low byte of the
 address of every write to $8000-$FFFF (the VRC4 takes the same
 write); power-on value $C0. Bit 7 selects the mode:
 - 0: VRC4, PRG from the first 128 KiB (VRC4 banks ANDed with
   $0F), CHR from the first CHR chip (512 KiB, 1 KiB banks ANDed
   with $1FF)
 - 1: GNROM-like: one 32 KiB PRG bank, PRG A17-A16 from register
   bits 7-6 and PRG A15 from bit 2 of VRC4 CHR register 0; one
   8 KiB CHR bank from the second CHR chip (CHR A19 set in the
   NES 2.0 image), CHR A16 from register bit 6 and CHR A15-A13
   from bits 2-0 of VRC4 CHR register 0
 The NESdev page names these fields (PRG A17..A16, CHR A16, the
 CHR chip select, the mode select, power-on $C0); the address
 table of the GNROM-like mode is taken from FCEUmm's 398.c.
 Mirroring and the IRQ are the VRC4's.

 Not documented, so as in FCEUmm: a console reset sets the outer
 register back to $C0 and clears the VRC4.

 NES 2.0: mapper 398

 In MAME: Supported.

 -------------------------------------------------*/

nes_jy_yy840820c_device::nes_jy_yy840820c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_JY_YY840820C, tag, owner, clock), m_outer(0xc0)
{
}

void nes_jy_yy840820c_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_outer));
}

void nes_jy_yy840820c_device::pcb_reset()
{
	// VRC4f: VRC4 A0/A1 on CPU A0/A1
	m_vrc_ls_prg_a = 1;
	m_vrc_ls_prg_b = 0;
	m_vrc_ls_chr = 0;
	m_outer = 0xc0;
	nes_konami_vrc4_device::pcb_reset();
	set_prg();
	set_chr();
}

void nes_jy_yy840820c_device::set_prg()
{
	if (BIT(m_outer, 7))
		prg32((BIT(m_outer, 6, 2) << 1) | BIT(m_mmc_vrom_bank[0], 2));
	else
		nes_konami_vrc4_device::set_prg(0x00, 0x0f);
}

void nes_jy_yy840820c_device::set_chr()
{
	if (BIT(m_outer, 7))
		chr8(0x40 | (BIT(m_outer, 6) << 3) | (m_mmc_vrom_bank[0] & 0x07), CHRROM);
	else
		nes_konami_vrc4_device::set_chr(0x000, 0x1ff);
}

void nes_jy_yy840820c_device::write_h(offs_t offset, u8 data)
{
	LOG("jy yy840820c write_h, offset: %04x, data: %02x\n", offset, data);

	m_outer = offset & 0xff;
	nes_konami_vrc4_device::write_h(offset, data);
	// the GNROM-like mode takes PRG A15 from CHR register 0, and the mode can change with any write
	set_prg();
	set_chr();
}

/*-------------------------------------------------

 Teenage Mutant Ninja Turtles II pirate (NES 2.0 mapper 563)

 Games: Teenage Mutant Ninja Turtles II - The Arcade Game (Asia)
 (En) (Pirate)

 NESdev has no page for this mapper; it is taken from FCEUmm's
 563.c: a VRC2 clone (VRC2 A0/A1 on CPU A0/A1) with an outer bank
 register and its own IRQ at $F000-$FFFF, decoded by A4-A2:
 - $F008: enable the IRQ and clear its counter
 - $F00C: disable the IRQ
 - $F018: outer bank: PRG OR = reg << 4, CHR OR = reg << 7, with
   256 KiB PRG (AND $1F) and 256 KiB CHR (AND $FF) inner banks
 - $F01C: clear the counter
 - every write to $F000-$FFFF acknowledges the IRQ
 FCEUmm's comment says the counter is "a sixteen-stage counter
 triggering on falling PA12" and approximates it by raising the
 IRQ every two scanlines. Here the falling edges of PPU A12 are
 counted and the IRQ is raised on every 16th edge while enabled,
 which with sprite patterns at $1000 (8 falls per line) is every
 two scanlines; the stage that drives the IRQ is not documented
 and is inferred from that comment. Submapper 1 (128 KiB inner
 banks unless outer bit 1 is set, in FCEUmm) is not handled: the
 only known dump is submapper 0. Power-on and reset: all 0.

 Checked against the known dump's own code: its IRQ handler
 ($FFD0) stores A to $FF04 as the acknowledge (the values written
 there are whatever A holds), counts down a number of IRQs set to
 (target line - 17) / 2 and then disables the IRQ and runs its
 split; in play the countdown ends on line 51, the bottom of the
 status bar. Its title screen has no logo: the nametable holds
 only the HI-SCORE line and the turtle select, and OAM only the
 cursor.

 NES 2.0: mapper 563

 In MAME: Supported.

 -------------------------------------------------*/

nes_tmnt2_563_device::nes_tmnt2_563_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc2_device(mconfig, NES_TMNT2_563, tag, owner, clock), m_prg{ 0, 0 }, m_outer(0), m_irq_enable(0), m_irq_count(0), m_a12(0)
{
}

void nes_tmnt2_563_device::device_start()
{
	nes_konami_vrc2_device::device_start();
	save_item(NAME(m_prg));
	save_item(NAME(m_outer));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_a12));
}

void nes_tmnt2_563_device::pcb_reset()
{
	// VRC2 A0/A1 on CPU A0/A1
	m_vrc_ls_prg_a = 1;
	m_vrc_ls_prg_b = 0;
	m_vrc_ls_chr = 0;
	nes_konami_vrc2_device::pcb_reset();
	m_prg[0] = m_prg[1] = 0;
	m_outer = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
	set_irq_line(CLEAR_LINE);
	update_banks();
}

void nes_tmnt2_563_device::update_banks()
{
	const int prg_or = (m_outer << 4) & ~0x1f;
	const int chr_or = (m_outer << 7) & ~0xff;
	prg8_89((m_prg[0] & 0x1f) | prg_or);
	prg8_ab((m_prg[1] & 0x1f) | prg_or);
	prg8_cd(0x1e | prg_or);
	prg8_ef(0x1f | prg_or);
	for (int i = 0; i < 8; i++)
		chr1_x(i, (m_mmc_vrom_bank[i] & 0xff) | chr_or, m_chr_source);
}

void nes_tmnt2_563_device::ppu_a12_level(int state)
{
	if (m_a12 && !state)
	{
		m_irq_count++;
		if (m_irq_enable && !(m_irq_count & 0x0f))
			set_irq_line(ASSERT_LINE);
	}
	m_a12 = state;
}

void nes_tmnt2_563_device::write_h(offs_t offset, u8 data)
{
	LOG("tmnt2 563 write_h, offset: %04x, data: %02x\n", offset, data);

	if ((offset & 0x7000) == 0x7000)
	{
		set_irq_line(CLEAR_LINE);
		switch (offset & 0x1c)
		{
			case 0x08: m_irq_enable = 1; m_irq_count = 0; break;
			case 0x0c: m_irq_enable = 0; break;
			case 0x18: m_outer = data; update_banks(); break;
			case 0x1c: m_irq_count = 0; break;
		}
		return;
	}
	if ((offset & 0x5000) == 0x0000)
		m_prg[BIT(offset, 13)] = data;     // $8000 and $A000
	nes_konami_vrc2_device::write_h(offset, data);
	update_banks();
}

/*-------------------------------------------------

 Sangokushi II pirate (NES 2.0 mapper 559)

 Games: Sangokushi II - Haou no Tairiku (China) (Ja) (Subor)
 (Pirate)

 NESdev has no page for this mapper; it is taken from FCEUmm's
 559.c: a VRC4 clone (VRC4 A0/A1 on CPU A10/A11) whose $B000-$FFFF
 registers take the high nibble of the data when A10 is set, and
 whose external select output (VRC4 address $9003, CPU $9C00-
 $9FFF) loads:
 - A2 set: the CIRAM page (bit 0) of nametable A1-A0
 - A2 clear: the third 8 KiB PRG bank (the one the VRC4 swap bit
   moves between $8000 and $C000)
 The nametables come from those four pages alone; the VRC4's
 mirroring register has no effect (FCEUmm sets the nametables
 only from them). PRG banks are ANDed with $1F, CHR with $1FF.
 Power-on values (FCEUmm): pages 0, 0, 1, 1, third bank $FE.

 NES 2.0: mapper 559

 In MAME: Supported.

 -------------------------------------------------*/

nes_sango2_559_device::nes_sango2_559_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_SANGO2_559, tag, owner, clock), m_prg2(0xfe), m_nt{ 0, 0, 1, 1 }
{
}

void nes_sango2_559_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_prg2));
	save_item(NAME(m_nt));
}

void nes_sango2_559_device::pcb_reset()
{
	// VRC4 A0/A1 on CPU A10/A11
	m_vrc_ls_prg_a = 11;
	m_vrc_ls_prg_b = 10;
	m_vrc_ls_chr = 0;
	m_prg2 = 0xfe;
	m_nt[0] = m_nt[1] = 0;
	m_nt[2] = m_nt[3] = 1;
	nes_konami_vrc4_device::pcb_reset();
	set_prg();
	update_ciram();
}

void nes_sango2_559_device::set_prg()
{
	prg8_x(0 ^ m_prg_flip, m_mmc_prg_bank[0] & 0x1f);
	prg8_x(1, m_mmc_prg_bank[1] & 0x1f);
	prg8_x(2 ^ m_prg_flip, m_prg2 & 0x1f);
	prg8_x(3, 0x1f);
}

void nes_sango2_559_device::update_ciram()
{
	for (int i = 0; i < 4; i++)
		set_nt_page(i, CIRAM, m_nt[i] & 1, 1);
}

void nes_sango2_559_device::write_h(offs_t offset, u8 data)
{
	LOG("sango2 559 write_h, offset: %04x, data: %02x\n", offset, data);

	if ((offset & 0x7000) == 0x1000)
	{
		switch (BIT(offset, 10, 2))    // VRC4 address A1-A0
		{
			case 0:
			case 1:
				return;                 // mirroring: replaced by the external nametable pages
			case 2:
				break;                  // PRG-RAM enable and PRG swap
			case 3:                     // external select
				if (BIT(offset, 2))
				{
					m_nt[offset & 3] = data;
					update_ciram();
				}
				else
				{
					m_prg2 = data;
					set_prg();
				}
				return;
		}
	}
	else if (offset >= 0x3000)
		data >>= BIT(offset, 10) ? 4 : 0;   // $B000-$FFFF: A10 selects the data's high nibble
	nes_konami_vrc4_device::write_h(offset, data);
}


/*-------------------------------------------------

 Akumajou Special - Boku Dracula-kun bootleg

 Games: Castle Vania III

 A VRC2/4 clone: its A0 and A1 go straight to the VRC's A0 and
 A1, and a PAL chip on the same bus adds nametable and IRQ
 control. Two 8 KiB PRG banks ($8000, $A000, any $8xxx and
 $Axxx write), the last 16 KiB fixed at $C000, eight 1 KiB CHR
 banks written a nibble at a time ($B000-$E003: A1 picks the
 bank of a pair, A0 the high nibble), VRC mirroring at $9000
 (0 vertical, 1 horizontal). The PAL chip decodes the address
 with mask $C00C: $8004 sets a mirroring override (data & 3, 2
 and 3 select one-screen page 0 and 1), $800C raises the IRQ,
 $C004 acknowledges it, $C008 enables counting and $C00C
 disables it, clears the counter and acknowledges. The counter
 counts falling edges of PPU A13 and raises the IRQ at 84. The
 same write also reaches the VRC decode, so $8004 and $800C
 write PRG bank 0 as well.

 FCEUmm's 272.c (from NESdev forum thread 15302) and puNES
 describe it; no NESdev page was found. The counted edges are
 those of the PPU address bus, as FCEUmm counts them; puNES
 counts one per eight dots while rendering.

 NES 2.0: mapper 272

 In MAME: Supported.

 -------------------------------------------------*/

nes_akumajo_272_device::nes_akumajo_272_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_AKUMAJO_272, tag, owner, clock)
	, m_mirr(0), m_pal_mirr(0), m_irq_enable(0), m_irq_count(0), m_last_a13(0)
{
	std::fill(std::begin(m_prg), std::end(m_prg), 0);
	std::fill(std::begin(m_chr), std::end(m_chr), 0);
}

void nes_akumajo_272_device::device_start()
{
	common_start();
	save_item(NAME(m_prg));
	save_item(NAME(m_chr));
	save_item(NAME(m_mirr));
	save_item(NAME(m_pal_mirr));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_last_a13));
}

void nes_akumajo_272_device::update_banks()
{
	prg8_89(m_prg[0]);
	prg8_ab(m_prg[1]);
	prg16_cdef(m_prg_chunks - 1);

	chr1_0(m_chr[0], CHRROM);
	chr1_1(m_chr[1], CHRROM);
	chr1_2(m_chr[2], CHRROM);
	chr1_3(m_chr[3], CHRROM);
	chr1_4(m_chr[4], CHRROM);
	chr1_5(m_chr[5], CHRROM);
	chr1_6(m_chr[6], CHRROM);
	chr1_7(m_chr[7], CHRROM);

	switch (m_pal_mirr)
	{
		case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
		default: set_nt_mirroring(m_mirr ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT); break;
	}
}

void nes_akumajo_272_device::pcb_reset()
{
	std::fill(std::begin(m_prg), std::end(m_prg), 0);
	std::fill(std::begin(m_chr), std::end(m_chr), 0);
	m_mirr = m_pal_mirr = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
	m_last_a13 = 0;
	update_banks();
	set_irq_line(CLEAR_LINE);
}

void nes_akumajo_272_device::write_h(offs_t offset, u8 data)
{
	LOG("akumajo_272 write_h, offset: %04x, data: %02x\n", offset, data);

	// the VRC chip: only A12-A15, A1 and A0 matter
	switch ((offset >> 12) & 7)
	{
		case 0:
			m_prg[0] = data;
			break;
		case 1:
			m_mirr = data & 1;
			break;
		case 2:
			m_prg[1] = data;
			break;
		case 3: case 4: case 5: case 6:
		{
			const int i = (((offset >> 12) & 7) - 3) * 2 + BIT(offset, 1);
			if (BIT(offset, 0))
				m_chr[i] = (m_chr[i] & 0x0f) | ((data & 0x0f) << 4);
			else
				m_chr[i] = (m_chr[i] & 0xf0) | (data & 0x0f);
			break;
		}
		default:
			break;
	}

	// the PAL chip on the same bus
	switch (offset & 0x400c)
	{
		case 0x0004:
			m_pal_mirr = data & 3;
			break;
		case 0x000c:
			set_irq_line(ASSERT_LINE);
			break;
		case 0x4004:
			set_irq_line(CLEAR_LINE);
			break;
		case 0x4008:
			m_irq_enable = 1;
			break;
		case 0x400c:
			m_irq_enable = 0;
			m_irq_count = 0;
			set_irq_line(CLEAR_LINE);
			break;
	}

	update_banks();
}

void nes_akumajo_272_device::ppu_addr_bus(u16 addr)
{
	const u8 a13 = BIT(addr, 13);

	if (m_last_a13 && !a13 && m_irq_enable)
	{
		if (++m_irq_count == 84)
		{
			m_irq_count = 0;
			set_irq_line(ASSERT_LINE);
		}
	}
	m_last_a13 = a13;
}
