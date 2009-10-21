/****************************************

Libble Rabble (c) 1983 Namco
Toypop        (c) 1986 Namco

driver by Edgardo E. Contini Salvan (pag2806@iperbole.bologna.it)

6809 main CPU,
6809 sound and
68000 to create the background image.

Libble Rabble and Toypop run on the same board, but the memory map is subtly
different.

Notes:
------
- Libble Rabble Easter egg:
  - enter service mode
  - turn off the service mode switch, and turn it on again quickly to remain
    on the monitor test grid
  - Enter the following sequence using the right joystick:
    9xU 2xR 9xD 2xL
  (c) 1983 NAMCO LTD. will appear on the screen.


TODO:
- I haven't found any Easter egg in Toy Pop. Maybe they had stopped inserting
  them by that time.

****************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "machine/namcoio.h"
#include "sound/namco.h"


// video\toypop.c
extern UINT16 *toypop_bg_image;
extern UINT8 *toypop_videoram;
WRITE8_HANDLER( toypop_videoram_w );
READ16_HANDLER( toypop_merged_background_r );
WRITE16_HANDLER( toypop_merged_background_w );
WRITE8_HANDLER( toypop_palettebank_w );
WRITE16_HANDLER( toypop_flipscreen_w );
VIDEO_START( toypop );
VIDEO_UPDATE( toypop );
PALETTE_INIT( toypop );



/***************************************************************************

  Custom I/O initialization

***************************************************************************/

static READ8_HANDLER( in0_l )	{ return input_port_read(space->machine, "INPUT_RIGHT"); }			// P1 joystick
static READ8_HANDLER( in0_h )	{ return input_port_read(space->machine, "INPUT_RIGHT") >> 4; }	// P2 joystick
static READ8_HANDLER( in1_l )	{ return input_port_read(space->machine, "SYSTEM"); }				// fire and start buttons
static READ8_HANDLER( in1_h )	{ return input_port_read(space->machine, "SYSTEM") >> 4; }			// coins
static READ8_HANDLER( dipA_l )	{ return input_port_read(space->machine, "DSW1"); }				// dips A
static READ8_HANDLER( dipA_h )	{ return input_port_read(space->machine, "DSW1") >> 4; }			// dips A
static READ8_HANDLER( dipB_l )	{ return input_port_read(space->machine, "DSW2"); }				// dips B
static READ8_HANDLER( dipB_h )	{ return input_port_read(space->machine, "DSW2") >> 4; }			// dips B
static READ8_HANDLER( in2_l )	{ return input_port_read(space->machine, "INPUT_LEFT"); }			// P1 joystick left in liblrabl
static READ8_HANDLER( in2_h )	{ return input_port_read(space->machine, "INPUT_LEFT") >> 4; }		// P2 joystick left in liblrabl
static READ8_HANDLER( in3 )		{ return input_port_read(space->machine, "SERVICE"); }				// test, cocktail, optional buttons
static WRITE8_HANDLER( out_coin0 )
{
	coin_lockout_global_w(data & 4);
	coin_counter_w(0,~data & 8);
}
static WRITE8_HANDLER( out_coin1 )
{
	coin_counter_w(1,~data & 1);
}
static WRITE8_HANDLER( flip )
{
	flip_screen_set(space->machine, data & 1);
}

/* chip #0: player inputs, buttons, coins */
static const struct namcoio_interface intf0_coin =
{
	{ in1_h, in0_l, in0_h, in1_l },	/* port read handlers */
	{ out_coin0, out_coin1 }		/* port write handlers */
};
static const struct namcoio_interface intf0 =
{
	{ in1_h, in0_l, in0_h, in1_l },	/* port read handlers */
	{ NULL, NULL }					/* port write handlers */
};

/* chip #1: dip switches */
static const struct namcoio_interface intf1 =
{
	{ dipA_h, dipB_l, dipB_h, dipA_l },	/* port read handlers */
	{ flip, NULL }						/* port write handlers */
};

/* chip #2: test/cocktail, optional buttons */
static const struct namcoio_interface intf2 =
{
	{ NULL, in2_l, in2_h, in3 },	/* port read handlers */
	{ NULL, NULL }					/* port write handlers */
};

static DRIVER_INIT( 58c_56_56 )
{
	namcoio_init(machine, 0, NAMCOIO_58XX, &intf0_coin, NULL);
	namcoio_init(machine, 1, NAMCOIO_56XX, &intf1, NULL);
	namcoio_init(machine, 2, NAMCOIO_56XX, &intf2, NULL);
}

