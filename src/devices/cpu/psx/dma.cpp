// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation DMA emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#include "emu.h"
#include "dma.h"

#define LOG_READ (1U << 1)
#define LOG_WRITE (1U << 2)
#define LOG_DMA (1U << 3)
#define LOG_IRQ (1U << 4)

//#define VERBOSE (LOG_READ | LOG_WRITE | LOG_DMA | LOG_IRQ)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGREAD(...)  LOGMASKED(LOG_READ, __VA_ARGS__)
#define LOGWRITE(...) LOGMASKED(LOG_WRITE, __VA_ARGS__)
#define LOGDMA(...)   LOGMASKED(LOG_DMA, __VA_ARGS__)
#define LOGIRQ(...)   LOGMASKED(LOG_IRQ, __VA_ARGS__)

psxdma_device::psxdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PSX_DMA, tag, owner, clock), m_ram(), m_ramsize(0), m_dpcp(0), m_dicr(0),
	m_irq_handler(*this)
{
}

void psxdma_device::device_reset()
{
	m_dpcp = 0;
	m_dicr = 0;

	for (int n = 0; n < 7; n++)
	{
		dma_stop_timer(n);
		m_channel[n].n_channelcontrol = 0;
	}
}

void psxdma_device::device_start()
{
	for (int index = 0; index < 7; index++)
	{
		psx_dma_channel *dma = &m_channel[index];

		dma->timer = timer_alloc(FUNC(psxdma_device::dma_finished), this);
		dma->b_running = 0;

		save_item(NAME(dma->n_base), index);
		save_item(NAME(dma->n_blockcontrol), index);
		save_item(NAME(dma->n_channelcontrol), index);
		save_item(NAME(dma->n_ticks), index);
		save_item(NAME(dma->b_running), index);
	}

	save_item(NAME(m_dpcp));
	save_item(NAME(m_dicr));
}

void psxdma_device::dma_start_timer(int index, uint32_t n_ticks)
{
	psx_dma_channel *dma = &m_channel[index];

	dma->timer->adjust(attotime::from_hz(33868800) * n_ticks, index);
	dma->n_ticks = n_ticks;
	dma->b_running = 1;
}

void psxdma_device::dma_stop_timer(int index)
{
	psx_dma_channel *dma = &m_channel[index];

	dma->timer->adjust(attotime::never);
	dma->b_running = 0;
}

void psxdma_device::dma_timer_adjust(int index)
{
	psx_dma_channel *dma = &m_channel[index];

	if (dma->b_running)
		dma_start_timer(index, dma->n_ticks);
	else
		dma_stop_timer(index);
}

void psxdma_device::dma_interrupt_update()
{
	int n_int;
	int n_mask;

	n_int = (m_dicr >> 24) & 0x7f;
	n_mask = (m_dicr >> 16) & 0xff;

	if ((n_mask & 0x80) != 0 && (n_int & n_mask) != 0)
	{
		LOGIRQ("dma_interrupt_update( %02x, %02x ) interrupt triggered\n", machine().describe_context(), n_int, n_mask);
		m_dicr |= 0x80000000;
		m_irq_handler(1);
	}
	else if (n_int != 0)
	{
		LOGIRQ("dma_interrupt_update( %02x, %02x ) interrupt not enabled\n", machine().describe_context(), n_int, n_mask);
	}
	m_dicr &= 0x00ffffff | (m_dicr << 8);
}

