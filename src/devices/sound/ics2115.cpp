// license:???
// copyright-holders:Alex Marshall,nimitz,austere
//ICS2115 by Raiden II team (c) 2010
//members: austere, nimitz, Alex Marshal
//
//Original driver by O. Galibert, ElSemi
//
//Use tab size = 4 for your viewing pleasure.

#include "emu.h"
#include "ics2115.h"
#include <cmath>

//#define ICS2115_DEBUG
//#define ICS2115_ISOLATE 6

// device type definition
const device_type ICS2115 = &device_creator<ics2115_device>;

ics2115_device::ics2115_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ICS2115, "ICS2115", tag, owner, clock, "ics2115", __FILE__),
		device_sound_interface(mconfig, *this), m_stream(nullptr),
		m_rom(*this, DEVICE_SELF),
		m_irq_cb(*this), m_active_osc(0), m_osc_select(0), m_reg_select(0), m_irq_enabled(0), m_irq_pending(0), m_irq_on(false), m_vmode(0)
{
}

void ics2115_device::device_start()
{
	m_timer[0].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ics2115_device::timer_cb_0),this), this);
	m_timer[1].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ics2115_device::timer_cb_1),this), this);
	m_stream = machine().sound().stream_alloc(*this, 0, 2, 33075);

	m_irq_cb.resolve_safe();

	//Exact formula as per patent 5809466
	//This seems to give the ok fit but it is not good enough.
	/*double maxvol = ((1 << volume_bits) - 1) * pow(2., (double)1/0x100);
	for (int i = 0; i < 0x1000; i++) {
	       m_volume[i] = floor(maxvol * pow(2.,(double)i/256 - 16) + 0.5);
	}*/

	//austere's table, derived from patent 5809466:
	//See section V starting from page 195
	//Subsection F (column 124, page 198) onwards
	for (int i = 0; i<4096; i++) {
		m_volume[i] = ((0x100 | (i & 0xff)) << (volume_bits-9)) >> (15 - (i>>8));
	}

	//u-Law table as per MIL-STD-188-113
	UINT16 lut[8];
	UINT16 lut_initial = 33 << 2;   //shift up 2-bits for 16-bit range.
	for(int i = 0; i < 8; i++)
		lut[i] = (lut_initial << i) - lut_initial;
	for(int i = 0; i < 256; i++) {
		UINT8 exponent = (~i >> 4) & 0x07;
		UINT8 mantissa = ~i & 0x0f;
		INT16 value = lut[exponent] + (mantissa << (exponent + 3));
		m_ulaw[i] = (i & 0x80) ? -value : value;
	}

	save_item(NAME(m_timer[0].period));
	save_item(NAME(m_timer[0].scale));
	save_item(NAME(m_timer[0].preset));
	save_item(NAME(m_timer[1].period));
	save_item(NAME(m_timer[1].scale));
	save_item(NAME(m_timer[1].preset));
	save_item(NAME(m_reg_select));
	save_item(NAME(m_osc_select));
	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_irq_on));
	save_item(NAME(m_active_osc));
	save_item(NAME(m_vmode));

	for(int i = 0; i < 32; i++) {
		save_item(NAME(m_voice[i].osc_conf.value), i);
		save_item(NAME(m_voice[i].state.value), i);
		save_item(NAME(m_voice[i].vol_ctrl.value), i);
		save_item(NAME(m_voice[i].osc.left), i);
		save_item(NAME(m_voice[i].osc.acc), i);
		save_item(NAME(m_voice[i].osc.start), i);
		save_item(NAME(m_voice[i].osc.end), i);
		save_item(NAME(m_voice[i].osc.fc), i);
		save_item(NAME(m_voice[i].osc.ctl), i);
		save_item(NAME(m_voice[i].osc.saddr), i);
		save_item(NAME(m_voice[i].vol.left), i);
		save_item(NAME(m_voice[i].vol.add), i);
		save_item(NAME(m_voice[i].vol.start), i);
		save_item(NAME(m_voice[i].vol.end), i);
		save_item(NAME(m_voice[i].vol.acc), i);
		save_item(NAME(m_voice[i].vol.regacc), i);
		save_item(NAME(m_voice[i].vol.incr), i);
		save_item(NAME(m_voice[i].vol.pan), i);
		save_item(NAME(m_voice[i].vol.mode), i);
	}
}


