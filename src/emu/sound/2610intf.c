/***************************************************************************

  2610intf.c

  The YM2610 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "ay8910.h"
#include "2610intf.h"
#include "fm.h"

static sound_type chip_type = SOUND_YM2610;
struct ym2610_info
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	void *			psg;
	const struct YM2610interface *intf;
};


static void psg_set_clock(void *param, int clock)
{
	struct ym2610_info *info = param;
	ay8910_set_clock_ym(info->psg, clock);
}

static void psg_write(void *param, int address, int data)
{
	struct ym2610_info *info = param;
	ay8910_write_ym(info->psg, address, data);
}

static int psg_read(void *param)
{
	struct ym2610_info *info = param;
	return ay8910_read_ym(info->psg);
}

static void psg_reset(void *param)
{
	struct ym2610_info *info = param;
	ay8910_reset_ym(info->psg);
}

static const struct ssg_callbacks psgintf =
{
	psg_set_clock,
	psg_write,
	psg_read,
	psg_reset
};

/*------------------------- TM2610 -------------------------------*/
/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	struct ym2610_info *info = param;
	if(info->intf->handler) info->intf->handler(irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_0 )
{
	struct ym2610_info *info = ptr;
	YM2610TimerOver(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_1 )
{
	struct ym2610_info *info = ptr;
	YM2610TimerOver(info->chip,1);
}

static void timer_handler(void *param,int c,int count,int clock)
{
	struct ym2610_info *info = param;
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
void YM2610UpdateRequest(void *param)
{
	struct ym2610_info *info = param;
	stream_update(info->stream);
}

#if BUILD_YM2610
static void ym2610_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffers, int length)
{
	struct ym2610_info *info = param;
	YM2610UpdateOne(info->chip, buffers, length);
}


static void ym2610_postload(void *param)
{
	struct ym2610_info *info = param;
	YM2610Postload(info->chip);
}


static void *ym2610_start(int sndindex, int clock, const void *config)
{
	static const struct YM2610interface generic_2610 = { 0 };
	const struct YM2610interface *intf = config ? config : &generic_2610;
	int rate = clock/72;
	void *pcmbufa,*pcmbufb;
	int  pcmsizea,pcmsizeb;
	struct ym2610_info *info;

	chip_type = SOUND_YM2610;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = intf;
	info->psg = ay8910_start_ym(SOUND_YM2610, sndindex, clock, 1, NULL, NULL, NULL, NULL);
	if (!info->psg) return NULL;

	/* Timer Handler set */
	info->timer[0] = timer_alloc(timer_callback_0, info);
	info->timer[1] = timer_alloc(timer_callback_1, info);

	/* stream system initialize */
	info->stream = stream_create(0,2,rate,info,ym2610_stream_update);
	/* setup adpcm buffers */
	pcmbufa  = (void *)(memory_region(info->intf->pcmroma));
	pcmsizea = memory_region_length(info->intf->pcmroma);
	pcmbufb  = (void *)(memory_region(info->intf->pcmromb));
	pcmsizeb = memory_region_length(info->intf->pcmromb);

	/**** initialize YM2610 ****/
	info->chip = YM2610Init(info,sndindex,clock,rate,
		           pcmbufa,pcmsizea,pcmbufb,pcmsizeb,
		           timer_handler,IRQHandler,&psgintf);

	state_save_register_func_postload_ptr(ym2610_postload, info);

	if (info->chip)
		return info;

	/* error */
	return NULL;
}
#endif

#if BUILD_YM2610B
static void ym2610b_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffers, int length)
{
	struct ym2610_info *info = param;
	YM2610BUpdateOne(info->chip, buffers, length);
}

