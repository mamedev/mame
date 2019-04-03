// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the Intel 82357 Integrated System Peripheral.
 *
 * This device was one part of the 3-chip Intel 82350 EISA bus chipset.
 *
 * Sources:
 *
 *   http://bitsavers.org/components/intel/_dataBooks/1996_Intel_Peripheral_Components.pdf
 *
 * TODO
 *   - expose everything to an actual EISA bus
 *   - 32-bit dma functionality
 *
 */

#include "emu.h"
#include "i82357.h"

#define LOG_GENERAL (1U << 0)

//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(I82357, i82357_device, "i82357", "Intel 82357 Integrated System Peripheral")

i82357_device::i82357_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, I82357, tag, owner, clock)
	, m_pic(*this, "pic%u", 0)
	, m_pit(*this, "pit%u", 0)
	, m_dma(*this, "dma%u", 0)
	, m_out_rtc(*this)
	, m_out_nmi(*this)
	, m_out_spkr(*this)
{
}

void i82357_device::device_add_mconfig(machine_config &config)
{
	PIC8259(config, m_pic[0], 0);
	m_pic[0]->in_sp_callback().set_constant(1);
	m_pic[0]->read_slave_ack_callback().set(
		[this](offs_t offset)
		{
			if (offset == 2)
				return m_pic[1]->acknowledge();

			return u32(0);
		});

	PIC8259(config, m_pic[1], 0);
	m_pic[1]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir2_w));
	m_pic[1]->in_sp_callback().set_constant(0);

	PIT8254(config, m_pit[0], 0);
	PIT8254(config, m_pit[1], 0);

	// timer 1 counter 0: system timer
	m_pit[0]->set_clk<0>(clock() / 12);
	m_pit[0]->out_handler<0>().set(m_pic[0], FUNC(pic8259_device::ir0_w));

	// timer 1 counter 1: refresh request
	m_pit[0]->set_clk<1>(clock() / 12);
	m_pit[0]->out_handler<1>().set(
		[this](int state)
		{
			// FIXME: not accurate, but good enough to pass diagnostic
			if (state)
				m_nmi_reg ^= NMI_REFRESH;

			m_pit[1]->write_gate2(state);
		});

	// timer 1 counter 2: speaker
	m_pit[0]->set_clk<2>(clock() / 12);
	m_pit[0]->out_handler<2>().set(
		[this](int state)
		{
			if (state)
				m_nmi_reg |= NMI_T1C2_STATE;
			else
				m_nmi_reg &= ~NMI_T1C2_STATE;

			m_out_spkr((m_nmi_reg & NMI_T1C2_STATE) && (m_nmi_reg & NMI_SPEAKER_DATA));
		});

	// timer 2 counter 0: fail-safe timer
	m_pit[1]->set_clk<0>(clock() / 48);
	m_pit[1]->out_handler<0>().set(
		[this](int state)
		{
			if (state && (m_nmi_ext & NMI_EXT_EN_FAILSAFE))
			{
				m_nmi_ext |= NMI_EXT_FAILSAFE;

				m_nmi_check->adjust(attotime::zero);
			}
		});

	// timer 2 counter 1: does not exist

	// timer 2 counter 2: cpu speed control
	m_pit[1]->set_clk<2>(8_MHz_XTAL);
	//m_pit[1]->out_handler<2>().set();

	EISA_DMA(config, m_dma[0], clock() / 3);
	EISA_DMA(config, m_dma[1], clock() / 3);
	m_dma[1]->out_dack_callback<0>().set(m_dma[0], FUNC(eisa_dma_device::hack_w)).invert();
}

