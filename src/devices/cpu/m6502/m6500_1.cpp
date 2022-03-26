// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    m6500_1.h

    MOS Technology 6500/1, original NMOS variant with onboard peripherals:
    * 6502 CPU
    * 2048*8 mask ROM
    * 64*8 static RAM
    * Four eight-bit open drain I/O ports
    * Sixteen-bit programmable counter/latch

    The onboad clock generator has mask options for an external crystal
    (2MHz to 6MHz) an external TTL-compatible clock with a 300Ω pull-up
    resistor (2MHz to 6MHz), or an RC oscillator with an external 47kΩ
    resistor and internal capacitor (nominally 2MHz).  The clock is
    divided by two to generate the two-phase CPU code clock.

    There is no on-board power-on reset generator.  The /RES pin must be
    held low (asserted) for at least eight phase 2 clock cycles after
    CPU core voltage reaches operating range and the clock stabilises.

    The RAM is fully static and has a separate power supply pin.  This
    allows you to assert /RES, stop the clock, and power down the CPU
    core while retaining RAM contents.

    The I/O ports have active low drivers and internal passive pull-up
    resistances.  There is a mask option to disable the internal
    pull-ups per port (i.e. per group of eight lines).  Rising edges on
    PA0 and falling edges on PA1 are detected and set bits in CR. This
    can be triggered by external circuitry or by the output drivers
    themselves.

    The sixteen-bit counter/timer counts down either on phase 2 clock or
    a rising edge on CNTR.  On overflow, the latch is transferred to the
    counter and a bit is set in CR.  The counter and latch are not
    affected by reset.  There are four counter modes:
    * 0 - interval timer: counter is free-running at clock phase 2 rate
    * 1 - pulse generator: like mode 0 but CNTR is toggled on overflow
    * 2 - event counter: counter is incremented on rising CNTR edge
    * 3 - pulse width measurement: like mode 0 gated by CNTR (low)

    According to the manual, the maximum rate that edges can be detected
    on the CNTR pin in event counter mode is half the phase 2 clock
    rate.  This suggests that an internal flag is set when a rasing edge
    is detected on CNTR and reset when the counter is synchronously
    decremented.  This is not emulated - for simplicity the counter is
    asynchronously decremented on detecting a rising edge on CNTR.

    The CNTR pin has an active low driver and internal passive pull-up.
    The pull-up can be disabled as a mask option.

    Applying +10V to the /RES pin activates test mode, redirecting
    memory fetches to port C.

    The 6570 and 6571 are compatible with the 6500/1.  Differences
    appear to include the addition of an onboard power-on reset.  It
    is unknown what other differences these devices have.

	TODO:
	- For some reason most if not all Amiga MCU programs accesses arbitrary
      zero page 0x90-0xff with a back-to-back cmp($00, x) opcode at
      PC=c06-c08 with the actual result discarded. X can be any value in
	  the 0x90-0xff range, depending on the last user keypress row source
      e.g. 0xdf-0xe0 for 'A', 0xef-0xf0 for 'Q', 0xfb-0xfc for function
	  keys.
	  This can be extremely verbose in the logging facility so we currently
      nop it out for the time being.

***************************************************************************/

#include "emu.h"
#include "m6500_1.h"


namespace {

constexpr u8 CR_CMC0    = 0x01U;
constexpr u8 CR_CMC1    = 0x02U;
constexpr u8 CR_A1IE    = 0x04U;
constexpr u8 CR_A0IE    = 0x08U;
constexpr u8 CR_CIE     = 0x10U;
constexpr u8 CR_A1ED    = 0x20U;
constexpr u8 CR_A0ED    = 0x40U;
constexpr u8 CR_CTRO    = 0x80U;

} // anonymous namespace


DEFINE_DEVICE_TYPE(M6500_1, m6500_1_device, "m6500_1", "MOS Technology 6500/1");


