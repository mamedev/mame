// license:BSD-3-Clause
// copyright-holders:David Haywood, R.Belmont

/*
    Radica Games 6502 based 'TV Game' hardware

    These use a 6502 derived CPU under a glob
    The CPU die is marked 'ELAN EU3A05'

    There is a second glob surrounded by TSOP48 pads
    this contains the ROM

    Space Invaders uses a 3rd glob marked
    AMIC (C) (M) 1998-1 AM3122A
    this is presumably for the bitmap layer on Qix

    --
    Known games on this hardware

    Tetris
    Space Invaders
    ABL Air-Blaster Joystick

    ---
    XaviX plug and play units almost always have a XaviX logo on the external packaging
    while the ones for this driver (and SunPlus etc.) don't seem to have any specific
    markings.

    Notes:

    Tetris - RAM 0xa0 and 0xa1 contain the ACD0 and AD1 values and player 2 controls if
    between certain values? probably read via serial (or ADC abuse?)

    Internal Test Menus:

    Tetris - hold P1 Down + P1 Anticlockwise (Button 2) on boot
    Space Invaders - hold P1 Down + P1 Button 1 on boot
    ABL Air-Blaster - none?

    -----------------------------------------------------
    Flaws (NOT emulation bugs, happen on hardware):
    -----------------------------------------------------

    rad_sinv:

    In QIX the sprites lag behind the line drawing, so you see the line infront of your player until you stop moving

    In Space Invaders the UFO can sometimes glitch for a frame when appearing, and wraps around at the edges
      (even if the hardware supports having higher priority tiles to prevent this, as used by Lunar Rescue, it isn't
       used here)

    Colony 7 has a typo in the instructions

    The fake 'colour band' effect does not apply to the thruster (and several other elements) in Lunar Rescue

    Enemies in Phoenix are rendered above the score panel

    The 200pt right facing bird on the Phoenix score table is corrupt

    Space Invaders seems to be using a darker than expected palette, there are lighter colours in the palette but
    they don't seem to be used.  It's difficult to judge from hardware videos, although it definitely isn't as
    white as the menu, so this might also be a non-bug. (Uncertain - to check)

    -------------------------

    airblasjs:

    This game is very buggy.

    The 3D stages are prone to softlocking when the refuel jet is meant to appear.

    2D stages will zap you of your lives and then continues one by one if you die on a boss meaning if you have
    2 continues left you'll be offered the continue screen twice while it drains you of your lives before
    actually presenting you with the Game Over screen.  The manual claims you can't continue on a boss however
    this isn't true for the 3D stages, where the continue feature works as expected.  Either way, this is a very
    crude way of implementing a 'no continue' feature on bosses if it isn't simply a bug in the game code that
    was explained away as a feature.

    Sprites clip on / off the top of the screen in parts - if you move your the player helipcopter to the top
    of the screen the top 8 pixels clip off too (not currently happening in MAME, probably need to take out
    sprite wrapping on y)

    Sprites wrap around on X too, if you move to the left edge you can see your shadow on the right etc.

    Sound sometimes stops working properly / shot changes for no reason.

    There's no indication of damage most of the time on bosses, some parts won't take damage until other parts
    have been destroyed, not always obvious.

    Very heavy sprite flicker (not emulated)

    Very heavy slowdown (MAME speed is approximate)

*/

