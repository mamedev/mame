// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/****************************************************************************

    pce220_ser.c

    Sharp PC-E220/PC-G850V Serial I/O

****************************************************************************/

#include "emu.h"
#include "pce220_ser.h"

// SIO configuration
#define SIO_BAUD_RATE       1200
#define SIO_EOT_BYTE        0x1a
#define SIO_DATA_BIT        8       // number of data bit
#define SIO_PARITY          0       // 0=none 1=odd 2=even
#define SIO_STOP_BIT        1       // number of stop bit

//SIO states
enum
{
	SIO_WAIT = 0,
	SIO_SEND_START,
	SIO_SEND_BIT0,
	SIO_SEND_BIT1,
	SIO_SEND_BIT2,
	SIO_SEND_BIT3,
	SIO_SEND_BIT4,
	SIO_SEND_BIT5,
	SIO_SEND_BIT6,
	SIO_SEND_BIT7,
	SIO_SEND_PARITY,
	SIO_SEND_STOP1,
	SIO_SEND_STOP2
};

// device type definition
DEFINE_DEVICE_TYPE(PCE220SERIAL, pce220_serial_device, "pce220_serial", "Sharp PC-E220 serial")

//-------------------------------------------------
//  pce220_serial_device - constructor
//-------------------------------------------------

pce220_serial_device::pce220_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PCE220SERIAL, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  pce220_serial_device - destructor
//-------------------------------------------------

pce220_serial_device::~pce220_serial_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pce220_serial_device::device_start()
{
	m_send_timer = timer_alloc(FUNC(pce220_serial_device::send_tick), this);
	m_receive_timer = timer_alloc(FUNC(pce220_serial_device::receive_tick), this);
	m_send_timer->reset();
	m_receive_timer->reset();
}


//-------------------------------------------------
//  device_reset - reset up the device
//-------------------------------------------------

void pce220_serial_device::device_reset()
{
	m_state = SIO_WAIT;
	m_bytes_count = 0;
	m_dout = m_busy = m_xout = 0;
	m_din = m_xin = m_ack = 0;
}


//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pce220_serial_device::send_tick)
{
	if (!m_enabled)
	{
		m_din = m_xin = m_ack = 0;
		return;
	}

	//send data
	if (m_bytes_count <= length())
	{
		switch(m_state)
		{
		case SIO_WAIT:
			m_ack = 1;  //data send request
			//waits until is ready to receive
			if(!m_busy)
				return;
			break;
		case SIO_SEND_START:
			//send start bit
			m_xin = 1;
			break;
		case SIO_SEND_BIT0:
		case SIO_SEND_BIT1:
		case SIO_SEND_BIT2:
		case SIO_SEND_BIT3:
		case SIO_SEND_BIT4:
		case SIO_SEND_BIT5:
		case SIO_SEND_BIT6:
		case SIO_SEND_BIT7:
			m_xin = BIT(~m_current_byte, m_state - SIO_SEND_BIT0);
			break;
		case SIO_SEND_PARITY:
			m_xin = calc_parity(m_current_byte);
			break;
		case SIO_SEND_STOP1:
		case SIO_SEND_STOP2:
			//stop bit
			m_xin = m_ack = 0;
			break;
		}

		if (m_state == (SIO_SEND_PARITY + SIO_STOP_BIT))
		{
			//next byte
			m_bytes_count++;
			popmessage("Send %d/%d bytes\n", m_bytes_count , (uint32_t)length());
			m_state = SIO_WAIT;
			if (m_bytes_count < length())
				fread(&m_current_byte, 1);
			else
				m_current_byte = SIO_EOT_BYTE;
		}
		else
		{
			m_state = get_next_state();
		}

	}
	else
	{
		m_din = m_xin = m_ack = 0;
	}
}

