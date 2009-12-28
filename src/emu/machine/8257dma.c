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

#define I8257_STATUS_UPDATE		0x10
#define I8257_STATUS_TC_CH3		0x08
#define I8257_STATUS_TC_CH2		0x04
#define I8257_STATUS_TC_CH1		0x02
#define I8257_STATUS_TC_CH0		0x01


typedef struct _dma8257_t i8257_t;
struct _dma8257_t
{
	devcb_resolved_write_line	out_hrq_func;
	devcb_resolved_write_line	out_tc_func;
	devcb_resolved_write_line	out_mark_func;
	devcb_resolved_read8		in_memr_func;
	devcb_resolved_write8		out_memw_func;
	devcb_resolved_read8		in_ior_func[I8257_NUM_CHANNELS];
	devcb_resolved_write8		out_iow_func[I8257_NUM_CHANNELS];

	emu_timer *timer;
	emu_timer *msbflip_timer;

	UINT16 registers[I8257_NUM_CHANNELS*2];

	UINT16 address[I8257_NUM_CHANNELS];
	UINT16 count[I8257_NUM_CHANNELS];
	UINT8  rwmode[I8257_NUM_CHANNELS];

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
static void dma8257_update_status(const device_config *device);


/* ----------------------------------------------------------------------- */

INLINE i8257_t *get_safe_token(const device_config *device) {
	assert( device != NULL );
	assert( device->token != NULL );
	assert( device->type == I8257 );
	return ( i8257_t * ) device->token;
}

static int dma8257_do_operation(const device_config *device, int channel)
{
	i8257_t *i8257 = get_safe_token(device);
	int done;
	UINT8 data;
	UINT8 mode;

	mode = i8257->rwmode[channel];
	if (i8257->count[channel] == 0x0000)
	{
		i8257->status |=  (0x01 << channel);

		devcb_call_write_line(&i8257->out_tc_func, ASSERT_LINE);
	}
	switch(mode) {
	case 1:
		if (&i8257->in_memr_func.target != NULL) {
			data = devcb_call_read8(&i8257->in_memr_func, i8257->address[channel]);
		} else {
			data = 0;
			logerror("8257: No memory read function defined.\n");
		}
		if (&i8257->out_iow_func[channel].target != NULL) {
			devcb_call_write8(&i8257->out_iow_func[channel], 0, data);
		} else {
			logerror("8257: No channel write function for channel %d defined.\n",channel);
		}

		i8257->address[channel]++;
		i8257->count[channel]--;
		done = (i8257->count[channel] == 0xFFFF);
		break;

	case 2:
		if (&i8257->in_ior_func[channel].target != NULL) {
			data = devcb_call_read8(&i8257->in_ior_func[channel], 0);
		} else {
			data = 0;
			logerror("8257: No channel read function for channel %d defined.\n",channel);
		}

		if (&i8257->out_memw_func.target != NULL) {
			devcb_call_write8(&i8257->out_memw_func, i8257->address[channel], data);
		} else {
			logerror("8257: No memory write function defined.\n");
		}
		i8257->address[channel]++;
		i8257->count[channel]--;
		done = (i8257->count[channel] == 0xFFFF);
		break;
	case 0: /* verify */
		i8257->address[channel]++;
		i8257->count[channel]--;
		done = (i8257->count[channel] == 0xFFFF);
		break;
	default:
		fatalerror("dma8257_do_operation: invalid mode!\n");
		break;
	}
	if (done)
	{
		if ((channel==2) && DMA_MODE_AUTOLOAD(i8257->mode)) {
			/* in case of autoload at the end channel 3 info is */
			/* copied to channel 2 info                         */
			i8257->registers[4] = i8257->registers[6];
			i8257->registers[5] = i8257->registers[7];
		}

		devcb_call_write_line(&i8257->out_tc_func, CLEAR_LINE);
	}
	return done;
}



static TIMER_CALLBACK( dma8257_timerproc )
{
	const device_config *device = (const device_config *)ptr;
	i8257_t *i8257 = get_safe_token(device);
	int i, channel = 0, rr;
	int done;

	rr = DMA_MODE_ROTPRIO(i8257->mode) ? i8257->rr : 0;
	for (i = 0; i < I8257_NUM_CHANNELS; i++)
	{
		channel = (i + rr) % I8257_NUM_CHANNELS;
		if ((i8257->status & (1 << channel)) == 0)
			if (i8257->mode & i8257->drq & (1 << channel))
				break;
	}
	done = dma8257_do_operation(device, channel);

	i8257->rr = (channel + 1) & 0x03;

	if (done)
	{
		i8257->drq    &= ~(0x01 << channel);
		dma8257_update_status(device);
		if (!(DMA_MODE_AUTOLOAD(i8257->mode) && channel==2)) {
			if (DMA_MODE_TCSTOP(i8257->mode)) {
				i8257->mode &= ~(0x01 << channel);
			}
		}
	}
}



static TIMER_CALLBACK( dma8257_msbflip_timerproc )
{
	const device_config *device = (const device_config *)ptr;
	i8257_t *i8257 = get_safe_token(device);
	i8257->msb ^= 1;
}



static void dma8257_update_status(const device_config *device)
{
	i8257_t *i8257 = get_safe_token(device);
	UINT16 pending_transfer;
	attotime next;

	/* no transfer is active right now; is there a transfer pending right now? */
	pending_transfer = i8257->drq & (i8257->mode & 0x0F);

	if (pending_transfer)
	{
		next = ATTOTIME_IN_HZ(device->clock / 4 );
		timer_adjust_periodic(i8257->timer,
			attotime_zero,
			0,
			/* 1 byte transferred in 4 clock cycles */
			next);
	}
	else
	{
		/* no transfers active right now */
		timer_reset(i8257->timer, attotime_never);
	}

	/* set the halt line */
	devcb_call_write_line(&i8257->out_hrq_func, pending_transfer ? ASSERT_LINE : CLEAR_LINE);
}



/* ----------------------------------------------------------------------- */

static void prepare_msb_flip(i8257_t *i8257)
{
	timer_adjust_oneshot(i8257->msbflip_timer, attotime_zero, 0);
}



READ8_DEVICE_HANDLER( i8257_r )
{
	i8257_t *i8257 = get_safe_token(device);
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
		data = ( i8257->registers[offset] >> (i8257->msb ? 8 : 0) ) & 0xFF;
		prepare_msb_flip(i8257);
		break;

	case 8:
		/* DMA status register */
		data = (UINT8) i8257->status;
		/* read resets status ! */
		i8257->status &= 0xF0;

		break;

	default:
		logerror("8257: Read from register %d.\n", offset);
		data = 0xFF;
		break;
	}
	return data;
}



