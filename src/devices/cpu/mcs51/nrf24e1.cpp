// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Nordic nRF24E1 wireless microcontroller. The Synopsys DW8051 CPU uses
    four-clock instruction cycles and shares its 4 KiB program RAM with
    MOVX data accesses. The integrated radio is an nRF2401.

    The undumped mask ROM bootstrap is represented by loading the external
    EEPROM image into program RAM before entering the reset vector.

    TODO:
    - Dump the bootstrap ROM and verify its exact loading and startup timing.
    - PWM output, UART pin routing and differential ADC measurements.
    - Subinstruction interrupt sampling, ADC clock phase and power-down oscillator startup.

***************************************************************************/

#include "emu.h"
#include "nrf24e1.h"

#include <algorithm>
#include <iterator>
#include <utility>

DEFINE_DEVICE_TYPE(NRF24E1, nrf24e1_device, "nrf24e1", "Nordic nRF24E1")

constexpr u8 nrf24e1_device::INSTRUCTION_CYCLES[256] =
{
	1, 3, 4, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	4, 3, 4, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	4, 3, 4, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	4, 3, 4, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 3, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 3, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 3, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 2, 3, 5, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 3, 2, 3, 5, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 3, 2, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	2, 3, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 3, 2, 1, 1, 4, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 3, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 3, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

nrf24e1_device::nrf24e1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8052_device(mconfig, NRF24E1, tag, owner, clock, 0)
	, m_radio(*this, "radio")
	, m_eeprom(*this, finder_base::DUMMY_TAG)
	, m_program_ram(*this, "program_ram")
	, m_gpio_out_cb(*this)
	, m_eeprom_read_cb(*this, 0xff)
	, m_adc_read_cb(*this, 0x0fff)
	, m_spi_clock_cb(*this)
	, m_spi_mosi_cb(*this)
	, m_spi_miso_cb(*this, 1)
{
	m_program_config = address_space_config("program", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(nrf24e1_device::ram_map), this));
	m_xdata_config = address_space_config("xdata", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(nrf24e1_device::ram_map), this));
	for (unsigned port = 0; port != 8; ++port)
	{
		m_port_in_cb[port].bind().set_constant(0xff);
		m_port_out_cb[port].bind().set_nop();
	}
}

void nrf24e1_device::ram_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share(m_program_ram);
}

void nrf24e1_device::sfr_map(address_map &map)
{
	map(0x80, 0xff).rw(FUNC(nrf24e1_device::extra_r), FUNC(nrf24e1_device::extra_w));
	i8052_device::sfr_map(map);
	map(0x80, 0x80).lrw8(NAME([this] () -> u8 { return gpio_r(0); }), NAME([this] (u8 data) { gpio_w(0, data); }));
	map(0x82, 0x86).lrw8(NAME([this] (offs_t offset) -> u8 { return extra_r(offset + SFR_DPL0); }), NAME([this] (offs_t offset, u8 data) { extra_w(offset + SFR_DPL0, data); }));
	map(0x90, 0x90).lrw8(NAME([this] () -> u8 { return gpio_r(1); }), NAME([this] (u8 data) { gpio_w(1, data); }));
	map(0xa0, 0xa0).rw(FUNC(nrf24e1_device::radio_r), FUNC(nrf24e1_device::radio_w));
	map(0xa8, 0xa8).lrw8(NAME([this] () -> u8 { return m_ie & 0xbf; }), NAME([this] (u8 data) { m_ie = data & 0xbf; m_irq_delay = 1; }));
	map(0xb0, 0xb0).unmaprw();
	map(0xb8, 0xb8).lrw8(NAME([this] () -> u8 { return m_ip; }), NAME([this] (u8 data) { m_ip = (data & 0x3f) | 0x80; m_irq_delay = 1; }));
}

void nrf24e1_device::device_add_mconfig(machine_config &config)
{
	NRF2401(config, m_radio, DERIVED_CLOCK(1, 1));
	m_radio->dr1_callback().set(FUNC(nrf24e1_device::radio_dr1_w));
	m_radio->dr2_callback().set(FUNC(nrf24e1_device::radio_dr2_w));
}

