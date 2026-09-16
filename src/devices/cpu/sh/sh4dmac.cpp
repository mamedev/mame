// license:BSD-3-Clause
// copyright-holders:R. Belmont
/* SHA3/4 DMA Controller */

#include "emu.h"
#include "sh4.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4dmac.h"

static const int dmasize[8] = { 8, 1, 2, 4, 32, 0, 0, 0 };

static const int sh3_dmasize[4] = { 1, 2, 4, 16 };

TIMER_CALLBACK_MEMBER( sh34_base_device::sh4_dmac_callback )
{
	int channel = param;

	LOG("SH4 '%s': DMA %d complete\n", tag(), channel);
	m_dma_timer_active[channel] = 0;
	switch (channel)
	{
	case 0:
		m_dmatcr0 = 0;
		m_chcr0 |= CHCR_TE;
		if (m_chcr0 & CHCR_IE)
			sh4_exception_request(SH4_INTC_DMTE0);
		break;
	case 1:
		m_dmatcr1 = 0;
		m_chcr1 |= CHCR_TE;
		if (m_chcr1 & CHCR_IE)
			sh4_exception_request(SH4_INTC_DMTE1);
		break;
	case 2:
		m_dmatcr2 = 0;
		m_chcr2 |= CHCR_TE;
		if (m_chcr2 & CHCR_IE)
			sh4_exception_request(SH4_INTC_DMTE2);
		break;
	case 3:
		m_dmatcr3 = 0;
		m_chcr3 |= CHCR_TE;
		if (m_chcr3 & CHCR_IE)
			sh4_exception_request(SH4_INTC_DMTE3);
		break;
	}
}

int sh34_base_device::sh4_dma_transfer(int channel, int timermode, uint32_t chcr, uint32_t *sar, uint32_t *dar, uint32_t *dmatcr)
{
	int incd = (chcr & CHCR_DM) >> 14;
	int incs = (chcr & CHCR_SM) >> 12;

	int size;
	if (m_cpu_type == CPU_TYPE_SH4)
	{
		size = dmasize[(chcr & CHCR_TS) >> 4];
	}
	else
	{
		size = sh3_dmasize[(chcr >> 3) & 3];
	}

	if (incd == 3 || incs == 3)
	{
		logerror("SH4: DMA: bad increment values (%d, %d, %d, %04x)\n", incd, incs, size, chcr);
		return 0;
	}

	uint32_t src   = *sar;
	uint32_t dst   = *dar;
	uint32_t count = *dmatcr;
	if (!count)
		count = 0x1000000;

	LOG("SH4: DMA %d start %x, %x, %x, %04x, %d, %d, %d\n", channel, src, dst, count, chcr, incs, incd, size);

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

	src &= SH34_AM;
	dst &= SH34_AM;

	switch (size)
	{
	case 1: // 8 bit
		for (;count > 0; count --)
		{
			if (incs == 2)
				src --;
			if (incd == 2)
				dst --;
			m_program->write_byte(dst & SH34_AM, m_program->read_byte(src & SH34_AM));
			if (incs == 1)
				src ++;
			if (incd == 1)
				dst ++;
		}
		break;
	case 2: // 16 bit
		src &= ~1;
		dst &= ~1;
		for (;count > 0; count --)
		{
			if (incs == 2)
				src -= 2;
			if (incd == 2)
				dst -= 2;
			m_program->write_word(dst & SH34_AM, m_program->read_word(src & SH34_AM));
			if (incs == 1)
				src += 2;
			if (incd == 1)
				dst += 2;
		}
		break;
	case 8: // 64 bit
		src &= ~7;
		dst &= ~7;
		for (;count > 0; count --)
		{
			if (incs == 2)
				src -= 8;
			if (incd == 2)
				dst -= 8;
			m_program->write_qword(dst & SH34_AM, m_program->read_qword(src & SH34_AM));
			if (incs == 1)
				src += 8;
			if (incd == 1)
				dst += 8;

		}
		break;
	case 4: // 32 bit
		src &= ~3;
		dst &= ~3;
		for (;count > 0; count --)
		{
			if (incs == 2)
				src -= 4;
			if (incd == 2)
				dst -= 4;
			m_program->write_dword(dst & SH34_AM, m_program->read_dword(src & SH34_AM));
			if (incs == 1)
				src += 4;
			if (incd == 1)
				dst += 4;

		}
		break;
	case 32:
		src &= ~31;
		dst &= ~31;
		for (;count > 0; count --)
		{
			if (incs == 2)
				src -= 32;
			if (incd == 2)
				dst -= 32;
			m_program->write_qword(dst & SH34_AM, m_program->read_qword(src & SH34_AM));
			m_program->write_qword((dst + 8) & SH34_AM, m_program->read_qword((src + 8) & SH34_AM));
			m_program->write_qword((dst + 16) & SH34_AM, m_program->read_qword((src + 16) & SH34_AM));
			m_program->write_qword((dst + 24) & SH34_AM, m_program->read_qword((src + 24) & SH34_AM));
			if (incs == 1)
				src += 32;
			if (incd == 1)
				dst += 32;
		}
		break;
	}
	*sar    = (*sar & ~SH34_AM) | src;
	*dar    = (*dar & ~SH34_AM) | dst;
	*dmatcr = count;
	return 1;
}

