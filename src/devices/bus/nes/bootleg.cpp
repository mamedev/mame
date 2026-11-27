// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Bootleg PCBs

 iNES mappers: 40, 42, 43, 50, 55, 103, 106, 108, 120, 125, 127, 222, 304, 309, 326, 328, 330, 357, 368,
               415, 416, 417, 418, 522, 535, 538, 539, 548, 557
 (mapper 35 used to be listed here but has no board in this file: it is routed to the J.Y. Company board, see jy.cpp)


 Here we emulate the PCBs used in FDS2NES conversions which are common in the Taiwanese & HK markets
 Notice that many of these have unusual PRG sizes (32KB+8KB, 32KB+24KB) partially mapped in the WRAM
 area, or WRAM overlapping the usual PRG area, so that we often skip the usual bankswitching mechanisms
 in favor of direct handling of the PRG accesses

 TODO:
 - the starting banks (often the main problem of not working games) are only verified where a source states
   them. The registers, banks and IRQs of mappers 40, 42, 43, 50, 55, 103, 106, 108, 120, 125, 127, 304, 309,
   326, 328, 330, 368, 415, 416, 417, 418, 522, 535 and 539 have been compared with NESdev's pages (for 326 with
   the forum analysis its page links, for 330 with krzysiobal's analysis on the NESdev forum, as it has no page)
   and, where one was available, with the FCEUmm, puNES or Nintendulator source and with a dump. Power-up values
   that NESdev leaves unspecified are only verified where the banners say so (the power-up banks of 125 and 368
   agree with the other emulators); the other boards' are not. Mappers 222 (NESdev documents no registers) and 557 (NESdev only names
   the board) are compared with the FCEUmm and puNES sources only (and Mesen's for 222), not with NESdev.
 - SMB2 bootlegs all seem to have timing issues. This is apparent on worlds A-D as the bottom of the
   world letter scrolls. Hardware footage of the mapper 50 version shows the letter bottoms flickering
   (though it could be a video/compression artifact). Not diagnosed: the IRQ registers and the 4096 cycle
   counter of mappers 40, 43 and 50 match NESdev and Nintendulator.

 ***********************************************************************************************************/


#include "emu.h"
#include "bootleg.h"

#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE

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

DEFINE_DEVICE_TYPE(NES_MARIOBABY,      nes_mbaby_device,     "nes_mbaby",     "NES Cart Mario Baby Bootleg PCB")
DEFINE_DEVICE_TYPE(NES_ASN,            nes_asn_device,       "nes_asn",       "NES Cart Ai Senshi Nicol Bootleg PCB")
DEFINE_DEVICE_TYPE(NES_SMB3PIRATE,     nes_smb3p_device,     "nes_smb3p",     "NES Cart Super Mario Bros. 3 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_BTL_CONTRAJ,    nes_btl_cj_device,    "nes_btl_cj",    "NES Cart Contra Japan Pirate PCB")
DEFINE_DEVICE_TYPE(NES_BTL_DDRAGON,    nes_btl_dd_device,    "nes_btl_dd",    "NES Cart Double Dragon II Pirate PCB")
DEFINE_DEVICE_TYPE(NES_BTL_DNINJA,     nes_btl_dn_device,    "nes_btl_dn",    "NES Cart DragonNinja Pirate PCB")
DEFINE_DEVICE_TYPE(NES_SMB2J,          nes_smb2j_device,     "nes_smb2j",     "NES Cart Super Mario Bros. 2 Jpn PCB")
DEFINE_DEVICE_TYPE(NES_SMB2JA,         nes_smb2ja_device,    "nes_smb2ja",    "NES Cart Super Mario Bros. 2 Jpn (Alt) PCB")
DEFINE_DEVICE_TYPE(NES_SMB2JB,         nes_smb2jb_device,    "nes_smb2jb",    "NES Cart Super Mario Bros. 2 Jpn (Alt 2) PCB")
DEFINE_DEVICE_TYPE(NES_N32_4IN1,       nes_n32_4in1_device,  "nes_n32_4in1",  "NES Cart N-32 4 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_0353,           nes_0353_device,      "nes_0353",      "NES Cart 0353 PCB")
DEFINE_DEVICE_TYPE(NES_09034A,         nes_09034a_device,    "nes_09034a",    "NES Cart 09-034A PCB")
DEFINE_DEVICE_TYPE(NES_L001,           nes_l001_device,      "nes_l001",      "NES Cart L-001 PCB")
DEFINE_DEVICE_TYPE(NES_BATMANFS,       nes_batmanfs_device,  "nes_batmanfs",  "NES Cart Batman Pirate PCB")
DEFINE_DEVICE_TYPE(NES_ROBOCOPFS,      nes_robocopfs_device, "nes_robocopfs", "NES Cart RoboCop Pirate PCB")
DEFINE_DEVICE_TYPE(NES_PALTHENA,       nes_palthena_device,  "nes_palthena",  "NES Cart Palthena no Kagami Pirate PCB")
DEFINE_DEVICE_TYPE(NES_TOBIDASE,       nes_tobidase_device,  "nes_tobidase",  "NES Cart Tobidase Daisakusen Pirate PCB")
DEFINE_DEVICE_TYPE(NES_DH08,           nes_dh08_device,      "nes_dh08",      "NES Cart DH-08 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_LE05,           nes_le05_device,      "nes_le05",      "NES Cart LE05 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_LG25,           nes_lg25_device,      "nes_lg25",      "NES Cart LG25 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_LH10,           nes_lh10_device,      "nes_lh10",      "NES Cart LH10 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_LH28_LH54,      nes_lh28_lh54_device, "nes_lh28_lh54", "NES Cart LH28/LH54 Pirate PCBs")
DEFINE_DEVICE_TYPE(NES_LH31,           nes_lh31_device,      "nes_lh31",      "NES Cart LH31 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_LH32,           nes_lh32_device,      "nes_lh32",      "NES Cart LH32 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_LH42,           nes_lh42_device,      "nes_lh42",      "NES Cart LH42 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_LH51,           nes_lh51_device,      "nes_lh51",      "NES Cart LH51 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_LH53,           nes_lh53_device,      "nes_lh53",      "NES Cart LH53 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_2708,           nes_2708_device,      "nes_2708",      "NES Cart BTL-2708 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_AC08,           nes_ac08_device,      "nes_ac08",      "NES Cart AC08 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_MMALEE,         nes_mmalee_device,    "nes_mmalee",    "NES Cart Super Mario Bros. Malee 2 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_RT01,           nes_rt01_device,      "nes_rt01",      "NES Cart RT-01 PCB")
DEFINE_DEVICE_TYPE(NES_YUNG08,         nes_yung08_device,    "nes_yung08",    "NES Cart Super Mario Bros. 2 YUNG-08 PCB")
DEFINE_DEVICE_TYPE(NES_60106416L,      nes_60106416l_device, "nes_60106416l", "NES Cart 60-1064-16L PCB")
DEFINE_DEVICE_TYPE(NES_CTC15,          nes_ctc15_device,     "nes_ctc15",     "NES Cart Co Tung CTC-15 PCB")
DEFINE_DEVICE_TYPE(NES_BITCORP4602,    nes_bitcorp4602_device, "nes_bitcorp4602", "NES Cart Bit Corp 4-in-1 (ID 4602) PCB")



nes_mbaby_device::nes_mbaby_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_irq_count(0), m_irq_enable(0), m_latch(0)
{
}

nes_mbaby_device::nes_mbaby_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_mbaby_device(mconfig, NES_MARIOBABY, tag, owner, clock)
{
}

nes_asn_device::nes_asn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_mbaby_device(mconfig, NES_ASN, tag, owner, clock)
{
}

nes_smb3p_device::nes_smb3p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_SMB3PIRATE, tag, owner, clock), m_irq_count(0), m_irq_enable(0)
{
}

nes_btl_cj_device::nes_btl_cj_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BTL_CONTRAJ, tag, owner, clock)
{
}

nes_btl_dd_device::nes_btl_dd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BTL_DDRAGON, tag, owner, clock), m_irq_count(0), m_irq_enable(0)
{
	std::fill(std::begin(m_prg_reg), std::end(m_prg_reg), 0xf);
	std::fill(std::begin(m_chr_reg), std::end(m_chr_reg), 0);
	std::fill(std::begin(m_mirror), std::end(m_mirror), 0);
}

nes_btl_dn_device::nes_btl_dn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BTL_DNINJA, tag, owner, clock), m_clock_mode(0), m_pending(0), m_counter1(0), m_counter2(0), m_prescaler(0)
{
}

nes_smb2j_device::nes_smb2j_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_SMB2J, tag, owner, clock), m_irq_count(0), m_irq_enable(0)
{
}

nes_smb2ja_device::nes_smb2ja_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_SMB2JA, tag, owner, clock), m_irq_count(0), m_irq_enable(0)
{
}

nes_smb2jb_device::nes_smb2jb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 bank67)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_irq_count(0), m_irq_enable(0), m_reg(0), m_bank67(bank67)
{
}

nes_smb2jb_device::nes_smb2jb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_smb2jb_device(mconfig, NES_SMB2JB, tag, owner, clock, 0x0f)
{
}

nes_n32_4in1_device::nes_n32_4in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_smb2jb_device(mconfig, NES_N32_4IN1, tag, owner, clock, 0x07)
{
}

nes_0353_device::nes_0353_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_0353, tag, owner, clock), m_reg(0)
{
}

nes_09034a_device::nes_09034a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_09034A, tag, owner, clock), m_irq_count(0), m_irq_enable(0), m_reg(0)
{
}

nes_l001_device::nes_l001_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_L001, tag, owner, clock), m_irq_count(0)
{
}

nes_batmanfs_device::nes_batmanfs_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool sub1)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_sub1(sub1), m_irq_count(0), m_irq_enable(0)
{
}

nes_batmanfs_device::nes_batmanfs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_batmanfs_device(mconfig, NES_BATMANFS, tag, owner, clock, false)
{
}

nes_robocopfs_device::nes_robocopfs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_batmanfs_device(mconfig, NES_ROBOCOPFS, tag, owner, clock, true)
{
}

