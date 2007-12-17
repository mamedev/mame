/***************************************************************************

    New York! New York! hardware

    Games Supported:
        * New York! New York! (2 sets)
        * Waga Seishun no Arcadia

    Known issues/to-do's:
        * What does port A on IC37 AY8910 do?  It looks like a DAC, but
          this is not supported by the writes to the port.  All the
          writes are 0x9X, usually 0x9b or 0x9d.  Note that this is
          incorrectly referred to as port B on the schematics, but the
          pin #'s confirm it is, in fact port A.
        * What is the main CPU clock?  11.2Mhz / 16 goes through
          a MC4044 and a MC4024 analog chips before going to the EXTAL
          pin of the M6809

    Notes:
        * The Sigma set has Japanese voice samples, while the Gottlieb
          one is English
        * In cocktail mode New York! New York! programs the CRTC with an
          incorrect value.  Interestingly, when the Flip Screen DIP
          is set, the value programmed is correct.  This bug does not
          exist in Waga Seishun no Arcadia
        * The Crosshatch switch only works on the title screen
        * The Service Mode switch, which displays the total number of
          credits stored in the NVRAM, only works on the "Start Game"
          screen after a coin has been insered.  Hold down the key to
          display the coin count
        * The schematics mixed up port A and B on both AY-8910


    Memory map main cpu (m6809)

    fedcba98
     --------
    000xxxxx  we1   $0000 8k (bitmap)
    100xxxxx  we1   $8000 8k (ram)

    010xxxxx  we2   $4000 8k (bitmap)
    110xxxxx  we2   $C000 8k (ram)

    001xxxxx  we3   $2000 16k x 3bits (color)

    011xxxxx  we4   $6000 16k x 3bits (color)

    10100000  SRAM  $A000 (HB4334P 1024-byte SRAM, but A8/A9 are always 0)
    10100001  CRTC  $A100
    10100010  PIA   $A200
    10100011  SOUND $A300 one latch for read one for write same address

    10101xxx  ROM7  $A800
    10110xxx  ROM6  $B000
    10111xxx  ROM5  $B800

    11100xxx  ROM4  $E000
    11101xxx  ROM3  $E800
    11110xxx  ROM2  $F000
    11111xxx  ROM1  $F800

***************************************************************************/

#include "driver.h"
#include "rescap.h"
#include "machine/6821pia.h"
#include "machine/74123.h"
#include "video/crtc6845.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/dac.h"



#define MAIN_CPU_MASTER_CLOCK	(11200000)
#define PIXEL_CLOCK				(MAIN_CPU_MASTER_CLOCK / 2)
#define CRTC_CLOCK				(MAIN_CPU_MASTER_CLOCK / 16)
#define AUDIO_1_MASTER_CLOCK	(4000000)
#define AUDIO_CPU_1_CLOCK		(AUDIO_1_MASTER_CLOCK / 4)	/* internal divisor */
#define AUDIO_2_MASTER_CLOCK	(4000000)
#define AUDIO_CPU_2_CLOCK		(AUDIO_2_MASTER_CLOCK / 4)	/* internal divisor */


static UINT8 *nyny_videoram_1;
static UINT8 *nyny_videoram_2;
static UINT8 *nyny_colorram_1;
static UINT8 *nyny_colorram_2;
static UINT8 flipscreen;

