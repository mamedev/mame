// license:BSD-3-Clause
// copyright-holders:

/*
    This is just a holder for the Roland "Style Data ROM" Software List to ensure they aren't orphaned
    These "Style Data ROM" cards are used by various devices, but none of the devices have been dumped

    Once a supported system is dumped this can be removed and the list can be hooked up to that

    Possible systems:
      Roland E-35
      Roland E-36
      Roland E-56
      Roland E-70
      Roland RA-90
      Roland KR-650
      Roland KR-3500
      Roland KR-4500
      Roland KR-5500

    The ROM dump of TN-SC2-04 begins with "Roland E-70A", so this was probably the first system the cards were intended for.
*/

#include "emu.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"


namespace {

class rlndtnsc2_state : public driver_device
{
public:
	rlndtnsc2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{ }

	void rlndtnsc2(machine_config &config);
protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	optional_device<generic_slot_device> m_cart;
};


static INPUT_PORTS_START( rlndtnsc2 )
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(rlndtnsc2_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");
	return std::make_pair(std::error_condition(), std::string());
}

void rlndtnsc2_state::rlndtnsc2(machine_config &config)
{
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "roland_tnsc2");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(rlndtnsc2_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("roland_tnsc2");
}

ROM_START( rlndtnsc2 )
ROM_END

} // anonymous namespace


CONS( 198?, rlndtnsc2, 0, 0, rlndtnsc2, rlndtnsc2, rlndtnsc2_state, empty_init, "Roland", "Roland Music Style Card TN-SC2 Software List holder", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