nes_palthena_device::nes_palthena_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_PALTHENA, tag, owner, clock), m_reg(0)
{
}

nes_tobidase_device::nes_tobidase_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_TOBIDASE, tag, owner, clock), m_latch(0)
{
}

nes_whirlwind_device::nes_whirlwind_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_reg(0)
{
}

nes_dh08_device::nes_dh08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_whirlwind_device(mconfig, NES_DH08, tag, owner, clock)
{
}

nes_le05_device::nes_le05_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_whirlwind_device(mconfig, NES_LE05, tag, owner, clock)
{
}

nes_lh28_lh54_device::nes_lh28_lh54_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_whirlwind_device(mconfig, NES_LH28_LH54, tag, owner, clock)
{
}

nes_lh31_device::nes_lh31_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_whirlwind_device(mconfig, NES_LH31, tag, owner, clock)
{
}

nes_lh32_device::nes_lh32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_LH32, tag, owner, clock), m_latch(0)
{
}

nes_lh42_device::nes_lh42_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_LH42, tag, owner, clock), m_latch(0)
{
}

nes_lg25_device::nes_lg25_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_LG25, tag, owner, clock), m_latch(0)
{
}

nes_lh10_device::nes_lh10_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_LH10, tag, owner, clock), m_latch(0)
{
}

nes_lh51_device::nes_lh51_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_LH51, tag, owner, clock)
{
}

nes_lh53_device::nes_lh53_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_LH53, tag, owner, clock), m_irq_count(0), m_irq_enable(0), m_reg(0)
{
}

nes_2708_device::nes_2708_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_2708, tag, owner, clock)
{
	m_reg[0] = 0;
	m_reg[1] = 0;
}

nes_ac08_device::nes_ac08_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_AC08, tag, owner, clock), m_latch(0)
{
}

nes_mmalee_device::nes_mmalee_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_MMALEE, tag, owner, clock)
{
}

nes_rt01_device::nes_rt01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_RT01, tag, owner, clock)
{
}

nes_yung08_device::nes_yung08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_YUNG08, tag, owner, clock), m_irq_count(0), m_irq_latch(0)
{
}




void nes_mbaby_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_latch));
}

void nes_mbaby_device::pcb_reset()
{
	prg32((m_prg_chunks - 1) >> 1);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_latch = 0;
}

void nes_asn_device::pcb_reset()
{
	nes_mbaby_device::pcb_reset();
	chr8(0, m_chr_source);
}

void nes_smb3p_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_smb3p_device::pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted)
{
	device_nes_cart_interface::pcb_start(machine, ciram_ptr, cart_mounted);
	// registers reliably boot up with all 1s according to hardware tests
	for (int i = 0; i < 4; i++)
		prg8_x(i, 0x1f);
}

void nes_smb3p_device::pcb_reset()
{
	// registers not cleared or initialized at reset
}

void nes_batmanfs_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_batmanfs_device::pcb_reset()
{
	prg32((m_prg_chunks >> 1) - 1);    // Last 8K bank is fixed, the rest are swappable
	// all the bank and nametable registers power up as 0 (FCEUmm, puNES)
	prg8_89(0);
	prg8_ab(0);
	prg8_cd(0);
	for (int i = 0; i < 8; i++)
		chr1_x(i, 0, CHRROM);
	for (int i = 0; i < 4; i++)
		set_nt_page(i, CIRAM, 0, 1);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_btl_cj_device::pcb_reset()
{
	prg32((m_prg_chunks >> 1) - 1);    // Last 8K bank is fixed, the rest are swappable
	chr8(0, CHRROM);
}

void nes_btl_dd_device::device_start()
{
	common_start();

	save_item(NAME(m_prg_reg));
	save_item(NAME(m_chr_reg));
	save_item(NAME(m_mirror));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_enable));
}

void nes_btl_dd_device::pcb_reset()
{
	std::fill(std::begin(m_prg_reg), std::end(m_prg_reg), 0xf);
	std::fill(std::begin(m_chr_reg), std::end(m_chr_reg), 0);
	std::fill(std::begin(m_mirror), std::end(m_mirror), 0);
	m_irq_count = 0;
	m_irq_enable = 0;
	update_banks();
}

void nes_btl_dn_device::device_start()
{
	common_start();
	save_item(NAME(m_clock_mode));
	save_item(NAME(m_pending));
	save_item(NAME(m_counter1));
	save_item(NAME(m_counter2));
	save_item(NAME(m_prescaler));
}

void nes_btl_dn_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_clock_mode = 0;
	m_pending = 0;
	m_counter1 = 0;
	m_counter2 = 0;
	m_prescaler = 0;
}

void nes_smb2j_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_smb2j_device::pcb_reset()
{
	prg8_89(1);
	prg8_ab(0);
	prg8_cd(4);    // switchable bank
	prg8_ef(9);
	chr8(0, CHRROM);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_smb2ja_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_smb2ja_device::pcb_reset()
{
	prg8_89(4);
	prg8_ab(5);
	prg8_cd(0);    // switchable bank
	prg8_ef(7);
	chr8(0, CHRROM);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_smb2jb_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_reg));
}

void nes_smb2jb_device::pcb_reset()
{
	prg8_89(0x08);
	prg8_ab(0x09);
	prg8_cd(0);    // switchable bank
	prg8_ef(0x0b);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_reg = 0;
}

void nes_n32_4in1_device::pcb_reset()
{
	// Powers up in menu. NESdev's NES 2.0 Mapper 416 page does not say what a reset does to the data latch, so what a
	// soft reset does here (the latch is cleared) is not verified against hardware.

	m_irq_enable = 0;
	m_irq_count = 0;
	m_reg = 0;
}

void nes_0353_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_0353_device::pcb_reset()
{
	prg32((m_prg_chunks >> 1) - 1);    // fixed 32K bank

	m_reg = 0;
}

void nes_09034a_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_reg));
}

void nes_09034a_device::pcb_reset()
{
	prg32(0);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_reg = 0;
}

void nes_l001_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_count));
}

void nes_l001_device::pcb_reset()
{
	prg32((m_prg_chunks >> 1) - 1);
	chr8(0, CHRROM);

	m_irq_count = 0;
}

void nes_palthena_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_palthena_device::pcb_reset()
{
	prg8_89(0x0c);
	prg8_ab(0);    // switchable bank: the register powers up as 0 (FCEUmm, puNES)
	prg16_cdef(m_prg_chunks - 1);

	m_reg = 0;
}

void nes_tobidase_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_tobidase_device::pcb_reset()
{
	prg32(2);

	m_latch = 0;
}

void nes_whirlwind_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_whirlwind_device::pcb_reset()
{
	prg32((m_prg_chunks >> 1) - 1);      // upper PRG: banks are always fixed
	chr8(0, m_chr_source);

	m_reg = (m_prg_chunks << 1) - 1;     // lower PRG: ProWres needs this fixed, others modify it
}

void nes_lh32_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_lh32_device::pcb_reset()
{
	prg32((m_prg_chunks - 1) >> 1);
	// 0xc000-0xdfff reads/writes WRAM
	set_nt_mirroring(PPU_MIRROR_VERT);
	m_latch = 0;    // Mesen, puNES, FCEUmm and Nintendulator all power the bank register up as 0
}

void nes_lh42_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_lh42_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);    // Last 16K is fixed

	m_latch = 0;
}

void nes_lg25_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_lg25_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);    // Last 16K is fixed

	m_latch = 0;
}

void nes_lh10_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_lh10_device::pcb_reset()
{
	prg8_89(0);
	prg8_ab(0);
	// 0xc000-0xdfff reads/writes WRAM
	prg8_ef((m_prg_chunks << 1) - 1);

	m_latch = 0;
}

void nes_lh51_device::pcb_reset()
{
	prg32((m_prg_chunks >> 1) - 1);    // first 8K is switchable, the rest fixed
	prg8_89(0);                        // the register powers up as 0 (FCEUmm, puNES)
	set_nt_mirroring(PPU_MIRROR_VERT); // and so does the mirroring bit
}

void nes_lh53_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_reg));
	save_item(NAME(m_wram));
}

void nes_lh53_device::pcb_reset()
{
	std::fill(std::begin(m_wram), std::end(m_wram), 0);
	prg8_89(0xc);
	prg8_ab(0xd);   // last 2K are overlaid by WRAM
	prg8_cd(0xe);   // first 6K are overlaid by WRAM
	prg8_ef(0xf);
	m_reg = 0;
	m_irq_count = 0;
	m_irq_enable = 0;
}

void nes_2708_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_2708_device::pcb_reset()
{
	prg32(7);
	// the upper PRG banks never change, but there are 8K of WRAM overlaid to the ROM area based on reg1
	// NESdev (INES Mapper 103): the registers are not cleared on reset, so they keep their values here too
}

void nes_ac08_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_ac08_device::pcb_reset()
{
	prg32(0xff);
	m_latch = 0xff;
}

void nes_rt01_device::pcb_reset()
{
	chr2_0(0, CHRROM);
	chr2_2(0, CHRROM);
	chr2_4(0, CHRROM);
	chr2_6(0, CHRROM);
	prg16_89ab(0);
	prg16_cdef(0);
}

void nes_yung08_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_latch));
}

void nes_yung08_device::pcb_reset()
{
	prg8_89(1);
	prg8_ab(0);
	prg8_cd(4);    // switchable bank: the register powers up as 0, which selects bank 4 (FCEUmm, puNES)
	prg8_ef(8);
	chr8(0, CHRROM);

	m_irq_count = 0;
	m_irq_latch = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 BTL-MARIOBABY

 Games: Mario Baby, Ai Senshi Nicol

 iNES: mapper 42

 In MAME: Supported.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_mbaby_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		// NESdev, INES Mapper 042: a 15-bit counter clocked every CPU cycle, and the IRQ is asserted while its two top bits
		// are set (FCEUX's implementation): not asserted for 24576 cycles, asserted for 8192, and so on. The converted games
		// take this as their frame timer and never acknowledge it, so the line has to fall again when the counter wraps
		m_irq_count = (m_irq_count + 1) & 0x7fff;

		if (m_irq_count == 0x6000)
			set_irq_line(ASSERT_LINE);
		else if (m_irq_count == 0)
			set_irq_line(CLEAR_LINE);
	}
}

