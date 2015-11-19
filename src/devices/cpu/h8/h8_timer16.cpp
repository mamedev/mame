// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8_timer16.h"

#define LOG_EVENT_TIME 0

const device_type H8_TIMER16          = &device_creator<h8_timer16_device>;
const device_type H8_TIMER16_CHANNEL  = &device_creator<h8_timer16_channel_device>;
const device_type H8H_TIMER16_CHANNEL = &device_creator<h8h_timer16_channel_device>;
const device_type H8S_TIMER16_CHANNEL = &device_creator<h8s_timer16_channel_device>;

h8_timer16_channel_device::h8_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, H8_TIMER16_CHANNEL, "H8 16-bits timer channel", tag, owner, clock, "h8_16bits_timer_channel", __FILE__),
	cpu(*this, "^^"), chained_timer(nullptr), intc(nullptr), intc_tag(nullptr), tier_mask(0), tgr_count(0), tbr_count(0), tgr_clearing(0), tcr(0), tier(0), ier(0), isr(0), clock_type(0), 
	clock_divider(0), tcnt(0), last_clock_update(0), event_time(0), phase(0), counter_cycle(0), counter_incrementing(false), channel_active(false)
{
	chain_tag = NULL;
}

h8_timer16_channel_device::h8_timer16_channel_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	cpu(*this, "^^"), chained_timer(nullptr), intc(nullptr), intc_tag(nullptr), tier_mask(0), tgr_count(0), tbr_count(0), tgr_clearing(0), tcr(0), tier(0), ier(0), isr(0), clock_type(0),
	clock_divider(0), tcnt(0), last_clock_update(0), event_time(0), phase(0), counter_cycle(0), counter_incrementing(false), channel_active(false)
{
	chain_tag = NULL;
}

void h8_timer16_channel_device::set_info(int _tgr_count, int _tbr_count, const char *intc, int irq_base)
{
	tgr_count = _tgr_count;
	tbr_count = _tbr_count;
	intc_tag = intc;

	interrupt[0] = irq_base++;
	interrupt[1] = irq_base++;
	interrupt[2] = -1;
	interrupt[3] = -1;
	interrupt[4] = irq_base;
	interrupt[5] = irq_base;
}

READ8_MEMBER(h8_timer16_channel_device::tcr_r)
{
	return tcr;
}

WRITE8_MEMBER(h8_timer16_channel_device::tcr_w)
{
	update_counter();
	tcr = data;
	logerror("%s: tcr_w %02x\n", tag(), data);
	tcr_update();
	recalc_event();
}

READ8_MEMBER(h8_timer16_channel_device::tmdr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h8_timer16_channel_device::tmdr_w)
{
	logerror("%s: tmdr_w %02x\n", tag(), data);
}

READ8_MEMBER(h8_timer16_channel_device::tior_r)
{
	return 0x00;
}

WRITE8_MEMBER(h8_timer16_channel_device::tior_w)
{
	logerror("%s: tior_w %d, %02x\n", tag(), offset, data);
}

void h8_timer16_channel_device::set_ier(UINT8 value)
{
	update_counter();
	ier = value;
	recalc_event();
}

void h8_timer16_channel_device::set_enable(bool enable)
{
	update_counter();
	channel_active = enable;
	recalc_event();
}

READ8_MEMBER(h8_timer16_channel_device::tier_r)
{
	return tier;
}

WRITE8_MEMBER(h8_timer16_channel_device::tier_w)
{
	update_counter();
	logerror("%s: tier_w %02x\n", tag(), data);
	tier = data;
	tier_update();
	logerror("%s: irq %c%c%c%c%c%c trigger=%d\n",
				tag(),
				ier & IRQ_A ? 'a' : '.',
				ier & IRQ_B ? 'b' : '.',
				ier & IRQ_C ? 'c' : '.',
				ier & IRQ_D ? 'd' : '.',
				ier & IRQ_V ? 'v' : '.',
				ier & IRQ_U ? 'u' : '.',
				ier & IRQ_TRIG ? 1 : 0);
	recalc_event();
}

