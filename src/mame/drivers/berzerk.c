/***************************************************************************

    Berzerk hardware

    Driver by Zsolt Vasvari
    Original sound driver by Alex Judd
    New sound driver by Aaron Giles, R. Belmont and Lord Nightmare

***************************************************************************/

#include "driver.h"
#include "exidy.h"
#include "machine/74181.h"
#include "sound/s14001a.h"
#include "video/resnet.h"


#define MONITOR_TYPE_PORT_TAG ("MONITOR_TYPE")

#define MASTER_CLOCK				(XTAL_10MHz)
#define MAIN_CPU_CLOCK  			(MASTER_CLOCK / 4)
#define PIXEL_CLOCK  				(MASTER_CLOCK / 2)
#define S14001A_CLOCK   			(MASTER_CLOCK / 2)
#define HTOTAL						(0x140)
#define HBEND						(0x000)
#define HBSTART						(0x100)
#define VTOTAL						(0x106)
#define VBEND						(0x020)
#define VBSTART						(0x100)
#define VCOUNTER_START_NO_VBLANK	(0x020)
#define VCOUNTER_START_VBLANK		(0x0da)
#define IRQS_PER_FRAME				(2)
#define NMIS_PER_FRAME				(8)

static const UINT8 irq_trigger_counts[IRQS_PER_FRAME] = { 0x80, 0xda };
static const UINT8 irq_trigger_v256s [IRQS_PER_FRAME] = { 0x00, 0x01 };

static const UINT8 nmi_trigger_counts[NMIS_PER_FRAME] = { 0x30, 0x50, 0x70, 0x90, 0xb0, 0xd0, 0xf0, 0xf0 };
static const UINT8 nmi_trigger_v256s [NMIS_PER_FRAME] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };


static UINT8 *berzerk_videoram;
static size_t berzerk_videoram_size;
static UINT8 *berzerk_colorram;

static UINT8 magicram_control;		/* 8-bit latch @ 6C */
static UINT8 last_shift_data;		/* 7-bit latch @ 7C */
static UINT8 intercept;				/* J-K flip-flop @ 6B */

static emu_timer *irq_timer;
static emu_timer *nmi_timer;
static UINT8 irq_enabled;			/* J-K flip-flop @ 3D */
static UINT8 nmi_enabled;			/* flip-flop made out of 2 NAND gates @ 1D */



/*************************************
 *
 *  Start LED handling
 *
 *************************************/

static READ8_HANDLER( led_on_r )
{
	set_led_status(0, 1);

	return 0;
}


static WRITE8_HANDLER( led_on_w )
{
	set_led_status(0, 1);
}


static READ8_HANDLER( led_off_r )
{
	set_led_status(0, 0);

	return 0;
}


static WRITE8_HANDLER( led_off_w )
{
	set_led_status(0, 0);
}



/*************************************
 *
 *  Convert to/from our line counting
 *  to the hardware's vsync chain
 *
 *************************************/

static void vpos_to_vysnc_chain_counter(int vpos, UINT8 *counter, UINT8 *v256)
{
	/* convert from a vertical position to the actual values on the vertical sync counters */
	*v256 = ((vpos < VBEND) || (vpos >= VBSTART));

	if (*v256)
	{
		int temp = vpos - VBSTART + VCOUNTER_START_VBLANK;

		if (temp < 0)
			*counter = temp + VTOTAL;
		else
			*counter = temp;
	}
	else
		*counter = vpos;
}


static int vysnc_chain_counter_to_vpos(UINT8 counter, UINT8 v256)
{
	/* convert from the vertical sync counters to an actual vertical position */
	int vpos;

	if (v256)
	{
		vpos = counter - VCOUNTER_START_VBLANK + VBSTART;

		if (vpos >= VTOTAL)
			vpos = vpos - VTOTAL;
	}
	else
		vpos = counter;

	return vpos;
}



/*************************************
 *
 *  IRQ generation
 *
 *  There are two IRQ's per frame
 *
 *************************************/

static WRITE8_HANDLER( irq_enable_w )
{
	irq_enabled = data & 0x01;
}


