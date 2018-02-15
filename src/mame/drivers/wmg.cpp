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
    can force a return to the Setup by holding F1 and pressing F3. You can also clear out the
    menu's NVRAM (not the game's NVRAM), by holding down L and pressing F3.

ToDo:

The game works perfectly. However the code is a bit of a bodge-job, originally for MisfitMAME 0.127.E,
then updated for HBMAME 0.148u1. It could do with a cleanup, and removal of the various hacks. Support
of save-state is also needed.


***********************************************************************************************************/

#include "emu.h"
#include "includes/williams.h"

#include "cpu/m6800/m6800.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"


#define MASTER_CLOCK        (XTAL(12'000'000))
#define SOUND_CLOCK         (XTAL(3'579'545))

class wmg_state : public williams_state
{
public:
	wmg_state(const machine_config &mconfig, device_type type, const char *tag)
		: williams_state(mconfig, type, tag)
		, m_p_ram(*this, "nvram")
		, m_keyboard(*this, "X%d", 0)
	{ }

	DECLARE_MACHINE_RESET(wmg);
	DECLARE_DRIVER_INIT(wmg);
	DECLARE_READ8_MEMBER(wmg_nvram_r);
	DECLARE_WRITE8_MEMBER(wmg_nvram_w);
	DECLARE_READ8_MEMBER(wmg_pia_0_r);
	DECLARE_WRITE8_MEMBER(wmg_c400_w);
	DECLARE_WRITE8_MEMBER(wmg_d000_w);
	DECLARE_WRITE_LINE_MEMBER(wmg_port_select_w);
	DECLARE_WRITE8_MEMBER(wmg_sound_reset_w);
	DECLARE_WRITE8_MEMBER(wmg_vram_select_w);
	DECLARE_CUSTOM_INPUT_MEMBER(wmg_mux_r);

	void wmg(machine_config &config);
	void wmg_cpu1(address_map &map);
	void wmg_cpu2(address_map &map);
private:

	uint8_t m_wmg_c400;
	uint8_t m_wmg_d000;
	uint8_t m_wmg_port_select;
	uint8_t m_wmg_vram_bank;
	required_shared_ptr<uint8_t> m_p_ram;
	required_ioport_array<17> m_keyboard;
};



/*************************************
 *
 *  Address Map
 *
 *************************************/
ADDRESS_MAP_START(wmg_state::wmg_cpu1)
	AM_RANGE(0x0000, 0x8fff) AM_READ_BANK("bank1") AM_WRITEONLY AM_SHARE("videoram")
	AM_RANGE(0x9000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xcfff) AM_ROMBANK("bank7")
	AM_RANGE(0xd000, 0xffff) AM_ROMBANK("bank5")
	AM_RANGE(0xd000, 0xd000) AM_WRITE(wmg_d000_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(wmg_state::wmg_cpu2)
	AM_RANGE(0x0000, 0x007f) AM_RAM     /* internal RAM */
	AM_RANGE(0x0080, 0x00ff) AM_RAM     /* MC6810 RAM */
	AM_RANGE(0x0400, 0x0403) AM_MIRROR(0x8000) AM_DEVREADWRITE("pia_2", pia6821_device, read, write)
/* These next 2 are actually banked in CPU 1, but its not something Mame can handle very well. Placed here instead. */
	AM_RANGE(0xc000, 0xc00f) AM_MIRROR(0x03f0) AM_WRITEONLY AM_SHARE("paletteram")
	AM_RANGE(0xd000, 0xefff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf000, 0xffff) AM_ROMBANK("bank6")
ADDRESS_MAP_END

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
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, wmg_state, wmg_mux_r, "0")

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, wmg_state, wmg_mux_r, "1")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
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
 *  NVRAM (8k x 8), banked
 *
 *************************************/
READ8_MEMBER( wmg_state::wmg_nvram_r )
{
	return m_p_ram[offset+(m_wmg_c400<<10)];
}

WRITE8_MEMBER( wmg_state::wmg_nvram_w )
{
	m_p_ram[offset+(m_wmg_c400<<10)] = data;
}

/*************************************
 *
 *  Bankswitching
 *
 *************************************/

/* switches the banks around when given a game number.
   The hardware has a lock feature, so that once a bank is selected, the next choice must be the menu */

WRITE8_MEMBER( wmg_state::wmg_c400_w )
{
	data &= 7;
	if (m_wmg_c400 == data)
		return;

	if ((data == 0) || (m_wmg_c400 == 0))   // we must be going to/from the menu
	{
		m_wmg_c400 = data;
		wmg_d000_w( space, 0, 0); // select i/o
		uint8_t *cpu = memregion("maincpu")->base();
		memcpy( &cpu[0x10000], &cpu[(data << 16) + 0x20000], 0x9000 );      // Gfx etc
		membank("bank5")->set_entry(data);      // Code
		membank("bank6")->set_entry(data);      // Sound
		m_soundcpu->reset();
	}
}

