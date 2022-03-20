// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Multigame Carts PCBs


 Here we emulate several PCBs used in multigame pirate carts (not MMC-3 based)

 TODO: Investigate further Gunsmoke on mc_8et40 and mc_2gn91. Both exhibit the
 same bug where enemies/barrels don't appear until they are halfway down the
 screen. They are both almost the same minor hack of the US release.

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

DEFINE_DEVICE_TYPE(NES_ACTION52,       nes_action52_device,       "nes_action52",       "NES Cart Action 52 PCB")
DEFINE_DEVICE_TYPE(NES_CALTRON6IN1,    nes_caltron6in1_device,    "nes_caltron6in1",    "NES Cart Caltron 6 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_CALTRON9IN1,    nes_caltron9in1_device,    "nes_caltron9in1",    "NES Cart Caltron 9 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_RUMBLESTATION,  nes_rumblestat_device,     "nes_rumblestat",     "NES Cart Rumblestation PCB")
DEFINE_DEVICE_TYPE(NES_SVISION16,      nes_svision16_device,      "nes_svision16",      "NES Cart Supervision 16 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_FARID_UNROM,    nes_farid_unrom_device,    "nes_farid_unrom",    "NES Cart Farid UNROM 8 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_KN42,           nes_kn42_device,           "nes_kn42",           "NES Cart KN-42 PCB")
DEFINE_DEVICE_TYPE(NES_N625092,        nes_n625092_device,        "nes_n625092",        "NES Cart N625092 PCB")
DEFINE_DEVICE_TYPE(NES_A65AS,          nes_a65as_device,          "nes_a65as",          "NES Cart A65AS PCB")
DEFINE_DEVICE_TYPE(NES_T262,           nes_t262_device,           "nes_t262",           "NES Cart T-262 PCB")
DEFINE_DEVICE_TYPE(NES_STUDYNGAME,     nes_studyngame_device,     "nes_studyngame",     "NES Cart Study n Game PCB")
DEFINE_DEVICE_TYPE(NES_SUPERGUN20IN1,  nes_sgun20in1_device,      "nes_sgun20in1",      "NES Cart Supergun 20 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_VT5201,         nes_vt5201_device,         "nes_vt5201",         "NES Cart VT5201 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_60311C,     nes_bmc_60311c_device,     "nes_bmc_60311c",     "NES Cart BMC 60311C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_80013B,     nes_bmc_80013b_device,     "nes_bmc_80013b",     "NES Cart BMC 80013-B PCB")
DEFINE_DEVICE_TYPE(NES_BMC_810544C,    nes_bmc_810544c_device,    "nes_bmc_810544c",    "NES Cart BMC 810544-C-A1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_830425C,    nes_bmc_830425c_device,    "nes_bmc_830425c",    "NES Cart BMC 830425C-4391T PCB")
DEFINE_DEVICE_TYPE(NES_BMC_830928C,    nes_bmc_830928c_device,    "nes_bmc_830928c",    "NES Cart BMC 830928C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_850437C,    nes_bmc_850437c_device,    "nes_bmc_850437c",    "NES Cart BMC 850437C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_891227,     nes_bmc_891227_device,     "nes_bmc_891227",     "NES Cart BMC 891227 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_970630C,    nes_bmc_970630c_device,    "nes_bmc_970630c",    "NES Cart BMC 970630C PCB")
DEFINE_DEVICE_TYPE(NES_NTD03,          nes_ntd03_device,          "nes_ntd03",          "NES Cart NTD-03 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_CTC09,      nes_bmc_ctc09_device,      "nes_bmc_ctc09",      "NES Cart BMC CTC-09 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_CTC12IN1,   nes_bmc_ctc12in1_device,   "nes_bmc_ctc12in1",   "NES Cart BMC CTC-12IN1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_FAM250,     nes_bmc_fam250_device,     "nes_bmc_fam250",     "NES Cart BMC FAM250 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GKA,        nes_bmc_gka_device,        "nes_bmc_gka",        "NES Cart BMC GK-A PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GKB,        nes_bmc_gkb_device,        "nes_bmc_gkb",        "NES Cart BMC GK-B PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GKCXIN1,    nes_bmc_gkcxin1_device,    "nes_bmc_gkcxin1",    "NES Cart BMC GKCXIN1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GN91B,      nes_bmc_gn91b_device,      "nes_bmc_gn91b",      "NES Cart BMC GN-91B PCB")
DEFINE_DEVICE_TYPE(NES_BMC_HP898F,     nes_bmc_hp898f_device,     "nes_bmc_hp898f",     "NES Cart BMC HP-898F PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K1029,      nes_bmc_k1029_device,      "nes_bmc_k1029",      "NES Cart BMC K-1029 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K3036,      nes_bmc_k3036_device,      "nes_bmc_k3036",      "NES Cart BMC K-3036 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K3046,      nes_bmc_k3046_device,      "nes_bmc_k3046",      "NES Cart BMC K-3046 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_SA005A,     nes_bmc_sa005a_device,     "nes_bmc_sa005a",     "NES Cart BMC SA005-A PCB")
DEFINE_DEVICE_TYPE(NES_BMC_TF2740,     nes_bmc_tf2740_device,     "nes_bmc_tf2740",     "NES Cart BMC TF2740 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_TJ03,       nes_bmc_tj03_device,       "nes_bmc_tj03",       "NES Cart BMC TJ-03 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_WS,         nes_bmc_ws_device,         "nes_bmc_ws",         "NES Cart BMC WS PCB")
DEFINE_DEVICE_TYPE(NES_BMC_11160,      nes_bmc_11160_device,      "nes_bmc_1160",       "NES Cart BMC-1160 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_G146,       nes_bmc_g146_device,       "nes_bmc_g146",       "NES Cart BMC-G-146 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_2751,       nes_bmc_2751_device,       "nes_bmc_2751",       "NES Cart BMC-2751 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_8157,       nes_bmc_8157_device,       "nes_bmc_8157",       "NES Cart BMC-8157 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_HIK300,     nes_bmc_hik300_device,     "nes_bmc_hik300",     "NES Cart BMC HIK 300 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_S700,       nes_bmc_s700_device,       "nes_bmc_s700",       "NES Cart BMC Super 700 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_BALL11,     nes_bmc_ball11_device,     "nes_bmc_ball11",     "NES Cart BMC Ball 11 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_22GAMES,    nes_bmc_22games_device,    "nes_bmc_22games",    "NES Cart BMC 22 Games PCB")
DEFINE_DEVICE_TYPE(NES_BMC_64Y2K,      nes_bmc_64y2k_device,      "nes_bmc_64y2k",      "NES Cart BMC Y2K 64 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_420Y2K,     nes_bmc_420y2k_device,     "nes_bmc_420y2k",     "NES Cart BMC Y2K 420 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_12IN1,      nes_bmc_12in1_device,      "nes_bmc_12in1",      "NES Cart BMC 12 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_20IN1,      nes_bmc_20in1_device,      "nes_bmc_20in1",      "NES Cart BMC 20 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_21IN1,      nes_bmc_21in1_device,      "nes_bmc_21in1",      "NES Cart BMC 21 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_31IN1,      nes_bmc_31in1_device,      "nes_bmc_31in1",      "NES Cart BMC 31 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_35IN1,      nes_bmc_35in1_device,      "nes_bmc_35in1",      "NES Cart BMC 35 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_36IN1,      nes_bmc_36in1_device,      "nes_bmc_36in1",      "NES Cart BMC 36 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_64IN1,      nes_bmc_64in1_device,      "nes_bmc_64in1",      "NES Cart BMC 64 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_70IN1,      nes_bmc_70in1_device,      "nes_bmc_70in1",      "NES Cart BMC 70 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_72IN1,      nes_bmc_72in1_device,      "nes_bmc_72in1",      "NES Cart BMC 72 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_76IN1,      nes_bmc_76in1_device,      "nes_bmc_76in1",      "NES Cart BMC 76 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_150IN1,     nes_bmc_150in1_device,     "nes_bmc_150in1",     "NES Cart BMC 150 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_190IN1,     nes_bmc_190in1_device,     "nes_bmc_190in1",     "NES Cart BMC 190 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_500IN1,     nes_bmc_500in1_device,     "nes_bmc_500in1",     "NES Cart BMC 500 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_800IN1,     nes_bmc_800in1_device,     "nes_bmc_800in1",     "NES Cart BMC 800 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_1200IN1,    nes_bmc_1200in1_device,    "nes_bmc_1200in1",    "NES Cart BMC 1200 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GOLD150,    nes_bmc_gold150_device,    "nes_bmc_gold150",    "NES Cart BMC Golden 150 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GOLD260,    nes_bmc_gold260_device,    "nes_bmc_gold260",    "NES Cart BMC Golden 260 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_TH22913,    nes_bmc_th22913_device,    "nes_bmc_th22913",    "NES Cart BMC TH2291-3 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_82AB,       nes_bmc_82ab_device,       "nes_bmc_82ab",       "NES Cart BMC 82AB PCB")
DEFINE_DEVICE_TYPE(NES_BMC_4IN1RESET,  nes_bmc_4in1reset_device,  "nes_bmc_4in1reset",  "NES Cart BMC 4 in 1 (Reset Based) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_42IN1RESET, nes_bmc_42in1reset_device, "nes_bmc_42in1reset", "NES Cart BMC 42 in 1 (Reset Based) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_NC20MB,     nes_bmc_nc20mb_device,     "nes_bmc_nc20mb",     "NES Cart BMC NC-20MB PCB")
DEFINE_DEVICE_TYPE(NES_BMC_LC160,      nes_bmc_lc160_device,      "nes_bmc_lc160",      "NES Cart BMC Little Com 160 PCB")


INPUT_PORTS_START( bmc_8157 )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x01, 0x01, "Menu Type" )
	PORT_CONFSETTING(    0x00, "20-in-1" )
	PORT_CONFSETTING(    0x01, "4-in-1" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_bmc_8157_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_8157 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

nes_action52_device::nes_action52_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_ACTION52, tag, owner, clock)
{
}

nes_caltron6in1_device::nes_caltron6in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_CALTRON6IN1, tag, owner, clock), m_latch(0), m_reg(0)
{
}

nes_caltron9in1_device::nes_caltron9in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_CALTRON9IN1, tag, owner, clock)
{
}

nes_rumblestat_device::nes_rumblestat_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_RUMBLESTATION, tag, owner, clock), m_prg(0), m_chr(0)
{
}

nes_svision16_device::nes_svision16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_SVISION16, tag, owner, clock), m_latch1(0), m_latch2(0)
{
}

