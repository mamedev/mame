/***************************************************************************

  2151intf.c

  Support interface YM2151(OPM)

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "fm.h"
#include "2151intf.h"
#include "ym2151.h"


struct ym2151_info
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	const ym2151_interface *intf;
};


static STREAM_UPDATE( ym2151_update )
{
	struct ym2151_info *info = param;
	ym2151_update_one(info->chip, outputs, samples);
}


static STATE_POSTLOAD( ym2151intf_postload )
{
	struct ym2151_info *info = param;
	ym2151_postload(machine, info->chip);
}


static SND_START( ym2151 )
{
	static const ym2151_interface dummy = { 0 };
	struct ym2151_info *info;
	int rate;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = device->static_config ? device->static_config : &dummy;

	rate = clock/64;

	/* stream setup */
	info->stream = stream_create(device,0,2,rate,info,ym2151_update);

	info->chip = ym2151_init(device,clock,rate);

	state_save_register_postload(device->machine, ym2151intf_postload, info);

	if (info->chip != 0)
	{
		ym2151_set_irq_handler(info->chip,info->intf->irqhandler);
		ym2151_set_port_write_handler(info->chip,info->intf->portwritehandler);
		return info;
	}
	return NULL;
}


static SND_STOP( ym2151 )
{
	struct ym2151_info *info = device->token;
	ym2151_shutdown(info->chip);
}

static SND_RESET( ym2151 )
{
	struct ym2151_info *info = device->token;
	ym2151_reset_chip(info->chip);
}

static int lastreg0,lastreg1,lastreg2;

READ8_HANDLER( ym2151_status_port_0_r )
{
	struct ym2151_info *token = sndti_token(SOUND_YM2151, 0);
	stream_update(token->stream);
	return ym2151_read_status(token->chip);
}

READ8_HANDLER( ym2151_status_port_1_r )
{
	struct ym2151_info *token = sndti_token(SOUND_YM2151, 1);
	stream_update(token->stream);
	return ym2151_read_status(token->chip);
}

READ8_HANDLER( ym2151_status_port_2_r )
{
	struct ym2151_info *token = sndti_token(SOUND_YM2151, 2);
	stream_update(token->stream);
	return ym2151_read_status(token->chip);
}

WRITE8_HANDLER( ym2151_register_port_0_w )
{
	lastreg0 = data;
}
WRITE8_HANDLER( ym2151_register_port_1_w )
{
	lastreg1 = data;
}
WRITE8_HANDLER( ym2151_register_port_2_w )
{
	lastreg2 = data;
}

WRITE8_HANDLER( ym2151_data_port_0_w )
{
	struct ym2151_info *token = sndti_token(SOUND_YM2151, 0);
	stream_update(token->stream);
	ym2151_write_reg(token->chip,lastreg0,data);
}

WRITE8_HANDLER( ym2151_data_port_1_w )
{
	struct ym2151_info *token = sndti_token(SOUND_YM2151, 1);
	stream_update(token->stream);
	ym2151_write_reg(token->chip,lastreg1,data);
}

WRITE8_HANDLER( ym2151_data_port_2_w )
{
	struct ym2151_info *token = sndti_token(SOUND_YM2151, 2);
	stream_update(token->stream);
	ym2151_write_reg(token->chip,lastreg2,data);
}

WRITE8_HANDLER( ym2151_word_0_w )
{
	if (offset)
		ym2151_data_port_0_w(space,0,data);
	else
		ym2151_register_port_0_w(space,0,data);
}

WRITE8_HANDLER( ym2151_word_1_w )
{
	if (offset)
		ym2151_data_port_1_w(space,0,data);
	else
		ym2151_register_port_1_w(space,0,data);
}

READ16_HANDLER( ym2151_status_port_0_lsb_r )
{
	return ym2151_status_port_0_r(space,0);
}

READ16_HANDLER( ym2151_status_port_1_lsb_r )
{
	return ym2151_status_port_1_r(space,0);
}

READ16_HANDLER( ym2151_status_port_2_lsb_r )
{
	return ym2151_status_port_2_r(space,0);
}


WRITE16_HANDLER( ym2151_register_port_0_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		ym2151_register_port_0_w(space, 0, data & 0xff);
}

WRITE16_HANDLER( ym2151_register_port_1_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		ym2151_register_port_1_w(space, 0, data & 0xff);
}

WRITE16_HANDLER( ym2151_register_port_2_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		ym2151_register_port_2_w(space, 0, data & 0xff);
}

WRITE16_HANDLER( ym2151_data_port_0_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		ym2151_data_port_0_w(space, 0, data & 0xff);
}

WRITE16_HANDLER( ym2151_data_port_1_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		ym2151_data_port_1_w(space, 0, data & 0xff);
}

WRITE16_HANDLER( ym2151_data_port_2_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		ym2151_data_port_2_w(space, 0, data & 0xff);
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( ym2151 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( ym2151 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( ym2151 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( ym2151 );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( ym2151 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( ym2151 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "YM2151");							break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");								break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);							break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
