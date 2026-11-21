// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Multigame Carts PCBs

 iNES mappers: 15, 39, 41, 46, 51, 53, 54, 57, 58, 59, 60, 62, 63, 174, 200, 201, 202, 203, 204,
               212, 213, 214, 217, 221, 225, 226, 227, 228, 229, 230, 231, 233, 235, 236, 237, 242, 255,
               261, 265, 274, 285, 288, 289, 290, 294, 299, 300, 301, 314, 319, 320, 324, 331, 332,
               335, 336, 337, 338, 340, 341, 349, 350, 354, 360, 380, 381, 382, 389, 396, 428, 431,
               433, 434, 437, 438, 452, 541


 Here we emulate several PCBs used in multigame pirate carts (not MMC-3 based)


 ***********************************************************************************************************/


#include "emu.h"
#include "multigame.h"


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
DEFINE_DEVICE_TYPE(NES_BMC_576,        nes_bmc_576_device,        "nes_bmc_576",        "NES Cart BMC 100-in-1 (NES 2.0 mapper 576) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_564,        nes_bmc_564_device,        "nes_bmc_564",        "NES Cart NTDEC 22-in-1 (NES 2.0 mapper 564) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_582,        nes_bmc_582_device,        "nes_bmc_582",        "NES Cart Super Micro Machines 1999 (NES 2.0 mapper 582) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_585,        nes_bmc_585_device,        "nes_bmc_585",        "NES Cart Super 72-in-1 (NES 2.0 mapper 585) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_810544C,    nes_bmc_810544c_device,    "nes_bmc_810544c",    "NES Cart BMC 810544-C-A1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_631601,     nes_bmc_631601_device,     "nes_bmc_631601",     "NES Cart BMC 63-1601 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_830425C,    nes_bmc_830425c_device,    "nes_bmc_830425c",    "NES Cart BMC 830425C-4391T PCB")
DEFINE_DEVICE_TYPE(NES_BMC_830928C,    nes_bmc_830928c_device,    "nes_bmc_830928c",    "NES Cart BMC 830928C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_850437C,    nes_bmc_850437c_device,    "nes_bmc_850437c",    "NES Cart BMC 850437C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_891227,     nes_bmc_891227_device,     "nes_bmc_891227",     "NES Cart BMC 891227 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_970630C,    nes_bmc_970630c_device,    "nes_bmc_970630c",    "NES Cart BMC 970630C PCB")
DEFINE_DEVICE_TYPE(NES_NTD03,          nes_ntd03_device,          "nes_ntd03",          "NES Cart NTD-03 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_CTC09,      nes_bmc_ctc09_device,      "nes_bmc_ctc09",      "NES Cart BMC CTC-09 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_CTC12IN1,   nes_bmc_ctc12in1_device,   "nes_bmc_ctc12in1",   "NES Cart BMC CTC-12IN1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_DS927,      nes_bmc_ds927_device,      "nes_bmc_ds927",      "NES Cart BMC DS-9-27 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_FAM250,     nes_bmc_fam250_device,     "nes_bmc_fam250",     "NES Cart BMC FAM250 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GKA,        nes_bmc_gka_device,        "nes_bmc_gka",        "NES Cart BMC GK-A PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GKB,        nes_bmc_gkb_device,        "nes_bmc_gkb",        "NES Cart BMC GK-B PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GKCXIN1,    nes_bmc_gkcxin1_device,    "nes_bmc_gkcxin1",    "NES Cart BMC GKCXIN1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GN91B,      nes_bmc_gn91b_device,      "nes_bmc_gn91b",      "NES Cart BMC GN-91B PCB")
DEFINE_DEVICE_TYPE(NES_BMC_HP898F,     nes_bmc_hp898f_device,     "nes_bmc_hp898f",     "NES Cart BMC HP-898F PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K1029,      nes_bmc_k1029_device,      "nes_bmc_k1029",      "NES Cart BMC K-1029 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K3036,      nes_bmc_k3036_device,      "nes_bmc_k3036",      "NES Cart BMC K-3036 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K3046,      nes_bmc_k3046_device,      "nes_bmc_k3046",      "NES Cart BMC K-3046 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K3071,      nes_bmc_k3071_device,      "nes_bmc_k3071",      "NES Cart BMC K-3071 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_S009,       nes_bmc_s009_device,       "nes_bmc_s009",       "NES Cart BMC S-009 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_SA005A,     nes_bmc_sa005a_device,     "nes_bmc_sa005a",     "NES Cart BMC SA005-A PCB")
DEFINE_DEVICE_TYPE(NES_BMC_TF2740,     nes_bmc_tf2740_device,     "nes_bmc_tf2740",     "NES Cart BMC TF2740 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_TH2348,     nes_bmc_th2348_device,     "nes_bmc_th2348",     "NES Cart BMC TH2348 PCB")
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
DEFINE_DEVICE_TYPE(NES_BMC_GOLD260,    nes_bmc_gold260_device,    "nes_bmc_gold260",    "NES Cart BMC Golden Game X in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_ET113,      nes_bmc_et113_device,      "nes_bmc_et113",      "NES Cart BMC ET-113 (iNES mapper 242 multicart) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_TH22913,    nes_bmc_th22913_device,    "nes_bmc_th22913",    "NES Cart BMC TH2291-3 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_82AB,       nes_bmc_82ab_device,       "nes_bmc_82ab",       "NES Cart BMC 82AB PCB")
DEFINE_DEVICE_TYPE(NES_BMC_4IN1RESET,  nes_bmc_4in1reset_device,  "nes_bmc_4in1reset",  "NES Cart BMC 4 in 1 (Reset Based) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_42IN1RESET, nes_bmc_42in1reset_device, "nes_bmc_42in1reset", "NES Cart BMC 42 in 1 (Reset Based) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_NC20MB,     nes_bmc_nc20mb_device,     "nes_bmc_nc20mb",     "NES Cart BMC NC-20MB PCB")
DEFINE_DEVICE_TYPE(NES_BMC_LC160,      nes_bmc_lc160_device,      "nes_bmc_lc160",      "NES Cart BMC Little Com 160 PCB")
DEFINE_DEVICE_TYPE(NES_BITCORP3150,    nes_bitcorp3150_device,    "nes_bitcorp3150",    "NES Cart Bit Corp 31-in-1 (ID 3150) PCB")


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

nes_action52_device::nes_action52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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

nes_bmc_631601_device::nes_bmc_631601_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_631601, tag, owner, clock), m_outer(0), m_inner(0)
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

nes_ntd03_device::nes_ntd03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_NTD03, tag, owner, clock)
{
}

nes_bmc_ctc09_device::nes_bmc_ctc09_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_CTC09, tag, owner, clock)
{
}

nes_bmc_ds927_device::nes_bmc_ds927_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_DS927, tag, owner, clock), m_latch(0), m_mode(0)
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
	, m_pad(*this, "PAD")
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

nes_bmc_k3071_device::nes_bmc_k3071_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_K3071, tag, owner, clock)
{
}

nes_bmc_s009_device::nes_bmc_s009_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_S009, tag, owner, clock), m_reg(0)
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

nes_bmc_th2348_device::nes_bmc_th2348_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_TH2348, tag, owner, clock), m_latch(0)
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

nes_bmc_64y2k_device::nes_bmc_64y2k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_64Y2K, tag, owner, clock), m_reg_mask(0)
{
}

nes_bmc_420y2k_device::nes_bmc_420y2k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_420Y2K, tag, owner, clock), m_latch(0), m_reg(0)
{
}

nes_bmc_12in1_device::nes_bmc_12in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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
	: nes_nrom_device(mconfig, NES_BMC_1200IN1, tag, owner, clock)
	, m_pad(*this, "PAD")
	, m_submapper(0), m_pad_mode(0), m_vram_protect(0)
{
}

