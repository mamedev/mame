// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
/******************************************************************************

    Short Description:

        Systems which run on the SPG243 SoC

        die markings show
        "SunPlus QL8041" ( also known as Sunplus SPG240 & PAC300 )

            All GameKeyReady units
                Disney Princess (GKR)
                Wheel of Fortune (GKR)
                JAKKS WWE (GKR)
                Fantastic 4 (GKR)
                Justice League (GKR)
                Dora the Explorer Nursery Rhyme (GKR)
                Dora the Explorer Play Park (GKR)
                Spiderman 5-in-1 (GKR)
                etc.

            (other non GKR JAKKS games)
            X-Men (Wolverine pad)
            Avatar: The Last Airbender
            Superman in Super Villain Showdown

            (other games)
            Mattel Classic Sports

        "SunPlus QL8041C" ( known as Sunplus SPG2??, seems to be compatible with above, so probably just a chip revision )

            Clickstart ( see clickstart.cpp instead)
            Wheel of Fortune 2nd Edition
            Spider-man - Villain Roundup
            Dream Life Superstar
            Designer's World
            Star Wars TV Touch

        "SunPlus QU7074-P69A"

            The Batman
            Star Wars (non-gamekey, which model? falcon? - check)
            Dream Life

        "SunPlus QL8167b" (is the scrambling built into the CPU, or external?)

            Lexibook Zeus IG900 20-in-1

        "SunPlus QL8139C"

            Radica Cricket
            V Smile Baby (Sweden) - see vsmileb.cpp

        ---

        Very likely the same

        "Sunplus QL8167" (these might have ROM scrambling if that is a 8167 feature)

            Disney Princess Magical Adventure
            Go Diego Go
            Shrek - Over the Hedge (this unit shows a 'GameKey Unlock More Games' on startup, but has no port, not even on the internal PCB)
            Marvel Heroes (Spider-man)
            Spiderman 3 (Movie - black)


        ---

        It is unknown if the following are close to this architecture or not (no dumps yet)

        "SunPlus QU7073-P69A"

            Mortal Kombat

        "Sunplus PU7799-P680?" (difficult to read)

            Mission Paintball

        ---

        These are definitely different but still unSP based

        "SunPlus PA7801" ( known as Sunplus SPG110? )
        - see spg110.cpp instead

        "GCM394" (this is clearly newer, has extra opcodes, different internal map etc. also scaling and higher resolutions based on Spongebob)
        - see sunplus_unsp20soc.cpp instead

    Status:

        Mostly working

    To-Do:

        Proper driver_device inheritance to untangle the mess of members

    Detailed list of bugs:

        All systems:
            Various inaccuracies in samples/envelopes.

        jak_wall, jak_sdoo:
            Game seems unhappy with NVRAM, clears contents on each boot.
        jak_disf:
            Shows corrupt logo on first boot with no valid nvram (possibly hardware does too - verify if possible to invalidate EEPROM on device)
        lexizeus:
            Some corrupt sound effects and a few corrupt ground tiles a few minutes in. (checksum is good, and a video recorded
             from one of these doesn't exhibit these problems, so either emulation issue or alt revision?)
        pvmil:
            Question order depends on SoC RNG, only reads when it wants a new value, so unless RNG runs on a timer question order ends up the same

        vii:
            When loading a cart from file manager, sometimes MAME will crash.
            The "MOTOR" option in the diagnostic menu does nothing when selected.
            The "SPEECH IC" option in the diagnostic menu does nothing when selected.
            On 'vii_vc1' & 'vii_vc2' cart, the left-right keys are transposed with the up-down keys.
            - This is not a bug per se, as the games are played with the controller physically rotated 90 degrees.

    Note:
        Cricket, Skateboarder, Skannerz and Football 2 list a 32-bit checksum at the start of ROM.
        This is the byte sum of the file, excluding the first 16 byte (where the checksum is stored)

        Test Modes:
        Justice League : press UP, DOWN, LEFT, BT3 on the JAKKS logo in that order, quickly, to get test menu
        WWE : press UP, BT1, BT2 together during startup logos

        Disney Friends, MS Pacman, WallE, Batman (and some other HotGen GameKeys) for test mode, hold UP,
        press A, press DOWN during startup

        Capcom test (same access as other Hotgen games) mode looks like this (tested on PAL unit, same ROM as dumped one)

        RAM OK     2800
                111111
                5432109876543210
        IOA    ............111.          (values go from . to 1 when inputs are moved, never 0 as in MAME!, core bug?)
                        GAMEKEY E0
        IOB0
        IOC    XXX.........X...
        SPRITES

        Care Bears : Hold analog stck up, rotate stick 360 degress back to up, press 'A' while still holding up

    TODO:
        Work out how to access the hidden TEST menus for all games (most JAKKS games should have one at least)

*******************************************************************************/

#include "emu.h"
#include "spg2xx.h"


/*************************
*    Common Helper    *
*************************/

void spg2xx_game_state::decrypt_ac_ff(uint16_t* ROM, int size)
{
	for (int i = 0; i < size / 2; i++)
	{
		ROM[i] = bitswap<16>(ROM[i], 15, 13, 14, 12,
									 7,  6,  5,  4,
									 11, 10, 9,  8,
									 3,  1,  2,  0);

		ROM[i] = ROM[i] ^ 0xfafa;
	}
}


/*************************
*    Machine Hardware    *
*************************/

void spg2xx_game_state::switch_bank(uint32_t bank)
{
	if (m_bank)
	{
		if (bank != m_current_bank)
		{
			m_current_bank = bank;
			m_bank->set_entry(bank);
			m_maincpu->invalidate_cache();
		}
	}
}

void spg2xx_game_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: porta_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');
}

void spg2xx_game_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: portb_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');
}

void spg2xx_game_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: portc_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');
}

void spg2xx_game_state::i2c_w(offs_t offset, uint8_t data)
{
	logerror("%s: i2c_w %05x %04x\n", machine().describe_context(), offset, data);
}

uint8_t spg2xx_game_state::i2c_r(offs_t offset)
{
	logerror("%s: i2c_r %04x\n", machine().describe_context(), offset);
	return 0x0000;
}

uint16_t spg2xx_game_state::base_porta_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_io_p1->read();
	logerror("%s: Port A Read: %04x (%04x)\n", machine().describe_context(), data, mem_mask);
	return data;
}

uint16_t spg2xx_game_state::base_portb_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_io_p2->read();
	logerror("%s: Port B Read: %04x (%04x)\n", machine().describe_context(), data, mem_mask);
	return data;
}

uint16_t spg2xx_game_state::base_portc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_io_p3->read();
	logerror("%s: Port C Read: %04x (%04x)\n", machine().describe_context(), data, mem_mask);
	return data;
}

uint16_t spg2xx_game_state::base_guny_r()
{
	uint16_t data = m_io_guny->read();
	logerror("%s: Gun Y Read: %04x\n", machine().describe_context(), data);
	return data;
}

uint16_t spg2xx_game_state::base_gunx_r()
{
	uint16_t data = m_io_gunx->read();
	logerror("%s: Gun X Read: %04x\n", machine().describe_context(), data);
	return data;
}



void spg2xx_game_state::mem_map_4m(address_map &map)
{
	map(0x000000, 0x3fffff).bankr("cartbank");
}

void spg2xx_game_state::mem_map_2m(address_map &map)
{
	map(0x000000, 0x1fffff).mirror(0x200000).bankr("cartbank");
}

void spg2xx_game_state::mem_map_1m(address_map &map)
{
	map(0x000000, 0x0fffff).mirror(0x300000).bankr("cartbank");
}

void spg2xx_game_gssytts_state::mem_map_upperbank(address_map &map)
{
	map(0x000000, 0x1fffff).bankr("cartbank");
	map(0x200000, 0x3fffff).bankr("upperbank");
}


void spg2xx_game_wfcentro_state::mem_map_wfcentro(address_map &map)
{
	map(0x000000, 0x37ffff).bankr("cartbank");
	map(0x380000, 0x3fffff).ram();
}


void spg2xx_game_lexiart_state::mem_map_lexiart(address_map &map)
{
	map(0x000000, 0x3fffff).bankr("cartbank");
	map(0x3f0000, 0x3f7fff).ram(); // 2 * 32Kb RAMs on PCB
}



