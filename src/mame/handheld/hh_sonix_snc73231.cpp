// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

	Sonix SNC73231 (Dual-Core ARM SoC) + SNAUD01 (audio)

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"


namespace {

class precur2w_state : public driver_device
{
public:
	precur2w_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cart(*this, "cartslot")
	{ }

	void precur2w(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<generic_slot_device> m_cart;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void arm_map(address_map &map) ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
};

uint32_t precur2w_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void precur2w_state::machine_start()
{
}

void precur2w_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(precur2w_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( precur2w )
INPUT_PORTS_END

void precur2w_state::arm_map(address_map &map)
{
}

void precur2w_state::precur2w(machine_config &config)
{
	ARM9(config, m_maincpu, 72000000); // Sonix SNC73231 (Dual-Core ARM SoC), unknown frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &precur2w_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(precur2w_state::screen_update));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "precur2w_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(precur2w_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("precur2w_cart");
}

ROM_START( precur2w )
	// likely an internal ROM to bootstrap SPI ROM below (has header with offsets of code in it)

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "by25q64ess.bin", 0x000000, 0x800000, CRC(35351cd0) SHA1(0b07a613c3e9b638e531ba668d72ebefd459d0d4) )
ROM_END

} // anonymous namespace

// "Muscat A 2022 01 15" on PCB
CONS( 2022, precur2w, 0, 0, precur2w, precur2w, precur2w_state, empty_init, "Bandai", "PreCure 2-Way Heart (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

