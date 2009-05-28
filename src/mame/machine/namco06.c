/***************************************************************************

The following Namco custom chips are all instances of the same 4-bit MCU,
the Fujitsu MB8843 (42-pin DIP package) and MB8842/MB8844 (28-pin DIP),
differently programmed.

chip  MCU   pins function
---- ------ ---- --------
50XX MB8842  28  player score handling (protection)
51XX MB8843  42  I/O (coin management built-in)
52XX MB8843  42  sample playback
53XX MB8843  42  I/O (steering wheel support)
54XX MB8844  28  explosion (noise) generator

06XX interface:
---------------
Galaga                  51XX  ----  ----  54XX
Bosconian (CPU board)   51XX  ----  50XX  54XX
Bosconian (Video board) 50XX  52XX  ----  ----
Xevious                 51XX  ----  50XX  54XX
Dig Dug                 51XX  53XX  ----  ----
Pole Position / PP II   51XX  53XX  52XX  54XX


Pinouts:

        MB8843                   MB8842/MB8844
       +------+                    +------+
  EXTAL|1   42|Vcc            EXTAL|1   28|Vcc
   XTAL|2   41|K3              XTAL|2   27|K3
 /RESET|3   40|K2            /RESET|3   26|K2
   /IRQ|4   39|K1                O0|4   25|K1
     SO|5   38|K0                O1|5   24|K0
     SI|6   37|R15               O2|6   23|R10 /IRQ
/SC /TO|7   36|R14               O3|7   22|R9 /TC
    /TC|8   35|R13               O4|8   21|R8
     P0|9   34|R12               O5|9   20|R7
     P1|10  33|R11               O6|10  19|R6
     P2|11  32|R10               O7|11  18|R5
     P3|12  31|R9                R0|12  17|R4
     O0|13  30|R8                R1|13  16|R3
     O1|14  29|R7               GND|14  15|R2
     O2|15  28|R6                  +------+
     O3|16  27|R5
     O4|17  26|R4
     O5|18  25|R3
     O6|19  24|R2
     O7|20  23|R1
    GND|21  22|R0
       +------+


      O  O  R  R  R  K
50XX  O  O  I     I  I
54XX  O  O  I  O     I

      P  O  O  R  R  R  R  K
51XX  O  O  O  I  I  I  I  I
52XX  O  O  O  I  I  O  O  I
53XX  O? O  O  I  I  I  I  I


For the 52XX, see sound/namco52.c

For the 54XX, see audio/namco54.c

***************************************************************************/

#include "driver.h"
#include "machine/namco06.h"
#include "machine/namco50.h"
#include "machine/namco51.h"
#include "machine/namco53.h"
#include "audio/namco52.h"
#include "audio/namco54.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



typedef struct _namco_06xx_state namco_06xx_state;
struct _namco_06xx_state
{
	UINT8 command;
	emu_timer *nmi_timer;
	const device_config *nmicpu;
	const device_config *device[4];
	read8_device_func read[4];
	void (*readreq[4])(const device_config *device);
	write8_device_func write[4];
};

INLINE namco_06xx_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == NAMCO_06XX);

	return (namco_06xx_state *)device->token;
}



