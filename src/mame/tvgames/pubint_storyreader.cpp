// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    Non-video 'electronic book' reader that takes cartridges

    Cartridges are accessed using serial read methods, contain data, not code
    so must be internal ROMs on the systems.


    ---

    V1 - rectangular cart
    glob up, pins toward you, pins 1-10, left to right

    1  tied high
    2  CE1 left glob
    3  CE2 right glob
    4  pin 5
    5  pin 4
    6  data
    7  ground
    8  clock
    9  3.3 volts
    10 tied high

    ----

    V2 - rounded top cart

    1  CE1 left glob
    2  CE2 right glob
    3  tied high
    4  ground
    5  clock
    6  data
    7  ground
    8  ground
    9  3.3 volts
    10 tied high

    V2 PCB silkscreened with GPR23L822A - 8Mbit serial ROM

    ----

    To dump:
    set data and clock high, CE1 low for 1 mS
    set CE1 high for 3 uS
    set data low for 2 uS
    set clock low for 2 uS
    set data high for 2 uS
    set clock high for 2 uS
    set clock low for 2 uS
    set data low for 2 uS
    loop 24 times:
      set clock high for 1 uS
      set clock low for 1 uS
    make data high Z
    loop 8M tims:
      set clock high for 1 uS
      sample data
      set clock low for 1 uS
    set CE1 low

    if 2 globs, repeat with CE2

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class pi_storyreader_state : public driver_device
{
public:
	pi_storyreader_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_cart_region(nullptr)
	{ }

	void pi_storyreader(machine_config &config);
	void pi_storyreader_v2(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
};



void pi_storyreader_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	}
}

void pi_storyreader_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(pi_storyreader_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( pi_storyreader )
INPUT_PORTS_END


void pi_storyreader_state::pi_storyreader(machine_config &config)
{
	// unknown CPU / MCU type

	// screenless

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "pi_storyreader_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(pi_storyreader_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("pi_storyreader_cart");
}

void pi_storyreader_state::pi_storyreader_v2(machine_config &config)
{
	// unknown CPU / MCU type

	// screenless

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "pi_storyreader_v2_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(pi_storyreader_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("pi_storyreader_v2_cart");
}


ROM_START( pi_stry )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "internal.mcu.rom", 0x0000, 0x1000, NO_DUMP ) // unknown type / size
ROM_END

ROM_START( pi_stry2 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "internal.mcu.rom", 0x0000, 0x1000, NO_DUMP ) // unknown type / size
ROM_END

} // anonymous namespace


//    year, name,        parent,    compat, machine,            input,            class,                  init,       company,    fullname,                         flags

// These are said to not be compatible with each other
CONS( 200?, pi_stry,     0,         0,      pi_storyreader,      pi_storyreader, pi_storyreader_state, empty_init, "Publications International Ltd", "Story Reader",                MACHINE_IS_SKELETON )
CONS( 200?, pi_stry2,    0,         0,      pi_storyreader_v2,   pi_storyreader, pi_storyreader_state, empty_init, "Publications International Ltd", "Story Reader 2.0",            MACHINE_IS_SKELETON )
