#include <math.h>
#include "sndintrf.h"
#include "deprecat.h"
#include "streams.h"
#include "2203intf.h"
#include "fm.h"


struct ym2203_info
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	void *			psg;
	const ym2203_interface *intf;
};


static void psg_set_clock(void *param, int clock)
{
	struct ym2203_info *info = param;
	ay8910_set_clock_ym(info->psg, clock);
}

static void psg_write(void *param, int address, int data)
{
	struct ym2203_info *info = param;
	ay8910_write_ym(info->psg, address, data);
}

static int psg_read(void *param)
{
	struct ym2203_info *info = param;
	return ay8910_read_ym(info->psg);
}

static void psg_reset(void *param)
{
	struct ym2203_info *info = param;
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
	struct ym2203_info *info = param;
	if(info->intf->handler) info->intf->handler(Machine, irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_2203_0 )
{
	struct ym2203_info *info = ptr;
	ym2203_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_2203_1 )
{
	struct ym2203_info *info = ptr;
	ym2203_timer_over(info->chip,1);
}

/* update request from fm.c */
void ym2203_update_request(void *param)
{
	struct ym2203_info *info = param;
	stream_update(info->stream);
}


static void timer_handler(void *param,int c,int count,int clock)
{
	struct ym2203_info *info = param;
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

static void ym2203_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	struct ym2203_info *info = param;
	ym2203_update_one(info->chip, buffer[0], length);
}


static STATE_POSTLOAD( ym2203_intf_postload )
{
	struct ym2203_info *info = param;
	ym2203_postload(info->chip);
}


static SND_START( ym2203 )
{
	static const ym2203_interface generic_2203 =
	{
		{
			AY8910_LEGACY_OUTPUT,
			AY8910_DEFAULT_LOADS,
			NULL, NULL, NULL, NULL
		},
		NULL
	};
	const ym2203_interface *intf = config ? config : &generic_2203;
	struct ym2203_info *info;
	int rate = clock/72; /* ??? */

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = intf;
	info->psg = ay8910_start_ym(SOUND_YM2203, tag, clock, &intf->ay8910_intf);
	if (!info->psg) return NULL;

	/* Timer Handler set */
	info->timer[0] = timer_alloc(Machine, timer_callback_2203_0, info);
	info->timer[1] = timer_alloc(Machine, timer_callback_2203_1, info);

	/* stream system initialize */
	info->stream = stream_create(0,1,rate,info,ym2203_stream_update);

	/* Initialize FM emurator */
	info->chip = ym2203_init(info,tag,clock,rate,timer_handler,IRQHandler,&psgintf);

	state_save_register_postload(Machine, ym2203_intf_postload, info);

	if (info->chip)
		return info;

	/* error */
	/* stream close */
	return NULL;
}

static SND_STOP( ym2203 )
{
	struct ym2203_info *info = token;
	ym2203_shutdown(info->chip);
	ay8910_stop_ym(info->psg);
}

static SND_RESET( ym2203 )
{
	struct ym2203_info *info = token;
	ym2203_reset_chip(info->chip);
}



READ8_HANDLER( ym2203_status_port_0_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 0); return ym2203_read(info->chip,0); }
READ8_HANDLER( ym2203_status_port_1_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 1); return ym2203_read(info->chip,0); }
READ8_HANDLER( ym2203_status_port_2_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 2); return ym2203_read(info->chip,0); }
READ8_HANDLER( ym2203_status_port_3_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 3); return ym2203_read(info->chip,0); }
READ8_HANDLER( ym2203_status_port_4_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 4); return ym2203_read(info->chip,0); }

READ8_HANDLER( ym2203_read_port_0_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 0); return ym2203_read(info->chip,1); }
READ8_HANDLER( ym2203_read_port_1_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 1); return ym2203_read(info->chip,1); }
READ8_HANDLER( ym2203_read_port_2_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 2); return ym2203_read(info->chip,1); }
READ8_HANDLER( ym2203_read_port_3_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 3); return ym2203_read(info->chip,1); }
READ8_HANDLER( ym2203_read_port_4_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 4); return ym2203_read(info->chip,1); }

WRITE8_HANDLER( ym2203_control_port_0_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 0);
	ym2203_write(info->chip,0,data);
}
WRITE8_HANDLER( ym2203_control_port_1_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 1);
	ym2203_write(info->chip,0,data);
}
WRITE8_HANDLER( ym2203_control_port_2_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 2);
	ym2203_write(info->chip,0,data);
}
WRITE8_HANDLER( ym2203_control_port_3_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 3);
	ym2203_write(info->chip,0,data);
}
WRITE8_HANDLER( ym2203_control_port_4_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 4);
	ym2203_write(info->chip,0,data);
}

