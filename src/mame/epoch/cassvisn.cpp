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
#include "softlist_dev.h"

namespace {

class cassvisn_state : public driver_device
{
public:
	cassvisn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{ }

	void cassvisn(machine_config &config);
protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	required_device<generic_slot_device> m_cart;
};

static INPUT_PORTS_START( cassvisn )
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(cassvisn_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");
	return std::make_pair(std::error_condition(), std::string());
}

void cassvisn_state::cassvisn(machine_config &config)
{
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "cassvisn_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(cassvisn_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("cassvisn_cart");
}

ROM_START( cassvisn )
ROM_END

} // anonymous namespace

CONS( 1981, cassvisn, 0, 0, cassvisn,  cassvisn, cassvisn_state, empty_init, "Epoch", "Cassette Vision", MACHINE_IS_SKELETON )
