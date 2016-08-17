// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Interton Electronic VC 4000 cart emulation


 ***********************************************************************************************************/


/*  Game List and Emulation Status

 When you load a game it will normally appear to be unresponsive. Most carts contain a number of variants
 of each game (e.g. Difficulty, Player1 vs Player2 or Player1 vs Computer, etc).

 Press F2 (if needed) to select which game variant you would like to play. The variant number will increment
 on-screen. When you've made your choice, press F1 to start. The main keys are unlabelled, because an overlay
 is provided with each cart. See below for a guide. You need to read the instructions that come with each game.

 In some games, the joystick is used like 4 buttons, and other games like a paddle. The two modes are
 incompatible when using a keyboard. Therefore (in the emulation) a config dipswitch is used. The preferred
 setting is listed below.

 (AC = Auto-centre, NAC = no auto-centre, 90 = turn controller 90 degrees).

 The list is rather incomplete, information will be added as it becomes available.

 The game names and numbers were obtained from the Amigan Software site.

 Cart Num    Name
 ----------------------------------------------
 1.      Grand Prix / Car Races / Autosport / Motor Racing / Road Race
 Config: Paddle, NAC
 Status: Working
 Controls: Left-Right: Steer; Up: Accelerate

 2.      Black Jack
 Status: Not working (some digits missing; indicator missing; dealer's cards missing)
 Controls: set bet with S and D; A to deal; 1 to hit, 2 to stay; Q accept insurance, E to decline; double-up (unknown key)
 Indicator: E make a bet then deal; I choose insurance; - you lost; + you won; X hit or stay

 3.      Olympics / Paddle Games / Bat & Ball / Pro Sport 60 / Sportsworld
 Config: Paddle, NAC
 Status: Working

 4.      Tank Battle / Combat
 Config: Button, 90
 Status: Working
 Controls: Left-Right: Steer; Up: Accelerate; Fire: Shoot

 5.      Maths 1
 Status: Working
 Controls: Z difficulty; X = addition or subtraction; C ask question; A=1;S=2;D=3;Q=4;W=5;E=6;1=7;2=8;3=9;0=0; C enter

 6.      Maths 2
 Status: Not working
 Controls: Same as above.

 7.      Air Sea Attack / Air Sea Battle
 Config: Button, 90
 Status: Working
 Controls: Left-Right: Move; Fire: Shoot

 8.      Treasure Hunt / Capture the Flag / Concentration / Memory Match
 Config: Buttons
 Status: Working

 9.      Labyrinth / Maze / Intelligence 1
 Config: Buttons
 Status: Working

 10.     Winter Sports
 Notes: Background colours should be Cyan and White instead of Red and Black

 11.     Hippodrome / Horse Race

 12.     Hunting / Shooting Gallery

 13.     Chess 1
 Status: Can't see what you're typing, wrong colours

 14.     Moto-cros

 15.     Four in a row / Intelligence 2
 Config: Buttons
 Status: Working
 Notes: Seems the unused squares should be black. The screen jumps about while the computer is "thinking".

 16.     Code Breaker / Master Mind / Intelligence 3 / Challenge

 17.     Circus
 STatus: severe gfx issues

 18.     Boxing / Prize Fight

 19.     Outer Space / Spacewar / Space Attack / Outer Space Combat

 20.     Melody Simon / Musical Memory / Follow the Leader / Musical Games / Electronic Music / Face the Music

 21.     Capture / Othello / Reversi / Attack / Intelligence 4
 Config: Buttons
 Status: Working
 Notes: Seems the unused squares should be black

 22.     Chess 2
 Status: Can't see what you're typing, wrong colours

 23.     Pinball / Flipper / Arcade
 Status: gfx issues

 24.     Soccer

 25.     Bowling / NinePins
 Config: Paddle, rotated 90 degrees, up/down autocentre, left-right does not
 Status: Working

 26.     Draughts

 27.     Golf
 Status: gfx issues

 28.     Cockpit
 Status: gfx issues

 29.     Metropolis / Hangman
 Status: gfx issues

 30.     Solitaire

 31.     Casino
 Status: gfx issues, items missing and unplayable
 Controls: 1 or 3=START; q=GO; E=STOP; D=$; Z=^; X=tens; C=units

 32.     Invaders / Alien Invasion / Earth Invasion
 Status: Works
 Config: Buttons

 33.     Super Invaders
 Status: Stars are missing, colours are wrong
 Config: Buttons (90)

 36.     BackGammon
 Status: Not all counters are visible, Dice & game number not visible.
 Controls: Fire=Exec; 1=D+; 3=D-; Q,W,E=4,5,6; A,S,D=1,2,3; Z=CL; X=STOP; C=SET

 37.     Monster Man / Spider's Web
 Status: Works
 Config: Buttons

 38.     Hyperspace
 Status: Works
 Config: Buttons (90)
 Controls: 3 - status button; Q,W,E,A,S,D,Z,X,C selects which galaxy to visit


 40.     Super Space
 Status: Works, some small gfx issues near the bottom
 Config: Buttons



 Acetronic: (dumps are compatible)
 ------------

 * Shooting Gallery
 Status: works but screen flickers
 Config: Buttons

 * Planet Defender
 Status: Works
 Config: Paddle (NAC)

 * Laser Attack
 Status: Works
 Config: Buttons



 Public Domain: (written for emulators, may not work on real hardware)
 ---------------
 * Picture (no controls) - works
 * Wincadia Stub (no controls) - works, small graphic error */



