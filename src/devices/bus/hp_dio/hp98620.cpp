// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP98620 DMA controller

***************************************************************************/

#include "emu.h"
#include "hp98620.h"

#define VERBOSE 0
#include "logmacro.h"


namespace {

class dio16_98620_device :
		public device_t,
		public bus::hp_dio::device_dio16_card_interface
{
public:
	// construction/destruction
	dio16_98620_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	dio16_98620_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	uint16_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint16_t data);

	void irq_w(int state);

private:

	static constexpr int REG0_RESET_ARM_INT = 0x00;
	static constexpr int REG0_ADDRESS_HIGH = 0x00;
	static constexpr int REG0_ADDRESS_LOW = 0x02;
	static constexpr int REG0_TRANSFER_COUNT = 0x04;
	static constexpr int REG0_CONTROL_ARM = 0x06;
	static constexpr int REG0_STATUS = 0x06;

	static constexpr int REG1_RESET_ARM_INT = 0x08;
	static constexpr int REG1_ADDRESS_HIGH = 0x08;
	static constexpr int REG1_ADDRESS_LOW = 0x0a;
	static constexpr int REG1_TRANSFER_COUNT = 0x0c;
	static constexpr int REG1_CONTROL_ARM = 0x0e;
	static constexpr int REG1_STATUS = 0x0e;

	static constexpr uint8_t REG_CONTROL_IE = 1 << 0;
	static constexpr uint8_t REG_CONTROL_WORD = 1 << 1;
	static constexpr uint8_t REG_CONTROL_OUT = 1 << 2;
	static constexpr uint8_t REG_CONTROL_PRI = 1 << 3;
	static constexpr uint8_t REG_CONTROL_INT_MASK = 0x7;
	static constexpr int REG_CONTROL_INT_SHIFT = 4;


	static constexpr uint8_t REG_STATUS_ARMED = 1 << 0;
	static constexpr uint8_t REG_STATUS_INT = 1 << 1;

	/* general control registers */
	static constexpr int REG_1TQ4_ID_LOW = 0x10;
	static constexpr int REG_1TQ4_ID_HIGH = 0x12;
	static constexpr int REG_GENERAL_CONTROL = 0x14;
	static constexpr int REG_GENERAL_CONTROL_RESET0 = 1 << 4;
	static constexpr int REG_GENERAL_CONTROL_RESET1 = 1 << 5;

	/* channel specific registers */
	static constexpr int REG0_1TQ4_ADDRESS_HIGH = 0x100;
	static constexpr int REG0_1TQ4_ADDRESS_LOW = 0x102;
	static constexpr int REG0_1TQ4_TRANSFER_COUNT_HIGH = 0x104;
	static constexpr int REG0_1TQ4_TRANSFER_COUNT_LOW = 0x106;

	static constexpr int REG0_1TQ4_CONTROL = 0x108;
	static constexpr int REG0_1TQ4_STATUS = 0x10a;

	static constexpr uint16_t REG_1TQ4_CONTROL_LWORD = 1 << 8;
	static constexpr uint16_t REG_1TQ4_CONTROL_START = 1 << 15;

	static constexpr uint16_t REG_1TQ4_STATUS_ARMED = 1 << 0;
	static constexpr uint16_t REG_1TQ4_STATUS_INT = 1 << 1;

	/* registers */
	static constexpr int REG1_1TQ4_ADDRESS_HIGH = 0x200;
	static constexpr int REG1_1TQ4_ADDRESS_LOW = 0x202;
	static constexpr int REG1_1TQ4_TRANSFER_COUNT_HIGH = 0x204;
	static constexpr int REG1_1TQ4_TRANSFER_COUNT_LOW = 0x206;

	static constexpr int REG1_1TQ4_CONTROL = 0x208;
	static constexpr int REG1_1TQ4_STATUS = 0x20a;

	void dmar0_in(int state) override;
	void dmar1_in(int state) override;

	void dma_transfer(int channel);
	void update_irq();
	void update_ctrl(const int channel, const uint16_t data, const bool is_1tq4);

	bool     m_installed_io;
	uint8_t  m_control;
	uint8_t  m_data;
	bool m_irq_state;

	struct dma_regs {
		uint32_t address = 0;
		uint32_t tc = 0;
		uint32_t control = 0;
		/* control register */
		int irq_level = 0;
		int tsz = 0;
		int subcount = 0;

		bool irq = false;
		bool ie = false;
		bool armed = false;

		bool dma_out = false;
		bool dma_pri = false; // TODO
		bool lword = false;
		bool word = false;
	} m_regs[2];

	bool m_dmar[2];
};

dio16_98620_device::dio16_98620_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98620_device(mconfig, HPDIO_98620, tag, owner, clock)
{
}

dio16_98620_device::dio16_98620_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t{mconfig, type, tag, owner, clock},
	device_dio16_card_interface{mconfig, *this},
	m_installed_io{false},
	m_control{0},
	m_data{0},
	m_irq_state{false},
	m_dmar {false}
{
}

void dio16_98620_device::device_start()
{
	m_installed_io = false;

	save_item(STRUCT_MEMBER(m_regs, address));
	save_item(STRUCT_MEMBER(m_regs, tc));
	save_item(STRUCT_MEMBER(m_regs, control));
	save_item(STRUCT_MEMBER(m_regs, irq_level));
	save_item(STRUCT_MEMBER(m_regs, tsz));
	save_item(STRUCT_MEMBER(m_regs, subcount));
	save_item(STRUCT_MEMBER(m_regs, irq));
	save_item(STRUCT_MEMBER(m_regs, ie));
	save_item(STRUCT_MEMBER(m_regs, armed));
	save_item(STRUCT_MEMBER(m_regs, dma_out));
	save_item(STRUCT_MEMBER(m_regs, dma_pri));
	save_item(STRUCT_MEMBER(m_regs, lword));
	save_item(STRUCT_MEMBER(m_regs, word));

	save_item(NAME(m_installed_io));
	save_item(NAME(m_control));
	save_item(NAME(m_data));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_dmar));
}

void dio16_98620_device::device_reset()
{
	if (!m_installed_io)
	{
		program_space().install_readwrite_handler(0x500000, 0x50020f,
				read16sm_delegate(*this, FUNC(dio16_98620_device::dma_r)),
				write16sm_delegate(*this, FUNC(dio16_98620_device::dma_w)));
		m_installed_io = true;
	}
	m_control = 0;
	m_irq_state = false;
}

uint16_t dio16_98620_device::dma_r(offs_t offset)
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
		if (m_dmar[channel])
			dma_transfer(channel);

	}
}
void dio16_98620_device::dma_w(offs_t offset, uint16_t data)
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

void dio16_98620_device::dmar0_in(int state)
{
	LOG("%s: %d\n", __FUNCTION__, state);
	m_dmar[0] = state;
	if (!state)
		return;

	dma_transfer(0);
}

void dio16_98620_device::dmar1_in(int state)
{
	LOG("%s: %d\n", __FUNCTION__, state);
	m_dmar[1] = state;

	if (!state)
		return;

	dma_transfer(1);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98620, bus::hp_dio::device_dio16_card_interface, dio16_98620_device, "hp98620", "HP98620 DMA Controller")
