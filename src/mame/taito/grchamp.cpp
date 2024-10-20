// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Phil Stroffolino
/***************************************************************************

    Grand Champion
    (C) 1981 Taito

    MAME driver by Ernesto Corvi and Phil Stroffolino

****************************************************************************

    Known Issues:

    -   pc3259 handling (for collision detection) is not accurate.
        Collision detection does work, but there are bits which define
        the type of collision (and determine whether a pit stop is
        required) that I don't know how to handle.

    -   sound: missing engine noise

    -   "radar" is probably wrong

    -   LED and tachometer display are missing/faked
        Note that a dipswitch setting allows score to be displayed
        onscreen, but there's no equivalent for tachometer.

    -   TODO: the two sound callbacks could, under unusual circumstances,
        conflict with one another, causing the newly written value to have
        its bit 7 cleared before the sound cpu reads it.
        The proper way to handle this is to use a single callback which
        processes a timestamped event queue.

    -   TODO: merge the sound system with taitosj.cpp, as the sound system
        is almost completely identical. Also import the taitosj.cpp dac/volume
        system, as the system used in audio/grchamp.cpp for dac output is
        incorrect.

    Notes:

    -   The object of the game is to avoid the opposing cars.

    -   The player has to drive through Dark Tunnels, Rain,
        Lightning, Sleet, Snow, and a Track that suddenly
        divides to get to the Finish Line first.

    -   "GRAND CHAMPION" has a Radar Feature which enables
        the Player to see his position relative to the other
        cars.

    -   A Rank Feature is also provided which shows the
        Player's numerical rank all through the race.

    -   The Speech Feature enhances the game play.

    -   Schematics: https://archive.org/download/ArcadeGameManualGrandchampion/grandchampion.pdf

***************************************************************************/

#include "emu.h"
#include "grchamp.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "grchamp.lh"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK        (18432000)
#define SOUND_CLOCK         (6000000)

#define PIXEL_CLOCK         (MASTER_CLOCK/3)

#define HTOTAL              (384)
#define HBEND               (0)
#define HBSTART             (256)

#define VTOTAL              (264)
#define VBEND               (16)
#define VBSTART             (240)



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void grchamp_state::machine_start()
{
	m_digits.resolve();
	m_led0.resolve();
	m_soundlatch_data = 0x00;
	m_soundlatch_flag = false;
	save_item(NAME(m_cpu0_out));
	save_item(NAME(m_cpu1_out));
	save_item(NAME(m_comm_latch));
	save_item(NAME(m_comm_latch2));
	save_item(NAME(m_ledlatch));
	save_item(NAME(m_ledaddr));
	save_item(NAME(m_ledram));
	save_item(NAME(m_collide));
	save_item(NAME(m_collmode));
}

	void grchamp_state::machine_reset()
{
	m_soundnmi->in_w<0>(0); // disable sound nmi
	/* if the coin system is 1 way, lock Coin B (Page 40) */
	machine().bookkeeping().coin_lockout_w(1, (ioport("DSWB")->read() & 0x10) ? 1 : 0);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

INTERRUPT_GEN_MEMBER(grchamp_state::cpu0_interrupt)
{
	if (m_cpu0_out[0] & 0x01)
		device.execute().set_input_line(0, ASSERT_LINE);
}


INTERRUPT_GEN_MEMBER(grchamp_state::cpu1_interrupt)
{
	if (m_cpu1_out[4] & 0x01)
		device.execute().set_input_line(0, ASSERT_LINE);
}



/*************************************
 *
 *  Main CPU outputs
 *
 *************************************/

void grchamp_state::cpu0_outputs_w(offs_t offset, uint8_t data)
{
	uint8_t diff = data ^ m_cpu0_out[offset];
	m_cpu0_out[offset] = data;

	switch (offset)
	{
		case 0x00:  /* OUT0 */

			/* bit 0: low = clear IRQ on main CPU */
			/* bit 1: /HTCLR = clear collision detect */
			/* bit 4: HEAD LAMP */
			/* bit 5: CHANGE */
			/* bit 6: FOG OUT */
			/* bit 7: RADARON */
			if ((diff & 0x01) && !(data & 0x01))
				m_maincpu->set_input_line(0, CLEAR_LINE);
			if ((diff & 0x02) && !(data & 0x02))
				m_collide = m_collmode = 0;
			break;

		case 0x01:  /* OUT1 */
			/* connects to pc3259, pin 23 (read collision data?) */
			m_collmode++;
			break;

		case 0x02:  /* OUT2 */
			/* bit 0-7: MYDH (car X position) */
			break;

		case 0x03:  /* OUT3 */
			/* bit 0-7: MYDV (car Y position) */
			break;

		case 0x04:  /* OUT4 */
			/* bit 0-3: player car tile select */
			/* bit 4-7: rain tile select */
			break;

		case 0x07:  /* OUT7 */
			/* bit 0-7: rain Y position */
			break;

		case 0x08:  /* OUT8 */
			/* bit 0-7: rain X position */
			break;

		case 0x09:  /* OUT9 */
			/* bit 0-3: n/c */
			/* bit 4:   coin lockout */
			/* bit 5:   Game Over lamp */
			/* bit 6-7: n/c */
			machine().bookkeeping().coin_lockout_global_w((data >> 4) & 1);
			m_led0 = BIT(~data, 5);
			break;

		case 0x0a:  /* OUT10 */
			/* bit 0: n/c */
			/* bit 1: G-12 */
			/* bit 2: G-S */
			/* bit 3: G-16 */
			/* bit 4: G-Y */
			/* bit 5: n/c */
			/* bit 6: G-Z */
			if (diff)
				osd_printf_debug("OUT10=%02X\n", data);
			break;

		case 0x0d:  /* OUT13 */
			m_watchdog->watchdog_reset();
			break;

		case 0x0e:  /* OUT14 */
			/* O-21 connector */
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(grchamp_state::soundlatch_w_cb), this), data); // soundlatch write, needs to synchronize
			break;
	}
}


