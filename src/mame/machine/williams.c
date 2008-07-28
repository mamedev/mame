/***************************************************************************

    Williams 6809 system

***************************************************************************/

#include "driver.h"
#include "audio/williams.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/ticket.h"
#include "williams.h"
#include "sound/dac.h"
#include "sound/hc55516.h"


/* timers */
static emu_timer *scanline_timer;
static emu_timer *scan240_timer;
static emu_timer *scan254_timer;

/* banking addresses set by the drivers */
UINT8 *mayday_protection;

/* internal bank switching tracking */
static UINT8 blaster_bank;
static UINT8 vram_bank;

/* other stuff */
static UINT16 joust2_current_sound_data;

/* older-Williams routines */
static void williams_main_irq(running_machine *machine, int state);
static void williams_main_firq(running_machine *machine, int state);
static void williams_snd_irq(running_machine *machine, int state);
static WRITE8_HANDLER( williams_snd_cmd_w );
static WRITE8_HANDLER( playball_snd_cmd_w );

/* input port mapping */
static UINT8 port_select;
static WRITE8_HANDLER( williams_port_select_w );
static READ8_HANDLER( williams_input_port_49way_0_5_r );
static READ8_HANDLER( williams_49way_port_0_r );

/* newer-Williams routines */
static WRITE8_HANDLER( williams2_snd_cmd_w );
static void mysticm_main_irq(running_machine *machine, int state);
static void tshoot_main_irq(running_machine *machine, int state);

/* Lotto Fun-specific code */
static READ8_HANDLER( lottofun_input_port_0_r );

/* Turkey Shoot-specific code */
static READ8_HANDLER( tshoot_input_port_0_3_r );
static WRITE8_HANDLER( tshoot_lamp_w );
static WRITE8_HANDLER( tshoot_maxvol_w );

/* Joust 2-specific code */
static WRITE8_HANDLER( joust2_snd_cmd_w );
static WRITE8_HANDLER( joust2_pia_3_cb1_w );



/*************************************
 *
 *  Generic old-Williams PIA interfaces
 *
 *************************************/

/* Generic PIA 0, maps to input ports 0 and 1 */
const pia6821_interface williams_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_0_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, 0,
	/*irqs   : A/B             */ 0, 0
};

/* Generic muxing PIA 0, maps to input ports 0/3 and 1; port select is CB2 */
/* Generic dual muxing PIA 0, maps to input ports 0/3 and 1/4; port select is CB2 */
/* muxing done in williams_mux_r */
const pia6821_interface williams_muxed_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_0_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, williams_port_select_w,
	/*irqs   : A/B             */ 0, 0
};

/* Generic 49-way joystick PIA 0 for Sinistar/Blaster */
const pia6821_interface williams_49way_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ williams_49way_port_0_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, 0,
	/*irqs   : A/B             */ 0, 0
};

/* Muxing 49-way joystick PIA 0 for Blaster kit */
const pia6821_interface williams_49way_muxed_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ williams_input_port_49way_0_5_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, williams_port_select_w,
	/*irqs   : A/B             */ 0, 0
};

/* Generic PIA 1, maps to input port 2, sound command out, and IRQs */
const pia6821_interface williams_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_2_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, williams_snd_cmd_w, 0, 0,
	/*irqs   : A/B             */ williams_main_irq, williams_main_irq
};

/* Generic PIA 2, maps to DAC data in and sound IRQs */
const pia6821_interface williams_snd_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ DAC_0_data_w, 0, 0, 0,
	/*irqs   : A/B             */ williams_snd_irq, williams_snd_irq
};



/*************************************
 *
 *  Game-specific old-Williams PIA interfaces
 *
 *************************************/

/* Special PIA 0 for Lotto Fun, to handle the controls and ticket dispenser */
const pia6821_interface lottofun_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ lottofun_input_port_0_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, ticket_dispenser_w, 0, 0,
	/*irqs   : A/B             */ 0, 0
};

/* Special PIA 2 for Sinistar, to handle the CVSD */
const pia6821_interface sinistar_snd_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ DAC_0_data_w, 0, hc55516_0_digit_w, hc55516_0_clock_w,
	/*irqs   : A/B             */ williams_snd_irq, williams_snd_irq
};