static INPUT_PORTS_START( spg2xx ) // base structure for easy debugging / figuring out of inputs
	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "P1:0001" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0001, "0001" )
	PORT_DIPNAME( 0x0002, 0x0002, "P1:0002" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0002, "0002" )
	PORT_DIPNAME( 0x0004, 0x0004, "P1:0004" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0004, "0004" )
	PORT_DIPNAME( 0x0008, 0x0008, "P1:0008" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0008, "0008" )
	PORT_DIPNAME( 0x0010, 0x0010, "P1:0010" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0010, "0010" )
	PORT_DIPNAME( 0x0020, 0x0020, "P1:0020" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0020, "0020" )
	PORT_DIPNAME( 0x0040, 0x0040, "P1:0040" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0040, "0040" )
	PORT_DIPNAME( 0x0080, 0x0080, "P1:0080" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0080, "0080" )
	PORT_DIPNAME( 0x0100, 0x0100, "P1:0100" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0100, "0100" )
	PORT_DIPNAME( 0x0200, 0x0200, "P1:0200" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0200, "0200" )
	PORT_DIPNAME( 0x0400, 0x0400, "P1:0400" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0400, "0400" )
	PORT_DIPNAME( 0x0800, 0x0800, "P1:0800" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0800, "0800" )
	PORT_DIPNAME( 0x1000, 0x1000, "P1:1000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x1000, "1000" )
	PORT_DIPNAME( 0x2000, 0x2000, "P1:2000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x2000, "2000" )
	PORT_DIPNAME( 0x4000, 0x4000, "P1:4000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x4000, "4000" )
	PORT_DIPNAME( 0x8000, 0x8000, "P1:8000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x8000, "8000" )

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "P2:0001" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0001, "0001" )
	PORT_DIPNAME( 0x0002, 0x0002, "P2:0002" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0002, "0002" )
	PORT_DIPNAME( 0x0004, 0x0004, "P2:0004" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0004, "0004" )
	PORT_DIPNAME( 0x0008, 0x0008, "P2:0008" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0008, "0008" )
	PORT_DIPNAME( 0x0010, 0x0010, "P2:0010" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0010, "0010" )
	PORT_DIPNAME( 0x0020, 0x0020, "P2:0020" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0020, "0020" )
	PORT_DIPNAME( 0x0040, 0x0040, "P2:0040" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0040, "0040" )
	PORT_DIPNAME( 0x0080, 0x0080, "P2:0080" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0080, "0080" )
	PORT_DIPNAME( 0x0100, 0x0100, "P2:0100" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0100, "0100" )
	PORT_DIPNAME( 0x0200, 0x0200, "P2:0200" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0200, "0200" )
	PORT_DIPNAME( 0x0400, 0x0400, "P2:0400" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0400, "0400" )
	PORT_DIPNAME( 0x0800, 0x0800, "P2:0800" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0800, "0800" )
	PORT_DIPNAME( 0x1000, 0x1000, "P2:1000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x1000, "1000" )
	PORT_DIPNAME( 0x2000, 0x2000, "P2:2000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x2000, "2000" )
	PORT_DIPNAME( 0x4000, 0x4000, "P2:4000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x4000, "4000" )
	PORT_DIPNAME( 0x8000, 0x8000, "P2:8000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x8000, "8000" )

	PORT_START("P3")
	PORT_DIPNAME( 0x0001, 0x0001, "P3:0001" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0001, "0001" )
	PORT_DIPNAME( 0x0002, 0x0002, "P3:0002" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0002, "0002" )
	PORT_DIPNAME( 0x0004, 0x0004, "P3:0004" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0004, "0004" )
	PORT_DIPNAME( 0x0008, 0x0008, "P3:0008" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0008, "0008" )
	PORT_DIPNAME( 0x0010, 0x0010, "P3:0010" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0010, "0010" )
	PORT_DIPNAME( 0x0020, 0x0020, "P3:0020" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0020, "0020" )
	PORT_DIPNAME( 0x0040, 0x0040, "P3:0040" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0040, "0040" )
	PORT_DIPNAME( 0x0080, 0x0080, "P3:0080" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0080, "0080" )
	PORT_DIPNAME( 0x0100, 0x0100, "P3:0100" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0100, "0100" )
	PORT_DIPNAME( 0x0200, 0x0200, "P3:0200" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0200, "0200" )
	PORT_DIPNAME( 0x0400, 0x0400, "P3:0400" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0400, "0400" )
	PORT_DIPNAME( 0x0800, 0x0800, "P3:0800" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0800, "0800" )
	PORT_DIPNAME( 0x1000, 0x1000, "P3:1000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x1000, "1000" )
	PORT_DIPNAME( 0x2000, 0x2000, "P3:2000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x2000, "2000" )
	PORT_DIPNAME( 0x4000, 0x4000, "P3:4000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x4000, "4000" )
	PORT_DIPNAME( 0x8000, 0x8000, "P3:8000" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x8000, "8000" )
INPUT_PORTS_END

static INPUT_PORTS_START( epo_tetr ) // all inputs verified against hidden test mode
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 A")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 B")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 L")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 R")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 L")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 R")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

INPUT_PORTS_END

static INPUT_PORTS_START( dmbtjunc )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Red")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Green")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Blue")
	// there's a 2nd set of buttons for P2, where do they map?
	// battery state is likely in here too
INPUT_PORTS_END

static INPUT_PORTS_START( ban_krkk )
	// inputs shown in hidden text mode, although it refers to the physical placement of the each button on the mat rather than the colours / symbols
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Blue/Top-Left")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Yellow/Top-Right")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Red/Bottom-Left")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Green/Bottom-Right")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Select")

	PORT_MODIFY("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Blue/Top-Left")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Yellow/Top-Right")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Red/Bottom-Left")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Green/Bottom-Right")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Select")

	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) // Battery State
INPUT_PORTS_END


static INPUT_PORTS_START( drumsups )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start / Enter")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Drum pad 1: Blue")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Drum pad 2: Yellow")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Drum pad 3: Purple")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Drum pad 4: Red")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Drum pad 5: Green")
INPUT_PORTS_END

static INPUT_PORTS_START( lexiart )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_DIPNAME( 0x0100, 0x0000, "Battery State" )
	PORT_DIPSETTING(      0x0000, "Ok" )
	PORT_DIPSETTING(      0x0100, "Low" )
INPUT_PORTS_END

static INPUT_PORTS_START( itvphone ) // hold 8 and ENTER for Diagnostics mode
	PORT_START("P1") // note, the physical inputs are in 'phone' order, so 1 is top left, not bottom left like a PC Keypad
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 8") PORT_CODE(KEYCODE_8_PAD) // needed for DIAGNOSTICS mode
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad *") PORT_CODE(KEYCODE_ASTERISK)
	PORT_CONFNAME( 0x7000, 0x0000, "Non-TV Mode Game" )
	PORT_CONFSETTING(      0x1000, "Learning Game" )
	PORT_CONFSETTING(      0x2000, "Finding Game" )
	PORT_CONFSETTING(      0x4000, "Memory Game" )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Enter") // needed for DIAGNOSTICS mode
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bell")
	PORT_CONFNAME( 0x0040, 0x0000, "TV / Non-TV mode" ) // this is shown as a button in DIAGNOSTICS mode
	PORT_CONFSETTING(      0x0000, "TV" )
	PORT_CONFSETTING(      0x0040, "Non-TV" )
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset") // reset? back?
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( rad_skat )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Full Left")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Full Right")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Slight Left") // you have to use this for the menus (eg trick lists)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Slight Right")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Front")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Back")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	// there only seem to be 3 buttons on the pad part, so presumably all the above are the skateboard, and below are the pad?
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("M Button")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("X Button")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("O Button")
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED ) // read but unused?

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_CUSTOM ) // NTSC (1) / PAL (0) flag
INPUT_PORTS_END

static INPUT_PORTS_START( rad_skatp )
	PORT_INCLUDE(rad_skat)

	PORT_MODIFY("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) // NTSC (1) / PAL (0) flag
INPUT_PORTS_END




static INPUT_PORTS_START( tvsprt10 )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0180, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Start
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( decathln )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0180, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Start
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( mattelcs ) // there is a 'secret test mode' that previously got activated before inputs were mapped, might need unused inputs to active?
	PORT_START("P1")
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_UNUSED ) // must be IP_ACTIVE_LOW or you can't switch to Football properly?
	PORT_DIPNAME( 0x0018, 0x0000, "Game Select Slider" ) // technically not a dipswitch, a 3 position slider, but how best map it?
	PORT_DIPSETTING(      0x0008, "Baseball (Left)" )
	PORT_DIPSETTING(      0x0010, "Basketball (Middle)" )
	PORT_DIPSETTING(      0x0000, "Football (Right)" )
	// no 4th position possible
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_BIT( 0xffa0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_NAME("Joypad Up")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_NAME("Joypad Down")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_NAME("Joypad Left")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_NAME("Sound") // toggles between sound+music, sound only, and no sound
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_NAME("Hike / Pitch")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_NAME("Shoot / Run")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_NAME("Kick / Hit")
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* hold 'Console Down' while powering up to get the test menu, including input tests
   the ball (Wired) and bat (IR) are read some other way as they don't seem to appear in the ports. */
static INPUT_PORTS_START( rad_crik )
	PORT_START("P1")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Console Enter") // these are the controls on the base unit
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Console Down")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Console Left")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Console Right")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Console Up")
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( rad_fb2 ) // controls must be multiplexed somehow, as there's no room for P2 controls otherwise (unless P2 controls were never finished and it was only sold in a single mat version, Radica left useless P2 menu options in the mini Genesis consoles)
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) // 'left'
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // 'up'
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) // 'right'
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // acts a 'motion ball' in menu (this is an analog input from the ball tho? at least in rad_fb in xavix.cpp so this might just be a debug input here)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) // 'p2 right'
	// none of the remaining inputs seem to do anything
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_CUSTOM ) // NTSC (1) / PAL (0) flag
INPUT_PORTS_END

static INPUT_PORTS_START( abltenni )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1") // Down and both buttons on startup for Diagnostics Menu
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	// all remaining bits in this port will stop the demo mode, also having them high/low determines if you get 2P demos or Vs. CPU demos, not sure what the real state would be
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_DIPNAME( 0x0020, 0x0000, "Used" ) // doesn't boot otherwise
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("P3")
INPUT_PORTS_END


static INPUT_PORTS_START( ordentv )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )
INPUT_PORTS_END

static INPUT_PORTS_START( fordrace )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1") // hold button 1 on powerup for test
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Select / Start")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Gear Up")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Gear Down")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Pause")

	PORT_MODIFY("P2")

	PORT_MODIFY("P3")

	PORT_START("AD0") // 12-bit port, Accelerator
	PORT_BIT(0x0fff, 0x0000, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)

	PORT_START("AD1") // 12-bit port, Brake
	PORT_BIT(0x0fff, 0x0000, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)

	PORT_START("AD2") // 12-bit port, Wheel is split across 2 ports, value added together?
	PORT_BIT( 0x0fff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(spg2xx_game_fordrace_state::wheel2_r))

	PORT_START("AD3") // 12-bit port, Wheel (see above)
	PORT_BIT( 0x0fff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(spg2xx_game_fordrace_state::wheel_r))

	PORT_START("WHEEL_REAL")
	PORT_BIT(0x1fff, 0x0000, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x1fff) PORT_NAME("Wheel")
INPUT_PORTS_END

static INPUT_PORTS_START( totspies )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// unit also has a 'select' button next to 'OK' and while test mode shows it onscreen too, it doesn't get tested, so probably isn't connected to anything?
	PORT_MODIFY("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("OK")
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( doyousud )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Menu")
	// is the on/off button visible at 0020?
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Higher / Up")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Pencil")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Lower / Down")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Ok")

