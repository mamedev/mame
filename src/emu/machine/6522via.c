/**********************************************************************

    Rockwell 6522 VIA interface and emulation

    This function emulates the functionality of up to 8 6522
    versatile interface adapters.

    This is based on the M6821 emulation in MAME.

    To do:

    T2 pulse counting mode
    Pulse mode handshake output
    More shift register

**********************************************************************/

/*
  1999-Dec-22 PeT
   vc20 random number generation only partly working
   (reads (uninitialized) timer 1 and timer 2 counter)
   timer init, reset, read changed
 */

#include "driver.h"
#include "6522via.h"

//#define TRACE_VIA

/******************* internal VIA data structure *******************/

struct via6522
{
	const struct via6522_interface *intf;

	UINT8 in_a;
	UINT8 in_ca1;
	UINT8 in_ca2;
	UINT8 out_a;
	UINT8 out_ca2;
	UINT8 ddr_a;

	UINT8 in_b;
	UINT8 in_cb1;
	UINT8 in_cb2;
	UINT8 out_b;
	UINT8 out_cb2;
	UINT8 ddr_b;

	UINT8 t1cl;
	UINT8 t1ch;
	UINT8 t1ll;
	UINT8 t1lh;
	UINT8 t2cl;
	UINT8 t2ch;
	UINT8 t2ll;
	UINT8 t2lh;

	UINT8 sr;
	UINT8 pcr;
	UINT8 acr;
	UINT8 ier;
	UINT8 ifr;

	emu_timer *t1;
	attotime time1;
	UINT8 t1_active;
	emu_timer *t2;
	attotime time2;
	UINT8 t2_active;
	UINT8 shift_counter;

	int clock;
};


/******************* convenince macros and defines *******************/

/* Macros for PCR */
#define CA1_LOW_TO_HIGH(c)		(c & 0x01)
#define CA1_HIGH_TO_LOW(c)		(!(c & 0x01))

#define CB1_LOW_TO_HIGH(c)		(c & 0x10)
#define CB1_HIGH_TO_LOW(c)		(!(c & 0x10))

#define CA2_INPUT(c)			(!(c & 0x08))
#define CA2_LOW_TO_HIGH(c)		((c & 0x0c) == 0x04)
#define CA2_HIGH_TO_LOW(c)		((c & 0x0c) == 0x00)
#define CA2_IND_IRQ(c)			((c & 0x0a) == 0x02)

#define CA2_OUTPUT(c)			(c & 0x08)
#define CA2_AUTO_HS(c)			((c & 0x0c) == 0x08)
#define CA2_HS_OUTPUT(c)		((c & 0x0e) == 0x08)
#define CA2_PULSE_OUTPUT(c)		((c & 0x0e) == 0x0a)
#define CA2_FIX_OUTPUT(c)		((c & 0x0c) == 0x0c)
#define CA2_OUTPUT_LEVEL(c)		((c & 0x02) >> 1)

#define CB2_INPUT(c)			(!(c & 0x80))
#define CB2_LOW_TO_HIGH(c)		((c & 0xc0) == 0x40)
#define CB2_HIGH_TO_LOW(c)		((c & 0xc0) == 0x00)
#define CB2_IND_IRQ(c)			((c & 0xa0) == 0x20)

#define CB2_OUTPUT(c)			(c & 0x80)
#define CB2_AUTO_HS(c)			((c & 0xc0) == 0x80)
#define CB2_HS_OUTPUT(c)		((c & 0xe0) == 0x80)
#define CB2_PULSE_OUTPUT(c)		((c & 0xe0) == 0xa0)
#define CB2_FIX_OUTPUT(c)		((c & 0xc0) == 0xc0)
#define CB2_OUTPUT_LEVEL(c)		((c & 0x20) >> 5)

/* Macros for ACR */
#define PA_LATCH_ENABLE(c)		(c & 0x01)
#define PB_LATCH_ENABLE(c)		(c & 0x02)

#define SR_DISABLED(c)			(!(c & 0x1c))
#define SI_T2_CONTROL(c)		((c & 0x1c) == 0x04)
#define SI_O2_CONTROL(c)		((c & 0x1c) == 0x08)
#define SI_EXT_CONTROL(c)		((c & 0x1c) == 0x0c)
#define SO_T2_RATE(c)			((c & 0x1c) == 0x10)
#define SO_T2_CONTROL(c)		((c & 0x1c) == 0x14)
#define SO_O2_CONTROL(c)		((c & 0x1c) == 0x18)
#define SO_EXT_CONTROL(c)		((c & 0x1c) == 0x1c)

#define T1_SET_PB7(c)			(c & 0x80)
#define T1_CONTINUOUS(c)		(c & 0x40)
#define T2_COUNT_PB6(c)			(c & 0x20)

/* Interrupt flags */
#define INT_CA2	0x01
#define INT_CA1	0x02
#define INT_SR	0x04
#define INT_CB2	0x08
#define INT_CB1	0x10
#define INT_T2	0x20
#define INT_T1	0x40
#define INT_ANY	0x80