/* Special PIA 1 for PlayBall, doesn't set the high bits on sound commands */
const pia6821_interface playball_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_2_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, playball_snd_cmd_w, 0, 0,
	/*irqs   : A/B             */ williams_main_irq, williams_main_irq
};

/* extra PIA 3 for Speed Ball */
const pia6821_interface spdball_pia_3_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_3_r, input_port_4_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, 0,
	/*irqs   : A/B             */ 0, 0
};



/*************************************
 *
 *  Generic later-Williams PIA interfaces
 *
 *************************************/

/* Generic muxing PIA 0, maps to input ports 0/3 and 1; port select is CA2 */
const pia6821_interface williams2_muxed_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_0_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, williams_port_select_w, 0,
	/*irqs   : A/B             */ 0, 0
};

/* Generic PIA 1, maps to input port 2, sound command out, and IRQs */
const pia6821_interface williams2_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_2_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, williams2_snd_cmd_w, 0, pia_2_ca1_w,
	/*irqs   : A/B             */ williams_main_irq, williams_main_irq
};

/* Generic PIA 2, maps to DAC data in and sound IRQs */
const pia6821_interface williams2_snd_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_1_portb_w, DAC_0_data_w, pia_1_cb1_w, 0,
	/*irqs   : A/B             */ williams_snd_irq, williams_snd_irq
};



/*************************************
 *
 *  Game-specific later-Williams PIA interfaces
 *
 *************************************/

/* Mystic Marathon PIA 0 */
const pia6821_interface mysticm_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_0_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, 0,
	/*irqs   : A/B             */ williams_main_firq, mysticm_main_irq
};

/* Mystic Marathon PIA 1 */
const pia6821_interface mysticm_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_2_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, williams2_snd_cmd_w, 0, pia_2_ca1_w,
	/*irqs   : A/B             */ mysticm_main_irq, mysticm_main_irq
};

/* Turkey Shoot PIA 0 */
const pia6821_interface tshoot_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ tshoot_input_port_0_3_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, tshoot_lamp_w, williams_port_select_w, 0,
	/*irqs   : A/B             */ tshoot_main_irq, tshoot_main_irq
};

/* Turkey Shoot PIA 1 */
const pia6821_interface tshoot_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_2_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, williams2_snd_cmd_w, 0, pia_2_ca1_w,
	/*irqs   : A/B             */ tshoot_main_irq, tshoot_main_irq
};

/* Turkey Shoot PIA 2 */
const pia6821_interface tshoot_snd_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_1_portb_w, DAC_0_data_w, pia_1_cb1_w, tshoot_maxvol_w,
	/*irqs   : A/B             */ williams_snd_irq, williams_snd_irq
};

/* Joust 2 PIA 1 */
const pia6821_interface joust2_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_2_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, joust2_snd_cmd_w, joust2_pia_3_cb1_w, pia_2_ca1_w,
	/*irqs   : A/B             */ williams_main_irq, williams_main_irq
};



/*************************************
 *
 *  Older Williams interrupts
 *
 *************************************/

static TIMER_CALLBACK( williams_va11_callback )
{
	int scanline = param;

	/* the IRQ signal comes into CB1, and is set to VA11 */
	pia_1_cb1_w(machine, 0, scanline & 0x20);

	/* set a timer for the next update */
	scanline += 0x20;
	if (scanline >= 256) scanline = 0;
	timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, scanline, 0), scanline);
}


static TIMER_CALLBACK( williams_count240_off_callback )
{
	/* the COUNT240 signal comes into CA1, and is set to the logical AND of VA10-VA13 */
	pia_1_ca1_w(machine, 0, 0);
}


static TIMER_CALLBACK( williams_count240_callback )
{
	/* the COUNT240 signal comes into CA1, and is set to the logical AND of VA10-VA13 */
	pia_1_ca1_w(machine, 0, 1);

	/* set a timer to turn it off once the scanline counter resets */
	timer_set(video_screen_get_time_until_pos(machine->primary_screen, 0, 0), NULL, 0, williams_count240_off_callback);

	/* set a timer for next frame */
	timer_adjust_oneshot(scan240_timer, video_screen_get_time_until_pos(machine->primary_screen, 240, 0), 0);
}


