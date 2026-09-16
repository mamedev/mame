// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68340 DMA module */

#include "emu.h"
#include "68340.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(MC68340_DMA_MODULE, mc68340_dma_module_device, "mc68340dma", "MC68340 DMA Module")


namespace {

constexpr uint8_t CSR_IRQ  = 0x80;
constexpr uint8_t CSR_DONE = 0x40;
constexpr uint8_t CSR_BES  = 0x20;
constexpr uint8_t CSR_BED  = 0x10;
constexpr uint8_t CSR_CONF = 0x08;
constexpr uint8_t CSR_BRKP = 0x04;
constexpr uint8_t CSR_CLEARABLE = CSR_DONE | CSR_BES | CSR_BED | CSR_CONF | CSR_BRKP;

constexpr uint16_t MCR_STP = 0x8000;
constexpr uint16_t MCR_SHARED = 0xe00f;

constexpr uint16_t CCR_INTB = 0x8000;
constexpr uint16_t CCR_INTN = 0x4000;
constexpr uint16_t CCR_INTE = 0x2000;
constexpr uint16_t CCR_ECO  = 0x1000;
constexpr uint16_t CCR_SAPI = 0x0800;
constexpr uint16_t CCR_DAPI = 0x0400;
constexpr uint16_t CCR_REQ  = 0x0030;
constexpr uint16_t CCR_SD   = 0x0002;
constexpr uint16_t CCR_STR  = 0x0001;

unsigned transfer_size(unsigned field)
{
	switch (field & 3)
	{
	case 0: return 4;
	case 1: return 1;
	case 2: return 2;
	default: return 0;
	}
}

} // anonymous namespace


uint16_t mc68340_dma_module_device::read(offs_t offset, uint16_t)
{
	unsigned const byte_offset = offset * 2;
	channel_state const &channel = m_channel[BIT(byte_offset, 5)];

	switch (byte_offset & 0x1e)
	{
	case 0x00: return channel.mcr;
	case 0x04: return channel.intr;
	case 0x08: return channel.ccr;
	case 0x0a: return (uint16_t(channel.csr) << 8) | channel.fcr;
	case 0x0c: return channel.sar >> 16;
	case 0x0e: return channel.sar;
	case 0x10: return channel.dar >> 16;
	case 0x12: return channel.dar;
	case 0x14: return channel.btc >> 16;
	case 0x16: return channel.btc;
	default: return 0;
	}
}


void mc68340_dma_module_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	unsigned const byte_offset = offset * 2;
	unsigned const channel_number = BIT(byte_offset, 5);
	channel_state &channel = m_channel[channel_number];
	unsigned const reg = byte_offset & 0x1e;

	auto combine16 = [data, mem_mask] (uint16_t &value)
	{
		value = (value & ~mem_mask) | (data & mem_mask);
	};
	auto combine32 = [reg, data, mem_mask] (uint32_t &value)
	{
		if (BIT(reg, 1))
			value = (value & ~(uint32_t(mem_mask))) | (data & mem_mask);
		else
			value = (value & ~(uint32_t(mem_mask) << 16)) | (uint32_t(data & mem_mask) << 16);
	};

	switch (reg)
	{
	case 0x00:
		combine16(channel.mcr);
		m_channel[channel_number ^ 1].mcr = (m_channel[channel_number ^ 1].mcr & ~MCR_SHARED) | (channel.mcr & MCR_SHARED);
		m_cpu->update_ipl();
		break;

	case 0x04:
		combine16(channel.intr);
		m_cpu->update_ipl();
		break;

	case 0x08:
		combine16(channel.ccr);
		if (channel.csr & CSR_IRQ)
			channel.ccr &= ~CCR_STR;
		m_cpu->update_ipl();
		run(channel_number);
		break;

	case 0x0a:
		if (ACCESSING_BITS_8_15)
		{
			channel.csr &= ~uint8_t((data >> 8) & CSR_CLEARABLE);
			if (!(channel.csr & CSR_CLEARABLE))
				channel.csr &= ~CSR_IRQ;
		}
		if (ACCESSING_BITS_0_7)
			channel.fcr = data;
		m_cpu->update_ipl();
		break;

	case 0x0c:
	case 0x0e:
		combine32(channel.sar);
		break;

	case 0x10:
	case 0x12:
		combine32(channel.dar);
		break;

	case 0x14:
	case 0x16:
		combine32(channel.btc);
		break;
	}
}


void mc68340_dma_module_device::dreq_w(unsigned channel_number, int state)
{
	channel_state &channel = m_channel[channel_number];
	uint8_t const old_state = channel.dreq;
	channel.dreq = bool(state);

	// DREQ is active low.  Burst requests are level-sensitive, while
	// cycle-steal requests are recognized on the falling edge.
	if (!channel.dreq && old_state)
		run(channel_number);
}


void mc68340_dma_module_device::done_w(unsigned channel_number, int state)
{
	m_channel[channel_number].done_in = bool(state);
}


bool mc68340_dma_module_device::irq_pending(channel_state const &channel) const
{
	return ((channel.csr & CSR_DONE) && (channel.ccr & CCR_INTN)) ||
		((channel.csr & (CSR_BES | CSR_BED | CSR_CONF)) && (channel.ccr & CCR_INTE)) ||
		((channel.csr & CSR_BRKP) && (channel.ccr & CCR_INTB));
}


uint8_t mc68340_dma_module_device::irq_level() const
{
	uint8_t level = 0;
	for (channel_state const &channel : m_channel)
	{
		if (irq_pending(channel))
			level = std::max<uint8_t>(level, (channel.intr >> 8) & 7);
	}
	return level;
}


uint8_t mc68340_dma_module_device::arbitrate(uint8_t level) const
{
	for (channel_state const &channel : m_channel)
	{
		if (irq_pending(channel) && (((channel.intr >> 8) & 7) == level))
			return channel.mcr & 0x0f;
	}
	return 0;
}


uint8_t mc68340_dma_module_device::irq_vector(uint8_t level) const
{
	// Channel 1 has priority when both channels use the same interrupt level.
	for (channel_state const &channel : m_channel)
	{
		if (irq_pending(channel) && (((channel.intr >> 8) & 7) == level))
			return channel.intr;
	}
	return 0x0f;
}


void mc68340_dma_module_device::run(unsigned channel_number)
{
	channel_state &channel = m_channel[channel_number];
	if (!(channel.ccr & CCR_STR) || (channel.mcr & MCR_STP))
		return;

	unsigned const request_mode = channel.ccr & CCR_REQ;
	if (!request_mode)
	{
		while (channel.ccr & CCR_STR)
			transfer(channel_number);
	}
	else if (!channel.dreq)
	{
		// Burst mode continues while DREQ remains asserted; cycle-steal mode
		// performs one operand transfer for each assertion.
		do
		{
			transfer(channel_number);
		}
		while ((request_mode == 0x0020) && !channel.dreq && (channel.ccr & CCR_STR));
	}
}


void mc68340_dma_module_device::transfer(unsigned channel_number)
{
	channel_state &channel = m_channel[channel_number];
	unsigned const source_size = transfer_size(channel.ccr >> 8);
	unsigned const destination_size = transfer_size(channel.ccr >> 6);
	unsigned const transfer_bytes = std::max(source_size, destination_size);

	// Single-address transfers require modelling the external DACK/DONE bus
	// handshake.  Dual-address transfers include the DHR packing modes.
	if ((channel.ccr & CCR_SD) || !source_size || !destination_size || !channel.btc ||
		(channel.btc % transfer_bytes) || (channel.sar & (source_size - 1)) ||
		(channel.dar & (destination_size - 1)))
	{
		set_status(channel, CSR_CONF);
		return;
	}

	uint8_t const source_fc = (channel.fcr >> 4) & 7;
	uint8_t const destination_fc = channel.fcr & 7;
	bool const external_request = (channel.ccr & CCR_REQ) != 0;
	bool const source_request = (channel.ccr & CCR_ECO) != 0;
	bool const last_transfer = channel.btc == transfer_bytes;
	uint32_t data = 0;

	if (external_request && source_request)
	{
		set_done_output(channel_number, last_transfer ? 0 : 1);
		set_dack(channel_number, 0);
	}

	for (unsigned byte = 0; byte < transfer_bytes; byte += source_size)
	{
		uint32_t source_data;
		switch (source_size)
		{
		case 1: source_data = m_cpu->m68ki_read_8_fc(channel.sar, source_fc); break;
		case 2: source_data = m_cpu->m68ki_read_16_fc(channel.sar, source_fc); break;
		default: source_data = m_cpu->m68ki_read_32_fc(channel.sar, source_fc); break;
		}
		data |= source_data << ((transfer_bytes - source_size - byte) * 8);
		if (channel.ccr & CCR_SAPI)
			channel.sar += source_size;
	}

	if (external_request && source_request)
	{
		set_dack(channel_number, 1);
		set_done_output(channel_number, 1);
	}
	else if (external_request)
	{
		set_done_output(channel_number, last_transfer ? 0 : 1);
		set_dack(channel_number, 0);
	}

	for (unsigned byte = 0; byte < transfer_bytes; byte += destination_size)
	{
		unsigned const shift = (transfer_bytes - destination_size - byte) * 8;
		switch (destination_size)
		{
		case 1: m_cpu->m68ki_write_8_fc(channel.dar, destination_fc, data >> shift); break;
		case 2: m_cpu->m68ki_write_16_fc(channel.dar, destination_fc, data >> shift); break;
		default: m_cpu->m68ki_write_32_fc(channel.dar, destination_fc, data); break;
		}
		if (channel.ccr & CCR_DAPI)
			channel.dar += destination_size;
	}

	if (external_request && !source_request)
	{
		set_dack(channel_number, 1);
		set_done_output(channel_number, 1);
	}

	channel.btc -= transfer_bytes;
	if (!channel.btc || !channel.done_in)
	{
		set_status(channel, CSR_DONE);
	}
}


void mc68340_dma_module_device::set_dack(unsigned channel_number, int state)
{
	channel_state &channel = m_channel[channel_number];
	if (channel.dack != bool(state))
	{
		channel.dack = bool(state);
		m_dack_out_cb[channel_number](state);
	}
}


void mc68340_dma_module_device::set_done_output(unsigned channel_number, int state)
{
	channel_state &channel = m_channel[channel_number];
	if (channel.done_out != bool(state))
	{
		channel.done_out = bool(state);
		m_done_out_cb[channel_number](state);
	}
}


void mc68340_dma_module_device::set_status(channel_state &channel, uint8_t status)
{
	channel.csr |= CSR_IRQ | status;
	channel.ccr &= ~CCR_STR;
	m_cpu->update_ipl();
}


void mc68340_dma_module_device::device_start()
{
	m_cpu = downcast<m68340_cpu_device *>(owner());

	save_item(STRUCT_MEMBER(m_channel, mcr));
	save_item(STRUCT_MEMBER(m_channel, intr));
	save_item(STRUCT_MEMBER(m_channel, ccr));
	save_item(STRUCT_MEMBER(m_channel, csr));
	save_item(STRUCT_MEMBER(m_channel, fcr));
	save_item(STRUCT_MEMBER(m_channel, sar));
	save_item(STRUCT_MEMBER(m_channel, dar));
	save_item(STRUCT_MEMBER(m_channel, btc));
	save_item(STRUCT_MEMBER(m_channel, dreq));
	save_item(STRUCT_MEMBER(m_channel, done_in));
	save_item(STRUCT_MEMBER(m_channel, dack));
	save_item(STRUCT_MEMBER(m_channel, done_out));
	machine().save().register_postload(save_prepost_delegate(FUNC(mc68340_dma_module_device::restore_outputs), this));
}


void mc68340_dma_module_device::device_reset()
{
	for (channel_state &channel : m_channel)
	{
		channel = {};
		channel.mcr = 0x0080;
		channel.intr = 0x000f;
		channel.dreq = 1;
		channel.done_in = 1;
		channel.dack = 1;
		channel.done_out = 1;
	}
	restore_outputs();
}


void mc68340_dma_module_device::module_reset()
{
	for (unsigned channel_number = 0; channel_number < 2; channel_number++)
	{
		channel_state &channel = m_channel[channel_number];
		channel.ccr &= ~CCR_STR;
		channel.csr = 0;
		channel.intr = 0x000f;
		set_dack(channel_number, 1);
		set_done_output(channel_number, 1);
	}
}


void mc68340_dma_module_device::restore_outputs()
{
	for (unsigned channel_number = 0; channel_number < 2; channel_number++)
	{
		m_dack_out_cb[channel_number](m_channel[channel_number].dack);
		m_done_out_cb[channel_number](m_channel[channel_number].done_out);
	}
}


mc68340_dma_module_device::mc68340_dma_module_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MC68340_DMA_MODULE, tag, owner, clock)
	, m_cpu(nullptr)
	, m_dack_out_cb(*this)
	, m_done_out_cb(*this)
{
}
