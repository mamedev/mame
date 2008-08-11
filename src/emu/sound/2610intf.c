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
#include "deprecat.h"
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
	const ym2610_interface *intf;
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

static const ssg_callbacks psgintf =
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
	if(info->intf->handler) info->intf->handler(Machine, irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_0 )
{
	struct ym2610_info *info = ptr;
	ym2610_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_1 )
{
	struct ym2610_info *info = ptr;
	ym2610_timer_over(info->chip,1);
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
void ym2610_update_request(void *param)
{
	struct ym2610_info *info = param;
	stream_update(info->stream);
}

#if BUILD_YM2610
static void ym2610_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffers, int length)
{
	struct ym2610_info *info = param;
	ym2610_update_one(info->chip, buffers, length);
}


static STATE_POSTLOAD( ym2610_intf_postload )
{
	struct ym2610_info *info = param;
	ym2610_postload(info->chip);
}


static void *ym2610_start(const char *tag, int sndindex, int clock, const void *config)
{
	static const ym2610_interface generic_2610 = { 0 };
	static const ay8910_interface generic_ay8910 =
	{
		AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT,
		AY8910_DEFAULT_LOADS,
		NULL, NULL, NULL, NULL
	};
	const ym2610_interface *intf = config ? config : &generic_2610;
	int rate = clock/72;
	void *pcmbufa,*pcmbufb;
	int  pcmsizea,pcmsizeb;
	struct ym2610_info *info;
	astring *name = astring_alloc();

	chip_type = SOUND_YM2610;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = intf;
	info->psg = ay8910_start_ym(SOUND_YM2610, sndindex, clock, &generic_ay8910);
	if (!info->psg) return NULL;

	/* Timer Handler set */
	info->timer[0] = timer_alloc(timer_callback_0, info);
	info->timer[1] = timer_alloc(timer_callback_1, info);

	/* stream system initialize */
	info->stream = stream_create(0,2,rate,info,ym2610_stream_update);
	/* setup adpcm buffers */
	pcmbufa  = (void *)(memory_region(Machine, tag));
	pcmsizea = memory_region_length(Machine, tag);
	astring_printf(name, "%s.deltat", tag);
	pcmbufb  = (void *)(memory_region(Machine, astring_c(name)));
	pcmsizeb = memory_region_length(Machine, astring_c(name));
	astring_free(name);
	if (pcmbufb == NULL || pcmsizeb == 0)
	{
		pcmbufb = pcmbufa;
		pcmsizeb = pcmsizea;
	}

	/**** initialize YM2610 ****/
	info->chip = ym2610_init(info,sndindex,clock,rate,
		           pcmbufa,pcmsizea,pcmbufb,pcmsizeb,
		           timer_handler,IRQHandler,&psgintf);

	state_save_register_postload(Machine, ym2610_intf_postload, info);

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
	ym2610b_update_one(info->chip, buffers, length);
}

