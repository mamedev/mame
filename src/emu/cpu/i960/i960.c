#include "emu.h"
#include "debugger.h"
#include "i960.h"

CPU_DISASSEMBLE( i960  );

#ifdef _MSC_VER
/* logb prototype is different for MS Visual C */
#include <float.h>
#define logb _logb
#endif


// Warning, IP = Instruction Pointer, called PC outside of Intel
//          PC = Process Control

enum { RCACHE_SIZE = 4 };

struct i960_state_t {
	UINT32 r[0x20];
	UINT32 rcache[RCACHE_SIZE][0x10];
	UINT32 rcache_frame_addr[RCACHE_SIZE];
	// rcache_pos = how deep in the stack we are.  0-(RCACHE_SIZE-1) means in-cache.
	// RCACHE_SIZE or greater means out of cache, must save to memory.
	INT32 rcache_pos;

	double fp[4];

	UINT32 SAT, PRCB, PC, AC;
	UINT32 IP, PIP, ICR;
	int bursting;

	int immediate_irq, immediate_vector, immediate_pri;

	device_irq_acknowledge_callback irq_cb;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;

	int icount;
};

INLINE i960_state_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == I960);
	return (i960_state_t *)downcast<legacy_cpu_device *>(device)->token();
}

static void do_call(i960_state_t *i960, UINT32 adr, int type, UINT32 stack);

INLINE UINT32 i960_read_dword_unaligned(i960_state_t *i960, UINT32 address)
{
	if (address & 3)
		return i960->program->read_byte(address) | i960->program->read_byte(address+1)<<8 | i960->program->read_byte(address+2)<<16 | i960->program->read_byte(address+3)<<24;
	else
		return i960->program->read_dword(address);
}

INLINE UINT16 i960_read_word_unaligned(i960_state_t *i960, UINT32 address)
{
	if (address & 1)
		return i960->program->read_byte(address) | i960->program->read_byte(address+1)<<8;
	else
		return i960->program->read_word(address);
}

INLINE void i960_write_dword_unaligned(i960_state_t *i960, UINT32 address, UINT32 data)
{
	if (address & 3)
	{
		i960->program->write_byte(address, data & 0xff);
		i960->program->write_byte(address+1, (data>>8)&0xff);
		i960->program->write_byte(address+2, (data>>16)&0xff);
		i960->program->write_byte(address+3, (data>>24)&0xff);
	}
	else
	{
		i960->program->write_dword(address, data);
	}
}

INLINE void i960_write_word_unaligned(i960_state_t *i960, UINT32 address, UINT16 data)
{
	if (address & 1)
	{
		i960->program->write_byte(address, data & 0xff);
		i960->program->write_byte(address+1, (data>>8)&0xff);
	}
	else
	{
		i960->program->write_word(address, data);
	}
}

INLINE void send_iac(i960_state_t *i960, UINT32 adr)
{
	UINT32 iac[4];
	iac[0] = i960->program->read_dword(adr);
	iac[1] = i960->program->read_dword(adr+4);
	iac[2] = i960->program->read_dword(adr+8);
	iac[3] = i960->program->read_dword(adr+12);

	switch(iac[0]>>24) {
	case 0x93: // reinit
		i960->SAT  = iac[1];
		i960->PRCB = iac[2];
		i960->IP   = iac[3];
		break;
	default:
		fatalerror("I960: %x: IAC %08x %08x %08x %08x\n", i960->PIP, iac[0], iac[1], iac[2], iac[3]);
		break;
	}
}

INLINE UINT32 get_ea(i960_state_t *i960, UINT32 opcode)
{
	int abase = (opcode >> 14) & 0x1f;
	if(!(opcode & 0x00001000)) { // MEMA
		UINT32 offset = opcode & 0x1fff;
		if(!(opcode & 0x2000))
			return offset;
		else
			return i960->r[abase]+offset;
	} else {                     // MEMB
		int index = opcode & 0x1f;
		int scale = (opcode >> 7) & 0x7;
		int mode  = (opcode >> 10) & 0xf;
		UINT32 ret;

		switch(mode) {
		case 0x4:
			return i960->r[abase];

		case 0x7:
			return i960->r[abase] + (i960->r[index] << scale);

		case 0xc:
			ret = i960->direct->read_decrypted_dword(i960->IP);
			i960->IP += 4;
			return ret;

		case 0xd:
			ret = i960->direct->read_decrypted_dword(i960->IP) + i960->r[abase];
			i960->IP += 4;
			return ret;

		case 0xe:
			ret = i960->direct->read_decrypted_dword(i960->IP) + (i960->r[index] << scale);
			i960->IP += 4;
			return ret;

		case 0xf:
			ret = i960->direct->read_decrypted_dword(i960->IP) + i960->r[abase] + (i960->r[index] << scale);
			i960->IP += 4;
			return ret;

		default:
			fatalerror("I960: %x: unhandled MEMB mode %x\n", i960->PIP, mode);
			return 0;
		}
	}
}

INLINE UINT32 get_1_ri(i960_state_t *i960, UINT32 opcode)
{
	if(!(opcode & 0x00000800))
		return i960->r[opcode & 0x1f];
	else
		return opcode & 0x1f;
}

INLINE UINT32 get_2_ri(i960_state_t *i960, UINT32 opcode)
{
	if(!(opcode & 0x00001000))
		return i960->r[(opcode>>14) & 0x1f];
	else
		return (opcode>>14) & 0x1f;
}

INLINE UINT64 get_2_ri64(i960_state_t *i960, UINT32 opcode)
{
	if(!(opcode & 0x00001000))
		return i960->r[(opcode>>14) & 0x1f] | ((UINT64)i960->r[((opcode>>14) & 0x1f)+1]<<32);
	else
		return (opcode>>14) & 0x1f;
}

INLINE void set_ri(i960_state_t *i960, UINT32 opcode, UINT32 val)
{
	if(!(opcode & 0x00002000))
		i960->r[(opcode>>19) & 0x1f] = val;
	else {
		fatalerror("I960: %x: set_ri on literal?\n", i960->PIP);
	}
}

INLINE void set_ri2(i960_state_t *i960, UINT32 opcode, UINT32 val, UINT32 val2)
{
	if(!(opcode & 0x00002000))
	{
		i960->r[(opcode>>19) & 0x1f] = val;
		i960->r[((opcode>>19) & 0x1f)+1] = val2;
	}
	else {
		fatalerror("I960: %x: set_ri2 on literal?\n", i960->PIP);
	}
}

INLINE void set_ri64(i960_state_t *i960, UINT32 opcode, UINT64 val)
{
	if(!(opcode & 0x00002000)) {
		i960->r[(opcode>>19) & 0x1f] = val;
		i960->r[((opcode>>19) & 0x1f)+1] = val >> 32;
	} else
		fatalerror("I960: %x: set_ri64 on literal?\n", i960->PIP);
}

INLINE double get_1_rif(i960_state_t *i960, UINT32 opcode)
{
	if(!(opcode & 0x00000800))
		return u2f(i960->r[opcode & 0x1f]);
	else {
		int idx = opcode & 0x1f;
		if(idx < 4)
			return i960->fp[idx];
		if(idx == 0x16)
			return 1.0;
		return 0.0;
	}
}

INLINE double get_2_rif(i960_state_t *i960, UINT32 opcode)
{
	if(!(opcode & 0x00001000))
		return u2f(i960->r[(opcode>>14) & 0x1f]);
	else {
		int idx = (opcode>>14) & 0x1f;
		if(idx < 4)
			return i960->fp[idx];
		if(idx == 0x16)
			return 1.0;
		return 0.0;
	}
}

INLINE void set_rif(i960_state_t *i960, UINT32 opcode, double val)
{
	if(!(opcode & 0x00002000))
		i960->r[(opcode>>19) & 0x1f] = f2u(val);
	else if(!(opcode & 0x00e00000))
		i960->fp[(opcode>>19) & 3] = val;
	else
		fatalerror("I960: %x: set_rif on literal?\n", i960->PIP);
}