void nrf24e1_device::device_start()
{
	i8052_device::device_start();
	m_adc_timer = timer_alloc(FUNC(nrf24e1_device::adc_complete), this);
	m_spi_timer = timer_alloc(FUNC(nrf24e1_device::spi_tick), this);
	m_regx_timer = timer_alloc(FUNC(nrf24e1_device::regx_complete), this);
	m_low_power_timer = timer_alloc(FUNC(nrf24e1_device::low_power_tick), this);
	m_powered = true;
	m_booted = false;
	save_item(NAME(m_extra));
	save_item(NAME(m_other_dptr));
	save_item(NAME(m_timer_divider));
	save_item(NAME(m_previous_gpio));
	save_item(NAME(m_irq_delay));
	save_item(NAME(m_adc_sample));
	save_item(NAME(m_adc_resolution));
	save_item(NAME(m_spi_tx));
	save_item(NAME(m_spi_rx));
	save_item(NAME(m_spi_edges));
	save_item(NAME(m_spi_target));
	save_item(NAME(m_spi_divider));
	save_item(NAME(m_regx_data));
	save_item(NAME(m_rtc_reload));
	save_item(NAME(m_rtc_counter));
	save_item(NAME(m_watchdog_counter));
	save_item(NAME(m_tick_counter));
	save_item(NAME(m_low_power_phase));
	save_item(NAME(m_rtc_enabled));
	save_item(NAME(m_watchdog_enabled));
	save_item(NAME(m_powered));
	save_item(NAME(m_booted));
	save_item(NAME(m_adc_remaining));
	save_item(NAME(m_spi_remaining));
}

void nrf24e1_device::device_reset()
{
	reset_registers();
	bootstrap();
}

void nrf24e1_device::device_reset_after_children()
{
	radio_w(m_powered ? m_p2 : 0);
	update_gpio(0);
	update_gpio(1);
	m_spi_clock_cb(0);
	m_spi_mosi_cb(0);
}

void nrf24e1_device::reset_registers()
{
	i8052_device::device_reset();
	std::fill(std::begin(m_extra), std::end(m_extra), 0);
	m_extra[SFR_CKCON] = 0x01;
	m_extra[SFR_EXIF] = 0x08;
	m_extra[SFR_P0_DIR] = 0xff;
	m_extra[SFR_P1_DIR] = 0xff;
	m_extra[SFR_ADCCON] = 0x80;
	m_extra[SFR_ADCSTATIC] = 0x0a;
	m_extra[SFR_TICK_DV] = 0x1d;
	m_extra[SFR_EICON] = 0x40;
	m_extra[SFR_EIE] = 0xe0;
	m_extra[SFR_EIP] = 0xe0;
	m_p2 = 0x80;
	m_pcon = 0x30;
	m_ip = 0x80;
	m_other_dptr = 0;
	m_timer_divider = 0;
	m_previous_gpio[0] = m_previous_gpio[1] = 0xff;
	m_irq_delay = 0;
	m_adc_sample = 0;
	m_adc_resolution = 0;
	m_spi_tx = m_spi_rx = m_spi_edges = m_spi_target = m_spi_divider = 0;
	m_regx_data = m_rtc_reload = m_rtc_counter = m_watchdog_counter = m_tick_counter = 0;
	m_low_power_phase = 0;
	m_rtc_enabled = m_watchdog_enabled = false;
	m_inst_cycles = 0;
	m_adc_remaining = m_spi_remaining = attotime::never;
	m_adc_timer->adjust(attotime::never);
	m_spi_timer->adjust(attotime::never);
	m_regx_timer->adjust(attotime::never);
	m_low_power_timer->adjust(attotime::from_hz(3000), 0, attotime::from_hz(3000));
}

