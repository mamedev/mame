// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************
 *
 *  Williams Multigame (6-game version) driver, by Robbbert,
 *    2007/2008.
 *
 *  Thanks to assistance from Peale.
 *
 **************************************************************/

/*********************************************************************************************************

Game numbers:
   0 = The Menu
   1 = robotron
   2 = joustr
   3 = stargate
   4 = bubbles
   5 = defender
   6 = splat
   7 = spare position (duplicate of bubbles)

Some documents can be downloaded from http://www.multigame.com/williams.html

Hardware features:

There are 3 boards - the CPU board, the Sound board, and the Widget board.

- The Sound board only has the new sound ROM. The sounds are selected via the bank number from C400.

- The Widget board has the joysticks and buttons connected to it. These are steered to the correct
    inputs by a custom-programmed large square XC9572 84-pin chip. This is also controlled by
    the bank number from C400.

- The CPU board has 3 components:
-- A DS1225AD 8k x 8 NVRAM. Like the other chips, it is banked, giving each game its own save area.
-- The new cpu ROM, containing code for the 6 games and the menu. Splat and Defender are slightly
    modified to work properly on the Robotron hardware. Splat is converted from SC2 to SC1; while
    Defender has the I/O devices remapped to suit the hardware.
-- A square custom-programmed 44-pin chip (number rubbed off), which performs all the bank-switching,
    the special changes for Defender, the lock feature, and the sound cpu reset. It is not known
    which square chip detects and acts on the P1+P2 reset.


Setting it up:

    When you first start WMG, you need to initialise the NVRAM properly. Do this by starting wmg,
    pressing 1 to cycle through the games list, pressing 2 to run the game. When it says "FACTORY
    SETTINGS RESTORED", press F3 to return to the menu. Repeat this procedure for all 6 games.
    After that, quit wmg, then restart. On the Setup menu you can personalise the settings with
    the 1 and 2 keys. The inputs default to be the same as on the original hardware. You may
    change any input individually for each game. Note that the WMG hardware has the Flap and
    Inviso buttons wired together. JOUST can be played by 2 players at the same time, player 1
    uses the left joystick and the Flap button, while player 2 uses the right joystick and the
    Fire button. SPLAT would normally require 4 joysticks, but since the WMG only has 2, it is
    limited to a one-player game. Cocktail mode is not supported, WMG must be played on an upright
    only. After playing a game, returning to the menu will return to the place you left from. You
    can force a return to the Setup by Auto Up and pressing F3. You can also clear out the
    menu's NVRAM (not the game's NVRAM), by holding down L and pressing F3.

ToDo:

The game works perfectly. However the code is a bit of a bodge-job, originally for MisfitMAME 0.127.E,
then updated for HBMAME 0.148u1. It could do with a cleanup, and removal of the various hacks. Support
of save-state is also needed.


***********************************************************************************************************/

#include "emu.h"
#include "williams.h"

#include "cpu/m6800/m6800.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "speaker.h"


namespace {


class wmg_state : public williams_state
{
public:
	wmg_state(const machine_config &mconfig, device_type type, const char *tag)
		: williams_state(mconfig, type, tag)
		, m_p_ram(*this, "nvram", 0x2000, ENDIANNESS_BIG)
		, m_keyboard(*this, "X%d", 0U)
		, m_mainbank(*this, "mainbank")
		, m_codebank(*this, "codebank")
		, m_nvrambank(*this, "nvrambank")
		, m_soundbank(*this, "soundbank")
		, m_io_view(*this, "io_view")
	{ }

	void wmg(machine_config &config);

	template <int N> ioport_value wmg_mux_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_p_ram;
	required_ioport_array<17> m_keyboard;
	required_memory_bank m_mainbank;
	required_memory_bank m_codebank;
	required_memory_bank m_nvrambank;
	required_memory_bank m_soundbank;
	memory_view m_io_view;

	uint8_t m_wmg_c400 = 0U;
	uint8_t m_wmg_d000 = 0U;
	uint8_t m_port_select = 0U;

	u8 wmg_pia_0_r(offs_t offset);
	void wmg_c400_w(u8 data);
	void wmg_d000_w(u8 data);
	void wmg_port_select_w(int state);
	void wmg_sound_reset_w(u8 data);
	void wmg_vram_select_w(u8 data);