WRITE8_DEVICE_HANDLER( i8257_w )
{
	i8257_t *i8257 = get_safe_token(device);

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
		if (i8257->msb)
			i8257->registers[offset] |= ((UINT16) data) << 8;
		else
			i8257->registers[offset] = data;

		if (DMA_MODE_AUTOLOAD(i8257->mode)) {
			/* in case of autoload when inserting channel 2 info */
			/* it is automaticaly copied to channel 3 info       */
			switch(offset) {
				case 4:
				case 5:
					if (i8257->msb)
						i8257->registers[offset+2] |= ((UINT16) data) << 8;
					else
						i8257->registers[offset+2] = data;
			}
		}

		prepare_msb_flip(i8257);
		break;

	case 8:
		/* DMA mode register */
		i8257->mode = data;
		break;

	default:
		logerror("8257: Write to register %d.\n", offset);
		break;
	}
}



static TIMER_CALLBACK( dma8257_drq_write_callback )
{
	const device_config *device = (const device_config *)ptr;
	i8257_t *i8257 = get_safe_token(device);
	int channel = param >> 1;
	int state = param & 0x01;

	/* normalize state */
	if (state)
	{
		i8257->drq |= 0x01 << channel;
		i8257->address[channel] =  i8257->registers[channel * 2];
		i8257->count[channel] =  i8257->registers[channel * 2 + 1] & 0x3FFF;
		i8257->rwmode[channel] =  i8257->registers[channel * 2 + 1] >> 14;
		/* clear channel TC */
		i8257->status &= ~(0x01 << channel);
	}
	else
		i8257->drq &= ~(0x01 << channel);

	dma8257_update_status(device);
}



