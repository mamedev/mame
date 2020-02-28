// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Western Digital WD7600 PC system chipset
 *
 *  WD76C10 - system control
 *  WD76C20 - FDC, RTC, Bus interface
 *  WD76C30 - 1 parallel and 2 serial ports
 *
 *  Created on: 5/05/2014
 *
 *  TODO:  pretty much everything
 */

#include "emu.h"
#include "machine/wd7600.h"

#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(WD7600, wd7600_device, "wd7600", "Western Digital WD7600 chipset")

void wd7600_device::device_add_mconfig(machine_config & config)
{
	AM9517A(config, m_dma1, 0);
	m_dma1->out_hreq_callback().set(m_dma2, FUNC(am9517a_device::dreq0_w));
	m_dma1->out_eop_callback().set(FUNC(wd7600_device::dma1_eop_w));
	m_dma1->in_memr_callback().set(FUNC(wd7600_device::dma_read_byte));
	m_dma1->out_memw_callback().set(FUNC(wd7600_device::dma_write_byte));
	m_dma1->in_ior_callback<0>().set(FUNC(wd7600_device::dma1_ior0_r));
	m_dma1->in_ior_callback<1>().set(FUNC(wd7600_device::dma1_ior1_r));
	m_dma1->in_ior_callback<2>().set(FUNC(wd7600_device::dma1_ior2_r));
	m_dma1->in_ior_callback<3>().set(FUNC(wd7600_device::dma1_ior3_r));
	m_dma1->out_iow_callback<0>().set(FUNC(wd7600_device::dma1_iow0_w));
	m_dma1->out_iow_callback<1>().set(FUNC(wd7600_device::dma1_iow1_w));
	m_dma1->out_iow_callback<2>().set(FUNC(wd7600_device::dma1_iow2_w));
	m_dma1->out_iow_callback<3>().set(FUNC(wd7600_device::dma1_iow3_w));
	m_dma1->out_dack_callback<0>().set(FUNC(wd7600_device::dma1_dack0_w));
	m_dma1->out_dack_callback<1>().set(FUNC(wd7600_device::dma1_dack1_w));
	m_dma1->out_dack_callback<2>().set(FUNC(wd7600_device::dma1_dack2_w));
	m_dma1->out_dack_callback<3>().set(FUNC(wd7600_device::dma1_dack3_w));

	AM9517A(config, m_dma2, 0);
	m_dma2->out_hreq_callback().set(FUNC(wd7600_device::dma2_hreq_w));
	m_dma2->in_memr_callback().set(FUNC(wd7600_device::dma_read_word));
	m_dma2->out_memw_callback().set(FUNC(wd7600_device::dma_write_word));
	m_dma2->in_ior_callback<1>().set(FUNC(wd7600_device::dma2_ior1_r));
	m_dma2->in_ior_callback<2>().set(FUNC(wd7600_device::dma2_ior2_r));
	m_dma2->in_ior_callback<3>().set(FUNC(wd7600_device::dma2_ior3_r));
	m_dma2->out_iow_callback<1>().set(FUNC(wd7600_device::dma2_iow1_w));
	m_dma2->out_iow_callback<2>().set(FUNC(wd7600_device::dma2_iow2_w));
	m_dma2->out_iow_callback<3>().set(FUNC(wd7600_device::dma2_iow3_w));
	m_dma2->out_dack_callback<0>().set(FUNC(wd7600_device::dma2_dack0_w));
	m_dma2->out_dack_callback<1>().set(FUNC(wd7600_device::dma2_dack1_w));
	m_dma2->out_dack_callback<2>().set(FUNC(wd7600_device::dma2_dack2_w));
	m_dma2->out_dack_callback<3>().set(FUNC(wd7600_device::dma2_dack3_w));

	PIC8259(config, m_pic1, 0);
	m_pic1->out_int_callback().set(FUNC(wd7600_device::pic1_int_w));
	m_pic1->in_sp_callback().set_constant(1);
	m_pic1->read_slave_ack_callback().set(FUNC(wd7600_device::pic1_slave_ack_r));

	PIC8259(config, m_pic2, 0);
	m_pic2->out_int_callback().set(m_pic1, FUNC(pic8259_device::ir2_w));
	m_pic2->in_sp_callback().set_constant(0);

	PIT8254(config, m_ctc, 0);
	m_ctc->set_clk<0>(XTAL(14'318'181) / 12.0);
	m_ctc->out_handler<0>().set(m_pic1, FUNC(pic8259_device::ir0_w));
	m_ctc->set_clk<1>(XTAL(14'318'181) / 12.0);
	m_ctc->out_handler<1>().set(FUNC(wd7600_device::ctc_out1_w));
	m_ctc->set_clk<2>(XTAL(14'318'181) / 12.0);
	m_ctc->out_handler<2>().set(FUNC(wd7600_device::ctc_out2_w));

	DS12885(config, m_rtc);
	m_rtc->irq().set(m_pic2, FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);
}