void ics2115_device::device_reset()
{
	m_irq_enabled = 0;
	m_irq_pending = 0;
	//possible re-suss
	m_active_osc = 31;
	m_osc_select = 0;
	m_reg_select = 0;
	m_vmode = 0;
	m_irq_on = false;
	memset(m_voice, 0, sizeof(m_voice));
	for(auto & elem : m_timer)
	{
		elem.timer->adjust(attotime::never);
		elem.period = 0;
		elem.scale = 0;
		elem.preset = 0;
	}
	for(auto & elem : m_voice) {
		elem.osc_conf.value = 2;
		elem.osc.fc = 0;
		elem.osc.acc = 0;
		elem.osc.start = 0;
		elem.osc.end = 0;
		elem.osc.ctl = 0;
		elem.osc.saddr = 0;
		elem.vol.acc = 0;
		elem.vol.incr = 0;
		elem.vol.start = 0;
		elem.vol.end = 0;
		elem.vol.pan = 0x7F;
		elem.vol_ctrl.value = 1;
		elem.vol.mode = 0;
		elem.state.value = 0;
	}
}

//TODO: improve using next-state logic from column 126 of patent 5809466
int ics2115_voice::update_volume_envelope()
{
	int ret = 0;
	if(vol_ctrl.bitflags.done || vol_ctrl.bitflags.stop)
		return ret;

	if(vol_ctrl.bitflags.invert) {
		vol.acc -= vol.add;
		vol.left = vol.acc - vol.start;
	} else {
		vol.acc += vol.add;
		vol.left = vol.end - vol.acc;
	}

	if(vol.left > 0)
		return ret;

	if(vol_ctrl.bitflags.irq) {
		vol_ctrl.bitflags.irq_pending = true;
		ret = 1;
	}

	if(osc_conf.bitflags.eightbit)
		return ret;

	if(vol_ctrl.bitflags.loop) {
		if(vol_ctrl.bitflags.loop_bidir)
			vol_ctrl.bitflags.invert = !vol_ctrl.bitflags.invert;

		if(vol_ctrl.bitflags.invert)
			vol.acc = vol.end + vol.left;
		else
			vol.acc = vol.start - vol.left;
	} else {
		state.bitflags.on = false;
		vol_ctrl.bitflags.done = true;
		if(vol_ctrl.bitflags.invert)
			vol.acc = vol.end;
		else
			vol.acc = vol.start;
	}

	return ret;
}

/*UINT32 ics2115_voice::next_address()
{
    //Patent 6,246,774 B1, Column 111, Row 25
    //LEN   BLEN    DIR     BC      NextAddress
    //x     x       0       0       add+fc
    //x     x       1       0       add-fc
    //0     x       x       1       add
    //1     0       0       1       start-(end-(add+fc))
    //1     0       1       1       end+((add+fc)-start)
    //1     1       0       1       end+(end-(add+fc))
    //1     1       1       1       start-((add-fc)-start)

}*/


int ics2115_voice::update_oscillator()
{
	int ret = 0;
	if(osc_conf.bitflags.stop)
		return ret;
	if(osc_conf.bitflags.invert) {
		osc.acc -= osc.fc << 2;
		osc.left = osc.acc - osc.start;
	} else {
		osc.acc += osc.fc << 2;
		osc.left = osc.end - osc.acc;
	}
	// > instead of >= to stop crackling?
	if(osc.left > 0)
		return ret;
	if(osc_conf.bitflags.irq) {
		osc_conf.bitflags.irq_pending = true;
		ret = 1;
	}
	if(osc_conf.bitflags.loop) {
		if(osc_conf.bitflags.loop_bidir)
			osc_conf.bitflags.invert = !osc_conf.bitflags.invert;
		//else
		//    printf("click!\n");

		if(osc_conf.bitflags.invert) {
			osc.acc = osc.end + osc.left;
			osc.left = osc.acc - osc.start;
		}
		else {
			osc.acc = osc.start - osc.left;
			osc.left = osc.end - osc.acc;
		}
	} else {
		state.bitflags.on = false;
		osc_conf.bitflags.stop = true;
		if(!osc_conf.bitflags.invert)
			osc.acc = osc.end;
		else
			osc.acc = osc.start;
	}
	return ret;
}

