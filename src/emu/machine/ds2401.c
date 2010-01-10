/*
 * DS2401
 *
 * Dallas Semiconductor
 * Silicon Serial Number
 *
 */

#include "emu.h"
#include "machine/ds2401.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine *machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", cpuexec_describe_context(machine), buf );
	}
}

struct ds2401_chip
{
	int state;
	int bit;
	int byte;
	int shift;
	int rx;
	int tx;
	const UINT8 *data;
	emu_timer *timer;
	emu_timer *reset_timer;
	attotime t_samp;
	attotime t_rdv;
	attotime t_rstl;
	attotime t_pdh;
	attotime t_pdl;
};

#define SIZE_DATA ( 8 )

#define STATE_IDLE ( 0 )
#define STATE_RESET ( 1 )
#define STATE_RESET1 ( 2 )
#define STATE_RESET2 ( 3 )
#define STATE_COMMAND ( 4 )
#define STATE_READROM ( 5 )

#define COMMAND_READROM ( 0x33 )

static struct ds2401_chip ds2401[ DS2401_MAXCHIP ];

static TIMER_CALLBACK( ds2401_reset )
{
	int which = param;
	struct ds2401_chip *c = &ds2401[ which ];

	verboselog( machine, 1, "ds2401_reset(%d)\n", which );

	c->state = STATE_RESET;
	timer_adjust_oneshot( c->timer, attotime_never, which );
}

static TIMER_CALLBACK( ds2401_tick )
{
	int which = param;
	struct ds2401_chip *c = &ds2401[ which ];

	switch( c->state )
	{
	case STATE_RESET1:
		verboselog( machine, 2, "ds2401_tick(%d) state_reset1 %d\n", which, c->rx );
		c->tx = 0;
		c->state = STATE_RESET2;
		timer_adjust_oneshot( c->timer, c->t_pdl, which );
		break;
	case STATE_RESET2:
		verboselog( machine, 2, "ds2401_tick(%d) state_reset2 %d\n", which, c->rx );
		c->tx = 1;
		c->bit = 0;
		c->shift = 0;
		c->state = STATE_COMMAND;
		break;
	case STATE_COMMAND:
		verboselog( machine, 2, "ds2401_tick(%d) state_command %d\n", which, c->rx );
		c->shift >>= 1;
		if( c->rx != 0 )
		{
			c->shift |= 0x80;
		}
		c->bit++;
		if( c->bit == 8 )
		{
			switch( c->shift )
			{
			case COMMAND_READROM:
				verboselog( machine, 1, "ds2401_tick(%d) readrom\n", which );
				c->bit = 0;
				c->byte = 0;
				c->state = STATE_READROM;
				break;
			default:
				verboselog( machine, 0, "ds2401_tick(%d) command not handled %02x\n", which, c->shift );
				c->state = STATE_IDLE;
				break;
			}
		}
		break;
	case STATE_READROM:
		c->tx = 1;
		if( c->byte == 8 )
		{
			verboselog( machine, 1, "ds2401_tick(%d) readrom finished\n", which );
			c->state = STATE_IDLE;
		}
		else
		{
			verboselog( machine, 2, "ds2401_tick(%d) readrom window closed\n", which );
		}
		break;
	default:
		verboselog( machine, 0, "ds2401_tick(%d) state not handled: %d\n", which, c->state );
		break;
	}
}

void ds2401_init( running_machine *machine, int which, const UINT8 *data )
{
	struct ds2401_chip *c = &ds2401[ which ];

	c->state = STATE_IDLE;
	c->bit = 0;
	c->byte = 0;
	c->shift = 0;
	c->rx = 1;
	c->tx = 1;
	c->data = data;
	c->t_samp = ATTOTIME_IN_USEC( 15 );
	c->t_rdv = ATTOTIME_IN_USEC( 15 );
	c->t_rstl = ATTOTIME_IN_USEC( 480 );
	c->t_pdh = ATTOTIME_IN_USEC( 15 );
	c->t_pdl = ATTOTIME_IN_USEC( 60 );

	state_save_register_item(machine,  "ds2401", NULL, which, c->state );
	state_save_register_item(machine,  "ds2401", NULL, which, c->bit );
	state_save_register_item(machine,  "ds2401", NULL, which, c->byte );
	state_save_register_item(machine,  "ds2401", NULL, which, c->shift );
	state_save_register_item(machine,  "ds2401", NULL, which, c->rx );
	state_save_register_item(machine,  "ds2401", NULL, which, c->tx );

	c->timer = timer_alloc(machine, ds2401_tick , NULL);
	c->reset_timer = timer_alloc(machine, ds2401_reset , NULL);
}

