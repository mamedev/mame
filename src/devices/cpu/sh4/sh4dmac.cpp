// license:BSD-3-Clause
// copyright-holders:R. Belmont
/* SHA3/4 DMA Controller */

#include "emu.h"
#include "debugger.h"
#include "sh4.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4dmac.h"

static const int dmasize[8] = { 8, 1, 2, 4, 32, 0, 0, 0 };

static const int sh3_dmasize[4] = { 1, 2, 4, 16 };

TIMER_CALLBACK_MEMBER( sh34_base_device::sh4_dmac_callback )
{
	int channel = param;

	LOG(("SH4 '%s': DMA %d complete\n", tag(), channel));
	m_dma_timer_active[channel] = 0;
	switch (channel)
	{
	case 0:
		m_SH4_DMATCR0 = 0;
		m_SH4_CHCR0 |= CHCR_TE;
		if (m_SH4_CHCR0 & CHCR_IE)
			sh4_exception_request(SH4_INTC_DMTE0);
		break;
	case 1:
		m_SH4_DMATCR1 = 0;
		m_SH4_CHCR1 |= CHCR_TE;
		if (m_SH4_CHCR1 & CHCR_IE)
			sh4_exception_request(SH4_INTC_DMTE1);
		break;
	case 2:
		m_SH4_DMATCR2 = 0;
		m_SH4_CHCR2 |= CHCR_TE;
		if (m_SH4_CHCR2 & CHCR_IE)
			sh4_exception_request(SH4_INTC_DMTE2);
		break;
	case 3:
		m_SH4_DMATCR3 = 0;
		m_SH4_CHCR3 |= CHCR_TE;
		if (m_SH4_CHCR3 & CHCR_IE)
			sh4_exception_request(SH4_INTC_DMTE3);
		break;
	}
}

int sh34_base_device::sh4_dma_transfer(int channel, int timermode, UINT32 chcr, UINT32 *sar, UINT32 *dar, UINT32 *dmatcr)
{
	int incs, incd, size;
	UINT32 src, dst, count;

	incd = (chcr & CHCR_DM) >> 14;
	incs = (chcr & CHCR_SM) >> 12;

	if (m_cpu_type == CPU_TYPE_SH4)
	{
		size = dmasize[(chcr & CHCR_TS) >> 4];
	}
	else
	{
		size = sh3_dmasize[(chcr >> 3) & 3];
	}

	if(incd == 3 || incs == 3)
	{
		logerror("SH4: DMA: bad increment values (%d, %d, %d, %04x)\n", incd, incs, size, chcr);
		return 0;
	}
	src   = *sar;
	dst   = *dar;
	count = *dmatcr;
	if (!count)
		count = 0x1000000;

	LOG(("SH4: DMA %d start %x, %x, %x, %04x, %d, %d, %d\n", channel, src, dst, count, chcr, incs, incd, size));

	if (timermode == 1) // timer actvated after a time based on the number of words to transfer
	{
		m_dma_timer_active[channel] = 1;
		m_dma_timer[channel]->adjust(cycles_to_attotime(2*count+1), channel);
	}
	else if (timermode == 2) // timer activated immediately
	{
		m_dma_timer_active[channel] = 1;
		m_dma_timer[channel]->adjust(attotime::zero, channel);
	}

	src &= AM;
	dst &= AM;

	switch(size)
	{
	case 1: // 8 bit
		for(;count > 0; count --)
		{
			if(incs == 2)
				src --;
			if(incd == 2)
				dst --;
			m_program->write_byte(dst, m_program->read_byte(src));
			if(incs == 1)
				src ++;
			if(incd == 1)
				dst ++;
		}
		break;
	case 2: // 16 bit
		src &= ~1;
		dst &= ~1;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 2;
			if(incd == 2)
				dst -= 2;
			m_program->write_word(dst, m_program->read_word(src));
			if(incs == 1)
				src += 2;
			if(incd == 1)
				dst += 2;
		}
		break;
	case 8: // 64 bit
		src &= ~7;
		dst &= ~7;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 8;
			if(incd == 2)
				dst -= 8;
			m_program->write_qword(dst, m_program->read_qword(src));
			if(incs == 1)
				src += 8;
			if(incd == 1)
				dst += 8;

		}
		break;
	case 4: // 32 bit
		src &= ~3;
		dst &= ~3;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 4;
			if(incd == 2)
				dst -= 4;
			m_program->write_dword(dst, m_program->read_dword(src));
			if(incs == 1)
				src += 4;
			if(incd == 1)
				dst += 4;

		}
		break;
	case 32:
		src &= ~31;
		dst &= ~31;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 32;
			if(incd == 2)
				dst -= 32;
			m_program->write_qword(dst, m_program->read_qword(src));
			m_program->write_qword(dst+8, m_program->read_qword(src+8));
			m_program->write_qword(dst+16, m_program->read_qword(src+16));
			m_program->write_qword(dst+24, m_program->read_qword(src+24));
			if(incs == 1)
				src += 32;
			if(incd == 1)
				dst += 32;
		}
		break;
	}
	*sar    = (*sar & ~AM) | src;
	*dar    = (*dar & ~AM) | dst;
	*dmatcr = count;
	return 1;
}

