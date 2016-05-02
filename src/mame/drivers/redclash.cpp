// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

Zero Hour / Red Clash

runs on hardware similar to Lady Bug

driver by inkling

Notes:
- In the Tehkan set (redclsha) the ship doesn't move during attract mode. Earlier version?
  Gameplay is different too.

TODO:
- Colors might be right, need screen shots to verify

- Some graphical problems in both games, but without screenshots its hard to
  know what we're aiming for

- Sound (analog, schematics available for Zero Hour)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "includes/redclash.h"


/* Sound comm between CPU's */
READ8_MEMBER(redclash_state::sraider_sound_low_r)
{
	return m_sound_low;
}

READ8_MEMBER(redclash_state::sraider_sound_high_r)
{
	return m_sound_high;
}

WRITE8_MEMBER(redclash_state::sraider_sound_low_w)
{
	m_sound_low = data;
}

WRITE8_MEMBER(redclash_state::sraider_sound_high_w)
{
	m_sound_high = data;
}

/* Protection? */
READ8_MEMBER(redclash_state::sraider_8005_r)
{
	/* This must return X011111X or cpu #1 will hang */
	/* see code at rst $10 */
	return 0x3e;
}

/* Unknown IO */
WRITE8_MEMBER(redclash_state::sraider_misc_w)
{
	switch(offset)
	{
		/* These 8 bits are stored in the latch at A7 */
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			m_weird_value[offset & 7] = data & 1;
			break;
		/* These 6 bits are stored in the latch at N7 */
		case 0x08:
			m_sraider_0x30 = data&0x3f;
			break;
		/* These 6 bits are stored in the latch at N8 */
		case 0x10:
			m_sraider_0x38 = data&0x3f;
			break;
		default:
			osd_printf_debug("(%04X) write to %02X\n", space.device().safe_pc(), offset);
			break;
	}
}

static ADDRESS_MAP_START( sraider_cpu1_map, AS_PROGRAM, 8, redclash_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x73ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x8005, 0x8005) AM_READ(sraider_8005_r)  // protection check?
	AM_RANGE(0x8006, 0x8006) AM_WRITE(sraider_sound_low_w)
	AM_RANGE(0x8007, 0x8007) AM_WRITE(sraider_sound_high_w)
	AM_RANGE(0x9000, 0x9000) AM_READ_PORT("IN0")
	AM_RANGE(0x9001, 0x9001) AM_READ_PORT("IN1")
	AM_RANGE(0x9002, 0x9002) AM_READ_PORT("DSW0")
	AM_RANGE(0x9003, 0x9003) AM_READ_PORT("DSW1")
	AM_RANGE(0xd000, 0xd3ff) AM_WRITE(ladybug_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd400, 0xd7ff) AM_WRITE(ladybug_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xe000, 0xe000) AM_WRITENOP  //unknown 0x10 when in attract, 0x20 when coined/playing
ADDRESS_MAP_END