void nrf24e1_device::bootstrap()
{
	m_booted = false;
	if (m_eeprom.bytes() < 3)
		fatalerror("%s: invalid nRF24E1 EEPROM header\n", tag());
	auto const read_byte = [this] (unsigned address) -> u8 { return m_eeprom_read_cb.isunset() ? m_eeprom[address] : m_eeprom_read_cb(address); };
	unsigned const offset = read_byte(1);
	if (offset < 3 || offset >= m_eeprom.bytes())
	{
		// The mask ROM's failure path is undumped; do not execute stale program RAM.
		logerror("Invalid EEPROM program offset %02x; bootstrap halted\n", offset);
		return;
	}
	// Existing EEPROM programs call routines in page N, so the page count is
	// treated as inclusive. Verify this against the undumped bootstrap ROM.
	unsigned const length = std::min<unsigned>({ 0x1000, unsigned(m_eeprom.bytes() - offset), (unsigned(read_byte(2)) + 1) * 0x100 - offset });
	std::fill_n(&m_program_ram[0], 0x1000, 0);
	for (unsigned address = 0; address != length; ++address)
		m_program_ram[address] = read_byte(address + offset);
	m_extra[SFR_RSTREAS] = 0x01;
	m_pc = 0;
	m_booted = true;
}

void nrf24e1_device::power_w(int state)
{
	bool const powered = bool(state);
	if (m_powered == powered)
		return;
	m_powered = powered;
	if (powered)
	{
		reset();
	}
	else
	{
		m_booted = false;
		m_adc_remaining = m_spi_remaining = attotime::never;
		m_adc_timer->adjust(attotime::never);
		m_spi_timer->adjust(attotime::never);
		m_regx_timer->adjust(attotime::never);
		m_low_power_timer->adjust(attotime::never);
		m_radio->pwr_up_w(0);
		m_gpio_out_cb[0](0, 0, 0);
		m_gpio_out_cb[1](0, 0, 0);
		m_spi_clock_cb(0);
		m_spi_mosi_cb(0);
	}
}

u64 nrf24e1_device::execute_clocks_to_cycles(u64 clocks) const noexcept
{
	return (clocks + 3) / 4;
}

u64 nrf24e1_device::execute_cycles_to_clocks(u64 cycles) const noexcept
{
	return cycles * 4;
}

u32 nrf24e1_device::execute_max_cycles() const noexcept
{
	return 13;
}

offs_t nrf24e1_device::external_ram_iaddr(offs_t offset, offs_t mem_mask)
{
	return mem_mask == 0xff ? ((u16(m_extra[SFR_MPAGE]) << 8) | offset) : offset;
}

void nrf24e1_device::execute_run()
{
	while (m_icount > 0)
	{
		if (!m_powered || !m_booted)
		{
			debugger_wait_hook();
			m_icount = 0;
			return;
		}
		sample_gpio();
		if (m_irq_delay)
			--m_irq_delay;
		else if (!BIT(m_pcon, 1) && !BIT(m_extra[SFR_CK_CTRL], 1))
			check_interrupts();
		if (BIT(m_extra[SFR_CK_CTRL], 1))
		{
			debugger_wait_hook();
			m_icount = 0;
			return;
		}
		unsigned cycles = 1;
		if (!(m_pcon & 3))
		{
			m_ppc = m_pc;
			debugger_instruction_hook(m_pc);
			u8 const op = m_program.read_byte(m_pc++);
			cycles = INSTRUCTION_CYCLES[op];
			if ((op & 0xef) == 0xe0 || (op & 0xee) == 0xe2)
				cycles += m_extra[SFR_CKCON] & 7;
			if (op != 0xa5)
				execute_op(op);
			if (op == 0x32)
				m_irq_delay = 1;
		}
		advance_cycles(cycles);
	}
}

void nrf24e1_device::advance_cycles(unsigned cycles)
{
	m_icount -= cycles;
	while (cycles--)
	{
		m_timer_divider = (m_timer_divider + 1) % 3;
		if (BIT(m_extra[SFR_CKCON], 3) || !m_timer_divider)
			timer01_tick(0);
		if (BIT(m_extra[SFR_CKCON], 4) || !m_timer_divider)
			timer01_tick(1);
		if (m_t2con & 0x30)
			timer2_tick(2);
		else if (BIT(m_extra[SFR_CKCON], 5) || !m_timer_divider)
			timer2_tick(1);
		if (!m_timer_divider)
			transmit_receive(0);
	}
}