INLINE double get_1_rifl(i960_state_t *i960, UINT32 opcode)
{
	if(!(opcode & 0x00000800)) {
		UINT64 v = i960->r[opcode & 0x1e];
		v |= ((UINT64)(i960->r[(opcode & 0x1e)+1]))<<32;
		return u2d(v);
	} else {
		int idx = opcode & 0x1f;
		if(idx < 4)
			return i960->fp[idx];
		if(idx == 0x16)
			return 1.0;
		return 0.0;
	}
}

INLINE double get_2_rifl(i960_state_t *i960, UINT32 opcode)
{
	if(!(opcode & 0x00001000)) {
		UINT64 v = i960->r[(opcode >> 14) & 0x1e];
		v |= ((UINT64)(i960->r[((opcode>>14) & 0x1e)+1]))<<32;
		return u2d(v);
	} else {
		int idx = (opcode>>14) & 0x1f;
		if(idx < 4)
			return i960->fp[idx];
		if(idx == 0x16)
			return 1.0;
		return 0.0;
	}
}

INLINE void set_rifl(i960_state_t *i960, UINT32 opcode, double val)
{
	if(!(opcode & 0x00002000)) {
		UINT64 v = d2u(val);
		i960->r[(opcode>>19) & 0x1e] = v;
		i960->r[((opcode>>19) & 0x1e)+1] = v>>32;
	} else if(!(opcode & 0x00e00000))
		i960->fp[(opcode>>19) & 3] = val;
	else
		fatalerror("I960: %x: set_rifl on literal?\n", i960->PIP);
}

INLINE UINT32 get_1_ci(i960_state_t *i960, UINT32 opcode)
{
	if(!(opcode & 0x00002000))
		return i960->r[(opcode >> 19) & 0x1f];
	else
		return (opcode >> 19) & 0x1f;
}

INLINE UINT32 get_2_ci(i960_state_t *i960, UINT32 opcode)
{
	return i960->r[(opcode >> 14) & 0x1f];
}

INLINE UINT32 get_disp(i960_state_t *i960, UINT32 opcode)
{
	UINT32 disp;
	disp = opcode & 0xffffff;
	if(disp & 0x00800000)
		disp |= 0xff000000;
	return disp-4;
}

INLINE UINT32 get_disp_s(i960_state_t *i960, UINT32 opcode)
{
	UINT32 disp;
	disp = opcode & 0x1fff;
	if(disp & 0x00001000)
		disp |= 0xffffe000;
	return disp-4;
}

INLINE void cmp_s(i960_state_t *i960, INT32 v1, INT32 v2)
{
	i960->AC &= ~7;
	if(v1<v2)
		i960->AC |= 4;
	else if(v1 == v2)
		i960->AC |= 2;
	else
		i960->AC |= 1;
}

INLINE void cmp_u(i960_state_t *i960, UINT32 v1, UINT32 v2)
{
	i960->AC &= ~7;
	if(v1<v2)
		i960->AC |= 4;
	else if(v1 == v2)
		i960->AC |= 2;
	else
		i960->AC |= 1;
}

INLINE void concmp_s(i960_state_t *i960, INT32 v1, INT32 v2)
{
	i960->AC &= ~7;
	if(v1 <= v2)
		i960->AC |= 2;
	else
		i960->AC |= 1;
}

INLINE void concmp_u(i960_state_t *i960, UINT32 v1, UINT32 v2)
{
	i960->AC &= ~7;
	if(v1 <= v2)
		i960->AC |= 2;
	else
		i960->AC |= 1;
}

INLINE void cmp_d(i960_state_t *i960, double v1, double v2)
{
	i960->AC &= ~7;
	if(v1<v2)
		i960->AC |= 4;
	else if(v1 == v2)
		i960->AC |= 2;
	else if(v1 > v2)
		i960->AC |= 1;
}

INLINE void bxx(i960_state_t *i960, UINT32 opcode, int mask)
{
	if(i960->AC & mask) {
		i960->IP += get_disp(i960, opcode);
	}
}

INLINE void bxx_s(i960_state_t *i960, UINT32 opcode, int mask)
{
	if(i960->AC & mask) {
		i960->IP += get_disp_s(i960, opcode);
	}
}

INLINE void test(i960_state_t *i960, UINT32 opcode, int mask)
{
	if(i960->AC & mask)
		i960->r[(opcode>>19) & 0x1f] = 1;
	else
		i960->r[(opcode>>19) & 0x1f] = 0;
}

INLINE const char *i960_get_strflags(i960_state_t *i960)
{
	static const char *const conditions[8] =
	{
		"no", "g", "e", "ge", "l", "ne", "le", "o"
	};

	return (conditions[i960->AC & 7]);
}

// interrupt dispatch
static void take_interrupt(i960_state_t *i960, int vector, int lvl)
{
	int int_tab =  i960->program->read_dword(i960->PRCB+20);	// interrupt table
	int int_SP  =  i960->program->read_dword(i960->PRCB+24);	// interrupt stack
	int SP;
	UINT32 IRQV;

	IRQV = i960->program->read_dword(int_tab + 36 + (vector-8)*4);

	// start the process
	if(!(i960->PC & 0x2000))	// if this is a nested interrupt, don't re-get int_SP
	{
		SP = int_SP;
	}
	else
	{
		SP = i960->r[I960_SP];
	}

	SP = (SP + 63) & ~63;
	SP += 128;	// emulate ElSemi's core, this fixes the crash in sonic the fighters

	do_call(i960, IRQV, 7, SP);

	// save the processor state
	i960->program->write_dword(i960->r[I960_FP]-16, i960->PC);
	i960->program->write_dword(i960->r[I960_FP]-12, i960->AC);
	// store the vector
	i960->program->write_dword(i960->r[I960_FP]-8, vector-8);

	i960->PC &= ~0x1f00;	// clear priority, state, trace-fault pending, and trace enable
	i960->PC |= (lvl<<16);	// set CPU level to current IRQ level
	i960->PC |= 0x2002;	// set supervisor mode & interrupt flag
}