static ADDRESS_MAP_START( sraider_cpu2_map, AS_PROGRAM, 8, redclash_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_READ(sraider_sound_low_r)
	AM_RANGE(0xa000, 0xa000) AM_READ(sraider_sound_high_r)
	AM_RANGE(0xc000, 0xc000) AM_READNOP //some kind of sync
	AM_RANGE(0xe000, 0xe0ff) AM_WRITEONLY AM_SHARE("grid_data")
	AM_RANGE(0xe800, 0xe800) AM_WRITE(sraider_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sraider_cpu2_io_map, AS_IO, 8, redclash_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("sn1", sn76489_device, write)
	AM_RANGE(0x08, 0x08) AM_DEVWRITE("sn2", sn76489_device, write)
	AM_RANGE(0x10, 0x10) AM_DEVWRITE("sn3", sn76489_device, write)
	AM_RANGE(0x18, 0x18) AM_DEVWRITE("sn4", sn76489_device, write)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("sn5", sn76489_device, write)
	AM_RANGE(0x28, 0x3f) AM_WRITE(sraider_misc_w)  // lots unknown
ADDRESS_MAP_END


WRITE8_MEMBER( redclash_state::irqack_w )
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

static ADDRESS_MAP_START( zerohour_map, AS_PROGRAM, 8, redclash_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x37ff) AM_RAM
	AM_RANGE(0x3800, 0x3bff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x4000, 0x43ff) AM_RAM_WRITE(redclash_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4800, 0x4800) AM_READ_PORT("IN0")    /* IN0 */
	AM_RANGE(0x4801, 0x4801) AM_READ_PORT("IN1")    /* IN1 */
	AM_RANGE(0x4802, 0x4802) AM_READ_PORT("DSW1")   /* DSW0 */
	AM_RANGE(0x4803, 0x4803) AM_READ_PORT("DSW2")   /* DSW1 */
	AM_RANGE(0x5000, 0x5007) AM_WRITENOP    /* to sound board */
	AM_RANGE(0x5800, 0x5800) AM_WRITE(redclash_star0_w)
	AM_RANGE(0x5801, 0x5804) AM_WRITENOP    /* to sound board */
	AM_RANGE(0x5805, 0x5805) AM_WRITE(redclash_star1_w)
	AM_RANGE(0x5806, 0x5806) AM_WRITE(redclash_star2_w)
	AM_RANGE(0x5807, 0x5807) AM_WRITE(redclash_flipscreen_w)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(redclash_star_reset_w)
	AM_RANGE(0x7800, 0x7800) AM_WRITE(irqack_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( redclash_map, AS_PROGRAM, 8, redclash_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
//  AM_RANGE(0x3000, 0x3000) AM_WRITENOP
//  AM_RANGE(0x3800, 0x3800) AM_WRITENOP
	AM_RANGE(0x4000, 0x43ff) AM_RAM_WRITE(redclash_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4800, 0x4800) AM_READ_PORT("IN0")    /* IN0 */
	AM_RANGE(0x4801, 0x4801) AM_READ_PORT("IN1")    /* IN1 */
	AM_RANGE(0x4802, 0x4802) AM_READ_PORT("DSW1")   /* DSW0 */
	AM_RANGE(0x4803, 0x4803) AM_READ_PORT("DSW2")   /* DSW1 */
	AM_RANGE(0x5000, 0x5007) AM_WRITENOP    /* to sound board */
	AM_RANGE(0x5800, 0x5800) AM_WRITE(redclash_star0_w)
	AM_RANGE(0x5801, 0x5801) AM_WRITE(redclash_gfxbank_w)
	AM_RANGE(0x5805, 0x5805) AM_WRITE(redclash_star1_w)
	AM_RANGE(0x5806, 0x5806) AM_WRITE(redclash_star2_w)
	AM_RANGE(0x5807, 0x5807) AM_WRITE(redclash_flipscreen_w)
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0x6800, 0x6bff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x7000, 0x7000) AM_WRITE(redclash_star_reset_w)
	AM_RANGE(0x7800, 0x7800) AM_WRITE(irqack_w)
ADDRESS_MAP_END

/*
  This game doesn't have VBlank interrupts.
  Interrupts are still used, but they are related to coin
  slots. Left slot generates an IRQ, Right slot a NMI.
*/
INPUT_CHANGED_MEMBER( redclash_state::left_coin_inserted )
{
	if(newval)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( redclash_state::right_coin_inserted )
{
	if(newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static INPUT_PORTS_START( redclash )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* Note that there are TWO VBlank inputs, one is active low, the other active */
	/* high. There are probably other differencies in the hardware, but emulating */
	/* them this way is enough to get the game running. */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Difficulty?" )
	PORT_DIPSETTING(    0x03, "Easy?" )
	PORT_DIPSETTING(    0x02, "Medium?" )
	PORT_DIPSETTING(    0x01, "Hard?" )
	PORT_DIPSETTING(    0x00, "Hardest?" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, "High Score" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x10, "10000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "7" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_9C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_9C ) )

	PORT_START("FAKE")
	/* The coin slots are not memory mapped. Coin Left causes a NMI, */
	/* Coin Right an IRQ. This fake input port is used by the interrupt */
	/* handler to be notified of coin insertions. We use IMPULSE to */
	/* trigger exactly one interrupt, without having to check when the */
	/* user releases the key. */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, redclash_state, left_coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, redclash_state, right_coin_inserted, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( zerohour )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* Note that there are TWO VBlank inputs, one is active low, the other active */
	/* high. There are probably other differencies in the hardware, but emulating */
	/* them this way is enough to get the game running. */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:8" )    /* Switches 6-8 are not used */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:6" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:4,3")   /* Also determines the default topscore, 0 for "No Bonus" */
	PORT_DIPSETTING(    0x00, "No Bonus" )
	PORT_DIPSETTING(    0x30, "5000" )
	PORT_DIPSETTING(    0x20, "8000" )
	PORT_DIPSETTING(    0x10, "10000" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x40, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:4,3,2,1")
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )    /* all other combinations give 1C_1C */
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:8,7,6,5")
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )    /* all other combinations give 1C_1C */
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )

	PORT_START("FAKE")
	/* The coin slots are not memory mapped. Coin Left causes a NMI, */
	/* Coin Right an IRQ. This fake input port is used by the interrupt */
	/* handler to be notified of coin insertions. We use IMPULSE to */
	/* trigger exactly one interrupt, without having to check when the */
	/* user releases the key. */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, redclash_state, left_coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, redclash_state, right_coin_inserted, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( sraider )
	PORT_START("IN0")   /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW0")  /* DSW0 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, "High Score Names" )
	PORT_DIPSETTING(    0x00, "3 Letters" )
	PORT_DIPSETTING(    0x04, "10 Letters" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x40, "5" )

	/* Free Play setting works when it's set for both */
	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	/* settings 0x00 through 0x05 all give 1 Coin/1 Credit */
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	/* settings 0x00 through 0x50 all give 1 Coin/1 Credit */
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 8*8, 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 1, 0 },
	{ STEP8(0,2) },
	{ STEP8(7*16,-16) },
	16*8
};

static const gfx_layout spritelayout16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 1, 0 },
	{ STEP8(24*2,2), STEP8(8*64+24*2,2) },
	{ STEP8(23*64,-64), STEP8(7*64,-64) },
	64*32
};

static const gfx_layout spritelayout24x24 =
{
	24,24,
	RGN_FRAC(1,1),
	2,
	{ 1, 0 },
	{ STEP8(0,2), STEP8(8*2,2), STEP8(16*2,2) },
	{ STEP8(23*64,-64), STEP8(15*64,-64), STEP8(7*64,-64) },
	64*32
};

static const gfx_layout spritelayout16x16bis =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 1, 0 },
	{ STEP8(0,2), STEP8(8*2,2) },
	{ STEP8(15*64,-64), STEP8(7*64,-64) },
	32*32
};