nes_bmc_gold260_device::nes_bmc_gold260_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_GOLD260, tag, owner, clock)
{
}

nes_bmc_et113_device::nes_bmc_et113_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_ET113, tag, owner, clock)
	, m_pad(*this, "PAD")
	, m_pad_mode(0), m_vram_protect(0)
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
	: nes_vram_protect_device(mconfig, type, tag, owner, clock), m_vram_wp(false)
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
	: nes_vram_protect_device(mconfig, type, tag, owner, clock), m_prg_a20(0)
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
}

void nes_a65as_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
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

	m_latch = 0;
}

void nes_sgun20in1_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRROM);
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

void nes_bmc_631601_device::device_start()
{
	common_start();
	save_item(NAME(m_outer));
	save_item(NAME(m_inner));
}

void nes_bmc_631601_device::pcb_reset()
{
	m_outer = 0;
	m_inner = 0;
	set_nt_mirroring(PPU_MIRROR_VERT);
	update_banks();
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

	m_latch = 0;
}

void nes_bmc_ctc09_device::pcb_reset()
{
// nes_slot's pcb_start sets us up in the main menu. Soft reset is empty so
// that games reset to their own title screens. This seems to be this cart's
// intended behavior as trying to reset to the menu here crashes (due to RAM
// contents?). Soft reset can similarly crash the main menu (BTANB?).
}

void nes_bmc_ds927_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_mode));
}

void nes_bmc_ds927_device::pcb_reset()
{
	assert(m_prgram.size() >= 0x2000);

	prg16_89ab(0);
	prg16_cdef(0);

	m_latch = 0;
	m_mode = 0;
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

	m_latch = 0;
}

void nes_bmc_k3036_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
}

void nes_bmc_k3046_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);
}

void nes_bmc_k3071_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, CHRROM);
}

void nes_bmc_s009_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bmc_s009_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);

	m_reg = 0;
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

void nes_bmc_th2348_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_th2348_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);

	m_latch = 0;
}

void nes_bmc_ws_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_ws_device::pcb_reset()
{
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
}

void nes_bmc_64y2k_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bmc_64y2k_device::pcb_reset()
{
	m_reg_mask = m_chr_source == CHRROM ? 0x03 : 0x01;
	m_reg[0] = 0x80;
	m_reg[1] = 0x43;
	m_reg[2] = m_reg[3] = 0;
	update_banks();
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
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	update_banks();
}

void nes_bmc_35in1_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);
}

void nes_bmc_36in1_device::pcb_reset()
{
	prg16_89ab(m_prg_chunks - 1);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_bmc_64in1_device::pcb_reset()
{
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
	save_item(NAME(m_pad_mode));
	save_item(NAME(m_vram_protect));
}

void nes_bmc_1200in1_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);   // the address latch powers up as 0
	m_pad_mode = 0;
	m_vram_protect = 0;
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
}

void nes_vram_protect_device::device_start()
{
	common_start();
	save_item(NAME(m_vram_protect));
}

void nes_vram_protect_device::pcb_reset()
{
	prg32(0);
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
	save_item(NAME(m_vram_wp));
}

void nes_bmc_ctc12in1_device::pcb_reset()
{
	nes_vram_protect_device::pcb_reset();
	prg16_89ab(0);
	prg16_cdef(0);

	m_reg[0] = m_reg[1] = 0;
	m_vram_wp = false;

	// all bits clear is NROM-128 mode with vertical mirroring, where CHR-RAM is write-protected
	set_nt_mirroring(PPU_MIRROR_VERT);
	m_vram_protect = true;
}

void nes_bmc_k1029_device::pcb_reset()
{
	nes_vram_protect_device::pcb_reset();

	// power-on and reset state is all bits clear: NROM-256 mode with vertical mirroring and write-protected CHR-RAM
	set_nt_mirroring(PPU_MIRROR_VERT);
	m_vram_protect = true;
}

void nes_bmc_fam250_device::device_start()
{
	nes_bmc_k1029_device::device_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
	save_item(NAME(m_prg_a20));
}

