/***************************************************************************

    Williams 6809 system

***************************************************************************/

#include "emu.h"
#include "audio/williams.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/ticket.h"
#include "includes/williams.h"
#include "sound/dac.h"
#include "sound/hc55516.h"


/* older-Williams routines */
static void williams_main_irq(device_t *device, int state);
static void williams_main_firq(device_t *device, int state);
static void williams_snd_irq(device_t *device, int state);
static void williams_snd_irq_b(device_t *device, int state);
static WRITE8_DEVICE_HANDLER( williams_snd_cmd_w );
static WRITE8_DEVICE_HANDLER( playball_snd_cmd_w );
static WRITE8_DEVICE_HANDLER( blaster_snd_cmd_w );

/* input port mapping */
static WRITE8_DEVICE_HANDLER( williams_port_select_w );
static READ8_DEVICE_HANDLER( williams_input_port_49way_0_5_r );
static READ8_DEVICE_HANDLER( williams_49way_port_0_r );

/* newer-Williams routines */
static WRITE8_DEVICE_HANDLER( williams2_snd_cmd_w );
static void mysticm_main_irq(device_t *device, int state);
static void tshoot_main_irq(device_t *device, int state);

/* Lotto Fun-specific code */
static WRITE8_DEVICE_HANDLER( lottofun_coin_lock_w );

/* Turkey Shoot-specific code */
static READ8_DEVICE_HANDLER( tshoot_input_port_0_3_r );
static WRITE8_DEVICE_HANDLER( tshoot_lamp_w );
static WRITE8_DEVICE_HANDLER( tshoot_maxvol_w );

/* Joust 2-specific code */
static WRITE8_DEVICE_HANDLER( joust2_snd_cmd_w );
static WRITE8_DEVICE_HANDLER( joust2_pia_3_cb1_w );


/*************************************
 *
 *  Generic old-Williams PIA interfaces
 *
 *************************************/

/* Generic PIA 0, maps to input ports 0 and 1 */
const pia6821_interface williams_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_NULL, DEVCB_NULL
};

/* Generic muxing PIA 0, maps to input ports 0/3 and 1; port select is CB2 */
/* Generic dual muxing PIA 0, maps to input ports 0/3 and 1/4; port select is CB2 */
/* muxing done in williams_mux_r */
const pia6821_interface williams_muxed_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_HANDLER(williams_port_select_w),
	/*irqs   : A/B             */ DEVCB_NULL, DEVCB_NULL
};

/* Generic 49-way joystick PIA 0 for Sinistar/Blaster */
const pia6821_interface williams_49way_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_HANDLER(williams_49way_port_0_r), DEVCB_INPUT_PORT("IN1"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_NULL, DEVCB_NULL
};

/* Muxing 49-way joystick PIA 0 for Blaster kit */
const pia6821_interface williams_49way_muxed_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_HANDLER(williams_input_port_49way_0_5_r), DEVCB_INPUT_PORT("IN1"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_HANDLER(williams_port_select_w),
	/*irqs   : A/B             */ DEVCB_NULL, DEVCB_NULL
};

/* Generic PIA 1, maps to input port 2, sound command out, and IRQs */
const pia6821_interface williams_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN2"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_HANDLER(williams_snd_cmd_w), DEVCB_NULL, DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_LINE(williams_main_irq), DEVCB_LINE(williams_main_irq)
};

/* Generic PIA 2, maps to DAC data in and sound IRQs */
const pia6821_interface williams_snd_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_DEVICE_HANDLER("wmsdac", dac_w), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_LINE(williams_snd_irq), DEVCB_LINE(williams_snd_irq)
};
/* Same as above, but for second sound board */
const pia6821_interface williams_snd_pia_b_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_DEVICE_HANDLER("wmsdac_b", dac_w), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_LINE(williams_snd_irq_b), DEVCB_LINE(williams_snd_irq_b)
};



/*************************************
 *
 *  Game-specific old-Williams PIA interfaces
 *
 *************************************/

/* Special PIA 0 for Lotto Fun, to handle the controls and ticket dispenser */
const pia6821_interface lottofun_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_DEVICE_HANDLER("ticket", ticket_dispenser_w), DEVCB_HANDLER(lottofun_coin_lock_w), DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_NULL, DEVCB_NULL
};

