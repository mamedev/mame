// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for MMC-3 clone PCBs


 Here we emulate several pirate PCBs based on MMC-3 boards

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc3_clones.h"
#include "cpu/m6502/m6502.h"

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

DEFINE_DEVICE_TYPE(NES_NITRA,         nes_nitra_device,         "nes_nitra",         "NES Cart Nitra PCB")
DEFINE_DEVICE_TYPE(NES_BMW8544,       nes_bmw8544_device,       "nes_bmw8544",       "NES Cart BMW8544 PCB")
DEFINE_DEVICE_TYPE(NES_FS6,           nes_fs6_device,           "nes_fs6",           "NES Cart Fight Street VI PCB")
DEFINE_DEVICE_TYPE(NES_SBROS11,       nes_sbros11_device,       "nes_smb11",         "NES Cart SMB 11 PCB")
DEFINE_DEVICE_TYPE(NES_MALISB,        nes_malisb_device,        "nes_malisb",        "NES Cart Mali Splash Bomb PCB")
DEFINE_DEVICE_TYPE(NES_FAMILY4646,    nes_family4646_device,    "nes_family4646",    "NES Cart BMC-FAMILY4646 PCB")
DEFINE_DEVICE_TYPE(NES_PIKAY2K,       nes_pikay2k_device,       "nes_pikay2k",       "NES Cart PIKACHU Y2K PCB")
DEFINE_DEVICE_TYPE(NES_8237,          nes_8237_device,          "nes_8237",          "NES Cart UNL-8237 PCB")
DEFINE_DEVICE_TYPE(NES_8237A,         nes_8237a_device,         "nes_8237a",         "NES Cart UNL-8237A PCB")
DEFINE_DEVICE_TYPE(NES_158B,          nes_158b_device,          "nes_158b",          "NES Cart UNL-158B PCB")
DEFINE_DEVICE_TYPE(NES_SG_LIONK,      nes_sglionk_device,       "nes_sglionk",       "NES Cart SuperGame Lion King PCB")
DEFINE_DEVICE_TYPE(NES_SG_BOOG,       nes_sgboog_device,        "nes_sgbooger",      "NES Cart SuperGame BoogerMan PCB")
DEFINE_DEVICE_TYPE(NES_KASING,        nes_kasing_device,        "nes_kasing",        "NES Cart Kasing PCB")
DEFINE_DEVICE_TYPE(NES_KAY,           nes_kay_device,           "nes_kay",           "NES Cart KAY PCB")
DEFINE_DEVICE_TYPE(NES_H2288,         nes_h2288_device,         "nes_h2288",         "NES Cart H-2288 PCB")
DEFINE_DEVICE_TYPE(NES_6035052,       nes_6035052_device,       "nes_6035052",       "NES Cart UNL-603-5052 PCB")
DEFINE_DEVICE_TYPE(NES_TXC_TW,        nes_txc_tw_device,        "nes_txc_tw",        "NES Cart TXC Thunder Warrior PCB")
DEFINE_DEVICE_TYPE(NES_KOF97,         nes_kof97_device,         "nes_kof97",         "NES Cart KoF 97 PCB")
DEFINE_DEVICE_TYPE(NES_KOF96,         nes_kof96_device,         "nes_kof96",         "NES Cart KoF 96 PCB")
DEFINE_DEVICE_TYPE(NES_SF3,           nes_sf3_device,           "nes_sf3",           "NES Cart Super Fighter III PCB")
DEFINE_DEVICE_TYPE(NES_COCOMA,        nes_cocoma_device,        "nes_cocoma",        "NES Cart Cocoma Core Pro PCB")
DEFINE_DEVICE_TYPE(NES_GOUDER,        nes_gouder_device,        "nes_gouder",        "NES Cart Gouder PCB")
DEFINE_DEVICE_TYPE(NES_SA9602B,       nes_sa9602b_device,       "nes_sa9602b",       "NES Cart SA-9602B PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_SHERO,  nes_sachen_shero_device,  "nes_shero",         "NES Cart Street Hero PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_ZGDH,   nes_sachen_zgdh_device,   "nes_zgdh",          "NES Cart Zhongguo Daheng PCB")
DEFINE_DEVICE_TYPE(NES_A9746,         nes_a9746_device,         "nes_bmc_a9746",     "NES Cart A-9746 PCB")

DEFINE_DEVICE_TYPE(NES_A88S1,         nes_a88s1_device,         "nes_a88s1",         "NES Cart BMC A88S-1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_EL86XC,    nes_bmc_el86xc_device,    "nes_bmc_el86xc",    "NES Cart BMC EL860947C/EL861121C PCB")
DEFINE_DEVICE_TYPE(NES_FK23C,         nes_fk23c_device,         "nes_fk23c",         "NES Cart FK23C PCB")
DEFINE_DEVICE_TYPE(NES_FK23CA,        nes_fk23ca_device,        "nes_fk23ca",        "NES Cart FK23CA PCB")
DEFINE_DEVICE_TYPE(NES_NT639,         nes_nt639_device,         "nes_nt639",         "NES Cart NT-639 PCB")
DEFINE_DEVICE_TYPE(NES_RESETTXROM,    nes_resettxrom_device,    "nes_resettxrom",    "NES Cart BMC RESET-TXROM PCB")
DEFINE_DEVICE_TYPE(NES_S24IN1SC03,    nes_s24in1sc03_device,    "nes_s24in1c03",     "NES Cart Super 24 in 1 SC-03 PCB")
DEFINE_DEVICE_TYPE(NES_TECHLINE9IN1,  nes_tech9in1_device,      "nes_tech9in1",      "NES Cart Techline 9 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_5IN1,      nes_bmc_5in1_device,      "nes_bmc_5in1",      "NES Cart BMC 5 in 1 1993 Copyright PCB")
DEFINE_DEVICE_TYPE(NES_BMC_8IN1,      nes_bmc_8in1_device,      "nes_bmc_8in1",      "NES Cart BMC GRM070 8 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_15IN1,     nes_bmc_15in1_device,     "nes_bmc_15in1",     "NES Cart BMC 15 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_SBIG7,     nes_bmc_sbig7_device,     "nes_bmc_sbig7",     "NES Cart BMC Super BIG 7 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_HIK8,      nes_bmc_hik8_device,      "nes_bmc_hik8",      "NES Cart BMC HIK 8 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_JY208,     nes_bmc_jy208_device,     "nes_bmc_jy208",     "NES Cart BMC JY-208 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_JY302,     nes_bmc_jy302_device,     "nes_bmc_jy302",     "NES Cart BMC JY-302 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_KC885,     nes_bmc_kc885_device,     "nes_bmc_kc885",     "NES Cart BMC KC885 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_SFC12,     nes_bmc_sfc12_device,     "nes_bmc_sfc12",     "NES Cart BMC SFC-12 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_HIK4,      nes_bmc_hik4_device,      "nes_bmc_hik4",      "NES Cart BMC HIK 4 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_F15,       nes_bmc_f15_device,       "nes_bmc_f15",       "NES Cart BMC F-15 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_F600,      nes_bmc_f600_device,      "nes_bmc_f600",      "NES Cart BMC F600 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GN45,      nes_bmc_gn45_device,      "nes_bmc_gn45",      "NES Cart BMC GN-45 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GOLD7IN1,  nes_bmc_gold7in1_device,  "nes_bmc_gold7in1",  "NES Cart BMC Golden 7 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K3006,     nes_bmc_k3006_device,     "nes_bmc_k3006",     "NES Cart BMC K-3006 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K3033,     nes_bmc_k3033_device,     "nes_bmc_k3033",     "NES Cart BMC K-3033 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_L6IN1,     nes_bmc_l6in1_device,     "nes_bmc_l6in1",     "NES Cart BMC L6IN1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_00202650,  nes_bmc_00202650_device,  "nes_bmc_00202650",  "NES Cart BMC 00202650 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_411120C,   nes_bmc_411120c_device,   "nes_bmc_411120c",   "NES Cart BMC 411120C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_810305C,   nes_bmc_810305c_device,   "nes_bmc_810305c",   "NES Cart BMC 81-03-05-C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_820720C,   nes_bmc_820720c_device,   "nes_bmc_820720c",   "NES Cart BMC 820720C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_830118C,   nes_bmc_830118c_device,   "nes_bmc_830118c",   "NES Cart BMC 830118C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_830832C,   nes_bmc_830832c_device,   "nes_bmc_830832c",   "NES Cart BMC 830832C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_YY841101C, nes_bmc_yy841101c_device, "nes_bmc_yy841101c", "NES Cart BMC YY841101C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_YY841155C, nes_bmc_yy841155c_device, "nes_bmc_yy841155c", "NES Cart BMC YY841155C PCB")
DEFINE_DEVICE_TYPE(NES_PJOY84,        nes_pjoy84_device,        "nes_pjoy84",        "NES Cart Powerjoy 84 PCB")
DEFINE_DEVICE_TYPE(NES_SMD133,        nes_smd133_device,        "nes_smd133",        "NES Cart SMD133 PCB")


INPUT_PORTS_START( sachen_shero )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x80, 0x00, "Title Screen" )
	PORT_CONFSETTING(    0x00, "Street Heroes" )
	PORT_CONFSETTING(    0x80, u8"\u4f8d\u9b42 (Shìhún)" )    // 侍魂
INPUT_PORTS_END

INPUT_PORTS_START( bmc_5in1 )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x01, 0x01, "Menu Type" )
	PORT_CONFSETTING(    0x00, "20 in 1" )
	PORT_CONFSETTING(    0x01, "5 in 1" )
INPUT_PORTS_END

INPUT_PORTS_START( bmc_f600 )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x80, 0x80, "Menu Type" )
	PORT_CONFSETTING(    0x00, "Mario 67 in 1" )
	PORT_CONFSETTING(    0x80, "Mario Party II (6 in 1)" )
INPUT_PORTS_END

INPUT_PORTS_START( bmc_kc885 )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x07, 0x07, "Menu Type" )
	PORT_CONFSETTING(    0x00, "6000 in 1" )
	PORT_CONFSETTING(    0x01, "400 in 1" )
	PORT_CONFSETTING(    0x02, "800 in 1" )
	PORT_CONFSETTING(    0x03, "5000 in 1" )
	PORT_CONFSETTING(    0x04, "190 in 1" )
	PORT_CONFSETTING(    0x05, "1000 in 1" )
	PORT_CONFSETTING(    0x06, "2000 in 1" )
	PORT_CONFSETTING(    0x07, "19 in 1" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_sachen_shero_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sachen_shero );
}

ioport_constructor nes_bmc_5in1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_5in1 );
}

ioport_constructor nes_bmc_f600_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_f600 );
}

ioport_constructor nes_bmc_kc885_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_kc885 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

nes_nitra_device::nes_nitra_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_NITRA, tag, owner, clock)
{
}

nes_bmw8544_device::nes_bmw8544_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMW8544, tag, owner, clock), m_reg(0)
{
}

nes_fs6_device::nes_fs6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_FS6, tag, owner, clock)
{
}

nes_sbros11_device::nes_sbros11_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_SBROS11, tag, owner, clock)
{
}

nes_malisb_device::nes_malisb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_MALISB, tag, owner, clock)
{
}

nes_family4646_device::nes_family4646_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_FAMILY4646, tag, owner, clock)
{
}

nes_pikay2k_device::nes_pikay2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_PIKAY2K, tag, owner, clock)
{
}

nes_8237_device::nes_8237_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 board)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_board(board)
{
}

nes_8237_device::nes_8237_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_8237_device(mconfig, NES_8237, tag, owner, clock, 0)
{
}

nes_8237a_device::nes_8237a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_8237_device(mconfig, NES_8237A, tag, owner, clock, 1)
{
}

nes_158b_device::nes_158b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_8237_device(mconfig, NES_158B, tag, owner, clock, 0)
{
}

nes_kasing_device::nes_kasing_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_mmc3_mode(true)
{
}

nes_kasing_device::nes_kasing_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_kasing_device(mconfig, NES_KASING, tag, owner, clock)
{
}

nes_sglionk_device::nes_sglionk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 board)
	: nes_kasing_device(mconfig, type, tag, owner, clock), m_board(board)
{
}

nes_sglionk_device::nes_sglionk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sglionk_device(mconfig, NES_SG_LIONK, tag, owner, clock, 0)
{
}

nes_sgboog_device::nes_sgboog_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sglionk_device(mconfig, NES_SG_BOOG, tag, owner, clock, 1)
{
}

nes_kay_device::nes_kay_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_KAY, tag, owner, clock), m_low_reg(0)
{
}

nes_h2288_device::nes_h2288_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_H2288, tag, owner, clock), m_mmc3_mode(true)
{
}

nes_6035052_device::nes_6035052_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_6035052, tag, owner, clock), m_prot(0)
{
}

nes_txc_tw_device::nes_txc_tw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_TXC_TW, tag, owner, clock)
{
}

nes_kof97_device::nes_kof97_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_KOF97, tag, owner, clock)
{
}

nes_kof96_device::nes_kof96_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_KOF96, tag, owner, clock)
	, m_outer(0)
	, m_prot(0)
	, m_mmc3_mode(true) {
}

nes_sf3_device::nes_sf3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_SF3, tag, owner, clock)
{
}

nes_cocoma_device::nes_cocoma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_COCOMA, tag, owner, clock)
{
}

nes_gouder_device::nes_gouder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_GOUDER, tag, owner, clock)
{
}

nes_sa9602b_device::nes_sa9602b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_SA9602B, tag, owner, clock), m_reg(0), m_prg_chip(0)
{
}

nes_sachen_shero_device::nes_sachen_shero_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_SACHEN_SHERO, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
	, m_reg(0)
{
}

nes_sachen_zgdh_device::nes_sachen_zgdh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_SACHEN_ZGDH, tag, owner, clock), m_reg(0)
{
}

nes_a9746_device::nes_a9746_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_A9746, tag, owner, clock)
{
}

nes_a88s1_device::nes_a88s1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_A88S1, tag, owner, clock)
{
}

nes_bmc_el86xc_device::nes_bmc_el86xc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_EL86XC, tag, owner, clock)
{
}

nes_fk23c_device::nes_fk23c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock),
	m_solder_pad(*this, "SOLDER_PAD") 
{
}

nes_fk23c_device::nes_fk23c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_fk23c_device(mconfig, NES_FK23C, tag, owner, clock) 
{
}