static TIMER_CALLBACK( nmi_generate )
{
	namco_06xx_state *state = get_safe_token((const device_config *)ptr);

	if (!cpu_is_suspended(state->nmicpu, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
	{
		LOG(("NMI cpu '%s'\n",state->nmicpu->tag));

		cpu_set_input_line(state->nmicpu, INPUT_LINE_NMI, PULSE_LINE);
	}
	else
		LOG(("NMI not generated because cpu '%s' is suspended\n",state->nmicpu->tag));
}


READ8_DEVICE_HANDLER( namco_06xx_data_r )
{
	namco_06xx_state *state = get_safe_token(device);

	LOG(("%s: 06XX '%s' read offset %d\n",cpuexec_describe_context(device->machine),device->tag,offset));

	if (!(state->command & 0x10))
	{
		logerror("%s: 06XX '%s' read in write mode %02x\n",cpuexec_describe_context(device->machine),device->tag,state->command);
		return 0;
	}

	switch (state->command & 0xf)
	{
		case 0x1: return (state->read[0] != NULL) ? (*state->read[0])(state->device[0],0) : 0xff;
		case 0x2: return (state->read[1] != NULL) ? (*state->read[1])(state->device[1],0) : 0xff;
		case 0x4: return (state->read[2] != NULL) ? (*state->read[2])(state->device[2],0) : 0xff;
		case 0x8: return (state->read[3] != NULL) ? (*state->read[3])(state->device[3],0) : 0xff;
		default:
			logerror("%s: 06XX '%s' read in unsupported mode %02x\n",cpuexec_describe_context(device->machine),device->tag,state->command);
			return 0xff;
	}
}


WRITE8_DEVICE_HANDLER( namco_06xx_data_w )
{
	namco_06xx_state *state = get_safe_token(device);

	LOG(("%s: 06XX '%s' write offset %d = %02x\n",cpuexec_describe_context(device->machine),device->tag,offset,data));

	if (state->command & 0x10)
	{
		logerror("%s: 06XX '%s' write in read mode %02x\n",cpuexec_describe_context(device->machine),device->tag,state->command);
		return;
	}

	switch (state->command & 0xf)
	{
		case 0x1: if (state->write[0] != NULL) (*state->write[0])(state->device[0],0,data); break;
		case 0x2: if (state->write[1] != NULL) (*state->write[1])(state->device[1],0,data); break;
		case 0x4: if (state->write[2] != NULL) (*state->write[2])(state->device[2],0,data); break;
		case 0x8: if (state->write[3] != NULL) (*state->write[3])(state->device[3],0,data); break;
		default:
			logerror("%s: 06XX '%s' write in unsupported mode %02x\n",cpuexec_describe_context(device->machine),device->tag,state->command);
			break;
	}
}


READ8_DEVICE_HANDLER( namco_06xx_ctrl_r )
{
	namco_06xx_state *state = get_safe_token(device);
	LOG(("%s: 06XX '%s' ctrl_r\n",cpuexec_describe_context(device->machine),device->tag));
	return state->command;
}

WRITE8_DEVICE_HANDLER( namco_06xx_ctrl_w )
{
	namco_06xx_state *state = get_safe_token(device);

	LOG(("%s: 06XX '%s' command %02x\n",cpuexec_describe_context(device->machine),device->tag,data));

	state->command = data;

	if ((state->command & 0x0f) == 0)
	{
		LOG(("disabling nmi generate timer\n"));
		timer_adjust_oneshot(state->nmi_timer, attotime_never, 0);
	}
	else
	{
		LOG(("setting nmi generate timer to 200us\n"));

		// this timing is critical. Due to a bug, Bosconian will stop responding to
		// inputs if a transfer terminates at the wrong time.
		// On the other hand, the time cannot be too short otherwise the 54XX will
		// not have enough time to process the incoming commands.
		timer_adjust_periodic(state->nmi_timer, ATTOTIME_IN_USEC(200), 0, ATTOTIME_IN_USEC(200));

		if (state->command & 0x10)
		{
			switch (state->command & 0xf)
			{
				case 0x1: if (state->readreq[0] != NULL) (*state->readreq[0])(state->device[0]); break;
				case 0x2: if (state->readreq[1] != NULL) (*state->readreq[1])(state->device[1]); break;
				case 0x4: if (state->readreq[2] != NULL) (*state->readreq[2])(state->device[2]); break;
				case 0x8: if (state->readreq[3] != NULL) (*state->readreq[3])(state->device[3]); break;
				default:
					logerror("%s: 06XX '%s' read in unsupported mode %02x\n",cpuexec_describe_context(device->machine),device->tag,state->command);
			}
		}
	}
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( namco_06xx )
{
	const namco_06xx_interface *config = (const namco_06xx_interface *)device->inline_config;
	namco_06xx_state *state = get_safe_token(device);
	int devnum;

	assert(config != NULL);

	/* resolve our CPU */
	state->nmicpu = cputag_get_cpu(device->machine, config->nmicpu);
	assert(state->nmicpu != NULL);

	/* resolve our devices */
	state->device[0] = (config->chip0 != NULL) ? devtag_get_device(device->machine, config->chip0) : NULL;
	assert(state->device[0] != NULL || config->chip0 == NULL);
	state->device[1] = (config->chip1 != NULL) ? devtag_get_device(device->machine, config->chip1) : NULL;
	assert(state->device[1] != NULL || config->chip1 == NULL);
	state->device[2] = (config->chip2 != NULL) ? devtag_get_device(device->machine, config->chip2) : NULL;
	assert(state->device[2] != NULL || config->chip2 == NULL);
	state->device[3] = (config->chip3 != NULL) ? devtag_get_device(device->machine, config->chip3) : NULL;
	assert(state->device[3] != NULL || config->chip3 == NULL);

	/* loop over devices and set their read/write handlers */
	for (devnum = 0; devnum < 4; devnum++)
		if (state->device[devnum] != NULL)
		{
			device_type type = state->device[devnum]->type;

			if (type == NAMCO_50XX)
			{
				state->read[devnum] = namco_50xx_read;
				state->readreq[devnum] = namco_50xx_read_request;
				state->write[devnum] = namco_50xx_write;
			}
			else if (type == NAMCO_51XX)
			{
				state->read[devnum] = namco_51xx_read;
				state->write[devnum] = namco_51xx_write;
			}
			else if (type == NAMCO_52XX)
				state->write[devnum] = namco_52xx_write;
			else if (type == NAMCO_53XX)
			{
				state->read[devnum] = namco_53xx_read;
				state->readreq[devnum] = namco_53xx_read_request;
			}
			else if (type == NAMCO_54XX)
				state->write[devnum] = namco_54xx_write;
			else
				fatalerror("Unknown device type %s connected to Namco 06xx", devtype_get_name(type));
		}

	/* allocate a timer */
	state->nmi_timer = timer_alloc(device->machine, nmi_generate, (void *)device);

	state_save_register_device_item(device, 0, state->command);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( namco_06xx )
{
	namco_06xx_state *state = get_safe_token(device);
	state->command = 0;
}


/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( namco_06xx )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(namco_06xx_state);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(namco_06xx_interface);			break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(namco_06xx); 	break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(namco_06xx); 	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Namco 06xx");					break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Namco I/O");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
