/***************************************************************************

    Namco 06XX

    This chip is used as an interface to up to 4 other custom chips.
    It signals IRQs to the custom MCUs when writes happen, and generates
    NMIs to the controlling CPU to drive reads based on a clock.

    SD0-SD7 are data I/O lines connecting to the controlling CPU
    SEL selects either control (1) or data (0), usually connected to
        an address line of the controlling CPU
    /NMI is an NMI signal line for the controlling CPU

    ID0-ID7 are data I/O lines connecting to the other custom chips
    /IO1-/IO4 are IRQ signal lines for each custom chip

                   +------+
                [1]|1   28|Vcc
                ID7|2   27|SD7
                ID6|3   26|SD6
                ID5|4   25|SD5
                ID4|5   24|SD4
                ID3|6   23|SD3
                ID2|7   22|SD2
                ID1|8   21|SD1
                ID0|9   20|SD0
               /IO1|10  19|/NMI
               /IO2|11  18|/CS
               /IO3|12  17|CLOCK
               /IO4|13  16|R/W
                GND|14  15|SEL
                   +------+

    [1] on polepos, galaga, xevious, and bosco: connected to K3 of the 51xx
        on bosco and xevious, connected to R8 of the 50xx


    06XX interface:
    ---------------
    Galaga                  51XX  ----  ----  54XX
    Bosconian (CPU board)   51XX  ----  50XX  54XX
    Bosconian (Video board) 50XX  52XX  ----  ----
    Xevious                 51XX  ----  50XX  54XX
    Dig Dug                 51XX  53XX  ----  ----
    Pole Position / PP II   51XX  53XX  52XX  54XX


    Galaga writes:
        control = 10(000), data = FF at startup
        control = 71(011), read 3, control = 10
        control = A1(101), write 4, control = 10
        control = A8(101), write 12, control = 10

    Xevious writes:
        control = 10 at startup
        control = A1(101), write 6, control = 10
        control = 71(011), read 3, control = 10
        control = 64(011), write 1, control = 10
        control = 74(011), read 4, control = 10
        control = 68(011), write 7, control = 10

    Dig Dug writes:
        control = 10(000), data = 10 at startup
        control = A1(101), write 3, control = 10
        control = 71(011), read 3, control = 10
        control = D2(110), read 2, control = 10

    Bosco writes:
        control = 10(000), data = FF at startup
        control = C8(110), write 17, control = 10
        control = 61(011), write 1, control = 10
        control = 71(011), read 3, control = 10
        control = 94(100), read 4, control = 10
        control = 64(011), write 1, control = 10
        control = 84(100), write 5, control = 10


        control = 34(001), write 1, control = 10

***************************************************************************/

#include "emu.h"
#include "machine/namco06.h"
#include "machine/namco50.h"
#include "machine/namco51.h"
#include "machine/namco53.h"
#include "audio/namco52.h"
#include "audio/namco54.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



struct namco_06xx_state
{
	UINT8 m_control;
	emu_timer *m_nmi_timer;
	cpu_device *m_nmicpu;
	device_t *m_device[4];
	read8_device_func m_read[4];
	void (*m_readreq[4])(device_t *device);
	write8_device_func m_write[4];
};

INLINE namco_06xx_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == NAMCO_06XX);

	return (namco_06xx_state *)downcast<namco_06xx_device *>(device)->token();
}



