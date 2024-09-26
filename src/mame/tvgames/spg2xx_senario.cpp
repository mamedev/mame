// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

/*
    General Senario games on SunPlus hardware

    these check for flash ROM and actually save user data at 0x700000 (senmil/senbbs/senapren) in the flash ROM

    TODO:
    senmil - Are the LEDs on the controllers meant to go out as players select answers like with pvmil, or are they just to show that the controller is connected?
    sencosmo - fix Flash hookup (crashes if you use a Flash chip right now)
    senapren - should it actually save data? chip really seems to be 2MB, data written at 7MB can't be saved at mirrored 1MB address or it would erase game code / data
    senpmate - again seems to actually be a 2MB chip

*/

#include "emu.h"
#include "spg2xx.h"

#include "machine/intelfsh.h"


namespace {

class spg2xx_senario_state : public spg2xx_game_state
{
public:
	spg2xx_senario_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

protected:
	//virtual void machine_start() override ATTR_COLD;
	//virtual void machine_reset() override ATTR_COLD;

private:
};

class spg2xx_senario_bbs_state : public spg2xx_senario_state
{
public:
	spg2xx_senario_bbs_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_senario_state(mconfig, type, tag)
	{ }

	void senbbs(machine_config& config);
	void mem_map_flash(address_map &map) ATTR_COLD;

protected:
	//virtual void machine_start() override ATTR_COLD;
	//virtual void machine_reset() override ATTR_COLD;

private:
};

class spg2xx_senario_cosmo_state : public spg2xx_senario_bbs_state
{
public:
	spg2xx_senario_cosmo_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_senario_bbs_state(mconfig, type, tag),
		m_romregion(*this, "flash")
	{ }

	void sencosmo(machine_config& config);
	void mem_map_flash_bypass(address_map &map) ATTR_COLD;

protected:
	//virtual void machine_start() override ATTR_COLD;
	//virtual void machine_reset() override ATTR_COLD;

	uint16_t read_bypass(offs_t offset) { return m_romregion[offset]; }
	void write_bypass(offs_t offset, uint16_t data) { logerror("Write to ROM area %08x %04x\n", offset, data); }

private:
	required_region_ptr<uint16_t> m_romregion;
};


class spg2xx_senario_mil_state : public spg2xx_senario_bbs_state
{
public:
	spg2xx_senario_mil_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_senario_bbs_state(mconfig, type, tag),
		m_portc_data(0)
	{ }

	void senmil(machine_config& config);

protected:
	//virtual void machine_start() override ATTR_COLD;
	//virtual void machine_reset() override ATTR_COLD;

	uint16_t portc_r();

	virtual void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

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

static INPUT_PORTS_START( senappren )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Red Team Select")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Red Team Ok")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Blue Team Select")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Blue Team Ok")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_1) PORT_NAME("Console Ok")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_5) PORT_NAME("Console Select")
	PORT_BIT( 0x7f80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Low Battery sensor

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( sencosmo ) // hold Pause during power on for Test Menu
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Player 1 A")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Player 1 B")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Player 1 C")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Player 1 D")
	PORT_BIT( 0x0030, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_1) PORT_NAME("Console Pause")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Player 2 A")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Player 2 B")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Player 2 C")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("Player 2 D")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_5) PORT_NAME("Console Power")
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Low Battery sensor

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( senpmate ) // hold Pause during power on for Test Menu
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player A")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Player B")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Player C")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Player D")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_F2) PORT_NAME("Console Reset")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Player Select")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Player Start")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // responds for 'any button' presses, doesn't appear to be a real button
	PORT_BIT( 0x7f00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Low Battery sensor

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void spg2xx_senario_bbs_state::mem_map_flash(address_map &map)
{
	map(0x000000, 0x3fffff).rw("flash", FUNC(spansion_s29gl064s_device::read), FUNC(spansion_s29gl064s_device::write));
}

// this is meant to be flash, but it crashes with invalid flash command when answering questions?
void spg2xx_senario_cosmo_state::mem_map_flash_bypass(address_map &map)
{
	map(0x000000, 0x1fffff).rw(FUNC(spg2xx_senario_cosmo_state::read_bypass), FUNC(spg2xx_senario_cosmo_state::write_bypass));
}


void spg2xx_senario_mil_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// -4-3 -2-1
	// change when a button for that player is pressed, none of these seem to be for the LEDs on the controllers (or are they 'always on')

	logerror("%s: spg2xx_senario_mil_state::portc_w %04x ---- %04x %04x \n", machine().describe_context(), data, data & 0x55, data & 0xaa);
	m_portc_data = data;
}

uint16_t spg2xx_senario_mil_state::portc_r()
{
	uint16_t ret = m_io_p3->read() & 0xffaa; // 0xaa must be set to register all controllers as turned on
	ret |= m_portc_data & 0x0055;
	return ret;
}