TIMER_CALLBACK_MEMBER(psxdma_device::dma_finished)
{
	int const index = param;
	psx_dma_channel *dma = &m_channel[index];

	if (dma->n_channelcontrol == 0x01000201)
	{
		int32_t n_size;
		uint32_t n_address;
		uint32_t n_adrmask;

		n_adrmask = m_ramsize - 1;
		n_address = (dma->n_base & n_adrmask);
		n_size = dma->n_blockcontrol;
		if ((dma->n_channelcontrol & 0x200) != 0)
		{
			uint32_t n_ba;
			n_ba = dma->n_blockcontrol >> 16;
			if (n_ba == 0)
			{
				n_ba = 0x10000;
			}
			n_size = (n_size & 0xffff) * n_ba;
		}

		LOGDMA("%s dma %d write block %08x %08x\n", machine().describe_context(), index, n_address, n_size);
		dma->fn_write(m_ram, n_address, n_size);
	}
	else if (dma->n_channelcontrol == 0x01000401 && index == 2)
	{
		uint32_t n_size;
		uint32_t n_total;
		uint32_t n_address = (dma->n_base & 0xffffff);
		uint32_t n_adrmask = m_ramsize - 1;
		uint32_t n_nextaddress;

		if (n_address != 0xffffff)
		{
			n_total = 0;
			for (;; )
			{
				if (n_address == 0xffffff)
				{
					dma->n_base = n_address;
					//HACK: fixes pse bios 2.x & other texture uploading issues, breaks kdeadeye test mode, gtrfrk7m & gtrkfrk8m loading
					//dma_start_timer( index, 19000 );
					dma_start_timer(index, 500);
					return;
				}
				if (n_total > 65535)
				{
					dma->n_base = n_address;
					//FIXME:
					// 16000 below is based on try and error.
					// Mametesters.org: sfex20103red
					//dma_start_timer( index, 16 );
					dma_start_timer(index, 16000);
					return;
				}
				n_address &= n_adrmask;
				n_nextaddress = m_ram[n_address / 4];
				n_size = n_nextaddress >> 24;
				dma->fn_write(m_ram, n_address + 4, n_size);
				//FIXME:
				// The following conditions will cause an endless loop.
				// If stopping the transfer is correct I cannot judge
				// The patch is meant as a hint for somebody who knows
				// the hardware.
				// Mametesters.org: psyforce0105u5red, raystorm0111u1red
				if ((n_nextaddress & 0xffffff) != 0xffffff)
				{
					if (n_address == m_ram[(n_nextaddress & n_adrmask) / 4] ||
						n_address == (n_nextaddress & n_adrmask))
					{
						break;
					}
				}
				n_address = (n_nextaddress & 0xffffff);

				n_total += (n_size + 1);
			}
		}
	}

	dma->n_channelcontrol &= ~((1L << 0x18) | (1L << 0x1c));

	m_dicr |= 1 << (24 + index);
	dma_interrupt_update();
	dma_stop_timer(index);
}

void psxdma_device::install_read_handler(int index, read_delegate p_fn_dma_read)
{
	m_channel[index].fn_read = p_fn_dma_read;
}

void psxdma_device::install_write_handler(int index, write_delegate p_fn_dma_write)
{
	m_channel[index].fn_write = p_fn_dma_write;
}

