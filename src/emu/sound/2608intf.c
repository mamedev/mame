/***************************************************************************

  2608intf.c

  The YM2608 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "ay8910.h"
#include "2608intf.h"
#include "fm.h"

struct ym2608_info
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	void *			psg;
	const struct YM2608interface *intf;
};



static void psg_set_clock(void *param, int clock)
{
	struct ym2608_info *info = param;
	ay8910_set_clock_ym(info->psg, clock);
}

static void psg_write(void *param, int address, int data)
{
	struct ym2608_info *info = param;
	ay8910_write_ym(info->psg, address, data);
}

static int psg_read(void *param)
{
	struct ym2608_info *info = param;
	return ay8910_read_ym(info->psg);
}

static void psg_reset(void *param)
{
	struct ym2608_info *info = param;
	ay8910_reset_ym(info->psg);
}

static const struct ssg_callbacks psgintf =
{
	psg_set_clock,
	psg_write,
	psg_read,
	psg_reset
};


/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	struct ym2608_info *info = param;
	if(info->intf->handler) info->intf->handler(irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_2608_0 )
{
	struct ym2608_info *info = ptr;
	YM2608TimerOver(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_2608_1 )
{
	struct ym2608_info *info = ptr;
	YM2608TimerOver(info->chip,1);
}

static void timer_handler(void *param,int c,int count,int clock)
{
	struct ym2608_info *info = param;
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
void YM2608UpdateRequest(void *param)
{
	struct ym2608_info *info = param;
	stream_update(info->stream);
}

static void ym2608_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffers, int length)
{
	struct ym2608_info *info = param;
	YM2608UpdateOne(info->chip, buffers, length);
}


static void ym2608_postload(void *param)
{
	struct ym2608_info *info = param;
	YM2608Postload(info->chip);
}


static void *ym2608_start(int sndindex, int clock, const void *config)
{
	static const struct YM2608interface generic_2608 = { 0 };
	const struct YM2608interface *intf = config ? config : &generic_2608;
	int rate = clock/72;
	void *pcmbufa;
	int  pcmsizea;

	struct ym2608_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = intf;
	info->psg = ay8910_start_ym(SOUND_YM2608, sndindex, clock, 1, intf->portAread, intf->portBread, intf->portAwrite, intf->portBwrite);
	if (!info->psg) return NULL;

	/* Timer Handler set */
	info->timer[0] = timer_alloc(timer_callback_2608_0, info);
	info->timer[1] = timer_alloc(timer_callback_2608_1, info);

	/* stream system initialize */
	info->stream = stream_create(0,2,rate,info,ym2608_stream_update);
	/* setup adpcm buffers */
	pcmbufa  = (void *)(memory_region(info->intf->pcmrom));
	pcmsizea = memory_region_length(info->intf->pcmrom);

	/* initialize YM2608 */
	info->chip = YM2608Init(info,sndindex,clock,rate,
		           pcmbufa,pcmsizea,
		           timer_handler,IRQHandler,&psgintf);

	state_save_register_func_postload_ptr(ym2608_postload, info);

	if (info->chip)
		return info;

	/* error */
	return NULL;
}

static void ym2608_stop(void *token)
{
	struct ym2608_info *info = token;
	YM2608Shutdown(info->chip);
	ay8910_stop_ym(info->psg);
}

static void ym2608_reset(void *token)
{
	struct ym2608_info *info = token;
	YM2608ResetChip(info->chip);
}

/************************************************/
/* Status Read for YM2608 - Chip 0              */
/************************************************/
READ8_HANDLER( YM2608_status_port_0_A_r )
{
//logerror("PC %04x: 2608 S0A=%02X\n",activecpu_get_pc(),YM2608Read(sndti_token(SOUND_YM2608, 0),0));
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 0);
	return YM2608Read(info->chip,0);
}

READ8_HANDLER( YM2608_status_port_0_B_r )
{
//logerror("PC %04x: 2608 S0B=%02X\n",activecpu_get_pc(),YM2608Read(sndti_token(SOUND_YM2608, 0),2));
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 0);
	return YM2608Read(info->chip,2);
}

/************************************************/
/* Status Read for YM2608 - Chip 1              */
/************************************************/
READ8_HANDLER( YM2608_status_port_1_A_r ) {
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 1);
	return YM2608Read(info->chip,0);
}

READ8_HANDLER( YM2608_status_port_1_B_r ) {
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 1);
	return YM2608Read(info->chip,2);
}

/************************************************/
/* Port Read for YM2608 - Chip 0                */
/************************************************/
READ8_HANDLER( YM2608_read_port_0_r ){
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 0);
	return YM2608Read(info->chip,1);
}

/************************************************/
/* Port Read for YM2608 - Chip 1                */
/************************************************/
READ8_HANDLER( YM2608_read_port_1_r ){
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 1);
	return YM2608Read(info->chip,1);
}

/************************************************/
/* Control Write for YM2608 - Chip 0            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( YM2608_control_port_0_A_w )
{
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 0);
	YM2608Write(info->chip,0,data);
}

WRITE8_HANDLER( YM2608_control_port_0_B_w )
{
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 0);
	YM2608Write(info->chip,2,data);
}

/************************************************/
/* Control Write for YM2608 - Chip 1            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( YM2608_control_port_1_A_w ){
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 1);
	YM2608Write(info->chip,0,data);
}

WRITE8_HANDLER( YM2608_control_port_1_B_w ){
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 1);
	YM2608Write(info->chip,2,data);
}

/************************************************/
/* Data Write for YM2608 - Chip 0               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( YM2608_data_port_0_A_w )
{
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 0);
	YM2608Write(info->chip,1,data);
}

WRITE8_HANDLER( YM2608_data_port_0_B_w )
{
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 0);
	YM2608Write(info->chip,3,data);
}

/************************************************/
/* Data Write for YM2608 - Chip 1               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( YM2608_data_port_1_A_w ){
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 1);
	YM2608Write(info->chip,1,data);
}
WRITE8_HANDLER( YM2608_data_port_1_B_w ){
	struct ym2608_info *info = sndti_token(SOUND_YM2608, 1);
	YM2608Write(info->chip,3,data);
}

/**************** end of file ****************/


/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void ym2608_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void ym2608_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = ym2608_set_info;		break;
		case SNDINFO_PTR_START:							info->start = ym2608_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = ym2608_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = ym2608_reset;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YM2608";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
