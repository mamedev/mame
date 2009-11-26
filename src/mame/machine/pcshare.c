/***************************************************************************

    machine/pcshare.c

    Functions to emulate general aspects of the machine
    (RAM, ROM, interrupts, I/O ports)

    The information herein is heavily based on
    'Ralph Browns Interrupt List'
    Release 52, Last Change 20oct96

    TODO:
    clean up (maybe split) the different pieces of hardware
    PIC, PIT, DMA... add support for LPT, COM (almost done)
    emulation of a serial mouse on a COM port (almost done)
    support for Game Controller port at 0x0201
    support for XT harddisk (under the way, see machine/pc_hdc.c)
    whatever 'hardware' comes to mind,
    maybe SoundBlaster? EGA? VGA?

***************************************************************************/

#include "driver.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"

#define VERBOSE_DBG 0       /* general debug messages */
#define DBG_LOG(N,M,A) \
	if(VERBOSE_DBG>=N){ if( M )logerror("%11.6f: %-24s",attotime_to_double(timer_get_time(pc_keyb.machine)),(char*)M ); logerror A; }

#define VERBOSE_JOY 0		/* JOY (joystick port) */
#define JOY_LOG(N,M,A) \
	if(VERBOSE_JOY>=N){ if( M )logerror("%11.6f: %-24s",attotime_to_double(timer_get_time(pc_keyb.machine)),(char*)M ); logerror A; }


static TIMER_CALLBACK( pc_keyb_timer );

/*
   keyboard seams to permanently sent data clocked by the mainboard
   clock line low for longer means "resync", keyboard sends 0xaa as answer
   will become automatically 0x00 after a while
*/

static struct {
	running_machine *machine;
	void (*int_cb)(running_machine *, int);
	emu_timer *timer;
	UINT8 data;
	int on;
	int self_test;
} pc_keyb;



void init_pc_common(running_machine *machine, UINT32 flags, void (*set_keyb_int_func)(running_machine *, int))
{
	/* PC-XT keyboard */
	if (flags & PCCOMMON_KEYBOARD_AT)
		at_keyboard_init(machine, AT_KEYBOARD_TYPE_AT);
	else
		at_keyboard_init(machine, AT_KEYBOARD_TYPE_PC);
	at_keyboard_set_scan_code_set(1);

	memset(&pc_keyb, 0, sizeof(pc_keyb));
	pc_keyb.machine = machine;
	pc_keyb.int_cb = set_keyb_int_func;
	pc_keyb.timer = timer_alloc(machine, pc_keyb_timer, NULL);
}

UINT8 pc_keyb_read(void)
{
	return pc_keyb.data;
}



static TIMER_CALLBACK( pc_keyb_timer )
{
	if ( pc_keyb.on ) {
		pc_keyboard();
	} else {
		/* Clock has been low for more than 5 msec, start diagnostic test */
		at_keyboard_reset(machine);
		pc_keyb.self_test = 1;
	}
}



void pc_keyb_set_clock(int on)
{
	on = on ? 1 : 0;

	if (pc_keyb.on != on)
	{
		if (!on)
			timer_adjust_oneshot(pc_keyb.timer, ATTOTIME_IN_MSEC(5), 0);
		else {
			if ( pc_keyb.self_test ) {
				/* The self test of the keyboard takes some time. 2 msec seems to work. */
				/* This still needs to verified against a real keyboard. */
				timer_adjust_oneshot(pc_keyb.timer, ATTOTIME_IN_MSEC( 2 ), 0);
			} else {
				timer_reset(pc_keyb.timer, attotime_never);
				pc_keyb.self_test = 0;
			}
		}

		pc_keyb.on = on;
	}
}

void pc_keyb_clear(void)
{
	pc_keyb.data = 0;
	if ( pc_keyb.int_cb ) {
		pc_keyb.int_cb(pc_keyb.machine, 0);
	}
}

void pc_keyboard(void)
{
	int data;

	at_keyboard_polling();

	if (pc_keyb.on)
	{
		if ( (data=at_keyboard_read())!=-1) {
			pc_keyb.data = data;
			DBG_LOG(1,"KB_scancode",("$%02x\n", pc_keyb.data));
			if ( pc_keyb.int_cb ) {
				pc_keyb.int_cb(pc_keyb.machine, 1);
			}
			pc_keyb.self_test = 0;
		}
	}
}
