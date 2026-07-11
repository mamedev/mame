// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_refresh.h

    H8/3006 / H8/3007 refresh controller compare-match timer.

    The H8/3007's DRAM refresh controller contains an 8-bit free-running
    timer (RTCNT, @0xee029) with a compare register (RTCOR, @0xee02a) and a
    SINGLE combined control/status register (RTMCSR, @0xee028 -- CMF/CMIE/
    CKS2-0 all in one byte, per hardware manual sec. 6.1.4/6.2.9). On compare
    match it can raise an interrupt (vector 21 on the H8/3007).  Flash Beats
    (sega/flashbeats.cpp) uses this as its periodic system tick (confirmed by
    disassembly: RTCOR<-0x7d, RTMCSR<-0x2f i.e. CKS=5, /512, giving a
    ~4.03ms tick at 16MHz); without it the main CPU spins forever waiting on
    a software tick counter and the game never boots.

    Only the compare-match-interrupt behaviour is modelled here (enough to
    drive the periodic interrupt); the actual DRAM refresh strobe is not
    relevant under emulation.

***************************************************************************/

#ifndef MAME_CPU_H8_H8_REFRESH_H
#define MAME_CPU_H8_H8_REFRESH_H

#pragma once

#include "h8.h"
#include "h8_intc.h"

class h8_refresh_device : public device_t {
public:
	h8_refresh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template<typename T, typename U> h8_refresh_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, U &&intc, int irq)
		: h8_refresh_device(mconfig, tag, owner, 0)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_irq = irq;
	}

	// called by the owning cpu device to advance the timer in cycle time
	u64 internal_update(u64 current_time);
	void notify_standby(int state);

	u8 rtcnt_r();
	void rtcnt_w(u8 data);
	u8 rtcor_r();
	void rtcor_w(u8 data);
	u8 rtmcsr_r();
	void rtmcsr_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<h8_device> m_cpu;
	required_device<h8_intc_device> m_intc;
	int m_irq;

	u8 m_rtcnt, m_rtcor, m_rtmcsr;
	u64 m_cycle_base;

	void rtcnt_update(u64 current_time = 0);
	u8 rtcnt_peek() const;
	bool cmf_peek() const;
};

DECLARE_DEVICE_TYPE(H8_REFRESH, h8_refresh_device)

#endif // MAME_CPU_H8_H8_REFRESH_H
