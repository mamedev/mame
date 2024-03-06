// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_watchdog.cpp

    H8 watchdog/timer

    TODO:
    - add RSTCSR for MCUs that have it (reset is only enabled when RSTI is 1)
    - It will only clear the overflow flag when writing 0 after reading it
      when it's set? It's how the databook explains it, similar to HD6301.

***************************************************************************/

#include "emu.h"
#include "h8_watchdog.h"

DEFINE_DEVICE_TYPE(H8_WATCHDOG, h8_watchdog_device, "h8_watchdog", "H8 watchdog")

const int h8_watchdog_device::div_bh[8] = { 1, 5, 6, 7,  8,  9, 11, 12 };
const int h8_watchdog_device::div_s [8] = { 1, 6, 7, 9, 11, 13, 15, 17 };

h8_watchdog_device::h8_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, H8_WATCHDOG, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG)
{
}


u64 h8_watchdog_device::internal_update(u64 current_time)
{
	tcnt_update(current_time);
	if(m_tcsr & TCSR_TME) {
		int shift = (m_type == S ? div_s : div_bh)[m_tcsr & TCSR_CKS];
		u64 spos = m_tcnt_cycle_base >> shift;
		return (spos + 0x100 - m_tcnt) << shift;
	} else
		return 0;
}

void h8_watchdog_device::notify_standby(int state)
{
	if(state)
		tcnt_update();
	else
		m_tcnt_cycle_base = m_cpu->total_cycles();
}

void h8_watchdog_device::tcnt_update(u64 cur_time)
{
	if(m_tcsr & TCSR_TME) {
		int shift = (m_type == S ? div_s : div_bh)[m_tcsr & TCSR_CKS];
		if(!cur_time)
			cur_time = m_cpu->total_cycles();
		u64 spos = m_tcnt_cycle_base >> shift;
		u64 epos = cur_time >> shift;

		u64 next_tcnt = m_tcnt + (epos - spos);
		m_tcnt = next_tcnt;
		m_tcnt_cycle_base = cur_time;
		//logerror("%10lld tcnt %02x -> %03x shift=%d\n", cur_time, m_tcnt, next_tcnt, shift);

		if(next_tcnt >= 0x100) {
			if(m_tcsr & TCSR_WT) {
				logerror("%s watchdog triggered\n", machine().time().as_string());
				if(m_type == B && !(m_tcsr & TCSR_NMI))
					m_intc->internal_interrupt(3);
				else
					m_cpu->reset();
			} else {
				if(!(m_tcsr & TCSR_OVF)) {
					m_tcsr |= TCSR_OVF;
					m_intc->internal_interrupt(m_irq);
				}
			}
		}
	} else
		m_tcnt = 0;
}

u16 h8_watchdog_device::wd_r()
{
	if(!machine().side_effects_disabled())
		tcnt_update();

	u8 tcsr_mask = m_type == B ? 0x10 : 0x18;
	return ((m_tcsr | tcsr_mask) << 8) | m_tcnt;
}

void h8_watchdog_device::wd_w(offs_t offset, u16 data, u16 mem_mask)
{
	if(mem_mask != 0xffff)
		return;

	if((data & 0xff00) == 0xa500) {
		tcnt_update();
		if(!(m_tcsr & TCSR_TME) && (data & TCSR_TME))
			m_tcnt_cycle_base = m_cpu->total_cycles();
		m_tcsr = (m_tcsr & data & TCSR_OVF) | (data & 0x7f);
		m_cpu->internal_update();
	}

	if((data & 0xff00) == 0x5a00) {
		if(m_tcsr & TCSR_TME) {
			m_tcnt = data & 0xff;
			m_tcnt_cycle_base = m_cpu->total_cycles();
			//logerror("%10lld tcnt = %02x\n", m_tcnt_cycle_base, m_tcnt);
		}
		m_cpu->internal_update();
	}
}

u16 h8_watchdog_device::rst_r()
{
	if(!machine().side_effects_disabled())
		logerror("rst_r\n");

	u8 rst_mask = m_type == S ? 0x1f : 0x3f;
	return m_rst | rst_mask;
}

void h8_watchdog_device::rst_w(offs_t offset, u16 data, u16 mem_mask)
{
	if(mem_mask != 0xffff)
		return;

	if((data & 0xff00) == 0xa500)
		logerror("wowf_w %02x\n", data & 0xff);
	if((data & 0xff00) == 0x5a00)
		logerror("rtse_w %02x\n", data & 0xff);
}

void h8_watchdog_device::device_start()
{
	save_item(NAME(m_tcnt));
	save_item(NAME(m_tcnt_cycle_base));
	save_item(NAME(m_tcsr));
	save_item(NAME(m_rst));
}

void h8_watchdog_device::device_reset()
{
	m_tcnt_cycle_base = m_cpu->total_cycles();
	m_tcnt = 0x00;
	m_tcsr = 0x00;
	m_rst = 0x00;
}
