// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Waixing PCBs

 Here we emulate the various PCBs used by Waixing for its games

 TODO:
 - investigate the PPU issues causing some games not to have sprites (e.g. some games using mappers 15, 164,
   242, 249)

 ***********************************************************************************************************/


#include "emu.h"
#include "waixing.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_WAIXING_A,     nes_waixing_a_device,     "nes_waixing_a",     "NES Cart Waixing Type A PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_A1,    nes_waixing_a1_device,    "nes_waixing_a1",    "NES Cart Waixing Type A (Alt) PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_B,     nes_waixing_b_device,     "nes_waixing_b",     "NES Cart Waixing Type B PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_C,     nes_waixing_c_device,     "nes_waixing_c",     "NES Cart Waixing Type C PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_D,     nes_waixing_d_device,     "nes_waixing_d",     "NES Cart Waixing Type D PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_E,     nes_waixing_e_device,     "nes_waixing_e",     "NES Cart Waixing Type E PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_F,     nes_waixing_f_device,     "nes_waixing_f",     "NES Cart Waixing Type F PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_F1,    nes_waixing_f1_device,    "nes_waixing_f1",    "NES Cart Waixing Type F1 PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_G,     nes_waixing_g_device,     "nes_waixing_g",     "NES Cart Waixing Type G PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_H,     nes_waixing_h_device,     "nes_waixing_h",     "NES Cart Waixing Type H PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_H1,    nes_waixing_h1_device,    "nes_waixing_h1",    "NES Cart Waixing Type H (Alt) PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_I,     nes_waixing_i_device,     "nes_waixing_i",     "NES Cart Waixing Type I PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_J,     nes_waixing_j_device,     "nes_waixing_j",     "NES Cart Waixing Type J PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_SH2,   nes_waixing_sh2_device,   "nes_waixing_sh2",   "NES Cart Waixing SH2 PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_SEC,   nes_waixing_sec_device,   "nes_waixing_sec",   "NES Cart Waixing Security Chip PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_SGZLZ, nes_waixing_sgzlz_device, "nes_waixing_sgzlz", "NES Cart Waixing San Guo Zhong Lie Zhuan PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_FFV,   nes_waixing_ffv_device,   "nes_waixing_ffv",   "NES Cart Waixing Final Fantasy V PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_WXZS,  nes_waixing_wxzs_device,  "nes_waixing_wxzs",  "NES Cart Waixing Wai Xing Zhan Shi PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_DQ8,   nes_waixing_dq8_device,   "nes_waixing_dq8",   "NES Cart Waixing Dragon Quest VIII PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_WXZS2, nes_waixing_wxzs2_device, "nes_waixing_wxzs2", "NES Cart Waixing Wai Xing Zhan Shi 2 PCB")
DEFINE_DEVICE_TYPE(NES_WAIXING_FS304, nes_waixing_fs304_device, "nes_waixing_fs304", "NES Cart Waixing FS-304 PCB")


nes_waixing_a_device::nes_waixing_a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock)
{
}

nes_waixing_a_device::nes_waixing_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_a_device(mconfig, NES_WAIXING_A, tag, owner, clock)
{
}

nes_waixing_a1_device::nes_waixing_a1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_a_device(mconfig, NES_WAIXING_A1, tag, owner, clock)
{
}

nes_waixing_b_device::nes_waixing_b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_a_device(mconfig, NES_WAIXING_B, tag, owner, clock)
{
}

nes_waixing_c_device::nes_waixing_c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_a_device(mconfig, NES_WAIXING_C, tag, owner, clock)
{
}

nes_waixing_d_device::nes_waixing_d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_a_device(mconfig, NES_WAIXING_D, tag, owner, clock)
{
}

nes_waixing_e_device::nes_waixing_e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_a_device(mconfig, NES_WAIXING_E, tag, owner, clock)
{
}

nes_waixing_f_device::nes_waixing_f_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock)
{
}

nes_waixing_f_device::nes_waixing_f_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_waixing_f_device(mconfig, NES_WAIXING_F, tag, owner, clock)
{
}

nes_waixing_f1_device::nes_waixing_f1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_waixing_f_device(mconfig, NES_WAIXING_F1, tag, owner, clock)
{
}

