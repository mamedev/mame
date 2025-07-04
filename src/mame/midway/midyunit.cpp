// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Williams/Midway Y/Z-unit system

    driver by Alex Pasadyn, Zsolt Vasvari, Kurt Mahan, Ernesto Corvi,
    and Aaron Giles

    Games supported (prototype and release versions):
        * Narc
        * Trog
        * Strike Force
        * Smash TV
        * High Impact Football
        * Super High Impact
        * Terminator 2
        * Mortal Kombat (Y-unit versions)
        * Total Carnage

    Known bugs:
        * when the Porsche spins in Narc, the wheels are missing for
            a single frame (may be an original bug)
        * Terminator 2 freezes while playing the movies after destroying
            skynet. Currently we have a hack in which prevents the freeze,
            but we really should eventually figure it out for real

    Notes:
        * Super Hi Impact Proto V4.0: You need to at least reset the high score
            table from the UTILITIES menu.  It's best to do a FULL FACTORY RESTORE


*************************************************************************

Super High Impact
Midway, 1991

This game runs on (typical) Midway Y-Unit hardware. The PCB is a base
board 'system' that can run other Y-Unit games by swapping ROMs and
the protection chip.

PCB Layout
----------

5770-12555-00 REV. - A
|-------------------------------------------------------------------|
|                     6264  PAL    53461 53461 53461 48MHZ 24MHz  J6|
|J4                                53461 53461 53461                |
|                                  53461 53461 53461                |
|                                  53461 53461 53461                |
|J2           MAX691  6264  PAL                         |-----|     |
|                         PAL PAL                       |TMS  |   J7|
|RESET_SW                                               |34010|     |
|  LED1                                                 |-----|     |
|    LED2             6264 PAL PAL                                  |
|                                                                   |
|J       41464 41464                                              J8|
|A  PAL  41464 41464                                                |
|M                                                                  |
|M                                                                  |
|A           U89  U90  U91  U92  U93     U95  U96  U97  U98         |
|                                                     |--------|    |
|                                                     |L1A3787 |    |
|                                                     |WILLIAMS|    |
|                                                     |5410-12239   |
|                                                     |--------|    |
|DSW1 DSW2                                                          |
|BATTERY                                             *A-5346-40017-8|
|                                                                   |
| J13   J12  U105 U106 U107 U108 U109    U111 U112 U113 U114        |
|-------------------------------------------------------------------|
Notes:
      *    - Intel P5C090-50 protection chip labelled 'A-5346-40017-8'
             clocks on pin1 6.00MHz, pin4 6.00MHz, pin21 6.00MHz
     J*    - multi-pin connectors for additional controls, cabinet switches
             and power input and output
     J8    - used to connect external sound PCB via a flat cable.
     J2    - Used to supply power to external sound PCB
     34010 - clock 6.000MHz [48/8]
     LED1  - power active
     LED2  - data active, flickers while PCB is working
     U89/105  - main program EPROMs 27C010
     Other U* - 27C020 EPROMs


Sound PCB (see /shared/williamssound.cpp)
---------

5766-12702-00 REV. B
|----------------------------------------|
|    3.579545MHz 6116  U4  U19  U20      |
|  YM2151                                |
|YM3012                                  |
|      458                         6809  |
| 458                                    |
|         MC1408                         |
|J1            6821                      |
|                                        |
| 458    55536                  8MHz     |
|     458                                |
|                                        |
|J2                                     J|
|         J3     J4       TDA2002 TDA2002|
|----------------------------------------|
Notes:
      6809   - clock 2.000MHz [8/4]
      YM2151 - clock 3.579545MHz
      55536  - Harris HC-55536 Continuously Variable Slope Delta Modulator
               (some early boards use an HC-55516, later boards use an HC-55536 or HC-55564)
      6116   - 2k x8 SRAM
      U*     - 27C010 EPROMs
      J4     - flat cable connector from main board J8
      J3     - power input connector
      J2     - connector for volume pot
      J      - speakers output connector

**************************************************************************/

#include "emu.h"
#include "midyunit.h"

#include "cpu/z80/z80.h"
#include "screen.h"
#include "speaker.h"




/*************************************
 *
 *  Yawdim sound banking
 *
 *************************************/

void mkyawdim_state::yawdim_oki_bank_w(uint8_t data)
{
	if (BIT(data, 2))
		m_oki->set_rom_bank(data & 3);
}

void mkyawdim_state::yawdim2_oki_bank_w(uint8_t data)
{
	int const bnk = (data >> 1 & 4) + (data & 3);
	m_oki->set_rom_bank(bnk);

	if (BIT(~data, 2))
		m_oki->reset();
}



/*************************************
 *
 *  Sound board comms
 *
 *************************************/

int midzunit_state::narc_talkback_strobe_r()
{
	return BIT(m_narc_sound->read(), 8);
}


ioport_value midzunit_state::narc_talkback_data_r()
{
	return m_narc_sound->read() & 0xff;
}


int midyunit_adpcm_state::adpcm_irq_state_r()
{
	return m_adpcm_sound->irq_read() & 1;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

void midyunit_base_state::main_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rw(FUNC(midyunit_base_state::vram_r), FUNC(midyunit_base_state::vram_w));
	map(0x01000000, 0x010fffff).ram().share(m_mainram);
	map(0x01400000, 0x0140ffff).rw(FUNC(midyunit_base_state::cmos_r), FUNC(midyunit_base_state::cmos_w));
	map(0x01800000, 0x0181ffff).ram().w(FUNC(midyunit_base_state::paletteram_w)).share("paletteram");
	map(0x01a00000, 0x01a0009f).mirror(0x00080000).rw(FUNC(midyunit_base_state::dma_r), FUNC(midyunit_base_state::dma_w));
	map(0x01c00000, 0x01c0005f).r(FUNC(midyunit_base_state::input_r));
	map(0x01c00060, 0x01c0007f).rw(FUNC(midyunit_base_state::protection_r), FUNC(midyunit_base_state::cmos_enable_w));
	map(0x01f00000, 0x01f0001f).w(FUNC(midyunit_base_state::control_w));
	map(0x02000000, 0x05ffffff).r(FUNC(midyunit_base_state::gfxrom_r));
	map(0xff800000, 0xffffffff).rom().region("maindata", 0);
}

void midzunit_state::zunit_main_map(address_map &map)
{
	main_map(map);
	map(0x01e00000, 0x01e0001f).w(FUNC(midzunit_state::narc_sound_w));
}

void midyunit_cvsd_state::cvsd_main_map(address_map &map)
{
	main_map(map);
	map(0x01e00000, 0x01e0001f).w(FUNC(midyunit_cvsd_state::cvsd_sound_w));
}

void midyunit_adpcm_state::adpcm_main_map(address_map &map)
{
	main_map(map);
	map(0x01e00000, 0x01e0001f).w(FUNC(midyunit_adpcm_state::adpcm_sound_w));
}

void term2_state::term2_main_map(address_map &map)
{
	main_map(map);
	map(0x01c00000, 0x01c0005f).r(FUNC(term2_state::term2_input_r));
	map(0x01e00000, 0x01e0001f).w(FUNC(term2_state::term2_sound_w));
}

void mkyawdim_state::yawdim_main_map(address_map &map)
{
	main_map(map);
	map(0x01e00000, 0x01e0000f).w(FUNC(mkyawdim_state::yawdim_sound_w));
}

void mkyawdim_state::yawdim2_main_map(address_map &map)
{
	main_map(map);
	map(0x01e00000, 0x01e0000f).w(FUNC(mkyawdim_state::yawdim2_sound_w));
}


void mkyawdim_state::yawdim_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x97ff).w(FUNC(mkyawdim_state::yawdim_oki_bank_w));
	map(0x9800, 0x9fff).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa7ff).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void mkyawdim_state::yawdim2_sound_map(address_map &map)
{
	yawdim_sound_map(map);
	map(0x9000, 0x97ff).w(FUNC(mkyawdim_state::yawdim2_oki_bank_w));
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( narc )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Crouch") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Fire") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Jump") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Rocket Bomb") PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Crouch") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Fire") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Jump") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Rocket Bomb") PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) // Video Freeze
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Vault Switch") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(midzunit_state::narc_talkback_strobe_r))
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED ) // memory protect interlock
	PORT_BIT( 0x3000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )
/*
    Test mode indicates "Cut for French" and "Cut for German", hinting that these
    are jumpers or wires that can be modified on the PCB. However, there are no
    French or German strings in the ROMs, and this "feature" was clearly never
    actually implemented
    PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Language ) )
    PORT_DIPSETTING(      0xc000, DEF_STR( English ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( French ) )
    PORT_DIPSETTING(      0x4000, DEF_STR( German ) )
*/

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(midzunit_state::narc_talkback_data_r))
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( trog )
	PORT_START("IN0") // Input 0-15 on D0-D15
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Punch") PORT_PLAYER(1)
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Punch") PORT_PLAYER(2)
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1") // coin1,coin2,start1,tilt,test,start2,service1,input23-31 on D0-D15
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BILL1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED ) // video freeze
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P3 Punch") PORT_PLAYER(3)

	PORT_START("IN2") // input32-input39 on D0-D7; D8-D15 unused
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P4 Punch") PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW") // DSW1 and DSW2 on D0-D15
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Cabinet ))         PORT_DIPLOCATION("DS1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Upright ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x0040, 0x0040, "Coinage Source" )          PORT_DIPLOCATION("DS1:2")
	PORT_DIPSETTING(      0x0040, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coinage ))         PORT_DIPLOCATION("DS1:5,4,3")
	PORT_DIPSETTING(      0x0038, "1" )
	PORT_DIPSETTING(      0x0018, "2" )
	PORT_DIPSETTING(      0x0028, "3" )
	PORT_DIPSETTING(      0x0008, "4" )
	PORT_DIPSETTING(      0x0030, "ECA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ))          PORT_DIPLOCATION("DS1:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ))          PORT_DIPLOCATION("DS1:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unused ))          PORT_DIPLOCATION("DS1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_DIPNAME( 0xc000, 0xc000, "Country" )                 PORT_DIPLOCATION("DS2:2,1")
	PORT_DIPSETTING(      0xc000, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( French ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( German ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Unused ))
	PORT_DIPNAME( 0x2000, 0x0000, "Power-Up Test" )           PORT_DIPLOCATION("DS2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x2000, DEF_STR( On ))
	PORT_DIPNAME( 0x1000, 0x1000, "Coin Counters" )           PORT_DIPLOCATION("DS2:4")
	PORT_DIPSETTING(      0x1000, "One" )
	PORT_DIPSETTING(      0x0000, "Two" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Players ) )        PORT_DIPLOCATION("DS2:6,5")
	PORT_DIPSETTING(      0x0c00, "4 Players" )
	PORT_DIPSETTING(      0x0400, "3 Players" )
	PORT_DIPSETTING(      0x0800, "2 Players" )
	PORT_DIPSETTING(      0x0000, "1 Player" )
	PORT_DIPNAME( 0x0200, 0x0200, "Video Freeze" )            PORT_DIPLOCATION("DS2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0100, 0x0100, "Test Switch" )             PORT_DIPLOCATION("DS2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_START("UNK0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( trogpa4 )
	PORT_INCLUDE(trog)

	// Player controls the hand in this prototype version.
	PORT_MODIFY("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Bone") PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Bone") PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P3 Bone") PORT_PLAYER(3)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P4 Bone") PORT_PLAYER(4)
INPUT_PORTS_END

static INPUT_PORTS_START( hiimpact )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Action") PORT_PLAYER(1)
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Action") PORT_PLAYER(2)
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Video Freeze") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P3 Action") PORT_PLAYER(3)

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P4 Action") PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC ( 0x0001, 0x0001, "DS1:8" )
	PORT_DIPUNUSED_DIPLOC ( 0x0002, 0x0002, "DS1:7" )
	PORT_DIPUNUSED_DIPLOC ( 0x0004, 0x0004, "DS1:6" )
	PORT_DIPNAME( 0x0078, 0x0078, DEF_STR( Coinage )) PORT_DIPLOCATION("DS1:5,4,3,2")
	PORT_DIPSETTING(      0x0078, DEF_STR( 1C_1C ))        PORT_CONDITION("DSW", 0xc000, EQUALS, 0xc000) // Generic coinage (no denomination); 2 identical chutes
	PORT_DIPSETTING(      0x0058, DEF_STR( 2C_1C ))        PORT_CONDITION("DSW", 0xc000, EQUALS, 0xc000)
	PORT_DIPSETTING(      0x0068, "2 Coins/1 Credit 4/3" ) PORT_CONDITION("DSW", 0xc000, EQUALS, 0xc000)
	PORT_DIPSETTING(      0x0048, "2 Coins/1 Credit 4/4" ) PORT_CONDITION("DSW", 0xc000, EQUALS, 0xc000)
	PORT_DIPSETTING(      0x0070, "ECA" )                  PORT_CONDITION("DSW", 0xc000, EQUALS, 0xc000) // 25 cents; 4 chutes - dollar/quarter/dime/nickel
	PORT_DIPSETTING(      0x0078, "1DM/1 Credit 6/5" )     PORT_CONDITION("DSW", 0xc000, EQUALS, 0x4000) // German coinage; these 4 have 2 chutes (1DM/5DM)
	PORT_DIPSETTING(      0x0058, "1DM/1 Credit 7/5" )     PORT_CONDITION("DSW", 0xc000, EQUALS, 0x4000)
	PORT_DIPSETTING(      0x0068, "1DM/1 Credit 8/5" )     PORT_CONDITION("DSW", 0xc000, EQUALS, 0x4000)
	PORT_DIPSETTING(      0x0048, "1DM/1 Credit" )         PORT_CONDITION("DSW", 0xc000, EQUALS, 0x4000)
	PORT_DIPSETTING(      0x0070, "ECA" )                  PORT_CONDITION("DSW", 0xc000, EQUALS, 0x4000) // 1/1DM 6/5DM; 3 chutes (5DM/2DM/1DM)
	PORT_DIPSETTING(      0x0078, "5F/2 Credits 10/5" )    PORT_CONDITION("DSW", 0xc000, EQUALS, 0x8000) // French coinage; 2 chutes (5F/10F)
	PORT_DIPSETTING(      0x0058, "5F/2 Credits" )         PORT_CONDITION("DSW", 0xc000, EQUALS, 0x8000)
	PORT_DIPSETTING(      0x0068, "5F/1 Credit 10/3" )     PORT_CONDITION("DSW", 0xc000, EQUALS, 0x8000)
	PORT_DIPSETTING(      0x0048, "5F/1 Credit" )          PORT_CONDITION("DSW", 0xc000, EQUALS, 0x8000)
	PORT_DIPSETTING(      0x0070, "ECA" )                  PORT_CONDITION("DSW", 0xc000, EQUALS, 0x8000) // 1/3F 2/5F 5/10F; 3 chutes (1F/5F/10F)
	PORT_DIPSETTING(      0x0040, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Players ))         PORT_DIPLOCATION("DS1:1")
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPSETTING(      0x0000, "2" )

	PORT_DIPNAME( 0x0100, 0x0100, "Test Switch" )             PORT_DIPLOCATION("DS2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPUNUSED_DIPLOC (0x0200, 0x0200, "DS2:7")
	PORT_DIPUNUSED_DIPLOC (0x0400, 0x0400, "DS2:6")
	PORT_DIPUNUSED_DIPLOC (0x0800, 0x0800, "DS2:5")
	PORT_DIPNAME( 0x1000, 0x1000, "Coin Counters" )           PORT_DIPLOCATION("DS2:4")
	PORT_DIPSETTING(      0x1000, "One" )
	PORT_DIPSETTING(      0x0000, "Two" )
	PORT_DIPNAME( 0x2000, 0x2000, "Power-Up Test" )           PORT_DIPLOCATION("DS2:3") // Manual says "unused"
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))                                       // Service menu says "Eat time"
	PORT_DIPSETTING(      0x2000, DEF_STR( On ))                                        // Service menu says "Don't eat time"
	PORT_DIPNAME( 0xc000, 0xc000, "Country" )                 PORT_DIPLOCATION("DS2:2,1") // Affects currency used. Language remains in English
	PORT_DIPSETTING(      0xc000, DEF_STR( USA ))
	PORT_DIPSETTING(      0x4000, DEF_STR( German ))
	PORT_DIPSETTING(      0x8000, DEF_STR( French ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Unused ))

	PORT_START("UNK0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( shimpact )
	PORT_INCLUDE( hiimpact )
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, "Card Dispenser" ) PORT_DIPLOCATION("DS1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( On ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
INPUT_PORTS_END

static INPUT_PORTS_START( smashtv )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_NAME("P1 Move Up") PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_NAME("P1 Move Down") PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_NAME("P1 Move Left") PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_NAME("P1 Move Right") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_NAME("P1 Fire Up") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_NAME("P1 Fire Down") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_NAME("P1 Fire Left")PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_NAME("P1 Fire Right") PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_NAME("P2 Move Up") PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_NAME("P2 Move Down") PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_NAME("P2 Move Left") PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_NAME("P2 Move Right") PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_NAME("P2 Fire Up") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_NAME("P2 Fire Down") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_NAME("P2 Fire Left") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_NAME("P2 Fire Right") PORT_PLAYER(2)


	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BILL1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED ) // video freeze
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")

	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS1:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x003f, 0x003f, DEF_STR( Coinage ))         PORT_DIPLOCATION("DS1:8,7,6,5,4,3") // To activate any coinage changes go to Test / Utilities
	PORT_DIPSETTING(      0x003f, "USA 1" ) // 1 Coin 1 Credit                                       and select 'FULL FACTORY RESTORE'
	PORT_DIPSETTING(      0x003e, "USA 2" )                                                       // Then exit test mode and the new coinage will be active.
	PORT_DIPSETTING(      0x003d, "USA 3" )                                                       // This actually checks the dipswitch and puts the coin/credit value
	PORT_DIPSETTING(      0x003c, "German 1" )                                                    // into CMOS. But it only checks when doing a full factory restore.
	PORT_DIPSETTING(      0x003b, "German 2" )
	PORT_DIPSETTING(      0x003a, "German 3" )
	PORT_DIPSETTING(      0x0039, "France 1" )
	PORT_DIPSETTING(      0x0038, "France 2" )
	PORT_DIPSETTING(      0x0037, "France 3" )
	PORT_DIPSETTING(      0x0036, "Swiss 1" )
	PORT_DIPSETTING(      0x0035, "Italy" )
	PORT_DIPSETTING(      0x0034, "U.K. 1" )
	PORT_DIPSETTING(      0x0033, "U.K. 2" )
	PORT_DIPSETTING(      0x0032, "U.K. ECA" )
	PORT_DIPSETTING(      0x0031, "Spain 1" )
	PORT_DIPSETTING(      0x0030, "Australia 1" )
	PORT_DIPSETTING(      0x002f, "Japan 1" )
	PORT_DIPSETTING(      0x002e, "Japan 2" )
	PORT_DIPSETTING(      0x002d, "Austria 1" )
	PORT_DIPSETTING(      0x002c, "Belgium 1" )
	PORT_DIPSETTING(      0x002b, "Belgium 2" )
	PORT_DIPSETTING(      0x002a, "Sweden" )
	PORT_DIPSETTING(      0x0029, "New Zealand 1" )
	PORT_DIPSETTING(      0x0028, "Netherlands" )
	PORT_DIPSETTING(      0x0027, "Finland" )
	PORT_DIPSETTING(      0x0026, "Norway" )
	PORT_DIPSETTING(      0x0025, "Denmark 1" )

	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )             PORT_DIPLOCATION("DS2:1") // Listed in kit manual as 'kit test' but only works on rev 6 and rev 8. F2 service is working on all versions.
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ))                                       // Maybe rev 6 and 8 are kit versions?
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x4000, 0x4000, "Rotary Joystick Enable?" ) PORT_DIPLOCATION("DS2:2") // Is rotary joystick supported in the code?
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ))                                       // Might be in a later undumped ROM version or not implemented?
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:3") // Later 'kit' manuals include the DIPs. 3-8 is unlisted so not used?
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:5")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_START("UNK0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( strkforc )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Fire") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Weapon") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Weapon Select") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Fire") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Weapon") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Weapon Select") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start / Transform")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 Start / Transform")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Difficulty ))      PORT_DIPLOCATION("DS1:3,2,1")
	PORT_DIPSETTING(      0x0080, "Level 1" )
	PORT_DIPSETTING(      0x00c0, "Level 2" )
	PORT_DIPSETTING(      0x00a0, "Level 3" )
	PORT_DIPSETTING(      0x00e0, "Level 4" )
	PORT_DIPSETTING(      0x0060, "Level 5" )
	PORT_DIPSETTING(      0x0040, "Level 6" )
	PORT_DIPSETTING(      0x0020, "Level 7" )
	PORT_DIPSETTING(      0x0000, "Level 8" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Lives ))           PORT_DIPLOCATION("DS1:4")
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, "Points for Extra Ship" )   PORT_DIPLOCATION("DS1:6,5")
	PORT_DIPSETTING(      0x0008, "40000" )
	PORT_DIPSETTING(      0x000c, "50000" )
	PORT_DIPSETTING(      0x0004, "75000" )
	PORT_DIPSETTING(      0x0000, "100000" )
	PORT_DIPNAME( 0x0002, 0x0002, "Credits to Start" )        PORT_DIPLOCATION("DS1:7")
	PORT_DIPSETTING(      0x0002, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Meter" )              PORT_DIPLOCATION("DS1:8")
	PORT_DIPSETTING(      0x0001, "Shared" )
	PORT_DIPSETTING(      0x0000, "Independent" )

	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )             PORT_DIPLOCATION("DS2:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x7800, 0x7800, DEF_STR( Coin_A ))          PORT_DIPLOCATION("DS2:5,4,3,2")
	PORT_DIPSETTING(      0x3000, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(      0x3800, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x4800, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x7800, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x6800, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(      0x5800, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(      0x2800, "1 Coin/1 Credit - 2 Coins/3 Credits" )
	PORT_DIPSETTING(      0x2000, "1 Coin/1 Credit - 3 Coins/4 Credits" )
	PORT_DIPSETTING(      0x1800, "1 Coin/1 Credit - 4 Coins/5 Credits" )
	PORT_DIPSETTING(      0x1000, "1 Coin/1 Credit - 5 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0800, "3 Coin/1 Credit - 5 Coins/2 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Coin/2 Credits - 2 Coins/5 Credits" )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_B ))          PORT_DIPLOCATION("DS2:8,7,6")
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(      0x0100, "U.K. Elect." )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))

	PORT_START("UNK0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( mkla4 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 High Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Block") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 High Kick") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 High Punch") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Block") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 High Kick") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BILL1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Low Punch") PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Low Kick") PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Block 2") PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Low Punch") PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Low Kick") PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(midyunit_adpcm_state::adpcm_irq_state_r))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Block 2") PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0080, 0x0080, "Violence" )                PORT_DIPLOCATION("DS1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0080, DEF_STR( On ))
	PORT_DIPNAME( 0x0040, 0x0040, "Blood" )                   PORT_DIPLOCATION("DS1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0040, DEF_STR( On ))
	PORT_DIPNAME( 0x0020, 0x0020, "Low Blows" )               PORT_DIPLOCATION("DS1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0020, DEF_STR( On ))
	PORT_DIPNAME( 0x0010, 0x0010, "Attract Sound" )           PORT_DIPLOCATION("DS1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0010, DEF_STR( On ))
	PORT_DIPNAME( 0x0008, 0x0008, "Comic Book Offer" )        PORT_DIPLOCATION("DS1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0008, DEF_STR( On ))
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ))          PORT_DIPLOCATION("DS1:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ))          PORT_DIPLOCATION("DS1:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unused ))          PORT_DIPLOCATION("DS1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_DIPNAME( 0x8000, 0x8000, "Coinage Source" )          PORT_DIPLOCATION("DS2:1")
	PORT_DIPSETTING(      0x8000, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x7c00, 0x5c00, DEF_STR( Coinage ))         PORT_DIPLOCATION("DS2:6,5,4,3,2") // ignored when DS2:1 set to CMOS
	PORT_DIPSETTING(      0x7c00, "USA-1" )
	PORT_DIPSETTING(      0x3c00, "USA-2" )
	PORT_DIPSETTING(      0x5c00, "USA-3" ) // 1 Coin 1 Credit
	PORT_DIPSETTING(      0x1c00, "USA-4" )
	PORT_DIPSETTING(      0x6c00, "USA-ECA" )
	PORT_DIPSETTING(      0x0c00, "USA-Free Play" )
	PORT_DIPSETTING(      0x7400, "German-1" )
	PORT_DIPSETTING(      0x3400, "German-2" )
	PORT_DIPSETTING(      0x5400, "German-3" )
	PORT_DIPSETTING(      0x1400, "German-4" )
	PORT_DIPSETTING(      0x6400, "German-5" )
	PORT_DIPSETTING(      0x2400, "German-ECA" )
	PORT_DIPSETTING(      0x0400, "German-Free Play" )
	PORT_DIPSETTING(      0x7800, "French-1" )
	PORT_DIPSETTING(      0x3800, "French-2" )
	PORT_DIPSETTING(      0x5800, "French-3" )
	PORT_DIPSETTING(      0x1800, "French-4" )
	PORT_DIPSETTING(      0x6800, "French-ECA" )
	PORT_DIPSETTING(      0x0800, "French-Free Play" )
	PORT_DIPNAME( 0x0200, 0x0200, "Counters" )                PORT_DIPLOCATION("DS2:7")
	PORT_DIPSETTING(      0x0200, "One" )
	PORT_DIPSETTING(      0x0000, "Two" )
	PORT_DIPNAME( 0x0100, 0x0100, "Test Switch" )             PORT_DIPLOCATION("DS2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_START("UNK0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( mkla2 )
	PORT_INCLUDE( mkla4 )
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ))          PORT_DIPLOCATION("DS1:5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
INPUT_PORTS_END


static INPUT_PORTS_START( mkyawdim )
	PORT_INCLUDE( mkla4 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED ) // no ADPCM in these bootlegs
INPUT_PORTS_END

static INPUT_PORTS_START( term2 )
	PORT_START("IN0")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Trigger") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Bomb") PORT_PLAYER(1)
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Trigger") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Bomb") PORT_PLAYER(2)
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED ) // video freeze
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x3000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(midyunit_adpcm_state::adpcm_irq_state_r))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0080, 0x0080, "Display" )                 PORT_DIPLOCATION("DS1:1")
	PORT_DIPSETTING(      0x0080, "Normal" )
	PORT_DIPSETTING(      0x0000, "Mirrored" )
	PORT_DIPNAME( 0x0040, 0x0040, "Coinage Source" )          PORT_DIPLOCATION("DS1:2")
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPSETTING(      0x0040, "Dipswitch" )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coinage ))         PORT_DIPLOCATION("DS1:5,4,3")
	PORT_DIPSETTING(      0x0038, "1" )
	PORT_DIPSETTING(      0x0018, "2" )
	PORT_DIPSETTING(      0x0028, "3" )
	PORT_DIPSETTING(      0x0008, "4" )
	PORT_DIPSETTING(      0x0030, "ECA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0007, 0x0003, "Credits" )                 PORT_DIPLOCATION("DS1:8,7,6")
	PORT_DIPSETTING(      0x0007, "2 Start/1 Continue" )
	PORT_DIPSETTING(      0x0006, "4 Start/1 Continue" )
	PORT_DIPSETTING(      0x0005, "2 Start/2 Continue" )
	PORT_DIPSETTING(      0x0004, "4 Start/2 Continue" )
	PORT_DIPSETTING(      0x0003, "1 Start/1 Continue" )
	PORT_DIPSETTING(      0x0002, "3 Start/2 Continue" )
	PORT_DIPSETTING(      0x0001, "3 Start/1 Continue" )
	PORT_DIPSETTING(      0x0000, "3 Start/3 Continue" )

	PORT_DIPNAME( 0xc000, 0xc000, "Country" )                 PORT_DIPLOCATION("DS2:2,1")
	PORT_DIPSETTING(      0xc000, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( French ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( German ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Unused ))
	PORT_DIPNAME( 0x2000, 0x0000, "Power-Up Test" )           PORT_DIPLOCATION("DS2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x2000, DEF_STR( On ))
	PORT_DIPNAME( 0x1000, 0x1000, "Counters" )                PORT_DIPLOCATION("DS2:4")
	PORT_DIPSETTING(      0x1000, "One" )
	PORT_DIPSETTING(      0x0000, "Two" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Players ) )        PORT_DIPLOCATION("DS2:5")
	PORT_DIPSETTING(      0x0800, "2 Players" )
	PORT_DIPSETTING(      0x0000, "1 Player" )
	PORT_DIPNAME( 0x0400, 0x0400, "Cabinet?" )                PORT_DIPLOCATION("DS2:6") // Manual says Dedicated / Kit. Does this do anything?
	PORT_DIPSETTING(      0x0400, "Dedicated" )
	PORT_DIPSETTING(      0x0000, "Kit" )
	PORT_DIPNAME( 0x0200, 0x0200, "Video Freeze" )            PORT_DIPLOCATION("DS2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0100, 0x0100, "Test Switch" )             PORT_DIPLOCATION("DS2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_START("UNK0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STICK0_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("STICK0_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("STICK1_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("STICK1_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( totcarn )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_NAME("P1 Move Up") PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_NAME("P1 Move Down") PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_NAME("P1 Move Left") PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_NAME("P1 Move Right") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_NAME("P1 Fire Up") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_NAME("P1 Fire Down") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_NAME("P1 Fire Left") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_NAME("P1 Fire Right") PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_NAME("P2 Move Up") PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_NAME("P2 Move Down") PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_NAME("P2 Move Left") PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_NAME("P2 Move Right") PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_NAME("P2 Fire Up") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_NAME("P2 Fire Down") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_NAME("P2 Fire Left") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_NAME("P2 Fire Right") PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start / Bomb")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 Start / Bomb")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BILL1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED ) // video freeze
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x3c00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(midyunit_adpcm_state::adpcm_irq_state_r))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0080, 0x0080, "Keys for Pleasure Dome" )  PORT_DIPLOCATION("DS1:1")
	PORT_DIPSETTING(      0x0080, "220" )
	PORT_DIPSETTING(      0x0000, "200" )
	PORT_DIPNAME( 0x0040, 0x0000, "Coinage Source" )          PORT_DIPLOCATION("DS1:2")
	PORT_DIPSETTING(      0x0040, "CMOS" )
	PORT_DIPSETTING(      0x0000, "Dipswitch" )
	PORT_DIPNAME( 0x003f, 0x003f, DEF_STR( Coinage ))         PORT_DIPLOCATION("DS1:8,7,6,5,4,3") // To activate any coinage changes go to Test / Utilities
	PORT_DIPSETTING(      0x003f, "USA 1" ) // 1 Coin 1 Credit                                       and select 'FULL FACTORY RESTORE'
	PORT_DIPSETTING(      0x003e, "USA 2" )                                                       // Then exit test mode and the new coinage will be active.
	PORT_DIPSETTING(      0x003d, "USA 3" )                                                       // This actually checks the dipswitch and puts the coin/credit value
	PORT_DIPSETTING(      0x003c, "German 1" )                                                    // into CMOS. But it only checks when doing a full factory restore.
	PORT_DIPSETTING(      0x003b, "German 2" )
	PORT_DIPSETTING(      0x003a, "German 3" )
	PORT_DIPSETTING(      0x0039, "France 2" )
	PORT_DIPSETTING(      0x0038, "France 3" )
	PORT_DIPSETTING(      0x0037, "France 4" )
	PORT_DIPSETTING(      0x0036, "Swiss 1" )
	PORT_DIPSETTING(      0x0035, "Italy" )
	PORT_DIPSETTING(      0x0034, "U.K. 1" )
	PORT_DIPSETTING(      0x0033, "U.K. 2" )
	PORT_DIPSETTING(      0x0032, "U.K. ECA" )
	PORT_DIPSETTING(      0x0031, "Spain 1" )
	PORT_DIPSETTING(      0x0030, "Australia 1" )
	PORT_DIPSETTING(      0x002f, "Japan 1" )
	PORT_DIPSETTING(      0x002e, "Japan 2" )
	PORT_DIPSETTING(      0x002d, "Austria 1" )
	PORT_DIPSETTING(      0x002c, "Belgium 1" )
	PORT_DIPSETTING(      0x002b, "Belgium 2" )
	PORT_DIPSETTING(      0x002a, "Sweden" )
	PORT_DIPSETTING(      0x0029, "New Zealand 1" )
	PORT_DIPSETTING(      0x0028, "Netherlands" )
	PORT_DIPSETTING(      0x0027, "Finland" )
	PORT_DIPSETTING(      0x0026, "Norway" )
	PORT_DIPSETTING(      0x0025, "Denmark 1" )
	PORT_DIPSETTING(      0x0000, "Denmark 2" )

	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:3")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:5")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ))         PORT_DIPLOCATION("DS2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0100, 0x0100, "Test Switch" )             PORT_DIPLOCATION("DS2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_START("UNK0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Z-unit machine driver
 *
 *************************************/

// master clocks vary based on game
static constexpr XTAL SLOW_MASTER_CLOCK   = XTAL(40'000'000);      // "slow" == smashtv, trog, hiimpact
static constexpr XTAL FAST_MASTER_CLOCK   = XTAL(48'000'000);      // "fast" == narc, mk, totcarn, strkforc
static constexpr XTAL FASTER_MASTER_CLOCK = XTAL(50'000'000);      // "faster" == term2

void midzunit_state::zunit(machine_config &config)
{
	// pixel clocks are 48MHz (narc) or 24MHz (all others) regardless
	constexpr XTAL MEDRES_PIXEL_CLOCK  = (XTAL(48'000'000) / 6);

	// basic machine hardware
	TMS34010(config, m_maincpu, FAST_MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &midzunit_state::zunit_main_map);
	m_maincpu->set_halt_on_reset(false);
	m_maincpu->set_pixel_clock(MEDRES_PIXEL_CLOCK);
	m_maincpu->set_pixels_per_clock(2);
	m_maincpu->set_scanline_ind16_callback(FUNC(midzunit_state::scanline_update));
	m_maincpu->set_shiftreg_in_callback(FUNC(midzunit_state::to_shiftreg));
	m_maincpu->set_shiftreg_out_callback(FUNC(midzunit_state::from_shiftreg));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	PALETTE(config, m_palette).set_entries(8192);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_ALWAYS_UPDATE);
	// from TMS340 registers
	screen.set_raw(MEDRES_PIXEL_CLOCK*2, 674, 122, 634, 433, 27, 427);
	screen.set_screen_update("maincpu", FUNC(tms34010_device::tms340x0_ind16));
	screen.set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();
	WILLIAMS_NARC_SOUND(config, m_narc_sound);
	m_narc_sound->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_narc_sound->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}



/*************************************
 *
 *  Y-unit machine drivers
 *
 *************************************/

void midyunit_base_state::yunit_core(machine_config &config)
{
	constexpr XTAL STDRES_PIXEL_CLOCK = (XTAL(24'000'000) / 6);

	// basic machine hardware
	TMS34010(config, m_maincpu, SLOW_MASTER_CLOCK);
	m_maincpu->set_halt_on_reset(false);
	m_maincpu->set_pixel_clock(STDRES_PIXEL_CLOCK);
	m_maincpu->set_pixels_per_clock(2);
	m_maincpu->set_scanline_ind16_callback(FUNC(midyunit_base_state::scanline_update));
	m_maincpu->set_shiftreg_in_callback(FUNC(midyunit_base_state::to_shiftreg));
	m_maincpu->set_shiftreg_out_callback(FUNC(midyunit_base_state::from_shiftreg));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	PALETTE(config, m_palette).set_entries(256);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_ALWAYS_UPDATE);
	// from TMS340 registers - visible area varies slightly between games
	// we use the largest visarea (smashtv's) here so that aviwrite will work nicely
	screen.set_raw(STDRES_PIXEL_CLOCK*2, 506, 90, 500, 289, 20, 276);
	screen.set_screen_update("maincpu", FUNC(tms34010_device::tms340x0_ind16));
	screen.set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
}


