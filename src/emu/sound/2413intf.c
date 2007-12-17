/****************************************************************

    MAME / MESS functions

****************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "ym2413.h"
#include "2413intf.h"

//#define YM2413ISA
#ifdef YM2413ISA
	#include <pc.h>
#endif


/* for stream system */
struct ym2413_info
{
	sound_stream *	stream;
	void *			chip;
};

/*
void YM2413DAC_update(int chip,stream_sample_t **inputs, stream_sample_t **_buffer,int length)
{
    INT16 *buffer = _buffer[0];
    static int out = 0;

    if ( ym2413[chip].reg[0x0F] & 0x01 )
    {
        out = ((ym2413[chip].reg[0x10] & 0xF0) << 7);
    }
    while (length--) *(buffer++) = out;
}
*/

static void ym2413_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffers, int length)
{
	struct ym2413_info *info = param;
	YM2413UpdateOne(info->chip, buffers, length);
}

static void _stream_update(void *param, int interval)
{
	struct ym2413_info *info = param;
	stream_update(info->stream);
}

static void *ym2413_start(int sndindex, int clock, const void *config)
{
	int rate = clock/72;
	struct ym2413_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	/* emulator create */
	info->chip = YM2413Init(clock, rate);
	if (!info->chip)
		return NULL;

	/* stream system initialize */
	info->stream = stream_create(0,2,rate,info,ym2413_stream_update);

	YM2413SetUpdateHandler(info->chip, _stream_update, info);

	return info;




#if 0
	int i, tst;
	char name[40];

	num = intf->num;

	tst = YM3812_sh_start (msound);
	if (tst)
		return 1;

	for (i=0;i<num;i++)
	{
		ym2413_reset (i);

		ym2413[i].DAC_stream = stream_create(0, 1, clock/72, i, YM2413DAC_update);

		if (ym2413[i].DAC_stream == -1)
			return 1;
	}
	return 0;
#endif

}

static void ym2413_stop (void *chip)
{
	struct ym2413_info *info = chip;
	YM2413Shutdown(info->chip);
}

static void ym2413_reset (void *chip)
{
	struct ym2413_info *info = chip;
	YM2413ResetChip(info->chip);
}


#ifdef YM2413ISA
WRITE8_HANDLER( YM2413_register_port_0_w ) {
int i,a;
	outportb(0x308,data); // ym2413_write (0, 0, data);
	//add delay
	for (i=0; i<0x20; i++)
		a = inportb(0x80);

 } /* 1st chip */
#else
WRITE8_HANDLER( YM2413_register_port_0_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 0); YM2413Write (info->chip, 0, data); } /* 1st chip */
#endif
WRITE8_HANDLER( YM2413_register_port_1_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 1); YM2413Write (info->chip, 0, data); } /* 2nd chip */
WRITE8_HANDLER( YM2413_register_port_2_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 2); YM2413Write (info->chip, 0, data); } /* 3rd chip */
WRITE8_HANDLER( YM2413_register_port_3_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 3); YM2413Write (info->chip, 0, data); } /* 4th chip */

#ifdef YM2413ISA
WRITE8_HANDLER( YM2413_data_port_0_w ) {
int i,a;
	outportb(0x309,data);// YM2413Write (sndti_token(SOUND_YM2413, 0), 1, data);
	//add delay
	for (i=0; i<0x40; i++)
		a = inportb(0x80);
 } /* 1st chip */
#else
WRITE8_HANDLER( YM2413_data_port_0_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 0); YM2413Write (info->chip, 1, data); } /* 1st chip */
#endif
WRITE8_HANDLER( YM2413_data_port_1_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 1); YM2413Write (info->chip, 1, data); } /* 2nd chip */
WRITE8_HANDLER( YM2413_data_port_2_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 2); YM2413Write (info->chip, 1, data); } /* 3rd chip */
WRITE8_HANDLER( YM2413_data_port_3_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 3); YM2413Write (info->chip, 1, data); } /* 4th chip */

WRITE16_HANDLER( YM2413_register_port_0_lsb_w ) { if (ACCESSING_LSB) YM2413_register_port_0_w(offset,data & 0xff); }
WRITE16_HANDLER( YM2413_register_port_0_msb_w ) { if (ACCESSING_MSB) YM2413_register_port_0_w(offset,((data & 0xff00) >> 8)); }
WRITE16_HANDLER( YM2413_register_port_1_lsb_w ) { if (ACCESSING_LSB) YM2413_register_port_1_w(offset,data & 0xff); }
WRITE16_HANDLER( YM2413_register_port_2_lsb_w ) { if (ACCESSING_LSB) YM2413_register_port_2_w(offset,data & 0xff); }
WRITE16_HANDLER( YM2413_register_port_3_lsb_w ) { if (ACCESSING_LSB) YM2413_register_port_3_w(offset,data & 0xff); }
WRITE16_HANDLER( YM2413_data_port_0_lsb_w ) { if (ACCESSING_LSB) YM2413_data_port_0_w(offset,data & 0xff); }
WRITE16_HANDLER( YM2413_data_port_0_msb_w ) { if (ACCESSING_MSB) YM2413_data_port_0_w(offset,((data & 0xff00) >> 8)); }
WRITE16_HANDLER( YM2413_data_port_1_lsb_w ) { if (ACCESSING_LSB) YM2413_data_port_1_w(offset,data & 0xff); }
WRITE16_HANDLER( YM2413_data_port_2_lsb_w ) { if (ACCESSING_LSB) YM2413_data_port_2_w(offset,data & 0xff); }
WRITE16_HANDLER( YM2413_data_port_3_lsb_w ) { if (ACCESSING_LSB) YM2413_data_port_3_w(offset,data & 0xff); }


/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void ym2413_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void ym2413_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = ym2413_set_info;		break;
		case SNDINFO_PTR_START:							info->start = ym2413_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = ym2413_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = ym2413_reset;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YM2413";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}
