// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"
#include "screen.h"


class mobigo_state : public driver_device
{
public:
	mobigo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_screen(*this, "screen")
	{ }

	void mobigo(machine_config &config);

protected:

	void video_start() override;

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	required_device<generic_slot_device> m_cart;
	required_device<screen_device> m_screen;
};


static INPUT_PORTS_START( mobigo )
INPUT_PORTS_END

void mobigo_state::video_start()
{
}

uint32_t mobigo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

DEVICE_IMAGE_LOAD_MEMBER(mobigo_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");
	return image_init_result::PASS;
}

void mobigo_state::mobigo(machine_config &config)
{
	// unSP based (but no Gpnandnand header in NAND rom)

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "mobigo_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(mobigo_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(mobigo_state::screen_update));

	SOFTWARE_LIST(config, "cart_list").set_original("mobigo_cart");
}

ROM_START( mobigo2 )
	ROM_REGION( 0x8400000, "mainrom", 0 )
	ROM_LOAD( "mobigo2_bios_ger.bin", 0x00000, 0x8400000, CRC(d5ab613d) SHA1(6fb104057dc3484fa958e2cb20c5dd0c19589f75) ) // SPANSION S34ML01G100TF100  
ROM_END


CONS( 200?, mobigo2, 0, 0, mobigo,  mobigo, mobigo_state, empty_init, "VTech", "MobiGo 2 (Germany)", MACHINE_IS_SKELETON )
