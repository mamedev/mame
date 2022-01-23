// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Wilbert Pol
/***************************************************************************

  Atari VCS 2600 driver

TODO:
- Move the 2 32-in-1 rom dumps into their own driver
- Add 128-in-1 driver

  Atari A2600 Point of Purchase Display Unit

TODO:
- Does the reset by pressing game select buttons only work on the infinite
  timer mode or also in the other modes?
- Asteroids is an 8KB software title with banking. Does the display unit
  use a special 4KB version or is there banking hardware installed?
- 2 joysticks and 2 paddles are hooked up to the unit; unknown how.


Selection buttons on the kiosk unit:
2 "Press to select game number" buttons
1 "Press to start" button


Switch A8 controls the gameplay duration. Factory default is one minute
gameplay duration.
1 - 2 minute gameplay
2 - 1 minute gameplay
3 - 30 second gameplay
4 - Infinite gameplay (gameplay allowed until end of game or until a "Press
     to select game number" button is pressed)

This seems to be completely hardware controlled; it is never queried or set up
by the game selection rom.


The POP unit comes with the following 42 games installed:
Location Game
D6 Prog ROM 1 ADVENTURE (CX2613)
E6 Prog ROM 2 AIR-SEA BATTLE (CX2602)
F6 Prog ROM 3 ASTEROIDS (CX2649)
J6 Prog ROM 4 BACKGAMMON (CX2617)
K6 Prog ROM 5 BASKETBALL (CX2624)
L6 Prog ROM 6 BOWLING (CX2628)
M6 Prog ROM 7 BREAKOUT (CX2622)
C5 Prog ROM 8 CANYON BOMBER (CX2607)
D5 Prog ROM 9 CASINO (CX2652)
E5 Prog ROM 10 CIRCUS ATARI (CX2630)
F5 Prog ROM 11 COMBAT (CX2601)
J5 Prog ROM 12 DODGE'EM (CX2637)
K5 Prog ROM 13 FOOTBALL (CX2625)
L5 Prog ROM 14 GOLF (CX2634)
M5 Prog ROM 15 HANGMAN (CX2662)
C4 Prog ROM 16 HOMERUN (CX2623)
D4 Prog ROM 17 H. CANNONBALL (CX2627)
E4 Prog ROM 18 MAZE CRAZE (CX2635)
F4 Prog ROM 19 MISSILE CMD (CX2638)
J4 Prog ROM 20 NIGHT DRIVER (CX2633)
K4 Prog ROM 21 OTHELLO (CX2639)
L4 Prog ROM 22 OUTLAW (CX2605)
M4 Prog ROM 23 SOCCER (CX2616)
C3 Prog ROM 24 SKYDIVER (CX2629)
D3 Prog ROM 25 sLOT RAcERS (CX2606)
E3 Prog ROM 26 SPACE INVADERS (CX2632)
F3 Prog ROM 27 STREET RACER (CX2612)
J3 Prog ROM 28 SUPERMAN (CX2631)
K3 Prog ROM 29 3D TIC-TAC-TOE (CX2618)
L3 Prog ROM 30 VIDEO CHECKERS (CX2636)
M3 Prog ROM 31 VIDEO CHESS (CX2645)
C2 Prog ROM 32 VIDEO OLYMPICS (CX2621)
D2 Prog ROM 33 VIDEO PINBALL (CX2648)
E2 Prog ROM 34 WAR LORDS (CX2610)
F2 Prog ROM 35 BERZERK (CX2650)
J2 Prog ROM 36 HAUNTED HOUSE (CX2654)
K2 Prog ROM 37 MATH GRAND PRIX (CX2658)
L2 Prog ROM 38 DEFENDER (CX2609)
M2 Prog ROM 39 YARS' REVENGE (CX2655)
C1 Prog ROM 40 PAC-MAN (CX2646)
D1 Prog ROM 41 SUPER BREAKOUT (CX2608)
E1 Prog ROM 42 DEMON/DIAMOND (CX2615)

***************************************************************************/

// the new RIOT does not work with the SuperCharger
// for example "mame a2600 scharger -cass offifrog" fails to load after playing the tape

#include "emu.h"

#include "includes/a2600.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


static constexpr auto MASTER_CLOCK_NTSC = 3.579575_MHz_XTAL;
static constexpr auto MASTER_CLOCK_PAL  = 3.546894_MHz_XTAL;

static const uint16_t supported_screen_heights[4] = { 262, 312, 328, 342 };


void a2600_base_state::a2600_mem(address_map &map) // 6507 has 13-bit address space, 0x0000 - 0x1fff
{
	map(0x0000, 0x007f).mirror(0x0f00).rw(m_tia, FUNC(tia_video_device::read), FUNC(tia_video_device::write));
	map(0x0080, 0x00ff).mirror(0x0d00).ram().share("riot_ram");
#if USE_NEW_RIOT
	map(0x0280, 0x029f).mirror(0x0d00).m("riot", FUNC(mos6532_t::io_map));
#else
	map(0x0280, 0x029f).mirror(0x0d00).rw("riot", FUNC(riot6532_device::read), FUNC(riot6532_device::write));
#endif
	// map(0x1000, 0x1fff) is cart data and it is configured at reset time, depending on the mounted cart!
}


