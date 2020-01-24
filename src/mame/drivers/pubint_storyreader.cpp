// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

	Non-video 'electronic book' reader that takes cartridges

	Cartridges are accessed using serial read methods, contain data, not code
	so must be internal ROMs on the systems.

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class pi_stry_state : public driver_device
{
public:
	pi_stry_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_cart_region(nullptr)
	{ }

	void pi_stry(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
};



void pi_stry_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	}
}

void pi_stry_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(pi_stry_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

static INPUT_PORTS_START( pi_stry )
INPUT_PORTS_END


void pi_stry_state::pi_stry(machine_config &config)
{
	// unknown CPU / MCU type

	// screenless

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "pi_stry_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(pi_stry_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("pi_stry_cart");
}


ROM_START( pi_stry )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "internal.mcu.rom", 0x0000, 0x1000, NO_DUMP ) // unknown type / size
ROM_END

//    year, name,        parent,    compat, machine,            input,            class,                  init,       company,    fullname,                         flags
CONS( 200?, pi_stry,     0,         0,      pi_stry,   pi_stry, pi_stry_state, empty_init, "Publications International Ltd", "Story Reader",                MACHINE_IS_SKELETON )
