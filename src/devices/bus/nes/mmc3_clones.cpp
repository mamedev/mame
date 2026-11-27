// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for MMC-3 clone PCBs

 iNES mappers: 44, 45, 49, 52, 114, 115, 121, 123, 126, 134, 182, 187, 189, 196, 197, 205, 208, 215,
               219, 238, 250, 254, 258, 259, 260, 262, 263, 268, 269, 287, 291, 292, 313, 322, 325, 333,
               334, 339, 345, 348, 351, 353, 356, 359, 361, 364, 366, 369, 370, 372, 376, 377, 392, 393,
               401, 410, 411, 412, 422, 432, 512, 513, 516, 523, 534, 594


 Here we emulate several pirate PCBs based on MMC-3 boards

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc3_clones.h"

#include <bit>
#include <cassert>

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
DEFINE_DEVICE_TYPE(NES_HENGG_412,     nes_hengg412_device,      "nes_hengg412",      "NES Cart Henggedianzi 412 PCB")
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
DEFINE_DEVICE_TYPE(NES_PJOY84,        nes_pjoy84_device,        "nes_pjoy84",        "NES Cart TEC9719 / PowerJoy 84 PCB")
DEFINE_DEVICE_TYPE(NES_BS300,         nes_bs300_device,         "nes_bs300",         "NES Cart BS-300 Family PCB")
DEFINE_DEVICE_TYPE(NES_ING022,        nes_ing022_device,        "nes_ing022",        "NES Cart ING-022 PCB")
DEFINE_DEVICE_TYPE(NES_8025,          nes_8025_device,          "nes_8025",          "NES Cart 8025 Enhanced MMC3 PCB")
DEFINE_DEVICE_TYPE(NES_SMD133,        nes_smd133_device,        "nes_smd133",        "NES Cart SMD133 PCB")
DEFINE_DEVICE_TYPE(NES_RINCO_FSG2,    nes_rinco_fsg2_device,    "nes_rinco_fsg2",    "NES Cart Rinco Talkman FSG2 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_HPXX,      nes_bmc_hpxx_device,      "nes_bmc_hpxx",      "NES Cart BMC HPxx PCB")
DEFINE_DEVICE_TYPE(NES_BMC_620,       nes_bmc_620_device,       "nes_bmc_620",       "NES Cart 1993 Super Fighter 22-in-1 (NES 2.0 mapper 620) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_N49C300,   nes_bmc_n49c300_device,   "nes_bmc_n49c300",   "NES Cart BMC N49C-300 PCB")
DEFINE_DEVICE_TYPE(NES_REALTEC432,    nes_realtec432_device,    "nes_realtec432",    "NES Cart Realtec 8090 PCB")
DEFINE_DEVICE_TYPE(NES_JNCOTA_FS005,  nes_jncota_fs005_device,  "nes_jncota_fs005",  "NES Cart Jncota FS005 (2 KiB CHR) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GX121,     nes_bmc_gx121_device,     "nes_bmc_gx121",     "NES Cart Games Xplosion 121-in-1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_SB5013,    nes_bmc_sb5013_device,    "nes_bmc_sb5013",    "NES Cart BMC-SB-5013 PCB")
DEFINE_DEVICE_TYPE(NES_UNL_82112C,    nes_unl_82112c_device,    "nes_unl_82112c",    "NES Cart UNL-82112C PCB")


INPUT_PORTS_START( bmc_8025 )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x07, 0x00, "8025 Register Address (Solder Pad)" )
	PORT_CONFSETTING(    0x00, "$5013" )
	PORT_CONFSETTING(    0x01, "$5023" )
	PORT_CONFSETTING(    0x02, "$5043" )
	PORT_CONFSETTING(    0x03, "$5083" )
	PORT_CONFSETTING(    0x04, "$5103" )
	PORT_CONFSETTING(    0x05, "$5203" )
	PORT_CONFSETTING(    0x06, "$5403" )
	PORT_CONFSETTING(    0x07, "$5803" )
INPUT_PORTS_END

INPUT_PORTS_START( kasing )
	PORT_START("PAD")
	PORT_CONFNAME( 0x07, 0x00, "Solder Pad Value ($6002)" )
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "3" )
	PORT_CONFSETTING(    0x04, "4" )
	PORT_CONFSETTING(    0x05, "5" )
	PORT_CONFSETTING(    0x06, "6" )
	PORT_CONFSETTING(    0x07, "7" )
INPUT_PORTS_END

INPUT_PORTS_START( family4646 )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "T4A54A solder pad" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x01, "High" )
INPUT_PORTS_END

INPUT_PORTS_START( sachen_shero )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x80, 0x00, "Title Screen" )
	PORT_CONFSETTING(    0x00, "Street Heroes" )
	PORT_CONFSETTING(    0x80, u8"\u4f8d\u9b42 (Shìhún)" )    // 侍魂
INPUT_PORTS_END

INPUT_PORTS_START( bmc_15in1 )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x02, 0x00, "Solder Pad" )
	PORT_CONFSETTING(    0x00, "15 in 1 menu" )
	PORT_CONFSETTING(    0x02, "3 in 1 menu" )    // forces block 1 to block 3, NESdev iNES mapper 205
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

INPUT_PORTS_START( bmc_hik8 )
	PORT_START("DIP")
	PORT_CONFNAME( 0x0f, 0x00, "DIP Switch ($5000-$5FFF reads)" )
	PORT_CONFSETTING(    0x00, "Not fitted" )
	PORT_CONFSETTING(    0x01, "Position 0" )
	PORT_CONFSETTING(    0x02, "Position 1" )
	PORT_CONFSETTING(    0x03, "Position 2" )
	PORT_CONFSETTING(    0x04, "Position 3" )
	PORT_CONFSETTING(    0x05, "Position 4" )
	PORT_CONFSETTING(    0x06, "Position 5" )
	PORT_CONFSETTING(    0x07, "Position 6" )
	PORT_CONFSETTING(    0x08, "Position 7" )

	PORT_START("MENU")
	PORT_CONFNAME( 0x07, 0x00, "Menu Selection (address line connected to PRG /CE)" )
	PORT_CONFSETTING(    0x00, "None" )
	PORT_CONFSETTING(    0x01, "PRG A20" )
	PORT_CONFSETTING(    0x02, "CHR A20" )
	PORT_CONFSETTING(    0x03, "PRG A19" )
	PORT_CONFSETTING(    0x04, "CHR A19" )
INPUT_PORTS_END

INPUT_PORTS_START( bmc_411120c )
	PORT_START("DIP")
	PORT_CONFNAME( 0x01, 0x00, "DIP Switch" )
	PORT_CONFSETTING(    0x00, "Off" )
	PORT_CONFSETTING(    0x01, "On" )
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

ioport_constructor nes_bmc_15in1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(bmc_15in1);
}

ioport_constructor nes_bmc_5in1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_5in1 );
}

ioport_constructor nes_bmc_f600_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_f600 );
}

ioport_constructor nes_bmc_hik8_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_hik8 );
}

ioport_constructor nes_bmc_411120c_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_411120c );
}

ioport_constructor nes_bmc_kc885_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_kc885 );
}

ioport_constructor nes_family4646_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( family4646 );
}

ioport_constructor nes_8025_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_8025 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

nes_nitra_device::nes_nitra_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_NITRA, tag, owner, clock)
{
}

nes_bmw8544_device::nes_bmw8544_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMW8544, tag, owner, clock), m_reg{0, 0, 0}, m_bus_latch(0)
{
}

nes_fs6_device::nes_fs6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_FS6, tag, owner, clock)
	, m_prg189(false)
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

nes_hengg412_device::nes_hengg412_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_HENGG_412, tag, owner, clock)
{
}

nes_family4646_device::nes_family4646_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_FAMILY4646, tag, owner, clock), m_config(*this, "CONFIG")
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
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_pad(*this, "PAD"), m_mmc3_mode(true)
{
}

nes_kasing_device::nes_kasing_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_kasing_device(mconfig, NES_KASING, tag, owner, clock)
{
}

nes_sglionk_device::nes_sglionk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 board)
	: nes_kasing_device(mconfig, type, tag, owner, clock), m_board(board)
{
	m_irq_type = 1;     // NESdev: mapper 114's IRQ resembles the MMC3A (Aladdin depends on it), mapper 115's the MMC3C
	m_mapper115 = false;
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
	: nes_txrom_device(mconfig, NES_KAY, tag, owner, clock)
	, m_ovr{0, 0, 0}
	, m_outer(0x80)
	, m_prot_out(0)
	, m_prot_index(0)
	, m_prot_latch(0)
	, m_prot_lock(0)
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
	: nes_txrom_device(mconfig, NES_KOF96, tag, owner, clock), m_mmc3_mode(true)
{
}

nes_sf3_device::nes_sf3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_SF3, tag, owner, clock), m_submapper(0), m_outer(0)
{
}

nes_cocoma_device::nes_cocoma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_COCOMA, tag, owner, clock)
{
}

nes_gouder_device::nes_gouder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_GOUDER, tag, owner, clock), m_submapper(0)
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
	, m_reg{0, 0, 0, 3}
	, m_ext(0)
{
}

nes_a88s1_device::nes_a88s1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_A88S1, tag, owner, clock), m_submapper(0)
{
}

nes_bmc_el86xc_device::nes_bmc_el86xc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_EL86XC, tag, owner, clock)
{
}

nes_fk23c_device::nes_fk23c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_mmc_cmd1(0)
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
	: nes_txrom_device(mconfig, NES_BMC_15IN1, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
{
}

nes_bmc_sbig7_device::nes_bmc_sbig7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_BMC_SBIG7, tag, owner, clock)
{
}

nes_bmc_hik8_device::nes_bmc_hik8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_count(0)
	, m_dip(*this, "DIP")
	, m_menu(*this, "MENU")
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
	, m_dip(*this, "DIP")
{
}

nes_bmc_810305c_device::nes_bmc_810305c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txsrom_device(mconfig, NES_BMC_810305C, tag, owner, clock), m_outer(0), m_slot_a17(0), m_last_slot(0), m_live_a17(false)
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
	: nes_pjoy84_device(mconfig, NES_PJOY84, tag, owner, clock, 126)
{
}

nes_pjoy84_device::nes_pjoy84_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u16 mapper)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_submapper(0), m_sl0(0xff), m_mapper(mapper)
{
}

nes_bs300_device::nes_bs300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_pjoy84_device(mconfig, NES_BS300, tag, owner, clock, 422)
{
}

nes_ing022_device::nes_ing022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_pjoy84_device(mconfig, NES_ING022, tag, owner, clock, 534)
{
}

nes_smd133_device::nes_smd133_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_SMD133, tag, owner, clock), m_submapper(0)
{
}




void nes_bmw8544_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_bus_latch));
}

void nes_bmw8544_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	m_bus_latch = 0;
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
	set_nt_mirroring(PPU_MIRROR_HORZ); // Space Shuttle on CB-4035 doesn't set mirroring bit. Whether this cart is hard-wired to reset correctly to horizontal mirroring is not clear.
}

void nes_hengg412_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_hengg412_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	mmc3_common_initialize(0xff, 0xff, 0);
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

void nes_8237_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

// NESdev INES Mapper 215: "The Outer Bank Register is reset to its power-up value if a Reset has been detected via an
// interruption of the M2 signal"; nothing else is reset: a console reset leaves $5000, the scrambling mode ($5007) and
// the MMC3 clone's registers and IRQ counter as they were (as the MMC3 itself, which has no reset input). At power-up
// all three take their power-up values: $5000 = $00, $5001 = $xF, $5007 = $00.
void nes_8237_device::pcb_reset()
{
	if (m_powered)
	{
		m_reg[1] = 0x0f;
		mmc3_soft_reset();
		update_banks();
		return;
	}
	m_powered = true;

	mmc3_common_initialize(0x1f, 0xff, 0);

	m_reg[0] = 0;
	m_reg[1] = 0x0f;
	m_reg[2] = 0;
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

ioport_constructor nes_kasing_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(kasing);
}

ioport_constructor nes_sglionk_device::device_input_ports() const
{
	return nullptr;     // mapper 114 has no solder pad
}

void nes_kasing_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_kasing_device::pcb_reset()
{
	m_mmc3_mode = true;
	mmc3_common_initialize(0xff, 0xff, m_irq_type);

	if (m_mapper115)    // power-on $6000 = $00: MMC3 mode with PRG A18 low, which leaves a 256 KiB MMC3 window
	{
		m_prg_base = 0;
		m_prg_mask = 0x1f;
		set_prg(m_prg_base, m_prg_mask);
	}
}

void nes_kay_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_ovr));
	save_item(NAME(m_outer));
	save_item(NAME(m_prot_out));
	save_item(NAME(m_prot_index));
	save_item(NAME(m_prot_latch));
	save_item(NAME(m_prot_lock));
}

void nes_kay_device::pcb_reset()
{
	std::fill(std::begin(m_ovr), std::end(m_ovr), 0x00);
	m_outer = 0x80;     // the outer bank register powers on set
	m_prot_out = 0;
	m_prot_index = 0;
	m_prot_latch = 0;
	m_prot_lock = 0;
	mmc3_common_initialize(0x1f, 0xff, 0);
	m_mmc_prg_bank[0] = 0x00;   // MMC3 registers R6/R7 power on as $00/$01
	m_mmc_prg_bank[1] = 0x01;
	update_outer();
}

void nes_kay_device::update_outer()
{
	m_prg_base = (m_outer & 0x80) >> 2;
	m_chr_base = (m_prg_size == m_vrom_size) ? ((m_outer & 0x80) << 1) : 0;     // only the A9713 (PRG and CHR the same size)
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_kay_device::prot_sync()
{
	// the protection index decides which banks the latched value goes to
	switch (m_prot_index & 0x3f)
	{
		case 0x20:
		case 0x29:
		case 0x2b:
		case 0x3c:
		case 0x3f:
			m_prot_lock = 1;
			m_ovr[0] = m_prot_latch;
			break;
		case 0x26:
			m_prot_lock = 0;
			m_ovr[0] = m_prot_latch;
			break;
		case 0x2c:
			m_prot_lock = 1;
			if (m_prot_latch)
				m_ovr[0] = m_prot_latch;
			break;
		case 0x28:
			m_prot_lock = 0;
			m_ovr[1] = m_prot_latch;
			break;
		case 0x2a:
			m_prot_lock = 0;
			m_ovr[2] = m_prot_latch;
			break;
		case 0x2f:      // keep the banks previously selected
			break;
		default:        // invalid index: restore normal MMC3 PRG banking
			m_prot_index = 0;
			break;
	}
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

void nes_kof96_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_kof96_device::pcb_reset()
{
	m_mmc3_mode = true;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_cocoma_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_gouder_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_submapper));
}

void nes_gouder_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	m_reg[5] = 0x11;    // power-on value of the PRG/mirroring register
	mmc3_common_initialize(0xff, 0xff, 0);
	m_mmc_prg_bank[0] = 0x00;   // MMC3 registers R6/R7 power on as $00/$01 (submapper 1 maps R6 >> 2 as its only PRG bank)
	m_mmc_prg_bank[1] = 0x01;
	if (m_submapper)
		set_prg(m_prg_base, m_prg_mask);
	else
	{
		prg32(bitswap<2>(m_reg[5], 4, 0));
		update_mirroring();
	}
}

void nes_sa9602b_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_prg_chip));
}

void nes_sa9602b_device::pcb_reset()
{
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
	save_item(NAME(m_ext));
}

void nes_a9746_device::pcb_reset()
{
	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
	m_reg[3] = 3;   // the outer bank register must be $03 for the menu to appear
	m_ext = 0;
	mmc3_common_initialize(0x0f, 0x7f, 0);
	m_mmc_prg_bank[0] = 0x00;   // MMC3 registers R6/R7 power on as $00/$01 (this board never writes them, so $8000-$BFFF would stay on the last banks)
	m_mmc_prg_bank[1] = 0x01;
	update_outer();
}

void nes_a88s1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_submapper));

	// power-on values for registers; they don't change with soft reset, only the DIP value advances
	m_reg[0] = 0x00;
	m_reg[1] = 0x03;
	m_reg[2] = 0xff;
}

void nes_a88s1_device::pcb_reset()
{
	m_reg[2]++;
	mmc3_common_initialize(0x1f, 0xff, 0);
	update_banks();
}

void nes_bmc_el86xc_device::pcb_reset()
{
	mmc3_common_initialize((m_outer_prg_size >> 3) - 1, 0x7f, 0);
}

void nes_fk23c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_mmc_cmd1));
}

void nes_fk23c_device::pcb_reset()
{
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
	m_mmc_cmd1 = 0;
	m_reg[0] = m_reg[1] = m_reg[2] = m_reg[3] = 0;
	m_reg[4] = m_reg[5] = m_reg[6] = m_reg[7] = 0xff;
	mmc3_common_initialize(0xff, 0xff, 0);
	fk23c_set_prg();
	fk23c_set_chr();
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
	// the loader only sets the game sizes for submappers 1 to 4; submapper 0 (and iNES 1.0 images) is NESdev's
	// 128 KiB PRG and 128 KiB CHR per game and ran with no bank masks at all before
	if (!m_outer_prg_size)
		m_outer_prg_size = 128;
	if (!m_outer_chr_size)
		m_outer_chr_size = 128;

	assert(std::has_single_bit(m_outer_prg_size));
	assert(std::has_single_bit(m_outer_chr_size));

	m_count = (m_count + 1) & 3;

	if (m_submapper == 4)
	{
		// NESdev submapper 4: the first game has 256 KiB of PRG-ROM, the other three 128 KiB each, all with 128 KiB of CHR-ROM
		const int prg_size = m_count ? 128 : 256;
		mmc3_common_initialize((prg_size >> 3) - 1, m_outer_chr_size - 1, 0);
		m_prg_base = m_count ? ((256 + (m_count - 1) * 128) >> 3) : 0;
	}
	else
	{
		mmc3_common_initialize((m_outer_prg_size >> 3) - 1, m_outer_chr_size - 1, 0);
		m_prg_base = m_count << (std::countr_zero(m_outer_prg_size) - 3);
	}
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
	save_item(NAME(m_prg_map));
	save_item(NAME(m_nrom_8k));
	save_item(NAME(m_chr_8k));
	save_item(NAME(m_mmc1_reg));
	save_item(NAME(m_mmc1_shift));
	save_item(NAME(m_mmc1_count));
	save_item(NAME(m_vrc_prg));
	save_item(NAME(m_vrc_chr));
	save_item(NAME(m_vrc_flip));
	save_item(NAME(m_vrc_irq_count));
	save_item(NAME(m_vrc_irq_latch));
	save_item(NAME(m_vrc_irq_enable));
	save_item(NAME(m_vrc_irq_enable_latch));
	save_item(NAME(m_vrc_irq_mode));
	save_item(NAME(m_vrc_irq_prescale));
}

void nes_tech9in1_device::pcb_reset()
{
	if (m_vram.empty())
		vram_alloc(0x2000);     // CHR-RAM mode ($5002 bit 0) replaces the CHR-ROM by 8 KiB of RAM

	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	std::fill(std::begin(m_prg_map), std::end(m_prg_map), 0);
	std::fill(std::begin(m_mmc1_reg), std::end(m_mmc1_reg), 0x00);
	m_mmc1_reg[0] = 0x0c;
	m_mmc1_shift = 0;
	m_mmc1_count = 0;
	std::fill(std::begin(m_vrc_prg), std::end(m_vrc_prg), 0);
	std::fill(std::begin(m_vrc_chr), std::end(m_vrc_chr), 0);
	m_vrc_flip = 0;
	m_vrc_irq_count = m_vrc_irq_latch = 0;
	m_vrc_irq_enable = m_vrc_irq_enable_latch = 0;
	m_vrc_irq_mode = 0;
	m_vrc_irq_prescale = 0;

	mmc3_common_initialize(0x1f, 0xff, 0);
	update_all();
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

void nes_bmc_hik8_device::pcb_reset()
{
	set_nt_mirroring(PPU_MIRROR_VERT);  // necessary since some boards/games don't reliably set mirroring (Rockman 1 on mc_s13 at least)

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
	mmc3_start();
	save_item(NAME(m_outer));
	save_item(NAME(m_slot_a17));
	save_item(NAME(m_last_slot));
	save_item(NAME(m_live_a17));
}

void nes_bmc_810305c_device::pcb_reset()
{
	m_outer = 0;
	m_slot_a17 = 0;
	m_last_slot = 0;
	m_live_a17 = false;
	mmc3_common_initialize(0x1f, 0x7f, 0);
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

void nes_bmc_yy841155c_device::pcb_reset()
{
	// both outer bank registers start clear (FCEUmm's mapper 376; NESdev's page gives no other value)
	m_reg[0] = m_reg[1] = 0;
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_pjoy84_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_submapper));
	save_item(NAME(m_sl0));
}