/*

Buzztime Trivia notes from Tahg (addresses based on base ROM, not cartridges)

300 Cursor Left
301 Cursor Top
302 Cursor Right
303 Cursor Bottom

3E5 Sent Command
3E6 Watchdog Counter
3E9 Active Player
3EA Sent Player Id/Incoming Byte
3EB Sent Player Button/Loop Counter

0-A Name
E-F Score
44x Player 1 Data
...
4Fx Player 12 Data

509 Cursor index
515 Name on Enter name screen

These read a byte, (aaabbbcc) MSB first on IO 5041. In general:
Set 5043 bit 0 high.
Repeat 8 times
  Wait for 5041 bit 0 to go high
  Read 5041 bit 1
Set 5043 bit 0 low

6B29  ReadCommandA  Exit on c=2 or c=1,b=7,a=7  Watchdog resets counters and reads port again
6BE8  ReadCommandB  Exit on c=2 or c=1,b=7,a=7  Watchdog exits with b=FF if 3E7 nonzero
6CB4  ReadCommandC  Exit on c=2                 Watchdog exits with b=FF

  Address         Publics by Value

 00000000:00000000       byte_0
 00000000:0000050A       index
 00000000:00006011       LoadTileSet
 00000000:0000605B       LoadPalette
 00000000:00006090       SetTileBase
 00000000:000060D7       DisableSprites
 00000000:000060F0       SetSpriteBase
 00000000:0000648F       Play0
 00000000:00006494       Play1
 00000000:00006499       Play2
 00000000:0000649E       Play3
 00000000:000064A3       Play4
 00000000:000064A8       Play5
 00000000:000064AD       Play6
 00000000:000064B2       Play7
 00000000:000064B7       Play8
 00000000:000064BC       Play9
 00000000:000064C1       PlayA
 00000000:000064C6       PlayB
 00000000:000064CB       DisableSoundCh0
 00000000:000064EA       DisableSoundCh1
 00000000:00006509       DisableSoundCh2
 00000000:00006528       DisableSoundCh3
 00000000:00006547       DisableSoundCh4
 00000000:00006566       DisableSoundCh5
 00000000:00006585       DisableSoundChAll
 00000000:000065AC       WaitForVBIAndSetSpriteBase
 00000000:000065B5       WaitForVBIAndSetTileBase
 00000000:000065BE       WaitForVBIAndLoadTileSet
 00000000:000065C7       JJLoadTileSet
 00000000:00006933       DrawBackground
 00000000:00006B29       ReadCommandA
 00000000:00006BE8       ReadCommandB
 00000000:00006CB4       ReadCommandC
 00000000:00006D66       DrawText
 00000000:00007E80       WaitXTimesForInput
 00000000:000099B4       NameScreenLoop
 00000000:00009EB0       NameScreenSetChar
 00000000:00009F43       NameScreenDeleteChar
 00000000:00009FC8       NameScreenCalcCharacter
 00000000:0000A02F       NameScreenCalculateCursor
 00000000:0000A051       NameScreenUpdateCursor
 00000000:0000A112       SelectPlayersLoop
 00000000:0000A290       SelectPlayer
 00000000:0000A31E       CreatePlayer
 00000000:0000A769       ReadAAndWaitForSelect
 00000000:0000A821       DrawRoundTitles
 00000000:0000AD8A       ReadAAndSetPlayer
 00000000:0000ADDF       ReadAAndCheckPlayer
 00000000:0000AF1F       ShowQuestion
 00000000:0000AF65       ShowChoices
 00000000:0000AFC5       ShowHint1
 00000000:0000B004       ShowHint2
 00000000:0000B033       ShowHint3
 00000000:0000B061       ShowAnswer
 00000000:0000B0BF       ShowReason
 00000000:0000B1C0       ShowScore
 00000000:0000C848       MenuUpdate
 00000000:0000D10D       WinScreen
 00000000:0000D1F1       DoFireworks
 00000000:0000E9AE       JLoadTileSet
 00000000:0000E9B6       JLoadPalette
 00000000:0000E9C2       JSetTileBase
 00000000:0000E9C8       JSetSpriteBase
 00000000:0000E9CE       JDisableSprites
 00000000:0000E9D4       SetGamePage
 00000000:0000E9DF       MemoryTest
 00000000:0000EA36       SRAMTest
 00000000:0000EBA9       nullsub_1
 00000000:0000EBAA       WaitForVBI
 00000000:0000EBCA       RomTest
 00000000:0000EC24       WaitForTimer
 00000000:0000ECE9       nullsub_2

*/

#include "emu.h"

#include "elan_eu3a05_a.h"

#include "cpu/m6502/m6502.h"
//#include "cpu/m6502/m65c02.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "machine/bankdev.h"
#include "elan_eu3a05gpio.h"
#include "elan_eu3a05sys.h"
#include "elan_eu3a05vid.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