INPUT_PORTS_END


ioport_value spg2xx_game_fordrace_state::wheel_r()
{
	return ioport("WHEEL_REAL")->read() >> 1;
}

ioport_value spg2xx_game_fordrace_state::wheel2_r()
{
//  return 0x0800;
	uint16_t dat = ioport("WHEEL_REAL")->read();

	return ((dat >> 1) ^ 0xfff) + (dat & 1);
}

static INPUT_PORTS_START( senspeed )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Accelerate / Select")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Reverse / Confirm")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A (Autojacks)")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B (Belt Tires)")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C (Cutting Saw)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("D (Deflector)")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("E (Evening Eyes)")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("F (Frogman Mode)")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("G (Go Robot)")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Pause")
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) // eeprom bit, handled in read function
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ablkickb )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_DIPNAME( 0x0040, 0x0040, "Show Coleco Logo" ) // must be wired inside unit for Coleco distributed ones (US?)
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("P2")

	PORT_MODIFY("P3")
INPUT_PORTS_END

static INPUT_PORTS_START( lxspidaj )
	PORT_START("P1") // base controller has dpad, 2 regular buttons, 2 turbo buttons, start button, reset button.  IR connected JetSki pad has the same inputs but in handlebar form.
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) // is this a button or comms with the controller on the Jetski, check code (possibly just 'start' tho)
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( comil )
	PORT_START("EXTRA0")
	PORT_BIT( 0x1, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("50:50")
	PORT_BIT( 0x2, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Phone A Friend")
	PORT_BIT( 0x4, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Ask The Audience")
	PORT_BIT( 0x8, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Walk Away")

	PORT_START("EXTRA1")
	PORT_BIT( 0x1, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x2, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x4, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x8, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("D")

	PORT_START("EXTRA2")
	PORT_BIT( 0xf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA3")
	PORT_BIT( 0xf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA4")
	PORT_BIT( 0xf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA5")
	PORT_BIT( 0xf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA6")
	PORT_BIT( 0xf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA7")
	PORT_BIT( 0xf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) // multiplex select for Port B
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED ) // multiplexed inputs
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( guitarfv )
	PORT_START("P1")  // Button 1 + 2 and start for service mode
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Wheel / Whammy") // 'Wheel' in test mode
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
INPUT_PORTS_END

static INPUT_PORTS_START( guitarss )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B1: Blue / Up")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B2: Yellow / Down")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("B3: Purple")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("B4: Red")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("B5: Green")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Strum / Select")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED ) // unused, strum is single direction here
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Whammy")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Start / Select") // pause
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // unused?

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // unused?

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // unused?

INPUT_PORTS_END


static INPUT_PORTS_START( senwfit )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )  PORT_16WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_16WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )  PORT_16WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_16WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Select")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Start")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Weight Left")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Weight Right")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Up-Left")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Down-Left")
	PORT_BIT( 0x1c00, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Up-Right")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Down-Right")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // unused?

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // unused?
INPUT_PORTS_END

static INPUT_PORTS_START( jjstrip )
	PORT_START("P1") // active LOW or HIGH?
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // on pad but not used?
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( pballpup )
	PORT_START("GUNY")
	PORT_BIT(0x0ff, 0x80, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 256.0f / 240.0f, 0.0, 0) PORT_MINMAX(0x000, 0x0ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX")
	PORT_BIT(0x1ff, 0x100, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 512.0f / 320.0f, -0.03f, 0) PORT_MINMAX(0x000, 0x1ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("P1")
	PORT_BIT( 0x003f, IP_ACTIVE_HIGH, IPT_UNUSED ) // lower bits are seeprom
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) // pause
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // trigger
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // hide
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) // reload
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( swclone )
	PORT_START("GUNY")
	PORT_BIT(0x0ff, 0x80, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0f, 0.0, 0) PORT_MINMAX(0x000, 0x0ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX")
	PORT_BIT(0x1ff, 0x100, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 512.0f / 320.0f, -0.105f, 0) PORT_MINMAX(0x000, 0x1ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED ) // i2cmem here
	PORT_BIT( 0x003e, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) // pause
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // trigger
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // grenade
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) // reload (doesn't exist here?)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tmntmutm )
	PORT_START("GUNY")
	PORT_BIT(0x0ff, 0x80, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 256.0f / 240.0f, -0.032f, 0) PORT_MINMAX(0x000, 0x0ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX")
	PORT_BIT(0x1ff, 0x100, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 512.0f / 320.0f, -0.127f, 0) PORT_MINMAX(0x000, 0x1ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // trigger
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( dreamlss )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("B")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("C")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Test (Debug)") // not externally connected on unit
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( knd )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tmntbftc )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0180, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( hotwheels )
	// 2 pads, each pad has 4 directions and 1 button, and an internal solder pad to select type, but input reading code seems a bit more complex
	// the unit this was dumped from was a PAL, with P1 as 'Bling' and P2 as 'Tuner' so those are the defaults used
	PORT_START("P1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x0500, 0x0000, "Player 2 Controller Type" )
	PORT_CONFSETTING(      0x0000, "Tuner" )
	PORT_CONFSETTING(      0x0100, "Off-Road" )
	//PORT_CONFSETTING(      0x0400, "Tuner" )
	PORT_CONFSETTING(      0x0500, "Nothing" )
	PORT_BIT( 0x0a00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_CONFNAME( 0x0080, 0x0000, "Player 1 Controller Type" )
	PORT_CONFSETTING(      0x0000, "Bling" )
	PORT_CONFSETTING(      0x0080, "Rally" )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1EXTRA")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag  (ACTIVE_HIGH = NTSC, ACTIVE_LOW = PAL)
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )   PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )   PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )   PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )    PORT_PLAYER(2)
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3") // never read?
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


// TODO: work out how to access hidden test mode again
static INPUT_PORTS_START( doraphone )
	PORT_START("P1")
	PORT_CONFNAME( 0x0070, 0x0060, "On/Off Mode Slider" )
	PORT_CONFSETTING(      0x0030, "Play Alone (no video)" )
	PORT_CONFSETTING(      0x0060, "Play on TV" )

	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Lift Handset") // this could be done as a toggle, although note, handset being down is treated like a button being held
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) // this setting is US NTSC, ACTIVE_LOW gives US PAL (invalid?) no way to switch to non-US?
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM ) // must be 0x0200 or resets over and over
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P1_ROW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 'Repeat'") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("P1_ROW2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 0") PORT_CODE(KEYCODE_0_PAD)

	PORT_START("P1_ROW3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Phone Pad 'Help'") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("P1_ROW4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Show Answer") // Not 'answer phone'
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Enter")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_PLAYER(1) PORT_NAME("Reset")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(1) PORT_NAME("Hear Dora (non-TV mode only)")

	PORT_START("P1_ROW5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_PLAYER(1) PORT_NAME("Dora The Explorer Logo Button (non-TV mode only)")  // manual doesn't list this? speech says 'Dora the Explorer' in alone mode, presumably when you press the logo
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Exit")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME("Adventure Mode")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_PLAYER(1) PORT_NAME("Hear Boots (non-TV mode only)")

	PORT_START("P1_ROW6")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Amusement Park")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("Quick Play")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("Big Drum Parade")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("Banana Grove")

	PORT_START("P2")
	PORT_BIT( 0xff7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) // battery state

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( doraphonep )
	PORT_INCLUDE( doraphone )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_CUSTOM ) // PAL mode
INPUT_PORTS_END


static INPUT_PORTS_START( doraglobe )
	PORT_START("P1")
	PORT_CONFNAME( 0x0070, 0x0060, "On/Off Mode Slider" )
	PORT_CONFSETTING(      0x0030, "Play Alone (no video)" )
	PORT_CONFSETTING(      0x0060, "Play on TV" )

	// TODO: check if these region bits are the same
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) // this setting is US NTSC, ACTIVE_LOW gives US PAL (invalid?) no way to switch to non-US?
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM ) // must be 0x0200 or resets over and over
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_ROW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Continent Button: Asia")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Continent Button: North America")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(1) PORT_NAME("Mode Button: Learn and Explore")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_PLAYER(1) PORT_NAME("Repeat")

	PORT_START("P1_ROW2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Continent Button: Europe")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("Continent Button: South America")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Enter")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Back")

	PORT_START("P1_ROW3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("Continent Button: Africa")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("Continent Button: Antarctica")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_PLAYER(1) PORT_NAME("Mode Button: Adventure Play")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_PLAYER(1) PORT_NAME("Show Answer")

	PORT_START("P1_ROW4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME("Continent Button: Australia")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED ) // skips over some cutscenes and makes a 'button press' sound, but doesn't seem to be a real input on the device
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_PLAYER(1) PORT_NAME("Reset")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_PLAYER(1) PORT_NAME("Mode Button: Explore and Find")

	PORT_START("P1_ROW5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)

	PORT_START("P1_ROW6")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED ) // no response to these

	PORT_START("P2")
	PORT_BIT( 0xff7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) // battery state

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( virtbb )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // why does this also act as 'hit'? doesn't seem likely the motion control sends this?
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) // footmat
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON4 )

	PORT_MODIFY("P3")
INPUT_PORTS_END

static INPUT_PORTS_START( virtten )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_MODIFY("P2")

	PORT_MODIFY("P3")
INPUT_PORTS_END


static INPUT_PORTS_START( ddr33v )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // quits out of songs
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END


static const ioport_value handle_table[3] =
{
	0x00, 0x01, 0x03,
};

// the on/off slider has 3 positions; off, on (low sound), on (high sound) but this seems to be a hardware feature, not read by the code
static INPUT_PORTS_START( prail )
	PORT_INCLUDE( spg2xx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Doors")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Horn")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Lights")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Conductor")
	PORT_BIT( 0xc000, 0x0000, IPT_POSITIONAL_V ) PORT_POSITIONS(3) PORT_REMAP_TABLE(handle_table) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) // battery state

	PORT_MODIFY("P3")
	PORT_BIT( 0x0003, 0x0000, IPT_POSITIONAL_V ) PORT_POSITIONS(3) PORT_REMAP_TABLE(handle_table) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END

