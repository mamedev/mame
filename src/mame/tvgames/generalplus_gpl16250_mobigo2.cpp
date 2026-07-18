// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
	Known Undumped:
	MobiGo 2 (USA, English)
	MobiGo 2 (UK and Australia, English)
    MobiGo 2 (Canada, English)
*/

#include "emu.h"

#include "generalplus_gpl16250_spi.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "machine/nandflash.h"

#include "softlist_dev.h"

namespace {

class generalplus_mobigo2_state : public generalplus_gpspispi_game_state
{
public:
	generalplus_mobigo2_state(const machine_config &mconfig, device_type type, const char *tag) :
		generalplus_gpspispi_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_nand(*this, "nandrom")
	{
	}

	void mobigo2(machine_config &config) ATTR_COLD;

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
	required_device<nand_device> m_nand;
};

static INPUT_PORTS_START( mobigo2 )
	PORT_START("IN0")
	PORT_START("IN1")
	PORT_START("IN2")
INPUT_PORTS_END


DEVICE_IMAGE_LOAD_MEMBER(generalplus_mobigo2_state::cart_load)
{
	u32 const size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");
	return std::make_pair(std::error_condition(), std::string());
}


void generalplus_mobigo2_state::mobigo2(machine_config &config)
{
	generalplus_gpspispi(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "mobigo_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(generalplus_mobigo2_state::cart_load));
	//m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("mobigo_cart");

	SAMSUNG_K9F1G08U0M(config, m_nand); // 128Mbyte part, with 0x800+0x40 sized pages
}


ROM_START( mobigo2 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00)	// appears to be the boot ROM, has GPspispi header
	ROM_LOAD16_WORD_SWAP( "n25s16.u3", 0x00000, 0x200000, CRC(dfbf5029) SHA1(2a079ddd8a13c5f3d8f40fa6d6c3de2dc1573449) )

	ROM_REGION( 0x8400000, "nandrom", ROMREGION_ERASE00 ) // no GPnandnand header so not a boot device
	ROM_LOAD( "mobigo2_bios_ger.bin", 0x00000, 0x8400000, CRC(d5ab613d) SHA1(6fb104057dc3484fa958e2cb20c5dd0c19589f75) ) // SPANSION S34ML01G100TF100
ROM_END

} // anonymous namespace

// ----------------------------------------------------
// these set SP to 6fff so must be 'GPL162xx A' type
//
// could be GPL16230A, GPL16240A or GPL16250A
// ----------------------------------------------------

CONS( 2013, mobigo2, 0,      0, mobigo2,  mobigo2, generalplus_mobigo2_state, init_spi, "VTech", "MobiGo 2 (Germany)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