static DRIVER_INIT( 58_56_56 )
{
	namcoio_init(machine, 0, NAMCOIO_58XX, &intf0, NULL);
	namcoio_init(machine, 1, NAMCOIO_56XX, &intf1, NULL);
	namcoio_init(machine, 2, NAMCOIO_56XX, &intf2, NULL);
}


/***************************************************************************/


static int interrupt_enable_68k;
static UINT8 *toypop_m68000_sharedram;


static READ8_DEVICE_HANDLER( toypop_sound_sharedram_r )
{
	return namco_soundregs[offset];
}

static WRITE8_DEVICE_HANDLER( toypop_sound_sharedram_w )
{
	if (offset < 0x40)
		namco_15xx_w(device,offset,data);
	else
		namco_soundregs[offset] = data;
}

static READ16_HANDLER( toypop_m68000_sharedram_r )
{
	return toypop_m68000_sharedram[offset];
}

static WRITE16_HANDLER( toypop_m68000_sharedram_w )
{
	if (ACCESSING_BITS_0_7)
		toypop_m68000_sharedram[offset] = data & 0xff;
}

static READ8_HANDLER( toypop_main_interrupt_enable_r )
{
	cpu_interrupt_enable(cputag_get_cpu(space->machine, "maincpu"), 1);
	return 0;
}

static WRITE8_HANDLER( toypop_main_interrupt_enable_w )
{
	cpu_interrupt_enable(cputag_get_cpu(space->machine, "maincpu"), 1);
	cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
}

static WRITE8_HANDLER( toypop_main_interrupt_disable_w )
{
	cpu_interrupt_enable(cputag_get_cpu(space->machine, "maincpu"), 0);
}

static WRITE8_HANDLER( toypop_sound_interrupt_enable_acknowledge_w )
{
	cpu_interrupt_enable(cputag_get_cpu(space->machine, "audiocpu"), 1);
	cputag_set_input_line(space->machine, "audiocpu", 0, CLEAR_LINE);
}

static WRITE8_HANDLER( toypop_sound_interrupt_disable_w )
{
	cpu_interrupt_enable(cputag_get_cpu(space->machine, "audiocpu"), 0);
}

static INTERRUPT_GEN( toypop_main_interrupt )
{
	irq0_line_assert(device);	// this also checks if irq is enabled - IMPORTANT!
								// so don't replace with cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);

	namcoio_set_irq_line(device->machine, 0, PULSE_LINE);
	namcoio_set_irq_line(device->machine, 1, PULSE_LINE);
	namcoio_set_irq_line(device->machine, 2, PULSE_LINE);
}

static WRITE8_HANDLER( toypop_sound_clear_w )
{
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, CLEAR_LINE);
}

static WRITE8_HANDLER( toypop_sound_assert_w )
{
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);
}

static WRITE8_HANDLER( toypop_m68000_clear_w )
{
	cputag_set_input_line(space->machine, "sub", INPUT_LINE_RESET, CLEAR_LINE);
}

static WRITE8_HANDLER( toypop_m68000_assert_w )
{
	cputag_set_input_line(space->machine, "sub", INPUT_LINE_RESET, ASSERT_LINE);
}

static TIMER_CALLBACK( disable_interrupts )
{
	cpu_interrupt_enable(cputag_get_cpu(machine, "maincpu"), 0);
	cputag_set_input_line(machine, "maincpu", 0, CLEAR_LINE);
	cpu_interrupt_enable(cputag_get_cpu(machine, "audiocpu"), 0);
	cputag_set_input_line(machine, "audiocpu", 0, CLEAR_LINE);
	interrupt_enable_68k = 0;
}

static MACHINE_RESET( toypop )
{
	/* we must do this on a timer in order to have it take effect */
	/* otherwise, the reset process will override our changes */
	timer_call_after_resynch(machine, NULL, 0, disable_interrupts);
}

static INTERRUPT_GEN( toypop_m68000_interrupt )
{
	if (interrupt_enable_68k)
		cpu_set_input_line(device, 6, HOLD_LINE);
}

static WRITE16_HANDLER( toypop_m68000_interrupt_enable_w )
{
	interrupt_enable_68k = 1;
}

