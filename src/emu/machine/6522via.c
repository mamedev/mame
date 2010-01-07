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


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define TRACE_VIA		0


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/******************* internal VIA data structure *******************/

typedef struct _via6522_t via6522_t;
struct _via6522_t
{
	devcb_resolved_read8 in_a_func;
	devcb_resolved_read8 in_b_func;
	devcb_resolved_read_line in_ca1_func;
	devcb_resolved_read_line in_cb1_func;
	devcb_resolved_read_line in_ca2_func;
	devcb_resolved_read_line in_cb2_func;
	devcb_resolved_write8 out_a_func;
	devcb_resolved_write8 out_b_func;
	devcb_resolved_write_line out_ca1_func;
	devcb_resolved_write_line out_cb1_func;
	devcb_resolved_write_line out_ca2_func;
	devcb_resolved_write_line out_cb2_func;
	devcb_resolved_write_line irq_func;

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

	emu_timer *shift_timer;
	UINT8 shift_counter;
};


/***************************************************************************
    MACROS
***************************************************************************/

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

#define CLR_PA_INT(device)	via_clear_int (device, INT_CA1 | ((!CA2_IND_IRQ(get_token(device)->pcr)) ? INT_CA2: 0))
#define CLR_PB_INT(device)	via_clear_int (device, INT_CB1 | ((!CB2_IND_IRQ(get_token(device)->pcr)) ? INT_CB2: 0))

#define IFR_DELAY 3

#define TIMER1_VALUE(v) (v->t1ll+(v->t1lh<<8))
#define TIMER2_VALUE(v) (v->t2ll+(v->t2lh<<8))

/***************************************************************************
    PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK( via_shift_callback );
static TIMER_CALLBACK( via_t1_timeout );
static TIMER_CALLBACK( via_t2_timeout );


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE via6522_t *get_token(const device_config *device)
{
	assert(device != NULL);
	assert((device->type == VIA6522));
	return (via6522_t *) device->token;
}


INLINE const via6522_interface *get_interface(const device_config *device)
{
	assert(device != NULL);
	assert((device->type == VIA6522));
	return (const via6522_interface *) device->static_config;
}


INLINE attotime v_cycles_to_time(const device_config *device, int c)
{
	return attotime_mul(ATTOTIME_IN_HZ(device->clock), c);
}


INLINE UINT32 v_time_to_cycles(const device_config *device, attotime t)
{
	return attotime_to_double(attotime_mul(t, device->clock));
}


INLINE UINT16 v_get_counter1_value(const device_config *device)
{
	via6522_t *v = get_token(device);
	UINT16 val;

	if (v->t1_active) {
		val = v_time_to_cycles(device, timer_timeleft(v->t1)) - IFR_DELAY;
	} else {
		val = 0xFFFF - v_time_to_cycles(device, attotime_sub(timer_get_time(device->machine), v->time1));
	}
	return val;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    DEVICE_START( via6522 )
-------------------------------------------------*/

