#include "emu.h"
#include "h8_watchdog.h"

const device_type H8_WATCHDOG = &device_creator<h8_watchdog_device>;

const int h8_watchdog_device::div_bh[8] = { 1, 6, 7, 9, 11, 13, 15, 17 };
const int h8_watchdog_device::div_s [8] = { 1, 5, 6, 7,  8,  9, 11, 12 };

h8_watchdog_device::h8_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, H8_WATCHDOG, "H8 watchdog", tag, owner, clock, "h8_watchdog", __FILE__),
	cpu(*this, DEVICE_SELF_OWNER)
{
}

void h8_watchdog_device::set_info(const char *_intc_tag, int _irq, int _type)
{
	intc_tag = _intc_tag;
	irq = _irq;
	type = _type;
}


UINT64 h8_watchdog_device::internal_update(UINT64 current_time)
{
	tcnt_update(current_time);
	if(tcsr & TCSR_TME) {
		int shift = (type == S ? div_s : div_bh)[tcsr & TCSR_CKS];
		UINT64 spos = tcnt_cycle_base >> shift;
		return (spos + 0x100 - tcnt) << shift;
		
	} else
		return 0;
}

void h8_watchdog_device::tcnt_update(UINT64 cur_time)
{
	if(tcsr & TCSR_TME) {
		int shift = (type == S ? div_s : div_bh)[tcsr & TCSR_CKS];
		if(!cur_time)
			cur_time = cpu->total_cycles();
		UINT64 spos = tcnt_cycle_base >> shift;
		UINT64 epos = cur_time >> shift;

		int next_tcnt = tcnt + int(epos - spos);
		tcnt = next_tcnt;
		tcnt_cycle_base = cur_time;
		//		logerror("%10lld tcnt %02x -> %03x shift=%d\n", cur_time, tcnt, next_tcnt, shift);

		if(next_tcnt >= 0x100) {
			logerror("watchdog triggered\n");
			if(tcsr & TCSR_WT) {
				if(type == B && !(tcsr & TCSR_NMI))
					intc->internal_interrupt(3);
				else
					cpu->reset();
			} else {
				if(!(tcsr & TCSR_OVF)) {
					tcsr |= TCSR_OVF;
					intc->internal_interrupt(irq);
				}				
			}
		}
	} else
		tcnt = 0;

}

READ16_MEMBER(h8_watchdog_device::wd_r)
{
	tcnt_update();

	logerror("read\n");
	return 0;
}

WRITE16_MEMBER(h8_watchdog_device::wd_w)
{
	if(mem_mask != 0xffff)
		return;

	if((data & 0xff00) == 0xa500) {
		tcnt_update();
		if(!(tcsr & TCSR_TME) && (data & TCSR_TME))
			tcnt_cycle_base = cpu->total_cycles();
		tcsr = data & 0xff;
		tcsr |= type == B ? 0x10 : 0x18;
		cpu->internal_update();
	}

	if((data & 0xff00) == 0x5a00) {
		if(tcsr & TCSR_TME) {
			tcnt = data & 0xff;
			tcnt_cycle_base = cpu->total_cycles();
			//			logerror("%10lld tcnt = %02x\n", tcnt_cycle_base, tcnt);
		}
		cpu->internal_update();
	}
}

READ16_MEMBER(h8_watchdog_device::rst_r)
{
	logerror("rst_r\n");
	return 0;
}

WRITE16_MEMBER(h8_watchdog_device::rst_w)
{
	if((data & 0xff00) == 0xa500)
		logerror("wowf_w %02x\n", data & 0xff);
	if((data & 0xff00) == 0x5a00)
		logerror("rtse_w %02x\n", data & 0xff);
}

void h8_watchdog_device::device_start()
{
	intc = siblingdevice<h8_intc_device>(intc_tag);
}

void h8_watchdog_device::device_reset()
{
	tcnt = 0x00;
	tcnt_cycle_base = cpu->total_cycles();
	tcsr = type == B ? 0x10 : 0x18;
	rst = type == S ? 0x1f : 0x3f;
}
