// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Chips 82C100 IBM PS/2 Model 30 and Super XT

    TODO:
    - EMS Page Registers
    - Keyboard NMI's maybe have issues

**********************************************************************/

#include "emu.h"
#include "82c100.h"

#include "multibyte.h"


#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(F82C100, f82c100_device, "82c100", "82C100 PS/2 Model 30 and Super XT")


f82c100_device::f82c100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, F82C100, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_dma8237(*this, "dma8237")
	, m_pic8259(*this, "pic8259")
	, m_pit8254(*this, "pit8254")
	, m_ppi8255(*this, "ppi8255")
	, m_intr_callback(*this)
	, m_nmi_callback(*this)
	, m_in_memr_callback(*this, 0)
	, m_out_memw_callback(*this)
	, m_in_ior_callback(*this, 0)
	, m_out_iow_callback(*this)
	, m_out_dack_callback(*this)
	, m_tc_callback(*this)
	, m_spkdata_callback(*this)
	, m_clock2(0)
{
}


void f82c100_device::map(address_map &map)
{
	map(0x0000, 0x000f).rw(m_dma8237, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0022, 0x0023).rw(FUNC(f82c100_device::config_r), FUNC(f82c100_device::config_w));
	map(0x0040, 0x0043).rw(m_pit8254, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0063).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0072, 0x0072).rw(FUNC(f82c100_device::nmi_control_r), FUNC(f82c100_device::nmi_control_w));
	map(0x007e, 0x007e).rw(FUNC(f82c100_device::nmi_status_r), FUNC(f82c100_device::nmi_status_w));
	map(0x007f, 0x007f).rw(FUNC(f82c100_device::pwr_control_r), FUNC(f82c100_device::pwr_control_w));
	map(0x0080, 0x0083).nopr().w(FUNC(f82c100_device::dma_page_w));
	map(0x00a0, 0x00af).w(FUNC(f82c100_device::nmi_control_w));
}


void f82c100_device::device_add_mconfig(machine_config &config)
{
	AM9517A(config, m_dma8237, clock() / 2);
	m_dma8237->out_hreq_callback().set(m_dma8237, FUNC(am9517a_device::hack_w));
	m_dma8237->out_eop_callback().set([this](int state) { m_tc_callback(state); });
	m_dma8237->in_memr_callback().set([this](offs_t offset) { return m_in_memr_callback((m_dma_page[m_dma_channel] << 16) + offset); });
	m_dma8237->out_memw_callback().set([this](offs_t offset, u8 data) { m_out_memw_callback((m_dma_page[m_dma_channel] << 16) + offset, data); });
	m_dma8237->in_ior_callback<0>().set([this]() { return m_in_ior_callback[0](); });
	m_dma8237->in_ior_callback<1>().set([this]() { return m_in_ior_callback[1](); });
	m_dma8237->in_ior_callback<2>().set([this]() { return m_in_ior_callback[2](); });
	m_dma8237->in_ior_callback<3>().set([this]() { return m_in_ior_callback[3](); });
	m_dma8237->out_iow_callback<0>().set([this](uint8_t data) { m_out_iow_callback[0](data); });
	m_dma8237->out_iow_callback<1>().set([this](uint8_t data) { m_out_iow_callback[1](data); });
	m_dma8237->out_iow_callback<2>().set([this](uint8_t data) { m_out_iow_callback[2](data); });
	m_dma8237->out_iow_callback<3>().set([this](uint8_t data) { m_out_iow_callback[3](data); });
	m_dma8237->out_dack_callback<0>().set([this](int state) { set_dack(0, state); });
	m_dma8237->out_dack_callback<1>().set([this](int state) { set_dack(1, state); });
	m_dma8237->out_dack_callback<2>().set([this](int state) { set_dack(2, state); });
	m_dma8237->out_dack_callback<3>().set([this](int state) { set_dack(3, state); });

	PIC8259(config, m_pic8259);
	m_pic8259->out_int_callback().set([this](int state) { m_intr_callback(state); });

	PIT8254(config, m_pit8254);
	m_pit8254->set_clk<0>(clock() / 12.0);
	m_pit8254->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_pit8254->set_clk<2>(clock() / 12.0);
	m_pit8254->out_handler<2>().set(FUNC(f82c100_device::pit8253_out2_changed));

	I8255A(config, m_ppi8255);
	m_ppi8255->in_pa_callback().set(FUNC(f82c100_device::ppi_porta_r));
	m_ppi8255->out_pa_callback().set([this](u8 data) { logerror("POST %d\n", data); });
	m_ppi8255->out_pb_callback().set(FUNC(f82c100_device::ppi_portb_w));
	m_ppi8255->in_pc_callback().set(FUNC(f82c100_device::ppi_portc_r));
}


