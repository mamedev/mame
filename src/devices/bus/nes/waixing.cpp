// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Waixing PCBs

 iNES mappers: 74, 162, 164, 165, 178, 191, 192, 194, 195, 198, 199, 223, 224, 242, 245, 249, 558

 Here we emulate the various PCBs used by Waixing for its games

 ***********************************************************************************************************/


#include "emu.h"
#include "waixing.h"

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

DEFINE_DEVICE_TYPE(NES_WAIXING_A,     nes_waixing_a_device,     "nes_waixing_a",     "NES Cart Waixing Type A PCB")
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
DEFINE_DEVICE_TYPE(NES_WAIXING_YC030309, nes_waixing_yc030309_device, "nes_waixing_yc030309", "NES Cart Waixing/Yancheng YC-03-09 PCB")
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

nes_waixing_b_device::nes_waixing_b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_waixing_a_device(mconfig, NES_WAIXING_B, tag, owner, clock), m_submapper(0), m_outer(0)
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
	, m_lpc(*this, "lpc")
	, m_pad(*this, "PAD")
	, m_submapper(0), m_pad_read(0), m_lpc_nibble(0), m_lpc_have_nibble(0)
{
}

nes_waixing_ffv_device::nes_waixing_ffv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_WAIXING_FFV, tag, owner, clock)
	, m_eeprom(*this, "eeprom")
	, m_last_nt(false), m_latch_a0(0), m_latch_a9(0)
{
}

nes_waixing_yc030309_device::nes_waixing_yc030309_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_WAIXING_YC030309, tag, owner, clock)
	, m_eeprom(*this, "eeprom")
	, m_reg{ 0, 0, 0, 0 }
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
	mmc3_common_initialize(0xff, 0xff, 0);

	std::fill(std::begin(mapper_ram), std::end(mapper_ram), 0x00);
}

void nes_waixing_g_device::pcb_reset()
{
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
	save_item(NAME(mapper_ram));
}

void nes_waixing_j_device::pcb_reset()
{
	m_reg[0] = 0x01;
	m_reg[1] = 0x02;
	m_reg[2] = 0x7e;
	m_reg[3] = 0x7f;
	mmc3_common_initialize(0xff, 0xff, 0);

	std::fill(std::begin(mapper_ram), std::end(mapper_ram), 0x00);
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
	m_reg = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_waixing_sgzlz_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_pad_read));
	save_item(NAME(m_lpc_nibble));
	save_item(NAME(m_lpc_have_nibble));
}

void nes_waixing_sgzlz_device::pcb_reset()
{
	prg32(0);
	chr8(0, m_chr_source);

	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	m_pad_read = 0;
	if (m_submapper == 2)
		lpc_reset();
}

void nes_waixing_sgzlz_device::device_add_mconfig(machine_config &config)
{
	TMS5220C(config, m_lpc, 640000).add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.5);
}

static INPUT_PORTS_START( sgzlz )
	PORT_START("PAD")
	PORT_CONFNAME( 0x03, 0x00, "Solder pad (submapper 3 menu)" )
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "3" )
INPUT_PORTS_END

ioport_constructor nes_waixing_sgzlz_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sgzlz );
}

void nes_waixing_sgzlz_device::lpc_reset()
{
	m_lpc->reset();
	m_lpc->data_w(0x60);   // speak external: following data goes to the FIFO
	m_lpc_nibble = 0;
	m_lpc_have_nibble = 0;
}

void nes_waixing_ffv_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_last_nt));
	save_item(NAME(m_latch_a0));
	save_item(NAME(m_latch_a9));
}

// the save EEPROM: a 93C66 with its ORG pin tied to GND (8-bit words)
void nes_waixing_ffv_device::device_add_mconfig(machine_config &config)
{
	EEPROM_93C66_8BIT(config, m_eeprom);
}

// NESdev mapper 164: all registers are initialized to $00 on reset (UxROM mode, so the reset vectors are in 16 KiB bank #31)
void nes_waixing_ffv_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	m_last_nt = false;
	m_latch_a0 = m_latch_a9 = 0;
	chr8(0, m_chr_source);
	update_banks();
}

void nes_waixing_fs304_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_last_nt));
	save_item(NAME(m_chr_a12));
}

void nes_waixing_fs304_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	m_last_nt = false;
	m_chr_a12 = 0;

	update_prg();
	chr8(0, m_chr_source);
}





/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Waixing Board Type A

 Games: Columbus - Ougon no Yoake (C), Ji Jia Zhan Shi,
 Jia A Fung Yun, Wei Luo Chuan Qi, Di Si Ci Ji Qi Ren Da Zhan
 (Robot War IV), Feng Yun (Traitor Legend, original 1997
 release)

 This mapper is quite similar to MMC3, but with two differences:
 mirroring is not the same, and when VROM banks 8,9 are accessed
 they point to CHRRAM and not CHRROM.

 iNES: mapper 74 (PCB 43-393 / 43-406 / 860908C)

 In MAME: Supported

 NESdev (mapper 74): 1K CHR banks #8 and #9 are CHRRAM (2K, not
 mirrored), everything else is CHRROM. NESdev notes that
 Nestopia emulates all of Waixing's MMC3 clones with a written
 value of 2 or 3 at $A000 selecting single-screen mirroring,
 which set_mirror() does too. The publicly available Jia A dump
 (PRG CRC32 78fb3ed6, jafy) has been modified to remove writes
 of zeros to the CHR-ROM banks that are mapped at power-on.

 Two dumps that are on this board in nes.xml are not mapper 74
 according to NESdev:
 - Columbus (columbusc, PRG CRC32 bff2e9ec) is the 2006
   re-release, which is mapper 176 (WAIXING-FS005, NES 2.0
   submapper 2). The original release (columbusca, e4e842d0)
   is mapper 195 and uses board E.
 - Feng Yun (fengyun, PRG CRC32 a9b36a7d) is one of the
   2005/2006 re-releases (a9b36a7d, c7e3f93a), also mapper 176.
 They have been left on this board here.

 TODO: the 1K of mapper RAM at $5000-$53ff is inherited by
 boards B-D, G, I and J, but is not described by NESdev.
 Verify it against hardware.

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
			LOG("Mapper set NT to invalid value %02x", nt);
			break;
	}
}