READ8_MEMBER(h8_timer16_channel_device::tsr_r)
{
	return isr_to_sr();
}

WRITE8_MEMBER(h8_timer16_channel_device::tsr_w)
{
	logerror("%s: tsr_w %02x\n", tag(), data);
	isr_update(data);
}

READ16_MEMBER(h8_timer16_channel_device::tcnt_r)
{
	update_counter();
	return tcnt;
}

WRITE16_MEMBER(h8_timer16_channel_device::tcnt_w)
{
	update_counter();
	COMBINE_DATA(&tcnt);
	logerror("%s: tcnt_w %04x\n", tag(), tcnt);
	recalc_event();
}

READ16_MEMBER(h8_timer16_channel_device::tgr_r)
{
	return tgr[offset];
}

WRITE16_MEMBER(h8_timer16_channel_device::tgr_w)
{
	update_counter();
	COMBINE_DATA(tgr + offset);
	if(1)
		logerror("%s: tgr%c_w %04x\n", tag(), 'a'+offset, tgr[offset]);
	recalc_event();
}

READ16_MEMBER(h8_timer16_channel_device::tbr_r)
{
	return tgr[offset+tgr_count];
}

WRITE16_MEMBER(h8_timer16_channel_device::tbr_w)
{
	COMBINE_DATA(tgr + offset + tgr_count);
	if(1)
		logerror("%s: tbr%c_w %04x\n", tag(), 'a'+offset, tgr[offset]);
}

void h8_timer16_channel_device::device_start()
{
	intc = owner()->siblingdevice<h8_intc_device>(intc_tag);
	channel_active = false;
	device_reset();

	save_item(NAME(tgr_clearing));
	save_item(NAME(tcr));
	save_item(NAME(tier));
	save_item(NAME(ier));
	save_item(NAME(isr));
	save_item(NAME(clock_type));
	save_item(NAME(clock_divider));
	save_item(NAME(tcnt));
	save_item(NAME(tgr));
	save_item(NAME(last_clock_update));
	save_item(NAME(event_time));
	save_item(NAME(phase));
	save_item(NAME(counter_cycle));
	save_item(NAME(counter_incrementing));
	save_item(NAME(channel_active));
}

void h8_timer16_channel_device::device_reset()
{
	// Don't touch channel_active here, top level device handles it
	tcr = 0;
	tcnt = 0;
	memset(tgr, 0xff, sizeof(tgr));
	tgr_clearing = TGR_CLEAR_NONE;
	clock_type = DIV_1;
	clock_divider = 0;
	counter_cycle = 0x10000;
	phase = 0;
	tier = 0x40 & tier_mask;
	ier = 0;
	isr = 0;
	last_clock_update = 0;
	event_time = 0;
	counter_incrementing = true;
}

UINT64 h8_timer16_channel_device::internal_update(UINT64 current_time)
{
	if(event_time && current_time >= event_time) {
		update_counter(current_time);
		if(0)
		logerror("%s: Reached event time (%ld), counter=%04x, dt=%d\n", tag(), long(current_time), tcnt, int(current_time - event_time));
		recalc_event(current_time);
	}

	return event_time;
}

