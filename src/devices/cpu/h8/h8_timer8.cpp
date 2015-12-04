// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8_timer8.h"

const device_type H8_TIMER8_CHANNEL  = &device_creator<h8_timer8_channel_device>;
const device_type H8H_TIMER8_CHANNEL = &device_creator<h8h_timer8_channel_device>;

h8_timer8_channel_device::h8_timer8_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, H8_TIMER8_CHANNEL, "H8 8-bits timer channel", tag, owner, clock, "h8_8bits_timer_channel", __FILE__),
	cpu(*this, "^"), chained_timer(nullptr), intc(nullptr), chain_tag(nullptr), intc_tag(nullptr), irq_ca(0), irq_cb(0), irq_v(0), chain_type(0), tcr(0), tcsr(0), tcnt(0), extra_clock_bit(false),
	has_adte(false), has_ice(false), clock_type(0), clock_divider(0), clear_type(0), counter_cycle(0), last_clock_update(0), event_time(0)
{
}

h8_timer8_channel_device::h8_timer8_channel_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	cpu(*this, "^"), chained_timer(nullptr), intc(nullptr), chain_tag(nullptr), intc_tag(nullptr), irq_ca(0), irq_cb(0), irq_v(0), chain_type(0), tcr(0), tcsr(0), tcnt(0), extra_clock_bit(false),
	has_adte(false), has_ice(false), clock_type(0), clock_divider(0), clear_type(0), counter_cycle(0), last_clock_update(0), event_time(0)
{
}

void h8_timer8_channel_device::set_info(const char *intc, int _irq_ca, int _irq_cb, int _irq_v, int div1, int div2, int div3, int div4, int div5, int div6)
{
	intc_tag = intc;
	irq_ca = _irq_ca;
	irq_cb = _irq_cb;
	irq_v = _irq_v;
	chain_tag = nullptr;
	chain_type = STOPPED;
	has_adte = false;
	has_ice = false;
	div_tab[0] = div1;
	div_tab[1] = div2;
	div_tab[2] = div3;
	div_tab[3] = div4;
	div_tab[4] = div5;
	div_tab[5] = div6;
}

READ8_MEMBER(h8_timer8_channel_device::tcr_r)
{
	return tcr;
}

WRITE8_MEMBER(h8_timer8_channel_device::tcr_w)
{
	update_counter();
	tcr = data;
	update_tcr();
	recalc_event();
}

void h8_timer8_channel_device::set_extra_clock_bit(bool bit)
{
	update_counter();
	extra_clock_bit = bit;
	update_tcr();
	recalc_event();
}

void h8_timer8_channel_device::update_tcr()
{
	switch(tcr & TCR_CKS) {
	case 0:
		clock_type = STOPPED;
		clock_divider = 0;
		logerror("%s: clock stopped", tag());
		break;

	case 1: case 2: case 3:
		clock_type = DIV;
		clock_divider = div_tab[((tcr & TCR_CKS)-1)*2 + extra_clock_bit];
		logerror("%s: clock %dHz", tag(), cpu->clock()/clock_divider);
		break;

	case 4:
		clock_type = chain_type;
		clock_divider = 0;
		logerror("%s: clock chained %s", tag(), clock_type == CHAIN_A ? "tcora" : "overflow");
		break;

	case 5:
		clock_type = INPUT_UP;
		clock_divider = 0;
		logerror("%s: clock external raising edge", tag());
		break;

	case 6:
		clock_type = INPUT_DOWN;
		clock_divider = 0;
		logerror("%s: clock external falling edge", tag());
		break;

	case 7:
		clock_type = INPUT_UPDOWN;
		clock_divider = 0;
		logerror("%s: clock external both edges", tag());
		break;
	}

	switch(tcr & TCR_CCLR) {
	case 0x00:
		clear_type = CLEAR_NONE;
		logerror(", no clear");
		break;

	case 0x08:
		clear_type = CLEAR_A;
		logerror(", clear on tcora");
		break;

	case 0x10:
		clear_type = CLEAR_B;
		logerror(", clear on tcorb");
		break;

	case 0x18:
		clear_type = CLEAR_EXTERNAL;
		logerror(", clear on external");
		break;
	}

	logerror(", irq=%c%c%c\n",
				tcr & TCR_CMIEB ? 'b' : '-',
				tcr & TCR_CMIEA ? 'a' : '-',
				tcr & TCR_OVIE  ? 'o' : '-');
}