void midyunit_base_state::yunit_4bpp(machine_config &config)
{
	yunit_core(config);

	// video hardware
	m_palette->set_entries(256);
	MCFG_VIDEO_START_OVERRIDE(midyunit_cvsd_state,midyunit_4bit)
}


void midyunit_base_state::yunit_6bpp(machine_config &config)
{
	yunit_core(config);

	// video hardware
	m_palette->set_entries(4096);
	MCFG_VIDEO_START_OVERRIDE(midyunit_cvsd_state,midyunit_6bit)
}


void midyunit_cvsd_state::yunit_cvsd_core(machine_config &config)
{
	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &midyunit_cvsd_state::cvsd_main_map);

	WILLIAMS_CVSD_SOUND(config, m_cvsd_sound).add_route(ALL_OUTPUTS, "speaker", 1.0);
}


void midyunit_cvsd_state::yunit_cvsd_4bit_slow(machine_config &config)
{
	yunit_4bpp(config);
	yunit_cvsd_core(config);
}


void midyunit_cvsd_state::yunit_cvsd_4bit_fast(machine_config &config)
{
	yunit_4bpp(config);
	yunit_cvsd_core(config);

	// basic machine hardware
	m_maincpu->set_clock(FAST_MASTER_CLOCK);
}


void midyunit_cvsd_state::yunit_cvsd_6bit_slow(machine_config &config)
{
	yunit_6bpp(config);
	yunit_cvsd_core(config);
}


void midyunit_adpcm_state::yunit_adpcm_core(machine_config &config)
{
	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &midyunit_adpcm_state::adpcm_main_map);

	WILLIAMS_ADPCM_SOUND(config, m_adpcm_sound).add_route(ALL_OUTPUTS, "speaker", 1.0);
}


void midyunit_adpcm_state::yunit_adpcm_6bit_fast(machine_config &config)
{
	yunit_6bpp(config);
	yunit_adpcm_core(config);

	// basic machine hardware
	m_maincpu->set_clock(FAST_MASTER_CLOCK);
}


void midyunit_adpcm_state::yunit_adpcm_6bit_faster(machine_config &config)
{
	yunit_6bpp(config);
	yunit_adpcm_core(config);

	// basic machine hardware
	m_maincpu->set_clock(FASTER_MASTER_CLOCK);
}


void term2_state::term2(machine_config &config)
{
	yunit_adpcm_6bit_faster(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &term2_state::term2_main_map);

	ADC0844(config, m_adc); // U2 on Coil Lamp Driver Board (A-14915)
	m_adc->ch1_callback().set_ioport("STICK0_X");
	m_adc->ch2_callback().set_ioport("STICK0_Y");
	m_adc->ch3_callback().set_ioport("STICK1_X");
	m_adc->ch4_callback().set_ioport("STICK1_Y");
}