static GFXDECODE_START( redclash )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,          0,  8 )
	GFXDECODE_ENTRY( "gfx3", 0x0000, spritelayout8x8,   4*8, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout16x16, 4*8, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout24x24, 4*8, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout16x16bis, 4*8, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0004, spritelayout16x16bis, 4*8, 16 )
GFXDECODE_END

static const gfx_layout charlayout2 =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	2,  /* 2 bits per pixel */
	{ 0, 512*8*8 }, /* the two bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};
static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	128,    /* 128 sprites */
	2,  /* 2 bits per pixel */
	{ 1, 0 },   /* the two bitplanes are packed in two consecutive bits */
	{ 0, 2, 4, 6, 8, 10, 12, 14,
			8*16+0, 8*16+2, 8*16+4, 8*16+6, 8*16+8, 8*16+10, 8*16+12, 8*16+14 },
	{ 23*16, 22*16, 21*16, 20*16, 19*16, 18*16, 17*16, 16*16,
			7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16 },
	64*8    /* every sprite takes 64 consecutive bytes */
};
static const gfx_layout spritelayout2 =
{
	8,8,    /* 8*8 sprites */
	512,    /* 512 sprites */
	2,  /* 2 bits per pixel */
	{ 1, 0 },   /* the two bitplanes are packed in two consecutive bits */
	{ 0, 2, 4, 6, 8, 10, 12, 14 },
	{ 7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16 },
	16*8    /* every sprite takes 16 consecutive bytes */
};

static const gfx_layout gridlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	1,  /* 1 bit per pixel */
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout gridlayout2 =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	1,  /* 1 bit per pixel */
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( sraider )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout2,             0,  8 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,          4*8, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout2,         4*8, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, gridlayout,    4*8+4*16+32,  1 )
	GFXDECODE_ENTRY( "gfx3", 0, gridlayout2,   4*8+4*16+32,  1 )
GFXDECODE_END

MACHINE_START_MEMBER(redclash_state,sraider)
{
	save_item(NAME(m_grid_color));
	save_item(NAME(m_sound_low));
	save_item(NAME(m_sound_high));
	save_item(NAME(m_sraider_0x30));
	save_item(NAME(m_sraider_0x38));
	save_item(NAME(m_weird_value));

	/* for stars */
	save_item(NAME(m_star_speed));
	save_item(NAME(m_stars_enable));
	save_item(NAME(m_stars_speed));
	save_item(NAME(m_stars_state));
	save_item(NAME(m_stars_offset));
	save_item(NAME(m_stars_count));
}

MACHINE_RESET_MEMBER(redclash_state,sraider)
{
	int i;

	m_grid_color = 0;
	m_sound_low = 0;
	m_sound_high = 0;
	m_sraider_0x30 = 0;
	m_sraider_0x38 = 0;

	/* for stars */
	m_star_speed = 0;
	m_stars_enable = 0;
	m_stars_speed = 0;
	m_stars_state = 0;
	m_stars_offset = 0;
	m_stars_count = 0;

	for (i = 0; i < 8; i++)
		m_weird_value[i] = 0;
}


