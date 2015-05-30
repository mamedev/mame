// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for MMC-3 clone PCBs


 Here we emulate several pirate PCBs based on MMC-3 boards

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc3_clones.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_NITRA = &device_creator<nes_nitra_device>;
const device_type NES_KS7057 = &device_creator<nes_ks7057_device>;
const device_type NES_SBROS11 = &device_creator<nes_sbros11_device>;
const device_type NES_MALISB = &device_creator<nes_malisb_device>;
const device_type NES_FAMILY4646 = &device_creator<nes_family4646_device>;
const device_type NES_PIKAY2K = &device_creator<nes_pikay2k_device>;
const device_type NES_8237 = &device_creator<nes_8237_device>;
const device_type NES_SG_LIONK = &device_creator<nes_sglionk_device>;
const device_type NES_SG_BOOG = &device_creator<nes_sgboog_device>;
const device_type NES_KASING = &device_creator<nes_kasing_device>;
const device_type NES_KAY = &device_creator<nes_kay_device>;
const device_type NES_H2288 = &device_creator<nes_h2288_device>;
const device_type NES_6035052 = &device_creator<nes_6035052_device>;
const device_type NES_TXC_TW = &device_creator<nes_txc_tw_device>;
const device_type NES_KOF97 = &device_creator<nes_kof97_device>;
const device_type NES_KOF96 = &device_creator<nes_kof96_device>;
const device_type NES_SF3 = &device_creator<nes_sf3_device>;
const device_type NES_GOUDER = &device_creator<nes_gouder_device>;
const device_type NES_SA9602B = &device_creator<nes_sa9602b_device>;
const device_type NES_SACHEN_SHERO = &device_creator<nes_sachen_shero_device>;
//const device_type NES_A9746 = &device_creator<nes_a9746_device>;

const device_type NES_FK23C = &device_creator<nes_fk23c_device>;
const device_type NES_FK23CA = &device_creator<nes_fk23ca_device>;
const device_type NES_S24IN1SC03 = &device_creator<nes_s24in1sc03_device>;
const device_type NES_BMC_15IN1 = &device_creator<nes_bmc_15in1_device>;
const device_type NES_BMC_SBIG7 = &device_creator<nes_bmc_sbig7_device>;
const device_type NES_BMC_HIK8 = &device_creator<nes_bmc_hik8_device>;
const device_type NES_BMC_HIK4 = &device_creator<nes_bmc_hik4_device>;
const device_type NES_BMC_MARIO7IN1 = &device_creator<nes_bmc_mario7in1_device>;
const device_type NES_BMC_GOLD7IN1 = &device_creator<nes_bmc_gold7in1_device>;
const device_type NES_BMC_GC6IN1 = &device_creator<nes_bmc_gc6in1_device>;
const device_type NES_BMC_411120C = &device_creator<nes_bmc_411120c_device>;
const device_type NES_BMC_830118C = &device_creator<nes_bmc_830118c_device>;
const device_type NES_PJOY84 = &device_creator<nes_pjoy84_device>;


nes_nitra_device::nes_nitra_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_NITRA, "NES Cart Nitra PCB", tag, owner, clock, "nes_nitra", __FILE__)
{
}

nes_ks7057_device::nes_ks7057_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_KS7057, "NES Cart KS-7057 PCB", tag, owner, clock, "nes_ks7057", __FILE__)
{
}

nes_sbros11_device::nes_sbros11_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_SBROS11, "NES Cart SMB 11 PCB", tag, owner, clock, "nes_smb11", __FILE__)
{
}

nes_malisb_device::nes_malisb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_MALISB, "NES Cart Mali Spash Bomb PCB", tag, owner, clock, "nes_malisb", __FILE__)
{
}

nes_family4646_device::nes_family4646_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_FAMILY4646, "NES Cart BMC-FAMILY-4646 PCB", tag, owner, clock, "nes_family4646", __FILE__)
{
}

nes_pikay2k_device::nes_pikay2k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_PIKAY2K, "NES Cart PIKACHU Y2K PCB", tag, owner, clock, "nes_pikay2k", __FILE__)
{
}

nes_8237_device::nes_8237_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_8237, "NES Cart UNL-8237 PCB", tag, owner, clock, "nes_8237", __FILE__)
{
}

nes_sglionk_device::nes_sglionk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_SG_LIONK, "NES Cart SuperGame Lion King PCB", tag, owner, clock, "nes_sglionk", __FILE__)
{
}

nes_sgboog_device::nes_sgboog_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_SG_BOOG, "NES Cart SuperGame BoogerMan PCB", tag, owner, clock, "nes_sgbooger", __FILE__)
{
}

nes_kasing_device::nes_kasing_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_KASING, "NES Cart Kasing PCB", tag, owner, clock, "nes_kasing", __FILE__)
{
}

nes_kay_device::nes_kay_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_KAY, "NES Cart KAY PCB", tag, owner, clock, "nes_kay", __FILE__)
{
}

nes_h2288_device::nes_h2288_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_H2288, "NES Cart H-2288 PCB", tag, owner, clock, "nes_h2288", __FILE__)
{
}

nes_6035052_device::nes_6035052_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_6035052, "NES Cart UNL-603-5052 PCB", tag, owner, clock, "nes_6035052", __FILE__)
{
}

nes_txc_tw_device::nes_txc_tw_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_TXC_TW, "NES Cart TXC Thunder Warrior PCB", tag, owner, clock, "nes_txc_tw", __FILE__)
{
}

nes_kof97_device::nes_kof97_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_KOF97, "NES Cart KOF 97 PCB", tag, owner, clock, "nes_kof97", __FILE__)
{
}

nes_kof96_device::nes_kof96_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_KOF96, "NES Cart KOF 96 PCB", tag, owner, clock, "nes_kof96", __FILE__)
{
}

nes_sf3_device::nes_sf3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_SF3, "NES Cart Super Fighter III PCB", tag, owner, clock, "nes_sf3", __FILE__)
{
}

nes_gouder_device::nes_gouder_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_GOUDER, "NES Cart Gouder PCB", tag, owner, clock, "nes_gouder", __FILE__)
{
}

nes_sa9602b_device::nes_sa9602b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_SA9602B, "NES Cart SA-9602B PCB", tag, owner, clock, "nes_sa9602b", __FILE__)
{
}

nes_sachen_shero_device::nes_sachen_shero_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_SACHEN_SHERO, "NES Cart Street Hero PCB", tag, owner, clock, "nes_shero", __FILE__)
{
}

//nes_a9746_device::nes_a9746_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//                  : nes_txrom_device(mconfig, NES_A9746, "NES Cart A-9746 PCB", tag, owner, clock, "nes_bmc_a9746", __FILE__)
//{
//}

nes_fk23c_device::nes_fk23c_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_txrom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_fk23c_device::nes_fk23c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_FK23C, "NES Cart FK23C PCB", tag, owner, clock, "nes_fk23c", __FILE__)
{
}

nes_fk23ca_device::nes_fk23ca_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_fk23c_device(mconfig, NES_FK23CA, "NES Cart FK23CA PCB", tag, owner, clock, "nes_fk23ca", __FILE__)
{
}

nes_s24in1sc03_device::nes_s24in1sc03_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_S24IN1SC03, "NES Cart Super 24 in 1 SC-03 PCB", tag, owner, clock, "nes_s24in1c03", __FILE__)
{
}

nes_bmc_15in1_device::nes_bmc_15in1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_BMC_15IN1, "NES Cart BMC 15 in 1 PCB", tag, owner, clock, "nes_bmc_15in1", __FILE__)
{
}

nes_bmc_sbig7_device::nes_bmc_sbig7_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_BMC_SBIG7, "NES Cart BMC Super BIG 7 in 1 PCB", tag, owner, clock, "nes_bmc_sbit7", __FILE__)
{
}

