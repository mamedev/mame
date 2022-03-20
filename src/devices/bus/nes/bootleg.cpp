// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Bootleg PCBs


 Here we emulate the PCBs used in FDS2NES conversions which are common in the Taiwanese & HK markets
 Notice that many of these have unusual PRG sizes (32KB+8KB, 32KB+24KB) partially mapped in the WRAM
 area, or WRAM overlapping the usual PRG area, so that we often skip the usual bankswitching mechanisms
 in favor of direct handling of the PRG accesses

 TODO:
 - review all PCBs and fix the starting banks (which are often the main problem of not working games)
 - SMB2 bootlegs all seem to have timing issues. This is apparent on worlds A-D as the bottom of the
   world letter scrolls. Hardware footage of the mapper 50 version shows the letter bottoms flickering
   (though it could be a video/compression artifact).

 ***********************************************************************************************************/


#include "emu.h"
#include "bootleg.h"

#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE
#include "screen.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_SC127,          nes_sc127_device,     "nes_sc127",     "NES Cart SC-127 PCB")
DEFINE_DEVICE_TYPE(NES_MARIOBABY,      nes_mbaby_device,     "nes_mbaby",     "NES Cart Mario Baby Bootleg PCB")
DEFINE_DEVICE_TYPE(NES_ASN,            nes_asn_device,       "nes_asn",       "NES Cart Ai Senshi Nicol Bootleg PCB")
DEFINE_DEVICE_TYPE(NES_SMB3PIRATE,     nes_smb3p_device,     "nes_smb3p",     "NES Cart Super Mario Bros. 3 Pirate PCB")
DEFINE_DEVICE_TYPE(NES_BTL_CONTRAJ,    nes_btl_cj_device,    "nes_btl_cj",    "NES Cart Contra Japan Pirate PCB")
DEFINE_DEVICE_TYPE(NES_BTL_DNINJA,     nes_btl_dn_device,    "nes_btl_dn",    "NES Cart DragonNinja Pirate PCB")
DEFINE_DEVICE_TYPE(NES_SMB2J,          nes_smb2j_device,     "nes_smb2j",     "NES Cart Super Mario Bros. 2 Jpn PCB")
DEFINE_DEVICE_TYPE(NES_SMB2JA,         nes_smb2ja_device,    "nes_smb2ja",    "NES Cart Super Mario Bros. 2 Jpn (Alt) PCB")
DEFINE_DEVICE_TYPE(NES_SMB2JB,         nes_smb2jb_device,    "nes_smb2jb",    "NES Cart Super Mario Bros. 2 Jpn (Alt 2) PCB")
DEFINE_DEVICE_TYPE(NES_N32_4IN1,       nes_n32_4in1_device,  "nes_n32_4in1",  "NES Cart N-32 4 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_0353,           nes_0353_device,      "nes_0353",      "NES Cart 0353 PCB")
DEFINE_DEVICE_TYPE(NES_09034A,         nes_09034a_device,    "nes_09034a",    "NES Cart 09-034A PCB")
DEFINE_DEVICE_TYPE(NES_L001,           nes_l001_device,      "nes_l001",      "NES Cart L-001 PCB")
DEFINE_DEVICE_TYPE(NES_BATMANFS,       nes_batmanfs_device,  "nes_batmanfs",  "NES Cart Batman Pirate PCB")
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


nes_sc127_device::nes_sc127_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SC127, tag, owner, clock), m_irq_count(0), m_irq_enable(0)
{
}

nes_mbaby_device::nes_mbaby_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_MARIOBABY, tag, owner, clock), m_irq_count(0), m_irq_enable(0), m_latch(0), irq_timer(nullptr)
{
}

nes_asn_device::nes_asn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_ASN, tag, owner, clock), m_latch(0)
{
}

nes_smb3p_device::nes_smb3p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_SMB3PIRATE, tag, owner, clock), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
{
}

nes_btl_cj_device::nes_btl_cj_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BTL_CONTRAJ, tag, owner, clock)
{
}

nes_btl_dn_device::nes_btl_dn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BTL_DNINJA, tag, owner, clock), m_irq_count(0)
{
}

nes_smb2j_device::nes_smb2j_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_SMB2J, tag, owner, clock), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
{
}

