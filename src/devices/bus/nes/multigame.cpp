// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Multigame Carts PCBs


 Here we emulate several PCBs used in multigame pirate carts (not MMC-3 based)

 ***********************************************************************************************************/


#include "emu.h"
#include "multigame.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_ACTION52 = &device_creator<nes_action52_device>;
const device_type NES_CALTRON6IN1 = &device_creator<nes_caltron_device>;
const device_type NES_RUMBLESTATION = &device_creator<nes_rumblestat_device>;
const device_type NES_SVISION16 = &device_creator<nes_svision16_device>;
const device_type NES_N625092 = &device_creator<nes_n625092_device>;
const device_type NES_A65AS = &device_creator<nes_a65as_device>;
const device_type NES_T262 = &device_creator<nes_t262_device>;
const device_type NES_NOVEL1 = &device_creator<nes_novel1_device>;
const device_type NES_NOVEL2 = &device_creator<nes_novel2_device>;
const device_type NES_STUDYNGAME = &device_creator<nes_studyngame_device>;
const device_type NES_SUPERGUN20IN1 = &device_creator<nes_sgun20in1_device>;
const device_type NES_VT5201 = &device_creator<nes_vt5201_device>;
const device_type NES_810544C = &device_creator<nes_810544c_device>;
const device_type NES_NTD03 = &device_creator<nes_ntd03_device>;
const device_type NES_BMC_GB63 = &device_creator<nes_bmc_gb63_device>;
const device_type NES_BMC_GKA = &device_creator<nes_bmc_gka_device>;
const device_type NES_BMC_GKB = &device_creator<nes_bmc_gkb_device>;
const device_type NES_BMC_WS = &device_creator<nes_bmc_ws_device>;
const device_type NES_BMC_11160 = &device_creator<nes_bmc_11160_device>;
const device_type NES_BMC_G146 = &device_creator<nes_bmc_g146_device>;
const device_type NES_BMC_8157 = &device_creator<nes_bmc_8157_device>;
const device_type NES_BMC_HIK300 = &device_creator<nes_bmc_hik300_device>;
const device_type NES_BMC_S700 = &device_creator<nes_bmc_s700_device>;
const device_type NES_BMC_BALL11 = &device_creator<nes_bmc_ball11_device>;
const device_type NES_BMC_22GAMES = &device_creator<nes_bmc_22games_device>;
const device_type NES_BMC_64Y2K = &device_creator<nes_bmc_64y2k_device>;
const device_type NES_BMC_12IN1 = &device_creator<nes_bmc_12in1_device>;
const device_type NES_BMC_20IN1 = &device_creator<nes_bmc_20in1_device>;
const device_type NES_BMC_21IN1 = &device_creator<nes_bmc_21in1_device>;
const device_type NES_BMC_31IN1 = &device_creator<nes_bmc_31in1_device>;
const device_type NES_BMC_35IN1 = &device_creator<nes_bmc_35in1_device>;
const device_type NES_BMC_36IN1 = &device_creator<nes_bmc_36in1_device>;
const device_type NES_BMC_64IN1 = &device_creator<nes_bmc_64in1_device>;
const device_type NES_BMC_70IN1 = &device_creator<nes_bmc_70in1_device>;
const device_type NES_BMC_72IN1 = &device_creator<nes_bmc_72in1_device>;
const device_type NES_BMC_76IN1 = &device_creator<nes_bmc_76in1_device>;
const device_type NES_BMC_110IN1 = &device_creator<nes_bmc_110in1_device>;
const device_type NES_BMC_150IN1 = &device_creator<nes_bmc_150in1_device>;
const device_type NES_BMC_190IN1 = &device_creator<nes_bmc_190in1_device>;
const device_type NES_BMC_800IN1 = &device_creator<nes_bmc_800in1_device>;
const device_type NES_BMC_1200IN1 = &device_creator<nes_bmc_1200in1_device>;
const device_type NES_BMC_GOLD150 = &device_creator<nes_bmc_gold150_device>;
const device_type NES_BMC_GOLD260 = &device_creator<nes_bmc_gold260_device>;
const device_type NES_BMC_CH001 = &device_creator<nes_bmc_ch001_device>;
const device_type NES_BMC_SUPER22 = &device_creator<nes_bmc_super22_device>;
const device_type NES_BMC_4IN1RESET = &device_creator<nes_bmc_4in1reset_device>;
const device_type NES_BMC_42IN1RESET = &device_creator<nes_bmc_42in1reset_device>;


nes_action52_device::nes_action52_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_ACTION52, "NES Cart Action 52 PCB", tag, owner, clock, "nes_action52", __FILE__)
{
}

nes_caltron_device::nes_caltron_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_CALTRON6IN1, "NES Cart Caltron 6 in 1 PCB", tag, owner, clock, "nes_caltron", __FILE__), m_latch(0)
				{
}

nes_rumblestat_device::nes_rumblestat_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_RUMBLESTATION, "NES Cart Rumblestation PCB", tag, owner, clock, "nes_rumblestat", __FILE__), m_prg(0), m_chr(0)
				{
}

nes_svision16_device::nes_svision16_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SVISION16, "NES Cart Supervision 16 in 1 PCB", tag, owner, clock, "nes_svision16", __FILE__), m_latch1(0), m_latch2(0)
				{
}

nes_n625092_device::nes_n625092_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_N625092, "NES Cart N625092 PCB", tag, owner, clock, "nes_n625092", __FILE__), m_latch1(0), m_latch2(0)
				{
}

nes_a65as_device::nes_a65as_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_A65AS, "NES Cart A65AS PCB", tag, owner, clock, "nes_a65as", __FILE__)
{
}

nes_t262_device::nes_t262_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_T262, "NES Cart T-262 PCB", tag, owner, clock, "nes_t262", __FILE__), m_latch1(0), m_latch2(0)
				{
}

nes_novel1_device::nes_novel1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NOVEL1, "NES Cart Novel Diamond Type 1 PCB", tag, owner, clock, "nes_novel1", __FILE__)
{
}

nes_novel2_device::nes_novel2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NOVEL2, "NES Cart Novel Diamond Type 2 PCB", tag, owner, clock, "nes_novel2", __FILE__)
{
}

nes_studyngame_device::nes_studyngame_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_STUDYNGAME, "NES Cart Study n Game PCB", tag, owner, clock, "nes_studyngame", __FILE__)
{
}

nes_sgun20in1_device::nes_sgun20in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SUPERGUN20IN1, "NES Cart Supergun 20 in 1 PCB", tag, owner, clock, "nes_sgun20in1", __FILE__)
{
}

nes_vt5201_device::nes_vt5201_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_VT5201, "NES Cart VT5201 PCB", tag, owner, clock, "nes_vt5201", __FILE__), m_latch(0), m_dipsetting(0)
				{
}

nes_810544c_device::nes_810544c_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_810544C, "NES Cart 810544-C-A1 PCB", tag, owner, clock, "nes_810544c", __FILE__)
{
}

nes_ntd03_device::nes_ntd03_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NTD03, "NES Cart NTD-03 PCB", tag, owner, clock, "nes_ntd03", __FILE__)
{
}