namespace {

class elan_eu3a05_state : public driver_device
{
public:
	elan_eu3a05_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sys(*this, "sys"),
		m_gpio(*this, "gpio"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_ram(*this, "ram"),
		m_sound(*this, "eu3a05sound"),
		m_vid(*this, "vid"),
		m_pixram(*this, "pixram"),
		m_bank(*this, "bank"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void elan_eu3a05(machine_config &config);
	void airblsjs(machine_config& config);

	void elan_sudoku(machine_config &config);
	void elan_sudoku_pal(machine_config &config);
	void elan_pvmilfin(machine_config &config);

	void init_sudelan();
	void init_sudelan3();

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<elan_eu3a05sys_device> m_sys;
	required_device<elan_eu3a05gpio_device> m_gpio;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);

	// for callback
	uint8_t read_full_space(offs_t offset);

	void elan_eu3a05_bank_map(address_map &map) ATTR_COLD;
	void elan_eu3a05_map(address_map &map) ATTR_COLD;
	void elan_sudoku_map(address_map &map) ATTR_COLD;


	virtual void video_start() override ATTR_COLD;

	required_shared_ptr<uint8_t> m_ram;
	required_device<elan_eu3a05_sound_device> m_sound;
	required_device<elan_eu3a05vid_device> m_vid;
	required_shared_ptr<uint8_t> m_pixram;
	required_device<address_map_bank_device> m_bank;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void sound_end0(int state) { m_sys->generate_custom_interrupt(2); }
	void sound_end1(int state) { m_sys->generate_custom_interrupt(3); }
	void sound_end2(int state) { m_sys->generate_custom_interrupt(4); }
	void sound_end3(int state) { m_sys->generate_custom_interrupt(5); }
	void sound_end4(int state) { m_sys->generate_custom_interrupt(6); }
	void sound_end5(int state) { m_sys->generate_custom_interrupt(7); }
};

class elan_eu3a05_buzztime_state : public elan_eu3a05_state
{
public:
	elan_eu3a05_buzztime_state(const machine_config &mconfig, device_type type, const char *tag) :
		elan_eu3a05_state(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{ }

	void elan_buzztime(machine_config& config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	//uint8_t random_r() { return machine().rand(); }
	uint8_t porta_r();
	void portb_w(uint8_t data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
};


class elan_eu3a05_pvwwcas_state : public elan_eu3a05_state
{
public:
	elan_eu3a05_pvwwcas_state(const machine_config &mconfig, device_type type, const char *tag) :
		elan_eu3a05_state(mconfig, type, tag),
		m_prevport_c(0xff)
	{ }

	void pvwwcas(machine_config& config);

	void init_pvwwcas();

protected:

private:
	uint8_t pvwwc_portc_r();
	void pvwwc_portc_w(uint8_t data);
	uint8_t m_prevport_c;
};

void elan_eu3a05_buzztime_state::machine_start()
{
	elan_eu3a05_state::machine_start();

	// if there's a cart make sure we can see it
	if (m_cart && m_cart->exists())
	{
		uint8_t *rom = memregion("maincpu")->base();
		uint8_t* cart = m_cart->get_rom_base();
		std::copy(&cart[0x000000], &cart[0x200000], &rom[0x200000]);
	}
	else
	{
		uint8_t *rom = memregion("maincpu")->base();
		uint8_t* bios = memregion("bios")->base();
		std::copy(&bios[0x000000], &bios[0x200000], &rom[0x200000]);
	}
}

DEVICE_IMAGE_LOAD_MEMBER(elan_eu3a05_buzztime_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	if (size != 0x20'0000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (only 2M supported)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_NATIVE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void elan_eu3a05_buzztime_state::elan_buzztime(machine_config &config)
{
	elan_eu3a05_state::elan_eu3a05(config);

	m_sys->set_alt_timer();

	m_gpio->read_0_callback().set(FUNC(elan_eu3a05_buzztime_state::porta_r)); // I/O lives in here
//  m_gpio->read_1_callback().set(FUNC(elan_eu3a05_buzztime_state::random_r)); // nothing of note
//  m_gpio->read_2_callback().set(FUNC(elan_eu3a05_buzztime_state::random_r)); // nothing of note
	m_gpio->write_1_callback().set(FUNC(elan_eu3a05_buzztime_state::portb_w)); // control related

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "buzztime_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(elan_eu3a05_buzztime_state::cart_load));

	SOFTWARE_LIST(config, "buzztime_cart").set_original("buzztime_cart");
}

uint8_t elan_eu3a05_buzztime_state::porta_r()
{
	logerror("%s: porta_r\n", machine().describe_context());
	return machine().rand();
}

void elan_eu3a05_buzztime_state::portb_w(uint8_t data)
{
	logerror("%s: portb_w %02x\n", machine().describe_context(), data);
}


void elan_eu3a05_state::video_start()
{
}


uint32_t elan_eu3a05_state::screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	return m_vid->screen_update(screen, bitmap, cliprect);
}

// sound callback
uint8_t elan_eu3a05_state::read_full_space(offs_t offset)
{
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	return fullbankspace.read_byte(offset);
}

// code at 8bc6 in Air Blaster makes unwanted reads, why, bug in code, flow issue?
//[:maincpu] ':maincpu' (8BC6): unmapped program memory read from 5972 & FF


void elan_eu3a05_state::elan_eu3a05_map(address_map &map)
{
	// can the addresses move around?
	map(0x0000, 0x3fff).ram().share("ram");
	map(0x4800, 0x49ff).rw(m_vid, FUNC(elan_eu3a05commonvid_device::palette_r), FUNC(elan_eu3a05commonvid_device::palette_w));

	map(0x5000, 0x501f).m(m_sys, FUNC(elan_eu3a05sys_device::map)); // including DMA controller
	map(0x5020, 0x503f).m(m_vid, FUNC(elan_eu3a05vid_device::map));

	// 504x GPIO area?
	map(0x5040, 0x5046).rw(m_gpio, FUNC(elan_eu3a05gpio_device::gpio_r), FUNC(elan_eu3a05gpio_device::gpio_w));
	// 5047
	map(0x5048, 0x504a).w(m_gpio, FUNC(elan_eu3a05gpio_device::gpio_unk_w));

	// 506x unknown
	map(0x5060, 0x506d).ram(); // read/written by tetris (ADC?)

	// 508x sound
	map(0x5080, 0x50bf).m(m_sound, FUNC(elan_eu3a05_sound_device::map));

	//map(0x5000, 0x50ff).ram();
	map(0x6000, 0xdfff).m(m_bank, FUNC(address_map_bank_device::amap8));

	map(0xe000, 0xffff).rom().region("maincpu", 0x3f8000);
	// not sure how these work, might be a modified 6502 core instead.
	map(0xfffa, 0xfffb).r(m_sys, FUNC(elan_eu3a05commonsys_device::nmi_vector_r)); // custom vectors handled with NMI for now
	//map(0xfffe, 0xffff).r(m_sys, FUNC(elan_eu3a05commonsys_device::irq_vector_r));  // allow normal IRQ for brk
}

// default e000 mapping is the same as eu3a14, other registers seem closer to eua05, probably a different chip type
void elan_eu3a05_state::elan_sudoku_map(address_map& map)
{
	elan_eu3a05_map(map);
	map(0xe000, 0xffff).rom().region("maincpu", 0x0000);
	// not sure how these work, might be a modified 6502 core instead.
	map(0xfffa, 0xfffb).r(m_sys, FUNC(elan_eu3a05commonsys_device::nmi_vector_r)); // custom vectors handled with NMI for now
	//map(0xfffe, 0xffff).r(m_sys, FUNC(elan_eu3a05commonsys_device::irq_vector_r));  // allow normal IRQ for brk
}


void elan_eu3a05_state::elan_eu3a05_bank_map(address_map &map)
{
	map(0x000000, 0xffffff).noprw(); // shut up any logging when video params are invalid
	map(0x000000, 0x3fffff).rom().region("maincpu", 0);
	map(0x400000, 0x40ffff).ram(); // ?? only ever cleared maybe a mirror of below?
	map(0x800000, 0x80ffff).ram().share("pixram"); // Qix writes here and sets the tile base here instead of ROM so it can have a pixel layer
}

static INPUT_PORTS_START( rad_sinv )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) // MENU
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static INPUT_PORTS_START( rad_tetr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Anticlockwise
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Clockwise
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // and Select
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Player 2 inputs must be read via serial or similar
	   the game doesn't read them directly, or even let
	   you select player 2 mode by default
	*/

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END


static INPUT_PORTS_START( airblsjs )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Pause")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Trigger")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Missile")
INPUT_PORTS_END


static INPUT_PORTS_START( sudoku )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sudoku2p )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( carlecfg )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Start/Pause")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Select")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) // guess, not used?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // guess, not used?

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



void elan_eu3a05_state::machine_start()
{
}

void elan_eu3a05_state::machine_reset()
{
	/* the 6502 core sets the default stack value to 0x01bd
	   and Tetris does not initialize it to anything else

	   Tetris stores the playfield data at 0x100 - 0x1c7 and
	   has a clear routine that will erase that range and
	   trash the stack

	   It seems likely this 6502 sets it to 0x1ff by default
	   at least.

	   According to
	   http://mametesters.org/view.php?id=6486
	   this isn't right for known 6502 types either
	*/
	m_maincpu->set_state_int(M6502_S, 0x1ff);
}

static const gfx_layout helper_4bpp_8_layout =
{
	8,1,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ STEP8(0,4) },
	{ 0 },
	8 * 4
};

static const gfx_layout helper_8bpp_8_layout =
{
	8,1,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,8) },
	{ 0 },
	8 * 8
};


