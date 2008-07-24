/**********************************************************************

    8257 DMA interface and emulation

    For datasheet http://www.threedee.com/jcm/library/index.html

    2008/05     Miodrag Milanovic

        - added support for autoload mode
        - fixed bug in calculating count

    2007/11     couriersud

        - architecture copied from 8237 DMA
        - significant changes to implementation

    The DMA works like this:

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
#include "8257dma.h"

typedef struct _dma8257_t dma8257_t;
struct _dma8257_t
{
	const dma8257_interface *intf;
	emu_timer *timer;
	emu_timer *msbflip_timer;

	UINT16 registers[DMA8257_NUM_CHANNELS*2];

	UINT16 address[DMA8257_NUM_CHANNELS];
	UINT16 count[DMA8257_NUM_CHANNELS];
	UINT8  rwmode[DMA8257_NUM_CHANNELS];

	UINT8 mode;
	UINT8 rr;

	UINT8 msb;
	UINT8 drq;

	/* bits  0- 3 :  Terminal count for channels 0-3 */
	UINT8 status;
};

#define DMA_MODE_AUTOLOAD(mode)		((mode) & 0x80)
#define DMA_MODE_TCSTOP(mode)		((mode) & 0x40)
#define DMA_MODE_EXWRITE(mode)		((mode) & 0x20)
#define DMA_MODE_ROTPRIO(mode)		((mode) & 0x10)
#define DMA_MODE_CH_EN(mode, chan)	((mode) & (1 << (chan)))

static TIMER_CALLBACK( dma8257_timerproc );
static TIMER_CALLBACK( dma8257_msbflip_timerproc );
static void dma8257_update_status(running_machine *machine, dma8257_t *dma8257);


/* ----------------------------------------------------------------------- */

INLINE dma8257_t *get_safe_token(const device_config *device) {
	assert( device != NULL );
	assert( device->token != NULL );
	assert( device->type == DMA8257 );
	return ( dma8257_t * ) device->token;
}

static int dma8257_do_operation(running_machine *machine, dma8257_t *dma8257, int channel)
{
	int done;
	UINT8 data;
	UINT8 mode;

	mode = dma8257->rwmode[channel];
	if (dma8257->count[channel] == 0x0000)
	{
		dma8257->status |=  (0x01 << channel);
		if (dma8257->intf->out_tc[channel])
			dma8257->intf->out_tc[channel](machine, 0, ASSERT_LINE);
	}
	switch(mode) {
	case 1:
		if (dma8257->intf->memory_read!=NULL) {
			data = dma8257->intf->memory_read(machine, dma8257->address[channel]);
		} else {
			data = 0;
			logerror("8257: No memory read function defined.\n");
		}
		if (dma8257->intf->channel_write[channel]!=NULL) {
			dma8257->intf->channel_write[channel](machine, 0, data);
		} else {
			logerror("8257: No channel write function for channel %d defined.\n",channel);
		}

		dma8257->address[channel]++;
		dma8257->count[channel]--;
		done = (dma8257->count[channel] == 0xFFFF);
		break;

	case 2:
		if (dma8257->intf->channel_read[channel]!=NULL) {
			data = dma8257->intf->channel_read[channel](machine, 0);
		} else {
			data = 0;
			logerror("8257: No channel read function for channel %d defined.\n",channel);
		}
		
		if (dma8257->intf->memory_write!=NULL) {
			dma8257->intf->memory_write(machine, dma8257->address[channel], data);
		} else {
			logerror("8257: No memory write function defined.\n");
		}
		dma8257->address[channel]++;
		dma8257->count[channel]--;
		done = (dma8257->count[channel] == 0xFFFF);
		break;
	case 0: /* verify */
		dma8257->address[channel]++;
		dma8257->count[channel]--;
		done = (dma8257->count[channel] == 0xFFFF);
		break;
	default:
		fatalerror("dma8257_do_operation: invalid mode!\n");
		break;
	}
	if (done)
	{
		if ((channel==2) && DMA_MODE_AUTOLOAD(dma8257->mode)) {
			/* in case of autoload at the end channel 3 info is */
			/* copied to channel 2 info                         */
			dma8257->registers[4] = dma8257->registers[6];
			dma8257->registers[5] = dma8257->registers[7];
		}
		if (dma8257->intf->out_tc[channel])
			dma8257->intf->out_tc[channel](machine, 0, CLEAR_LINE);
	}
	return done;
}



