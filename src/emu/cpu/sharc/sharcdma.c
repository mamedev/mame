/* SHARC DMA operations */

#define DMA_PMODE_NO_PACKING		0
#define DMA_PMODE_16_32				1
#define DMA_PMODE_16_48				2
#define DMA_PMODE_32_48				3
#define DMA_PMODE_8_48				4

static void schedule_chained_dma_op(SHARC_REGS *cpustate, int channel, UINT32 dma_chain_ptr, int chained_direction)
{
	UINT32 op_ptr = 0x20000 + dma_chain_ptr;

	UINT32 int_index		= dm_read32(cpustate, op_ptr - 0);
	UINT32 int_modifier		= dm_read32(cpustate, op_ptr - 1);
	UINT32 int_count		= dm_read32(cpustate, op_ptr - 2);
	UINT32 chain_ptr		= dm_read32(cpustate, op_ptr - 3);
	//UINT32 gen_purpose        = dm_read32(cpustate, op_ptr - 4);
	UINT32 ext_index		= dm_read32(cpustate, op_ptr - 5);
	UINT32 ext_modifier 	= dm_read32(cpustate, op_ptr - 6);
	UINT32 ext_count		= dm_read32(cpustate, op_ptr - 7);

	if (cpustate->dma_op[channel].active)
	{
		fatalerror("schedule_chained_dma_op: DMA operation already scheduled at %08X!\n", cpustate->pc);
	}

	if (chained_direction)		// Transmit to external
	{
		cpustate->dma_op[channel].dst			= ext_index;
		cpustate->dma_op[channel].dst_modifier	= ext_modifier;
		cpustate->dma_op[channel].dst_count		= ext_count;
		cpustate->dma_op[channel].src			= int_index;
		cpustate->dma_op[channel].src_modifier	= int_modifier;
		cpustate->dma_op[channel].src_count		= int_count;
	}
	else						// Receive from external
	{
		cpustate->dma_op[channel].src			= ext_index;
		cpustate->dma_op[channel].src_modifier	= ext_modifier;
		cpustate->dma_op[channel].src_count		= ext_count;
		cpustate->dma_op[channel].dst			= int_index;
		cpustate->dma_op[channel].dst_modifier	= int_modifier;
		cpustate->dma_op[channel].dst_count		= int_count;
	}

	cpustate->dma_op[channel].pmode = 0;
	cpustate->dma_op[channel].chain_ptr = chain_ptr;
	cpustate->dma_op[channel].chained_direction = chained_direction;

	cpustate->dma_op[channel].active = true;

	int cycles = cpustate->dma_op[channel].src_count / 4;
	cpustate->dma_op[channel].timer->adjust(cpustate->device->cycles_to_attotime(cycles), channel);

	// enable busy flag
	cpustate->dma_status |= (1 << channel);
}

static void schedule_dma_op(SHARC_REGS *cpustate, int channel, UINT32 src, UINT32 dst, int src_modifier, int dst_modifier, int src_count, int dst_count, int pmode)
{
	if (cpustate->dma_op[channel].active)
	{
		fatalerror("schedule_dma_op: DMA operation already scheduled at %08X!\n", cpustate->pc);
	}

	cpustate->dma_op[channel].src = src;
	cpustate->dma_op[channel].dst = dst;
	cpustate->dma_op[channel].src_modifier = src_modifier;
	cpustate->dma_op[channel].dst_modifier = dst_modifier;
	cpustate->dma_op[channel].src_count = src_count;
	cpustate->dma_op[channel].dst_count = dst_count;
	cpustate->dma_op[channel].pmode = pmode;
	cpustate->dma_op[channel].chain_ptr = 0;

	cpustate->dma_op[channel].active = true;

	int cycles = src_count / 4;
	cpustate->dma_op[channel].timer->adjust(cpustate->device->cycles_to_attotime(cycles), channel);

	// enable busy flag
	cpustate->dma_status |= (1 << channel);
}