static DEVICE_START( via6522 )
{
	via6522_t *v = get_token(device);
	const via6522_interface *intf = get_interface(device);

	memset(v, 0, sizeof(*v));

	devcb_resolve_read8(&v->in_a_func, &intf->in_a_func, device);
	devcb_resolve_read8(&v->in_b_func, &intf->in_b_func, device);
	devcb_resolve_read_line(&v->in_ca1_func, &intf->in_ca1_func, device);
	devcb_resolve_read_line(&v->in_cb1_func, &intf->in_cb1_func, device);
	devcb_resolve_read_line(&v->in_ca2_func, &intf->in_ca2_func, device);
	devcb_resolve_read_line(&v->in_cb2_func, &intf->in_cb2_func, device);
	devcb_resolve_write8(&v->out_a_func, &intf->out_a_func, device);
	devcb_resolve_write8(&v->out_b_func, &intf->out_b_func, device);
	devcb_resolve_write_line(&v->out_ca1_func, &intf->out_ca1_func, device);
	devcb_resolve_write_line(&v->out_cb1_func, &intf->out_cb1_func, device);
	devcb_resolve_write_line(&v->out_ca2_func, &intf->out_ca2_func, device);
	devcb_resolve_write_line(&v->out_cb2_func, &intf->out_cb2_func, device);
	devcb_resolve_write_line(&v->irq_func, &intf->irq_func, device);

	v->t1ll = 0xf3; /* via at 0x9110 in vic20 show these values */
	v->t1lh = 0xb5; /* ports are not written by kernel! */
	v->t2ll = 0xff; /* taken from vice */
	v->t2lh = 0xff;
	v->time2 = v->time1 = timer_get_time(device->machine);
	v->t1 = timer_alloc(device->machine, via_t1_timeout, (void *) device);
	v->t2 = timer_alloc(device->machine, via_t2_timeout, (void *) device);
	v->shift_timer = timer_alloc(device->machine, via_shift_callback, (void *) device);

	/* Default clock is from CPU1 */
	if (device->clock == 0)
		device_set_clock(device, device->machine->firstcpu->clock);

	/* save state register */
	state_save_register_device_item(device, 0, v->in_a);
	state_save_register_device_item(device, 0, v->in_ca1);
	state_save_register_device_item(device, 0, v->in_ca2);
	state_save_register_device_item(device, 0, v->out_a);
	state_save_register_device_item(device, 0, v->out_ca2);
	state_save_register_device_item(device, 0, v->ddr_a);
	state_save_register_device_item(device, 0, v->in_b);
	state_save_register_device_item(device, 0, v->in_cb1);
	state_save_register_device_item(device, 0, v->in_cb2);
	state_save_register_device_item(device, 0, v->out_b);
	state_save_register_device_item(device, 0, v->out_cb2);
	state_save_register_device_item(device, 0, v->ddr_b);
	state_save_register_device_item(device, 0, v->t1cl);
	state_save_register_device_item(device, 0, v->t1ch);
	state_save_register_device_item(device, 0, v->t1ll);
	state_save_register_device_item(device, 0, v->t1lh);
	state_save_register_device_item(device, 0, v->t2cl);
	state_save_register_device_item(device, 0, v->t2ch);
	state_save_register_device_item(device, 0, v->t2ll);
	state_save_register_device_item(device, 0, v->t2lh);
	state_save_register_device_item(device, 0, v->sr);
	state_save_register_device_item(device, 0, v->pcr);
	state_save_register_device_item(device, 0, v->acr);
	state_save_register_device_item(device, 0, v->ier);
	state_save_register_device_item(device, 0, v->ifr);
	state_save_register_device_item(device, 0, v->t1_active);
	state_save_register_device_item(device, 0, v->t2_active);
	state_save_register_device_item(device, 0, v->shift_counter);
}


/*-------------------------------------------------
    via_set_int - external interrupt check
-------------------------------------------------*/

static void via_set_int (const device_config *device, int data)
{
	via6522_t *v = get_token(device);

	v->ifr |= data;
	if (TRACE_VIA)
		logerror("%s:6522VIA chip %s: IFR = %02X\n", cpuexec_describe_context(device->machine), device->tag, v->ifr);

	if (v->ier & v->ifr)
    {
		v->ifr |= INT_ANY;
		devcb_call_write_line(&v->irq_func, ASSERT_LINE);
    }
}


/*-------------------------------------------------
    via_clear_int - external interrupt check
-------------------------------------------------*/

static void via_clear_int (const device_config *device, int data)
{
	via6522_t *v = get_token(device);

	v->ifr = (v->ifr & ~data) & 0x7f;

	if (TRACE_VIA)
		logerror("%s:6522VIA chip %s: IFR = %02X\n", cpuexec_describe_context(device->machine), device->tag, v->ifr);

	if (v->ifr & v->ier)
		v->ifr |= INT_ANY;
	else
	{
		devcb_call_write_line(&v->irq_func, CLEAR_LINE);
	}
}


/*-------------------------------------------------
    via_shift
-------------------------------------------------*/

static void via_shift(const device_config *device)
{
	via6522_t *v = get_token(device);

	if (SO_O2_CONTROL(v->acr))
	{
		v->out_cb2 = (v->sr >> 7) & 1;
		v->sr =  (v->sr << 1) | v->out_cb2;

		devcb_call_write_line(&v->out_cb2_func, v->out_cb2);

		v->in_cb1=1;

		/* this should be one cycle wide */
		devcb_call_write_line(&v->out_cb1_func, 0);
		devcb_call_write_line(&v->out_cb1_func, 1);

		v->shift_counter = (v->shift_counter + 1) % 8;

		if (v->shift_counter)
			timer_adjust_oneshot(v->shift_timer, v_cycles_to_time(device, 2), 0);
		else
		{
			if (!(v->ifr & INT_SR))
				via_set_int(device, INT_SR);
		}
	}
	if (SO_EXT_CONTROL(v->acr))
	{
		v->out_cb2 = (v->sr >> 7) & 1;
		v->sr =  (v->sr << 1) | v->out_cb2;

		devcb_call_write_line(&v->out_cb2_func, v->out_cb2);

		v->shift_counter = (v->shift_counter + 1) % 8;

		if (v->shift_counter == 0)
		{
			if (!(v->ifr & INT_SR))
				via_set_int(device, INT_SR);
		}
	}
	if (SI_EXT_CONTROL(v->acr))
	{
		if (v->in_cb2_func.read != NULL)
			v->in_cb2 = devcb_call_read_line(&v->in_cb2_func);

		v->sr =  (v->sr << 1) | (v->in_cb2 & 1);

		v->shift_counter = (v->shift_counter + 1) % 8;

		if (v->shift_counter == 0)
		{
			if (!(v->ifr & INT_SR))
			{
				via_set_int(device, INT_SR);
			}
		}
	}
}


