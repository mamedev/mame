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
	const ym2610_interface *intf;
	const device_config *device;
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
	if(info->intf->handler) info->intf->handler(info->device->machine, irq);
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


static SND_START( ym2610 )
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
	info->device = device;
	info->psg = ay8910_start_ym(SOUND_YM2610, device, clock, &generic_ay8910);
	if (!info->psg) return NULL;

	/* Timer Handler set */
	info->timer[0] = timer_alloc(device->machine, timer_callback_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_1, info);

	/* stream system initialize */
	info->stream = stream_create(device,0,2,rate,info,ym2610_stream_update);
	/* setup adpcm buffers */
	pcmbufa  = device->region;
	pcmsizea = device->regionbytes;
	astring_printf(name, "%s.deltat", device->tag);
	pcmbufb  = (void *)(memory_region(device->machine, astring_c(name)));
	pcmsizeb = memory_region_length(device->machine, astring_c(name));
	astring_free(name);
	if (pcmbufb == NULL || pcmsizeb == 0)
	{
		pcmbufb = pcmbufa;
		pcmsizeb = pcmsizea;
	}

	/**** initialize YM2610 ****/
	info->chip = ym2610_init(info,device,clock,rate,
		           pcmbufa,pcmsizea,pcmbufb,pcmsizeb,
		           timer_handler,IRQHandler,&psgintf);

	state_save_register_postload(device->machine, ym2610_intf_postload, info);

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

static SND_START( ym2610b )
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
	info->device = device;
	info->psg = ay8910_start_ym(SOUND_YM2610B, device, clock, &generic_ay8910);
	if (!info->psg) return NULL;

	/* Timer Handler set */
	info->timer[0] =timer_alloc(device->machine, timer_callback_0, info);
	info->timer[1] =timer_alloc(device->machine, timer_callback_1, info);

	/* stream system initialize */
	info->stream = stream_create(device, 0,2,rate,info,ym2610b_stream_update);
	/* setup adpcm buffers */
	pcmbufa  = device->region;
	pcmsizea = device->regionbytes;
	astring_printf(name, "%s.deltat", device->tag);
	pcmbufb  = (void *)(memory_region(device->machine, astring_c(name)));
	pcmsizeb = memory_region_length(device->machine, astring_c(name));
	astring_free(name);
	if (pcmbufb == NULL || pcmsizeb == 0)
	{
		pcmbufb = pcmbufa;
		pcmsizeb = pcmsizea;
	}

	/**** initialize YM2610 ****/
	info->chip = ym2610_init(info,device,clock,rate,
		           pcmbufa,pcmsizea,pcmbufb,pcmsizeb,
		           timer_handler,IRQHandler,&psgintf);
	if (info->chip)
		return info;

	/* error */
	return NULL;
}
#endif

static SND_STOP( ym2610 )
{
	struct ym2610_info *info = device->token;
	ym2610_shutdown(info->chip);
	ay8910_stop_ym(info->psg);
}

static SND_RESET( ym2610 )
{
	struct ym2610_info *info = device->token;
	ym2610_reset_chip(info->chip);
}

/************************************************/
/* Status Read for YM2610 - Chip 0              */
/************************************************/
READ8_HANDLER( ym2610_status_port_0_a_r )
{
//logerror("PC %04x: 2610 S0A=%02X\n",cpu_get_pc(space->cpu),ym2610_read(sndti_token(chip_type,0,0));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return ym2610_read(info->chip,0);
}

READ16_HANDLER( ym2610_status_port_0_a_lsb_r )
{
//logerror("PC %04x: 2610 S0A=%02X\n",cpu_get_pc(space->cpu),ym2610_read(sndti_token(chip_type,0,0));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return ym2610_read(info->chip,0);
}

READ8_HANDLER( ym2610_status_port_0_b_r )
{
//logerror("PC %04x: 2610 S0B=%02X\n",cpu_get_pc(space->cpu),ym2610_read(sndti_token(chip_type,0,2));
	struct ym2610_info *info = sndti_token(chip_type,0);
	return ym2610_read(info->chip,2);
}

READ16_HANDLER( ym2610_status_port_0_b_lsb_r )
{
//logerror("PC %04x: 2610 S0B=%02X\n",cpu_get_pc(space->cpu),ym2610_read(sndti_token(chip_type,0,2));
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
//logerror("PC %04x: 2610 Reg A %02X",cpu_get_pc(space->cpu),data);
	struct ym2610_info *info = sndti_token(chip_type,0);
	ym2610_write(info->chip,0,data);
}

WRITE16_HANDLER( ym2610_control_port_0_a_lsb_w )
{
//logerror("PC %04x: 2610 Reg A %02X",cpu_get_pc(space->cpu),data);
	if (ACCESSING_BITS_0_7)
	{
		struct ym2610_info *info = sndti_token(chip_type,0);
		ym2610_write(info->chip,0,data);
	}
}

WRITE8_HANDLER( ym2610_control_port_0_b_w )
{
//logerror("PC %04x: 2610 Reg B %02X",cpu_get_pc(space->cpu),data);
	struct ym2610_info *info = sndti_token(chip_type,0);
	ym2610_write(info->chip,2,data);
}

WRITE16_HANDLER( ym2610_control_port_0_b_lsb_w )
{
//logerror("PC %04x: 2610 Reg B %02X",cpu_get_pc(space->cpu),data);
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
static SND_SET_INFO( ym2610 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( ym2610 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( ym2610 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( ym2610 );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( ym2610 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( ym2610 );				break;

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
static SND_SET_INFO( ym2610b )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( ym2610b )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( ym2610b );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( ym2610b );			break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( ym2610 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( ym2610 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YM2610B";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

#endif