nes_farid_unrom_device::nes_farid_unrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_FARID_UNROM, tag, owner, clock), m_reg(0)
{
}

nes_kn42_device::nes_kn42_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_KN42, tag, owner, clock), m_latch(0)
{
}

nes_a65as_device::nes_a65as_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_A65AS, tag, owner, clock)
{
}

nes_t262_device::nes_t262_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_T262, tag, owner, clock), m_latch(0)
{
}

nes_studyngame_device::nes_studyngame_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_STUDYNGAME, tag, owner, clock)
{
}

nes_sgun20in1_device::nes_sgun20in1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
{
}

nes_sgun20in1_device::nes_sgun20in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sgun20in1_device(mconfig, NES_SUPERGUN20IN1, tag, owner, clock)
{
}

nes_bmc_190in1_device::nes_bmc_190in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sgun20in1_device(mconfig, NES_BMC_190IN1, tag, owner, clock)
{
}

nes_vt5201_device::nes_vt5201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_VT5201, tag, owner, clock), m_latch(0), m_jumper(0)
{
}

nes_bmc_80013b_device::nes_bmc_80013b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_80013B, tag, owner, clock), m_latch(0)
{
}

nes_bmc_810544c_device::nes_bmc_810544c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_810544C, tag, owner, clock)
{
}

nes_bmc_830425c_device::nes_bmc_830425c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_830425C, tag, owner, clock), m_latch(0)
{
}

nes_bmc_830928c_device::nes_bmc_830928c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_830928C, tag, owner, clock), m_latch(0)
{
}

nes_bmc_850437c_device::nes_bmc_850437c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_850437C, tag, owner, clock)
{
}

nes_bmc_970630c_device::nes_bmc_970630c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_970630C, tag, owner, clock), m_latch(0)
{
}

nes_ntd03_device::nes_ntd03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_NTD03, tag, owner, clock)
{
}

nes_bmc_ctc09_device::nes_bmc_ctc09_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_CTC09, tag, owner, clock)
{
}

nes_bmc_gka_device::nes_bmc_gka_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_GKA, tag, owner, clock)
{
}

nes_bmc_gkb_device::nes_bmc_gkb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_GKB, tag, owner, clock)
{
}

nes_bmc_gkcxin1_device::nes_bmc_gkcxin1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_GKCXIN1, tag, owner, clock)
{
}

nes_bmc_gn91b_device::nes_bmc_gn91b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_GN91B, tag, owner, clock), m_latch(0)
{
}

nes_bmc_hp898f_device::nes_bmc_hp898f_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_HP898F, tag, owner, clock)
{
}

nes_bmc_k3036_device::nes_bmc_k3036_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_K3036, tag, owner, clock)
{
}

nes_bmc_k3046_device::nes_bmc_k3046_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_K3046, tag, owner, clock)
{
}

nes_bmc_sa005a_device::nes_bmc_sa005a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_SA005A, tag, owner, clock)
{
}

nes_bmc_tf2740_device::nes_bmc_tf2740_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_TF2740, tag, owner, clock), m_jumper(0)
{
}

nes_bmc_tj03_device::nes_bmc_tj03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_TJ03, tag, owner, clock)
{
}

nes_bmc_ws_device::nes_bmc_ws_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BMC_WS, tag, owner, clock), m_latch(0)
{
}

nes_bmc_11160_device::nes_bmc_11160_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_11160, tag, owner, clock)
{
}

nes_bmc_g146_device::nes_bmc_g146_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_G146, tag, owner, clock)
{
}

nes_bmc_2751_device::nes_bmc_2751_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_2751, tag, owner, clock)
{
}

nes_bmc_8157_device::nes_bmc_8157_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_8157, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
	, m_latch(0)
{
}

nes_bmc_hik300_device::nes_bmc_hik300_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_HIK300, tag, owner, clock)
{
}

nes_bmc_s700_device::nes_bmc_s700_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_S700, tag, owner, clock)
{
}

nes_bmc_ball11_device::nes_bmc_ball11_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_BALL11, tag, owner, clock)
{
}

nes_bmc_22games_device::nes_bmc_22games_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_22GAMES, tag, owner, clock), m_latch(0), m_reset(0)
{
}

nes_bmc_64y2k_device::nes_bmc_64y2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BMC_64Y2K, tag, owner, clock)
{
}

nes_bmc_420y2k_device::nes_bmc_420y2k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_420Y2K, tag, owner, clock), m_latch(0), m_reg(0)
{
}

nes_bmc_12in1_device::nes_bmc_12in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BMC_12IN1, tag, owner, clock)
{
}

nes_bmc_20in1_device::nes_bmc_20in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_20IN1, tag, owner, clock)
{
}

nes_bmc_21in1_device::nes_bmc_21in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_21IN1, tag, owner, clock)
{
}

nes_bmc_31in1_device::nes_bmc_31in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_31IN1, tag, owner, clock)
{
}

nes_bmc_35in1_device::nes_bmc_35in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BMC_35IN1, tag, owner, clock)
{
}

nes_bmc_36in1_device::nes_bmc_36in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BMC_36IN1, tag, owner, clock)
{
}

nes_bmc_64in1_device::nes_bmc_64in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BMC_64IN1, tag, owner, clock)
{
}

nes_bmc_70in1_device::nes_bmc_70in1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_jumper(type == NES_BMC_70IN1 ? 0x0d : 0x06)
{
}

nes_bmc_70in1_device::nes_bmc_70in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_70in1_device(mconfig, NES_BMC_70IN1, tag, owner, clock)
{
}

nes_bmc_800in1_device::nes_bmc_800in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_70in1_device(mconfig, NES_BMC_800IN1, tag, owner, clock)
{
}

nes_bmc_72in1_device::nes_bmc_72in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_72IN1, tag, owner, clock)
{
}

nes_bmc_76in1_device::nes_bmc_76in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_76IN1, tag, owner, clock)
{
}

nes_bmc_150in1_device::nes_bmc_150in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_150IN1, tag, owner, clock)
{
}

nes_bmc_500in1_device::nes_bmc_500in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_500IN1, tag, owner, clock)
{
}

nes_bmc_1200in1_device::nes_bmc_1200in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BMC_1200IN1, tag, owner, clock), m_vram_protect(0)
{
}

nes_bmc_gold150_device::nes_bmc_gold150_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BMC_GOLD150, tag, owner, clock), m_latch(0)
{
}

nes_bmc_gold260_device::nes_bmc_gold260_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BMC_GOLD260, tag, owner, clock)
{
}

nes_bmc_4in1reset_device::nes_bmc_4in1reset_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_4IN1RESET, tag, owner, clock), m_latch(0)
{
}

nes_bmc_42in1reset_device::nes_bmc_42in1reset_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 mirror_flip)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_latch(0), m_mirror_flip(mirror_flip)
{
}

nes_bmc_42in1reset_device::nes_bmc_42in1reset_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_42in1reset_device(mconfig, NES_BMC_42IN1RESET, tag, owner, clock, 0)
{
}