WRITE8_HANDLER( ym2203_write_port_0_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 0);
	ym2203_write(info->chip,1,data);
}
WRITE8_HANDLER( ym2203_write_port_1_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 1);
	ym2203_write(info->chip,1,data);
}
WRITE8_HANDLER( ym2203_write_port_2_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 2);
	ym2203_write(info->chip,1,data);
}
WRITE8_HANDLER( ym2203_write_port_3_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 3);
	ym2203_write(info->chip,1,data);
}
WRITE8_HANDLER( ym2203_write_port_4_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 4);
	ym2203_write(info->chip,1,data);
}


READ16_HANDLER( ym2203_status_port_0_lsb_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 0); return ym2203_read(info->chip,0) | 0xff00; }
READ16_HANDLER( ym2203_status_port_1_lsb_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 1); return ym2203_read(info->chip,0) | 0xff00; }
READ16_HANDLER( ym2203_status_port_2_lsb_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 2); return ym2203_read(info->chip,0) | 0xff00; }
READ16_HANDLER( ym2203_status_port_3_lsb_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 3); return ym2203_read(info->chip,0) | 0xff00; }
READ16_HANDLER( ym2203_status_port_4_lsb_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 4); return ym2203_read(info->chip,0) | 0xff00; }

READ16_HANDLER( ym2203_read_port_0_lsb_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 0); return ym2203_read(info->chip,1) | 0xff00; }
READ16_HANDLER( ym2203_read_port_1_lsb_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 1); return ym2203_read(info->chip,1) | 0xff00; }
READ16_HANDLER( ym2203_read_port_2_lsb_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 2); return ym2203_read(info->chip,1) | 0xff00; }
READ16_HANDLER( ym2203_read_port_3_lsb_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 3); return ym2203_read(info->chip,1) | 0xff00; }
READ16_HANDLER( ym2203_read_port_4_lsb_r ) { struct ym2203_info *info = sndti_token(SOUND_YM2203, 4); return ym2203_read(info->chip,1) | 0xff00; }

WRITE16_HANDLER( ym2203_control_port_0_lsb_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 0);
	if (ACCESSING_BITS_0_7)
		ym2203_write(info->chip,0,data);
}
WRITE16_HANDLER( ym2203_control_port_1_lsb_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 1);
	if (ACCESSING_BITS_0_7)
		ym2203_write(info->chip,0,data);
}
WRITE16_HANDLER( ym2203_control_port_2_lsb_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 2);
	if (ACCESSING_BITS_0_7)
		ym2203_write(info->chip,0,data);
}
WRITE16_HANDLER( ym2203_control_port_3_lsb_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 3);
	if (ACCESSING_BITS_0_7)
		ym2203_write(info->chip,0,data);
}
WRITE16_HANDLER( ym2203_control_port_4_lsb_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 4);
	if (ACCESSING_BITS_0_7)
		ym2203_write(info->chip,0,data);
}

WRITE16_HANDLER( ym2203_write_port_0_lsb_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 0);
	if (ACCESSING_BITS_0_7)
		ym2203_write(info->chip,1,data);
}
WRITE16_HANDLER( ym2203_write_port_1_lsb_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 1);
	if (ACCESSING_BITS_0_7)
		ym2203_write(info->chip,1,data);
}
WRITE16_HANDLER( ym2203_write_port_2_lsb_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 2);
	if (ACCESSING_BITS_0_7)
		ym2203_write(info->chip,1,data);
}
WRITE16_HANDLER( ym2203_write_port_3_lsb_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 3);
	if (ACCESSING_BITS_0_7)
		ym2203_write(info->chip,1,data);
}
WRITE16_HANDLER( ym2203_write_port_4_lsb_w )
{
	struct ym2203_info *info = sndti_token(SOUND_YM2203, 4);
	if (ACCESSING_BITS_0_7)
		ym2203_write(info->chip,1,data);
}


WRITE8_HANDLER( ym2203_word_0_w )
{
	if (offset)
		ym2203_write_port_0_w(space,0,data);
	else
		ym2203_control_port_0_w(space,0,data);
}

WRITE8_HANDLER( ym2203_word_1_w )
{
	if (offset)
		ym2203_write_port_1_w(space,0,data);
	else
		ym2203_control_port_1_w(space,0,data);
}





/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( ym2203 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( ym2203 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( ym2203 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( ym2203 );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( ym2203 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( ym2203 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YM2203";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
