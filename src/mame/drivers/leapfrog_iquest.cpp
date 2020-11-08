// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

   Leapfrog IQuest

   has LCD display

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"
#include "screen.h"

class leapfrog_iquest_state : public driver_device
{
public:
	leapfrog_iquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_screen(*this, "screen")
		, m_cart_region(nullptr)
	{ }

	void leapfrog_iquest(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void prog_map(address_map &map);
	void ext_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<screen_device> m_screen;
	memory_region *m_cart_region;
};



void leapfrog_iquest_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

void leapfrog_iquest_state::machine_reset()
{
}

void leapfrog_iquest_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0x10000); // TODO: banking
}

void leapfrog_iquest_state::ext_map(address_map &map)
{
}

DEVICE_IMAGE_LOAD_MEMBER(leapfrog_iquest_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

static INPUT_PORTS_START( leapfrog_iquest )
INPUT_PORTS_END

uint32_t leapfrog_iquest_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void leapfrog_iquest_state::leapfrog_iquest(machine_config &config)
{
	I8032(config, m_maincpu, 96000000/10); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &leapfrog_iquest_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &leapfrog_iquest_state::ext_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(90, 64);
	m_screen->set_visarea(0, 90-1, 0, 64-1);
	m_screen->set_screen_update(FUNC(leapfrog_iquest_state::screen_update));
	//m_screen->screen_vblank().set(FUNC(leapfrog_iquest_state::screen_vblank));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_iquest_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_iquest_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("leapfrog_iquest_cart");
}

ROM_START( iquest )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "iquest.bin", 0x000000, 0x400000, CRC(f785dc4e) SHA1(ec002c18df536737334fe6b7db0e7342bad7b66b))
ROM_END

//    year, name,        parent,    compat, machine,            input,            class,                  init,       company,    fullname,                         flags
// it is unknown if the versions of IQuest without 4.0 on the case have different system ROM
CONS( 200?, iquest,      0,         0,      leapfrog_iquest,    leapfrog_iquest,  leapfrog_iquest_state,  empty_init, "LeapFrog", "IQuest 4.0 (US)",                    MACHINE_IS_SKELETON )