MACHINE_START_MEMBER(redclash_state,redclash)
{
	save_item(NAME(m_star_speed));
	save_item(NAME(m_gfxbank));
	save_item(NAME(m_stars_enable));
	save_item(NAME(m_stars_speed));
	save_item(NAME(m_stars_state));
	save_item(NAME(m_stars_offset));
	save_item(NAME(m_stars_count));
}

MACHINE_RESET_MEMBER(redclash_state,redclash)
{
	m_star_speed = 0;
	m_gfxbank = 0;
	m_stars_enable = 0;
	m_stars_speed = 0;
	m_stars_state = 0;
	m_stars_offset = 0;
	m_stars_count = 0;
}

static MACHINE_CONFIG_START( zerohour, redclash_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)  /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(zerohour_map)

	MCFG_MACHINE_START_OVERRIDE(redclash_state,redclash)
	MCFG_MACHINE_RESET_OVERRIDE(redclash_state,redclash)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(redclash_state, screen_update_redclash)
	MCFG_SCREEN_VBLANK_DRIVER(redclash_state, screen_eof_redclash)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", redclash)
	MCFG_PALETTE_ADD("palette", 4*8+4*16+32)
	MCFG_PALETTE_INDIRECT_ENTRIES(32+32)
	MCFG_PALETTE_INIT_OWNER(redclash_state,redclash)
	MCFG_VIDEO_START_OVERRIDE(redclash_state,redclash)

	/* sound hardware */
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( redclash, redclash_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)  /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(redclash_map)

	MCFG_MACHINE_START_OVERRIDE(redclash_state,redclash)
	MCFG_MACHINE_RESET_OVERRIDE(redclash_state,redclash)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(redclash_state, screen_update_redclash)
	MCFG_SCREEN_VBLANK_DRIVER(redclash_state, screen_eof_redclash)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", redclash)
	MCFG_PALETTE_ADD("palette", 4*8+4*16+32)
	MCFG_PALETTE_INDIRECT_ENTRIES(32+32)
	MCFG_PALETTE_INIT_OWNER(redclash_state,redclash)
	MCFG_VIDEO_START_OVERRIDE(redclash_state,redclash)

	/* sound hardware */
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sraider, redclash_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(sraider_cpu1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", redclash_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, 4000000)   /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(sraider_cpu2_map)
	MCFG_CPU_IO_MAP(sraider_cpu2_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", redclash_state,  irq0_line_hold)

	MCFG_MACHINE_START_OVERRIDE(redclash_state,sraider)
	MCFG_MACHINE_RESET_OVERRIDE(redclash_state,sraider)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(redclash_state, screen_update_sraider)
	MCFG_SCREEN_VBLANK_DRIVER(redclash_state, screen_eof_sraider)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sraider)
	MCFG_PALETTE_ADD("palette", 4*8+4*16+32+2)
	MCFG_PALETTE_INDIRECT_ENTRIES(32+32+1)
	MCFG_PALETTE_INIT_OWNER(redclash_state,sraider)

	MCFG_VIDEO_START_OVERRIDE(redclash_state,sraider)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76489, 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn2", SN76489, 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn3", SN76489, 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn4", SN76489, 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn5", SN76489, 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( zerohour )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ze1_1.c8", 0x0000, 0x0800, CRC(0dff4b48) SHA1(4911255f953851d0e5c2b66090b95254ac59ac9e) )
	ROM_LOAD( "ze2_2.c7", 0x0800, 0x0800, CRC(cf41b6ac) SHA1(263794e6be22c20e2b10fe9099e475097475df7b) )
	ROM_LOAD( "ze3_3.c6", 0x1000, 0x0800, CRC(5ef48b67) SHA1(ae291aa84b109e6a51eebdd5526abca1d901b7b9) )
	ROM_LOAD( "ze4_4.c5", 0x1800, 0x0800, CRC(25c5872d) SHA1(df008db607b72a92c4284d6a8127eafec2432ca4) )
	ROM_LOAD( "ze5_5.c4", 0x2000, 0x0800, CRC(d7ce3add) SHA1(d8dd7ad98e7a0a4f35de181549b2e88a9e0a73d6) )
	ROM_LOAD( "ze6_6.c3", 0x2800, 0x0800, CRC(8a93ae6e) SHA1(a66f05bb27e67b755c64ac8b68fa38ffe4cd961c) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "z9", 0x0000, 0x0800, CRC(17ae6f13) SHA1(ce7a02f4e1aa2e5292d3807a0cfed6d92752fc7a) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "z7", 0x0000, 0x0800, CRC(4c12f59d) SHA1(b99a21415bff0e59b6130df60182f05b1a5d0811) )
	ROM_LOAD( "z8", 0x0800, 0x0800, CRC(6b9a6b6e) SHA1(f80d893b1b26c75c297e1da1c20db04e7129c92a) )

	ROM_REGION( 0x1000, "gfx3", ROMREGION_ERASE00 )
	/* gfx data will be rearranged here for 8x8 sprites */

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "z1.ic2", 0x0000, 0x0020, CRC(b55aee56) SHA1(33e4767c8afbb7b3af67517ea1dfd69bf692cac7) ) /* 82S123, palette */
	ROM_LOAD( "z2.n2",  0x0020, 0x0020, CRC(9adabf46) SHA1(f3538fdbc4280b6be46a4d7ebb4c34bd1a1ce2b7) ) /* MM6330, sprite color lookup table */
	ROM_LOAD( "z3.u6",  0x0040, 0x0020, CRC(27fa3a50) SHA1(7cf59b7a37c156640d6ea91554d1c4276c1780e0) ) /* MM6330, Unknown purpose */
