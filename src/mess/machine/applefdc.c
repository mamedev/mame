/*********************************************************************

    applefdc.c

    Implementation of various Apple Floppy Disk Controllers, including
    the classic Apple controller and the IWM (Integrated Woz Machine)
    chip

    The IWM chip was used as the floppy disk controller for early Macs and the
    Apple IIgs, and was eventually superseded by the SWIM chp.

    Nate Woods
    Raphael Nabet

    Writing this code would not be possible if it weren't for the work of the
    XGS and KEGS emulators which also contain IWM emulations.

    TODO
      - Implement the unimplemented IWM modes
            - IWM_MODE_CLOCKSPEED
            - IWM_MODE_BITCELLTIME
            - IWM_MODE_HANDSHAKEPROTOCOL
            - IWM_MODE_LATCHMODE
      - Investigate the differences between the IWM and the classic Apple II
        controller more fully.  It is currently unclear what are genuine
        differences and what are effectively hacks that "just seem" to work.
      - Figure out iwm_readenable2handshake() and iwm_enable2(); they are
        hackish at best
      - Support the SWIM chip
      - Proper timing
      - This code was originally IWM specific; we need to clean up IWMisms in
        the code
      - Make it faster?
      - Add sound?

*********************************************************************/

#include "applefdc.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

/* logging */
#define LOG_APPLEFDC		0
#define LOG_APPLEFDC_EXTRA	0



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* mask for FDC lines */
#define IWM_MOTOR	0x10
#define IWM_DRIVE	0x20
#define IWM_Q6		0x40
#define IWM_Q7		0x80

enum applefdc_t
{
	APPLEFDC_APPLE2,	/* classic Apple II disk controller (pre-IWM) */
	APPLEFDC_IWM,		/* Integrated Woz Machine */
	APPLEFDC_SWIM		/* Sander/Woz Integrated Machine */
};


static UINT8 swim_default_parms[16] =
{
	0x38, 0x18, 0x41, 0x2e, 0x2e, 0x18, 0x18, 0x1b,
	0x1b, 0x2f, 0x2f, 0x19, 0x19, 0x97, 0x1b, 0x57
};

/***************************************************************************
    IWM MODE

    The IWM mode has the following values:

    Bit 7     Reserved
    Bit 6     Reserved
    Bit 5     Reserved
    Bit 4   ! Clock speed
                0=7MHz; used by Apple IIgs
                1=8MHz; used by Mac (I believe)
    Bit 3   ! Bit cell time
                0=4usec/bit (used for 5.25" drives)
                1=2usec/bit (used for 3.5" drives)
    Bit 2     Motor-off delay
                0=leave on for 1 sec after system turns it off
                1=turn off immediately
    Bit 1   ! Handshake protocol
                0=synchronous (software supplies timing for writing data; used for 5.25" drives)
                1=asynchronous (IWM supplies timing; used for 3.5" drives)
    Bit 0   ! Latch mode
                0=read data stays valid for 7usec (used for 5.25" drives)
                1=read data stays valid for full byte time (used for 3.5" drives)

 ***************************************************************************/

enum
{
	IWM_MODE_CLOCKSPEED			= 0x10,
	IWM_MODE_BITCELLTIME		= 0x08,
	IWM_MODE_MOTOROFFDELAY		= 0x04,
	IWM_MODE_HANDSHAKEPROTOCOL	= 0x02,
	IWM_MODE_LATCHMODE			= 0x01
};

enum
{
	SWIM_MODE_IWM,
	SWIM_MODE_SWIM,
	SWIM_MODE_SWIM2,
	SWIM_MODE_SWIM3
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct applefdc_token
{
	/* data that is constant for the lifetime of the emulation */
	emu_timer *motor_timer;
	applefdc_t type;

	/* data that changes at emulation time */
	UINT8 write_byte;
	UINT8 lines;					/* flags from IWM_MOTOR - IWM_Q7 */
	UINT8 mode;						/* 0-31; see above */
	UINT8 handshake_hack;			/* not sure what this is for */

	/* SWIM extentions */
	UINT8 swim_mode;
	UINT8 swim_magic_state;
	UINT8 ism_regs[8];
	UINT8 parm_offset;
	UINT8 parms[16];
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK(iwm_turnmotor_onoff);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE void assert_is_applefdc(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == APPLEFDC) || (device->type() == IWM) || (device->type() == SWIM));
}