void spg2xx_senario_bbs_state::senbbs(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_senario_bbs_state::mem_map_flash);

	spg2xx_base(config);

	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");

	SPANSION_S29GL064S(config, "flash");
}

void spg2xx_senario_cosmo_state::sencosmo(machine_config& config)
{
	senbbs(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_senario_cosmo_state::mem_map_flash_bypass);
	/* the game crashes if you get no matches after the 2nd spin on the 'Fashion Disaster' slot machine mini-game.
	   this could be a real game bug (as to trigger it you'd have to make a poor choice not to hold any matches after the first spin)
	   however with the recompiler execution of bad data causes MAME to immediately drop to commandline with no error message
	   without recompiler the game just hangs */
	m_maincpu->set_force_no_drc(true);

}


void spg2xx_senario_mil_state::senmil(machine_config& config)
{
	senbbs(config);

	m_maincpu->portc_in().set(FUNC(spg2xx_senario_mil_state::portc_r));
	m_maincpu->portc_out().set(FUNC(spg2xx_senario_mil_state::portc_w));
}

// is ROM_REGION16_BE correct here, allows us to use ROM_LOAD16_WORD_SWAP as when loading SunPlus stuff in other cases

ROM_START( senapren )
	ROM_REGION16_BE( 0x800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "apprentice.bin", 0x000000, 0x200000, CRC(1c919e72) SHA1(20efcb992bd3ff8ab78470bd484f4f0b226e6c15) )
	// That one has a SOP44 COB instead of a TSOP48 chip.  Pin 1 is N/C, 32 is grounded, and 33 is tied high.  That means a max of A0-A20 = 21 16-bit address lines, for 4MB
	// Data repeated dumped as 4MB, so 2MB? (but game attempts to write to 0x700000 for flash user data which would erase game data in a 2MB ROM)
	// ROM also had a sticker that says D44B 16M 050818.  16Mbit is 2MB, and the 16-bit sum of the 2MB file is D44B.
	// Maybe the Flash ROM save just isn't meant to work here?
ROM_END

ROM_START( senpmate )
	ROM_REGION16_BE( 0x800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "perfectmate.bin", 0x000000, 0x200000, CRC(fa7f8ca0) SHA1(fcc78f8efb183e9c65545eb502da475225253a94) )
	// Perfect Mate's COB also had a sticker: 7DC1 16M 050822.  The 2MB file I dumped sums to 7DC1
	// The Perfect Mate checksum in the ROM header matches the sum of bytes from 0x10 to the end.
ROM_END

ROM_START( sencosmo )
	ROM_REGION16_BE( 0x400000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "cosmo.bin", 0x000000, 0x400000, CRC(1ec50795) SHA1(621c4e03b5713f3678d2935f8938f15c5d4a5fdf) )
	// attempts to write to 0x380000 for flash user data? different Flash type?
ROM_END

ROM_START( senstriv )
	ROM_REGION16_BE( 0x400000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "senariosportstriviapro_m5m29gt320_001c0020.bin", 0x000000, 0x400000, CRC(095ffbca) SHA1(d91328855a9ca542ba38253d2353545dc8b47fa4) ) // chip was 'flipped' (reverse pinout)
	// attempts to write to 0x380000 for flash user data? different Flash type?
ROM_END

ROM_START( senmil )
	ROM_REGION16_BE( 0x800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wwtbam_nouserdata.bin", 0x000000, 0x800000, CRC(b2626df6) SHA1(f06943d63dbb1c9d211cb35b40dcb18cb8b39ecd) )
ROM_END

ROM_START( senbbs )
	ROM_REGION16_BE( 0x800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "bigbonusslots.bin", 0x000000, 0x800000, CRC(071effc3) SHA1(892c05a8b64a388b331ad0d361bf4c523c6c14c9) )
ROM_END

} // anonymous namespace


CONS( 2005, senbbs,      0,     0,        senbbs,       senbbs,    spg2xx_senario_bbs_state,   empty_init, "Senario", "Big Bonus Slots (Senario, Plug and Play)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, senapren,    0,     0,        sencosmo,     senappren, spg2xx_senario_cosmo_state, empty_init, "Senario", "The Apprentice (Senario, Plug and Play)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, senpmate,    0,     0,        senbbs,       senpmate,  spg2xx_senario_bbs_state,   empty_init, "Senario", "The Perfect Mate (Senario, Plug and Play)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, sencosmo,    0,     0,        sencosmo,     sencosmo,  spg2xx_senario_cosmo_state, empty_init, "Senario", "Cosmo Girl (Senario, Plug and Play)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, senstriv,    0,     0,        sencosmo,     sencosmo,  spg2xx_senario_cosmo_state, empty_init, "Senario", "Sports Trivia Professional Edition (Senario, Plug and Play)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005?,senmil,      0,     0,        senmil,       senmil,    spg2xx_senario_mil_state,   empty_init, "Senario", "Who Wants to Be a Millionaire? (Senario, Plug and Play, US)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