// these are fake just to make looking at the texture pages easier
static const uint32_t texlayout_xoffset_8bpp[256] = { STEP256(0,8) };
static const uint32_t texlayout_yoffset_8bpp[256] = { STEP256(0,256*8) };
static const gfx_layout texture_helper_8bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*8,
	texlayout_xoffset_8bpp,
	texlayout_yoffset_8bpp
};

static const uint32_t texlayout_xoffset_4bpp[256] = { STEP256(0,4) };
static const uint32_t texlayout_yoffset_4bpp[256] = { STEP256(0,256*4) };
static const gfx_layout texture_helper_4bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*4,
	texlayout_xoffset_4bpp,
	texlayout_yoffset_4bpp
};

static GFXDECODE_START( gfx_elan_eu3a05_fake )
	GFXDECODE_ENTRY( "maincpu", 0, helper_4bpp_8_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, texture_helper_4bpp_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, helper_8bpp_8_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, texture_helper_8bpp_layout,  0x0, 1  )
GFXDECODE_END



INTERRUPT_GEN_MEMBER(elan_eu3a05_state::interrupt)
{
	m_sys->generate_custom_interrupt(9);
}


/* Tetris (PAL version)      has XTAL of 21.281370
   Air Blaster (PAL version) has XTAL of 21.2813
   what are the NTSC clocks?

   not confirmed on Space Invaders, actual CPU clock unknown.
   21281370 is the same value as a PAL SNES

   game logic speed (but not level scroll speed) in Air Blaster appears to be limited entirely by CPU speed (and therefore needs to be around 2-3mhz
   at most to match hardware) - a divider of 8 gives something close to original hardware
   it is unclear exactly what limits the clock speed (maybe video / sound causes waitstates? - dma in progress could also slow / stop the CPU
   and is not going to be 'instant' on hardware)

   using a low clock speed also helps with the badly programmed controls in Tetris as that likewise seems to run the game logic 'as fast as possible'
   there don't appear to be any kind of blanking bits being checked.
*/

