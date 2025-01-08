// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Apple Computer 343S1021 PIC (Peripheral Interface Controller)

    NCR standard cell ASIC, 65CX02 CPU core

**********************************************************************/

#include "emu.h"
#include "applepic.h"

#define VERBOSE 0
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(APPLEPIC, applepic_device, "applepic", "Apple 343S1021 PIC")

const std::string_view applepic_device::s_interrupt_names[8] = { "0", "DMA 1", "DMA 2", "peripheral", "host", "timer", "6", "7" };

static constexpr int IRQ_DMA1       = 1;
static constexpr int IRQ_DMA2       = 2;
static constexpr int IRQ_PERIPHERAL = 3;
static constexpr int IRQ_HOST       = 4;
static constexpr int IRQ_TIMER      = 5;

static constexpr u8 DMAEN       = 0x01;     // 1 = channel enabled
static constexpr u8 DREQ        = 0x02;     // 1 = DREQ line for this channel is active
static constexpr u8 DMADIR      = 0x04;     // 1 = I/O to RAM, 0 = RAM to I/O
static constexpr u8 DENxONx     = 0x08;     // 1 = enable this channel when the other channel completes a transfer
static constexpr int DIOASHIFT  = 0x04;     // right shift amount for I/O offset bits

applepic_device::applepic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLEPIC, tag, owner, clock)
	, m_iopcpu(*this, "iopcpu")
	, m_prd_callback(*this, 0)
	, m_pwr_callback(*this)
	, m_hint_callback(*this)
	, m_gpin_callback(*this, 0)
	, m_gpout_callback(*this)
	, m_timer1(nullptr)
	, m_dma_timer(nullptr)
	, m_timer_last_expired(attotime::zero)
	, m_ram_address(0)
	, m_status_reg(0x80)
	, m_timer_latch(0)
	, m_scc_control(0)
	, m_io_control(0)
	, m_timer_dpll_control(0)
	, m_int_mask(0)
	, m_int_reg(0)
{
	for (dma_channel &channel : m_dma_channel)
	{
		channel.control = 0;
		channel.map = 0;
		channel.tc = 0;
		channel.req = false;
	}
}

void applepic_device::internal_map(address_map &map)
{
	map(0x0000, 0x6fff).mirror(0x8000).ram();
	map(0x7000, 0x77ff).ram();
	map(0x7800, 0x7fff).mirror(0x8000).ram();
	map(0xf010, 0xf013).rw(FUNC(applepic_device::timer_r), FUNC(applepic_device::timer_w));
	map(0xf020, 0xf02f).rw(FUNC(applepic_device::dma_channel_r), FUNC(applepic_device::dma_channel_w));
	map(0xf030, 0xf030).rw(FUNC(applepic_device::scc_control_r), FUNC(applepic_device::scc_control_w));
	map(0xf031, 0xf031).rw(FUNC(applepic_device::io_control_r), FUNC(applepic_device::io_control_w));
	map(0xf032, 0xf032).rw(FUNC(applepic_device::timer_dpll_control_r), FUNC(applepic_device::timer_dpll_control_w));
	map(0xf033, 0xf033).rw(FUNC(applepic_device::int_mask_r), FUNC(applepic_device::int_mask_w));
	map(0xf034, 0xf034).rw(FUNC(applepic_device::int_reg_r), FUNC(applepic_device::int_reg_w));
	map(0xf035, 0xf035).rw(FUNC(applepic_device::host_reg_r), FUNC(applepic_device::host_reg_w));
	map(0xf040, 0xf04f).rw(FUNC(applepic_device::device_reg_r), FUNC(applepic_device::device_reg_w));
}

void applepic_device::device_add_mconfig(machine_config &config)
{
	R65C02(config, m_iopcpu, DERIVED_CLOCK(1, 8));
	m_iopcpu->set_addrmap(AS_PROGRAM, &applepic_device::internal_map);
}

void applepic_device::device_start()
{
	// Initialize timer
	m_timer1 = timer_alloc(FUNC(applepic_device::timer1_callback), this);

	// get a timer for the DMA
	m_dma_timer = timer_alloc(FUNC(applepic_device::dma_timer_callback), this);

	// Save internal state
	save_item(NAME(m_timer_last_expired));
	save_item(NAME(m_ram_address));
	save_item(NAME(m_status_reg));
	save_item(NAME(m_timer_latch));
	save_item(STRUCT_MEMBER(m_dma_channel, control));
	save_item(STRUCT_MEMBER(m_dma_channel, map));
	save_item(STRUCT_MEMBER(m_dma_channel, tc));
	save_item(NAME(m_scc_control));
	save_item(NAME(m_io_control));
	save_item(NAME(m_timer_dpll_control));
	save_item(NAME(m_int_mask));
	save_item(NAME(m_int_reg));
}

void applepic_device::device_reset()
{
	m_status_reg &= 0xc3;
	m_int_mask = 0;

	m_iopcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_iopcpu->set_input_line(r65c02_device::IRQ_LINE, CLEAR_LINE);
	m_hint_callback(CLEAR_LINE);

	for (dma_channel &channel : m_dma_channel)
	{
		channel.control = 0;
	}

	m_dma_timer->adjust(attotime::zero, 0, clocks_to_attotime(8));
}

u8 applepic_device::host_r(offs_t offset)
{
	if (BIT(offset, 4))
	{
		if (BIT(m_scc_control, 0))
			return m_prd_callback(offset & 0x0f);
		else
		{
			if (!machine().side_effects_disabled())
				logerror("%s: Read from device register $%X in non-bypass mode\n", machine().describe_context(), offset & 0x0f);
			return 0;
		}
	}
	else if (BIT(offset, 2))
	{
		// Shared RAM read access
		u8 data = m_iopcpu->space(AS_PROGRAM).read_byte(m_ram_address);
		if (BIT(m_status_reg, 1) && !machine().side_effects_disabled())
			++m_ram_address;
		return data;
	}
	else if (BIT(offset, 1))
	{
		// PINT (D6) and /REQ (D7) are used in bypass mode only
		return BIT(m_scc_control, 0) ? m_status_reg | 0x01 : (m_status_reg & 0x3f) | 0x80;
	}
	else if (BIT(offset, 0))
		return m_ram_address & 0x00ff;
	else
		return (m_ram_address & 0xff00) >> 8;
}

void applepic_device::host_w(offs_t offset, u8 data)
{
	if (BIT(offset, 4))
	{
		if (BIT(m_scc_control, 0))
			m_pwr_callback(offset & 0x0f, data);
		else
			logerror("%s: Write $%02X to device register $%X in non-bypass mode\n", machine().describe_context(), data, offset & 0x0f);
	}
	else if (BIT(offset, 2))
	{
		// Shared RAM write access
		m_iopcpu->space(AS_PROGRAM).write_byte(m_ram_address, data);
		if (BIT(m_status_reg, 1))
			++m_ram_address;
	}
	else if (BIT(offset, 1))
	{
		if (BIT(m_status_reg, 2) != BIT(data, 2))
		{
			LOG("%s: /RSTPIC %dactive\n", machine().describe_context(), BIT(data, 2) ? "in" : "");
			m_iopcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 2) ? CLEAR_LINE : ASSERT_LINE);
		}
		if (BIT(data, 3))
			set_interrupt(IRQ_HOST);
		if ((m_status_reg & data & 0x30) != 0)
		{
			m_status_reg &= ~(data & 0x30);
			if ((m_status_reg & 0x30) == 0)
			{
				LOG("%s: Host interrupts acknowledged\n", machine().describe_context());
				m_hint_callback(CLEAR_LINE);
			}
		}
		m_status_reg = (data & 0x06) | (m_status_reg & 0xf0);
	}
	else if (BIT(offset, 0))
		m_ram_address = (m_ram_address & 0xff00) | data;
	else
		m_ram_address = u16(data) << 8 | (m_ram_address & 0x00ff);
}

void applepic_device::pint_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_status_reg |= 0x40;
		if (!BIT(m_scc_control, 0))
			set_interrupt(IRQ_PERIPHERAL);
	}
	else
	{
		m_status_reg &= 0xbf;
		if (!BIT(m_scc_control, 0))
			reset_interrupt(IRQ_PERIPHERAL);
	}
}

void applepic_device::reqa_w(int state)
{
	m_dma_channel[0].req = state;
	if (state)
	{
		m_dma_channel[0].control |= DREQ;
	}
	else
	{
		m_dma_channel[0].control &= ~DREQ;
	}
}