wd7600_device::wd7600_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WD7600, tag, owner, clock),
	m_read_ior(*this),
	m_write_iow(*this),
	m_write_tc(*this),
	m_write_hold(*this),
	m_write_nmi(*this),
	m_write_intr(*this),
	m_write_cpureset(*this),
	m_write_a20m(*this),
	m_write_spkr(*this),
	m_dma1(*this, "dma1"),
	m_dma2(*this, "dma2"),
	m_pic1(*this, "intc1"),
	m_pic2(*this, "intc2"),
	m_ctc(*this, "ctc"),
	m_rtc(*this, "rtc"),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_keybc(*this, finder_base::DUMMY_TAG),
	m_ram(*this, finder_base::DUMMY_TAG),
	m_bios(*this, finder_base::DUMMY_TAG),
	m_isa(*this, finder_base::DUMMY_TAG),
	m_portb(0x0f),
	m_iochck(1),
	m_nmi_mask(1),
	m_alt_a20(0),
	m_ext_gatea20(0),
	m_kbrst(1),
	m_refresh_toggle(0),
	m_dma_eop(0),
	m_dma_high_byte(0xff),
	m_dma_channel(-1)
{
}


void wd7600_device::device_start()
{
	// make sure the ram device is already running
	if (!m_ram->started())
		throw device_missing_dependencies();

	// resolve callbacks
	m_read_ior.resolve_safe(0);
	m_write_iow.resolve_safe();
	m_write_tc.resolve_safe();
	m_write_hold.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_intr.resolve_safe();
	m_write_cpureset.resolve_safe();
	m_write_a20m.resolve_safe();
	m_write_spkr.resolve_safe();

	m_space = &m_cpu->space(AS_PROGRAM);
	m_space_io = &m_cpu->space(AS_IO);

	// install base memory
	m_space->install_ram(0x000000, 0x09ffff, m_ram->pointer());
	m_space->install_ram(0x0d0000, 0x0effff, m_ram->pointer() + 0xd0000);

	// install extended memory
	if (m_ram->size() > 0x100000)
		m_space->install_ram(0x100000, m_ram->size() - 1, m_ram->pointer() + 0x100000);

	// install video BIOS (we should use the VGA BIOS at the beginning of the system BIOS ROM, but that gives a
	// blank display (but still runs))
	//m_space->install_rom(0x000c0000, 0x000cffff, &m_bios[0x00000]);
	m_space->install_rom(0x000c0000, 0x000cffff, m_isa);

	// install BIOS ROM at cpu initial pc
	m_space->install_rom(0x000f0000, 0x000fffff, &m_bios[0x10000]);
	if(m_space->addrmask() == 0xffffffff)  // 32-bit address space only
		m_space->install_rom(0xffff0000, 0xffffffff, &m_bios[0x10000]);
	else
		m_space->install_rom(0x00ff0000, 0x00ffffff, &m_bios[0x10000]);

	// install i/o accesses
	if (m_space_io->data_width() == 16)
	{
		// FIXME: are all these address ranges correct?
		m_space_io->install_readwrite_handler(0x0000, 0x000f, read8sm_delegate(*m_dma1, FUNC(am9517a_device::read)), write8sm_delegate(*m_dma1, FUNC(am9517a_device::write)), 0xffff);
		m_space_io->install_readwrite_handler(0x0020, 0x003f, read8sm_delegate(*m_pic1, FUNC(pic8259_device::read)), write8sm_delegate(*m_pic1, FUNC(pic8259_device::write)), 0xffff);
		m_space_io->install_readwrite_handler(0x0040, 0x0043, read8sm_delegate(*m_ctc, FUNC(pit8254_device::read)), write8sm_delegate(*m_ctc, FUNC(pit8254_device::write)), 0xffff);
		m_space_io->install_readwrite_handler(0x0060, 0x0061, read8smo_delegate(*this, FUNC(wd7600_device::keyb_data_r)), write8smo_delegate(*this, FUNC(wd7600_device::keyb_data_w)), 0x00ff);
		m_space_io->install_readwrite_handler(0x0060, 0x0061, read8smo_delegate(*this, FUNC(wd7600_device::portb_r)), write8smo_delegate(*this, FUNC(wd7600_device::portb_w)), 0xff00);
		m_space_io->install_readwrite_handler(0x0064, 0x0065, read8smo_delegate(*this, FUNC(wd7600_device::keyb_status_r)), write8smo_delegate(*this, FUNC(wd7600_device::keyb_cmd_w)), 0x00ff);
		m_space_io->install_readwrite_handler(0x0070, 0x007f, read8sm_delegate(*m_rtc, FUNC(mc146818_device::read)), write8sm_delegate(*this, FUNC(wd7600_device::rtc_w)), 0xffff);
		m_space_io->install_readwrite_handler(0x0080, 0x008f, read8sm_delegate(*this, FUNC(wd7600_device::dma_page_r)), write8sm_delegate(*this, FUNC(wd7600_device::dma_page_w)), 0xffff);
		m_space_io->install_readwrite_handler(0x0092, 0x0093, read8smo_delegate(*this, FUNC(wd7600_device::a20_reset_r)), write8smo_delegate(*this, FUNC(wd7600_device::a20_reset_w)), 0x00ff);
		m_space_io->install_readwrite_handler(0x00a0, 0x00a3, read8sm_delegate(*m_pic2, FUNC(pic8259_device::read)), write8sm_delegate(*m_pic2, FUNC(pic8259_device::write)), 0xffff);
		m_space_io->install_readwrite_handler(0x00c0, 0x00df, read8sm_delegate(*m_dma2, FUNC(am9517a_device::read)), write8sm_delegate(*m_dma2, FUNC(am9517a_device::write)), 0x00ff);
		m_space_io->install_readwrite_handler(0x2072, 0x2073, read16smo_delegate(*this, FUNC(wd7600_device::refresh_r)), write16smo_delegate(*this, FUNC(wd7600_device::refresh_w)));
		m_space_io->install_readwrite_handler(0x2872, 0x2873, read16smo_delegate(*this, FUNC(wd7600_device::chipsel_r)), write16smo_delegate(*this, FUNC(wd7600_device::chipsel_w)));
		m_space_io->install_readwrite_handler(0x3872, 0x3873, read16smo_delegate(*this, FUNC(wd7600_device::mem_ctrl_r)), write16smo_delegate(*this, FUNC(wd7600_device::mem_ctrl_w)));
		m_space_io->install_readwrite_handler(0x4872, 0x4873, read16s_delegate(*this, FUNC(wd7600_device::bank_01_start_r)), write16s_delegate(*this, FUNC(wd7600_device::bank_01_start_w)));
		m_space_io->install_readwrite_handler(0x5072, 0x5073, read16s_delegate(*this, FUNC(wd7600_device::bank_23_start_r)), write16s_delegate(*this, FUNC(wd7600_device::bank_23_start_w)));
		m_space_io->install_readwrite_handler(0x5872, 0x5873, read16smo_delegate(*this, FUNC(wd7600_device::split_addr_r)), write16smo_delegate(*this, FUNC(wd7600_device::split_addr_w)));
		m_space_io->install_readwrite_handler(0x9872, 0x9873, read16smo_delegate(*this, FUNC(wd7600_device::diag_r)), write16smo_delegate(*this, FUNC(wd7600_device::diag_w)));
	}
	else
	{
		assert(m_space_io->data_width() == 32);
		m_space_io->install_readwrite_handler(0x0000, 0x000f, read8sm_delegate(*m_dma1, FUNC(am9517a_device::read)), write8sm_delegate(*m_dma1, FUNC(am9517a_device::write)), 0xffffffff);
		m_space_io->install_readwrite_handler(0x0020, 0x003f, read8sm_delegate(*m_pic1, FUNC(pic8259_device::read)), write8sm_delegate(*m_pic1, FUNC(pic8259_device::write)), 0x0000ffff);
		m_space_io->install_readwrite_handler(0x0040, 0x0043, read8sm_delegate(*m_ctc, FUNC(pit8254_device::read)), write8sm_delegate(*m_ctc, FUNC(pit8254_device::write)), 0xffffffff);
		m_space_io->install_readwrite_handler(0x0060, 0x0063, read8smo_delegate(*this, FUNC(wd7600_device::keyb_data_r)), write8smo_delegate(*this, FUNC(wd7600_device::keyb_data_w)), 0x000000ff);
		m_space_io->install_readwrite_handler(0x0060, 0x0063, read8smo_delegate(*this, FUNC(wd7600_device::portb_r)), write8smo_delegate(*this, FUNC(wd7600_device::portb_w)), 0x0000ff00);
		m_space_io->install_readwrite_handler(0x0064, 0x0067, read8smo_delegate(*this, FUNC(wd7600_device::keyb_status_r)), write8smo_delegate(*this, FUNC(wd7600_device::keyb_cmd_w)), 0x000000ff);
		m_space_io->install_readwrite_handler(0x0070, 0x007f, read8sm_delegate(*m_rtc, FUNC(mc146818_device::read)), write8sm_delegate(*this, FUNC(wd7600_device::rtc_w)), 0x0000ffff);
		m_space_io->install_readwrite_handler(0x0080, 0x008f, read8sm_delegate(*this, FUNC(wd7600_device::dma_page_r)), write8sm_delegate(*this, FUNC(wd7600_device::dma_page_w)), 0xffffffff);
		m_space_io->install_readwrite_handler(0x0090, 0x0093, read8smo_delegate(*this, FUNC(wd7600_device::a20_reset_r)), write8smo_delegate(*this, FUNC(wd7600_device::a20_reset_w)), 0x00ff0000);
		m_space_io->install_readwrite_handler(0x00a0, 0x00a3, read8sm_delegate(*m_pic2, FUNC(pic8259_device::read)), write8sm_delegate(*m_pic2, FUNC(pic8259_device::write)), 0x0000ffff);
		m_space_io->install_readwrite_handler(0x00c0, 0x00df, read8sm_delegate(*m_dma2, FUNC(am9517a_device::read)), write8sm_delegate(*m_dma2, FUNC(am9517a_device::write)), 0x00ff00ff);
		m_space_io->install_readwrite_handler(0x2070, 0x2073, read16smo_delegate(*this, FUNC(wd7600_device::refresh_r)), write16smo_delegate(*this, FUNC(wd7600_device::refresh_w)), 0xffff0000);
		m_space_io->install_readwrite_handler(0x2870, 0x2873, read16smo_delegate(*this, FUNC(wd7600_device::chipsel_r)), write16smo_delegate(*this, FUNC(wd7600_device::chipsel_w)), 0xffff0000);
		m_space_io->install_readwrite_handler(0x3870, 0x3873, read16smo_delegate(*this, FUNC(wd7600_device::mem_ctrl_r)), write16smo_delegate(*this, FUNC(wd7600_device::mem_ctrl_w)), 0xffff0000);
		m_space_io->install_readwrite_handler(0x4870, 0x4873, read16s_delegate(*this, FUNC(wd7600_device::bank_01_start_r)), write16s_delegate(*this, FUNC(wd7600_device::bank_01_start_w)), 0xffff0000);
		m_space_io->install_readwrite_handler(0x5070, 0x5073, read16s_delegate(*this, FUNC(wd7600_device::bank_23_start_r)), write16s_delegate(*this, FUNC(wd7600_device::bank_23_start_w)), 0xffff0000);
		m_space_io->install_readwrite_handler(0x5870, 0x5873, read16smo_delegate(*this, FUNC(wd7600_device::split_addr_r)), write16smo_delegate(*this, FUNC(wd7600_device::split_addr_w)), 0xffff0000);
		m_space_io->install_readwrite_handler(0x9870, 0x9873, read16smo_delegate(*this, FUNC(wd7600_device::diag_r)), write16smo_delegate(*this, FUNC(wd7600_device::diag_w)), 0xffff0000);
	}
}