void nes_mbaby_device::write_h(offs_t offset, u8 data)
{
	LOG("Mario Baby write_h, offset: %04x, data: %02x\n", offset, data);

	// NESdev: register addresses are masked with $E003
	if (offset >= 0x6000)
	{
		switch (offset & 0x03)
		{
			case 0x00:
				m_latch = data;
				break;
			case 0x01:
				set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;
			case 0x02:
				m_irq_enable = BIT(data, 1);
				if (!m_irq_enable)
				{
					m_irq_count = 0;
					set_irq_line(CLEAR_LINE);
				}
				break;
		}
	}
}

u8 nes_mbaby_device::read_m(offs_t offset)
{
	LOG("Mario Baby read_m, offset: %04x\n", offset);
	// the $6000 bank register is 4 bits wide (NESdev INES Mapper 042); m_latch holds the whole data byte
	return m_prg[(((m_latch & 0x0f) * 0x2000) + (offset & 0x1fff)) % m_prg_size];
}

/*-------------------------------------------------

 BTL-AISENSHINICOL

 Games: Ai Senshi Nicol

 iNES: mapper 42 with CHR-ROM. NESdev's INES Mapper 042 describes this board and the Mario Baby board as one mapper
 with the register addresses masked with $E003: CHR select at $8000 (this board only), PRG at $E000, mirroring at
 $E001 and the IRQ at $E002, so this board is the Mario Baby board plus the CHR select. Nintendulator's mapper 42
 agrees about the mask. The Kaiser dumps in the sheet (Ai Senshi Nicol and v1.1) write the PRG register at both
 $E000 and $F000 and the CHR select at $8000; they do not write the mirroring or IRQ registers in the first 1800
 frames, so those two paths are not exercised by them.

 In MAME: Supported.

 Tested: both Ai Senshi Nicol (Asia) (Ja) (Kaiser) (Pirate) dumps play their intro and a stage.

 -------------------------------------------------*/

void nes_asn_device::write_h(offs_t offset, uint8_t data)
{
	LOG("Ai Senshi Nicol write_h, offset: %04x, data: %02x\n", offset, data);

	// the CHR select is at $8000, masked with $E003; the PRG, mirroring and IRQ registers are the Mario Baby board's
	if ((offset & 0x6003) == 0x0000)
		chr8(data & 0x0f, CHRROM);
	else
		nes_mbaby_device::write_h(offset, data);
}

uint8_t nes_asn_device::read_m(offs_t offset)
{
	LOG("Ai Senshi Nicol read_m, offset: %04x\n", offset);
	// the $6000 bank register is 4 bits wide (NESdev INES Mapper 042, xxxx PPPP); m_latch holds the whole data byte
	return m_prg[(((m_latch & 0x0f) * 0x2000) + (offset & 0x1fff)) & (m_prg_size - 1)];
}

/*-------------------------------------------------

 BTL-SMB3

 Games: Super Mario Bros. 3 Pirate

 iNES: mapper 106

 In MAME: Supported.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_smb3p_device::irq_timer_tick)
{
	// counter does not stop when interrupts are disabled
	if (m_irq_count != 0xffff)
		m_irq_count++;
	if (m_irq_enable && m_irq_count == 0xffff)
		set_irq_line(ASSERT_LINE);
}

void nes_smb3p_device::write_h(offs_t offset, u8 data)
{
	LOG("btl_smb3_w, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x0f)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
			chr1_x(offset & 0x07, (data & 0x7e) | BIT(offset, 0), CHRROM);
			break;
		case 0x04: case 0x05: case 0x06: case 0x07:
			chr1_x(offset & 0x07, data & 0x7f, CHRROM);
			break;
		case 0x08:
		case 0x0b:
			prg8_x(offset & 0x03, (data | 0x10) & 0x1f);
			break;
		case 0x09:
		case 0x0a:
			prg8_x(offset & 0x03, data & 0x1f);
			break;
		case 0x0c:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x0d:
			m_irq_count = 0;
			m_irq_enable = 0;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x0e:
			m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x0f:
			m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
			m_irq_enable = 1;
			break;
	}
}

/*-------------------------------------------------

 BTL-CONTRAJ

 Games: Contra (J) pirate

 NES 2.0: mapper 326

 This PCB has swappable 8K banks at 0x8000-0x9fff,
 0xa000-0xbfff, and 0xc000-0xdfff and associated
 registers in those ranges. Selectable 1K banks for
 both CHRROM and CIRAM are also in registers across
 upper memory, 0x8000-0xffff, with mask 0x8010.

 NESdev (NES 2.0 Mapper 326) points to krzysiobal's analysis of the dumped board (NESdev forum, "Gryzor -
 interesting bootleg"), which this follows: no bus conflicts; PRG registers 0x8000, 0xa000 and 0xc000 with
 mask 0xe010 (0xe000 is fixed to the last bank); CHR 1K registers 0x8010-0x8017 and nametable pages
 0x8018-0x801b (bit 0), both with mask 0x801f, so 0x801c-0x801f are not registers. The ROM also writes to
 mirrors of the registers (for example 0xf018 and 0xc003), which is why the masks matter.

 In MAME: Supported.

 The sheet's dump (Contra (Asia) (Ja) (Pirate)) runs through its whole attract cycle without a crash: the title,
 the intro story, the jungle demo and back to the title, over 3,600 frames. The crash in the intro story that this
 note used to report, with a dump that differs from contraj in only a couple hundred bytes, was not reproduced.

 -------------------------------------------------*/

