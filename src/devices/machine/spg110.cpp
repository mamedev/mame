// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    SunPlus SPG110-series SoC peripheral emulation

    It is possible this shares some video features with spg110 and
    can be made a derived device

**********************************************************************/

#include "emu.h"
#include "spg110.h"

DEFINE_DEVICE_TYPE(SPG110, spg110_device, "spg110", "SPG110 System-on-a-Chip")

spg110_device::spg110_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG)
{
}

spg110_device::spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg110_device(mconfig, SPG110, tag, owner, clock)
{
}

READ16_MEMBER(spg110_device::spg110_2062_r)
{
	return 0x1fff; // DMA related?
}

// irq source or similar?
READ16_MEMBER(spg110_device::spg110_2063_r)
{
	// checks for bits 0x20 and 0x08 in the IRQ function (all IRQs point to the same place)
	return 0x0008;
}

WRITE16_MEMBER(spg110_device::spg110_2063_w)
{
	// writes 0x28, probably clears the IRQ / IRQ sources? 0x63 is the same offset for this in spg2xx but bits used seem to be different
	m_cpu->set_state_unsynced(UNSP_IRQ0_LINE, CLEAR_LINE);
}

void spg110_device::map(address_map &map)
{
	map(0x000000, 0x000fff).ram();

	// vregs are at 2000?
	map(0x002060, 0x002060).nopw();
	map(0x002062, 0x002062).r(FUNC(spg110_device::spg110_2062_r)).nopw();;
	map(0x002063, 0x002063).rw(FUNC(spg110_device::spg110_2063_r),FUNC(spg110_device::spg110_2063_w));
	map(0x002066, 0x002066).nopw();

	map(0x002200, 0x0022ff).ram();
	map(0x003000, 0x0030ff).ram();
}

/*
TIMER_CALLBACK_MEMBER(spg110_device::test_timer)
{
    //
}
*/
void spg110_device::device_start()
{
//  m_test_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg110_device::test_timer), this));
}

void spg110_device::device_reset()
{
}


uint32_t spg110_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

WRITE_LINE_MEMBER(spg110_device::vblank)
{
	if (!state)
	{
		m_cpu->set_state_unsynced(UNSP_IRQ0_LINE, ASSERT_LINE);
	//  m_test_timer->adjust(attotime::from_usec(100), 0);
	}

	return;
}