void grchamp_state::led_board_w(offs_t offset, uint8_t data)
{
	static const uint8_t ls247_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x58,0x4c,0x62,0x69,0x78,0x00 };

	switch (offset)
	{
		case 0x00:
			m_ledlatch = (m_ledlatch & 0xff00) | (data << 0);
			break;

		case 0x04:
			m_ledlatch = (m_ledlatch & 0x00ff) | (data << 8);
			break;

		case 0x08:
			m_ledaddr = data & 0x0f;
			break;

		case 0x0c:
			m_ledram[m_ledaddr & 0x07] = m_ledlatch;
			m_digits[m_ledaddr & 0x07] = ls247_map[m_ledram[m_ledaddr & 0x07] & 0x0f];
			/*
			    ledram[0] & 0x0f = score LSD
			    ledram[1] & 0x0f = score
			    ledram[2] & 0x0f = score
			    ledram[3] & 0x0f = score
			    ledram[4] & 0x0f = score MSD
			    ledram[5] & 0x0f = time LSD
			    ledram[6] & 0x0f = time MSD
			    ledram[7] & 0x0f = ???
			*/
			break;
	}
}



/*************************************
 *
 *  Sub CPU outputs
 *
 *************************************/

void grchamp_state::cpu1_outputs_w(offs_t offset, uint8_t data)
{
	uint8_t diff = data ^ m_cpu1_out[offset];
	m_cpu1_out[offset] = data;

	switch (offset)
	{
		case 0x00:  /* OUT0 */
			/* bit 0-7: left/right synk bus xscroll LSBs */
			break;

		case 0x01:  /* OUT1 */
			/* bit 0: left/right synk bus xscroll MSB */
			break;

		case 0x02:  /* OUT2 */
			/* bit 0-7: left synk bus yscroll */
			break;

		case 0x03:  /* OUT3 */
			/* bit 0-3: analog tachometer output */
			/* bit 4:   palette MSB */
			/* bit 5:   disable the 256H line in the center tilemap */
			break;

		case 0x04:  /* OUT4 */
			/* bit 0:   interrupt enable for CPU 1 */
			if ((diff & 0x01) && !(data & 0x01))
				m_subcpu->set_input_line(0, CLEAR_LINE);
			break;

		case 0x05:  /* OUT5 - unused */
			break;

		case 0x06:  /* OUT6 - unused */
			break;

		case 0x07:  /* OUT7 */
			/* bit 0-7: right synk bus yscroll */
			break;

		case 0x08:  /* OUT8 */
			/* bit 0-7: latches data to main CPU input port 2 */
			m_comm_latch = data;
			break;

		case 0x09:  /* OUT9 */
			/* bit 0-7: center synk bus xscroll LSBs */
			break;

		case 0x0a:  /* OUTA */
			/* bit 0: center synk bus xscroll MSB */
			break;

		case 0x0b:  /* OUTB */
			/* bit 0-7: center synk bus yscroll */
			break;

		case 0x0c: /* OUTC */
			/* bit 0:   FOG */
			/* bit 1:   IDLING */
			/* bit 2-4: ATTACK UP 1-3 */
			/* bit 5-6: SIFT 1-2 */
			/* bit 7:   ENGINE CS */
			m_discrete->write(GRCHAMP_ENGINE_CS_EN, data & 0x80);
			m_discrete->write(GRCHAMP_SIFT_DATA, (data >> 5) & 0x03);
			m_discrete->write(GRCHAMP_ATTACK_UP_DATA, (data >> 2) & 0x07);
			m_discrete->write(GRCHAMP_IDLING_EN, data & 0x02);
			m_discrete->write(GRCHAMP_FOG_EN, data & 0x01);
			break;

		case 0x0d: /* OUTD */
			/* bit 0-3: ATTACK SPEED 1-4 */
			/* bit 4-7: PLAYER SPEED 1-4 */
			m_discrete->write(GRCHAMP_PLAYER_SPEED_DATA, (data >> 4) & 0x0f);
			m_discrete->write(GRCHAMP_ATTACK_SPEED_DATA,  data & 0x0f);
			break;

		default:
			/* OUTE - Page 48: goes to connector Q-27 */
			/* OUTF - unused */
			break;
	}
}



