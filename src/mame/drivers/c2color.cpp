// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    basic information
    https://gbatemp.net/threads/the-c2-color-game-console-an-obscure-chinese-handheld.509320/

    "The C2 is a glorious console with a D-Pad, Local 2.4GHz WiFi, Cartridge slot, A, B, and C buttons,
     and has micro usb power! Don't be fooled though, there is no lithium battery, so you have to put in
     3 AA batteries if you don't want to play with it tethered to a charger.

     It comes with a built in game based on the roco kingdom characters.

     In addition, there is a slot on the side of the console allowing cards to be swiped through. Those
     cards can add characters to the game. The console scans the barcode and a new character or item appears in the game for you to use.

     The C2 comes with 9 holographic game cards that will melt your eyes."

    also includes a link to the following video
    https://www.youtube.com/watch?v=D3XO4aTZEko

    TODO:
    identify CPU type, and if the system ROM is needed to run carts or not

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "emupal.h"
#include "softlist.h"
#include "speaker.h"

class c2_color_state : public driver_device
{
public:
	c2_color_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
	{ }

	void c2_color(machine_config &config);
	void leapfrog_mfleappad(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

uint32_t c2_color_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}




void c2_color_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	}
}

void c2_color_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(c2_color_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

static INPUT_PORTS_START( c2_color )
INPUT_PORTS_END

void c2_color_state::c2_color(machine_config &config)
{
	// unknown CPU

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(c2_color_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "c2color_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(c2_color_state::cart_load), this);

	SOFTWARE_LIST(config, "cart_list").set_original("c2color_cart");
}

ROM_START( c2color )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "system.rom", 0x000000, 0x400000, NO_DUMP ) // must have an internal rom for the built in game, unknown size etc.
ROM_END

//    year, name,         parent,  compat, machine,      input,        class,              init,       company,  fullname,                             flags
CONS( 201?, c2color,      0,       0,      c2_color,   c2_color, c2_color_state, empty_init, "Baiyi Animation", "C2 Color (China)", MACHINE_IS_SKELETON )