void a2600_pop_state::memory_map(address_map &map) // 6507 has 13-bit address space, 0x0000 - 0x1fff
{
	map(0x0000, 0x007f).mirror(0x0f00).rw(m_tia, FUNC(tia_video_device::read), FUNC(tia_video_device::write));
	map(0x0080, 0x00ff).mirror(0x0d00).ram().share("riot_ram");
#if USE_NEW_RIOT
	map(0x0280, 0x029f).mirror(0x0d00).m("riot", FUNC(mos6532_t::io_map));
#else
	map(0x0280, 0x029f).mirror(0x0d00).rw("riot", FUNC(riot6532_device::read), FUNC(riot6532_device::write));
#endif
	map(0x0800, 0x0800).rw(FUNC(a2600_pop_state::rom_switch_r), FUNC(a2600_pop_state::rom_switch_w));
	map(0x1000, 0x1fff).bankr(m_bank);
}


//  read returns number of empty game rom slots
uint8_t a2600_pop_state::rom_switch_r(offs_t offset)
{
	return 5;   // Max 47 games, 5 empty slots => 42 games
}


//  Rom switch
void a2600_pop_state::rom_switch_w(offs_t offset, uint8_t data)
{
	m_bank->set_entry(data & 0x7f);
	if (data & 0x80)
	{
		// (re)start reset timer?
		// Unknown what happens when multiple dipswitches are set
		uint8_t a8 = m_a8->read();
		attotime reset_time = attotime::never;
		if (a8 & 0x08)
		{
			// infinite
			// enable reset by pressing game selection button when on infinite timer
			m_game_select_button_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
		}
		else if (a8 & 0x04)
		{
			// 30 seconds
			reset_time = attotime::from_seconds(30);
		}
		else if (a8 & 0x02)
		{
			// 1 minute
			reset_time = attotime::from_seconds(60);
		}
		else if (a8 & 0x01)
		{
			// 2 minutes
			reset_time = attotime::from_seconds(120);
		}
		m_reset_timer->adjust(reset_time);
	}
}


TIMER_CALLBACK_MEMBER(a2600_pop_state::reset_timer_callback)
{
	machine().schedule_soft_reset();
}


TIMER_CALLBACK_MEMBER(a2600_pop_state::game_select_button_timer_callback)
{
	if ((m_swb->read() & 0x14) != 0x14) {
		machine().schedule_soft_reset();
	}
}


uint8_t a2600_state::cart_over_all_r(address_space &space, offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_cart->write_bank(space, offset, 0);

	int masked_offset = offset &~ 0x0d00;
	uint8_t ret = 0x00;

	if (masked_offset < 0x80)
	{
		ret = m_tia->read(masked_offset&0x7f);
	}
	else if (masked_offset < 0x100)
	{
		ret = m_riot_ram[masked_offset & 0x7f];
	}
	/* 0x100 - 0x1ff already masked out */
	else if (masked_offset < 0x280)
	{
		ret = m_tia->read(masked_offset&0x7f);
	}
	else if (masked_offset < 0x2a0)
	{
#if USE_NEW_RIOT
		ret = m_riot->io_r(masked_offset);
#else
		ret = m_riot->read(masked_offset);
#endif
	}
	else if (masked_offset < 0x300)
	{
		/* 0x2a0 - 0x2ff nothing? */
	}
	/* 0x300 - 0x3ff already masked out */

	return ret;
}

void a2600_state::cart_over_all_w(address_space &space, offs_t offset, uint8_t data)
{
	m_cart->write_bank(space, offset, 0);

	int masked_offset = offset &~ 0x0d00;

	if (masked_offset < 0x80)
	{
		m_tia->write(masked_offset & 0x7f, data);
	}
	else if (masked_offset < 0x100)
	{
		m_riot_ram[masked_offset & 0x7f] = data;
	}
	/* 0x100 - 0x1ff already masked out */
	else if (masked_offset < 0x280)
	{
		m_tia->write(masked_offset & 0x7f, data);
	}
	else if (masked_offset < 0x2a0)
	{
#if USE_NEW_RIOT
		m_riot->io_w(masked_offset, data);
#else
		m_riot->write(masked_offset, data);
#endif
	}
	else if (masked_offset < 0x300)
	{
		/* 0x2a0 - 0x2ff nothing? */
	}
	/* 0x300 - 0x3ff already masked out */
}

void a2600_base_state::switch_A_w(uint8_t data)
{
	/* Left controller port */
	m_joy1->joy_w( data >> 4 );

	/* Right controller port */
	m_joy2->joy_w( data & 0x0f );

//  switch( ioport("CONTROLLERS")->read() % 16 )
//  {
//  case 0x0a:  /* KidVid voice module */
//      m_cassette->change_state(( data & 0x02 ) ? CASSETTE_MOTOR_DISABLED : (CASSETTE_MOTOR_ENABLED | CASSETTE_PLAY), CASSETTE_MOTOR_DISABLED );
//      break;
//  }
}

