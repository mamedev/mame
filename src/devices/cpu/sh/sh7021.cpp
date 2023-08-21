// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "sh7021.h"

DEFINE_DEVICE_TYPE(SH2A_SH7021, sh2a_sh7021_device, "sh2a_sh7021", "Hitachi SH-2A (SH7021)")


sh2a_sh7021_device::sh2a_sh7021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh2_device(mconfig, SH2A_SH7021, tag, owner, clock, CPU_TYPE_SH2, address_map_constructor(FUNC(sh2a_sh7021_device::sh7021_map), this), 28, 0xc7ffffff)
{
}

void sh2a_sh7021_device::device_start()
{
	sh2_device::device_start();

	save_item(NAME(m_sh7021_regs));
	save_item(NAME(m_dmaor));
	save_item(STRUCT_MEMBER(m_dma, sar));
	save_item(STRUCT_MEMBER(m_dma, dar));
	save_item(STRUCT_MEMBER(m_dma, tcr));
	save_item(STRUCT_MEMBER(m_dma, chcr));
}

void sh2a_sh7021_device::device_reset()
{
	sh2_device::device_reset();

	std::fill(std::begin(m_sh7021_regs), std::end(m_sh7021_regs), 0);

	m_dmaor = 0;

	for(int i = 0; i < 4; i++)
	{
		m_dma[i].sar = 0;
		m_dma[i].dar = 0;
		m_dma[i].tcr = 0;
		m_dma[i].chcr = 0;
	}
}

void sh2a_sh7021_device::sh7021_map(address_map &map)
{
//  fall-back
	map(0x05fffe00, 0x05ffffff).rw(FUNC(sh2a_sh7021_device::sh7021_r), FUNC(sh2a_sh7021_device::sh7021_w)); // SH-7032H internal i/o
//  overrides
	map(0x05ffff40, 0x05ffff43).rw(FUNC(sh2a_sh7021_device::dma_sar0_r), FUNC(sh2a_sh7021_device::dma_sar0_w));
	map(0x05ffff44, 0x05ffff47).rw(FUNC(sh2a_sh7021_device::dma_dar0_r), FUNC(sh2a_sh7021_device::dma_dar0_w));
	map(0x05ffff48, 0x05ffff49).rw(FUNC(sh2a_sh7021_device::dmaor_r), FUNC(sh2a_sh7021_device::dmaor_w));
	map(0x05ffff4a, 0x05ffff4b).rw(FUNC(sh2a_sh7021_device::dma_tcr0_r), FUNC(sh2a_sh7021_device::dma_tcr0_w));
	map(0x05ffff4e, 0x05ffff4f).rw(FUNC(sh2a_sh7021_device::dma_chcr0_r), FUNC(sh2a_sh7021_device::dma_chcr0_w));
//  map(0x07000000, 0x070003ff).ram().share("oram"); // on-chip RAM, actually at 0xf000000 (1 kb)
//  map(0x0f000000, 0x0f0003ff).ram().share("oram"); // on-chip RAM, actually at 0xf000000 (1 kb)
}

void sh2a_sh7021_device::sh7021_dma_exec(int ch)
{
	const short dma_word_size[4] = { 0, +1, -1, 0 };
	uint8_t rs = (m_dma[ch].chcr >> 8) & 0xf; /**< Resource Select bits */
	if(rs != 0xc) // Auto-Request
	{
		logerror("Warning: SH7021 DMA enables non auto-request transfer\n");
		return;
	}

	// channel enable & master enable
	if((m_dma[ch].chcr & 1) == 0 || (m_dmaor & 1) == 0)
		return;

	// printf("%08x %08x %04x\n",m_dma[ch].sar,m_dma[ch].dar,m_dma[ch].chcr);
	uint8_t dm = (m_dma[ch].chcr >> 14) & 3;  /**< Destination Address Mode bits */
	uint8_t sm = (m_dma[ch].chcr >> 12) & 3;  /**< Source Address Mode bits */
	bool ts = (m_dma[ch].chcr & 8);         /**< Transfer Size bit */
	int src_word_size = dma_word_size[sm] * ((ts == true) ? 2 : 1);
	int dst_word_size = dma_word_size[dm] * ((ts == true) ? 2 : 1);
	uint32_t src_addr = m_dma[ch].sar;
	uint32_t dst_addr = m_dma[ch].dar;
	uint32_t size_index = m_dma[ch].tcr;
	if(size_index == 0)
		size_index = 0x10000;

	if(ts == false)
		logerror("SH7021: DMA byte mode check\n");

	for(int index = size_index;index>-1;index--)
	{
		if(ts == true)
			m_program->write_word(dst_addr,m_program->read_word(src_addr));
		else
			m_program->write_byte(dst_addr,m_program->read_byte(src_addr));

		src_addr += src_word_size;
		dst_addr += dst_word_size;
	}

	m_dma[ch].chcr &= ~1; /**< @todo non-instant DMA */
	// printf("%02x %02x %02x %1d\n",sm,dm,rs,ts);
}

uint32_t sh2a_sh7021_device::dma_sar0_r()
{
	return m_dma[0].sar;
}

void sh2a_sh7021_device::dma_sar0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dma[0].sar);
}

uint32_t sh2a_sh7021_device::dma_dar0_r()
{
	return m_dma[0].dar;
}

void sh2a_sh7021_device::dma_dar0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dma[0].dar);
}

uint16_t sh2a_sh7021_device::dma_tcr0_r()
{
	return m_dma[0].tcr;
}

void sh2a_sh7021_device::dma_tcr0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("%04x\n",data);
	COMBINE_DATA(&m_dma[0].tcr);
}

uint16_t sh2a_sh7021_device::dma_chcr0_r()
{
	return m_dma[0].chcr;
}

void sh2a_sh7021_device::dma_chcr0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("%04x CHCR0\n",data);
	COMBINE_DATA(&m_dma[0].chcr);
	sh7021_dma_exec(0);
}

uint16_t sh2a_sh7021_device::dmaor_r()
{
	return m_dmaor;
}

void sh2a_sh7021_device::dmaor_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dmaor);
	sh7021_dma_exec(0);
}

uint16_t sh2a_sh7021_device::sh7021_r(offs_t offset)
{
	return m_sh7021_regs[offset];
}

void sh2a_sh7021_device::sh7021_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sh7021_regs[offset]);
}
