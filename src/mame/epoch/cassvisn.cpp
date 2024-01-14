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

#include "softlist_dev.h"


namespace {

class cassvisn_state : public driver_device
{
public:
	cassvisn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot")
	{ }

	void cassvisn(machine_config &config);

protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
private:

	required_device<upd777_cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
};

static INPUT_PORTS_START( cassvisn )
	PORT_START("IN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Jump
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(cassvisn_state::cart_load)
{
	if (!image.loaded_through_softlist())
		return std::make_pair(image_error::UNSUPPORTED, "Cartridges must be loaded from the software list");

	uint32_t size = m_cart->common_get_size("prg");

	if (size != 0xf00)
		return std::make_pair(image_error::UNSUPPORTED, "prg region size must be 0xf00 in size");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_BIG);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "prg");
	uint16_t* prgbase = m_maincpu->get_prgregion();
	memcpy(prgbase, m_cart->get_rom_base(), size);

	// TODO: why is this needed? doesn't matter if we set endianness to big or little
	for (int i = 0; i < size / 2; i++)
	{
		prgbase[i] = ((prgbase[i] & 0xff00) >> 8) | ((prgbase[i] & 0x00ff) << 8);
	}

	size = m_cart->common_get_size("pat");

	if (size != 0x4d0)
		return std::make_pair(image_error::UNSUPPORTED, "pat region size must be 0x4d0 in size");

	m_cart->common_load_rom(m_maincpu->get_patregion(), size, "pat");

	return std::make_pair(std::error_condition(), std::string());
}

void cassvisn_state::cassvisn(machine_config &config)
{
	UPD777(config, m_maincpu, 1000000); // frequency? UPD774 / UPD778 in some carts?
	m_maincpu->in_cb().set_ioport("IN");

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