void nes_bmc_fam250_device::pcb_reset()
{
	nes_vram_protect_device::pcb_reset();

	// NESdev: power-on and reset value all bits clear (NROM-256 bank 0, vertical mirroring, CHR-RAM writable, PRG A20
	// and A21 clear)
	m_prg_a20 = 0;
	m_latch = 0;
	m_reg = 0;
	prg32(0);
	set_nt_mirroring(PPU_MIRROR_VERT);
	m_vram_protect = 0;
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

void nes_action52_device::write_h(offs_t offset, u8 data)
{
	LOG("ae_act52_w, offset: %04x, data: %02x\n", offset, data);

	u8 bank = BIT(offset, 6, 7);
	u8 mode = !BIT(offset, 5);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8((data & 0x03) | (offset & 0x0f) << 2, CHRROM);
	set_nt_mirroring(BIT(offset, 13) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
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
	LOG("caltron6in1 write_m, offset: %04x, data: %02x\n", offset, data);

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
	LOG("caltron6in1 write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("caltron9in1 write_h, offset: %04x, data: %02x\n", offset, data);
	int nibble = BIT(offset, 12, 3);
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

 In MAME: Supported.

 -------------------------------------------------*/

void nes_rumblestat_device::write_m(offs_t offset, uint8_t data)
{
	LOG("rumblestation write_m, offset: %04x, data: %02x\n", offset, data);

	m_prg = (m_prg & 0x01) | ((data & 0x0f) << 1);
	m_chr = (m_chr & 0x07) | ((data & 0xf0) >> 1);
	prg32(m_prg);
	chr8(m_chr, CHRROM);
}

void nes_rumblestat_device::write_h(offs_t offset, uint8_t data)
{
	LOG("rumblestation write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	m_prg = (m_prg & ~0x01) | BIT(data, 0);
	m_chr = (m_chr & ~0x07) | BIT(data, 4, 3);
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
	LOG("svision16 write_m, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_latch1, 4))
	{
		m_latch1 = data;
		update_prg();
		set_nt_mirroring(BIT(m_latch1, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}
}

void nes_svision16_device::write_h(offs_t offset, u8 data)
{
	LOG("svision16 write_h, offset: %04x, data: %02x\n", offset, data);
	m_latch2 = data;
	update_prg();
}

u8 nes_svision16_device::read_m(offs_t offset)
{
	LOG("svision16 read_m, offset: %04x\n", offset);

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
	LOG("farid_unrom write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("kn42 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg16_89ab(m_latch | (data & 0x07) << 1 | BIT(data, 4));
}

/*-------------------------------------------------

 Board BMC-A65AS

 Games: 3-in-1 (N068)

 In MAME: Supported

 -------------------------------------------------*/

void nes_a65as_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t helper = (data & 0x30) >> 1;
	LOG("a65as write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("t262 write_h, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_latch, 13))
	{
		m_latch = offset;
		set_nt_mirroring(BIT(m_latch, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}

	u8 bank = bitswap<4>(m_latch, 9, 8, 6, 5) << 3 | (data & 0x07);    // NesDev shows the high bit here, but is it correct? So far no cart is large enough to use this.
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

 In MAME: Partially Supported (problems with PRG bankswitch,
 only keyboard exercise work).

 -------------------------------------------------*/

void nes_studyngame_device::write_h(offs_t offset, uint8_t data)
{
	LOG("studyngame write_h, offset: %04x, data: %02x\n", offset, data);
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
	LOG("supergun20in1 write_h, offset: %04x, data: %02x\n", offset, data);

// Hogan's Alley in 20-in-1 will occasionally sweep through 0x66xx-0x68xx which
// causes an abrupt goto Bomberman title screen. This mask is a best guess.
	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x7000:
			offset >>= 2;
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
	LOG("vt5201 write_h, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_latch, 1))    // lock bit
	{
		m_latch = offset >> 8;

		u8 bank = BIT(offset, 4, 3);
		u8 mode = !BIT(offset, 7);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);

		chr8(offset & 0x07, CHRROM);
		set_nt_mirroring(BIT(offset, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}
}

u8 nes_vt5201_device::read_h(offs_t offset)
{
	LOG("bmc_vt5201 read_h, offset: %04x\n", offset);

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
	LOG("bmc_80013b write_h, offset: %04x, data: %02x\n", offset, data);
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
	LOG("bmc_810544c write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = bitswap<4>(offset, 9, 8, 7, 5);
	u8 mode = BIT(offset, 6);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8(offset & 0x0f, CHRROM);
	set_nt_mirroring(BIT(offset, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-63-1601

 Games: 16-in-1 (1682), 2 MiB of UNROM games

 UNROM with an outer bank register at $4100 (mask $C100, so any
 address with A14 and A8 set and A15 clear): bits 3-0 select the
 128 KiB outer PRG-ROM bank (PRG A20-A17) and bit 4 the mirroring
 (0 = vertical, 1 = horizontal). The inner bank register is the
 usual UNROM one at $8000-$FFFF, with bank 7 of the outer bank
 fixed at $C000.

 NES 2.0: mapper 294

 In MAME: Supported from the NESdev description; no mapper 294
 cart was available to test.

 -------------------------------------------------*/

void nes_bmc_631601_device::update_banks()
{
	prg16_89ab(m_outer << 3 | m_inner);
	prg16_cdef(m_outer << 3 | 0x07);
}

void nes_bmc_631601_device::write_outer(u8 data)
{
	m_outer = data & 0x0f;
	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	update_banks();
}

void nes_bmc_631601_device::write_l(offs_t offset, u8 data)
{
	LOG("bmc_631601 write_l, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset + 0x100, 8))
		write_outer(data);
}

void nes_bmc_631601_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_631601 write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 8))
		write_outer(data);
}

void nes_bmc_631601_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_631601 write_h, offset: %04x, data: %02x\n", offset, data);

	m_inner = data & 0x07;
	update_banks();
}

/*-------------------------------------------------

 BMC-830425C-4391T

 Games: Super HiK 6-in-1 (A-030)

 NES 2.0: mapper 320

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_830425c_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_830425c write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_830928c write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_850437c write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_970630c write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = BIT(offset, 2, 5);
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
	LOGMASKED(LOG_HIFREQ, "bmc_970630c read_h, offset: %04x\n", offset);

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

void nes_ntd03_device::write_h(offs_t offset, u8 data)
{
	LOG("ntd03 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = bitswap<5>(offset, 14, 13, 12, 11, 6);
	u8 mode = !BIT(offset, 7);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8(bitswap<5>(offset, 9, 8, 2, 1, 0), CHRROM);
	set_nt_mirroring(BIT(offset, 10) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-CTC-09

 Games: 10 in 1

 NES 2.0: mapper 335

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_ctc09_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_ctc09 write_h, offset: %04x, data: %02x\n", offset, data);

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

 Board DS-9-27

 Games: 190 in 1

 Bizarro board of mostly simple games that has
 8K of WRAM that is mappable, with mirroring in
 NROM128 mode, to anywhere in upper memory.

 NES 2.0: mapper 452

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_ds927_device::read_h(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "bmc_ds927 read_h, offset: %04x\n", offset);

	int bits = m_mode == 1 ? 1 : 2;

	if (BIT(offset, 13, bits) == BIT(m_latch, 4, bits))
		return m_prgram[offset & 0x1fff];

	return hi_access_rom(offset);
}

void nes_bmc_ds927_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_ds927 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset < 0x6000)
	{
		m_latch = data;
		m_mode = bitswap<2>(data, 3, 1);

		int bank = BIT(offset, 1, 7);

		switch (m_mode)
		{
			case 0:
				prg16_89ab(bank >> 1);
				prg16_cdef(0);
				break;
			case 1:
				for (int i = 0; i < 4; i++)
					prg8_x(i, bank);
				break;
			case 2:
			case 3:
				prg8_89(bank);
				prg8_ab(bank | 1);
				prg8_cd(bank | 2);
				prg8_ef(bank | 3 | (data & 0x04));
				break;
		}
	}

	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	int bits = m_mode == 1 ? 1 : 2;

	if (BIT(offset, 13, bits) == BIT(m_latch, 4, bits))
		m_prgram[offset & 0x1fff] = data;
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
	LOG("bmc_gka read_m, offset: %04x\n", offset);
	return 0; // TODO: menus differ by jumper/DIP settings readable at 0x6000.
}

void nes_bmc_gka_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_gka write_h, offset: %04x, data: %02x\n", offset, data);

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

 NESdev INES Mapper 058 (and 213, a duplicate): a write to $8000-$FFFF
 sets, from its address (A15 1... .... MSCC CPPP): PPP: PRG A16-A14;
 CCC: CHR A15-A13; S: 0 is NROM-256 (PRG A14 is CPU A14), 1 is
 NROM-128; M: mirroring, 0 vertical, 1 horizontal.

 Jewel on mc_wq55 crashes after its title screen, and that dump is
 flagged as a bad dump: after Start the CPU runs zero page (data, with
 nothing on the stack) and the garbage it executes writes the address
 latch. The writes are a result of the crash, not a bank switch made
 by the game.

 Checked: the menus of 68-in-1 (WQ2006 D), 77-in-1 (WQ1806 E), 122-in-1
 (MC3122) and 55-in-1 (WQ2006 B), the last being mc_wq55, the dump flagged
 as bad, launch a game with a RAM routine that writes the latch and jumps
 to an entry address (the 55-in-1 keeps six such records). For these nine
 records the window the page's decode maps has a valid reset target (the
 stub at $FFF2 or startup code) in eight and real code in the ninth, and
 five with S read the other way round; C036 in the 68-in-1 settles S: S=0
 maps banks 6 and 7, whose vectors lead to the stub, while S=1 would
 mirror bank 6, whose reset vector is not a ROM address. Legend of Kage
 in the 122-in-1 writes the latch every frame, alternating CCC 2 and 3 as
 CNROM-style CHR switching, draws correctly, and has M=1 (horizontal
 mirroring) while scrolling vertically; no dump isolates the mirroring bit
 further. The 55-in-1 menu writes $8038 at frame 0, so the power-on
 mirroring and CHR bank do not matter there; the power-on PRG mapping
 (banks 0 and 1) does, and the dump boots to its menu with it. Jewel's
 crash is the same with S cleared.

 -------------------------------------------------*/

void nes_bmc_gkb_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_gkb write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = offset & 0x07;
	u8 mode = !BIT(offset, 6);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8(BIT(offset, 3, 3), CHRROM);
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
	LOG("bmc_gkcxin1 write_h, offset: %04x, data: %02x\n", offset, data);

	prg32(BIT(offset, 3, 2));
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
	LOG("bmc_gn91b write_h, offset: %04x, data: %02x\n", offset, data);

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

 NESdev NES 2.0 Mapper 319 (HP-898F and KD-7/9-E boards): a write to
 $6000-$7FFF with A2 set ($6004) selects the 16 KiB PRG banks (PRG A15
 is bit 3, A16 bit 4, A14 bit 5, or CPU A14 when bit 6 is set) and the
 mirroring (bit 7: 0 horizontal, 1 vertical); with A2 clear ($6000),
 bits 7-4 select the 8 KiB CHR bank. A read of $5000-$5FFF returns the
 solder pad in bit 6 and the rest of the bits as open bus (the page
 documents only the pad). The pad picks the menu, and is a machine
 configuration setting.

 Checked: in 9999999-in-1 - 14 Giochi X-140 (Italy) (En) (Pirate).nes
 and Olympic 2000 - 1000000-in-1 (Asia) (En) (Pirate).nes the routine
 that reads $5FF0 keeps the pad bit ($C1 and $80), which now follows the
 setting; the first dump's menu title is 9999999 IN 1 at pad 0 and
 1000000 IN 1 at pad 1.

 Checked: Super Game 4-in-1 (KG-410), whose menu reads 0210, writes
 $6004 and $6000 as C8/10 for Adventure Island, 30/60 for Ninja-kun,
 90/20 for Mappy and 20/40 for Road Fighter. Ninja-kun is PRG bank 5 and
 CHR bank 6, as those values decode. The mirroring bit gives vertical for
 Adventure Island and Mappy and horizontal for Ninja-kun and Road
 Fighter, the same as the headers of the original games, so no variant
 board is needed. Ninja-kun on 4 in 1 (0207) is mc_4z207, whose PRG and
 CHR the software list flags as bad dumps; it was not available to test.

 -------------------------------------------------*/

static INPUT_PORTS_START( hp898f )
	PORT_START("PAD")
	PORT_CONFNAME( 0x01, 0x00, "Solder pad (menu)" )
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x01, "1" )
INPUT_PORTS_END

ioport_constructor nes_bmc_hp898f_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( hp898f );
}

u8 nes_bmc_hp898f_device::read_l(offs_t offset)
{
	LOG("bmc_hp898f read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (offset & 0x1000)
		return (get_open_bus() & ~0x40) | ((m_pad->read() & 0x01) << 6);

	return get_open_bus();
}

void nes_bmc_hp898f_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_hp898f write_m, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_k3036 write_h, offset: %04x, data: %02x\n", offset, data);
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
	LOG("bmc_k3046 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	data &= 0x1f;
	prg16_89ab(data);
	prg16_cdef(data | 0x07);
}

/*-------------------------------------------------

 BMC-K-3071

 Games: 110 in 1

 NES 2.0: mapper 438

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_k3071_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_k3071 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = BIT(offset, 1, 5);
	u8 mode = BIT(offset, 0);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8(BIT(data, 1, 5), CHRROM);
	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-S-009

 Games: 8 in 1

 NES 2.0: mapper 434

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_s009_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_s009 write_m, offset: %04x, data: %02x\n", offset, data);

	m_reg = (data & 0x07) << 3;
	prg16_89ab(m_reg);
	prg16_cdef(m_reg | 0x07);

	set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
}

void nes_bmc_s009_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_s009 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg16_89ab(m_reg | (data & 0x07));
}

/*-------------------------------------------------

 BMC-SA005-A

 Games: 16 in 1

 NES 2.0: mapper 338

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_sa005a_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_sa005a write_h, offset: %04x, data: %02x\n", offset, data);
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
	LOG("bmc_tf2740 write_m, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_tf2740 write_h, offset: %04x, data: %02x\n", offset, data);
	m_reg[0] = data;
	update_chr();
}

u8 nes_bmc_tf2740_device::read_m(offs_t offset)
{
	LOG("bmc_tf2740 read_m, offset: %04x\n", offset);
	return (get_open_bus() & ~0x03) | m_jumper;    // TODO: add jumper settings, m_jumper is 0 for now
}

/*-------------------------------------------------

 BMC-TH2348

 Games: Golden 8 in 1

 NES 2.0: mapper 437

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_th2348_device::write_l(offs_t offset, u8 data)
{
	LOG("bmc_th2348 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000)
	{
		m_latch = (offset & 0x07) << 3;
		prg16_89ab(m_latch);
		prg16_cdef(m_latch | 0x07);

		set_nt_mirroring(BIT(offset, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}
}

void nes_bmc_th2348_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_th2348 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg16_89ab(m_latch | (data & 0x07));
}

/*-------------------------------------------------

 BMC-TJ-03

 Games: 4 in 1

 NES 2.0: mapper 341

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_tj03_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_tj03 write_h, offset: %04x, data: %02x\n", offset, data);
	u8 bank = BIT(offset, 8, 2);
	prg32(bank);
	chr8(bank, CHRROM);
	set_nt_mirroring(BIT(offset, 9) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Board BMC-WS

 Games: Super 40-in-1

 In MAME: Partially Supported (some games, like Galaxian, have
 issues)

 -------------------------------------------------*/

void nes_bmc_ws_device::write_m(offs_t offset, uint8_t data)
{
	LOG("bmc_ws write_m, offset: %04x, data: %02x\n", offset, data);

	if (offset < 0x1000)
	{
		switch (offset & 0x01)
		{
			case 0:
				if (!m_latch)
				{
					m_latch = data & 0x20;
					set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
					u8 mode = !BIT(data, 3);
					prg16_89ab(data & ~mode);
					prg16_cdef(data | mode);
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
	LOG("bmc_11160 write_h, offset: %04x, data: %02x\n", offset, data);

	prg32(BIT(data, 4, 2));
	chr8(bitswap<4>(data, 5, 4, 1, 0), CHRROM);
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
	LOG("bmc_g146 write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_2751 write_h, offset: %04x, data: %02x\n", offset, data);
	u8 bank = BIT(offset, 4, 3);
	u8 mode = BIT(offset, 7);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	chr8(BIT(offset, 1, 3), CHRROM);
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
	LOG("bmc_8157 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = BIT(offset, 2, 5);
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
	LOG("bmc_8157 read_h, offset: %04x\n", offset);
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
	LOG("bmc_hik300 read_m, offset: %04x, data: %02x\n", offset);
	return get_open_bus() | (!BIT(offset, 4) << 7);    // some games have a protection MSB
}

void nes_bmc_hik300_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_hik300 write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_s700 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = (offset & 0x40) | BIT(offset, 8, 6);
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
	LOG("bmc_ball11 read_m, offset: %04x, data: %02x\n", offset);

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
	LOG("bmc_ball11 write_m, offset: %04x, data: %02x\n", offset, data);
	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_reg[0] = data & 0x02;
	update_prg();
}

void nes_bmc_ball11_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_ball11 write_h, offset: %04x, data: %02x\n", offset, data);
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
	LOG("bmc_22games write_h, offset: %04x, data: %02x\n", offset, data);

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

 NES 2.0: mapper 314

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_64y2k_device::update_banks()
{
	if (BIT(m_reg[0] & m_reg[1], 7))
		prg32(m_reg[1]);
	else
	{
		u8 bank = m_reg[1] << 1 | BIT(m_reg[1], 6);
		prg16_89ab(bank);
		prg16_cdef(bank | (BIT(m_reg[0], 7) ? 0 : 7));
	}

	chr8(BIT(m_reg[0], 1, 2) | (m_reg[2] << 2), m_chr_source);
	set_nt_mirroring(BIT(m_reg[0], 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_bmc_64y2k_device::write_l(offs_t offset, u8 data)
{
	LOG("bmc_64y2k write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000)
	{
		m_reg[offset & m_reg_mask] = data;
		update_banks();
	}
}

void nes_bmc_64y2k_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_64y2k write_h, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_reg[0], 7))
		prg16_89ab(((m_reg[1] << 1) & ~0x07) | (data & 0x07));
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
	LOG("bmc_420y2k write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_420y2k read_h, offset: %04x\n", offset);
	// latch bit 0 is only used to determine the menu, and the behavior of
	// this cart seems hardwired to OR $02 (ORing $00-$03 allows four menus)
	return hi_access_rom(offset | (m_latch & 1) << 1);
}

/*-------------------------------------------------

 BMC-12IN1

 Unknown Bootleg Multigame Board
 Games: 7 in 1

 NES 2.0: mapper 331

 In MAME: Supported.

 TODO: Supposedly PRG banks change depending on what
 CHR page the PPU is rendering from. This is not
 emulated but the cart still works since both inner
 PRG registers are always set to the same value.

 -------------------------------------------------*/

void nes_bmc_12in1_device::update_banks()
{
	u8 outer = (m_reg[3] & 0x03) << 3;
	u8 mode = BIT(m_reg[3], 3) ? 0x01 : 0x07; // NROM256 or UNROM-like mode
	prg16_89ab(outer | (m_reg[1] & 0x07));
	prg16_cdef(outer | (m_reg[2] & 0x07) | mode);

	chr4_0(outer | (m_reg[1] >> 3), m_chr_source);
	chr4_4(outer | (m_reg[2] >> 3), m_chr_source);

	set_nt_mirroring(BIT(m_reg[3], 2) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_bmc_12in1_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_12in1 write_h, offset: %04x, data: %02x\n", offset, data);

	m_reg[BIT(offset, 13, 2)] = data;
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
	LOG("bmc_20in1 write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_21in1 write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_31in1 write_h, offset: %04x, data: %02x\n", offset, data);
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

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_35in1_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_35in1 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = BIT(data, 2, 2);
	prg16_89ab(bank);
	prg16_cdef(bank);
	chr8(data & 0x03, CHRROM);
}

/*-------------------------------------------------

 BMC-36IN1

 Unknown Bootleg Multigame Board
 Games: 36 in 1, 1200 in 1

 iNES: mapper 200

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_36in1_device::write_h(offs_t offset, uint8_t data)
{
	LOG("bmc_36in1 write_h, offset: %04x, data: %02x\n", offset, data);

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

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_64in1_device::write_h(offs_t offset, uint8_t data)
{
	int bank = (offset >> 1) & (offset >> 2) & 0x01;

	LOG("bmc_64in1 write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc70in1 write_h, offset: %04x, data: %02x\n", offset, data);
	m_latch[BIT(offset, 14)] = offset;
	update_banks();
	set_nt_mirroring(BIT(m_latch[0], 5) ? PPU_MIRROR_HORZ: PPU_MIRROR_VERT);
}

uint8_t nes_bmc_70in1_device::read_h(offs_t offset)
{
	LOG("bmc70in1 read_h, offset: %04x\n", offset);

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
	LOG("bmc_72in1 read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (offset >= 0x1800)    // 4 nibbles of RAM mirrored above 0x5800
		return m_extra_ram[offset & 0x03];

	return get_open_bus();
}

void nes_bmc_72in1_device::write_l(offs_t offset, u8 data)
{
	LOG("bmc_72in1 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1800)    // 4 nibbles of RAM mirrored above 0x5800
		m_extra_ram[offset & 0x03] = data & 0x0f;
}

void nes_bmc_72in1_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_72in1 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 high = (offset >> 8) & 0x40;
	u8 bank = high | BIT(offset, 6, 6);
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
	LOG("bmc_76in1 write_h, offset: %04x, data: %02x\n", offset, data);
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
	LOG("bmc_150in1 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = BIT(offset, 1, 3);
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
	LOG("bmc500in1 write_h, offset: %04x, data: %02x\n", offset, data);
	prg32(BIT(offset, 2, 3));
	chr8(offset & 0x07, CHRROM);
}

/*-------------------------------------------------

 BMC-1200IN1

 Unknown Bootleg Multigame Board
 Games: 1200 in 1, 295 in 1, 76 in 1

 NESdev INES Mapper 227, an address-latch multicart with 8 KiB of
 unbanked CHR-RAM; a write to $8000-$FFFF sets, from its address
 (A15 1... .mLQ OQQP PpMS): S: PRG A14 is p (0) or CPU A14 (1); M:
 mirroring, 0 vertical, 1 horizontal; PPp: inner PRG bank (A16-A14);
 QQQ: outer bank (A19-A17); O: 1 is NROM (NROM-128 with S=0, NROM-256
 with S=1, whose inner bank is PP), 0 is UNROM-like, with CPU A14=1
 giving inner bank 7 if L is set and 0 if not; m: PRG A3-A0 are the
 solder pads (submapper 1 only). On a board without battery-backed RAM
 CHR-RAM is write-protected while O is set; the Chinese RPGs have 8 KiB
 of it and no UNROM-like modes. Everything powers up as 0. The pads
 are a machine configuration setting.

 Checked: a synthetic image with numbered PRG banks gives the page's
 bank for each mode (NROM-256 with p set was wrong), the pad replacing
 A3-A0 and the CHR-RAM protection; the multicart plays a game with the
 protection on, and three battery-backed RPG dumps draw as before.

 TODO: submapper 2 (the outer bank A18-A17 is 0 when the inner bank is
 0) is not emulated.

 iNES: mapper 227

 In MAME: Supported.

 -------------------------------------------------*/

static INPUT_PORTS_START( bmc_1200in1 )
	PORT_START("PAD")
	PORT_CONFNAME( 0x0f, 0x00, "Solder pads (submapper 1 menu)" )
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "3" )
	PORT_CONFSETTING(    0x04, "4" )
	PORT_CONFSETTING(    0x05, "5" )
	PORT_CONFSETTING(    0x06, "6" )
	PORT_CONFSETTING(    0x07, "7" )
	PORT_CONFSETTING(    0x08, "8" )
	PORT_CONFSETTING(    0x09, "9" )
	PORT_CONFSETTING(    0x0a, "10" )
	PORT_CONFSETTING(    0x0b, "11" )
	PORT_CONFSETTING(    0x0c, "12" )
	PORT_CONFSETTING(    0x0d, "13" )
	PORT_CONFSETTING(    0x0e, "14" )
	PORT_CONFSETTING(    0x0f, "15" )
INPUT_PORTS_END

ioport_constructor nes_bmc_1200in1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_1200in1 );
}

void nes_bmc_1200in1_device::chr_w(offs_t offset, uint8_t data)
{
	int bank = offset >> 10;

	if (!m_vram_protect)
		m_chr_access[bank][offset & 0x3ff] = data;
}

uint8_t nes_bmc_1200in1_device::read_h(offs_t offset)
{
	// submapper 1, m bit set: PRG A3-A0 come from the solder pads instead of the CPU
	if (m_submapper == 1 && m_pad_mode)
		return hi_access_rom((offset & ~0x0f) | (m_pad->read() & 0x0f));
	return hi_access_rom(offset);
}

void nes_bmc_1200in1_device::write_h(offs_t offset, uint8_t data)
{
	int bank = ((offset >> 2) & 0x1f) |  ((offset & 0x0100) >> 3);

	LOG("bmc_1200in1 write_h, offset: %04x, data: %02x\n", offset, data);

	m_pad_mode = BIT(offset, 10);

	if (offset & 0x80)
	{
		// NROM: S=1 takes PRG A14 from the CPU, so the inner bank is PP and p is not used
		prg16_89ab(bank & ~(offset & 1));
		prg16_cdef(bank | (offset & 1));
	}
	else
	{
		int low_mask = (offset & 1) ? 0x3e : 0xff;

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

	// CHR-RAM is write-protected in the NROM modes, but not on the boards with battery-backed RAM
	m_vram_protect = BIT(offset, 7) && m_battery.empty();

	set_nt_mirroring(BIT(offset, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-GOLDEN260IN1

 Unknown Bootleg Multigame Board
 Games: Golden Game 150 in 1, Golden Game 210 in 1,
 Golden Game 260 in 1, 1500 in 1

 iNES: mapper 235

 A9-A8 pick one of four 1 MiB ROM sockets and the menu checks
 each socket's checksum, so an image has to keep the empty
 sockets in place. The software list does that with $FF
 fill; for iNES files, which leave the empty sockets out, the
 loader puts a 1 MiB image in socket 0, a 2 MiB one in sockets
 0 and 2 and a 3 MiB one in sockets 0, 2 and 3 (NESdev: only
 sockets 0 and 2 are filled on the 2 MiB cart; the software
 list has the 260 in 1 in sockets 0, 2 and 3).

 In MAME: Supported.

 TODO: Return open bus when ROM sockets are empty? ($FF now)

 -------------------------------------------------*/

void nes_bmc_gold260_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_gold260 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = bitswap<8>(offset, 9, 8, 4, 3, 2, 1, 0, 12);
	u8 mode = !BIT(offset, 11);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	set_nt_mirroring(BIT(offset, 10) ? PPU_MIRROR_LOW : BIT(offset, 13) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-ET113

 Games: 1992 Gaishi 190-in-1 Hua Mian Xuan Guan, 211-in-1 - 1992 Ultra
 Power Card, 1996 Aoyunhui Jingji Ban 24-in-1

 NESdev INES Mapper 242, an address-latch multicart with 8 KiB of
 unbanked CHR-RAM; a write to $8000-$FFFF sets, from its address
 (A15 1... .CLm OQQP PpMS): S: PRG A14 is p (0) or CPU A14 (1); M:
 mirroring, 0 vertical, 1 horizontal; PPp: inner PRG bank (A16-A14);
 QQ: outer bank (A18-A17); O: 1 is NROM (NROM-128 with S=0, NROM-256
 with S=1, whose inner bank is PP), 0 is UNROM-like, with CPU A14=1
 giving inner bank 7 if L is set and 0 if not; m: PRG A4-A0 are the
 solder pads; C: on the ET-113 PCB, which adds a 128 KiB PRG-ROM chip
 (the last 128 KiB of a 640 KiB image) with a replacement menu, 0
 selects that chip and 1 the 512 KiB one. Everything powers up as 0,
 so the 128 KiB chip is the one that boots. CHR-RAM is write-protected
 while O is set. The pads are a machine configuration setting. The
 Chinese RPGs from Waixing, which have 8 KiB of battery-backed RAM and
 only the 32 KiB latch, are nes_waixing_wxzs_device.

 iNES: mapper 242

 In MAME: Supported.

 -------------------------------------------------*/

static INPUT_PORTS_START( bmc_et113 )
	PORT_START("PAD")
	PORT_CONFNAME( 0x1f, 0x00, "Solder pads (menu select)" )
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "3" )
	PORT_CONFSETTING(    0x04, "4" )
	PORT_CONFSETTING(    0x05, "5" )
	PORT_CONFSETTING(    0x06, "6" )
	PORT_CONFSETTING(    0x07, "7" )
	PORT_CONFSETTING(    0x08, "8" )
	PORT_CONFSETTING(    0x09, "9" )
	PORT_CONFSETTING(    0x0a, "10" )
	PORT_CONFSETTING(    0x0b, "11" )
	PORT_CONFSETTING(    0x0c, "12" )
	PORT_CONFSETTING(    0x0d, "13" )
	PORT_CONFSETTING(    0x0e, "14" )
	PORT_CONFSETTING(    0x0f, "15" )
	PORT_CONFSETTING(    0x10, "16" )
	PORT_CONFSETTING(    0x11, "17" )
	PORT_CONFSETTING(    0x12, "18" )
	PORT_CONFSETTING(    0x13, "19" )
	PORT_CONFSETTING(    0x14, "20" )
	PORT_CONFSETTING(    0x15, "21" )
	PORT_CONFSETTING(    0x16, "22" )
	PORT_CONFSETTING(    0x17, "23" )
	PORT_CONFSETTING(    0x18, "24" )
	PORT_CONFSETTING(    0x19, "25" )
	PORT_CONFSETTING(    0x1a, "26" )
	PORT_CONFSETTING(    0x1b, "27" )
	PORT_CONFSETTING(    0x1c, "28" )
	PORT_CONFSETTING(    0x1d, "29" )
	PORT_CONFSETTING(    0x1e, "30" )
	PORT_CONFSETTING(    0x1f, "31" )
INPUT_PORTS_END

ioport_constructor nes_bmc_et113_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_et113 );
}

void nes_bmc_et113_device::device_start()
{
	common_start();
	save_item(NAME(m_pad_mode));
	save_item(NAME(m_vram_protect));
}

void nes_bmc_et113_device::update_banks(offs_t latch)
{
	const int bank = (latch >> 2) & 0x1f;                 // PRG A18-A14
	const int s = BIT(latch, 0);
	int low, high;

	if (BIT(latch, 7))
	{
		// NROM: S=1 takes PRG A14 from the CPU, so the inner bank is PP and p is not used
		low = bank & ~s;
		high = bank | s;
	}
	else
	{
		low = s ? (bank & 0x1e) : bank;
		high = BIT(latch, 9) ? (bank | 0x07) : (bank & 0x18);
	}

	// C=0 selects the 128 KiB chip behind the 512 KiB one, if the image has it: its inner bank is A16-A14 only
	if (m_prg_chunks == 40 && !BIT(latch, 10))
	{
		low = 32 + (low & 7);
		high = 32 + (high & 7);
	}

	prg16_89ab(low);
	prg16_cdef(high);
}

void nes_bmc_et113_device::pcb_reset()
{
	update_banks(0);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);   // the address latch powers up as 0
	m_pad_mode = 0;
	m_vram_protect = 0;
}

void nes_bmc_et113_device::chr_w(offs_t offset, uint8_t data)
{
	int bank = offset >> 10;

	if (!m_vram_protect)
		m_chr_access[bank][offset & 0x3ff] = data;
}

uint8_t nes_bmc_et113_device::read_h(offs_t offset)
{
	// m set: PRG A4-A0 come from the solder pads instead of the CPU
	if (m_pad_mode)
		return hi_access_rom((offset & ~0x1f) | (m_pad->read() & 0x1f));
	return hi_access_rom(offset);
}

void nes_bmc_et113_device::write_h(offs_t offset, uint8_t data)
{
	LOG("bmc_et113 write_h, offset: %04x, data: %02x\n", offset, data);

	m_pad_mode = BIT(offset, 8);
	update_banks(offset);

	// CHR-RAM is write-protected in the NROM modes
	m_vram_protect = BIT(offset, 7);

	set_nt_mirroring(BIT(offset, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
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
	LOG("bmc_42in1reset write_h, offset: %04x, data: %02x\n", offset, data);

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
	LOG("bmc_lc160 write_h, offset: %04x, data: %02x\n", offset, data);
	if (offset >= 0x4000)
	{
		u8 bank = BIT(offset, 2, 6);
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
	LOG("bmc_60311c write_m, offset: %04x, data: %02x\n", offset, data);
	m_reg[offset & 1] = data & 0x7f;
	update_banks();
}

void nes_bmc_60311c_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_60311c write_h, offset: %04x, data: %02x\n", offset, data);
	m_reg[2] = data & 0x07;
	update_banks();
}

/*-------------------------------------------------

 BMC-CTC-12IN1

 Games: 12 in 1 Game Card

 The register at 0x8000-0xbfff is PPMB B... (banking mode, mirroring and the 128K
 outer bank) and the one at 0xc000-0xffff holds the 16K inner bank in its low three
 bits. Address bit 13 of the second write is the CHR-RAM write-protect bit, which is
 latched with it. CHR-RAM is always write-protected in the NROM modes.

 NES 2.0: mapper 337

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_ctc12in1_device::read_m(offs_t offset)
{
	LOG("bmc_ctc12in1 read_m, offset: %04x, data: %02x\n", offset);
	return m_prg[0x01 * 0x2000 + offset];    // fixed to bank 1
}

void nes_bmc_ctc12in1_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_ctc12in1 write_h, offset: %04x, data: %02x\n", offset, data);
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

	if (BIT(offset, 14))
		m_vram_wp = BIT(offset, 13);
	m_vram_protect = !BIT(m_reg[0], 7) || m_vram_wp;
	set_nt_mirroring(BIT(m_reg[0], 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC-891227

 Games: Super 15 in 1 Game Card

 NES 2.0: mapper 350

 This board is similar to CTC-12IN1 above but with
 the mirroring and banking mode bits moved and with
 support for Contra on a separate 128K chip. Unlike CTC-12IN1 it has no
 CHR-RAM write-protect bit: CHR-RAM is write-protected in the NROM modes only.

 In MAME: Supported.

 TODO: Contra bugs: Without input character sprite
 is always in the UP position. Cannot pickup weapons.

 -------------------------------------------------*/

void nes_bmc_891227_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_891227 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset < 0x4000)
		 data = bitswap<8>(data, 6, 5, 7, 4, 3, 2, 1, 0);

	nes_bmc_ctc12in1_device::write_h(offset, data);
	m_vram_protect = !BIT(m_reg[0], 7);

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

 NESdev NES 2.0 Mapper 354: the register is latched from the address and data
 bits (mode in A2-A0, CHR-RAM write-protect in A3, PRG A20 in A4, PRG A21 in
 A12) and is all clear at power-on and on reset. Submapper 0 (FAM250, 810139C)
 answers only at $F000-$FFFF, where A12 is always 1; submapper 1 (810331C/
 SCHI-24, 1992 1050-in-1) answers at $E000-$FFFF and A12 is PRG A21.

 NES 2.0: mapper 354

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_fam250_device::read_m(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "fam250 read_m, offset: %04x\n", offset);

	if (m_latch == 5)    // only the Bubble Bobble FDS bootleg uses this
		return m_prg[((((m_reg & 0x3f) | m_prg_a20 << 6) << 1 | BIT(m_reg, 7)) * 0x2000 + offset) & (m_prg_size - 1)];
	else
		return get_open_bus();
}

void nes_bmc_fam250_device::write_h(offs_t offset, u8 data)
{
	LOG("fam250 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset >= ((m_submapper == 1) ? 0x6000 : 0x7000))
	{
		// PRG A20 is address bit 4 (NESdev: A~111B .... ...A CSSS); on submapper 1, A12 is PRG A21 (on submapper 0 it
		// is always 1 and the chip's A21 output is not connected)
		m_prg_a20 = BIT(offset, 4) | ((m_submapper == 1) ? (BIT(offset, 12) << 1) : 0);
		nes_bmc_k1029_device::write_h(offset, data);

		m_latch = offset & 0x07;
		m_reg = data;
		if (m_latch == 5)
			prg32((m_reg & 0x38) >> 1 | 0x03 | m_prg_a20 << 5);

		m_vram_protect = BIT(offset, 3);
	}
}

/*-------------------------------------------------

 BMC-K-1029

 Games: 100 in 1 Contra Function 16, 168 in 1

 The latch holds the address bits A1-A0 (which select the banking mode) along
 with the data bits. Its power-on and reset state is all bits clear, which is
 NROM-256 mode with vertical mirroring and write-protected CHR-RAM. CHR-RAM is
 writable only in the UNROM and NROM-64 modes.

 iNES: mapper 15

 In MAME: Supported.

 TODO: Contra games in 100-in-1 endlessly loop the
 second level played regardless of starting level.
 BTANB? Bad dump? FCEUX exhibits the same behavior.

 -------------------------------------------------*/

void nes_bmc_k1029_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_k1029 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = (data & 0x3f) | m_prg_a20 << 6;
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
	LOG("n625092 write_h, offset: %04x, data: %02x\n", offset, data);

	m_latch[BIT(offset, 14)] = offset;

	u8 bank = bitswap<4>(m_latch[0], 9, 7, 6, 5) << 3 | (m_latch[1] & 0x07);
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

 Submapper 1 (82AB) has one PRG bank bit less than submapper 0, with the CHR-RAM
 write-protect bit moved into its place.

 iNES: mapper 63

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_th22913_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_th22913 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = (offset >> 2) & ((1 << (m_vram_prot_bit - 2)) - 1);
	u8 mode = BIT(offset, 1);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	set_nt_mirroring(BIT(offset, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_vram_protect = BIT(offset, m_vram_prot_bit);
}


/*-------------------------------------------------

 Bit Corp 31-in-1 (cartridge ID 3150, PCB P3150)

 NESdev NES 2.0 Mapper 360: there is no menu and no register;
 five DIP switches on the cartridge select the outer 16 KiB
 PRG-ROM bank, mapped at both $8000 and $C000. Only when all five
 are off does the bank become 32 KiB (Super Mario Bros.). The
 fifth switch (bank bit 4) also selects the mirroring: off
 vertical, on horizontal.

 The 8 KiB CHR-ROM bank is the same switch value, as in FCEUmm's
 360.c (NESdev does not say). The switches are a machine
 configuration setting read at reset (default 0, Super Mario
 Bros.). FCEUmm treats setting 1 as Super Mario Bros. too, and
 steps the switches on every reset as a convenience, and has a
 "submapper 1" with a register at $4100; NESdev describes none of
 these, so they are not emulated.

 NES 2.0: mapper 360

 In MAME: Supported.

 -------------------------------------------------*/

static INPUT_PORTS_START( bitcorp3150 )
	PORT_START("DSW")
	PORT_CONFNAME( 0x1f, 0x00, "DIP Switches (game)" )
	PORT_CONFSETTING(    0x00, "0 (Super Mario Bros.)" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "3" )
	PORT_CONFSETTING(    0x04, "4" )
	PORT_CONFSETTING(    0x05, "5" )
	PORT_CONFSETTING(    0x06, "6" )
	PORT_CONFSETTING(    0x07, "7" )
	PORT_CONFSETTING(    0x08, "8" )
	PORT_CONFSETTING(    0x09, "9" )
	PORT_CONFSETTING(    0x0a, "10" )
	PORT_CONFSETTING(    0x0b, "11" )
	PORT_CONFSETTING(    0x0c, "12" )
	PORT_CONFSETTING(    0x0d, "13" )
	PORT_CONFSETTING(    0x0e, "14" )
	PORT_CONFSETTING(    0x0f, "15" )
	PORT_CONFSETTING(    0x10, "16" )
	PORT_CONFSETTING(    0x11, "17" )
	PORT_CONFSETTING(    0x12, "18" )
	PORT_CONFSETTING(    0x13, "19" )
	PORT_CONFSETTING(    0x14, "20" )
	PORT_CONFSETTING(    0x15, "21" )
	PORT_CONFSETTING(    0x16, "22" )
	PORT_CONFSETTING(    0x17, "23" )
	PORT_CONFSETTING(    0x18, "24" )
	PORT_CONFSETTING(    0x19, "25" )
	PORT_CONFSETTING(    0x1a, "26" )
	PORT_CONFSETTING(    0x1b, "27" )
	PORT_CONFSETTING(    0x1c, "28" )
	PORT_CONFSETTING(    0x1d, "29" )
	PORT_CONFSETTING(    0x1e, "30" )
	PORT_CONFSETTING(    0x1f, "31" )
INPUT_PORTS_END

ioport_constructor nes_bitcorp3150_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bitcorp3150 );
}

nes_bitcorp3150_device::nes_bitcorp3150_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BITCORP3150, tag, owner, clock), m_dsw(*this, "DSW")
{
}

void nes_bitcorp3150_device::pcb_reset()
{
	const u8 dip = m_dsw->read() & 0x1f;
	if (!dip)
		prg32(0);
	else
	{
		prg16_89ab(dip);
		prg16_cdef(dip);
	}
	chr8(dip, CHRROM);
	set_nt_mirroring(BIT(dip, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 BMC 100-in-1 (NES 2.0 mapper 576)

 Games: 100-in-1 (Asia) (En) (Pirate)

 NESdev has no page for this mapper; it is taken from FCEUmm's
 576.c. Every write to $8000-$FFFF latches the address:
 - A8 set: one 16 KiB PRG bank (A4 and up) at $8000 and $C000;
   clear: one 32 KiB PRG bank (A5 and up)
 - one 8 KiB CHR bank from the low address bits
 - A9: mirroring, 1 horizontal, 0 vertical
 A console reset clears the latch (FCEUmm); power-on value 0.

 NES 2.0: mapper 576

 In MAME: Supported.

 -------------------------------------------------*/

nes_bmc_576_device::nes_bmc_576_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_576, tag, owner, clock), m_latch(0)
{
}

void nes_bmc_576_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_576_device::pcb_reset()
{
	m_latch = 0;
	update_banks();
}

void nes_bmc_576_device::update_banks()
{
	if (BIT(m_latch, 8))
	{
		prg16_89ab(m_latch >> 4);
		prg16_cdef(m_latch >> 4);
	}
	else
		prg32(m_latch >> 5);
	chr8(m_latch, CHRROM);
	set_nt_mirroring(BIT(m_latch, 9) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_bmc_576_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc 576 write_h, offset: %04x, data: %02x\n", offset, data);

	m_latch = offset & 0x3ff;
	update_banks();
}

/*-------------------------------------------------

 NTDEC 22-in-1 (NES 2.0 mapper 564)

 Games: NTDEC 22-in-1 (Asia) (Pirate)

 NESdev has no page for this mapper; it is taken from FCEUmm's
 564.c. Every write to $8000-$FFFF latches the data:
 - the latch selects the 32 KiB PRG bank; CHR is one 8 KiB bank
   (CHR-RAM on the known cart)
 - bit 5 clear: bit 4 selects horizontal (1) or vertical (0)
   mirroring; bit 5 set: one-screen, page from bit 4
 - once bit 5 is set the latch is locked to the game's mode:
   later writes keep bits 5 and 3, and also bit 2 while bit 3
   is clear (ANROM-like), and take the other bits from the data
   (the game's own banks and one-screen page)
 A console reset clears the latch (FCEUmm); power-on value 0.

 NES 2.0: mapper 564

 In MAME: Supported.

 -------------------------------------------------*/

nes_bmc_564_device::nes_bmc_564_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_564, tag, owner, clock), m_latch(0)
{
}

void nes_bmc_564_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_564_device::pcb_reset()
{
	m_latch = 0;
	update_banks();
}

void nes_bmc_564_device::update_banks()
{
	prg32(m_latch);
	chr8(0, m_chr_source);
	if (BIT(m_latch, 5))
		set_nt_mirroring(BIT(m_latch, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	else
		set_nt_mirroring(BIT(m_latch, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_bmc_564_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc 564 write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(m_latch, 5))
	{
		const u8 keep = BIT(m_latch, 3) ? 0x28 : 0x2c;   // AOROM-like : ANROM-like
		data = (m_latch & keep) | (data & ~keep);
	}
	m_latch = data;
	update_banks();
}

/*-------------------------------------------------

 Super Micro Machines 1999 (NES 2.0 mapper 582)

 Games: Super Micro Machines 1999 (Asia) (En) (Pirate)

 NESdev has no page for this mapper; it is taken from FCEUmm's
 582.c. Four registers selected by A14-A13 ($8000, $A000, $C000,
 $E000):
 - $8000-$BFFF: 16 KiB PRG bank (reg 1 << 3) | (reg 0 & 7)
 - $C000-$FFFF: 16 KiB PRG bank (reg 3 << 3) | (reg 2 & 7)
 CHR is one 8 KiB bank (CHR-RAM on the known cart); mirroring is
 fixed by the header. Power-on and reset set all four registers
 to $FF (FCEUmm).

 NES 2.0: mapper 582

 In MAME: Supported.

 -------------------------------------------------*/

nes_bmc_582_device::nes_bmc_582_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_582, tag, owner, clock), m_reg{ 0xff, 0xff, 0xff, 0xff }
{
}

void nes_bmc_582_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bmc_582_device::pcb_reset()
{
	m_reg[0] = m_reg[1] = m_reg[2] = m_reg[3] = 0xff;
	update_prg();
	chr8(0, m_chr_source);
}

void nes_bmc_582_device::update_prg()
{
	prg16_89ab((m_reg[1] << 3) | (m_reg[0] & 0x07));
	prg16_cdef((m_reg[3] << 3) | (m_reg[2] & 0x07));
}

void nes_bmc_582_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc 582 write_h, offset: %04x, data: %02x\n", offset, data);

	m_reg[BIT(offset, 13, 2)] = data;
	update_prg();
}

/*-------------------------------------------------

 Super 72-in-1 (NES 2.0 mapper 585)

 Games: Super 72-in-1 (Asia) (En) (Pirate)

 NESdev has no page for this mapper; it is taken from FCEUmm's
 585.c. Every write to $8000-$FFFF latches the address:
 - A0 set: one 32 KiB PRG bank (A2 and up); clear: one 16 KiB
   bank (A1 and up) at $8000 and $C000
 - one 8 KiB CHR bank (A1 and up)
 - A4: mirroring, 1 vertical, 0 horizontal
 While (latch & pad & $60) is not 0, reads of $8000-$FFFF are
 open bus; the solder pad (bits 6-5) is a configuration setting
 here (FCEUmm steps it by $20 on every reset). A console reset
 clears the latch (FCEUmm); power-on value 0.

 NES 2.0: mapper 585

 In MAME: Supported.

 -------------------------------------------------*/

static INPUT_PORTS_START( bmc_585 )
	PORT_START("PAD")
	PORT_CONFNAME( 0x60, 0x00, "Solder Pad" )
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x20, "1" )
	PORT_CONFSETTING(    0x40, "2" )
	PORT_CONFSETTING(    0x60, "3" )
INPUT_PORTS_END

ioport_constructor nes_bmc_585_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_585 );
}

nes_bmc_585_device::nes_bmc_585_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BMC_585, tag, owner, clock), m_pad_port(*this, "PAD"), m_latch(0)
{
}

void nes_bmc_585_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_bmc_585_device::pcb_reset()
{
	m_latch = 0;
	update_banks();
}

void nes_bmc_585_device::update_banks()
{
	if (BIT(m_latch, 0))
		prg32(m_latch >> 2);
	else
	{
		prg16_89ab(m_latch >> 1);
		prg16_cdef(m_latch >> 1);
	}
	chr8(m_latch >> 1, CHRROM);
	set_nt_mirroring(BIT(m_latch, 4) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
}

u8 nes_bmc_585_device::read_h(offs_t offset)
{
	if (m_latch & m_pad_port->read() & 0x60)
		return get_open_bus();
	return hi_access_rom(offset);
}

void nes_bmc_585_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc 585 write_h, offset: %04x, data: %02x\n", offset, data);

	m_latch = offset & 0x7f;
	update_banks();
}
