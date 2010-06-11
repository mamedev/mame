/*
  fairchild f3853 smi static ram interface
  with integrated interrupt controller and timer

  databook found at www.freetradezone.com
*/

#include "emu.h"
#include "f3853.h"

/*
  the smi does not have DC0 and DC1, only DC0
  it is not reacting to the cpus DC0/DC1 swap instruction
  --> might lead to 2 devices having reacting to the same DC0 address
  and placing their bytes to the databus!
*/


typedef struct _f3853 f3853_t;
struct _f3853 {
    const f3853_config *config;

    UINT8 high,low; // bit 7 set to 0 for timer interrupt, to 1 for external interrupt
    int external_enable;
    int timer_enable;

    int request_flipflop;

    int priority_line; /* inverted level*/
    int external_interrupt_line;/* inverted level */

    emu_timer *timer;
};

/*
   8 bit shift register
   feedback in0 = not ( (out3 xor out4) xor (out5 xor out7) )
   interrupt at 0xfe
   0xff stops register (0xfe never reached!)
*/
static UINT8 f3853_value_to_cycle[0x100];


static TIMER_CALLBACK( f3853_timer_callback );
#define INTERRUPT_VECTOR(external) ( external ? f3853->low | ( f3853->high << 8 ) | 0x80 \
					: ( f3853->low | ( f3853->high << 8 ) ) & ~0x80 )


INLINE f3853_t *get_safe_token(running_device *device)
{
	assert( device != NULL );
	assert( device->type() == F3853 );
	return ( f3853_t * ) downcast<legacy_device_base *>(device)->token();
}


static void f3853_set_interrupt_request_line(running_device *device)
{
	f3853_t	*f3853 = get_safe_token( device );

    if ( ! f3853->config->interrupt_request )
		return;

	if ( f3853->external_enable && ! f3853->priority_line )
		f3853->config->interrupt_request(device, INTERRUPT_VECTOR(TRUE), TRUE);
	else if ( f3853->timer_enable && ! f3853->priority_line && f3853->request_flipflop)
		f3853->config->interrupt_request(device, INTERRUPT_VECTOR(FALSE), TRUE);
	else
		f3853->config->interrupt_request(device, 0, FALSE);
}


static void f3853_timer_start(running_device *device, UINT8 value)
{
	f3853_t	*f3853 = get_safe_token( device );

	attotime period = (value != 0xff) ? attotime_mul(ATTOTIME_IN_HZ(device->clock()), f3853_value_to_cycle[value]*31) : attotime_never;

	timer_adjust_oneshot(f3853->timer, period, 0);
}


static TIMER_CALLBACK( f3853_timer_callback )
{
	running_device *device = (running_device *)ptr;
	f3853_t	*f3853 = get_safe_token( device );

    if (f3853->timer_enable)
	{
		f3853->request_flipflop = TRUE;
		f3853_set_interrupt_request_line( device );
    }
    f3853_timer_start( device, 0xfe);
}


void f3853_set_external_interrupt_in_line(running_device *device, int level)
{
	f3853_t	*f3853 = get_safe_token( device );

    if ( f3853->external_interrupt_line && ! level && f3853->external_enable)
	f3853->request_flipflop = TRUE;
    f3853->external_interrupt_line = level;
    f3853_set_interrupt_request_line( device );
}


void f3853_set_priority_in_line(running_device *device, int level)
{
	f3853_t	*f3853 = get_safe_token( device );

    f3853->priority_line = level;
    f3853_set_interrupt_request_line( device );
}


READ8_DEVICE_HANDLER(f3853_r)
{
	f3853_t	*f3853 = get_safe_token( device );
    UINT8 data=0;

    switch (offset)
	{
    case 0:
		data = f3853->high;
		break;
    case 1:
		data = f3853->low;
		break;
    case 2: // interrupt control; not readable
    case 3: // timer; not readable
		break;
    }
    return data;
}


WRITE8_DEVICE_HANDLER(f3853_w)
{
	f3853_t	*f3853 = get_safe_token( device );
	switch (offset)
	{
	case 0:
		f3853->high = data;
		break;
	case 1:
		f3853->low = data;
		break;
	case 2: //interrupt control
		f3853->external_enable = ((data&3)==1);
		f3853->timer_enable = ((data&3)==3);
		f3853_set_interrupt_request_line( device );
		break;
	case 3: //timer
		f3853->request_flipflop = FALSE;
		f3853_set_interrupt_request_line( device );
		f3853_timer_start( device, data );
		break;
	}
}


static DEVICE_START( f3853 )
{
	f3853_t *f3853 = get_safe_token( device );
	UINT8 reg=0xfe;
	int i;

	f3853->config = (const f3853_config *)device->baseconfig().static_config();

	for (i=254/*known to get 0xfe after 255 cycles*/; i>=0; i--)
	{
		int o7 = ( reg & 0x80 ) ? TRUE : FALSE;
		int o5 = ( reg & 0x20 ) ? TRUE : FALSE;
		int o4 = ( reg & 0x10 ) ? TRUE : FALSE;
		int o3 = ( reg & 8 ) ? TRUE : FALSE;
		f3853_value_to_cycle[reg]=i;
		reg<<=1;
		if (!((o7!=o5)!=(o4!=o3))) reg|=1;
	}

	f3853->timer = timer_alloc( device->machine, f3853_timer_callback, (void *)device );

	state_save_register_device_item( device, 0, f3853->high );
	state_save_register_device_item( device, 0, f3853->low );
	state_save_register_device_item( device, 0, f3853->external_enable );
	state_save_register_device_item( device, 0, f3853->timer_enable );
	state_save_register_device_item( device, 0, f3853->request_flipflop );
	state_save_register_device_item( device, 0, f3853->priority_line );
	state_save_register_device_item( device, 0, f3853->external_interrupt_line );
}


static DEVICE_RESET( f3853 )
{
	f3853_t	*f3853 = get_safe_token( device );

	f3853->high = 0;
	f3853->low = 0;
	f3853->external_enable = 0;
	f3853->timer_enable = 0;
	f3853->request_flipflop = 0;
	f3853->priority_line = FALSE;
	f3853->external_interrupt_line = TRUE;

	timer_enable( f3853->timer, 0 );
}


DEVICE_GET_INFO( f3853 )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = sizeof(f3853_t);						break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:		info->i = 0;									break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(f3853);			break;
		case DEVINFO_FCT_STOP:						/* nothing */									break;
		case DEVINFO_FCT_RESET:						info->reset = DEVICE_RESET_NAME(f3853);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "F3853");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "F8");							break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright the MAME and MESS Teams"); break;
	}
}


DEFINE_LEGACY_DEVICE(F3853, f3853);