nes_smb2ja_device::nes_smb2ja_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_SMB2JA, tag, owner, clock), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
{
}

nes_smb2jb_device::nes_smb2jb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 bank67)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_irq_count(0), m_irq_enable(0), m_reg(0), m_bank67(bank67), irq_timer(nullptr)
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
	: nes_nrom_device(mconfig, NES_09034A, tag, owner, clock), m_irq_count(0), m_irq_enable(0), m_reg(0), irq_timer(nullptr)
{
}

nes_l001_device::nes_l001_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_L001, tag, owner, clock), m_irq_count(0), irq_timer(nullptr)
{
}

nes_batmanfs_device::nes_batmanfs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BATMANFS, tag, owner, clock), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
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

nes_lh10_device::nes_lh10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_LH10, tag, owner, clock), m_latch(0)
{
}

nes_lh51_device::nes_lh51_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_LH51, tag, owner, clock)
{
}

nes_lh53_device::nes_lh53_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_LH53, tag, owner, clock), m_irq_count(0), m_irq_enable(0), m_reg(0), irq_timer(nullptr)
{
}

nes_2708_device::nes_2708_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_2708, tag, owner, clock)
{
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
	: nes_nrom_device(mconfig, NES_YUNG08, tag, owner, clock), m_irq_count(0), m_irq_latch(0), irq_timer(nullptr)
{
}




void nes_sc127_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_sc127_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0xff);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_mbaby_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_latch));
}

void nes_mbaby_device::pcb_reset()
{
	prg32((m_prg_chunks - 1) >> 1);
	chr8(0, CHRRAM);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_latch = 0;
}

void nes_asn_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_asn_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32((m_prg_chunks - 1) >> 1);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_smb3p_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

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
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_batmanfs_device::pcb_reset()
{
	prg32((m_prg_chunks >> 1) - 1);    // Last 8K bank is fixed, the rest are swappable
	chr8(0, CHRROM);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_btl_cj_device::pcb_reset()
{
	prg32((m_prg_chunks >> 1) - 1);    // Last 8K bank is fixed, the rest are swappable
	chr8(0, CHRROM);
}

void nes_btl_dn_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_count));
}

void nes_btl_dn_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_count = 0;
}

void nes_smb2j_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

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
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

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
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

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
	chr8(0, CHRRAM);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_reg = 0;
}

void nes_n32_4in1_device::pcb_reset()
{
	// Powers up in menu, but soft reset does not touch banks (so each game returns to its own title). Is this correct?

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
	chr8(0, CHRRAM);

	m_reg = 0;
}

void nes_09034a_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_reg));
}

void nes_09034a_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_reg = 0;
}

void nes_l001_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

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
	// 0xa000-0xbfff switchable bank
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRRAM);

	m_reg = 0;
}

void nes_tobidase_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_tobidase_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(2);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_whirlwind_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_whirlwind_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
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
	chr8(0, CHRRAM);

	prg32((m_prg_chunks - 1) >> 1);
	// 0xc000-0xdfff reads/writes WRAM
	m_latch = 0xf;
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
	chr8(0, CHRRAM);

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
	chr8(0, CHRRAM);

	m_latch = 0;
}

void nes_lh10_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_lh10_device::pcb_reset()
{
	prg8_89(0);
	prg8_ab(0);
	// 0xc000-0xdfff reads/writes WRAM
	prg8_ef((m_prg_chunks << 1) - 1);
	chr8(0, CHRRAM);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_latch = 0;
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
}

void nes_lh51_device::pcb_reset()
{
	prg32((m_prg_chunks >> 1) - 1);    // first 8K is switchable, the rest fixed
	chr8(0, CHRRAM);
}

void nes_lh53_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_reg));
}

void nes_lh53_device::pcb_reset()
{
	chr8(0, CHRRAM);

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
	chr8(0, CHRRAM);

	prg32(7);
	// the upper PRG banks never change, but there are 8K of WRAM overlaid to the ROM area based on reg1
	m_reg[0] = 0;
	m_reg[1] = 0;
}

void nes_ac08_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_ac08_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	chr8(0, m_chr_source);
	prg32(0xff);
	m_latch = 0xff;
}