void spg2xx_game_state::machine_start()
{
	if (m_bank)
	{
		m_bank->configure_entries(0, (memregion("maincpu")->bytes() + 0x7fffff) / 0x800000, memregion("maincpu")->base(), 0x800000);
		m_bank->set_entry(0);
	}

	save_item(NAME(m_current_bank));
}

void spg2xx_game_state::machine_reset()
{
	m_current_bank = -1;
	switch_bank(0);
	m_maincpu->reset();
}

void spg2xx_game_state::spg2xx_base(machine_config &config)
{
	m_maincpu->porta_out().set(FUNC(spg2xx_game_state::porta_w));
	m_maincpu->portb_out().set(FUNC(spg2xx_game_state::portb_w));
	m_maincpu->portc_out().set(FUNC(spg2xx_game_state::portc_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("maincpu", FUNC(spg2xx_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(spg2xx_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
}

void spg2xx_game_state::non_spg_base(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_state::mem_map_4m);

	spg2xx_base(config);
}

void spg2xx_game_state::rad_skat(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->i2c_w().set(FUNC(spg2xx_game_state::i2c_w));
	m_maincpu->i2c_r().set(FUNC(spg2xx_game_state::i2c_r));
}

void spg2xx_game_state::spg2xx(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_state::base_portc_r));
}

void spg2xx_game_state::spg2xx_pal(machine_config& config)
{
	spg2xx(config);

	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);
}

void spg2xx_game_fordrace_state::fordrace(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_fordrace_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_fordrace_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_fordrace_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_fordrace_state::base_portc_r));

	// these do something in test mode, but in game the ADC interrupt is never generated?
	m_maincpu->adc_in<0>().set_ioport("AD0"); // pedals1
	m_maincpu->adc_in<1>().set_ioport("AD1"); // pedal2
	m_maincpu->adc_in<2>().set_ioport("AD2"); // steering
	m_maincpu->adc_in<3>().set_ioport("AD3"); // steering

}

uint16_t spg2xx_game_senspeed_state::portb_r()
{
	uint16_t ret = 0x0000;
	ret = m_i2cmem->read_sda() ? 1: 0;

	logerror("%s: spg2xx_game_senspeed_state::portb_r (%04x)\n", machine().describe_context(), ret);
	return ret;
}

void spg2xx_game_senspeed_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(mem_mask, 1))
		m_i2cmem->write_scl(BIT(data, 1));
	if (BIT(mem_mask, 0))
		m_i2cmem->write_sda(BIT(data, 0));
}

void spg2xx_game_senspeed_state::senspeed(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_senspeed_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set(FUNC(spg2xx_game_senspeed_state::portb_r));
	m_maincpu->portc_in().set_ioport("P3");

	m_maincpu->portb_out().set(FUNC(spg2xx_game_senspeed_state::portb_w));

	/*
	    ATMLH806
	    02B 1
	    A7J4565E
	*/
	I2C_24C01(config, "i2cmem", 0); // saves 0x80 bytes, but loading fails?
}


uint16_t spg2xx_game_comil_state::porta_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_porta_data;
	logerror("%s: Port A Read: %04x (%04x)\n", machine().describe_context(), data, mem_mask);
	return data;
}

void spg2xx_game_comil_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: Port A Write: %04x (%04x)\n", machine().describe_context(), data, mem_mask);
	m_porta_data = data;
}

uint16_t spg2xx_game_comil_state::portb_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_io_p2->read() & 0xfff0;

	if (!(m_porta_data & 0x0001)) data |= (m_extra_in[0]->read() & 0xf);
	if (!(m_porta_data & 0x0002)) data |= (m_extra_in[1]->read() & 0xf);
	if (!(m_porta_data & 0x0004)) data |= (m_extra_in[2]->read() & 0xf);
	if (!(m_porta_data & 0x0008)) data |= (m_extra_in[3]->read() & 0xf);
	if (!(m_porta_data & 0x0010)) data |= (m_extra_in[4]->read() & 0xf);
	if (!(m_porta_data & 0x0020)) data |= (m_extra_in[5]->read() & 0xf);
	if (!(m_porta_data & 0x0040)) data |= (m_extra_in[6]->read() & 0xf);
	if (!(m_porta_data & 0x0080)) data |= (m_extra_in[7]->read() & 0xf);

	logerror("%s: Port B Read: %04x (%04x)\n", machine().describe_context(), data, mem_mask);
	return data;
}

void spg2xx_game_comil_state::comil(machine_config &config)
{
	SPG28X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_comil_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_comil_state::porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_comil_state::portb_r));

	m_maincpu->porta_out().set(FUNC(spg2xx_game_comil_state::porta_w));
	//m_maincpu->portb_out().set(FUNC(spg2xx_game_comil_state::portb_w));
}

void spg2xx_game_state::guitarfv(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);

	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");
}


void spg2xx_game_state::tvsprt10(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_state::mem_map_2m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_state::base_portc_r));
}

void spg2xx_game_state::spg28x(machine_config &config)
{
	SPG28X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_state::base_portc_r));
}


uint16_t spg2xx_game_tmntmutm_state::guny_r()
{
	int frame = m_screen->frame_number() & 1; // game will not register shots if the co-ordinates are exactly the same as previous shot
	uint16_t data = m_io_guny->read() ^ frame;
	logerror("%s: Gun Y Read: %04x\n", machine().describe_context(), data);
	return data;
}

uint16_t spg2xx_game_tmntmutm_state::gunx_r()
{
	int frame = (m_screen->frame_number() >> 1) & 1;
	uint16_t data = m_io_gunx->read() ^ frame;
	logerror("%s: Gun X Read: %04x\n", machine().describe_context(), data);
	return data;
}



void spg2xx_game_tmntmutm_state::tmntmutm(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_tmntmutm_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_tmntmutm_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_tmntmutm_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_tmntmutm_state::base_portc_r));

	m_maincpu->guny_in().set(FUNC(spg2xx_game_tmntmutm_state::guny_r));
	m_maincpu->gunx_in().set(FUNC(spg2xx_game_tmntmutm_state::gunx_r));

	I2C_24C08(config, "i2cmem", 0);
}

uint16_t spg2xx_game_albkickb_state::portb_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = machine().rand();// TODO
	logerror("%s: Port B Read: %04x (%04x)\n", machine().describe_context(), data, mem_mask);
	return data;
}

void spg2xx_game_albkickb_state::ablkickb(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_albkickb_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_albkickb_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_albkickb_state::portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_albkickb_state::base_portc_r));
}


uint16_t spg2xx_game_pballpup_state::porta_r()
{
	uint16_t ret = m_io_p1->read() & 0xfff7;
//  logerror("%s: spg2xx_game_pballpup_state::porta_r\n", machine().describe_context());
	ret |= m_eeprom->do_read() ? 0x8 : 0x0;
	return ret;
}

void spg2xx_game_pballpup_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: spg2xx_game_pballpup_state::porta_w (%04x)\n", machine().describe_context(), data);
	m_eeprom->di_write(BIT(data, 2));
	m_eeprom->cs_write(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);

	// this can actually change bank from running code, because the code part in each bank is almost identical, just the data changes
	switch_bank((data & 0x1000) ? 1 : 0);
}


void spg2xx_game_pballpup_state::pballpup(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_pballpup_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_pballpup_state::porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_pballpup_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_pballpup_state::base_portc_r));

	m_maincpu->porta_out().set(FUNC(spg2xx_game_pballpup_state::porta_w));
	m_maincpu->portb_out().set(FUNC(spg2xx_game_pballpup_state::portb_w));
	m_maincpu->portc_out().set(FUNC(spg2xx_game_pballpup_state::portc_w));

	m_maincpu->guny_in().set(FUNC(spg2xx_game_pballpup_state::base_guny_r));
	m_maincpu->gunx_in().set(FUNC(spg2xx_game_pballpup_state::base_gunx_r));

	EEPROM_93C66_16BIT(config, m_eeprom); // type?
}

uint16_t spg2xx_game_swclone_state::porta_r()
{
	uint16_t ret = m_io_p1->read() & 0xfffe;
	ret |= m_i2cmem->read_sda() ? 0x1: 0x0;

	//logerror("%s: spg2xx_game_swclone_state::porta_r (%04x)\n", machine().describe_context(), ret);
	return ret;
}

void spg2xx_game_swclone_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//logerror("%s: spg2xx_game_swclone_state::porta_w (%04x & %04x)\n", machine().describe_context(), data, mem_mask);

	if (BIT(mem_mask, 1))
		m_i2cmem->write_scl(BIT(data, 1));
	if (BIT(mem_mask, 0))
		m_i2cmem->write_sda(BIT(data, 0));

	m_porta_data = data;
}


void spg2xx_game_swclone_state::swclone(machine_config &config)
{
	SPG2XX_128(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_swclone_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_swclone_state::porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_swclone_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_swclone_state::base_portc_r));

	m_maincpu->porta_out().set(FUNC(spg2xx_game_swclone_state::porta_w));
	m_maincpu->portb_out().set(FUNC(spg2xx_game_swclone_state::portb_w));
	m_maincpu->portc_out().set(FUNC(spg2xx_game_swclone_state::portc_w));

	m_maincpu->guny_in().set(FUNC(spg2xx_game_swclone_state::base_guny_r));
	m_maincpu->gunx_in().set(FUNC(spg2xx_game_swclone_state::base_gunx_r));

	I2C_24C08(config, "i2cmem", 0);
}



uint16_t spg2xx_game_dreamlss_state::porta_r()
{
	uint16_t ret = m_io_p1->read()&0xefff;
	ret |= m_porta_data & 0x1000; // needs to be able to read back current bank
	logerror("%s: spg2xx_game_dreamlss_state::porta_r\n", machine().describe_context());
	return ret;
}

void spg2xx_game_dreamlss_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: spg2xx_game_dreamlss_state::porta_w (%04x)\n", machine().describe_context(), data);

	m_porta_data = data;

	switch_bank((data & 0x1000) ? 1 : 0);
}


