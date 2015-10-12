// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Lee Taylor
/***************************************************************************

Universal board numbers (found on the schematics)

Cosmic Guerilla - 7907A
Cosmic Alien    - 7910
Magical Spot    - 8013
Magical Spot II - 8013
Devil Zone      - 8022

TODO:
- double check irq sources via schematics

2008-08
Dip locations verified with manuals for all the games.


Note on Coinage DSW (devzone and magspot only):

According to manuals, coinage settings (see "DSW" port) should be set by
a physical DSW B but only read when SWA:3,4 are both set to OFF. Currently,

* In magspot, SWA:3 affects the number of lives & SWA:4 seems to have no
    effect
* In devzone, setting SWA:3,4 on anything but OFF,OFF results in no coins
    accepted at all

cosmicg - background should be blue
cosmicg - board can operate in b&w mode if there is no PROM, in this case
          a colour overlay should be used.

***************************************************************************/


#include "emu.h"
#include "cpu/tms9900/tms9980a.h"
#include "cpu/z80/z80.h"
#include "sound/samples.h"
#include "sound/dac.h"
#include "includes/cosmic.h"


/* Schematics show 12 triggers for discrete sound circuits */

WRITE8_MEMBER(cosmic_state::panic_sound_output_w)
{
	/* Sound Enable / Disable */
	if (offset == 11)
	{
		int count;
		if (data == 0)
			for (count = 0; count < 9; count++)
				m_samples->stop(count);

		m_sound_enabled = data;
	}

	if (m_sound_enabled)
	{
		switch (offset)
		{
		case 0: if (data) m_samples->start(0, 0); break;    /* Walk */
		case 1: if (data) m_samples->start(0, 5); break;    /* Enemy Die 1 */
		case 2: if (data)                                   /* Drop 1 */
				{
					if (!m_samples->playing(1))
					{
						m_samples->stop(2);
						m_samples->start(1, 3);
					}
				}
				else
					m_samples->stop(1);
				break;

		case 3: if (data && !m_samples->playing(6))         /* Oxygen */
					m_samples->start(6, 9, true);
				break;

		case 4: break;                                      /* Drop 2 */
		case 5: if (data) m_samples->start(0, 5); break;    /* Enemy Die 2 (use same sample as 1) */
		case 6: if (data && !m_samples->playing(1) && !m_samples->playing(3))   /* Hang */
					m_samples->start(2, 2);
				break;

		case 7: if (data)                                   /* Escape */
				{
					m_samples->stop(2);
					m_samples->start(3, 4);
				}
				else
					m_samples->stop(3);
				break;

		case 8: if (data) m_samples->start(0, 1); break;    /* Stairs */
		case 9: if (data)                                   /* Extend */
					m_samples->start(4, 8);
				else
					m_samples->stop(4);
				break;

		case 10:    m_dac->write_unsigned8(data); break;/* Bonus */
		case 15:    if (data) m_samples->start(0, 6); break;    /* Player Die */
		case 16:    if (data) m_samples->start(5, 7); break;    /* Enemy Laugh */
		case 17:    if (data) m_samples->start(0, 10); break;   /* Coin - Not triggered by software */
		}
	}

	#ifdef MAME_DEBUG
	logerror("Sound output %x=%x\n", offset, data);
	#endif
}

WRITE8_MEMBER(cosmic_state::panic_sound_output2_w)
{
	panic_sound_output_w(space, offset + 15, data);
}

WRITE8_MEMBER(cosmic_state::cosmicg_output_w)
{
	/* Sound Enable / Disable */
	if (offset == 12)
	{
		int count;

		m_sound_enabled = data;
		if (data == 0)
			for (count = 0; count < 9; count++)
				m_samples->stop(count);
	}

	if (m_sound_enabled)
	{
		switch (offset)
		{
		/* The schematics show a direct link to the sound amp  */
		/* as other cosmic series games, but it never seems to */
		/* be used for anything. It is implemented for sake of */
		/* completness. Maybe it plays a tune if you win ?     */
		case 1: m_dac->write_unsigned8(-data); break;
		case 2: if (data) m_samples->start(0, m_march_select); break;   /* March Sound */
		case 3: m_march_select = (m_march_select & 0xfe) | data; break;
		case 4: m_march_select = (m_march_select & 0xfd) | (data << 1); break;
		case 5: m_march_select = (m_march_select & 0xfb) | (data << 2); break;

		case 6: if (data)                           /* Killer Attack (crawly thing at bottom of screen) */
					m_samples->start(1, 8, true);
				else
					m_samples->stop(1);
				break;

		case 7: if (data)                               /* Bonus Chance & Got Bonus */
				{
					m_samples->stop(4);
					m_samples->start(4, 10);
				}
				break;

		case 8: if (data)
				{
					if (!m_samples->playing(4)) m_samples->start(4, 9, true);
				}
				else
					m_samples->stop(4);
				break;

		case 9: if (data) m_samples->start(3, 11); break;   /* Got Ship */
//      case 11: watchdog_reset_w(0, 0); break;             /* Watchdog */
		case 13:    if (data) m_samples->start(8, 13 - m_gun_die_select); break;  /* Got Monster / Gunshot */
		case 14:    m_gun_die_select = data; break;
		case 15:    if (data) m_samples->start(5, 14); break;   /* Coin Extend (extra base) */
		}
	}

	#ifdef MAME_DEBUG
	if (offset != 11) logerror("Output %x=%x\n", offset, data);
	#endif
}


WRITE8_MEMBER(cosmic_state::cosmica_sound_output_w)
{
	/* Sound Enable / Disable */
	if (offset == 11)
	{
		int count;
		if (data == 0)
			for (count = 0; count < 12; count++)
				m_samples->stop(count);
		else
		{
			m_samples->start(0, 0, true); /*Background Noise*/
		}

		m_sound_enabled = data;
	}

	if (m_sound_enabled)
	{
		switch (offset)
		{
		case 0: if (data) m_samples->start(1, 2); break; /*Dive Bombing Type A*/

		case 2: /*Dive Bombing Type B (Main Control)*/
			if (data)
			{
				switch (m_dive_bomb_b_select)
				{
				case 2:
					if (m_samples->playing(2))
					{
						m_samples->stop(2);
						m_samples->start(2, 3); break;
					}
					else
						m_samples->start(2, 3); break;

				case 3:
					if (m_samples->playing(3))
					{
						m_samples->stop(3);
						m_samples->start(3, 4); break;
					}
					else
						m_samples->start(3, 4); break;

				case 4:
					if (m_samples->playing(4))
					{
						m_samples->stop(4);
						m_samples->start(4, 5); break;
					}
					else
						m_samples->start(4, 5); break;

				case 5:
					if (m_samples->playing(5))
					{
						m_samples->stop(5);
						m_samples->start(5, 6); break;
					}
					else
						m_samples->start(5, 6); break;

				case 6:
					if (m_samples->playing(6))
					{
						m_samples->stop(6);
						m_samples->start(6, 7); break;
					}
					else
						m_samples->start(6, 7); break;

				case 7:
					if (m_samples->playing(7))
					{
						m_samples->stop(7);
						m_samples->start(7, 8); break;
					}
					else
						m_samples->start(7, 8); break;
				}
			}

		case 3: /*Dive Bombing Type B (G.S.B)*/
			if (data)
				m_dive_bomb_b_select |= 0x04;
			else
				m_dive_bomb_b_select &= 0xfb;
			break;


		case 4: /*Dive Bombing Type B (M.S.B)*/
			if (data)
				m_dive_bomb_b_select |= 0x02;
			else
				m_dive_bomb_b_select &= 0xfd;
			break;

		case 5: /*Dive Bombing Type B (L.S.B)*/
			if (data)
				m_dive_bomb_b_select |= 0x01;
			else
				m_dive_bomb_b_select &= 0xfe;
			break;


		case 6: if (data) m_samples->start(8, 9); break; /*Fire Control*/

		case 7: if (data) m_samples->start(9, 10); break; /*Small Explosion*/

		case 8: if (data) m_samples->start(10, 11); break; /*Loud Explosion*/

		case 9:
			if (data)
				m_samples->start(11, 1, true);
			else
				m_samples->stop(11);
			break; /*Extend Sound control*/

		case 12:    if (data) m_samples->start(11,12); break; /*Insert Coin*/
		}
	}

	#ifdef MAME_DEBUG
	logerror("Sound output %x=%x\n", offset, data);
	#endif
}