//TODO: proper interpolation for uLaw (fill_output doesn't use this) and 8-bit samples (looping)
stream_sample_t ics2115_device::get_sample(ics2115_voice& voice)
{
	UINT32 curaddr = ((voice.osc.saddr << 20) & 0xffffff) | (voice.osc.acc >> 12);
	UINT32 nextaddr;

	if (voice.state.bitflags.on && voice.osc_conf.bitflags.loop && !voice.osc_conf.bitflags.loop_bidir &&
			(voice.osc.left < (voice.osc.fc <<2))) {
		//printf("C?[%x:%x]", voice.osc.left, voice.osc.acc);
		nextaddr = ((voice.osc.saddr << 20) & 0xffffff) | (voice.osc.start >> 12);
	}
	else
		nextaddr = curaddr + 2;


	INT16 sample1, sample2;
	if (voice.osc_conf.bitflags.eightbit) {
		sample1 = ((INT8)m_rom[curaddr]) << 8;
		sample2 = ((INT8)m_rom[curaddr + 1]) << 8;
	}
	else {
		sample1 = m_rom[curaddr + 0] | (((INT8)m_rom[curaddr + 1]) << 8);
		sample2 = m_rom[nextaddr+ 0] | (((INT8)m_rom[nextaddr+ 1]) << 8);
		//sample2 = m_rom[curaddr + 2] | (((INT8)m_rom[curaddr + 3]) << 8);
	}

	//no need for interpolation since it's around 1 note a cycle?
	//if(voice.osc.fc >> 10)
	//    return sample1;

	//linear interpolation as in US patent 6,246,774 B1, column 2 row 59
	//LEN=1, BLEN=0, DIR=0, start+end interpolation
	INT32 sample, diff;
	UINT16 fract;
	diff = sample2 - sample1;
	fract = (voice.osc.acc >> 3) & 0x1ff;

	sample = (((INT32)sample1 << 9) + diff * fract) >> 9;
	//sample = sample1;
	return sample;
}

bool ics2115_voice::playing()
{
	return state.bitflags.on && !((vol_ctrl.bitflags.done || vol_ctrl.bitflags.stop) && osc_conf.bitflags.stop);
}

void ics2115_voice::update_ramp() {
	//slow attack
	if (state.bitflags.on && !osc_conf.bitflags.stop) {
		if (state.bitflags.ramp < 0x40)
			state.bitflags.ramp += 0x1;
		else
			state.bitflags.ramp = 0x40;
	}
	//slow release
	else {
		if (state.bitflags.ramp)
			state.bitflags.ramp -= 0x1;
	}
}

int ics2115_device::fill_output(ics2115_voice& voice, stream_sample_t *outputs[2], int samples)
{
	bool irq_invalid = false;
	UINT16 fine = 1 << (3*(voice.vol.incr >> 6));
	voice.vol.add = (voice.vol.incr & 0x3F)<< (10 - fine);

	for (int i = 0; i < samples; i++) {
		UINT32 volacc = (voice.vol.acc >> 10) & 0xffff;
		UINT32 volume = (m_volume[volacc >> 4] * voice.state.bitflags.ramp) >> 6;
		UINT16 vleft = volume; //* (255 - voice.vol.pan) / 0x80];
		UINT16 vright = volume; //* (voice.vol.pan + 1) / 0x80];

		//From GUS doc:
		//In general, it is necessary to remember that all voices are being summed in to the
		//final output, even if they are not running.  This means that whatever data value
		//that the voice is pointing at is contributing to the summation.
		//(austere note: this will of course fix some of the glitches due to multiple transition)
		stream_sample_t sample;
		if(voice.osc_conf.bitflags.ulaw) {
			UINT32 curaddr = ((voice.osc.saddr << 20) & 0xffffff) | (voice.osc.acc >> 12);
			sample = m_ulaw[m_rom[curaddr]];
		}
		else
			sample = get_sample(voice);

		//15-bit volume + (5-bit worth of 32 channel sum) + 16-bit samples = 4-bit extra
		if (!m_vmode || voice.playing()) {
		//if (voice.playing()) {
			outputs[0][i] += (sample * vleft) >> (5 + volume_bits - 16);
			outputs[1][i] += (sample * vright) >> (5 + volume_bits - 16);
		}

		voice.update_ramp();
		if (voice.playing()) {
			if (voice.update_oscillator())
				irq_invalid = true;
			if (voice.update_volume_envelope())
				irq_invalid = true;
		}
	}
	return irq_invalid;
}