/* Special PIA 2 for Sinistar, to handle the CVSD */
const pia6821_interface sinistar_snd_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_DEVICE_HANDLER("wmsdac", dac_w), DEVCB_NULL, DEVCB_DEVICE_LINE("cvsd", hc55516_digit_w), DEVCB_DEVICE_LINE("cvsd", hc55516_clock_w),
	/*irqs   : A/B             */ DEVCB_LINE(williams_snd_irq), DEVCB_LINE(williams_snd_irq)
};

/* Special PIA 1 for PlayBall, doesn't set the high bits on sound commands */
const pia6821_interface playball_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN2"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_HANDLER(playball_snd_cmd_w), DEVCB_NULL, DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_LINE(williams_main_irq), DEVCB_LINE(williams_main_irq)
};

/* Special PIA 1 for Blaster, to support two sound boards */
const pia6821_interface blaster_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN2"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_HANDLER(blaster_snd_cmd_w), DEVCB_NULL, DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_LINE(williams_main_irq), DEVCB_LINE(williams_main_irq)
};

/* extra PIA 3 for Speed Ball */
const pia6821_interface spdball_pia_3_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN3"), DEVCB_INPUT_PORT("IN4"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_NULL, DEVCB_NULL
};



/*************************************
 *
 *  Generic later-Williams PIA interfaces
 *
 *************************************/

/* Generic muxing PIA 0, maps to input ports 0/3 and 1; port select is CA2 */
const pia6821_interface williams2_muxed_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_NULL, DEVCB_HANDLER(williams_port_select_w), DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_NULL, DEVCB_NULL
};

/* Generic PIA 1, maps to input port 2, sound command out, and IRQs */
const pia6821_interface williams2_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN2"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_HANDLER(williams2_snd_cmd_w), DEVCB_NULL, DEVCB_DEVICE_LINE_MEMBER("pia_2", pia6821_device, ca1_w),
	/*irqs   : A/B             */ DEVCB_LINE(williams_main_irq), DEVCB_LINE(williams_main_irq)
};

/* Generic PIA 2, maps to DAC data in and sound IRQs */
const pia6821_interface williams2_snd_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_DEVICE_MEMBER("pia_1", pia6821_device, portb_w), DEVCB_DEVICE_HANDLER("wmsdac", dac_w), DEVCB_DEVICE_LINE_MEMBER("pia_1", pia6821_device, cb1_w), DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_LINE(williams_snd_irq), DEVCB_LINE(williams_snd_irq)
};



/*************************************
 *
 *  Game-specific later-Williams PIA interfaces
 *
 *************************************/

/* Mystic Marathon PIA 0 */
const pia6821_interface mysticm_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_LINE(williams_main_firq), DEVCB_LINE(mysticm_main_irq)
};

/* Mystic Marathon PIA 1 */
const pia6821_interface mysticm_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN2"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_HANDLER(williams2_snd_cmd_w), DEVCB_NULL, DEVCB_DEVICE_LINE_MEMBER("pia_2", pia6821_device, ca1_w),
	/*irqs   : A/B             */ DEVCB_LINE(mysticm_main_irq), DEVCB_LINE(mysticm_main_irq)
};

/* Turkey Shoot PIA 0 */
const pia6821_interface tshoot_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_HANDLER(tshoot_input_port_0_3_r), DEVCB_INPUT_PORT("IN1"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_HANDLER(tshoot_lamp_w), DEVCB_HANDLER(williams_port_select_w), DEVCB_NULL,
	/*irqs   : A/B             */ DEVCB_LINE(tshoot_main_irq), DEVCB_LINE(tshoot_main_irq)
};

/* Turkey Shoot PIA 1 */
const pia6821_interface tshoot_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN2"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_HANDLER(williams2_snd_cmd_w), DEVCB_NULL, DEVCB_DEVICE_LINE_MEMBER("pia_2", pia6821_device, ca1_w),
	/*irqs   : A/B             */ DEVCB_LINE(tshoot_main_irq), DEVCB_LINE(tshoot_main_irq)
};

/* Turkey Shoot PIA 2 */
const pia6821_interface tshoot_snd_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_DEVICE_MEMBER("pia_1", pia6821_device, portb_w), DEVCB_DEVICE_HANDLER("wmsdac", dac_w), DEVCB_DEVICE_LINE_MEMBER("pia_1", pia6821_device, cb1_w), DEVCB_HANDLER(tshoot_maxvol_w),
	/*irqs   : A/B             */ DEVCB_LINE(williams_snd_irq), DEVCB_LINE(williams_snd_irq)
};

