// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
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
    CONSTANTS
***************************************************************************/

// logging
#define LOG_APPLEFDC        0
#define LOG_APPLEFDC_EXTRA  0

// mask for FDC lines
#define IWM_MOTOR           0x10
#define IWM_DRIVE           0x20
#define IWM_Q6              0x40
#define IWM_Q7              0x80

const device_timer_id TIMER_MOTOR_ONOFF = 1;





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
	IWM_MODE_CLOCKSPEED         = 0x10,
	IWM_MODE_BITCELLTIME        = 0x08,
	IWM_MODE_MOTOROFFDELAY      = 0x04,
	IWM_MODE_HANDSHAKEPROTOCOL  = 0x02,
	IWM_MODE_LATCHMODE          = 0x01
};



/***************************************************************************
    BASE DEVICE
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

applefdc_base_device::applefdc_base_device(applefdc_base_device::applefdc_t fdc_type, const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
	m_type = fdc_type;
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void applefdc_base_device::device_start()
{
	// timer
	m_motor_timer = timer_alloc(TIMER_MOTOR_ONOFF);

	// state
	m_write_byte        = 0x00;
	m_lines             = 0x00;
	m_mode              = 0x1F; // default value needed by Lisa 2 - no, I don't know if it is true
	m_handshake_hack    = 0x00;

	// register save states
	save_item(NAME(m_write_byte));
	save_item(NAME(m_lines));
	save_item(NAME(m_mode));
	save_item(NAME(m_handshake_hack));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void applefdc_base_device::device_reset(void)
{
	m_handshake_hack = 0x00;
	m_write_byte = 0x00;
	m_lines = 0x00;
	m_mode = 0x1F;  /* default value needed by Lisa 2 - no, I don't know if it is true */
	m_motor_timer->reset();
}



//-------------------------------------------------
//  device_timer - device-specific timer callbacks
//-------------------------------------------------

void applefdc_base_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_MOTOR_ONOFF:
			turn_motor_onoff(param != 0);
			break;
	}
}



//-------------------------------------------------
//  get_interface - gets the interface
//-------------------------------------------------

const applefdc_interface *applefdc_base_device::get_interface()
{
	static const applefdc_interface dummy_interface = {nullptr, };

	return (static_config() != nullptr)
		? (const applefdc_interface *) static_config()
		: &dummy_interface;
}



//-------------------------------------------------
//  iwm_enable2 - hackish function
//-------------------------------------------------

int applefdc_base_device::iwm_enable2()
{
	/* R. Nabet : This function looks more like a hack than a real feature of the IWM; */
	/* it is not called from the Mac Plus driver */
	return (m_lines & APPLEFDC_PH1) && (m_lines & APPLEFDC_PH3);
}



//-------------------------------------------------
//  iwm_readenable2handshake - hackish function
//-------------------------------------------------

UINT8 applefdc_base_device::iwm_readenable2handshake()
{
	/* R. Nabet : This function looks more like a hack than a real feature of the IWM; */
	/* it is not called from the Mac Plus driver */
	m_handshake_hack++;
	m_handshake_hack %= 4;
	return (m_handshake_hack != 0) ? 0xc0 : 0x80;
}



//-------------------------------------------------
//  statusreg_r - reads the status register
//-------------------------------------------------

UINT8 applefdc_base_device::statusreg_r()
{
	UINT8 result;
	int status;
	const applefdc_interface *intf = get_interface();

	/* IWM status:
	 *
	 * Bit 7    Sense input (write protect for 5.25" drive and general status line for 3.5")
	 * Bit 6    Reserved
	 * Bit 5    Drive enable (is 1 if drive is on)
	 * Bits 4-0 Same as IWM mode bits 4-0
	 */

	status = iwm_enable2() ? 1 : (intf->read_status ? intf->read_status(this) : 0);

	result = (status ? 0x80 : 0x00);

	if (m_type != APPLEFDC_APPLE2)
			result |= (((m_lines & IWM_MOTOR) ? 1 : 0) << 5) | m_mode;
	return result;
}



//-------------------------------------------------
//  iwm_modereg_w - changes the mode register
//-------------------------------------------------