/*************************************
 *
 *  CPU 0 Inputs
 *
 *************************************/

uint8_t grchamp_state::get_pc3259_bits(int offs)
{
	int bits;

	/* force a partial update to the current position */
	m_screen->update_partial(m_screen->vpos());

	/* get the relevant 4 bits */
	bits = (m_collide >> (offs*4)) & 0x0f;

	/* replicate to both nibbles */
	return bits | (bits << 4);
}


uint8_t grchamp_state::pc3259_0_r()
{
	return get_pc3259_bits(0);
}


uint8_t grchamp_state::pc3259_1_r()
{
	return get_pc3259_bits(1);
}


uint8_t grchamp_state::pc3259_2_r()
{
	return get_pc3259_bits(2);
}


uint8_t grchamp_state::pc3259_3_r()
{
	return get_pc3259_bits(3);
}



/*************************************
 *
 *  Inter-CPU communication
 *
 *************************************/

uint8_t grchamp_state::sub_to_main_comm_r()
{
	return m_comm_latch;
}


TIMER_CALLBACK_MEMBER(grchamp_state::main_to_sub_comm_sync_w)
{
	int offset = param >> 8;
	m_comm_latch2[offset & 3] = param;
}


void grchamp_state::main_to_sub_comm_w(offs_t offset, uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(grchamp_state::main_to_sub_comm_sync_w),this), data | (offset << 8));
}


uint8_t grchamp_state::main_to_sub_comm_r(offs_t offset)
{
	return m_comm_latch2[offset];
}



/*************************************
 *
 *  Sound port handlers
 *
 *************************************/
TIMER_CALLBACK_MEMBER(grchamp_state::soundlatch_w_cb)
{
	if (m_soundlatch_flag && (m_soundlatch_data != param))
		logerror("Warning: soundlatch written before being read. Previous: %02x, new: %02x\n", m_soundlatch_data, param);
	m_soundlatch_data = param;
	m_soundlatch_flag = true;
	m_soundnmi->in_w<1>(1);
}

TIMER_CALLBACK_MEMBER(grchamp_state::soundlatch_clear7_w_cb)
{
	if (m_soundlatch_flag)
		logerror("Warning: soundlatch bit 7 cleared before being read. Previous: %02x, new: %02x\n", m_soundlatch_data, m_soundlatch_data&0x7f);
	m_soundlatch_data &= 0x7F;
}

// RD5000
uint8_t grchamp_state::soundlatch_r()
{
	if (!machine().side_effects_disabled())
	{
		m_soundlatch_flag = false;
		m_soundnmi->in_w<1>(0);
	}
	return m_soundlatch_data;
}

// WR5000
void grchamp_state::soundlatch_clear7_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(grchamp_state::soundlatch_clear7_w_cb), this), data);
}

// RD5001
uint8_t grchamp_state::soundlatch_flags_r()
{
	return (m_soundlatch_flag?8:0) /*| (m_sound_semaphore2?4:0)*/ | 3;
}

void grchamp_state::portA_0_w(uint8_t data)
{
	m_discrete->write(GRCHAMP_A_DATA, data);
}

void grchamp_state::portB_0_w(uint8_t data)
{
	m_discrete->write(GRCHAMP_B_DATA, 255-data);
}

void grchamp_state::portA_2_w(uint8_t data)
{
	/* A0/A1 modify the output of AY8910 #2 with filter capacitors */
	/* A7 contributes to the volume control for the discrete logic dac hanging off of AY8910 #0's i/o ports */
}
void grchamp_state::portB_2_w(uint8_t data)
{
	/* B0 is the sound nmi enable, active low */
	m_soundnmi->in_w<0>((~data)&1);
}


/*************************************
 *
 *  Graphics Layouts
 *
 *************************************/

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2),0 },
	{ STEP8(0,1),STEP8(64,1) },
	{ STEP8(0,8),STEP8(128,8) },
	16*16
};

static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0,4,RGN_FRAC(1,2),RGN_FRAC(1,2)+4 },
	{ STEP4(0,1),STEP4(8*8+0,1) },
	{ STEP8(0,8) },
	16*8
};

static GFXDECODE_START( gfx_grchamp )
	GFXDECODE_ENTRY( "gfx1", 0x0000, gfx_8x8x2_planar,  0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, tile_layout,       0, 2 )
	GFXDECODE_ENTRY( "gfx3", 0x0000, tile_layout,       0, 2 )
	GFXDECODE_ENTRY( "gfx4", 0x0000, tile_layout,       0, 2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, sprite_layout,     0, 8 )
GFXDECODE_END



/*************************************
 *
 *  Address maps
 *
 *************************************/

