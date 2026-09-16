// license:BSD-3-Clause
// copyright-holders:smf

#include "emu.h"
#include "mb90570.h"

DEFINE_DEVICE_TYPE(MB90F574, mb90f574_device, "mb90f574", "Fujitsu MB90F574")

mb90f574_device::mb90f574_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mb90570_device(mconfig, MB90F574, tag, owner, clock, 0x28ff)
{
}

mb90570_device::mb90570_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint16_t internal_ram_end) :
	f2mc16_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor(FUNC(mb90570_device::internal_map), this)),
	m_internal_ram_end(internal_ram_end),
	m_adc(*this, "adc"),
	m_clock_generator(*this, "clock_generator"),
	m_intc(*this, "intc"),
	m_port(*this, "port%x", 1U),
	m_ppg(*this, "ppg"),
	m_uart(*this, "uart%u", 0U)
{
}

device_memory_interface::space_config_vector mb90570_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void mb90570_device::device_add_mconfig(machine_config &mconfig)
{
	F2MC16_PORT(mconfig, m_port[0x0], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x1], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x2], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x3], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x4], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x5], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x6], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x7], 0x00, 0x1f);
	F2MC16_PORT(mconfig, m_port[0x8], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x9], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0xa], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0xb], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0xc], 0x00, 0xff);

	F2MC16_INTC(mconfig, m_intc);
	set_irq_acknowledge_callback(m_intc, FUNC(f2mc16_intc_device::irq_acknowledge_callback));
	F2MC16_CLOCK_GENERATOR(mconfig, m_clock_generator, DERIVED_CLOCK(1, 1), m_intc, 34);
	F2MC16_ADC(mconfig, m_adc, DERIVED_CLOCK(1, 1), m_intc, 11);
	m_intc->i2osclr<0x1f>().set(m_adc, FUNC(f2mc16_adc_device::i2osclr));
	F2MC16_UART(mconfig, m_uart[0], DERIVED_CLOCK(1, 1), m_intc, 39, 40);
	F2MC16_UART(mconfig, m_uart[1], DERIVED_CLOCK(1, 1), m_intc, 37, 38);
	F2MC16_PPG(mconfig, m_ppg, DERIVED_CLOCK(1, 1), m_intc, 26, 28);
	m_clock_generator->timebase_hz().set(m_ppg, FUNC(f2mc16_ppg_device::timebase_hz));
}

