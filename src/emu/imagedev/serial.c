/****************************************************************************

    serial.c

    Code for handling serial port image devices

****************************************************************************/

#include "emu.h"
#include "serial.h"


// device type definition
const device_type SERIAL = &device_creator<serial_image_device>;

//-------------------------------------------------
//  serial_image_device - constructor
//-------------------------------------------------

serial_image_device::serial_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SERIAL, "Serial", tag, owner, clock),
		device_serial_interface(mconfig, *this),
		device_image_interface(mconfig, *this)
{

}

//-------------------------------------------------
//  serial_image_device - destructor
//-------------------------------------------------

serial_image_device::~serial_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void serial_image_device::device_config_complete()
{
	// inherit a copy of the static data
	const serial_image_interface *intf = reinterpret_cast<const serial_image_interface *>(static_config());
	if (intf != NULL)
		*static_cast<serial_image_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_baud_rate = 2400;
		m_data_bits = 8;
		m_stop_bits = 2;
		m_parity = SERIAL_PARITY_NONE;
		m_transmit_on_start = FALSE;
		m_tag_connected = NULL;
	}

	// set brief and instance name
	update_names();
}

static TIMER_CALLBACK(serial_device_baud_rate_callback)
{
	reinterpret_cast<serial_image_device *>(ptr)->timer_callback();
}

/*-------------------------------------------------
    input_callback - called when other side
    has updated state
-------------------------------------------------*/
void serial_image_device::input_callback(UINT8 state)
{
	m_input_state = state;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void serial_image_device::device_start()
{
	set_data_frame(m_data_bits, m_stop_bits,m_parity);

	m_timer = machine().scheduler().timer_alloc(FUNC(serial_device_baud_rate_callback), this);

	/* signal to other end it is clear to send! */
	/* data is initially high state */
	/* set /rts */
	m_connection_state |= SERIAL_STATE_RTS;
	/* signal to other end data is ready to be accepted */
	/* set /dtr */
	m_connection_state |= SERIAL_STATE_DTR;

	set_out_data_bit(1);
	serial_connection_out();
	transmit_register_reset();
	receive_register_reset();

	device_serial_interface *intf = NULL;
	if (m_tag_connected) {
		device_t *dev = machine().device(m_tag_connected);
		if (dev!=NULL && dev->interface(intf)) {
			intf->connect(this);
		}
	}
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/* reset position in stream */
void serial_image_device::data_stream_reset(serial_data_stream *stream)
{
	/* reset byte offset */
	stream->ByteCount= 0;
	/* reset bit count */
	stream->BitCount = 0;
}

/* free stream */
void serial_image_device::data_stream_free(serial_data_stream *stream)
{
	if (stream->pData!=NULL)
	{
		free(stream->pData);
		stream->pData = NULL;
	}
	stream->DataLength = 0;
}

/* initialise stream */
void serial_image_device::data_stream_init(serial_data_stream *stream, unsigned char *pData, unsigned long DataLength)
{
	stream->pData = pData;
	stream->DataLength = DataLength;
	data_stream_reset(stream);
}

/* get a bit from input stream */

int serial_image_device::data_stream_get_data_bit_from_data_byte(serial_data_stream *stream)
{
	int data_bit;
	int data_byte;

	if (stream->ByteCount<stream->DataLength)
	{
		/* get data from buffer */
		data_byte= stream->pData[stream->ByteCount];
	}
	else
	{
		/* over end of buffer, so return 0 */
		data_byte= 0;
	}

	/* get bit from data */
	data_bit = (data_byte>>(7-stream->BitCount)) & 0x01;

	/* update bit count */
	stream->BitCount++;
	/* ripple overflow onto byte count */
	stream->ByteCount+=stream->BitCount>>3;
	/* lock bit count into range */
	stream->BitCount &=0x07;

	/* do not let it read over end of data */
	if (stream->ByteCount>=stream->DataLength)
	{
		stream->ByteCount = stream->DataLength-1;
	}

	return data_bit;
}


void serial_image_device::set_transmit_state(int state)
{
	int previous_state = m_transmit_state;

	m_transmit_state = state;

	if ((state^previous_state)!=0)
	{
		if (state)
		{
			/* start timer */
			m_timer->adjust(attotime::zero, 0, attotime::from_hz(m_baud_rate));
		}
		else
		{
			/* remove timer */
			m_timer->reset();
		}
	}

}

void serial_image_device::sent_char()
{
	int bit;
	UINT8 data_byte;

	/* generate byte to transmit */
	data_byte = 0;
	for (int i=0; i< m_data_bits; i++)
	{
		data_byte = data_byte<<1;
		bit = data_stream_get_data_bit_from_data_byte(&m_transmit);
		data_byte = data_byte|bit;
	}
	/* setup register */
	transmit_register_setup(data_byte);

	logerror("serial device transmitted char: %02x\n",data_byte);
}

/*-------------------------------------------------
    timer_callback
-------------------------------------------------*/
void serial_image_device::timer_callback()
{
	/* receive data into receive register */
	receive_register_update_bit(get_in_data_bit());

	if (is_receive_register_full())
	{
		//logerror("SERIAL DEVICE\n");
		receive_register_extract();

		logerror("serial device receive char: %02x\n",get_received_char());
	}

	/* is transmit empty? */
	if (is_transmit_register_empty())
	{
		/* char has been sent, execute callback */
		sent_char();
	}

	/* other side says it is clear to send? */
	if (m_connection_state & SERIAL_STATE_CTS)
	{
		/* send bit */
		transmit_register_send_bit();
	}
}

int serial_image_device::load_internal(unsigned char **ptr, int *pDataSize)
{
	int datasize;
	unsigned char *data;

	/* get file size */
	datasize = length();

	if (datasize!=0)
	{
		/* malloc memory for this data */
		data = (unsigned char *)malloc(datasize);

		if (data!=NULL)
		{
			/* read whole file */
			fread(data, datasize);

			*ptr = data;
			*pDataSize = datasize;

			logerror("File loaded!\r\n");

			/* ok! */
			return 1;
		}
	}
	return 0;
}

/*-------------------------------------------------
    call_load
-------------------------------------------------*/
bool serial_image_device::call_load()
{
	int data_length;
	unsigned char *data;
	/* load file and setup transmit data */
	if (load_internal(&data, &data_length))
	{
		data_stream_init(&m_transmit, data, data_length);
		set_transmit_state(m_transmit_on_start ? 1 :0);
		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_FAIL;
}


/*-------------------------------------------------
    call_unload
-------------------------------------------------*/
void serial_image_device::call_unload()
{
	/* stop transmit */
	set_transmit_state(0);

	/* free streams */
	data_stream_free(&m_transmit);
	data_stream_free(&m_receive);
}
