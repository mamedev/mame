// license:BSD-3-Clause
// copyright-holders:Vas Crabb

/*

for architecture details see
https://www.oguchi-rd.com/LSI%20products.php
the linked 'design note' contains a large amount of useful information
https://www.oguchi-rd.com/777/777%20Design%20Note.pdf

*/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/upd777/upd777.h"

#include "softlist_dev.h"


namespace {

class cassvisn_state : public driver_device
{
public:
	cassvisn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_in(*this, "IN%u", 0)
	{ }

	void cassvisn(machine_config &config);

protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
private:
	u8 input_r(offs_t offset);

	required_device<upd777_cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<2> m_in;
};

u8 cassvisn_state::input_r(offs_t offset)
{
	return m_in[offset & 1]->read();
}

static INPUT_PORTS_START( cassvisn )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Jump on elvpanic

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON10 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON9 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // Jump on monstrmn 
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(cassvisn_state::cart_load)
{
	if (!image.loaded_through_softlist())
		return std::make_pair(image_error::UNSUPPORTED, "Cartridges must be loaded from the software list");

	auto const prgsize = m_cart->common_get_size("prg");
	if (prgsize != 0xf00)
		return std::make_pair(image_error::BADSOFTWARE, "prg region must be 0xf00 bytes in size");

	m_cart->rom_alloc(prgsize, GENERIC_ROM16_WIDTH, ENDIANNESS_BIG);
	m_cart->common_load_rom(m_cart->get_rom_base(), prgsize, "prg");
	uint16_t *prgbase = m_maincpu->get_prgregion();
	memcpy(prgbase, m_cart->get_rom_base(), prgsize);

	auto const patsize = m_cart->common_get_size("pat");
	if (patsize != 0x4d0)
		return std::make_pair(image_error::BADSOFTWARE, "pat region patsize must be 0x4d0 bytes in size");

	m_cart->common_load_rom(m_maincpu->get_patregion(), patsize, "pat");

	return std::make_pair(std::error_condition(), std::string());
}

void cassvisn_state::cassvisn(machine_config &config)
{
	UPD777(config, m_maincpu, 2'000'000); // frequency? UPD774 / UPD778 in some carts?
	m_maincpu->in_cb().set(FUNC(cassvisn_state::input_r));

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
