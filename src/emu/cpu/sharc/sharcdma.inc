// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* SHARC DMA operations */

#define DMA_PMODE_NO_PACKING        0
#define DMA_PMODE_16_32             1
#define DMA_PMODE_16_48             2
#define DMA_PMODE_32_48             3
#define DMA_PMODE_8_48              4

void adsp21062_device::schedule_chained_dma_op(int channel, UINT32 dma_chain_ptr, int chained_direction)
{
	UINT32 op_ptr = 0x20000 + dma_chain_ptr;

	UINT32 int_index        = dm_read32(op_ptr - 0);
	UINT32 int_modifier     = dm_read32(op_ptr - 1);
	UINT32 int_count        = dm_read32(op_ptr - 2);
	UINT32 chain_ptr        = dm_read32(op_ptr - 3);
	//UINT32 gen_purpose        = dm_read32(op_ptr - 4);
	UINT32 ext_index        = dm_read32(op_ptr - 5);
	UINT32 ext_modifier     = dm_read32(op_ptr - 6);
	UINT32 ext_count        = dm_read32(op_ptr - 7);

	if (m_dma_op[channel].active)
	{
		fatalerror("schedule_chained_dma_op: DMA operation already scheduled at %08X!\n", m_pc);
	}

	if (chained_direction)      // Transmit to external
	{
		m_dma_op[channel].dst           = ext_index;
		m_dma_op[channel].dst_modifier  = ext_modifier;
		m_dma_op[channel].dst_count     = ext_count;
		m_dma_op[channel].src           = int_index;
		m_dma_op[channel].src_modifier  = int_modifier;
		m_dma_op[channel].src_count     = int_count;
	}
	else                        // Receive from external
	{
		m_dma_op[channel].src           = ext_index;
		m_dma_op[channel].src_modifier  = ext_modifier;
		m_dma_op[channel].src_count     = ext_count;
		m_dma_op[channel].dst           = int_index;
		m_dma_op[channel].dst_modifier  = int_modifier;
		m_dma_op[channel].dst_count     = int_count;
	}

	m_dma_op[channel].pmode = 0;
	m_dma_op[channel].chain_ptr = chain_ptr;
	m_dma_op[channel].chained_direction = chained_direction;

	m_dma_op[channel].active = true;

	int cycles = m_dma_op[channel].src_count / 4;
	m_dma_op[channel].timer->adjust(cycles_to_attotime(cycles), channel);

	// enable busy flag
	m_dma_status |= (1 << channel);
}

void adsp21062_device::schedule_dma_op(int channel, UINT32 src, UINT32 dst, int src_modifier, int dst_modifier, int src_count, int dst_count, int pmode)
{
	if (m_dma_op[channel].active)
	{
		fatalerror("schedule_dma_op: DMA operation already scheduled at %08X!\n", m_pc);
	}

	m_dma_op[channel].src = src;
	m_dma_op[channel].dst = dst;
	m_dma_op[channel].src_modifier = src_modifier;
	m_dma_op[channel].dst_modifier = dst_modifier;
	m_dma_op[channel].src_count = src_count;
	m_dma_op[channel].dst_count = dst_count;
	m_dma_op[channel].pmode = pmode;
	m_dma_op[channel].chain_ptr = 0;

	m_dma_op[channel].active = true;

	int cycles = src_count / 4;
	m_dma_op[channel].timer->adjust(cycles_to_attotime(cycles), channel);

	// enable busy flag
	m_dma_status |= (1 << channel);
}

void adsp21062_device::dma_op(int channel)
{
	int i;
	UINT32 src          = m_dma_op[channel].src;
	UINT32 dst          = m_dma_op[channel].dst;
	int src_modifier    = m_dma_op[channel].src_modifier;
	int dst_modifier    = m_dma_op[channel].dst_modifier;
	int src_count       = m_dma_op[channel].src_count;
	//int dst_count     = m_dma_op[channel].dst_count;
	int pmode           = m_dma_op[channel].pmode;

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

	if (channel == 6)
	{
		m_irptl |= (1 << (channel+10));

		/* DMA interrupt */
		if (m_imask & (1 << (channel+10)))
		{
			m_irq_active |= 1 << (channel+10);
		}
	}

	// clear busy flag
	m_dma_status &= ~(1 << channel);

	m_dma_op[channel].active = false;
}

void adsp21062_device::sharc_dma_exec(int channel)
{
	UINT32 src, dst;
	UINT32 src_count, dst_count;
	UINT32 src_modifier, dst_modifier;
	int chen, tran, dtype, pmode, /*mswf, master,*/ ishake, intio/*, ext, flsh*/;

	chen = (m_dma[channel].control >> 1) & 0x1;
	tran = (m_dma[channel].control >> 2) & 0x1;
	dtype = (m_dma[channel].control >> 5) & 0x1;
	pmode = (m_dma[channel].control >> 6) & 0x3;
	//mswf = (m_dma[channel].control >> 8) & 0x1;
	//master = (m_dma[channel].control >> 9) & 0x1;
	ishake = (m_dma[channel].control >> 10) & 0x1;
	intio = (m_dma[channel].control >> 11) & 0x1;
	//ext = (m_dma[channel].control >> 12) & 0x1;
	//flsh = (m_dma[channel].control >> 13) & 0x1;

	if (ishake)
		fatalerror("SHARC: dma_exec: handshake not supported\n");
	if (intio)
		fatalerror("SHARC: dma_exec: single-word interrupt enable not supported\n");


	if (chen)       // Chained DMA
	{
		UINT32 dma_chain_ptr = m_dma[channel].chain_ptr & 0x1ffff;

		schedule_chained_dma_op(channel, dma_chain_ptr, tran);
	}
	else
	{
		if (tran)       // Transmit to external
		{
			dst             = m_dma[channel].ext_index;
			dst_modifier    = m_dma[channel].ext_modifier;
			dst_count       = m_dma[channel].ext_count;
			src             = (m_dma[channel].int_index & 0x1ffff) | 0x20000;
			src_modifier    = m_dma[channel].int_modifier;
			src_count       = m_dma[channel].int_count;
		}
		else            // Receive from external
		{
			src             = m_dma[channel].ext_index;
			src_modifier    = m_dma[channel].ext_modifier;
			src_count       = m_dma[channel].ext_count;
			dst             = (m_dma[channel].int_index & 0x1ffff) | 0x20000;
			dst_modifier    = m_dma[channel].int_modifier;
			dst_count       = m_dma[channel].int_count;
		}

		if (dtype)
		//if (src_count != dst_count)
		{
			pmode = DMA_PMODE_8_48;
		}

		schedule_dma_op(channel, src, dst, src_modifier, dst_modifier, src_count, dst_count, pmode);
	}
}

TIMER_CALLBACK_MEMBER(adsp21062_device::sharc_dma_callback)
{
	int channel = param;

	m_dma_op[channel].timer->adjust(attotime::never, 0);

	m_irptl |= (1 << (channel+10));

	// DMA interrupt
	if (m_imask & (1 << (channel+10)))
	{
		m_irq_active |= 1 << (channel+10);
	}

	dma_op(channel);
	if (m_dma_op[channel].chain_ptr != 0)
	{
		schedule_chained_dma_op(channel, m_dma_op[channel].chain_ptr, m_dma_op[channel].chained_direction);
	}
}