void nes_waixing_a_device::chr_cb(int start, int bank, int source)
{
	// NESdev mapper 74: the board redirects 1 KiB CHR banks #8 and #9 to 2 KiB of CHR-RAM; all other banks are CHR-ROM
	int chr_src = (bank == 0x08 || bank == 0x09) ? CHRRAM : CHRROM;
	chr1_x(start, bank, chr_src);
}

void nes_waixing_a_device::waixing_write(offs_t offset, uint8_t data)
{
	LOG("waixing_write, offset: %04x, data: %02x\n", offset, data);

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
	LOG("waixing read_l, offset: %04x\n", offset);
	offset += 0x100;
	if (offset >= 0x1000 && offset < 0x1400)
		return mapper_ram[offset & 0x3ff];
	else
		return 0xff;
}

void nes_waixing_a_device::write_l(offs_t offset, uint8_t data)
{
	LOG("waixing write_l, offset: %04x, data: %02x\n", offset, data);
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

 NESdev: Xianfeng Cartoon's translation of Sugoro Quest, with
 128K PRG-ROM + 128K CHR-ROM (bank bit 7 selects the 2K of
 CHRRAM). The 256K + 256K dump is a 4-in-1 multicart with a
 GAL outer bank register at $90aa (see prg_cb/chr_cb).
 The common dump trims the CHR-ROM to 128K, which removes the
 CHR data of the three bonus games, so it runs as submapper 0.

 iNES: mapper 191 (submapper 0 = single game, submapper 1 =
 4-in-1 multicart; iNES 1.0 images with 256K of CHR-ROM are
 treated as submapper 1)

 In MAME: Supported.

 -------------------------------------------------*/

void nes_waixing_b_device::device_start()
{
	nes_waixing_a_device::device_start();
	save_item(NAME(m_outer));
}

void nes_waixing_b_device::pcb_reset()
{
	m_outer = 0;
	nes_waixing_a_device::pcb_reset();
}

// NESdev mapper 191 submapper 1 (4-in-1 multicart, 256K PRG-ROM + 256K CHR-ROM): a GAL snoops writes to
// ($90aa & $f0ff) and keeps D1-D0 as an outer bank register (power-on value 0). The write also reaches the MMC3 clone.
void nes_waixing_b_device::waixing_write(offs_t offset, uint8_t data)
{
	if (m_submapper == 1 && (offset & 0x70ff) == 0x10aa)
	{
		m_outer = data & 0x03;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}

	nes_waixing_a_device::waixing_write(offset, data);
}

// Submapper 1: PRG A15..A14 for CPU $c000-$ffff (bits 2-1 of the 8K bank number) come from the outer bank register,
// so the fixed banks are $18/$19, $1a/$1b, $1c/$1d or $1e/$1f
void nes_waixing_b_device::prg_cb(int start, int bank)
{
	if (m_submapper == 1 && start >= 2)
		bank = (bank & ~0x06) | (m_outer << 1);

	prg8_x(start, bank);
}

// MMC3's CHR A17 output (bit 7 of the 1K bank number) selects CHR-RAM instead of CHR-ROM.
// Submapper 1 with an outer bank register other than 3 keeps CHR-ROM A17 high and disables the CHR-RAM
void nes_waixing_b_device::chr_cb(int start, int bank, int source)
{
	int chr_src = CHRROM;

	if (m_submapper == 1 && m_outer != 3)
		bank |= 0x80;
	else if (BIT(bank, 7))
		chr_src = CHRRAM;

	chr1_x(start, bank, chr_src);
}

/*-------------------------------------------------

 Waixing Board Type C

 Games: Ying Lie Qun Xia Zhuan, Young Chivalry

 MMC3 clone. This is a minor modification of Mapper 74,
 in the sense that it is the same board except for the
 CHRRAM pages.

 NESdev (mapper 192): Waixing FS308 board. CHR banks $08-$0b
 are CHRRAM (4K, not mirrored) and all other banks are CHRROM,
 i.e. mapper 74 with twice the CHRRAM. With a NES 2.0 header the
 CHRRAM size is given separately, so 74 and 192 are emulated
 identically there.

 iNES: mapper 192

 In MAME: Supported.

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

 NESdev (mapper 194): CHR banks $00-$01 are CHRRAM (2K) and all
 other banks are CHRROM; otherwise it behaves like a normal MMC3.

 iNES: mapper 194

 In MAME: Supported.

 -------------------------------------------------*/

void nes_waixing_d_device::chr_cb(int start, int bank, int source)
{
	int chr_src = (bank < 0x02) ? CHRRAM : CHRROM;
	chr1_x(start, bank, chr_src);
}

/*-------------------------------------------------

 Waixing Board Type E (FS303)

 Games: Captain Tsubasa Vol. II (C), Chaos World, God
 Slayer (C), Zu Qiu Xiao Jiang, Columbus (C) (original release
 only), and Du Shen / Du Wang / Luo Ke Ren X (Waixing's
 re-release of the Sugoro Quest translation, originally
 mapper 191)

 MMC3 clone with a GAL that selects which CHR banks are
 CHRRAM instead of CHRROM. The command is the 1 KiB CHR bank
 number (CHR A10-A17) that the MMC3 clone outputs for the PPU
 address being written, not the data: the GAL cannot see CPU
 data bits. A game maps a CHR window to a bank number with bit
 7 set, writes anything to $2007 at that address and maps it
 back (NESdev, iNES Mapper 195, and NewRisingSun's
 measurements in the NESdev forum thread 'FS306: Mixed
 CHR-ROM/CHR-RAM cartridge'):

   $80 = banks $28-$2b    $c0 = banks $46-$47
   $82 = banks $00-$03    $c2 = banks $7c-$7d
   $88 = banks $4c-$4f    $c8 = banks $0a-$0b
   $8a = banks $64-$67    $ca = no CHRRAM

 (these are bank numbers). Bit 7 must be set, bit 6 selects 2K
 instead of 4K of CHRRAM, bit 4 forces CHRROM everywhere, bits 3
 and 1 select the first bank, bits 5, 2 and 0 are ignored. The
 board has 32K of CHRRAM but only A10-A12 are wired to it, so
 banks that differ only in the upper bits ($00, $28) are the
 same RAM. The power-on value is $80 (NESdev); dumps of the
 unhacked games contain RAM junk at banks $28-$2f for that
 reason.

 The commonly circulating Captain Tsubasa II dumps are hacked
 to a fixed setting of $82 (identifiable by the bytes
 60 EA EA EA at .NES file offset $41CC, NESdev), so they expect
 banks $00-$03 to be CHRRAM from the start; images with that
 signature start at $82 here. FCEUX treats all of mapper 195
 as having CHRRAM at banks $00-$03, which only suits that
 setting and the games that use only it (Sugoro Quest
 translations, Crystalis).

 The common Chinese Columbus image (PRG CRC32 bff2e9ec) and the
 Traitor Legend images that are labelled mapper 195
 (a9b36a7d, c7e3f93a) are 2005/2006 re-releases that use
 mapper 176, so they do not belong on this board.

 God Slayer's compatible board adds 4K of non-battery-backed
 PRG-RAM at $5000-$5fff (NESdev). FCEUX gives every mapper 195
 game that RAM, so it is provided for the whole board here
 rather than the 1K inherited from board A.

 iNES: mapper 195

 In MAME: Supported.

 VERIFY: only the $80 and $82 settings have been run by a game.
 Columbus uses $80 from power-on without issuing a command,
 Captain Tsubasa II (unhacked) switches between $80 and $82,
 and Luo Ke Ren X and God Slayer issue $82. The $88, $8a, $c0,
 $c2, $c8 and $ca settings, the bit 4 disable and the
 power-on override for the hacked Tsubasa II dumps follow the
 measurements but have not been run (no such game or dump).

 -------------------------------------------------*/

void nes_waixing_e_device::device_start()
{
	nes_waixing_a_device::device_start();
	save_item(NAME(m_ram5000));
	save_item(NAME(m_gal));
}

void nes_waixing_e_device::pcb_reset()
{
	// must be set before the base class reset, which maps the CHR banks
	// NESdev: the power-on value is $80. The circulating Captain Tsubasa II dumps have the GAL command patched out and
	// expect the $82 setting ($00-$03) from the start; they are identified by 60 EA EA EA at .NES file offset $41cc
	m_gal = 0x80;
	if (m_prg_size > 0x41c0 && m_prg[0x41bc] == 0x60 && m_prg[0x41bd] == 0xea && m_prg[0x41be] == 0xea && m_prg[0x41bf] == 0xea)
		m_gal = 0x82;
	std::fill(std::begin(m_ram5000), std::end(m_ram5000), 0x00);
	nes_waixing_a_device::pcb_reset();
}

uint8_t nes_waixing_e_device::read_l(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "waixing_e read_l, offset: %04x\n", offset);
	offset += 0x100;
	if (offset >= 0x1000 && offset < 0x2000)
		return m_ram5000[offset & 0x0fff];
	else
		return 0xff;
}

