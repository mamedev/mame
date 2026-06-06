// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC14 (THERA) peripheral

******************************************************************************/

#include "emu.h"
#include "thera.h"


#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(THERA, thera_device, "thera", "Psion ASIC14 (THERA)")

thera_device::thera_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, THERA, tag, owner, clock)
	, m_int_cb(*this)
	, m_col_cb(*this)
	, m_spi_r(*this, 0)
	, m_port_r(*this, 0x00)
	, m_port_w(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void thera_device::device_start()
{
	m_fg[0] = timer_alloc(FUNC(thera_device::update_fg), this);
	m_fg[1] = timer_alloc(FUNC(thera_device::update_fg), this);
	m_timer[0] = timer_alloc(FUNC(thera_device::update_timer), this);
	m_timer[1] = timer_alloc(FUNC(thera_device::update_timer), this);

	save_item(NAME(m_syscon));
	save_item(NAME(m_ctrl_status));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_ddr));
	save_item(NAME(m_port_level));
	save_item(NAME(m_port_run));
	save_item(NAME(m_timer_load));
	save_item(NAME(m_timer_value));
	save_item(NAME(m_timer_ctrl));
	save_item(NAME(m_fg_load));
	save_item(NAME(m_fg_value));
	save_item(NAME(m_fg_ctrl));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_irq_edge));
	save_item(NAME(m_status3));
	save_item(NAME(m_keyb));
	save_item(NAME(m_spi_status));
	save_item(NAME(m_spi_data));
	save_item(NAME(m_spi_fn));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void thera_device::device_reset()
{
	m_fg[0]->adjust(attotime::never);
	m_fg[1]->adjust(attotime::never);
	m_timer[0]->adjust(attotime::never);
	m_timer[1]->adjust(attotime::never);

	m_syscon = 0x00;
	m_ctrl_status = 0x00;
	m_irq_status = 0x0000;
	m_irq_mask = 0x0000;
	m_irq_edge = 0x0000;
	m_status3 = 0x00;
	m_keyb = 0x00;
	m_spi_status = (1 << 1) | (1 << 10) | (1 << 11);
	m_spi_data = 0x00;
	m_spi_fn = 0x00;

	m_prom_addr = 0;

	std::fill(std::begin(m_prom), std::end(m_prom), 0);

	// defaults expected by the touchscreen code
	m_prom[0x0a] = 20;
	m_prom[0x0b] = 20;
	m_prom[0x0c] = 20;
	m_prom[0x0d] = 30;

	// set up the Psion's unique ID
	m_prom[0x1b] = 0xde;
	m_prom[0x1a] = 0xad;
	m_prom[0x19] = 0xbe;
	m_prom[0x18] = 0xef;

	// machine type
	m_prom[0x28] = m_machine_type.length();
	for (int i = 0; i < 16; i++)
		m_prom[0x29 + i] = (i < m_machine_type.length()) ? m_machine_type[i] : 0x00;

	// calculate the checksum
	uint8_t chksum = 0;
	for (int i = 1; i < 0x80; i++)
		chksum ^= m_prom[i];

	// EPOC is expecting 0x42
	m_prom[0x00] = chksum ^ 0x42;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void thera_device::update_interrupts()
{
	if (m_irq_status & m_irq_mask)
		m_int_cb(ASSERT_LINE);
	else
		m_int_cb(CLEAR_LINE);

	LOG("update_interrupts: status %04x mask %04x state %d\n", m_irq_status, m_irq_mask, m_irq_status & m_irq_mask ? 1 : 0);
}

TIMER_CALLBACK_MEMBER(thera_device::update_fg)
{
	switch (--m_fg_value[param])
	{
	case 0x0000:
		m_fg_state[param] ^= 1;
		m_port_w[PORTC]((m_port_data[PORTC] & 0xfc) | (m_fg_state[1] << 1) | (m_fg_state[0] << 0));
		break;

	case 0xffff:
		m_fg_value[param] = m_fg_load[param];
		break;
	}
}

TIMER_CALLBACK_MEMBER(thera_device::update_timer)
{
	switch (--m_timer_value[param])
	{
	case 0x0000:
		LOG("Flagging Timer %d IRQ\n", param + 1);
		m_irq_status |= (1 << (IRQ_TC1 + param));
		update_interrupts();
		break;

	case 0xffff:
		if (BIT(m_timer_ctrl[param], 1))
		{
			m_timer_value[param] = m_timer_load[param];
		}
		break;
	}
}

void thera_device::set_timer_ctrl(int timer, uint8_t value)
{
	if (value != m_timer_ctrl[timer])
	{
		if (BIT(value, 0))
		{
			attotime interval = BIT(value, 2) ? attotime::from_hz(14400) : attotime::from_hz(clock());
			m_timer[timer]->adjust(interval, timer, interval);
		}
		else
		{
			m_timer[timer]->adjust(attotime::never);
			m_timer_value[timer] = 0;
		}
		m_timer_ctrl[timer] = value;
	}
}


void thera_device::set_pb_line(int line, int state)
{
	// interrupt +ve edge trigger
	if (BIT(m_irq_edge, line) && !BIT(m_port_data[PORTB], line) && state)
		m_irq_status |= 1 << (line + 8);

	// interrupt -ve edge trigger
	if (BIT(m_irq_edge, line + 8) && BIT(m_port_data[PORTB], line) && !state)
		m_irq_status |= 1 << (line + 8);

	if (state)
		m_port_data[PORTB] |= (1 << line);
	else
		m_port_data[PORTB] &= ~(1 << line);

	update_interrupts();
}


uint16_t thera_device::read(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0x00;

	switch (offset)
	{
	case 0x00: // System Control Register
		//   b0    SSP Enable
		//   b1    Charger Disable
		//   b2    Bias Enable
		//   b3    CCFL Enable
		//   b4    Port Select
		//   b5    Port Select
		//   b6    Ext Clk Enable
		//   b7
		data = m_syscon;
		LOG("%s read: SysCon => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x01: // Status Register
		LOG("%s read: Status => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x02: // Port A Register
		switch (BIT(m_syscon, 4, 2))
		{
		case 0:
			data = m_port_r[PORTA]();
			LOG("%s read: PortAData => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 1:
			data = m_port_ddr[PORTA];
			LOG("%s read: PortADdr => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 2:
			data = m_port_level[PORTA];
			LOG("%s read: PortALevel => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 3:
			data = m_port_run[PORTA];
			LOG("%s read: PortARun => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		}
		break;

	case 0x03: // Port B Register
		switch (BIT(m_syscon, 4, 2))
		{
		case 0:
			data = m_port_data[PORTB]; // m_port_r[PORTB]();
			LOG("%s read: PortBData => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 1:
			data = m_port_ddr[PORTB];
			LOG("%s read: PortBDdr => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 2:
			data = m_port_level[PORTB];
			LOG("%s read: PortBLevel => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 3:
			data = m_port_run[PORTB];
			LOG("%s read: PortBRun => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		}
		break;

	case 0x04: // Port C Register
		switch (BIT(m_syscon, 4, 2))
		{
		case 0:
			data = m_port_r[PORTC]();
			LOG("%s read: PortCData => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 1:
			data = m_port_ddr[PORTC];
			LOG("%s read: PortCDdr => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 2:
			data = m_port_level[PORTC];
			LOG("%s read: PortCLevel => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 3:
			data = m_port_run[PORTC];
			LOG("%s read: PortCRun => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		}
		break;

	case 0x05: // Port D Register
		switch (BIT(m_syscon, 4, 2))
		{
		case 0:
			data = m_port_r[PORTD]();
			LOG("%s read: PortDData => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 1:
			data = m_port_ddr[PORTD];
			LOG("%s read: PortDDdr => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 2:
			data = m_port_level[PORTD];
			LOG("%s read: PortDLevel => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		case 3:
			data = m_port_run[PORTD];
			LOG("%s read: PortDRun => %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;
		}
		break;

	case 0x06: // Status of SSP
		data = 0x0b40;
		LOG("%s read: SspStatus => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x07: // Control of SSP
		LOG("%s read: SspControl => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x09: // Location of I2C Read or Write Data
		LOG("%s read: EeData => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x0a: // Location of A/D Data
		LOG("%s read: AdRxFifo => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x0b: // Bias PWM Control Register
		LOG("%s read: BiasControl => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x0c: // CCFL PWM Control Register
		LOG("%s read: CcflControl => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x0d: // Timer 1 Pre-load Register
		data = m_timer_load[0];
		LOG("%s read: Timer1Preload => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x0e: // Timer 1 Count Value
		data = m_timer_value[0];
		LOG("%s read: Timer1Count => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x0f: // Timer 1 IRQ Count
		data = 0;
		LOG("%s read: Timer1IrqCount => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x10: // Timer 2 Pre-load Register
		data = m_timer_load[1];
		LOG("%s read: Timer2Preload => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x11: // Timer 2 Count Value
		data = m_timer_value[1];
		LOG("%s read: Timer2Count => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x12: // Timer 2 IRQ Count
		data = 0;
		LOG("%s read: Timer2IrqCount => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x13: // Frequency Generator 1 Pre-load
		data = m_fg_load[0];
		LOG("%s read: Fg1Preload => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x14: // Frequency Generator 2 Pre-load
		data = m_fg_load[1];
		LOG("%s read: Fg2Preload => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x15: // Interrupt Status
		data = m_irq_status;
		LOG("%s read: IrqStatus => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x16: // Interrupt Mask
		data = m_irq_mask;
		LOG("%s read: IrqMask => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x17: // PCMCIA Control Register
		data = m_status3;
		LOG("%s read: PcmciaControl => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x18: // Keyboard Control Register
		data = m_keyb;
		LOG("%s read: Keyb => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x19: // Interrupt Edge Trigger Setup Register
		data = m_irq_edge;
		LOG("%s read: IrqEdge => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x1a: // CODEC Volume Control Register
		LOG("%s read: VolControl => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x1f: // Enable External Status Buffer
		data = 0; // xffff;
		LOG("%s read: ExtStat => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x20: // SPI Status
		data = m_spi_status;
		//m_spi_status &= ~2;
		//m_spi_status &= ~4;
		m_spi_status &= ~m_spi_status;
		LOG("%s read: SpiStatus => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x24: // SPI Data
		//if (data == 4 || data == 6 || data == 1)
		//  m_spi_status &= ~2;

		//if (data == 1)
		//{
		//  //m_irq_status |= 1 << 7; // ADC
		//  m_spi_status &= ~4;
		//}
		data = m_prom[m_prom_addr] | m_prom[m_prom_addr + 1] << 8;
		m_prom_addr += 2;
		LOG("%s read: SpiData => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x26: // SPI Function
		LOG("%s read: SpiFn => %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	default:
		LOG("%s read: unknown register %02x => %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);
		break;
	}
	return data;
}

void thera_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	case 0x00: // System Control Register
		//   b0    SSP Enable
		//   b1    Charger Disable
		//   b2    Bias Enable
		//   b3    CCFL Enable
		//   b4    Port Select
		//   b5    Port Select
		//   b6    Ext Clk Enable
		//   b7
		LOG("%s write: SysCon <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_syscon = data;
		break;

	case 0x02: // Port A Register
		switch (BIT(m_syscon, 4, 2))
		{
		case 0:
			LOG("%s write: PortAData <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_data[PORTA] = data;
			m_port_w[PORTA](data);
			break;
		case 1:
			LOG("%s write: PortADdr <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_ddr[PORTA] = data;
			break;
		case 2:
			LOG("%s write: PortALevel <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_level[PORTA] = data;
			break;
		case 3:
			LOG("%s write: PortARun <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_run[PORTA] = data;
			break;
		}
		break;

	case 0x03: // Port B Register
		switch (BIT(m_syscon, 4, 2))
		{
		case 0:
			LOG("%s write: PortBData <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_data[PORTB] = data;
			m_port_w[PORTB](data);
			break;
		case 1:
			LOG("%s write: PortBDdr <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_ddr[PORTB] = data;
			break;
		case 2:
			LOG("%s write: PortBLevel <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_level[PORTB] = data;
			break;
		case 3:
			LOG("%s write: PortBRun <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_run[PORTB] = data;
			break;
		}
		break;

	case 0x04: // Port C Register
		switch (BIT(m_syscon, 4, 2))
		{
		case 0:
			LOG("%s write: PortCData <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_data[PORTC] = data;
			m_port_w[PORTC](data);
			break;
		case 1:
			LOG("%s write: PortCDdr <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_ddr[PORTC] = data;
			break;
		case 2:
			LOG("%s write: PortCLevel <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_level[PORTC] = data;
			break;
		case 3:
			LOG("%s write: PortCRun <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_run[PORTC] = data;
			break;
		}
		break;

	case 0x05: // Port D Register
		switch (BIT(m_syscon, 4, 2))
		{
		case 0:
			LOG("%s write: PortDData <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_data[PORTD] = data;
			m_port_w[PORTD](data);
			break;
		case 1:
			LOG("%s write: PortDDdr <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_ddr[PORTD] = data;
			break;
		case 2:
			LOG("%s write: PortDLevel <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_level[PORTD] = data;
			break;
		case 3:
			LOG("%s write: PortDRun <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
			m_port_run[PORTD] = data;
			break;
		}
		break;

	case 0x06: // Status of SSP
		LOG("%s write: SspStatus <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x07: // Control of SSP
		LOG("%s write: SspControl <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x08: // Use to initialise I2C Read or Write Cycle
		LOG("%s write: EeControl <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x09: // Location of I2C Read or Write Data
		LOG("%s write: EeData <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x0a: //  Use to initialise A/D Cycle
		LOG("%s write: AdTxFifo <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x0b: // Bias PWM Control Register
		LOG("%s write: BiasControl <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x0c: // CCFL PWM Control Register
		LOG("%s write: CcflControl <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x0d: // Timer 1 Pre-load Register
		LOG("%s write: Timer1Preload <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_timer_load[0] = data;
		break;

	case 0x0e: // Timer 1 Control
		LOG("%s write: Timer1Count <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		set_timer_ctrl(0, data);
		break;

	case 0x0f: // Timer 1 IRQ Count
		LOG("%s write: Timer1IrqCount <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x10: // Timer 2 Pre-load Register
		LOG("%s write: Timer2Preload <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_timer_load[1] = data;
		break;

	case 0x11: // Timer 2 Control
		LOG("%s write: Timer2Count <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		set_timer_ctrl(1, data);
		break;

	case 0x12: // Timer 2 IRQ Count
		LOG("%s write: Timer2IrqCount <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x13: // Frequency Generator 1 Pre-load
		LOG("%s write: Fg1Preload <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_fg_load[0] = data;
		m_fg_value[0] = data;
		if (m_fg_load[0])
			m_fg[0]->adjust(attotime::from_hz(28800), 0, attotime::from_hz(28800));
		else
			m_fg[0]->adjust(attotime::never);
		break;

	case 0x14: // Frequency Generator 2 Pre-load
		LOG("%s write: Fg2Preload <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_fg_load[1] = data;
		m_fg_value[1] = data;
		if (m_fg_load[1])
			m_fg[1]->adjust(attotime::from_hz(28800), 1, attotime::from_hz(28800));
		else
			m_fg[1]->adjust(attotime::never);
		break;

	case 0x15: // Interrupt Clear
		LOG("%s write: IrqClear <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_irq_status &= ~data;
		update_interrupts();
		break;

	case 0x16: // Interrupt Mask
		LOG("%s write: IrqMask <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_irq_mask = data;
		update_interrupts();
		break;

	case 0x17: // PCMCIA Control Register
		LOG("%s write: PcmciaControl <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_status3 &= ~data;
		break;

	case 0x18: // Keyboard Control Register
		LOG("%s write: Keyb <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_keyb = data;
		if (BIT(data, 3))
			m_col_cb(1 << (data & 7));
		else if (data & 0x0f)
			m_col_cb(0x00);
		else
			m_col_cb(0xff);
		break;

	case 0x19: // Interrupt Edge Trigger Setup Register
		LOG("%s write: IrqEdge <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_irq_edge = data;
		break;

	case 0x1a: // CODEC Volume Control Register
		LOG("%s write: VolControl <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x20: // SPI Status
		LOG("%s write: SpiStatus <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_spi_status = data;
		break;

	case 0x24: // SPI Data
		LOG("%s write: SpiData <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_spi_data = data;
		break;

	case 0x26: // SPI Function
		LOG("%s write: SpiFn <= %04x & %04x\n", machine().describe_context(), data, mem_mask);
		m_spi_fn = data;
		if (data == 4 || data == 6 || data == 1)
			m_spi_status |= 2;

		if (data == 1)
		{
			m_irq_status |= 1 << 7; // ADC
			m_spi_status |= 4;
			update_interrupts();
		}
		break;

	default:
		LOG("%s write: unknown register %02x <= %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);
		break;
	}
}
