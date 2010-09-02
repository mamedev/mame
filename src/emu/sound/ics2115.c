/*
   ICS 2115 sound synthesizer.

   By O. Galibert, with a lot of help from the nebula
   ics emulation by Elsemi.
*/

#include "emu.h"
#include "streams.h"
#include "ics2115.h"

#define ICS2115LOGERROR 0

// a:401ae90.000 l:1c23c.0 e:1e1d8.0  09  tone
// a:4023c40.000 l:25d60.0 e:266cb.0  08  tone
// a:4028fc0.000 l:28fc0.0 e:2b6ef.0  01  violon, noisy
// a:4034020.000 l:34560.0 e:36001.0  19  percussion
// a:4034218.1e8 l:34560.0 e:36001.0  19  percussion
// a:4037f10.000 l:37f3e.0 e:39cb7.0  08  tone
// a:4044f10.000 l:463ea.0 e:46476.0  09  tone
// a:40490d0.000 l:760e9.0 e:910d8.0  19  percussion moche
// a:4051bd0.000 l:51bd0.0 e:528df.0  01  percussion
// a:40621f0.000 l:621f0.0 e:62aef.0  01  percussion faible
// a:4063430.000 l:63c78.0 e:63d25.0  08  tone
// a:40668a0.000 l:668a0.0 e:670ec.0  01  percussion
// a:4067940.000 l:67940.0 e:68140.0  01  percussion
// a:40aff36.000 l:aff36.0 e:b194d.0  20  Selection menu
// a:40b5f26.000 l:b5f26.0 e:b63a5.0  20  Move up/down
// a:4102772.000 l:02772.0 e:03a31.0  20  Voice test (screwed up?)

// conf:
//   10b6: 00
//   11ee: 20
//   1867: a0
//   188b: 00
//   20ba: 01 08 09
//   2299: 01 09 19

enum { V_ON = 1, V_DONE = 2 };

typedef struct _ics2115_state ics2115_state;
struct _ics2115_state
{
	const ics2115_interface *intf;
	running_device *device;
	UINT8 *rom;
	INT16 *ulaw;

	struct {
		UINT16 fc, addrh, addrl, strth, endh, volacc;
		UINT8 strtl, endl, saddr, pan, conf, ctl;
		UINT8 vstart, vend, vctl;
		UINT8 state;
	} voice[32];

	struct {
		UINT8 scale, preset;
		emu_timer *timer;
		UINT64 period;	/* in nsec */
	} timer[2];

	UINT8 reg, osc;
	UINT8 irq_en, irq_pend;
	int irq_on;
	sound_stream * stream;
};

INLINE ics2115_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == ICS2115);
	return (ics2115_state *)downcast<legacy_device_base *>(device)->token();
}

static void recalc_irq(ics2115_state *chip)
{
	int i;
	int irq = 0;
	if (chip->irq_en & chip->irq_pend)
		irq = 1;
	for (i = 0; !irq && i<32; i++)
		if (chip->voice[i].state & V_DONE)
			irq = 1;
	if (irq != chip->irq_on)
	{
		chip->irq_on = irq;
		if (chip->intf->irq_cb)
			chip->intf->irq_cb(chip->device, irq ? ASSERT_LINE : CLEAR_LINE);
	}
}


static STREAM_UPDATE( update )
{
	ics2115_state *chip = (ics2115_state *)param;
	int osc, i;
	int rec_irq = 0;

	memset(outputs[0], 0, samples*sizeof(*outputs[0]));
	memset(outputs[1], 0, samples*sizeof(*outputs[0]));

	for (osc = 0; osc < 32; osc++)
		if (chip->voice[osc].state & V_ON)
		{
			UINT32 adr = (chip->voice[osc].addrh << 16) | chip->voice[osc].addrl;
			UINT32 end = (chip->voice[osc].endh << 16) | (chip->voice[osc].endl << 8);
			UINT32 loop = (chip->voice[osc].strth << 16) | (chip->voice[osc].strtl << 8);
			UINT32 badr = (chip->voice[osc].saddr << 20) & 0xffffff;
			UINT32 delta = chip->voice[osc].fc << 2;
			UINT8 conf = chip->voice[osc].conf;
			INT32 vol = chip->voice[osc].volacc;
			vol = (((vol & 0xff0)|0x1000)<<(vol>>12))>>12;

			if (ICS2115LOGERROR) logerror("ICS2115: KEYRUN %02d adr=%08x end=%08x delta=%08x\n",
					 osc, adr, end, delta);

			for (i = 0; i < samples; i++)
			{
				INT32 v = chip->rom[badr|(adr >> 12)];
				if (conf & 1)
					v = chip->ulaw[v];
				else
					v = ((INT8)v) << 6;

				v = (v*vol)>>(16+5);
				outputs[0][i] += v;
				outputs[1][i] += v;
				adr += delta;
				if (adr >= end)
				{
					if (ICS2115LOGERROR) logerror("ICS2115: KEYDONE %2d\n", osc);
					adr -= (end-loop);
					chip->voice[osc].state &= ~V_ON;
					chip->voice[osc].state |= V_DONE;
					rec_irq = 1;
					break;
				}
			}
			chip->voice[osc].addrh = adr >> 16;
			chip->voice[osc].addrl = adr;
		}
	if (rec_irq)
		recalc_irq(chip);
}

static void keyon(ics2115_state *chip, int osc)
{
	if (ICS2115LOGERROR) logerror("ICS2115: KEYON %2d conf:%02x vctl:%02x a:%07x.%03x l:%05x.%x e:%05x.%x v:%03x f:%d\n",
			osc,
			chip->voice[chip->osc].conf,
			chip->voice[chip->osc].vctl,
			(chip->voice[osc].saddr << 20)|(chip->voice[osc].addrh << 4)|(chip->voice[osc].addrl >> 12),
			(chip->voice[osc].addrl >> 3) & 0x1ff,
			(chip->voice[osc].strth << 4)|(chip->voice[osc].strtl >> 4),
			chip->voice[osc].strtl & 0xf,
			(chip->voice[osc].endh << 4)|(chip->voice[osc].endl >> 4),
			chip->voice[osc].endl & 0xf,
			chip->voice[osc].volacc>>4,
			(chip->voice[chip->osc].fc*33075+512)/1024);
	chip->voice[osc].state |= V_ON;
}


static TIMER_CALLBACK( timer_cb_0 )
{
	ics2115_state *chip = (ics2115_state *)ptr;
	chip->irq_pend |= 1<<0;
	recalc_irq(chip);
}

static TIMER_CALLBACK( timer_cb_1 )
{
	ics2115_state *chip = (ics2115_state *)ptr;
	chip->irq_pend |= 1<<1;
	recalc_irq(chip);
}

static void recalc_timer(ics2115_state *chip, int timer)
{
	UINT64 period = 1000000000 * chip->timer[timer].scale*chip->timer[timer].preset / 33868800;
	if (period)
		period = 1000000000/62.8206;

	if (period)
	{
		if (ICS2115LOGERROR) logerror("ICS2115: timer %d freq=%fHz\n", timer, 1.0/(period / 1000000000.0));
	}
	else
	{
		if (ICS2115LOGERROR) logerror("ICS2115: timer %d off\n", timer);
	}

	if (chip->timer[timer].period != period)
	{
		chip->timer[timer].period = period;
		if (period)
			timer_adjust_periodic(chip->timer[timer].timer, ATTOTIME_IN_NSEC(period), 0, ATTOTIME_IN_NSEC(period));
		else
			timer_adjust_oneshot(chip->timer[timer].timer, attotime_never, 0);
	}
}