READ8_MEMBER(cosmic_state::cosmica_pixel_clock_r)
{
	return (m_screen->vpos() >> 2) & 0x3f;
}

READ8_MEMBER(cosmic_state::cosmicg_port_0_r)
{
	/* The top four address lines from the CRTC are bits 0-3 */
	return (ioport("IN0")->read() & 0xf0) | ((m_screen->vpos() & 0xf0) >> 4);
}

READ8_MEMBER(cosmic_state::magspot_coinage_dip_r)
{
	return (ioport("DSW")->read_safe(0) & (1 << (7 - offset))) ? 0 : 1;
}


/* Has 8 way joystick, remap combinations to missing directions */

READ8_MEMBER(cosmic_state::nomnlnd_port_0_1_r)
{
	int control = ioport(offset ? "IN1" : "IN0")->read();
	int fire = ioport("IN3")->read();

	/* If firing - stop tank */
	if ((fire & 0xc0) == 0) return 0xff;

	/* set bit according to 8 way direction */
	if ((control & 0x82) == 0 ) return 0xfe;    /* Up & Left */
	if ((control & 0x0a) == 0 ) return 0xfb;    /* Down & Left */
	if ((control & 0x28) == 0 ) return 0xef;    /* Down & Right */
	if ((control & 0xa0) == 0 ) return 0xbf;    /* Up & Right */

	return control;
}



WRITE8_MEMBER(cosmic_state::flip_screen_w)
{
	flip_screen_set(data & 0x80);
}


static ADDRESS_MAP_START( panic_map, AS_PROGRAM, 8, cosmic_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x601f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x6800, 0x6800) AM_READ_PORT("P1")
	AM_RANGE(0x6801, 0x6801) AM_READ_PORT("P2")
	AM_RANGE(0x6802, 0x6802) AM_READ_PORT("DSW")
	AM_RANGE(0x6803, 0x6803) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x7000, 0x700b) AM_WRITE(panic_sound_output_w)
	AM_RANGE(0x700c, 0x700e) AM_WRITE(cosmic_color_register_w)
	AM_RANGE(0x700f, 0x700f) AM_WRITE(flip_screen_w)
	AM_RANGE(0x7800, 0x7801) AM_WRITE(panic_sound_output2_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( cosmica_map, AS_PROGRAM, 8, cosmic_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x601f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x6800, 0x6800) AM_READ_PORT("P1")
	AM_RANGE(0x6801, 0x6801) AM_READ_PORT("P2")
	AM_RANGE(0x6802, 0x6802) AM_READ_PORT("DSW")
	AM_RANGE(0x6803, 0x6803) AM_READ(cosmica_pixel_clock_r)
	AM_RANGE(0x7000, 0x700b) AM_WRITE(cosmica_sound_output_w)
	AM_RANGE(0x700c, 0x700d) AM_WRITE(cosmic_color_register_w)
	AM_RANGE(0x700f, 0x700f) AM_WRITE(flip_screen_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( cosmicg_map, AS_PROGRAM, 8, cosmic_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cosmicg_io_map, AS_IO, 8, cosmic_state )
	AM_RANGE(0x00, 0x00) AM_READ(cosmicg_port_0_r)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x00, 0x15) AM_WRITE(cosmicg_output_w)
	AM_RANGE(0x16, 0x17) AM_WRITE(cosmic_color_register_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( magspot_map, AS_PROGRAM, 8, cosmic_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3800, 0x3807) AM_READ(magspot_coinage_dip_r)
	AM_RANGE(0x4000, 0x401f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x4800, 0x4800) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0x480c, 0x480d) AM_WRITE(cosmic_color_register_w)
	AM_RANGE(0x480f, 0x480f) AM_WRITE(flip_screen_w)
	AM_RANGE(0x5000, 0x5000) AM_READ_PORT("IN0")
	AM_RANGE(0x5001, 0x5001) AM_READ_PORT("IN1")
	AM_RANGE(0x5002, 0x5002) AM_READ_PORT("IN2")
	AM_RANGE(0x5003, 0x5003) AM_READ_PORT("IN3")
	AM_RANGE(0x6000, 0x7fff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END



INPUT_CHANGED_MEMBER(cosmic_state::panic_coin_inserted)
{
	panic_sound_output_w(m_maincpu->space(AS_PROGRAM), 17, newval == 0);
}

static INPUT_PORTS_START( panic )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW:6,5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	/* 0x06 and 0x07 disabled */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x10, "5000" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmic_state,panic_coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmic_state,panic_coin_inserted, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(cosmic_state::cosmica_coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( cosmica )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:4,3")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING(    0x30, "5000" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmic_state,cosmica_coin_inserted, 0)
INPUT_PORTS_END

/* These are used for the CR handling - This can be used to */
/* from 1 to 16 bits from any bit offset between 0 and 4096 */

/* Offsets are in BYTES, so bits 0-7 are at offset 0 etc.   */

INPUT_CHANGED_MEMBER(cosmic_state::cosmicg_coin_inserted)
{
	m_maincpu->set_input_line(INT_9980A_LEVEL4, newval? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( cosmicg )
	PORT_START("IN0")   /* 4-7 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL )    /* pixel clock */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY

	PORT_START("IN1")   /* 8-15 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_2WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x20, "1500" )
	PORT_DIPSETTING(    0x30, "2000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START("IN2")   /* Hard wired settings */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmic_state,cosmicg_coin_inserted, 0)

	/* This dip switch is not read by the program at any time   */
	/* but is wired to enable or disable the flip screen output */

	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )

	/* This odd setting is marked as shown on the schematic,    */
	/* and again, is not read by the program, but wired into    */
	/* the watchdog circuit. The book says to leave it off      */

	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW:6" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(cosmic_state::coin_inserted_irq0)
{
	m_maincpu->set_input_line(0, newval ? HOLD_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(cosmic_state::coin_inserted_nmi)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( magspot )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x1c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_DIPNAME( 0xc0, 0x40, "Bonus Game" )
	PORT_DIPSETTING(    0x40, "5000" )
	PORT_DIPSETTING(    0x80, "10000" )
	PORT_DIPSETTING(    0xc0, "15000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x1c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x02, "3000" )
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	/* According to the manual, bits 2-3 should control coinage like in devzone
	and only bit 4 should control lives setting. */
//  PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SWA:3,4")
//  PORT_DIPSETTING(    0x0c, "Use Coin A & B" )
//  PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
//  PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:2")
//  PORT_DIPSETTING(    0x00, "2" )
//  PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SWA:4" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:2,3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNUSED )     /* always HI */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL )    /* reads what was written to 4808.  Probably not used?? */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	/* Fake port to handle coins */
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmic_state,coin_inserted_irq0, 0) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmic_state,coin_inserted_nmi, 0)

	/* Fake port to handle coinage dip switches. Each bit goes to 3800-3807 */
	PORT_START("DSW")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( devzone )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x1c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x1c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "6000" )
	PORT_DIPSETTING(    0x03, "8000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SWA:3,4")
	PORT_DIPSETTING(    0x0c, "Use Coin A & B" )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x3e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	/* Fake port to handle coins */
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmic_state,coin_inserted_irq0, 0) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmic_state,coin_inserted_nmi, 0)

	PORT_START("DSW")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( devzone2 )
	PORT_INCLUDE( devzone )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x02, "3000" )
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( nomnlnd )
	PORT_START("IN0")   /* Controls - Remapped for game */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x55, IP_ACTIVE_LOW, IPT_SPECIAL )    /* diagonals */

	PORT_START("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x55, IP_ACTIVE_LOW, IPT_SPECIAL )    /* diagonals */

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x02, "3000" )
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNUSED )     /* always HI */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL )    /* reads what was written to 4808.  Probably not used?? */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	/* Fake port to handle coin */
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmic_state,coin_inserted_nmi, 0)
INPUT_PORTS_END


static INPUT_PORTS_START( nomnlndg )
	PORT_INCLUDE( nomnlnd )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "8000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
INPUT_PORTS_END


