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

#ifdef UNUSED_FUNCTION
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
#endif

static STREAM_UPDATE( ym2413_stream_update )
{
	struct ym2413_info *info = param;
	ym2413_update_one(info->chip, outputs, samples);
}

static void _stream_update(void *param, int interval)
{
	struct ym2413_info *info = param;
	stream_update(info->stream);
}

static SND_START( ym2413 )
{
	int rate = clock/72;
	struct ym2413_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	/* emulator create */
	info->chip = ym2413_init(device, clock, rate);
	if (!info->chip)
		return NULL;

	/* stream system initialize */
	info->stream = stream_create(device,0,2,rate,info,ym2413_stream_update);

	ym2413_set_update_handler(info->chip, _stream_update, info);

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

		ym2413[i].DAC_stream = stream_create(device, 0, 1, clock/72, i, YM2413DAC_update);

		if (ym2413[i].DAC_stream == -1)
			return 1;
	}
	return 0;
#endif

}

static SND_STOP( ym2413 )
{
	struct ym2413_info *info = device->token;
	ym2413_shutdown(info->chip);
}

static SND_RESET( ym2413 )
{
	struct ym2413_info *info = device->token;
	ym2413_reset_chip(info->chip);
}


#ifdef YM2413ISA
WRITE8_HANDLER( ym2413_register_port_0_w ) {
int i,a;
	outportb(0x308,data); // ym2413_write (0, 0, data);
	//add delay
	for (i=0; i<0x20; i++)
		a = inportb(0x80);

 } /* 1st chip */
#else
WRITE8_HANDLER( ym2413_register_port_0_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 0); ym2413_write (info->chip, 0, data); } /* 1st chip */
#endif
WRITE8_HANDLER( ym2413_register_port_1_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 1); ym2413_write (info->chip, 0, data); } /* 2nd chip */
WRITE8_HANDLER( ym2413_register_port_2_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 2); ym2413_write (info->chip, 0, data); } /* 3rd chip */
WRITE8_HANDLER( ym2413_register_port_3_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 3); ym2413_write (info->chip, 0, data); } /* 4th chip */

#ifdef YM2413ISA
WRITE8_HANDLER( ym2413_data_port_0_w ) {
int i,a;
	outportb(0x309,data);// ym2413_write (sndti_token(SOUND_YM2413, 0), 1, data);
	//add delay
	for (i=0; i<0x40; i++)
		a = inportb(0x80);
 } /* 1st chip */
#else
WRITE8_HANDLER( ym2413_data_port_0_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 0); ym2413_write (info->chip, 1, data); } /* 1st chip */
#endif
WRITE8_HANDLER( ym2413_data_port_1_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 1); ym2413_write (info->chip, 1, data); } /* 2nd chip */
WRITE8_HANDLER( ym2413_data_port_2_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 2); ym2413_write (info->chip, 1, data); } /* 3rd chip */
WRITE8_HANDLER( ym2413_data_port_3_w ) { struct ym2413_info *info = sndti_token(SOUND_YM2413, 3); ym2413_write (info->chip, 1, data); } /* 4th chip */

WRITE16_HANDLER( ym2413_register_port_0_lsb_w ) { if (ACCESSING_BITS_0_7) ym2413_register_port_0_w(space,offset,data & 0xff); }
WRITE16_HANDLER( ym2413_register_port_0_msb_w ) { if (ACCESSING_BITS_8_15) ym2413_register_port_0_w(space,offset,((data & 0xff00) >> 8)); }
WRITE16_HANDLER( ym2413_register_port_1_lsb_w ) { if (ACCESSING_BITS_0_7) ym2413_register_port_1_w(space,offset,data & 0xff); }
WRITE16_HANDLER( ym2413_register_port_2_lsb_w ) { if (ACCESSING_BITS_0_7) ym2413_register_port_2_w(space,offset,data & 0xff); }
WRITE16_HANDLER( ym2413_register_port_3_lsb_w ) { if (ACCESSING_BITS_0_7) ym2413_register_port_3_w(space,offset,data & 0xff); }
WRITE16_HANDLER( ym2413_data_port_0_lsb_w ) { if (ACCESSING_BITS_0_7) ym2413_data_port_0_w(space,offset,data & 0xff); }
WRITE16_HANDLER( ym2413_data_port_0_msb_w ) { if (ACCESSING_BITS_8_15) ym2413_data_port_0_w(space,offset,((data & 0xff00) >> 8)); }
WRITE16_HANDLER( ym2413_data_port_1_lsb_w ) { if (ACCESSING_BITS_0_7) ym2413_data_port_1_w(space,offset,data & 0xff); }
WRITE16_HANDLER( ym2413_data_port_2_lsb_w ) { if (ACCESSING_BITS_0_7) ym2413_data_port_2_w(space,offset,data & 0xff); }
WRITE16_HANDLER( ym2413_data_port_3_lsb_w ) { if (ACCESSING_BITS_0_7) ym2413_data_port_3_w(space,offset,data & 0xff); }


/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( ym2413 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( ym2413 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( ym2413 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( ym2413 );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( ym2413 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( ym2413 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YM2413";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
