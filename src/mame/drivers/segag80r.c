// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega G-80 raster hardware

    Games supported:
        * Astro Blaster
        * Monster Bash
        * 005
        * Space Odyssey
        * Pig Newton
        * Sindbad Mystery

    Known bugs:
        * Pig Newton doesn't read cocktail controls (might be game bug)
        * Astro Blaster v1 dies when you first start if you only
          have one credit at the time (bad dump?, protection?)

****************************************************************************

    The Sega G-80 system is an interchangeable raster/vector system with
    a common main board and a stack of video/sound boards for each game.
    The CPU boards were protected with the famous G-80 security chips,
    which scramble the address lines on memory writes. Below are a list of
    games and their board stacks:

    Astro Blaster:
        * G-80 CPU board (315-0062 security)
        * G-80 2716 EPROM board
        * Video I board
        * G-80 Speech Synthesis Board
        * Astro Blaster Sound board (discrete)

    005:
        * G-80 CPU board (315-0070 security)
        * G-80 2716 EPROM board
        * Video I board
        * 005 Sound board (melody generator + DAC, discrete)

    Space Odyssey:
        * G-80 CPU board (315-0063 security)
        * G-80 2716 EPROM board
        * Video I board
        * Space Odyssey Background Generator Board
        * Space Odyssey Sound board (discrete)

    Monster Bash:
        * G-80 CPU board (315-0082 security)
        * G-80 2716 EPROM board
        * Video I board
        * Monster Bash Background Board
        * Monster Bash Sound Board (N7751+DAC, TMS3617, discrete)

    It appears that later on in the evolution of the hardware, Sega
    produced a 2-board version of the hardware which put all the CPU,
    EPROM, video, and background hardware on one or two boards:

    Monster Bash (2 board):
        * G-80 consolidated boardset (315-5006 encrypted Z80)
        * Monster Bash Sound Board (N7751+DAC, TMS3617, discrete)

    Pig Newton:
        * G-80 consolidated boardset (315-0062 security)
        * Sega Universal Sound Board (I8035, discrete)

    Finally, it looks like Sega also evolved the hardware in the
    direction of their widely-used System 1 hardware. The final G-80
    game had a new sound board that was SN76496-based and seemingly
    identical to the final System 1 hardware. It also used an encrypted
    Z80 CPU very much like the System 1 games (the 2-board Monster Bash
    also used one of these):

    Sindbad Mystery:
        * Unknown boardset (background doesn't match consolidated)
        * System 1 Sound (Z80, 2xSN76496)

****************************************************************************

    See also sega.c for the Sega G-80 Vector games.

    Many thanks go to Dave Fish for the fine detective work he did into the
    G-80 security chips (315-0064, 315-0070, 315-0076, 315-0082) which provided
    me with enough information to emulate those chips at runtime along with
    the 315-0062 Astro Blaster chip and the 315-0063 Space Odyssey chip.

    Special note (24-MAR-1999) - Sindbad Mystery does *not* use the standard
    G-80 security chip; rather, it uses the Sega System 1 encryption.

    Thanks also go to Paul Tonizzo, Clay Cowgill, John Bowes, and Kevin Klopp
    for all the helpful information, samples, and schematics!

    TODO:
    - verify Pig Newton and Sindbad Mystery DIPs
    - attempt 005 sound
    - fix Space Odyssey background
    - figure out why Astro Blaster version 1 ends the game right away

    - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************

    26/3/2000:  ** Darren Hatton (UKVAC) / Adrian Purser (UKVAC) **
                Added a 3rd Astro Blaster ROM set (ASTROB2).
                Updated Dip Switches to be correct for the Astro Blaster sets.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/sn76496.h"
#include "sound/samples.h"
#include "machine/i8255.h"
#include "machine/segacrpt.h"
#include "machine/segag80.h"
#include "includes/segag80r.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define CPU_CLOCK           8000000     /* not used when video board is connected */
#define VIDEO_CLOCK         15468000
#define SINDBADM_SOUND_CLOCK 8000000

#define PIXEL_CLOCK         (VIDEO_CLOCK/3)

#define HTOTAL              (328)
#define HBEND               (0)
#define HBSTART             (256)

#define VTOTAL              (262)
#define VBEND               (0)
#define VBSTART             (224)



/*************************************
 *
 *  Machine setup and config
 *
 *************************************/

INPUT_CHANGED_MEMBER(segag80r_state::service_switch)
{
	/* pressing the service switch sends an NMI */
	if (newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


void segag80r_state::machine_start()
{
	/* register for save states */
}



/*************************************
 *
 *  RAM writes/decryption
 *
 *************************************/

offs_t segag80r_state::decrypt_offset(address_space &space, offs_t offset)
{
	/* ignore anything but accesses via opcode $32 (LD $(XXYY),A) */
	offs_t pc = space.device().safe_pcbase();
	if ((UINT16)pc == 0xffff || space.read_byte(pc) != 0x32)
		return offset;

	/* fetch the low byte of the address and munge it */
	return (offset & 0xff00) | (*m_decrypt)(pc, space.read_byte(pc + 1));
}

WRITE8_MEMBER(segag80r_state::mainram_w)
{
	m_mainram[decrypt_offset(space, offset)] = data;
}

WRITE8_MEMBER(segag80r_state::vidram_w){ segag80r_videoram_w(space, decrypt_offset(space, offset), data); }
WRITE8_MEMBER(segag80r_state::monsterb_vidram_w){ monsterb_videoram_w(space, decrypt_offset(space, offset), data); }
WRITE8_MEMBER(segag80r_state::pignewt_vidram_w){ pignewt_videoram_w(space, decrypt_offset(space, offset), data); }
WRITE8_MEMBER(segag80r_state::sindbadm_vidram_w){ sindbadm_videoram_w(space, decrypt_offset(space, offset), data); }
WRITE8_MEMBER(segag80r_state::usb_ram_w){ m_usbsnd->ram_w(space, decrypt_offset(m_maincpu->space(AS_PROGRAM), offset), data); }



/*************************************
 *
 *  Input ports
 *
 *************************************/

inline UINT8 segag80r_state::demangle(UINT8 d7d6, UINT8 d5d4, UINT8 d3d2, UINT8 d1d0)
{
	return ((d7d6 << 7) & 0x80) | ((d7d6 << 2) & 0x40) |
			((d5d4 << 5) & 0x20) | ((d5d4 << 0) & 0x10) |
			((d3d2 << 3) & 0x08) | ((d3d2 >> 2) & 0x04) |
			((d1d0 << 1) & 0x02) | ((d1d0 >> 4) & 0x01);
}


READ8_MEMBER(segag80r_state::mangled_ports_r)
{
	/* The input ports are odd. Neighboring lines are read via a mux chip  */
	/* one bit at a time. This means that one bank of DIP switches will be */
	/* read as two bits from each of 4 ports. For this reason, the input   */
	/* ports have been organized logically, and are demangled at runtime.  */
	/* 4 input ports each provide 8 bits of information. */
	UINT8 d7d6 = ioport("D7D6")->read();
	UINT8 d5d4 = ioport("D5D4")->read();
	UINT8 d3d2 = ioport("D3D2")->read();
	UINT8 d1d0 = ioport("D1D0")->read();
	int shift = offset & 3;
	return demangle(d7d6 >> shift, d5d4 >> shift, d3d2 >> shift, d1d0 >> shift);
}


READ8_MEMBER(segag80r_state::spaceod_mangled_ports_r)
{
	/* Space Odyssey has different (and conflicting) wiring for upright */
	/* versus cocktail cabinets; we fix this here. The input ports are */
	/* coded for cocktail mode; for upright mode, we manually shuffle the */
	/* bits around. */
	UINT8 d7d6 = ioport("D7D6")->read();
	UINT8 d5d4 = ioport("D5D4")->read();
	UINT8 d3d2 = ioport("D3D2")->read();
	UINT8 d1d0 = ioport("D1D0")->read();
	int shift = offset & 3;

	/* tweak bits for the upright case */
	UINT8 upright = d3d2 & 0x04;
	if (upright)
	{
		UINT8 fc = ioport("FC")->read();
		d7d6 |= 0x60;
		d5d4 = (d5d4 & ~0x1c) |
				((~fc & 0x20) >> 3) | /* IPT_BUTTON2 */
				((~fc & 0x10) >> 1) | /* IPT_BUTTON1 */
				((~fc & 0x08) << 1) | /* IPT_JOYSTICK_UP */
				0xc0;
	}
	return demangle(d7d6 >> shift, d5d4 >> shift, d3d2 >> shift, d1d0 >> shift);
}


READ8_MEMBER(segag80r_state::spaceod_port_fc_r)
{
	UINT8 upright = ioport("D3D2")->read() & 0x04;
	UINT8 fc = ioport("FC")->read();

	/* tweak bits for the upright case */
	if (upright)
	{
		fc = (fc & ~0x03) |
				((fc & 0x02) >> 1) |    /* IPT_JOYSTICK_RIGHT */
				((fc & 0x01) << 1);     /* IPT_JOYSTICK_LEFT */
		fc &= 0x07;
	}
	return fc;
}


WRITE8_MEMBER(segag80r_state::coin_count_w)
{
	coin_counter_w(machine(), 0, (data >> 7) & 1);
	coin_counter_w(machine(), 1, (data >> 6) & 1);
}




/*************************************
 *
 *  sindbad Mystery sound handling
 *
 *************************************/

WRITE8_MEMBER(segag80r_state::sindbadm_soundport_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(50));
}


WRITE8_MEMBER(segag80r_state::sindbadm_misc_w)
{
	coin_counter_w(machine(), 0, data & 0x02);
//  osd_printf_debug("Unknown = %02X\n", data);
}


/* the data lines are flipped */
WRITE8_MEMBER(segag80r_state::sindbadm_sn1_SN76496_w)
{
		m_sn1->write(space, offset, BITSWAP8(data, 0,1,2,3,4,5,6,7));
}
WRITE8_MEMBER(segag80r_state::sindbadm_sn2_SN76496_w)
{
		m_sn2->write(space, offset, BITSWAP8(data, 0,1,2,3,4,5,6,7));
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/* complete memory map derived from schematics */
static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, segag80r_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM     /* CPU board ROM */
	AM_RANGE(0x0800, 0x7fff) AM_ROM     /* PROM board ROM area */
	AM_RANGE(0x8000, 0xbfff) AM_ROM     /* PROM board ROM area */
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(mainram_w) AM_SHARE("mainram")
	AM_RANGE(0xe000, 0xffff) AM_RAM_WRITE(vidram_w) AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, segag80r_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
	AM_RANGE(0x8000, 0xbfff) AM_ROM AM_REGION("maincpu", 0x8000)
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(mainram_w) AM_SHARE("mainram")
	AM_RANGE(0xe000, 0xffff) AM_RAM_WRITE(vidram_w) AM_SHARE("videoram")
ADDRESS_MAP_END


/* complete memory map derived from schematics */
static ADDRESS_MAP_START( main_portmap, AS_IO, 8, segag80r_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xbe, 0xbf) AM_READWRITE(segag80r_video_port_r, segag80r_video_port_w)
	AM_RANGE(0xf9, 0xf9) AM_MIRROR(0x04) AM_WRITE(coin_count_w)
	AM_RANGE(0xf8, 0xfb) AM_READ(mangled_ports_r)
	AM_RANGE(0xfc, 0xfc) AM_READ_PORT("FC")
ADDRESS_MAP_END


static ADDRESS_MAP_START( main_ppi8255_portmap, AS_IO, 8, segag80r_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0xbe, 0xbf) AM_READWRITE(segag80r_video_port_r, segag80r_video_port_w)
	AM_RANGE(0xf9, 0xf9) AM_MIRROR(0x04) AM_WRITE(coin_count_w)
	AM_RANGE(0xf8, 0xfb) AM_READ(mangled_ports_r)
	AM_RANGE(0xfc, 0xfc) AM_READ_PORT("FC")
ADDRESS_MAP_END


static ADDRESS_MAP_START( sindbadm_portmap, AS_IO, 8, segag80r_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x42, 0x43) AM_READWRITE(segag80r_video_port_r, segag80r_video_port_w)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0xf8, 0xfb) AM_READ(mangled_ports_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

/* complete memory map derived from System 1 schematics */
static ADDRESS_MAP_START( sindbadm_sound_map, AS_PROGRAM, 8, segag80r_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0xa000, 0xa003) AM_MIRROR(0x1ffc) AM_WRITE(sindbadm_sn1_SN76496_w)
	AM_RANGE(0xc000, 0xc003) AM_MIRROR(0x1ffc) AM_WRITE(sindbadm_sn2_SN76496_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x1fff) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Generic Port definitions
 *
 *************************************/

static INPUT_PORTS_START( g80r_generic )
	PORT_START("D7D6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)  /* P1.5 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                 /* n/c */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )                 /* n/c */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                 /* n/c */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)  /* P1.8 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.13 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.14 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                 /* n/c */

	PORT_START("D5D4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )               /* P1.10 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )                 /* P1.15 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.16 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.17 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.18 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )                 /* P1.19 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.20 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.21 */

	PORT_START("D3D2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:8" ) /* Listed as "Unused" (astrob) */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:7" ) /* Listed as "Unused" (astrob) */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:6" ) /* Listed as "Unused" (astrob) */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:5" ) /* Listed as "Unused" (astrob) */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:4" ) /* Listed as "Unused" (astrob) */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:3" ) /* Listed as "Unused" (astrob) */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:2" ) /* Listed as "Unused" (astrob) */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:1" ) /* Listed as "Unused" (astrob) */

	PORT_START("D1D0")
	PORT_DIPNAME( 0x0f, 0x03, DEF_STR( Coin_A )) PORT_DIPLOCATION("SW2:8,7,6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x09, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x0a, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x0b, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x0d, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x0e, "1 Coin/2 Credits 5/11" )
	PORT_DIPSETTING(    0x0f, "1 Coin/2 Credits 4/9" )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ))
	PORT_DIPNAME( 0xf0, 0x30, DEF_STR( Coin_B )) PORT_DIPLOCATION("SW2:4,3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x90, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0xa0, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0xb0, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0xc0, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0xd0, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0xe0, "1 Coin/2 Credits 5/11" )
	PORT_DIPSETTING(    0xf0, "1 Coin/2 Credits 4/9" )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_6C ))

	PORT_START("FC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.23 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.24 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.25 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.26 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.27 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.28 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.29 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.30 */

	PORT_START("SERVICESW")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_HIGH ) PORT_CHANGED_MEMBER(DEVICE_SELF, segag80r_state,service_switch, 0)