INLINE applefdc_token *get_token(device_t *device)
{
	assert_is_applefdc(device);
	return (applefdc_token *) downcast<applefdc_base_device *>(device)->token();
}



INLINE const applefdc_interface *get_interface(device_t *device)
{
	static const applefdc_interface dummy_interface = {0, };

	assert_is_applefdc(device);
	return (device->static_config() != NULL)
		? (const applefdc_interface *) device->static_config()
		: &dummy_interface;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    applefdc_start - starts up an FDC
-------------------------------------------------*/

static void applefdc_start(device_t *device, applefdc_t type)
{
	applefdc_token *fdc = get_token(device);

	memset(fdc, 0, sizeof(*fdc));
	fdc->type = type;
	fdc->motor_timer = device->machine().scheduler().timer_alloc(FUNC(iwm_turnmotor_onoff), (void *) device);
	fdc->lines = 0x00;
	fdc->mode = 0x1F;	/* default value needed by Lisa 2 - no, I don't know if it is true */
	fdc->swim_mode = SWIM_MODE_IWM;

	/* register save states */
	state_save_register_item(device->machine(), "applefdc", NULL, 0, fdc->write_byte);
	state_save_register_item(device->machine(), "applefdc", NULL, 0, fdc->lines);
	state_save_register_item(device->machine(), "applefdc", NULL, 0, fdc->mode);
	state_save_register_item(device->machine(), "applefdc", NULL, 0, fdc->handshake_hack);
}



/*-------------------------------------------------
    DEVICE_RESET(applefdc) - resets an FDC
-------------------------------------------------*/

static DEVICE_RESET(applefdc)
{
	applefdc_token *fdc = get_token(device);

	fdc->handshake_hack = 0x00;
	fdc->write_byte = 0x00;
	fdc->lines = 0x00;
	fdc->mode = 0x1F;	/* default value needed by Lisa 2 - no, I don't know if it is true */
	fdc->swim_magic_state = 0;
	fdc->swim_mode = SWIM_MODE_IWM;
	fdc->parm_offset = 0;

	// setup SWIM default parms if it's a SWIM
	if (fdc->type == APPLEFDC_SWIM)
	{
		for (int i = 0; i < 16; i++)
		{
			fdc->parms[i] = swim_default_parms[i];
		}
	}

	fdc->motor_timer->reset();
}



/*-------------------------------------------------
    iwm_enable2 - hackish function
-------------------------------------------------*/

static int iwm_enable2(device_t *device)
{
	applefdc_token *fdc = get_token(device);

	/* R. Nabet : This function looks more like a hack than a real feature of the IWM; */
	/* it is not called from the Mac Plus driver */
	return (fdc->lines & APPLEFDC_PH1) && (fdc->lines & APPLEFDC_PH3);
}



/*-------------------------------------------------
    iwm_readenable2handshake - hackish function
-------------------------------------------------*/

static UINT8 iwm_readenable2handshake(device_t *device)
{
	applefdc_token *fdc = get_token(device);

	/* R. Nabet : This function looks more like a hack than a real feature of the IWM; */
	/* it is not called from the Mac Plus driver */
	fdc->handshake_hack++;
	fdc->handshake_hack %= 4;
	return (fdc->handshake_hack != 0) ? 0xc0 : 0x80;
}



/*-------------------------------------------------
    applefdc_statusreg_r - reads the status register
-------------------------------------------------*/

static UINT8 applefdc_statusreg_r(device_t *device)
{
	UINT8 result;
	int status;
	applefdc_token *fdc = get_token(device);
	const applefdc_interface *intf = get_interface(device);

	/* IWM status:
     *
     * Bit 7    Sense input (write protect for 5.25" drive and general status line for 3.5")
     * Bit 6    Reserved
     * Bit 5    Drive enable (is 1 if drive is on)
     * Bits 4-0 Same as IWM mode bits 4-0
     */

	status = iwm_enable2(device) ? 1 : (intf->read_status ? intf->read_status(device) : 0);

	result = (status ? 0x80 : 0x00);

	if (fdc->type != APPLEFDC_APPLE2)
		 result |= (((fdc->lines & IWM_MOTOR) ? 1 : 0) << 5) | fdc->mode;
	return result;
}



/*-------------------------------------------------
    iwm_modereg_w - changes the mode register
-------------------------------------------------*/

static void iwm_modereg_w(device_t *device, UINT8 data)
{
	applefdc_token *fdc = get_token(device);

	fdc->mode = data & 0x1f;	/* write mode register */

	// SWIM mode is unlocked by writing 1/0/1/1 in a row to bit 6 (which is unused on IWM)
	// when SWIM mode engages, the IWM is disconnected from both the 68k and the drives,
	// and the ISM is substituted.
	if (fdc->type == APPLEFDC_SWIM)
	{
		switch (fdc->swim_magic_state)
		{
			case 0:
			case 2:
			case 3:
				if (data & 0x40)
				{
					fdc->swim_magic_state++;
				}
				else
				{
					fdc->swim_magic_state = 0;
				}
				break;
			case 1:
				if (!(data & 0x40))
				{
					fdc->swim_magic_state++;
				}
				else
				{
					fdc->swim_magic_state = 0;
				}
				break;
		}

		if (fdc->swim_magic_state == 4)
		{
			fdc->swim_magic_state = 0;
//          printf("IWM: switching to SWIM mode\n");
			fdc->swim_mode = SWIM_MODE_SWIM;
		}
	}

	if (LOG_APPLEFDC_EXTRA)
		logerror("iwm_modereg_w: iwm_mode=0x%02x\n", (unsigned) fdc->mode);
}



/*-------------------------------------------------
    applefdc_read_reg - reads a register
-------------------------------------------------*/

static UINT8 applefdc_read_reg(device_t *device, int lines)
{
	applefdc_token *fdc = get_token(device);
	const applefdc_interface *intf = get_interface(device);
	UINT8 result = 0;

	switch(lines)
	{
		case 0:
			/* Read data register */
			if ((fdc->type != APPLEFDC_APPLE2) && (iwm_enable2(device) || !(fdc->lines & IWM_MOTOR)))
			{
				result = 0xFF;
			}
			else
			{
				/*
                         * Right now, this function assumes latch mode; which is always used for
                         * 3.5 inch drives.  Eventually we should check to see if latch mode is
                         * off
                         */
				if (LOG_APPLEFDC)
				{
					if ((fdc->mode & IWM_MODE_LATCHMODE) == 0x00)
						logerror("applefdc_read_reg(): latch mode off not implemented\n");
				}

				result = (intf->read_data ? intf->read_data(device) : 0x00);
			}
			break;

		case IWM_Q6:
			/* Read status register */
			result = applefdc_statusreg_r(device);
			break;

		case IWM_Q7:
			/* Classic Apple II: Read status register
             * IWM: Read handshake register
             */
			if (fdc->type == APPLEFDC_APPLE2)
				result = applefdc_statusreg_r(device);
			else
				result = iwm_enable2(device) ? iwm_readenable2handshake(device) : 0x80;
			break;
	}
	return result;
}



/*-------------------------------------------------
    applefdc_write_reg - writes a register
-------------------------------------------------*/

static void applefdc_write_reg(device_t *device, UINT8 data)
{
	applefdc_token *fdc = get_token(device);
	const applefdc_interface *intf = get_interface(device);

	switch(fdc->lines & (IWM_Q6 | IWM_Q7))
	{
		case IWM_Q6 | IWM_Q7:
			if (!(fdc->lines & IWM_MOTOR))
			{
				iwm_modereg_w(device, data);
			}
			else if (!iwm_enable2(device))
			{
				/*
                         * Right now, this function assumes latch mode; which is always used for
                         * 3.5 inch drives.  Eventually we should check to see if latch mode is
                         * off
                         */
				if (LOG_APPLEFDC)
				{
					if ((fdc->mode & IWM_MODE_LATCHMODE) == 0)
						logerror("applefdc_write_reg(): latch mode off not implemented\n");
				}

				if (intf->write_data != NULL)
					intf->write_data(device,data);
			}
			break;
	}
}



/*-------------------------------------------------
    TIMER_CALLBACK(iwm_turnmotor_onoff) - timer
    callback for turning motor on or off
-------------------------------------------------*/

static TIMER_CALLBACK(iwm_turnmotor_onoff)
{
	device_t *device = (device_t *) ptr;
	applefdc_token *fdc = get_token(device);
	const applefdc_interface *intf = get_interface(device);
	int status = param;
	int enable_lines;

	if (status != 0)
	{
		fdc->lines |= IWM_MOTOR;
		enable_lines = (fdc->lines & IWM_DRIVE) ? 2 : 1;
	}
	else
	{
		fdc->lines &= ~IWM_MOTOR;

		if (fdc->type == APPLEFDC_APPLE2)
			enable_lines = (fdc->lines & IWM_DRIVE) ? 2 : 1;
		else
			enable_lines = 0;
	}

	/* invoke callback, if present */
	if (intf->set_enable_lines != NULL)
		intf->set_enable_lines(device,enable_lines);

	if (LOG_APPLEFDC_EXTRA)
		logerror("iwm_turnmotor_onoff(): Turning motor %s\n", status ? "on" : "off");
}



/*-------------------------------------------------
    iwm_access
-------------------------------------------------*/

static void iwm_access(device_t *device, int offset)
{
	static const char *const lines[] =
	{
		"PH0",
		"PH1",
		"PH2",
		"PH3",
		"MOTOR",
		"DRIVE",
		"Q6",
		"Q7"
	};

	applefdc_token *fdc = get_token(device);
	const applefdc_interface *intf = get_interface(device);

	if (offset & 1)
		fdc->lines |= (1 << (offset >> 1));
	else
		fdc->lines &= ~(1 << (offset >> 1));

	if (LOG_APPLEFDC_EXTRA)
	{
		logerror("iwm_access(): %s line %s => %02x\n",
			(offset & 1) ? "setting" : "clearing", lines[offset >> 1], fdc->lines);
	}

	if ((offset < 0x08) && (intf->set_lines != NULL))
		intf->set_lines(device,fdc->lines & 0x0f);

	switch(offset)
	{
		case 0x08:
			/* turn off motor */
			fdc->motor_timer->adjust(
				(fdc->mode & IWM_MODE_MOTOROFFDELAY) ? attotime::zero : attotime::from_seconds(1), 0);
			break;

		case 0x09:
			/* turn on motor */
			fdc->motor_timer->adjust(attotime::zero, 1);
			break;

		case 0x0A:
			/* turn off IWM_DRIVE */
			if ((fdc->lines & IWM_MOTOR) && (intf->set_enable_lines != NULL))
				intf->set_enable_lines(device,1);
			break;

		case 0x0B:
			/* turn on IWM_DRIVE */
			if ((fdc->lines & IWM_MOTOR) && (intf->set_enable_lines != NULL))
				intf->set_enable_lines(device,2);
			break;
	}
}



/*-------------------------------------------------
    applefdc_r - reads a byte from the FDC
-------------------------------------------------*/

READ8_DEVICE_HANDLER( applefdc_r )
{
	applefdc_token *fdc = get_token(device);
	const applefdc_interface *intf = get_interface(device);
	UINT8 result = 0;

	/* normalize offset */
	offset &= 0xf;

	if (LOG_APPLEFDC_EXTRA)
		logerror("applefdc_r: offset=%i\n", offset);

	if ((fdc->type < APPLEFDC_SWIM) || (fdc->swim_mode == SWIM_MODE_IWM))
	{
		iwm_access(device, offset);

		switch(fdc->type)
		{
			case APPLEFDC_APPLE2:
				switch(offset)
				{
					case 0x0C:
						if (fdc->lines & IWM_Q7)
						{
							if (intf->write_data != NULL)
								intf->write_data(device,fdc->write_byte);
							result = 0;
						}
						else
							result = applefdc_read_reg(device, 0);

						break;
					case 0x0D:
						result = applefdc_read_reg(device, IWM_Q6);
						break;
					case 0x0E:
						result = applefdc_read_reg(device, IWM_Q7);
						break;
					case 0x0F:
						result = applefdc_read_reg(device, IWM_Q7 | IWM_Q6);
						break;
				}
				break;

			case APPLEFDC_IWM:
				if ((offset & 1) == 0)
					result = applefdc_read_reg(device, fdc->lines & (IWM_Q6 | IWM_Q7));
				break;

			case APPLEFDC_SWIM:
				if ((offset & 1) == 0)
					result = applefdc_read_reg(device, fdc->lines & (IWM_Q6 | IWM_Q7));
				break;
		}
	}
	else if (fdc->swim_mode >= SWIM_MODE_SWIM)
	{
		// reading parameter RAM?
		if ((offset & 7) == 3)
		{
			result = fdc->parms[fdc->parm_offset++];
			fdc->parm_offset &= 0xf;
		}
		else
		{
			result = fdc->ism_regs[offset&7];
		}
		printf("SWIM: read %02x from offset %x\n", result, offset & 7);
	}

	return result;
}



/*-------------------------------------------------
    applefdc_w - writes a byte to the FDC
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( applefdc_w )
{
	applefdc_token *fdc = get_token(device);
	const applefdc_interface *intf = get_interface(device);

	/* normalize offset */
	offset &= 15;

	if (LOG_APPLEFDC_EXTRA)
		logerror("applefdc_w: offset=%i data=0x%02x\n", offset, data);

	if ((fdc->type < APPLEFDC_SWIM) || (fdc->swim_mode == SWIM_MODE_IWM))
	{
		iwm_access(device, offset);

		switch(fdc->type)
		{
			case APPLEFDC_APPLE2:
				switch(offset)
				{
					case 0x0C:
						if (fdc->lines & IWM_Q7)
						{
							if (intf->write_data != NULL)
								intf->write_data(device,fdc->write_byte);
						}
						break;

					case 0x0D:
						fdc->write_byte = data;
						break;
				}
				break;

			case APPLEFDC_IWM:
				if (offset & 1)
					applefdc_write_reg(device, data);
				break;

			case APPLEFDC_SWIM:
				if (offset & 1)
					applefdc_write_reg(device, data);
				break;
		}
	}
	else if (fdc->swim_mode >= SWIM_MODE_SWIM)
	{
		printf("SWIM: write %02x to offset %x\n", data, offset & 7);
		switch (offset & 7)
		{
			case 2: // write CRC
				break;

			case 3: // write parameter
				fdc->parms[fdc->parm_offset++] = data;
				fdc->parm_offset &= 0xf;
				break;

			case 6: // write zeros to status (also zeroes parameter RAM pointer)
				fdc->ism_regs[6] &= ~data;
				fdc->parm_offset = 0;

				if (data == 0xf8)	// magic "revert to IWM" value
				{
					printf("SWIM: reverting to IWM\n");
					fdc->swim_mode = SWIM_MODE_IWM;
				}
				break;

			case 7: // write ones to status
				fdc->ism_regs[6] |= data;
				break;

			default:
				fdc->ism_regs[offset & 7] = data;
				break;

		}
	}
}