static void williams_main_irq(running_machine *machine, int state)
{
	int combined_state = pia_get_irq_a(1) | pia_get_irq_b(1);

	/* IRQ to the main CPU */
	cpunum_set_input_line(machine, 0, M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


static void williams_main_firq(running_machine *machine, int state)
{
	/* FIRQ to the main CPU */
	cpunum_set_input_line(machine, 0, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


static void williams_snd_irq(running_machine *machine, int state)
{
	int combined_state = pia_get_irq_a(2) | pia_get_irq_b(2);

	/* IRQ to the sound CPU */
	cpunum_set_input_line(machine, 1, M6800_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Newer Williams interrupts
 *
 *************************************/

static void mysticm_main_irq(running_machine *machine, int state)
{
	int combined_state = pia_get_irq_b(0) | pia_get_irq_a(1) | pia_get_irq_b(1);

	/* IRQ to the main CPU */
	cpunum_set_input_line(machine, 0, M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


static void tshoot_main_irq(running_machine *machine, int state)
{
	int combined_state = pia_get_irq_a(0) | pia_get_irq_b(0) | pia_get_irq_a(1) | pia_get_irq_b(1);

	/* IRQ to the main CPU */
	cpunum_set_input_line(machine, 0, M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Older Williams initialization
 *
 *************************************/

static MACHINE_RESET( williams_common )
{
	/* reset the PIAs */
	pia_reset();

	/* reset the ticket dispenser (Lotto Fun) */
	ticket_dispenser_init(70, TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_HIGH);

	/* set a timer to go off every 16 scanlines, to toggle the VA11 line and update the screen */
	scanline_timer = timer_alloc(williams_va11_callback, NULL);
	timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), 0);

	/* also set a timer to go off on scanline 240 */
	scan240_timer = timer_alloc(williams_count240_callback, NULL);
	timer_adjust_oneshot(scan240_timer, video_screen_get_time_until_pos(machine->primary_screen, 240, 0), 0);

	state_save_register_global(vram_bank);
}


MACHINE_RESET( williams )
{
	MACHINE_RESET_CALL(williams_common);

	/* configure the memory bank */
	memory_configure_bank(1, 0, 1, williams_videoram, 0);
	memory_configure_bank(1, 1, 1, memory_region(machine, "main") + 0x10000, 0);
}



/*************************************
 *
 *  Newer Williams interrupts
 *
 *************************************/

static TIMER_CALLBACK( williams2_va11_callback )
{
	int scanline = param;

	/* the IRQ signal comes into CB1, and is set to VA11 */
	pia_0_cb1_w(machine, 0, scanline & 0x20);
	pia_1_ca1_w(machine, 0, scanline & 0x20);

	/* set a timer for the next update */
	scanline += 0x20;
	if (scanline >= 256) scanline = 0;
	timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, scanline, 0), scanline);
}


static TIMER_CALLBACK( williams2_endscreen_off_callback )
{
	/* the /ENDSCREEN signal comes into CA1 */
	pia_0_ca1_w(machine, 0, 1);
}


static TIMER_CALLBACK( williams2_endscreen_callback )
{
	/* the /ENDSCREEN signal comes into CA1 */
	pia_0_ca1_w(machine, 0, 0);

	/* set a timer to turn it off once the scanline counter resets */
	timer_set(video_screen_get_time_until_pos(machine->primary_screen, 8, 0), NULL, 0, williams2_endscreen_off_callback);

	/* set a timer for next frame */
	timer_adjust_oneshot(scan254_timer, video_screen_get_time_until_pos(machine->primary_screen, 254, 0), 0);
}



/*************************************
 *
 *  Newer Williams initialization
 *
 *************************************/

static STATE_POSTLOAD( williams2_postload )
{
	williams2_bank_select_w(machine, 0, vram_bank);
}


MACHINE_RESET( williams2 )
{
	/* reset the PIAs */
	pia_reset();

	/* configure memory banks */
	memory_configure_bank(1, 0, 1, williams_videoram, 0);
	memory_configure_bank(1, 1, 4, memory_region(machine, "main") + 0x10000, 0x10000);

	/* make sure our banking is reset */
	williams2_bank_select_w(machine, 0, 0);

	/* set a timer to go off every 16 scanlines, to toggle the VA11 line and update the screen */
	scanline_timer = timer_alloc(williams2_va11_callback, NULL);
	timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), 0);

	/* also set a timer to go off on scanline 254 */
	scan254_timer = timer_alloc(williams2_endscreen_callback, NULL);
	timer_adjust_oneshot(scan254_timer, video_screen_get_time_until_pos(machine->primary_screen, 254, 0), 0);

	state_save_register_global(vram_bank);
	state_save_register_postload(machine, williams2_postload, NULL);
}



/*************************************
 *
 *  VRAM/ROM banking
 *
 *************************************/

WRITE8_HANDLER( williams_vram_select_w )
{
	/* VRAM/ROM banking from bit 0 */
	vram_bank = data & 0x01;
	memory_set_bank(1, vram_bank);

	/* cocktail flip from bit 1 */
	williams_cocktail = data & 0x02;
}


WRITE8_HANDLER( williams2_bank_select_w )
{
	vram_bank = data & 0x07;

	/* the low two bits control the paging */
	switch (vram_bank & 0x03)
	{
		/* page 0 is video ram */
		case 0:
			memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x8fff, 0, 0, SMH_BANK1);
			memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x8000, 0x87ff, 0, 0, SMH_BANK4);
			memory_set_bank(1, 0);
			memory_set_bankptr(4, &williams_videoram[0x8000]);
			break;

		/* pages 1 and 2 are ROM */
		case 1:
		case 2:
			memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x8fff, 0, 0, SMH_BANK1);
			memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x8000, 0x87ff, 0, 0, SMH_BANK4);
			memory_set_bank(1, 1 + ((vram_bank & 6) >> 1));
			memory_set_bankptr(4, &williams_videoram[0x8000]);
			break;

		/* page 3 accesses palette RAM; the remaining areas are as if page 1 ROM was selected */
		case 3:
			memory_install_readwrite8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x8000, 0x87ff, 0, 0, SMH_BANK4, williams2_paletteram_w);
			memory_set_bank(1, 1 + ((vram_bank & 4) >> 1));
			memory_set_bankptr(4, paletteram);
			break;
	}
}