void wd7600_device::device_reset()
{
	m_split_start = 0;
	m_chip_sel = 0;
	m_refresh_ctrl = 0;
	m_memory_ctrl = 0;
	m_diagnostic = 0xe080;

	for(auto & elem : m_bank_start)
		elem = 0;

	// initialize dma controller clocks
	m_dma1->set_unscaled_clock(clock());
	m_dma2->set_unscaled_clock(clock());
}


WRITE_LINE_MEMBER( wd7600_device::iochck_w )
{
	if (BIT(m_portb, 3) == 0)
	{
		if (m_iochck && state == 0)
		{
			// set channel check latch
			m_portb |= 1 << 6;
			nmi();
		}

		m_iochck = state;
	}
}

void wd7600_device::nmi()
{
	if (m_nmi_mask & BIT(m_portb, 6))
	{
		m_write_nmi(1);
		m_write_nmi(0);
	}
}

void wd7600_device::a20m()
{
	// TODO: ignore keyboard A20 signal if set in Diagnostic register (0x9872)
	m_write_a20m(m_alt_a20 | m_ext_gatea20);
}

void wd7600_device::keyboard_gatea20(int state)
{
	m_ext_gatea20 = state;
	a20m();
}

void wd7600_device::rtc_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		m_nmi_mask = !BIT(data, 7);
		data &= 0x7f;
	}

	m_rtc->write(offset, data);
}

uint8_t wd7600_device::pic1_slave_ack_r(offs_t offset)
{
	if (offset == 2) // IRQ 2
		return m_pic2->acknowledge();

	return 0x00;
}

// Timer outputs
WRITE_LINE_MEMBER( wd7600_device::ctc_out1_w )
{
	m_refresh_toggle ^= state;
	m_portb = (m_portb & 0xef) | (m_refresh_toggle << 4);
}

WRITE_LINE_MEMBER( wd7600_device::ctc_out2_w )
{
	m_write_spkr(!(state));
	m_portb = (m_portb & 0xdf) | (state << 5);
}

// Keyboard
void wd7600_device::keyb_data_w(uint8_t data)
{
//  LOG("WD7600: keyboard data write %02x\n", data);
	m_keybc->data_w(data);
}

uint8_t wd7600_device::keyb_data_r()
{
	uint8_t ret = m_keybc->data_r();
//  LOG("WD7600: keyboard data read %02x\n", ret);
	return ret;
}

void wd7600_device::keyb_cmd_w(uint8_t data)
{
//  LOG("WD7600: keyboard command %02x\n", data);
	m_keybc->command_w(data);
}

uint8_t wd7600_device::keyb_status_r()
{
	return m_keybc->status_r();
}

uint8_t wd7600_device::portb_r()
{
	return m_portb;
}

