//ICS2115 by Raiden II team (c) 2010
//members: austere, nimitz, trap15
//
//Original driver by O. Galibert, ElSemi
//
//Use tab size = 4 for your viewing pleasure.

#include "emu.h"
#include "ics2115.h"
#include <cmath>

#define ICS2115_DEBUG 0

const device_type ICS2115 = ics2115_device_config::static_alloc_device_config;

void ics2115_device_config::static_set_irqf(device_config *device, void (*irqf)(running_device *device, int state))
{
	ics2115_device_config *ics2115 = downcast<ics2115_device_config *>(device);
	ics2115->m_irq_func = irqf;
}

ics2115_device_config::ics2115_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "ICS2115", tag, owner, clock),
	  device_config_sound_interface(mconfig, *this)
{
}

device_config *ics2115_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(ics2115_device_config(mconfig, tag, owner, clock));
}

device_t *ics2115_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, ics2115_device(machine, *this));
}

ics2115_device::ics2115_device(running_machine &machine, const ics2115_device_config &config)
	: device_t(machine, config),
	  device_sound_interface(machine, config, *this),
	  m_config(config),
	  m_irq_cb(m_config.m_irq_func)
{
}

void ics2115_device::device_start()
{
    m_rom = *region();
    m_timer[0].timer = timer_alloc(machine, timer_cb_0, this);
    m_timer[1].timer = timer_alloc(machine, timer_cb_1, this);
    m_stream = stream_create(this, 0, 2, 33075, this, static_stream_generate);

    //Exact formula as per patent 5809466
    //This seems to give the best fit.
    double maxvol = ((1 << volume_bits) - 1) * pow(2., (double)1/0x100);
    for (int i = 0; i < 0x1000; i++) {
           m_volume[i] = floor(maxvol * pow(2.,(double)i/256 - 16) + 0.5);
    }

    //Log rule -- wayyyy too much compression
    /*
    for(int i=0; i<4096; i++) {
        m_volume[i] = (UINT16)((double)0x1000 * log2(i+12270) / 7.);
    }*/

    //austere's table, derived from patent 5809466:
    //See section V starting from page 195
    //Subsection F (column 124, page 198) onwards
    //NOTE: This is 15-bit to maintain decent dynamic range -- shift values down accordingly
    //This on its own does not sound correct, but it follows the poor GF-1 documentation
    //There is a possible logarithmic compression step that follows this later!
    /*for (int i = 0; i<4096; i++) {
        m_volume[i] = ((256 + i & 0xff)<<6) >> (15 - (i >> 8));
    }*/

    //As per MIL-STD-188-113
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

	state_save_register_device_item(this, 0, m_timer[0].period);
	state_save_register_device_item(this, 0, m_timer[0].scale);
	state_save_register_device_item(this, 0, m_timer[0].preset);
	state_save_register_device_item(this, 0, m_timer[1].period);
	state_save_register_device_item(this, 0, m_timer[1].scale);
	state_save_register_device_item(this, 0, m_timer[1].preset);
	state_save_register_device_item(this, 0, m_reg_select);
	state_save_register_device_item(this, 0, m_osc_select);
	state_save_register_device_item(this, 0, m_irq_enabled);
	state_save_register_device_item(this, 0, m_irq_pending);
	state_save_register_device_item(this, 0, m_irq_on);
	state_save_register_device_item(this, 0, m_active_osc);
	state_save_register_device_item(this, 0, m_outhalt);

	for(int i = 0; i < 32; i++) {
		state_save_register_device_item(this, i, m_voice[i].osc_conf.value);
		state_save_register_device_item(this, i, m_voice[i].state.value);
		state_save_register_device_item(this, i, m_voice[i].vol_ctrl.value);
		state_save_register_device_item(this, i, m_voice[i].osc.left);
		state_save_register_device_item(this, i, m_voice[i].osc.acc);
		state_save_register_device_item(this, i, m_voice[i].osc.start);
		state_save_register_device_item(this, i, m_voice[i].osc.end);
		state_save_register_device_item(this, i, m_voice[i].osc.fc);
		state_save_register_device_item(this, i, m_voice[i].osc.ctl);
		state_save_register_device_item(this, i, m_voice[i].osc.saddr);
		state_save_register_device_item(this, i, m_voice[i].vol.left);
		state_save_register_device_item(this, i, m_voice[i].vol.add);
		state_save_register_device_item(this, i, m_voice[i].vol.start);
		state_save_register_device_item(this, i, m_voice[i].vol.end);
		state_save_register_device_item(this, i, m_voice[i].vol.acc);
		state_save_register_device_item(this, i, m_voice[i].vol.regacc);
		state_save_register_device_item(this, i, m_voice[i].vol.incr);
		state_save_register_device_item(this, i, m_voice[i].vol.pan);
		state_save_register_device_item(this, i, m_voice[i].vol.mode);
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
    m_outhalt = 0;
	memset(m_voice, 0, sizeof(m_voice));
	timer_adjust_oneshot(m_timer[0].timer, attotime_never, 0);
	timer_adjust_oneshot(m_timer[1].timer, attotime_never, 0);
	m_timer[0].period = 0;
	m_timer[1].period = 0;
	for(int i = 0; i < 32; i++) {
		m_voice[i].osc_conf.value = 2;
		m_voice[i].osc.fc = 0;
		m_voice[i].osc.acc = 0;
		m_voice[i].osc.start = 0;
		m_voice[i].osc.end = 0;
		m_voice[i].osc.ctl = 0;
		m_voice[i].osc.saddr = 0;
		m_voice[i].vol.acc = 0;
		m_voice[i].vol.incr = 0;
		m_voice[i].vol.start = 0;
		m_voice[i].vol.end = 0;
		m_voice[i].vol.pan = 0x7F;
		m_voice[i].vol_ctrl.value = 1;
		m_voice[i].vol.mode = 0;
		m_voice[i].state.value = 0;
	}
}

STREAM_UPDATE( ics2115_device::static_stream_generate )
{
	reinterpret_cast<ics2115_device *>(param)->stream_generate(inputs, outputs, samples);
}

//TODO: improve using next-state logic from column 126 of patent 5809466
int ics2115_voice::update_volume_envelope()
{
	int ret = 0;
    if(vol_ctrl.done || vol_ctrl.stop)
		return ret;

	if(vol_ctrl.invert) {
		vol.acc -= vol.add;
		vol.left = vol.acc - vol.start;
	} else {
		vol.acc += vol.add;
		vol.left = vol.end - vol.acc;
	}

    //> instead of >= to stop crackling?
	if(vol.left > 0)
		return ret;

	if(vol_ctrl.irq) {
        vol_ctrl.irq_pending = true;
		ret = 1;
	}

	if(osc_conf.eightbit)
		return ret;

	if(vol_ctrl.loop) {
		if(vol_ctrl.loop_bidir)
			vol_ctrl.invert = !vol_ctrl.invert;

		if(vol_ctrl.invert)
			vol.acc = vol.end + vol.left;
		else
			vol.acc = vol.start - vol.left;
	} else {
		state.on = false;
		vol_ctrl.done = true;
		if(vol_ctrl.invert)
			vol.acc = vol.end;
		else
			vol.acc = vol.start;
	}

	return ret;
}

int ics2115_voice::update_oscillator()
{
	int ret = 0;
	if(osc_conf.stop)
		return ret;
	if(osc_conf.invert) {
		osc.acc -= osc.fc << 2;
		osc.left = osc.acc - osc.start;
	} else {
		osc.acc += osc.fc << 2;
		osc.left = osc.end - osc.acc;
	}
    // > instead of >= to stop crackling?
	if(osc.left > 0)
		return ret;
	if(osc_conf.irq) {
		osc_conf.irq_pending = true;
        ret = 1;
	}
	if(osc_conf.loop) {
        //This possibly does not function correctly, sometimes it starts playing sounds continuously
        //For example, the PGM test after deactivating shortly after activation. The possible problem
        //Might be masked by the keyoff command, uncomment voice.state.on = false to reveal it.
		if(osc_conf.loop_bidir)
			osc_conf.invert = !osc_conf.invert;

		if(osc_conf.invert)
			osc.acc = osc.end + osc.left;
		else
			osc.acc = osc.start - osc.left;
	}else{
		state.on = false;
		osc_conf.stop = true;
		if(osc_conf.invert)
			osc.acc = osc.end;
		else
			osc.acc = osc.start;
	}
	return ret;
}

stream_sample_t ics2115_device::get_sample(ics2115_voice& voice, UINT32 curaddr)
{
    INT16 sample1, sample2;
    if (voice.osc_conf.eightbit) {
        sample1 = ((INT8)m_rom[curaddr]) << 8;
        sample2 = ((INT8)m_rom[curaddr + 1]) << 8;
    }
    else {
        sample1 = m_rom[curaddr + 0] | (((INT8)m_rom[curaddr + 1]) << 8);
        sample2 = m_rom[curaddr + 2] | (((INT8)m_rom[curaddr + 3]) << 8);
    }
    
    //linear interpolation -- TODO: program the case where FC skips over samples
    //may cause some glitches at the end -- probably needs special termination condition
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
    return state.on && !((vol_ctrl.done || vol_ctrl.stop) && osc_conf.stop);
}

int ics2115_device::fill_output(ics2115_voice& voice, stream_sample_t *outputs[2], int samples)
{
    bool irq_invalid = false;
    UINT16 fine = 1 << (3*(voice.vol.incr >> 6));
    voice.vol.add = (voice.vol.incr & 0x3F)<< (10 - fine);

    for (int i = 0; i < samples; i++) {
        UINT32 volacc = (voice.vol.acc >> 10) & 0xffff;
        UINT16 vleft = m_volume[volacc >> 4]; //* (255 - voice.vol.pan) / 0x80];
        UINT16 vright = m_volume[volacc >> 4]; //* (voice.vol.pan + 1) / 0x80];
        
        //From GUS doc:
        //In general, it is necessary to remember that all voices are being summed in to the
        //final output, even if they are not running.  This means that whatever data value
        //that the voice is pointing at is contributing to the summation.
        //(austere note: this will of course fix some of the glitches due to multiple transition)
        UINT32 curaddr = ((voice.osc.saddr << 20) & 0xffffff) | (voice.osc.acc >> 12);
        stream_sample_t sample;
        if(voice.osc_conf.ulaw)
            sample = m_ulaw[m_rom[curaddr]];
        else
            sample = get_sample(voice, curaddr);
        
        //15-bit volume + (5-bit worth of 32 channel sum) + 16-bit samples = 4-bit extra
        if (!m_outhalt || voice.playing()) {
            outputs[0][i] += (sample * vleft) >> (5 + volume_bits - 16);
            outputs[1][i] += (sample * vright) >> (5 + volume_bits - 16);
        }

        if (voice.playing()) {
            if (voice.update_oscillator())
                irq_invalid = true;
            if (voice.update_volume_envelope())
                irq_invalid = true;
        }
    }
    return irq_invalid;
}

void ics2115_device::stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	memset(outputs[0], 0, samples * sizeof(stream_sample_t));
	memset(outputs[1], 0, samples * sizeof(stream_sample_t));

	bool irq_invalid = false;
	for(int osc = 0; osc <= m_active_osc; osc++) {
		ics2115_voice& voice = m_voice[osc];

		if(fill_output(voice, outputs, samples))
			irq_invalid = true;
#if ICS2115_DEBUG
        if(voice.playing())
            printf("%d ", osc);
#endif
	}
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
			//ret = v->Vol.Incr;
			ret = voice.vol.incr;
			break;

		case 0x07: // [osc] Volume Start
			//ret = v->Vol.Start;
			ret = voice.vol.start >> (10+8);
			break;

		case 0x08: // [osc] Volume End
			//ret = v->Vol.End;
			ret = voice.vol.end >> (10+8);
			break;

		case 0x09: // [osc] Volume accumulator
			//ret = v->Vol.Acc;
			ret = voice.vol.acc  >> (10);
			break;

		case 0x0A: // [osc] Wavesample address
			//ret = (v->Osc.Acc >> 16) & 0xFFFF;
			ret = (voice.osc.acc >> 16) & 0xffff;
			break;

		case 0x0B: // [osc] Wavesample address
			//ret = (v->Osc.Acc >> 0) & 0xFFF8;
			ret = (voice.osc.acc >> 0) & 0xfff8;
			break;


		case 0x0C: // [osc] Pan
			//ret = v->Vol.Pan << 8;
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
            if (!m_outhalt)
			    ret = voice.vol_ctrl.irq ? 0x81 : 0x01;
            else
                ret = 0x01;
            //ret = voice.vol_ctrl.value | 0x1;
			ret <<= 8;
			break;

		case 0x0E: // Active Voices
			ret = m_active_osc;
			break;

		case 0x0F:{// [osc] Interrupt source/oscillator
			ret = 0xff;
			for (int i = 0; i <= m_active_osc; i++) {
			    ics2115_voice& v = m_voice[i];
			    if (v.osc_conf.irq_pending || v.vol_ctrl.irq_pending) {
					ret = i | 0xe0;
					ret &= v.vol_ctrl.irq_pending ? (~0x40) : 0xff;
					ret &= v.osc_conf.irq_pending ? (~0x80) : 0xff;
					recalc_irq();
					if (v.osc_conf.irq_pending) {
                    	v.osc_conf.irq_pending = 0;
                    	ret &= ~0x80;
                	}
                	if (v.vol_ctrl.irq_pending) {
                    	v.vol_ctrl.irq_pending = 0;
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
#if ICS2115_DEBUG
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
			if(msb)
				voice.osc.acc = (voice.osc.acc & 0x00ffffff) | (data << 24);
			else
				voice.osc.acc = (voice.osc.acc & 0xff00ffff) | (data << 16);
			break;

		case 0x0B: // [osc] Wavesample address low
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
#if ICS2115_DEBUG
                    if (!voice.osc_conf.stop || !voice.vol_ctrl.stop)
                        printf("[%02d STOP]\n", m_osc_select);
#endif
                    if (!m_outhalt) {
                        voice.osc_conf.stop = true;
                        voice.vol_ctrl.stop = true;
                        //try to key it off as well!
                        voice.state.on = false;
                    }
                }
#if ICS2115_DEBUG
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
                m_outhalt = data;
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
#if ICS2115_DEBUG
            printf("ICS2115: Unhandled write %x onto %x(%d) [voice = %d]\n", data, m_reg_select, msb, m_osc_select);
#endif
            break;
    }
}

//UINT8 ics2115_device::read (offs_t offset)
READ8_DEVICE_HANDLER(ics2115_device::read)
{
    ics2115_device *chip = downcast<ics2115_device *>(device);
    UINT8 ret = 0;

    switch(offset) {
        case 0:
            //TODO: check this suspect code
            if (chip->m_irq_on) {
                ret |= 0x80;
                if (chip->m_irq_enabled && (chip->m_irq_pending & 3))
                    ret |= 1;
                for (int i = 0; i <= chip->m_active_osc; i++) {
                    if (chip->m_voice[i].vol_ctrl.irq_pending ||
                        chip->m_voice[i].osc_conf.irq_pending) {
                        ret |= 2;
                        break;
                    }
                }
            }

            break;
        case 1:
            ret = chip->m_reg_select;
            break;
        case 2:
            ret = (UINT8)(chip->reg_read());
            break;
        case 3:
            ret = chip->reg_read() >> 8;
            break;
        default:
#if ICS2115_DEBUG
            printf("ICS2115: Unhandled memory read at %x\n", offset);
#endif
            break;
    }
    return ret;
}

//UINT8 ics2115_device::write(offs_t offset, UINT8 data)
WRITE8_DEVICE_HANDLER(ics2115_device::write)
{
    ics2115_device *chip = downcast<ics2115_device *>(device);
    switch(offset) {
        case 1:
            chip->m_reg_select = data;
            break;
        case 2:
            chip->reg_write(data,0);
            break;
        case 3:
            chip->reg_write(data,1);
            break;
        default:
#if ICS2115_DEBUG
            printf("ICS2115: Unhandled memory write %02x to %x\n", data, offset);
#endif
            break;
    }
}

void ics2115_device::keyon()
{
    //set initial condition (may need to invert?) -- does NOT work since these are set to zero even
    m_voice[m_osc_select].state.on = true;

#if ICS2115_DEBUG
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
		irq |=  m_voice[i].vol_ctrl.irq_pending && m_voice[i].osc_conf.irq_pending;
	m_irq_on = irq;
	if(m_irq_cb)
		m_irq_cb(this, irq ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_CALLBACK( ics2115_device::timer_cb_0 )
{
	ics2115_device *chip = (ics2115_device *)ptr;
	chip->m_irq_pending |= 1 << 0;
    chip->recalc_irq();
}

TIMER_CALLBACK( ics2115_device::timer_cb_1 )
{
	ics2115_device *chip = (ics2115_device *)ptr;
	chip->m_irq_pending |= 1 << 1;
	chip->recalc_irq();
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
			timer_adjust_periodic(m_timer[timer].timer, ATTOTIME_IN_NSEC(period), 0, ATTOTIME_IN_NSEC(period));
		else // Kill the timer if length == 0
			timer_adjust_oneshot(m_timer[timer].timer, attotime_never, 0);
	}
}

//Unused code -- might be useful in the future.

/*
//0000MMMMMMMMXXXX -> MMMMMMMM
//EEEEMMMMMMMMXXXX -> MMMMMMMM.XXXX << EEEE
UINT32 vol_16float2int(UINT16 x) {
    UINT8 extra = x & 0xf;
    UINT8 mantissa = (x & 0xff0) >> 4;
    INT8 exponent = (x & 0xf000) >> 12;

    UINT8 extrashift = (4 - exponent > 0) ? (4-exponent) : 0;

    return (mantissa<< exponent) | ((extra >> extrashift) << (exponent - 4));
}

//0000MMMM -> MMMM0000
UINT32 vol_8float2int(UINT8 x) {
    UINT8 mantissa = (x & 0xf);
    UINT8 exponent = (x & 0x70) >> 4;

    return mantissa<< (exponent + 4);
}*/