static UINT8 star_enable;
static UINT16 star_delay_counter;
static UINT16 star_shift_reg;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static WRITE8_HANDLER( flipscreen_w );
static WRITE8_HANDLER( audio_2_command_w );



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static void main_cpu_irq(int state)
{
	cpunum_set_input_line(0, M6809_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


static void main_cpu_firq(int state)
{
	cpunum_set_input_line(0, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  PIA1
 *
 *************************************/

static INTERRUPT_GEN( update_pia_1 )
{
	/* update the different PIA pins from the input ports */

	/* CA1 - copy of PA0 (COIN1) */
	pia_1_ca1_w(0, input_port_0_r(0) & 0x01);

	/* CA2 - copy of PA1 (SERVICE1) */
	pia_1_ca2_w(0, input_port_0_r(0) & 0x02);

	/* CB1 - (crosshatch) */
	pia_1_cb1_w(0, input_port_5_r(0));

	/* CB2 - NOT CONNECTED */
}


static const pia6821_interface pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_0_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, 0,
	/*irqs   : A/B             */ main_cpu_irq, main_cpu_irq
};



/*************************************
 *
 *  PIA2
 *
 *************************************/

static WRITE8_HANDLER( pia_2_port_a_w )
{
	star_delay_counter = (star_delay_counter & 0x0f00) | data;
}


static WRITE8_HANDLER( pia_2_port_b_w )
{
	/* bits 0-3 go to bits 8-11 of the star delay counter */
	star_delay_counter = (star_delay_counter & 0x00ff) | ((data & 0x0f) << 8);

	/* bit 4 is star field enable */
	star_enable = data & 0x10;

	/* bits 5-7 go to the music board connector */
	audio_2_command_w(0, data & 0xe0);
}


static const pia6821_interface pia_2_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_2_port_a_w, pia_2_port_b_w, flipscreen_w, 0,
	/*irqs   : A/B             */ main_cpu_firq, main_cpu_irq
};



/*************************************
 *
 *  IC48 #1 - 74123
 *
 *  This timer is responsible for
 *  delaying the setting of PIA2's
 *  CA1 line.  This delay ensures that
 *  CA1 is only changed in the VBLANK
 *  region, but not in HBLANK
 *
 *************************************/

static void ic48_1_74123_output_changed(int output)
{
	pia_2_ca1_w(0, output);
}


static TTL74123_interface ic48_1_intf =
{
	TTL74123_GROUNDED,	/* the hook up type */
	RES_K(22),			/* resistor connected to RCext */
	CAP_U(0.01),		/* capacitor connected to Cext and RCext */
	1,					/* A pin - driven by the CRTC */
	1,					/* B pin - pulled high */
	1,					/* Clear pin - pulled high */
	ic48_1_74123_output_changed
};



/*************************************
 *
 *  Machine start
 *
 *************************************/

static MACHINE_START( nyny )
{
	pia_config(1, &pia_1_intf);
	pia_config(2, &pia_2_intf);

	TTL74123_config(0, &ic48_1_intf);

	/* setup for save states */
	state_save_register_global(flipscreen);
	state_save_register_global(star_enable);
	state_save_register_global(star_delay_counter);
	state_save_register_global(star_shift_reg);
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

static MACHINE_RESET( nyny )
{
	pia_reset();
	TTL74123_reset(0);
}



/*************************************
 *
 *  Video system
 *
 *************************************/

#define NUM_PENS	(8)


static WRITE8_HANDLER( flipscreen_w )
{
	flipscreen = data ? 0 : 1;
}


static void *nyny_begin_update(running_machine *machine, int screen,
							   mame_bitmap *bitmap, const rectangle *cliprect)
{
	/* create the pens */
	offs_t i;
	static pen_t pens[NUM_PENS];

	for (i = 0; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}

	return pens;
}


static void nyny_update_row(mame_bitmap *bitmap, const rectangle *cliprect,
							UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, void *param)
{
	UINT8 cx;

	pen_t *pens = (pen_t *)param;
	UINT8 x = 0;

	for (cx = 0; cx < x_count; cx++)
	{
		int i;
		UINT8 data1, data2, color1, color2;

		/* the memory is hooked up to the MA, RA lines this way */
		offs_t offs = ((ma << 5) & 0x8000) |
		   			  ((ma << 3) & 0x1f00) |
		   			  ((ra << 5) & 0x00e0) |
		   			  ((ma << 0) & 0x001f);

		if (flipscreen)
			offs = offs ^ 0x9fff;

		data1 = nyny_videoram_1[offs];
		data2 = nyny_videoram_2[offs];
		color1 = nyny_colorram_1[offs] & 0x07;
		color2 = nyny_colorram_2[offs] & 0x07;

		for (i = 0; i < 8; i++)
		{
			UINT8 bit1, bit2, color;

			if (flipscreen)
			{
				bit1 = data1 & 0x80;
				bit2 = data2 & 0x80;

				data1 = data1 << 1;
				data2 = data2 << 1;
			}
			else
			{
				bit1 = data1 & 0x01;
				bit2 = data2 & 0x01;

				data1 = data1 >> 1;
				data2 = data2 >> 1;
			}

			/* plane 1 has priority over plane 2 */
			if (bit1)
				color = color1;
			else
				color = bit2 ? color2 : 0;

			*BITMAP_ADDR32(bitmap, y, x) = pens[color];

			x = x + 1;
		}

		ma = ma + 1;
	}
}


INLINE void shift_star_generator(void)
{
	star_shift_reg = (star_shift_reg << 1) | (((~star_shift_reg >> 15) & 0x01) ^ ((star_shift_reg >> 2) & 0x01));
}


static void nyny_end_update(mame_bitmap *bitmap, const rectangle *cliprect, void *param)
{
	/* the the star field into the bitmap */
	int y;

	pen_t *pens = (pen_t *)param;
	UINT16 delay_counter = star_delay_counter;

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int x;

		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			/* check if the star status */
			if (star_enable &&
			    (*BITMAP_ADDR32(bitmap, y, x) == 0) &&
			    ((star_shift_reg & 0x80ff) == 0x00ff) &&
			    (((y & 0x01) ^ flipscreen) ^ (((x & 0x08) >> 3) ^ flipscreen)))
			{
				UINT8 color = ((star_shift_reg & 0x0100) >>  8) |	/* R */
							  ((star_shift_reg & 0x0400) >>  9) |	/* G */
							  ((star_shift_reg & 0x1000) >> 10);	/* B */

				*BITMAP_ADDR32(bitmap, y, x) = pens[color];
			}

			if (delay_counter == 0)
				shift_star_generator();
			else
				delay_counter = delay_counter - 1;
		}
	}
}


static void nyny_display_enable_changed(int display_enabled)
{
	TTL74123_A_w(0, display_enabled);
}


static const crtc6845_interface crtc6845_intf =
{
	0,							/* screen we are acting on */
	CRTC_CLOCK, 				/* the clock (pin 21) of the chip */
	8,							/* number of pixels per video memory address */
	nyny_begin_update,			/* before pixel update callback */
	nyny_update_row,			/* row update callback */
	nyny_end_update,			/* after pixel update callback */
	nyny_display_enable_changed	/* call back for display state changes */
};


static VIDEO_START( nyny )
{
	/* configure the CRT controller */
	crtc6845_config(0, &crtc6845_intf);
}



/*************************************
 *
 *  Audio system - CPU 1
 *
 *************************************/

static WRITE8_HANDLER( audio_1_command_w )
{
	soundlatch_w(0, data);
	cpunum_set_input_line(1, M6802_IRQ_LINE, HOLD_LINE);
}


static WRITE8_HANDLER( audio_1_answer_w )
{
	soundlatch3_w(0, data);
	cpunum_set_input_line(0, M6809_IRQ_LINE, HOLD_LINE);
}


static WRITE8_HANDLER( nyny_ay8910_37_port_a_w )
{
	/* not sure what this does */

	/*logerror("%x PORT A write %x at  Y=%x X=%x\n", safe_activecpu_get_pc(), data, video_screen_get_vpos(0),  video_screen_get_hpos(0));*/
}


static WRITE8_HANDLER( nyny_ay8910_37_port_b_w )
{
	DAC_data_w(0, data);
}


static struct AY8910interface ay8910_37_interface =
{
	0,
	0,
	nyny_ay8910_37_port_a_w,
	nyny_ay8910_37_port_b_w
};


static struct AY8910interface ay8910_64_interface =
{
	input_port_3_r,
	input_port_2_r
};



/*************************************
 *
 *  Audio system - CPU 2
 *
 *************************************/

static WRITE8_HANDLER( audio_2_command_w )
{
	soundlatch2_w(0, (data & 0x60) >> 5);
	cpunum_set_input_line(2, M6802_IRQ_LINE, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static READ8_HANDLER( nyny_pia_1_2_r )
{
	UINT8 ret = 0;

	/* the address bits are directly connected to the chip selects */
	if (offset & 0x04)  ret = pia_1_r(offset & 0x03);
	if (offset & 0x08)  ret = pia_2_alt_r(offset & 0x03);

	return ret;
}


static WRITE8_HANDLER( nyny_pia_1_2_w )
{
	/* the address bits are directly connected to the chip selects */
	if (offset & 0x04)  pia_1_w(offset & 0x03, data);
	if (offset & 0x08)  pia_2_alt_w(offset & 0x03, data);
}


static ADDRESS_MAP_START( nyny_main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_BASE(&nyny_videoram_1)
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_BASE(&nyny_colorram_1)
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_BASE(&nyny_videoram_2)
	AM_RANGE(0x6000, 0x7fff) AM_RAM AM_BASE(&nyny_colorram_2)
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xa0ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size) /* SRAM (coin counter, shown when holding F2) */
	AM_RANGE(0xa100, 0xa100) AM_MIRROR(0x00fe) AM_WRITE(crtc6845_address_w)
	AM_RANGE(0xa101, 0xa101) AM_MIRROR(0x00fe) AM_WRITE(crtc6845_register_w)
	AM_RANGE(0xa200, 0xa20f) AM_MIRROR(0x00f0) AM_READWRITE(nyny_pia_1_2_r, nyny_pia_1_2_w)
	AM_RANGE(0xa300, 0xa300) AM_MIRROR(0x00ff) AM_READWRITE(soundlatch3_r, audio_1_command_w)
	AM_RANGE(0xa400, 0xa7ff) AM_NOP
	AM_RANGE(0xa800, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( nyny_audio_1_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(15) )
	AM_RANGE(0x0000, 0x007f) AM_RAM		/* internal RAM */
	AM_RANGE(0x0080, 0x0fff) AM_NOP
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x0fff) AM_READWRITE(soundlatch_r, audio_1_answer_w)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0fff) AM_READ(input_port_4_r)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0ffc) AM_READWRITE(AY8910_read_port_0_r, AY8910_write_port_0_w)
	AM_RANGE(0x3001, 0x3001) AM_MIRROR(0x0ffc) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x3002, 0x3002) AM_MIRROR(0x0ffc) AM_READWRITE(AY8910_read_port_1_r, AY8910_write_port_1_w)
	AM_RANGE(0x3003, 0x3003) AM_MIRROR(0x0ffc) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x4000, 0x4fff) AM_NOP
	AM_RANGE(0x5000, 0x57ff) AM_MIRROR(0x0800) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_MIRROR(0x0800) AM_ROM
	AM_RANGE(0x7000, 0x77ff) AM_MIRROR(0x0800) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( nyny_audio_2_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(15) )
	AM_RANGE(0x0000, 0x007f) AM_RAM		/* internal RAM */
	AM_RANGE(0x0080, 0x0fff) AM_NOP
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x0fff) AM_READ(soundlatch2_r)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0ffe) AM_READWRITE(AY8910_read_port_2_r, AY8910_write_port_2_w)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x0ffe) AM_WRITE(AY8910_control_port_2_w)
	AM_RANGE(0x3000, 0x6fff) AM_NOP
	AM_RANGE(0x7000, 0x77ff) AM_MIRROR(0x0800) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( nyny )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )     /* PIA0 PA0 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 )  /* PIA0 PA1 */
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_HIGH)	/* PIA0 PA2 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )	/* PIA0 PA3 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL /* PIA0 PA4 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )	/* PIA0 PA5 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )	/* PIA0 PA6 */
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL /* PIA0 PB0 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL /* PIA0 PB1 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY	/* PIA0 PB2 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY	/* PIA0 PB3 */
    PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("SW1")	/* port 2 */
	PORT_DIPNAME( 0x03, 0x03, "Bombs from UFO (Screens 3+)" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(	0x01, "3" )
	PORT_DIPSETTING(	0x00, "6" )
	PORT_DIPSETTING(	0x03, "9" )
	PORT_DIPSETTING(	0x02, "12" )
	PORT_DIPNAME( 0x04, 0x00, "Bombs from UFO (Screens 1 and 2)" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(	0x04, "6" )
	PORT_DIPSETTING(	0x00, "9" )
	PORT_DIPNAME( 0x80, 0x80, "Voice Volume" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(	0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(	0x80, DEF_STR( High ) )

	PORT_START_TAG("SW2")	/* port 3 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(	0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x00, "Bonus Game" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(	0x18, "No Bonus Game" )
	PORT_DIPSETTING(	0x10, "5000" )
	PORT_DIPSETTING(	0x00, "10000" )
	PORT_DIPSETTING(	0x08, "15000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(	0x00, "3000" )
	PORT_DIPSETTING(	0x40, "5000" )
	PORT_DIPNAME( 0x80, 0x80, "Bonus Life Awarded" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Yes ) )

	PORT_START_TAG("SW3")	/* port 4 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x1c, 0x00, "Vertical Screen Position" ) PORT_DIPLOCATION("SW3:3,4,5")
	PORT_DIPSETTING(	0x1c, "-1" )
	PORT_DIPSETTING(	0x18, "-2" )
	PORT_DIPSETTING(	0x14, "-3" )
	PORT_DIPSETTING(	0x00, "Neutral" )
	PORT_DIPSETTING(	0x04, "+1" )
	PORT_DIPSETTING(	0x08, "+2" )
	PORT_DIPSETTING(	0x0c, "+3" )
	PORT_DIPNAME( 0xe0, 0x00, "Horizontal Screen Position" ) PORT_DIPLOCATION("SW3:6,7,8")
	PORT_DIPSETTING(	0xe0, "-1" )
	PORT_DIPSETTING(	0xc0, "-2" )
	PORT_DIPSETTING(	0xa0, "-3" )
	PORT_DIPSETTING(	0x00, "Neutral" )
	PORT_DIPSETTING(	0x60, "+1" )
	PORT_DIPSETTING(	0x40, "+2" )
	PORT_DIPSETTING(	0x20, "+3" )

    PORT_START      /* connected to PIA1 CB1 input */
    PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PS1 (Crosshatch)") PORT_CODE(KEYCODE_F1)

INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( nyny )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6809, 1400000)	/* 1.40 MHz? The clock signal is generated by analog chips */
	MDRV_CPU_PROGRAM_MAP(nyny_main_map,0)
	MDRV_CPU_PERIODIC_INT(update_pia_1, 25)

	MDRV_CPU_ADD(M6802, AUDIO_CPU_1_CLOCK)
	MDRV_CPU_PROGRAM_MAP(nyny_audio_1_map,0)

	MDRV_CPU_ADD(M6802, AUDIO_CPU_2_CLOCK)
	MDRV_CPU_PROGRAM_MAP(nyny_audio_2_map,0)

	MDRV_MACHINE_START(nyny)
	MDRV_MACHINE_RESET(nyny)
	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_VIDEO_START(nyny)
	MDRV_VIDEO_UPDATE(crtc6845)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 256, 0, 256, 256, 0, 256)	/* temporary, CRTC will configure screen */

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, AUDIO_CPU_1_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_37_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, AUDIO_CPU_1_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_64_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, AUDIO_CPU_2_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.03)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( nyny )
	ROM_REGION(0x10000, REGION_CPU1, 0)	/* main CPU */
	ROM_LOAD( "nyny01s.100",  0xa800, 0x0800, CRC(a2b76eca) SHA1(e46717e6ad330be4c4e7d9fab4f055f89aa31bcc) )
	ROM_LOAD( "nyny02s.099",  0xb000, 0x0800, CRC(ef2d4dae) SHA1(718c0ecf7770a780aebb1dc8bf4ca86ea0a5ea28) )
	ROM_LOAD( "nyny03s.098",  0xb800, 0x0800, CRC(2734c229) SHA1(b028d057d26838bae50b8ddb90a3755b5315b4ee) )
	ROM_LOAD( "nyny04s.097",  0xe000, 0x0800, CRC(bd94087f) SHA1(02dde604bb84097fcd95c434847c55198b4e4309) )
	ROM_LOAD( "nyny05s.096",  0xe800, 0x0800, CRC(248b22c4) SHA1(d64d89bf78fa19d36e02720c296a60621ab8fe21) )
	ROM_LOAD( "nyny06s.095",  0xf000, 0x0800, CRC(8c073052) SHA1(0ce103ac0e79124ac9f1e097dda1a0664b92b89b) )
	ROM_LOAD( "nyny07s.094",  0xf800, 0x0800, CRC(d49d7429) SHA1(c12eaae7ba0b1d44c45a584232db03c5731c046a) )

	ROM_REGION(0x10000, REGION_CPU2, 0)	/* first audio CPU */
	ROM_LOAD( "nyny08.093",   0x5000, 0x0800, CRC(19ddb6c3) SHA1(0097fad542f9a33849565093c2fb106d90007b1a) )
	ROM_LOAD( "nyny09.092",   0x6000, 0x0800, CRC(a359c6f1) SHA1(1bc7b487581399908c3cec823733810fb6d944ce) )
	ROM_LOAD( "nyny10.091",   0x7000, 0x0800, CRC(a72a70fa) SHA1(deed7dec9cc43fa1d6c4854ba18169c894c9a2f0) )

	ROM_REGION(0x10000, REGION_CPU3, 0) /* second audio CPU */
	ROM_LOAD( "nyny11.snd",   0x7000, 0x0800, CRC(650450fc) SHA1(214693df394ca05eff5dbe1e800107d326ba80f6) )
ROM_END


ROM_START( nynyg )
	ROM_REGION(0x10000, REGION_CPU1, 0)	/* main CPU */
	ROM_LOAD( "gny1.cpu",     0xa800, 0x0800, CRC(fb5b8f17) SHA1(2202325451dfd4e7c16cba93f0fade46929ffa72) )
	ROM_LOAD( "gny2.cpu",     0xb000, 0x0800, CRC(d248dd93) SHA1(0c4579698f8917332041c08af6902b8f8acd7d62) )
	ROM_LOAD( "gny3.cpu",     0xb800, 0x0800, CRC(223a9d09) SHA1(c2b12270d375587489208d6a1b37a4e3ec87bc20) )
	ROM_LOAD( "gny4.cpu",     0xe000, 0x0800, CRC(7964ec1f) SHA1(dba3dc2e928fb3fc04a9dca12951343669a4ecbe) )
	ROM_LOAD( "gny5.cpu",     0xe800, 0x0800, CRC(4799dcfc) SHA1(13dcc4a58a029c14a4e9acd0bf584c71d5302c03) )
	ROM_LOAD( "gny6.cpu",     0xf000, 0x0800, CRC(4839d4d2) SHA1(cfd6f2f252ee2f6a4d881496a017c02d7dd77944) )
	ROM_LOAD( "gny7.cpu",     0xf800, 0x0800, CRC(b7564c5b) SHA1(e1d8fe7f37aa7aa98f18c538fe6e688675cc2de1) )

	ROM_REGION(0x10000, REGION_CPU2, 0)	/* first audio CPU */
	ROM_LOAD( "gny8.cpu",     0x5000, 0x0800, CRC(e0bf7d00) SHA1(7afca3affa413179f4f59ce2cad89525cfa5efbc) )
	ROM_LOAD( "gny9.cpu",     0x6000, 0x0800, CRC(639bc81a) SHA1(91819d49099e438ac8c70920a787aeaed3ed82e9) )
	ROM_LOAD( "gny10.cpu",    0x7000, 0x0800, CRC(73764021) SHA1(bb2f62130142487afbd8d2540e2d4fe5bb67c4ee) )

	ROM_REGION(0x10000, REGION_CPU3, 0) /* second audio CPU */
	/* The original dump of this ROM was bad [FIXED BITS (x1xxxxxx)] */
	/* Since what's left is identical to the Sigma version, I'm assuming it's the same. */
	ROM_LOAD( "nyny11.snd",   0x7000, 0x0800, CRC(650450fc) SHA1(214693df394ca05eff5dbe1e800107d326ba80f6) )
ROM_END


ROM_START( arcadia )
	ROM_REGION(0x10000, REGION_CPU1, 0)	/* main CPU */
	ROM_LOAD( "ar-01",        0xa800, 0x0800, CRC(7b7e8f27) SHA1(2bb1d07d87ad5b952de9460c840d7e8b59ed1b4a) )
	ROM_LOAD( "ar-02",        0xb000, 0x0800, CRC(81d9e172) SHA1(4279582f1edf54f0974fa277565d8ade6d9faa50) )
	ROM_LOAD( "ar-03",        0xb800, 0x0800, CRC(2c5feb05) SHA1(6f8952e7744ba7d7b8b345d67f546b504f7a3b30) )
	ROM_LOAD( "ar-04",        0xe000, 0x0800, CRC(66fcbd7f) SHA1(7b8c09593b7d0d25cbe0b28097d58772c32f13bb) )
	ROM_LOAD( "ar-05",        0xe800, 0x0800, CRC(b2320e20) SHA1(977afc2d26ef500eff4499e6bc61f14314b19130) )
	ROM_LOAD( "ar-06",        0xf000, 0x0800, CRC(27b79cc0) SHA1(2c5c3a9a09069751c5e9c23d0840ee4996006c0b) )
	ROM_LOAD( "ar-07",        0xf800, 0x0800, CRC(be77a477) SHA1(817c069855634dd844f0068d64bfbf1862980d6b) )

	ROM_REGION(0x10000, REGION_CPU2, 0)	/* first audio CPU */
	ROM_LOAD( "ar-08",        0x5000, 0x0800, CRC(38569b25) SHA1(887a9afaa65d0961097f7fb5f1ae390d40e9c164) )
	ROM_LOAD( "nyny09.092",   0x6000, 0x0800, CRC(a359c6f1) SHA1(1bc7b487581399908c3cec823733810fb6d944ce) )
	ROM_LOAD( "nyny10.091",   0x7000, 0x0800, CRC(a72a70fa) SHA1(deed7dec9cc43fa1d6c4854ba18169c894c9a2f0) )

	ROM_REGION(0x10000, REGION_CPU3, 0) /* second audio CPU */
	ROM_LOAD( "ar-11",        0x7000, 0x0800, CRC(208f4488) SHA1(533f8942e1c964cc88253e9dc4ec711f77607e4c) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980, nyny,    0,    nyny, nyny, 0, ROT270, "Sigma Enterprises Inc.", "New York! New York!", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1980, nynyg,   nyny, nyny, nyny, 0, ROT270, "Sigma Enterprises Inc. (Gottlieb license)", "New York! New York! (Gottlieb)", GAME_IMPERFECT_SOUND  | GAME_SUPPORTS_SAVE )
GAME( 1980, arcadia, nyny, nyny, nyny, 0, ROT270, "Sigma Enterprises Inc.", "Waga Seishun no Arcadia", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