void wd7600_device::portb_w(uint8_t data)
{
	m_portb = (m_portb & 0xf0) | (data & 0x0f);

	// bit 5 forced to 1 if timer disabled
	if (!BIT(m_portb, 0))
		m_portb |= 1 << 5;

	m_ctc->write_gate2(BIT(m_portb, 0));

	m_write_spkr(!BIT(m_portb, 1));

	// clear channel check latch?
	if (BIT(m_portb, 3))
		m_portb &= 0xbf;
}

// DMA controllers
offs_t wd7600_device::page_offset()
{
	switch (m_dma_channel)
	{
		case 0: return (offs_t) m_dma_page[0x07] << 16;
		case 1: return (offs_t) m_dma_page[0x03] << 16;
		case 2: return (offs_t) m_dma_page[0x01] << 16;
		case 3: return (offs_t) m_dma_page[0x02] << 16;
		case 5: return (offs_t) m_dma_page[0x0b] << 16;
		case 6: return (offs_t) m_dma_page[0x09] << 16;
		case 7: return (offs_t) m_dma_page[0x0a] << 16;
	}

	// should never get here
	return 0xff0000;
}

uint8_t wd7600_device::dma_read_byte(offs_t offset)
{
	if (m_dma_channel == -1)
		return 0xff;

	return m_space->read_byte(page_offset() + offset);
}

void wd7600_device::dma_write_byte(offs_t offset, uint8_t data)
{
	if (m_dma_channel == -1)
		return;

	m_space->write_byte(page_offset() + offset, data);
}