void f82c100_device::device_start()
{
	m_maincpu->space(AS_PROGRAM).install_read_tap(0x08, 0x0b, "nmi_vector", [this](offs_t offset, u16 &data, u16 mem_mask)
	{
		// Substitute NMI vector
		if (BIT(m_cfg_regs[0x4b], 6))
		{
			data = get_u16le(&m_cfg_regs[0x3c + offset]); // Index 44H - 47H
		}
	});

	save_item(NAME(m_cfg_regs));
	save_item(NAME(m_cfg_indx));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_nmi_status));
	save_item(NAME(m_pwr_control));
	save_item(NAME(m_nmi_control));
	save_item(NAME(m_dma_page));
	save_item(NAME(m_dma_channel));
	save_item(NAME(m_ppi_portb));
	save_item(NAME(m_scan_code));
}

void f82c100_device::device_reset()
{
	for (int i = 0; i < 256; i++)
		m_cfg_regs[i] = 0;

	m_cfg_regs[0x40] = 0x01;
	m_cfg_regs[0x43] = 0x30;
	m_cfg_regs[0x48] = 0x01;

	m_cfg_indx = 0;

	m_nmi_enable  = true;
	m_nmi_status  = 0;
	m_pwr_control = 0;
	m_nmi_control = 0;

	for (int i = 0; i < 4; i++)
		m_dma_page[i] = 0;
	m_dma_channel = 0;

	m_spkdata = 0;
	m_pit_out2 = 1;

	// keyboard interface
	m_kbclklo = 0;
	m_kbclk = 1;
	m_kbdata = 0;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

u8 f82c100_device::config_r(offs_t offset)
{
	u8 data = 0x00;

	switch (offset & 1)
	{
	case 0:
		data = m_cfg_indx;
		break;
	case 1:
		data = m_cfg_regs[m_cfg_indx];
		break;
	}

	return data;
}

void f82c100_device::config_w(offs_t offset, u8 data)
{
	switch (offset & 1)
	{
	case 0:
		m_cfg_indx = data;
		break;
	case 1:
		m_cfg_regs[m_cfg_indx] = data;
		LOG("CR[%02x] = %02x\n", m_cfg_indx, data);

		if (m_cfg_indx == 0x40) // Clock/Mode Size
		{
			m_maincpu->set_unscaled_clock(BIT(data, 7) ? m_clock2 / 3 : clock() / 3);
		}
		break;
	}
}


void f82c100_device::pit8253_out2_changed(int state)
{
	m_pit_out2 = state;
	m_spkdata_callback(m_spkdata & m_pit_out2);
}

void f82c100_device::set_spkrdata(int state)
{
	m_spkdata = state;
	m_spkdata_callback(m_spkdata & m_pit_out2);
}


u8 f82c100_device::ppi_porta_r()
{
	// Keyboard Scan Code
	return m_scan_code;
}

void f82c100_device::ppi_portb_w(u8 data)
{
	// b0 TMR2GTSPK - Timer 2 Gate Speaker
	// b1 SPKDATA   - Speaker Data
	// b2 Reserved
	// b3 Read High/Low Switches
	// b4 nPCKEn   - Parity Check Enable
	// b5 nIOCHKEN - I/O Channel Check Enable
	// b6 nKBCLKLO - Keyboard Clock Low
	// b7 nKBEN    - Keyboard enable/clear
	m_ppi_portb = data;
	//logerror("ppi_portb_w: %02x\n", data);

	m_pit8254->write_gate2(BIT(data, 0));

	set_spkrdata(BIT(data, 1));

	m_kbclklo = !BIT(data, 6);

	if (BIT(data, 7))
	{
		m_scan_code = 0x00;
		m_scan_bit = 0;
		m_pic8259->ir1_w(CLEAR_LINE);

		if (BIT(m_nmi_control, 6))
			m_nmi_callback(ASSERT_LINE);
	}
}

u8 f82c100_device::ppi_portc_r()
{
	// b0-3 High/Low Switches
	// b4   Reserved
	// b5   Timer Channel 2 Out
	// b6   I/O Channel Check
	// b7   RAM Parity Check
	u8 data = 0x00;

	if (BIT(m_ppi_portb, 3))
		data |= (m_cfg_regs[0x43] >> 4) & 0x0f;
	else
		data |= m_cfg_regs[0x43] & 0x0f;

	data |= m_pit_out2 << 5;

	return data;
}


void f82c100_device::update_nmi()
{
	//logerror("update_nmi: %d status %02x\n", m_nmi_enable, m_nmi_status);
	if (m_nmi_enable && (m_nmi_status != 0x00))
		m_nmi_callback(ASSERT_LINE);
	else
		m_nmi_callback(CLEAR_LINE);
}

u8 f82c100_device::nmi_control_r()
{
	// b0 Reserved
	// b1 FDC Power Control
	// b2 Sleep Clock ON/OFF
	// b3 Enable RTC NMI
	// b4 Enable Keyboard Data NMI
	// b5 Enable Suspend NMI
	// b6 Enable Keyboard Clear NMI
	// b7 Reserved
	return m_nmi_control;
}

void f82c100_device::nmi_control_w(u8 data)
{
	//logerror("nmi_control_w: %02x\n", data);
	m_nmi_control = data;
}

u8 f82c100_device::nmi_status_r()
{
	// b0 Keyboard Data
	// b1 Reserved
	// b2 RTC NMI
	// b3 Suspend NMI
	// b4 Keyboard Clear
	// b5 PERR - Parity Error
	// b6 IOCHK - I/O Channel Check
	// b7 Reserved
	return m_nmi_status;
}

void f82c100_device::nmi_status_w(u8 data)
{
	m_nmi_status = data;
	update_nmi();
}

u8 f82c100_device::pwr_control_r()
{
	// b0 Reserved
	// b1 Request Power Off
	// b2 Reserved
	// b3 Software Controlled Reset
	// b4 Reserved
	// b5 Reserved
	// b6 External Power
	// b7 Low Battery
	return m_pwr_control;
}

void f82c100_device::pwr_control_w(u8 data)
{
	m_pwr_control = data;

	// Request Power Off
	if (BIT(data, 1))
	{
		m_nmi_status |= 0x08; // set Suspend NMI
		update_nmi();
	}
}

void f82c100_device::nmi_mask_w(u8 data)
{
	// b7 NMI Enable
	m_nmi_enable = BIT(data, 7);

	if (!m_nmi_enable)
	{
		m_nmi_status &= ~0x08; // clear Suspend NMI
		m_nmi_status &= ~0x04; // clear RTC NMI
	}
	update_nmi();
}

void f82c100_device::npnmi_w(int state)
{
	// TODO: unknown NMI status mask, maybe 0x02?

	update_nmi();
}

void f82c100_device::rtcnmi_w(int state)
{
	if (state && m_nmi_enable)
		m_nmi_status |= 0x04;  // set RTC NMI
	else
		m_nmi_status &= ~0x04; // clear RTC NMI

	update_nmi();
}

void f82c100_device::pwrnmi_w(int state)
{
	if (state && m_nmi_enable && BIT(m_cfg_regs[0x4b], 5))
		m_nmi_status |= 0x08;  // set Suspend NMI
	else
		m_nmi_status &= ~0x08; // clear Suspend NMI

	update_nmi();
}

void f82c100_device::set_dack(u8 channel, int state)
{
	if (!state) m_dma_channel = channel;

	m_out_dack_callback[channel](state);
}

void f82c100_device::dma_page_w(offs_t offset, u8 data)
{
	switch(offset % 4)
	{
	case 1:
		m_dma_page[2] = data;
		break;
	case 2:
		m_dma_page[3] = data;
		break;
	case 3:
		m_dma_page[0] = m_dma_page[1] = data;
		break;
	}
}


int f82c100_device::kbclk_r()
{
	if (m_kbclklo)
		return 0;
	else
		return 1; // pulled high externally
}

int f82c100_device::kbdata_r()
{
	if (!BIT(m_ppi_portb, 7)) // nKBEN
		return 1; // pulled high externally
	else
		return 1;
}

void f82c100_device::kbclk_w(int state)
{
	// TODO: this is crude but functional, implement start bit detection
	if (!state && m_kbclk)
	{
		m_scan_bit++;
		m_scan_code = (m_scan_code >> 1) | (m_kbdata << 7);

		if (m_scan_bit == 9) // 8 data bits + start bit
		{
			m_pic8259->ir1_w(ASSERT_LINE);
		}
	}
	m_kbclk = state;
}

void f82c100_device::kbdata_w(int state)
{
	m_kbdata = state;
}