int sh34_base_device::sh4_dma_transfer_device(int channel, uint32_t chcr, uint32_t *sar, uint32_t *dar, uint32_t *dmatcr)
{
	int incd = (chcr & CHCR_DM) >> 14;
	int incs = (chcr & CHCR_SM) >> 12;

	int size;
	if (m_cpu_type == CPU_TYPE_SH4)
	{
		size = dmasize[(chcr & CHCR_TS) >> 4];
	}
	else
	{
		size = sh3_dmasize[(chcr >> 3) & 3];
	}

	int mod = ((chcr & CHCR_RS) >> 8);
	if (incd == 3 || incs == 3)
	{
		logerror("SH4: DMA: bad increment values (%d, %d, %d, %04x)\n", incd, incs, size, chcr);
		return 0;
	}

	uint32_t src   = *sar;
	uint32_t dst   = *dar;
	uint32_t count = *dmatcr;
	if (!count)
		count = 0x1000000;

	LOG("SH4: DMA %d start device<->memory %x, %x, %x, %04x, %d, %d, %d\n", channel, src, dst, count, chcr, incs, incd, size);

	m_dma_timer_active[channel] = 1;

	src &= SH34_AM;
	dst &= SH34_AM;

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
	uint32_t dmatcr, chcr, sar, dar;

	switch (channel)
	{
	case 0:
		sar = m_sar0;
		dar = m_dar0;
		chcr = m_chcr0;
		dmatcr = m_dmatcr0;
		break;
	case 1:
		sar = m_sar1;
		dar = m_dar1;
		chcr = m_chcr1;
		dmatcr = m_dmatcr1;
		break;
	case 2:
		sar = m_sar2;
		dar = m_dar2;
		chcr = m_chcr2;
		dmatcr = m_dmatcr2;
		break;
	case 3:
		sar = m_sar3;
		dar = m_dar3;
		chcr = m_chcr3;
		dmatcr = m_dmatcr3;
		break;
	default:
		return;
	}
	if (chcr & m_dmaor & DMAOR_DME)
	{
		if ((((chcr & CHCR_RS) >> 8) < 2) || (((chcr & CHCR_RS) >> 8) > 6))
			return;
		if (!m_dma_timer_active[channel] && !(chcr & CHCR_TE) && !(m_dmaor & (DMAOR_AE | DMAOR_NMIF)))
		{
			if (((chcr & CHCR_RS) >> 8) > 3)
				sh4_dma_transfer(channel, 1, chcr, &sar, &dar, &dmatcr);
			else if ((m_dmaor & DMAOR_DDT) == 0)
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
	int channel = s->channel;
	void *data = s->buffer;

	if (!m_dma_timer_active[channel])
		return 0;

	if (m_dma_mode[channel] == 2)
	{
		// device receives data
		uint32_t len = m_dma_count[channel];
		if (s->length < len)
			len = s->length;
		uint32_t siz = m_dma_wordsize[channel];
		for (uint32_t pos = 0; pos < len; pos++)
		{
			switch (siz)
			{
			case 8:
				if (m_dma_source_increment[channel] == 2)
					m_dma_source[channel] -= 8;
				*(uint64_t *)data = m_program->read_qword(m_dma_source[channel] & SH34_AM & ~7);
				if (m_dma_source_increment[channel] == 1)
					m_dma_source[channel] += 8;
				break;
			case 1:
				if (m_dma_source_increment[channel] == 2)
					m_dma_source[channel]--;
				*(uint8_t *)data = m_program->read_byte(m_dma_source[channel] & SH34_AM);
				if (m_dma_source_increment[channel] == 1)
					m_dma_source[channel]++;
				break;
			case 2:
				if (m_dma_source_increment[channel] == 2)
					m_dma_source[channel] -= 2;
				*(uint16_t *)data = m_program->read_word(m_dma_source[channel] & SH34_AM & ~1);
				if (m_dma_source_increment[channel] == 1)
					m_dma_source[channel] += 2;
				break;
			case 4:
				if (m_dma_source_increment[channel] == 2)
					m_dma_source[channel] -= 4;
				*(uint32_t *)data = m_program->read_dword(m_dma_source[channel] & SH34_AM & ~3);
				if (m_dma_source_increment[channel] == 1)
					m_dma_source[channel] += 4;
				break;
			case 32:
				if (m_dma_source_increment[channel] == 2)
					m_dma_source[channel] -= 32;
				*(uint64_t *)data = m_program->read_qword(m_dma_source[channel] & SH34_AM & ~31);
				*((uint64_t *)data+1) = m_program->read_qword((m_dma_source[channel] & SH34_AM & ~31)+8);
				*((uint64_t *)data+2) = m_program->read_qword((m_dma_source[channel] & SH34_AM & ~31)+16);
				*((uint64_t *)data+3) = m_program->read_qword((m_dma_source[channel] & SH34_AM & ~31)+24);
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
		uint32_t len = m_dma_count[channel];
		if (s->length < len)
			len = s->length;
		uint32_t siz = m_dma_wordsize[channel];
		for (uint32_t pos = 0; pos < len; pos++)
		{
			switch (siz)
			{
			case 8:
				if (m_dma_destination_increment[channel] == 2)
					m_dma_destination[channel]-=8;
				m_program->write_qword(m_dma_destination[channel] & SH34_AM & ~7, *(uint64_t *)data);
				if (m_dma_destination_increment[channel] == 1)
					m_dma_destination[channel]+=8;
				break;
			case 1:
				if (m_dma_destination_increment[channel] == 2)
					m_dma_destination[channel]--;
				m_program->write_byte(m_dma_destination[channel] & SH34_AM, *(uint8_t *)data);
				if (m_dma_destination_increment[channel] == 1)
					m_dma_destination[channel]++;
				break;
			case 2:
				if (m_dma_destination_increment[channel] == 2)
					m_dma_destination[channel]-=2;
				m_program->write_word(m_dma_destination[channel] & SH34_AM & ~1, *(uint16_t *)data);
				if (m_dma_destination_increment[channel] == 1)
					m_dma_destination[channel]+=2;
				break;
			case 4:
				if (m_dma_destination_increment[channel] == 2)
					m_dma_destination[channel]-=4;
				m_program->write_dword(m_dma_destination[channel] & SH34_AM & ~3, *(uint32_t *)data);
				if (m_dma_destination_increment[channel] == 1)
					m_dma_destination[channel]+=4;
				break;
			case 32:
				if (m_dma_destination_increment[channel] == 2)
					m_dma_destination[channel]-=32;
				m_program->write_qword(m_dma_destination[channel] & SH34_AM & ~31, *(uint64_t *)data);
				m_program->write_qword((m_dma_destination[channel] & SH34_AM & ~31)+8, *((uint64_t *)data+1));
				m_program->write_qword((m_dma_destination[channel] & SH34_AM & ~31)+16, *((uint64_t *)data+2));
				m_program->write_qword((m_dma_destination[channel] & SH34_AM & ~31)+24, *((uint64_t *)data+3));
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
	if (m_cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_dma_ddt uses m_m[] with SH3\n");

	if (m_dma_timer_active[s->channel])
		return;
	if (s->mode >= 0)
	{
		switch (s->channel)
		{
		case 0:
			if (s->mode & 1)
				s->source = m_sar0;
			if (s->mode & 2)
				m_sar0 = s->source;
			if (s->mode & 4)
				s->destination = m_dar0;
			if (s->mode & 8)
				m_dar0 = s->destination;
			break;
		case 1:
			if (s->mode & 1)
				s->source = m_sar1;
			if (s->mode & 2)
				m_sar1 = s->source;
			if (s->mode & 4)
				s->destination = m_dar1;
			if (s->mode & 8)
				m_dar1 = s->destination;
			break;
		case 2:
			if (s->mode & 1)
				s->source = m_sar2;
			if (s->mode & 2)
				m_sar2 = s->source;
			if (s->mode & 4)
				s->destination = m_dar2;
			if (s->mode & 8)
				m_dar2 = s->destination;
			break;
		case 3:
		default:
			if (s->mode & 1)
				s->source = m_sar3;
			if (s->mode & 2)
				m_sar3 = s->source;
			if (s->mode & 4)
				s->destination = m_dar3;
			if (s->mode & 8)
				m_dar3 = s->destination;
			break;
		}
		uint32_t len;
		uint32_t chcr;
		switch (s->channel)
		{
		case 0:
			chcr = m_chcr0;
			len = m_dmatcr0;
			break;
		case 1:
			chcr = m_chcr1;
			len = m_dmatcr1;
			break;
		case 2:
			chcr = m_chcr2;
			len = m_dmatcr2;
			break;
		case 3:
		default:
			chcr = m_chcr3;
			len = m_dmatcr3;
			break;
		}

		if (s->direction == 0)
			chcr = (chcr & 0xffff3fff) | ((s->mode & 0x30) << 10);
		else
			chcr = (chcr & 0xffffcfff) | ((s->mode & 0x30) << 8);

		uint32_t siz = 0;
		if (m_cpu_type == CPU_TYPE_SH4)
		{
			//siz = dmasize[(chcr & CHCR_TS) >> 4];
			siz = dmasize[(chcr >> 4) & 7];
		}
		else
		{
			siz = sh3_dmasize[(chcr >> 3) & 3];
		}


		if (siz && s->size)
			if (len * siz != s->length * s->size)
				return;
		sh4_dma_transfer(s->channel, 0, chcr, &s->source, &s->destination, &len);
	}
	else
	{
		if (s->size == 4)
		{
			if ((s->direction) == 0)
			{
				uint32_t *p32bits = (uint32_t *)s->buffer;
				for (uint32_t pos = 0; pos < s->length; pos++)
				{
					*p32bits++ = m_program->read_dword(s->source & SH34_AM);
					s->source = s->source + 4;
				}
			}
			else
			{
				uint32_t *p32bits = (uint32_t *)s->buffer;
				for (uint32_t pos = 0; pos < s->length; pos++)
				{
					m_program->write_dword(s->destination & SH34_AM, *p32bits);
					p32bits++;
					s->destination = s->destination + 4;
				}
			}
		}
		if (s->size == 32)
		{
			if ((s->direction) == 0)
			{
				uint64_t *p32bytes = (uint64_t *)s->buffer;
				for (uint32_t pos = 0; pos < s->length * 4; pos++)
				{
					*p32bytes++ = m_program->read_qword(s->source & SH34_AM);
					s->destination = s->destination + 8;
				}
			}
			else
			{
				uint64_t *p32bytes = (uint64_t *)s->buffer;
				for (uint32_t pos = 0; pos < s->length * 4; pos++)
				{
					m_program->write_qword(s->destination & SH34_AM, *p32bytes);
					p32bytes++;
					s->destination = s->destination + 8;
				}
			}
		}
	}
}

uint32_t sh34_base_device::sar0_r(offs_t offset, uint32_t mem_mask)
{
	return m_sar0;
}

void sh34_base_device::sar0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sar0);
}

uint32_t sh34_base_device::dar0_r(offs_t offset, uint32_t mem_mask)
{
	return m_dar0;
}

void sh34_base_device::dar0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dar0);
}

uint32_t sh34_base_device::dmatcr0_r(offs_t offset, uint32_t mem_mask)
{
	return m_dmatcr0;
}

void sh34_base_device::dmatcr0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmatcr0);
}

uint32_t sh34_base_device::chcr0_r(offs_t offset, uint32_t mem_mask)
{
	return m_chcr0;
}

void sh34_base_device::chcr0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_chcr0);
	sh4_dmac_check(0);
}