nes_fk23ca_device::nes_fk23ca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_fk23c_device(mconfig, NES_FK23CA, tag, owner, clock) 
{
}

nes_nt639_device::nes_nt639_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_NT639, tag, owner, clock)
{
}

nes_resettxrom_device::nes_resettxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_RESETTXROM, tag, owner, clock), m_count(-1)
{
}

nes_s24in1sc03_device::nes_s24in1sc03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_S24IN1SC03, tag, owner, clock)
{
}

nes_tech9in1_device::nes_tech9in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_TECHLINE9IN1, tag, owner, clock)
{
}

nes_bmc_5in1_device::nes_bmc_5in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_5IN1, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
{
}

nes_bmc_8in1_device::nes_bmc_8in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_8IN1, tag, owner, clock)
{
}

nes_bmc_15in1_device::nes_bmc_15in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_15IN1, tag, owner, clock), m_jumper(0)
{
}

nes_bmc_sbig7_device::nes_bmc_sbig7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_BMC_SBIG7, tag, owner, clock)
{
}

nes_bmc_hik8_device::nes_bmc_hik8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_count(0)
{
}

nes_bmc_hik8_device::nes_bmc_hik8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_hik8_device(mconfig, NES_BMC_HIK8, tag, owner, clock)
{
}

nes_bmc_jy208_device::nes_bmc_jy208_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_hik8_device(mconfig, NES_BMC_JY208, tag, owner, clock)
{
}

nes_bmc_jy302_device::nes_bmc_jy302_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_hik8_device(mconfig, NES_BMC_JY302, tag, owner, clock)
{
}

nes_bmc_kc885_device::nes_bmc_kc885_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_hik8_device(mconfig, NES_BMC_KC885, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
{
}

nes_bmc_sfc12_device::nes_bmc_sfc12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_hik8_device(mconfig, NES_BMC_SFC12, tag, owner, clock)
{
}

nes_bmc_hik4_device::nes_bmc_hik4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_HIK4, tag, owner, clock), m_mmc3_mode(true)
{
}

nes_bmc_f15_device::nes_bmc_f15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_F15, tag, owner, clock)
{
}

nes_bmc_f600_device::nes_bmc_f600_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txsrom_device(mconfig, NES_BMC_F600, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
	, m_reg(0)
{
}

nes_bmc_gn45_device::nes_bmc_gn45_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_GN45, tag, owner, clock), m_lock(false)
{
}

nes_bmc_gold7in1_device::nes_bmc_gold7in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_GOLD7IN1, tag, owner, clock), m_lock(false)
{
}

nes_bmc_k3006_device::nes_bmc_k3006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_K3006, tag, owner, clock)
{
}

nes_bmc_k3033_device::nes_bmc_k3033_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_K3033, tag, owner, clock), m_mmc3_mode(false)
{
}

nes_bmc_l6in1_device::nes_bmc_l6in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_L6IN1, tag, owner, clock), m_reg(0)
{
}

nes_bmc_00202650_device::nes_bmc_00202650_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_00202650, tag, owner, clock), m_mmc3_mode(false)
{
}

nes_bmc_411120c_device::nes_bmc_411120c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_411120C, tag, owner, clock), m_reg(0)
{
}

nes_bmc_810305c_device::nes_bmc_810305c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txsrom_device(mconfig, NES_BMC_810305C, tag, owner, clock), m_outer(0)
{
}

nes_bmc_820720c_device::nes_bmc_820720c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_820720C, tag, owner, clock), m_reg(0)
{
}

nes_bmc_830118c_device::nes_bmc_830118c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_830118C, tag, owner, clock)
{
}

nes_bmc_830832c_device::nes_bmc_830832c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_830832C, tag, owner, clock)
{
}

nes_bmc_yy841101c_device::nes_bmc_yy841101c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_YY841101C, tag, owner, clock)
{
}

nes_bmc_yy841155c_device::nes_bmc_yy841155c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_YY841155C, tag, owner, clock)
{
}

nes_pjoy84_device::nes_pjoy84_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_PJOY84, tag, owner, clock)
{
}

nes_smd133_device::nes_smd133_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_SMD133, tag, owner, clock)
{
}


static INPUT_PORTS_START(fk23c)
	PORT_START("SOLDER_PAD")
	PORT_CONFNAME(0x07, 0x01, "FK23C Solder Pad")
	PORT_CONFSETTING(0x00, "Setting 0 - A4 ($5010-$5013)")
	PORT_CONFSETTING(0x01, "Setting 1 - A5 ($5020-$5023)")
	PORT_CONFSETTING(0x02, "Setting 2 - A6 ($5040-$5043)")
	PORT_CONFSETTING(0x03, "Setting 3 - A7 ($5080-$5083)")
	PORT_CONFSETTING(0x04, "Setting 4 - A8 ($5100-$5103)")
	PORT_CONFSETTING(0x05, "Setting 5 - A9 ($5200-$5203)")
	PORT_CONFSETTING(0x06, "Setting 6 - A10 ($5400-$5403)")
	PORT_CONFSETTING(0x07, "Setting 7 - A11 ($5800-$5803)")
INPUT_PORTS_END

ioport_constructor nes_fk23c_device::device_input_ports() const {
	return INPUT_PORTS_NAME(fk23c);
}

void nes_bmw8544_device::device_start()
{
	mmc3_start();

	save_item(NAME(m_reg));
	save_item(NAME(m_bmw_extra_chr));
}

void nes_bmw8544_device::pcb_reset()
{
	m_reg = 0;
	m_bmw_extra_chr[0] = 0;
	m_bmw_extra_chr[1] = 0;

	// The standard MMC3 banks cover the lower 128 KiB PRG region and
	// 256 KiB CHR range. BMW8544's additional address logic extends
	// the banks where required.
	mmc3_common_initialize(0x0f, 0xff, 0);
}

void nes_family4646_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_family4646_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	mmc3_common_initialize(0x1f, 0xff, 0);
	
	// Space Shuttle on CB-4035 does not initialize the MMC3 mirroring
	// register. The observed board starts with horizontal mirroring,
	// although whether this is hardwired remains uncertain.
	set_nt_mirroring(PPU_MIRROR_HORZ);
}

void nes_pikay2k_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_pikay2k_device::pcb_reset()
{
	m_reg[0] = 0xff;
	m_reg[1] = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_8237_device::device_start() {
	mmc3_start();

	save_item(NAME(m_reg));
}

void nes_8237_device::pcb_reset() {
	m_reg[0] = 0x00;
	m_reg[1] = 0x03;
	m_reg[2] = 0x00;

	mmc3_common_initialize(0x1f, 0xff, 0);
	update_banks();
}

void nes_158b_device::device_start()
{
	nes_8237_device::device_start();
	save_item(NAME(m_prot));
}

void nes_158b_device::pcb_reset()
{
	nes_8237_device::pcb_reset();
	m_prot = 7;    // Blood of Jurassic needs this row of prot_table
}

void nes_kasing_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_kasing_device::pcb_reset()
{
	m_mmc3_mode = true;
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
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	m_low_reg = 0;
	mmc3_common_initialize(0x1f, 0xff, 0);
}


void nes_h2288_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_h2288_device::pcb_reset()
{
	m_mmc3_mode = true;
	mmc3_common_initialize(0x3f, 0xff, 0);
}

void nes_6035052_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_prot));
}

void nes_6035052_device::pcb_reset()
{
	mmc3_common_initialize(0xff, 0xff, 0);
	m_prot = 0;
}

void nes_kof96_device::device_start() {
	mmc3_start();

	save_item(NAME(m_outer));
	save_item(NAME(m_prot));
	save_item(NAME(m_mmc3_mode));
}

void nes_kof96_device::pcb_reset() {
	m_outer = 0;
	m_prot = 0;
	m_mmc3_mode = true;

	mmc3_common_initialize(0x3f, 0xff, 0);
}

void nes_cocoma_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_gouder_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_gouder_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	mmc3_common_initialize(0xff, 0xff, 0);
	
	// The PRG/mirroring register powers up as $11.
	prg32(bitswap<2>(0x11, 4, 0));
	set_nt_mirroring(PPU_MIRROR_VERT);
}

void nes_sa9602b_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_prg_chip));
}

void nes_sa9602b_device::pcb_reset() {
	m_reg = 0;
	m_prg_chip = 0;

	// Six MMC3 PRG bank bits select within a 512 KiB chip.
	// The board has 32 KiB of battery-backed CHR-RAM.
	mmc3_common_initialize(0x3f, 0x1f, 0);
	//	mmc3_common_initialize(0x1ff, 0xff, 0);    // 1.5MB of PRG-ROM, no CHR-ROM but 32K CHR-RAM
}



void nes_sachen_shero_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_sachen_shero_device::pcb_reset()
{
	m_reg = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_sachen_zgdh_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_sachen_zgdh_device::pcb_reset()
{
	assert(m_vram.size() >= 0x2000);

	m_reg = 0;
	mmc3_common_initialize(0x1f, 0x7f, 0);
}

void nes_a9746_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_a9746_device::pcb_reset()
{
	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
	mmc3_common_initialize(0x7f, 0xff, 0);
}

void nes_a88s1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));

	// power-on values for registers; they don't seem to change with soft reset
	m_reg[0] = 0x80;
	m_reg[1] = 0x82;
}

void nes_a88s1_device::pcb_reset()
{
	mmc3_common_initialize(0x1f, 0xff, 0);
	update_banks();
}

void nes_bmc_el86xc_device::pcb_reset()
{
	mmc3_common_initialize((m_outer_prg_size >> 3) - 1, 0x7f, 0);
}

void nes_fk23c_device::device_start() {
	mmc3_start();

	save_item(NAME(m_reg));
	save_item(NAME(m_mmc_cmd1));
	save_item(NAME(m_wram_bank));
	save_item(NAME(m_ram_in_first_chr));
	save_item(NAME(m_single_screen_mirroring));
	save_item(NAME(m_fk23_registers_enabled));
	save_item(NAME(m_wram_config_enabled));
	save_item(NAME(m_wram_enabled));
	save_item(NAME(m_wram_write_protected));
	save_item(NAME(m_powered));
}

void nes_fk23c_device::pcb_reset() {
	if (m_powered && m_wram_config_enabled && BIT(m_reg[0], 5) && !m_battery.empty()) {
		// Battery-backed CHR-RAM cartridges preserve their mapper
		// state but clear the outer PRG-ROM bank bits on soft reset.
		m_reg[0] &= ~0x88;
		m_reg[1] = 0;
		m_reg[2] &= ~0x40;
		fk23c_update_state();
		return;
	}

	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);

	// Submapper 1 cartridges with 1 MiB each of PRG-ROM and
	// CHR-ROM power up in the second 512 KiB PRG-ROM region.
	if (m_prg_size == 0x100000 && m_vrom_size == 0x100000) {
		m_reg[1] = 0x20;
	}

	m_reg[4] = 0xff;
	m_reg[5] = 0xff;
	m_reg[6] = 0xff;
	m_reg[7] = 0xff;

	m_mmc_cmd1 = 0;
	m_wram_bank = 0;
	m_ram_in_first_chr = false;
	m_single_screen_mirroring = false;
	m_fk23_registers_enabled = false;
	m_wram_config_enabled = false;
	m_wram_enabled = false;
	m_wram_write_protected = false;

	mmc3_common_initialize(0xff, 0xff, 0);

	m_mmc_prg_bank[0] = 0x00;
	m_mmc_prg_bank[1] = 0x01;
	m_mmc_prg_bank[2] = 0xfe;
	m_mmc_prg_bank[3] = 0xff;

	m_wram_protect = 0;
	m_powered = true;

	fk23c_update_state();
}

void nes_fk23ca_device::pcb_reset() {
	nes_fk23c_device::pcb_reset();
}

void nes_nt639_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0xff, 0);
}

// These reset-based PCBs are emulated here with no additional handlers
void nes_resettxrom_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_count));
}

void nes_resettxrom_device::pcb_reset()
{
	mmc3_common_initialize((m_outer_prg_size >> 3) - 1, m_outer_chr_size - 1, 0);

	m_count = (m_count + 1) & 3;
	m_prg_base = m_count << (std::countr_zero(m_outer_prg_size) - 3);
	m_chr_base = m_count << std::countr_zero(m_outer_chr_size);
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_s24in1sc03_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_s24in1sc03_device::pcb_reset()
{
	m_reg[0] = 0x24;
	m_reg[1] = 0x9f;
	m_reg[2] = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_tech9in1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_tech9in1_device::pcb_reset()
{
	m_reg[0] = m_reg[1] = m_reg[2] = 0;
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_bmc_5in1_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
	prg32(0);
}

void nes_bmc_8in1_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
	prg32(0);
}

void nes_bmc_15in1_device::pcb_reset()
{
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_bmc_sbig7_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_hik8_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_count));
}

void nes_bmc_hik8_device::pcb_reset() {
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_count = 0;
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);

	mmc3_common_initialize(0x3f, 0xff, 0);
}

void nes_bmc_kc885_device::pcb_reset()
{
	nes_bmc_hik8_device::pcb_reset();
	m_prg_mask = 0x1f;
	set_prg(m_prg_base, m_prg_mask);
}

void nes_bmc_hik4_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_bmc_hik4_device::pcb_reset()
{
	m_mmc3_mode = true;
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_f15_device::pcb_reset()
{
	mmc3_common_initialize(0x1f, 0xff, 0);
	prg16_89ab(0);
	prg16_cdef(0);
}

void nes_bmc_f600_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_f600_device::pcb_reset()
{
	m_reg = 0;
	mmc3_common_initialize(0x1f, 0x7f, 0);
}

void nes_bmc_gn45_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_lock));
}

void nes_bmc_gn45_device::pcb_reset()
{
	m_lock = false;
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_gold7in1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_lock));
}

void nes_bmc_gold7in1_device::pcb_reset()
{
	m_lock = false;
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_bmc_k3006_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
	prg16_89ab(0);
	prg16_cdef(0);
}

void nes_bmc_k3033_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_bmc_k3033_device::pcb_reset()
{
	m_mmc3_mode = false;
	mmc3_common_initialize(0x0f, 0x7f, 0);
	prg16_89ab(0);
	prg16_cdef(0);
}

void nes_bmc_l6in1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_l6in1_device::pcb_reset()
{
	m_reg = 0;
	mmc3_common_initialize(0x0f, 0x07, 0);
}

