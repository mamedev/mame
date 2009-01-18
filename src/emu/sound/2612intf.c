/***************************************************************************

  2612intf.c

  The YM2612 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "sound/fm.h"
#include "sound/2612intf.h"


struct ym2612_info
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	const ym2612_interface *intf;
	const device_config *device;
};

/*------------------------- TM2612 -------------------------------*/
/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	struct ym2612_info *info = param;
	if(info->intf->handler) info->intf->handler(info->device->machine, irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_2612_0 )
{
	struct ym2612_info *info = ptr;
	ym2612_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_2612_1 )
{
	struct ym2612_info *info = ptr;
	ym2612_timer_over(info->chip,1);
}

static void timer_handler(void *param,int c,int count,int clock)
{
	struct ym2612_info *info = param;
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
void ym2612_update_request(void *param)
{
	struct ym2612_info *info = param;
	stream_update(info->stream);
}

/***********************************************************/
/*    YM2612                                               */
/***********************************************************/

static STREAM_UPDATE( ym2612_stream_update )
{
	struct ym2612_info *info = param;
	ym2612_update_one(info->chip, outputs, samples);
}


static STATE_POSTLOAD( ym2612_intf_postload )
{
	struct ym2612_info *info = param;
	ym2612_postload(info->chip);
}


static SND_START( ym2612 )
{
	static const ym2612_interface dummy = { 0 };
	struct ym2612_info *info = device->token;
	int rate = clock/72;

	info->intf = device->static_config ? device->static_config : &dummy;
	info->device = device;

	/* FM init */
	/* Timer Handler set */
	info->timer[0] = timer_alloc(device->machine, timer_callback_2612_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_2612_1, info);

	/* stream system initialize */
	info->stream = stream_create(device,0,2,rate,info,ym2612_stream_update);

	/**** initialize YM2612 ****/
	info->chip = ym2612_init(info,device,clock,rate,timer_handler,IRQHandler);
	assert_always(info->chip != NULL, "Error creating YM2612 chip");

	state_save_register_postload(device->machine, ym2612_intf_postload, info);
	
	return DEVICE_START_OK;
}


static SND_STOP( ym2612 )
{
	struct ym2612_info *info = device->token;
	ym2612_shutdown(info->chip);
}

static SND_RESET( ym2612 )
{
	struct ym2612_info *info = device->token;
	ym2612_reset_chip(info->chip);
}


/************************************************/
/* Status Read for YM2612 - Chip 0              */
/************************************************/
READ8_HANDLER( ym2612_status_port_0_a_r )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  return ym2612_read(info->chip,0);
}

READ8_HANDLER( ym2612_status_port_0_b_r )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  return ym2612_read(info->chip,2);
}

/************************************************/
/* Status Read for YM2612 - Chip 1              */
/************************************************/
READ8_HANDLER( ym2612_status_port_1_a_r ) {
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  return ym2612_read(info->chip,0);
}

READ8_HANDLER( ym2612_status_port_1_b_r ) {
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  return ym2612_read(info->chip,2);
}

/************************************************/
/* Port Read for YM2612 - Chip 0                */
/************************************************/
READ8_HANDLER( ym2612_read_port_0_r ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  return ym2612_read(info->chip,1);
}

/************************************************/
/* Port Read for YM2612 - Chip 1                */
/************************************************/
READ8_HANDLER( ym2612_read_port_1_r ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  return ym2612_read(info->chip,1);
}

/************************************************/
/* Control Write for YM2612 - Chip 0            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2612_control_port_0_a_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  ym2612_write(info->chip,0,data);
}

WRITE8_HANDLER( ym2612_control_port_0_b_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  ym2612_write(info->chip,2,data);
}

/************************************************/
/* Control Write for YM2612 - Chip 1            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2612_control_port_1_a_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  ym2612_write(info->chip,0,data);
}

WRITE8_HANDLER( ym2612_control_port_1_b_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  ym2612_write(info->chip,2,data);
}

/************************************************/
/* Data Write for YM2612 - Chip 0               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2612_data_port_0_a_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  ym2612_write(info->chip,1,data);
}

WRITE8_HANDLER( ym2612_data_port_0_b_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  ym2612_write(info->chip,3,data);
}

/************************************************/
/* Data Write for YM2612 - Chip 1               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2612_data_port_1_a_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  ym2612_write(info->chip,1,data);
}
WRITE8_HANDLER( ym2612_data_port_1_b_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  ym2612_write(info->chip,3,data);
}

#if BUILD_YM3438

/************************************************/
/* Status Read for YM3438 - Chip 0              */
/************************************************/
READ8_HANDLER( ym3438_status_port_0_a_r )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  return ym2612_read(info->chip,0);
}

READ8_HANDLER( ym3438_status_port_0_b_r )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  return ym2612_read(info->chip,2);
}

/************************************************/
/* Status Read for YM3438 - Chip 1              */
/************************************************/
READ8_HANDLER( ym3438_status_port_1_a_r ) {
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  return ym2612_read(info->chip,0);
}

READ8_HANDLER( ym3438_status_port_1_b_r ) {
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  return ym2612_read(info->chip,2);
}

/************************************************/
/* Port Read for YM3438 - Chip 0                */
/************************************************/
READ8_HANDLER( ym3438_read_port_0_r ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  return ym2612_read(info->chip,1);
}

/************************************************/
/* Port Read for YM3438 - Chip 1                */
/************************************************/
READ8_HANDLER( ym3438_read_port_1_r ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  return ym2612_read(info->chip,1);
}

/************************************************/
/* Control Write for YM3438 - Chip 0            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym3438_control_port_0_a_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  ym2612_write(info->chip,0,data);
}

WRITE8_HANDLER( ym3438_control_port_0_b_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  ym2612_write(info->chip,2,data);
}

/************************************************/
/* Control Write for YM3438 - Chip 1            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym3438_control_port_1_a_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  ym2612_write(info->chip,0,data);
}

WRITE8_HANDLER( ym3438_control_port_1_b_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  ym2612_write(info->chip,2,data);
}

/************************************************/
/* Data Write for YM3438 - Chip 0               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym3438_data_port_0_a_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  ym2612_write(info->chip,1,data);
}

WRITE8_HANDLER( ym3438_data_port_0_b_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  ym2612_write(info->chip,3,data);
}

/************************************************/
/* Data Write for YM3438 - Chip 1               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym3438_data_port_1_a_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  ym2612_write(info->chip,1,data);
}
WRITE8_HANDLER( ym3438_data_port_1_b_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  ym2612_write(info->chip,3,data);
}

#endif

/**************** end of file ****************/

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( ym2612 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( ym2612 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_TOKEN_BYTES:					info->i = sizeof(struct ym2612_info);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( ym2612 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( ym2612 );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( ym2612 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( ym2612 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "YM2612");							break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");								break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);							break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

#if BUILD_YM3438

static SND_SET_INFO( ym3438 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( ym3438 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_TOKEN_BYTES:					info->i = sizeof(struct ym2612_info);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( ym3438 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( ym2612 );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( ym2612 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( ym2612 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "YM3438");							break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");								break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);							break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

#endif
