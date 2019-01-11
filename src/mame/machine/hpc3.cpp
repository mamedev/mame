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
	, m_mainram(*this, ":mainram")
	, m_unkpbus0(*this, ":unkpbus0")
{
}

#define VERBOSE_LEVEL ( 0 )

inline void ATTR_PRINTF(3,4) hpc3_device::verboselog(int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror("%s: %s", machine().describe_context(), buf);
	}
}

void hpc3_device::device_start()
{
}

void hpc3_device::device_reset()
{
	m_enetr_nbdp = 0x80000000;
	m_enetr_cbp = 0x80000000;
	m_pbus_dma.m_active = 0;
}

void hpc3_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_DMA:
		do_dma(ptr, param);
		break;
	default:
		assert_always(false, "Unknown id in hpc3_device::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(hpc3_device::do_dma)
{
	timer_set(attotime::never, TIMER_DMA);
#if 0
	if( m_pbus_dma.m_active )
	{
		uint16_t temp16 = ( m_mainram[(m_pbus_dma.m_cur_ptr - 0x08000000)/4] & 0xffff0000 ) >> 16;
		int16_t stemp16 = (int16_t)((temp16 >> 8) | (temp16 << 8));

		m_dac->write_signed16(stemp16);

		m_pbus_dma.m_cur_ptr += 4;

		m_pbus_dma.m_words_left -= 4;
		if( m_pbus_dma.m_words_left == 0 )
		{
			if( m_pbus_dma.m_next_ptr != 0 )
			{
				m_pbus_dma.m_desc_ptr = m_pbus_dma.m_next_ptr;
				m_pbus_dma.m_cur_ptr = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4];
				m_pbus_dma.m_words_left = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4+1];
				m_pbus_dma.m_next_ptr = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4+2];
			}
			else
			{
				m_pbus_dma.m_active = 0;
				return;
			}
		}
		timer_set(attotime::from_hz(44100), TIMER_DMA);
	}
#endif
}

READ32_MEMBER(hpc3_device::hd_enet_r)
{
	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 0, "HPC3 SCSI0DESC Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_scsi0_desc );
		return m_scsi0_desc;
	case 0x1004/4:
		//verboselog((machine, 0, "HPC3 SCSI0DMACTRL Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_scsi0_dma_ctrl );
		return m_scsi0_dma_ctrl;
	case 0x4000/4:
		//verboselog((machine, 2, "HPC3 ENETR CBP Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_enetr_nbdp );
		return m_enetr_cbp;
	case 0x4004/4:
		//verboselog((machine, 2, "HPC3 ENETR NBDP Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_enetr_nbdp );
		return m_enetr_nbdp;
	default:
		//verboselog((machine, 0, "Unknown HPC3 ENET/HDx Read: %08x (%08x)\n", 0x1fb90000 + ( offset << 2 ), mem_mask );
		return 0;
	}
}

WRITE32_MEMBER(hpc3_device::hd_enet_w)
{
	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 2, "HPC3 SCSI0DESC Write: %08x\n", data );
		m_scsi0_desc = data;
		break;
	case 0x1004/4:
		//verboselog((machine, 2, "HPC3 SCSI0DMACTRL Write: %08x\n", data );
		m_scsi0_dma_ctrl = data;
		break;
	case 0x4000/4:
		//verboselog((machine, 2, "HPC3 ENETR CBP Write: %08x\n", data );
		m_enetr_cbp = data;
		break;
	case 0x4004/4:
		//verboselog((machine, 2, "HPC3 ENETR NBDP Write: %08x\n", data );
		m_enetr_nbdp = data;
		break;
	default:
		//verboselog((machine, 0, "Unknown HPC3 ENET/HDx write: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2 ), mem_mask, data );
		break;
	}
}

READ32_MEMBER(hpc3_device::hd0_r)
{
	switch( offset )
	{
	case 0x0000/4:
	case 0x4000/4:
//      //verboselog((machine, 2, "HPC3 HD0 Status Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, nHPC3_hd0_regs[0x17] );
		if (ACCESSING_BITS_0_7)
		{
			return m_wd33c93->read( space, 0 );
		}
		else
		{
			return 0;
		}
	case 0x0004/4:
	case 0x4004/4:
//      //verboselog((machine, 2, "HPC3 HD0 Register Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, nHPC3_hd0_regs[nHPC3_hd0_register] );
		if (ACCESSING_BITS_0_7)
		{
			return m_wd33c93->read( space, 1 );
		}
		else
		{
			return 0;
		}
	default:
		//verboselog((machine, 0, "Unknown HPC3 HD0 Read: %08x (%08x) [%x] PC=%x\n", 0x1fbc0000 + ( offset << 2 ), mem_mask, offset, m_maincpu->pc() );
		return 0;
	}
}