/* Joust 2 PIA 1 */
const pia6821_interface joust2_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ DEVCB_INPUT_PORT("IN2"), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B,CA/B2       */ DEVCB_NULL, DEVCB_HANDLER(joust2_snd_cmd_w), DEVCB_HANDLER(joust2_pia_3_cb1_w), DEVCB_DEVICE_LINE_MEMBER("pia_2", pia6821_device, ca1_w),
	/*irqs   : A/B             */ DEVCB_LINE(williams_main_irq), DEVCB_LINE(williams_main_irq)
};



/*************************************
 *
 *  Older Williams interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK( williams_va11_callback )
{
	pia6821_device *pia_1 = timer.machine().device<pia6821_device>("pia_1");
	int scanline = param;

	/* the IRQ signal comes into CB1, and is set to VA11 */
	pia_1->cb1_w(scanline & 0x20);

	/* set a timer for the next update */
	scanline += 0x20;
	if (scanline >= 256) scanline = 0;
	timer.adjust(timer.machine().primary_screen->time_until_pos(scanline), scanline);
}


static TIMER_CALLBACK( williams_count240_off_callback )
{
	pia6821_device *pia_1 = machine.device<pia6821_device>("pia_1");

	/* the COUNT240 signal comes into CA1, and is set to the logical AND of VA10-VA13 */
	pia_1->ca1_w(0);
}


TIMER_DEVICE_CALLBACK( williams_count240_callback )
{
	pia6821_device *pia_1 = timer.machine().device<pia6821_device>("pia_1");

	/* the COUNT240 signal comes into CA1, and is set to the logical AND of VA10-VA13 */
	pia_1->ca1_w(1);

	/* set a timer to turn it off once the scanline counter resets */
	timer.machine().scheduler().timer_set(timer.machine().primary_screen->time_until_pos(0), FUNC(williams_count240_off_callback));

	/* set a timer for next frame */
	timer.adjust(timer.machine().primary_screen->time_until_pos(240));
}


