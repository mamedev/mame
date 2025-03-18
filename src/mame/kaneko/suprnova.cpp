// license:BSD-3-Clause
// copyright-holders:David Haywood, Sylvain Glaize, Paul Priest, Olivier Galibert
/*
   Super Kaneko Nova System
    Original Driver by Sylvain Glaize
    taken to pieces and attempted reconstruction by David Haywood

Mainboard + Cart combo
  Regions are not cross compatible and have their own BIOS
   Regions by color coded cart:

    White      = Japan
    Light Blue = Europe
    Dark Blue  = USA
    Green      = Asia
    Red        = Korean

  Credits (in no particular order):
   Olivier Galibert for all the assistance and information he's provided
   R. Belmont for working on the SH2 timers so sound worked
   Nicola Salmoria for hooking up the Roz and improving the dirty tile handling
   Paul Priest for a lot of things
   Stephh for spotting what was wrong with Puzz Loop's inputs

Puzz Loop is currently the only game dumped for all known regions.  This game is normally found on a ROM 4 BOARD
   so the "U" location is printed on the label as U4 & U6.  However this same game has also been found on the
   ROM-2-BOARD using EPROMs labeled for the ROM 4 BOARD, but inserted in sockets at U8 & U10

ToDo:

galpanis: Are the priorities correct on the KANEKO logo at the start, the invisible faded logo obscures the rotating white lines

video:   Sprite Zooming - the current algorithm is leaving gaps, most noticeable in Gals Panic 4, and Jackie Chan which is sharing
         the video code.

video:   Sprite positions still kludged slightly (see skns_sprite_kludge)

------------------

SUPER-KANEKO-NOVA-SYSTEM
MAIN-BOARD-A
NEP-16


   BATT                                      HM514260
   CR2032                       SKNSA1       HM514260       SH2
            W24257                          LH5168
YMZ280B-F   W24257   BABY004  LATTICE  SW1  M62X42
                              pLSI1016
  HITACHI          28.636MHz 33.3333MHz
  DF1                        21.504MHz
J D49307     ALTAIR                   DENEB
             BY006-224                BY007-32F
A            (QFP208)                 (QFP208)

M                  W24257  W24257           HM514260
            /-     W24257  W24257                                 /-
M VEGA      ||             W24257  SPCII-B  HM514260              ||
  BY005-197 ||             W24257  JH-6186                        ||
A (QFP144)  ||                     (QFP208)                       ||
            ||                               W24257               ||
  DSW1(8)   ||      VIEWIII-A      SPCII-A   W24257 LH540202      ||
            ||      BL-001         JH-4181   W24257               ||
            \-      (QFP240)       (QFP208)  W24257               \-
  OKIM6253


Notes:
      SKNSA1 is BIOS (Asia)
      HM514260, W24257, LH5168, M62X42 all smt SRAM
      LH540202 is DIP SRAM


Cart Layout
-----------

SUPER-KANEKO-NOVA-SYSTEM
ROM-BOARD
NEP-16


 /-                                                    /-
 ||SK300-00  *  PAL PAL D431000 D431000  *       *     ||
 ||                                                    ||
 ||                                                    ||
 ||                        *   SK-200-00               ||
 ||          SK01A  SK01A                              ||
 ||          U8     U10    *      *    SK-101 SK-100-00||
 \-                                                    \-

 Notes:
       *: unpopulated position for surface mounted 16MBit SOP44 mask ROM
       U8 and U10 are socketed 27C040 EPROM
       All other ROMs are surface mounted SOP44 mask ROM

Cart Layout
-----------

SUPER KANEKO NOVA SYSTEM
ROM 4 BOARD
NEP-16

Top Side:
 /-                                                    /-
 ||SS300-00 SS210-00     SS102-00 SS103-00   SS201-00  ||
 ||U1       U3           U8       U32        U9        ||
 ||                                                    ||
 ||                                                    ||
 || 082*    SG01A  SG01A  #      #                     ||
 ||          U4     U6    U29    U30                   ||
 \-                                                    \-

Bottom Side:
 /-                                                    /-
 ||SS200-00 SS000-00     SS101-00       ^       ^      ||
 ||U17      U21          U20            U24     U26    ||
 ||                                                    ||
 ||                                                    ||
 ||                                  NEC     NEC       ||
 ||                                  D431000 D43100    ||
 \-                                                    \-

* Kaneko (208 pin PQFP)    # Empty sockets for 27C4001 / 27C040
  ROM0                     ^ Empty sockets for uPD23C32000
  082                        Full NEC ram number: D431000AGW-70LL
  9709PK002


Cart Layout
-----------

SUPER-KANEKO-NOVA-SYSTEM
ROM-2-BOARD
NEP-16


 /-                                                    /-
 ||PZL-300-00 * PAL PAL  *   PZL-200-00 PZL-210-00  *  ||
 ||                                                    ||
 ||                             D431000 D431000        ||
 ||                                                    ||
 ||          PZ01U  PZ01U  #    #                      ||
 ||          U8     U10    U43  U44   *   * PZL-100-00 ||
 \-                                                    \-


*/

#include "emu.h"
#include "suprnova.h"

#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "sound/ymz280b.h"

#include "speaker.h"


static void hit_calc_orig(uint16_t p, uint16_t s, uint16_t org, uint16_t *l, uint16_t *r)
{
	switch(org & 3) {
	case 0:
		*l = p;
		*r = p+s;
	break;
	case 1:
		*l = p-s/2;
		*r = *l+s;
	break;
	case 2:
		*l = p-s;
		*r = p;
	break;
	case 3:
		*l = p-s;
		*r = p+s;
	break;
	}
}

static void hit_calc_axis(uint16_t x1p, uint16_t x1s, uint16_t x2p, uint16_t x2s, uint16_t org,
				uint16_t *x1_p1, uint16_t *x1_p2, uint16_t *x2_p1, uint16_t *x2_p2,
				int16_t *x_in, uint16_t *x1tox2)
{
	uint16_t x1l=0, x1r=0, x2l=0, x2r=0;
	hit_calc_orig(x1p, x1s, org,      &x1l, &x1r);
	hit_calc_orig(x2p, x2s, org >> 8, &x2l, &x2r);

	*x1tox2 = x2p-x1p;
	*x1_p1 = x1p;
	*x2_p1 = x2p;
	*x1_p2 = x1r;
	*x2_p2 = x2l;
	*x_in = x1r-x2l;
}

void skns_state::hit_recalc()
{
	hit_t &hit = m_hit;

	hit_calc_axis(hit.x1p, hit.x1s, hit.x2p, hit.x2s, hit.org,
		&hit.x1_p1, &hit.x1_p2, &hit.x2_p1, &hit.x2_p2,
		&hit.x_in, &hit.x1tox2);
	hit_calc_axis(hit.y1p, hit.y1s, hit.y2p, hit.y2s, hit.org,
		&hit.y1_p1, &hit.y1_p2, &hit.y2_p1, &hit.y2_p2,
		&hit.y_in, &hit.y1toy2);
	hit_calc_axis(hit.z1p, hit.z1s, hit.z2p, hit.z2s, hit.org,
		&hit.z1_p1, &hit.z1_p2, &hit.z2_p1, &hit.z2_p2,
		&hit.z_in, &hit.z1toz2);

	hit.flag = 0;
	hit.flag |= hit.y2p > hit.y1p ? 0x8000 : hit.y2p == hit.y1p ? 0x4000 : 0x2000;
	hit.flag |= hit.y_in >= 0 ? 0 : 0x1000;
	hit.flag |= hit.x2p > hit.x1p ? 0x0800 : hit.x2p == hit.x1p ? 0x0400 : 0x0200;
	hit.flag |= hit.x_in >= 0 ? 0 : 0x0100;
	hit.flag |= hit.z2p > hit.z1p ? 0x0080 : hit.z2p == hit.z1p ? 0x0040 : 0x0020;
	hit.flag |= hit.z_in >= 0 ? 0 : 0x0010;
	hit.flag |= hit.x_in >= 0 && hit.y_in >= 0 && hit.z_in >= 0 ? 8 : 0;
	hit.flag |= hit.z_in >= 0 && hit.x_in >= 0                  ? 4 : 0;
	hit.flag |= hit.y_in >= 0 && hit.z_in >= 0                  ? 2 : 0;
	hit.flag |= hit.x_in >= 0 && hit.y_in >= 0                  ? 1 : 0;
/*  if(0)
    log_event("HIT", "Recalc, (%d,%d)-(%d,%d)-(%d,%d):(%d,%d)-(%d,%d)-(%d,%d):%04x, (%d,%d,%d), %04x",
          hit.x1p, hit.x1s, hit.y1p, hit.y1s, hit.z1p, hit.z1s,
          hit.x2p, hit.x2s, hit.y2p, hit.y2s, hit.z2p, hit.z2s,
          hit.org,
          hit.x_in, hit.y_in, hit.z_in, hit.flag);
*/
}

void skns_state::hit_w(offs_t offset, uint32_t data)
//void hit_w(uint32_t adr, uint32_t data, int type)
{
	hit_t &hit = m_hit;
	int adr = offset * 4;

	switch(adr) {
	case 0x00:
	case 0x28:
		hit.x1p = data;
	break;
	case 0x08:
	case 0x30:
		hit.y1p = data;
	break;
	case 0x38:
	case 0x50:
		hit.z1p = data;
	break;
	case 0x04:
	case 0x2c:
		hit.x1s = data;
	break;
	case 0x0c:
	case 0x34:
		hit.y1s = data;
	break;
	case 0x3c:
	case 0x54:
		hit.z1s = data;
	break;
	case 0x10:
	case 0x58:
		hit.x2p = data;
	break;
	case 0x18:
	case 0x60:
		hit.y2p = data;
	break;
	case 0x20:
	case 0x68:
		hit.z2p = data;
	break;
	case 0x14:
	case 0x5c:
		hit.x2s = data;
	break;
	case 0x1c:
	case 0x64:
		hit.y2s = data;
	break;
	case 0x24:
	case 0x6c:
		hit.z2s = data;
	break;
	case 0x70:
		hit.org = data;
	break;
	default:
//      log_write("HIT", adr, data, type);
	break;
	}
	hit_recalc();
}

void skns_state::hit2_w(uint32_t data)
{
	hit_t &hit = m_hit;

	// Decide to unlock on country char of string "FOR xxxxx" in BIOS ROM at offset 0x420
	// this code simulates behaviour of protection PLD
	data>>= 24;
	hit.disconnect = 1;
	switch (m_region)
	{
		case 'J':
			if (data == 0) hit.disconnect= 0;
		break;
		case 'U':
			if (data == 1) hit.disconnect= 0;
		break;
		case 'K':
			if (data == 2) hit.disconnect= 0;
		break;
		case 'E':
			if (data == 3) hit.disconnect= 0;
		break;
		case 'A':
			if (data < 2) hit.disconnect= 0;
		break;
		// unknown country id, unlock per default
		default:
			hit.disconnect= 0;
		break;
	}
}


uint32_t skns_state::hit_r(offs_t offset)
//uint32_t hit_r(uint32_t adr, int type)
{
	hit_t &hit = m_hit;
	int adr = offset *4;

//  log_read("HIT", adr, type);

	if(hit.disconnect)
		return 0x0000;
	switch(adr) {
	case 0x28:
	case 0x2a:
		return (uint16_t)machine().rand();
	case 0x00:
	case 0x10:
		return (uint16_t)hit.x_in;
	case 0x04:
	case 0x14:
		return (uint16_t)hit.y_in;
	case 0x18:
		return (uint16_t)hit.z_in;
	case 0x08:
	case 0x1c:
		return hit.flag;
	case 0x40:
		return hit.x1p;
	case 0x48:
		return hit.y1p;
	case 0x50:
		return hit.z1p;
	case 0x44:
		return hit.x1s;
	case 0x4c:
		return hit.y1s;
	case 0x54:
		return hit.z1s;
	case 0x58:
		return hit.x2p;
	case 0x60:
		return hit.y2p;
	case 0x68:
		return hit.z2p;
	case 0x5c:
		return hit.x2s;
	case 0x64:
		return hit.y2s;
	case 0x6c:
		return hit.z2s;
	case 0x70:
		return hit.org;
	case 0x80:
		return hit.x1tox2;
	case 0x84:
		return hit.y1toy2;
	case 0x88:
		return hit.z1toz2;
	case 0x90:
		return hit.x1_p1;
	case 0xa0:
		return hit.y1_p1;
	case 0xb0:
		return hit.z1_p1;
	case 0x98:
		return hit.x1_p2;
	case 0xa8:
		return hit.y1_p2;
	case 0xb8:
		return hit.z1_p2;
	case 0x94:
		return hit.x2_p1;
	case 0xa4:
		return hit.y2_p1;
	case 0xb4:
		return hit.z2_p1;
	case 0x9c:
		return hit.x2_p2;
	case 0xac:
		return hit.y2_p2;
	case 0xbc:
		return hit.z2_p2;
	default:
//      log_read("HIT", adr, type);
	return 0;
	}
}

