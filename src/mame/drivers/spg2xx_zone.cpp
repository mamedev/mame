// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

/* 'Zone' systems */

#include "emu.h"
#include "includes/spg2xx.h"


class wireless60_state : public spg2xx_game_state
{
public:
	wireless60_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_bankmask(0x7)
	{ }

	void wireless60(machine_config& config);

	void init_lx_jg7415();
	void init_zone100();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint8_t m_w60_controller_input;
	uint16_t m_w60_porta_data;
	uint16_t m_w60_p1_ctrl_mask;
	uint16_t m_w60_p2_ctrl_mask;
	uint8_t m_bankmask;

	DECLARE_WRITE16_MEMBER(wireless60_porta_w);
	DECLARE_WRITE16_MEMBER(wireless60_portb_w);
	DECLARE_READ16_MEMBER(wireless60_porta_r);

private:
};

class zone40_state : public wireless60_state
{
public:
	zone40_state(const machine_config& mconfig, device_type type, const char* tag) :
		wireless60_state(mconfig, type, tag),
		m_romregion(*this, "maincpu")
	{ }

	void zone40(machine_config &config);

	void init_zone40();


protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	virtual void mem_map_z40(address_map &map);
	DECLARE_READ16_MEMBER(z40_rom_r);
	DECLARE_READ16_MEMBER(zone40_porta_r);
	DECLARE_WRITE16_MEMBER(zone40_porta_w);
	required_region_ptr<uint16_t> m_romregion;
	uint8_t m_z40_rombase;
};


WRITE16_MEMBER(wireless60_state::wireless60_porta_w)
{
	//logerror("%s: wireless60_porta_w %04x\n", machine().describe_context(), data);

	m_w60_porta_data = (data & 0x300) | m_w60_p1_ctrl_mask | m_w60_p2_ctrl_mask;
	switch (m_w60_porta_data & 0x300)
	{
	case 0x300:
		m_w60_controller_input = -1;
		break;

	case 0x200:
		m_w60_controller_input++;
		break;

	default:
		uint16_t temp1 = m_io_p1->read();
		uint16_t temp2 = m_io_p2->read();
		uint16_t temp3 = 1 << m_w60_controller_input;
		if (temp1 & temp3) m_w60_porta_data ^= m_w60_p1_ctrl_mask;
		if (temp2 & temp3) m_w60_porta_data ^= m_w60_p2_ctrl_mask;
		break;
	}
}

READ16_MEMBER(wireless60_state::wireless60_porta_r)
{
	//logerror("%s: wireless60_porta_r\n", machine().describe_context());
	return m_w60_porta_data;
}

WRITE16_MEMBER(wireless60_state::wireless60_portb_w)
{
	logerror("%s: wireless60_portb_w (bankswitch) %04x\n", machine().describe_context(), data);
	switch_bank(data & m_bankmask);
}

WRITE16_MEMBER(zone40_state::zone40_porta_w)
{
	wireless60_porta_w(space, offset, data);

	if ((data & 0x00ff) != m_z40_rombase)
	{
		m_z40_rombase = data & 0x00ff;
		m_maincpu->invalidate_cache();
	}
}

READ16_MEMBER(zone40_state::zone40_porta_r)
{
	uint16_t ret = wireless60_porta_r(space, offset) & (0x0300 | m_w60_p1_ctrl_mask | m_w60_p2_ctrl_mask);
	ret = (ret & 0xff00) | m_z40_rombase;
	return ret;
}


READ16_MEMBER(zone40_state::z40_rom_r)
{
	// due to granularity of rom bank this manual method is safer
	return m_romregion[(offset + (m_z40_rombase * 0x20000)) & 0x1ffffff];
}

void zone40_state::mem_map_z40(address_map &map)
{
	map(0x000000, 0x3fffff).r(FUNC(zone40_state::z40_rom_r));
}

void wireless60_state::machine_start()
{
	spg2xx_game_state::machine_start();

	save_item(NAME(m_w60_controller_input));
	save_item(NAME(m_w60_porta_data));

	m_w60_p1_ctrl_mask = 0x0400;
	m_w60_p2_ctrl_mask = 0x0800;
}

void zone40_state::machine_start()
{
	wireless60_state::machine_start();

	save_item(NAME(m_z40_rombase));

	m_z40_rombase = 0xe0;
	m_w60_p1_ctrl_mask = 0x0400;
	m_w60_p2_ctrl_mask = 0x1000;
}

void wireless60_state::machine_reset()
{
	spg2xx_game_state::machine_reset();
	m_w60_controller_input = -1;
	m_w60_porta_data = 0;
}

void zone40_state::machine_reset()
{
	wireless60_state::machine_reset();
	m_z40_rombase = 0xe0;
	m_maincpu->invalidate_cache();
	m_maincpu->reset();
}

