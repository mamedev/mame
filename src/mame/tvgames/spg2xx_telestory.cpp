// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

#include "emu.h"
#include "spg2xx.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"


namespace {

class telestory_state : public spg2xx_game_state
{
public:
	telestory_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_cart_region(nullptr)
	{ }

	void telestory(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mem_map_4m_tsram(address_map &map) ATTR_COLD;

	uint16_t porta_r();
	uint16_t portb_r();
	uint16_t portc_r();

	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load_telestory);

	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
};


void telestory_state::mem_map_4m_tsram(address_map &map)
{
	map(0x000000, 0x3fffff).bankr("cartbank");
	map(0x3e0000, 0x3fffff).ram(); // is this in the cart or in the system?
}

static INPUT_PORTS_START( telestory ) // there is a hidden test mode, if you return rand() on this port you get it, TODO: figure out button combination
	PORT_START("P1") // I/O test also lists 'Headphone 1' but it isn't mapped to any of these ports?
	PORT_DIPNAME( 0x0001, 0x0001, "Port 1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Pause / Menu")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Read To Me")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Red / Triangle")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Blue / Square")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Yellow / Star")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Green / Circle")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Right Arrow / Next Page") // Test mode calls this Right Arrow / LED (so probalby LED output too)
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) // port changes on these bits advance the word in manual mode (scroll wheel)
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )  // port changes on these bits advance the word in manual mode (scroll wheel)
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Left Arrow / Previous Page")


	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "Port 2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P3")
	PORT_DIPNAME( 0x0001, 0x0001, "Port 3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


uint16_t telestory_state::porta_r()
{
	uint16_t data = m_io_p1->read();
	//logerror("%s: porta_r: %04x\n", machine().describe_context(), data);
	return data;
}

void telestory_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	not used as an output very often

	':maincpu' (0145D5): porta_w (0000)
	':maincpu' (0145DC): porta_w (0000)
	':maincpu' (0145F4): porta_w (0000)
	':maincpu' (0145F4): porta_w (6000)
	*/
	logerror("%s: porta_w (%04x)\n", machine().describe_context(), data);
}

uint16_t telestory_state::portb_r()
{
	// not used?
	uint16_t data = m_io_p2->read();
	logerror("%s: portb_r %04x\n", machine().describe_context(), data);
	return data;
}

void telestory_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// not used?
	logerror("%s: portb_w (%04x)\n", machine().describe_context(), data);
}

// What is here? accessed using serial protocol with clocked bits
uint16_t telestory_state::portc_r()
{
	//logerror("%s: portc_r\n", machine().describe_context());
	return machine().rand() & 0x0c00; // bits need to change state or it doesn't boot
}

void telestory_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//logerror("%s: portc_w (%04x)\n", machine().describe_context(), data);
}



void telestory_state::machine_start()
{
	spg2xx_game_state::machine_start();

	// if there's a cart, override the standard banking
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
		m_bank->configure_entries(0, (m_cart_region->bytes() + 0x7fffff) / 0x800000, m_cart_region->base(), 0x800000);
		m_bank->set_entry(0);
	}
}

void telestory_state::machine_reset()
{
	spg2xx_game_state::machine_reset();
}

DEVICE_IMAGE_LOAD_MEMBER(telestory_state::cart_load_telestory)

{
	uint32_t const size = m_cart->common_get_size("rom");

	if (size > 0x80'0000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be no more than 8M)");

	m_cart->rom_alloc(0x800000, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void telestory_state::telestory(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &telestory_state::mem_map_4m_tsram);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(telestory_state::porta_r));
	m_maincpu->porta_out().set(FUNC(telestory_state::porta_w));
	m_maincpu->portb_in().set(FUNC(telestory_state::portb_r));
	m_maincpu->portb_out().set(FUNC(telestory_state::portb_w));
	m_maincpu->portc_in().set(FUNC(telestory_state::portc_r));
	m_maincpu->portc_out().set(FUNC(telestory_state::portc_w));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "telestory_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(telestory_state::cart_load_telestory));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "telestory_cart").set_original("telestory_cart");
}

ROM_START( telestry )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	// no internal ROM, only RAM
ROM_END

} // anonymous namespace


/* Sound data for the narrator is written to the SIO data port
   013127: D319 3D54 [3d54] = r1   (Cinderella)  - 4-bit data?
   The hidden test mode calls the speech test 'CELP' (possibly "Code-excited linear prediction" based?)
   https://en.wikipedia.org/wiki/Code-excited_linear_prediction

   TODO: check if it's a common implementation eg.  Speex
   SIO port is not currently implemented in spg2xx_io.cpp however
*/
CONS( 2006, telestry,  0,        0, telestory, telestory,   telestory_state, empty_init, "JAKKS Pacific Inc / Toymax", "Telestory",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