void h8_timer16_channel_device::update_counter(UINT64 cur_time)
{
	if(clock_type != DIV_1)
		return;

	if(!cur_time)
		cur_time = cpu->total_cycles();

	if(!channel_active) {
		last_clock_update = cur_time;
		return;
	}

	UINT64 base_time = last_clock_update;
	UINT64 new_time = cur_time;
	if(clock_divider) {
		base_time = (base_time + phase) >> clock_divider;
		new_time = (new_time + phase) >> clock_divider;
	}
	if(counter_incrementing) {
		int ott = tcnt;
		int tt = tcnt + new_time - base_time;
		tcnt = tt % counter_cycle;
		if(0)
		logerror("%s: Updating %d (%ld %ld) (%ld %ld) -> %d/%d\n",
					tag(),
					ott,
					long(last_clock_update), long(cur_time),
					long(base_time), long(new_time),
					tt, tcnt);

		for(int i=0; i<tgr_count; i++)
			if((ier & (1 << i)) && (tt == tgr[i] || tcnt == tgr[i]) && interrupt[i] != -1) {
				isr |= 1 << i;
				intc->internal_interrupt(interrupt[i]);
			}
		if(tt >= 0x10000 && (ier & IRQ_V) && interrupt[4] != -1) {
			isr |= IRQ_V;
			intc->internal_interrupt(interrupt[4]);
		}
	} else
		tcnt = (((tcnt ^ 0xffff) + new_time - base_time) % counter_cycle) ^ 0xffff;
	last_clock_update = cur_time;
}

void h8_timer16_channel_device::recalc_event(UINT64 cur_time)
{
	if(!channel_active) {
		event_time = 0;
		return;
	}

	bool update_cpu = cur_time == 0;
	UINT64 old_event_time = event_time;

	if(clock_type != DIV_1) {
		event_time = 0;
		if(old_event_time && update_cpu)
			cpu->internal_update();

		return;
	}

	if(!cur_time)
		cur_time = cpu->total_cycles();

	if(counter_incrementing) {
		UINT32 event_delay = 0xffffffff;
		if(tgr_clearing >= 0 && tgr[tgr_clearing])
			counter_cycle = tgr[tgr_clearing];
		else {
			counter_cycle = 0x10000;
			if(ier & IRQ_V) {
				event_delay = counter_cycle - tcnt;
				if(!event_delay)
					event_delay = counter_cycle;
			}
		}
		for(int i=0; i<tgr_count; i++)
			if(ier & (1 << i)) {
				UINT32 new_delay = 0xffffffff;
				if(tgr[i] > tcnt) {
					if(tcnt >= counter_cycle || tgr[i] <= counter_cycle)
						new_delay = tgr[i] - tcnt;
				} else if(tgr[i] <= counter_cycle) {
					if(tcnt < counter_cycle)
						new_delay = (counter_cycle - tcnt) + tgr[i];
					else
						new_delay = (0x10000 - tcnt) + tgr[i];
				}

				if(0)
				logerror("%s: tcnt=%d tgr%c=%d cycle=%d -> delay=%d\n",
							tag(), tcnt, 'a'+i, tgr[i], counter_cycle, new_delay);
				if(event_delay > new_delay)
					event_delay = new_delay;
			}

		if(event_delay != 0xffffffff)
			event_time = ((((cur_time + (1ULL << clock_divider) - phase) >> clock_divider) + event_delay - 1) << clock_divider) + phase;
		else
			event_time = 0;

		if(event_time && LOG_EVENT_TIME)
			logerror("%s: next event in %d cycles (%ld)\n", tag(), int(event_time - cpu->total_cycles()), long(event_time));

	} else {
		logerror("decrementing counter\n");
		exit(1);
	}

	if(old_event_time != event_time && update_cpu)
		cpu->internal_update();
}

h8_timer16_device::h8_timer16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, H8_TIMER16, "H8 16-bits timer", tag, owner, clock, "h8_timer16", __FILE__),
	cpu(*this, DEVICE_SELF_OWNER)
{
}

void h8_timer16_device::set_info(int count, UINT8 tstr)
{
	timer_count = count;
	default_tstr = tstr;
}

void h8_timer16_device::device_start()
{
	memset(timer_channel, 0, sizeof(timer_channel));
	for(int i=0; i<timer_count; i++) {
		char tm[2];
		sprintf(tm, "%d", i);
		timer_channel[i] = subdevice<h8_timer16_channel_device>(tm);
	}

	save_item(NAME(tstr));
}