void ics2115_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	memset(outputs[0], 0, samples * sizeof(stream_sample_t));
	memset(outputs[1], 0, samples * sizeof(stream_sample_t));

	bool irq_invalid = false;
	for(int osc = 0; osc <= m_active_osc; osc++) {
		ics2115_voice& voice = m_voice[osc];

#ifdef ICS2115_ISOLATE
		if(osc != ICS2115_ISOLATE)
			continue;
#endif
/*
#ifdef ICS2115_DEBUG
        UINT32 curaddr = ((voice.osc.saddr << 20) & 0xffffff) | (voice.osc.acc >> 12);
        stream_sample_t sample;
        if(voice.osc_conf.bitflags.ulaw)
            sample = m_ulaw[m_rom[curaddr]];
        else
            sample = get_sample(voice);
        printf("[%06x=%04x]", curaddr, (INT16)sample);
#endif
*/
		if(fill_output(voice, outputs, samples))
			irq_invalid = true;

#ifdef ICS2115_DEBUG
		if(voice.playing()) {
			printf("%d", osc);
			if (voice.osc_conf.bitflags.invert)
				printf("+");
			else if ((voice.osc.fc >> 1) > 0x1ff)
				printf("*");
			printf(" ");

			/*int min = 0x7fffffff, max = 0x80000000;
			double average = 0;
			for (int i = 0; i < samples; i++) {
			    if (outputs[0][i] > max) max = outputs[0][i];
			    if (outputs[0][i] < min) min = outputs[0][i];
			    average += fabs(outputs[0][i]);
			}
			average /= samples;
			average /= 1 << 16;
			printf("<Mi:%d Mx:%d Av:%g>", min >> 16, max >> 16, average);*/
		}
#endif
	}

#ifdef ICS2115_DEBUG
	printf("|");
#endif

	//rescale
	for (int i = 0; i < samples; i++) {
		outputs[0][i] >>= 16;
		outputs[1][i] >>= 16;
	}

	if(irq_invalid)
		recalc_irq();

}

//Helper Function (Reads off current register)
UINT16 ics2115_device::reg_read() {
	UINT16 ret;
	ics2115_voice& voice = m_voice[m_osc_select];

	switch(m_reg_select) {
		case 0x00: // [osc] Oscillator Configuration
			ret = voice.osc_conf.value;
			ret <<= 8;
			break;

		case 0x01: // [osc] Wavesample frequency
			// freq = fc*33075/1024 in 32 voices mode, fc*44100/1024 in 24 voices mode
			//ret = v->Osc.FC;
			ret = voice.osc.fc;
			break;

		case 0x02: // [osc] Wavesample loop start high
			//TODO: are these returns valid? might be 0x00ff for this one...
			ret = (voice.osc.start >> 16) & 0xffff;
			break;

		case 0x03: // [osc] Wavesample loop start low
			ret = (voice.osc.start >> 0) & 0xff00;
			break;

		case 0x04: // [osc] Wavesample loop end high
			ret = (voice.osc.end >> 16) & 0xffff;
			break;

		case 0x05: // [osc] Wavesample loop end low
			ret = (voice.osc.end >> 0) & 0xff00;
			break;

		case 0x06: // [osc] Volume Increment
			ret = voice.vol.incr;
			break;

		case 0x07: // [osc] Volume Start
			ret = voice.vol.start >> (10+8);
			break;

		case 0x08: // [osc] Volume End
			ret = voice.vol.end >> (10+8);
			break;

		case 0x09: // [osc] Volume accumulator
			//ret = v->Vol.Acc;
			ret = voice.vol.acc  >> (10);
			break;

		case 0x0A: // [osc] Wavesample address
			ret = (voice.osc.acc >> 16) & 0xffff;
			break;

		case 0x0B: // [osc] Wavesample address
			ret = (voice.osc.acc >> 0) & 0xfff8;
			break;


		case 0x0C: // [osc] Pan
			ret = voice.vol.pan << 8;
			break;

		/* DDP3 code (trap15's reversal) */
		/* 0xA13's work:
		    res = read() & 0xC3;
		    if(!(res & 2)) res |= 1;
		    e = d = res;
		*/
		/* 0xA4F's work:
		    while(!(read() & 1))
		*/
		case 0x0D: // [osc] Volume Envelope Control
			//ret = v->Vol.Ctl | ((v->state & FLAG_STATE_VOLIRQ) ? 0x81 : 1);
			// may expect |8 on voice irq with &40 == 0
			// may expect |8 on reg 0 on voice irq with &80 == 0
			// ret = 0xFF;
			if (!m_vmode)
				ret = voice.vol_ctrl.bitflags.irq ? 0x81 : 0x01;
			else
				ret = 0x01;
			//ret = voice.vol_ctrl.bitflags.value | 0x1;
			ret <<= 8;
			break;

		case 0x0E: // Active Voices
			ret = m_active_osc;
			break;

		case 0x0F:{// [osc] Interrupt source/oscillator
			ret = 0xff;
			for (int i = 0; i <= m_active_osc; i++) {
				ics2115_voice& v = m_voice[i];
				if (v.osc_conf.bitflags.irq_pending || v.vol_ctrl.bitflags.irq_pending) {
					ret = i | 0xe0;
					ret &= v.vol_ctrl.bitflags.irq_pending ? (~0x40) : 0xff;
					ret &= v.osc_conf.bitflags.irq_pending ? (~0x80) : 0xff;
					recalc_irq();
					if (v.osc_conf.bitflags.irq_pending) {
						v.osc_conf.bitflags.irq_pending = 0;
						ret &= ~0x80;
					}
					if (v.vol_ctrl.bitflags.irq_pending) {
						v.vol_ctrl.bitflags.irq_pending = 0;
						ret &= ~0x40;
					}
					break;
				}
			}
			ret <<= 8;
			break;}

		case 0x10: // [osc] Oscillator Control
			ret = voice.osc.ctl << 8;
			break;

		case 0x11: // [osc] Wavesample static address 27-20
			ret = voice.osc.saddr << 8;
			break;

		case 0x40: // Timer 0 clear irq
		case 0x41: // Timer 1 clear irq
			//TODO: examine this suspect code
			ret = m_timer[m_reg_select & 0x1].preset;
			m_irq_pending &= ~(1 << (m_reg_select & 0x1));
			recalc_irq();
			break;

		case 0x43: // Timer status
			ret = m_irq_pending & 3;
			break;

		case 0x4A: // IRQ Pending
			ret = m_irq_pending;
			break;

		case 0x4B: // Address of Interrupting Oscillator
			ret = 0x80;
			break;

		case 0x4C: // Chip Revision
			ret = revision;
			break;

		default:
#ifdef ICS2115_DEBUG
			printf("ICS2115: Unhandled read %x\n", m_reg_select);
#endif
			ret = 0;
			break;
	}
	return ret;
}