static const gfx_layout cosmic_spritelayout16 =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{  0*8+0,  0*8+1,  0*8+2,  0*8+3,  0*8+4,  0*8+5,  0*8+6,  0*8+7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7},
	{ 0*8, 1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout cosmic_spritelayout32 =
{
	32,32,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0*32*8+0, 0*32*8+1, 0*32*8+2, 0*32*8+3, 0*32*8+4, 0*32*8+5, 0*32*8+6, 0*32*8+7,
		1*32*8+0, 1*32*8+1, 1*32*8+2, 1*32*8+3, 1*32*8+4, 1*32*8+5, 1*32*8+6, 1*32*8+7,
		2*32*8+0, 2*32*8+1, 2*32*8+2, 2*32*8+3, 2*32*8+4, 2*32*8+5, 2*32*8+6, 2*32*8+7,
		3*32*8+0, 3*32*8+1, 3*32*8+2, 3*32*8+3, 3*32*8+4, 3*32*8+5, 3*32*8+6, 3*32*8+7 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8 },
	128*8
};


static GFXDECODE_START( panic )
	GFXDECODE_ENTRY( "gfx1", 0, cosmic_spritelayout16, 16, 8 )
	GFXDECODE_ENTRY( "gfx1", 0, cosmic_spritelayout32, 16, 8 )
GFXDECODE_END

static GFXDECODE_START( cosmica )
	GFXDECODE_ENTRY( "gfx1", 0, cosmic_spritelayout16,  8, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, cosmic_spritelayout32,  8, 16 )
GFXDECODE_END


static const char *const cosmica_sample_names[] =
{
	"*cosmica",
	"backgr",
	"extend",
	"divea",
	"diveb1",
	"diveb2",
	"diveb3",
	"diveb4",
	"diveb5",
	"diveb6",
	"fire",
	"loudexp",
	"smallexp",
	"coin",
	0       /* end of array */
};


static const char *const panic_sample_names[] =
{
	"*panic",
	"walk",
	"upordown",
	"trapped",
	"falling",
	"escaping",
	"ekilled",
	"death",
	"elaugh",
	"extral",
	"oxygen",
	"coin",
	0
};


static const char *const cosmicg_sample_names[] =
{
	"*cosmicg",
	"cg_m0",    /* 8 Different pitches of March Sound */
	"cg_m1",
	"cg_m2",
	"cg_m3",
	"cg_m4",
	"cg_m5",
	"cg_m6",
	"cg_m7",
	"cg_att",   /* Killer Attack */
	"cg_chnc",  /* Bonus Chance  */
	"cg_gotb",  /* Got Bonus - have not got correct sound for */
	"cg_dest",  /* Gun Destroy */
	"cg_gun",   /* Gun Shot */
	"cg_gotm",  /* Got Monster */
	"cg_ext",   /* Coin Extend */
	0
};


MACHINE_START_MEMBER(cosmic_state,cosmic)
{
	save_item(NAME(m_sound_enabled));
	save_item(NAME(m_march_select));
	save_item(NAME(m_gun_die_select));
	save_item(NAME(m_dive_bomb_b_select));
	save_item(NAME(m_pixel_clock));

	save_item(NAME(m_background_enable));
	save_item(NAME(m_color_registers));
}

MACHINE_RESET_MEMBER(cosmic_state,cosmic)
{
	m_pixel_clock = 0;
	m_background_enable = 0;
	m_color_registers[0] = 0;
	m_color_registers[1] = 0;
	m_color_registers[2] = 0;
}

MACHINE_RESET_MEMBER(cosmic_state,cosmicg)
{
	m_pixel_clock = 0;
	m_background_enable = 0;
	m_color_registers[0] = 0;
	m_color_registers[1] = 0;
	m_color_registers[2] = 0;
	m_maincpu->set_input_line(INT_9980A_RESET, ASSERT_LINE);
	m_maincpu->set_input_line(INT_9980A_RESET, CLEAR_LINE);
}

static MACHINE_CONFIG_START( cosmic, cosmic_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,Z80_MASTER_CLOCK/6) /* 1.8026 MHz*/

	MCFG_MACHINE_START_OVERRIDE(cosmic_state,cosmic)
	MCFG_MACHINE_RESET_OVERRIDE(cosmic_state,cosmic)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_PALETTE("palette")
MACHINE_CONFIG_END

TIMER_DEVICE_CALLBACK_MEMBER(cosmic_state::panic_scanline)
{
	int scanline = param;

	if(scanline == 224) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xd7); /* RST 10h */

	if(scanline == 0) // vblank-in irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xcf); /* RST 08h */
}