void h8_timer16_device::device_reset()
{
	tstr = default_tstr;
	for(int i=0; i<timer_count; i++)
		timer_channel[i]->set_enable((tstr >> i) & 1);
}


READ8_MEMBER(h8_timer16_device::tstr_r)
{
	return tstr;
}

WRITE8_MEMBER(h8_timer16_device::tstr_w)
{
	logerror("%s: tstr_w %02x\n", tag(), data);
	tstr = data;
	for(int i=0; i<timer_count; i++)
		timer_channel[i]->set_enable((tstr >> i) & 1);
}

READ8_MEMBER(h8_timer16_device::tsyr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h8_timer16_device::tsyr_w)
{
	logerror("%s: tsyr_w %02x\n", tag(), data);
}

READ8_MEMBER(h8_timer16_device::tmdr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h8_timer16_device::tmdr_w)
{
	logerror("%s: tmdr_w %02x\n", tag(), data);
}

READ8_MEMBER(h8_timer16_device::tfcr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h8_timer16_device::tfcr_w)
{
	logerror("%s: tfcr_w %02x\n", tag(), data);
}

READ8_MEMBER(h8_timer16_device::toer_r)
{
	return 0x00;
}

WRITE8_MEMBER(h8_timer16_device::toer_w)
{
	logerror("%s: toer_w %02x\n", tag(), data);
}

READ8_MEMBER(h8_timer16_device::tocr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h8_timer16_device::tocr_w)
{
	logerror("%s: tocr_w %02x\n", tag(), data);
}

READ8_MEMBER(h8_timer16_device::tisr_r)
{
	UINT8 r = 0;
	for(int i=0; i<timer_count; i++)
		r |= timer_channel[i]->tisr_r(offset) << i;
	for(int i=timer_count; i<4; i++)
		r |= 0x11 <<i;

	logerror("%s: tisr%c_r %02x\n", tag(), 'a'+offset, r);

	return r;
}

WRITE8_MEMBER(h8_timer16_device::tisr_w)
{
	logerror("%s: tisr%c_w %02x\n", tag(), 'a'+offset, data);
	for(int i=0; i<timer_count; i++)
		timer_channel[i]->tisr_w(offset, data >> i);
}

READ8_MEMBER(h8_timer16_device::tisrc_r)
{
	return tisr_r(space, 2, mem_mask);
}

WRITE8_MEMBER(h8_timer16_device::tisrc_w)
{
	tisr_w(space, 2, data, mem_mask);
}

WRITE8_MEMBER(h8_timer16_device::tolr_w)
{
	logerror("%s: tocr_w %02x\n", tag(), data);
}



void h8_timer16_channel_device::tier_update()
{
}

void h8_timer16_channel_device::isr_update(UINT8 val)
{
}

UINT8 h8_timer16_channel_device::isr_to_sr() const
{
	return 0x00;
}

void h8_timer16_channel_device::tcr_update()
{
}

void h8_timer16_channel_device::tisr_w(int offset, UINT8 value)
{
	update_counter();
	if(!(value & 0x01)) {
		switch(offset) {
		case 0:
			isr &= ~IRQ_A;
			break;
		case 1:
			isr &= ~IRQ_B;
			break;
		case 2:
			isr &= ~IRQ_V;
			break;
		}
	}
	if(value & 0x10) {
		switch(offset) {
		case 0:
			ier |= IRQ_A;
			break;
		case 1:
			ier |= IRQ_B;
			break;
		case 2:
			ier |= IRQ_V;
			break;
		}
	} else {
		switch(offset) {
		case 0:
			ier &= ~IRQ_A;
			break;
		case 1:
			ier &= ~IRQ_B;
			break;
		case 2:
			ier &= ~IRQ_V;
			break;
		}
	}
	recalc_event();
}

