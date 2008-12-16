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


typedef struct dma8237	dma8237_t;

struct dma8237
{
	const struct dma8237_interface *intf;
	emu_timer *timer;
	emu_timer *msbflip_timer;

	struct
	{
		UINT16 address;
		UINT16 count;
		UINT8 mode;
	} chan[4];

	UINT32 msb : 1;
	UINT32 eop : 1;
	UINT8 temp;
	UINT8 command;
	UINT8 drq;
	UINT8 mask;

	/* bits  0- 3 :  Terminal count for channels 0-3
     * bits  4- 7 :  Transfer in progress for channels 0-3 */
	UINT8 status;
};


#define DMA_MODE_CHANNEL(mode)		((mode) & 0x03)
#define DMA_MODE_OPERATION(mode)	(((mode) >> 2) & 0x03)
#define DMA_MODE_DIRECTION(mode)	(((mode) & 0x20) ? -1 : +1)
#define DMA_MODE_TRANSFERMODE(mode)	(((mode) >> 6) & 0x03)



static TIMER_CALLBACK( dma8237_timerproc );
static TIMER_CALLBACK( dma8237_msbflip_timerproc );
static void dma8237_update_status(const device_config *device);

/* ----------------------------------------------------------------------- */

INLINE dma8237_t *get_safe_token(const device_config *device) {
	assert( device != NULL );
	assert( device->token != NULL );
	assert( device->type == DEVICE_GET_INFO_NAME(dma8237) );
	return ( dma8237_t *) device->token;
}


/* ----------------------------------------------------------------------- */


static int dma8237_do_operation(const device_config *device, int channel)
{
	dma8237_t	*dma8237 = get_safe_token(device);
	int done;
	UINT8 data;
	UINT8 mode;

	mode = dma8237->chan[channel].mode;

	switch(DMA_MODE_OPERATION(mode)) {
	case 1:
		data = dma8237->intf->channel_read_func[channel](device);
		dma8237->intf->memory_write_func(device, channel, dma8237->chan[channel].address, data);

		dma8237->chan[channel].address += DMA_MODE_DIRECTION(mode);
		dma8237->chan[channel].count--;
		done = (dma8237->chan[channel].count == 0xFFFF);
		break;

	case 2:
		data = dma8237->intf->memory_read_func(device, channel, dma8237->chan[channel].address);
		dma8237->intf->channel_write_func[channel](device, data);

		dma8237->chan[channel].address += DMA_MODE_DIRECTION(mode);
		dma8237->chan[channel].count--;
		done = (dma8237->chan[channel].count == 0xFFFF);
		break;

	default:
		done = TRUE;
		break;
	}
	return done;
}



static TIMER_CALLBACK( dma8237_timerproc )
{
	const device_config *device = ptr;
	dma8237_t	*dma8237 = get_safe_token(device);
	int channel = param % 4;
	int done;

	done = dma8237_do_operation(device, channel);

	if (done)
	{
		dma8237->status &= ~(0x10 << channel);
		dma8237->status |=  (0x01 << channel);
		dma8237->drq    &= ~(0x01 << channel);
		dma8237_update_status(device);
	}
}



static TIMER_CALLBACK( dma8237_msbflip_timerproc )
{
	const device_config *device = ptr;
	dma8237_t	*dma8237 = get_safe_token(device);
	dma8237->msb ^= 1;
}