void applefdc_base_device::iwm_modereg_w(UINT8 data)
{
	m_mode = data & 0x1f;   /* write mode register */

	if (LOG_APPLEFDC_EXTRA)
		logerror("iwm_modereg_w: iwm_mode=0x%02x\n", (unsigned) m_mode);
}



//-------------------------------------------------
//  read_reg - reads a register
//-------------------------------------------------

UINT8 applefdc_base_device::read_reg(int lines)
{
	const applefdc_interface *intf = get_interface();
	UINT8 result = 0;

	switch(lines)
	{
		case 0:
			// read data register
			if ((m_type != APPLEFDC_APPLE2) && (iwm_enable2() || !(m_lines & IWM_MOTOR)))
			{
				result = 0xFF;
			}
			else
			{
				// Right now, this function assumes latch mode; which is always used for
				// 3.5 inch drives.  Eventually we should check to see if latch mode is
				// off
				if (LOG_APPLEFDC)
				{
					if ((m_mode & IWM_MODE_LATCHMODE) == 0x00)
						logerror("applefdc_read_reg(): latch mode off not implemented\n");
				}

				result = (intf->read_data ? intf->read_data(this) : 0x00);
			}
			break;

		case IWM_Q6:
			// read status register
			result = statusreg_r();
			break;

		case IWM_Q7:
			// Classic Apple II: Read status register
			// IWM: Read handshake register
			if (m_type == APPLEFDC_APPLE2)
				result = statusreg_r();
			else
				result = iwm_enable2() ? iwm_readenable2handshake() : 0x80;
			break;
	}
	return result;
}



//-------------------------------------------------
//  write_reg - writes a register
//-------------------------------------------------

void applefdc_base_device::write_reg(UINT8 data)
{
	const applefdc_interface *intf = get_interface();

	switch(m_lines & (IWM_Q6 | IWM_Q7))
	{
		case IWM_Q6 | IWM_Q7:
			if (!(m_lines & IWM_MOTOR))
			{
				iwm_modereg_w(data);
			}
			else if (!iwm_enable2())
			{
				// Right now, this function assumes latch mode; which is always used for
				// 3.5 inch drives.  Eventually we should check to see if latch mode is
				// off
				if (LOG_APPLEFDC)
				{
					if ((m_mode & IWM_MODE_LATCHMODE) == 0)
						logerror("applefdc_write_reg(): latch mode off not implemented\n");
				}

				if (intf->write_data != nullptr)
					intf->write_data(this, data);
			}
			break;
	}
}



//-------------------------------------------------
//  turn_motor_onoff - timer callback for turning
//  motor on or off
//-------------------------------------------------

void applefdc_base_device::turn_motor_onoff(bool status)
{
	const applefdc_interface *intf = get_interface();
	int enable_lines;

	if (status)
	{
		m_lines |= IWM_MOTOR;
		enable_lines = (m_lines & IWM_DRIVE) ? 2 : 1;
	}
	else
	{
		m_lines &= ~IWM_MOTOR;

		if (m_type == APPLEFDC_APPLE2)
			enable_lines = (m_lines & IWM_DRIVE) ? 2 : 1;
		else
			enable_lines = 0;
	}

	/* invoke callback, if present */
	if (intf->set_enable_lines != nullptr)
		intf->set_enable_lines(this, enable_lines);

	if (LOG_APPLEFDC_EXTRA)
		logerror("iwm_turnmotor_onoff(): Turning motor %s\n", status ? "on" : "off");
}



//-------------------------------------------------
//  iwm_access
//-------------------------------------------------