void nes_mmalee_device::device_start()
{
	common_start();
}

void nes_mmalee_device::pcb_reset()
{
	chr8(0, CHRROM);
	prg32(0);
}

void nes_rt01_device::device_start()
{
	common_start();
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
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_latch));
}

void nes_yung08_device::pcb_reset()
{
	prg8_89(1);
	prg8_ab(0);
	prg8_cd(0);    // switchable bank
	prg8_ef(8);
	chr8(0, CHRROM);

	m_irq_count = 0;
	m_irq_latch = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 SC-127 Board

 Games: Wario World II (Kirby Hack)

 iNES: mapper 35

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sc127_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (scanline < ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE && m_irq_enable)
	{
		m_irq_count--;

		if (!blanked && (m_irq_count == 0))
		{
			LOG_MMC(("irq fired, scanline: %d\n", scanline));
			set_irq_line(ASSERT_LINE);
			m_irq_enable = 0;
		}
	}
}

void nes_sc127_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("sc127 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x0001:
			prg8_ab(data);
			break;
		case 0x0002:
//          m_mmc_prg_bank[offset & 0x02] = data;
			prg8_cd(data);
			break;
		case 0x1000:
		case 0x1001:
		case 0x1002:
		case 0x1003:
		case 0x1004:
		case 0x1005:
		case 0x1006:
		case 0x1007:
//          m_mmc_vrom_bank[offset & 0x07] = data;
			chr1_x(offset & 0x07, data, CHRROM);
			break;
		case 0x4002:
			m_irq_enable = 0;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x4003:
			m_irq_enable = 1;
			break;
		case 0x4005:
			m_irq_count = data;
			break;
		case 0x5001:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*-------------------------------------------------

 BTL-MARIOBABY

 Games: Mario Baby, Ai Senshi Nicol

 iNES: mapper 42

 In MAME: Supported.

 -------------------------------------------------*/

void nes_mbaby_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			m_irq_count = (m_irq_count + 1) & 0x7fff;  // unverified 15-bit counter based on FCEUX

			if (m_irq_count >= 0x6000)
				set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_mbaby_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("Mario Baby write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x7000)
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
	LOG_MMC(("Mario Baby read_m, offset: %04x\n", offset));
	return m_prg[(m_latch * 0x2000) + (offset & 0x1fff)];
}

/*-------------------------------------------------

 BTL-AISENSHINICOL

 Games: Ai Senshi Nicol

 iNES: mapper 42 with no IRQ and no NT, but CHR switch

 In MAME: Partially supported.

 -------------------------------------------------*/

void nes_asn_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("Ai Senshi Nicol write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x0000)
		chr8(data, CHRROM);

	if (offset == 0x7000)
		m_latch = data;
}

uint8_t nes_asn_device::read_m(offs_t offset)
{
	LOG_MMC(("Ai Senshi Nicol read_m, offset: %04x\n", offset));
	return m_prg[((m_latch * 0x2000) + (offset & 0x1fff)) & (m_prg_size - 1)];
}

/*-------------------------------------------------

 BTL-SMB3

 Games: Super Mario Bros. 3 Pirate

 iNES: mapper 106

 In MAME: Supported.

 -------------------------------------------------*/

void nes_smb3p_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		// counter does not stop when interrupts are disabled
		if (m_irq_count != 0xffff)
			m_irq_count++;
		else if (m_irq_enable)
			set_irq_line(ASSERT_LINE);
	}
}

void nes_smb3p_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("btl_smb3_w, offset: %04x, data: %02x\n", offset, data));

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

 In MAME: Supported.

 TODO: Find out why this crashes in the intro story.
 Bootleggers missed the bug? Bad dump? This differs
 only in a couple hundred bytes from contraj, so there
 aren't that many places for the problem to hide.

 -------------------------------------------------*/

void nes_btl_cj_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("btl_cj write_h, offset: %04x, data: %02x\n", offset, data));

	if (BIT(offset, 4))
	{
		if (BIT(offset, 3))
			set_nt_page(offset & 0x03, CIRAM, data & 1, 1);
		else
			chr1_x(offset & 0x07, data, CHRROM);
	}
	else
	{
		offset = (offset >> 13) & 0x03;
		if (offset != 3)
			prg8_x(offset, data & 0x0f);
	}
}

