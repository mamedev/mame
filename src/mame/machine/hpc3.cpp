// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HPC3 "High-performance Peripheral Controller" emulation

**********************************************************************/

#include "emu.h"
#include "machine/hpc3.h"

DEFINE_DEVICE_TYPE(SGI_HPC3, hpc3_device, "hpc3", "SGI HPC3")

hpc3_device::hpc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HPC3, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_wd33c93(*this, finder_base::DUMMY_TAG)
	, m_ioc2(*this, finder_base::DUMMY_TAG)
	, m_dac(*this, finder_base::DUMMY_TAG)
	, m_mainram(*this, ":mainram")
	, m_unkpbus0(*this, ":unkpbus0")
{
}

#define VERBOSE_LEVEL (0)

inline void ATTR_PRINTF(3,4) hpc3_device::verboselog(int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		logerror("%s: %s", machine().describe_context(), buf);
	}
}

void hpc3_device::device_start()
{
	m_pbus_dma_timer = timer_alloc(TIMER_PBUS_DMA);
	m_pbus_dma_timer->adjust(attotime::never);
}

void hpc3_device::device_reset()
{
	m_enetr_nbdp = 0x80000000;
	m_enetr_cbp = 0x80000000;
	m_pbus_dma.m_active = 0;
	m_pbus_dma_timer->adjust(attotime::never);
}

void hpc3_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_PBUS_DMA:
		do_dma(ptr, param);
		break;
	default:
		assert_always(false, "Unknown id in hpc3_device::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(hpc3_device::do_dma)
{
	m_pbus_dma_timer->adjust(attotime::never);

	if (m_pbus_dma.m_active)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		uint16_t temp16 = space.read_dword(m_pbus_dma.m_cur_ptr) >> 16;
		int16_t stemp16 = (int16_t)((temp16 >> 8) | (temp16 << 8));

		m_dac->write(stemp16);

		m_pbus_dma.m_cur_ptr += 4;
		m_pbus_dma.m_words_left -= 4;

		if (m_pbus_dma.m_words_left == 0)
		{
			if (m_pbus_dma.m_next_ptr != 0)
			{
				m_pbus_dma.m_desc_ptr = m_pbus_dma.m_next_ptr;
				logerror("Next PBUS_DMA_DescPtr = %08x\n", m_pbus_dma.m_desc_ptr); fflush(stdout);
				m_pbus_dma.m_cur_ptr = space.read_dword(m_pbus_dma.m_desc_ptr);
				m_pbus_dma.m_words_left = space.read_dword(m_pbus_dma.m_desc_ptr + 4);
				m_pbus_dma.m_next_ptr = space.read_dword(m_pbus_dma.m_desc_ptr + 8);
				logerror("Next PBUS_DMA_CurPtr = %08x\n", m_pbus_dma.m_cur_ptr); fflush(stdout);
				logerror("Next PBUS_DMA_WordsLeft = %08x\n", m_pbus_dma.m_words_left); fflush(stdout);
				logerror("Next PBUS_DMA_NextPtr = %08x\n", m_pbus_dma.m_next_ptr); fflush(stdout);
			}
			else
			{
				m_pbus_dma.m_active = 0;
				return;
			}
		}
		m_pbus_dma_timer->adjust(attotime::from_hz(44100));
	}
}

READ32_MEMBER(hpc3_device::hd_enet_r)
{
	switch (offset)
	{
	case 0x0004/4:
		//verboselog(machine, 0, "HPC3 SCSI0DESC Read: %08x (%08x): %08x\n", 0x1fb90000 + (offset << 2), mem_mask, m_scsi0_desc);
		return m_scsi0_desc;
	case 0x1004/4:
		//verboselog(machine, 0, "HPC3 SCSI0DMACTRL Read: %08x (%08x): %08x\n", 0x1fb90000 + (offset << 2), mem_mask, m_scsi0_dma_ctrl);
		return m_scsi0_dma_ctrl;
	case 0x4000/4:
		//verboselog(machine, 2, "HPC3 ENETR CBP Read: %08x (%08x): %08x\n", 0x1fb90000 + (offset << 2), mem_mask, m_enetr_nbdp);
		return m_enetr_cbp;
	case 0x4004/4:
		//verboselog(machine, 2, "HPC3 ENETR NBDP Read: %08x (%08x): %08x\n", 0x1fb90000 + (offset << 2), mem_mask, m_enetr_nbdp);
		return m_enetr_nbdp;
	default:
		//verboselog(machine, 0, "Unknown HPC3 ENET/HDx Read: %08x (%08x)\n", 0x1fb90000 + (offset << 2), mem_mask);
		return 0;
	}
}