/* end hit.c */


/* start old driver code */


TIMER_DEVICE_CALLBACK_MEMBER(skns_state::interrupt_callback)
{
	m_maincpu->set_input_line(param, HOLD_LINE);
}

void skns_state::machine_start()
{
	m_btiles = memregion("gfx3")->base();

	save_pointer(NAME(m_btiles), memregion("gfx3")->bytes());
	save_item(NAME(m_hit.x1p));
	save_item(NAME(m_hit.y1p));
	save_item(NAME(m_hit.z1p));
	save_item(NAME(m_hit.x1s));
	save_item(NAME(m_hit.y1s));
	save_item(NAME(m_hit.z1s));
	save_item(NAME(m_hit.x2p));
	save_item(NAME(m_hit.y2p));
	save_item(NAME(m_hit.z2p));
	save_item(NAME(m_hit.x2s));
	save_item(NAME(m_hit.y2s));
	save_item(NAME(m_hit.z2s));
	save_item(NAME(m_hit.org));
	save_item(NAME(m_hit.x1_p1));
	save_item(NAME(m_hit.x1_p2));
	save_item(NAME(m_hit.y1_p1));
	save_item(NAME(m_hit.y1_p2));
	save_item(NAME(m_hit.z1_p1));
	save_item(NAME(m_hit.z1_p2));
	save_item(NAME(m_hit.x2_p1));
	save_item(NAME(m_hit.x2_p2));
	save_item(NAME(m_hit.y2_p1));
	save_item(NAME(m_hit.y2_p2));
	save_item(NAME(m_hit.z2_p1));
	save_item(NAME(m_hit.z2_p2));
	save_item(NAME(m_hit.x1tox2));
	save_item(NAME(m_hit.y1toy2));
	save_item(NAME(m_hit.z1toz2));
	save_item(NAME(m_hit.x_in));
	save_item(NAME(m_hit.y_in));
	save_item(NAME(m_hit.z_in));
	save_item(NAME(m_hit.flag));
	save_item(NAME(m_hit.disconnect));
}

void skns_state::machine_reset()
{
	hit_t &hit = m_hit;

	if (m_region != 'A')
		hit.disconnect= 1;
	else
		hit.disconnect= 0;

	membank("bank1")->set_base(memregion("user1")->base());
}


TIMER_DEVICE_CALLBACK_MEMBER(skns_state::irq)
{
	int scanline = param;

	if(scanline == 240)
		m_maincpu->set_input_line(5,HOLD_LINE); //vblank
	else if(scanline == 0)
		m_maincpu->set_input_line(1,HOLD_LINE); // spc
}

/**********************************************************************************

    Input port definitions

    NOTE: The driver reads data from eight 8bit input ports, even if they
           are unused. So I left them mapped.

**********************************************************************************/

template <int P>
ioport_value skns_state::paddle_r()
{
	return m_paddle[P]->read();
}

static INPUT_PORTS_START( skns )        /* 3 buttons, 2 players */
	PORT_START("400000")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("400004")
	PORT_SERVICE( 0x00000001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Flip_Screen ) ) // This port affects 0x000000040191c8 function
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, "Use Backup Ram" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( No ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, "Freeze" )
	PORT_DIPSETTING(          0x00000000, "Freezes the game")
	PORT_DIPSETTING(          0x00000080, "Right value")
	PORT_BIT( 0x0000ff00, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(skns_state::paddle_r<2>)) // Paddle C
	PORT_BIT( 0x00ff0000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(skns_state::paddle_r<1>)) // Paddle B
	PORT_BIT( 0xff000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(skns_state::paddle_r<0>)) // Paddle A

	PORT_START("40000c")
	PORT_BIT( 0x000000ff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(skns_state::paddle_r<3>)) // Paddle D
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Paddle.A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Paddle.B")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Paddle.C")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Paddle.D")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( skns_1p )     /* 2 buttons, 1 player */
	PORT_INCLUDE( skns )

	PORT_MODIFY("400000")
	/* jjparads and jjparad2 are 1 player only games
	   ryouran and teljan have an unemulated feature
	   that allows to play them in two player mode
	   via a cable-network connection (untestable)
	   Service mode test shows only P1 inputs */
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	/* same as above, coin 2 and start 2 are untestable
	   in ryouran and teljan. So I left disabled for now */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED )   /* Start 2 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED )   /* Coin 2 */
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )   /* No Button 3 */

	PORT_MODIFY("400004")
	PORT_DIPNAME( 0x00000010, 0x00000010, "Test Mode" )
	PORT_DIPSETTING(          0x00000010, DEF_STR(Off) )
	PORT_DIPSETTING(          0x00000000, DEF_STR(On) )
INPUT_PORTS_END

static INPUT_PORTS_START( cyvern )      /* 2 buttons, 2 players */
	PORT_INCLUDE( skns )

	PORT_MODIFY("400000")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )   /* No Button 3 */
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )   /* No Button 3 */
INPUT_PORTS_END

static INPUT_PORTS_START( galpanis )    /* 1 button, 2 players */
	PORT_INCLUDE( skns )

	PORT_MODIFY("400000")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNUSED )   /* No Button 2 */
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )   /* No Button 3 */
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNUSED )   /* No Button 2 */
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )   /* No Button 3 */
INPUT_PORTS_END

static INPUT_PORTS_START( puzzloop )    /* 2 buttons, 2 players, paddle */
	PORT_INCLUDE( skns )

	PORT_MODIFY("400000")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )   /* No Button 3 */
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )   /* No Button 3 */

	PORT_MODIFY("Paddle.A")  /* Paddle A */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_MODIFY("Paddle.B")  /* Paddle B */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( vblokbrk )    /* 3 buttons, 2 players, paddle */
	PORT_INCLUDE( skns )

	PORT_MODIFY("Paddle.A")  /* Paddle A */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_MODIFY("Paddle.B")  /* Paddle B */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END



void skns_state::io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch(offset) {
	case 2:
		if(ACCESSING_BITS_24_31)
		{ /* Coin Lock/Count */
//          machine().bookkeeping().coin_counter_w(0, data & 0x01000000);
//          machine().bookkeeping().coin_counter_w(1, data & 0x02000000);
//          machine().bookkeeping().coin_lockout_w(0, ~data & 0x04000000);
//          machine().bookkeeping().coin_lockout_w(1, ~data & 0x08000000); // Works in puzzloop, others behave strange.
		}
		if(ACCESSING_BITS_16_23)
		{ /* Analogue Input Select */
		}
		if(ACCESSING_BITS_8_15)
		{ /* Extended Output - Port A, Mahjong inputs, Comms etc. */
		}
		if(ACCESSING_BITS_0_7)
		{ /* Extended Output - Port B */
		}
	break;
	case 3:
		if(ACCESSING_BITS_8_15)
		{ /* Interrupt Clear, do we need these? */
/*          if(data&0x01)
                m_maincpu->set_input_line(1,CLEAR_LINE);
            if(data&0x02)
                m_maincpu->set_input_line(3,CLEAR_LINE);
            if(data&0x04)
                m_maincpu->set_input_line(5,CLEAR_LINE);
            if(data&0x08)
                m_maincpu->set_input_line(7,CLEAR_LINE);
            if(data&0x10)
                m_maincpu->set_input_line(9,CLEAR_LINE);
            if(data&0x20)
                m_maincpu->set_input_line(0xb,CLEAR_LINE);
            if(data&0x40)
                m_maincpu->set_input_line(0xd,CLEAR_LINE);
            if(data&0x80)
                m_maincpu->set_input_line(0xf,CLEAR_LINE);*/

			/* idle skip for vblokbrk/sarukani, i can't find a better place to put it :-( but i think it works ok unless its making the game too fast */
			if (m_maincpu->pc()==0x04013B42)
			{
				if (!strcmp(machine().system().name,"vblokbrk") ||
					!strcmp(machine().system().name,"sarukani"))
					m_maincpu->spin_until_interrupt();
			}

		}
		else
		{
			logerror("Unk IO Write memmask:%08x offset:%08x data:%08x\n", mem_mask, offset, data);
		}
	break;
	default:
		logerror("Unk IO Write memmask:%08x offset:%08x data:%08x\n", mem_mask, offset, data);
	break;
	}
}

/* end old driver code */

void skns_state::v3t_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_v3t_ram[offset]);

	m_gfxdecode->gfx(1)->mark_dirty(offset/0x40);
	m_gfxdecode->gfx(3)->mark_dirty(offset/0x20);

	data = m_v3t_ram[offset];
// i think we need to swap around to decode .. endian issues?

	m_btiles[offset*4+0] = (data & 0xff000000) >> 24;
	m_btiles[offset*4+1] = (data & 0x00ff0000) >> 16;
	m_btiles[offset*4+2] = (data & 0x0000ff00) >> 8;
	m_btiles[offset*4+3] = (data & 0x000000ff) >> 0;
}

void skns_state::skns_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom(); /* BIOS ROM */
	map(0x00400000, 0x0040000f).w(FUNC(skns_state::io_w)); /* I/O Write */
	map(0x00400000, 0x00400003).portr("400000");
	map(0x00400004, 0x00400007).portr("400004");
	/* In between is write only */
	map(0x0040000c, 0x0040000f).portr("40000c");
	map(0x00800000, 0x00801fff).ram().share("nvram"); /* 'backup' RAM */
	map(0x00c00000, 0x00c00001).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)); /* ymz280_w (sound) */
	map(0x01000000, 0x0100000f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0x01800000, 0x01800003).w(FUNC(skns_state::hit2_w));
	map(0x02000000, 0x02003fff).ram().share("spriteram"); /* sprite ram */
	map(0x02100000, 0x0210003f).ram().share("spc_regs"); /* sprite registers */
	map(0x02400000, 0x0240007f).ram().w(FUNC(skns_state::v3_regs_w)).share("v3_regs"); /* tilemap registers */
	map(0x02500000, 0x02503fff).ram().w(FUNC(skns_state::tilemapA_w)).share("tilemapa_ram"); /* tilemap A */
	map(0x02504000, 0x02507fff).ram().w(FUNC(skns_state::tilemapB_w)).share("tilemapb_ram"); /* tilemap B */
	map(0x02600000, 0x02607fff).ram().share("v3slc_ram"); /* tilemap linescroll */
	map(0x02a00000, 0x02a0001f).ram().w(FUNC(skns_state::pal_regs_w)).share("pal_regs");
	map(0x02a40000, 0x02a5ffff).ram().w(FUNC(skns_state::palette_ram_w)).share("palette_ram");
	map(0x02f00000, 0x02f000ff).rw(FUNC(skns_state::hit_r), FUNC(skns_state::hit_w));
	map(0x04000000, 0x041fffff).bankr("bank1"); /* GAME ROM */
	map(0x04800000, 0x0483ffff).ram().w(FUNC(skns_state::v3t_w)).share("v3t_ram"); /* tilemap b ram based tiles */
	map(0x05000000, 0x05000003).nopw();  /* watchdog, probably.  Always writes 0 */
	map(0x06000000, 0x060fffff).ram().share("main_ram");
	map(0xc0000000, 0xc0000fff).ram().share("cache_ram"); /* 'cache' RAM */
}

/***** GFX DECODE *****/

static GFXDECODE_START( skns_bg )
	/* "spritegen" is sprites, RLE encoded */
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x8_raw, 0x000, 128 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_16x16x8_raw, 0x000, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_packed_lsb, 0x000, 128 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_16x16x4_packed_lsb, 0x000, 128 )
GFXDECODE_END

/***** MACHINE DRIVER *****/