void elan_eu3a05_state::elan_eu3a05(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(21'281'370)/8); // wrong, this is the PAL clock
	m_maincpu->set_addrmap(AS_PROGRAM, &elan_eu3a05_state::elan_eu3a05_map);
	m_maincpu->set_vblank_int("screen", FUNC(elan_eu3a05_state::interrupt));

	ADDRESS_MAP_BANK(config, "bank").set_map(&elan_eu3a05_state::elan_eu3a05_bank_map).set_options(ENDIANNESS_LITTLE, 8, 24, 0x8000);

	PALETTE(config, m_palette).set_entries(256);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(elan_eu3a05_state::screen_update));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_elan_eu3a05_fake);

	ELAN_EU3A05_GPIO(config, m_gpio, 0);
	m_gpio->read_0_callback().set_ioport("IN0");
	m_gpio->read_1_callback().set_ioport("IN1");
	m_gpio->read_2_callback().set_ioport("IN2");

	ELAN_EU3A05_SYS(config, m_sys, 0);
	m_sys->set_cpu("maincpu");
	m_sys->set_addrbank("bank");

	ELAN_EU3A05_VID(config, m_vid, 0);
	m_vid->set_cpu("maincpu");
	m_vid->set_addrbank("bank");
	m_vid->set_palette("palette");
	m_vid->set_entries(256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ELAN_EU3A05_SOUND(config, m_sound, 8000);
	m_sound->space_read_callback().set(FUNC(elan_eu3a05_state::read_full_space));
	m_sound->add_route(ALL_OUTPUTS, "mono", 1.0);
	/* just causes select sound to loop in Tetris for now!
	m_sound->sound_end_cb<0>().set(FUNC(elan_eu3a05_state::sound_end0));
	m_sound->sound_end_cb<1>().set(FUNC(elan_eu3a05_state::sound_end1));
	m_sound->sound_end_cb<2>().set(FUNC(elan_eu3a05_state::sound_end2));
	m_sound->sound_end_cb<3>().set(FUNC(elan_eu3a05_state::sound_end3));
	m_sound->sound_end_cb<4>().set(FUNC(elan_eu3a05_state::sound_end4));
	m_sound->sound_end_cb<5>().set(FUNC(elan_eu3a05_state::sound_end5));
	*/
}

