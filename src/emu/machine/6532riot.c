/***************************************************************************

  RIOT 6532 emulation

The timer seems to follow these rules:
- When the timer flag changes from 0 to 1 the timer continues to count
  down at a 1 cycle rate.
- When the timer is being read or written the timer flag is reset.
- When the timer flag is set and the timer contents are 0, the counting
  stops.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "machine/6532riot.h"

struct riot6532
{
	const struct riot6532_interface *intf;

	UINT8 in_a;
	UINT8 out_a;
	UINT8 in_b;
	UINT8 out_b;
	UINT8 ddr_a;
	UINT8 ddr_b;

	int shift;

	int pa7_enable;
	int pa7_direction;	/* 0x80 = high-to-low, 0x00 = low-to-high */
	int pa7_flag;
	UINT8 pa7;

	int timer_irq_enable;
	int timer_irq;
	emu_timer	*counter_timer;

	int clock;
};


static struct riot6532 r6532[MAX_R6532];


INLINE void r6532_set_timer(int which, UINT8 count)
{
	timer_adjust_oneshot( r6532[which].counter_timer, attotime_mul(ATTOTIME_IN_HZ(r6532[which].clock), (count << r6532[which].shift) + 1), which );
}


static TIMER_CALLBACK( r6532_irq_timer_callback )
{
	int which = param;

	if ( r6532[which].timer_irq_enable )
	{
		r6532[which].timer_irq = 1;
		if (r6532[which].intf->irq_func)
			(*r6532[which].intf->irq_func)(ASSERT_LINE);
		else
			logerror("6532RIOT chip #%d: Interrupt is asserted but there is no callback function.  PC: %08X\n", which, safe_activecpu_get_pc());
	}
}


static void r6532_pa7_check(int which){
	UINT8 data = ( ( r6532[which].ddr_a & r6532[which].out_a ) | ( ~r6532[which].ddr_a & r6532[which].in_a ) ) & 0x80;
	if ((r6532[which].pa7 ^ data) && (r6532[which].pa7_direction ^ data))
	{
		r6532[which].pa7_flag = 1;
		if (r6532[which].pa7_enable)
		{
			if (r6532[which].intf->irq_func)
				(*r6532[which].intf->irq_func)(ASSERT_LINE);
			else
				logerror("6532RIOT chip #%d: Interrupt is asserted but there is no callback function.  PC: %08X\n", which, safe_activecpu_get_pc());
		}
	}
	r6532[which].pa7 = data;
}