static void check_irqs(i960_state_t *i960)
{
	int int_tab =  i960->program->read_dword(i960->PRCB+20);	// interrupt table
	int cpu_pri = (i960->PC>>16)&0x1f;
	int pending_pri;
	int lvl, irq, take = -1;
	int vword;
	static const UINT32 lvlmask[4] = { 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

	pending_pri = i960->program->read_dword(int_tab);		// read pending priorities

	if ((i960->immediate_irq) && ((cpu_pri < i960->immediate_pri) || (i960->immediate_pri == 31)))
	{
		take_interrupt(i960, i960->immediate_vector, i960->immediate_pri);
		i960->immediate_irq = 0;
	}
	else
	{
		for(lvl = 31; lvl >= 0; lvl--) {
			if((pending_pri & (1 << lvl)) && ((cpu_pri < lvl) || (lvl == 31))) {
				int word, wordl, wordh;

				// figure out which word contains this level's priorities
				word = ((lvl / 4) * 4) + 4;	// (lvl/4) = word address, *4 for byte address, +4 to skip pending priorities
				wordl = (lvl % 4) * 8;
				wordh = (wordl + 8) - 1;

				vword = i960->program->read_dword(int_tab + word);

				// take the first vector we find for this level
				for (irq = wordh; irq >= wordl; irq--) {
					if(vword & (1 << irq)) {
						// clear pending bit
						vword &= ~(1 << irq);
						i960->program->write_dword(int_tab + word, vword);
						take = irq;
						break;
					}
				}

				// if no vectors were found at our level, it's an error
				if(take == -1) {
					logerror("i960: ERROR! no vector found for pending level %d\n", lvl);

					// try to recover...
					pending_pri &= ~(1 << lvl);
					i960->program->write_dword(int_tab, pending_pri);
					return;
				}

				// if no vectors are waiting for this level, clear the level bit
				if(!(vword & lvlmask[lvl % 4])) {
					pending_pri &= ~(1 << lvl);
					i960->program->write_dword(int_tab, pending_pri);
				}

				take += ((lvl/4) * 32);

				take_interrupt(i960, take, lvl);
				return;
			}
		}
	}
}

static void do_call(i960_state_t *i960, UINT32 adr, int type, UINT32 stack)
{
	int i;
	UINT32 FP;

	// call and callx take 9 cycles base
	i960->icount -= 9;

	// set the new RIP
	i960->r[I960_RIP] = i960->IP;
//  mame_printf_debug("CALL (type %d): FP %x, %x => %x, stack %x, rcache_pos %d\n", type, i960->r[I960_FP], i960->r[I960_RIP], adr, stack, i960->rcache_pos);

	// are we out of cache entries?
	if (i960->rcache_pos >= RCACHE_SIZE) {
		// flush the current register set to the current frame
		FP = i960->r[I960_FP] & ~0x3f;
		for (i = 0; i < 16; i++) {
			i960->program->write_dword(FP + (i*4), i960->r[i]);
		}
	}
	else	// a cache entry is available, use it
	{
		memcpy(&i960->rcache[i960->rcache_pos][0], i960->r, 0x10 * sizeof(UINT32));
		i960->rcache_frame_addr[i960->rcache_pos] = i960->r[I960_FP] & ~0x3f;
	}
	i960->rcache_pos++;

	i960->IP = adr;
	i960->r[I960_PFP] = i960->r[I960_FP] & ~7;
	i960->r[I960_PFP] |= type;

	if(type == 7) {	// interrupts need special handling
		// set the stack to the passed-in value to properly handle nested interrupts
		// (can't set it externally or the original program's SP will be lost)
		i960->r[I960_SP] = stack;
	}

	i960->r[I960_FP]  = (i960->r[I960_SP] + 63) & ~63;
	i960->r[I960_SP]  = i960->r[I960_FP] + 64;
}

static void do_ret_0(i960_state_t *i960)
{
//  int type = i960->r[I960_PFP] & 7;

	i960->r[I960_FP] = i960->r[I960_PFP] & ~0x3f;

	i960->rcache_pos--;

	// normal situation: if we're still above rcache size, we're not in cache.
	// abnormal situation (after the app does a FLUSHREG): rcache_pos will be 0
	// coming in, but we must still treat it as a not-in-cache situation.
	if ((i960->rcache_pos >= RCACHE_SIZE) || (i960->rcache_pos < 0))
	{
		int i;
		for(i=0; i<0x10; i++)
			i960->r[i] = i960->program->read_dword(i960->r[I960_FP]+4*i);

		if (i960->rcache_pos < 0)
		{
			i960->rcache_pos = 0;
		}
	}
	else
	{
		memcpy(i960->r, i960->rcache[i960->rcache_pos], 0x10*sizeof(UINT32));
	}

//  mame_printf_debug("RET (type %d): FP %x, %x => %x, rcache_pos %d\n", type, i960->r[I960_FP], i960->IP, i960->r[I960_RIP], i960->rcache_pos);
	i960->IP = i960->r[I960_RIP];
}

static void do_ret(i960_state_t *i960)
{
	UINT32 x, y;
	i960->icount -= 7;
	switch(i960->r[I960_PFP] & 7) {
	case 0:
		do_ret_0(i960);
		break;

	case 7:
		x = i960->program->read_dword(i960->r[I960_FP]-16);
		y = i960->program->read_dword(i960->r[I960_FP]-12);
		do_ret_0(i960);
		i960->AC = y;
		// #### test supervisor
		i960->PC = x;

		// check for another IRQ now that we're back
		check_irqs(i960);
		break;

	default:
		fatalerror("I960: %x: Unsupported return mode %d\n", i960->PIP, i960->r[I960_PFP] & 7);
	}
}

INLINE void execute_op(i960_state_t *i960, UINT32 opcode)
{
	UINT32 t1, t2;
	double t1f, t2f;

	switch(opcode >> 24) {
		case 0x08: // b
			i960->icount--;
			i960->IP += get_disp(i960, opcode);
			break;

		case 0x09: // call
			do_call(i960, i960->IP+get_disp(i960, opcode), 0, i960->r[I960_SP]);
			break;

		case 0x0a: // ret
			do_ret(i960);
			break;

		case 0x0b: // bal
			i960->icount -= 5;
			i960->r[0x1e] = i960->IP;
			i960->IP += get_disp(i960, opcode);
			break;

		case 0x10: // bno
			i960->icount--;
			if(!(i960->AC & 7)) {
				i960->IP += get_disp(i960, opcode);
			}
			break;

		case 0x11: // bg
			i960->icount--;
			bxx(i960, opcode, 1);
			break;

		case 0x12: // be
			i960->icount--;
			bxx(i960, opcode, 2);
			break;

		case 0x13: // bge
			i960->icount--;
			bxx(i960, opcode, 3);
			break;

		case 0x14: // bl
			i960->icount--;
			bxx(i960, opcode, 4);
			break;

		case 0x15: // bne
			i960->icount--;
			bxx(i960, opcode, 5);
			break;

		case 0x16: // ble
			i960->icount--;
			bxx(i960, opcode, 6);
			break;

		case 0x17: // bo
			i960->icount--;
			bxx(i960, opcode, 7);
			break;

		case 0x20: // testno
			i960->icount--;
			if(!(i960->AC & 7))
				i960->r[(opcode>>19) & 0x1f] = 1;
			else
				i960->r[(opcode>>19) & 0x1f] = 0;
			break;

		case 0x21: // testg
			i960->icount--;
			test(i960, opcode, 1);
			break;

		case 0x22: // teste
			i960->icount--;
			test(i960, opcode, 2);
			break;

		case 0x23: // testge
			i960->icount--;
			test(i960, opcode, 3);
			break;

		case 0x24: // testl
			i960->icount--;
			test(i960, opcode, 4);
			break;

		case 0x25: // testne
			i960->icount--;
			test(i960, opcode, 5);
			break;

		case 0x26: // testle
			i960->icount--;
			test(i960, opcode, 6);
			break;

		case 0x27: // testo
			i960->icount--;
			test(i960, opcode, 7);
			break;

		case 0x30: // bbc
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode) & 0x1f;
			t2 = get_2_ci(i960, opcode);
			if(!(t2 & (1<<t1))) {
				i960->AC = (i960->AC & ~7) | 2;
				i960->IP += get_disp_s(i960, opcode);
			} else
				i960->AC &= ~7;
			break;

		case 0x31: // cmp0bg
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_u(i960, t1, t2);
			bxx_s(i960, opcode, 1);
			break;

		case 0x32: // cmpobe
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_u(i960, t1, t2);
			bxx_s(i960, opcode, 2);
			break;

		case 0x33: // cmpobge
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_u(i960, t1, t2);
			bxx_s(i960, opcode, 3);
			break;

		case 0x34: // cmpobl
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_u(i960, t1, t2);
			bxx_s(i960, opcode, 4);
			break;

		case 0x35: // cmpobne
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_u(i960, t1, t2);
			bxx_s(i960, opcode, 5);
			break;

		case 0x36: // cmpoble
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_u(i960, t1, t2);
			bxx_s(i960, opcode, 6);
			break;

		case 0x37: // bbs
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode) & 0x1f;
			t2 = get_2_ci(i960, opcode);
			if(t2 & (1<<t1)) {
				i960->AC = (i960->AC & ~7) | 2;
				i960->IP += get_disp_s(i960, opcode);
			} else
				i960->AC &= ~7;
			break;

		case 0x39: // cmpibg
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_s(i960, t1, t2);
			bxx_s(i960, opcode, 1);
			break;

		case 0x3a: // cmpibe
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_s(i960, t1, t2);
			bxx_s(i960, opcode, 2);
			break;

		case 0x3b: // cmpibge
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_s(i960, t1, t2);
			bxx_s(i960, opcode, 3);
			break;

		case 0x3c: // cmpibl
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_s(i960, t1, t2);
			bxx_s(i960, opcode, 4);
			break;

		case 0x3d: // cmpibne
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_s(i960, t1, t2);
			bxx_s(i960, opcode, 5);
			break;

		case 0x3e: // cmpible
			i960->icount -= 4;
			t1 = get_1_ci(i960, opcode);
			t2 = get_2_ci(i960, opcode);
			cmp_s(i960, t1, t2);
			bxx_s(i960, opcode, 6);
			break;

		case 0x58:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // notbit
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2 ^ (1<<(t1 & 31)));
				break;

			case 0x1: // and
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2 & t1);
				break;

			case 0x2: // andnot
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2 & ~t1);
				break;

			case 0x3: // setbit
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2 | (1<<(t1 & 31)));
				break;

			case 0x4: // notand
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, (~t2) & t1);
				break;

			case 0x6: // xor
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2 ^ t1);
				break;

			case 0x7: // or
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2 | t1);
				break;

			case 0x8: // nor
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, ((~t2) & (~t1)));
				break;

			case 0x9: // xnor
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, ~(t2 ^ t1));
				break;

			case 0xa: // not
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				set_ri(i960, opcode, ~t1);
				break;

			case 0xb: // ornot
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2 | ~t1);
				break;

			case 0xc: // clrbit
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2 & ~(1<<(t1 & 31)));
				break;

			case 0xd: // notor
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, (~t2) | t1);
				break;

			case 0xe: // nand
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, ~t2 | ~t1);
				break;

			case 0xf: // alterbit
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				if(i960->AC & 2)
					set_ri(i960, opcode, t2 | (1<<(t1 & 31)));
				else
					set_ri(i960, opcode, t2 & ~(1<<(t1 & 31)));
				break;

			default:
				fatalerror("I960: %x: Unhandled 58.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x59:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // addo
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2+t1);
				break;

			case 0x1: // addi
				// #### overflow
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2+t1);
				break;

			case 0x2: // subo
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2-t1);
				break;

			case 0x3: // subi
				// #### overflow
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2-t1);
				break;

			case 0x8: // shro
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2>>t1);
				break;

			case 0xa: // shrdi
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				if(((INT32)t2) < 0) {
					if(t2 & ((1<<t1)-1))
						set_ri(i960, opcode, (((INT32)t2)>>t1)+1);
					else
						set_ri(i960, opcode, ((INT32)t2)>>t1);
				} else
					set_ri(i960, opcode, t2>>t1);
				break;

			case 0xb: // shri
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, ((INT32)t2)>>t1);
				break;

			case 0xc: // shlo
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2<<t1);
				break;

			case 0xd: // rotate
				i960->icount--;
				t1 = get_1_ri(i960, opcode) & 0x1f;
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, (t2<<t1)|(t2>>(32-t1)));
				break;

			case 0xe: // shli
				// missing overflow
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2<<t1);
				break;

			default:
				fatalerror("I960: %x: Unhandled 59.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5a:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // cmpo
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				cmp_u(i960, t1, t2);
				break;

			case 0x1: // cmpi
				i960->icount--;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				cmp_s(i960, t1, t2);
				break;

			case 0x2: // concmpo
				i960->icount--;
				if(!(i960->AC & 0x4)) {
					t1 = get_1_ri(i960, opcode);
					t2 = get_2_ri(i960, opcode);
					concmp_u(i960, t1, t2);
				}
				break;

			case 0x3: // concmpi
				i960->icount--;
				if(!(i960->AC & 0x4)) {
					t1 = get_1_ri(i960, opcode);
					t2 = get_2_ri(i960, opcode);
					concmp_s(i960, t1, t2);
				}
				break;

			case 0x4: // cmpinco
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				cmp_u(i960, t1, t2);
				set_ri(i960, opcode, t2+1);
				break;

			case 0x5: // cmpinci
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				cmp_s(i960, t1, t2);
				set_ri(i960, opcode, t2+1);
				break;

			case 0x6: // cmpdeco
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				cmp_u(i960, t1, t2);
				set_ri(i960, opcode, t2-1);
				break;

			case 0x7: // cmpdeci
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				cmp_s(i960, t1, t2);
				set_ri(i960, opcode, t2-1);
				break;

			case 0xe: // chkbit
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode) & 0x1f;
				t2 = get_2_ri(i960, opcode);
				if(t2 & (1<<t1))
					i960->AC = (i960->AC & ~7) | 2;
				else
					i960->AC &= ~7;
				break;

			default:
				fatalerror("I960: %x: Unhandled 5a.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5b:
			switch((opcode >> 7) & 0xf) {
			case 0x0:	// addc
				{
					UINT64 res;

					i960->icount -= 2;
					t1 = get_1_ri(i960, opcode);
					t2 = get_2_ri(i960, opcode);
					res = t2+(t1+((i960->AC>>1)&1));
					set_ri(i960, opcode, res&0xffffffff);

					i960->AC &= ~0x3;	// clear C and V
					// set carry
					i960->AC |= ((res) & (((UINT64)1) << 32)) ? 0x2 : 0;
					// set overflow
					i960->AC |= (((res) ^ (t1)) & ((res) ^ (t2)) & 0x80000000) ? 1: 0;
				}
				break;

			case 0x2:	// subc
				{
					UINT64 res;

					i960->icount -= 2;
					t1 = get_1_ri(i960, opcode);
					t2 = get_2_ri(i960, opcode);
					res = t2-(t1+((i960->AC>>1)&1));
					set_ri(i960, opcode, res&0xffffffff);

					i960->AC &= ~0x3;	// clear C and V
					// set carry
					i960->AC |= ((res) & (((UINT64)1) << 32)) ? 0x2 : 0;
					// set overflow
					i960->AC |= (((t2) ^ (t1)) & ((t2) ^ (res)) & 0x80000000) ? 1 : 0;
				}
				break;

			default:
				fatalerror("I960: %x: Unhandled 5b.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5c:
			switch((opcode >> 7) & 0xf) {
			case 0xc: // mov
				i960->icount -= 2;
				t1 = get_1_ri(i960, opcode);
				set_ri(i960, opcode, t1);
				break;

			default:
				fatalerror("I960: %x: Unhandled 5c.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5d:
			switch((opcode >> 7) & 0xf) {
			case 0xc: // movl
				i960->icount -= 2;
				t2 = (opcode>>19) & 0x1e;
				if(opcode & 0x00000800) { // litteral
					t1 = opcode & 0x1f;
					i960->r[t2] = i960->r[t2+1] = t1;
				} else
					memcpy(i960->r+t2, i960->r+(opcode & 0x1f), 2*sizeof(UINT32));
				break;

			default:
				fatalerror("I960: %x: Unhandled 5d.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5e:
			switch((opcode >> 7) & 0xf) {
			case 0xc: // movt
				i960->icount -= 3;
				t2 = (opcode>>19) & 0x1c;
				if(opcode & 0x00000800) { // litteral
					t1 = opcode & 0x1f;
					i960->r[t2] = i960->r[t2+1] = i960->r[t2+2]= t1;
				} else
					memcpy(i960->r+t2, i960->r+(opcode & 0x1f), 3*sizeof(UINT32));
				break;

			default:
				fatalerror("I960: %x: Unhandled 5e.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5f:
			switch((opcode >> 7) & 0xf) {
			case 0xc: // movq
				i960->icount -= 4;
				t2 = (opcode>>19) & 0x1c;
				if(opcode & 0x00000800) { // litteral
					t1 = opcode & 0x1f;
					i960->r[t2] = i960->r[t2+1] = i960->r[t2+2] = i960->r[t2+3] = t1;
				} else
					memcpy(i960->r+t2, i960->r+(opcode & 0x1f), 4*sizeof(UINT32));
				break;

			default:
				fatalerror("I960: %x: Unhandled 5f.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x60:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // synmov
				i960->icount -= 6;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				// interrupt control register
				if(t1 == 0xff000004)
					i960->ICR = i960->program->read_dword(t2);
				else
					i960->program->write_dword(t1,    i960->program->read_dword(t2));
				i960->AC = (i960->AC & ~7) | 2;
				break;

			case 0x2: // synmovq
				i960->icount -= 12;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				if(t1 == 0xff000010)
					send_iac(i960, t2);
				else {
					i960->program->write_dword(t1,    i960->program->read_dword(t2));
					i960->program->write_dword(t1+4,  i960->program->read_dword(t2+4));
					i960->program->write_dword(t1+8,  i960->program->read_dword(t2+8));
					i960->program->write_dword(t1+12, i960->program->read_dword(t2+12));
				}
				i960->AC = (i960->AC & ~7) | 2;
				break;

			default:
				fatalerror("I960: %x: Unhandled 60.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x64:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // spanbit
				{
					UINT32 res = 0xffffffff;
					int i;

					i960->icount -= 10;

					t1 = get_1_ri(i960, opcode);
					i960->AC &= ~7;

					for (i = 31; i >= 0; i--)
					{
						if (!(t1 & (1<<i)))
						{
							i960->AC |= 2;
							res = i;
							break;
						}
					}

					set_ri(i960, opcode, res);
				}
				break;

			case 0x1: // scanbit
				{
					UINT32 res = 0xffffffff;
					int i;

					i960->icount -= 10;

					t1 = get_1_ri(i960, opcode);
					i960->AC &= ~7;

					for (i = 31; i >= 0; i--)
					{
						if (t1 & (1<<i))
						{
							i960->AC |= 2;
							res = i;
							break;
						}
					}

					set_ri(i960, opcode, res);
				}
				break;

			case 0x5: // modac
				i960->icount -= 10;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, i960->AC);
				i960->AC = (i960->AC & ~t1) | (t2 & t1);
				break;

			default:
				fatalerror("I960: %x: Unhandled 64.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x65:
			switch((opcode >> 7) & 0xf) {
			case 0x5: // modpc
				i960->icount -= 10;
				t1 = i960->PC;
				t2 = get_2_ri(i960, opcode);
				i960->PC = (i960->PC & ~t2) | (i960->r[(opcode>>19) & 0x1f] & t2);
				set_ri(i960, opcode, t1);
				break;

			default:
				fatalerror("I960: %x: Unhandled 65.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x66:
			switch((opcode >> 7) & 0xf) {
			case 0xd: // flushreg
				if (i960->rcache_pos > 4)
				{
					i960->rcache_pos = 4;
				}
				for(t1=0; t1 < i960->rcache_pos; t1++)
				{
					int i;

					for (i = 0; i < 0x10; i++)
					{
						i960->program->write_dword(i960->rcache_frame_addr[t1] + (i * sizeof(UINT32)), i960->rcache[t1][i]);
					}
				}
				i960->rcache_pos = 0;
				break;

			default:
				fatalerror("I960: %x: Unhandled 66.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x67:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // emul
				i960->icount -= 37;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);

				set_ri64(i960, opcode, (INT64)t1 * (INT64)t2);
				break;

			case 0x1: // ediv
				i960->icount -= 37;
				{
					UINT64 src1, src2;

					src1 = get_1_ri(i960, opcode);
					src2 = get_2_ri64(i960, opcode);

					set_ri2(i960, opcode, src2 % src1, src2 / src1);
				}
				break;

			case 0x4: // cvtir
				i960->icount -= 30;
				t1 = get_1_ri(i960, opcode);
				set_rif(i960, opcode, (double)(INT32)t1);
				break;

			case 0x6: // scalerl
				i960->icount -= 30;
				t1 = get_1_ri(i960, opcode);
				t2f = get_2_rifl(i960, opcode);
				set_rifl(i960, opcode, t2f * pow(2.0, (double)(INT32)t1));
				break;

			case 0x7: // scaler
				i960->icount -= 30;
				t1 = get_1_ri(i960, opcode);
				t2f = get_2_rif(i960, opcode);
			set_rif(i960, opcode, t2f * pow(2.0, (double)(INT32)t1));
				break;

			default:
				fatalerror("I960: %x: Unhandled 67.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x68:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // atanr
				i960->icount -= 267;
				t1f = get_1_rif(i960, opcode);
				t2f = get_2_rif(i960, opcode);
				set_rif(i960, opcode, atan2(t2f, t1f));
				break;

			case 0x1: // logepr
				i960->icount -= 400;
				t1f = get_1_rif(i960, opcode);
				t2f = get_2_rif(i960, opcode);
				set_rif(i960, opcode, t2f*log(t1f+1.0)/log(2.0));
				break;

			case 0x3: // remr
				i960->icount -= 67;	// (67 to 75878 depending on opcodes!!!)
				t1f = get_1_rif(i960, opcode);
				t2f = get_2_rif(i960, opcode);
				set_rif(i960, opcode, fmod(t2f, t1f));
				break;

			case 0x5: // cmpr
				i960->icount -= 10;
				t1f = get_1_rif(i960, opcode);
				t2f = get_2_rif(i960, opcode);
				cmp_d(i960, t1f, t2f);
				break;

			case 0x8: // sqrtr
				i960->icount -= 104;
				t1f = get_1_rif(i960, opcode);
				set_rif(i960, opcode, sqrt(t1f));
				break;

			case 0xa: // logbnr
				i960->icount -= 37;
				t1f = get_1_rif(i960, opcode);
				set_rif(i960, opcode, logb(t1f));
				break;

			case 0xb: // roundr
				{
					INT32 st1 = get_1_rif(i960, opcode);
					i960->icount -= 69;
					set_rif(i960, opcode, (double)st1);
				}
				break;

			case 0xc: // sinr
				i960->icount -= 406;
				t1f = get_1_rif(i960, opcode);
				set_rif(i960, opcode, sin(t1f));
				break;

			case 0xd: // cosr
				i960->icount -= 406;
				t1f = get_1_rif(i960, opcode);
				set_rif(i960, opcode, sin(t1f));
				break;

			case 0xe: // tanr
				i960->icount -= 293;
				t1f = get_1_rif(i960, opcode);
				set_rif(i960, opcode, tan(t1f));
				break;

			default:
				fatalerror("I960: %x: Unhandled 68.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x69:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // atanrl
				i960->icount -= 350;
				t1f = get_1_rifl(i960, opcode);
				t2f = get_2_rifl(i960, opcode);
				set_rifl(i960, opcode, atan2(t2f, t1f));
				break;

			case 0x2: // logrl
				i960->icount -= 438;
				t1f = get_1_rifl(i960, opcode);
				set_rifl(i960, opcode, log(t1f));
				break;

			case 0x5: // cmprl
				i960->icount -= 12;
				t1f = get_1_rifl(i960, opcode);
				t2f = get_2_rifl(i960, opcode);
				cmp_d(i960, t1f, t2f);
				break;

			case 0x8: // sqrtrl
				i960->icount -= 104;
				t1f = get_1_rifl(i960, opcode);
				set_rifl(i960, opcode, sqrt(t1f));
				break;

			case 0x9: // exprl
				i960->icount -= 334;
				t1f = get_1_rifl(i960, opcode);
				set_rifl(i960, opcode, pow(2.0, t1f)-1.0);
				break;

			case 0xa: // logbnrl
				i960->icount -= 37;
				t1f = get_1_rifl(i960, opcode);
				set_rifl(i960, opcode, logb(t1f));
				break;

			case 0xb: // roundrl
				{
					INT32 st1 = get_1_rifl(i960, opcode);
					i960->icount -= 70;
					set_rifl(i960, opcode, (double)st1);
				}
				break;

			case 0xc: // sinrl
				i960->icount -= 441;
				t1f = get_1_rifl(i960, opcode);
				set_rifl(i960, opcode, sin(t1f));
				break;

			case 0xd: // cosrl
				i960->icount -= 441;
				t1f = get_1_rifl(i960, opcode);
				set_rifl(i960, opcode, cos(t1f));
				break;

			case 0xe: // tanrl
				i960->icount -= 323;
				t1f = get_1_rifl(i960, opcode);
				set_rifl(i960, opcode, tan(t1f));
				break;

			default:
				fatalerror("I960: %x: Unhandled 69.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x6c:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // cvtri
				i960->icount -= 33;
				t1f = get_1_rif(i960, opcode);
				// apply rounding mode
				// we do this a little indirectly to avoid some odd GCC warnings
				t2f = 0.0;
				switch((i960->AC>>30)&3)
				{
					case 0: t2f = floor(t1f+0.5); break;
					case 1: t2f = floor(t1f); break;
					case 2: t2f = ceil(t1f); break;
					case 3: t2f = t1f; break;
				}
				set_ri(i960, opcode, (INT32)t2f);
				break;

			case 0x2: // cvtzri
				i960->icount -= 43;
				t1f = get_1_rif(i960, opcode);
				set_ri(i960, opcode, (INT32)t1f);
				break;

			case 0x3: // cvtzril
				i960->icount -= 44;
				t1f = get_1_rif(i960, opcode);
				set_ri64(i960, opcode, (INT64)t1f);
				break;

			case 0x9: // movr
				i960->icount -= 5;
				t1f = get_1_rif(i960, opcode);
				set_rif(i960, opcode, t1f);
				break;

			default:
				fatalerror("I960: %x: Unhandled 6c.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x6d:
			switch((opcode >> 7) & 0xf) {
			case 0x9: // movrl
				i960->icount -= 6;
				t1f = get_1_rifl(i960, opcode);
				set_rifl(i960, opcode, t1f);
				break;

			default:
				fatalerror("I960: %x: Unhandled 6d.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x6e:
			switch((opcode >> 7) & 0xf) {
			case 0x1: // movre
				{
					UINT32 *src=0, *dst=0;

					i960->icount -= 8;

					if(!(opcode & 0x00000800)) {
						src = (UINT32 *)&i960->r[opcode & 0x1e];
					} else {
						int idx = opcode & 0x1f;
						if(idx < 4)
							src = (UINT32 *)&i960->fp[idx];
					}

					if(!(opcode & 0x00002000)) {
						dst = (UINT32 *)&i960->r[(opcode>>19) & 0x1e];
					} else if(!(opcode & 0x00e00000))
						dst = (UINT32 *)&i960->fp[(opcode>>19) & 3];

					dst[0] = src[0];
					dst[1] = src[1];
					dst[2] = src[2]&0xffff;
				}
				break;
			case 0x2: // cpysre
				i960->icount -= 8;
				t1f = get_1_rifl(i960, opcode);
				t2f = get_2_rifl(i960, opcode);

				if (t2f >= 0.0)
					set_rifl(i960, opcode, fabs(t1f));
				else
					set_rifl(i960, opcode, -fabs(t1f));
				break;
			default:
				fatalerror("I960: %x: Unhandled 6e.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x70:
			switch((opcode >> 7) & 0xf) {
			case 0x1: // mulo
				i960->icount -= 18;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2*t1);
				break;

			case 0x8: // remo
				i960->icount -= 37;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, t2%t1);
				break;

			case 0xb: // divo
				i960->icount -= 37;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				if (t1 == 0)	// HACK!
					set_ri(i960, opcode, 0);
				else
					set_ri(i960, opcode, t2/t1);
				break;

			default:
				fatalerror("I960: %x: Unhandled 70.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x74:
			switch((opcode >> 7) & 0xf) {
			case 0x1: // muli
				i960->icount -= 18;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, ((INT32)t2)*((INT32)t1));
				break;

			case 0x8: // remi
				i960->icount -= 37;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, ((INT32)t2)%((INT32)t1));
				break;

			case 0x9:{// modi
				INT32 src1, src2, dst;
				i960->icount -= 37;
				src1 = (INT32)get_1_ri(i960, opcode);
				src2 = (INT32)get_2_ri(i960, opcode);
				dst = src2 - ((src2/src1)*src1);
				if(((src2*src1) < 0) && (dst != 0))
					dst += src1;
				set_ri(i960, opcode, dst);
				break;
			}

			case 0xb: // divi
				i960->icount -= 37;
				t1 = get_1_ri(i960, opcode);
				t2 = get_2_ri(i960, opcode);
				set_ri(i960, opcode, ((INT32)t2)/((INT32)t1));
				break;

			default:
				fatalerror("I960: %x: Unhandled 74.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x78:
			switch((opcode >> 7) & 0xf) {
			case 0xb: // divr
				i960->icount -= 35;
				t1f = get_1_rif(i960, opcode);
				t2f = get_2_rif(i960, opcode);
				set_rif(i960, opcode, t2f/t1f);
				break;

			case 0xc: // mulr
				i960->icount -= 18;
				t1f = get_1_rif(i960, opcode);
				t2f = get_2_rif(i960, opcode);
				set_rif(i960, opcode, t2f*t1f);
				break;

			case 0xd: // subr
				i960->icount -= 10;
				t1f = get_1_rif(i960, opcode);
				t2f = get_2_rif(i960, opcode);
				set_rif(i960, opcode, t2f-t1f);
				break;

			case 0xf: // addr
				i960->icount -= 10;
				t1f = get_1_rif(i960, opcode);
				t2f = get_2_rif(i960, opcode);
				set_rif(i960, opcode, t2f+t1f);
				break;

			default:
				fatalerror("I960: %x: Unhandled 78.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x79:
			switch((opcode >> 7) & 0xf) {
			case 0xb: // divrl
				i960->icount -= 77;
				t1f = get_1_rifl(i960, opcode);
				t2f = get_2_rifl(i960, opcode);
				set_rifl(i960, opcode, t2f/t1f);
				break;

			case 0xc: // mulrl
				i960->icount -= 36;
				t1f = get_1_rifl(i960, opcode);
				t2f = get_2_rifl(i960, opcode);
				set_rifl(i960, opcode, t2f*t1f);
				break;

			case 0xd: // subrl
				i960->icount -= 13;
				t1f = get_1_rifl(i960, opcode);
				t2f = get_2_rifl(i960, opcode);
				set_rifl(i960, opcode, t2f-t1f);
				break;

			case 0xf: // addrl
				i960->icount -= 13;
				t1f = get_1_rifl(i960, opcode);
				t2f = get_2_rifl(i960, opcode);
				set_rifl(i960, opcode, t2f+t1f);
				break;

			default:
				fatalerror("I960: %x: Unhandled 79.%x\n", i960->PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x80: // ldob
			i960->icount -= 4;
			i960->r[(opcode>>19)&0x1f] = i960->program->read_byte(get_ea(i960, opcode));
			break;

		case 0x82: // stob
			i960->icount -= 2;
			i960->program->write_byte(get_ea(i960, opcode), i960->r[(opcode>>19)&0x1f]);
			break;

		case 0x84: // bx
			i960->icount -= 3;
			i960->IP = get_ea(i960, opcode);
			break;

		case 0x85: // balx
			i960->icount -= 5;
			t1 = get_ea(i960, opcode);
			i960->r[(opcode>>19)&0x1f] = i960->IP;
			i960->IP = t1;
			break;

		case 0x86: // callx
			t1 = get_ea(i960, opcode);
			do_call(i960, t1, 0, i960->r[I960_SP]);
			break;

		case 0x88: // ldos
			i960->icount -= 4;
			i960->r[(opcode>>19)&0x1f] = i960_read_word_unaligned(i960, get_ea(i960, opcode));
			break;

		case 0x8a: // stos
			i960->icount -= 2;
			i960_write_word_unaligned(i960, get_ea(i960, opcode), i960->r[(opcode>>19)&0x1f]);
			break;

		case 0x8c: // lda
			i960->icount--;
			i960->r[(opcode>>19)&0x1f] = get_ea(i960, opcode);
			break;

		case 0x90: // ld
			i960->icount -= 4;
			i960->r[(opcode>>19)&0x1f] = i960_read_dword_unaligned(i960, get_ea(i960, opcode));
			break;

		case 0x92: // st
			i960->icount -= 2;
			i960_write_dword_unaligned(i960, get_ea(i960, opcode), i960->r[(opcode>>19)&0x1f]);
			break;

		case 0x98:{// ldl
			int i;
			i960->icount -= 5;
			t1 = get_ea(i960, opcode);
			t2 = (opcode>>19)&0x1e;
			i960->bursting = 1;
			for(i=0; i<2; i++) {
				i960->r[t2+i] = i960_read_dword_unaligned(i960, t1);
				if(i960->bursting)
					t1 += 4;
			}
			break;
		}

		case 0x9a:{// stl
			int i;
			i960->icount -= 3;
			t1 = get_ea(i960, opcode);
			t2 = (opcode>>19)&0x1e;
			i960->bursting = 1;
			for(i=0; i<2; i++) {
				i960_write_dword_unaligned(i960, t1, i960->r[t2+i]);
				if(i960->bursting)
					t1 += 4;
			}
			break;
		}

		case 0xa0:{// ldt
			int i;
			i960->icount -= 6;
			t1 = get_ea(i960, opcode);
			t2 = (opcode>>19)&0x1c;
			i960->bursting = 1;
			for(i=0; i<3; i++) {
				i960->r[t2+i] = i960_read_dword_unaligned(i960, t1);
				if(i960->bursting)
					t1 += 4;
			}
			break;
		}

		case 0xa2:{// stt
			int i;
			i960->icount -= 4;
			t1 = get_ea(i960, opcode);
			t2 = (opcode>>19)&0x1c;
			i960->bursting = 1;
			for(i=0; i<3; i++) {
				i960_write_dword_unaligned(i960, t1, i960->r[t2+i]);
				if(i960->bursting)
					t1 += 4;
			}
			break;
		}

		case 0xb0:{// ldq
			int i;
			i960->icount -= 7;
			t1 = get_ea(i960, opcode);
			t2 = (opcode>>19)&0x1c;
			i960->bursting = 1;
			for(i=0; i<4; i++) {
				i960->r[t2+i] = i960_read_dword_unaligned(i960, t1);
				if(i960->bursting)
					t1 += 4;
			}
			break;
		}

		case 0xb2:{// stq
			int i;
			i960->icount -= 5;
			t1 = get_ea(i960, opcode);
			t2 = (opcode>>19)&0x1c;
			i960->bursting = 1;
			for(i=0; i<4; i++) {
				i960_write_dword_unaligned(i960, t1, i960->r[t2+i]);
				if(i960->bursting)
					t1 += 4;
			}
			break;
		}

		case 0xc0: // ldib
			i960->icount -= 4;
			i960->r[(opcode>>19)&0x1f] = (INT8)i960->program->read_byte(get_ea(i960, opcode));
			break;

		case 0xc2: // stib
			i960->icount -= 2;
			i960->program->write_byte(get_ea(i960, opcode), i960->r[(opcode>>19)&0x1f]);
			break;

		case 0xc8: // ldis
			i960->icount -= 4;
			i960->r[(opcode>>19)&0x1f] = (INT16)i960_read_word_unaligned(i960, get_ea(i960, opcode));
			break;

		case 0xca: // stis
			i960->icount -= 2;
			i960_write_word_unaligned(i960, get_ea(i960, opcode), i960->r[(opcode>>19)&0x1f]);
			break;

		default:
			fatalerror("I960: %x: Unhandled %02x\n", i960->PIP, opcode >> 24);
	}

}

static CPU_EXECUTE( i960 )
{
	i960_state_t *i960 = get_safe_token(device);
	UINT32 opcode;

	check_irqs(i960);
	while(i960->icount > 0) {
		i960->PIP = i960->IP;
		debugger_instruction_hook(device, i960->IP);

		i960->bursting = 0;

		opcode = i960->direct->read_decrypted_dword(i960->IP);
		i960->IP += 4;

		execute_op(i960, opcode);
	}
}

static void set_irq_line(i960_state_t *i960, int irqline, int state)
{
	int int_tab =  i960->program->read_dword(i960->PRCB+20);	// interrupt table
	int cpu_pri = (i960->PC>>16)&0x1f;
	int vector =0;
	int priority;
	UINT32 pend, word, wordofs;

	// We support the 4 external IRQ lines in "normal" mode only.
	// The i960's interrupt support is a bit more complete than that,
	// but Namco and Sega both went for the cheapest solution.

	switch (irqline)
	{
		case I960_IRQ0:
			vector = i960->ICR & 0xff;
			break;

		case I960_IRQ1:
			vector = (i960->ICR>>8)&0xff;
			break;

		case I960_IRQ2:
			vector = (i960->ICR>>16)&0xff;
			break;

		case I960_IRQ3:
			vector = (i960->ICR>>24)&0xff;
			break;
	}

	if(!vector)
	{
		logerror("i960: interrupt line %d in IAC mode, unsupported!\n", irqline);
		return;
	}


	priority = vector / 8;

	if(state) {
		// check if we can take this "right now"
		if (((cpu_pri < priority) || (priority == 31)) && (i960->immediate_irq == 0))
		{
			i960->immediate_irq = 1;
			i960->immediate_vector = vector;
			i960->immediate_pri = priority;
		}
		else
		{
			// store the interrupt in the "pending" table
			pend = i960->program->read_dword(int_tab);
			pend |= (1 << priority);
			i960->program->write_dword(int_tab, pend);

			// now bitfield-ize the vector
			word = ((vector / 32) * 4) + 4;
			wordofs = vector % 32;
			pend = i960->program->read_dword(int_tab + word);
			pend |= (1 << wordofs);
			i960->program->write_dword(int_tab + word, pend);
		}

		// and ack it to the core now that it's queued
		(*i960->irq_cb)(i960->device, irqline);
	}
}

static CPU_SET_INFO( i960 )
{
	i960_state_t *i960 = get_safe_token(device);

	if(state >= CPUINFO_INT_REGISTER+I960_R0 && state <= CPUINFO_INT_REGISTER + I960_G15) {
		i960->r[state - (CPUINFO_INT_REGISTER + I960_R0)] = info->i;
		return;
	}

	switch(state) {
		// Interfacing
	case CPUINFO_INT_REGISTER + I960_IP:		i960->IP = info->i; 						break;
	case CPUINFO_INT_INPUT_STATE + I960_IRQ0:	set_irq_line(i960, I960_IRQ0, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + I960_IRQ1:	set_irq_line(i960, I960_IRQ1, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + I960_IRQ2:	set_irq_line(i960, I960_IRQ2, info->i);		break;
	case CPUINFO_INT_INPUT_STATE + I960_IRQ3:	set_irq_line(i960, I960_IRQ3, info->i);		break;

	default:
		fatalerror("i960_set_info %x\n", state);
	}
}

static CPU_INIT( i960 )
{
	i960_state_t *i960 = get_safe_token(device);

	i960->irq_cb = irqcallback;
	i960->device = device;
	i960->program = &device->space(AS_PROGRAM);
	i960->direct = &i960->program->direct();

	device->save_item(NAME(i960->PIP));
	device->save_item(NAME(i960->SAT));
	device->save_item(NAME(i960->PRCB));
	device->save_item(NAME(i960->PC));
	device->save_item(NAME(i960->AC));
	device->save_item(NAME(i960->ICR));
	device->save_item(NAME(i960->r));
	device->save_item(NAME(i960->fp));
	device->save_item(NAME(i960->rcache));
	device->save_item(NAME(i960->rcache_frame_addr));
}

static CPU_RESET( i960 )
{
	i960_state_t *i960 = get_safe_token(device);

	i960->SAT        = i960->program->read_dword(0);
	i960->PRCB       = i960->program->read_dword(4);
	i960->IP         = i960->program->read_dword(12);
	i960->PC         = 0x001f2002;
	i960->AC         = 0;
	i960->ICR	    = 0xff000000;
	i960->bursting   = 0;
	i960->immediate_irq = 0;

	memset(i960->r, 0, sizeof(i960->r));
	memset(i960->rcache, 0, sizeof(i960->rcache));

	i960->r[I960_FP] = i960->program->read_dword(i960->PRCB+24);
	i960->r[I960_SP] = i960->r[I960_FP] + 64;
	i960->rcache_pos = 0;
}

CPU_GET_INFO( i960 )
{
	i960_state_t *i960 = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	if(state >= CPUINFO_INT_REGISTER+I960_R0 && state <= CPUINFO_INT_REGISTER + I960_G15) {
		info->i = i960->r[state - (CPUINFO_INT_REGISTER + I960_R0)];
		return;
	}

	switch(state) {
		// Interface functions and variables
	case CPUINFO_FCT_SET_INFO:					info->setinfo     = CPU_SET_INFO_NAME(i960);	break;
	case CPUINFO_FCT_INIT:						info->init        = CPU_INIT_NAME(i960);		break;
	case CPUINFO_FCT_RESET:						info->reset       = CPU_RESET_NAME(i960);		break;
	case CPUINFO_FCT_EXIT:						info->exit        = 0;							break;
	case CPUINFO_FCT_EXECUTE:					info->execute     = CPU_EXECUTE_NAME(i960);		break;
	case CPUINFO_FCT_BURN:						info->burn        = 0;							break;
	case CPUINFO_FCT_DISASSEMBLE:				info->disassemble = CPU_DISASSEMBLE_NAME(i960);	break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:		info->icount      = &i960->icount;				break;
	case CPUINFO_INT_CONTEXT_SIZE:				info->i           = sizeof(i960_state_t);		break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:		info->i           = 4;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:		info->i           = 8;							break;

		// Bus sizes
	case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 32;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:	info->i = 0;						break;
	case CPUINFO_INT_LOGADDR_WIDTH_PROGRAM:	info->i = 0;						break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;						break;
	case CPUINFO_INT_LOGADDR_WIDTH_DATA:	info->i = 0;						break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;						break;
	case CPUINFO_INT_LOGADDR_WIDTH_IO:		info->i = 0;						break;

		// Internal maps
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map32 = NULL;break;
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map32 = NULL;break;
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_IO:		info->internal_map32 = NULL;break;

		// CPU misc parameters
	case CPUINFO_STR_NAME:					strcpy(info->s, "i960KB");							break;
	case CPUINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);							break;
	case CPUINFO_STR_FLAGS:					strcpy(info->s, i960_get_strflags(i960));			break;
	case CPUINFO_INT_ENDIANNESS:			info->i = ENDIANNESS_LITTLE;								break;
	case CPUINFO_INT_INPUT_LINES:			info->i = 4;										break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:	info->i = -1;										break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:		info->i = 1;										break;
	case CPUINFO_INT_CLOCK_DIVIDER:			info->i = 1;										break;

		// CPU main state
	case CPUINFO_INT_PC:					info->i = i960->IP;									break;
	case CPUINFO_INT_SP:					info->i = i960->r[I960_SP];							break;
	case CPUINFO_INT_PREVIOUSPC:			info->i = i960->PIP;								break;

	case CPUINFO_INT_REGISTER + I960_SAT:	info->i = i960->SAT;								break;
	case CPUINFO_INT_REGISTER + I960_PRCB:	info->i = i960->PRCB;								break;
	case CPUINFO_INT_REGISTER + I960_PC:	info->i = i960->PC;									break;
	case CPUINFO_INT_REGISTER + I960_AC:	info->i = i960->AC;									break;
	case CPUINFO_INT_REGISTER + I960_IP:	info->i = i960->IP;									break;
	case CPUINFO_INT_REGISTER + I960_PIP:	info->i = i960->PIP;								break;

		// CPU debug stuff
	case CPUINFO_STR_REGISTER + I960_SAT:	sprintf(info->s, "sat  :%08x", i960->SAT);			break;
	case CPUINFO_STR_REGISTER + I960_PRCB:	sprintf(info->s, "prcb :%08x", i960->PRCB);			break;
	case CPUINFO_STR_REGISTER + I960_PC:	sprintf(info->s, "pc   :%08x", i960->PC);			break;
	case CPUINFO_STR_REGISTER + I960_AC:	sprintf(info->s, "ac   :%08x", i960->AC);			break;
	case CPUINFO_STR_REGISTER + I960_IP:	sprintf(info->s, "ip   :%08x", i960->IP);			break;
	case CPUINFO_STR_REGISTER + I960_PIP:	sprintf(info->s, "pip  :%08x", i960->PIP);			break;

	case CPUINFO_STR_REGISTER + I960_R0:	sprintf(info->s, "pfp  :%08x", i960->r[ 0]);		break;
	case CPUINFO_STR_REGISTER + I960_R1:	sprintf(info->s, "sp   :%08x", i960->r[ 1]);		break;
	case CPUINFO_STR_REGISTER + I960_R2:	sprintf(info->s, "rip  :%08x", i960->r[ 2]);		break;
	case CPUINFO_STR_REGISTER + I960_R3:	sprintf(info->s, "r3   :%08x", i960->r[ 3]);		break;
	case CPUINFO_STR_REGISTER + I960_R4:	sprintf(info->s, "r4   :%08x", i960->r[ 4]);		break;
	case CPUINFO_STR_REGISTER + I960_R5:	sprintf(info->s, "r5   :%08x", i960->r[ 5]);		break;
	case CPUINFO_STR_REGISTER + I960_R6:	sprintf(info->s, "r6   :%08x", i960->r[ 6]);		break;
	case CPUINFO_STR_REGISTER + I960_R7:	sprintf(info->s, "r7   :%08x", i960->r[ 7]);		break;
	case CPUINFO_STR_REGISTER + I960_R8:	sprintf(info->s, "r8   :%08x", i960->r[ 8]);		break;
	case CPUINFO_STR_REGISTER + I960_R9:	sprintf(info->s, "r9   :%08x", i960->r[ 9]);		break;
	case CPUINFO_STR_REGISTER + I960_R10:	sprintf(info->s, "r10  :%08x", i960->r[10]);		break;
	case CPUINFO_STR_REGISTER + I960_R11:	sprintf(info->s, "r11  :%08x", i960->r[11]);		break;
	case CPUINFO_STR_REGISTER + I960_R12:	sprintf(info->s, "r12  :%08x", i960->r[12]);		break;
	case CPUINFO_STR_REGISTER + I960_R13:	sprintf(info->s, "r13  :%08x", i960->r[13]);		break;
	case CPUINFO_STR_REGISTER + I960_R14:	sprintf(info->s, "r14  :%08x", i960->r[14]);		break;
	case CPUINFO_STR_REGISTER + I960_R15:	sprintf(info->s, "r15  :%08x", i960->r[15]);		break;

	case CPUINFO_STR_REGISTER + I960_G0:	sprintf(info->s, "g0   :%08x", i960->r[16]);		break;
	case CPUINFO_STR_REGISTER + I960_G1:	sprintf(info->s, "g1   :%08x", i960->r[17]);		break;
	case CPUINFO_STR_REGISTER + I960_G2:	sprintf(info->s, "g2   :%08x", i960->r[18]);		break;
	case CPUINFO_STR_REGISTER + I960_G3:	sprintf(info->s, "g3   :%08x", i960->r[19]);		break;
	case CPUINFO_STR_REGISTER + I960_G4:	sprintf(info->s, "g4   :%08x", i960->r[20]);		break;
	case CPUINFO_STR_REGISTER + I960_G5:	sprintf(info->s, "g5   :%08x", i960->r[21]);		break;
	case CPUINFO_STR_REGISTER + I960_G6:	sprintf(info->s, "g6   :%08x", i960->r[22]);		break;
	case CPUINFO_STR_REGISTER + I960_G7:	sprintf(info->s, "g7   :%08x", i960->r[23]);		break;
	case CPUINFO_STR_REGISTER + I960_G8:	sprintf(info->s, "g8   :%08x", i960->r[24]);		break;
	case CPUINFO_STR_REGISTER + I960_G9:	sprintf(info->s, "g9   :%08x", i960->r[25]);		break;
	case CPUINFO_STR_REGISTER + I960_G10:	sprintf(info->s, "g10  :%08x", i960->r[26]);		break;
	case CPUINFO_STR_REGISTER + I960_G11:	sprintf(info->s, "g11  :%08x", i960->r[27]);		break;
	case CPUINFO_STR_REGISTER + I960_G12:	sprintf(info->s, "g12  :%08x", i960->r[28]);		break;
	case CPUINFO_STR_REGISTER + I960_G13:	sprintf(info->s, "g13  :%08x", i960->r[29]);		break;
	case CPUINFO_STR_REGISTER + I960_G14:	sprintf(info->s, "g14  :%08x", i960->r[30]);		break;
	case CPUINFO_STR_REGISTER + I960_G15:	sprintf(info->s, "fp   :%08x", i960->r[31]);		break;

//  default:
//      fatalerror("i960_get_info %x          \n", state);
	}
}

// call from any read/write handler for a memory area that can't be bursted
// on the real hardware (e.g. Model 2's interrupt control registers)
void i960_noburst(device_t *device)
{
	i960_state_t *i960 = get_safe_token(device);
	i960->bursting = 0;
}

void i960_stall(device_t *device)
{
	i960_state_t *i960 = get_safe_token(device);
	i960->IP = i960->PIP;
}

DEFINE_LEGACY_CPU_DEVICE(I960, i960);