static MACHINE_CONFIG_DERIVED( panic, cosmic )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(panic_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", cosmic_state, panic_scanline, "screen", 0, 1)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", panic)
	MCFG_PALETTE_ADD("palette", 16+8*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(16)
	MCFG_PALETTE_INIT_OWNER(cosmic_state,panic)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(cosmic_state, screen_update_panic)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(9)
	MCFG_SAMPLES_NAMES(panic_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cosmica, cosmic )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(cosmica_map)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cosmica)
	MCFG_PALETTE_ADD("palette", 8+16*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(8)
	MCFG_PALETTE_INIT_OWNER(cosmic_state,cosmica)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(cosmic_state, screen_update_cosmica)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(13)
	MCFG_SAMPLES_NAMES(cosmica_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( cosmicg, cosmic_state )

	/* basic machine hardware */
	MCFG_TMS99xx_ADD("maincpu", TMS9980A, COSMICG_MASTER_CLOCK/8, cosmicg_map, cosmicg_io_map)
		/* 9.828 MHz Crystal */
		/* R Nabet : huh ? This would imply the crystal frequency is somehow divided by 2 before being
		fed to the tms9904 or tms9980.  Also, I have never heard of a tms9900/9980 operating under
		1.5MHz.  So, if someone can check this... */

	MCFG_MACHINE_START_OVERRIDE(cosmic_state,cosmic)
	MCFG_MACHINE_RESET_OVERRIDE(cosmic_state,cosmicg)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cosmic_state, screen_update_cosmicg)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(cosmic_state,cosmicg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(9)
	MCFG_SAMPLES_NAMES(cosmicg_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( magspot, cosmic )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(magspot_map)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", panic)
	MCFG_PALETTE_ADD("palette", 16+8*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(16)
	MCFG_PALETTE_INIT_OWNER(cosmic_state,magspot)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(cosmic_state, screen_update_magspot)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( devzone, magspot )

	/* basic machine hardware */

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(cosmic_state, screen_update_devzone)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( nomnlnd, cosmic )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(magspot_map)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", panic)
	MCFG_PALETTE_ADD("palette", 16+8*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(16)
	MCFG_PALETTE_INIT_OWNER(cosmic_state,nomnlnd)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(cosmic_state, screen_update_nomnlnd)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( panic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spe1",         0x0000, 0x0800, CRC(70ac0888) SHA1(bdc6dfb74b4643df36cae60923f9759751340c86) )
	ROM_LOAD( "spe2",         0x0800, 0x0800, CRC(2b910c48) SHA1(9ebb15694e068a4d8769ec5d312af1148818d472) )
	ROM_LOAD( "spe3",         0x1000, 0x0800, CRC(03810148) SHA1(768418bc0a3a5bc9f7ec07b8edd4099da69efac6) )
	ROM_LOAD( "spe4",         0x1800, 0x0800, CRC(119bbbfd) SHA1(2b3722300b1eebe1bffa4a4e39fceb45aefde24f) )
	ROM_LOAD( "spcpanic.5",   0x2000, 0x0800, CRC(5b80f277) SHA1(b060e57c88679f547153aed041a5554dc26a83aa) )
	ROM_LOAD( "spcpanic.6",   0x2800, 0x0800, CRC(b73babf0) SHA1(229944a6b3653601bc20afea5a9aec787fd95ce0) )
	ROM_LOAD( "spe7",         0x3000, 0x0800, CRC(2894106e) SHA1(625896225b0ec03ac12f3e8b97e801cb743f37e7) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "spcpanic.11",  0x0000, 0x0800, CRC(acea9df4) SHA1(7de2a82da8160ad1a01c32a516d10c19dc306051) )
	ROM_LOAD( "spcpanic.12",  0x0800, 0x0800, CRC(e83423d0) SHA1(eba1129537869f1ecb5afeeae19db19b134865f6) )
	ROM_LOAD( "spcpanic.10",  0x1000, 0x0800, CRC(c9631c2d) SHA1(e5ab95e19c1b22a798a70a1a6599bc1f5e853c60) )
	ROM_LOAD( "spcpanic.9",   0x1800, 0x0800, CRC(eec78b4c) SHA1(efd21d0a26b988a490c45315a7a121607f74d147) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.sp",    0x0000, 0x0020, CRC(35d43d2f) SHA1(2ce164c92ed7ba3ee26a907f0c5969ec3decca01) )

	ROM_REGION( 0x0800, "user1", 0 ) /* color map */
	ROM_LOAD( "spcpanic.8",   0x0000, 0x0800, CRC(7da0b321) SHA1(b450cc02de9cc27e3f336c626221c90c6961b51e) )
ROM_END

ROM_START( panic2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spcpanic.1",   0x0000, 0x0800, CRC(405ae6f9) SHA1(92000f5f9bc1384ebae36dd30e715764747504d8) )
	ROM_LOAD( "spcpanic.2",   0x0800, 0x0800, CRC(b6a286c5) SHA1(b33beb1fbe622e9c90888d25d018fd5bef6cb65b) )
	ROM_LOAD( "spcpanic.3",   0x1000, 0x0800, CRC(85ae8b2e) SHA1(a5676d38e3c0ea0aeedc29bea0c04086e51da67f) )
	ROM_LOAD( "spcpanic.4",   0x1800, 0x0800, CRC(b6d4f52f) SHA1(431e5ef00768a633d17449a888ac9ce46975272d) )
	ROM_LOAD( "spcpanic.5",   0x2000, 0x0800, CRC(5b80f277) SHA1(b060e57c88679f547153aed041a5554dc26a83aa) )
	ROM_LOAD( "spcpanic.6",   0x2800, 0x0800, CRC(b73babf0) SHA1(229944a6b3653601bc20afea5a9aec787fd95ce0) )
	ROM_LOAD( "spcpanic.7",   0x3000, 0x0800, CRC(fc27f4e5) SHA1(80064ccfb810d11f6d7d79bfd991adb2eb2f1c16) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "spcpanic.11",  0x0000, 0x0800, CRC(acea9df4) SHA1(7de2a82da8160ad1a01c32a516d10c19dc306051) )
	ROM_LOAD( "spcpanic.12",  0x0800, 0x0800, CRC(e83423d0) SHA1(eba1129537869f1ecb5afeeae19db19b134865f6) )
	ROM_LOAD( "spcpanic.10",  0x1000, 0x0800, CRC(c9631c2d) SHA1(e5ab95e19c1b22a798a70a1a6599bc1f5e853c60) )
	ROM_LOAD( "spcpanic.9",   0x1800, 0x0800, CRC(eec78b4c) SHA1(efd21d0a26b988a490c45315a7a121607f74d147) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.sp",    0x0000, 0x0020, CRC(35d43d2f) SHA1(2ce164c92ed7ba3ee26a907f0c5969ec3decca01) )

	ROM_REGION( 0x0800, "user1", 0 ) /* color map */
	ROM_LOAD( "spcpanic.8",   0x0000, 0x0800, CRC(7da0b321) SHA1(b450cc02de9cc27e3f336c626221c90c6961b51e) )
ROM_END

ROM_START( panic3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "panica.1",     0x0000, 0x0800, CRC(289720ce) SHA1(8601bda95ac32a55f17fe9c723796bfe8b2b2fa7) )
	ROM_LOAD( "spcpanic.2",   0x0800, 0x0800, CRC(b6a286c5) SHA1(b33beb1fbe622e9c90888d25d018fd5bef6cb65b) )
	ROM_LOAD( "spcpanic.3",   0x1000, 0x0800, CRC(85ae8b2e) SHA1(a5676d38e3c0ea0aeedc29bea0c04086e51da67f) )
	ROM_LOAD( "spcpanic.4",   0x1800, 0x0800, CRC(b6d4f52f) SHA1(431e5ef00768a633d17449a888ac9ce46975272d) )
	ROM_LOAD( "spcpanic.5",   0x2000, 0x0800, CRC(5b80f277) SHA1(b060e57c88679f547153aed041a5554dc26a83aa) )
	ROM_LOAD( "spcpanic.6",   0x2800, 0x0800, CRC(b73babf0) SHA1(229944a6b3653601bc20afea5a9aec787fd95ce0) )
	ROM_LOAD( "panica.7",     0x3000, 0x0800, CRC(3641cb7f) SHA1(94a5108233cf9517f782759bb396e4eab58b8551) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "spcpanic.11",  0x0000, 0x0800, CRC(acea9df4) SHA1(7de2a82da8160ad1a01c32a516d10c19dc306051) )
	ROM_LOAD( "spcpanic.12",  0x0800, 0x0800, CRC(e83423d0) SHA1(eba1129537869f1ecb5afeeae19db19b134865f6) )
	ROM_LOAD( "spcpanic.10",  0x1000, 0x0800, CRC(c9631c2d) SHA1(e5ab95e19c1b22a798a70a1a6599bc1f5e853c60) )
	ROM_LOAD( "spcpanic.9",   0x1800, 0x0800, CRC(eec78b4c) SHA1(efd21d0a26b988a490c45315a7a121607f74d147) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.sp",    0x0000, 0x0020, CRC(35d43d2f) SHA1(2ce164c92ed7ba3ee26a907f0c5969ec3decca01) )

	ROM_REGION( 0x0800, "user1", 0 ) /* color map */
	ROM_LOAD( "spcpanic.8",   0x0000, 0x0800, CRC(7da0b321) SHA1(b450cc02de9cc27e3f336c626221c90c6961b51e) )
ROM_END

ROM_START( panich )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sph1",         0x0000, 0x0800, CRC(f6e9c6ef) SHA1(90b5bba0fd726e4c6618793467eba8c18c63fd43) )
	ROM_LOAD( "sph2",         0x0800, 0x0800, CRC(58dbc49b) SHA1(f716e8cdbb7eb456bd7f2996241b5ebd03086de3) )
	ROM_LOAD( "sph3",         0x1000, 0x0800, CRC(c4f275ad) SHA1(446be24dc99e46f3c69cf2cfb657958053857b7d) )
	ROM_LOAD( "sph4",         0x1800, 0x0800, CRC(6e7785de) SHA1(d12326791cbcae37980e240e2bfc20d7618f3ef5) )
	ROM_LOAD( "sph5",         0x2000, 0x0800, CRC(1916c9b8) SHA1(ab4a353340f152d6ba181555ee211afeb7877509) )
	ROM_LOAD( "sph6",         0x2800, 0x0800, CRC(54b92314) SHA1(970ebae831ea0a1958b8d711ebc5956ef4f932fe) )
	ROM_LOAD( "sph7",         0x3000, 0x0800, CRC(8600b881) SHA1(2eed176de531f44d10b7755141621050d72ad7ac) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "spcpanic.11",  0x0000, 0x0800, CRC(acea9df4) SHA1(7de2a82da8160ad1a01c32a516d10c19dc306051) )
	ROM_LOAD( "spcpanic.12",  0x0800, 0x0800, CRC(e83423d0) SHA1(eba1129537869f1ecb5afeeae19db19b134865f6) )
	ROM_LOAD( "spcpanic.10",  0x1000, 0x0800, CRC(c9631c2d) SHA1(e5ab95e19c1b22a798a70a1a6599bc1f5e853c60) )
	ROM_LOAD( "spcpanic.9",   0x1800, 0x0800, CRC(eec78b4c) SHA1(efd21d0a26b988a490c45315a7a121607f74d147) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.sp",    0x0000, 0x0020, CRC(35d43d2f) SHA1(2ce164c92ed7ba3ee26a907f0c5969ec3decca01) )

	ROM_REGION( 0x0800, "user1", 0 ) /* color map */
	ROM_LOAD( "spcpanic.8",   0x0000, 0x0800, CRC(7da0b321) SHA1(b450cc02de9cc27e3f336c626221c90c6961b51e) )
ROM_END

ROM_START( panicger )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spacepan.001", 0x0000, 0x0800, CRC(a6d9515a) SHA1(20fe6fa4cb10e83f97b77e19d9d4f883aba73d1a) )
	ROM_LOAD( "spacepan.002", 0x0800, 0x0800, CRC(cfc22663) SHA1(44036a69ca3463759c56637c3435a3305b102879) )
	ROM_LOAD( "spacepan.003", 0x1000, 0x0800, CRC(e1f36893) SHA1(689b77b4df15dc980d35cf245aca1affe46d6b21) )
	ROM_LOAD( "spacepan.004", 0x1800, 0x0800, CRC(01be297c) SHA1(d22856ef192d8239a3520f16bbe5a6f7f4c3adc8) )
	ROM_LOAD( "spacepan.005", 0x2000, 0x0800, CRC(e0d54805) SHA1(5852f69cee9a8f9984b175268bcfafe4f3f124ba) )
	ROM_LOAD( "spacepan.006", 0x2800, 0x0800, CRC(aae1458e) SHA1(79dd5992b81f316cf86efdb2809b7002e824e0e7) )
	ROM_LOAD( "spacepan.007", 0x3000, 0x0800, CRC(14e46e70) SHA1(f49f09a12b796f7a7713d872ecd12e246c56c261) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "spcpanic.11",  0x0000, 0x0800, CRC(acea9df4) SHA1(7de2a82da8160ad1a01c32a516d10c19dc306051) )
	ROM_LOAD( "spcpanic.12",  0x0800, 0x0800, CRC(e83423d0) SHA1(eba1129537869f1ecb5afeeae19db19b134865f6) )
	ROM_LOAD( "spcpanic.10",  0x1000, 0x0800, CRC(c9631c2d) SHA1(e5ab95e19c1b22a798a70a1a6599bc1f5e853c60) )
	ROM_LOAD( "spcpanic.9",   0x1800, 0x0800, CRC(eec78b4c) SHA1(efd21d0a26b988a490c45315a7a121607f74d147) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.sp",    0x0000, 0x0020, CRC(35d43d2f) SHA1(2ce164c92ed7ba3ee26a907f0c5969ec3decca01) )

	ROM_REGION( 0x0800, "user1", 0 ) /* color map */
	ROM_LOAD( "spcpanic.8",   0x0000, 0x0800, CRC(7da0b321) SHA1(b450cc02de9cc27e3f336c626221c90c6961b51e) )
ROM_END

ROM_START( cosmica ) /* later revision 7910-AII pcb; some roms are marked II-x; note that this set does NOT have the 1979 copyright date on the titlescreen! */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ii-1.e3",        0x0000, 0x0800, CRC(535ee0c5) SHA1(3ec3056b7fabe07ef49a9179114aa74be44a943e) ) /* tms2516 */
	ROM_LOAD( "ii-2.e4",        0x0800, 0x0800, CRC(ed3cf8f7) SHA1(6ba1d98d82400519e844b950cb2fb1274c06d89a) ) /* tms2516; has an & stamped on the chip */
	ROM_LOAD( "ii-3.e5",        0x1000, 0x0800, CRC(6a111e5e) SHA1(593be409bc969cece2ff88623e53c166b4dc43cd) ) /* tms2516 */
	ROM_LOAD( "ii-4.e6",        0x1800, 0x0800, CRC(c9b5ca2a) SHA1(3384b98954b6bc9a64e753b95757f61ce1d3c52e) ) /* tms2516 */
	ROM_LOAD( "ii-5.e7",        0x2000, 0x0800, CRC(43666d68) SHA1(e44492360a77d93aeaaaa0f38f4ac19732998559) ) /* tms2516; has an & stamped on the chip */

	ROM_REGION( 0x1000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "ii-7.n2",        0x0000, 0x0800, CRC(aa6c6079) SHA1(af4ab73e9e1c189290b26bf42adb511d5a347df9) ) // verify marking
	ROM_LOAD( "ii-6.n1",        0x0800, 0x0800, CRC(431e866c) SHA1(b007cd3cc856360a0247bd78bb49d173f5cef321) ) // verify marking

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "u7910.d9",    0x0000, 0x0020, CRC(dfb60f19) SHA1(d510327ff3492f098659c551f7245835f61a2959) ) // verify marking

	ROM_REGION( 0x0400, "user1", 0 ) /* color map */
	ROM_LOAD( "9.e2",        0x0000, 0x0400, CRC(ea4ee931) SHA1(d0a4afda4b493efb40286c2d67bf56a2a8b8da9d) ) /* 2708 */

	ROM_REGION( 0x0400, "user2", 0 ) /* starfield generator */
	ROM_LOAD( "8.k3",       0x0000, 0x0400, CRC(acbd4e98) SHA1(d33fe8bdc77bb18a3ffb369ea692210d1b890771) ) // verify marking
ROM_END

ROM_START( cosmica2 ) /* this set appears to be an intermediate version between the 'II' (cosmica) version and the early cosmica1 version; It still has the (C) 1979 titlescreen (which was removed on the II version since it may have came out in 1980?), and on all tms2708 eproms on a special rom daughterboard called "7910-V3"; one possible reason is that 2708 eproms became cheaper than tms2516s for a time, so production was switched to them for a while between the early and II versions? */
/* roms a-1 and b-2 match ii-1 from cosmica
 * roms c-3 and d-4 are unique
 * roms e-5 and f-6 match ii-3 from cosmica
 * rom g-7 probably SHOULD match first half of ii-4 from cosmica (and the current 'bad dump' g-7 rom does) but the sum16 from mameinfo doesn't match. since the game works fine I (LN) suspect the sum on mameinfo was wrong.
 * rom h-8 matches 2nd half of ii-4 from cosmica
 * roms i-9 and j-0 are unique
 */
	ROM_REGION( 0x10000, "maincpu", 0 ) /* all located on 7910-V3 sub pcb */
	ROM_LOAD( "a-1.e2",       0x0000, 0x0400, CRC(8a401b22) SHA1(9518fdbc09e935ede72af201028d80d09062a48d) ) /* tms2708 - sum16 6dd8 */
	ROM_LOAD( "b-2.d3",       0x0400, 0x0400, CRC(c8bf86b1) SHA1(324ce057ae9f152c7915d3af7837b09c8d48dec1) ) /* tms2708 - sum16 2fc0 */
	ROM_LOAD( "c-3.e3",       0x0800, 0x0400, CRC(699c849e) SHA1(90a58ab8ede9c31eec3df1f8f251b59858f85eb6) ) /* tms2708 - sum16 4767 */
	ROM_LOAD( "d-4.d4",       0x0c00, 0x0400, CRC(168e38da) SHA1(63c5f8346861aa7c70ad58a05977c7af413cbfaf) ) /* tms2708 - sum16 9148 */
	ROM_LOAD( "e-5.e4",       0x1000, 0x0400, CRC(80cc1fb8) SHA1(a301b236e372574ad3790aef72957cea249f18dc) ) /* tms2708 - sum16 afe2 */
	ROM_LOAD( "f-6.d5",       0x1400, 0x0400, CRC(0dc464f7) SHA1(9ad68fd100bd3021202c3831477c8715b4b8f6b8) ) /* tms2708 - sum16 b403 */
	ROM_LOAD( "g-7.e5",       0x1800, 0x0400, BAD_DUMP CRC(d5381c54) SHA1(57c170d02aa6d41f7cd4542e084af95ba3fcff7d) ) /* tms2708 - bad? sum16 should be d1aa according to mameinfo; is afda, but works fine... */
	ROM_LOAD( "h-8.d6",       0x1c00, 0x0400, CRC(2175fe6f) SHA1(930c70f5d1509f82581bbf760033eb97c34cfce6) ) /* tms2708 - sum16 a096 */
	ROM_LOAD( "i-9.e6",       0x2000, 0x0400, CRC(3bb57720) SHA1(2d1edcad57767a4fa2c7713726ed0cb1203f6fbc) ) /* tms2708 - sum16 9b55 */
	ROM_LOAD( "j-0.d7",       0x2400, 0x0400, CRC(4ff70f45) SHA1(791499be62a7b91bde75e7a7ab6c546f5fb63027) ) /* tms2708 - sum16 7c3c */

	ROM_REGION( 0x1000, "gfx1", 0 ) /* sprites, on mainboard (note: the locations of these two MIGHT be switched around) */
	ROM_LOAD( "k-8.n2",        0x0000, 0x0800, CRC(aa6c6079) SHA1(af4ab73e9e1c189290b26bf42adb511d5a347df9) ) /* Fujitsu MB8516 - sum16 4d9c */
	ROM_LOAD( "l-7.n1",        0x0800, 0x0800, CRC(431e866c) SHA1(b007cd3cc856360a0247bd78bb49d173f5cef321) ) /* Fujitsu MB8516 - sum16 bb6b */

	ROM_REGION( 0x0020, "proms", 0 )/* on mainboard */
	ROM_LOAD( "u7910.d9",    0x0000, 0x0020, CRC(dfb60f19) SHA1(d510327ff3492f098659c551f7245835f61a2959) ) /* MMI 6331-1 - sum16 0706 */

	ROM_REGION( 0x0400, "user1", 0 ) /* color map, on mainboard */
	ROM_LOAD( "9-9.e2",        0x0000, 0x0400, CRC(ea4ee931) SHA1(d0a4afda4b493efb40286c2d67bf56a2a8b8da9d) ) /* tms2708 - sum16 9027 */

	ROM_REGION( 0x0400, "user2", 0 ) /* starfield generator */
	ROM_LOAD( "8-8.k3",       0x0000, 0x0400, CRC(acbd4e98) SHA1(d33fe8bdc77bb18a3ffb369ea692210d1b890771) ) /* tms2708; located on 7910-BII subpcb, sum16 97c8 */
ROM_END

ROM_START( cosmica1 ) /* earlier 7910-A pcb, had lots of rework; roms do NOT have 'II' markings stamped on them as on the cosmica set */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.e3",        0x0000, 0x0800, CRC(2e44f50e) SHA1(9d87c519a498c47296aa02453806fba95fc4c455) ) /* tms2516 */
	ROM_LOAD( "2.e4",        0x0800, 0x0800, CRC(9e5c5281) SHA1(eaf9ca2a37196df758453a73ee145c83e0e3c476) ) /* tms2516; has an & stamped on the chip */
	ROM_LOAD( "3.e5",        0x1000, 0x0800, CRC(9e1309db) SHA1(1afbaa8da68abc90bf6f4acd9df9e4d3610f10ce) ) /* tms2516 */
	ROM_LOAD( "4.e6",        0x1800, 0x0800, CRC(ba4a9295) SHA1(c7ed9daf48e01ef87253addb0a7e5c62fa1f37cd) ) /* tms2516 */
	ROM_LOAD( "5.e7",        0x2000, 0x0800, CRC(2106c82a) SHA1(fa807cf0321813e20dc2d2f2a8ae3778496fa97c) ) /* tms2516; has an & stamped on the chip */

	ROM_REGION( 0x1000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "7.n2",        0x0000, 0x0800, CRC(ee3e86fc) SHA1(4fb5fbee06b2d590a83519761f63ec9d6b90efb3) ) /* tms2516 */
	ROM_LOAD( "6.n1",        0x0800, 0x0800, CRC(81c86ca0) SHA1(4cea1a61523ae1c3c681b1102b8e18ab26d0040a) ) /* tms2516 */

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "u7910.d9",    0x0000, 0x0020, CRC(dfb60f19) SHA1(d510327ff3492f098659c551f7245835f61a2959) ) /* MMI 6331 */

	ROM_REGION( 0x0400, "user1", 0 ) /* color map */
	ROM_LOAD( "9.e2",        0x0000, 0x0400, CRC(ea4ee931) SHA1(d0a4afda4b493efb40286c2d67bf56a2a8b8da9d) ) /* 2708 */

	ROM_REGION( 0x0400, "user2", 0 ) /* starfield generator */
	ROM_LOAD( "8.k3",       0x0000, 0x0400, CRC(acbd4e98) SHA1(d33fe8bdc77bb18a3ffb369ea692210d1b890771) ) /* 2708; located on sub pcb */
ROM_END

ROM_START( cosmicg )
	ROM_REGION( 0x10000, "maincpu", 0 )  /* 8k for code */
	ROM_LOAD( "cosmicg1.bin", 0x0000, 0x0400, CRC(e1b9f894) SHA1(bab7fd9b3db145a889542653191905b6efc5ce75) )
	ROM_LOAD( "cosmicg2.bin", 0x0400, 0x0400, CRC(35c75346) SHA1(4e50eaa0b50ab04802dc63992ad2600c227301ad) )
	ROM_LOAD( "cosmicg3.bin", 0x0800, 0x0400, CRC(82a49b48) SHA1(4cf9f684f3eb18b99a88ca879bb7083b1334f0cc) )
	ROM_LOAD( "cosmicg4.bin", 0x0c00, 0x0400, CRC(1c1c934c) SHA1(011b2b3ec4d31869fda13a3654c7bc51f3ce4dc2) )
	ROM_LOAD( "cosmicg5.bin", 0x1000, 0x0400, CRC(b1c00fbf) SHA1(136267f75e2d5b445695cabef4538f986e6f1b10) )
	ROM_LOAD( "cosmicg6.bin", 0x1400, 0x0400, CRC(f03454ce) SHA1(32c87f369475c7154fe3243d2c7be4a25444e530) )
	ROM_LOAD( "cosmicg7.bin", 0x1800, 0x0400, CRC(f33ebae7) SHA1(915bca53d5356e12c94ec765103ceced7306d1dd) )
	ROM_LOAD( "cosmicg8.bin", 0x1c00, 0x0400, CRC(472e4990) SHA1(d5797b9d89446aa6533f7515e6a5fc8368d82f91) )

	ROM_REGION( 0x0400, "user1", 0 ) /* color map */
	ROM_LOAD( "cosmicg9.bin", 0x0000, 0x0400, CRC(689c2c96) SHA1(ddfdc3fd29c56fdebd3b1c3443a7c39f567d5355) )
ROM_END


ROM_START( cosmicgi )
	ROM_REGION( 0x10000, "maincpu", 0 )  /* 8k for code */
	ROM_LOAD( "1g118.2h", 0x0000, 0x0400, CRC(4bda1711) SHA1(746fd15dbe08c9e2af74547c19a55a84f7b65303) )
	ROM_LOAD( "2g118.3h", 0x0400, 0x0400, CRC(3c10b2ba) SHA1(127a950d90420417a91aa3c8fabec7d7e7d526f5) )
	ROM_LOAD( "3.4h",     0x0800, 0x0400, CRC(82a49b48) SHA1(4cf9f684f3eb18b99a88ca879bb7083b1334f0cc) )
	ROM_LOAD( "4g118.5h", 0x0c00, 0x0400, CRC(42bb0611) SHA1(3894e4372f1443402ea7145b1101e1219fe2cde2) ) // changes in here cause trails when you move the ship, PCB does the same and ROM gives the same read every time, possible a bit has been flipped tho. check
	ROM_LOAD( "5.6h",     0x1000, 0x0400, CRC(b1c00fbf) SHA1(136267f75e2d5b445695cabef4538f986e6f1b10) )
	ROM_LOAD( "6.7h",     0x1400, 0x0400, CRC(f03454ce) SHA1(32c87f369475c7154fe3243d2c7be4a25444e530) )
	ROM_LOAD( "7.8h",     0x1800, 0x0400, CRC(84656c97) SHA1(2180faa07dd5bc618c80ae033babfc1191a0b890) ) // standard label but different anyway?
	ROM_LOAD( "8g128.9h", 0x1c00, 0x0400, CRC(7f48307c) SHA1(5929c131d790b0c8f9113730715531809c6840e2) )

	ROM_REGION( 0x0400, "user1", 0 ) /* color map */ // population of this is optional, board runs as b&w without (this board didn't have it populated)
	ROM_LOAD( "cosmicg9.bin", 0x0000, 0x0400, CRC(689c2c96) SHA1(ddfdc3fd29c56fdebd3b1c3443a7c39f567d5355) )
ROM_END


/* rom 9 not dumped according to readme? */
ROM_START( magspot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ms1.bin",      0x0000, 0x0800, CRC(59e9019d) SHA1(3c64ae956ec4eed988018b89c986ad8f6f065fe0) )
	ROM_LOAD( "ms2.bin",      0x0800, 0x0800, CRC(98b913b1) SHA1(2ce86f5069e2664e2ea44bda567ca26432fd59f7) )
	ROM_LOAD( "ms3.bin",      0x1000, 0x0800, CRC(ea58c124) SHA1(7551c14ed9563e3aed7220cc03f7bca4029b3a4e) )
	ROM_LOAD( "ms5.bin",      0x1800, 0x0800, CRC(4302a658) SHA1(9590be8db27b7122c87cfb27f8e09c2ecbf6fbd0) )
	ROM_LOAD( "ms4.bin",      0x2000, 0x0800, CRC(088582ab) SHA1(ad2d86184b4a6ee74464d1df40f4e841434c46c8) )
	ROM_LOAD( "ms6.bin",      0x2800, 0x0800, CRC(e6bf492c) SHA1(ada3a33c54b6c02f3fb9590181fceefafdc429bc) )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "ms8.bin",      0x0000, 0x0800, CRC(9e1d63a2) SHA1(d8642e515871da44880e105e6891c4b25222744f) )
	ROM_LOAD( "ms7.bin",      0x0800, 0x0800, CRC(1ab338d3) SHA1(4e3bf93f94119fd10c40953245cec735db8417fb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ms.d9",        0x0000, 0x0020, CRC(36e2aa2a) SHA1(4813b013cb8260157858e3adc7323efc6654e170) )

	ROM_REGION( 0x0400, "user1", 0 ) /* color map */
	ROM_LOAD( "ms.e2",        0x0000, 0x0400, CRC(89f23ebd) SHA1(a56bda82f8be8e541a50d2a411ada89a6d9c0373) )
ROM_END

ROM_START( magspot2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ms.e3",        0x0000, 0x0800, CRC(c0085ade) SHA1(ab60ba7c0e45ea2576d935135e930e2fdf165867) )
	ROM_LOAD( "ms.e4",        0x0800, 0x0800, CRC(d534a68b) SHA1(fd3b5e619b22a8c53e3c6f5f5351068a3f26eb61) )
	ROM_LOAD( "ms.e5",        0x1000, 0x0800, CRC(25513b2a) SHA1(c7f3d9a53cb7e7cf523ff710c333dbc744088e31) )
	ROM_LOAD( "ms.e7",        0x1800, 0x0800, CRC(8836bbc4) SHA1(9da6c1b4e9a446108bc324e7fc280bfaeaf50504) )
	ROM_LOAD( "ms.e6",        0x2000, 0x0800, CRC(6a08ab94) SHA1(5d9272a5304546cef6668c975e815f6750bcfa15) )
	ROM_LOAD( "ms.e8",        0x2800, 0x0800, CRC(77c6d109) SHA1(bb265bd56d4d597d2ef75d169d5d30db1499e3be) )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "ms.n2",        0x0000, 0x0800, CRC(9e1d63a2) SHA1(d8642e515871da44880e105e6891c4b25222744f) )
	ROM_LOAD( "ms.n1",        0x0800, 0x0800, CRC(1ab338d3) SHA1(4e3bf93f94119fd10c40953245cec735db8417fb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ms.d9",        0x0000, 0x0020, CRC(36e2aa2a) SHA1(4813b013cb8260157858e3adc7323efc6654e170) )

	ROM_REGION( 0x0400, "user1", 0 ) /* color map */
	ROM_LOAD( "ms.e2",        0x0000, 0x0400, CRC(89f23ebd) SHA1(a56bda82f8be8e541a50d2a411ada89a6d9c0373) )
ROM_END

ROM_START( devzone )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dv1.e3",       0x0000, 0x0800, CRC(c70faf00) SHA1(d3f0f071e6c7552724eba64a7182637dae4438c7) )
	ROM_LOAD( "dv2.e4",       0x0800, 0x0800, CRC(eacfed61) SHA1(493c0d21fd1574b12978dd1f52e8735df6c1732c) )
	ROM_LOAD( "dv3.e5",       0x1000, 0x0800, CRC(7973317e) SHA1(d236e3dad8c991c32a2550e561518b522a4580bc) )
	ROM_LOAD( "dv5.e7",       0x1800, 0x0800, CRC(b71a3989) SHA1(aad14021ee569e221ea632416d6a006e60dd94e5) )
	ROM_LOAD( "dv4.e6",       0x2000, 0x0800, CRC(a58c5b8c) SHA1(7ff08007aedd2ff1d7ef64263da92a5b77ae2dc4) )
	ROM_LOAD( "dv6.e8",       0x2800, 0x0800, CRC(3930fb67) SHA1(919883e833d6caa8fe7c3ceaa184575a3b4932b6) )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "dv8.n2",       0x0000, 0x0800, CRC(da1cbec1) SHA1(08a668f19c68335f4fc9f98cd53b44047dd8aad9) )
	ROM_LOAD( "dv7.n1",       0x0800, 0x0800, CRC(e7562fcf) SHA1(0a0833dbb8d4be69fbf8897aa3e045a87ae42024) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ms.d9",        0x0000, 0x0020, CRC(36e2aa2a) SHA1(4813b013cb8260157858e3adc7323efc6654e170) )

	ROM_REGION( 0x0400, "user1", 0 ) /* color map */
	ROM_LOAD( "dz9.e2",       0x0000, 0x0400, CRC(693855b6) SHA1(1c29d72be511c1d38b30b9534d647d0813b2ef57) )

	ROM_REGION( 0x0800, "user2", 0 ) /* grid horizontal line positions */
	ROM_LOAD( "ic12.sub",     0x0000, 0x0800, CRC(f61c1c45) SHA1(9016710409ae2bccfc60f8e3d1131c125333c034) )

	ROM_REGION( 0x0020, "user3", 0 ) /* grid vertical line positions */
	ROM_LOAD( "ic1.sub",      0x0000, 0x0020, CRC(df974878) SHA1(2ef2e1b771923f9a0bfe1841444de61200298605) )
ROM_END

ROM_START( devzone2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p10_1.e3",     0x0000, 0x0800, BAD_DUMP CRC(38bd45a4) SHA1(192eee64ff53c20fb5b369703b52a5bb3976ba1d)  )
	ROM_LOAD( "my4_2.e4",     0x0800, 0x0800, BAD_DUMP CRC(e1637800) SHA1(3705ce1f02f3fefec0285f5db6a7606e6cec1bac)  )
	ROM_LOAD( "ms6_3.e5",     0x1000, 0x0800, BAD_DUMP CRC(c1952e2f) SHA1(d42f0f547e989a71254957e5e634ac359e72bb14)  )
	ROM_LOAD( "mx6_5.e7",     0x1800, 0x0800, BAD_DUMP CRC(c5394215) SHA1(8c970f6a8d34963bc4848f2bef90cee850c9c28d)  )
	ROM_LOAD( "my1_4.e6",     0x2000, 0x0800, BAD_DUMP CRC(5d965d93) SHA1(49fe79e4b5cec1c7aa2f8e1eb750b39bb7dda16c)  )
	ROM_LOAD( "mz7_6.e8",     0x2800, 0x0800, BAD_DUMP CRC(8504e8c9) SHA1(40e08ff38673544c734a9fc19b38edaa8cc74f23)  )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "my8_8.n2",     0x0000, 0x0800, CRC(18abce02) SHA1(5cac11f4e6f1a4801bd02007399a906cdff66b85) )
	ROM_LOAD( "mx3_7.n1",     0x0800, 0x0800, CRC(c089c9e3) SHA1(2fb725338a19d5d4f9e445e7d46d105b8db9733c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ms.d9",        0x0000, 0x0020, CRC(36e2aa2a) SHA1(4813b013cb8260157858e3adc7323efc6654e170) )

	ROM_REGION( 0x0400, "user1", 0 ) /* color map */
	ROM_LOAD( "dz9.e2",       0x0000, 0x0400, CRC(693855b6) SHA1(1c29d72be511c1d38b30b9534d647d0813b2ef57) )

	ROM_REGION( 0x0800, "user2", 0 ) /* grid horizontal line positions */
	ROM_LOAD( "ic12.sub",     0x0000, 0x0800, CRC(f61c1c45) SHA1(9016710409ae2bccfc60f8e3d1131c125333c034) )

	ROM_REGION( 0x0020, "user3", 0 ) /* grid vertical line positions */
	ROM_LOAD( "ic1.sub",      0x0000, 0x0020, CRC(df974878) SHA1(2ef2e1b771923f9a0bfe1841444de61200298605) )
ROM_END

ROM_START( nomnlnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x0800, CRC(ba117ba6) SHA1(7399e7ac8a585ed6502ea0d740850b1ed2dc5bcd) )
	ROM_LOAD( "2.bin",        0x0800, 0x0800, CRC(e5ed654f) SHA1(c26dc12ade6dc63392945ec0caca229d936f7f89) )
	ROM_LOAD( "3.bin",        0x1000, 0x0800, CRC(7fc42724) SHA1(0f8fdfad0a2557b9dd99ae3890c37bbc5c59bc89) )
	ROM_LOAD( "5.bin",        0x1800, 0x0800, CRC(9cc2f1d9) SHA1(453c67b613550c84364f445705019188bb580d64) )
	ROM_LOAD( "4.bin",        0x2000, 0x0800, CRC(0e8cd46a) SHA1(14cf9017e408b862a4ed63bb8acd37064b3919a8) )
	ROM_LOAD( "6.bin",        0x2800, 0x0800, CRC(ba472ba5) SHA1(49be1500b3805a19c7210e53ad5c2c4a5876bf4e) )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "nml8.n2",      0x0000, 0x0800, CRC(739009b4) SHA1(bbabd6ce7b1ded025f20120adaebdb97fb755ef0) )
	ROM_LOAD( "nml7.n1",      0x0800, 0x0800, CRC(d08ed22f) SHA1(33f450b6f63110bf804105280dc679f1591422f6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "nml.d9",       0x0000, 0x0020, CRC(65e911f9) SHA1(6420a03195f63edeed17cc3a235e46e3f88d2037) )

	ROM_REGION( 0x0400, "user1", 0 ) /* color map */
	ROM_LOAD( "nl9.e2",       0x0000, 0x0400, CRC(9e05f14e) SHA1(76fc0b2b12cc9a0a64b539d2e75edefdb4a2ae61) )

	ROM_REGION( 0x0800, "user2", 0 ) /* tree + river */
	ROM_LOAD( "nl10.ic4",     0x0000, 0x0400, CRC(5b13f64e) SHA1(b04d2423fb443d46fff69c031b0312d956a5b789) )
	ROM_LOAD( "nl11.ic7",     0x0400, 0x0400, CRC(e717b241) SHA1(6d234a75514e22d484dc027db5bb85cf8b58f4f2) )
ROM_END

ROM_START( nomnlndg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nml1.e3",      0x0000, 0x0800, CRC(e212ed91) SHA1(135c20fc97790769d5e1619d7ac844a1d3f6aace) )
	ROM_LOAD( "nml2.e4",      0x0800, 0x0800, CRC(f66ef3d8) SHA1(c42a325dd952cda074ef2857e7fa5154f0b7c7ce) )
	ROM_LOAD( "nml3.e5",      0x1000, 0x0800, CRC(d422fc8a) SHA1(18cafc462ce0800fea2af277439827dc1f4fc91b) )
	ROM_LOAD( "nml5.e7",      0x1800, 0x0800, CRC(d58952ac) SHA1(1c82a49cc1f0203e6436c5292ebd6e9004bd6a84) )
	ROM_LOAD( "nml4.e6",      0x2000, 0x0800, CRC(994c9afb) SHA1(c8e6af30d9b2cb5ca52fa325c6ac9a41413d067c) )
	ROM_LOAD( "nml6.e8",      0x2800, 0x0800, CRC(01ed2d8c) SHA1(bfa31e9100a1f9276c521ed8699e1cb0d067e0fa) )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "nml8.n2",      0x0000, 0x0800, CRC(739009b4) SHA1(bbabd6ce7b1ded025f20120adaebdb97fb755ef0) )
	ROM_LOAD( "nml7.n1",      0x0800, 0x0800, CRC(d08ed22f) SHA1(33f450b6f63110bf804105280dc679f1591422f6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "nml.d9",       0x0000, 0x0020, CRC(65e911f9) SHA1(6420a03195f63edeed17cc3a235e46e3f88d2037) )

	ROM_REGION( 0x0400, "user1", 0 ) /* color map */
	ROM_LOAD( "nl9.e2",       0x0000, 0x0400, CRC(9e05f14e) SHA1(76fc0b2b12cc9a0a64b539d2e75edefdb4a2ae61) )

	ROM_REGION( 0x0800, "user2", 0 ) /* tree + river */
	ROM_LOAD( "nl10.ic4",     0x0000, 0x0400, CRC(5b13f64e) SHA1(b04d2423fb443d46fff69c031b0312d956a5b789) )
	ROM_LOAD( "nl11.ic7",     0x0400, 0x0400, CRC(e717b241) SHA1(6d234a75514e22d484dc027db5bb85cf8b58f4f2) )
ROM_END


DRIVER_INIT_MEMBER(cosmic_state,cosmicg)
{
	/* Program ROMs have data pins connected different from normal */
	offs_t offs, len;
	UINT8 *rom;
	len = memregion("maincpu")->bytes();
	rom = memregion("maincpu")->base();
	for (offs = 0; offs < len; offs++)
	{
		UINT8 scrambled = rom[offs];

		UINT8 normal = (scrambled >> 3 & 0x11)
						| (scrambled >> 1 & 0x22)
						| (scrambled << 1 & 0x44)
						| (scrambled << 3 & 0x88);

		rom[offs] = normal;
	}

	m_sound_enabled = 0;
	m_march_select = 0;
	m_gun_die_select = 0;
}


DRIVER_INIT_MEMBER(cosmic_state,cosmica)
{
	m_sound_enabled = 1;
	m_dive_bomb_b_select = 0;
}


DRIVER_INIT_MEMBER(cosmic_state,devzone)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4807, 0x4807,write8_delegate(FUNC(cosmic_state::cosmic_background_enable_w),this));
}


DRIVER_INIT_MEMBER(cosmic_state,nomnlnd)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5000, 0x5001, read8_delegate(FUNC(cosmic_state::nomnlnd_port_0_1_r),this));
	m_maincpu->space(AS_PROGRAM).nop_write(0x4800, 0x4800);
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4807, 0x4807, write8_delegate(FUNC(cosmic_state::cosmic_background_enable_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x480a, 0x480a, write8_delegate(FUNC(dac_device::write_unsigned8),(dac_device*)m_dac));
}

DRIVER_INIT_MEMBER(cosmic_state,panic)
{
	m_sound_enabled = 1;
}


GAME( 1979, cosmicg,  0,       cosmicg,  cosmicg, cosmic_state,  cosmicg, ROT270, "Universal", "Cosmic Guerilla", MACHINE_IMPERFECT_SOUND | MACHINE_NO_COCKTAIL /*| MACHINE_SUPPORTS_SAVE */)
GAME( 1979, cosmicgi, cosmicg, cosmicg,  cosmicg, cosmic_state,  cosmicg, ROT270, "bootleg (Inder)", "Cosmic Guerilla (Spanish bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_NO_COCKTAIL  /*| MACHINE_SUPPORTS_SAVE */)
GAME( 1979, cosmica,  0,       cosmica,  cosmica, cosmic_state,  cosmica, ROT270, "Universal", "Cosmic Alien (version II)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, cosmica1, cosmica, cosmica,  cosmica, cosmic_state,  cosmica, ROT270, "Universal", "Cosmic Alien (first version)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, cosmica2, cosmica, cosmica,  cosmica, cosmic_state,  cosmica, ROT270, "Universal", "Cosmic Alien (early version II?)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, nomnlnd,  0,       nomnlnd,  nomnlnd, cosmic_state,  nomnlnd, ROT270, "Universal", "No Man's Land", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, nomnlndg, nomnlnd, nomnlnd,  nomnlndg, cosmic_state, nomnlnd, ROT270, "Universal (Gottlieb license)", "No Man's Land (Gottlieb)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, magspot,  0,       magspot,  magspot, driver_device,  0,       ROT270, "Universal", "Magical Spot", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, magspot2, 0,       magspot,  magspot, driver_device,  0,       ROT270, "Universal", "Magical Spot II", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, panic,    0,       panic,    panic, cosmic_state,    panic,   ROT270, "Universal", "Space Panic (version E)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, panic2,   panic,   panic,    panic, cosmic_state,    panic,   ROT270, "Universal", "Space Panic (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, panic3,   panic,   panic,    panic, cosmic_state,    panic,   ROT270, "Universal", "Space Panic (set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, panich,   panic,   panic,    panic, cosmic_state,    panic,   ROT270, "Universal", "Space Panic (harder)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, panicger, panic,   panic,    panic, cosmic_state,    panic,   ROT270, "Universal (ADP Automaten license)", "Space Panic (German)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, devzone,  0,       devzone,  devzone, cosmic_state,  devzone, ROT270, "Universal", "Devil Zone", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, devzone2, devzone, devzone,  devzone2, cosmic_state, devzone, ROT270, "Universal", "Devil Zone (easier)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