static void williams_main_irq(device_t *device, int state)
{
	pia6821_device *pia_1 = device->machine().device<pia6821_device>("pia_1");
	int combined_state = pia_1->irq_a_state() | pia_1->irq_b_state();

	/* IRQ to the main CPU */
	cputag_set_input_line(device->machine(), "maincpu", M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


static void williams_main_firq(device_t *device, int state)
{
	/* FIRQ to the main CPU */
	cputag_set_input_line(device->machine(), "maincpu", M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


static void williams_snd_irq(device_t *device, int state)
{
	pia6821_device *pia_2 = device->machine().device<pia6821_device>("pia_2");
	int combined_state = pia_2->irq_a_state() | pia_2->irq_b_state();

	/* IRQ to the sound CPU */
	cputag_set_input_line(device->machine(), "soundcpu", M6800_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}
/* Same as above, but for second sound board */
static void williams_snd_irq_b(device_t *device, int state)
{
	pia6821_device *pia_2 = device->machine().device<pia6821_device>("pia_2b");
	int combined_state = pia_2->irq_a_state() | pia_2->irq_b_state();

	/* IRQ to the sound CPU */
	cputag_set_input_line(device->machine(), "soundcpu_b", M6800_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Newer Williams interrupts
 *
 *************************************/

static void mysticm_main_irq(device_t *device, int state)
{
	pia6821_device *pia_0 = device->machine().device<pia6821_device>("pia_0");
	pia6821_device *pia_1 = device->machine().device<pia6821_device>("pia_1");
	int combined_state = pia_0->irq_b_state() | pia_1->irq_a_state() | pia_1->irq_b_state();

	/* IRQ to the main CPU */
	cputag_set_input_line(device->machine(), "maincpu", M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


static void tshoot_main_irq(device_t *device, int state)
{
	pia6821_device *pia_0 = device->machine().device<pia6821_device>("pia_0");
	pia6821_device *pia_1 = device->machine().device<pia6821_device>("pia_1");
	int combined_state = pia_0->irq_a_state() | pia_0->irq_b_state() | pia_1->irq_a_state() | pia_1->irq_b_state();

	/* IRQ to the main CPU */
	cputag_set_input_line(device->machine(), "maincpu", M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Older Williams initialization
 *
 *************************************/

static MACHINE_START( williams_common )
{
	williams_state *state = machine.driver_data<williams_state>();
	/* configure the memory bank */
	memory_configure_bank(machine, "bank1", 0, 1, state->m_videoram, 0);
	memory_configure_bank(machine, "bank1", 1, 1, machine.region("maincpu")->base() + 0x10000, 0);

	state_save_register_global(machine, state->m_vram_bank);
}


static MACHINE_RESET( williams_common )
{
	/* set a timer to go off every 16 scanlines, to toggle the VA11 line and update the screen */
	timer_device *scan_timer = machine.device<timer_device>("scan_timer");
	scan_timer->adjust(machine.primary_screen->time_until_pos(0));

	/* also set a timer to go off on scanline 240 */
	timer_device *l240_timer = machine.device<timer_device>("240_timer");
	l240_timer->adjust(machine.primary_screen->time_until_pos(240));
}


MACHINE_START( williams )
{
	MACHINE_START_CALL(williams_common);
}


MACHINE_RESET( williams )
{
	MACHINE_RESET_CALL(williams_common);
}



/*************************************
 *
 *  Newer Williams interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK( williams2_va11_callback )
{
	pia6821_device *pia_0 = timer.machine().device<pia6821_device>("pia_0");
	pia6821_device *pia_1 = timer.machine().device<pia6821_device>("pia_1");
	int scanline = param;

	/* the IRQ signal comes into CB1, and is set to VA11 */
	pia_0->cb1_w(scanline & 0x20);
	pia_1->ca1_w(scanline & 0x20);

	/* set a timer for the next update */
	scanline += 0x20;
	if (scanline >= 256) scanline = 0;
	timer.adjust(timer.machine().primary_screen->time_until_pos(scanline), scanline);
}


static TIMER_CALLBACK( williams2_endscreen_off_callback )
{
	pia6821_device *pia_0 = machine.device<pia6821_device>("pia_0");

	/* the /ENDSCREEN signal comes into CA1 */
	pia_0->ca1_w(1);
}


TIMER_DEVICE_CALLBACK( williams2_endscreen_callback )
{
	pia6821_device *pia_0 = timer.machine().device<pia6821_device>("pia_0");

	/* the /ENDSCREEN signal comes into CA1 */
	pia_0->ca1_w(0);

	/* set a timer to turn it off once the scanline counter resets */
	timer.machine().scheduler().timer_set(timer.machine().primary_screen->time_until_pos(8), FUNC(williams2_endscreen_off_callback));

	/* set a timer for next frame */
	timer.adjust(timer.machine().primary_screen->time_until_pos(254));
}



/*************************************
 *
 *  Newer Williams initialization
 *
 *************************************/

static void williams2_postload(running_machine &machine)
{
	williams_state *state = machine.driver_data<williams_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	state->williams2_bank_select_w(*space, 0, state->m_vram_bank);
}


MACHINE_START( williams2 )
{
	williams_state *state = machine.driver_data<williams_state>();
	/* configure memory banks */
	memory_configure_bank(machine, "bank1", 0, 1, state->m_videoram, 0);
	memory_configure_bank(machine, "bank1", 1, 4, machine.region("maincpu")->base() + 0x10000, 0x10000);

	/* register for save states */
	state_save_register_global(machine, state->m_vram_bank);
	machine.save().register_postload(save_prepost_delegate(FUNC(williams2_postload), &machine));
}


MACHINE_RESET( williams2 )
{
	williams_state *state = machine.driver_data<williams_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	/* make sure our banking is reset */
	state->williams2_bank_select_w(*space, 0, 0);

	/* set a timer to go off every 16 scanlines, to toggle the VA11 line and update the screen */
	timer_device *scan_timer = machine.device<timer_device>("scan_timer");
	scan_timer->adjust(machine.primary_screen->time_until_pos(0));

	/* also set a timer to go off on scanline 254 */
	timer_device *l254_timer = machine.device<timer_device>("254_timer");
	l254_timer->adjust(machine.primary_screen->time_until_pos(254));
}



/*************************************
 *
 *  VRAM/ROM banking
 *
 *************************************/

WRITE8_MEMBER(williams_state::williams_vram_select_w)
{
	/* VRAM/ROM banking from bit 0 */
	m_vram_bank = data & 0x01;
	memory_set_bank(machine(), "bank1", m_vram_bank);

	/* cocktail flip from bit 1 */
	m_cocktail = data & 0x02;
}


WRITE8_MEMBER(williams_state::williams2_bank_select_w)
{
	m_vram_bank = data & 0x07;

	/* the low two bits control the paging */
	switch (m_vram_bank & 0x03)
	{
		/* page 0 is video ram */
		case 0:
			space.install_read_bank(0x0000, 0x8fff, "bank1");
			space.install_write_bank(0x8000, 0x87ff, "bank4");
			memory_set_bank(machine(), "bank1", 0);
			memory_set_bankptr(machine(), "bank4", &m_videoram[0x8000]);
			break;

		/* pages 1 and 2 are ROM */
		case 1:
		case 2:
			space.install_read_bank(0x0000, 0x8fff, "bank1");
			space.install_write_bank(0x8000, 0x87ff, "bank4");
			memory_set_bank(machine(), "bank1", 1 + ((m_vram_bank & 6) >> 1));
			memory_set_bankptr(machine(), "bank4", &m_videoram[0x8000]);
			break;

		/* page 3 accesses palette RAM; the remaining areas are as if page 1 ROM was selected */
		case 3:
			space.install_read_bank(0x8000, 0x87ff, "bank4");
			space.install_write_handler(0x8000, 0x87ff, write8_delegate(FUNC(williams_state::williams2_paletteram_w),this));
			memory_set_bank(machine(), "bank1", 1 + ((m_vram_bank & 4) >> 1));
			memory_set_bankptr(machine(), "bank4", m_generic_paletteram_8);
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
	pia6821_device *pia_2 = machine.device<pia6821_device>("pia_2");

	pia_2->portb_w(param);
	pia_2->cb1_w((param == 0xff) ? 0 : 1);
}

WRITE8_DEVICE_HANDLER( williams_snd_cmd_w )
{
	/* the high two bits are set externally, and should be 1 */
	device->machine().scheduler().synchronize(FUNC(williams_deferred_snd_cmd_w), data | 0xc0);
}

WRITE8_DEVICE_HANDLER( playball_snd_cmd_w )
{
	device->machine().scheduler().synchronize(FUNC(williams_deferred_snd_cmd_w), data);
}

static TIMER_CALLBACK( blaster_deferred_snd_cmd_w )
{
	pia6821_device *pia_2l = machine.device<pia6821_device>("pia_2");
	pia6821_device *pia_2r = machine.device<pia6821_device>("pia_2b");
	UINT8 l_data = param | 0x80;
	UINT8 r_data = (param >> 1 & 0x40) | (param & 0x3f) | 0x80;

	pia_2l->portb_w(l_data); pia_2l->cb1_w((l_data == 0xff) ? 0 : 1);
	pia_2r->portb_w(r_data); pia_2r->cb1_w((r_data == 0xff) ? 0 : 1);
}

WRITE8_DEVICE_HANDLER( blaster_snd_cmd_w )
{
	device->machine().scheduler().synchronize(FUNC(blaster_deferred_snd_cmd_w), data);
}


static TIMER_CALLBACK( williams2_deferred_snd_cmd_w )
{
	pia6821_device *pia_2 = machine.device<pia6821_device>("pia_2");

	pia_2->porta_w(param);
}

static WRITE8_DEVICE_HANDLER( williams2_snd_cmd_w )
{
	device->machine().scheduler().synchronize(FUNC(williams2_deferred_snd_cmd_w), data);
}



/*************************************
 *
 *  General input port handlers
 *
 *************************************/

WRITE8_DEVICE_HANDLER( williams_port_select_w )
{
	williams_state *state = device->machine().driver_data<williams_state>();
	state->m_port_select = data;
}

CUSTOM_INPUT( williams_mux_r )
{
	williams_state *state = field.machine().driver_data<williams_state>();
	const char *tag = (const char *)param;

	if (state->m_port_select != 0)
		tag += strlen(tag) + 1;

	return input_port_read(field.machine(), tag);
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

READ8_DEVICE_HANDLER( williams_49way_port_0_r )
{
	static const UINT8 translate49[7] = { 0x0, 0x4, 0x6, 0x7, 0xb, 0x9, 0x8 };
	return (translate49[input_port_read(device->machine(), "49WAYX") >> 4] << 4) | translate49[input_port_read(device->machine(), "49WAYY") >> 4];
}


READ8_DEVICE_HANDLER( williams_input_port_49way_0_5_r )
{
	williams_state *state = device->machine().driver_data<williams_state>();
	if (state->m_port_select)
		return williams_49way_port_0_r(device, 0);
	else
		return input_port_read(device->machine(), "IN3");
}



/*************************************
 *
 *  CMOS access
 *
 *************************************/

WRITE8_MEMBER(williams_state::williams_cmos_w)
{
	/* only 4 bits are valid */
	m_nvram[offset] = data | 0xf0;
}


WRITE8_MEMBER(williams_state::bubbles_cmos_w)
{
	/* bubbles has additional CMOS for a full 8 bits */
	m_nvram[offset] = data;
}



/*************************************
 *
 *  Watchdog
 *
 *************************************/

WRITE8_MEMBER(williams_state::williams_watchdog_reset_w)
{
	/* yes, the data bits are checked for this specific value */
	if (data == 0x39)
		watchdog_reset_w(space,0,0);
}


WRITE8_MEMBER(williams_state::williams2_watchdog_reset_w)
{
	/* yes, the data bits are checked for this specific value */
	if ((data & 0x3f) == 0x14)
		watchdog_reset_w(space,0,0);
}



/*************************************
 *
 *  Diagnostic controls
 *
 *************************************/

WRITE8_MEMBER(williams_state::williams2_7segment_w)
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

static void defender_postload(running_machine &machine)
{
	williams_state *state = machine.driver_data<williams_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	state->defender_bank_select_w(*space, 0, state->m_vram_bank);
}


MACHINE_START( defender )
{
	MACHINE_START_CALL(williams_common);

	/* configure the banking and make sure it is reset to 0 */
	memory_configure_bank(machine, "bank1", 0, 9, &machine.region("maincpu")->base()[0x10000], 0x1000);

	machine.save().register_postload(save_prepost_delegate(FUNC(defender_postload), &machine));
}


MACHINE_RESET( defender )
{
	williams_state *state = machine.driver_data<williams_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	MACHINE_RESET_CALL(williams_common);

	state->defender_bank_select_w(*space, 0, 0);
}


WRITE8_MEMBER(williams_state::defender_video_control_w)
{
	m_cocktail = data & 0x01;
}


WRITE8_MEMBER(williams_state::defender_bank_select_w)
{
	m_vram_bank = data & 0x0f;

	/* set bank address */
	switch (data)
	{
		/* page 0 is I/O &space */
		case 0:
			defender_install_io_space(&space);
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
			space.install_read_bank(0xc000, 0xcfff, "bank1");
			space.unmap_write(0xc000, 0xcfff);
			memory_set_bank(machine(), "bank1", m_vram_bank - 1);
			break;

		/* pages A-F are not connected */
		default:
			space.nop_readwrite(0xc000, 0xcfff);
			break;
	}
}



/*************************************
 *
 *  Mayday-specific routines
 *
 *************************************/

READ8_MEMBER(williams_state::mayday_protection_r)
{
	/* Mayday does some kind of protection check that is not currently understood  */
	/* However, the results of that protection check are stored at $a190 and $a191 */
	/* These are compared against $a193 and $a194, respectively. Thus, to prevent  */
	/* the protection from resetting the machine(), we just return $a193 for $a190,  */
	/* and $a194 for $a191. */
	return m_mayday_protection[offset + 3];
}



/*************************************
 *
 *  Sinistar-specific routines
 *
 *************************************/

WRITE8_MEMBER(williams_state::sinistar_vram_select_w)
{
	/* low two bits are standard */
	williams_vram_select_w(space, offset, data);

	/* window enable from bit 2 (clips to 0x7400) */
	m_blitter_window_enable = data & 0x04;
}



/*************************************
 *
 *  Blaster-specific routines
 *
 *************************************/

MACHINE_START( blaster )
{
	williams_state *state = machine.driver_data<williams_state>();
	MACHINE_START_CALL(williams_common);

	/* banking is different for blaster */
	memory_configure_bank(machine, "bank1", 0, 1, state->m_videoram, 0);
	memory_configure_bank(machine, "bank1", 1, 16, machine.region("maincpu")->base() + 0x18000, 0x4000);

	memory_configure_bank(machine, "bank2", 0, 1, state->m_videoram + 0x4000, 0);
	memory_configure_bank(machine, "bank2", 1, 16, machine.region("maincpu")->base() + 0x10000, 0x0000);

	state_save_register_global(machine, state->m_blaster_bank);
}


MACHINE_RESET( blaster )
{
	MACHINE_RESET_CALL(williams_common);
}


INLINE void update_blaster_banking(running_machine &machine)
{
	williams_state *state = machine.driver_data<williams_state>();
	memory_set_bank(machine, "bank1", state->m_vram_bank * (state->m_blaster_bank + 1));
	memory_set_bank(machine, "bank2", state->m_vram_bank * (state->m_blaster_bank + 1));
}


WRITE8_MEMBER(williams_state::blaster_vram_select_w)
{
	/* VRAM/ROM banking from bit 0 */
	m_vram_bank = data & 0x01;
	update_blaster_banking(machine());

	/* cocktail flip from bit 1 */
	m_cocktail = data & 0x02;

	/* window enable from bit 2 (clips to 0x9700) */
	m_blitter_window_enable = data & 0x04;
}


WRITE8_MEMBER(williams_state::blaster_bank_select_w)
{
	m_blaster_bank = data & 15;
	update_blaster_banking(machine());
}



/*************************************
 *
 *  Lotto Fun-specific routines
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( lottofun_coin_lock_w )
{
	coin_lockout_global_w(device->machine(), data & 1); /* bit 5 of PIC control port A */
}



/*************************************
 *
 *  Turkey Shoot-specific routines
 *
 *************************************/

static READ8_DEVICE_HANDLER( tshoot_input_port_0_3_r )
{
	/* merge in the gun inputs with the standard data */
	int data = input_port_read(device->machine(), "IN0");
	int gun = (data & 0x3f) ^ ((data & 0x3f) >> 1);
	return (data & 0xc0) | gun;

	// FIXME: this code is never reached
	return 0;
}


static WRITE8_DEVICE_HANDLER( tshoot_maxvol_w )
{
	/* something to do with the sound volume */
	logerror("tshoot maxvol = %d (%s)\n", data, device->machine().describe_context());
}


static WRITE8_DEVICE_HANDLER( tshoot_lamp_w )
{
	/* set the grenade lamp */
	output_set_value("Grenade_lamp", (~data & 0x4)>>2 );
	/* set the gun lamp */
	output_set_value("Gun_lamp", (~data & 0x8)>>3 );
	/* gun coil */
	output_set_value("Player1_Gun_Recoil", (data & 0x10)>>4 );
	/* feather coil */
	output_set_value("Feather_Blower", (data & 0x20)>>5 );

}



/*************************************
 *
 *  Joust 2-specific routines
 *
 *************************************/

MACHINE_START( joust2 )
{
	williams_state *state = machine.driver_data<williams_state>();
	MACHINE_START_CALL(williams2);
	williams_cvsd_init(machine);
	state_save_register_global(machine, state->m_joust2_current_sound_data);
}


MACHINE_RESET( joust2 )
{
	pia6821_device *pia_3 = machine.device<pia6821_device>("cvsdpia");

	/* standard init */
	MACHINE_RESET_CALL(williams2);
	pia_3->ca1_w(1);
}


static TIMER_CALLBACK( joust2_deferred_snd_cmd_w )
{
	pia6821_device *pia_2 = machine.device<pia6821_device>("pia_2");
	pia_2->porta_w(param & 0xff);
}


static WRITE8_DEVICE_HANDLER( joust2_pia_3_cb1_w )
{
	williams_state *state = device->machine().driver_data<williams_state>();
	pia6821_device *pia_3 = device->machine().device<pia6821_device>("cvsdpia");

	state->m_joust2_current_sound_data = (state->m_joust2_current_sound_data & ~0x100) | ((data << 8) & 0x100);
	pia_3->cb1_w(data);
}


static WRITE8_DEVICE_HANDLER( joust2_snd_cmd_w )
{
	williams_state *state = device->machine().driver_data<williams_state>();
	state->m_joust2_current_sound_data = (state->m_joust2_current_sound_data & ~0xff) | (data & 0xff);
	williams_cvsd_data_w(device->machine(), state->m_joust2_current_sound_data);
	device->machine().scheduler().synchronize(FUNC(joust2_deferred_snd_cmd_w), state->m_joust2_current_sound_data);
}