/* complete memory map derived from schematics */
void grchamp_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).mirror(0x0400).ram();
	map(0x4800, 0x4bff).mirror(0x0400).ram().share("radarram");
	map(0x5000, 0x53ff).mirror(0x0400).ram().share("videoram");
	map(0x5800, 0x58ff).mirror(0x0700).ram().share("spriteram");
}


void grchamp_state::main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x78).portr("ACCEL");
	map(0x02, 0x02).mirror(0x78).r(FUNC(grchamp_state::sub_to_main_comm_r));
	map(0x03, 0x03).mirror(0x78).portr("WHEEL");
	map(0x04, 0x04).mirror(0x78).portr("DSWA");
	map(0x05, 0x05).mirror(0x78).portr("DSWB");
	map(0x06, 0x06).mirror(0x78).portr("TILT");
	map(0x01, 0x01).mirror(0x60).r(FUNC(grchamp_state::pc3259_0_r));
	map(0x09, 0x09).mirror(0x60).r(FUNC(grchamp_state::pc3259_1_r));
	map(0x11, 0x11).mirror(0x60).r(FUNC(grchamp_state::pc3259_2_r));
	map(0x19, 0x19).mirror(0x60).r(FUNC(grchamp_state::pc3259_3_r));
	map(0x00, 0x0f).mirror(0x40).w(FUNC(grchamp_state::cpu0_outputs_w));
	map(0x10, 0x13).mirror(0x40).w(FUNC(grchamp_state::main_to_sub_comm_w));
	map(0x20, 0x20).select(0x0c).mirror(0x53).w(FUNC(grchamp_state::led_board_w));
}


/* complete memory map derived from schematics */
void grchamp_state::sub_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x27ff).ram().w(FUNC(grchamp_state::left_w)).share("leftram");
	map(0x2800, 0x2fff).ram().w(FUNC(grchamp_state::right_w)).share("rightram");
	map(0x3000, 0x37ff).ram().w(FUNC(grchamp_state::center_w)).share("centerram");
	map(0x4000, 0x43ff).mirror(0x0400).ram();
	map(0x5000, 0x6fff).rom();
}


void grchamp_state::sub_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).r(FUNC(grchamp_state::main_to_sub_comm_r));
	map(0x00, 0x0f).mirror(0x70).w(FUNC(grchamp_state::cpu1_outputs_w));
}