/*-------------------------------------------------
    TIMER_CALLBACK( via_shift_callback )
-------------------------------------------------*/

static TIMER_CALLBACK( via_shift_callback )
{
	const device_config *device = (const device_config *)ptr;
	via_shift(device);
}


/*-------------------------------------------------
    TIMER_CALLBACK( via_t1_timeout )
-------------------------------------------------*/

static TIMER_CALLBACK( via_t1_timeout )
{
	const device_config *device = (const device_config *)ptr;
	via6522_t *v = get_token(device);

	if (T1_CONTINUOUS (v->acr))
    {
		if (T1_SET_PB7(v->acr))
			v->out_b ^= 0x80;
		timer_adjust_oneshot(v->t1, v_cycles_to_time(device, TIMER1_VALUE(v) + IFR_DELAY), 0);
    }
	else
    {
		if (T1_SET_PB7(v->acr))
			v->out_b |= 0x80;
		v->t1_active = 0;
		v->time1 = timer_get_time(device->machine);
    }
	if (v->ddr_b)
	{
		UINT8 write_data = (v->out_b & v->ddr_b) | (v->ddr_b ^ 0xff);
		devcb_call_write8(&v->out_b_func, 0, write_data);
	}

	if (!(v->ifr & INT_T1))
		via_set_int (device, INT_T1);
}


/*-------------------------------------------------
    TIMER_CALLBACK( via_t2_timeout )
-------------------------------------------------*/

static TIMER_CALLBACK( via_t2_timeout )
{
	const device_config *device = (const device_config *)ptr;
	via6522_t *v = get_token(device);

	v->t2_active = 0;
	v->time2 = timer_get_time(device->machine);

	if (!(v->ifr & INT_T2))
		via_set_int (device, INT_T2);
}


/*-------------------------------------------------
    DEVICE_RESET( via6522 )
-------------------------------------------------*/

static DEVICE_RESET( via6522 )
{
	via6522_t *v = get_token(device);
	v->in_a = 0;
	v->in_ca1 = 0;
	v->in_ca2 = 0;
	v->out_a = 0;
	v->out_ca2 = 0;
	v->ddr_a = 0;
	v->in_b = 0;
	v->in_cb1 = 0;
	v->in_cb2 = 0;
	v->out_b = 0;
	v->out_cb2 = 0;
	v->ddr_b = 0;

	v->t1cl = 0;
	v->t1ch = 0;
	v->t2cl = 0;
	v->t2ch = 0;

	v->sr = 0;
	v->pcr = 0;
	v->acr = 0;
	v->ier = 0;
	v->ifr = 0;
	v->t1_active = 0;
	v->t2_active = 0;
	v->shift_counter = 0;
}


/*-------------------------------------------------
    via_r - CPU interface for VIA read
-------------------------------------------------*/