ROM_END

ROM_START( zerohoura ) /* Earlier version? */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "z1_1.c8", 0x0000, 0x0800, CRC(77418b5d) SHA1(2ff7da1f0d9b311a32736c4784b1ba6acfc29512) )
	ROM_LOAD( "z2_2.c7", 0x0800, 0x0800, CRC(9b23b5ac) SHA1(2baca7a9ef65d5fe8216dce891a933d39bcdb1b1) )
	ROM_LOAD( "z3_3.c6", 0x1000, 0x0800, CRC(7aa25c95) SHA1(0bab20cabeb6ffe77d3122f2b489348b5efbdcb1) )
	ROM_LOAD( "z4_4.c5", 0x1800, 0x0800, CRC(b0a26dea) SHA1(69b00a5c4971fc161453efe6126bd36711420f99) )
	ROM_LOAD( "z5_5.c4", 0x2000, 0x0800, CRC(d7ce3add) SHA1(d8dd7ad98e7a0a4f35de181549b2e88a9e0a73d6) ) /* Same as above set */
	ROM_LOAD( "z6_6.c3", 0x2800, 0x0800, CRC(8a93ae6e) SHA1(a66f05bb27e67b755c64ac8b68fa38ffe4cd961c) ) /* Same as above set */

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "z9", 0x0000, 0x0800, CRC(17ae6f13) SHA1(ce7a02f4e1aa2e5292d3807a0cfed6d92752fc7a) ) /* Same as above set */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "z7", 0x0000, 0x0800, CRC(4c12f59d) SHA1(b99a21415bff0e59b6130df60182f05b1a5d0811) ) /* Same as above set */
	ROM_LOAD( "z8", 0x0800, 0x0800, CRC(6b9a6b6e) SHA1(f80d893b1b26c75c297e1da1c20db04e7129c92a) ) /* Same as above set */

	ROM_REGION( 0x1000, "gfx3", ROMREGION_ERASE00 )
	/* gfx data will be rearranged here for 8x8 sprites */

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "z1.ic2", 0x0000, 0x0020, CRC(b55aee56) SHA1(33e4767c8afbb7b3af67517ea1dfd69bf692cac7) ) /* 82S123, palette */
	ROM_LOAD( "z2.n2",  0x0020, 0x0020, CRC(9adabf46) SHA1(f3538fdbc4280b6be46a4d7ebb4c34bd1a1ce2b7) ) /* MM6330, sprite color lookup table */
	ROM_LOAD( "z3.u6",  0x0040, 0x0020, CRC(27fa3a50) SHA1(7cf59b7a37c156640d6ea91554d1c4276c1780e0) ) /* MM6330, Unknown purpose */
ROM_END