m6500_1_device::m6500_1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: m6502_mcu_device(mconfig, M6500_1, tag, owner, clock)
	, m_port_in_cb{ *this }
	, m_port_out_cb{ *this }
	, m_cntr_out_cb{ *this }
	, m_cr{ 0x00U }
	, m_port_in{ 0xffU, 0xffU, 0xffU, 0xffU }
	, m_port_buf{ 0xffU, 0xffU, 0xffU, 0xffU }
	, m_counter_base{ 0U }
	, m_counter{ 0x0000 }
	, m_latch{ 0x0000 }
	, m_cntr_in{ 1U }
	, m_cntr_out{ 1U }
	, m_ul{ 0U }
	, m_ll{ 0U }
	, m_uc{ 0U }
	, m_lc{ 0U }
{
	program_config.m_internal_map = address_map_constructor(FUNC(m6500_1_device::memory_map), this);
}


void m6500_1_device::pa_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(m6500_1_device::set_port_in<0>), this), unsigned(data));
}

void m6500_1_device::pb_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(m6500_1_device::set_port_in<1>), this), unsigned(data));
}

void m6500_1_device::pc_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(m6500_1_device::set_port_in<2>), this), unsigned(data));
}

void m6500_1_device::pd_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(m6500_1_device::set_port_in<3>), this), unsigned(data));
}


WRITE_LINE_MEMBER(m6500_1_device::cntr_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(m6500_1_device::set_cntr_in), this), state);
}


void m6500_1_device::device_resolve_objects()
{
	m6502_mcu_device::device_resolve_objects();

	m_port_in_cb.resolve_all();
	m_port_out_cb.resolve_all_safe();
	m_cntr_out_cb.resolve_safe();
}

void m6500_1_device::device_start()
{
	m6502_mcu_device::device_start();

	m_counter_base = 0U;

	state_add(M6500_1_CR, "CR", m_cr).callimport().callexport();
	state_add(M6500_1_UL, "UL", m_ul).callimport().callexport();
	state_add(M6500_1_LL, "LL", m_ll).callimport().callexport();
	state_add(M6500_1_UC, "UC", m_uc).callimport().callexport();
	state_add(M6500_1_LC, "LC", m_lc).callimport().callexport();

	save_item(NAME(m_cr));
	save_item(NAME(m_port_in));
	save_item(NAME(m_port_buf));
	save_item(NAME(m_counter_base));
	save_item(NAME(m_counter));
	save_item(NAME(m_latch));
	save_item(NAME(m_cntr_in));
	save_item(NAME(m_cntr_out));
}

void m6500_1_device::device_reset()
{
	m6502_mcu_device::device_reset();

	SP = 0x003fU;

	internal_update();

	m_cr = 0x00U;

	for (unsigned i = 0; std::size(m_port_buf) > i; ++i)
	{
		if (0xffU != m_port_buf[i])
			m_port_out_cb[i](m_port_buf[i] = 0xffU);
	}

	if (!m_cntr_out)
		m_cntr_out_cb(m_cntr_out = 1U);

	internal_update();
	update_irq();
}


u64 m6500_1_device::execute_clocks_to_cycles(u64 clocks) const noexcept
{
	return (clocks + 1) / 2;
}

u64 m6500_1_device::execute_cycles_to_clocks(u64 cycles) const noexcept
{
	return cycles * 2;
}


void m6500_1_device::state_import(device_state_entry const &entry)
{
	switch (entry.index())
	{
	case M6500_1_CR:
		update_irq();
		if (!pulse_generator_mode() && !m_cntr_out)
			m_cntr_out_cb(m_cntr_out = 1U);
		internal_update();
		break;

	case M6500_1_UL:
		m_latch = (m_latch & 0x00ffU) | (u16(m_ul) << 8);
		break;

	case M6500_1_LL:
		m_latch = (m_latch & 0xff00U) | u16(m_ll);
		break;

	case M6500_1_UC:
		internal_update();
		m_counter = (m_counter & 0x00ffU) | (u16(m_uc) << 8);
		internal_update();
		break;

	case M6500_1_LC:
		internal_update();
		m_counter = (m_counter & 0xff00U) | u16(m_lc);
		internal_update();
		break;

	default:
		m6502_mcu_device::state_import(entry);
	}
}

