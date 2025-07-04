// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    LeapFrog Leapster Explorer

     - runs Linux
     - unknown ARM9 based SoC

     Internal ROM not currently dumped, this file exists to reference the
     Software List

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class leapfrog_leapster_explorer_state : public driver_device
{
public:
	leapfrog_leapster_explorer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void leapfrog_leapster_explorer(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<cpu_device> m_maincpu;

	required_device<screen_device> m_screen;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;

	uint32_t screen_update_innotab(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t leapfrog_leapster_explorer_state::screen_update_innotab(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void leapfrog_leapster_explorer_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

DEVICE_IMAGE_LOAD_MEMBER(leapfrog_leapster_explorer_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( leapfrog_leapster_explorer )
INPUT_PORTS_END


void leapfrog_leapster_explorer_state::leapfrog_leapster_explorer(machine_config& config)
{
	ARM9(config, m_maincpu, 393000000); // unknown ARM9 type

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320 - 1, 0, 240 - 1);
	m_screen->set_screen_update(FUNC(leapfrog_leapster_explorer_state::screen_update_innotab));

	SPEAKER(config, "speaker", 2).front();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_leapster_explorer_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_leapster_explorer_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("leapster_explorer_cart");
}

ROM_START( leapexpr )
	ROM_REGION( 0x0100000, "maincpu", ROMREGION_ERASEFF )
	// unknown internal ROM
	ROM_LOAD( "internal.rom", 0x000000, 0x0100000, NO_DUMP )
ROM_END

} // anonymous namespace


CONS( 2010, leapexpr,     0,       0,      leapfrog_leapster_explorer, leapfrog_leapster_explorer, leapfrog_leapster_explorer_state, empty_init, "LeapFrog", "Leapster Explorer",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
