/* PAD emulation */

#include "psxcport.h"
#include "machine/psxcard.h"

#define PAD_STATE_IDLE ( 0 )
#define PAD_STATE_LISTEN ( 1 )
#define PAD_STATE_ACTIVE ( 2 )
#define PAD_STATE_READ ( 3 )
#define PAD_STATE_UNLISTEN ( 4 )
#define PAD_STATE_MEMCARD ( 5 )

#define PAD_TYPE_STANDARD ( 4 )
#define PAD_BYTES_STANDARD ( 2 )

#define PAD_CMD_START ( 0x01 )
#define PAD_CMD_READ  ( 0x42 ) /* B */

#define PAD_DATA_OK   ( 0x5a ) /* Z */
#define PAD_DATA_IDLE ( 0xff )

const device_type PSXCONTROLLERPORTS = &device_creator<psxcontrollerports_device>;

psxcontrollerports_device::psxcontrollerports_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	psxsiodev_device(mconfig, PSXCONTROLLERPORTS, "PSXCONTROLLERPORTS", tag, owner, clock)
{
}

void psxcontrollerports_device::device_start()
{
	psxsiodev_device::device_start();

	m_ack_timer = timer_alloc( 0 );
}

void psxcontrollerports_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	int n_port = param;
	pad_t *pad = &m_pad[ n_port ];

	if( pad->n_state != PAD_STATE_IDLE )
	{
		data_out( pad->b_ack * PSX_SIO_IN_DSR, PSX_SIO_IN_DSR );

		if( !pad->b_ack )
		{
			pad->b_ack = 1;
			m_ack_timer->adjust( attotime::from_usec( 2 ), n_port );
		}
	}
}

void psxcontrollerports_device::psx_pad( int n_port, int n_data )
{
	pad_t *pad = &m_pad[ n_port ];
	int b_sel;
	int b_clock;
	int b_data;
	int b_ack;
	int b_ready;
	static const char *const portnames[] = { ":IN0", ":IN1", ":IN2", ":IN3" };
	psxcard_device *psxcard = NULL;

	if (n_port == 0)
	{
		psxcard = machine().device<psxcard_device>(":card1");
	}
	else
	{
		psxcard = machine().device<psxcard_device>(":card2");
	}

	b_sel = ( n_data & PSX_SIO_OUT_DTR ) / PSX_SIO_OUT_DTR;
	b_clock = ( n_data & PSX_SIO_OUT_CLOCK ) / PSX_SIO_OUT_CLOCK;
	b_data = ( n_data & PSX_SIO_OUT_DATA ) / PSX_SIO_OUT_DATA;
	b_ready = 0;
	b_ack = 0;

	if( b_sel )
	{
		pad->n_state = PAD_STATE_IDLE;
	}

	switch( pad->n_state )
	{
	case PAD_STATE_LISTEN:
	case PAD_STATE_ACTIVE:
	case PAD_STATE_READ:
	case PAD_STATE_MEMCARD:
		if( pad->b_lastclock && !b_clock )
		{
			data_out( ( pad->n_shiftout & 1 ) * PSX_SIO_IN_DATA, PSX_SIO_IN_DATA );
			pad->n_shiftout >>= 1;
		}
		if( !pad->b_lastclock && b_clock )
		{
			pad->n_shiftin >>= 1;
			pad->n_shiftin |= b_data << 7;
			pad->n_bits++;

			if( pad->n_bits == 8 )
			{
				pad->n_bits = 0;
				b_ready = 1;
			}
		}
		break;
	}

	pad->b_lastclock = b_clock;

	switch( pad->n_state )
	{
	case PAD_STATE_IDLE:
		if( !b_sel )
		{
			pad->n_state = PAD_STATE_LISTEN;
			pad->n_shiftout = PAD_DATA_IDLE;
			pad->n_bits = 0;
		}
		break;
	case PAD_STATE_LISTEN:
		if( b_ready )
		{
			if( pad->n_shiftin == PAD_CMD_START )
			{
				pad->n_state = PAD_STATE_ACTIVE;
				pad->n_shiftout = ( PAD_TYPE_STANDARD << 4 ) | ( PAD_BYTES_STANDARD >> 1 );
				b_ack = 1;
			}
			else if( psxcard->transfer(pad->n_shiftin, &pad->n_shiftout) )
			{
				pad->n_state = PAD_STATE_MEMCARD;
				b_ack = 1;
			}
			else
			{
				pad->n_state = PAD_STATE_UNLISTEN;
			}
		}
		break;
	case PAD_STATE_MEMCARD:
		if( b_ready )
		{
			if( psxcard->transfer(pad->n_shiftin, &pad->n_shiftout) )
			{
				b_ack = 1;
			}
			else
			{
				b_ack = 0;
				pad->n_state = PAD_STATE_IDLE;
			}
		}
		break;
	case PAD_STATE_ACTIVE:
		if( b_ready )
		{
			if( pad->n_shiftin == PAD_CMD_READ )
			{
				pad->n_state = PAD_STATE_READ;
				pad->n_shiftout = PAD_DATA_OK;
				pad->n_byte = 0;
				b_ack = 1;
			}
			else
			{
				pad->n_state = PAD_STATE_UNLISTEN;
			}
		}
		break;
	case PAD_STATE_READ:
		if( b_ready )
		{
			if( pad->n_byte < PAD_BYTES_STANDARD )
			{
				pad->n_shiftout = ioport(portnames[pad->n_byte + ( n_port * PAD_BYTES_STANDARD )])->read();
				pad->n_byte++;
				b_ack = 1;
			}
			else
			{
				pad->n_state = PAD_STATE_LISTEN;
			}
		}
		break;
	}

	if( b_ack )
	{
		pad->b_ack = 0;
		m_ack_timer->adjust( attotime::from_usec( 10 ), n_port );
	}
}

void psxcontrollerports_device::data_in( int data, int mask )
{
	/* todo: raise data & ack when nothing is driving it low */
	psx_pad( 0, data );
	psx_pad( 1, data ^ PSX_SIO_OUT_DTR );
}