nes_bmc_nc20mb_device::nes_bmc_nc20mb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_42in1reset_device(mconfig, NES_BMC_NC20MB, tag, owner, clock, 1)
{
}

nes_bmc_lc160_device::nes_bmc_lc160_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_LC160, tag, owner, clock)
{
}

nes_vram_protect_device::nes_vram_protect_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	 : nes_nrom_device(mconfig, type, tag, owner, clock), m_vram_protect(false)
{
}

nes_bmc_60311c_device::nes_bmc_60311c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_vram_protect_device(mconfig, NES_BMC_60311C, tag, owner, clock)
{
}

nes_bmc_ctc12in1_device::nes_bmc_ctc12in1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_vram_protect_device(mconfig, type, tag, owner, clock)
{
}

nes_bmc_ctc12in1_device::nes_bmc_ctc12in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_ctc12in1_device(mconfig, NES_BMC_CTC12IN1, tag, owner, clock)
{
}

nes_bmc_891227_device::nes_bmc_891227_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_ctc12in1_device(mconfig, NES_BMC_891227, tag, owner, clock)
{
}

nes_bmc_k1029_device::nes_bmc_k1029_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_vram_protect_device(mconfig, type, tag, owner, clock)
{
}

nes_bmc_k1029_device::nes_bmc_k1029_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_k1029_device(mconfig, NES_BMC_K1029, tag, owner, clock)
{
}

nes_bmc_fam250_device::nes_bmc_fam250_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_k1029_device(mconfig, NES_BMC_FAM250, tag, owner, clock), m_latch(0), m_reg(0)
{
}

nes_n625092_device::nes_n625092_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_vram_protect_device(mconfig, NES_N625092, tag, owner, clock)
{
}

nes_bmc_th22913_device::nes_bmc_th22913_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 vram_prot_bit)
	: nes_vram_protect_device(mconfig, type, tag, owner, clock), m_vram_prot_bit(vram_prot_bit)
{
}

nes_bmc_th22913_device::nes_bmc_th22913_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_th22913_device(mconfig, NES_BMC_TH22913, tag, owner, clock, 10)
{
}

nes_bmc_82ab_device::nes_bmc_82ab_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_th22913_device(mconfig, NES_BMC_82AB, tag, owner, clock, 9)
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

void nes_caltron6in1_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_caltron6in1_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRROM);

	m_latch = 0;
	m_reg = 0;
}

void nes_caltron9in1_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_caltron9in1_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRROM);

	m_latch[0] = m_latch[1] = m_latch[2] = 0;
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
	prg32(0);
	chr8(0, CHRRAM);

	m_latch1 = 0;
	m_latch2 = 0;
}

void nes_farid_unrom_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_farid_unrom_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, CHRRAM);

	m_reg &= 0x87;    // only middle four bits cleared on soft reset
}

void nes_kn42_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_kn42_device::pcb_reset()
{
	m_latch ^= 0x10;
	prg16_89ab(m_latch);
	prg16_cdef(m_latch | 0x0f);    // fixed to last bank for either game
	chr8(0, CHRRAM);
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
	save_item(NAME(m_latch));
}

void nes_t262_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, CHRRAM);

	m_latch = 0;
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

void nes_sgun20in1_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRROM);
	set_nt_mirroring(PPU_MIRROR_VERT);
}

void nes_vt5201_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_jumper));
}

void nes_vt5201_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRROM);

	m_latch = 0;
	m_jumper = 0;
}

void nes_bmc_80013b_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_bmc_80013b_device::pcb_reset()
{
	chr8(0, CHRRAM);

	m_latch = 0x80;
	m_reg[0] = m_reg[1] = 0;
	update_prg();
}

void nes_bmc_810544c_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRROM);
}

void nes_bmc_830425c_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_830425c_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0x0f);
	chr8(0, CHRRAM);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_latch = 0;
}

void nes_bmc_830928c_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_830928c_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, CHRRAM);

	m_latch = 0;
}

void nes_bmc_850437c_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bmc_850437c_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, CHRRAM);

	m_reg[0] = m_reg[1] = 0;
}

void nes_bmc_970630c_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_970630c_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, CHRRAM);

	m_latch = 0;
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

void nes_bmc_ctc09_device::pcb_reset()
{
// nes_slot's pcb_start sets us up in the main menu. Soft reset is empty so
// that games reset to their own title screens. This seems to be this cart's
// intended behavior as trying to reset to the menu here crashes (due to RAM
// contents?). Soft reset can similarly crash the main menu (BTANB?).
}

void nes_bmc_gka_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bmc_gka_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRROM);

	m_reg[0] = m_reg[1] = 0;
}

void nes_bmc_gn91b_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_gn91b_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, CHRRAM);

	m_latch = 0;
}

void nes_bmc_k3036_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, CHRRAM);
}

void nes_bmc_k3046_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, CHRRAM);
}

void nes_bmc_sa005a_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRROM);
}

void nes_bmc_tf2740_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_jumper));
}

void nes_bmc_tf2740_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRROM);

	m_reg[0] = m_reg[1] = m_reg[2] = 0;
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

void nes_bmc_2751_device::pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted)
{
	device_nes_cart_interface::pcb_start(machine, ciram_ptr, cart_mounted);
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRROM);
}

void nes_bmc_2751_device::pcb_reset()
{
	// this board does not reset to menu on soft reset
}

void nes_bmc_8157_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_8157_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRRAM);

	m_latch = 0;
}

void nes_bmc_hik300_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRROM);
}

void nes_bmc_ball11_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bmc_ball11_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRRAM);

	m_reg[0] = 2;
	m_reg[1] = 0;
}

void nes_bmc_22games_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reset));
}

void nes_bmc_22games_device::pcb_reset()
{
	if (m_reset)
	{
		prg32(4);
		m_latch = 1;
	}
	else
	{
		prg16_89ab(0);
		prg16_cdef(7);
		set_nt_mirroring(PPU_MIRROR_VERT);
		m_reset = 1;
	}
	chr8(0, CHRRAM);
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

void nes_bmc_420y2k_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_bmc_420y2k_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, CHRRAM);

	m_latch = 0;
	m_reg = 0;
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
	save_item(NAME(m_latch));
}

void nes_bmc_70in1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	chr8(0, m_chr_source);

	m_latch[0] = m_latch[1] = 0;
	update_banks();
}

void nes_bmc_72in1_device::device_start()
{
	common_start();
	save_item(NAME(m_extra_ram));
}

void nes_bmc_76in1_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bmc_76in1_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRRAM);

	m_reg[0] = m_reg[1] = 0;
}

void nes_bmc_150in1_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRROM);
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

// This PCB is fully emulated here :)
void nes_bmc_4in1reset_device::device_start()
{
	common_start();
	m_latch = 3;
	save_item(NAME(m_latch));
}

void nes_bmc_4in1reset_device::pcb_reset()
{
	m_latch = (m_latch + 1) & 3;
	prg16_89ab(m_latch);
	prg16_cdef(m_latch);
	chr8(m_latch, CHRROM);
}


void nes_bmc_42in1reset_device::device_start()
{
	common_start();
	m_latch = 0x20;
	save_item(NAME(m_latch));
}

void nes_bmc_42in1reset_device::pcb_reset()
{
	m_latch ^= 0x20;
	prg32(m_latch >> 1);
	chr8(0, CHRRAM);
}

void nes_vram_protect_device::device_start()
{
	common_start();
	save_item(NAME(m_vram_protect));
}

void nes_vram_protect_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRRAM);
	m_vram_protect = false;
}

void nes_bmc_60311c_device::device_start()
{
	nes_vram_protect_device::device_start();
	save_item(NAME(m_reg));
}

void nes_bmc_60311c_device::pcb_reset()
{
	nes_vram_protect_device::pcb_reset();

	m_reg[0] = m_reg[1] = m_reg[2] = 0;
	update_banks();
}

void nes_bmc_ctc12in1_device::device_start()
{
	nes_vram_protect_device::device_start();
	save_item(NAME(m_reg));
}

void nes_bmc_ctc12in1_device::pcb_reset()
{
	nes_vram_protect_device::pcb_reset();
	prg16_89ab(0);
	prg16_cdef(0);

	m_reg[0] = m_reg[1] = 0;
}