#define CLR_PA_INT(v, which)	via_clear_int (which, INT_CA1 | ((!CA2_IND_IRQ(v->pcr)) ? INT_CA2: 0))
#define CLR_PB_INT(v, which)	via_clear_int (which, INT_CB1 | ((!CB2_IND_IRQ(v->pcr)) ? INT_CB2: 0))

#define IFR_DELAY 3

#define TIMER1_VALUE(v) (v->t1ll+(v->t1lh<<8))
#define TIMER2_VALUE(v) (v->t2ll+(v->t2lh<<8))

/******************* static variables *******************/

static struct via6522 via[MAX_VIA];

/******************* configuration *******************/

void via_set_clock(int which,int clock)
{
	via[which].clock = clock;
}

void via_config(int which, const struct via6522_interface *intf)
{
	assert(which < MAX_VIA);

	via[which].intf = intf;
	via[which].t1ll = 0xf3; /* via at 0x9110 in vic20 show these values */
	via[which].t1lh = 0xb5; /* ports are not written by kernel! */
	via[which].t2ll = 0xff; /* taken from vice */
	via[which].t2lh = 0xff;
	via[which].time2 = via[which].time1 = timer_get_time();

	/* Default clock is from CPU1 */
	via_set_clock (which, Machine->drv->cpu[0].clock);
}

/******************* external interrupt check *******************/

static void via_set_int (int which, int data)
{
	struct via6522 *v = via + which;


	v->ifr |= data;
#ifdef TRACE_VIA
logerror("6522VIA chip %d: IFR = %02X.  PC: %08X\n", which, v->ifr, safe_activecpu_get_pc());
#endif

	if (v->ier & v->ifr)
    {
		v->ifr |= INT_ANY;
		if (v->intf->irq_func)
			(*v->intf->irq_func)(ASSERT_LINE);
		else
			logerror("6522VIA chip %d: Interrupt is asserted but there is no callback function.  PC: %08X\n", which, safe_activecpu_get_pc());
    }
}

static void via_clear_int (int which, int data)
{
	struct via6522 *v = via + which;


	v->ifr = (v->ifr & ~data) & 0x7f;
#ifdef TRACE_VIA
logerror("6522VIA chip %d: IFR = %02X.  PC: %08X\n", which, v->ifr, safe_activecpu_get_pc());
#endif

	if (v->ifr & v->ier)
		v->ifr |= INT_ANY;
	else
	{
		if (v->intf->irq_func)
			(*v->intf->irq_func)(CLEAR_LINE);
//      else
//          logerror("6522VIA chip %d: Interrupt is cleared but there is no callback function.  PC: %08X\n", which, safe_activecpu_get_pc());
	}
}


INLINE attotime v_cycles_to_time(struct via6522 *v, int c)
{
	return attotime_mul(ATTOTIME_IN_HZ(v->clock), c);
}


INLINE UINT32 v_time_to_cycles(struct via6522 *v, attotime t)
{
	return attotime_to_double(attotime_mul(t, v->clock));
}


INLINE UINT16 v_get_counter1_value(struct via6522 *v) {
	UINT16 val;

	if (v->t1_active) {
		val = v_time_to_cycles(v, timer_timeleft(v->t1)) - IFR_DELAY;
	} else {
		val = 0xFFFF - v_time_to_cycles(v, attotime_sub(timer_get_time(), v->time1));
	}
	return val;
}


/************************ shift register ************************/

static TIMER_CALLBACK( via_shift_callback );

static void via_shift(int which)
{
	struct via6522 *v = via + which;

	if (SO_O2_CONTROL(v->acr))
	{
		v->out_cb2 = (v->sr >> 7) & 1;
		v->sr =  (v->sr << 1) | v->out_cb2;

		if (v->intf->out_cb2_func)
			v->intf->out_cb2_func(0, v->out_cb2);

		v->in_cb1=1;
		if (v->intf->out_cb1_func)
		{
			/* this should be one cycle wide */
			v->intf->out_cb1_func(0, 0);
			v->intf->out_cb1_func(0, 1);
		}

		v->shift_counter = (v->shift_counter + 1) % 8;

		if (v->shift_counter)
			timer_set(v_cycles_to_time(v, 2), NULL, which, via_shift_callback);
		else
		{
			if (!(v->ifr & INT_SR))
				via_set_int(which, INT_SR);
		}
	}
	if (SO_EXT_CONTROL(v->acr))
	{
		v->out_cb2 = (v->sr >> 7) & 1;
		v->sr =  (v->sr << 1) | v->out_cb2;

		if (v->intf->out_cb2_func)
			v->intf->out_cb2_func(0, v->out_cb2);

		v->shift_counter = (v->shift_counter + 1) % 8;

		if (v->shift_counter == 0)
		{
			if (!(v->ifr & INT_SR))
				via_set_int(which, INT_SR);
		}
	}
	if (SI_EXT_CONTROL(v->acr))
	{
		if (v->intf->in_cb2_func)
			v->in_cb2 = v->intf->in_cb2_func(0);

		v->sr =  (v->sr << 1) | (v->in_cb2 & 1);

		v->shift_counter = (v->shift_counter + 1) % 8;

		if (v->shift_counter == 0)
		{
			if (!(v->ifr & INT_SR))
			{
				via_set_int(which, INT_SR);
			}
		}
	}
}