READ8_DEVICE_HANDLER(via_r)
{
	via6522_t *v = get_token(device);
	int val = 0;

	offset &= 0xf;

	switch (offset)
    {
    case VIA_PB:
		/* update the input */
		if (PB_LATCH_ENABLE(v->acr) == 0)
		{
			if (v->in_b_func.read != NULL)
				v->in_b = devcb_call_read8(&v->in_b_func, 0);
			else
				logerror("%s:6522VIA chip %s: Port B is being read but has no handler\n", cpuexec_describe_context(device->machine), device->tag);
		}

		CLR_PB_INT(device);

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
			if (v->in_a_func.read != NULL)
				v->in_a = devcb_call_read8(&v->in_a_func, 0);
			else
				logerror("%s:6522VIA chip %s: Port A is being read but has no handler\n", cpuexec_describe_context(device->machine), device->tag);
		}

		/* combine input and output values */
		val = (v->out_a & v->ddr_a) + (v->in_a & ~v->ddr_a);

		CLR_PA_INT(device);

		/* If CA2 is configured as output and in pulse or handshake mode,
           CA2 is set now */
		if (CA2_AUTO_HS(v->pcr))
		{
			if (v->out_ca2)
			{
				/* set CA2 */
				v->out_ca2 = 0;

				/* call the CA2 output function */
				devcb_call_write_line(&v->out_ca2_func, 0);
			}
		}

		break;

    case VIA_PANH:
		/* update the input */
		if (PA_LATCH_ENABLE(v->acr) == 0)
		{
			if (v->in_a_func.read != NULL)
				v->in_a = devcb_call_read8(&v->in_a_func, 0);
			else
				logerror("%s:6522VIA chip %s: Port A is being read but has no handler\n", cpuexec_describe_context(device->machine), device->tag);
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
		via_clear_int (device, INT_T1);
		val = v_get_counter1_value(device) & 0xFF;
		break;

    case VIA_T1CH:
		val = v_get_counter1_value(device) >> 8;
		break;

    case VIA_T1LL:
		val = v->t1ll;
		break;

    case VIA_T1LH:
		val = v->t1lh;
		break;

    case VIA_T2CL:
		via_clear_int (device, INT_T2);
		if (v->t2_active)
			val = v_time_to_cycles(device, timer_timeleft(v->t2)) & 0xff;
		else
		{
			if (T2_COUNT_PB6(v->acr))
				val = v->t2cl;
			else
				val = (0x10000- (v_time_to_cycles(device, attotime_sub(timer_get_time(device->machine), v->time2)) & 0xffff) - 1) & 0xff;
		}
		break;

    case VIA_T2CH:
		if (v->t2_active)
			val = v_time_to_cycles(device, timer_timeleft(v->t2)) >> 8;
		else
		{
			if (T2_COUNT_PB6(v->acr))
				val = v->t2ch;
			else
				val = (0x10000- (v_time_to_cycles(device, attotime_sub(timer_get_time(device->machine), v->time2)) & 0xffff) - 1) >> 8;
		}
		break;

    case VIA_SR:
		val = v->sr;
		via_clear_int(device, INT_SR);
		if (SO_O2_CONTROL(v->acr))
		{
			v->shift_counter=0;
			timer_adjust_oneshot(v->shift_timer, v_cycles_to_time(device, 2), 0);
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


/*-------------------------------------------------
    via_w - CPU interface for VIA write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER(via_w)
{
	via6522_t *v = get_token(device);

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
			devcb_call_write8(&v->out_b_func, 0, write_data);
		}

		CLR_PB_INT(device);

		/* If CB2 is configured as output and in pulse or handshake mode,
           CB2 is set now */
		if (CB2_AUTO_HS(v->pcr))
		{
			if (v->out_cb2)
			{
				/* set CB2 */
				v->out_cb2 = 0;

				/* call the CB2 output function */
				devcb_call_write_line(&v->out_cb2_func, 0);
			}
		}
		break;

    case VIA_PA:
		v->out_a = data;

		if (v->ddr_a)
		{
			UINT8 write_data = (v->out_a & v->ddr_a) | (v->ddr_a ^ 0xff);
			devcb_call_write8(&v->out_a_func, 0, write_data);
		}

		CLR_PA_INT(device);

		/* If CA2 is configured as output and in pulse or handshake mode,
           CA2 is set now */
		if (CA2_PULSE_OUTPUT(v->pcr))
		{
			/* call the CA2 output function */
			devcb_call_write_line(&v->out_ca2_func, 0);
			devcb_call_write_line(&v->out_ca2_func, 1);

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
				devcb_call_write_line(&v->out_ca2_func, 0);
			}
		}

		break;

    case VIA_PANH:
		v->out_a = data;

		if (v->ddr_a)
		{
			UINT8 write_data = (v->out_a & v->ddr_a) | (v->ddr_a ^ 0xff);
			devcb_call_write8(&v->out_a_func, 0, write_data);
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
				devcb_call_write8(&v->out_b_func, 0, write_data);
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
				devcb_call_write8(&v->out_a_func, 0, write_data);
			}
		}
		break;

    case VIA_T1CL:
    case VIA_T1LL:
		v->t1ll = data;
		break;

	case VIA_T1LH:
	    v->t1lh = data;
	    via_clear_int (device, INT_T1);
	    break;

    case VIA_T1CH:
		v->t1ch = v->t1lh = data;
		v->t1cl = v->t1ll;

		via_clear_int (device, INT_T1);

		if (T1_SET_PB7(v->acr))
		{
			v->out_b &= 0x7f;

			//if (v->ddr_b)
			{
				UINT8 write_data = (v->out_b & v->ddr_b) | (v->ddr_b ^ 0xff);
				devcb_call_write8(&v->out_b_func, 0, write_data);
			}
		}
		timer_adjust_oneshot(v->t1, v_cycles_to_time(device, TIMER1_VALUE(v) + IFR_DELAY), 0);
		v->t1_active = 1;
		break;

    case VIA_T2CL:
		v->t2ll = data;
		break;

    case VIA_T2CH:
		v->t2ch = v->t2lh = data;
		v->t2cl = v->t2ll;

		via_clear_int (device, INT_T2);

		if (!T2_COUNT_PB6(v->acr))
		{
			timer_adjust_oneshot(v->t2, v_cycles_to_time(device, TIMER2_VALUE(v) + IFR_DELAY), 0);
			v->t2_active = 1;
		}
		else
		{
			v->time2 = timer_get_time(device->machine);
		}
		break;

    case VIA_SR:
		v->sr = data;
		v->shift_counter=0;
		via_clear_int(device, INT_SR);
		if (SO_O2_CONTROL(v->acr))
		{
			timer_set(device->machine, v_cycles_to_time(device, 2), (void *) device, 0, via_shift_callback);
		}
		break;

    case VIA_PCR:
		v->pcr = data;

		if (TRACE_VIA)
			logerror("%s:6522VIA chip %s: PCR = %02X\n", cpuexec_describe_context(device->machine), device->tag, data);

		if (CA2_FIX_OUTPUT(data) && CA2_OUTPUT_LEVEL(data) ^ v->out_ca2)
		{
			v->out_ca2 = CA2_OUTPUT_LEVEL(data);
			devcb_call_write_line(&v->out_ca2_func, v->out_ca2);
		}

		if (CB2_FIX_OUTPUT(data) && CB2_OUTPUT_LEVEL(data) ^ v->out_cb2)
		{
			v->out_cb2 = CB2_OUTPUT_LEVEL(data);
			devcb_call_write_line(&v->out_cb2_func, v->out_cb2);
		}
		break;

    case VIA_ACR:
		{
			UINT16 counter1 = v_get_counter1_value(device);
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
					devcb_call_write8(&v->out_b_func, 0, write_data);
				}
			}
			if (T1_CONTINUOUS(data))
			{
				timer_adjust_oneshot(v->t1, v_cycles_to_time(device, counter1 + IFR_DELAY), 0);
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
				devcb_call_write_line(&v->irq_func, CLEAR_LINE);
			}
		}
		else
		{
			if ((v->ier & v->ifr) & 0x7f)
			{
				v->ifr |= INT_ANY;
				devcb_call_write_line(&v->irq_func, ASSERT_LINE);
			}
		}
		break;

	case VIA_IFR:
		if (data & INT_ANY)
			data = 0x7f;
		via_clear_int (device, data);
		break;
    }
}