uint8_t a2600_base_state::switch_A_r()
{
	uint8_t val = 0;

	// Left controller port PINs 1-4 ( 4321 )
	val |= (m_joy1->read_joy() & 0x0f) << 4;

	// Right controller port PINs 1-4 ( 4321 )
	val |= m_joy2->read_joy() & 0x0f;

	return val;
}

void a2600_base_state::switch_B_w(uint8_t data)
{
}

WRITE_LINE_MEMBER(a2600_base_state::irq_callback)
{
}

uint16_t a2600_base_state::a2600_read_input_port(offs_t offset)
{
	switch (offset)
	{
	case 0: // Left controller port PIN 5
		return m_joy1->read_pot_x();

	case 1: // Left controller port PIN 9
		return m_joy1->read_pot_y();

	case 2: // Right controller port PIN 5
		return m_joy2->read_pot_x();

	case 3: // Right controller port PIN 9
		return m_joy2->read_pot_y();

	case 4: // Left controller port PIN 6
		return (m_joy1->read_joy() & 0x20) ? 0xff : 0x7f;

	case 5: // Right controller port PIN 6
		return (m_joy2->read_joy() & 0x20) ? 0xff : 0x7f;
	}
	return 0xff;
}

/* There are a few games that do an LDA ($80-$FF),Y instruction.
   The contents off the databus then depend on whatever was read
   from the RAM. To do this really properly the 6502 core would
   need to keep track of the last databus contents so we can query
   that. For now this is a quick hack to determine that value anyway.
   Examples:
   Q-Bert's Qubes (NTSC,F6) at 0x1594
   Berzerk at 0xF093.
*/
uint8_t a2600_base_state::a2600_get_databus_contents(offs_t offset)
{
	uint16_t  last_address, prev_address;
	uint8_t   last_byte, prev_byte;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	last_address = m_maincpu->pc() + 1;
	if ( ! ( last_address & 0x1080 ) )
	{
		return offset;
	}
	last_byte = prog_space.read_byte(last_address );
	if ( last_byte < 0x80 || last_byte == 0xFF )
	{
		return last_byte;
	}
	prev_address = last_address - 1;
	if ( ! ( prev_address & 0x1080 ) )
	{
		return last_byte;
	}
	prev_byte = prog_space.read_byte(prev_address );
	if ( prev_byte == 0xB1 )
	{   /* LDA (XX),Y */
		return prog_space.read_byte(last_byte + 1 );
	}
	return last_byte;
}

#if 0
static const rectangle visarea[4] = {
	{ 26, 26 + 160 + 16, 24, 24 + 192 + 31 },   /* 262 */
	{ 26, 26 + 160 + 16, 32, 32 + 228 + 31 },   /* 312 */
	{ 26, 26 + 160 + 16, 45, 45 + 240 + 31 },   /* 328 */
	{ 26, 26 + 160 + 16, 48, 48 + 240 + 31 }    /* 342 */
};
#endif

void a2600_base_state::a2600_tia_vsync_callback(uint16_t data)
{
	for ( int i = 0; i < std::size(supported_screen_heights); i++ )
	{
		if ( data >= supported_screen_heights[i] - 3 && data <= supported_screen_heights[i] + 3 )
		{
			if ( supported_screen_heights[i] != m_current_screen_height )
			{
				m_current_screen_height = supported_screen_heights[i];
//              m_screen->configure(228, m_current_screen_height, &visarea[i], HZ_TO_ATTOSECONDS( MASTER_CLOCK_NTSC ) * 228 * m_current_screen_height );
			}
		}
	}
}

void a2600_base_state::a2600_tia_vsync_callback_pal(uint16_t data)
{
	for ( int i = 0; i < std::size(supported_screen_heights); i++ )
	{
		if ( data >= supported_screen_heights[i] - 3 && data <= supported_screen_heights[i] + 3 )
		{
			if ( supported_screen_heights[i] != m_current_screen_height )
			{
				m_current_screen_height = supported_screen_heights[i];
//              m_screen->configure(228, m_current_screen_height, &visarea[i], HZ_TO_ATTOSECONDS( MASTER_CLOCK_PAL ) * 228 * m_current_screen_height );
			}
		}
	}
}

// TODO: is this the correct behavior for the real hardware?!?
uint8_t a2600_state::cart_over_riot_r(address_space &space, offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_cart->write_bank(space, offset, 0);
	return m_riot_ram[0x20 + offset];
}

void a2600_state::cart_over_riot_w(address_space &space, offs_t offset, uint8_t data)
{
	m_cart->write_bank(space, offset, 0);
	m_riot_ram[0x20 + offset] = data;

}

void a2600_state::cart_over_tia_w(address_space &space, offs_t offset, uint8_t data)
{
	// Both Cart & TIA see these addresses
	m_cart->write_bank(space, offset, data);
	m_tia->write(offset, data);
}