static void dma_op(SHARC_REGS *cpustate, int channel)
{
	int i;
	UINT32 src			= cpustate->dma_op[channel].src;
	UINT32 dst			= cpustate->dma_op[channel].dst;
	int src_modifier	= cpustate->dma_op[channel].src_modifier;
	int dst_modifier	= cpustate->dma_op[channel].dst_modifier;
	int src_count		= cpustate->dma_op[channel].src_count;
	//int dst_count     = cpustate->dma_op[channel].dst_count;
	int pmode			= cpustate->dma_op[channel].pmode;

	//printf("dma_op: %08X, %08X, %08X, %08X, %08X, %08X, %d\n", src, dst, src_modifier, dst_modifier, src_count, dst_count, pmode);

	switch (pmode)
	{
		case DMA_PMODE_NO_PACKING:
		{
			for (i=0; i < src_count; i++)
			{
				UINT32 data = dm_read32(cpustate, src);
				dm_write32(cpustate, dst, data);
				src += src_modifier;
				dst += dst_modifier;
			}
			break;
		}
		case DMA_PMODE_16_32:
		{
			int length = src_count/2;
			for (i=0; i < length; i++)
			{
				UINT32 data = ((dm_read32(cpustate, src+0) & 0xffff) << 16) | (dm_read32(cpustate, src+1) & 0xffff);

				dm_write32(cpustate, dst, data);
				src += src_modifier * 2;
				dst += dst_modifier;
			}
			break;
		}
		case DMA_PMODE_8_48:
		{
			int length = src_count/6;
			for (i=0; i < length; i++)
			{
				UINT64 data = ((UINT64)(dm_read32(cpustate, src+0) & 0xff) <<  0) |
							  ((UINT64)(dm_read32(cpustate, src+1) & 0xff) <<  8) |
							  ((UINT64)(dm_read32(cpustate, src+2) & 0xff) << 16) |
							  ((UINT64)(dm_read32(cpustate, src+3) & 0xff) << 24) |
							  ((UINT64)(dm_read32(cpustate, src+4) & 0xff) << 32) |
							  ((UINT64)(dm_read32(cpustate, src+5) & 0xff) << 40);

				pm_write48(cpustate, dst, data);
				src += src_modifier * 6;
				dst += dst_modifier;
			}
			break;
		}
		default:
		{
			fatalerror("SHARC: dma_op: unimplemented packing mode %d\n", pmode);
		}
	}

	if (channel == 6)
	{
		cpustate->irptl |= (1 << (channel+10));

		/* DMA interrupt */
		if (cpustate->imask & (1 << (channel+10)))
		{
			cpustate->irq_active |= 1 << (channel+10);
		}
	}

	// clear busy flag
	cpustate->dma_status &= ~(1 << channel);

	cpustate->dma_op[channel].active = false;
}

static void sharc_dma_exec(SHARC_REGS *cpustate, int channel)
{
	UINT32 src, dst;
	UINT32 src_count, dst_count;
	UINT32 src_modifier, dst_modifier;
	int chen, tran, dtype, pmode, /*mswf, master,*/ ishake, intio/*, ext, flsh*/;

	chen = (cpustate->dma[channel].control >> 1) & 0x1;
	tran = (cpustate->dma[channel].control >> 2) & 0x1;
	dtype = (cpustate->dma[channel].control >> 5) & 0x1;
	pmode = (cpustate->dma[channel].control >> 6) & 0x3;
	//mswf = (cpustate->dma[channel].control >> 8) & 0x1;
	//master = (cpustate->dma[channel].control >> 9) & 0x1;
	ishake = (cpustate->dma[channel].control >> 10) & 0x1;
	intio = (cpustate->dma[channel].control >> 11) & 0x1;
	//ext = (cpustate->dma[channel].control >> 12) & 0x1;
	//flsh = (cpustate->dma[channel].control >> 13) & 0x1;

	if (ishake)
		fatalerror("SHARC: dma_exec: handshake not supported\n");
	if (intio)
		fatalerror("SHARC: dma_exec: single-word interrupt enable not supported\n");



	if (chen)		// Chained DMA
	{
		UINT32 dma_chain_ptr = cpustate->dma[channel].chain_ptr & 0x1ffff;

		schedule_chained_dma_op(cpustate, channel, dma_chain_ptr, tran);
	}
	else
	{
		if (tran)		// Transmit to external
		{
			dst 			= cpustate->dma[channel].ext_index;
			dst_modifier	= cpustate->dma[channel].ext_modifier;
			dst_count		= cpustate->dma[channel].ext_count;
			src				= cpustate->dma[channel].int_index;
			src_modifier	= cpustate->dma[channel].int_modifier;
			src_count		= cpustate->dma[channel].int_count;
		}
		else			// Receive from external
		{
			src 			= cpustate->dma[channel].ext_index;
			src_modifier	= cpustate->dma[channel].ext_modifier;
			src_count		= cpustate->dma[channel].ext_count;
			dst				= cpustate->dma[channel].int_index;
			dst_modifier	= cpustate->dma[channel].int_modifier;
			dst_count		= cpustate->dma[channel].int_count;

			if (dst < 0x20000)
			{
				dst |= 0x20000;
			}
		}

		if (dtype)
		//if (src_count != dst_count)
		{
			pmode = DMA_PMODE_8_48;
		}

		schedule_dma_op(cpustate, channel, src, dst, src_modifier, dst_modifier, src_count, dst_count, pmode);
	}
}

static TIMER_CALLBACK(sharc_dma_callback)
{
	SHARC_REGS *cpustate = (SHARC_REGS *)ptr;
	int channel = param;

	cpustate->dma_op[channel].timer->adjust(attotime::never, 0);

	cpustate->irptl |= (1 << (channel+10));

	// DMA interrupt
	if (cpustate->imask & (1 << (channel+10)))
	{
		cpustate->irq_active |= 1 << (channel+10);
	}

	dma_op(cpustate, channel);
	if (cpustate->dma_op[channel].chain_ptr != 0)
	{
		schedule_chained_dma_op(cpustate, channel, cpustate->dma_op[channel].chain_ptr, cpustate->dma_op[channel].chained_direction);
	}
}