/*-------------------------------------------------
    via_porta_w - interface setting VIA port
    A input
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER(via_porta_w)
{
	via6522_t *v = get_token(device);

	/* set the input, what could be easier? */
	v->in_a = data;
}


/*-------------------------------------------------
    via_ca1_r - interface retrieving VIA port
    CA1 input
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER(via_ca1_r)
{
	via6522_t *v = get_token(device);
	return v->in_ca1;
}


/*-------------------------------------------------
    via_ca1_w - interface setting VIA port
    CA1 input
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER(via_ca1_w)
{
	via6522_t *v = get_token(device);

	/* handle the active transition */
	if (state != v->in_ca1)
    {
		if (TRACE_VIA)
			logerror("%s:6522VIA chip %s: CA1 = %02X\n", cpuexec_describe_context(device->machine), device->tag, state);

		if ((CA1_LOW_TO_HIGH(v->pcr) && state) || (CA1_HIGH_TO_LOW(v->pcr) && !state))
		{
			if (PA_LATCH_ENABLE(v->acr))
			{
				if (v->in_a_func.read != NULL)
					v->in_a = devcb_call_read8(&v->in_a_func, 0);
				else
					logerror("%s:6522VIA chip %s: Port A is being read but has no handler\n", cpuexec_describe_context(device->machine), device->tag);
			}

			via_set_int (device, INT_CA1);

			/* CA2 is configured as output and in pulse or handshake mode,
               CA2 is cleared now */
			if (CA2_AUTO_HS(v->pcr))
			{
				if (!v->out_ca2)
				{
					/* clear CA2 */
					v->out_ca2 = 1;

					/* call the CA2 output function */
					devcb_call_write_line(&v->out_ca2_func, 1);
				}
			}
		}

		v->in_ca1 = state;
    }
}


