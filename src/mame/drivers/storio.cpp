// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    vtech Storio

	Skeleton driver, to reference Software List so that it gets validated

	TODO: everything!

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class vtech_storio_state : public driver_device
{
public:
	vtech_storio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void vtech_storio(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart);

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

DEVICE_IMAGE_LOAD_MEMBER(vtech_storio_state, cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

static INPUT_PORTS_START( vtech_storio )
INPUT_PORTS_END


void vtech_storio_state::vtech_storio(machine_config &config)
{
	//ARM(config, m_maincpu, XTAL(200'000'000)); // What type of CPU?

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(vtech_storio_state::screen_update_storio));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vtech_storio_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(device_image_load_delegate(&vtech_storio_state::device_image_load_cart, this));

	SOFTWARE_LIST(config, "cart_list").set_original("vtech_storio_cart");
}

ROM_START( storio )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	/* load BIOS roms here if it doesn't boot straight from cart? */
ROM_END

// year, name, parent, compat, machine, input, class, init, company, fullname, flags
CONS( 200?, storio,  0,      0, vtech_storio,  vtech_storio, vtech_storio_state, empty_init, "VTech", "Storio",      MACHINE_IS_SKELETON )
