/**********************************************************************

    8237 DMA interface and emulation

    The DMA works like this:
    (summarized from http://www.infran.ru/TechInfo/BSD/handbook258.html#410)

    1.  The device asserts the DRQn line
    2.  The DMA clears the TC (terminal count) line
    3.  The DMA asserts the CPU's HRQ (halt request) line
    4.  Upon acknowledgement of the halt, the DMA will let the device
        know that it needs to send information by asserting the DACKn
        line
    5.  The DMA will read the byte from the device
    6.  The device clears the DRQn line
    7.  The DMA clears the CPU's HRQ line
    8.  (steps 3-7 are repeated for every byte in the chain)

**********************************************************************/

#include "driver.h"
#include "memconv.h"
#include "8237dma.h"


/* States that the i8237 device can be in */
typedef enum {
	DMA8237_SI,			/* Idle state */
	DMA8237_S0,			/* HRQ has been triggered, waiting to receive HLDA */
//  DMA8237_SW,         /* Wait state */

	/* Normal transfer states */
	DMA8237_S1,			/* Output A8-A15; only used when A8-A15 really needs to be output */
	DMA8237_S2,			/* Output A0-A7 */
	DMA8237_S3,			/* Initiate read; skipped in compressed timing. On the S2->S3 transition DACK is set. */
	DMA8237_S4,			/* Perform read/write */

	/* Memory to memory transfer states */
	DMA8237_S11,		/* Output A8-A15 */
//  DMA8237_S12,        /* Output A0-A7 */
//  DMA8237_S13,        /* Initiate read */
//  DMA8237_S14,        /* Perform read/write */
//  DMA8237_S21,        /* Output A8-A15 */
//  DMA8237_S22,        /* Output A0-A7 */
//  DMA8237_S23,        /* Initiate read */
//  DMA8237_S24,        /* Perform read/write */
} dma8237_state;


typedef struct _i8237_t i8237_t;
struct _i8237_t
{
	devcb_resolved_write_line	out_hrq_func;
	devcb_resolved_write_line	out_eop_func;
	devcb_resolved_read8		in_memr_func;
	devcb_resolved_write8		out_memw_func;

	emu_timer *timer;

	struct
	{
		devcb_resolved_read8		in_ior_func;
		devcb_resolved_write8		out_iow_func;
		devcb_resolved_write_line	out_dack_func;
		UINT16 base_address;
		UINT16 base_count;
		UINT16 address;
		UINT16 count;
		UINT8 mode;
		int high_address_changed;
	} chan[4];

	UINT32 msb : 1;
	UINT32 eop : 1;
	UINT8 temp;
	UINT8 temporary_data;
	UINT8 command;
	UINT8 drq;
	UINT8 mask;
	UINT8 hrq;
	UINT8 hlda;

	/* bits  0- 3 :  Terminal count for channels 0-3
     * bits  4- 7 :  Transfer in progress for channels 0-3 */
	UINT8 status;

	dma8237_state state;		/* State the device is currently in */
	int service_channel;		/* Channel we will be servicing */
	int last_service_channel;	/* Previous channel we serviced; used to determine channel priority. */
};


#define DMA_MODE_CHANNEL(mode)		((mode) & 0x03)
#define DMA_MODE_OPERATION(mode)	((mode) & 0x0c)
#define DMA_MODE_AUTO_INIT(mode)	((mode) & 0x10)
#define DMA_MODE_DIRECTION(mode)	((mode) & 0x20)
#define DMA_MODE_TRANSFERMODE(mode)	((mode) & 0xc0)

#define DMA8237_VERIFY_TRANSFER		0x00
#define DMA8237_WRITE_TRANSFER		0x04
#define DMA8237_READ_TRANSFER		0x08
#define DMA8237_ILLEGAL_TRANSFER	0x0c

#define DMA8237_DEMAND_MODE		0x00
#define DMA8237_SINGLE_MODE		0x40
#define DMA8237_BLOCK_MODE		0x80
#define DMA8237_CASCADE_MODE	0xc0


/* ----------------------------------------------------------------------- */

INLINE i8237_t *get_safe_token(const device_config *device) {
	assert( device != NULL );
	assert( device->token != NULL );
	assert( device->type == I8237 );
	return ( i8237_t *) device->token;
}


/* ----------------------------------------------------------------------- */


INLINE void dma8237_do_read( const device_config *device )
{
	i8237_t	*i8237 = get_safe_token( device );
	int			channel = i8237->service_channel;

	switch( DMA_MODE_OPERATION( i8237->chan[ channel ].mode ) )
	{
	case DMA8237_WRITE_TRANSFER:
		i8237->temporary_data = devcb_call_read8(&i8237->chan[channel].in_ior_func, 0);
		break;
	case DMA8237_READ_TRANSFER:
		i8237->temporary_data = devcb_call_read8(&i8237->in_memr_func, i8237->chan[ channel ].address);
		break;
	case DMA8237_VERIFY_TRANSFER:
	case DMA8237_ILLEGAL_TRANSFER:
		break;
	}
}


INLINE void dma8237_do_write( const device_config *device )
{
	i8237_t	*i8237 = get_safe_token( device );
	int			channel = i8237->service_channel;

	switch( DMA_MODE_OPERATION( i8237->chan[ channel ].mode ) )
	{
	case DMA8237_WRITE_TRANSFER:
		devcb_call_write8(&i8237->out_memw_func, i8237->chan[ channel ].address, i8237->temporary_data);
		break;
	case DMA8237_READ_TRANSFER:
		devcb_call_write8(&i8237->chan[channel].out_iow_func, 0, i8237->temporary_data);
		break;
	case DMA8237_VERIFY_TRANSFER:
	case DMA8237_ILLEGAL_TRANSFER:
		break;
	}
}


INLINE void dma8237_advance( const device_config *device )
{
	i8237_t	*i8237 = get_safe_token( device );
	int			channel = i8237->service_channel;
	int			mode = i8237->chan[channel].mode;

	switch ( DMA_MODE_OPERATION( mode ) )
	{
	case DMA8237_VERIFY_TRANSFER:
	case DMA8237_WRITE_TRANSFER:
	case DMA8237_READ_TRANSFER:
		i8237->chan[channel].high_address_changed = 0;

		if ( DMA_MODE_DIRECTION( mode ) )
		{
			i8237->chan[channel].address -= 1;
			if ( ( i8237->chan[channel].address & 0xFF ) == 0xFF )
				i8237->chan[channel].high_address_changed  = 1;
		}
		else
		{
			i8237->chan[channel].address += 1;
			if ( ( i8237->chan[channel].address & 0xFF ) == 0x00 )
				i8237->chan[channel].high_address_changed = 1;
		}

		i8237->chan[channel].count--;

		if ( i8237->chan[channel].count == 0xFFFF )
		{
			/* Set TC bit for this channel */
			i8237->status |= ( 0x01 << channel );

			if ( DMA_MODE_AUTO_INIT( mode ) )
			{
				i8237->chan[channel].address = i8237->chan[channel].base_address;
				i8237->chan[channel].count = i8237->chan[channel].base_count;
				i8237->chan[channel].high_address_changed = 1;
			}
			else
			{
				i8237->mask |= ( 0x01 << channel );
			}
		}
		break;
	case DMA8237_ILLEGAL_TRANSFER:
		break;
	}

}

static void set_dack(i8237_t *i8237, int channel)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		int state = (i == channel) ^ !BIT(i8237->command, 7);

		devcb_call_write_line(&i8237->chan[i].out_dack_func, state);
	}
}

static TIMER_CALLBACK( dma8237_timerproc )
{
	const device_config *device = (const device_config *)ptr;
	i8237_t	*i8237 = get_safe_token(device);

	/* Check if operation is disabled */
	if ( i8237->command & 0x04 )
		return;

	switch ( i8237->state ) {

	case DMA8237_SI:
		/* Make sure EOP is high */
		if ( ! i8237->eop )
		{
			i8237->eop = 1;
			devcb_call_write_line(&i8237->out_eop_func, i8237->eop ? ASSERT_LINE : CLEAR_LINE);
		}

		/* Check if a new DMA request has been received. */
		{
			/* Bit 6 of the command register determines whether the DREQ signals are active
              high or active low. */
			UINT16 pending_request = ( ( i8237->command & 0x40 ) ? ~i8237->drq : i8237->drq ) & ~i8237->mask;

			if ( pending_request & 0x0f )
			{
				int i, channel, prio_channel = 0;

				/* Determine the channel that should be serviced */
				channel = ( i8237->command & 0x10 ) ? i8237->last_service_channel : 3;
				for ( i = 0; i < 4; i++ )
				{
					if ( pending_request & ( 1 << channel ) )
						prio_channel = channel;
					channel = ( channel - 1 ) & 0x03;
				}

				/* Store the channel we will be servicing and go to the next state */
				i8237->service_channel = prio_channel;
				i8237->last_service_channel = prio_channel;
				i8237->hrq = 1;
				devcb_call_write_line(&i8237->out_hrq_func, i8237->hrq);
				i8237->state = DMA8237_S0;

				timer_enable( i8237->timer, 1 );
			}
			else
			{
				timer_enable( i8237->timer, 0 );
			}
		}
		break;

	case DMA8237_S0:
		/* S0 is the first of the DMA service. We have requested a hold but are waiting
          for confirmation. */
		if ( i8237->hlda )
		{
			if ( i8237->command & 0x01 )
			{
				/* Memory-to-memory transfers */
				i8237->state = DMA8237_S11;
			}
			else
			{
				/* Regular transfers */
				i8237->state = DMA8237_S1;
			}
		}
		break;

	case DMA8237_S1:	/* Output A8-A15 */
		i8237->state = DMA8237_S2;
		break;

	case DMA8237_S2:	/* Output A7-A0 */
		/* set DACK */
		set_dack(i8237, i8237->service_channel);

		/* Check for compressed timing */
		if ( i8237->command & 0x08 )
			i8237->state = DMA8237_S4;
		else
			i8237->state = DMA8237_S3;
		break;

	case DMA8237_S3:	/* Initiate read */
		dma8237_do_read( device );
		i8237->state = DMA8237_S4;
		break;

	case DMA8237_S4:	/* Perform read/write */
		/* Perform read when in compressed timing mode */
		if ( i8237->command & 0x08 )
			dma8237_do_read( device );

		/* Perform write */
		dma8237_do_write( device );

		/* Advance */
		dma8237_advance( device );

		{
			int channel = i8237->service_channel;

			switch( DMA_MODE_TRANSFERMODE( i8237->chan[channel].mode ) )
			{
			case DMA8237_DEMAND_MODE:
				/* Check for terminal count or EOP signal or DREQ begin de-asserted */
				if ( ( i8237->status & ( 0x01 << channel ) ) || ! i8237->eop || ! ( i8237->drq & ( 0x01 << channel ) ) )
				{
					i8237->hrq = 0;
					i8237->hlda = 0;
					devcb_call_write_line(&i8237->out_hrq_func, i8237->hrq);
					i8237->state = DMA8237_SI;
				}
				else
				{
					i8237->state = i8237->chan[channel].high_address_changed ? DMA8237_S1 : DMA8237_S2;
				}
				break;

			case DMA8237_SINGLE_MODE:
				i8237->hrq = 0;
				i8237->hlda = 0;
				devcb_call_write_line(&i8237->out_hrq_func, i8237->hrq);
				i8237->state = DMA8237_SI;
				break;

			case DMA8237_BLOCK_MODE:
				/* Check for terminal count or EOP signal */
				if ( ( i8237->status & ( 0x01 << channel ) ) || ! i8237->eop )
				{
					i8237->hrq = 0;
					i8237->hlda = 0;
					devcb_call_write_line(&i8237->out_hrq_func, i8237->hrq);
					i8237->state = DMA8237_SI;
				}
				else
				{
					i8237->state = i8237->chan[channel].high_address_changed ? DMA8237_S1 : DMA8237_S2;
				}
				break;

			case DMA8237_CASCADE_MODE:
				if ( ! ( i8237->drq & ( 0x01 << channel ) ) )
				{
					i8237->hrq = 0;
					i8237->hlda = 0;
					devcb_call_write_line(&i8237->out_hrq_func, i8237->hrq);
					i8237->state = DMA8237_SI;
				}
				break;
			}

			/* Check if EOP output needs to be asserted */
			if ( i8237->status & ( 0x01 << channel ) )
			{
				i8237->eop = 0;
				devcb_call_write_line(&i8237->out_eop_func, i8237->eop ? ASSERT_LINE : CLEAR_LINE);
			}
		}

		/* clear DACK */
		set_dack(i8237, -1);
		break;

	case DMA8237_S11:	/* Output A8-A15 */
		break;
	}
}