void nes_waixing_e_device::write_l(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_HIFREQ, "waixing_e write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;
	if (offset >= 0x1000 && offset < 0x2000)
		m_ram5000[offset & 0x0fff] = data;
}

bool nes_waixing_e_device::gal_chr_is_ram(int bank) const
{
	static const uint8_t first_bank[2][4] =
	{
		{ 0x28, 0x00, 0x4c, 0x64 },     // 4K of CHRRAM
		{ 0x46, 0x7c, 0x0a, 0xff }      // 2K of CHRRAM ($ca: none)
	};

	if (BIT(m_gal, 4))
		return false;

	const int two_k = BIT(m_gal, 6);
	const int sel = (BIT(m_gal, 3) << 1) | BIT(m_gal, 1);
	const int first = first_bank[two_k][sel];
	if (first > 0xfe)
		return false;

	return bank >= first && bank < (first + (two_k ? 2 : 4));
}

void nes_waixing_e_device::chr_cb(int start, int bank, int source)
{
	chr1_x(start, bank, gal_chr_is_ram(bank) ? CHRRAM : CHRROM);
}

// 1 KiB CHR bank number (CHR A10-A17) that the MMC3 clone outputs for one of the eight 1 KiB windows, before any masking
uint8_t nes_waixing_e_device::chr_bank_for_window(int window) const
{
	const int reg = window ^ ((m_latch & 0x80) >> 5);   // which of the six CHR registers feeds this window

	switch (reg)
	{
	case 0: return m_mmc_vrom_bank[0] & ~0x01;
	case 1: return m_mmc_vrom_bank[0] | 0x01;
	case 2: return m_mmc_vrom_bank[1] & ~0x01;
	case 3: return m_mmc_vrom_bank[1] | 0x01;
	default: return m_mmc_vrom_bank[reg - 2];
	}
}

void nes_waixing_e_device::chr_w(offs_t offset, uint8_t data)
{
	// The GAL cannot see CPU data. While PPU /WE is low it looks at the CHR bank number the MMC3 clone is outputting for
	// the address being written, and a bank number with bit 7 set is the command, whatever is written (NewRisingSun's
	// measurements, NESdev forum "FS306: Mixed CHR-ROM/CHR-RAM cartridge"). Those banks are always CHRROM, so a command
	// write does nothing else.
	const uint8_t bank = chr_bank_for_window((offset >> 10) & 7);
	if (BIT(bank, 7) && bank != m_gal)
	{
		LOG("waixing_e GAL command %02x\n", bank);
		m_gal = bank;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}

	nes_waixing_a_device::chr_w(offset, data);
}

/*-------------------------------------------------

 Waixing Board Type F, F1

 Games: Tenchi wo Kurau II translations (mapper 198),
 and (mapper 199) Chengjisihan, Tangmu Lixian Ji,
 Fengse Huanxiang, Datang Fengyun VI Dai

 MMC3 clone with an extra 4K WRAM at 0x5000-0x5fff.
 Mapper 198 banks its 8K CHR RAM and 199 does not.

 NESdev (mapper 198): Tun Shi Tian Di: San Guo Wai Zhuan. 8K
 CHRRAM, 8K battery-backed WRAM at $6000-$7fff and 4K
 non-backed WRAM at $5000-$5fff. 640K of PRG-ROM spread over a
 512K chip (banks $00-$3f) and a 128K chip (banks $40-$4f), so
 the two fixed MMC3 banks are $4e and $4f (see prg_cb).

 NESdev (mapper 199): Waixing FS309, TNROM compatible. 32K of
 battery-backed PRG-RAM, 12K of which are mapped at
 $5000-$7fff, and 8K of unbanked CHRRAM (the games do not
 initialize the MMC3 CHR registers to a contiguous 8K block,
 which is why board F1 ignores CHR banking). The PRG-ROM can be
 larger than 512K. Games: Genghis Khan, Tom Sawyer (Feng Se
 Huan Xiang) and the 1 MiB re-release of San Guo Wai Zhuan.

 iNES: mappers 198, 199 (NESdev lists mapper 223 as a duplicate
 of 199, MAME selects board I for it)

 In MAME: Supported.

 Every F/F1 entry in nes.xml has 4K "wram" ($5000-$5fff) plus 8K
 "bwram" ($6000-$7fff). NESdev describes the 4K at $5000 as
 non-backed on 198 but as part of the battery-backed RAM on
 199, which is not distinguished here for software-list entries.
 Images that declare only battery RAM (the NES 2.0 dumps of
 mapper 199) take the 4K at $5000 from the top of it.

 TODO: Sort out G board games which were assigned
 to mapper 199 previously.

 -------------------------------------------------*/

