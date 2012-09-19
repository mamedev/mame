/*
    Panasonic MN10200 emulator

    Written by Olivier Galibert
    MAME conversion, timers, and IRQ controller by R. Belmont

*/

#include "emu.h"
#include "debugger.h"
#include "mn10200.h"

#define log_write(...)
#define log_event(...)

#define MEM_BYTE (0)
#define MEM_WORD (1)

#define NUM_PRESCALERS (2)
#define NUM_TIMERS_8BIT	(10)
#define NUM_IRQ_GROUPS (31)

extern int mn102_disassemble(char *buffer, UINT32 pc, const UINT8 *oprom);

struct mn102_info
{
	// The UINT32s are really UINT24
	UINT32 pc;
	UINT32 d[4];
	UINT32 a[4];

	UINT8 nmicr, iagr;
	UINT8 icrl[NUM_IRQ_GROUPS], icrh[NUM_IRQ_GROUPS];
	UINT16 psw;
	UINT16 mdr;

	struct {
		UINT8 mode;
		UINT8 base;
		UINT8 cur;
	} simple_timer[NUM_TIMERS_8BIT];

	emu_timer *timer_timers[NUM_TIMERS_8BIT];

	struct {
		UINT8 cycles;
		UINT8 mode;
	} prescaler[NUM_PRESCALERS];

	struct {
		UINT32 adr;
		UINT32 count;
		UINT16 iadr;
		UINT8 ctrll, ctrlh, irq;
	} dma[8];

	struct {
		UINT8 ctrll, ctrlh;
		UINT8 buf;
	} serial[2];

	UINT8 ddr[8];

	int cycles;

	legacy_cpu_device *device;
	address_space *program;
	address_space *io;
};

static void mn10200_w(mn102_info *mn102, UINT32 adr, UINT32 data, int type);
static UINT32 mn10200_r(mn102_info *mn102, UINT32 adr, int type);

INLINE UINT8 mn102_read_byte(mn102_info *mn102, UINT32 address)
{
	if (address >= 0xfc00 && address < 0x10000)
	{
		return mn10200_r(mn102, address-0xfc00, MEM_BYTE);
	}

	return mn102->program->read_byte(address);
}

INLINE UINT16 mn102_read_word(mn102_info *mn102, UINT32 address)
{
	if (address >= 0xfc00 && address < 0x10000)
	{
		return mn10200_r(mn102, address-0xfc00, MEM_WORD);
	}

	if (address & 1)
	{
		return mn102->program->read_byte(address) | (mn102->program->read_byte(address+1)<<8);
	}

	return mn102->program->read_word(address);
}

INLINE void mn102_write_byte(mn102_info *mn102, UINT32 address, UINT8 data)
{
	if (address >= 0xfc00 && address < 0x10000)
	{
		mn10200_w(mn102, address-0xfc00, data, MEM_BYTE);
		return;
	}

	mn102->program->write_byte(address, data);
}

INLINE void mn102_write_word(mn102_info *mn102, UINT32 address, UINT16 data)
{
	if (address >= 0xfc00 && address < 0x10000)
	{
		mn10200_w(mn102, address-0xfc00, data, MEM_WORD);
		return;
	}

	if (address & 1)
	{
		mn102->program->write_byte(address, data&0xff);
		mn102->program->write_byte(address+1, (data>>8)&0xff);
		return;
	}

	mn102->program->write_word(address, data);
}

INLINE INT32 r24u(mn102_info *mn102, offs_t adr)
{
	return mn102_read_word(mn102, adr)|(mn102_read_byte(mn102, adr+2)<<16);
}

INLINE void w24(mn102_info *mn102, offs_t adr, UINT32 val)
{
/*  if(adr == 0x4075aa || adr == 0x40689a || adr == 0x4075a2) {
        log_write("TRACE", adr, val, MEM_LONG);
    }*/
	mn102_write_byte(mn102, adr, val);
	mn102_write_byte(mn102, adr+1, val>>8);
	mn102_write_byte(mn102, adr+2, val>>16);
}

INLINE void mn102_change_pc(mn102_info *mn102, UINT32 pc)
{
	 mn102->pc = pc & 0xffffff;
}

INLINE mn102_info *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MN10200);

	return (mn102_info *)downcast<legacy_cpu_device *>(device)->token();
}

static void mn102_take_irq(mn102_info *mn102, int level, int group)
{
	if(!(mn102->psw & 0x800))
	{
//      if (group != 8) printf("MN10200: Dropping irq L %d G %d pc=%x, a3=%x\n", level, group, mn102->pc, mn102->a[3]);
		return;
	}

//  if (group != 8) printf("MN10200: Taking irq L %d G %d pc=%x, a3=%x\n", level, group, mn102->pc, mn102->a[3]);

	mn102->a[3] -= 6;
	w24(mn102, mn102->a[3]+2, mn102->pc);
	mn102_write_word(mn102, mn102->a[3], mn102->psw);
	mn102_change_pc(mn102, 0x80008);
	mn102->psw = (mn102->psw & 0xf0ff) | (level << 8);
	mn102->iagr = group << 1;
}

static void refresh_timer(mn102_info *cpustate, int tmr)
{
	// enabled?
	if (cpustate->simple_timer[tmr].mode & 0x80)
	{
		UINT8 source = (cpustate->simple_timer[tmr].mode & 3);

		// source is a prescaler?
		if (source >= 2)
		{
			INT32 rate;

			// is prescaler enabled?
			if (cpustate->prescaler[source-2].mode & 0x80)
			{
				// rate = (sysclock / prescaler) / our count
				rate = cpustate->device->unscaled_clock() / cpustate->prescaler[source-2].cycles;
				rate /= cpustate->simple_timer[tmr].base;

				if (tmr != 8)	// HACK: timer 8 is run at 500 kHz by the Taito program for no obvious reason, which kills performance
					cpustate->timer_timers[tmr]->adjust(attotime::from_hz(rate), tmr);
			}
			else
			{
				logerror("MN10200: timer %d using prescaler %d which isn't enabled!\n", tmr, source-2);
			}
		}
	}
	else	// disabled, so stop it
	{
		cpustate->timer_timers[tmr]->adjust(attotime::never, tmr);
	}
}

INLINE void timer_tick_simple(mn102_info *cpustate, int tmr)
{
	cpustate->simple_timer[tmr].cur--;

	// did we expire?
	if (cpustate->simple_timer[tmr].cur == 0)
	{
		int group, irq_in_grp, level;

		// does timer auto-reload?  apparently.
		cpustate->simple_timer[tmr].cur = cpustate->simple_timer[tmr].base;

		// signal the cascade if we're not timer 9
		if (tmr < (NUM_TIMERS_8BIT-1))
		{
			// is next timer enabled?
			if (cpustate->simple_timer[tmr+1].mode & 0x80)
			{
				// is it's source "cascade"?
				if ((cpustate->simple_timer[tmr+1].mode & 0x3) == 1)
				{
					// recurse!
					timer_tick_simple(cpustate, tmr+1);
				}
			}
		}

		// interrupt from this timer if possible
		group = (tmr / 4);
		irq_in_grp = (tmr % 4);
		level = (cpustate->icrh[group]>>4) & 0x7;

		// indicate interrupt pending
		cpustate->icrl[group] |= (1 << (4 + irq_in_grp));

		// interrupt detect = pending AND enable
		cpustate->icrl[group] |= (cpustate->icrh[group]&0x0f) & (cpustate->icrl[group]>>4);

		// is the result enabled?
		if (cpustate->icrl[group] & (1 << irq_in_grp))
		{
//          printf("Timer %d IRQ! (Group %d in_grp %d ICRH %x ICRL %x\n", tmr, group, irq_in_grp, cpustate->icrh[group], cpustate->icrl[group]);
			// try to take it now
			mn102_take_irq(cpustate, level, group + 1);
		}
	}
}

static TIMER_CALLBACK( simple_timer_cb )
{
	mn102_info *cpustate = (mn102_info *)ptr;
	int tmr = param;

	// handle our expiring and also tick our cascaded children
	cpustate->simple_timer[tmr].cur = 1;
	timer_tick_simple(cpustate, tmr);

	// refresh this timer
	refresh_timer(cpustate, tmr);
}

static CPU_INIT(mn10200)
{
	mn102_info *cpustate = get_safe_token(device);
	int tmr;

	memset(cpustate, 0, sizeof(mn102_info));

	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->io = &device->space(AS_IO);

	device->save_item(NAME(cpustate->pc));
	device->save_item(NAME(cpustate->d));
	device->save_item(NAME(cpustate->a));
	device->save_item(NAME(cpustate->nmicr));
	device->save_item(NAME(cpustate->iagr));
	device->save_item(NAME(cpustate->icrl));
	device->save_item(NAME(cpustate->icrh));
	device->save_item(NAME(cpustate->psw));
	device->save_item(NAME(cpustate->mdr));
//  device->save_item(NAME(cpustate->simple_timer));
//  device->save_item(NAME(cpustate->prescaler));
//  device->save_item(NAME(cpustate->dma));
//  device->save_item(NAME(cpustate->serial));
	device->save_item(NAME(cpustate->ddr));

	for (tmr = 0; tmr < NUM_TIMERS_8BIT; tmr++)
	{
		cpustate->timer_timers[tmr] = device->machine().scheduler().timer_alloc(FUNC(simple_timer_cb), cpustate);
		cpustate->timer_timers[tmr]->adjust(attotime::never, tmr);
	}
}

static CPU_RESET(mn10200)
{
	mn102_info *cpustate = get_safe_token(device);
	int tmr, grp;

	memset(cpustate->d, 0, sizeof(cpustate->d));
	memset(cpustate->a, 0, sizeof(cpustate->a));
	cpustate->pc = 0x80000;
	cpustate->psw = 0;
	cpustate->nmicr = 0;
	memset(cpustate->icrl, 0, sizeof(cpustate->icrl));
	memset(cpustate->icrh, 0, sizeof(cpustate->icrh));

	// reset all timers
	for (tmr = 0; tmr < NUM_TIMERS_8BIT; tmr++)
	{
		cpustate->simple_timer[tmr].mode = 0;
		cpustate->simple_timer[tmr].cur = 0;
		cpustate->simple_timer[tmr].base = 0;
		cpustate->timer_timers[tmr]->adjust(attotime::never, tmr);
	}

	// clear all interrupt groups
	for (grp = 0; grp < NUM_IRQ_GROUPS; grp++)
	{
		cpustate->icrl[grp] = cpustate->icrh[grp] = 0;
	}
}