/*-------------------------------------------------
    via_ca2_r - interface retrieving VIA port
    CA2 input
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER(via_ca2_r)
{
	via6522_t *v = get_token(device);
	return v->in_ca2;
}


/*-------------------------------------------------
    via_ca2_w - interface setting VIA port
    CA2 input
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER(via_ca2_w)
{
	via6522_t *v = get_token(device);

	/* CA2 is in input mode */
	if (CA2_INPUT(v->pcr))
    {
		/* the new state has caused a transition */
		if (v->in_ca2 != state)
		{
			/* handle the active transition */
			if ((state && CA2_LOW_TO_HIGH(v->pcr)) || (!state && CA2_HIGH_TO_LOW(v->pcr)))
			{
				/* mark the IRQ */
				via_set_int (device, INT_CA2);
			}
			/* set the new value for CA2 */
			v->in_ca2 = state;
		}
    }


}


/*-------------------------------------------------
    via_portb_r - interface retrieving VIA port
    B input
-------------------------------------------------*/

READ8_DEVICE_HANDLER(via_portb_r)
{
	via6522_t *v = get_token(device);
	return v->in_b;
}


/*-------------------------------------------------
    via_portb_w - interface setting VIA port
    B input
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER(via_portb_w)
{
	via6522_t *v = get_token(device);

	/* set the input, what could be easier? */
	v->in_b = data;
}


/*-------------------------------------------------
    via_cb1_r - interface retrieving VIA port
    CB1 input
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER(via_cb1_r)
{
	via6522_t *v = get_token(device);
	return v->in_cb1;
}


/*-------------------------------------------------
    via_cb1_w - interface setting VIA port
    CB1 input
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER(via_cb1_w)
{
	via6522_t *v = get_token(device);

	/* handle the active transition */
	if (state != v->in_cb1)
    {
		if ((CB1_LOW_TO_HIGH(v->pcr) && state) || (CB1_HIGH_TO_LOW(v->pcr) && !state))
		{
			if (PB_LATCH_ENABLE(v->acr))
			{
				if (v->in_b_func.read != NULL)
					v->in_b = devcb_call_read8(&v->in_b_func, 0);
				else
					logerror("%s:6522VIA chip %s: Port B is being read but has no handler\n", cpuexec_describe_context(device->machine), device->tag);
			}
			if (SO_EXT_CONTROL(v->acr) || SI_EXT_CONTROL(v->acr))
				via_shift (device);

			via_set_int (device, INT_CB1);

			/* CB2 is configured as output and in pulse or handshake mode,
               CB2 is cleared now */
			if (CB2_AUTO_HS(v->pcr))
			{
				if (!v->out_cb2)
				{
					/* clear CB2 */
					v->out_cb2 = 1;

					/* call the CB2 output function */
					devcb_call_write_line(&v->out_cb2_func, 1);
				}
			}
		}
		v->in_cb1 = state;
    }
}


/*-------------------------------------------------
    via_cb2_r - interface retrieving VIA port
    CB2 input
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER(via_cb2_r)
{
	via6522_t *v = get_token(device);
	return v->in_cb2;
}


/*-------------------------------------------------
    via_cb2_w - interface setting VIA port
    CB2 input
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER(via_cb2_w)
{
	via6522_t *v = get_token(device);

	/* CB2 is in input mode */
	if (CB2_INPUT(v->pcr))
    {
		/* the new state has caused a transition */
		if (v->in_cb2 != state)
		{
			/* handle the active transition */
			if ((state && CB2_LOW_TO_HIGH(v->pcr)) || (!state && CB2_HIGH_TO_LOW(v->pcr)))
			{
				/* mark the IRQ */
				via_set_int (device, INT_CB2);
			}
			/* set the new value for CB2 */
			v->in_cb2 = state;
		}
    }
}


/*-------------------------------------------------
    DEVICE_GET_INFO( via6522 )
-------------------------------------------------*/

DEVICE_GET_INFO(via6522)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(via6522_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(via6522);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(via6522);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "6522 VIA");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "6522 VIA");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						/* Nothing */								break;
	}
}
