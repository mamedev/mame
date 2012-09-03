/******************************************************************************
    uPD7002 Analogue to Digital Converter

    MESS Driver By:

    Gordon Jefferyes
    mess_bbc@gjeffery.dircon.co.uk

******************************************************************************/

#include "emu.h"
#include "upd7002.h"


typedef struct _uPD7002_t uPD7002_t;
struct _uPD7002_t
{
	/* Pointer to our interface */
	const uPD7002_interface *intf;

	/* Status Register
        D0 and D1 define the currently selected input channel
        D2 flag output
        D3 0 = 8 bit mode   1 = 12 bit mode
        D4 2nd MSB of conversion
        D5     MSB of conversion
        D6 0 = busy, 1 = not busy    (~busy)
        D7 0 = conversion completed, 1 = conversion not completed  (~EOC)
    */
	int status;

	/* High data byte
        This byte contains the 8 most significant bits of the analogue to digital conversion. */
	int data1;

	/* Low data byte
        In 12 bit mode: Bits 7 to 4 define the four low order bits of the conversion.
        In  8 bit mode. All bits 7 to 4 are inaccurate.
        Bits 3 to 0 are always set to low. */
	int data0;


	/* temporary store of the next A to D conversion */
	int digitalvalue;

	/* this counter is used to check a full end of conversion has been reached
    if the uPD7002 is half way through one conversion and a new conversion is requested
    the counter at the end of the first conversion will not match and not be processed
    only then at the end of the second conversion will the conversion complete function run */
	int conversion_counter;
};


/*****************************************************************************
 Implementation
*****************************************************************************/

INLINE uPD7002_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == UPD7002);

	return (uPD7002_t *)downcast<uPD7002_device *>(device)->token();
}

READ8_DEVICE_HANDLER ( uPD7002_EOC_r )
{
	uPD7002_t *uPD7002 = get_safe_token(device);
	return (uPD7002->status>>7)&0x01;
}


static TIMER_CALLBACK(uPD7002_conversioncomplete)
{
	device_t *device = (device_t *)ptr;
	uPD7002_t *uPD7002 = get_safe_token(device);

	int counter_value = param;
	if (counter_value==uPD7002->conversion_counter)
	{
		// this really always does a 12 bit conversion
		uPD7002->data1 = uPD7002->digitalvalue>>8;
		uPD7002->data0 = uPD7002->digitalvalue&0xf0;

		// set the status register with top 2 MSB, not busy and conversion complete
		uPD7002->status = (uPD7002->status & 0x0f)|((uPD7002->data1 & 0xc0)>>2)|0x40;

		// call the EOC function with EOC from status
		// uPD7002_EOC_r(0) this has just been set to 0
		if (uPD7002->intf->EOC_func) (uPD7002->intf->EOC_func)(device,0);
		uPD7002->conversion_counter=0;
	}
}


READ8_DEVICE_HANDLER ( uPD7002_r )
{
	uPD7002_t *uPD7002 = get_safe_token(device);

	switch(offset&0x03)
	{
		case 0:
			return uPD7002->status;

		case 1:
			return uPD7002->data1;

		case 2: case 3:
			return uPD7002->data0;
	}
	return 0;
}



WRITE8_DEVICE_HANDLER ( uPD7002_w )
{
	uPD7002_t *uPD7002 = get_safe_token(device);
	/* logerror("write to uPD7002 $%02X = $%02X\n",offset,data); */

	switch(offset&0x03)
	{
		case 0:
		/*
        Data Latch/AD start
            D0 and D1 together define which one of the four input channels is selected
            D2 flag input, normally set to 0????
            D3 defines whether an 8 (0) or 12 (1) bit resolution conversion should occur
            D4 to D7 not used.

            an 8  bit conversion typically takes 4ms
            an 12 bit conversion typically takes 10ms

            writing to this register will initiate a conversion.
        */

		/* set D6=0 busy ,D7=1 conversion not complete */
		uPD7002->status=(data & 0x0f) | 0x80;

		// call the EOC function with EOC from status
		// uPD7002_EOC_r(0) this has just been set to 1
		if (uPD7002->intf->EOC_func) uPD7002->intf->EOC_func(device, 1);

		/* the uPD7002 works by sampling the analogue value at the start of the conversion
           so it is read hear and stored until the end of the A to D conversion */

		// this function should return a 16 bit value.
		uPD7002->digitalvalue = uPD7002->intf->get_analogue_func(device, uPD7002->status & 0x03);

		uPD7002->conversion_counter++;

		// call a timer to start the conversion
		if (uPD7002->status & 0x08)
		{
			// 12 bit conversion takes 10ms
			device->machine().scheduler().timer_set(attotime::from_msec(10), FUNC(uPD7002_conversioncomplete), uPD7002->conversion_counter, (void *)device);
		} else {
			// 8 bit conversion takes 4ms
			device->machine().scheduler().timer_set(attotime::from_msec(4), FUNC(uPD7002_conversioncomplete), uPD7002->conversion_counter, (void *)device);
		}
		break;

		case 1: case 2:
		/* Nothing */
		break;

		case 3:
		/* Test Mode: Used for inspecting the device, The data input-output terminals assume an input
              state and are connected to the A/D counter. Therefore, the A/D conversion data
              read out after this is meaningless.
        */
		break;
	}
}

/* Device Interface */

static DEVICE_START( uPD7002 )
{
	uPD7002_t *uPD7002 = get_safe_token(device);
	// validate arguments

	assert(device != NULL);
	assert(device->tag() != NULL);
	assert(device->static_config() != NULL);

	uPD7002->intf = (const uPD7002_interface*)device->static_config();
	uPD7002->status = 0;
	uPD7002->data1 = 0;
	uPD7002->data0 = 0;
	uPD7002->digitalvalue = 0;
	uPD7002->conversion_counter = 0;

	// register for state saving
	state_save_register_item(device->machine(), "uPD7002", device->tag(), 0, uPD7002->status);
	state_save_register_item(device->machine(), "uPD7002", device->tag(), 0, uPD7002->data1);
	state_save_register_item(device->machine(), "uPD7002", device->tag(), 0, uPD7002->data0);
	state_save_register_item(device->machine(), "uPD7002", device->tag(), 0, uPD7002->digitalvalue);
	state_save_register_item(device->machine(), "uPD7002", device->tag(), 0, uPD7002->conversion_counter);
}

static DEVICE_RESET( uPD7002 )
{
	uPD7002_t *uPD7002 = get_safe_token(device);
	uPD7002->status = 0;
	uPD7002->data1 = 0;
	uPD7002->data0 = 0;
	uPD7002->digitalvalue = 0;
	uPD7002->conversion_counter = 0;
}

const device_type UPD7002 = &device_creator<uPD7002_device>;

uPD7002_device::uPD7002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD7002, "uPD7002", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(uPD7002_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void uPD7002_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void uPD7002_device::device_start()
{
	DEVICE_START_NAME( uPD7002 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void uPD7002_device::device_reset()
{
	DEVICE_RESET_NAME( uPD7002 )(this);
}


