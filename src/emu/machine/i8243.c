/***************************************************************************

    i8243.c

    Intel 8243 Port Expander

    Copyright Aaron Giles

***************************************************************************/

#include "emu.h"
#include "i8243.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* live processor state */
typedef struct _i8243_state i8243_state;
struct _i8243_state
{
	UINT8		p[4];				/* 4 ports' worth of data */
	UINT8		p2out;				/* port 2 bits that will be returned */
	UINT8		p2;					/* most recent port 2 value */
	UINT8		opcode;				/* latched opcode */
	UINT8		prog;				/* previous PROG state */
};


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE i8243_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type == I8243);
	return (i8243_state *)device->token;
}


INLINE i8243_config *get_safe_config(running_device *device)
{
	assert(device != NULL);
	assert(device->type == I8243);
	return (i8243_config *)device->baseconfig().inline_config;
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    i8243_p2_r - handle a read from port 2
-------------------------------------------------*/

READ8_DEVICE_HANDLER( i8243_p2_r )
{
	i8243_state *i8243 = get_safe_token(device);
	return i8243->p2out;
}


/*-------------------------------------------------
    i8243_p2_r - handle a write to port 2
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( i8243_p2_w )
{
	i8243_state *i8243 = get_safe_token(device);
	i8243->p2 = data & 0x0f;
}


/*-------------------------------------------------
    i8243_prog_w - handle a change in the PROG
    line state
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( i8243_prog_w )
{
	i8243_state *i8243 = get_safe_token(device);
	i8243_config *config = get_safe_config(device);

	/* only care about low bit */
	data &= 1;

	/* on high->low transition state, latch opcode/port */
	if (i8243->prog && !data)
	{
		i8243->opcode = i8243->p2;

		/* if this is a read opcode, copy result to p2out */
		if ((i8243->opcode >> 2) == MCS48_EXPANDER_OP_READ)
		{
			if (config->readhandler != NULL)
				i8243->p[i8243->opcode & 3] = (*config->readhandler)(device, i8243->opcode & 3);
			i8243->p2out = i8243->p[i8243->opcode & 3] & 0x0f;
		}
	}

	/* on low->high transition state, act on opcode */
	else if (!i8243->prog && data)
	{
		switch (i8243->opcode >> 2)
		{
			case MCS48_EXPANDER_OP_WRITE:
				i8243->p[i8243->opcode & 3] = i8243->p2 & 0x0f;
				if (config->writehandler != NULL)
					(*config->writehandler)(device, i8243->opcode & 3, i8243->p[i8243->opcode & 3]);
				break;

			case MCS48_EXPANDER_OP_OR:
				i8243->p[i8243->opcode & 3] |= i8243->p2 & 0x0f;
				if (config->writehandler != NULL)
					(*config->writehandler)(device, i8243->opcode & 3, i8243->p[i8243->opcode & 3]);
				break;

			case MCS48_EXPANDER_OP_AND:
				i8243->p[i8243->opcode & 3] &= i8243->p2 & 0x0f;
				if (config->writehandler != NULL)
					(*config->writehandler)(device, i8243->opcode & 3, i8243->p[i8243->opcode & 3]);
				break;
		}
	}

	/* remember the state */
	i8243->prog = data;
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    DEVICE_START( i8243 )
-------------------------------------------------*/

static DEVICE_START( i8243 )
{
}


/*-------------------------------------------------
    DEVICE_RESET( i8243 )
-------------------------------------------------*/

static DEVICE_RESET( i8243 )
{
	i8243_state *i8243 = get_safe_token(device);

	i8243->p2 = 0x0f;
	i8243->p2out = 0x0f;
	i8243->prog = 1;
}


/*-------------------------------------------------
    DEVICE_GET_INFO( i8243 )
-------------------------------------------------*/

DEVICE_GET_INFO( i8243 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(i8243_state);			break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = sizeof(i8243_config);			break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:			info->start = DEVICE_START_NAME(i8243);					break;
		case DEVINFO_FCT_RESET:			info->reset = DEVICE_RESET_NAME(i8243);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "I8243");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "MCS-48");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						/* Nothing */							break;
	}
}
