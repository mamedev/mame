// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Motorola M68HC05E1/E4/etc. 8-bit microcontroller family
*/

#include "emu.h"
#include "m68hc05e1.h"
#include "m6805defs.h"
#include "6805dasm.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(M68HC05E1, m68hc05e1_device, "m68hc05e1", "Motorola M68HC05E1")
DEFINE_DEVICE_TYPE(M68HC05E5, m68hc05e5_device, "m68hc05e5", "Motorola M68HC05E5")

constexpr int M68HC05EX_INT_IRQ = M6805_IRQ_LINE;
constexpr int M68HC05EX_INT_TIMER = M68HC05EX_INT_IRQ + 1;
constexpr int M68HC05EX_INT_CPI = M68HC05EX_INT_IRQ + 2;

m68hc05ex_device::m68hc05ex_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, address_map_constructor internal_map) :
	m6805_base_device(mconfig, tag, owner, clock, type, {s_hc_s_ops, s_hc_cycles, 13, 0x00ff, 0x00c0, 0xfffc}),
	m_program_config("program", ENDIANNESS_BIG, 8, addrbits, 0, internal_map),
	m_read_p(*this, 0),
	m_write_p(*this),
	m_pll_ctrl(0), m_timer_ctrl(0), m_onesec(0)
{
	std::fill(std::begin(m_pullups), std::end(m_pullups), 0);
}

void m68hc05ex_device::device_start()
{
	m6805_base_device::device_start();

	save_item(NAME(m_ports));
	save_item(NAME(m_ddrs));
	save_item(NAME(m_pll_ctrl));
	save_item(NAME(m_timer_ctrl));
	save_item(NAME(m_onesec));

	memset(m_ports, 0, sizeof(m_ports));
	memset(m_ddrs, 0, sizeof(m_ddrs));

	m_timer = timer_alloc(FUNC(m68hc05ex_device::seconds_tick), this);
	m_prog_timer = timer_alloc(FUNC(m68hc05ex_device::timer_tick), this);
}

void m68hc05ex_device::device_reset()
{
	m6805_base_device::device_reset();
	rm16<false>(0x1ffe, m_pc);

	// all ports reset to input on startup
	memset(m_ports, 0, sizeof(m_ports));
	memset(m_ddrs, 0, sizeof(m_ddrs));
}

device_memory_interface::space_config_vector m68hc05ex_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void m68hc05ex_device::interrupt_vector()
{
	if (BIT(m_pending_interrupts, M68HC05EX_INT_IRQ))
	{
		m_pending_interrupts &= ~(1 << M68HC05EX_INT_IRQ);
		rm16<false>(0x1ffa, m_pc);
	}
	else if (BIT(m_pending_interrupts, M68HC05EX_INT_TIMER))
	{
		m_pending_interrupts &= ~(1 << M68HC05EX_INT_TIMER);
		rm16<false>(0x1ff8, m_pc);
	}
	else if (BIT(m_pending_interrupts, M68HC05EX_INT_CPI))
	{
		m_pending_interrupts &= ~(1 << M68HC05EX_INT_CPI);
		rm16<false>(0x1ff6, m_pc);
	}
}

u64 m68hc05ex_device::execute_clocks_to_cycles(u64 clocks) const noexcept
{
	return (clocks + 1) / 2;
}

u64 m68hc05ex_device::execute_cycles_to_clocks(u64 cycles) const noexcept
{
	return cycles * 2;
}

std::unique_ptr<util::disasm_interface> m68hc05ex_device::create_disassembler()
{
	return std::make_unique<m68hc05_disassembler>();
}

void m68hc05ex_device::send_port(u8 offset, u8 data)
{
	m_write_p[offset](data);
}

u8 m68hc05ex_device::ports_r(offs_t offset)
{
	u8 incoming = m_read_p[offset]();

	// apply data direction registers
	incoming &= (m_ddrs[offset] ^ 0xff);
	// OR in ddr-masked version of port writes
	incoming |= (m_ports[offset] & m_ddrs[offset]);

	return incoming;
}

void m68hc05ex_device::ports_w(offs_t offset, u8 data)
{
	send_port(offset, (data & m_ddrs[offset]) | (m_pullups[offset] & ~m_ddrs[offset]));
	m_ports[offset] = data;
}

u8 m68hc05ex_device::ddrs_r(offs_t offset)
{
	return m_ddrs[offset];
}

void m68hc05ex_device::ddrs_w(offs_t offset, u8 data)
{
	send_port(offset, (m_ports[offset] & data) | (m_pullups[offset] & ~data));
	m_ddrs[offset] = data;
}

u8 m68hc05ex_device::pll_r()
{
	return m_pll_ctrl;
}

void m68hc05ex_device::pll_w(u8 data)
{
	// Motorola documentation for both the 68HC05E1 and E5 says that rate 3 (4 MHz) is illegal.
	// The Cuda code sets it to 2 MHz, but comments in the code as well as the cycle counts in
	// the ADB routines indicate the CPU is intended to run at 4.2 MHz, not 2.1.
	// So we do this little cheat.
	if ((data & 3) == 2)
	{
		data |= 3;
	}

	if (m_pll_ctrl != data)
	{
		static const int clocks[4] = {524288, 1048576, 2097152, 4194304};
		LOG("PLL ctrl: clock %d TCS:%d BCS:%d AUTO:%d BWC:%d PLLON:%d (PC=%x)\n", clocks[data & 3],
			(data & 0x80) ? 1 : 0,
			(data & 0x40) ? 1 : 0,
			(data & 0x20) ? 1 : 0,
			(data & 0x10) ? 1 : 0,
			(data & 0x08) ? 1 : 0, pc());

		m_prog_timer->adjust(attotime::from_hz(clocks[data & 3] / 1024), 0, attotime::from_hz(clocks[data & 3] / 1024));
	}

	m_pll_ctrl = data;
}