void applepic_device::reqb_w(int state)
{
	m_dma_channel[1].req = state;
	if (state)
	{
		m_dma_channel[1].control |= DREQ;
	}
	else
	{
		m_dma_channel[1].control &= ~DREQ;
	}
}

u8 applepic_device::timer_r(offs_t offset)
{
	u16 reg = BIT(offset, 1) ? m_timer_latch : get_timer_count();
	if (BIT(offset, 0))
		return (reg & 0xff00) >> 8;
	else
	{
		if (offset == 0 && !machine().side_effects_disabled())
			reset_interrupt(IRQ_TIMER);
		return reg & 0x00ff;
	}
}

void applepic_device::timer_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
	{
		m_timer_latch = u16(data) << 8 | (m_timer_latch & 0x00ff);
		if (offset == 1)
		{
			reset_interrupt(IRQ_TIMER);
			m_timer1->adjust(clocks_to_attotime(m_timer_latch * 8 + 12));
		}
	}
	else
		m_timer_latch = (m_timer_latch & 0xff00) | data;
}

u16 applepic_device::get_timer_count() const
{
	if (m_timer1->enabled())
		return u16((attotime_to_clocks(m_timer1->remaining()) - 4) / 8);
	else
		return 0xffff - u16((attotime_to_clocks(machine().time() - m_timer_last_expired) + 4) / 8);
}

TIMER_CALLBACK_MEMBER(applepic_device::timer1_callback)
{
	set_interrupt(IRQ_TIMER);
	if (BIT(m_timer_dpll_control, 0))
		m_timer1->adjust(clocks_to_attotime((m_timer_latch + 2) * 8));
	else
		m_timer_last_expired = machine().time();
}

u8 applepic_device::dma_channel_r(offs_t offset)
{
	dma_channel &channel = m_dma_channel[BIT(offset, 3)];

	switch (offset & 7)
	{
	case 0:
		return channel.control;

	case 1:
		return channel.map & 0x00ff;

	case 2:
		return (channel.map & 0xff00) >> 8;

	case 3:
		return channel.tc & 0x0ff;

	case 4:
		return (channel.tc & 0x700) >> 8;

	default:
		if (!machine().side_effects_disabled())
			logerror("%s: Read from undefined DMA register $%02X\n", machine().describe_context(), 0x20 + offset);
		return 0;
	}
}

void applepic_device::dma_channel_w(offs_t offset, u8 data)
{
	dma_channel &channel = m_dma_channel[BIT(offset, 3)];

	switch (offset & 7)
	{
	case 0:
		channel.control = ((data & ~DREQ) | (channel.control & DREQ));
		break;

	case 1:
		channel.map = (channel.map & 0xff00) | data;
		break;

	case 2:
		channel.map = u16(data) << 8 | (channel.map & 0x00ff);
		break;

	case 3:
		channel.tc = (channel.tc & 0x700) | data;
		break;

	case 4:
		channel.tc = ((data & 0x07) << 8) | (channel.tc & 0x0ff);
		break;

	default:
		logerror("%s: Write $%02X to undefined DMA register $%02X\n", machine().describe_context(), data, 0x20 + offset);
		break;
	}
}

TIMER_CALLBACK_MEMBER(applepic_device::dma_timer_callback)
{
	for (int ch = 0; ch < 2; ch++)
	{
		auto &channel = m_dma_channel[ch];
		auto other_channel = m_dma_channel[ch ^ 1];

		if ((channel.control & DMAEN) && (channel.req) && (channel.tc > 0))
		{
			if (channel.control & DMADIR)
			{
				const u8 xfer = m_prd_callback(channel.control >> DIOASHIFT);
				m_iopcpu->space(AS_PROGRAM).write_byte(channel.map, xfer);
			}
			else
			{
				const u8 xfer = m_iopcpu->space(AS_PROGRAM).read_byte(channel.map);
				m_pwr_callback(channel.control >> DIOASHIFT, xfer);
			}

			channel.map++;
			channel.tc--;

			// if this channel completed, handle the DEN1ON2/DEN2ON1 alternating transfer bits and IRQ
			if (channel.tc == 0)
			{
				channel.control &= ~DMAEN;
				if (other_channel.control & DENxONx)
				{
					other_channel.control |= DMAEN;
				}

				set_interrupt(IRQ_DMA1 + ch);
			}
		}
	}
}

u8 applepic_device::scc_control_r()
{
	return m_scc_control;
}

void applepic_device::scc_control_w(u8 data)
{
	m_scc_control = data;
	if (!BIT(data, 0) && BIT(m_status_reg, 6))
		set_interrupt(IRQ_PERIPHERAL);
	else
		reset_interrupt(IRQ_PERIPHERAL);
	m_gpout_callback[1](BIT(data, 7));
}

u8 applepic_device::io_control_r()
{
	return m_io_control;
}

void applepic_device::io_control_w(u8 data)
{
	m_io_control = data;
}

u8 applepic_device::timer_dpll_control_r()
{
	return m_timer_dpll_control | (m_gpin_callback() & 3) << 2;
}

void applepic_device::timer_dpll_control_w(u8 data)
{
	m_timer_dpll_control = (m_timer_dpll_control & 0xa0) | (data & 0x53);
	m_gpout_callback[0](BIT(data, 1));
}

u8 applepic_device::int_mask_r()
{
	return m_int_mask;
}

void applepic_device::int_mask_w(u8 data)
{
	for (int which = 1; which <= 5; which++)
		if (BIT(m_int_mask, which) != BIT(data, which))
			LOG("%s: %sabling %s interrupt\n", machine().describe_context(), BIT(data, which) ? "En" : "Dis", s_interrupt_names[which]);

	m_int_mask = data & 0x3e;
	m_iopcpu->set_input_line(r65c02_device::IRQ_LINE, (m_int_mask & m_int_reg) != 0 ? ASSERT_LINE : CLEAR_LINE);
}

u8 applepic_device::int_reg_r()
{
	return m_int_reg;
}

void applepic_device::int_reg_w(u8 data)
{
	if ((m_int_reg & data) != 0)
	{
		m_int_reg &= ~data;
		if ((m_int_mask & m_int_reg) == 0)
		{
			LOG("%s: 6502 interrupts acknowledged\n", machine().describe_context());
			m_iopcpu->set_input_line(r65c02_device::IRQ_LINE, CLEAR_LINE);
		}
	}
}

void applepic_device::set_interrupt(int which)
{
	if (!BIT(m_int_reg, which))
	{
		LOG("%s: Setting %s interrupt\n", machine().describe_context(), s_interrupt_names[which]);
		m_int_reg |= (1 << which);
		if ((m_int_reg & m_int_mask) == (1 << which))
			m_iopcpu->set_input_line(r65c02_device::IRQ_LINE, ASSERT_LINE);
	}
}

void applepic_device::reset_interrupt(int which)
{
	if (BIT(m_int_reg, which))
	{
		LOG("%s: Resetting %s interrupt\n", machine().describe_context(), s_interrupt_names[which]);
		if ((m_int_reg & m_int_mask) == (1 << which))
			m_iopcpu->set_input_line(r65c02_device::IRQ_LINE, CLEAR_LINE);
		m_int_reg &= ~(1 << which);
	}
}

u8 applepic_device::host_reg_r()
{
	return (m_status_reg & 0x30) >> 2;
}

void applepic_device::host_reg_w(u8 data)
{
	LOG("%s: INTHST0 %srequested, INTHST1 %srequested\n", machine().describe_context(), BIT(data, 2) ? "" : "not ", BIT(data, 3) ? "" : "not ");
	m_status_reg |= (data & 0x0c) << 2;
	if ((data & 0x0c) != 0)
		m_hint_callback(ASSERT_LINE);
}

u8 applepic_device::device_reg_r(offs_t offset)
{
	if (BIT(m_scc_control, 0))
	{
		if (!machine().side_effects_disabled())
			logerror("%s: Read from device register $%X in bypass mode\n", machine().describe_context(), offset & 0x0f);
		return 0;
	}
	else
		return m_prd_callback(offset & 0x0f);
}

void applepic_device::device_reg_w(offs_t offset, u8 data)
{
	if (BIT(m_scc_control, 0))
		logerror("%s: Read from device register $%X in bypass mode\n", machine().describe_context(), offset & 0x0f);
	else
		m_pwr_callback(offset & 0x0f, data);
}