/*-------------------------------------------------
    applefdc_w - writes a byte to the FDC
-------------------------------------------------*/

UINT8 applefdc_get_lines(device_t *device)
{
	applefdc_token *fdc = get_token(device);
	return fdc->lines & 0x0f;
}



/***************************************************************************
    INTERFACE
***************************************************************************/

/*-------------------------------------------------
    DEVICE_START(oldfdc) - device start
    callback
-------------------------------------------------*/

static DEVICE_START(oldfdc)
{
	applefdc_start(device, APPLEFDC_APPLE2);
}



/*-------------------------------------------------
    DEVICE_START(iwm) - device start
    callback
-------------------------------------------------*/

static DEVICE_START(iwm)
{
	applefdc_start(device, APPLEFDC_IWM);
}


/*-------------------------------------------------
    DEVICE_START(iwm) - device start
    callback
-------------------------------------------------*/

static DEVICE_START(swim)
{
	applefdc_start(device, APPLEFDC_SWIM);
}


applefdc_base_device::applefdc_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock)
{
	m_token = global_alloc_clear(applefdc_token);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void applefdc_base_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void applefdc_base_device::device_reset()
{
	DEVICE_RESET_NAME( applefdc )(this);
}


const device_type APPLEFDC = &device_creator<applefdc_device>;

applefdc_device::applefdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: applefdc_base_device(mconfig, APPLEFDC, "Apple FDC", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void applefdc_device::device_start()
{
	DEVICE_START_NAME( oldfdc )(this);
}


const device_type IWM = &device_creator<iwm_device>;

iwm_device::iwm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: applefdc_base_device(mconfig, IWM, "Apple IWM (Integrated Woz Machine)", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iwm_device::device_start()
{
	DEVICE_START_NAME( iwm )(this);
}


const device_type SWIM = &device_creator<swim_device>;

swim_device::swim_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: applefdc_base_device(mconfig, SWIM, "Apple SWIM (Steve Woz Integrated Machine)", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void swim_device::device_start()
{
	DEVICE_START_NAME( swim )(this);
}