bool nrf24e1_device::timer01_enabled(unsigned timer) const
{
	u8 const pins = m_previous_gpio[0] | ~m_extra[SFR_P0_ALT];
	bool const running = (timer && ((m_tmod & 3) == 3)) || BIT(m_tcon, timer ? TCON_TR1 : TCON_TR0);
	return running
		&& (!timer || (m_tmod & 0x30) != 0x30)
		&& (!BIT(m_tmod, timer ? TMOD_GATE1 : TMOD_GATE0) || BIT(pins, timer + 3));
}

void nrf24e1_device::timer01_tick(unsigned timer)
{
	// The base helpers gate against interrupt flags rather than the INT pin levels.
	u8 const mode = m_tmod;
	u8 const run_mask = timer ? 0x40 : 0x10;
	u8 const run = m_tcon & run_mask;
	bool const enabled = timer01_enabled(timer);
	bool const split = timer && ((mode & 3) == 3);
	u8 const tf1 = m_tcon & 0x80;
	if (!enabled)
		(timer ? m_t1_cnt : m_t0_cnt) = 0;
	m_tcon = (m_tcon & ~run_mask) | (enabled ? run_mask : 0);
	m_tmod &= ~(timer ? 0x80 : 0x08);
	// In split mode Timer 1 retains its counter/gate inputs but cannot set TF1.
	if (split)
		m_tmod &= ~3;
	if (timer)
		update_timer_t1(1);
	else
		update_timer_t0(1);
	m_tmod = mode;
	m_tcon = (m_tcon & ~run_mask) | run;
	if (split)
		m_tcon = (m_tcon & ~0x80) | tf1;
}

void nrf24e1_device::timer2_tick(unsigned cycles)
{
	if (!BIT(m_t2con, T2CON_TR2))
	{
		m_t2_cnt = 0;
		return;
	}
	unsigned const delta = BIT(m_t2con, T2CON_CT2) ? m_t2_cnt : cycles;
	m_t2_cnt = 0;
	u32 count = m_t2 + delta;
	if (count > 0xffff)
	{
		if (m_t2con & 0x30)
			transmit_receive(2);
		else
			set_tf2(1);
		if (!BIT(m_t2con, T2CON_CP) || (m_t2con & 0x30))
			count += m_rcap2;
	}
	m_t2 = count;
}

void nrf24e1_device::sample_gpio()
{
	u8 const p0 = gpio_r(0);
	u8 const p1 = gpio_r(1);
	for (unsigned pin = 3; pin != 5; ++pin)
	{
		bool const active = BIT(m_extra[SFR_P0_ALT], pin) && !BIT(p0, pin);
		execute_set_input(pin - 3, active ? ASSERT_LINE : CLEAR_LINE);
		if (active)
			clock_control_w(m_extra[SFR_CK_CTRL] & ~2);
	}
	u8 const falling0 = m_previous_gpio[0] & ~p0;
	u8 const falling1 = m_previous_gpio[1] & ~p1;
	m_previous_gpio[0] = p0;
	m_previous_gpio[1] = p1;
	if (BIT(m_extra[SFR_CK_CTRL], 1))
		return;
	if (BIT(m_extra[SFR_P0_ALT], 5) && BIT(falling0, 5) && BIT(m_tmod, TMOD_CT0) && timer01_enabled(0))
		++m_t0_cnt;
	if (BIT(m_extra[SFR_P0_ALT], 6) && BIT(falling0, 6) && BIT(m_tmod, TMOD_CT1) && timer01_enabled(1))
		++m_t1_cnt;
	if (BIT(m_extra[SFR_P1_ALT], 0) && BIT(falling1, 0) && BIT(m_t2con, T2CON_CT2) && BIT(m_t2con, T2CON_TR2))
		++m_t2_cnt;
}

