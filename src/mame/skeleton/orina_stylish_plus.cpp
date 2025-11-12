// license:BSD-3-Clause
// copyright-holders:

/*
    This is a placeholder driver for the Orina Stylish Plus so that a
    Software List for the cartridges can exist.

    Internal hardware of machine is currently unknown
*/

#include "emu.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"


namespace {

class orinasp_state : public driver_device
{
public:
	orinasp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{ }

	void orinasp(machine_config &config);
protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	optional_device<generic_slot_device> m_cart;
};


static INPUT_PORTS_START( orinasp )
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(orinasp_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");
	return std::make_pair(std::error_condition(), std::string());
}

void orinasp_state::orinasp(machine_config &config)
{
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "orina_stylish_plus_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(orinasp_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("orina_stylish_plus_cart");
}

ROM_START( orinasp )
ROM_END

} // anonymous namespace


CONS( 2021, orinasp, 0, 0, orinasp,  orinasp, orinasp_state, empty_init, "Takara Tomy", "Orina Stylish+ (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
