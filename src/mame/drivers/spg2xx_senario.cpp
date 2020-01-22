// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

/*
	General Senario games on SunPlus hardware
	
	these check for flash ROM and actually save user data at 0x700000 in the flash ROM

	TODO:
	Are the LEDs on the controllers meant to go out as players select answers like with pvmil, or are they just to show that the controller is connected?
*/

#include "includes/spg2xx.h"
#include "machine/intelfsh.h"

class spg2xx_senario_state : public spg2xx_game_state
{
public:
	spg2xx_senario_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void senbbs(machine_config& config);
	
	void mem_map_flash(address_map &map);

protected:
	//virtual void machine_start() override;
	//virtual void machine_reset() override;

private:
};

class spg2xx_senario_mil_state : public spg2xx_senario_state
{
public:
	spg2xx_senario_mil_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_senario_state(mconfig, type, tag),
		m_portc_data(0)
	{ }

	void senmil(machine_config& config);
	
protected:
	//virtual void machine_start() override;
	//virtual void machine_reset() override;

	DECLARE_READ16_MEMBER(portc_r);

	virtual DECLARE_WRITE16_MEMBER(porta_w) override;
	virtual DECLARE_WRITE16_MEMBER(portb_w) override;
	virtual DECLARE_WRITE16_MEMBER(portc_w) override;

private:
	uint16_t m_portc_data;
};

static INPUT_PORTS_START( senmil ) // reset with Console Start and Console Select held down for test mode
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Player 1 A")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Player 1 B")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Player 1 C")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Player 1 D")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Player 2 A")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Player 2 B")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Player 2 C")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("Player 2 D")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("Player 3 A")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("Player 3 B")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("Player 3 C")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3) PORT_NAME("Player 3 D")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("Player 4 A")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("Player 4 B")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("Player 4 C")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4) PORT_NAME("Player 4 D")

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Player 1 Select")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("Player 1 OK")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("Player 2 Select")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("Player 2 OK")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3) PORT_NAME("Player 3 Select")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(3) PORT_NAME("Player 3 OK")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4) PORT_NAME("Player 4 Select")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4) PORT_NAME("Player 4 OK")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Pad Connection Status (see spg2xx_senario_mil_state::portc_r)
	PORT_BIT( 0x0300, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Low Battery sensor
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START ) PORT_CODE(KEYCODE_1) PORT_NAME("Console Start")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SELECT ) PORT_CODE(KEYCODE_5) PORT_NAME("Console Select")
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( senbbs ) // reset with Select and Spin held down for test mode
	PORT_START("P1")
	// To use Gambling controls or not? This is a Plug and Play themed controller, not an actual gambling unit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Spin")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Lever")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Select")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("OK")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Bet")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Cash Out")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Bet Max")
	PORT_BIT( 0x7f80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Low Battery sensor

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void spg2xx_senario_state::mem_map_flash(address_map &map)
{
	map(0x000000, 0x3fffff).rw("flash", FUNC(spansion_s29gl064s_device::read), FUNC(spansion_s29gl064s_device::write));
}

WRITE16_MEMBER(spg2xx_senario_mil_state::portc_w)
{
	// -4-3 -2-1
	// change when a button for that player is pressed, none of these seem to be for the LEDs on the controllers (or are they 'always on')

	logerror("%s: spg2xx_senario_mil_state::portc_w %04x ---- %04x %04x \n", machine().describe_context(), data, data & 0x55, data & 0xaa);
	m_portc_data = data;
}
	
READ16_MEMBER(spg2xx_senario_mil_state::portc_r)
{
	uint16_t ret = m_io_p3->read() & 0xffaa; // 0xaa must be set to register all controllers as turned on
	ret |= m_portc_data & 0x0055;
	return ret;
}

void spg2xx_senario_state::senbbs(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_senario_state::mem_map_flash);

	spg2xx_base(config);

	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");

	SPANSION_S29GL064S(config, "flash");
}

void spg2xx_senario_mil_state::senmil(machine_config& config)
{
	spg2xx_senario_state::senbbs(config);

	m_maincpu->portc_in().set(FUNC(spg2xx_senario_mil_state::portc_r));
	m_maincpu->portc_out().set(FUNC(spg2xx_senario_mil_state::portc_w));
}

// note not using ROM_LOAD16_WORD_SWAP because this is a Flash ROM region, and we end up with wrong endian if we do

ROM_START( senmil )
	ROM_REGION( 0x800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "wwtbam_nouserdata.bin", 0x000000, 0x800000, CRC(b2626df6) SHA1(f06943d63dbb1c9d211cb35b40dcb18cb8b39ecd) )
ROM_END

ROM_START( senbbs )
	ROM_REGION( 0x800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "bigbonusslots.bin", 0x000000, 0x800000, CRC(071effc3) SHA1(892c05a8b64a388b331ad0d361bf4c523c6c14c9) )
ROM_END

CONS( 2005, senbbs,      0,     0,        senbbs,       senbbs,    spg2xx_senario_state,     empty_init, "Senario", "Big Bonus Slots (Senario, Plug and Play)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2006, senmil,      0,     0,        senmil,       senmil,    spg2xx_senario_mil_state, empty_init, "Senario", "Who Wants to Be a Millionaire? (Senario, Plug and Play, US)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