void nes_bmc_fam250_device::device_start()
{
	nes_bmc_k1029_device::device_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_bmc_fam250_device::pcb_reset()
{
	nes_bmc_k1029_device::pcb_reset();

	m_latch = 0;
	m_reg = 0;
}

void nes_n625092_device::device_start()
{
	nes_vram_protect_device::device_start();
	save_item(NAME(m_latch));
}

void nes_n625092_device::pcb_reset()
{
	nes_vram_protect_device::pcb_reset();
	prg16_89ab(0);
	prg16_cdef(0);

	m_latch[0] = m_latch[1] = 0;
}

void nes_bmc_th22913_device::pcb_reset()
{
	nes_vram_protect_device::pcb_reset();
	prg16_89ab(0);
	prg16_cdef(0);
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Active Entertainment Action 52 board emulation

 iNES: mapper 228

 -------------------------------------------------*/

void nes_action52_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t pmode = offset & 0x20;
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

 In MAME: Supported.

 -------------------------------------------------*/

void nes_caltron6in1_device::update_chr()
{
	chr8(((m_latch >> 1) & 0x0c) | m_reg, CHRROM);
}

void nes_caltron6in1_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("caltron6in1 write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x1800)
	{
		case 0x0000:
			m_latch = offset & 0x3f;
			prg32(offset & 0x07);
			update_chr();
			set_nt_mirroring(BIT(offset, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

void nes_caltron6in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("caltron6in1 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (BIT(m_latch, 2))
	{
		m_reg = data & 0x03;
		update_chr();
	}
}

/*-------------------------------------------------

 Caltron 9 in 1 Board

 Games: 9 in 1 by Caltron

 NES 2.0: mapper 389

 In MAME: Supported.

 -------------------------------------------------*/

void nes_caltron9in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("caltron9in1 write_h, offset: %04x, data: %02x\n", offset, data));
	int nibble = (offset >> 12) & 0x07;
	m_latch[std::min(nibble, 2)] = offset & 0x7f;

	if (BIT(m_latch[1], 1))
	{
		u8 outer = (m_latch[0] >> 2) & ~0x03;
		u8 inner = (m_latch[2] >> 2) & 0x03;
		prg16_89ab(outer | inner);
		prg16_cdef(outer | 0x03);
	}
	else
		prg32(m_latch[0] >> 3);

	if (nibble)
		chr8(((m_latch[1] >> 1) & 0x1c) | (m_latch[2] & 0x03), CHRROM);
	else
		set_nt_mirroring(BIT(m_latch[0], 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

}

/*-------------------------------------------------

 Rumblestation Board

 Games: Rumblestation 15 in 1

 iNES: mapper 46

 In MESS: Supported.

 -------------------------------------------------*/

void nes_rumblestat_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("rumblestation write_m, offset: %04x, data: %02x\n", offset, data));

	m_prg = (m_prg & 0x01) | ((data & 0x0f) << 1);
	m_chr = (m_chr & 0x07) | ((data & 0xf0) >> 1);
	prg32(m_prg);
	chr8(m_chr, CHRROM);
}

void nes_rumblestat_device::write_h(offs_t offset, uint8_t data)
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

 This cart contains a 32KB boot menu that selects
 games from a separate 2MB of ROM. The common iNES
 file arbitrarily puts this first before the 2MB ROM.
 A simpler alternate approach would be to place the
 32KB menu at the end of contiguous PRG. In that case
 pcb_reset should be changed to point to the last
 part of PRG and the +2s and +4 below can be removed.

 iNES: mapper 53

 In MAME: Supported.

 -------------------------------------------------*/

void nes_svision16_device::update_prg()
{
	if (BIT(m_latch1, 4))
	{
		u8 bank = (m_latch1 & 0x0f) << 3 | (m_latch2 & 0x07);
		prg16_89ab(bank + 2);             // +2 due to the 32KB menu
		prg16_cdef((bank | 0x07) + 2);    // +2 due to the 32KB menu
	}
}

void nes_svision16_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("svision16 write_m, offset: %04x, data: %02x\n", offset, data));

	if (!BIT(m_latch1, 4))
	{
		m_latch1 = data;
		update_prg();
		set_nt_mirroring(BIT(m_latch1, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}
}

void nes_svision16_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("svision16 write_h, offset: %04x, data: %02x\n", offset, data));
	m_latch2 = data;
	update_prg();
}

u8 nes_svision16_device::read_m(offs_t offset)
{
	LOG_MMC(("svision16 read_m, offset: %04x\n", offset));

	u8 bank = m_latch1 << 4 | 0x0f;
	return m_prg[((bank + 4) * 0x2000 + offset) % m_prg_size];    // +4 due to the 32KB menu
}

/*-------------------------------------------------

 FARID_UNROM_8-IN-1

 Games: 8 in 1

 NES 2.0: mapper 324

 In MAME: Supported.

 -------------------------------------------------*/

void nes_farid_unrom_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("farid_unrom write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (BIT(data, 7) && !(m_reg & 0x88))
		m_reg = data;
	else
		m_reg = (m_reg & 0x78) | (data & 0x87);

	u8 bank = (m_reg & 0x70) >> 1 | (m_reg & 0x07);
	prg16_89ab(bank);
	prg16_cdef(bank | 0x07);
}

/*-------------------------------------------------

 Bootleg Board KN-42

 Games: 2 in 1 - Big Nose & Big Nose Freaks Out

 NES 2.0: mapper 381

 In MAME: Supported.

 TODO: Big Nose Freaks Out has timing issues like
 many Camerica games. It happens with the singleton
 dump and is unrelated to the bootleg board here.

 -------------------------------------------------*/

void nes_kn42_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("kn42 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg16_89ab(m_latch | (data & 0x07) << 1 | BIT(data, 4));
}

/*-------------------------------------------------

 Board BMC-A65AS

 Games: 3-in-1 (N068)

 In MESS: Supported

 -------------------------------------------------*/

void nes_a65as_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t helper = (data & 0x30) >> 1;
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

 Games: 4-in-1 (D-010), 8-in-1 (A-020), and others

 NES 2.0: mapper 265

 In MAME: Supported.

 -------------------------------------------------*/

void nes_t262_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("t262 write_h, offset: %04x, data: %02x\n", offset, data));

	if (!BIT(m_latch, 13))
	{
		m_latch = offset;
		set_nt_mirroring(BIT(m_latch, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}

	u8 bank = (m_latch & 0x300) >> 3 | (m_latch & 0x60) >> 2 | (data & 0x07);    // NesDev shows the high bit here, but is it correct? So far no cart is large enough to use this.
	u8 mode = BIT(m_latch, 0);
	if (BIT(m_latch, 7))    // NROM mode
	{
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
	else                    // UNROM mode
	{
		prg16_89ab(bank);
		prg16_cdef(bank | 0x07);
	}
}

/*-------------------------------------------------

 Board UNL-STUDYNGAME

 Games: Study n Game 32 in 1

 iNES: mapper 39

 In MESS: Partially Supported (problems with PRG bankswitch,
 only keyboard exercise work).

 -------------------------------------------------*/

void nes_studyngame_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("studyngame write_h, offset: %04x, data: %02x\n", offset, data));
	prg32(data);
}

/*-------------------------------------------------

 Boards BMC-SUPERGUN-20IN1, BMC-190IN1

 Unknown Bootleg Multigame Boards
 Games: Super Gun 20 in 1, Golden 190 in 1

 iNES: mapper 214
 NES 2.0: mapper 300

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sgun20in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("supergun20in1 write_h, offset: %04x, data: %02x\n", offset, data));

// Hogan's Alley in 20-in-1 will occasionally sweep through 0x66xx-0x68xx which
// causes an abrupt goto Bomberman title screen. This mask is a best guess.
	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x7000:
			offset = (offset >> 2) & (m_prg_mask >> 1);
			prg16_89ab(offset);
			prg16_cdef(offset);
			chr8(offset, CHRROM);
	}
}

void nes_bmc_190in1_device::write_h(offs_t offset, u8 data)
{
	nes_sgun20in1_device::write_h(offset, data);
	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-VT5201, BMC-T3H53, BMC-D1038

 Games: Super 35 in 1, TN 95 in 1, 46 in 1, 65 in 1,
 74 in 1, 77 in 1

 iNES: mapper 59

 In MAME: Supported.

 -------------------------------------------------*/

void nes_vt5201_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("vt5201 write_h, offset: %04x, data: %02x\n", offset, data));

	if (!BIT(m_latch, 1))    // lock bit
	{
		m_latch = offset >> 8;

		u8 bank = (offset >> 4) & 0x07;
		u8 mode = !BIT(offset, 7);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);

		chr8(offset & 0x07, CHRROM);
		set_nt_mirroring(BIT(offset, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}
}

u8 nes_vt5201_device::read_h(offs_t offset)
{
	LOG_MMC(("bmc_vt5201 read_h, offset: %04x\n", offset));

	if (BIT(m_latch, 0))
		return (get_open_bus() & ~0x03) | m_jumper;    // TODO: add jumper settings, m_jumper is 0 for now
	else
		return hi_access_rom(offset);
}

