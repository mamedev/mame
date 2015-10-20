// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "fdc_pll.h"

std::string fdc_pll_t::tts(attotime t)
{
	char buf[256];
	bool neg = t.seconds() < 0;
	if(neg)
		t = attotime::zero - t;
	int nsec = t.attoseconds() / ATTOSECONDS_PER_NANOSECOND;
	sprintf(buf, "%c%3d.%03d,%03d,%03d", neg ? '-' : ' ', int(t.seconds()), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
	return buf;
}

void fdc_pll_t::set_clock(const attotime &_period)
{
	period = _period;
	period_adjust_base = period * 0.05;
	min_period = period * 0.75;
	max_period = period * 1.25;
}

void fdc_pll_t::reset(const attotime &when)
{
	ctime = when;
	phase_adjust = attotime::zero;
	freq_hist = 0;
	write_position = 0;
	write_start_time = attotime::never;
}

void fdc_pll_t::start_writing(const attotime &tm)
{
	write_start_time = tm;
	write_position = 0;
}

void fdc_pll_t::stop_writing(floppy_image_device *floppy, const attotime &tm)
{
	commit(floppy, tm);
	write_start_time = attotime::never;
}

void fdc_pll_t::commit(floppy_image_device *floppy, const attotime &tm)
{
	if(write_start_time.is_never() || tm == write_start_time)
		return;

	if(floppy)
		floppy->write_flux(write_start_time, tm, write_position, write_buffer);
	write_start_time = tm;
	write_position = 0;
}

int fdc_pll_t::get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	attotime edge = floppy ? floppy->get_next_transition(ctime) : attotime::never;

	attotime next = ctime + period + phase_adjust;

#if 0
	if(!edge.is_never())
		fprintf(stderr, "ctime=%s, transition_time=%s, next=%s, pha=%s\n", tts(ctime).c_str(), tts(edge).c_str(), tts(next).c_str(), tts(phase_adjust).c_str());
#endif

	if(next > limit)
		return -1;

	ctime = next;
	tm = next;

	if(edge.is_never() || edge >= next) {
		// No transition in the window means 0 and pll in free run mode
		phase_adjust = attotime::zero;
		return 0;
	}

	// Transition in the window means 1, and the pll is adjusted

	attotime delta = edge - (next - period/2);

	if(delta.seconds() < 0)
		phase_adjust = attotime::zero - ((attotime::zero - delta)*65)/100;
	else
		phase_adjust = (delta*65)/100;

	if(delta < attotime::zero) {
		if(freq_hist < 0)
			freq_hist--;
		else
			freq_hist = -1;
	} else if(delta > attotime::zero) {
		if(freq_hist > 0)
			freq_hist++;
		else
			freq_hist = 1;
	} else
		freq_hist = 0;

	if(freq_hist) {
		int afh = freq_hist < 0 ? -freq_hist : freq_hist;
		if(afh > 1) {
			attotime aper = attotime::from_double(period_adjust_base.as_double()*delta.as_double()/period.as_double());
			period += aper;

			if(period < min_period)
				period = min_period;
			else if(period > max_period)
				period = max_period;
		}
	}

	return 1;
}

bool fdc_pll_t::write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	if(write_start_time.is_never()) {
		write_start_time = ctime;
		write_position = 0;
	}

	attotime etime = ctime + period;
	if(etime > limit)
		return true;

	if(bit && write_position < ARRAY_LENGTH(write_buffer))
		write_buffer[write_position++] = ctime + period/2;

	tm = etime;
	ctime = etime;
	return false;
}