READ8_MEMBER(h8_timer8_channel_device::tcsr_r)
{
	return tcsr;
}

WRITE8_MEMBER(h8_timer8_channel_device::tcsr_w)
{
	update_counter();

	UINT8 mask = has_adte || has_ice ? 0x1f : 0x0f;
	tcsr = (tcsr & ~mask) | (data & mask);
	tcsr &= data | 0x1f;

	logerror("%s: tcsr_w %02x\n", tag(), tcsr);

	recalc_event();
}

READ8_MEMBER(h8_timer8_channel_device::tcor_r)
{
	return tcor[offset];
}

WRITE8_MEMBER(h8_timer8_channel_device::tcor_w)
{
	update_counter();
	tcor[offset] = data;
	logerror("%s: tcor%c_w %02x\n", tag(), 'a'+offset, data);
	recalc_event();
}

READ8_MEMBER(h8_timer8_channel_device::tcnt_r)
{
	update_counter();
	recalc_event();
	return tcnt;
}

WRITE8_MEMBER(h8_timer8_channel_device::tcnt_w)
{
	update_counter();
	tcnt = data;
	logerror("%s: tcnt_w %02x\n", tag(), data);
	recalc_event();
}

void h8_timer8_channel_device::device_start()
{
	intc = siblingdevice<h8_intc_device>(intc_tag);
	if(chain_tag)
		chained_timer = siblingdevice<h8_timer8_channel_device>(chain_tag);
	else
		chained_timer = nullptr;
}

void h8_timer8_channel_device::device_reset()
{
	tcr = 0x00;
	tcsr = has_adte || has_ice ? 0x00 : 0x10;
	tcor[0] = 0xff;
	tcor[1] = 0xff;
	tcnt = 0x00;
	counter_cycle = 0x100;
	clock_type = STOPPED;
	clock_divider = 0;
	clear_type = CLEAR_NONE;
	last_clock_update = 0;
	event_time = 0;
	extra_clock_bit = false;
}

UINT64 h8_timer8_channel_device::internal_update(UINT64 current_time)
{
	if(event_time && current_time >= event_time) {
		update_counter(current_time);
		if(0)
			logerror("%s: Reached event time (%ld), counter=%02x, dt=%d\n", tag(), long(current_time), tcnt, int(current_time - event_time));
		recalc_event(current_time);
	}

	return event_time;
}

void h8_timer8_channel_device::update_counter(UINT64 cur_time)
{
	if(clock_type != DIV)
		return;

	if(!cur_time)
		cur_time = cpu->total_cycles();

	UINT64 base_time = (last_clock_update + clock_divider/2) / clock_divider;
	UINT64 new_time = (cur_time + clock_divider/2) / clock_divider;

	int tt = tcnt + new_time - base_time;
	tcnt = tt % counter_cycle;

	if(tt == tcor[0] || tcnt == tcor[0]) {
		if(chained_timer)
			chained_timer->chained_timer_tcora();

		if(!(tcsr & TCSR_CMFA)) {
			tcsr |= TCSR_CMFA;
			if(tcr & TCR_CMIEA)
				intc->internal_interrupt(irq_ca);
		}
	}

	if(!(tcsr & TCSR_CMFB) && (tt == tcor[1] || tcnt == tcor[1])) {
		tcsr |= TCSR_CMFB;
		if(tcr & TCR_CMIEB)
			intc->internal_interrupt(irq_cb);
	}

	if(tt >= 0x100) {
		if(chained_timer)
			chained_timer->chained_timer_overflow();
		if(!(tcsr & TCSR_OVF)) {
			tcsr |= TCSR_OVF;
			if(tcr & TCR_OVIE)
				intc->internal_interrupt(irq_v);
		}
	}
	last_clock_update = cur_time;
}