int sh34_base_device::sh4_dma_transfer_device(int channel, UINT32 chcr, UINT32 *sar, UINT32 *dar, UINT32 *dmatcr)
{
	int incs, incd, size, mod;
	UINT32 src, dst, count;

	incd = (chcr & CHCR_DM) >> 14;
	incs = (chcr & CHCR_SM) >> 12;


	if (m_cpu_type == CPU_TYPE_SH4)
	{
		size = dmasize[(chcr & CHCR_TS) >> 4];
	}
	else
	{
		size = sh3_dmasize[(chcr >> 3) & 3];
	}

	mod = ((chcr & CHCR_RS) >> 8);
	if (incd == 3 || incs == 3)
	{
		logerror("SH4: DMA: bad increment values (%d, %d, %d, %04x)\n", incd, incs, size, chcr);
		return 0;
	}
	src   = *sar;
	dst   = *dar;
	count = *dmatcr;
	if (!count)
		count = 0x1000000;

	LOG(("SH4: DMA %d start device<->memory %x, %x, %x, %04x, %d, %d, %d\n", channel, src, dst, count, chcr, incs, incd, size));

	m_dma_timer_active[channel] = 1;

	src &= AM;
	dst &= AM;

	// remember parameters
	m_dma_source[channel]=src;
	m_dma_destination[channel]=dst;
	m_dma_count[channel]=count;
	m_dma_wordsize[channel]=size;
	m_dma_source_increment[channel]=incs;
	m_dma_destination_increment[channel]=incd;
	m_dma_mode[channel]=mod;

	// inform device its ready to transfer
	m_io->write_dword(SH4_IOPORT_DMA, channel | (mod << 16));
	return 1;
}

void sh34_base_device::sh4_dmac_check(int channel)
{
	UINT32 dmatcr, chcr, sar, dar;

	switch (channel)
	{
	case 0:
		sar = m_SH4_SAR0;
		dar = m_SH4_DAR0;
		chcr = m_SH4_CHCR0;
		dmatcr = m_SH4_DMATCR0;
		break;
	case 1:
		sar = m_SH4_SAR1;
		dar = m_SH4_DAR1;
		chcr = m_SH4_CHCR1;
		dmatcr = m_SH4_DMATCR1;
		break;
	case 2:
		sar = m_SH4_SAR2;
		dar = m_SH4_DAR2;
		chcr = m_SH4_CHCR2;
		dmatcr = m_SH4_DMATCR2;
		break;
	case 3:
		sar = m_SH4_SAR3;
		dar = m_SH4_DAR3;
		chcr = m_SH4_CHCR3;
		dmatcr = m_SH4_DMATCR3;
		break;
	default:
		return;
	}
	if (chcr & m_SH4_DMAOR & DMAOR_DME)
	{
		if ((((chcr & CHCR_RS) >> 8) < 2) || (((chcr & CHCR_RS) >> 8) > 6))
			return;
		if (!m_dma_timer_active[channel] && !(chcr & CHCR_TE) && !(m_SH4_DMAOR & (DMAOR_AE | DMAOR_NMIF)))
		{
			if (((chcr & CHCR_RS) >> 8) > 3)
				sh4_dma_transfer(channel, 1, chcr, &sar, &dar, &dmatcr);
			else if ((m_SH4_DMAOR & DMAOR_DDT) == 0)
				sh4_dma_transfer_device(channel, chcr, &sar, &dar, &dmatcr); // tell device we are ready to transfer
		}
	}
	else
	{
		if (m_dma_timer_active[channel])
		{
			logerror("SH4: DMA %d cancelled in-flight but all data transferred", channel);
			m_dma_timer[channel]->adjust(attotime::never, channel);
			m_dma_timer_active[channel] = 0;
		}
	}
}