static INPUT_PORTS_START( wirels60 )
	PORT_START("P1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("Menu")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("Start")

	PORT_START("P2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_NAME("Joypad Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_NAME("Joypad Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_NAME("Joypad Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_NAME("Joypad Right")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(2) PORT_NAME("A Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(2) PORT_NAME("B Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(2) PORT_NAME("Menu")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(2) PORT_NAME("Start")
INPUT_PORTS_END


void wireless60_state::wireless60(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &wireless60_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_out().set(FUNC(wireless60_state::wireless60_porta_w));
	m_maincpu->portb_out().set(FUNC(wireless60_state::wireless60_portb_w));
	m_maincpu->porta_in().set(FUNC(wireless60_state::wireless60_porta_r));
}

void zone40_state::zone40(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &zone40_state::mem_map_z40);

	spg2xx_base(config);

	m_maincpu->porta_out().set(FUNC(zone40_state::zone40_porta_w));
	m_maincpu->porta_in().set(FUNC(zone40_state::zone40_porta_r));
}


void wireless60_state::init_lx_jg7415()
{
	uint8_t blocks[32] = {
		// these parts of the ROM contain the code that gets selected
		0x00, 0x01, 0x06, 0x07, 0x08, 0x09, 0x0e, 0x0f,   0x10, 0x11, 0x16, 0x17, 0x18, 0x19, 0x1e, 0x1f,
		// these parts of the ROM contain code / data but go unused, this seems intentional, some of these areas don't read consistently so likely this double size ROM was used knowing that some areas were bad and could be avoided
		0x02, 0x03, 0x04, 0x05, 0x0a, 0x0b, 0x0c, 0x0d,   0x12, 0x13, 0x14, 0x15, 0x1a, 0x1b, 0x1c, 0x1d
	};

	uint8_t *src = memregion("maincpu")->base();
	std::vector<u8> buffer(0x10000000);

	for (int addr = 0; addr < 0x10000000; addr++)
	{
		int bank = (addr & 0xf800000) >> 23;
		int newbank = blocks[bank];
		int newaddr = (addr & 0x07fffff) | (newbank << 23);
		buffer[addr] = src[newaddr];
	}

	std::copy(buffer.begin(), buffer.end(), &src[0]);

	m_bankmask = 0xf;
}

void wireless60_state::init_zone100()
{
	m_bankmask = 0xf;
}

void zone40_state::init_zone40()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0; i < size/2; i++)
	{
		ROM[i] = ROM[i] ^ 0xbb88;

		ROM[i] = bitswap<16>(ROM[i], 11, 10, 3,  2,  4,  12, 5,  13,
									 9,  1,  8,  0,  6,  7,  14, 15);
	}
}

ROM_START( zone40 )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "zone40.bin", 0x0000, 0x4000000, CRC(4ba1444f) SHA1(de83046ab93421486668a247972ad6d3cda19440) )
ROM_END

ROM_START( zone60 )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "zone60.bin", 0x0000, 0x4000000, CRC(4cb637d1) SHA1(1f97cbdb4299ac0fbafc2a3aa592066cb0727066))
ROM_END

ROM_START( wirels60 )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wirels60.bin", 0x0000, 0x4000000, CRC(b4df8b28) SHA1(00e3da542e4bc14baf4724ad436f66d4c0f65c84))
ROM_END

ROM_START( zone100 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "zone100.bin", 0x0000, 0x8000000, CRC(b966a54e) SHA1(e38156ebc4e2f2935b1acbeca33d1866d45c4f65) )
ROM_END

// PCB marked 'Zone 100 110728 V2.1'
ROM_START( lx_jg7415 )
	ROM_REGION( 0x10000000, "maincpu", ROMREGION_ERASE00 )
	// reads of some unused areas were not 100% consistent, but this seems intentional, the game has a ROM twice the size it needs and is wired up in such a way that those bad areas are unused by the game
	// if adding a clone make sure to check if there are actual differences in the used areas
	ROM_LOAD16_WORD_SWAP( "rom.bin", 0x0000, 0x10000000, CRC(59442e00) SHA1(7e91cf6b19c37f9b4fa4dc21e241c6634d6a6f95) )
ROM_END

// these don't have real motion controls
CONS( 2009, zone40,   0, 0, zone40,     wirels60, zone40_state,      init_zone40,     "Jungle Soft / Ultimate Products (HK) Ltd",    "Zone 40",     MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2010, zone60,   0, 0, wireless60, wirels60, wireless60_state,  empty_init,      "Jungle's Soft / Ultimate Products (HK) Ltd",  "Zone 60",     MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, zone100,  0, 0, wireless60, wirels60, wireless60_state,  init_zone100,    "Jungle's Soft / Ultimate Products (HK) Ltd",  "Zone 100",    MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // unit was black, menus still show white controllers, unlike wireless 60
CONS( 2010, wirels60, 0, 0, wireless60, wirels60, wireless60_state,  empty_init,      "Jungle Soft / Kids Station Toys Inc",         "Wireless 60", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2011, lx_jg7415,0, 0, wireless60, wirels60, wireless60_state,  init_lx_jg7415,  "Lexibook",  "Lexibook JG7415 120-in-1",                      MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )



