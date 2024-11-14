// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 Direct Memory Access Controller

  TODO list (not comprehensive):
    - Channel priority
    - External dual and single address modes
    - DREQ, DACK, DRAK pins

***************************************************************************/

#include "emu.h"
#include "sh7014_dmac.h"

#define LOG_TX (1U << 1)

// #define VERBOSE (LOG_GENERAL | LOG_TX)

#include "logmacro.h"


DEFINE_DEVICE_TYPE(SH7014_DMAC, sh7014_dmac_device, "sh7014dmac", "SH7014 Direct Memory Access Controller")
DEFINE_DEVICE_TYPE(SH7014_DMAC_CHANNEL, sh7014_dmac_channel_device, "sh7014dmacchan", "SH7014 Direct Memory Access Controller Channel")


sh7014_dmac_device::sh7014_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7014_DMAC, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_intc(*this, finder_base::DUMMY_TAG)
	, m_chan(*this, "ch%u", 0u)
{
}

void sh7014_dmac_device::device_start()
{
	save_item(NAME(m_dmaor));
}

void sh7014_dmac_device::device_reset()
{
	m_dmaor = 0;
}

void sh7014_dmac_device::device_add_mconfig(machine_config &config)
{
	SH7014_DMAC_CHANNEL(config, m_chan[0], DERIVED_CLOCK(1, 1), *this, m_cpu, m_intc,
		0, // channel
		sh7014_intc_device::INT_VECTOR_DMA_CH0
	);

	SH7014_DMAC_CHANNEL(config, m_chan[1], DERIVED_CLOCK(1, 1), *this, m_cpu, m_intc,
		1, // channel
		sh7014_intc_device::INT_VECTOR_DMA_CH1
	);
}

void sh7014_dmac_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(sh7014_dmac_device::dmaor_r), FUNC(sh7014_dmac_device::dmaor_w));
	map(0x10, 0x1f).m(m_chan[0], FUNC(sh7014_dmac_channel_device::map));
	map(0x20, 0x2f).m(m_chan[1], FUNC(sh7014_dmac_channel_device::map));
}

///

uint16_t sh7014_dmac_device::dmaor_r()
{
	return m_dmaor & 7;
}

void sh7014_dmac_device::dmaor_w(uint16_t data)
{
	m_dmaor = (data & ~6) | (m_dmaor & data & 6);
	m_chan[0]->dma_check();
	m_chan[1]->dma_check();
}

bool sh7014_dmac_device::is_transfer_allowed()
{
	const bool is_enabled = (m_dmaor & DMAOR_DME) != 0;
	const bool has_address_error = (m_dmaor & DMAOR_AE) != 0;
	const bool has_nmi_flag = (m_dmaor & DMAOR_NMIF) != 0;
	return is_enabled && !has_nmi_flag && !has_address_error;
}

int sh7014_dmac_device::is_dma_activated(int vector)
{
	int activated = 0;

	if (!is_transfer_allowed())
		return 0;

	if (m_chan[0]->is_dma_activated(vector))
		activated |= 1;
	if (m_chan[1]->is_dma_activated(vector))
		activated |= 2;

	return activated;
}


//////////////////


void sh7014_dmac_channel_device::map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(sh7014_dmac_channel_device::sar_r), FUNC(sh7014_dmac_channel_device::sar_w));
	map(0x04, 0x07).rw(FUNC(sh7014_dmac_channel_device::dar_r), FUNC(sh7014_dmac_channel_device::dar_w));
	map(0x08, 0x0b).rw(FUNC(sh7014_dmac_channel_device::dmatcr_r), FUNC(sh7014_dmac_channel_device::dmatcr_w));
	map(0x0c, 0x0f).rw(FUNC(sh7014_dmac_channel_device::chcr_r), FUNC(sh7014_dmac_channel_device::chcr_w));
}

sh7014_dmac_channel_device::sh7014_dmac_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7014_DMAC_CHANNEL, tag, owner, clock)
	, m_dmac(*this, finder_base::DUMMY_TAG)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_intc(*this, finder_base::DUMMY_TAG)
	, m_notify_dma_source_cb(*this)
{
}

void sh7014_dmac_channel_device::device_start()
{
	save_item(NAME(m_sar));
	save_item(NAME(m_dar));
	save_item(NAME(m_dmatcr));
	save_item(NAME(m_chcr));

	save_item(NAME(m_dma_timer_active));
	save_item(NAME(m_active_dma_addr_mode_source));
	save_item(NAME(m_active_dma_addr_mode_dest));
	save_item(NAME(m_active_dma_unit_size));
	save_item(NAME(m_active_dma_is_burst));

	save_item(NAME(m_selected_resource));
	save_item(NAME(m_request_source_type));
	save_item(NAME(m_expected_irq_vector));

	m_dma_current_active_timer = timer_alloc(FUNC(sh7014_dmac_channel_device::dma_timer_callback), this);
}

void sh7014_dmac_channel_device::device_reset()
{
	m_sar = m_dar = 0;
	m_dmatcr = 0;
	m_chcr = 0;

	m_dma_timer_active = false;
	m_active_dma_addr_mode_source = 0;
	m_active_dma_addr_mode_dest = 0;
	m_active_dma_unit_size = 1;
	m_active_dma_is_burst = false;

	m_selected_resource = RS_EXTERNAL_DUAL_ADDR;
	m_request_source_type = RS_TYPE_EXTERNAL_DUAL;
	m_expected_irq_vector = -1;

	m_dma_current_active_timer->adjust(attotime::never);
}

///

bool sh7014_dmac_channel_device::is_enabled()
{
	return (m_chcr & CHCR_DE) != 0;
}

uint32_t sh7014_dmac_channel_device::sar_r()
{
	return m_sar;
}

void sh7014_dmac_channel_device::sar_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sar);
}

uint32_t sh7014_dmac_channel_device::dar_r()
{
	return m_dar;
}

void sh7014_dmac_channel_device::dar_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dar);
}

uint32_t sh7014_dmac_channel_device::dmatcr_r()
{
	return m_dmatcr & 0xffff;
}

void sh7014_dmac_channel_device::dmatcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmatcr);
	m_dmatcr &= 0xffff;
}

uint32_t sh7014_dmac_channel_device::chcr_r()
{
	return m_chcr & 0xbff7f;
}

void sh7014_dmac_channel_device::chcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_chcr);

	m_selected_resource = BIT(m_chcr, 8, 4);
	m_expected_irq_vector = -1;

	switch (m_selected_resource) {
		case RS_MTU_TGI0A:
			m_request_source_type = RS_TYPE_INTERNAL;
			m_expected_irq_vector = sh7014_intc_device::INT_VECTOR_MTU_TGI0A;
			break;
		case RS_MTU_TGI1A:
			m_request_source_type = RS_TYPE_INTERNAL;
			m_expected_irq_vector = sh7014_intc_device::INT_VECTOR_MTU_TGI1A;
			break;
		case RS_MTU_TGI2A:
			m_request_source_type = RS_TYPE_INTERNAL;
			m_expected_irq_vector = sh7014_intc_device::INT_VECTOR_MTU_TGI2A;
			break;
		case RS_AD:
			m_request_source_type = RS_TYPE_INTERNAL;
			m_expected_irq_vector = sh7014_intc_device::INT_VECTOR_AD;
			break;
		case RS_SCI_TXI0:
			m_request_source_type = RS_TYPE_INTERNAL;
			m_expected_irq_vector = sh7014_intc_device::INT_VECTOR_SCI_TXI0;
			break;
		case RS_SCI_RXI0:
			m_request_source_type = RS_TYPE_INTERNAL;
			m_expected_irq_vector = sh7014_intc_device::INT_VECTOR_SCI_RXI0;
			break;
		case RS_SCI_TXI1:
			m_request_source_type = RS_TYPE_INTERNAL;
			m_expected_irq_vector = sh7014_intc_device::INT_VECTOR_SCI_TXI1;
			break;
		case RS_SCI_RXI1:
			m_request_source_type = RS_TYPE_INTERNAL;
			m_expected_irq_vector = sh7014_intc_device::INT_VECTOR_SCI_RXI1;
			break;

		case RS_AUTO_REQUEST:
			m_request_source_type = RS_TYPE_AUTO;
			break;

		case RS_EXTERNAL_DUAL_ADDR:
			m_request_source_type = RS_TYPE_EXTERNAL_DUAL;
			break;

		case RS_EXTERNAL_SINGLE_ADDR_TO_DEV:
		case RS_EXTERNAL_SINGLE_DEV_TO_ADDR:
			m_request_source_type = RS_TYPE_EXTERNAL_SINGLE;
			break;

		default:
			m_request_source_type = RS_TYPE_PROHIBITED;
			break;
	}

	dma_check();
}

bool sh7014_dmac_channel_device::is_dma_activated(int vector)
{
	if (m_request_source_type != RS_TYPE_INTERNAL || m_expected_irq_vector == -1 || vector != m_expected_irq_vector)
		return false;

	if (!m_dma_timer_active)
		dma_check();

	if (!m_dma_timer_active)
		return false;

	m_dma_current_active_timer->adjust(attotime::from_ticks(2, clock()));

	return true;
}

TIMER_CALLBACK_MEMBER( sh7014_dmac_channel_device::dma_timer_callback )
{
	if (!m_dma_timer_active)
		return;

	if (!m_dmac->is_transfer_allowed()) {
		LOG("SH7014: DMA %d transfer aborted\n", m_channel_id);
		m_dma_timer_active = false;
		return;
	}

	LOGMASKED(LOG_TX, "DMAC ch %d remaining %08x size %d | source %08x dest %08x\n", m_channel_id, m_dmatcr, m_active_dma_unit_size, m_sar, m_dar);

	if (m_active_dma_addr_mode_source == DMA_ADDR_MODE_DEC)
		m_sar -= m_active_dma_unit_size;

	if (m_active_dma_addr_mode_dest == DMA_ADDR_MODE_DEC)
		m_dar -= m_active_dma_unit_size;

	if (m_request_source_type == RS_TYPE_INTERNAL)
		m_notify_dma_source_cb(m_selected_resource);

	switch (m_active_dma_unit_size) {
		case 1:
			m_cpu->m_program->write_byte(
				m_dar,
				m_cpu->m_program->read_byte(m_sar)
			);
			break;
		case 2:
			m_cpu->m_program->write_word(
				m_dar,
				m_cpu->m_program->read_word(m_sar)
			);
			break;
		case 4:
			m_cpu->m_program->write_dword(
				m_dar,
				m_cpu->m_program->read_dword(m_sar)
			);
			break;
	}

	if (m_active_dma_addr_mode_source == DMA_ADDR_MODE_INC)
		m_sar += m_active_dma_unit_size;

	if (m_active_dma_addr_mode_dest == DMA_ADDR_MODE_INC)
		m_dar += m_active_dma_unit_size;

	m_dmatcr--;

	if (m_dmatcr <= 0) {
		LOGMASKED(LOG_TX, "SH7014: DMA %d complete\n", m_channel_id);

		m_dmatcr = 0;
		m_chcr |= CHCR_TE; // transfer end
		m_dma_timer_active = false;

		if (m_active_dma_is_burst)
			m_cpu->resume(SUSPEND_REASON_HALT);

		const auto interrupt_enable = (m_chcr & CHCR_IE) != 0;
		if (interrupt_enable)
			m_intc->set_interrupt(m_vector, ASSERT_LINE);
	} else {
		// schedule next DMA callback
		// Internal source transfers in burst mode end on last transfer, otherwise should end after the first transfer when burst mode is off
		// TODO: DREQ and DACK are needed for external sources instead of being handled like an auto requests
		if (m_request_source_type != RS_TYPE_INTERNAL || (m_request_source_type == RS_TYPE_INTERNAL && m_active_dma_is_burst))
			m_dma_current_active_timer->adjust(attotime::from_ticks(2, clock()));
	}
}

void sh7014_dmac_channel_device::dma_check()
{
	if (!m_dmac->is_transfer_allowed()) {
		if (m_dma_timer_active) {
			LOG("SH7014: DMA %d cancelled in-flight\n", m_channel_id);
			m_dma_current_active_timer->adjust(attotime::never);
			m_dma_timer_active = false;
		}

		return;
	}

	if (m_dma_timer_active)
		return;

	m_active_dma_addr_mode_dest = BIT(m_chcr, 14, 2); // DM
	m_active_dma_addr_mode_source = BIT(m_chcr, 12, 2); // SM
	m_active_dma_unit_size = 1 << BIT(m_chcr, 3, 2); // TS
	m_active_dma_is_burst = (m_chcr & CHCR_TM) != 0;

	if (m_active_dma_addr_mode_dest == DMA_ADDR_MODE_PROHIBITED || m_active_dma_addr_mode_source == DMA_ADDR_MODE_PROHIBITED || m_active_dma_unit_size > 4) {
		LOG("SH7014: DMA %d: bad increment values (%d, %d, %d, %04x)\n", m_channel_id, m_active_dma_addr_mode_dest, m_active_dma_addr_mode_source, m_active_dma_unit_size, m_chcr);
		return;
	}

	if (m_dmatcr == 0)
		m_dmatcr = 0x10000;

	m_dma_timer_active = true;

	LOG("SH7014: DMA %d start %x, %x, %x, %04x, %d, %d, %d, %d\n", m_channel_id, m_sar, m_dar, m_dmatcr, m_chcr, m_active_dma_addr_mode_source, m_active_dma_addr_mode_dest, m_active_dma_unit_size, m_active_dma_is_burst);

	if (m_active_dma_unit_size > 1) {
		const int mask = m_active_dma_unit_size - 1;
		m_sar &= ~mask;
		m_dar &= ~mask;
	}

	if (m_active_dma_is_burst)
		m_cpu->suspend(SUSPEND_REASON_HALT, 1);

	if (m_request_source_type != RS_TYPE_INTERNAL || (m_request_source_type == RS_TYPE_INTERNAL && m_active_dma_is_burst))
		m_dma_current_active_timer->adjust(attotime::from_ticks(2, clock()));
}