void ds2401_write( running_machine *machine, int which, int data )
{
	struct ds2401_chip *c = &ds2401[ which ];

	verboselog( machine, 1, "ds2401_write( %d, %d )\n", which, data );

	if( data == 0 && c->rx != 0 )
	{
		switch( c->state )
		{
		case STATE_IDLE:
			break;
		case STATE_COMMAND:
			verboselog( machine, 2, "ds2401_write(%d) state_command\n", which );
			timer_adjust_oneshot( c->timer, c->t_samp, which );
			break;
		case STATE_READROM:
			if( c->bit == 0 )
			{
				c->shift = c->data[ 7 - c->byte ];
				verboselog( machine, 1, "ds2401_write(%d) <- data %02x\n", which, c->shift );
			}
			c->tx = c->shift & 1;
			c->shift >>= 1;
			c->bit++;
			if( c->bit == 8 )
			{
				c->bit = 0;
				c->byte++;
			}
			verboselog( machine, 2, "ds2401_write(%d) state_readrom %d\n", which, c->tx );
			timer_adjust_oneshot( c->timer, c->t_rdv, which );
			break;
		default:
			verboselog( machine, 0, "ds2401_write(%d) state not handled: %d\n", which, c->state );
			break;
		}
		timer_adjust_oneshot( c->reset_timer, c->t_rstl, which );
	}
	else if( data == 1 && c->rx == 0 )
	{
		switch( c->state )
		{
		case STATE_RESET:
			c->state = STATE_RESET1;
			timer_adjust_oneshot( c->timer, c->t_pdh, which );
			break;
		}
		timer_adjust_oneshot( c->reset_timer, attotime_never, which );
	}
	c->rx = data;
}

int ds2401_read( running_machine *machine, int which )
{
	struct ds2401_chip *c = &ds2401[ which ];

	verboselog( machine, 2, "ds2401_read( %d ) %d\n", which, c->tx & c->rx );
	return c->tx & c->rx;
}

/*

app74.pdf

Under normal circumstances an ibutton will sample the line 30us after the falling edge of the start condition.
The internal time base of ibutton may deviate from its nominal value. The allowed tollerance band ranges from 15us to 60us.
This means that the actual slave sampling may occur anywhere from 15 and 60us after the start condition, which is a ratio of 1 to 4.
During this time frame the voltage on the data line must stay below Vilmax or above Vihmin.

In the 1-Wire system, the logical values 1 and 0 are represented by certain voltages in special waveforms.
The waveforms needed to write commands or data to ibuttons are called write-1 and write-0 time slots.
The duration of a low pulse to write a 1 must be shorter than 15us.
To write a 0, the duration of the low pulse must be at least 60us to cope with worst-case conditions.

The duration of the active part of a time slot can be extended beyond 60us.
The maximum extension is limited by the fact that a low pulse of a duration of at least eight active time slots ( 480us ) is defined as a Reset Pulse.
Allowing the same worst-case tolerance ratio, a low pulse of 120us might be sufficient for a reset.
This limits the extension of the active part of a time slot to a maximum of 120us to prevent misinterpretation with reset.

Commands and data are sent to ibuttons by combining write-0 and write-1 time slots.
To read data, the master has to generate read-data time slots to define the start condition of each bit.
The read-data time slots looks essentially the same as a write-1 time slot from the masters point of view.
Starting at the high-to-low transition, the ibuttons sends 1 bit of its addressed contents.
If the data bit is a 1, the ibutton leaves the pulse unchanged.
If the data bit is a 0, the ibutton will pull the data line low for 15us.
In this time frame data is valid for reading by the master.
The duration of the low pulse sent by the master should be a minimum of 1us with a maximum value as short as possible to maximize the master sampling window.

The Reset Pulse provides a clear starting condition that supersedes any time slot synchronisation.
It is defined as single low pulse of minimum duration of eight time slots or 480us followed by a Reset-high time tRSTH of another 480us.
After a Reset Pulse has been sent, the ibutton will wait for the time tPDH and then generate a Pulse-Presence Pulse of duration tPDL.
No other communication on the 1-Wire bus is allowed during tRSTH.

There are 1,000 microseconds in a millisecond, and 1,000 milliseconds in a second.
Thus, there are 1,000,000 microseconds in a second. Why is it "usec"?
The "u" is supposed to look like the Greek letter Mu that we use for "micro". .
*/
