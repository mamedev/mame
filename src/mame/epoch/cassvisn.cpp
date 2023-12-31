// license:BSD-3-Clause
// copyright-holders:

/*

for architecture details see
https://www.oguchi-rd.com/LSI%20products.php
the linked 'design note' contains a large amount of useful information
https://www.oguchi-rd.com/777/777%20Design%20Note.pdf

*/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/upd777/upd777.h"
#include "cpu/upd777/upd777dasm.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"


namespace {

class cassvisn_state : public driver_device
{
public:
	cassvisn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void cassvisn(machine_config &config);

	void init_cass();
protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

uint32_t cassvisn_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


// documentation says patterns 0x00 - 0x6e are 7x7
// and patterns 0x70 - 0x7e are 8x7
// but they all seem to be stored at 11x7, just with some columns blank?
static const gfx_layout test_layout =
{
	11,7,
	0x80,
	8,
	{ 0 },
	{ 0,1,2,3,4,5,6,7,8,9,10 },
	{ 0*11,1*11,2*11,3*11,4*11,5*11,6*11 },
	7*11
};


static GFXDECODE_START( gfx_cassvisn )
	GFXDECODE_ENTRY( "patterns", 0, test_layout, 0, 1  )
GFXDECODE_END


static INPUT_PORTS_START( cassvisn )
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(cassvisn_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("prg");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "prg");
	memcpy(memregion("maincpu")->base(), m_cart->get_rom_base(), size);

	size = m_cart->common_get_size("pat");
	m_cart->common_load_rom(memregion("patterns")->base(), size, "pat");
	return std::make_pair(std::error_condition(), std::string());
}

void cassvisn_state::cassvisn(machine_config &config)
{
	UPD777(config, m_maincpu, 1000000); // frequency? UPD774 / UPD778 in some carts?

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "cassvisn_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(cassvisn_state::cart_load));
	m_cart->set_must_be_loaded(true);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(cassvisn_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cassvisn);
	PALETTE(config, m_palette).set_entries(0x1000);

	SOFTWARE_LIST(config, "cart_list").set_original("cassvisn_cart");
}

ROM_START( cassvisn )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_REGION( 0x4d0, "patterns", ROMREGION_ERASEFF )
ROM_END

void cassvisn_state::init_cass()
{
}

} // anonymous namespace

CONS( 1981, cassvisn, 0, 0, cassvisn,  cassvisn, cassvisn_state, init_cass, "Epoch", "Cassette Vision", MACHINE_IS_SKELETON )