static WRITE16_HANDLER( toypop_m68000_interrupt_disable_w )
{
	interrupt_enable_68k = 0;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( liblrabl_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM_WRITE(toypop_videoram_w) AM_BASE(&toypop_videoram)	/* video RAM */
	AM_RANGE(0x0800, 0x0f7f) AM_RAM											/* general RAM, area 1 */
	AM_RANGE(0x0f80, 0x0fff) AM_RAM AM_BASE(&spriteram)						/* sprite RAM, area 1 */
	AM_RANGE(0x1000, 0x177f) AM_RAM											/* general RAM, area 2 */
	AM_RANGE(0x1780, 0x17ff) AM_RAM AM_BASE(&spriteram_2)					/* sprite RAM, area 2 */
	AM_RANGE(0x1800, 0x1f7f) AM_RAM											/* general RAM, area 3 */
	AM_RANGE(0x1f80, 0x1fff) AM_RAM AM_BASE(&spriteram_3)					/* sprite RAM, area 3 */
	AM_RANGE(0x2800, 0x2fff) AM_RAM AM_BASE(&toypop_m68000_sharedram)		/* shared RAM with the 68000 CPU */
	AM_RANGE(0x6000, 0x63ff) AM_DEVREADWRITE("namco", toypop_sound_sharedram_r, toypop_sound_sharedram_w) /* shared RAM with sound CPU */
	AM_RANGE(0x6800, 0x683f) AM_READWRITE(namcoio_r, namcoio_w)				/* custom I/O */
	AM_RANGE(0x7000, 0x7000) AM_WRITE(toypop_main_interrupt_enable_w)		/* enable interrupt */
	AM_RANGE(0x7800, 0x7800) AM_READWRITE(watchdog_reset_r, toypop_main_interrupt_disable_w) /* disable interrupt */
	AM_RANGE(0x8000, 0x8000) AM_WRITE(toypop_m68000_clear_w)				/* reset 68000 */
	AM_RANGE(0x8800, 0x8800) AM_WRITE(toypop_m68000_assert_w)				/* reset 68000 */
	AM_RANGE(0x9000, 0x9000) AM_WRITE(toypop_sound_clear_w)					/* sound CPU reset */
	AM_RANGE(0x9800, 0x9800) AM_WRITE(toypop_sound_assert_w)				/* sound CPU reset */
	AM_RANGE(0xa000, 0xa001) AM_WRITE(toypop_palettebank_w)					/* background image palette */
	AM_RANGE(0x8000, 0xffff) AM_ROM											/* ROM code */
ADDRESS_MAP_END

static ADDRESS_MAP_START( toypop_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM_WRITE(toypop_videoram_w) AM_BASE(&toypop_videoram)	/* video RAM */
	AM_RANGE(0x0800, 0x0f7f) AM_RAM											/* general RAM, area 1 */
	AM_RANGE(0x0f80, 0x0fff) AM_RAM AM_BASE(&spriteram)						/* sprite RAM, area 1 */
	AM_RANGE(0x1000, 0x177f) AM_RAM											/* general RAM, area 2 */
	AM_RANGE(0x1780, 0x17ff) AM_RAM AM_BASE(&spriteram_2)					/* sprite RAM, area 2 */
	AM_RANGE(0x1800, 0x1f7f) AM_RAM											/* general RAM, area 3 */
	AM_RANGE(0x1f80, 0x1fff) AM_RAM AM_BASE(&spriteram_3)					/* sprite RAM, area 3 */
	AM_RANGE(0x2800, 0x2fff) AM_RAM AM_BASE(&toypop_m68000_sharedram)		/* shared RAM with the 68000 CPU */
	AM_RANGE(0x6000, 0x603f) AM_READWRITE(namcoio_r, namcoio_w)				/* custom I/O */
	AM_RANGE(0x6800, 0x6bff) AM_DEVREADWRITE("namco", toypop_sound_sharedram_r, toypop_sound_sharedram_w) /* shared RAM with sound CPU */
	AM_RANGE(0x7000, 0x7000) AM_READWRITE(toypop_main_interrupt_enable_r, toypop_main_interrupt_disable_w) /* disable interrupt */
	AM_RANGE(0x8000, 0x8000) AM_WRITE(toypop_m68000_clear_w)				/* reset 68000 */
	AM_RANGE(0x8800, 0x8800) AM_WRITE(toypop_m68000_assert_w)				/* reset 68000 */
	AM_RANGE(0x9000, 0x9000) AM_WRITE(toypop_sound_clear_w)					/* sound CPU reset */
	AM_RANGE(0x9800, 0x9800) AM_WRITE(toypop_sound_assert_w)				/* sound CPU reset */
	AM_RANGE(0xa000, 0xa001) AM_WRITE(toypop_palettebank_w)					/* background image palette */
	AM_RANGE(0x8000, 0xffff) AM_ROM											/* ROM code */
ADDRESS_MAP_END


/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_DEVREADWRITE("namco", toypop_sound_sharedram_r, toypop_sound_sharedram_w) AM_BASE(&namco_soundregs)	/* shared RAM with the main CPU + sound registers */
	AM_RANGE(0x2000, 0x2000) AM_WRITE(toypop_sound_interrupt_disable_w)	/* ??? toypop doesn't write here */
	AM_RANGE(0x4000, 0x4000) AM_WRITE(toypop_sound_interrupt_enable_acknowledge_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  68k CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( m68k_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x007fff) AM_ROM										/* ROM code */
	AM_RANGE(0x080000, 0x0bffff) AM_RAM										/* RAM */
	AM_RANGE(0x100000, 0x100fff) AM_READWRITE(toypop_m68000_sharedram_r, toypop_m68000_sharedram_w)	/* shared RAM with the main CPU */
	AM_RANGE(0x180000, 0x187fff) AM_READWRITE(toypop_merged_background_r, toypop_merged_background_w) /* RAM that has to be merged with the background image */
	AM_RANGE(0x18fffc, 0x18ffff) AM_WRITE(toypop_flipscreen_w)				/* flip mode */
	AM_RANGE(0x190000, 0x1dffff) AM_RAM AM_BASE(&toypop_bg_image)			/* RAM containing the background image */
	AM_RANGE(0x300000, 0x300001) AM_WRITE(toypop_m68000_interrupt_enable_w)	/* interrupt enable */
	AM_RANGE(0x380000, 0x380001) AM_WRITE(toypop_m68000_interrupt_disable_w)/* interrupt disable */
ADDRESS_MAP_END



static INPUT_PORTS_START( liblrabl )
	/* The inputs are not memory mapped, they are handled by three I/O chips. */
	PORT_START("INPUT_RIGHT")	/* 58XX #0 pins 22-29 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("SYSTEM")	/* 58XX #0 pins 30-33 and 38-41 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW1")	/* 56XX #1 pins 22-29 */
	/* default setting: all OFF */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:8,7")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWA:6,5,4")
	// bonus scores for common
	PORT_DIPSETTING(    0x1c, "40k 120k 200k 400k 600k 1m" )
	PORT_DIPSETTING(    0x0c, "40k 140k 250k 400k 700k 1m" )
	// bonus scores for 1, 2 or 3 lives
	PORT_DIPSETTING(    0x14, "50k 150k 300k 500k 700k 1m" ) PORT_CONDITION("DSW1", 0x03, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x04, "40k 120k and every 120k" )    PORT_CONDITION("DSW1", 0x03, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x18, "40k 150k and every 150k" )    PORT_CONDITION("DSW1", 0x03, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x08, "50k 150k 300k" )              PORT_CONDITION("DSW1", 0x03, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x10, "40k 120k 200k" )              PORT_CONDITION("DSW1", 0x03, PORTCOND_NOTEQUALS, 0x01)
	// bonus scores for 5 lives
	PORT_DIPSETTING(    0x14, "40k 120k" )                   PORT_CONDITION("DSW1", 0x03, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x04, "50k 150k" )                   PORT_CONDITION("DSW1", 0x03, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x18, "50k 150k and every 150k" )    PORT_CONDITION("DSW1", 0x03, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x08, "60k 200k and every 200k" )    PORT_CONDITION("DSW1", 0x03, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x10, "50k" )                        PORT_CONDITION("DSW1", 0x03, PORTCOND_EQUALS, 0x01)
	// bonus scores for common
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:3,2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")	/* 56XX #1 pins 30-33 and 38-41 */
	PORT_DIPNAME( 0x01, 0x01, "Freeze" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Rack Test" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x20, 0x20, "Practice" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:2,1")
	PORT_DIPSETTING(    0xc0, "A" )
	PORT_DIPSETTING(    0x40, "B" )
	PORT_DIPSETTING(    0x80, "C" )
	PORT_DIPSETTING(    0x00, "D" )

	PORT_START("INPUT_LEFT")	/* 56XX #2 pins 22-29 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("SERVICE")	/* 56XX #2 pins 30-33 */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( toypop )
	/* The inputs are not memory mapped, they are handled by three I/O chips. */
	PORT_START("INPUT_RIGHT")	/* 58XX #0 pins 22-29 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("SYSTEM")	/* 58XX #0 pins 30-33 and 38-41 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW1")	/* 56XX #1 pins 22-29 */
	/* default setting: all OFF */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:8,7")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, 0x80, "SWA:1" )

	PORT_START("DSW2")	/* 56XX #1 pins 30-33 and 38-41 */
	PORT_DIPNAME( 0x01, 0x01, "Freeze" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "2 Players Game" ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x04, "2 Credits" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Entering" ) PORT_DIPLOCATION("SWB:4")	// buy in
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Every 15000 points" )
	PORT_DIPSETTING(    0x00, "Every 20000 points" )

	PORT_START("INPUT_LEFT")	/* 56XX #2 pins 22-29 */
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")	/* 56XX #2 pins 30-33 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )	// would be Cabinet, but this game has no cocktail mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )	// service mode again
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
	24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	32 * 8, 33 * 8, 34 * 8, 35 * 8, 36 * 8, 37 * 8, 38 * 8, 39 * 8 },
	64*8
};

static GFXDECODE_START( toypop )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 128*4,  64 )
GFXDECODE_END



static const namco_interface namco_config =
{
	8,		/* number of voices */
	0		/* stereo */
};



static MACHINE_DRIVER_START( liblrabl )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, 1536000)	/* 1.536 MHz (measured on Libble Rabble board) */
	MDRV_CPU_PROGRAM_MAP(liblrabl_map)
	MDRV_CPU_VBLANK_INT("screen", toypop_main_interrupt)

	MDRV_CPU_ADD("audiocpu", M6809, 1536000)
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_assert)

	MDRV_CPU_ADD("sub", M68000, 6144000)	/* 6.144 MHz (measured on Libble Rabble board) */
	MDRV_CPU_PROGRAM_MAP(m68k_map)
	MDRV_CPU_VBLANK_INT("screen", toypop_m68000_interrupt)

	MDRV_QUANTUM_TIME(HZ(6000))    /* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MDRV_MACHINE_RESET(toypop)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60.606060)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(36*8, 28*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 0*8, 28*8-1)

	MDRV_GFXDECODE(toypop)
	MDRV_PALETTE_LENGTH(128*4+64*4+16*2)

	MDRV_PALETTE_INIT(toypop)
	MDRV_VIDEO_START(toypop)
	MDRV_VIDEO_UPDATE(toypop)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("namco", NAMCO_15XX, 24000)
	MDRV_SOUND_CONFIG(namco_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( toypop )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(liblrabl)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(toypop_map)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( liblrabl )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* 64k for the first CPU */
	ROM_LOAD( "5b.rom",   0x8000, 0x4000, CRC(da7a93c2) SHA1(fe4a02cdab66722eb7b8cf58825f899b1949a6a2) )
	ROM_LOAD( "5c.rom",   0xc000, 0x4000, CRC(6cae25dc) SHA1(de74317a7d5de1865d096c377923a764be5e6879) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "2c.rom",   0xe000, 0x2000, CRC(7c09e50a) SHA1(5f004d60bbb7355e008a9cda137b28bc2192b8ef) )

	ROM_REGION( 0x8000, "sub", 0 )		/* 32k for the third CPU */
	ROM_LOAD16_BYTE( "8c.rom",   0x0000, 0x4000, CRC(a00cd959) SHA1(cc5621103c31cfbc65941615cab391db0f74e6ce) )
	ROM_LOAD16_BYTE("10c.rom",   0x0001, 0x4000, CRC(09ce209b) SHA1(2ed46d6592f8227bac8ab54963d9a300706ade47) )

	/* temporary space for graphics (disposed after conversion) */
	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5p.rom",   0x0000, 0x2000, CRC(3b4937f0) SHA1(06d9de576f1c2262c34aeb91054e68c9298af688) )	/* characters */

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "9t.rom",   0x0000, 0x4000, CRC(a88e24ca) SHA1(eada133579f19de09255084dcdc386311606a335) )	/* sprites */

	ROM_REGION( 0x0600, "proms", 0 )	/* color proms */
	ROM_LOAD( "lr1-3.1r", 0x0000, 0x0100, CRC(f3ec0d07) SHA1(b0aad1fb6df79f202889600f486853995352f9c2) )	// palette: red component
	ROM_LOAD( "lr1-2.1s", 0x0100, 0x0100, CRC(2ae4f702) SHA1(838fdca9e91fea4f64a59880ac47c48973bb8fbf) )	// palette: green component
	ROM_LOAD( "lr1-1.1t", 0x0200, 0x0100, CRC(7601f208) SHA1(572d070ca387b780030ed5de38a8970b7cc14349) )	// palette: blue component
	ROM_LOAD( "lr1-5.5l", 0x0300, 0x0100, CRC(940f5397) SHA1(825a7bd78a8a08d30bad2e4890ae6e9ad88b36b8) )	/* characters */
	ROM_LOAD( "lr1-6.2p", 0x0400, 0x0200, CRC(a6b7f850) SHA1(7cfde16dfd5c4d5b876b4fbe4f924f1385932a93) )	/* sprites */

	ROM_REGION( 0x0100, "namco", 0 )	/* sound prom */
	ROM_LOAD( "lr1-4.3d", 0x0000, 0x0100, CRC(16a9166a) SHA1(847cbaf7c88616576c410177e066ae1d792ac0ba) )
