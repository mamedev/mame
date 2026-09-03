// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    GPL16250 / GPAC800 / GMC384 / GCM420 related support

    GPL16250 is the GeneralPlus / SunPlus part number
    GPAC800 is the JAKKS Pacific codename
    GMC384 / GCM420 is what is printed on the die

    ----

    GPL16250 Mobigo support
    the original Mobigo is ROM+RAM config
    the Mobigo 2 is SPI+RAM+NAND config (see generalplus_gpl16250_mobigo2.cpp)
    cartridges are compatible

    Known Undumped:
    MobiGo (UK and Australia, English)
    MobiGo (Canada, English)
    MobiGo (Canada, French)
*/

#include "emu.h"
#include "generalplus_gpl16250_nand.h"
#include "generalplus_gpl16250_romram.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"

#include "screen.h"
#include "softlist_dev.h"


namespace {


class mobigo_state : public wrlshunt_game_state
{
public:
	mobigo_state(const machine_config &mconfig, device_type type, const char *tag) :
		wrlshunt_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{ }

	void mobigo(machine_config &config) ATTR_COLD;
	void init_mobigo() ATTR_COLD;

protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
};


static INPUT_PORTS_START( mobigo )
	PORT_START("IN0")
	PORT_START("IN1")
	PORT_START("IN2")
INPUT_PORTS_END




DEVICE_IMAGE_LOAD_MEMBER(mobigo_state::cart_load)
{
	u32 const size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");
	return std::make_pair(std::error_condition(), std::string());
}


void mobigo_state::mobigo(machine_config &config)
{
	gcm394_game_state::base(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "mobigo_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(mobigo_state::cart_load));
	//m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("mobigo_cart");
}

void mobigo_state::init_mobigo()
{
	m_sdram.resize(0x400000); // 0x400000 bytes, 0x800000 words (needs verifying)
}

// MobiOURS0.1 versions

ROM_START( mobigo ) // with additional built-in game
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00)
	// MobiOURS0.1  2010-01-02
	ROM_LOAD16_WORD_SWAP("mobigo.bin", 0x0000, 0x800000, CRC(49479bad) SHA1(4ee82c7ba13072cf25a34893cf6272f2da5d8928) )
ROM_END

ROM_START( mobigouk ) // no built in game
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00)
	// MobiOURS0.1  2010-01-02
	ROM_LOAD16_WORD_SWAP("mobiours0.1_2010-01-02_uk.u3", 0x0000, 0x200000, CRC(5dfe21b0) SHA1(bdbf75909c96c6cd3636ac8be6fde4646b83ca6b) )

	ROM_REGION( 0x800000, "i2mem", ROMREGION_ERASE00) // for settings
	ROM_LOAD("k24c08c.u7", 0x000, 0x400, CRC(dea8dad3) SHA1(81409019a262a6e410d7ac2cbdc99680e73ba1a9) )
ROM_END

// MobiOURS1.0 versions

ROM_START( mobigos )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00)
	// MobiOURS1.0  2010-07-21
	ROM_LOAD16_WORD_SWAP("mobigospanish.bin", 0x0000, 0x200000, CRC(462b4f9d) SHA1(1541152f1a359bc18de4d4f3d5038a954c9a3ad4) )

	ROM_REGION( 0x800000, "spi", ROMREGION_ERASE00) // for built-in games and/or downloads? (not a boot ROM)
	ROM_LOAD16_WORD_SWAP("mx25l1606e.u3", 0x0000, 0x200000, NO_DUMP ) // assume the 1.0 sets should have this
ROM_END

ROM_START( mobigouka )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00)
	// MobiOURS1.0  2010-07-21
	ROM_LOAD16_WORD_SWAP("mobigo_uk.u5", 0x0000, 0x200000, CRC(efd27779) SHA1(b8f99a5c05d0fc8bbaf22996f9e699e0ea9ab4da) )

	ROM_REGION( 0x800000, "spi", ROMREGION_ERASE00) // for built-in games and/or downloads? (not a boot ROM)
	ROM_LOAD16_WORD_SWAP("mx25l1606e.u3", 0x0000, 0x200000, CRC(94a1b745) SHA1(a99ef67a6cbf93dd33ee2756627fe96abfe8690e) )
ROM_END

ROM_START( mobigoukb )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00)
	// MobiOURS1.0  2010-07-21
	ROM_LOAD16_WORD_SWAP("mobigo_uk.u5", 0x0000, 0x200000, CRC(efd27779) SHA1(b8f99a5c05d0fc8bbaf22996f9e699e0ea9ab4da) )

	ROM_REGION( 0x800000, "spi", ROMREGION_ERASE00) // for built-in games and/or downloads? (not a boot ROM)
	// only this differs from mobigouk  TODO: check if there's anything important stored on either
	ROM_LOAD16_WORD_SWAP("mx25l1606e.u3", 0x0000, 0x200000, CRC(15fa384f) SHA1(0fb5115ad19dcc8a190fcef476775aaca4591aad) )
ROM_END

ROM_START( mobigof )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00)
	// MobiOURS1.0  2010-07-21
	ROM_LOAD16_WORD_SWAP("mobigo_fr.u5", 0x0000, 0x200000, CRC(61d739aa) SHA1(d23d4c806f1b60058c57ea9b339a7c5e3124bafa))

	ROM_REGION( 0x800000, "spi", ROMREGION_ERASE00) // for built-in games and/or downloads? (not a boot ROM)
	ROM_LOAD16_WORD_SWAP("mx25l1606e.u3", 0x0000, 0x200000, CRC(31110b90) SHA1(b6a3b707d9ff688635be8bc01221b4db503ce159))
ROM_END

} // anonymous namespace

// ----------------------------------------------------
// these set SP to 6fff so must be 'GPL162xx A' type
//
// could be GPL16230A, GPL16240A or GPL16250A
// ----------------------------------------------------

// MobiOURS0.1 versions (released in 2010, only has SEEPROM for settings, no downloads)
CONS( 2010, mobigo,    0,      0, mobigo, mobigo, mobigo_state, init_mobigo, "VTech", "MobiGo (USA, version 0.1)",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2010, mobigouk,  mobigo, 0, mobigo, mobigo, mobigo_state, init_mobigo, "VTech", "MobiGo (UK, version 0.1)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// MobiOURS1.0 versions (released in 2011, with SPI ROM for settings/downloads)
CONS( 2011, mobigouka, mobigo, 0, mobigo, mobigo, mobigo_state, init_mobigo, "VTech", "MobiGo (UK, version 1.0, set 1)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, mobigoukb, mobigo, 0, mobigo, mobigo, mobigo_state, init_mobigo, "VTech", "MobiGo (UK, version 1.0, set 2)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, mobigos,   mobigo, 0, mobigo, mobigo, mobigo_state, init_mobigo, "VTech", "MobiGo (Spain, version 1.0)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, mobigof,   mobigo, 0, mobigo, mobigo, mobigo_state, init_mobigo, "VTech", "MobiGo (France, version 1.0)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
