// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP98620 DMA controller

***************************************************************************/

#include "emu.h"
#include "hp98620.h"

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE_NS(HPDIO_98620, bus::hp_dio, dio16_98620_device, "hp98620", "HP98620 DMA Controller")

namespace bus {
	namespace hp_dio {

dio16_98620_device::dio16_98620_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98620_device(mconfig, HPDIO_98620, tag, owner, clock)
{
}

dio16_98620_device::dio16_98620_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio32_card_interface(mconfig, *this),
	m_irq_state(false)
{
}

void dio16_98620_device::device_start()
{
	m_installed_io = false;

	save_item(NAME(m_regs[0].address));
	save_item(NAME(m_regs[0].tc));
	save_item(NAME(m_regs[0].control));
	save_item(NAME(m_regs[0].irq_level));
	save_item(NAME(m_regs[0].tsz));
	save_item(NAME(m_regs[0].subcount));
	save_item(NAME(m_regs[0].irq));
	save_item(NAME(m_regs[0].ie));
	save_item(NAME(m_regs[0].armed));
	save_item(NAME(m_regs[0].dma_out));
	save_item(NAME(m_regs[0].dma_pri));
	save_item(NAME(m_regs[0].lword));
	save_item(NAME(m_regs[0].word));


	save_item(NAME(m_regs[1].address));
	save_item(NAME(m_regs[1].tc));
	save_item(NAME(m_regs[1].control));
	save_item(NAME(m_regs[1].irq_level));
	save_item(NAME(m_regs[1].tsz));
	save_item(NAME(m_regs[1].subcount));
	save_item(NAME(m_regs[1].irq));
	save_item(NAME(m_regs[1].ie));
	save_item(NAME(m_regs[1].armed));
	save_item(NAME(m_regs[1].dma_out));
	save_item(NAME(m_regs[1].dma_pri));
	save_item(NAME(m_regs[1].lword));
	save_item(NAME(m_regs[1].word));

	save_item(NAME(m_installed_io));
	save_item(NAME(m_control));
	save_item(NAME(m_data));
	save_item(NAME(m_irq_state));
	save_item(NAME(dmar));
}

void dio16_98620_device::device_reset()
{
	if (!m_installed_io)
	{
		program_space().install_readwrite_handler(0x500000, 0x50020f,
				read16_delegate(FUNC(dio16_98620_device::dma_r), this),
				write16_delegate(FUNC(dio16_98620_device::dma_w), this));
		m_installed_io = true;
	}
	m_control = 0;
	m_irq_state = false;
}

uint16_t dio16_98620_device::get_ctrl(const int channel)
{
	uint16_t ret = 0;

	if (m_regs[channel].ie)
		ret |= REG_CONTROL_IE;
	if (m_regs[channel].tsz == 2)
		ret |= REG_CONTROL_WORD;
	if (m_regs[channel].tsz == 4)
		ret |= REG_1TQ4_CONTROL_LWORD;
	if (m_regs[channel].dma_out)
		ret |= REG_CONTROL_OUT;
	if (m_regs[channel].dma_pri)
		ret |= REG_CONTROL_PRI;

	ret |= ((m_regs[channel].irq_level-3) & REG_CONTROL_INT_MASK) << REG_CONTROL_INT_SHIFT;

	return ret;
}

READ16_MEMBER(dio16_98620_device::dma_r)
{

	uint16_t ret = 0;

	switch(offset << 1) {
	case REG0_RESET_ARM_INT:
		m_regs[0].irq = false;
		m_regs[0].armed = false;
		update_irq();
		ret = 0x00;
		break;
	case REG1_RESET_ARM_INT:
		m_regs[1].irq = false;
		m_regs[1].armed = false;
		update_irq();
		ret = 0x00;
		break;
	case REG0_TRANSFER_COUNT:
		ret = m_regs[0].tc & 0xffff;
		break;
	case REG1_TRANSFER_COUNT:
		ret = m_regs[1].tc & 0xffff;
		break;
	case REG0_STATUS:
		ret = 0;
		if (m_regs[0].armed)
			ret |= REG_STATUS_ARMED;
		if (m_regs[0].irq)
			ret |= REG_STATUS_INT;
		break;
	case REG1_STATUS:
		ret = 0;
		if (m_regs[1].armed)
			ret |= REG_STATUS_ARMED;
		if (m_regs[1].irq)
			ret |= REG_STATUS_INT;
		break;
	case REG_1TQ4_ID_LOW:
		ret = 0x3230;
		break;
	case REG_1TQ4_ID_HIGH:
		ret = 0x4330;
		break;
	case REG0_1TQ4_ADDRESS_LOW:
		ret = m_regs[0].address & 0xffff;
		break;
	case REG0_1TQ4_ADDRESS_HIGH:
		ret = m_regs[0].address >> 16;
		break;
	case REG1_1TQ4_ADDRESS_LOW:
		ret = m_regs[1].address & 0xffff;
		break;
	case REG1_1TQ4_ADDRESS_HIGH:
		ret = m_regs[1].address >> 16;
		break;

	case REG0_1TQ4_TRANSFER_COUNT_LOW:
		ret = m_regs[0].tc & 0xffff;
		break;
	case REG0_1TQ4_TRANSFER_COUNT_HIGH:
		ret = m_regs[0].tc >> 16;
		break;
	case REG1_1TQ4_TRANSFER_COUNT_LOW:
		ret = m_regs[1].tc & 0xffff;
		break;
	case REG1_1TQ4_TRANSFER_COUNT_HIGH:
		ret = m_regs[1].tc >> 16;
		break;
	case REG0_1TQ4_CONTROL:
		ret = m_regs[0].control;
		break;
	case REG1_1TQ4_CONTROL:
		ret = m_regs[1].control;
		break;
	case REG0_1TQ4_STATUS:
		ret = 0;
		if (m_regs[0].armed)
			ret |= REG_1TQ4_STATUS_ARMED;
		if (m_regs[0].irq)
			ret |= REG_1TQ4_STATUS_INT;
		break;
	case REG1_1TQ4_STATUS:
		ret = 0;
		if (m_regs[1].armed)
			ret |= REG_1TQ4_STATUS_ARMED;
		if (m_regs[1].irq)
			ret |= REG_1TQ4_STATUS_INT;

		break;
	default:
		LOG("%s: unknown register read: %02X\n", __FUNCTION__, offset << 1);
		break;
	}
	LOG("dma_r: offset=%02X ret=%02X\n", offset << 1, ret);
	return ret;
}

void dio16_98620_device::update_ctrl(const int channel, const uint16_t data, const bool is_1tq4)
{
	assert(channel < 2);

	m_regs[channel].control = data & ~REG_1TQ4_CONTROL_START;
	m_regs[channel].ie = data & REG_CONTROL_IE;
	m_regs[channel].irq_level = 3 + ((data >> REG_CONTROL_INT_SHIFT) & REG_CONTROL_INT_MASK);
	m_regs[channel].lword = (data & REG_1TQ4_CONTROL_LWORD) && is_1tq4;
	m_regs[channel].word = data & REG_CONTROL_WORD;
	m_regs[channel].dma_out = data & REG_CONTROL_OUT;
	if (data & REG_1TQ4_CONTROL_LWORD)
		m_regs[channel].tsz = 4;
	else if (data & REG_CONTROL_WORD)
		m_regs[channel].tsz = 2;
	else
		m_regs[channel].tsz = 1;

	if ((data & REG_1TQ4_CONTROL_START) || !is_1tq4) {
		LOG("DMA Channel %d: addr %08x, tc %d, size %d, int %sabled on IRQ %d\n", channel,
				m_regs[channel].address,
				(m_regs[channel].tc+1)*m_regs[channel].tsz,
				m_regs[channel].tsz,
				m_regs[channel].ie ? "en" : "dis",
				m_regs[channel].irq_level);
		m_regs[channel].subcount = m_regs[channel].tsz-1;
		m_regs[channel].armed = true;
		m_regs[channel].irq = false;
		if (dmar[channel])
			dma_transfer(channel);

	}
}
WRITE16_MEMBER(dio16_98620_device::dma_w)
{
	LOG("dma_w: offset=%02X, data=%02X\n", offset << 1, data);

	switch(offset << 1) {
	case REG0_1TQ4_ADDRESS_HIGH:
	case REG0_ADDRESS_HIGH:
		m_regs[0].address &= 0xffff;
		m_regs[0].address |= (data << 16);
		break;
	case REG0_1TQ4_ADDRESS_LOW:
	case REG0_ADDRESS_LOW:
		m_regs[0].address &= 0xffff0000;
		m_regs[0].address |= data;
		break;
	case REG1_1TQ4_ADDRESS_HIGH:
	case REG1_ADDRESS_HIGH:
		m_regs[1].address &= 0xffff;
		m_regs[1].address |= (data << 16);
		break;
	case REG1_1TQ4_ADDRESS_LOW:
	case REG1_ADDRESS_LOW:
		m_regs[1].address &= 0xffff0000;
		m_regs[1].address |= data;
		break;
	case REG0_TRANSFER_COUNT:
		m_regs[0].tc = data;
		break;
	case REG0_1TQ4_TRANSFER_COUNT_HIGH:
		m_regs[0].tc &= 0xffff;
		m_regs[0].tc |= (data << 16);
		break;
	case REG0_1TQ4_TRANSFER_COUNT_LOW:
		m_regs[0].tc &= 0xffff0000;
		m_regs[0].tc |= data;
		break;
	case REG1_1TQ4_TRANSFER_COUNT_HIGH:
		m_regs[1].tc &= 0xffff;
		m_regs[1].tc |= (data << 16);
		break;
	case REG1_1TQ4_TRANSFER_COUNT_LOW:
		m_regs[1].tc &= 0xffff0000;
		m_regs[1].tc |= data;
		break;
	case REG1_TRANSFER_COUNT:
		m_regs[1].tc = data;
		break;

	case REG0_CONTROL_ARM:
		update_ctrl(0, data, false);
		break;

	case REG1_CONTROL_ARM:
		update_ctrl(1, data, false);
		break;

	case REG_GENERAL_CONTROL:
		if (data & REG_GENERAL_CONTROL_RESET0) {
			m_regs[0].armed = 0;
		}

		if (data & REG_GENERAL_CONTROL_RESET1) {
			m_regs[1].armed = 0;
		}
		break;

	case REG0_1TQ4_CONTROL:
		update_ctrl(0, data, true);
		break;

	case REG1_1TQ4_CONTROL:
		update_ctrl(1, data, true);
		break;

	default:
		LOG("%s: unknown register write: %02X\n", __FUNCTION__, offset << 1);
		break;
	}
}

void dio16_98620_device::update_irq()
{
	irq3_out((m_regs[0].irq_level == 3 && m_regs[0].irq && m_regs[0].ie) ||
			(m_regs[1].irq_level == 3 && m_regs[1].irq && m_regs[1].ie));

	irq4_out((m_regs[0].irq_level == 4 && m_regs[0].irq && m_regs[0].ie) ||
			(m_regs[1].irq_level == 4 && m_regs[1].irq && m_regs[1].ie));

	irq5_out((m_regs[0].irq_level == 5 && m_regs[0].irq && m_regs[0].ie) ||
			(m_regs[1].irq_level == 5 && m_regs[1].irq && m_regs[1].ie));

	irq6_out((m_regs[0].irq_level == 6 && m_regs[0].irq && m_regs[0].ie) ||
			(m_regs[1].irq_level == 6 && m_regs[1].irq && m_regs[1].ie));

	irq7_out((m_regs[0].irq_level == 7 && m_regs[0].irq && m_regs[0].ie) ||
			(m_regs[1].irq_level == 7 && m_regs[1].irq && m_regs[1].ie));
}

void dio16_98620_device::dma_transfer(int channel)
{
	assert(channel < 2);

	if (!(m_regs[channel].armed))
		return;

	LOG("dma_transfer %s: tc %d/%d\n", m_regs[channel].dma_out ? "out" : "in",
			m_regs[channel].tc, m_regs[channel].subcount);


	if (m_regs[channel].dma_out) {
			dmack_w_out(channel, program_space().read_byte(m_regs[channel].address++));
	} else {
			program_space().write_byte(m_regs[channel].address++, dmack_r_out(channel));

	}

	if (m_regs[channel].subcount == 0) {
		if (m_regs[channel].tc-- == 0) {
			LOG("DMA%d done\n", channel);
			m_regs[channel].armed = false;
			if (m_regs[channel].ie) {
				m_regs[channel].irq = true;
				update_irq();
			}
			return;
		}

		m_regs[channel].subcount = m_regs[channel].tsz;
	}

	m_regs[channel].subcount--;
}

WRITE_LINE_MEMBER(dio16_98620_device::dmar0_in)
{
	LOG("%s: %d\n", __FUNCTION__, state);
	dmar[0] = state;
	if (!state)
		return;

	dma_transfer(0);
}

WRITE_LINE_MEMBER(dio16_98620_device::dmar1_in)
{
	LOG("%s: %d\n", __FUNCTION__, state);
	dmar[1] = state;

	if (!state)
		return;

	dma_transfer(1);
}

} // namespace bus::hp_dio
} // namespace bus