ROM_END

ROM_START( toypop )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* 64k for the first CPU */
	ROM_LOAD( "tp1-2.5b", 0x8000, 0x4000, CRC(87469620) SHA1(2ee257486c9c044386ac7d0cd4a90583eaeb3e97) )
	ROM_LOAD( "tp1-1.5c", 0xc000, 0x4000, CRC(dee2fd6e) SHA1(b2c12008d6d3e7544ba3c12a52a6abf9181842c8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "tp1-3.2c", 0xe000, 0x2000, CRC(5f3bf6e2) SHA1(d1b3335661b9b23cb10001416c515b77b5e783e9) )

	ROM_REGION( 0x8000, "sub", 0 )		/* 32k for the third CPU */
	ROM_LOAD16_BYTE( "tp1-4.8c", 0x0000, 0x4000, CRC(76997db3) SHA1(5023a2f20a5f2c9baff130f6832583493c71f883) )
	ROM_LOAD16_BYTE("tp1-5.10c", 0x0001, 0x4000, CRC(37de8786) SHA1(710365e34c05d01815844c414518f93234b6160b) )

	/* temporary space for graphics (disposed after conversion) */
	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tp1-7.5p", 0x0000, 0x2000, CRC(95076f9e) SHA1(1e3d32b21f6d46591ec3921aba51f672d64a9023) )	/* characters */

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "tp1-6.9t", 0x0000, 0x4000, CRC(481ffeaf) SHA1(c51735ad3a1dbb46ad414408b54554e9223b2219) )	/* sprites */

	ROM_REGION( 0x0600, "proms", 0 )	/* color proms */
	ROM_LOAD( "tp1-3.1r", 0x0000, 0x0100, CRC(cfce2fa5) SHA1(b42aa0f34d885389d2650bf7a0531b95703b8a28) )	// palette: red component
	ROM_LOAD( "tp1-2.1s", 0x0100, 0x0100, CRC(aeaf039d) SHA1(574560526100d38635aecd71eb73499c4f57d586) )	// palette: green component
	ROM_LOAD( "tp1-1.1t", 0x0200, 0x0100, CRC(08e7cde3) SHA1(5261aca6834d635d17f8afaa8e35848930030ba4) )	// palette: blue component
	ROM_LOAD( "tp1-4.5l", 0x0300, 0x0100, CRC(74138973) SHA1(2e21dbb1b19dd089da52e70fcb0ca91336e004e6) )	/* characters */
	ROM_LOAD( "tp1-5.2p", 0x0400, 0x0200, CRC(4d77fa5a) SHA1(2438910314b23ecafb553230244f3931861ad2da) )	/* sprites */

	ROM_REGION( 0x0100, "namco", 0 )	/* sound prom */
	ROM_LOAD( "tp1-6.3d", 0x0000, 0x0100, CRC(16a9166a) SHA1(847cbaf7c88616576c410177e066ae1d792ac0ba) )
ROM_END



GAME( 1983, liblrabl, 0, liblrabl, liblrabl, 58_56_56,  ROT0, "Namco", "Libble Rabble", 0 )
GAME( 1986, toypop,   0, toypop,   toypop,   58c_56_56, ROT0, "Namco", "Toypop", 0 )