static TIMER_CALLBACK( irq_callback )
{
	int irq_number = param;
	UINT8 next_counter;
	UINT8 next_v256;
	int next_vpos;
	int next_irq_number;

	/* set the IRQ line if enabled */
	if (irq_enabled)
		cpunum_set_input_line_and_vector(0, 0, HOLD_LINE, 0xfc);

	/* set up for next interrupt */
	next_irq_number = (irq_number + 1) % IRQS_PER_FRAME;
	next_counter = irq_trigger_counts[next_irq_number];
	next_v256 = irq_trigger_v256s[next_irq_number];

	next_vpos = vysnc_chain_counter_to_vpos(next_counter, next_v256);
	timer_adjust(irq_timer, video_screen_get_time_until_pos(0, next_vpos, 0), next_irq_number, attotime_zero);
}


static void create_irq_timer(void)
{
	irq_timer = timer_alloc(irq_callback, NULL);
}


static void start_irq_timer(void)
{
	int vpos = vysnc_chain_counter_to_vpos(irq_trigger_counts[0], irq_trigger_v256s[0]);
	timer_adjust(irq_timer, video_screen_get_time_until_pos(0, vpos, 0), 0, attotime_zero);
}



/*************************************
 *
 *  NMI generation
 *
 *  An NMI is asserted roughly every
 *  32 scanlines when V16 clocks HI.
 *  The NMI is cleared 2 pixels later.
 *  Since this happens so quickly, I am
 *  not emulating it, just pulse
 *  the line instead.
 *
 *************************************/

static WRITE8_HANDLER( nmi_enable_w )
{
	nmi_enabled = 1;
}


static WRITE8_HANDLER( nmi_disable_w )
{
	nmi_enabled = 0;
}


static READ8_HANDLER( nmi_enable_r )
{
	nmi_enabled = 1;

	return 0;
}


static READ8_HANDLER( nmi_disable_r )
{
	nmi_enabled = 0;

	return 0;
}


static TIMER_CALLBACK( nmi_callback )
{
	int nmi_number = param;
	UINT8 next_counter;
	UINT8 next_v256;
	int next_vpos;
	int next_nmi_number;

	/* pulse the NMI line if enabled */
	if (nmi_enabled)
		cpunum_set_input_line(0, INPUT_LINE_NMI, PULSE_LINE);

	/* set up for next interrupt */
	next_nmi_number = (nmi_number + 1) % NMIS_PER_FRAME;
	next_counter = nmi_trigger_counts[next_nmi_number];
	next_v256 = nmi_trigger_v256s[next_nmi_number];

	next_vpos = vysnc_chain_counter_to_vpos(next_counter, next_v256);
	timer_adjust(nmi_timer, video_screen_get_time_until_pos(0, next_vpos, 0), next_nmi_number, attotime_zero);
}


static void create_nmi_timer(void)
{
	nmi_timer = timer_alloc(nmi_callback, NULL);
}