void nes_pjoy84_device::pcb_reset()
{
	m_sl0++;    // soft resets cycle through the SL0 (menu selection) input values
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	mmc3_common_initialize(0x1f, 0xff, 0);
	m_mmc_prg_bank[0] = 0x00;   // MMC3 registers R6/R7 power on as $00/$01
	m_mmc_prg_bank[1] = 0x01;
	update_banks();
}

void nes_smd133_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_submapper));
}

void nes_smd133_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	mmc3_common_initialize(0xff, 0xff, 1);      // NESdev: the AA6023's IRQ follows the NEC MMC3 behavior (a latch value of zero disables it)
	update_banks();
}

void nes_smd133_device::set_smd133_submapper(u8 val)
{
	// submapper bit 0 selects the solder pad: odd = registers at $5000-$5FFF, even = registers overlay WRAM at $6000-$6FFF
	// ($7000-$7FFF for submapper 2, see write_m); NESdev decodes them with mask $F007
	m_submapper = val;
	m_smd133_addr = BIT(val, 0) ? 0x5000 : 0x6000;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board by Nitra

 Games: Queen Bee, Time Diver Avenger

 MMC3 clone with A10 tied to what is normally A0 and
 data instead coming from the lower address lines.

 Queen Bee V also has a 74153 mux that drives PRG A13: 0 for $C000-$DFFF, 1 for $E000-$FFFF
 and the MMC3's own output for $8000-$BFFF (NESdev). That only differs from a plain MMC3 when
 MMC3 PRG mode 1 maps an odd R6 value at $C000. The two games share a header, so the mux is
 emulated for both.

 iNES: mapper 250

 In MAME: Supported.

 -------------------------------------------------*/

void nes_nitra_device::write_h(offs_t offset, u8 data)
{
	LOG("nitra write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write((offset & 0x6000) | BIT(offset, 10), offset);
}

void nes_nitra_device::prg_cb(int start, int bank)
{
	// 74153 mux: PRG A13 is 0 for $C000-$DFFF and 1 for $E000-$FFFF, the MMC3's own output is used for $8000-$BFFF
	if (start == 2)
		bank &= ~1;
	else if (start == 3)
		bank |= 1;
	nes_txrom_device::prg_cb(start, bank);
}

/*-------------------------------------------------

 Board BMW8544

 Games: Dragon Fighter (Flying Star)

 MMC3 clone with an overridden $8000 PRG bank and custom CHR
 banking (2K XOR bank at PPU $0000, 2K bank at $0800, 4K bank at
 $1000) that is fed by two extra registers. Reading $6000-$6FFF
 (even addresses, PRG-RAM enabled) copies the data of the last CPU
 write to ANY address (including zero page) into one of those
 registers, so the NES driver forwards every CPU write to
 cpu_write_latch().

 NES 2.0: mapper 292 (UNIF UNL-DRAGONFIGHTER)

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmw8544_device::prg_cb(int start, int bank)
{
	// the $8000 bank is replaced by the board's own register
	prg8_x(start, start == 0 ? (m_reg[0] & 0x1f) : bank);
}

void nes_bmw8544_device::chr_cb(int start, int bank, int source)
{
	if (start == 0)
		chr2_0((bank >> 1) ^ m_reg[1], source);
	else if (start == 2)
		chr2_2((bank >> 1) | ((m_reg[2] & 0x40) << 1), source);
	else if (start == 4)
		chr4_4(m_reg[2] & 0x3f, source);
}

u8 nes_bmw8544_device::read_m(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "bmw8544 read_m, offset: %04x\n", offset);

	if (offset < 0x1000 && !BIT(offset, 0))
	{
		if ((m_reg[0] & 0xe0) == 0xc0)
			m_reg[1] = m_bus_latch;
		else
			m_reg[2] = m_bus_latch;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	return 0;
}

void nes_bmw8544_device::write_m(offs_t offset, u8 data)
{
	LOG("bmw8544 write_m, offset: %04x, data: %02x\n", offset, data);

	if (offset < 0x1000 && !BIT(offset, 0))
	{
		m_reg[0] = data;
		set_prg(m_prg_base, m_prg_mask);
	}
}

/*-------------------------------------------------

 Board UNL-FS6

 Games: Street Fighter VI / Fight Street VI, Master Fighter II (MRCM)

 MMC3 clone (identical, but for switched address lines:
 the MMC3's A0 input comes from CPU A1). Submapper 3, the MRCM
 board, also takes the 32 KiB PRG bank from a register at
 $6000-$7FFF like mapper 189 does, ignoring the MMC3's own PRG
 registers, and has no WRAM (NESdev INES Mapper 196).

 NES 2.0: mapper 196 submappers 1 and 3

 In MAME: Supported.

 -------------------------------------------------*/

void nes_fs6_device::write_h(offs_t offset, u8 data)
{
	LOG("fs6 write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write(bitswap<2>(offset, 0, 1) | (offset & ~0x03), data);
}

u8 nes_fs6_device::read_m(offs_t offset)
{
	return m_prg189 ? get_open_bus() : nes_txrom_device::read_m(offset);
}

void nes_fs6_device::write_m(offs_t offset, u8 data)
{
	LOG("fs6 write_m, offset: %04x, data: %02x\n", offset, data);

	if (m_prg189)
		prg32((data | (data >> 4)) & 0x0f);
	else
		nes_txrom_device::write_m(offset, data);
}

void nes_fs6_device::prg_cb(int start, int bank)
{
	if (!m_prg189)
		nes_txrom_device::prg_cb(start, bank);
}

void nes_fs6_device::pcb_reset()
{
	nes_txrom_device::pcb_reset();

	if (m_prg189)
		prg32(0);
}

/*-------------------------------------------------

 BTL-SUPERBROS11

 Games: Super Mario Bros. 11, Super Mario Bros. 17

 This acts basically like a MMC3 with different use of write
 address.

 iNES: mapper 196

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sbros11_device::write_h(offs_t offset, u8 data)
{
	LOG("smb11 write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write((offset & 0x6000) | BIT(offset, 2), data);
}

/*-------------------------------------------------

 UNL-MALISB

 Games: Super Mali Bros Splash Bomb

 This is very similar to mapper 196, but with additional
 data bit swap.

 NES 2.0: mapper 325

 In MAME: Supported.

 -------------------------------------------------*/

void nes_malisb_device::prg_cb(int start, int bank)
{
	bank = bitswap<4>(bank, 2, 3, 1, 0);
	prg8_x(start, bank);
}

void nes_malisb_device::chr_cb(int start, int bank, int source)
{
	bank = bitswap<8>(bank, 7, 6, 1, 4, 3, 2, 5, 0);
	chr1_x(start, bank, source);
}

void nes_malisb_device::write_h(offs_t offset, u8 data)
{
	LOG("malisb write_h, offset: %04x, data: %02x\n", offset, data);

	offset = (offset & 0x6000) | BIT(offset, 3) | (BIT(offset, 14) & BIT(offset, 2));
	txrom_write(offset, data);
}

/*-------------------------------------------------

 BMC-FAMILY-4646B

 Known Boards: Unknown Multigame Bootleg Board (4646B)
 Games: 2 in 1 - Family Kid & Aladdin 4, various multicarts.

 MMC3 clone with banking for multigame menu.

 iNES: mapper 134 (T4A54A / WX-KB4K / BS-5652)

 $6000: mode (CNROM CHR mode, PRG A19, CHR A19, solder pad
 read mode, register lock), $6001: outer banks, $6002: CNROM
 CHR bank, $6003: unknown (only $00 is ever written).
 With $6000 bit 6 set, reads from $8000-$FFFF return the
 solder pad setting in bit 0 (configurable below).

 $6001 bit 2 / bit 6 select 128 KiB outer PRG / CHR banks, in
 which PRG A17 / CHR A17 come from $6001 bit 0 / bit 4; otherwise
 they come from the MMC3. In NROM mode ($6001 bit 7) the MMC3
 sees A13 = A14 = 0, so register 6 (or the fixed bank when PRG A14
 is inverted) applies to the whole range, with A13 from the CPU and
 A14 from the CPU (NROM-256) or the MMC3 ($6001 bit 3, NROM-128).
 In CNROM mode ($6000 bit 3) MMC3 CHR writes do not change the
 8 KiB bank taken from $6002.

 In MAME: Supported.

 TODO: YH-4103 doesn't soft reset cleanly like the other
 carts, but does this happen on hardware?

 -------------------------------------------------*/

void nes_family4646_device::prg_cb(int start, int bank)
{
	if (!BIT(m_reg[1], 7))    // MMC3 mode
		nes_txrom_device::prg_cb(start, bank);
	else if (start == 0)      // NROM mode, only uses MMC3's $8000 bank; PRG A13 and A14 come from the CPU (or A14 from the MMC3)
	{
		if (BIT(m_reg[1], 3))
		{
			prg16_89ab(bank >> 1);
			prg16_cdef(bank >> 1);
		}
		else
			prg32(bank >> 2);
	}
}

void nes_family4646_device::chr_cb(int start, int bank, int source)
{
	if (BIT(m_reg[0], 3))     // CNROM mode: the 8 KiB bank comes from $6002, the MMC3 CHR registers are not used
		bank = m_chr_base | ((m_reg[2] & (m_chr_mask >> 3)) << 3) | start;
	chr1_x(start, bank, source);
}

u8 nes_family4646_device::read_h(offs_t offset)
{
	// $6000 bit 6: CPU $8000-$FFFF reads the solder pad setting instead of PRG-ROM
	if (BIT(m_reg[0], 6))
		return (get_open_bus() & 0xfe) | BIT(m_config.read_safe(0), 0);
	return hi_access_rom(offset);
}

void nes_family4646_device::write_m(offs_t offset, u8 data)
{
	LOG("family4646 write_m, offset: %04x, data: %02x\n", offset, data);

	int reg = offset & 0x03;
	if (!BIT(m_reg[0], 7))    // lock bit
		m_reg[reg] = data;
	else if (reg == 2)        // final two bits of reg 2 respond regardless
		m_reg[reg] = (m_reg[reg] & ~0x03) | (data & 0x03);
	else
		return;

	// outer PRG: A19 = $6000.4, A18 = $6001.1, A17 = $6001.0 only in 128 KiB mode (otherwise A17 comes from the MMC3)
	m_prg_base = ((m_reg[0] & 0x10) << 2) | ((m_reg[1] & 0x02) << 4) | (BIT(m_reg[1], 2) ? ((m_reg[1] & 0x01) << 4) : 0);
	m_prg_mask = 0x1f >> BIT(m_reg[1], 2);
	set_prg(m_prg_base, m_prg_mask);

	// outer CHR: A19 = $6000.5, A18 = $6001.5, A17 = $6001.4 only in 128 KiB mode (otherwise A17 comes from the MMC3)
	m_chr_base = ((m_reg[0] & 0x20) << 4) | ((m_reg[1] & 0x20) << 3) | (BIT(m_reg[1], 6) ? ((m_reg[1] & 0x10) << 3) : 0);
	m_chr_mask = 0xff >> BIT(m_reg[1], 6);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/*-------------------------------------------------

 HENGGEDIANZI 412 (no UNIF board)

 Games: Super Game 4-in-1 (Hengge 18001, 18003, 18007, 18010, 18016),
 Intellivision 10-in-1 Plug 'n Play 2nd Edition (not held)

 "MMC3 plus NROM" multicart. The outer registers sit at $6000-$6002
 (mask $E003) behind the MMC3's WRAM interface, so $A001 has to enable
 the WRAM before they can be written.

 $6000: CHR A18..A13 in NROM mode (bits 7-2).
 $6001: bit 0 locks $6001; bit 1 takes PRG A18 from bit 2 instead of the
 MMC3; bit 4 takes PRG A17 from bit 6; bit 5 takes CHR A17 from bit 7;
 bit 3 is CHR A18. (NESdev's labels for bits 2 and 6 read "if Q=2" and
 "if P=1", which do not fit the selects it gives for bits 1 and 4; they
 are taken as the values those selects choose.)
 $6002: bit 1 selects NROM banking, bit 2 NROM-256 (otherwise NROM-128),
 bit 3 is PRG A14 in NROM-128, bits 7-4 are PRG A18..A15.

 NES 2.0: mapper 412

 In MAME: Supported.

 -------------------------------------------------*/

void nes_hengg412_device::prg_cb(int start, int bank)
{
	if (!BIT(m_reg[2], 1))    // MMC3 mode: PRG A17 and A18 can come from $6001 instead of the MMC3
	{
		if (BIT(m_reg[1], 4))
			bank = (bank & ~0x10) | (BIT(m_reg[1], 6) << 4);
		if (BIT(m_reg[1], 1))
			bank = (bank & ~0x20) | (BIT(m_reg[1], 2) << 5);
		prg8_x(start, bank);
	}
	else if (start == 0)      // NROM mode: the MMC3's PRG registers are not used
	{
		if (BIT(m_reg[2], 2))
			prg32(m_reg[2] >> 4);
		else
		{
			const int bank16 = ((m_reg[2] >> 4) << 1) | BIT(m_reg[2], 3);
			prg16_89ab(bank16);
			prg16_cdef(bank16);
		}
	}
}

void nes_hengg412_device::chr_cb(int start, int bank, int source)
{
	if (BIT(m_reg[2], 1))     // NROM mode: one 8 KiB bank from $6000
		bank = ((m_reg[0] >> 2) << 3) | start;
	else                      // MMC3 mode: CHR A17 can come from $6001 instead of the MMC3, CHR A18 always does
	{
		if (BIT(m_reg[1], 5))
			bank = (bank & ~0x80) | (BIT(m_reg[1], 7) << 7);
		bank = (bank & ~0x100) | (BIT(m_reg[1], 3) << 8);
	}
	chr1_x(start, bank, source);
}

void nes_hengg412_device::write_m(offs_t offset, u8 data)
{
	LOG("hengg412 write_m, offset: %04x, data: %02x\n", offset, data);

	// the registers are written through the MMC3's WRAM interface: $A001 has to enable it, and not write protect it
	if ((m_wram_protect & 0xc0) != 0x80)
		return;

	const int reg = offset & 0x03;
	if (reg == 3)
		return;                         // no fourth register
	if (reg == 1 && BIT(m_reg[1], 0))   // $6001 is locked
		return;

	m_reg[reg] = data;
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/*-------------------------------------------------

 BTL-PIKACHUY2K

 Games: Pikachu Y2k

 iNES: mapper 254

 In MAME: Supported.

 WRAM reads are XORed with the $A001 value until the
 first write to $8000, as NESdev describes (previously $A000 ended it).

 -------------------------------------------------*/

void nes_pikay2k_device::write_h(offs_t offset, uint8_t data)
{
	LOG("pikay2k write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x0000:    // bank select: the first write ends the XOR of WRAM reads
			m_reg[0] = 0;
			txrom_write(offset, data);
			break;

		case 0x2001:    // PRG-RAM protect: also sets the XOR mask
			m_reg[1] = data;
			txrom_write(offset, data);
			break;

		default:
			txrom_write(offset, data);
			break;
	}
}

// NESdev (from MesenCE): WRAM reads are XORed with the value last written to $A001 until the game writes $8000
void nes_pikay2k_device::write_m(offs_t offset, uint8_t data)
{
	LOG("pikay2k write_m, offset: %04x, data: %02x\n", offset, data);

	m_prgram[offset & 0x1fff] = data;
}

uint8_t nes_pikay2k_device::read_m(offs_t offset)
{
	LOG("pikay2k read_m, offset: %04x\n", offset);

	return m_prgram[offset & 0x1fff] ^ (m_reg[0] & m_reg[1]);
}

/*-------------------------------------------------

 Board UNL-8237, UNL-8237A

 Games: Pocahontas 2, MK3, various multicarts

 MMC3 clone with register and address scrambling and
 a few extra banking modes by writing 0x5000-0x5fff.

 NES 2.0: mapper 215, submapper 0 (UNL-8237) and submapper 1 (UNL-8237A)

 Checked against NESdev's INES Mapper 215 page, register by register: the
 $F007 register mask, $5000 (NROM override, 128 KiB mode, NROM-256, 16 KiB
 bank), $5001 for both submappers (PRG A18-A20 and CHR A18-A20, A17 in
 128 KiB mode), $5007 and both scrambling tables, the power-up values, and
 the reset rule (only $5001 is reset). Not documented, so not modelled: the
 MMC3 clone's IRQ revision, and the alternative outer bank registers in the
 $6xxx range that some multicarts may have.

 TODO: The Lion King in mc_9king (9-in-1 King001) stops after level 1. Its
 code (decrypted into RAM at $0680) sums two 256-byte windows ($828E and
 $BCC6, as sum of X xor byte) read with $5000 = $8F and $5001 = $07, i.e.
 16 KiB bank 63 (file offset $FC000), and wants $3E and $BF; the dump gives
 $64 and $B8 there, and no 16 KiB block or 32 KiB pair of it gives $3E and
 $BF. The standalone release's same check passes on MAME's mapping. Either
 the dump (flagged baddump) or the multicart itself is at fault; a second
 dump or the cartridge would tell.
 After a reset, Lion King, Somari and Aladdin restart into Super Aladdin,
 not the menu: the menu's code (8 KiB bank $F6) returns early when RAM $01B0
 holds $89, which it sets at the first boot and which a console reset keeps.
 Not compared with the cartridge.

 -------------------------------------------------*/

void nes_8237_device::update_banks()
{
	int a17 = BIT(m_reg[0], 6);
	m_chr_base = m_board ? (m_reg[1] & 0x0e) << 7 : (m_reg[1] & 0x0c) << 6;
	m_chr_base |= (BIT(m_reg[1], 5) & a17) << 7;
	m_chr_mask = 0xff >> a17;
	set_chr(m_chr_source, m_chr_base, m_chr_mask);

	m_prg_base = m_board ? bitswap<3>(m_reg[1], 3, 1, 0) : m_reg[1] & 0x03;
	m_prg_base = m_prg_base << 5 | (m_reg[1] & (a17 << 4));
	m_prg_mask = 0x1f >> a17;

	if (BIT(m_reg[0], 7))
	{
		u8 bank = m_prg_base >> 1 | (m_reg[0] & (m_prg_mask >> 1));
		u8 mode = BIT(m_reg[0], 5);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
	else
		set_prg(m_prg_base, m_prg_mask);
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

 MMC3 clone that appears to be the same as the 8237
 boards above with added protection reads at 0x5000.

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

 Per NESdev, mapper 115 decodes A1-A0 ($6000 mode/PRG, $6001 outer
 CHR A18, $6002 read = 3-bit solder pad), $6000 bit 6 is PRG A18 in both modes and
 the MMC3 window is 256 KiB. Mapper 114 (Super Game) shares this code without those.

 -------------------------------------------------*/

void nes_kasing_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)
		nes_txrom_device::prg_cb(start, bank);
}

u8 nes_kasing_device::read_m(offs_t offset)
{
	LOG("kasing read_m, offset: %04x\n", offset);

	// NESdev (mapper 115, mask $E003): $6002 reads the solder pad value in bits 2-0
	if (m_mapper115 && (offset & 0x03) == 0x02)
		return (get_open_bus() & 0xf8) | (m_pad.read_safe(0) & 0x07);

	return nes_txrom_device::read_m(offset);
}

void nes_kasing_device::write_m(offs_t offset, u8 data)
{
	LOG("kasing write_m, offset: %04x, data: %02x\n", offset, data);

	// NESdev: mapper 115 decodes A1-A0 (mask $E003), mapper 114 only A0 (mask probably $E001)
	switch (offset & (m_mapper115 ? 0x03 : 0x01))
	{
		case 0x01:  // outer CHR-ROM bank, 256 KiB (CHR A18)
			m_chr_base = BIT(data, 0) << 8;
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;

		case 0x00:  // mode / NROM bank
			m_mmc3_mode = !BIT(data, 7);
			if (m_mmc3_mode)
			{
				if (m_mapper115)    // PRG A18 (bit 6) also applies in MMC3 mode, to a 256 KiB MMC3 window
				{
					m_prg_base = BIT(data, 6) << 5;
					m_prg_mask = 0x1f;
				}
				set_prg(m_prg_base, m_prg_mask);
			}
			else
			{
				u8 bank = data & 0x0f;
				if (m_mapper115)
					bank |= BIT(data, 6) << 4;      // PRG A18
				u8 mode = BIT(data, 5);
				prg16_89ab(bank & ~mode);
				prg16_cdef(bank | mode);
			}
			break;
	}
}

/*-------------------------------------------------

 Bootleg Boards by Super Game and Hosenkan

 Games: The Lion King, Aladdin, Boogerman, Pocohon,
 Super Donkey Kong

 MMC3 clone with register and address scrambling and
 a few extra banking modes by writing 0x6000-0x7fff.
 This is the same as mapper 115 with scrambling. NES 2.0
 submapper 0 (Lion King, Aladdin, Pocohontas, Super Donkey Kong)
 and submapper 1 (Boogerman) use different scrambling patterns.

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

 Writes to $5000-$5FFF select one of four protection values that
 reads of $5000-$5FFF return ($83, $83, $42, $00). $8003 selects a
 protection index and also writes its value to the MMC3 bank select
 register ($8000); $8001 latches the written value with its lower
 six bits reversed, and depending on the index that value replaces
 the $A000/$C000/$E000 banks (see prot_sync()). The A9713 (512 KiB
 PRG and CHR) has an outer bank register at $5180 that powers on
 set. On boards with more CHR than PRG (only Street Fighter Zero 2 '97)
 CHR A18 is set for the pattern table that holds the MMC3's 2 KiB banks,
 as in FCEUmm's 121.c.

 iNES: mapper 121

 In MAME: Supported.

 On the A9711 NESdev makes CHR A18 follow $8000 bit 7 (0: inverted PPU A12,
 1: PPU A12), the same bit that inverts the MMC3's CHR banks, which is the
 rule chr_cb() implements: the pattern table holding the 2 KiB banks always
 uses the second 256 KiB.

 Super Lion King 2's title screen, once reported as broken, draws correctly.

 -------------------------------------------------*/

void nes_kay_device::write_l(offs_t offset, u8 data)
{
	LOG("kay write_l, offset: %04x, data: %02x\n", offset, data);

	static constexpr u8 prot_table[4] = {0x83, 0x83, 0x42, 0x00};

	offset += 0x100;
	if (offset >= 0x1000)
	{
		m_prot_out = prot_table[data & 0x03];

		if ((offset & 0x1180) == 0x1180)    // A9713 outer bank register
		{
			m_outer = data;
			update_outer();
		}
	}
}

u8 nes_kay_device::read_l(offs_t offset)
{
	LOG("kay read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (offset >= 0x1000)
		return m_prot_out;
	else
		return get_open_bus();
}

void nes_kay_device::chr_cb(int start, int bank, int source)
{
	// boards with more CHR than PRG (the A9713 uses its outer bank register instead): CHR A18 is set for the pattern table
	// that holds the MMC3's 2 KiB banks, i.e. the right one while the CHR invert bit is set and the left one while it is clear
	if (m_prg_size != m_vrom_size && ((start >= 4) == BIT(m_latch, 7)))
		bank |= 0x100;
	chr1_x(start, bank, source);
}

void nes_kay_device::set_prg(int prg_base, int prg_mask)
{
	nes_txrom_device::set_prg(prg_base, prg_mask);

	if (m_prot_index & 0x3f)
	{
		prg8_ab(prg_base | m_ovr[2]);
		prg8_cd(prg_base | m_ovr[1]);
		prg8_ef(prg_base | m_ovr[0]);
	}
}

void nes_kay_device::write_h(offs_t offset, u8 data)
{
	LOG("kay write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x2000)       // $A000-$FFFF: plain MMC3 registers
	{
		txrom_write(offset, data);
		return;
	}

	switch (offset & 0x0003)
	{
		case 0x0001:    // protection latch: lower six bits reversed; the MMC3 clone always receives the write as well
			m_prot_latch = bitswap<6>(data, 0, 1, 2, 3, 4, 5);
			if (!m_prot_lock)
				prot_sync();
			txrom_write(offset, data);
			break;

		case 0x0003:    // protection index; the value is also written to the MMC3 bank select register ($8000)
			m_prot_index = data;
			prot_sync();
			txrom_write(offset & ~0x0003, data);
			break;

		case 0x0002:    // 121.c ignores $8002 (NESdev's $E001 mask would make it a second $8000); no known game writes it
			break;

		default:        // $8000: MMC3 bank select
			txrom_write(offset, data);
			break;
	}
	set_prg(m_prg_base, m_prg_mask);
}

/*-------------------------------------------------

 UNL-H2288

 Games: Earthworm Jim 2, Ultimate Mortal Kombat 3

 iNES: mapper 123

 In MAME: Supported.

 The $5800 PRG override register takes the 16 KiB bank number from
 bits 5, 2, 3 and 0 (bit 3 is bank bit 1). NESdev's diagram shows bit 4 there instead, but
 Earthworm Jim 2 writes $60, $61, $68 and $69 to step through four consecutive banks (bits 3
 and 0 count), shows a blank screen with NESdev's order and runs with this one.

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

void nes_txc_tw_device::write_l(offs_t offset, uint8_t data)
{
	if (offset < 0x20)   // the register starts at $4120
		return;
	write_m(offset, data);
}

// NESdev INES Mapper 189: [AAAA BBBB] at $4120-$7FFF selects the 32 KiB PRG bank, the two nibbles effectively OR'd
// (a 74175 provides 4 bits); the MMC3's own PRG registers are ignored and there is no WRAM
void nes_txc_tw_device::write_m(offs_t offset, uint8_t data)
{
	LOG("txc_tw write_l, offset: %04x, data: %02x\n", offset, data);

	prg32((data | (data >> 4)) & 0x0f);
}

// writes to 0x8000-0xffff are like MMC3 but no PRG bankswitch (because it is handled by low writes)
void nes_txc_tw_device::prg_cb(int start, int bank)
{
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

 MMC3 clone with an NROM-like PRG override register at $5000 and, on King of Fighters '96, 512 KiB of CHR-ROM

 iNES: mapper 187

 In MAME: Supported.

 -------------------------------------------------*/

void nes_kof96_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)
		nes_txrom_device::prg_cb(start, bank);
}

void nes_kof96_device::chr_cb(int start, int bank, int source)
{
	// NESdev: CHR A18 is selected directly by PPU A12, whatever the MMC3 is set to do, so the right pattern table always uses
	// the second 256 KiB (only King of Fighters '96 has that much CHR-ROM; the A18 bit wraps away on smaller images)
	bank |= (start & 4) << 6;
	chr1_x(start, bank, source);
}

void nes_kof96_device::write_l(offs_t offset, u8 data)
{
	LOG("kof96 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if ((offset & 0x1001) == 0x1000)
	{
		m_mmc3_mode = !BIT(data, 7);
		if (m_mmc3_mode)
			set_prg(m_prg_base, m_prg_mask);
		else
		{
			u8 bank = BIT(data, 1, 4);
			u8 mode = BIT(data, 5);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
	}
}

u8 nes_kof96_device::read_l(offs_t offset)
{
	LOG("kof96 read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (offset >= 0x1000)
		return 0x80;  // unknown protection read, kof96 expects that MSB is set
	else
		return 0;
}

/*-------------------------------------------------

 Bootleg Board for Super Fighter III

 MMC3 clone wired for 512 KiB of CHR-ROM: PPU A10 drives CHR-ROM A10 and the MMC3's CHR A10-A17 drive CHR-ROM A11-A18,
 so every MMC3 CHR bank number selects a 2 KiB bank. NES 2.0 submappers (NESdev) differ in what the MMC3 sees on its
 PA10/PA11 inputs: submapper 0 (and 3): PA10 = PPU A11, PA11 low, so R0 is a 4 KiB bank at $0000 and R2/R3 the 2 KiB
 banks at $1000/$1800; submapper 1: PA11 high, R1 (4 KiB), R4 and R5; submapper 2: PA10 and PA11 = PPU A11, R0, R1, R2
 and R5 as four 2 KiB banks. The MMC3's CHR inversion bit swaps the pattern tables as usual.
 Submapper 3 also has an outer PRG A17 register at $6000-$7FFF (mask $E000, only written while
 WRAM is enabled and not write-protected): bit 3 selects whether PRG A17 is the MMC3's (256 KiB)
 or bit 0 of the register (128 KiB).

 iNES: mapper 197

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sf3_device::set_chr(uint8_t chr_source, int chr_base, int chr_mask)
{
	const int invert = BIT(m_latch, 7);

	for (int slot = 0; slot < 8; slot++)
	{
		const int a12 = BIT(slot, 2) ^ invert;
		const int a11 = BIT(slot, 1);
		const int a10 = BIT(slot, 0);
		int pa10, pa11;

		switch (m_submapper)
		{
			case 1:     pa10 = a11; pa11 = 1; break;
			case 2:     pa10 = pa11 = a11; break;
			default:    pa10 = a11; pa11 = 0; break;
		}

		// the MMC3 register seen at that address and the 1 KiB bank number it drives (2 KiB registers use PA10 as bit 0)
		const int bank = a12 ? m_mmc_vrom_bank[2 + (pa11 << 1) + pa10] : ((m_mmc_vrom_bank[pa11] & 0xfe) | pa10);

		chr1_x(slot, ((chr_base | (bank & chr_mask)) << 1) | a10, chr_source);
	}
}

void nes_sf3_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_outer));
}

void nes_sf3_device::pcb_reset()
{
	m_outer = 0;
	nes_txrom_device::pcb_reset();
}

void nes_sf3_device::prg_cb(int start, int bank)
{
	if (m_submapper == 3 && BIT(m_outer, 3))
		bank = (bank & ~0x10) | (BIT(m_outer, 0) << 4);
	nes_txrom_device::prg_cb(start, bank);
}

void nes_sf3_device::write_m(offs_t offset, uint8_t data)
{
	LOG("sf3 write_m, offset: %04x, data: %02x\n", offset, data);

	if (m_submapper == 3 && (m_wram_protect & 0xc0) == 0x80)
	{
		m_outer = data & 0x09;
		set_prg(m_prg_base, m_prg_mask);
	}
	nes_txrom_device::write_m(offset, data);
}

void nes_sf3_device::write_h(offs_t offset, uint8_t data)
{
	LOG("sf3 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x0001:
			{
				const uint8_t cmd = m_latch & 0x07;
				if (cmd < 6)
				{
					m_mmc_vrom_bank[cmd] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
				}
				else
				{
					m_mmc_prg_bank[cmd - 6] = data;
					set_prg(m_prg_base, m_prg_mask);
				}
			}
			break;

		default:
			txrom_write(offset, data);
			break;
	}
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

 NES 2.0: mapper 208, submapper 0 (Gouder: $4800-$4FFF/$6800-$6FFF
 select a 32 KiB PRG bank and the mirroring, $5000-$5FFF is the XOR
 protection) and submapper 1 (Mortal Kombat JJ-01 without the Gouder
 copyright: no extra registers, 32 KiB PRG bank is MMC3 register 6 / 4).

 In MAME: Supported.

 -------------------------------------------------*/

void nes_gouder_device::write_l(offs_t offset, u8 data)
{
	static constexpr u8 conv_table[256] =
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

	LOG("gouder write_l, offset: %04x, data: %02x\n", offset, data);

	if (m_submapper)    // submapper 1 has no registers here
		return;

	offset += 0x100;
	switch (offset & 0x1800)
	{
		case 0x0800:
			m_reg[5] = data;
			prg32(bitswap<2>(m_reg[5], 4, 0));
			update_mirroring();
			break;
		case 0x1000:
			m_reg[4] = data;
			break;
		case 0x1800:
			m_reg[offset & 0x03] = data ^ conv_table[m_reg[4]];
			break;
	}
}

u8 nes_gouder_device::read_l(offs_t offset)
{
	LOG("gouder read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (!m_submapper && offset >= 0x1800)
		return m_reg[offset & 0x03];

	return get_open_bus();
}

void nes_gouder_device::update_mirroring()
{
	// submapper 0: the MMC3 mirroring register is ignored, bit 5 of the PRG register selects mirroring
	set_nt_mirroring(BIT(m_reg[5], 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_gouder_device::write_m(offs_t offset, u8 data)
{
	LOG("gouder write_m, offset: %04x, data: %02x\n", offset, data);

	// $6800-$6FFF mirrors the $4800-$4FFF PRG/mirroring register
	if (!m_submapper && (offset & 0x1800) == 0x0800)
	{
		m_reg[5] = data;
		prg32(bitswap<2>(m_reg[5], 4, 0));
		update_mirroring();
	}
}

void nes_gouder_device::write_h(offs_t offset, u8 data)
{
	txrom_write(offset, data);
	if (!m_submapper)
		update_mirroring();
}

// submapper 0: writes to 0x8000-0xffff are like MMC3 but no PRG bankswitch (it is handled by the PRG register)
// submapper 1: one 32 KiB PRG bank at $8000 selected by MMC3 register 6 / 4
void nes_gouder_device::prg_cb(int start, int bank)
{
	if (m_submapper && start == 0)
		prg32(m_mmc_prg_bank[0] >> 2);
}


/*-------------------------------------------------

 UNL-SA-9602B

 Sachen board used for Meishaonu Menggongchang III (Princess Maker 3)

 MMC3 clone with 32 KiB of battery-backed CHR-RAM and no CHR-ROM. The CHR bank
 registers ($8000 values 0-5) are PP.CCCCC: bits 4-0 select the 1 KiB CHR-RAM bank,
 bits 7-6 the 512 KiB outer PRG bank, which only applies to the switchable PRG
 banks (the fixed banks come from the first 512 KiB). The ROM has three 512 KiB
 chips' worth of address space; selecting outer bank 3 would enable two chips at
 once on the real board, so it is treated as bank 2 like FCEUX does.

 NES 2.0: mapper 513

 In MAME: Supported.

 As described by NESdev; the game has only been run as far as
 its name entry screen.

 -------------------------------------------------*/

void nes_sa9602b_device::prg_cb(int start, int bank)
{
	if (m_prg_chip == 3)
	{
		LOG("sa9602b selected PRG chip 3, which does not exist; using chip 2\n");
		m_prg_chip = 2; // assume that the higher PRG chip is mirrored twice
	}
	prg8_x(start, (m_prg_chip * 0x40) | (bank & 0x3f));

	if (m_latch & 0x40)
		prg8_89(0x3e);
	else
		prg8_cd(0x3e);

	prg8_ef(0x3f);
}


void nes_sa9602b_device::write_h(offs_t offset, uint8_t data)
{
	LOG("sa9602b write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x0000:
			m_reg = data;
			break;
		case 0x0001:
			if ((m_reg & 7) < 6)
				m_prg_chip = BIT(data, 6, 2);
			set_prg(0, m_prg_mask);
			break;
	}

	txrom_write(offset, data);
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
		return m_jumper->read();
	else
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

	offset += 0x100;
	if ((offset & 0x1100) == 0x0100)
	{
		m_reg = data & 0x03;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 UNL-A9746

 Games: Toy Story, Super 1997 4 in 1 (NT-8029)

 MMC3 clone

 NES 2.0: mapper 219 (Kasheng A9461)

 MMC3 mode (banks masked to 128 KiB) and an extended mode with
 reversed-bit PRG bank registers $23-$26 and CHR latch registers,
 plus a two-bit 128 KiB outer PRG/CHR bank written at $5002/$5003
 (power-on value $03, which makes the menu appear).

 $8002 bit 5 selects the mode: with it clear $8000/$8001 are the
 normal MMC3 bank select/data registers, with it set a write to
 $8000 or $8002 selects one of the extended PRG ($23-$26) or CHR
 ($08-$1F) registers that $8001 then writes. $A000-$FFFF are always
 the regular MMC3 registers.

 NOTE: FCEUX (a9746.c) and Nestopia (NstBoardUnlA9746) never pass
 $8000/$8001 on to the MMC3 and drop $A000/$A001; the mode bit and
 the MMC3 passthrough follow NESdev. If Toy Story or the 4-in-1
 misbehave, remove the txrom_write() calls for $8000-$8002 below.

 -------------------------------------------------*/

void nes_a9746_device::update_outer()
{
	m_prg_base = m_reg[3] << 4;
	m_chr_base = m_reg[3] << 7;
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_a9746_device::prg_cb(int start, int bank)
{
	// MMC3 mode banks are masked to 128 KiB, the outer register selects the 128 KiB block
	prg8_x(start, m_prg_base | (bank & 0x0f));
}

void nes_a9746_device::chr_cb(int start, int bank, int source)
{
	chr1_x(start, m_chr_base | (bank & 0x7f), source);
}

void nes_a9746_device::write_l(offs_t offset, u8 data)
{
	LOG("unl_a9746 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	switch (offset & 0x1003)
	{
		case 0x1002:    // $5002: outer bank bit 0
			m_reg[3] = (m_reg[3] & 0x02) | BIT(data, 0);
			update_outer();
			break;
		case 0x1003:    // $5003: outer bank bit 1
			m_reg[3] = (m_reg[3] & 0x01) | (BIT(data, 5) << 1);
			update_outer();
			break;
	}
}

void nes_a9746_device::update_banks(uint8_t value)
{
	// extended PRG registers: index $23-$26 ($2B-$2E too, bit 3 is not decoded) selects the 8 KiB bank at $E000/$C000/$A000/$8000;
	// the bank number is made of bits 5-2 of the data, in reversed order
	const u8 sel = m_reg[0];
	const u8 prg_sel = sel;
	if ((prg_sel & 0x30) == 0x20 && (prg_sel & 0x07) >= 3 && (prg_sel & 0x07) <= 6)
		prg8_x(6 - (prg_sel & 0x07), m_prg_base | bitswap<4>(value, 2, 3, 4, 5));

	// extended CHR registers
	const u8 chr_sel = sel;
	if (!BIT(chr_sel, 0) && ((chr_sel & 0x38) == 0x08 || (chr_sel & 0x30) == 0x10))
		m_reg[2] = (value & 0x07) << 4;                                                 // bank number latch: CHR A16-A14 of the next bank
	else if ((chr_sel & 0x39) == 0x09)                                                  // 1 KiB banks at $0000-$0FFF, pairs share bank bit 0
		chr1_x(BIT(chr_sel, 1, 2), m_chr_base | m_reg[2] | (value >> 1) | BIT(chr_sel, 1), m_chr_source);
	else if ((chr_sel & 0x31) == 0x11)                                                  // 1 KiB banks at $1000-$1FFF
		chr1_x(4 + BIT(chr_sel, 2, 2), m_chr_base | m_reg[2] | (value >> 1), m_chr_source);
}

void nes_a9746_device::write_h(offs_t offset, uint8_t data)
{
	LOG("unl_a9746 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6003)
	{
		case 0x0000:    // MMC3 bank select, or index of the extended register
			m_reg[0] = data;
			if (!m_ext)
				txrom_write(offset, data);
			break;

		case 0x0001:    // MMC3 bank data, or data of the selected extended register
			if (m_ext)
				update_banks(data);
			else
				txrom_write(offset, data);
			break;

		case 0x0002:    // mode register: bit 5 selects extended mode, otherwise the same as the MMC3 bank select
			m_ext = BIT(data, 5);
			m_reg[0] = data;
			if (!m_ext)
				txrom_write(offset & ~0x0002, data);
			break;

		case 0x0003:
			break;

		default:        // $A000-$FFFF are regular MMC3 registers
			txrom_write(offset, data);
			break;
	}
}



/*-------------------------------------------------

 MULTIGAME CARTS BASED ON MMC3

 -------------------------------------------------*/

/*-------------------------------------------------

 Board BMC-FK23C / BMC-FK23CA

 MMC3 clone

 8025 enhanced MMC3 (FK-xxx/BS-xxx, NES 2.0 mapper 176 submapper 1).

 These older boards are no longer used: the fk23c and fk23ca slots and the
 UNIF boards BMC-FK23C and BMC-FK23CA now load nes_8025_device with
 submapper 1 (a software list entry can name another with a submapper
 feature, as the FS005 entries do), which follows the NESdev register description (extended MMC3
 registers $8-$B, the solder pad register address, CNROM latch). The devices
 below are still compiled but no slot creates them.

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
	uint8_t mask = (0x3f >> (m_reg[0] & 0x03));

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
		prg32(BIT(m_reg[1], 1, 6));
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

void nes_fk23c_device::write_l(offs_t offset, uint8_t data)
{
	LOG("fk23c write_l, offset: %04x, data: %02x\n", offset, data);
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

void nes_fk23c_device::write_h(offs_t offset, uint8_t data)
{
	LOG("fk23c write_h, offset: %04x, data: %02x\n", offset, data);

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
					txrom_write(offset, data);
				break;

			case 0x2000:
				set_nt_mirroring(data ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;

			default:
				txrom_write(offset, data);
				break;
		}
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

 NES 2.0 mapper 176 submapper 0 images (LP-8002KB, SFC-12B) are
 emulated by nes_8025_device; this older board is only used for UNIF
 files and software list entries.

 In MAME: Partially supported.

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

 Techline XB-xxx multicarts (Super 9-in-1 XB-F808 and similar)

 A single ASIC that emulates an MMC3, an MMC1 or a VRC4 for the inner
 bank selection of each game, with three outer registers:
 $5000: CHR base (8 KiB units, bits 7-2) and ASIC mode (bits 1-0:
 0/1 = MMC3, 2 = MMC1, 3 = VRC4); $5001: PRG base (16 KiB units,
 bits 7-2); $5002 (.CCp PP.R): R = CHR-RAM mode, PP/p = inner PRG
 bank size and PRG banking mode (p = 1: NROM), CCp = inner CHR bank
 size. All of them are cleared on reset. The VRC4 mode takes its A0/A1
 from CPU A2/A3, swapped when CPU A11 is set. In NROM mode $4025 bit 3
 selects the mirroring. In CHR-RAM mode the CHR-ROM becomes the second
 half of the PRG address space, which $5001 can then reach.

 NES 2.0: mapper 351

 In MAME: Supported.

 As described by NESdev. The 8 KiB NROM bank size
 (PP = 1x) is not described well enough to know which 8 KiB half is
 selected, so the even one is used; none of the MMC1, VRC4 and CHR-RAM
 modes have been seen on more than the Super 9-in-1.

 -------------------------------------------------*/

void nes_tech9in1_device::update_outer()
{
	const bool nrom = BIT(m_reg[2], 4);
	const u8 pp = BIT(m_reg[2], 2, 2);

	// inner PRG bank size in 8 KiB units, as a mask
	if (nrom)
		m_prg_mask = pp == 0 ? 0x03 : pp == 1 ? 0x01 : 0x00;
	else
		m_prg_mask = pp == 0 ? 0x1f : 0x0f;
	m_nrom_8k = nrom && pp >= 2;
	m_prg_base = ((m_reg[1] >> 2) << 1) & ~m_prg_mask;

	// inner CHR bank size in 1 KiB units, as a mask
	const u8 c = BIT(m_reg[2], 4, 3);
	m_chr_8k = BIT(c, 2);
	m_chr_mask = m_chr_8k ? 0x00 : BIT(c, 1) ? 0x7f : BIT(c, 0) ? 0x1f : 0xff;
	m_chr_base = ((m_reg[0] >> 2) << 3) & ~m_chr_mask;
}

void nes_tech9in1_device::update_all()
{
	update_outer();
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_tech9in1_device::set_mirror(u8 data)
{
	switch (data & 0x03)
	{
		case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
		case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
		case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
	}
}

// the windows record 8 KiB banks of PRG-ROM followed by CHR-ROM; CHR-ROM is only reachable in CHR-RAM mode
void nes_tech9in1_device::prg_cb(int start, int bank)
{
	m_prg_map[start] = bank;
}

void nes_tech9in1_device::set_prg(int prg_base, int prg_mask)
{
	if (BIT(m_reg[2], 4))   // NROM mode
	{
		const int base = m_prg_base;
		for (int i = 0; i < 4; i++)
			prg_cb(i, m_nrom_8k ? base : (base | (i & m_prg_mask)));
		return;
	}

	switch (m_reg[0] & 0x03)
	{
		case 0:
		case 1:     // MMC3
			nes_txrom_device::set_prg(m_prg_base, m_prg_mask);
			break;

		case 2:     // MMC1
		{
			const int last = m_prg_mask;
			const int n = m_mmc1_reg[3] & 0x0f;
			switch (BIT(m_mmc1_reg[0], 2, 2))
			{
				case 0:
				case 1:     // 32 KiB
					for (int i = 0; i < 4; i++)
						prg_cb(i, m_prg_base | ((((n & 0x0e) << 1) + i) & m_prg_mask));
					break;
				case 2:     // first 16 KiB fixed, 16 KiB switched at $C000
					prg_cb(0, m_prg_base | 0);
					prg_cb(1, m_prg_base | 1);
					prg_cb(2, m_prg_base | (((n << 1) + 0) & m_prg_mask));
					prg_cb(3, m_prg_base | (((n << 1) + 1) & m_prg_mask));
					break;
				case 3:     // 16 KiB switched at $8000, last 16 KiB fixed
					prg_cb(0, m_prg_base | (((n << 1) + 0) & m_prg_mask));
					prg_cb(1, m_prg_base | (((n << 1) + 1) & m_prg_mask));
					prg_cb(2, m_prg_base | ((last - 1) & m_prg_mask));
					prg_cb(3, m_prg_base | (last & m_prg_mask));
					break;
			}
			break;
		}

		case 3:     // VRC4
		{
			const int last = m_prg_mask;
			prg_cb(0 ^ (m_vrc_flip ? 2 : 0), m_prg_base | (m_vrc_prg[0] & m_prg_mask));
			prg_cb(1, m_prg_base | (m_vrc_prg[1] & m_prg_mask));
			prg_cb(2 ^ (m_vrc_flip ? 2 : 0), m_prg_base | ((last - 1) & m_prg_mask));
			prg_cb(3, m_prg_base | (last & m_prg_mask));
			break;
		}
	}
}

void nes_tech9in1_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (BIT(m_reg[2], 0))   // CHR-RAM mode
	{
		chr8(0, CHRRAM);
		return;
	}

	if (m_chr_8k)
	{
		for (int i = 0; i < 8; i++)
			chr1_x(i, m_chr_base + i, CHRROM);
		return;
	}

	switch (m_reg[0] & 0x03)
	{
		case 0:
		case 1:     // MMC3
			nes_txrom_device::set_chr(CHRROM, m_chr_base, m_chr_mask);
			break;

		case 2:     // MMC1
			if (BIT(m_mmc1_reg[0], 4))  // two 4 KiB banks
			{
				for (int i = 0; i < 4; i++)
				{
					chr1_x(i, m_chr_base | (((m_mmc1_reg[1] << 2) + i) & m_chr_mask), CHRROM);
					chr1_x(4 + i, m_chr_base | (((m_mmc1_reg[2] << 2) + i) & m_chr_mask), CHRROM);
				}
			}
			else                        // one 8 KiB bank
			{
				for (int i = 0; i < 8; i++)
					chr1_x(i, m_chr_base | ((((m_mmc1_reg[1] & 0x1e) << 2) + i) & m_chr_mask), CHRROM);
			}
			break;

		case 3:     // VRC4
			for (int i = 0; i < 8; i++)
				chr1_x(i, m_chr_base | (m_vrc_chr[i] & m_chr_mask), CHRROM);
			break;
	}
}

u8 nes_tech9in1_device::read_h(offs_t offset)
{
	const u32 prg_banks = m_prg_size >> 13;
	const u32 chr_banks = m_vrom_size >> 13;
	u32 bank = m_prg_map[(offset >> 13) & 3];

	if (BIT(m_reg[2], 0) && chr_banks)  // CHR-RAM mode: PRG-ROM and CHR-ROM form one address space
		bank &= std::bit_ceil(prg_banks + chr_banks) - 1;
	else
		bank &= prg_banks - 1;

	if (bank < prg_banks)
		return m_prg[(bank << 13) | (offset & 0x1fff)];
	else if ((bank - prg_banks) < chr_banks)
		return m_vrom[((bank - prg_banks) << 13) | (offset & 0x1fff)];
	return get_open_bus();
}

void nes_tech9in1_device::write_l(offs_t offset, u8 data)
{
	LOG("tech9in1 write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	switch (offset & 0x1003)    // writes $5000-$5002, mask $F003
	{
		case 0x1000: case 0x1001: case 0x1002:
			m_reg[offset & 3] = data;
			update_all();
			break;
	}
}

void nes_tech9in1_device::write_ex(offs_t offset, u8 data)
{
	LOG("tech9in1 write_ex, offset: %04x, data: %02x\n", offset, data);

	// NROM mode: $4025 bit 3 selects the mirroring (FDS style)
	if (offset == 0x05 && BIT(m_reg[2], 4))
		set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_tech9in1_device::write_h(offs_t offset, u8 data)
{
	LOG("tech9in1 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (m_reg[0] & 0x03)
	{
		case 0:
		case 1:
			txrom_write(offset, data);
			break;
		case 2:
			mmc1_write(offset, data);
			break;
		case 3:
			vrc_write(offset, data);
			break;
	}
}

void nes_tech9in1_device::mmc1_write(offs_t offset, u8 data)
{
	if (BIT(data, 7))
	{
		m_mmc1_shift = 0;
		m_mmc1_count = 0;
		m_mmc1_reg[0] |= 0x0c;
	}
	else
	{
		m_mmc1_shift = (m_mmc1_shift >> 1) | (BIT(data, 0) << 4);
		if (++m_mmc1_count < 5)
			return;
		m_mmc1_reg[BIT(offset, 13, 2)] = m_mmc1_shift;
		m_mmc1_shift = 0;
		m_mmc1_count = 0;
	}

	switch (m_mmc1_reg[0] & 0x03)
	{
		case 0: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 1: set_nt_mirroring(PPU_MIRROR_HIGH); break;
		case 2: set_nt_mirroring(PPU_MIRROR_VERT); break;
		case 3: set_nt_mirroring(PPU_MIRROR_HORZ); break;
	}
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_tech9in1_device::vrc_write(offs_t offset, u8 data)
{
	// the ASIC takes the VRC's A0/A1 from CPU A2/A3, swapped when CPU A11 is set
	const u8 addr_lines = BIT(offset, 11) ? bitswap<2>(offset, 2, 3) : bitswap<2>(offset, 3, 2);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
			m_vrc_prg[BIT(offset, 13)] = data & 0x1f;
			set_prg(m_prg_base, m_prg_mask);
			break;
		case 0x1000:
			if (BIT(addr_lines, 1))
			{
				m_vrc_flip = BIT(data, 1);
				set_prg(m_prg_base, m_prg_mask);
			}
			else
				set_mirror(data);
			break;
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
		{
			const int bank = 2 * (BIT(offset, 12, 3) - 3) + BIT(addr_lines, 1);
			const int shift = BIT(addr_lines, 0) * 4;
			const int mask = shift ? 0x1f0 : 0x0f;
			m_vrc_chr[bank] = (m_vrc_chr[bank] & ~mask) | ((data << shift) & mask);
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;
		}
		case 0x7000:
			switch (addr_lines)
			{
				case 0:
					m_vrc_irq_latch = (m_vrc_irq_latch & 0xf0) | (data & 0x0f);
					break;
				case 1:
					m_vrc_irq_latch = (m_vrc_irq_latch & 0x0f) | ((data & 0x0f) << 4);
					break;
				case 2:     // control: any write acknowledges the IRQ and resets the prescaler
					m_vrc_irq_mode = BIT(data, 2);
					m_vrc_irq_enable = BIT(data, 1);
					m_vrc_irq_enable_latch = BIT(data, 0);
					m_vrc_irq_prescale = 341;
					if (m_vrc_irq_enable)
						m_vrc_irq_count = m_vrc_irq_latch;
					set_irq_line(CLEAR_LINE);
					break;
				case 3:     // acknowledge
					m_vrc_irq_enable = m_vrc_irq_enable_latch;
					set_irq_line(CLEAR_LINE);
					break;
			}
			break;
	}
}

void nes_tech9in1_device::vrc_irq_tick()
{
	if (m_vrc_irq_count == 0xff)
	{
		m_vrc_irq_count = m_vrc_irq_latch;
		set_irq_line(ASSERT_LINE);
	}
	else
		m_vrc_irq_count++;
}

void nes_tech9in1_device::vrc_irq_clock()
{
	if ((m_reg[0] & 0x03) != 3 || !m_vrc_irq_enable)
		return;

	if (m_vrc_irq_mode)     // cycle mode
		vrc_irq_tick();
	else                    // scanline mode: the prescaler divides the CPU cycles by 114, 114, 113
	{
		m_vrc_irq_prescale -= 3;
		if (m_vrc_irq_prescale <= 0)
		{
			m_vrc_irq_prescale += 341;
			vrc_irq_tick();
		}
	}
}

/*-------------------------------------------------

 BMC-5IN1

 Unknown Bootleg Multigame Board
 Games: 5 in 1 1993 Copyright

 NES 2.0: mapper 334

 In MAME: Supported.

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

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_8in1_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_8in1 write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 12))
	{
		// NESdev: the outer bank (bits 3-2) selects the 128 KiB PRG and CHR window in both modes
		m_chr_base = (data & 0x0c) << 5;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		if (BIT(data, 4))
		{
			m_prg_base = (data & 0x0c) << 2;
			set_prg(m_prg_base, m_prg_mask);
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

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_15in1_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_15in1 write_m, offset: %04x, data: %02x\n", offset, data);

	// the solder pad forces block 1 to block 3 (NESdev iNES mapper 205): the 15 in 1 menu writes block 1 and
	// shows a different menu depending on the block that answers
	if (data & 1)
		data |= m_jumper->read();

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

 In MAME: Supported.

 It also uses mmc3_irq.

 -------------------------------------------------*/

void nes_bmc_sbig7_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t page;
	LOG("bmc_sbig7 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x2001: // $a001 - Select 128K ROM/VROM base (0..5) or last 256K ROM/VRAM base (6)
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
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 BMC-HIK8IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Many, many multigame carts

 iNES: mapper 45

 In MAME: Supported.

 It also uses mmc3_irq. The DIP switch that selects one of eight menus
 is read at $5000-$5FFF (D0 is set when the read address has the bit for the switch position
 set) and is not fitted by default. The menu selection by an address line connected to PRG /CE
 (PRG A20, CHR A20, PRG A19 or CHR A19 disables the PRG-ROM while it is set) can be configured
 and is not connected by default.

 -------------------------------------------------*/

u8 nes_bmc_hik8_device::read_l(offs_t offset)
{
	LOG("bmc_hik8 read_l, offset: %04x\n", offset);

	u8 data = get_open_bus();
	const u8 dip = m_dip.read_safe(0);
	if (dip && offset >= 0xf00)    // $5000-$5FFF: D0 is set when address bit 4 + switch position is set
		data = (data & 0xfe) | BIT(offset + 0x100, 3 + dip);
	return data;
}

bool nes_bmc_hik8_device::prg_ce_disabled()
{
	switch (m_menu.read_safe(0))
	{
		case 1:     return BIT(m_reg[1], 7);    // PRG A20
		case 2:     return BIT(m_reg[2], 6);    // CHR A20
		case 3:     return BIT(m_reg[1], 6);    // PRG A19
		case 4:     return BIT(m_reg[2], 5);    // CHR A19
		default:    return false;
	}
}

u8 nes_bmc_hik8_device::read_h(offs_t offset)
{
	if (prg_ce_disabled())
		return get_open_bus();
	return hi_access_rom(offset);
}

void nes_bmc_hik8_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_hik8 write_m, offset: %04x, data: %02x\n", offset, data);

	nes_txrom_device::write_m(offset, data);  // registers overlay WRAM

	// NESdev: mask $F001, so only $6000-$6FFF is decoded; $6000 writes the four outer bank registers in turn (unless the
	// lock bit is set) and any write to $6001 resets them as a soft reset would, releasing the lock
	if (offset >= 0x1000)
		return;

	if (BIT(offset, 0))
	{
		m_count = 0;
		std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	}
	else if (!BIT(m_reg[3], 6))  // outer register lock bit
	{
		m_reg[m_count] = data;
		m_count = (m_count + 1) & 0x03;
	}
	else
		return;

	m_prg_base = (m_reg[2] & 0xc0) << 2 | m_reg[1];
	m_prg_mask = ~m_reg[3] & 0x3f;
	set_prg(m_prg_base, m_prg_mask);

	m_chr_base = (m_reg[2] & 0xf0) << 4 | m_reg[0];
	m_chr_mask = 0xff >> (~m_reg[2] & 0x0f);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
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

 In MAME: Supported.

 It also uses mmc3_irq.

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

 MMC3 clone with banking for multigame menu. $5000-$5FFF holds
 the outer registers; reading it returns a DIP value that advances
 on every soft reset.

 NES 2.0: mapper 411, submapper 0 (JY-201..204), submapper 1 (CHR
 A18 taken from $5001 only) and submapper 2 (registers written at
 any $5000-$5FFF address, 256 KiB PRG size bit moved to bit 0)

 In MAME: Supported.

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
	set_prg(0, 0xff);
	set_chr(m_chr_source, 0, 0xff);
}

void nes_a88s1_device::prg_cb(int start, int bank)
{
	const int prg_or = ((m_reg[1] << 1) & 0x10) | ((m_reg[1] >> 1) & 0x60);
	const int prg_and = (m_submapper == 2 ? BIT(m_reg[1], 0) : BIT(m_reg[1], 1)) ? 0x1f : 0x0f;

	if (BIT(m_reg[0], 6))   // NROM mode
	{
		const int nrom_bank = (m_reg[0] & 0x05) | ((m_reg[0] >> 2) & 0x02) | (prg_or >> 1);
		if (BIT(m_reg[0], 1))
			prg32(nrom_bank >> 1);
		else
		{
			prg16_89ab(nrom_bank);
			prg16_cdef(nrom_bank);
		}
	}
	else                    // MMC3 mode
		prg8_x(start, (bank & prg_and) | (prg_or & ~prg_and));
}

void nes_a88s1_device::chr_cb(int start, int bank, int source)
{
	const int chr_and = BIT(m_reg[1], 1) ? 0xff : 0x7f;
	int chr_or = (m_reg[1] << 5) & 0x080;
	if (m_submapper == 1)
		chr_or |= (m_reg[1] << 2) & 0x100;
	else
		chr_or |= ((m_reg[0] << 4) & 0x100) | ((m_reg[1] << 2) & 0x200);

	chr1_x(start, (bank & chr_and) | (chr_or & ~chr_and), source);
}

u8 nes_a88s1_device::read_l(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "a88s1 read_l, offset: %04x\n", offset);

	// $5000-$5FFF returns the DIP value
	return (offset >= 0x0f00) ? m_reg[2] : get_open_bus();
}

void nes_a88s1_device::write_l(offs_t offset, u8 data)
{
	LOG("a88s1 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000 && (m_submapper == 2 || BIT(offset, 11)))
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

 The outer register is the low byte of the write address
 in $5000-$5FFF. NESdev gives both mask bits (A5 for PRG, A2 for CHR) the same
 sense, 1 = 256 KiB; the PRG one used to be inverted here, as in FCEUmm, which
 has its own FIXME about a reversed mask for one game. No cart was available
 to test the change.

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
		m_prg_mask = 0x1f >> BIT(~m_reg, 5);   // NESdev: 1 = 256 KiB, like the CHR mask below
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = (m_reg & 0x07) << 7;
		m_chr_mask = 0xff >> BIT(~m_reg, 2);
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
		assert(std::has_single_bit(m_outer_prg_size));

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

 The outer register ($6000, once, locked by bit 7) is
 LTCc SBPp: PRG A19-A17 come from B, P and (with S) p; CHR A17 comes from c
 when T is set. The CHR A19 and A18 bits are taken from C (bit 5) and B
 (bit 2) respectively: NESdev's diagram has them the other way round, but
 the Mario 7 in 1 writes $CC (B set, C clear) for Super Bros. 5, whose tiles
 are at 256 KiB in its CHR-ROM; the 512 KiB the NESdev order selects gave tile
 garbage, and $C9 (Mario Is Missing) and $D2 (Mario 3) also run. FCEUmm has a
 comment that the 256 KiB CHR index bits are inverted.

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
		m_chr_base = (bitswap<3>(data, 5, 2, 4) & (0x06 | chr128k)) << 7;    // A19 = C, A18 = B (see the banner), A17 = c
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

 The outer register is taken from the low bits of the
 write address in $6000-$7FFF (..II MmOO = address bits 5-0), not from the data.
 NESdev's page shows the layout as data bits, but the 4 in 1 (411120-C) menu
 writes $00 to $6802, $6801 and $680B to start its games, so only the address
 can carry the choice; $680B (M set) is Master Fighter II, the 32 KiB game.
 With the DIP switch on, bit 2 of the outer register also selects the 32 KiB PRG mode like bit 3.

 -------------------------------------------------*/

void nes_bmc_411120c_device::prg_cb(int start, int bank)
{
	if (!prg32_mode())    // all games use MMC3 PRG banking except Master Fighter II
		nes_txrom_device::prg_cb(start, bank);
}

void nes_bmc_411120c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_411120c write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(m_wram_protect, 7))
	{
		m_reg = offset;
		if (prg32_mode())
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

 Outer register at $8080-$E080 (bits 13-14): bank 0 uses TxSROM
 mirroring (CHR A17), bank 2 is a 128 KiB PRG bank with CHR A17
 serving as PRG A17 (and unbanked CHR-RAM when CHR A17 is high),
 bank 3 replaces PRG A17 with CPU A14 while CHR A17 is low. In that
 state the upper two windows ($C000-$FFFF) repeat R6 and R7 from the second
 128 KiB half instead of using the MMC3's fixed last banks: the menu's stage 2
 code builds its NMI trampoline in RAM and expects the NMI vector in R7's bank
 at $E000 (NMI at $809C). CHR A17 is bit 7 of the MMC3 bank number serving the
 window the PPU is fetching from, so PRG A17 follows every pattern fetch.

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_810305c_device::prg_cb(int start, int bank)
{
	const bool chr_a17 = m_live_a17;

	if (m_outer == 2 && chr_a17)
		bank |= 0x10;                                   // CHR A17 serves as PRG A17
	else if (m_outer == 3 && !chr_a17)
	{
		// CPU A14 serves as PRG A17: $8000-$BFFF come from the first 128 KiB half and $C000-$FFFF from the second,
		// and the upper half is not fixed to the MMC3's last two banks but repeats R6 and R7 (the stage 2 code of
		// the menu expects its NMI vector there)
		if (start >= 2)
			bank = m_prg_base | 0x10 | (m_mmc_prg_bank[start - 2] & 0x0f);
		else
			bank &= ~0x10;
	}

	prg8_x(start, bank);
}

void nes_bmc_810305c_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	// CHR A17 of each 1 KiB window is bit 7 of the MMC3 bank number that serves it
	const u8 chr_page = (m_latch & 0x80) >> 5;
	const u16 banks[8] = {
		u16(m_mmc_vrom_bank[0] & ~0x01), u16(m_mmc_vrom_bank[0] | 0x01), u16(m_mmc_vrom_bank[1] & ~0x01), u16(m_mmc_vrom_bank[1] | 0x01),
		m_mmc_vrom_bank[2], m_mmc_vrom_bank[3], m_mmc_vrom_bank[4], m_mmc_vrom_bank[5]
	};
	m_slot_a17 = 0;
	for (int i = 0; i < 8; i++)
		m_slot_a17 |= BIT(banks[i], 7) << (chr_page ^ i);

	if (m_outer)
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
	else
		nes_txsrom_device::set_chr(chr, chr_base, chr_mask);

	update_live_a17(BIT(m_slot_a17, m_last_slot));
}

void nes_bmc_810305c_device::chr_cb(int start, int bank, int source)
{
	if (m_outer == 2 && BIT(m_slot_a17, start))
		chr1_x(start, start, CHRRAM);    // CHR A17 high selects the unbanked 8 KiB of CHR-RAM
	else
		nes_txrom_device::chr_cb(start, bank, source);
}

u8 nes_bmc_810305c_device::chr_r(offs_t offset)
{
	// PRG A17 follows CHR A17, which changes with the window of every pattern fetch
	m_last_slot = BIT(offset, 10, 3);
	update_live_a17(BIT(m_slot_a17, m_last_slot));

	return nes_txsrom_device::chr_r(offset);
}

void nes_bmc_810305c_device::update_live_a17(bool a17)
{
	if (a17 != m_live_a17)
	{
		m_live_a17 = a17;
		if (m_outer == 2 || m_outer == 3)
			set_prg(m_prg_base, m_prg_mask);
	}
}

void nes_bmc_810305c_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_810305c write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 7))    // outer register
	{
		m_outer = BIT(offset, 13, 2);

		m_prg_base = m_outer << 5;
		m_prg_mask = 0x1f >> ((m_outer == 2) ? 1 : 0);
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = m_outer << 7;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else if (m_outer)      // standard MMC3 registers
		nes_txrom_device::write_h(offset, data);
	else                   // TxSROM MMC3 registers without mirroring bit
		nes_txsrom_device::write_h(offset, data);
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

 The outer register at $6000-$7FFF (mask probably $E000 per NESdev) only
 responds when PRG-RAM is enabled and writable ($A001 = $80-$BF). Outer bank 3 selects
 the GNROM-like mode: MMC3 banks for $8000-$BFFF with bit 1
 cleared, and for $C000-$FFFF with bit 1 set.

 In MAME: Preliminary.

 Supported.

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

void nes_bmc_830832c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_830832c write_m, offset: %04x, data: %02x\n", offset, data);

	// NESdev: the outer bank register decodes with mask $E000, so all of $6000-$7FFF writes it
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

void nes_bmc_yy841101c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_yy841101c write_m, offset: %04x, data: %02x\n", offset, data);
	if (offset & 0x1000)    // games only write 0x7000, this mask is a guess
	{
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

void nes_bmc_yy841155c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_yy841155c write_m, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x1000)
	{
		m_reg[offset & 1] = data;

		m_prg_base = (m_reg[1] & 0x01) << 5 | (m_reg[0] & 0x40) >> 2;
		if (BIT(m_reg[0], 7))    // NROM mode
		{
			u8 bank = m_prg_base >> 1 | (m_reg[0] & 0x07);
			u8 mode = BIT(m_reg[0], 5);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
		else                     // MMC3 mode
			set_prg(m_prg_base, m_prg_mask);

		m_chr_base = m_prg_base << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 TEC9719 (mapper 126), BS-300 / ING003C family (mapper 422) and
 ING-022 (mapper 534) enhanced MMC3 ASICs

 Games: PowerJoy 84-in-1 (126); many multicarts (422); Atari
 Flashback Mini 7800, Intellivision 25-in-1 and others (534)

 Four outer registers overlay WRAM at $6000-$7FFF (writable only
 while $A001 bit 7 is set): $6000 PRG/CHR outer banks and sizes,
 $6001 SL0 menu input / mirroring mode / PRG chip select,
 $6002 CNROM CHR bank and bit locks, $6003 banking mode (MMC3,
 NROM-128/256, UNROM, ANROM) and register lock. The ASIC addresses
 up to 4 MiB of PRG-ROM and repurposes high PRG address lines as
 CHR lines: mapper 126 swaps CHR A18/A19 compared to 422 and 534.
 Mapper 534 also inverts the data written to the MMC3 IRQ latch
 at $C000. NES 2.0 submapper 1 uses PRG A21 as chip select
 between two 1 MiB PRG chips, submapper 2 uses $6001 bit 2, submapper 3
 substitutes PRG A14 and CHR A14 with $6000 bit 5 (when $6000 bit 2 is
 set) and submapper 4 (LD822 PCB) wires CHR A20..A18 to PRG A20..A18.
 The banking follows FCEUmm's 126_422_534.c, which also lists some old
 dumps that need their PRG/CHR halves swapped (not handled here).

 NES 2.0: mappers 126, 422 and 534

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_pjoy84_device::get_mmc3_bank(int bank) const
{
	if (!(bank & 1) && BIT(m_latch, 6))
		bank ^= 2;
	return (bank & 2) ? (0xfe | (bank & 1)) : u8(m_mmc_prg_bank[bank & 1]);
}

void nes_pjoy84_device::update_prg()
{
	int prg_and = BIT(m_reg[0], 6) ? 0x0f : 0x1f;     // 128 or 256 KiB inner PRG bank
	int prg_or = (((m_reg[0] << 4) & 0x070) | (((m_reg[0] ^ 0x20) << 3) & 0x180)) & ~prg_and;     // outer PRG bank
	if (m_submapper == 1)
		prg_or = (prg_or & 0x7f) | ((prg_or >> 1) & 0x80);      // PRG A21 selects between two 1 MiB chips
	if (m_submapper == 2)
		prg_or = (prg_or & 0x7f) | ((m_reg[1] << 5) & 0x80);    // $6001 bit 2 selects between two 1 MiB chips
	if (m_submapper == 3 && BIT(m_reg[0], 2))
	{
		// PRG A14 is replaced by PRG A21
		prg_and &= ~0x02;
		prg_or |= BIT(m_reg[0], 5) ? 0x00 : 0x02;
	}

	// In UNROM-like mode (CT3=1, CT2=1, CT0=1) the MMC3 sees A13=0 and A14=CPU A14 during reads, making register 6 apply
	// from $8000-$BFFF and the fixed bank from $C000-$FFFF. In NROM-128, NROM-256, ANROM and UNROM modes (CT0=1) it sees
	// A13=0 and A14=0, making register 6 apply from $8000-$FFFF.
	const int page_mask = ((m_reg[3] & 0x0d) == 0x0d) ? 2 : BIT(m_reg[3], 0) ? 0 : 3;

	for (int page = 0; page < 4; page++)
	{
		u8 v = get_mmc3_bank(page & page_mask);

		// UNROM and ANROM modes mean that MMC3 register 6 selects 16 and 32 KiB rather than 8 KiB banks
		if (BIT(m_reg[3], 3))
		{
			switch (m_reg[3] & 0x03)
			{
				case 0: v = (v & 3) | ((v << 1) & ~3); break;       // PRG A14 appears twice
				case 1: v = (page & 3) | ((v << 1) & ~1); break;    // 16 KiB mode, bit 0 OR'd with CPU A14
				case 2: v = (v & 3) | ((v << 2) & ~3); break;       // PRG A13 and PRG A14 appear twice
				case 3: v = (page & 3) | ((v << 2) & ~3); break;    // 32 KiB mode
			}
		}
		else if (BIT(m_reg[3], 0))  // regular NROM modes
		{
			v = (page & 1) | (v & ~1);
			if (BIT(m_reg[3], 1))
				v = (page & 2) | (v & ~2);                          // NROM-256
		}

		prg8_x(page, (v & prg_and) | (prg_or & ~prg_and));
	}
}

void nes_pjoy84_device::update_chr()
{
	int chr_and = BIT(m_reg[0], 7) ? 0x7f : 0xff;     // 128 or 256 KiB inner CHR bank
	int chr_or;                                         // outer CHR bank
	if (m_mapper == 126)    // mapper 126 swaps CHR A18 and A19
		chr_or = (((m_reg[0] << 4) & 0x080) | (((m_reg[0] ^ 0x20) << 3) & 0x100) | ((m_reg[0] << 5) & 0x200)) & ~chr_and;
	else if (m_submapper == 4)  // LD822 PCB: CHR A20..A18 = PRG A20..A18
		chr_or = (((m_reg[0] << 4) & 0x080) | ((m_reg[0] << 7) & 0x100) | ((~m_reg[0] << 4) & 0x200) | ((m_reg[0] << 6) & 0x400)) & ~chr_and;
	else
		chr_or = ((((m_reg[0] ^ 0x20) << 4) & 0x380) | ((m_reg[0] << 8) & 0x400)) & ~chr_and;
	if (m_submapper == 3 && BIT(m_reg[0], 2))
	{
		// CHR A14 is replaced by PRG A21
		chr_and &= ~0x10;
		chr_or |= BIT(m_reg[0], 5) ? 0x00 : 0x10;
	}

	if (BIT(m_reg[3], 4))   // CNROM mode: the 8 KiB inner CHR bank comes from outer bank register #2
	{
		chr8((m_reg[2] & (chr_and >> 3)) | ((chr_or & ~chr_and) >> 3), m_chr_source);
		return;
	}

	const u8 chr_page = (m_latch & 0x80) >> 5;
	chr1_x(chr_page ^ 0, ((m_mmc_vrom_bank[0] & ~0x01) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 1, ((m_mmc_vrom_bank[0] |  0x01) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 2, ((m_mmc_vrom_bank[1] & ~0x01) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 3, ((m_mmc_vrom_bank[1] |  0x01) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 4, (u8(m_mmc_vrom_bank[2]) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 5, (u8(m_mmc_vrom_bank[3]) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 6, (u8(m_mmc_vrom_bank[4]) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 7, (u8(m_mmc_vrom_bank[5]) & chr_and) | chr_or, m_chr_source);
}

void nes_pjoy84_device::update_mirroring()
{
	if (BIT(m_reg[3], 5))   // ANROM mirroring from MMC3 register 6 bit 4
		set_nt_mirroring(BIT(m_mmc_prg_bank[0], 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	else if (BIT(m_reg[1], 1))  // extended mirroring
	{
		switch (m_mmc_mirror & 3)
		{
			case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
			case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
			case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
		}
	}
	else
		set_nt_mirroring(BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_pjoy84_device::update_banks()
{
	update_prg();
	update_chr();
	update_mirroring();
}

void nes_pjoy84_device::prg_cb(int start, int bank)
{
	update_prg();
}

void nes_pjoy84_device::chr_cb(int start, int bank, int source)
{
	update_chr();
}

u8 nes_pjoy84_device::read_m(offs_t offset)
{
	// WRAM stays connected regardless of the MMC3 WRAM enable bit
	return device_nes_cart_interface::read_m(offset);
}

u8 nes_pjoy84_device::read_h(offs_t offset)
{
	// $6001 bit 0: PRG A0 comes from the SL0 input (menu selection)
	if (BIT(m_reg[1], 0))
		offset = (offset & ~1) | BIT(m_sl0, 0);
	return hi_access_rom(offset);
}

void nes_pjoy84_device::write_m(offs_t offset, u8 data)
{
	LOG("pjoy84 write_m, offset: %04x, data: %02x\n", offset, data);

	device_nes_cart_interface::write_m(offset, data);   // clearing the MMC3 WRAM bit does not disable WRAM itself

	// the outer registers can only be written while the MMC3 WRAM bit is set
	if (!BIT(m_wram_protect, 7))
		return;

	const u8 reg = offset & 0x03;
	if (reg == 2)
	{
		// CNROM bank (D0-D3), bank enable (D4-D6) and bank enable lock (D7)
		const int latch_mask = 0xff & ~(BIT(m_reg[2], 7) ? 0xf0 : 0x00) & ~((m_reg[2] >> 3) & 0x0e);
		m_reg[2] = (m_reg[2] & ~latch_mask) | (data & latch_mask);
		update_chr();
	}
	else if (!BIT(m_reg[3], 7))
	{
		// lock bit clear: update any other outer bank register
		m_reg[reg] = data;
		update_banks();
	}
}

void nes_pjoy84_device::write_h(offs_t offset, u8 data)
{
	LOG("pjoy84 write_h, offset: %04x, data: %02x\n", offset, data);

	if ((m_reg[3] & 0x09) == 0x09)
		txrom_write(1, data);       // UNROM and ANROM modes treat all writes to $8000-$FFFF as if they were going to $8001
	else
	{
		if (offset >= 0x4000 && m_mapper == 534)
			data ^= 0xff;           // mapper 534 inverts the MMC3 scanline counter reload value
		txrom_write(offset, data);
	}
	update_banks();
}

/*-------------------------------------------------

 COOLBOY, MINDKIDS, and other AA6023 / SMD132 / SMD133 boards

 Games: many multicarts, Chinese games, and some
 modern re-releases such as Metal Storm and Holy Diver

 MMC3 clone with six outer registers. NES 2.0 submapper bit 0
 selects the solder pad (odd: registers at $5000-$5FFF, even: registers
 overlay WRAM at $6000-$6FFF); submapper bits 1-3 select the revision /
 PCB wiring: 0/1 original implementation (CHR-RAM only, or mixed
 CHR-ROM/RAM selected by register 4), 2/3 later revision with a different
 arrangement of register 1, 4/5 LD622D (PRG A20-21 moved to register 0),
 6/7 J-852C (CHR A17 selects between two PRG chips), 8/9 2 MiB regular plus
 an extra latch and write-protectable CHR-RAM, 10/11 single-screen mirroring
 controlled by register 0.

 NES 2.0: mapper 268, submappers 0-11

 In MAME: Supported.

 -------------------------------------------------*/

void nes_smd133_device::prg_cb(int start, int bank)
{
	const int prg_mask_mmc3 = (BIT(m_reg[3], 4) ? 0x00 : 0x0f)
			| (BIT(m_reg[0], 6) ? 0x00 : 0x10)
			| (BIT(m_reg[1], 7) ? 0x00 : 0x20)
			| (BIT(m_reg[1], 6) ? 0x40 : 0x00)
			| (BIT(m_reg[1], 5) ? 0x80 : 0x00);
	int prg_mask_gnrom = 0;
	int prg_offset = 0;

	switch (m_submapper & ~1)
	{
		default:    // original AA6023
			prg_mask_gnrom = BIT(m_reg[3], 4) ? (BIT(m_reg[1], 1) ? 0x03 : 0x01) : 0;
			prg_offset = (m_reg[3] & 0x0e) | ((m_reg[0] << 4) & 0x070) | ((m_reg[1] << 3) & 0x080)
					| ((m_reg[1] << 6) & 0x300) | ((m_reg[0] << 6) & 0xc00) | ((~m_reg[1] << 12) & 0x1000);
			break;

		case 2:     // later revision with a different arrangement of register 1
			prg_mask_gnrom = BIT(m_reg[3], 4) ? (BIT(m_reg[1], 4) ? 0x01 : 0x03) : 0;
			prg_offset = (m_reg[3] & 0x0e) | ((m_reg[0] << 4) & 0x070) | ((m_reg[1] << 4) & 0x080)
					| ((m_reg[1] << 6) & 0x100) | ((m_reg[1] << 8) & 0x200) | ((m_reg[0] << 6) & 0xc00);
			break;

		case 4:     // LD622D: PRG A20-21 moved to register 0
			prg_mask_gnrom = BIT(m_reg[3], 4) ? (BIT(m_reg[1], 1) ? 0x03 : 0x01) : 0;
			prg_offset = (m_reg[3] & 0x0e) | ((m_reg[0] << 4) & 0x070) | ((m_reg[0] << 3) & 0x180);
			break;

		case 6:     // J-852C: CHR A17 (or bit 3 of register 0) selects one of two PRG chips
		{
			prg_mask_gnrom = BIT(m_reg[3], 4) ? (BIT(m_reg[1], 1) ? 0x03 : 0x01) : 0;
			prg_offset = (m_reg[3] & 0x0e) | ((m_reg[0] << 4) & 0x070) | ((m_reg[1] << 3) & 0x080)
					| ((m_reg[1] << 6) & 0x300) | ((m_reg[0] << 6) & 0xc00);
			const int chip_size = (m_prg_size / 0x2000) / 2;
			prg_offset &= chip_size - 1;
			if (BIT(m_reg[0], 7) ? BIT(m_reg[0], 3) : BIT(m_mmc_vrom_bank[0], 7))
				prg_offset |= chip_size;
			break;
		}

		case 8:     // 2 MiB regular plus extra latch (register 6)
			prg_mask_gnrom = BIT(m_reg[3], 4) ? (BIT(m_reg[1], 1) ? 0x03 : 0x01) : 0;
			prg_offset = (m_reg[3] & 0x0e) | ((m_reg[0] << 4) & 0x070) | ((m_reg[1] << 3) & 0x080)
					| ((m_reg[6] << 8) & 0x300) | ((m_reg[6] << 6) & 0xc00);
			break;
	}

	prg_offset &= ~(prg_mask_mmc3 | prg_mask_gnrom);
	prg8_x(start, (bank & prg_mask_mmc3) | prg_offset | (start & prg_mask_gnrom));
}

void nes_smd133_device::chr_cb(int start, int bank, int source)
{
	const int chr_mask_mmc3 = BIT(m_reg[3], 4) ? 0x00 : BIT(m_reg[0], 7) ? 0x7f : 0xff;
	const int chr_mask_gnrom = BIT(m_reg[3], 4) ? 0x07 : 0x00;
	int chr_offset = ((m_reg[0] << 9) & 0xc00) | ((m_reg[0] << 4) & 0x380) | ((m_reg[2] << 3) & 0x078);
	chr_offset &= ~(chr_mask_mmc3 | chr_mask_gnrom);

	// mixed CHR-ROM/RAM boards: a bank matching register 4 selects CHR-RAM
	const bool mixed_ram = !m_vram.empty() && m_vrom && BIT(m_reg[4], 0) && ((bank & 0xfe) == (m_reg[4] & 0xfe));
	chr1_x(start, (bank & chr_mask_mmc3) | chr_offset | (start & chr_mask_gnrom), mixed_ram ? CHRRAM : source);
}

void nes_smd133_device::update_mirroring()
{
	if ((m_submapper & ~1) == 10 && !BIT(m_reg[0], 5))
		set_nt_mirroring(BIT(m_reg[0], 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);   // single-screen mirroring
	else
		set_nt_mirroring(BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_smd133_device::update_banks()
{
	set_prg(0, 0xff);
	set_chr(m_chr_source, 0, 0xff);
	update_mirroring();
}

u8 nes_smd133_device::read_m(offs_t offset)
{
	return (m_wram_protect & 0xa0) ? device_nes_cart_interface::read_m(offset) : get_open_bus();
}

void nes_smd133_device::smd133_write(offs_t offset, u8 data)
{
	const int index = offset & 0x07;
	if (index > 5)
		return;
	if ((m_reg[3] & 0x90) == 0x80 && index != 2)    // register lock; it does not work in GNROM mode (register 3 bit 4)
		return;

	if (index == 2)
	{
		// bit 7 locks the upper nibble, bits 4-6 lock individual lower bits
		if (BIT(m_reg[2], 7))
			data = (data & 0x0f) | (m_reg[2] & 0xf0);
		data &= ((~m_reg[2] >> 3) & 0x0e) | 0xf1;
	}

	if ((m_submapper & ~1) == 8 && index == 1 && (data & 0x0c) == 0x0c)
		m_reg[6] = data;

	m_reg[index] = data;
	update_banks();
}

void nes_smd133_device::write_l(offs_t offset, u8 data)
{
	LOG("smd133 write_l, offset: %04x, data: %02x\n", offset, data);

	if (m_smd133_addr == 0x5000 && offset >= 0x0f00)
		smd133_write(offset, data);
}

void nes_smd133_device::write_m(offs_t offset, u8 data)
{
	LOG("smd133 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((BIT(m_wram_protect, 7) && !BIT(m_wram_protect, 6)) || BIT(m_wram_protect, 5))
		device_nes_cart_interface::write_m(offset, data);   // registers overlay, but are independent of, WRAM

	// the registers overlay WRAM only with the $6000 solder pad setting: even submappers on iNES images, the
	// smd133-addr feature on software list entries ($6000-$6FFF, or $7000-$7FFF on submapper 2, NESdev)
	const offs_t base = (m_submapper == 2) ? 0x1000 : 0x0000;
	if (m_smd133_addr == 0x6000 && offset >= base && offset < base + 0x1000)
		smd133_write(offset, data);
}

void nes_smd133_device::write_h(offs_t offset, u8 data)
{
	txrom_write(offset, data);
	update_mirroring();     // submappers 10/11 override the MMC3 mirroring register
}

void nes_smd133_device::chr_w(offs_t offset, u8 data)
{
	// submapper 8/9: bit 4 of register 0 write-protects the main CHR-RAM (not the extra CHR-RAM of mixed CHR-ROM/RAM boards)
	if ((m_submapper & ~1) == 8 && BIT(m_reg[0], 4) && !m_vrom)
		return;
	device_nes_cart_interface::chr_w(offset, data);
}

/*-------------------------------------------------

 8025 enhanced MMC3 ASIC

 Games: Waixing 2005+ re-releases and 245-in-1 Real Game (FS005/FS006),
 Super Mario 160-in-1 Funny Time (JX9003B), GameStar Smart Genius Deluxe,
 Game 500-in-1 (HST-162)

 MMC3 clone with outer registers at $5xx0-$5xx3 (mask selected by a solder
 pad, see the config port) for PRG/CHR banking modes and outer banks, up to
 32 MiB of PRG, optional extended MMC3 mode (RAMBO-1 style 1 KiB CHR banks and
 four selectable PRG banks), UNROM/CNROM latches and several submapper
 specific upper address lines:
 - submapper 0 (LP-8002KB, SFC-12B): 6 MMC3 PRG bits, CHR-ROM/CHR-RAM selected by $5xx0.5
 - submapper 1 (FK-xxx/BS-xxx): 8 MMC3 PRG bits, extended MMC3 mode, CNROM CHR latch
 - submapper 2 (FS005): PRG A21-A25 from $5xx0/$5xx2, RAM configuration
   register at $A001 (32 KiB WRAM, mixed CHR-ROM/RAM, single-screen
   mirroring, outer registers can be disabled; used for copy protection)
 - submapper 3 (JX9003B): PRG/CHR MSB registers at $5xx5/$5xx6
 - submapper 4: PRG A21 from $5xx2 bit 7
 - submapper 5 (HST-162): PRG MSB at $4800, $5xx1 only supplies A14-A18

 NES 2.0: mapper 176, submappers 0-5. The older BMC-SUPER24IN1SC03 and
 BMC-FK23C/FK23CA boards above are only used for UNIF files and software
 list entries.

 In MAME: Supported.

 Checked against the NESdev register description.

 -------------------------------------------------*/

nes_8025_device::nes_8025_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_8025_device(mconfig, NES_8025, tag, owner, clock)
{
}

nes_8025_device::nes_8025_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock)
	, m_config(*this, "CONFIG")
	, m_submapper(0)
	, m_mode(0)
	, m_prg_lsb(0)
	, m_chr_lsb(0)
	, m_ext(0)
	, m_prg_msb(0)
	, m_chr_msb(0)
	, m_latch_data(0)
{
}

void nes_8025_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_submapper));
	save_item(NAME(m_mode));
	save_item(NAME(m_prg_lsb));
	save_item(NAME(m_chr_lsb));
	save_item(NAME(m_ext));
	save_item(NAME(m_prg_msb));
	save_item(NAME(m_chr_msb));
	save_item(NAME(m_latch_data));
}

void nes_8025_device::pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted)
{
	if (m_submapper == 2)
	{
		// FS005: one 32 KiB SRAM (four 8 KiB banks), battery-backed when the cart has a battery, and 8 KiB CHR-RAM
		if (m_battery.empty())
		{
			if (m_prgram.size() < 0x8000)
				prgram_alloc(0x8000);
		}
		else if (m_battery.size() < 0x8000)
			battery_alloc(0x8000);
		if (m_vram.size() < 0x2000)
			vram_alloc(0x2000);
	}
	nes_txrom_device::pcb_start(machine, ciram_ptr, cart_mounted);
}

void nes_8025_device::pcb_reset()
{
	m_mode = m_prg_lsb = m_chr_lsb = m_ext = m_prg_msb = m_chr_msb = m_latch_data = 0;
	mmc3_common_initialize(0xff, 0xff, 0);

	// power-on values of the MMC3 registers: R6 = $00, R7 = $01, R8 = $FE, R9 = $FF, R10 = R11 = $FF
	m_mmc_prg_bank[0] = 0x00;
	m_mmc_prg_bank[1] = 0x01;
	m_mmc_vrom_bank[6] = 0xff;
	m_mmc_vrom_bank[7] = 0xff;

	if (m_submapper == 2)
		m_wram_protect = 0x00;  // RAM configuration register powers on cleared

	update_banks();
}

int nes_8025_device::mmc3_prg_bank(int page, bool ext) const
{
	switch ((page & 1) ? page : (page ^ (BIT(m_latch, 6) ? 2 : 0)))     // PRG A14 inversion only swaps $8000 and $C000
	{
		case 0:  return m_mmc_prg_bank[0] & 0xff;
		case 1:  return m_mmc_prg_bank[1] & 0xff;
		case 2:  return ext ? (m_mmc_prg_bank[2] & 0xff) : 0xfe;
		default: return ext ? (m_mmc_prg_bank[3] & 0xff) : 0xff;
	}
}

int nes_8025_device::prg_bank(int page) const
{
	const bool ext = ext_mode();
	u8 mode = ext ? 0 : (m_mode & 0x07);    // the PRG banking mode is ignored in extended MMC3 mode
	if (mode > 5)
		mode = 0;                           // modes 6 and 7 are never used

	// outer PRG bank in 8 KiB units (bit 0 = A13); the PRG base register supplies A14 and up
	int outer = (m_prg_lsb & 0x7f) << 1;
	switch (m_submapper)
	{
		case 2:
			// A21 = $5xx0.3, A22 = $5xx0.7, A23 = $5xx2.6, A24 = $5xx2.7, A25 = $5xx2.5
			outer |= (BIT(m_mode, 3) << 8) | (BIT(m_mode, 7) << 9) | (BIT(m_chr_lsb, 6) << 10) | (BIT(m_chr_lsb, 7) << 11) | (BIT(m_chr_lsb, 5) << 12);
			break;
		case 3:
			outer |= (m_prg_msb & 0x0f) << 8;               // A24-A21
			break;
		case 4:
			outer |= BIT(m_chr_lsb, 7) << 8;                // A21
			break;
		case 5:
			outer = ((m_prg_lsb & 0x1f) << 1) | ((m_prg_msb & 0x3f) << 6);     // $5xx1 = A18-A14, $4800 = A24-A19
			break;
		default:
			break;
	}

	switch (mode)
	{
		default:
		case 0:
		case 1:
		case 2:
		{
			// MMC3 PRG modes: 2 MiB (submappers 1 and 3 only) / 512 KiB, 256 KiB or 128 KiB inner banks
			int mask;
			if (ext)
				mask = 0xff;
			else if (mode == 0)
				mask = (m_submapper == 1 || m_submapper == 3) ? 0xff : 0x3f;
			else
				mask = (mode == 1) ? 0x1f : 0x0f;
			return (mmc3_prg_bank(page, ext) & mask) | (outer & ~mask);
		}

		case 3:     // NROM-128: 16 KiB at $8000-$BFFF mirrored at $C000-$FFFF
			return (outer & ~1) | (page & 1);

		case 4:     // NROM-256: 32 KiB at $8000-$FFFF
			return (outer & ~3) | page;

		case 5:     // UNROM: 16 KiB at $8000-$BFFF from the latch, inner bank #7 at $C000-$FFFF
		{
			const int latch = (page < 2) ? (m_latch_data & 0x07) : 0x07;
			return ((((outer >> 1) & ~0x07) | latch) << 1) | (page & 1);
		}
	}
}

void nes_8025_device::set_prg(int prg_base, int prg_mask)
{
	for (int page = 0; page < 4; page++)
		prg8_x(page, prg_bank(page));
}

void nes_8025_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	u8 source = chr;
	if (m_submapper <= 1 && BIT(m_mode, 5) && !m_vram.empty() && m_vrom_chunks)
		source = CHRRAM;    // boards with both CHR-ROM and CHR-RAM (e.g. SFC-12B) select between them with $5xx0.5
	if (source == CHRRAM && m_vram.empty())
		source = CHRROM;
	if (source == CHRROM && !m_vrom_chunks)
		source = CHRRAM;

	// FS005: the RAM configuration register can make the first 8 KiB of the CHR address space CHR-RAM (like mapper 195)
	const bool ram_low = ram_cfg() && BIT(m_wram_protect, 2) && !m_vram.empty();
	auto put = [this, source, ram_low] (int slot, int bank)
	{
		chr1_x(slot, bank, (ram_low && bank < 8) ? CHRRAM : source);
	};

	if (!BIT(m_mode, 6))
	{
		// MMC3 CHR mode: 256 KiB ($5xx0.4 = 0) or 128 KiB inner banks, CHR base register supplies the upper bits
		const int chr_and = BIT(m_mode, 4) ? 0x7f : 0xff;
		int outer = m_chr_lsb << 3;                         // CHR A20-A13 in 1 KiB units
		if (m_submapper == 3)
			outer |= (m_chr_msb & 0x0f) << 11;              // CHR A24-A21

		const bool ext = ext_mode();
		const int page = (m_latch & 0x80) >> 5;
		int bank[8];
		if (ext)
		{
			// extended MMC3 mode: six 1 KiB banks plus R10 and R11
			bank[0] = m_mmc_vrom_bank[0]; bank[1] = m_mmc_vrom_bank[6];
			bank[2] = m_mmc_vrom_bank[1]; bank[3] = m_mmc_vrom_bank[7];
		}
		else
		{
			bank[0] = m_mmc_vrom_bank[0] & ~0x01; bank[1] = m_mmc_vrom_bank[0] | 0x01;
			bank[2] = m_mmc_vrom_bank[1] & ~0x01; bank[3] = m_mmc_vrom_bank[1] | 0x01;
		}
		bank[4] = m_mmc_vrom_bank[2];
		bank[5] = m_mmc_vrom_bank[3];
		bank[6] = m_mmc_vrom_bank[4];
		bank[7] = m_mmc_vrom_bank[5];

		for (int i = 0; i < 8; i++)
			put(page ^ i, (bank[i] & chr_and) | (outer & ~chr_and));
	}
	else
	{
		// CHR A10-A12 come from the PPU: an 8 KiB bank selected by the CHR base register (and the CNROM latch on submapper 1)
		int bank8 = m_chr_lsb;
		if (m_submapper == 1 && !BIT(m_mode, 5))
			bank8 = BIT(m_mode, 4) ? ((m_chr_lsb & ~0x01) | (m_latch_data & 0x01)) : ((m_chr_lsb & ~0x03) | (m_latch_data & 0x03));
		if (m_submapper == 3)
			bank8 |= (m_chr_msb & 0x0f) << 8;

		for (int i = 0; i < 8; i++)
			put(i, (bank8 << 3) | i);
	}
}

void nes_8025_device::update_mirroring()
{
	if (ram_cfg() && BIT(m_mmc_mirror, 1))      // single-screen mirroring is only available when the RAM configuration register is enabled
		set_nt_mirroring(BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	else
		set_nt_mirroring(BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_8025_device::update_banks()
{
	set_prg(0, 0);
	set_chr(m_chr_source, 0, 0);
	update_mirroring();
}

u8 nes_8025_device::read_l(offs_t offset)
{
	const u16 addr = offset + 0x4100;
	std::vector<u8> &ram = wram();
	if (addr >= 0x5000 && !regs_enabled() && !ram.empty())
		return ram[(0x4000 + 0x1000 + (addr & 0x0fff)) & (ram.size() - 1)];
	return get_open_bus();
}

void nes_8025_device::write_l(offs_t offset, u8 data)
{
	LOG("8025 write_l, offset: %04x, data: %02x\n", offset, data);

	const u16 addr = offset + 0x4100;

	// submapper 5: PRG base MSB at $4800-$4FFF (mask $F800)
	if (m_submapper == 5 && (addr & 0xf800) == 0x4800)
	{
		m_prg_msb = data & 0x3f;
		update_banks();
		return;
	}

	if (addr < 0x5000)
		return;

	if (!regs_enabled())
	{
		// $5000-$5FFF is the second half of WRAM bank 2 while the outer registers are disabled (FS005 copy protection)
		std::vector<u8> &ram = wram();
		if (!ram.empty())
			ram[(0x4000 + 0x1000 + (addr & 0x0fff)) & (ram.size() - 1)] = data;
		return;
	}

	// the solder pad selects which of the address lines A4-A11 must be set to access the registers
	if (!BIT(addr, 4 + (m_config.read_safe(0) & 0x07)))
		return;

	switch (addr & (m_submapper == 3 ? 0x07 : 0x03))
	{
		case 0:     // mode register
			m_mode = data;
			break;
		case 1:     // PRG base register (LSB)
			m_prg_lsb = data;
			break;
		case 2:     // CHR base register (LSB)
			m_chr_lsb = data;
			break;
		case 3:     // extended mode register (submappers 1 and 2)
			if (m_submapper != 1 && m_submapper != 2)
				return;
			m_ext = data;
			break;
		case 5:     // PRG base register (MSB, submapper 3)
			if (m_submapper != 3)
				return;
			m_prg_msb = data;
			break;
		case 6:     // CHR base register (MSB, submapper 3)
			if (m_submapper != 3)
				return;
			m_chr_msb = data;
			break;
		default:
			return;
	}
	update_banks();
}

u8 nes_8025_device::read_m(offs_t offset)
{
	if (ram_cfg())
	{
		std::vector<u8> &ram = wram();
		if (!BIT(m_wram_protect, 7) || ram.empty())
			return get_open_bus();
		return ram[(((m_wram_protect & 0x03) * 0x2000) + offset) & (ram.size() - 1)];
	}
	return nes_txrom_device::read_m(offset);
}

void nes_8025_device::write_m(offs_t offset, u8 data)
{
	LOG("8025 write_m, offset: %04x, data: %02x\n", offset, data);

	if (ram_cfg())
	{
		std::vector<u8> &ram = wram();
		if (BIT(m_wram_protect, 7) && !ram.empty())
			ram[(((m_wram_protect & 0x03) * 0x2000) + offset) & (ram.size() - 1)] = data;
		return;
	}
	nes_txrom_device::write_m(offset, data);
}

void nes_8025_device::write_h(offs_t offset, u8 data)
{
	LOG("8025 write_h, offset: %04x, data: %02x\n", offset, data);

	m_latch_data = data;    // UNROM / CNROM latch: any write to $8000-$FFFF, no bus conflicts

	// the MMC3 registers decode $E003: $x002 and $x003 (e.g. $9FFF) do nothing
	if (!BIT(offset, 1))
	{
		switch (offset & 0x6001)
		{
			case 0x0000:    // bank select
				m_latch = data;
				break;

			case 0x0001:    // bank data
			{
				const int cmd = m_latch & (ext_mode() ? 0x0f : 0x07);
				switch (cmd)
				{
					case 0: case 1: case 2: case 3: case 4: case 5:
						m_mmc_vrom_bank[cmd] = data;
						break;
					case 6: case 7: case 8: case 9:
						m_mmc_prg_bank[cmd - 6] = data;
						break;
					case 10: case 11:
						m_mmc_vrom_bank[cmd - 4] = data;
						break;
					default:
						break;
				}
				break;
			}

			case 0x2000:    // mirroring
				m_mmc_mirror = data & 0x03;
				break;

			case 0x2001:    // WRAM protect, or the RAM configuration register on submapper 2
				m_wram_protect = data;
				break;

			default:        // IRQ registers
				txrom_write(offset, data);
				break;
		}
	}
	update_banks();
}


/*-------------------------------------------------

 Rinco Talkman FSG2 board

 Games: Rinco Talkman 3-in-1 (Captain Bassam, The Dragon,
 Os Cavaleiros do Zodiaco), for Rinco's Talkman RS-7600
 famiclone, whose enhanced audio it uses

 NESdev NES 2.0 Mapper 594 (register details as in FCEUmm's
 594.c, which agrees with the page):
 - an MMC3 clone at $8000-$FFFF, except $9000-$9FFF and
   $B000-$BFFF, which hold the board's own registers
 - $9000 (even): the 8 KiB PRG-ROM bank at $6000-$7FFF
 - $9001 (odd): function unknown, games write $10 before $9000
 - $B000 (even): address space and mode, bits 7-6:
   00: $6000 bank over PRG $000000-$1FFFFF, MMC3 over PRG
       $000000-$07FFFF and CHR as INES Mapper 187 (CHR A18 =
       PPU A12, 512 KiB)
   40: $6000 bank and MMC3 over PRG $180000-$1FFFFF, MMC3 over
       CHR $80000-$BFFFF (256 KiB)
   80: $6000 bank over PRG $200000-$2FFFFF, MMC3 over PRG
       $200000-$27FFFF and CHR $C0000-$FFFFF
 - $5000 (even, write): a byte into the 1024-byte Am7202A FIFO
 - $5001 (odd, write): empties the FIFO and sets the OKI
   MSM6585 sample rate from bits 7-6: 4, 8, 16 or 32 kHz
 - $5000-$5FFF (read): bit 6 is set while the FIFO is less
   than half full, the other bits read 0 (FCEUmm); games keep
   filling the FIFO while bit 6 is set
 The MSM6585 takes the high nibble of each FIFO byte first
 (FCEUmm).

 Not documented, so not emulated: the $9001 register, what an
 empty FIFO gives the MSM6585 (here $08, a step up and back,
 which holds the output), the MSM6585 clock (its usual 640 kHz
 here) and the mixing level (the same 0.5 as the other cart
 audio here), and whether the MMC3 clone's IRQ matches a
 Nintendo MMC3 (FCEUmm uses an AX5202P model; the Nintendo
 MMC3's IRQ is used here).

 NES 2.0: mapper 594

 In MAME: Supported.

 -------------------------------------------------*/

nes_rinco_fsg2_device::nes_rinco_fsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_RINCO_FSG2, tag, owner, clock)
	, m_adpcm(*this, "adpcm")
	, m_fifo_read(0), m_fifo_count(0), m_adpcm_byte(0x08), m_adpcm_low(false)
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
	std::fill(std::begin(m_fifo), std::end(m_fifo), 0);
}

void nes_rinco_fsg2_device::device_add_mconfig(machine_config &config)
{
	// additional sound hardware, mixed into the console audio through the cartridge connector
	MSM6585(config, m_adpcm, 640'000);
	m_adpcm->set_prescaler_selector(msm6585_device::S160);
	m_adpcm->vck_legacy_callback().set(FUNC(nes_rinco_fsg2_device::adpcm_vck));
	m_adpcm->add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.50);
}

void nes_rinco_fsg2_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_read));
	save_item(NAME(m_fifo_count));
	save_item(NAME(m_adpcm_byte));
	save_item(NAME(m_adpcm_low));
}

void nes_rinco_fsg2_device::fifo_clear()
{
	m_fifo_read = 0;
	m_fifo_count = 0;
}

void nes_rinco_fsg2_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
	fifo_clear();
	m_adpcm_low = false;
	m_adpcm->reset_w(0);
	m_adpcm->playmode_w(msm6585_device::S160 & 7);
	mmc3_common_initialize(0xff, 0xff, 0);
}

int nes_rinco_fsg2_device::prg_or() const
{
	return (BIT(m_reg[2], 6) ? 0x0c0 : 0x000) | (BIT(m_reg[2], 7) ? 0x100 : 0x000);
}

void nes_rinco_fsg2_device::prg_cb(int start, int bank)
{
	prg8_x(start, (bank & 0x3f) | prg_or());
}

void nes_rinco_fsg2_device::chr_cb(int start, int bank, int source)
{
	// mode 00: CHR A18 follows PPU A12 (INES Mapper 187); modes 40/80: one 256 KiB window
	const int chr_and = (m_reg[2] & 0xc0) ? 0x0ff : 0x1ff;
	const int chr_or = (BIT(m_reg[2], 6) ? 0x200 : 0x000) | (BIT(m_reg[2], 7) ? 0x300 : 0x000);
	const int b = (bank & 0xff) | ((start & 4) ? 0x100 : 0x000);
	chr1_x(start, (b & chr_and) | (chr_or & ~chr_and), source);
}

u8 nes_rinco_fsg2_device::read_m(offs_t offset)
{
	return m_prg[((u32(m_reg[0] | prg_or()) * 0x2000) + (offset & 0x1fff)) % m_prg_size];
}

void nes_rinco_fsg2_device::write_m(offs_t offset, u8 data)
{
	// PRG-ROM at $6000-$7FFF
}

void nes_rinco_fsg2_device::write_h(offs_t offset, u8 data)
{
	LOG("rinco fsg2 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x7000)
	{
		case 0x1000:   // $9000-$9FFF
		case 0x3000:   // $B000-$BFFF
			m_reg[((offset >> 12) & 2) | (offset & 1)] = data;
			set_prg(m_prg_base, m_prg_mask);
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;
		default:
			txrom_write(offset, data);
			break;
	}
}

u8 nes_rinco_fsg2_device::read_l(offs_t offset)
{
	if (((offset + 0x4100) & 0xf000) == 0x5000)
		return (m_fifo_count < sizeof(m_fifo) / 2) ? 0x40 : 0x00;
	return get_open_bus();
}

void nes_rinco_fsg2_device::write_l(offs_t offset, u8 data)
{
	const u16 addr = offset + 0x4100;
	if ((addr & 0xf000) != 0x5000)
		return;
	if (BIT(addr, 0))
	{
		// rate code 0-3 is 4, 8, 16, 32 kHz: S1 = bit 0, S2 = bit 1 of the MSM6585 selector
		m_adpcm->playmode_w(((data >> 6) & 3) | 4);
		fifo_clear();
	}
	else if (m_fifo_count < sizeof(m_fifo))
	{
		m_fifo[(m_fifo_read + m_fifo_count) % sizeof(m_fifo)] = data;
		m_fifo_count++;
	}
}

void nes_rinco_fsg2_device::adpcm_vck(int state)
{
	if (!m_adpcm_low)
	{
		if (m_fifo_count)
		{
			m_adpcm_byte = m_fifo[m_fifo_read];
			m_fifo_read = (m_fifo_read + 1) % sizeof(m_fifo);
			m_fifo_count--;
		}
		else
			m_adpcm_byte = 0x08;
		m_adpcm->data_w(m_adpcm_byte >> 4);
	}
	else
		m_adpcm->data_w(m_adpcm_byte & 0x0f);
	m_adpcm_low = !m_adpcm_low;
}


/*-------------------------------------------------

 BMC N49C-300 board

 Games: Super Mario Bros. Party 6-in-1

 NESdev NES 2.0 Mapper 369: the Mario games' mappers 0, 4 and 40
 behind an outer bank register at $4100 (mask $C100). NESdev
 says the meaning of its bits is not understood and lists the
 five values the menu writes:
   $00  NROM, PRG 32K bank 0, CHR 8K bank 0 (Mario Bros.)
   $01  NROM, PRG 32K bank 1, CHR 8K bank 1 (Super Mario Bros.)
   $13  mapper 40 (NTDEC 2722) in the 64K at 8K bank 8, CHR 8K
        bank 3 (Super Mario Bros. 2J)
   $37  MMC3 with 128K PRG and 128K CHR at 8K bank $10 (SMB2)
   $FF  MMC3 with 256K PRG and 256K CHR at 8K bank $20 (SMB3)
 NESdev gives Super Mario Bros.' PRG offset as 8K bank $01; the
 dump settles it at 32K bank 1 (8K bank 4), as in FCEUmm: only
 there are its vectors Super Mario Bros.' own (reset $8000, NMI
 $8082); at 8K bank 1 they are $AB82 and $2BFF.

 Not documented, so taken from FCEUmm: only those five values
 change the banks (any other value leaves them as they were);
 the mapper 40 mode maps 8K banks $0E at $6000, $0C, $0D, $08 +
 the $E000 register (3 bits) and $0F at $8000-$FFFF, $8000
 writes stop and acknowledge its IRQ, $A000 writes start it; the
 MMC3 modes map the PRG-RAM at $6000 and the MMC3's scanline IRQ
 does not run in the mapper 40 mode. The mapper 40 IRQ counter
 is the tree's mapper 40 one (NESdev INES Mapper 040: 13 bits,
 /IRQ while bit 12 is set, released when it wraps); FCEUmm stops
 at 4096 instead. Mirroring stays with the MMC3's $A000 register
 in every mode (FCEUmm does not set it in the NROM and mapper 40
 modes either). Power-on value of $4100: 0 (FCEUmm).

 NES 2.0: mapper 369

 In MAME: Supported.

 -------------------------------------------------*/

nes_bmc_n49c300_device::nes_bmc_n49c300_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_N49C300, tag, owner, clock), m_mode(0), m_smb2j_bank(0), m_smb2j_irq_enable(0), m_smb2j_irq_count(0)
{
}

void nes_bmc_n49c300_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mode));
	save_item(NAME(m_smb2j_bank));
	save_item(NAME(m_smb2j_irq_enable));
	save_item(NAME(m_smb2j_irq_count));
}

void nes_bmc_n49c300_device::pcb_reset()
{
	m_mode = 0;
	m_smb2j_bank = 0;
	m_smb2j_irq_enable = 0;
	m_smb2j_irq_count = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
	update_banks();
}

void nes_bmc_n49c300_device::update_banks()
{
	switch (m_mode)
	{
		case 0x00:
		case 0x01:
			prg32(m_mode);
			chr8(m_mode, CHRROM);
			break;
		case 0x13:
			prg8_89(0x0c);
			prg8_ab(0x0d);
			prg8_cd(0x08 | m_smb2j_bank);
			prg8_ef(0x0f);
			chr8(0x03, CHRROM);
			break;
		case 0x37:
		case 0xff:
			set_prg(m_prg_base, m_prg_mask);
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;
	}
}

void nes_bmc_n49c300_device::prg_cb(int start, int bank)
{
	if (m_mode == 0x37)
		prg8_x(start, (bank & 0x0f) | 0x10);
	else if (m_mode == 0xff)
		prg8_x(start, (bank & 0x1f) | 0x20);
}

void nes_bmc_n49c300_device::chr_cb(int start, int bank, int source)
{
	if (m_mode == 0x37)
		chr1_x(start, (bank & 0x7f) | 0x80, source);
	else if (m_mode == 0xff)
		chr1_x(start, (bank & 0xff) | 0x100, source);
}

u8 nes_bmc_n49c300_device::read_m(offs_t offset)
{
	if (m_mode == 0x13)
		return m_prg[((0x0e * 0x2000) + (offset & 0x1fff)) & (m_prg_size - 1)];
	if (m_mode == 0x37 || m_mode == 0xff)
		return nes_txrom_device::read_m(offset);
	return get_open_bus();
}

void nes_bmc_n49c300_device::write_m(offs_t offset, u8 data)
{
	if (m_mode == 0x37 || m_mode == 0xff)
		nes_txrom_device::write_m(offset, data);
}

void nes_bmc_n49c300_device::write_l(offs_t offset, u8 data)
{
	LOG("bmc n49c300 write_l, offset: %04x, data: %02x\n", offset, data);

	if (((offset + 0x4100) & 0xc100) != 0x4100)
		return;
	switch (data)
	{
		case 0x00: case 0x01: case 0x13: case 0x37: case 0xff:
			m_mode = data;
			update_banks();
			break;
	}
}

void nes_bmc_n49c300_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc n49c300 write_h, offset: %04x, data: %02x\n", offset, data);

	if (m_mode == 0x13)
	{
		switch (offset & 0x6000)
		{
			case 0x0000:    // $8000: stop, clear and acknowledge
				m_smb2j_irq_enable = 0;
				m_smb2j_irq_count = 0;
				set_irq_line(CLEAR_LINE);
				break;
			case 0x2000:    // $A000: start
				m_smb2j_irq_enable = 1;
				break;
			case 0x6000:    // $E000: the bank at $C000
				m_smb2j_bank = data & 0x07;
				update_banks();
				break;
		}
		return;
	}

	txrom_write(offset, data);
	if (m_mode != 0x37 && m_mode != 0xff)
		update_banks();     // the NROM modes keep their fixed banks
}

void nes_bmc_n49c300_device::ppu_a12_rise()
{
	if (m_mode != 0x13)
		nes_txrom_device::ppu_a12_rise();
}

void nes_bmc_n49c300_device::m2_w(int state, offs_t address, bool write)
{
	if (state || m_mode != 0x13 || !m_smb2j_irq_enable)
		return;
	m_smb2j_irq_count = (m_smb2j_irq_count + 1) & 0x1fff;     // 13-bit counter, as the tree's mapper 40
	if (BIT(m_smb2j_irq_count, 12))
		set_irq_line(ASSERT_LINE);
	else if (!m_smb2j_irq_count)
		set_irq_line(CLEAR_LINE);
}


/*-------------------------------------------------

 BMC HPxx board (BMC-HPxx, BMC-HP2018-A)

 Games: HP10xx/HP20xx multicarts (Super 64-in-1 and others)

 NESdev NES 2.0 Mapper 260, a predecessor of FK23C:
 - $5000 (mask $F003, write): L... .MMM, L locks the $5xxx
   registers; modes 0-3 MMC3 with 256/128 KiB PRG and 256/128
   KiB CHR inner banks (mode bit 1: 128 KiB PRG, bit 0: 128 KiB
   CHR), 4 NROM-128, 5 NROM-256, 6 CNROM 16 KiB CHR, 7 CNROM
   32 KiB CHR
 - $5000 (read): DIP switch in bits 1-0
 - $5001: 16 KiB PRG base, $5002: 8 KiB CHR base; the MMC3
   bank is masked by the inner size and OR'ed with the base
   masked the other way
 - $8000-$FFFF (mask $8000): the CNROM/nametable latch .... .MLL
   (inner CHR bank in the CNROM modes, M: 0 vertical, 1
   horizontal mirroring in modes 4-7)
 - $8000-$FFFF (mask $E001): MMC3 registers; mirroring comes
   from $A000 only in modes 0-3
 - WRAM at $6000-$7FFF

 The DIP switch is a configuration setting (default 0).

 Not documented, so not emulated: the read mask of the DIP
 switch ($F003 here, as the writes), the bits 7-2 of the $5000
 read (open bus here), the power-on values (0: mode 0, bases
 0, unlocked). NESdev notes
 that no dump uses the CNROM modes (their games were changed
 to write $5002); they follow FCEUX's description there.

 NES 2.0: mapper 260

 In MAME: Supported.

 -------------------------------------------------*/

static INPUT_PORTS_START( bmc_hpxx )
	PORT_START("DSW")
	PORT_CONFNAME( 0x03, 0x00, "DIP Switch" )
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "3" )
INPUT_PORTS_END

ioport_constructor nes_bmc_hpxx_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_hpxx );
}

nes_bmc_hpxx_device::nes_bmc_hpxx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_HPXX, tag, owner, clock), m_dsw(*this, "DSW"), m_mode(0), m_prg_outer(0), m_chr_outer(0), m_cn_latch(0)
{
}

void nes_bmc_hpxx_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mode));
	save_item(NAME(m_prg_outer));
	save_item(NAME(m_chr_outer));
	save_item(NAME(m_cn_latch));
}

void nes_bmc_hpxx_device::pcb_reset()
{
	m_mode = m_prg_outer = m_chr_outer = m_cn_latch = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_bmc_hpxx_device::prg_cb(int start, int bank)
{
	if (m_mode & 4)
		return;
	const int mask = BIT(m_mode, 1) ? 0x0f : 0x1f;
	prg8_x(start, (bank & mask) | ((m_prg_outer << 1) & ~mask));
}

void nes_bmc_hpxx_device::chr_cb(int start, int bank, int source)
{
	if (m_mode & 4)
		return;
	const int mask = BIT(m_mode, 0) ? 0x7f : 0xff;
	chr1_x(start, (bank & mask) | ((m_chr_outer << 3) & ~mask), source);
}

void nes_bmc_hpxx_device::update_banks()
{
	switch (m_mode & 7)
	{
		case 4:   // NROM-128
			prg16_89ab(m_prg_outer);
			prg16_cdef(m_prg_outer);
			chr8(m_chr_outer, CHRROM);
			break;
		case 5:   // NROM-256
			prg32(m_prg_outer >> 1);
			chr8(m_chr_outer, CHRROM);
			break;
		case 6:   // CNROM, 16 KiB CHR
			prg32(m_prg_outer >> 1);
			chr8((m_chr_outer & ~1) | (m_cn_latch & 1), CHRROM);
			break;
		case 7:   // CNROM, 32 KiB CHR
			prg32(m_prg_outer >> 1);
			chr8((m_chr_outer & ~3) | (m_cn_latch & 3), CHRROM);
			break;
		default:
			set_prg(m_prg_base, m_prg_mask);
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			set_nt_mirroring(BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			return;
	}
	set_nt_mirroring(BIT(m_cn_latch, 2) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

u8 nes_bmc_hpxx_device::read_l(offs_t offset)
{
	if (((offset + 0x4100) & 0xf003) == 0x5000)
		return (get_open_bus() & 0xfc) | (m_dsw->read() & 0x03);
	return get_open_bus();
}

void nes_bmc_hpxx_device::write_l(offs_t offset, u8 data)
{
	LOG("bmc hpxx write_l, offset: %04x, data: %02x\n", offset, data);

	const u16 addr = offset + 0x4100;
	if ((addr & 0xf000) != 0x5000 || BIT(m_mode, 7))
		return;
	switch (addr & 0x0003)
	{
		case 0: m_mode = data; break;
		case 1: m_prg_outer = data & 0x3f; break;
		case 2: m_chr_outer = data & 0x7f; break;
		default: return;
	}
	update_banks();
}

void nes_bmc_hpxx_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc hpxx write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write(offset, data);
	m_cn_latch = data;
	if (m_mode & 4)
		update_banks();   // the latch, and the mirroring an MMC3 $A000 write just set, belong to the latch here
}


/*-------------------------------------------------

 Realtec 8090 and related boards

 Games: 7-in-1 HuaMian Xuan Guan (CM-020) and other Realtec
 multicarts

 NESdev NES 2.0 Mapper 432 (MMC3-based):
 - $6000-$7FFE (even, mask $E001): bit 0 S, reads of $8000-
   $FFFF return the solder pad value (submapper 1: Q instead of
   S; submapper 4: open bus while S is set and the pad is
   connected)
 - $6001-$7FFF (odd, mask $E001): pNQP CmMb; both registers
   go through the MMC3 clone's WRAM interface, so they are only
   written while $A001 enables WRAM and does not write-protect
   it
   b: PRG/CHR A17 where M or m is set; M: PRG A17 from b
   (128 KiB inner PRG); m: CHR A17 from b (128 KiB inner CHR);
   C: CHR A18; P: PRG A18; N: NROM-128 (the MMC3 sees A14 low,
   so its registers 6 and 7 select both halves)
   Q, by submapper: 0: PRG and CHR A19; 1: read the solder pad;
   2: NROM-256; 3: 512 KiB CHR mode, wired as INES Mapper 197
   submapper 2 (PPU A10 to CHR A10, PPU A11 to the MMC3's PA10
   and PA11, the MMC3's CHR A10-A17 to CHR A11-A18; the outer
   CHR bits are not used)
   p, by submapper: 0 and 1: NROM-256; 2 and 3: lock of the
   outer bank register (the $6000 register stays writable)
 Submapper 3 therefore has no NROM-256 mode. Submapper 4
 (940702) is described only by its pad read, so it is otherwise
 handled as submapper 0. The solder pad is a configuration
 setting (4 bits, as read back; 0 by default).

 Not documented, so not emulated: the MMC3 clone's IRQ revision
 (Nintendo MMC3 here, AX5202P in FCEUmm). FCEUmm differs from
 NESdev in using Q as A19 on every submapper, in NROM-256 on
 submapper 3, in locking both registers and in locking only on
 submapper 3; NESdev is followed.

 NES 2.0: mapper 432

 In MAME: Supported.

 -------------------------------------------------*/

static INPUT_PORTS_START( realtec432 )
	PORT_START("PAD")
	PORT_CONFNAME( 0x0f, 0x00, "Solder Pad" )
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

ioport_constructor nes_realtec432_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( realtec432 );
}

nes_realtec432_device::nes_realtec432_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_REALTEC432, tag, owner, clock), m_pad_port(*this, "PAD"), m_submapper(0), m_pad(0)
{
	m_reg[0] = m_reg[1] = 0;
	m_mmc_prg[0] = m_mmc_prg[1] = 0;
}

void nes_realtec432_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_mmc_prg));
	save_item(NAME(m_pad));
}

void nes_realtec432_device::pcb_reset()
{
	m_pad = m_pad_port->read() & 0x0f;
	m_reg[0] = m_reg[1] = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_realtec432_device::prg_cb(int start, int bank)
{
	const u8 r = m_reg[1];
	if (start < 2)
		m_mmc_prg[start] = bank;
	if (BIT(r, 6))
	{
		// NROM modes: the MMC3 sees A14 low; NROM-256 is p on submappers 0, 1 and 4, Q on submapper 2, absent on 3
		bool nrom256;
		switch (m_submapper)
		{
			case 2:  nrom256 = BIT(r, 5); break;
			case 3:  nrom256 = false;     break;
			default: nrom256 = BIT(r, 7); break;
		}
		const int mask = nrom256 ? 3 : 1;
		bank = (m_mmc_prg[start & 1] & ~mask) | (start & mask);
	}
	const int and_mask = BIT(r, 1) ? 0x0f : 0x1f;
	const int or_bits = ((r << 4) & 0x10) | ((r << 1) & 0x20) | (q_is_a19() ? ((r << 1) & 0x40) : 0);
	prg8_x(start, (bank & and_mask) | (or_bits & ~and_mask));
}

void nes_realtec432_device::chr_cb(int start, int bank, int source)
{
	const u8 r = m_reg[1];
	const int and_mask = BIT(r, 2) ? 0x7f : 0xff;
	const int or_bits = ((r << 7) & 0x80) | ((r << 5) & 0x100) | (q_is_a19() ? ((r << 4) & 0x200) : 0);
	chr1_x(start, (bank & and_mask) | (or_bits & ~and_mask), source);
}

void nes_realtec432_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (!chr512())
	{
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
		return;
	}

	// 512 KiB CHR mode (INES Mapper 197 submapper 2 wiring): the MMC3 sees PPU A11 on both PA10 and PA11
	const u8 chr_page = (m_latch & 0x80) >> 5;
	int mmc[8];
	mmc[chr_page ^ 0] = (m_mmc_vrom_bank[0] & ~0x01) & 0xff;
	mmc[chr_page ^ 1] = (m_mmc_vrom_bank[0] |  0x01) & 0xff;
	mmc[chr_page ^ 2] = (m_mmc_vrom_bank[1] & ~0x01) & 0xff;
	mmc[chr_page ^ 3] = (m_mmc_vrom_bank[1] |  0x01) & 0xff;
	mmc[chr_page ^ 4] = m_mmc_vrom_bank[2] & 0xff;
	mmc[chr_page ^ 5] = m_mmc_vrom_bank[3] & 0xff;
	mmc[chr_page ^ 6] = m_mmc_vrom_bank[4] & 0xff;
	mmc[chr_page ^ 7] = m_mmc_vrom_bank[5] & 0xff;
	for (int slot = 0; slot < 8; slot++)
	{
		const int a11 = BIT(slot, 1);
		const int seen = (slot & 4) | (a11 << 1) | a11;
		chr1_x(slot, (mmc[seen] << 1) | (slot & 1), chr);
	}
}

void nes_realtec432_device::write_m(offs_t offset, u8 data)
{
	LOG("realtec 432 write_m, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_wram_protect, 7) || BIT(m_wram_protect, 6))
		return;
	if (BIT(offset, 0) && ((m_submapper == 2) || (m_submapper == 3)) && BIT(m_reg[1], 7))
		return;   // the outer bank register is locked
	m_reg[offset & 1] = data;
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

u8 nes_realtec432_device::read_h(offs_t offset)
{
	switch (m_submapper)
	{
		case 1:
			if (BIT(m_reg[1], 5))
				return m_pad & 0x0f;
			break;
		case 4:
			if (BIT(m_reg[0], 0) && BIT(m_pad, 0))
				return get_open_bus();
			break;
		default:
			if (BIT(m_reg[0], 0))
				return m_pad & 0x0f;
			break;
	}
	return hi_access_rom(offset);
}


/*-------------------------------------------------

 Jncota FS005 board with 2 KiB CHR banking

 Games: Fengshenbang: Fumo San Taizi (Jncota re-release of the
 C&E game on INES Mapper 246)

 NESdev NES 2.0 Mapper 523: the Waixing FS005 board (INES
 Mapper 176, here its submapper 2), except that
 - mirroring is hard-wired (horizontal for this game; taken
   from the header's mirroring bit here),
 - CHR-ROM A10 is PPU A10, CHR-ROM A11-A18 are the MMC3 clone's
   CHR A10-A17 outputs and the clone's PPU A10 input is
   grounded, so in the extended MMC3 mode the clone's
   registers select 2 KiB CHR banks: each 1 KiB window takes
   the bank the clone outputs for the even window of its pair,
   shifted up one bit, with PPU A10 as the lowest bit.

 NES 2.0: mapper 523

 In MAME: Supported.

 -------------------------------------------------*/

nes_jncota_fs005_device::nes_jncota_fs005_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_8025_device(mconfig, NES_JNCOTA_FS005, tag, owner, clock)
{
	set_8025_submapper(2);   // the FS005
}

void nes_jncota_fs005_device::pcb_reset()
{
	nes_8025_device::pcb_reset();
	set_nt_mirroring(hardwired_mirroring());
}

void nes_jncota_fs005_device::write_h(offs_t offset, u8 data)
{
	nes_8025_device::write_h(offset, data);
	set_nt_mirroring(hardwired_mirroring());   // hard-wired: the clone's mirroring output is not connected
}

void nes_jncota_fs005_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	nes_8025_device::set_chr(chr, chr_base, chr_mask);

	// the clone answers PPU A10 = 0: the output for the even window of each pair, one bit up, PPU A10 below it
	int bank[8], source[8];
	for (int i = 0; i < 8; i++)
	{
		bank[i] = m_chr_orig[i] / 0x400;
		source[i] = m_chr_src[i];
	}
	for (int i = 0; i < 8; i++)
		chr1_x(i, ((bank[i & ~1] & 0xff) << 1) | (i & 1), source[i & ~1]);
}


/*-------------------------------------------------

 Games Xplosion 121-in-1, 15000-in-1, 18000-in-1

 NESdev NES 2.0 Mapper 269: an MMC3 clone with four outer bank
 registers, all written at $5000 (mask probably $F008, as some
 games write $5008 and those writes must be ignored): the
 first write goes to register 0, the next to 1, then 2, 3 and
 0 again.
 - register 0: outer 1 KiB CHR bank bits 7-0
 - register 1: outer 8 KiB PRG bank bits 7-0
 - register 2: CCCC EMMM as in FCEUmm's 269.c (see below), C
   outer CHR bank bits 11-8, E limits the MMC3 CHR bank to
   MMM+1 bits: CHR = (MMC3 & inner mask) | outer
 - register 3 (NESdev): OOMM MMMM, MMMMMM the bit mask of the
   PRG bank bits that come from the outer bank, OO outer PRG
   bank bits 9-8 and CHR bank bits 13-12, no lock:
   PRG = (MMC3 & ~mask) | (outer & mask), the bank bits above
   the MMC3's 6 always from the outer bank
 - WRAM at $6000-$7FFF: the loader gives the board 8 KiB whatever
   the header says (NESdev's size; the 15000-in-1 and 18000-in-1
   headers declare none). Tested on (KB-983-40C) 18000-in-1.nes:
   Super Mario Bros. 3 (entry 4) draws its World 1 map and status
   bar, and its $6000-$7FFF takes 66,091 writes and 166,669 reads by
   frame 3000, so the game uses that RAM. NOT TESTED: whether the saved
   data survives a reload (no save or reload test was run).
 - power-on: registers 0, 1, 3 = $00, register 2 = $0F
 - PRG and CHR share one ROM, the CHR fetches reading it with
   the data bits scrambled (stored 76543210 = actual 70615243);
   the board unscrambles a copy of the PRG ROM at start-up and
   uses it as the CHR ROM

 Register 2 departs from NESdev, which describes its low nibble
 as the number of outer CHR bits (power-on $0F: every CHR bank
 from the outer register): on the 15000-in-1 dump that reading
 draws the menu with broken tiles from power-on, while FCEUmm's
 reading draws the menu, the stage select and a stage
 correctly. Register 3 follows NESdev; FCEUmm instead uses its
 bit 7 as a lock of the $5000 registers and ORs the whole outer
 bank, and nothing in the dump decides between the two.

 NES 2.0: mapper 269

 In MAME: Supported.

 -------------------------------------------------*/

nes_bmc_gx121_device::nes_bmc_gx121_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_GX121, tag, owner, clock), m_reg_index(0)
{
	m_reg[0] = m_reg[1] = m_reg[3] = 0x00;
	m_reg[2] = 0x0f;
}

void nes_bmc_gx121_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_reg_index));
}

void nes_bmc_gx121_device::pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted)
{
	// the CHR fetches read the PRG ROM with scrambled data bits: keep an unscrambled copy as the CHR ROM
	if (!m_vrom && m_prg && m_prg_size)
	{
		vrom_alloc(m_prg_size);
		for (u32 i = 0; i < m_prg_size; i++)
			m_vrom[i] = bitswap<8>(m_prg[i], 7, 0, 6, 1, 5, 2, 4, 3);
	}
	nes_txrom_device::pcb_start(machine, ciram_ptr, cart_mounted);
}

void nes_bmc_gx121_device::pcb_reset()
{
	m_reg[0] = m_reg[1] = m_reg[3] = 0x00;
	m_reg[2] = 0x0f;
	m_reg_index = 0;
	m_chr_source = CHRROM;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_bmc_gx121_device::prg_cb(int start, int bank)
{
	// NESdev: PRG = (MMC3 & ~mask) | (outer & mask), mask = register 3 bits 5-0; the outer bank bits above the
	// MMC3's 6 come from the outer bank, with register 3 bits 7-6 as outer bits 9-8
	const int mask = m_reg[3] & 0x3f;
	const int outer = m_reg[1] | ((m_reg[3] & 0xc0) << 2);
	prg8_x(start, (bank & 0x3f & ~mask) | (outer & (mask | ~0x3f)));
}

void nes_bmc_gx121_device::chr_cb(int start, int bank, int source)
{
	// register 2 bit 3 limits the MMC3 bank to (bits 2-0) + 1 bits; bits 7-4 are outer CHR bank bits 11-8
	int inner = bank & 0xff;
	if (BIT(m_reg[2], 3))
		inner &= (1 << ((m_reg[2] & 7) + 1)) - 1;
	chr1_x(start, inner | m_reg[0] | ((m_reg[2] & 0xf0) << 4) | ((m_reg[3] & 0xc0) << 6), CHRROM);   // register 3 bits 7-6: CHR bits 13-12 (NESdev)
}

void nes_bmc_gx121_device::write_l(offs_t offset, u8 data)
{
	LOG("bmc gx121 write_l, offset: %04x, data: %02x\n", offset, data);

	if (((offset + 0x4100) & 0xf008) != 0x5000)
		return;
	m_reg[m_reg_index] = data;
	m_reg_index = (m_reg_index + 1) & 3;
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}


/*-------------------------------------------------

 BMC-SB-5013 (SB-5013, GCL8050, 841242C) board

 Games: Super Boy 6-in-1, Super Boy Top 5-in-1 (CHR-RAM),
 96 Supreme 3-in-1 (NT-646) (CHR-ROM)

 NESdev NES 2.0 Mapper 359 (all registers mask $F003). Not an
 MMC3 register-wise; it is here because its PPU A12 IRQ mode is
 the MMC3's scanline counter, which this board reuses.
 - $8000/$8001/$8002/$8003: 8 KiB PRG-ROM banks at $8000, $A000,
   $C000 and $6000; $E000 is the last bank of the outer bank
 - $9000: ..PP P..., the 128 KiB outer PRG bank
 - $9001: .C.. ..SS, outer PRG bank size (0 512 KiB, inner AND
   $3F; 1 256 KiB, $1F; 2 unused, $2F; 3 128 KiB, $0F), C outer
   CHR bank size (0 128 KiB, inner AND $7F; 1 256 KiB, $FF)
 - $9002: mirroring (vertical, horizontal, one-screen 0 and 1)
 - $9003: ......CC, the 128 KiB outer CHR-ROM bank
 - $A000-$A003, $B000-$B003: 1 KiB CHR-ROM banks (no effect with
   the unbanked CHR-RAM)
 - $C000: M2 mode, the low byte of the 16-bit counter; PA12 mode,
   sets the reload flag (as the MMC3's $C001); with the auto
   flag, disables counting (M2) or IRQ generation (PA12)
 - $C001: M2 mode, the high byte; PA12 mode, the reload value
   (as the MMC3's $C000); with the auto flag, enables
 - $C002: .AME, E enable, M source (0 M2, 1 filtered PPU A12
   rises), A the auto flag; $C003: ...E, no effect with A set
 - M2 mode: while enabled the counter counts down each M2 cycle
   until zero, then raises the IRQ; PA12 mode: as the MMC3

 Not documented, so not emulated: how the IRQ is acknowledged
 (here: any write that disables it, and every $C000-$C003 write
 in M2 mode, clears it), whether the M2 counter stops at zero
 (it stops here), the M2 edge counted (the rise), the power-on
 values (all registers 0: 512 KiB banks from outer bank 0,
 vertical mirroring, IRQ off), and the $9001 SS=2 mask ($2F as
 NESdev guesses).

 FCEUmm's 359.c has the same banking and power-on values and
 also enables FDS expansion sound for this mapper; NESdev does
 not mention the FDS sound, so it is not added here.

 The menu of Super Boy 6-in-1 writes PPUADDR on its first frame
 after power-on, before the PPU accepts writes (NESdev PPU power
 up state: $2000, $2001, $2005 and $2006 are ignored for about
 29658 CPU cycles on the NES-001; on the Famicom the PPU's reset
 is tied high and its warm-up ends about a frame before the CPU
 starts). On the nes machine the address is ignored, the menu's
 font lands at CHR $0001 instead of $0200 and the menu draws as
 repeated tiles, as it would on a front-loading NES; on the
 famicom machine the font lands at $0200 and the menu is drawn
 correctly. Not a board fault.

 NES 2.0: mapper 359

 In MAME: Supported.

 -------------------------------------------------*/

nes_bmc_sb5013_device::nes_bmc_sb5013_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock)
	, m_prg_outer(0), m_size(0), m_chr_outer(0), m_irq_mode(0), m_irq_on(false), m_m2_count(0)
{
	std::fill(std::begin(m_prg_reg), std::end(m_prg_reg), 0);
	std::fill(std::begin(m_chr), std::end(m_chr), 0);
}

nes_bmc_sb5013_device::nes_bmc_sb5013_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_sb5013_device(mconfig, NES_BMC_SB5013, tag, owner, clock)
{
}

nes_unl_82112c_device::nes_unl_82112c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_sb5013_device(mconfig, NES_UNL_82112C, tag, owner, clock)
{
}

void nes_bmc_sb5013_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_prg_reg));
	save_item(NAME(m_chr));
	save_item(NAME(m_prg_outer));
	save_item(NAME(m_size));
	save_item(NAME(m_chr_outer));
	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_on));
	save_item(NAME(m_m2_count));
}

void nes_bmc_sb5013_device::pcb_reset()
{
	std::fill(std::begin(m_prg_reg), std::end(m_prg_reg), 0);
	std::fill(std::begin(m_chr), std::end(m_chr), 0);
	m_prg_outer = m_size = m_chr_outer = m_irq_mode = 0;
	m_irq_on = false;
	m_m2_count = 0;
	mmc3_common_initialize(0xff, 0xff, 0);   // the scanline counter; the banks come from set_prg/set_chr below
	m_irq_enable = 0;
	set_nt_mirroring(PPU_MIRROR_VERT);
	set_irq_line(CLEAR_LINE);
}

void nes_bmc_sb5013_device::set_prg(int prg_base, int prg_mask)
{
	static const u8 masks[4] = { 0x3f, 0x1f, 0x2f, 0x0f };
	const int mask = masks[m_size & 3];
	const int base = ((m_prg_outer >> 3) & 7) << 4;   // 128 KiB outer bank, in 8 KiB banks
	prg8_89((m_prg_reg[0] & mask) | (base & ~mask));
	prg8_ab((m_prg_reg[1] & mask) | (base & ~mask));
	prg8_cd((m_prg_reg[2] & mask) | (base & ~mask));
	prg8_ef((0xff & mask) | (base & ~mask));
}

u8 nes_bmc_sb5013_device::read_m(offs_t offset)
{
	static const u8 masks[4] = { 0x3f, 0x1f, 0x2f, 0x0f };
	const int mask = masks[m_size & 3];
	const int base = ((m_prg_outer >> 3) & 7) << 4;
	const u32 bank = (m_prg_reg[3] & mask) | (base & ~mask);
	return m_prg[(bank * 0x2000 + (offset & 0x1fff)) % m_prg_size];
}

void nes_bmc_sb5013_device::write_m(offs_t offset, u8 data)
{
	// PRG-ROM at $6000-$7FFF
}

void nes_bmc_sb5013_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (!m_vrom_chunks)
	{
		chr8(0, CHRRAM);
		return;
	}
	const int mask = BIT(m_size, 6) ? 0xff : 0x7f;
	const int base = (m_chr_outer & 3) << 7;   // 128 KiB outer bank, in 1 KiB banks
	for (int i = 0; i < 8; i++)
		chr1_x(i, (m_chr[i] & mask) | (base & ~mask), CHRROM);
}

void nes_bmc_sb5013_device::update_irq_enable()
{
	// PA12 mode: the MMC3 counter keeps counting; only its IRQ generation follows the enable
	m_irq_enable = (BIT(m_irq_mode, 1) && m_irq_on) ? 1 : 0;
}

void nes_bmc_sb5013_device::m2_w(int state, offs_t address, bool write)
{
	if (!state || BIT(m_irq_mode, 1) || !m_irq_on || !m_m2_count)
		return;
	if (!--m_m2_count)
		set_irq_line(ASSERT_LINE);
}

void nes_bmc_sb5013_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc sb5013 write_h, offset: %04x, data: %02x\n", offset, data);

	const bool pa12 = BIT(m_irq_mode, 1);
	const bool autoflag = BIT(m_irq_mode, 2);
	switch (offset & 0x7003)
	{
		case 0x0000: case 0x0001: case 0x0002: case 0x0003:
			m_prg_reg[offset & 3] = data;
			set_prg(m_prg_base, m_prg_mask);
			break;
		case 0x1000:
			m_prg_outer = data;
			set_prg(m_prg_base, m_prg_mask);
			break;
		case 0x1001:
			m_size = data;
			set_prg(m_prg_base, m_prg_mask);
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;
		case 0x1002:
			switch (data & 3)
			{
				case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;
		case 0x1003:
			m_chr_outer = data;
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;
		case 0x2000: case 0x2001: case 0x2002: case 0x2003:
		case 0x3000: case 0x3001: case 0x3002: case 0x3003:
			m_chr[((offset >> 10) & 4) | (offset & 3)] = data;
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;
		case 0x4000:
			if (pa12)
				m_irq_clear = 1;
			else
				m_m2_count = (m_m2_count & 0xff00) | data;
			if (autoflag)
				m_irq_on = false;
			break;
		case 0x4001:
			if (pa12)
				m_irq_count_latch = data;
			else
				m_m2_count = (m_m2_count & 0x00ff) | (data << 8);
			if (autoflag)
				m_irq_on = true;
			break;
		case 0x4002:
			m_irq_mode = data & 7;
			m_irq_on = BIT(data, 0);
			break;
		case 0x4003:
			if (!autoflag)
				m_irq_on = BIT(data, 0);
			break;
		default:
			return;
	}
	if ((offset & 0x7000) == 0x4000)
	{
		if (!m_irq_on || !BIT(m_irq_mode, 1))
			set_irq_line(CLEAR_LINE);
		update_irq_enable();
	}
}

/*-------------------------------------------------

 UNL-82112C

 Games: Master Fighter VI' (EJ-4003)

 The BMC-SB-5013 board above (PRG banks and outer bank,
 mirroring, the M2 or PPU A12 IRQ), with its CHR registers used
 as four 2 KiB banks: registers 0 and 1 at PPU $0000 and
 $0800, registers 6 and 7 at $1000 and $1800; the CHR outer
 bank and size bits do not apply. FCEUmm's 359.c, which
 implements both mappers, is the only description; no NESdev
 page was found.

 NES 2.0: mapper 540

 In MAME: Supported.

 -------------------------------------------------*/

void nes_unl_82112c_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (!m_vrom_chunks)
	{
		chr8(0, CHRRAM);
		return;
	}
	chr2_0(m_chr[0], CHRROM);
	chr2_2(m_chr[1], CHRROM);
	chr2_4(m_chr[6], CHRROM);
	chr2_6(m_chr[7], CHRROM);
}

/*-------------------------------------------------

 1993 Super Fighter 22-in-1 (NES 2.0 mapper 620)

 Games: 1993 Super Fighter 22-in-1 (Asia) (En) (Unl)

 NESdev has no page for this mapper; it is taken from FCEUmm's
 620.c (MMC3 clone type AX5202P there). An outer register is
 written at $4100-$5FFF and $6000-$7FFF when A8 is set (in the
 $6000 range only while $A001 bit 6 does not write-protect it,
 as FCEUmm's AX5202P passes such writes regardless of bit 7):
 - PRG comes from the outer register alone (the MMC3's PRG banks
   are not used): bit 6 set: one 16 KiB bank
   ((reg >> 3) & $0E) | bit 7 at $8000 and $C000; clear: one
   32 KiB bank (reg >> 4) & 7
 - CHR: the MMC3's 1 KiB banks ANDed with $7F, CHR A17 from bit 6
 Mirroring and the IRQ are the MMC3's (the tree's MMC3, not a
 separate AX5202P model). Power-on and reset value $40 (FCEUmm).

 NES 2.0: mapper 620

 In MAME: Supported.

 -------------------------------------------------*/

nes_bmc_620_device::nes_bmc_620_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_620, tag, owner, clock), m_outer(0x40)
{
}

void nes_bmc_620_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_outer));
}

void nes_bmc_620_device::pcb_reset()
{
	m_outer = 0x40;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_bmc_620_device::prg_cb(int start, int bank)
{
	// the outer register alone selects PRG; start 0 maps the whole window
	if (start != 0)
		return;
	if (BIT(m_outer, 6))
	{
		const int b16 = ((m_outer >> 3) & 0x0e) | BIT(m_outer, 7);
		prg16_89ab(b16);
		prg16_cdef(b16);
	}
	else
		prg32((m_outer >> 4) & 0x07);
}

void nes_bmc_620_device::chr_cb(int start, int bank, int source)
{
	chr1_x(start, (bank & 0x7f) | ((m_outer << 1) & 0x80), source);
}

void nes_bmc_620_device::outer_w(u8 data)
{
	m_outer = data;
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_bmc_620_device::write_l(offs_t offset, u8 data)
{
	LOG("bmc 620 write_l, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset + 0x4100, 8))
		outer_w(data);
}

void nes_bmc_620_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc 620 write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 8) && !BIT(m_wram_protect, 6))
		outer_w(data);
	nes_txrom_device::write_m(offset, data);
}