static TIMER_CALLBACK( dma8257_timerproc )
{
	dma8257_t *dma8257 = ptr;
	int i, channel = 0, rr;
	int done;

	rr = DMA_MODE_ROTPRIO(dma8257->mode) ? dma8257->rr : 0;
	for (i = 0; i < DMA8257_NUM_CHANNELS; i++)
	{
		channel = (i + rr) % DMA8257_NUM_CHANNELS;
		if ((dma8257->status & (1 << channel)) == 0)
			if (dma8257->mode & dma8257->drq & (1 << channel))
				break;
	}
	done = dma8257_do_operation(machine, dma8257, channel);

	dma8257->rr = (channel + 1) & 0x03;

	if (done)
	{
		dma8257->drq    &= ~(0x01 << channel);
		dma8257_update_status(machine, dma8257);
  		if (!(DMA_MODE_AUTOLOAD(dma8257->mode) && channel==2)) {		
			if (DMA_MODE_TCSTOP(dma8257->mode)) {
				dma8257->mode &= ~(0x01 << channel);
			}
		}
	}
}



static TIMER_CALLBACK( dma8257_msbflip_timerproc )
{
	dma8257_t *dma8257 = ptr;
	dma8257->msb ^= 1;
}



static void dma8257_update_status(running_machine *machine, dma8257_t *dma8257)
{
	UINT16 pending_transfer;
	attotime next;

	/* no transfer is active right now; is there a transfer pending right now? */
	pending_transfer = dma8257->drq & (dma8257->mode & 0x0F);

	if (pending_transfer)
	{
		next = ATTOTIME_IN_HZ(dma8257->intf->clockhz / 4 );
		timer_adjust_periodic(dma8257->timer,
			attotime_zero,
			0,
			/* 1 byte transferred in 4 clock cycles */
			next);
	}
	else
	{
		/* no transfers active right now */
		timer_reset(dma8257->timer, attotime_never);
	}

	/* set the halt line */
	if (dma8257->intf && dma8257->intf->cpunum >= 0)
	{
		cpunum_set_input_line(machine, dma8257->intf->cpunum, INPUT_LINE_HALT,
			pending_transfer ? ASSERT_LINE : CLEAR_LINE);
	}

}



/* ----------------------------------------------------------------------- */

static void prepare_msb_flip(dma8257_t *dma8257)
{
	timer_adjust_oneshot(dma8257->msbflip_timer, attotime_zero, 0);
}



READ8_DEVICE_HANDLER( dma8257_r )
{
	dma8257_t *dma8257 = get_safe_token(device);
	UINT8 data = 0xFF;

	switch(offset) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		/* DMA address/count register */
		data = ( dma8257->registers[offset] >> (dma8257->msb ? 8 : 0) ) & 0xFF;
		prepare_msb_flip(dma8257);
		break;

	case 8:
		/* DMA status register */
		data = (UINT8) dma8257->status;
		/* read resets status ! */
		dma8257->status &= 0xF0;

		break;

	default:
		logerror("8257: Read from register %d.\n", offset);
		data = 0xFF;
		break;
	}
	return data;
}



