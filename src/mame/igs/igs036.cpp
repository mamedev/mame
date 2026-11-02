// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "igs036.h"

#include "cpu/arm7/arm7core.h"

// unknown configuration, but uses MPU not MMU, so closer to ARM946ES
igs036_cpu_device::igs036_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm946es_cpu_device(mconfig, IGS036, tag, owner, clock, address_map_constructor(FUNC(igs036_cpu_device::onboard_peripherals), this))
{
}

void igs036_cpu_device::onboard_peripherals(address_map &map)
{
	map(0x0000'0000, 0x0000'3fff).rom().region(DEVICE_SELF, 0);

	// IGS036 system peripherals, also used by PGM2.
	//map(0xfffa'0000, 0xfffa'00c7).rw(FUNC(m036_base_state::timer_r), FUNC(m036_base_state::timer_w));
	//map(0xffff'ec00, 0xffff'ec7f).nopw(); // TODO: static memory controller
	//map(0xffff'ee00, 0xffff'ee57).nopw(); // TODO: bus matrix
	//map(0xffff'f000, 0xffff'f14b).m(m_arm_aic, FUNC(arm_aic_device::regs_map));
	//map(0xffff'f200, 0xffff'f247).nopw(); // TODO: debug UART
	//map(0xffff'f400, 0xffff'f4af).nopw(); // TODO: remaining parallel I/O registers
	//map(0xffff'f430, 0xffff'f437).w(FUNC(m036_base_state::gpio_outputs_w));
	//map(0xffff'f438, 0xffff'f43b).lr32(NAME([this] () -> u32 { return m_pio_out; }));
	//map(0xffff'f43c, 0xffff'f43f).portr(m_service);
	// Several internal ROMs verify that bit 2 reads back set after enabling
	// decryption, and stop before the checksum if it is clear.
	//map(0xffff'fa08, 0xffff'fa0b).lr32(NAME([this] () -> u32 { return m_has_decrypted ? 4U : 0U; })).w(FUNC(m036_base_state::encryption_control_w));
	map(0xffff'fa0c, 0xffff'fa0f).lr32(NAME([] () -> u32 { return 0x180; })).nopw(); // startup ready, as in pgm2.cpp
	//map(0xffff'fc00, 0xffff'fcff).rw(FUNC(m036_base_state::encryption_r), FUNC(m036_base_state::encryption_w));
	//map(0xffff'fd00, 0xffff'fd0b).nopw(); // TODO: reset controller
	//map(0xffff'fd20, 0xffff'fd27).nopw(); // TODO: real-time timer control
	map(0xffff'fd28, 0xffff'fd2b).lr32(NAME([this] () -> u32 { return machine().time().seconds(); }));
	//map(0xffff'fd40, 0xffff'fd4b).nopw(); // TODO: watchdog
}


DEFINE_DEVICE_TYPE(IGS036,       igs036_cpu_device,       "igs036",       "IGS036")