/*-------------------------------------------------

 BTL-DRAGONNINJA

 Games: Dragon Ninja (Bootleg), Super Mario Bros. 8

 iNES: mapper 222

 In MAME: Unsupported.

 -------------------------------------------------*/

/* Scanline based IRQ ? */
void nes_btl_dn_device::hblank_irq(int scanline, int vblank, int blanked )
{
	if (!m_irq_count || ++m_irq_count < 240)
		return;

	m_irq_count = 0;
	hold_irq_line();
}

void nes_btl_dn_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t bank;
	LOG_MMC(("btl_dn write_h, offset: %04x, data: %02x\n", offset, data));

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
			bank = ((offset & 0x7000) - 0x3000) / 0x0800 + ((offset & 0x0002) >> 1);
			chr1_x(bank, data, CHRROM);
			break;
		case 0x7000:
			m_irq_count = data;
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
	LOG_MMC(("whirlwind read_m, offset: %04x\n", offset));
	return m_prg[((m_reg & m_prg_mask) * 0x2000) + offset];
}

void nes_dh08_device::write_h(offs_t offset, u8 data)      // submapper 1
{
	LOG_MMC(("dh08 write_h, offset: %04x, data: %02x\n", offset, data));
	if (offset >= 0x7000)
		m_reg = data;
}

void nes_le05_device::write_h(offs_t offset, u8 data)      // submapper 4
{
	LOG_MMC(("le05 write_h, offset: %04x, data: %02x\n", offset, data));
	chr8(data & 1, m_chr_source);
}

void nes_lh28_lh54_device::write_h(offs_t offset, u8 data) // submapper 3
{
	LOG_MMC(("lh28_lh54 write_h, offset: %04x, data: %02x\n", offset, data));
	m_reg = data;
}

void nes_lh31_device::write_h(offs_t offset, u8 data)      // submapper 2
{
	LOG_MMC(("lh31 write_h, offset: %04x, data: %02x\n", offset, data));
	if (offset >= 0x6000)
	{
		m_reg = data;
		chr8(data & (m_vrom_chunks - 1), m_chr_source);
	}
}

/*-------------------------------------------------

 Bootleg Board SMB2J

 Games: Super Mario Bros. 2 Pirates (TONY-I, YS-612)

 iNES: mapper 43

 In MAME: Supported.

 -------------------------------------------------*/

void nes_smb2j_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			m_irq_count = (m_irq_count + 1) & 0xfff;
			if (!m_irq_count)
				set_irq_line(ASSERT_LINE);
		}
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
	LOG_MMC(("smb2j write_ex, offset: %04x, data: %02x\n", offset, data));
	nes_smb2j_device::write_45(offset + 0x4020, data);
}

void nes_smb2j_device::write_l(offs_t offset, u8 data)
{
	LOG_MMC(("smb2j write_l, offset: %04x, data: %02x\n", offset, data));
	nes_smb2j_device::write_45(offset + 0x4100, data);
}

void nes_smb2j_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("smb2j write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x122)    // $8122 is also IRQ for YS-612, mask unknown
		nes_smb2j_device::update_irq(data);
}

u8 nes_smb2j_device::read_l(offs_t offset)
{
	LOG_MMC(("smb2j read_l, offset: %04x\n", offset));
	offset += 0x100;

	if (offset >= 0x1000)
		return m_prg[0x10000 + (offset & 0x0fff)];

	return get_open_bus();
}

u8 nes_smb2j_device::read_m(offs_t offset)
{
	LOG_MMC(("smb2j read_m, offset: %04x\n", offset));
	return m_prg[0x4000 + offset];
}

/*-------------------------------------------------

 BTL-SMB2JA

 Games: Super Mario Bros. 2 Pirates (LF36, Hey Sung, 1990 SMB4)

 iNES: mapper 40

 In MAME: Supported.

 -------------------------------------------------*/

void nes_smb2ja_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
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
}

void nes_smb2ja_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("smb2ja write_h, offset: %04x, data: %02x\n", offset, data));

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
	LOG_MMC(("smb2ja read_m, offset: %04x\n", offset));
	return m_prg[0x06 * 0x2000 + offset];    // fixed 8K bank
}