WRITE32_MEMBER(hpc3_device::hd_enet_w)
{
	switch (offset)
	{
	case 0x0004/4:
		//verboselog(machine, 2, "HPC3 SCSI0DESC Write: %08x\n", data);
		m_scsi0_desc = data;
		break;
	case 0x1004/4:
		//verboselog(machine, 2, "HPC3 SCSI0DMACTRL Write: %08x\n", data);
		m_scsi0_dma_ctrl = data;
		break;
	case 0x4000/4:
		//verboselog(machine, 2, "HPC3 ENETR CBP Write: %08x\n", data);
		m_enetr_cbp = data;
		break;
	case 0x4004/4:
		//verboselog(machine, 2, "HPC3 ENETR NBDP Write: %08x\n", data);
		m_enetr_nbdp = data;
		break;
	default:
		//verboselog(machine, 0, "Unknown HPC3 ENET/HDx write: %08x (%08x): %08x\n", 0x1fb90000 + (offset << 2), mem_mask, data);
		break;
	}
}

READ32_MEMBER(hpc3_device::hd0_r)
{
	switch (offset)
	{
	case 0x0000/4:
	case 0x4000/4:
//      //verboselog(machine, 2, "HPC3 HD0 Status Read: %08x (%08x): %08x\n", 0x1fb90000 + (offset << 2), mem_mask, nHPC3_hd0_regs[0x17]);
		if (ACCESSING_BITS_0_7)
		{
			return m_wd33c93->read(space, 0);
		}
		else
		{
			return 0;
		}
	case 0x0004/4:
	case 0x4004/4:
//      //verboselog(machine, 2, "HPC3 HD0 Register Read: %08x (%08x): %08x\n", 0x1fb90000 + (offset << 2), mem_mask, nHPC3_hd0_regs[nHPC3_hd0_register]);
		if (ACCESSING_BITS_0_7)
		{
			return m_wd33c93->read(space, 1);
		}
		else
		{
			return 0;
		}
	default:
		//verboselog(machine, 0, "Unknown HPC3 HD0 Read: %08x (%08x) [%x] PC=%x\n", 0x1fbc0000 + (offset << 2), mem_mask, offset, m_maincpu->pc());
		return 0;
	}
}

WRITE32_MEMBER(hpc3_device::hd0_w)
{
	switch (offset)
	{
	case 0x0000/4:
	case 0x4000/4:
//      //verboselog(machine, 2, "HPC3 HD0 Register Select Write: %08x\n", data);
		if (ACCESSING_BITS_0_7)
		{
			m_wd33c93->write(space, 0, data & 0x000000ff);
		}
		break;
	case 0x0004/4:
	case 0x4004/4:
//      //verboselog(machine, 2, "HPC3 HD0 Register %d Write: %08x\n", nHPC3_hd0_register, data);
		if (ACCESSING_BITS_0_7)
		{
			m_wd33c93->write(space, 1, data & 0x000000ff);
		}
		break;
	default:
		//verboselog(machine, 0, "Unknown HPC3 HD0 Write: %08x (%08x): %08x\n", 0x1fbc0000 + (offset << 2), mem_mask, data);
		break;
	}
}


READ32_MEMBER(hpc3_device::pbus4_r)
{
	switch (offset)
	{
	case 0x0004/4:
		//verboselog(machine, 2, "HPC3 PBUS4 Unknown 0 Read: (%08x): %08x\n", mem_mask, m_unk0);
		return m_unk0;
	case 0x000c/4:
		//verboselog(machine, 2, "Interrupt Controller(?) Read: (%08x): %08x\n", mem_mask, m_ic_unk0);
		return m_ic_unk0;
	case 0x0014/4:
		//verboselog(machine, 2, "HPC3 PBUS4 Unknown 1 Read: (%08x): %08x\n", mem_mask, m_unk1);
		return m_unk1;
	default:
		//verboselog(machine, 0, "Unknown HPC3 PBUS4 Read: %08x (%08x)\n", 0x1fbd9000 + (offset << 2), mem_mask);
		return 0;
	}
}

WRITE32_MEMBER(hpc3_device::pbus4_w)
{
	switch (offset)
	{
	case 0x0004/4:
		//verboselog(machine, 2, "HPC3 PBUS4 Unknown 0 Write: %08x (%08x)\n", data, mem_mask);
		m_unk0 = data;
		break;
	case 0x000c/4:
		//verboselog(machine, 2, "Interrupt Controller(?) Write: (%08x): %08x\n", mem_mask, data);
		m_ic_unk0 = data;
		break;
	case 0x0014/4:
		//verboselog(machine, 2, "HPC3 PBUS4 Unknown 1 Write: %08x (%08x)\n", data, mem_mask);
		m_unk1 = data;
		break;
	default:
		//verboselog(machine, 0, "Unknown HPC3 PBUS4 Write: %08x (%08x): %08x\n", 0x1fbd9000 + (offset << 2), mem_mask, data);
		break;
	}
}

