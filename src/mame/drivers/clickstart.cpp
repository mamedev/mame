// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
/******************************************************************************

    Leapfrog Clickstart Emulation

    Status:

        Calls to unmapped space

		Some games have Checksums listed in the header area that appear to be
		 like the byte checksums on the Radica games in vii.cpp, however the
		 calculation doesn't add up correctly.  There is also a checksum in
		 a footer area at the end of every ROM that does add up correctly in
		 all cases.

		 The ROM carts are marked for 4MByte ROMs at least so the sizes
		 should be correct.

		What type of SPG is this?

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "cpu/unsp/unsp.h"

#include "machine/spg2xx.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class clickstart_state : public driver_device
{
public:
	clickstart_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_spg(*this, "spg")
		, m_cart(*this, "cartslot")
		, m_system_region(*this, "maincpu")
		, m_cart_region(nullptr)
	{ }

	void clickstart(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void mem_map(address_map &map);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart);

	DECLARE_READ16_MEMBER(rom_r);

	DECLARE_WRITE16_MEMBER(porta_w);
	DECLARE_WRITE16_MEMBER(portb_w);
	DECLARE_WRITE16_MEMBER(portc_w);
	DECLARE_READ16_MEMBER(porta_r);
	DECLARE_READ16_MEMBER(portb_r);
	DECLARE_READ16_MEMBER(portc_r);

	DECLARE_WRITE8_MEMBER(chip_sel_w);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<spg2xx_device> m_spg;
	required_device<generic_slot_device> m_cart;
	required_memory_region m_system_region;
	memory_region *m_cart_region;

	uint16_t m_unk_portc_toggle;
};


void clickstart_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	}

	save_item(NAME(m_unk_portc_toggle));
}

void clickstart_state::machine_reset()
{
	m_unk_portc_toggle = 0;
}

DEVICE_IMAGE_LOAD_MEMBER(clickstart_state, cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

READ16_MEMBER(clickstart_state::rom_r)
{
	if (offset < 0x400000 / 2)
	{
		if (m_cart->exists())
			return ((uint16_t*)m_cart_region->base())[offset];
		else
			return ((uint16_t*)m_system_region->base())[offset];
	}
	else
	{
		return ((uint16_t*)m_system_region->base())[offset];
	}
}

WRITE16_MEMBER(clickstart_state::porta_w)
{
	logerror("%s: porta_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

WRITE16_MEMBER(clickstart_state::portb_w)
{
	logerror("%s: portb_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

WRITE16_MEMBER(clickstart_state::portc_w)
{
	//logerror("%s: portc_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

READ16_MEMBER(clickstart_state::porta_r)
{
	uint16_t data = 0x5000;
	logerror("%s: porta_r: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	return data;
}

READ16_MEMBER(clickstart_state::portb_r)
{
	logerror("%s: portb_r: %04x\n", machine().describe_context(), mem_mask);
	return 0;
}

READ16_MEMBER(clickstart_state::portc_r)
{
	uint16_t data = m_unk_portc_toggle;
	m_unk_portc_toggle ^= 0x0400;
	//logerror("%s: portc_r: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	return data;
}

WRITE8_MEMBER(clickstart_state::chip_sel_w)
{
	// Seems unused, currently
}

void clickstart_state::mem_map(address_map &map)
{
	map(0x000000, 0x3fffff).r(FUNC(clickstart_state::rom_r));
	map(0x000000, 0x003fff).m(m_spg, FUNC(spg2xx_device::map));
}

static INPUT_PORTS_START( clickstart )
INPUT_PORTS_END

// There is a SEEPROM on the motherboard (type?)

void clickstart_state::clickstart(machine_config &config)
{
	UNSP(config, m_maincpu, XTAL(27'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &clickstart_state::mem_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("spg", FUNC(spg2xx_device::screen_update));
	m_screen->screen_vblank().set(m_spg, FUNC(spg2xx_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	m_spg->porta_out().set(FUNC(clickstart_state::porta_w));
	m_spg->portb_out().set(FUNC(clickstart_state::portb_w));
	m_spg->portc_out().set(FUNC(clickstart_state::portc_w));
	m_spg->porta_in().set(FUNC(clickstart_state::porta_r));
	m_spg->portb_in().set(FUNC(clickstart_state::portb_r));
	m_spg->portc_in().set(FUNC(clickstart_state::portc_r));
	m_spg->adc_in().set_constant(0x0fff);
	m_spg->chip_select().set(FUNC(clickstart_state::chip_sel_w));
	m_spg->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_spg->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "clickstart_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(device_image_load_delegate(&clickstart_state::device_image_load_cart, this));

	SOFTWARE_LIST(config, "cart_list").set_original("clickstart_cart");
}

ROM_START( clikstrt )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "clickstartbios.bin", 0x000000, 0x800000, CRC(7c833bd0) SHA1(2e9ef38e1a7582705920339e6b9944f6404fcf9b) )
ROM_END

// year, name, parent, compat, machine, input, class, init, company, fullname, flags
CONS( 2007, clikstrt,  0,      0, clickstart,  clickstart, clickstart_state, empty_init, "LeapFrog Enterprises", "ClickStart",      MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // 'My First Computer' tagline