#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  vc4000_rom_device - constructor
//-------------------------------------------------

const device_type VC4000_ROM_STD = &device_creator<vc4000_rom_device>;
const device_type VC4000_ROM_ROM4K = &device_creator<vc4000_rom4k_device>;
const device_type VC4000_ROM_RAM1K = &device_creator<vc4000_ram1k_device>;
const device_type VC4000_ROM_CHESS2 = &device_creator<vc4000_chess2_device>;


vc4000_rom_device::vc4000_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_vc4000_cart_interface( mconfig, *this )
{
}

vc4000_rom_device::vc4000_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, VC4000_ROM_STD, "VC 4000 Standard Carts", tag, owner, clock, "vc4000_rom", __FILE__),
						device_vc4000_cart_interface( mconfig, *this )
{
}

vc4000_rom4k_device::vc4000_rom4k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: vc4000_rom_device(mconfig, VC4000_ROM_ROM4K, "VC 4000 Carts w/4K ROM", tag, owner, clock, "vc4000_rom4k", __FILE__)
{
}

vc4000_ram1k_device::vc4000_ram1k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: vc4000_rom_device(mconfig, VC4000_ROM_RAM1K, "VC 4000 Carts w/1K RAM", tag, owner, clock, "vc4000_ram1k", __FILE__)
{
}

vc4000_chess2_device::vc4000_chess2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: vc4000_rom_device(mconfig, VC4000_ROM_CHESS2, "VC 4000 Chess II Cart", tag, owner, clock, "vc4000_chess2", __FILE__)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(vc4000_rom_device::read_rom)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}


READ8_MEMBER(vc4000_ram1k_device::read_ram)
{
	return m_ram[offset & (m_ram.size() - 1)];
}

WRITE8_MEMBER(vc4000_ram1k_device::write_ram)
{
	m_ram[offset & (m_ram.size() - 1)] = data;
}


READ8_MEMBER(vc4000_chess2_device::extra_rom)
{
	if (offset < (m_rom_size - 0x2000))
		return m_rom[offset + 0x2000];
	else
		return 0xff;
}

READ8_MEMBER(vc4000_chess2_device::read_ram)
{
	return m_ram[offset & (m_ram.size() - 1)];
}

WRITE8_MEMBER(vc4000_chess2_device::write_ram)
{
	m_ram[offset & (m_ram.size() - 1)] = data;
}