// k7, k5 and c4 did not give consistent reads, k7/k5 are graphics and clearly meant to be the same
//
// c4 was more damaged (lots of bytes replaced with 0x00) possibly UV exposed.
// I'm not sure where this rom is used by the game (wp 2000,800,r and bp 2000,800 catch nothing) for now I'm assuming it was meant to match.
// the game seems to be based off the 'zerohour' set but like many Inder sets it has some code inserted at the end of the last rom that is called by the game (bp 2fe0)
// Inder boards with the regular Universal code have also been seen.
ROM_START( zerohouri )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "zhi.c8", 0x0000, 0x0800, CRC(0dff4b48) SHA1(4911255f953851d0e5c2b66090b95254ac59ac9e) )
	ROM_LOAD( "zhi.c7", 0x0800, 0x0800, CRC(dcfefb4c) SHA1(9d8f57f5f09368225bae06a64971b82a848b1a7e) )
	ROM_LOAD( "zhi.c6", 0x1000, 0x0800, CRC(ddc66d36) SHA1(1cbf6e27e7b2ccd39199c1a26783d53d6d90d195) )
	ROM_LOAD( "zhi.c5", 0x1800, 0x0800, CRC(25c5872d) SHA1(df008db607b72a92c4284d6a8127eafec2432ca4) ) // == ze4_4.c5
//  ROM_LOAD( "zhi.c4", 0x2000, 0x0800, CRC(9b70464b) SHA1(ccd173e12630ba044fe659915dfce21f2b5e0e39) ) // corrupt
	ROM_LOAD( "zhi.c4", 0x2000, 0x0800, CRC(d7ce3add) SHA1(d8dd7ad98e7a0a4f35de181549b2e88a9e0a73d6) ) // use rom from above sets instead
	ROM_LOAD( "zhi.c3", 0x2800, 0x0800, CRC(29dee5e4) SHA1(13c1778d427a11f5c24ce8116fe55d60e98e3d83) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "zhi.k7", 0x0000, 0x0800, CRC(17ae6f13) SHA1(ce7a02f4e1aa2e5292d3807a0cfed6d92752fc7a) ) /* Same as above set */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "zhi.k4", 0x0000, 0x0800, CRC(4c12f59d) SHA1(b99a21415bff0e59b6130df60182f05b1a5d0811) ) /* Same as above set */
	ROM_LOAD( "zhi.k5", 0x0800, 0x0800, CRC(6b9a6b6e) SHA1(f80d893b1b26c75c297e1da1c20db04e7129c92a) ) /* Same as above set */

	ROM_REGION( 0x1000, "gfx3", ROMREGION_ERASE00 )
	/* gfx data will be rearranged here for 8x8 sprites */

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "z1.ic2", 0x0000, 0x0020, CRC(b55aee56) SHA1(33e4767c8afbb7b3af67517ea1dfd69bf692cac7) ) /* 82S123, palette */
	ROM_LOAD( "z2.n2",  0x0020, 0x0020, CRC(9adabf46) SHA1(f3538fdbc4280b6be46a4d7ebb4c34bd1a1ce2b7) ) /* MM6330, sprite color lookup table */
	ROM_LOAD( "z3.u6",  0x0040, 0x0020, CRC(27fa3a50) SHA1(7cf59b7a37c156640d6ea91554d1c4276c1780e0) ) /* MM6330, Unknown purpose */
ROM_END


ROM_START( redclash )
	ROM_REGION(0x10000, "maincpu", 0 )
	ROM_LOAD( "11.11c",       0x0000, 0x1000, CRC(695e070e) SHA1(8d0451a05572f62e0f282ab96bdd26d08b77a6c9) )
	ROM_LOAD( "13.7c",        0x1000, 0x1000, CRC(c2090318) SHA1(71725cdf51aedf5f29fa1dd1a41ad5e62c9a580d) )
	ROM_LOAD( "12.9c",        0x2000, 0x1000, CRC(b60e5ada) SHA1(37440f382c5e8852d804fa9837c36cc1e9d94d1d) )

	ROM_REGION(0x0800, "gfx1", 0 )
	ROM_LOAD( "6.12a",        0x0000, 0x0800, CRC(da9bbcc2) SHA1(4cbe03c7f5e99cc2f124e0089ea3c392156b5d92) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "14.3e",        0x0000, 0x0800, CRC(483a1293) SHA1(e7812475c7509389bcf8fee35598e9894428eb37) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "15.3d",        0x0800, 0x0800, CRC(c45d9601) SHA1(2f156ad61161d65284df0cc55eb1b3b990eb41cb) )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x2000, "gfx3", ROMREGION_ERASE00 )
	/* gfx data will be rearranged here for 8x8 sprites */

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "1.12f",        0x0000, 0x0020, CRC(43989681) SHA1(0d471e6f499294f2f62f27392b8370e2af8e38a3) ) /* palette */
	ROM_LOAD( "2.4a",         0x0020, 0x0020, CRC(9adabf46) SHA1(f3538fdbc4280b6be46a4d7ebb4c34bd1a1ce2b7) ) /* sprite color lookup table */
	ROM_LOAD( "3.11e",        0x0040, 0x0020, CRC(27fa3a50) SHA1(7cf59b7a37c156640d6ea91554d1c4276c1780e0) ) /* ?? */