READ32_MEMBER(hpc3_device::pbusdma_r)
{
	//uint32_t channel = offset / (0x2000/4);
	//verboselog(machine(), 0, "PBUS DMA Channel %d Read: 0x%08x (%08x)\n", channel, 0x1fb80000 + offset*4, mem_mask);
	return 0;
}

WRITE32_MEMBER(hpc3_device::pbusdma_w)
{
	uint32_t channel = offset / (0x2000/4);

	switch (offset & 0x07ff)
	{
	case 0x0000/4:
		//verboselog(machine, 0, "PBUS DMA Channel %d Buffer Pointer Write: 0x%08x\n", channel, data);
		return;
	case 0x0004/4:
		//verboselog(machine, 0, "PBUS DMA Channel %d Descriptor Pointer Write: 0x%08x\n", channel, data);
		if (channel == 1)
		{
			m_pbus_dma.m_desc_ptr = data;
			logerror("PBUS_DMA_DescPtr = %08x\n", m_pbus_dma.m_desc_ptr); fflush(stdout);
			m_pbus_dma.m_cur_ptr = space.read_dword(m_pbus_dma.m_desc_ptr);
			m_pbus_dma.m_words_left = space.read_dword(m_pbus_dma.m_desc_ptr + 4);
			m_pbus_dma.m_next_ptr = space.read_dword(m_pbus_dma.m_desc_ptr + 8);
			logerror("PBUS_DMA_CurPtr = %08x\n", m_pbus_dma.m_cur_ptr); fflush(stdout);
			logerror("PBUS_DMA_WordsLeft = %08x\n", m_pbus_dma.m_words_left); fflush(stdout);
			logerror("PBUS_DMA_NextPtr = %08x\n", m_pbus_dma.m_next_ptr); fflush(stdout);
		}
		return;
	case 0x1000/4:
		logerror("PBUS DMA Channel %d Control Register Write: 0x%08x\n", channel, data);
		if (data & PBUS_CTRL_ENDIAN)
		{
			logerror("    Little Endian\n");
		}
		else
		{
			logerror("    Big Endian\n");
		}
		if (data & PBUS_CTRL_RECV)
		{
			logerror("    RX DMA\n");
		}
		else
		{
			logerror("    TX DMA\n");
		}
		if (data & PBUS_CTRL_FLUSH)
		{
			logerror("    Flush for RX\n");
		}
		if (data & PBUS_CTRL_DMASTART)
		{
			logerror("    Start DMA\n");
		}
		if (data & PBUS_CTRL_LOAD_EN)
		{
			logerror("    Load Enable\n");
		}
		logerror("    High Water Mark: %04x bytes\n", (data & PBUS_CTRL_HIGHWATER) >> 8);
		logerror("    FIFO Begin: Row %04x\n", (data & PBUS_CTRL_FIFO_BEG) >> 16);
		logerror("    FIFO End: Rowe %04x\n", (data & PBUS_CTRL_FIFO_END) >> 24);
		if ((data & PBUS_CTRL_DMASTART) || (data & PBUS_CTRL_LOAD_EN))
		{
			m_pbus_dma_timer->adjust(attotime::from_hz(44100));
			m_pbus_dma.m_active = 1;
		}
		return;
	}
	logerror("Unknown PBUS DMA Channel %d Write: 0x%08x: 0x%08x (%08x)\n", channel, 0x1fb80000 + offset*4, data, mem_mask);
}

READ32_MEMBER(hpc3_device::unkpbus0_r)
{
	return 0;
	//logerror("Unknown PBUS Read: 0x%08x (%08x)\n", 0x1fbc8000 + offset*4, mem_mask);
	//return m_unkpbus0[offset];
}

WRITE32_MEMBER(hpc3_device::unkpbus0_w)
{
	//logerror("Unknown PBUS Write: 0x%08x = 0x%08x (%08x)\n", 0x1fbc8000 + offset*4, data, mem_mask);
	//COMBINE_DATA(&m_unkpbus0[offset]);
}

void hpc3_device::dump_chain(address_space &space, uint32_t ch_base)
{
	logerror("node: %08x %08x %08x (len = %x)\n", space.read_dword(ch_base), space.read_dword(ch_base+4), space.read_dword(ch_base+8), space.read_dword(ch_base+4) & 0x3fff);

	if ((space.read_dword(ch_base+8) != 0) && !(space.read_dword(ch_base+4) & 0x80000000))
	{
		dump_chain(space, space.read_dword(ch_base+8));
	}
}