void elan_eu3a05_state::elan_sudoku(machine_config& config)
{
	elan_eu3a05(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &elan_eu3a05_state::elan_sudoku_map);
	m_vid->set_is_sudoku();
	m_vid->set_use_spritepages();
	m_sys->set_alt_timer(); // for Carl Edwards'
}

void elan_eu3a05_state::elan_sudoku_pal(machine_config& config)
{
	elan_sudoku(config);
	m_sys->set_pal(); // TODO: also set PAL clocks
	m_screen->set_refresh_hz(50);
}

void elan_eu3a05_state::elan_pvmilfin(machine_config& config)
{
	elan_eu3a05(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &elan_eu3a05_state::elan_sudoku_map);
	m_vid->set_is_pvmilfin();
	m_sys->set_alt_timer();
	m_sys->set_pal(); // TODO: also set PAL clocks
	m_screen->set_refresh_hz(50);
}

void elan_eu3a05_state::airblsjs(machine_config& config)
{
	elan_eu3a05(config);
	m_screen->set_refresh_hz(50);
	m_sys->set_pal(); // TODO: also set PAL clocks
}


uint8_t elan_eu3a05_pvwwcas_state::pvwwc_portc_r()
{
	int pc = m_maincpu->pc();

	if ((pc!=0xEBAC) && (pc!= 0xEBB5) && (pc != 0xEBA3) && (pc != 0xEBE1))
		logerror("%s: pvwwc_portc_r\n", machine().describe_context().c_str());

	if (pc == 0xEBE1)
		logerror("%s: pvwwc_portc_r reading input bit\n\n", machine().describe_context());

	return m_prevport_c | 0x4;
}

void elan_eu3a05_pvwwcas_state::pvwwc_portc_w(uint8_t data)
{
	logerror("%s: pvwwc_portc_w %02x\n", machine().describe_context());


	if ((m_prevport_c & 0x01) != (data & 0x01))
	{
		if (data & 0x01)
			logerror("bit 0 going high\n");
		else
			logerror("bit 0 going low\n");
	}

	if ((m_prevport_c & 0x02) != (data & 0x02))
	{
		if (data & 0x02)
			logerror("bit 1 going high\n");
		else
			logerror("bit 1 going low\n");
	}

	m_prevport_c = data;
}

void elan_eu3a05_pvwwcas_state::pvwwcas(machine_config& config)
{
	elan_eu3a05(config);
	m_screen->set_refresh_hz(50);
	m_sys->set_pal(); // TODO: also set PAL clocks

	m_gpio->read_2_callback().set(FUNC(elan_eu3a05_pvwwcas_state::pvwwc_portc_r));
	m_gpio->write_2_callback().set(FUNC(elan_eu3a05_pvwwcas_state::pvwwc_portc_w));
}