void applefdc_base_device::iwm_access(int offset)
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

	const applefdc_interface *intf = get_interface();

	if (offset & 1)
		m_lines |= (1 << (offset >> 1));
	else
		m_lines &= ~(1 << (offset >> 1));

	if (LOG_APPLEFDC_EXTRA)
	{
		logerror("iwm_access(): %s line %s => %02x\n",
			(offset & 1) ? "setting" : "clearing", lines[offset >> 1], m_lines);
	}

	if ((offset < 0x08) && (intf->set_lines != nullptr))
		intf->set_lines(this, m_lines & 0x0f);

	switch(offset)
	{
		case 0x08:
			/* turn off motor */
			m_motor_timer->adjust(
				(m_mode & IWM_MODE_MOTOROFFDELAY) ? attotime::zero : attotime::from_seconds(1), 0);
			break;

		case 0x09:
			/* turn on motor */
			m_motor_timer->adjust(attotime::zero, 1);
			break;

		case 0x0A:
			/* turn off IWM_DRIVE */
			if ((m_lines & IWM_MOTOR) && (intf->set_enable_lines != nullptr))
				intf->set_enable_lines(this, 1);
			break;

		case 0x0B:
			/* turn on IWM_DRIVE */
			if ((m_lines & IWM_MOTOR) && (intf->set_enable_lines != nullptr))
				intf->set_enable_lines(this, 2);
			break;
	}
}



//-------------------------------------------------
//  read - reads a byte from the FDC
//-------------------------------------------------

UINT8 applefdc_base_device::read(UINT8 offset)
{
	const applefdc_interface *intf = get_interface();
	UINT8 result = 0;

	// normalize offset
	offset &= 0xf;

	if (LOG_APPLEFDC_EXTRA)
		logerror("applefdc_r: offset=%i\n", offset);

	iwm_access(offset);

	switch(m_type)
	{
		case APPLEFDC_APPLE2:
			switch(offset)
			{
				case 0x0C:
					if (m_lines & IWM_Q7)
					{
						if (intf->write_data != nullptr)
							intf->write_data(this, m_write_byte);
						result = 0;
					}
					else
						result = read_reg(0);
					break;

				case 0x0D:
					result = read_reg(IWM_Q6);
					break;

				case 0x0E:
					result = read_reg(IWM_Q7);
					break;

				case 0x0F:
					result = read_reg(IWM_Q7 | IWM_Q6);
					break;
			}
			break;

		case APPLEFDC_IWM:
			if ((offset & 1) == 0)
				result = read_reg(m_lines & (IWM_Q6 | IWM_Q7));
			break;

		case APPLEFDC_SWIM:
			if ((offset & 1) == 0)
				result = read_reg(m_lines & (IWM_Q6 | IWM_Q7));
			break;
	}

	return result;
}



//-------------------------------------------------
//  write - writes a byte to the FDC
//-------------------------------------------------

void applefdc_base_device::write(UINT8 offset, UINT8 data)
{
	const applefdc_interface *intf = get_interface();

	/* normalize offset */
	offset &= 15;

	if (LOG_APPLEFDC_EXTRA)
		logerror("applefdc_w: offset=%i data=0x%02x\n", offset, data);

	iwm_access(offset);

	switch(m_type)
	{
		case APPLEFDC_APPLE2:
			switch(offset)
			{
				case 0x0C:
					if (m_lines & IWM_Q7)
					{
						if (intf->write_data != nullptr)
							intf->write_data(this, m_write_byte);
					}
					break;

				case 0x0D:
					m_write_byte = data;
					break;
			}
			break;

		case APPLEFDC_IWM:
			if (offset & 1)
				write_reg(data);
			break;

		case APPLEFDC_SWIM:
			if (offset & 1)
				write_reg(data);
			break;
	}
}



//-------------------------------------------------
//  get_lines - accessor
//-------------------------------------------------

UINT8 applefdc_base_device::get_lines()
{
	return m_lines & 0x0f;
}



/***************************************************************************
    APPLE FDC - Used on Apple II
***************************************************************************/

const device_type APPLEFDC = &device_creator<applefdc_device>;

applefdc_device::applefdc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: applefdc_base_device(APPLEFDC_APPLE2, mconfig, APPLEFDC, "Apple FDC", tag, owner, clock, "apple_fdc", __FILE__)
{
}



/***************************************************************************
    IWM - Used on early Macs
***************************************************************************/

const device_type IWM = &device_creator<iwm_device>;

iwm_device::iwm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: applefdc_base_device(APPLEFDC_IWM, mconfig, IWM, "Apple IWM (Integrated Woz Machine)", tag, owner, clock, "iwm", __FILE__)
{
}
