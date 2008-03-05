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
#include "deprecat.h"
#include "memconv.h"
#include "8237dma.h"

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

static struct dma8237 *dma;
static int dma_count;


#define DMA_MODE_CHANNEL(mode)		((mode) & 0x03)
#define DMA_MODE_OPERATION(mode)	(((mode) >> 2) & 0x03)
#define DMA_MODE_DIRECTION(mode)	(((mode) & 0x20) ? -1 : +1)
#define DMA_MODE_TRANSFERMODE(mode)	(((mode) >> 6) & 0x03)



static TIMER_CALLBACK( dma8237_timerproc );
static TIMER_CALLBACK( dma8237_msbflip_timerproc );
static void dma8237_update_status(int which);

/* ----------------------------------------------------------------------- */

int dma8237_init(int count)
{
	int which;

	dma = auto_malloc(count * sizeof(struct dma8237));
	memset(dma, 0, count * sizeof(struct dma8237));
	dma_count = count;

	for (which = 0; which < dma_count; which++)
	{
		dma[which].status = 0x0F;
		dma[which].timer = timer_alloc(dma8237_timerproc, NULL);
		dma[which].msbflip_timer = timer_alloc(dma8237_msbflip_timerproc, NULL);
		dma[which].eop = 1;
	}
	return 0;
}



void dma8237_config(int which, const struct dma8237_interface *intf)
{
	dma[which].intf = intf;
}



void dma8237_reset(void)
{
	int which;

	for (which = 0; which < dma_count; which++)
	{
		dma[which].mask = 0x00;
		dma[which].status = 0x0F;
		dma[which].chan[0].mode = 0;
		dma[which].chan[1].mode = 0;
		dma[which].chan[2].mode = 0;
		dma[which].chan[3].mode = 0;
		dma8237_update_status(which);
	}
}



/* ----------------------------------------------------------------------- */



static int dma8237_do_operation(int which, int channel)
{
	int done;
	UINT8 data;
	UINT8 mode;

	mode = dma[which].chan[channel].mode;

	switch(DMA_MODE_OPERATION(mode)) {
	case 1:
		data = dma[which].intf->channel_read_func[channel]();
		dma[which].intf->memory_write_func(channel, dma[which].chan[channel].address, data);

		dma[which].chan[channel].address += DMA_MODE_DIRECTION(mode);
		dma[which].chan[channel].count--;
		done = (dma[which].chan[channel].count == 0xFFFF);
		break;

	case 2:
		data = dma[which].intf->memory_read_func(channel, dma[which].chan[channel].address);
		dma[which].intf->channel_write_func[channel](data);

		dma[which].chan[channel].address += DMA_MODE_DIRECTION(mode);
		dma[which].chan[channel].count--;
		done = (dma[which].chan[channel].count == 0xFFFF);
		break;

	default:
		done = TRUE;
		break;
	}
	return done;
}



static TIMER_CALLBACK( dma8237_timerproc )
{
	int which = param / 4;
	int channel = param % 4;
	int done;

	done = dma8237_do_operation(which, channel);

	if (done)
	{
		dma[which].status &= ~(0x10 << channel);
		dma[which].status |=  (0x01 << channel);
		dma[which].drq    &= ~(0x01 << channel);
		dma8237_update_status(which);
	}
}



static TIMER_CALLBACK( dma8237_msbflip_timerproc )
{
	dma[param].msb ^= 1;
}