// XTALs : 28.636MHz, 33.3333MHz, 21.504MHz
void skns_state::skns(machine_config &config)
{
	SH7604(config, m_maincpu, XTAL(28'636'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &skns_state::skns_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(skns_state::irq), "screen", 0, 1);

	MSM6242(config, "rtc", XTAL(32'768));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	timer_device &int15_timer(TIMER(config, "int15_timer"));
	int15_timer.configure_periodic(FUNC(skns_state::interrupt_callback), attotime::from_msec(2));
	int15_timer.config_param(15);
	timer_device &int11_timer(TIMER(config, "int11_timer"));
	int11_timer.configure_periodic(FUNC(skns_state::interrupt_callback), attotime::from_msec(8));
	int11_timer.config_param(11);
	timer_device &int9_timer(TIMER(config, "int9_timer"));
	int9_timer.configure_periodic(FUNC(skns_state::interrupt_callback), attotime::from_hz(XTAL(28'636'000)/1824));
	int9_timer.config_param(9);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_refresh_hz(59.5971); // measured by Guru
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(340,262);
	m_screen->set_visarea(0,319,0,239);
	m_screen->set_screen_update(FUNC(skns_state::screen_update));
	m_screen->screen_vblank().set(FUNC(skns_state::screen_vblank));

	PALETTE(config, m_palette).set_entries(32768);
	GFXDECODE(config, m_gfxdecode, m_palette, skns_bg);

	SKNS_SPRITE(config, m_spritegen, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(33'333'333) / 2));
	ymz.add_route(0, "lspeaker", 1.0);
	ymz.add_route(1, "rspeaker", 1.0);
}

MACHINE_RESET_MEMBER(skns_state,sknsa)
{
	m_region = 'A';
	skns_state::machine_reset();
}

MACHINE_RESET_MEMBER(skns_state,sknsj)
{
	m_region = 'J';
	skns_state::machine_reset();
}

MACHINE_RESET_MEMBER(skns_state,sknsu)
{
	m_region = 'U';
	skns_state::machine_reset();
}

MACHINE_RESET_MEMBER(skns_state,sknse)
{
	m_region = 'E';
	skns_state::machine_reset();
}

MACHINE_RESET_MEMBER(skns_state,sknsk)
{
	m_region = 'K';
	skns_state::machine_reset();
}


void skns_state::sknsa(machine_config &config)
{
	skns(config);
	MCFG_MACHINE_RESET_OVERRIDE(skns_state,sknsa)
}

void skns_state::sknsj(machine_config &config)
{
	skns(config);
	MCFG_MACHINE_RESET_OVERRIDE(skns_state,sknsj)
}

void skns_state::sknsu(machine_config &config)
{
	skns(config);
	MCFG_MACHINE_RESET_OVERRIDE(skns_state,sknsu)
}

void skns_state::sknse(machine_config &config)
{
	skns(config);
	MCFG_MACHINE_RESET_OVERRIDE(skns_state,sknse)
}

void skns_state::sknsk(machine_config &config)
{
	skns(config);
	MCFG_MACHINE_RESET_OVERRIDE(skns_state,sknsk)
}

/***** IDLE SKIPPING *****/

uint32_t skns_state::gutsn_speedup_r()
{
/*
    0402206A: MOV.L   @($8C,PC),R5
    0402206C: MOV.L   @($8C,PC),R1
    0402206E: MOV.L   @R1,R2       // R1 == 600C780
    04022070: MOV.L   @R5,R3       // R5 == 6000078
    04022072: CMP/EQ  R2,R3
    04022074: BT      $0402206C
*/
	if (m_maincpu->pc()==0x402206e)
	{
		if(m_main_ram[0x00078/4] == m_main_ram[0x0c780/4])
			m_maincpu->spin_until_interrupt();
	}
	return m_main_ram[0x0c780/4];
}

uint32_t skns_state::cyvern_speedup_r()
{
	if (m_maincpu->pc()==0x402ebd2) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x4d3c8/4];
}

uint32_t skns_state::puzzloopj_speedup_r()
{
	if (m_maincpu->pc()==0x401dca0) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x86714/4];
}

uint32_t skns_state::puzzloopa_speedup_r()
{
	if (m_maincpu->pc()==0x401d9d4) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x85bcc/4];
}

uint32_t skns_state::puzzloopu_speedup_r()
{
	if (m_maincpu->pc()==0x401dab0) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x85cec/4];
}

uint32_t skns_state::puzzloope_speedup_r()
{
/*
    0401DA12: MOV.L   @($80,PC),R1
    0401DA14: MOV.L   @R1,R0 (R1=0x6081d38)
    0401DA16: TST     R0,R0
    0401DA18: BF      $0401DA26
    0401DA26: BRA     $0401DA12
*/
	if (m_maincpu->pc()==0x401da14) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x81d38/4];
}

uint32_t skns_state::senknow_speedup_r()
{
	if (m_maincpu->pc()==0x4017dce) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x0000dc/4];
}

uint32_t skns_state::teljan_speedup_r()
{
	if (m_maincpu->pc()==0x401ba32) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x002fb4/4];
}

uint32_t skns_state::jjparads_speedup_r()
{
	if (m_maincpu->pc()==0x4015e84) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x000994/4];
}

uint32_t skns_state::jjparad2_speedup_r()
{
	if (m_maincpu->pc()==0x401620a) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x000984/4];
}

uint32_t skns_state::ryouran_speedup_r()
{
	if (m_maincpu->pc()==0x40182ce) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x000a14/4];
}

uint32_t skns_state::galpans2_speedup_r()
{
	if (m_maincpu->pc()==0x4049ae2) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x0fb6bc/4];
}

uint32_t skns_state::panicstr_speedup_r()
{
	if (m_maincpu->pc()==0x404e68a) m_maincpu->spin_until_interrupt();
	return m_main_ram[0x0f19e4/4];
}

uint32_t skns_state::sengekis_speedup_r()// 60006ee  600308e
{
	if (m_maincpu->pc()==0x60006ec) m_maincpu->spin_until_interrupt();
	return m_main_ram[0xb74bc/4];
}

uint32_t skns_state::sengekij_speedup_r()// 60006ee  600308e
{
	if (m_maincpu->pc()==0x60006ec) m_maincpu->spin_until_interrupt();
	return m_main_ram[0xb7380/4];
}

void skns_state::init_drc()
{
	// init DRC to fastest options
	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);
	m_maincpu->sh2drc_add_fastram(0x02000000, 0x02003fff, 0, &m_spriteram[0]);
	m_maincpu->sh2drc_add_fastram(0x02100000, 0x0210003f, 0, &m_spc_regs[0]);
	m_maincpu->sh2drc_add_fastram(0x02600000, 0x02607fff, 0, &m_v3slc_ram[0]);
}

void skns_state::set_drc_pcflush(uint32_t addr)
{
	m_maincpu->sh2drc_add_pcflush(addr);
}

void skns_state::init_galpani4()   { m_spritegen->skns_sprite_kludge(-5,-1); init_drc();  }
void skns_state::init_galpanis()   { m_spritegen->skns_sprite_kludge(-5,-1); init_drc();  }
void skns_state::init_cyvern()     { m_spritegen->skns_sprite_kludge(+0,+2); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x604d3c8, 0x604d3cb, read32smo_delegate(*this, FUNC(skns_state::cyvern_speedup_r)) );  set_drc_pcflush(0x402ebd2);  }
void skns_state::init_galpans2()   { m_spritegen->skns_sprite_kludge(-1,-1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x60fb6bc, 0x60fb6bf, read32smo_delegate(*this, FUNC(skns_state::galpans2_speedup_r)) ); set_drc_pcflush(0x4049ae2); }
void skns_state::init_gutsn()      { m_spritegen->skns_sprite_kludge(+0,+0); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x600c780, 0x600c783, read32smo_delegate(*this, FUNC(skns_state::gutsn_speedup_r)) ); set_drc_pcflush(0x402206e); }
void skns_state::init_panicstr()   { m_spritegen->skns_sprite_kludge(-1,-1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x60f19e4, 0x60f19e7, read32smo_delegate(*this, FUNC(skns_state::panicstr_speedup_r)) ); set_drc_pcflush(0x404e68a);  }
void skns_state::init_senknow()    { m_spritegen->skns_sprite_kludge(+1,+1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x60000dc, 0x60000df, read32smo_delegate(*this, FUNC(skns_state::senknow_speedup_r)) ); set_drc_pcflush(0x4017dce);  }
void skns_state::init_puzzloope()  { m_spritegen->skns_sprite_kludge(-9,-1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x6081d38, 0x6081d3b, read32smo_delegate(*this, FUNC(skns_state::puzzloope_speedup_r)) ); set_drc_pcflush(0x401da14); }
void skns_state::init_puzzloopj()  { m_spritegen->skns_sprite_kludge(-9,-1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x6086714, 0x6086717, read32smo_delegate(*this, FUNC(skns_state::puzzloopj_speedup_r)) ); set_drc_pcflush(0x401dca0); }
void skns_state::init_puzzloopa()  { m_spritegen->skns_sprite_kludge(-9,-1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x6085bcc, 0x6085bcf, read32smo_delegate(*this, FUNC(skns_state::puzzloopa_speedup_r)) ); set_drc_pcflush(0x401d9d4); }
void skns_state::init_puzzloopu()  { m_spritegen->skns_sprite_kludge(-9,-1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x6085cec, 0x6085cef, read32smo_delegate(*this, FUNC(skns_state::puzzloopu_speedup_r)) ); set_drc_pcflush(0x401dab0); }
void skns_state::init_jjparads()   { m_spritegen->skns_sprite_kludge(+5,+1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000994, 0x6000997, read32smo_delegate(*this, FUNC(skns_state::jjparads_speedup_r)) ); set_drc_pcflush(0x4015e84); }
void skns_state::init_jjparad2()   { m_spritegen->skns_sprite_kludge(+5,+1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000984, 0x6000987, read32smo_delegate(*this, FUNC(skns_state::jjparad2_speedup_r)) ); set_drc_pcflush(0x401620a); }
void skns_state::init_ryouran()    { m_spritegen->skns_sprite_kludge(+5,+1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000a14, 0x6000a17, read32smo_delegate(*this, FUNC(skns_state::ryouran_speedup_r)) );  set_drc_pcflush(0x40182ce); }
void skns_state::init_teljan()     { m_spritegen->skns_sprite_kludge(+5,+1); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x6002fb4, 0x6002fb7, read32smo_delegate(*this, FUNC(skns_state::teljan_speedup_r)) ); set_drc_pcflush(0x401ba32); }
void skns_state::init_sengekis()   { m_spritegen->skns_sprite_kludge(-192,-272); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x60b74bc, 0x60b74bf, read32smo_delegate(*this, FUNC(skns_state::sengekis_speedup_r)) ); set_drc_pcflush(0x60006ec); }
void skns_state::init_sengekij()   { m_spritegen->skns_sprite_kludge(-192,-272); init_drc();m_maincpu->space(AS_PROGRAM).install_read_handler(0x60b7380, 0x60b7383, read32smo_delegate(*this, FUNC(skns_state::sengekij_speedup_r)) ); set_drc_pcflush(0x60006ec); }
void skns_state::init_sarukani()   { m_spritegen->skns_sprite_kludge(-1,-1); init_drc(); set_drc_pcflush(0x4013b42); } // Speedup is in io_w()
void skns_state::init_galpans3()   { m_spritegen->skns_sprite_kludge(-1,-1); init_drc();  }



/***** ROM LOADING *****/

// maybe we should treat each motherboard region as a separate parent / root?

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))

 /* NOTE: The Euro BIOS ROM has been found labeled SKNSE1 and SKNSE2 but the data is the same */
#define SKNS_BIOS \
	ROM_REGION( 0x0100000, "maincpu", 0 ) \
	ROM_SYSTEM_BIOS( 0, "japan", "Japan" ) \
	ROM_LOAD_BIOS( 0, "sknsj1.u10", 0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) )  \
	ROM_SYSTEM_BIOS( 1, "europe", "Europe" ) \
	ROM_LOAD_BIOS( 1, "sknse2.u10", 0x000000, 0x080000, CRC(e2b9d7d1) SHA1(b530a3bb9dedc8cfafcba9f1f10277590be04a15) )  \
	ROM_SYSTEM_BIOS( 2, "asia", "Asia" ) \
	ROM_LOAD_BIOS( 2, "sknsa1.u10", 0x000000, 0x080000, CRC(745e5212) SHA1(caba649ab2d83b2d7e007eecee0fc582c019df38) )  \
	ROM_SYSTEM_BIOS( 3, "usa", "USA" ) \
	ROM_LOAD_BIOS( 3, "sknsu1.u10", 0x000000, 0x080000, CRC(384d21ec) SHA1(a27e8a18099d9cea64fa32db28d01101c2a78815) )  \
	ROM_SYSTEM_BIOS( 4, "korea", "Korea" ) \
	ROM_LOAD_BIOS( 4, "sknsk1.u10", 0x000000, 0x080000, CRC(ff1c9f79) SHA1(a51e598d43e76d37da69b1f094c111273bdfc94a) )  \
	ROM_SYSTEM_BIOS( 5, "japanmod", "Japan (No Region Lock)" ) /* hack */ \
	ROM_LOAD_BIOS( 5, "supernova_modbios-japan.u10", 0x000000, 0x080000, CRC(b8d3190c) SHA1(62c9a4a075fd944e89fe95c6b46046101eb6de1c) ) \
	ROM_SYSTEM_BIOS( 6, "koreamod", "Korea (No Region Lock)" ) /* hack */ \
	ROM_LOAD_BIOS( 6, "supernova-modbios-korea.u10", 0x000000, 0x080000, CRC(1d90517c) SHA1(463962ffe19880135f0d95a7beda79e27448e872) )



#define SKNS_JAPAN \
	SKNS_BIOS \
	ROM_DEFAULT_BIOS( "japan" )

#define SKNS_EUROPE \
	SKNS_BIOS \
	ROM_DEFAULT_BIOS( "europe" )