nes_bmc_gb63_device::nes_bmc_gb63_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_GB63, "NES Cart BMC Ghostbusters 63 in 1 PCB", tag, owner, clock, "nes_bmc_gb63", __FILE__), m_latch(0), m_dipsetting(0), m_vram_disable(0)
				{
}

nes_bmc_gka_device::nes_bmc_gka_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_GKA, "NES Cart BMC GK-A PCB", tag, owner, clock, "nes_bmc_gka", __FILE__), m_latch1(0), m_latch2(0)
				{
}

nes_bmc_gkb_device::nes_bmc_gkb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_GKB, "NES Cart BMC GK-B PCB", tag, owner, clock, "nes_bmc_gkb", __FILE__)
{
}

nes_bmc_ws_device::nes_bmc_ws_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_WS, "NES Cart BMC WS PCB", tag, owner, clock, "nes_bmc_ws", __FILE__), m_latch(0)
				{
}

nes_bmc_11160_device::nes_bmc_11160_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_11160, "NES Cart BMC-11160 PCB", tag, owner, clock, "nes_bmc_11160", __FILE__)
{
}

nes_bmc_g146_device::nes_bmc_g146_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_G146, "NES Cart BMC-G-146 PCB", tag, owner, clock, "nes_bmc_g146", __FILE__)
{
}

nes_bmc_8157_device::nes_bmc_8157_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_8157, "NES Cart BMC-8157 PCB", tag, owner, clock, "nes_bmc_8157", __FILE__)
{
}

nes_bmc_hik300_device::nes_bmc_hik300_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_HIK300, "NES Cart BMC HIK 300 in 1 PCB", tag, owner, clock, "nes_bmc_hik300", __FILE__)
{
}

nes_bmc_s700_device::nes_bmc_s700_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_S700, "NES Cart BMC Super 700 in 1 PCB", tag, owner, clock, "nes_bmc_s700", __FILE__)
{
}

nes_bmc_ball11_device::nes_bmc_ball11_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_BALL11, "NES Cart BMC Ball 11 in 1 PCB", tag, owner, clock, "nes_bmc_ball11", __FILE__)
{
}

nes_bmc_22games_device::nes_bmc_22games_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_22GAMES, "NES Cart BMC 22 Games PCB", tag, owner, clock, "nes_bmc_22games", __FILE__)
{
}

nes_bmc_64y2k_device::nes_bmc_64y2k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_64Y2K, "NES Cart BMC 64 in 1 Y2K PCB", tag, owner, clock, "nes_bmc_64y2k", __FILE__)
{
}

nes_bmc_12in1_device::nes_bmc_12in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_12IN1, "NES Cart BMC 12 in 1 PCB", tag, owner, clock, "nes_bmc_12in1", __FILE__)
{
}

nes_bmc_20in1_device::nes_bmc_20in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_20IN1, "NES Cart BMC 20 in 1 PCB", tag, owner, clock, "nes_bmc_20in1", __FILE__)
{
}

nes_bmc_21in1_device::nes_bmc_21in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_21IN1, "NES Cart BMC 21 in 1 PCB", tag, owner, clock, "nes_bmc_21in1", __FILE__)
{
}

nes_bmc_31in1_device::nes_bmc_31in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_31IN1, "NES Cart BMC 31 in 1 PCB", tag, owner, clock, "nes_bmc_31in1", __FILE__)
{
}

nes_bmc_35in1_device::nes_bmc_35in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_35IN1, "NES Cart BMC 35 in 1 PCB", tag, owner, clock, "nes_bmc_35in1", __FILE__)
{
}

nes_bmc_36in1_device::nes_bmc_36in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_36IN1, "NES Cart BMC 36 in 1 PCB", tag, owner, clock, "nes_bmc_36in1", __FILE__)
{
}

nes_bmc_64in1_device::nes_bmc_64in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_64IN1, "NES Cart BMC 64 in 1 PCB", tag, owner, clock, "nes_bmc_64in1", __FILE__)
{
}

nes_bmc_70in1_device::nes_bmc_70in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_70IN1, "NES Cart BMC 70 in 1 PCB", tag, owner, clock, "nes_bmc_70in1", __FILE__), m_mode(0)
				{
}

nes_bmc_72in1_device::nes_bmc_72in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_72IN1, "NES Cart BMC 72 in 1 PCB", tag, owner, clock, "nes_bmc_72in1", __FILE__)
{
}

nes_bmc_76in1_device::nes_bmc_76in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_76IN1, "NES Cart BMC 76 in 1 PCB", tag, owner, clock, "nes_bmc_76in1", __FILE__), m_latch1(0), m_latch2(0)
				{
}

nes_bmc_110in1_device::nes_bmc_110in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_110IN1, "NES Cart BMC 110 in 1 PCB", tag, owner, clock, "nes_bmc_110in1", __FILE__)
{
}

nes_bmc_150in1_device::nes_bmc_150in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_150IN1, "NES Cart BMC 150 in 1 PCB", tag, owner, clock, "nes_bmc_150in1", __FILE__)
{
}

nes_bmc_190in1_device::nes_bmc_190in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_190IN1, "NES Cart BMC 190 in 1 PCB", tag, owner, clock, "nes_bmc_190in1", __FILE__)
{
}

nes_bmc_800in1_device::nes_bmc_800in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_800IN1, "NES Cart BMC 800 in 1 PCB", tag, owner, clock, "nes_bmc_800in1", __FILE__), m_mode(0)
				{
}

nes_bmc_1200in1_device::nes_bmc_1200in1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_1200IN1, "NES Cart BMC 1200 in 1 PCB", tag, owner, clock, "nes_bmc_1200in1", __FILE__), m_vram_protect(0)
				{
}

nes_bmc_gold150_device::nes_bmc_gold150_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_GOLD150, "NES Cart BMC Golden 150 in 1 PCB", tag, owner, clock, "nes_bmc_gold150", __FILE__), m_latch(0)
				{
}

nes_bmc_gold260_device::nes_bmc_gold260_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_GOLD260, "NES Cart BMC Golden 260 in 1 PCB", tag, owner, clock, "nes_bmc_gold260", __FILE__)
{
}

nes_bmc_ch001_device::nes_bmc_ch001_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_CH001, "NES Cart BMC CH-001 PCB", tag, owner, clock, "nes_bmc_ch001", __FILE__), m_latch(0)
				{
}

nes_bmc_super22_device::nes_bmc_super22_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_SUPER22, "NES Cart BMC Super 22 Games PCB", tag, owner, clock, "nes_bmc_super22", __FILE__)
{
}

nes_bmc_4in1reset_device::nes_bmc_4in1reset_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_4IN1RESET, "NES Cart BMC 4 in 1 (Reset Based) PCB", tag, owner, clock, "nes_bmc_4in1reset", __FILE__), m_latch(0)
				{
}

nes_bmc_42in1reset_device::nes_bmc_42in1reset_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BMC_42IN1RESET, "NES Cart BMC 42 in 1 (Reset Based) PCB", tag, owner, clock, "nes_bmc_42in1reset", __FILE__), m_latch(0)
				{
}




void nes_action52_device::device_start()
{
	common_start();
}

void nes_action52_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_caltron_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_caltron_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_rumblestat_device::device_start()
{
	common_start();
	save_item(NAME(m_prg));
	save_item(NAME(m_chr));
}