static void dma8237_update_status(int which)
{
	UINT16 pending_transfer;
	int channel;
	UINT32 new_eop;

	if ((dma[which].status & 0xF0) == 0)
	{
		/* no transfer is active right now; is there a transfer pending right now? */
		pending_transfer = dma[which].drq & ~dma[which].mask;

		if (pending_transfer)
		{
			/* we do have a transfer in progress */
			for (channel = 3; (pending_transfer & (1 << channel)) == 0; channel--)
				;

			dma[which].status |= 0x10 << channel;
			dma[which].status &= ~(0x01 << channel);

			timer_adjust_periodic(dma[which].timer,
				attotime_zero,
				which * 4 + channel,
				double_to_attotime(dma[which].intf->bus_speed));
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

		/* set the eop line, if it has changed */
		new_eop = (dma[which].status & 0x0F) == 0x0F ? 1 : 0;
		if (dma[which].eop != new_eop)
		{
			dma[which].eop = new_eop;
			if (dma[which].intf->out_eop_func)
				dma[which].intf->out_eop_func(new_eop ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}



/* ----------------------------------------------------------------------- */

static void dma8237_verify(int which)
{
}



static void prepare_msb_flip(int which)
{
	timer_adjust_oneshot(dma[which].msbflip_timer, attotime_zero, which);
}



static UINT8 dma8237_read(int which, offs_t offset)
{
	UINT8 data = 0xFF;
	UINT8 mode;

	dma8237_verify(which);
	offset &= 0x0F;

	switch(offset) {
	case 0:
	case 2:
	case 4:
	case 6:
		/* DMA address register */
		data = dma[which].chan[offset / 2].address >> (dma[which].msb ? 8 : 0);
		prepare_msb_flip(which);

		/* hack simulating refresh activity for 'ibmxt' BIOS; I do not know
         * why this is needed; but in any case, the ibmxt driver does not load
         * if this code is not present */
		mode = dma[which].chan[0].mode;
		if ((DMA_MODE_OPERATION(mode) == 2) && (offset == 0))
		{
			dma[which].chan[0].address++;
			dma[which].chan[0].count--;
		}
		break;

	case 1:
	case 3:
	case 5:
	case 7:
		/* DMA count register */
		data = dma[which].chan[offset / 2].count >> (dma[which].msb ? 8 : 0);
		prepare_msb_flip(which);
		break;

	case 8:
		/* DMA status register */
		data = (UINT8) dma[which].status;
		break;

	case 10:
		/* DMA mask register */
		data = dma[which].mask;
		break;

	case 13:
		/* DMA master clear */
		data = dma[which].temp;
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



static void dma8237_write(int which, offs_t offset, UINT8 data)
{
	int channel;

	dma8237_verify(which);
	offset &= 0x0F;

	switch(offset) {
	case 0:
	case 2:
	case 4:
	case 6:
		/* DMA address register */
		if (dma[which].msb)
			dma[which].chan[offset / 2].address |= ((UINT16) data) << 8;
		else
			dma[which].chan[offset / 2].address = data;
		prepare_msb_flip(which);
		break;

	case 1:
	case 3:
	case 5:
	case 7:
		/* DMA count register */
		if (dma[which].msb)
			dma[which].chan[offset / 2].count |= ((UINT16) data) << 8;
		else
			dma[which].chan[offset / 2].count = data;
		prepare_msb_flip(which);
		break;

	case 8:
		/* DMA command register */
		dma[which].command = data;
		break;

	case 10:
		/* DMA mask register */
		channel = DMA_MODE_CHANNEL(data);
		if (data & 0x04)
			dma[which].mask |= 0x11 << channel;
		else
			dma[which].mask &= ~(0x11 << channel);
		break;

	case 11:
		/* DMA mode register */
		channel = DMA_MODE_CHANNEL(data);
		dma[which].chan[channel].mode = data;
		break;

	case 12:
		/* DMA clear byte pointer flip-flop */
		dma[which].temp = data;
		dma[which].msb = 0;
		break;

	case 13:
		/* DMA master clear */
		dma[which].msb = 0;
		break;

	case 14:
		/* DMA clear mask register */
		dma[which].mask &= ~data;
		dma8237_update_status(which);
		break;

	case 15:
		/* DMA write mask register */
		dma[which].mask |= data;
		break;
	}
}



static void dma8237_drq_write_callback(int param)
{
	int which = param >> 3;
	int channel = (param >> 1) & 0x03;
	int state = param & 0x01;

	/* normalize state */
	if (state)
		dma[which].drq |= 0x01 << channel;
	else
		dma[which].drq &= ~(0x01 << channel);

	dma8237_update_status(which);
}



void dma8237_drq_write(int which, int channel, int state)
{
	int param;

	param = (which << 3) | (channel << 1) | (state ? 1 : 0);
	//timer_call_after_resynch(NULL, param, dma8237_drq_write_callback);
	dma8237_drq_write_callback(param);
}



/******************* Unfortunate hacks *******************/

void dma8237_run_transfer(int which, int channel)
{
	dma[which].status |= 0x10 << channel;	/* reset DMA running flag */

	while(!dma8237_do_operation(which, channel))
		;

	dma[which].status &= ~(0x10 << channel);
	dma[which].status |=  (0x01 << channel);
}



/******************* Standard 8-bit/32-bit/64-bit CPU interfaces *******************/

READ8_HANDLER( dma8237_0_r )	{ return dma8237_read(0, offset); }
READ8_HANDLER( dma8237_1_r )	{ return dma8237_read(1, offset); }
WRITE8_HANDLER( dma8237_0_w ) { dma8237_write(0, offset, data); }
WRITE8_HANDLER( dma8237_1_w ) { dma8237_write(1, offset, data); }

READ16_HANDLER( dma8237_16le_0_r ) { return read16le_with_read8_handler(dma8237_0_r, machine, offset, mem_mask); }
READ16_HANDLER( dma8237_16le_1_r ) { return read16le_with_read8_handler(dma8237_1_r, machine, offset, mem_mask); }
WRITE16_HANDLER( dma8237_16le_0_w ) { write16le_with_write8_handler(dma8237_0_w, machine, offset, data, mem_mask); }
WRITE16_HANDLER( dma8237_16le_1_w ) { write16le_with_write8_handler(dma8237_1_w, machine, offset, data, mem_mask); }

READ32_HANDLER( dma8237_32le_0_r ) { return read32le_with_read8_handler(dma8237_0_r, machine, offset, mem_mask); }
READ32_HANDLER( dma8237_32le_1_r ) { return read32le_with_read8_handler(dma8237_1_r, machine, offset, mem_mask); }
WRITE32_HANDLER( dma8237_32le_0_w ) { write32le_with_write8_handler(dma8237_0_w, machine, offset, data, mem_mask); }
WRITE32_HANDLER( dma8237_32le_1_w ) { write32le_with_write8_handler(dma8237_1_w, machine, offset, data, mem_mask); }

READ64_HANDLER( dma8237_64be_0_r ) { return read64be_with_read8_handler(dma8237_0_r, machine, offset, mem_mask); }
READ64_HANDLER( dma8237_64be_1_r ) { return read64be_with_read8_handler(dma8237_1_r, machine, offset, mem_mask); }
WRITE64_HANDLER( dma8237_64be_0_w ) { write64be_with_write8_handler(dma8237_0_w, machine, offset, data, mem_mask); }
WRITE64_HANDLER( dma8237_64be_1_w ) { write64be_with_write8_handler(dma8237_1_w, machine, offset, data, mem_mask); }