void nrf24e1_device::check_interrupts()
{
	if (!BIT(m_ie, 7))
		return;
	u16 pending = BIT(m_tcon, 1) | (BIT(m_tcon, 5) << 1) | (BIT(m_tcon, 3) << 2) | (BIT(m_tcon, 7) << 3)
		| ((m_scon & 3) ? 0x10 : 0) | ((m_t2con & 0xc0) ? 0x20 : 0);
	pending &= m_ie & 0x3f;
	pending |= (((m_extra[SFR_EXIF] >> 4) | (BIT(m_extra[SFR_EICON], 3) << 4)) & m_extra[SFR_EIE] & 0x1f) << 6;
	if (!pending)
		return;
	int selected = -1;
	int priority = -1;
	for (unsigned source = 0; source != 11; ++source)
	{
		int const level = source < 6 ? BIT(m_ip, source) : BIT(m_extra[SFR_EIP], source - 6);
		if (BIT(pending, source) && level > priority)
		{
			selected = source;
			priority = level;
		}
	}
	if (selected < 0 || (m_irq_active && priority <= m_cur_irq_prio))
		return;
	m_pcon &= ~1;
	push_pc();
	m_pc = selected < 6 ? selected * 8 + 3 : (selected - 6) * 8 + 0x43;
	m_cur_irq_prio = priority;
	m_irq_active |= 1 << priority;
	if (selected == 0 && BIT(m_tcon, 0))
		set_ie0(0);
	else if (selected == 1)
		set_tf0(0);
	else if (selected == 2 && BIT(m_tcon, 2))
		set_ie1(0);
	else if (selected == 3)
		set_tf1(0);
	advance_cycles(4);
}

u8 nrf24e1_device::extra_r(offs_t offset)
{
	// DPL/DPH SFRs name the physical pointers; DPS selects the instruction pointer.
	if (offset >= SFR_DPL0 && offset <= SFR_DPH1)
	{
		bool const selected = BIT(m_extra[SFR_DPS], 0) == BIT(offset - SFR_DPL0, 1);
		return (selected ? m_dptr : m_other_dptr) >> (8 * BIT(offset, 0));
	}
	return m_extra[offset];
}

void nrf24e1_device::extra_w(offs_t offset, u8 data)
{
	if (offset >= SFR_DPL0 && offset <= SFR_DPH1)
	{
		u16 &pointer = (BIT(m_extra[SFR_DPS], 0) == BIT(offset - SFR_DPL0, 1)) ? m_dptr : m_other_dptr;
		unsigned const shift = 8 * BIT(offset, 0);
		pointer = (pointer & ~(0xff << shift)) | (u16(data) << shift);
		return;
	}
	u8 const old = m_extra[offset];
	switch (offset)
	{
	case SFR_DPS:
		data &= 1;
		if (data != old)
			std::swap(m_dptr, m_other_dptr);
		break;
	case SFR_EXIF: data = (data & 0xf0) | 8; break;
	case SFR_ADCDATAH: case SFR_ADCDATAL: case SFR_HWREV: return;
	case SFR_SPI_DATA: spi_start(data); return;
	case SFR_REGX_CTRL: regx_start(data); return;
	case SFR_CK_CTRL: clock_control_w(data); return;
	case SFR_EICON: data = (data & 8) | 0x40; break;
	case SFR_EIE: case SFR_EIP: data |= 0xe0; m_irq_delay = 1; break;
	}
	m_extra[offset] = data;
	if (offset >= SFR_P0_DIR && offset <= SFR_P1_ALT)
		update_gpio((offset - SFR_P0_DIR) / 2);
	else if (offset == SFR_SPI_CTRL)
		update_gpio(1);
	else if (offset == SFR_ADCCON)
	{
		if (!BIT(data, 5))
		{
			m_adc_timer->adjust(attotime::never);
			m_adc_remaining = attotime::never;
		}
		else if ((BIT(data, 6) && !(old & 0x40)) || (!BIT(data, 6) && !BIT(old, 7) && BIT(data, 7)))
			adc_start();
	}
}

void nrf24e1_device::clock_control_w(u8 data)
{
	bool const stopped = BIT(data, 1);
	if (stopped != BIT(m_extra[SFR_CK_CTRL], 1))
	{
		if (stopped)
		{
			m_adc_remaining = m_adc_timer->enabled() ? m_adc_timer->remaining() : attotime::never;
			m_spi_remaining = m_spi_timer->enabled() ? m_spi_timer->remaining() : attotime::never;
			m_adc_timer->adjust(attotime::never);
			m_spi_timer->adjust(attotime::never);
		}
		else
		{
			m_adc_timer->adjust(m_adc_remaining);
			m_spi_timer->adjust(m_spi_remaining);
			m_adc_remaining = m_spi_remaining = attotime::never;
		}
	}
	m_extra[SFR_CK_CTRL] = data;
}

