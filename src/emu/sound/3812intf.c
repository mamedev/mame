/******************************************************************************
* FILE
*   Yamaha 3812 emulator interface - MAME VERSION
*
* CREATED BY
*   Ernesto Corvi
*
* UPDATE LOG
*   JB  28-04-2002  Fixed simultaneous usage of all three different chip types.
*                       Used real sample rate when resample filter is active.
*       AAT 12-28-2001  Protected Y8950 from accessing unmapped port and keyboard handlers.
*   CHS 1999-01-09  Fixes new ym3812 emulation interface.
*   CHS 1998-10-23  Mame streaming sound chip update
*   EC  1998        Created Interface
*
* NOTES
*
******************************************************************************/
#include "sndintrf.h"
#include "streams.h"
#include "cpuintrf.h"
#include "3812intf.h"
#include "fm.h"
#include "sound/fmopl.h"


#if BUILD_YM3812

struct ym3812_info
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	const ym3812_interface *intf;
	const device_config *device;
};

static void IRQHandler_3812(void *param,int irq)
{
	struct ym3812_info *info = param;
	if (info->intf->handler) (info->intf->handler)(info->device->machine, irq ? ASSERT_LINE : CLEAR_LINE);
}
static TIMER_CALLBACK( timer_callback_3812_0 )
{
	struct ym3812_info *info = ptr;
	ym3812_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_3812_1 )
{
	struct ym3812_info *info = ptr;
	ym3812_timer_over(info->chip,1);
}

static void TimerHandler_3812(void *param,int c,attotime period)
{
	struct ym3812_info *info = param;
	if( attotime_compare(period, attotime_zero) == 0 )
	{	/* Reset FM Timer */
		timer_enable(info->timer[c], 0);
	}
	else
	{	/* Start FM Timer */
		timer_adjust_oneshot(info->timer[c], period, 0);
	}
}


static STREAM_UPDATE( ym3812_stream_update )
{
	struct ym3812_info *info = param;
	ym3812_update_one(info->chip, outputs[0], samples);
}

static void _stream_update_3812(void * param, int interval)
{
	struct ym3812_info *info = param;
	stream_update(info->stream);
}


static SND_START( ym3812 )
{
	static const ym3812_interface dummy = { 0 };
	struct ym3812_info *info;
	int rate = clock/72;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = config ? config : &dummy;
	info->device = device;

	/* stream system initialize */
	info->chip = ym3812_init(device,clock,rate);
	if (!info->chip)
		return NULL;

	info->stream = stream_create(device,0,1,rate,info,ym3812_stream_update);

	/* YM3812 setup */
	ym3812_set_timer_handler (info->chip, TimerHandler_3812, info);
	ym3812_set_irq_handler   (info->chip, IRQHandler_3812, info);
	ym3812_set_update_handler(info->chip, _stream_update_3812, info);

	info->timer[0] = timer_alloc(device->machine, timer_callback_3812_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_3812_1, info);

	return info;
}

static SND_STOP( ym3812 )
{
	struct ym3812_info *info = device->token;
	ym3812_shutdown(info->chip);
}

static SND_RESET( ym3812 )
{
	struct ym3812_info *info = device->token;
	ym3812_reset_chip(info->chip);
}

WRITE8_HANDLER( ym3812_control_port_0_w ) {
	struct ym3812_info *info = sndti_token(SOUND_YM3812, 0);
	ym3812_write(info->chip, 0, data);
}
WRITE8_HANDLER( ym3812_write_port_0_w ) {
	struct ym3812_info *info = sndti_token(SOUND_YM3812, 0);
	ym3812_write(info->chip, 1, data);
}
READ8_HANDLER( ym3812_status_port_0_r ) {
	struct ym3812_info *info = sndti_token(SOUND_YM3812, 0);
	return ym3812_read(info->chip, 0);
}
READ8_HANDLER( ym3812_read_port_0_r ) {
	struct ym3812_info *info = sndti_token(SOUND_YM3812, 0);
	return ym3812_read(info->chip, 1);
}


