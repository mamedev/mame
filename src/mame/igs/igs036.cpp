// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    TODO:
    - add support for missing peripherals (not needed at the moment)
    - move igs036crypt handling into here (currently tricky due to ddpdojt using RAM not ROM)
    - figure out if the igs_m036.cpp video emulation is an IGS036 feature that PGM2 ignores
      (and if so, move handling into here too)
*/

#include "emu.h"
#include "igs036.h"

#include "cpu/arm7/arm7core.h"
#include "machine/nvram.h"

#include <algorithm>
#include <iterator>

// unknown configuration, but uses MPU not MMU, so closer to ARM946ES
igs036_cpu_device::igs036_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm946es_cpu_device(mconfig, IGS036, tag, owner, clock, address_map_constructor(FUNC(igs036_cpu_device::onboard_peripherals), this))
	, m_arm_aic(*this, "arm_aic")
	, m_read_gpio(*this, 0)
	, m_write_gpio(*this)
	, m_do_decrypt(*this)
{
}

u32 igs036_cpu_device::gpio_out_r()
{
	return m_pio_out_data;
}

u32 igs036_cpu_device::gpio_r()
{
	return m_read_gpio();
}

void igs036_cpu_device::gpio_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const previous = m_pio_out_data;
	if (offset == 0)
	{
		m_pio_out_data |= data & mem_mask;
	}
	else
	{
		m_pio_out_data &= ~(data & mem_mask);
	}

	if (m_pio_out_data != previous)
		m_write_gpio(m_pio_out_data);
}

u32 igs036_cpu_device::encryption_control_r()
{
	return m_has_decrypted ? 4U : 0U;
}

void igs036_cpu_device::encryption_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	// The internal ROM uploads 256 key bytes, then writes 4 here before reading
	// the external program. Other control bits are unknown.
	if (BIT(mem_mask, 2))
	{
		m_has_decrypted = BIT(data, 2);
		m_do_decrypt(m_has_decrypted);
	}
}

void igs036_cpu_device::onboard_peripherals(address_map &map)
{
	map(0x0000'0000, 0x0000'3fff).rom().region(DEVICE_SELF, 0);

	// this NVRAM range appears to be internal to the CPU
	map(0x0200'0000, 0x0200'ffff).ram().share("sram");

	// IGS036 system peripherals, also used by PGM2.
	map(0xfffa'0000, 0xfffa'00c7).rw(FUNC(igs036_cpu_device::timer_r), FUNC(igs036_cpu_device::timer_w));
	map(0xffff'ec00, 0xffff'ec7f).nopw(); // TODO: static memory controller
	map(0xffff'ee00, 0xffff'ee57).nopw(); // TODO: bus matrix
	map(0xffff'f000, 0xffff'f14b).m(m_arm_aic, FUNC(arm_aic_device::regs_map));
	map(0xffff'f200, 0xffff'f247).nopw(); // TODO: debug UART
	map(0xffff'f430, 0xffff'f437).w(FUNC(igs036_cpu_device::gpio_w));
	map(0xffff'f430, 0xffff'f437).r(FUNC(igs036_cpu_device::gpio_out_r));
	map(0xffff'f438, 0xffff'f43f).r(FUNC(igs036_cpu_device::gpio_r));
	// Several internal ROMs verify that bit 2 reads back set after enabling
	// decryption, and stop before the checksum if it is clear.
	map(0xffff'fa08, 0xffff'fa0b).rw(FUNC(igs036_cpu_device::encryption_control_r), FUNC(igs036_cpu_device::encryption_control_w));
	map(0xffff'fa0c, 0xffff'fa0f).lr32(NAME([] () -> u32 { return 0x180; })).nopw(); // startup ready, as in pgm2.cpp
	map(0xffff'fc00, 0xffff'fcff).rw(FUNC(igs036_cpu_device::encryption_r), FUNC(igs036_cpu_device::encryption_w));
	map(0xffff'fd00, 0xffff'fd0b).nopw(); // TODO: reset controller
	map(0xffff'fd20, 0xffff'fd27).nopw(); // TODO: real-time timer control
	map(0xffff'fd28, 0xffff'fd2b).lr32(NAME([this] () -> u32 { return machine().time().seconds(); }));
	map(0xffff'fd40, 0xffff'fd4b).nopw(); // TODO: watchdog
}

void igs036_cpu_device::device_add_mconfig(machine_config &config)
{
	arm946es_cpu_device::device_add_mconfig(config);

	ARM_AIC(config, m_arm_aic).irq_callback().set_inputline(DEVICE_SELF, ARM7_IRQ_LINE);
	NVRAM(config, "sram", nvram_device::DEFAULT_ALL_0);
}

u8 igs036_cpu_device::encryption_r(offs_t offset)
{
	return m_encryption_table[offset];
}