/*-------------------------------------------------

 BMC-80013-B

 Games: Cartridge Story I, II, and III

 NES 2.0: mapper 274

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_80013b_device::update_prg()
{
	prg16_89ab(m_latch | (m_reg[1] & 0x70) | m_reg[0]);
	prg16_cdef(m_reg[1]);
}

void nes_bmc_80013b_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_80013b write_h, offset: %04x, data: %02x\n", offset, data));
	if (offset & 0x6000)
	{
		m_reg[1] = data & 0x7f;
		m_latch = !BIT(offset, 14) << 7;
	}
	else
	{
		m_reg[0] = data & 0x0f;
		set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}
	update_prg();
}

/*-------------------------------------------------

 BMC-810544-C-A1, NTDEC 2746

 Games: 200-in-1 Elfland, 14 in 1

 NES 2.0: mapper 261

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_810544c_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_810544c write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = bitswap<4>(offset, 9, 8, 7, 5);
	u8 mode = BIT(offset, 6);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8(offset & 0x0f, CHRROM);
	set_nt_mirroring(BIT(offset, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-830425C-4391T

 Games: Super HiK 6-in-1 (A-030)

 NES 2.0: mapper 320

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_830425c_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_830425c write_h, offset: %04x, data: %02x\n", offset, data));

	if ((offset & 0x7fe0) == 0x70e0)
		m_latch = offset & 0x1f;

	u8 outer = (m_latch & 0x0f) << 3;
	u8 mode = !BIT(m_latch, 4) << 3 | 0x07;
	prg16_89ab(outer | (data & mode));
	prg16_cdef(outer | mode);
}

/*-------------------------------------------------

 BMC-830928C

 Games: 9 in 1

 NES 2.0: mapper 382

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_830928c_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_830928c write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (!BIT(m_latch, 5))
	{
		m_latch = offset & 0x3f;
		set_nt_mirroring(BIT(m_latch, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}

	u8 outer = (m_latch & 0x07) << 3;
	if (BIT(m_latch, 3))    // BNROM mode
		prg32(outer >> 1 | (data & 0x03));
	else                    // UNROM mode
	{
		prg16_89ab(outer | (data & 0x07));
		prg16_cdef(outer | 7);
	}
}

/*-------------------------------------------------

 BMC-850437C

 Games: Super 8-in-1 (JY-050 rev0, JY-085, JY-086)

 NES 2.0: mapper 396

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_850437c_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_850437c write_h, offset: %04x, data: %02x\n", offset, data));

	m_reg[(offset & 0x6000) == 0x2000] = data;    // outer banking is always at 0xa000, mask is a guess

	u8 bank = (m_reg[1] & 0x07) << 3 | (m_reg[0] & 0x07);
	prg16_89ab(bank);
	prg16_cdef(bank | 0x07);

	set_nt_mirroring(BIT(m_reg[1], 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-970630C

 Games: 2 in 1 (NT-811), 4 in 1 1999

 NES 2.0: mapper 380

 In MAME: Supported.

 TODO: Contra has the same graphics glitches as in
 BMC-8157. These are marked as partially supported
 in the softlist, but maybe this is just a BTANB?

 -------------------------------------------------*/

