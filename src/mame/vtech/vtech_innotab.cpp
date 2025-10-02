// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    VTech InnoTab 1/2/3
    NOT InnoTab MAX

    InnoTab 1/2/3 appear to be compatible with each other (updated internal
    software etc.)

    where do the InnoTab 3S and InnoTab 2 Baby fit in?

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class vtech_innotab_state : public driver_device
{
public:
	vtech_innotab_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void vtech_innotab(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<cpu_device> m_maincpu;

	required_device<screen_device> m_screen;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;

	uint32_t screen_update_innotab(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t vtech_innotab_state::screen_update_innotab(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void vtech_innotab_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

DEVICE_IMAGE_LOAD_MEMBER(vtech_innotab_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( vtech_innotab )
INPUT_PORTS_END


void vtech_innotab_state::vtech_innotab(machine_config& config)
{
	ARM9(config, m_maincpu, 240000000); // unknown ARM type

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320 - 1, 0, 240 - 1);
	m_screen->set_screen_update(FUNC(vtech_innotab_state::screen_update_innotab));

	SPEAKER(config, "speaker", 2).front();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vtech_innotab_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(vtech_innotab_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("vtech_innotab_cart");
}

/*
**************************************************************
APP Version : 10.90
     Device : THGBM4G4D1HBAIR(ISP)_4Bit
**************************************************************


File Name :
*******************************************************************
D:\Xgpro\UserData\EMMC_Data
ECSD_CSD.BIN
*******************************************************************


  <1> -- Production Info.and Device life time--

      MID          : 11
      PNM          : 002G49
      Product Date : 5-2012
      Version      : MMC V4.41

      Device life time Type A : Not defined
      Device life time Type B : Not defined
      Device life time (PRE_EOL_INFO) : Not Defined

  <2> -- Partition Size Info.--

      BOOT1 SIZE   : 1024 KB
      BOOT2 SIZE   : 1024 KB
      RPMB SIZE    : 128 KB
      GPP1 SIZE    : 0 KB
      GPP2 SIZE    : 0 KB
      GPP3 SIZE    : 0 KB
      GPP4 SIZE    : 0 KB
      USER SIZE    : 1,916,928 KB
                      ( 0x 00_7500_0000 )
      Password Protect Features : YES

  <5> -- other Informations --

      MAX_READ_BL_LEN   : 1024 bytes
      MAX_WRITE_BL_LEN  : 512 bytes
      MAX_TRAN_SPEED    : 55.000 MHZ

      BOOT_BUS_CONDITIONS[177]    : 00
      BOOT_CONFIG_PROT[178]       : 00
      PARTITION_CONFIG[179]       : 00
      RST_n_FUNCTION[162]         : 00

      ENH_START_ADDR              : 00000000
      ENH_SIZE_MULT               : 000000
      MAX_ENH_SIZE_MULT           : 0003A8
      PARTITIONS_ATTRIBUTE        : 00
      WR_REL_SET                  : 00
      WR_REL_PARAM                : 05
      PARTITION_SETTING_COMPLETED : 00

      HC_WP_GRP_SIZE    : 1
      HC_ERASE_GRP_SIZE : 2
      WP_GRP_ENABLE     : 1
      WP_GRP_SIZE       : 1
      ERASE_GRP_MULT    : 31
      ERASE_GRP_SIZE    : 31
      CCC               : 00F5
      DSR implemented   : 0
      PARTITION_ACCESS  : 10 ms
      ERASED_MEM_CONT   : 01
      DYNCAP_NEEDED     : 00
      SECURE_WP_INFO    : 00
      SEC_ERASE_MULT    : 10
      ERASE_TIMEOUT_MULT: 02
      NATIVE_SECTOR_SIZE: 00 (512B)
      INI_TIMEOUT_AP    : 3000 ms
      INI_TIMEOUT_EMU       : 0 ms

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------

**************************************************************
APP Version : 10.90
     Device : THGBM4G4D1HBAIR(ISP)_4Bit
**************************************************************

Init EMMC... OK!    ( OCR register: 80FF8080 )
Verifing CSD Succeeded
Verifing ECSD Succeeded
Verifing BOOT1 : Succeeded. Time : 0. 47 S -- Partition Size :1024 KB Processing Size from the File : 1024 KB )
Verifing BOOT2 : Succeeded. Time : 0. 47 S -- Partition Size :1024 KB Processing Size from the File : 1024 KB )
Analysis file is complete, Space usage: 20.26% time: 13S
Verifing User Area : Succeeded. Time : 36.531 S -- Partition Size :1916928 KB( Processing Size from the File : 1916928 KB )
32 bits CheckSum :  0x 86F14222

*/

ROM_START( innotab2 )
	ROM_REGION( 0x0100000, "maincpu", ROMREGION_ERASEFF )
	// are there any other dumpable devices?, or internal ROM in the CPU for booting from?


	// this uses a "eMMC" type ROM chip, should it be treated as a CHD, or like a NAND ROM?
	ROM_REGION( 0x0100000, "emmc_boot", ROMREGION_ERASEFF )
	// these are both blank, unused, or read protected in some way?
	//ROM_LOAD( "boot1.bin", 0x000000, 0x0100000, CRC(956bac74) SHA1(bf0b121670df23f2cc64302d9f215e7c81187bbb) ) // FIXED BITS (11111111)
	//ROM_LOAD( "boot2.bin", 0x000000, 0x0100000, CRC(956bac74) SHA1(bf0b121670df23f2cc64302d9f215e7c81187bbb) ) // FIXED BITS (11111111)

	ROM_REGION( 0x220, "emmc_misc1", ROMREGION_ERASEFF )
	ROM_LOAD( "ecsd_csd.bin", 0x000000, 0x220, CRC(a30bcb97) SHA1(ba83c5b2c73f26ad89ac7cc44b0ea6971050cfa4) )

	ROM_REGION( 0x75000000, "emmc_user", ROMREGION_ERASEFF )
	ROM_LOAD( "userdata.bin", 0x000000, 0x75000000, CRC(3c063d5d) SHA1(41a980b9e19e9fdf00f5175bf332b50f741aecb9) )

	//ROM_REGION( 0x3712, "emmc_misc2", ROMREGION_ERASEFF )
	// this appears to be a project file used by the dumping software, not a ROM
	//ROM_LOAD( "emmc_ghost.mpj", 0x000000, 0x3712, CRC(16b705da) SHA1(fdb576385cf46984ea40d8e8b83758d94f67507e) )
ROM_END

} // anonymous namespace


CONS( 2011, innotab2,     0,       0,      vtech_innotab, vtech_innotab, vtech_innotab_state, empty_init, "VTech", "InnoTAB 2 (UK)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
