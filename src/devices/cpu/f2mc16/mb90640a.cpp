// license:BSD-3-Clause
// copyright-holders:smf

#include "emu.h"
#include "mb90640a.h"

DEFINE_DEVICE_TYPE(MB90641A, mb90641a_device, "mb90641a", "Fujitsu MB90641A")

mb90640a_device::mb90640a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	f2mc16_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor(FUNC(mb90640a_device::internal_map), this)),
	m_clock_generator(*this, "clock_generator"),
	m_intc(*this, "intc"),
	m_port(*this, "port%x", 1U),
	m_ppg(*this, "ppg"),
	m_reload_timer(*this, "reload_timer%u", 0U),
	m_uart(*this, "uart%u", 0U)
{
}

mb90641a_device::mb90641a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mb90640a_device(mconfig, MB90641A, tag, owner, clock)
{
}

device_memory_interface::space_config_vector mb90640a_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void mb90640a_device::device_add_mconfig(machine_config &mconfig)
{
	F2MC16_PORT(mconfig, m_port[0x0], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x1], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x2], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x3], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x4], 0x00, 0xff);
	F2MC16_PORT(mconfig, m_port[0x5], 0x00, 0x3f);
	F2MC16_PORT(mconfig, m_port[0x6], 0xff, 0xff);
	F2MC16_PORT(mconfig, m_port[0x7], 0x00, 0x7f);
	F2MC16_PORT(mconfig, m_port[0x8], 0x00, 0x7f);
	F2MC16_PORT(mconfig, m_port[0x9], 0x00, 0x3f);
	F2MC16_PORT(mconfig, m_port[0xa], 0x00, 0xfe);

	F2MC16_INTC(mconfig, m_intc);
	set_irq_acknowledge_callback(m_intc, FUNC(f2mc16_intc_device::irq_acknowledge_callback));
	F2MC16_CLOCK_GENERATOR(mconfig, m_clock_generator, DERIVED_CLOCK(1, 1), m_intc, 0x22);
	F2MC16_RELOAD_TIMER(mconfig, m_reload_timer[0], DERIVED_CLOCK(1, 1), m_intc, 0x1d);
	F2MC16_RELOAD_TIMER(mconfig, m_reload_timer[1], DERIVED_CLOCK(1, 1), m_intc, 0x1e);
	F2MC16_RELOAD_TIMER(mconfig, m_reload_timer[2], DERIVED_CLOCK(1, 1), m_intc, 0x12);
	F2MC16_RELOAD_TIMER(mconfig, m_reload_timer[3], DERIVED_CLOCK(1, 1), m_intc, 0x14);
	F2MC16_RELOAD_TIMER(mconfig, m_reload_timer[4], DERIVED_CLOCK(1, 1), m_intc, 0x16);
	m_intc->i2osclr<0x1d>().set(m_reload_timer[0], FUNC(f2mc16_reload_timer_device::i2osclr));
	m_intc->i2osclr<0x1e>().set(m_reload_timer[1], FUNC(f2mc16_reload_timer_device::i2osclr));
	m_intc->i2osclr<0x12>().set(m_reload_timer[2], FUNC(f2mc16_reload_timer_device::i2osclr));
	m_intc->i2osclr<0x14>().set(m_reload_timer[3], FUNC(f2mc16_reload_timer_device::i2osclr));
	m_intc->i2osclr<0x16>().set(m_reload_timer[4], FUNC(f2mc16_reload_timer_device::i2osclr));
	F2MC16_UART(mconfig, m_uart[0], DERIVED_CLOCK(1, 1), m_intc, 0x27, 0x18);
	F2MC16_UART(mconfig, m_uart[1], DERIVED_CLOCK(1, 1), m_intc, 0x25, 0x1a);
	m_reload_timer[0]->internal_hz().set(m_uart[0], FUNC(f2mc16_uart_device::internal_timer_hz));
	m_reload_timer[0]->internal_hz().append(m_uart[1], FUNC(f2mc16_uart_device::internal_timer_hz));
	F2MC16_PPG(mconfig, m_ppg, DERIVED_CLOCK(1, 1), m_intc, 0x1b, 0x1c);
	m_clock_generator->timebase_hz().set(m_ppg, FUNC(f2mc16_ppg_device::timebase_hz));
}