uint32_t sh34_base_device::sar1_r(offs_t offset, uint32_t mem_mask)
{
	return m_sar1;
}

void sh34_base_device::sar1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sar1);
}

uint32_t sh34_base_device::dar1_r(offs_t offset, uint32_t mem_mask)
{
	return m_dar1;
}

void sh34_base_device::dar1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dar1);
}

uint32_t sh34_base_device::dmatcr1_r(offs_t offset, uint32_t mem_mask)
{
	return m_dmatcr1;
}

void sh34_base_device::dmatcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmatcr1);
}

uint32_t sh34_base_device::chcr1_r(offs_t offset, uint32_t mem_mask)
{
	return m_chcr1;
}

void sh34_base_device::chcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_chcr1);
	sh4_dmac_check(1);
}

uint32_t sh34_base_device::sar2_r(offs_t offset, uint32_t mem_mask)
{
	return m_sar2;
}

void sh34_base_device::sar2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sar2);
}

uint32_t sh34_base_device::dar2_r(offs_t offset, uint32_t mem_mask)
{
	return m_dar2;
}

void sh34_base_device::dar2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dar2);
}

uint32_t sh34_base_device::dmatcr2_r(offs_t offset, uint32_t mem_mask)
{
	return m_dmatcr2;
}

void sh34_base_device::dmatcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmatcr2);
}