ROM_END

ROM_START( redclasha )
	ROM_REGION(0x10000, "maincpu", 0 )
	ROM_LOAD( "rc1.11c",      0x0000, 0x1000, CRC(5b62ff5a) SHA1(981d3c72f28b7d136a0bad9243d39fd1ba3abc97) )
	ROM_LOAD( "rc3.7c",       0x1000, 0x1000, CRC(409c4ee7) SHA1(15c03a4093d7695751a143aa749229fcb7721f46) )
	ROM_LOAD( "rc2.9c",       0x2000, 0x1000, CRC(5f215c9a) SHA1(c305f7be19f6a052c08feb0b63a0326b6a1bd808) )

	ROM_REGION(0x0800, "gfx1", 0 )
	ROM_LOAD( "rc6.12a",      0x0000, 0x0800, CRC(da9bbcc2) SHA1(4cbe03c7f5e99cc2f124e0089ea3c392156b5d92) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "rc4.3e",       0x0000, 0x0800, CRC(64ca8b63) SHA1(5fd1ca9b81f66b4d2041674900718dc8c94c2a97) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "rc5.3d",       0x0800, 0x0800, CRC(fce610a2) SHA1(0be829c6f6f5c3a19056ba1594141c1965c7aa2a) )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x2000, "gfx3", ROMREGION_ERASE00 )
	/* gfx data will be rearranged here for 8x8 sprites */

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "1.12f",        0x0000, 0x0020, CRC(43989681) SHA1(0d471e6f499294f2f62f27392b8370e2af8e38a3) ) /* palette */
	ROM_LOAD( "2.4a",         0x0020, 0x0020, CRC(9adabf46) SHA1(f3538fdbc4280b6be46a4d7ebb4c34bd1a1ce2b7) ) /* sprite color lookup table */
	ROM_LOAD( "3.11e",        0x0040, 0x0020, CRC(27fa3a50) SHA1(7cf59b7a37c156640d6ea91554d1c4276c1780e0) ) /* ?? */
ROM_END

ROM_START( redclashk )
	ROM_REGION(0x10000, "maincpu", 0 )
	ROM_LOAD( "rc1.8c",       0x0000, 0x0800, CRC(fd90622a) SHA1(a65a32d519e7fee89b160f8152322df20b6af4ea) )
	ROM_LOAD( "rc2.7c",       0x0800, 0x0800, CRC(c8f33440) SHA1(60d1faee415faa13102b8e744f444f1480b8bd73) )
	ROM_LOAD( "rc3.6c",       0x1000, 0x0800, CRC(2172b1e9) SHA1(b6f7ee8924bda9f8da13baaa2db3ffb7d623236c) )
	ROM_LOAD( "rc4.5c",       0x1800, 0x0800, CRC(55c0d1b5) SHA1(2f1d729d29184de8f8bb914015352730325cda12) )
	ROM_LOAD( "rc5.4c",       0x2000, 0x0800, CRC(744b5261) SHA1(6c5de2f91f57c463230e0ea04b336347840161a3) )
	ROM_LOAD( "rc6.3c",       0x2800, 0x0800, CRC(fa507e17) SHA1(dd0e27b08e902b91c5e9552351206c671ed2f3c0) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "rc6.12a",      0x0000, 0x0800, CRC(da9bbcc2) SHA1(4cbe03c7f5e99cc2f124e0089ea3c392156b5d92) ) /* rc9.7m */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "rc4.4m",       0x0000, 0x0800, CRC(483a1293) SHA1(e7812475c7509389bcf8fee35598e9894428eb37) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "rc5.5m",       0x0800, 0x0800, CRC(c45d9601) SHA1(2f156ad61161d65284df0cc55eb1b3b990eb41cb) )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x2000, "gfx3", ROMREGION_ERASE00 )
	/* gfx data will be rearranged here for 8x8 sprites */

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "1.12f",        0x0000, 0x0020, CRC(43989681) SHA1(0d471e6f499294f2f62f27392b8370e2af8e38a3) ) /* 6331.7e */
	ROM_LOAD( "2.4a",         0x0020, 0x0020, CRC(9adabf46) SHA1(f3538fdbc4280b6be46a4d7ebb4c34bd1a1ce2b7) ) /* 6331.2r */
	ROM_LOAD( "3.11e",        0x0040, 0x0020, CRC(27fa3a50) SHA1(7cf59b7a37c156640d6ea91554d1c4276c1780e0) ) /* 6331.6w */