WRITE8_MEMBER( wmg_state::wmg_sound_reset_w )
{
	/* This resets the sound card when bit 0 is low */
	if (!BIT(data, 0)) m_soundcpu->reset();
}

WRITE8_MEMBER( wmg_state::wmg_vram_select_w )
{
	/* VRAM/ROM banking from bit 0 */
	m_wmg_vram_bank = BIT(data, 0);
	membank("bank1")->set_entry(m_wmg_vram_bank);

	/* cocktail flip from bit 1 */
	m_cocktail = BIT(data, 1);
}


// if defender, choose a rom, else enable i/o
WRITE8_MEMBER( wmg_state::wmg_d000_w )
{
	data &= 15;
	if (m_wmg_d000 == data)
		return;

	// defender && data > 0
	if ((m_wmg_c400 == 5) && (data))
	{
		// replace i/o with defender roms
		if (m_wmg_d000 == 0) // was i/o?
		{
			space.install_read_bank(0xc000, 0xcfff, "bank7");
			space.unmap_write(0xc000, 0xcfff);
		}

		switch (data)
		{
			/* pages 1,2,3,7 map to ROM banks */
			case 1:
			case 2:
			case 3:
				membank("bank7")->set_entry(data);
				break;

			case 7:
				membank("bank7")->set_entry(4);
				break;

			default:
				printf("Unknown bank %X selected\n",data);
		}
	}
	else
	// everything else - choose i/o space
	if (data == 0)
	{
		/* install the i/o devices into c000-cfff */
		pia6821_device *pia0 = m_pia_0;
		pia6821_device *pia1 = m_pia_1;

		space.unmap_read(0xc000, 0xcfff); // throw out bank7
		space.install_write_bank       (0xc000, 0xc00f, "bank4");
		membank("bank4")->set_base(m_generic_paletteram_8);
		space.install_write_handler    (0xc010, 0xc01f, write8_delegate(FUNC(williams_state::defender_video_control_w),this));
		space.install_write_handler    (0xc400, 0xc400, write8_delegate(FUNC(wmg_state::wmg_c400_w),this));
		space.install_write_handler    (0xc401, 0xc401, write8_delegate(FUNC(wmg_state::wmg_sound_reset_w),this));
		space.install_readwrite_handler(0xc804, 0xc807, read8_delegate(FUNC(wmg_state::wmg_pia_0_r),this), write8_delegate(FUNC(pia6821_device::write), pia0));
		space.install_readwrite_handler(0xc80c, 0xc80f, read8_delegate(FUNC(pia6821_device::read), pia1), write8_delegate(FUNC(pia6821_device::write), pia1));
		space.install_write_handler    (0xc900, 0xc9ff, write8_delegate(FUNC(wmg_state::wmg_vram_select_w),this));
		space.install_write_handler    (0xca00, 0xca07, write8_delegate(FUNC(williams_state::williams_blitter_w),this));
		space.install_write_handler    (0xcbff, 0xcbff, write8_delegate(FUNC(williams_state::williams_watchdog_reset_w),this));
		space.install_read_handler     (0xcb00, 0xcbff, read8_delegate(FUNC(williams_state::williams_video_counter_r),this));
		space.install_readwrite_handler(0xcc00, 0xcfff, read8_delegate(FUNC(wmg_state::wmg_nvram_r), this), write8_delegate(FUNC(wmg_state::wmg_nvram_w),this));
	}

	m_wmg_d000 = data;
}


MACHINE_RESET_MEMBER( wmg_state, wmg )
{
	address_space &space1 = m_maincpu->space(AS_PROGRAM);
	m_wmg_c400=0xff;
	m_wmg_d000=0xff;
	m_wmg_port_select=0;
	m_wmg_vram_bank=0;
	wmg_c400_w( space1, 0, 0);
	MACHINE_RESET_CALL_MEMBER(williams_common);
	m_maincpu->reset();
}

/*************************************
 *
 *  Input selector code
 *
 *************************************/

WRITE_LINE_MEMBER( wmg_state::wmg_port_select_w )
{
	m_wmg_port_select = state | (m_wmg_c400 << 1);
}

CUSTOM_INPUT_MEMBER(wmg_state::wmg_mux_r)
{
	const char *port = (const char *)param;

	if (port[0] == '0')
	{
		uint8_t ports[17] = { 0,0,2,2,5,4,7,7,9,9,11,11,14,13,9,9 };
		return m_keyboard[ports[m_wmg_port_select]]->read();
	}
	else
	{
		uint8_t ports[17] = { 1,1,3,3,6,6,8,8,10,10,12,12,16,15,10,10 };
		return m_keyboard[ports[m_wmg_port_select]]->read();
	}
}

