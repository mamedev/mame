// license:BSD-3-Clause
// copyright-holders:David Haywood

// Yeno educational laptops
//
// The CPU is marked KS56C1660-12
// ROM is a glob near the CPU but appears to contain only data(?)
//
// The only reference to KS56C1660 appears to be a Samsung manual
// https://bitsavers.trailing-edge.com/components/samsung/_Databooks/1990_Samsung_Semiconductor_Product_Guide.pdf
// which suggests this is a 4-bit MCU with 16256 bytes of internal ROM

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"

#include "screen.h"
#include "softlist_dev.h"

namespace {

class yeno_ks56c1660_state : public driver_device
{
public:
	yeno_ks56c1660_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_screen(*this, "screen")
	{
	}

	void yeno(machine_config &config) ATTR_COLD;

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<generic_slot_device> m_cart;
	required_device<screen_device> m_screen;
};

uint32_t yeno_ks56c1660_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START(yeno)
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(yeno_ks56c1660_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void yeno_ks56c1660_state::yeno(machine_config &config)
{

	// YENO 9205 KS56C1660-12

	// monochrome LCD display, resolution unknown
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(FUNC(yeno_ks56c1660_state::screen_update));
	m_screen->set_size(120, 18);
	m_screen->set_visarea_full();

	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "yeno_laptop_cart");
	m_cart->set_device_load(FUNC(yeno_ks56c1660_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("yeno_laptop_cart");
}

ROM_START(misterx2)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD("ks56c1660", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION(0x40000, "data", 0)
	ROM_LOAD("misterx2ger.bin", 0x00000, 0x40000, CRC(b6b23552) SHA1(1c1b345de3b12a468c47ea8aa2281156eb21f12b) )
ROM_END

ROM_START(intelct2)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD("ks56c1660", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION(0x40000, "data", 0)
	ROM_LOAD("intellectus2.bin", 0x00000, 0x40000, CRC(baf94d1a) SHA1(3f71697f21a507856410c831c8348bdc6f45ac90) )
ROM_END

ROM_START(ckidpers)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD("ks56c1660", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION(0x40000, "data", 0)
	ROM_LOAD("computerkidpersonal.bin", 0x00000, 0x40000, CRC(7a2a0ee7) SHA1(61fdd82a0cbd6bace0742cb9d9c25d372827b452) )
ROM_END

} // anonymous namespace

CONS( 199?, misterx2,  0,          0,  yeno,  yeno, yeno_ks56c1660_state, empty_init, "Yeno",              "Mister X2 (Germany)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 199?, intelct2,  misterx2,   0,  yeno,  yeno, yeno_ks56c1660_state, empty_init, "Yeno",              "Intellectus 2 (France)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 199?, ckidpers,  misterx2,   0,  yeno,  yeno, yeno_ks56c1660_state, empty_init, "Yeno / Clementoni", "Computer Kid Personal (Italy)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