uint8_t wd7600_device::dma_read_word(offs_t offset)
{
	if (m_dma_channel == -1)
		return 0xff;

	uint16_t result = m_space->read_word((page_offset() & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result >> 8;

	return result;
}

void wd7600_device::dma_write_word(offs_t offset, uint8_t data)
{
	if (m_dma_channel == -1)
		return;

	m_space->write_word((page_offset() & 0xfe0000) | (offset << 1), (m_dma_high_byte << 8) | data);
}

WRITE_LINE_MEMBER( wd7600_device::dma2_dack0_w )
{
	m_dma1->hack_w(state ? 0 : 1); // inverted?
}

WRITE_LINE_MEMBER( wd7600_device::dma1_eop_w )
{
	m_dma_eop = state;
	if (m_dma_channel != -1)
		m_write_tc(m_dma_channel, state, 0xff);
}

void wd7600_device::set_dma_channel(int channel, bool state)
{
	//m_write_dack(channel, state);

	if (!state)
	{
		m_dma_channel = channel;
		if (m_dma_eop)
			m_write_tc(channel, 1, 0xff);
	}
	else
	{
		if (m_dma_channel == channel)
		{
			m_dma_channel = -1;
			if (m_dma_eop)
				m_write_tc(channel, 0, 0xff);
		}
	}
}

WRITE_LINE_MEMBER( wd7600_device::gatea20_w )
{
	keyboard_gatea20(state);
}

WRITE_LINE_MEMBER( wd7600_device::kbrst_w )
{
	// convert to active low signal (gets inverted in at_keybc.c)
	state = (state == ASSERT_LINE ? 0 : 1);

	// detect transition
	if (m_kbrst == 1 && state == 0)
	{
		m_write_cpureset(1);
		m_write_cpureset(0);
	}

	m_kbrst = state;
}

void wd7600_device::a20_reset_w(uint8_t data)
{
	m_alt_a20 = BIT(data,1);
	a20m();
	// TODO: proper timing.  Reset occurs 128 cycles after changing to a 1, and lasts for 16 cycles
	if(BIT(data,0))
	{
		m_write_cpureset(1);
		m_write_cpureset(0);
		LOG("WD7600: System reset\n");
	}
}

uint8_t wd7600_device::a20_reset_r()
{
	uint8_t ret = 0;
	if(m_alt_a20)
		ret |= 0x02;
	return ret;
}

// port 0x2072 - Refresh Control, and serial/parallel port address select
uint16_t wd7600_device::refresh_r()
{
	return m_refresh_ctrl;
}

void wd7600_device::refresh_w(uint16_t data)
{
	// TODO: select serial/parallel I/O port location
	m_refresh_ctrl = data;
	LOG("WD7600: Refresh Control write %04x\n", data);
}

// port 0x2872 - chip select
uint16_t wd7600_device::chipsel_r()
{
	return m_chip_sel;
}

void wd7600_device::chipsel_w(uint16_t data)
{
	m_chip_sel = data;
	LOG("WD7600: Chip Select write %04x\n", data);
}

// port 0x3872 - Memory Control
uint16_t wd7600_device::mem_ctrl_r()
{
	return m_memory_ctrl;
}

void wd7600_device::mem_ctrl_w(uint16_t data)
{
	m_memory_ctrl = data;
	LOG("WD7600: Memory Control write %04x\n", data);
}

// port 0x4872 - Bank 0 and 1 start address
uint16_t wd7600_device::bank_01_start_r(offs_t offset, uint16_t mem_mask)
{
	return (m_bank_start[1] << 8) | m_bank_start[0];
}

void wd7600_device::bank_01_start_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(ACCESSING_BITS_0_7)
	{
		m_bank_start[0] = data & 0xff;
		LOG("WD7600: Bank 0 start address %08x\n", m_bank_start[0] << 16);
	}
	if(ACCESSING_BITS_8_15)
	{
		m_bank_start[1] = (data & 0xff00) >> 8;
		LOG("WD7600: Bank 1 start address %08x\n", m_bank_start[1] << 16);
	}
}

// port 0x5072 - Bank 2 and 3 start address
uint16_t wd7600_device::bank_23_start_r(offs_t offset, uint16_t mem_mask)
{
	return (m_bank_start[3] << 8) | m_bank_start[2];
}

void wd7600_device::bank_23_start_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(ACCESSING_BITS_0_7)
	{
		m_bank_start[2] = data & 0xff;
		LOG("WD7600: Bank 2 start address %08x\n", m_bank_start[2] << 16);
	}
	if(ACCESSING_BITS_8_15)
	{
		m_bank_start[3] = (data & 0xff00) >> 8;
		LOG("WD7600: Bank 3 start address %08x\n", m_bank_start[3] << 16);
	}
}

// port 0x5872 - split starting address (used for BIOS shadowing)
uint16_t wd7600_device::split_addr_r()
{
	return m_split_start;
}

void wd7600_device::split_addr_w(uint16_t data)
{
	m_split_start = data;
	LOG("WD7600: Split start address write %04x\n", data);
}

// port 0x9872 - Diagnostic
uint16_t wd7600_device::diag_r()
{
	return m_diagnostic | 0xe080;
}

void wd7600_device::diag_w(uint16_t data)
{
	m_diagnostic = data;
	LOG("WD7600: Diagnostic write %04x\n", data);
}