READ8_MEMBER( wmg_state::wmg_pia_0_r )
{
/* if player presses P1 and P2 in a game, return to the menu.
    Since there is no code in rom to handle this, it must be a hardware feature
    which probably just resets the cpu. */

	uint8_t data = m_pia_0->read(space, offset);

	if ((m_wmg_c400) && (offset == 0) && ((data & 0x30) == 0x30))   // P1 and P2 pressed
	{
		wmg_c400_w( space, 0, 0);
		m_maincpu->reset();
	}

	return data;
}

/*************************************
 *
 *  Driver Initialisation
 *
 *************************************/
DRIVER_INIT_MEMBER( wmg_state, wmg )
{
	uint8_t *cpu = memregion("maincpu")->base();
	uint8_t *snd = memregion("soundcpu")->base();
	membank("bank5")->configure_entries(0, 8, &cpu[0x2d000], 0x10000);  // Code
	membank("bank6")->configure_entries(0, 8, &snd[0x10000], 0x1000);   // Sound
	membank("bank7")->configure_entries(1, 4, &cpu[0x78000], 0x1000);   // Defender roms
	m_blitter_config = WILLIAMS_BLITTER_SC1;
	m_blitter_clip_address = 0xc000;
}

/*************************************
 *
 *  Machine Driver
 *
 *************************************/
MACHINE_CONFIG_START(wmg_state::wmg)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", MC6809E, MASTER_CLOCK/3/4)
	MCFG_CPU_PROGRAM_MAP(wmg_cpu1)

	MCFG_CPU_ADD("soundcpu", M6808, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(wmg_cpu2)

	MCFG_MACHINE_START_OVERRIDE(williams_state,williams)
	MCFG_MACHINE_RESET_OVERRIDE(wmg_state, wmg)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TIMER_DRIVER_ADD("scan_timer", williams_state, williams_va11_callback)
	MCFG_TIMER_DRIVER_ADD("240_timer", williams_state, williams_count240_callback)

	MCFG_WATCHDOG_ADD("watchdog")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE | VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK*2/3, 512, 6, 298, 260, 7, 247)
	MCFG_SCREEN_UPDATE_DRIVER(williams_state, screen_update_williams)

	MCFG_VIDEO_START_OVERRIDE(williams_state,williams)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("speaker")

	MCFG_SOUND_ADD("dac", MC1408, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.25) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)

	/* pia */
	MCFG_DEVICE_ADD("pia_0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("IN0"))
	MCFG_PIA_READPB_HANDLER(IOPORT("IN1"))
	MCFG_PIA_CB2_HANDLER(WRITELINE(wmg_state, wmg_port_select_w))

	MCFG_DEVICE_ADD("pia_1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("IN2"))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(williams_state, williams_snd_cmd_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams_state, williams_main_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams_state, williams_main_irq))

	MCFG_DEVICE_ADD("pia_2", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("dac", dac_byte_interface, write))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams_state,williams_snd_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams_state,williams_snd_irq))
MACHINE_CONFIG_END

/*************************************
 *
 *  Roms
 *
 *************************************/
ROM_START( wmg )
	ROM_REGION( 0xa0000, "maincpu", 0 )
	ROM_LOAD( "wmg.cpu", 0x20000, 0x80000, CRC(975516ec) SHA1(571aaf9bff8ebfc36448cd9394b47bcfae7d1b83) )

	/* This little HACK! lets the menu boot up.
	It patches a jump to some new code, which sets a few memory locations, and sets the stack pointer.
	Then it jumps back to continue the main run. */

	ROM_COPY( "maincpu", 0x2e0da, 0x2f800, 0x0001b )
	ROM_FILL( 0x2f81b, 1, 0x7e )
	ROM_FILL( 0x2f81c, 1, 0xe0 )
	ROM_FILL( 0x2f81d, 1, 0xba )
	ROM_FILL( 0x2e0b7, 1, 0x7e )
	ROM_FILL( 0x2e0b8, 1, 0xf8 )
	ROM_FILL( 0x2e0b9, 1, 0x00 )

	ROM_REGION( 0x18000, "soundcpu", 0 )
	ROM_LOAD( "wmg.snd",         0x10000, 0x8000, CRC(1d08990e) SHA1(7bfb29426b3876f113e6ec3bc6c2fce9d2d1eb0c) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",       0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",       0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END


/*******************************************************
 *
 *  Game Driver
 *
 *******************************************************/

GAME( 2001, wmg, 0, wmg, wmg, wmg_state, wmg, ROT0, "hack (Clay Cowgill)", "Williams Multigame", 0 )