u8 m68hc05ex_device::timer_ctrl_r()
{
	return m_timer_ctrl;
}

void m68hc05ex_device::timer_ctrl_w(u8 data)
{
	if ((m_timer_ctrl & 0x80) && !(data & 0x80))
	{
		set_input_line(M68HC05EX_INT_TIMER, CLEAR_LINE);
		m_timer_ctrl &= ~0x80;
	}
	else if ((m_timer_ctrl & 0x40) && !(data & 0x40))
	{
		set_input_line(M68HC05EX_INT_TIMER, CLEAR_LINE);
		m_timer_ctrl &= ~0x40;
	}

	m_timer_ctrl &= 0xc0;
	m_timer_ctrl |= (data & ~0xc0);
}

u8 m68hc05ex_device::timer_counter_r()
{
	// this returns an always-incrementing 8-bit value incremented at 1/4th of the CPU's clock rate.
	return (total_cycles() / 4) % 256;
}

u8 m68hc05ex_device::onesec_r()
{
	return m_onesec;
}

void m68hc05ex_device::onesec_w(u8 data)
{
	m_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	if ((m_onesec & 0x40) && !(data & 0x40))
	{
		set_input_line(M68HC05EX_INT_CPI, CLEAR_LINE);
	}

	m_onesec = data;
}

TIMER_CALLBACK_MEMBER(m68hc05ex_device::seconds_tick)
{
	m_onesec |= 0x40;

	if (m_onesec & 0x10)
	{
		set_input_line(M68HC05EX_INT_CPI, ASSERT_LINE);
	}
}

TIMER_CALLBACK_MEMBER(m68hc05ex_device::timer_tick)
{
	m_timer_ctrl |= 0x80;

	if (m_timer_ctrl & 0x20)
	{
		set_input_line(M68HC05EX_INT_TIMER, ASSERT_LINE);
	}
}

// M68HC05E1
void m68hc05e1_device::m68hc05e1_map(address_map &map)
{
	map(0x0000, 0x0002).rw(FUNC(m68hc05e1_device::ports_r), FUNC(m68hc05e1_device::ports_w));
	map(0x0004, 0x0006).rw(FUNC(m68hc05e1_device::ddrs_r), FUNC(m68hc05e1_device::ddrs_w));
	map(0x0007, 0x0007).rw(FUNC(m68hc05e1_device::pll_r), FUNC(m68hc05e1_device::pll_w));
	map(0x0008, 0x0008).rw(FUNC(m68hc05e1_device::timer_ctrl_r), FUNC(m68hc05e1_device::timer_ctrl_w));
	map(0x0009, 0x0009).r(FUNC(m68hc05e1_device::timer_counter_r));
	map(0x0012, 0x0012).rw(FUNC(m68hc05e1_device::onesec_r), FUNC(m68hc05e1_device::onesec_w));
	map(0x0090, 0x01ff).ram().share(m_internal_ram); // work RAM and stack
	map(0x0f00, 0x1fff).rom().region(DEVICE_SELF, 0);
}

m68hc05e1_device::m68hc05e1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	m68hc05ex_device(mconfig, M68HC05E1, tag, owner, clock, 13, address_map_constructor(FUNC(m68hc05e1_device::m68hc05e1_map), this)),
	m_internal_ram(*this, "internal_ram")
{
}

m68hc05e1_device::m68hc05e1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, address_map_constructor internal_map) :
	m68hc05ex_device(mconfig, type, tag, owner, clock, 13, internal_map),
	m_internal_ram(*this, "internal_ram")
{
}

u8 m68hc05e1_device::read_internal_ram(offs_t offset)
{
	return m_internal_ram[offset];
}

void m68hc05e1_device::write_internal_ram(offs_t offset, u8 data)
{
	m_internal_ram[offset] = data;
}

// M68HC05E5 - Same as E1 with more ROM and SPI and I2C hardware support
void m68hc05e5_device::m68hc05e5_map(address_map &map)
{
	map(0x0000, 0x0002).rw(FUNC(m68hc05e5_device::ports_r), FUNC(m68hc05e5_device::ports_w));
	map(0x0004, 0x0006).rw(FUNC(m68hc05e5_device::ddrs_r), FUNC(m68hc05e5_device::ddrs_w));
	map(0x0007, 0x0007).rw(FUNC(m68hc05e5_device::pll_r), FUNC(m68hc05e5_device::pll_w));
	map(0x0008, 0x0008).rw(FUNC(m68hc05e5_device::timer_ctrl_r), FUNC(m68hc05e5_device::timer_ctrl_w));
	map(0x0009, 0x0009).r(FUNC(m68hc05e5_device::timer_counter_r));
//  map(0x000a, 0x000c) // SSI (SPI) registers
	map(0x0012, 0x0012).rw(FUNC(m68hc05e5_device::onesec_r), FUNC(m68hc05e5_device::onesec_w));
	map(0x0090, 0x01ff).ram().share(m_internal_ram); // work RAM and stack
	map(0x0b00, 0x1fff).rom().region(DEVICE_SELF, 0);
}

m68hc05e5_device::m68hc05e5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	m68hc05e1_device(mconfig, M68HC05E5, tag, owner, clock, 13, address_map_constructor(FUNC(m68hc05e5_device::m68hc05e5_map), this)),
	m_internal_ram(*this, "internal_ram")
{
}

u8 m68hc05e5_device::read_internal_ram(offs_t offset)
{
	return m_internal_ram[offset];
}

void m68hc05e5_device::write_internal_ram(offs_t offset, u8 data)
{
	m_internal_ram[offset] = data;
}