u8 nrf24e1_device::gpio_drive_mask(unsigned port) const
{
	u8 mask = ~m_extra[SFR_P0_DIR + port * 2];
	if (!port)
		mask = (mask & ~(m_extra[SFR_P0_ALT] & 0x7c)) | (m_extra[SFR_P0_ALT] & 0x84);
	else
	{
		mask &= 3;
		if (BIT(m_extra[SFR_P1_ALT], 0))
			mask &= ~1;
		if (m_extra[SFR_SPI_CTRL] == 1)
			mask = 3;
	}
	return mask;
}

u8 nrf24e1_device::gpio_r(unsigned port)
{
	u8 const latch = port ? m_p1 : m_p0;
	if (m_rwm)
		return latch;
	u8 const mask = gpio_drive_mask(port);
	return (latch & mask) | (m_port_in_cb[port]() & ~mask);
}

void nrf24e1_device::gpio_w(unsigned port, u8 data)
{
	(port ? m_p1 : m_p0) = data;
	update_gpio(port);
}

void nrf24e1_device::update_gpio(unsigned port)
{
	m_gpio_out_cb[port](0, port ? m_p1 : m_p0, gpio_drive_mask(port));
}

u8 nrf24e1_device::radio_r()
{
	if (m_rwm)
		return m_p2;
	return (m_p2 & 0x22) | m_radio->data_r() | (m_radio->dr1_r() << 2)
		| (m_radio->data2_r() << 4) | (m_radio->dr2_r() << 6);
}

void nrf24e1_device::radio_w(u8 data)
{
	m_p2 = data;
	m_radio->pwr_up_w(BIT(data, 7));
	m_radio->cs_w(BIT(data, 3));
	m_radio->ce_w(BIT(data, 6));
	if (m_extra[SFR_SPI_CTRL] != 2)
	{
		m_radio->data_w(BIT(data, 0));
		m_radio->clock_w(BIT(data, 1));
	}
	if (m_extra[SFR_SPI_CTRL] != 3)
		m_radio->clock2_w(BIT(data, 5));
}

void nrf24e1_device::radio_dr1_w(int state)
{
	if (state)
		m_extra[SFR_EXIF] |= 0x40;
}

void nrf24e1_device::radio_dr2_w(int state)
{
	if (state)
		m_extra[SFR_EXIF] |= 0x80;
}

void nrf24e1_device::adc_start(bool continuous)
{
	m_adc_sample = m_adc_read_cb(m_extra[SFR_ADCCON] & 0x0f) & 0x0fff;
	m_adc_resolution = m_extra[SFR_ADCSTATIC] & 3;
	u32 const divider = BIT(m_extra[SFR_ADCSTATIC], 5) ? 8 : 32;
	unsigned const cycles = (continuous ? 4 : 6) + m_adc_resolution;
	attotime const duration = clocks_to_attotime(cycles * divider);
	m_adc_remaining = BIT(m_extra[SFR_CK_CTRL], 1) ? duration : attotime::never;
	m_adc_timer->adjust(BIT(m_extra[SFR_CK_CTRL], 1) ? attotime::never : duration);
}

TIMER_CALLBACK_MEMBER(nrf24e1_device::adc_complete)
{
	u16 const result = m_adc_sample & (0x0fff << (6 - 2 * m_adc_resolution));
	m_extra[SFR_ADCDATAH] = m_adc_resolution ? result >> 4 : result >> 6;
	m_extra[SFR_ADCDATAL] = (result << 4) & 0xf0;
	m_extra[SFR_EXIF] |= 0x10;
	if ((m_extra[SFR_ADCCON] & 0x60) == 0x60)
		adc_start(true);
}