u8 nes_waixing_f_device::read_l(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "waixing_f read_l, offset: %04x\n", offset);

	offset += 0x100;
	u8 *const ram = exram_ptr(offset);
	return ram ? *ram : get_open_bus();
}

void nes_waixing_f_device::write_l(offs_t offset, u8 data)
{
	LOGMASKED(LOG_HIFREQ, "waixing_f write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	u8 *const ram = exram_ptr(offset);
	if (ram)
		*ram = data;
}

// $5000-$5FFF RAM: the board's volatile PRG-RAM (mapper 198); otherwise, on the FS309 (mapper 199: 32 KiB battery-backed
// PRG-RAM, 12 KiB of it at $5000-$7FFF per NESdev), the 4 KiB of battery RAM above the 8 KiB window at $6000-$7FFF.
// NES 2.0 images of the FS309 games declare only battery RAM, so $5000 read back open bus and they hung at boot
u8 *nes_waixing_f_device::exram_ptr(offs_t offset)
{
	if (offset < 0x1000)
		return nullptr;
	if (!m_prgram.empty())
		return &m_prgram[offset & 0x0fff & (m_prgram.size() - 1)];
	if (m_battery.size() >= 0x3000)
		return &m_battery[0x2000 + (offset & 0x0fff)];
	return nullptr;
}

// NESdev (mapper 198): 640K of PRG-ROM spread over a 512K chip (8K banks $00-$3f) and a 128K chip ($40-$4f), with the two
// fixed MMC3 banks in the second chip ($4e and $4f). FCEUX (M198PW) wraps bank numbers of $50 and up to $40-$4f in the same way.
// Only done for a 640K ROM: mapper 199 uses a conventional (oversize) PRG-ROM layout.
void nes_waixing_f_device::prg_cb(int start, int bank)
{
	if (bank >= 0x50 && get_prg_size() == 0xa0000)
		bank &= 0x4f;

	prg8_x(start, bank);
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

 iNES: none, only selected through the slot (mapper 199
 maps to board F1)

 In MAME: Supported.

 TODO: NESdev describes mapper 199 (FS309) as having unbanked
 8K CHRRAM, and notes that some images set to mapper 199 write
 $02 to $5ff3 and are really mapper 176. Check which of these
 games belong on board F1 or on mapper 176.

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
	LOG("waixing_g write_h, offset: %04x, data: %02x\n", offset, data);

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

 MMC3 clone.

 NESdev (mapper 245): Waixing F003 board, with 8K of
 battery-backed WRAM and 8K of unbanked CHRRAM. The MMC3 CHR
 registers do not bank anything (CHRRAM is wired directly to
 PPU A10-A12), but the MMC3's CHR A11 output drives PRG A19:
 bit 1 of the active CHR bank register (R0/R1 when $8000.7=0,
 R2-R5 when $8000.7=1) selects the 512K half of the PRG-ROM
 for the whole of $8000-$ffff, and the games keep all active
 registers consistent. Only Dragon Quest VII needs this, the
 other games on the board have smaller PRG-ROMs and the board
 acts like TNROM.

 Notice that Chinese Zelda translation stops working if
 WRAM protect bit is ignored (i.e. if writes to 0x2001
 are skipped)! OTOH, Chinese Monster Maker 1/2 translations
 and Zheng Ba ShiJi stop working if WRAM protect is
 accounted. So we split the board into two subtypes

 Board H ignores writes to $a001 and runs those games.
 Board H1 (Shen Mi Jin San Jiao, the Chinese Zelda
 translation) honors the WRAM protect bit.

 NESdev (mapper 245 page and the talk page of mapper 224): the
 board is only known to be needed by Dragon Quest VII. Most
 other images that are marked as mapper 245 are plain mapper 4
 (or mapper 176 for one Zelda dump), and one hacked Zelda dump
 replaces the WRAM enable writes with NOPs.

 iNES: mapper 245

 In MAME: Supported.

 TODO: only CHR register 0 selects the PRG-ROM half here, NESdev
 has registers 0/1 or 2-5, depending on $8000.7, all drive it
 (the game keeps them consistent).

 -------------------------------------------------*/

void nes_waixing_h_device::chr_cb(int start, int bank, int source)
{
	if (source == CHRROM)
		chr1_x(start, bank, source);
}

void nes_waixing_h_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t cmd;
	LOG("waixing_h write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("waixing_h1 write_h, offset: %04x, data: %02x\n", offset, data);

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
			}
			break;

		default:
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 Waixing Board Type I

 MMC3 clone, derived from board A (so it has the same 1K of
 mapper RAM at $5000-$53ff). This variant is not emulated
 separately yet: it behaves exactly like board A.

 iNES: mapper 223 (NESdev lists mapper 223 as a duplicate of
 mapper 199, which is board F1; board I is kept because it is
 used, and is also selectable through the waixing_i slot)

 -------------------------------------------------*/

/*-------------------------------------------------

 Waixing Board Type J

 Games: Final Fantasy III (C)

 MMC3 clone.

 iNES: mapper 224

 In MAME: Preliminary support.

 -------------------------------------------------*/

void nes_waixing_j_device::set_prg( int prg_base, int prg_mask )
{
	uint8_t prg_flip = (m_latch & 0x40) ? 2 : 0;

	prg_cb(0, m_reg[0 ^ prg_flip]);
	prg_cb(1, m_reg[1]);
	prg_cb(2, m_reg[2 ^ prg_flip]);
	prg_cb(3, m_reg[3]);
}

void nes_waixing_j_device::chr_cb(int start, int bank, int source)
{
	// the 2 KiB CHR-RAM window among CHR-ROM banks belongs to the Type A board: here every bank comes from the CHR chip the
	// cart has (some carts have no CHR-ROM at all, so asking the ROM for a bank read through a null pointer)
	nes_txrom_device::chr_cb(start, bank, source);
}

void nes_waixing_j_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t cmd;
	LOG("waixing_j write_h, offset: %04x, data: %02x\n", offset, data);

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

 NESdev (mapper 165): MMC3 registers, PRG, mirroring and IRQ
 with the CHR swapping and CHR latches of the MMC2, and 4K of
 CHRRAM in addition to the CHRROM (CHR page 0 is CHRRAM). The
 game toggles the latch by reading PPU $xFD0 / $xFE0 through
 $2007.

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

 Waixing Board with Security Chip (T9552)

 Games: Duo Bao Xiao Ying Hao - Guang Ming yu An Hei Chuan Shuo,
 Myth Struggle, San Shi Liu Ji, Shui Hu Zhuan

 MMC3 clone with an additional register at $5000-$5fff
 (mask $f000, power-on value $00) which selects how the PRG
 and CHR ROM address lines are scrambled: bits 1-0 select one
 of 4 PRG patterns (A14-A17), bits 2-0 select one of 8 CHR
 patterns (A12-A17). Pattern 2 is the unscrambled bank order.
 All known games write $02 right after reset.

 iNES mapper 249 images store the banks in the order they
 appear with $5000=$00, NES 2.0 mapper 4 submapper 5 images
 in the order they appear with $5000=$02.

 iNES: mapper 249, mapper 4 submapper 5

 In MAME: Supported.

 -------------------------------------------------*/

// T9552 address line scrambling, see https://www.nesdev.org/wiki/T9552
// For each value of the pattern, these lists give the MMC3 bank bit that drives each ROM address line:
// PRG A14-A17 (bits 1-4 of an 8K bank number) and CHR A12-A17 (bits 2-7 of a 1K bank number)
static const u8 t9552_prg_lines[4][4] =
{
	{ 3, 4, 2, 1 },
	{ 4, 3, 1, 2 },
	{ 1, 2, 3, 4 },
	{ 2, 1, 4, 3 }
};

static const u8 t9552_chr_lines[8][6] =
{
	{ 5, 2, 6, 7, 4, 3 },
	{ 4, 5, 3, 2, 7, 6 },
	{ 2, 3, 4, 5, 6, 7 },
	{ 6, 4, 2, 3, 7, 5 },
	{ 5, 3, 7, 6, 2, 4 },
	{ 4, 2, 5, 6, 7, 3 },
	{ 3, 6, 4, 5, 2, 7 },
	{ 2, 5, 6, 7, 3, 4 }
};

// cur: lines for the current $5000 value, ref: lines for the $5000 value of the ROM image's bank order
static int t9552_scramble(int bank, const u8 *cur, const u8 *ref, int first, int count)
{
	int result = bank & ~(((1 << count) - 1) << first);
	for (int i = 0; i < count; i++)
	{
		if (BIT(bank, cur[i]))
			result |= 1 << ref[i];
	}
	return result;
}

void nes_waixing_sec_device::prg_cb(int start, int bank)
{
	prg8_x(start, t9552_scramble(bank, t9552_prg_lines[m_reg & 3], t9552_prg_lines[m_t9552_file_order & 3], 1, 4));
}

void nes_waixing_sec_device::chr_cb(int start, int bank, int source)
{
	chr1_x(start, t9552_scramble(bank, t9552_chr_lines[m_reg & 7], t9552_chr_lines[m_t9552_file_order & 7], 2, 6), source);
}

void nes_waixing_sec_device::write_l(offs_t offset, uint8_t data)
{
	LOG("waixing_sec write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	if (offset >= 0x1000)   // $5000-$5fff, mask $f000
	{
		m_reg = data & 0x07;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 Waixing San Guo Zhong Lie Zhuan Board

 Games: Fan Kong Jing Ying, San Guo Zhong Lie Zhuan, Xing
 Ji Zheng Ba, Chong Wu Da Jia Zu Bu Luo Fen Zheng

 NESdev INES Mapper 178 (Waixing FS305, Nanjing NJ0430, Jncota,
 Hengge Dianzi and GameStar educational computers):
 - $4800: bit 0 mirroring (0 vertical, 1 horizontal), bits 2-1 PRG
   mode (0 NROM-256/BNROM, 1 UNROM, 2 NROM-128, 3 UNROM with bit 0
   of the fixed bank selectable)
 - $4801 low PRG bank (A16-A14), $4802 high PRG bank (A24-A17):
   16 KiB bank = $4802 << 3 | $4801 & 7; the banks follow every
   write to $4800-$4802
 - $4803: 8 KiB PRG-RAM bank at $6000-$7FFF
 - 8 KiB of unbanked CHR-RAM
 Submapper 2 has a TMS5220-compatible LPC speech chip at $5000-$5FFF
 with the interface of mapper 241 (see txc.cpp). Submapper 3 returns
 the 2-bit solder pad value instead of PRG-ROM once a value with bit
 0 set has been written to $6000-$7FFF, and PRG-ROM again after one
 with bit 0 clear; the pad is a machine configuration setting.

 Not documented by NESdev: the registers answer over all of
 $4800-$4FFF (address bits 1-0 pick the register, as in Cah4e3's
 implementation) and power up as 0; the bits above the pad value read
 0; the speech chip clock (640 kHz, the TMS5220's nominal one).

 Checked: a synthetic image with 32 KiB of PRG-RAM reads back the four
 banks written through $4803 and wraps at bank 4. In 42-in-1 Y2K (Asia)
 (En) (Aftermarket) (Pirate).nes the routine at $E8D3 writes $01 to
 $6801, reads $E8E7, masks the byte with 3 and keeps it at $C1: $C1 now
 follows the pad setting, and pads 2 and 3 draw their menus right only
 on the dendy machine. FCEUmm reads PRG-ROM with A0-A1 replaced by the
 pad instead, which would give that routine only 0 and 1 from the
 table at $E8E4. The same stream of speech data gives the same audio
 on submapper 2 as on mapper 241. 32-in-1 Speech Educational Software,
 run on the dendy machine with the Subor keyboard (Enter reaches it),
 uses the chip the way the code does: its routine waits for bit 6 of
 $5FFF to be set, writes $00 to reset, and sends each data byte as its
 low and then its high nibble ORed with $50 (the other submapper 2
 dumps carry the same routine, at $5FF0). The chip's audio comes in
 bursts there, where the build without it is silent. Whether the speech
 sounds right was not compared with a recording or hardware.

 TODO: submapper 1's infrared sensor ($5000 read, bit 7 of a $6000
 write enables its interrupts) is not emulated; NESdev gives neither
 the sensor's idle level nor how its interrupt is raised and cleared.

 iNES: mapper 178

 In MAME: Supported.

 -------------------------------------------------*/

uint8_t nes_waixing_sgzlz_device::read_l(offs_t offset)
{
	if (m_submapper == 2 && offset >= 0x0f00)   // $5000-$5FFF: bit 6 = ready (the chip's READY output is active low)
		return (get_open_bus() & ~0x40) | (m_lpc->readyq_r() ? 0x00 : 0x40);
	return get_open_bus();
}

void nes_waixing_sgzlz_device::write_l(offs_t offset, uint8_t data)
{
	LOG("waixing_sgzlz write_l, offset: %04x, data: %02x\n", offset, data);
	if (offset >= 0x700 && offset <= 0xEFF)
	{
		m_reg[offset & 0x03] = data;
		const uint8_t low = m_reg[1] & 0x7;     // $4801
		const uint8_t high = m_reg[2];          // $4802
		const uint8_t bank = (high << 3) | low;

		if (BIT(m_reg[0], 1))
		{
			// UNROM-ish mode
			prg16_89ab(bank);
			if (BIT(m_reg[0], 2))
			{
				prg16_cdef((high << 3) | 6 | BIT(m_reg[1], 0));
			}
			else
			{
				prg16_cdef((high << 3) | 7);
			}
		}
		else
		{   // NROM mode
			if (BIT(m_reg[0], 2))
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
	else if (m_submapper == 2 && offset >= 0x0f00)
	{
		// the speech chip takes one nibble per write, low nibble of each byte first
		switch (data & 0x50)
		{
			case 0x50:
				if (!m_lpc_have_nibble)
				{
					m_lpc_nibble = data & 0x0f;
					m_lpc_have_nibble = 1;
				}
				else
				{
					m_lpc->data_w(m_lpc_nibble | (data & 0x0f) << 4);
					m_lpc_have_nibble = 0;
				}
				break;
			case 0x00:   // both clear: reset the chip
				lpc_reset();
				break;
		}
	}
}

// the 8 KiB PRG-RAM bank of $6000-$7FFF is selected by $4803
uint32_t nes_waixing_sgzlz_device::ram_index(offs_t offset, size_t size) const
{
	return ((m_reg[3] << 13) | (offset & 0x1fff)) & (size - 1);
}

uint8_t nes_waixing_sgzlz_device::read_m(offs_t offset)
{
	if (!m_battery.empty())
		return m_battery[ram_index(offset, m_battery.size())];
	if (!m_prgram.empty())
		return m_prgram[ram_index(offset, m_prgram.size())];
	return get_open_bus();
}

void nes_waixing_sgzlz_device::write_m(offs_t offset, uint8_t data)
{
	if (m_submapper == 3)
		m_pad_read = BIT(data, 0);
	if (!m_battery.empty())
		m_battery[ram_index(offset, m_battery.size())] = data;
	if (!m_prgram.empty())
		m_prgram[ram_index(offset, m_prgram.size())] = data;
}

uint8_t nes_waixing_sgzlz_device::read_h(offs_t offset)
{
	if (m_submapper == 3 && m_pad_read)
		return m_pad->read() & 0x03;
	return hi_access_rom(offset);
}

/*-------------------------------------------------

 Waixing Final Fantasy V Board

 Games: Darkseed, Digital Dragon, Final Fantasy V, Pocket
 Monster Red

 NESdev also lists Da Hua Xi You, Chibi Maruko-chan, Yue Fei
 Zhuan and Pokemon Gold for this board (PEC-9588 / cy2000-3).
 The WXZS2 device currently carries Da Hua Xi You, Yue Fei
 Zhuan and Pokemon Gold.

 iNES: mapper 164

 In MAME: Supported.

 VERIFY: the 93C66 EEPROM ($5200/$5500) and the 1 bpp CHR mode
 ($5000 bit 7) have not been exercised by any game.

 -------------------------------------------------*/

/*-------------------------------------------------

 Yancheng cy2000-3 / Dongda PEC-9588 (iNES mapper 164), per NESdev:
 $5000: CSQM PPPp  M=0: UxROM, $8000 = 16 KiB bank QPPPp, $C000 = $1F (S=0) or $1C|p<<1 (S=1), vertical mirroring
                   M=1: BxROM, $8000 = 32 KiB bank PPPp, mirroring from $5300.7
                   C: 1 bpp mode (CHR A3 = PPU A0, CHR A12 = PPU A9, both latched on the last rise of PPU A13)
 $5100: PRG A20..A19   $5200: 93C66 EEPROM CS (bit 4), CLK (bit 2), DAT (bit 0)
 $5300: bit 7 = mirroring if M=1 (0 horizontal, 1 vertical)   $5500 (read): bit 2 = EEPROM DAT, inverted

 -------------------------------------------------*/

void nes_waixing_ffv_device::update_banks()
{
	const uint8_t r = m_reg[0];
	const uint8_t high = (m_reg[1] & 0x03) << 5;   // PRG A20..A19 as 16 KiB bank bits 6..5
	if (BIT(r, 4))
	{
		prg32((high >> 1) | (r & 0x0f));
		set_nt_mirroring(BIT(m_reg[3], 7) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	}
	else
	{
		prg16_89ab(high | (BIT(r, 5) << 4) | (r & 0x0f));
		prg16_cdef(high | (BIT(r, 6) ? (0x1c | (BIT(r, 0) << 1)) : 0x1f));
		set_nt_mirroring(PPU_MIRROR_VERT);
	}
}

void nes_waixing_ffv_device::write_l(offs_t offset, uint8_t data)
{
	offset += 0x100;
	switch (offset & 0x1f00)
	{
		case 0x1000: m_reg[0] = data; update_banks(); break;
		case 0x1100: m_reg[1] = data; update_banks(); break;
		case 0x1200:
			m_reg[2] = data;
			m_eeprom->cs_write(BIT(data, 4));
			m_eeprom->di_write(BIT(data, 0));
			m_eeprom->clk_write(BIT(data, 2));
			break;
		case 0x1300: m_reg[3] = data; update_banks(); break;
	}
}

uint8_t nes_waixing_ffv_device::read_l(offs_t offset)
{
	offset += 0x100;
	if ((offset & 0x1f00) == 0x1500)
		return (get_open_bus() & ~0x04) | (m_eeprom->do_read() ? 0x00 : 0x04);
	return get_open_bus();
}

// 1 bpp mode: the pattern address takes CHR A3 and CHR A12 from the nametable address of the last rise of PPU A13
uint8_t nes_waixing_ffv_device::nt_r(offs_t offset)
{
	if (!m_last_nt)
	{
		m_latch_a0 = BIT(offset, 0);
		m_latch_a9 = BIT(offset, 9);
	}
	m_last_nt = true;
	return nes_nrom_device::nt_r(offset);
}

void nes_waixing_ffv_device::nt_w(offs_t offset, uint8_t data)
{
	if (!m_last_nt)
	{
		m_latch_a0 = BIT(offset, 0);
		m_latch_a9 = BIT(offset, 9);
	}
	m_last_nt = true;
	nes_nrom_device::nt_w(offset, data);
}

uint8_t nes_waixing_ffv_device::chr_r(offs_t offset)
{
	m_last_nt = false;
	if (BIT(m_reg[0], 7) && !m_vram.empty())
		return m_vram[((offset & 0x0ff7) | (m_latch_a0 << 3) | (m_latch_a9 << 12)) & (m_vram.size() - 1)];
	return nes_nrom_device::chr_r(offset);
}

void nes_waixing_ffv_device::chr_w(offs_t offset, uint8_t data)
{
	m_last_nt = false;
	if (BIT(m_reg[0], 7) && !m_vram.empty())
		m_vram[((offset & 0x0ff7) | (m_latch_a0 << 3) | (m_latch_a9 << 12)) & (m_vram.size() - 1)] = data;
	else
		nes_nrom_device::chr_w(offset, data);
}

/*-------------------------------------------------

 Waixing/Yancheng YC-03-09 Board

 Games: Pokemon Crystal, Dahua Sanguo (Yancheng); Pet Family, Pet Evolve,
 Pokemon Red/Diamond, Digimon 4, Commandos (Waixing)

 NES 2.0 mapper 558, a later version of the mapper 164 chipset. 32 KiB PRG-ROM
 bank, 8 KiB CHR-RAM, hard-wired mirroring, up to 8 KiB PRG-RAM at 0x6000
 (battery-backed on the Waixing carts) and a 93C66 serial EEPROM on the Yancheng
 carts. All registers (mask 0xff00, except 0x5100 whose mask is 0xfb00) power up
 as 0:
 - 0x5000: PRG A18-A15
 - 0x5100: PRG A20-A19
 - 0x5200: EEPROM, bit 0 DAT, bit 1 CLK, bit 2 CS (after the swap below)
 - 0x5300: bit 0 clear swaps D0 and D1 on writes to 0x5000-0x5200 (it does not
   apply to this register itself); bit 2 clear holds PRG A15 and A16 high, so the
   game boots in 32 KiB bank 3
 - 0x5500 read: bit 2 is the EEPROM DAT input (not inverted, unlike mapper 164)

 iNES: mapper 558

 -------------------------------------------------*/

void nes_waixing_yc030309_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_waixing_yc030309_device::device_add_mconfig(machine_config &config)
{
	EEPROM_93C66_8BIT(config, m_eeprom);
}

void nes_waixing_yc030309_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	chr8(0, m_chr_source);
	update_banks();
}

void nes_waixing_yc030309_device::update_banks()
{
	uint8_t low = m_reg[0] & 0x0f;
	if (!BIT(m_reg[3], 2))
		low |= 0x03;   // PRG A15 and A16 are held high until $5300 bit 2 is set
	prg32(((m_reg[1] & 0x03) << 4) | low);
}

void nes_waixing_yc030309_device::write_l(offs_t offset, uint8_t data)
{
	const offs_t addr = offset + 0x4100;
	LOG("waixing_yc030309 write_l, addr: %04x, data: %02x\n", addr, data);

	// D0 and D1 are swapped on writes to $5000-$5200 while $5300 bit 0 is clear; the registers latch the swapped value
	const uint8_t swapped = BIT(m_reg[3], 0) ? data : bitswap<8>(data, 7, 6, 5, 4, 3, 2, 0, 1);

	if ((addr & 0xff00) == 0x5000)
	{
		m_reg[0] = swapped;
		update_banks();
	}
	else if ((addr & 0xfb00) == 0x5100)
	{
		m_reg[1] = swapped;
		update_banks();
	}
	else if ((addr & 0xff00) == 0x5200)
	{
		m_reg[2] = swapped;
		m_eeprom->cs_write(BIT(swapped, 2));
		m_eeprom->di_write(BIT(swapped, 0));
		m_eeprom->clk_write(BIT(swapped, 1));
	}
	else if ((addr & 0xff00) == 0x5300)
	{
		m_reg[3] = data;   // not subject to the swap
		update_banks();
	}
}

uint8_t nes_waixing_yc030309_device::read_l(offs_t offset)
{
	const offs_t addr = offset + 0x4100;
	if ((addr & 0xff00) == 0x5500)
		return (get_open_bus() & ~0x04) | (m_eeprom->do_read() ? 0x04 : 0x00);
	return get_open_bus();
}

/*-------------------------------------------------

 Waixing Zhan Shi Board

 Games: Wai Xing Zhan Shi

 Simple mapper: writes to 0x8000-0xffff sets prg32 banks to
 (offset>>3)&f. written data&3 sets the mirroring (with
 switched high/low compared to the standard one).

 A crc check is required to support Dragon Quest VIII (which
 uses a slightly different board)

 NESdev (mapper 242): the Waixing RPG variant of this address
 latch board adds 8K of battery-backed WRAM and does not
 implement the UNROM-like banking modes; its UNIF MAPR is
 UNL-43272. The multicarts of this mapper number, which have
 no battery and the full address latch, are nes_bmc_et113_device
 in multigame.cpp.

 iNES: mapper 242

 In MAME: Supported.

 -------------------------------------------------*/

void nes_waixing_wxzs_device::write_h(offs_t offset, uint8_t data)
{
	LOG("waixing_zs write_h, offset: %04x, data: %02x\n", offset, data);

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

 In MAME: Supported.

 -------------------------------------------------*/

void nes_waixing_dq8_device::write_h(offs_t offset, uint8_t data)
{
	LOG("waixing_dq8 write_h, offset: %04x, data: %02x\n", offset, data);

	prg32(offset >> 3);
}


/*-------------------------------------------------

 Waixing WXZS2 / PS2 board

 Games: Wai Xing Zhan Shi 2 (aka Phantasy Star 2),
 Bao Xiao Tien Guo, Bio Hazard, Pokemon Gold, Subor (R),
 Xiao Ao Jiang Hu, Da Hua Xi You, Yue Fei Zhuan, Da Fu Weng 2

 FIXME: This used to be mapper 15. NesDev research says the
 only two games found on board K-1029 (now in multigame.cpp)
 are the two Contra multicarts. All other Waixing games
 appear to be ROM hacks to work with mapper 15. Games on
 this device need to be reassigned to correct device and/or
 replaced with original dumps that match the original boards.
 NESdev lists Da Hua Xi You, Yue Fei Zhuan and Pokemon Gold
 as mapper 164 games (see board FFV). Its mapper 15 page says
 the other known mapper 15 images are mapper hacks, usually of
 mappers 164 and 227 (227 includes the Waixing FW-01 board), and
 that running them needs CHRRAM that is not write protected and
 8K of PRG-RAM at $6000-$7fff.

 -------------------------------------------------*/

void nes_waixing_wxzs2_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t flip = BIT(data, 7);
	uint8_t helper = data << 1;

	LOG("waixing_wxzs2 write_h, offset: %04x, data: %02x\n", offset, data);

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

 Games: A Link to the Past by Waixing, Mummy, Pharaoh,
 Fire Emblem III (Sheng Huo Hui Zhang III), Fire Emblem:
 Genealogy of the Holy War (Huo Yan Wen Zhang), Xi You Ji Hou
 Zhuan

 iNES: mapper 162 (UNIF UNL-FS304 is NES 2.0 mapper 162.0 with
 8K PRG-RAM and 8K CHR-RAM)

 NESdev (mapper 162): 32K switchable PRG-ROM bank, 8K unbanked
 battery-backed PRG-RAM at $6000-$7fff, 8K unbanked CHRRAM and
 hard-wired mirroring. Four registers at $5000/$5100/$5200/$5300
 (mask $ff00), all cleared on reset:

   $5000  C... PPpq   PRG A18..A17 = PP, p and q see $5300,
                      C = automatic 4K CHRRAM switch
   $5100  .... ..p.   PRG A15 if $5300.0=0
   $5200  .... ..PP   PRG A20..A19
   $5300  .... .A.B   A = source of PRG A16, B = source of PRG A15

   A=0: PRG A16 = 1             A=1: PRG A16 = $5000.1
   B=0: PRG A15 = $5100.1       B=1: PRG A15 = 1 if A=0,
                                     $5000.0 if A=1

 Because everything is cleared on reset the board starts in
 32K bank #2 (A16=1, A15=$5100.1=0). The Waixing games run
 with $5300=$04.

 VERIFY: the alternative PRG bank mode ($5300 = $00) has not been
 exercised by any game; the automatic 4K CHRRAM switch ($5000 bit
 7) is used by Liangshan Yingxiong.

 In MAME: Supported.

 -------------------------------------------------*/

// PRG A20..A15 are the 32K bank number. $5300 bit 2 selects the source of A16 and bit 0 the source of A15
void nes_waixing_fs304_device::update_prg()
{
	const bool a16_from_5000 = BIT(m_reg[3], 2);
	const int a16 = a16_from_5000 ? BIT(m_reg[0], 1) : 1;
	int a15;
	if (!BIT(m_reg[3], 0))
		a15 = BIT(m_reg[1], 1);
	else
		a15 = a16_from_5000 ? BIT(m_reg[0], 0) : 1;

	prg32(((m_reg[2] & 0x03) << 4) | (m_reg[0] & 0x0c) | (a16 << 1) | a15);   // $5200: only D1-D0, PRG A20-A19 (NESdev)
}

// NESdev mapper 162, $5000.7: automatic 4 KiB CHR-RAM switch. While set, a pattern-table access (PPU A13=0) uses
// as CHR A12 the PPU A9 latched on the last rise of PPU A13, i.e. from the nametable byte just fetched, so the
// left pattern table serves the top half of every nametable and the right one the bottom half (like mapper 96).
// Attribute fetches keep A13 high and do not update the latch
uint8_t nes_waixing_fs304_device::nt_r(offs_t offset)
{
	if (!m_last_nt)
		m_chr_a12 = BIT(offset, 9);
	m_last_nt = true;
	return nes_nrom_device::nt_r(offset);
}

void nes_waixing_fs304_device::nt_w(offs_t offset, uint8_t data)
{
	if (!m_last_nt)
		m_chr_a12 = BIT(offset, 9);
	m_last_nt = true;
	nes_nrom_device::nt_w(offset, data);
}

uint8_t nes_waixing_fs304_device::chr_r(offs_t offset)
{
	m_last_nt = false;
	if (BIT(m_reg[0], 7) && !m_vram.empty())
		return m_vram[((m_chr_a12 << 12) | (offset & 0x0fff)) & (m_vram.size() - 1)];
	return nes_nrom_device::chr_r(offset);
}

void nes_waixing_fs304_device::chr_w(offs_t offset, uint8_t data)
{
	m_last_nt = false;
	if (BIT(m_reg[0], 7) && !m_vram.empty())
		m_vram[((m_chr_a12 << 12) | (offset & 0x0fff)) & (m_vram.size() - 1)] = data;
	else
		nes_nrom_device::chr_w(offset, data);
}

void nes_waixing_fs304_device::write_l(offs_t offset, uint8_t data)
{
	LOG("fs304 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if ((offset & 0x1c00) == 0x1000)    // $5000-$53ff, mask $ff00
	{
		m_reg[BIT(offset, 8, 2)] = data;
		update_prg();
		chr8(0, CHRRAM);
	}
}