void r6532_write(int which, offs_t offset, UINT8 data)
{
	if (offset & 4)
	{
		if (offset & 0x10)
		{
			switch (offset & 3)
			{
			case 0:
				r6532[which].shift = 0;
				break;
			case 1:
				r6532[which].shift = 3;
				break;
			case 2:
				r6532[which].shift = 6;
				break;
			case 3:
				r6532[which].shift = 10;
				break;
			}
			r6532[which].timer_irq_enable = (offset & 8);
			r6532_set_timer( which, data );
		}
		else
		{
			r6532[which].pa7_enable = (offset & 2) >> 1;
			r6532[which].pa7_direction = ( offset & 1 ) << 7;
		}
	}
	else
	{
		offset &= 3;

		switch (offset)
		{
		case 0:
			r6532[which].out_a = data;
			if (r6532[which].ddr_a)
			{
				UINT8 write_data = ( r6532[which].ddr_a & r6532[which].out_a ) | ( ~r6532[which].ddr_a & 0xFF );
				if (r6532[which].intf->out_a_func)
					r6532[which].intf->out_a_func(Machine, 0, write_data);
				else
					logerror("6532RIOT chip #%d: Port A is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), write_data);
				/* Check for PA7 change */
				r6532_pa7_check(which);
			}
			break;
		case 1:
			r6532[which].ddr_a = data;
			r6532_pa7_check(which);
			break;
		case 2:
			r6532[which].out_b = data;
			if (r6532[which].ddr_b)
			{
				UINT8 write_data = ( r6532[which].ddr_b & r6532[which].out_b ) | ( ~r6532[which].ddr_b & 0xFF );
				if (r6532[which].intf->out_b_func)
					r6532[which].intf->out_b_func(Machine, 0, write_data);
				else
					logerror("6532RIOT chip #%d: Port B is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), write_data);
			}
			break;
		case 3:
			r6532[which].ddr_b = data;
			break;
		}
	}
}


INLINE UINT8 r6532_read_timer(int which)
{
	int timer_cycles_left = ( attotime_to_double(timer_timeleft( r6532[which].counter_timer )) * r6532[which].clock ) - 1;
	if ( timer_cycles_left >= 0)
	{
		timer_cycles_left = timer_cycles_left >> r6532[which].shift;
	}
	else
	{
		if (timer_cycles_left != -1)
		{
			if (r6532[which].intf->irq_func && r6532[which].timer_irq)
				(*r6532[which].intf->irq_func)(CLEAR_LINE);
			else
				logerror("6532RIOT chip #%d: Interrupt is cleared but there is no callback function.  PC: %08X\n", which, safe_activecpu_get_pc());

			/* Timer flag is cleared, so adjust the target */
			timer_cycles_left = ( timer_cycles_left > -256 ) ? timer_cycles_left & 0xFF : 0;
			r6532_set_timer( which, timer_cycles_left );
		}
	}
	return timer_cycles_left;
}


void r6532_set_input_a(int which, UINT8 data)
{
	r6532[which].in_a = data;
	/* Check for PA7 change */
	r6532_pa7_check(which);
}


void r6532_set_input_b(int which, UINT8 data)
{
	r6532[which].in_b = data;
}


INLINE UINT8 r6532_read_irq_flags(int which)
{
	int timer_cycles_left = ( attotime_to_double(timer_timeleft( r6532[which].counter_timer )) * r6532[which].clock ) - 1;
	int res = 0;

	if ( timer_cycles_left < 0 )
	{
		res |= 0x80;
		if ( timer_cycles_left < -1 )
		{
			if ( r6532[which].intf->irq_func)
				(*r6532[which].intf->irq_func)(CLEAR_LINE);
			else
				logerror("6532RIOT chip #%d: Interrupt is cleared but there is no callback function.  PC: %08X\n", which, safe_activecpu_get_pc());

			/* Timer flag is cleared, so adjust the target */
			r6532_set_timer( which, timer_cycles_left > -256 ? timer_cycles_left & 0xFF : 0 );
		}
	}

	if (r6532[which].pa7_flag)
	{
		res |= 0x40;
		r6532[which].pa7_flag = 0;

		if (r6532[which].intf->irq_func && timer_cycles_left != -1)
			(*r6532[which].intf->irq_func)(CLEAR_LINE);
		else
			logerror("6532RIOT chip #%d: Interrupt is cleared but there is no callback function.  PC: %08X\n", which, safe_activecpu_get_pc());
	}

	return res;
}


UINT8 r6532_read(int which, offs_t offset)
{
	UINT8 val = 0;

	switch (offset & 7)
	{
	case 0:
		if (r6532[which].intf->in_a_func)
			r6532[which].in_a = r6532[which].intf->in_a_func(Machine, 0);
		else
			logerror("6532RIOT chip #%d: Port A is being read but has no handler.  PC: %08X\n", which, safe_activecpu_get_pc());
		val = ( r6532[which].ddr_a & r6532[which].out_a ) | ( ~r6532[which].ddr_a & r6532[which].in_a );
		/* Check for PA7 change */
		r6532_pa7_check(which);
		break;
	case 1:
		val = r6532[which].ddr_a;
		break;
	case 2:
		if (r6532[which].intf->in_b_func)
			r6532[which].in_b = r6532[which].intf->in_b_func(Machine, 0);
		else
			logerror("6532RIOT chip #%d: Port B is being read but has no handler.  PC: %08X\n", which, safe_activecpu_get_pc());

		val = ( r6532[which].ddr_b & r6532[which].out_b ) | ( ~r6532[which].ddr_b & r6532[which].in_b );
		break;
	case 3:
		val = r6532[which].ddr_b;
		break;
	case 4:
	case 6:
		r6532[which].timer_irq_enable = offset & 8;
		val = r6532_read_timer(which);
		break;
	case 5:
	case 7:
		val = r6532_read_irq_flags(which);
		break;
	}

	return val;
}


void r6532_set_clock(int which, int clock)
{
	r6532[which].clock = clock;
}


void r6532_reset(int which)
{
	r6532[which].out_a = 0;
	r6532[which].out_b = 0;
	r6532[which].ddr_a = 0;
	r6532[which].ddr_b = 0;

	r6532[which].shift = 10;

	r6532[which].counter_timer = timer_alloc(r6532_irq_timer_callback, NULL);

	r6532_set_timer( which, 0xFF );

	r6532[which].pa7_enable = 0;
	r6532[which].pa7_direction = 0x80;
	r6532[which].pa7_flag = 0;
	r6532[which].pa7 = 0;

	r6532[which].timer_irq_enable = 0;
	r6532[which].timer_irq = 0;

	if (r6532[which].intf->irq_func)
		(*r6532[which].intf->irq_func)(CLEAR_LINE);
	else
		logerror("6532RIOT chip #%d: Interrupt is cleared but there is no callback function.  PC: %08X\n", which, safe_activecpu_get_pc());
}


void r6532_config(int which, const struct riot6532_interface* intf)
{
	assert_always(mame_get_phase(Machine) == MAME_PHASE_INIT, "Can only call r6532_init at init time!");
	assert_always( which < MAX_R6532, "which exceeds maximum number of configured r6532s!" );

	r6532[which].intf = intf;

	/* Default clock is CPU #0 clock */
	r6532_set_clock( which, cpunum_get_clock(0) );
}


WRITE8_HANDLER( r6532_0_w ) { r6532_write(0, offset, data); }
WRITE8_HANDLER( r6532_1_w ) { r6532_write(1, offset, data); }
WRITE8_HANDLER( r6532_2_w ) { r6532_write(2, offset, data); }
WRITE8_HANDLER( r6532_3_w ) { r6532_write(3, offset, data); }
WRITE8_HANDLER( r6532_4_w ) { r6532_write(4, offset, data); }
WRITE8_HANDLER( r6532_5_w ) { r6532_write(5, offset, data); }
WRITE8_HANDLER( r6532_6_w ) { r6532_write(6, offset, data); }
WRITE8_HANDLER( r6532_7_w ) { r6532_write(7, offset, data); }

READ8_HANDLER( r6532_0_r ) { return r6532_read(0, offset); }
READ8_HANDLER( r6532_1_r ) { return r6532_read(1, offset); }
READ8_HANDLER( r6532_2_r ) { return r6532_read(2, offset); }
READ8_HANDLER( r6532_3_r ) { return r6532_read(3, offset); }
READ8_HANDLER( r6532_4_r ) { return r6532_read(4, offset); }
READ8_HANDLER( r6532_5_r ) { return r6532_read(5, offset); }
READ8_HANDLER( r6532_6_r ) { return r6532_read(6, offset); }
READ8_HANDLER( r6532_7_r ) { return r6532_read(7, offset); }

WRITE8_HANDLER( r6532_0_porta_w) { r6532_set_input_a(0, data); }
WRITE8_HANDLER( r6532_1_porta_w) { r6532_set_input_a(1, data); }
WRITE8_HANDLER( r6532_2_porta_w) { r6532_set_input_a(2, data); }
WRITE8_HANDLER( r6532_3_porta_w) { r6532_set_input_a(3, data); }
WRITE8_HANDLER( r6532_4_porta_w) { r6532_set_input_a(4, data); }
WRITE8_HANDLER( r6532_5_porta_w) { r6532_set_input_a(5, data); }
WRITE8_HANDLER( r6532_6_porta_w) { r6532_set_input_a(6, data); }
WRITE8_HANDLER( r6532_7_porta_w) { r6532_set_input_a(7, data); }

WRITE8_HANDLER( r6532_0_portb_w) { r6532_set_input_b(0, data); }
WRITE8_HANDLER( r6532_1_portb_w) { r6532_set_input_b(1, data); }
WRITE8_HANDLER( r6532_2_portb_w) { r6532_set_input_b(2, data); }
WRITE8_HANDLER( r6532_3_portb_w) { r6532_set_input_b(3, data); }
WRITE8_HANDLER( r6532_4_portb_w) { r6532_set_input_b(4, data); }
WRITE8_HANDLER( r6532_5_portb_w) { r6532_set_input_b(5, data); }
WRITE8_HANDLER( r6532_6_portb_w) { r6532_set_input_b(6, data); }
WRITE8_HANDLER( r6532_7_portb_w) { r6532_set_input_b(7, data); }

READ8_HANDLER( r6532_0_porta_r) { return r6532[0].in_a; }
READ8_HANDLER( r6532_1_porta_r) { return r6532[1].in_a; }
READ8_HANDLER( r6532_2_porta_r) { return r6532[2].in_a; }
READ8_HANDLER( r6532_3_porta_r) { return r6532[3].in_a; }
READ8_HANDLER( r6532_4_porta_r) { return r6532[4].in_a; }
READ8_HANDLER( r6532_5_porta_r) { return r6532[5].in_a; }
READ8_HANDLER( r6532_6_porta_r) { return r6532[6].in_a; }
READ8_HANDLER( r6532_7_porta_r) { return r6532[7].in_a; }

READ8_HANDLER( r6532_0_portb_r) { return r6532[0].in_b; }
READ8_HANDLER( r6532_1_portb_r) { return r6532[1].in_b; }
READ8_HANDLER( r6532_2_portb_r) { return r6532[2].in_b; }
READ8_HANDLER( r6532_3_portb_r) { return r6532[3].in_b; }
READ8_HANDLER( r6532_4_portb_r) { return r6532[4].in_b; }
READ8_HANDLER( r6532_5_portb_r) { return r6532[5].in_b; }
READ8_HANDLER( r6532_6_portb_r) { return r6532[6].in_b; }
READ8_HANDLER( r6532_7_portb_r) { return r6532[7].in_b; }