static void ics2115_reg_w(ics2115_state *chip, UINT8 reg, UINT8 data, int msb)
{
	running_machine *machine = chip->device->machine;

	switch (reg)
	{
	case 0x00: // [osc] Oscillator Configuration
		if (msb)
		{
			chip->voice[chip->osc].conf = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: conf = %02x\n", cpuexec_describe_context(machine), chip->osc,
					 chip->voice[chip->osc].conf);
		}
		break;

	case 0x01: // [osc] Wavesample frequency
		// freq = fc*33075/1024 in 32 voices mode, fc*44100/1024 in 24 voices mode
		if (msb)
			chip->voice[chip->osc].fc = (chip->voice[chip->osc].fc & 0xff)|(data << 8);
		else
			chip->voice[chip->osc].fc = (chip->voice[chip->osc].fc & 0xff00)|data;
		if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: fc = %04x (%dHz)\n", cpuexec_describe_context(machine), chip->osc,
				 chip->voice[chip->osc].fc, chip->voice[chip->osc].fc*33075/1024);
		break;

	case 0x02: // [osc] Wavesample loop start address 19-4
		if (msb)
			chip->voice[chip->osc].strth = (chip->voice[chip->osc].strth & 0xff)|(data << 8);
		else
			chip->voice[chip->osc].strth = (chip->voice[chip->osc].strth & 0xff00)|data;
		if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: strth = %04x\n", cpuexec_describe_context(machine), chip->osc,
				 chip->voice[chip->osc].strth);
		break;

	case 0x03: // [osc] Wavesample loop start address 3-0.3-0
		if (msb)
		{
			chip->voice[chip->osc].strtl = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: strtl = %02x\n", cpuexec_describe_context(machine), chip->osc,
					 chip->voice[chip->osc].strtl);
		}
		break;

	case 0x04: // [osc] Wavesample loop end address 19-4
		if (msb)
			chip->voice[chip->osc].endh = (chip->voice[chip->osc].endh & 0xff)|(data << 8);
		else
			chip->voice[chip->osc].endh = (chip->voice[chip->osc].endh & 0xff00)|data;
		if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: endh = %04x\n", cpuexec_describe_context(machine), chip->osc,
				 chip->voice[chip->osc].endh);
		break;

	case 0x05: // [osc] Wavesample loop end address 3-0.3-0
		if (msb)
		{
			chip->voice[chip->osc].endl = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: endl = %02x\n", cpuexec_describe_context(machine), chip->osc,
					 chip->voice[chip->osc].endl);
		}
		break;

	case 0x07: // [osc] Volume Start
		if (msb)
		{
			chip->voice[chip->osc].vstart = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: vstart = %02x\n", cpuexec_describe_context(machine), chip->osc,
					 chip->voice[chip->osc].vstart);
		}
		break;

	case 0x08: // [osc] Volume End
		if (msb)
		{
			chip->voice[chip->osc].vend = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: vend = %02x\n", cpuexec_describe_context(machine), chip->osc,
					 chip->voice[chip->osc].vend);
		}
		break;

	case 0x09: // [osc] Volume accumulator
		if (msb)
			chip->voice[chip->osc].volacc = (chip->voice[chip->osc].volacc & 0xff)|(data << 8);
		else
			chip->voice[chip->osc].volacc = (chip->voice[chip->osc].volacc & 0xff00)|data;
		if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: volacc = %04x\n", cpuexec_describe_context(machine), chip->osc,
				 chip->voice[chip->osc].volacc);
		break;

	case 0x0a: // [osc] Wavesample address 19-4
		if (msb)
			chip->voice[chip->osc].addrh = (chip->voice[chip->osc].addrh & 0xff)|(data << 8);
		else
			chip->voice[chip->osc].addrh = (chip->voice[chip->osc].addrh & 0xff00)|data;
		if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: addrh = %04x\n", cpuexec_describe_context(machine), chip->osc,
				 chip->voice[chip->osc].addrh);
		break;

	case 0x0b: // [osc] Wavesample address 3-0.8-0
		if (msb)
			chip->voice[chip->osc].addrl = (chip->voice[chip->osc].addrl & 0xff)|(data << 8);
		else
			chip->voice[chip->osc].addrl = (chip->voice[chip->osc].addrl & 0xff00)|data;
		if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: addrl = %04x\n", cpuexec_describe_context(machine), chip->osc,
				 chip->voice[chip->osc].addrl);
		break;


	case 0x0c: // [osc] Pan
		if (msb)
		{
			chip->voice[chip->osc].pan = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: pan = %02x\n", cpuexec_describe_context(machine), chip->osc,
					 chip->voice[chip->osc].pan);
		}
		break;

	case 0x0d: // [osc] Volume Enveloppe Control
		if (msb)
		{
			chip->voice[chip->osc].vctl = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: vctl = %02x\n", cpuexec_describe_context(machine), chip->osc,
					 chip->voice[chip->osc].vctl);
		}
		break;

	case 0x10: // [osc] Oscillator Control
		if (msb)
		{
			chip->voice[chip->osc].ctl = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: ctl = %02x\n", cpuexec_describe_context(machine), chip->osc,
					 chip->voice[chip->osc].ctl);
			if (data == 0)
				keyon(chip, chip->osc);
		}
		break;

	case 0x11: // [osc] Wavesample static address 27-20
		if (msb)
		{
			chip->voice[chip->osc].saddr = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: saddr = %02x\n", cpuexec_describe_context(machine), chip->osc,
					 chip->voice[chip->osc].saddr);
		}
		break;

	case 0x40: // Timer 1 Preset
		if (!msb)
		{
			chip->timer[0].preset = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: t1preset = %d\n", cpuexec_describe_context(machine), chip->timer[0].preset);
			recalc_timer(chip, 0);
		}
		break;

	case 0x41: // Timer 2 Preset
		if (!msb)
		{
			chip->timer[1].preset = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: t2preset = %d\n", cpuexec_describe_context(machine), chip->timer[1].preset);
			recalc_timer(chip, 1);
		}
		break;

	case 0x42: // Timer 1 Prescaler
		if (!msb)
		{
			chip->timer[0].scale = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: t1scale = %d\n", cpuexec_describe_context(machine), chip->timer[0].scale);
			recalc_timer(chip, 0);
		}
		break;

	case 0x43: // Timer 2 Prescaler
		if (!msb)
		{
			chip->timer[1].scale = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: t2scale = %d\n", cpuexec_describe_context(machine), chip->timer[1].scale);
			recalc_timer(chip, 1);
		}
		break;

	case 0x4a: // IRQ Enable
		if (!msb)
		{
			chip->irq_en = data;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: irq_en = %02x\n", cpuexec_describe_context(machine), chip->irq_en);
			recalc_irq(chip);
		}
		break;

	case 0x4f: // Oscillator Address being Programmed
		if (!msb)
		{
			chip->osc = data & 31;
			if (ICS2115LOGERROR) logerror("%s:ICS2115: oscnumber = %d\n", cpuexec_describe_context(machine), chip->osc);
		}
		break;

	default:
		if (ICS2115LOGERROR) logerror("%s:ICS2115: write %02x, %02x:%d\n", cpuexec_describe_context(machine), reg, data, msb);
	}
}

