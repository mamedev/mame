// license:BSD-3-Clause
// copyright-holders:AJR
/******************************************************************************

    Skeleton driver for Litek LMS46 SBC.

    No additional information is known about this system.

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/msm5832.h"


namespace {

class lms46_state : public driver_device
{
public:
	lms46_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
	{
	}

	void lms46(machine_config &mconfig);

private:
	u8 rtc_r(offs_t offset);
	void rtc_w(offs_t offset, u8 data);
	u8 busy_r();
	void misc_control_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<msm5832_device> m_rtc;
};


u8 lms46_state::rtc_r(offs_t offset)
{
	m_rtc->cs_w(1);
	m_rtc->read_w(1);
	m_rtc->address_w(offset);
	u8 data = m_rtc->data_r();
	m_rtc->read_w(0);
	m_rtc->cs_w(0);
	return data;
}

void lms46_state::rtc_w(offs_t offset, u8 data)
{
	m_rtc->cs_w(1);
	m_rtc->address_w(offset);
	m_rtc->data_w(data & 0x0f);
	m_rtc->write_w(1);
	m_rtc->write_w(0);
	m_rtc->cs_w(0);
}

u8 lms46_state::busy_r()
{
	// Code loops endlessly until bit 0 clears
	return 0;
}

void lms46_state::misc_control_w(u8 data)
{
	m_rtc->hold_w(BIT(data, 5));
}

void lms46_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("bios", 0).nopw();
	map(0x8000, 0x87ff).ram(); // possibly NVRAM
	map(0x8800, 0x8fff).ram();
}

void lms46_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).rw(FUNC(lms46_state::rtc_r), FUNC(lms46_state::rtc_w));
	map(0x10, 0x10).r(FUNC(lms46_state::busy_r));
	map(0x20, 0x20).w(FUNC(lms46_state::misc_control_w));
	map(0x30, 0x30).nopr(); // Watchdog reset? (value unused)
}


static INPUT_PORTS_START(lms46)
INPUT_PORTS_END

void lms46_state::lms46(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &lms46_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lms46_state::io_map);

	MSM5832(config, m_rtc, 32768);
}


ROM_START(lms46)
	ROM_REGION(0x4000, "bios", 0)
	// Original dump has been extensively hand-patched between 0000 and 00EB where bit 3 was stuck high.
	// More single-bit errors in this area are likely, and some current guesses might be wrong.
	// Code organization also suggests the use of two separate 8K ROMs rather than one 16K ROM.
	ROM_LOAD("lms4002.rom", 0x0000, 0x4000, CRC(d9bc2384) SHA1(979038c53f5611bb9078d6a44e1c521093207881) BAD_DUMP)
ROM_END

} // anonymous namespace


COMP(1988, lms46, 0, 0, lms46, lms46, lms46_state, empty_init, "Litek Information Systems", "LMS46-V9", MACHINE_IS_SKELETON)
