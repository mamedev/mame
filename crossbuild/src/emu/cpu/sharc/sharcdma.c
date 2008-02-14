/* SHARC DMA operations */

#define DMA_PMODE_NO_PACKING		0
#define DMA_PMODE_16_32				1
#define DMA_PMODE_16_48				2
#define DMA_PMODE_32_48				3
#define DMA_PMODE_8_48				4

static void schedule_chained_dma_op(int channel, UINT32 dma_chain_ptr, int chained_direction)
{
	UINT32 op_ptr = 0x20000 + dma_chain_ptr;

	UINT32 int_index 		= dm_read32(op_ptr - 0);
	UINT32 int_modifier		= dm_read32(op_ptr - 1);
	UINT32 int_count		= dm_read32(op_ptr - 2);
	UINT32 chain_ptr 		= dm_read32(op_ptr - 3);
	//UINT32 gen_purpose        = dm_read32(op_ptr - 4);
	UINT32 ext_index 		= dm_read32(op_ptr - 5);
	UINT32 ext_modifier 	= dm_read32(op_ptr - 6);
	UINT32 ext_count 		= dm_read32(op_ptr - 7);

	if (sharc.dmaop_cycles > 0)
	{
		fatalerror("schedule_chained_dma_op: DMA operation already scheduled at %08X!", sharc.pc);
	}

	if (chained_direction)		// Transmit to external
	{
		sharc.dmaop_dst 			= ext_index;
		sharc.dmaop_dst_modifier	= ext_modifier;
		sharc.dmaop_dst_count		= ext_count;
		sharc.dmaop_src				= int_index;
		sharc.dmaop_src_modifier	= int_modifier;
		sharc.dmaop_src_count		= int_count;
	}
	else			// Receive from external
	{
		sharc.dmaop_src 			= ext_index;
		sharc.dmaop_src_modifier	= ext_modifier;
		sharc.dmaop_src_count		= ext_count;
		sharc.dmaop_dst				= int_index;
		sharc.dmaop_dst_modifier	= int_modifier;
		sharc.dmaop_dst_count		= int_count;
	}

	sharc.dmaop_pmode = 0;
	sharc.dmaop_channel = channel;
	sharc.dmaop_cycles = sharc.dmaop_src_count / 4;
	sharc.dmaop_chain_ptr = chain_ptr;
	sharc.dmaop_chained_direction = chained_direction;
}

static void schedule_dma_op(int channel, UINT32 src, UINT32 dst, int src_modifier, int dst_modifier, int src_count, int dst_count, int pmode)
{
	if (sharc.dmaop_cycles > 0)
	{
		fatalerror("schedule_dma_op: DMA operation already scheduled at %08X!", sharc.pc);
	}

	sharc.dmaop_channel = channel;
	sharc.dmaop_src = src;
	sharc.dmaop_dst = dst;
	sharc.dmaop_src_modifier = src_modifier;
	sharc.dmaop_dst_modifier = dst_modifier;
	sharc.dmaop_src_count = src_count;
	sharc.dmaop_dst_count = dst_count;
	sharc.dmaop_pmode = pmode;
	sharc.dmaop_chain_ptr = 0;
	sharc.dmaop_cycles = src_count / 4;
}

static void dma_op(UINT32 src, UINT32 dst, int src_modifier, int dst_modifier, int src_count, int dst_count, int pmode)
{
	int i;
	//printf("dma_op: %08X, %08X, %08X, %08X, %08X, %08X, %d\n", src, dst, src_modifier, dst_modifier, src_count, dst_count, pmode);

	switch (pmode)
	{
		case DMA_PMODE_NO_PACKING:
		{
			for (i=0; i < src_count; i++)
			{
				UINT32 data = dm_read32(src);
				dm_write32(dst, data);
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
				UINT32 data = ((dm_read32(src+0) & 0xffff) << 16) | (dm_read32(src+1) & 0xffff);

				dm_write32(dst, data);
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
				UINT64 data = ((UINT64)(dm_read32(src+0) & 0xff) <<  0) |
							  ((UINT64)(dm_read32(src+1) & 0xff) <<  8) |
							  ((UINT64)(dm_read32(src+2) & 0xff) << 16) |
							  ((UINT64)(dm_read32(src+3) & 0xff) << 24) |
							  ((UINT64)(dm_read32(src+4) & 0xff) << 32) |
							  ((UINT64)(dm_read32(src+5) & 0xff) << 40);

				pm_write48(dst, data);
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

	if (sharc.dmaop_channel == 6)
	{
		sharc.irptl |= (1 << (sharc.dmaop_channel+10));

		/* DMA interrupt */
		if (sharc.imask & (1 << (sharc.dmaop_channel+10)))
		{
			sharc.irq_active |= 1 << (sharc.dmaop_channel+10);
		}
	}
}

static void sharc_dma_exec(int channel)
{
	UINT32 src, dst;
	UINT32 src_count, dst_count;
	UINT32 src_modifier, dst_modifier;
	int chen, tran, dtype, pmode, mswf, master, ishake, intio, ext, flsh;

	chen = (sharc.dma[channel].control >> 1) & 0x1;
	tran = (sharc.dma[channel].control >> 2) & 0x1;
	dtype = (sharc.dma[channel].control >> 5) & 0x1;
	pmode = (sharc.dma[channel].control >> 6) & 0x3;
	mswf = (sharc.dma[channel].control >> 8) & 0x1;
	master = (sharc.dma[channel].control >> 9) & 0x1;
	ishake = (sharc.dma[channel].control >> 10) & 0x1;
	intio = (sharc.dma[channel].control >> 11) & 0x1;
	ext = (sharc.dma[channel].control >> 12) & 0x1;
	flsh = (sharc.dma[channel].control >> 13) & 0x1;

	if (ishake)
		fatalerror("SHARC: dma_exec: handshake not supported");
	if (intio)
		fatalerror("SHARC: dma_exec: single-word interrupt enable not supported");



	if (chen)		// Chained DMA
	{
		UINT32 dma_chain_ptr = sharc.dma[channel].chain_ptr & 0x1ffff;

		schedule_chained_dma_op(channel, dma_chain_ptr, tran);
	}
	else
	{
		if (tran)		// Transmit to external
		{
			dst 			= sharc.dma[channel].ext_index;
			dst_modifier	= sharc.dma[channel].ext_modifier;
			dst_count		= sharc.dma[channel].ext_count;
			src				= sharc.dma[channel].int_index;
			src_modifier	= sharc.dma[channel].int_modifier;
			src_count		= sharc.dma[channel].int_count;
		}
		else			// Receive from external
		{
			src 			= sharc.dma[channel].ext_index;
			src_modifier	= sharc.dma[channel].ext_modifier;
			src_count		= sharc.dma[channel].ext_count;
			dst				= sharc.dma[channel].int_index;
			dst_modifier	= sharc.dma[channel].int_modifier;
			dst_count		= sharc.dma[channel].int_count;

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

		schedule_dma_op(channel, src, dst, src_modifier, dst_modifier, src_count, dst_count, pmode);
	}
}
