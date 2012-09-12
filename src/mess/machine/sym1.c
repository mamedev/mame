/******************************************************************************
 Synertek Systems Corp. SYM-1

 Early driver by PeT mess@utanet.at May 2000
 Rewritten by Dirk Best October 2007

******************************************************************************/


#include "emu.h"
#include "includes/sym1.h"

/* M6502 CPU */
#include "cpu/m6502/m6502.h"

/* Peripheral chips */
#include "machine/6522via.h"
#include "machine/6532riot.h"
#include "machine/74145.h"
#include "sound/speaker.h"
#include "machine/ram.h"

#define LED_REFRESH_DELAY  attotime::from_usec(70)





/******************************************************************************
 6532 RIOT
******************************************************************************/


static WRITE_LINE_DEVICE_HANDLER( sym1_74145_output_0_w )
{
	sym1_state *drvstate = device->machine().driver_data<sym1_state>();
	if (state) drvstate->m_led_update->adjust(LED_REFRESH_DELAY);
}


static WRITE_LINE_DEVICE_HANDLER( sym1_74145_output_1_w )
{
	sym1_state *drvstate = device->machine().driver_data<sym1_state>();
	if (state) drvstate->m_led_update->adjust(LED_REFRESH_DELAY, 1);
}


static WRITE_LINE_DEVICE_HANDLER( sym1_74145_output_2_w )
{
	sym1_state *drvstate = device->machine().driver_data<sym1_state>();
	if (state) drvstate->m_led_update->adjust(LED_REFRESH_DELAY, 2);
}


static WRITE_LINE_DEVICE_HANDLER( sym1_74145_output_3_w )
{
	sym1_state *drvstate = device->machine().driver_data<sym1_state>();
	if (state) drvstate->m_led_update->adjust(LED_REFRESH_DELAY, 3);
}


static WRITE_LINE_DEVICE_HANDLER( sym1_74145_output_4_w )
{
	sym1_state *drvstate = device->machine().driver_data<sym1_state>();
	if (state) drvstate->m_led_update->adjust(LED_REFRESH_DELAY, 4);
}


static WRITE_LINE_DEVICE_HANDLER( sym1_74145_output_5_w )
{
	sym1_state *drvstate = device->machine().driver_data<sym1_state>();
	if (state) drvstate->m_led_update->adjust(LED_REFRESH_DELAY, 5);
}


static TIMER_CALLBACK( led_refresh )
{
	sym1_state *state = machine.driver_data<sym1_state>();
	output_set_digit_value(param, state->m_riot_port_a);
}


static READ8_DEVICE_HANDLER(sym1_riot_a_r)
{
	sym1_state *state = device->machine().driver_data<sym1_state>();
	int data = 0x7f;

	/* scan keypad rows */
	if (!(state->m_riot_port_a & 0x80)) data &= device->machine().root_device().ioport("ROW-0")->read();
	if (!(state->m_riot_port_b & 0x01)) data &= device->machine().root_device().ioport("ROW-1")->read();
	if (!(state->m_riot_port_b & 0x02)) data &= device->machine().root_device().ioport("ROW-2")->read();
	if (!(state->m_riot_port_b & 0x04)) data &= device->machine().root_device().ioport("ROW-3")->read();

	/* determine column */
	if ( ((state->m_riot_port_a ^ 0xff) & (state->ioport("ROW-0")->read() ^ 0xff)) & 0x7f )
		data &= ~0x80;

	return data;
}


static READ8_DEVICE_HANDLER(sym1_riot_b_r)
{
	sym1_state *state = device->machine().driver_data<sym1_state>();
	int data = 0xff;

	/* determine column */
	if ( ((state->m_riot_port_a ^ 0xff) & (device->machine().root_device().ioport("ROW-1")->read() ^ 0xff)) & 0x7f )
		data &= ~0x01;

	if ( ((state->m_riot_port_a ^ 0xff) & (device->machine().root_device().ioport("ROW-2")->read() ^ 0xff)) & 0x3f )
		data &= ~0x02;

	if ( ((state->m_riot_port_a ^ 0xff) & (state->ioport("ROW-3")->read() ^ 0xff)) & 0x1f )
		data &= ~0x04;

	data &= ~0x80; // else hangs 8b02

	return data;
}