static CPU_EXIT(mn10200)
{
}

static void unemul(mn102_info *mn102)
{
	fatalerror("MN10200: unknown opcode @ PC=%x\n", mn102->pc);
}

static UINT32 do_add(mn102_info *mn102, UINT32 a, UINT32 b)
{
	UINT32 r24, r16;
	r24 = (a & 0xffffff) + (b & 0xffffff);
	r16 = (a & 0xffff)   + (b & 0xffff);

	mn102->psw &= 0xff00;
	if((~(a^b)) & (a^r24) & 0x00800000)
	mn102->psw |= 0x80;
	if(r24 & 0x01000000)
	mn102->psw |= 0x40;
	if(r24 & 0x00800000)
	mn102->psw |= 0x20;
	if(!(r24 & 0x00ffffff))
	mn102->psw |= 0x10;
	if((~(a^b)) & (a^r16) & 0x00008000)
	mn102->psw |= 0x08;
	if(r16 & 0x00010000)
	mn102->psw |= 0x04;
	if(r16 & 0x00008000)
	mn102->psw |= 0x02;
	if(!(r16 & 0x0000ffff))
	mn102->psw |= 0x01;
	return r24 & 0xffffff;
}

static UINT32 do_addc(mn102_info *mn102, UINT32 a, UINT32 b)
{
	UINT32 r24, r16;
	r24 = (a & 0xffffff) + (b & 0xffffff);
	r16 = (a & 0xffff)   + (b & 0xffff);

	if(mn102->psw & 0x04) {
	r24++;
	r16++;
	}

	mn102->psw &= 0xff00;
	if((~(a^b)) & (a^r24) & 0x00800000)
	mn102->psw |= 0x80;
	if(r24 & 0x01000000)
	mn102->psw |= 0x40;
	if(r24 & 0x00800000)
	mn102->psw |= 0x20;
	if(!(r24 & 0x00ffffff))
	mn102->psw |= 0x10;
	if((~(a^b)) & (a^r16) & 0x00008000)
	mn102->psw |= 0x08;
	if(r16 & 0x00010000)
	mn102->psw |= 0x04;
	if(r16 & 0x00008000)
	mn102->psw |= 0x02;
	if(!(r16 & 0x0000ffff))
	mn102->psw |= 0x01;
	return r24 & 0xffffff;
}

static UINT32 do_sub(mn102_info *mn102, UINT32 a, UINT32 b)
{
	UINT32 r24, r16;
	r24 = (a & 0xffffff) - (b & 0xffffff);
	r16 = (a & 0xffff)   - (b & 0xffff);

	mn102->psw &= 0xff00;
	if((a^b) & (a^r24) & 0x00800000)
		mn102->psw |= 0x80;
	if(r24 & 0x01000000)
		mn102->psw |= 0x40;
	if(r24 & 0x00800000)
		mn102->psw |= 0x20;
	if(!(r24 & 0x00ffffff))
		mn102->psw |= 0x10;
	if((a^b) & (a^r16) & 0x00008000)
		mn102->psw |= 0x08;
	if(r16 & 0x00010000)
		mn102->psw |= 0x04;
	if(r16 & 0x00008000)
		mn102->psw |= 0x02;
	if(!(r16 & 0x0000ffff))
		mn102->psw |= 0x01;
	return r24 & 0xffffff;
}

static UINT32 do_subc(mn102_info *mn102, UINT32 a, UINT32 b)
{
	UINT32 r24, r16;
	r24 = (a & 0xffffff) - (b & 0xffffff);
	r16 = (a & 0xffff)   - (b & 0xffff);

	if(mn102->psw & 0x04) {
	r24--;
	r16--;
	}

	mn102->psw &= 0xff00;
	if(r24 >= 0x00800000 && r24 < 0xff800000)
		mn102->psw |= 0x80;
	if(r24 & 0x01000000)
		mn102->psw |= 0x40;
	if(r24 & 0x00800000)
		mn102->psw |= 0x20;
	if(!(r24 & 0x00ffffff))
		mn102->psw |= 0x10;
	if(r16 >= 0x00008000 && r16 < 0xffff8000)
		mn102->psw |= 0x08;
	if(r16 & 0x00010000)
		mn102->psw |= 0x04;
	if(r16 & 0x00008000)
		mn102->psw |= 0x02;
	if(!(r16 & 0x0000ffff))
		mn102->psw |= 0x01;
	return r24 & 0xffffff;
}

INLINE void test_nz16(mn102_info *mn102, UINT16 v)
{
	mn102->psw &= 0xfff0;
	if(v & 0x8000)
		mn102->psw |= 2;
	if(!v)
		mn102->psw |= 1;
}

INLINE void do_jsr(mn102_info *mn102, UINT32 to, UINT32 ret)
{
	mn102_change_pc(mn102, to & 0xffffff);
	mn102->a[3] -= 4;
	w24(mn102, mn102->a[3], ret);
}

#if 0
  log_event("MN102", "PSW change S=%d irq=%s im=%d %c%c%c%c %c%c%c%c",
	    (psw >> 12) & 3,
	    psw & 0x0800 ? "on" : "off",
	    (psw >> 8) & 7,
	    psw & 0x0080 ? 'V' : '-',
	    psw & 0x0040 ? 'C' : '-',
	    psw & 0x0020 ? 'N' : '-',
	    psw & 0x0010 ? 'Z' : '-',
	    psw & 0x0008 ? 'v' : '-',
	    psw & 0x0004 ? 'c' : '-',
	    psw & 0x0002 ? 'n' : '-',
	    psw & 0x0001 ? 'z' : '-');
#endif

// take an external IRQ
static void mn102_extirq(mn102_info *mn102, int irqnum, int status)
{
	int level = (mn102->icrh[7]>>4)&0x7;

//  printf("mn102_extirq: irq %d status %d G8 ICRL %x\n", irqnum, status, mn102->icrl[7]);

	// if interrupt is enabled, handle it
	if (status)
	{
		// indicate interrupt pending
		mn102->icrl[7] |= (1 << (4 + irqnum));

		// set interrupt detect = pending AND enable
		mn102->icrl[7] |= (mn102->icrh[7]&0x0f) & (mn102->icrl[7]>>4);

		// is the result enabled?
		if (mn102->icrl[7] & (1 << irqnum))
		{
			// try to take it now
			mn102_take_irq(mn102, level, 8);
		}
	}
}