void a2600_base_state::machine_start()
{
	m_current_screen_height = m_screen->height();
	memset(m_riot_ram, 0x00, 0x80);

	save_item(NAME(m_current_screen_height));
}

void a2600_state::machine_start()
{
	a2600_base_state::machine_start();

	switch (m_cart->get_cart_type())
	{
	case A26_2K:
	case A26_4K:
	case A26_F4:
	case A26_F8:
	case A26_F8SW:
	case A26_FA:
	case A26_E0:
	case A26_E7:
	case A26_CV:
	case A26_DC:
	case A26_FV:
	case A26_8IN1:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x1000, 0x1fff, write8m_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		break;
	case A26_F6:
	case A26_DPC:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x1000, 0x1fff, write8m_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		break;
	case A26_FE:
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)), write8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_ram)));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x01fe, 0x01ff, read8m_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_bank)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x01fe, 0x01fe, write8m_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		break;
	case A26_3E:
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)), write8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_ram)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x00, 0x3f, write8m_delegate(*this, FUNC(a2600_state::cart_over_tia_w)));
		break;
	case A26_3F:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x00, 0x3f, write8m_delegate(*this, FUNC(a2600_state::cart_over_tia_w)));
		break;
	case A26_UA:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x200, 0x27f, read8m_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_bank)), write8m_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		break;
	case A26_JVP:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x1000, 0x1fff, write8m_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		// to verify the actual behavior...
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xfa0, 0xfc0, read8m_delegate(*this, FUNC(a2600_state::cart_over_riot_r)), write8m_delegate(*this, FUNC(a2600_state::cart_over_riot_w)));
		break;
	case A26_4IN1:
	case A26_32IN1:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		break;
	case A26_SS:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		break;
	case A26_CM:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		break;
	case A26_X07:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x1000, 0x1fff, write8m_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0000, 0x0fff, read8m_delegate(*this, FUNC(a2600_state::cart_over_all_r)), write8m_delegate(*this, FUNC(a2600_state::cart_over_all_w)));
		break;
	case A26_HARMONY:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x1000, 0x1fff, write8m_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		break;
	}
}


void a2600_pop_state::machine_start()
{
	a2600_base_state::machine_start();
	m_bank->configure_entries(0, 48, memregion("maincpu")->base(), 0x1000);
	m_bank->set_entry(0);
	m_reset_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(a2600_pop_state::reset_timer_callback),this), nullptr);
	m_game_select_button_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(a2600_pop_state::game_select_button_timer_callback),this), nullptr);
}


void a2600_pop_state::machine_reset()
{
	a2600_base_state::machine_reset();

	m_bank->set_entry(0);
	m_reset_timer->adjust(attotime::never);
	m_game_select_button_timer->adjust(attotime::never);
}


static INPUT_PORTS_START( a2600 )
	PORT_START("SWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Game") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game") PORT_CODE(KEYCODE_1)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, "TV Type" ) PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_DIPSETTING(    0x08, "Color" )
	PORT_DIPSETTING(    0x00, "B&W" )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, "Left Diff. Switch" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x40, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x80, 0x00, "Right Diff. Switch" ) PORT_CODE(KEYCODE_4) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, "A" )
	PORT_DIPSETTING(    0x00, "B" )
INPUT_PORTS_END