// TODO: how does the SEEPROM hook up? (will need hack removing in init_dreamlss )
uint16_t spg2xx_game_dreamlss_state::portb_r()
{
	uint16_t ret = m_portb_data & 0xfffe;
	ret |= m_i2cmem->read_sda() ? 0x1: 0x0;

	//logerror("%s: spg2xx_game_dreamlss_state::portb_r (%04x)\n", machine().describe_context(), ret);
	return ret;
}

void spg2xx_game_dreamlss_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//logerror("%s: spg2xx_game_dreamlss_state::portb_w (%04x & %04x)\n", machine().describe_context(), data, mem_mask);

	if (BIT(mem_mask, 1))
		m_i2cmem->write_scl(BIT(data, 1));
	if (BIT(mem_mask, 0))
		m_i2cmem->write_sda(BIT(data, 0));

	m_portb_data = data;
}


void spg2xx_game_dreamlss_state::dreamlss(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_dreamlss_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_dreamlss_state::porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_dreamlss_state::portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_dreamlss_state::base_portc_r));

	m_maincpu->porta_out().set(FUNC(spg2xx_game_dreamlss_state::porta_w));
	m_maincpu->portb_out().set(FUNC(spg2xx_game_dreamlss_state::portb_w));
	m_maincpu->portc_out().set(FUNC(spg2xx_game_dreamlss_state::portc_w));

	I2C_24C08(config, "i2cmem", 0);
}

void spg2xx_game_gssytts_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int bank = 0;

	logerror("%s: portc_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');

	if (mem_mask & 1)
		if (data & 1)
			bank |= 1;

	if (mem_mask & 2)
		if (data & 2)
			bank |= 2;

	m_upperbank->set_entry(bank);
	m_maincpu->invalidate_cache();
}

void spg2xx_game_gssytts_state::machine_start()
{
	m_upperbank->configure_entries(0, memregion("maincpu")->bytes()/0x400000, memregion("maincpu")->base(), 0x400000);
	m_upperbank->set_entry(1);

	spg2xx_game_state::machine_start();
}

void spg2xx_game_gssytts_state::machine_reset()
{
	m_upperbank->set_entry(1);

	spg2xx_game_state::machine_reset();
}


void spg2xx_game_gssytts_state::gssytts(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_gssytts_state::mem_map_upperbank);

	spg2xx_base(config);

//  m_maincpu->porta_out().set(FUNC(spg2xx_game_state::porta_w));
//  m_maincpu->portb_out().set(FUNC(spg2xx_game_state::portb_w));
//  m_maincpu->portc_out().set(FUNC(spg2xx_game_state::portc_w));

	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");
}


void spg2xx_game_wfcentro_state::wfcentro(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_wfcentro_state::mem_map_wfcentro);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_wfcentro_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_wfcentro_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_wfcentro_state::base_portc_r));
}

void spg2xx_game_lexiart_state::lexiart(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_lexiart_state::mem_map_lexiart);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_lexiart_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_lexiart_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_lexiart_state::base_portc_r));
}

void spg2xx_game_senwfit_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int bank = 0;

	logerror("%s: portc_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');

	if (mem_mask & 1)
		if (data & 1)
			bank |= 1;

	if (mem_mask & 2)
		if (data & 2)
			bank |= 2;

	if (mem_mask & 4)
		if (data & 4)
			bank |= 4;

	m_upperbank->set_entry(bank);
	m_maincpu->invalidate_cache();
}


void spg2xx_game_senwfit_state::init_senwfit()
{
	uint8_t *src = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	std::vector<u8> buffer(len);

	for (int i = 0; i < len; i++)
	{
		int newaddr = bitswap<25>(i, 24,23,22,20,9,19,18,21,17,16,15,14,13,12,11,10,8,7,6,5,4,3,2,1,0);
		buffer[i] = src[newaddr];
	}
	std::copy(buffer.begin(), buffer.end(), &src[0]);

}

void spg2xx_game_state::rad_skatp(machine_config &config)
{
	rad_skat(config);
	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);
//  m_screen->set_size(320, 312);
}


void spg2xx_game_state::rad_crik(machine_config &config)
{
	SPG28X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->i2c_w().set(FUNC(spg2xx_game_state::i2c_w));
	m_maincpu->i2c_r().set(FUNC(spg2xx_game_state::i2c_r));
}

uint16_t spg2xx_game_ordentv_state::ordentv_portc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_io_p3->read() ^ (machine().rand() & 1);
	logerror("%s: Port C Read: %04x (%04x)\n", machine().describe_context(), data, mem_mask);
	return data;
}

void spg2xx_game_ordentv_state::ordentv(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_ordentv_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_ordentv_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_ordentv_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_ordentv_state::ordentv_portc_r));
}


void spg2xx_game_hotwheels_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: porta_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');

	m_porta_dat_hot = data;
}

uint16_t spg2xx_game_hotwheels_state::hotwheels_porta_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data;
	if (m_porta_dat_hot)
		data = m_io_p1->read();
	else
		data = m_io_p1_extra->read();

	logerror("%s: Port A Read: %04x (%04x)\n", machine().describe_context(), data, mem_mask);
	return data;
}

void spg2xx_game_hotwheels_state::hotwheels(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_hotwheels_state::mem_map_2m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_hotwheels_state::hotwheels_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_hotwheels_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_hotwheels_state::base_portc_r));

	m_maincpu->porta_out().set(FUNC(spg2xx_game_hotwheels_state::porta_w));

	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);
}

uint16_t spg2xx_game_doraphone_state::porta_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t matrix = 0x000f;
	for (int b = 1; 6 >= b; ++b)
	{
		if (!BIT(m_portb_data, b))
			matrix &= m_io_p1_rows[b - 1]->read();
	}

	return matrix | (m_io_p1->read() & 0xfff0);
}

void spg2xx_game_doraphone_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_portb_data = data;
}

void spg2xx_game_doraphone_state::doraphone(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_doraphone_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_doraphone_state::porta_r));
	m_maincpu->portb_out().set(FUNC(spg2xx_game_doraphone_state::portb_w));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_doraphone_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_doraphone_state::base_portc_r));
}

void spg2xx_game_prail_state::prail_portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask & 0x0f)
	{
		uint8_t const bank = bitswap<4>(data, 3, 2, 0, 1);
		switch_bank(bank);
	}

	portb_w(offset, data & ~0x000f, mem_mask & ~0x000f);
}

void spg2xx_game_prail_state::prail(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen); // SPG243
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_prail_state::mem_map_4m);

	spg2xx_base(config);
	m_maincpu->porta_in().set(FUNC(spg2xx_game_prail_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(spg2xx_game_prail_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(spg2xx_game_prail_state::base_portc_r));

	m_maincpu->portb_out().set(FUNC(spg2xx_game_prail_state::prail_portb_w));

	// TODO: this is not currently hooked up, it's used to store the unlock states for the gallery
	I2C_24C02(config, "i2cmem", 0); // ATMLH13402C (24C02 compatible)
}


void spg2xx_game_doraphone_state::doraphonep(machine_config &config)
{
	doraphone(config);

	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);
}


void epo_tetr_game_state::machine_start()
{
	spg2xx_game_state::machine_start();

	save_item(NAME(m_old_portb_data));
	save_item(NAME(m_old_portb_extra_latch));
}

void epo_tetr_game_state::machine_reset()
{
	spg2xx_game_state::machine_reset();

	m_old_portb_data = 0;
	m_old_portb_extra_latch = 0;
}

uint16_t epo_tetr_game_state::epo_tetr_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	int eeprom = m_i2cmem->read_sda();
	data |= (eeprom << 1);

	int p2 = m_old_portb_extra_latch & 1;
	data |= (p2 << 6);

	if (!machine().side_effects_disabled())
		logerror("%s: epo_tetr_r: %04x (%04x)\n", machine().describe_context(), data, mem_mask);

	return data;
}

void epo_tetr_game_state::epo_tetr_portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	portb_w(offset, data, mem_mask);

	if (BIT(mem_mask, 0))
		m_i2cmem->write_scl(BIT(data, 0));
	if (BIT(mem_mask, 1))
		m_i2cmem->write_sda(BIT(data, 1));

	if (BIT(mem_mask, 4))
	{
		if ((BIT(data, 4)) && (!(BIT(m_old_portb_data, 4))))
		{
			m_old_portb_extra_latch = m_ioextra->read();
		}
	}

	if (BIT(mem_mask, 5))
	{
		if ((BIT(data, 5)) && (!(BIT(m_old_portb_data, 5))))
		{
			m_old_portb_extra_latch >>= 1;
		}
	}

	m_old_portb_data = data;
}


void epo_tetr_game_state::epo_tetr(machine_config& config)
{
	spg2xx(config);

	I2C_24C02(config, "i2cmem", 0); // S24CS02A

	m_maincpu->portb_in().set(FUNC(epo_tetr_game_state::epo_tetr_r));
	m_maincpu->portb_out().set(FUNC(epo_tetr_game_state::epo_tetr_portb_w));
}



void spg2xx_game_ddr33v_state::init_ddr33v()
{
	// what is this checking? timer? battery state? protection? it goes to a blank screen after the boot logo otherwise
	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();
	rom[0x208055] = 0x4440;
}

ROM_START( rad_skat )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "skateboarder.bin", 0x000000, 0x400000, CRC(08b9ab91) SHA1(6665edc4740804956136c68065890925a144626b) )
ROM_END

ROM_START( rad_skatp ) // rom was dumped from the NTSC version, but region comes from an io port, so ROM is probably the same
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "skateboarder.bin", 0x000000, 0x400000, CRC(08b9ab91) SHA1(6665edc4740804956136c68065890925a144626b) )
ROM_END

ROM_START( rad_fb2 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "football2.bin", 0x000000, 0x400000, CRC(96b4f0d2) SHA1(e91f2ac679fb0c026ffe216eb4ab58802f361a17) )
ROM_END



ROM_START( rad_crik ) // only released in EU?
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "cricket.bin", 0x000000, 0x200000, CRC(6fa0aaa9) SHA1(210d2d4f542181f59127ce2f516d0408dc6de7a8) )
ROM_END