/*************************************
 *
 *  Sound commands
 *
 *************************************/

static TIMER_CALLBACK( williams_deferred_snd_cmd_w )
{
	pia_2_portb_w(machine, 0, param);
	pia_2_cb1_w(machine, 0, (param == 0xff) ? 0 : 1);
}

WRITE8_HANDLER( williams_snd_cmd_w )
{
	/* the high two bits are set externally, and should be 1 */
	timer_call_after_resynch(NULL, data | 0xc0, williams_deferred_snd_cmd_w);
}

WRITE8_HANDLER( playball_snd_cmd_w )
{
	timer_call_after_resynch(NULL, data, williams_deferred_snd_cmd_w);
}


static TIMER_CALLBACK( williams2_deferred_snd_cmd_w )
{
	pia_2_porta_w(machine, 0, param);
}

static WRITE8_HANDLER( williams2_snd_cmd_w )
{
	timer_call_after_resynch(NULL, data, williams2_deferred_snd_cmd_w);
}



/*************************************
 *
 *  General input port handlers
 *
 *************************************/

WRITE8_HANDLER( williams_port_select_w )
{
	port_select = data;
}

CUSTOM_INPUT( williams_mux_r )
{
	const char *tag = param;

	if (port_select != 0)
		tag += strlen(tag) + 1;

	return input_port_read(field->port->machine, tag);
}

/*
 *  Williams 49-way joystick
 *
 *  The joystick works on a 7x7 grid system:
 *
 *      + + + | + + +
 *      + + + | + + +
 *      + + + | + + +
 *      ------+------
 *      + + + | + + +
 *      + + + | + + +
 *      + + + | + + +
 *
 *  Each axis has 7 positions, reported as follows
 *  in 4 bits/axis:
 *
 *      0000 = left/up full
 *      0100 = left/up 2/3
 *      0110 = left/up 1/3
 *      0111 = center
 *      1011 = right/down 1/3
 *      1001 = right/down 2/3
 *      1000 = right/down full
 */