static CPU_EXECUTE(mn10200)
{
	mn102_info *mn102 = get_safe_token(device);

	while(mn102->cycles > 0)
	{
		UINT8 opcode;

		debugger_instruction_hook(device, mn102->pc);

		opcode = mn102_read_byte(mn102, mn102->pc);
		switch(opcode) {
		// mov dm, (an)
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			mn102->cycles -= 1;
			mn102_write_word(mn102, mn102->a[(opcode>>2)&3], (UINT16)mn102->d[opcode & 3]);
			mn102->pc += 1;
			break;

		// movb dm, (an)
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			mn102->cycles -= 1;
			mn102_write_byte(mn102, mn102->a[(opcode>>2)&3], (UINT8)mn102->d[opcode & 3]);
			mn102->pc += 1;
			break;

		// mov (an), dm
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = (INT16)mn102_read_word(mn102, mn102->a[(opcode>>2)&3]);
			mn102->pc += 1;
			break;

		// movbu (an), dm
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = mn102_read_byte(mn102, mn102->a[(opcode>>2)&3]);
			mn102->pc += 1;
			break;

		// mov dm, (d8, an)
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			mn102->cycles -= 1;
			mn102_write_word(mn102, (mn102->a[(opcode>>2)&3]+(INT8)mn102_read_byte(mn102, mn102->pc+1)) & 0xffffff, (UINT16)mn102->d[opcode & 3]);
			mn102->pc += 2;
			break;

		// mov am, (d8, an)
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			mn102->cycles -= 2;
			w24(mn102, (mn102->a[(opcode>>2)&3]+(INT8)mn102_read_byte(mn102, mn102->pc+1)) & 0xffffff, mn102->a[opcode & 3]);
			mn102->pc += 2;
			break;

		// mov (d8, an), dm
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = (INT16)mn102_read_word(mn102, (mn102->a[(opcode>>2) & 3] + (INT8)mn102_read_byte(mn102, mn102->pc+1)) & 0xffffff);
			mn102->pc += 2;
			break;

		// mov (d8, an), am
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			mn102->cycles -= 2;
			mn102->a[opcode & 3] = r24u(mn102, (mn102->a[(opcode>>2) & 3] + (INT8)mn102_read_byte(mn102, mn102->pc+1)) & 0xffffff);
			mn102->pc += 2;
			break;

		// mov dm, dn
		case 0x81: case 0x82: case 0x83: case 0x84: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8b: case 0x8c: case 0x8d: case 0x8e:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = mn102->d[(opcode>>2) & 3];
			mn102->pc += 1;
			break;

		// mov imm8, dn
		case 0x80: case 0x85: case 0x8a: case 0x8f:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = (INT8)mn102_read_byte(mn102, mn102->pc+1);
			mn102->pc += 2;
			break;

		// add dn, dm
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = do_add(mn102, mn102->d[opcode & 3], mn102->d[(opcode >> 2) & 3]);
			mn102->pc += 1;
			break;

		// sub dn, dm
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = do_sub(mn102, mn102->d[opcode & 3], mn102->d[(opcode >> 2) & 3]);
			mn102->pc += 1;
			break;

		// extx dn
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = (INT16)mn102->d[opcode & 3] & 0xffffff;
			mn102->pc += 1;
			break;

		// extxu dn
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = (UINT16)mn102->d[opcode & 3];
			mn102->pc += 1;
			break;

		// extxb dn
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = (INT8)mn102->d[opcode & 3] & 0xffffff;
			mn102->pc += 1;
			break;

		// extxbu dn
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = (UINT8)mn102->d[opcode & 3];
			mn102->pc += 1;
			break;

		// mov dn, (imm16)
		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
			mn102->cycles -= 2;
			mn102_write_word(mn102, mn102_read_word(mn102, mn102->pc+1), (UINT16)mn102->d[opcode & 3]);
			mn102->pc += 3;
			break;

		// movb dn, (imm16)
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			mn102->cycles -= 1;
			mn102_write_byte(mn102, mn102_read_word(mn102, mn102->pc+1), (UINT8)mn102->d[opcode & 3]);
			mn102->pc += 3;
			break;

		// mov (abs16), dn
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = (INT16)mn102_read_word(mn102, mn102_read_word(mn102, mn102->pc+1));
			mn102->pc += 3;
			break;

		// movbu (abs16), dn
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
			mn102->cycles -= 2;
			mn102->d[opcode & 3] = mn102_read_byte(mn102, mn102_read_word(mn102, mn102->pc+1));
			mn102->pc += 3;
			break;

		// add imm8, an
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
			mn102->cycles -= 1;
			mn102->a[opcode & 3] = do_add(mn102, mn102->a[opcode & 3], (INT8)mn102_read_byte(mn102, mn102->pc+1));
			mn102->pc += 2;
			break;

		// add imm8, dn
		case 0xd4: case 0xd5: case 0xd6: case 0xd7:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = do_add(mn102, mn102->d[opcode & 3], (INT8)mn102_read_byte(mn102, mn102->pc+1));
			mn102->pc += 2;
			break;

		// cmp imm8, dn
		case 0xd8: case 0xd9: case 0xda: case 0xdb:
			mn102->cycles -= 1;
			do_sub(mn102, mn102->d[opcode & 3], (INT8)mn102_read_byte(mn102, mn102->pc+1));
			mn102->pc += 2;
			break;

		// mov imm16, an
		case 0xdc: case 0xdd: case 0xde: case 0xdf:
			mn102->cycles -= 1;
			mn102->a[opcode & 3] = mn102_read_word(mn102, mn102->pc+1);
			mn102->pc += 3;
			break;

		// blt label8
		case 0xe0:
			if(((mn102->psw & 0x0a) == 2) || ((mn102->psw & 0x0a) == 8))
			{
				mn102->cycles -= 2;
				mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			}
			else
			{
				mn102->cycles -= 1;
				mn102->pc += 2;
			}
			break;

		// bgt label8
		case 0xe1:
			if(((mn102->psw & 0x0b) == 0) || ((mn102->psw & 0x0b) == 0xa))
			{
				mn102->cycles -= 2;
				mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			}
			else
			{
				mn102->cycles -= 1;
				mn102->pc += 2;
			}
			break;

		// bge label8
		case 0xe2:
			if(((mn102->psw & 0x0a) == 0) || ((mn102->psw & 0x0a) == 0xa))
			{
				mn102->cycles -= 2;
				mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			}
			else
			{
				mn102->cycles -= 1;
				mn102->pc += 2;
			}
			break;

		// ble label8
		case 0xe3:
			if((mn102->psw & 0x01) || ((mn102->psw & 0x0a) == 2) || ((mn102->psw & 0x0a) == 8))
			{
				mn102->cycles -= 2;
				mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			}
			else
			{
			mn102->cycles -= 1;
			mn102->pc += 2;
			}
			break;

		// bcs label8
		case 0xe4:
			if(mn102->psw & 0x04)
			{
				mn102->cycles -= 2;
				mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			}
			else
			{
				mn102->cycles -= 1;
				mn102->pc += 2;
			}
			break;

		// bhi label8
		case 0xe5:
			if(!(mn102->psw & 0x05))
			{
				mn102->cycles -= 2;
				mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			}
			else
			{
				mn102->cycles -= 1;
				mn102->pc += 2;
			}
			break;

		// bcc label8
		case 0xe6:
			if(!(mn102->psw & 0x04))
			{
				mn102->cycles -= 2;
				mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			}
			else
			{
				mn102->cycles -= 1;
				mn102->pc += 2;
			}
			break;

		// bls label8
		case 0xe7:
			if(mn102->psw & 0x05)
			{
				mn102->cycles -= 2;
				mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			}
			else
			{
				mn102->cycles -= 1;
				mn102->pc += 2;
			}
			break;

		// beq label8
		case 0xe8:
			if(mn102->psw & 0x01)
			{
				mn102->cycles -= 2;
				mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			}
			else
			{
				mn102->cycles -= 1;
				mn102->pc += 2;
			}
			break;

		// bne label8
		case 0xe9:
			if(!(mn102->psw & 0x01))
			{
				mn102->cycles -= 2;
				mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			}
			else
			{
				mn102->cycles -= 1;
				mn102->pc += 2;
			}
			break;

		// bra label8
		case 0xea:
			mn102->cycles -= 2;
			mn102_change_pc(mn102, mn102->pc+2+(INT8)mn102_read_byte(mn102, mn102->pc+1));
			break;

		// rti
		case 0xeb:
			mn102->cycles -= 6;
			mn102->psw = mn102_read_word(mn102, mn102->a[3]);
			mn102_change_pc(mn102, r24u(mn102, mn102->a[3]+2));
			mn102->a[3] += 6;
			break;

		// cmp imm16, an
		case 0xec: case 0xed: case 0xee: case 0xef:
			mn102->cycles -= 1;
			do_sub(mn102, mn102->a[opcode & 3], mn102_read_word(mn102, mn102->pc+1));
			mn102->pc += 3;
			break;

		case 0xf0:
			opcode = mn102_read_byte(mn102, mn102->pc+1);
			switch(opcode) {
				// jmp (an)
				case 0x00: case 0x04: case 0x08: case 0x0c:
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->a[(opcode>>2) & 3]);
				break;

				// jsr (an)
				case 0x01: case 0x05: case 0x09: case 0x0d:
				mn102->cycles -= 5;
				do_jsr(mn102, mn102->a[(opcode>>2) & 3], mn102->pc+2);
				break;

				// bset dm, (an)
				case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
				case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: {
				UINT8 v;
				mn102->cycles -= 5;
				v = mn102_read_byte(mn102, mn102->a[(opcode>>2) & 3]);
				test_nz16(mn102, v & mn102->d[opcode & 3]);
				mn102_write_byte(mn102, mn102->a[(opcode>>2) & 3], v | mn102->d[opcode & 3]);
				mn102->pc += 2;
				break;
				}

				// bclr dm, (an)
				case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
				case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f: {
				UINT8 v;
				mn102->cycles -= 5;
				v = mn102_read_byte(mn102, mn102->a[(opcode>>2) & 3]);
				test_nz16(mn102, v & mn102->d[opcode & 3]);
				mn102_write_byte(mn102, mn102->a[(opcode>>2) & 3], v & ~mn102->d[opcode & 3]);
				mn102->pc += 2;
				break;
				}

				// movb (di, an), dm
				case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
				case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
				case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
				case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
				case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
				case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = (INT8)mn102_read_byte(mn102, (mn102->a[(opcode>>2) & 3] + mn102->d[(opcode>>4) & 3]) & 0xffffff);
				mn102->pc += 2;
				break;

				// movbu (di, an), dm
				case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
				case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
				case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
				case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
				case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
				case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = mn102_read_byte(mn102, (mn102->a[(opcode>>2) & 3] + mn102->d[(opcode>>4) & 3]) & 0xffffff);
				mn102->pc += 2;
				break;

				// movb dm, (di, an)
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
				case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
				mn102->cycles -= 2;
				mn102_write_byte(mn102, (mn102->a[(opcode>>2) & 3] + mn102->d[(opcode>>4) & 3]) & 0xffffff, mn102->d[opcode & 3]);
				mn102->pc += 2;
				break;

				default:
				unemul(mn102);
				break;
			}
			break;

		case 0xf1:
			opcode = mn102_read_byte(mn102, mn102->pc+1);
			switch(opcode>>6) {

				// mov (di, an), am
				case 0:
				mn102->cycles -= 3;
				mn102->a[opcode & 3] = r24u(mn102, (mn102->a[(opcode>>2) & 3] + mn102->d[(opcode>>4) & 3]) & 0xffffff);
				mn102->pc += 2;
				break;

				// mov (di, an), dm
				case 1:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = mn102_read_word(mn102, (mn102->a[(opcode>>2) & 3] + mn102->d[(opcode>>4) & 3]) & 0xffffff);
				mn102->pc += 2;
				break;

				// mov am, (di, an)
				case 2:
				mn102->cycles -= 3;
				w24(mn102, (mn102->a[(opcode>>2) & 3] + mn102->d[(opcode>>4) & 3]) & 0xffffff, mn102->a[opcode & 3]);
				mn102->pc += 2;
				break;

				// mov dm, (di, an)
				case 3:
				mn102->cycles -= 2;
				mn102_write_word(mn102, (mn102->a[(opcode>>2) & 3] + mn102->d[(opcode>>4) & 3]) & 0xffffff, mn102->d[opcode & 3]);
				mn102->pc += 2;
				break;

				default:
				unemul(mn102);
				break;
				}
			break;

		case 0xf2:
			opcode = mn102_read_byte(mn102, mn102->pc+1);
			switch(opcode>>4) {
				// add dm, an
				case 0x0:
				mn102->cycles -= 2;
				mn102->a[opcode & 3] = do_add(mn102, mn102->a[opcode & 3], mn102->d[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// sub dm, an
				case 0x1:
				mn102->cycles -= 2;
				mn102->a[opcode & 3] = do_sub(mn102, mn102->a[opcode & 3], mn102->d[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// cmp dm, an
				case 0x2:
				mn102->cycles -= 2;
				do_sub(mn102, mn102->a[opcode & 3], mn102->d[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// mov am, dn
				case 0x3:
				mn102->cycles -= 2;
				mn102->a[opcode & 3] = mn102->d[(opcode>>2) & 3];
				mn102->pc += 2;
				break;

				// add am, an
				case 0x4:
				mn102->cycles -= 2;
				mn102->a[opcode & 3] = do_add(mn102, mn102->a[opcode & 3], mn102->a[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// sub am, an
				case 0x5:
				mn102->cycles -= 2;
				mn102->a[opcode & 3] = do_sub(mn102, mn102->a[opcode & 3], mn102->a[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// cmp am, an
				case 0x6:
				mn102->cycles -= 2;
				do_sub(mn102, mn102->a[opcode & 3], mn102->a[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// mov am, an
				case 0x7:
				mn102->cycles -= 2;
				mn102->a[opcode & 3] = mn102->a[(opcode>>2) & 3];
				mn102->pc += 2;
				break;

				// addc dm, dn
				case 0x8:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = do_addc(mn102, mn102->d[opcode & 3], mn102->d[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// subc dm, dn
				case 0x9:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = do_subc(mn102, mn102->d[opcode & 3], mn102->d[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// add am, dn
				case 0xc:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = do_add(mn102, mn102->d[opcode & 3], mn102->a[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// sub am, dn
				case 0xd:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = do_sub(mn102, mn102->d[opcode & 3], mn102->a[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// cmp am, dn
				case 0xe:
				mn102->cycles -= 2;
				do_sub(mn102, mn102->d[opcode & 3], mn102->a[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// mov an, dm
				case 0xf:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = mn102->a[(opcode>>2) & 3];
				mn102->pc += 2;
				break;

				default:
				unemul(mn102);
				break;
				}
			break;

		case 0xf3:
			opcode = mn102_read_byte(mn102, mn102->pc+1);
			switch(opcode) {
				// and dm, dn
				case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
				case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] &= 0xff0000|mn102->d[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// or dm, dn
				case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
				case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] |= 0x00ffff&mn102->d[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// xor dm, dn
				case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
				case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] ^= 0x00ffff&mn102->d[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// rol dn
				case 0x30: case 0x31: case 0x32: case 0x33: {
				UINT32 d = mn102->d[opcode & 3];
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] = (d & 0xff0000) | ((d << 1) & 0x00fffe) | ((mn102->psw & 0x04) ? 1 : 0));
				if(d & 0x8000)
				mn102->psw |= 0x04;
				mn102->pc += 2;
				break;
				}

				// ror dn
				case 0x34: case 0x35: case 0x36: case 0x37: {
				UINT32 d = mn102->d[opcode & 3];
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] = (d & 0xff0000) | ((d >> 1) & 0x007fff) | ((mn102->psw & 0x04) ? 0x8000 : 0));
				if(d & 1)
				mn102->psw |= 0x04;
				mn102->pc += 2;
				break;
				}

				// asr dn
				case 0x38: case 0x39: case 0x3a: case 0x3b: {
				UINT32 d = mn102->d[opcode & 3];
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] = (d & 0xff8000) | ((d >> 1) & 0x007fff));
				if(d & 1)
				mn102->psw |= 0x04;
				mn102->pc += 2;
				break;
				}

				// lsr dn
				case 0x3c: case 0x3d: case 0x3e: case 0x3f: {
				UINT32 d = mn102->d[opcode & 3];
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] = (d & 0xff0000) | ((d >> 1) & 0x007fff));
				if(d & 1)
				mn102->psw |= 0x04;
				mn102->pc += 2;
				break;
				}

				// mul dn, dm
				case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
				case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f: {
				UINT32 res;
				mn102->cycles -= 12;
				res = ((INT16)mn102->d[opcode & 3])*((INT16)mn102->d[(opcode>>2) & 3]);
				mn102->d[opcode & 3] = res & 0xffffff;
				mn102->psw &= 0xff00;
				if(res & 0x80000000)
				mn102->psw |= 2;
				if(!res)
				mn102->psw |= 1;
				mn102->mdr = res >> 16;
				mn102->pc += 2;
				break;
				}

				// mulu dn, dm
				case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
				case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f: {
				UINT32 res;
				mn102->cycles -= 12;
				res = ((UINT16)mn102->d[opcode & 3])*((UINT16)mn102->d[(opcode>>2) & 3]);
				mn102->d[opcode & 3] = res & 0xffffff;
				mn102->psw &= 0xff00;
				if(res & 0x80000000)
				mn102->psw |= 2;
				if(!res)
				mn102->psw |= 1;
				mn102->mdr = res >> 16;
				mn102->pc += 2;
				break;
				}

				// divu dn, dm
				case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
				case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f: {
				UINT32 n, d, q, r;
				mn102->cycles -= 13;
				mn102->pc += 2;
				mn102->psw &= 0xff00;

				n = (mn102->mdr<<16)|(UINT16)mn102->d[opcode & 3];
				d = (UINT16)mn102->d[(opcode>>2) & 3];
				if(!d) {
				mn102->psw |= 8;
				break;
				}
				q = n/d;
				r = n%d;
				if(q >= 0x10000) {
				mn102->psw |= 8;
				break;
				}
				mn102->d[opcode & 3] = q;
				mn102->mdr = r;
				if(!q)
				mn102->psw |= 0x11;
				if(q & 0x8000)
				mn102->psw |= 2;
				break;
				}

				// cmp dm, dn
				case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
				case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				mn102->cycles -= 2;
				do_sub(mn102, mn102->d[opcode & 3], mn102->d[(opcode>>2) & 3]);
				mn102->pc += 2;
				break;

				// mov mdr, dn
				case 0xc0: case 0xc4: case 0xc8: case 0xcc:
				mn102->cycles -= 2;
				mn102->mdr = mn102->d[(opcode>>2) & 3];
				mn102->pc += 2;
				break;

				// ext dn
				case 0xc1: case 0xc5: case 0xc9: case 0xcd:
				mn102->cycles -= 3;
				mn102->mdr = mn102->d[(opcode>>2) & 3] & 0x8000 ? 0xffff : 0x0000;
				mn102->pc += 2;
				break;

				// mov dn, psw
				case 0xd0: case 0xd4: case 0xd8: case 0xdc:
				mn102->cycles -= 3;
				mn102->psw = mn102->d[(opcode>>2) & 3];
				mn102->pc += 2;
				break;

				// mov dn, mdr
				case 0xe0: case 0xe1: case 0xe2: case 0xe3:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = mn102->mdr;
				mn102->pc += 2;
				break;

				// not dn
				case 0xe4: case 0xe5: case 0xe6: case 0xe7:
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] ^= 0x00ffff);
				mn102->pc += 2;
				break;

				// mov psw, dn
				case 0xf0: case 0xf1: case 0xf2: case 0xf3:
				mn102->cycles -= 3;
				mn102->d[(opcode>>2) & 3] = mn102->psw;
				mn102->pc += 2;
				break;

				default:
				unemul(mn102);
				break;
				}
			break;

		case 0xf4:
			opcode = mn102_read_byte(mn102, mn102->pc+1);
			switch(opcode) {
				// mov dm, (abs24, an)
				case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
				case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				mn102->cycles -= 3;
				mn102_write_word(mn102, (r24u(mn102, mn102->pc+2) + mn102->a[(opcode>>2) & 3]) & 0xffffff, mn102->d[opcode & 3]);
				mn102->pc += 5;
				break;

				// mov am, (abs24, an)
				case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
				case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				mn102->cycles -= 4;
				w24(mn102, (r24u(mn102, mn102->pc+2) + mn102->a[(opcode>>2) & 3]) & 0xffffff, mn102->a[opcode & 3]);
				mn102->pc += 5;
				break;

				// movb dm, (abs24, an)
				case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
				case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				mn102->cycles -= 3;
				mn102_write_byte(mn102, (r24u(mn102, mn102->pc+2) + mn102->a[(opcode>>2) & 3]) & 0xffffff, mn102->d[opcode & 3]);
				mn102->pc += 5;
				break;

				// movx dm, (abs24, an)
				case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
				case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				mn102->cycles -= 4;
				w24(mn102, (r24u(mn102, mn102->pc+2) + mn102->a[(opcode>>2) & 3]) & 0xffffff, mn102->d[opcode & 3]);
				mn102->pc += 5;
				break;

				// mov dn, (abs24)
				case 0x40: case 0x41: case 0x42: case 0x43:
				mn102->cycles -= 3;
				mn102_write_word(mn102, r24u(mn102, mn102->pc+2), mn102->d[opcode & 3]);
				mn102->pc += 5;
				break;

				// movb dn, (abs24)
				case 0x44: case 0x45: case 0x46: case 0x47:
				mn102->cycles -= 3;
				mn102_write_byte(mn102, r24u(mn102, mn102->pc+2), mn102->d[opcode & 3]);
				mn102->pc += 5;
				break;

				// mov an, (abs24)
				case 0x50: case 0x51: case 0x52: case 0x53:
				mn102->cycles -= 4;
				w24(mn102, r24u(mn102, mn102->pc+2), mn102->a[opcode & 3]);
				mn102->pc += 5;
				break;

				// add abs24, dn
				case 0x60: case 0x61: case 0x62: case 0x63:
				mn102->cycles -= 3;
				mn102->d[opcode & 3] = do_add(mn102, mn102->d[opcode & 3], r24u(mn102, mn102->pc+2));
				mn102->pc += 5;
				break;

				// add abs24, an
				case 0x64: case 0x65: case 0x66: case 0x67:
				mn102->cycles -= 3;
				mn102->a[opcode & 3] = do_add(mn102, mn102->a[opcode & 3], r24u(mn102, mn102->pc+2));
				mn102->pc += 5;
				break;

				// sub abs24, dn
				case 0x68: case 0x69: case 0x6a: case 0x6b:
				mn102->cycles -= 3;
				mn102->d[opcode & 3] = do_sub(mn102, mn102->d[opcode & 3], r24u(mn102, mn102->pc+2));
				mn102->pc += 5;
				break;

				// sub abs24, an
				case 0x6c: case 0x6d: case 0x6e: case 0x6f:
				mn102->cycles -= 3;
				mn102->a[opcode & 3] = do_sub(mn102, mn102->a[opcode & 3], r24u(mn102, mn102->pc+2));
				mn102->pc += 5;
				break;

				// mov imm24, dn
				case 0x70: case 0x71: case 0x72: case 0x73:
				mn102->cycles -= 3;
				mn102->d[opcode & 3] = r24u(mn102, mn102->pc+2);
				mn102->pc += 5;
				break;

				// mov imm24, an
				case 0x74: case 0x75: case 0x76: case 0x77:
				mn102->cycles -= 3;
				mn102->a[opcode & 3] = r24u(mn102, mn102->pc+2);
				mn102->pc += 5;
				break;

				// cmp abs24, dn
				case 0x78: case 0x79: case 0x7a: case 0x7b:
				mn102->cycles -= 3;
				do_sub(mn102, mn102->d[opcode & 3], r24u(mn102, mn102->pc+2));
				mn102->pc += 5;
				break;

				// cmp abs24, an
				case 0x7c: case 0x7d: case 0x7e: case 0x7f:
				mn102->cycles -= 3;
				do_sub(mn102, mn102->a[opcode & 3], r24u(mn102, mn102->pc+2));
				mn102->pc += 5;
				break;

				// mov (abs24, an), dm
				case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
				case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				mn102->cycles -= 3;
				mn102->d[opcode & 3] = (INT16)mn102_read_word(mn102, (mn102->a[(opcode>>2) & 3] + r24u(mn102, mn102->pc+2)) & 0xffffff);
				mn102->pc += 5;
				break;

				// movbu (abs24, an), dm
				case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
				case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				mn102->cycles -= 3;
				mn102->d[opcode & 3] = mn102_read_byte(mn102, (mn102->a[(opcode>>2) & 3] + r24u(mn102, mn102->pc+2)) & 0xffffff);
				mn102->pc += 5;
				break;

				// movb (abs24, an), dm
				case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
				case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
				mn102->cycles -= 3;
				mn102->d[opcode & 3] = (INT8)mn102_read_byte(mn102, (mn102->a[(opcode>>2) & 3] + r24u(mn102, mn102->pc+2)) & 0xffffff);
				mn102->pc += 5;
				break;

				// movx (abs24, an), dm
				case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
				case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				mn102->cycles -= 4;
				mn102->d[opcode & 3] = r24u(mn102, (mn102->a[(opcode>>2) & 3] + r24u(mn102, mn102->pc+2)) & 0xffffff);
				mn102->pc += 5;
				break;

				// mov (abs24), dn
				case 0xc0: case 0xc1: case 0xc2: case 0xc3:
				mn102->cycles -= 3;
				mn102->d[opcode & 3] = (INT16)mn102_read_word(mn102, r24u(mn102, mn102->pc+2));
				mn102->pc += 5;
				break;

				// movb (abs24), dn
				case 0xc4: case 0xc5: case 0xc6: case 0xc7:
				mn102->cycles -= 3;
				mn102->d[opcode & 3] = (INT8)mn102_read_byte(mn102, r24u(mn102, mn102->pc+2));
				mn102->pc += 5;
				break;

				// movbu (abs24), dn
				case 0xc8: case 0xc9: case 0xca: case 0xcb:
				mn102->cycles -= 3;
				mn102->d[opcode & 3] = mn102_read_byte(mn102, r24u(mn102, mn102->pc+2));
				mn102->pc += 5;
				break;

				// mov (abs24), an
				case 0xd0: case 0xd1: case 0xd2: case 0xd3:
				mn102->cycles -= 4;
				mn102->a[opcode & 3] = r24u(mn102, r24u(mn102, mn102->pc+2));
				mn102->pc += 5;
				break;

				// jmp imm24
				case 0xe0:
				mn102->cycles -= 4;
				mn102_change_pc(mn102, mn102->pc+5+r24u(mn102, mn102->pc+2));
				break;

				// jsr label24
				case 0xe1:
				mn102->cycles -= 5;
				do_jsr(mn102, mn102->pc+5+r24u(mn102, mn102->pc+2), mn102->pc+5);
				break;


				// mov (abs24, an), am
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
				mn102->cycles -= 4;
				mn102->a[opcode & 3] = r24u(mn102, (mn102->a[(opcode>>2) & 3] + r24u(mn102, mn102->pc+2)) & 0xffffff);
				mn102->pc += 5;
				break;

				default:
				unemul(mn102);
				break;
				}
			break;

		case 0xf5:
			opcode = mn102_read_byte(mn102, mn102->pc+1);
			switch(opcode) {

				// and imm8, dn
				case 0x00: case 0x01: case 0x02: case 0x03:
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] &= 0xff0000|mn102_read_byte(mn102, mn102->pc+2));
				mn102->pc += 3;
				break;

				// btst imm8, dn
				case 0x04: case 0x05: case 0x06: case 0x07:
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] & mn102_read_byte(mn102, mn102->pc+2));
				mn102->pc += 3;
				break;

				// or imm8, dn
				case 0x08: case 0x09: case 0x0a: case 0x0b:
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] |= mn102_read_byte(mn102, mn102->pc+2));
				mn102->pc += 3;
				break;

				// addnf imm8, an
				case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				mn102->cycles -= 2;
				mn102->a[opcode & 3] = mn102->a[opcode & 3] +(INT8)mn102_read_byte(mn102, mn102->pc+2);
				mn102->pc += 3;
				break;

				// movb dm, (d8, an)
				case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
				case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				mn102->cycles -= 2;
				mn102_write_byte(mn102, (mn102->a[(opcode>>2) & 3]+(INT8)mn102_read_byte(mn102, mn102->pc+2)) & 0xffffff, mn102->d[opcode & 3]);
				mn102->pc += 3;
				break;

				// movb (d8, an), dm
				case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
				case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = (INT8)mn102_read_byte(mn102, (mn102->a[(opcode>>2) & 3]+(INT8)mn102_read_byte(mn102, mn102->pc+2)) & 0xffffff);
				mn102->pc += 3;
				break;

				// movbu (d8, an), dm
				case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
				case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = mn102_read_byte(mn102, (mn102->a[(opcode>>2) & 3]+(INT8)mn102_read_byte(mn102, mn102->pc+2)) & 0xffffff);
				mn102->pc += 3;
				break;

				// movx dm, (d8, an)
				case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
				case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				mn102->cycles -= 3;
				w24(mn102, (mn102->a[(opcode>>2) & 3]+(INT8)mn102_read_byte(mn102, mn102->pc+2)) & 0xffffff, mn102->d[opcode & 3]);
				mn102->pc += 3;
				break;

				// movx (d8, an), dm
				case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
				case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
				mn102->cycles -= 3;
				mn102->d[opcode & 3] = r24u(mn102, (mn102->a[(opcode>>2) & 3]+(INT8)mn102_read_byte(mn102, mn102->pc+2)) & 0xffffff);
				mn102->pc += 3;
				break;

				// bltx label8
				case 0xe0:
				if(((mn102->psw & 0xa0) == 0x20) || ((mn102->psw & 0xa0) == 0x80)) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;

				// bgtx label8
				case 0xe1:
				if(((mn102->psw & 0xb0) == 0) || ((mn102->psw & 0xb0) == 0xa0)) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;

				// bgex label8
				case 0xe2:
				if(((mn102->psw & 0xa0) == 0) || ((mn102->psw & 0xa0) == 0xa0)) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;

				// blex label8
				case 0xe3:
				if((mn102->psw & 0x10) || ((mn102->psw & 0xa0) == 0x20) || ((mn102->psw & 0xa0) == 0x80)) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;

				// bcsx label8
				case 0xe4:
				if(mn102->psw & 0x40) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;

				// bhix label8
				case 0xe5:
				if(!(mn102->psw & 0x50)) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;

				// bccx label8
				case 0xe6:
				if(!(mn102->psw & 0x40)) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;

				// blsx label8
				case 0xe7:
				if(mn102->psw & 0x50) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;

				// beqx label8
				case 0xe8:
				if(mn102->psw & 0x10) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;

				// bnex label8
				case 0xe9:
				if(!(mn102->psw & 0x10)) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;


				// bnc label8
				case 0xfe:
				if(!(mn102->psw & 0x02)) {
				mn102->cycles -= 3;
				mn102_change_pc(mn102, mn102->pc+3+(INT8)mn102_read_byte(mn102, mn102->pc+2));
				} else {
				mn102->cycles -= 2;
				mn102->pc += 3;
				}
				break;

				default:
				unemul(mn102);
				break;
				}
			break;

		case 0xf7:
			opcode = mn102_read_byte(mn102, mn102->pc+1);
			switch(opcode) {
				// and imm16, dn
				case 0x00: case 0x01: case 0x02: case 0x03:
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] &= 0xff0000|mn102_read_word(mn102, mn102->pc+2));
				mn102->pc += 4;
				break;

				// btst imm16, dn
				case 0x04: case 0x05: case 0x06: case 0x07:
				mn102->cycles -= 2;
				test_nz16(mn102, mn102->d[opcode & 3] & mn102_read_word(mn102, mn102->pc+2));
				mn102->pc += 4;
				break;

				// add imm16, an
				case 0x08: case 0x09: case 0x0a: case 0x0b:
				mn102->cycles -= 2;
				mn102->a[opcode & 3] = do_add(mn102, mn102->a[opcode & 3], (INT16)mn102_read_word(mn102, mn102->pc+2));
				mn102->pc += 4;
				break;

				// sub imm16, an
				case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				mn102->cycles -= 2;
				mn102->a[opcode & 3] = do_sub(mn102, mn102->a[opcode & 3], (INT16)mn102_read_word(mn102, mn102->pc+2));
				mn102->pc += 4;
				break;

				// and imm16, psw
				case 0x10:
				mn102->cycles -= 3;
				mn102->psw &= mn102_read_word(mn102, mn102->pc+2);
				mn102->pc += 4;
				break;

				// or imm16, psw
				case 0x14:
				mn102->cycles -= 3;
				mn102->psw |= mn102_read_word(mn102, mn102->pc+2);
				mn102->pc += 4;
				break;

				// add imm16, dn
				case 0x18: case 0x19: case 0x1a: case 0x1b:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = do_add(mn102, mn102->d[opcode & 3], (INT16)mn102_read_word(mn102, mn102->pc+2));
				mn102->pc += 4;
				break;

				// sub imm16, dn
				case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = do_sub(mn102, mn102->d[opcode & 3], (INT16)mn102_read_word(mn102, mn102->pc+2));
				mn102->pc += 4;
				break;

				// or imm16, dn
				case 0x40: case 0x41: case 0x42: case 0x43:
				mn102->cycles -= 3;
				test_nz16(mn102, mn102->d[opcode & 3] |= mn102_read_word(mn102, mn102->pc+2));
				mn102->pc += 4;
				break;

				// cmp imm16, dn
				case 0x48: case 0x49: case 0x4a: case 0x4b:
				mn102->cycles -= 2;
				do_sub(mn102, mn102->d[opcode & 3], (INT16)mn102_read_word(mn102, mn102->pc+2));
				mn102->pc += 4;
				break;

				// xor imm16, dn
				case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				mn102->cycles -= 3;
				test_nz16(mn102, mn102->d[opcode & 3] ^= mn102_read_word(mn102, mn102->pc+2));
				mn102->pc += 4;
				break;

				// mov dm, (imm16, an)
				case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
				case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				mn102->cycles -= 2;
				mn102_write_word(mn102, (mn102->a[(opcode>>2)&3]+(INT16)mn102_read_word(mn102, mn102->pc+2)) & 0xffffff, (UINT16)mn102->d[opcode & 3]);
				mn102->pc += 4;
				break;

				// mov (imm16, an), dm
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
				mn102->cycles -= 2;
				mn102->d[opcode & 3] = (INT16)mn102_read_word(mn102, (mn102->a[(opcode>>2) & 3] + (INT16)mn102_read_byte(mn102, mn102->pc+2)) & 0xffffff) & 0xffffff;
				mn102->pc += 4;
				break;


				default:
				unemul(mn102);
				break;
				}
			break;

		// mov imm16, dn
		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
			mn102->cycles -= 1;
			mn102->d[opcode & 3] = (INT16)mn102_read_word(mn102, mn102->pc+1);
			mn102->pc += 3;
			break;

		// jmp label16
		case 0xfc:
			mn102->cycles -= 2;
			mn102_change_pc(mn102, mn102->pc+3+(INT16)mn102_read_word(mn102, mn102->pc+1));
			break;

		// jsr label16
		case 0xfd:
			mn102->cycles -= 4;
			do_jsr(mn102, mn102->pc+3+(INT16)mn102_read_word(mn102, mn102->pc+1), mn102->pc+3);
			break;

		// rts
		case 0xfe:
			mn102->cycles -= 5;
			mn102_change_pc(mn102, r24u(mn102, mn102->a[3]));
			mn102->a[3] += 4;
			break;

		default:
			unemul(mn102);
			break;
		}
	}
}

static const char *const inames[10][4] = {
  { "timer0", "timer1", "timer2", "timer3" },
  { "timer4", "timer5", "timer6", "timer7" },
  { "timer8", "timer9", "timer12a", "timer12b" },
  { "timer10u", "timer10a", "timer10b", "?" },
  { "timer11u", "timer11a", "timer11b", "?" },
  { "dma0", "dma1", "dma2", "dma3" },
  { "dma4", "dma5", "dma6", "dma7" },
  { "X0", "X1", "X2", "X3" },
  { "ser0tx", "ser0rx", "ser1tx", "ser1rx" },
  { "key", "a/d", "?", "?" }
};

static void mn10200_w(mn102_info *mn102, UINT32 adr, UINT32 data, int type)
{
  if(type == MEM_WORD) {
    mn10200_w(mn102, adr, data & 0xff, MEM_BYTE);
    mn10200_w(mn102, adr+1, (data>>8) & 0xff, MEM_BYTE);
    return;
  }

  switch(adr) {
  case 0x000:
    if(data & 12) {
      log_event("CPU", "Stop request");
	}
    break;
  case 0x001:
    log_event("WATCHDOG", "Write %d", data>>7);
    break;

  case 0x002: case 0x003: // Memory control
    break;

  case 0x030: case 0x031: // Memory mode reg 0
  case 0x032: case 0x033: // Memory mode reg 1
  case 0x034: case 0x035: // Memory mode reg 2
  case 0x036: case 0x037: // Memory mode reg 3
    break;

    // Non-Maskable irqs
  case 0x040:
    mn102->nmicr = data & 6;
    break;
  case 0x041:
    break;

    // Maskable irq control
  case 0x042: case 0x044: case 0x046: case 0x048: case 0x04a:
  case 0x04c: case 0x04e: case 0x050: case 0x052: case 0x054:
    {
    	// note: writes here ack interrupts
	mn102->icrl[((adr & 0x3f)>>1)-1] = data;
    }
    break;

  case 0x043: case 0x045: case 0x047: case 0x049: case 0x04b:
  case 0x04d: case 0x04f: case 0x051: case 0x053: case 0x055: {
    int irq = ((adr & 0x3f)>>1)-1;
    if((mn102->icrh[irq] != data) && (data & 15)) {
      printf("MN10200: irq %d enabled, level=%x, enable= %s %s %s %s\n", irq+1, (data >> 4) & 7,
		data & 1 ? inames[irq][0] : "-",
		data & 2 ? inames[irq][1] : "-",
		data & 4 ? inames[irq][2] : "-",
		data & 8 ? inames[irq][3] : "-");
	}
    if((mn102->icrh[irq] != data) && !(data & 15)) {
        printf("MN10200: irq %d disabled\n", irq+1);
	}
    mn102->icrh[irq] = data;
    break;
  }

  case 0x056: {
//    const char *modes[4] = { "l", "h", "fall", "rise" };
//    log_event("MN102", "irq3=%s irq2=%s irq1=%s irq0=%s",
//        modes[(data >> 6) & 3], modes[(data >> 4) & 3], modes[(data >> 2) & 3], modes[data & 3]);
    break;
  }

  case 0x057: {
//    const char *modes[4] = { "l", "1", "fall", "3" };
//    log_event("MN102", "irq_ki=%s", modes[data & 3]);
    break;
  }

  case 0x100: case 0x101: // DRAM control reg
  case 0x102: case 0x103: // Refresh counter
    break;

  case 0x180: case 0x190: {
    int ser = (adr-0x180) >> 4;
//    const char *parity[8] = { "no", "1", "2", "3", "l", "h", "even", "odd" };
//    const char *source[4] = { "sbt0", "timer 8", "2", "timer 9" };
    mn102->serial[ser].ctrll = data;

//    log_event("MN102", "Serial %d length=%c, parity=%s, stop=%c, source=%s",
//        ser,
//        data & 0x80 ? '8' : '7', parity[(data >> 4) & 7],
//        data & 8 ? '2' : '1', source[data & 3]);
    break;
  }

  case 0x181: case 0x191: {
    int ser = (adr-0x180) >> 4;
    mn102->serial[ser].ctrlh = data;
//    log_event("MN102", "Serial %d transmit=%s, receive=%s, break=%s, proto=%s, order=%s",
//        ser,
//        data & 0x80 ? "on" : "off", data & 0x40 ? "on" : "off",
//        data & 0x20 ? "on" : "off", data & 8 ? "sync" : "async",
//        data & 2 ? "msb" : "lsb");
    break;
  }

  case 0x182: case 0x192: {
    int ser = (adr-0x180) >> 4;
    mn102->serial[ser].buf = data;
    log_event("MN102", "Serial %d buffer=%02x", ser, data);
    break;
  }

	case 0x1a0:
		log_event("MN102", "AN %s timer7=%s /%d %s %s",
			data & 0x80 ? "on" : "off", data & 0x40 ? "on" : "off",
			1 << ((data >> 2) & 3), data & 2 ? "continuous" : "single", data & 1 ? "one" : "multi");
		break;

	case 0x1a1:
		log_event("MN102", "AN chans=0-%d current=%d", (data >> 4) & 7, data & 7);
		break;

	case 0x210: case 0x211: case 0x212: case 0x213: case 0x214:
	case 0x215: case 0x216: case 0x217: case 0x218: case 0x219:
		mn102->simple_timer[adr-0x210].base = data + 1;
//      printf("MN10200: Timer %d value set %02x\n", adr-0x210, data);
		refresh_timer(mn102, adr-0x210);
		break;

	case 0x21a:
		mn102->prescaler[0].cycles = data+1;
//      printf("MN10200: Prescale 0 cycle count %d\n", data+1);
		break;

	case 0x21b:
		mn102->prescaler[1].cycles = data+1;
//      printf("MN10200: Prescale 1 cycle count %d\n", data+1);
		break;

	case 0x220: case 0x221: case 0x222: case 0x223: case 0x224:
	case 0x225: case 0x226: case 0x227: case 0x228: case 0x229:
	{
//      const char *source[4] = { "TMxIO", "cascade", "prescale 0", "prescale 1" };
		mn102->simple_timer[adr-0x220].mode = data;
//      printf("MN10200: Timer %d %s b6=%d, source=%s\n", adr-0x220, data & 0x80 ? "on" : "off", (data & 0x40) != 0, source[data & 3]);

		if (data & 0x40)
		{
//          printf("MN10200: loading timer %d\n", adr-0x220);
			mn102->simple_timer[adr-0x220].cur = mn102->simple_timer[adr-0x220].base;
		}
		refresh_timer(mn102, adr-0x220);
		break;
	}

	case 0x22a:
//      printf("MN10200: Prescale 0 %s, ps0bc %s\n",
//          data & 0x80 ? "on" : "off", data & 0x40 ? "-> ps0br" : "off");
		mn102->prescaler[0].mode = data;
		break;

	case 0x22b:
//      printf("MN10200: Prescale 1 %s, ps1bc %s\n",
//          data & 0x80 ? "on" : "off", data & 0x40 ? "-> ps1br" : "off");
		mn102->prescaler[1].mode = data;
		break;

	case 0x230: case 0x240: case 0x250:
	{
//      const char *modes[4] = { "single", "double", "ioa", "iob" };
//      const char *sources[8] = { "pres.0", "pres.1", "iob", "sysclk", "*4", "*1", "6", "7" };
//      printf("MN10200: Timer %d comp=%s on_1=%s on_match=%s phase=%s source=%s\n",
//          10 + ((adr-0x230) >> 4),
//          modes[data >> 6], data & 0x20 ? "cleared" : "not cleared",  data & 0x10 ? "cleared" : "not cleared",
//          data & 8 ? "tff" : "rsff", sources[data & 7]);
		break;
	}

	case 0x231: case 0x241: case 0x251:
	{
//      const char *modes[4] = { "up", "down", "up on ioa", "up on iob" };
//      printf("MN10200: Timer %d %s ff=%s op=%s ext_trig=%s %s\n",
//      10 + ((adr-0x230) >> 4),
//      data & 0x80 ? "enable" : "disable", data & 0x40 ? "operate" : "clear",
//      modes[(data >> 4) & 3], data & 2 ? "on" : "off", data & 1 ? "one-shot" : "repeat");

		break;
	}

  case 0x234: case 0x244: case 0x254:
    log_event("MN102", "Timer %d ca=--%02x", 10 + ((adr-0x230) >> 4), data);
    break;

  case 0x235: case 0x245: case 0x255:
    log_event("MN102", "Timer %d ca=%02x--", 10 + ((adr-0x230) >> 4), data);
    break;

  case 0x236: case 0x246: case 0x256:
    log_event("MN102", "Timer %d ca read trigger", 10 + ((adr-0x230) >> 4));
    break;

  case 0x237: case 0x247: case 0x257: break;

  case 0x238: case 0x248: case 0x258:
    log_event("MN102", "Timer %d cb=--%02x", 10 + ((adr-0x230) >> 4), data);
    break;

  case 0x239: case 0x249: case 0x259:
    log_event("MN102", "Timer %d cb=%02x--", 10 + ((adr-0x230) >> 4), data);
    break;

  case 0x23a: case 0x24a: case 0x25a:
    log_event("MN102", "Timer %d cb read trigger", 10 + ((adr-0x230) >> 4));
    break;

  case 0x23b: case 0x24b: case 0x25b: break;

  case 0x260: case 0x261: {
//    const char *mode[4] = { "sysbuf", "4-phase", "4-phase 1/2", "3" };
//    log_event("MN102", "Sync Output %c timing=%s out=%s dir=%s mode=%s",
//        adr == 0x261 ? 'B' : 'A',
//        data & 0x10 ? "12A" : "1", data & 8 ? "sync a" :"P13-10",
//        data & 4 ? "ccw" : "cw", mode[data & 3]);
    break;
  }

  case 0x262:
    log_event("MN102", "Sync Output buffer = %02x", data);
    break;

  case 0x264:
	mn102->io->write_byte(MN10200_PORT1, data);
    break;

  case 0x280: case 0x290: case 0x2a0: case 0x2b0: case 0x2c0: case 0x2d0: case 0x2e0: case 0x2f0: {
    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].adr = (mn102->dma[dma].adr & 0x00ffff00) | data;
    logerror("MN10200: DMA %d adr=%06x\n", dma, mn102->dma[dma].adr);
    break;
  }

  case 0x281: case 0x291: case 0x2a1: case 0x2b1: case 0x2c1: case 0x2d1: case 0x2e1: case 0x2f1: {
    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].adr = (mn102->dma[dma].adr & 0x00ff00ff) | (data << 8);
    logerror("MN10200: DMA %d adr=%06x\n", dma, mn102->dma[dma].adr);
    break;
  }

  case 0x282: case 0x292: case 0x2a2: case 0x2b2: case 0x2c2: case 0x2d2: case 0x2e2: case 0x2f2: {
    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].adr = (mn102->dma[dma].adr & 0x0000ffff) | (data << 16);
    logerror("MN10200: DMA %d adr=%06x\n", dma, mn102->dma[dma].adr);
    break;
  }

  case 0x283: case 0x293: case 0x2a3: case 0x2b3: case 0x2c3: case 0x2d3: case 0x2e3: case 0x2f3:
    break;

  case 0x284: case 0x294: case 0x2a4: case 0x2b4: case 0x2c4: case 0x2d4: case 0x2e4: case 0x2f4: {
    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].count = (mn102->dma[dma].count & 0x00ffff00) | data;
    logerror("MN10200: DMA %d count=%06x\n", dma, mn102->dma[dma].count);
    break;
  }

  case 0x285: case 0x295: case 0x2a5: case 0x2b5: case 0x2c5: case 0x2d5: case 0x2e5: case 0x2f5: {
    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].count = (mn102->dma[dma].count & 0x00ff00ff) | (data << 8);
    logerror("MN10200: DMA %d count=%06x\n", dma, mn102->dma[dma].count);
    break;
  }

  case 0x286: case 0x296: case 0x2a6: case 0x2b6: case 0x2c6: case 0x2d6: case 0x2e6: case 0x2f6: {
    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].count = (mn102->dma[dma].count & 0x0000ffff) | (data << 16);
    logerror("MN10200: DMA %d count=%06x\n", dma, mn102->dma[dma].count);
    break;
  }

  case 0x287: case 0x297: case 0x2a7: case 0x2b7: case 0x2c7: case 0x2d7: case 0x2e7: case 0x2f7:
    break;

  case 0x288: case 0x298: case 0x2a8: case 0x2b8: case 0x2c8: case 0x2d8: case 0x2e8: case 0x2f8: {
    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].iadr = (mn102->dma[dma].iadr & 0xff00) | data;
    logerror("MN10200: DMA %d iadr=%03x\n", dma, mn102->dma[dma].iadr);
    break;
  }

  case 0x289: case 0x299: case 0x2a9: case 0x2b9: case 0x2c9: case 0x2d9: case 0x2e9: case 0x2f9: {
    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].iadr = (mn102->dma[dma].iadr & 0x00ff) | ((data & 3) << 8);
    logerror("MN10200: DMA %d iadr=%03x\n", dma, mn102->dma[dma].iadr);
    break;
  }

  case 0x28a: case 0x29a: case 0x2aa: case 0x2ba: case 0x2ca: case 0x2da: case 0x2ea: case 0x2fa: {
    static const char *const trans[4] = { "M-IO", "M-M", "M-X1", "m-X2" };
    static const char *const start[32] = {
      "soft", "a/d", "ser0tx", "set0rx", "ser1tx", "ser1rx",
      "timer0", "timer1", "timer2", "timer3", "timer4", "timer5", "timer6", "timer7", "timer8", "timer9",
      "timer10u", "timer10a", "timer10b",
      "timer11u", "timer11a", "timer12b",
      "timer12a", "timer12b",
      "irq0", "irq1", "irq2", "irq3",
      "X0e", "X1e", "X0l", "X1l"
    };

    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].ctrll = data;
    logerror("MN10200: DMA %d control ack=%s, trans=%s, start=%s\n",
	      dma,
	      data & 0x80 ? "level" : "pulse",
	      trans[(data >> 5) & 3],
	      start[data & 31]);
    break;
  }

  case 0x28b: case 0x29b: case 0x2ab: case 0x2bb: case 0x2cb: case 0x2db: case 0x2eb: case 0x2fb: {
    static const char *const tradr[4] = { "inc", "dec", "fixed", "reserved" };
    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].ctrlh = data;
	logerror("MN10200: DMA %d control %s irq=%s %s %s dir=%s %s %s\n",
          dma,
          data & 0x80 ? "enable" : "disable",
          data & 0x40 ? "off" : "on",
          data & 0x20 ? "byte" : "word",
          data & 0x10 ? "burst" : "single",
          data & 0x08 ? "dst" : "src",
          data & 0x04 ? "continue" : "normal",
          tradr[data & 3]);
    break;
  }

  case 0x28c: case 0x29c: case 0x2ac: case 0x2bc: case 0x2cc: case 0x2dc: case 0x2ec: case 0x2fc: {
    int dma = (adr-0x280) >> 4;
    mn102->dma[dma].irq = data & 7;
    logerror("MN10200: DMA %d irq=%d\n", dma, data & 7);
    break;
  }

  case 0x28d: case 0x29d: case 0x2ad: case 0x2bd: case 0x2cd: case 0x2dd: case 0x2ed: case 0x2fd:
    break;

  case 0x3b0:
    log_event("MN102", "Pull-ups 0-7 = -%c%c%c%c%c%c%c",
	      data & 0x40 ? '#' : '.',
	      data & 0x20 ? '#' : '.',
	      data & 0x10 ? '#' : '.',
	      data & 0x08 ? '#' : '.',
	      data & 0x04 ? '#' : '.',
	      data & 0x02 ? '#' : '.',
	      data & 0x01 ? '#' : '.');
    break;

  case 0x3b1:
    log_event("MN102", "Pull-ups 8-f = --%c%c%c%c%c%c",
	      data & 0x20 ? '#' : '.',
	      data & 0x10 ? '#' : '.',
	      data & 0x08 ? '#' : '.',
	      data & 0x04 ? '#' : '.',
	      data & 0x02 ? '#' : '.',
	      data & 0x01 ? '#' : '.');
    break;

  case 0x3b2:
    log_event("MN102", "Timer I/O 4-0 = %c%c%c%c%c",
	      data & 0x10 ? 'o' : 'i',
	      data & 0x08 ? 'o' : 'i',
	      data & 0x04 ? 'o' : 'i',
	      data & 0x02 ? 'o' : 'i',
	      data & 0x01 ? 'o' : 'i');
    break;

  case 0x3b3:
    log_event("MN102", "Timer I/O 12b/a-10b/a = %c%c %c%c %c%c",
	      data & 0x20 ? 'o' : 'i',
	      data & 0x10 ? 'o' : 'i',
	      data & 0x08 ? 'o' : 'i',
	      data & 0x04 ? 'o' : 'i',
	      data & 0x02 ? 'o' : 'i',
	      data & 0x01 ? 'o' : 'i');
    break;

	case 0x3c0:	// port 0 data
		mn102->io->write_byte(MN10200_PORT0, data);
		break;

	case 0x3c2:	// port 2 data
		mn102->io->write_byte(MN10200_PORT2, data);
		break;

	case 0x3c3:	// port 3 data
		mn102->io->write_byte(MN10200_PORT3, data);
		break;

	case 0x3e0:	// port0 ddr
		mn102->ddr[0] = data;
		break;

	case 0x3e1:	// port1 ddr
		mn102->ddr[1] = data;
		break;

	case 0x3e2:	// port2 ddr
		mn102->ddr[2] = data;
		break;

	case 0x3e3:	// port3 ddr
		mn102->ddr[3] = data;
		break;

  case 0x3f3:
/*    log_event("MN102", "Port 3 bits 4=%s 3=%s 2=%s 1=%s 0=%s",
          data & 0x10 ? data & 0x40 ? "serial_1" : "tm9" : "p34",
          data & 0x08 ? data & 0x20 ? "serial_0" : "tm8" : "p33",
          data & 0x04 ? "tm7" : "p32",
          data & 0x04 ? "tm6" : "p31",
          data & 0x04 ? "tm5" : "p30");*/
    break;


  default:
    log_event("MN102", "internal_w %04x, %02x (%03x)", adr+0xfc00, data, adr);
    break;
  }
}

static UINT32 mn10200_r(mn102_info *mn102, UINT32 adr, int type)
{
	if(type == MEM_WORD)
	{
		return mn10200_r(mn102, adr, MEM_BYTE) | (mn10200_r(mn102, adr+1, MEM_BYTE) << 8);
	}

  switch(adr) {
  case 0x00e:
    return mn102->iagr;

  case 0x00f:
    return 0;

  case 0x042: case 0x044: case 0x046: case 0x048: case 0x04a:
  case 0x04c: case 0x04e: case 0x050: case 0x052: case 0x054:
    return mn102->icrl[((adr & 0x3f)>>1)-1];

  case 0x043: case 0x045: case 0x047: case 0x049: case 0x04b:
  case 0x04d: case 0x04f: case 0x051: case 0x053: case 0x055:
    return mn102->icrh[((adr & 0x3f)>>1)-1];

  case 0x056:
    return 0;

    // p4i1 = tms empty line
  case 0x057:
    return 0x20;

  case 0x180: case 0x190:
    return mn102->serial[(adr-0x180) >> 4].ctrll;

  case 0x181: case 0x191:
    return mn102->serial[(adr-0x180) >> 4].ctrlh;

  case 0x182: {
    static int zz;
    return zz++;
  }

	case 0x183:
		return 0x10;

	case 0x200: case 0x201: case 0x202: case 0x203: case 0x204:
	case 0x205: case 0x206: case 0x207: case 0x208: case 0x209:
//      printf("MN10200: timer %d value read = %d\n", adr-0x200, mn102->simple_timer[adr-0x200].cur);
		return mn102->simple_timer[adr-0x200].cur;
		break;

	case 0x264:	// port 1 data
		return mn102->io->read_byte(MN10200_PORT1);

	case 0x28c: case 0x29c: case 0x2ac: case 0x2bc: case 0x2cc: case 0x2dc: case 0x2ec: case 0x2fc:
		{
			int dma = (adr-0x280) >> 4;
			return mn102->dma[dma].irq;
		}

	case 0x3c0:	// port 0 data
		return mn102->io->read_byte(MN10200_PORT0);

	case 0x3c2:	// port 2 data
		return mn102->io->read_byte(MN10200_PORT2);

	case 0x3c3:	// port 3 data
		return mn102->io->read_byte(MN10200_PORT3);

  default:
    log_event("MN102", "internal_r %04x (%03x)", adr+0xfc00, adr);
  }

  return 0;
}