WRITE8_DEVICE_HANDLER( dma8257_w )
{
	dma8257_t *dma8257 = get_safe_token(device);

	switch(offset) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		/* DMA address/count register */
		if (dma8257->msb)
			dma8257->registers[offset] |= ((UINT16) data) << 8;
		else
			dma8257->registers[offset] = data;

		if (DMA_MODE_AUTOLOAD(dma8257->mode)) {
			/* in case of autoload when inserting channel 2 info */
			/* it is automaticaly copied to channel 3 info       */
			switch(offset) {
				case 4:
				case 5:
					if (dma8257->msb)
						dma8257->registers[offset+2] |= ((UINT16) data) << 8;
					else
						dma8257->registers[offset+2] = data;
			}
		}

		prepare_msb_flip(dma8257);
		break;

	case 8:
		/* DMA mode register */
		dma8257->mode = data;
		break;

	default:
		logerror("8257: Write to register %d.\n", offset);
		break;
	}
}



static TIMER_CALLBACK( dma8257_drq_write_callback )
{
	int channel = param >> 1;
	int state = param & 0x01;
	dma8257_t *dma8257 = ptr;

	/* normalize state */
	if (state)
	{
		dma8257->drq |= 0x01 << channel;
		dma8257->address[channel] =  dma8257->registers[channel * 2];
		dma8257->count[channel] =  dma8257->registers[channel * 2 + 1] & 0x3FFF;
		dma8257->rwmode[channel] =  dma8257->registers[channel * 2 + 1] >> 14;
		/* clear channel TC */
		dma8257->status &= ~(0x01 << channel);
	}
	else
		dma8257->drq &= ~(0x01 << channel);

	dma8257_update_status(machine, dma8257);
}



WRITE8_DEVICE_HANDLER( dma8257_drq_w )
{
	dma8257_t *dma8257 = get_safe_token(device);
	int param = (offset << 1) | (data ? 1 : 0);

	timer_call_after_resynch(dma8257, param, dma8257_drq_write_callback);
}


/* ----------------------------------------------------------------------- */

/* device interface */

static DEVICE_START( dma8257 )
{
	dma8257_t *dma8257 = get_safe_token(device);
	char unique_tag[30];

	/* validate arguments */
	assert(device != NULL);
	assert(device->tag != NULL);
	assert(strlen(device->tag) < 20);

	//dma8257->device_type = device_type;
	dma8257->intf = device->static_config;

	dma8257->status = 0x0f;
	dma8257->timer = timer_alloc(dma8257_timerproc, dma8257);
	dma8257->msbflip_timer = timer_alloc(dma8257_msbflip_timerproc, dma8257);

	state_save_combine_module_and_tag(unique_tag, "dma8257", device->tag);

	state_save_register_item_array(unique_tag, 0, dma8257->address);
	state_save_register_item_array(unique_tag, 0, dma8257->count);
	state_save_register_item_array(unique_tag, 0, dma8257->rwmode);
	state_save_register_item_array(unique_tag, 0, dma8257->registers);

	state_save_register_item(unique_tag, 0, dma8257->mode);
	state_save_register_item(unique_tag, 0, dma8257->rr);
	state_save_register_item(unique_tag, 0, dma8257->msb);
	state_save_register_item(unique_tag, 0, dma8257->drq);
	state_save_register_item(unique_tag, 0, dma8257->status);

}


static DEVICE_RESET( dma8257 )
{
	dma8257_t *dma8257 = get_safe_token(device);

	dma8257->status &= 0xf0;
	dma8257->mode = 0;
	dma8257_update_status(device->machine, dma8257 );
}


static DEVICE_SET_INFO( dma8257 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


DEVICE_GET_INFO( dma8257 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(dma8257_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:						info->set_info = DEVICE_SET_INFO_NAME(dma8257); break;
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(dma8257);break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(dma8257);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							info->s = "DMA8257";					break;
		case DEVINFO_STR_FAMILY:						info->s = "DMA controllers";			break;
		case DEVINFO_STR_VERSION:						info->s = "1.0";						break;
		case DEVINFO_STR_SOURCE_FILE:					info->s = __FILE__;						break;
		case DEVINFO_STR_CREDITS:						info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