/* complete memory map derived from schematics;
   the grchamp sound system is almost identical to taitosj.cpp sound system
*/
/* Sound cpu address map ( * = used within this section; x = don't care )
           |           |           |
15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 0  *  *  *                                       R  74LS138 @ 10B
 0  0  0  0  *  *  *  *  *  *  *  *  *  *  *  *   R  ROM (7B)
 0  0  0  1  *  *  *  *  *  *  *  *  *  *  *  *   R  ROM (6B)
(0  0  1  0  *  *  *  *  *  *  *  *  *  *  *  *   R  ROM (5B), not populated, OPEN BUS)
(0  0  1  1  *  *  *  *  *  *  *  *  *  *  *  *   R  ROM (4B), not populated, OPEN BUS)
(0  1  *  *  x  x  x  x  x  x  x  x  x  x  x  x   R is also decoded by the 74LS138 @ 10B, but those four outputs are not connected anywhere and that region is occupied below)
 0  1  *  *  *                                    RW 74LS138 @ 9C
 0  1  0  0  0  0  *  *  *  *  *  *  *  *  *  *   RW SRAMs (2x 2114 @ 5C, 4C)
 0  1  0  0  0  1  x  x  x  x  x  x  x  x  x  x   OPEN BUS
 0  1  0  0  1                                    RW /CS5 (AY chips, this area has a 2 clock waitstate penalty on any access)
 0  1  0  0  1  x  x  x  x  x  x  x  x  0  0  0   W  Address AY @ 3B (with dac/volume connected to ioa/iob)
 0  1  0  0  1  x  x  x  x  x  x  x  x  0  0  1   RW  Data AY @ 3B (with dac/volume connected to ioa/iob)
 0  1  0  0  1  x  x  x  x  x  x  x  x  0  1  0   W  Address AY @ 2B
 0  1  0  0  1  x  x  x  x  x  x  x  x  0  1  1   RW  Data AY @ 2B
 0  1  0  0  1  x  x  x  x  x  x  x  x  1  x  0   W  Address AY @ 1B (with filter caps on ioa1,ioa0, nmi enable on iob0, dac attenuate on ioa7)
 0  1  0  0  1  x  x  x  x  x  x  x  x  1  x  1   RW  Data AY @ 1B (with filter caps on ioa1,ioa0, nmi enable on iob0, dac attenuate on ioa7)
 0  1  0  1  0                                    RW /CS6 (soundlatch/semaphores/nmi state)
 0  1  0  1  0  x  x  x  x  x  x  x  x  x  *  *   RW  74155 @ 6D
 0  1  0  1  0  x  x  x  x  x  x  x  x  x  0  0   R   Read soundlatch, and clear main->sound semaphore
 0  1  0  1  0  x  x  x  x  x  x  x  x  x  0  0   W   Clear bit 7 in soundlatch
 0  1  0  1  0  x  x  x  x  x  x  x  x  x  0  1   R   Read main->sound semaphore state in bit 3, bit 2 is 0, bits 1 and 0 are both 1, remaining bits are open bus
 0  1  0  1  0  x  x  x  x  x  x  x  x  x  0  1   W   OPEN BUS
 0  1  0  1  0  x  x  x  x  x  x  x  x  x  1  x   RW  OPEN BUS
*/
void grchamp_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3005).nopr().nopw();
	map(0x4000, 0x43ff).ram();
	map(0x4800, 0x4801).mirror(0x07f8).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x4801, 0x4801).mirror(0x07f8).r("ay1", FUNC(ay8910_device::data_r));
	map(0x4802, 0x4803).mirror(0x07f8).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x4803, 0x4803).mirror(0x07f8).r("ay2", FUNC(ay8910_device::data_r));
	map(0x4804, 0x4805).mirror(0x07fa).w("ay3", FUNC(ay8910_device::address_data_w));
	map(0x4805, 0x4805).mirror(0x07fa).r("ay3", FUNC(ay8910_device::data_r));
	map(0x5000, 0x5000).mirror(0x07fc).r(FUNC(grchamp_state::soundlatch_r)).w(FUNC(grchamp_state::soundlatch_clear7_w));
	map(0x5001, 0x5001).mirror(0x07fc).r(FUNC(grchamp_state::soundlatch_flags_r)).nopw(); // writes here on taitosj reset the secondary semaphore, which doesn't exist on grchamp, but the code tries to reset it anyway!
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( grchamp )

	PORT_START("TILT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )   /* High Score reset switch */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_TOGGLE   /* High Gear */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )       /* Tilt */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )   /* Service */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )      /* Coin A */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )      /* Coin B */

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_CODE_INC(KEYCODE_LCONTROL)
	//mask,default,type,sensitivity,delta,min,max

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_REVERSE
	//mask,default,type,sensitivity,delta,min,max

	PORT_START("DSWA")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x02, "Extra Race" )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "4th" )
	PORT_DIPSETTING(    0x02, "5th" )
	PORT_DIPSETTING(    0x01, "6th" )
	PORT_DIPSETTING(    0x03, "7th" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_HIGH, "SW2:3" )  /* Listed as "Unused and should remain in the OFF position" */
	PORT_DIPNAME( 0x08, 0x00, "RAM Test" )          PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin System" )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "1 Way" )
	PORT_DIPSETTING(    0x00, "2 Way" )
	PORT_DIPNAME( 0x20, 0x00, "Display '1981'" )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Display Score" )     PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "LEDs" )
	PORT_DIPSETTING(    0x40, "On Screen" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void grchamp_state::grchamp(machine_config &config)
{
	/* basic machine hardware */
	/* CPU BOARD */
	Z80(config, m_maincpu, PIXEL_CLOCK/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &grchamp_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &grchamp_state::main_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(grchamp_state::cpu0_interrupt));

	/* GAME BOARD */
	Z80(config, m_subcpu, PIXEL_CLOCK/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &grchamp_state::sub_map);
	m_subcpu->set_addrmap(AS_IO, &grchamp_state::sub_portmap);
	m_subcpu->set_vblank_int("screen", FUNC(grchamp_state::cpu1_interrupt));

	/* SOUND BOARD */
	Z80(config, m_audiocpu, SOUND_CLOCK/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &grchamp_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(grchamp_state::irq0_line_hold), attotime::from_hz((double)SOUND_CLOCK/4/16/16/10/16));

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count(m_screen, 8);
	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_grchamp);
	PALETTE(config, m_palette, FUNC(grchamp_state::grchamp_palette), 32);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(grchamp_state::screen_update));


	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	INPUT_MERGER_ALL_HIGH(config, m_soundnmi).output_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ay8910_device &ay1(AY8910(config, "ay1", SOUND_CLOCK/4));    /* 3B */
	ay1.port_a_write_callback().set(FUNC(grchamp_state::portA_0_w));
	ay1.port_b_write_callback().set(FUNC(grchamp_state::portB_0_w));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.2);

	AY8910(config, "ay2", SOUND_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 0.2);

	ay8910_device &ay3(AY8910(config, "ay3", SOUND_CLOCK/4));    /* 1B */
	ay3.port_a_write_callback().set(FUNC(grchamp_state::portA_2_w));
	ay3.port_b_write_callback().set(FUNC(grchamp_state::portB_2_w));
	ay3.add_route(ALL_OUTPUTS, "mono", 0.2);

	DISCRETE(config, m_discrete, grchamp_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( grchamp )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gm03",   0x0000, 0x1000, CRC(47fda76e) SHA1(fd5f1a651481669d64e5e0799369c22472265535) )
	ROM_LOAD( "gm04",   0x1000, 0x1000, CRC(07a623dc) SHA1(bb8a6531d95e996148c06fd336db4054eb1d28dd) )
	ROM_LOAD( "gm05",   0x2000, 0x1000, CRC(716e1fba) SHA1(fe596873c932513227b982cd23af440d31612de9) )
	ROM_LOAD( "gm06",   0x3000, 0x1000, CRC(157db30b) SHA1(a74314d3aef4659ea96ed659e5db2883e7ae1cb1) )

	ROM_REGION( 0x8000, "sub", 0 )
	ROM_LOAD( "gm09",   0x0000, 0x1000, CRC(d57bd109) SHA1(d1cb5ba783eaceda45893f6404fe9dbac740a2de) )
	ROM_LOAD( "gm10",   0x1000, 0x1000, CRC(41ba07f1) SHA1(103eeacdd36b4347fc62debb6b5f4163083313f4) )
	ROM_LOAD( "gr16",   0x5000, 0x1000, CRC(885d708e) SHA1(d5d2978a0eeca167ec1fb9f6f981388de46fbf81) )
	ROM_LOAD( "gr15",   0x6000, 0x1000, CRC(a822430b) SHA1(4d29612489362d2dc3f3a9eab609902a50c34aff) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gm07",   0x0000, 0x1000, CRC(65dcc572) SHA1(c9b19af365fa7ade2698be0bb892591ba281ecb0) )
	ROM_LOAD( "gm08",   0x1000, 0x1000, CRC(224d880c) SHA1(68aaaa0213d09cf34ba50c91d8c031d041f8a76f) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* characters/sprites */
	ROM_LOAD( "gm02",   0x0000, 0x1000, CRC(5911948d) SHA1(6f3a9a7f8d6a04b8e6d83756764c9c4185983d9b) )
	ROM_LOAD( "gm01",   0x1000, 0x1000, CRC(846f8e89) SHA1(346bfd69268606fde27643b4d135b481536b73b1) )

	ROM_REGION( 0x2000, "gfx2", 0 ) /* left tiles */
	ROM_LOAD( "gr20",   0x0000, 0x1000, CRC(88ba2c03) SHA1(4dfd136f122663223043c6cd79566f8eeec72681) )
	ROM_LOAD( "gr19",   0x1000, 0x1000, CRC(ff34b444) SHA1(51c67a1691da3a2d8ddcff5fd8fa816b1f9c60c0) )

	ROM_REGION( 0x2000, "gfx3", 0 ) /* right tiles */
	ROM_LOAD( "gr21",   0x0000, 0x1000, CRC(2f77a9f3) SHA1(9e20a776c5e8c7577c3e8467d4f8ac7ac909901f) )
	ROM_LOAD( "gr22",   0x1000, 0x1000, CRC(31bb5fc7) SHA1(9f638e632e7c72461bedecb710ac9b30f015eebf) )

	ROM_REGION( 0x2000, "gfx4", 0 ) /* center tiles */
	ROM_LOAD( "gr13",   0x0000, 0x1000, CRC(d5e19ebd) SHA1(d0ca553eec87619ec489f7ba6238f1fdde7c480b) )
	ROM_LOAD( "gr14",   0x1000, 0x1000, CRC(d129b8e4) SHA1(db25bfde2a48e14d38a43133d88d479c3cc1397a) )

	ROM_REGION( 0x0800, "gfx5", 0 ) /* rain */
	ROM_LOAD( "gr10",   0x0000, 0x0800, CRC(b1f0a873) SHA1(f7ef1a16556ae3e7d70209bcb38ea3ae94208789) )

	ROM_REGION( 0x0800, "gfx6", 0 ) /* headlights */
	ROM_LOAD( "gr12",   0x0000, 0x0800, CRC(f3bc599e) SHA1(3ec19584896a0bf10b9c5750f3c78ad3e722cc49) )

	ROM_REGION( 0x1000, "gfx7", 0 ) /* player */
	ROM_LOAD( "gr11",   0x0000, 0x1000, CRC(54eb3ec9) SHA1(22739240f53c708d8e53094d96916778e12beeed) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "gr23.bpr", 0x00, 0x20, CRC(41c6c48d) SHA1(8bd14b5f02f9da0a68e3125955be18462b57401d) ) /* background colors */
	ROM_LOAD( "gr09.bpr", 0x20, 0x20, CRC(260fb2b9) SHA1(db0bf49f12a944613d113317d7dfea25bd7469fc) ) /* sprite/text colors */
ROM_END

ROM_START( grchampa )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gr05a",  0x0000, 0x1000, CRC(4e54eeeb) SHA1(e8327e2a4596233f5f4a4dbf7cdf90f67447fc2a) )
	ROM_LOAD( "gr06",   0x1000, 0x1000, CRC(4afd9fb2) SHA1(93b6829b5196cbeb37925bda29722f02a9ef83be) )
	ROM_LOAD( "gr07-1", 0x2000, 0x1000, CRC(4deed8a1) SHA1(4616e1467bc696272117ad3f55cbff420c7e1deb) )
	ROM_LOAD( "gr08",   0x3000, 0x1000, CRC(2e0670ab) SHA1(b9a39133d60f1d8316005f9d029db63f4e486174) )

	ROM_REGION( 0x8000, "sub", 0 )
	ROM_LOAD( "gr18",   0x0000, 0x1000, CRC(becd0e81) SHA1(a5469711efab6c4befed7a5ecdccf5d14e6e0aa9) )
	ROM_LOAD( "gr17",   0x1000, 0x1000, CRC(aeba2f28) SHA1(c3efe5fede642465d42a1317d114bddd0b961884) )
	ROM_LOAD( "gr16",   0x5000, 0x1000, CRC(885d708e) SHA1(d5d2978a0eeca167ec1fb9f6f981388de46fbf81) )
	ROM_LOAD( "gr15",   0x6000, 0x1000, CRC(a822430b) SHA1(4d29612489362d2dc3f3a9eab609902a50c34aff) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gr24",   0x0000, 0x0800, CRC(66621fa8) SHA1(957f60fbf4bc0a36931a8c2bde05aea1f9e6671f) )
	ROM_LOAD( "gr25",   0x0800, 0x0800, CRC(9b8cdbf1) SHA1(e96af63896b8af0e2ed5a94172c6efdaaf56f678) )
	ROM_LOAD( "gr26",   0x1000, 0x0800, CRC(335b0f73) SHA1(790c31cda27efd149ddaab695852cad121fa1586) )
	ROM_LOAD( "gr27",   0x1800, 0x0800, CRC(3bad8c2e) SHA1(0a429c687f95b995979f995a44238f44c00a5a63) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* characters/sprites */
	ROM_LOAD( "gr04",   0x0000, 0x0800, CRC(7617e65c) SHA1(c83c15cff14a9d7acc2696807b3f0c1b83d095c8) )
	ROM_LOAD( "gr03",   0x0800, 0x0800, CRC(e7bdf2ee) SHA1(343d8a2b9e11cbe47d66f698150036f4a7d51c84) )
	ROM_LOAD( "gr02",   0x1000, 0x0800, CRC(7d60dd41) SHA1(9021ad635acc65341532280f9ba1f349ac3a5af0) )
	ROM_LOAD( "gr01",   0x1800, 0x0800, CRC(5f13ae33) SHA1(b9c05c9e3f5676c6bcd4019f6ac82277d56d253e) )

	ROM_REGION( 0x2000, "gfx2", 0 ) /* left tiles */
	ROM_LOAD( "gr20",   0x0000, 0x1000, CRC(88ba2c03) SHA1(4dfd136f122663223043c6cd79566f8eeec72681) )
	ROM_LOAD( "gr19",   0x1000, 0x1000, CRC(ff34b444) SHA1(51c67a1691da3a2d8ddcff5fd8fa816b1f9c60c0) )

	ROM_REGION( 0x2000, "gfx3", 0 ) /* right tiles */
	ROM_LOAD( "gr21",   0x0000, 0x1000, CRC(2f77a9f3) SHA1(9e20a776c5e8c7577c3e8467d4f8ac7ac909901f) )
	ROM_LOAD( "gr22",   0x1000, 0x1000, CRC(31bb5fc7) SHA1(9f638e632e7c72461bedecb710ac9b30f015eebf) )

	ROM_REGION( 0x2000, "gfx4", 0 ) /* center tiles */
	ROM_LOAD( "gr13",   0x0000, 0x1000, CRC(d5e19ebd) SHA1(d0ca553eec87619ec489f7ba6238f1fdde7c480b) )
	ROM_LOAD( "gr14",   0x1000, 0x1000, CRC(d129b8e4) SHA1(db25bfde2a48e14d38a43133d88d479c3cc1397a) )

	ROM_REGION( 0x0800, "gfx5", 0 ) /* rain */
	ROM_LOAD( "gr10",   0x0000, 0x0800, CRC(b1f0a873) SHA1(f7ef1a16556ae3e7d70209bcb38ea3ae94208789) )

	ROM_REGION( 0x0800, "gfx6", 0 ) /* headlights */
	ROM_LOAD( "gr12",   0x0000, 0x0800, CRC(f3bc599e) SHA1(3ec19584896a0bf10b9c5750f3c78ad3e722cc49) )

	ROM_REGION( 0x1000, "gfx7", 0 ) /* player */
	ROM_LOAD( "gr11",   0x0000, 0x1000, CRC(54eb3ec9) SHA1(22739240f53c708d8e53094d96916778e12beeed) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "gr23.bpr", 0x00, 0x20, CRC(41c6c48d) SHA1(8bd14b5f02f9da0a68e3125955be18462b57401d) ) /* background colors */
	ROM_LOAD( "gr09.bpr", 0x20, 0x20, CRC(260fb2b9) SHA1(db0bf49f12a944613d113317d7dfea25bd7469fc) ) /* sprite/text colors */
ROM_END

ROM_START( grchampb )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gg01-a", 0x0000, 0x1000, CRC(9a815139) SHA1(0f4ea577c12dd13da9fa114d6a1653cb516324f3) )
	ROM_LOAD( "gg02-a", 0x1000, 0x1000, CRC(e35ac684) SHA1(202899f1b55f68ea6017b18b149f254735661bea) )
	ROM_LOAD( "gg03",   0x2000, 0x1000, CRC(feb0ad96) SHA1(1335725d25777f9c462b4c97f7a268378dc7753d) )
	ROM_LOAD( "gg04",   0x3000, 0x1000, CRC(afabfe96) SHA1(d265595522ccf94395f2235c02d8c37f1ada9950) )

	ROM_REGION( 0x8000, "sub", 0 )
	ROM_LOAD( "gm09",   0x0000, 0x1000, CRC(d57bd109) SHA1(d1cb5ba783eaceda45893f6404fe9dbac740a2de) )
	ROM_LOAD( "gm10",   0x1000, 0x1000, CRC(41ba07f1) SHA1(103eeacdd36b4347fc62debb6b5f4163083313f4) )
	ROM_LOAD( "gr16",   0x5000, 0x1000, CRC(885d708e) SHA1(d5d2978a0eeca167ec1fb9f6f981388de46fbf81) )
	ROM_LOAD( "gr15",   0x6000, 0x1000, CRC(a822430b) SHA1(4d29612489362d2dc3f3a9eab609902a50c34aff) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gm07",   0x0000, 0x1000, CRC(65dcc572) SHA1(c9b19af365fa7ade2698be0bb892591ba281ecb0) )
	ROM_LOAD( "gm08",   0x1000, 0x1000, CRC(224d880c) SHA1(68aaaa0213d09cf34ba50c91d8c031d041f8a76f) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* characters/sprites */
	ROM_LOAD( "gm02",   0x0000, 0x1000, CRC(5911948d) SHA1(6f3a9a7f8d6a04b8e6d83756764c9c4185983d9b) )
	ROM_LOAD( "gm01",   0x1000, 0x1000, CRC(846f8e89) SHA1(346bfd69268606fde27643b4d135b481536b73b1) )

	ROM_REGION( 0x2000, "gfx2", 0 ) /* left tiles */
	ROM_LOAD( "gr20",   0x0000, 0x1000, CRC(88ba2c03) SHA1(4dfd136f122663223043c6cd79566f8eeec72681) )
	ROM_LOAD( "gr19",   0x1000, 0x1000, CRC(ff34b444) SHA1(51c67a1691da3a2d8ddcff5fd8fa816b1f9c60c0) )

	ROM_REGION( 0x2000, "gfx3", 0 ) /* right tiles */
	ROM_LOAD( "gr21",   0x0000, 0x1000, CRC(2f77a9f3) SHA1(9e20a776c5e8c7577c3e8467d4f8ac7ac909901f) )
	ROM_LOAD( "gr22",   0x1000, 0x1000, CRC(31bb5fc7) SHA1(9f638e632e7c72461bedecb710ac9b30f015eebf) )

	ROM_REGION( 0x2000, "gfx4", 0 ) /* center tiles */
	ROM_LOAD( "gr13",   0x0000, 0x1000, CRC(d5e19ebd) SHA1(d0ca553eec87619ec489f7ba6238f1fdde7c480b) )
	ROM_LOAD( "gr14",   0x1000, 0x1000, CRC(d129b8e4) SHA1(db25bfde2a48e14d38a43133d88d479c3cc1397a) )

	ROM_REGION( 0x0800, "gfx5", 0 ) /* rain */
	ROM_LOAD( "gr10",   0x0000, 0x0800, CRC(b1f0a873) SHA1(f7ef1a16556ae3e7d70209bcb38ea3ae94208789) )

	ROM_REGION( 0x0800, "gfx6", 0 ) /* headlights */
	ROM_LOAD( "gr12",   0x0000, 0x0800, CRC(f3bc599e) SHA1(3ec19584896a0bf10b9c5750f3c78ad3e722cc49) )

	ROM_REGION( 0x1000, "gfx7", 0 ) /* player */
	ROM_LOAD( "gr11",   0x0000, 0x1000, CRC(54eb3ec9) SHA1(22739240f53c708d8e53094d96916778e12beeed) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "gr23.bpr", 0x00, 0x20, CRC(41c6c48d) SHA1(8bd14b5f02f9da0a68e3125955be18462b57401d) ) /* background colors */
	ROM_LOAD( "gr09.bpr", 0x20, 0x20, CRC(260fb2b9) SHA1(db0bf49f12a944613d113317d7dfea25bd7469fc) ) /* sprite/text colors */
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1981, grchamp,        0, grchamp, grchamp, grchamp_state, empty_init, ROT270, "Taito", "Grand Champion (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_grchamp )
GAMEL( 1981, grchampa, grchamp, grchamp, grchamp, grchamp_state, empty_init, ROT270, "Taito", "Grand Champion (set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_grchamp ) // uses different ports. Bad dump?
GAMEL( 1981, grchampb, grchamp, grchamp, grchamp, grchamp_state, empty_init, ROT270, "Taito", "Grand Champion (set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_grchamp )