/*-------------------------------------------------

 BTL-SMB2JB (PCB 761214)

 Games: Super Mario Bros. 2 Pirate (N-32)

 This was marked as Alt. Levels. is it true?

 iNES: mapper 50

 In MAME: Supported.

 -------------------------------------------------*/

void nes_smb2jb_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (BIT(++m_irq_count, 12))
				set_irq_line(ASSERT_LINE);
		}
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
	LOG_MMC(("smb2jb write_ex, offset: %04x, data: %02x\n", offset, data));
	write_45(offset + 0x4020, data);
}

void nes_smb2jb_device::write_l(offs_t offset, u8 data)
{
	LOG_MMC(("smb2jb write_l, offset: %04x, data: %02x\n", offset, data));
	write_45(offset + 0x4100, data);
}

u8 nes_smb2jb_device::read_m(offs_t offset)
{
	LOG_MMC(("smb2jb read_m, offset: %04x\n", offset));
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

 In MAME: Supported.

 -------------------------------------------------*/

void nes_n32_4in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("n32_4in1 write_h, offset: %04x, data: %02x\n", offset, data));

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

		chr8((data >> 1) & 0x03, CHRROM);
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
// LOG_MMC(("0353 read_m, offset: %04x\n", offset));
	return m_prg[m_reg * 0x2000 + offset];
}

void nes_0353_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("0353 write_h, offset: %04x, data: %02x\n", offset, data));

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
 world 4-4 and the beginning of world 5-1. How the IRQ
 functions is to be confirmed but it likely uses a 12-bit
 counter just like the other SMB2 bootlegs.  That is how
 we presently emulate it here.

 NES 2.0: mapper 304

 In MAME: Supported.

 -------------------------------------------------*/

void nes_09034a_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			m_irq_count = (m_irq_count + 1) & 0x0fff;
			if (!m_irq_count)
				set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_09034a_device::write_ex(offs_t offset, u8 data)
{
	LOG_MMC(("09-034a write_ex, offset: %04x, data: %02x\n", offset, data));

	offset += 0x20;
	switch (offset)
	{
		case 0x0027:
			m_reg = data & 1;
			break;
		case 0x0068:
			m_irq_enable = BIT(data, 0);
			if (!m_irq_enable)
			{
				m_irq_count = 0;
				set_irq_line(CLEAR_LINE);
			}
			break;
	}
}

u8 nes_09034a_device::read_ex(offs_t offset)
{
	LOG_MMC(("09-034a read_ex, offset: %04x, data: %02x\n", offset));

	offset += 0x20;
	// SMB2 does not boot with the default open bus reads in this range
	if (offset >= 0x42 && offset <= 0x55)
		return 0xff;
	else
		return get_open_bus();
}

u8 nes_09034a_device::read_m(offs_t offset)
{
	LOG_MMC(("09-034a read_m, offset: %04x\n", offset));
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

 NES 2.0: mapper 330

 In MAME: Supported.

 -------------------------------------------------*/

void nes_l001_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (BIT(m_irq_count, 15))
		{
			if (++m_irq_count == 0)
				set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_l001_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("l-001 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6400)
	{
		case 0x0000:
		case 0x2000:
			chr1_x((offset >> 11) & 0x07, data, CHRROM);
			break;
		case 0x0400:
			m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x2400:
			m_irq_count = (m_irq_count & 0x00ff) | data << 8;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x4000:
			set_nt_page((offset >> 11) & 0x03, CIRAM, data & 1, 1);
			break;
		case 0x6000:
			if (offset < 0x7800)
				prg8_x((offset >> 11) & 0x03, data & 0x1f);
			break;
	}
}

/*-------------------------------------------------

 BTL-BATMANFS

 Games: Batman "Fine Studio" pirate

 NES 2.0: mapper 417

 In MAME: Supported.

 -------------------------------------------------*/

void nes_batmanfs_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		// 10-bit counter does not stop when interrupts are disabled
		m_irq_count = (m_irq_count + 1) & 0x3ff;
		if (m_irq_enable && !m_irq_count)
			set_irq_line(ASSERT_LINE);
	}
}

