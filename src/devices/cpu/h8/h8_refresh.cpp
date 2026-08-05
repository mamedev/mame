// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_refresh.cpp

    H8/3006 / H8/3007 refresh controller compare-match timer.

    See h8_refresh.h for the rationale.  Modelled on h8_watchdog: the count
    register advances in cycle time, and the device asks the cpu for the
    next event time so the interrupt fires at exactly the right cycle.

***************************************************************************/

#include "emu.h"
#include "h8_refresh.h"

DEFINE_DEVICE_TYPE(H8_REFRESH, h8_refresh_device, "h8_refresh", "H8 refresh-timer")

namespace {

// RTMCSR bits (refresh timer control/status register, @0xee028 -- a SINGLE
// register, confirmed by disassembly of the vec21 ISR's compare-match-flag
// ack idiom: read @0xee028, AND #0x7f, write back, which only makes sense if
// CMF/CMIE/CKS all live in the same byte). Per the H8/3006/3007 hardware
// manual sec. 6.2.9: CMF=bit7, CMIE=bit6, CKS2-CKS0=bits5-3, bits2-0
// reserved (always read as 1).
enum {
	RTMCSR_CMF  = 0x80,  // compare-match flag (bit 7, hw-set/sw-clear-only)
	RTMCSR_CMIE = 0x40,  // compare-match interrupt enable (bit 6)
	RTMCSR_CKS  = 0x38,  // clock select (CKS2-CKS0, bits 5-3)
};

// phi prescaler shift for each RTMCSR CKS2-CKS0 value, per the H8/3006/3007
// hardware manual RTMCSR table (sec. 6.2.9): halted, /2, /8, /32, /128, /512,
// /2048, /4096. Confirmed against the actual ROM: at PC 0x2c8 it writes
// RTMCSR <- 0x2f (CKS=5, i.e. /512, shift 9) after priming RTCOR <- 0x7d at
// PC 0x2ba, then later (PC 0x870) ORs in CMIE (0x40) without touching CKS.
// -1 marks "halted" (RTCNT does not advance).
const int div_shift[8] = { -1, 1, 3, 5, 7, 9, 11, 12 };

} // anonymous namespace

h8_refresh_device::h8_refresh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, H8_REFRESH, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_irq(0),
	m_rtcnt(0), m_rtcor(0), m_rtmcsr(0),
	m_cycle_base(0)
{
}

u64 h8_refresh_device::internal_update(u64 current_time)
{
	rtcnt_update(current_time);
	int shift = div_shift[(m_rtmcsr & RTMCSR_CKS) >> 3];
	if(shift >= 0 && (m_rtmcsr & RTMCSR_CMIE)) {
		u64 spos = m_cycle_base >> shift;
		// next compare match is (RTCOR+1 - current count) ticks away
		return (spos + (m_rtcor + 1) - m_rtcnt) << shift;
	}
	return 0;
}

void h8_refresh_device::notify_standby(int state)
{
	if(state)
		rtcnt_update();
	else
		m_cycle_base = m_cpu->total_cycles();
}

void h8_refresh_device::rtcnt_update(u64 cur_time)
{
	int shift = div_shift[(m_rtmcsr & RTMCSR_CKS) >> 3];
	if(shift < 0)
		return; // CKS2-CKS0 = 0: no clock selected, RTCNT does not advance
	if(!cur_time)
		cur_time = m_cpu->total_cycles();

	u64 spos = m_cycle_base >> shift;
	u64 epos = cur_time >> shift;
	u64 elapsed = epos - spos;
	m_cycle_base = cur_time;

	if(elapsed == 0)
		return;

	// RTCNT counts 0..RTCOR, clears on compare match, and (if enabled) sets
	// CMF in RTMCSR (bit 7) + raises the interrupt.  Only once the game's
	// vec21 ISR clears CMF (read-then-write-0, @0xee028) does a fresh compare
	// match re-raise the interrupt.
	u32 period = m_rtcor + 1;
	u32 next = m_rtcnt + elapsed;
	if(next >= period) {
		if(!(m_rtmcsr & RTMCSR_CMF)) {
			m_rtmcsr |= RTMCSR_CMF;
			if(m_rtmcsr & RTMCSR_CMIE)
				m_intc->internal_interrupt(m_irq);
		}
		next %= period;
	}
	m_rtcnt = next;
}