void h8_timer8_channel_device::recalc_event(UINT64 cur_time)
{
	bool update_cpu = cur_time == 0;
	UINT64 old_event_time = event_time;

	if(clock_type != DIV) {
		event_time = 0;
		if(old_event_time && update_cpu)
			cpu->internal_update();
		return;
	}

	if(!cur_time)
		cur_time = cpu->total_cycles();

	UINT32 event_delay = 0xffffffff;
	if(clear_type == CLEAR_A || clear_type == CLEAR_B)
		counter_cycle = tcor[clear_type - CLEAR_A];
	else {
		counter_cycle = 0x100;
		event_delay = counter_cycle - tcnt;
		if(!event_delay)
			event_delay = counter_cycle;
	}

	for(int i=0; i<2; i++) {
		UINT32 new_delay = 0xffffffff;
		if(tcor[i] > tcnt) {
			if(tcnt >= counter_cycle || tcor[i] <= counter_cycle)
				new_delay = tcor[i] - tcnt;
		} else if(tcor[i] <= counter_cycle) {
			if(tcnt < counter_cycle)
				new_delay = (counter_cycle - tcnt) + tcor[i];
			else
				new_delay = (0x100 - tcnt) + tcor[i];
		}
		if(event_delay > new_delay)
			event_delay = new_delay;
	}

	if(event_delay != 0xffffffff)
		event_time = ((((cur_time + clock_divider) / clock_divider) + event_delay - 1) * clock_divider) + clock_divider/2;
	else
		event_time = 0;

	if(old_event_time != event_time && update_cpu)
		cpu->internal_update();
}

void h8_timer8_channel_device::chained_timer_overflow()
{
	if(clock_type == CHAIN_OVERFLOW)
		timer_tick();
}

void h8_timer8_channel_device::chained_timer_tcora()
{
	if(clock_type == CHAIN_A)
		timer_tick();
}

void h8_timer8_channel_device::timer_tick()
{
	tcnt++;

	if(tcnt == tcor[0]) {
		if(chained_timer)
			chained_timer->chained_timer_tcora();

		if(!(tcsr & TCSR_CMFA)) {
			tcsr |= TCSR_CMFA;
			if(tcr & TCR_CMIEA)
				intc->internal_interrupt(irq_ca);
		}
	}

	if(!(tcsr & TCSR_CMFB) && tcnt == tcor[1]) {
		tcsr |= TCSR_CMFB;
		if(tcr & TCR_CMIEB)
			intc->internal_interrupt(irq_cb);
	}

	if(tcnt == 0x00) {
		if(chained_timer)
			chained_timer->chained_timer_overflow();
		if(!(tcsr & TCSR_OVF)) {
			tcsr |= TCSR_OVF;
			if(tcr & TCR_OVIE)
				intc->internal_interrupt(irq_v);
		}
	}
}

h8h_timer8_channel_device::h8h_timer8_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	h8_timer8_channel_device(mconfig, H8H_TIMER8_CHANNEL, "H8H 8-bits timer channel", tag, owner, clock, "h8h_8bits_timer_channel", __FILE__)
{
}

h8h_timer8_channel_device::~h8h_timer8_channel_device()
{
}

void h8h_timer8_channel_device::set_info(const char *intc, int _irq_ca, int _irq_cb, int _irq_v, const char *_chain_tag, int _chain_type, bool _has_adte, bool _has_ice)
{
	intc_tag = intc;
	irq_ca = _irq_ca;
	irq_cb = _irq_cb;
	irq_v = _irq_v;
	chain_tag = _chain_tag;
	chain_type = _chain_type;
	has_adte = _has_adte;
	has_ice = _has_ice;
	// The extra clock bit is not used for h8h+
	div_tab[0] = 8;
	div_tab[1] = 8;
	div_tab[2] = 64;
	div_tab[3] = 64;
	div_tab[4] = 8192;
	div_tab[5] = 8192;
}