void mb90640a_device::internal_map(address_map &map)
{
	map(0x0000, 0x0000).rw(m_port[0x0], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 0 data register *8
	map(0x0001, 0x0001).rw(m_port[0x1], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 1 data register *7
	map(0x0002, 0x0002).rw(m_port[0x2], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 2 data register *6
	map(0x0003, 0x0003).rw(m_port[0x3], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 3 data register *6
	map(0x0004, 0x0004).rw(m_port[0x4], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 4 data register
	map(0x0005, 0x0005).rw(m_port[0x5], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 5 data register *8
	map(0x0006, 0x0006).rw(m_port[0x6], FUNC(f2mc16_port_device::pdr_adc_r), FUNC(f2mc16_port_device::pdr_w)); // Port 6 data register
	map(0x0007, 0x0007).rw(m_port[0x7], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 7 data register
	map(0x0008, 0x0008).rw(m_port[0x8], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 8 data register
	map(0x0009, 0x0009).rw(m_port[0x9], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port 9 data register
	map(0x000a, 0x000a).rw(m_port[0xa], FUNC(f2mc16_port_device::pdr_r), FUNC(f2mc16_port_device::pdr_w)); // Port a data register *8
	map(0x0010, 0x0010).rw(m_port[0x0], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 0 direction register *8
	map(0x0011, 0x0011).rw(m_port[0x1], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 1 direction register *7
	map(0x0012, 0x0012).rw(m_port[0x2], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 2 direction register *6
	map(0x0013, 0x0013).rw(m_port[0x3], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 3 direction register *6
	map(0x0014, 0x0014).rw(m_port[0x4], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 4 direction register
	map(0x0015, 0x0015).rw(m_port[0x5], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 5 direction register *8
	map(0x0016, 0x0016).rw(m_port[0x6], FUNC(f2mc16_port_device::ader_r), FUNC(f2mc16_port_device::ader_w)); // Analog input enable register
	map(0x0017, 0x0017).rw(m_port[0x7], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 7 direction register
	map(0x0018, 0x0018).rw(m_port[0x8], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 8 direction register
	map(0x0019, 0x0019).rw(m_port[0x9], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port 9 direction register
	map(0x001a, 0x001a).rw(m_port[0xa], FUNC(f2mc16_port_device::ddr_r), FUNC(f2mc16_port_device::ddr_w)); // Port a direction register
	map(0x0020, 0x0020).rw(m_uart[0], FUNC(f2mc16_uart_device::smr_r), FUNC(f2mc16_uart_device::smr_w)); // Serial mode register 0
	map(0x0021, 0x0021).rw(m_uart[0], FUNC(f2mc16_uart_device::scr_r), FUNC(f2mc16_uart_device::scr_w)); // Serial control register 0
	map(0x0022, 0x0022).rw(m_uart[0], FUNC(f2mc16_uart_device::sidr_r), FUNC(f2mc16_uart_device::sodr_w)); // Serial input data register 0/Serial output data register 0
	map(0x0023, 0x0023).rw(m_uart[0], FUNC(f2mc16_uart_device::ssr_r), FUNC(f2mc16_uart_device::ssr_w)); // Serial control register 0
	map(0x0024, 0x0024).rw(m_uart[1], FUNC(f2mc16_uart_device::smr_r), FUNC(f2mc16_uart_device::smr_w)); // Serial mode register 1
	map(0x0025, 0x0025).rw(m_uart[1], FUNC(f2mc16_uart_device::scr_r), FUNC(f2mc16_uart_device::scr_w)); // Serial control register 1
	map(0x0026, 0x0026).rw(m_uart[1], FUNC(f2mc16_uart_device::sidr_r), FUNC(f2mc16_uart_device::sodr_w)); // Serial input data register 1/Serial output data register 1
	map(0x0027, 0x0027).rw(m_uart[1], FUNC(f2mc16_uart_device::ssr_r), FUNC(f2mc16_uart_device::ssr_w)); // Serial control register 1
	map(0x0028, 0x0028).rw(m_intc, FUNC(f2mc16_intc_device::enir_r), FUNC(f2mc16_intc_device::enir_w)); // Interrupt/DTP enable register
	map(0x0029, 0x0029).rw(m_intc, FUNC(f2mc16_intc_device::eirr_r), FUNC(f2mc16_intc_device::eirr_w)); // Interrupt/DTP source register
	map(0x002a, 0x002b).rw(m_intc, FUNC(f2mc16_intc_device::elvr_r), FUNC(f2mc16_intc_device::elvr_w)); // Interrupt level setting register
	map(0x0030, 0x0031).rw(m_ppg, FUNC(f2mc16_ppg_device::ppgc_r), FUNC(f2mc16_ppg_device::ppgc_w)); // operation mode control register
	map(0x0034, 0x0035).rw(m_ppg, FUNC(f2mc16_ppg_device::prl_r<0>), FUNC(f2mc16_ppg_device::prl_w<0>)); // reload register
	map(0x0036, 0x0037).rw(m_ppg, FUNC(f2mc16_ppg_device::prl_r<1>), FUNC(f2mc16_ppg_device::prl_w<1>)); // reload register
	map(0x0038, 0x0039).rw(m_reload_timer[0], FUNC(f2mc16_reload_timer_device::tmcsr_r), FUNC(f2mc16_reload_timer_device::tmcsr_w));
	map(0x003a, 0x003b).rw(m_reload_timer[0], FUNC(f2mc16_reload_timer_device::tmr_r), FUNC(f2mc16_reload_timer_device::tmrlr_w));
	map(0x003c, 0x003d).rw(m_reload_timer[1], FUNC(f2mc16_reload_timer_device::tmcsr_r), FUNC(f2mc16_reload_timer_device::tmcsr_w));
	map(0x003e, 0x003f).rw(m_reload_timer[1], FUNC(f2mc16_reload_timer_device::tmr_r), FUNC(f2mc16_reload_timer_device::tmrlr_w));
	map(0x0048, 0x004f).ram(); // CSCR0-CSCR7
	map(0x0051, 0x0051).w(m_uart[0], FUNC(f2mc16_uart_device::cdcr_w)); // UART0 (uart) machine clock division control register
	map(0x0053, 0x0053).w(m_uart[1], FUNC(f2mc16_uart_device::cdcr_w)); // UART1 (uart) machine clock division control register
	map(0x0058, 0x0059).rw(m_reload_timer[2], FUNC(f2mc16_reload_timer_device::tmcsr_r), FUNC(f2mc16_reload_timer_device::tmcsr_w));
	map(0x005a, 0x005b).rw(m_reload_timer[2], FUNC(f2mc16_reload_timer_device::tmr_r), FUNC(f2mc16_reload_timer_device::tmrlr_w));
	map(0x005c, 0x005d).rw(m_reload_timer[3], FUNC(f2mc16_reload_timer_device::tmcsr_r), FUNC(f2mc16_reload_timer_device::tmcsr_w));
	map(0x005e, 0x005f).rw(m_reload_timer[3], FUNC(f2mc16_reload_timer_device::tmr_r), FUNC(f2mc16_reload_timer_device::tmrlr_w));
	map(0x0060, 0x0061).rw(m_reload_timer[4], FUNC(f2mc16_reload_timer_device::tmcsr_r), FUNC(f2mc16_reload_timer_device::tmcsr_w));
	map(0x0062, 0x0063).rw(m_reload_timer[4], FUNC(f2mc16_reload_timer_device::tmr_r), FUNC(f2mc16_reload_timer_device::tmrlr_w));
	map(0x0064, 0x0067).ram(); // HACK
	//map(0x0064, 0x0066).ram(); // TPCR Timer pin control register
	map(0x006e, 0x006f).ram(); // HACK
	//map(0x006f, 0x006f).ram(); // ROMM ROM mirror function
	map(0x009f, 0x009f).rw(m_intc, FUNC(f2mc16_intc_device::dirr_r), FUNC(f2mc16_intc_device::dirr_w)); // Delay interrupt generate/release register
	map(0x00a0, 0x00a0).rw(m_clock_generator, FUNC(f2mc16_clock_generator_device::lpmcr_r), FUNC(f2mc16_clock_generator_device::lpmcr_w)); // Low power mode control register
	map(0x00a1, 0x00a1).rw(m_clock_generator, FUNC(f2mc16_clock_generator_device::ckscr_r), FUNC(f2mc16_clock_generator_device::ckscr_w)); // Clock selection register
	map(0x00a4, 0x00a7).nopw(); // HACK
	//map(0x00a5, 0x00a5).nopw(); // ARSR Auto-ready function selection register
	//map(0x00a6, 0x00a6).nopw(); // HACR External address output control register
	//map(0x00a7, 0x00a7).nopw(); // ECSR Bus control signal selection register
	map(0x00a8, 0x00a8).rw(m_clock_generator, FUNC(f2mc16_clock_generator_device::wdtc_r), FUNC(f2mc16_clock_generator_device::wdtc_w)); // Watchdog timer control register
	map(0x00a9, 0x00a9).rw(m_clock_generator, FUNC(f2mc16_clock_generator_device::tbtc_r), FUNC(f2mc16_clock_generator_device::tbtc_w));
	map(0x00b0, 0x00bf).rw(m_intc, FUNC(f2mc16_intc_device::icr_r), FUNC(f2mc16_intc_device::icr_w));
	map(0x0100, 0x08ff).ram();
}