void mkyawdim_state::mkyawdim(machine_config &config)
{
	yunit_core(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &mkyawdim_state::yawdim_main_map);

	Z80(config, m_audiocpu, XTAL(8'000'000) / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &mkyawdim_state::yawdim_sound_map);

	// video hardware
	m_palette->set_entries(4096);

	// sound hardware
	GENERIC_LATCH_8(config, m_soundlatch);

	OKIM6295(config, m_oki, XTAL(8'000'000) / 8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void mkyawdim_state::mkyawdim2(machine_config &config)
{
	mkyawdim(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &mkyawdim_state::yawdim2_main_map);
	m_audiocpu->set_addrmap(AS_PROGRAM, &mkyawdim_state::yawdim2_sound_map);

	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/


ROM_START( narc )
	ROM_REGION( 0x90000, "narcsnd:cpu0", 0 )    // sound CPU
	ROM_LOAD( "rev2_narc_sound_rom_u4.u4", 0x50000, 0x10000, CRC(450a591a) SHA1(bbda8061262738e5866f2707f69483a0a51d2910) )
	ROM_RELOAD(                            0x60000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u5.u5", 0x70000, 0x10000, CRC(e551e5e3) SHA1(c8b4f53dbd4c534abb77d4dc07c4d12653b79894) )
	ROM_RELOAD(                            0x80000, 0x10000 )

	ROM_REGION( 0x90000, "narcsnd:cpu1", 0 )    // slave sound CPU
	ROM_LOAD( "rev2_narc_sound_rom_u35.u35", 0x10000, 0x10000, CRC(81295892) SHA1(159664e5ee03c88d6e940e70e87e2150dc5b8b25) )
	ROM_RELOAD(                              0x20000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u36.u36", 0x30000, 0x10000, CRC(16cdbb13) SHA1(2dfd961a5d909c1804f4fda34de33ee2664c4bc6) )
	ROM_RELOAD(                              0x40000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u37.u37", 0x50000, 0x10000, CRC(29dbeffd) SHA1(4cbdc619db34f9c552de1ed3d034f8c079987e03) )
	ROM_RELOAD(                              0x60000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u38.u38", 0x70000, 0x10000, CRC(09b03b80) SHA1(a45782d29a426fac38299b56af0815e844e35ae4) )
	ROM_RELOAD(                              0x80000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "rev7_narc_game_rom_u42.u42", 0x80000, 0x20000, CRC(d1111b76) SHA1(9700261aaba6a1ac0415362874817499f90b142a) )
	ROM_LOAD16_BYTE( "rev7_narc_game_rom_u24.u24", 0x80001, 0x20000, CRC(aa0d3082) SHA1(7da59098319c49842406e7daf06aceae80fbd0ed) )
	ROM_LOAD16_BYTE( "rev7_narc_game_rom_u41.u41", 0xc0000, 0x20000, CRC(3903191f) SHA1(1ad89cb03956f6625d9403e98951383fc9219478) )
	ROM_LOAD16_BYTE( "rev7_narc_game_rom_u23.u23", 0xc0001, 0x20000, CRC(7a316582) SHA1(f640966c79bab70b536f2f92d4f46475a021b5b1) )
	// U59, U60, U77 & U78 sockets not populated

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "rev2_narc_image_rom_u94.u94", 0x000000, 0x10000, CRC(ca3194e4) SHA1(d6aa6a09e4353a1dddd502abf85acf48e6e94cef) )
	ROM_LOAD( "rev2_narc_image_rom_u93.u93", 0x010000, 0x10000, CRC(0ed7f7f5) SHA1(913d0dc81531adc6a7e6ffabfe681150aa4638a3) )
	ROM_LOAD( "rev2_narc_image_rom_u92.u92", 0x020000, 0x10000, CRC(40d2fc66) SHA1(95b8d90e6abe336ad05dc3746d02b38823d2b8cd) )
	ROM_LOAD( "rev2_narc_image_rom_u91.u91", 0x030000, 0x10000, CRC(f39325e0) SHA1(c1179825c76ed2934dfeff263a9296c2c1a5abe4) )
	ROM_LOAD( "rev2_narc_image_rom_u90.u90", 0x040000, 0x10000, CRC(0132aefa) SHA1(9bf11ebc06f1069ea056427750902c204facbd3d) )
	ROM_LOAD( "rev2_narc_image_rom_u89.u89", 0x050000, 0x10000, CRC(f7260c9e) SHA1(5a3fd88c7c0fa01ec2eb6fdef380ccee9d7da3a8) )
	ROM_LOAD( "rev2_narc_image_rom_u88.u88", 0x060000, 0x10000, CRC(edc19f42) SHA1(b7121b3df743e5744ae72de2216b679fe71a2049) )
	ROM_LOAD( "rev2_narc_image_rom_u87.u87", 0x070000, 0x10000, CRC(d9b42ff9) SHA1(cab05a5f8aadff010fba1107eb2000cc128063ff) )
	ROM_LOAD( "rev2_narc_image_rom_u86.u86", 0x080000, 0x10000, CRC(af7daad3) SHA1(e2635a0acd6a238159ef91c1c3c9dfe8de8ae18f) )
	ROM_LOAD( "rev2_narc_image_rom_u85.u85", 0x090000, 0x10000, CRC(095fae6b) SHA1(94f1df799142990a559e54cd949d9723481806b1) )
	ROM_LOAD( "rev2_narc_image_rom_u84.u84", 0x0a0000, 0x10000, CRC(3fdf2057) SHA1(25ac6263a4eb962d90a305572fb95b75cb9f4138) )
	ROM_LOAD( "rev2_narc_image_rom_u83.u83", 0x0b0000, 0x10000, CRC(f2d27c9f) SHA1(de30c7e0191adf62b11b2f2fbdf80687e653de12) )
	ROM_LOAD( "rev2_narc_image_rom_u82.u82", 0x0c0000, 0x10000, CRC(962ce47c) SHA1(ea32f7f58a5ec1d941b372db5378d14fd850a2a7) )
	ROM_LOAD( "rev2_narc_image_rom_u81.u81", 0x0d0000, 0x10000, CRC(00fe59ec) SHA1(85efd623b9cd75b249e19b2e97440a47718da728) )
	ROM_LOAD( "rev2_narc_image_rom_u80.u80", 0x0e0000, 0x10000, CRC(147ba8e9) SHA1(1065b57082e0198025fe6f0bb3548f37c6a715e4) )
	// U79 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u76.u76", 0x200000, 0x10000, CRC(1cd897f4) SHA1(80414c3718ac6719abcca83f483302fc16fcfef3) )
	ROM_LOAD( "rev2_narc_image_rom_u75.u75", 0x210000, 0x10000, CRC(78abfa01) SHA1(1523f537491b901f2d987d4443077b92e24b969d) )
	ROM_LOAD( "rev2_narc_image_rom_u74.u74", 0x220000, 0x10000, CRC(66d2a234) SHA1(290b3051fa9d35e24a9d00fcc2b72d2751f3e7f1) )
	ROM_LOAD( "rev2_narc_image_rom_u73.u73", 0x230000, 0x10000, CRC(efa5cd4e) SHA1(7aca6058d644a025c6799d55ffa082ba8eb5d76f) )
	ROM_LOAD( "rev2_narc_image_rom_u72.u72", 0x240000, 0x10000, CRC(70638eb5) SHA1(fbafb354fca7c3c402be5073fa03060de569f536) )
	ROM_LOAD( "rev2_narc_image_rom_u71.u71", 0x250000, 0x10000, CRC(61226883) SHA1(09a366df0603cc0afc8c6c5547ec6ae3a02724b2) )
	ROM_LOAD( "rev2_narc_image_rom_u70.u70", 0x260000, 0x10000, CRC(c808849f) SHA1(bd3f69c4641331738e415d6d72fafe0eeeb2e56b) )
	ROM_LOAD( "rev2_narc_image_rom_u69.u69", 0x270000, 0x10000, CRC(e7f9c34f) SHA1(f65aed012f1d575a63690222b8c8f2c56bc196c3) )
	ROM_LOAD( "rev2_narc_image_rom_u68.u68", 0x280000, 0x10000, CRC(88a634d5) SHA1(9ddf86ca8cd91965348bc311cc722151f831db21) )
	ROM_LOAD( "rev2_narc_image_rom_u67.u67", 0x290000, 0x10000, CRC(4ab8b69e) SHA1(4320407c78864edc7876ad3604405414a3e7762d) )
	ROM_LOAD( "rev2_narc_image_rom_u66.u66", 0x2a0000, 0x10000, CRC(e1da4b25) SHA1(c81ed1ffc0a4bf64e794a1313559453f9455c312) )
	ROM_LOAD( "rev2_narc_image_rom_u65.u65", 0x2b0000, 0x10000, CRC(6df0d125) SHA1(37392cc917e73cfa09970fd24503b45ced399976) )
	ROM_LOAD( "rev2_narc_image_rom_u64.u64", 0x2c0000, 0x10000, CRC(abab1b16) SHA1(2913a94e1fcf8df52e29d0fb6e373aa64d23c019) )
	ROM_LOAD( "rev2_narc_image_rom_u63.u63", 0x2d0000, 0x10000, CRC(80602f31) SHA1(f1c5c4476dbf80382f33c0776c103cff9bed8346) )
	ROM_LOAD( "rev2_narc_image_rom_u62.u62", 0x2e0000, 0x10000, CRC(c2a476d1) SHA1(ffde1784548050d87f1404aaca3689417e6f7a81) )
	// U61 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u58.u58", 0x400000, 0x10000, CRC(8a7501e3) SHA1(dcd87c464fcb88180cc1c24ec82586440a197a5c) )
	ROM_LOAD( "rev2_narc_image_rom_u57.u57", 0x410000, 0x10000, CRC(a504735f) SHA1(2afe58e576eea2e0326c6b42adb621358a270881) )
	ROM_LOAD( "rev2_narc_image_rom_u56.u56", 0x420000, 0x10000, CRC(55f8cca7) SHA1(0b0a0d50be4401e4ac4e75d8040f18540f9ddc61) )
	ROM_LOAD( "rev2_narc_image_rom_u55.u55", 0x430000, 0x10000, CRC(d3c932c1) SHA1(1a7ffc04e796ba355506bf9037c21aef18fe01a3) )
	ROM_LOAD( "rev2_narc_image_rom_u54.u54", 0x440000, 0x10000, CRC(c7f4134b) SHA1(aea523e17f95c27d1f2c1f69884f626d96c8cb3b) )
	ROM_LOAD( "rev2_narc_image_rom_u53.u53", 0x450000, 0x10000, CRC(6be4da56) SHA1(35a93a259be04a644ca70df4922f6915274c3932) )
	ROM_LOAD( "rev2_narc_image_rom_u52.u52", 0x460000, 0x10000, CRC(1ea36a4a) SHA1(78e5437d46c1ecff5e221bc301925b10f00c5269) )
	ROM_LOAD( "rev2_narc_image_rom_u51.u51", 0x470000, 0x10000, CRC(9d4b0324) SHA1(80fb38a9ac81a0383112df680b9755d7cccbd50b) )
	ROM_LOAD( "rev2_narc_image_rom_u50.u50", 0x480000, 0x10000, CRC(6f9f0c26) SHA1(be77d99fb37fa31c3824725b28ee74206c584b90) )
	ROM_LOAD( "rev2_narc_image_rom_u49.u49", 0x490000, 0x10000, CRC(80386fce) SHA1(f182ed0f1a3753dedc56cb120cb8d10e1556e966) )
	ROM_LOAD( "rev2_narc_image_rom_u48.u48", 0x4a0000, 0x10000, CRC(05c16185) SHA1(429910c5b1f1fe47fdec6cfcba765ee9f10749f0) )
	ROM_LOAD( "rev2_narc_image_rom_u47.u47", 0x4b0000, 0x10000, CRC(4c0151f1) SHA1(b526066fc594f3ec83bb4866986e3b73cdae3992) )
	ROM_LOAD( "rev2_narc_image_rom_u46.u46", 0x4c0000, 0x10000, CRC(5670bfcb) SHA1(b20829b715c6421894c10c02aebb08d22b5109c9) )
	ROM_LOAD( "rev2_narc_image_rom_u45.u45", 0x4d0000, 0x10000, CRC(27f10d98) SHA1(b027ade2b4a52977d9c40c9549b9067d37fab41c) )
	ROM_LOAD( "rev2_narc_image_rom_u44.u44", 0x4e0000, 0x10000, CRC(93b8eaa4) SHA1(b786f3286c5443cf08e556e9fb030b3444288f3c) )
	// U43 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u40.u40", 0x600000, 0x10000, CRC(7fcaebc7) SHA1(b951d63c072d693f7dfc7e362a12513eb9bd6bab) )
	ROM_LOAD( "rev2_narc_image_rom_u39.u39", 0x610000, 0x10000, CRC(7db5cf52) SHA1(478aefc1126493378d22c857646e2fce221c7d21) )
	ROM_LOAD( "rev2_narc_image_rom_u38.u38", 0x620000, 0x10000, CRC(3f9f3ef7) SHA1(5315e8c372bb63d95f814d8eafe0f41e4d95ba1a) )
	ROM_LOAD( "rev2_narc_image_rom_u37.u37", 0x630000, 0x10000, CRC(ed81826c) SHA1(afe1c0fc692a802279c1f7f31143d33028d35ce4) )
	ROM_LOAD( "rev2_narc_image_rom_u36.u36", 0x640000, 0x10000, CRC(e5d855c0) SHA1(3fa0f765238ad2a27c0c65805bf56ebfbe50bf05) )
	ROM_LOAD( "rev2_narc_image_rom_u35.u35", 0x650000, 0x10000, CRC(3a7b1329) SHA1(e8b547a3b8f85cd13e12cfe0bf3949acc1486e6b) )
	ROM_LOAD( "rev2_narc_image_rom_u34.u34", 0x660000, 0x10000, CRC(fe982b0e) SHA1(a03e7e348186339fd93ce119f65e8f0ea7b7bb7a) )
	ROM_LOAD( "rev2_narc_image_rom_u33.u33", 0x670000, 0x10000, CRC(6bc7eb0f) SHA1(6964ef63d0daf1bc7fa9585567659cfc198b6cc3) )
	ROM_LOAD( "rev2_narc_image_rom_u32.u32", 0x680000, 0x10000, CRC(5875a6d3) SHA1(ae64aa786239be39c3c99bbe019bdc91003c1691) )
	ROM_LOAD( "rev2_narc_image_rom_u31.u31", 0x690000, 0x10000, CRC(2fa4b8e5) SHA1(8e4e4abd60d20e0ef955ac4b1f300cfd157e50ca) )
	ROM_LOAD( "rev2_narc_image_rom_u30.u30", 0x6a0000, 0x10000, CRC(7e4bb8ee) SHA1(7166bd56a569329e01ed0c03579a403d659a4a7b) )
	ROM_LOAD( "rev2_narc_image_rom_u29.u29", 0x6b0000, 0x10000, CRC(45136fd9) SHA1(44388e16d02a8c55fed0dbbcd842c941fa4b11b1) )
	ROM_LOAD( "rev2_narc_image_rom_u28.u28", 0x6c0000, 0x10000, CRC(d6cdac24) SHA1(d4bbe3a1be89be7d21769bfe476b50c05cd0c357) )
	ROM_LOAD( "rev2_narc_image_rom_u27.u27", 0x6d0000, 0x10000, CRC(4d33bbec) SHA1(05a3bd66ff91c824e841ca3943585f6aa383c5c2) )
	ROM_LOAD( "rev2_narc_image_rom_u26.u26", 0x6e0000, 0x10000, CRC(cb19f784) SHA1(1e4d85603c940e247fdc45f0366dfb484285e588) )
	// U25 socket not populated

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "a-5346-3036-1_narc_pls153.u28", 0x000, 0x0eb, CRC(4db36615) SHA1(781d13af2735ad3da0c7594487e7f4e19b5f459f) )
	ROM_LOAD( "a-5346-3036-2_narc_pls153.u78", 0x100, 0x0eb, CRC(4b151863) SHA1(0740c5fba978b9bb68a50472b57c0801c7591d6f) )
	ROM_LOAD( "a-5346-3036-3_narc_pls153.u79", 0x200, 0x0eb, CRC(35bd6ed8) SHA1(98b4ad4f61d2f2013815fa79c3428a2d610c8a6f) )
	ROM_LOAD( "a-5346-3036-4_narc_pls153.u80", 0x300, 0x0eb, CRC(9c10e4cf) SHA1(77d0a6a6e19e2f5b17e9356459215e5452247be3) )
	ROM_LOAD( "a-5346-3036-5_narc_pls153.u83", 0x400, 0x0eb, CRC(3a2f21b2) SHA1(bafc45b8b0e83d044bc4963abbdbbf4b6da1694b) )
ROM_END


ROM_START( narc6 )
	ROM_REGION( 0x90000, "narcsnd:cpu0", 0 )    // sound CPU
	ROM_LOAD( "rev2_narc_sound_rom_u4.u4", 0x50000, 0x10000, CRC(450a591a) SHA1(bbda8061262738e5866f2707f69483a0a51d2910) )
	ROM_RELOAD(                            0x60000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u5.u5", 0x70000, 0x10000, CRC(e551e5e3) SHA1(c8b4f53dbd4c534abb77d4dc07c4d12653b79894) )
	ROM_RELOAD(                            0x80000, 0x10000 )

	ROM_REGION( 0x90000, "narcsnd:cpu1", 0 )    // slave sound CPU
	ROM_LOAD( "rev2_narc_sound_rom_u35.u35", 0x10000, 0x10000, CRC(81295892) SHA1(159664e5ee03c88d6e940e70e87e2150dc5b8b25) )
	ROM_RELOAD(                              0x20000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u36.u36", 0x30000, 0x10000, CRC(16cdbb13) SHA1(2dfd961a5d909c1804f4fda34de33ee2664c4bc6) )
	ROM_RELOAD(                              0x40000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u37.u37", 0x50000, 0x10000, CRC(29dbeffd) SHA1(4cbdc619db34f9c552de1ed3d034f8c079987e03) )
	ROM_RELOAD(                              0x60000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u38.u38", 0x70000, 0x10000, CRC(09b03b80) SHA1(a45782d29a426fac38299b56af0815e844e35ae4) )
	ROM_RELOAD(                              0x80000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "rev6_narc_game_rom_u78.u78", 0x00000, 0x10000, CRC(2c9e799b) SHA1(28847e5aea08f4d4b12321cd2bfc13280ee8ec4f) )
	ROM_RELOAD(                                    0x20000, 0x10000 )
	ROM_LOAD16_BYTE( "rev6_narc_game_rom_u60.u60", 0x00001, 0x10000, CRC(5f6b0429) SHA1(1bda1a7644f2c7939800d64c3e73d589296032d9) )
	ROM_RELOAD(                                    0x20001, 0x10000 )
	ROM_LOAD16_BYTE( "rev6_narc_game_rom_u77.u77", 0x40000, 0x10000, CRC(508cfa38) SHA1(9b90d4bc199f9c30c38d986692829fe8ba458090) )
	ROM_RELOAD(                                    0x60000, 0x10000 )
	ROM_LOAD16_BYTE( "rev6_narc_game_rom_u59.u59", 0x40001, 0x10000, CRC(84bc91fc) SHA1(6203f1ac473f095d9a8fa9ed9081777526d7abb9) )
	ROM_RELOAD(                                    0x60001, 0x10000 )
	ROM_LOAD16_BYTE( "rev6_narc_game_rom_u42.u42", 0x80000, 0x10000, CRC(ee8ae9d4) SHA1(52721b40ff63c8e6d96ecb550e540e3d34c5d692) )
	ROM_RELOAD(                                    0xa0000, 0x10000 )
	ROM_LOAD16_BYTE( "rev6_narc_game_rom_u24.u24", 0x80001, 0x10000, CRC(4fbe2ff5) SHA1(9396018c095947f361e03196bee3dc598da57181) )
	ROM_RELOAD(                                    0xa0001, 0x10000 )
	ROM_LOAD16_BYTE( "rev6_narc_game_rom_u41.u41", 0xc0000, 0x10000, CRC(43a1bbbc) SHA1(0afe83e3c9a8a76fcadddf3d37f96f55a29fbb22) )
	ROM_RELOAD(                                    0xe0000, 0x10000 )
	ROM_LOAD16_BYTE( "rev6_narc_game_rom_u23.u23", 0xc0001, 0x10000, CRC(ed0d149d) SHA1(02b376cc7584fd26f537ad8122f73211bf0e66b8) )
	ROM_RELOAD(                                    0xe0001, 0x10000 )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "rev2_narc_image_rom_u94.u94", 0x000000, 0x10000, CRC(ca3194e4) SHA1(d6aa6a09e4353a1dddd502abf85acf48e6e94cef) )
	ROM_LOAD( "rev2_narc_image_rom_u93.u93", 0x010000, 0x10000, CRC(0ed7f7f5) SHA1(913d0dc81531adc6a7e6ffabfe681150aa4638a3) )
	ROM_LOAD( "rev2_narc_image_rom_u92.u92", 0x020000, 0x10000, CRC(40d2fc66) SHA1(95b8d90e6abe336ad05dc3746d02b38823d2b8cd) )
	ROM_LOAD( "rev2_narc_image_rom_u91.u91", 0x030000, 0x10000, CRC(f39325e0) SHA1(c1179825c76ed2934dfeff263a9296c2c1a5abe4) )
	ROM_LOAD( "rev2_narc_image_rom_u90.u90", 0x040000, 0x10000, CRC(0132aefa) SHA1(9bf11ebc06f1069ea056427750902c204facbd3d) )
	ROM_LOAD( "rev2_narc_image_rom_u89.u89", 0x050000, 0x10000, CRC(f7260c9e) SHA1(5a3fd88c7c0fa01ec2eb6fdef380ccee9d7da3a8) )
	ROM_LOAD( "rev2_narc_image_rom_u88.u88", 0x060000, 0x10000, CRC(edc19f42) SHA1(b7121b3df743e5744ae72de2216b679fe71a2049) )
	ROM_LOAD( "rev2_narc_image_rom_u87.u87", 0x070000, 0x10000, CRC(d9b42ff9) SHA1(cab05a5f8aadff010fba1107eb2000cc128063ff) )
	ROM_LOAD( "rev2_narc_image_rom_u86.u86", 0x080000, 0x10000, CRC(af7daad3) SHA1(e2635a0acd6a238159ef91c1c3c9dfe8de8ae18f) )
	ROM_LOAD( "rev2_narc_image_rom_u85.u85", 0x090000, 0x10000, CRC(095fae6b) SHA1(94f1df799142990a559e54cd949d9723481806b1) )
	ROM_LOAD( "rev2_narc_image_rom_u84.u84", 0x0a0000, 0x10000, CRC(3fdf2057) SHA1(25ac6263a4eb962d90a305572fb95b75cb9f4138) )
	ROM_LOAD( "rev2_narc_image_rom_u83.u83", 0x0b0000, 0x10000, CRC(f2d27c9f) SHA1(de30c7e0191adf62b11b2f2fbdf80687e653de12) )
	ROM_LOAD( "rev2_narc_image_rom_u82.u82", 0x0c0000, 0x10000, CRC(962ce47c) SHA1(ea32f7f58a5ec1d941b372db5378d14fd850a2a7) )
	ROM_LOAD( "rev2_narc_image_rom_u81.u81", 0x0d0000, 0x10000, CRC(00fe59ec) SHA1(85efd623b9cd75b249e19b2e97440a47718da728) )
	ROM_LOAD( "rev2_narc_image_rom_u80.u80", 0x0e0000, 0x10000, CRC(147ba8e9) SHA1(1065b57082e0198025fe6f0bb3548f37c6a715e4) )
	// U79 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u76.u76", 0x200000, 0x10000, CRC(1cd897f4) SHA1(80414c3718ac6719abcca83f483302fc16fcfef3) )
	ROM_LOAD( "rev2_narc_image_rom_u75.u75", 0x210000, 0x10000, CRC(78abfa01) SHA1(1523f537491b901f2d987d4443077b92e24b969d) )
	ROM_LOAD( "rev2_narc_image_rom_u74.u74", 0x220000, 0x10000, CRC(66d2a234) SHA1(290b3051fa9d35e24a9d00fcc2b72d2751f3e7f1) )
	ROM_LOAD( "rev2_narc_image_rom_u73.u73", 0x230000, 0x10000, CRC(efa5cd4e) SHA1(7aca6058d644a025c6799d55ffa082ba8eb5d76f) )
	ROM_LOAD( "rev2_narc_image_rom_u72.u72", 0x240000, 0x10000, CRC(70638eb5) SHA1(fbafb354fca7c3c402be5073fa03060de569f536) )
	ROM_LOAD( "rev2_narc_image_rom_u71.u71", 0x250000, 0x10000, CRC(61226883) SHA1(09a366df0603cc0afc8c6c5547ec6ae3a02724b2) )
	ROM_LOAD( "rev2_narc_image_rom_u70.u70", 0x260000, 0x10000, CRC(c808849f) SHA1(bd3f69c4641331738e415d6d72fafe0eeeb2e56b) )
	ROM_LOAD( "rev2_narc_image_rom_u69.u69", 0x270000, 0x10000, CRC(e7f9c34f) SHA1(f65aed012f1d575a63690222b8c8f2c56bc196c3) )
	ROM_LOAD( "rev2_narc_image_rom_u68.u68", 0x280000, 0x10000, CRC(88a634d5) SHA1(9ddf86ca8cd91965348bc311cc722151f831db21) )
	ROM_LOAD( "rev2_narc_image_rom_u67.u67", 0x290000, 0x10000, CRC(4ab8b69e) SHA1(4320407c78864edc7876ad3604405414a3e7762d) )
	ROM_LOAD( "rev2_narc_image_rom_u66.u66", 0x2a0000, 0x10000, CRC(e1da4b25) SHA1(c81ed1ffc0a4bf64e794a1313559453f9455c312) )
	ROM_LOAD( "rev2_narc_image_rom_u65.u65", 0x2b0000, 0x10000, CRC(6df0d125) SHA1(37392cc917e73cfa09970fd24503b45ced399976) )
	ROM_LOAD( "rev2_narc_image_rom_u64.u64", 0x2c0000, 0x10000, CRC(abab1b16) SHA1(2913a94e1fcf8df52e29d0fb6e373aa64d23c019) )
	ROM_LOAD( "rev2_narc_image_rom_u63.u63", 0x2d0000, 0x10000, CRC(80602f31) SHA1(f1c5c4476dbf80382f33c0776c103cff9bed8346) )
	ROM_LOAD( "rev2_narc_image_rom_u62.u62", 0x2e0000, 0x10000, CRC(c2a476d1) SHA1(ffde1784548050d87f1404aaca3689417e6f7a81) )
	// U61 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u58.u58", 0x400000, 0x10000, CRC(8a7501e3) SHA1(dcd87c464fcb88180cc1c24ec82586440a197a5c) )
	ROM_LOAD( "rev2_narc_image_rom_u57.u57", 0x410000, 0x10000, CRC(a504735f) SHA1(2afe58e576eea2e0326c6b42adb621358a270881) )
	ROM_LOAD( "rev2_narc_image_rom_u56.u56", 0x420000, 0x10000, CRC(55f8cca7) SHA1(0b0a0d50be4401e4ac4e75d8040f18540f9ddc61) )
	ROM_LOAD( "rev2_narc_image_rom_u55.u55", 0x430000, 0x10000, CRC(d3c932c1) SHA1(1a7ffc04e796ba355506bf9037c21aef18fe01a3) )
	ROM_LOAD( "rev2_narc_image_rom_u54.u54", 0x440000, 0x10000, CRC(c7f4134b) SHA1(aea523e17f95c27d1f2c1f69884f626d96c8cb3b) )
	ROM_LOAD( "rev2_narc_image_rom_u53.u53", 0x450000, 0x10000, CRC(6be4da56) SHA1(35a93a259be04a644ca70df4922f6915274c3932) )
	ROM_LOAD( "rev2_narc_image_rom_u52.u52", 0x460000, 0x10000, CRC(1ea36a4a) SHA1(78e5437d46c1ecff5e221bc301925b10f00c5269) )
	ROM_LOAD( "rev2_narc_image_rom_u51.u51", 0x470000, 0x10000, CRC(9d4b0324) SHA1(80fb38a9ac81a0383112df680b9755d7cccbd50b) )
	ROM_LOAD( "rev2_narc_image_rom_u50.u50", 0x480000, 0x10000, CRC(6f9f0c26) SHA1(be77d99fb37fa31c3824725b28ee74206c584b90) )
	ROM_LOAD( "rev2_narc_image_rom_u49.u49", 0x490000, 0x10000, CRC(80386fce) SHA1(f182ed0f1a3753dedc56cb120cb8d10e1556e966) )
	ROM_LOAD( "rev2_narc_image_rom_u48.u48", 0x4a0000, 0x10000, CRC(05c16185) SHA1(429910c5b1f1fe47fdec6cfcba765ee9f10749f0) )
	ROM_LOAD( "rev2_narc_image_rom_u47.u47", 0x4b0000, 0x10000, CRC(4c0151f1) SHA1(b526066fc594f3ec83bb4866986e3b73cdae3992) )
	ROM_LOAD( "rev2_narc_image_rom_u46.u46", 0x4c0000, 0x10000, CRC(5670bfcb) SHA1(b20829b715c6421894c10c02aebb08d22b5109c9) )
	ROM_LOAD( "rev2_narc_image_rom_u45.u45", 0x4d0000, 0x10000, CRC(27f10d98) SHA1(b027ade2b4a52977d9c40c9549b9067d37fab41c) )
	ROM_LOAD( "rev2_narc_image_rom_u44.u44", 0x4e0000, 0x10000, CRC(93b8eaa4) SHA1(b786f3286c5443cf08e556e9fb030b3444288f3c) )
	// U43 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u40.u40", 0x600000, 0x10000, CRC(7fcaebc7) SHA1(b951d63c072d693f7dfc7e362a12513eb9bd6bab) )
	ROM_LOAD( "rev2_narc_image_rom_u39.u39", 0x610000, 0x10000, CRC(7db5cf52) SHA1(478aefc1126493378d22c857646e2fce221c7d21) )
	ROM_LOAD( "rev2_narc_image_rom_u38.u38", 0x620000, 0x10000, CRC(3f9f3ef7) SHA1(5315e8c372bb63d95f814d8eafe0f41e4d95ba1a) )
	ROM_LOAD( "rev2_narc_image_rom_u37.u37", 0x630000, 0x10000, CRC(ed81826c) SHA1(afe1c0fc692a802279c1f7f31143d33028d35ce4) )
	ROM_LOAD( "rev2_narc_image_rom_u36.u36", 0x640000, 0x10000, CRC(e5d855c0) SHA1(3fa0f765238ad2a27c0c65805bf56ebfbe50bf05) )
	ROM_LOAD( "rev2_narc_image_rom_u35.u35", 0x650000, 0x10000, CRC(3a7b1329) SHA1(e8b547a3b8f85cd13e12cfe0bf3949acc1486e6b) )
	ROM_LOAD( "rev2_narc_image_rom_u34.u34", 0x660000, 0x10000, CRC(fe982b0e) SHA1(a03e7e348186339fd93ce119f65e8f0ea7b7bb7a) )
	ROM_LOAD( "rev2_narc_image_rom_u33.u33", 0x670000, 0x10000, CRC(6bc7eb0f) SHA1(6964ef63d0daf1bc7fa9585567659cfc198b6cc3) )
	ROM_LOAD( "rev2_narc_image_rom_u32.u32", 0x680000, 0x10000, CRC(5875a6d3) SHA1(ae64aa786239be39c3c99bbe019bdc91003c1691) )
	ROM_LOAD( "rev2_narc_image_rom_u31.u31", 0x690000, 0x10000, CRC(2fa4b8e5) SHA1(8e4e4abd60d20e0ef955ac4b1f300cfd157e50ca) )
	ROM_LOAD( "rev2_narc_image_rom_u30.u30", 0x6a0000, 0x10000, CRC(7e4bb8ee) SHA1(7166bd56a569329e01ed0c03579a403d659a4a7b) )
	ROM_LOAD( "rev2_narc_image_rom_u29.u29", 0x6b0000, 0x10000, CRC(45136fd9) SHA1(44388e16d02a8c55fed0dbbcd842c941fa4b11b1) )
	ROM_LOAD( "rev2_narc_image_rom_u28.u28", 0x6c0000, 0x10000, CRC(d6cdac24) SHA1(d4bbe3a1be89be7d21769bfe476b50c05cd0c357) )
	ROM_LOAD( "rev2_narc_image_rom_u27.u27", 0x6d0000, 0x10000, CRC(4d33bbec) SHA1(05a3bd66ff91c824e841ca3943585f6aa383c5c2) )
	ROM_LOAD( "rev2_narc_image_rom_u26.u26", 0x6e0000, 0x10000, CRC(cb19f784) SHA1(1e4d85603c940e247fdc45f0366dfb484285e588) )
	// U25 socket not populated

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "a-5346-3036-1_narc_pls153.u28", 0x000, 0x0eb, CRC(4db36615) SHA1(781d13af2735ad3da0c7594487e7f4e19b5f459f) )
	ROM_LOAD( "a-5346-3036-2_narc_pls153.u78", 0x100, 0x0eb, CRC(4b151863) SHA1(0740c5fba978b9bb68a50472b57c0801c7591d6f) )
	ROM_LOAD( "a-5346-3036-3_narc_pls153.u79", 0x200, 0x0eb, CRC(35bd6ed8) SHA1(98b4ad4f61d2f2013815fa79c3428a2d610c8a6f) )
	ROM_LOAD( "a-5346-3036-4_narc_pls153.u80", 0x300, 0x0eb, CRC(9c10e4cf) SHA1(77d0a6a6e19e2f5b17e9356459215e5452247be3) )
	ROM_LOAD( "a-5346-3036-5_narc_pls153.u83", 0x400, 0x0eb, CRC(3a2f21b2) SHA1(bafc45b8b0e83d044bc4963abbdbbf4b6da1694b) )
ROM_END


ROM_START( narc4 )
	ROM_REGION( 0x90000, "narcsnd:cpu0", 0 )    // sound CPU
	ROM_LOAD( "rev2_narc_sound_rom_u4.u4", 0x50000, 0x10000, CRC(450a591a) SHA1(bbda8061262738e5866f2707f69483a0a51d2910) )
	ROM_RELOAD(                            0x60000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u5.u5", 0x70000, 0x10000, CRC(e551e5e3) SHA1(c8b4f53dbd4c534abb77d4dc07c4d12653b79894) )
	ROM_RELOAD(                            0x80000, 0x10000 )

	ROM_REGION( 0x90000, "narcsnd:cpu1", 0 )    // slave sound CPU
	ROM_LOAD( "rev2_narc_sound_rom_u35.u35", 0x10000, 0x10000, CRC(81295892) SHA1(159664e5ee03c88d6e940e70e87e2150dc5b8b25) )
	ROM_RELOAD(                              0x20000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u36.u36", 0x30000, 0x10000, CRC(16cdbb13) SHA1(2dfd961a5d909c1804f4fda34de33ee2664c4bc6) )
	ROM_RELOAD(                              0x40000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u37.u37", 0x50000, 0x10000, CRC(29dbeffd) SHA1(4cbdc619db34f9c552de1ed3d034f8c079987e03) )
	ROM_RELOAD(                              0x60000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u38.u38", 0x70000, 0x10000, CRC(09b03b80) SHA1(a45782d29a426fac38299b56af0815e844e35ae4) )
	ROM_RELOAD(                              0x80000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "rev4_narc_game_rom_u78.u78", 0x00000, 0x10000, CRC(99bbd587) SHA1(716c446d1aceacbc608883dde499ef6de23847f2) )
	ROM_RELOAD(                                    0x20000, 0x10000 )
	ROM_LOAD16_BYTE( "rev4_narc_game_rom_u60.u60", 0x00001, 0x10000, CRC(beec5f1a) SHA1(aaf900820a1722c19607642ffc2acdedab5cc0da) )
	ROM_RELOAD(                                    0x20001, 0x10000 )
	ROM_LOAD16_BYTE( "rev4_narc_game_rom_u77.u77", 0x40000, 0x10000, CRC(0b9bdd76) SHA1(8bcc330e99b0694fe243470123f360185b91e20b) )
	ROM_RELOAD(                                    0x60000, 0x10000 )
	ROM_LOAD16_BYTE( "rev4_narc_game_rom_u59.u59", 0x40001, 0x10000, CRC(0169e4c3) SHA1(859cf38ec982f26bdfe23a7f99f0b0c7f1329171) )
	ROM_RELOAD(                                    0x60001, 0x10000 )
	ROM_LOAD16_BYTE( "rev4_narc_game_rom_u42.u42", 0x80000, 0x10000, CRC(a7b0347d) SHA1(6c7e46003a2925a3b99b77a40f207ed1522b5f65) )
	ROM_RELOAD(                                    0xa0000, 0x10000 )
	ROM_LOAD16_BYTE( "rev4_narc_game_rom_u24.u24", 0x80001, 0x10000, CRC(613c9f54) SHA1(e7305b3ad672eef8aeb228cb11ed2b958bdf1d2e) )
	ROM_RELOAD(                                    0xa0001, 0x10000 )
	ROM_LOAD16_BYTE( "rev4_narc_game_rom_u41.u41", 0xc0000, 0x10000, CRC(80e83440) SHA1(f26f8cddaf82909ef0f73aa568720ec6b44671ac) )
	ROM_RELOAD(                                    0xe0000, 0x10000 )
	ROM_LOAD16_BYTE( "rev4_narc_game_rom_u23.u23", 0xc0001, 0x10000, CRC(425a3f8f) SHA1(d884cb40816ae9f0c1b7b536a88d6fa36488a004) )
	ROM_RELOAD(                                    0xe0001, 0x10000 )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "rev2_narc_image_rom_u94.u94", 0x000000, 0x10000, CRC(ca3194e4) SHA1(d6aa6a09e4353a1dddd502abf85acf48e6e94cef) )
	ROM_LOAD( "rev2_narc_image_rom_u93.u93", 0x010000, 0x10000, CRC(0ed7f7f5) SHA1(913d0dc81531adc6a7e6ffabfe681150aa4638a3) )
	ROM_LOAD( "rev2_narc_image_rom_u92.u92", 0x020000, 0x10000, CRC(40d2fc66) SHA1(95b8d90e6abe336ad05dc3746d02b38823d2b8cd) )
	ROM_LOAD( "rev2_narc_image_rom_u91.u91", 0x030000, 0x10000, CRC(f39325e0) SHA1(c1179825c76ed2934dfeff263a9296c2c1a5abe4) )
	ROM_LOAD( "rev2_narc_image_rom_u90.u90", 0x040000, 0x10000, CRC(0132aefa) SHA1(9bf11ebc06f1069ea056427750902c204facbd3d) )
	ROM_LOAD( "rev2_narc_image_rom_u89.u89", 0x050000, 0x10000, CRC(f7260c9e) SHA1(5a3fd88c7c0fa01ec2eb6fdef380ccee9d7da3a8) )
	ROM_LOAD( "rev2_narc_image_rom_u88.u88", 0x060000, 0x10000, CRC(edc19f42) SHA1(b7121b3df743e5744ae72de2216b679fe71a2049) )
	ROM_LOAD( "rev2_narc_image_rom_u87.u87", 0x070000, 0x10000, CRC(d9b42ff9) SHA1(cab05a5f8aadff010fba1107eb2000cc128063ff) )
	ROM_LOAD( "rev2_narc_image_rom_u86.u86", 0x080000, 0x10000, CRC(af7daad3) SHA1(e2635a0acd6a238159ef91c1c3c9dfe8de8ae18f) )
	ROM_LOAD( "rev2_narc_image_rom_u85.u85", 0x090000, 0x10000, CRC(095fae6b) SHA1(94f1df799142990a559e54cd949d9723481806b1) )
	ROM_LOAD( "rev2_narc_image_rom_u84.u84", 0x0a0000, 0x10000, CRC(3fdf2057) SHA1(25ac6263a4eb962d90a305572fb95b75cb9f4138) )
	ROM_LOAD( "rev2_narc_image_rom_u83.u83", 0x0b0000, 0x10000, CRC(f2d27c9f) SHA1(de30c7e0191adf62b11b2f2fbdf80687e653de12) )
	ROM_LOAD( "rev2_narc_image_rom_u82.u82", 0x0c0000, 0x10000, CRC(962ce47c) SHA1(ea32f7f58a5ec1d941b372db5378d14fd850a2a7) )
	ROM_LOAD( "rev2_narc_image_rom_u81.u81", 0x0d0000, 0x10000, CRC(00fe59ec) SHA1(85efd623b9cd75b249e19b2e97440a47718da728) )
	ROM_LOAD( "rev2_narc_image_rom_u80.u80", 0x0e0000, 0x10000, CRC(147ba8e9) SHA1(1065b57082e0198025fe6f0bb3548f37c6a715e4) )
	// U79 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u76.u76", 0x200000, 0x10000, CRC(1cd897f4) SHA1(80414c3718ac6719abcca83f483302fc16fcfef3) )
	ROM_LOAD( "rev2_narc_image_rom_u75.u75", 0x210000, 0x10000, CRC(78abfa01) SHA1(1523f537491b901f2d987d4443077b92e24b969d) )
	ROM_LOAD( "rev2_narc_image_rom_u74.u74", 0x220000, 0x10000, CRC(66d2a234) SHA1(290b3051fa9d35e24a9d00fcc2b72d2751f3e7f1) )
	ROM_LOAD( "rev2_narc_image_rom_u73.u73", 0x230000, 0x10000, CRC(efa5cd4e) SHA1(7aca6058d644a025c6799d55ffa082ba8eb5d76f) )
	ROM_LOAD( "rev2_narc_image_rom_u72.u72", 0x240000, 0x10000, CRC(70638eb5) SHA1(fbafb354fca7c3c402be5073fa03060de569f536) )
	ROM_LOAD( "rev2_narc_image_rom_u71.u71", 0x250000, 0x10000, CRC(61226883) SHA1(09a366df0603cc0afc8c6c5547ec6ae3a02724b2) )
	ROM_LOAD( "rev2_narc_image_rom_u70.u70", 0x260000, 0x10000, CRC(c808849f) SHA1(bd3f69c4641331738e415d6d72fafe0eeeb2e56b) )
	ROM_LOAD( "rev2_narc_image_rom_u69.u69", 0x270000, 0x10000, CRC(e7f9c34f) SHA1(f65aed012f1d575a63690222b8c8f2c56bc196c3) )
	ROM_LOAD( "rev2_narc_image_rom_u68.u68", 0x280000, 0x10000, CRC(88a634d5) SHA1(9ddf86ca8cd91965348bc311cc722151f831db21) )
	ROM_LOAD( "rev2_narc_image_rom_u67.u67", 0x290000, 0x10000, CRC(4ab8b69e) SHA1(4320407c78864edc7876ad3604405414a3e7762d) )
	ROM_LOAD( "rev2_narc_image_rom_u66.u66", 0x2a0000, 0x10000, CRC(e1da4b25) SHA1(c81ed1ffc0a4bf64e794a1313559453f9455c312) )
	ROM_LOAD( "rev2_narc_image_rom_u65.u65", 0x2b0000, 0x10000, CRC(6df0d125) SHA1(37392cc917e73cfa09970fd24503b45ced399976) )
	ROM_LOAD( "rev2_narc_image_rom_u64.u64", 0x2c0000, 0x10000, CRC(abab1b16) SHA1(2913a94e1fcf8df52e29d0fb6e373aa64d23c019) )
	ROM_LOAD( "rev2_narc_image_rom_u63.u63", 0x2d0000, 0x10000, CRC(80602f31) SHA1(f1c5c4476dbf80382f33c0776c103cff9bed8346) )
	ROM_LOAD( "rev2_narc_image_rom_u62.u62", 0x2e0000, 0x10000, CRC(c2a476d1) SHA1(ffde1784548050d87f1404aaca3689417e6f7a81) )
	// U61 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u58.u58", 0x400000, 0x10000, CRC(8a7501e3) SHA1(dcd87c464fcb88180cc1c24ec82586440a197a5c) )
	ROM_LOAD( "rev2_narc_image_rom_u57.u57", 0x410000, 0x10000, CRC(a504735f) SHA1(2afe58e576eea2e0326c6b42adb621358a270881) )
	ROM_LOAD( "rev2_narc_image_rom_u56.u56", 0x420000, 0x10000, CRC(55f8cca7) SHA1(0b0a0d50be4401e4ac4e75d8040f18540f9ddc61) )
	ROM_LOAD( "rev2_narc_image_rom_u55.u55", 0x430000, 0x10000, CRC(d3c932c1) SHA1(1a7ffc04e796ba355506bf9037c21aef18fe01a3) )
	ROM_LOAD( "rev2_narc_image_rom_u54.u54", 0x440000, 0x10000, CRC(c7f4134b) SHA1(aea523e17f95c27d1f2c1f69884f626d96c8cb3b) )
	ROM_LOAD( "rev2_narc_image_rom_u53.u53", 0x450000, 0x10000, CRC(6be4da56) SHA1(35a93a259be04a644ca70df4922f6915274c3932) )
	ROM_LOAD( "rev2_narc_image_rom_u52.u52", 0x460000, 0x10000, CRC(1ea36a4a) SHA1(78e5437d46c1ecff5e221bc301925b10f00c5269) )
	ROM_LOAD( "rev2_narc_image_rom_u51.u51", 0x470000, 0x10000, CRC(9d4b0324) SHA1(80fb38a9ac81a0383112df680b9755d7cccbd50b) )
	ROM_LOAD( "rev2_narc_image_rom_u50.u50", 0x480000, 0x10000, CRC(6f9f0c26) SHA1(be77d99fb37fa31c3824725b28ee74206c584b90) )
	ROM_LOAD( "rev2_narc_image_rom_u49.u49", 0x490000, 0x10000, CRC(80386fce) SHA1(f182ed0f1a3753dedc56cb120cb8d10e1556e966) )
	ROM_LOAD( "rev2_narc_image_rom_u48.u48", 0x4a0000, 0x10000, CRC(05c16185) SHA1(429910c5b1f1fe47fdec6cfcba765ee9f10749f0) )
	ROM_LOAD( "rev2_narc_image_rom_u47.u47", 0x4b0000, 0x10000, CRC(4c0151f1) SHA1(b526066fc594f3ec83bb4866986e3b73cdae3992) )
	ROM_LOAD( "rev2_narc_image_rom_u46.u46", 0x4c0000, 0x10000, CRC(5670bfcb) SHA1(b20829b715c6421894c10c02aebb08d22b5109c9) )
	ROM_LOAD( "rev2_narc_image_rom_u45.u45", 0x4d0000, 0x10000, CRC(27f10d98) SHA1(b027ade2b4a52977d9c40c9549b9067d37fab41c) )
	ROM_LOAD( "rev2_narc_image_rom_u44.u44", 0x4e0000, 0x10000, CRC(93b8eaa4) SHA1(b786f3286c5443cf08e556e9fb030b3444288f3c) )
	// U43 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u40.u40", 0x600000, 0x10000, CRC(7fcaebc7) SHA1(b951d63c072d693f7dfc7e362a12513eb9bd6bab) )
	ROM_LOAD( "rev2_narc_image_rom_u39.u39", 0x610000, 0x10000, CRC(7db5cf52) SHA1(478aefc1126493378d22c857646e2fce221c7d21) )
	ROM_LOAD( "rev2_narc_image_rom_u38.u38", 0x620000, 0x10000, CRC(3f9f3ef7) SHA1(5315e8c372bb63d95f814d8eafe0f41e4d95ba1a) )
	ROM_LOAD( "rev2_narc_image_rom_u37.u37", 0x630000, 0x10000, CRC(ed81826c) SHA1(afe1c0fc692a802279c1f7f31143d33028d35ce4) )
	ROM_LOAD( "rev2_narc_image_rom_u36.u36", 0x640000, 0x10000, CRC(e5d855c0) SHA1(3fa0f765238ad2a27c0c65805bf56ebfbe50bf05) )
	ROM_LOAD( "rev2_narc_image_rom_u35.u35", 0x650000, 0x10000, CRC(3a7b1329) SHA1(e8b547a3b8f85cd13e12cfe0bf3949acc1486e6b) )
	ROM_LOAD( "rev2_narc_image_rom_u34.u34", 0x660000, 0x10000, CRC(fe982b0e) SHA1(a03e7e348186339fd93ce119f65e8f0ea7b7bb7a) )
	ROM_LOAD( "rev2_narc_image_rom_u33.u33", 0x670000, 0x10000, CRC(6bc7eb0f) SHA1(6964ef63d0daf1bc7fa9585567659cfc198b6cc3) )
	ROM_LOAD( "rev2_narc_image_rom_u32.u32", 0x680000, 0x10000, CRC(5875a6d3) SHA1(ae64aa786239be39c3c99bbe019bdc91003c1691) )
	ROM_LOAD( "rev2_narc_image_rom_u31.u31", 0x690000, 0x10000, CRC(2fa4b8e5) SHA1(8e4e4abd60d20e0ef955ac4b1f300cfd157e50ca) )
	ROM_LOAD( "rev2_narc_image_rom_u30.u30", 0x6a0000, 0x10000, CRC(7e4bb8ee) SHA1(7166bd56a569329e01ed0c03579a403d659a4a7b) )
	ROM_LOAD( "rev2_narc_image_rom_u29.u29", 0x6b0000, 0x10000, CRC(45136fd9) SHA1(44388e16d02a8c55fed0dbbcd842c941fa4b11b1) )
	ROM_LOAD( "rev2_narc_image_rom_u28.u28", 0x6c0000, 0x10000, CRC(d6cdac24) SHA1(d4bbe3a1be89be7d21769bfe476b50c05cd0c357) )
	ROM_LOAD( "rev2_narc_image_rom_u27.u27", 0x6d0000, 0x10000, CRC(4d33bbec) SHA1(05a3bd66ff91c824e841ca3943585f6aa383c5c2) )
	ROM_LOAD( "rev2_narc_image_rom_u26.u26", 0x6e0000, 0x10000, CRC(cb19f784) SHA1(1e4d85603c940e247fdc45f0366dfb484285e588) )
	// U25 socket not populated

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "a-5346-3036-1_narc_pls153.u28", 0x000, 0x0eb, CRC(4db36615) SHA1(781d13af2735ad3da0c7594487e7f4e19b5f459f) )
	ROM_LOAD( "a-5346-3036-2_narc_pls153.u78", 0x100, 0x0eb, CRC(4b151863) SHA1(0740c5fba978b9bb68a50472b57c0801c7591d6f) )
	ROM_LOAD( "a-5346-3036-3_narc_pls153.u79", 0x200, 0x0eb, CRC(35bd6ed8) SHA1(98b4ad4f61d2f2013815fa79c3428a2d610c8a6f) )
	ROM_LOAD( "a-5346-3036-4_narc_pls153.u80", 0x300, 0x0eb, CRC(9c10e4cf) SHA1(77d0a6a6e19e2f5b17e9356459215e5452247be3) )
	ROM_LOAD( "a-5346-3036-5_narc_pls153.u83", 0x400, 0x0eb, CRC(3a2f21b2) SHA1(bafc45b8b0e83d044bc4963abbdbbf4b6da1694b) )
ROM_END


ROM_START( narc3 )
	ROM_REGION( 0x90000, "narcsnd:cpu0", 0 )    // sound CPU
	ROM_LOAD( "rev2_narc_sound_rom_u4.u4", 0x50000, 0x10000, CRC(450a591a) SHA1(bbda8061262738e5866f2707f69483a0a51d2910) )
	ROM_RELOAD(                            0x60000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u5.u5", 0x70000, 0x10000, CRC(e551e5e3) SHA1(c8b4f53dbd4c534abb77d4dc07c4d12653b79894) )
	ROM_RELOAD(                            0x80000, 0x10000 )

	ROM_REGION( 0x90000, "narcsnd:cpu1", 0 )    // slave sound CPU
	ROM_LOAD( "rev2_narc_sound_rom_u35.u35", 0x10000, 0x10000, CRC(81295892) SHA1(159664e5ee03c88d6e940e70e87e2150dc5b8b25) )
	ROM_RELOAD(                              0x20000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u36.u36", 0x30000, 0x10000, CRC(16cdbb13) SHA1(2dfd961a5d909c1804f4fda34de33ee2664c4bc6) )
	ROM_RELOAD(                              0x40000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u37.u37", 0x50000, 0x10000, CRC(29dbeffd) SHA1(4cbdc619db34f9c552de1ed3d034f8c079987e03) )
	ROM_RELOAD(                              0x60000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u38.u38", 0x70000, 0x10000, CRC(09b03b80) SHA1(a45782d29a426fac38299b56af0815e844e35ae4) )
	ROM_RELOAD(                              0x80000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "rev3_narc_game_rom_u78.u78", 0x00000, 0x10000, CRC(388581b0) SHA1(9d3c718c7bee8f3db9b87ae08ec3bcc1bf65084b) )
	ROM_RELOAD(                                    0x20000, 0x10000 )
	ROM_LOAD16_BYTE( "rev3_narc_game_rom_u60.u60", 0x00001, 0x10000, CRC(f273bc04) SHA1(d4a75c1d6fa706f582ac8131387042a3c9abd08e) )
	ROM_RELOAD(                                    0x20001, 0x10000 )
	ROM_LOAD16_BYTE( "rev3_narc_game_rom_u77.u77", 0x40000, 0x10000, CRC(bdafaccc) SHA1(9e0607d2a2a939847e95489970969df5af1fb708) )
	ROM_RELOAD(                                    0x60000, 0x10000 )
	ROM_LOAD16_BYTE( "rev3_narc_game_rom_u59.u59", 0x40001, 0x10000, CRC(96314a99) SHA1(917cde404b325d0689a2c5848a145eedfd31fc57) )
	ROM_RELOAD(                                    0x60001, 0x10000 )
	ROM_LOAD16_BYTE( "rev3_narc_game_rom_u42.u42", 0x80000, 0x10000, CRC(56aebc81) SHA1(5177ea0121e1b742934ffdcf85795b2c9595b5de) )
	ROM_RELOAD(                                    0xa0000, 0x10000 )
	ROM_LOAD16_BYTE( "rev3_narc_game_rom_u24.u24", 0x80001, 0x10000, CRC(11d7e143) SHA1(c58bc9615d480a97443cc5d4fb2f8ce9fba9db63) )
	ROM_RELOAD(                                    0xa0001, 0x10000 )
	ROM_LOAD16_BYTE( "rev3_narc_game_rom_u41.u41", 0xc0000, 0x10000, CRC(6142fab7) SHA1(e1cc5b088bf2fb9be51d4620b3ff3e50e0fd3117) )
	ROM_RELOAD(                                    0xe0000, 0x10000 )
	ROM_LOAD16_BYTE( "rev3_narc_game_rom_u23.u23", 0xc0001, 0x10000, CRC(98cdd178) SHA1(dd46a957462f2a9dc6de89379fe3e21664873a3c) )
	ROM_RELOAD(                                    0xe0001, 0x10000 )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "rev2_narc_image_rom_u94.u94", 0x000000, 0x10000, CRC(ca3194e4) SHA1(d6aa6a09e4353a1dddd502abf85acf48e6e94cef) )
	ROM_LOAD( "rev2_narc_image_rom_u93.u93", 0x010000, 0x10000, CRC(0ed7f7f5) SHA1(913d0dc81531adc6a7e6ffabfe681150aa4638a3) )
	ROM_LOAD( "rev2_narc_image_rom_u92.u92", 0x020000, 0x10000, CRC(40d2fc66) SHA1(95b8d90e6abe336ad05dc3746d02b38823d2b8cd) )
	ROM_LOAD( "rev2_narc_image_rom_u91.u91", 0x030000, 0x10000, CRC(f39325e0) SHA1(c1179825c76ed2934dfeff263a9296c2c1a5abe4) )
	ROM_LOAD( "rev2_narc_image_rom_u90.u90", 0x040000, 0x10000, CRC(0132aefa) SHA1(9bf11ebc06f1069ea056427750902c204facbd3d) )
	ROM_LOAD( "rev2_narc_image_rom_u89.u89", 0x050000, 0x10000, CRC(f7260c9e) SHA1(5a3fd88c7c0fa01ec2eb6fdef380ccee9d7da3a8) )
	ROM_LOAD( "rev2_narc_image_rom_u88.u88", 0x060000, 0x10000, CRC(edc19f42) SHA1(b7121b3df743e5744ae72de2216b679fe71a2049) )
	ROM_LOAD( "rev2_narc_image_rom_u87.u87", 0x070000, 0x10000, CRC(d9b42ff9) SHA1(cab05a5f8aadff010fba1107eb2000cc128063ff) )
	ROM_LOAD( "rev2_narc_image_rom_u86.u86", 0x080000, 0x10000, CRC(af7daad3) SHA1(e2635a0acd6a238159ef91c1c3c9dfe8de8ae18f) )
	ROM_LOAD( "rev2_narc_image_rom_u85.u85", 0x090000, 0x10000, CRC(095fae6b) SHA1(94f1df799142990a559e54cd949d9723481806b1) )
	ROM_LOAD( "rev2_narc_image_rom_u84.u84", 0x0a0000, 0x10000, CRC(3fdf2057) SHA1(25ac6263a4eb962d90a305572fb95b75cb9f4138) )
	ROM_LOAD( "rev2_narc_image_rom_u83.u83", 0x0b0000, 0x10000, CRC(f2d27c9f) SHA1(de30c7e0191adf62b11b2f2fbdf80687e653de12) )
	ROM_LOAD( "rev2_narc_image_rom_u82.u82", 0x0c0000, 0x10000, CRC(962ce47c) SHA1(ea32f7f58a5ec1d941b372db5378d14fd850a2a7) )
	ROM_LOAD( "rev2_narc_image_rom_u81.u81", 0x0d0000, 0x10000, CRC(00fe59ec) SHA1(85efd623b9cd75b249e19b2e97440a47718da728) )
	ROM_LOAD( "rev2_narc_image_rom_u80.u80", 0x0e0000, 0x10000, CRC(147ba8e9) SHA1(1065b57082e0198025fe6f0bb3548f37c6a715e4) )
	// U79 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u76.u76", 0x200000, 0x10000, CRC(1cd897f4) SHA1(80414c3718ac6719abcca83f483302fc16fcfef3) )
	ROM_LOAD( "rev2_narc_image_rom_u75.u75", 0x210000, 0x10000, CRC(78abfa01) SHA1(1523f537491b901f2d987d4443077b92e24b969d) )
	ROM_LOAD( "rev2_narc_image_rom_u74.u74", 0x220000, 0x10000, CRC(66d2a234) SHA1(290b3051fa9d35e24a9d00fcc2b72d2751f3e7f1) )
	ROM_LOAD( "rev2_narc_image_rom_u73.u73", 0x230000, 0x10000, CRC(efa5cd4e) SHA1(7aca6058d644a025c6799d55ffa082ba8eb5d76f) )
	ROM_LOAD( "rev2_narc_image_rom_u72.u72", 0x240000, 0x10000, CRC(70638eb5) SHA1(fbafb354fca7c3c402be5073fa03060de569f536) )
	ROM_LOAD( "rev2_narc_image_rom_u71.u71", 0x250000, 0x10000, CRC(61226883) SHA1(09a366df0603cc0afc8c6c5547ec6ae3a02724b2) )
	ROM_LOAD( "rev2_narc_image_rom_u70.u70", 0x260000, 0x10000, CRC(c808849f) SHA1(bd3f69c4641331738e415d6d72fafe0eeeb2e56b) )
	ROM_LOAD( "rev2_narc_image_rom_u69.u69", 0x270000, 0x10000, CRC(e7f9c34f) SHA1(f65aed012f1d575a63690222b8c8f2c56bc196c3) )
	ROM_LOAD( "rev2_narc_image_rom_u68.u68", 0x280000, 0x10000, CRC(88a634d5) SHA1(9ddf86ca8cd91965348bc311cc722151f831db21) )
	ROM_LOAD( "rev2_narc_image_rom_u67.u67", 0x290000, 0x10000, CRC(4ab8b69e) SHA1(4320407c78864edc7876ad3604405414a3e7762d) )
	ROM_LOAD( "rev2_narc_image_rom_u66.u66", 0x2a0000, 0x10000, CRC(e1da4b25) SHA1(c81ed1ffc0a4bf64e794a1313559453f9455c312) )
	ROM_LOAD( "rev2_narc_image_rom_u65.u65", 0x2b0000, 0x10000, CRC(6df0d125) SHA1(37392cc917e73cfa09970fd24503b45ced399976) )
	ROM_LOAD( "rev2_narc_image_rom_u64.u64", 0x2c0000, 0x10000, CRC(abab1b16) SHA1(2913a94e1fcf8df52e29d0fb6e373aa64d23c019) )
	ROM_LOAD( "rev2_narc_image_rom_u63.u63", 0x2d0000, 0x10000, CRC(80602f31) SHA1(f1c5c4476dbf80382f33c0776c103cff9bed8346) )
	ROM_LOAD( "rev2_narc_image_rom_u62.u62", 0x2e0000, 0x10000, CRC(c2a476d1) SHA1(ffde1784548050d87f1404aaca3689417e6f7a81) )
	// U61 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u58.u58", 0x400000, 0x10000, CRC(8a7501e3) SHA1(dcd87c464fcb88180cc1c24ec82586440a197a5c) )
	ROM_LOAD( "rev2_narc_image_rom_u57.u57", 0x410000, 0x10000, CRC(a504735f) SHA1(2afe58e576eea2e0326c6b42adb621358a270881) )
	ROM_LOAD( "rev2_narc_image_rom_u56.u56", 0x420000, 0x10000, CRC(55f8cca7) SHA1(0b0a0d50be4401e4ac4e75d8040f18540f9ddc61) )
	ROM_LOAD( "rev2_narc_image_rom_u55.u55", 0x430000, 0x10000, CRC(d3c932c1) SHA1(1a7ffc04e796ba355506bf9037c21aef18fe01a3) )
	ROM_LOAD( "rev2_narc_image_rom_u54.u54", 0x440000, 0x10000, CRC(c7f4134b) SHA1(aea523e17f95c27d1f2c1f69884f626d96c8cb3b) )
	ROM_LOAD( "rev2_narc_image_rom_u53.u53", 0x450000, 0x10000, CRC(6be4da56) SHA1(35a93a259be04a644ca70df4922f6915274c3932) )
	ROM_LOAD( "rev2_narc_image_rom_u52.u52", 0x460000, 0x10000, CRC(1ea36a4a) SHA1(78e5437d46c1ecff5e221bc301925b10f00c5269) )
	ROM_LOAD( "rev2_narc_image_rom_u51.u51", 0x470000, 0x10000, CRC(9d4b0324) SHA1(80fb38a9ac81a0383112df680b9755d7cccbd50b) )
	ROM_LOAD( "rev2_narc_image_rom_u50.u50", 0x480000, 0x10000, CRC(6f9f0c26) SHA1(be77d99fb37fa31c3824725b28ee74206c584b90) )
	ROM_LOAD( "rev2_narc_image_rom_u49.u49", 0x490000, 0x10000, CRC(80386fce) SHA1(f182ed0f1a3753dedc56cb120cb8d10e1556e966) )
	ROM_LOAD( "rev2_narc_image_rom_u48.u48", 0x4a0000, 0x10000, CRC(05c16185) SHA1(429910c5b1f1fe47fdec6cfcba765ee9f10749f0) )
	ROM_LOAD( "rev2_narc_image_rom_u47.u47", 0x4b0000, 0x10000, CRC(4c0151f1) SHA1(b526066fc594f3ec83bb4866986e3b73cdae3992) )
	ROM_LOAD( "rev2_narc_image_rom_u46.u46", 0x4c0000, 0x10000, CRC(5670bfcb) SHA1(b20829b715c6421894c10c02aebb08d22b5109c9) )
	ROM_LOAD( "rev2_narc_image_rom_u45.u45", 0x4d0000, 0x10000, CRC(27f10d98) SHA1(b027ade2b4a52977d9c40c9549b9067d37fab41c) )
	ROM_LOAD( "rev2_narc_image_rom_u44.u44", 0x4e0000, 0x10000, CRC(93b8eaa4) SHA1(b786f3286c5443cf08e556e9fb030b3444288f3c) )
	// U43 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u40.u40", 0x600000, 0x10000, CRC(7fcaebc7) SHA1(b951d63c072d693f7dfc7e362a12513eb9bd6bab) )
	ROM_LOAD( "rev2_narc_image_rom_u39.u39", 0x610000, 0x10000, CRC(7db5cf52) SHA1(478aefc1126493378d22c857646e2fce221c7d21) )
	ROM_LOAD( "rev2_narc_image_rom_u38.u38", 0x620000, 0x10000, CRC(3f9f3ef7) SHA1(5315e8c372bb63d95f814d8eafe0f41e4d95ba1a) )
	ROM_LOAD( "rev2_narc_image_rom_u37.u37", 0x630000, 0x10000, CRC(ed81826c) SHA1(afe1c0fc692a802279c1f7f31143d33028d35ce4) )
	ROM_LOAD( "rev2_narc_image_rom_u36.u36", 0x640000, 0x10000, CRC(e5d855c0) SHA1(3fa0f765238ad2a27c0c65805bf56ebfbe50bf05) )
	ROM_LOAD( "rev2_narc_image_rom_u35.u35", 0x650000, 0x10000, CRC(3a7b1329) SHA1(e8b547a3b8f85cd13e12cfe0bf3949acc1486e6b) )
	ROM_LOAD( "rev2_narc_image_rom_u34.u34", 0x660000, 0x10000, CRC(fe982b0e) SHA1(a03e7e348186339fd93ce119f65e8f0ea7b7bb7a) )
	ROM_LOAD( "rev2_narc_image_rom_u33.u33", 0x670000, 0x10000, CRC(6bc7eb0f) SHA1(6964ef63d0daf1bc7fa9585567659cfc198b6cc3) )
	ROM_LOAD( "rev2_narc_image_rom_u32.u32", 0x680000, 0x10000, CRC(5875a6d3) SHA1(ae64aa786239be39c3c99bbe019bdc91003c1691) )
	ROM_LOAD( "rev2_narc_image_rom_u31.u31", 0x690000, 0x10000, CRC(2fa4b8e5) SHA1(8e4e4abd60d20e0ef955ac4b1f300cfd157e50ca) )
	ROM_LOAD( "rev2_narc_image_rom_u30.u30", 0x6a0000, 0x10000, CRC(7e4bb8ee) SHA1(7166bd56a569329e01ed0c03579a403d659a4a7b) )
	ROM_LOAD( "rev2_narc_image_rom_u29.u29", 0x6b0000, 0x10000, CRC(45136fd9) SHA1(44388e16d02a8c55fed0dbbcd842c941fa4b11b1) )
	ROM_LOAD( "rev2_narc_image_rom_u28.u28", 0x6c0000, 0x10000, CRC(d6cdac24) SHA1(d4bbe3a1be89be7d21769bfe476b50c05cd0c357) )
	ROM_LOAD( "rev2_narc_image_rom_u27.u27", 0x6d0000, 0x10000, CRC(4d33bbec) SHA1(05a3bd66ff91c824e841ca3943585f6aa383c5c2) )
	ROM_LOAD( "rev2_narc_image_rom_u26.u26", 0x6e0000, 0x10000, CRC(cb19f784) SHA1(1e4d85603c940e247fdc45f0366dfb484285e588) )
	// U25 socket not populated

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "a-5346-3036-1_narc_pls153.u28", 0x000, 0x0eb, CRC(4db36615) SHA1(781d13af2735ad3da0c7594487e7f4e19b5f459f) )
	ROM_LOAD( "a-5346-3036-2_narc_pls153.u78", 0x100, 0x0eb, CRC(4b151863) SHA1(0740c5fba978b9bb68a50472b57c0801c7591d6f) )
	ROM_LOAD( "a-5346-3036-3_narc_pls153.u79", 0x200, 0x0eb, CRC(35bd6ed8) SHA1(98b4ad4f61d2f2013815fa79c3428a2d610c8a6f) )
	ROM_LOAD( "a-5346-3036-4_narc_pls153.u80", 0x300, 0x0eb, CRC(9c10e4cf) SHA1(77d0a6a6e19e2f5b17e9356459215e5452247be3) )
	ROM_LOAD( "a-5346-3036-5_narc_pls153.u83", 0x400, 0x0eb, CRC(3a2f21b2) SHA1(bafc45b8b0e83d044bc4963abbdbbf4b6da1694b) )
ROM_END


ROM_START( narc2 )
	ROM_REGION( 0x90000, "narcsnd:cpu0", 0 )    // sound CPU
	ROM_LOAD( "rev2_narc_sound_rom_u4.u4", 0x50000, 0x10000, CRC(450a591a) SHA1(bbda8061262738e5866f2707f69483a0a51d2910) )
	ROM_RELOAD(                            0x60000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u5.u5", 0x70000, 0x10000, CRC(e551e5e3) SHA1(c8b4f53dbd4c534abb77d4dc07c4d12653b79894) )
	ROM_RELOAD(                            0x80000, 0x10000 )

	ROM_REGION( 0x90000, "narcsnd:cpu1", 0 )    // slave sound CPU
	ROM_LOAD( "rev2_narc_sound_rom_u35.u35", 0x10000, 0x10000, CRC(81295892) SHA1(159664e5ee03c88d6e940e70e87e2150dc5b8b25) )
	ROM_RELOAD(                              0x20000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u36.u36", 0x30000, 0x10000, CRC(16cdbb13) SHA1(2dfd961a5d909c1804f4fda34de33ee2664c4bc6) )
	ROM_RELOAD(                              0x40000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u37.u37", 0x50000, 0x10000, CRC(29dbeffd) SHA1(4cbdc619db34f9c552de1ed3d034f8c079987e03) )
	ROM_RELOAD(                              0x60000, 0x10000 )
	ROM_LOAD( "rev2_narc_sound_rom_u38.u38", 0x70000, 0x10000, CRC(09b03b80) SHA1(a45782d29a426fac38299b56af0815e844e35ae4) )
	ROM_RELOAD(                              0x80000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "rev2_narc_game_rom_u78.u78", 0x00000, 0x10000, CRC(150c2dc4) SHA1(c7e3f90f5fed08f2a6951779166cbc6d2dbcd380) )
	ROM_RELOAD(                                    0x20000, 0x10000 )
	ROM_LOAD16_BYTE( "rev2_narc_game_rom_u60.u60", 0x00001, 0x10000, CRC(9720ddea) SHA1(27f0182799f14c1c7c8dc48f7cf4160768b24662) )
	ROM_RELOAD(                                    0x20001, 0x10000 )
	ROM_LOAD16_BYTE( "rev2_narc_game_rom_u77.u77", 0x40000, 0x10000, CRC(75ba4c74) SHA1(8713c22d30107d01612571d3a42aa9edda795fb0) )
	ROM_RELOAD(                                    0x60000, 0x10000 )
	ROM_LOAD16_BYTE( "rev2_narc_game_rom_u59.u59", 0x40001, 0x10000, CRC(f7c6c104) SHA1(1b57a95f2232a9433831b99b689802ef185ff203) )
	ROM_RELOAD(                                    0x60001, 0x10000 )
	ROM_LOAD16_BYTE( "rev2_narc_game_rom_u42.u42", 0x80000, 0x10000, CRC(3db20bb8) SHA1(688844bd573e5d0c5225fccbc12ae91b88b95bd8) )
	ROM_RELOAD(                                    0xa0000, 0x10000 )
	ROM_LOAD16_BYTE( "rev2_narc_game_rom_u24.u24", 0x80001, 0x10000, CRC(91bae451) SHA1(549ad5938ae9ae4e320d0c5f8f30f23f5de2c802) )
	ROM_RELOAD(                                    0xa0001, 0x10000 )
	ROM_LOAD16_BYTE( "rev2_narc_game_rom_u41.u41", 0xc0000, 0x10000, CRC(b0d463e1) SHA1(f6f1a9088aab838f3efe21f71616374ffec35a05) )
	ROM_RELOAD(                                    0xe0000, 0x10000 )
	ROM_LOAD16_BYTE( "rev2_narc_game_rom_u23.u23", 0xc0001, 0x10000, CRC(a9eb4825) SHA1(9c0b98451f1a240a3cb7ed4c1aab6c7c4abd27e6) )
	ROM_RELOAD(                                    0xe0001, 0x10000 )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "rev2_narc_image_rom_u94.u94", 0x000000, 0x10000, CRC(ca3194e4) SHA1(d6aa6a09e4353a1dddd502abf85acf48e6e94cef) )
	ROM_LOAD( "rev2_narc_image_rom_u93.u93", 0x010000, 0x10000, CRC(0ed7f7f5) SHA1(913d0dc81531adc6a7e6ffabfe681150aa4638a3) )
	ROM_LOAD( "rev2_narc_image_rom_u92.u92", 0x020000, 0x10000, CRC(40d2fc66) SHA1(95b8d90e6abe336ad05dc3746d02b38823d2b8cd) )
	ROM_LOAD( "rev2_narc_image_rom_u91.u91", 0x030000, 0x10000, CRC(f39325e0) SHA1(c1179825c76ed2934dfeff263a9296c2c1a5abe4) )
	ROM_LOAD( "rev2_narc_image_rom_u90.u90", 0x040000, 0x10000, CRC(0132aefa) SHA1(9bf11ebc06f1069ea056427750902c204facbd3d) )
	ROM_LOAD( "rev2_narc_image_rom_u89.u89", 0x050000, 0x10000, CRC(f7260c9e) SHA1(5a3fd88c7c0fa01ec2eb6fdef380ccee9d7da3a8) )
	ROM_LOAD( "rev2_narc_image_rom_u88.u88", 0x060000, 0x10000, CRC(edc19f42) SHA1(b7121b3df743e5744ae72de2216b679fe71a2049) )
	ROM_LOAD( "rev2_narc_image_rom_u87.u87", 0x070000, 0x10000, CRC(d9b42ff9) SHA1(cab05a5f8aadff010fba1107eb2000cc128063ff) )
	ROM_LOAD( "rev2_narc_image_rom_u86.u86", 0x080000, 0x10000, CRC(af7daad3) SHA1(e2635a0acd6a238159ef91c1c3c9dfe8de8ae18f) )
	ROM_LOAD( "rev2_narc_image_rom_u85.u85", 0x090000, 0x10000, CRC(095fae6b) SHA1(94f1df799142990a559e54cd949d9723481806b1) )
	ROM_LOAD( "rev2_narc_image_rom_u84.u84", 0x0a0000, 0x10000, CRC(3fdf2057) SHA1(25ac6263a4eb962d90a305572fb95b75cb9f4138) )
	ROM_LOAD( "rev2_narc_image_rom_u83.u83", 0x0b0000, 0x10000, CRC(f2d27c9f) SHA1(de30c7e0191adf62b11b2f2fbdf80687e653de12) )
	ROM_LOAD( "rev2_narc_image_rom_u82.u82", 0x0c0000, 0x10000, CRC(962ce47c) SHA1(ea32f7f58a5ec1d941b372db5378d14fd850a2a7) )
	ROM_LOAD( "rev2_narc_image_rom_u81.u81", 0x0d0000, 0x10000, CRC(00fe59ec) SHA1(85efd623b9cd75b249e19b2e97440a47718da728) )
	ROM_LOAD( "rev2_narc_image_rom_u80.u80", 0x0e0000, 0x10000, CRC(147ba8e9) SHA1(1065b57082e0198025fe6f0bb3548f37c6a715e4) )
	// U79 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u76.u76", 0x200000, 0x10000, CRC(1cd897f4) SHA1(80414c3718ac6719abcca83f483302fc16fcfef3) )
	ROM_LOAD( "rev2_narc_image_rom_u75.u75", 0x210000, 0x10000, CRC(78abfa01) SHA1(1523f537491b901f2d987d4443077b92e24b969d) )
	ROM_LOAD( "rev2_narc_image_rom_u74.u74", 0x220000, 0x10000, CRC(66d2a234) SHA1(290b3051fa9d35e24a9d00fcc2b72d2751f3e7f1) )
	ROM_LOAD( "rev2_narc_image_rom_u73.u73", 0x230000, 0x10000, CRC(efa5cd4e) SHA1(7aca6058d644a025c6799d55ffa082ba8eb5d76f) )
	ROM_LOAD( "rev2_narc_image_rom_u72.u72", 0x240000, 0x10000, CRC(70638eb5) SHA1(fbafb354fca7c3c402be5073fa03060de569f536) )
	ROM_LOAD( "rev2_narc_image_rom_u71.u71", 0x250000, 0x10000, CRC(61226883) SHA1(09a366df0603cc0afc8c6c5547ec6ae3a02724b2) )
	ROM_LOAD( "rev2_narc_image_rom_u70.u70", 0x260000, 0x10000, CRC(c808849f) SHA1(bd3f69c4641331738e415d6d72fafe0eeeb2e56b) )
	ROM_LOAD( "rev2_narc_image_rom_u69.u69", 0x270000, 0x10000, CRC(e7f9c34f) SHA1(f65aed012f1d575a63690222b8c8f2c56bc196c3) )
	ROM_LOAD( "rev2_narc_image_rom_u68.u68", 0x280000, 0x10000, CRC(88a634d5) SHA1(9ddf86ca8cd91965348bc311cc722151f831db21) )
	ROM_LOAD( "rev2_narc_image_rom_u67.u67", 0x290000, 0x10000, CRC(4ab8b69e) SHA1(4320407c78864edc7876ad3604405414a3e7762d) )
	ROM_LOAD( "rev2_narc_image_rom_u66.u66", 0x2a0000, 0x10000, CRC(e1da4b25) SHA1(c81ed1ffc0a4bf64e794a1313559453f9455c312) )
	ROM_LOAD( "rev2_narc_image_rom_u65.u65", 0x2b0000, 0x10000, CRC(6df0d125) SHA1(37392cc917e73cfa09970fd24503b45ced399976) )
	ROM_LOAD( "rev2_narc_image_rom_u64.u64", 0x2c0000, 0x10000, CRC(abab1b16) SHA1(2913a94e1fcf8df52e29d0fb6e373aa64d23c019) )
	ROM_LOAD( "rev2_narc_image_rom_u63.u63", 0x2d0000, 0x10000, CRC(80602f31) SHA1(f1c5c4476dbf80382f33c0776c103cff9bed8346) )
	ROM_LOAD( "rev2_narc_image_rom_u62.u62", 0x2e0000, 0x10000, CRC(c2a476d1) SHA1(ffde1784548050d87f1404aaca3689417e6f7a81) )
	// U61 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u58.u58", 0x400000, 0x10000, CRC(8a7501e3) SHA1(dcd87c464fcb88180cc1c24ec82586440a197a5c) )
	ROM_LOAD( "rev2_narc_image_rom_u57.u57", 0x410000, 0x10000, CRC(a504735f) SHA1(2afe58e576eea2e0326c6b42adb621358a270881) )
	ROM_LOAD( "rev2_narc_image_rom_u56.u56", 0x420000, 0x10000, CRC(55f8cca7) SHA1(0b0a0d50be4401e4ac4e75d8040f18540f9ddc61) )
	ROM_LOAD( "rev2_narc_image_rom_u55.u55", 0x430000, 0x10000, CRC(d3c932c1) SHA1(1a7ffc04e796ba355506bf9037c21aef18fe01a3) )
	ROM_LOAD( "rev2_narc_image_rom_u54.u54", 0x440000, 0x10000, CRC(c7f4134b) SHA1(aea523e17f95c27d1f2c1f69884f626d96c8cb3b) )
	ROM_LOAD( "rev2_narc_image_rom_u53.u53", 0x450000, 0x10000, CRC(6be4da56) SHA1(35a93a259be04a644ca70df4922f6915274c3932) )
	ROM_LOAD( "rev2_narc_image_rom_u52.u52", 0x460000, 0x10000, CRC(1ea36a4a) SHA1(78e5437d46c1ecff5e221bc301925b10f00c5269) )
	ROM_LOAD( "rev2_narc_image_rom_u51.u51", 0x470000, 0x10000, CRC(9d4b0324) SHA1(80fb38a9ac81a0383112df680b9755d7cccbd50b) )
	ROM_LOAD( "rev2_narc_image_rom_u50.u50", 0x480000, 0x10000, CRC(6f9f0c26) SHA1(be77d99fb37fa31c3824725b28ee74206c584b90) )
	ROM_LOAD( "rev2_narc_image_rom_u49.u49", 0x490000, 0x10000, CRC(80386fce) SHA1(f182ed0f1a3753dedc56cb120cb8d10e1556e966) )
	ROM_LOAD( "rev2_narc_image_rom_u48.u48", 0x4a0000, 0x10000, CRC(05c16185) SHA1(429910c5b1f1fe47fdec6cfcba765ee9f10749f0) )
	ROM_LOAD( "rev2_narc_image_rom_u47.u47", 0x4b0000, 0x10000, CRC(4c0151f1) SHA1(b526066fc594f3ec83bb4866986e3b73cdae3992) )
	ROM_LOAD( "rev2_narc_image_rom_u46.u46", 0x4c0000, 0x10000, CRC(5670bfcb) SHA1(b20829b715c6421894c10c02aebb08d22b5109c9) )
	ROM_LOAD( "rev2_narc_image_rom_u45.u45", 0x4d0000, 0x10000, CRC(27f10d98) SHA1(b027ade2b4a52977d9c40c9549b9067d37fab41c) )
	ROM_LOAD( "rev2_narc_image_rom_u44.u44", 0x4e0000, 0x10000, CRC(93b8eaa4) SHA1(b786f3286c5443cf08e556e9fb030b3444288f3c) )
	// U43 socket not populated

	ROM_LOAD( "rev2_narc_image_rom_u40.u40", 0x600000, 0x10000, CRC(7fcaebc7) SHA1(b951d63c072d693f7dfc7e362a12513eb9bd6bab) )
	ROM_LOAD( "rev2_narc_image_rom_u39.u39", 0x610000, 0x10000, CRC(7db5cf52) SHA1(478aefc1126493378d22c857646e2fce221c7d21) )
	ROM_LOAD( "rev2_narc_image_rom_u38.u38", 0x620000, 0x10000, CRC(3f9f3ef7) SHA1(5315e8c372bb63d95f814d8eafe0f41e4d95ba1a) )
	ROM_LOAD( "rev2_narc_image_rom_u37.u37", 0x630000, 0x10000, CRC(ed81826c) SHA1(afe1c0fc692a802279c1f7f31143d33028d35ce4) )
	ROM_LOAD( "rev2_narc_image_rom_u36.u36", 0x640000, 0x10000, CRC(e5d855c0) SHA1(3fa0f765238ad2a27c0c65805bf56ebfbe50bf05) )
	ROM_LOAD( "rev2_narc_image_rom_u35.u35", 0x650000, 0x10000, CRC(3a7b1329) SHA1(e8b547a3b8f85cd13e12cfe0bf3949acc1486e6b) )
	ROM_LOAD( "rev2_narc_image_rom_u34.u34", 0x660000, 0x10000, CRC(fe982b0e) SHA1(a03e7e348186339fd93ce119f65e8f0ea7b7bb7a) )
	ROM_LOAD( "rev2_narc_image_rom_u33.u33", 0x670000, 0x10000, CRC(6bc7eb0f) SHA1(6964ef63d0daf1bc7fa9585567659cfc198b6cc3) )
	ROM_LOAD( "rev2_narc_image_rom_u32.u32", 0x680000, 0x10000, CRC(5875a6d3) SHA1(ae64aa786239be39c3c99bbe019bdc91003c1691) )
	ROM_LOAD( "rev2_narc_image_rom_u31.u31", 0x690000, 0x10000, CRC(2fa4b8e5) SHA1(8e4e4abd60d20e0ef955ac4b1f300cfd157e50ca) )
	ROM_LOAD( "rev2_narc_image_rom_u30.u30", 0x6a0000, 0x10000, CRC(7e4bb8ee) SHA1(7166bd56a569329e01ed0c03579a403d659a4a7b) )
	ROM_LOAD( "rev2_narc_image_rom_u29.u29", 0x6b0000, 0x10000, CRC(45136fd9) SHA1(44388e16d02a8c55fed0dbbcd842c941fa4b11b1) )
	ROM_LOAD( "rev2_narc_image_rom_u28.u28", 0x6c0000, 0x10000, CRC(d6cdac24) SHA1(d4bbe3a1be89be7d21769bfe476b50c05cd0c357) )
	ROM_LOAD( "rev2_narc_image_rom_u27.u27", 0x6d0000, 0x10000, CRC(4d33bbec) SHA1(05a3bd66ff91c824e841ca3943585f6aa383c5c2) )
	ROM_LOAD( "rev2_narc_image_rom_u26.u26", 0x6e0000, 0x10000, CRC(cb19f784) SHA1(1e4d85603c940e247fdc45f0366dfb484285e588) )
	// U25 socket not populated

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "a-5346-3036-1_narc_pls153.u28", 0x000, 0x0eb, CRC(4db36615) SHA1(781d13af2735ad3da0c7594487e7f4e19b5f459f) )
	ROM_LOAD( "a-5346-3036-2_narc_pls153.u78", 0x100, 0x0eb, CRC(4b151863) SHA1(0740c5fba978b9bb68a50472b57c0801c7591d6f) )
	ROM_LOAD( "a-5346-3036-3_narc_pls153.u79", 0x200, 0x0eb, CRC(35bd6ed8) SHA1(98b4ad4f61d2f2013815fa79c3428a2d610c8a6f) )
	ROM_LOAD( "a-5346-3036-4_narc_pls153.u80", 0x300, 0x0eb, CRC(9c10e4cf) SHA1(77d0a6a6e19e2f5b17e9356459215e5452247be3) )
	ROM_LOAD( "a-5346-3036-5_narc_pls153.u83", 0x400, 0x0eb, CRC(3a2f21b2) SHA1(bafc45b8b0e83d044bc4963abbdbbf4b6da1694b) )
ROM_END


ROM_START( narc1 )
	ROM_REGION( 0x90000, "narcsnd:cpu0", 0 )    // sound CPU
	ROM_LOAD( "rev1_narc_sound_rom_u4.u4", 0x50000, 0x10000, CRC(345b5b0b) SHA1(98918785b2c096ceec14a42199f1ad24149ef936) )
	ROM_RELOAD(                            0x60000, 0x10000 )
	ROM_LOAD( "rev1_narc_sound_rom_u5.u5", 0x70000, 0x10000, CRC(bfa112b7) SHA1(2156b52b6ad838e3c9ba33d27da4f753f8a26bef) )
	ROM_RELOAD(                            0x80000, 0x10000 )

	ROM_REGION( 0x90000, "narcsnd:cpu1", 0 )    // slave sound CPU
	ROM_LOAD( "rev1_narc_sound_rom_u35.u35", 0x10000, 0x10000, CRC(c932e69f) SHA1(dca1113c63f997b5a779d6e3ca6c555cb266b8ab) )
	ROM_RELOAD(                              0x20000, 0x10000 )
	ROM_LOAD( "rev1_narc_sound_rom_u36.u36", 0x30000, 0x10000, CRC(974ad9f4) SHA1(5646099214d4e29ed529b66a5dedbaeca0ad375e) )
	ROM_RELOAD(                              0x40000, 0x10000 )
	ROM_LOAD( "rev1_narc_sound_rom_u37.u37", 0x50000, 0x10000, CRC(b6469f79) SHA1(58b59ac8466813def7f1f0c6d0f7cdd167500889) )
	ROM_RELOAD(                              0x60000, 0x10000 )
	ROM_LOAD( "rev1_narc_sound_rom_u38.u38", 0x70000, 0x10000, CRC(c535d11c) SHA1(b7e670669713bbf860c2025b9830bfc4d941651c) )
	ROM_RELOAD(                              0x80000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "rev1_narc_game_rom_u42.u42", 0x80000, 0x20000, CRC(145e1346) SHA1(3b6c4d07b9fc5b7ab875d6c9e4d6f1eefaa25412) )
	ROM_LOAD16_BYTE( "rev1_narc_game_rom_u24.u24", 0x80001, 0x20000, CRC(6ac3ba9a) SHA1(3dbe44b6d4638510952fc10dd7b29fe7c6b677cd) )
	ROM_LOAD16_BYTE( "rev1_narc_game_rom_u41.u41", 0xc0000, 0x20000, CRC(68723c97) SHA1(0d6315f0ec3c70a3f0347a1fd79594309c4a2628) )
	ROM_LOAD16_BYTE( "rev1_narc_game_rom_u23.u23", 0xc0001, 0x20000, CRC(78ecc8e5) SHA1(51529f53e783fd2ad691aa4f77f0fa5afbf54d9d) )
	// U59, U60, U77 & U78 sockets not populated

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "rev1_narc_image_rom_u94.u94", 0x000000, 0x10000, CRC(1b4cec52) SHA1(9f3e72034f96cfbf48d33be8bb5ca073bad30a97) )
	ROM_LOAD( "rev1_narc_image_rom_u93.u93", 0x010000, 0x10000, CRC(6e69f198) SHA1(fdc98bdd90c48058364646783bdaedd98d2a9034) )
	ROM_LOAD( "rev1_narc_image_rom_u92.u92", 0x020000, 0x10000, CRC(4a0cb243) SHA1(6abbd582bfb1f85b771bf82d2d9734e9b411c92d) )
	ROM_LOAD( "rev1_narc_image_rom_u91.u91", 0x030000, 0x10000, CRC(38759eb9) SHA1(b1af4102d8eb9eed7ea1c3fccd8f50bd857b0f8c) )
	ROM_LOAD( "rev1_narc_image_rom_u90.u90", 0x040000, 0x10000, CRC(33cf4379) SHA1(735f2bcb356d1d508d578f157a1c0ee3cca7ba06) )
	ROM_LOAD( "rev1_narc_image_rom_u89.u89", 0x050000, 0x10000, CRC(3cbd682e) SHA1(be512c9490e6ca1fca08c0768647af3f11b8984d) )
	ROM_LOAD( "rev1_narc_image_rom_u88.u88", 0x060000, 0x10000, CRC(4fcfad66) SHA1(540d89ee500984a9ed53968e9c19af0b0370601d) )
	ROM_LOAD( "rev1_narc_image_rom_u87.u87", 0x070000, 0x10000, CRC(afc56436) SHA1(ce5082cb002ee0bdbc072d71ba9dc4599631b8ec) )
	ROM_LOAD( "rev1_narc_image_rom_u86.u86", 0x080000, 0x10000, CRC(f1d02184) SHA1(637115aae9042fc7c12b98afa340350603850c19) )
	ROM_LOAD( "rev1_narc_image_rom_u85.u85", 0x090000, 0x10000, CRC(dfe851c8) SHA1(9058dfbbe995392039a1ce73fac00a162ec7f6a9) )
	ROM_LOAD( "rev1_narc_image_rom_u84.u84", 0x0a0000, 0x10000, CRC(8c28c9c4) SHA1(a2f2603ca2b319eff3f65704b54b1b8b32a576bd) )
	ROM_LOAD( "rev1_narc_image_rom_u83.u83", 0x0b0000, 0x10000, CRC(f65cf5ac) SHA1(77090f343b8a6245826527b96083390dd35d0305) )
	ROM_LOAD( "rev1_narc_image_rom_u82.u82", 0x0c0000, 0x10000, CRC(153cb85f) SHA1(e38fde9bf96aa190e970c1dfd79ece0bfaf31772) )
	ROM_LOAD( "rev1_narc_image_rom_u81.u81", 0x0d0000, 0x10000, CRC(61d873fb) SHA1(71a48015a82eaf0f902bc393cdb566f3946fea0c) )
	// U80 & U79 sockets not populated

	ROM_LOAD( "rev1_narc_image_rom_u76.u76", 0x200000, 0x10000, CRC(7b5d553a) SHA1(57fbb29a1e84b44abb015e9e99ae90997f49bdf5) )
	ROM_LOAD( "rev1_narc_image_rom_u75.u75", 0x210000, 0x10000, CRC(b49a3295) SHA1(ee8cf8f614ae3ebb7f02890480c6dbecb17f7e5c) )
	ROM_LOAD( "rev1_narc_image_rom_u74.u74", 0x220000, 0x10000, CRC(4767fbd3) SHA1(ead9d599777e4c95efd1967f1b5d1a83ca9d0908) )
	ROM_LOAD( "rev1_narc_image_rom_u73.u73", 0x230000, 0x10000, CRC(c2c5be8b) SHA1(51a3c03d4100e297e3a2361e56f0606bf8152a6f) )
	ROM_LOAD( "rev1_narc_image_rom_u72.u72", 0x240000, 0x10000, CRC(a4b8e0e4) SHA1(389de5f9526cde99b97820c4258abb2b8104eec1) )
	ROM_LOAD( "rev1_narc_image_rom_u71.u71", 0x250000, 0x10000, CRC(02fde40d) SHA1(1dcfcc3d4cc145daaf743aef8539e18b5427c8d0) )
	ROM_LOAD( "rev1_narc_image_rom_u70.u70", 0x260000, 0x10000, CRC(e53d378e) SHA1(0fd23b1c2dbba283c25163564f49726e5eeb1c14) )
	ROM_LOAD( "rev1_narc_image_rom_u69.u69", 0x270000, 0x10000, CRC(3b00bacc) SHA1(aa7f5d2f646380c544509ec0d565a056a2d9ea3f) )
	ROM_LOAD( "rev1_narc_image_rom_u68.u68", 0x280000, 0x10000, CRC(bd3ccb95) SHA1(608731a6aad6a56e728c3180cc04e59a705515e3) )
	ROM_LOAD( "rev1_narc_image_rom_u67.u67", 0x290000, 0x10000, CRC(2e30c11b) SHA1(a8571bc7cfa178fd613bbc2466d070070db7334e) )
	ROM_LOAD( "rev1_narc_image_rom_u66.u66", 0x2a0000, 0x10000, CRC(1bb2da9d) SHA1(33d3b487573369d0abed74c55b568b3b380d62d8) )
	ROM_LOAD( "rev1_narc_image_rom_u65.u65", 0x2b0000, 0x10000, CRC(1b928ebb) SHA1(aef82be332cf834ddcf8cba6a6dc230457b92cb3) )
	ROM_LOAD( "rev1_narc_image_rom_u64.u64", 0x2c0000, 0x10000, CRC(69539c8a) SHA1(441adad1b7150603c1abe279404e613448b15b2e) )
	ROM_LOAD( "rev1_narc_image_rom_u63.u63", 0x2d0000, 0x10000, CRC(1254a7f8) SHA1(ded2da8438aef45773c33b4f5bfa2541345ffd8a) )
	// U62 & U61 sockets not populated

	ROM_LOAD( "rev1_narc_image_rom_u58.u58", 0x400000, 0x10000, CRC(82cb4229) SHA1(45ac76778f5b574a51bb91f1c8bb86ec76e77e9d) )
	ROM_LOAD( "rev1_narc_image_rom_u57.u57", 0x410000, 0x10000, CRC(4309d12f) SHA1(b055c876c34a220db6de805199da1244fe188607) )
	ROM_LOAD( "rev1_narc_image_rom_u56.u56", 0x420000, 0x10000, CRC(6c4b7b3a) SHA1(54d78847e9f3fba35db21162d6362b6d3d94abaf) )
	ROM_LOAD( "rev1_narc_image_rom_u55.u55", 0x430000, 0x10000, CRC(edb40703) SHA1(cffde287455e52b7ef50e957c133fc5e1fe2edc5) )
	ROM_LOAD( "rev1_narc_image_rom_u54.u54", 0x440000, 0x10000, CRC(9643faad) SHA1(7964f1ce5187498fe5d182da679176ffea2e2c7b) )
	ROM_LOAD( "rev1_narc_image_rom_u53.u53", 0x450000, 0x10000, CRC(d6695995) SHA1(d77ac3c8bd902a16e24381338007f61fc6a3cfff) )
	ROM_LOAD( "rev1_narc_image_rom_u52.u52", 0x460000, 0x10000, CRC(59258d8b) SHA1(4197377e2e94a5d740e1122e4c4c5648de907113) )
	ROM_LOAD( "rev1_narc_image_rom_u51.u51", 0x470000, 0x10000, CRC(c0fd44fc) SHA1(1dbfd67012b2396cb5ff35a01d651abf8743182a) )
	ROM_LOAD( "rev1_narc_image_rom_u50.u50", 0x480000, 0x10000, CRC(7cacd767) SHA1(57379e0626a6cf300710c2c02b49278a7005bfb8) )
	ROM_LOAD( "rev1_narc_image_rom_u49.u49", 0x490000, 0x10000, CRC(64440a70) SHA1(eed9bde050d735bde6d94b573503f86e4e0bfccc) )
	ROM_LOAD( "rev1_narc_image_rom_u48.u48", 0x4a0000, 0x10000, CRC(45162e8a) SHA1(6448f8b16ea0f99a8b306e3c1323b3c765a44b69) )
	ROM_LOAD( "rev1_narc_image_rom_u47.u47", 0x4b0000, 0x10000, CRC(ac5c7f57) SHA1(111ecfa4957c6eef84515e0f53706e2b8b684107) )
	ROM_LOAD( "rev1_narc_image_rom_u46.u46", 0x4c0000, 0x10000, CRC(8bb8d8e1) SHA1(7e1c09dafb2b891ef713dd1255a3c0fdeb67af8f) )
	ROM_LOAD( "rev1_narc_image_rom_u45.u45", 0x4d0000, 0x10000, CRC(b60466c0) SHA1(df0a23f78e21169e517689a06d4b5ebe3b9832d6) )
	// U44 & U43 sockets not populated

	ROM_LOAD( "rev1_narc_image_rom_u40.u40", 0x600000, 0x10000, CRC(02ce306a) SHA1(889cdbdd6cc1ace80c6686e5599627d14363bb41) )
	ROM_LOAD( "rev1_narc_image_rom_u39.u39", 0x610000, 0x10000, CRC(6d702163) SHA1(4d94afbc1901ed63964443280ad1545deb762663) )
	ROM_LOAD( "rev1_narc_image_rom_u38.u38", 0x620000, 0x10000, CRC(349988cf) SHA1(ccb915024df4b60754629546c0d705f5a2544e6c) )
	ROM_LOAD( "rev1_narc_image_rom_u37.u37", 0x630000, 0x10000, CRC(0f8e6588) SHA1(2fc8d6c8d5b75d7018a3570fb6747eea1cca89ac) )
	ROM_LOAD( "rev1_narc_image_rom_u36.u36", 0x640000, 0x10000, CRC(645723cd) SHA1(f0ae22d3b7879f8ab530ef01bff624cf64db2156) )
	ROM_LOAD( "rev1_narc_image_rom_u35.u35", 0x650000, 0x10000, CRC(00661dec) SHA1(19721ae5fa649040aa6cafbb0da4cd9c091c1e75) )
	ROM_LOAD( "rev1_narc_image_rom_u34.u34", 0x660000, 0x10000, CRC(89dcbf17) SHA1(98e98d9ef6adf9f23f6df3bfd0b986d8fa7f7632) )
	ROM_LOAD( "rev1_narc_image_rom_u33.u33", 0x670000, 0x10000, CRC(ad220197) SHA1(f10f3ff526efbf4ab43824693256a2eaf07a6a0f) )
	ROM_LOAD( "rev1_narc_image_rom_u32.u32", 0x680000, 0x10000, CRC(40aa3f31) SHA1(62ecac3aa32c9bb6ed59315c531ee33f30f0f20b) )
	ROM_LOAD( "rev1_narc_image_rom_u31.u31", 0x690000, 0x10000, CRC(81cf74aa) SHA1(9a6cd036077eed30c627e188b3709cda5b1837a1) )
	ROM_LOAD( "rev1_narc_image_rom_u30.u30", 0x6a0000, 0x10000, CRC(b3e07443) SHA1(c75f63d26209827ce003d21ac8037feb7f5d220c) )
	ROM_LOAD( "rev1_narc_image_rom_u29.u29", 0x6b0000, 0x10000, CRC(fdaedb84) SHA1(5d81d5f2083727f8b9a16ab08a212ceb2bcc76ba) )
	ROM_LOAD( "rev1_narc_image_rom_u28.u28", 0x6c0000, 0x10000, CRC(3012cd6e) SHA1(670ade7b46b3232a0d3c5fea2ab25f8af7664a4e) )
	ROM_LOAD( "rev1_narc_image_rom_u27.u27", 0x6d0000, 0x10000, CRC(e631fe7d) SHA1(e952418f23939e169dd8aa666ad3564e810c9554) )
	// U26 & U25 sockets not populated

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "a-5346-3036-1_narc_pls153.u28", 0x000, 0x0eb, CRC(4db36615) SHA1(781d13af2735ad3da0c7594487e7f4e19b5f459f) )
	ROM_LOAD( "a-5346-3036-2_narc_pls153.u78", 0x100, 0x0eb, CRC(4b151863) SHA1(0740c5fba978b9bb68a50472b57c0801c7591d6f) )
	ROM_LOAD( "a-5346-3036-3_narc_pls153.u79", 0x200, 0x0eb, CRC(35bd6ed8) SHA1(98b4ad4f61d2f2013815fa79c3428a2d610c8a6f) )
	ROM_LOAD( "a-5346-3036-4_narc_pls153.u80", 0x300, 0x0eb, CRC(9c10e4cf) SHA1(77d0a6a6e19e2f5b17e9356459215e5452247be3) )
	ROM_LOAD( "a-5346-3036-5_narc_pls153.u83", 0x400, 0x0eb, CRC(3a2f21b2) SHA1(bafc45b8b0e83d044bc4963abbdbbf4b6da1694b) )
ROM_END


ROM_START( trog )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD( "trog_ii_u-4_sl_1.u4",   0x10000, 0x10000, CRC(759d0bf4) SHA1(c4c3fa51c43cf7fd241ac1f33d7d220aa9f9edb3) )
	ROM_RELOAD(                        0x20000, 0x10000 )
	ROM_LOAD( "trog_ii_u-19_sl_1.u19", 0x30000, 0x10000, CRC(960c333d) SHA1(da8ce8dfffffe7a2d60b3f75cc5aa88e5e2be659) )
	ROM_RELOAD(                        0x40000, 0x10000 )
	ROM_LOAD( "trog_ii_u-20_sl_1.u20", 0x50000, 0x10000, CRC(67f1658a) SHA1(c85dc920ff4b292afa9f6681f31918a200799cc9) )
	ROM_RELOAD(                        0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "trog_ii_u-105_la-5.u105", 0xc0000, 0x20000, CRC(d62cc51a) SHA1(a63ed5b0e08dd89a1392e04cd88c9d83d75810c6) )
	ROM_LOAD16_BYTE( "trog_ii_u-89_la-5.u89",   0xc0001, 0x20000, CRC(edde0bc8) SHA1(95389b75c438c0f0cad668a35570fcb4f7790a02) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "trog_ii_u-111_la-1.u111", 0x000000, 0x20000, CRC(9ded08c1) SHA1(bbf069f218e3f3d67e45fa2229a471451b3a8f96) )
	ROM_LOAD( "trog_ii_u-112_la-1.u112", 0x020000, 0x20000, CRC(42293843) SHA1(cae77eeddd784573beccb79e54573da0e4ccdd8a) )
	ROM_LOAD( "trog_ii_u-113_la-1.u113", 0x040000, 0x20000, CRC(77f50cbb) SHA1(5f2df3aedd90871ac02bca07c66387f6cda0dfdf) )

	ROM_LOAD( "trog_ii_u-106_la-1.u106", 0x080000, 0x20000, CRC(af2eb0d8) SHA1(3767e6f3853b092b40664c2b6c6a838f0243514b) )
	ROM_LOAD( "trog_ii_u-107_la-1.u107", 0x0a0000, 0x20000, CRC(88a7b3f6) SHA1(ba55f66929841a915d7b96aabf4b11e50ba6cfbd) )

	ROM_LOAD( "trog_ii_u-95_la-1.u95",   0x200000, 0x20000, CRC(f3ba2838) SHA1(2bee6c783c84a9f3f9309d802f42983857190ece) )
	ROM_LOAD( "trog_ii_u-96_la-1.u96",   0x220000, 0x20000, CRC(cfed2e77) SHA1(7fc0f52ac844c9efcbcc3004c40f9f4fc7e1c346) )
	ROM_LOAD( "trog_ii_u-97_la-1.u97",   0x240000, 0x20000, CRC(3262d1f8) SHA1(754e3e8223edd11398b2db77fd5db619dad1577b) )

	ROM_LOAD( "trog_ii_u-90_la-1.u90",   0x280000, 0x20000, CRC(16e06753) SHA1(62ec2b18e6b965ea0792d655d7878b4225da3aca) )
	ROM_LOAD( "trog_ii_u-91_la-1.u91",   0x2a0000, 0x20000, CRC(880a02c7) SHA1(ab1b2d24be4571a183b230d267c6c8167d4a42a4) )
ROM_END


ROM_START( trog4 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD( "trog_ii_u-4_sl_1.u4",   0x10000, 0x10000, CRC(759d0bf4) SHA1(c4c3fa51c43cf7fd241ac1f33d7d220aa9f9edb3) )
	ROM_RELOAD(                        0x20000, 0x10000 )
	ROM_LOAD( "trog_ii_u-19_sl_1.u19", 0x30000, 0x10000, CRC(960c333d) SHA1(da8ce8dfffffe7a2d60b3f75cc5aa88e5e2be659) )
	ROM_RELOAD(                        0x40000, 0x10000 )
	ROM_LOAD( "trog_ii_u-20_sl_1.u20", 0x50000, 0x10000, CRC(67f1658a) SHA1(c85dc920ff4b292afa9f6681f31918a200799cc9) )
	ROM_RELOAD(                        0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "trog_ii_u-105_la-4.u105", 0xc0000, 0x20000, CRC(e6095189) SHA1(a2caaf64e371050b37c63d9608ba5d289cf3cd91) )
	ROM_LOAD16_BYTE( "trog_ii_u-89_la-4.u89",   0xc0001, 0x20000, CRC(fdd7cc65) SHA1(bfc4339953c122bca968f9cfa3a82df3584a3727) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "trog_ii_u-111_la-1.u111", 0x000000, 0x20000, CRC(9ded08c1) SHA1(bbf069f218e3f3d67e45fa2229a471451b3a8f96) )
	ROM_LOAD( "trog_ii_u-112_la-1.u112", 0x020000, 0x20000, CRC(42293843) SHA1(cae77eeddd784573beccb79e54573da0e4ccdd8a) )
	ROM_LOAD( "trog_ii_u-113_la-1.u113", 0x040000, 0x20000, CRC(77f50cbb) SHA1(5f2df3aedd90871ac02bca07c66387f6cda0dfdf) )

	ROM_LOAD( "trog_ii_u-106_la-1.u106", 0x080000, 0x20000, CRC(af2eb0d8) SHA1(3767e6f3853b092b40664c2b6c6a838f0243514b) )
	ROM_LOAD( "trog_ii_u-107_la-1.u107", 0x0a0000, 0x20000, CRC(88a7b3f6) SHA1(ba55f66929841a915d7b96aabf4b11e50ba6cfbd) )

	ROM_LOAD( "trog_ii_u-95_la-1.u95",   0x200000, 0x20000, CRC(f3ba2838) SHA1(2bee6c783c84a9f3f9309d802f42983857190ece) )
	ROM_LOAD( "trog_ii_u-96_la-1.u96",   0x220000, 0x20000, CRC(cfed2e77) SHA1(7fc0f52ac844c9efcbcc3004c40f9f4fc7e1c346) )
	ROM_LOAD( "trog_ii_u-97_la-1.u97",   0x240000, 0x20000, CRC(3262d1f8) SHA1(754e3e8223edd11398b2db77fd5db619dad1577b) )

	ROM_LOAD( "trog_ii_u-90_la-1.u90",   0x280000, 0x20000, CRC(16e06753) SHA1(62ec2b18e6b965ea0792d655d7878b4225da3aca) )
	ROM_LOAD( "trog_ii_u-91_la-1.u91",   0x2a0000, 0x20000, CRC(880a02c7) SHA1(ab1b2d24be4571a183b230d267c6c8167d4a42a4) )
ROM_END


ROM_START( trog3 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD( "trog_ii_u-4_sl_1.u4",   0x10000, 0x10000, CRC(759d0bf4) SHA1(c4c3fa51c43cf7fd241ac1f33d7d220aa9f9edb3) )
	ROM_RELOAD(                        0x20000, 0x10000 )
	ROM_LOAD( "trog_ii_u-19_sl_1.u19", 0x30000, 0x10000, CRC(960c333d) SHA1(da8ce8dfffffe7a2d60b3f75cc5aa88e5e2be659) )
	ROM_RELOAD(                        0x40000, 0x10000 )
	ROM_LOAD( "trog_ii_u-20_sl_1.u20", 0x50000, 0x10000, CRC(67f1658a) SHA1(c85dc920ff4b292afa9f6681f31918a200799cc9) )
	ROM_RELOAD(                        0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "trog_ii_u-105_la-3.u105", 0xc0000, 0x20000, CRC(d09cea97) SHA1(0c1384be2af8abbaf1c5c7f86f31ec605c18e798) ) // sldh - rev LA3 2/14/91
	ROM_LOAD16_BYTE( "trog_ii_u-89_la-3.u89",   0xc0001, 0x20000, CRC(a61e3572) SHA1(5366f4c9592dc9e23ffe867a16cbf51d1811a622) ) // sldh - rev LA3 2/14/91

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "trog_ii_u-111_la-1.u111", 0x000000, 0x20000, CRC(9ded08c1) SHA1(bbf069f218e3f3d67e45fa2229a471451b3a8f96) )
	ROM_LOAD( "trog_ii_u-112_la-1.u112", 0x020000, 0x20000, CRC(42293843) SHA1(cae77eeddd784573beccb79e54573da0e4ccdd8a) )
	ROM_LOAD( "trog_ii_u-113_la-1.u113", 0x040000, 0x20000, CRC(77f50cbb) SHA1(5f2df3aedd90871ac02bca07c66387f6cda0dfdf) )

	ROM_LOAD( "trog_ii_u-106_la-1.u106", 0x080000, 0x20000, CRC(af2eb0d8) SHA1(3767e6f3853b092b40664c2b6c6a838f0243514b) )
	ROM_LOAD( "trog_ii_u-107_la-1.u107", 0x0a0000, 0x20000, CRC(88a7b3f6) SHA1(ba55f66929841a915d7b96aabf4b11e50ba6cfbd) )

	ROM_LOAD( "trog_ii_u-95_la-1.u95",   0x200000, 0x20000, CRC(f3ba2838) SHA1(2bee6c783c84a9f3f9309d802f42983857190ece) )
	ROM_LOAD( "trog_ii_u-96_la-1.u96",   0x220000, 0x20000, CRC(cfed2e77) SHA1(7fc0f52ac844c9efcbcc3004c40f9f4fc7e1c346) )
	ROM_LOAD( "trog_ii_u-97_la-1.u97",   0x240000, 0x20000, CRC(3262d1f8) SHA1(754e3e8223edd11398b2db77fd5db619dad1577b) )

	ROM_LOAD( "trog_ii_u-90_la-1.u90",   0x280000, 0x20000, CRC(16e06753) SHA1(62ec2b18e6b965ea0792d655d7878b4225da3aca) )
	ROM_LOAD( "trog_ii_u-91_la-1.u91",   0x2a0000, 0x20000, CRC(880a02c7) SHA1(ab1b2d24be4571a183b230d267c6c8167d4a42a4) )
ROM_END


ROM_START( trog3a )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD( "trog_ii_u-4_sl_1.u4",   0x10000, 0x10000, CRC(759d0bf4) SHA1(c4c3fa51c43cf7fd241ac1f33d7d220aa9f9edb3) )
	ROM_RELOAD(                        0x20000, 0x10000 )
	ROM_LOAD( "trog_ii_u-19_sl_1.u19", 0x30000, 0x10000, CRC(960c333d) SHA1(da8ce8dfffffe7a2d60b3f75cc5aa88e5e2be659) )
	ROM_RELOAD(                        0x40000, 0x10000 )
	ROM_LOAD( "trog_ii_u-20_sl_1.u20", 0x50000, 0x10000, CRC(67f1658a) SHA1(c85dc920ff4b292afa9f6681f31918a200799cc9) )
	ROM_RELOAD(                        0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "trog_ii_u-105_la-3.u105", 0xc0000, 0x20000, CRC(9b3841dd) SHA1(7af3b30857531de324afc44fc7577cefaea5aebb) ) // sldh - rev LA3 2/10/91
	ROM_LOAD16_BYTE( "trog_ii_u-89_la-3.u89",   0xc0001, 0x20000, CRC(9c0e6542) SHA1(a80ce0f1135cd48dcbf6f98e3f385ddcdba35af7) ) // sldh - rev LA3 2/10/91

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "trog_ii_u-111_la-1.u111", 0x000000, 0x20000, CRC(9ded08c1) SHA1(bbf069f218e3f3d67e45fa2229a471451b3a8f96) )
	ROM_LOAD( "trog_ii_u-112_la-1.u112", 0x020000, 0x20000, CRC(42293843) SHA1(cae77eeddd784573beccb79e54573da0e4ccdd8a) )
	ROM_LOAD( "trog_ii_u-113_la-1.u113", 0x040000, 0x20000, CRC(77f50cbb) SHA1(5f2df3aedd90871ac02bca07c66387f6cda0dfdf) ) // changes for LA1

	ROM_LOAD( "trog_ii_u-106_la-1.u106", 0x080000, 0x20000, CRC(af2eb0d8) SHA1(3767e6f3853b092b40664c2b6c6a838f0243514b) )
	ROM_LOAD( "trog_ii_u-107_la-1.u107", 0x0a0000, 0x20000, CRC(88a7b3f6) SHA1(ba55f66929841a915d7b96aabf4b11e50ba6cfbd) )

	ROM_LOAD( "trog_ii_u-95_la-1.u95",   0x200000, 0x20000, CRC(f3ba2838) SHA1(2bee6c783c84a9f3f9309d802f42983857190ece) )
	ROM_LOAD( "trog_ii_u-96_la-1.u96",   0x220000, 0x20000, CRC(cfed2e77) SHA1(7fc0f52ac844c9efcbcc3004c40f9f4fc7e1c346) )
	ROM_LOAD( "trog_ii_u-97_la-1.u97",   0x240000, 0x20000, CRC(3262d1f8) SHA1(754e3e8223edd11398b2db77fd5db619dad1577b) ) // changes for LA1

	ROM_LOAD( "trog_ii_u-90_la-1.u90",   0x280000, 0x20000, CRC(16e06753) SHA1(62ec2b18e6b965ea0792d655d7878b4225da3aca) )
	ROM_LOAD( "trog_ii_u-91_la-1.u91",   0x2a0000, 0x20000, CRC(880a02c7) SHA1(ab1b2d24be4571a183b230d267c6c8167d4a42a4) )
ROM_END


ROM_START( trogpa6 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD( "trog_ii_u-4_sl_1.u4",   0x10000, 0x10000, CRC(759d0bf4) SHA1(c4c3fa51c43cf7fd241ac1f33d7d220aa9f9edb3) )
	ROM_RELOAD(                        0x20000, 0x10000 )
	ROM_LOAD( "trog_ii_u-19_sl_1.u19", 0x30000, 0x10000, CRC(960c333d) SHA1(da8ce8dfffffe7a2d60b3f75cc5aa88e5e2be659) )
	ROM_RELOAD(                        0x40000, 0x10000 )
	ROM_LOAD( "trog_ii_u-20_sl_1.u20", 0x50000, 0x10000, CRC(67f1658a) SHA1(c85dc920ff4b292afa9f6681f31918a200799cc9) )
	ROM_RELOAD(                        0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "trog_ii_u-105_rev.6.u105", 0xc0000, 0x20000, CRC(71ad1903) SHA1(e7ff1344a7bdc3b90f09ce8251ebcd25012be602) )
	ROM_LOAD16_BYTE( "trog_ii_u-89_rev.6.u89",   0xc0001, 0x20000, CRC(04473da8) SHA1(47d9e918fba93b4af1e3cacbac9df843e6a10091) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "trog_ii_u-111_la-1.u111", 0x000000, 0x20000, CRC(9ded08c1) SHA1(bbf069f218e3f3d67e45fa2229a471451b3a8f96) )
	ROM_LOAD( "trog_ii_u-112_la-1.u112", 0x020000, 0x20000, CRC(42293843) SHA1(cae77eeddd784573beccb79e54573da0e4ccdd8a) )
	ROM_LOAD( "trog_ii_u-113_rev.5.u113",0x040000, 0x20000, CRC(ae50e5ea) SHA1(915b76f76e7ccbf2c4c28829cea15feaafea498b) )

	ROM_LOAD( "trog_ii_u-106_la-1.u106", 0x080000, 0x20000, CRC(af2eb0d8) SHA1(3767e6f3853b092b40664c2b6c6a838f0243514b) )
	ROM_LOAD( "trog_ii_u-107_la-1.u107", 0x0a0000, 0x20000, CRC(88a7b3f6) SHA1(ba55f66929841a915d7b96aabf4b11e50ba6cfbd) )

	ROM_LOAD( "trog_ii_u-95_la-1.u95",   0x200000, 0x20000, CRC(f3ba2838) SHA1(2bee6c783c84a9f3f9309d802f42983857190ece) )
	ROM_LOAD( "trog_ii_u-96_la-1.u96",   0x220000, 0x20000, CRC(cfed2e77) SHA1(7fc0f52ac844c9efcbcc3004c40f9f4fc7e1c346) )
	ROM_LOAD( "trog_ii_u-97_rev.5.u97",  0x240000, 0x20000, CRC(354b1cb3) SHA1(88400e39f0476d32a0798c50855a8ff9dc0a6617) )

	ROM_LOAD( "trog_ii_u-90_la-1.u90",   0x280000, 0x20000, CRC(16e06753) SHA1(62ec2b18e6b965ea0792d655d7878b4225da3aca) )
	ROM_LOAD( "trog_ii_u-91_la-1.u91",   0x2a0000, 0x20000, CRC(880a02c7) SHA1(ab1b2d24be4571a183b230d267c6c8167d4a42a4) )
ROM_END


ROM_START( trogpa5 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD( "trog_ii_u-4_sl_1.u4",   0x10000, 0x10000, CRC(759d0bf4) SHA1(c4c3fa51c43cf7fd241ac1f33d7d220aa9f9edb3) )
	ROM_RELOAD(                        0x20000, 0x10000 )
	ROM_LOAD( "trog_ii_u-19_sl_1.u19", 0x30000, 0x10000, CRC(960c333d) SHA1(da8ce8dfffffe7a2d60b3f75cc5aa88e5e2be659) )
	ROM_RELOAD(                        0x40000, 0x10000 )
	ROM_LOAD( "trog_ii_u-20_sl_1.u20", 0x50000, 0x10000, CRC(67f1658a) SHA1(c85dc920ff4b292afa9f6681f31918a200799cc9) )
	ROM_RELOAD(                        0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "trog_ii_u-105_rev.5.u105", 0xc0000, 0x20000, CRC(da645900) SHA1(202a3c89b5fbda676a1c875b6e4c19853ab75983) )
	ROM_LOAD16_BYTE( "trog_ii_u-89_rev.5.u89",   0xc0001, 0x20000, CRC(d42d0f71) SHA1(8fc8af1544ff6fb7258ce9d810e566c3751c871c) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "trog_ii_u-111_la-1.u111", 0x000000, 0x20000, CRC(9ded08c1) SHA1(bbf069f218e3f3d67e45fa2229a471451b3a8f96) )
	ROM_LOAD( "trog_ii_u-112_la-1.u112", 0x020000, 0x20000, CRC(42293843) SHA1(cae77eeddd784573beccb79e54573da0e4ccdd8a) )
	ROM_LOAD( "trog_ii_u-113_rev.5.u113",0x040000, 0x20000, CRC(ae50e5ea) SHA1(915b76f76e7ccbf2c4c28829cea15feaafea498b) ) // changes with PAC5

	ROM_LOAD( "trog_ii_u-106_la-1.u106", 0x080000, 0x20000, CRC(af2eb0d8) SHA1(3767e6f3853b092b40664c2b6c6a838f0243514b) )
	ROM_LOAD( "trog_ii_u-107_la-1.u107", 0x0a0000, 0x20000, CRC(88a7b3f6) SHA1(ba55f66929841a915d7b96aabf4b11e50ba6cfbd) )

	ROM_LOAD( "trog_ii_u-95_la-1.u95",   0x200000, 0x20000, CRC(f3ba2838) SHA1(2bee6c783c84a9f3f9309d802f42983857190ece) )
	ROM_LOAD( "trog_ii_u-96_la-1.u96",   0x220000, 0x20000, CRC(cfed2e77) SHA1(7fc0f52ac844c9efcbcc3004c40f9f4fc7e1c346) )
	ROM_LOAD( "trog_ii_u-97_rev.5.u97",  0x240000, 0x20000, CRC(354b1cb3) SHA1(88400e39f0476d32a0798c50855a8ff9dc0a6617) ) // changes with PAC5

	ROM_LOAD( "trog_ii_u-90_la-1.u90",   0x280000, 0x20000, CRC(16e06753) SHA1(62ec2b18e6b965ea0792d655d7878b4225da3aca) )
	ROM_LOAD( "trog_ii_u-91_la-1.u91",   0x2a0000, 0x20000, CRC(880a02c7) SHA1(ab1b2d24be4571a183b230d267c6c8167d4a42a4) )
ROM_END


ROM_START( trogpa4 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD( "trog_ii_u-4_sl_1.u4",   0x10000, 0x10000, CRC(759d0bf4) SHA1(c4c3fa51c43cf7fd241ac1f33d7d220aa9f9edb3) )
	ROM_RELOAD(                        0x20000, 0x10000 )
	ROM_LOAD( "trog_ii_u-19_sl_1.u19", 0x30000, 0x10000, CRC(960c333d) SHA1(da8ce8dfffffe7a2d60b3f75cc5aa88e5e2be659) )
	ROM_RELOAD(                        0x40000, 0x10000 )
	ROM_LOAD( "trog_ii_u-20_sl_1.u20", 0x50000, 0x10000, CRC(67f1658a) SHA1(c85dc920ff4b292afa9f6681f31918a200799cc9) )
	ROM_RELOAD(                        0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "u105-pa4",  0xc0000, 0x20000, CRC(526a3f5b) SHA1(8ad8cb15ada527f989f774a4fb81a171697c6dad) )
	ROM_LOAD16_BYTE( "u89-pa4",   0xc0001, 0x20000, CRC(38d68685) SHA1(42b73a64641301bf2991929cf365b8f45fc1b5d8) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "trog_ii_u-111_la-1.u111", 0x000000, 0x20000, CRC(9ded08c1) SHA1(bbf069f218e3f3d67e45fa2229a471451b3a8f96) )
	ROM_LOAD( "trog_ii_u-112_la-1.u112", 0x020000, 0x20000, CRC(42293843) SHA1(cae77eeddd784573beccb79e54573da0e4ccdd8a) )
	ROM_LOAD( "trogu113.pa4",            0x040000, 0x20000, CRC(2980a56f) SHA1(1e6ab16be6071d6568149e9ba56e146e3431b5f2) ) // unique to rev 4.00

	ROM_LOAD( "trog_ii_u-106_la-1.u106", 0x080000, 0x20000, CRC(af2eb0d8) SHA1(3767e6f3853b092b40664c2b6c6a838f0243514b) )
	ROM_LOAD( "trog_ii_u-107_la-1.u107", 0x0a0000, 0x20000, CRC(88a7b3f6) SHA1(ba55f66929841a915d7b96aabf4b11e50ba6cfbd) )

	ROM_LOAD( "trog_ii_u-95_la-1.u95",   0x200000, 0x20000, CRC(f3ba2838) SHA1(2bee6c783c84a9f3f9309d802f42983857190ece) )
	ROM_LOAD( "trog_ii_u-96_la-1.u96",   0x220000, 0x20000, CRC(cfed2e77) SHA1(7fc0f52ac844c9efcbcc3004c40f9f4fc7e1c346) )
	ROM_LOAD( "trogu97.pa4",             0x240000, 0x20000, CRC(f94b77c1) SHA1(d4ca3d7270ea1d86cb5c53e85dc7682b0e5945ef) ) // unique to rev 4.00

	ROM_LOAD( "trog_ii_u-90_la-1.u90",   0x280000, 0x20000, CRC(16e06753) SHA1(62ec2b18e6b965ea0792d655d7878b4225da3aca) )
	ROM_LOAD( "trog_ii_u-91_la-1.u91",   0x2a0000, 0x20000, CRC(880a02c7) SHA1(ab1b2d24be4571a183b230d267c6c8167d4a42a4) )
ROM_END


ROM_START( mazebl ) // Trog bootleg. 2-PCB set, upper one has a RC 0112 sticker
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )
	ROM_LOAD( "1", 0x10000, 0x10000, CRC(cf3a206e) SHA1(a30bab30ef546cad3ad65a93a67c68b1fff51e79) )
	ROM_CONTINUE(  0x30000, 0x10000 )
	ROM_CONTINUE(  0x50000, 0x10000 )
	ROM_CONTINUE(  0x70000, 0x10000 )
	ROM_COPY( "cvsd:cpu", 0x10000, 0x20000, 0x10000 )
	ROM_COPY( "cvsd:cpu", 0x30000, 0x40000, 0x10000 )
	ROM_COPY( "cvsd:cpu", 0x50000, 0x60000, 0x10000 )
	ROM_COPY( "cvsd:cpu", 0x70000, 0x80000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "2", 0xc0000, 0x20000, CRC(e6095189) SHA1(a2caaf64e371050b37c63d9608ba5d289cf3cd91) ) // == trog_ii_u-105_la-4.u105
	ROM_LOAD16_BYTE( "3", 0xc0001, 0x20000, CRC(fdd7cc65) SHA1(bfc4339953c122bca968f9cfa3a82df3584a3727) ) // == trog_ii_u-89_la-4.u89

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "4", 0x000000, 0x80000, CRC(dd7cd402) SHA1(78419fddd98bc37d5382f22d2ce2615948f80d5f) )
	ROM_LOAD( "6", 0x080000, 0x40000, CRC(9c001d17) SHA1(c0770fa3968355e06e7c99bff56edbf834c43196) )
	ROM_LOAD( "5", 0x200000, 0x80000, CRC(e036e21a) SHA1(584d06e82f1df571851a22fcb57248d03e49c233) )
	ROM_LOAD( "7", 0x280000, 0x40000, CRC(c5ce99fa) SHA1(29e8d7935816355d2e9d9deca3f5950ddd30a632) )
ROM_END


ROM_START( smashtv )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u4.u4",   0x10000, 0x10000, CRC(29d3f6c8) SHA1(8a90cdff54f59ddb7dba521504d880515a59df08) )
	ROM_RELOAD(                                  0x20000, 0x10000 )
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u19.u19", 0x30000, 0x10000, CRC(ac5a402a) SHA1(c476018062126dc3936caa2c328de490737165ec) ) // == SL1 SMASH T.V. SOUND ROM U19
	ROM_RELOAD(                                  0x40000, 0x10000 )
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u20.u20", 0x50000, 0x10000, CRC(875c66d9) SHA1(51cdad62ec57e69bba6fcf14e59841ec628dec11) ) // == SL1 SMASH T.V. SOUND ROM U20
	ROM_RELOAD(                                  0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la8_smash_tv_game_rom_u105.u105", 0xc0000, 0x20000, CRC(48cd793f) SHA1(7d0d9edccf0610f57e40934ab33e32315369656d) )
	ROM_LOAD16_BYTE( "la8_smash_tv_game_rom_u89.u89",   0xc0001, 0x20000, CRC(8e7fe463) SHA1(629332be706cda26f8b170b8e2877355230119ee) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_smash_tv_game_rom_u111.u111",  0x000000, 0x20000, CRC(72f0ba84) SHA1(2e925b3cdd3c8e14046b3948d82f0f3cde3c22c5) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u112.u112",  0x020000, 0x20000, CRC(436f0283) SHA1(ec33a8942c0fc326db885e08dad9346ec5a63360) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u113.u113",  0x040000, 0x20000, CRC(4a4b8110) SHA1(9f1881d1d2682764ab85aebd685d97eb8b4afe46) )

	ROM_LOAD ( "la1_smash_tv_game_rom_u95.u95",    0x200000, 0x20000, CRC(e864a44b) SHA1(40eb8e11a183f4f82dc8decb36aaeded9cd1bc26) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u96.u96",    0x220000, 0x20000, CRC(15555ea7) SHA1(4fefc059736ca424dc05a08cb55b9acf9e31228b) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u97.u97",    0x240000, 0x20000, CRC(ccac9d9e) SHA1(a43d70d1a0bbd377f0fc539c2e8b725f7079f463) )

	ROM_LOAD ( "la1_smash_tv_game_rom_u106.u106",  0x400000, 0x20000, CRC(5c718361) SHA1(6178b1d53411f24d5a5a01559727e300cd27d587) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u107.u107",  0x420000, 0x20000, CRC(0fba1e36) SHA1(17038cf35a72678bba149a632f1ad1b80cc3a38c) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u108.u108",  0x440000, 0x20000, CRC(cb0a092f) SHA1(33cbb87b4be1eadb1f3624ef5e218e65109fa3eb) )
ROM_END


ROM_START( smashtv6 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u4.u4",   0x10000, 0x10000, CRC(29d3f6c8) SHA1(8a90cdff54f59ddb7dba521504d880515a59df08) )
	ROM_RELOAD(                                  0x20000, 0x10000 )
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u19.u19", 0x30000, 0x10000, CRC(ac5a402a) SHA1(c476018062126dc3936caa2c328de490737165ec) ) // == SL1 SMASH T.V. SOUND ROM U19
	ROM_RELOAD(                                  0x40000, 0x10000 )
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u20.u20", 0x50000, 0x10000, CRC(875c66d9) SHA1(51cdad62ec57e69bba6fcf14e59841ec628dec11) ) // == SL1 SMASH T.V. SOUND ROM U20
	ROM_RELOAD(                                  0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la6_smash_tv_game_rom_u105.u105", 0xc0000, 0x20000, CRC(f1666017) SHA1(2283e71ad55a7cd3bc97bd6b20aebb90ad618bf8) )
	ROM_LOAD16_BYTE( "la6_smash_tv_game_rom_u89.u89",   0xc0001, 0x20000, CRC(908aca5d) SHA1(c97f05ecb8d96306fecef40330331e279d29f78d) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_smash_tv_game_rom_u111.u111",  0x000000, 0x20000, CRC(72f0ba84) SHA1(2e925b3cdd3c8e14046b3948d82f0f3cde3c22c5) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u112.u112",  0x020000, 0x20000, CRC(436f0283) SHA1(ec33a8942c0fc326db885e08dad9346ec5a63360) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u113.u113",  0x040000, 0x20000, CRC(4a4b8110) SHA1(9f1881d1d2682764ab85aebd685d97eb8b4afe46) )

	ROM_LOAD ( "la1_smash_tv_game_rom_u95.u95",    0x200000, 0x20000, CRC(e864a44b) SHA1(40eb8e11a183f4f82dc8decb36aaeded9cd1bc26) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u96.u96",    0x220000, 0x20000, CRC(15555ea7) SHA1(4fefc059736ca424dc05a08cb55b9acf9e31228b) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u97.u97",    0x240000, 0x20000, CRC(ccac9d9e) SHA1(a43d70d1a0bbd377f0fc539c2e8b725f7079f463) )

	ROM_LOAD ( "la1_smash_tv_game_rom_u106.u106",  0x400000, 0x20000, CRC(5c718361) SHA1(6178b1d53411f24d5a5a01559727e300cd27d587) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u107.u107",  0x420000, 0x20000, CRC(0fba1e36) SHA1(17038cf35a72678bba149a632f1ad1b80cc3a38c) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u108.u108",  0x440000, 0x20000, CRC(cb0a092f) SHA1(33cbb87b4be1eadb1f3624ef5e218e65109fa3eb) )
ROM_END


ROM_START( smashtv5 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 ) // sound CPU
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u4.u4",   0x10000, 0x10000, CRC(29d3f6c8) SHA1(8a90cdff54f59ddb7dba521504d880515a59df08) )
	ROM_RELOAD(                                  0x20000, 0x10000 )
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u19.u19", 0x30000, 0x10000, CRC(ac5a402a) SHA1(c476018062126dc3936caa2c328de490737165ec) ) // == SL1 SMASH T.V. SOUND ROM U19
	ROM_RELOAD(                                  0x40000, 0x10000 )
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u20.u20", 0x50000, 0x10000, CRC(875c66d9) SHA1(51cdad62ec57e69bba6fcf14e59841ec628dec11) ) // == SL1 SMASH T.V. SOUND ROM U20
	ROM_RELOAD(                                  0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la5_smash_tv_game_rom_u105.u105", 0xc0000, 0x20000, CRC(81f564b9) SHA1(5bddcda054be6766b40af88ae2519b3a87c33667) )
	ROM_LOAD16_BYTE( "la5_smash_tv_game_rom_u89.u89",   0xc0001, 0x20000, CRC(e5017d25) SHA1(27e544efa7f5cbe6ed3fc3211b12694c15a316c7) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_smash_tv_game_rom_u111.u111",  0x000000, 0x20000, CRC(72f0ba84) SHA1(2e925b3cdd3c8e14046b3948d82f0f3cde3c22c5) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u112.u112",  0x020000, 0x20000, CRC(436f0283) SHA1(ec33a8942c0fc326db885e08dad9346ec5a63360) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u113.u113",  0x040000, 0x20000, CRC(4a4b8110) SHA1(9f1881d1d2682764ab85aebd685d97eb8b4afe46) )

	ROM_LOAD ( "la1_smash_tv_game_rom_u95.u95",    0x200000, 0x20000, CRC(e864a44b) SHA1(40eb8e11a183f4f82dc8decb36aaeded9cd1bc26) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u96.u96",    0x220000, 0x20000, CRC(15555ea7) SHA1(4fefc059736ca424dc05a08cb55b9acf9e31228b) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u97.u97",    0x240000, 0x20000, CRC(ccac9d9e) SHA1(a43d70d1a0bbd377f0fc539c2e8b725f7079f463) )

	ROM_LOAD ( "la1_smash_tv_game_rom_u106.u106",  0x400000, 0x20000, CRC(5c718361) SHA1(6178b1d53411f24d5a5a01559727e300cd27d587) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u107.u107",  0x420000, 0x20000, CRC(0fba1e36) SHA1(17038cf35a72678bba149a632f1ad1b80cc3a38c) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u108.u108",  0x440000, 0x20000, CRC(cb0a092f) SHA1(33cbb87b4be1eadb1f3624ef5e218e65109fa3eb) )
ROM_END


ROM_START( smashtv4 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u4.u4",   0x10000, 0x10000, CRC(29d3f6c8) SHA1(8a90cdff54f59ddb7dba521504d880515a59df08) )
	ROM_RELOAD(                                  0x20000, 0x10000 )
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u19.u19", 0x30000, 0x10000, CRC(ac5a402a) SHA1(c476018062126dc3936caa2c328de490737165ec) ) // == SL1 SMASH T.V. SOUND ROM U19
	ROM_RELOAD(                                  0x40000, 0x10000 )
	ROM_LOAD ( "sl2_smash_tv_sound_rom_u20.u20", 0x50000, 0x10000, CRC(875c66d9) SHA1(51cdad62ec57e69bba6fcf14e59841ec628dec11) ) // == SL1 SMASH T.V. SOUND ROM U20
	ROM_RELOAD(                                  0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la4_smash_tv_game_rom_u105.u105", 0xc0000, 0x20000, CRC(a50ccb71) SHA1(414dfe355e314f6460ce07edbdd5e4b801451cf8) )
	ROM_LOAD16_BYTE( "la4_smash_tv_game_rom_u89.u89",   0xc0001, 0x20000, CRC(ef0b0279) SHA1(baad5a2a8d51d007e365f378f3214bbd2ea9699c) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_smash_tv_game_rom_u111.u111",  0x000000, 0x20000, CRC(72f0ba84) SHA1(2e925b3cdd3c8e14046b3948d82f0f3cde3c22c5) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u112.u112",  0x020000, 0x20000, CRC(436f0283) SHA1(ec33a8942c0fc326db885e08dad9346ec5a63360) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u113.u113",  0x040000, 0x20000, CRC(4a4b8110) SHA1(9f1881d1d2682764ab85aebd685d97eb8b4afe46) )

	ROM_LOAD ( "la1_smash_tv_game_rom_u95.u95",    0x200000, 0x20000, CRC(e864a44b) SHA1(40eb8e11a183f4f82dc8decb36aaeded9cd1bc26) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u96.u96",    0x220000, 0x20000, CRC(15555ea7) SHA1(4fefc059736ca424dc05a08cb55b9acf9e31228b) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u97.u97",    0x240000, 0x20000, CRC(ccac9d9e) SHA1(a43d70d1a0bbd377f0fc539c2e8b725f7079f463) )

	ROM_LOAD ( "la1_smash_tv_game_rom_u106.u106",  0x400000, 0x20000, CRC(5c718361) SHA1(6178b1d53411f24d5a5a01559727e300cd27d587) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u107.u107",  0x420000, 0x20000, CRC(0fba1e36) SHA1(17038cf35a72678bba149a632f1ad1b80cc3a38c) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u108.u108",  0x440000, 0x20000, CRC(cb0a092f) SHA1(33cbb87b4be1eadb1f3624ef5e218e65109fa3eb) )
ROM_END


ROM_START( smashtv3 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_smash_tv_sound_rom_u4.u4",   0x10000, 0x10000, CRC(a3370987) SHA1(e03c9980a243200a8c0f1ad546868c77991a6f53) )
	ROM_RELOAD(                                  0x20000, 0x10000 )
	ROM_LOAD ( "sl1_smash_tv_sound_rom_u19.u19", 0x30000, 0x10000, CRC(ac5a402a) SHA1(c476018062126dc3936caa2c328de490737165ec) )
	ROM_RELOAD(                                  0x40000, 0x10000 )
	ROM_LOAD ( "sl1_smash_tv_sound_rom_u20.u20", 0x50000, 0x10000, CRC(875c66d9) SHA1(51cdad62ec57e69bba6fcf14e59841ec628dec11) )
	ROM_RELOAD(                                  0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la3_smash_tv_game_rom_u105.u105", 0xc0000, 0x20000, CRC(33b626c3) SHA1(8f0582f6fe08dc7de920aeac578ed570ca4e717f) )
	ROM_LOAD16_BYTE( "la3_smash_tv_game_rom_u89.u89",   0xc0001, 0x20000, CRC(5f6fbc25) SHA1(d623f5e64ff4e70e24d770ac3ac0d32ff3928ce0) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_smash_tv_game_rom_u111.u111",  0x000000, 0x20000, CRC(72f0ba84) SHA1(2e925b3cdd3c8e14046b3948d82f0f3cde3c22c5) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u112.u112",  0x020000, 0x20000, CRC(436f0283) SHA1(ec33a8942c0fc326db885e08dad9346ec5a63360) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u113.u113",  0x040000, 0x20000, CRC(4a4b8110) SHA1(9f1881d1d2682764ab85aebd685d97eb8b4afe46) )

	ROM_LOAD ( "la1_smash_tv_game_rom_u95.u95",    0x200000, 0x20000, CRC(e864a44b) SHA1(40eb8e11a183f4f82dc8decb36aaeded9cd1bc26) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u96.u96",    0x220000, 0x20000, CRC(15555ea7) SHA1(4fefc059736ca424dc05a08cb55b9acf9e31228b) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u97.u97",    0x240000, 0x20000, CRC(ccac9d9e) SHA1(a43d70d1a0bbd377f0fc539c2e8b725f7079f463) )

	ROM_LOAD ( "la1_smash_tv_game_rom_u106.u106",  0x400000, 0x20000, CRC(5c718361) SHA1(6178b1d53411f24d5a5a01559727e300cd27d587) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u107.u107",  0x420000, 0x20000, CRC(0fba1e36) SHA1(17038cf35a72678bba149a632f1ad1b80cc3a38c) )
	ROM_LOAD ( "la1_smash_tv_game_rom_u108.u108",  0x440000, 0x20000, CRC(cb0a092f) SHA1(33cbb87b4be1eadb1f3624ef5e218e65109fa3eb) )
ROM_END


ROM_START( hiimpact )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_high_impact_sound_u4.u4",   0x10000, 0x20000, CRC(28effd6a) SHA1(4a839f15e1b453a22fdef7b1801b8cc5cfdf3c29) )
	ROM_LOAD ( "sl1_high_impact_sound_u19.u19", 0x30000, 0x20000, CRC(0ea22c89) SHA1(6d4579f6b10cac685be01348451b3537a0626034) )
	ROM_LOAD ( "sl1_high_impact_sound_u20.u20", 0x50000, 0x20000, CRC(4e747ab5) SHA1(82040f40aac7dae577376a742eadaaa9644500c1) )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la5_high_impact_game_rom_u105.u105", 0xc0000, 0x20000, CRC(104c30e7) SHA1(62b48b9c20730ffbaa1810650ff55aba14b6880d) )
	ROM_LOAD16_BYTE( "la5_high_impact_game_rom_u89.u89",   0xc0001, 0x20000, CRC(07aa0010) SHA1(7dfd34028afeea4444e70c40fa30c6576ff22f7d) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_high_impact_game_rom_u111.u111", 0x000000, 0x20000, CRC(49560560) SHA1(03d51e6019afa9a396c91a484969be4922fa4c99) )
	ROM_LOAD ( "la1_high_impact_game_rom_u112.u112", 0x020000, 0x20000, CRC(4dd879dc) SHA1(ac4f02fcb933df38f1ebf51b109092b77563b684) )
	ROM_LOAD ( "la1_high_impact_game_rom_u113.u113", 0x040000, 0x20000, CRC(b67aeb70) SHA1(dd1512329c46da4254712712b6f847544f4487bd) )
	ROM_LOAD ( "la1_high_impact_game_rom_u114.u114", 0x060000, 0x20000, CRC(9a4bc44b) SHA1(309eb5214fe5e1fe64d724d515190a31fc524aae) )

	ROM_LOAD ( "la1_high_impact_game_rom_u95.u95",   0x200000, 0x20000, CRC(e1352dc0) SHA1(7faa2cfa9ebaf2d99b243232316221b672869703) )
	ROM_LOAD ( "la1_high_impact_game_rom_u96.u96",   0x220000, 0x20000, CRC(197d0f34) SHA1(2d544588c3241423188ac7fb7aff87043fdd063d) )
	ROM_LOAD ( "la1_high_impact_game_rom_u97.u97",   0x240000, 0x20000, CRC(908ea575) SHA1(79802d8df4e016d178be98333d2b1d047a27eccc) )
	ROM_LOAD ( "la1_high_impact_game_rom_u98.u98",   0x260000, 0x20000, CRC(6dcbab11) SHA1(7432172810fd4b922b61769c68d86f24769a42cf) )

	ROM_LOAD ( "la1_high_impact_game_rom_u106.u106", 0x400000, 0x20000, CRC(7d0ead0d) SHA1(1e65b6e7e629021d70603df37db5fa89cfe93175) )
	ROM_LOAD ( "la1_high_impact_game_rom_u107.u107", 0x420000, 0x20000, CRC(ef48e8fa) SHA1(538de37cd8342085ec27f67292a7eeb1007e3b1f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u108.u108", 0x440000, 0x20000, CRC(5f363e12) SHA1(da398c0204f785aad4c52007d2f25031ecc1c63f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u109.u109", 0x460000, 0x20000, CRC(3689fbbc) SHA1(d95c0a2e3abf977ba7a899e419c22d004020c560) )

	ROM_REGION( 0x1100, "plds", 0 )
	ROM_LOAD( "ep610.u8",    0x0000, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pls153a.u40", 0x0400, 0x00eb, CRC(69e5143f) SHA1(1a1e7b3233f7d5a1c161564710e8e984a9b0a16c) )
	ROM_LOAD( "ep610.u51",   0x0500, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u52",   0x0900, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u65",   0x0d00, 0x032f, NO_DUMP ) // PAL is read protected
ROM_END


ROM_START( hiimpact4 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_high_impact_sound_u4.u4",   0x10000, 0x20000, CRC(28effd6a) SHA1(4a839f15e1b453a22fdef7b1801b8cc5cfdf3c29) )
	ROM_LOAD ( "sl1_high_impact_sound_u19.u19", 0x30000, 0x20000, CRC(0ea22c89) SHA1(6d4579f6b10cac685be01348451b3537a0626034) )
	ROM_LOAD ( "sl1_high_impact_sound_u20.u20", 0x50000, 0x20000, CRC(4e747ab5) SHA1(82040f40aac7dae577376a742eadaaa9644500c1) )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la4_high_impact_game_rom_u105.u105", 0xc0000, 0x20000, CRC(5f67f823) SHA1(4171b6949682d1b2180e39d44c4e0033c4c07149) )
	ROM_LOAD16_BYTE( "la4_high_impact_game_rom_u89.u89",   0xc0001, 0x20000, CRC(404d260b) SHA1(46bb44b3f1895d3424dba7664f198bce7dee911d) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_high_impact_game_rom_u111.u111", 0x000000, 0x20000, CRC(49560560) SHA1(03d51e6019afa9a396c91a484969be4922fa4c99) )
	ROM_LOAD ( "la1_high_impact_game_rom_u112.u112", 0x020000, 0x20000, CRC(4dd879dc) SHA1(ac4f02fcb933df38f1ebf51b109092b77563b684) )
	ROM_LOAD ( "la1_high_impact_game_rom_u113.u113", 0x040000, 0x20000, CRC(b67aeb70) SHA1(dd1512329c46da4254712712b6f847544f4487bd) )
	ROM_LOAD ( "la1_high_impact_game_rom_u114.u114", 0x060000, 0x20000, CRC(9a4bc44b) SHA1(309eb5214fe5e1fe64d724d515190a31fc524aae) )

	ROM_LOAD ( "la1_high_impact_game_rom_u95.u95",   0x200000, 0x20000, CRC(e1352dc0) SHA1(7faa2cfa9ebaf2d99b243232316221b672869703) )
	ROM_LOAD ( "la1_high_impact_game_rom_u96.u96",   0x220000, 0x20000, CRC(197d0f34) SHA1(2d544588c3241423188ac7fb7aff87043fdd063d) )
	ROM_LOAD ( "la1_high_impact_game_rom_u97.u97",   0x240000, 0x20000, CRC(908ea575) SHA1(79802d8df4e016d178be98333d2b1d047a27eccc) )
	ROM_LOAD ( "la1_high_impact_game_rom_u98.u98",   0x260000, 0x20000, CRC(6dcbab11) SHA1(7432172810fd4b922b61769c68d86f24769a42cf) )

	ROM_LOAD ( "la1_high_impact_game_rom_u106.u106", 0x400000, 0x20000, CRC(7d0ead0d) SHA1(1e65b6e7e629021d70603df37db5fa89cfe93175) )
	ROM_LOAD ( "la1_high_impact_game_rom_u107.u107", 0x420000, 0x20000, CRC(ef48e8fa) SHA1(538de37cd8342085ec27f67292a7eeb1007e3b1f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u108.u108", 0x440000, 0x20000, CRC(5f363e12) SHA1(da398c0204f785aad4c52007d2f25031ecc1c63f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u109.u109", 0x460000, 0x20000, CRC(3689fbbc) SHA1(d95c0a2e3abf977ba7a899e419c22d004020c560) )

	ROM_REGION( 0x1100, "plds", 0 )
	ROM_LOAD( "ep610.u8",    0x0000, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pls153a.u40", 0x0400, 0x00eb, CRC(69e5143f) SHA1(1a1e7b3233f7d5a1c161564710e8e984a9b0a16c) )
	ROM_LOAD( "ep610.u51",   0x0500, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u52",   0x0900, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u65",   0x0d00, 0x032f, NO_DUMP ) // PAL is read protected
ROM_END


ROM_START( hiimpact3 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_high_impact_sound_u4.u4",   0x10000, 0x20000, CRC(28effd6a) SHA1(4a839f15e1b453a22fdef7b1801b8cc5cfdf3c29) )
	ROM_LOAD ( "sl1_high_impact_sound_u19.u19", 0x30000, 0x20000, CRC(0ea22c89) SHA1(6d4579f6b10cac685be01348451b3537a0626034) )
	ROM_LOAD ( "sl1_high_impact_sound_u20.u20", 0x50000, 0x20000, CRC(4e747ab5) SHA1(82040f40aac7dae577376a742eadaaa9644500c1) )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la3_high_impact_game_rom_u105.u105", 0xc0000, 0x20000, CRC(b9190c4a) SHA1(adcf1023d62f67fbde7a7a7aeeda068d7711f7cf) )
	ROM_LOAD16_BYTE( "la3_high_impact_game_rom_u89.u89",   0xc0001, 0x20000, CRC(1cbc72a5) SHA1(ba0b4b54453fcd1888d40690848e0ee4150bb8e1) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_high_impact_game_rom_u111.u111", 0x000000, 0x20000, CRC(49560560) SHA1(03d51e6019afa9a396c91a484969be4922fa4c99) )
	ROM_LOAD ( "la1_high_impact_game_rom_u112.u112", 0x020000, 0x20000, CRC(4dd879dc) SHA1(ac4f02fcb933df38f1ebf51b109092b77563b684) )
	ROM_LOAD ( "la1_high_impact_game_rom_u113.u113", 0x040000, 0x20000, CRC(b67aeb70) SHA1(dd1512329c46da4254712712b6f847544f4487bd) )
	ROM_LOAD ( "la1_high_impact_game_rom_u114.u114", 0x060000, 0x20000, CRC(9a4bc44b) SHA1(309eb5214fe5e1fe64d724d515190a31fc524aae) )

	ROM_LOAD ( "la1_high_impact_game_rom_u95.u95",   0x200000, 0x20000, CRC(e1352dc0) SHA1(7faa2cfa9ebaf2d99b243232316221b672869703) )
	ROM_LOAD ( "la1_high_impact_game_rom_u96.u96",   0x220000, 0x20000, CRC(197d0f34) SHA1(2d544588c3241423188ac7fb7aff87043fdd063d) )
	ROM_LOAD ( "la1_high_impact_game_rom_u97.u97",   0x240000, 0x20000, CRC(908ea575) SHA1(79802d8df4e016d178be98333d2b1d047a27eccc) )
	ROM_LOAD ( "la1_high_impact_game_rom_u98.u98",   0x260000, 0x20000, CRC(6dcbab11) SHA1(7432172810fd4b922b61769c68d86f24769a42cf) )

	ROM_LOAD ( "la1_high_impact_game_rom_u106.u106", 0x400000, 0x20000, CRC(7d0ead0d) SHA1(1e65b6e7e629021d70603df37db5fa89cfe93175) )
	ROM_LOAD ( "la1_high_impact_game_rom_u107.u107", 0x420000, 0x20000, CRC(ef48e8fa) SHA1(538de37cd8342085ec27f67292a7eeb1007e3b1f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u108.u108", 0x440000, 0x20000, CRC(5f363e12) SHA1(da398c0204f785aad4c52007d2f25031ecc1c63f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u109.u109", 0x460000, 0x20000, CRC(3689fbbc) SHA1(d95c0a2e3abf977ba7a899e419c22d004020c560) )

	ROM_REGION( 0x1100, "plds", 0 )
	ROM_LOAD( "ep610.u8",    0x0000, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pls153a.u40", 0x0400, 0x00eb, CRC(69e5143f) SHA1(1a1e7b3233f7d5a1c161564710e8e984a9b0a16c) )
	ROM_LOAD( "ep610.u51",   0x0500, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u52",   0x0900, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u65",   0x0d00, 0x032f, NO_DUMP ) // PAL is read protected
ROM_END


ROM_START( hiimpact2 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_high_impact_sound_u4.u4",   0x10000, 0x20000, CRC(28effd6a) SHA1(4a839f15e1b453a22fdef7b1801b8cc5cfdf3c29) )
	ROM_LOAD ( "sl1_high_impact_sound_u19.u19", 0x30000, 0x20000, CRC(0ea22c89) SHA1(6d4579f6b10cac685be01348451b3537a0626034) )
	ROM_LOAD ( "sl1_high_impact_sound_u20.u20", 0x50000, 0x20000, CRC(4e747ab5) SHA1(82040f40aac7dae577376a742eadaaa9644500c1) )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la2_high_impact_game_rom_u105.u105", 0xc0000, 0x20000, CRC(25d83ba1) SHA1(4422a34b2957aabb3f0a26ca129e290dfc062933) )
	ROM_LOAD16_BYTE( "la2_high_impact_game_rom_u89.u89",   0xc0001, 0x20000, CRC(811f1253) SHA1(125a6cca26d37fae343b78046774f54ee6e8d996) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_high_impact_game_rom_u111.u111", 0x000000, 0x20000, CRC(49560560) SHA1(03d51e6019afa9a396c91a484969be4922fa4c99) )
	ROM_LOAD ( "la1_high_impact_game_rom_u112.u112", 0x020000, 0x20000, CRC(4dd879dc) SHA1(ac4f02fcb933df38f1ebf51b109092b77563b684) )
	ROM_LOAD ( "la1_high_impact_game_rom_u113.u113", 0x040000, 0x20000, CRC(b67aeb70) SHA1(dd1512329c46da4254712712b6f847544f4487bd) )
	ROM_LOAD ( "la1_high_impact_game_rom_u114.u114", 0x060000, 0x20000, CRC(9a4bc44b) SHA1(309eb5214fe5e1fe64d724d515190a31fc524aae) )

	ROM_LOAD ( "la1_high_impact_game_rom_u95.u95",   0x200000, 0x20000, CRC(e1352dc0) SHA1(7faa2cfa9ebaf2d99b243232316221b672869703) )
	ROM_LOAD ( "la1_high_impact_game_rom_u96.u96",   0x220000, 0x20000, CRC(197d0f34) SHA1(2d544588c3241423188ac7fb7aff87043fdd063d) )
	ROM_LOAD ( "la1_high_impact_game_rom_u97.u97",   0x240000, 0x20000, CRC(908ea575) SHA1(79802d8df4e016d178be98333d2b1d047a27eccc) )
	ROM_LOAD ( "la1_high_impact_game_rom_u98.u98",   0x260000, 0x20000, CRC(6dcbab11) SHA1(7432172810fd4b922b61769c68d86f24769a42cf) )

	ROM_LOAD ( "la1_high_impact_game_rom_u106.u106", 0x400000, 0x20000, CRC(7d0ead0d) SHA1(1e65b6e7e629021d70603df37db5fa89cfe93175) )
	ROM_LOAD ( "la1_high_impact_game_rom_u107.u107", 0x420000, 0x20000, CRC(ef48e8fa) SHA1(538de37cd8342085ec27f67292a7eeb1007e3b1f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u108.u108", 0x440000, 0x20000, CRC(5f363e12) SHA1(da398c0204f785aad4c52007d2f25031ecc1c63f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u109.u109", 0x460000, 0x20000, CRC(3689fbbc) SHA1(d95c0a2e3abf977ba7a899e419c22d004020c560) )

	ROM_REGION( 0x1100, "plds", 0 )
	ROM_LOAD( "ep610.u8",    0x0000, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pls153a.u40", 0x0400, 0x00eb, CRC(69e5143f) SHA1(1a1e7b3233f7d5a1c161564710e8e984a9b0a16c) )
	ROM_LOAD( "ep610.u51",   0x0500, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u52",   0x0900, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u65",   0x0d00, 0x032f, NO_DUMP ) // PAL is read protected
ROM_END


ROM_START( hiimpact1 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_high_impact_sound_u4.u4",   0x10000, 0x20000, CRC(28effd6a) SHA1(4a839f15e1b453a22fdef7b1801b8cc5cfdf3c29) )
	ROM_LOAD ( "sl1_high_impact_sound_u19.u19", 0x30000, 0x20000, CRC(0ea22c89) SHA1(6d4579f6b10cac685be01348451b3537a0626034) )
	ROM_LOAD ( "sl1_high_impact_sound_u20.u20", 0x50000, 0x20000, CRC(4e747ab5) SHA1(82040f40aac7dae577376a742eadaaa9644500c1) )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la1_high_impact_game_rom_u105.u105", 0xc0000, 0x20000, CRC(e86228ba) SHA1(0af263e51cb65115038ee5bf508515674e05913e) )
	ROM_LOAD16_BYTE( "la1_high_impact_game_rom_u89.u89",   0xc0001, 0x20000, CRC(f23e972e) SHA1(e5ae5eaf5f97ec271b92072fd674e8cd93b36778) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_high_impact_game_rom_u111.u111", 0x000000, 0x20000, CRC(49560560) SHA1(03d51e6019afa9a396c91a484969be4922fa4c99) ) // All ROMs had a "LA1" sticker over a "PA1" label
	ROM_LOAD ( "la1_high_impact_game_rom_u112.u112", 0x020000, 0x20000, CRC(4dd879dc) SHA1(ac4f02fcb933df38f1ebf51b109092b77563b684) )
	ROM_LOAD ( "la1_high_impact_game_rom_u113.u113", 0x040000, 0x20000, CRC(b67aeb70) SHA1(dd1512329c46da4254712712b6f847544f4487bd) )
	ROM_LOAD ( "la1_high_impact_game_rom_u114.u114", 0x060000, 0x20000, CRC(9a4bc44b) SHA1(309eb5214fe5e1fe64d724d515190a31fc524aae) )

	ROM_LOAD ( "la1_high_impact_game_rom_u95.u95",   0x200000, 0x20000, CRC(e1352dc0) SHA1(7faa2cfa9ebaf2d99b243232316221b672869703) )
	ROM_LOAD ( "la1_high_impact_game_rom_u96.u96",   0x220000, 0x20000, CRC(197d0f34) SHA1(2d544588c3241423188ac7fb7aff87043fdd063d) )
	ROM_LOAD ( "la1_high_impact_game_rom_u97.u97",   0x240000, 0x20000, CRC(908ea575) SHA1(79802d8df4e016d178be98333d2b1d047a27eccc) )
	ROM_LOAD ( "la1_high_impact_game_rom_u98.u98",   0x260000, 0x20000, CRC(6dcbab11) SHA1(7432172810fd4b922b61769c68d86f24769a42cf) )

	ROM_LOAD ( "la1_high_impact_game_rom_u106.u106", 0x400000, 0x20000, CRC(7d0ead0d) SHA1(1e65b6e7e629021d70603df37db5fa89cfe93175) )
	ROM_LOAD ( "la1_high_impact_game_rom_u107.u107", 0x420000, 0x20000, CRC(ef48e8fa) SHA1(538de37cd8342085ec27f67292a7eeb1007e3b1f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u108.u108", 0x440000, 0x20000, CRC(5f363e12) SHA1(da398c0204f785aad4c52007d2f25031ecc1c63f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u109.u109", 0x460000, 0x20000, CRC(3689fbbc) SHA1(d95c0a2e3abf977ba7a899e419c22d004020c560) )

	ROM_REGION( 0x1100, "plds", 0 )
	ROM_LOAD( "ep610.u8",    0x0000, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pls153a.u40", 0x0400, 0x00eb, CRC(69e5143f) SHA1(1a1e7b3233f7d5a1c161564710e8e984a9b0a16c) )
	ROM_LOAD( "ep610.u51",   0x0500, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u52",   0x0900, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u65",   0x0d00, 0x032f, NO_DUMP ) // PAL is read protected
ROM_END


ROM_START( hiimpactp )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_high_impact_sound_u4.u4",   0x10000, 0x20000, CRC(28effd6a) SHA1(4a839f15e1b453a22fdef7b1801b8cc5cfdf3c29) )
	ROM_LOAD ( "sl1_high_impact_sound_u19.u19", 0x30000, 0x20000, CRC(0ea22c89) SHA1(6d4579f6b10cac685be01348451b3537a0626034) )
	ROM_LOAD ( "sl1_high_impact_sound_u20.u20", 0x50000, 0x20000, CRC(4e747ab5) SHA1(82040f40aac7dae577376a742eadaaa9644500c1) )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "pa1_high_impact_game_rom_u105.u105", 0xc0000, 0x20000, CRC(79ef9a35) SHA1(200d50b108401e889b6200c53c203ee5041d1423) )
	ROM_LOAD16_BYTE( "pa1_high_impact_game_rom_u89.u89",   0xc0001, 0x20000, CRC(2bd3de30) SHA1(ee3615c1cc5b948731eb258887641f059b942b25) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_high_impact_game_rom_u111.u111", 0x000000, 0x20000, CRC(49560560) SHA1(03d51e6019afa9a396c91a484969be4922fa4c99) ) // All ROMs were labeled "PA 1" which Williams later designated "LA 1"
	ROM_LOAD ( "la1_high_impact_game_rom_u112.u112", 0x020000, 0x20000, CRC(4dd879dc) SHA1(ac4f02fcb933df38f1ebf51b109092b77563b684) ) // without changing any of the contents when they release ver LA 1
	ROM_LOAD ( "la1_high_impact_game_rom_u113.u113", 0x040000, 0x20000, CRC(b67aeb70) SHA1(dd1512329c46da4254712712b6f847544f4487bd) )
	ROM_LOAD ( "la1_high_impact_game_rom_u114.u114", 0x060000, 0x20000, CRC(9a4bc44b) SHA1(309eb5214fe5e1fe64d724d515190a31fc524aae) )

	ROM_LOAD ( "la1_high_impact_game_rom_u95.u95",   0x200000, 0x20000, CRC(e1352dc0) SHA1(7faa2cfa9ebaf2d99b243232316221b672869703) )
	ROM_LOAD ( "la1_high_impact_game_rom_u96.u96",   0x220000, 0x20000, CRC(197d0f34) SHA1(2d544588c3241423188ac7fb7aff87043fdd063d) )
	ROM_LOAD ( "la1_high_impact_game_rom_u97.u97",   0x240000, 0x20000, CRC(908ea575) SHA1(79802d8df4e016d178be98333d2b1d047a27eccc) )
	ROM_LOAD ( "la1_high_impact_game_rom_u98.u98",   0x260000, 0x20000, CRC(6dcbab11) SHA1(7432172810fd4b922b61769c68d86f24769a42cf) )

	ROM_LOAD ( "la1_high_impact_game_rom_u106.u106", 0x400000, 0x20000, CRC(7d0ead0d) SHA1(1e65b6e7e629021d70603df37db5fa89cfe93175) )
	ROM_LOAD ( "la1_high_impact_game_rom_u107.u107", 0x420000, 0x20000, CRC(ef48e8fa) SHA1(538de37cd8342085ec27f67292a7eeb1007e3b1f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u108.u108", 0x440000, 0x20000, CRC(5f363e12) SHA1(da398c0204f785aad4c52007d2f25031ecc1c63f) )
	ROM_LOAD ( "la1_high_impact_game_rom_u109.u109", 0x460000, 0x20000, CRC(3689fbbc) SHA1(d95c0a2e3abf977ba7a899e419c22d004020c560) )

	ROM_REGION( 0x1100, "plds", 0 )
	ROM_LOAD( "ep610.u8",    0x0000, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pls153a.u40", 0x0400, 0x00eb, CRC(69e5143f) SHA1(1a1e7b3233f7d5a1c161564710e8e984a9b0a16c) )
	ROM_LOAD( "ep610.u51",   0x0500, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u52",   0x0900, 0x032f, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "ep610.u65",   0x0d00, 0x032f, NO_DUMP ) // PAL is read protected
ROM_END


ROM_START( shimpact )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_super_high_impact_u4_sound_rom.u4",   0x10000, 0x20000, CRC(1e5a012c) SHA1(4077fc266799a01738b7f88e867535f1fbacd557) )
	ROM_LOAD ( "sl1_super_high_impact_u19_sound_rom.u19", 0x30000, 0x20000, CRC(10f9684e) SHA1(1fdc5364f87fb65f4f2a438841e0fe847f765aaf) )
	ROM_LOAD ( "sl1_super_high_impact_u20_sound_rom.u20", 0x50000, 0x20000, CRC(1b4a71c1) SHA1(74b7b4ae76ebe65f1f46b2117970bfefefbb5344) )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la1_super_high_impact_game_rom_u105.u105", 0xc0000, 0x20000, CRC(f2cf8de3) SHA1(97428d05208c18a9fcf8f2e3c6ed2bf6441350c3) )
	ROM_LOAD16_BYTE( "la1_super_high_impact_game_rom_u89.u89",   0xc0001, 0x20000, CRC(f97d9b01) SHA1(d5f39d6a5db23f5efd123cf9da0d09c84893b9c4) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u111.u111", 0x000000, 0x40000, CRC(80ae2a86) SHA1(1ff76e3064c7636f6877e426f4a88c094d1a6325) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u112.u112", 0x040000, 0x40000, CRC(3ffc27e9) SHA1(ec337629c17daaa2445fb344e08243de7f09536e) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u113.u113", 0x080000, 0x40000, CRC(01549d00) SHA1(40604e949cef056f90031850bdb91782135e7ec2) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u114.u114", 0x0c0000, 0x40000, CRC(a68af319) SHA1(9ed2e620a952dce26e08d0931f52eaeb638fc14d) )

	ROM_LOAD ( "la1_super_high_impact_game_rom_u95.u95",   0x200000, 0x40000, CRC(e8f56ef5) SHA1(7cb0b6bad3f0be822ef9b92e6523572e45776969) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u96.u96",   0x240000, 0x40000, CRC(24ed04f9) SHA1(f4e91640713c0c376861652f3f0db33bff32656d) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u97.u97",   0x280000, 0x40000, CRC(dd7f41a9) SHA1(a14a285ccc593f8f1d50b0d5574af4845a1e287e) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u98.u98",   0x2c0000, 0x40000, CRC(23ef65dd) SHA1(58400c305dfad1de18b84a8c118f72529b507414) )

	ROM_LOAD ( "la1_super_high_impact_game_rom_u106.u106", 0x400000, 0x40000, CRC(6f5bf337) SHA1(5b1a0d927302c7e1727976c2d8c612a80b8f1484) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u107.u107", 0x440000, 0x40000, CRC(a8815dad) SHA1(627d916a4b0ab03a943d123ca0eabd514634ad30) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u108.u108", 0x480000, 0x40000, CRC(d39685a3) SHA1(84e5da34a9946b954635befd37760683850d310b) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u109.u109", 0x4c0000, 0x40000, CRC(36e0b2b2) SHA1(96d76698a09cd884349bf0c4c1b75423b4404432) )
ROM_END


ROM_START( shimpactp6 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_super_high_impact_u4_sound_rom.u4",   0x10000, 0x20000, CRC(1e5a012c) SHA1(4077fc266799a01738b7f88e867535f1fbacd557) )
	ROM_LOAD ( "sl1_super_high_impact_u19_sound_rom.u19", 0x30000, 0x20000, CRC(10f9684e) SHA1(1fdc5364f87fb65f4f2a438841e0fe847f765aaf) )
	ROM_LOAD ( "sl1_super_high_impact_u20_sound_rom.u20", 0x50000, 0x20000, CRC(1b4a71c1) SHA1(74b7b4ae76ebe65f1f46b2117970bfefefbb5344) )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "proto6_super_high_impact_game_rom_u105.u105", 0xc0000, 0x20000, CRC(33e1978d) SHA1(a88af5551d6b4777e0c5f5e3844b2f1d61bbb35d) ) // also known to be labeled "PRO6" or "PA6"
	ROM_LOAD16_BYTE( "proto6_super_high_impact_game_rom_u89.u89",   0xc0001, 0x20000, CRC(6c070978) SHA1(ca6657c48810d78496c51eb750f45a3e08132ce3) ) // also known to be labeled "PRO6" or "PA6"

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u111.u111", 0x000000, 0x40000, CRC(80ae2a86) SHA1(1ff76e3064c7636f6877e426f4a88c094d1a6325) ) // also known to be labeled PA1
	ROM_LOAD ( "la1_super_high_impact_game_rom_u112.u112", 0x040000, 0x40000, CRC(3ffc27e9) SHA1(ec337629c17daaa2445fb344e08243de7f09536e) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u113.u113", 0x080000, 0x40000, CRC(01549d00) SHA1(40604e949cef056f90031850bdb91782135e7ec2) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u114.u114", 0x0c0000, 0x40000, CRC(a68af319) SHA1(9ed2e620a952dce26e08d0931f52eaeb638fc14d) )

	ROM_LOAD ( "la1_super_high_impact_game_rom_u95.u95",   0x200000, 0x40000, CRC(e8f56ef5) SHA1(7cb0b6bad3f0be822ef9b92e6523572e45776969) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u96.u96",   0x240000, 0x40000, CRC(24ed04f9) SHA1(f4e91640713c0c376861652f3f0db33bff32656d) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u97.u97",   0x280000, 0x40000, CRC(dd7f41a9) SHA1(a14a285ccc593f8f1d50b0d5574af4845a1e287e) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u98.u98",   0x2c0000, 0x40000, CRC(23ef65dd) SHA1(58400c305dfad1de18b84a8c118f72529b507414) )

	ROM_LOAD ( "la1_super_high_impact_game_rom_u106.u106", 0x400000, 0x40000, CRC(6f5bf337) SHA1(5b1a0d927302c7e1727976c2d8c612a80b8f1484) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u107.u107", 0x440000, 0x40000, CRC(a8815dad) SHA1(627d916a4b0ab03a943d123ca0eabd514634ad30) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u108.u108", 0x480000, 0x40000, CRC(d39685a3) SHA1(84e5da34a9946b954635befd37760683850d310b) )
	ROM_LOAD ( "la1_super_high_impact_game_rom_u109.u109", 0x4c0000, 0x40000, CRC(36e0b2b2) SHA1(96d76698a09cd884349bf0c4c1b75423b4404432) )
ROM_END


ROM_START( shimpactp5 )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_super_high_impact_u4_sound_rom.u4",   0x10000, 0x20000, CRC(1e5a012c) SHA1(4077fc266799a01738b7f88e867535f1fbacd557) )
	ROM_LOAD ( "sl1_super_high_impact_u19_sound_rom.u19", 0x30000, 0x20000, CRC(10f9684e) SHA1(1fdc5364f87fb65f4f2a438841e0fe847f765aaf) )
	ROM_LOAD ( "sl1_super_high_impact_u20_sound_rom.u20", 0x50000, 0x20000, CRC(1b4a71c1) SHA1(74b7b4ae76ebe65f1f46b2117970bfefefbb5344) )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "proto5_super_high_impact_game_rom_u105.u105", 0xc0000, 0x20000, CRC(4342cd45) SHA1(a8e8609efbd67a957104316a0fd4824802134290) ) // Both program ROMs had a "PROTO5" sticker
	ROM_LOAD16_BYTE( "proto5_super_high_impact_game_rom_u89.u89",   0xc0001, 0x20000, CRC(cda47b73) SHA1(9b51f7d0cd6ffa07a5880e4cc8a855c2f7616c22) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u111.u111", 0x000000, 0x40000, CRC(80ae2a86) SHA1(1ff76e3064c7636f6877e426f4a88c094d1a6325) ) // All graphic ROMs had a "PROTO3" sticker
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u112.u112", 0x040000, 0x40000, CRC(3ffc27e9) SHA1(ec337629c17daaa2445fb344e08243de7f09536e) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u113.u113", 0x080000, 0x40000, CRC(01549d00) SHA1(40604e949cef056f90031850bdb91782135e7ec2) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u114.u114", 0x0c0000, 0x40000, CRC(56f96a67) SHA1(070ba9c34c23b3037e91c2a7e0a85093c95def69) )

	ROM_LOAD ( "proto3_super_high_impact_game_rom_u95.u95",   0x200000, 0x40000, CRC(e8f56ef5) SHA1(7cb0b6bad3f0be822ef9b92e6523572e45776969) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u96.u96",   0x240000, 0x40000, CRC(24ed04f9) SHA1(f4e91640713c0c376861652f3f0db33bff32656d) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u97.u97",   0x280000, 0x40000, CRC(dd7f41a9) SHA1(a14a285ccc593f8f1d50b0d5574af4845a1e287e) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u98.u98",   0x2c0000, 0x40000, CRC(28418723) SHA1(d4eef3131c82f1ecb65d6623b195c4f76010aa1b) )

	ROM_LOAD ( "proto3_super_high_impact_game_rom_u106.u106", 0x400000, 0x40000, CRC(6f5bf337) SHA1(5b1a0d927302c7e1727976c2d8c612a80b8f1484) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u107.u107", 0x440000, 0x40000, CRC(a8815dad) SHA1(627d916a4b0ab03a943d123ca0eabd514634ad30) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u108.u108", 0x480000, 0x40000, CRC(d39685a3) SHA1(84e5da34a9946b954635befd37760683850d310b) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u109.u109", 0x4c0000, 0x40000, CRC(58f71141) SHA1(f7143bdaa7325b88e01a1d6be3aeb1d69cf0672b) )
ROM_END


ROM_START( shimpactp4 ) // You must manualy reset the high score table or game will hang after initial demo screen, it's best to do a "Full Factory Restore"
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_super_high_impact_u4_sound_rom.u4",   0x10000, 0x20000, CRC(1e5a012c) SHA1(4077fc266799a01738b7f88e867535f1fbacd557) )
	ROM_LOAD ( "sl1_super_high_impact_u19_sound_rom.u19", 0x30000, 0x20000, CRC(10f9684e) SHA1(1fdc5364f87fb65f4f2a438841e0fe847f765aaf) )
	ROM_LOAD ( "sl1_super_high_impact_u20_sound_rom.u20", 0x50000, 0x20000, CRC(1b4a71c1) SHA1(74b7b4ae76ebe65f1f46b2117970bfefefbb5344) )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "proto4_super_high_impact_game_rom_u105.u105", 0xc0000, 0x20000, CRC(770b31ce) SHA1(d0bc2ed0f6134afb0dd53236377044a122e7f181) ) // Both program ROMs had a "PROTO4" sticker
	ROM_LOAD16_BYTE( "proto4_super_high_impact_game_rom_u89.u89",   0xc0001, 0x20000, CRC(96b622a5) SHA1(6d21c69ad1b0990679b616a79ba698772c8d98ff) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u111.u111", 0x000000, 0x40000, CRC(80ae2a86) SHA1(1ff76e3064c7636f6877e426f4a88c094d1a6325) ) // All graphic ROMs had a "PROTO3" sticker
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u112.u112", 0x040000, 0x40000, CRC(3ffc27e9) SHA1(ec337629c17daaa2445fb344e08243de7f09536e) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u113.u113", 0x080000, 0x40000, CRC(01549d00) SHA1(40604e949cef056f90031850bdb91782135e7ec2) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u114.u114", 0x0c0000, 0x40000, CRC(56f96a67) SHA1(070ba9c34c23b3037e91c2a7e0a85093c95def69) )

	ROM_LOAD ( "proto3_super_high_impact_game_rom_u95.u95",   0x200000, 0x40000, CRC(e8f56ef5) SHA1(7cb0b6bad3f0be822ef9b92e6523572e45776969) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u96.u96",   0x240000, 0x40000, CRC(24ed04f9) SHA1(f4e91640713c0c376861652f3f0db33bff32656d) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u97.u97",   0x280000, 0x40000, CRC(dd7f41a9) SHA1(a14a285ccc593f8f1d50b0d5574af4845a1e287e) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u98.u98",   0x2c0000, 0x40000, CRC(28418723) SHA1(d4eef3131c82f1ecb65d6623b195c4f76010aa1b) )

	ROM_LOAD ( "proto3_super_high_impact_game_rom_u106.u106", 0x400000, 0x40000, CRC(6f5bf337) SHA1(5b1a0d927302c7e1727976c2d8c612a80b8f1484) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u107.u107", 0x440000, 0x40000, CRC(a8815dad) SHA1(627d916a4b0ab03a943d123ca0eabd514634ad30) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u108.u108", 0x480000, 0x40000, CRC(d39685a3) SHA1(84e5da34a9946b954635befd37760683850d310b) )
	ROM_LOAD ( "proto3_super_high_impact_game_rom_u109.u109", 0x4c0000, 0x40000, CRC(58f71141) SHA1(f7143bdaa7325b88e01a1d6be3aeb1d69cf0672b) )
ROM_END


ROM_START( strkforc )
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )    // sound CPU
	ROM_LOAD ( "sl1_strike_force_sound_rom_u4.u4",   0x10000, 0x10000, CRC(8f747312) SHA1(729929c209741e72eb83b407cf95d7709ec1b5ae) )
	ROM_RELOAD(                                      0x20000, 0x10000 )
	ROM_LOAD ( "sl1_strike_force_sound_rom_u19.u19", 0x30000, 0x10000, CRC(afb29926) SHA1(ad904c0968a90b8187cc87d6c171fbc021d2f66f) )
	ROM_RELOAD(                                      0x40000, 0x10000 )
	ROM_LOAD ( "sl1_strike_force_sound_rom_u20.u20", 0x50000, 0x10000, CRC(1bc9b746) SHA1(a5ad40ce7f228f30c21c5a7bdc2893c2a7fe7f58) )
	ROM_RELOAD(                                      0x60000, 0x10000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la1_strike_force_game_rom_u105.u105", 0xc0000, 0x20000, CRC(7895e0e3) SHA1(fa471af9e673a82713a590f463f87a4c59e3d5d8) )
	ROM_LOAD16_BYTE( "la1_strike_force_game_rom_u89.u89",   0xc0001, 0x20000, CRC(26114d9e) SHA1(79906966859f0ae0884b956e7d520e3cff78fab7) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_strike_force_game_rom_u111.u111", 0x000000, 0x20000, CRC(878efc80) SHA1(94448002faff5839beab5466e5a41195869face3) )
	ROM_LOAD ( "la1_strike_force_game_rom_u112.u112", 0x020000, 0x20000, CRC(93394399) SHA1(67ad5c27c3c82fa6055032df98c365c56c7f8b1b) )
	ROM_LOAD ( "la1_strike_force_game_rom_u113.u113", 0x040000, 0x20000, CRC(9565a79b) SHA1(ebb90132ed8acbbed09bbcdff435cdf60a3ef8ab) )
	ROM_LOAD ( "la1_strike_force_game_rom_u114.u114", 0x060000, 0x20000, CRC(b71152da) SHA1(784e229a5ae51776a3e984f22d1d73b2286cfc68) )

	ROM_LOAD ( "la1_strike_force_game_rom_u106.u106", 0x080000, 0x20000, CRC(a394d4cf) SHA1(d08c5994b08dafd233a270d24b4c851bcedf5cbe) )
	ROM_LOAD ( "la1_strike_force_game_rom_u107.u107", 0x0a0000, 0x20000, CRC(edef1419) SHA1(cda8de55355eabf8146a243f917f6d27babe5ce3) )

	ROM_LOAD ( "la1_strike_force_game_rom_u95.u95",   0x200000, 0x20000, CRC(519cb2b4) SHA1(9059d2ca2705bd297c066a9470b756aecb395431) )
	ROM_LOAD ( "la1_strike_force_game_rom_u96.u96",   0x220000, 0x20000, CRC(61214796) SHA1(bad32ef909f714289ee7cf2a5179a3b96678a72a) )
	ROM_LOAD ( "la1_strike_force_game_rom_u97.u97",   0x240000, 0x20000, CRC(eb5dee5f) SHA1(9432140b4c983472fdc41f36390ee4db67896475) )
	ROM_LOAD ( "la1_strike_force_game_rom_u98.u98",   0x260000, 0x20000, CRC(c5c079e7) SHA1(3cbd56db7d0eeaa6fb4f1cc8793cd1deff4e3c2c) )

	ROM_LOAD ( "la1_strike_force_game_rom_u90.u90",   0x280000, 0x20000, CRC(607bcdc0) SHA1(f174a549ade75df2f86142150a1e4c3554907602) )
	ROM_LOAD ( "la1_strike_force_game_rom_u91.u91",   0x2a0000, 0x20000, CRC(da02547e) SHA1(d29c071bd9deab2414ac0733d9a18fcf8c68b4d9) )
ROM_END


ROM_START( mkla4 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "l4_mortal_kombat_game_rom_u-105.u105", 0x00000, 0x80000, CRC(29af348f) SHA1(9f8a57606647c5ea056d61aa4ab1232538539fd8) )
	ROM_LOAD16_BYTE( "l4_mortal_kombat_game_rom_u-89.u89",   0x00001, 0x80000, CRC(1ad76662) SHA1(bee4ab5371f58df799365e73ec0cc02e903f240c) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) )// known to be labeled LA1 or just L1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END


ROM_START( mkla3 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "l3_mortal_kombat_game_rom_u-105.u105", 0x00000, 0x80000, CRC(2ce843c5) SHA1(d48efcecd6528414249f3884edc32e0dafa9677f) )
	ROM_LOAD16_BYTE( "l3_mortal_kombat_game_rom_u-89.u89",   0x00001, 0x80000, CRC(49a46e10) SHA1(c63c00531b29c01ee864acc141b1713507d25c69) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // also known to be labeled as LA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END


ROM_START( mkla2 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "l2_mortal_kombat_game_rom_u-105.u105", 0x00000, 0x80000, CRC(8531d44e) SHA1(652c7946cc725e11815f852af8891511b87de186) )
	ROM_LOAD16_BYTE( "l2_mortal_kombat_game_rom_u-89.u89",   0x00001, 0x80000, CRC(b88dc26e) SHA1(bf34a03bdb70b67fd9c0b6d636b038a63827151e) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // also known to be labeled as LA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END


ROM_START( mkla1 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "l1_mortal_kombat_game_rom_u-105.u105", 0x00000, 0x80000, CRC(e1f7b4c9) SHA1(dc62e67e03b54460494bd94a50347327c19b72ec) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_game_rom_u-89.u89",   0x00001, 0x80000, CRC(9d38ac75) SHA1(86ff581cd3546f6b1be75e1d0744a8d767b22f5a) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // also known to be labeled as LA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END


ROM_START( mkprot9 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "proto9_mortal_kombat_game_rom_u-105.u105", 0x00000, 0x80000, CRC(20772bbd) SHA1(d5b400700b91c7a70bd2441c5254300cf1f743d7) ) // "PROTO9" sticker over label
	ROM_LOAD16_BYTE( "proto9_mortal_kombat_game_rom_u-89.u89",   0x00001, 0x80000, CRC(3238d45b) SHA1(8a4e827994d0d20feda3785a5f8f0f77b737052b) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // with prototype sets, these where labeled as PA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END


ROM_START( mkprot8 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "proto8_mortal_kombat_game_rom_u-105.u105", 0x00000, 0x80000, CRC(2f3c095d) SHA1(f6e9ac0fc0f997f4b323ba48590b042eae079a16) ) // "PROTO8" sticker over label
	ROM_LOAD16_BYTE( "proto8_mortal_kombat_game_rom_u-89.u89",   0x00001, 0x80000, CRC(edcf217f) SHA1(29e17bd20844a3e666e794c2fc068a011ccff2e8) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // with prototype sets, these where labeled as PA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END

ROM_START( mkprot4 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "proto4_mortal_kombat_game_rom_u-105.u105", 0x00000, 0x80000, CRC(d7f8d78b) SHA1(736f16d8c0407ee6dc8d3e40df08d1c926147a16) ) // "PROTO4" sticker over label
	ROM_LOAD16_BYTE( "proto4_mortal_kombat_game_rom_u-89.u89",   0x00001, 0x80000, CRC(a6b5d6d2) SHA1(917dbcff6d601d3fb015c8e26c6f0768290cd64a) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // with prototype sets, these where labeled as PA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END

ROM_START( mkyturbo )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	// A 'NIBBLE BOARD' daughtercard holding a GAL16V8A-2SP, 27C040 EPROM and a 9.8304MHz XTAL plugs into the U89 socket
	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "kombo-rom-u105.bin", 0x00000, 0x80000, CRC(80d5618c) SHA1(9bdfddbc70b61c94c1871abac1de153b8b728761) )
	ROM_LOAD16_BYTE(  "kombo-rom-u89.bin", 0x00001, 0x80000, CRC(450788e3) SHA1(34e4fa9c2ede66799301c3d1755df25edc432539) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // also known to be labeled as LA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END

ROM_START( mkyturboe )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	// A 'NIBBLE BOARD' daughtercard holding a GAL16V8A-2SP, 27C040 EPROM and a 9.8304MHz XTAL plugs into the U89 socket
	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "turbo30.u105", 0x00000, 0x80000, CRC(59747c59) SHA1(69e1450a6b2b41b8939ce84903cb35c1906b81e2) )
	ROM_LOAD16_BYTE(  "turbo30.u89", 0x00001, 0x80000, CRC(84d66a75) SHA1(11ee7ae7fc1c13cafa8312f101878393ae6fd8b7) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // also known to be labeled as LA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END

ROM_START( mkrep )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "u105",  0x00000, 0x80000, CRC(49a352ed) SHA1(c75f2ca7ff43e65c75cfe1fbd4375a00f54e2676) )
	ROM_LOAD16_BYTE(  "u89",  0x00001, 0x80000, CRC(5d5113b2) SHA1(3f870b8fd26865f76c88a9b5e8b72b9f891104c4) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // also known to be labeled as LA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END


ROM_START( mknifty )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "nifty.105", 0x00000, 0x80000, CRC(c66fd38d) SHA1(92e99f7c46422e47f503057398385168f63814cc) )
	ROM_LOAD16_BYTE(  "nifty.89", 0x00001, 0x80000, CRC(bbf8738d) SHA1(38acaf7c29e59b5c3ba32d5cb950d0fe8852ff51) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // also known to be labeled as LA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END

ROM_START( mknifty666 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD ( "l1_mortal_kombat_u3_sound_rom.u3", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) ) // same data as the "T-Unit" soundboard, but these are labeled L1 vs SL1

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "l1_mortal_kombat_u12_sound_rom.u12", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(                                      0x40000, 0x40000 )
	ROM_LOAD ( "l1_mortal_kombat_u13_sound_rom.u13", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(                                      0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "mortall_kombo_rom_u105-j4.u105.bin", 0x00000, 0x80000, CRC(243d8009) SHA1(e275f93d2d4b3a454303ce106641707a98bae084) )
	ROM_LOAD16_BYTE( "kombo-u89.u89", 0x00001, 0x80000, CRC(7b26a6b1) SHA1(378bd54fcc5c801ad8cc10ed94157a1e60572199))

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // also known to be labeled as LA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END

ROM_START( mkyawdim )
	ROM_REGION( 0x10000, "audiocpu", 0 )    // sound CPU
	ROM_LOAD (  "1.u67", 0x00000, 0x10000, CRC(b58d229e) SHA1(3ed14ef650dfa7f9d460611b19e9233a022cbea6) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM
	ROM_LOAD( "2.u59",  0x00000, 0x20000, CRC(a72ad81e) SHA1(7be4285b28755bd48acce670f34d6a7f043dda96) )
	ROM_CONTINUE(       0x40000, 0x20000 )
	ROM_CONTINUE(       0x80000, 0x20000 )
	ROM_CONTINUE(       0xc0000, 0x20000 )
	ROM_LOAD( "3.u60",  0x20000, 0x20000, CRC(6e68e0b0) SHA1(edb7aa6507452ffa5ce7097e3b1855a69542971c) )
	ROM_CONTINUE(       0x60000, 0x20000 )
	ROM_CONTINUE(       0xa0000, 0x20000 )
	ROM_CONTINUE(       0xe0000, 0x20000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "4.u25",  0x00000, 0x80000, CRC(b12b3bf2) SHA1(deb7755e8407d9de25124b3fdbc4c834a25d8252) )
	ROM_LOAD16_BYTE( "5.u26",  0x00001, 0x80000, CRC(7a37dc5c) SHA1(c4fc6933d8b990c5c56c65282b1f72b90b5d5435) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-111.u111", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) ) // also known to be labeled as LA1
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-112.u112", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-113.u113", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-114.u114", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-95.u95",   0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-96.u96",   0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-97.u97",   0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-98.u98",   0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-106.u106", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-107.u107", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-108.u108", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "l1_mortal_kombat_game_rom_u-109.u109", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END

ROM_START( mkyawdim2 )
	ROM_REGION( 0x10000, "audiocpu", 0 )    // sound CPU
	// Differs from other mkyawdim sets
	ROM_LOAD ( "yawdim.u167", 0x00000, 0x08000, CRC(16da7efb) SHA1(ac1db81a55aca36136b94977a91a1fc778b7b164) )  // 27c512
	ROM_CONTINUE(             0x00000, 0x08000 )

	ROM_REGION( 0x200000, "oki", 0 )    // ADPCM
	ROM_LOAD( "yawdim.u159", 0x000000, 0x20000, CRC(95b120af) SHA1(41b6fb384e5048926b87959a2c58d96b95698aba) )  // 27c020  Half size of mkyawdim set
	ROM_CONTINUE(            0x100000, 0x20000 )
	ROM_RELOAD(              0x040000, 0x20000 )
	ROM_CONTINUE(            0x140000, 0x20000 )
	ROM_RELOAD(              0x080000, 0x20000 )
	ROM_CONTINUE(            0x180000, 0x20000 )
	ROM_RELOAD(              0x0c0000, 0x20000 )
	ROM_CONTINUE(            0x1c0000, 0x20000 )
	ROM_LOAD( "mw-15.u160",  0x020000, 0x20000, CRC(6e68e0b0) SHA1(edb7aa6507452ffa5ce7097e3b1855a69542971c) )  // 4Mbit mask
	ROM_CONTINUE(            0x060000, 0x20000 )
	ROM_CONTINUE(            0x0a0000, 0x20000 )
	ROM_CONTINUE(            0x0e0000, 0x20000 )
	ROM_RELOAD(              0x120000, 0x20000 )
	ROM_CONTINUE(            0x160000, 0x20000 )
	ROM_CONTINUE(            0x1a0000, 0x20000 )
	ROM_CONTINUE(            0x1e0000, 0x20000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "4.u25",  0x00000, 0x80000, CRC(b12b3bf2) SHA1(deb7755e8407d9de25124b3fdbc4c834a25d8252) )  // 2x 4Mbit masks
	ROM_LOAD16_BYTE( "5.u26",  0x00001, 0x80000, CRC(7a37dc5c) SHA1(c4fc6933d8b990c5c56c65282b1f72b90b5d5435) )

	ROM_REGION( 0x800000, "gfx", 0 ) // 8mbit dumps
	ROM_LOAD ( "b-1.bin",  0x000000, 0x100000, CRC(f41e61c6) SHA1(7dad38839d5c9aa0cfa7b2f7199f14e0f2c4494b) )  // 6x 8Mbit masks
	ROM_LOAD ( "b-2.bin",  0x100000, 0x100000, CRC(8052740b) SHA1(f1b7fd536966d9d0ce690cdec635069c340d678e) )

	ROM_LOAD ( "a-1.bin",  0x200000, 0x100000, CRC(7da3cb93) SHA1(23b9053b3241b69988f7f2e6a9d1353dac4fc8ab) )
	ROM_LOAD ( "a-2.bin",  0x300000, 0x100000, CRC(1eedb0f8) SHA1(27c056c469c17bb176325b91cf92296c89681ac6) )

	ROM_LOAD ( "c-1.bin",  0x400000, 0x100000, CRC(de27c4c3) SHA1(a7760d239749c7463808adec72795f9785f553ec) )
	ROM_LOAD ( "c-2.bin",  0x500000, 0x100000, CRC(d99203f3) SHA1(46ea21cbedfd42838562594b9bdc5d80360b7e5e) )

	ROM_REGION( 0xf00, "plds", 0 )
	ROM_LOAD( "22v10.p1",  0x000, 0x2dd, CRC(15c24092) SHA1(7bbd1453c9a230dfa641239f15abb5aec93eb0dd) )  // unsecured
	ROM_LOAD( "22v10.p2",  0x000, 0x2dd, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "22v10.p3",  0x000, 0x2dd, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "16v8.p4",   0x000, 0x117, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "22v10.p5",  0x000, 0x2dd, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "16v8.p6",   0x000, 0x117, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "16v8.p7",   0x300, 0x117, CRC(fbbdc832) SHA1(8fe5448dad6025f98c70a9dd9aa7d07a75a6762c) )  // secured, bruteforced ok
	ROM_LOAD( "16v8.p8",   0x500, 0x117, CRC(8c573ab4) SHA1(9f16936c34cbaaa89b56356353f2a76fd4f28605) )  // secured, bruteforced ok
	ROM_LOAD( "22v10.p9",  0x700, 0x2e5, CRC(4e68c9ba) SHA1(fb984b3f417590dee3ee68feb44ed73707555f7e) ) // secured, bruteforced ok
	ROM_LOAD( "20v8.p10",  0x000, 0x157, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "20v8.p11",  0x000, 0x157, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "22v10.p12", 0x000, 0x2dd, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "22v10.p13", 0xa00, 0x2e5, CRC(3ccf1a6f) SHA1(9593fc3ca3b3ed77a79adabf930c01171c33b937) ) // secured, bruteforced ok
	ROM_LOAD( "22v10.p14", 0x000, 0x2dd, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "22v10.p15", 0x000, 0x2dd, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "20v8.p16",  0x000, 0x157, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "20v8.p17",  0x000, 0x157, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "22v10.p18", 0x000, 0x2dd, NO_DUMP )                                                       // secured, registered
	ROM_LOAD( "16v8.p19",  0xd00, 0x117, CRC(0346b5fc) SHA1(0cc1e1dcd6017de2e80eb1d40ac3a591e589b030) )  // unsecured
	// also u130: ti TPC1020AFN-084C PLCC84, unattempted
ROM_END


ROM_START( mkyawdim3 )
	ROM_REGION( 0x10000, "audiocpu", 0 )    // sound CPU
	ROM_LOAD (  "15.bin", 0x00000, 0x10000, CRC(b58d229e) SHA1(3ed14ef650dfa7f9d460611b19e9233a022cbea6) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM
	ROM_LOAD( "13.bin",  0x00000, 0x20000, CRC(921c613d) SHA1(be62b87f195b6347112ab13cc14514d4c88a8b86) ) // Half size of mkyawdim2 set and a quarter of mkyawdim
	ROM_RELOAD(          0x40000, 0x20000 )
	ROM_RELOAD(          0x80000, 0x20000 )
	ROM_RELOAD(          0xc0000, 0x20000 )
	ROM_LOAD( "14.bin",  0x20000, 0x20000, CRC(6e68e0b0) SHA1(edb7aa6507452ffa5ce7097e3b1855a69542971c) )
	ROM_CONTINUE(        0x60000, 0x20000 )
	ROM_CONTINUE(        0xa0000, 0x20000 )
	ROM_CONTINUE(        0xe0000, 0x20000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "p1.bin",  0x00000, 0x80000, CRC(2337a0f9) SHA1(d25743e5bb7b4a60f181783d17f217aa0a64536a) ) // differs from other Yawdim sets
	ROM_LOAD16_BYTE( "p2.bin",  0x00001, 0x80000, CRC(7a37dc5c) SHA1(c4fc6933d8b990c5c56c65282b1f72b90b5d5435) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "12.bin",  0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) )
	ROM_LOAD ( "8.bin",   0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD ( "10.bin",  0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD ( "3.bin",   0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD (  "6.bin",  0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD (  "1.bin",  0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD (  "11.bin", 0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD (  "2.bin",  0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD ( "4.bin",   0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD ( "7.bin",   0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD ( "9.bin",   0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD ( "5.bin",   0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END


/*************************************************************************

Mortal Kombat bootleg

PCB Layout
----------

Kombat rev. 2 1c
|-----------------------------------------------------|
|TDA2003 LM358 15  TMS34010                         19|
|              16          48MHz                    20|
| 14           8MHz                                   |
| 6116                                              21|
| Z80         M6295                                 22|
|J                                                    |
|A                           41464      4464 4464   23|
|M                  6264     41464      4464 4464   24|
|M                  17       41464      4464 4464     |
|A                  18       41464      4464 4464   25|
| DSW2(4)                               4464 4464   26|
|             6264                      4464 4464     |
|     DSW1(8)                                       27|
|             6264               TPC1020            28|
|                                                   29|
|                                                   30|
|-----------------------------------------------------|
Notes:
      Z80 @ 4MHz [8/2]
      TMS34010 @ 48MHz
      M6295 @ 1MHz[8/8]. Pin 7 HIGH
      41464/4464 - 64kx4-bit DRAM
      6264 - 8kx8-bit SRAM
      6116 - 2kx8-bit SRAM
      EPROMS: 14 is 27C512, 15 is 27C010, all others are 27C040
              14 - Z80 program
              15-16 - Oki samples
              17-18 - Main program
              19-30 - Graphics

*************************************************************************/

// same as mkyawdim3, but with its own main program roms
ROM_START( mkyawdim4 )
	ROM_REGION( 0x10000, "audiocpu", 0 )    // sound CPU
	ROM_LOAD( "14.bin", 0x00000, 0x10000, CRC(b58d229e) SHA1(3ed14ef650dfa7f9d460611b19e9233a022cbea6) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM
	ROM_LOAD( "15.bin", 0x00000, 0x20000, CRC(921c613d) SHA1(be62b87f195b6347112ab13cc14514d4c88a8b86) )
	ROM_RELOAD(         0x40000, 0x20000 )
	ROM_RELOAD(         0x80000, 0x20000 )
	ROM_RELOAD(         0xc0000, 0x20000 )
	ROM_LOAD( "16.bin", 0x20000, 0x20000, CRC(6e68e0b0) SHA1(edb7aa6507452ffa5ce7097e3b1855a69542971c) )
	ROM_CONTINUE(       0x60000, 0x20000 )
	ROM_CONTINUE(       0xa0000, 0x20000 )
	ROM_CONTINUE(       0xe0000, 0x20000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "17.bin", 0x00000, 0x80000, CRC(671b533d) SHA1(20859ceb0635126047216f85a6e35072e14766ad) )
	ROM_LOAD16_BYTE( "18.bin", 0x00001, 0x80000, CRC(4e857747) SHA1(b94c7d5e4356ac6890e6bfaf75c76d94408e5bc5) )
//  ROM_LOAD16_BYTE( "17.bin", 0x00000, 0x80000, CRC(b12b3bf2) SHA1(deb7755e8407d9de25124b3fdbc4c834a25d8252) ) // other PCB: mkyawdim3 with mkyawdim main program
//  ROM_LOAD16_BYTE( "18.bin", 0x00001, 0x80000, CRC(7a37dc5c) SHA1(c4fc6933d8b990c5c56c65282b1f72b90b5d5435) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "22.bin", 0x000000, 0x80000, CRC(d17096c4) SHA1(01ef390a372c9d94adf138f9543ebb88b89f4c38) )
	ROM_LOAD( "21.bin", 0x080000, 0x80000, CRC(993bc2e4) SHA1(7791edbec2b4b8971a3e790346dd7564ecf16d5c) )
	ROM_LOAD( "20.bin", 0x100000, 0x80000, CRC(6fb91ede) SHA1(a3735b49f93b08c44fbc97e2b5aad394628fbe90) )
	ROM_LOAD( "19.bin", 0x180000, 0x80000, CRC(ed1ff88a) SHA1(6b090b658ee6148af953bd0c9216f37162b6460f) )

	ROM_LOAD( "26.bin", 0x200000, 0x80000, CRC(a002a155) SHA1(3cf7909e92bcd428063596fc5b9953e0000d6eca) )
	ROM_LOAD( "25.bin", 0x280000, 0x80000, CRC(dcee8492) SHA1(a912b74d3b26ebd1b1613cc631080f83ececeaf8) )
	ROM_LOAD( "24.bin", 0x300000, 0x80000, CRC(de88caef) SHA1(a7927b504dc56ca5c9048373977fe5743b0a3f0b) )
	ROM_LOAD( "23.bin", 0x380000, 0x80000, CRC(37eb01b4) SHA1(06092460bd137e08d0f8df8560942ed877d40e09) )

	ROM_LOAD( "30.bin", 0x400000, 0x80000, CRC(45acaf21) SHA1(5edd36c55f4e5d3c74fb85171728ec0a58284b12) )
	ROM_LOAD( "29.bin", 0x480000, 0x80000, CRC(2a6c10a0) SHA1(cc90923c44f2961b945a0fd0f85ecc2ba04af2cb) )
	ROM_LOAD( "28.bin", 0x500000, 0x80000, CRC(23308979) SHA1(0b36788624a1cf0d3f4c895be5ba967b8dfcf85e) )
	ROM_LOAD( "27.bin", 0x580000, 0x80000, CRC(cafc47bb) SHA1(8610af6e52f7089ff4acd850c53ab8b4119e4445) )
ROM_END


ROM_START( mkla3bl ) // strange bootleg with peculiarly arranged GFX ROMs.
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )
	ROM_LOAD ( "m1-a003", 0x10000, 0x40000, CRC(c615844c) SHA1(5732f9053a5f73b0cc3b0166d7dc4430829d5bc7) )

	ROM_REGION( 0x100000, "adpcm:oki", 0 )
	ROM_LOAD ( "m1-a001", 0x00000, 0x40000, CRC(258bd7f9) SHA1(463890b23f17350fb9b8a85897b0777c45bc2d54) )
	ROM_RELOAD(               0x40000, 0x40000 )
	ROM_LOAD ( "m1-a002", 0x80000, 0x40000, CRC(7b7ec3b6) SHA1(6eec1b90d4a4855f34a7ebfbf93f3358d5627db4) )
	ROM_RELOAD(               0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 )
	ROM_LOAD16_BYTE( "ax422-m1-5", 0x00000, 0x80000, CRC(2ce843c5) SHA1(d48efcecd6528414249f3884edc32e0dafa9677f) )
	ROM_LOAD16_BYTE( "ax422-m1-4", 0x00001, 0x80000, CRC(49a46e10) SHA1(c63c00531b29c01ee864acc141b1713507d25c69) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "pw3412-m1-a", 0x000000, 0x200000, CRC(87776f14) SHA1(83533049545b175fa1fc8e021056466f6a37b2a5) ) // even == a-2.bin, odd == b-2.bin
	ROM_LOAD ( "pw3412-m1-b", 0x200000, 0x200000, CRC(30724e04) SHA1(a5f354b82fd5f73535ba77ed8be473f862528682) ) // even == a-1.bin, odd == b-1.bin
	ROM_LOAD ( "pw3412-m1-c", 0x400000, 0x200000, CRC(96612f94) SHA1(4cc80962bd04992ce95857650514c6f40f16fdad) ) // 1st half == c-1.bin, 2nd half == c-2.bin
ROM_END


ROM_START( term2 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD (  "sl1_terminator_2_u3_sound_rom.u3",   0x10000, 0x20000, CRC(73c3f5c4) SHA1(978dd974590e77294dbe9a647aebd3d24af6397f) )
	ROM_RELOAD ( 0x30000, 0x20000 )

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "sl1_terminator_2_u12_sound_rom.u12", 0x00000, 0x40000, CRC(e192a40d) SHA1(1f7a0e282c0c8eb66cbe514128bd104433e53b7a) )
	ROM_RELOAD( 0x40000, 0x40000 )
	ROM_LOAD ( "sl1_terminator_2_u13_sound_rom.u13", 0x80000, 0x40000, CRC(956fa80b) SHA1(02ab504627f4b25a394fa4192bb134138cbf6a4f) )
	ROM_RELOAD( 0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la4_terminator_2_game_rom_u105.u105", 0x00000, 0x80000, CRC(d4d8d884) SHA1(3209e131b128f12af30b3c6056fd63df497f93eb) )
	ROM_LOAD16_BYTE( "la4_terminator_2_game_rom_u89.u89",   0x00001, 0x80000, CRC(25359415) SHA1(ca8b7e1b5a363b78499f92c979a11ace6f1dceab) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_terminator_2_game_rom_u111.u111", 0x000000, 0x80000, CRC(916d0197) SHA1(3b53d3770955b10cc6002e3d3bf0f71429667af0) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u112.u112", 0x080000, 0x80000, CRC(39ae1c87) SHA1(a9d332dffc77c1e0dc50549825f5b403cf19c41d) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u113.u113", 0x100000, 0x80000, CRC(cb5084e5) SHA1(58cab00d8ebc72792f6c29899013ae6a0d2278b9) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u114.u114", 0x180000, 0x80000, CRC(53c516ec) SHA1(2a33639bc5bb4e7f7b3e341ddb59173260461d20) )

	ROM_LOAD ( "la1_terminator_2_game_rom_u95.u95",   0x200000, 0x80000, CRC(dd39cf73) SHA1(53bb54e66e4dfbe58385915004b0ad57583a7543) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u96.u96",   0x280000, 0x80000, CRC(31f4fd36) SHA1(766fca7d5e5043fc9e68bd3dc15e6a7830279d88) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u97.u97",   0x300000, 0x80000, CRC(7f72e775) SHA1(7e2369c6b4ed5d653700b041df58355a0960193a) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u98.u98",   0x380000, 0x80000, CRC(1a20ce29) SHA1(9089b7f77da5d67ad46ed249d72de8b8e0e5d807) )

	ROM_LOAD ( "la1_terminator_2_game_rom_u106.u106", 0x400000, 0x80000, CRC(f08a9536) SHA1(fbac314bd52f23c7a704acd3c707e75cdf204c07) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u107.u107", 0x480000, 0x80000, CRC(268d4035) SHA1(89a310830be9fbc91794fcccc57053d0933c42a3) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u108.u108", 0x500000, 0x80000, CRC(379fdaed) SHA1(408df6702c8ea8d3dce0b231955c6a60f3f5f22b) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u109.u109", 0x580000, 0x80000, CRC(306a9366) SHA1(b94c23c033221f7f7fddd2911b8cec9549929768) )
ROM_END


ROM_START( term2la3 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD (  "sl1_terminator_2_u3_sound_rom.u3",   0x10000, 0x20000, CRC(73c3f5c4) SHA1(978dd974590e77294dbe9a647aebd3d24af6397f) )
	ROM_RELOAD ( 0x30000, 0x20000 )

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "sl1_terminator_2_u12_sound_rom.u12", 0x00000, 0x40000, CRC(e192a40d) SHA1(1f7a0e282c0c8eb66cbe514128bd104433e53b7a) )
	ROM_RELOAD( 0x40000, 0x40000 )
	ROM_LOAD ( "sl1_terminator_2_u13_sound_rom.u13", 0x80000, 0x40000, CRC(956fa80b) SHA1(02ab504627f4b25a394fa4192bb134138cbf6a4f) )
	ROM_RELOAD( 0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la3_terminator_2_game_rom_u105.u105", 0x00000, 0x80000, CRC(34142b28) SHA1(985fd169b3d62c4197fe4c6f11055a6c17872899) )
	ROM_LOAD16_BYTE( "la3_terminator_2_game_rom_u89.u89",   0x00001, 0x80000, CRC(5ffea427) SHA1(c6f65bc57b33ae1a123f610c635e0d65663e54da) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_terminator_2_game_rom_u111.u111", 0x000000, 0x80000, CRC(916d0197) SHA1(3b53d3770955b10cc6002e3d3bf0f71429667af0) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u112.u112", 0x080000, 0x80000, CRC(39ae1c87) SHA1(a9d332dffc77c1e0dc50549825f5b403cf19c41d) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u113.u113", 0x100000, 0x80000, CRC(cb5084e5) SHA1(58cab00d8ebc72792f6c29899013ae6a0d2278b9) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u114.u114", 0x180000, 0x80000, CRC(53c516ec) SHA1(2a33639bc5bb4e7f7b3e341ddb59173260461d20) )

	ROM_LOAD ( "la1_terminator_2_game_rom_u95.u95",   0x200000, 0x80000, CRC(dd39cf73) SHA1(53bb54e66e4dfbe58385915004b0ad57583a7543) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u96.u96",   0x280000, 0x80000, CRC(31f4fd36) SHA1(766fca7d5e5043fc9e68bd3dc15e6a7830279d88) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u97.u97",   0x300000, 0x80000, CRC(7f72e775) SHA1(7e2369c6b4ed5d653700b041df58355a0960193a) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u98.u98",   0x380000, 0x80000, CRC(1a20ce29) SHA1(9089b7f77da5d67ad46ed249d72de8b8e0e5d807) )

	ROM_LOAD ( "la1_terminator_2_game_rom_u106.u106", 0x400000, 0x80000, CRC(f08a9536) SHA1(fbac314bd52f23c7a704acd3c707e75cdf204c07) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u107.u107", 0x480000, 0x80000, CRC(268d4035) SHA1(89a310830be9fbc91794fcccc57053d0933c42a3) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u108.u108", 0x500000, 0x80000, CRC(379fdaed) SHA1(408df6702c8ea8d3dce0b231955c6a60f3f5f22b) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u109.u109", 0x580000, 0x80000, CRC(306a9366) SHA1(b94c23c033221f7f7fddd2911b8cec9549929768) )
ROM_END


ROM_START( term2la2 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD (  "sl1_terminator_2_u3_sound_rom.u3",   0x10000, 0x20000, CRC(73c3f5c4) SHA1(978dd974590e77294dbe9a647aebd3d24af6397f) )
	ROM_RELOAD ( 0x30000, 0x20000 )

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "sl1_terminator_2_u12_sound_rom.u12", 0x00000, 0x40000, CRC(e192a40d) SHA1(1f7a0e282c0c8eb66cbe514128bd104433e53b7a) )
	ROM_RELOAD( 0x40000, 0x40000 )
	ROM_LOAD ( "sl1_terminator_2_u13_sound_rom.u13", 0x80000, 0x40000, CRC(956fa80b) SHA1(02ab504627f4b25a394fa4192bb134138cbf6a4f) )
	ROM_RELOAD( 0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la2_terminator_2_game_rom_u105.u105", 0x00000, 0x80000, CRC(7177de98) SHA1(0987be413d6cb5ded7059ad6ebbca49331b046b2) )
	ROM_LOAD16_BYTE( "la2_terminator_2_game_rom_u89.u89",   0x00001, 0x80000, CRC(14d7b9f5) SHA1(b8676d21d53fd3c8492d8911e749d74df1c66b1d) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_terminator_2_game_rom_u111.u111", 0x000000, 0x80000, CRC(916d0197) SHA1(3b53d3770955b10cc6002e3d3bf0f71429667af0) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u112.u112", 0x080000, 0x80000, CRC(39ae1c87) SHA1(a9d332dffc77c1e0dc50549825f5b403cf19c41d) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u113.u113", 0x100000, 0x80000, CRC(cb5084e5) SHA1(58cab00d8ebc72792f6c29899013ae6a0d2278b9) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u114.u114", 0x180000, 0x80000, CRC(53c516ec) SHA1(2a33639bc5bb4e7f7b3e341ddb59173260461d20) )

	ROM_LOAD ( "la1_terminator_2_game_rom_u95.u95",   0x200000, 0x80000, CRC(dd39cf73) SHA1(53bb54e66e4dfbe58385915004b0ad57583a7543) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u96.u96",   0x280000, 0x80000, CRC(31f4fd36) SHA1(766fca7d5e5043fc9e68bd3dc15e6a7830279d88) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u97.u97",   0x300000, 0x80000, CRC(7f72e775) SHA1(7e2369c6b4ed5d653700b041df58355a0960193a) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u98.u98",   0x380000, 0x80000, CRC(1a20ce29) SHA1(9089b7f77da5d67ad46ed249d72de8b8e0e5d807) )

	ROM_LOAD ( "la1_terminator_2_game_rom_u106.u106", 0x400000, 0x80000, CRC(f08a9536) SHA1(fbac314bd52f23c7a704acd3c707e75cdf204c07) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u107.u107", 0x480000, 0x80000, CRC(268d4035) SHA1(89a310830be9fbc91794fcccc57053d0933c42a3) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u108.u108", 0x500000, 0x80000, CRC(379fdaed) SHA1(408df6702c8ea8d3dce0b231955c6a60f3f5f22b) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u109.u109", 0x580000, 0x80000, CRC(306a9366) SHA1(b94c23c033221f7f7fddd2911b8cec9549929768) )
ROM_END


ROM_START( term2la1 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD (  "sl1_terminator_2_u3_sound_rom.u3",   0x10000, 0x20000, CRC(73c3f5c4) SHA1(978dd974590e77294dbe9a647aebd3d24af6397f) )
	ROM_RELOAD ( 0x30000, 0x20000 )

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "sl1_terminator_2_u12_sound_rom.u12", 0x00000, 0x40000, CRC(e192a40d) SHA1(1f7a0e282c0c8eb66cbe514128bd104433e53b7a) )
	ROM_RELOAD( 0x40000, 0x40000 )
	ROM_LOAD ( "sl1_terminator_2_u13_sound_rom.u13", 0x80000, 0x40000, CRC(956fa80b) SHA1(02ab504627f4b25a394fa4192bb134138cbf6a4f) )
	ROM_RELOAD( 0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la1_terminator_2_game_rom_u105.u105", 0x00000, 0x80000, CRC(ca52a8b0) SHA1(20b91bdd9fe8e7be6a3c3cb9684769733d66d401) )
	ROM_LOAD16_BYTE( "la1_terminator_2_game_rom_u89.u89",   0x00001, 0x80000, CRC(08535210) SHA1(a7986541bc504294bd6523ce691e19e496f8be7c) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_terminator_2_game_rom_u111.u111", 0x000000, 0x80000, CRC(916d0197) SHA1(3b53d3770955b10cc6002e3d3bf0f71429667af0) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u112.u112", 0x080000, 0x80000, CRC(39ae1c87) SHA1(a9d332dffc77c1e0dc50549825f5b403cf19c41d) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u113.u113", 0x100000, 0x80000, CRC(cb5084e5) SHA1(58cab00d8ebc72792f6c29899013ae6a0d2278b9) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u114.u114", 0x180000, 0x80000, CRC(53c516ec) SHA1(2a33639bc5bb4e7f7b3e341ddb59173260461d20) )

	ROM_LOAD ( "la1_terminator_2_game_rom_u95.u95",   0x200000, 0x80000, CRC(dd39cf73) SHA1(53bb54e66e4dfbe58385915004b0ad57583a7543) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u96.u96",   0x280000, 0x80000, CRC(31f4fd36) SHA1(766fca7d5e5043fc9e68bd3dc15e6a7830279d88) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u97.u97",   0x300000, 0x80000, CRC(7f72e775) SHA1(7e2369c6b4ed5d653700b041df58355a0960193a) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u98.u98",   0x380000, 0x80000, CRC(1a20ce29) SHA1(9089b7f77da5d67ad46ed249d72de8b8e0e5d807) )

	ROM_LOAD ( "la1_terminator_2_game_rom_u106.u106", 0x400000, 0x80000, CRC(f08a9536) SHA1(fbac314bd52f23c7a704acd3c707e75cdf204c07) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u107.u107", 0x480000, 0x80000, CRC(268d4035) SHA1(89a310830be9fbc91794fcccc57053d0933c42a3) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u108.u108", 0x500000, 0x80000, CRC(379fdaed) SHA1(408df6702c8ea8d3dce0b231955c6a60f3f5f22b) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u109.u109", 0x580000, 0x80000, CRC(306a9366) SHA1(b94c23c033221f7f7fddd2911b8cec9549929768) )
ROM_END


ROM_START( term2pa2 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD (  "sl1_terminator_2_u3_sound_rom.u3",   0x10000, 0x20000, CRC(73c3f5c4) SHA1(978dd974590e77294dbe9a647aebd3d24af6397f) )
	ROM_RELOAD ( 0x30000, 0x20000 )

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "sl1_terminator_2_u12_sound_rom.u12", 0x00000, 0x40000, CRC(e192a40d) SHA1(1f7a0e282c0c8eb66cbe514128bd104433e53b7a) )
	ROM_RELOAD( 0x40000, 0x40000 )
	ROM_LOAD ( "sl1_terminator_2_u13_sound_rom.u13", 0x80000, 0x40000, CRC(956fa80b) SHA1(02ab504627f4b25a394fa4192bb134138cbf6a4f) )
	ROM_RELOAD( 0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "pa2_terminator_2_game_rom_u105.u105", 0x00000, 0x80000, CRC(e7842129) SHA1(b2b00ac9995ef021e37ab6670c2c219ea09329dc) )
	ROM_LOAD16_BYTE( "pa2_terminator_2_game_rom_u89.u89",   0x00001, 0x80000, CRC(41d50e55) SHA1(37bf9fc5625b35a1b88cae291b71c4f761b687ab) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_terminator_2_game_rom_u111.u111", 0x000000, 0x80000, CRC(916d0197) SHA1(3b53d3770955b10cc6002e3d3bf0f71429667af0) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u112.u112", 0x080000, 0x80000, CRC(39ae1c87) SHA1(a9d332dffc77c1e0dc50549825f5b403cf19c41d) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u113.u113", 0x100000, 0x80000, CRC(cb5084e5) SHA1(58cab00d8ebc72792f6c29899013ae6a0d2278b9) )
	ROM_LOAD ( "pa1_terminator_2_game_rom_u114.u114", 0x180000, 0x80000, CRC(2c2bda49) SHA1(6a06ddf0eb5c6939f71f7e7a15561e9974c1505f) ) // labeled as PA1

	ROM_LOAD ( "la1_terminator_2_game_rom_u95.u95",   0x200000, 0x80000, CRC(dd39cf73) SHA1(53bb54e66e4dfbe58385915004b0ad57583a7543) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u96.u96",   0x280000, 0x80000, CRC(31f4fd36) SHA1(766fca7d5e5043fc9e68bd3dc15e6a7830279d88) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u97.u97",   0x300000, 0x80000, CRC(7f72e775) SHA1(7e2369c6b4ed5d653700b041df58355a0960193a) )
	ROM_LOAD ( "pa1_terminator_2_game_rom_u98.u98",   0x380000, 0x80000, CRC(3f80a9b2) SHA1(a1c8dcba55b1618e6b722c2371c7725d600372d0) ) // labeled as PA1

	ROM_LOAD ( "la1_terminator_2_game_rom_u106.u106", 0x400000, 0x80000, CRC(f08a9536) SHA1(fbac314bd52f23c7a704acd3c707e75cdf204c07) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u107.u107", 0x480000, 0x80000, CRC(268d4035) SHA1(89a310830be9fbc91794fcccc57053d0933c42a3) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u108.u108", 0x500000, 0x80000, CRC(379fdaed) SHA1(408df6702c8ea8d3dce0b231955c6a60f3f5f22b) )
	ROM_LOAD ( "pa2_terminator_2_game_rom_u109.u109", 0x580000, 0x80000, CRC(8d115894) SHA1(b25d1e3978028c0618dad40c87236602edfd021e) ) // labeled as PA2
ROM_END


ROM_START( term2lg1 ) // All reported German versions use standard English sound roms
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD (  "sl1_terminator_2_u3_sound_rom.u3",   0x10000, 0x20000, CRC(73c3f5c4) SHA1(978dd974590e77294dbe9a647aebd3d24af6397f) )
	ROM_RELOAD ( 0x30000, 0x20000 )

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "sl1_terminator_2_u12_sound_rom.u12", 0x00000, 0x40000, CRC(e192a40d) SHA1(1f7a0e282c0c8eb66cbe514128bd104433e53b7a) )
	ROM_RELOAD( 0x40000, 0x40000 )
	ROM_LOAD ( "sl1_terminator_2_u13_sound_rom.u13", 0x80000, 0x40000, CRC(956fa80b) SHA1(02ab504627f4b25a394fa4192bb134138cbf6a4f) )
	ROM_RELOAD( 0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "lg1_terminator_2_game_rom_u105.u105", 0x00000, 0x80000, CRC(6aad6389) SHA1(912f4e1911e537ed17775dbff6be0ab28ac820a9) )
	ROM_LOAD16_BYTE( "lg1_terminator_2_game_rom_u89.u89",   0x00001, 0x80000, CRC(5a052766) SHA1(a746c18476000fb38107482e22c767a13dd580d2) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_terminator_2_game_rom_u111.u111", 0x000000, 0x80000, CRC(916d0197) SHA1(3b53d3770955b10cc6002e3d3bf0f71429667af0) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u112.u112", 0x080000, 0x80000, CRC(39ae1c87) SHA1(a9d332dffc77c1e0dc50549825f5b403cf19c41d) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u113.u113", 0x100000, 0x80000, CRC(cb5084e5) SHA1(58cab00d8ebc72792f6c29899013ae6a0d2278b9) )
	ROM_LOAD ( "lg1_terminator_2_game_rom_u114.u114", 0x180000, 0x80000, CRC(1f0c6d8f) SHA1(b8908a19e87a4fdeb6f06944eb496cc8766d51e7) )

	ROM_LOAD ( "la1_terminator_2_game_rom_u95.u95",   0x200000, 0x80000, CRC(dd39cf73) SHA1(53bb54e66e4dfbe58385915004b0ad57583a7543) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u96.u96",   0x280000, 0x80000, CRC(31f4fd36) SHA1(766fca7d5e5043fc9e68bd3dc15e6a7830279d88) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u97.u97",   0x300000, 0x80000, CRC(7f72e775) SHA1(7e2369c6b4ed5d653700b041df58355a0960193a) )
	ROM_LOAD ( "lg1_terminator_2_game_rom_u98.u98",   0x380000, 0x80000, CRC(800c6205) SHA1(8f82a5e94505e33aa6c044040d7d002ea09045ef) )

	ROM_LOAD ( "la1_terminator_2_game_rom_u106.u106", 0x400000, 0x80000, CRC(f08a9536) SHA1(fbac314bd52f23c7a704acd3c707e75cdf204c07) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u107.u107", 0x480000, 0x80000, CRC(268d4035) SHA1(89a310830be9fbc91794fcccc57053d0933c42a3) )
	ROM_LOAD ( "la1_terminator_2_game_rom_u108.u108", 0x500000, 0x80000, CRC(379fdaed) SHA1(408df6702c8ea8d3dce0b231955c6a60f3f5f22b) )
	ROM_LOAD ( "lg1_terminator_2_game_rom_u109.u109", 0x580000, 0x80000, CRC(70dc2ff3) SHA1(de756a3c1e3f5f916d0e5c463ec758814fdcd7f5) )
ROM_END


ROM_START( totcarn )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD (  "sl1_total_carnage_sound_rom_u3.u3", 0x10000, 0x20000, CRC(5bdb4665) SHA1(c6b90b914785b8703790957cc4bb4983a332fba6) )
	ROM_RELOAD (                                     0x30000, 0x20000 )

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "sl1_total_carnage_sound_rom_u12.u12", 0x00000, 0x40000, CRC(d0000ac7) SHA1(2d476c7727462623feb2f1a23fb797eaeed5ce30) )
	ROM_RELOAD(                                       0x40000, 0x40000 )
	ROM_LOAD ( "sl1_total_carnage_sound_rom_u13.u13", 0x80000, 0x40000, CRC(e48e6f0c) SHA1(bf7d548b6b1901966f99c815129ea160ef36f024) )
	ROM_RELOAD(                                       0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "la1_total_carnage_game_rom_u105.u105", 0x80000, 0x40000, CRC(7c651047) SHA1(530c8b4e453778a81479d02913ffe7097903447f) )
	ROM_LOAD16_BYTE( "la1_total_carnage_game_rom_u89.u89",   0x80001, 0x40000, CRC(6761daf3) SHA1(8be881ecc5ea1121bb6cee1a34901a4d5e50dbb6) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_total_carnage_game_rom_u111.u111", 0x000000, 0x40000, CRC(13f3f231) SHA1(6df0dca72e170818c260d9931477103a38864a1e) ) // Also known to be labeled as PA1
	ROM_LOAD ( "la1_total_carnage_game_rom_u112.u112", 0x040000, 0x40000, CRC(72e45007) SHA1(b6f5dfb844b6ff46a3594d20e85f1f20bdbfb793) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u113.u113", 0x080000, 0x40000, CRC(2c8ec753) SHA1(9393179ea19cbec7ac7e4f8e912bb4f86d93e8bd) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u114.u114", 0x0c0000, 0x40000, CRC(6210c36c) SHA1(607acdf024c1d36238ed19841c3ef2c96f49038f) )

	ROM_LOAD ( "la1_total_carnage_game_rom_u95.u95",   0x200000, 0x40000, CRC(579caeba) SHA1(de7d9921a210839e1db4bf54fb96833bcb073862) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u96.u96",   0x240000, 0x40000, CRC(f43f1ffe) SHA1(60401092be1fed52a028dc81b7a28ade923c35ea) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u97.u97",   0x280000, 0x40000, CRC(1675e50d) SHA1(1479712b03fa2b67fcd2d4694f26ce1bd1959b97) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u98.u98",   0x2c0000, 0x40000, CRC(ab06c885) SHA1(09163060269fed2ebd697b71602166e906c95317) )

	ROM_LOAD ( "la1_total_carnage_game_rom_u106.u106", 0x400000, 0x40000, CRC(146e3863) SHA1(1933e62a060eb667889b1edd5002c30a37ae00a7) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u107.u107", 0x440000, 0x40000, CRC(95323320) SHA1(5296206f3d84c21374968ffcacfe59eb3215ca46) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u108.u108", 0x480000, 0x40000, CRC(ed152acc) SHA1(372dbc4fdb581ac00a7eb5669cc1ac7afd6033f8) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u109.u109", 0x4c0000, 0x40000, CRC(80715252) SHA1(4586a259780963837ce362b526f161122d2e3cb4) )
ROM_END


ROM_START( totcarnp2 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD (  "sl1_total_carnage_sound_rom_u3.u3", 0x10000, 0x20000, CRC(5bdb4665) SHA1(c6b90b914785b8703790957cc4bb4983a332fba6) )
	ROM_RELOAD (                                     0x30000, 0x20000 )

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "sl1_total_carnage_sound_rom_u12.u12", 0x00000, 0x40000, CRC(d0000ac7) SHA1(2d476c7727462623feb2f1a23fb797eaeed5ce30) )
	ROM_RELOAD(                                       0x40000, 0x40000 )
	ROM_LOAD ( "sl1_total_carnage_sound_rom_u13.u13", 0x80000, 0x40000, CRC(e48e6f0c) SHA1(bf7d548b6b1901966f99c815129ea160ef36f024) )
	ROM_RELOAD(                                       0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "proto2_total_carnage_game_rom_u105.u105", 0x80000, 0x40000, CRC(e273d43c) SHA1(9f2040328917f02153edced28f9a4e1fb7d0cee7) )
	ROM_LOAD16_BYTE( "proto2_total_carnage_game_rom_u89.u89",   0x80001, 0x40000, CRC(e759078b) SHA1(a7712b51215029422e21ba803dac3afd44c941ac) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_total_carnage_game_rom_u111.u111", 0x000000, 0x40000, CRC(13f3f231) SHA1(6df0dca72e170818c260d9931477103a38864a1e) ) // Also known to be labeled as PA1
	ROM_LOAD ( "la1_total_carnage_game_rom_u112.u112", 0x040000, 0x40000, CRC(72e45007) SHA1(b6f5dfb844b6ff46a3594d20e85f1f20bdbfb793) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u113.u113", 0x080000, 0x40000, CRC(2c8ec753) SHA1(9393179ea19cbec7ac7e4f8e912bb4f86d93e8bd) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u114.u114", 0x0c0000, 0x40000, CRC(6210c36c) SHA1(607acdf024c1d36238ed19841c3ef2c96f49038f) )

	ROM_LOAD ( "la1_total_carnage_game_rom_u95.u95",   0x200000, 0x40000, CRC(579caeba) SHA1(de7d9921a210839e1db4bf54fb96833bcb073862) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u96.u96",   0x240000, 0x40000, CRC(f43f1ffe) SHA1(60401092be1fed52a028dc81b7a28ade923c35ea) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u97.u97",   0x280000, 0x40000, CRC(1675e50d) SHA1(1479712b03fa2b67fcd2d4694f26ce1bd1959b97) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u98.u98",   0x2c0000, 0x40000, CRC(ab06c885) SHA1(09163060269fed2ebd697b71602166e906c95317) )

	ROM_LOAD ( "la1_total_carnage_game_rom_u106.u106", 0x400000, 0x40000, CRC(146e3863) SHA1(1933e62a060eb667889b1edd5002c30a37ae00a7) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u107.u107", 0x440000, 0x40000, CRC(95323320) SHA1(5296206f3d84c21374968ffcacfe59eb3215ca46) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u108.u108", 0x480000, 0x40000, CRC(ed152acc) SHA1(372dbc4fdb581ac00a7eb5669cc1ac7afd6033f8) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u109.u109", 0x4c0000, 0x40000, CRC(80715252) SHA1(4586a259780963837ce362b526f161122d2e3cb4) )
ROM_END


ROM_START( totcarnp1 )
	ROM_REGION( 0x50000, "adpcm:cpu", 0 )   // sound CPU
	ROM_LOAD (  "sl1_total_carnage_sound_rom_u3.u3", 0x10000, 0x20000, CRC(5bdb4665) SHA1(c6b90b914785b8703790957cc4bb4983a332fba6) )
	ROM_RELOAD (                                     0x30000, 0x20000 )

	ROM_REGION( 0x100000, "adpcm:oki", 0 )  // ADPCM
	ROM_LOAD ( "sl1_total_carnage_sound_rom_u12.u12", 0x00000, 0x40000, CRC(d0000ac7) SHA1(2d476c7727462623feb2f1a23fb797eaeed5ce30) )
	ROM_RELOAD(                                       0x40000, 0x40000 )
	ROM_LOAD ( "sl1_total_carnage_sound_rom_u13.u13", 0x80000, 0x40000, CRC(e48e6f0c) SHA1(bf7d548b6b1901966f99c815129ea160ef36f024) )
	ROM_RELOAD(                                       0xc0000, 0x40000 )

	ROM_REGION16_LE( 0x100000, "maindata", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "proto1_total_carnage_game_rom_u105.u105", 0x80000, 0x40000, CRC(7a782cae) SHA1(806894e23876325fffcad4d707c850fbd91d973a) )
	ROM_LOAD16_BYTE( "proto1_total_carnage_game_rom_u89.u89",   0x80001, 0x40000, CRC(1c899a8d) SHA1(953d4def814f036969b9ecf3be16e145c2d2bf9f) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD ( "la1_total_carnage_game_rom_u111.u111", 0x000000, 0x40000, CRC(13f3f231) SHA1(6df0dca72e170818c260d9931477103a38864a1e) ) // Also known to be labeled as PA1
	ROM_LOAD ( "la1_total_carnage_game_rom_u112.u112", 0x040000, 0x40000, CRC(72e45007) SHA1(b6f5dfb844b6ff46a3594d20e85f1f20bdbfb793) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u113.u113", 0x080000, 0x40000, CRC(2c8ec753) SHA1(9393179ea19cbec7ac7e4f8e912bb4f86d93e8bd) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u114.u114", 0x0c0000, 0x40000, CRC(6210c36c) SHA1(607acdf024c1d36238ed19841c3ef2c96f49038f) )

	ROM_LOAD ( "la1_total_carnage_game_rom_u95.u95",   0x200000, 0x40000, CRC(579caeba) SHA1(de7d9921a210839e1db4bf54fb96833bcb073862) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u96.u96",   0x240000, 0x40000, CRC(f43f1ffe) SHA1(60401092be1fed52a028dc81b7a28ade923c35ea) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u97.u97",   0x280000, 0x40000, CRC(1675e50d) SHA1(1479712b03fa2b67fcd2d4694f26ce1bd1959b97) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u98.u98",   0x2c0000, 0x40000, CRC(ab06c885) SHA1(09163060269fed2ebd697b71602166e906c95317) )

	ROM_LOAD ( "la1_total_carnage_game_rom_u106.u106", 0x400000, 0x40000, CRC(146e3863) SHA1(1933e62a060eb667889b1edd5002c30a37ae00a7) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u107.u107", 0x440000, 0x40000, CRC(95323320) SHA1(5296206f3d84c21374968ffcacfe59eb3215ca46) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u108.u108", 0x480000, 0x40000, CRC(ed152acc) SHA1(372dbc4fdb581ac00a7eb5669cc1ac7afd6033f8) )
	ROM_LOAD ( "la1_total_carnage_game_rom_u109.u109", 0x4c0000, 0x40000, CRC(80715252) SHA1(4586a259780963837ce362b526f161122d2e3cb4) )
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1988, narc,       0,        zunit,                   narc,     midzunit_state,       init_narc,     ROT0,               "Williams",         "Narc (rev 7.00)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, narc6,      narc,     zunit,                   narc,     midzunit_state,       init_narc,     ROT0,               "Williams",         "Narc (rev 6.00)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, narc4,      narc,     zunit,                   narc,     midzunit_state,       init_narc,     ROT0,               "Williams",         "Narc (rev 4.00)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, narc3,      narc,     zunit,                   narc,     midzunit_state,       init_narc,     ROT0,               "Williams",         "Narc (rev 3.20)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, narc2,      narc,     zunit,                   narc,     midzunit_state,       init_narc,     ROT0,               "Williams",         "Narc (rev 2.00)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, narc1,      narc,     zunit,                   narc,     midzunit_state,       init_narc,     ROT0,               "Williams",         "Narc (rev 1.80)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, trog,       0,        yunit_cvsd_4bit_slow,    trog,     midyunit_cvsd_state,  init_trog,     ROT0,               "Midway",           "Trog (rev LA5 3/29/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, trog4,      trog,     yunit_cvsd_4bit_slow,    trog,     midyunit_cvsd_state,  init_trog,     ROT0,               "Midway",           "Trog (rev LA4 3/11/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, trog3,      trog,     yunit_cvsd_4bit_slow,    trog,     midyunit_cvsd_state,  init_trog,     ROT0,               "Midway",           "Trog (rev LA3 2/14/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, trog3a,     trog,     yunit_cvsd_4bit_slow,    trog,     midyunit_cvsd_state,  init_trog,     ROT0,               "Midway",           "Trog (rev LA3 2/10/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, trogpa6,    trog,     yunit_cvsd_4bit_slow,    trog,     midyunit_cvsd_state,  init_trog,     ROT0,               "Midway",           "Trog (prototype, rev PA6-PAC 9/09/90)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, trogpa5,    trog,     yunit_cvsd_4bit_slow,    trog,     midyunit_cvsd_state,  init_trog,     ROT0,               "Midway",           "Trog (prototype, rev PA5-PAC 8/28/90)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, trogpa4,    trog,     yunit_cvsd_4bit_slow,    trogpa4,  midyunit_cvsd_state,  init_trog,     ROT0,               "Midway",           "Trog (prototype, rev 4.00 7/27/90)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, mazebl,     trog,     yunit_cvsd_4bit_slow,    trog,     midyunit_cvsd_state,  init_trog,     ROT0,               "bootleg",          "Maze (Trog rev LA4 3/11/91 bootleg)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, smashtv,    0,        yunit_cvsd_6bit_slow,    smashtv,  midyunit_cvsd_state,  init_smashtv,  ROT0,               "Williams",         "Smash T.V. (rev 8.00)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, smashtv6,   smashtv,  yunit_cvsd_6bit_slow,    smashtv,  midyunit_cvsd_state,  init_smashtv,  ROT0,               "Williams",         "Smash T.V. (rev 6.00)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, smashtv5,   smashtv,  yunit_cvsd_6bit_slow,    smashtv,  midyunit_cvsd_state,  init_smashtv,  ROT0,               "Williams",         "Smash T.V. (rev 5.00)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, smashtv4,   smashtv,  yunit_cvsd_6bit_slow,    smashtv,  midyunit_cvsd_state,  init_smashtv,  ROT0,               "Williams",         "Smash T.V. (rev 4.00)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, smashtv3,   smashtv,  yunit_cvsd_6bit_slow,    smashtv,  midyunit_cvsd_state,  init_smashtv,  ROT0,               "Williams",         "Smash T.V. (rev 3.01)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, hiimpact,   0,        yunit_cvsd_6bit_slow,    hiimpact, midyunit_cvsd_state,  init_hiimpact, ROT0,               "Williams",         "High Impact Football (rev LA5 02/15/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, hiimpact4,  hiimpact, yunit_cvsd_6bit_slow,    hiimpact, midyunit_cvsd_state,  init_hiimpact, ROT0,               "Williams",         "High Impact Football (rev LA4 02/04/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, hiimpact3,  hiimpact, yunit_cvsd_6bit_slow,    hiimpact, midyunit_cvsd_state,  init_hiimpact, ROT0,               "Williams",         "High Impact Football (rev LA3 12/27/90)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, hiimpact2,  hiimpact, yunit_cvsd_6bit_slow,    hiimpact, midyunit_cvsd_state,  init_hiimpact, ROT0,               "Williams",         "High Impact Football (rev LA2 12/26/90)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, hiimpact1,  hiimpact, yunit_cvsd_6bit_slow,    hiimpact, midyunit_cvsd_state,  init_hiimpact, ROT0,               "Williams",         "High Impact Football (rev LA1 12/16/90)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, hiimpactp,  hiimpact, yunit_cvsd_6bit_slow,    hiimpact, midyunit_cvsd_state,  init_hiimpact, ROT0,               "Williams",         "High Impact Football (prototype, revision0 proto 8.6 12/09/90)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, shimpact,   0,        yunit_cvsd_6bit_slow,    shimpact, midyunit_cvsd_state,  init_shimpact, ROT0,               "Midway",           "Super High Impact (rev LA1 09/30/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, shimpactp6, shimpact, yunit_cvsd_6bit_slow,    shimpact, midyunit_cvsd_state,  init_shimpact, ROT0,               "Midway",           "Super High Impact (prototype, proto 6.0 09/23/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, shimpactp5, shimpact, yunit_cvsd_6bit_slow,    shimpact, midyunit_cvsd_state,  init_shimpact, ROT0,               "Midway",           "Super High Impact (prototype, proto 5.0 09/15/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, shimpactp4, shimpact, yunit_cvsd_6bit_slow,    shimpact, midyunit_cvsd_state,  init_shimpact, ROT0,               "Midway",           "Super High Impact (prototype, proto 4.0 09/10/91)", MACHINE_SUPPORTS_SAVE ) // See notes about factory restore above

GAME( 1991, strkforc,   0,        yunit_cvsd_4bit_fast,    strkforc, midyunit_cvsd_state,  init_strkforc, ROT0,               "Midway",           "Strike Force (rev 1 02/25/91)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, term2,      0,        term2,                   term2,    term2_state,          init_term2,    ORIENTATION_FLIP_X, "Midway",           "Terminator 2 - Judgment Day (rev LA4 08/03/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, term2la3,   term2,    term2,                   term2,    term2_state,          init_term2la3, ORIENTATION_FLIP_X, "Midway",           "Terminator 2 - Judgment Day (rev LA3 03/27/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, term2la2,   term2,    term2,                   term2,    term2_state,          init_term2la2, ORIENTATION_FLIP_X, "Midway",           "Terminator 2 - Judgment Day (rev LA2 12/09/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, term2la1,   term2,    term2,                   term2,    term2_state,          init_term2la1, ORIENTATION_FLIP_X, "Midway",           "Terminator 2 - Judgment Day (rev LA1 11/01/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, term2pa2,   term2,    term2,                   term2,    term2_state,          init_term2la1, ORIENTATION_FLIP_X, "Midway",           "Terminator 2 - Judgment Day (prototype, rev PA2 10/18/91)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, term2lg1,   term2,    term2,                   term2,    term2_state,          init_term2la1, ORIENTATION_FLIP_X, "Midway",           "Terminator 2 - Judgment Day (German, rev LG1 11/04/91)", MACHINE_SUPPORTS_SAVE )

GAME( 1992, mkla4,      mk,       yunit_adpcm_6bit_fast,   mkla4,    midyunit_adpcm_state, init_mkyunit,   ROT0,              "Midway",           "Mortal Kombat (rev 4.0 09/28/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mkla3,      mk,       yunit_adpcm_6bit_fast,   mkla4,    midyunit_adpcm_state, init_mkyunit,   ROT0,              "Midway",           "Mortal Kombat (rev 3.0 08/31/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mkla2,      mk,       yunit_adpcm_6bit_fast,   mkla2,    midyunit_adpcm_state, init_mkyunit,   ROT0,              "Midway",           "Mortal Kombat (rev 2.0 08/18/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mkla1,      mk,       yunit_adpcm_6bit_fast,   mkla2,    midyunit_adpcm_state, init_mkyunit,   ROT0,              "Midway",           "Mortal Kombat (rev 1.0 08/09/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mkprot9,    mk,       yunit_adpcm_6bit_faster, mkla2,    midyunit_adpcm_state, init_mkyunit,   ROT0,              "Midway",           "Mortal Kombat (prototype, rev 9.0 07/28/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mkprot8,    mk,       yunit_adpcm_6bit_faster, mkla2,    midyunit_adpcm_state, init_mkyunit,   ROT0,              "Midway",           "Mortal Kombat (prototype, rev 8.0 07/21/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mkprot4,    mk,       yunit_adpcm_6bit_faster, mkla2,    midyunit_adpcm_state, init_mkyunit,   ROT0,              "Midway",           "Mortal Kombat (prototype, rev 4.0 07/14/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mkyturbo,   mk,       yunit_adpcm_6bit_fast,   mkla4,    midyunit_adpcm_state, init_mkyturbo,  ROT0,              "hack",             "Mortal Kombat (Turbo 3.1 09/09/93, hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mkyturboe,  mk,       yunit_adpcm_6bit_fast,   mkla4,    midyunit_adpcm_state, init_mkyturbo,  ROT0,              "hack",             "Mortal Kombat (Turbo 3.0 08/31/92, hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mknifty,    mk,       yunit_adpcm_6bit_fast,   mkla4,    midyunit_adpcm_state, init_mkyturbo,  ROT0,              "hack",             "Mortal Kombat (Nifty Kombo, hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mknifty666, mk,       yunit_adpcm_6bit_fast,   mkla4,    midyunit_adpcm_state, init_mkyturbo,  ROT0,              "hack",             "Mortal Kombat (Nifty Kombo 666, hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mkrep,      mk,       yunit_adpcm_6bit_fast,   mkla4,    midyunit_adpcm_state, init_mkyturbo,  ROT0,              "hack",             "Mortal Kombat (Reptile Man hack)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1992, mkyawdim,   mk,       mkyawdim,                mkyawdim, mkyawdim_state,       init_mkyawdim,  ROT0,              "bootleg (Yawdim)", "Mortal Kombat (Yawdim bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mkyawdim2,  mk,       mkyawdim2,               mkyawdim, mkyawdim_state,       init_mkyawdim,  ROT0,              "bootleg (Yawdim)", "Mortal Kombat (Yawdim bootleg, set 2)", MACHINE_SUPPORTS_SAVE ) // some sound effects are missing on real pcb
GAME( 1992, mkyawdim3,  mk,       mkyawdim,                mkyawdim, mkyawdim_state,       init_mkyawdim,  ROT0,              "bootleg (Yawdim)", "Mortal Kombat (Yawdim bootleg, set 3)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND) // are some sound effects missing/wrong?
GAME( 1992, mkyawdim4,  mk,       mkyawdim,                mkyawdim, mkyawdim_state,       init_mkyawdim,  ROT0,              "bootleg (Yawdim)", "Mortal Kombat (Yawdim bootleg, set 4)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND) // are some sound effects missing/wrong?
GAME( 1992, mkla3bl,    mk,       yunit_adpcm_6bit_fast,   mkla4,    midyunit_adpcm_state, init_mkla3bl,   ROT0,              "bootleg (Victor)", "Mortal Kombat (Victor bootleg of rev 3.0 08/31/92)", MACHINE_SUPPORTS_SAVE )

GAME( 1992, totcarn,    0,        yunit_adpcm_6bit_fast,   totcarn,  midyunit_adpcm_state, init_totcarn,   ROT0,              "Midway",           "Total Carnage (rev LA1 03/10/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, totcarnp2,  totcarn,  yunit_adpcm_6bit_fast,   totcarn,  midyunit_adpcm_state, init_totcarn,   ROT0,              "Midway",           "Total Carnage (prototype, proto v 2.0 02/10/92)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, totcarnp1,  totcarn,  yunit_adpcm_6bit_fast,   totcarn,  midyunit_adpcm_state, init_totcarn,   ROT0,              "Midway",           "Total Carnage (prototype, proto v 1.0 01/25/92)", MACHINE_SUPPORTS_SAVE )