void ics2115_device::reg_write(UINT8 data, bool msb) {
	ics2115_voice& voice = m_voice[m_osc_select];

	switch(m_reg_select) {
		case 0x00: // [osc] Oscillator Configuration
			if(msb) {
				voice.osc_conf.value &= 0x80;
				voice.osc_conf.value |= data & 0x7f;
			}
			break;

		case 0x01: // [osc] Wavesample frequency
			// freq = fc*33075/1024 in 32 voices mode, fc*44100/1024 in 24 voices mode
			if(msb)
				voice.osc.fc = (voice.osc.fc & 0x00ff) | (data << 8);
			else
				//last bit not used!
				voice.osc.fc = (voice.osc.fc & 0xff00) | (data & 0xfe);
			break;

		case 0x02: // [osc] Wavesample loop start high
			if(msb)
				voice.osc.start = (voice.osc.start & 0x00ffffff) | (data << 24);
			else
				voice.osc.start = (voice.osc.start & 0xff00ffff) | (data << 16);
			break;

		case 0x03: // [osc] Wavesample loop start low
			if(msb)
				voice.osc.start = (voice.osc.start & 0xffff00ff) | (data << 8);
			// This is unused?
			//else
				//voice.osc.start = (voice.osc.start & 0xffffff00) | (data & 0);
			break;

		case 0x04: // [osc] Wavesample loop end high
			if(msb)
				voice.osc.end = (voice.osc.end & 0x00ffffff) | (data << 24);
			else
				voice.osc.end = (voice.osc.end & 0xff00ffff) | (data << 16);
			break;

		case 0x05: // [osc] Wavesample loop end low
			if(msb)
				voice.osc.end = (voice.osc.end & 0xffff00ff) | (data << 8);
			// lsb is unused?
			break;

		case 0x06: // [osc] Volume Increment
			if(msb)
				voice.vol.incr = data;
			break;

		case 0x07: // [osc] Volume Start
			if (!msb)
				voice.vol.start = data << (10+8);
			break;

		case 0x08: // [osc] Volume End
			if (!msb)
				voice.vol.end = data << (10+8);
			break;

		case 0x09: // [osc] Volume accumulator
			if(msb)
				voice.vol.regacc = (voice.vol.regacc & 0x00ff) | (data << 8);
			else
				voice.vol.regacc = (voice.vol.regacc & 0xff00) | data;
			voice.vol.acc = voice.vol.regacc << 10;
			break;

		case 0x0A: // [osc] Wavesample address high
#ifdef ICS2115_DEBUG
#ifdef ICS2115_ISOLATE
			if(m_osc_select == ICS2115_ISOLATE)
#endif
				printf("<%d:oa:H[%d]=%x>", m_osc_select, msb, data);
#endif
			if(msb)
				voice.osc.acc = (voice.osc.acc & 0x00ffffff) | (data << 24);
			else
				voice.osc.acc = (voice.osc.acc & 0xff00ffff) | (data << 16);
			break;

		case 0x0B: // [osc] Wavesample address low
#ifdef ICS2115_DEBUG
#ifdef ICS2115_ISOLATE
			if(m_osc_select == ICS2115_ISOLATE)
#endif
				printf("<%d:oa:L[%d]=%x>", m_osc_select, msb, data);
#endif
			if(msb)
				voice.osc.acc = (voice.osc.acc & 0xffff00ff) | (data << 8);
			else
				voice.osc.acc = (voice.osc.acc & 0xffffff00) | (data & 0xF8);
			break;

		case 0x0C: // [osc] Pan
			if(msb)
				voice.vol.pan = data;
			break;

		case 0x0D: // [osc] Volume Envelope Control
			if(msb) {
				voice.vol_ctrl.value &= 0x80;
				voice.vol_ctrl.value |= data & 0x7F;
			}
			break;

		case 0x0E: // Active Voices
			//Does this value get added to 1? Not sure. Could trace for writes of 32.
			if(msb) {
				m_active_osc = data & 0x1F; // & 0x1F ? (Guessing)
			}
			break;
		//2X8 ?
		case 0x10: // [osc] Oscillator Control
			//Could this be 2X9?
			//[7 R | 6 M2 | 5 M1 | 4-2 Reserve | 1 - Timer 2 Strt | 0 - Timer 1 Strt]

			if (msb) {
				voice.osc.ctl = data;
				if (!data)
					keyon();
				//guessing here
				else if(data == 0xf) {
#ifdef ICS2115_DEBUG
#ifdef ICS2115_ISOLATE
					if (m_osc_select == ICS2115_ISOLATE)
#endif
					if (!voice.osc_conf.bitflags.stop || !voice.vol_ctrl.bitflags.stop)
						printf("[%02d STOP]\n", m_osc_select);
#endif
					if (!m_vmode) {
						voice.osc_conf.bitflags.stop = true;
						voice.vol_ctrl.bitflags.stop = true;
						//try to key it off as well!
						voice.state.bitflags.on = false;
					}
				}
#ifdef ICS2115_DEBUG
				else
					printf("ICS2115: Unhandled* data write %d onto 0x10.\n", data);
#endif
			}
			break;

		case 0x11: // [osc] Wavesample static address 27-20
			if(msb)
				//v->Osc.SAddr = data;
				voice.osc.saddr = data;
			break;
		case 0x12:
			//Could be per voice! -- investigate.
			if (msb)
				m_vmode = data;
			break;
		case 0x40: // Timer 1 Preset
		case 0x41: // Timer 2 Preset
			if(!msb) {
				m_timer[m_reg_select & 0x1].preset = data;
				recalc_timer(m_reg_select & 0x1);
			}
			break;

		case 0x42: // Timer 1 Prescale
		case 0x43: // Timer 2 Prescale
			if(!msb) {
				m_timer[m_reg_select & 0x1].scale = data;
				recalc_timer(m_reg_select & 0x1);
			}
			break;

		case 0x4A: // IRQ Enable
			if(!msb) {
				m_irq_enabled = data;
				recalc_irq();
			}
			break;

		case 0x4F: // Oscillator Address being Programmed
			if(!msb) {
				m_osc_select = data % (1+m_active_osc);
			}
			break;
		default:
#ifdef ICS2115_DEBUG
			printf("ICS2115: Unhandled write %x onto %x(%d) [voice = %d]\n", data, m_reg_select, msb, m_osc_select);
#endif
			break;
	}
}

