// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Chips 82C606 CHIPSpak Multifunction Controller

**********************************************************************/

#include "emu.h"
#include "82c606.h"


#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(P82C606, p82c606_device, "82c606", "82C606 CHIPSpak Multifunction Controller")


p82c606_device::p82c606_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, P82C606, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_rtc(*this, "rtc")
	, m_cmos_ram(*this, "cmos_ram", 0x40, ENDIANNESS_LITTLE)
	, m_nvram(*this, "nvram")
	, m_lpt(*this, "parallel")
	, m_serial(*this, "serial%u", 1U)
	, m_irq3_callback(*this)
	, m_irq4_callback(*this)
	, m_irq5_callback(*this)
	, m_irq7_callback(*this)
	, m_txd1_callback(*this)
	, m_dtr1_callback(*this)
	, m_rts1_callback(*this)
	, m_txd2_callback(*this)
	, m_dtr2_callback(*this)
	, m_rts2_callback(*this)
{
}


void p82c606_device::device_add_mconfig(machine_config &config)
{
	// serial ports
	NS16450(config, m_serial[0], clock());
	m_serial[0]->out_int_callback().set(FUNC(p82c606_device::int_select<S1>));
	m_serial[0]->out_tx_callback().set([this](int state) { m_txd1_callback(state); });
	m_serial[0]->out_dtr_callback().set([this](int state) { m_dtr1_callback(state); });
	m_serial[0]->out_rts_callback().set([this](int state) { m_rts1_callback(state); });

	NS16450(config, m_serial[1], clock());
	m_serial[1]->out_int_callback().set(FUNC(p82c606_device::int_select<S2>));
	m_serial[1]->out_tx_callback().set([this](int state) { m_txd2_callback(state); });
	m_serial[1]->out_dtr_callback().set([this](int state) { m_dtr2_callback(state); });
	m_serial[1]->out_rts_callback().set([this](int state) { m_rts2_callback(state); });

	// parallel port
	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set(FUNC(p82c606_device::int_select<PP>));

	// rtc
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(FUNC(p82c606_device::int_select<RC>));
	m_rtc->set_binary(true);
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}