static void dma8237_update_status(const device_config *device)
{
	dma8237_t	*dma8237 = get_safe_token(device);
	UINT16 pending_transfer;
	int channel;
	UINT32 new_eop;

	if ((dma8237->status & 0xF0) == 0)
	{
		/* no transfer is active right now; is there a transfer pending right now? */
		pending_transfer = dma8237->drq & ~dma8237->mask;

		if (pending_transfer)
		{
			/* we do have a transfer in progress */
			for (channel = 3; (pending_transfer & (1 << channel)) == 0; channel--)
				;

			dma8237->status |= 0x10 << channel;
			dma8237->status &= ~(0x01 << channel);

			timer_adjust_periodic(dma8237->timer,
				attotime_zero,
				channel,
				double_to_attotime(dma8237->intf->bus_speed));
		}
		else
		{
			/* no transfers active right now */
			timer_reset(dma8237->timer, attotime_never);
		}

		/* set the halt line */
		if (dma8237->intf && dma8237->intf->cputag != NULL)
		{
			cputag_set_input_line(device->machine, dma8237->intf->cputag, INPUT_LINE_HALT,
				pending_transfer ? ASSERT_LINE : CLEAR_LINE);
		}

		/* set the eop line, if it has changed */
		new_eop = (dma8237->status & 0x0F) == 0x0F ? 1 : 0;
		if (dma8237->eop != new_eop)
		{
			dma8237->eop = new_eop;
			if (dma8237->intf->out_eop_func)
				dma8237->intf->out_eop_func(device, new_eop ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}



/* ----------------------------------------------------------------------- */


INLINE void prepare_msb_flip(const device_config *device)
{
	dma8237_t	*dma8237 = get_safe_token(device);

	timer_adjust_oneshot(dma8237->msbflip_timer, attotime_zero, 0);
}



READ8_DEVICE_HANDLER( dma8237_r )
{
	dma8237_t	*dma8237 = get_safe_token(device);
	UINT8 data = 0xFF;
	UINT8 mode;

	offset &= 0x0F;

	switch(offset) {
	case 0:
	case 2:
	case 4:
	case 6:
		/* DMA address register */
		data = dma8237->chan[offset / 2].address >> (dma8237->msb ? 8 : 0);
		prepare_msb_flip(device);

		/* hack simulating refresh activity for 'ibmxt' BIOS; I do not know
         * why this is needed; but in any case, the ibmxt driver does not load
         * if this code is not present */
		mode = dma8237->chan[0].mode;
		if ((DMA_MODE_OPERATION(mode) == 2) && (offset == 0))
		{
			dma8237->chan[0].address++;
			dma8237->chan[0].count--;
		}
		break;

	case 1:
	case 3:
	case 5:
	case 7:
		/* DMA count register */
		data = dma8237->chan[offset / 2].count >> (dma8237->msb ? 8 : 0);
		prepare_msb_flip(device);
		break;

	case 8:
		/* DMA status register */
		data = (UINT8) dma8237->status;
		break;

	case 10:
		/* DMA mask register */
		data = dma8237->mask;
		break;

	case 13:
		/* DMA master clear */
		data = dma8237->temp;
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



WRITE8_DEVICE_HANDLER( dma8237_w )
{
	dma8237_t	*dma8237 = get_safe_token(device);
	int channel;

	offset &= 0x0F;

	switch(offset) {
	case 0:
	case 2:
	case 4:
	case 6:
		/* DMA address register */
		if (dma8237->msb)
			dma8237->chan[offset / 2].address |= ((UINT16) data) << 8;
		else
			dma8237->chan[offset / 2].address = data;
		prepare_msb_flip(device);
		break;

	case 1:
	case 3:
	case 5:
	case 7:
		/* DMA count register */
		if (dma8237->msb)
			dma8237->chan[offset / 2].count |= ((UINT16) data) << 8;
		else
			dma8237->chan[offset / 2].count = data;
		prepare_msb_flip(device);
		break;

	case 8:
		/* DMA command register */
		dma8237->command = data;
		break;

	case 10:
		/* DMA mask register */
		channel = DMA_MODE_CHANNEL(data);
		if (data & 0x04)
			dma8237->mask |= 0x11 << channel;
		else
			dma8237->mask &= ~(0x11 << channel);
		break;

	case 11:
		/* DMA mode register */
		channel = DMA_MODE_CHANNEL(data);
		dma8237->chan[channel].mode = data;
		break;

	case 12:
		/* DMA clear byte pointer flip-flop */
		dma8237->temp = data;
		dma8237->msb = 0;
		break;

	case 13:
		/* DMA master clear */
		dma8237->msb = 0;
		break;

	case 14:
		/* DMA clear mask register */
		dma8237->mask &= ~data;
		dma8237_update_status(device);
		break;

	case 15:
		/* DMA write mask register */
		dma8237->mask |= data;
		break;
	}
}



static void dma8237_drq_write_callback(const device_config *device, int param)
{
	dma8237_t	*dma8237 = get_safe_token(device);
	int channel = (param >> 1) & 0x03;
	int state = param & 0x01;

	/* normalize state */
	if (state)
		dma8237->drq |= 0x01 << channel;
	else
		dma8237->drq &= ~(0x01 << channel);

	dma8237_update_status(device);
}



void dma8237_drq_write(const device_config *device, int channel, int state)
{
	int param = (channel << 1) | (state ? 1 : 0);
	//timer_call_after_resynch(device->machine, NULL, param, dma8237_drq_write_callback);
	dma8237_drq_write_callback(device, param);
}



/******************* Unfortunate hacks *******************/

void dma8237_run_transfer(const device_config *device, int channel)
{
	dma8237_t	*dma8237 = get_safe_token(device);

	dma8237->status |= 0x10 << channel;	/* reset DMA running flag */

	while(!dma8237_do_operation(device, channel))
		;

	dma8237->status &= ~(0x10 << channel);
	dma8237->status |=  (0x01 << channel);
}



static DEVICE_START( dma8237 ) {
	dma8237_t	*dma8237 = get_safe_token(device);

	dma8237->intf = device->static_config;

	return DEVICE_START_OK;
}


static DEVICE_RESET( dma8237 ) {
	dma8237_t	*dma8237 = get_safe_token(device);

	dma8237->status = 0x0F;
	dma8237->timer = timer_alloc(device->machine, dma8237_timerproc, (void *)device);
	dma8237->msbflip_timer = timer_alloc(device->machine, dma8237_msbflip_timerproc, (void *)device);
	dma8237->eop = 1;

	dma8237->mask = 0x00;
	dma8237->status = 0x0F;
	dma8237->chan[0].mode = 0;
	dma8237->chan[1].mode = 0;
	dma8237->chan[2].mode = 0;
	dma8237->chan[3].mode = 0;

	dma8237_update_status(device);
}


static DEVICE_SET_INFO( dma8237 ) {
	switch ( state ) {
		/* no parameters to set */
	}
}


DEVICE_GET_INFO( dma8237 ) {
	switch ( state ) {
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = sizeof(dma8237_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:		info->i = 0;								break;
		case DEVINFO_INT_CLASS:						info->i = DEVICE_CLASS_PERIPHERAL;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:					info->set_info = DEVICE_SET_INFO_NAME(dma8237);	break;
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(dma8237);	break;
		case DEVINFO_FCT_STOP:						/* nothing */								break;
		case DEVINFO_FCT_RESET:						info->reset = DEVICE_RESET_NAME(dma8237);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						info->s = "Intel DMA8237";					break;
		case DEVINFO_STR_FAMILY:					info->s = "DMA8237";						break;
		case DEVINFO_STR_VERSION:					info->s = "1.00";							break;
		case DEVINFO_STR_SOURCE_FILE:				info->s = __FILE__;							break;
		case DEVINFO_STR_CREDITS:					info->s = "Copyright the MAME and MESS Teams";	break;
	}
}