	void wmg_cpu1(address_map &map) ATTR_COLD;
	void wmg_cpu2(address_map &map) ATTR_COLD;
	void wmg_banked_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Address Map
 *
 *************************************/
void wmg_state::wmg_cpu1(address_map &map)
{
	map(0x0000, 0xbfff).ram().share(m_videoram);
	map(0x0000, 0x8fff).view(m_rom_view);
	m_rom_view[0](0x0000, 0x8fff).bankr(m_mainbank);
	map(0xc000, 0xcfff).view(m_io_view);
	m_io_view[0](0xc000, 0xc00f).mirror(0x03e0).writeonly().share(m_paletteram);
	m_io_view[0](0xc010, 0xc01f).mirror(0x03e0).lw8(NAME([this] (u8 data) { m_cocktail = BIT(data, 0); })); // TODO: should this really be here?  m_cocktail is set in wmg_vram_select_w
	m_io_view[0](0xc400, 0xc400).w(FUNC(wmg_state::wmg_c400_w));
	m_io_view[0](0xc401, 0xc401).w(FUNC(wmg_state::wmg_sound_reset_w));
	m_io_view[0](0xc804, 0xc807).r(FUNC(wmg_state::wmg_pia_0_r)).w(m_pia[0], FUNC(pia6821_device::write));
	m_io_view[0](0xc80c, 0xc80f).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	m_io_view[0](0xc900, 0xc9ff).nopr().w(FUNC(wmg_state::wmg_vram_select_w));
	m_io_view[0](0xca00, 0xca07).w(FUNC(wmg_state::blitter_w));
	m_io_view[0](0xcb00, 0xcbff).r(FUNC(wmg_state::video_counter_r));
	m_io_view[0](0xcbff, 0xcbff).w(FUNC(wmg_state::watchdog_reset_w));
	m_io_view[0](0xcc00, 0xcfff).bankrw(m_nvrambank);
	m_io_view[1](0xc000, 0xcfff).rom().region("maincpu", 0x58000); // Defender ROMs
	m_io_view[2](0xc000, 0xcfff).rom().region("maincpu", 0x59000);
	m_io_view[3](0xc000, 0xcfff).rom().region("maincpu", 0x5a000);
	m_io_view[4](0xc000, 0xcfff).rom().region("maincpu", 0x5b000);
	map(0xd000, 0xffff).bankr(m_codebank);
	map(0xd000, 0xd000).w(FUNC(wmg_state::wmg_d000_w));
}

void wmg_state::wmg_cpu2(address_map &map)
{
	map(0x0000, 0x007f).ram();     /* internal RAM */
	map(0x0080, 0x00ff).ram();     /* MC6810 RAM */
	map(0x0400, 0x0403).mirror(0x8000).rw(m_pia[2], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf000, 0xffff).bankr(m_soundbank);
}

/***************************************************************
 *
 *  Inputs, banked. One set for each game.
 *  Note: defaults from original games are used.
 *  JOYSTICK PLAYER(1) is really JOYSTICKLEFT on WMG hardware
 *  JOYSTICK PLAYER(2) is really JOYSTICKRIGHT on WMG hardware
 *
 ***************************************************************/
static INPUT_PORTS_START( wmg )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(wmg_state, wmg_mux_r<0>)

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(wmg_state, wmg_mux_r<1>)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

/* menu */
	PORT_START("X0") // IN000 (game,port,player)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_NAME("Menu/Left/Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_NAME("Menu/Left/Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_NAME("Menu/Left/Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_NAME("Menu/Left/Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_NAME("Menu/Right/Up")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_NAME("Menu/Right/Down")

	PORT_START("X1") // IN010
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_NAME("Menu/Right/Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_NAME("Menu/Right/Right")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Menu/Fire")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu/Thrust")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu/Smart Bomb")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Menu/Hyperspace")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Menu/Reverse")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Menu/Inviso or Flap")

/* robotron */
	PORT_START("X2") // IN100
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_NAME("Robotron/Left/Move Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_NAME("Robotron/Left/Move Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_NAME("Robotron/Left/Move Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_NAME("Robotron/Left/Move Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_NAME("Robotron/Right/Fire Up")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_NAME("Robotron/Right/Fire Down")

	PORT_START("X3") // IN110
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_NAME("Robotron/Right/Fire Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_NAME("Robotron/Right/Fire Right")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

/* joust */
	PORT_START("X4") // IN201
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1) PORT_NAME("Joust/P1/Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1) PORT_NAME("Joust/P1/Right")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Joust/P1/Flap")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xc8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X5") // IN202
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2) PORT_NAME("Joust/P2/Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2) PORT_NAME("Joust/P2/Right")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Joust/P2/Flap")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xc8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X6") // IN210
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

/* stargate */
	PORT_START("X7") // IN300
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Stargate/Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Stargate/Thrust")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Stargate/Smart Bomb")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Stargate/Hyperspace")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Stargate/Reverse")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_NAME("Stargate/Down")

	PORT_START("X8") // IN310
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_NAME("Stargate/Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Stargate/Inviso")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