void nes_rumblestat_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_prg = 0;
	m_chr = 0;
}

void nes_svision16_device::device_start()
{
	common_start();
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
}

void nes_svision16_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_latch1 = 0;
	m_latch2 = 0;
}

void nes_n625092_device::device_start()
{
	common_start();
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
}

void nes_n625092_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);

	m_latch1 = 0;
	m_latch2 = 0;
}

void nes_a65as_device::device_start()
{
	common_start();
}

void nes_a65as_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(7);
	set_nt_mirroring(PPU_MIRROR_VERT);
	chr8(0, m_chr_source);
}

void nes_t262_device::device_start()
{
	common_start();
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
}

void nes_t262_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, m_chr_source);

	m_latch1 = 0;
	m_latch2 = 0;
}

void nes_novel1_device::device_start()
{
	common_start();
}

void nes_novel1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);
}

void nes_novel2_device::device_start()
{
	common_start();
}

void nes_novel2_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_studyngame_device::device_start()
{
	common_start();
}

void nes_studyngame_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_sgun20in1_device::device_start()
{
	common_start();
}

void nes_sgun20in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);
}

void nes_vt5201_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_dipsetting));
}

void nes_vt5201_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
	m_dipsetting = 0;
}

void nes_810544c_device::device_start()
{
	common_start();
}

void nes_810544c_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);
}

void nes_ntd03_device::device_start()
{
	common_start();
}

void nes_ntd03_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_bmc_gb63_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_dipsetting));
	save_item(NAME(m_reg));
	save_item(NAME(m_vram_disable));
}

void nes_bmc_gb63_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0xfff);
	chr8(0, CHRRAM);

	m_latch = 0;
	m_dipsetting = 0;
	m_reg[0] = 0;
	m_reg[1] = 0;
	update_banks();
	m_vram_disable = 0;
}

void nes_bmc_gka_device::device_start()
{
	common_start();
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
}

void nes_bmc_gka_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);

	m_latch1 = 0;
	m_latch2 = 0;
}

void nes_bmc_gkb_device::device_start()
{
	common_start();
}

void nes_bmc_gkb_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_bmc_ws_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_ws_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_bmc_11160_device::device_start()
{
	common_start();
}

void nes_bmc_11160_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_bmc_g146_device::device_start()
{
	common_start();
}

void nes_bmc_g146_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_bmc_8157_device::device_start()
{
	common_start();
}

void nes_bmc_8157_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_bmc_hik300_device::device_start()
{
	common_start();
}

void nes_bmc_hik300_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0xff);
	chr8(0xff, CHRROM);
}

void nes_bmc_s700_device::device_start()
{
	common_start();
}

void nes_bmc_s700_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_bmc_ball11_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bmc_ball11_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_reg[0] = 1;
	m_reg[1] = 0;
	set_banks();
}

void nes_bmc_22games_device::device_start()
{
	common_start();
}

void nes_bmc_22games_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, m_chr_source);
}

void nes_bmc_64y2k_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bmc_64y2k_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	chr8(0, m_chr_source);

	m_reg[0] = 0x80;
	m_reg[1] = 0x43;
	m_reg[2] = m_reg[3] = 0;
	set_prg();
	set_nt_mirroring(PPU_MIRROR_VERT);
}

void nes_bmc_12in1_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bmc_12in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
	update_banks();
}

void nes_bmc_20in1_device::device_start()
{
	common_start();
}

void nes_bmc_20in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);
}

void nes_bmc_21in1_device::device_start()
{
	common_start();
}

void nes_bmc_21in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_bmc_31in1_device::device_start()
{
	common_start();
}

void nes_bmc_31in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(1);
	chr8(0, m_chr_source);
}

void nes_bmc_35in1_device::device_start()
{
	common_start();
}

void nes_bmc_35in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);
}

void nes_bmc_36in1_device::device_start()
{
	common_start();
}

void nes_bmc_36in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(m_prg_chunks - 1);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_bmc_64in1_device::device_start()
{
	common_start();
}

void nes_bmc_64in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);
}

void nes_bmc_70in1_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_mode));
}

void nes_bmc_70in1_device::pcb_reset()
{
	prg16_89ab(m_prg_chunks - 1);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRROM);
	m_reg[0] = 0;
	m_reg[1] = 0;
	m_mode = 0;
}

void nes_bmc_72in1_device::device_start()
{
	common_start();
}

void nes_bmc_72in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_bmc_76in1_device::device_start()
{
	common_start();
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
}

void nes_bmc_76in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_latch1 = 0;
	m_latch2 = 0;
}

void nes_bmc_110in1_device::device_start()
{
	common_start();
}

void nes_bmc_110in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);
}

void nes_bmc_150in1_device::device_start()
{
	common_start();
}

void nes_bmc_150in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);
}

void nes_bmc_190in1_device::device_start()
{
	common_start();
}

void nes_bmc_190in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);
}

void nes_bmc_800in1_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_mode));
}

void nes_bmc_800in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(m_prg_chunks - 1);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	m_reg[0] = 0;
	m_reg[1] = 0;
	m_mode = 0;
}

void nes_bmc_1200in1_device::device_start()
{
	common_start();
	save_item(NAME(m_vram_protect));
}

void nes_bmc_1200in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);
	m_vram_protect = 0;
}

void nes_bmc_gold150_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_gold150_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_bmc_gold260_device::device_start()
{
	common_start();
}

void nes_bmc_gold260_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_bmc_ch001_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_ch001_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_bmc_super22_device::device_start()
{
	common_start();
}

void nes_bmc_super22_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}


// This PCB is fully emulated here :)
void nes_bmc_4in1reset_device::device_start()
{
	common_start();
	m_latch = -1;
	save_item(NAME(m_latch));
}

void nes_bmc_4in1reset_device::pcb_reset()
{
	m_latch++;
	m_latch &= 3;
	chr8(m_latch, CHRROM);
	prg16_89ab(m_latch);
	prg16_cdef(m_latch);
}


void nes_bmc_42in1reset_device::device_start()
{
	common_start();
	m_latch = -1;
	save_item(NAME(m_reg));
}

void nes_bmc_42in1reset_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	m_latch++;
	m_latch &= 1;
	chr8(0, m_chr_source);
	prg32(m_latch << 4);

	m_reg[0] = 0;
	m_reg[1] = 0;
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Active Entertainment Action 52 board emulation

 iNES: mapper 228

 -------------------------------------------------*/