static WRITE8_DEVICE_HANDLER(sym1_riot_a_w)
{
	sym1_state *state = device->machine().driver_data<sym1_state>();
	logerror("%x: riot_a_w 0x%02x\n", device->machine().device("maincpu") ->safe_pc( ), data);

	/* save for later use */
	state->m_riot_port_a = data;
}


static WRITE8_DEVICE_HANDLER(sym1_riot_b_w)
{
	sym1_state *state = device->machine().driver_data<sym1_state>();
	logerror("%x: riot_b_w 0x%02x\n", device->machine().device("maincpu") ->safe_pc( ), data);

	/* save for later use */
	state->m_riot_port_b = data;

	/* first 4 pins are connected to the 74145 */
	device->machine().device<ttl74145_device>("ttl74145")->write(data & 0x0f);
}


const riot6532_interface sym1_r6532_interface =
{
	DEVCB_HANDLER(sym1_riot_a_r),
	DEVCB_HANDLER(sym1_riot_b_r),
	DEVCB_HANDLER(sym1_riot_a_w),
	DEVCB_HANDLER(sym1_riot_b_w)
};


const ttl74145_interface sym1_ttl74145_intf =
{
	DEVCB_LINE(sym1_74145_output_0_w),  /* connected to DS0 */
	DEVCB_LINE(sym1_74145_output_1_w),  /* connected to DS1 */
	DEVCB_LINE(sym1_74145_output_2_w),  /* connected to DS2 */
	DEVCB_LINE(sym1_74145_output_3_w),  /* connected to DS3 */
	DEVCB_LINE(sym1_74145_output_4_w),  /* connected to DS4 */
	DEVCB_LINE(sym1_74145_output_5_w),  /* connected to DS5 */
	DEVCB_DEVICE_LINE(SPEAKER_TAG, speaker_level_w),
	DEVCB_NULL,	/* not connected */
	DEVCB_NULL,	/* not connected */
	DEVCB_NULL	/* not connected */
};


/******************************************************************************
 6522 VIA
******************************************************************************/


static void sym1_irq(device_t *device, int level)
{
	device->machine().device("maincpu")->execute().set_input_line(M6502_IRQ_LINE, level);
}


static READ8_DEVICE_HANDLER( sym1_via0_b_r )
{
	return 0xff;
}


static WRITE8_DEVICE_HANDLER( sym1_via0_b_w )
{
	logerror("%s: via0_b_w 0x%02x\n", device->machine().describe_context(), data);
}


/* PA0: Write protect R6532 RAM
 * PA1: Write protect RAM 0x400-0x7ff
 * PA2: Write protect RAM 0x800-0xbff
 * PA3: Write protect RAM 0xc00-0xfff
 */
static WRITE8_DEVICE_HANDLER( sym1_via2_a_w )
{
	address_space *cpu0space = device->machine().device( "maincpu")->memory().space( AS_PROGRAM );

	logerror("SYM1 VIA2 W 0x%02x\n", data);

	if ((device->machine().root_device().ioport("WP")->read() & 0x01) && !(data & 0x01)) {
		cpu0space->nop_write(0xa600, 0xa67f);
	} else {
		cpu0space->install_write_bank(0xa600, 0xa67f, "bank5");
	}
	if ((device->machine().root_device().ioport("WP")->read() & 0x02) && !(data & 0x02)) {
		cpu0space->nop_write(0x0400, 0x07ff);
	} else {
		cpu0space->install_write_bank(0x0400, 0x07ff, "bank2");
	}
	if ((device->machine().root_device().ioport("WP")->read() & 0x04) && !(data & 0x04)) {
		cpu0space->nop_write(0x0800, 0x0bff);
	} else {
		cpu0space->install_write_bank(0x0800, 0x0bff, "bank3");
	}
	if ((device->machine().root_device().ioport("WP")->read() & 0x08) && !(data & 0x08)) {
		cpu0space->nop_write(0x0c00, 0x0fff);
	} else {
		cpu0space->install_write_bank(0x0c00, 0x0fff, "bank4");
	}
}