void psxdma_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	int index = offset / 4;
	psx_dma_channel *dma = &m_channel[index];

	if (index < 7)
	{
		switch (offset % 4)
		{
		case 0:
			LOGWRITE("%s dmabase( %d ) = %08x\n", machine().describe_context(), index, data);
			dma->n_base = data;
			break;

		case 1:
			LOGWRITE("%s dmablockcontrol( %d ) = %08x\n", machine().describe_context(), index, data);
			dma->n_blockcontrol = data;
			break;

		case 2:
		case 3:
			LOGWRITE("%s dmachannelcontrol( %d ) = %08x\n", machine().describe_context(), index, data);
			dma->n_channelcontrol = data;

			if ((dma->n_channelcontrol & (1L << 0x18)) != 0 && (m_dpcp & (1 << (3 + (index * 4)))) != 0)
			{
				int32_t n_size;
				uint32_t n_address;
				uint32_t n_nextaddress;
				uint32_t n_adrmask;

				n_adrmask = m_ramsize - 1;

				n_address = (dma->n_base & n_adrmask);
				n_size = dma->n_blockcontrol;
				if ((dma->n_channelcontrol & 0x200) != 0)
				{
					uint32_t n_ba;
					n_ba = dma->n_blockcontrol >> 16;
					if (n_ba == 0)
						n_ba = 0x10000;
					n_size = (n_size & 0xffff) * n_ba;
				}

				if (dma->n_channelcontrol == 0x01000000 &&
					!dma->fn_read.isnull())
				{
					LOGDMA("%s dma %d read block %08x %08x\n", machine().describe_context(), index, n_address, n_size);
					dma->fn_read(m_ram, n_address, n_size);
					dma_finished(index);
				}
				else if ((dma->n_channelcontrol & 0xffbffeff) == 0x11000000 && // CD DMA
					!dma->fn_read.isnull())
				{
					LOGDMA("%s dma %d read block %08x %08x\n", machine().describe_context(), index, n_address, n_size);

					// pSX's CD DMA size calc formula
					int oursize = (dma->n_blockcontrol >> 16);
					oursize = (oursize > 1) ? oursize : 1;
					oursize *= (dma->n_blockcontrol & 0xffff);

					dma->fn_read(m_ram, n_address, oursize);
					dma_finished(index);
				}
				else if (dma->n_channelcontrol == 0x01000200 &&
					!dma->fn_read.isnull())
				{
					LOGDMA("%s dma %d read block %08x %08x\n", machine().describe_context(), index, n_address, n_size);
					dma->fn_read(m_ram, n_address, n_size);
					if (index == 1)
						dma_start_timer(index, 26000);
					else
						dma_finished(index);
				}
				else if (dma->n_channelcontrol == 0x01000201 &&
					!dma->fn_write.isnull())
				{
					if (index == 4)
						dma_start_timer(index, 24000);
					else
						dma_finished(index);
				}
				else if (dma->n_channelcontrol == 0x11050100 &&
					!dma->fn_write.isnull())
				{
					/* todo: check this is a write not a read... */
					LOGDMA("%s dma %d write block %08x %08x\n", machine().describe_context(), index, n_address, n_size);
					dma->fn_write(m_ram, n_address, n_size);
					dma_finished(index);
				}
				else if (dma->n_channelcontrol == 0x11150100 &&
					!dma->fn_write.isnull())
				{
					/* todo: check this is a write not a read... */
					LOGDMA("%s dma %d write block %08x %08x\n", machine().describe_context(), index, n_address, n_size);
					dma->fn_write(m_ram, n_address, n_size);
					dma_finished(index);
				}
				else if (dma->n_channelcontrol == 0x01000401 &&
					index == 2 &&
					!dma->fn_write.isnull())
				{
					LOGDMA("%s dma %d write linked list %08x\n", machine().describe_context(), index, dma->n_base);

					dma_finished(index);
				}
				else if (dma->n_channelcontrol == 0x11000002 &&
					index == 6)
				{
					LOGDMA("%s dma 6 reverse clear %08x %08x\n", machine().describe_context(), dma->n_base, dma->n_blockcontrol);
					if (n_size > 0)
					{
						n_size--;
						while (n_size > 0)
						{
							n_nextaddress = (n_address - 4) & 0xffffff;
							m_ram[n_address / 4] = n_nextaddress;
							n_address = n_nextaddress;
							n_size--;
						}
						m_ram[n_address / 4] = 0xffffff;
					}
					dma_start_timer(index, 2150);
				}
				else
					LOGDMA("%s dma %d unknown mode %08x\n", machine().describe_context(), index, dma->n_channelcontrol);
			}
			else if (dma->n_channelcontrol != 0)
				LOGDMA("%s psx_dma_w( %04x, %08x, %08x ) channel not enabled\n", machine().describe_context(), offset, dma->n_channelcontrol, mem_mask);
			break;
		}
	}
	else
	{
		switch (offset % 4)
		{
		case 0x0:
			LOGWRITE("%s dpcr_w(0x%08x)\n", machine().describe_context(), data);
			m_dpcp = data;
			break;
		case 0x1:
			{
				uint32_t dicr = (m_dicr & 0x80000000) |
					(m_dicr & ~data & 0x7f000000) |
					(data & 0xff803f);

				LOGWRITE("%s dicr_w(0x%08x) 0x%08x -> 0x%08x\n", machine().describe_context(), data, m_dicr, dicr);
				m_dicr = dicr;
			}

			if ((m_dicr & 0x80000000) != 0 && (m_dicr & 0x7f000000) == 0)
			{
				LOGIRQ("%s dma interrupt cleared\n", machine().describe_context());
				m_dicr &= ~0x80000000;
				m_irq_handler(0);
			}
			break;
		}
	}
}

uint32_t psxdma_device::read(offs_t offset, uint32_t mem_mask)
{
	int index = offset / 4;
	psx_dma_channel *dma = &m_channel[index];

	if (index < 7)
	{
		switch (offset % 4)
		{
		case 0:
			if (!machine().side_effects_disabled())
				LOGREAD("%s psx_dma_r dmabase[ %d ] ( %08x )\n", machine().describe_context(), index, dma->n_base);
			return dma->n_base;

		case 1:
			if (!machine().side_effects_disabled())
				LOGREAD("%s psx_dma_r dmablockcontrol[ %d ] ( %08x )\n", machine().describe_context(), index, dma->n_blockcontrol);
			return dma->n_blockcontrol;

		case 2:
		case 3:
			if (!machine().side_effects_disabled())
				LOGREAD("%s psx_dma_r dmachannelcontrol[ %d ] ( %08x )\n", machine().describe_context(), index, dma->n_channelcontrol);
			return dma->n_channelcontrol;
		}
	}
	else
	{
		switch (offset % 4)
		{
		case 0x0:
			if (!machine().side_effects_disabled())
				LOGREAD("%s dpcr_r() 0x%08x\n", machine().describe_context(), m_dpcp);
			return m_dpcp;

		case 0x1:
			if (!machine().side_effects_disabled())
				LOGREAD("%s dicr_r() 0x%08x\n", machine().describe_context(), m_dicr);
			return m_dicr;
		}
	}

	return 0;
}

DEFINE_DEVICE_TYPE(PSX_DMA, psxdma_device, "psxdma", "Sony PSX DMA")