INPUT_PORTS_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( astrob )
	PORT_INCLUDE(g80r_generic)

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives )) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet )) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x08, 0x00, "Demo Speech" ) PORT_DIPLOCATION("SW1:5") /* Listed as "Unused" */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_MODIFY("FC")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_2WAY PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( astrob2 )
	PORT_INCLUDE(astrob)

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives )) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	//PORT_DIPSETTING(    0x02, "3" )
	//PORT_DIPSETTING(    0x03, "3" )
INPUT_PORTS_END


static INPUT_PORTS_START( 005 )
	PORT_INCLUDE(g80r_generic)

	PORT_MODIFY("D7D6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives )) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet )) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))

	PORT_MODIFY("FC")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_4WAY PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( spaceod )
	PORT_INCLUDE(g80r_generic)

	PORT_MODIFY("D7D6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
/* upright:
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
*/
	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives )) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet )) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Bonus_Life )) PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x10, "40000" )
	PORT_DIPSETTING(    0x08, "60000" )
	PORT_DIPSETTING(    0x18, "80000" )
	PORT_DIPNAME( 0x80, 0x80, "Infinite Lives (Cheat)") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_MODIFY("FC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
/* upright:
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
*/
INPUT_PORTS_END


static INPUT_PORTS_START( monsterb )
	PORT_INCLUDE(g80r_generic)

	PORT_MODIFY("D7D6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives )) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet )) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Difficulty )) PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ))
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ))
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ))
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ))
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Bonus_Life )) PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x40, "20000" )
	PORT_DIPSETTING(    0x60, "40000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ))
	PORT_DIPNAME( 0x80, 0x80, "Infinite Lives (Cheat)") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_MODIFY("FC")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_4WAY PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( pignewt )
	PORT_INCLUDE(g80r_generic)

	PORT_MODIFY("D7D6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet )) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives )) PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x0c, "6" )

	PORT_MODIFY("FC")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_4WAY PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( pignewta )
	PORT_INCLUDE(pignewt)

	PORT_MODIFY("D7D6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("FC")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
INPUT_PORTS_END


static INPUT_PORTS_START( sindbadm )
	PORT_INCLUDE(g80r_generic)

	PORT_MODIFY("D7D6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives )) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet )) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x80, 0x80, "Infinite Lives (Cheat)") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_MODIFY("D1D0")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A )) PORT_DIPLOCATION("SW2:8,7,6,5")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x00, "1 Coin/2 Credits 5/11" )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ))
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B )) PORT_DIPLOCATION("SW2:4,3,2,1")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3" )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x00, "1 Coin/2 Credits 5/11" )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ))

	PORT_MODIFY("FC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	256,
	2,
	{ 0x1000*8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( segag80r )
	GFXDECODE_ENTRY( NULL, 0x0000, charlayout, 0, 16 )
GFXDECODE_END


static GFXDECODE_START( spaceod )
	GFXDECODE_ENTRY( NULL,           0x0000, charlayout,        0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, gfx_8x8x6_planar, 64, 1 )
GFXDECODE_END


static GFXDECODE_START( monsterb )
	GFXDECODE_ENTRY( NULL,           0x0000, charlayout,        0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, gfx_8x8x2_planar, 64, 16 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( g80r_base, segag80r_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, VIDEO_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", segag80r_state,  segag80r_vblank_start)


	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", segag80r)
	MCFG_PALETTE_ADD("palette", 64)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(segag80r_state, screen_update_segag80r)
	MCFG_SCREEN_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( astrob, g80r_base )

	/* basic machine hardware */

	/* sound boards */
	MCFG_FRAGMENT_ADD(astrob_sound_board)
	MCFG_FRAGMENT_ADD(sega_speech_board)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( 005, g80r_base )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(main_ppi8255_portmap)

	/* sound boards */
	MCFG_FRAGMENT_ADD(005_sound_board)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( spaceod, g80r_base )

	/* basic machine hardware */

	/* background board changes */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_GFXDECODE_MODIFY("gfxdecode", spaceod)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(64+64)

	/* sound boards */
	MCFG_FRAGMENT_ADD(spaceod_sound_board)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( monsterb, g80r_base )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(main_ppi8255_portmap)

	/* background board changes */
	MCFG_GFXDECODE_MODIFY("gfxdecode", monsterb)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(64+64)

	/* sound boards */
	MCFG_FRAGMENT_ADD(monsterb_sound_board)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( monster2, monsterb )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pignewt, g80r_base )

	/* basic machine hardware */

	/* background board changes */
	MCFG_GFXDECODE_MODIFY("gfxdecode", monsterb)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(64+64)

	/* sound boards */
	MCFG_SEGAUSB_ADD("usbsnd")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sindbadm, g80r_base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(sindbadm_portmap)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", segag80r_state,  sindbadm_vblank_start)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(segag80r_state, sindbadm_soundport_w))
	MCFG_I8255_IN_PORTB_CB(IOPORT("FC"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(segag80r_state, sindbadm_misc_w))

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", monsterb)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(64+64)

	/* sound boards */

	MCFG_CPU_ADD("audiocpu", Z80, SINDBADM_SOUND_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(sindbadm_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(segag80r_state, irq0_line_hold, 4*60)

	/* sound hardware */
	MCFG_SOUND_ADD("sn1", SN76496, SINDBADM_SOUND_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn2", SN76496, SINDBADM_SOUND_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( astrob )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "829b.cpu-u25",   0x0000, 0x0800, CRC(14ae953c) SHA1(eb63d1b95faa5193db7fa6ab245e99325d519b5e) )
	ROM_LOAD( "907a.prom-u1",   0x0800, 0x0800, CRC(a9aaaf38) SHA1(73c2b9421b267563acb33d63fbbbda818793c4c1) )
	ROM_LOAD( "908a.prom-u2",   0x1000, 0x0800, CRC(897f2b87) SHA1(5468e68053c9a29791f367658630d5c0af332f03) )
	ROM_LOAD( "909a.prom-u3",   0x1800, 0x0800, CRC(55a339e6) SHA1(10f603c2105761ce22c3977e3b76ab568bcde1b3) )
	ROM_LOAD( "910a.prom-u4",   0x2000, 0x0800, CRC(7972b60a) SHA1(17f6a8a9a45c46ccb03242bac7906b235391cffc) )
	ROM_LOAD( "911a.prom-u5",   0x2800, 0x0800, CRC(af87520f) SHA1(df92d0c38c1ab6dc0464d986d89afe3cc02f3dd6) )
	ROM_LOAD( "912a.prom-u6",   0x3000, 0x0800, CRC(b656f929) SHA1(1b8545603f724a37727d175614fcca340714a7e3) )
	ROM_LOAD( "913a.prom-u7",   0x3800, 0x0800, CRC(321074b3) SHA1(73c306326cea202f9481daf52a76bd9d72ef691a) )
	ROM_LOAD( "914a.prom-u8",   0x4000, 0x0800, CRC(90d2493e) SHA1(6343cdd61a6e47edcbc6d60c8d124a56d3060e96) )
	ROM_LOAD( "915a.prom-u9",   0x4800, 0x0800, CRC(aaf828d1) SHA1(ac175746ff24b5ebdc78a4b7ca0c37185ffa3046) )
	ROM_LOAD( "916a.prom-u10",  0x5000, 0x0800, CRC(56d92ab9) SHA1(fd024fd178c9714fd55241f0d7081f4f3c07c463) )
	ROM_LOAD( "917a.prom-u11",  0x5800, 0x0800, CRC(9dcdaf2d) SHA1(21d0b1e555c31ec9db14cf297be4965f9eb162ec) )
	ROM_LOAD( "918a.prom-u12",  0x6000, 0x0800, CRC(c9d09655) SHA1(c9f642a9daab994a6d58a5f8a2f058041a0df5c2) )
	ROM_LOAD( "919a.prom-u13",  0x6800, 0x0800, CRC(448bd318) SHA1(ba1467ebc3667e935a0153b207763ee4fcef6007) )
	ROM_LOAD( "920a.prom-u14",  0x7000, 0x0800, CRC(3524a383) SHA1(310340218ce2881137a0781dc51fe406b220d140) )
	ROM_LOAD( "921a.prom-u15",  0x7800, 0x0800, CRC(98c14834) SHA1(5a5a25d9eb92eaf820b10dbed1820a4210493d8f) )
	ROM_LOAD( "922a.prom-u16",  0x8000, 0x0800, CRC(4311513c) SHA1(ec2e4d92eb128b3b75464379324f7e0341ae2494) )
	ROM_LOAD( "923a.prom-u17",  0x8800, 0x0800, CRC(50f0462c) SHA1(659552ff8d6e6cf71f9f250d95e025371a10a2d0) )
	ROM_LOAD( "924a.prom-u18",  0x9000, 0x0800, CRC(120a39c7) SHA1(d8fdf97290725cf9ebddab9eeb34d7adba097394) )
	ROM_LOAD( "925a.prom-u19",  0x9800, 0x0800, CRC(790a7f4e) SHA1(16b7b8e864a8f5f59da6bf2ad17f1e4791f34abe) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "808b.speech-u7", 0x0000, 0x0800, CRC(5988c767) SHA1(3b91a8cd46aa7e714028cc40f700fea32287afb1) )

	ROM_REGION( 0x4000, "speech", 0 )
	ROM_LOAD( "809a.speech-u6", 0x0000, 0x0800, CRC(893f228d) SHA1(41c08210d322105f5446cfaa1258c194dd078a34) )
	ROM_LOAD( "810.speech-u5",  0x0800, 0x0800, CRC(ff0163c5) SHA1(158a12f9bf01d25c7e98f34fce56df51d49e5a85) )
	ROM_LOAD( "811.speech-u4",  0x1000, 0x0800, CRC(219f3978) SHA1(728edb9251f7cde237fa3b005971366a099c6342) )
	ROM_LOAD( "812a.speech-u3", 0x1800, 0x0800, CRC(410ad0d2) SHA1(9b5f05bb64a6ecfe3543025a10c6ec67de797333) )
ROM_END

ROM_START( astrob2 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "829b.cpu-u25",   0x0000, 0x0800, CRC(14ae953c) SHA1(eb63d1b95faa5193db7fa6ab245e99325d519b5e) )
	ROM_LOAD( "888a.prom-u1",   0x0800, 0x0800, CRC(42601744) SHA1(6bb58384c28b2105746a2f410f5e0979609db9bf) )
	ROM_LOAD( "889.prom-u2",    0x1000, 0x0800, CRC(dd9ab173) SHA1(35617bb4480a668bdd07b0f8a6a879fd83c53448) )
	ROM_LOAD( "890.prom-u3",    0x1800, 0x0800, CRC(26f5b4cf) SHA1(be45e802f976b8847689ae8de7159843ac9100eb) )
	ROM_LOAD( "891.prom-u4",    0x2000, 0x0800, CRC(6437c95f) SHA1(c9340d8edcbee254976f8ed089261fce7ae76017) )
	ROM_LOAD( "892.prom-u5",    0x2800, 0x0800, CRC(2d3c949b) SHA1(17e3c5300793f2345ff6e28e82cd7a22f1d6e41f) )
	ROM_LOAD( "893.prom-u6",    0x3000, 0x0800, CRC(ccdb1a76) SHA1(1c8f0555e397c5558bbfca1fa1487cc32aca8592) )
	ROM_LOAD( "894.prom-u7",    0x3800, 0x0800, CRC(66ae5ced) SHA1(81bb6e3adcc76ffbeafefecce5fe5541a7eefc37) )
	ROM_LOAD( "895.prom-u8",    0x4000, 0x0800, CRC(202cf3a3) SHA1(26fcccfb3e94b2a01d38c14daa66713c223efb18) )
	ROM_LOAD( "896.prom-u9",    0x4800, 0x0800, CRC(b603fe23) SHA1(3128877355a9c5bba5cd22e9addf4c8b79ee39d2) )
	ROM_LOAD( "897.prom-u10",   0x5000, 0x0800, CRC(989198c6) SHA1(3344bf7272e388571026c4e68a2e4e5e0ebbc5e3) )
	ROM_LOAD( "898.prom-u11",   0x5800, 0x0800, CRC(ef2bab04) SHA1(108a9812cb9d1ec4629b0306c45ba164f94ab426) )
	ROM_LOAD( "899.prom-u12",   0x6000, 0x0800, CRC(e0d189ee) SHA1(dcab31d64e6b2d248a82cbae9e37afe031dfc6cd) )
	ROM_LOAD( "900.prom-u13",   0x6800, 0x0800, CRC(682d4604) SHA1(6ac0d2d8ff407cc7e10b460736ae7fbc21148640) )
	ROM_LOAD( "901.prom-u14",   0x7000, 0x0800, CRC(9ed11c61) SHA1(dd965c06d2013acdabd958e713109eeb049d5d5e) )
	ROM_LOAD( "902.prom-u15",   0x7800, 0x0800, CRC(b4d6c330) SHA1(922a562b5f1a8a286e6777ba7d141bd0db6e2a92) )
	ROM_LOAD( "903a.prom-u16",  0x8000, 0x0800, CRC(84acc38c) SHA1(86bed143ac2d95116e50e77b5c262d67156c6a59) )
	ROM_LOAD( "904.prom-u17",   0x8800, 0x0800, CRC(5eba3097) SHA1(e785d1c1cea50aa25e5eea5e58a0c48fd53208c6) )
	ROM_LOAD( "905.prom-u18",   0x9000, 0x0800, CRC(4f08f9f4) SHA1(755a825b18ed50caa7bf274a0a5c3a1b00b1c070) )
	ROM_LOAD( "906.prom-u19",   0x9800, 0x0800, CRC(58149df1) SHA1(2bba56576a225ca47ce31a5b6dcc491546dfffec) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "808b.speech-u7", 0x0000, 0x0800, CRC(5988c767) SHA1(3b91a8cd46aa7e714028cc40f700fea32287afb1) )

	ROM_REGION( 0x4000, "speech", 0 )
	ROM_LOAD( "809a.speech-u6", 0x0000, 0x0800, CRC(893f228d) SHA1(41c08210d322105f5446cfaa1258c194dd078a34) )
	ROM_LOAD( "810.speech-u5",  0x0800, 0x0800, CRC(ff0163c5) SHA1(158a12f9bf01d25c7e98f34fce56df51d49e5a85) )
	ROM_LOAD( "811.speech-u4",  0x1000, 0x0800, CRC(219f3978) SHA1(728edb9251f7cde237fa3b005971366a099c6342) )
	ROM_LOAD( "812a.speech-u3", 0x1800, 0x0800, CRC(410ad0d2) SHA1(9b5f05bb64a6ecfe3543025a10c6ec67de797333) )
ROM_END

ROM_START( astrob2a )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "829b.cpu-u25",   0x0000, 0x0800, CRC(14ae953c) SHA1(eb63d1b95faa5193db7fa6ab245e99325d519b5e) )
	ROM_LOAD( "888c.prom-u1",   0x0800, 0x0800, CRC(15fa9c3e) SHA1(8998a5fa21765eec1c393373e38015552735d119) )
	ROM_LOAD( "889.prom-u2",    0x1000, 0x0800, CRC(dd9ab173) SHA1(35617bb4480a668bdd07b0f8a6a879fd83c53448) )
	ROM_LOAD( "890.prom-u3",    0x1800, 0x0800, CRC(26f5b4cf) SHA1(be45e802f976b8847689ae8de7159843ac9100eb) )
	ROM_LOAD( "891.prom-u4",    0x2000, 0x0800, CRC(6437c95f) SHA1(c9340d8edcbee254976f8ed089261fce7ae76017) )
	ROM_LOAD( "892.prom-u5",    0x2800, 0x0800, CRC(2d3c949b) SHA1(17e3c5300793f2345ff6e28e82cd7a22f1d6e41f) )
	ROM_LOAD( "893.prom-u6",    0x3000, 0x0800, CRC(ccdb1a76) SHA1(1c8f0555e397c5558bbfca1fa1487cc32aca8592) )
	ROM_LOAD( "894.prom-u7",    0x3800, 0x0800, CRC(66ae5ced) SHA1(81bb6e3adcc76ffbeafefecce5fe5541a7eefc37) )
	ROM_LOAD( "895.prom-u8",    0x4000, 0x0800, CRC(202cf3a3) SHA1(26fcccfb3e94b2a01d38c14daa66713c223efb18) )
	ROM_LOAD( "896.prom-u9",    0x4800, 0x0800, CRC(b603fe23) SHA1(3128877355a9c5bba5cd22e9addf4c8b79ee39d2) )
	ROM_LOAD( "897b.prom-u10",  0x5000, 0x0800, CRC(a4f6008a) SHA1(79749ca9536cb8326d2d94cbce85cd1243733761) )
	ROM_LOAD( "898a.prom-u11",  0x5800, 0x0800, CRC(a92c7826) SHA1(dbc853d29616ab76832c9f702fab689af5012f26) )
	ROM_LOAD( "899.prom-u12",   0x6000, 0x0800, CRC(e0d189ee) SHA1(dcab31d64e6b2d248a82cbae9e37afe031dfc6cd) )
	ROM_LOAD( "900.prom-u13",   0x6800, 0x0800, CRC(682d4604) SHA1(6ac0d2d8ff407cc7e10b460736ae7fbc21148640) )
	ROM_LOAD( "901.prom-u14",   0x7000, 0x0800, CRC(9ed11c61) SHA1(dd965c06d2013acdabd958e713109eeb049d5d5e) )
	ROM_LOAD( "902.prom-u15",   0x7800, 0x0800, CRC(b4d6c330) SHA1(922a562b5f1a8a286e6777ba7d141bd0db6e2a92) )
	ROM_LOAD( "903c.prom-u16",  0x8000, 0x0800, CRC(ec8c8d98) SHA1(1148a120d054c7d518774883e106d5d65fdb20ff) )
	ROM_LOAD( "904a.prom-u17",  0x8800, 0x0800, CRC(d1df0f3e) SHA1(c5baa3c430327838f0135929b943d07cfe8a27c7) )
	ROM_LOAD( "905.prom-u18",   0x9000, 0x0800, CRC(4f08f9f4) SHA1(755a825b18ed50caa7bf274a0a5c3a1b00b1c070) )
	ROM_LOAD( "906.prom-u19",   0x9800, 0x0800, CRC(58149df1) SHA1(2bba56576a225ca47ce31a5b6dcc491546dfffec) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "808b.speech-u7", 0x0000, 0x0800, CRC(5988c767) SHA1(3b91a8cd46aa7e714028cc40f700fea32287afb1) )

	ROM_REGION( 0x4000, "speech", 0 )
	ROM_LOAD( "809a.speech-u6", 0x0000, 0x0800, CRC(893f228d) SHA1(41c08210d322105f5446cfaa1258c194dd078a34) )
	ROM_LOAD( "810.speech-u5",  0x0800, 0x0800, CRC(ff0163c5) SHA1(158a12f9bf01d25c7e98f34fce56df51d49e5a85) )
	ROM_LOAD( "811.speech-u4",  0x1000, 0x0800, CRC(219f3978) SHA1(728edb9251f7cde237fa3b005971366a099c6342) )
	ROM_LOAD( "812a.speech-u3", 0x1800, 0x0800, CRC(410ad0d2) SHA1(9b5f05bb64a6ecfe3543025a10c6ec67de797333) )
ROM_END

ROM_START( astrob1 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "829.cpu-u25",    0x0000, 0x0800, CRC(5f66046e) SHA1(6aa7f94122db1a75a89c12ad9d087aec1a70d675) ) // verify that this isn't just a bad dump..
	ROM_LOAD( "837.prom-u1",    0x0800, 0x0800, CRC(ce9c3763) SHA1(2cb4c3041905d38b040ef76f69f6197d699f9ec5) )
	ROM_LOAD( "838.prom-u2",    0x1000, 0x0800, CRC(3557289e) SHA1(57258b149f0872c34f82fe3dc06bc1fc60d7393f) )
	ROM_LOAD( "839.prom-u3",    0x1800, 0x0800, CRC(c88bda24) SHA1(9253306bd929e04a2136a0d9a9627dab8979baa1) )
	ROM_LOAD( "840.prom-u4",    0x2000, 0x0800, CRC(24c9fe23) SHA1(306ad65c8c821ae90eb50416c38fe37c7acf36de) )
	ROM_LOAD( "841.prom-u5",    0x2800, 0x0800, CRC(f153c683) SHA1(a6338a0e9867e1cbe27921182c268aeb4b09a503) )
	ROM_LOAD( "842.prom-u6",    0x3000, 0x0800, CRC(4c5452b2) SHA1(e7b1138379ba4ad29f5ffad275164716c37c92e9) )
	ROM_LOAD( "843.prom-u7",    0x3800, 0x0800, CRC(673161a6) SHA1(90db12ce06cd6150a924421cf9d93ad4051ccf97) )
	ROM_LOAD( "844.prom-u8",    0x4000, 0x0800, CRC(6bfc59fd) SHA1(a5f7edfa66a25e3e84e60b5b3812d16ffdc5e409) )
	ROM_LOAD( "845.prom-u9",    0x4800, 0x0800, CRC(018623f3) SHA1(bf264921c36ced3c332d5627b80886c3463f2237) )
	ROM_LOAD( "846.prom-u10",   0x5000, 0x0800, CRC(4d7c5fb3) SHA1(3c0dd17cb907b8476abb4b2a16bd41dd2f107849) )
	ROM_LOAD( "847.prom-u11",   0x5800, 0x0800, CRC(24d1d50a) SHA1(0691cccc4e58ffa07308a6a3ae80fbcef6cc8c9b) )
	ROM_LOAD( "848.prom-u12",   0x6000, 0x0800, CRC(1c145541) SHA1(73e23161f71f4c25405e1211f41a31323845efaa) )
	ROM_LOAD( "849.prom-u13",   0x6800, 0x0800, CRC(d378c169) SHA1(9b30ff9741429d798c8a2146d1c76a223f05495c) )
	ROM_LOAD( "850.prom-u14",   0x7000, 0x0800, CRC(9da673ae) SHA1(817fb7cbeedefa3b5b43ca3b7914289f6908ed53) )
	ROM_LOAD( "851.prom-u15",   0x7800, 0x0800, CRC(3d4cf9f0) SHA1(11e996f33f3a104e50d0a54a0814ea3e07735683) )
	ROM_LOAD( "852.prom-u16",   0x8000, 0x0800, CRC(af88a97e) SHA1(fe7993101c629b296b5da05519b0990cc2b78286) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "808b.speech-u7", 0x0000, 0x0800, CRC(5988c767) SHA1(3b91a8cd46aa7e714028cc40f700fea32287afb1) )

	ROM_REGION( 0x4000, "speech", 0 )
	ROM_LOAD( "809a.speech-u6", 0x0000, 0x0800, CRC(893f228d) SHA1(41c08210d322105f5446cfaa1258c194dd078a34) )
	ROM_LOAD( "810.speech-u5",  0x0800, 0x0800, CRC(ff0163c5) SHA1(158a12f9bf01d25c7e98f34fce56df51d49e5a85) )
	ROM_LOAD( "811.speech-u4",  0x1000, 0x0800, CRC(219f3978) SHA1(728edb9251f7cde237fa3b005971366a099c6342) )
	ROM_LOAD( "812a.speech-u3", 0x1800, 0x0800, CRC(410ad0d2) SHA1(9b5f05bb64a6ecfe3543025a10c6ec67de797333) )
ROM_END

ROM_START( astrobg )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "829b.u25",   0x0000, 0x0800, CRC(14ae953c) SHA1(eb63d1b95faa5193db7fa6ab245e99325d519b5e) )
	ROM_LOAD( "834b.u01",   0x0800, 0x0800, CRC(82630950) SHA1(6e13cf5868d64835d9de823801cc4162b3c7b316) )
	ROM_LOAD( "814a.u02",   0x1000, 0x0800, CRC(d70d7d5e) SHA1(3d663cb36579c91cdb0ea82fd0af3b0ade8246b7) )
	ROM_LOAD( "815a.u03",   0x1800, 0x0800, CRC(0dbad477) SHA1(1fc6b3c628abd0d7a13b75eb2aeed7c04381f674) )
	ROM_LOAD( "816a.u04",   0x2000, 0x0800, CRC(8fa809ab) SHA1(c1dcae3b825dc283526b5ba742ca6816e5722464) )
	ROM_LOAD( "817a.u05",   0x2800, 0x0800, CRC(c7a3c014) SHA1(da25c2c2a116128e74df4de21727e2a4820c1045) )
	ROM_LOAD( "818a.u06",   0x3000, 0x0800, CRC(f13e804c) SHA1(550a02a4d21915d1127ba1be587539a8e27aa7ef) )
	ROM_LOAD( "819a.u07",   0x3800, 0x0800, CRC(2194a624) SHA1(a8a9baf202eb353b93dced71d74de713d54a5e5e) )
	ROM_LOAD( "820a.u08",   0x4000, 0x0800, CRC(95d84829) SHA1(a1684a695de41270721da851bed887a0c379e8c1) )
	ROM_LOAD( "821a.u09",   0x4800, 0x0800, CRC(1495059f) SHA1(d771274d5205421757076b9a5cfbd73fe2949901) )
	ROM_LOAD( "822a.u10",   0x5000, 0x0800, CRC(d036a5c5) SHA1(5557795280c5e492b94d670b156024d67d51aa8b) )
	ROM_LOAD( "823a.u11",   0x5800, 0x0800, CRC(788380dd) SHA1(6392386f9625c6fd609174e765a24e2c25fca957) )
	ROM_LOAD( "824a.u12",   0x6000, 0x0800, CRC(a43686c4) SHA1(1a6eae526de1667a7aa3a995b2655950555461eb) )
	ROM_LOAD( "825a.u13",   0x6800, 0x0800, CRC(43c8b973) SHA1(12ee379d24e2bcdd6b491262ac02f862a8a33aaf) )
	ROM_LOAD( "826a.u14",   0x7000, 0x0800, CRC(9f26d132) SHA1(5323a61de883ed1f2dca776b9d7c53a484249c02) )
	ROM_LOAD( "835.u15",    0x7800, 0x0800, CRC(6eeeb409) SHA1(1caf9b7ac08a4adcbf8c17f9e4b398373db706e1) )
	ROM_LOAD( "836.u16",    0x8000, 0x0800, CRC(07ffe6dc) SHA1(70673e8266139034afa64bf980b1b9ddbf294e0f) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "808b_speech_de.u07", 0x0000, 0x0800, CRC(5988c767) SHA1(3b91a8cd46aa7e714028cc40f700fea32287afb1) )

	ROM_REGION( 0x4000, "speech", 0 )
	ROM_LOAD( "830_speech_de.u06", 0x0000, 0x0800, CRC(2d840552) SHA1(7a2a7b54378b6cc85b8ab5c26e42266aa747c635) )
	ROM_LOAD( "831_speech_de.u05", 0x0800, 0x0800, CRC(46b30ee4) SHA1(c9e19a9b9ebc9b3b853e79f93ad74e4ec5dfd1ae) )
	ROM_LOAD( "832_speech_de.u04", 0x1000, 0x0800, CRC(d05280b8) SHA1(8d30b23b83b32465a8a2decd2ce9bfed24394e7e) )
	ROM_LOAD( "833_speech_de.u03", 0x1800, 0x0800, CRC(08f11459) SHA1(da6dc2bf30b95882f95c21739ec02fc89d286a66) )
ROM_END

ROM_START( 005 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "1346b.cpu-u25",     0x0000, 0x0800, CRC(8e68533e) SHA1(a257c556d31691068ed5c991f1fb2b51da4826db) )
	ROM_LOAD( "5092.prom-u1",      0x0800, 0x0800, CRC(29e10a81) SHA1(c4b4e6c75bcf276e53f39a456d8d633c83dcf485) )
	ROM_LOAD( "5093.prom-u2",      0x1000, 0x0800, CRC(e1edc3df) SHA1(4f593546bbb0f50850dc6286cb514af6831c27a7) )
	ROM_LOAD( "5094.prom-u3",      0x1800, 0x0800, CRC(995773bb) SHA1(98dd826527853bc031edfb9a821778cc3e906150) )
	ROM_LOAD( "5095.prom-u4",      0x2000, 0x0800, CRC(f887f575) SHA1(de96573a91b60b090b1f441f1410ecad63c9467c) )
	ROM_LOAD( "5096.prom-u5",      0x2800, 0x0800, CRC(5545241e) SHA1(ee504ccaab469100137717341a1b461175ff792d) )
	ROM_LOAD( "5097.prom-u6",      0x3000, 0x0800, CRC(428edb54) SHA1(4f3df6017068d939014a8f638f28e3228acb7add) )
	ROM_LOAD( "5098.prom-u7",      0x3800, 0x0800, CRC(5bcb9d63) SHA1(c0c91bc9f75ad88a6e15c554a980d5c075725fe8) )
	ROM_LOAD( "5099.prom-u8",      0x4000, 0x0800, CRC(0ea24ba3) SHA1(95a30c9b63ef1c346df0da71af3fdecd1a75cb8f) )
	ROM_LOAD( "5100.prom-u9",      0x4800, 0x0800, CRC(a79af131) SHA1(0ba34130174e196015bc9b9c135c420209dfd524) )
	ROM_LOAD( "5101.prom-u10",     0x5000, 0x0800, CRC(8a1cdae0) SHA1(f7c617f9bdb7818e6069a981d0c8820deade134c) )
	ROM_LOAD( "5102.prom-u11",     0x5800, 0x0800, CRC(70826a15) SHA1(a86322d0e8a88534e9b78dcde42ae4c441276913) )
	ROM_LOAD( "5103.prom-u12",     0x6000, 0x0800, CRC(7f80c5b0) SHA1(00748cd5fc7f75fdca194e748524d406c006296d) )
	ROM_LOAD( "5104.prom-u13",     0x6800, 0x0800, CRC(0140930e) SHA1(f8ef894c46d3663bd89d2d817675a67075d3e0d6) )
	ROM_LOAD( "5105.prom-u14",     0x7000, 0x0800, CRC(17807a05) SHA1(bd99f5beab0155f6e4d2fab2fa5f4e147c5730d5) )
	ROM_LOAD( "5106.prom-u15",     0x7800, 0x0800, CRC(c7cdfa9d) SHA1(6ab7adc60ac7bb53a7175e8de51924008737c9ac) )
	ROM_LOAD( "5107.prom-u16",     0x8000, 0x0800, CRC(95f8a2e6) SHA1(89c92e000b3e1630380db779370cf9f5b13e5719) )
	ROM_LOAD( "5108.prom-u17",     0x8800, 0x0800, CRC(d371cacd) SHA1(8f2cdcc0b4e3b77e0958d257e37accefc5749cde) )
	ROM_LOAD( "5109.prom-u18",     0x9000, 0x0800, CRC(48a20617) SHA1(5b4bc3beda0404ff0a61bb42751b87f71817f363) )
	ROM_LOAD( "5110.prom-u19",     0x9800, 0x0800, CRC(7d26111a) SHA1(a6d3652ae606a5b75026e524c9d6aaa78300741e) )
	ROM_LOAD( "5111.prom-u20",     0xa000, 0x0800, CRC(a888e175) SHA1(4c0af94441bf51dfc852372a5b90d0830df81363) )

	ROM_REGION( 0x0800, "005", 0 )
	ROM_LOAD( "epr-1286.sound-16", 0x0000, 0x0800, CRC(fbe0d501) SHA1(bfa277689790f835d8a43be4beee0581e1096bcc) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6331.sound-u8",     0x0000, 0x0020, BAD_DUMP CRC(1d298cb0) SHA1(bb0bb62365402543e3154b9a77be9c75010e6abc) )  /* missing sound PROM! */
ROM_END


ROM_START( spaceod )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "so-959.cpu-25",     0x0000, 0x0800, CRC(bbae3cd1) SHA1(2e99fd4b0db60462721b174b0db1b10b2fd13d25) )
	ROM_LOAD( "spod941b.prom-u1",  0x0800, 0x0800, CRC(b396c097) SHA1(6fbff7cb72662f5d3e6940784c14ae40d03885d0) )
	ROM_LOAD( "spod942a.prom-u2",  0x1000, 0x0800, CRC(e4451554) SHA1(ed2da0455c6d135cdc2fb0c0ed92e496f1871cdf) )
	ROM_LOAD( "spod943a.prom-u3",  0x1800, 0x0800, CRC(0d87a8f3) SHA1(04aaa5f58f4b656d058b116da118bd5731c3fad8) )
	ROM_LOAD( "spod944a.prom-u4",  0x2000, 0x0800, CRC(9c79f505) SHA1(219a1ae73711b485d7b97801d8b25f31046826b0) )
	ROM_LOAD( "spod945a.prom-u5",  0x2800, 0x0800, CRC(375681e4) SHA1(3c79b43f5e91e8f13788e7f11abdcc33ef12d90a) )
	ROM_LOAD( "spod946a.prom-u6",  0x3000, 0x0800, BAD_DUMP CRC(4a27ff64) SHA1(bec51c734dfab2fad1be1ddc992bc954e4a1fc81) )
	ROM_LOAD( "spod947a.prom-u7",  0x3800, 0x0800, CRC(6170de1c) SHA1(5939c5a7fbd0d7b73d4d20a6c641753042682293) )
	ROM_LOAD( "spod948a.prom-u8",  0x4000, 0x0800, CRC(44fbbc87) SHA1(199b9214a44bda053d711348329cea80209cbaf1) )
	ROM_LOAD( "spod949a.prom-u9",  0x4800, 0x0800, CRC(b2705596) SHA1(fc02a929b28985fcf6995aa2ccfb5a361183c1be) )
	ROM_LOAD( "so-950.prom-u10",   0x5000, 0x0800, CRC(70a7a3b4) SHA1(4c7da7571039f583689cb388c9dd28b605d06fb2) )
	ROM_LOAD( "spod951a.prom-u11", 0x5800, 0x0800, CRC(2bb7b5a3) SHA1(40fee5a8fd1265dd0b5b3eb9624fa4aefa04cc7d) )
	ROM_LOAD( "so-952.prom-u12",   0x6000, 0x0800, CRC(5bf19a12) SHA1(3bc2977b15072d1fa1e28145b4c3abfb94bd8db5) )
	ROM_LOAD( "so-953.prom-u13",   0x6800, 0x0800, CRC(8066ac83) SHA1(7b2d158368cf1a325c1736a83152ee3226515a9e) )
	ROM_LOAD( "so-954.prom-u14",   0x7000, 0x0800, CRC(44ed6a0d) SHA1(974319958a6d7c023b842980e9b09f1b42f65105) )
	ROM_LOAD( "spod955a.prom-u15", 0x7800, 0x0800, CRC(254ca0fa) SHA1(506bb44885020daaeb608e805cf9e6d2a6b67d8a) )
	ROM_LOAD( "so-956.prom-u16",   0x8000, 0x0800, CRC(97de45a9) SHA1(d4086dc55d25eeedb481e77da87fc13a59a65228) )
	ROM_LOAD( "spod957a.prom-u17", 0x8800, 0x0800, CRC(1e31d118) SHA1(94b9799e99247792d07d8402a7bf0bc4c4f11e78) )
	ROM_LOAD( "spod958a.prom-u18", 0x9000, 0x0800, CRC(aa07e8e7) SHA1(0356047ba92edf445b38e17e9452b917866382f2) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "epr-18.bg-u10",     0x0000, 0x1000, CRC(24a81c04) SHA1(99a2b271ecf0a00a10188f93c49318a0720aea88) )
	ROM_LOAD( "epr-17.bg-u9",      0x1000, 0x1000, CRC(6c7490c0) SHA1(b0af14dd6510d230ea799e61f83e9c89e8ac5e19) )
	ROM_LOAD( "epr-16.bg-u8",      0x2000, 0x1000, CRC(acdf203e) SHA1(89c4974d3f61c152976343d0ff2ab7136105e80f) )
	ROM_LOAD( "epr-15.bg-u7",      0x3000, 0x1000, CRC(ae0e5d71) SHA1(083a24d42e8f72feb1f60b2cf004ce2be38b9910) )
	ROM_LOAD( "epr-14.bg-u6",      0x4000, 0x1000, CRC(d2ebd915) SHA1(c9bf888c10b8c91196696e1d42e18826eb7c98a0) )
	ROM_LOAD( "epr-13.bg-u5",      0x5000, 0x1000, CRC(74bd7f9a) SHA1(daa3304916e05c35a48d7be5d283121e94a1a6c7) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "epr-09.bg-u1",      0x0000, 0x1000, CRC(a87bfc0a) SHA1(7a5000f757c723a618d89c0732a5ae8818ba0f64) )
	ROM_LOAD( "epr-10.bg-u2",      0x1000, 0x1000, CRC(8ce88100) SHA1(61177a75512bbf3629df6329e23f06fc470f36ed) )
	ROM_LOAD( "epr-11.bg-u3",      0x2000, 0x1000, CRC(1bdbdab5) SHA1(3bb60c7bd029dd53bac7ebe640206d3980b3b426) )
	ROM_LOAD( "epr-12.bg-u4",      0x3000, 0x1000, CRC(629a4a1f) SHA1(19badfa72207cb750364f4cab229529078b7af63) )
ROM_END

ROM_START( spaceod2 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "so-959.cpu-25",   0x0000, 0x0800, CRC(bbae3cd1) SHA1(2e99fd4b0db60462721b174b0db1b10b2fd13d25) )
	ROM_LOAD( "so-941.prom-u1",  0x0800, 0x0800, CRC(8b63585a) SHA1(eb064a2dca5cb44373f1acc86243a3dcca1951ee) )
	ROM_LOAD( "so-942.prom-u2",  0x1000, 0x0800, CRC(93e7d900) SHA1(dd2ec1a48a243fd1abc5467e600b8aa10dca1cfb) )
	ROM_LOAD( "so-943.prom-u3",  0x1800, 0x0800, CRC(e2f5dc10) SHA1(7b5fb00b4a46dd8eb8e0017601aae6ca771550d9) )
	ROM_LOAD( "so-944.prom-u4",  0x2000, 0x0800, CRC(b5ab01e9) SHA1(7b076a92df4cff792e2d3c2d3f78a8bcceac35aa) )
	ROM_LOAD( "so-945.prom-u5",  0x2800, 0x0800, CRC(6c5fa1b1) SHA1(35d3aed689ab8179db0aef55e9e3e48b4804837b) )
	ROM_LOAD( "so-946.prom-u6",  0x3000, 0x0800, CRC(4cef25d6) SHA1(6249ef9a906ad07bfa09fe618443538ae3f83359) )
	ROM_LOAD( "so-947.prom-u7",  0x3800, 0x0800, CRC(7220fc42) SHA1(3d939aeabb1e2eba3e788232e6df2c74b524abac) )
	ROM_LOAD( "so-948.prom-u8",  0x4000, 0x0800, CRC(94bcd726) SHA1(1f9efb81d04ea8dc970738fc388e91a70e9335ac) )
	ROM_LOAD( "so-949.prom-u9",  0x4800, 0x0800, CRC(e11e7034) SHA1(a1e80a1c0bc2cf600c84df0000007228d30f0935) )
	ROM_LOAD( "so-950.prom-u10", 0x5000, 0x0800, CRC(70a7a3b4) SHA1(4c7da7571039f583689cb388c9dd28b605d06fb2) )
	ROM_LOAD( "so-951.prom-u11", 0x5800, 0x0800, CRC(f5f0d3f9) SHA1(ea6db0e3178374190191c8c27bb1752fa6fdcc34) )
	ROM_LOAD( "so-952.prom-u12", 0x6000, 0x0800, CRC(5bf19a12) SHA1(3bc2977b15072d1fa1e28145b4c3abfb94bd8db5) )
	ROM_LOAD( "so-953.prom-u13", 0x6800, 0x0800, CRC(8066ac83) SHA1(7b2d158368cf1a325c1736a83152ee3226515a9e) )
	ROM_LOAD( "so-954.prom-u14", 0x7000, 0x0800, CRC(44ed6a0d) SHA1(974319958a6d7c023b842980e9b09f1b42f65105) )
	ROM_LOAD( "so-955.prom-u15", 0x7800, 0x0800, CRC(b5e2748d) SHA1(acac198f17f3245e2f5de2aa2b4d3f8d50311332) )
	ROM_LOAD( "so-956.prom-u16", 0x8000, 0x0800, CRC(97de45a9) SHA1(d4086dc55d25eeedb481e77da87fc13a59a65228) )
	ROM_LOAD( "so-957.prom-u17", 0x8800, 0x0800, CRC(c14b98c4) SHA1(b44fb6bee12c14190fd88bfc1b566181529dfdd7) )
	ROM_LOAD( "so-958.prom-u18", 0x9000, 0x0800, CRC(4c0a7242) SHA1(ba8317cea6986bed445ed333136e6a9649c6a89a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "epr-18.bg-u10",   0x0000, 0x1000, CRC(24a81c04) SHA1(99a2b271ecf0a00a10188f93c49318a0720aea88) )
	ROM_LOAD( "epr-17.bg-u9",    0x1000, 0x1000, CRC(6c7490c0) SHA1(b0af14dd6510d230ea799e61f83e9c89e8ac5e19) )
	ROM_LOAD( "epr-16.bg-u8",    0x2000, 0x1000, CRC(acdf203e) SHA1(89c4974d3f61c152976343d0ff2ab7136105e80f) )
	ROM_LOAD( "epr-15.bg-u7",    0x3000, 0x1000, CRC(ae0e5d71) SHA1(083a24d42e8f72feb1f60b2cf004ce2be38b9910) )
	ROM_LOAD( "epr-14.bg-u6",    0x4000, 0x1000, CRC(d2ebd915) SHA1(c9bf888c10b8c91196696e1d42e18826eb7c98a0) )
	ROM_LOAD( "epr-13.bg-u5",    0x5000, 0x1000, CRC(74bd7f9a) SHA1(daa3304916e05c35a48d7be5d283121e94a1a6c7) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "epr-09.bg-u1",    0x0000, 0x1000, CRC(a87bfc0a) SHA1(7a5000f757c723a618d89c0732a5ae8818ba0f64) )
	ROM_LOAD( "epr-10.bg-u2",    0x1000, 0x1000, CRC(8ce88100) SHA1(61177a75512bbf3629df6329e23f06fc470f36ed) )
	ROM_LOAD( "epr-11.bg-u3",    0x2000, 0x1000, CRC(1bdbdab5) SHA1(3bb60c7bd029dd53bac7ebe640206d3980b3b426) )
	ROM_LOAD( "epr-12.bg-u4",    0x3000, 0x1000, CRC(629a4a1f) SHA1(19badfa72207cb750364f4cab229529078b7af63) )
ROM_END


ROM_START( monsterb )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "1778.cpu-u25",   0x0000, 0x0800, CRC(19761be3) SHA1(551a5eb958b6efac41f32e7feb2786400fcfb6d3) )
	ROM_LOAD( "1779c.prom-u1",  0x0800, 0x0800, CRC(5b67dc4c) SHA1(5d2c5128b6cba2d8aa98cae8cb78dbe0c998e965) )
	ROM_LOAD( "1780b.prom-u2",  0x1000, 0x0800, CRC(fac5aac6) SHA1(52a6b98760f011aa68f374801cddf0aa3efa4e69) )
	ROM_LOAD( "1781b.prom-u3",  0x1800, 0x0800, CRC(3b104103) SHA1(50c68144cd76343f0e7cde35a655994f3063250f) )
	ROM_LOAD( "1782b.prom-u4",  0x2000, 0x0800, CRC(c1523553) SHA1(c63d77b3add7afed54454d7b7bfc4f42276713ce) )
	ROM_LOAD( "1783b.prom-u5",  0x2800, 0x0800, CRC(e0ea08c5) SHA1(1df1acd0132ee32c9cc10f55125feb95d9257706) )
	ROM_LOAD( "1784b.prom-u6",  0x3000, 0x0800, CRC(48976d11) SHA1(3e64a908485d09f2949589f6f0d540627ea20c38) )
	ROM_LOAD( "1785b.prom-u7",  0x3800, 0x0800, CRC(297d33ae) SHA1(af01951b41cc93bb645d4fa7f9e95bbcacd4481a) )
	ROM_LOAD( "1786b.prom-u8",  0x4000, 0x0800, CRC(ef94c8f4) SHA1(a1e9b8210dc647643540643009929424d6b5a0d8) )
	ROM_LOAD( "1787b.prom-u9",  0x4800, 0x0800, CRC(1b62994e) SHA1(9ab8ecac299d1e218e2bac1dd162225ca7c38c47) )
	ROM_LOAD( "1788b.prom-u10", 0x5000, 0x0800, CRC(a2e32d91) SHA1(5c0ca2a8803e5b630d2f0dd9087b9022c8326f5a) )
	ROM_LOAD( "1789b.prom-u11", 0x5800, 0x0800, CRC(08a172dc) SHA1(d6665904c914ebce036a320c329e1d9cb7127063) )
	ROM_LOAD( "1790b.prom-u12", 0x6000, 0x0800, CRC(4e320f9d) SHA1(af39c08f1afb5396932f9dc334ad4c31c080cafe) )
	ROM_LOAD( "1791b.prom-u13", 0x6800, 0x0800, CRC(3b4cba31) SHA1(6141717f6b041996971270bc387eab3092d0928c) )
	ROM_LOAD( "1792b.prom-u14", 0x7000, 0x0800, CRC(7707b9f8) SHA1(0084c073fbbc453a07a32a6e51b8695a123b5235) )
	ROM_LOAD( "1793b.prom-u15", 0x7800, 0x0800, CRC(a5d05155) SHA1(254012db05aeb617b590f67fa18675fa3a9dcb92) )
	ROM_LOAD( "1794b.prom-u16", 0x8000, 0x0800, CRC(e4813da9) SHA1(1bfd1679ad77e2e539549811b343472890bde09a) )
	ROM_LOAD( "1795b.prom-u17", 0x8800, 0x0800, CRC(4cd6ed88) SHA1(51876f5f95c2e67a8b42b19f946ccf2b3bc391e3) )
	ROM_LOAD( "1796b.prom-u18", 0x9000, 0x0800, CRC(9f141a42) SHA1(278e5902ed2fbb59c24228d2c6c32407a6717757) )
	ROM_LOAD( "1797b.prom-u19", 0x9800, 0x0800, CRC(ec14ad16) SHA1(7d828a6917d5c50b9c3c943271668dfd6212b366) )
	ROM_LOAD( "1798b.prom-u20", 0xa000, 0x0800, CRC(86743a4f) SHA1(33d6b6a24b47bc2090636f8e89eff997eb35501d) )
	ROM_LOAD( "1799b.prom-u21", 0xa800, 0x0800, CRC(41198a83) SHA1(8432fc921ab174c767c594fca1211cb20c0efd55) )
	ROM_LOAD( "1800b.prom-u22", 0xb000, 0x0800, CRC(6a062a04) SHA1(cae125f5c0867898f2c0a159026da69ff5a2897f) )
	ROM_LOAD( "1801b.prom-u23", 0xb800, 0x0800, CRC(f38488fe) SHA1(dd0f2c655970e8755f9ca1898313ff5fd9f11563) )

	ROM_REGION( 0x400, "audiocpu", 0 )
	ROM_LOAD( "7751.bin",       0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "1516.bg-u13",    0x0000, 0x2000, CRC(e93a2281) SHA1(61c9022edfb8fee2b7214d87d6bbed415fba9601) )
	ROM_LOAD( "1517.bg-u8",     0x2000, 0x2000, CRC(1e589101) SHA1(6805644e18e5b18b96e6a407ec217f02c8931ec2) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "1518a.bg-u22",   0x0000, 0x2000, CRC(2d5932fe) SHA1(a9ca239a062e047b307cf3d0740cb6492a55abb4) )

	ROM_REGION( 0x2000, "n7751", 0 )
	ROM_LOAD( "1543snd.bin",    0x0000, 0x1000, CRC(b525ce8f) SHA1(61e541061a0a579101e52ffa2431540010b9df3e) ) /* U19 */
	ROM_LOAD( "1544snd.bin",    0x1000, 0x1000, CRC(56c79fb0) SHA1(26de83efcc97318220603f83acf4387f6d70d806) ) /* U23 */

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "pr1512.u31",     0x0000, 0x0020, CRC(414ebe9b) SHA1(3df8694e3d26635d19fd4cdf02bd0998e8538b5b) )  /* U31 */
ROM_END

ROM_START( monsterb2 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-1548.2",   0x0000, 0x2000, CRC(239f77c1) SHA1(2945e4b135c1c46bf3e0d947b3d9be052f12e8d8) )
	ROM_LOAD( "epr-1549.3",   0x2000, 0x2000, CRC(40aeb223) SHA1(8e0cc1b53ded819673719ffe1fd69feb1ca6fa29) )
	ROM_LOAD( "epr-1550.13",  0x4000, 0x2000, CRC(b42bb2b3) SHA1(88bd5b027c46cde9f89e90f50ae2c381681ae483) )
	ROM_LOAD( "epr-1551.14",  0x6000, 0x2000, CRC(ad7728cc) SHA1(e9ca8a92dae39528ae7a003cb641af4342067b14) )
	ROM_LOAD( "epr-1552.22",  0x8000, 0x2000, CRC(e876e216) SHA1(31301f2b576689aefcb42a4233f8fafb7f4791a7) )
	ROM_LOAD( "epr-1553.23",  0xa000, 0x2000, CRC(4a839fb2) SHA1(3a15d74a0abd0548cb90c13f4d5baebe3ec83d23) )

	ROM_REGION( 0x400, "audiocpu", 0 )
	ROM_LOAD( "7751.34",      0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "epr-1661.19",  0x0000, 0x2000, CRC(e93a2281) SHA1(61c9022edfb8fee2b7214d87d6bbed415fba9601) )
	ROM_LOAD( "epr-1662.5",   0x2000, 0x2000, CRC(1e589101) SHA1(6805644e18e5b18b96e6a407ec217f02c8931ec2) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "epr-1554.58",  0x0000, 0x2000, CRC(a87937d0) SHA1(cfc2fca52bd74beb2f20ece07e9dd3e3f1038f7c) )

	ROM_REGION( 0x2000, "n7751", 0 )
	ROM_LOAD( "epr-1543.19",  0x0000, 0x1000, CRC(b525ce8f) SHA1(61e541061a0a579101e52ffa2431540010b9df3e) )
	ROM_LOAD( "epr-1544.23",  0x1000, 0x1000, CRC(56c79fb0) SHA1(26de83efcc97318220603f83acf4387f6d70d806) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "pr-1542.31",   0x0000, 0x0020, CRC(414ebe9b) SHA1(3df8694e3d26635d19fd4cdf02bd0998e8538b5b) )

	ROM_REGION( 0x2000, "user2", 0 )              /* other proms (unused) */
	ROM_LOAD( "pr-1535.118",  0x0000, 0x0020, CRC(087df496) SHA1(b6905626595f7a5587a0fd5db0d0bbf7f1fdf695) )
	ROM_LOAD( "pr-1536.128",  0x0000, 0x0020, CRC(57c65534) SHA1(5714720ddb3c90f10fd880faa9c18990c7947a0d) )
	ROM_LOAD( "pr-1537.156",  0x0000, 0x0020, CRC(e4451c6c) SHA1(8a4290fccca37564db3a4415057602c7f530947f) )
	ROM_LOAD( "pr-1538.55",   0x0000, 0x0100, CRC(025996b1) SHA1(16e927c3a94c46ab2d870a37aa0dfacb4f95bdbf) )
	ROM_LOAD( "pr-1539.135",  0x0000, 0x0100, CRC(dd18a9ab) SHA1(365e2f36e60c54f2d782b0c918350f6b565aeda8) )
	ROM_LOAD( "pr1540.36",    0x0000, 0x0100, CRC(e767ab01) SHA1(97a1f891f95a2f862ee1319033411d51c47bd593) )
	ROM_LOAD( "pr1541.145",   0x0000, 0x0100, CRC(411aa2a5) SHA1(bc6a7119679aaa22f171a9038f49265e8cd4a166) )
	ROM_LOAD( "pr-5021.39",   0x0000, 0x0020, CRC(5222c542) SHA1(0745570ca38cf81e28a0fd6221ee26f559ada6b4) )
ROM_END


ROM_START( pignewt )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "cpu.u25",        0x0000, 0x0800, BAD_DUMP CRC(eccc814d) SHA1(d999d7d433bde5d7773cd7afaf3e673089ba2544))
	ROM_LOAD( "1888c.prom-u1",  0x0800, 0x0800, CRC(fd18ed09) SHA1(8bba49d93ae72dbc0497a5a24991c5da26d169d3) )
	ROM_LOAD( "1889c.prom-u2",  0x1000, 0x0800, CRC(f633f5ff) SHA1(b647bebfd8a2093b0b0b7587f7c816aade796b26) )
	ROM_LOAD( "1890c.prom-u3",  0x1800, 0x0800, CRC(22009d7f) SHA1(2c90460ecf8d9fd9fab4a4e6e78ec634ad5f84ef) )
	ROM_LOAD( "1891c.prom-u4",  0x2000, 0x0800, CRC(1540a7d6) SHA1(666d7f0668d585927d4fa56015f843749973da1e) )
	ROM_LOAD( "1892c.prom-u5",  0x2800, 0x0800, CRC(960385d0) SHA1(e8a2f9fd8c9d68cd33af2457d8ba0f34a084c8bf) )
	ROM_LOAD( "1893c.prom-u6",  0x3000, 0x0800, CRC(58c5c461) SHA1(a0748d15feaac39fa67edf4c6be5919c2a3bd0ba) )
	ROM_LOAD( "1894c.prom-u7",  0x3800, 0x0800, CRC(5817a59d) SHA1(ca8494620c02d85fb0d5edf0291ffdef4f8a3ba6) )
	ROM_LOAD( "1895c.prom-u8",  0x4000, 0x0800, CRC(812f67d7) SHA1(f45c0ea56a6393f3efcc6bcaab84c546a105a9b2) )
	ROM_LOAD( "1896c.prom-u9",  0x4800, 0x0800, CRC(cc0ecdd0) SHA1(12126d1d1f3662a5b7a3bc5cdef6bb9450b703f2) )
	ROM_LOAD( "1897c.prom-u10", 0x5000, 0x0800, CRC(7820e93b) SHA1(12a8b4bb88a74b1fe6f63e114558c0793f3f1730) )
	ROM_LOAD( "1898c.prom-u11", 0x5800, 0x0800, CRC(e9a10ded) SHA1(67c636efe4dba8d51af489e2790852824597d7a4) )
	ROM_LOAD( "1899c.prom-u12", 0x6000, 0x0800, CRC(d7ddf02b) SHA1(728e89e574bdec85bd8ef5a9a55c184b756e35a4) )
	ROM_LOAD( "1900c.prom-u13", 0x6800, 0x0800, CRC(8deff4e5) SHA1(24f8a8779d8ef0a185250b324c7bbfc92cf63167) )
	ROM_LOAD( "1901c.prom-u14", 0x7000, 0x0800, CRC(46051305) SHA1(14ebcaec8a1bef6f2e309fe6363127da1b61922a) )
	ROM_LOAD( "1902c.prom-u15", 0x7800, 0x0800, CRC(cb937e19) SHA1(4a1755b69249bc6e12a05094d991722e42fb59f8) )
	ROM_LOAD( "1903c.prom-u16", 0x8000, 0x0800, CRC(53239f12) SHA1(e5d2dfd683fcf350c3734f82069393d0468280a3) )
	ROM_LOAD( "1913c.prom-u17", 0x8800, 0x0800, CRC(4652cb0c) SHA1(06bc30120cc7b25943fb70ccd8f6075b29c86a14) )
	ROM_LOAD( "1914c.prom-u18", 0x9000, 0x0800, CRC(cb758697) SHA1(fb18b0e94d3c6495f499cef9eb1b1a9db4a216f1) )
	ROM_LOAD( "1915c.prom-u19", 0x9800, 0x0800, CRC(9f3bad66) SHA1(985c3afce9be7afd6300d0894a6e3a3f0f0a558c) )
	ROM_LOAD( "1916c.prom-u20", 0xa000, 0x0800, CRC(5bb6f61e) SHA1(6d2f1540a3f4dc806b2a87e16a438332cdd51b00) )
	ROM_LOAD( "1917c.prom-u21", 0xa800, 0x0800, CRC(725e2c87) SHA1(3ea6ca8576a239236b97c3b529d2c226b3f0d28f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "1904c.bg",       0x0000, 0x2000, CRC(e9de2c8b) SHA1(a8957585911422e07a17ec67430b30a24a6ed16c) )
	ROM_LOAD( "1905c.bg",       0x2000, 0x2000, CRC(af7cfe0b) SHA1(e8a64564596d7e6e6bce00546379bd01a5b9b3d9) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "1906c.bg",       0x0000, 0x1000, CRC(c79d33ce) SHA1(8a5332a801d0db6e5f33c0d39d165819f9914e65) )
	ROM_LOAD( "1907c.bg",       0x1000, 0x1000, CRC(bc839d3c) SHA1(80b1c96cac7c51e49ca40a1c5fbc156b12529d2f) )
	ROM_LOAD( "1908c.bg",       0x2000, 0x1000, CRC(92cb14da) SHA1(257db7bb2758d579bcf171cda410acff1877122c) )
ROM_END

ROM_START( pignewta )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "cpu.u25",       0x0000, 0x0800, BAD_DUMP CRC(eccc814d) SHA1(d999d7d433bde5d7773cd7afaf3e673089ba2544))
	ROM_LOAD( "1888a.prom-u1", 0x0800, 0x0800, CRC(491c0835) SHA1(65c917ebcfa8e5199e9923c04626c067fda3c637) )
	ROM_LOAD( "1889a.prom-u2", 0x1000, 0x0800, CRC(0dcf0af2) SHA1(788c24c87f44f9206c994286c7ba093d365f056f) )
	ROM_LOAD( "1890a.prom-u3", 0x1800, 0x0800, CRC(640b8b2e) SHA1(838c3283ad92eb4390e9935e420322c4b0426800) )
	ROM_LOAD( "1891a.prom-u4", 0x2000, 0x0800, CRC(7b8aa07f) SHA1(6d6c72ef7fd8765f1b9861928e438b854cb409cb) )
	ROM_LOAD( "1892a.prom-u5", 0x2800, 0x0800, CRC(afc545cb) SHA1(f6f5aa654fc7eb99fe726e24c2340dee013afe17) )
	ROM_LOAD( "1893a.prom-u6", 0x3000, 0x0800, CRC(82448619) SHA1(1206880cf8c3a9f0660a0e2af6b81d369f6a20be) )
	ROM_LOAD( "1894a.prom-u7", 0x3800, 0x0800, CRC(4302dbfb) SHA1(acd1d2be5cb3e92ed2cbb7fb3fdd9ac6647ad21b) )
	ROM_LOAD( "1895a.prom-u8", 0x4000, 0x0800, CRC(137ebaaf) SHA1(d1ac66e3cc575d8bb4102acf1a2f6a5bc1c1b96f) )
	ROM_LOAD( "1896.prom-u9",  0x4800, 0x0800, CRC(1604c811) SHA1(93d14eb19bcbe446e06c178c39d73d24edf664a4) )
	ROM_LOAD( "1897.prom-u10", 0x5000, 0x0800, CRC(3abee406) SHA1(3f48b7951364f0665e2e386e91634b10f7adbdc9) )
	ROM_LOAD( "1898.prom-u11", 0x5800, 0x0800, CRC(a96410dc) SHA1(418801b60f3f3b57a501bb48c7aec2b71cc50e60) )
	ROM_LOAD( "1899.prom-u12", 0x6000, 0x0800, CRC(612568a5) SHA1(ba835783b2279692ecd043288474a34646f0a23e) )
	ROM_LOAD( "1900.prom-u13", 0x6800, 0x0800, CRC(5b231cea) SHA1(76e3bed0707a8e7348ed4eef29fe39379f916952) )
	ROM_LOAD( "1901.prom-u14", 0x7000, 0x0800, CRC(3fd74b05) SHA1(5848f821a9e0ac5f169a810fd482f17441704423) )
	ROM_LOAD( "1902.prom-u15", 0x7800, 0x0800, CRC(d568fc22) SHA1(60e3f041911ce644b2c475593305f505240e83b7) )
	ROM_LOAD( "1903.prom-u16", 0x8000, 0x0800, CRC(7d16633b) SHA1(daa1af79b71c3354482e8fb89104a9fdbc88c5d2) )
	ROM_LOAD( "1913.prom-u17", 0x8800, 0x0800, CRC(fa4be04f) SHA1(d8d5dbe74abc5b0457d16f6a173de1fe4fdafbc4) )
	ROM_LOAD( "1914.prom-u18", 0x9000, 0x0800, CRC(08253c50) SHA1(024a9d43d03b1db5e51ebd53d30a6f08c5a8c144) )
	ROM_LOAD( "1915.prom-u19", 0x9800, 0x0800, CRC(de786c3b) SHA1(444edc18bc6e194bbe15abd72fc73f0f8907dcc4) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "1904a.bg",      0x0000, 0x2000, BAD_DUMP CRC(e9de2c8b) SHA1(a8957585911422e07a17ec67430b30a24a6ed16c) )
	ROM_LOAD( "1905a.bg",      0x2000, 0x2000, BAD_DUMP CRC(af7cfe0b) SHA1(e8a64564596d7e6e6bce00546379bd01a5b9b3d9) )

	ROM_REGION( 0x4000, "gfx2", 0 )           /* background charmaps */
	/* NOTE: No background ROMs for set A have been dumped, so the
	ROMs from set C have been copied and renamed. This is to
	provide a reminder that these ROMs still need to be dumped. */
	ROM_LOAD( "1906a.bg",  0x0000, 0x1000, BAD_DUMP CRC(c79d33ce) SHA1(8a5332a801d0db6e5f33c0d39d165819f9914e65)  ) /* ??? */
	ROM_LOAD( "1907a.bg",  0x1000, 0x1000, BAD_DUMP CRC(bc839d3c) SHA1(80b1c96cac7c51e49ca40a1c5fbc156b12529d2f)  ) /* ??? */
	ROM_LOAD( "1908a.bg",  0x2000, 0x1000, BAD_DUMP CRC(92cb14da) SHA1(257db7bb2758d579bcf171cda410acff1877122c)  ) /* ??? */
ROM_END


ROM_START( sindbadm )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5393.4d", 0x0000, 0x2000, CRC(51f2e51e) SHA1(0fd96863d0dfaa0bab09be6fea1e7d12b9c40d68) )
	ROM_LOAD( "epr-5394.4e", 0x2000, 0x2000, CRC(d39ce2ee) SHA1(376065a40caa499da99e556098a03387edca5883) )
	ROM_LOAD( "epr-5395.4h", 0x4000, 0x2000, CRC(b1d15c82) SHA1(04f888faee0103cce8c0cd57296d75474b770632) )
	ROM_LOAD( "epr-5396.4j", 0x6000, 0x2000, CRC(ea9d40bf) SHA1(f61ec6e405ba3aa4166a5d6127d9e7bc940f19df) )
	ROM_LOAD( "epr-5397.4l", 0x8000, 0x2000, CRC(595d16dc) SHA1(ea8aa068fc45bb2ee64c4290fad4b8b51e1abe97) )
	ROM_LOAD( "epr-5398.4m", 0xa000, 0x2000, CRC(e57ff63c) SHA1(eac36221cb210743d3c04e51da5956623a28dbdb) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "epr-5400.4a", 0x0000, 0x2000, CRC(5114f18e) SHA1(343f96c728f96df5d50a9888fc87488d9440d7f4) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "epr-5428.9m", 0x0000, 0x2000, CRC(f6044a1e) SHA1(19622aa0991553604236a1ff64a3e5dd1d881ed8) )
	ROM_LOAD( "epr-5429.9p", 0x2000, 0x2000, CRC(b23eca10) SHA1(e00ab3b50b52e16d7281ece42d73603fb188c9b3) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "epr-5424.9e", 0x0000, 0x2000, CRC(4bfc2e95) SHA1(7d513df944d5768b14983f44a1e3c76930a55e9a) )
	ROM_LOAD( "epr-5425.9h", 0x2000, 0x2000, CRC(b654841a) SHA1(9b224fbe5f4c7bbb486a3d15550cc10e4f317631) )
	ROM_LOAD( "epr-5426.9j", 0x4000, 0x2000, CRC(9de0da28) SHA1(79e01005861e2426a8112544b1bc6d1c6a9ce936) )
	ROM_LOAD( "epr-5427.9l", 0x6000, 0x2000, CRC(a94f4d41) SHA1(fe4f412ea3680c0e5a6242827eab9e82a841d7c7) )
ROM_END



/*************************************
 *
 *  Driver init helpers
 *
 *************************************/

void segag80r_state::monsterb_expand_gfx(const char *region)
{
	UINT8 *dest;
	int i;

	/* expand the background ROMs; A11/A12 of each ROM is independently controlled via */
	/* banking */
	dest = memregion(region)->base();
	dynamic_buffer temp(0x4000);
	memcpy(&temp[0], dest, 0x4000);

	/* 16 effective total banks */
	for (i = 0; i < 16; i++)
	{
		memcpy(&dest[0x0000 + i * 0x800], &temp[0x0000 + (i & 3) * 0x800], 0x800);
		memcpy(&dest[0x8000 + i * 0x800], &temp[0x2000 + (i >> 2) * 0x800], 0x800);
	}
}



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(segag80r_state,astrob)
{
	address_space &iospace = m_maincpu->space(AS_IO);

	/* configure the 315-0062 security chip */
	m_decrypt = segag80_security(62);

	/* configure video */
	m_background_pcb = G80_BACKGROUND_NONE;

	/* install speech board */
	iospace.install_write_handler(0x38, 0x38, write8_delegate(FUNC(speech_sound_device::data_w), (speech_sound_device*)m_speech));
	iospace.install_write_handler(0x3b, 0x3b, write8_delegate(FUNC(speech_sound_device::control_w), (speech_sound_device*)m_speech));

	/* install Astro Blaster sound board */
	iospace.install_write_handler(0x3e, 0x3f, write8_delegate(FUNC(segag80r_state::astrob_sound_w),this));

	save_item(NAME(m_sound_state));
	save_item(NAME(m_sound_rate));
}


DRIVER_INIT_MEMBER(segag80r_state,005)
{
	/* configure the 315-0070 security chip */
	m_decrypt = segag80_security(70);

	/* configure video */
	m_background_pcb = G80_BACKGROUND_NONE;

	save_item(NAME(m_sound_state));
	save_item(NAME(m_sound_addr));
	save_item(NAME(m_sound_data));
	save_item(NAME(m_square_state));
	save_item(NAME(m_square_count));
}


DRIVER_INIT_MEMBER(segag80r_state,spaceod)
{
	address_space &iospace = m_maincpu->space(AS_IO);

	/* configure the 315-0063 security chip */
	m_decrypt = segag80_security(63);

	/* configure video */
	m_background_pcb = G80_BACKGROUND_SPACEOD;

	/* configure ports for the background board */
	iospace.install_readwrite_handler(0x08, 0x0f, read8_delegate(FUNC(segag80r_state::spaceod_back_port_r),this), write8_delegate(FUNC(segag80r_state::spaceod_back_port_w),this));

	/* install Space Odyssey sound board */
	iospace.install_write_handler(0x0e, 0x0f, write8_delegate(FUNC(segag80r_state::spaceod_sound_w),this));

	/* install our wacky mangled ports */
	iospace.install_read_handler(0xf8, 0xfb, read8_delegate(FUNC(segag80r_state::spaceod_mangled_ports_r),this));
	iospace.install_read_handler(0xfc, 0xfc, read8_delegate(FUNC(segag80r_state::spaceod_port_fc_r),this));

	save_item(NAME(m_sound_state));
}


DRIVER_INIT_MEMBER(segag80r_state,monsterb)
{
	address_space &iospace = m_maincpu->space(AS_IO);
	address_space &pgmspace = m_maincpu->space(AS_PROGRAM);

	/* configure the 315-0082 security chip */
	m_decrypt = segag80_security(82);

	/* configure video */
	m_background_pcb = G80_BACKGROUND_MONSTERB;
	monsterb_expand_gfx("gfx1");

	/* install background board handlers */
	iospace.install_write_handler(0xb8, 0xbd, write8_delegate(FUNC(segag80r_state::monsterb_back_port_w),this));
	pgmspace.install_write_handler(0xe000, 0xffff, write8_delegate(FUNC(segag80r_state::monsterb_vidram_w),this));

	save_item(NAME(m_sound_state));
	save_item(NAME(m_sound_addr));
	save_item(NAME(m_n7751_command));
	save_item(NAME(m_n7751_busy));
}


DRIVER_INIT_MEMBER(segag80r_state,monster2)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x88,0x08,0x80,0x00 }, { 0x00,0x08,0x20,0x28 },   /* ...0...0...0...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x28,0xa8,0x08,0x88 },   /* ...0...0...0...1 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...0...1...0 */
		{ 0x88,0x08,0x80,0x00 }, { 0x88,0x08,0x80,0x00 },   /* ...0...0...1...1 */
		{ 0x00,0x08,0x20,0x28 }, { 0x88,0x08,0x80,0x00 },   /* ...0...1...0...0 */
		{ 0xa0,0x80,0x20,0x00 }, { 0x80,0x88,0x00,0x08 },   /* ...0...1...0...1 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0x20,0x00 },   /* ...0...1...1...0 */
		{ 0x88,0x08,0x80,0x00 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...1...1...1 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x80,0x88,0x00,0x08 },   /* ...1...0...0...0 */
		{ 0x80,0x88,0x00,0x08 }, { 0x00,0x08,0x20,0x28 },   /* ...1...0...0...1 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0x28,0xa8,0x08,0x88 },   /* ...1...0...1...0 */
		{ 0x00,0x08,0x20,0x28 }, { 0x80,0xa0,0x88,0xa8 },   /* ...1...0...1...1 */
		{ 0x80,0x88,0x00,0x08 }, { 0xa0,0x80,0x20,0x00 },   /* ...1...1...0...0 */
		{ 0x80,0xa0,0x88,0xa8 }, { 0xa0,0x80,0x20,0x00 },   /* ...1...1...0...1 */
		{ 0xa0,0x80,0x20,0x00 }, { 0x80,0xa0,0x88,0xa8 },   /* ...1...1...1...0 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0x00,0x08,0x20,0x28 }    /* ...1...1...1...1 */
	};

	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x8000, convtable);

	address_space &iospace = m_maincpu->space(AS_IO);
	address_space &pgmspace = m_maincpu->space(AS_PROGRAM);

	/* configure the 315-5006 security chip */
	m_decrypt = segag80_security(0);

	/* configure video */
	m_background_pcb = G80_BACKGROUND_PIGNEWT;
	monsterb_expand_gfx("gfx1");

	/* install background board handlers */
	iospace.install_write_handler(0xb4, 0xb5, write8_delegate(FUNC(segag80r_state::pignewt_back_color_w),this));
	iospace.install_write_handler(0xb8, 0xbd, write8_delegate(FUNC(segag80r_state::pignewt_back_port_w),this));
	pgmspace.install_write_handler(0xe000, 0xffff, write8_delegate(FUNC(segag80r_state::pignewt_vidram_w),this));

	save_item(NAME(m_sound_state));
	save_item(NAME(m_sound_addr));
	save_item(NAME(m_n7751_command));
	save_item(NAME(m_n7751_busy));
}


DRIVER_INIT_MEMBER(segag80r_state,pignewt)
{
	address_space &iospace = m_maincpu->space(AS_IO);
	address_space &pgmspace = m_maincpu->space(AS_PROGRAM);

	/* configure the 315-0063? security chip */
	m_decrypt = segag80_security(63);

	/* configure video */
	m_background_pcb = G80_BACKGROUND_PIGNEWT;
	monsterb_expand_gfx("gfx1");

	/* install background board handlers */
	iospace.install_write_handler(0xb4, 0xb5, write8_delegate(FUNC(segag80r_state::pignewt_back_color_w),this));
	iospace.install_write_handler(0xb8, 0xbd, write8_delegate(FUNC(segag80r_state::pignewt_back_port_w),this));
	pgmspace.install_write_handler(0xe000, 0xffff, write8_delegate(FUNC(segag80r_state::pignewt_vidram_w),this));

	/* install Universal sound board */
	iospace.install_readwrite_handler(0x3f, 0x3f, read8_delegate(FUNC(usb_sound_device::status_r), (usb_sound_device*)m_usbsnd), write8_delegate(FUNC(usb_sound_device::data_w), (usb_sound_device*)m_usbsnd));
	pgmspace.install_read_handler(0xd000, 0xdfff, read8_delegate(FUNC(usb_sound_device::ram_r), (usb_sound_device*)m_usbsnd));
	pgmspace.install_write_handler(0xd000, 0xdfff, write8_delegate(FUNC(segag80r_state::usb_ram_w),this));
}


DRIVER_INIT_MEMBER(segag80r_state,sindbadm)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...0...0...0...0 */
		{ 0xa8,0xa0,0x88,0x80 }, { 0x00,0x20,0x80,0xa0 },   /* ...0...0...0...1 */
		{ 0xa8,0xa0,0x88,0x80 }, { 0x00,0x20,0x80,0xa0 },   /* ...0...0...1...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...0...0...1...1 */
		{ 0xa8,0x88,0xa0,0x80 }, { 0xa0,0x20,0xa8,0x28 },   /* ...0...1...0...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...0...1...0...1 */
		{ 0xa8,0xa0,0x88,0x80 }, { 0x00,0x20,0x80,0xa0 },   /* ...0...1...1...0 */
		{ 0xa8,0xa0,0x88,0x80 }, { 0x00,0x20,0x80,0xa0 },   /* ...0...1...1...1 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...1...0...0...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...1...0...0...1 */
		{ 0xa8,0xa0,0x88,0x80 }, { 0x00,0x20,0x80,0xa0 },   /* ...1...0...1...0 */
		{ 0xa8,0xa0,0x88,0x80 }, { 0x00,0x20,0x80,0xa0 },   /* ...1...0...1...1 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...1...1...0...0 */
		{ 0xa8,0x88,0xa0,0x80 }, { 0xa0,0x20,0xa8,0x28 },   /* ...1...1...0...1 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...1...1...1...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 }    /* ...1...1...1...1 */
	};

	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x8000, convtable);

	address_space &iospace = m_maincpu->space(AS_IO);
	address_space &pgmspace = m_maincpu->space(AS_PROGRAM);

	/* configure the encrypted Z80 */
	m_decrypt = segag80_security(0);

	/* configure video */
	m_background_pcb = G80_BACKGROUND_SINDBADM;

	/* install background board handlers */
	iospace.install_write_handler(0x40, 0x41, write8_delegate(FUNC(segag80r_state::sindbadm_back_port_w),this));
	pgmspace.install_write_handler(0xe000, 0xffff, write8_delegate(FUNC(segag80r_state::sindbadm_vidram_w),this));
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR, NAME,      PARENT,   MACHINE,  INPUT,    INIT,     MONITOR,COMPANY,FULLNAME,FLAGS
/* basic G-80 system with: CPU board, PROM board, Video I board, custom sound boards */
GAME( 1981, astrob,    0,        astrob,   astrob, segag80r_state,   astrob,   ROT270, "Sega", "Astro Blaster (version 3)", GAME_IMPERFECT_SOUND )
GAME( 1981, astrob2,   astrob,   astrob,   astrob2, segag80r_state,  astrob,   ROT270, "Sega", "Astro Blaster (version 2)", GAME_IMPERFECT_SOUND )
GAME( 1981, astrob2a,  astrob,   astrob,   astrob2, segag80r_state,  astrob,   ROT270, "Sega", "Astro Blaster (version 2a)", GAME_IMPERFECT_SOUND )
GAME( 1981, astrob1,   astrob,   astrob,   astrob, segag80r_state,   astrob,   ROT270, "Sega", "Astro Blaster (version 1)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) // instant death if you start game with 1 credit, protection?, bad dump?
GAME( 1981, astrobg,   astrob,   astrob,   astrob, segag80r_state,   astrob,   ROT270, "Sega", "Astro Blaster (German)", GAME_IMPERFECT_SOUND )
GAME( 1981, 005,       0,        005,      005, segag80r_state,      005,      ROT270, "Sega", "005", GAME_IMPERFECT_SOUND )


/* basic G-80 system with individual background boards */
GAME( 1981, spaceod,   0,        spaceod,  spaceod, segag80r_state,  spaceod,  ROT270, "Sega", "Space Odyssey (version 2)", GAME_IMPERFECT_SOUND )
GAME( 1981, spaceod2,  spaceod,  spaceod,  spaceod, segag80r_state,  spaceod,  ROT270, "Sega", "Space Odyssey (version 1)", GAME_IMPERFECT_SOUND )
GAME( 1982, monsterb,  0,        monsterb, monsterb, segag80r_state, monsterb, ROT270, "Sega", "Monster Bash", GAME_IMPERFECT_SOUND )

/* 2-board G-80 system */
GAME( 1982, monsterb2, monsterb, monster2, monsterb, segag80r_state, monster2, ROT270, "Sega", "Monster Bash (2 board version)", GAME_IMPERFECT_SOUND )
GAME( 1983, pignewt,   0,        pignewt,  pignewt, segag80r_state,  pignewt,  ROT270, "Sega", "Pig Newton (version C)", GAME_IMPERFECT_SOUND )
GAME( 1983, pignewta,  pignewt,  pignewt,  pignewta, segag80r_state, pignewt,  ROT270, "Sega", "Pig Newton (version A)", GAME_IMPERFECT_SOUND )
GAME( 1983, sindbadm,  0,        sindbadm, sindbadm, segag80r_state, sindbadm, ROT270, "Sega", "Sindbad Mystery", 0 )
