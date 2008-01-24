/**********************************************************************

    8257 DMA interface and emulation

    For datasheet http://www.threedee.com/jcm/library/index.html

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

struct dma8257
{
	const struct dma8257_interface *intf;
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

static struct dma8257 *dma;
static int dma_count;

#define DMA_MODE_AUTOLOAD(mode)		((mode) & 0x80)
#define DMA_MODE_TCSTOP(mode)		((mode) & 0x40)
#define DMA_MODE_EXWRITE(mode)		((mode) & 0x20)
#define DMA_MODE_ROTPRIO(mode)		((mode) & 0x10)
#define DMA_MODE_CH_EN(mode, chan)	((mode) & (1 << (chan)))

static TIMER_CALLBACK( dma8257_timerproc );
static TIMER_CALLBACK( dma8257_msbflip_timerproc );
static void dma8257_update_status(int which);

/* ----------------------------------------------------------------------- */

int dma8257_init(int count)
{
	int which;

	dma = auto_malloc(count * sizeof(struct dma8257));
	memset(dma, 0, count * sizeof(struct dma8257));
	dma_count = count;

	for (which = 0; which < dma_count; which++)
	{
		dma[which].status = 0x0F;
		dma[which].timer = timer_alloc(dma8257_timerproc, NULL);
		dma[which].msbflip_timer = timer_alloc(dma8257_msbflip_timerproc, NULL);

		state_save_register_item_array("8257", which, dma[which].address);
		state_save_register_item_array("8257", which, dma[which].count);
		state_save_register_item_array("8257", which, dma[which].rwmode);
		state_save_register_item_array("8257", which, dma[which].registers);

		state_save_register_item("8257", which, dma[which].mode);
		state_save_register_item("8257", which, dma[which].rr);
		state_save_register_item("8257", which, dma[which].msb);
		state_save_register_item("8257", which, dma[which].drq);
		state_save_register_item("8257", which, dma[which].status);

	}

	return 0;
}



void dma8257_config(int which, const struct dma8257_interface *intf)
{
	dma[which].intf = intf;
}



void dma8257_reset(void)
{
	int which;

	for (which = 0; which < dma_count; which++)
	{
		dma[which].status &= 0xF0;
		dma[which].mode = 0;
		dma8257_update_status(which);
	}
}



/* ----------------------------------------------------------------------- */



static int dma8257_do_operation(int which, int channel)
{
	int done;
	UINT8 data;
	UINT8 mode;

	mode = dma[which].rwmode[channel];
	if (dma[which].count[channel] == 0x0000)
	{
		dma[which].status |=  (0x01 << channel);
		if (dma[which].intf->out_tc_func[channel])
			dma[which].intf->out_tc_func[channel](ASSERT_LINE);
	}
	switch(mode) {
	case 1:
		data = dma[which].intf->memory_read_func(channel, dma[which].address[channel]);
		dma[which].intf->channel_write_func[channel](data);

		dma[which].address[channel]++;
		dma[which].count[channel]--;
		done = (dma[which].count[channel] == 0xFFFF);
		break;

	case 2:
		data = dma[which].intf->channel_read_func[channel]();
		dma[which].intf->memory_write_func(channel, dma[which].address[channel], data);

		dma[which].address[channel]++;
		dma[which].count[channel]--;
		done = (dma[which].count[channel] == 0xFFFF);
		break;
	case 0: /* verify */
		dma[which].address[channel]++;
		dma[which].count[channel]--;
		done = (dma[which].count[channel] == 0xFFFF);
		break;
	default:
		fatalerror("dma8257_do_operation: invalid mode!\n");
		break;
	}
	if (done)
	{
		if (dma[which].intf->out_tc_func[channel])
			dma[which].intf->out_tc_func[channel](CLEAR_LINE);
	}
	return done;
}



static TIMER_CALLBACK( dma8257_timerproc )
{
	int which = param;
	int i, channel = 0, rr;
	int done;

	rr = DMA_MODE_ROTPRIO(dma[which].mode) ? dma[which].rr : 0;
	for (i = 0; i < DMA8257_NUM_CHANNELS; i++)
	{
		channel = (i + rr) % DMA8257_NUM_CHANNELS;
		if ((dma[which].status & (1 << channel)) == 0)
			if (dma[which].mode & dma[which].drq & (1 << channel))
				break;
	}
	done = dma8257_do_operation(which, channel);

	dma[which].rr = (channel + 1) & 0x03;

	if (done)
	{
		dma[which].drq    &= ~(0x01 << channel);
		dma8257_update_status(which);
		if (DMA_MODE_TCSTOP(dma[which].mode))
			dma[which].mode &= ~(0x01 << channel);
	}
}



static TIMER_CALLBACK( dma8257_msbflip_timerproc )
{
	dma[param].msb ^= 1;
}



