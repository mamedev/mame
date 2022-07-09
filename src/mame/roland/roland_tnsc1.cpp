// license:BSD-3-Clause
// copyright-holders:

/*
    This is just a holder for the Roland "Style Data ROM" Software List to ensure they aren't orphaned
    These "Style Data ROM" cards are used by various devices, but none of the devices have been dumped

    Once a supported system is dumped this can be removed and the list can be hooked up to that

    Possible systems:
      Roland E-5
      Roland E-20
      Roland E-30
      Roland E-35
      Roland E-70
      Roland Pro-E
      Roland E/RA-50
      Roland RA-90
      Roland CA-30
      Roland KR-500
      Roland KR-3000
*/

#include "emu.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"


class rlndtnsc1_state : public driver_device
{
public:
	rlndtnsc1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{ }

	void rlndtnsc1(machine_config &config);
protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	optional_device<generic_slot_device> m_cart;
};


static INPUT_PORTS_START( rlndtnsc1 )
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(rlndtnsc1_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");
	return image_init_result::PASS;
}

void rlndtnsc1_state::rlndtnsc1(machine_config &config)
{
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "roland_tnsc1");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(rlndtnsc1_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("roland_tnsc1");
}

ROM_START( rlndtnsc1 )
ROM_END


CONS( 198?, rlndtnsc1, 0, 0, rlndtnsc1, rlndtnsc1, rlndtnsc1_state, empty_init, "Roland", "Roland Music Style Card Software List holder", MACHINE_IS_SKELETON )