WRITE32_MEMBER(hpc3_device::hd0_w)
{
	switch( offset )
	{
	case 0x0000/4:
	case 0x4000/4:
//      //verboselog((machine, 2, "HPC3 HD0 Register Select Write: %08x\n", data );
		if (ACCESSING_BITS_0_7)
		{
			m_wd33c93->write( space, 0, data & 0x000000ff );
		}
		break;
	case 0x0004/4:
	case 0x4004/4:
//      //verboselog((machine, 2, "HPC3 HD0 Register %d Write: %08x\n", nHPC3_hd0_register, data );
		if (ACCESSING_BITS_0_7)
		{
			m_wd33c93->write( space, 1,  data & 0x000000ff );
		}
		break;
	default:
		//verboselog((machine, 0, "Unknown HPC3 HD0 Write: %08x (%08x): %08x\n", 0x1fbc0000 + ( offset << 2 ), mem_mask, data );
		break;
	}
}


READ32_MEMBER(hpc3_device::pbus4_r)
{
	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 0 Read: (%08x): %08x\n", mem_mask, m_unk0 );
		return m_unk0;
	case 0x000c/4:
		//verboselog((machine, 2, "Interrupt Controller(?) Read: (%08x): %08x\n", mem_mask, m_ic_unk0 );
		return m_ic_unk0;
	case 0x0014/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 1 Read: (%08x): %08x\n", mem_mask, m_unk1 );
		return m_unk1;
	default:
		//verboselog((machine, 0, "Unknown HPC3 PBUS4 Read: %08x (%08x)\n", 0x1fbd9000 + ( offset << 2 ), mem_mask );
		return 0;
	}
}

WRITE32_MEMBER(hpc3_device::pbus4_w)
{
	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 0 Write: %08x (%08x)\n", data, mem_mask );
		m_unk0 = data;
		break;
	case 0x000c/4:
		//verboselog((machine, 2, "Interrupt Controller(?) Write: (%08x): %08x\n", mem_mask, data );
		m_ic_unk0 = data;
		break;
	case 0x0014/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 1 Write: %08x (%08x)\n", data, mem_mask );
		m_unk1 = data;
		break;
	default:
		//verboselog((machine, 0, "Unknown HPC3 PBUS4 Write: %08x (%08x): %08x\n", 0x1fbd9000 + ( offset << 2 ), mem_mask, data );
		break;
	}
}

READ32_MEMBER(hpc3_device::pbusdma_r)
{
	//uint32_t channel = offset / (0x2000/4);
	//verboselog((machine(), 0, "PBUS DMA Channel %d Read: 0x%08x (%08x)\n", channel, 0x1fb80000 + offset*4, mem_mask );
	return 0;
}

WRITE32_MEMBER(hpc3_device::pbusdma_w)
{
	uint32_t channel = offset / (0x2000/4);

	switch( offset & 0x07ff )
	{
	case 0x0000/4:
		//verboselog((machine, 0, "PBUS DMA Channel %d Buffer Pointer Write: 0x%08x\n", channel, data );
		return;
	case 0x0004/4:
		//verboselog((machine, 0, "PBUS DMA Channel %d Descriptor Pointer Write: 0x%08x\n", channel, data );
		if( channel == 1 )
		{
			m_pbus_dma.m_desc_ptr = data;
			m_pbus_dma.m_cur_ptr = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4];
			m_pbus_dma.m_words_left = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4+1];
			m_pbus_dma.m_next_ptr = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4+2];
			//verboselog((machine, 0, "nPBUS_DMA_DescPtr = %08x\n", m_pbus_dma.m_desc_ptr );
			//verboselog((machine, 0, "nPBUS_DMA_CurPtr = %08x\n", m_pbus_dma.m_cur_ptr );
			//verboselog((machine, 0, "nPBUS_DMA_WordsLeft = %08x\n", m_pbus_dma.m_words_left );
			//verboselog((machine, 0, "nPBUS_DMA_NextPtr = %08x\n", m_pbus_dma.m_next_ptr );
		}
		return;
	case 0x1000/4:
		//verboselog((machine, 0, "PBUS DMA Channel %d Control Register Write: 0x%08x\n", channel, data );
		if( data & PBUS_CTRL_ENDIAN )
		{
			//verboselog((machine, 0, "    Little Endian\n" );
		}
		else
		{
			//verboselog((machine, 0, "    Big Endian\n" );
		}
		if( data & PBUS_CTRL_RECV )
		{
			//verboselog((machine, 0, "    RX DMA\n" );
		}
		else
		{
			//verboselog((machine, 0, "    TX DMA\n" );
		}
		if( data & PBUS_CTRL_FLUSH )
		{
			//verboselog((machine, 0, "    Flush for RX\n" );
		}
		if( data & PBUS_CTRL_DMASTART )
		{
			//verboselog((machine, 0, "    Start DMA\n" );
		}
		if( data & PBUS_CTRL_LOAD_EN )
		{
			//verboselog((machine, 0, "    Load Enable\n" );
		}
		//verboselog((machine, 0, "    High Water Mark: %04x bytes\n", ( data & PBUS_CTRL_HIGHWATER ) >> 8 );
		//verboselog((machine, 0, "    FIFO Begin: Row %04x\n", ( data & PBUS_CTRL_FIFO_BEG ) >> 16 );
		//verboselog((machine, 0, "    FIFO End: Rowe %04x\n", ( data & PBUS_CTRL_FIFO_END ) >> 24 );
		if( ( data & PBUS_CTRL_DMASTART ) || ( data & PBUS_CTRL_LOAD_EN ) )
		{
			timer_set(attotime::from_hz(44100), TIMER_DMA);
			m_pbus_dma.m_active = 1;
		}
		return;
	}
	//verboselog((machine, 0, "Unknown PBUS DMA Channel %d Write: 0x%08x: 0x%08x (%08x)\n", channel, 0x1fb80000 + offset*4, data, mem_mask );
}

