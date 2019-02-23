// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    Sega Beena

	non-video 'book' based learning system, like LeapPad etc.

	unknown CPU type (inside Sega custom?)

	cartridge ROM has 'edinburgh' in the header, maybe a system codename?
	ROM is also full of OGG files containing the string 'Encoded with Speex speex-1.0.4'
	as well as .mid files for music

	TODO: component list!

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist.h"
#include "speaker.h"

class sega_beena_state : public driver_device
{
public:
	sega_beena_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void sega_beena(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart);

	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
};

void sega_beena_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	}
}

void sega_beena_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(sega_beena_state, cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

static INPUT_PORTS_START( sega_beena )
INPUT_PORTS_END


void sega_beena_state::sega_beena(machine_config &config)
{
	// unknown CPU

	// no screen

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "sega_beena_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(device_image_load_delegate(&sega_beena_state::device_image_load_cart, this));

	SOFTWARE_LIST(config, "cart_list").set_original("sega_beena_cart");
}

ROM_START( beena )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	// no BIOS or internal to CPU
ROM_END


//    year, name,         parent,  compat, machine,      input,        class,              init,       company,  fullname,                             flags
CONS( 200?, beena,      0,       0,      sega_beena, sega_beena, sega_beena_state, empty_init, "Sega", "Beena", MACHINE_IS_SKELETON )