void igs036_cpu_device::encryption_w(offs_t offset, u8 data)
{
	m_encryption_table[offset] = data;
	// FIXME: verify key changes while decryption is enabled on hardware.
	if (m_has_decrypted)
		m_do_decrypt(1);
}


u32 igs036_cpu_device::timer_clock(u32 channel) const
{
	// Keep the peripheral time base separate from the provisional CPU PLL.
	// Scaling it with the HD core clock accelerates input scanning/debounce
	// and breaks normal coin/button operation in Hou Ye Leyuan.
	static constexpr u32 DIVISORS[4] = { 2, 8, 32, 128 };
	u32 const clock = m_tc_regs[channel][1] & 7;
	return clock < 4 ? (20_MHz_XTAL).value() / DIVISORS[clock] : clock == 4 ? 32'768 : 0;
}


void igs036_cpu_device::timer_schedule(u32 channel)
{
	u32 const clock = timer_clock(channel);
	if (m_tc_enabled[channel] && clock)
		m_tc_timer[channel]->adjust(attotime::from_ticks(m_tc_regs[channel][7] ? m_tc_regs[channel][7] : 0x10'000, clock), channel);
	else
		m_tc_timer[channel]->adjust(attotime::never);
}


TIMER_CALLBACK_MEMBER(igs036_cpu_device::timer_expired)
{
	u32 const channel = param;
	m_tc_regs[channel][8] |= 0x10; // CPCS
	if (m_tc_regs[channel][11] & 0x10)
		m_arm_aic->set_irq(6 + channel, ASSERT_LINE);
	timer_schedule(channel);
}


u32 igs036_cpu_device::timer_r(offs_t offset)
{
	u32 const channel = offset / 16;
	u32 const reg = offset % 16;
	if (channel >= 3)
		return 0;
	if (reg == 4) // CV
		return m_tc_enabled[channel] ? m_tc_timer[channel]->elapsed().as_ticks(timer_clock(channel)) & 0xffff : 0;
	if (reg == 8) // SR, interrupt flags cleared on read
	{
		u32 const result = m_tc_regs[channel][8] | (m_tc_enabled[channel] ? 0x1'0000 : 0);
		if (!machine().side_effects_disabled())
		{
			m_tc_regs[channel][8] = 0;
			m_arm_aic->set_irq(6 + channel, CLEAR_LINE);
		}
		return result;
	}
	return m_tc_regs[channel][reg];
}


void igs036_cpu_device::timer_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const channel = offset / 16;
	u32 const reg = offset % 16;
	if (channel >= 3)
	{
		if (offset == 0x30 && BIT(data & mem_mask, 0))
		{
			for (u32 i = 0; i < 3; ++i)
				timer_schedule(i);
		}
		return;
	}
	data &= mem_mask;
	switch (reg)
	{
	case 0: // CCR
		if (BIT(data, 0))
			m_tc_enabled[channel] = true;
		if (BIT(data, 1))
			m_tc_enabled[channel] = false;
		timer_schedule(channel);
		break;
	case 9: // IER
		m_tc_regs[channel][11] |= data;
		m_arm_aic->set_irq(6 + channel, (m_tc_regs[channel][8] & m_tc_regs[channel][11] & 0x10) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case 10: // IDR
		m_tc_regs[channel][11] &= ~data;
		m_arm_aic->set_irq(6 + channel, (m_tc_regs[channel][8] & m_tc_regs[channel][11] & 0x10) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case 4: case 8: case 11: // read-only
		break;
	default:
		COMBINE_DATA(&m_tc_regs[channel][reg]);
		if (reg == 1 || reg == 7)
			timer_schedule(channel);
		break;
	}
}

void igs036_cpu_device::device_start()
{
	arm946es_cpu_device::device_start();

	save_item(NAME(m_has_decrypted));
	save_item(NAME(m_encryption_table));
	save_item(NAME(m_tc_regs));
	save_item(NAME(m_tc_enabled));
	save_item(NAME(m_pio_out_data));

	for (u32 i = 0; i < std::size(m_tc_timer); ++i)
		m_tc_timer[i] = timer_alloc(FUNC(igs036_cpu_device::timer_expired), this);
}

void igs036_cpu_device::device_reset()
{
	arm946es_cpu_device::device_reset();

	std::fill(std::begin(m_encryption_table), std::end(m_encryption_table), 0);

	m_has_decrypted = false;
	m_pio_out_data = 0;

	for (u32 i = 0; i < 3; ++i)
	{
		std::fill(std::begin(m_tc_regs[i]), std::end(m_tc_regs[i]), 0);
		m_tc_enabled[i] = false;
		m_tc_timer[i]->adjust(attotime::never);
	}
}

DEFINE_DEVICE_TYPE(IGS036,       igs036_cpu_device,       "igs036",       "IGS036")