/* ----------------------------------------------------------------------- */


READ8_DEVICE_HANDLER( i8237_r )
{
	i8237_t	*i8237 = get_safe_token(device);
	UINT8 data = 0xFF;

	offset &= 0x0F;

	switch(offset) {
	case 0:
	case 2:
	case 4:
	case 6:
		/* DMA address register */
		data = i8237->chan[offset / 2].address >> (i8237->msb ? 8 : 0);
		i8237->msb ^= 1;
		break;

	case 1:
	case 3:
	case 5:
	case 7:
		/* DMA count register */
		data = i8237->chan[offset / 2].count >> (i8237->msb ? 8 : 0);
		i8237->msb ^= 1;
		break;

	case 8:
		/* DMA status register */
		data = (UINT8) i8237->status;

		/* TC bits are cleared on a status read */
		i8237->status &= 0xF0;
		break;

	case 10:
		/* DMA mask register */
		data = i8237->mask;
		break;

	case 13:
		/* DMA master clear */
		data = i8237->temp;
		break;

	case 9:		/* DMA write request register */
	case 11:	/* DMA mode register */
	case 12:	/* DMA clear byte pointer flip-flop */
	case 14:	/* DMA clear mask register */
	case 15:	/* DMA write mask register */
		data = 0xFF;
		break;
	}

	return data;
}



WRITE8_DEVICE_HANDLER( i8237_w )
{
	i8237_t	*i8237 = get_safe_token(device);
	int channel;

	offset &= 0x0F;

	logerror("i8237_w: offset = %02x, data = %02x\n", offset, data );

	switch(offset) {
	case 0:
	case 2:
	case 4:
	case 6:
		/* DMA address register */
		channel = offset / 2;
		if (i8237->msb)
		{
			i8237->chan[channel].base_address = ( i8237->chan[channel].base_address & 0x00FF ) | ( data << 8 );
			i8237->chan[channel].address = ( i8237->chan[channel].address & 0x00FF ) | ( data << 8 );
		}
		else
		{
			i8237->chan[channel].base_address = ( i8237->chan[channel].base_address & 0xFF00 ) | data;
			i8237->chan[channel].address = ( i8237->chan[channel].address & 0xFF00 ) | data;
		}
		i8237->msb ^= 1;
		break;

	case 1:
	case 3:
	case 5:
	case 7:
		/* DMA count register */
		channel = offset / 2;
		if (i8237->msb)
		{
			i8237->chan[channel].base_count = ( i8237->chan[channel].base_count & 0x00FF ) | ( data << 8 );
			i8237->chan[channel].count = ( i8237->chan[channel].count & 0x00FF ) | ( data << 8 );
		}
		else
		{
			i8237->chan[channel].base_count = ( i8237->chan[channel].base_count & 0xFF00 ) | data;
			i8237->chan[channel].count = ( i8237->chan[channel].count & 0xFF00 ) | data;
		}
		i8237->msb ^= 1;
		break;

	case 8:
		/* DMA command register */
		i8237->command = data;
		timer_enable( i8237->timer, ( i8237->command & 0x04 ) ? 0 : 1 );
		break;

	case 9:
		/* DMA request register */
		channel = DMA_MODE_CHANNEL(data);
		if ( data & 0x04 )
		{
			i8237->drq |= 0x01 << channel;
			timer_enable( i8237->timer, ( i8237->command & 0x04 ) ? 0 : 1 );
		}
		else
		{
			i8237->status &= ~ ( 0x10 << channel );
			i8237->drq &= ~ ( 0x01 << channel );
		}
		break;

	case 10:
		/* DMA mask register */
		channel = DMA_MODE_CHANNEL(data);
		if (data & 0x04)
			i8237->mask |= 0x11 << channel;
		else
			i8237->mask &= ~(0x11 << channel);
		break;

	case 11:
		/* DMA mode register */
		channel = DMA_MODE_CHANNEL(data);
		i8237->chan[channel].mode = data;
		/* Apparently mode writes also clear the TC bit(?) */
		i8237->status &= ~ ( 1 << channel );
		break;

	case 12:
		/* DMA clear byte pointer flip-flop */
		i8237->temp = data;
		i8237->msb = 0;
		break;

	case 13:
		/* DMA master clear */
		i8237->msb = 0;
		i8237->mask = 0x0f;
		i8237->state = DMA8237_SI;
		i8237->status &= 0xF0;
		break;

	case 14:
		/* DMA clear mask register */
		i8237->mask &= ~data;
		i8237->mask = 0;
		break;

	case 15:
		/* DMA write mask register */
		i8237->mask = data & 0x0f;
		break;
	}
}