// called by drivers to transfer data in a cpu<->device dma. 'device' must be a SH4 cpu
int sh34_base_device::sh4_dma_data(struct sh4_device_dma *s)
{
	UINT32 pos, len, siz;
	int channel = s->channel;
	void *data = s->buffer;

	if (!m_dma_timer_active[channel])
		return 0;

	if (m_dma_mode[channel] == 2)
	{
		// device receives data
		len = m_dma_count[channel];
		if (s->length < len)
			len = s->length;
		siz = m_dma_wordsize[channel];
		for (pos = 0;pos < len;pos++) {
			switch (siz)
			{
			case 8:
				if (m_dma_source_increment[channel] == 2)
					m_dma_source[channel] -= 8;
				*(UINT64 *)data = m_program->read_qword(m_dma_source[channel] & ~7);
				if (m_dma_source_increment[channel] == 1)
					m_dma_source[channel] += 8;
				break;
			case 1:
				if (m_dma_source_increment[channel] == 2)
					m_dma_source[channel]--;
				*(UINT8 *)data = m_program->read_byte(m_dma_source[channel]);
				if (m_dma_source_increment[channel] == 1)
					m_dma_source[channel]++;
				break;
			case 2:
				if (m_dma_source_increment[channel] == 2)
					m_dma_source[channel] -= 2;
				*(UINT16 *)data = m_program->read_word(m_dma_source[channel] & ~1);
				if (m_dma_source_increment[channel] == 1)
					m_dma_source[channel] += 2;
				break;
			case 4:
				if (m_dma_source_increment[channel] == 2)
					m_dma_source[channel] -= 4;
				*(UINT32 *)data = m_program->read_dword(m_dma_source[channel] & ~3);
				if (m_dma_source_increment[channel] == 1)
					m_dma_source[channel] += 4;
				break;
			case 32:
				if (m_dma_source_increment[channel] == 2)
					m_dma_source[channel] -= 32;
				*(UINT64 *)data = m_program->read_qword(m_dma_source[channel] & ~31);
				*((UINT64 *)data+1) = m_program->read_qword((m_dma_source[channel] & ~31)+8);
				*((UINT64 *)data+2) = m_program->read_qword((m_dma_source[channel] & ~31)+16);
				*((UINT64 *)data+3) = m_program->read_qword((m_dma_source[channel] & ~31)+24);
				if (m_dma_source_increment[channel] == 1)
					m_dma_source[channel] += 32;
				break;
			}
			m_dma_count[channel]--;
		}
		if (m_dma_count[channel] == 0) // all data transferred ?
		{
			m_dma_timer[channel]->adjust(attotime::zero, channel);
			return 2;
		}
		return 1;
	}
	else if (m_dma_mode[channel] == 3)
	{
		// device sends data
		len = m_dma_count[channel];
		if (s->length < len)
			len = s->length;
		siz = m_dma_wordsize[channel];
		for (pos = 0;pos < len;pos++) {
			switch (siz)
			{
			case 8:
				if (m_dma_destination_increment[channel] == 2)
					m_dma_destination[channel]-=8;
				m_program->write_qword(m_dma_destination[channel] & ~7, *(UINT64 *)data);
				if (m_dma_destination_increment[channel] == 1)
					m_dma_destination[channel]+=8;
				break;
			case 1:
				if (m_dma_destination_increment[channel] == 2)
					m_dma_destination[channel]--;
				m_program->write_byte(m_dma_destination[channel], *(UINT8 *)data);
				if (m_dma_destination_increment[channel] == 1)
					m_dma_destination[channel]++;
				break;
			case 2:
				if (m_dma_destination_increment[channel] == 2)
					m_dma_destination[channel]-=2;
				m_program->write_word(m_dma_destination[channel] & ~1, *(UINT16 *)data);
				if (m_dma_destination_increment[channel] == 1)
					m_dma_destination[channel]+=2;
				break;
			case 4:
				if (m_dma_destination_increment[channel] == 2)
					m_dma_destination[channel]-=4;
				m_program->write_dword(m_dma_destination[channel] & ~3, *(UINT32 *)data);
				if (m_dma_destination_increment[channel] == 1)
					m_dma_destination[channel]+=4;
				break;
			case 32:
				if (m_dma_destination_increment[channel] == 2)
					m_dma_destination[channel]-=32;
				m_program->write_qword(m_dma_destination[channel] & ~31, *(UINT64 *)data);
				m_program->write_qword((m_dma_destination[channel] & ~31)+8, *((UINT64 *)data+1));
				m_program->write_qword((m_dma_destination[channel] & ~31)+16, *((UINT64 *)data+2));
				m_program->write_qword((m_dma_destination[channel] & ~31)+24, *((UINT64 *)data+3));
				if (m_dma_destination_increment[channel] == 1)
					m_dma_destination[channel]+=32;
				break;
			}
			m_dma_count[channel]--;
		}

		if (m_dma_count[channel] == 0) // all data transferred ?
		{
			m_dma_timer[channel]->adjust(attotime::zero, channel);
			return 2;
		}
		return 1;
	}
	else
		return 0;
}