static void dma8257_update_status(int which)
{
	UINT16 pending_transfer;
	attotime next;

	/* no transfer is active right now; is there a transfer pending right now? */
	pending_transfer = dma[which].drq & (dma[which].mode & 0x0F);

	if (pending_transfer)
	{
		next = ATTOTIME_IN_HZ(dma[which].intf->clockhz / 4 );
		timer_adjust(dma[which].timer,
			attotime_zero,
			which,
			/* 1 byte transferred in 4 clock cycles */
			next);
	}
	else
	{
		/* no transfers active right now */
		timer_reset(dma[which].timer, attotime_never);
	}

	/* set the halt line */
	if (dma[which].intf && dma[which].intf->cpunum >= 0)
	{
		cpunum_set_input_line(Machine, dma[which].intf->cpunum, INPUT_LINE_HALT,
			pending_transfer ? ASSERT_LINE : CLEAR_LINE);
	}

}



/* ----------------------------------------------------------------------- */

static void prepare_msb_flip(int which)
{
	timer_adjust(dma[which].msbflip_timer, attotime_zero, which, attotime_zero);
}



static UINT8 dma8257_read(int which, offs_t offset)
{
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
		data = ( dma[which].registers[offset] >> (dma[which].msb ? 8 : 0) ) & 0xFF;
		prepare_msb_flip(which);
		break;

	case 8:
		/* DMA status register */
		data = (UINT8) dma[which].status;
		/* read resets status ! */
		dma[which].status &= 0xF0;

		break;

	default:
		logerror("8257: Read from register %d.\n", offset);
		data = 0xFF;
		break;
	}
	return data;
}



static void dma8257_write(int which, offs_t offset, UINT8 data)
{

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
		if (dma[which].msb)
			dma[which].registers[offset] |= ((UINT16) data) << 8;
		else
			dma[which].registers[offset] = data;
		prepare_msb_flip(which);
		break;

	case 8:
		/* DMA mode register */
		if (DMA_MODE_AUTOLOAD(data)	)
			fatalerror("8257: Autoload not supported!\n");
		dma[which].mode = data;
		break;

	default:
		logerror("8257: Write to register %d.\n", offset);
		break;
	}
}



static TIMER_CALLBACK( dma8257_drq_write_callback )
{
	int which = param >> 3;
	int channel = (param >> 1) & 0x03;
	int state = param & 0x01;

	/* normalize state */
	if (state)
	{
		dma[which].drq |= 0x01 << channel;
		dma[which].address[channel] =  dma[which].registers[channel * 2];
		dma[which].count[channel] =  dma[which].registers[channel * 2 + 1] & 0x3FF;
		dma[which].rwmode[channel] =  dma[which].registers[channel * 2 + 1] >> 14;
		/* clear channel TC */
		dma[which].status &= ~(0x01 << channel);
	}
	else
		dma[which].drq &= ~(0x01 << channel);

	dma8257_update_status(which);
}



void dma8257_drq_write(int which, int channel, int state)
{
	int param;

	param = (which << 3) | (channel << 1) | (state ? 1 : 0);
	timer_call_after_resynch(NULL, param, dma8257_drq_write_callback);
}


/******************* Standard 8-bit/32-bit/64-bit CPU interfaces *******************/

READ8_HANDLER( dma8257_0_r )	{ return dma8257_read(0, offset); }
READ8_HANDLER( dma8257_1_r )	{ return dma8257_read(1, offset); }
WRITE8_HANDLER( dma8257_0_w ) { dma8257_write(0, offset, data); }
WRITE8_HANDLER( dma8257_1_w ) { dma8257_write(1, offset, data); }

READ16_HANDLER( dma8257_16le_0_r ) { return read16le_with_read8_handler(dma8257_0_r, offset, mem_mask); }
READ16_HANDLER( dma8257_16le_1_r ) { return read16le_with_read8_handler(dma8257_1_r, offset, mem_mask); }
WRITE16_HANDLER( dma8257_16le_0_w ) { write16le_with_write8_handler(dma8257_0_w, offset, data, mem_mask); }
WRITE16_HANDLER( dma8257_16le_1_w ) { write16le_with_write8_handler(dma8257_1_w, offset, data, mem_mask); }

READ32_HANDLER( dma8257_32le_0_r ) { return read32le_with_read8_handler(dma8257_0_r, offset, mem_mask); }
READ32_HANDLER( dma8257_32le_1_r ) { return read32le_with_read8_handler(dma8257_1_r, offset, mem_mask); }
WRITE32_HANDLER( dma8257_32le_0_w ) { write32le_with_write8_handler(dma8257_0_w, offset, data, mem_mask); }
WRITE32_HANDLER( dma8257_32le_1_w ) { write32le_with_write8_handler(dma8257_1_w, offset, data, mem_mask); }

READ64_HANDLER( dma8257_64be_0_r ) { return read64be_with_read8_handler(dma8257_0_r, offset, mem_mask); }
READ64_HANDLER( dma8257_64be_1_r ) { return read64be_with_read8_handler(dma8257_1_r, offset, mem_mask); }
WRITE64_HANDLER( dma8257_64be_0_w ) { write64be_with_write8_handler(dma8257_0_w, offset, data, mem_mask); }
WRITE64_HANDLER( dma8257_64be_1_w ) { write64be_with_write8_handler(dma8257_1_w, offset, data, mem_mask); }