void nes_batmanfs_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("batmanfs write_h, offset: %04x, data: %02x\n", offset, data));
	switch (offset & 0x70)
	{
		case 0x00:
			if ((offset & 0x03) != 0x03)
				prg8_x(offset & 0x03, data & 0x0f);
			break;
		case 0x10:
		case 0x20:
			chr1_x((offset & 0x03) + 4 * BIT(offset, 5), data, CHRROM);
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
			set_nt_page(offset & 0x03, CIRAM, data & 1, 1);
			break;
	}
}

/*-------------------------------------------------

 BTL-PALTHENA

 Games: Palthena no Kagami (FDS conversion)

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
//  LOG_MMC(("palthena read_m, offset: %04x\n", offset));
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
	LOG_MMC(("palthena write_m, offset: %04x, data: %02x\n", offset, data));
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
//  LOG_MMC(("palthena read_h, offset: %04x\n", offset));
	u8 page = (offset >> 8);
	if ((page >= 0x40 && page < 0x52) || page == 0x5f)
		return m_prgram[offset & 0x1fff];
	else if (page == 0x02)
		return m_prgram[offset | 0x1000];

	return hi_access_rom(offset);
}

void nes_palthena_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("palthena write_h, offset: %04x, data: %02x\n", offset, data));
	u8 page = (offset >> 8);
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

 In MAME: Partially supported.

 -------------------------------------------------*/

void nes_tobidase_device::write_l(offs_t offset, uint8_t data)
{
	LOG_MMC(("tobidase write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x4100;

	if ((offset & 0x63c0) == 0x41c0)
		m_latch = data & 0x0f;
}

uint8_t nes_tobidase_device::read_m(offs_t offset)
{
	LOG_MMC(("tobidase read_m, offset: %04x\n", offset));
	if (m_latch >= 0x0c)
		m_latch -= 4;
	return m_prg[(m_latch * 0x2000) + (offset & 0x1fff)];
}

/*-------------------------------------------------

 UNL-LH32

 Games: Monty no Doki Doki Daidassou (FDS conversion)

 This PCB maps WRAM in 0xc000-0xdfff and PRG in 0x6000-0x7fff

 iNES: mapper 125

 In MAME: Supported.

-------------------------------------------------*/

uint8_t nes_lh32_device::read_m(offs_t offset)
{
	LOG_MMC(("lh32 read_m, offset: %04x\n", offset));
	return m_prg[(m_latch * 0x2000) + (offset & 0x1fff)];
}

uint8_t nes_lh32_device::read_h(offs_t offset)
{
//  LOG_MMC(("lh32 read_h, offset: %04x\n", offset));

	if (offset >= 0x4000 && offset < 0x6000)
		return m_prgram[offset & 0x1fff];

	return hi_access_rom(offset);
}

void nes_lh32_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("lh32 write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0)    // 0x6000 only?
	{
//      printf("write %x\n", data);
		m_latch = data & 0xf;
	}
}

void nes_lh32_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("lh32 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x4000 && offset < 0x6000)
		m_prgram[offset & 0x1fff] = data;
}

/*-------------------------------------------------

 UNL-LH42

 Games: Highway Star (Whirlwind Manu bootleg)

 NES 2.0: mapper 418

 In MAME: Preliminary supported.

 TODO: Investigate garbage tiles on bottom half of
 course map screens. This should be car dashboard?

 -------------------------------------------------*/

void nes_lh42_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("lh42 write_h, offset: %04x, data: %02x\n", offset, data));

	if (BIT(offset, 0))
	{
		switch (m_latch)
		{
			case 1:
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;
			case 2:
			case 3:
				prg8_x(m_latch & 1, data & 0x0f);
				break;
		}
	}
	else
		m_latch = data & 0x03;
}

/*-------------------------------------------------

 UNL-LG25

 Games: Moero TwinBee Cinnamon Hakase o Sukue! (FDS conversion)

 In addition to the two swappable 8K PRG banks at
 0x8000 and 0xa000, this board has 8K WRAM at 0x6000.

 NES 2.0: mapper 557

 In MAME: Supported.

 -------------------------------------------------*/