static UINT16 ics2115_reg_r(ics2115_state *chip, UINT8 reg)
{
	running_machine *machine = chip->device->machine;

	switch (reg)
	{
	case 0x0d: // [osc] Volume Enveloppe Control
		if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: read vctl\n", cpuexec_describe_context(machine), chip->osc);
		//      res = chip->voice[chip->osc].vctl << 8;
		// may expect |8 on voice irq with &40 == 0
		// may expect |8 on reg 0 on voice irq with &80 == 0
		return 0x100;

	case 0x0f:{// [osc] Interrupt source/oscillator
		int osc;
		UINT8 res = 0xff;
		for (osc = 0; osc < 32; osc++)
			if (chip->voice[osc].state & V_DONE)
			{
				chip->voice[osc].state &= ~V_DONE;
				if (ICS2115LOGERROR) logerror("ICS2115: KEYOFF %2d\n", osc);
				recalc_irq(chip);
				res = 0x40 | osc; // 0x40 ? 0x80 ?
				break;
			}
		if (ICS2115LOGERROR) logerror("%s:ICS2115: read irqv %02x\n", cpuexec_describe_context(machine), res);
		return res << 8;
	}

	case 0x40: // Timer 0 clear irq
		//      if (ICS2115LOGERROR) logerror("%s:ICS2115: clear timer 0\n", cpuexec_describe_context(machine));
		chip->irq_pend &= ~(1<<0);
		recalc_irq(chip);
		return chip->timer[0].preset;

	case 0x41: // Timer 1 clear irq
		if (ICS2115LOGERROR) logerror("%s:ICS2115: clear timer 1\n", cpuexec_describe_context(machine));
		chip->irq_pend &= ~(1<<1);
		recalc_irq(chip);
		return chip->timer[1].preset;

	case 0x43: // Timer status
		//      if (ICS2115LOGERROR) logerror("%s:ICS2115: read timer status %02x\n", cpuexec_describe_context(machine), chip->irq_pend & 3);
		return chip->irq_pend & 3;

	case 0x4a: // IRQ Pending
		if (ICS2115LOGERROR) logerror("%s:ICS2115: read irq_pend %02x\n", cpuexec_describe_context(machine), chip->irq_pend);
		return chip->irq_pend;

	case 0x4b: // Address of Interrupting Oscillator
		if (ICS2115LOGERROR) logerror("%s:ICS2115: %2d: read intoscaddr\n", cpuexec_describe_context(machine), chip->osc);
		return 0x80;

	case 0x4c: // Chip revision
		if (ICS2115LOGERROR) logerror("%s:ICS2115: read revision\n", cpuexec_describe_context(machine));
		return 0x01;

	default:
		if (ICS2115LOGERROR) logerror("%s:ICS2115: read %02x unmapped\n", cpuexec_describe_context(machine), reg);
		return 0;
	}
}