READ8_MEMBER(ics2115_device::read)
{
	UINT8 ret = 0;

	switch(offset) {
		case 0:
			//TODO: check this suspect code
			if (m_irq_on) {
				ret |= 0x80;
				if (m_irq_enabled && (m_irq_pending & 3))
					ret |= 1;
				for (int i = 0; i <= m_active_osc; i++) {
					if (//m_voice[i].vol_ctrl.bitflags.irq_pending ||
						m_voice[i].osc_conf.bitflags.irq_pending) {
						ret |= 2;
						break;
					}
				}
			}

			break;
		case 1:
			ret = m_reg_select;
			break;
		case 2:
			ret = (UINT8)(reg_read());
			break;
		case 3:
			ret = reg_read() >> 8;
			break;
		default:
#ifdef ICS2115_DEBUG
			printf("ICS2115: Unhandled memory read at %x\n", offset);
#endif
			break;
	}
	return ret;
}

WRITE8_MEMBER(ics2115_device::write)
{
	switch(offset) {
		case 1:
			m_reg_select = data;
			break;
		case 2:
			reg_write(data,0);
			break;
		case 3:
			reg_write(data,1);
			break;
		default:
#ifdef ICS2115_DEBUG
			printf("ICS2115: Unhandled memory write %02x to %x\n", data, offset);
#endif
			break;
	}
}