void nes_lg25_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("lg25 write_h, offset: %04x, data: %02x\n", offset, data));

	if (BIT(offset, 0))
	{
		switch (m_latch)
		{
			case 1:
				set_nt_mirroring(BIT(data, 2) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
				break;
			case 2:
			case 3:
				prg8_x(m_latch & 1, data & 0x0f);
				break;
		}
	}
	else
		m_latch = data & 0x03;
}

/*-------------------------------------------------

 UNL-LH10

 Games: Fuuun Shaolin Kyo (FDS conversion)

 This PCB maps WRAM in 0xc000-0xdfff and PRG in 0x6000-0x7fff
 This is very similar to KS7037 (see kaiser.cpp)

 NES 2.0: mapper 522

 In MAME: Supported.

 -------------------------------------------------*/

void nes_lh10_device::update_prg()
{
	prg8_89(m_reg[6]);
	prg8_ab(m_reg[7]);
}

uint8_t nes_lh10_device::read_m(offs_t offset)
{
	LOG_MMC(("lh10 read_m, offset: %04x\n", offset));
	return m_prg[(0x0e * 0x2000) + (offset & 0x1fff)];
}

uint8_t nes_lh10_device::read_h(offs_t offset)
{
//  LOG_MMC(("lh10 read_h, offset: %04x\n", offset));

	if (offset >= 0x4000 && offset < 0x6000)
		return m_prgram[offset & 0x1fff];

	return hi_access_rom(offset);
}

void nes_lh10_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("lh10 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x4000 && offset < 0x6000)
		m_prgram[offset & 0x1fff] = data;
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

 UNL-LH51

 Games: Ai Senshi Nicol (Whirlwind Manu FDS conversion)

 A simple board with swappable 8K PRG at 0x8000-0x9fff,
 fixed PRG above that, and 8K WRAM at 0x6000-0x7fff.
 The game's sound code is broken and does not work on
 real hardware.

 NES 2.0: mapper 309

 In MAME: Supported.

 -------------------------------------------------*/

void nes_lh51_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("lh51 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x0000:
		case 0x1000:
			prg8_89(data & 0x0f);
			break;
		case 0x6000:
		case 0x7000:
			set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*-------------------------------------------------

 UNL-LH53

 Games: Nazo no Murasamejou (FDS conversion)

 This PCB maps WRAM in 0xb800-0xd7ff and PRG in 0x6000-0x7fff

 NES 2.0: mapper 535

 In MAME: Preliminary support only.

 -------------------------------------------------*/

void nes_lh53_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			m_irq_count++;
			if (m_irq_count > 7560)//value from FCEUMM...
			{
				m_irq_count = 0;
				set_irq_line(ASSERT_LINE);
			}
		}
	}
}

uint8_t nes_lh53_device::read_m(offs_t offset)
{
	LOG_MMC(("lh53 read_m, offset: %04x\n", offset));
	return m_prg[m_reg * 0x2000 + offset];
}

uint8_t nes_lh53_device::read_h(offs_t offset)
{
//  LOG_MMC(("lh53 read_h, offset: %04x\n", offset));

	if (offset >= 0x3800 && offset < 0x5800)
		return m_prgram[offset - 0x3800];

	return hi_access_rom(offset);
}