nes_bmc_hik8_device::nes_bmc_hik8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_BMC_HIK8, "NES Cart BMC Super HIK 8 in 1 PCB", tag, owner, clock, "nes_bmc_hik8", __FILE__)
{
}

nes_bmc_hik4_device::nes_bmc_hik4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_BMC_HIK4, "NES Cart BMC Super HIK 4 in 1 PCB", tag, owner, clock, "nes_bmc_hik4", __FILE__)
{
}

nes_bmc_mario7in1_device::nes_bmc_mario7in1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_BMC_MARIO7IN1, "NES Cart BMC Mario 7 in 1 PCB", tag, owner, clock, "nes_bmc_mario7in1", __FILE__)
{
}

nes_bmc_gold7in1_device::nes_bmc_gold7in1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_BMC_GOLD7IN1, "NES Cart BMC Golden 7 in 1 PCB", tag, owner, clock, "nes_bmc_gold7in1", __FILE__)
{
}

nes_bmc_gc6in1_device::nes_bmc_gc6in1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_BMC_GC6IN1, "NES Cart BMC Golden Card 6 in 1 PCB", tag, owner, clock, "nes_bmc_gc6in1", __FILE__)
{
}

nes_bmc_411120c_device::nes_bmc_411120c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_BMC_411120C, "NES Cart BMC 411120C PCB", tag, owner, clock, "nes_bmc_411120c", __FILE__)
{
}

nes_bmc_830118c_device::nes_bmc_830118c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_BMC_830118C, "NES Cart BMC 830118C PCB", tag, owner, clock, "nes_bmc_830118c", __FILE__)
{
}

nes_pjoy84_device::nes_pjoy84_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_PJOY84, "NES Cart Powerjoy 84 PCB", tag, owner, clock, "nes_pjoy84", __FILE__)
{
}



void nes_family4646_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_pikay2k_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_pikay2k_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg[0] = 0xff;
	m_reg[1] = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_8237_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_cd_enable));
}

void nes_8237_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
	m_cd_enable = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_sglionk_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_reg_enabled));
}

void nes_sglionk_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg = m_reg_enabled = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_sgboog_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_mode));
}

void nes_sgboog_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_mode = 0x04;
	m_reg[0] = 0x00;
	m_reg[1] = 0xff;
	m_reg[2] = 0;
	mmc3_common_initialize(0x1f, 0xff, 0);
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_kasing_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_kasing_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}


void nes_kay_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_low_reg));
	save_item(NAME(m_reg));
}

void nes_kay_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	memset(m_reg, 0, sizeof(m_reg));
	m_low_reg = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}


void nes_h2288_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_h2288_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg[0] = 0;
	m_reg[1] = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_6035052_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_prot));
}

void nes_6035052_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0xff, 0xff, 0);

	m_prot = 0;
}

void nes_kof96_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_kof96_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	memset(m_reg, 0, sizeof(m_reg));
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_gouder_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_gouder_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	memset(m_reg, 0, sizeof(m_reg));
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_sa9602b_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_prg_chip));
}

void nes_sa9602b_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg = 0;
	m_prg_chip = 0;
	mmc3_common_initialize(0x1ff, 0xff, 0);    // 1.5MB of PRG-ROM, no CHR-ROM but 32K CHR-RAM
}

void nes_sachen_shero_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_sachen_shero_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg = 0;
	mmc3_common_initialize(0xff, 0x1ff, 0);
}

void nes_fk23c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_mmc_cmd1));
}

void nes_fk23c_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_mmc_cmd1 = 0;
	m_reg[0] = 4;
	m_reg[1] = 0xff;
	m_reg[2] = m_reg[3] = 0;
	m_reg[4] = m_reg[5] = m_reg[6] = m_reg[7] = 0xff;
	mmc3_common_initialize(0xff, 0xff, 0);
	fk23c_set_prg();
	fk23c_set_chr();

}

void nes_fk23ca_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_mmc_cmd1 = 0;
	m_reg[0] = m_reg[1] = m_reg[2] = m_reg[3] = 0;
	m_reg[4] = m_reg[5] = m_reg[6] = m_reg[7] = 0xff;
	mmc3_common_initialize(0xff, 0xff, 0);
	fk23c_set_prg();
	fk23c_set_chr();
}

void nes_s24in1sc03_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_s24in1sc03_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg[0] = 0x24;
	m_reg[1] = 0x9f;
	m_reg[2] = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_bmc_15in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	mmc3_common_initialize(0x1f, 0xff, 0);
	m_prg_base = 0x10;  // this board has a diff prg_base
	set_prg(m_prg_base, m_prg_mask);
}

void nes_bmc_sbig7_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_hik8_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_count));
}

void nes_bmc_hik8_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_count = 0;
	memset(m_reg, 0, sizeof(m_reg));
	mmc3_common_initialize(0x3f, 0xff, 0);
}

void nes_bmc_hik4_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_mario7in1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg_written));
}

void nes_bmc_mario7in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg_written = 0;
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_bmc_gold7in1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg_written));
}

void nes_bmc_gold7in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg_written = 0;
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_bmc_gc6in1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_gc6in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg[0] = 0x00;
	m_reg[1] = 0xff;
	m_reg[2] = 0x03;
	m_reg[3] = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_bmc_411120c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_411120c_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg = 0;
	mmc3_common_initialize(0x7f, 0x7f, 0);
}

void nes_bmc_830118c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_830118c_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg = 0;
	mmc3_common_initialize(0x7f, 0x7f, 0);
}

void nes_pjoy84_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_pjoy84_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	memset(m_reg, 0, sizeof(m_reg));
	set_base_mask();
	mmc3_common_initialize(m_prg_mask, m_chr_mask, 0);
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board by Nitra

 Games: Time Diver Avenger

 This acts basically like a MMC3 with different use of write
 address.

 iNES: mapper 250

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_nitra_device::write_h)
{
	LOG_MMC(("nitra write_h, offset: %04x, data: %02x\n", offset, data));

	txrom_write(space, (offset & 0x6000) | ((offset & 0x400) >> 10), offset & 0xff, mem_mask);
}

/*-------------------------------------------------

 Board UNL-KS7057

 Games: Street Fighter VI / Fight Street VI

 MMC3 clone (identical, but for switched address lines)

 In MESS: Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ks7057_device::write_h)
{
	LOG_MMC(("ks7057 write_h, offset: %04x, data: %02x\n", offset, data));

	offset = (BIT(offset, 0) << 1) | BIT(offset, 1) | (offset & ~0x03);
	txrom_write(space, offset, data, mem_mask);
}

/*-------------------------------------------------

 BTL-SUPERBROS11

 Games: Super Mario Bros. 11, Super Mario Bros. 17

 This acts basically like a MMC3 with different use of write
 address.

 iNES: mapper 196

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sbros11_device::write_h)
{
	LOG_MMC(("smb11 write_h, offset: %04x, data: %02x\n", offset, data));

	txrom_write(space, (offset & 0x6000) | ((offset & 0x04) >> 2), data, mem_mask);
}

/*-------------------------------------------------

 UNL-MALISB

 Games: Super Mali Bros Splash Bomb

 This is very similar to mapper 196, but with additional
 data bit swap.

 In MESS: Supported.

 -------------------------------------------------*/

void nes_malisb_device::prg_cb(int start, int bank)
{
	bank = (bank & 3) | ((bank & 8) >> 1) | ((bank & 4) << 1);
	prg8_x(start, bank);
}