ROM_START( mattelcs )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mattelclassicsports.bin", 0x000000, 0x100000, CRC(e633e7ad) SHA1(bf3e325a930cf645a7e32195939f3c79c6d35dac) )
ROM_END

ROM_START( abltenni )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ablpnpwirelesstennis.bin", 0x000000, 0x400000, CRC(66bd8ef1) SHA1(a83640d5d9e84e10d29a065a61e0d7bbec16c6e4) )
ROM_END

ROM_START( totspies )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "w321tg.u2", 0x000000, 0x400000, CRC(76152ad7) SHA1(b37ea950670eb927f3f0ab5e38d0e2a5f3ca7904) )
ROM_END

ROM_START( ablkickb )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ablkickboxing.bin", 0x000000, 0x800000,  CRC(61394c45) SHA1(291d28a39edcb32a8f5d776a5e5c05e6fd0abece) )
ROM_END

ROM_START( lxspidaj )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mx29lv320ct.u2", 0x000000, 0x400000, CRC(e7e03c62) SHA1(ab13452f0436efb767f01dff54dd48a528538e3f) )
ROM_END

ROM_START( lxairjet )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "w321tg.u5", 0x000000, 0x400000, CRC(105b226f) SHA1(2622678a8586ee6edae17715657037a5419e3321) )
ROM_END


ROM_START( fordrace )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "fordracing_29lv320ct_00c222a7.bin", 0x000000, 0x400000, CRC(998cad17) SHA1(98a65e9e0ec17e3366e0ac6ddc2d852a7efb360e) )
ROM_END


ROM_START( comil )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ukmillionaire.bin", 0x000000, 0x400000, CRC(b7e8e126) SHA1(fc76dba672eb5c4c115e16d8ea4a45a6e859f87c) )
ROM_END

ROM_START( tvsprt10 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tvsports10in1.bin", 0x000000, 0x400000, CRC(98b79889) SHA1(b0ba534d59b794bb38c071c70ab5bcf711364e06) )
ROM_END

ROM_START( decathln )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "decathlon.bin", 0x000000, 0x400000, CRC(63c8e6b6) SHA1(6a25b68b45336e04a2bfd75b43a494349024d714) )
ROM_END

ROM_START( decathlna )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "abldecathlon2.bin", 0x000000, 0x400000, CRC(594ed954) SHA1(6ddd9df8f645ac8e93ee37337ca9fb5f7f942827) )
ROM_END

ROM_START( guitarfv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mx26l64.bin", 0x000000, 0x800000, CRC(eaadd2c2) SHA1(0c3fe004dbaa52a335c6ddcecb9e9f5582d7ef35) )
ROM_END

ROM_START( guitarss )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "guitar_superstar_stratocaster.bin", 0x000000, 0x800000, CRC(63950016) SHA1(28b9613571f47c49995aa35c4d4a2d6f68389813) )
ROM_END

ROM_START( guitarssa )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "guitar_superstar_flying_v.bin", 0x000000, 0x800000, CRC(af0c837c) SHA1(f04c9a4292f811d92311d19fb35dcee3f1649a14) )
ROM_END

ROM_START( drumsups )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "drumsuperstar.bin", 0x000000, 0x800000, CRC(f3d5fd6d) SHA1(4d0c9ba7531b3df68bd9c020e46d07445301adf9) )
ROM_END


ROM_START( tmntbftc )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tmntbftc.bin", 0x000000, 0x400000, CRC(f923da5b) SHA1(79b290b75d06dabd0f579800edc4453b044c8fd4) )
ROM_END

ROM_START( knd )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "knd_sst39vf3201_00bf235b.bin", 0x000000, 0x400000, CRC(3b82479d) SHA1(2a4ddd5c6af2376e4725aeb44e79b0f9c45ca8c1) )
ROM_END

ROM_START( gssytts )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "guitarssytts.bin", 0x000000, 0x800000, CRC(ec3de9e1) SHA1(690efe2676c664c2be52cda00d6dcb9d60a26e9a) ) // no data
	ROM_CONTINUE(0x000000, 0x800000) // 1st 8mb
	ROM_CONTINUE(0x800000, 0x800000) // no data
	ROM_CONTINUE(0x800000, 0x800000) // 2nd 8mb
ROM_END

ROM_START( senwfit )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wirelessfit.bin", 0x000000, 0x2000000, CRC(bfdc9c56) SHA1(dd0d4262720fcc3fab5b66d39df9be3419b07178) )
ROM_END


ROM_START( jjstrip )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "strippoker.bin", 0x000000, 0x200000, CRC(7a70e6c8) SHA1(3d5da4774b00977939f309f3e71473dde9b70435) )
ROM_END

ROM_START( tmntmutm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tmntmutantsandmonsters_sst39vf3201_00bf235b.bin", 0x000000, 0x400000, CRC(93ab5ff7) SHA1(e78a5d380663d351ad9be5087ec8434d9be16ba7) )
ROM_END

ROM_START( pballpup )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "paintballpoweredup.bin", 0x000000, 0x1000000, CRC(57dbdfd1) SHA1(d98cb7321cc7af092f6f4f83e85fabbdbc1bbd95) )

	ROM_REGION16_BE( 0x200, "eeprom", ROMREGION_ERASE00 )
	// ensure eeprom defaults to 00 or there are unwanted invalid entries already saved
ROM_END

ROM_START( dreamlss )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "dreamlifesuperstar.bin", 0x000000, 0x1000000, CRC(aefad2c3) SHA1(cf962082f09e27d7d24cfc722ae978d9aa735a57) )
ROM_END

ROM_START( swclone )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "swclone.bin", 0x000000, 0x800000, CRC(2c983509) SHA1(6138f21fe0b82a7121c4639b6833d4014d5aeb74) )

	ROM_REGION( 0x400, "i2cmem", ROMREGION_ERASE00 )
	// ensure eeprom defaults to 00 or there are unwanted invalid entries already saved
ROM_END

ROM_START( vtechtvssp )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "vtechtvstation_sp.bin", 0x000000, 0x800000, CRC(4a2e91eb) SHA1(1ff9cc0360b670cc0ad7efa9de0edd2c68d4d8e3) )
ROM_END

ROM_START( vtechtvsgr )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "vtechtvstation_gr.bin", 0x000000, 0x800000, CRC(879f1b12) SHA1(c14d52bead2c190130ce88cbdd4f5e93145f13f9) )
ROM_END

ROM_START( itvphone )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "inttvphone.bin", 0x000000, 0x200000, CRC(2ecbb0ad) SHA1(2b6babaaf1582e6b1de944258eba87ddf30406c5) )
ROM_END

ROM_START( jouet )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jouet.bin", 0x000000, 0x400000, CRC(da46097e) SHA1(f760f4d126a8291b7dacdea7a70691b25ad8b989) )
ROM_END

ROM_START( doraphon )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "doraphone.bin", 0x000000, 0x800000, CRC(a79c154b) SHA1(f5b9bf63ea52d058252ab6702508b519fbdee0cc) )
ROM_END

ROM_START( doraphonf )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "doraphone_fr.u4", 0x000000, 0x800000, CRC(216632a1) SHA1(b2bd81656a261e09814792f52428eead2ea7ce1f) )
ROM_END

ROM_START( doraglob )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "doraglobe.bin", 0x000000, 0x800000, CRC(6f454c50) SHA1(201e2de3d90abe017a8dc141613cbf6383423d13) )
ROM_END

ROM_START( doraglobf )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "doraglobefrance.bin", 0x000000, 0x800000, CRC(7124edc1) SHA1(b144fc1f13a28299ef14f1d01f7acd2677e4ebb9) )
ROM_END

ROM_START( doraglobg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "doraglobegerman.bin", 0x000000, 0x800000, CRC(538aa197) SHA1(e97e0641df04074a0e45d02cecb43fbec91a4ce6) )
ROM_END

ROM_START( senspeed )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "speedracer.bin", 0x000000, 0x800000, CRC(4efbcd39) SHA1(2edffbaa9ea309ad308fa60f32d8b7a98ee313c7) )
ROM_END

ROM_START( ordentv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "taikeeordenadortv.bin", 0x000000, 0x800000, CRC(ba15895a) SHA1(0a18076cbc3264c91473b8518dfb10d679321b47) )
ROM_END

ROM_START( wfcentro )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "winfuncentro.bin", 0x000000, 0x800000, CRC(fd6ad052) SHA1(78af844729bf4843dc70531349e38a8c25caf748) )
ROM_END

ROM_START( wfart )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "artstudio.bin", 0x000000, 0x800000, CRC(f5fd657e) SHA1(0005826a5b22a17cafffaf7328092c8d84217398) )
ROM_END

ROM_START( lexiart )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "lexibookartstudio.u3", 0x000000, 0x800000, CRC(fc417abb) SHA1(c0a18a2cf11c47086722f0ec88410614fed7c6f7) )
ROM_END

ROM_START( tiktokmm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "webcamthingy.bin", 0x000000, 0x800000, CRC(54c0d4a9) SHA1(709ee607ca447baa6f7e686268df1998372fe617) )
ROM_END

ROM_START( jeuint )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jeuinteractiftv.bin", 0x000000, 0x800000, CRC(27103086) SHA1(d1313f416ae8ec85e523fefd523d6f4b7970eaf3) )
ROM_END

ROM_START( hotwhl2p )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "hotwheels.bin", 0x000000, 0x400000, CRC(f3520b74) SHA1(02a53558d68cf3640a9ab09514cd6cebff8b30af) )
ROM_END

ROM_START( doyousud )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "doyousudoku.bin", 0x000000, 0x100000, CRC(83cafebb) SHA1(a84c7191bc6b0d321415af0b7d2dd69e52c134a1) )

	ROM_REGION( 0x800, "eeprom", ROMREGION_ERASE00 ) // probably just used for saving puzzle progress
	ROM_LOAD( "at24c16a.u3", 0x000, 0x800, CRC(414ea94d) SHA1(8565a66fd0228104c64a169cdb20715e7b23cfaf) )