void nrf24e1_device::spi_start(u8 data)
{
	m_extra[SFR_SPI_DATA] = data;
	if (!(m_extra[SFR_SPI_CTRL] & 3))
		return;
	m_spi_tx = data;
	m_spi_rx = 0;
	m_spi_edges = 0;
	m_spi_target = m_extra[SFR_SPI_CTRL] & 3;
	m_spi_divider = 4 << (m_extra[SFR_SPICLK] & 3);
	if (m_spi_target == 1)
	{
		m_spi_clock_cb(0);
		m_spi_mosi_cb(BIT(m_spi_tx, 7));
	}
	else if (m_spi_target == 2)
		m_radio->data_w(BIT(m_spi_tx, 7));
	attotime const duration = clocks_to_attotime(m_spi_divider);
	m_spi_remaining = BIT(m_extra[SFR_CK_CTRL], 1) ? duration : attotime::never;
	m_spi_timer->adjust(BIT(m_extra[SFR_CK_CTRL], 1) ? attotime::never : duration);
}

TIMER_CALLBACK_MEMBER(nrf24e1_device::spi_tick)
{
	if (m_spi_edges == 16)
	{
		m_extra[SFR_SPI_DATA] = m_spi_rx;
		m_extra[SFR_EXIF] |= 0x20;
		return;
	}
	bool const rising = !BIT(m_spi_edges, 0);
	if (m_spi_target == 1)
		m_spi_clock_cb(rising);
	else if (m_spi_target == 2)
		m_radio->clock_w(rising);
	else
		m_radio->clock2_w(rising);
	if (rising)
	{
		int const bit = m_spi_target == 1 ? m_spi_miso_cb() : m_spi_target == 2 ? m_radio->data_r() : m_radio->data2_r();
		m_spi_rx = (m_spi_rx << 1) | bit;
	}
	else
	{
		m_spi_tx <<= 1;
		if (m_spi_target == 1)
			m_spi_mosi_cb(BIT(m_spi_tx, 7));
		else if (m_spi_target == 2)
			m_radio->data_w(BIT(m_spi_tx, 7));
	}
	if (++m_spi_edges == 16)
		m_spi_timer->adjust(clocks_to_attotime(7));
	else
		m_spi_timer->adjust(clocks_to_attotime(m_spi_divider));
}

void nrf24e1_device::regx_start(u8 data)
{
	if (BIT(m_extra[SFR_REGX_CTRL], 4))
		return;
	m_extra[SFR_REGX_CTRL] = (data & 0x0f) | 0x10;
	m_regx_data = (u16(m_extra[SFR_REGX_MSB]) << 8) | m_extra[SFR_REGX_LSB];
	m_regx_timer->adjust(attotime::from_hz(3000));
}

TIMER_CALLBACK_MEMBER(nrf24e1_device::regx_complete)
{
	u16 result = m_regx_data;
	switch (m_extra[SFR_REGX_CTRL] & 0x0f)
	{
	case 0: result = m_watchdog_counter; break;
	case 2: result = m_rtc_reload; break;
	case 3: result = m_rtc_counter; break;
	case 8: m_watchdog_counter = result; m_watchdog_enabled = true; break;
	case 10: m_rtc_reload = m_rtc_counter = result; m_rtc_enabled = true; break;
	case 11: m_rtc_enabled = false; break;
	}
	m_extra[SFR_REGX_MSB] = result >> 8;
	m_extra[SFR_REGX_LSB] = result;
	m_extra[SFR_REGX_CTRL] &= 0x0f;
}

TIMER_CALLBACK_MEMBER(nrf24e1_device::low_power_tick)
{
	if (++m_low_power_phase == 2)
	{
		m_low_power_phase = 0;
		if ((m_t2con & 0x0c) == 0x0c && !BIT(m_extra[SFR_CK_CTRL], 1))
		{
			if (!(m_t2con & 0x30))
			{
				if (BIT(m_t2con, T2CON_CP))
					m_rcap2 = m_t2;
				else
					m_t2 = m_rcap2;
			}
			set_exf2(1);
		}
	}
	if (++m_tick_counter <= m_extra[SFR_TICK_DV])
		return;
	m_tick_counter = 0;
	if (m_rtc_enabled && !m_rtc_counter--)
	{
		m_rtc_counter = m_rtc_reload;
		m_extra[SFR_EICON] |= 8;
		clock_control_w(m_extra[SFR_CK_CTRL] & ~2);
	}
	if (m_watchdog_enabled && !--m_watchdog_counter)
	{
		reset_registers();
		m_extra[SFR_RSTREAS] = 1;
		m_radio->reset();
		device_reset_after_children();
	}
}