TIMER_CALLBACK_MEMBER(pce220_serial_device::receive_tick)
{
	if (!m_enabled)
	{
		m_din = m_xin = m_ack = 0;
		return;
	}

	//receive data
	switch(m_state)
	{
	case SIO_WAIT:
		//wait send request
		if(m_busy)
			return;
		m_ack = 1; //acknowledge
		break;
	case SIO_SEND_START:
		//wait for the start bit
		if (!m_xout)
			return;
		break;
	case SIO_SEND_BIT0:
	case SIO_SEND_BIT1:
	case SIO_SEND_BIT2:
	case SIO_SEND_BIT3:
	case SIO_SEND_BIT4:
	case SIO_SEND_BIT5:
	case SIO_SEND_BIT6:
	case SIO_SEND_BIT7:
		m_current_byte |= ((~m_xout)&1) << (m_state - SIO_SEND_BIT0);
		break;
	case SIO_SEND_PARITY:
		if (m_xout != calc_parity(m_current_byte))
			logerror("SIO %s: byte %d has wrong parity!\n", tag(), m_bytes_count);
		break;
	case SIO_SEND_STOP1:
	case SIO_SEND_STOP2:
		//receive stop bit
		m_ack = 0;
		break;
	}

	if (m_state == (SIO_SEND_PARITY + SIO_STOP_BIT))
	{
		//next byte
		m_bytes_count++;
		popmessage("Received %d bytes\n", m_bytes_count);
		m_state = SIO_WAIT;
		if (m_current_byte != SIO_EOT_BYTE)
			fwrite(&m_current_byte, 1);
		m_current_byte = 0;
	}
	else
	{
		m_state = get_next_state();
	}
}

/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( pce220_serial )
-------------------------------------------------*/

image_init_result pce220_serial_device::call_load()
{
	m_state = SIO_WAIT;
	m_bytes_count = 0;
	m_send_timer->adjust(attotime::from_hz(SIO_BAUD_RATE), 0, attotime::from_hz(SIO_BAUD_RATE));

	//read the first byte
	fread(&m_current_byte, 1);

	return image_init_result::PASS;
}

/*-------------------------------------------------
    DEVICE_IMAGE_CREATE( pce220_serial )
-------------------------------------------------*/

image_init_result pce220_serial_device::call_create(int format_type, util::option_resolution *format_options)
{
	m_state = SIO_WAIT;
	m_bytes_count = 0;
	m_current_byte = 0;
	m_receive_timer->adjust(attotime::from_hz(SIO_BAUD_RATE), 0, attotime::from_hz(SIO_BAUD_RATE));

	return image_init_result::PASS;
}


/*-------------------------------------------------
    DEVICE_IMAGE_UNLOAD( pce220_serial )
-------------------------------------------------*/

void pce220_serial_device::call_unload()
{
	m_send_timer->reset();
	m_receive_timer->reset();
	m_state = SIO_WAIT;
	m_bytes_count = 0;
	m_current_byte = 0;
}


/*-------------------------------------------------
    calculate the byte parity
-------------------------------------------------*/

int pce220_serial_device::calc_parity(uint8_t data)
{
	uint8_t count = 0;

	while(data != 0)
	{
		count++;
		data &= (data-1);
	}

	return (count ^ BIT(SIO_PARITY, 1)) & 1;
}


/*-------------------------------------------------
    returns the next SIO state basing on the
    configuration
-------------------------------------------------*/

int pce220_serial_device::get_next_state()
{
	switch(m_state)
	{
	case SIO_SEND_BIT6:
		if (SIO_DATA_BIT == 8)
			return SIO_SEND_BIT7;
	//fall through
	case SIO_SEND_BIT7:
		return (SIO_PARITY ? SIO_SEND_PARITY : SIO_SEND_STOP1);
	case SIO_SEND_STOP1:
	case SIO_SEND_STOP2:
		return (SIO_STOP_BIT == 2 ? SIO_SEND_STOP2 : SIO_SEND_STOP1);
	default:
		return m_state + 1;
	}
}
