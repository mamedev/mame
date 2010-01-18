/***************************************************************************

  2608intf.c

  The YM2608 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "ay8910.h"
#include "2608intf.h"
#include "fm.h"

typedef struct _ym2608_state ym2608_state;
struct _ym2608_state
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	void *			psg;
	const ym2608_interface *intf;
	running_device *device;
};


INLINE ym2608_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_YM2608);
	return (ym2608_state *)device->token;
}



static void psg_set_clock(void *param, int clock)
{
	ym2608_state *info = (ym2608_state *)param;
	ay8910_set_clock_ym(info->psg, clock);
}

static void psg_write(void *param, int address, int data)
{
	ym2608_state *info = (ym2608_state *)param;
	ay8910_write_ym(info->psg, address, data);
}

static int psg_read(void *param)
{
	ym2608_state *info = (ym2608_state *)param;
	return ay8910_read_ym(info->psg);
}

static void psg_reset(void *param)
{
	ym2608_state *info = (ym2608_state *)param;
	ay8910_reset_ym(info->psg);
}

static const ssg_callbacks psgintf =
{
	psg_set_clock,
	psg_write,
	psg_read,
	psg_reset
};


/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	ym2608_state *info = (ym2608_state *)param;
	if(info->intf->handler) info->intf->handler(info->device, irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_2608_0 )
{
	ym2608_state *info = (ym2608_state *)ptr;
	ym2608_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_2608_1 )
{
	ym2608_state *info = (ym2608_state *)ptr;
	ym2608_timer_over(info->chip,1);
}

static void timer_handler(void *param,int c,int count,int clock)
{
	ym2608_state *info = (ym2608_state *)param;
	if( count == 0 )
	{	/* Reset FM Timer */
		timer_enable(info->timer[c], 0);
	}
	else
	{	/* Start FM Timer */
		attotime period = attotime_mul(ATTOTIME_IN_HZ(clock), count);
		if (!timer_enable(info->timer[c], 1))
			timer_adjust_oneshot(info->timer[c], period, 0);
	}
}

/* update request from fm.c */
void ym2608_update_request(void *param)
{
	ym2608_state *info = (ym2608_state *)param;
	stream_update(info->stream);
}

static STREAM_UPDATE( ym2608_stream_update )
{
	ym2608_state *info = (ym2608_state *)param;
	ym2608_update_one(info->chip, outputs, samples);
}


static STATE_POSTLOAD( ym2608_intf_postload )
{
	ym2608_state *info = (ym2608_state *)param;
	ym2608_postload(info->chip);
}


static DEVICE_START( ym2608 )
{
	static const ym2608_interface generic_2608 =
	{
		{
			AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT,
			AY8910_DEFAULT_LOADS,
			DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
		},
		NULL
	};
	const ym2608_interface *intf = device->baseconfig().static_config ? (const ym2608_interface *)device->baseconfig().static_config : &generic_2608;
	int rate = device->clock/72;
	void *pcmbufa;
	int  pcmsizea;

	ym2608_state *info = get_safe_token(device);

	info->intf = intf;
	info->device = device;

	/* FIXME: Force to use simgle output */
	info->psg = ay8910_start_ym(NULL, SOUND_YM2608, device, device->clock, &intf->ay8910_intf);
	assert_always(info->psg != NULL, "Error creating YM2608/AY8910 chip");

	/* Timer Handler set */
	info->timer[0] = timer_alloc(device->machine, timer_callback_2608_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_2608_1, info);

	/* stream system initialize */
	info->stream = stream_create(device,0,2,rate,info,ym2608_stream_update);
	/* setup adpcm buffers */
	pcmbufa  = *device->region;
	pcmsizea = device->region->bytes();

	/* initialize YM2608 */
	info->chip = ym2608_init(info,device,device->clock,rate,
		           pcmbufa,pcmsizea,
		           timer_handler,IRQHandler,&psgintf);
	assert_always(info->chip != NULL, "Error creating YM2608 chip");

	state_save_register_postload(device->machine, ym2608_intf_postload, info);
}

static DEVICE_STOP( ym2608 )
{
	ym2608_state *info = get_safe_token(device);
	ym2608_shutdown(info->chip);
	ay8910_stop_ym(info->psg);
}

static DEVICE_RESET( ym2608 )
{
	ym2608_state *info = get_safe_token(device);
	ym2608_reset_chip(info->chip);
}


READ8_DEVICE_HANDLER( ym2608_r )
{
	ym2608_state *info = get_safe_token(device);
	return ym2608_read(info->chip, offset & 3);
}

WRITE8_DEVICE_HANDLER( ym2608_w )
{
	ym2608_state *info = get_safe_token(device);
	ym2608_write(info->chip, offset & 3, data);
}

READ8_DEVICE_HANDLER( ym2608_read_port_r ) { return ym2608_r(device, 1); }
READ8_DEVICE_HANDLER( ym2608_status_port_a_r ) { return ym2608_r(device, 0); }
READ8_DEVICE_HANDLER( ym2608_status_port_b_r ) { return ym2608_r(device, 2); }

WRITE8_DEVICE_HANDLER( ym2608_control_port_a_w ) { ym2608_w(device, 0, data); }
WRITE8_DEVICE_HANDLER( ym2608_control_port_b_w ) { ym2608_w(device, 2, data); }
WRITE8_DEVICE_HANDLER( ym2608_data_port_a_w ) { ym2608_w(device, 1, data); }
WRITE8_DEVICE_HANDLER( ym2608_data_port_b_w ) { ym2608_w(device, 3, data); }



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( ym2608 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ym2608_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( ym2608 );				break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( ym2608 );				break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( ym2608 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "YM2608");							break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