void nes_btl_cj_device::write_h(offs_t offset, u8 data)
{
	LOG("btl_cj write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 4))
	{
		if (BIT(offset, 3))
		{
			// the nametable registers are 0x8018-0x801b with mask 0x801f: 0x801c-0x801f are not registers
			if (!BIT(offset, 2))
				set_nt_page(offset & 0x03, CIRAM, data & 1, 1);
		}
		else
			chr1_x(offset & 0x07, data, CHRROM);
	}
	else
	{
		offset = BIT(offset, 13, 2);
		if (offset != 3)
			prg8_x(offset, data & 0x0f);
	}
}

/*-------------------------------------------------

 BTL-DDRAGON

 Games: Double Dragon II - The Revenge (pirate)

 NES 2.0: mapper 127

 Four 8K PRG banks, eight 1K CHR banks, a separate CIRAM page for each of the four nametables and an 8 bit IRQ counter
 clocked by every CPU cycle. The registers are in $8000-$FFFF, decoded on A6-A4 and A1-A0 (mask $73):
   $00-$02  8K PRG bank at $8000, $A000 and $C000 (low 4 bits)
   $03      8K PRG bank at $E000 (low 2 bits, with bits 2 and 3 forced to 1)
   $10-$13  1K CHR banks at PPU $0000-$0FFF, $20-$23 at PPU $1000-$1FFF (low 7 bits)
   $30-$33  enable the IRQ (the counter is not touched)
   $40-$43  disable the IRQ, reset the counter to 0 and acknowledge
   $50-$53  CIRAM page (bit 0) of nametable 0-3
 The counter counts down while the IRQ is enabled and asserts the IRQ when it reaches 0; it keeps counting. All PRG
 registers power up as $F.

 There is no NESdev page for this mapper, and FCEUX, FCEUmm and Mesen do not implement it (FCEUX and FCEUmm list it
 commented out). The description is Nintendulator's mapper 127 source (QMT Productions).

 The soft reset of Nintendulator keeps the registers; here they are cleared on every reset.

 In MAME: Supported.

 Tested: Double Dragon II - The Revenge (Asia) (Ja) (Pirate) plays.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_btl_dd_device::irq_timer_tick)
{
	if (m_irq_enable && !--m_irq_count)
		set_irq_line(ASSERT_LINE);
}

void nes_btl_dd_device::update_banks()
{
	for (int i = 0; i < 4; i++)
	{
		prg8_x(i, m_prg_reg[i]);
		set_nt_page(i, CIRAM, m_mirror[i], 1);
	}
	for (int i = 0; i < 8; i++)
		chr1_x(i, m_chr_reg[i], CHRROM);
}

void nes_btl_dd_device::write_h(offs_t offset, u8 data)
{
	LOG("btl_dd write_h, offset: %04x, data: %02x\n", offset, data);

	const u8 reg = offset & 0x73;
	switch (reg & 0x70)
	{
	case 0x00:
		m_prg_reg[reg & 3] = (reg == 0x03) ? ((data & 0x03) | 0x0c) : (data & 0x0f);
		break;
	case 0x10:
		m_chr_reg[reg & 3] = data & 0x7f;
		break;
	case 0x20:
		m_chr_reg[4 + (reg & 3)] = data & 0x7f;
		break;
	case 0x30:
		m_irq_enable = 1;
		return;
	case 0x40:
		m_irq_enable = 0;
		m_irq_count = 0;
		set_irq_line(CLEAR_LINE);
		return;
	case 0x50:
		m_mirror[reg & 3] = data & 1;
		break;
	default:
		return;
	}
	update_banks();
}

/*-------------------------------------------------

 BTL-DRAGONNINJA

 Games: Dragon Ninja (Bootleg), Super Mario Bros. 8

 iNES: mapper 222

 NESdev's INES Mapper 222 page documents no registers and says there is no better information than FCEUX's
 source, so this board cannot be checked against NESdev. The IRQ follows FCEUmm's and puNES's mapper 222 (Mesen has one
 too): a prescaler clocked by every CPU cycle whose bit 6 rising clocks two 4 bit counters; a write to 0xf000 stops the
 clocking, 0xf001 acknowledges the IRQ and, while it is stopped, loads the counters from the low and high nibble of the
 data, 0xf002 starts it, and the IRQ is asserted when both counters have reached 15. It used to be a scanline guess.

 In MAME: Supported.

 Tested: Dragon Ninja (Asia) (Ja) (Pirate) and Super Bros. 8 (Asia) (Ja) (JY211) (Pirate) play.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_btl_dn_device::irq_timer_tick)
{
	const u8 previous = m_prescaler;
	m_prescaler = m_pending ? 0 : m_prescaler + 1;
	if (m_clock_mode && !(previous & 0x40) && (m_prescaler & 0x40))
	{
		if (++m_counter1 == 0x0f && ++m_counter2 == 0x0f)
		{
			m_pending = 1;
			set_irq_line(ASSERT_LINE);
		}
		m_counter1 &= 0x0f;
		m_counter2 &= 0x0f;
	}
}

void nes_btl_dn_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t bank;
	LOG("btl_dn write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x7000)    // 0xf000-0xffff: the IRQ registers
	{
		switch (offset & 0x03)
		{
			case 0:
				m_clock_mode = 0;
				break;
			case 1:
				m_pending = 0;
				set_irq_line(CLEAR_LINE);
				if (!m_clock_mode)
				{
					m_counter1 = data & 0x0f;
					m_counter2 = data >> 4;
				}
				break;
			case 2:
				m_clock_mode = 1;
				break;
		}
		return;
	}

	switch (offset & 0x7003)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x1000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x2000:
			prg8_ab(data);
			break;
		case 0x3000:
		case 0x3002:
		case 0x4000:
		case 0x4002:
		case 0x5000:
		case 0x5002:
		case 0x6000:
		case 0x6002:
			bank = 2 * (BIT(offset, 12, 3) - 3) + BIT(offset, 1);
			chr1_x(bank, data, CHRROM);
			break;
	}
}

/*-------------------------------------------------

 BOOTLEG FC VERSIONS OF FDS GAMES

 -------------------------------------------------*/

/*-------------------------------------------------

 Boards DH-08, LE05, LH28, LH31, LH54 (same as LH28)

 Games: Bubble Bobble alt 1, ProWres, Meikyuu Jiin Dababa,
        Bubble Bobble alt 2, Falsion

 Similar PCB variants for FDS conversions by Whirlwind Manu. They
 map PRG in 0x6000-0x7fff. They vary slightly in the use/nonuse
 of CHR RAM, switchable banks, and valid register locations.

 iNES: mapper 108, 4 submappers

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_whirlwind_device::read_m(offs_t offset)
{
	LOG("whirlwind read_m, offset: %04x\n", offset);
	return m_prg[((m_reg & m_prg_mask) * 0x2000) + offset];
}

void nes_dh08_device::write_h(offs_t offset, u8 data)      // submapper 1
{
	LOG("dh08 write_h, offset: %04x, data: %02x\n", offset, data);
	if (offset >= 0x7000)
		m_reg = data;
}

void nes_le05_device::write_h(offs_t offset, u8 data)      // submapper 4
{
	LOG("le05 write_h, offset: %04x, data: %02x\n", offset, data);
	chr8(data & 1, m_chr_source);
}

void nes_lh28_lh54_device::write_h(offs_t offset, u8 data) // submapper 3
{
	LOG("lh28_lh54 write_h, offset: %04x, data: %02x\n", offset, data);
	m_reg = data;
}

void nes_lh31_device::write_h(offs_t offset, u8 data)      // submapper 2
{
	LOG("lh31 write_h, offset: %04x, data: %02x\n", offset, data);
	if (offset >= 0x6000)
	{
		m_reg = data;
		chr8(data, m_chr_source);
	}
}

/*-------------------------------------------------

 Bootleg Board SMB2J

 Games: Super Mario Bros. 2 Pirates (TONY-I, YS-612)

 iNES: mapper 43

 In MAME: Supported.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_smb2j_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		m_irq_count = (m_irq_count + 1) & 0xfff;
		if (!m_irq_count)
			set_irq_line(ASSERT_LINE);
	}
}

void nes_smb2j_device::update_irq(u8 data)
{
	m_irq_enable = BIT(data, 0);
	if (!m_irq_enable)
	{
		set_irq_line(CLEAR_LINE);
		m_irq_count = 0;
	}
}

void nes_smb2j_device::write_45(offs_t offset, u8 data)
{
	static const u8 bank[8] = {4, 3, 4, 4, 4, 7, 5, 6};

	switch (offset & 0x71ff)
	{
		case 0x4022:
			prg8_cd(bank[data & 0x07]);
			break;
		case 0x4122:
			nes_smb2j_device::update_irq(data);
			break;
	}
}

void nes_smb2j_device::write_ex(offs_t offset, u8 data)
{
	LOG("smb2j write_ex, offset: %04x, data: %02x\n", offset, data);
	nes_smb2j_device::write_45(offset + 0x4020, data);
}

void nes_smb2j_device::write_l(offs_t offset, u8 data)
{
	LOG("smb2j write_l, offset: %04x, data: %02x\n", offset, data);
	nes_smb2j_device::write_45(offset + 0x4100, data);
}

void nes_smb2j_device::write_h(offs_t offset, u8 data)
{
	LOG("smb2j write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset == 0x122)    // $8122 is also IRQ for YS-612, mask unknown
		nes_smb2j_device::update_irq(data);
}

u8 nes_smb2j_device::read_l(offs_t offset)
{
	LOG("smb2j read_l, offset: %04x\n", offset);
	offset += 0x100;

	if (offset >= 0x1000)
		return m_prg[0x10000 + (offset & 0x0fff)];

	return get_open_bus();
}

u8 nes_smb2j_device::read_m(offs_t offset)
{
	LOG("smb2j read_m, offset: %04x\n", offset);
	return m_prg[0x4000 + offset];
}

/*-------------------------------------------------

 BTL-SMB2JA

 Games: Super Mario Bros. 2 Pirates (LF36, Hey Sung, 1990 SMB4)

 iNES: mapper 40

 In MAME: Supported.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_smb2ja_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		m_irq_count = (m_irq_count + 1) & 0x1fff;    // 13-bit counter
		if (BIT(m_irq_count, 12))
			set_irq_line(ASSERT_LINE);
		else if (!m_irq_count)
			set_irq_line(CLEAR_LINE);            // CD4020 acknowledges on overflow
	}
}

void nes_smb2ja_device::write_h(offs_t offset, u8 data)
{
	LOG("smb2ja write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6000)
	{
		case 0x0000:
			m_irq_enable = 0;
			m_irq_count = 0;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x2000:
			m_irq_enable = 1;
			break;
		case 0x6000:
			prg8_cd(data);
			break;
	}
}

u8 nes_smb2ja_device::read_m(offs_t offset)
{
	LOG("smb2ja read_m, offset: %04x\n", offset);
	return m_prg[0x06 * 0x2000 + offset];    // fixed 8K bank
}

/*-------------------------------------------------

 BTL-SMB2JB (PCB 761214)

 Games: Super Mario Bros. 2 Pirate (N-32)

 NESdev's INES Mapper 050 page gives Super Mario Bros. (JU) (Alt Levels) (SMB2j pirate cart) as its example game:
 it is the Japanese Super Mario Bros. 2, whose levels are the alternate ones.

 iNES: mapper 50

 In MAME: Supported.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_smb2jb_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		if (BIT(++m_irq_count, 12))
			set_irq_line(ASSERT_LINE);
	}
}

void nes_smb2jb_device::write_45(offs_t offset, u8 data)
{
	switch (offset & 0x4120)
	{
		case 0x4020:
			m_reg = bitswap<4>(data, 3, 0, 2, 1);
			prg8_cd(m_reg);
			break;
		case 0x4120:
			m_irq_enable = BIT(data, 0);
			if (!m_irq_enable)
			{
				set_irq_line(CLEAR_LINE);
				m_irq_count = 0;
			}
			break;
	}
}

void nes_smb2jb_device::write_ex(offs_t offset, u8 data)
{
	LOG("smb2jb write_ex, offset: %04x, data: %02x\n", offset, data);
	write_45(offset + 0x4020, data);
}

void nes_smb2jb_device::write_l(offs_t offset, u8 data)
{
	LOG("smb2jb write_l, offset: %04x, data: %02x\n", offset, data);
	write_45(offset + 0x4100, data);
}

u8 nes_smb2jb_device::read_m(offs_t offset)
{
	LOG("smb2jb read_m, offset: %04x\n", offset);
	return m_prg[(m_bank67 * 0x2000 + offset) & (m_prg_size - 1)];
}

/*-------------------------------------------------

 BMC-N32-4IN1

 Unknown Bootleg Multigame Board
 Games: 4 in 1

 This multicart contains the mapper 50 version of SMB2.
 There are changes to bank locations compared to that
 game, but otherwise we rely on the existing SMB2JB
 emulation (unless other differences are discovered?)

 NES 2.0: mapper 416

 Checked against NESdev (NES 2.0 Mapper 416) and FCEUmm's mapper 416: 0x6000 is fixed to bank 7, the data latch's
 Normal mode banking (NROM-064, NROM-128 and NROM-256 rules, the PRG A16 mode bit, CHR A14-A13 and mirroring) matches
 both, and the IRQ (0x4120) is a 12 bit counter that fires on overflow.
 The SMB2J PRG register (0x4020) keeps the fourth bit of mapper 50's register: NESdev and FCEUmm describe it as three
 bits ([.... .BAC]), puNES and this code use four. The 4-in-1 HQ dump settles it: its code writes $0E to the register,
 reads 0xc52e and compares it with 0xb8, and only the four bit decode (8K bank 11) finds 0xb8 there, the three bit
 decode (bank 3) finds 0x80. NESdev says the latch is written at $8000 with mask $8000, FCEUmm latches only at exactly
 $8000, puNES and this code at $8000-$9fff. The dump plays both of its games (the SMB2J one and the second menu entry).
 The dump rules out NESdev's mask: its SMB2J game keeps writing $02 to $B522-$B524 (over 8000 times each in 3000 frames)
 and other values to $B517 and $C65E-$C683, left over from the FDS original where $6000-$DFFF is RAM. With the latch
 answering there, those writes select CHR bank 1 and World 1-1 draws the wrong tiles (rocky ground, mushroom trees), so
 the latch does not answer at $A000 and above. Whether it answers in all of $8000-$9FFF or only at $8000 is still open
 (the dump only writes $00, to $8000).

 In MAME: Supported.

 -------------------------------------------------*/

void nes_n32_4in1_device::write_h(offs_t offset, u8 data)
{
	LOG("n32_4in1 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset < 0x2000)
	{
		if (BIT(data, 3))    // NROM games
		{
			u8 bank = bitswap<3>(data, 3, 7, 5);
			u8 mode = BIT(data, 7);
			if (data & 0xc0)
			{
				prg16_89ab(bank & ~mode);
				prg16_cdef(bank | mode);
			}
			else
				for (int i = 0; i < 4; i++)
					prg8_x(i, bank << 1);
		}
		else                 // SMB2 only
		{
			prg8_89(0);
			prg8_ab(1);
			prg8_cd(m_reg);
			prg8_ef(3);
		}

		chr8(BIT(data, 1, 2), CHRROM);
		set_nt_mirroring(BIT(data, 2) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}
}

/*-------------------------------------------------

 (BTL-)0353

 Games: Lucky Rabbit (Roger Rabbit conversion)

 NES 2.0: mapper 415

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_0353_device::read_m(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "0353 read_m, offset: %04x\n", offset);
	return m_prg[m_reg * 0x2000 + offset];
}

void nes_0353_device::write_h(offs_t offset, u8 data)
{
	LOG("0353 write_h, offset: %04x, data: %02x\n", offset, data);

	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_reg = data & 0x0f;
}

/*-------------------------------------------------

 (UNL-)09-034A

 Games: Zanac FDS conversion with two PRG chips and
 no CHRROM, and SMB2 and Volleyball FDS conversions
 with two PRG chips and CHRROM. Originally dumps
 were marked as UNL-SMB2J PCB.

 Only SMB2 uses the IRQ and it has been documented as
 being broken on real hardware. Most notably the status
 bar scrolls with the rest of the screen and the game
 completely crashes between the "our princess" scene of
 world 4-4 and the beginning of world 5-1. NESdev (NES 2.0
 Mapper 304) takes the IRQ from FCEUX: the counter increases
 on every M2 cycle while enabled and fires at 5750, which
 FCEUX's source calls "completely by guess" (it works with the
 LE10 image); a hardware recording suggests the real board has
 no working counter. A write to $4068 acknowledges the IRQ and
 resets the counter. With a 12-bit counter (4096 cycles) the status
 bar of SMB2 (09-034A PCB dump) was displaced and garbled in the
 first level; with 5750 it stays in place.

 NES 2.0: mapper 304

 In MAME: Supported.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_09034a_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		if (++m_irq_count >= 5750)
		{
			m_irq_enable = 0;
			set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_09034a_device::write_ex(offs_t offset, u8 data)
{
	LOG("09-034a write_ex, offset: %04x, data: %02x\n", offset, data);

	offset += 0x20;
	switch (offset)
	{
		case 0x0027:
			m_reg = data & 1;
			break;
		case 0x0068:
			m_irq_enable = BIT(data, 0);
			m_irq_count = 0;
			set_irq_line(CLEAR_LINE);
			break;
	}
}

u8 nes_09034a_device::read_ex(offs_t offset)
{
	LOG("09-034a read_ex, offset: %04x, data: %02x\n", offset);

	offset += 0x20;
	// SMB2 does not boot with the default open bus reads in this range
	if (offset >= 0x42 && offset <= 0x55)
		return 0xff;
	else
		return get_open_bus();
}

u8 nes_09034a_device::read_m(offs_t offset)
{
	LOG("09-034a read_m, offset: %04x\n", offset);
	// in 0x6000-0x7fff is mapped the 2nd PRG chip which starts after 32K (hence the +4)
	return m_prg[(((m_reg + 4) * 0x2000) + offset) & (m_prg_size - 1)];
}

/*-------------------------------------------------

 Board L-001

 Games: Sangokushi III (Sangokushi II bootleg)

 This board has swappable 8K PRG banks at 0x8000, 0xa000,
 and 0xc000, while 0xe000 is fixed to the final bank.
 CHRROM and CIRAM are also swappable in 1K banks.
 The board has a 16-bit IRQ counter with the enable bit
 acting as the MSB. The enhanced audio of the original
 Namco 163 board is not retained.

 The register layout is krzysiobal's reverse engineering of the board (NESdev forum, "Sangokushi II - Haou no
 Tairiku hack - N163 bootleg"): CHR 1K banks at 0x8000-0xb800 (mask 0xfc00), nametable pages at 0xc000-0xd800,
 PRG banks at 0xe000, 0xe800 and 0xf000 (5 bits), IRQ low at 0x8400 and high at 0xa400 (mask 0xe400) with a
 15 bit counter that counts while the enable bit is set, stops when it wraps and is acknowledged by reloading
 the high byte. The code matches it; I did not find a NESdev page for this mapper to compare with.

 NES 2.0: mapper 330

 In MAME: Supported.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_l001_device::irq_timer_tick)
{
	if (BIT(m_irq_count, 15))
	{
		if (++m_irq_count == 0)
			set_irq_line(ASSERT_LINE);
	}
}

void nes_l001_device::write_h(offs_t offset, u8 data)
{
	LOG("l-001 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6400)
	{
		case 0x0000:
		case 0x2000:
			chr1_x(BIT(offset, 11, 3), data, CHRROM);
			break;
		case 0x0400:
			m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x2400:
			m_irq_count = (m_irq_count & 0x00ff) | data << 8;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x4000:
			set_nt_page(BIT(offset, 11, 2), CIRAM, data & 1, 1);
			break;
		case 0x6000:
			if (offset < 0x7800)
				prg8_x(BIT(offset, 11, 2), data & 0x1f);
			break;
	}
}

/*-------------------------------------------------

 BTL-BATMANFS

 Games: Batman "Fine Studio" pirate (submapper 0), RoboCop "Fine Studio" pirate (submapper 1)

 Checked against NESdev (NES 2.0 Mapper 417): PRG registers 0x8000-0x8002, CHR registers 0x8010-0x8013 and
 0x8020-0x8023, the IRQ at 0x8030 (reset and enable) and 0x8040 (disable and acknowledge), all with mask 0x8073
 (0x8070 for the IRQ), the counter that runs on every M2 cycle whether the IRQ is enabled or not and fires on a
 10 bit overflow (12 bit for submapper 1). Submapper 0 selects the nametable pages at 0x8050-0x8053; submapper 1
 takes them from bit 7 of the CHR writes at 0x8010-0x8013 and has no 0x8050 register.

 NES 2.0: mapper 417

 In MAME: Supported.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_batmanfs_device::irq_timer_tick)
{
	// the counter does not stop when interrupts are disabled: 10 bits, 12 bits in submapper 1 (NESdev, NES 2.0 Mapper 417)
	m_irq_count = (m_irq_count + 1) & (m_sub1 ? 0xfff : 0x3ff);
	if (m_irq_enable && !m_irq_count)
		set_irq_line(ASSERT_LINE);
}

void nes_batmanfs_device::write_h(offs_t offset, u8 data)
{
	LOG("batmanfs write_h, offset: %04x, data: %02x\n", offset, data);
	switch (offset & 0x70)
	{
		case 0x00:
			if ((offset & 0x03) != 0x03)
				prg8_x(offset & 0x03, data & 0x0f);
			break;
		case 0x10:
		case 0x20:
			chr1_x((offset & 0x03) + 4 * BIT(offset, 5), data, CHRROM);
			// submapper 1: bit 7 of the writes at 0x8010-0x8013 selects the CIRAM page of the nametable
			if (m_sub1 && !BIT(offset, 5))
				set_nt_page(offset & 0x03, CIRAM, BIT(data, 7), 1);
			break;
		case 0x30:
			m_irq_enable = 1;
			m_irq_count = 0;
			break;
		case 0x40:
			m_irq_enable = 0;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x50:
			if (!m_sub1)
				set_nt_page(offset & 0x03, CIRAM, data & 1, 1);
			break;
	}
}

/*-------------------------------------------------

 BTL-PALTHENA

 Games: Palthena no Kagami (FDS conversion)

 Checked against NESdev (NES 2.0 Mapper 539, a conversion of the FDS Kid Icarus): the fixed banks 0xd, 0xc, 0xe and
 0xf, the PRG register at 0xa000 (4 bits), the mirroring register at 0xffff (bit 3, 1 = horizontal), the CHR-RAM
 and the six RAM overlay ranges below all match; NESdev gives both register masks as unknown.

 This board has fixed 8K PRG banks at 0x6000, 0x8000,
 0xc000, and 0xe000. The PRG bank at 0xa000 is switchable
 by writing to the register in the same 0xa000-0xbfff
 range. What makes the board interesting is the overlaid
 8K of RAM with only 6K addressable:

   8K WRAM:           Addr:
     0x0000-0x11ff      0xc000-0xd1ff
     0x1200-0x12ff      0x8200-0x82ff
     0x1300-0x17ff           N/A
     0x1800-0x18ff      0x6000-0x60ff
     0x1900-0x19ff           N/A
     0x1a00-0x1aff      0x6200-0x62ff
     0x1b00-0x1bff           N/A
     0x1c00-0x1dff      0x6400-0x65ff
     0x1e00-0x1eff           N/A
     0x1f00-0x1fff      0xdf00-0xdfff

 NES 2.0: mapper 539

 In MAME: Supported.

-------------------------------------------------*/

u8 nes_palthena_device::read_m(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "palthena read_m, offset: %04x\n", offset);
	switch (offset & 0x1f00)
	{
		case 0x0000:
		case 0x0200:
		case 0x0400:
		case 0x0500:
			return m_prgram[offset | 0x1800];
		default:
			return m_prg[0x0d * 0x2000 + offset];    // fixed PRG bank
	}
}

void nes_palthena_device::write_m(offs_t offset, u8 data)
{
	LOG("palthena write_m, offset: %04x, data: %02x\n", offset, data);
	switch (offset & 0x1f00)
	{
		case 0x0000:
		case 0x0200:
		case 0x0400:
		case 0x0500:
			m_prgram[offset | 0x1800] = data;
			break;
	}
}

u8 nes_palthena_device::read_h(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "palthena read_h, offset: %04x\n", offset);
	u8 page = offset >> 8;
	if ((page >= 0x40 && page < 0x52) || page == 0x5f)
		return m_prgram[offset & 0x1fff];
	else if (page == 0x02)
		return m_prgram[offset | 0x1000];

	return hi_access_rom(offset);
}

void nes_palthena_device::write_h(offs_t offset, u8 data)
{
	LOG("palthena write_h, offset: %04x, data: %02x\n", offset, data);
	u8 page = offset >> 8;
	if ((page >= 0x40 && page < 0x52) || page == 0x5f)
		m_prgram[offset & 0x1fff] = data;
	else if (page == 0x02)
		m_prgram[offset | 0x1000] = data;
	else if (page >= 0x20 && page < 0x40)
		prg8_ab(data & 0x0f);
	else if (offset == 0x7fff)
		set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Bootleg Board used for FDS conversion

 Games: Tobidase Daisakusen (FDS conversion)

 This PCB maps PRG in 0x6000-0x7fff

 iNES: mapper 120

 NESdev's INES Mapper 120 page: a 3 bit PRG register ([.... .PPP], address mask $E100), which is what this
 does (it used to take four bits, remap the values 12-15 to 8-11 and decode only $41C0-$41FF). Tobidase writes only
 $41FF: 0, then 5, 6 and 7, then 1 to 8 in turn, then 0; while the register holds 8 the game never reads the
 $6000-$7FFF window (about 1,280 reads under each of the values 1 to 7), so the old and new decodes show identical
 pictures over 2400 frames.

 In MAME: Supported.

 Tested: Tobidase Daisakusen (Asia) (Ja) (Pirate) plays its 3D runner stage.

 -------------------------------------------------*/

void nes_tobidase_device::write_l(offs_t offset, uint8_t data)
{
	LOG("tobidase write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x4100;

	if ((offset & 0xe100) == 0x4100)    // NESdev: address mask $E100
		m_latch = data & 0x07;
}

uint8_t nes_tobidase_device::read_m(offs_t offset)
{
	LOG("tobidase read_m, offset: %04x\n", offset);
	return m_prg[(m_latch * 0x2000) + (offset & 0x1fff)];
}

/*-------------------------------------------------

 UNL-LH32

 Games: Monty no Doki Doki Daidassou (FDS conversion)

 This PCB maps WRAM in 0xc000-0xdfff and PRG in 0x6000-0x7fff

 iNES: mapper 125

 NESdev (INES Mapper 125) and Nintendulator's mapper 125 agree: the PRG register is decoded across the whole of
 0x6000-0x7fff (NESdev: mask probably 0xe000) and the mirroring is hard-wired to vertical, whatever the header says.
 0x8000, 0xa000 and 0xe000 are PRG banks 12, 13 and 15. The bank register powers up as 0 in Mesen, puNES, FCEUmm and Nintendulator,
 and here (it used to be 0xf).

 In MAME: Supported.

 Tested: Monty no Doki Doki Daisassou (Asia) (Ja) (Pirate), whose only writes are to 0x6000.

-------------------------------------------------*/

uint8_t nes_lh32_device::read_m(offs_t offset)
{
	LOG("lh32 read_m, offset: %04x\n", offset);
	return m_prg[(m_latch * 0x2000) + (offset & 0x1fff)];
}

uint8_t nes_lh32_device::read_h(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "lh32 read_h, offset: %04x\n", offset);

	if (offset >= 0x4000 && offset < 0x6000)
		return m_prgram[offset & 0x1fff];

	return hi_access_rom(offset);
}

void nes_lh32_device::write_m(offs_t offset, uint8_t data)
{
	LOG("lh32 write_m, offset: %04x, data: %02x\n", offset, data);

	m_latch = data & 0xf;
}

void nes_lh32_device::write_h(offs_t offset, uint8_t data)
{
	LOG("lh32 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x4000 && offset < 0x6000)
		m_prgram[offset & 0x1fff] = data;
}

/*-------------------------------------------------

 UNL-LH42

 Games: Highway Star (Whirlwind Manu bootleg)

 NES 2.0: mapper 418 (NESdev: the 820106-C/821007C boards of the LH42 Highway Star, with a schematic on the forum;
 the page has no register list). FCEUmm's mapper 418 is the same board: an MMC3-like select (even) and data (odd)
 pair, registers 6 and 7 as the PRG banks with a 0x0f mask, 8K of unbanked CHR and the mirroring from register 5 bit 0
 (1 = horizontal), which is what this code does.

 In MAME: Supported.

 -------------------------------------------------*/

void nes_lh42_device::write_h(offs_t offset, u8 data)
{
	LOG("lh42 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x0000:
			m_latch = data & 0x07;
			break;
		case 0x0001:
			switch (m_latch)
			{
				case 5:
					set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
					break;
				case 6:
				case 7:
					prg8_x(m_latch & 1, data & 0x0f);
					break;
			}
			break;
	}
}

/*-------------------------------------------------

 UNL-LG25

 Games: Moero TwinBee Cinnamon Hakase o Sukue! (FDS conversion)

 In addition to the two swappable 8K PRG banks at
 0x8000 and 0xa000, this board has 8K WRAM at 0x6000
 (the loader gives it 8K whatever the header says).
 NESdev only names the board (NTDEC 2718). Compared with FCEUmm's and puNES's mapper 557: the select (even) and data
 (odd) register pair at 0x8000-0x9fff, registers 6 and 7 as the PRG banks (0x0f mask), the mirroring from register 5
 bit 5 (1 = horizontal), the fixed upper PRG, 8K of CHR and the 8K of WRAM match.

 NES 2.0: mapper 557

 In MAME: Supported.

 -------------------------------------------------*/

void nes_lg25_device::write_h(offs_t offset, u8 data)
{
	LOG("lg25 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x0000:
			m_latch = data & 0x07;
			break;
		case 0x0001:
			switch (m_latch)
			{
				case 5:
					set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
					break;
				case 6:
				case 7:
					prg8_x(m_latch & 1, data & 0x0f);
					break;
			}
			break;
	}
}

/*-------------------------------------------------

 UNL-LH10

 Games: Fuuun Shaolin Kyo (FDS conversion)

 This PCB maps WRAM in 0xc000-0xdfff and PRG in 0x6000-0x7fff
 This is very similar to KS7037 (see kaiser.cpp)
 Checked against NESdev (NES 2.0 Mapper 522): 0x6000 is hard-wired to the second-to-last bank (bank 14 here, which
 is right for the 128K image), 0x8000 and 0xa000 are switchable through an MMC3-like select (even) and data
 (odd) register pair, 0xc000 is 8K of RAM and 0xe000 the last bank; mirroring is hard-wired.

 NES 2.0: mapper 522

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_lh10_device::read_m(offs_t offset)
{
	LOG("lh10 read_m, offset: %04x\n", offset);
	return m_prg[(0x0e * 0x2000 + offset) & (m_prg_size - 1)];
}

u8 nes_lh10_device::read_h(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "lh10 read_h, offset: %04x\n", offset);

	if (offset >= 0x4000 && offset < 0x6000)
		return m_prgram[offset & 0x1fff];

	return hi_access_rom(offset);
}

void nes_lh10_device::write_h(offs_t offset, u8 data)
{
	LOG("lh10 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x0000:
			m_latch = data & 7;
			break;
		case 0x0001:
			switch (m_latch)
			{
				case 6:
				case 7:
					prg8_x(m_latch & 1, data & 0x0f);
					break;
			}
			break;
		case 0x4000:
		case 0x4001:
			m_prgram[offset & 0x1fff] = data;
			break;
	}
}

/*-------------------------------------------------

 UNL-LH51

 Games: Ai Senshi Nicol (Whirlwind Manu FDS conversion)

 A simple board with swappable 8K PRG at 0x8000-0x9fff,
 fixed PRG above that, and 8K WRAM at 0x6000-0x7fff.
 The game's sound code is broken and does not work on
 real hardware.

 NES 2.0: mapper 309

 The registers power up as 0 (bank 0 at 0x8000, vertical mirroring) in FCEUmm and puNES, and here.

 Checked against NESdev (NES 2.0 Mapper 309): 8K PRG-RAM at 0x6000, PRG register at 0x8000-0x9fff (4 bits),
 fixed banks 13, 14 and 15 above it, mirroring at 0xe000-0xffff bit 3 (1 = horizontal), CHR-RAM; NESdev gives
 both register masks as "probably 0xe000". The 8K of WRAM is part of the board, so the loader gives it 8K whatever
 the header says.

 In MAME: Supported.

 -------------------------------------------------*/

void nes_lh51_device::write_h(offs_t offset, u8 data)
{
	LOG("lh51 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6000)
	{
		case 0x0000:
			prg8_89(data & 0x0f);
			break;
		case 0x6000:
			set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*-------------------------------------------------

 UNL-LH53

 Games: Nazo no Murasamejou (FDS conversion)

 This PCB maps 8K of WRAM in 0xb800-0xd7ff (the board's own, so it does not depend on the header) and PRG in
 0x6000-0x7fff

 NES 2.0: mapper 535

 In MAME: Preliminary.

 Tested: Nazo no Murasamejou (Asia) (Ja) (Pirate) reaches its name screens and, with no name chosen, the castle
 screen before the first stage, where it stays (4,000 frames of input) and its copyright line reads "HLHUSHPJ"
 instead of "NINTENDO". The cause is not known.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_lh53_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		// NESdev NES 2.0 Mapper 535: the IRQ is raised when the counter, cleared by the $E000 write, reaches 7560
		if (++m_irq_count == 7560)
		{
			m_irq_count = 0;
			set_irq_line(ASSERT_LINE);
		}
	}
}

uint8_t nes_lh53_device::read_m(offs_t offset)
{
	LOG("lh53 read_m, offset: %04x\n", offset);
	return m_prg[m_reg * 0x2000 + offset];
}

uint8_t nes_lh53_device::read_h(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "lh53 read_h, offset: %04x\n", offset);

	if (offset >= 0x3800 && offset < 0x5800)
		return m_wram[offset - 0x3800];

	return hi_access_rom(offset);
}

void nes_lh53_device::write_h(offs_t offset, uint8_t data)
{
	LOG("lh53 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x3800 && offset < 0x5800)
		m_wram[offset - 0x3800] = data;
	else
	{
		switch (offset & 0x7000)
		{
			case 0x6000:
				// NESdev: a write acknowledges a pending IRQ and resets the counter, whether it enables counting or not
				m_irq_enable = BIT(data, 1);
				m_irq_count = 0;
				set_irq_line(CLEAR_LINE);
				break;
			case 0x7000:
				m_reg = data & 0x0f;
				break;
		}
	}
}

/*-------------------------------------------------

 BTL-2708

 Games: Doki Doki Panic (FDS conversion)

 This board has a very unique setup, with 8KB of WRAM
 in 0x6000-0x7fff and other 8KB of WRAM in 0xb800-0xd7ff
 which can be switched in 2KB chunks (we attempt to
 emulate this by intercepting reads in that area before
 they get to the PRG banks...)

 iNES: mapper 103

 Checked against NESdev (INES Mapper 103): the PRG bank at 0x6000 (0x8000-0x8fff, 4 bits), the mirroring (0xe000-0xefff,
 bit 3, 1 = horizontal), the RAM disable bit (0xf000-0xffff, bit 4) with mask 0xf000, the 0x8000-0xb7ff ROM, the RAM
 at 0xb800-0xd7ff and the rule that writes to the RAM areas always reach the RAM all match. NESdev says the registers
 are not cleared on reset and their power-up state is unknown; here they power up as 0 and keep their values on a reset.

 In MAME: Supported.

 -------------------------------------------------*/

uint8_t nes_2708_device::read_m(offs_t offset)
{
	LOG("btl-2708 read_m, offset: %04x\n", offset);
	if (!m_reg[1])
		return m_prgram[offset];    // lower 8K of WRAM
	else
		return m_prg[(m_reg[0] * 0x2000) + (offset & 0x1fff)];
}

void nes_2708_device::write_m(offs_t offset, uint8_t data)
{
	LOG("btl-2708 write_m, offset: %04x, data: %02x\n", offset, data);
//  if (!m_reg[1])
		m_prgram[offset] = data;    // lower 8K of WRAM
}

uint8_t nes_2708_device::read_h(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "btl-2708 read_h, offset: %04x\n", offset);

	if (offset >= 0x3800 && offset < 0x5800 && !m_reg[1])
		return m_prgram[0x2000 + ((offset - 0x3800) & 0x1fff)]; // higher 8K of WRAM

	return hi_access_rom(offset);
}

void nes_2708_device::write_h(offs_t offset, uint8_t data)
{
	LOG("btl-2708 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x3800 && offset < 0x5800/* && !m_reg[1]*/)
		m_prgram[0x2000 + ((offset - 0x3800) & 0x1fff)] = data; // higher 8K of WRAM

	switch (offset & 0x7000)
	{
		case 0x0000:
			m_reg[0] = data & 0x0f; // PRG bank in 0x6000-0x7fff
			break;
		case 0x6000:
			set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x7000:
			m_reg[1] = BIT(data, 4);    // bit4 enables the two WRAM banks
			break;
	}
}

/*-------------------------------------------------

 UNL-AC08

 Games: Green Beret (FDS conversions)

 This board has two PRG chips. The first (128K) is
 connected to 0x6000-0x7fff and switches among the
 16x8K banks; the second (32K) is fixed in 0x8000-0xffff

 NES 2.0: mapper 42 submapper 2 (UNIF UNL-AC08), the 160K Green Beret images; $8001 writes give the bank in
 bits 1-4, other writes in bits 0-3, and $4025 bit 3 is the mirroring

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ac08_device::write_ex(offs_t offset, uint8_t data)
{
	LOG("AC-08 write_ex, offset: %04x, data: %02x\n", offset, data);
	if (offset == 5)    // $4025
		set_nt_mirroring(!BIT(data, 3) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
}

uint8_t nes_ac08_device::read_m(offs_t offset)
{
	LOG("AC-08 read_m, offset: %04x\n", offset);
	// write_h stores 4 bits; the power-on value $ff is masked the same way, and the image (160K) is not a power of two
	return m_prg[(((m_latch & 0x0f) * 0x2000) + (offset & 0x1fff)) % m_prg_size];
}

void nes_ac08_device::write_h(offs_t offset, uint8_t data)
{
	LOG("AC-08 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset == 1)
		m_latch = BIT(data, 1, 4);
	else
		m_latch = data & 0x0f;  // apparently there also is a Castlevania FDS conversion using same board with different banking lines
}

/*-------------------------------------------------

 BTL-MARIO1-MALEE2 (aka Genius Merio Bros)

 Games: Super Mario Bros Malee 2

 This PCB has two PRG chips (32K+2K) + one CHR chip (8K)
 + 2KB of WRAM
 NESdev (INES Mapper 055): the second PRG chip (2K) is at 0x6000-0x6fff, mirrored once, while the WRAM is at
 0x7000-0x7fff, mirrored once. Nintendulator agrees. The image holds the 32K first and then the 2K chip, which
 starts at 0x8000. The WRAM is part of the board, so the loader gives it 2K whatever the header says.

 iNES: mapper 55

 In MAME: Supported.

 No dump of this board is held for the audit: the two mirrors were checked with a synthetic
 image only, not with a game.

 -------------------------------------------------*/

uint8_t nes_mmalee_device::read_m(offs_t offset)
{
	LOG("mmalee read_m, offset: %04x\n", offset);

	if (offset < 0x1000)    // 2K of PRG-ROM, mirrored once
	{
		if ((0x8000 + (offset & 0x7ff)) < m_prg_size)
			return m_prg[0x8000 + (offset & 0x7ff)];
	}
	else if (!m_prgram.empty())   // 2K of WRAM, mirrored once
		return m_prgram[offset & 0x7ff];

	return ((offset + 0x6000) & 0xff00) >> 8;
}

void nes_mmalee_device::write_m(offs_t offset, uint8_t data)
{
	LOG("mmalee write_m, offset: %04x, data: %02x\n", offset, data);

	if (!m_prgram.empty() && offset >= 0x1000)    // 2K of WRAM, mirrored once
		m_prgram[offset & 0x7ff] = data;
}

/*-------------------------------------------------

 RT-01

 Games: Russian test cart

 The PRG EPROM has copy protected areas with
 "weak bits", which is tested at some points (info
 from Cah4e3).

 NES 2.0: mapper 328

 NESdev (NES 2.0 Mapper 328): the reads at 0xce80-0xceff and 0xfe80-0xfeff return 1111 RR1R, the random bits
 simulating the weak bits of the protected EPROM areas (the program checksums each area and loops until the
 checksum changes); NESdev gives the mask as unknown (0xbfff if CaH4e3's description is accurate). There is only
 16K of PRG and 2K of CHR and the mirroring is hard-wired. The code does exactly this.

 In MAME: Supported.

 -------------------------------------------------*/

uint8_t nes_rt01_device::read_h(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "rt01 read_h, offset: %04x\n", offset);

	if ((offset >= 0x4e80) && (offset < 0x4f00))
		return 0xf2 | (machine().rand() & 0x0d);
	if ((offset >= 0x7e80) && (offset < 0x7f00))
		return 0xf2 | (machine().rand() & 0x0d);

	return hi_access_rom(offset);
}

/*-------------------------------------------------

 YUNG-08

 Games: Super Mario Bros. 2 Pirate (YUNG-08)

 NES 2.0: mapper 368

 Checked against NESdev (NES 2.0 Mapper 368): the fixed banks 2, 1, 0 and 8, the 0x4022 bank table
 (4,3,5,3,6,3,7,3) and mask 0xf1ff, the 0x4122 latch write (bits 5, 4, 2 and 0 are read back) and read
 (10LL 1L1L), and the 12 bit IRQ counter that fires on overflow all match. NESdev does not say what the
 0xc000 register powers up as; FCEUmm and puNES power it up as 0, which the table above turns into bank 4, as here
 (it used to start with bank 0).

 In MAME: Supported.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_yung08_device::irq_timer_tick)
{
	if (BIT(m_irq_latch, 0))
	{
		m_irq_count = (m_irq_count + 1) & 0x0fff;
		if (!m_irq_count)
			set_irq_line(ASSERT_LINE);
	}
}

void nes_yung08_device::write_45(offs_t offset, u8 data)
{
	switch (offset & 0x51ff)
	{
		case 0x4022:
			prg8_cd(data & 1 ? 3 : 4 + BIT(data, 1, 2));
			break;
		case 0x4122:
			m_irq_latch = data & 0x35;
			if (!BIT(m_irq_latch, 0))
			{
				set_irq_line(CLEAR_LINE);
				m_irq_count = 0;
			}
			break;
	}
}

void nes_yung08_device::write_ex(offs_t offset, u8 data)
{
	LOG("yung08 write_ex, offset: %04x, data: %02x\n", offset, data);
	write_45(offset + 0x4020, data);
}

void nes_yung08_device::write_l(offs_t offset, u8 data)
{
	LOG("yung08 write_l, offset: %04x, data: %02x\n", offset, data);
	write_45(offset + 0x4100, data);
}

u8 nes_yung08_device::read_l(offs_t offset)
{
	LOG("yung08 read_l, offset: %04x\n", offset);
	offset += 0x100;
	if ((offset & 0x11ff) == 0x0122)    // 0x4122
		return m_irq_latch | 0x8a;
	return get_open_bus();
}

u8 nes_yung08_device::read_m(offs_t offset)
{
	LOG("yung08 read_m, offset: %04x\n", offset);
	return m_prg[0x02 * 0x2000 + offset];    // fixed to bank #2
}


/*-------------------------------------------------

 60-1064-16L board

 Games: Super Soccer Champion (conversion of Konami's FDS
 Exciting Soccer)

 NESdev NES 2.0 Mapper 538:
 - CPU $6000-$7FFF and $8000-$9FFF: 8 KiB PRG-ROM banks set
   by one register at $C000-$DFFF (mask $E000), low nibble:
   $6000 bank = value | 1,
   $8000 bank = (value & 1) && !(value & 8) ? 10 : value & ~1
 - CPU $A000, $C000, $E000: banks 13, 14, 15
 - 8 KiB unbanked CHR-RAM, mirroring hard-wired to vertical
 The conversion keeps the FDS sound register writes; the
 cartridge has no FDS sound, so none is produced here.

 Not documented, so not emulated: the register's power-on
 value (0 here: bank 1 at $6000, bank 0 at $8000).

 NES 2.0: mapper 538

 In MAME: Supported.

 -------------------------------------------------*/

nes_60106416l_device::nes_60106416l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_60106416L, tag, owner, clock), m_reg(0)
{
}

void nes_60106416l_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_60106416l_device::pcb_reset()
{
	prg8_ab(13);
	prg8_cd(14);
	prg8_ef(15);
	chr8(0, CHRRAM);
	set_nt_mirroring(PPU_MIRROR_VERT);
	m_reg = 0;
	update_banks();
}

void nes_60106416l_device::update_banks()
{
	prg8_89((BIT(m_reg, 0) && !BIT(m_reg, 3)) ? 10 : (m_reg & 0x0e));
}

u8 nes_60106416l_device::read_m(offs_t offset)
{
	return m_prg[((u32(m_reg | 1) * 0x2000) + (offset & 0x1fff)) & (m_prg_size - 1)];
}

void nes_60106416l_device::write_h(offs_t offset, u8 data)
{
	LOG("60-1064-16L write_h, offset: %04x, data: %02x\n", offset, data);

	if ((offset & 0x6000) == 0x4000)   // $C000-$DFFF
	{
		m_reg = data & 0x0f;
		update_banks();
	}
}


/*-------------------------------------------------

 Co Tung CTC-15 board

 Games: Almana no Kiseki (conversion of the FDS game)

 NESdev NES 2.0 Mapper 548:
 - CPU $6000-$7FFF: 8 KiB unbanked PRG-RAM
 - CPU $8000-$BFFF: 16 KiB PRG-ROM bank, $C000-$FFFF: bank 3
 - 8 KiB unbanked CHR-RAM, mirroring hard-wired to horizontal
 - $4800-$4FFF: the address bits A5, A3, A2 are latched as a
   3-bit value ABC (power-on value 7); C is also the IRQ
   control: 0 lets the counter count and raise IRQs, 1 stops
   it, acknowledges the IRQ and resets it to 0
 - $5000-$57FF: the latched value XOR 5 becomes the 16 KiB
   bank at $8000 (power-on bank 7)
 - the counter counts falling edges of M2; /IRQ is low while
   ((count / 640) & 37) == 37, i.e. from 23680 to 24319 the
   first time
 Not documented, so not emulated: whether the IRQ control
 bit acts at the $4800 write or only at the $5000 apply (it
 acts at once here, being listed apart from the latch), and
 the counter's width (32 bits here).

 NES 2.0: mapper 548

 In MAME: Supported.

 -------------------------------------------------*/

nes_ctc15_device::nes_ctc15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_CTC15, tag, owner, clock), m_latch(0), m_irq_count(0)
{
}

void nes_ctc15_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_irq_count));
}