ROM_END

ROM_START( sraider )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sraid3.r4",    0x0000, 0x2000, CRC(0f389774) SHA1(c67596e6bf00175ff0a241506cd2f88114d05933) )
	ROM_LOAD( "sraid2.n4",    0x2000, 0x2000, CRC(38a48db0) SHA1(6f4f384d702fb8ee4bb2ef579638239d57e32ddd) )
	ROM_LOAD( "sraid1.m4",    0x4000, 0x2000, CRC(2f302a4e) SHA1(3a902ce6858f38df88b60830bef4b1d45b09b2df) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "sraid-s4.h6",  0x0000, 0x2000, CRC(57173a12) SHA1(6cb8fd4826e499f9a4e63621d58bc4b596cc261e) )
	ROM_LOAD( "sraid-s5.j6",  0x2000, 0x2000, CRC(5a459179) SHA1(a261c8f3c7c4cd4587c003bbbe815d2c4e01ffbc) )
	ROM_LOAD( "sraid-s6.l6",  0x4000, 0x2000, CRC(ea3aa25d) SHA1(353c0d075d5e0a3bc25a65e2748f5eb5212a844d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sraid-s0.k6",  0x0000, 0x1000, CRC(a0373909) SHA1(00e3bd5dd90769d670fc3c51edd1cd4b69e6132d) )
	ROM_LOAD( "sraids11.l6",  0x1000, 0x1000, CRC(ba22d949) SHA1(83762ced1df92ff594887e44d5b783826bbfb0c9) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sraid-s7.m2",  0x0000, 0x1000, CRC(299f8e07) SHA1(1de71f251286088487da7285d6f8070147002af5) )
	ROM_LOAD( "sraid-s8.n2",  0x1000, 0x1000, CRC(57ba8888) SHA1(2aa1a5f682d146a55a96e471bb78e5c60da02bf9) )

	ROM_REGION( 0x1000, "gfx3", 0 ) /* fixed portion of the grid */
	ROM_LOAD( "sraid-s9.f6",  0x0000, 0x1000, CRC(2380b90f) SHA1(0310554e3f2ec973c2bb6e816d04e5c0c1e0a0b9) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "srpr10-1.a2",  0x0000, 0x0020, CRC(121fdb99) SHA1(3bc092da40beb129a4df3db2f55d22bbbcf7bad8) )
	ROM_LOAD( "srpr10-2.l3",  0x0020, 0x0020, CRC(88b67e70) SHA1(e21ee2939e96dffee101bd92c62ed975b6b64001) )
	ROM_LOAD( "srpr10-3.c1",  0x0040, 0x0020, CRC(27fa3a50) SHA1(7cf59b7a37c156640d6ea91554d1c4276c1780e0) ) /* ?? */
ROM_END


DRIVER_INIT_MEMBER(redclash_state,redclash)
{
	int i,j;
	const UINT8 *src = memregion("gfx2")->base();
	UINT8 *dst = memregion("gfx3")->base();
	int len = memregion("gfx3")->bytes();

	/* rearrange the sprite graphics */
	for (i = 0;i < len;i++)
	{
		j = (i & ~0x003e) | ((i & 0x0e) << 2) | ((i & 0x30) >> 3);
		dst[i] = src[j];
	}
}


GAME( 1980, zerohour,  0,        zerohour, zerohour, redclash_state, redclash, ROT270, "Universal", "Zero Hour (set 1)",  MACHINE_NO_SOUND | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1980, zerohoura, zerohour, zerohour, zerohour, redclash_state, redclash, ROT270, "Universal", "Zero Hour (set 2)",  MACHINE_NO_SOUND | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1980, zerohouri, zerohour, zerohour, zerohour, redclash_state, redclash, ROT270, "bootleg (Inder SA)", "Zero Hour (Inder)",  MACHINE_NO_SOUND | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, redclash,  0,        redclash, redclash, redclash_state, redclash, ROT270, "Tehkan",    "Red Clash (set 1)",  MACHINE_NO_SOUND | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, redclasha, redclash, redclash, redclash, redclash_state, redclash, ROT270, "Tehkan",    "Red Clash (set 2)",  MACHINE_NO_SOUND | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, redclashk, redclash, redclash, redclash, redclash_state, redclash, ROT270, "Tehkan (Kaneko license)", "Red Clash (Kaneko)", MACHINE_NO_SOUND | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1982, sraider,  0,       sraider, sraider, driver_device,  0,       ROT270, "Universal", "Space Raider", MACHINE_SUPPORTS_SAVE )