void nes_malisb_device::chr_cb(int start, int bank, int source)
{
	bank = (bank & 0xdd) | ((bank & 0x20) >> 4) | ((bank & 2) << 4);
	chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_malisb_device::write_h)
{
	LOG_MMC(("malisb write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset > 0x4000)
		txrom_write(space, (offset & 0xfffe) | ((offset & 0x04) >> 2) | ((offset & 0x08) >> 3), data, mem_mask);
	else
		txrom_write(space, (offset & 0xfffe) | ((offset & 0x08) >> 3), data, mem_mask);
}

/*-------------------------------------------------

 BMC-FAMILY-4646B

 Known Boards: Unknown Multigame Bootleg Board (4646B)
 Games: 2 in 1 - Family Kid & Aladdin 4

 MMC3 clone

 iNES: mapper 134

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_family4646_device::write_m)
{
	LOG_MMC(("family4646 write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x01)
	{
		m_prg_base = (data & 0x02) << 4;
		m_prg_mask = 0x1f;
		m_chr_base = (data & 0x20) << 3;
		m_chr_mask = 0xff;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BTL-PIKACHUY2K

 Games: Pikachu Y2k

 iNES: mapper 254

 In MESS:

 -------------------------------------------------*/

WRITE8_MEMBER(nes_pikay2k_device::write_h)
{
	LOG_MMC(("pikay2k write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x2001:
			m_reg[1] = data;
			break;

		case 0x2000:
			m_reg[0] = 0;
		default:
			txrom_write(space, offset, data, mem_mask);
			break;
	}
}

// strange WRAM usage: it is protected at start, and gets unprotected after the first write to 0xa000
WRITE8_MEMBER(nes_pikay2k_device::write_m)
{
	LOG_MMC(("pikay2k write_m, offset: %04x, data: %02x\n", offset, data));

	m_prgram[offset & 0x1fff] = data;
}

READ8_MEMBER(nes_pikay2k_device::read_m)
{
	LOG_MMC(("pikay2k read_m, offset: %04x\n", offset));

	return m_prgram[offset & 0x1fff] ^ (m_reg[0] & m_reg[1]);
}

/*-------------------------------------------------

 Board UNL-8237

 Games: Pocahontas 2

 MMC3 clone

 In MESS: Supported

 -------------------------------------------------*/

void nes_8237_device::prg_cb(int start, int bank)
{
	if (!(m_reg[0] & 0x80))
		prg8_x(start, bank);
}

void nes_8237_device::chr_cb(int start, int bank, int source)
{
	bank |= ((m_reg[1] << 6) & 0x100);
	chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_8237_device::write_l)
{
	LOG_MMC(("unl_8237 write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1000)
	{
		m_reg[0] = data;
		if (m_reg[0] & 0x80)
		{
			if (m_reg[0] & 0x20)
				prg32((m_reg[0] & 0x0f) >> 1);
			else
			{
				prg16_89ab(m_reg[0] & 0x1f);
				prg16_cdef(m_reg[0] & 0x1f);
			}
		}
		else
			set_prg(m_prg_base, m_prg_mask);
	}

	if (offset == 0x1001)
	{
		m_reg[1] = data;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}

	if (offset == 0x1007)
	{
		m_reg[2] = data & 0x07; // this selects different permutations for addresses and regs (to be implemented)
	}
}

WRITE8_MEMBER(nes_8237_device::write_h)
{
	static const UINT8 conv_table[8] = {0, 2, 6, 1, 7, 3, 4, 5};
	LOG_MMC(("unl_8237 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x1000:
			set_nt_mirroring((data | (data >> 7)) & 0x01 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x2000:
		case 0x3000:
			m_cd_enable = 1;
			data = (data & 0xc0) | conv_table[data & 0x07];
			txrom_write(space, 0x0000, data, mem_mask);
			break;

		case 0x4000:
		case 0x5000:
			if (m_cd_enable)
			{
				m_cd_enable = 0;
				txrom_write(space, 0x0001, data, mem_mask);
			}
			break;

		case 0x6000:
			break;

		case 0x7000:
			txrom_write(space, 0x6001, data, mem_mask);
			txrom_write(space, 0x4000, data, mem_mask);
			txrom_write(space, 0x4001, data, mem_mask);
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board by Super Game

 Games: The Lion King

 MMC3 clone.

 iNES: mapper 114

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sglionk_device::write_m)
{
	LOG_MMC(("sglionk write_m, offset: %04x, data: %02x\n", offset, data));

	m_reg = data;

	if (m_reg & 0x80)
	{
		prg16_89ab(data & 0x1f);
		prg16_cdef(data & 0x1f);
	}
	else
		set_prg(m_prg_base, m_prg_mask);

}

WRITE8_MEMBER(nes_sglionk_device::write_h)
{
	static const UINT8 conv_table[8] = {0, 3, 1, 5, 6, 7, 2, 4};
	LOG_MMC(("sglionk write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x6000)
	{
		switch (offset & 0x6000)
		{
			case 0x0000:
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;
			case 0x2000:
				m_reg_enabled = 1;
				data = (data & 0xc0) | conv_table[data & 0x07];
				txrom_write(space, 0x0000, data, mem_mask);
				break;
			case 0x4000:
				if (m_reg_enabled && (m_reg & 0x80) == 0)
				{
					m_reg_enabled = 0;
					txrom_write(space, 0x0001, data, mem_mask);
				}
				break;
		}
	}
	else
	{
		switch (offset & 0x03)
		{
			case 0x02:
				txrom_write(space, 0x6000, data, mem_mask);
				break;
			case 0x03:
				txrom_write(space, 0x6001, data, mem_mask);
				txrom_write(space, 0x4000, data, mem_mask);
				txrom_write(space, 0x4001, data, mem_mask);
				break;
		}
	}
}

/*-------------------------------------------------

 Bootleg Board by Super Game

 Games: Boogerman, Mortal Kombat III

 MMC3 clone. Also, it probably needs a hack to support both
 variants (Boogerman & MK3).

 iNES: mapper 215

 In MESS: Preliminary support.

 -------------------------------------------------*/

void nes_sgboog_device::prg_cb(int start, int bank)
{
	if (!(m_reg[0] & 0x80))  // if this is != 0 we should never even arrive here
	{
		if (m_reg[1] & 0x08)
			bank = (bank & 0x1f) | 0x20;
		else
			bank = (bank & 0x0f) | (m_reg[1] & 0x10);

		prg8_x(start, bank);
	}
}

void nes_sgboog_device::chr_cb(int start, int bank, int source)
{
	if ((m_reg[1] & 0x04))
		bank |= 0x100;
	else
		bank = (bank & 0x7f) | ((m_reg[1] & 0x10) << 3);

	chr1_x(start, bank, source);
}

void nes_sgboog_device::set_prg(int prg_base, int prg_mask)
{
	if (m_reg[0] & 0x80)
	{
		prg16_89ab((m_reg[0] & 0xf0) | (m_reg[1] & 0x10));
		prg16_cdef((m_reg[0] & 0xf0) | (m_reg[1] & 0x10));
	}
	else
	{
		// here standard MMC3 bankswitch
		UINT8 prg_flip = (m_latch & 0x40) ? 2 : 0;

		prg_cb(0, prg_base | (m_mmc_prg_bank[0 ^ prg_flip] & prg_mask));
		prg_cb(1, prg_base | (m_mmc_prg_bank[1] & prg_mask));
		prg_cb(2, prg_base | (m_mmc_prg_bank[2 ^ prg_flip] & prg_mask));
		prg_cb(3, prg_base | (m_mmc_prg_bank[3] & prg_mask));
	}
}

WRITE8_MEMBER(nes_sgboog_device::write_l)
{
	LOG_MMC(("sgboog write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1000)
	{
		m_reg[0] = data;
		set_prg(m_prg_base, m_prg_mask);
	}
	else if (offset == 0x1001)
	{
		m_reg[1] = data;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else if (offset == 0x1007)
	{
		m_latch = 0;
		m_mode = data;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

WRITE8_MEMBER(nes_sgboog_device::write_m)
{
	LOG_MMC(("sgboog write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x0000)
	{
		m_reg[0] = data;
		set_prg(m_prg_base, m_prg_mask);
	}
	else if (offset == 0x0001)
	{
		m_reg[1] = data;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else if (offset == 0x0007)
	{
		m_latch = 0;
		m_mode = data;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

WRITE8_MEMBER(nes_sgboog_device::write_h)
{
	static const UINT8 conv_table[8] = {0,2,5,3,6,1,7,4};
	LOG_MMC(("sgboog write_h, offset: %04x, data: %02x\n", offset, data));

	if (m_mode)
	{
		switch (offset & 0x6001)
		{
			case 0x0000:
				break;

			case 0x0001:
				if (m_reg[2] && ((m_reg[0] & 0x80) == 0 || (m_latch & 0x07) < 6)) // if we use the prg16 banks and cmd=6,7 DON'T enter!
				{
					m_reg[2] = 0;
					txrom_write(space, 0x0001, data, mem_mask);
				}
				break;

			case 0x2000:
				data = (data & 0xc0) | conv_table[data & 0x07];
				m_reg[2] = 1;
				txrom_write(space, 0x0000, data, mem_mask);
				break;

			case 0x4000:
				set_nt_mirroring(((data >> 7) | data) & 0x01 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;

			case 0x4001:
				txrom_write(space, 0x6001, data, mem_mask);
				break;

			case 0x6001:
				txrom_write(space, 0x4000, data, mem_mask);
				txrom_write(space, 0x4001, data, mem_mask);
				break;

			default:
				txrom_write(space, offset, data, mem_mask);
				break;
		}
	}
	else
		txrom_write(space, offset, data, mem_mask);
}

/*-------------------------------------------------

 Bootleg Board by Kasing

 Games: AV Jiu Ji Mahjong, Bao Qing Tian, Thunderbolt 2,
 Shisen Mahjong 2

 MMC3 clone

 iNES: mapper 115

 In MESS: Supported

 -------------------------------------------------*/

void nes_kasing_device::prg_cb(int start, int bank)
{
	if (BIT(m_reg, 7))
		prg32(m_reg >> 1);
	else
		prg8_x(start, bank);
}

WRITE8_MEMBER(nes_kasing_device::write_m)
{
	LOG_MMC(("kasing write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x01)
	{
		case 0x00:
			m_reg = data;
			set_prg(m_prg_base, m_prg_mask);
			break;
		case 0x01:
			m_chr_base = (data & 0x01) ? 0x100 : 0x000;
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board by Kay (for Panda Prince)

 Games: The Panda Prince, Sonic 3d Blast 6, SFZ2 '97, YuYu '97
 (and its title hack MK6), UMK3, Super Lion King 2

 MMC3 clone. This is basically KOF96 board + protection

 iNES: mapper 121

 In MESS: Most game works, with some graphical issues.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_kay_device::write_l)
{
	LOG_MMC(("kay write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset >= 0x1000)
	{
		switch (data & 0x03)
		{
			case 0x00:
			case 0x01:
				m_low_reg = 0x83;
				break;
			case 0x02:
				m_low_reg = 0x42;
				break;
			case 0x03:
				m_low_reg = 0x00;
				break;
		}
	}
}

READ8_MEMBER(nes_kay_device::read_l)
{
	LOG_MMC(("kay read_l, offset: %04x\n", offset));
	offset += 0x100;

	if (offset >= 0x1000)
		return m_low_reg;
	else
		return 0xff;
}

void nes_kay_device::update_regs()
{
	switch (m_reg[5] & 0x3f)
	{
		case 0x20:
		case 0x29:
		case 0x2b:
		case 0x3f:
			m_reg[7] = 1;
			m_reg[1] = m_reg[6];
			break;
		case 0x26:
			m_reg[7] = 0;
			m_reg[1] = m_reg[6];
			break;
		case 0x2c:
			m_reg[7] = 1;
			if (m_reg[6])
				m_reg[1] = m_reg[6];
			break;

		case 0x28:
			m_reg[7] = 0;
			m_reg[2] = m_reg[6];
			break;

		case 0x2a:
			m_reg[7] = 0;
			m_reg[3] = m_reg[6];
			break;

		case 0x2f:
			break;

		default:
			m_reg[5] = 0;
			break;
	}
}

void nes_kay_device::prg_cb(int start, int bank)
{
	if (m_reg[5] & 0x3f)
	{
		prg8_x(start, bank & 0x3f);
		prg8_ef(m_reg[1]);
		prg8_cd(m_reg[2]);
		prg8_ab(m_reg[3]);
	}
	else
		prg8_x(start, bank & 0x3f);
}

void nes_kay_device::chr_cb(int start, int bank, int source)
{
	UINT8 chr_page = (m_latch & 0x80) >> 5;

	if ((start & 0x04) == chr_page)
		bank |= 0x100;

	chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_kay_device::write_h)
{
	LOG_MMC(("kay write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6003)
	{
		case 0x0000:
			txrom_write(space, offset, data, mem_mask);
			set_prg(m_prg_base, m_prg_mask);
			break;

		case 0x0001:
			m_reg[6] = (BIT(data, 0) << 5) | (BIT(data, 1) << 4) | (BIT(data, 2) << 3)
			| (BIT(data, 3) << 2) | (BIT(data, 4) << 1) | BIT(data, 5);
			if (!m_reg[7])
				update_regs();
			txrom_write(space, offset, data, mem_mask);
			set_prg(m_prg_base, m_prg_mask);
			break;

		case 0x0003:
			m_reg[5] = data;
			update_regs();
			txrom_write(space, 0x0000, data, mem_mask);
			set_prg(m_prg_base, m_prg_mask);
			break;

		default:
			txrom_write(space, offset, data, mem_mask);
			break;
	}
}

/*-------------------------------------------------

 UNL-H2288

 -------------------------------------------------*/

void nes_h2288_device::prg_cb(int start, int bank)
{
	if (!(m_reg[0] & 0x40))
		prg8_x(start, bank);
}

WRITE8_MEMBER(nes_h2288_device::write_l)
{
	LOG_MMC(("h2288 write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset >= 0x1800)
	{
		m_reg[offset & 1] = data;
		if (m_reg[0] & 0x40)
		{
			UINT8 helper1 = (m_reg[0] & 0x05) | ((m_reg[0] >> 2) & 0x0a);
			UINT8 helper2 = BIT(m_reg[0], 1);
			prg16_89ab(helper1 & ~helper2);
			prg16_cdef(helper1 |  helper2);
		}
		else
			set_prg(m_prg_base, m_prg_mask);
	}
}

READ8_MEMBER(nes_h2288_device::read_l)
{
	LOG_MMC(("h2288 read_l, offset: %04x\n", offset));
	offset += 0x100;

	if (offset >= 0x1000)
	{
		int helper = offset >> 8;
		if (offset & 1)
			return helper | 0x01;
		else
			return helper ^ 0x01;
	}

	return 0;
}

WRITE8_MEMBER(nes_h2288_device::write_h)
{
	static const UINT8 conv_table[8] = {0, 3, 1, 5, 6, 7, 2, 4};
	LOG_MMC(("h2288 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			txrom_write(space, 0x0000, (data & 0xc0) | conv_table[data & 0x07], mem_mask);
			break;

		default:
			txrom_write(space, offset, data, mem_mask);
			break;
	}
}

/*-------------------------------------------------

 UNL-603-5052

 MMC3 + protection access in 0x4020 - 0x7fff

 in MESS: Partial support

 -------------------------------------------------*/

WRITE8_MEMBER(nes_6035052_device::write_ex)
{
	LOG_MMC(("6035052 write_ex, offset: %04x, data: %02x\n", offset, data));
	m_prot = data & 0x03;
	if (m_prot == 1)
		m_prot = 2;
}

READ8_MEMBER(nes_6035052_device::read_ex)
{
	LOG_MMC(("6035052 read_ex, offset: %04x\n", offset));
	return m_prot;
}

/*-------------------------------------------------

 Bootleg Board 'Thunder Warrior' by TXC

 Games: Master Fighter II, Master Fighter 3, Thunder Warrior

 MMC3 clone

 iNES: mapper 189

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_txc_tw_device::write_l)
{
	LOG_MMC(("txc_tw write_l, offset: %04x, data: %02x\n", offset, data));

	prg32((data >> 4) | data);
}

/* writes to 0x8000-0xffff are like MMC3 but no PRG bankswitch (beacuse it is handled by low writes) */
void nes_txc_tw_device::prg_cb(int start, int bank)
{
	return;
}

/*-------------------------------------------------

 Board UNL-KOF97

 Games: King of Fighters 97 (Rex Soft)

 MMC3 clone

 In MESS: Not working

 -------------------------------------------------*/

inline UINT8 kof97_unscramble( UINT8 data )
{
	return ((data >> 1) & 0x01) | ((data >> 4) & 0x02) | ((data << 2) & 0x04) | ((data >> 0) & 0xd8) | ((data << 3) & 0x20);
}

WRITE8_MEMBER(nes_kof97_device::write_h)
{
	LOG_MMC(("kof97 write_h, offset: %04x, data: %02x\n", offset, data));

	/* Addresses 0x9000, 0xa000, 0xd000 & 0xf000 behaves differently than MMC3 */
	if (offset == 0x1000)
		txrom_write(space, 0x0001, kof97_unscramble(data), mem_mask);
	else if (offset == 0x2000)
		txrom_write(space, 0x0000, kof97_unscramble(data), mem_mask);
	else if (offset == 0x5000)
		txrom_write(space, 0x4001, kof97_unscramble(data), mem_mask);
	else if (offset == 0x7000)
		txrom_write(space, 0x6001, kof97_unscramble(data), mem_mask);
	/* Other addresses behaves like MMC3, up to unscrambling data */
	else
		txrom_write(space, offset, kof97_unscramble(data), mem_mask);
}

/*-------------------------------------------------

 Bootleg Board for KOF96

 Games: The King of Fighters 96, Sonic 3D Blast 6, Street
 Fighter Zero 2

 MMC3 clone

 iNES: mapper 187

 In MESS: Preliminary Support.

 -------------------------------------------------*/

void nes_kof96_device::prg_cb(int start, int bank)
{
	if (!(m_reg[0] & 0x80))
		prg8_x(start, bank);
}

void nes_kof96_device::chr_cb(int start, int bank, int source)
{
	UINT8 chr_page = (m_latch & 0x80) >> 5;

	if ((start & 0x04) == chr_page)
		bank |= 0x100;

	chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_kof96_device::write_l)
{
	UINT8 new_bank;
	LOG_MMC(("kof96 write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1000)
	{
		m_reg[0] = data;

		if (m_reg[0] & 0x80)
		{
			new_bank = (m_reg[0] & 0x1f);

			if (m_reg[0] & 0x20)
				prg32(new_bank >> 2);
			else
			{
				prg16_89ab(new_bank);
				prg16_cdef(new_bank);
			}
		}
		else
			set_prg(m_prg_base, m_prg_mask);
	}

	if (offset >= 0x1000)
	{
		switch (data & 0x03)
		{
			case 0x00:
			case 0x01:
				m_reg[1] = 0x83;
				break;
			case 0x02:
				m_reg[1] = 0x42;
				break;
			case 0x03:
				m_reg[1] = 0x00;
				break;
		}

	}

	if (!m_reg[3] && offset > 0x1000)
	{
		m_reg[3] = 1;
		space.write_byte(0x4017, 0x40);
	}
}

READ8_MEMBER(nes_kof96_device::read_l)
{
	LOG_MMC(("kof96 read_l, offset: %04x\n", offset));
	offset += 0x100;

	if (!(offset < 0x1000))
		return m_reg[1];
	else
		return 0;
}

WRITE8_MEMBER(nes_kof96_device::write_h)
{
	LOG_MMC(("kof96 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6003)
	{
		case 0x0000:
			m_reg[2] = 1;
			txrom_write(space, 0x0000, data, mem_mask);
			break;

		case 0x0001:
			if (m_reg[2])
				txrom_write(space, 0x0001, data, mem_mask);
			break;

		case 0x0002:
			break;

		case 0x0003:
			m_reg[2] = 0;

			if (data == 0x28)
				prg8_cd(0x17);
			else if (data == 0x2a)
				prg8_ab(0x0f);
			break;

		default:
			txrom_write(space, offset, data, mem_mask);
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board for Super Fighter III

 MMC3 clone

 iNES: mapper 197

 In MESS: Supported.

 -------------------------------------------------*/

void nes_sf3_device::set_chr(UINT8 chr_source, int chr_base, int chr_mask)
{
	chr4_0(chr_base | ((m_mmc_vrom_bank[0] >> 1) & chr_mask), chr_source);
	chr2_4(chr_base | (m_mmc_vrom_bank[1] & chr_mask), chr_source);
	chr2_6(chr_base | (m_mmc_vrom_bank[2] & chr_mask), chr_source);
}

WRITE8_MEMBER(nes_sf3_device::write_h)
{
	UINT8 cmd;
	LOG_MMC(("sf3 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0001:
			cmd = m_latch & 0x07;
			switch (cmd)
			{
				case 0: case 2: case 4:
					m_mmc_vrom_bank[cmd >> 1] = data;
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
			txrom_write(space, offset, data, mem_mask);
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board 37017 (?) by Gouder

 Games: Street Fighter IV

 MMC3 clone. It also requires reads from 0x5000-0x7fff.

 iNES: mapper 208

 In MESS: Preliminary Support.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_gouder_device::write_l)
{
	static const UINT8 conv_table[256] =
	{
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x49,0x19,0x09,0x59,0x49,0x19,0x09,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x51,0x41,0x11,0x01,0x51,0x41,0x11,0x01,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x49,0x19,0x09,0x59,0x49,0x19,0x09,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x51,0x41,0x11,0x01,0x51,0x41,0x11,0x01,
		0x00,0x10,0x40,0x50,0x00,0x10,0x40,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x08,0x18,0x48,0x58,0x08,0x18,0x48,0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x10,0x40,0x50,0x00,0x10,0x40,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x08,0x18,0x48,0x58,0x08,0x18,0x48,0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x58,0x48,0x18,0x08,0x58,0x48,0x18,0x08,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x50,0x40,0x10,0x00,0x50,0x40,0x10,0x00,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x58,0x48,0x18,0x08,0x58,0x48,0x18,0x08,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x50,0x40,0x10,0x00,0x50,0x40,0x10,0x00,
		0x01,0x11,0x41,0x51,0x01,0x11,0x41,0x51,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x09,0x19,0x49,0x59,0x09,0x19,0x49,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x11,0x41,0x51,0x01,0x11,0x41,0x51,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x09,0x19,0x49,0x59,0x09,0x19,0x49,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	LOG_MMC(("gouder write_l, offset: %04x, data: %02x\n", offset, data));

	if (!(offset < 0x1700))
		m_reg[offset & 0x03] = data ^ conv_table[m_reg[4]];
	else if (!(offset < 0xf00))
		m_reg[4] = data;
	else if (!(offset < 0x700))
		prg32(((data >> 3) & 0x02) | (data & 0x01));
}

READ8_MEMBER(nes_gouder_device::read_l)
{
	LOG_MMC(("gouder read_l, offset: %04x\n", offset));

	if (!(offset < 0x1700))
		return m_reg[offset & 0x03];

	return 0x00;
}

/* writes to 0x8000-0xffff are like MMC3 but no PRG bankswitch (beacuse it is handled by low writes) */
void nes_gouder_device::prg_cb(int start, int bank)
{
	return;
}


/*-------------------------------------------------

 UNL-SA-9602B

 Sachen boards used for a chinese port of Princess Maker (?)

 in MESS: Very Preliminary support, based on Cah4e3
 code in FCEUMM

 -------------------------------------------------*/

void nes_sa9602b_device::prg_cb(int start, int bank)
{
	if (m_prg_chip == 3)
	{
		printf("Accessing PRG beyond end of ROM!\n");
		m_prg_chip = 2; // assume that the higher PRG chip is mirrored twice
	}
	prg8_x(start, (m_prg_chip * 0x40) | (bank & 0x3f));

	if (m_latch & 0x40)
		prg8_89(0x3e);
	else
		prg8_cd(0x3e);

	prg8_ef(0x3f);
//  printf("start %d, bank %d\n", start, bank);
}


WRITE8_MEMBER( nes_sa9602b_device::write_h )
{
	LOG_MMC(("sa9602b write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			m_reg = data;
			break;
		case 0x0001:
			if ((m_reg & 7) < 6)
				m_prg_chip = (data & 0xc0) >> 6;
			set_prg(0, m_prg_mask);
			break;
	}

	txrom_write(space, offset, data, mem_mask);
}

/*-------------------------------------------------

 UNL-SHERO

 Sachen boards used for Street Heroes

 in MESS: Very Preliminary support

 -------------------------------------------------*/


void nes_sachen_shero_device::chr_cb( int start, int bank, int source )
{
	int shift = start < 2 ? 5 :
				start < 4 ? 6 :
				start < 6 ? 8 : 7;
	if (!BIT(m_reg, 6))
		chr1_x(start, ((m_reg << shift) & 0x100) | bank, source);
}


WRITE8_MEMBER( nes_sachen_shero_device::write_l )
{
	LOG_MMC(("shero write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x4100;

	if (offset == 0x4100)
	{
		m_reg = data;
		if (BIT(m_reg, 6))
			chr8(0, CHRRAM);
		else
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

READ8_MEMBER( nes_sachen_shero_device::read_l )
{
	LOG_MMC(("shero read_l, offset: %04x\n", offset));
	offset += 0x4100;

	if (offset == 0x4100)
	{
		// DSW read!
	}
	return m_open_bus;  // open bus
}



/*-------------------------------------------------

 MULTIGAME CARTS BASED ON MMC3

 -------------------------------------------------*/

	/*-------------------------------------------------

	Board BMC-FK23C

	MMC3 clone

	In MESS: partially supported (still to sort initial banking
	for many games)

	-------------------------------------------------*/

#if 0
// alt version...
void nes_fk23c_device::prg_cb(int start, int bank)
{
	if (((m_reg[0] & 0x07) - 3) > 1 && (!(m_reg[3] & 0x02) || start < 2))
	{
		if (m_reg[0] & 0x03)
			bank = (bank & (0x3f >> (m_reg[0] & 0x03))) | (m_reg[1] << 1);

		prg8_x(start, bank);
	}
}

void nes_fk23c_device::chr_cb(int start, int bank, int source)
{
	if (!(m_reg[0] & 0x40) && (!(m_reg[3] & 0x02) || (start != 1 && start != 3)))
		chr1_x(start, ((m_reg[2] & 0x7f) << 3) | bank, source);
}

#endif

void nes_fk23c_device::prg_cb(int start, int bank)
{
	UINT8 mask = (0x3f >> (m_reg[0] & 0x03));

	if ((m_reg[0] & 0x07) < 3)
	{
		if (!(m_reg[0] & 0x03))
			bank = (bank & mask) | ((m_reg[1]  & (0x7f ^ mask)) << 1);

		prg8_x(start, bank);
	}
}

void nes_fk23c_device::chr_cb(int start, int bank, int source)
{
	if (!(m_reg[0] & 0x40) && (!(m_reg[3] & 0x02) || (start != 1 && start != 3)))
		chr1_x(start, ((m_reg[2] & 0x7f) << 3) | bank, source);
}

void nes_fk23c_device::fk23c_set_prg()
{
	if ((m_reg[0] & 0x07) == 4)
		prg32((m_reg[1] & 0x7f) >> 1);
	else if ((m_reg[0] & 0x07) == 3)
	{
		prg16_89ab(m_reg[1] & 0x7f);
		prg16_cdef(m_reg[1] & 0x7f);
	}
	else
	{
		if (m_reg[3] & 0x02)
		{
			prg8_cd(m_reg[4]);
			prg8_ef(m_reg[5]);
		}
		else
			set_prg(m_prg_base, m_prg_mask);
	}
}

void nes_fk23c_device::fk23c_set_chr()
{
	if (m_reg[0] & 0x40)
		chr8(m_reg[2] | m_mmc_cmd1, m_chr_source);
	else
	{
		if (m_reg[3] & 0x02)
		{
			int base = (m_reg[2] & 0x7f) << 3;
			chr1_x(1, base | m_reg[6], m_chr_source);
			chr1_x(3, base | m_reg[7], m_chr_source);
		}
		else
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

WRITE8_MEMBER(nes_fk23c_device::write_l)
{
	LOG_MMC(("fk23c write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset >= 0x1000)
	{
		if (offset & (1 << 4))  // here it should be (4 + m_mmc_dipsetting)
		{
			m_reg[offset & 0x03] = data;

			fk23c_set_prg();
			fk23c_set_chr();
		}
	}
}

WRITE8_MEMBER(nes_fk23c_device::write_h)
{
	LOG_MMC(("fk23c write_h, offset: %04x, data: %02x\n", offset, data));

	if (m_reg[0] & 0x40)
	{
		if (m_reg[0] & 0x30)
			m_mmc_cmd1 = 0;
		else
		{
			m_mmc_cmd1 = data & 0x03;
			fk23c_set_chr();
		}
	}
	else
	{
		switch (offset & 0x6001)
		{
			case 0x0001:
				if ((m_reg[3] & 0x02) && (m_latch & 0x08))
				{
					m_reg[4 | (m_latch & 0x03)] = data;
					fk23c_set_prg();
					fk23c_set_chr();
				}
				else
					txrom_write(space, offset, data, mem_mask);
				break;

			case 0x2000:
				set_nt_mirroring(data ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;

			default:
				txrom_write(space, offset, data, mem_mask);
				break;
		}
	}
}

/*-------------------------------------------------

 Board BMC-SUPER24IN1SC03

 Games: Super 24-in-1

 In MESS: Partially Supported

 -------------------------------------------------*/

void nes_s24in1sc03_device::prg_cb(int start, int bank)
{
	static const UINT8 masks[8] = {0x3f, 0x1f, 0x0f, 0x01, 0x03, 0x00, 0x00, 0x00};
	int prg_base = m_reg[1] << 1;
	int prg_mask = masks[m_reg[0] & 0x07];

	bank = prg_base | (bank & prg_mask);
	prg8_x(start, bank);
}

void nes_s24in1sc03_device::chr_cb(int start, int bank, int source)
{
	UINT8 chr = BIT(m_reg[0], 5) ? CHRRAM : CHRROM;
	int chr_base = (m_reg[2] << 3) & 0xf00;

	chr1_x(start, chr_base | bank, chr);
}

WRITE8_MEMBER(nes_s24in1sc03_device::write_l)
{
	LOG_MMC(("s24in1sc03 write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1ff0)
	{
		m_reg[0] = data;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
		set_prg(m_prg_base, m_prg_mask);
	}

	if (offset == 0x1ff1)
	{
		m_reg[1] = data;
		set_prg(m_prg_base, m_prg_mask);
	}

	if (offset == 0x1ff2)
	{
		m_reg[2] = data;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-15IN1

 Unknown Bootleg Multigame Board
 Games: 3 in 1, 15 in 1

 iNES: mapper 205, MMC3 clone

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_15in1_device::write_m)
{
	LOG_MMC(("bmc_15in1 write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x0800)
	{
		m_prg_base = (data & 0x03) << 4;
		m_prg_mask = (data & 0x02) ? 0x0f : 0x1f;
		m_chr_base = (data & 0x03) << 7;
		m_chr_mask = (data & 0x02) ? 0x7f : 0xff;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-SUPERBIG-7IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Kunio 8 in 1, Super Big 7 in 1

 iNES: mapper 44

 In MESS: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_sbig7_device::write_h)
{
	UINT8 page;
	LOG_MMC(("bmc_sbig7 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x2001: /* $a001 - Select 128K ROM/VROM base (0..5) or last 256K ROM/VRAM base (6) */
			page = (data & 0x07);
			if (page > 6)
				page = 6;

			m_prg_base = page << 4;
			m_prg_mask = (page > 5) ? 0x1f : 0x0f;
			m_chr_base = page << 7;
			m_chr_mask = (page > 5) ? 0xff : 0x7f;
			set_prg(m_prg_base, m_prg_mask);
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;

		default:
			txrom_write(space, offset, data, mem_mask);
			break;
	}
}

/*-------------------------------------------------

 BMC-HIK8IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Street Fighter V, various multigame carts

 iNES: mapper 45

 In MESS: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_hik8_device::write_m)
{
	LOG_MMC(("bmc_hik8 write_m, offset: %04x, data: %02x\n", offset, data));

	/* This bit is the "register lock". Once register are locked, writes go to WRAM
	 and there is no way to unlock them (except by resetting the machine) */
	if ((m_reg[3] & 0x40) && !m_prgram.empty())
		m_prgram[offset] = data;
	else
	{
		m_reg[m_count] = data;
		m_count = (m_count + 1) & 0x03;

		if (!m_count)
		{
			LOG_MMC(("bmc_hik8 write_m, command completed %02x %02x %02x %02x\n", m_reg[3],
						m_reg[2], m_reg[1], m_reg[0]));

			m_prg_base = m_reg[1];
			m_prg_mask = 0x3f ^ (m_reg[3] & 0x3f);
			m_chr_base = ((m_reg[2] & 0xf0) << 4) | m_reg[0];
			if (BIT(m_reg[2], 3))
				m_chr_mask = (1 << ((m_reg[2] & 7) + 1)) - 1;
			else if (m_reg[2])
				m_chr_mask = 0;
			else
				m_chr_mask = 0xff;  // i.e. we use the vrom_bank with no masking

			set_prg(m_prg_base, m_prg_mask);
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
		}
	}
}

/*-------------------------------------------------

 BMC-SUPERHIK-4IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Super HIK 4 in 1

 iNES: mapper 49

 In MESS: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_hik4_device::write_m)
{
	LOG_MMC(("bmc_hik4 write_m, offset: %04x, data: %02x\n", offset, data));

	/* mid writes only work when WRAM is enabled. not sure if I should
	 change the condition to m_mmc_latch2==0x80 (i.e. what is the effect of
	 the read-only bit?) */
	if (m_wram_protect & 0x80)
	{
		if (data & 0x01)    /* if this is 0, then we have 32k PRG blocks */
		{
			m_prg_base = (data & 0xc0) >> 2;
			m_prg_mask = 0x0f;
			set_prg(m_prg_base, m_prg_mask);
		}
		else
			prg32((data & 0x30) >> 4);

		m_chr_base = (data & 0xc0) << 1;
		m_chr_mask = 0x7f;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-MARIOPARTY-7IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Mario 7 in 1

 MMC3 clone

 iNES: mapper 52

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_mario7in1_device::write_m)
{
	UINT8 helper1, helper2;
	LOG_MMC(("bmc_mario7in1 write_m, offset: %04x, data: %02x\n", offset, data));

	/* mid writes only work when WRAM is enabled. not sure if I should
	 change the condition to m_map52_reg_written == 0x80 (i.e. what is the effect of
	 the read-only bit?) and it only can happen once! */
	if ((m_wram_protect & 0x80) && !m_reg_written)
	{
		helper1 = (data & 0x08);
		helper2 = (data & 0x40);

		m_prg_base = helper1 ? ((data & 0x07) << 4) : ((data & 0x06) << 4);
		m_prg_mask = helper1 ? 0x0f : 0x1f;
		m_chr_base = ((data & 0x20) << 4) | ((data & 0x04) << 6) | (helper2 ? ((data & 0x10) << 3) : 0);
		m_chr_mask = helper2 ? 0x7f : 0xff;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		m_reg_written = 1;
	}
	else
		m_prgram[offset] = data;
}

/*-------------------------------------------------

 BMC-GOLD-7IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Super HIK Gold 7 in 1, Golden 7 in 1 and many more

 MMC3 clone, same as BMC-MARIOPARTY-7IN1 but with switched CHR
 bank lines

 iNES: mapper 52

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_gold7in1_device::write_m)
{
	UINT8 helper1, helper2;
	LOG_MMC(("bmc_gold7in1 write_m, offset: %04x, data: %02x\n", offset, data));

	if ((m_wram_protect & 0x80) && !m_reg_written)
	{
		helper1 = (data & 0x08);
		helper2 = (data & 0x40);

		m_prg_base = helper1 ? ((data & 0x07) << 4) : ((data & 0x06) << 4);
		m_prg_mask = helper1 ? 0x0f : 0x1f;
		m_chr_base = ((data & 0x20) << 3) | ((data & 0x04) << 7) | (helper2 ? ((data & 0x10) << 3) : 0);
		m_chr_mask = helper2 ? 0x7f : 0xff;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		m_reg_written = BIT(data, 7); // mc_2hikg & mc_s3nt3 write here multiple time
	}
	else
		m_prgram[offset] = data;
}

/*-------------------------------------------------

 BMC-GOLDENCARD-6IN1

 Known Boards: Unknown Bootleg Multigame Board
 Games: Golden Card 6 in 1

 MMC3 clone

 iNES: mapper 217

 In MESS: Supported.

 -------------------------------------------------*/

void nes_bmc_gc6in1_device::prg_cb(int start, int bank)
{
	if (m_reg[1] & 0x08)
		bank &= 0x1f;
	else
	{
		bank &= 0x0f;
		bank |= m_reg[1] & 0x10;
	}

	prg8_x(start, bank | ((m_reg[1] & 0x03) << 5));
}

void nes_bmc_gc6in1_device::chr_cb(int start, int bank, int source)
{
	if (!(m_reg[1] & 0x08))
		bank = ((m_reg[1] & 0x10) << 3) | (bank & 0x7f);


	bank |= ((m_reg[1] << 6) & 0x100);
	chr1_x(start, ((m_reg[1] & 0x03) << 8) | bank, source);
}

WRITE8_MEMBER(nes_bmc_gc6in1_device::write_l)
{
	UINT8 bank;
	LOG_MMC(("bmc_gc6in1 write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1000)
	{
		m_reg[0] = data;
		if (data & 0x80)
		{
			bank = (data & 0x0f) | ((m_reg[1] & 0x03) << 4);
			prg16_89ab(bank);
			prg16_cdef(bank);
		}
		else
			set_prg(m_prg_base, m_prg_mask);
	}
	else if (offset == 0x1001)
	{
		m_reg[1] = data;
		set_prg(m_prg_base, m_prg_mask);
	}
	else if (offset == 0x1007)
	{
		m_reg[2] = data;
	}
}

WRITE8_MEMBER(nes_bmc_gc6in1_device::write_h)
{
	UINT8 cmd;
	static const UINT8 conv_table[8] = {0, 6, 3, 7, 5, 2, 4, 1};
	LOG_MMC(("bmc_gc6in1 write_h, offset: %04x, data: %02x\n", offset, data));

	if (!m_reg[2])
	{
		// in this case we act like MMC3, only with alt prg/chr handlers
		txrom_write(space, offset, data, mem_mask);
	}
	else
	{
		switch (offset & 0x6001)
		{
			case 0x0000:
				txrom_write(space, 0x4000, data, mem_mask);
				break;

			case 0x0001:
				data = (data & 0xc0) | conv_table[data & 0x07];
				m_reg[3] = 1;
				txrom_write(space, 0x0000, data, mem_mask);
				break;

			case 0x2000:
				cmd = m_latch & 0x07;
				if (m_reg[3])
				{
					m_reg[3] = 0;
					switch (cmd)
					{
						case 0: case 1: // these do not need to be separated: we take care of them in set_chr!
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
				}
				break;


			case 0x2001:
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;

			default:
				txrom_write(space, offset, data, mem_mask);
				break;
		}
	}
}

/*-------------------------------------------------

 BMC-411120C


 MMC3 clone


 In MESS: Very Preliminary Support

 -------------------------------------------------*/

void nes_bmc_411120c_device::prg_cb(int start, int bank)
{
	if (m_reg & 8)  // & 0xc when DSW change (diff menu?)
		prg32(((m_reg >> 4) & 3) | 0x0c);
	else
		prg8_x(start, (bank & 0x0f) | ((m_reg & 0x03) << 4));
}

void nes_bmc_411120c_device::chr_cb(int start, int bank, int source)
{
	chr1_x(start, bank | ((m_reg & 3) << 7), source);
}

WRITE8_MEMBER(nes_bmc_411120c_device::write_m)
{
	LOG_MMC(("bmc_411120c write_m, offset: %04x, data: %02x\n", offset, data));

	m_reg = data;
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/*-------------------------------------------------

 BMC-830118C


 MMC3 clone


 In MESS: Very Preliminary Support

 -------------------------------------------------*/

void nes_bmc_830118c_device::prg_cb(int start, int bank)
{
	if ((m_reg & 0x0c) != 0x0c)
		prg8_x(start, (bank & 0x0f) | ((m_reg & 0x0c) << 2));
	else
	{
		if (start == 0)
		{
			prg8_89((bank & 0x0f) | ((m_reg & 0x0c) << 2));
			prg8_ab((bank & 0x0f) | 0x20);
		}
		else if (start == 2)
		{
			prg8_cd((bank & 0x0f) | ((m_reg & 0x0c) << 2));
			prg8_ef((bank & 0x0f) | 0x20);
		}
	}
}

void nes_bmc_830118c_device::chr_cb(int start, int bank, int source)
{
	chr1_x(start, (bank & 0x7f) | ((m_reg & 0x0c) << 5), source);
}

WRITE8_MEMBER(nes_bmc_830118c_device::write_m)
{
	LOG_MMC(("bmc_830118c write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x800 && offset < 0x900)
	{
		m_reg = data;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-POWERJOY

 -------------------------------------------------*/

void nes_pjoy84_device::prg_cb(int start, int bank)
{
	UINT8 flip = (m_latch & 0x40) ? 2 : 0;

	if (!(m_reg[3] & 0x03))
		prg8_x(start, bank);
	else if (start == flip)
	{
		if ((m_reg[3] & 0x03) == 0x03)
			prg32(bank >> 2);
		else
		{
			prg16_89ab(bank >> 1);
			prg16_cdef(bank >> 1);
		}
	}
}

void nes_pjoy84_device::chr_cb(int start, int bank, int source)
{
	if (!(m_reg[3] & 0x10))
		chr1_x(start, bank, source);
}

inline void nes_pjoy84_device::set_base_mask()
{
	m_prg_base = ((m_reg[0] & (0x06 | BIT(m_reg[0], 6))) << 4) |
	(BIT(m_reg[0], 4) << 7);

	m_chr_base = ((~m_reg[0] << 0) & 0x080 & m_reg[2]) |
	((m_reg[0] << 4) & 0x080 & m_reg[0]) |
	((m_reg[0] << 3) & 0x100) |
	((m_reg[0] << 5) & 0x200);

	m_prg_mask = BIT(m_reg[0], 6) ? 0x0f : 0x1f;
	m_chr_mask = BIT(m_reg[0], 7) ? 0x7f : 0xff;
}

WRITE8_MEMBER(nes_pjoy84_device::write_m)
{
	LOG_MMC(("pjoy84 write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x03)
	{
		case 0x00:
		case 0x03:
			if (m_reg[3] & 0x80)
				return; // else we act as if offset & 3 = 1,2
		case 0x01:
		case 0x02:
			m_reg[offset & 0x03] = data;
			set_base_mask();
			if (m_reg[3] & 0x10)
				chr8((m_chr_base >> 3) | (m_reg[2] & 0x0f), m_chr_source);
			else
				set_chr(m_chr_source, m_chr_base, m_chr_mask);
			set_prg(m_prg_base, m_prg_mask);
			break;
	}
}

#ifdef UNUSED_FUNCTION
/*-------------------------------------------------

 UNL-A9746


 MMC3 clone


 Preliminary emulation based on Cah4e3's code
 No dump is available (yet) for this.

 -------------------------------------------------*/

void nes_a9746_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_a9746_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;

	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
	mmc3_common_initialize(0x7f, 0xff, 0);
}

void nes_a9746_device::update_banks(UINT8 value)
{
	UINT8 bank = BITSWAP8(value & 0x3c,7,6,0,1,2,3,4,5);

	switch (m_reg[0])
	{
		case 0x26: prg8_89(bank); break;
		case 0x25: prg8_ab(bank); break;
		case 0x24: prg8_cd(bank); break;
		case 0x23: prg8_ef(bank); break;
	}

	switch (m_reg[1])
	{
		case 0x08: case 0x0a: case 0x0c: case 0x0e:
		case 0x10: case 0x12: case 0x14: case 0x16:
		case 0x18: case 0x1a: case 0x1c: case 0x1e:
			m_reg[2] = (value << 4);
			break;
		case 0x09: chr1_0(m_reg[2] | (value >> 1), m_chr_source); break;
		case 0x0b: chr1_1(m_reg[2] | (value >> 1) | 1, m_chr_source);  break;
		case 0x0d: chr1_2(m_reg[2] | (value >> 1), m_chr_source);  break;
		case 0x0f: chr1_3(m_reg[2] | (value >> 1) | 1, m_chr_source);  break;
		case 0x11: chr1_4(m_reg[2] | (value >> 1), m_chr_source); break;
		case 0x15: chr1_5(m_reg[2] | (value >> 1), m_chr_source);  break;
		case 0x19: chr1_6(m_reg[2] | (value >> 1), m_chr_source);  break;
		case 0x1d: chr1_7(m_reg[2] | (value >> 1), m_chr_source);  break;
	}
}

WRITE8_MEMBER(nes_a9746_device::write_h)
{
	LOG_MMC(("unl_a9746 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6003)
	{
		case 0x0000:
			m_reg[1] = data;
			m_reg[0] = 0;
			break;
		case 0x0001:
			update_banks(data);
			break;
		case 0x0002:
			m_reg[0] = data;
			m_reg[1] = 0;
			break;

		case 0x0003:
		case 0x2000:
		case 0x2001:
		case 0x2002:
		case 0x2003:
			break;

		default:
			txrom_write(space, offset, data, mem_mask);
			break;
	}
}
#endif