static void *ym2610b_start(int sndindex, int clock, const void *config)
{
	static const struct YM2610interface generic_2610 = { 0 };
	const struct YM2610interface *intf = config ? config : &generic_2610;
	int rate = clock/72;
	void *pcmbufa,*pcmbufb;
	int  pcmsizea,pcmsizeb;
	struct ym2610_info *info;

	chip_type = SOUND_YM2610B;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = intf;
	info->psg = ay8910_start_ym(SOUND_YM2610B, sndindex, clock, 1, NULL, NULL, NULL, NULL);
	if (!info->psg) return NULL;

	/* Timer Handler set */
	info->timer[0] =timer_alloc(timer_callback_0, info);
	info->timer[1] =timer_alloc(timer_callback_1, info);

	/* stream system initialize */
	info->stream = stream_create(0,2,rate,info,ym2610b_stream_update);
	/* setup adpcm buffers */
	pcmbufa  = (void *)(memory_region(info->intf->pcmroma));
	pcmsizea = memory_region_length(info->intf->pcmroma);
	pcmbufb  = (void *)(memory_region(info->intf->pcmromb));
	pcmsizeb = memory_region_length(info->intf->pcmromb);

	/**** initialize YM2610 ****/
	info->chip = YM2610Init(info,sndindex,clock,rate,
		           pcmbufa,pcmsizea,pcmbufb,pcmsizeb,
		           timer_handler,IRQHandler,&psgintf);
	if (info->chip)
		return info;

	/* error */
	return NULL;
}
#endif

static void ym2610_stop(void *token)
{
	struct ym2610_info *info = token;
	YM2610Shutdown(info->chip);
	ay8910_stop_ym(info->psg);
}

static void ym2610_reset(void *token)
{
	struct ym2610_info *info = token;
	YM2610ResetChip(info->chip);
}

/************************************************/
/* Status Read for YM2610 - Chip 0              */
/************************************************/
READ8_HANDLER( YM2610_status_port_0_A_r )
{
//logerror("PC %04x: 2610 S0A=%02X\n",activecpu_get_pc(),YM2610Read(sndti_token(chip_type,0,0));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return YM2610Read(info->chip,0);
}

READ16_HANDLER( YM2610_status_port_0_A_lsb_r )
{
//logerror("PC %04x: 2610 S0A=%02X\n",activecpu_get_pc(),YM2610Read(sndti_token(chip_type,0,0));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return YM2610Read(info->chip,0);
}

READ8_HANDLER( YM2610_status_port_0_B_r )
{
//logerror("PC %04x: 2610 S0B=%02X\n",activecpu_get_pc(),YM2610Read(sndti_token(chip_type,0,2));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return YM2610Read(info->chip,2);
}

READ16_HANDLER( YM2610_status_port_0_B_lsb_r )
{
//logerror("PC %04x: 2610 S0B=%02X\n",activecpu_get_pc(),YM2610Read(sndti_token(chip_type,0,2));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return YM2610Read(info->chip,2);
}

/************************************************/
/* Status Read for YM2610 - Chip 1              */
/************************************************/
READ8_HANDLER( YM2610_status_port_1_A_r ) {
	struct ym2610_info *info = sndti_token(chip_type,1);
	return YM2610Read(info->chip,0);
}

READ16_HANDLER( YM2610_status_port_1_A_lsb_r ) {
	struct ym2610_info *info = sndti_token(chip_type,1);
	return YM2610Read(info->chip,0);
}

READ8_HANDLER( YM2610_status_port_1_B_r ) {
	struct ym2610_info *info = sndti_token(chip_type,1);
	return YM2610Read(info->chip,2);
}

READ16_HANDLER( YM2610_status_port_1_B_lsb_r ) {
	struct ym2610_info *info = sndti_token(chip_type,1);
	return YM2610Read(info->chip,2);
}

/************************************************/
/* Port Read for YM2610 - Chip 0                */
/************************************************/
READ8_HANDLER( YM2610_read_port_0_r ){
	struct ym2610_info *info = sndti_token(chip_type,0);
	return YM2610Read(info->chip,1);
}

READ16_HANDLER( YM2610_read_port_0_lsb_r ){
	struct ym2610_info *info = sndti_token(chip_type,0);
	return YM2610Read(info->chip,1);
}

/************************************************/
/* Port Read for YM2610 - Chip 1                */
/************************************************/
READ8_HANDLER( YM2610_read_port_1_r ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	return YM2610Read(info->chip,1);
}

READ16_HANDLER( YM2610_read_port_1_lsb_r ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	return YM2610Read(info->chip,1);
}