static WRITE8_DEVICE_HANDLER( dma8257_drq_w )
{
	int param = (offset << 1) | (data ? 1 : 0);

	timer_call_after_resynch(device->machine, (void *) device, param, dma8257_drq_write_callback);
}

WRITE_LINE_DEVICE_HANDLER( i8257_hlda_w )
{
}

WRITE_LINE_DEVICE_HANDLER( i8257_ready_w )
{
}

WRITE_LINE_DEVICE_HANDLER( i8257_drq0_w ) { dma8257_drq_w(device, 0, state); }
WRITE_LINE_DEVICE_HANDLER( i8257_drq1_w ) { dma8257_drq_w(device, 1, state); }
WRITE_LINE_DEVICE_HANDLER( i8257_drq2_w ) { dma8257_drq_w(device, 2, state); }
WRITE_LINE_DEVICE_HANDLER( i8257_drq3_w ) { dma8257_drq_w(device, 3, state); }


/* ----------------------------------------------------------------------- */

/* device interface */

static DEVICE_START( i8257 )
{
	i8257_t *i8257 = get_safe_token(device);
	i8257_interface *intf = (i8257_interface *)device->static_config;
	int i;

	/* validate arguments */
	assert(device != NULL);
	assert(device->tag != NULL);

	/* resolve callbacks */
	devcb_resolve_write_line(&i8257->out_hrq_func, &intf->out_hrq_func, device);
	devcb_resolve_write_line(&i8257->out_tc_func, &intf->out_tc_func, device);
	devcb_resolve_write_line(&i8257->out_mark_func, &intf->out_mark_func, device);
	devcb_resolve_read8(&i8257->in_memr_func, &intf->in_memr_func, device);
	devcb_resolve_write8(&i8257->out_memw_func, &intf->out_memw_func, device);

	for (i = 0; i < I8257_NUM_CHANNELS; i++)
	{
		devcb_resolve_read8(&i8257->in_ior_func[i], &intf->in_ior_func[i], device);
		devcb_resolve_write8(&i8257->out_iow_func[i], &intf->out_iow_func[i], device);
	}

	/* set initial values */
	i8257->status = 0x0f;
	i8257->timer = timer_alloc(device->machine, dma8257_timerproc, (void *) device);
	i8257->msbflip_timer = timer_alloc(device->machine, dma8257_msbflip_timerproc, (void *) device);

	/* register for state saving */
	state_save_register_device_item_array(device, 0, i8257->address);
	state_save_register_device_item_array(device, 0, i8257->count);
	state_save_register_device_item_array(device, 0, i8257->rwmode);
	state_save_register_device_item_array(device, 0, i8257->registers);
	state_save_register_device_item(device, 0, i8257->mode);
	state_save_register_device_item(device, 0, i8257->rr);
	state_save_register_device_item(device, 0, i8257->msb);
	state_save_register_device_item(device, 0, i8257->drq);
	state_save_register_device_item(device, 0, i8257->status);
}


static DEVICE_RESET( i8257 )
{
	i8257_t *i8257 = get_safe_token(device);

	i8257->status &= 0xf0;
	i8257->mode = 0;
	dma8257_update_status(device);
}


DEVICE_GET_INFO( i8257 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(i8257_t);			break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(i8257);break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(i8257);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "DMA8257");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "DMA controllers");		break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