WRITE8_HANDLER( ym3812_control_port_1_w ) {
	struct ym3812_info *info = sndti_token(SOUND_YM3812, 1);
	ym3812_write(info->chip, 0, data);
}
WRITE8_HANDLER( ym3812_write_port_1_w ) {
	struct ym3812_info *info = sndti_token(SOUND_YM3812, 1);
	ym3812_write(info->chip, 1, data);
}
READ8_HANDLER( ym3812_status_port_1_r ) {
	struct ym3812_info *info = sndti_token(SOUND_YM3812, 1);
	return ym3812_read(info->chip, 0);
}
READ8_HANDLER( ym3812_read_port_1_r ) {
	struct ym3812_info *info = sndti_token(SOUND_YM3812, 1);
	return ym3812_read(info->chip, 1);
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( ym3812 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( ym3812 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( ym3812 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( ym3812 );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( ym3812 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( ym3812 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "YM3812");							break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");								break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);							break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

#endif


#if BUILD_YM3526

struct ym3526_info
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	const ym3526_interface *intf;
	const device_config *device;
};


/* IRQ Handler */
static void IRQHandler_3526(void *param,int irq)
{
	struct ym3526_info *info = param;
	if (info->intf->handler) (info->intf->handler)(info->device->machine, irq ? ASSERT_LINE : CLEAR_LINE);
}
/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_3526_0 )
{
	struct ym3526_info *info = ptr;
	ym3526_timer_over(info->chip,0);
}
static TIMER_CALLBACK( timer_callback_3526_1 )
{
	struct ym3526_info *info = ptr;
	ym3526_timer_over(info->chip,1);
}
/* TimerHandler from fm.c */
static void TimerHandler_3526(void *param,int c,attotime period)
{
	struct ym3526_info *info = param;
	if( attotime_compare(period, attotime_zero) == 0 )
	{	/* Reset FM Timer */
		timer_enable(info->timer[c], 0);
	}
	else
	{	/* Start FM Timer */
		timer_adjust_oneshot(info->timer[c], period, 0);
	}
}


static STREAM_UPDATE( ym3526_stream_update )
{
	struct ym3526_info *info = param;
	ym3526_update_one(info->chip, outputs[0], samples);
}

static void _stream_update_3526(void *param, int interval)
{
	struct ym3526_info *info = param;
	stream_update(info->stream);
}


static SND_START( ym3526 )
{
	static const ym3526_interface dummy = { 0 };
	struct ym3526_info *info;
	int rate = clock/72;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = config ? config : &dummy;
	info->device = device;

	/* stream system initialize */
	info->chip = ym3526_init(device,clock,rate);
	if (!info->chip)
		return NULL;

	info->stream = stream_create(device,0,1,rate,info,ym3526_stream_update);
	/* YM3526 setup */
	ym3526_set_timer_handler (info->chip, TimerHandler_3526, info);
	ym3526_set_irq_handler   (info->chip, IRQHandler_3526, info);
	ym3526_set_update_handler(info->chip, _stream_update_3526, info);

	info->timer[0] = timer_alloc(device->machine, timer_callback_3526_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_3526_1, info);

	return info;
}

static SND_STOP( ym3526 )
{
	struct ym3526_info *info = device->token;
	ym3526_shutdown(info->chip);
}

static SND_RESET( ym3526 )
{
	struct ym3526_info *info = device->token;
	ym3526_reset_chip(info->chip);
}

WRITE8_HANDLER( ym3526_control_port_0_w ) {
	struct ym3526_info *info = sndti_token(SOUND_YM3526, 0);
	ym3526_write(info->chip, 0, data);
}
WRITE8_HANDLER( ym3526_write_port_0_w ) {
	struct ym3526_info *info = sndti_token(SOUND_YM3526, 0);
	ym3526_write(info->chip, 1, data);
}
READ8_HANDLER( ym3526_status_port_0_r ) {
	struct ym3526_info *info = sndti_token(SOUND_YM3526, 0);
	return ym3526_read(info->chip, 0);
}
READ8_HANDLER( ym3526_read_port_0_r ) {
	struct ym3526_info *info = sndti_token(SOUND_YM3526, 0);
	return ym3526_read(info->chip, 1);
}


WRITE8_HANDLER( ym3526_control_port_1_w ) {
	struct ym3526_info *info = sndti_token(SOUND_YM3526, 1);
	ym3526_write(info->chip, 0, data);
}
WRITE8_HANDLER( ym3526_write_port_1_w ) {
	struct ym3526_info *info = sndti_token(SOUND_YM3526, 1);
	ym3526_write(info->chip, 1, data);
}
READ8_HANDLER( ym3526_status_port_1_r ) {
	struct ym3526_info *info = sndti_token(SOUND_YM3526, 1);
	return ym3526_read(info->chip, 0);
}
READ8_HANDLER( ym3526_read_port_1_r ) {
	struct ym3526_info *info = sndti_token(SOUND_YM3526, 1);
	return ym3526_read(info->chip, 1);
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( ym3526 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( ym3526 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( ym3526 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( ym3526 );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( ym3526 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( ym3526 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "YM3526");							break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");								break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);							break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

#endif


#if BUILD_Y8950

struct y8950_info
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	const y8950_interface *intf;
	const device_config *device;
};

static void IRQHandler_8950(void *param,int irq)
{
	struct y8950_info *info = param;
	if (info->intf->handler) (info->intf->handler)(info->device->machine, irq ? ASSERT_LINE : CLEAR_LINE);
}
static TIMER_CALLBACK( timer_callback_8950_0 )
{
	struct y8950_info *info = ptr;
	y8950_timer_over(info->chip,0);
}
static TIMER_CALLBACK( timer_callback_8950_1 )
{
	struct y8950_info *info = ptr;
	y8950_timer_over(info->chip,1);
}
static void TimerHandler_8950(void *param,int c,attotime period)
{
	struct y8950_info *info = param;
	if( attotime_compare(period, attotime_zero) == 0 )
	{	/* Reset FM Timer */
		timer_enable(info->timer[c], 0);
	}
	else
	{	/* Start FM Timer */
		timer_adjust_oneshot(info->timer[c], period, 0);
	}
}


static unsigned char Y8950PortHandler_r(void *param)
{
	struct y8950_info *info = param;
	if (info->intf->portread)
		return info->intf->portread(info->device,0);
	return 0;
}

static void Y8950PortHandler_w(void *param,unsigned char data)
{
	struct y8950_info *info = param;
	if (info->intf->portwrite)
		info->intf->portwrite(info->device,0,data);
}

static unsigned char Y8950KeyboardHandler_r(void *param)
{
	struct y8950_info *info = param;
	if (info->intf->keyboardread)
		return info->intf->keyboardread(info->device,0);
	return 0;
}

static void Y8950KeyboardHandler_w(void *param,unsigned char data)
{
	struct y8950_info *info = param;
	if (info->intf->keyboardwrite)
		info->intf->keyboardwrite(info->device,0,data);
}

static STREAM_UPDATE( y8950_stream_update )
{
	struct y8950_info *info = param;
	y8950_update_one(info->chip, outputs[0], samples);
}

static void _stream_update_8950(void *param, int interval)
{
	struct y8950_info *info = param;
	stream_update(info->stream);
}


static SND_START( y8950 )
{
	static const y8950_interface dummy = { 0 };
	struct y8950_info *info;
	int rate = clock/72;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = config ? config : &dummy;
	info->device = device;

	/* stream system initialize */
	info->chip = y8950_init(device,clock,rate);
	if (!info->chip)
		return NULL;

	/* ADPCM ROM data */
	y8950_set_delta_t_memory(info->chip, device->region, device->regionbytes);

	info->stream = stream_create(device,0,1,rate,info,y8950_stream_update);

	/* port and keyboard handler */
	y8950_set_port_handler(info->chip, Y8950PortHandler_w, Y8950PortHandler_r, info);
	y8950_set_keyboard_handler(info->chip, Y8950KeyboardHandler_w, Y8950KeyboardHandler_r, info);

	/* Y8950 setup */
	y8950_set_timer_handler (info->chip, TimerHandler_8950, info);
	y8950_set_irq_handler   (info->chip, IRQHandler_8950, info);
	y8950_set_update_handler(info->chip, _stream_update_8950, info);

	info->timer[0] = timer_alloc(device->machine, timer_callback_8950_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_8950_1, info);

	return info;
}

static SND_STOP( y8950 )
{
	struct y8950_info *info = device->token;
	y8950_shutdown(info->chip);
}

static SND_RESET( y8950 )
{
	struct y8950_info *info = device->token;
	y8950_reset_chip(info->chip);
}

WRITE8_HANDLER( y8950_control_port_0_w ) {
	struct y8950_info *info = sndti_token(SOUND_Y8950, 0);
	y8950_write(info->chip, 0, data);
}
WRITE8_HANDLER( y8950_write_port_0_w ) {
	struct y8950_info *info = sndti_token(SOUND_Y8950, 0);
	y8950_write(info->chip, 1, data);
}
READ8_HANDLER( y8950_status_port_0_r ) {
	struct y8950_info *info = sndti_token(SOUND_Y8950, 0);
	return y8950_read(info->chip, 0);
}
READ8_HANDLER( y8950_read_port_0_r ) {
	struct y8950_info *info = sndti_token(SOUND_Y8950, 0);
	return y8950_read(info->chip, 1);
}


WRITE8_HANDLER( y8950_control_port_1_w ) {
	struct y8950_info *info = sndti_token(SOUND_Y8950, 1);
	y8950_write(info->chip, 0, data);
}
WRITE8_HANDLER( y8950_write_port_1_w ) {
	struct y8950_info *info = sndti_token(SOUND_Y8950, 1);
	y8950_write(info->chip, 1, data);
}
READ8_HANDLER( y8950_status_port_1_r ) {
	struct y8950_info *info = sndti_token(SOUND_Y8950, 1);
	return y8950_read(info->chip, 0);
}
READ8_HANDLER( y8950_read_port_1_r ) {
	struct y8950_info *info = sndti_token(SOUND_Y8950, 1);
	return y8950_read(info->chip, 1);
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( y8950 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( y8950 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( y8950 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( y8950 );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( y8950 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( y8950 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "Y8950");							break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");								break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);							break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

#endif