UINT8 h8_timer16_channel_device::tisr_r(int offset) const
{
	switch(offset) {
	case 0:
		return ((ier & IRQ_A) ? 0x10 : 0x00) | ((isr & IRQ_A) ? 0x01 : 0x00);
	case 1:
		return ((ier & IRQ_B) ? 0x10 : 0x00) | ((isr & IRQ_B) ? 0x01 : 0x00);
	case 2:
		return ((ier & IRQ_V) ? 0x10 : 0x00) | ((isr & IRQ_V) ? 0x01 : 0x00);
	}
	return 0x00;
}

h8h_timer16_channel_device::h8h_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	h8_timer16_channel_device(mconfig, H8H_TIMER16_CHANNEL, "H8H 16-bits timer channel", tag, owner, clock, "h8h_16bits_timer_channel", __FILE__)
{
}

h8h_timer16_channel_device::~h8h_timer16_channel_device()
{
}

void h8h_timer16_channel_device::set_info(int _tgr_count, int _tbr_count, const char *intc, int irq_base)
{
	tgr_count = _tgr_count;
	tbr_count = _tbr_count;
	intc_tag = intc;

	interrupt[0] = irq_base++;
	interrupt[1] = irq_base++;
	interrupt[2] = -1;
	interrupt[3] = -1;
	interrupt[4] = irq_base;
	interrupt[5] = irq_base;
}

void h8h_timer16_channel_device::tier_update()
{
	tier = tier | 0xf8;
	ier =
		(tier & 0x01 ? IRQ_A : 0) |
		(tier & 0x02 ? IRQ_B : 0) |
		(tier & 0x04 ? IRQ_V : 0);
}

void h8h_timer16_channel_device::isr_update(UINT8 val)
{
	if(!(val & 1))
		isr &= ~IRQ_A;
	if(!(val & 2))
		isr &= ~IRQ_B;
	if(!(val & 4))
		isr &= ~IRQ_V;
}

UINT8 h8h_timer16_channel_device::isr_to_sr() const
{
	return 0xf8 | (isr & IRQ_V ? 4 : 0) | (isr & IRQ_B ? 2 : 0) | (isr & IRQ_A ? 1 : 0);
}


void h8h_timer16_channel_device::tcr_update()
{
	switch(tcr & 0x60) {
	case 0x00:
		tgr_clearing = TGR_CLEAR_NONE;
		logerror("%s: No automatic tcnt clearing\n", tag());
		break;
	case 0x20: case 0x40: {
		tgr_clearing = tcr & 0x20 ? 0 : 1;
		logerror("%s: Auto-clear on tgr%c (%04x)\n", tag(), 'a'+tgr_clearing, tgr[tgr_clearing]);
		break;
	}
	case 0x60:
		tgr_clearing = TGR_CLEAR_EXT;
		logerror("%s: External sync clear\n", tag());
		break;
	}

	int count_type = tcr & 7;
	if(count_type < 4) {
		clock_type = DIV_1;
		clock_divider = count_type;
		logerror("%s: clock divider %d (%d)\n", tag(), clock_divider, 1 << clock_divider);
		if(count_type <= DIV_2)
			phase = 0;
		else {
			switch(tcr & 0x18) {
			case 0x00:
				phase = 0;
				logerror("%s: Phase 0\n", tag());
				break;
			case 0x08:
				phase = 1 << (clock_divider-1);
				logerror("%s: Phase 180\n", tag());
				break;
			case 0x10: case 0x18:
				phase = 0;
				clock_divider--;
				logerror("%s: Phase 0+180\n", tag());
				break;
			}
		}
	} else {
		clock_type = INPUT_A + (count_type-4);
		clock_divider = 0;
		phase = 0;
		logerror("%s: counting input %c\n", tag(), 'a'+count_type-INPUT_A);
	}
}

h8s_timer16_channel_device::h8s_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	h8_timer16_channel_device(mconfig, H8S_TIMER16_CHANNEL, "H8S 16-bits timer channel", tag, owner, clock, "h8s_16bits_timer_channel", __FILE__)
{
}