const via6522_interface sym1_via0 =
{
	DEVCB_NULL,           /* VIA Port A Input */
	DEVCB_HANDLER(sym1_via0_b_r),  /* VIA Port B Input */
	DEVCB_NULL,           /* VIA Port CA1 Input */
	DEVCB_NULL,           /* VIA Port CB1 Input */
	DEVCB_NULL,           /* VIA Port CA2 Input */
	DEVCB_NULL,           /* VIA Port CB2 Input */
	DEVCB_NULL,           /* VIA Port A Output */
	DEVCB_HANDLER(sym1_via0_b_w),  /* VIA Port B Output */
	DEVCB_NULL,           /* VIA Port CA1 Output */
	DEVCB_NULL,           /* VIA Port CB1 Output */
	DEVCB_NULL,           /* VIA Port CA2 Output */
	DEVCB_NULL,           /* VIA Port CB2 Output */
	DEVCB_LINE(sym1_irq)        /* VIA IRQ Callback */
};


const via6522_interface sym1_via1 =
{
	DEVCB_NULL,           /* VIA Port A Input */
	DEVCB_NULL,           /* VIA Port B Input */
	DEVCB_NULL,           /* VIA Port CA1 Input */
	DEVCB_NULL,           /* VIA Port CB1 Input */
	DEVCB_NULL,           /* VIA Port CA2 Input */
	DEVCB_NULL,           /* VIA Port CB2 Input */
	DEVCB_NULL,           /* VIA Port A Output */
	DEVCB_NULL,           /* VIA Port B Output */
	DEVCB_NULL,           /* VIA Port CA1 Output */
	DEVCB_NULL,           /* VIA Port CB1 Output */
	DEVCB_NULL,           /* VIA Port CA2 Output */
	DEVCB_NULL,           /* VIA Port CB2 Output */
	DEVCB_LINE(sym1_irq)        /* VIA IRQ Callback */
};


const via6522_interface sym1_via2 =
{
	DEVCB_NULL,           /* VIA Port A Input */
	DEVCB_NULL,           /* VIA Port B Input */
	DEVCB_NULL,           /* VIA Port CA1 Input */
	DEVCB_NULL,           /* VIA Port CB1 Input */
	DEVCB_NULL,           /* VIA Port CA2 Input */
	DEVCB_NULL,           /* VIA Port CB2 Input */
	DEVCB_HANDLER(sym1_via2_a_w),  /* VIA Port A Output */
	DEVCB_NULL,           /* VIA Port B Output */
	DEVCB_NULL,           /* VIA Port CA1 Output */
	DEVCB_NULL,           /* VIA Port CB1 Output */
	DEVCB_NULL,           /* VIA Port CA2 Output */
	DEVCB_NULL,           /* VIA Port CB2 Output */
	DEVCB_LINE(sym1_irq)        /* VIA IRQ Callback */
};



/******************************************************************************
 Driver init and reset
******************************************************************************/


DRIVER_INIT_MEMBER(sym1_state,sym1)
{
	/* wipe expansion memory banks that are not installed */
	if (machine().device<ram_device>(RAM_TAG)->size() < 4*1024)
	{
		machine().device( "maincpu")->memory().space( AS_PROGRAM )->nop_readwrite(
			machine().device<ram_device>(RAM_TAG)->size(), 0x0fff);
	}

	/* allocate a timer to refresh the led display */
	m_led_update = machine().scheduler().timer_alloc(FUNC(led_refresh));
}


MACHINE_RESET( sym1 )
{
	sym1_state *state = machine.driver_data<sym1_state>();
	/* make 0xf800 to 0xffff point to the last half of the monitor ROM
       so that the CPU can find its reset vectors */
	machine.device( "maincpu")->memory().space( AS_PROGRAM )->install_read_bank(0xf800, 0xffff, "bank1");
	machine.device( "maincpu")->memory().space( AS_PROGRAM )->nop_write(0xf800, 0xffff);
	state->membank("bank1")->set_base(state->m_monitor + 0x800);
	machine.device("maincpu")->reset();
}