// called by drivers to transfer data in a DDT dma.
void sh34_base_device::sh4_dma_ddt(struct sh4_ddt_dma *s)
{
	UINT32 chcr;
	UINT32 *p32bits;
	UINT64 *p32bytes;
	UINT32 pos,len,siz;

	if (m_cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_dma_ddt uses m_m[] with SH3\n");

	if (m_dma_timer_active[s->channel])
		return;
	if (s->mode >= 0) {
		switch (s->channel)
		{
		case 0:
			if (s->mode & 1)
				s->source = m_SH4_SAR0;
			if (s->mode & 2)
				m_SH4_SAR0 = s->source;
			if (s->mode & 4)
				s->destination = m_SH4_DAR0;
			if (s->mode & 8)
				m_SH4_DAR0 = s->destination;
			break;
		case 1:
			if (s->mode & 1)
				s->source = m_SH4_SAR1;
			if (s->mode & 2)
				m_SH4_SAR1 = s->source;
			if (s->mode & 4)
				s->destination = m_SH4_DAR1;
			if (s->mode & 8)
				m_SH4_DAR1 = s->destination;
			break;
		case 2:
			if (s->mode & 1)
				s->source = m_SH4_SAR2;
			if (s->mode & 2)
				m_SH4_SAR2 = s->source;
			if (s->mode & 4)
				s->destination = m_SH4_DAR2;
			if (s->mode & 8)
				m_SH4_DAR2 = s->destination;
			break;
		case 3:
		default:
			if (s->mode & 1)
				s->source = m_SH4_SAR3;
			if (s->mode & 2)
				m_SH4_SAR3 = s->source;
			if (s->mode & 4)
				s->destination = m_SH4_DAR3;
			if (s->mode & 8)
				m_SH4_DAR3 = s->destination;
			break;
		}
		switch (s->channel)
		{
		case 0:
			chcr = m_SH4_CHCR0;
			len = m_SH4_DMATCR0;
			break;
		case 1:
			chcr = m_SH4_CHCR1;
			len = m_SH4_DMATCR1;
			break;
		case 2:
			chcr = m_SH4_CHCR2;
			len = m_SH4_DMATCR2;
			break;
		case 3:
		default:
			chcr = m_SH4_CHCR3;
			len = m_SH4_DMATCR3;
			break;
		}
		if ((s->direction) == 0) {
			chcr = (chcr & 0xffff3fff) | ((s->mode & 0x30) << 10);
		} else {
			chcr = (chcr & 0xffffcfff) | ((s->mode & 0x30) << 8);
		}


		if (m_cpu_type == CPU_TYPE_SH4)
		{
			//siz = dmasize[(chcr & CHCR_TS) >> 4];
			siz = dmasize[(chcr >> 4) & 7];
		}
		else
		{
			siz = sh3_dmasize[(chcr >> 3) & 3];
		}


		if (siz && (s->size))
			if ((len * siz) != (s->length * s->size))
				return;
		sh4_dma_transfer(s->channel, 0, chcr, &s->source, &s->destination, &len);
	} else {
		if (s->size == 4) {
			if ((s->direction) == 0) {
				len = s->length;
				p32bits = (UINT32 *)(s->buffer);
				for (pos = 0;pos < len;pos++) {
					*p32bits = m_program->read_dword(s->source);
					p32bits++;
					s->source = s->source + 4;
				}
			} else {
				len = s->length;
				p32bits = (UINT32 *)(s->buffer);
				for (pos = 0;pos < len;pos++) {
					m_program->write_dword(s->destination, *p32bits);
					p32bits++;
					s->destination = s->destination + 4;
				}
			}
		}
		if (s->size == 32) {
			if ((s->direction) == 0) {
				len = s->length * 4;
				p32bytes = (UINT64 *)(s->buffer);
				for (pos = 0;pos < len;pos++) {
					*p32bytes = m_program->read_qword(s->source);
					p32bytes++;
					s->destination = s->destination + 8;
				}
			} else {
				len = s->length * 4;
				p32bytes = (UINT64 *)(s->buffer);
				for (pos = 0;pos < len;pos++) {
					m_program->write_qword(s->destination, *p32bytes);
					p32bytes++;
					s->destination = s->destination + 8;
				}
			}
		}
	}
}


	void sh34_base_device::sh4_handle_sar0_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_SAR0);
}

	void sh34_base_device::sh4_handle_sar1_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_SAR1);
}

	void sh34_base_device::sh4_handle_sar2_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_SAR2);
}

	void sh34_base_device::sh4_handle_sar3_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_SAR3);
}

	void sh34_base_device::sh4_handle_dar0_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_DAR0);
}

	void sh34_base_device::sh4_handle_dar1_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_DAR1);
}

	void sh34_base_device::sh4_handle_dar2_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_DAR2);
}

	void sh34_base_device::sh4_handle_dar3_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_DAR3);
}

	void sh34_base_device::sh4_handle_dmatcr0_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_DMATCR0);
}

	void sh34_base_device::sh4_handle_dmatcr1_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_DMATCR1);
}

	void sh34_base_device::sh4_handle_dmatcr2_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_DMATCR2);
}

	void sh34_base_device::sh4_handle_dmatcr3_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_DMATCR3);
}

	void sh34_base_device::sh4_handle_chcr0_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_CHCR0);
	sh4_dmac_check(0);
}

	void sh34_base_device::sh4_handle_chcr1_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_CHCR1);
	sh4_dmac_check(1);
}

	void sh34_base_device::sh4_handle_chcr2_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_CHCR2);
	sh4_dmac_check(2);
}

	void sh34_base_device::sh4_handle_chcr3_addr_w(UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&m_SH4_CHCR3);
	sh4_dmac_check(3);
}

	void sh34_base_device::sh4_handle_dmaor_addr_w(UINT32 data, UINT32 mem_mask)
{
	UINT32 old = m_SH4_DMAOR;
	COMBINE_DATA(&m_SH4_DMAOR);

	if ((m_SH4_DMAOR & DMAOR_AE) && (~old & DMAOR_AE))
		m_SH4_DMAOR &= ~DMAOR_AE;
	if ((m_SH4_DMAOR & DMAOR_NMIF) && (~old & DMAOR_NMIF))
		m_SH4_DMAOR &= ~DMAOR_NMIF;
	sh4_dmac_check(0);
	sh4_dmac_check(1);
	sh4_dmac_check(2);
	sh4_dmac_check(3);
}