/************************************************/
/* Control Write for YM2610 - Chip 0            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( YM2610_control_port_0_A_w )
{
//logerror("PC %04x: 2610 Reg A %02X",activecpu_get_pc(),data);
	struct ym2610_info *info = sndti_token(chip_type,0);
	YM2610Write(info->chip,0,data);
}

WRITE16_HANDLER( YM2610_control_port_0_A_lsb_w )
{
//logerror("PC %04x: 2610 Reg A %02X",activecpu_get_pc(),data);
	if (ACCESSING_BYTE_0)
	{
		struct ym2610_info *info = sndti_token(chip_type,0);
		YM2610Write(info->chip,0,data);
	}
}

WRITE8_HANDLER( YM2610_control_port_0_B_w )
{
//logerror("PC %04x: 2610 Reg B %02X",activecpu_get_pc(),data);
	struct ym2610_info *info = sndti_token(chip_type,0);
	YM2610Write(info->chip,2,data);
}

WRITE16_HANDLER( YM2610_control_port_0_B_lsb_w )
{
//logerror("PC %04x: 2610 Reg B %02X",activecpu_get_pc(),data);
	if (ACCESSING_BYTE_0)
	{
		struct ym2610_info *info = sndti_token(chip_type,0);
		YM2610Write(info->chip,2,data);
	}
}

/************************************************/
/* Control Write for YM2610 - Chip 1            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( YM2610_control_port_1_A_w ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	YM2610Write(info->chip,0,data);
}

WRITE16_HANDLER( YM2610_control_port_1_A_lsb_w ){
	if (ACCESSING_BYTE_0)
	{
		struct ym2610_info *info = sndti_token(chip_type,1);
		YM2610Write(info->chip,0,data);
	}
}

WRITE8_HANDLER( YM2610_control_port_1_B_w ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	YM2610Write(info->chip,2,data);
}

WRITE16_HANDLER( YM2610_control_port_1_B_lsb_w ){
	if (ACCESSING_BYTE_0)
	{
		struct ym2610_info *info = sndti_token(chip_type,1);
		YM2610Write(info->chip,2,data);
	}
}

/************************************************/
/* Data Write for YM2610 - Chip 0               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( YM2610_data_port_0_A_w )
{
//logerror(" =%02X\n",data);
	struct ym2610_info *info = sndti_token(chip_type,0);
	YM2610Write(info->chip,1,data);
}

WRITE16_HANDLER( YM2610_data_port_0_A_lsb_w )
{
//logerror(" =%02X\n",data);
	if (ACCESSING_BYTE_0)
	{
		struct ym2610_info *info = sndti_token(chip_type,0);
		YM2610Write(info->chip,1,data);
	}
}

WRITE8_HANDLER( YM2610_data_port_0_B_w )
{
//logerror(" =%02X\n",data);
	struct ym2610_info *info = sndti_token(chip_type,0);
	YM2610Write(info->chip,3,data);
}

WRITE16_HANDLER( YM2610_data_port_0_B_lsb_w )
{
//logerror(" =%02X\n",data);
	if (ACCESSING_BYTE_0)
	{
		struct ym2610_info *info = sndti_token(chip_type,0);
		YM2610Write(info->chip,3,data);
	}
}

/************************************************/
/* Data Write for YM2610 - Chip 1               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( YM2610_data_port_1_A_w ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	YM2610Write(info->chip,1,data);
}

WRITE16_HANDLER( YM2610_data_port_1_A_lsb_w ){
	if (ACCESSING_BYTE_0)
	{
		struct ym2610_info *info = sndti_token(chip_type,1);
		YM2610Write(info->chip,1,data);
	}
}

WRITE8_HANDLER( YM2610_data_port_1_B_w ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	YM2610Write(info->chip,3,data);
}

WRITE16_HANDLER( YM2610_data_port_1_B_lsb_w ){
	if (ACCESSING_BYTE_0)
	{
		struct ym2610_info *info = sndti_token(chip_type,1);
		YM2610Write(info->chip,3,data);
	}
}

/**************** end of file ****************/


/**************************************************************************
 * Generic get_info
 **************************************************************************/

#if BUILD_YM2610
static void ym2610_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void ym2610_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = ym2610_set_info;		break;
		case SNDINFO_PTR_START:							info->start = ym2610_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = ym2610_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = ym2610_reset;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YM2610";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
#endif


/**************************************************************************
 * Generic get_info
 **************************************************************************/

#if BUILD_YM2610B
static void ym2610b_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void ym2610b_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = ym2610b_set_info;		break;
		case SNDINFO_PTR_START:							info->start = ym2610b_start;			break;
		case SNDINFO_PTR_STOP:							info->stop = ym2610_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = ym2610_reset;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YM2610B";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

#endif