uint32_t sh34_base_device::chcr2_r(offs_t offset, uint32_t mem_mask)
{
	return m_chcr2;
}

void sh34_base_device::chcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_chcr2);
	sh4_dmac_check(2);
}

uint32_t sh34_base_device::sar3_r(offs_t offset, uint32_t mem_mask)
{
	return m_sar3;
}

void sh34_base_device::sar3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sar3);
}

uint32_t sh34_base_device::dar3_r(offs_t offset, uint32_t mem_mask)
{
	return m_dar3;
}

void sh34_base_device::dar3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dar3);
}

uint32_t sh34_base_device::dmatcr3_r(offs_t offset, uint32_t mem_mask)
{
	return m_dmatcr3;
}

void sh34_base_device::dmatcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmatcr3);
}

uint32_t sh34_base_device::chcr3_r(offs_t offset, uint32_t mem_mask)
{
	return m_chcr3;
}

void sh34_base_device::chcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_chcr3);
	sh4_dmac_check(3);
}

uint32_t sh34_base_device::dmaor_r(offs_t offset, uint32_t mem_mask)
{
	return m_dmaor;
}

void sh34_base_device::dmaor_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old = m_dmaor;
	COMBINE_DATA(&m_dmaor);

	if ((m_dmaor & DMAOR_AE) && (~old & DMAOR_AE))
		m_dmaor &= ~DMAOR_AE;
	if ((m_dmaor & DMAOR_NMIF) && (~old & DMAOR_NMIF))
		m_dmaor &= ~DMAOR_NMIF;
	sh4_dmac_check(0);
	sh4_dmac_check(1);
	sh4_dmac_check(2);
	sh4_dmac_check(3);
}