#define SKNS_ASIA \
	SKNS_BIOS \
	ROM_DEFAULT_BIOS( "asia" )

#define SKNS_USA \
	SKNS_BIOS \
	ROM_DEFAULT_BIOS( "usa" )

#define SKNS_KOREA \
	SKNS_BIOS \
	ROM_DEFAULT_BIOS( "korea" )


ROM_START( skns )
	SKNS_BIOS

	ROM_REGION32_BE( 0x200000, "user1", ROMREGION_ERASE00 ) // SH-2 Code mapped at 0x04000000

	ROM_REGION( 0x800000, "spritegen", ROMREGION_ERASE00 ) // Sprites

	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00 ) // Tiles Plane A

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B

	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) // Samples
ROM_END


ROM_START( cyvern )
	SKNS_USA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "cv-usa.u10", 0x000000, 0x100000, CRC(1023ddca) SHA1(7967e3e876cdb797bdaa2eb5136a33cd43941501) )
	ROM_LOAD16_BYTE( "cv-usa.u8",  0x000001, 0x100000, CRC(f696f6be) SHA1(d9e66173ca12693255d2bb0982da2fb96bfd155d) )

	ROM_REGION( 0x800000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "cv100-00.u24", 0x000000, 0x400000, CRC(cd4ae88a) SHA1(925f4ae01a6ad3633be2a61be69e163f05401cf6) )
	ROM_LOAD( "cv101-00.u20", 0x400000, 0x400000, CRC(a6cb3f0b) SHA1(8d83f44a096ca0a70962ca4c602c4331874c8560) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "cv200-00.u16", 0x000000, 0x400000, CRC(ddc8c67e) SHA1(9b99e87e69e88011e6d693d19ac5e115b4fa50b0) )
	ROM_LOAD( "cv201-00.u13", 0x400000, 0x400000, CRC(65863321) SHA1(b8b75f50406068ffc3fca3887d2f0a653ca491c9) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "cv210-00.u18", 0x400000, 0x400000, CRC(7486bf3a) SHA1(3b4285ca570e9c5ad396c615bfc054372d1b0162) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "cv300-00.u4", 0x000000, 0x400000, CRC(fbeda465) SHA1(4d5066a22f4589b6b7f85b3e77c348d900ac4bdd) )
ROM_END

ROM_START( cyvernj )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "cvj-even.u10", 0x000000, 0x100000, CRC(802fadb4) SHA1(cbfac3a87a4863466117c61f2ecaf63d506352f6) )
	ROM_LOAD16_BYTE( "cvj-odd.u8",   0x000001, 0x100000, CRC(f8a0fbdd) SHA1(5cc8c12c13b5eb3456083e70100450ba041de76e) )

	ROM_REGION( 0x800000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "cv100-00.u24", 0x000000, 0x400000, CRC(cd4ae88a) SHA1(925f4ae01a6ad3633be2a61be69e163f05401cf6) )
	ROM_LOAD( "cv101-00.u20", 0x400000, 0x400000, CRC(a6cb3f0b) SHA1(8d83f44a096ca0a70962ca4c602c4331874c8560) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "cv200-00.u16", 0x000000, 0x400000, CRC(ddc8c67e) SHA1(9b99e87e69e88011e6d693d19ac5e115b4fa50b0) )
	ROM_LOAD( "cv201-00.u13", 0x400000, 0x400000, CRC(65863321) SHA1(b8b75f50406068ffc3fca3887d2f0a653ca491c9) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "cv210-00.u18", 0x400000, 0x400000, CRC(7486bf3a) SHA1(3b4285ca570e9c5ad396c615bfc054372d1b0162) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "cv300-00.u4", 0x000000, 0x400000, CRC(fbeda465) SHA1(4d5066a22f4589b6b7f85b3e77c348d900ac4bdd) )
ROM_END