void nes_ctc15_device::pcb_reset()
{
	m_latch = 7;
	m_irq_count = 0;
	set_irq_line(CLEAR_LINE);
	prg16_89ab(7);
	prg16_cdef(3);
	chr8(0, CHRRAM);
	set_nt_mirroring(PPU_MIRROR_HORZ);
}

void nes_ctc15_device::m2_w(int state, offs_t address, bool write)
{
	if (state || BIT(m_latch, 0))
		return;
	m_irq_count++;
	set_irq_line((((m_irq_count / 640) & 37) == 37) ? ASSERT_LINE : CLEAR_LINE);
}

void nes_ctc15_device::write_l(offs_t offset, u8 data)
{
	LOG("ctc-15 write_l, offset: %04x, data: %02x\n", offset, data);

	const u16 addr = offset + 0x4100;
	switch (addr & 0xf800)
	{
		case 0x4800:
			m_latch = (BIT(addr, 5) << 2) | (BIT(addr, 3) << 1) | BIT(addr, 2);
			if (BIT(m_latch, 0))
			{
				m_irq_count = 0;
				set_irq_line(CLEAR_LINE);
			}
			break;
		case 0x5000:
			prg16_89ab(m_latch ^ 5);
			break;
	}
}


/*-------------------------------------------------

 Bit Corp 4-in-1 (cartridge ID 4602)

 Games: 4-in-1 (Mr. Mary 2, Bit Corp's hack of the YUNG-08
 Super Mario Bros. 2 (J) conversion, and three UNROM games)

 NESdev NES 2.0 Mapper 357. There is no menu: two DIP switches
 that stick out of the shell select the outer 128 KiB PRG-ROM
 bank, which also sets the mode (bank 0 the SMB2J conversion,
 banks 1-3 UNROM) and the mirroring (vertical for 0-2,
 horizontal for 3). 8 KiB of unbanked CHR-RAM.
 SMB2J mode:
 - CPU $5000-$5FFF: 4 KiB PRG-ROM bank 16
 - CPU $6000-$7FFF: 8 KiB bank 2, or 0 when $4120 bit 0 is set
 - CPU $8000/$A000: 8 KiB banks 1 and 0
 - CPU $C000-$DFFF: $4022 (mask probably $71FF) bits 2-0 select
   bank 4, 3, 5, 3, 6, 3, 7, 3
 - CPU $E000-$FFFF: 8 KiB bank 10, or 8 when $4120 bit 0 is set
 - $4120 (mask probably $71FF): bit 0 selects the $6000 and
   $E000 banks above; it has no IRQ function
 - $4122 (mask probably $F1FF): bit 0 clear acknowledges and
   disables the IRQ and resets the counter, set enables it; the
   12-bit counter counts M2 cycles and its overflow raises the
   IRQ
 UNROM mode: the UNROM latch at $8000-$FFFF selects the 16 KiB
 bank at $8000 within the outer 128 KiB bank, whose last 16 KiB
 is fixed at $C000.

 Not documented, so not emulated: whether the SMB2J registers
 answer in UNROM mode (they do not here) and whether the UNROM
 latch has bus conflicts (none here); the edge of M2 the counter
 counts (the rise, as on the YUNG-08 board); the DIP switch
 setting (0 here, game 1). A change of the DIP switches takes
 effect at the next reset.

 NES 2.0: mapper 357

 In MAME: Supported.

 -------------------------------------------------*/

