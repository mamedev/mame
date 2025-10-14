// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    VTech Storio and VTech V.Reader
    Main processor: Nuvoton W55FA9363SDN (ARM926EJ-S CPU core)
        http://www.dingsung.com.cn/download/n32905/1507301944.pdf

    Storio (Europe) and V.Reader (North America) cartridges are physically
    different, but there is no software region lock, so you can play
    Storio games on the V.Reader (and viceversa) by modifying the game's
    plastic cartrige (so it can fit in).

    Skeleton driver, to reference Software List so that it gets validated

    TODO: everything!

    NAND types

    Storio Spanish old BIOS TC58NVG0S3ETA00 (2048+64) x 64 x 1024

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class vtech_storio_state : public driver_device
{
public:
	vtech_storio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void vtech_storio(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void vtech_storio_base(machine_config &config);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<cpu_device> m_maincpu;

	required_device<screen_device> m_screen;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;

	uint32_t screen_update_storio(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t vtech_storio_state::screen_update_storio(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void vtech_storio_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	}
}

void vtech_storio_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(vtech_storio_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( vtech_storio )
INPUT_PORTS_END


void vtech_storio_state::vtech_storio_base(machine_config &config)
{
	ARM9(config, m_maincpu, 240000000); // ARM926EJ-S CPU core (probably 240MHz, but not sure)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(vtech_storio_state::screen_update_storio));

	SPEAKER(config, "speaker", 2).front();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vtech_storio_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(vtech_storio_state::cart_load));

}

void vtech_storio_state::vtech_storio(machine_config &config)
{
	vtech_storio_base(config);
	SOFTWARE_LIST(config, "cart_list").set_original("vtech_storio_cart");
}

// BIOS is 1 GBIT (128M × 8 BIT) CMOS NAND EEPROM (Toshiba TC58NVG0S3ETA00)

// ROM image from VTech, not padded to the real ROM size
ROM_START( vreader )
	ROM_REGION( 0x038e906c, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "useng-pack_20111017.bin", 0x000000, 0x038e906c, CRC(add3f7e5) SHA1(43ecfb0ba3c98c5852f93ed620021697167aa156) )
ROM_END

// ROM image from VTech, not padded to the real ROM size
ROM_START( vreadercaen )
	ROM_REGION( 0x038e906c, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "caeng-pack_20111017.bin", 0x000000, 0x038e906c, CRC(0b64caf3) SHA1(79648e2b315c59f60aaf8cb8806fdbe773e484a2) )
ROM_END

// ROM image from VTech, not padded to the real ROM size
ROM_START( vreadercafr )
	ROM_REGION( 0x037d93a6, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "cafre-pack_20111017.bin", 0x000000, 0x037d93a6, CRC(d3e0039c) SHA1(3d69f4afcf56ba40261bba0af335680c3c05b319) )
ROM_END

// ROM image from VTech, not padded to the real ROM size
ROM_START( storio )
	ROM_REGION( 0x01bf3dcb, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gbeng-pack_20111017.bin", 0x000000, 0x01bf3dcb, CRC(b0962d2c) SHA1(4f316cbcc87ae24022568a358ac94c7b4cac39a6) )
ROM_END

// ROM image from VTech, not padded to the real ROM size
ROM_START( storiode )
	ROM_REGION( 0x03740a0d, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "deger-pack_20111017.bin", 0x000000, 0x03740a0d, CRC(548c8882) SHA1(e64474be082bd3ae3c365c6c766b2ec5081f3ebd) )
ROM_END

// ROM image from VTech, not padded to the real ROM size
ROM_START( storioes )
	ROM_REGION( 0x03c62bfc, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "esspa-pack_20111017.bin", 0x000000, 0x03c62bfc, CRC(fe9b78f9) SHA1(c114a8f82799861a0cca432ee145e436aca5f400) )
ROM_END

// ROM image dumped from a real Spanish VTech Storio console.
// Seems to be the "2011.06.17" compilation, although there is a "Copyright (c) 2009 - 2012 Nuvoton" text on the ROM
ROM_START( storioesa )
	ROM_REGION( 0x08400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "storiospanisholdbios.bin", 0x000000, 0x08400000, CRC(c462cac4) SHA1(37e5497342a3a27366288b5c5dffd00d0826e183) )
ROM_END

// ROM image from VTech, not padded to the real ROM size
ROM_START( storiofr )
	ROM_REGION( 0x038c2a19, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "frfre-pack_20111017.bin", 0x000000, 0x038c2a19, CRC(f3d87f50) SHA1(240ddebd4cb1c4be24afb4da35c65ddf64628034) )
ROM_END

// ROM image from VTech, not padded to the real ROM size
ROM_START( storionl )
	ROM_REGION( 0x03af81c6, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "nldut-pack_20111017.bin", 0x000000, 0x03af81c6, CRC(6cfac599) SHA1(d16b45fd287c9d823bde13b88eb6c8158ac2b475) )
ROM_END

} // anonymous namespace


//    year, name,         parent,  compat, machine,      input,        class,              init,       company,  fullname,                             flags
CONS( 2011, vreader,      0,       0,      vtech_storio, vtech_storio, vtech_storio_state, empty_init, "VTech", "V.Reader (US, English, 2011-10-17)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, vreadercaen,  vreader, 0,      vtech_storio, vtech_storio, vtech_storio_state, empty_init, "VTech", "V.Reader (CA, English, 2011-10-17)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, vreadercafr,  vreader, 0,      vtech_storio, vtech_storio, vtech_storio_state, empty_init, "VTech", "V.Reader (CA, French, 2011-10-17)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, storio,       vreader, 0,      vtech_storio, vtech_storio, vtech_storio_state, empty_init, "VTech", "Storio (GB, English, 2011-10-17)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, storiode,     vreader, 0,      vtech_storio, vtech_storio, vtech_storio_state, empty_init, "VTech", "Storio (DE, German, 2011-10-17)",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, storioes,     vreader, 0,      vtech_storio, vtech_storio, vtech_storio_state, empty_init, "VTech", "Storio (ES, Spanish, 2011-10-17)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, storioesa,    vreader, 0,      vtech_storio, vtech_storio, vtech_storio_state, empty_init, "VTech", "Storio (ES, Spanish, 2011-06-17?)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, storiofr,     vreader, 0,      vtech_storio, vtech_storio, vtech_storio_state, empty_init, "VTech", "Storio (FR, French, 2011-10-17)",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2011, storionl,     vreader, 0,      vtech_storio, vtech_storio, vtech_storio_state, empty_init, "VTech", "Storio (NL, Dutch, 2011-10-17)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