void nes_bmc_00202650_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_bmc_00202650_device::pcb_reset()
{
	m_mmc3_mode = false;
	mmc3_common_initialize(0x0f, 0x7f, 0);

	// PCB has an extra 32KB boot menu chip, only the menu uses the CHRRAM
	prg32((m_prg_chunks >> 1) - 1);
	chr8(0, CHRRAM);
}

void nes_bmc_411120c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_411120c_device::pcb_reset()
{
	m_reg = 0;
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_810305c_device::device_start()
{
	nes_txsrom_device::device_start();

	save_item(NAME(m_outer));
	save_item(NAME(m_tks_mir));
	save_item(NAME(m_ppu_chr_bus));
}

void nes_bmc_810305c_device::pcb_reset()
{
	nes_txsrom_device::pcb_reset();

	m_outer = 0;
	m_ppu_chr_bus = 0;

	for (auto &mir : m_tks_mir)
		mir = 0;

	m_prg_base = 0;
	m_prg_mask = 0x1f;

	m_chr_base = 0;
	m_chr_mask = 0x7f;

	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}


void nes_bmc_820720c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_820720c_device::pcb_reset()
{
	m_reg = 0;
	mmc3_common_initialize(0x0f, 0xff, 0);
}

void nes_bmc_830118c_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_830832c_device::pcb_reset()
{
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_bmc_yy841101c_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_yy841155c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_yy841155c_device::pcb_reset() {
	m_reg[0] = 0;
	m_reg[1] = 0;

	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_pjoy84_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_pjoy84_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	set_base_mask();
	mmc3_common_initialize(m_prg_mask, m_chr_mask, 0);
}

void nes_smd133_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_smd133_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	mmc3_common_initialize(0x3f, 0xff, 0);
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board by Nitra

 Games: Queen Bee, Time Diver Avenger

 MMC3 clone with A10 tied to what is normally A0 and
 data instead coming from the lower address lines.

 iNES: mapper 250

 In MAME: Supported.

 -------------------------------------------------*/

void nes_nitra_device::write_h(offs_t offset, u8 data) {
	LOG("nitra write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write((offset & 0x6000) | BIT(offset, 10), u8(offset));
}

/*-------------------------------------------------

 Board BMW8544

 Games: Dragon Fighter (Flying Star)

  https://www.nesdev.org/wiki/NES_2.0_Mapper_292

 NES 2.0: mapper 292

 In MAME: Supported

 -------------------------------------------------*/

void nes_bmw8544_device::set_prg(int prg_base, int prg_mask) {
	nes_txrom_device::set_prg(prg_base, prg_mask);

	// $6000-$7FFF bits 4-0 override the MMC3 bank at $8000-$9FFF.
	prg8_89(m_reg & 0x1f);
}

void nes_bmw8544_device::set_chr(u8 chr, int chr_base, int chr_mask) {
	// $0000-$07FF:
	// MMC3 R0 2 KiB bank XORed with the sprite XOR register.
	const int bank_0 = ((m_mmc_vrom_bank[0] & 0xfe) >> 1) ^ m_bmw_extra_chr[0];

	// $0800-$0FFF:
	// MMC3 R1 2 KiB bank with bit 7 supplied by bit 6 of the
	// background/sprite register.
	const int bank_1 = ((m_mmc_vrom_bank[1] & 0xfe) >> 1) ^ ((m_bmw_extra_chr[1] << 1) & 0x80);

	chr2_0(bank_0, chr);
	chr2_2(bank_1, chr);
	chr4_4(m_bmw_extra_chr[1] & 0x3f, chr);
}

u8 nes_bmw8544_device::read_m(offs_t offset) {
	LOG("bmw8544 read_m, offset: %04x\n", offset);

	const u8 last_write = m_maincpu6502->get_last_cpu_write_latch();

	m_bmw_extra_chr[BIT(m_reg, 5)] = last_write;
	set_chr(m_chr_source, m_chr_base, m_chr_mask);

	// BMW8544 decodes the read for its side effect but does not drive CPU data.
	return get_open_bus();
}

void nes_bmw8544_device::write_m(offs_t offset, u8 data) {
	LOG("bmw8544 write_m, offset: %04x, data: %02x\n", offset, data);

	m_reg = data;
	set_prg(m_prg_base, m_prg_mask);
}

/*-------------------------------------------------

 Board UNL-FS6

 Games: Street Fighter VI / Fight Street VI

 MMC3 clone with CPU address lines A0 and A1 swapped
 for writes to the mapper registers.

 This is an alternate mapper-196 wiring and is not
 equivalent to the more common mapper-196 address
 decoding used by BTL-SUPERBROS11.

 iNES: mapper 196, alternate wiring

 In MAME: Supported.

 -------------------------------------------------*/

void nes_fs6_device::write_h(offs_t offset, u8 data) {
	LOG("fs6 write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write((offset & ~0x03) | bitswap<2>(offset, 0, 1), data);
}

/*-------------------------------------------------

 BTL-SUPERBROS11

 Games: Super Mario Bros. 11, Super Mario Bros. 17

 MMC3 clone with mapper register selection generated
 from CPU address lines A1-A3.

 For $8000-$BFFF, MMC3 A0 is A1 OR A2 OR A3.
 For $C000-$FFFF, MMC3 A0 is A2 OR A3.

 iNES: mapper 196

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sbros11_device::write_h(offs_t offset, u8 data) {
	LOG("smb11 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 reg_select = BIT(offset, 2) | BIT(offset, 3);

	if (offset < 0x4000)
		reg_select |= BIT(offset, 1);

	txrom_write((offset & 0x6000) | reg_select, data);
}

/*-------------------------------------------------

 UNL-MALISB

 Game: Super Mali Bros Splash Bomb

 MMC3 clone with modified mapper-register address
 decoding and permuted PRG-ROM and CHR-ROM address
 lines.

 MMC3 register A0 is CPU A3 below $C000. At
 $C000-$FFFF, MMC3 register A0 is CPU A3 OR CPU A2.

 NES 2.0: mapper 325

 In MAME: Supported.

 -------------------------------------------------*/

void nes_malisb_device::prg_cb(int start, int bank) {
	bank = bitswap<4>(bank, 2, 3, 1, 0);
	prg8_x(start, bank);
}

void nes_malisb_device::chr_cb(int start, int bank, int source) {
	bank = bitswap<8>(bank, 7, 6, 1, 4, 3, 2, 5, 0);
	chr1_x(start, bank, source);
}

void nes_malisb_device::write_h(offs_t offset, u8 data) {
	LOG("malisb write_h, offset: %04x, data: %02x\n", offset, data);

	const offs_t mmc3_offset = (offset & 0x6000) | BIT(offset, 3) | (BIT(offset, 14) & BIT(offset, 2));

	txrom_write(mmc3_offset, data);
}

/*-------------------------------------------------

 BMC-FAMILY-4646B

 Known Boards: Unknown Multigame Bootleg Board (4646B)
 Games: 2 in 1 - Family Kid & Aladdin 4, various
 multicarts.

 MMC3 clone with outer PRG-ROM and CHR-ROM banking,
 optional NROM/CNROM modes, and register locking.

 iNES: mapper 134

 In MAME: Preliminary supported.

 TODO: Register $6003 receives writes, although only
 $00 has been observed. Some cartridges may expose
 solder-pad settings through reads at $8000. YH-4103
 reportedly behaves differently on soft reset.

 -------------------------------------------------*/

void nes_family4646_device::prg_cb(int start, int bank) {
	if ((m_reg[1] & 0x88) == 0x80) {
		// NROM-256 mode. The selected 32 KiB bank comes entirely
		// from the outer registers and ignores MMC3 PRG banks.
		if (start == 0) {
			const int nrom_bank = (m_reg[0] & 0x10) | ((m_reg[1] & 0x02) << 2);

			prg32(nrom_bank);
		}

		return;
	}

	if ((m_reg[1] & 0x88) == 0x88) {
		// NROM-128 mode. Each 16 KiB CPU region receives the
		// selected bank, with the normal MMC3 bank contributing
		// its bit 4.
		const int nrom_bank = ((m_reg[0] & 0x10) << 1) | ((bank & 0x10) >> 1) | ((m_reg[1] & 0x03) << 3);

		if (start < 2)
			prg16_89ab(nrom_bank);
		else
			prg16_cdef(nrom_bank);

		return;
	}

	int mapped_bank;

	if (BIT(m_reg[1], 2)) {
		// 128 KiB MMC3 inner region.
		mapped_bank = ((m_reg[0] & 0x10) << 2) | ((m_reg[1] & 0x03) << 4) | (bank & 0x0f);
	}
	else {
		// 256 KiB MMC3 inner region.
		mapped_bank = ((m_reg[0] & 0x10) << 2) | ((m_reg[1] & 0x02) << 4) | (bank & 0x1f);
	}

	prg8_x(start, mapped_bank);
}

void nes_family4646_device::chr_cb(int start, int bank, int source) {
	if (BIT(m_reg[0], 3)) {
		// CNROM mode. $6002 directly selects the 8 KiB CHR bank.
		if (start == 0)
			chr8(m_reg[2], source);

		return;
	}

	int mapped_bank;

	if (BIT(m_reg[1], 6)) {
		// 128 KiB MMC3 inner CHR region.
		mapped_bank = ((m_reg[0] & 0x20) << 4) | ((m_reg[1] & 0x30) << 3) | (bank & 0x7f);
	}
	else {
		// 256 KiB MMC3 inner CHR region.
		mapped_bank = ((m_reg[0] & 0x20) << 4) | ((m_reg[1] & 0x20) << 3) | (bank & 0xff);
	}

	chr1_x(start, mapped_bank, source);
}

void nes_family4646_device::write_m(offs_t offset, u8 data) {
	LOG("family4646 write_m, offset: %04x, data: %02x\n", offset, data);

	const int reg = offset & 0x03;

	if (BIT(m_reg[0], 7)) {
		// Once locked, only $6002 bits 0-1 remain writable.
		if (reg != 2)
			return;

		m_reg[2] = (m_reg[2] & 0xfc) | (data & 0x03);
	}
	else {
		m_reg[reg] = data;
	}

	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/*-------------------------------------------------

 BTL-PIKACHUY2K

 Game: Pikachu Y2K

 MMC3 clone with scrambled PRG-RAM reads.

 Before the first write to $8000, PRG-RAM reads are
 XORed with the key written to $A001. An exact write
 to $8000 permanently disables the scrambling until
 reset.

 PRG-RAM writes are not scrambled.

 iNES: mapper 254

 In MAME: Supported.

 -------------------------------------------------*/

void nes_pikay2k_device::write_h(offs_t offset, u8 data) {
	LOG("pikay2k write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset == 0x0000) {
		// An exact write to CPU $8000 disables PRG-RAM read
		// scrambling until the mapper is reset.
		m_reg[0] = 0x00;
	}
	else if (offset == 0x2001) {
		// An exact write to CPU $A001 sets the scrambling key.
		m_reg[1] = data;
	}

	// These addresses retain their normal MMC3 functions.
	txrom_write(offset, data);
}

void nes_pikay2k_device::write_m(offs_t offset, u8 data) {
	LOG("pikay2k write_m, offset: %04x, data: %02x\n", offset, data);

	// PRG-RAM writes are always stored without scrambling.
	m_prgram[offset & 0x1fff] = data;
}

u8 nes_pikay2k_device::read_m(offs_t offset) {
	LOG("pikay2k read_m, offset: %04x\n", offset);

	// m_reg[0] is $FF while scrambling is enabled and $00 after
	// the first exact write to $8000.
	return m_prgram[offset & 0x1fff] ^ (m_reg[0] & m_reg[1]);
}

/*-------------------------------------------------

 Board UNL-8237, UNL-8237A

 Games: Pocahontas 2, MK3, various multicarts

 MMC3 clone with register and address scrambling and
 a few extra banking modes selected by writes to
 $5000-$5FFF.

 NES 2.0: mapper 215, submappers 0 and 1

 In MAME: Preliminary supported.

 TODO: The Lion King intentionally freezes after the first boss
 when its PRG-ROM integrity check, executed from RAM at $0682,
 does not match the expected values. This also occurs in other
 mapper implementations and is believed to be a bug in the
 multicart's modified game data. Behavior on the original
 cartridge has not been verified.

 Soft reset from the included games returns to Lion King rather
 than the multicart menu. Original hardware behavior is unknown.

 -------------------------------------------------*/

void nes_8237_device::update_banks()
{
	int a17 = BIT(m_reg[0], 6);

	m_chr_base = m_board ? (m_reg[1] & 0x0e) << 7 : (m_reg[1] & 0x0c) << 6;
	m_chr_base |= (BIT(m_reg[1], 5) & a17) << 7;
	m_chr_mask = 0xff >> a17;
	set_chr(m_chr_source, m_chr_base, m_chr_mask);

	m_prg_base = m_board ? bitswap<3>(m_reg[1], 3, 1, 0) : m_reg[1] & 0x03;
	m_prg_base = (m_prg_base << 5) | (m_reg[1] & (a17 << 4));
	m_prg_mask = 0x1f >> a17;

	if (BIT(m_reg[0], 7))
	{
		// $5000.7 = 1:
		// NROM override mode. Ignore MMC3 PRG bank registers and use $5000 bits.
		//
		// $5000 bits 0-3 select the 16 KiB PRG bank.
		// $5000 bit 5 selects whether bit 0 is replaced by CPU A14:
		//   0 = NROM-128 style, mirror one 16 KiB bank into both halves
		//   1 = NROM-256 style, map adjacent 16 KiB banks as a 32 KiB area
		u8 bank = (m_prg_base >> 1) | (m_reg[0] & (m_prg_mask >> 1));
		u8 mode = BIT(m_reg[0], 5);

		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
	else
	{
		// $5000.7 = 0:
		// Normal MMC3 PRG banking, constrained by the selected outer PRG bank.
		set_prg(m_prg_base, m_prg_mask);
	}
}

void nes_8237_device::prg_cb(int start, int bank)
{
	if (!BIT(m_reg[0], 7))
		nes_txrom_device::prg_cb(start, bank);
}

void nes_8237_device::write_l(offs_t offset, u8 data)
{
	LOG("unl_8237 write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	switch (offset & 0x1007)
	{
		case 0x1000:
		case 0x1001:
			m_reg[offset & 1] = data;
			update_banks();
			break;
		case 0x1007:
			m_reg[2] = data & 0x07;
			break;
	}
}

void nes_8237_device::write_h(offs_t offset, u8 data)
{
	LOG("unl_8237 write_h, offset: %04x, data: %02x\n", offset, data);

	static constexpr u8 reg_table[8][8] =
	{
		{0, 1, 2, 3, 4, 5, 6, 7},
		{0, 2, 6, 1, 7, 3, 4, 5},
		{0, 5, 4, 1, 7, 2, 6, 3},
		{0, 6, 3, 7, 5, 2, 4, 1},
		{0, 2, 5, 3, 6, 1, 7, 4},
		{0, 1, 2, 3, 4, 5, 6, 7},
		{0, 1, 2, 3, 4, 5, 6, 7},
		{0, 1, 2, 3, 4, 5, 6, 7}
	};

	static constexpr u16 addr_table[8][8] =
	{
		{0x8000, 0x8001, 0xa000, 0xa001, 0xc000, 0xc001, 0xe000, 0xe001},
		{0xa001, 0xa000, 0x8000, 0xc000, 0x8001, 0xc001, 0xe000, 0xe001},
		{0x8000, 0x8001, 0xa000, 0xa001, 0xc000, 0xc001, 0xe000, 0xe001},
		{0xc001, 0x8000, 0x8001, 0xa000, 0xa001, 0xe001, 0xe000, 0xc000},
		{0xa001, 0x8001, 0x8000, 0xc000, 0xa000, 0xc001, 0xe000, 0xe001},
		{0x8000, 0x8001, 0xa000, 0xa001, 0xc000, 0xc001, 0xe000, 0xe001},
		{0x8000, 0x8001, 0xa000, 0xa001, 0xc000, 0xc001, 0xe000, 0xe001},
		{0x8000, 0x8001, 0xa000, 0xa001, 0xc000, 0xc001, 0xe000, 0xe001},
	};

	u16 addr = addr_table[m_reg[2]][bitswap<3>(offset, 14, 13, 0)];
	if (addr == 0x8000)
		data = (data & 0xc0) | reg_table[m_reg[2]][data & 0x07];
	txrom_write(addr & 0x6001, data);
}

/*-------------------------------------------------

 Board UNL-158B

 Games: Blood of Jurassic, Super Hang-On

 MMC3 clone based on the UNL-8237 design with
 additional protection reads throughout $5000-$5FFF.

 Blood of Jurassic uses a Zapper. Super Hang-On is
 a protected hack of Rad Racer/Highway Star.

 NES 2.0: mapper 258

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_158b_device::read_l(offs_t offset)
{
	LOG("unl_158b read_l, offset: %04x\n", offset);

	static constexpr u8 prot_table[8][8] =
	{
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x05, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0f, 0x00}
	};

	u8 temp = get_open_bus();

	offset += 0x100;
	if (offset >= 0x1000)
		temp = (temp & 0xf0) | prot_table[m_prot][offset & 0x07];

	return temp;
}

void nes_158b_device::write_l(offs_t offset, u8 data)
{
	LOG("unl_158b write_l, offset: %04x, data: %02x\n", offset, data);

	switch ((offset + 0x100) & 0x1003)
	{
		case 0x1002:
			m_prot = data & 0x07;
			break;
		default:
			nes_8237_device::write_l(offset, data);
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board by Kasing

 Games: AV Jiu Ji Mahjong, Bao Qing Tian, Thunderbolt 2,
 Shisen Mahjong 2, Garou Densetsu Special

 MMC3 clone with extra banking modes at 0x6000-0x7fff.

 iNES: mapper 115

 In MAME: Supported.

 -------------------------------------------------*/

void nes_kasing_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)
		nes_txrom_device::prg_cb(start, bank);
}

void nes_kasing_device::write_m(offs_t offset, u8 data)
{
	LOG("kasing write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 0))
	{
		m_chr_base = BIT(data, 0) << 8;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else
	{
		m_mmc3_mode = !BIT(data, 7);
		if (m_mmc3_mode)
			set_prg(m_prg_base, m_prg_mask);
		else
		{
			u8 bank = data & 0x0f;
			u8 mode = BIT(data, 5);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
	}
}

/*-------------------------------------------------

 Bootleg Boards by Super Game and Hosenkan

 Games: The Lion King, Aladdin, Boogerman, Pocohon,
 Super Donkey Kong

 MMC3 clone with register and address scrambling and
 a few extra banking modes by writing 0x6000-0x7fff.
 This is the same as mapper 115 with scrambling.

 iNES: mapper 114 (and 182)

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sglionk_device::write_h(offs_t offset, u8 data)
{
	LOG("sglionk write_h, offset: %04x, data: %02x\n", offset, data);

	static constexpr u8 reg_table[2][8] =
	{
		{0, 3, 1, 5, 6, 7, 2, 4},
		{0, 2, 5, 3, 6, 1, 7, 4}
	};

	static constexpr u16 addr_table[2][8] =
	{
		{0xa001, 0xa000, 0x8000, 0xc000, 0x8001, 0xc001, 0xe000, 0xe001},
		{0xa001, 0x8001, 0x8000, 0xc001, 0xa000, 0xc000, 0xe000, 0xe001}
	};

	u16 addr = addr_table[m_board][bitswap<3>(offset, 14, 13, 0)];
	if (addr == 0x8000)
		data = (data & 0xc0) | reg_table[m_board][data & 0x07];
	txrom_write(addr & 0x6001, data);
}

/*-------------------------------------------------

 A9711/A9713 Bootleg Boards by Kay (for Panda Prince)

 Games: The Panda Prince, Sonic 3d Blast 6, SFZ2 '97, YuYu '97
 (and its title hack MK6), UMK3, Super Lion King 2

 MMC3 clone. This is basically KOF96 board + protection

 iNES: mapper 121

 In MAME: Supported.

 TODO: Most of the fighting games have graphical issues
 with backgrounds on some stages. Due to common timing
 issue with PPU/MMC3? Super Lion King 2's title screen
 is also inexplicably broken.

 -------------------------------------------------*/

void nes_kay_device::update_protection() {
	switch (m_reg[4]) {
		case 0x20:
		case 0x29:
		case 0x2b:
		case 0x3c:
		case 0x3f:
			m_reg[2] = m_reg[3];
			break;

		case 0x2c:
			if (m_reg[3])
				m_reg[2] = m_reg[3];
			break;

		case 0x26:
			m_reg[2] = m_reg[3];
			break;

		case 0x28:
			m_reg[1] = m_reg[3];
			break;

		case 0x2a:
			m_reg[0] = m_reg[3];
			break;

		case 0x2f:
			break;

		default:
			m_reg[4] = 0;
			break;
	}
}

void nes_kay_device::write_l(offs_t offset, u8 data)
{
	LOG("kay write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000)
		m_low_reg = data & 0x03;

	if ((offset & 0x1180) == 0x1180)  // only written by Super 3-in-1
	{
		m_prg_base = (data & 0x80) >> 2;
		set_prg(m_prg_base, m_prg_mask);
		m_chr_base = m_prg_base << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

u8 nes_kay_device::read_l(offs_t offset)
{
	LOG("kay read_l, offset: %04x\n", offset);

	static constexpr u8 prot_table[4] = {0x83, 0x83, 0x42, 0x00};

	offset += 0x100;
	if (offset >= 0x1000)
		return prot_table[m_low_reg];
	else
		return get_open_bus();
}

void nes_kay_device::chr_cb(int start, int bank, int source) {
	// CHR A18 is connected directly to physical PPU A12.
	// It does not follow MMC3 CHR inversion.
	bank |= BIT(start, 2) << 8;
	chr1_x(start, bank, source);
}

void nes_kay_device::set_prg(int prg_base, int prg_mask) {
	nes_txrom_device::set_prg(prg_base, prg_mask);

	// A nonzero valid protection index keeps the three override
	// banks active. Invalid indices are converted to zero.
	if (m_reg[4]) {
		prg8_ab(prg_base | m_reg[0]);
		prg8_cd(prg_base | m_reg[1]);
		prg8_ef(prg_base | m_reg[2]);
	}
}

void nes_kay_device::write_h(offs_t offset, u8 data) {
	LOG("kay write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6003) {
		case 0x0001:
			// $8001 is both the normal MMC3 bank-data register and
			// the source of the protection unit's bit-reversed value.
			m_reg[3] = bitswap<6>(data, 0, 1, 2, 3, 4, 5);

			// These indices continue following subsequent $8001 writes.
			switch (m_reg[4]) {
				case 0x26:
				case 0x28:
				case 0x2a:
					update_protection();
					break;
			}

			txrom_write(offset, data);
			set_prg(m_prg_base, m_prg_mask);
			break;

		case 0x0003:
			// Writing $8003 also writes the same value to the MMC3
			// $8000 bank-select register, not the $8001 data register.
			txrom_write(offset & ~offs_t(1), data);

			m_reg[4] = data & 0x3f;
			update_protection();
			set_prg(m_prg_base, m_prg_mask);
			break;

		default:
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 UNL-H2288

 Games: Earthworm Jim 2, Ultimate Mortal Kombat 3

 iNES: mapper 123

 In MAME: Supported.

 -------------------------------------------------*/

void nes_h2288_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)
		nes_txrom_device::prg_cb(start, bank);
}

void nes_h2288_device::write_l(offs_t offset, u8 data)
{
	LOG("h2288 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1800)
	{
		m_mmc3_mode = !BIT(data, 6);
		if (m_mmc3_mode)
			set_prg(m_prg_base, m_prg_mask);
		else
		{
			u8 bank = bitswap<4>(data, 5, 2, 3, 0);
			u8 mode = BIT(data, 1);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
	}
}

void nes_h2288_device::write_h(offs_t offset, u8 data)
{
	LOG("h2288 write_h, offset: %04x, data: %02x\n", offset, data);

	static constexpr u8 reg_table[8] = {0, 3, 1, 5, 6, 7, 2, 4};

	if (!(offset & 0x6001))
		data = (data & 0xc0) | reg_table[data & 0x07];
	txrom_write(offset, data);
}

/*-------------------------------------------------

 UNL-603-5052

 MMC3 + protection access in 0x4020 - 0x7fff

 in MAME: Partial support

 -------------------------------------------------*/

void nes_6035052_device::write_ex(offs_t offset, uint8_t data)
{
	LOG("6035052 write_ex, offset: %04x, data: %02x\n", offset, data);
	m_prot = data & 0x03;
	if (m_prot == 1)
		m_prot = 2;
}

uint8_t nes_6035052_device::read_ex(offs_t offset)
{
	LOG("6035052 read_ex, offset: %04x\n", offset);
	return m_prot;
}

/*-------------------------------------------------

 Bootleg Board 'Thunder Warrior' by TXC

 Games: Master Fighter II, Master Fighter 3, Thunder Warrior

 MMC3 clone

 iNES: mapper 189

 In MAME: Supported.

 -------------------------------------------------*/

void nes_txc_tw_device::write_l(offs_t offset, uint8_t data) {
	LOG("txc_tw write_l, offset: %04x, data: %02x\n", offset, data);

	prg32((data | (data >> 4)) & 0x0f);
}

void nes_txc_tw_device::prg_cb(int start, int bank) {
}

/*-------------------------------------------------

 Board UNL-KOF97

 Games: King of Fighters 97 (Rex Soft), Boogerman II

 MMC3 clone

 In MAME: Supported.

 -------------------------------------------------*/

void nes_kof97_device::write_h(offs_t offset, u8 data)
{
	LOG("kof97 write_h, offset: %04x, data: %02x\n", offset, data);

	// unscramble address and data
	offset = (offset & 0x6000) | BIT(offset, 12);
	data = bitswap<8>(data, 7, 6, 2, 4, 3, 0, 5, 1);

	txrom_write(offset, data);
}

/*-------------------------------------------------

 Bootleg Board for KOF96

 Games: The King of Fighters 96, Street Fighter Zero 2

 MMC3 clone

 iNES: mapper 187

 In MAME: Supported.

 -------------------------------------------------*/

void nes_kof96_device::update_kof96_prg() {
	m_mmc3_mode = !BIT(m_outer, 7);

	if (m_mmc3_mode) {
		set_prg(m_prg_base, m_prg_mask);
		return;
	}

	const u8 bank = m_outer & 0x1f;

	if (!BIT(m_outer, 5)) {
		// Mirror one selected 16 KiB bank into both CPU halves.
		prg16_89ab(bank);
		prg16_cdef(bank);
	}
	else if (BIT(m_outer, 6)) {
		// 32 KiB mode using bits 4-2.
		prg32(bank >> 2);
	}
	else {
		// Alternate 32 KiB mode using bits 4-1.
		prg32(bank >> 1);
	}
}

void nes_kof96_device::prg_cb(int start, int bank) {
	if (m_mmc3_mode)
		nes_txrom_device::prg_cb(start, bank & 0x3f);
}

void nes_kof96_device::chr_cb(int start, int bank, int source) {
	if (BIT(start, 2) == BIT(m_latch, 7))
		bank |= 0x100;

	chr1_x(start, bank, source);
}

void nes_kof96_device::write_l(offs_t offset, u8 data) {
	LOG("kof96 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;

	// The outer PRG register responds at CPU $5000.
	if (offset == 0x1000) {
		m_outer = data;
		update_kof96_prg();
	}
}

void nes_kof96_device::write_m(offs_t offset, u8 data) {
	LOG("kof96 write_m, offset: %04x, data: %02x\n", offset, data);

	// CPU $6000 is another write location for the outer register.
	if (offset == 0x0000) {
		m_outer = data;
		update_kof96_prg();
	}
}

u8 nes_kof96_device::read_l(offs_t offset) {
	LOG("kof96 read_l, offset: %04x\n", offset);

	static constexpr u8 prot_data[4] = {
		0x83, 0x83, 0x42, 0x00
	};

	offset += 0x100;

	if (offset >= 0x1000)
		return prot_data[m_prot & 0x03];

	return get_open_bus();
}

void nes_kof96_device::write_h(offs_t offset, u8 data) {
	LOG("kof96 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset == 0x0000) {
		m_prot = 1;
		txrom_write(offset, data);
	}
	else if (offset == 0x0001) {
		if (m_prot == 1)
			txrom_write(offset, data);
	}
	else {
		txrom_write(offset, data);
	}
}

/*-------------------------------------------------

 Bootleg Board for Super Fighter III

 MMC3 clone

 iNES: mapper 197

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sf3_device::set_chr(uint8_t chr_source, int chr_base, int chr_mask) {
	if (!BIT(m_latch, 7)) {
		// Normal arrangement:
		// R0 supplies one 4 KiB bank at $0000.
		// R2 and R3 supply two 2 KiB banks at $1000 and $1800.
		chr4_0(chr_base | ((m_mmc_vrom_bank[0] >> 1) & chr_mask), chr_source);
		chr2_4(chr_base | (m_mmc_vrom_bank[2] & chr_mask), chr_source);
		chr2_6(chr_base | (m_mmc_vrom_bank[3] & chr_mask), chr_source);
	}
	else {
		// Inverted arrangement:
		// R2 supplies the 4 KiB bank at $0000.
		// The two halves of R0 supply $1000 and $1800.
		chr4_0(chr_base | ((m_mmc_vrom_bank[2] >> 1) & chr_mask), chr_source);
		chr2_4(chr_base | (m_mmc_vrom_bank[0] & chr_mask), chr_source);
		chr2_6(chr_base | ((m_mmc_vrom_bank[0] | 0x01) & chr_mask), chr_source);
	}
}

void nes_sf3_device::write_h(offs_t offset, uint8_t data) {
	LOG("sf3 write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write(offset, data);
}

/*-------------------------------------------------

 Cocoma Core Pro Board

 Games: BrilliantCom Cocoma Pack 1 and 2

 MMC3 clone with an extra outer bank latch.

 NES 2.0: mapper 516

 In MAME: Supported.

 -------------------------------------------------*/

void nes_cocoma_device::write_h(offs_t offset, u8 data)
{
	LOG("cocoma write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 4))
	{
		m_prg_base = (offset & 0x03) << 4;
		set_prg(m_prg_base, m_prg_mask);
		m_chr_base = (offset & 0x0c) << 5;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else
		txrom_write(offset, data);
}

/*-------------------------------------------------

 Bootleg Board 37017 (?) by Gouder

 Games: Street Fighter IV

 MMC3 clone. It also requires reads from 0x5000-0x7fff.

 iNES: mapper 208

 In MAME: Supported

 -------------------------------------------------*/

void nes_gouder_device::write_m(offs_t offset, u8 data) {
	LOG("gouder write_m, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x1800) {
		case 0x0800:	// $6800-$6FFF
			prg32(bitswap<2>(data, 4, 0));
			set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x1000:	// $7000-$77FF
			m_reg[4] = data;
			break;

		case 0x1800:	// $7800-$7FFF
			m_reg[offset & 0x03] = data ^ gouder_conv_table[m_reg[4]];
			break;
	}
}

u8 nes_gouder_device::read_m(offs_t offset) {
	LOG("gouder read_m, offset: %04x\n", offset);

	if ((offset & 0x1800) == 0x1800)
		return m_reg[offset & 0x03];

	return get_open_bus();
}

void nes_gouder_device::write_l(offs_t offset, u8 data)
{
	LOG("gouder write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	switch (offset & 0x1800)
	{
		case 0x0800:
			prg32(bitswap<2>(data, 4, 0));
			set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x1000:
			m_reg[4] = data;
			break;
		case 0x1800:
			m_reg[offset & 0x03] = data ^ gouder_conv_table[m_reg[4]];
			break;
	}
}

u8 nes_gouder_device::read_l(offs_t offset)
{
	LOG("gouder read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (offset >= 0x1800)
		return m_reg[offset & 0x03];

	return get_open_bus();
}

// writes to 0x8000-0xffff are like MMC3 but no PRG bankswitch (because it is handled by low writes)
void nes_gouder_device::prg_cb(int start, int bank)
{
}


/*-------------------------------------------------

 UNL-SA-9602B

 Sachen boards used for a chinese port of Princess Maker (?)

 in MAME: Very Preliminary support, based on Cah4e3
 code in FCEUMM
 
 NES 2.0 Mapper 513

 -------------------------------------------------*/

void nes_sa9602b_device::prg_cb(int start, int bank) {
	// Values 0-2 select one of the three 512 KiB PRG-ROM chips.
	//
	// Selecting value 3 would enable two chips simultaneously on the
	// documented hardware. Preserve FCEUX's behavior of treating it as
	// a mirror of chip 2, but retain the original register value.
	const u8 prg_chip = (m_prg_chip == 3) ? 2 : m_prg_chip;

	prg8_x(start, (prg_chip << 6) | (bank & 0x3f));

	// The two fixed 8 KiB banks always come from outer PRG chip 0.
	if (BIT(m_latch, 6))
		prg8_89(0x3e);
	else
		prg8_cd(0x3e);

	prg8_ef(0x3f);
}

void nes_sa9602b_device::write_h(offs_t offset, uint8_t data) {
	LOG("sa9602b write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001) {
		case 0x0000:
			m_reg = data;
			txrom_write(offset, data);
			break;

		case 0x0001:
			if ((m_reg & 0x07) < 6) {
				// MMC3 CHR-register writes use D7-D6 to select the
				// 512 KiB PRG-ROM chip.
				m_prg_chip = BIT(data, 6, 2);

				txrom_write(offset, data);

				// A CHR-register write normally only reapplies CHR
				// banking, but it also changed this board's outer
				// PRG chip.
				set_prg(m_prg_base, m_prg_mask);
			}
			else {
				txrom_write(offset, data);
			}
			break;

		default:
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 UNL-SHERO

 Sachen boards used for Street Heroes

 NES 2.0: mapper 262

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sachen_shero_device::chr_cb(int start, int bank, int source)
{
	static constexpr u8 shift[4] = {5, 6, 8, 7};

	if (!BIT(m_reg, 6))
		chr1_x(start, ((m_reg << shift[start >> 1]) & 0x100) | bank, source);
}

void nes_sachen_shero_device::write_l(offs_t offset, u8 data)
{
	LOG("shero write_l, offset: %04x, data: %02x\n", offset, data);
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

u8 nes_sachen_shero_device::read_l(offs_t offset)
{
	LOG("shero read_l, offset: %04x\n", offset);
	offset += 0x4100;

	if (offset == 0x4100)
		return (get_open_bus() & 0x7f) | m_jumper->read();

	return get_open_bus();
}

/*-------------------------------------------------

 SACHEN-ZGDH

 Sachen board used for Zhongguo Daheng

 NES 2.0: mapper 512

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_sachen_zgdh_device::nt_r(offs_t offset)
{
	if (m_reg == 1)
		return m_vram[0x1000 + (offset & 0x0fff)];
	else
		return device_nes_cart_interface::nt_r(offset);
}

void nes_sachen_zgdh_device::nt_w(offs_t offset, u8 data)
{
	if (m_reg == 1)
		m_vram[0x1000 + (offset & 0x0fff)] = data;
	else
		device_nes_cart_interface::nt_w(offset, data);
}

void nes_sachen_zgdh_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (m_reg <= 1)
	{
		chr = CHRROM;
		m_chr_mask = 0x7f;
	}
	else
	{
		chr = CHRRAM;
		m_chr_mask = 0x03;
	}
	nes_txrom_device::set_chr(chr, chr_base, m_chr_mask);
}

void nes_sachen_zgdh_device::write_l(offs_t offset, u8 data)
{
	LOG("zgdh write_l, offset: %04x, data: %02x\n", offset, data);

	const u16 address = offset + 0x4100;

	if ((address & 0xc100) == 0x4100)
	{
		m_reg = data & 0x03;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 UNL-A9746

 Games: Toy Story, Super 1997 4 in 1 (NT-8029)

 MMC3 clone with extended PRG-ROM and CHR-ROM
 banking registers at $8000-$8002.

 iNES: mapper 219

 In MAME: Supported.

 -------------------------------------------------*/

void nes_a9746_device::update_banks(uint8_t value)
{
	uint8_t bank = bitswap<8>(value & 0x3c, 7, 6, 0, 1, 2, 3, 4, 5);

	switch (m_reg[0])
	{
		case 0x26:
			prg8_89(bank);
			break;

		case 0x25:
			prg8_ab(bank);
			break;

		case 0x24:
			prg8_cd(bank);
			break;

		case 0x23:
			prg8_ef(bank);
			break;
	}

	switch (m_reg[1])
	{
		case 0x08:
		case 0x0a:
		case 0x0e:
		case 0x12:
		case 0x16:
		case 0x1a:
		case 0x1e:
			m_reg[2] = value << 4;
			break;

		case 0x09:
			chr1_0(m_reg[2] | ((value >> 1) & 0x0e), m_chr_source);
			break;

		case 0x0b:
			chr1_1(m_reg[2] | (value >> 1) | 0x01, m_chr_source);
			break;

		case 0x0c:
		case 0x0d:
			chr1_2(m_reg[2] | ((value >> 1) & 0x0e), m_chr_source);
			break;

		case 0x0f:
			chr1_3(m_reg[2] | (value >> 1) | 0x01, m_chr_source);
			break;

		case 0x10:
		case 0x11:
			chr1_4(m_reg[2] | ((value >> 1) & 0x0f), m_chr_source);
			break;

		case 0x14:
		case 0x15:
			chr1_5(m_reg[2] | ((value >> 1) & 0x0f), m_chr_source);
			break;

		case 0x18:
		case 0x19:
			chr1_6(m_reg[2] | ((value >> 1) & 0x0f), m_chr_source);
			break;

		case 0x1c:
		case 0x1d:
			chr1_7(m_reg[2] | ((value >> 1) & 0x0f), m_chr_source);
			break;
	}
}

void nes_a9746_device::write_h(offs_t offset, uint8_t data)
{
	LOG("unl_a9746 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset < 0x2000)
	{
		switch (offset & 0x0003)
		{
			case 0x0000:
				m_reg[0] = 0;
				m_reg[1] = data;
				break;

			case 0x0001:
				update_banks(data);
				break;

			case 0x0002:
				m_reg[0] = data;
				m_reg[1] = 0;
				break;

			case 0x0003:
				break;
		}

		return;
	}

	txrom_write(offset, data);
}

/*-------------------------------------------------

 MULTIGAME CARTS BASED ON MMC3

 -------------------------------------------------*/

/*-------------------------------------------------

 Board BMC-FK23C

 FK23C/8025 enhanced MMC3 clone used by multicarts,
 Chinese single-game cartridges and educational
 computer cartridges.

 The four outer registers are written in the
 $5000-$5FFF range. The board supports normal and
 extended MMC3 banking, direct PRG-ROM modes,
 CNROM-like CHR banking, CHR-RAM selection, banked
 WRAM and extended nametable mirroring.

 NES 2.0: mapper 176

 In MAME: Preliminary support.

 -------------------------------------------------*/

uint8_t nes_fk23c_device::fk23c_ram_r(uint8_t bank, offs_t offset) {
	std::vector<uint8_t> &ram = !m_battery.empty() ? m_battery : m_prgram;

	if (ram.empty()) {
		return get_open_bus();
	}

	return ram[((bank * 0x2000) + (offset & 0x1fff)) % ram.size()];
}

void nes_fk23c_device::fk23c_ram_w(uint8_t bank, offs_t offset, uint8_t data) {
	std::vector<uint8_t> &ram = !m_battery.empty() ? m_battery : m_prgram;

	if (ram.empty()) {
		return;
	}

	ram[((bank * 0x2000) + (offset & 0x1fff)) % ram.size()] = data;
}

void nes_fk23c_device::fk23c_set_mirror() {
	if (m_mirroring == PPU_MIRROR_4SCREEN) {
		return;
	}

	switch (m_mmc_mirror & (m_single_screen_mirroring ? 0x03 : 0x01)) {
		case 0:
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;

		case 1:
			set_nt_mirroring(PPU_MIRROR_HORZ);
			break;

		case 2:
			set_nt_mirroring(PPU_MIRROR_LOW);
			break;

		case 3:
			set_nt_mirroring(PPU_MIRROR_HIGH);
			break;
	}
}

void nes_fk23c_device::fk23c_set_prg() {
	uint16_t prg_base = m_reg[1] & 0x7f;

	prg_base |= (m_reg[0] & 0x08) << 4;
	prg_base |= (m_reg[0] & 0x80) << 1;
	prg_base |= (m_reg[2] & 0x40) << 3;

	switch (m_reg[0] & 0x07) {
		case 0:
		case 1:
		case 2: {
			const uint8_t swap = BIT(m_latch, 6) ? 2 : 0;

			if (BIT(m_reg[3], 1)) {
				const uint16_t outer = prg_base << 1;

				prg8_x(0 ^ swap, outer | m_mmc_prg_bank[0]);
				prg8_x(1, outer | m_mmc_prg_bank[1]);
				prg8_x(2 ^ swap, outer | m_mmc_prg_bank[2]);
				prg8_x(3, outer | m_mmc_prg_bank[3]);
			}
			else {
				const uint16_t mask = 0x3f >> (m_reg[0] & 0x03);
				const uint16_t outer = (prg_base << 1) & ~mask;

				prg8_x(0 ^ swap, outer | (m_mmc_prg_bank[0] & mask));
				prg8_x(1, outer | (m_mmc_prg_bank[1] & mask));
				prg8_x(2 ^ swap, outer | (0xfe & mask));
				prg8_x(3, outer | (0xff & mask));
			}
			break;
		}

		case 3:
			prg16_89ab(prg_base);
			prg16_cdef(prg_base);
			break;

		case 4:
			prg32(prg_base >> 1);
			break;

		default:
			break;
	}
}

void nes_fk23c_device::fk23c_set_chr() {
	auto map_chr = [this](int start, uint16_t bank) {
		const bool use_ram = !m_vram.empty() &&
				(!m_vrom_size || BIT(m_reg[0], 5) ||
				(m_wram_config_enabled && m_ram_in_first_chr && bank <= 7));

		chr1_x(start, bank, use_ram ? CHRRAM : CHRROM);
	};

	if (BIT(m_reg[0], 6)) {
		// $5xx0.6 selects direct 8 KiB CHR banking instead of
		// the normal MMC3 CHR registers.
		//
		// $5xx3 bits 6 or 2 enable the CNROM write latch.
		// $5xx0.4 selects the number of active latch bits:
		//
		//   $5xx0.4 = 0: two latch bits, 32 KiB region
		//   $5xx0.4 = 1: one latch bit, 16 KiB region
		//
		// When neither $5xx3 bit is set, the latch is disabled
		// and $5xx2 alone supplies the direct CHR bank.
		const uint8_t latch_mask =
				(m_reg[3] & 0x44) ?
				(BIT(m_reg[0], 4) ? 0x01 : 0x03) :
				0x00;

		const uint8_t chr_bank =
				(m_reg[2] & ~latch_mask) |
				(m_mmc_cmd1 & latch_mask);

		const uint16_t base = uint16_t(chr_bank) << 3;

		for (int start = 0; start < 8; start++) {
			map_chr(start, base | start);
		}

		return;
	}

	const uint8_t swap = BIT(m_latch, 7) ? 4 : 0;
	uint16_t outer = uint16_t(m_reg[2]) << 3;

	if (BIT(m_reg[3], 1)) {
		map_chr(0 ^ swap, outer | m_mmc_vrom_bank[0]);
		map_chr(1 ^ swap, outer | m_reg[6]);
		map_chr(2 ^ swap, outer | m_mmc_vrom_bank[1]);
		map_chr(3 ^ swap, outer | m_reg[7]);
		map_chr(4 ^ swap, outer | m_mmc_vrom_bank[2]);
		map_chr(5 ^ swap, outer | m_mmc_vrom_bank[3]);
		map_chr(6 ^ swap, outer | m_mmc_vrom_bank[4]);
		map_chr(7 ^ swap, outer | m_mmc_vrom_bank[5]);
	}
	else {
		const uint16_t mask = BIT(m_reg[0], 4) ? 0x7f : 0xff;
		outer &= ~mask;

		map_chr(0 ^ swap, outer | ((m_mmc_vrom_bank[0] & 0xfe) & mask));
		map_chr(1 ^ swap, outer | ((m_mmc_vrom_bank[0] | 0x01) & mask));
		map_chr(2 ^ swap, outer | ((m_mmc_vrom_bank[1] & 0xfe) & mask));
		map_chr(3 ^ swap, outer | ((m_mmc_vrom_bank[1] | 0x01) & mask));
		map_chr(4 ^ swap, outer | (m_mmc_vrom_bank[2] & mask));
		map_chr(5 ^ swap, outer | (m_mmc_vrom_bank[3] & mask));
		map_chr(6 ^ swap, outer | (m_mmc_vrom_bank[4] & mask));
		map_chr(7 ^ swap, outer | (m_mmc_vrom_bank[5] & mask));
	}
}

void nes_fk23c_device::fk23c_update_state() {
	fk23c_set_mirror();
	fk23c_set_prg();
	fk23c_set_chr();
}

uint8_t nes_fk23c_device::read_ex(offs_t offset) {
	if (m_wram_config_enabled) {
		return fk23c_ram_r((m_wram_bank + 1) & 0x03, offset + 0x20);
	}

	return get_open_bus();
}

uint8_t nes_fk23c_device::read_l(offs_t offset) {
	if (m_wram_config_enabled) {
		return fk23c_ram_r((m_wram_bank + 1) & 0x03, offset + 0x100);
	}

	return get_open_bus();
}

uint8_t nes_fk23c_device::read_m(offs_t offset) {
	if (m_wram_config_enabled) {
		return fk23c_ram_r(m_wram_bank, offset);
	}

	if (m_wram_enabled) {
		return fk23c_ram_r(0, offset);
	}

	return get_open_bus();
}

void nes_fk23c_device::write_ex(offs_t offset, uint8_t data) {
	if (m_wram_config_enabled) {
		fk23c_ram_w((m_wram_bank + 1) & 0x03, offset + 0x20, data);
	}
}

void nes_fk23c_device::fk23c_write_low(uint16_t address, uint8_t data) {
	if (address < 0x5000) {
		if (m_wram_config_enabled) {
			fk23c_ram_w((m_wram_bank + 1) & 0x03, address & 0x1fff, data);
		}
		return;
	}

	if (m_fk23_registers_enabled || !m_wram_config_enabled) {
		// The solder-pad setting selects which CPU address line,
		// A4 through A11, enables the four FK23C registers.
		if (BIT(address, 4 + m_fk23c_solder_pad)) {
			m_reg[address & 0x03] = data;

			if ((address & 0x03) == 2) {
				m_mmc_cmd1 = 0;
			}

			fk23c_update_state();
		}
		return;
	}

	fk23c_ram_w((m_wram_bank + 1) & 0x03, address & 0x1fff, data);
}

void nes_fk23c_device::write_l(offs_t offset, uint8_t data) {
	LOG("fk23c write_l, offset: %04x, data: %02x\n", offset, data);

	fk23c_write_low(offset + 0x4100, data);
}

void nes_fk23c_device::write_m(offs_t offset, uint8_t data) {
	LOG("fk23c write_m, offset: %04x, data: %02x\n", offset, data);

	if (m_wram_config_enabled) {
		fk23c_ram_w(m_wram_bank, offset, data);
	}
	else if (m_wram_enabled && !m_wram_write_protected) {
		fk23c_ram_w(0, offset, data);
	}
}

void nes_fk23c_device::write_h(offs_t offset, uint8_t data) {
	LOG("fk23c write_h, offset: %04x, data: %02x\n", offset, data);

	// In CNROM mode, writes in $8000-$9FFF and $C000-$FFFF
	// update the two-bit CHR latch. These writes still reach
	// the MMC3-compatible register interface.
	if (BIT(m_reg[0], 6) && (m_reg[3] & 0x44) &&
		(offset <= 0x1fff || offset >= 0x4000)) {
		m_mmc_cmd1 = data & 0x03;
		fk23c_set_chr();
	}

	switch (offset & 0x6001) {
		case 0x0000:
			// Subtype 2 swaps extended commands $46 and $47.
			if (m_prg_size == 0x1000000 && !m_vrom_size &&
				(data == 0x46 || data == 0x47)) {
				data ^= 0x01;
			}

			m_latch = data;
			fk23c_update_state();
			break;

		case 0x0001: {
			const uint8_t reg =
					m_latch & (BIT(m_reg[3], 1) ? 0x0f : 0x07);

			if (reg < 6) {
				m_mmc_vrom_bank[reg] = data;
			}
			else if (reg < 10) {
				m_mmc_prg_bank[reg - 6] = data;
			}
			else if (reg < 12) {
				m_reg[reg - 4] = data;
			}

			if (reg < 12) {
				fk23c_update_state();
			}
			break;
		}

		case 0x2000:
			m_mmc_mirror = data & 0x03;
			fk23c_set_mirror();
			break;

		case 0x2001:
			if (!BIT(data, 5)) {
				data &= 0xc0;
			}

			m_wram_bank = data & 0x03;
			m_ram_in_first_chr = BIT(data, 2);
			m_single_screen_mirroring = BIT(data, 3);
			m_wram_config_enabled = BIT(data, 5);
			m_fk23_registers_enabled = BIT(data, 6);
			m_wram_write_protected = BIT(data, 6);
			m_wram_enabled = BIT(data, 7);
			m_wram_protect = data;
			fk23c_update_state();
			break;

		case 0x4000:
		case 0x4001:
		case 0x6000:
		case 0x6001:
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 BMC-NT-639

 Unknown Bootleg Multigame Board
 Games: 2 in 1 Mortal Kombat VI/VII

 NES 2.0: mapper 291

 In MAME: Supported.

 -------------------------------------------------*/

void nes_nt639_device::write_m(offs_t offset, u8 data)
{
	LOG("nt639 write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(data, 5))
		prg32(bitswap<3>(data, 6, 2, 1));
	else
	{
		m_prg_base = (data & 0x40) >> 2;
		set_prg(m_prg_base, m_prg_mask);
	}

	m_chr_base = (data & 0x40) << 2;
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/*-------------------------------------------------

 Board BMC-SUPER24IN1SC03

 Games: Super 24-in-1

 In MAME: Supported
 
 NES 2.0: mapper 176 No Games in MAME List

 -------------------------------------------------*/

void nes_s24in1sc03_device::prg_cb(int start, int bank)
{
	static const uint8_t masks[8] = {0x3f, 0x1f, 0x0f, 0x01, 0x03, 0x00, 0x00, 0x00};
	int prg_base = m_reg[1] << 1;
	int prg_mask = masks[m_reg[0] & 0x07];

	bank = prg_base | (bank & prg_mask);
	prg8_x(start, bank);
}

void nes_s24in1sc03_device::chr_cb(int start, int bank, int source)
{
	uint8_t chr = BIT(m_reg[0], 5) ? CHRRAM : CHRROM;
	int chr_base = (m_reg[2] << 3) & 0xf00;

	chr1_x(start, chr_base | bank, chr);
}

void nes_s24in1sc03_device::write_l(offs_t offset, uint8_t data)
{
	LOG("s24in1sc03 write_l, offset: %04x, data: %02x\n", offset, data);
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

 BMC-TECHLINE9IN1

 Unknown Bootleg Multigame Board
 Games: 9 in 1

 NES 2.0: mapper 351

 In MAME: Partially Supported. No Games in MAME List

 -------------------------------------------------*/

void nes_tech9in1_device::update_banks()
{
	if (BIT(m_reg[2], 7))    // NROM mode
	{
		u8 bank = m_reg[1] >> 2;
		u8 mode = !BIT(m_reg[2], 6);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
	else                     // MMC3 mode
	{
		m_prg_base = (m_reg[1] & 0xfc) >> 1;
		m_prg_mask = 0x1f >> BIT(m_reg[2], 5);
		set_prg(m_prg_base, m_prg_mask);
	}

	m_chr_base = (m_reg[0] & 0xfc) << 1;
	m_chr_mask = BIT(m_reg[2], 7) ? 0x1f : 0xff >> BIT(m_reg[2], 5);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_tech9in1_device::write_l(offs_t offset, u8 data)
{
	LOG("tech9in1 write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	switch (offset & 0x1003)    // writes $5000-$5002, mask is a best guess
	{
		case 0x1000: case 0x1001: case 0x1002:
			m_reg[offset & 3] = data;
			update_banks();
			break;
	}
}

/*-------------------------------------------------

 BMC-5IN1

 Unknown Bootleg Multigame Board
 Games: 5 in 1 1993 Copyright

 NES 2.0: mapper 334

 In MAME: Supported. No Games in MAME List

 -------------------------------------------------*/

u8 nes_bmc_5in1_device::read_m(offs_t offset)
{
	LOG("bmc_5in1 read_m, offset: %04x\n", offset);

	if ((offset & 0x03) == 0x02)
		return (get_open_bus() & 0xfe) | m_jumper->read();
	else
		return get_open_bus();
}

void nes_bmc_5in1_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_5in1 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80)
	{
		if (!(offset & 0x03))
			prg32(BIT(data, 1, 2));
	}
}

/*-------------------------------------------------

 BMC-NEWSTAR-GRM070-8IN1

 Unknown Bootleg Multigame Board
 Games: New Star Super 8 in 1

 NES 2.0: mapper 333

 In MAME: Supported. No Games in MAME List

 -------------------------------------------------*/

void nes_bmc_8in1_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_8in1 write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 12))
	{
		if (BIT(data, 4))
		{
			m_prg_base = (data & 0x0c) << 2;
			set_prg(m_prg_base, m_prg_mask);
			m_chr_base = m_prg_base << 3;
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
		}
		else
			prg32(data & 0x0f);
	}
	else
		txrom_write(offset, data);
}

/*-------------------------------------------------

 BMC-15IN1 (PCB JC-016-2?)

 Games: 3 in 1, 15 in 1

 MMC3 clone with banking for multigame menu.

 iNES: mapper 205

 In MAME: Supported. No Games in MAME List

 -------------------------------------------------*/

void nes_bmc_15in1_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_15in1 write_m, offset: %04x, data: %02x\n", offset, data);

	if (data & 1)
		data |= m_jumper;    // TODO: add jumper settings, m_jumper is 0 for now

	m_prg_base = (data & 0x03) << 4;
	m_prg_mask = 0x1f >> BIT(data, 1);
	set_prg(m_prg_base, m_prg_mask);

	m_chr_base = m_prg_base << 3;
	m_chr_mask = 0xff >> BIT(data, 1);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/*-------------------------------------------------

 BMC-SUPERBIG-7IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Kunio 8 in 1, Super Big 7 in 1

 iNES: mapper 44

 In MAME: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

void nes_bmc_sbig7_device::write_h(offs_t offset, uint8_t data) {
	LOG("bmc_sbig7 write_h, offset: %04x, data: %02x\n", offset, data);

	if ((offset & 0x6001) == 0x2001) {
		// $A001 bits 7-6 retain the normal MMC3 WRAM controls.
		txrom_write(offset, data);

		// Bits 2-0 select blocks 0-6. Block 7 mirrors block 6.
		const uint8_t page = std::min<uint8_t>(data & 0x07, 6);

		m_prg_base = page << 4;
		m_prg_mask = page == 6 ? 0x1f : 0x0f;

		m_chr_base = page << 7;
		m_chr_mask = page == 6 ? 0xff : 0x7f;

		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
		return;
	}

	txrom_write(offset, data);
}

/*-------------------------------------------------

 BMC-HIK8IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Many, many multigame carts

 iNES: mapper 45

 In MAME: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

void nes_bmc_hik8_device::write_m(offs_t offset, u8 data) {
	LOG("bmc_hik8 write_m, offset: %04x, data: %02x\n", offset, data);

	nes_txrom_device::write_m(offset, data);

	if (!BIT(m_reg[3], 6)) {
		m_reg[m_count] = data;
		m_count = (m_count + 1) & 0x03;

		m_prg_base = ((m_reg[2] & 0xc0) << 2) | m_reg[1];
		m_prg_mask = ~m_reg[3] & 0x3f;
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = ((m_reg[2] & 0xf0) << 4) | m_reg[0];
		m_chr_mask = 0xff >> (~m_reg[2] & 0x0f);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-JY-208

 Games: Rockman 7 in 1

 Variant of the HIK8IN1 boards that adds 8K of VRAM
 to be used presumably by Rockman 4/6 and a 4 screen
 mirroring mode for Sachen's Rocman X.

 NES 2.0: mapper 356

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_jy208_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (BIT(m_reg[2], 5))
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
	else
		chr8(0, CHRRAM);
}

void nes_bmc_jy208_device::write_m(offs_t offset, u8 data)
{
	nes_bmc_hik8_device::write_m(offset, data);

	if (BIT(m_reg[2], 6))
		m_mirroring = PPU_MIRROR_4SCREEN;  // prevent MMC3 mirror switching
	else
		m_mirroring = BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT;  // allow MMC3 mirror switching and get its current setting

	set_nt_mirroring(m_mirroring);  // actually change nametable pointers
}

/*-------------------------------------------------

 BMC-JY-302

 Games: Super 8 in 1

 Variant of the HIK8IN1 boards that adds 8K of VRAM
 which seems the same as the SFC-12 board below,
 other than the line which selects VRAM/VROM.

 NES 2.0: mapper 410

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_jy302_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (BIT(m_reg[2], 6))
		chr8(0, CHRRAM);
	else
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
}

/*-------------------------------------------------

 BMC-KC885

 Games: Super 8 in 1 VIP19

 Variant of the HIK8IN1 boards that has different
 upper address lines for PRG banking. Exactly how
 these lines are selected is determined by three
 jumper/solder pads.

 NES 2.0: mapper 401

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_kc885_device::read_h(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "bmc_kc885 read_h, offset: %04x\n", offset);

	if (BIT(m_reg[1], 7) & BIT(m_jumper->read(), 2))
		return get_open_bus();
	else
		return hi_access_rom(offset);
}

void nes_bmc_kc885_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_kc885 write_m, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_reg[3], 6))  // outer register lock bit
	{
		if (m_count == 1)
			data = (data & ~0x60) | bitswap<2>(data, 5, 6) << 5;
		m_reg[m_count] = data;
		m_count = (m_count + 1) & 0x03;

		u8 mask = 0x80 | (m_jumper->read() & 0x03) << 5;
		m_prg_base = (m_reg[2] & mask) | (m_reg[1] & ~mask);
		m_prg_mask = ~m_reg[3] & 0x1f;
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = (m_reg[2] & 0xf0) << 4 | m_reg[0];
		m_chr_mask = 0xff >> (~m_reg[2] & 0x0f);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-SFC-12

 Games: Rockman 6 in 1 (alt 2)

 Variant of the HIK8IN1 boards that adds 8K of VRAM
 to be used presumably by Rockman 4/6.

 NES 2.0: mapper 372

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_sfc12_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (BIT(m_reg[2], 5))
		chr8(0, CHRRAM);
	else
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
}

/*-------------------------------------------------

 BMC-SUPERHIK-4IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Super HIK 4 in 1

 iNES: mapper 49

 In MAME: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

void nes_bmc_hik4_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)    // all games use MMC3 PRG banking except Master Fighter III
		nes_txrom_device::prg_cb(start, bank);
}

void nes_bmc_hik4_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_hik4 write_m, offset: %04x, data: %02x\n", offset, data);

	// mid writes only work when WRAM is enabled and writable
	if ((m_wram_protect & 0xc0) == 0x80)
	{
		m_mmc3_mode = data & 0x01;
		if (m_mmc3_mode)
		{
			m_prg_base = (data & 0xc0) >> 2;
			set_prg(m_prg_base, m_prg_mask);
		}
		else                // Master Fighter III only
			prg32(BIT(data, 4, 2));

		m_chr_base = (data & 0xc0) << 1;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-A88S-1

 Games: 1997 Super 7 in 1, 6 in 1 (JY-201 to JY-206)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 411

 In MAME: Preliminary supported.

 TODO: Investigate...
 - Hokuto no Ken has broken sound on each cart it's on.
 - Bird Fighter boss fights have corrupt graphics.
 - whether not being able to reset from within certain
   games is correct. Examples are Samurai (title scroll
   only) and Bomber Man (in game only) on JY-202, or
   SD Gunman in JY-204.

 -------------------------------------------------*/

void nes_a88s1_device::update_banks()
{
	u8 outer = bitswap<2>(m_reg[1], 6, 3);
	u8 size_128k = !BIT(m_reg[1], 1);

	if (BIT(m_reg[0], 6))    // NROM mode
	{
		u8 bank = outer << 3 | bitswap<3>(m_reg[0], 2, 3, 0);
		u8 mode = BIT(m_reg[0], 1);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
	else                     // MMC3 mode
	{
		m_prg_base = (outer & (0xfe | size_128k)) << 4;
		m_prg_mask = 0x1f >> size_128k;
		set_prg(m_prg_base, m_prg_mask);
	}

	m_chr_base = (m_reg[0] & 0x10) << 4 | (BIT(m_reg[1], 2) & size_128k) << 7;
	m_chr_mask = 0xff >> size_128k;
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_a88s1_device::prg_cb(int start, int bank)
{
	if (!BIT(m_reg[0], 6))
		nes_txrom_device::prg_cb(start, bank);
}

void nes_a88s1_device::write_l(offs_t offset, u8 data)
{
	LOG("a88s1 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000)
	{
		m_reg[offset & 1] = data;
		update_banks();
	}
}

/*-------------------------------------------------

 BMC-F-15

 Unknown Multigame Bootleg Board
 Games: 150 in 1 Unchained Melody

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 259

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_f15_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_f15 write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(m_wram_protect, 7))
	{
		u8 bank = data & 0x0f;
		u8 mode = BIT(data, 3);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
}

void nes_bmc_f15_device::prg_cb(int start, int bank)
{
	// Ignore MMC3 PRG bank switching. Master Fighter II (game #150) uses the bank switching above.
}

/*-------------------------------------------------

 BMC-F600

 Games: Golden Mario Party II 6 in 1

 MMC3 clone with banking for multigame menu. Note,
 this cart has one TxSROM game (SMB4) and so it uses
 the nonstandard MMC3 mirroring of those boards.

 NES 2.0: mapper 370

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_f600_device::read_l(offs_t offset)
{
	LOG("bmc_f600 read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (offset >= 0x1000)
		return (get_open_bus() & 0x7f) | m_jumper->read();
	else
		return get_open_bus();
}

void nes_bmc_f600_device::write_l(offs_t offset, u8 data)
{
	LOG("bmc_f600 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000)
	{
		m_reg = offset;

		m_prg_base = (m_reg & 0x38) << 1;
		m_prg_mask = 0x1f >> BIT(m_reg, 5);
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = (m_reg & 0x07) << 7;
		m_chr_mask = 0xff >> !BIT(m_reg, 2);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

void nes_bmc_f600_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_f600 write_h, offset: %04x, data: %02x\n", offset, data);
	if ((m_reg & 0x07) == 1)
		nes_txsrom_device::write_h(offset, data);
	else
		nes_txrom_device::write_h(offset, data);
}

void nes_bmc_f600_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if ((m_reg & 0x07) == 1)
		nes_txsrom_device::set_chr(chr, chr_base, chr_mask);
	else
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
}

/*-------------------------------------------------

 BMC EL860947C and EL861121C boards

 Games: 8 in 1 (JY-111, JY-112, and JY-119)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 267 and mapper 377

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_el86xc_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_el86xc write_m, offset: %04x, data: %02x\n", offset, data);
	if (!(offset & 0x1000))    // game only banks via 0x6000, this mask is a guess
	{
		int outer = bitswap<3>(data, 5, 2, 1);
		m_prg_base = outer << (std::countr_zero(m_outer_prg_size) - 3);
		set_prg(m_prg_base, m_prg_mask);
		m_chr_base = outer << 7;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-GN-45

 Games: 4 in 1 (K-3131GS and K-3131SS)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 366

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_gn45_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_gn45 write_m, offset: %04x, data: %02x\n", offset, data);

	nes_txrom_device::write_m(offset, data);  // registers overlay WRAM

	if (!m_lock)
	{
		m_prg_base = offset & 0x70;
		set_prg(m_prg_base, m_prg_mask);
		m_chr_base = m_prg_base << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		m_lock = BIT(offset, 7);
	}
}

/*-------------------------------------------------

 BMC-GOLD-7IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Super HIK Gold 7 in 1, Golden 7 in 1 and many more

 MMC3 clone with banking for multigame menu.

 iNES: mapper 52

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_gold7in1_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_gold7in1 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80 && !m_lock)
	{
		u8 prg128k = BIT(data, 3);
		u8 chr128k = BIT(data, 6);

		m_prg_base = (data & (0x06 | prg128k)) << 4;
		m_prg_mask = 0x1f >> prg128k;
		m_chr_base = (bitswap<3>(data, 2, 5, 4) & (0x06 | chr128k)) << 7;
		m_chr_mask = 0xff >> chr128k;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		m_lock = BIT(data, 7); // mc_2hikg & mc_s3nt3 write here multiple time
	}
	else
		nes_txrom_device::write_m(offset, data);  // write WRAM
}

/*-------------------------------------------------

 BMC-K-3006

 Games: 21 in 1

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 339

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_k3006_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_k3006 write_m, offset: %04x, data: %02x\n", offset, data);
	if ((m_wram_protect & 0xc0) == 0x80)
	{
		if (BIT(offset, 5))    // MMC3 mode
		{
			m_prg_base = (offset & 0x18) << 1;
			set_prg(m_prg_base, m_prg_mask);
			m_chr_base = m_prg_base << 3;
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
		}
		else                   // NROM mode
		{
			u8 bank = offset & 0x1f;
			u8 mode = (offset & 0x06) == 0x06;
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
	}
}

/*-------------------------------------------------

 BMC-K-3033

 Games: 35 in 1

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 322

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_k3033_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)
		nes_txrom_device::prg_cb(start, bank);
}

void nes_bmc_k3033_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_k3033 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80)
	{
		m_mmc3_mode = BIT(offset, 5);

		u8 mode_128k = !(BIT(offset, 7) && m_mmc3_mode);
		m_chr_base = bitswap<3>(offset, 6, 4, 3) << (8 - mode_128k);
		m_chr_mask = 0xff >> mode_128k;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		if (m_mmc3_mode)
		{
			m_prg_base = m_chr_base >> 3;
			m_prg_mask = m_chr_mask >> 3;
			set_prg(m_prg_base, m_prg_mask);
		}
		else                // NROM mode
		{
			u8 bank = (offset & 0x40) >> 1 | (offset & 0x1f);
			u8 mode = !!(offset & 0x03);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
	}
}

/*-------------------------------------------------

 BMC-L6IN1

 Games: New Star 6 in 1

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 345

 Note: Cabal leaves the APU in a bad state so that
 after soft resetting and loading Track & Field there
 is a constant drone pitch. This is likely a game bug.

 -------------------------------------------------*/

void nes_bmc_l6in1_device::set_prg(int prg_base, int prg_mask)
{
	if (m_reg & 0x0c)    // MMC3 mode
		nes_txrom_device::set_prg(prg_base, prg_mask);
	else                // AxROM mode
		prg32(prg_base >> 2 | (m_reg & 0x03));
}

void nes_bmc_l6in1_device::set_mirror()
{
	if (BIT(m_reg, 5))
		set_nt_mirroring(BIT(m_reg, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	else
		set_nt_mirroring(m_mmc_mirror ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_bmc_l6in1_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_l6in1 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80)
	{
		m_reg = data;
		m_prg_base = (m_reg & 0xc0) >> 2;
		set_prg(m_prg_base, m_prg_mask);
		set_mirror();
	}
}

void nes_bmc_l6in1_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_l6in1 write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write(offset, data);
	if ((offset & 0x6001) == 0x2000)
		set_mirror();
}

/*-------------------------------------------------

 BMC-00202650

 Games: 8 in 1

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 392

 In MAME: Supported.

 TODO: Soft reset doesn't work for some games some of
 the time. Seems to be caused by main RAM contents.
 Does this happen on hardware?

 -------------------------------------------------*/

void nes_bmc_00202650_device::set_prg(int prg_base, int prg_mask)
{
	if (m_mmc3_mode)
		nes_txrom_device::set_prg(prg_base, prg_mask);
}

void nes_bmc_00202650_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (m_mmc3_mode)
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
}

void nes_bmc_00202650_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_00202650 write_m, offset: %04x, data: %02x\n", offset, data);

	if (!m_mmc3_mode)
	{
		m_mmc3_mode = BIT(data, 4);

		m_prg_base = (data & 0x07) << 4;
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = m_prg_base << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-411120C, BMC-810849-C

 Games: 4 in 1, 19 in 1

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 287

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_411120c_device::prg_cb(int start, int bank)
{
	if (!BIT(m_reg, 3))    // all games use MMC3 PRG banking except Master Fighter II
		nes_txrom_device::prg_cb(start, bank);
}

void nes_bmc_411120c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_411120c write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(m_wram_protect, 7))
	{
		m_reg = offset;
		if (BIT(m_reg, 3))
			prg32(bitswap<5>(m_reg, 2, 1, 0, 5, 4));
		else
		{
			m_prg_base = (m_reg & 0x07) << 4;
			set_prg(m_prg_base, m_prg_mask);
		}
		m_chr_base = (m_reg & 0x07) << 7;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-81-03-05-C

 Games: 92' Super Mario Family

 MMC3 clone with banking for multigame menu. Note,
 this cart has one TxSROM game (SMB4) and so it uses
 the nonstandard MMC3 mirroring of those boards.

 NES 2.0: mapper 353

 In MAME: Partially supported.

 TODO: Banking isn't quite right. Golden Mario, the
 4th game, should be SMB2J but loads as plain old
 Mario Bros with garbled graphics. Golden Mario
 should be on the last outer bank #3.

 -------------------------------------------------*/

void nes_bmc_810305c_device::set_prg(int prg_base, int prg_mask)
{
	const bool chr_a17 = BIT(m_mmc_vrom_bank[0], 7);

	const u8 exp2 = (m_outer == 3 && !chr_a17)
		? ((m_mmc_prg_bank[0] & 0x0f) | 0x10)
		: 0xfe;

	const u8 exp3 = (m_outer == 3 && !chr_a17)
		? ((m_mmc_prg_bank[1] & 0x0f) | 0x10)
		: 0xff;

	u8 mask = 0x1f;
	u8 block = m_outer << 5;

	if (m_outer == 2)
	{
		mask = 0x0f;
		block = (chr_a17 ? 0x10 : 0x00) | (m_outer << 5);
	}

	logerror("M353 set_prg outer=%d chr_a17=%d latch=%02X prg0=%03X prg1=%03X "
		"mask=%02X block=%02X exp2=%02X exp3=%02X mode=%d\n",
		m_outer,
		chr_a17 ? 1 : 0,
		m_latch,
		m_mmc_prg_bank[0],
		m_mmc_prg_bank[1],
		mask,
		block,
		exp2,
		exp3,
		BIT(m_latch, 6));

	if (BIT(m_latch, 6))
	{
		prg8_89((exp2 & mask) | block);
		prg8_ab((m_mmc_prg_bank[1] & mask) | block);
		prg8_cd((m_mmc_prg_bank[0] & mask) | block);
		prg8_ef((exp3 & mask) | block);
	}
	else
	{
		prg8_89((m_mmc_prg_bank[0] & mask) | block);
		prg8_ab((m_mmc_prg_bank[1] & mask) | block);
		prg8_cd((exp2 & mask) | block);
		prg8_ef((exp3 & mask) | block);
	}
}

void nes_bmc_810305c_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	logerror("M353 set_chr outer=%d chr_a17=%d chr=%02X base=%03X mask=%03X "
		"vrom0=%03X vrom1=%03X\n",
		m_outer,
		BIT(m_mmc_vrom_bank[0], 7),
		chr,
		chr_base,
		chr_mask,
		m_mmc_vrom_bank[0],
		m_mmc_vrom_bank[1]);

	if (m_outer == 0)
	{
		// Outer 0 is TXSROM / mapper 118 style.
		// Do NOT force TKSPPU behavior here; MAME chr_cb is a bank-install
		// callback, not a PPU access callback.
		nes_txsrom_device::set_chr(chr, chr_base, chr_mask);
	}
	else if (m_outer == 2 && BIT(m_mmc_vrom_bank[0], 7))
	{
		// Mapper 353 outer 2 + CHR A17 high = 8 KiB CHR RAM.
		chr8(0, CHRRAM);
	}
	else
	{
		// FCEU equivalent:
		// setchr1(A, (V & 0x7F) | (outer << 7));
		nes_txrom_device::set_chr(chr, m_outer << 7, 0x7f);
	}
}

void nes_bmc_810305c_device::chr_cb(int start, int bank, int source)
{
	if (m_outer == 0)
	{
		// Let TXSROM handle the mapper-118 style mirroring/banking.
		// This avoids applying FCEU's TKSPPU mirroring at the wrong time.
		nes_txsrom_device::chr_cb(start, bank, source);
		return;
	}

	const u8 page = start & 0x07;
	m_ppu_chr_bus = page;

	const int mapped_bank = (bank & 0x7f) | (m_outer << 7);

	logerror("M353 chr_cb start=%04X page=%d bank=%03X mapped=%03X source=%d "
		"outer=%d\n",
		start,
		page,
		bank,
		mapped_bank,
		source,
		m_outer);

	nes_txrom_device::chr_cb(start, mapped_bank, source);
}

void nes_bmc_810305c_device::write_h(offs_t offset, u8 data)
{
	logerror("M353 write_h offset=%04X data=%02X\n", offset, data);

	// FCEU M353Write:
	// if (A & 0x80) {
	//   EXPREGS[0] = (A >> 13) & 0x03;
	//   FixMMC3PRG(MMC3_cmd);
	//   FixMMC3CHR(MMC3_cmd);
	// }
	//
	// MAME offset is CPU address - $8000, so offset bit 7 is CPU A7.
	if (BIT(offset, 7))
	{
		m_outer = BIT(offset, 13, 2);

		logerror("M353 OUTER apply offset=%04X data=%02X outer=%d\n",
			offset,
			data,
			m_outer);

		m_prg_base = m_outer << 5;
		m_prg_mask = (m_outer == 2) ? 0x0f : 0x1f;

		m_chr_base = m_outer << 7;
		m_chr_mask = 0x7f;

		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else if (offset < 0x4000)
	{
		// FCEU uses MMC3_CMDWrite for A < $C000.
		// Let TXROM handle normal MMC3 bank select / bank data behavior.
		nes_txrom_device::write_h(offset, data);

		// FCEU calls FixMMC3PRG after MMC3 command writes.
		// Also reapply CHR because CHR callback/mirroring depends on CHR regs.
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else
	{
		// IRQ / A12 / MMC3 control area.
		nes_txrom_device::write_h(offset, data);
	}
}

/*-------------------------------------------------

 BMC-820720C

 Games: 1993 Super HiK 8 in 1 (G-002)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 393

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_820720c_device::set_prg(int prg_base, int prg_mask)
{
	switch (m_reg >> 4)
	{
		case 0:            // MMC3 mode
		case 1:
			nes_txrom_device::set_prg(prg_base, prg_mask);
			break;
		case 2:            // BNROM mode
			prg32((prg_base | (m_mmc_prg_bank[BIT(m_latch, 6) << 1] & prg_mask)) >> 2);
			break;
		case 3:            // UNROM mode
			prg16_89ab(m_prg_base >> 1);
			prg16_cdef(m_prg_base >> 1 | 0x07);
			break;
	}
}

void nes_bmc_820720c_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (BIT(m_reg, 3))
		chr8(0, CHRRAM);
	else
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
}

void nes_bmc_820720c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_820720c write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80)
	{
		m_reg = offset & 0x3f;

		m_prg_base = (m_reg & 0x07) << 4;
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = (m_reg & 0x01) << 8;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

void nes_bmc_820720c_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_820720c write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write(offset, data);     // MMC3 regs always written (for mirroring in non-MMC3 modes)

	if ((m_reg & 0x30) == 0x30)    // UNROM mode
	{
		prg16_89ab(m_prg_base >> 1 | (data & 0x07));
		prg16_cdef(m_prg_base >> 1 | 0x07);
	}
}

/*-------------------------------------------------

 BMC-830118C

 Games: 7 in 1 (EW-002, M-022, M-026, M-027)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 348

 In MAME: Preliminary supported.

 TODO: Only M-022 seems to reliably soft reset to the
 menu for all games. The other three carts are hit or
 miss. MMC3 games in particular are not likely to
 reset to the menu. Investigate further...

 -------------------------------------------------*/

void nes_bmc_830118c_device::prg_cb(int start, int bank)
{
	if (m_prg_base == 0x30)
	{
		if (start <= 1)
		{
			prg8_x(start, bank & ~2);
			prg8_x(start + 2, bank | 2);
		}
	}
	else
		nes_txrom_device::prg_cb(start, bank);
}

void nes_bmc_830118c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_830118c write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80)
	{
		m_prg_base = (data & 0x0c) << 2;
		set_prg(m_prg_base, m_prg_mask);
		m_chr_base = m_prg_base << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-830832C

 Games: 1994 Super HiK 3 in 1 (JY-007)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 364

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_830832c_device::write_m(offs_t offset, u8 data) {
	LOG("bmc_830832c write_m, offset: %04x, data: %02x\n", offset, data);

	// $6000-$7FFF, mask $E000.
	//
	// D6: PRG A18
	// D5: 0 preserves MMC3 PRG/CHR A17
	//     1 forces PRG/CHR A17 to zero
	// D4: CHR A18
	m_prg_base = (data & 0x40) >> 1;
	m_prg_mask = 0x1f >> BIT(data, 5);
	set_prg(m_prg_base, m_prg_mask);

	m_chr_base = (data & 0x10) << 4;
	m_chr_mask = 0xff >> BIT(data, 5);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/*-------------------------------------------------

 BMC-YY841101C

 Games: 4 in 1 (OK-411, JY-009, JY-018, JY-019, JY-020)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 361

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_yy841101c_device::write_m(offs_t offset, u8 data) {
	LOG("bmc_yy841101c write_m, offset: %04x, data: %02x\n", offset, data);

	// The confirmed register is $7000. Its complete address mask
	// remains unknown, so currently decode $7000-$7FFF.
	if (offset & 0x1000) {
		m_prg_base = data & 0xf0;
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = (data & 0xf0) << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-YY841155C

 Games: Donkey Kong 8 in 1 (JY-041)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 376

 In MAME: Supported.

 -------------------------------------------------*/
void nes_bmc_yy841155c_device::prg_cb(int start, int bank) {
	// In NROM mode, MMC3 PRG outputs are disconnected.
	if (!BIT(m_reg[0], 7))
		nes_txrom_device::prg_cb(start, bank);
}

void nes_bmc_yy841155c_device::write_m(offs_t offset, u8 data) {
	LOG("bmc_yy841155c write_m, offset: %04x, data: %02x\n", offset, data);

	// $7000/$7001, mirrored through $7000-$7FFF.
	if (offset >= 0x1000) {
		m_reg[offset & 1] = data;

		m_prg_base =
				((m_reg[1] & 0x01) << 5) |
				((m_reg[0] & 0x40) >> 2);

		if (BIT(m_reg[0], 7)) {
			// NROM mode:
			// bit 5 selects NROM-128 or NROM-256.
			const u8 bank =
					(m_prg_base >> 1) |
					(m_reg[0] & 0x07);
			const u8 mode = BIT(m_reg[0], 5);

			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
		else {
			set_prg(m_prg_base, m_prg_mask);
		}

		m_chr_base = m_prg_base << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-POWERJOY

 Game: PowerJoy 84 in 1 (PJ-008)

 MMC3 clone with four outer banking registers
 throughout $6000-$7FFF.

 iNES: mapper 126

 In MAME: Supported.

 -------------------------------------------------*/

void nes_pjoy84_device::prg_cb(int start, int bank)
{
	uint8_t flip = (m_latch & 0x40) ? 2 : 0;

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

inline void nes_pjoy84_device::set_base_mask() {
	m_prg_mask = BIT(m_reg[0], 6) ? 0x0f : 0x1f;
	m_prg_base =
			((m_reg[0] & 0x30) << 3) |
			((m_reg[0] & 0x07) << 4);
	m_prg_base &= ~m_prg_mask;

	// Mapper 126 CHR outer wiring:
	// R0.3 -> CHR A17
	// R0.5 -> CHR A18
	// R0.4 -> CHR A19
	m_chr_mask = BIT(m_reg[0], 7) ? 0x7f : 0xff;
	m_chr_base =
			((m_reg[0] & 0x08) << 4) |
			((m_reg[0] & 0x20) << 3) |
			((m_reg[0] & 0x10) << 5);
	m_chr_base &= ~m_chr_mask;
}

void nes_pjoy84_device::write_m(offs_t offset, uint8_t data) {
	LOG("pjoy84 write_m, offset: %04x, data: %02x\n", offset, data);

	const uint8_t reg = offset & 0x03;

	if (!BIT(m_reg[3], 7)) {
		m_reg[reg] = data;
	}
	else if (reg == 2) {
		// After locking, only the low CHR banking bits of
		// register 2 remain writable.
		const uint8_t mask = 0x03 >> BIT(m_reg[2], 4);
		m_reg[2] = (m_reg[2] & ~mask) | (data & mask);
	}
	else {
		return;
	}

	set_base_mask();

	if (BIT(m_reg[3], 4)) {
		chr8(
				(m_chr_base >> 3) |
				(m_reg[2] & (m_chr_mask >> 3)),
				m_chr_source);
	}
	else {
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}

	set_prg(m_prg_base, m_prg_mask);
}

/*-------------------------------------------------

 COOLBOY, MINDKIDS, and others with SMD132/133 chips

 Games: many multicarts, Chinese games, and some
 modern re-releases such as Metal Storm and Holy Diver

 NES 2.0: mapper 268

 In MAME: Partially supported.

 TODO: There are many unimplemented features, though
 there may be no carts that use them. Some of the
 unemulated stuff: regs 4 and 5 not used, more GNROM
 bits in reg 2, "weird" modes, WRAM at 0x5000, etc.

 -------------------------------------------------*/

void nes_smd133_device::prg_cb(int start, int bank)
{
	if (BIT(m_reg[3], 4))    // GNROM mode
	{
		u8 mask = (m_reg[1] & 0x02) | 0x01;
		bank &= ~0x0f;
		bank |= (m_reg[3] & ~mask & 0x0f) | (start & mask);
	}

	prg8_x(start, bank);
}

void nes_smd133_device::chr_cb(int start, int bank, int source)
{
	if (BIT(m_reg[3], 4))    // GNROM mode
	{
		bank &= ~m_chr_mask;
		bank |= (m_reg[2] & 0x0f) << 3 | start;
	}

	chr1_x(start, bank, source);
}

void nes_smd133_device::smd133_write(offs_t offset, u8 data) {
	const u8 reg = offset & 0x07;

	if (reg >= 6) {
		return;
	}

	// Lockout only applies in MMC3 mode. Register 2 remains
	// partially writable after lock.
	if (!BIT(m_reg[3], 4) && BIT(m_reg[3], 7) && reg != 2) {
		return;
	}

	if (reg == 2) {
		// Bit 7 locks the upper nibble.
		if (BIT(m_reg[2], 7)) {
			data = (data & 0x0f) | (m_reg[2] & 0xf0);
		}

		// Bits 6-4 mask subsequent updates to the corresponding
		// GNROM CHR bank bits.
		data &= ((~m_reg[2] & 0x70) >> 3) | 0xf1;
	}

	m_reg[reg] = data;

	m_prg_base =
			((m_reg[0] & 0x30) << 6) |
			((m_reg[1] & 0x0c) << 6) |
			((m_reg[1] & 0x10) << 3) |
			((m_reg[0] & 0x07) << 4);

	m_prg_mask =
			((bitswap<3>(m_reg[1], 5, 6, 7) << 5) |
			((m_reg[0] & 0x40) >> 2) |
			0x0f) ^ 0x30;

	set_prg(m_prg_base, m_prg_mask);

	m_chr_mask = BIT(m_reg[0], 7) ? 0x7f : 0xff;

	m_chr_base =
			((m_reg[0] & 0x08) << 4) |
			((m_reg[0] & 0x30) << 4);
	m_chr_base &= ~m_chr_mask;

	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_smd133_device::write_l(offs_t offset, u8 data)
{
	LOG("smd133 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000 && m_smd133_addr == 0x5000)
		smd133_write(offset, data);
}

void nes_smd133_device::write_m(offs_t offset, u8 data)
{
	LOG("smd133 write_m, offset: %04x, data: %02x\n", offset, data);

	nes_txrom_device::write_m(offset, data);  // registers overlay WRAM

	if (offset < 0x1000 && m_smd133_addr == 0x6000)
		smd133_write(offset, data);
}
