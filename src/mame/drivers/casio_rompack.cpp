// license:BSD-3-Clause
// copyright-holders:

/*
    This is just a holder for the Casio ROM Pack Software List to ensure they aren't orphaned
    These ROM Packs are used by various devices, but none of the devices have been dumped

    Once a supported system is dumped this can be removed and the list can be hooked up to that

    Possible systems:

    TODO
*/

#include "emu.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"


class casiorom_state : public driver_device
{
public:
	casiorom_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{ }

	void casiorom(machine_config &config);
protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	optional_device<generic_slot_device> m_cart;
};


static INPUT_PORTS_START( casiorom )
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(casiorom_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");
	return image_init_result::PASS;
}

void casiorom_state::casiorom(machine_config &config)
{
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "casio_rompack");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(casiorom_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("casio_rompack");
}

ROM_START( casiorom )
ROM_END


CONS( 198?, casiorom, 0, 0, casiorom,  casiorom, casiorom_state, empty_init, "Casio", "Casio ROM Pack Software List holder", MACHINE_IS_SKELETON )