READ32_MEMBER(hpc3_device::unkpbus0_r)
{
	return 0;
	////verboselog((machine(), 0, "Unknown PBUS Read: 0x%08x (%08x)\n", 0x1fbc8000 + offset*4, mem_mask );
	//return m_unkpbus0[offset];
}

WRITE32_MEMBER(hpc3_device::unkpbus0_w)
{
	////verboselog((machine(), 0, "Unknown PBUS Write: 0x%08x = 0x%08x (%08x)\n", 0x1fbc8000 + offset*4, data, mem_mask );
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

WRITE_LINE_MEMBER(hpc3_device::scsi_irq)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (state)
	{
		if (m_wd33c93->get_dma_count())
		{
			logerror("m_wd33c93->get_dma_count() is %d\n", m_wd33c93->get_dma_count() );
			if (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENABLE)
			{
				if (m_scsi0_dma_ctrl & HPC3_DMACTRL_IRQ) logerror("IP22: Unhandled SCSI DMA IRQ\n");
			}

			// HPC3 DMA: host to device
			if ((m_scsi0_dma_ctrl & HPC3_DMACTRL_ENABLE) && (m_scsi0_dma_ctrl & HPC3_DMACTRL_DIR))
			{
				uint32_t wptr, tmpword;
				int words, dptr, twords;

				words = m_wd33c93->get_dma_count();
				words /= 4;

				wptr = space.read_dword(m_scsi0_desc);
				m_scsi0_desc += words*4;
				dptr = 0;

				logerror("DMA to device: %d words @ %x\n", words, wptr);

				dump_chain(space, m_scsi0_desc);

				if (words <= (512/4))
				{
					// one-shot
					//m_wd33c93->dma_read_data(m_wd33c93->get_dma_count(), m_dma_buffer);

					while (words)
					{
						tmpword = space.read_dword(wptr);

						if (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN)
						{
							m_dma_buffer[dptr+3] = (tmpword>>24)&0xff;
							m_dma_buffer[dptr+2] = (tmpword>>16)&0xff;
							m_dma_buffer[dptr+1] = (tmpword>>8)&0xff;
							m_dma_buffer[dptr] = tmpword&0xff;
						}
						else
						{
							m_dma_buffer[dptr] = (tmpword>>24)&0xff;
							m_dma_buffer[dptr+1] = (tmpword>>16)&0xff;
							m_dma_buffer[dptr+2] = (tmpword>>8)&0xff;
							m_dma_buffer[dptr+3] = tmpword&0xff;
						}

						wptr += 4;
						dptr += 4;
						words--;
					}

					words = m_wd33c93->get_dma_count();
					m_wd33c93->dma_write_data(words, m_dma_buffer);
				}
				else
				{
					while (words)
					{
						//m_wd33c93->dma_read_data(512, m_dma_buffer);
						twords = 512/4;
						m_scsi0_desc += 512;
						dptr = 0;

						while (twords)
						{
							tmpword = space.read_dword(wptr);

							if (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN)
							{
								m_dma_buffer[dptr+3] = (tmpword>>24)&0xff;
								m_dma_buffer[dptr+2] = (tmpword>>16)&0xff;
								m_dma_buffer[dptr+1] = (tmpword>>8)&0xff;
								m_dma_buffer[dptr] = tmpword&0xff;
							}
							else
							{
								m_dma_buffer[dptr] = (tmpword>>24)&0xff;
								m_dma_buffer[dptr+1] = (tmpword>>16)&0xff;
								m_dma_buffer[dptr+2] = (tmpword>>8)&0xff;
								m_dma_buffer[dptr+3] = tmpword&0xff;
							}

							wptr += 4;
							dptr += 4;
							twords--;
						}

						m_wd33c93->dma_write_data(512, m_dma_buffer);

						words -= (512/4);
					}
				}

				// clear DMA on the controller too
				m_wd33c93->clear_dma();
#if 0
				uint32_t dptr, tmpword;
				uint32_t bc = space.read_dword(m_scsi0_desc + 4);
				uint32_t rptr = space.read_dword(m_scsi0_desc);
				int length = bc & 0x3fff;
				int xie = (bc & 0x20000000) ? 1 : 0;
				int eox = (bc & 0x80000000) ? 1 : 0;

				dump_chain(space, m_scsi0_desc);

				logerror("%s DMA to device: length %x xie %d eox %d\n", machine().describe_context().c_str(), length, xie, eox);

				if (length <= 0x4000)
				{
					dptr = 0;
					while (length > 0)
					{
						tmpword = space.read_dword(rptr);
						if (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN)
						{
							m_dma_buffer[dptr+3] = (tmpword>>24)&0xff;
							m_dma_buffer[dptr+2] = (tmpword>>16)&0xff;
							m_dma_buffer[dptr+1] = (tmpword>>8)&0xff;
							m_dma_buffer[dptr] = tmpword&0xff;
						}
						else
						{
							m_dma_buffer[dptr] = (tmpword>>24)&0xff;
							m_dma_buffer[dptr+1] = (tmpword>>16)&0xff;
							m_dma_buffer[dptr+2] = (tmpword>>8)&0xff;
							m_dma_buffer[dptr+3] = tmpword&0xff;
						}

						dptr += 4;
						rptr += 4;
						length -= 4;
					}

					length = space.read_dword(m_scsi0_desc+4) & 0x3fff;
					m_wd33c93->write_data(length, m_dma_buffer);

					// clear DMA on the controller too
					m_wd33c93->clear_dma();
				}
				else
				{
					logerror("IP22: overly large host to device transfer, can't handle!\n");
				}
#endif
			}

			// HPC3 DMA: device to host
			if ((m_scsi0_dma_ctrl & HPC3_DMACTRL_ENABLE) && !(m_scsi0_dma_ctrl & HPC3_DMACTRL_DIR))
			{
				uint32_t wptr, tmpword;
				int words, sptr, twords;

				words = m_wd33c93->get_dma_count();
				words /= 4;

				wptr = space.read_dword(m_scsi0_desc);
				sptr = 0;

//              osd_printf_info("DMA from device: %d words @ %x\n", words, wptr);

				dump_chain(space, m_scsi0_desc);

				if (words <= (1024/4))
				{
					// one-shot
					m_wd33c93->dma_read_data(m_wd33c93->get_dma_count(), m_dma_buffer);

					while (words)
					{
						if (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN)
						{
							tmpword = m_dma_buffer[sptr+3]<<24 | m_dma_buffer[sptr+2]<<16 | m_dma_buffer[sptr+1]<<8 | m_dma_buffer[sptr];
						}
						else
						{
							tmpword = m_dma_buffer[sptr]<<24 | m_dma_buffer[sptr+1]<<16 | m_dma_buffer[sptr+2]<<8 | m_dma_buffer[sptr+3];
						}

						space.write_dword(wptr, tmpword);
						wptr += 4;
						sptr += 4;
						words--;
					}
				}
				else
				{
					while (words)
					{
						m_wd33c93->dma_read_data(512, m_dma_buffer);
						twords = 512/4;
						sptr = 0;

						while (twords)
						{
							if (m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN)
							{
								tmpword = m_dma_buffer[sptr+3]<<24 | m_dma_buffer[sptr+2]<<16 | m_dma_buffer[sptr+1]<<8 | m_dma_buffer[sptr];
							}
							else
							{
								tmpword = m_dma_buffer[sptr]<<24 | m_dma_buffer[sptr+1]<<16 | m_dma_buffer[sptr+2]<<8 | m_dma_buffer[sptr+3];
							}
							space.write_dword(wptr, tmpword);

							wptr += 4;
							sptr += 4;
							twords--;
						}

						words -= (512/4);
					}
				}

				// clear DMA on the controller too
				m_wd33c93->clear_dma();
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