static TIMER_CALLBACK( via_shift_callback )
{
	via_shift(param);
}

/******************* Timer timeouts *************************/

static TIMER_CALLBACK( via_t1_timeout )
{
	int which = param;
	struct via6522 *v = via + which;


	if (T1_CONTINUOUS (v->acr))
    {
		if (T1_SET_PB7(v->acr))
			v->out_b ^= 0x80;
		timer_adjust(v->t1, v_cycles_to_time(v, TIMER1_VALUE(v) + IFR_DELAY), which, attotime_zero);
    }
	else
    {
		if (T1_SET_PB7(v->acr))
			v->out_b |= 0x80;
		v->t1_active = 0;
		v->time1 = timer_get_time();
    }
	if (v->ddr_b)
	{
		UINT8 write_data = (v->out_b & v->ddr_b) | (v->ddr_b ^ 0xff);

		if (v->intf->out_b_func)
			v->intf->out_b_func(0, write_data);
		else
			logerror("6522VIA chip %d: Port B is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), write_data);
	}

	if (!(v->ifr & INT_T1))
		via_set_int (which, INT_T1);
}

static TIMER_CALLBACK( via_t2_timeout )
{
	int which = param;
	struct via6522 *v = via + which;

	v->t2_active = 0;
	v->time2 = timer_get_time();

	if (!(v->ifr & INT_T2))
		via_set_int (which, INT_T2);
}

/******************* reset *******************/

void via_reset(void)
{
	int i;
	struct via6522 v;

	memset(&v, 0, sizeof(v));

	for (i = 0; i < MAX_VIA; i++)
    {
		v.intf = via[i].intf;
		v.t1ll = via[i].t1ll;
		v.t1lh = via[i].t1lh;
		v.t2ll = via[i].t2ll;
		v.t2lh = via[i].t2lh;
		v.time1 = via[i].time1;
		v.time2 = via[i].time2;
		v.clock = via[i].clock;

		v.t1 = timer_alloc(via_t1_timeout, NULL);
		v.t1_active = 0;
		v.t2 = timer_alloc(via_t2_timeout, NULL);
		v.t2_active = 0;

		via[i] = v;
    }
}

/******************* CPU interface for VIA read *******************/

int via_read(int which, int offset)
{
	struct via6522 *v = via + which;
	int val = 0;

	offset &= 0xf;

	switch (offset)
    {
    case VIA_PB:
		/* update the input */
		if (PB_LATCH_ENABLE(v->acr) == 0)
		{
			if (v->intf->in_b_func)
				v->in_b = v->intf->in_b_func(0);
			else
				logerror("6522VIA chip %d: Port B is being read but has no handler.  PC: %08X\n", which, safe_activecpu_get_pc());
		}

		CLR_PB_INT(v, which);

		/* combine input and output values, hold DDRB bit 7 high if T1_SET_PB7 */
		if (T1_SET_PB7(v->acr))
			val = (v->out_b & (v->ddr_b | 0x80)) | (v->in_b & ~(v->ddr_b | 0x80));
		else
			val = (v->out_b & v->ddr_b) + (v->in_b & ~v->ddr_b);
		break;

    case VIA_PA:
		/* update the input */
		if (PA_LATCH_ENABLE(v->acr) == 0)
		{
			if (v->intf->in_a_func)
				v->in_a = v->intf->in_a_func(0);
			else
				logerror("6522VIA chip %d: Port A is being read but has no handler.  PC: %08X\n", which, safe_activecpu_get_pc());
		}

		/* combine input and output values */
		val = (v->out_a & v->ddr_a) + (v->in_a & ~v->ddr_a);

		CLR_PA_INT(v, which);

		/* If CA2 is configured as output and in pulse or handshake mode,
           CA2 is set now */
		if (CA2_AUTO_HS(v->pcr))
		{
			if (v->out_ca2)
			{
				/* set CA2 */
				v->out_ca2 = 0;

				/* call the CA2 output function */
				if (v->intf->out_ca2_func)
					v->intf->out_ca2_func(0, 0);
				else
					logerror("6522VIA chip %d: Port CA2 is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), 0);
			}
		}

		break;

    case VIA_PANH:
		/* update the input */
		if (PA_LATCH_ENABLE(v->acr) == 0)
		{
			if (v->intf->in_a_func)
				v->in_a = v->intf->in_a_func(0);
			else
				logerror("6522VIA chip %d: Port A is being read but has no handler.  PC: %08X\n", which, safe_activecpu_get_pc());
		}

		/* combine input and output values */
		val = (v->out_a & v->ddr_a) + (v->in_a & ~v->ddr_a);
		break;

    case VIA_DDRB:
		val = v->ddr_b;
		break;

    case VIA_DDRA:
		val = v->ddr_a;
		break;

    case VIA_T1CL:
		via_clear_int (which, INT_T1);
		val = v_get_counter1_value(v) & 0xFF;
		break;

    case VIA_T1CH:
		val = v_get_counter1_value(v) >> 8;
		break;

    case VIA_T1LL:
		val = v->t1ll;
		break;

    case VIA_T1LH:
		val = v->t1lh;
		break;

    case VIA_T2CL:
		via_clear_int (which, INT_T2);
		if (v->t2_active)
			val = v_time_to_cycles(v, timer_timeleft(v->t2)) & 0xff;
		else
		{
			if (T2_COUNT_PB6(v->acr))
				val = v->t2cl;
			else
				val = (0x10000- (v_time_to_cycles(v, attotime_sub(timer_get_time(), v->time2)) & 0xffff) - 1) & 0xff;
		}
		break;

    case VIA_T2CH:
		if (v->t2_active)
			val = v_time_to_cycles(v, timer_timeleft(v->t2)) >> 8;
		else
		{
			if (T2_COUNT_PB6(v->acr))
				val = v->t2ch;
			else
				val = (0x10000- (v_time_to_cycles(v, attotime_sub(timer_get_time(), v->time2)) & 0xffff) - 1) >> 8;
		}
		break;

    case VIA_SR:
		val = v->sr;
		via_clear_int(which, INT_SR);
		if (SO_O2_CONTROL(v->acr))
		{
			v->shift_counter=0;
			timer_set(v_cycles_to_time(v, 2), NULL, which,via_shift_callback);
		}
		break;

    case VIA_PCR:
		val = v->pcr;
		break;

    case VIA_ACR:
		val = v->acr;
		break;

    case VIA_IER:
		val = v->ier | 0x80;
		break;

    case VIA_IFR:
		val = v->ifr;
		break;
    }
	return val;
}


/******************* CPU interface for VIA write *******************/

void via_write(int which, int offset, int data)
{
	struct via6522 *v = via + which;

	offset &=0x0f;

	switch (offset)
    {
    case VIA_PB:
		if (T1_SET_PB7(v->acr))
			v->out_b = (v->out_b & 0x80) | (data  & 0x7f);
		else
			v->out_b = data;

		if (v->ddr_b)
		{
			UINT8 write_data = (v->out_b & v->ddr_b) | (v->ddr_b ^ 0xff);

			if (v->intf->out_b_func)
				v->intf->out_b_func(0, write_data);
			else
				logerror("6522VIA chip %d: Port B is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), write_data);
		}

		CLR_PB_INT(v, which);

		/* If CB2 is configured as output and in pulse or handshake mode,
           CB2 is set now */
		if (CB2_AUTO_HS(v->pcr))
		{
			if (v->out_cb2)
			{
				/* set CB2 */
				v->out_cb2 = 0;

				/* call the CB2 output function */
				if (v->intf->out_cb2_func)
					v->intf->out_cb2_func(0, 0);
				else
					logerror("6522VIA chip %d: Port CB2 is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), 0);
			}
		}
		break;

    case VIA_PA:
		v->out_a = data;

		if (v->ddr_a)
		{
			UINT8 write_data = (v->out_a & v->ddr_a) | (v->ddr_a ^ 0xff);

			if (v->intf->out_a_func)
				v->intf->out_a_func(0, write_data);
			else
				logerror("6522VIA chip %d: Port A is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), write_data);
		}

		CLR_PA_INT(v, which);

		/* If CA2 is configured as output and in pulse or handshake mode,
           CA2 is set now */
		if (CA2_PULSE_OUTPUT(v->pcr))
		{
			/* call the CA2 output function */
			if (v->intf->out_ca2_func)
			{
				v->intf->out_ca2_func(0, 0);
				v->intf->out_ca2_func(0, 1);
			}
			else
				logerror("6522VIA chip %d: Port CA2 is being pulsed but has no handler.  PC: %08X\n", which, safe_activecpu_get_pc());

			/* set CA2 (shouldn't be needed) */
			v->out_ca2 = 1;
		}
		else if (CA2_AUTO_HS(v->pcr))
		{
			if (v->out_ca2)
			{
				/* set CA2 */
				v->out_ca2 = 0;

				/* call the CA2 output function */
				if (v->intf->out_ca2_func)
					v->intf->out_ca2_func(0, 0);
				else
					logerror("6522VIA chip %d: Port CA2 is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), 0);
			}
		}

		break;

    case VIA_PANH:
		v->out_a = data;

		if (v->ddr_a)
		{
			UINT8 write_data = (v->out_a & v->ddr_a) | (v->ddr_a ^ 0xff);

			if (v->intf->out_a_func)
				v->intf->out_a_func(0, write_data);
			else
				logerror("6522VIA chip %d: Port A is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), write_data);
		}

		break;

    case VIA_DDRB:
    	/* EHC 03/04/2000 - If data direction changed, present output on the lines */
    	if ( data != v->ddr_b )
    	{
			v->ddr_b = data;

			//if (v->ddr_b)
			{
				UINT8 write_data = (v->out_b & v->ddr_b) | (v->ddr_b ^ 0xff);

				if (v->intf->out_b_func)
					v->intf->out_b_func(0, write_data);
				else
					logerror("6522VIA chip %d: Port B is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), write_data);
			}
		}
		break;

    case VIA_DDRA:
    	/* EHC 03/04/2000 - If data direction changed, present output on the lines */
    	if ( data != v->ddr_a )
    	{
			v->ddr_a = data;

			//if (v->ddr_a)
			{
				UINT8 write_data = (v->out_a & v->ddr_a) | (v->ddr_a ^ 0xff);

				if (v->intf->out_a_func)
					v->intf->out_a_func(0, write_data);
				else
					logerror("6522VIA chip %d: Port A is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), write_data);
			}
		}
		break;

    case VIA_T1CL:
    case VIA_T1LL:
		v->t1ll = data;
		break;

	case VIA_T1LH:
	    v->t1lh = data;
	    via_clear_int (which, INT_T1);
	    break;

    case VIA_T1CH:
		v->t1ch = v->t1lh = data;
		v->t1cl = v->t1ll;

		via_clear_int (which, INT_T1);

		if (T1_SET_PB7(v->acr))
		{
			v->out_b &= 0x7f;

			//if (v->ddr_b)
			{
				UINT8 write_data = (v->out_b & v->ddr_b) | (v->ddr_b ^ 0xff);

				if (v->intf->out_b_func)
					v->intf->out_b_func(0, write_data);
				else
					logerror("6522VIA chip %d: Port B is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), write_data);
			}
		}
		timer_adjust(v->t1, v_cycles_to_time(v, TIMER1_VALUE(v) + IFR_DELAY), which, attotime_zero);
		v->t1_active = 1;
		break;

    case VIA_T2CL:
		v->t2ll = data;
		break;

    case VIA_T2CH:
		v->t2ch = v->t2lh = data;
		v->t2cl = v->t2ll;

		via_clear_int (which, INT_T2);

		if (!T2_COUNT_PB6(v->acr))
		{
			timer_adjust(v->t2, v_cycles_to_time(v, TIMER2_VALUE(v) + IFR_DELAY), which, attotime_zero);
			v->t2_active = 1;
		}
		else
		{
			v->time2 = timer_get_time();
		}
		break;

    case VIA_SR:
		v->sr = data;
		v->shift_counter=0;
		via_clear_int(which, INT_SR);
		if (SO_O2_CONTROL(v->acr))
		{
			timer_set(v_cycles_to_time(v, 2), NULL, which, via_shift_callback);
		}
		break;

    case VIA_PCR:
		v->pcr = data;
#ifdef TRACE_VIA
logerror("6522VIA chip %d: PCR = %02X.  PC: %08X\n", which, data, safe_activecpu_get_pc());
#endif

		if (CA2_FIX_OUTPUT(data) && CA2_OUTPUT_LEVEL(data) ^ v->out_ca2)
		{
			v->out_ca2 = CA2_OUTPUT_LEVEL(data);
			if (v->intf->out_ca2_func)
				v->intf->out_ca2_func(0, v->out_ca2);
			else
				logerror("6522VIA chip %d: Port CA2 is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), v->out_ca2);
		}

		if (CB2_FIX_OUTPUT(data) && CB2_OUTPUT_LEVEL(data) ^ v->out_cb2)
		{
			v->out_cb2 = CB2_OUTPUT_LEVEL(data);
			if (v->intf->out_cb2_func)
				v->intf->out_cb2_func(0, v->out_cb2);
			else
				logerror("6522VIA chip %d: Port CB2 is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), v->out_cb2);
		}
		break;

    case VIA_ACR:
		{
			UINT16 counter1 = v_get_counter1_value(v);
			v->acr = data;
			if (T1_SET_PB7(v->acr))
			{
				if (v->t1_active)
					v->out_b &= ~0x80;
				else
					v->out_b |= 0x80;

				//if (v->ddr_b)
				{
					UINT8 write_data = (v->out_b & v->ddr_b) | (v->ddr_b ^ 0xff);
	
					if (v->intf->out_b_func)
						v->intf->out_b_func(0, write_data);
					else
						logerror("6522VIA chip %d: Port B is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), write_data);
				}
			}
			if (T1_CONTINUOUS(data))
			{
				timer_adjust(v->t1, v_cycles_to_time(v, counter1 + IFR_DELAY), which, attotime_zero);
				v->t1_active = 1;
			}
		}
		break;

	case VIA_IER:
		if (data & 0x80)
			v->ier |= data & 0x7f;
		else
			v->ier &= ~(data & 0x7f);

		if (v->ifr & INT_ANY)
		{
			if (((v->ifr & v->ier) & 0x7f) == 0)
			{
				v->ifr &= ~INT_ANY;
				if (v->intf->irq_func)
					(*v->intf->irq_func)(CLEAR_LINE);
//              else
//                  logerror("6522VIA chip %d: Interrupt is cleared but there is no callback function.  PC: %08X\n", which, safe_activecpu_get_pc());
			}
		}
		else
		{
			if ((v->ier & v->ifr) & 0x7f)
			{
				v->ifr |= INT_ANY;
				if (v->intf->irq_func)
					(*v->intf->irq_func)(ASSERT_LINE);
				else
					logerror("6522VIA chip %d: Interrupt is asserted but there is no callback function.  PC: %08X\n", which, safe_activecpu_get_pc());
			}
		}
		break;

	case VIA_IFR:
		if (data & INT_ANY)
			data = 0x7f;
		via_clear_int (which, data);
		break;
    }
}

/******************* interface setting VIA port A input *******************/

void via_set_input_a(int which, int data)
{
	struct via6522 *v = via + which;

	/* set the input, what could be easier? */
	v->in_a = data;
}

/******************* interface setting VIA port CA1 input *******************/

void via_set_input_ca1(int which, int data)
{
	struct via6522 *v = via + which;

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	/* handle the active transition */
	if (data != v->in_ca1)
    {
#ifdef TRACE_VIA
logerror("6522VIA chip %d: CA1 = %02X.  PC: %08X\n", which, data, safe_activecpu_get_pc());
#endif
		if ((CA1_LOW_TO_HIGH(v->pcr) && data) || (CA1_HIGH_TO_LOW(v->pcr) && !data))
		{
			if (PA_LATCH_ENABLE(v->acr))
			{
				if (v->intf->in_a_func)
					v->in_a = v->intf->in_a_func(0);
				else
					logerror("6522VIA chip %d: Port A is being read but has no handler.  PC: %08X\n", which, safe_activecpu_get_pc());
			}

			via_set_int (which, INT_CA1);

			/* CA2 is configured as output and in pulse or handshake mode,
               CA2 is cleared now */
			if (CA2_AUTO_HS(v->pcr))
			{
				if (!v->out_ca2)
				{
					/* clear CA2 */
					v->out_ca2 = 1;

					/* call the CA2 output function */
					if (v->intf->out_ca2_func)
						v->intf->out_ca2_func(0, 1);
					else
						logerror("6522VIA chip %d: Port CA2 is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), 1);
				}
			}
		}

		v->in_ca1 = data;
    }
}

/******************* interface setting VIA port CA2 input *******************/

void via_set_input_ca2(int which, int data)
{
	struct via6522 *v = via + which;

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	/* CA2 is in input mode */
	if (CA2_INPUT(v->pcr))
    {
		/* the new state has caused a transition */
		if (v->in_ca2 != data)
		{
			/* handle the active transition */
			if ((data && CA2_LOW_TO_HIGH(v->pcr)) || (!data && CA2_HIGH_TO_LOW(v->pcr)))
			{
				/* mark the IRQ */
				via_set_int (which, INT_CA2);
			}
			/* set the new value for CA2 */
			v->in_ca2 = data;
		}
    }


}

/******************* interface setting VIA port B input *******************/

void via_set_input_b(int which, int data)
{
	struct via6522 *v = via + which;

	/* set the input, what could be easier? */
	v->in_b = data;
}



/******************* interface setting VIA port CB1 input *******************/

void via_set_input_cb1(int which, int data)
{
	struct via6522 *v = via + which;

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	/* handle the active transition */
	if (data != v->in_cb1)
    {
		if ((CB1_LOW_TO_HIGH(v->pcr) && data) || (CB1_HIGH_TO_LOW(v->pcr) && !data))
		{
			if (PB_LATCH_ENABLE(v->acr))
			{
				if (v->intf->in_b_func)
					v->in_b = v->intf->in_b_func(0);
				else
					logerror("6522VIA chip %d: Port B is being read but has no handler.  PC: %08X\n", which, safe_activecpu_get_pc());
			}
			if (SO_EXT_CONTROL(v->acr) || SI_EXT_CONTROL(v->acr))
				via_shift (which);

			via_set_int (which, INT_CB1);

			/* CB2 is configured as output and in pulse or handshake mode,
               CB2 is cleared now */
			if (CB2_AUTO_HS(v->pcr))
			{
				if (!v->out_cb2)
				{
					/* clear CB2 */
					v->out_cb2 = 1;

					/* call the CB2 output function */
					if (v->intf->out_cb2_func)
						v->intf->out_cb2_func(0, 1);
					else
						logerror("6522VIA chip %d: Port CB2 is being written to but has no handler.  PC: %08X - %02X\n", which, safe_activecpu_get_pc(), 1);
				}
			}
		}
		v->in_cb1 = data;
    }
}

/******************* interface setting VIA port CB2 input *******************/

void via_set_input_cb2(int which, int data)
{
	struct via6522 *v = via + which;

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	/* CB2 is in input mode */
	if (CB2_INPUT(v->pcr))
    {
		/* the new state has caused a transition */
		if (v->in_cb2 != data)
		{
			/* handle the active transition */
			if ((data && CB2_LOW_TO_HIGH(v->pcr)) || (!data && CB2_HIGH_TO_LOW(v->pcr)))
			{
				/* mark the IRQ */
				via_set_int (which, INT_CB2);
			}
			/* set the new value for CB2 */
			v->in_cb2 = data;
		}
    }
}

/******************* Standard 8-bit CPU interfaces, D0-D7 *******************/

READ8_HANDLER( via_0_r) { return via_read(0, offset); }
READ8_HANDLER( via_1_r) { return via_read(1, offset); }
READ8_HANDLER( via_2_r) { return via_read(2, offset); }
READ8_HANDLER( via_3_r) { return via_read(3, offset); }
READ8_HANDLER( via_4_r) { return via_read(4, offset); }
READ8_HANDLER( via_5_r) { return via_read(5, offset); }
READ8_HANDLER( via_6_r) { return via_read(6, offset); }
READ8_HANDLER( via_7_r) { return via_read(7, offset); }

WRITE8_HANDLER( via_0_w) { via_write(0, offset, data); }
WRITE8_HANDLER( via_1_w) { via_write(1, offset, data); }
WRITE8_HANDLER( via_2_w) { via_write(2, offset, data); }
WRITE8_HANDLER( via_3_w) { via_write(3, offset, data); }
WRITE8_HANDLER( via_4_w) { via_write(4, offset, data); }
WRITE8_HANDLER( via_5_w) { via_write(5, offset, data); }
WRITE8_HANDLER( via_6_w) { via_write(6, offset, data); }
WRITE8_HANDLER( via_7_w) { via_write(7, offset, data); }

/******************* 8-bit A/B port interfaces *******************/

WRITE8_HANDLER( via_0_porta_w) { via_set_input_a(0, data); }
WRITE8_HANDLER( via_1_porta_w) { via_set_input_a(1, data); }
WRITE8_HANDLER( via_2_porta_w) { via_set_input_a(2, data); }
WRITE8_HANDLER( via_3_porta_w) { via_set_input_a(3, data); }
WRITE8_HANDLER( via_4_porta_w) { via_set_input_a(4, data); }
WRITE8_HANDLER( via_5_porta_w) { via_set_input_a(5, data); }
WRITE8_HANDLER( via_6_porta_w) { via_set_input_a(6, data); }
WRITE8_HANDLER( via_7_porta_w) { via_set_input_a(7, data); }

WRITE8_HANDLER( via_0_portb_w) { via_set_input_b(0, data); }
WRITE8_HANDLER( via_1_portb_w) { via_set_input_b(1, data); }
WRITE8_HANDLER( via_2_portb_w) { via_set_input_b(2, data); }
WRITE8_HANDLER( via_3_portb_w) { via_set_input_b(3, data); }
WRITE8_HANDLER( via_4_portb_w) { via_set_input_b(4, data); }
WRITE8_HANDLER( via_5_portb_w) { via_set_input_b(5, data); }
WRITE8_HANDLER( via_6_portb_w) { via_set_input_b(6, data); }
WRITE8_HANDLER( via_7_portb_w) { via_set_input_b(7, data); }

READ8_HANDLER( via_0_porta_r) { return via[0].in_a; }
READ8_HANDLER( via_1_porta_r) { return via[1].in_a; }
READ8_HANDLER( via_2_porta_r) { return via[2].in_a; }
READ8_HANDLER( via_3_porta_r) { return via[3].in_a; }
READ8_HANDLER( via_4_porta_r) { return via[4].in_a; }
READ8_HANDLER( via_5_porta_r) { return via[5].in_a; }
READ8_HANDLER( via_6_porta_r) { return via[6].in_a; }
READ8_HANDLER( via_7_porta_r) { return via[7].in_a; }

READ8_HANDLER( via_0_portb_r) { return via[0].in_b; }
READ8_HANDLER( via_1_portb_r) { return via[1].in_b; }
READ8_HANDLER( via_2_portb_r) { return via[2].in_b; }
READ8_HANDLER( via_3_portb_r) { return via[3].in_b; }
READ8_HANDLER( via_4_portb_r) { return via[4].in_b; }
READ8_HANDLER( via_5_portb_r) { return via[5].in_b; }
READ8_HANDLER( via_6_portb_r) { return via[6].in_b; }
READ8_HANDLER( via_7_portb_r) { return via[7].in_b; }

/******************* 1-bit CA1/CA2/CB1/CB2 port interfaces *******************/

WRITE8_HANDLER( via_0_ca1_w) { via_set_input_ca1(0, data); }
WRITE8_HANDLER( via_1_ca1_w) { via_set_input_ca1(1, data); }
WRITE8_HANDLER( via_2_ca1_w) { via_set_input_ca1(2, data); }
WRITE8_HANDLER( via_3_ca1_w) { via_set_input_ca1(3, data); }
WRITE8_HANDLER( via_4_ca1_w) { via_set_input_ca1(4, data); }
WRITE8_HANDLER( via_5_ca1_w) { via_set_input_ca1(5, data); }
WRITE8_HANDLER( via_6_ca1_w) { via_set_input_ca1(6, data); }
WRITE8_HANDLER( via_7_ca1_w) { via_set_input_ca1(7, data); }
WRITE8_HANDLER( via_0_ca2_w) { via_set_input_ca2(0, data); }
WRITE8_HANDLER( via_1_ca2_w) { via_set_input_ca2(1, data); }
WRITE8_HANDLER( via_2_ca2_w) { via_set_input_ca2(2, data); }
WRITE8_HANDLER( via_3_ca2_w) { via_set_input_ca2(3, data); }
WRITE8_HANDLER( via_4_ca2_w) { via_set_input_ca2(4, data); }
WRITE8_HANDLER( via_5_ca2_w) { via_set_input_ca2(5, data); }
WRITE8_HANDLER( via_6_ca2_w) { via_set_input_ca2(6, data); }
WRITE8_HANDLER( via_7_ca2_w) { via_set_input_ca2(7, data); }

WRITE8_HANDLER( via_0_cb1_w) { via_set_input_cb1(0, data); }
WRITE8_HANDLER( via_1_cb1_w) { via_set_input_cb1(1, data); }
WRITE8_HANDLER( via_2_cb1_w) { via_set_input_cb1(2, data); }
WRITE8_HANDLER( via_3_cb1_w) { via_set_input_cb1(3, data); }
WRITE8_HANDLER( via_4_cb1_w) { via_set_input_cb1(4, data); }
WRITE8_HANDLER( via_5_cb1_w) { via_set_input_cb1(5, data); }
WRITE8_HANDLER( via_6_cb1_w) { via_set_input_cb1(6, data); }
WRITE8_HANDLER( via_7_cb1_w) { via_set_input_cb1(7, data); }
WRITE8_HANDLER( via_0_cb2_w) { via_set_input_cb2(0, data); }
WRITE8_HANDLER( via_1_cb2_w) { via_set_input_cb2(1, data); }
WRITE8_HANDLER( via_2_cb2_w) { via_set_input_cb2(2, data); }
WRITE8_HANDLER( via_3_cb2_w) { via_set_input_cb2(3, data); }
WRITE8_HANDLER( via_4_cb2_w) { via_set_input_cb2(4, data); }
WRITE8_HANDLER( via_5_cb2_w) { via_set_input_cb2(5, data); }
WRITE8_HANDLER( via_6_cb2_w) { via_set_input_cb2(6, data); }
WRITE8_HANDLER( via_7_cb2_w) { via_set_input_cb2(7, data); }

READ8_HANDLER( via_0_ca1_r) { return via[0].in_ca1; }
READ8_HANDLER( via_1_ca1_r) { return via[1].in_ca1; }
READ8_HANDLER( via_2_ca1_r) { return via[2].in_ca1; }
READ8_HANDLER( via_3_ca1_r) { return via[3].in_ca1; }
READ8_HANDLER( via_4_ca1_r) { return via[4].in_ca1; }
READ8_HANDLER( via_5_ca1_r) { return via[5].in_ca1; }
READ8_HANDLER( via_6_ca1_r) { return via[6].in_ca1; }
READ8_HANDLER( via_7_ca1_r) { return via[7].in_ca1; }
READ8_HANDLER( via_0_ca2_r) { return via[0].in_ca2; }
READ8_HANDLER( via_1_ca2_r) { return via[1].in_ca2; }
READ8_HANDLER( via_2_ca2_r) { return via[2].in_ca2; }
READ8_HANDLER( via_3_ca2_r) { return via[3].in_ca2; }
READ8_HANDLER( via_4_ca2_r) { return via[4].in_ca2; }
READ8_HANDLER( via_5_ca2_r) { return via[5].in_ca2; }
READ8_HANDLER( via_6_ca2_r) { return via[6].in_ca2; }
READ8_HANDLER( via_7_ca2_r) { return via[7].in_ca2; }

READ8_HANDLER( via_0_cb1_r) { return via[0].in_cb1; }
READ8_HANDLER( via_1_cb1_r) { return via[1].in_cb1; }
READ8_HANDLER( via_2_cb1_r) { return via[2].in_cb1; }
READ8_HANDLER( via_3_cb1_r) { return via[3].in_cb1; }
READ8_HANDLER( via_4_cb1_r) { return via[4].in_cb1; }
READ8_HANDLER( via_5_cb1_r) { return via[5].in_cb1; }
READ8_HANDLER( via_6_cb1_r) { return via[6].in_cb1; }
READ8_HANDLER( via_7_cb1_r) { return via[7].in_cb1; }
READ8_HANDLER( via_0_cb2_r) { return via[0].in_cb2; }
READ8_HANDLER( via_1_cb2_r) { return via[1].in_cb2; }
READ8_HANDLER( via_2_cb2_r) { return via[2].in_cb2; }
READ8_HANDLER( via_3_cb2_r) { return via[3].in_cb2; }
READ8_HANDLER( via_4_cb2_r) { return via[4].in_cb2; }
READ8_HANDLER( via_5_cb2_r) { return via[5].in_cb2; }
READ8_HANDLER( via_6_cb2_r) { return via[6].in_cb2; }
READ8_HANDLER( via_7_cb2_r) { return via[7].in_cb2; }


#undef TRACE_VIA