void i82357_device::map(address_map &map)
{
	map(0x000, 0x00f).rw(m_dma[0], FUNC(eisa_dma_device::read), FUNC(eisa_dma_device::write));
	map(0x020, 0x021).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x040, 0x043).rw(m_pit[0], FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x048, 0x04b).rw(m_pit[1], FUNC(pit8254_device::read), FUNC(pit8254_device::write));

	// FIXME: also 0x065 - "NMI Status"
	map(0x061, 0x061).rw(FUNC(i82357_device::nmi_reg_r), FUNC(i82357_device::nmi_reg_w));

	// NMI Enable Register (0x70, 72, 74, 76)
	map(0x070, 0x077).lw8("nmi_rtc",
		[this](address_space &space, offs_t offset, u8 data)
		{
			m_nmi_enabled = !BIT(data, 7);

			m_out_rtc(space, 0, data & 0x7f);

			m_nmi_check->adjust(attotime::zero);
		}).umask64(0xff);

	map(0x081, 0x081).rw(m_dma[0], FUNC(eisa_dma_device::get_address_page<2>), FUNC(eisa_dma_device::set_address_page<2>));
	map(0x082, 0x082).rw(m_dma[0], FUNC(eisa_dma_device::get_address_page<3>), FUNC(eisa_dma_device::set_address_page<3>));
	map(0x083, 0x083).rw(m_dma[0], FUNC(eisa_dma_device::get_address_page<1>), FUNC(eisa_dma_device::set_address_page<1>));
	map(0x087, 0x087).rw(m_dma[0], FUNC(eisa_dma_device::get_address_page<0>), FUNC(eisa_dma_device::set_address_page<0>));
	map(0x089, 0x089).rw(m_dma[1], FUNC(eisa_dma_device::get_address_page<2>), FUNC(eisa_dma_device::set_address_page<2>));
	map(0x08a, 0x08a).rw(m_dma[1], FUNC(eisa_dma_device::get_address_page<3>), FUNC(eisa_dma_device::set_address_page<3>));
	map(0x08b, 0x08b).rw(m_dma[1], FUNC(eisa_dma_device::get_address_page<1>), FUNC(eisa_dma_device::set_address_page<1>));
	map(0x08f, 0x08f).rw(m_dma[1], FUNC(eisa_dma_device::get_address_page<0>), FUNC(eisa_dma_device::set_address_page<0>));

	// TODO: also 0xa4, a8, ac, b0, b4, b8, bc
	map(0x0a0, 0x0a1).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0c0, 0x0df).rw(m_dma[1], FUNC(eisa_dma_device::read), FUNC(eisa_dma_device::write)).umask16(0x00ff);

	map(0x401, 0x401).rw(m_dma[0], FUNC(eisa_dma_device::get_count_high<0>), FUNC(eisa_dma_device::set_count_high<0>));
	map(0x403, 0x403).rw(m_dma[0], FUNC(eisa_dma_device::get_count_high<1>), FUNC(eisa_dma_device::set_count_high<1>));
	map(0x405, 0x405).rw(m_dma[0], FUNC(eisa_dma_device::get_count_high<2>), FUNC(eisa_dma_device::set_count_high<2>));
	map(0x407, 0x407).rw(m_dma[0], FUNC(eisa_dma_device::get_count_high<3>), FUNC(eisa_dma_device::set_count_high<3>));

	//map(0x40a, 0x40a); // DMA1 Set Chaining Mode (w)/Interrupt Status (r)
	//map(0x40b, 0x40b); // DMA1 Ext Write Mode (w)
	//map(0x40c, 0x40c); // Chain Buffer Control
	//map(0x40d, 0x40d); // Stepping Level Register (ro)
	//map(0x40e, 0x40e); // ISP Test Register
	//map(0x40f, 0x40f); // ISP Test Register

	map(0x461, 0x461).rw(FUNC(i82357_device::nmi_ext_r), FUNC(i82357_device::nmi_ext_w));
	map(0x462, 0x462).lw8("nmi_ioport",
		[this](u8 data)
		{
			if (m_nmi_ext & NMI_EXT_EN_IOPORT)
			{
				m_nmi_ext |= NMI_EXT_IOPORT;

				m_nmi_check->adjust(attotime::zero);
			}
		});
	//map(0x464, 0x464); // Last 32-bit bus master granted (L)

	map(0x481, 0x481).rw(m_dma[0], FUNC(eisa_dma_device::get_address_page_high<2>), FUNC(eisa_dma_device::set_address_page_high<2>));
	map(0x482, 0x482).rw(m_dma[0], FUNC(eisa_dma_device::get_address_page_high<3>), FUNC(eisa_dma_device::set_address_page_high<3>));
	map(0x483, 0x483).rw(m_dma[0], FUNC(eisa_dma_device::get_address_page_high<1>), FUNC(eisa_dma_device::set_address_page_high<1>));
	map(0x487, 0x487).rw(m_dma[0], FUNC(eisa_dma_device::get_address_page_high<0>), FUNC(eisa_dma_device::set_address_page_high<0>));
	map(0x489, 0x489).rw(m_dma[1], FUNC(eisa_dma_device::get_address_page_high<2>), FUNC(eisa_dma_device::set_address_page_high<2>));
	map(0x48a, 0x48a).rw(m_dma[1], FUNC(eisa_dma_device::get_address_page_high<3>), FUNC(eisa_dma_device::set_address_page_high<3>));
	map(0x48b, 0x48b).rw(m_dma[1], FUNC(eisa_dma_device::get_address_page_high<1>), FUNC(eisa_dma_device::set_address_page_high<1>));
	map(0x48f, 0x48f).rw(m_dma[1], FUNC(eisa_dma_device::get_address_page_high<0>), FUNC(eisa_dma_device::set_address_page_high<0>));

	map(0x4c2, 0x4c2).rw(m_dma[1], FUNC(eisa_dma_device::get_count_high<0>), FUNC(eisa_dma_device::set_count_high<0>));
	map(0x4c6, 0x4c6).rw(m_dma[1], FUNC(eisa_dma_device::get_count_high<1>), FUNC(eisa_dma_device::set_count_high<1>));
	map(0x4ca, 0x4ca).rw(m_dma[1], FUNC(eisa_dma_device::get_count_high<2>), FUNC(eisa_dma_device::set_count_high<2>));
	map(0x4ce, 0x4ce).rw(m_dma[1], FUNC(eisa_dma_device::get_count_high<3>), FUNC(eisa_dma_device::set_count_high<3>));
	map(0x4d0, 0x4d1).lrw8("elcr",
		[this](offs_t offset) { return m_elcr[offset]; },
		[this](offs_t offset, u8 data) { m_elcr[offset] = data; });

	//map(0x4d4, 0x4d4); // DMA2 Set Chaining Mode
	//map(0x4d6, 0x4d6); // DMA2 Ext Write Mode Register

	map(0x4e0, 0x4e3).rw(m_dma[0], FUNC(eisa_dma_device::get_stop<0>), FUNC(eisa_dma_device::set_stop<0>));
	map(0x4e4, 0x4e7).rw(m_dma[0], FUNC(eisa_dma_device::get_stop<1>), FUNC(eisa_dma_device::set_stop<1>));
	map(0x4e8, 0x4eb).rw(m_dma[0], FUNC(eisa_dma_device::get_stop<2>), FUNC(eisa_dma_device::set_stop<2>));
	map(0x4ec, 0x4ef).rw(m_dma[0], FUNC(eisa_dma_device::get_stop<3>), FUNC(eisa_dma_device::set_stop<3>));

	map(0x4f0, 0x4f3).rw(m_dma[1], FUNC(eisa_dma_device::get_stop<0>), FUNC(eisa_dma_device::set_stop<0>)); // reserved
	map(0x4f4, 0x4f7).rw(m_dma[1], FUNC(eisa_dma_device::get_stop<1>), FUNC(eisa_dma_device::set_stop<1>));
	map(0x4f8, 0x4fb).rw(m_dma[1], FUNC(eisa_dma_device::get_stop<2>), FUNC(eisa_dma_device::set_stop<2>));
	map(0x4fc, 0x4ff).rw(m_dma[1], FUNC(eisa_dma_device::get_stop<3>), FUNC(eisa_dma_device::set_stop<3>));
}

void i82357_device::device_start()
{
	m_out_rtc.resolve_safe();
	m_out_nmi.resolve_safe();
	m_out_spkr.resolve_safe();

	m_nmi_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(i82357_device::nmi_check), this));
}

void i82357_device::device_reset()
{
	m_elcr[0] = 0;
	m_elcr[1] = 0;

	m_nmi_enabled = true;
	m_nmi_reg = 0;
	m_nmi_ext = 0;

	m_nmi_check->adjust(attotime::zero);
}

void i82357_device::nmi_reg_w(u8 data)
{
	m_pit[0]->write_gate2(!!(data & NMI_T1C2_GATE));

	// clear parity nmi
	if (data & NMI_PARITY_DISABLE)
		m_nmi_reg &= ~NMI_PARITY;

	// clear iochk nmi
	if (data & NMI_IOCHK_DISABLE)
		m_nmi_reg &= ~NMI_IOCHK;

	m_nmi_reg = (m_nmi_reg & ~NMI_WMASK) | (data & NMI_WMASK);

	// update speaker state
	m_out_spkr((m_nmi_reg & NMI_T1C2_STATE) && (m_nmi_reg & NMI_SPEAKER_DATA));

	m_nmi_check->adjust(attotime::zero);
}

void i82357_device::nmi_ext_w(u8 data)
{
	if (!(data & NMI_EXT_EN_IOPORT))
		m_nmi_ext &= ~NMI_EXT_IOPORT;

	if (!(data & NMI_EXT_EN_TIMEOUT))
		m_nmi_ext &= ~NMI_EXT_TIMEOUT;

	if (!(data & NMI_EXT_EN_FAILSAFE))
		m_nmi_ext &= ~NMI_EXT_FAILSAFE;

	m_nmi_ext = (m_nmi_ext & ~NMI_EXT_WMASK) | (data & NMI_EXT_WMASK);

	m_nmi_check->adjust(attotime::zero);
}

WRITE_LINE_MEMBER(i82357_device::in_iochk)
{
	if (!state && !(m_nmi_reg & NMI_PARITY_DISABLE))
	{
		m_nmi_reg |= NMI_IOCHK;

		m_nmi_check->adjust(attotime::zero);
	}
	else
		m_nmi_reg &= ~NMI_IOCHK;
}

WRITE_LINE_MEMBER(i82357_device::in_parity)
{
	if (!state && !(m_nmi_reg & NMI_IOCHK_DISABLE))
	{
		m_nmi_reg |= NMI_PARITY;

		m_nmi_check->adjust(attotime::zero);
	}
	else
		m_nmi_reg &= ~NMI_PARITY;
}

TIMER_CALLBACK_MEMBER(i82357_device::nmi_check)
{
	if (m_nmi_enabled)
	{
		if ((m_nmi_reg & NMI_NMI) || (m_nmi_ext & NMI_EXT_NMI))
		{
			if (!m_out_nmi_asserted)
			{
				m_out_nmi_asserted = true;
				m_out_nmi(1);
			}

			return;
		}
	}

	// disabled or no source
	if (m_out_nmi_asserted)
	{
		m_out_nmi_asserted = false;
		m_out_nmi(0);
	}
}