static INPUT_PORTS_START( bitcorp4602 )
	PORT_START("DSW")
	PORT_CONFNAME( 0x03, 0x00, "DIP Switches (game)" )
	PORT_CONFSETTING(    0x00, "1 (Mr. Mary 2)" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFSETTING(    0x02, "3" )
	PORT_CONFSETTING(    0x03, "4" )
INPUT_PORTS_END

ioport_constructor nes_bitcorp4602_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bitcorp4602 );
}

nes_bitcorp4602_device::nes_bitcorp4602_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BITCORP4602, tag, owner, clock)
	, m_dsw(*this, "DSW")
	, m_outer(0), m_sel2(0), m_irq_count(0), m_irq_enable(false)
{
}

void nes_bitcorp4602_device::device_start()
{
	common_start();
	save_item(NAME(m_outer));
	save_item(NAME(m_sel2));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_enable));
}

void nes_bitcorp4602_device::pcb_reset()
{
	m_outer = m_dsw->read() & 3;
	m_sel2 = 0;
	m_irq_count = 0;
	m_irq_enable = false;
	set_irq_line(CLEAR_LINE);
	chr8(0, CHRRAM);
	set_nt_mirroring((m_outer == 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	if (m_outer)
	{
		prg16_89ab(m_outer << 3);
		prg16_cdef((m_outer << 3) | 7);
	}
	else
	{
		prg8_89(1);
		prg8_ab(0);
		prg8_cd(4);     // $4022 powers up as 0 here
		prg8_ef(10);
	}
}

void nes_bitcorp4602_device::m2_w(int state, offs_t address, bool write)
{
	if (!state || !m_irq_enable)
		return;
	m_irq_count = (m_irq_count + 1) & 0x0fff;
	if (!m_irq_count)
		set_irq_line(ASSERT_LINE);
}

void nes_bitcorp4602_device::write_regs(offs_t addr, u8 data)
{
	if (m_outer)
		return;
	if ((addr & 0x71ff) == 0x4022)
		prg8_cd(BIT(data, 0) ? 3 : 4 + BIT(data, 1, 2));
	else if ((addr & 0x71ff) == 0x4120)
	{
		m_sel2 = BIT(data, 0);
		prg8_ef(m_sel2 ? 8 : 10);
	}
	else if ((addr & 0xf1ff) == 0x4122)
	{
		m_irq_enable = BIT(data, 0);
		if (!m_irq_enable)
		{
			m_irq_count = 0;
			set_irq_line(CLEAR_LINE);
		}
	}
}

void nes_bitcorp4602_device::write_ex(offs_t offset, u8 data)
{
	LOG("bitcorp 4602 write_ex, offset: %04x, data: %02x\n", offset, data);
	write_regs(offset + 0x4020, data);
}

void nes_bitcorp4602_device::write_l(offs_t offset, u8 data)
{
	LOG("bitcorp 4602 write_l, offset: %04x, data: %02x\n", offset, data);
	write_regs(offset + 0x4100, data);
}

u8 nes_bitcorp4602_device::read_l(offs_t offset)
{
	const u16 addr = offset + 0x4100;
	if (!m_outer && (addr >= 0x5000))
		return m_prg[(16 * 0x1000 + (addr & 0x0fff)) & (m_prg_size - 1)];
	return get_open_bus();
}

u8 nes_bitcorp4602_device::read_m(offs_t offset)
{
	if (m_outer)
		return get_open_bus();
	return m_prg[((m_sel2 ? 0 : 2) * 0x2000 + (offset & 0x1fff)) & (m_prg_size - 1)];
}

void nes_bitcorp4602_device::write_h(offs_t offset, u8 data)
{
	LOG("bitcorp 4602 write_h, offset: %04x, data: %02x\n", offset, data);
	if (m_outer)
		prg16_89ab((m_outer << 3) | (data & 7));
}