h8s_timer16_channel_device::~h8s_timer16_channel_device()
{
}

void h8s_timer16_channel_device::set_chain(const char *_chain_tag)
{
	chain_tag = _chain_tag;
}

void h8s_timer16_channel_device::set_info(int _tgr_count, UINT8 _tier_mask, const char *intc, int irq_base,
											int t0, int t1, int t2, int t3, int t4, int t5, int t6, int t7)
{
	tgr_count = _tgr_count;
	tbr_count = 0;
	tier_mask = _tier_mask;
	intc_tag = intc;

	interrupt[0] = irq_base++;
	interrupt[1] = irq_base++;
	interrupt[2] = tier_mask & 0x04 ? -1 : irq_base++;
	interrupt[3] = tier_mask & 0x08 ? -1 : irq_base++;
	interrupt[4] = irq_base;
	interrupt[5] = tier_mask & 0x20 ? -1 : irq_base++;

	count_types[0] = t0;
	count_types[1] = t1;
	count_types[2] = t2;
	count_types[3] = t3;
	count_types[4] = t4;
	count_types[5] = t5;
	count_types[6] = t6;
	count_types[7] = t7;
}

void h8s_timer16_channel_device::tier_update()
{
	tier = (tier & ~tier_mask) | 0x40;
	ier =
		(tier & 0x01 ? IRQ_A : 0) |
		(tier & 0x02 ? IRQ_B : 0) |
		(tier & 0x04 ? IRQ_C : 0) |
		(tier & 0x08 ? IRQ_D : 0) |
		(tier & 0x10 ? IRQ_V : 0) |
		(tier & 0x20 ? IRQ_U : 0) |
		(tier & 0x80 ? IRQ_TRIG : 0);
}

void h8s_timer16_channel_device::isr_update(UINT8 val)
{
	isr &= (val | tier_mask | 0xc0);
}

UINT8 h8s_timer16_channel_device::isr_to_sr() const
{
	return 0xc0 | isr;
}

void h8s_timer16_channel_device::tcr_update()
{
	switch(tcr & 0x60) {
	case 0x00:
		tgr_clearing = TGR_CLEAR_NONE;
		logerror("%s: No automatic tcnt clearing\n", tag());
		break;
	case 0x20: case 0x40: {
		tgr_clearing = tcr & 0x20 ? 0 : 1;
		if(tgr_count > 2 && (tcr & 0x80))
			tgr_clearing += 2;
		logerror("%s: Auto-clear on tgr%c\n", tag(), 'a'+tgr_clearing);
		break;
	}
	case 0x60:
		tgr_clearing = TGR_CLEAR_EXT;
		logerror("%s: External sync clear\n", tag());
		break;
	}

	int count_type = count_types[tcr & 7];
	if(count_type >= DIV_1 && clock_type <= DIV_4) {
		clock_type = DIV_1;
		clock_divider = count_type - DIV_1;
		logerror("%s: clock divider %d (%d)\n", tag(), clock_divider, 1 << clock_divider);
		if(!clock_divider)
			phase = 0;
		else {
			switch(tcr & 0x18) {
			case 0x00:
				phase = 0;
				logerror("%s: Phase 0\n", tag());
				break;
			case 0x08:
				phase = 1 << (clock_divider-1);
				logerror("%s: Phase 180\n", tag());
				break;
			case 0x10: case 0x18:
				phase = 0;
				clock_divider--;
				logerror("%s: Phase 0+180\n", tag());
				break;
			}
		}

	} else if(count_type == CHAIN) {
		clock_type = CHAIN;
		clock_divider = 0;
		phase = 0;
		logerror("%s: chained timer\n", tag());

	} else if(count_type >= INPUT_A && count_type <= INPUT_D) {
		clock_type = count_type;
		clock_divider = 0;
		phase = 0;
		logerror("%s: counting input %c\n", tag(), 'a'+count_type-INPUT_A);
	}
}