ROM_START( rad_tetr )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "tetrisrom.bin", 0x000000, 0x100000, CRC(40538e08) SHA1(1aef9a2c678e39243eab8d910bb7f9f47bae0aee) )
	ROM_RELOAD(0x100000, 0x100000)
	ROM_RELOAD(0x200000, 0x100000)
	ROM_RELOAD(0x300000, 0x100000)
ROM_END

ROM_START( rad_sinv )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "spaceinvadersrom.bin", 0x000000, 0x100000, CRC(5ffb2c8f) SHA1(9bde42ec5c65d9584a802de7d7c8b842ebf8cbd8) )
	ROM_RELOAD(0x100000, 0x100000)
	ROM_RELOAD(0x200000, 0x100000)
	ROM_RELOAD(0x300000, 0x100000)
ROM_END

ROM_START( airblsjs )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "airblsjs.bin", 0x000000, 0x400000, BAD_DUMP CRC(d10a6a84) SHA1(fa65f06e7da229006ddaffb245eef2cc4f90a66d) ) // ROM probably ok, but needs verification pass
ROM_END

ROM_START( sudelan3 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sudoku.bin", 0x00000, 0x100000, CRC(c2596173) SHA1(cc74932648b577b735151014e8c04ed778e11704) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_RELOAD(0x200000,0x100000)
	ROM_RELOAD(0x300000,0x100000)
ROM_END

ROM_START( sudelan )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "klaussudoku.bin", 0x00000, 0x200000, CRC(afd2b06a) SHA1(21db956fb40b2e3d61fc2bac89000cf7f61fe99e) )
	ROM_RELOAD(0x200000,0x200000)
ROM_END

ROM_START( sudoku2p )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "1596.bin", 0x00000, 0x100000, CRC(fe8ce5bc) SHA1(8fb1463c6d349e07f6483da2b6cce10a4f25fcec) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_RELOAD(0x200000,0x100000)
	ROM_RELOAD(0x300000,0x100000)
ROM_END



ROM_START( carlecfg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "carledwardsracing.bin", 0x00000, 0x100000, CRC(920f633e) SHA1(8460b77b9635a2484edab1111f35bbda74eb68e4) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_RELOAD(0x200000,0x100000)
	ROM_RELOAD(0x300000,0x100000)
ROM_END

ROM_START( pvmil8 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "millionare_8bit.bin", 0x000000, 0x200000, CRC(8934a8d6) SHA1(24681e06d02f1567a57b84ec1c6f0a23a5f308ac) )
	ROM_RELOAD(0x200000,0x200000)
ROM_END


ROM_START( pvmilfin )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "fwwtbam.bin", 0x000000, 0x200000, CRC(2cfef9ab) SHA1(b64f55e36b59790a310ae33154774ac613b5d49f) )
	ROM_RELOAD(0x200000,0x200000)
ROM_END



ROM_START( buzztime )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "buzztimeunit.bin", 0x000000, 0x200000, CRC(8ba3569c) SHA1(3e704338a53daed63da90aba0db4f6adb5bccd21) )
ROM_END


ROM_START( pvwwcas )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "playvisiontaikeecasino.bin", 0x000000, 0x200000, CRC(b5e4a58d) SHA1(0a7809e91023258ecd55386b3e36b1541fb9e7f6) )
	ROM_RELOAD(0x200000, 0x200000)
ROM_END

void elan_eu3a05_state::init_sudelan3()
{
	// skip infinite loop (why is this needed? does it think we've soft shutdown?)
	uint8_t* ROM = memregion("maincpu")->base();
	ROM[0x0fcc] = 0xea;
	ROM[0x0fcd] = 0xea;
	ROM[0x0fce] = 0xea;
}

void elan_eu3a05_state::init_sudelan()
{
	// avoid jump to infinite loop (why is this needed? does it think we've soft shutdown?)
	uint8_t* ROM = memregion("maincpu")->base();
	ROM[0xd0f] = 0xea;
	ROM[0xd10] = 0xea;
	ROM[0xd11] = 0xea;
}

void elan_eu3a05_pvwwcas_state::init_pvwwcas()
{
	// avoid jump to infinite loop (why is this needed? does it think we've soft shutdown? or I/O failure?)
	uint8_t* ROM = memregion("maincpu")->base();
	ROM[0x3f8d92] = 0xea;
	ROM[0x3f8d93] = 0xea;
	ROM[0x3f8d94] = 0xea;
}

} // anonymous namespace