void ics2115_device::keyon()
{
#ifdef ICS2115_ISOLATE
	if (m_osc_select != ICS2115_ISOLATE)
		return;
#endif
	//set initial condition (may need to invert?) -- does NOT work since these are set to zero even
	m_voice[m_osc_select].state.bitflags.on = true;
	//no ramp up...
	m_voice[m_osc_select].state.bitflags.ramp = 0x40;

#ifdef ICS2115_DEBUG
	printf("[%02d vs:%04x ve:%04x va:%04x vi:%02x vc:%02x os:%06x oe:%06x oa:%06x of:%04x SA:%02x oc:%02x][%04x]\n", m_osc_select,
			m_voice[m_osc_select].vol.start >> 10,
			m_voice[m_osc_select].vol.end >> 10,
			m_voice[m_osc_select].vol.acc >> 10,
			m_voice[m_osc_select].vol.incr,
			m_voice[m_osc_select].vol_ctrl.value,
			m_voice[m_osc_select].osc.start >> 12,
			m_voice[m_osc_select].osc.end >> 12,
			m_voice[m_osc_select].osc.acc >> 12,
			m_voice[m_osc_select].osc.fc,
			m_voice[m_osc_select].osc.saddr,
			m_voice[m_osc_select].osc_conf.value,
			m_volume[(m_voice[m_osc_select].vol.acc >> 14)]
			);
#endif
	//testing memory corruption issue with mame stream
	//printf("m_volume[0x%x]=0x%x\n", mastervolume, m_volume[mastervolume]);
}

void ics2115_device::recalc_irq()
{
	//Suspect
	bool irq = (m_irq_pending & m_irq_enabled);
	for(int i = 0; (!irq) && (i < 32); i++)
		irq |=  m_voice[i].vol_ctrl.bitflags.irq_pending && m_voice[i].osc_conf.bitflags.irq_pending;
	m_irq_on = irq;
	if(!m_irq_cb.isnull())
		m_irq_cb(irq ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER( ics2115_device::timer_cb_0 )
{
	m_irq_pending |= 1 << 0;
	recalc_irq();
}

TIMER_CALLBACK_MEMBER( ics2115_device::timer_cb_1 )
{
	m_irq_pending |= 1 << 1;
	recalc_irq();
}

void ics2115_device::recalc_timer(int timer)
{
	//Old regression-based formula (minus constant)
	//UINT64 period = m_timer[timer].preset * (m_timer[timer].scale << 16) / 60;

	//New formula based on O.Galibert's reverse engineering of ICS2115 card firmware
	UINT64 period  = ((m_timer[timer].scale & 0x1f) + 1) * (m_timer[timer].preset + 1);
	period = (period << (4 + (m_timer[timer].scale >> 5)))*78125/2646;

	if(m_timer[timer].period != period) {
		m_timer[timer].period = period;
		// Adjust the timer lengths
		if(period) // Reset the length
			m_timer[timer].timer->adjust(attotime::from_nsec(period), 0, attotime::from_nsec(period));
		else // Kill the timer if length == 0
			m_timer[timer].timer->adjust(attotime::never);
	}
}