static DEVICE_START( ics2115 )
{
	ics2115_state *chip = get_safe_token(device);
	int i, vv;

	chip->device = device;
	chip->intf = (const ics2115_interface *)device->baseconfig().static_config();
	chip->rom = *device->region();
	chip->timer[0].timer = timer_alloc(device->machine, timer_cb_0, chip);
	chip->timer[1].timer = timer_alloc(device->machine, timer_cb_1, chip);
	chip->ulaw = auto_alloc_array(device->machine, INT16, 256);
	chip->stream = stream_create(device, 0, 2, 33075, chip, update);

	for (i = 0; i < 256; i++)
	{
		UINT8 c = ~i;
		int v;
		v = ((c & 15) << 1) + 33;
		v <<= ((c & 0x70) >> 4);
		if (c & 0x80)
			v = 33 - v;
		else
			v = v - 33;
		chip->ulaw[i] = v;
	}

	state_save_register_device_item(device, 0, chip->timer[0].period);
	state_save_register_device_item(device, 0, chip->timer[0].scale);
	state_save_register_device_item(device, 0, chip->timer[0].preset);
	state_save_register_device_item(device, 0, chip->timer[1].period);
	state_save_register_device_item(device, 0, chip->timer[1].scale);
	state_save_register_device_item(device, 0, chip->reg);
	state_save_register_device_item(device, 0, chip->osc);
	state_save_register_device_item(device, 0, chip->irq_en);
	state_save_register_device_item(device, 0, chip->irq_pend);
	state_save_register_device_item(device, 0, chip->irq_on);

	for (vv = 0; vv < 32; vv++)
	{
		state_save_register_device_item(device, vv, chip->voice[vv].fc);
		state_save_register_device_item(device, vv, chip->voice[vv].addrh);
		state_save_register_device_item(device, vv, chip->voice[vv].addrl);
		state_save_register_device_item(device, vv, chip->voice[vv].strth);
		state_save_register_device_item(device, vv, chip->voice[vv].endh);
		state_save_register_device_item(device, vv, chip->voice[vv].volacc);
		state_save_register_device_item(device, vv, chip->voice[vv].strtl);
		state_save_register_device_item(device, vv, chip->voice[vv].endl);
		state_save_register_device_item(device, vv, chip->voice[vv].saddr);
		state_save_register_device_item(device, vv, chip->voice[vv].pan);
		state_save_register_device_item(device, vv, chip->voice[vv].conf);
		state_save_register_device_item(device, vv, chip->voice[vv].ctl);
		state_save_register_device_item(device, vv, chip->voice[vv].vstart);
		state_save_register_device_item(device, vv, chip->voice[vv].vend);
		state_save_register_device_item(device, vv, chip->voice[vv].vctl);
		state_save_register_device_item(device, vv, chip->voice[vv].state);
	}
}

READ8_DEVICE_HANDLER( ics2115_r )
{
	ics2115_state *chip = get_safe_token(device);
	switch (offset)
	{
	case 0:
		{
		UINT8 res = 0;
		if (chip->irq_on)
		{
			int i;
			res |= 0x80;
			if (chip->irq_en & chip->irq_pend & 3)
				res |= 1; // Timer irq
			for (i = 0; i < 32; i++)
				if (chip->voice[i].state & V_DONE)
				{
					res |= 2;
					break;
				}
		}
		//      if (ICS2115LOGERROR) logerror("%s:ICS2115: read status %02x\n", cpuexec_describe_context(space->machine), res);

		return res;
	}
	case 1:
		return chip->reg;
	case 2:
		return ics2115_reg_r(chip, chip->reg);
	case 3:
	default:
		return ics2115_reg_r(chip, chip->reg) >> 8;
	}
}

WRITE8_DEVICE_HANDLER( ics2115_w )
{
	ics2115_state *chip = get_safe_token(device);
	switch (offset)
	{
	case 1:
		chip->reg = data;
		break;
	case 2:
		ics2115_reg_w(chip, chip->reg, data, 0);
		break;
	case 3:
		ics2115_reg_w(chip, chip->reg, data, 1);
		break;
	}
	//  if (ICS2115LOGERROR) logerror("ICS2115: wi %d, %02x\n", cpuexec_describe_context(space->machine), offset, data);
}

static DEVICE_RESET( ics2115 )
{
	ics2115_state *chip = get_safe_token(device);
	chip->irq_en = 0;
	chip->irq_pend = 0;
	memset(chip->voice, 0, sizeof(chip->voice));
	timer_adjust_oneshot(chip->timer[0].timer, attotime_never, 0);
	timer_adjust_oneshot(chip->timer[1].timer, attotime_never, 0);
	chip->timer[0].period = 0;
	chip->timer[1].period = 0;
	recalc_irq(chip);
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( ics2115 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ics2115_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( ics2115 );		break;
		case DEVINFO_FCT_STOP:							/* nothing */									break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( ics2115 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ICS2115");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "ICS");							break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.01");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(ICS2115, ics2115);