// non-mutating equivalent of rtcnt_update(), for side-effects-disabled reads
// (debugger memory views etc.) -- computes what RTCNT would read as right
// now without touching m_cycle_base/m_rtcnt or raising the interrupt.
u8 h8_refresh_device::rtcnt_peek() const
{
	int shift = div_shift[(m_rtmcsr & RTMCSR_CKS) >> 3];
	if(shift < 0)
		return m_rtcnt;

	u64 spos = m_cycle_base >> shift;
	u64 epos = m_cpu->total_cycles() >> shift;
	u32 period = m_rtcor + 1;
	u32 next = m_rtcnt + (epos - spos);
	return next % period;
}

// non-mutating equivalent of the CMF-latch decision in rtcnt_update()
bool h8_refresh_device::cmf_peek() const
{
	if(m_rtmcsr & RTMCSR_CMF)
		return true;
	int shift = div_shift[(m_rtmcsr & RTMCSR_CKS) >> 3];
	if(shift < 0)
		return false;

	u64 spos = m_cycle_base >> shift;
	u64 epos = m_cpu->total_cycles() >> shift;
	u32 period = m_rtcor + 1;
	return (m_rtcnt + (epos - spos)) >= period;
}

u8 h8_refresh_device::rtcnt_r()
{
	if(machine().side_effects_disabled())
		return rtcnt_peek();
	rtcnt_update();
	return m_rtcnt;
}

void h8_refresh_device::rtcnt_w(u8 data)
{
	rtcnt_update();
	m_rtcnt = data;
	m_cpu->internal_update();
}

u8 h8_refresh_device::rtcor_r()
{
	return m_rtcor;
}

void h8_refresh_device::rtcor_w(u8 data)
{
	rtcnt_update();
	m_rtcor = data;
	m_cpu->internal_update();
}

u8 h8_refresh_device::rtmcsr_r()
{
	// bits 2-0 are reserved and always read as 1
	if(machine().side_effects_disabled())
		return (m_rtmcsr | 0x07) | (cmf_peek() ? RTMCSR_CMF : 0);
	rtcnt_update();
	return m_rtmcsr | 0x07;
}

void h8_refresh_device::rtmcsr_w(u8 data)
{
	rtcnt_update();
	// CMIE (bit 6) and CKS2-0 (bits 5-3) are directly settable. CMF (bit 7)
	// can only be cleared by software (write 0 after reading 1), never set --
	// this is the vec21 ISR's read-AND #0x7f-write idiom (@0xee028).
	m_rtmcsr = (m_rtmcsr & RTMCSR_CMF) | (data & (RTMCSR_CMIE | RTMCSR_CKS));
	if(!(data & RTMCSR_CMF))
		m_rtmcsr &= ~RTMCSR_CMF;
	// If CMIE is being enabled while CMF is already latched from an earlier
	// (unmasked) compare match, the interrupt condition is newly true right
	// now -- fire it here too, not just on the next compare match. Without
	// this, a stale unacked CMF permanently blocks rtcnt_update()'s "don't
	// refire while CMF is still set" guard and the tick interrupt deadlocks.
	if((m_rtmcsr & RTMCSR_CMF) && (m_rtmcsr & RTMCSR_CMIE))
		m_intc->internal_interrupt(m_irq);
	m_cycle_base = m_cpu->total_cycles();
	m_cpu->internal_update();
}

void h8_refresh_device::device_start()
{
	save_item(NAME(m_rtcnt));
	save_item(NAME(m_rtcor));
	save_item(NAME(m_rtmcsr));
	save_item(NAME(m_cycle_base));
}

void h8_refresh_device::device_reset()
{
	m_rtcnt = 0x00;
	m_rtcor = 0xff;
	m_rtmcsr = 0x00;
	m_cycle_base = m_cpu->total_cycles();
}