ROM_START( galpani4 )
	SKNS_EUROPE

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gp4-000-e0.u10", 0x000000, 0x080000, CRC(7464cc28) SHA1(4ea6185b62d3e25efdaafb91397b51bc8f12fdee) ) // Hitachi HN27C4001G-10
	ROM_LOAD16_BYTE( "gp4-001-e0.u8",  0x000001, 0x080000, CRC(8d162069) SHA1(1bd0181a9c9f37c8e8c9d6f75f045d76dffcd903) ) // Hitachi HN27C4001G-10

	ROM_REGION( 0x400000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gp4-100-00.u24", 0x000000, 0x200000, CRC(1df61f01) SHA1(a9e95bbb3013e8f2fd01243b1b392ff07b4f7d02) )
	ROM_LOAD( "gp4-101-00.u20", 0x200000, 0x100000, CRC(8e2c9349) SHA1(a58fa9bcc9684ed4558e3395d592b64a1978a902) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gp4-200-00.u16", 0x000000, 0x200000, CRC(f0781376) SHA1(aeab9553a9af922524e528eb2d019cf36b6e2094) )
	ROM_LOAD( "gp4-201-00.u18", 0x200000, 0x200000, CRC(10c4b183) SHA1(80e05f3932495ad4fc9bf928fa66e6d2931bbb06) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gp4-300-00.u4", 0x000000, 0x200000, CRC(8374663a) SHA1(095512564f4de25dc3752d9fbd254b9dabd16d1b) ) // Doesn't seem to use these samples at all
	ROM_LOAD( "gp4-301-00.u7", 0x200000, 0x200000, CRC(53e9f8fb) SHA1(72104f31089f42cd3384d9494952f33b15cd18ad) ) // Changed some samples when compared to U4 ROM - Different then GP4-301-01 used below

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "skns-r09.u9",  0x000, 0x117, CRC(b02058d9) SHA1(77d07e0f329fb1969aa4543cd124e36ad34b07ba) ) // Atmel ATF16V8B
	ROM_LOAD( "skns-r11.u11", 0x200, 0x117, CRC(a9f05af4) SHA1(018684c1f9f7c2e1c86f0cb2db2ec7fb02e35cd8) ) // Atmel ATF16V8B
ROM_END

ROM_START( galpani4j )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gp4j1.u10", 0x000000, 0x080000, CRC(919a3893) SHA1(83b89a9e628a1f46f8a56ea512fc8ad641d5e239) )
	ROM_LOAD16_BYTE( "gp4j1.u8",  0x000001, 0x080000, CRC(94cb1fb7) SHA1(ac90103dd43cdce6a287ffc13631c1de477a9a71) )

	ROM_REGION( 0x400000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gp4-100-00.u24", 0x000000, 0x200000, CRC(1df61f01) SHA1(a9e95bbb3013e8f2fd01243b1b392ff07b4f7d02) )
	ROM_LOAD( "gp4-101-00.u20", 0x200000, 0x100000, CRC(8e2c9349) SHA1(a58fa9bcc9684ed4558e3395d592b64a1978a902) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gp4-200-00.u16", 0x000000, 0x200000, CRC(f0781376) SHA1(aeab9553a9af922524e528eb2d019cf36b6e2094) )
	ROM_LOAD( "gp4-201-00.u18", 0x200000, 0x200000, CRC(10c4b183) SHA1(80e05f3932495ad4fc9bf928fa66e6d2931bbb06) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gp4-300-00.u4", 0x000000, 0x200000, CRC(8374663a) SHA1(095512564f4de25dc3752d9fbd254b9dabd16d1b) )
ROM_END

ROM_START( galpani4k ) /* ROM-BOARD NEP-16 part number GP04K00372 with extra sound sample ROM at U7 */
	SKNS_KOREA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gp4k1.u10", 0x000000, 0x080000, CRC(cbd5c3a0) SHA1(17fc0d6f6050ffd31707cee3fcc263cd5b9d0c4f) )
	ROM_LOAD16_BYTE( "gp4k1.u8",  0x000001, 0x080000, CRC(7a95bfe2) SHA1(82e24fd4674ec25bc6608ced0921e8573fcff2c2) )

	ROM_REGION( 0x400000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gp4-100-00.u24", 0x000000, 0x200000, CRC(1df61f01) SHA1(a9e95bbb3013e8f2fd01243b1b392ff07b4f7d02) )
	ROM_LOAD( "gp4-101-00.u20", 0x200000, 0x100000, CRC(8e2c9349) SHA1(a58fa9bcc9684ed4558e3395d592b64a1978a902) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gp4-200-00.u16", 0x000000, 0x200000, CRC(f0781376) SHA1(aeab9553a9af922524e528eb2d019cf36b6e2094) )
	ROM_LOAD( "gp4-201-00.u18", 0x200000, 0x200000, CRC(10c4b183) SHA1(80e05f3932495ad4fc9bf928fa66e6d2931bbb06) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gp4-300-00.u4", 0x000000, 0x200000, CRC(8374663a) SHA1(095512564f4de25dc3752d9fbd254b9dabd16d1b) ) // Doesn't seem to use these samples at all
	ROM_LOAD( "gp4-301-01.u7", 0x200000, 0x200000, CRC(886ef77f) SHA1(047d5fecf2034339c69b2cb605b623a814a18f0d) ) // Changed some samples when compared to U4 ROM
ROM_END

/*
The Gals Panic EX version is very strange.

Found on a green cart which is generally known to be for Asian region.  It boots with the Asia BIOS and machine.
But it's clearly meant for Korea as it shows a Korean region warning and KA+cat Korean version of Kaneko
*/
ROM_START( galpaniex ) /* ROM-BOARD NEP-16 part number GP04K01334 with extra sound sample ROM at U7 */
	SKNS_ASIA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gpex_even_u10_d804_2001-2-6.u10", 0x000000, 0x100000, CRC(d3b504e6) SHA1(c5b3e06ee35386db5e0032b4cb7d49b7fbbcf531) ) // GPEX  even U10  2001/2/6    D804
	ROM_LOAD16_BYTE( "gpex_odd_u8_14c6_2001-2-6.u8",    0x000001, 0x100000, CRC(f3fe305a) SHA1(99d2f4e0857ebffd93674de22fb2836353f68974) ) // GPEX   odd U8   2001/2/6    14C6

	ROM_REGION( 0x400000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gp4-100-00.u24", 0x000000, 0x200000, CRC(1df61f01) SHA1(a9e95bbb3013e8f2fd01243b1b392ff07b4f7d02) )
	ROM_LOAD( "gp4-101-00.u20", 0x200000, 0x100000, CRC(8e2c9349) SHA1(a58fa9bcc9684ed4558e3395d592b64a1978a902) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gp4-200-00.u16", 0x000000, 0x200000, CRC(f0781376) SHA1(aeab9553a9af922524e528eb2d019cf36b6e2094) )
	ROM_LOAD( "gp4-201-00.u18", 0x200000, 0x200000, CRC(10c4b183) SHA1(80e05f3932495ad4fc9bf928fa66e6d2931bbb06) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gp4-300-00.u4", 0x000000, 0x200000, CRC(8374663a) SHA1(095512564f4de25dc3752d9fbd254b9dabd16d1b) ) // Doesn't seem to use these samples at all
	ROM_LOAD( "gp4-301-01.u7", 0x200000, 0x200000, CRC(886ef77f) SHA1(047d5fecf2034339c69b2cb605b623a814a18f0d) ) // Changed some samples when compared to U4 ROM
ROM_END

ROM_START( galpanidx )
	SKNS_ASIA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gpdx.u10", 0x000000, 0x100000, CRC(8eca883d) SHA1(f5a102ac21aeebc44afcd0ef7f655de21d4442b2) )
	ROM_LOAD16_BYTE( "gpdx.u8",  0x000001, 0x100000, CRC(b0088d8f) SHA1(2363620265d3fc53bae1c5889ea761444f80735a) )

	ROM_REGION( 0x400000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gp4-100-00.u24", 0x000000, 0x200000, CRC(1df61f01) SHA1(a9e95bbb3013e8f2fd01243b1b392ff07b4f7d02) )
	ROM_LOAD( "gp4-101-00.u20", 0x200000, 0x100000, CRC(8e2c9349) SHA1(a58fa9bcc9684ed4558e3395d592b64a1978a902) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gp4-200-00.u16", 0x000000, 0x200000, CRC(f0781376) SHA1(aeab9553a9af922524e528eb2d019cf36b6e2094) )
	ROM_LOAD( "gp4-201-00.u18", 0x200000, 0x200000, CRC(10c4b183) SHA1(80e05f3932495ad4fc9bf928fa66e6d2931bbb06) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gp4-300-00.u4", 0x000000, 0x200000, CRC(8374663a) SHA1(095512564f4de25dc3752d9fbd254b9dabd16d1b) ) // Doesn't seem to use these samples at all
	ROM_LOAD( "gp4-301-01.u7", 0x200000, 0x200000, CRC(886ef77f) SHA1(047d5fecf2034339c69b2cb605b623a814a18f0d) ) // Changed some samples when compared to U4 ROM
ROM_END

ROM_START( galpanisu )
	SKNS_KOREA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gpsu_e_1q07_00.6.5.u10", 0x000000, 0x100000, CRC(3bf4c440) SHA1(88941ee62a1d975c4a848e6fa79c2757afe47833) ) // handwritten label
	ROM_LOAD16_BYTE( "gpsu_o_16d1_00.6.5.u8",  0x000001, 0x100000, CRC(d4fef828) SHA1(ccf5c62ad3a5b54ea35a421cfe9d402a4a2dda6c) ) // handwritten label

	ROM_REGION( 0x400000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gp4-100-00.u24", 0x000000, 0x200000, CRC(1df61f01) SHA1(a9e95bbb3013e8f2fd01243b1b392ff07b4f7d02) )
	ROM_LOAD( "gp4-101-00.u20", 0x200000, 0x100000, CRC(8e2c9349) SHA1(a58fa9bcc9684ed4558e3395d592b64a1978a902) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gp4-200-00.u16", 0x000000, 0x200000, CRC(f0781376) SHA1(aeab9553a9af922524e528eb2d019cf36b6e2094) )
	ROM_LOAD( "gp4-201-00.u18", 0x200000, 0x200000, CRC(10c4b183) SHA1(80e05f3932495ad4fc9bf928fa66e6d2931bbb06) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gp4-300-00.u4", 0x000000, 0x200000, CRC(8374663a) SHA1(095512564f4de25dc3752d9fbd254b9dabd16d1b) )
	ROM_LOAD( "gp4-301-01.u7", 0x200000, 0x200000, CRC(886ef77f) SHA1(047d5fecf2034339c69b2cb605b623a814a18f0d) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "skns-r09.u9",  0x000, 0x117, CRC(b02058d9) SHA1(77d07e0f329fb1969aa4543cd124e36ad34b07ba) )
	ROM_LOAD( "skns-r11.u11", 0x200, 0x117, CRC(a9f05af4) SHA1(018684c1f9f7c2e1c86f0cb2db2ec7fb02e35cd8) )
ROM_END

ROM_START( galpanis )
	SKNS_EUROPE

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gps-000-e1.u10", 0x000000, 0x100000, CRC(b9ea3c44) SHA1(c1913545cd71ee75e60ade744a2a1054f770b981) ) // revision 1
	ROM_LOAD16_BYTE( "gps-001-e1.u8",  0x000001, 0x100000, CRC(ded57bd0) SHA1(4c0122f0521829d4d83b6b1c403f7e6470f14951) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gps-100-00.u24", 0x000000, 0x400000, CRC(a1a7acf2) SHA1(52c86ae907f0c0236808c19f652955b09e90ec5a) )
	ROM_LOAD( "gps-101-00.u20", 0x400000, 0x400000, CRC(49f764b6) SHA1(9f4289858c3dac625ef623cc381a47b45aa5d8e2) )
	ROM_LOAD( "gps-102-00.u17", 0x800000, 0x400000, CRC(51980272) SHA1(6c0706d913b33995579aaf0688c4bf26d6d35a78) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gps-200-00.u16", 0x000000, 0x400000, CRC(c146a09e) SHA1(5af5a7b9d9a55ec7aba3fd85a3a0211b92b1b84f) )
	ROM_LOAD( "gps-201-00.u13", 0x400000, 0x400000, CRC(9dfa2dc6) SHA1(a058c42fd76c23c0e5c8c11f5617fd29e056be7d) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gps-300-00.u4", 0x000000, 0x400000, CRC(9e4da8e3) SHA1(6506d9300a442883357003a05fd2c78d364c35bb) )
ROM_END

ROM_START( galpanise )
	SKNS_EUROPE

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "u10", 0x000000, 0x100000, CRC(e78e1623) SHA1(f68346b65d2613c8515894d9a239fcbb0b5cb52d) ) // mask ROM with no labels - GPS-000-E0?
	ROM_LOAD16_BYTE( "u8",  0x000001, 0x100000, CRC(098eff7c) SHA1(3cac22cbb11905a46afaa62c0470624b3b554fc0) ) // mask ROM with no labels - GPS-001-E0?

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gps-100-00.u24", 0x000000, 0x400000, CRC(a1a7acf2) SHA1(52c86ae907f0c0236808c19f652955b09e90ec5a) )
	ROM_LOAD( "gps-101-00.u20", 0x400000, 0x400000, CRC(49f764b6) SHA1(9f4289858c3dac625ef623cc381a47b45aa5d8e2) )
	ROM_LOAD( "gps-102-00.u17", 0x800000, 0x400000, CRC(51980272) SHA1(6c0706d913b33995579aaf0688c4bf26d6d35a78) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gps-200-00.u16", 0x000000, 0x400000, CRC(c146a09e) SHA1(5af5a7b9d9a55ec7aba3fd85a3a0211b92b1b84f) )
	ROM_LOAD( "gps-201-00.u13", 0x400000, 0x400000, CRC(9dfa2dc6) SHA1(a058c42fd76c23c0e5c8c11f5617fd29e056be7d) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gps-300-00.u4", 0x000000, 0x400000, CRC(9e4da8e3) SHA1(6506d9300a442883357003a05fd2c78d364c35bb) )
ROM_END

ROM_START( galpanisj )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gps-000-j1.u10", 0x000000, 0x100000, CRC(c6938c3f) SHA1(05853ee6a44a55702788a75580b04a4be45e9bcb) )
	ROM_LOAD16_BYTE( "gps-001-j1.u8",  0x000001, 0x100000, CRC(e764177a) SHA1(3a1333eb1022ed1a275b9c3d44b5f4ab81618fb6) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gps-100-00.u24", 0x000000, 0x400000, CRC(a1a7acf2) SHA1(52c86ae907f0c0236808c19f652955b09e90ec5a) )
	ROM_LOAD( "gps-101-00.u20", 0x400000, 0x400000, CRC(49f764b6) SHA1(9f4289858c3dac625ef623cc381a47b45aa5d8e2) )
	ROM_LOAD( "gps-102-00.u17", 0x800000, 0x400000, CRC(51980272) SHA1(6c0706d913b33995579aaf0688c4bf26d6d35a78) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gps-200-00.u16", 0x000000, 0x400000, CRC(c146a09e) SHA1(5af5a7b9d9a55ec7aba3fd85a3a0211b92b1b84f) )
	ROM_LOAD( "gps-201-00.u13", 0x400000, 0x400000, CRC(9dfa2dc6) SHA1(a058c42fd76c23c0e5c8c11f5617fd29e056be7d) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gps-300-00.u4", 0x000000, 0x400000, CRC(9e4da8e3) SHA1(6506d9300a442883357003a05fd2c78d364c35bb) )
ROM_END

ROM_START( galpanisa )
	SKNS_ASIA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gps-000-a0_9abc.u10", 0x000000, 0x100000, CRC(4e24b799) SHA1(614f4eb6a7b0ab03ea6ada28a670ed0759b3f4f9) ) // hand written labels with checksum
	ROM_LOAD16_BYTE( "gps-001-a0_bd64.u8",  0x000001, 0x100000, CRC(aa4db8af) SHA1(10cc15fa065b6a2dcaf8c7d701c5ae7c18e4e863) ) // hand written labels with checksum

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gps-100-00.u24", 0x000000, 0x400000, CRC(a1a7acf2) SHA1(52c86ae907f0c0236808c19f652955b09e90ec5a) )
	ROM_LOAD( "gps-101-00.u20", 0x400000, 0x400000, CRC(49f764b6) SHA1(9f4289858c3dac625ef623cc381a47b45aa5d8e2) )
	ROM_LOAD( "gps-102-00.u17", 0x800000, 0x400000, CRC(51980272) SHA1(6c0706d913b33995579aaf0688c4bf26d6d35a78) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gps-200-00.u16", 0x000000, 0x400000, CRC(c146a09e) SHA1(5af5a7b9d9a55ec7aba3fd85a3a0211b92b1b84f) )
	ROM_LOAD( "gps-201-00.u13", 0x400000, 0x400000, CRC(9dfa2dc6) SHA1(a058c42fd76c23c0e5c8c11f5617fd29e056be7d) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gps-300-00.u4", 0x000000, 0x400000, CRC(9e4da8e3) SHA1(6506d9300a442883357003a05fd2c78d364c35bb) )
ROM_END

ROM_START( galpanisk )
	SKNS_KOREA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gps-000-k1.u10", 0x000000, 0x100000, CRC(c9ff3d8a) SHA1(edfec265654aaa8cb307424e5b2899e708392cd0) ) // revision 1
	ROM_LOAD16_BYTE( "gps-001-k1.u8",  0x000001, 0x100000, CRC(354e601d) SHA1(4d176f2337a3b0b63548b2e542f9fa87d0a1ef7b) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gps-100-00.u24", 0x000000, 0x400000, CRC(a1a7acf2) SHA1(52c86ae907f0c0236808c19f652955b09e90ec5a) )
	ROM_LOAD( "gps-101-00.u20", 0x400000, 0x400000, CRC(49f764b6) SHA1(9f4289858c3dac625ef623cc381a47b45aa5d8e2) )
	ROM_LOAD( "gps-102-00.u17", 0x800000, 0x400000, CRC(51980272) SHA1(6c0706d913b33995579aaf0688c4bf26d6d35a78) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gps-200-00.u16", 0x000000, 0x400000, CRC(c146a09e) SHA1(5af5a7b9d9a55ec7aba3fd85a3a0211b92b1b84f) )
	ROM_LOAD( "gps-201-00.u13", 0x400000, 0x400000, CRC(9dfa2dc6) SHA1(a058c42fd76c23c0e5c8c11f5617fd29e056be7d) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gps-300-00.u4", 0x000000, 0x400000, CRC(9e4da8e3) SHA1(6506d9300a442883357003a05fd2c78d364c35bb) )
ROM_END

ROM_START( galpaniska )
	SKNS_KOREA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gps-000-k0.u10", 0x000000, 0x100000, CRC(be74128c) SHA1(6d506e1bb715e6bfd98eda5e69475d2f75911d26) ) // revision 0 (first version)
	ROM_LOAD16_BYTE( "gps-001-k0.u8",  0x000001, 0x100000, CRC(701f793d) SHA1(7278dc68d5b92a26675333ef8b4b2c04c27f1b03) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gps-100-00.u24", 0x000000, 0x400000, CRC(a1a7acf2) SHA1(52c86ae907f0c0236808c19f652955b09e90ec5a) )
	ROM_LOAD( "gps-101-00.u20", 0x400000, 0x400000, CRC(49f764b6) SHA1(9f4289858c3dac625ef623cc381a47b45aa5d8e2) )
	ROM_LOAD( "gps-102-00.u17", 0x800000, 0x400000, CRC(51980272) SHA1(6c0706d913b33995579aaf0688c4bf26d6d35a78) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gps-200-00.u16", 0x000000, 0x400000, CRC(c146a09e) SHA1(5af5a7b9d9a55ec7aba3fd85a3a0211b92b1b84f) )
	ROM_LOAD( "gps-201-00.u13", 0x400000, 0x400000, CRC(9dfa2dc6) SHA1(a058c42fd76c23c0e5c8c11f5617fd29e056be7d) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gps-300-00.u4", 0x000000, 0x400000, CRC(9e4da8e3) SHA1(6506d9300a442883357003a05fd2c78d364c35bb) )
ROM_END

ROM_START( galpans2 ) //only the 2 program ROMs were dumped, but mask ROMs are supposed to match.
	SKNS_EUROPE

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gps2e_u6__ver.3.u6", 0x000000, 0x100000, CRC(72fff5d1) SHA1(57001e04c469281a82a2956c6bc33502d5a3b882) )
	ROM_LOAD16_BYTE( "gps2e_u4__ver.3.u4", 0x000001, 0x100000, CRC(95061601) SHA1(f98f1af9877b097e97acc5a3844ef9c523a92843) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gs210000.u21", 0x000000, 0x400000, CRC(294b2f14) SHA1(90cbd0acdaa2d89d208c28aae33ab57c03624089) )
	ROM_LOAD( "gs210100.u20", 0x400000, 0x400000, CRC(f75c5a9a) SHA1(3919643cee6c88185a1aa3c58c5bc80599bf734e) )
	ROM_LOAD( "gs210200.u8",  0x800000, 0x400000, CRC(25b4f56b) SHA1(f9a33d5ed54a04ecece3035e75508d191bbe74b1) )
	ROM_LOAD( "gs210300.u32", 0xc00000, 0x400000, CRC(db6d4424) SHA1(0a88dafd0ee2490ff2ef39ce8eb1931c41bdda42) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gs220000.u17", 0x000000, 0x400000, CRC(5caae1c0) SHA1(8f77e4cf018d7290b2d804cbff9fccf0bf4d2404) )
	ROM_LOAD( "gs220100.u9",  0x400000, 0x400000, CRC(8d51f197) SHA1(19d2afab823ea179918e7bcbf4df2283e77570f0) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "gs221000.u3",  0x400000, 0x400000, CRC(58800a18) SHA1(5e6d55ecd12275662d6f59559e137b759f23fff6) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gs230000.u1",  0x000000, 0x400000, CRC(0348e8e1) SHA1(8a21c7e5cea0bc08a2595213d689c58c0251fdb5) )
ROM_END

ROM_START( galpans2j )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gps2j.u6", 0x000000, 0x100000, CRC(6e74005b) SHA1(a57e8307062e262c2e7a84e2c58f7dfe03fc0f78) )
	ROM_LOAD16_BYTE( "gps2j.u4", 0x000001, 0x100000, CRC(9b4b2304) SHA1(0b481f4d71d92bf23f38ed22979efd4409004857) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gs210000.u21", 0x000000, 0x400000, CRC(294b2f14) SHA1(90cbd0acdaa2d89d208c28aae33ab57c03624089) )
	ROM_LOAD( "gs210100.u20", 0x400000, 0x400000, CRC(f75c5a9a) SHA1(3919643cee6c88185a1aa3c58c5bc80599bf734e) )
	ROM_LOAD( "gs210200.u8",  0x800000, 0x400000, CRC(25b4f56b) SHA1(f9a33d5ed54a04ecece3035e75508d191bbe74b1) )
	ROM_LOAD( "gs210300.u32", 0xc00000, 0x400000, CRC(db6d4424) SHA1(0a88dafd0ee2490ff2ef39ce8eb1931c41bdda42) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gs220000.u17", 0x000000, 0x400000, CRC(5caae1c0) SHA1(8f77e4cf018d7290b2d804cbff9fccf0bf4d2404) )
	ROM_LOAD( "gs220100.u9",  0x400000, 0x400000, CRC(8d51f197) SHA1(19d2afab823ea179918e7bcbf4df2283e77570f0) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "gs221000.u3",  0x400000, 0x400000, CRC(58800a18) SHA1(5e6d55ecd12275662d6f59559e137b759f23fff6) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gs230000.u1",  0x000000, 0x400000, CRC(0348e8e1) SHA1(8a21c7e5cea0bc08a2595213d689c58c0251fdb5) )
ROM_END

ROM_START( galpans2a )
	SKNS_ASIA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gps2av11.u6", 0x000000, 0x100000, CRC(61c05d5f) SHA1(e47c7951c1f688edb6c677532f750537a64bb7b3) )
	ROM_LOAD16_BYTE( "gps2av11.u4", 0x000001, 0x100000, CRC(2e8c0ac2) SHA1(d066260d6d3c2924b42394e867523e6112a125c5) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gs210000.u21", 0x000000, 0x400000, CRC(294b2f14) SHA1(90cbd0acdaa2d89d208c28aae33ab57c03624089) )
	ROM_LOAD( "gs210100.u20", 0x400000, 0x400000, CRC(f75c5a9a) SHA1(3919643cee6c88185a1aa3c58c5bc80599bf734e) )
	ROM_LOAD( "gs210200.u8",  0x800000, 0x400000, CRC(25b4f56b) SHA1(f9a33d5ed54a04ecece3035e75508d191bbe74b1) )
	ROM_LOAD( "gs210300.u32", 0xc00000, 0x400000, CRC(db6d4424) SHA1(0a88dafd0ee2490ff2ef39ce8eb1931c41bdda42) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gs220000.u17", 0x000000, 0x400000, CRC(5caae1c0) SHA1(8f77e4cf018d7290b2d804cbff9fccf0bf4d2404) )
	ROM_LOAD( "gs220100.u9",  0x400000, 0x400000, CRC(8d51f197) SHA1(19d2afab823ea179918e7bcbf4df2283e77570f0) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "gs221000.u3",  0x400000, 0x400000, CRC(58800a18) SHA1(5e6d55ecd12275662d6f59559e137b759f23fff6) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gs230000.u1",  0x000000, 0x400000, CRC(0348e8e1) SHA1(8a21c7e5cea0bc08a2595213d689c58c0251fdb5) )
ROM_END

ROM_START( galpansu ) // standard Kaneko NOVA SYSTEM ROM 4 BOARD cart with genuine Kaneko labels & chips
	SKNS_KOREA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gps2k_u6_ver.5.u6", 0x000000, 0x100000, CRC(18ffb6d3) SHA1(73d1c5483ab93855c855c32807a70bf0c4185a1e) )
	ROM_LOAD16_BYTE( "gps2k_u4_ver.5.u4", 0x000001, 0x100000, CRC(211f3c7c) SHA1(06d3cb95ef7f1e55965b37fb837ec4f3317c83d4) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gs210000.u21", 0x000000, 0x400000, CRC(294b2f14) SHA1(90cbd0acdaa2d89d208c28aae33ab57c03624089) )
	ROM_LOAD( "gs210100.u20", 0x400000, 0x400000, CRC(f75c5a9a) SHA1(3919643cee6c88185a1aa3c58c5bc80599bf734e) )
	ROM_LOAD( "gs210200.u8",  0x800000, 0x400000, CRC(25b4f56b) SHA1(f9a33d5ed54a04ecece3035e75508d191bbe74b1) )
	ROM_LOAD( "gs210300.u32", 0xc00000, 0x400000, CRC(db6d4424) SHA1(0a88dafd0ee2490ff2ef39ce8eb1931c41bdda42) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gs220000.u17", 0x000000, 0x400000, CRC(5caae1c0) SHA1(8f77e4cf018d7290b2d804cbff9fccf0bf4d2404) )
	ROM_LOAD( "gs220100.u9",  0x400000, 0x400000, CRC(8d51f197) SHA1(19d2afab823ea179918e7bcbf4df2283e77570f0) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "gs221000.u3",  0x400000, 0x400000, CRC(58800a18) SHA1(5e6d55ecd12275662d6f59559e137b759f23fff6) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gs230000.u1",  0x000000, 0x400000, CRC(0348e8e1) SHA1(8a21c7e5cea0bc08a2595213d689c58c0251fdb5) )
ROM_END

/*

Gals Panic SU (Kaneko 1999)
Korean hacked version. Runs on Super Kaneko Nova System mainboard
and original Super Kaneko Nova System ROM board labelled "ROM-BOARD"
EPROMs at U8, U10 and mainboard U10 are new to this version.
The ROM board is wired to accept 16MBit SOP44 maskROMs.
The actual ROMs used are 32M. There are some wire mods to enable the
higher capacity ROMs, basically wiring pin 44 of the SOP44's to
some logic to enable it.
All of the SOP44 ROMs are from Gals Panic 2, but because Gals Panic 2
uses a different ROM board the Gals Panic SU ROMs are at different
locations.
For Gals Panic SU, the 32M ROMs can be taken from the existing
Gals Panic 2 set.

*/

ROM_START( galpansua )
	SKNS_KOREA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "su.u10", 0x000000, 0x100000, CRC(5ae66218) SHA1(c3f32603e1da945efb984ff99e1a30202e535773) )
	ROM_LOAD16_BYTE( "su.u8",  0x000001, 0x100000, CRC(10977a03) SHA1(2ab95398d6b88d8819f368ee6104d7f8b485778d) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "24", 0x000000, 0x400000, CRC(294b2f14) SHA1(90cbd0acdaa2d89d208c28aae33ab57c03624089) ) // == gs210000.u21
	ROM_LOAD( "20", 0x400000, 0x400000, CRC(f75c5a9a) SHA1(3919643cee6c88185a1aa3c58c5bc80599bf734e) ) // == gs210100.u20
	ROM_LOAD( "17", 0x800000, 0x400000, CRC(25b4f56b) SHA1(f9a33d5ed54a04ecece3035e75508d191bbe74b1) ) // == gs210200.u8
	ROM_LOAD( "32", 0xc00000, 0x400000, CRC(db6d4424) SHA1(0a88dafd0ee2490ff2ef39ce8eb1931c41bdda42) ) // == gs210300.u32

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "16", 0x000000, 0x400000, CRC(5caae1c0) SHA1(8f77e4cf018d7290b2d804cbff9fccf0bf4d2404) ) // == gs220000.u17
	ROM_LOAD( "13", 0x400000, 0x400000, CRC(8d51f197) SHA1(19d2afab823ea179918e7bcbf4df2283e77570f0) ) // == gs220100.u9

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "7",  0x400000, 0x400000, CRC(58800a18) SHA1(5e6d55ecd12275662d6f59559e137b759f23fff6) ) // == gs221000.u3

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "4",  0x000000, 0x400000, CRC(0348e8e1) SHA1(8a21c7e5cea0bc08a2595213d689c58c0251fdb5) ) // == gs230000.u1
ROM_END

ROM_START( galpans3 )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gpss3.u10", 0x000000, 0x100000, CRC(c1449a72) SHA1(02db81a0ea349742d6ddf71d59fcfce45f0c5212) )
	ROM_LOAD16_BYTE( "gpss3.u8",  0x000001, 0x100000, CRC(11eb44cf) SHA1(482ef27fa86d6777def46918eac8be019896c0b0) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "u24.bin", 0x000000, 0x800000, CRC(70613168) SHA1(637c50e733dbc0226b1e0acc8000faa7e8977cb6) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "u16.bin", 0x000000, 0x800000, CRC(a96daf2a) SHA1(40f4c32158d320146aeeac34c15ca6816a6876bc) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "u4.bin", 0x000000, 0x400000, CRC(bf5736c6) SHA1(781292d87e9da1d21c1ac540baefff5e2f84a3f5) )
ROM_END

ROM_START( gutsn )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "gts000j0.u6", 0x000000, 0x080000, CRC(8ee91310) SHA1(8dd918189fe445d79c7f028168862b852f70a6f2) )
	ROM_LOAD16_BYTE( "gts001j0.u4", 0x000001, 0x080000, CRC(80b8ee66) SHA1(4faf5f358ceee866f09bd81e63ba3ebd21bde835) )

	ROM_REGION( 0x400000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "gts10000.u24", 0x000000, 0x400000, CRC(1959979e) SHA1(92a68784664dd833ca6fcca1b15cd46b9365d081) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "gts20000.u16", 0x000000, 0x400000, CRC(c443aac3) SHA1(b0416a09ead26077e9276bae98d94eeb1cf86877) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "gts30000.u4", 0x000000, 0x400000, CRC(8c169141) SHA1(41caea6fa644515f7417c84bdac599b13ad07e8c) )
ROM_END

ROM_START( panicstr )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "ps1000j0.u10", 0x000000, 0x100000, CRC(59645f89) SHA1(8da205c6e38899d6c637941700dd7eea56011c10) )
	ROM_LOAD16_BYTE( "ps1001j0.u8",  0x000001, 0x100000, CRC(c4722be9) SHA1(7009d320a80cfa7d80efc5fc915081914bc3c827) )

	ROM_REGION( 0x800000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "ps-10000.u24", 0x000000, 0x400000, CRC(294b2f14) SHA1(90cbd0acdaa2d89d208c28aae33ab57c03624089) )
	ROM_LOAD( "ps110100.u20", 0x400000, 0x400000, CRC(e292f393) SHA1(b0914f7f0abf9f821f2592c289ea4e3b3e7f819a) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "ps120000.u16", 0x000000, 0x400000, CRC(d772ac15) SHA1(6bf7b9bfccdcb7481b21fa2ab9b683d79033a192) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "ps-30000.u4",  0x000000, 0x400000, CRC(2262e263) SHA1(73443e5f40f5c5c9bd41c6207fa6376072f0f65e) )
ROM_END

ROM_START( puzzloop )
	SKNS_EUROPE

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "pl00e4.u6", 0x000000, 0x080000, CRC(7d3131a5) SHA1(f9302aa27addb8a730102b1869a34063d8b44e62) ) // V0.94
	ROM_LOAD16_BYTE( "pl00e4.u4", 0x000001, 0x080000, CRC(40dc3291) SHA1(d955752a2c884e6dd951f9a87f9d249bb1ab9116) ) // V0.94

	ROM_REGION( 0x800000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

ROM_START( puzzloope )
	SKNS_EUROPE

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "pl00e1.u6", 0x000000, 0x080000, CRC(273adc38) SHA1(37ca873342ba9fb9951114048a9cd255f73fe19c) ) // V0.93
	ROM_LOAD16_BYTE( "pl00e1.u4", 0x000001, 0x080000, CRC(14ac2870) SHA1(d1abcfd64d7c0ead67e879c40e1010453fd4da13) ) // V0.93

	ROM_REGION( 0x800000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

ROM_START( puzzloopj )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "pl0j2.u6", 0x000000, 0x080000, CRC(23c3bf97) SHA1(77ea1f32bed5709a6ad5b250370f08cfe8036867) ) // V0.94
	ROM_LOAD16_BYTE( "pl0j2.u4", 0x000001, 0x080000, CRC(55b2a3cb) SHA1(d4cbe143fe2ad622af808cbd9eedffeff3b77e0d) ) // V0.94

	ROM_REGION( 0x800000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

ROM_START( puzzloopa )
	SKNS_ASIA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "pl0a3.u6", 0x000000, 0x080000, CRC(4e8673b8) SHA1(17acfb0550912e6f2519df2bc24fbf629a1f6147) ) // V0.94
	ROM_LOAD16_BYTE( "pl0a3.u4", 0x000001, 0x080000, CRC(e08a1a07) SHA1(aba58a81ae46c7b4e235a3213984026d170fa189) ) // V0.94

	ROM_REGION( 0x800000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

ROM_START( puzzloopk )
	SKNS_KOREA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "pl0k4.u6", 0x000000, 0x080000, CRC(8d81f20c) SHA1(c32a525e8f92a625e3fecb7c43dd04b13e0a75e4) ) // V0.94
	ROM_LOAD16_BYTE( "pl0k4.u4", 0x000001, 0x080000, CRC(17c78e41) SHA1(4a4b612ae00d521d2947ab32554ebb615be72471) ) // V0.94

	ROM_REGION( 0x800000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

ROM_START( puzzloopu )
	SKNS_USA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "plue5.u6", 0x000000, 0x080000, CRC(e6f3f82f) SHA1(ac61dc22fa3c1b1c2f3a41d3a8fb43938b77ca68) ) // V0.94
	ROM_LOAD16_BYTE( "plue5.u4", 0x000001, 0x080000, CRC(0d081d30) SHA1(ec0cdf120126104b9bb706f68c9ba9c3777dd69c) ) // V0.94

	ROM_REGION( 0x800000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

ROM_START( jjparads )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "jp1j1.u10", 0x000000, 0x080000, CRC(de2fb669) SHA1(229ff1ae0ec5bc77fc17642964e0bb0146594e86) )
	ROM_LOAD16_BYTE( "jp1j1.u8",  0x000001, 0x080000, CRC(7276efb1) SHA1(3edc265b5c02da7d21a2494a6dc2878fbad93f87) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "jp100-00.u24", 0x000000, 0x400000, CRC(f31b2e95) SHA1(7e5bb518d4f6423785d3f9f2752a624a66b42469) )
	ROM_LOAD( "jp101-00.u20", 0x400000, 0x400000, CRC(70cc8c24) SHA1(a4805ce19f512b047829548b635e68690d714175) )
	ROM_LOAD( "jp102-00.u17", 0x800000, 0x400000, CRC(35401c1e) SHA1(38fe86a08555bb823b8d64ac043330aaaa6b8892) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "jp200-00.u16", 0x000000, 0x200000, CRC(493d63db) SHA1(4b8fe7ff1ae14a914a675ce4072a4d9e5cfc08b0) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x200000, "ymz", 0 ) // Samples
	ROM_LOAD( "jp300-00.u4", 0x000000, 0x200000, CRC(7023fe46) SHA1(24a92133bc664d63b3be67c2ef11cd7b605ee7e8) )
ROM_END

ROM_START( jjparad2 )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "jp2000j1.u6", 0x000000, 0x080000, CRC(5d75e765) SHA1(33bcd8f929f6025b00df2ea783b13a391a28a5c3) )
	ROM_LOAD16_BYTE( "jp2001j1.u4", 0x000001, 0x080000, CRC(1771910a) SHA1(7ca9584d379d7b41f303a3ba861f943c570ad97c) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "jp210000.u21", 0x000000, 0x400000, CRC(79a7e3d7) SHA1(bd0f8d01971e5895395f97f2520bcd03ab19d229) )
	ROM_LOAD( "jp210100.u20", 0x400000, 0x400000, CRC(42415e0c) SHA1(f7bff86d55fa9002fbd14e4c62f9d3df8faaf7d0) )
	ROM_LOAD( "jp210200.u8",  0x800000, 0x400000, CRC(26731745) SHA1(8939d36b82b10b1010e4b924e6b9fdd4742efe48) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "jp220000.u17", 0x000000, 0x400000, CRC(d0e71873) SHA1(c6ffba3624e6d4c2d4e12ef7d88a02cbc3867b18) )
	ROM_LOAD( "jp220100.u9",  0x400000, 0x400000, CRC(4c7d964d) SHA1(3352cd866a64466f4f5a990c2c5e3e28e7028a99) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "jp230000.u1", 0x000000, 0x400000, CRC(73e30d7f) SHA1(af5b16cec722dbbf0e03d73edfa133dbf10ac4f3) )
ROM_END

ROM_START( sengekis )
	SKNS_ASIA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "ss01a.u6", 0x000000, 0x080000, CRC(962fe857) SHA1(3df74c5efff11333dea9316a063129dcec0d7bdd) )
	ROM_LOAD16_BYTE( "ss01a.u4", 0x000001, 0x080000, CRC(ee853c23) SHA1(ddbf7f7cf509788ee3daf7b4d8ae1482e6e31a03) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "ss100-00.u21", 0x000000, 0x400000, CRC(bc7b3dfa) SHA1(dff10a7aef548abda48470293382057a2ca9557e) )
	ROM_LOAD( "ss101-00.u20", 0x400000, 0x400000, CRC(ab2df280) SHA1(e456c578a36f585b24379d74def1bcab276c2b1b) )
	ROM_LOAD( "ss102-00.u8",  0x800000, 0x400000, CRC(0845eafe) SHA1(663b163bf4e87c7df0030e791f95b1a5827de315) )
	ROM_LOAD( "ss103-00.u32", 0xc00000, 0x400000, CRC(ee451ac9) SHA1(01cc6b6f371c0090a6a7f4c33d05f4b9a6c59fee) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "ss200-00.u17", 0x000000, 0x400000, CRC(cd773976) SHA1(38b8df5e685be65c3fde09f9e585591f678632d4) )
	ROM_LOAD( "ss201-00.u9",  0x400000, 0x400000, CRC(301fad4c) SHA1(15faf37eeec5cc46afcb4bd236345b5c3dd647ac) )

	ROM_REGION( 0x600000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "ss210-00.u3",  0x400000, 0x200000, CRC(c3697805) SHA1(bd41064e3527cdc4b9a4ab9c423c916309b3f057) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "ss300-00.u1", 0x000000, 0x400000, CRC(35b04b18) SHA1(b69f33fc6a50ec20382329317d20b3c1e7f01b87) )
ROM_END

ROM_START( sengekisj )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "ss01j.u6", 0x000000, 0x080000, CRC(9efdcd5a) SHA1(66cca04d07999dc8ca0bcf19db925996b34d0390) )
	ROM_LOAD16_BYTE( "ss01j.u4", 0x000001, 0x080000, CRC(92c3f45e) SHA1(60c647e66b0126fb7749874be39938972481b957) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "ss100-00.u21", 0x000000, 0x400000, CRC(bc7b3dfa) SHA1(dff10a7aef548abda48470293382057a2ca9557e) )
	ROM_LOAD( "ss101-00.u20", 0x400000, 0x400000, CRC(ab2df280) SHA1(e456c578a36f585b24379d74def1bcab276c2b1b) )
	ROM_LOAD( "ss102-00.u8",  0x800000, 0x400000, CRC(0845eafe) SHA1(663b163bf4e87c7df0030e791f95b1a5827de315) )
	ROM_LOAD( "ss103-00.u32", 0xc00000, 0x400000, CRC(ee451ac9) SHA1(01cc6b6f371c0090a6a7f4c33d05f4b9a6c59fee) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "ss200-00.u17", 0x000000, 0x400000, CRC(cd773976) SHA1(38b8df5e685be65c3fde09f9e585591f678632d4) )
	ROM_LOAD( "ss201-00.u9",  0x400000, 0x400000, CRC(301fad4c) SHA1(15faf37eeec5cc46afcb4bd236345b5c3dd647ac) )

	ROM_REGION( 0x600000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "ss210-00.u3",  0x400000, 0x200000, CRC(c3697805) SHA1(bd41064e3527cdc4b9a4ab9c423c916309b3f057) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "ss300-00.u1", 0x000000, 0x400000, CRC(35b04b18) SHA1(b69f33fc6a50ec20382329317d20b3c1e7f01b87) )
ROM_END

ROM_START( senknow )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "snw000j1.u6", 0x000000, 0x080000, CRC(0d6136f6) SHA1(eedd011cfe03577bfaf386723502d03f6e5dbd8c) )
	ROM_LOAD16_BYTE( "snw001j1.u4", 0x000001, 0x080000, CRC(4a10ec3d) SHA1(bbec4fc53bd61d06ffe5a53debada5785b124fdd) )

	ROM_REGION( 0x0800000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "snw10000.u21", 0x000000, 0x400000, CRC(5133c69c) SHA1(d279df3ffd005dbf0930a8e40eaf2467f8653284) )
	ROM_LOAD( "snw10100.u20", 0x400000, 0x400000, CRC(9dafe03f) SHA1(978b4597ff2a54ac5049fd64798e8173b29dd363) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "snw20000.u17", 0x000000, 0x400000, CRC(d5fe5f8c) SHA1(817d8d0a5fbc0c50dc3c592f938150f82df97cec) )
	ROM_LOAD( "snw20100.u9",  0x400000, 0x400000, CRC(c0037846) SHA1(3267b142ebce47e1717250239d98fdb4af7964f8) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?
	ROM_LOAD( "snw21000.u3",  0x400000, 0x400000, CRC(f5c23e79) SHA1(b509680001c3205b289f43d4f44aaaa7f896419b) )

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "snw30000.u1",  0x000000, 0x400000, CRC(ec9eef40) SHA1(8f74ec9cb6054a77227c0505094f0ef8bc371429) )
ROM_END

ROM_START( teljan )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "tel1j.u10", 0x000000, 0x080000, CRC(09b552fe) SHA1(2f315fd09eb22fa8c81faa1e926038f20daa845f) )
	ROM_LOAD16_BYTE( "tel1j.u8",  0x000001, 0x080000, CRC(070b4345) SHA1(5743f12a351b89593c6adfaeb8a5a2ab7bc8b424) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "tj100-00.u24", 0x000000, 0x400000, CRC(810144f1) SHA1(1c90e71e5f34ee05771ab4a673329f78f17791df) )
	ROM_LOAD( "tj101-00.u20", 0x400000, 0x400000, CRC(82f570e1) SHA1(3ba9d1775f897052aca5cff2edbf575399101c5c) )
	ROM_LOAD( "tj102-00.u17", 0x800000, 0x400000, CRC(ace875dc) SHA1(be97c895beeac979c5704986e818d4f3cfa00e49) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "tj200-00.u16", 0x000000, 0x400000, CRC(be0f90b2) SHA1(1848a65f244e1e8a3ff7ab38e76f86cabca8b47e) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "tj300-00.u4", 0x000000, 0x400000, CRC(685495c4) SHA1(3853c0583b84ed3163370ae48e4b3912cbeb986e) )
ROM_END


ROM_START( ryouran )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "or-000-j2.u10",  0x000000, 0x080000, CRC(cba8ca4e) SHA1(7389502622a04101ca34f7b390ca0da820f62590) )
	ROM_LOAD16_BYTE( "or-001-j2.u8",   0x000001, 0x080000, CRC(8e79c6b7) SHA1(0441d279cdc998e96abd6f607eceb4f866f58337) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "or100-00.u24", 0x000000, 0x400000, CRC(e9c7695b) SHA1(0a104d4e4e0c933d2eaaf410a8c243db6673786a) )
	ROM_LOAD( "or101-00.u20", 0x400000, 0x400000, CRC(fe06bf12) SHA1(f3a2f88aed65bcc1c16f37fd4c0011e3538128f7) )
	ROM_LOAD( "or102-00.u17", 0x800000, 0x400000, CRC(f2a5237b) SHA1(b8871f9c0f3864c334ec9a8146cf7dd1961ecb94) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "or200-00.u16", 0x000000, 0x400000, CRC(4c4701a8) SHA1(7b397b553ba86bba2ee82228cabdf2179e878d69) )
	ROM_LOAD( "or201-00.u13", 0x400000, 0x400000, CRC(a94064aa) SHA1(5d736f810ffdbb6ada5c5efcb5fb29eedafc3e2f) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "or300-00.u4", 0x000000, 0x400000, CRC(a3f64b79) SHA1(6ecb2b4c0d213fe5384b19d6bfdb86871f21fd9f) )
ROM_END

ROM_START( ryourano )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "or000j1.u10",  0x000000, 0x080000, CRC(d93aa491) SHA1(dc01707f1e80d81f28d6b685d08fc6c0d2bf7330) )
	ROM_LOAD16_BYTE( "or001j1.u8",   0x000001, 0x080000, CRC(f466e5e9) SHA1(65d699f6f9e299333e51a6a52cb13a0f1a902fe1) )

	ROM_REGION( 0x1000000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "or100-00.u24", 0x000000, 0x400000, CRC(e9c7695b) SHA1(0a104d4e4e0c933d2eaaf410a8c243db6673786a) )
	ROM_LOAD( "or101-00.u20", 0x400000, 0x400000, CRC(fe06bf12) SHA1(f3a2f88aed65bcc1c16f37fd4c0011e3538128f7) )
	ROM_LOAD( "or102-00.u17", 0x800000, 0x400000, CRC(f2a5237b) SHA1(b8871f9c0f3864c334ec9a8146cf7dd1961ecb94) )

	ROM_REGION( 0x800000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "or200-00.u16", 0x000000, 0x400000, CRC(4c4701a8) SHA1(7b397b553ba86bba2ee82228cabdf2179e878d69) )
	ROM_LOAD( "or201-00.u13", 0x400000, 0x400000, CRC(a94064aa) SHA1(5d736f810ffdbb6ada5c5efcb5fb29eedafc3e2f) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x400000, "ymz", 0 ) // Samples
	ROM_LOAD( "or300-00.u4", 0x000000, 0x400000, CRC(a3f64b79) SHA1(6ecb2b4c0d213fe5384b19d6bfdb86871f21fd9f) )
ROM_END

ROM_START( vblokbrk )
	SKNS_EUROPE

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "sk000e2-e.u10", 0x000000, 0x080000, CRC(5a278f10) SHA1(6f1be0657da76144b0feeed672d92a85091289b6) ) // labeled  SK000e2/E
	ROM_LOAD16_BYTE( "sk000e-o.u8",   0x000001, 0x080000, CRC(aecf0647) SHA1(4752e5012bae8e7af3972b455b4346499ec2b49c) ) // labeled  SK000e/O

	ROM_REGION( 0x0400000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "sk-100-00.u24", 0x000000, 0x200000, CRC(151dd88a) SHA1(87bb1039a9883f721a315760eb2c4abe4a94046f) )
	ROM_LOAD( "sk-101.u20",    0x200000, 0x100000, CRC(779cce23) SHA1(70147b36d982524ba9921823e481ce8fbb5daa26) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "sk-200-00.u16", 0x000000, 0x200000, CRC(2e297c61) SHA1(4071b945a1294fbc3d18fab1f144bf09af4349e8) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x200000, "ymz", 0 ) // Samples
	ROM_LOAD( "sk-300-00.u4", 0x000000, 0x200000, CRC(e6535c05) SHA1(8895b7c326e0261691cb184887ac1ca637302460) )
ROM_END

ROM_START( vblokbrka )
	SKNS_ASIA

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "sk01a.u10", 0x000000, 0x080000, CRC(4d1be53e) SHA1(3d28b73a67530147962b8df6244af8bea2ab080f) )
	ROM_LOAD16_BYTE( "sk01a.u8",  0x000001, 0x080000, CRC(461e0197) SHA1(003573a4abdbecc6dd234a13c61ef07a25d980e2) )

	ROM_REGION( 0x0400000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "sk-100-00.u24", 0x000000, 0x200000, CRC(151dd88a) SHA1(87bb1039a9883f721a315760eb2c4abe4a94046f) )
	ROM_LOAD( "sk-101.u20",    0x200000, 0x100000, CRC(779cce23) SHA1(70147b36d982524ba9921823e481ce8fbb5daa26) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "sk-200-00.u16", 0x000000, 0x200000, CRC(2e297c61) SHA1(4071b945a1294fbc3d18fab1f144bf09af4349e8) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x200000, "ymz", 0 ) // Samples
	ROM_LOAD( "sk-300-00.u4", 0x000000, 0x200000, CRC(e6535c05) SHA1(8895b7c326e0261691cb184887ac1ca637302460) )
ROM_END

ROM_START( sarukani )
	SKNS_JAPAN

	ROM_REGION32_BE( 0x200000, "user1", 0 ) // SH-2 Code mapped at 0x04000000
	ROM_LOAD16_BYTE( "sk1j1.u10", 0x000000, 0x080000, CRC(fcc131b6) SHA1(5e3e71ee1f736b6098e671e6f57b1fb313c81adb) )
	ROM_LOAD16_BYTE( "sk1j1.u8",  0x000001, 0x080000, CRC(3b6aa343) SHA1(a969b20b1170d82351024cab9e37f2fbfd01ddeb) )

	ROM_REGION( 0x0400000, "spritegen", 0 ) // Sprites
	ROM_LOAD( "sk-100-00.u24", 0x000000, 0x200000, CRC(151dd88a) SHA1(87bb1039a9883f721a315760eb2c4abe4a94046f) )
	ROM_LOAD( "sk-101.u20",    0x200000, 0x100000, CRC(779cce23) SHA1(70147b36d982524ba9921823e481ce8fbb5daa26) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Tiles Plane A
	ROM_LOAD( "sk-200-00.u16", 0x000000, 0x200000, CRC(2e297c61) SHA1(4071b945a1294fbc3d18fab1f144bf09af4349e8) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) // Tiles Plane B
	// First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles
	// 0x040000 - 0x3fffff empty?

	ROM_REGION( 0x200000, "ymz", 0 ) // Samples
	ROM_LOAD( "sk-300-00.u4", 0x000000, 0x200000, CRC(e6535c05) SHA1(8895b7c326e0261691cb184887ac1ca637302460) )
ROM_END


/***** GAME DRIVERS *****/

GAME( 1996, skns,      0,        skns,  skns,     skns_state, empty_init,     ROT0,  "Kaneko", "Super Kaneko Nova System BIOS", MACHINE_IS_BIOS_ROOT )

GAME( 1996, galpani4,  skns,     sknse, cyvern,   skns_state, init_galpani4,  ROT0,  "Kaneko", "Gals Panic 4 (Europe)",                          MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, galpani4j, galpani4, sknsj, cyvern,   skns_state, init_galpani4,  ROT0,  "Kaneko", "Gals Panic 4 (Japan)",                           MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, galpani4k, galpani4, sknsk, cyvern,   skns_state, init_galpani4,  ROT0,  "Kaneko", "Gals Panic 4 (Korea)",                           MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, galpaniex, galpani4, sknsa, cyvern,   skns_state, init_galpani4,  ROT0,  "Kaneko", "Gals Panic EX (Korea)",                          MACHINE_IMPERFECT_GRAPHICS ) // copyright 2000, re-release for the Asian/Korean market?
GAME( 2000, galpanisu, galpani4, sknsk, cyvern,   skns_state, init_galpani4,  ROT0,  "Kaneko", "Gals Panic SU (Korea, Gals Panic 4 re-release)", MACHINE_IMPERFECT_GRAPHICS ) // copyright 2000, re-release for the Asian/Korean market?
GAME( 2001, galpanidx, galpani4, sknsa, cyvern,   skns_state, init_galpani4,  ROT0,  "Kaneko", "Gals Panic DX (Asia)",                           MACHINE_IMPERFECT_GRAPHICS ) // copyright 2001, re-release for the Asian market?

GAME( 1996, jjparads,  skns,     sknsj, skns_1p,  skns_state, init_jjparads,  ROT0,  "Electro Design", "Jan Jan Paradise", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1997, galpanis,  skns,     sknse, galpanis, skns_state, init_galpanis,  ROT0,  "Kaneko", "Gals Panic S - Extra Edition (Europe, revision 1)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, galpanise, galpanis, sknse, galpanis, skns_state, init_galpanis,  ROT0,  "Kaneko", "Gals Panic S - Extra Edition (Europe, set 2)", MACHINE_IMPERFECT_GRAPHICS ) // E0 set??
GAME( 1997, galpanisj, galpanis, sknsj, galpanis, skns_state, init_galpanis,  ROT0,  "Kaneko", "Gals Panic S - Extra Edition (Japan, revision 1)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, galpanisa, galpanis, sknsa, galpanis, skns_state, init_galpanis,  ROT0,  "Kaneko", "Gals Panic S - Extra Edition (Asia)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, galpanisk, galpanis, sknsk, galpanis, skns_state, init_galpanis,  ROT0,  "Kaneko", "Gals Panic S - Extra Edition (Korea, revision 1)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, galpaniska,galpanis, sknsk, galpanis, skns_state, init_galpanis,  ROT0,  "Kaneko", "Gals Panic S - Extra Edition (Korea)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1997, jjparad2,  skns,     sknsj, skns_1p,  skns_state, init_jjparad2,  ROT0,  "Electro Design", "Jan Jan Paradise 2", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1997, sengekis,  skns,     sknsa, skns,     skns_state, init_sengekis,  ROT90, "Kaneko / Warashi", "Sengeki Striker (Asia)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sengekisj, sengekis, sknsj, skns,     skns_state, init_sengekij,  ROT90, "Kaneko / Warashi", "Sengeki Striker (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1997, vblokbrk,  skns,     sknse, vblokbrk, skns_state, init_sarukani,  ROT0,  "Kaneko / Mediaworks", "VS Block Breaker (Europe)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, vblokbrka, vblokbrk, sknsa, vblokbrk, skns_state, init_sarukani,  ROT0,  "Kaneko / Mediaworks", "VS Block Breaker (Asia)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sarukani,  vblokbrk, sknsj, vblokbrk, skns_state, init_sarukani,  ROT0,  "Kaneko / Mediaworks", "Saru-Kani-Hamu-Zou (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1998, cyvern,    skns,     sknsu, cyvern,   skns_state, init_cyvern,    ROT90, "Kaneko", "Cyvern - The Dragon Weapons (US)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, cyvernj,   cyvern,   sknsj, cyvern,   skns_state, init_cyvern,    ROT90, "Kaneko", "Cyvern - The Dragon Weapons (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1998, puzzloop,  skns,     sknse, puzzloop, skns_state, init_puzzloopu, ROT0,  "Mitchell", "Puzz Loop (Europe, v0.94)", MACHINE_IMPERFECT_GRAPHICS ) // Same speed up as US version
GAME( 1998, puzzloope, puzzloop, sknse, puzzloop, skns_state, init_puzzloope, ROT0,  "Mitchell", "Puzz Loop (Europe, v0.93)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, puzzloopj, puzzloop, sknsj, puzzloop, skns_state, init_puzzloopj, ROT0,  "Mitchell", "Puzz Loop (Japan)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, puzzloopa, puzzloop, sknsa, puzzloop, skns_state, init_puzzloopa, ROT0,  "Mitchell", "Puzz Loop (Asia)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, puzzloopk, puzzloop, sknsk, puzzloop, skns_state, init_puzzloopu, ROT0,  "Mitchell", "Puzz Loop (Korea)", MACHINE_IMPERFECT_GRAPHICS ) // Same speed up as US version
GAME( 1998, puzzloopu, puzzloop, sknsu, puzzloop, skns_state, init_puzzloopu, ROT0,  "Mitchell", "Puzz Loop (USA)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1998, ryouran ,  skns,     sknsj, skns_1p,  skns_state, init_ryouran,   ROT0,  "Electro Design", "VS Mahjong Otome Ryouran (revision 2)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, ryourano,  ryouran,  sknsj, skns_1p,  skns_state, init_ryouran,   ROT0,  "Electro Design", "VS Mahjong Otome Ryouran (revision 1)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1999, galpans2,  skns,     sknse, galpanis, skns_state, init_galpans2,  ROT0,  "Kaneko", "Gals Panic S2 (Europe, version 3)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, galpans2j, galpans2, sknsj, galpanis, skns_state, init_galpans2,  ROT0,  "Kaneko", "Gals Panic S2 (Japan)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, galpans2a, galpans2, sknsa, galpanis, skns_state, init_galpans2,  ROT0,  "Kaneko", "Gals Panic S2 (Asia, version 1.1)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, galpansu,  galpans2, sknsk, galpanis, skns_state, init_galpans2,  ROT0,  "Kaneko", "Gals Panic SU (Korea, version 5)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, galpansua, galpans2, sknsk, galpanis, skns_state, init_galpans2,  ROT0,  "Kaneko", "Gals Panic SU (Korea)", MACHINE_IMPERFECT_GRAPHICS ) // official or hack?

GAME( 1999, panicstr,  skns,     sknsj, galpanis, skns_state, init_panicstr,  ROT0,  "Kaneko", "Panic Street (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1999, senknow ,  skns,     sknsj, skns,     skns_state, init_senknow,   ROT0,  "Kaneko / Kouyousha", "Sen-Know (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1999, teljan  ,  skns,     sknsj, skns_1p,  skns_state, init_teljan,    ROT0,  "Electro Design", "Tel Jan", MACHINE_IMPERFECT_GRAPHICS )

GAME( 2000, gutsn,     skns,     sknsj, skns,     skns_state, init_gutsn,     ROT0,  "Kaneko / Kouyousha", "Guts'n (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 2002, galpans3,  skns,     sknsj, galpanis, skns_state, init_galpans3,  ROT0,  "Kaneko", "Gals Panic S3 (Japan)", MACHINE_IMPERFECT_GRAPHICS )