static CPU_SET_INFO(mn10200)
{
	mn102_info *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
	        /* --- the following bits of info are set as 64-bit signed integers --- */
	        case CPUINFO_INT_PC:    /* intentional fallthrough */
	        case CPUINFO_INT_REGISTER + MN10200_PC:            cpustate->pc = info->i;                         break;
		case CPUINFO_INT_REGISTER + MN10200_PSW:           cpustate->psw = info->i;  break;
		case CPUINFO_INT_REGISTER + MN10200_MDR:           cpustate->mdr = info->i;  break;
		case CPUINFO_INT_REGISTER + MN10200_D0:            cpustate->d[0] = info->i;   break;
		case CPUINFO_INT_REGISTER + MN10200_D1:            cpustate->d[1] = info->i;   break;
		case CPUINFO_INT_REGISTER + MN10200_D2:            cpustate->d[2] = info->i;   break;
		case CPUINFO_INT_REGISTER + MN10200_D3:            cpustate->d[3] = info->i;   break;
		case CPUINFO_INT_REGISTER + MN10200_A0:            cpustate->a[0] = info->i;   break;
		case CPUINFO_INT_REGISTER + MN10200_A1:            cpustate->a[1] = info->i;   break;
		case CPUINFO_INT_REGISTER + MN10200_A2:            cpustate->a[2] = info->i;   break;
		case CPUINFO_INT_REGISTER + MN10200_A3:            cpustate->a[3] = info->i;   break;
		case CPUINFO_INT_REGISTER + MN10200_NMICR:         cpustate->nmicr = info->i;   break;
		case CPUINFO_INT_REGISTER + MN10200_IAGR:          cpustate->iagr = info->i;   break;

		case CPUINFO_INT_INPUT_STATE + MN10200_IRQ0:	mn102_extirq(cpustate, 0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + MN10200_IRQ1:	mn102_extirq(cpustate, 1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + MN10200_IRQ2:	mn102_extirq(cpustate, 2, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + MN10200_IRQ3:	mn102_extirq(cpustate, 3, info->i);	break;
	}
}

CPU_GET_INFO( mn10200 )
{
	mn102_info *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:          info->i = sizeof(mn102_info);   break;
		case CPUINFO_INT_INPUT_LINES:           info->i = 0;                    break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:    info->i = 0;                    break;
		case CPUINFO_INT_ENDIANNESS:            info->i = ENDIANNESS_LITTLE;    break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:      info->i = 1;                    break;
		case CPUINFO_INT_CLOCK_DIVIDER:         info->i = 1;                    break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES: info->i = 1;                    break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES: info->i = 4;                    break;
		case CPUINFO_INT_MIN_CYCLES:            info->i = 1;                    break;
		case CPUINFO_INT_MAX_CYCLES:            info->i = 8;                    break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM: info->i = 16;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 24;                   break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;                    break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:    info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:    info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:    info->i = 0;                    break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:      info->i = 8;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:      info->i = 8;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:      info->i = 0;                    break;

		case CPUINFO_INT_PC:    /* intentional fallthrough */
		case CPUINFO_INT_REGISTER + MN10200_PC:    info->i = cpustate->pc;                      break;
		case CPUINFO_INT_REGISTER + MN10200_PSW:   info->i = cpustate->psw; 			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:              info->setinfo = CPU_SET_INFO_NAME(mn10200);        break;
		case CPUINFO_FCT_INIT:                  info->init = CPU_INIT_NAME(mn10200);               break;
		case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(mn10200);             break;
		case CPUINFO_FCT_EXIT:                  info->exit = CPU_EXIT_NAME(mn10200);               break;
		case CPUINFO_FCT_EXECUTE:               info->execute = CPU_EXECUTE_NAME(mn10200);         break;
		case CPUINFO_FCT_BURN:                  info->burn = NULL;                              break;
		case CPUINFO_FCT_DISASSEMBLE:           info->disassemble = CPU_DISASSEMBLE_NAME(mn10200); break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:   info->icount = &cpustate->cycles;               break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "Panasonic MN10200");     break;
		case CPUINFO_STR_FAMILY:                   strcpy(info->s, "MN10200");                break;
		case CPUINFO_STR_VERSION:                  strcpy(info->s, "1.0");                 break;
		case CPUINFO_STR_SOURCE_FILE:                     strcpy(info->s, __FILE__);              break;
		case CPUINFO_STR_CREDITS:                  strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;

		case CPUINFO_STR_FLAGS:                    // intentional fallthrough
		case CPUINFO_STR_REGISTER + MN10200_PSW:   sprintf(info->s, "S=%d irq=%s im=%d %c%c%c%c %c%c%c%c",
									(cpustate->psw >> 12) & 3,
									cpustate->psw & 0x0800 ? "on" : "off",
									(cpustate->psw >> 8) & 7,
									cpustate->psw & 0x0080 ? 'V' : '-',
									cpustate->psw & 0x0040 ? 'C' : '-',
									cpustate->psw & 0x0020 ? 'N' : '-',
									cpustate->psw & 0x0010 ? 'Z' : '-',
									cpustate->psw & 0x0008 ? 'v' : '-',
									cpustate->psw & 0x0004 ? 'c' : '-',
									cpustate->psw & 0x0002 ? 'n' : '-',
									cpustate->psw & 0x0001 ? 'z' : '-');
									break;

		case CPUINFO_STR_REGISTER + MN10200_MDR:           sprintf(info->s, "MDR:  %04x", cpustate->mdr); break;

		case CPUINFO_STR_REGISTER + MN10200_D0:            sprintf(info->s, "D0: %06x", cpustate->d[0]); break;
		case CPUINFO_STR_REGISTER + MN10200_D1:            sprintf(info->s, "D1: %06x", cpustate->d[1]); break;
		case CPUINFO_STR_REGISTER + MN10200_D2:            sprintf(info->s, "D2: %06x", cpustate->d[2]); break;
		case CPUINFO_STR_REGISTER + MN10200_D3:            sprintf(info->s, "D3: %06x", cpustate->d[3]); break;
		case CPUINFO_STR_REGISTER + MN10200_A0:            sprintf(info->s, "A0: %06x", cpustate->a[0]); break;
		case CPUINFO_STR_REGISTER + MN10200_A1:            sprintf(info->s, "A1: %06x", cpustate->a[1]); break;
		case CPUINFO_STR_REGISTER + MN10200_A2:            sprintf(info->s, "A2: %06x", cpustate->a[2]); break;
		case CPUINFO_STR_REGISTER + MN10200_A3:            sprintf(info->s, "A3: %06x", cpustate->a[3]); break;

		case CPUINFO_STR_REGISTER + MN10200_NMICR:         sprintf(info->s, "MNICR:  %02x", cpustate->nmicr); break;
		case CPUINFO_STR_REGISTER + MN10200_IAGR:          sprintf(info->s, "IAGR:  %02x", cpustate->iagr); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(MN10200, mn10200);