/* bubbles */
	PORT_START("X9") // IN400
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_NAME("Bubbles/Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_NAME("Bubbles/Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_NAME("Bubbles/Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_NAME("Bubble/Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("X10") // IN410
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

/* defender */
	PORT_START("X11") // IN500
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Defender/Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Defender/Thrust")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Defender/Smart Bomb")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Defender/Hyperspace")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Defender/Reverse")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_2WAY PORT_NAME("Defender/Down")

	PORT_START("X12") // IN510
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_NAME("Defender/Up")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

/* splat - there are no P2 controls, so it can only be played by a single player */
	PORT_START("X13") // IN601
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Splat/P1/Left/Move Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Splat/P1/Left/Move Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Splat/P1/Left/Move Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Splat/P1/Left/Move Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Splat/P1/Right/Fire Up")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Splat/P1/Right/Fire Down")

	PORT_START("X14") // IN602
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xcf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("X15") // IN611
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Splat/P1/Right/Fire Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Splat/P1/Right/Fire Right")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("X16") // IN612
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

/*************************************
 *
 *  Bankswitching
 *
 *************************************/

/* switches the banks around when given a game number.
   The hardware has a lock feature, so that once a bank is selected, the next choice must be the menu */

void wmg_state::wmg_c400_w(u8 data)
{
	data &= 7;
	if (m_wmg_c400 == data)
		return;

	if ((data == 0) || (m_wmg_c400 == 0)) // we must be going to/from the menu
	{
		m_wmg_c400 = data;
		wmg_d000_w(0);                    // select I/O
		m_mainbank->set_entry(data);      // Graphics, etc.
		m_codebank->set_entry(data);      // Code
		m_nvrambank->set_entry(data);     // NVRAM
		m_soundbank->set_entry(data);     // Sound
		m_soundcpu->reset();
	}
}

void wmg_state::wmg_sound_reset_w(u8 data)
{
	/* This resets the sound card when bit 0 is low */
	if (!BIT(data, 0)) m_soundcpu->reset();
}

void wmg_state::wmg_vram_select_w(u8 data)
{
	/* VRAM/ROM banking from bit 0 */
	if (BIT(data, 0))
		m_rom_view.select(0);
	else
		m_rom_view.disable();

	/* cocktail flip from bit 1 */
	m_cocktail = BIT(data, 1);
}


// if defender, choose a rom, else enable i/o
void wmg_state::wmg_d000_w(u8 data)
{
	data &= 15;
	if (m_wmg_d000 == data)
		return;

	// defender && data > 0
	if ((m_wmg_c400 == 5) && (data))
	{
		// replace i/o with defender roms
		switch (data)
		{
			/* pages 1,2,3,7 map to ROM banks */
			case 1:
			case 2:
			case 3:
				m_io_view.select(data);
				break;

			case 7:
				m_io_view.select(4);
				break;

			default:
				logerror("Unknown bank %X selected\n",data);
		}
	}
	else if (data == 0)
	{
		// everything else - choose i/o space
		m_io_view.select(0);
	}

	m_wmg_d000 = data;
}


void wmg_state::machine_start()
{
	williams_state::machine_start();

	uint8_t *const cpu = memregion("maincpu")->base();
	uint8_t *const snd = memregion("soundcpu")->base();
	m_mainbank->configure_entries(0, 8, &cpu[0x00000], 0x10000);    // Graphics, etc.
	m_codebank->configure_entries(0, 8, &cpu[0x0d000], 0x10000);    // Code
	m_nvrambank->configure_entries(0, 8, &m_nvram[0], 0x400);       // NVRAM
	m_soundbank->configure_entries(0, 8, &snd[0x00000], 0x1000);    // Sound

	save_item(NAME(m_wmg_c400));
	save_item(NAME(m_wmg_d000));
	save_item(NAME(m_port_select));
}

void wmg_state::machine_reset()
{
	williams_state::machine_reset();

	m_wmg_c400 = 0xff;
	m_wmg_d000 = 0xff;
	m_port_select = 0;
	wmg_c400_w(0);
	m_maincpu->reset();
}

/*************************************
 *
 *  Input selector code
 *
 *************************************/

void wmg_state::wmg_port_select_w(int state)
{
	m_port_select = state | (m_wmg_c400 << 1);
}

template <int N>
ioport_value wmg_state::wmg_mux_r()
{
	if (N == 0)
	{
		uint8_t const ports[17] = { 0,0,2,2,5,4,7,7,9,9,11,11,14,13,9,9 };
		return m_keyboard[ports[m_port_select]]->read();
	}
	else
	{
		uint8_t const ports[17] = { 1,1,3,3,6,6,8,8,10,10,12,12,16,15,10,10 };
		return m_keyboard[ports[m_port_select]]->read();
	}
}