READ8_HANDLER( williams_49way_port_0_r )
{
	static const UINT8 translate49[7] = { 0x0, 0x4, 0x6, 0x7, 0xb, 0x9, 0x8 };
	return (translate49[input_port_read(machine, "49WAYX") >> 4] << 4) | translate49[input_port_read(machine, "49WAYY") >> 4];
}


READ8_HANDLER( williams_input_port_49way_0_5_r )
{
	if (port_select)
		return williams_49way_port_0_r(machine,0);
	else
		return input_port_read(machine, "IN3");
}



/*************************************
 *
 *  CMOS access
 *
 *************************************/

WRITE8_HANDLER( williams_cmos_w )
{
	/* only 4 bits are valid */
	generic_nvram[offset] = data | 0xf0;
}


WRITE8_HANDLER( bubbles_cmos_w )
{
	/* bubbles has additional CMOS for a full 8 bits */
	generic_nvram[offset] = data;
}



/*************************************
 *
 *  Watchdog
 *
 *************************************/

WRITE8_HANDLER( williams_watchdog_reset_w )
{
	/* yes, the data bits are checked for this specific value */
	if (data == 0x39)
		watchdog_reset_w(machine,0,0);
}


WRITE8_HANDLER( williams2_watchdog_reset_w )
{
	/* yes, the data bits are checked for this specific value */
	if ((data & 0x3f) == 0x14)
		watchdog_reset_w(machine,0,0);
}



/*************************************
 *
 *  Diagnostic controls
 *
 *************************************/

WRITE8_HANDLER( williams2_7segment_w )
{
	int n;
	char dot;

	switch (data & 0x7F)
	{
		case 0x40:	n = 0; break;
		case 0x79:	n = 1; break;
		case 0x24:	n = 2; break;
		case 0x30:	n = 3; break;
		case 0x19:	n = 4; break;
		case 0x12:	n = 5; break;
		case 0x02:	n = 6; break;
		case 0x03:	n = 6; break;
		case 0x78:	n = 7; break;
		case 0x00:	n = 8; break;
		case 0x18:	n = 9; break;
		case 0x10:	n = 9; break;
		default:	n =-1; break;
	}

	if ((data & 0x80) == 0x00)
		dot = '.';
	else
		dot = ' ';

	if (n == -1)
		logerror("[ %c]\n", dot);
	else
		logerror("[%d%c]\n", n, dot);
}



/*************************************
 *
 *  Defender-specific routines
 *
 *************************************/

static STATE_POSTLOAD( defender_postload )
{
	defender_bank_select_w(machine, 0, vram_bank);
}


MACHINE_RESET( defender )
{
	MACHINE_RESET_CALL(williams_common);

	/* configure the banking and make sure it is reset to 0 */
	memory_configure_bank(1, 0, 9, &memory_region(machine, "main")[0x10000], 0x1000);
	defender_bank_select_w(machine, 0, 0);

	state_save_register_postload(machine, defender_postload, NULL);
}


WRITE8_HANDLER( defender_video_control_w )
{
	williams_cocktail = data & 0x01;
}


WRITE8_HANDLER( defender_bank_select_w )
{
	vram_bank = data & 0x0f;

	/* set bank address */
	switch (data)
	{
		/* page 0 is I/O space */
		case 0:
			defender_install_io_space(machine);
			break;

		/* pages 1-9 map to ROM banks */
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			memory_install_readwrite8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xcfff, 0, 0, SMH_BANK1, SMH_UNMAP);
			memory_set_bank(1, vram_bank - 1);
			break;

		/* pages A-F are not connected */
		default:
			memory_install_readwrite8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xcfff, 0, 0, SMH_NOP, SMH_NOP);
			break;
	}
}



/*************************************
 *
 *  Mayday-specific routines
 *
 *************************************/

READ8_HANDLER( mayday_protection_r )
{
	/* Mayday does some kind of protection check that is not currently understood  */
	/* However, the results of that protection check are stored at $a190 and $a191 */
	/* These are compared against $a193 and $a194, respectively. Thus, to prevent  */
	/* the protection from resetting the machine, we just return $a193 for $a190,  */
	/* and $a194 for $a191. */
	return mayday_protection[offset + 3];
}



/*************************************
 *
 *  Sinistar-specific routines
 *
 *************************************/