nes_waixing_g_device::nes_waixing_g_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_a_device(mconfig, NES_WAIXING_G, tag, owner, clock)
{
}

nes_waixing_h_device::nes_waixing_h_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock)
{
}

nes_waixing_h_device::nes_waixing_h_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_h_device(mconfig, NES_WAIXING_H, tag, owner, clock)
{
}

nes_waixing_h1_device::nes_waixing_h1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_h_device(mconfig, NES_WAIXING_H1, tag, owner, clock)
{
}

nes_waixing_i_device::nes_waixing_i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_a_device(mconfig, NES_WAIXING_I, tag, owner, clock)
{
}

nes_waixing_j_device::nes_waixing_j_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_a_device(mconfig, NES_WAIXING_J, tag, owner, clock)
{
}

nes_waixing_sh2_device::nes_waixing_sh2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_WAIXING_SH2, tag, owner, clock)
{
}

nes_waixing_sec_device::nes_waixing_sec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_WAIXING_SEC, tag, owner, clock), m_reg(0)
{
}

nes_waixing_sgzlz_device::nes_waixing_sgzlz_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_WAIXING_SGZLZ, tag, owner, clock), m_reg{ 0, 0, 0, 0 }
{
}

nes_waixing_ffv_device::nes_waixing_ffv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_WAIXING_FFV, tag, owner, clock)
{
}

nes_waixing_wxzs_device::nes_waixing_wxzs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_WAIXING_WXZS, tag, owner, clock)
{
}

nes_waixing_dq8_device::nes_waixing_dq8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_WAIXING_DQ8, tag, owner, clock)
{
}

nes_waixing_wxzs2_device::nes_waixing_wxzs2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_WAIXING_WXZS2, tag, owner, clock)
{
}

nes_waixing_fs304_device::nes_waixing_fs304_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_WAIXING_FS304, tag, owner, clock)
{
}


void nes_waixing_a_device::device_start()
{
	mmc3_start();
	save_item(NAME(mapper_ram));
}

void nes_waixing_a_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0xff, 0xff, 0);

	std::fill(std::begin(mapper_ram), std::end(mapper_ram), 0x00);
}

void nes_waixing_g_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0xff, 0xff, 0);

	std::fill(std::begin(mapper_ram), std::end(mapper_ram), 0x00);
	m_mmc_prg_bank[0] = 0x00;
	m_mmc_prg_bank[1] = 0x01;
	m_mmc_prg_bank[2] = 0x3e;
	m_mmc_prg_bank[3] = 0x3f;
	m_mmc_vrom_bank[0] = 0x00;
	m_mmc_vrom_bank[1] = 0x02;
	m_mmc_vrom_bank[2] = 0x04;
	m_mmc_vrom_bank[3] = 0x05;
	m_mmc_vrom_bank[4] = 0x06;
	m_mmc_vrom_bank[5] = 0x07;
	m_mmc_vrom_bank[6] = 0x01;
	m_mmc_vrom_bank[7] = 0x03;
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_waixing_j_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_waixing_j_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg[0] = 0x01;
	m_reg[1] = 0x02;
	m_reg[2] = 0x7e;
	m_reg[3] = 0x7f;
	mmc3_common_initialize(0xff, 0xff, 0);
	set_prg(m_prg_base, m_prg_mask);
}

void nes_waixing_sh2_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_waixing_sh2_device::pcb_reset()
{
	m_reg[0] = m_reg[1] = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
	set_nt_mirroring(PPU_MIRROR_VERT); // first Fire Emblem doesn't properly set mirroring like the other games do
}

void nes_waixing_sec_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_waixing_sec_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_waixing_sgzlz_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_waixing_sgzlz_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
}

void nes_waixing_ffv_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_waixing_ffv_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0x1f);
	chr8(0, m_chr_source);

	m_reg[0] = m_reg[1] = 0;
}

void nes_waixing_wxzs2_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);
}

void nes_waixing_fs304_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_waixing_fs304_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
}