static TIMER_CALLBACK( nmi_generate )
{
	namco_06xx_state *state = get_safe_token((device_t *)ptr);

	if (!state->m_nmicpu->suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
	{
		LOG(("NMI cpu '%s'\n",state->m_nmicpu->tag()));

		state->m_nmicpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
	else
		LOG(("NMI not generated because cpu '%s' is suspended\n",state->m_nmicpu->tag()));
}


READ8_DEVICE_HANDLER( namco_06xx_data_r )
{
	namco_06xx_state *state = get_safe_token(device);
	UINT8 result = 0xff;
	int devnum;

	LOG(("%s: 06XX '%s' read offset %d\n",device->machine().describe_context(),device->tag(),offset));

	if (!(state->m_control & 0x10))
	{
		logerror("%s: 06XX '%s' read in write mode %02x\n",device->machine().describe_context(),device->tag(),state->m_control);
		return 0;
	}

	for (devnum = 0; devnum < 4; devnum++)
		if ((state->m_control & (1 << devnum)) && state->m_read[devnum] != NULL)
			result &= (*state->m_read[devnum])(state->m_device[devnum], 0);

	return result;
}


WRITE8_DEVICE_HANDLER( namco_06xx_data_w )
{
	namco_06xx_state *state = get_safe_token(device);
	int devnum;

	LOG(("%s: 06XX '%s' write offset %d = %02x\n",device->machine().describe_context(),device->tag(),offset,data));

	if (state->m_control & 0x10)
	{
		logerror("%s: 06XX '%s' write in read mode %02x\n",device->machine().describe_context(),device->tag(),state->m_control);
		return;
	}

	for (devnum = 0; devnum < 4; devnum++)
		if ((state->m_control & (1 << devnum)) && state->m_write[devnum] != NULL)
			(*state->m_write[devnum])(state->m_device[devnum], 0, data);
}


READ8_DEVICE_HANDLER( namco_06xx_ctrl_r )
{
	namco_06xx_state *state = get_safe_token(device);
	LOG(("%s: 06XX '%s' ctrl_r\n",device->machine().describe_context(),device->tag()));
	return state->m_control;
}

WRITE8_DEVICE_HANDLER( namco_06xx_ctrl_w )
{
	namco_06xx_state *state = get_safe_token(device);
	int devnum;

	LOG(("%s: 06XX '%s' control %02x\n",device->machine().describe_context(),device->tag(),data));

	state->m_control = data;

	if ((state->m_control & 0x0f) == 0)
	{
		LOG(("disabling nmi generate timer\n"));
		state->m_nmi_timer->adjust(attotime::never);
	}
	else
	{
		LOG(("setting nmi generate timer to 200us\n"));

		// this timing is critical. Due to a bug, Bosconian will stop responding to
		// inputs if a transfer terminates at the wrong time.
		// On the other hand, the time cannot be too short otherwise the 54XX will
		// not have enough time to process the incoming controls.
		state->m_nmi_timer->adjust(attotime::from_usec(200), 0, attotime::from_usec(200));

		if (state->m_control & 0x10)
			for (devnum = 0; devnum < 4; devnum++)
				if ((state->m_control & (1 << devnum)) && state->m_readreq[devnum] != NULL)
					(*state->m_readreq[devnum])(state->m_device[devnum]);
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
	const namco_06xx_config *config = (const namco_06xx_config *)device->static_config();
	namco_06xx_state *state = get_safe_token(device);
	int devnum;

	assert(config != NULL);

	/* resolve our CPU */
	state->m_nmicpu = device->machine().device<cpu_device>(config->nmicpu);
	assert(state->m_nmicpu != NULL);

	/* resolve our devices */
	state->m_device[0] = (config->chip0 != NULL) ? device->machine().device(config->chip0) : NULL;
	assert(state->m_device[0] != NULL || config->chip0 == NULL);
	state->m_device[1] = (config->chip1 != NULL) ? device->machine().device(config->chip1) : NULL;
	assert(state->m_device[1] != NULL || config->chip1 == NULL);
	state->m_device[2] = (config->chip2 != NULL) ? device->machine().device(config->chip2) : NULL;
	assert(state->m_device[2] != NULL || config->chip2 == NULL);
	state->m_device[3] = (config->chip3 != NULL) ? device->machine().device(config->chip3) : NULL;
	assert(state->m_device[3] != NULL || config->chip3 == NULL);

	/* loop over devices and set their read/write handlers */
	for (devnum = 0; devnum < 4; devnum++)
		if (state->m_device[devnum] != NULL)
		{
			device_type type = state->m_device[devnum]->type();

			if (type == NAMCO_50XX)
			{
				state->m_read[devnum] = namco_50xx_read;
				state->m_readreq[devnum] = namco_50xx_read_request;
				state->m_write[devnum] = namco_50xx_write;
			}
			else if (type == NAMCO_51XX)
			{
				state->m_read[devnum] = namco_51xx_read;
				state->m_write[devnum] = namco_51xx_write;
			}
			else if (type == NAMCO_52XX)
				state->m_write[devnum] = namco_52xx_write;
			else if (type == NAMCO_53XX)
			{
				state->m_read[devnum] = namco_53xx_read;
				state->m_readreq[devnum] = namco_53xx_read_request;
			}
			else if (type == NAMCO_54XX)
				state->m_write[devnum] = namco_54xx_write;
			else
				fatalerror("Unknown device type %s connected to Namco 06xx\n", state->m_device[devnum]->name());
		}

	/* allocate a timer */
	state->m_nmi_timer = device->machine().scheduler().timer_alloc(FUNC(nmi_generate), (void *)device);

	device->save_item(NAME(state->m_control));
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( namco_06xx )
{
	namco_06xx_state *state = get_safe_token(device);
	state->m_control = 0;
}


const device_type NAMCO_06XX = &device_creator<namco_06xx_device>;

namco_06xx_device::namco_06xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NAMCO_06XX, "Namco 06xx", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(namco_06xx_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void namco_06xx_device::device_config_complete()
{
	m_shortname = "namco06xx";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_06xx_device::device_start()
{
	DEVICE_START_NAME( namco_06xx )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_06xx_device::device_reset()
{
	DEVICE_RESET_NAME( namco_06xx )(this);
}