WRITE8_MEMBER(nes_action52_device::write_h)
{
	UINT8 pmode = offset & 0x20;
	int pbank = (offset & 0x1fc0) >> 6;
	int cbank = (data & 0x03) | ((offset & 0x0f) << 2);
	LOG_MMC(("ae_act52_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(offset, 13) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	chr8(cbank, CHRROM);

	if (pmode)
	{
		prg16_89ab(pbank);
		prg16_cdef(pbank);
	}
	else
		prg32(pbank >> 1);
}

/*-------------------------------------------------

 Caltron 6 in 1 Board

 Games: 6 in 1 by Caltron

 iNES: mapper 41

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_caltron_device::write_m)
{
	LOG_MMC(("caltron write_m, offset: %04x, data: %02x\n", offset, data));

	m_latch = offset & 0xff;
	set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	prg32(offset & 0x07);
}

WRITE8_MEMBER(nes_caltron_device::write_h)
{
	LOG_MMC(("caltron write_h, offset: %04x, data: %02x\n", offset, data));

	if (m_latch & 0x04)
		chr8(((m_latch & 0x18) >> 1) | (data & 0x03), CHRROM);
}

/*-------------------------------------------------

 Rumblestation Board

 Games: Rumblestation 15 in 1

 iNES: mapper 46

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_rumblestat_device::write_m)
{
	LOG_MMC(("rumblestation write_m, offset: %04x, data: %02x\n", offset, data));

	m_prg = (m_prg & 0x01) | ((data & 0x0f) << 1);
	m_chr = (m_chr & 0x07) | ((data & 0xf0) >> 1);
	prg32(m_prg);
	chr8(m_chr, CHRROM);
}

WRITE8_MEMBER(nes_rumblestat_device::write_h)
{
	LOG_MMC(("rumblestation write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	m_prg = (m_prg & ~0x01) | (data & 0x01);
	m_chr = (m_chr & ~0x07) | ((data & 0x70) >> 4);
	prg32(m_prg);
	chr8(m_chr, CHRROM);
}

/*-------------------------------------------------

 Supervision 16 in 1 Board

 Games: Supervision 16 in 1

 iNES: mapper 53

 In MESS: Partially Supported. (the dump contains 32KB of
 EEPROM which is not currently handled well)

 -------------------------------------------------*/

void nes_svision16_device::update_prg()
{
	int base = (m_latch1 & 0x0f) << 3;
	if (m_latch1 & 0x10)
	{
		prg16_89ab((base | (m_latch2 & 7)) + 2);    // +2 due to the eeprom
		prg16_cdef((base | 0x07) + 2);  // +2 due to the eeprom
	}
	else
	{
		prg16_89ab(0);
		prg16_cdef(1);
	}

}

READ8_MEMBER(nes_svision16_device::read_m)
{
	int bank = (((m_latch1 & 0x0f) << 4) | 0x0f) + 4 ;  // +4 due to the eeprom
	LOG_MMC(("svision16 read_m, offset: %04x\n", offset));
	return m_prg[((bank * 0x2000) + (offset & 0x1fff)) & m_prg_mask];
}

WRITE8_MEMBER(nes_svision16_device::write_m)
{
	LOG_MMC(("svision16 write_m, offset: %04x, data: %02x\n", offset, data));

	m_latch1 = data;
	update_prg();
	set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

WRITE8_MEMBER(nes_svision16_device::write_h)
{
	LOG_MMC(("svision16 write_h, offset: %04x, data: %02x\n", offset, data));
	m_latch2 = data;
	update_prg();
}

/*-------------------------------------------------

 Bootleg Board N625092

 Games: 400 in 1, 700 in 1, 1000 in 1

 iNES: mapper 221

 In MESS: Supported.

 -------------------------------------------------*/

void nes_n625092_device::set_prg(UINT8 reg1, UINT8 reg2)
{
	UINT8 helper1, helper2;

	helper1 = !(reg1 & 0x01) ? reg2 : (reg1 & 0x80) ? reg2 : (reg2 & 0x06) | 0x00;
	helper2 = !(reg1 & 0x01) ? reg2 : (reg1 & 0x80) ? 0x07 : (reg2 & 0x06) | 0x01;

	prg16_89ab(helper1 | ((reg1 & 0x70) >> 1));
	prg16_cdef(helper2 | ((reg1 & 0x70) >> 1));
}

WRITE8_MEMBER(nes_n625092_device::write_h)
{
	LOG_MMC(("n625092 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x4000)
	{
		set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		offset = (offset >> 1) & 0xff;

		if (m_latch1 != offset)
		{
			m_latch1 = offset;
			set_prg(m_latch1, m_latch2);
		}
	}
	else
	{
		offset &= 0x07;

		if (m_latch2 != offset)
		{
			m_latch2 = offset;
			set_prg(m_latch1, m_latch2);
		}
	}
}


/*-------------------------------------------------

 Board BMC-A65AS

 Games: 3-in-1 (N068)

 In MESS: Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_a65as_device::write_h)
{
	UINT8 helper = (data & 0x30) >> 1;
	LOG_MMC(("a65as write_h, offset: %04x, data: %02x\n", offset, data));

	if (data & 0x80)
		set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	else
		set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (data & 0x40)
		prg32(data >> 1);
	else
	{
		prg16_89ab(helper | (data & 0x07));
		prg16_cdef(helper | 0x07);
	}
}

/*-------------------------------------------------

 Board BMC-T-262

 Games: 4-in-1 (D-010), 8-in-1 (A-020)

 In MESS: Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_t262_device::write_h)
{
	UINT8 mmc_helper;
	LOG_MMC(("t262 write_h, offset: %04x, data: %02x\n", offset, data));

	if (m_latch2 || offset == 0)
	{
		m_latch1 = (m_latch1 & 0x38) | (data & 0x07);
		prg16_89ab(m_latch1);
	}
	else
	{
		m_latch2 = 1;
		set_nt_mirroring(BIT(data, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		mmc_helper = ((offset >> 3) & 0x20) | ((offset >> 2) & 0x18);
		m_latch1 = mmc_helper | (m_latch1 & 0x07);
		prg16_89ab(m_latch1);
		prg16_cdef(mmc_helper | 0x07);
	}
}


/*-------------------------------------------------

 BMC-NOVELDIAMOND and BMC-999999in1

 Unknown Bootleg Multigame Board
 Games: I only found 'Novel Diamond 999999-in-1.unf' using
 this mapper (hence the code is used for BMC_NOVELDIAMOND
 board). The code is included here in case a mapper 54
 dump arises.

 iNES: mappers 54 and 213

 In MESS: Partial Support.

 -------------------------------------------------*/

// Are this correct or should they work the same?
WRITE8_MEMBER(nes_novel1_device::write_h)
{
	LOG_MMC(("novel1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(offset & 0x03);
	chr8(offset & 0x07, CHRROM);
}

WRITE8_MEMBER(nes_novel2_device::write_h)
{
	LOG_MMC(("novel2 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(offset >> 1);
	chr8(offset >> 3, CHRROM);
}


/*-------------------------------------------------

 Board UNL-STUDYNGAME

 Games: Study n Game 32 in 1

 iNES: mapper 39

 In MESS: Partially Supported (problems with PRG bankswitch,
 only keyboard exercise work).

 -------------------------------------------------*/

WRITE8_MEMBER(nes_studyngame_device::write_h)
{
	LOG_MMC(("studyngame write_h, offset: %04x, data: %02x\n", offset, data));
	prg32(data);
}

/*-------------------------------------------------

 BMC-SUPERGUN-20IN1

 Unknown Bootleg Multigame Board
 Games: Super Gun 20 in 1

 iNES: mapper 214

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sgun20in1_device::write_h)
{
	LOG_MMC(("supergun20in1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(offset >> 2);
	prg16_cdef(offset >> 2);
	chr8(offset, CHRROM);
}

/*-------------------------------------------------

 BMC-VT5201

 -------------------------------------------------*/

WRITE8_MEMBER(nes_vt5201_device::write_h)
{
	LOG_MMC(("vt5201 wirte_h, offset: %04x, data: %02x\n", offset, data));

	m_latch = BIT(offset, 8);

	// not sure about this mirroring bit!!
	// without it TN 95 in 1 has glitches in Lunar Ball; with it TN 95 in 1 has glitches in Galaxian!
	set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	if (BIT(offset, 7))
	{
		prg16_89ab((offset >> 4) & 0x07);
		prg16_cdef((offset >> 4) & 0x07);
	}
	else
		prg32((offset >> 5) & 0x03);
	chr8(offset, CHRROM);
}

READ8_MEMBER(nes_vt5201_device::read_h)
{
	LOG_MMC(("bmc_vt5201 read_h, offset: %04x\n", offset));
	//  m_dipsetting = ioport("CARTDIPS")->read();

	if (m_latch)
		return m_dipsetting; // cart mode, depending on the Dip Switches (always zero atm, given we have no way to add cart-based DIPs)
	else
		return hi_access_rom(offset);
}

/*-------------------------------------------------

 BMC-810544-C-A1

 Games: 200-in-1 Elfland

 -------------------------------------------------*/

WRITE8_MEMBER(nes_810544c_device::write_h)
{
	UINT8 bank = (offset >> 7);
	LOG_MMC(("810544 write_h, offset: %04x, data: %02x\n", offset, data));

	if (!BIT(offset, 6))
	{
		prg16_89ab((bank << 1) | BIT(offset, 5));
		prg16_cdef((bank << 1) | BIT(offset, 5));
	}
	else
		prg32(bank);

	set_nt_mirroring(BIT(offset, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	chr8(offset & 0x0f, CHRROM);
}

/*-------------------------------------------------

 BMC-NTD-03

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ntd03_device::write_h)
{
	UINT8 pbank = (offset >> 10) & 0x1e;
	UINT8 cbank = ((offset & 0x300) >> 5) | (offset & 0x07);
	LOG_MMC(("ntd03 write_h, offset: %04x, data: %02x\n", offset, data));

	if (BIT(offset, 7))
	{
		prg16_89ab(pbank | BIT(offset, 6));
		prg16_cdef(pbank | BIT(offset, 6));
	}
	else
		prg32(pbank >> 1);

	set_nt_mirroring(BIT(offset, 10) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	chr8(cbank, CHRROM);
}

/*-------------------------------------------------

 BMC-GHOSTBUSTERS63IN1

 in MESS: only preliminar support

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_gb63_device::chr_w)
{
	int bank = offset >> 10;

	if (!m_vram_disable)
		m_chr_access[bank][offset & 0x3ff] = data;
}

void nes_bmc_gb63_device::update_banks()
{
	set_nt_mirroring(BIT(m_reg[0], 6) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);

	if (BIT(m_reg[0], 5))
	{
		prg16_89ab(m_reg[0] & 0x1f);
		prg16_cdef(m_reg[0] & 0x1f);
	}
	else
		prg32((m_reg[0] & 0x1f) >> 1);

// according to FCEUMM source, the game should be able to disable the VRAM, but this stops the game from working
// maybe the VRAM disable does not work at start?
//  m_vram_disable = BIT(m_reg[1], 1) ? 0 : 1;
}

WRITE8_MEMBER(nes_bmc_gb63_device::write_h)
{
	LOG_MMC(("bmc_gb63 write_h, offset: %04x, data: %02x\n", offset, data));

	m_reg[offset & 1] = data;
	m_latch = BIT(m_reg[0], 7) | (BIT(m_reg[1], 0) << 1);
	update_banks();
}

READ8_MEMBER(nes_bmc_gb63_device::read_h)
{
	LOG_MMC(("bmc_gb63 read_h, offset: %04x\n", offset));
	//  m_dipsetting = ioport("CARTDIPS")->read();

	if (m_latch == 1)
		return m_open_bus;    // open bus
	else
		return hi_access_rom(offset);
}


/*-------------------------------------------------

 Board BMC-GKA

 Unknown Bootleg Multigame Board
 Games: 6 in 1, 54 in 1, 106 in 1

 iNES: mapper 57

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_gka_device::write_h)
{
	LOG_MMC(("bmc_gka write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x0800)
		m_latch2 = data;
	else
		m_latch1 = data;

	if (m_latch2 & 0x80)
		prg32(2 | (m_latch2 >> 6));
	else
	{
		prg16_89ab((m_latch2 >> 5) & 0x03);
		prg16_cdef((m_latch2 >> 5) & 0x03);
	}

	set_nt_mirroring((m_latch2 & 0x08) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	chr8((m_latch1 & 0x03) | (m_latch2 & 0x07) | ((m_latch2 & 0x10) >> 1), CHRROM);
}


/*-------------------------------------------------

 Board BMC-GKB

 Unknown Bootleg Multigame Board
 Games: 68 in 1, 73 in 1, 98 in 1

 iNES: mapper 58

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_gkb_device::write_h)
{
	UINT8 bank = (offset & 0x40) ? 0 : 1;
	LOG_MMC(("bmc_gkb write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(offset & ~bank);
	prg16_cdef(offset | bank);
	chr8(offset >> 3, m_chr_source);
	set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Board BMC-WS

 Games: Super 40-in-1

 In MESS: Partially Supported (some games, like Galaxian, have
 issues)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_ws_device::write_m)
{
	UINT8 mmc_helper;
	LOG_MMC(("bmc_ws write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x1000)
	{
		switch (offset & 0x01)
		{
			case 0:
				if (!m_latch)
				{
					m_latch = data & 0x20;
					set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
					mmc_helper = (~data & 0x08) >> 3;
					prg16_89ab(data & ~mmc_helper);
					prg16_cdef(data |  mmc_helper);
				}
				break;
			case 1:
				if (!m_latch)
					chr8(data, CHRROM);
				break;
		}
	}
}

/*-------------------------------------------------

 Board BMC-11160 (by TXC?)

 Games: 6 in 1 (MGC-023)

 In MESS: Partially Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_11160_device::write_h)
{
	LOG_MMC(("bmc_11160 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32((data & 0x70) >> 4);
	chr8(((data & 0x70) >> 2) | (data & 3), m_chr_source);
	set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Board BMC-G-146

 Games: 1994 Super HIK 14 in 1 (G-136)

 In MESS: Partially Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_g146_device::write_h)
{
	LOG_MMC(("bmc_g146 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x800)
	{
		// UNROM mode
		int helper = offset & BIT(offset, 6);
		prg16_89ab((offset & 0x1f) | helper);
		prg16_cdef((offset & 0x18) | 7);
	}
	else if (offset & 0x40)
	{
		// 16KB mode
		prg16_89ab(offset & 0x1f);
		prg16_cdef(offset & 0x1f);
	}
	else
		prg32((offset & 0x1f) >> 4);

	set_nt_mirroring(!BIT(offset, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Board BMC-8157

 Games: 4 in 1 1993 (CK-001)

 In MESS: Partially Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_8157_device::write_h)
{
	LOG_MMC(("bmc_8157 write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab((offset >> 2) & 0x1f);
	if (offset & 0x200)
		prg16_cdef(offset & 0x1f);
	else
		prg16_cdef((offset & 0x18) | 7);

	set_nt_mirroring(!BIT(offset, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

READ8_MEMBER(nes_bmc_8157_device::read_h)
{
	LOG_MMC(("bmc_8157 read_h, offset: %04x\n", offset));
	//  m_dipsetting = ioport("CARTDIPS")->read();

//  UINT8 val = hi_access_rom(offset);
//  return val | m_noise; // the first write_h sets m_noise=0xff or 0 depending on dsw

	return hi_access_rom(offset);
}


/*-------------------------------------------------

 BMC-SUPERHIK_300IN1

 Unknown Bootleg Multigame Board
 Games: 100000 in 1, Super HIK 300 in 1, 1997 in 1

 iNES: mapper 212

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_hik300_device::write_h)
{
	LOG_MMC(("bmc_hik300 write_h, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	chr8(offset, CHRROM);

	if (offset < 0x4000)
	{
		prg16_89ab(offset);
		prg16_cdef(offset);
	}
	else
		prg32(offset >> 1);
}

/*-------------------------------------------------

 BMC-SUPER-700IN1

 Unknown Bootleg Multigame Board
 Games: Super 700 in 1

 iNES: mapper 62

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_s700_device::write_h)
{
	LOG_MMC(("bmc_s700 write_h, offset :%04x, data: %02x\n", offset, data));

	chr8(((offset & 0x1f) << 2) | (data & 0x03), CHRROM);

	if (offset & 0x20)
	{
		prg16_89ab((offset & 0x40) | ((offset >> 8) & 0x3f));
		prg16_cdef((offset & 0x40) | ((offset >> 8) & 0x3f));
	}
	else
	{
		prg32(((offset & 0x40) | ((offset >> 8) & 0x3f)) >> 1);
	}

	set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-BALLGAMES-11IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: 11 in 1 Ball Games

 iNES: mapper 51

 In MESS: Partially Supported.

 -------------------------------------------------*/

void nes_bmc_ball11_device::set_banks()
{
	set_nt_mirroring((m_reg[0] == 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (m_reg[0] & 0x01)
	{
		prg32(m_reg[1]);
	}
	else
	{
		prg16_89ab((m_reg[1] << 1) | (m_reg[0] >> 1));
		prg16_cdef((m_reg[1] << 1) | 0x07);
	}
}

WRITE8_MEMBER(nes_bmc_ball11_device::write_m)
{
	LOG_MMC(("bmc_ball11 write_m, offset: %04x, data: %02x\n", offset, data));

	m_reg[0] = ((data >> 1) & 0x01) | ((data >> 3) & 0x02);
	set_banks();
}

WRITE8_MEMBER(nes_bmc_ball11_device::write_h)
{
	LOG_MMC(("bmc_ball11 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x4000:    // here we also update reg[0] upper bit
			m_reg[0] = (m_reg[0] & 0x01) | ((data >> 3) & 0x02);
		case 0x0000:
		case 0x2000:
		case 0x6000:
			m_reg[1] = data & 0x0f;
			set_banks();
			break;
	}
}

/*-------------------------------------------------

 BMC-22GAMES

 Unknown Bootleg Multigame Board
 Games: 22 in 1

 iNES: mapper 230

 In MESS: Partially Supported. It would need a reset
 to work (not possible yet)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_22games_device::write_h)
{
	LOG_MMC(("bmc_22games write_h, offset: %04x, data: %02x\n", offset, data));

	if (1)  // this should flip at reset
	{
		prg16_89ab(data & 0x07);
	}
	else
	{
		if (data & 0x20)
		{
			prg16_89ab((data & 0x1f) + 8);
			prg16_cdef((data & 0x1f) + 8);
		}
		else
		{
			prg16_89ab((data & 0x1f) + 8);
			prg16_cdef((data & 0x1f) + 9);
		}
		set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	}
}

/*-------------------------------------------------

 Board BMC-64IN1NOREPEAT

 Games: 64-in-1 Y2K

 In MESS: Supported

 -------------------------------------------------*/

void nes_bmc_64y2k_device::set_prg()
{
	UINT8 helper1 = (m_reg[1] & 0x1f);
	UINT8 helper2 = (helper1 << 1) | ((m_reg[1] & 0x40) >> 6);

	if (m_reg[0] & 0x80)
	{
		if (m_reg[1] & 0x80)
			prg32(helper1);
		else
		{
			prg16_89ab(helper2);
			prg16_cdef(helper2);
		}
	}
	else
		prg16_cdef(helper2);
}

WRITE8_MEMBER(nes_bmc_64y2k_device::write_l)
{
	LOG_MMC(("bmc64y2k write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	switch (offset)
	{
		case 0x1000:
		case 0x1001:
		case 0x1002:
		case 0x1003:
			m_reg[offset & 0x03] = data;
			set_prg();
			chr8(((m_reg[0] >> 1) & 0x03) | (m_reg[2] << 2), CHRROM);
			break;
	}
	if (offset == 0x1000)   /* write to reg[0] also sets mirroring */
		set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

WRITE8_MEMBER(nes_bmc_64y2k_device::write_h)
{
	LOG_MMC(("bmc64y2k write_h, offset: %04x, data: %02x\n", offset, data));

	m_reg[3] = data;    // reg[3] is currently unused?!?
}

/*-------------------------------------------------

 BMC-12IN1

 Unknown Bootleg Multigame Board
 Games:

 iNES:

 In MESS: Supported.

 -------------------------------------------------*/

void nes_bmc_12in1_device::update_banks()
{
	int bank = (m_reg[2] & 3) << 3;

	chr4_0((m_reg[0] >> 3) | (bank << 2), m_chr_source);
	chr4_4((m_reg[1] >> 3) | (bank << 2), m_chr_source);

	if (m_reg[2] & 8)
		prg32(((m_reg[0] & 7) >> 1) | bank);
	else
	{
		prg16_89ab((m_reg[0] & 7) | bank);
		prg16_cdef(7 | bank);
	}

	set_nt_mirroring(BIT(m_reg[2], 2) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

WRITE8_MEMBER(nes_bmc_12in1_device::write_h)
{
	LOG_MMC(("bmc_12in1 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x2000: m_reg[0] = data; break;
		case 0x4000: m_reg[1] = data; break;
		case 0x6000: m_reg[2] = data; break;
	}
	update_banks();
}

/*-------------------------------------------------

 BMC-20IN1

 Unknown Bootleg Multigame Board
 Games: 20 in 1

 iNES: mapper 231

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_20in1_device::write_h)
{
	LOG_MMC(("bmc_20in1 write_h, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	prg16_89ab((offset & 0x1e));
	prg16_cdef((offset & 0x1e) | ((offset & 0x20) ? 1 : 0));
}

/*-------------------------------------------------

 BMC-21IN1

 Unknown Bootleg Multigame Board
 Games: 8 in 1, 21 in 1

 iNES: mapper 201

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_21in1_device::write_h)
{
	LOG_MMC(("bmc_21in1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(offset & 0x03);
	chr8(offset & 0x03, CHRROM);
}

/*-------------------------------------------------

 BMC-31IN1

 Unknown Bootleg Multigame Board
 Games: 31 in 1

 iNES: mapper 229

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_31in1_device::write_h)
{
	LOG_MMC(("bmc_31in1 write_h, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	chr8(offset, CHRROM);

	if ((offset & 0x1e) == 0)
	{
		prg16_89ab(0);
		prg16_89ab(1);
	}
	else
	{
		prg16_89ab(offset & 0x1f);
		prg16_89ab(offset & 0x1f);
	}
}

/*-------------------------------------------------

 BMC-35IN1

 Unknown Bootleg Multigame Board
 Games: 35 in 1

 iNES: mapper 203

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_35in1_device::write_h)
{
	LOG_MMC(("bmc_35in1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab((data >> 2) & 0x03);
	prg16_cdef((data >> 2) & 0x03);
	chr8(data & 0x03, CHRROM);
}

/*-------------------------------------------------

 BMC-36IN1

 Unknown Bootleg Multigame Board
 Games: 36 in 1, 1200 in 1

 iNES: mapper 200

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_36in1_device::write_h)
{
	LOG_MMC(("bmc_36in1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(offset & 0x07);
	prg16_cdef(offset & 0x07);
	chr8(offset & 0x07, CHRROM);

	set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-64IN1

 Unknown Bootleg Multigame Board
 Games: 64 in 1

 iNES: mapper 204

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_64in1_device::write_h)
{
	int bank = (offset >> 1) & (offset >> 2) & 0x01;

	LOG_MMC(("bmc_64in1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(offset & ~bank);
	prg16_cdef(offset | bank);
	chr8(offset & ~bank, CHRROM);

	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ: PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-70IN1

 Unknown Bootleg Multigame Board
 Games:

 iNES: mapper

 This is same hardware as BMC-800IN1 below, but this
 cart has CHR and slightly diff bankswitch.
 DSW not emulated yet.

 In MESS: Preliminary Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_70in1_device::write_h)
{
	LOG_MMC(("bmc70in1 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x4000)
	{
		set_nt_mirroring(BIT(offset, 5) ? PPU_MIRROR_HORZ: PPU_MIRROR_VERT);
		chr8(offset & 7, CHRROM);
	}
	else
	{
		switch (offset & 0x30)
		{
			case 0x00: m_mode = 0x0; m_reg[0] = (m_reg[0] & 0x38) | (offset & 0x7); m_reg[1] = m_reg[0] | 0x7; break;
			case 0x10: m_mode = 0x1; m_reg[0] = (m_reg[0] & 0x38) | (offset & 0x7); m_reg[1] = m_reg[0] | 0x7; break;
			case 0x20: m_mode = 0x0; m_reg[0] = (m_reg[0] & 0x38) | (offset & 0x6); m_reg[1] = m_reg[0] | 0x1; break;
			case 0x30: m_mode = 0x0; m_reg[0] = (m_reg[0] & 0x38) | (offset & 0x7); m_reg[1] = m_reg[0] | 0x0; break;
		}
		prg16_89ab(m_reg[0]);
		prg16_cdef(m_reg[1]);
	}
}

READ8_MEMBER(nes_bmc_70in1_device::read_h)
{
	LOG_MMC(("bmc70in1 read_h, offset: %04x\n", offset));

	if (m_mode)
		offset = (offset & 0x7ff0) | m_mode;

	return hi_access_rom(offset);
}

/*-------------------------------------------------

 BMC-72IN1

 Unknown Bootleg Multigame Board
 Games: 72 in 1, 115 in 1 and other multigame carts

 iNES: mapper 225

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_72in1_device::write_h)
{
	int hi_bank = offset & 0x40;
	int size_16 = offset & 0x1000;
	int bank = (offset & 0xf80) >> 7;

	LOG_MMC(("bmc_72in1 write_h, offset: %04x, data: %02x\n", offset, data));

	chr8(offset, CHRROM);
	set_nt_mirroring((offset & 0x2000) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	if (size_16)
	{
		bank <<= 1;
		if (hi_bank)
			bank ++;

		prg16_89ab(bank);
		prg16_cdef(bank);
	}
	else
		prg32(bank);
}

/*-------------------------------------------------

 BMC-76IN1

 Unknown Bootleg Multigame Board
 Games: 76 in 1, Super 42 in 1

 iNES: mapper 226

 In MESS: Supported.

 -------------------------------------------------*/

// does this work for super42in1 as well?!?
WRITE8_MEMBER(nes_bmc_76in1_device::write_h)
{
	int hi_bank;
	int size_16;
	int bank;

	LOG_MMC(("bmc_76in1 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x01)
		m_latch2 = data;
	else
		m_latch1 = data;

	set_nt_mirroring(BIT(m_latch1, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	hi_bank = m_latch1 & 0x01;
	size_16 = m_latch1 & 0x20;
	bank = ((m_latch1 & 0x1e) >> 1) | ((m_latch1 & 0x80) >> 3) | ((m_latch2 & 0x01) << 5);

	if (size_16)
	{
		bank <<= 1;
		if (hi_bank)
			bank ++;

		prg16_89ab(bank);
		prg16_cdef(bank);
	}
	else
		prg32(bank);
}

/*-------------------------------------------------

 BMC-110IN1

 Known Boards: Unknown Bootleg Board
 Games: 110 in 1

 iNES: mapper 255

 In MESS: Preliminary support.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_110in1_device::write_h)
{
	UINT8 helper1 = (offset >> 12) ? 0 : 1;
	UINT8 helper2 = ((offset >> 8) & 0x40) | ((offset >> 6) & 0x3f);

	LOG_MMC(("bmc_110in1 write_h, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring((offset & 0x2000) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	prg16_89ab(helper1 & ~helper2);
	prg16_cdef(helper1 | helper2);
	chr8(((offset >> 8) & 0x40) | (offset & 0x3f), CHRROM);
}


/*-------------------------------------------------

 BMC-150IN1

 Unknown Bootleg Multigame Board
 Games: 150 in 1

 iNES: mapper 202

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_150in1_device::write_h)
{
	int bank = (offset >> 1) & 0x07;

	LOG_MMC(("bmc_150in1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(bank);
	prg16_cdef(bank + (((bank & 0x06) == 0x06) ? 1 : 0));
	chr8(bank, CHRROM);

	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ: PPU_MIRROR_VERT);
}


/*-------------------------------------------------

 Board BMC-190IN1

 Games: 190-in-1

 In MESS: Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_190in1_device::write_h)
{
	LOG_MMC(("bmc190in1 write_h, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	offset >>= 2;
	prg16_89ab(offset);
	prg16_cdef(offset);
	chr8(offset, CHRROM);
}

/*-------------------------------------------------

 BMC-800IN1

 Unknown Bootleg Multigame Board
 Games:

 iNES: mapper

 DSW not emulated yet.

 In MESS: Preliminary Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_800in1_device::write_h)
{
	LOG_MMC(("bmc800in1 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x4000)
	{
		set_nt_mirroring(BIT(offset, 5) ? PPU_MIRROR_HORZ: PPU_MIRROR_VERT);

		m_reg[0] = (m_reg[0]) | ((offset << 3) & 0x38);
		m_reg[1] = (m_reg[1]) | ((offset << 3) & 0x38);
	}
	else
	{
		switch (offset & 0x30)
		{
			case 0x00: m_mode = 0x0; m_reg[0] = (m_reg[0] & 0x38) | (offset & 0x7); m_reg[1] = m_reg[0] | 0x7; break;
			case 0x10: m_mode = 0x1; m_reg[0] = (m_reg[0] & 0x38) | (offset & 0x7); m_reg[1] = m_reg[0] | 0x7; break;
			case 0x20: m_mode = 0x0; m_reg[0] = (m_reg[0] & 0x38) | (offset & 0x6); m_reg[1] = m_reg[0] | 0x1; break;
			case 0x30: m_mode = 0x0; m_reg[0] = (m_reg[0] & 0x38) | (offset & 0x7); m_reg[1] = m_reg[0] | 0x0; break;
		}
	}
	prg16_89ab(m_reg[0]);
	prg16_cdef(m_reg[1]);
}

READ8_MEMBER(nes_bmc_800in1_device::read_h)
{
	LOG_MMC(("bmc800in1 read_h, offset: %04x\n", offset));

	if (m_mode)
		offset = (offset & 0x7ff0) | m_mode;

	return hi_access_rom(offset);
}

/*-------------------------------------------------

 BMC-1200IN1

 Unknown Bootleg Multigame Board
 Games: 1200 in 1, 295 in 1, 76 in 1

 iNES: mapper 227

 In MESS: Preliminary Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_1200in1_device::chr_w)
{
	int bank = offset >> 10;

	if (!m_vram_protect)
		m_chr_access[bank][offset & 0x3ff] = data;
}


WRITE8_MEMBER(nes_bmc_1200in1_device::write_h)
{
	int bank = ((offset >> 2) & 0x1f) |  ((offset & 0x0100) >> 3);

	LOG_MMC(("bmc_1200in1 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x80)
	{
//      m_vram_protect = 1;
		prg16_89ab(bank);
		prg16_cdef(bank + (offset & 1));
	}
	else
	{
		int low_mask = (offset & 1) ? 0x3e : 0xff;

//      m_vram_protect = 0;
		if (!BIT(offset, 9))
		{
			prg16_89ab(bank & low_mask);
			prg16_cdef(bank & 0x38);
		}
		else
		{
			prg16_89ab(bank & low_mask);
			prg16_cdef(bank | 0x07);
		}
	}

	set_nt_mirroring(BIT(offset, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}


/*-------------------------------------------------

 BMC-GOLDEN260IN1

 Unknown Bootleg Multigame Board
 Games:

 iNES: mapper 235

 In MESS: Preliminary Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_gold260_device::write_h)
{
	int bank = (offset & 0x1f) |  ((offset & 0x0300) >> 3);
	LOG_MMC(("bmc_gold260 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x400)
		set_nt_mirroring(PPU_MIRROR_LOW);
	else
		set_nt_mirroring(BIT(offset, 13) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (offset & 0x800)
	{
		bank = (bank << 1) | BIT(offset, 12);
		prg16_89ab(bank);
		prg16_cdef(bank);
	}
	else
		prg32(bank);
}


/*-------------------------------------------------

 BMC-GOLDEN150IN1

 Unknown Bootleg Multigame Board
 Games:

 iNES: mapper 235

 Same as the above + open bus in 0x8000-0xffff when
 enabled

 In MESS: Preliminary Supported.

 -------------------------------------------------*/


WRITE8_MEMBER(nes_bmc_gold150_device::write_h)
{
	int bank = (offset & 0x1f) |  ((offset & 0x0200) >> 4);
	LOG_MMC(("bmc_gold150 write_h, offset: %04x, data: %02x\n", offset, data));

	m_latch = (offset & 0x0100);

	if (offset & 0x400)
		set_nt_mirroring(PPU_MIRROR_LOW);
	else
		set_nt_mirroring(BIT(offset, 13) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (offset & 0x800)
	{
		bank = (bank << 1) | BIT(offset, 12);
		prg16_89ab(bank);
		prg16_cdef(bank);
	}
	else
		prg32(bank);
}

READ8_MEMBER(nes_bmc_gold150_device::read_h)
{
	LOG_MMC(("bmc_gold150 read_h, offset: %04x\n", offset));

	if (m_latch)    // open bus
		return m_open_bus;
	else
		return hi_access_rom(offset);
}


/*-------------------------------------------------

 BMC-POWERFUL-255

 Unknown Bootleg Multigame Board
 Games:

 iNES: mapper 63


 In MESS: Preliminary Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_ch001_device::write_h)
{
	int bank = ((offset >> 1) & 0x1fc);
	LOG_MMC(("bmc_ch001 write_h, offset: %04x, data: %02x\n", offset, data));

	m_latch = ((offset & 0x300) == 0x300);
	set_nt_mirroring(BIT(offset, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (offset & 2)
	{
		prg8_89(bank + 0);
		prg8_ab(bank + 1);
		prg8_cd(bank + 2);
		prg8_ef(bank + 3);
	}
	else
	{
		bank |= (offset & 4) >> 1;
		prg8_89(bank + 0);
		prg8_ab(bank + 1);
		prg8_cd(bank + 0);
		prg8_ef(bank + 1);
	}

	if (offset & 0x800) // in this case, the last 8KB bank is switched differently...
		prg8_ef((offset & 0x07c ) | ((offset & 0x6) ? 0x3 : 0x1));
}

READ8_MEMBER(nes_bmc_ch001_device::read_h)
{
	LOG_MMC(("bmc_ch001 read_h, offset: %04x\n", offset));

	if (m_latch && offset < 0x4000) // open bus
		return m_open_bus;
	else
		return hi_access_rom(offset);
}

/*-------------------------------------------------

 BMC-SUPER22GAMES

 Unknown Bootleg Multigame Board
 Games:

 iNES:

 is there a dsw to access the higher banks above 0x20?

 In MESS: Preliminary Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_super22_device::write_h)
{
	LOG_MMC(("bmc_super22 write_h, offset: %04x, data: %02x\n", offset, data));

	if (data & 0x20)
	{
		prg16_89ab(data & 0x1f);
		prg16_cdef(data & 0x1f);
	}
	else
		prg32((data & 0x1f) >> 1);

	switch (data & 0xc0)
	{
		case 0x00: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 0x40: set_nt_mirroring(PPU_MIRROR_VERT); break;
		case 0x80: set_nt_mirroring(PPU_MIRROR_HORZ); break;
		case 0xc0: set_nt_mirroring(PPU_MIRROR_HIGH); break;
	}
}


/*-------------------------------------------------

 BMC-RESETBASED4IN1

 Unknown Bootleg Multigame Board

 Games:

 iNES:

 No need to use handlers. At reset the banks change
 and so does the game.

 In MESS: Preliminary Supported.

 -------------------------------------------------*/

/*-------------------------------------------------

 BMC-42IN1RESETBASED

 Unknown Bootleg Multigame Board

 Games:

 iNES:

 In MESS: Preliminary Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_bmc_42in1reset_device::write_h)
{
	int bank;
	LOG_MMC(("bmc_42in1reset write_h, offset: %04x, data: %02x\n", offset, data));

	m_reg[offset & 1] = data;
	bank = (m_reg[0] & 0x1f) | (m_latch << 5) | ((m_reg[1] & 1) << 6);

	if (!(m_reg[0] & 0x20))
		prg32(bank >> 1);
	else
	{
		prg16_89ab(bank);
		prg16_cdef(bank);
	}

	set_nt_mirroring(BIT(offset, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

}