/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Waixing Board Type A

 Games: Columbus - Ougon no Yoake (C), Ji Jia Zhan Shi,
 Jia A Fung Yun, Wei Luo Chuan Qi

 This mapper is quite similar to MMC3, but with two differences:
 mirroring is not the same, and when VROM banks 8,9 are accessed
 they point to CHRRAM and not CHRROM.

 iNES: mapper 74

 In MESS: Supported

 -------------------------------------------------*/

/* MIRROR_LOW and MIRROR_HIGH are swapped! */
void nes_waixing_a_device::set_mirror(uint8_t nt)
{
	switch (nt)
	{
		case 0:
		case 1:
			set_nt_mirroring(nt ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 2:
			set_nt_mirroring(PPU_MIRROR_LOW);
			break;
		case 3:
			set_nt_mirroring(PPU_MIRROR_HIGH);
			break;
		default:
			LOG_MMC(("Mapper set NT to invalid value %02x", nt));
			break;
	}
}

/* Luo Ke Ren X only works with this */
void nes_waixing_a_device::chr_cb(int start, int bank, int source)
{
	int chr_src = (bank <= 9) ? CHRRAM : CHRROM;
	chr1_x(start, bank, chr_src);
}

/* Ji Jia Zhan Shi only works with this */
void nes_waixing_a1_device::chr_cb(int start, int bank, int source)
{
	int chr_src = ((bank == 8) || (bank == 9)) ? CHRRAM : CHRROM;
	chr1_x(start, bank, chr_src);
}

void nes_waixing_a_device::waixing_write(offs_t offset, uint8_t data)
{
	LOG_MMC(("waixing_write, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x2000:
			set_mirror(data);    //maybe data & 0x03?
			break;

		case 0x2001:
			break;

		default:
			txrom_write(offset, data);
			break;
	}
}

uint8_t nes_waixing_a_device::read_l(offs_t offset)
{
	LOG_MMC(("waixing read_l, offset: %04x\n", offset));
	offset += 0x100;
	if (offset >= 0x1000 && offset < 0x1400)
		return mapper_ram[offset & 0x3ff];
	else
		return 0xff;
}

void nes_waixing_a_device::write_l(offs_t offset, uint8_t data)
{
	LOG_MMC(("waixing write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;
	if (offset >= 0x1000 && offset < 0x1400)
		mapper_ram[offset & 0x3ff] = data;
}


/*-------------------------------------------------

 Waixing Board Type B

 Games: Sugoro Quest (C)

 MMC3 clone. This is a minor modification of Mapper 74,
 in the sense that it is the same board except for the
 CHRRAM pages.

 iNES: mapper 191

 In MESS: Supported.

 -------------------------------------------------*/

void nes_waixing_b_device::chr_cb(int start, int bank, int source)
{
	int chr_src = BIT(bank, 7) ? CHRRAM : CHRROM;
	chr1_x(start, bank, chr_src);
}

/*-------------------------------------------------

 Waixing Board Type C

 Games: Ying Lie Qun Xia Zhuan, Young Chivalry

 MMC3 clone. This is a minor modification of Mapper 74,
 in the sense that it is the same board except for the
 CHRRAM pages.

 iNES: mapper 192

 In MESS: Supported.

 -------------------------------------------------*/

void nes_waixing_c_device::chr_cb(int start, int bank, int source)
{
	int chr_src = ((bank == 0x08) || (bank == 0x09) || (bank == 0x0a) || (bank == 0x0b)) ? CHRRAM : CHRROM;
	chr1_x(start, bank, chr_src);
}

/*-------------------------------------------------

 Waixing Board Type D

 Games: Super Robot Taisen (C)

 MMC3 clone. This is a minor modification of Mapper 74,
 in the sense that it is the same board except for the
 CHRRAM pages.

 iNES: mapper 194

 In MESS: Supported.

 -------------------------------------------------*/

void nes_waixing_d_device::chr_cb(int start, int bank, int source)
{
	int chr_src = (bank < 0x02) ? CHRRAM : CHRROM;
	chr1_x(start, bank, chr_src);
}

/*-------------------------------------------------

 Waixing Board Type E

 Games: Captain Tsubasa Vol. II (C), Chaos World, God
 Slayer (C), Zu Qiu Xiao Jiang

 MMC3 clone. This is a minor modification of Mapper 74,
 in the sense that it is the same board except for the
 CHRRAM pages.

 iNES: mapper 195

 In MESS: Supported.

 -------------------------------------------------*/

void nes_waixing_e_device::chr_cb(int start, int bank, int source)
{
	int chr_src = (bank < 0x04) ? CHRRAM : CHRROM;
	chr1_x(start, bank, chr_src);
}

/*-------------------------------------------------

 Waixing Board Type F, F1

 Games: Tenchi wo Kurau II translations (mapper 198),
 and (mapper 199) Chengjisihan, Tangmu Lixian Ji,
 Fengse Huanxiang, Datang Fengyun VI Dai

 MMC3 clone with an extra 4K WRAM at 0x5000-0x5fff.
 Mapper 198 banks its 8K CHR RAM and 199 does not.

 iNES: mappers 198, 199

 In MAME: Supported.

 TODO: Sort out G board games which were assigned
 to mapper 199 previously.

 -------------------------------------------------*/

u8 nes_waixing_f_device::read_l(offs_t offset)
{
// LOG_MMC(("waixing_f read_l, offset: %04x\n", offset));

	offset += 0x100;
	if (!m_prgram.empty() && offset >= 0x1000)
		return m_prgram[offset & 0x0fff & (m_prgram.size() - 1)];

	return get_open_bus();
}

void nes_waixing_f_device::write_l(offs_t offset, u8 data)
{
// LOG_MMC(("waixing_f write_l, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;
	if (!m_prgram.empty() && offset >= 0x1000)
		m_prgram[offset & 0x0fff & (m_prgram.size() - 1)] = data;
}

void nes_waixing_f1_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
// ignore CHR banking as all F1 games use 8K unbanked CHR RAM
}

/*-------------------------------------------------

 Waixing Board Type G

 Games: San Guo Zhi 2, Dragon Ball Z Gaiden (C), Dragon
 Ball Z II (C)

 MMC3 clone, capable of switching all 4 PRG banks

 iNES: mapper 199

 In MESS: Supported.

 -------------------------------------------------*/

void nes_waixing_g_device::chr_cb(int start, int bank, int source)
{
	int chr_src = (bank < 0x08) ? CHRRAM : CHRROM;
	chr1_x(start, bank, chr_src);
}

void nes_waixing_g_device::set_chr(uint8_t chr, int chr_base, int chr_mask)
{
	uint8_t chr_page = (m_latch & 0x80) >> 5;

	chr_cb(chr_page ^ 0, chr_base | (m_mmc_vrom_bank[0] & chr_mask), chr);
	chr_cb(chr_page ^ 1, chr_base | (m_mmc_vrom_bank[6] & chr_mask), chr);
	chr_cb(chr_page ^ 2, chr_base | (m_mmc_vrom_bank[1] & chr_mask), chr);
	chr_cb(chr_page ^ 3, chr_base | (m_mmc_vrom_bank[7] & chr_mask), chr);
	chr_cb(chr_page ^ 4, chr_base | (m_mmc_vrom_bank[2] & chr_mask), chr);
	chr_cb(chr_page ^ 5, chr_base | (m_mmc_vrom_bank[3] & chr_mask), chr);
	chr_cb(chr_page ^ 6, chr_base | (m_mmc_vrom_bank[4] & chr_mask), chr);
	chr_cb(chr_page ^ 7, chr_base | (m_mmc_vrom_bank[5] & chr_mask), chr);
}

void nes_waixing_g_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t cmd;
	LOG_MMC(("waixing_g write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0001:
			cmd = m_latch & 0x0f;
			switch (cmd)
			{
				case 0: case 1:
				case 2: case 3: case 4: case 5:
					m_mmc_vrom_bank[cmd] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
					break;
				case 6:
				case 7:
				case 8:
				case 9:
					m_mmc_prg_bank[cmd - 6] = data;
					set_prg(m_prg_base, m_prg_mask);
					break;
				case 0x0a: case 0x0b:
					m_mmc_vrom_bank[cmd - 4] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
					break;
			}
			break;

		default:
			waixing_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 Waixing Board Type H

 Games: Ying Xiong Yuan Yi Jing Chuan Qi, Yong Zhe Dou E
 Long - Dragon Quest VII

 MMC3 clone. More info to come.

 Notice that Chinese Zelda translation stops working if
 WRAM protect bit is ignored (i.e. if writes to 0x2001
 are skipped)! OTOH, Chinese Monster Maker 1/2 translations
 and Zheng Ba ShiJi stop working if WRAM protect is
 accounted. So we split the board into two subtypes

 iNES: mapper 245

 In MESS: Supported.

 -------------------------------------------------*/

void nes_waixing_h_device::chr_cb(int start, int bank, int source)
{
	if (source == CHRROM)
		chr1_x(start, bank, source);
}

void nes_waixing_h_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t cmd;
	LOG_MMC(("waixing_h write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0001:
			cmd = m_latch & 0x07;
			switch (cmd)
			{
				case 0:     // in this case we set prg_base in addition to m_mmc_vrom_bank!
					m_prg_base = (data << 5) & 0x40;
					m_prg_mask = 0x3f;
					set_prg(m_prg_base, m_prg_mask);
					[[fallthrough]];
				case 1:
				case 2: case 3: case 4: case 5:
					m_mmc_vrom_bank[cmd] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
					break;
				case 6:
				case 7:
					m_mmc_prg_bank[cmd - 6] = data;
					set_prg(m_prg_base, m_prg_mask);
					break;
				case 0x0a: case 0x0b:   // CHR switches in 1K banks only, and bank1 & bank3 are controlled by these
					m_mmc_vrom_bank[cmd - 4] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
					break;
			}
			break;

		case 0x2001:
			break;

		default:
			txrom_write(offset, data);
			break;
	}
}

void nes_waixing_h1_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t cmd;
	LOG_MMC(("waixing_h1 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0001:
			cmd = m_latch & 0x07;
			switch (cmd)
			{
				case 0:     // in this case we set prg_base in addition to m_mmc_vrom_bank!
					m_prg_base = (data << 5) & 0x40;
					m_prg_mask = 0x3f;
					set_prg(m_prg_base, m_prg_mask);
					[[fallthrough]];
				case 1:
				case 2: case 3: case 4: case 5:
					m_mmc_vrom_bank[cmd] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
					break;
				case 6:
				case 7:
					m_mmc_prg_bank[cmd - 6] = data;
					set_prg(m_prg_base, m_prg_mask);
					break;
				case 0x0a: case 0x0b:   // CHR switches in 1K banks only, and bank1 & bank3 are controlled by these
					m_mmc_vrom_bank[cmd - 4] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
					break;
			}
			break;

		default:
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 Waixing Board Type J

 Games: Final Fantasy III (C)

 MMC3 clone.

 In MESS: Preliminary support.

 -------------------------------------------------*/

void nes_waixing_j_device::set_prg( int prg_base, int prg_mask )
{
	uint8_t prg_flip = (m_latch & 0x40) ? 2 : 0;

	prg_cb(0, m_reg[0 ^ prg_flip]);
	prg_cb(1, m_reg[1]);
	prg_cb(2, m_reg[2 ^ prg_flip]);
	prg_cb(3, m_reg[3]);
}

void nes_waixing_j_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t cmd;
	LOG_MMC(("waixing_f write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0001:
			cmd = m_latch & 0x07;
			switch (cmd)
			{
				case 0: case 1: // these do not need to be separated: we take care of them in set_chr!
				case 2: case 3: case 4: case 5:
					m_mmc_vrom_bank[cmd] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
					break;
				case 6:
				case 7:
				case 8:
				case 9:
					m_reg[cmd - 6] = data;
					set_prg(m_prg_base, m_prg_mask);
					break;
			}
			break;

//      case 0x2001:
//          break;

		default:
			waixing_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 Waixing SH2 Board

 Games: Fire Emblem (C), Fire Emblem Gaiden (C),
 Zhentian Shi Yongshi

 MMC3 clone with MMC2-like CHR banking.

 iNES: mapper 165

 In MAME: Supported.

 -------------------------------------------------*/

void nes_waixing_sh2_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	int bank1 = m_mmc_vrom_bank[m_reg[0] ? 1 : 0] >> 2;
	int bank2 = m_mmc_vrom_bank[m_reg[1] ? 4 : 2] >> 2;

	chr4_0(bank1, bank1 ? CHRROM : CHRRAM);
	chr4_4(bank2, bank2 ? CHRROM : CHRRAM);
}

u8 nes_waixing_sh2_device::chr_r(offs_t offset)
{
	int val = device_nes_cart_interface::chr_r(offset);

	switch (offset & 0xff8)
	{
		case 0xfd0:
		case 0xfe8:
			m_reg[BIT(offset, 12)] = BIT(offset, 3);
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;
	}

	return val;
}

/*-------------------------------------------------

 Waixing Board with Security Chip

 Games: Duo Bao Xiao Ying Hao - Guang Ming yu An Hei Chuan Shuo,
 Myth Struggle, San Shi Liu Ji, Shui Hu Zhuan

 MMC3 clone

 iNES: mapper 249

 In MESS: Partially Supported.

 -------------------------------------------------*/

void nes_waixing_sec_device::prg_cb(int start, int bank)
{
	if (m_reg)
		bank = bitswap<8>(bank & 0x1f,7,6,5,2,1,3,4,0);

	prg8_x(start, bank);
}

void nes_waixing_sec_device::chr_cb(int start, int bank, int source)
{
	if (m_reg)
		bank = bitswap<8>(bank, 5,4,2,6,7,3,1,0);

	chr1_x(start, bank, source);
}

void nes_waixing_sec_device::write_l(offs_t offset, uint8_t data)
{
	LOG_MMC(("waixing_sec write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1000)
	{
		m_reg = data & 0x02;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 Waixing San Guo Zhong Lie Zhuan Board

 Games: Fan Kong Jing Ying, San Guo Zhong Lie Zhuan, Xing
 Ji Zheng Ba, Chong Wu Da Jia Zu Bu Luo Fen Zheng

 iNES: mapper 178

 In MESS: Supported.

 Implementations wildly vary between emulators, but
 Cah4e3's implementation boots up both the games and
 the educational carts that assumedly use this board.

 TODO: Is this even correct compared to real hardware?

 -------------------------------------------------*/

void nes_waixing_sgzlz_device::write_l(offs_t offset, uint8_t data)
{
	LOG_MMC(("waixing_sgzlz write_l, offset: %04x, data: %02x\n", offset, data));
	if (offset >= 0x700 && offset <= 0xEFF)
	{
		m_reg[offset & 0x03] = data;
		const uint8_t hbank = m_reg[1] & 0x7;
		const uint8_t lbank = m_reg[2];
		const uint8_t bank = (lbank << 3) | hbank;

		// NESDev docs do it like this:
		// case 0x700:
		//  set_nt_mirroring(data ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		// case 0x701:
		//  reg1 = (value >> 1);
		//  bank = reg1 + (reg2 << 2);
		//  prg32(bank);
		//  break;
		// case 0x702:
		//  reg2 = value;
		//  break;

		if (BIT(m_reg[0], 1))
		{
			// UNROM-ish mode
			prg16_89ab(bank);
			if (BIT(m_reg[0], 3))
			{
				prg16_cdef((lbank << 3) | 6 | BIT(m_reg[1], 0));
			}
			else
			{
				prg16_cdef((lbank << 3) | 7);
			}
		}
		else
		{   // NROM mode
			if (BIT(m_reg[0], 3))
			{
				prg16_89ab(bank);
				prg16_cdef(bank);
			}
			else
			{
				prg32(bank >> 1);
			}
		}

		set_nt_mirroring(BIT(m_reg[0], 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}
}

/*-------------------------------------------------

 Waixing Final Fantasy V Board

 Games: Darkseed, Digital Dragon, Final Fantasy V, Pocket
 Monster Red

 iNES: mapper 164

 In MESS: Supported.

 -------------------------------------------------*/

void nes_waixing_ffv_device::write_l(offs_t offset, uint8_t data)
{
	uint8_t helper;
	LOG_MMC(("waixing_ffv write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100; /* the checks work better on addresses */

	if ((offset & 0x1200) == 0x1000)
	{
		m_reg[BIT(offset, 8)] = data;
		helper = BIT(m_reg[1], 0) << 5;
		switch (m_reg[0] & 0x70)
		{
			case 0x00:
			case 0x20:
			case 0x40:
			case 0x60:
				prg16_89ab(helper | ((m_reg[0] >> 1) & 0x10) | (m_reg[0] & 0x0f));
				prg16_cdef(helper & 0x1f);
				break;
			case 0x50:
				prg32((helper >> 1) | (m_reg[0] & 0x0f));
				break;
			case 0x70:
				prg16_89ab(helper | ((m_reg[0] << 1) & 0x10) | (m_reg[0] & 0x0f));
				prg16_cdef(helper & 0x1f);
				break;
		}
	}
}

/*-------------------------------------------------

 Waixing Zhan Shi Board

 Games: Wai Xing Zhan Shi

 Simple mapper: writes to 0x8000-0xffff sets prg32 banks to
 (offset>>3)&f. written data&3 sets the mirroring (with
 switched high/low compared to the standard one).

 A crc check is required to support Dragon Quest VIII (which
 uses a slightly different board)

 iNES: mapper 242

 In MESS: Supported.

 -------------------------------------------------*/

void nes_waixing_wxzs_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("waixing_zs write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(offset >> 3);

	switch (data & 0x03)
	{
		case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
		case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
		case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
	}
}

/*-------------------------------------------------

 Waixing Dragon Quest VIII Board

 Games: Dragon Quest VIII

 Simple mapper: writes to 0x8000-0xffff sets prg32 banks to
 (offset>>3)&f.

 iNES: mapper 242

 In MESS: Supported.

 -------------------------------------------------*/

void nes_waixing_dq8_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("waixing_dq8 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(offset >> 3);
}


/*-------------------------------------------------

 Waixing WXZS2 / PS2 board

 Games: Wai Xing Zhan Shi 2 (aka Phantasy Star 2),
 Bao Xiao Tien Guo, Bio Hazard, Pokemon Gold, Subor (R)

 FIXME: This used to be mapper 15. NesDev research says the
 only two games found on board K-1029 (now in multigame.cpp)
 are the two Contra multicarts. All other Waixing games
 appear to be ROM hacks to work with mapper 15. Games on
 this device need to be reassigned to correct device and/or
 replaced with original dumps that match the original boards.

 -------------------------------------------------*/

void nes_waixing_wxzs2_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t flip = BIT(data, 7);
	uint8_t helper = data << 1;

	LOG_MMC(("waixing_wxzs2 write_h, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	switch (offset & 0x0fff)
	{
		case 0x000:
			prg8_89((helper + 0) ^ flip);
			prg8_ab((helper + 1) ^ flip);
			prg8_cd((helper + 2) ^ flip);
			prg8_ef((helper + 3) ^ flip);
			break;
		case 0x001:
			helper |= flip;
			prg8_89(helper);
			prg8_ab(helper + 1);
			prg8_cd(helper + 1);
			prg8_ef(helper + 1);
			break;
		case 0x002:
			helper |= flip;
			prg8_89(helper);
			prg8_ab(helper);
			prg8_cd(helper);
			prg8_ef(helper);
			break;
		case 0x003:
			helper |= flip;
			prg8_89(helper);
			prg8_ab(helper + 1);
			prg8_cd(helper);
			prg8_ef(helper + 1);
			break;
	}
}

/*-------------------------------------------------

 Board UNL-FS304

 Games: A Link to the Past by Waixing

 iNES: mapper 162? (only found in UNIF format)

 In MESS: Supported.

 -------------------------------------------------*/

void nes_waixing_fs304_device::write_l(offs_t offset, uint8_t data)
{
	LOG_MMC(("fs304 write_l, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;
	if (offset >= 0x1000)
	{
		m_reg[BIT(offset, 8, 2)] = data;
		int bank = ((m_reg[2] & 0x0f) << 4) | BIT(m_reg[1], 1) | (m_reg[0] & 0x0e);
		prg32(bank);
		chr8(0, CHRRAM);
	}
}
