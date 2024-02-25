// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

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

void h8_watchdog_device::tcnt_update(u64 cur_time)
{
	if(m_tcsr & TCSR_TME) {
		int shift = (m_type == S ? div_s : div_bh)[m_tcsr & TCSR_CKS];
		if(!cur_time)
			cur_time = m_cpu->total_cycles();
		u64 spos = m_tcnt_cycle_base >> shift;
		u64 epos = cur_time >> shift;

		int next_tcnt = m_tcnt + int(epos - spos);
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
	return (m_tcsr << 8) | m_tcnt;
}

void h8_watchdog_device::wd_w(offs_t offset, u16 data, u16 mem_mask)
{
	if(mem_mask != 0xffff)
		return;

	if((data & 0xff00) == 0xa500) {
		tcnt_update();
		if(!(m_tcsr & TCSR_TME) && (data & TCSR_TME))
			m_tcnt_cycle_base = m_cpu->total_cycles();
		m_tcsr = data & 0xff;
		m_tcsr |= m_type == B ? 0x10 : 0x18;
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
	return 0;
}

void h8_watchdog_device::rst_w(u16 data)
{
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
	m_tcnt = 0x00;
	m_tcnt_cycle_base = m_cpu->total_cycles();
	m_tcsr = m_type == B ? 0x10 : 0x18;
	m_rst = m_type == S ? 0x1f : 0x3f;
}