void nes_bmc_970630c_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_970630c write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = (offset >> 2) & 0x1f;
	if (BIT(offset, 9))    // NROM mode
	{
		u8 mode = !BIT(offset, 0);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
	else                   // UNROM-esque mode
	{
		prg16_89ab(bank);
		prg16_cdef(bank | 0x07);
	}

	set_nt_mirroring(BIT(offset, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_latch = BIT(offset, 8);
}

u8 nes_bmc_970630c_device::read_h(offs_t offset)
{
//  LOG_MMC(("bmc_970630c read_h, offset: %04x\n", offset));

	if (m_latch)
		return 0;    // TODO: menu supposedly varies by solder pad value returned here, but it doesn't seem to work...
	else
		return hi_access_rom(offset);
}

/*-------------------------------------------------

 BMC-NTD-03

 Games: ASDER 20 in 1

 NES 2.0: mapper 290

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ntd03_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t pbank = (offset >> 10) & 0x1e;
	uint8_t cbank = ((offset & 0x300) >> 5) | (offset & 0x07);
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

 BMC-CTC-09

 Games: 10 in 1

 NES 2.0: mapper 335

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_ctc09_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_ctc09 write_h, offset: %04x, data: %02x\n", offset, data));

	if (BIT(offset, 14))
	{
		if (BIT(data, 4))
		{
			u8 bank = ((data & 0x07) << 1) | BIT(data, 3);
			prg16_89ab(bank);
			prg16_cdef(bank);
		}
		else
			prg32(data & 0x07);
		set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}
	else
		chr8(data & 0x0f, CHRROM);
}

/*-------------------------------------------------

 Board BMC-GKA

 Unknown Bootleg Multigame Board
 Games: 6 in 1, 54 in 1, 106 in 1

 iNES: mapper 57

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_gka_device::read_m(offs_t offset)
{
	LOG_MMC(("bmc_gka read_m, offset: %04x\n", offset));
	return 0; // TODO: menus differ by jumper/DIP settings readable at 0x6000.
}

void nes_bmc_gka_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_gka write_h, offset: %04x, data: %02x\n", offset, data));

	m_reg[BIT(offset, 11)] = data;

	u8 bank = m_reg[1] >> 5;
	u8 mode = BIT(m_reg[1], 4);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8((m_reg[0] & 0x40) >> 3 | (m_reg[1] & 0x04) | (m_reg[BIT(m_reg[0], 7)] & 0x03), CHRROM);
	set_nt_mirroring(BIT(m_reg[1], 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Board BMC-GKB

 Unknown Bootleg Multigame Board
 Games: 68 in 1, 73 in 1, 98 in 1, and many others

 iNES: mapper 58

 In MAME: Supported.

 TODO: Investigate why Jewel on mc_wq55 crashes after
 its title screen. It further invokes write_h, causing
 bank switching (and these games are all NROM/CNROM so
 they should never switch PRG once the menu loads them),
 but ignoring those writes doesn't fix the problem. Is
 the game non-working on hardware? A bad dump?

 -------------------------------------------------*/

void nes_bmc_gkb_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_gkb write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = offset & 0x07;
	u8 mode = !BIT(offset, 6);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8((offset >> 3) & 0x07, CHRROM);
	set_nt_mirroring(BIT(offset, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Board BMC-GKCXIN1

 Unknown Bootleg Multigame Board
 Games: 21 in 1, 64 in 1

 NES 2.0: mapper 288

 In MAME: Supported.

 TODO: This has some sort of solder pad settings.

 -------------------------------------------------*/

void nes_bmc_gkcxin1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_gkcxin1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32((offset >> 3) & 0x03);
	chr8(offset & 0x07, CHRROM);
}

/*-------------------------------------------------

 Board BMC-GN-91B

 Unknown Bootleg Multigame Board
 Games: 2 in 1

 NES 2.0: mapper 431

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_gn91b_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_gn91b write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x4000)
	{
		m_latch = (offset & 0x10) >> 1;
		prg16_cdef(m_latch | 0x07);
		set_nt_mirroring(m_latch ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}
	prg16_89ab(m_latch | (data & 0x07));
}

/*-------------------------------------------------

 Board HP-898F

 Games: Prima soft 9999999 in 1 and others

 NES 2.0: mapper 319

 In MAME: Supported.

 TODO: Mirroring is clearly incorrect for Ninja-kun
 on 4 in 1 (0207). Is this an error on the original
 cart or should there really be a variant device?

 -------------------------------------------------*/

u8 nes_bmc_hp898f_device::read_l(offs_t offset)
{
	LOG_MMC(("bmc_hp898f read_l, offset: %04x\n", offset));

	offset += 0x100;
	if (offset & 0x1000)
		return 0;    // FIXME: some carts read jumpers that change menu

	return get_open_bus();
}

void nes_bmc_hp898f_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_hp898f write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x04)
	{
		u8 bank = bitswap<3>(data, 4, 3, 5);
		u8 mode = BIT(data, 6);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	}
	else
		chr8(data >> 4, CHRROM);
}

/*-------------------------------------------------

 BMC-K-3036

 Games: 35 in 1

 NES 2.0: mapper 340

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_k3036_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_k3036 write_h, offset: %04x, data: %02x\n", offset, data));
	u8 bank = offset & 0x1f;
	prg16_89ab(bank);
	prg16_cdef(bank | (BIT(offset, 5) ? 0 : 7));
	set_nt_mirroring((offset & 0x25) == 0x25 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-K-3046

 Games: 11 in 1

 NES 2.0: mapper 336

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_k3046_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_k3046 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	data &= 0x1f;
	prg16_89ab(data);
	prg16_cdef(data | 0x07);
}

/*-------------------------------------------------

 BMC-SA005-A

 Games: 16 in 1

 NES 2.0: mapper 338

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_sa005a_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_sa005a write_h, offset: %04x, data: %02x\n", offset, data));
	u8 bank = offset & 0x0f;
	prg16_89ab(bank);
	prg16_cdef(bank);
	chr8(bank, CHRROM);
	set_nt_mirroring(BIT(offset, 3) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
}

/*-------------------------------------------------

 BMC-TF2740

 Games: 14 in 1, 40 in 1, 158 in 1, 9999 in 1,
 10000000 in 1

 NES 2.0: mapper 428

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_tf2740_device::update_chr()
{
	chr8(m_reg[2] & 0xc0 ? (m_reg[1] & 0x04) | (m_reg[0] & 0x03) : m_reg[1] & 0x07, CHRROM);
}

void nes_bmc_tf2740_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_tf2740 write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x03)
	{
		case 0x01:
		{
			u8 bank = data >> 5;
			u8 mode = BIT(data, 4);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);

			set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			[[fallthrough]];
		}
		case 0x02:
			m_reg[offset & 0x03] = data;
			update_chr();
			break;
	}
}

void nes_bmc_tf2740_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_tf2740 write_h, offset: %04x, data: %02x\n", offset, data));
	m_reg[0] = data;
	update_chr();
}

u8 nes_bmc_tf2740_device::read_m(offs_t offset)
{
	LOG_MMC(("bmc_tf2740 read_m, offset: %04x\n", offset));
	return (get_open_bus() & ~0x03) | m_jumper;    // TODO: add jumper settings, m_jumper is 0 for now
}

/*-------------------------------------------------

 BMC-TJ-03

 Games: 4 in 1

 NES 2.0: mapper 341

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_tj03_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_tj03 write_h, offset: %04x, data: %02x\n", offset, data));
	u8 bank = (offset >> 8) & 0x03;
	prg32(bank);
	chr8(bank, CHRROM);
	set_nt_mirroring(BIT(offset, 9) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Board BMC-WS

 Games: Super 40-in-1

 In MESS: Partially Supported (some games, like Galaxian, have
 issues)

 -------------------------------------------------*/

void nes_bmc_ws_device::write_m(offs_t offset, uint8_t data)
{
	uint8_t mmc_helper;
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

 Board BMC-11160 by TXC

 Games: 6 in 1 (MGC-023)

 NES 2.0: mapper 299

 In MAME: Partially supported. (Light gun hit detection is broken)

 -------------------------------------------------*/

void nes_bmc_11160_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_11160 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32((data & 0x30) >> 4);
	chr8(((data & 0x30) >> 2) | (data & 0x03), CHRROM);
	set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
}

/*-------------------------------------------------

 Board BMC-G-146

 Games: 1994 Super HIK 14 in 1 (G-136)

 NES 2.0: mapper 349

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_g146_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_g146 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 prg_lo = offset & 0x1f;
	u8 prg_hi = prg_lo;         // default: NROM-128 mode
	if (BIT(offset, 11))
		prg_hi |= 0x07;     // UNROM mode
	else if (!BIT(offset, 6))
	{
		prg_lo &= ~0x01;    // NROM-256 mode
		prg_hi |= 0x01;
	}
	prg16_89ab(prg_lo);
	prg16_cdef(prg_hi);

	set_nt_mirroring(BIT(offset, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Board BMC-2751

 Games: 5 in 1

 iNES: mapper 174

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_2751_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_2751 write_h, offset: %04x, data: %02x\n", offset, data));
	u8 bank = (offset >> 4) & 0x07;
	u8 mode = BIT(offset, 7);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);
	chr8((offset >> 1) & 0x07, CHRROM);
	set_nt_mirroring(BIT(offset, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Board BMC-8157

 Games: 4 in 1 1993 (CK-001)

 NES 2.0: mapper 301

 In MAME: Preliminary supported.

 TODO: Determine the cause of Contra graphics glitches.
 Is NesDev description of board wrong? It seems b9 = 1
 and b7 = 0 always? Also couldn't see evidence of the
 mirroring bit being used.

 -------------------------------------------------*/

void nes_bmc_8157_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_8157 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = (offset >> 2) & 0x1f;
	prg16_89ab(bank);
	if (BIT(offset, 9))
		bank |= 0x07;
	else if (BIT(offset, 7))
		bank &= ~0x07;
	prg16_cdef(bank);

	m_latch = BIT(offset, 8);
	set_nt_mirroring(BIT(offset, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

u8 nes_bmc_8157_device::read_h(offs_t offset)
{
	LOG_MMC(("bmc_8157 read_h, offset: %04x\n", offset));
	if (m_latch)
		offset = (offset & ~0x01) | m_jumper->read();
	return hi_access_rom(offset);
}

/*-------------------------------------------------

 BMC-SUPERHIK_300IN1

 Unknown Bootleg Multigame Board
 Games: 100000 in 1, Super HIK 300 in 1, 1997 in 1

 iNES: mapper 212

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_hik300_device::read_m(offs_t offset)
{
	LOG_MMC(("bmc_hik300 read_m, offset: %04x, data: %02x\n", offset));
	return get_open_bus() | (!BIT(offset, 4) << 7);    // some games have a protection MSB
}

void nes_bmc_hik300_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_hik300 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = offset & 0x07;
	u8 mode = BIT(offset, 14);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8(bank, CHRROM);
	set_nt_mirroring(BIT(offset, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-SUPER-700IN1

 Unknown Bootleg Multigame Board
 Games: Super 700 in 1, Super 190 in 1

 iNES: mapper 62

 In MAME: Supported.

 TODO:
 - Investigate why Gradius menu items 643-649 crash
   700-in-1 after pressing start at title screen.
   Gradius works from other menu numbers fine.
 - Slalom and Star Force have the wrong mirroring
   on 190-in-1. Bug on cart, bad dump, or is there
   another bit that affects mirroring?

 -------------------------------------------------*/

void nes_bmc_s700_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_s700 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = (offset & 0x40) | ((offset >> 8) & 0x3f);
	u8 mode = !BIT(offset, 5);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8((offset & 0x1f) << 2 | (data & 0x03), CHRROM);
	set_nt_mirroring(BIT(offset, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-BALLGAMES-11IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: 11 in 1 Ball Series

 iNES: mapper 51

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_ball11_device::read_m(offs_t offset)
{
	LOG_MMC(("bmc_ball11 read_m, offset: %04x, data: %02x\n", offset));

	u8 bank = m_reg[1] << 2 | (m_reg[0] ? 0x23 : 0x2f);
	return m_prg[(bank * 0x2000 + offset) & (m_prg_size - 1)];
}

void nes_bmc_ball11_device::update_prg()
{
	if (m_reg[0])
		prg32(m_reg[1]);
	else
	{
		prg16_89ab(m_reg[1] << 1 | BIT(m_reg[1], 4));
		prg16_cdef(m_reg[1] << 1 | 0x07);
	}
}

void nes_bmc_ball11_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_ball11 write_m, offset: %04x, data: %02x\n", offset, data));
	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_reg[0] = data & 0x02;
	update_prg();
}

void nes_bmc_ball11_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_ball11 write_h, offset: %04x, data: %02x\n", offset, data));
	m_reg[1] = data & 0x1f;
	update_prg();
}

/*-------------------------------------------------

 BMC-22GAMES

 Unknown Bootleg Multigame Board
 Games: Contra/22 in 1 combo

 iNES: mapper 230

 In MAME: Supported.

 TODO: Determine whether reset works as written or
 whether it toggles between Contra and the game menu.
 Reset to Contra from the menu or other games often
 fails due to RAM contents it seems.

 -------------------------------------------------*/

void nes_bmc_22games_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_22games write_h, offset: %04x, data: %02x\n", offset, data));

	if (m_latch)
	{
		u8 bank = (data & 0x1f) + 8;
		u8 mode = !BIT(data, 5);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
		set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	}
	else
		prg16_89ab(data & 0x07);
}

/*-------------------------------------------------

 Board BMC-64IN1NOREPEAT

 Games: 64-in-1 Y2K

 In MESS: Supported

 -------------------------------------------------*/

void nes_bmc_64y2k_device::set_prg()
{
	uint8_t helper1 = (m_reg[1] & 0x1f);
	uint8_t helper2 = (helper1 << 1) | ((m_reg[1] & 0x40) >> 6);

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

void nes_bmc_64y2k_device::write_l(offs_t offset, uint8_t data)
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

void nes_bmc_64y2k_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("bmc64y2k write_h, offset: %04x, data: %02x\n", offset, data));

	m_reg[3] = data;    // reg[3] is currently unused?!?
}

/*-------------------------------------------------

 Board BMC-TELETUBBIES
 (name assigned by BootGod who said the board has
 no markings, a glop top, and an 8K VRAM chip)

 Games: Y2K 420 in 1

 iNES: mapper 237

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_420y2k_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_420y2k write_h, offset: %04x, data: %02x\n", offset, data));

	if (BIT(m_latch, 1))    // lock bit
		m_reg = (m_reg & ~0x07) | (data & 0x07);
	else
	{
		m_latch = offset;
		m_reg = data;
	}

	u8 bank = BIT(m_latch, 2) << 5 | (m_reg & 0x1f);
	u8 mode = BIT(m_reg, 6);
	prg16_89ab(bank & ~mode);    // strangely for UNROM games it DOESN'T ignore the NROM mode bit
	prg16_cdef(bank | (BIT(m_reg, 7) ? mode : 0x07));

	set_nt_mirroring(BIT(m_reg, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

u8 nes_bmc_420y2k_device::read_h(offs_t offset)
{
	LOG_MMC(("bmc_420y2k read_h, offset: %04x\n", offset));
	// latch bit 0 is only used to determine the menu, and the behavior of
	// this cart seems hardwired to OR $02 (ORing $00-$03 allows four menus)
	return hi_access_rom(offset | (m_latch & 1) << 1);
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

void nes_bmc_12in1_device::write_h(offs_t offset, uint8_t data)
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

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_20in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_20in1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(offset & 0x1e);
	prg16_cdef((offset & 0x1e) | BIT(offset, 5));
	set_nt_mirroring(BIT(offset, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-21IN1, BMC-NOVELDIAMOND

 Unknown Bootleg Multigame Board
 Games: 8 in 1, 21 in 1, 9999999 in 1

 iNES: mapper 201

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_21in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_21in1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(offset & 0x03);
	chr8(offset & 0x07, CHRROM);
}

/*-------------------------------------------------

 BMC-31IN1

 Unknown Bootleg Multigame Board
 Games: 31 in 1

 iNES: mapper 229

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_31in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_31in1 write_h, offset: %04x, data: %02x\n", offset, data));
	set_nt_mirroring(BIT(offset, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	offset &= 0x1f;
	if (offset & 0x1e)
	{
		prg16_89ab(offset);
		prg16_cdef(offset);
	}
	else
		prg32(0);

	chr8(offset, CHRROM);
}

/*-------------------------------------------------

 BMC-35IN1

 Unknown Bootleg Multigame Board
 Games: 35 in 1

 iNES: mapper 203

 In MESS: Supported.

 -------------------------------------------------*/

void nes_bmc_35in1_device::write_h(offs_t offset, uint8_t data)
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

void nes_bmc_36in1_device::write_h(offs_t offset, uint8_t data)
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

void nes_bmc_64in1_device::write_h(offs_t offset, uint8_t data)
{
	int bank = (offset >> 1) & (offset >> 2) & 0x01;

	LOG_MMC(("bmc_64in1 write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(offset & ~bank);
	prg16_cdef(offset | bank);
	chr8(offset & ~bank, CHRROM);

	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ: PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-70IN1 (FIXME: according to NesDev the boards for this
 and BMC-800IN1 are Realtec 8031 and 8155. Which is which?)

 Games: 35 in 1, 68 in 1, 70 in 1

 iNES: mapper 236

 This is the same hardware as BMC-800IN1 below, but
 these carts have CHRROM and lack the extended PRG
 bank switching. Solder pads/jumpers not emulated yet.

 In MAME: Supported.

 TODO: Determine if Excitebike on 68 in 1 is actually
 bugged on the real cartridge. It sets mirroring bit
 to horizontal when it should be vertical.

 -------------------------------------------------*/

void nes_bmc_70in1_device::update_banks()
{
	update_prg(m_latch[1] & 0x07);
	chr8(m_latch[0] & 0x07, CHRROM);
}

void nes_bmc_70in1_device::update_prg(u8 bank)
{
	if (BIT(m_latch[1], 5))    // NROM mode
	{
		u8 mode = !BIT(m_latch[1], 4);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
	else                       // UNROM mode
	{
		prg16_89ab(bank);
		prg16_cdef(bank | 0x07);
	}
}

void nes_bmc_70in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc70in1 write_h, offset: %04x, data: %02x\n", offset, data));
	m_latch[BIT(offset, 14)] = offset;
	update_banks();
	set_nt_mirroring(BIT(m_latch[0], 5) ? PPU_MIRROR_HORZ: PPU_MIRROR_VERT);
}

uint8_t nes_bmc_70in1_device::read_h(offs_t offset)
{
	LOG_MMC(("bmc70in1 read_h, offset: %04x\n", offset));

	if ((m_latch[1] & 0x30) == 0x10)
		offset = (offset & ~0x0f) | m_jumper;    // TODO: make jumper selectable

	return hi_access_rom(offset);
}

/*-------------------------------------------------

 BMC-800IN1

 Games: 800 in 1

 iNES: mapper 236

 This is the same hardware as BMC-70IN1, but this
 cart supports larger PRG and has unbanked CHRRAM.

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_800in1_device::update_banks()
{
	update_prg((m_latch[0] & 0x07) << 3 | (m_latch[1] & 0x07));
}

/*-------------------------------------------------

 BMC-72IN1, BMC-110IN1

 Unknown Bootleg Multigame Board
 Games: 72 in 1, 115 in 1 and other multigame carts

 iNES: mappers 225, 255

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_72in1_device::read_l(offs_t offset)
{
	LOG_MMC(("bmc_72in1 read_l, offset: %04x\n", offset));

	offset += 0x100;
	if (offset >= 0x1800)    // 4 nibbles of RAM mirrored above 0x5800
		return m_extra_ram[offset & 0x03];

	return get_open_bus();
}

void nes_bmc_72in1_device::write_l(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_72in1 write_l, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;
	if (offset >= 0x1800)    // 4 nibbles of RAM mirrored above 0x5800
		m_extra_ram[offset & 0x03] = data & 0x0f;
}

void nes_bmc_72in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_72in1 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 high = (offset >> 8) & 0x40;
	u8 bank = high | ((offset >> 6) & 0x3f);
	u8 mode = !BIT(offset, 12);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8(high | (offset & 0x3f), CHRROM);
	set_nt_mirroring(BIT(offset, 13) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-76IN1, BMC-SUPER42IN1, BMC-GHOSTBUSTERS63IN1

 Unknown Bootleg Multigame Board
 Games: 76 in 1, Super 42 in 1, Ghostbusters 63 in 1

 iNES: mapper 226

 In MAME: Supported.

 TODO: Investigate nature of GB63's PCB. It likely has
 three 512KB chips and it seems the two high-order bits
 here select between them. However, 0,1 select the first
 chip and 2,3 correspond to the other two respectively.
 Does the PCB have an empty socket for a fourth chip in
 position 1? We mirror the first chip by reloading in
 the softlist, but should this really be open bus?

 -------------------------------------------------*/

void nes_bmc_76in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_76in1 write_h, offset: %04x, data: %02x\n", offset, data));
	m_reg[offset & 0x01] = data;

	u8 bank = m_reg[1] << 6 | (m_reg[0] & 0x80) >> 2 | (m_reg[0] & 0x1f);
	u8 mode = !BIT(m_reg[0], 5);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	set_nt_mirroring(BIT(m_reg[0], 6) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
}

/*-------------------------------------------------

 BMC-150IN1

 Unknown Bootleg Multigame Board
 Games: 150 in 1

 iNES: mapper 202

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_150in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_150in1 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = (offset >> 1) & 0x07;
	u8 mode = (bank & 0x06) == 0x06;
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8(bank, CHRROM);
	set_nt_mirroring(BIT(offset, 0) ? PPU_MIRROR_HORZ: PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-500IN1

 Unknown Bootleg Multigame Board
 Games: 500 in 1, 2000 in 1 Unchained Melody, etc

 iNES: mapper 217

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_500in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc500in1 write_h, offset: %04x, data: %02x\n", offset, data));
	prg32((offset >> 2) & 0x07);
	chr8(offset & 0x07, CHRROM);
}

/*-------------------------------------------------

 BMC-1200IN1

 Unknown Bootleg Multigame Board
 Games: 1200 in 1, 295 in 1, 76 in 1

 iNES: mapper 227

 In MESS: Preliminary Supported.

 -------------------------------------------------*/

void nes_bmc_1200in1_device::chr_w(offs_t offset, uint8_t data)
{
	int bank = offset >> 10;

	if (!m_vram_protect)
		m_chr_access[bank][offset & 0x3ff] = data;
}


void nes_bmc_1200in1_device::write_h(offs_t offset, uint8_t data)
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

void nes_bmc_gold260_device::write_h(offs_t offset, uint8_t data)
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


void nes_bmc_gold150_device::write_h(offs_t offset, uint8_t data)
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

uint8_t nes_bmc_gold150_device::read_h(offs_t offset)
{
	LOG_MMC(("bmc_gold150 read_h, offset: %04x\n", offset));

	if (m_latch)    // open bus
		return get_open_bus();
	else
		return hi_access_rom(offset);
}

/*-------------------------------------------------

 BMC-RESETBASED4IN1

 Unknown Bootleg Multigame Board

 Games: 4 in 1 (several of them)

 iNES: mapper 60

 No need to use handlers. At reset the banks change
 and so does the game.

 In MAME: Supported.

 -------------------------------------------------*/

/*-------------------------------------------------

 BMC-42IN1RESETBASED, BMC-NC-20MB

 Games: 42 (22 + 20) in 1, 20 in 1 (CA-006)

 Here we implement two nearly identical mappers. The
 NC-20MB board does not have the reset latch and its
 mirroring bit is inverted from the 42 in 1.

 There is an identical dump of the 42 in 1 identified
 as SUPER22GAMES that supposedly selected between the
 two menus via dip switch. It's not clear which, if
 any, variants of this cart exist, nor if this is
 really reset-based.

 iNES: mapper 233
 NES 2.0: mapper 433

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_42in1reset_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_42in1reset write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = m_latch | (data & 0x1f);
	u8 mode = !BIT(data, 5);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	// some sources say there are 1-screen modes for 42-in-1, but this works as is
	set_nt_mirroring(BIT(data, 6) ^ m_mirror_flip ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
}

/*-------------------------------------------------

 BMC-LITTLECOM-160

 Unknown Bootleg Multigame Board

 Games: Little Com 160

 NES 2.0: mapper 541

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_lc160_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_lc160 write_h, offset: %04x, data: %02x\n", offset, data));
	if (offset >= 0x4000)
	{
		u8 bank = (offset >> 2) & 0x3f;
		u8 mode = !BIT(offset, 1);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
		set_nt_mirroring(BIT(offset, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	}
}

/*-------------------------------------------------

 BMC-60311C

 Games: 17 in 1

 NES 2.0: mapper 289

 In MAME: Preliminary supported.

 TODO: All the Capcom games and Konami's Hyper Soccer
 sound horrible here. BTANB? Also Journey to the West
 doesn't work as an individual rom (jwest) or here.

 -------------------------------------------------*/

void nes_bmc_60311c_device::update_banks()
{
	if (BIT(m_reg[0], 1))    // UNROM mode
	{
		prg16_89ab((m_reg[1] & ~0x07) | (BIT(m_reg[0], 0) ? 0x07 : m_reg[2]));
		prg16_cdef(m_reg[1] | 0x07);
	}
	else                     // NROM mode
	{
		prg16_89ab(m_reg[1] & ~BIT(m_reg[0], 0));
		prg16_cdef(m_reg[1] | BIT(m_reg[0], 0));
	}

	set_nt_mirroring(BIT(m_reg[0], 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_vram_protect = BIT(m_reg[0], 2);
}

void nes_bmc_60311c_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_60311c write_m, offset: %04x, data: %02x\n", offset, data));
	m_reg[offset & 1] = data & 0x7f;
	update_banks();
}

void nes_bmc_60311c_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_60311c write_h, offset: %04x, data: %02x\n", offset, data));
	m_reg[2] = data & 0x07;
	update_banks();
}

/*-------------------------------------------------

 BMC-CTC-12IN1

 Games: 12 in 1 Game Card

 NES 2.0: mapper 337

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_ctc12in1_device::read_m(offs_t offset)
{
	LOG_MMC(("bmc_ctc12in1 read_m, offset: %04x, data: %02x\n", offset));
	return m_prg[0x01 * 0x2000 + offset];    // fixed to bank 1
}

void nes_bmc_ctc12in1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_ctc12in1 write_h, offset: %04x, data: %02x\n", offset, data));
	m_reg[BIT(offset, 14)] = data;

	u8 bank = (m_reg[0] & 0x18) | (m_reg[1] & 0x07);
	switch (m_reg[0] >> 6)
	{
		case 0:
			prg16_89ab(bank);
			prg16_cdef(bank);
			break;
		case 1:
			prg32(bank >> 1);
			break;
		case 2:
		case 3:
			prg16_89ab(bank);
			prg16_cdef(bank | 0x07);
			break;
	}

	m_vram_protect = !BIT(m_reg[0], 7) || (offset & 0x6000) == 0x6000;
	set_nt_mirroring(BIT(m_reg[0], 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-891227

 Games: Super 15 in 1 Game Card

 NES 2.0: mapper 350

 This board is similar to CTC-12IN1 above but with
 the mirroring and banking mode bits moved and with
 support for Contra on a separate 128K chip.

 In MAME: Supported.

 TODO: Contra bugs: Without input character sprite
 is always in the UP position. Cannot pickup weapons.

 -------------------------------------------------*/

void nes_bmc_891227_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_891227 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x4000)
		 data = (data & 0x80) >> 2 | (data & 0x60) << 1 | (data & 0x1f);

	nes_bmc_ctc12in1_device::write_h(offset, data);

	if ((m_reg[0] & 0xc0) == 0xc0)  // Contra only mode
	{
		prg16_89ab(0x20 | (m_reg[1] & 0x07));
		prg16_cdef(0x27);
	}
}

/*-------------------------------------------------

 BMC-FAM250

 Games: 250 in 1

 This board is very close to the K-1029 board of the
 famous Contra 100 and 168 multicarts. It sports an
 extra mode to support a Bubble Bobble FDS bootleg.

 NES 2.0: mapper 354

 In MAME: Supported.

 TODO: Circus Charlie's mirroring is incorrectly set
 to horizontal. Is this a BTANB?

 -------------------------------------------------*/

u8 nes_bmc_fam250_device::read_m(offs_t offset)
{
// LOG_MMC(("fam250 read_m, offset: %04x\n", offset));

	if (m_latch == 5)    // only the Bubble Bobble FDS bootleg uses this
		return m_prg[(((m_reg & 0x1f) << 1 | BIT(m_reg, 7)) * 0x2000 + offset) & (m_prg_size - 1)];
	else
		return get_open_bus();
}

void nes_bmc_fam250_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("fam250 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x7000)
	{
		nes_bmc_k1029_device::write_h(offset, data);

		m_latch = offset & 0x07;
		m_reg = data;
		if (m_latch == 5)
			prg32((m_reg & 0x18) >> 1 | 0x03);

		m_vram_protect = BIT(offset, 3);
	}
}

/*-------------------------------------------------

 BMC-K-1029

 Games: 100 in 1 Contra Function 16, 168 in 1

 iNES: mapper 15

 In MAME: Supported.

 TODO: Contra games in 100-in-1 endlessly loop the
 second level played regardless of starting level.
 BTANB? Bad dump? FCEUX exhibits the same behavior.

 -------------------------------------------------*/

void nes_bmc_k1029_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_k1029 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = data & 0x3f;
	switch (offset & 3)
	{
		case 0:
			prg32(bank >> 1);
			break;
		case 1:
			prg16_89ab(bank);
			prg16_cdef(bank | 0x07);
			break;
		case 2:
			bank = bank << 1 | BIT(data, 7);
			prg8_89(bank);
			prg8_ab(bank);
			prg8_cd(bank);
			prg8_ef(bank);
			break;
		case 3:
			prg16_89ab(bank);
			prg16_cdef(bank);
			break;
	}

	set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_vram_protect = BIT(offset, 0) == BIT(offset, 1);
}

/*-------------------------------------------------

 Bootleg Board N625092

 Games: 320 in 1, 400 in 1, 700 in 1, 800 in 1,
 1000 in 1, 2000 in 1, 5000000 in 1

 iNES: mapper 221

 In MAME: Supported.

 TODO: Several games have incorrect mirroring bits
 on all carts they appear on. It's subtle so it's
 likely a BTANB? Noticeable in Flappy, Pacman, and
 Warpman title scrolls and at bottom of screen in
 Zippy Race in-game.

 -------------------------------------------------*/

void nes_n625092_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("n625092 write_h, offset: %04x, data: %02x\n", offset, data));

	m_latch[BIT(offset, 14)] = offset;

	u8 bank = (m_latch[0] & 0x200) >> 3 | (m_latch[0] & 0xe0) >> 2 | (m_latch[1] & 0x07);
	u8 mode = BIT(m_latch[0], 1);
	if (mode && BIT(m_latch[0], 8))    // UNROM mode
	{
		prg16_89ab(bank);
		prg16_cdef(bank | 7);
	}
	else                               // NROM mode
	{
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}

	set_nt_mirroring(BIT(m_latch[0], 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_vram_protect = BIT(m_latch[1], 3);
}

/*-------------------------------------------------

 BMC-TH2291-3, BMC-CH-011, BMC-82AB

 Games: Powerful 250 in 1, Powerful 255 in 1, 82 in 1

 iNES: mapper 63

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_th22913_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bmc_th22913 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = offset >> 2;
	u8 mode = BIT(offset, 1);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	set_nt_mirroring(BIT(offset, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_vram_protect = BIT(offset, m_vram_prot_bit);
}