static void *ym2610b_start(const char *tag, int sndindex, int clock, const void *config)
{
	static const ym2610_interface generic_2610 = { 0 };
	static const ay8910_interface generic_ay8910 =
	{
		AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT,
		AY8910_DEFAULT_LOADS,
		NULL, NULL, NULL, NULL
	};
	const ym2610_interface *intf = config ? config : &generic_2610;
	int rate = clock/72;
	void *pcmbufa,*pcmbufb;
	int  pcmsizea,pcmsizeb;
	struct ym2610_info *info;
	astring *name = astring_alloc();

	chip_type = SOUND_YM2610B;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = intf;
	info->psg = ay8910_start_ym(SOUND_YM2610B, sndindex, clock, &generic_ay8910);
	if (!info->psg) return NULL;

	/* Timer Handler set */
	info->timer[0] =timer_alloc(timer_callback_0, info);
	info->timer[1] =timer_alloc(timer_callback_1, info);

	/* stream system initialize */
	info->stream = stream_create(0,2,rate,info,ym2610b_stream_update);
	/* setup adpcm buffers */
	pcmbufa  = (void *)(memory_region(Machine, tag));
	pcmsizea = memory_region_length(Machine, tag);
	astring_printf(name, "%s.deltat", tag);
	pcmbufb  = (void *)(memory_region(Machine, astring_c(name)));
	pcmsizeb = memory_region_length(Machine, astring_c(name));
	astring_free(name);
	if (pcmbufb == NULL || pcmsizeb == 0)
	{
		pcmbufb = pcmbufa;
		pcmsizeb = pcmsizea;
	}

	/**** initialize YM2610 ****/
	info->chip = ym2610_init(info,sndindex,clock,rate,
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
	ym2610_shutdown(info->chip);
	ay8910_stop_ym(info->psg);
}

static void ym2610_reset(void *token)
{
	struct ym2610_info *info = token;
	ym2610_reset_chip(info->chip);
}

/************************************************/
/* Status Read for YM2610 - Chip 0              */
/************************************************/
READ8_HANDLER( ym2610_status_port_0_a_r )
{
//logerror("PC %04x: 2610 S0A=%02X\n",activecpu_get_pc(),ym2610_read(sndti_token(chip_type,0,0));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return ym2610_read(info->chip,0);
}

READ16_HANDLER( ym2610_status_port_0_a_lsb_r )
{
//logerror("PC %04x: 2610 S0A=%02X\n",activecpu_get_pc(),ym2610_read(sndti_token(chip_type,0,0));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return ym2610_read(info->chip,0);
}

READ8_HANDLER( ym2610_status_port_0_b_r )
{
//logerror("PC %04x: 2610 S0B=%02X\n",activecpu_get_pc(),ym2610_read(sndti_token(chip_type,0,2));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return ym2610_read(info->chip,2);
}

READ16_HANDLER( ym2610_status_port_0_b_lsb_r )
{
//logerror("PC %04x: 2610 S0B=%02X\n",activecpu_get_pc(),ym2610_read(sndti_token(chip_type,0,2));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return ym2610_read(info->chip,2);
}

/************************************************/
/* Status Read for YM2610 - Chip 1              */
/************************************************/
READ8_HANDLER( ym2610_status_port_1_a_r ) {
	struct ym2610_info *info = sndti_token(chip_type,1);
	return ym2610_read(info->chip,0);
}

READ16_HANDLER( ym2610_status_port_1_a_lsb_r ) {
	struct ym2610_info *info = sndti_token(chip_type,1);
	return ym2610_read(info->chip,0);
}

READ8_HANDLER( ym2610_status_port_1_b_r ) {
	struct ym2610_info *info = sndti_token(chip_type,1);
	return ym2610_read(info->chip,2);
}

READ16_HANDLER( ym2610_status_port_1_b_lsb_r ) {
	struct ym2610_info *info = sndti_token(chip_type,1);
	return ym2610_read(info->chip,2);
}

/************************************************/
/* Port Read for YM2610 - Chip 0                */
/************************************************/
READ8_HANDLER( ym2610_read_port_0_r ){
	struct ym2610_info *info = sndti_token(chip_type,0);
	return ym2610_read(info->chip,1);
}

READ16_HANDLER( ym2610_read_port_0_lsb_r ){
	struct ym2610_info *info = sndti_token(chip_type,0);
	return ym2610_read(info->chip,1);
}

/************************************************/
/* Port Read for YM2610 - Chip 1                */
/************************************************/
READ8_HANDLER( ym2610_read_port_1_r ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	return ym2610_read(info->chip,1);
}

READ16_HANDLER( ym2610_read_port_1_lsb_r ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	return ym2610_read(info->chip,1);
}

/************************************************/
/* Control Write for YM2610 - Chip 0            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2610_control_port_0_a_w )
{
//logerror("PC %04x: 2610 Reg A %02X",activecpu_get_pc(),data);
	struct ym2610_info *info = sndti_token(chip_type,0);
	ym2610_write(info->chip,0,data);
}

WRITE16_HANDLER( ym2610_control_port_0_a_lsb_w )
{
//logerror("PC %04x: 2610 Reg A %02X",activecpu_get_pc(),data);
	if (ACCESSING_BITS_0_7)
	{
		struct ym2610_info *info = sndti_token(chip_type,0);
		ym2610_write(info->chip,0,data);
	}
}

WRITE8_HANDLER( ym2610_control_port_0_b_w )
{
//logerror("PC %04x: 2610 Reg B %02X",activecpu_get_pc(),data);
	struct ym2610_info *info = sndti_token(chip_type,0);
	ym2610_write(info->chip,2,data);
}

WRITE16_HANDLER( ym2610_control_port_0_b_lsb_w )
{
//logerror("PC %04x: 2610 Reg B %02X",activecpu_get_pc(),data);
	if (ACCESSING_BITS_0_7)
	{
		struct ym2610_info *info = sndti_token(chip_type,0);
		ym2610_write(info->chip,2,data);
	}
}

/************************************************/
/* Control Write for YM2610 - Chip 1            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2610_control_port_1_a_w ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	ym2610_write(info->chip,0,data);
}

WRITE16_HANDLER( ym2610_control_port_1_a_lsb_w ){
	if (ACCESSING_BITS_0_7)
	{
		struct ym2610_info *info = sndti_token(chip_type,1);
		ym2610_write(info->chip,0,data);
	}
}

WRITE8_HANDLER( ym2610_control_port_1_b_w ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	ym2610_write(info->chip,2,data);
}

WRITE16_HANDLER( ym2610_control_port_1_b_lsb_w ){
	if (ACCESSING_BITS_0_7)
	{
		struct ym2610_info *info = sndti_token(chip_type,1);
		ym2610_write(info->chip,2,data);
	}
}

/************************************************/
/* Data Write for YM2610 - Chip 0               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2610_data_port_0_a_w )
{
//logerror(" =%02X\n",data);
	struct ym2610_info *info = sndti_token(chip_type,0);
	ym2610_write(info->chip,1,data);
}

WRITE16_HANDLER( ym2610_data_port_0_a_lsb_w )
{
//logerror(" =%02X\n",data);
	if (ACCESSING_BITS_0_7)
	{
		struct ym2610_info *info = sndti_token(chip_type,0);
		ym2610_write(info->chip,1,data);
	}
}

WRITE8_HANDLER( ym2610_data_port_0_b_w )
{
//logerror(" =%02X\n",data);
	struct ym2610_info *info = sndti_token(chip_type,0);
	ym2610_write(info->chip,3,data);
}

WRITE16_HANDLER( ym2610_data_port_0_b_lsb_w )
{
//logerror(" =%02X\n",data);
	if (ACCESSING_BITS_0_7)
	{
		struct ym2610_info *info = sndti_token(chip_type,0);
		ym2610_write(info->chip,3,data);
	}
}

/************************************************/
/* Data Write for YM2610 - Chip 1               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2610_data_port_1_a_w ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	ym2610_write(info->chip,1,data);
}

WRITE16_HANDLER( ym2610_data_port_1_a_lsb_w ){
	if (ACCESSING_BITS_0_7)
	{
		struct ym2610_info *info = sndti_token(chip_type,1);
		ym2610_write(info->chip,1,data);
	}
}

WRITE8_HANDLER( ym2610_data_port_1_b_w ){
	struct ym2610_info *info = sndti_token(chip_type,1);
	ym2610_write(info->chip,3,data);
}

WRITE16_HANDLER( ym2610_data_port_1_b_lsb_w ){
	if (ACCESSING_BITS_0_7)
	{
		struct ym2610_info *info = sndti_token(chip_type,1);
		ym2610_write(info->chip,3,data);
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