static INPUT_PORTS_START(a2600_pop)
	PORT_START("SWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Game") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Tens") PORT_CODE(KEYCODE_3)  // Tens
	PORT_DIPNAME( 0x08, 0x08, "TV Type" ) PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_DIPSETTING(    0x08, "Color" )
	PORT_DIPSETTING(    0x00, "B&W" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Ones")  PORT_CODE(KEYCODE_4) // Ones
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Start") PORT_CODE(KEYCODE_5) // Start
	// Left difficulty switch not present
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	// Right difficulty switch not present
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A8")
	PORT_DIPNAME(0x0f, 0x02, "Gameplay duration")
	PORT_DIPSETTING(0x01, "2 minutes")
	PORT_DIPSETTING(0x02, "1 minute")
	PORT_DIPSETTING(0x04, "30 seconds")
	PORT_DIPSETTING(0x08, "Infinite")
INPUT_PORTS_END


static void a2600_cart(device_slot_interface &device)
{
	device.option_add("a26_2k",    A26_ROM_2K);
	device.option_add("a26_4k",    A26_ROM_4K);
	device.option_add("a26_f4",    A26_ROM_F4);
	device.option_add("a26_f6",    A26_ROM_F6);
	device.option_add("a26_f8",    A26_ROM_F8);
	device.option_add("a26_f8sw",  A26_ROM_F8_SW);
	device.option_add("a26_fa",    A26_ROM_FA);
	device.option_add("a26_fe",    A26_ROM_FE);
	device.option_add("a26_3e",    A26_ROM_3E);
	device.option_add("a26_3f",    A26_ROM_3F);
	device.option_add("a26_e0",    A26_ROM_E0);
	device.option_add("a26_e7",    A26_ROM_E7);
	device.option_add("a26_ua",    A26_ROM_UA);
	device.option_add("a26_cv",    A26_ROM_CV);
	device.option_add("a26_dc",    A26_ROM_DC);
	device.option_add("a26_fv",    A26_ROM_FV);
	device.option_add("a26_jvp",   A26_ROM_JVP);
	device.option_add("a26_cm",    A26_ROM_COMPUMATE);
	device.option_add("a26_ss",    A26_ROM_SUPERCHARGER);
	device.option_add("a26_dpc",   A26_ROM_DPC);
	device.option_add("a26_4in1",  A26_ROM_4IN1);
	device.option_add("a26_8in1",  A26_ROM_8IN1);
	device.option_add("a26_32in1", A26_ROM_32IN1);
	device.option_add("a26_x07",   A26_ROM_X07);
	device.option_add("a26_harmony",   A26_ROM_HARMONY);
}

void a2600_state::a2600_cartslot(machine_config &config)
{
	VCS_CART_SLOT(config, "cartslot", a2600_cart, nullptr).set_must_be_loaded(true);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("a2600");
	SOFTWARE_LIST(config, "cass_list").set_original("a2600_cass");
}

void a2600_base_state::a2600_base_ntsc(machine_config &config)
{
	/* basic machine hardware */
	M6507(config, m_maincpu, MASTER_CLOCK_NTSC / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &a2600_state::a2600_mem);

	/* video hardware */
	TIA_NTSC_VIDEO(config, m_tia, 0, "tia");
	m_tia->read_input_port_callback().set(FUNC(a2600_state::a2600_read_input_port));
	m_tia->databus_contents_callback().set(FUNC(a2600_state::a2600_get_databus_contents));
	m_tia->vsync_callback().set(FUNC(a2600_state::a2600_tia_vsync_callback));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK_NTSC, 228, 26, 26 + 160 + 16, 262, 24 , 24 + 192 + 31);
	m_screen->set_screen_update("tia_video", FUNC(tia_video_device::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	TIA(config, "tia", MASTER_CLOCK_NTSC/114).add_route(ALL_OUTPUTS, "mono", 0.90);

	/* devices */
#if USE_NEW_RIOT
	MOS6532_NEW(config, m_riot, MASTER_CLOCK_NTSC / 3);
	m_riot->pa_rd_callback().set(FUNC(a2600_state::switch_A_r));
	m_riot->pa_wr_callback().set(FUNC(a2600_state::switch_A_w));
	m_riot->pb_rd_callback().set_ioport("SWB");
	m_riot->pb_wr_callback().set(FUNC(a2600_state::switch_B_w));
	m_riot->irq_wr_callback().set(FUNC(a2600_state::irq_callback));
#else
	RIOT6532(config, m_riot, MASTER_CLOCK_NTSC / 3);
	m_riot->in_pa_callback().set(FUNC(a2600_state::switch_A_r));
	m_riot->out_pa_callback().set(FUNC(a2600_state::switch_A_w));
	m_riot->in_pb_callback().set_ioport("SWB");
	m_riot->out_pb_callback().set(FUNC(a2600_state::switch_B_w));
	m_riot->irq_callback().set(FUNC(a2600_state::irq_callback));
#endif
}


void a2600_state::a2600(machine_config &config)
{
	a2600_base_ntsc(config);

	VCS_CONTROL_PORT(config, CONTROL1_TAG, vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, CONTROL2_TAG, vcs_control_port_devices, nullptr);

	a2600_cartslot(config);
	subdevice<software_list_device>("cart_list")->set_filter("NTSC");
}


void a2600_state::a2600p(machine_config &config)
{
	/* basic machine hardware */
	M6507(config, m_maincpu, MASTER_CLOCK_PAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &a2600_state::a2600_mem);

	/* video hardware */
	TIA_PAL_VIDEO(config, m_tia, 0, "tia");
	m_tia->read_input_port_callback().set(FUNC(a2600_state::a2600_read_input_port));
	m_tia->databus_contents_callback().set(FUNC(a2600_state::a2600_get_databus_contents));
	m_tia->vsync_callback().set(FUNC(a2600_state::a2600_tia_vsync_callback_pal));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK_PAL, 228, 26, 26 + 160 + 16, 312, 32, 32 + 228 + 31);
	m_screen->set_screen_update("tia_video", FUNC(tia_video_device::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	TIA(config, "tia", MASTER_CLOCK_PAL/114).add_route(ALL_OUTPUTS, "mono", 0.90);

	/* devices */
#if USE_NEW_RIOT
	MOS6532_NEW(config, m_riot, MASTER_CLOCK_PAL / 3);
	m_riot->pa_rd_callback().set(FUNC(a2600_state::switch_A_r));
	m_riot->pa_wr_callback().set(FUNC(a2600_state::switch_A_w));
	m_riot->pb_rd_callback().set_ioport("SWB");
	m_riot->pb_wr_callback().set(FUNC(a2600_state::switch_B_w));
	m_riot->irq_wr_callback().set(FUNC(a2600_state::irq_callback));
#else
	RIOT6532(config, m_riot, MASTER_CLOCK_PAL / 3);
	m_riot->in_pa_callback().set(FUNC(a2600_state::switch_A_r));
	m_riot->out_pa_callback().set(FUNC(a2600_state::switch_A_w));
	m_riot->in_pb_callback().set_ioport("SWB");
	m_riot->out_pb_callback().set(FUNC(a2600_state::switch_B_w));
	m_riot->irq_callback().set(FUNC(a2600_state::irq_callback));
#endif

	VCS_CONTROL_PORT(config, CONTROL1_TAG, vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, CONTROL2_TAG, vcs_control_port_devices, nullptr);

	a2600_cartslot(config);
	subdevice<software_list_device>("cart_list")->set_filter("PAL");
}


void a2600_pop_state::a2600_pop(machine_config &config)
{
	a2600_base_ntsc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &a2600_pop_state::memory_map);

	VCS_CONTROL_PORT(config, CONTROL1_TAG, vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, CONTROL2_TAG, vcs_control_port_devices, "joy");
}


ROM_START(a2600)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
ROM_END

#define rom_a2600p rom_a2600

ROM_START( a2600_pop )
	ROM_REGION(0x30000, "maincpu", 0)
	// Boot/Game selection
	ROM_LOAD("136003_101.c6", 0x000, 0x800, CRC(9d3cfba6) SHA1(b84c34aaedd84fca30b6e8223ee062439acf4fe0))
	ROM_COPY("maincpu", 0x000, 0x800, 0x800)
	// Games
	// Hashes taken from the software list. These all need to be verified.
	// Judging by ROM labels from the parts list there is the possibility
	// that AIR-SEA BATTLE, ASTEROIDS, DODGE'EM, and SUPERMAN may have been
	// modified for the point of purchase unit.
	ROM_LOAD("c012013.d6", 0x1000, 0x1000, BAD_DUMP CRC(a6db4b3a) SHA1(e07e48d463d30321239a8acc00c490f27f1f7422)) // Prog ROM 1 ADVENTURE (CX2613)
	ROM_LOAD("c011202a.e6", 0x2000, 0x800, BAD_DUMP CRC(1fc6c0f1) SHA1(a746fdc82b336a9d499bf17f50b41e0193ba595e)) // Prog ROM 2 AIR-SEA BATTLE (CX2602)
	ROM_COPY("maincpu", 0x2000, 0x2800, 0x800)
	// Original game is 8KB, special 4KB version or is there banking hardware for this game?
	ROM_LOAD("c016449a.f6", 0x3000, 0x1000, NO_DUMP) // Prog ROM 3 ASTEROIDS (CX2649)
	ROM_LOAD("c012017.j6", 0x4000, 0x1000, BAD_DUMP CRC(e2a7ffa6) SHA1(9b1da7fbd0bf6fcadf1b60c11eeb31b6a61a03c3)) // Prog ROM 4 BACKGAMMON (CX2617)
	ROM_LOAD("c011224.k6", 0x5000, 0x800, BAD_DUMP CRC(711a2340) SHA1(9375c5c5298e81b37224dbde9bc5af151181ca27)) // Prog ROM 5 BASKETBALL (CX2624)
	ROM_COPY("maincpu", 0x5000, 0x5800, 0x800)
	ROM_LOAD("c011228.l6", 0x6000, 0x800, BAD_DUMP CRC(fea0d6d5) SHA1(cf6ce244b3edaad7ad5e9ca5f01668135c2f93d0)) // Prog ROM 6 BOWLING (CX2628)
	ROM_COPY("maincpu", 0x6000, 0x6800, 0x800)
	ROM_LOAD("c011222.m6", 0x7000, 0x800, BAD_DUMP CRC(3037638c) SHA1(8d473b87b70e26890268e6c417c0bb7f01e402eb)) // Prog ROM 7 BREAKOUT (CX2622)
	ROM_COPY("maincpu", 0x7000, 0x7800, 0x800)
	ROM_LOAD("c011207.c5", 0x8000, 0x800, BAD_DUMP CRC(e914b8ca) SHA1(b89443a0029e765c2716774fe2582be37650115c)) // Prog ROM 8 CANYON BOMBER (CX2607)
	ROM_COPY("maincpu", 0x8000, 0x8800, 0x800)
	ROM_LOAD("c012052.d5", 0x9000, 0x1000, BAD_DUMP CRC(420b8248) SHA1(08598101e38756916613f37581ef1b61c719016f)) // Prog ROM 9 CASINO (CX2652)
	ROM_LOAD("c012030.e5", 0xa000, 0x1000, BAD_DUMP CRC(a4b9a830) SHA1(6821c334d9cb85da17fa2a960636620966b91f96)) // Prog ROM 10 CIRCUS ATARI (CX2630)
	ROM_LOAD("c011201.f5", 0xb000, 0x800, BAD_DUMP CRC(9c326a97) SHA1(ce7580059e8b41cb4a1e734c9b35ce3774bf777a)) // Prog ROM 11 COMBAT (CX2601)
	ROM_COPY("maincpu", 0xb000, 0xb800, 0x800)
	ROM_LOAD("c012037a.j5", 0xc000, 0x1000, BAD_DUMP CRC(bc3602b5) SHA1(0ffc02c54190e9dd51ac1e52c911d3d7d730a40a)) // Prog ROM 12 DODGE'EM (CX2637)
	ROM_LOAD("c011225.k5", 0xd000, 0x800, BAD_DUMP CRC(3b73ee02) SHA1(c6fe4ce24bc1ebd538258d98cfe829963323acca)) // Prog ROM 13 FOOTBALL (CX2625)
	ROM_COPY("maincpu", 0xd000, 0xd800, 0x800)
	ROM_LOAD("c011234.l5", 0xe000, 0x800, BAD_DUMP CRC(46a9f200) SHA1(a25d52770408314dec6f41aaf5f9f0a2a3e2c18f)) // Prog ROM 14 GOLF (CX2634)
	ROM_COPY("maincpu", 0xe000, 0xe800, 0x800)
	ROM_LOAD("c012062.m5", 0xf000, 0x1000, BAD_DUMP CRC(c2bcc789) SHA1(561bccf508e162bc70c42d85c170cf0d1d4691a3)) // Prog ROM 15 HANGMAN (CX2662)
	ROM_LOAD("c011223.c4", 0x10000, 0x800, BAD_DUMP CRC(45ace998) SHA1(f362d2b3a50e5ae3c2b412b6c08ecdcfee47a688)) // Prog ROM 16 HOMERUN (CX2623)
	ROM_COPY("maincpu", 0x10000, 0x10800, 0x800)
	ROM_LOAD("c011227.d4", 0x11000, 0x800, BAD_DUMP CRC(f05a41e1) SHA1(d4b0b2aa379893356c72414ee0065a3a91cf9f97)) // Prog ROM 17 H. CANNONBALL (CX2627)
	ROM_COPY("maincpu", 0x11000, 0x11800, 0x800)
	ROM_LOAD("c012035.e4", 0x12000, 0x1000, BAD_DUMP CRC(0098e428) SHA1(aba25089d87cd6fee8d206b880baa5d938aae255)) // Prog ROM 18 MAZE CRAZE (CX2635)
	ROM_LOAD("c012038.f4", 0x13000, 0x1000, BAD_DUMP CRC(cff14904) SHA1(faa06bb0643dbf556b13591c31917d277a83110b)) // Prog ROM 19 MISSILE CMD (CX2638)
	ROM_LOAD("c011233.j4", 0x14000, 0x800, BAD_DUMP CRC(600d8a96) SHA1(372771aeb4e2fb2cd1dead5497e3821e4236d5fc)) // Prog ROM 20 NIGHT DRIVER (CX2633)
	ROM_COPY("maincpu", 0x14000, 0x14800, 0x800)
	ROM_LOAD("c011239.k4", 0x15000, 0x800, BAD_DUMP CRC(171ae72f) SHA1(cbecf1a32d9366a3dd4ad643916cd59cdc820a8b)) // Prog ROM 21 OTHELLO (CX2639)
	ROM_COPY("maincpu", 0x15000, 0x15800, 0x800)
	ROM_LOAD("c011205.l4", 0x16000, 0x800, BAD_DUMP CRC(68dd7acd) SHA1(f8eeaaf4635ac39b4bdf7ded1348bce46313ef9f)) // Prog ROM 22 OUTLAW (CX2605)
	ROM_COPY("maincpu", 0x16000, 0x16800, 0x800)
	ROM_LOAD("c012016.m4", 0x17000, 0x1000, BAD_DUMP CRC(3f59bb60) SHA1(832283530f5dee332f29cf8c4854dd554f2030a0)) // Prog ROM 23 SOCCER (CX2616)
	ROM_LOAD("c011229.c3", 0x18000, 0x800, BAD_DUMP CRC(00312ea9) SHA1(4dde18d4abc139562fdd7a9d2fd49a1f00a9e64a)) // Prog ROM 24 SKYDIVER (CX2629)
	ROM_COPY("maincpu", 0x18000, 0x18800, 0x800)
	ROM_LOAD("c011206.d3", 0x19000, 0x800, BAD_DUMP CRC(177dbdf8) SHA1(a2b13017d759346174e3d8dd53b6347222d3b85d)) // Prog ROM 25 SLOT RACERS (CX2606)
	ROM_COPY("maincpu", 0x19000, 0x19800, 0x800)
	ROM_LOAD("c012032.e3", 0x1a000, 0x1000, BAD_DUMP CRC(a6e867b3) SHA1(31d9668fe5812c3d2e076987ca327ac6b2e280bf)) // Prog ROM 26 SPACE INVADERS (CX2632)
	ROM_LOAD("c011212.f3", 0x1b000, 0x800, BAD_DUMP CRC(47592880) SHA1(bffb3d41916c83398624151eb00aa2a3acd23ab8)) // Prog ROM 27 STREET RACER (CX2612)
	ROM_COPY("maincpu", 0x1b000, 0x1b800, 0x800)
	ROM_LOAD("c012031b.j3", 0x1c000, 0x1000, BAD_DUMP CRC(39562bd7) SHA1(b9dee027c8d7dd2a46be111ab0b8363c1becc081)) // Prog ROM 28 SUPERMAN (CX2631)
	ROM_LOAD("c011218.k3", 0x1d000, 0x800, BAD_DUMP CRC(58805709) SHA1(21d983f2f52b84c22ecae84b0943678ae2c31c10)) // Prog ROM 29 3D TIC-TAC-TOE (CX2618)
	ROM_COPY("maincpu", 0x1d000, 0x1d800, 0x800)
	ROM_LOAD("c012036.l3", 0x1e000, 0x1000, BAD_DUMP CRC(3df33335) SHA1(babae88a832b76d8c5af6ea63b8f10a0da5bb992)) // Prog ROM 30 VIDEO CHECKERS (CX2636)
	ROM_LOAD("c012045.m3", 0x1f000, 0x1000, BAD_DUMP CRC(b6226a54) SHA1(043ef523e4fcb9fc2fc2fda21f15671bf8620fc3)) // Prog ROM 31 VIDEO CHESS (CX2645)
	ROM_LOAD("c011221.c2", 0x20000, 0x800, BAD_DUMP CRC(e4bc89c4) SHA1(1ffe89d79d55adabc0916b95cc37e18619ef7830)) // Prog ROM 32 VIDEO OLYMPICS (CX2621)
	ROM_COPY("maincpu", 0x20000, 0x20800, 0x800)
	ROM_LOAD("c012048.d2", 0x21000, 0x1000, BAD_DUMP CRC(10d95426) SHA1(2c16c1a6374c8e22275d152d93dd31ffba26271f)) // Prog ROM 33 VIDEO PINBALL (CX2648)
	ROM_LOAD("c012010.e2", 0x22000, 0x1000, BAD_DUMP CRC(cf174b57) SHA1(2d7563d337cbc0cdf4fc14f69853ab6757697788)) // Prog ROM 34 WAR LORDS (CX2610)
	ROM_LOAD("c012050.f2", 0x23000, 0x1000, BAD_DUMP CRC(2e8b4b5f) SHA1(08bcbc8954473e8f0242b881315b0af4466998ae)) // Prog ROM 35 BERZERK (CX2650)
	ROM_LOAD("c012054.j2", 0x24000, 0x1000, BAD_DUMP CRC(aa62d961) SHA1(1476c869619075b551b20f2c7f95b11e0d16aec1)) // Prog ROM 36 HAUNTED HOUSE (CX2654)
	ROM_LOAD("c012058.k2", 0x25000, 0x1000, BAD_DUMP CRC(ccc90c98) SHA1(18fac606400c08a0469aebd9b071ae3aec2a3cf2)) // Prog ROM 37 MATH GRAND PRIX (CX2658)
	ROM_LOAD("c012009.l2", 0x26000, 0x1000, BAD_DUMP CRC(0df43d8e) SHA1(79facc1bf70e642685057999f5c2b8e94b102439)) // Prog ROM 38 DEFENDER (CX2609)
	ROM_LOAD("c012055.m2", 0x27000, 0x1000, BAD_DUMP CRC(dfa1c825) SHA1(e2cd8996c1cf929e29130690024d1ec23d3b0bde)) // Prog ROM 39 YARS' REVENGE (CX2655)
	ROM_LOAD("c012046.c1", 0x28000, 0x1000, BAD_DUMP CRC(ddc9a881) SHA1(0940fea7f04cdb6d4b90c5ad1a7e344e68f6dbb1)) // Prog ROM 40 PAC-MAN (CX2646)
	ROM_LOAD("c012008.d1", 0x29000, 0x1000, BAD_DUMP CRC(b0f20d31) SHA1(ac2aad2196c155c1d87d6f42fa88891825f4fde6)) // Prog ROM 41 SUPER BREAKOUT (CX2608)
	ROM_LOAD("c012015.e1", 0x2a000, 0x1000, BAD_DUMP CRC(9b97c3da) SHA1(b45582de81c48b04c2bb758d69021e8088c70ce7)) // Prog ROM 42 DEMON/DIAMOND (CX2615)
	// empty slots f1, j1, k1, l1, m1 ?
ROM_END


/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME */
CONS( 1977, a2600,  0,      0,      a2600,   a2600, a2600_state, empty_init, "Atari",    "Atari 2600 (NTSC)" , MACHINE_SUPPORTS_SAVE )
CONS( 1978, a2600p, a2600,  0,      a2600p,  a2600, a2600_state, empty_init, "Atari",    "Atari 2600 (PAL)",   MACHINE_SUPPORTS_SAVE )

// Released in 1981/1982
// Games 35-42 are copyright 1982 and looking at the game list they seem to be
// added later.
GAME( 198?, a2600_pop, 0,      a2600_pop, a2600_pop, a2600_pop_state, empty_init, ROT0, "Atari",    "Atari 2600 Point of Purchase Display",   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