static void start_nmi_timer(void)
{
	int vpos = vysnc_chain_counter_to_vpos(nmi_trigger_counts[0], nmi_trigger_v256s[0]);
	timer_adjust(nmi_timer, video_screen_get_time_until_pos(0, vpos, 0), 0, attotime_zero);
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

static MACHINE_START( berzerk )
{
	create_irq_timer();
	create_nmi_timer();

	/* register for state saving */
	state_save_register_global(magicram_control);
	state_save_register_global(last_shift_data);
	state_save_register_global(intercept);
	state_save_register_global(irq_enabled);
	state_save_register_global(nmi_enabled);
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

static MACHINE_RESET( berzerk )
{
	irq_enabled = 0;
	nmi_enabled = 0;
	set_led_status(0, 0);
	magicram_control = 0;

	start_irq_timer();
	start_nmi_timer();
}



/*************************************
 *
 *  Video system
 *
 *************************************/

#define NUM_PENS	(0x10)

#define LS181_12C 	(0)
#define LS181_10C 	(1)


static VIDEO_START( berzerk )
{
	TTL74181_config(LS181_12C, 0);
	TTL74181_write(LS181_12C, TTL74181_INPUT_M, 1, 1);

	TTL74181_config(LS181_10C, 0);
	TTL74181_write(LS181_10C, TTL74181_INPUT_M, 1, 1);
}


static WRITE8_HANDLER( magicram_w )
{
	UINT8 alu_output;

	UINT8 current_video_data = berzerk_videoram[offset];

	/* shift data towards LSB.  MSB bits are filled by data from last_shift_data.
       The shifter consists of 5 74153 devices @ 7A, 8A, 9A, 10A and 11A,
       followed by 4 more 153's at 11B, 10B, 9B and 8B, which optionally
       reverse the order of the resulting bits */
	UINT8 shift_flop_output = (((UINT16)last_shift_data << 8) | data) >> (magicram_control & 0x07);

	if (magicram_control & 0x08)
		shift_flop_output = BITSWAP8(shift_flop_output, 0, 1, 2, 3, 4, 5, 6, 7);

	/* collision detection - AND gate output goes to the K pin of the flip-flop,
       while J is LO, therefore, it only resets, never sets */
	if (shift_flop_output & current_video_data)
		intercept = 0;

	/* perform ALU step */
	TTL74181_write(LS181_12C, TTL74181_INPUT_A0, 4, shift_flop_output & 0x0f);
	TTL74181_write(LS181_10C, TTL74181_INPUT_A0, 4, shift_flop_output >> 4);
	TTL74181_write(LS181_12C, TTL74181_INPUT_B0, 4, current_video_data & 0x0f);
	TTL74181_write(LS181_10C, TTL74181_INPUT_B0, 4, current_video_data >> 4);
	TTL74181_write(LS181_12C, TTL74181_INPUT_S0, 4, magicram_control >> 4);
	TTL74181_write(LS181_10C, TTL74181_INPUT_S0, 4, magicram_control >> 4);

	alu_output = (TTL74181_read(LS181_10C, TTL74181_OUTPUT_F0, 4) << 4) |
				 (TTL74181_read(LS181_12C, TTL74181_OUTPUT_F0, 4) << 0);

	berzerk_videoram[offset] = alu_output ^ 0xff;

	/* save data for next time */
	last_shift_data = data & 0x7f;
}


static WRITE8_HANDLER( magicram_control_w )
{
	/* save the control byte, clear the shift data latch,
       and set the intercept flip-flop */
	magicram_control = data;
	last_shift_data = 0;
	intercept = 1;
}


static READ8_HANDLER( intercept_v256_r )
{
	UINT8 counter;
	UINT8 v256;

	vpos_to_vysnc_chain_counter(video_screen_get_vpos(0), &counter, &v256);

	return (!intercept << 7) | v256;
}


static void get_pens(pen_t *pens)
{
	static const int resistances_wg[] = { 750, 0 };
	static const int resistances_el[] = { 1.0 / ((1.0 / 750.0) + (1.0 / 360.0)), 0 };

	int color;
	double color_weights[2];

	if (readinputportbytag(MONITOR_TYPE_PORT_TAG) == 0)
		compute_resistor_weights(0, 0xff, -1.0,
								 2, resistances_wg, color_weights, 0, 270,
								 2, resistances_wg, color_weights, 0, 270,
								 2, resistances_wg, color_weights, 0, 270);
	else
		compute_resistor_weights(0, 0xff, -1.0,
								 2, resistances_el, color_weights, 0, 270,
								 2, resistances_el, color_weights, 0, 270,
								 2, resistances_el, color_weights, 0, 270);

	for (color = 0; color < NUM_PENS; color++)
	{
		UINT8 r_bit = (color >> 0) & 0x01;
		UINT8 g_bit = (color >> 1) & 0x01;
		UINT8 b_bit = (color >> 2) & 0x01;
		UINT8 i_bit = (color >> 3) & 0x01;

		UINT8 r = combine_2_weights(color_weights, r_bit & i_bit, r_bit);
		UINT8 g = combine_2_weights(color_weights, g_bit & i_bit, g_bit);
		UINT8 b = combine_2_weights(color_weights, b_bit & i_bit, b_bit);

		pens[color] = MAKE_RGB(r, g, b);
	}
}


static VIDEO_UPDATE( berzerk )
{
	pen_t pens[NUM_PENS];
	offs_t offs;

	get_pens(pens);

	for (offs = 0; offs < berzerk_videoram_size; offs++)
	{
		int i;

		UINT8 data = berzerk_videoram[offs];
		UINT8 color = berzerk_colorram[((offs >> 2) & 0x07e0) | (offs & 0x001f)];

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		for (i = 0; i < 4; i++)
		{
			pen_t pen = (data & 0x80) ? pens[color >> 4] : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pen;

			x = x + 1;
			data = data << 1;
		}

		for (; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? pens[color & 0x0f] : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pen;

			x = x + 1;
			data = data << 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  Audio system
 *
 *************************************/

static const struct S14001A_interface berzerk_s14001a_interface =
{
	REGION_SOUND1	/* voice data region */
};


static const struct CustomSound_interface berzerk_custom_interface =
{
	exidy_sh6840_sh_start,
	0,
	exidy_sh6840_sh_reset
};


static WRITE8_HANDLER( berzerk_audio_w )
{
	switch (offset)
	{
	/* offset 6 writes to the sfxcontrol latch */
	case 6:
		exidy_sfxctrl_w(data >> 6, data);
		break;

	/* offset 4 writes to the S14001A */
	case 4:
		if ((data & 0xc0) == 0x40) /* VSU-1000 control write */
		{
			/* volume and frequency control goes here */
			/* mame_printf_debug("TODO: VSU-1000 Control write (ignored for now)\n");*/
			S14001A_set_volume(((data & 0x38) >> 3) + 1);
			S14001A_set_rate((16 - (data & 0x07)) * 16); /* second LS161 has load triggered by its own TC(when it equals 16) long before the first ls161 will TC and fire again, so effectively it only divides by 15 and not 16. If the clock, as opposed to the E enable, had been tied to the first LS161's TC instead, it would divide by 16 as expected */
		}
		else if ((data & 0xc0) != 0x00)
			/* vsu-1000 ignores these writes entirely */
			mame_printf_debug("bogus write ignored\n");
		else
		{
			/* select word input */
			if (S14001A_bsy_0_r()) /* skip if busy... */
			{
				mame_printf_debug("S14001A busy, ignoring write\n");
				break;
			}

			/* write to the register */
			S14001A_reg_0_w(data & 0x3f);
			S14001A_rst_0_w(1);
			S14001A_rst_0_w(0);
		}
		break;

	/* everything else writes to the 6840 */
	default:
		exidy_sh6840_w(offset, data);
		break;

	}
}


static READ8_HANDLER( berzerk_audio_r )
{
	return ((offset == 4) && !S14001A_bsy_0_r()) ? 0x40 : 0x00;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( berzerk_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x0bff) AM_MIRROR(0x0400) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x1000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_BASE(&berzerk_videoram) AM_SIZE(&berzerk_videoram_size) AM_SHARE(1)
	AM_RANGE(0x6000, 0x7fff) AM_READWRITE(MRA8_RAM, magicram_w) AM_SHARE(1)
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x3800) AM_RAM AM_BASE(&berzerk_colorram)
	AM_RANGE(0xc000, 0xffff) AM_NOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( frenzy_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_BASE(&berzerk_videoram) AM_SIZE(&berzerk_videoram_size) AM_SHARE(1)
	AM_RANGE(0x6000, 0x7fff) AM_READWRITE(MRA8_RAM, magicram_w) AM_SHARE(1)
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x3800) AM_RAM AM_BASE(&berzerk_colorram)
	AM_RANGE(0xc000, 0xcfff) AM_ROM
	AM_RANGE(0xf800, 0xfbff) AM_MIRROR(0x0400) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
ADDRESS_MAP_END



/*************************************
 *
 *  Port handlers
 *
 *************************************/

static ADDRESS_MAP_START( berzerk_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x3f) AM_NOP
	AM_RANGE(0x40, 0x47) AM_READWRITE(berzerk_audio_r, berzerk_audio_w)
	AM_RANGE(0x48, 0x48) AM_READWRITE(input_port_0_r, MWA8_NOP)
	AM_RANGE(0x49, 0x49) AM_READWRITE(input_port_1_r, MWA8_NOP)
	AM_RANGE(0x4a, 0x4a) AM_READWRITE(input_port_2_r, MWA8_NOP)
	AM_RANGE(0x4b, 0x4b) AM_READWRITE(MRA8_NOP, magicram_control_w)
	AM_RANGE(0x4c, 0x4c) AM_READWRITE(nmi_enable_r, nmi_enable_w)
	AM_RANGE(0x4d, 0x4d) AM_READWRITE(nmi_disable_r, nmi_disable_w)
	AM_RANGE(0x4e, 0x4e) AM_READWRITE(intercept_v256_r, MWA8_NOP)
	AM_RANGE(0x4f, 0x4f) AM_READWRITE(MRA8_NOP, irq_enable_w)
	AM_RANGE(0x50, 0x57) AM_NOP /* second sound board, but not used */
	AM_RANGE(0x58, 0x5f) AM_NOP
	AM_RANGE(0x60, 0x60) AM_MIRROR(0x18) AM_READWRITE(input_port_3_r, MWA8_NOP)
	AM_RANGE(0x61, 0x61) AM_MIRROR(0x18) AM_READWRITE(input_port_4_r, MWA8_NOP)
	AM_RANGE(0x62, 0x62) AM_MIRROR(0x18) AM_READWRITE(input_port_5_r, MWA8_NOP)
	AM_RANGE(0x63, 0x63) AM_MIRROR(0x18) AM_READWRITE(input_port_6_r, MWA8_NOP)
	AM_RANGE(0x64, 0x64) AM_MIRROR(0x18) AM_READWRITE(input_port_7_r, MWA8_NOP)
	AM_RANGE(0x65, 0x65) AM_MIRROR(0x18) AM_READWRITE(input_port_8_r, MWA8_NOP)
	AM_RANGE(0x66, 0x66) AM_MIRROR(0x18) AM_READWRITE(led_off_r, led_off_w)
	AM_RANGE(0x67, 0x67) AM_MIRROR(0x18) AM_READWRITE(led_on_r, led_on_w)
	AM_RANGE(0x80, 0xff) AM_NOP
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

#define BERZERK_COINAGE(CHUTE, DIPBANK) \
	PORT_DIPNAME( 0x0f, 0x00, "Coin "#CHUTE )  PORT_DIPLOCATION(#DIPBANK":1,2,3,4") \
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x0d, DEF_STR( 4C_3C ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_5C ) ) \
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0x0f, DEF_STR( 4C_7C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_7C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) ) \
	PORT_DIPSETTING(    0x07, "1 Coin/10 Credits" ) \
	PORT_DIPSETTING(    0x08, "1 Coin/14 Credits" ) \
	PORT_BIT( 0xf0, IP_ACTIVE_LOW,  IPT_UNUSED )


static INPUT_PORTS_START( berzerk )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START      /* IN3 */
	PORT_BIT(    0x01, 0x00, IPT_DIPSWITCH_NAME ) PORT_NAME("Input Test Mode") PORT_CODE(KEYCODE_F2) PORT_TOGGLE PORT_DIPLOCATION("F3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT(    0x02, 0x00, IPT_DIPSWITCH_NAME ) PORT_NAME("Crosshair Pattern") PORT_CODE(KEYCODE_F4) PORT_TOGGLE PORT_DIPLOCATION("F3:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("F3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Spanish ) )

	PORT_START      /* IN4 */
	PORT_BIT(    0x03, 0x00, IPT_DIPSWITCH_NAME ) PORT_NAME("Color Test") PORT_CODE(KEYCODE_F5) PORT_TOGGLE PORT_DIPLOCATION("F2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, DEF_STR( On ) )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("F2:7,8")
	PORT_DIPSETTING(    0xc0, "5000 and 10000" )
	PORT_DIPSETTING(    0x40, "5000" )
	PORT_DIPSETTING(    0x80, "10000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START      /* IN5 */
	BERZERK_COINAGE(3, F6)

	PORT_START      /* IN6 */
	BERZERK_COINAGE(2, F5)

	PORT_START      /* IN7 */
	BERZERK_COINAGE(1, F4)

	PORT_START      /* IN8 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x7e, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Stats") PORT_CODE(KEYCODE_F1)

	/* fake port for monitor type */
	PORT_START_TAG(MONITOR_TYPE_PORT_TAG)
	PORT_CONFNAME( 0x01, 0x00, "Monitor Type" )
	PORT_CONFSETTING(    0x00, "Wells-Gardner" )
	PORT_CONFSETTING(    0x01, "Electrohome" )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( frenzy )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START      /* IN3 */
	PORT_DIPNAME( 0x0f, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "1000" )
	PORT_DIPSETTING(    0x02, "2000" )
	PORT_DIPSETTING(    0x03, "3000" )
	PORT_DIPSETTING(    0x04, "4000" )
	PORT_DIPSETTING(    0x05, "5000" )
	PORT_DIPSETTING(    0x06, "6000" )
	PORT_DIPSETTING(    0x07, "7000" )
	PORT_DIPSETTING(    0x08, "8000" )
	PORT_DIPSETTING(    0x09, "9000" )
	PORT_DIPSETTING(    0x0a, "10000" )
	PORT_DIPSETTING(    0x0b, "11000" )
	PORT_DIPSETTING(    0x0c, "12000" )
	PORT_DIPSETTING(    0x0d, "13000" )
	PORT_DIPSETTING(    0x0e, "14000" )
	PORT_DIPSETTING(    0x0f, "15000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Spanish ) )

	PORT_START      /* IN4 */
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Bit 0 does some more hardware tests */
	PORT_BIT(    0x04, 0x00, IPT_DIPSWITCH_NAME ) PORT_NAME("Input Test Mode") PORT_CODE(KEYCODE_F2) PORT_TOGGLE
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT(    0x08, 0x00, IPT_DIPSWITCH_NAME ) PORT_NAME("Crosshair Pattern") PORT_CODE(KEYCODE_F4) PORT_TOGGLE
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	/* The following 3 ports use all 8 bits, but I didn't feel like adding all 256 values :-) */
	PORT_START      /* IN5 */
	PORT_DIPNAME( 0x0f, 0x01, "Coins/Credit B" )
	/*PORT_DIPSETTING(    0x00, "0" )    Can't insert coins  */
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )
	PORT_DIPSETTING(    0x0a, "10" )
	PORT_DIPSETTING(    0x0b, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0d, "13" )
	PORT_DIPSETTING(    0x0e, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH,  IPT_UNUSED )

	PORT_START      /* IN6 */
	PORT_DIPNAME( 0x0f, 0x01, "Coins/Credit A" )
	/*PORT_DIPSETTING(    0x00, "0" )    Can't insert coins  */
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )
	PORT_DIPSETTING(    0x0a, "10" )
	PORT_DIPSETTING(    0x0b, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0d, "13" )
	PORT_DIPSETTING(    0x0e, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH,  IPT_UNUSED )

	PORT_START      /* IN7 */
	PORT_DIPNAME( 0x0f, 0x01, "Coin Multiplier" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )
	PORT_DIPSETTING(    0x0a, "10" )
	PORT_DIPSETTING(    0x0b, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0d, "13" )
	PORT_DIPSETTING(    0x0e, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH,  IPT_UNUSED )

	PORT_START      /* IN8 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x7e, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Stats") PORT_CODE(KEYCODE_F1)

	/* fake port for monitor type */
	PORT_START_TAG(MONITOR_TYPE_PORT_TAG)
	PORT_CONFNAME( 0x01, 0x00, "Monitor Type" )
	PORT_CONFSETTING(    0x00, "Wells-Gardner" )
	PORT_CONFSETTING(    0x01, "Electrohome" )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( berzerk )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", Z80, MAIN_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(berzerk_map,0)
	MDRV_CPU_IO_MAP(berzerk_io_map,0)

	MDRV_MACHINE_START(berzerk)
	MDRV_MACHINE_RESET(berzerk)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_VIDEO_START(berzerk)
	MDRV_VIDEO_UPDATE(berzerk)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(S14001A, S14001A_CLOCK)	/* CPU clock divided by 16 divided by a programmable TTL setup */
	MDRV_SOUND_CONFIG(berzerk_s14001a_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(CUSTOM, 0)
	MDRV_SOUND_CONFIG(berzerk_custom_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( frenzy )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(berzerk)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(frenzy_map,0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( berzerk )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1c-0",         0x0000, 0x0800, CRC(ca566dbc) SHA1(fae2647f12f1cd82826db61b53b116a5e0c9f995) )
	ROM_LOAD( "1d-1",         0x1000, 0x0800, CRC(7ba69fde) SHA1(69af170c4a39a3494dcd180737e5c87b455f9203) )
	ROM_LOAD( "3d-2",         0x1800, 0x0800, CRC(a1d5248b) SHA1(a0b7842f6a5f86c16d80d78e7012c78b3ea11d1d) )
	ROM_LOAD( "5d-3",         0x2000, 0x0800, CRC(fcaefa95) SHA1(07f849aa39f1e3db938187ffde4a46a588156ddc) )
	ROM_LOAD( "6d-4",         0x2800, 0x0800, CRC(1e35b9a0) SHA1(5a5e549ec0e4803ab2d1eac6b3e7171aedf28244) )
	ROM_LOAD( "5c-5",         0x3000, 0x0800, CRC(c8c665e5) SHA1(e9eca4b119549e0061384abf52327c14b0d56624) )
	ROM_FILL( 0x3800, 0x0800, 0xff )

	ROM_REGION( 0x01000, REGION_SOUND1, 0 ) /* voice data */
	ROM_LOAD( "1c",           0x0000, 0x0800, CRC(2cfe825d) SHA1(f12fed8712f20fa8213f606c4049a8144bfea42e) )	/* VSU-1000 board */
	ROM_LOAD( "2c",           0x0800, 0x0800, CRC(d2b6324e) SHA1(20a6611ad6ec19409ac138bdae7bdfaeab6c47cf) )
ROM_END

ROM_START( berzerk1 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "rom0.1c",      0x0000, 0x0800, CRC(5b7eb77d) SHA1(8de488e279036fe40d6fb4c0dde16075309342fd) )
	ROM_LOAD( "rom1.1d",      0x1000, 0x0800, CRC(e58c8678) SHA1(a11f08448b457d690b270512c9f02fcf1e41d9e0) )
	ROM_LOAD( "rom2.3d",      0x1800, 0x0800, CRC(705bb339) SHA1(845191df90cd7d80f8fed3d2b69305301d921549) )
	ROM_LOAD( "rom3.5d",      0x2000, 0x0800, CRC(6a1936b4) SHA1(f1635e9d2f25514c35559d2a247c3bc4b4034c19) )
	ROM_LOAD( "rom4.6d",      0x2800, 0x0800, CRC(fa5dce40) SHA1(b3a3ee52bf65bbb3a20f905d3e4ebdf6871dcb5d) )
	ROM_LOAD( "rom5.5c",      0x3000, 0x0800, CRC(2579b9f4) SHA1(890f0237afbb194166eae88c98de81989f408548) )
	ROM_FILL( 0x3800, 0x0800, 0xff )

	ROM_REGION( 0x01000, REGION_SOUND1, 0 ) /* voice data */
	ROM_LOAD( "1c",           0x0000, 0x0800, CRC(2cfe825d) SHA1(f12fed8712f20fa8213f606c4049a8144bfea42e) )	/* VSU-1000 board */
	ROM_LOAD( "2c",           0x0800, 0x0800, CRC(d2b6324e) SHA1(20a6611ad6ec19409ac138bdae7bdfaeab6c47cf) )
ROM_END


ROM_START( frenzy )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1c-0",         0x0000, 0x1000, CRC(abdd25b8) SHA1(e6a3ab826b51b2c6ddd63d55681848fccad800dd) )
	ROM_LOAD( "1d-1",         0x1000, 0x1000, CRC(536e4ae8) SHA1(913385c43b8902d3d3ad2194a3137e19e61c6573) )
	ROM_LOAD( "3d-2",         0x2000, 0x1000, CRC(3eb9bc9b) SHA1(1e43e76ae0606a6d41d9006005d6001bdee48694) )
	ROM_LOAD( "5d-3",         0x3000, 0x1000, CRC(e1d3133c) SHA1(2af4a9bc2b29735a548ae770f872127bc009cc42) )
	ROM_LOAD( "6d-4",         0xc000, 0x1000, CRC(5581a7b1) SHA1(1f633c1c29d3b64f701c601feba26da66a6c6f23) )

	ROM_REGION( 0x01000, REGION_SOUND1, 0 ) /* voice data */
	ROM_LOAD( "1c",           0x0000, 0x0800, CRC(2cfe825d) SHA1(f12fed8712f20fa8213f606c4049a8144bfea42e) )	/* VSU-1000 board */
	ROM_LOAD( "2c",           0x0800, 0x0800, CRC(d2b6324e) SHA1(20a6611ad6ec19409ac138bdae7bdfaeab6c47cf) )        /* ditto */

	ROM_REGION( 0x0020, REGION_PROMS, ROMREGION_DISPOSE )
	ROM_LOAD( "prom.6e",      0x0000, 0x0020, CRC(4471ca5d) SHA1(ba8dca2ec076818f8ad8c17b15c77965e36fa05e) ) /* address decoder/rom select prom (N82S123N) */
ROM_END


/*
   The original / prototype version of moon war appears to run on Frenzy hardware, however the only board found
   had been stripped for parts, leaving only the sound ROMs.

   The more common version of Moon War runs on modified Super Cobra (scobra.c) hardware and is often called
   'Moon War 2' because it is the second version, and many of the PCBs are labeled as such
*/
ROM_START( moonwarp )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1c",         0x0000, 0x1000, NO_DUMP )
	/*ROM_LOAD( "3c",         0x?000, 0x?000, NO_DUMP ) */ /* likely unused */
	ROM_LOAD( "1d",         0x1000, 0x1000, NO_DUMP )
	ROM_LOAD( "3d",         0x2000, 0x1000, NO_DUMP )
	ROM_LOAD( "5d",         0x3000, 0x1000, NO_DUMP )
	ROM_LOAD( "6d",         0xc000, 0x1000, NO_DUMP )

	ROM_REGION( 0x01000, REGION_SOUND1, 0 ) /* voice data */
	ROM_LOAD( "moonwar.1c.bin",           0x0000, 0x0800, CRC(9e9a653f) SHA1(cf49a38ef343ace271ba1e5dde38bd8b9c0bd876) )	/* VSU-1000 board */
	ROM_LOAD( "moonwar.2c.bin",           0x0800, 0x0800, CRC(73fd988d) SHA1(08a2aeb4d87eee58e38e4e3f749a95f2308aceb0) )    /* ditto */

	ROM_REGION( 0x0020, REGION_PROMS, ROMREGION_DISPOSE )
	ROM_LOAD( "prom.6e",        0x0000, 0x0020, CRC(56bffba3) SHA1(c8e24f6361c50bcb4c9d3f39cdaf4172c2a2b318) ) /* address decoder/rom select prom */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980, berzerk,  0,       berzerk, berzerk, 0, ROT0, "Stern", "Berzerk (set 1)", 0 )
GAME( 1980, berzerk1, berzerk, berzerk, berzerk, 0, ROT0, "Stern", "Berzerk (set 2)", 0 )
GAME( 1981, frenzy,   0,       frenzy,  frenzy,  0, ROT0, "Stern", "Frenzy", 0 )
GAME( 1981, moonwarp, 0,       frenzy,  frenzy,  0, ROT0, "Stern", "Moon War (prototype on Frenzy hardware)", GAME_NOT_WORKING )