void hpc3_device::fetch_chain(address_space &space)
{
	m_scsi0_addr = space.read_dword(m_scsi0_desc);
	m_scsi0_flags = space.read_dword(m_scsi0_desc+4);
	m_scsi0_byte_count = m_scsi0_flags & 0x3fff;
	m_scsi0_next_addr = space.read_dword(m_scsi0_desc+8);
	//logerror("Fetching chain from %08x: %08x %08x %08x (length %04x)\n", m_scsi0_desc, m_scsi0_addr, m_scsi0_flags, m_scsi0_next_addr, m_scsi0_byte_count);
}

bool hpc3_device::decrement_chain(address_space &space)
{
	m_scsi0_byte_count--;
	if (m_scsi0_byte_count == 0)
	{
		if (BIT(m_scsi0_flags, 31))
		{
			return false;
		}
		m_scsi0_desc = m_scsi0_next_addr;
		fetch_chain(space);
	}
	return true;
}

WRITE_LINE_MEMBER(hpc3_device::scsi_irq)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (state)
	{
		if (m_wd33c93->get_dma_count())
		{
			//logerror("m_wd33c93->get_dma_count() is %d\n", m_wd33c93->get_dma_count());
			if (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENABLE)
			{
				if (m_scsi0_dma_ctrl & HPC3_DMACTRL_IRQ)
					logerror("IP22: Unhandled SCSI DMA IRQ\n");
			}

			bool big_endian = (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN);
			if (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENABLE)
			{
				if (m_scsi0_dma_ctrl & HPC3_DMACTRL_DIR)
				{
					// HPC3 DMA: host to device
					int byte_count = m_wd33c93->get_dma_count();
					//dump_chain(space, m_scsi0_desc);
					fetch_chain(space);

					//logerror("DMA to device: %d bytes @ %x\n", byte_count, m_scsi0_addr);

					if (byte_count <= 512)
					{
						for (int i = 0; i < byte_count; i++)
						{
							m_dma_buffer[big_endian ? BYTE4_XOR_BE(i) : BYTE4_XOR_LE(i)] = space.read_byte(m_scsi0_addr+i);
							if (!decrement_chain(space))
								break;
						}

						m_wd33c93->dma_write_data(byte_count, m_dma_buffer);
					}
					else
					{
						int dstoffs = 0;
						while (byte_count)
						{
							int sub_count = std::min(512, byte_count);

							for (int i = 0; i < sub_count; i++)
							{
								m_dma_buffer[big_endian ? BYTE4_XOR_BE(dstoffs+i) : BYTE4_XOR_LE(dstoffs+i)] = space.read_byte(m_scsi0_addr);
								m_scsi0_addr++;
								if (!decrement_chain(space))
									break;
							}

							m_wd33c93->dma_write_data(sub_count, m_dma_buffer);

							byte_count -= sub_count;
						}
					}

					// clear DMA on the controller too
					m_wd33c93->clear_dma();
				}
				else
				{
					// HPC3 DMA: device to host
					int byte_count = m_wd33c93->get_dma_count();
					//dump_chain(space, m_scsi0_desc);
					fetch_chain(space);

	//              logerror("DMA from device: %d words @ %x\n", words, dstoffs);

					if (byte_count < 512)
					{
						m_wd33c93->dma_read_data(byte_count, m_dma_buffer);

						for (int i = 0; i < byte_count; i++)
						{
							space.write_byte(big_endian ? BYTE4_XOR_BE(m_scsi0_addr+i) : BYTE4_XOR_LE(m_scsi0_addr+i), m_dma_buffer[i]);
							if (!decrement_chain(space))
								break;
						}
					}
					else
					{
						while (byte_count)
						{
							int sub_count = m_wd33c93->dma_read_data(512, m_dma_buffer);

							for (int i = 0; i < sub_count; i++)
							{
								space.write_byte(big_endian ? BYTE4_XOR_BE(m_scsi0_addr) : BYTE4_XOR_LE(m_scsi0_addr), m_dma_buffer[i]);
								m_scsi0_addr++;
								if (!decrement_chain(space))
									break;
							}

							byte_count -= sub_count;
						}
					}

					// clear DMA on the controller too
					m_wd33c93->clear_dma();
				}
			}
		}

		// clear HPC3 DMA active flag
		m_scsi0_dma_ctrl &= ~HPC3_DMACTRL_ENABLE;

		// set the interrupt
		m_ioc2->raise_local0_irq(ioc2_device::INT3_LOCAL0_SCSI0);
	}
	else
	{
		m_ioc2->lower_local0_irq(ioc2_device::INT3_LOCAL0_SCSI0);
	}
}