WRITE8_HANDLER( sinistar_vram_select_w )
{
	/* low two bits are standard */
	williams_vram_select_w(machine, offset, data);

	/* window enable from bit 2 (clips to 0x7400) */
	williams_blitter_window_enable = data & 0x04;
}



/*************************************
 *
 *  Blaster-specific routines
 *
 *************************************/

MACHINE_RESET( blaster )
{
	MACHINE_RESET_CALL(williams_common);

	/* banking is different for blaster */
	memory_configure_bank(1, 0, 1, williams_videoram, 0);
	memory_configure_bank(1, 1, 16, memory_region(machine, "main") + 0x18000, 0x4000);

	memory_configure_bank(2, 0, 1, williams_videoram + 0x4000, 0);
	memory_configure_bank(2, 1, 16, memory_region(machine, "main") + 0x10000, 0x0000);

	state_save_register_global(blaster_bank);
}


INLINE void update_blaster_banking(void)
{
	memory_set_bank(1, vram_bank * (blaster_bank + 1));
	memory_set_bank(2, vram_bank * (blaster_bank + 1));
}


WRITE8_HANDLER( blaster_vram_select_w )
{
	/* VRAM/ROM banking from bit 0 */
	vram_bank = data & 0x01;
	update_blaster_banking();

	/* cocktail flip from bit 1 */
	williams_cocktail = data & 0x02;

	/* window enable from bit 2 (clips to 0x9700) */
	williams_blitter_window_enable = data & 0x04;
}


WRITE8_HANDLER( blaster_bank_select_w )
{
	blaster_bank = data & 15;
	update_blaster_banking();
}



/*************************************
 *
 *  Lotto Fun-specific routines
 *
 *************************************/

static READ8_HANDLER( lottofun_input_port_0_r )
{
	/* merge in the ticket dispenser status */
	return input_port_read(machine, "IN0") | ticket_dispenser_r(machine,offset);
}



/*************************************
 *
 *  Turkey Shoot-specific routines
 *
 *************************************/

static READ8_HANDLER( tshoot_input_port_0_3_r )
{
	/* merge in the gun inputs with the standard data */
	int data = input_port_read(machine, "IN0");
	int gun = (data & 0x3f) ^ ((data & 0x3f) >> 1);
	return (data & 0xc0) | gun;

	return 0;
}


static WRITE8_HANDLER( tshoot_maxvol_w )
{
	/* something to do with the sound volume */
	logerror("tshoot maxvol = %d (pc:%x)\n", data, activecpu_get_pc());
}


static WRITE8_HANDLER( tshoot_lamp_w )
{
	/* set the grenade lamp */
	set_led_status(0,data & 0x04);

	/* set the gun lamp */
	set_led_status(1,data & 0x08);

#if 0
	/* gun coil */
	if (data & 0x10)
		mame_printf_debug("[gun coil] ");
	else
		mame_printf_debug("           ");

	/* feather coil */
	if (data & 0x20)
		mame_printf_debug("[feather coil] ");
	else
		mame_printf_debug("               ");

	mame_printf_debug("\n");
#endif
}



/*************************************
 *
 *  Joust 2-specific routines
 *
 *************************************/

MACHINE_START( joust2 )
{
	williams_cvsd_init(3);
}


MACHINE_RESET( joust2 )
{
	/* standard init */
	MACHINE_RESET_CALL(williams2);
	pia_set_input_ca1(3, 1);
	state_save_register_global(joust2_current_sound_data);
}


static TIMER_CALLBACK( joust2_deferred_snd_cmd_w )
{
	pia_2_porta_w(machine, 0, param & 0xff);
}


static WRITE8_HANDLER( joust2_pia_3_cb1_w )
{
	joust2_current_sound_data = (joust2_current_sound_data & ~0x100) | ((data << 8) & 0x100);
	pia_3_cb1_w(machine, offset, data);
}


static WRITE8_HANDLER( joust2_snd_cmd_w )
{
	joust2_current_sound_data = (joust2_current_sound_data & ~0xff) | (data & 0xff);
	williams_cvsd_data_w(joust2_current_sound_data);
	timer_call_after_resynch(NULL, joust2_current_sound_data, joust2_deferred_snd_cmd_w);
}