void m6500_1_device::state_export(device_state_entry const &entry)
{
	switch (entry.index())
	{
	case M6500_1_CR:
		internal_update();
		break;

	case M6500_1_UL:
		m_ul = u8(m_latch >> 8);
		break;

	case M6500_1_LL:
		m_ll = u8(m_latch);
		break;

	case M6500_1_UC:
		internal_update();
		m_uc = u8(m_counter >> 8);
		break;

	case M6500_1_LC:
		internal_update();
		m_lc = u8(m_counter);
		break;

	default:
		m6502_mcu_device::state_export(entry);
	}
}


void m6500_1_device::internal_update(u64 current_time)
{
	u64 event_time(0U);
	add_event(event_time, update_counter(current_time));
	recompute_bcount(event_time);
}


u8 m6500_1_device::read_control_register()
{
	internal_update();
	return m_cr;
}

void m6500_1_device::write_control_register(u8 data)
{
	internal_update();
	m_cr = (m_cr & (CR_A1ED | CR_A0ED | CR_CTRO)) | (data & (CR_CMC0 | CR_CMC1 | CR_A1IE | CR_A0IE | CR_CIE));
	update_irq();
	if (!pulse_generator_mode() && !m_cntr_out)
		m_cntr_out_cb(m_cntr_out = 1U);
	internal_update();
}

void m6500_1_device::update_irq()
{
	set_input_line(M6502_IRQ_LINE, (m_cr & (m_cr << 3) & (CR_A1ED | CR_A0ED | CR_CTRO)) ? ASSERT_LINE : CLEAR_LINE);
}


u8 m6500_1_device::read_port(offs_t offset)
{
	if (!machine().side_effects_disabled() && m_port_in_cb[offset])
	{
		u8 const prev(m_port_in[offset]);
		m_port_in[offset] = m_port_in_cb[offset]();
		if (!offset)
		{
			u8 const diff((prev ^ m_port_in[0]) & m_port_buf[0]);
			if (BIT(diff, 0) && BIT(m_port_in[0], 0))
				m_cr |= CR_A0ED;
			if (BIT(diff, 1) && !BIT(m_port_in[0], 1))
				m_cr |= CR_A1ED;
			update_irq();
		}
	}

	return m_port_in[offset] & m_port_buf[offset];
}

void m6500_1_device::write_port(offs_t offset, u8 data)
{
	u8 const prev(m_port_in[offset] & m_port_buf[offset]);
	if (m_port_buf[offset] != data)
		m_port_out_cb[offset](m_port_buf[offset] = data);

	if (!offset)
	{
		if (!machine().side_effects_disabled() && m_port_in_cb[0])
			m_port_in[0] = m_port_in_cb[0]();
		u8 const effective(m_port_in[0] & data);
		u8 const diff(prev ^ effective);
		if (BIT(diff, 0) && BIT(effective, 0))
			m_cr |= CR_A0ED;
		if (BIT(diff, 1) && !BIT(effective, 1))
			m_cr |= CR_A1ED;
		update_irq();
	}
}

void m6500_1_device::clear_edge(offs_t offset, u8 data)
{
	m_cr &= BIT(offset, 0) ? ~CR_A1ED : ~CR_A0ED;
	update_irq();
}

template <unsigned Port> TIMER_CALLBACK_MEMBER(m6500_1_device::set_port_in)
{
	u8 const prev(m_port_in[Port]);
	m_port_in[Port] = m_port_in_cb[Port] ? m_port_in_cb[Port]() : u8(u32(param));

	if (!Port)
	{
		u8 const diff((prev ^ m_port_in[0]) & m_port_buf[0]);
		if (BIT(diff, 0) && BIT(m_port_in[0], 0))
			m_cr |= CR_A0ED;
		if (BIT(diff, 1) && !BIT(m_port_in[0], 1))
			m_cr |= CR_A1ED;
		update_irq();
	}
}


u8 m6500_1_device::read_upper_count()
{
	internal_update();
	return u8(m_counter >> 8);
}

u8 m6500_1_device::read_lower_count()
{
	internal_update();
	if (!machine().side_effects_disabled())
	{
		m_cr &= ~CR_CTRO;
		update_irq();
	}

	return u8(m_counter);
}