void p82c606_device::device_start()
{
	address_space &io_space = m_maincpu->space(AS_IO);

	// CHIPSpak configuration
	io_space.install_write_tap(0x2fa, 0x2fb, 0x100, "chipspak6", [this](offs_t offset, u16 &data, u16 mem_mask) { chipspak_w(offset, data & 0xff); });

	// register defaults
	const u8 cfg_regs_defaults[] = { 0x00, 0x00, 0x00, 0xb0, 0xfe, 0xbe, 0x9e, 0x80, 0xec, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	for (int i = 0; i < std::size(cfg_regs_defaults); i++)
	{
		m_cfg_regs[i] = cfg_regs_defaults[i];
	}

	m_nvram->set_base(m_cmos_ram, 0x40);

	save_item(NAME(m_cmos_addr));
	save_item(NAME(m_cfg_regs));
	save_item(NAME(m_cfg_indx));
	save_item(NAME(m_cfg_state));
}

void p82c606_device::device_reset()
{
}


void p82c606_device::chipspak_w(offs_t offset, u8 data)
{
	LOG("chipspak_w: %04x %02x\n", offset, data);

	address_space &io_space = m_maincpu->space(AS_IO);

	// configuration sequence
	if (m_cfg_state == 0 && offset == 0x2fa)
	{
		m_cfg_key = data;
		m_cfg_state++;
	}
	else if (m_cfg_state == 1 && offset == 0x3fa && data == (u8)(~m_cfg_key))
	{
		m_cfg_state++;
	}
	else if (m_cfg_state == 2 && offset == 0x3fa && data == 0x36) // part number (ASCII 6)
	{
		m_cfg_state++;
	}
	else if (m_cfg_state == 3 && offset == 0x3fa)
	{
		m_cfg_cri = data << 2;
		m_cfg_state++;
	}
	else if (m_cfg_state == 4 && offset == 0x2fa && data == (u8)(~m_cfg_cri >> 2))
	{
		m_cfg_state++;
		m_cfg_regs[0x0f] = m_cfg_cri >> 2; // Config Addr

		LOG("CFG -- mode on -- CRI %04x\n", m_cfg_cri);

		// install the CRI and CAP ports
		io_space.install_readwrite_handler(m_cfg_cri, m_cfg_cri + 1, emu::rw_delegate(*this, FUNC(p82c606_device::cfg_r)), emu::rw_delegate(*this, FUNC(p82c606_device::cfg_w)));
	}
	else
	{
		m_cfg_state = 0;
	}
}


u8 p82c606_device::cfg_r(offs_t offset, u8 mem_mask)
{
	u8 data = 0x00;

	switch (offset & 1)
	{
	case 0: // CRI
		data = m_cfg_indx;
		break;

	case 1: // CAP
		data = m_cfg_regs[m_cfg_indx];
		LOG("CR[%02x] => %02x\n", m_cfg_indx, data);
	}

	return data;
}

void p82c606_device::cfg_w(offs_t offset, u8 data, u8 mem_mask)
{
	address_space &io_space = m_maincpu->space(AS_IO);

	switch (offset & 1)
	{
	case 0: // CRI
		m_cfg_indx = data & 0x0f;
		break;

	case 1: // CAP
		if (m_cfg_indx != 0x0f)
			m_cfg_regs[m_cfg_indx] = data;

		switch (m_cfg_indx)
		{
		case 0x00: // Enable
			LOG("CR[%02x] <= %02x\n", m_cfg_indx, data);
			LOG("GP %s\n", BIT(data, GP) ? "enabled" : "disabled");
			LOG("S1 %s\n", BIT(data, S1) ? "enabled" : "disabled");
			LOG("S2 %s\n", BIT(data, S2) ? "enabled" : "disabled");
			LOG("PP %s\n", BIT(data, PP) ? "enabled" : "disabled");
			LOG("RC %s\n", BIT(data, RC) ? "enabled" : "disabled");
			break;

		case 0x01: // Configuration
			LOG("CR[%02x] <= %02x\n", m_cfg_indx, data);
			break;

		case 0x02: // Ext Baud Rate Select
			LOG("CR[%02x] <= %02x\n", m_cfg_indx, data);
			break;

		case 0x03: // RTC Port Base Address
			LOG("CR[%02x] <= %02x  RC address %04x\n", m_cfg_indx, data, data << 2);
			break;

		case 0x04: // UART1 Port Base Address
			LOG("CR[%02x] <= %02x  S1 address %04x\n", m_cfg_indx, data, (data & 0xfe) << 2);
			break;

		case 0x05: // UART2 Port Base Address
			LOG("CR[%02x] <= %02x  S2 address %04x\n", m_cfg_indx, data, (data & 0xfe) << 2);
			break;

		case 0x06: // Parallel Port Base Address
			LOG("CR[%02x] <= %02x  PP address %04x\n", m_cfg_indx, data, data << 2);
			break;

		case 0x07: // Game Port Base Address
			LOG("CR[%02x] <= %02x  GP address %04x\n", m_cfg_indx, data, data << 2);
			break;

		case 0x08: // Interrupt Select
			LOG("CR[%02x] <= %02x\n", m_cfg_indx, data);
			break;

		case 0x0f: // Config Termination
			m_cfg_state = 0;
			LOG("CFG -- mode off --\n");

			// install enabled devices
			if (BIT(m_cfg_regs[0], GP))
			{
				//offs_t addr = m_cfg_regs[7] << 2;
				//io_space.install_readwrite_handler(addr, addr + 1, emu::rw_delegate(*m_serial[0], FUNC(ins8250_device::ins8250_r)), emu::rw_delegate(*m_serial[0], FUNC(ins8250_device::ins8250_w)));
			}
			if (BIT(m_cfg_regs[0], S1))
			{
				offs_t addr = (m_cfg_regs[4] & 0xfe) << 2;
				io_space.install_readwrite_handler(addr, addr + 7, emu::rw_delegate(*m_serial[0], FUNC(ins8250_device::ins8250_r)), emu::rw_delegate(*m_serial[0], FUNC(ins8250_device::ins8250_w)));
			}
			if (BIT(m_cfg_regs[0], S2))
			{
				offs_t addr = (m_cfg_regs[5] & 0xfe) << 2;
				io_space.install_readwrite_handler(addr, addr + 7, emu::rw_delegate(*m_serial[1], FUNC(ins8250_device::ins8250_r)), emu::rw_delegate(*m_serial[1], FUNC(ins8250_device::ins8250_w)));
			}
			if (BIT(m_cfg_regs[0], PP))
			{
				offs_t addr = m_cfg_regs[6] << 2;
				io_space.install_readwrite_handler(addr, addr + 3, emu::rw_delegate(*m_lpt, FUNC(pc_lpt_device::read)), emu::rw_delegate(*m_lpt, FUNC(pc_lpt_device::write)));
			}
			if (BIT(m_cfg_regs[0], RC))
			{
				offs_t addr = m_cfg_regs[3] << 2;
				io_space.install_readwrite_handler(addr, addr + 1, emu::rw_delegate(*this, FUNC(p82c606_device::rtc_r)), emu::rw_delegate(*this, FUNC(p82c606_device::rtc_w)));
			}
			break;
		}
	}
}


u8 p82c606_device::rtc_r(offs_t offset)
{
	u8 data = 0x00;

	if (offset & 1)
	{
		if (m_cmos_addr & 0x40)
			data = m_cmos_ram[m_cmos_addr & 0x3f];
		else
			data = m_rtc->read_direct(m_cmos_addr & 0x3f);
	}

	return data;
}

void p82c606_device::rtc_w(offs_t offset, u8 data)
{
	if (offset & 1)
	{
		if (m_cmos_addr & 0x40)
			m_cmos_ram[m_cmos_addr & 0x3f] = data;
		else
			m_rtc->write_direct(m_cmos_addr & 0x3f, data);
	}
	else
	{
		m_cmos_addr = data & 0x7f;
	}
}


template <unsigned N>
void p82c606_device::int_select(int state)
{
	switch (N)
	{
	case S1:
		if (BIT(m_cfg_regs[8], 6, 2) == 2) m_irq3_callback(state);
		if (BIT(m_cfg_regs[8], 4, 2) == 2) m_irq4_callback(state);
		if (BIT(m_cfg_regs[8], 2, 2) == 2) m_irq5_callback(state);
		break;

	case S2:
		if (BIT(m_cfg_regs[8], 6, 2) == 3) m_irq3_callback(state);
		if (BIT(m_cfg_regs[8], 4, 2) == 3) m_irq4_callback(state);
		if (BIT(m_cfg_regs[8], 0, 2) == 2) m_irq7_callback(state);
		break;

	case PP:
		if (BIT(m_cfg_regs[8], 2, 2) == 3) m_irq5_callback(state);
		if (BIT(m_cfg_regs[8], 0, 2) == 3) m_irq7_callback(state);
		break;

	case RC:
		if (BIT(m_cfg_regs[8], 6, 2) == 1) m_irq3_callback(state);
		if (BIT(m_cfg_regs[8], 4, 2) == 1) m_irq4_callback(state);
		if (BIT(m_cfg_regs[8], 2, 2) == 1) m_irq5_callback(state);
		if (BIT(m_cfg_regs[8], 0, 2) == 1) m_irq7_callback(state);
		break;
	}
}