static void dma8237_drq_write(const device_config *device, int channel, int state)
{
	i8237_t	*i8237 = get_safe_token( device );

	if (state)
		i8237->drq |= ( 0x01 << channel );
	else
		i8237->drq &= ~( 0x01 << channel );

	timer_enable( i8237->timer, ( i8237->command & 0x04 ) ? 0 : 1 );
}

WRITE_LINE_DEVICE_HANDLER( i8237_hlda_w )
{
	i8237_t	*i8237 = get_safe_token( device );

	i8237->hlda = state;
}

WRITE_LINE_DEVICE_HANDLER( i8237_ready_w )
{
}

WRITE_LINE_DEVICE_HANDLER( i8237_dreq0_w ) { dma8237_drq_write(device, 0, state); }
WRITE_LINE_DEVICE_HANDLER( i8237_dreq1_w ) { dma8237_drq_write(device, 1, state); }
WRITE_LINE_DEVICE_HANDLER( i8237_dreq2_w ) { dma8237_drq_write(device, 2, state); }
WRITE_LINE_DEVICE_HANDLER( i8237_dreq3_w ) { dma8237_drq_write(device, 3, state); }

WRITE_LINE_DEVICE_HANDLER( i8237_eop_w )
{
}


static DEVICE_START( i8237 ) {
	i8237_t	*i8237 = get_safe_token(device);
	i8237_interface *intf = (i8237_interface *)device->static_config;
	int i;

	/* resolve callbacks */
	devcb_resolve_write_line(&i8237->out_hrq_func, &intf->out_hrq_func, device);
	devcb_resolve_write_line(&i8237->out_eop_func, &intf->out_eop_func, device);
	devcb_resolve_read8(&i8237->in_memr_func, &intf->in_memr_func, device);
	devcb_resolve_write8(&i8237->out_memw_func, &intf->out_memw_func, device);

	for (i = 0; i < 4; i++)
	{
		devcb_resolve_read8(&i8237->chan[i].in_ior_func, &intf->in_ior_func[i], device);
		devcb_resolve_write8(&i8237->chan[i].out_iow_func, &intf->out_iow_func[i], device);
		devcb_resolve_write_line(&i8237->chan[i].out_dack_func, &intf->out_dack_func[i], device);
	}

	i8237->timer = timer_alloc(device->machine, dma8237_timerproc, (void *)device);
}


static DEVICE_RESET( i8237 ) {
	i8237_t	*i8237 = get_safe_token(device);

	i8237->status = 0x0F;
	i8237->eop = 1;
	i8237->state = DMA8237_SI;
	i8237->last_service_channel = 3;

	i8237->mask = 0x00;
	i8237->status = 0x0F;
	i8237->hrq = 0;
	i8237->hlda = 0;
	i8237->chan[0].mode = 0;
	i8237->chan[1].mode = 0;
	i8237->chan[2].mode = 0;
	i8237->chan[3].mode = 0;

	timer_adjust_periodic(i8237->timer,
		ATTOTIME_IN_HZ(device->clock),
		0,
		ATTOTIME_IN_HZ(device->clock));
}


DEVICE_GET_INFO( i8237 ) {
	switch ( state ) {
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = sizeof(i8237_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:		info->i = 0;								break;
		case DEVINFO_INT_CLASS:						info->i = DEVICE_CLASS_PERIPHERAL;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(i8237);	break;
		case DEVINFO_FCT_STOP:						/* nothing */								break;
		case DEVINFO_FCT_RESET:						info->reset = DEVICE_RESET_NAME(i8237);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "Intel 8237");			break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Intel 8080");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.01");					break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright the MAME and MESS Teams");	break;
	}
}