u8 wmg_state::wmg_pia_0_r(offs_t offset)
{
/* if player presses P1 and P2 in a game, return to the menu.
    Since there is no code in rom to handle this, it must be a hardware feature
    which probably just resets the cpu. */

	uint8_t const data = m_pia[0]->read(offset);

	if (!machine().side_effects_disabled())
	{
		if ((m_wmg_c400) && (offset == 0) && ((data & 0x30) == 0x30))   // P1 and P2 pressed
		{
			wmg_c400_w(0);
			m_maincpu->reset();
		}
	}

	return data;
}

/*************************************
 *
 *  Machine Driver
 *
 *************************************/

void wmg_state::wmg(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 12_MHz_XTAL;
	constexpr XTAL SOUND_CLOCK = 3.579545_MHz_XTAL;

	/* basic machine hardware */
	MC6809E(config, m_maincpu, MASTER_CLOCK/3/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &wmg_state::wmg_cpu1);

	M6808(config, m_soundcpu, SOUND_CLOCK);
	m_soundcpu->set_addrmap(AS_PROGRAM, &wmg_state::wmg_cpu2);

	// 8k x 8, banked
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// set a timer to go off every 32 scanlines, to toggle the VA11 line and update the screen
	TIMER(config, "scan_timer").configure_scanline(FUNC(wmg_state::va11_callback), "screen", 0, 32);

	// also set a timer to go off on scanline 240
	TIMER(config, "240_timer").configure_scanline(FUNC(wmg_state::count240_callback), "screen", 0, 240);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE | VIDEO_ALWAYS_UPDATE);
	m_screen->set_raw(MASTER_CLOCK*2/3, 512, 6, 298, 260, 7, 247);
	m_screen->set_screen_update(FUNC(wmg_state::screen_update));

	PALETTE(config, m_palette, FUNC(wmg_state::palette_init), 256);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC

	/* pia */
	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	INPUT_MERGER_ANY_HIGH(config, "soundirq").output_handler().set_inputline(m_soundcpu, M6808_IRQ_LINE);

	pia6821_device &pia0(PIA6821(config, "pia_0"));
	pia0.readpa_handler().set_ioport("IN0");
	pia0.readpb_handler().set_ioport("IN1");
	pia0.cb2_handler().set(FUNC(wmg_state::wmg_port_select_w));

	pia6821_device &pia1(PIA6821(config, "pia_1"));
	pia1.readpa_handler().set_ioport("IN2");
	pia1.writepb_handler().set(FUNC(wmg_state::snd_cmd_w));
	pia1.irqa_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<0>));
	pia1.irqb_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<1>));

	pia6821_device &pia2(PIA6821(config, "pia_2"));
	pia2.writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	pia2.irqa_handler().set("soundirq", FUNC(input_merger_any_high_device::in_w<0>));
	pia2.irqb_handler().set("soundirq", FUNC(input_merger_any_high_device::in_w<1>));

	m_blitter_config = WILLIAMS_BLITTER_SC1;
	m_blitter_clip_address = 0xc000;
}

/*************************************
 *
 *  Roms
 *
 *************************************/
ROM_START( wmg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wmg.cpu", 0x00000, 0x80000, CRC(975516ec) SHA1(571aaf9bff8ebfc36448cd9394b47bcfae7d1b83) )

	/* This little HACK! lets the menu boot up.
	It patches a jump to some new code, which sets a few memory locations, and sets the stack pointer.
	Then it jumps back to continue the main run. */

	ROM_COPY( "maincpu", 0xe0da, 0xf800, 0x0001b )
	ROM_FILL( 0xf81b, 1, 0x7e )
	ROM_FILL( 0xf81c, 1, 0xe0 )
	ROM_FILL( 0xf81d, 1, 0xba )
	ROM_FILL( 0xe0b7, 1, 0x7e )
	ROM_FILL( 0xe0b8, 1, 0xf8 )
	ROM_FILL( 0xe0b9, 1, 0x00 )

	ROM_REGION( 0x8000, "soundcpu", 0 )
	ROM_LOAD( "wmg.snd",         0x0000, 0x8000, CRC(1d08990e) SHA1(7bfb29426b3876f113e6ec3bc6c2fce9d2d1eb0c) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",       0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",       0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END

} // anonymous namespace


/*******************************************************
 *
 *  Game Driver
 *
 *******************************************************/

GAME( 2001, wmg, 0, wmg, wmg, wmg_state, empty_init, ROT0, "hack (Clay Cowgill)", "Williams Multigame", 0 )