ROM_END

ROM_START( virtbb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "virtualbb.bin", 0x000000, 0x400000, CRC(7cb7a69f) SHA1(eae0c516c1ff89a369662d09321feafc8a8054b0) )
ROM_END

ROM_START( virtten )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "virttennis.bin", 0x000000, 0x400000, CRC(e665bea9) SHA1(8c2c9f879c929e224cd885165ed60aed8baeb19d) )
ROM_END

ROM_START( ddr33v )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ddr33v.bin", 0x000000, 0x800000, CRC(56c7015c) SHA1(a1faef2ab6eb191dea1497f8cfd4ccbd8c504e6d) )
ROM_END

ROM_START( anpantv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "anpanman_tv.bin", 0x000000, 0x800000, CRC(5e32dc1a) SHA1(bae260ffc56f5315cdafd5bc40966ec6d31e267f) )
ROM_END

ROM_START( dmbtjunc )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "battlejunction.u3", 0x000000, 0x800000, CRC(31471c53) SHA1(94fdd8c4c67914054e304a55042c10710af2e596) )
ROM_END

ROM_START( ban_krkk )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "quiz.bin", 0x000000, 0x400000, CRC(6f51180a) SHA1(38017ecaae4eead38482aeb04c90b5a5eeebd6ca) )
ROM_END

ROM_START( epo_tetr )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mx29lv320mb.bin", 0x000000, 0x400000, CRC(b4ad30e0) SHA1(83e30e199854c647f9a197562d1bf1f3bc847fff) )
ROM_END


ROM_START( prail )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "traingame.u1", 0x000000, 0x8000000, CRC(5c96d526) SHA1(cda0280b320762bda7a7358ec7ce29690aa815fb) )
ROM_END


void spg2xx_game_state::init_crc()
{
	// several games have a byte sum checksum listed at the start of ROM, this little helper function logs what it should match.
	const int length = memregion("maincpu")->bytes();
	const uint8_t* rom = memregion("maincpu")->base();

	uint32_t checksum = 0x00000000;
	// the first 0x10 bytes are where the "chksum:xxxxxxxx " string is listed, so skip over them
	for (int i = 0x10; i < length; i++)
	{
		checksum += rom[i];
	}

	logerror("Calculated Byte Sum of bytes from 0x10 to 0x%08x is %08x)\n", length - 1, checksum);
}


void spg2xx_game_state::init_tvsprt10()
{
	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();
	// hack 'MASK' check (some kind of EEPROM maybe?)
	// this hack means the ROM CRC fails, but without it the CRC is OK, so dump is good.
	/*
	port b 0010 = chip select? / always set when accessing?
	       0008 = write enable (set when writing, not when reading)
	       0004 = chip select? / always set when accessing?
	       0002 = clock? (toggles)
	       0001 = data in / out
	*/
	if (rom[0x18c55d] == 0x9240) rom[0x18c55d] ^= 0x0001; // tvsprt10
	if (rom[0x179911] == 0x9240) rom[0x179911] ^= 0x0001; // decathln
	if (rom[0x16fa67] == 0x9240) rom[0x16fa67] ^= 0x0001; // decathlna
}

void spg2xx_game_swclone_state::init_swclone()
{
	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();
	rom[0x2649d1] = 0x0000; // don't write 0x1234 to the start of the RAM that is copied to spriteram on startup (this is an explicit write, probably actually a failure condition for something?)
}

void spg2xx_game_albkickb_state::init_ablkickb()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	decrypt_ac_ff(ROM, size);
}

void spg2xx_game_ordentv_state::init_ordentv()
{
	// the game will die by jumping to an infinite loop if this check fails, what is it checking?
	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();
	rom[0x4fef8] = 0xee07;
}

void spg2xx_game_ordentv_state::init_jeuint()
{
	// the game will die by jumping to an infinite loop if this check fails, what is it checking?
	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();
	rom[0x53376] = 0xee07;
}

void spg2xx_game_state::init_itvphone()
{
	// the game will die by jumping to an infinite loop if this check fails, what is it checking?
	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();
	rom[0xf152] = 0xee08;
}

// year, name, parent, compat, machine, input, class, init, company, fullname, flags