void nes_lh53_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("lh53 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x3800 && offset < 0x5800)
		m_prgram[offset - 0x3800] = data;
	else
	{
		switch (offset & 0x7000)
		{
			case 0x6000:
				m_irq_enable = BIT(data, 1);
				m_irq_count = 0;
				if (!m_irq_enable)
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

 In MAME: Supported.

 -------------------------------------------------*/

uint8_t nes_2708_device::read_m(offs_t offset)
{
	LOG_MMC(("btl-2708 read_m, offset: %04x\n", offset));
	if (!m_reg[1])
		return m_prgram[offset];    // lower 8K of WRAM
	else
		return m_prg[(m_reg[0] * 0x2000) + (offset & 0x1fff)];
}

void nes_2708_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("btl-2708 write_m, offset: %04x, data: %02x\n", offset, data));
//  if (!m_reg[1])
		m_prgram[offset] = data;    // lower 8K of WRAM
}

uint8_t nes_2708_device::read_h(offs_t offset)
{
//  LOG_MMC(("btl-2708 read_h, offset: %04x\n", offset));

	if (offset >= 0x3800 && offset < 0x5800 && !m_reg[1])
		return m_prgram[0x2000 + ((offset - 0x3800) & 0x1fff)]; // higher 8K of WRAM

	return hi_access_rom(offset);
}

void nes_2708_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("btl-2708 write_h, offset: %04x, data: %02x\n", offset, data));

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

 iNES: ???

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ac08_device::write_ex(offs_t offset, uint8_t data)
{
	LOG_MMC(("AC-08 write_ex, offset: %04x, data: %02x\n", offset, data));
	if (offset == 5)    // $4025
		set_nt_mirroring(!BIT(data, 3) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
}

uint8_t nes_ac08_device::read_m(offs_t offset)
{
	LOG_MMC(("AC-08 read_m, offset: %04x\n", offset));
	return m_prg[(m_latch * 0x2000) + (offset & 0x1fff)];
}

void nes_ac08_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("AC-08 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 1)
		m_latch = (data >> 1) & 0x0f;
	else
		m_latch = data & 0x0f;  // apparently there also is a Castlevania FDS conversion using same board with different banking lines
}

/*-------------------------------------------------

 BTL-MARIO1-MALEE2 (aka Genius Merio Bros)

 Games: Super Mario Bros Malee 2

 This PCB has two PRG chips (32K+2K) + one CHR chip (8K)
 + 2KB of WRAM
 The second PRG chip (2K) is connected at 0x6000-0x6800
 while WRAM is at 0x7000-0x7800

 iNES: mapper 55

 In MAME: Supported.

 -------------------------------------------------*/

uint8_t nes_mmalee_device::read_m(offs_t offset)
{
	LOG_MMC(("mmalee read_m, offset: %04x\n", offset));

	if (offset < 0x0800)
		return m_prg[0x8000 + offset];
	else if (!m_prgram.empty() && offset >= 0x1000 && offset < 0x1800)   // WRAM only in these 2K
		return m_prgram[offset & 0x7ff];

	return ((offset + 0x6000) & 0xff00) >> 8;
}

void nes_mmalee_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("mmalee write_m, offset: %04x, data: %02x\n", offset, data));

	if (!m_prgram.empty() && offset >= 0x1000 && offset < 0x1800)    // WRAM only in these 2K
		m_prgram[offset & 0x7ff] = data;
}

/*-------------------------------------------------

 RT-01

 Games: Russian test cart

 The PRG EPROM has copy protected areas with
 "weak bits", which is tested at some points (info
 from Cah4e3).

 NES 2.0: mapper 328

 In MAME: Partially supported?

 -------------------------------------------------*/

uint8_t nes_rt01_device::read_h(offs_t offset)
{
//  LOG_MMC(("rt01 read_h, offset: %04x\n", offset));

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

 In MAME: Supported.

 -------------------------------------------------*/

void nes_yung08_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (BIT(m_irq_latch, 0))
		{
			m_irq_count = (m_irq_count + 1) & 0x0fff;
			if (!m_irq_count)
				set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_yung08_device::write_45(offs_t offset, u8 data)
{
	switch (offset & 0x51ff)
	{
		case 0x4022:
			prg8_cd(data & 1 ? 3 : 4 + ((data & 0x07) >> 1));
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
	LOG_MMC(("yung08 write_ex, offset: %04x, data: %02x\n", offset, data));
	write_45(offset + 0x4020, data);
}

void nes_yung08_device::write_l(offs_t offset, u8 data)
{
	LOG_MMC(("yung08 write_l, offset: %04x, data: %02x\n", offset, data));
	write_45(offset + 0x4100, data);
}

u8 nes_yung08_device::read_l(offs_t offset)
{
	LOG_MMC(("yung08 read_l, offset: %04x\n", offset));
	offset += 0x100;
	if ((offset & 0x11ff) == 0x0122)    // 0x4122
		return m_irq_latch | 0x8a;
	return get_open_bus();
}

u8 nes_yung08_device::read_m(offs_t offset)
{
	LOG_MMC(("yung08 read_m, offset: %04x\n", offset));
	return m_prg[0x02 * 0x2000 + offset];    // fixed to bank #2
}