void mb90570_device::internal_map(address_map &map)
{
	//*7: This register is only available when the address/data bus is in multiplex mode. Access to the register is prohibited in non-multiplex mode.
	//*8: This register is only available when the external data bus is in 8-bit mode. Access to the register is prohibited in 16-bit mode.
	map(0x0000, 0x0000).rw(m_port[0x0], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 0 data register
	map(0x0001, 0x0001).rw(m_port[0x1], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 1 data register
	map(0x0002, 0x0002).rw(m_port[0x2], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 2 data register
	map(0x0003, 0x0003).rw(m_port[0x3], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 3 data register
	map(0x0004, 0x0004).rw(m_port[0x4], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 4 data register
	map(0x0005, 0x0005).rw(m_port[0x5], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 5 data register
	map(0x0006, 0x0006).rw(m_port[0x6], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 6 data register
	map(0x0007, 0x0007).rw(m_port[0x7], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 7 data register
	map(0x0008, 0x0008).rw(m_port[0x8], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 8 data register
	map(0x0009, 0x0009).rw(m_port[0x9], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 9 data register
	map(0x000a, 0x000a).rw(m_port[0xa], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port a data register
	map(0x000b, 0x000b).rw(m_port[0xa], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port b data register
	map(0x000c, 0x000c).rw(m_port[0xa], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port c data register
	map(0x0010, 0x0010).rw(m_port[0x0], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 1 direction register
	map(0x0011, 0x0011).rw(m_port[0x1], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 1 direction register
	map(0x0012, 0x0012).rw(m_port[0x2], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 2 direction register
	map(0x0013, 0x0013).rw(m_port[0x3], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 3 direction register
	map(0x0014, 0x0014).rw(m_port[0x4], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 4 direction register
	map(0x0015, 0x0015).rw(m_port[0x5], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 5 direction register
	map(0x0016, 0x0016).rw(m_port[0x6], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 6 direction register
	map(0x0017, 0x0017).rw(m_port[0x7], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 7 direction register
	map(0x0018, 0x0018).rw(m_port[0x8], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 8 direction register
	map(0x0019, 0x0019).rw(m_port[0x9], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 9 direction register
	map(0x001a, 0x001a).rw(m_port[0xa], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port a direction register
	map(0x001b, 0x001b).rw(m_port[0xb], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port b direction register
	map(0x001c, 0x001c).rw(m_port[0xc], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port c direction register
	// 0x1d odr4 Port 4 output register
	// 0x1e ader Analog input enable register
	map(0x0020, 0x0020).rw(m_uart[0], FUNC(f2mc16_uart_device::smr_r), FUNC(f2mc16_uart_device::smr_w)); // Serial mode register 0
	map(0x0021, 0x0021).rw(m_uart[0], FUNC(f2mc16_uart_device::scr_r), FUNC(f2mc16_uart_device::scr_w)); // Serial control register 0
	map(0x0022, 0x0022).rw(m_uart[0], FUNC(f2mc16_uart_device::sidr_r), FUNC(f2mc16_uart_device::sodr_w)); // Serial input data register 0/Serial output data register 0
	map(0x0023, 0x0023).rw(m_uart[0], FUNC(f2mc16_uart_device::ssr_r), FUNC(f2mc16_uart_device::ssr_w)); // Serial control register 0
	map(0x0024, 0x0024).rw(m_uart[1], FUNC(f2mc16_uart_device::smr_r), FUNC(f2mc16_uart_device::smr_w)); // Serial mode register 1
	map(0x0025, 0x0025).rw(m_uart[1], FUNC(f2mc16_uart_device::scr_r), FUNC(f2mc16_uart_device::scr_w)); // Serial control register 1
	map(0x0026, 0x0026).rw(m_uart[1], FUNC(f2mc16_uart_device::sidr_r), FUNC(f2mc16_uart_device::sodr_w)); // Serial input data register 1/Serial output data register 1
	map(0x0027, 0x0027).rw(m_uart[1], FUNC(f2mc16_uart_device::ssr_r), FUNC(f2mc16_uart_device::ssr_w)); // Serial control register 1
	map(0x0028, 0x0028).w(m_uart[0], FUNC(f2mc16_uart_device::cdcr_w)); // UART0 (uart) machine clock division control register
	map(0x002a, 0x002a).w(m_uart[1], FUNC(f2mc16_uart_device::cdcr_w)); // UART1 (uart) machine clock division control register
	map(0x0030, 0x0030).rw(m_intc, FUNC(f2mc16_intc_device::enir_r), FUNC(f2mc16_intc_device::enir_w)); // Interrupt/DTP enable register
	map(0x0031, 0x0031).rw(m_intc, FUNC(f2mc16_intc_device::eirr_r), FUNC(f2mc16_intc_device::eirr_w)); // Interrupt/DTP source register
	map(0x0032, 0x0033).rw(m_intc, FUNC(f2mc16_intc_device::elvr_r), FUNC(f2mc16_intc_device::elvr_w)); // Interrupt level setting register
	map(0x0036, 0x0037).rw(m_adc, FUNC(f2mc16_adc_device::adcs_r), FUNC(f2mc16_adc_device::adcs_w)); // AD control status register
	map(0x0038, 0x0039).rw(m_adc, FUNC(f2mc16_adc_device::adcr_r), FUNC(f2mc16_adc_device::adcr_w)); // AD data register
	// 0x3a dadr0 d/a converter data register ch.0
	// 0x3b dadr1 d/a converter data register ch.1
	// 0x3c dacr0 d/a control register ch.0
	// 0x3d dacr1 d/a control register ch.1
	// 0x3e clkr Clock output enable register
	map(0x0040, 0x0041).rw(m_ppg, FUNC(f2mc16_ppg_device::prl_r<0>), FUNC(f2mc16_ppg_device::prl_w<0>)); // reload register
	map(0x0042, 0x0043).rw(m_ppg, FUNC(f2mc16_ppg_device::prl_r<1>), FUNC(f2mc16_ppg_device::prl_w<1>)); // reload register
	map(0x0044, 0x0045).rw(m_ppg, FUNC(f2mc16_ppg_device::ppgc_r), FUNC(f2mc16_ppg_device::ppgc_w)); // operation mode control register
	// 0x46 ppgoe PPG0 and 1 output control registers ch.0 and ch.1
	// 0x48 smcsl0 Serial mode control lower status register 0
	// 0x49 smcsh0 Serial mode control upper status register 0
	// 0x4a sdr0 Serial data register 0
	// 0x4c smcsl1 Serial mode control lower status register 1
	// 0x4d smcsh1 Serial mode control upper status register 1
	// 0x4e sdr1 Serial data register 1
	// 0x50, 0x51 ipcp0 ICU data register ch.0
	// 0x52, 0x53 ipcp1 ICU data register ch.1
	// 0x54 ics01 ICU control status register
	// 0x56, 0x57 tcdt Free run timer data register
	// 0x58 tccs Free run timer control status register
	// 0x5a, 0x5b occp0 OCU compare register ch.0
	// 0x5c, 0x5d occp1 OCU compare register ch.1
	// 0x5e, 0x5f occp2 OCU compare register ch.2
	// 0x60, 0x61 occp3 OCU compare register ch.3
	// 0x62 ocs0 OCU control status register ch.0
	// 0x63 ocs1 OCU control status register ch.1
	// 0x64 ocs2 OCU control status register ch.2
	// 0x65 ocs3 OCU control status register ch.3
	// 0x68 ibsr I2C bus status register
	// 0x69 ibcr I2C bus control register
	// 0x6a iccr I2C bus clock control register
	// 0x6b iadr I2C bus address register
	// 0x6c idar I2C bus data register
	// 0x6f romm ROM mirroring function selection register
	// 0x70 udcr0 Up/down count register 0
	// 0x71 udcr1 Up/down count register 1
	// 0x72 rcr0 Reload compare register 0
	// 0x73 rcr1 Reload compare register 1
	// 0x74 csr0 Counter status register 0
	// 0x76, 0x77 ccr0/ccr0 Counter control register 0
	// 0x78 csr1 Counter status register 1
	// 0x7a, 0x7b ccr1/ccr1 Counter control register 1
	// 0x7c smcsl2 Serial mode control lower status register 2
	// 0x7d smcsh2 Serial mode control higher status register 2
	// 0x7e Serial data register 2
	map(0x0080, 0x0087).ram(); // CSCR0-CSCR6
	// 0x9e pacsr Program address detection control status register
	map(0x009f, 0x009f).rw(m_intc, FUNC(f2mc16_intc_device::dirr_r), FUNC(f2mc16_intc_device::dirr_w)); // Delay interrupt generate/release register
	map(0x00a0, 0x00a0).rw(m_clock_generator, FUNC(f2mc16_clock_generator_device::lpmcr_r), FUNC(f2mc16_clock_generator_device::lpmcr_w)); // Low power mode control register
	map(0x00a1, 0x00a1).rw(m_clock_generator, FUNC(f2mc16_clock_generator_device::ckscr_r), FUNC(f2mc16_clock_generator_device::ckscr_w)); // Clock selection register
	// 0x00a5 arsr Auto-ready function selection register
	// 0x00a6 hacr External address output control register
	// 0x00a7 ecsr Bus control signal selection register
	map(0x00a8, 0x00a8).rw(m_clock_generator, FUNC(f2mc16_clock_generator_device::wdtc_r), FUNC(f2mc16_clock_generator_device::wdtc_w)); // Watchdog timer control register
	map(0x00a9, 0x00a9).rw(m_clock_generator, FUNC(f2mc16_clock_generator_device::tbtc_r), FUNC(f2mc16_clock_generator_device::tbtc_w));
	// 0xaa wtc Clock timer control register
	// 0xae fmcs Flash control register
	map(0x00b0, 0x00bf).rw(m_intc, FUNC(f2mc16_intc_device::icr_r), FUNC(f2mc16_intc_device::icr_w));
	map(0x0100, m_internal_ram_end).ram();
	// 0x1ff0, 0x1ff2 padr0
	// 0x1ff3, 0x1ff5 padr1
}