template <bool Transfer> void m6500_1_device::write_upper_latch(u8 data)
{
	m_latch = (m_latch & 0x00ffU) | u16(data << 8);
	if (Transfer)
	{
		internal_update();
		m_counter = m_latch;
		m_cr &= ~CR_CTRO;
		update_irq();
		internal_update();
		toggle_cntr();
	}
}

void m6500_1_device::write_lower_latch(u8 data)
{
	m_latch = (m_latch & 0xff00U) | u16(data);
}

u64 m6500_1_device::update_counter(u64 current_time)
{
	u64 elapsed(current_time - m_counter_base);
	m_counter_base = current_time;
	if (!should_count())
		return 0U;

	if (elapsed <= m_counter)
	{
		m_counter -= elapsed;
	}
	else
	{
		m_cr |= CR_CTRO;
		elapsed -= m_counter + 1;
		u32 const period(u32(m_latch) + 1);
		u64 const events((elapsed / period) + 1);
		m_counter = u16(m_latch - (elapsed % period));
		update_irq();
		if (events % 2)
			toggle_cntr();
	}

	if (pulse_generator_mode() || (m_cr & CR_CIE))
		return current_time + m_counter + 1;
	else
		return 0U;
}

bool m6500_1_device::should_count() const
{
	switch (m_cr & (CR_CMC0 | CR_CMC1))
	{
	case 0x00U: // interval timer
	case 0x01U: // pulse generator
		return true;
	case 0x02U: // event counter
		return false;
	case 0x03U: // pulse width measurement
		assert(m_cntr_out);
		return !m_cntr_in;
	}

	// unreachable
	throw false;
}

bool m6500_1_device::pulse_generator_mode() const
{
	return (m_cr & (CR_CMC0 | CR_CMC1)) == 0x01U;
}

bool m6500_1_device::event_counter_mode() const
{
	return (m_cr & (CR_CMC0 | CR_CMC1)) == 0x02U;
}

TIMER_CALLBACK_MEMBER(m6500_1_device::set_cntr_in)
{
	if (bool(m_cntr_in) != bool(param))
	{
		internal_update();
		m_cntr_in = param ? 1U : 0U;
		if (param && event_counter_mode())
		{
			if (m_counter)
			{
				--m_counter;
			}
			else
			{
				m_cr |= CR_CTRO;
				m_counter = m_latch;
			}
		}
		internal_update();
	}
}

void m6500_1_device::toggle_cntr()
{
	if (pulse_generator_mode())
		m_cntr_out_cb(m_cntr_out = m_cntr_out ? 0U : 1U);
}


void m6500_1_device::memory_map(address_map &map)
{
	// there's probably a lot more mirroring here
	// it's likely RAM is mirrored at 0x0040-0x007f and the entire 0x0000-0x00ff repeats every 0x100 up to 0x07ff
	// this would make the decoding simple:
	// 0xxx 0xaa aaaa -> RAM
	// 0xxx 1xxx aaaa -> peripheral
	// 1aaa aaaa aaaa -> ROM

	map.global_mask(0x0fff); // guessed
	map.unmap_value_high(); // guessed

	map(0x0000, 0x003f).ram();

	map(0x0080, 0x0083).rw(FUNC(m6500_1_device::read_port), FUNC(m6500_1_device::write_port));
	map(0x0084, 0x0084).w(FUNC(m6500_1_device::write_upper_latch<false>));
	map(0x0085, 0x0085).w(FUNC(m6500_1_device::write_lower_latch));
	map(0x0086, 0x0086).r(FUNC(m6500_1_device::read_upper_count));
	map(0x0087, 0x0087).r(FUNC(m6500_1_device::read_lower_count));
	map(0x0088, 0x0088).w(FUNC(m6500_1_device::write_upper_latch<true>));
	map(0x0089, 0x008a).w(FUNC(m6500_1_device::clear_edge));

	map(0x008f, 0x008f).rw(FUNC(m6500_1_device::read_control_register), FUNC(m6500_1_device::write_control_register));

	// TODO: mirror or actually unmapped?
	map(0x0090, 0x00ff).nopr();

	map(0x0800, 0x0fff).rom().region(DEVICE_SELF, 0);
}