CONS( 2004, rad_sinv, 0, 0, elan_eu3a05, rad_sinv, elan_eu3a05_state, empty_init, "Radica (licensed from Taito)",                      "Space Invaders [Lunar Rescue, Colony 7, Qix, Phoenix] (Radica, Arcade Legends TV Game)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // "5 Taito games in 1"

CONS( 2004, rad_tetr, 0, 0, elan_eu3a05, rad_tetr, elan_eu3a05_state, empty_init, "Radica (licensed from Elorg / The Tetris Company)", "Tetris (Radica, Arcade Legends TV Game)", MACHINE_NOT_WORKING ) // "5 Tetris games in 1"

// ROM contains the string "Credit:XiAn Hummer Software Studio(CHINA) Tel:86-29-84270600 Email:HummerSoft@126.com"  PCB has datecode of "050423" (23rd April 2005)
CONS( 2005, airblsjs, 0, 0, airblsjs, airblsjs, elan_eu3a05_state, empty_init, "Advance Bright Ltd", "Air-Blaster Joystick (AB1500, PAL)", MACHINE_NOT_WORKING )

CONS( 2004, buzztime, 0, 0, elan_buzztime, sudoku, elan_eu3a05_buzztime_state, empty_init, "Cadaco", "Buzztime Home Trivia System", MACHINE_NOT_WORKING )

// Below are probably not EU3A05 but use similar modes (possibly EU3A13?)

CONS( 2006, sudelan3, 0,        0, elan_sudoku,      sudoku,   elan_eu3a05_state, init_sudelan3,  "All in 1 Products Ltd / Senario",  "Ultimate Sudoku TV Edition 3-in-1 (All in 1 / Senario)", MACHINE_NOT_WORKING )
// Senario also distributed this version in the US without the '3 in 1' text on the box, ROM has not been verified to match
CONS( 2005, sudelan, 0,         0, elan_sudoku_pal,  sudoku,   elan_eu3a05_state, init_sudelan,  "All in 1 Products Ltd / Play Vision",  "Carol Vorderman's Sudoku Plug & Play TV Game (All in 1 / Play Vision)", MACHINE_NOT_WORKING )

CONS( 2005, sudoku2p, 0,        0, elan_sudoku_pal,  sudoku2p, elan_eu3a05_state, empty_init,  "<unknown>",  "Sudoku TV Game (PAL, 2 players)", MACHINE_NOT_WORKING ) // a pair of yellow controllers with 'TV Sudoku Awesome Puzzles' on their label

CONS( 200?, carlecfg, 0,        0, elan_sudoku,  carlecfg,   elan_eu3a05_state, empty_init,  "Excalibur Electronics",  "Carl Edwards' Chase For Glory", MACHINE_NOT_WORKING )

// this is in very similar packaging to the 'pvmil' game in tvgames/spg2xx_playvision.cpp, and the casing is identical
// however this is from a year earlier, and there is a subtle difference in the otherwise identical text on the back of the box, mentioning that it uses an 8-bit processor, where the other box states 16-bit
CONS( 2005, pvmil8,   0,        0, elan_pvmilfin,  sudoku,   elan_eu3a05_state, empty_init,  "Play Vision", "Who Wants to Be a Millionaire? (Play Vision, Plug and Play, UK, 8-bit version)", MACHINE_NOT_WORKING )
// see https://millionaire.fandom.com/wiki/Haluatko_miljon%C3%A4%C3%A4riksi%3F_(Play_Vision_game)
CONS( 2005, pvmilfin, pvmil8,   0, elan_pvmilfin,  sudoku,   elan_eu3a05_state, empty_init,  "Play Vision", u8"Haluatko miljonääriksi? (Finland)", MACHINE_NOT_WORKING )

CONS( 2005, pvwwcas,  0,        0, pvwwcas,    sudoku,   elan_eu3a05_pvwwcas_state, init_pvwwcas, "Play Vision / Taikee / V-Tac", "Worldwide Casino Tour 12-in-1", MACHINE_NOT_WORKING )