// Radica TV games
CONS( 2006, rad_skat,   0,        0, rad_skat,  rad_skat,  spg2xx_game_state,          init_crc,      "Radica",                                                 "Play TV Skateboarder (NTSC)",                                           MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2006, rad_skatp,  rad_skat, 0, rad_skatp, rad_skatp, spg2xx_game_state,          init_crc,      "Radica",                                                 "Connectv Skateboarder (PAL)",                                           MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2006, rad_crik,   0,        0, rad_crik,  rad_crik,  spg2xx_game_state,          init_crc,      "Radica",                                                 "Connectv Cricket (PAL)",                                                MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // Version 3.00 20/03/06 is listed in INTERNAL TEST

CONS( 2007, rad_fb2,    0,        0, rad_skat,  rad_fb2,   spg2xx_game_state,          init_crc,      "Radica",                                                 "Play TV Football 2",                                                    MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // offers a 2 player option in menus, but seems to have only been programmed for, and released as, a single player unit, P2 controls appear unfinished.

// ABL TV Games
CONS( 2006, abltenni,   0,        0, spg2xx,    abltenni,  spg2xx_game_state,          empty_init,    "Advance Bright Ltd / V-Tac Technology Co Ltd.",          "Wireless Tennis (WT2000, ABL TV Game)",                                 MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2006, ablkickb,   0,        0, ablkickb,  ablkickb,  spg2xx_game_albkickb_state, init_ablkickb, "Advance Bright Ltd / Coleco / V-Tac Technology Co Ltd.", "Kick Boxing (BJ8888, ABL TV Game)",                                     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // 4 motion sensors, one for each limb

CONS( 2007, lxspidaj,   0,        0, spg2xx_pal,lxspidaj,  spg2xx_game_albkickb_state, init_ablkickb, "Lexibook",                                               "Spider-Man Super TV Air Jet (Lexibook Junior, JG6000SP)",               MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2007, lxairjet,   0,        0, spg2xx_pal,lxspidaj,  spg2xx_game_albkickb_state, init_ablkickb, "Lexibook",                                               "Super TV Air Jet 6-in-1 (Lexibook Junior)",                             MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2006, totspies,   0,        0, spg2xx_pal,totspies,  spg2xx_game_state,          empty_init,    "Senario / Marathon - Mystery Animation Inc.",            "Totally Spies! (France)",                                               MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2006, fordrace,   0,        0, fordrace,  fordrace,  spg2xx_game_fordrace_state, empty_init,    "Excalibur Electronics",                                  "Ford Racing",                                                           MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2008, comil,      0,        0, comil,     comil,     spg2xx_game_comil_state,    empty_init,    "Character Options",                                      "Who Wants to Be a Millionaire? (Character Options, Plug and Play, UK)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Same as Excalibur Decathlon? Not identical to the ABL game below, but built on the same engine
CONS( 2006, tvsprt10,   0,        0, tvsprt10,  tvsprt10,  spg2xx_game_state,          init_tvsprt10, "Simba / V-Tac Technology Co Ltd.",                       "TV Sports 10-in-1 / Decathlon Athletic Sport Games",                    MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, decathln,   0,        0, tvsprt10,  decathln,  spg2xx_game_state,          init_tvsprt10, "Advance Bright Ltd / V-Tac Technology Co Ltd.",          "Decathlon (set 1)",                                                     MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // unit found in Spain
CONS( 200?, decathlna,  decathln, 0, tvsprt10,  decathln,  spg2xx_game_state,          init_tvsprt10, "Advance Bright Ltd / V-Tac Technology Co Ltd.",          "Decathlon (set 2, SM570, ABL TV Game)",                                 MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // unit found in UK

CONS( 2007, guitarfv,   0,        0, guitarfv,  guitarfv,  spg2xx_game_state,          empty_init,    "Advance Bright Ltd",                                     "Guitar Fever (2007.07.03 Ver 2.7)",                                     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// The box for these has 'YOU take the stage' text, but unlike the sequel, it is not part of the ingame title screen, this sometimes causes confusion
CONS( 200?, guitarss,   0,        0, spg28x,    guitarss,  spg2xx_game_state,          empty_init,    "Senario",                                                "Guitar Super Star ('Fender Stratocaster' style)",                       MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, guitarssa,  guitarss, 0, spg28x,    guitarss,  spg2xx_game_state,          empty_init,    "Senario",                                                "Guitar Super Star (red 'Gibson Flying V' style)",                       MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// The sequel has 'You Take The Stage' on both the box and title screen
CONS( 2009, gssytts,    0,        0, gssytts,   guitarss,  spg2xx_game_gssytts_state,  empty_init,    "Senario",                                                "Guitar Super Star: You Take The Stage",                                 MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2007, drumsups,   0,        0, spg28x,    drumsups,  spg2xx_game_state,          empty_init,    "Senario",                                                "Drum Super Star",                                                       MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2009, senwfit,    0,        0, gssytts,   senwfit,   spg2xx_game_senwfit_state,  init_senwfit,  "Senario",                                                "Wireless Fitness / Dance Fit (Senario)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// VTech "TV Station" / "TV Learning Station" / "Nitro Vision"
CONS( 2006, vtechtvssp, 0,        0, spg2xx,    spg2xx,    spg2xx_game_state,          empty_init,    "VTech",                                                  "TV Station (VTech, Spain)",                                             MACHINE_NOT_WORKING )
CONS( 2006, vtechtvsgr, 0,        0, spg2xx,    spg2xx,    spg2xx_game_state,          empty_init,    "VTech",                                                  "TV Learning Station (VTech, Germany)",                                  MACHINE_NOT_WORKING )

CONS( 2007, itvphone,   0,        0, spg2xx_pal, itvphone, spg2xx_game_state,          init_itvphone, "Taikee / Oregon Scientific / V-Tac Technology Co Ltd.",  u8"Teléfono interactivo de TV (Spain)",                                  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// "Boots's" is used on the title screen and in the manual, even if "Boots'" is usually used outside of this game.
CONS( 2006, doraphon,   0,        0, doraphone, doraphone, spg2xx_game_doraphone_state,empty_init,    "VTech",                                                  "Dora the Explorer - Dora TV Explorer Phone / Boots's Special Day (US)",            MACHINE_IMPERFECT_SOUND )
CONS( 2006, doraphonf,  doraphon, 0, doraphonep,doraphonep,spg2xx_game_doraphone_state,empty_init,    "VTech",                                                  "Dora the Explorer - Dora TV Explorer Phone / L'anniversaire de Babouche (France)", MACHINE_IMPERFECT_SOUND )
// This was from a 'cost reduced' unit with the 'non-TV' mode switch and internal speaker removed, however it looks like the code was not disabled or removed as the mode is fully functional.
// The ZC-Infinity video for this on YouTube shows the map scrolling to center the continent, there doesn't appear to be an input for this, different revision?
// a Dutch localized version also exists, which again must be different code
CONS( 2007, doraglob,   0,        0, doraphone, doraglobe, spg2xx_game_doraphone_state,empty_init,    "VTech",                                                  "Dora the Explorer - Dora TV Adventure Globe",                           MACHINE_IMPERFECT_SOUND )
CONS( 2007, doraglobf,  doraglob, 0, doraphone, doraglobe, spg2xx_game_doraphone_state,empty_init,    "VTech",                                                  "Dora the Explorer - Dora TV Globe-Trotter (France)",                    MACHINE_IMPERFECT_SOUND )
CONS( 2007, doraglobg,  doraglob, 0, doraphone, doraglobe, spg2xx_game_doraphone_state,empty_init,    "VTech",                                                  "Dora the Explorer - Doras Abenteuer-Globus (Germany)",                  MACHINE_IMPERFECT_SOUND )


// ROM checksum fails, but is expecting 0 as a result? shows 'CopyRight' when booting normally? protection?
CONS( 200?, jouet, 0,             0, spg2xx,    spg2xx,    spg2xx_game_state,          empty_init,    "<unknown>",                                              "10 Jeux Interactifs / Jeux Pour Filles (France)",                       MACHINE_NOT_WORKING )

CONS( 2008, senspeed,  0,         0, senspeed,  senspeed,  spg2xx_game_senspeed_state, empty_init,    "Senario",                                                "Speed Racer (Senario)",                                                 MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, jjstrip,    0,        0, tvsprt10,  jjstrip,   spg2xx_game_state,          empty_init,    "Shiggles Inc.",                                          "Club Jenna Presents: Jenna Jameson's Strip Poker",                      MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2005, tmntbftc,   0,        0, spg2xx,    tmntbftc,  spg2xx_game_state,          empty_init,    "Tech2Go / WayForward",                                   "Teenage Mutant Ninja Turtles: Battle for the City",                     MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// The black flashing square on startup is part of the tilemap layer, it doesn't appear to happen on hardware
// P.L.U.G.G.U.H.S. = Play Lots of Unbelievable Games, Getting Ultra High Scores
CONS( 2005, knd,        0,        0, spg2xx,    knd,       spg2xx_game_state,          init_crc,      "Tech2Go / One Man Band",                                 "Codename: Kids Next Door - Operation: P.L.U.G.G.U.H.S.",                MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2005, tmntmutm,   0,        0, tmntmutm,  tmntmutm,  spg2xx_game_tmntmutm_state, empty_init,    "Tech2Go / WayForward",                                   "Teenage Mutant Ninja Turtles: Mutant and Monster Mayhem",               MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2006, pballpup,   0,        0, pballpup,  pballpup,  spg2xx_game_pballpup_state, empty_init,    "Hasbro / Tiger Electronics",                             "Mission: Paintball Powered Up",                                         MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2007, dreamlss,   0,        0, dreamlss,  dreamlss,  spg2xx_game_dreamlss_state, empty_init,    "Hasbro / Tiger Electronics",                             "Dream Life Superstar (Version 0.3, Mar 16 2007)",                       MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Needs a hack to not show garbage sprite on startup
CONS( 2008, swclone,    0,        0, swclone,   swclone,   spg2xx_game_swclone_state,  init_swclone,  "Hasbro / Tiger Electronics",                             "Star Wars: The Clone Wars - Clone Trooper Blaster Game",                MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Mattel games
CONS( 2005, mattelcs,   0,        0, rad_skat,  mattelcs,  spg2xx_game_state,          empty_init,    "Mattel",                                                 "Mattel Classic Sports",                                                 MACHINE_IMPERFECT_SOUND )

// there's also a single player Hot Wheels Plug and Play that uses a wheel style controller
CONS( 2006, hotwhl2p,   0,        0, hotwheels, hotwheels, spg2xx_game_hotwheels_state,empty_init,    "Mattel",                                                 "Hot Wheels (2 player, pad controllers)",                                MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// there was also an English release of this, simply titled "Interactive TV Computer"
CONS( 2007, ordentv,    0,        0, ordentv,   ordentv,   spg2xx_game_ordentv_state,  init_ordentv,  "Taikee / V-Tac",                                         "Ordenador-TV (Spain)",                                                  MACHINE_NOT_WORKING )
CONS( 2007, jeuint,     ordentv,  0, ordentv,   ordentv,   spg2xx_game_ordentv_state,  init_jeuint,   "Taikee / V-Tac",                                         u8"Jeu Intéractif TV (France)",                                          MACHINE_NOT_WORKING)

CONS( 200?, wfart,      0,        0, wfcentro,  spg2xx,    spg2xx_game_wfcentro_state, empty_init,    "WinFun",                                                 "TV Art Design Center",                                                  MACHINE_NOT_WORKING )
CONS( 200?, wfcentro,   wfart,    0, wfcentro,  spg2xx,    spg2xx_game_wfcentro_state, empty_init,    "WinFun",                                                 "Centro TV de Diseno Artistico (Spain)",                                 MACHINE_NOT_WORKING )

CONS( 200?, lexiart,    0,        0, lexiart,   lexiart,   spg2xx_game_lexiart_state,  empty_init,    "Lexibook",                                               "Lexibook Junior My 1st Drawing Studio",                                 MACHINE_NOT_WORKING )

// set 2862 to 0003 (irq enable) when it stalls on boot to show something (doesn't turn on IRQs again otherwise?) needs camera emulating
CONS( 200?, tiktokmm,   0,        0, spg2xx,    spg2xx,    spg2xx_game_wfcentro_state, empty_init,    "TikTokTech Ltd. / 3T Games / Senario",                   "Moving Music (MM-TV110)",                                               MACHINE_NOT_WORKING )

CONS( 2005, doyousud,   0,        0, spg2xx,    doyousud,  spg2xx_game_state,          empty_init,    "SDW Games",                                              "Sudoku: Do You Sudoku?",                                                MACHINE_NOT_WORKING )

CONS( 200?, virtbb,     0,        0, spg2xx,    virtbb,    spg2xx_game_state,          empty_init,    "VTG Interactive",                                        "Virtual Baseball (VTG)",                                                MACHINE_NOT_WORKING ) // motion controls not fully understood
CONS( 200?, virtten,    0,        0, spg2xx,    virtten,   spg2xx_game_state,          empty_init,    "VTG Interactive",                                        "Virtual Tennis (VTG)",                                                  MACHINE_NOT_WORKING ) // motion controls not fully understood

// 2007 ingame, 2008 on box.  Hyperkin is mentioned as being the registered trademark holder alongside DDRGame on the box.
// Songs "composed by Kenneth Baylon"
CONS( 2008, ddr33v,     0,        0, spg2xx,    ddr33v,    spg2xx_game_ddr33v_state,   init_ddr33v,   "DDRGame / Hyperkin",                                    "16-bit TV Dance Pad with 15 songs / Dance Dance Party Mix (DDRGame)",   MACHINE_IMPERFECT_SOUND )

// PCB has 'Anpanman TV 2006 Ver 1.4' printed on it, ROM has SPG260 header.  Uses custom built-in keyboard, no display built into the unit.
CONS( 2006, anpantv,    0,        0, spg2xx,    spg2xx,    spg2xx_game_state,          empty_init,    "Bandai",                                                "Anpanman TV (Japan)",                                                   MACHINE_NOT_WORKING )

// Has an AT24C08, not currently hooked up (probably for storing database unlocks)
//
// There is also a card reader/scanner which can read barcodes from Digimon cards
// and IR connectivity which allowed for data exchange with various services using
// an external device, including transfering characters to/from an arcade game.
// Neither is currently emulated
//
// Will report 'ERROR' sometimes, maybe as a result of these not being hooked up.
CONS( 2006, dmbtjunc,   0,        0, spg2xx,    dmbtjunc,  spg2xx_game_state,          empty_init,    "Bandai",                                                "Let's! TV Play Digital Monster Battle Junction (Japan)",                MACHINE_NOT_WORKING )

// Let's!TVプレイ 脳と体を鍛える 体感頭脳ファミリーマットレ  - Let's! TV Play branding appears on the box
CONS( 2006, ban_krkk,   0,        0, spg2xx,    ban_krkk,  spg2xx_game_state,          init_crc,      "Bandai",                                                "Let's! TV Play Nou to Karada o Kitaeru Taikan Zunou Family Mattore (Japan)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, epo_tetr,   0,        0, epo_tetr,  epo_tetr,  epo_tetr_game_state,          empty_init,    "Epoch",                                                "Minna no Tetris (Japan)", MACHINE_IMPERFECT_SOUND )

// Train Game V1.4 2012-08-15 on PCB. SPG243 headers in each chunk.
// Last few bytes of SEEPROM have 'JUNGT' in them, is this developed by JungleSoft/JungleTac?
CONS( 2012, prail,      0,        0, prail,     prail,     spg2xx_game_prail_state,    empty_init,   "Takara Tomy",                                            "Boku wa Plarail Untenshi - Shinkansen de Ikou! (Japan)",                MACHINE_IMPERFECT_SOUND )
// the 'plus' version from 2015 runs on newer hardware, see generalplus_gpl16250_spi.cpp
