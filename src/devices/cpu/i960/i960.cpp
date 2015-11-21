// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
#include "emu.h"
#include "debugger.h"
#include "i960.h"

CPU_DISASSEMBLE( i960  );

#ifdef _MSC_VER
/* logb prototype is different for MS Visual C */
#include <float.h>
#define logb _logb
#endif


const device_type I960 = &device_creator<i960_cpu_device>;


i960_cpu_device::i960_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, I960, "i960kb", tag, owner, clock, "i960kb", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0), m_rcache_pos(0), m_SAT(0), m_PRCB(0), m_PC(0), m_AC(0), m_IP(0), m_PIP(0), m_ICR(0), m_bursting(0), m_immediate_irq(0), 
	m_immediate_vector(0), m_immediate_pri(0), m_program(nullptr), m_direct(nullptr), m_icount(0)
{
}


UINT32 i960_cpu_device::i960_read_dword_unaligned(UINT32 address)
{
	if (address & 3)
		return m_program->read_byte(address) | m_program->read_byte(address+1)<<8 | m_program->read_byte(address+2)<<16 | m_program->read_byte(address+3)<<24;
	else
		return m_program->read_dword(address);
}

UINT16 i960_cpu_device::i960_read_word_unaligned(UINT32 address)
{
	if (address & 1)
		return m_program->read_byte(address) | m_program->read_byte(address+1)<<8;
	else
		return m_program->read_word(address);
}

void i960_cpu_device::i960_write_dword_unaligned(UINT32 address, UINT32 data)
{
	if (address & 3)
	{
		m_program->write_byte(address, data & 0xff);
		m_program->write_byte(address+1, (data>>8)&0xff);
		m_program->write_byte(address+2, (data>>16)&0xff);
		m_program->write_byte(address+3, (data>>24)&0xff);
	}
	else
	{
		m_program->write_dword(address, data);
	}
}

void i960_cpu_device::i960_write_word_unaligned(UINT32 address, UINT16 data)
{
	if (address & 1)
	{
		m_program->write_byte(address, data & 0xff);
		m_program->write_byte(address+1, (data>>8)&0xff);
	}
	else
	{
		m_program->write_word(address, data);
	}
}

void i960_cpu_device::send_iac(UINT32 adr)
{
	UINT32 iac[4];
	iac[0] = m_program->read_dword(adr);
	iac[1] = m_program->read_dword(adr+4);
	iac[2] = m_program->read_dword(adr+8);
	iac[3] = m_program->read_dword(adr+12);

	switch(iac[0]>>24) {
	case 0x93: // reinit
		m_SAT  = iac[1];
		m_PRCB = iac[2];
		m_IP   = iac[3];
		break;
	default:
		fatalerror("I960: %x: IAC %08x %08x %08x %08x\n", m_PIP, iac[0], iac[1], iac[2], iac[3]);
	}
}

UINT32 i960_cpu_device::get_ea(UINT32 opcode)
{
	int abase = (opcode >> 14) & 0x1f;
	if(!(opcode & 0x00001000)) { // MEMA
		UINT32 offset = opcode & 0x1fff;
		if(!(opcode & 0x2000))
			return offset;
		else
			return m_r[abase]+offset;
	} else {                     // MEMB
		int index = opcode & 0x1f;
		int scale = (opcode >> 7) & 0x7;
		int mode  = (opcode >> 10) & 0xf;
		UINT32 ret;

		switch(mode) {
		case 0x4:
			return m_r[abase];

		case 0x7:
			return m_r[abase] + (m_r[index] << scale);

		case 0xc:
			ret = m_direct->read_dword(m_IP);
			m_IP += 4;
			return ret;

		case 0xd:
			ret = m_direct->read_dword(m_IP) + m_r[abase];
			m_IP += 4;
			return ret;

		case 0xe:
			ret = m_direct->read_dword(m_IP) + (m_r[index] << scale);
			m_IP += 4;
			return ret;

		case 0xf:
			ret = m_direct->read_dword(m_IP) + m_r[abase] + (m_r[index] << scale);
			m_IP += 4;
			return ret;

		default:
			fatalerror("I960: %x: unhandled MEMB mode %x\n", m_PIP, mode);
		}
	}
}

UINT32 i960_cpu_device::get_1_ri(UINT32 opcode)
{
	if(!(opcode & 0x00000800))
		return m_r[opcode & 0x1f];
	else
		return opcode & 0x1f;
}

UINT32 i960_cpu_device::get_2_ri(UINT32 opcode)
{
	if(!(opcode & 0x00001000))
		return m_r[(opcode>>14) & 0x1f];
	else
		return (opcode>>14) & 0x1f;
}

UINT64 i960_cpu_device::get_2_ri64(UINT32 opcode)
{
	if(!(opcode & 0x00001000))
		return m_r[(opcode>>14) & 0x1f] | ((UINT64)m_r[((opcode>>14) & 0x1f)+1]<<32);
	else
		return (opcode>>14) & 0x1f;
}

void i960_cpu_device::set_ri(UINT32 opcode, UINT32 val)
{
	if(!(opcode & 0x00002000))
		m_r[(opcode>>19) & 0x1f] = val;
	else {
		fatalerror("I960: %x: set_ri on literal?\n", m_PIP);
	}
}

void i960_cpu_device::set_ri2(UINT32 opcode, UINT32 val, UINT32 val2)
{
	if(!(opcode & 0x00002000))
	{
		m_r[(opcode>>19) & 0x1f] = val;
		m_r[((opcode>>19) & 0x1f)+1] = val2;
	}
	else {
		fatalerror("I960: %x: set_ri2 on literal?\n", m_PIP);
	}
}

void i960_cpu_device::set_ri64(UINT32 opcode, UINT64 val)
{
	if(!(opcode & 0x00002000)) {
		m_r[(opcode>>19) & 0x1f] = val;
		m_r[((opcode>>19) & 0x1f)+1] = val >> 32;
	} else
		fatalerror("I960: %x: set_ri64 on literal?\n", m_PIP);
}

double i960_cpu_device::get_1_rif(UINT32 opcode)
{
	if(!(opcode & 0x00000800))
		return u2f(m_r[opcode & 0x1f]);
	else {
		int idx = opcode & 0x1f;
		if(idx < 4)
			return m_fp[idx];
		if(idx == 0x16)
			return 1.0;
		return 0.0;
	}
}

double i960_cpu_device::get_2_rif(UINT32 opcode)
{
	if(!(opcode & 0x00001000))
		return u2f(m_r[(opcode>>14) & 0x1f]);
	else {
		int idx = (opcode>>14) & 0x1f;
		if(idx < 4)
			return m_fp[idx];
		if(idx == 0x16)
			return 1.0;
		return 0.0;
	}
}

void i960_cpu_device::set_rif(UINT32 opcode, double val)
{
	if(!(opcode & 0x00002000))
		m_r[(opcode>>19) & 0x1f] = f2u(val);
	else if(!(opcode & 0x00e00000))
		m_fp[(opcode>>19) & 3] = val;
	else
		fatalerror("I960: %x: set_rif on literal?\n", m_PIP);
}

double i960_cpu_device::get_1_rifl(UINT32 opcode)
{
	if(!(opcode & 0x00000800)) {
		UINT64 v = m_r[opcode & 0x1e];
		v |= ((UINT64)(m_r[(opcode & 0x1e)+1]))<<32;
		return u2d(v);
	} else {
		int idx = opcode & 0x1f;
		if(idx < 4)
			return m_fp[idx];
		if(idx == 0x16)
			return 1.0;
		return 0.0;
	}
}

double i960_cpu_device::get_2_rifl(UINT32 opcode)
{
	if(!(opcode & 0x00001000)) {
		UINT64 v = m_r[(opcode >> 14) & 0x1e];
		v |= ((UINT64)(m_r[((opcode>>14) & 0x1e)+1]))<<32;
		return u2d(v);
	} else {
		int idx = (opcode>>14) & 0x1f;
		if(idx < 4)
			return m_fp[idx];
		if(idx == 0x16)
			return 1.0;
		return 0.0;
	}
}

void i960_cpu_device::set_rifl(UINT32 opcode, double val)
{
	if(!(opcode & 0x00002000)) {
		UINT64 v = d2u(val);
		m_r[(opcode>>19) & 0x1e] = v;
		m_r[((opcode>>19) & 0x1e)+1] = v>>32;
	} else if(!(opcode & 0x00e00000))
		m_fp[(opcode>>19) & 3] = val;
	else
		fatalerror("I960: %x: set_rifl on literal?\n", m_PIP);
}

UINT32 i960_cpu_device::get_1_ci(UINT32 opcode)
{
	if(!(opcode & 0x00002000))
		return m_r[(opcode >> 19) & 0x1f];
	else
		return (opcode >> 19) & 0x1f;
}

UINT32 i960_cpu_device::get_2_ci(UINT32 opcode)
{
	return m_r[(opcode >> 14) & 0x1f];
}

UINT32 i960_cpu_device::get_disp(UINT32 opcode)
{
	UINT32 disp;
	disp = opcode & 0xffffff;
	if(disp & 0x00800000)
		disp |= 0xff000000;
	return disp-4;
}

UINT32 i960_cpu_device::get_disp_s(UINT32 opcode)
{
	UINT32 disp;
	disp = opcode & 0x1fff;
	if(disp & 0x00001000)
		disp |= 0xffffe000;
	return disp-4;
}

void i960_cpu_device::cmp_s(INT32 v1, INT32 v2)
{
	m_AC &= ~7;
	if(v1<v2)
		m_AC |= 4;
	else if(v1 == v2)
		m_AC |= 2;
	else
		m_AC |= 1;
}

void i960_cpu_device::cmp_u(UINT32 v1, UINT32 v2)
{
	m_AC &= ~7;
	if(v1<v2)
		m_AC |= 4;
	else if(v1 == v2)
		m_AC |= 2;
	else
		m_AC |= 1;
}

void i960_cpu_device::concmp_s(INT32 v1, INT32 v2)
{
	m_AC &= ~7;
	if(v1 <= v2)
		m_AC |= 2;
	else
		m_AC |= 1;
}

void i960_cpu_device::concmp_u(UINT32 v1, UINT32 v2)
{
	m_AC &= ~7;
	if(v1 <= v2)
		m_AC |= 2;
	else
		m_AC |= 1;
}

void i960_cpu_device::cmp_d(double v1, double v2)
{
	m_AC &= ~7;
	if(v1<v2)
		m_AC |= 4;
	else if(v1 == v2)
		m_AC |= 2;
	else if(v1 > v2)
		m_AC |= 1;
}

void i960_cpu_device::bxx(UINT32 opcode, int mask)
{
	if(m_AC & mask) {
		m_IP += get_disp(opcode);
	}
}

void i960_cpu_device::bxx_s(UINT32 opcode, int mask)
{
	if(m_AC & mask) {
		m_IP += get_disp_s(opcode);
	}
}

void i960_cpu_device::test(UINT32 opcode, int mask)
{
	if(m_AC & mask)
		m_r[(opcode>>19) & 0x1f] = 1;
	else
		m_r[(opcode>>19) & 0x1f] = 0;
}


// interrupt dispatch
void i960_cpu_device::take_interrupt(int vector, int lvl)
{
	int int_tab =  m_program->read_dword(m_PRCB+20);    // interrupt table
	int int_SP  =  m_program->read_dword(m_PRCB+24);    // interrupt stack
	int SP;
	UINT32 IRQV;

	IRQV = m_program->read_dword(int_tab + 36 + (vector-8)*4);

	// start the process
	if(!(m_PC & 0x2000))    // if this is a nested interrupt, don't re-get int_SP
	{
		SP = int_SP;
	}
	else
	{
		SP = m_r[I960_SP];
	}

	SP = (SP + 63) & ~63;
	SP += 128;  // emulate ElSemi's core, this fixes the crash in sonic the fighters

	do_call(IRQV, 7, SP);

	// save the processor state
	m_program->write_dword(m_r[I960_FP]-16, m_PC);
	m_program->write_dword(m_r[I960_FP]-12, m_AC);
	// store the vector
	m_program->write_dword(m_r[I960_FP]-8, vector-8);

	m_PC &= ~0x1f00;    // clear priority, state, trace-fault pending, and trace enable
	m_PC |= (lvl<<16);  // set CPU level to current IRQ level
	m_PC |= 0x2002; // set supervisor mode & interrupt flag
}

void i960_cpu_device::check_irqs()
{
	int int_tab =  m_program->read_dword(m_PRCB+20);    // interrupt table
	int cpu_pri = (m_PC>>16)&0x1f;
	int pending_pri;
	int lvl, irq, take = -1;
	int vword;
	static const UINT32 lvlmask[4] = { 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

	pending_pri = m_program->read_dword(int_tab);       // read pending priorities

	if ((m_immediate_irq) && ((cpu_pri < m_immediate_pri) || (m_immediate_pri == 31)))
	{
		take_interrupt(m_immediate_vector, m_immediate_pri);
		m_immediate_irq = 0;
	}
	else
	{
		for(lvl = 31; lvl >= 0; lvl--) {
			if((pending_pri & (1 << lvl)) && ((cpu_pri < lvl) || (lvl == 31))) {
				int word, wordl, wordh;

				// figure out which word contains this level's priorities
				word = ((lvl / 4) * 4) + 4; // (lvl/4) = word address, *4 for byte address, +4 to skip pending priorities
				wordl = (lvl % 4) * 8;
				wordh = (wordl + 8) - 1;

				vword = m_program->read_dword(int_tab + word);

				// take the first vector we find for this level
				for (irq = wordh; irq >= wordl; irq--) {
					if(vword & (1 << irq)) {
						// clear pending bit
						vword &= ~(1 << irq);
						m_program->write_dword(int_tab + word, vword);
						take = irq;
						break;
					}
				}

				// if no vectors were found at our level, it's an error
				if(take == -1) {
					logerror("i960: ERROR! no vector found for pending level %d\n", lvl);

					// try to recover...
					pending_pri &= ~(1 << lvl);
					m_program->write_dword(int_tab, pending_pri);
					return;
				}

				// if no vectors are waiting for this level, clear the level bit
				if(!(vword & lvlmask[lvl % 4])) {
					pending_pri &= ~(1 << lvl);
					m_program->write_dword(int_tab, pending_pri);
				}

				take += ((lvl/4) * 32);

				take_interrupt(take, lvl);
				return;
			}
		}
	}
}

void i960_cpu_device::do_call(UINT32 adr, int type, UINT32 stack)
{
	int i;
	UINT32 FP;

	// call and callx take 9 cycles base
	m_icount -= 9;

	// set the new RIP
	m_r[I960_RIP] = m_IP;
//  osd_printf_debug("CALL (type %d): FP %x, %x => %x, stack %x, rcache_pos %d\n", type, m_r[I960_FP], m_r[I960_RIP], adr, stack, m_rcache_pos);

	// are we out of cache entries?
	if (m_rcache_pos >= I960_RCACHE_SIZE) {
		// flush the current register set to the current frame
		FP = m_r[I960_FP] & ~0x3f;
		for (i = 0; i < 16; i++) {
			m_program->write_dword(FP + (i*4), m_r[i]);
		}
	}
	else    // a cache entry is available, use it
	{
		memcpy(&m_rcache[m_rcache_pos][0], m_r, 0x10 * sizeof(UINT32));
		m_rcache_frame_addr[m_rcache_pos] = m_r[I960_FP] & ~0x3f;
	}
	m_rcache_pos++;

	m_IP = adr;
	m_r[I960_PFP] = m_r[I960_FP] & ~7;
	m_r[I960_PFP] |= type;

	if(type == 7) { // interrupts need special handling
		// set the stack to the passed-in value to properly handle nested interrupts
		// (can't set it externally or the original program's SP will be lost)
		m_r[I960_SP] = stack;
	}

	m_r[I960_FP]  = (m_r[I960_SP] + 63) & ~63;
	m_r[I960_SP]  = m_r[I960_FP] + 64;
}

void i960_cpu_device::do_ret_0()
{
//  int type = m_r[I960_PFP] & 7;

	m_r[I960_FP] = m_r[I960_PFP] & ~0x3f;

	m_rcache_pos--;

	// normal situation: if we're still above rcache size, we're not in cache.
	// abnormal situation (after the app does a FLUSHREG): rcache_pos will be 0
	// coming in, but we must still treat it as a not-in-cache situation.
	if ((m_rcache_pos >= I960_RCACHE_SIZE) || (m_rcache_pos < 0))
	{
		int i;
		for(i=0; i<0x10; i++)
			m_r[i] = m_program->read_dword(m_r[I960_FP]+4*i);

		if (m_rcache_pos < 0)
		{
			m_rcache_pos = 0;
		}
	}
	else
	{
		memcpy(m_r, m_rcache[m_rcache_pos], 0x10*sizeof(UINT32));
	}

//  osd_printf_debug("RET (type %d): FP %x, %x => %x, rcache_pos %d\n", type, m_r[I960_FP], m_IP, m_r[I960_RIP], m_rcache_pos);
	m_IP = m_r[I960_RIP];
}

void i960_cpu_device::do_ret()
{
	UINT32 x, y;
	m_icount -= 7;
	switch(m_r[I960_PFP] & 7) {
	case 0:
		do_ret_0();
		break;

	case 7:
		x = m_program->read_dword(m_r[I960_FP]-16);
		y = m_program->read_dword(m_r[I960_FP]-12);
		do_ret_0();
		m_AC = y;
		// #### test supervisor
		m_PC = x;

		// check for another IRQ now that we're back
		check_irqs();
		break;

	default:
		fatalerror("I960: %x: Unsupported return mode %d\n", m_PIP, m_r[I960_PFP] & 7);
	}
}

void i960_cpu_device::execute_op(UINT32 opcode)
{
	UINT32 t1, t2;
	double t1f, t2f;

	switch(opcode >> 24) {
		case 0x08: // b
			m_icount--;
			m_IP += get_disp(opcode);
			break;

		case 0x09: // call
			do_call(m_IP+get_disp(opcode), 0, m_r[I960_SP]);
			break;

		case 0x0a: // ret
			do_ret();
			break;

		case 0x0b: // bal
			m_icount -= 5;
			m_r[0x1e] = m_IP;
			m_IP += get_disp(opcode);
			break;

		case 0x10: // bno
			m_icount--;
			if(!(m_AC & 7)) {
				m_IP += get_disp(opcode);
			}
			break;

		case 0x11: // bg
			m_icount--;
			bxx(opcode, 1);
			break;

		case 0x12: // be
			m_icount--;
			bxx(opcode, 2);
			break;

		case 0x13: // bge
			m_icount--;
			bxx(opcode, 3);
			break;

		case 0x14: // bl
			m_icount--;
			bxx(opcode, 4);
			break;

		case 0x15: // bne
			m_icount--;
			bxx(opcode, 5);
			break;

		case 0x16: // ble
			m_icount--;
			bxx(opcode, 6);
			break;

		case 0x17: // bo
			m_icount--;
			bxx(opcode, 7);
			break;

		case 0x20: // testno
			m_icount--;
			if(!(m_AC & 7))
				m_r[(opcode>>19) & 0x1f] = 1;
			else
				m_r[(opcode>>19) & 0x1f] = 0;
			break;

		case 0x21: // testg
			m_icount--;
			test(opcode, 1);
			break;

		case 0x22: // teste
			m_icount--;
			test(opcode, 2);
			break;

		case 0x23: // testge
			m_icount--;
			test(opcode, 3);
			break;

		case 0x24: // testl
			m_icount--;
			test(opcode, 4);
			break;

		case 0x25: // testne
			m_icount--;
			test(opcode, 5);
			break;

		case 0x26: // testle
			m_icount--;
			test(opcode, 6);
			break;

		case 0x27: // testo
			m_icount--;
			test(opcode, 7);
			break;

		case 0x30: // bbc
			m_icount -= 4;
			t1 = get_1_ci(opcode) & 0x1f;
			t2 = get_2_ci(opcode);
			if(!(t2 & (1<<t1))) {
				m_AC = (m_AC & ~7) | 2;
				m_IP += get_disp_s(opcode);
			} else
				m_AC &= ~7;
			break;

		case 0x31: // cmp0bg
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_u(t1, t2);
			bxx_s(opcode, 1);
			break;

		case 0x32: // cmpobe
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_u(t1, t2);
			bxx_s(opcode, 2);
			break;

		case 0x33: // cmpobge
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_u(t1, t2);
			bxx_s(opcode, 3);
			break;

		case 0x34: // cmpobl
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_u(t1, t2);
			bxx_s(opcode, 4);
			break;

		case 0x35: // cmpobne
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_u(t1, t2);
			bxx_s(opcode, 5);
			break;

		case 0x36: // cmpoble
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_u(t1, t2);
			bxx_s(opcode, 6);
			break;

		case 0x37: // bbs
			m_icount -= 4;
			t1 = get_1_ci(opcode) & 0x1f;
			t2 = get_2_ci(opcode);
			if(t2 & (1<<t1)) {
				m_AC = (m_AC & ~7) | 2;
				m_IP += get_disp_s(opcode);
			} else
				m_AC &= ~7;
			break;

		case 0x39: // cmpibg
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_s(t1, t2);
			bxx_s(opcode, 1);
			break;

		case 0x3a: // cmpibe
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_s(t1, t2);
			bxx_s(opcode, 2);
			break;

		case 0x3b: // cmpibge
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_s(t1, t2);
			bxx_s(opcode, 3);
			break;

		case 0x3c: // cmpibl
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_s(t1, t2);
			bxx_s(opcode, 4);
			break;

		case 0x3d: // cmpibne
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_s(t1, t2);
			bxx_s(opcode, 5);
			break;

		case 0x3e: // cmpible
			m_icount -= 4;
			t1 = get_1_ci(opcode);
			t2 = get_2_ci(opcode);
			cmp_s(t1, t2);
			bxx_s(opcode, 6);
			break;

		case 0x58:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // notbit
				m_icount -= 2;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2 ^ (1<<(t1 & 31)));
				break;

			case 0x1: // and
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2 & t1);
				break;

			case 0x2: // andnot
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2 & ~t1);
				break;

			case 0x3: // setbit
				m_icount -= 2;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2 | (1<<(t1 & 31)));
				break;

			case 0x4: // notand
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, (~t2) & t1);
				break;

			case 0x6: // xor
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2 ^ t1);
				break;

			case 0x7: // or
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2 | t1);
				break;

			case 0x8: // nor
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, ((~t2) & (~t1)));
				break;

			case 0x9: // xnor
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, ~(t2 ^ t1));
				break;

			case 0xa: // not
				m_icount--;
				t1 = get_1_ri(opcode);
				set_ri(opcode, ~t1);
				break;

			case 0xb: // ornot
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2 | ~t1);
				break;

			case 0xc: // clrbit
				m_icount -= 2;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2 & ~(1<<(t1 & 31)));
				break;

			case 0xd: // notor
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, (~t2) | t1);
				break;

			case 0xe: // nand
				m_icount -= 2;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, ~t2 | ~t1);
				break;

			case 0xf: // alterbit
				m_icount -= 2;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				if(m_AC & 2)
					set_ri(opcode, t2 | (1<<(t1 & 31)));
				else
					set_ri(opcode, t2 & ~(1<<(t1 & 31)));
				break;

			default:
				fatalerror("I960: %x: Unhandled 58.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x59:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // addo
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2+t1);
				break;

			case 0x1: // addi
				// #### overflow
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2+t1);
				break;

			case 0x2: // subo
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2-t1);
				break;

			case 0x3: // subi
				// #### overflow
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2-t1);
				break;

			case 0x8: // shro
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2>>t1);
				break;

			case 0xa: // shrdi
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				if(((INT32)t2) < 0) {
					if(t2 & ((1<<t1)-1))
						set_ri(opcode, (((INT32)t2)>>t1)+1);
					else
						set_ri(opcode, ((INT32)t2)>>t1);
				} else
					set_ri(opcode, t2>>t1);
				break;

			case 0xb: // shri
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, ((INT32)t2)>>t1);
				break;

			case 0xc: // shlo
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2<<t1);
				break;

			case 0xd: // rotate
				m_icount--;
				t1 = get_1_ri(opcode) & 0x1f;
				t2 = get_2_ri(opcode);
				set_ri(opcode, (t2<<t1)|(t2>>(32-t1)));
				break;

			case 0xe: // shli
				// missing overflow
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2<<t1);
				break;

			default:
				fatalerror("I960: %x: Unhandled 59.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5a:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // cmpo
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				cmp_u(t1, t2);
				break;

			case 0x1: // cmpi
				m_icount--;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				cmp_s(t1, t2);
				break;

			case 0x2: // concmpo
				m_icount--;
				if(!(m_AC & 0x4)) {
					t1 = get_1_ri(opcode);
					t2 = get_2_ri(opcode);
					concmp_u(t1, t2);
				}
				break;

			case 0x3: // concmpi
				m_icount--;
				if(!(m_AC & 0x4)) {
					t1 = get_1_ri(opcode);
					t2 = get_2_ri(opcode);
					concmp_s(t1, t2);
				}
				break;

			case 0x4: // cmpinco
				m_icount -= 2;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				cmp_u(t1, t2);
				set_ri(opcode, t2+1);
				break;

			case 0x5: // cmpinci
				m_icount -= 2;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				cmp_s(t1, t2);
				set_ri(opcode, t2+1);
				break;

			case 0x6: // cmpdeco
				m_icount -= 2;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				cmp_u(t1, t2);
				set_ri(opcode, t2-1);
				break;

			case 0x7: // cmpdeci
				m_icount -= 2;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				cmp_s(t1, t2);
				set_ri(opcode, t2-1);
				break;

			case 0xe: // chkbit
				m_icount -= 2;
				t1 = get_1_ri(opcode) & 0x1f;
				t2 = get_2_ri(opcode);
				if(t2 & (1<<t1))
					m_AC = (m_AC & ~7) | 2;
				else
					m_AC &= ~7;
				break;

			default:
				fatalerror("I960: %x: Unhandled 5a.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5b:
			switch((opcode >> 7) & 0xf) {
			case 0x0:   // addc
				{
					UINT64 res;

					m_icount -= 2;
					t1 = get_1_ri(opcode);
					t2 = get_2_ri(opcode);
					res = t2+(t1+((m_AC>>1)&1));
					set_ri(opcode, res&0xffffffff);

					m_AC &= ~0x3;   // clear C and V
					// set carry
					m_AC |= ((res) & (((UINT64)1) << 32)) ? 0x2 : 0;
					// set overflow
					m_AC |= (((res) ^ (t1)) & ((res) ^ (t2)) & 0x80000000) ? 1: 0;
				}
				break;

			case 0x2:   // subc
				{
					UINT64 res;

					m_icount -= 2;
					t1 = get_1_ri(opcode);
					t2 = get_2_ri(opcode);
					res = t2-(t1+((m_AC>>1)&1));
					set_ri(opcode, res&0xffffffff);

					m_AC &= ~0x3;   // clear C and V
					// set carry
					m_AC |= ((res) & (((UINT64)1) << 32)) ? 0x2 : 0;
					// set overflow
					m_AC |= (((t2) ^ (t1)) & ((t2) ^ (res)) & 0x80000000) ? 1 : 0;
				}
				break;

			default:
				fatalerror("I960: %x: Unhandled 5b.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5c:
			switch((opcode >> 7) & 0xf) {
			case 0xc: // mov
				m_icount -= 2;
				t1 = get_1_ri(opcode);
				set_ri(opcode, t1);
				break;

			default:
				fatalerror("I960: %x: Unhandled 5c.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5d:
			switch((opcode >> 7) & 0xf) {
			case 0xc: // movl
				m_icount -= 2;
				t2 = (opcode>>19) & 0x1e;
				if(opcode & 0x00000800) { // litteral
					t1 = opcode & 0x1f;
					m_r[t2] = m_r[t2+1] = t1;
				} else
					memcpy(m_r+t2, m_r+(opcode & 0x1f), 2*sizeof(UINT32));
				break;

			default:
				fatalerror("I960: %x: Unhandled 5d.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5e:
			switch((opcode >> 7) & 0xf) {
			case 0xc: // movt
				m_icount -= 3;
				t2 = (opcode>>19) & 0x1c;
				if(opcode & 0x00000800) { // litteral
					t1 = opcode & 0x1f;
					m_r[t2] = m_r[t2+1] = m_r[t2+2]= t1;
				} else
					memcpy(m_r+t2, m_r+(opcode & 0x1f), 3*sizeof(UINT32));
				break;

			default:
				fatalerror("I960: %x: Unhandled 5e.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x5f:
			switch((opcode >> 7) & 0xf) {
			case 0xc: // movq
				m_icount -= 4;
				t2 = (opcode>>19) & 0x1c;
				if(opcode & 0x00000800) { // litteral
					t1 = opcode & 0x1f;
					m_r[t2] = m_r[t2+1] = m_r[t2+2] = m_r[t2+3] = t1;
				} else
					memcpy(m_r+t2, m_r+(opcode & 0x1f), 4*sizeof(UINT32));
				break;

			default:
				fatalerror("I960: %x: Unhandled 5f.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x60:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // synmov
				m_icount -= 6;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				// interrupt control register
				if(t1 == 0xff000004)
					m_ICR = m_program->read_dword(t2);
				else
					m_program->write_dword(t1,    m_program->read_dword(t2));
				m_AC = (m_AC & ~7) | 2;
				break;

			case 0x2: // synmovq
				m_icount -= 12;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				if(t1 == 0xff000010)
					send_iac(t2);
				else {
					m_program->write_dword(t1,    m_program->read_dword(t2));
					m_program->write_dword(t1+4,  m_program->read_dword(t2+4));
					m_program->write_dword(t1+8,  m_program->read_dword(t2+8));
					m_program->write_dword(t1+12, m_program->read_dword(t2+12));
				}
				m_AC = (m_AC & ~7) | 2;
				break;

			default:
				fatalerror("I960: %x: Unhandled 60.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x64:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // spanbit
				{
					UINT32 res = 0xffffffff;
					int i;

					m_icount -= 10;

					t1 = get_1_ri(opcode);
					m_AC &= ~7;

					for (i = 31; i >= 0; i--)
					{
						if (!(t1 & (1<<i)))
						{
							m_AC |= 2;
							res = i;
							break;
						}
					}

					set_ri(opcode, res);
				}
				break;

			case 0x1: // scanbit
				{
					UINT32 res = 0xffffffff;
					int i;

					m_icount -= 10;

					t1 = get_1_ri(opcode);
					m_AC &= ~7;

					for (i = 31; i >= 0; i--)
					{
						if (t1 & (1<<i))
						{
							m_AC |= 2;
							res = i;
							break;
						}
					}

					set_ri(opcode, res);
				}
				break;

			case 0x5: // modac
				m_icount -= 10;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, m_AC);
				m_AC = (m_AC & ~t1) | (t2 & t1);
				break;

			default:
				fatalerror("I960: %x: Unhandled 64.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x65:
			switch((opcode >> 7) & 0xf) {
			case 0x5: // modpc
				m_icount -= 10;
				t1 = m_PC;
				t2 = get_2_ri(opcode);
				m_PC = (m_PC & ~t2) | (m_r[(opcode>>19) & 0x1f] & t2);
				set_ri(opcode, t1);
				break;

			default:
				fatalerror("I960: %x: Unhandled 65.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x66:
			switch((opcode >> 7) & 0xf) {
			case 0xd: // flushreg
				if (m_rcache_pos > 4)
				{
					m_rcache_pos = 4;
				}
				for(t1=0; t1 < m_rcache_pos; t1++)
				{
					int i;

					for (i = 0; i < 0x10; i++)
					{
						m_program->write_dword(m_rcache_frame_addr[t1] + (i * sizeof(UINT32)), m_rcache[t1][i]);
					}
				}
				m_rcache_pos = 0;
				break;

			default:
				fatalerror("I960: %x: Unhandled 66.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x67:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // emul
				m_icount -= 37;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);

				set_ri64(opcode, (INT64)t1 * (INT64)t2);
				break;

			case 0x1: // ediv
				m_icount -= 37;
				{
					UINT64 src1, src2;

					src1 = get_1_ri(opcode);
					src2 = get_2_ri64(opcode);

					set_ri2(opcode, src2 % src1, src2 / src1);
				}
				break;

			case 0x4: // cvtir
				m_icount -= 30;
				t1 = get_1_ri(opcode);
				set_rif(opcode, (double)(INT32)t1);
				break;

			case 0x5: // cvtilr
				m_icount -= 30;
				t1 = get_1_ri(opcode);
				set_rifl(opcode, (double)(INT32)t1);
				break;

			case 0x6: // scalerl
				m_icount -= 30;
				t1 = get_1_ri(opcode);
				t2f = get_2_rifl(opcode);
				set_rifl(opcode, t2f * pow(2.0, (double)(INT32)t1));
				break;

			case 0x7: // scaler
				m_icount -= 30;
				t1 = get_1_ri(opcode);
				t2f = get_2_rif(opcode);
			set_rif(opcode, t2f * pow(2.0, (double)(INT32)t1));
				break;

			default:
				fatalerror("I960: %x: Unhandled 67.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x68:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // atanr
				m_icount -= 267;
				t1f = get_1_rif(opcode);
				t2f = get_2_rif(opcode);
				set_rif(opcode, atan2(t2f, t1f));
				break;

			case 0x1: // logepr
				m_icount -= 400;
				t1f = get_1_rif(opcode);
				t2f = get_2_rif(opcode);
				set_rif(opcode, t2f*log(t1f+1.0)/log(2.0));
				break;

			case 0x3: // remr
				m_icount -= 67; // (67 to 75878 depending on opcodes!!!)
				t1f = get_1_rif(opcode);
				t2f = get_2_rif(opcode);
				set_rif(opcode, fmod(t2f, t1f));
				break;

			case 0x5: // cmpr
				m_icount -= 10;
				t1f = get_1_rif(opcode);
				t2f = get_2_rif(opcode);
				cmp_d(t1f, t2f);
				break;

			case 0x8: // sqrtr
				m_icount -= 104;
				t1f = get_1_rif(opcode);
				set_rif(opcode, sqrt(t1f));
				break;

			case 0xa: // logbnr
				m_icount -= 37;
				t1f = get_1_rif(opcode);
				set_rif(opcode, logb(t1f));
				break;

			case 0xb: // roundr
				{
					INT32 st1 = get_1_rif(opcode);
					m_icount -= 69;
					set_rif(opcode, (double)st1);
				}
				break;

			case 0xc: // sinr
				m_icount -= 406;
				t1f = get_1_rif(opcode);
				set_rif(opcode, sin(t1f));
				break;

			case 0xd: // cosr
				m_icount -= 406;
				t1f = get_1_rif(opcode);
				set_rif(opcode, cos(t1f));
				break;

			case 0xe: // tanr
				m_icount -= 293;
				t1f = get_1_rif(opcode);
				set_rif(opcode, tan(t1f));
				break;

			default:
				fatalerror("I960: %x: Unhandled 68.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x69:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // atanrl
				m_icount -= 350;
				t1f = get_1_rifl(opcode);
				t2f = get_2_rifl(opcode);
				set_rifl(opcode, atan2(t2f, t1f));
				break;

			case 0x2: // logrl
				m_icount -= 438;
				t1f = get_1_rifl(opcode);
				set_rifl(opcode, log(t1f));
				break;

			case 0x5: // cmprl
				m_icount -= 12;
				t1f = get_1_rifl(opcode);
				t2f = get_2_rifl(opcode);
				cmp_d(t1f, t2f);
				break;

			case 0x8: // sqrtrl
				m_icount -= 104;
				t1f = get_1_rifl(opcode);
				set_rifl(opcode, sqrt(t1f));
				break;

			case 0x9: // exprl
				m_icount -= 334;
				t1f = get_1_rifl(opcode);
				set_rifl(opcode, pow(2.0, t1f)-1.0);
				break;

			case 0xa: // logbnrl
				m_icount -= 37;
				t1f = get_1_rifl(opcode);
				set_rifl(opcode, logb(t1f));
				break;

			case 0xb: // roundrl
				{
					INT32 st1 = get_1_rifl(opcode);
					m_icount -= 70;
					set_rifl(opcode, (double)st1);
				}
				break;

			case 0xc: // sinrl
				m_icount -= 441;
				t1f = get_1_rifl(opcode);
				set_rifl(opcode, sin(t1f));
				break;

			case 0xd: // cosrl
				m_icount -= 441;
				t1f = get_1_rifl(opcode);
				set_rifl(opcode, cos(t1f));
				break;

			case 0xe: // tanrl
				m_icount -= 323;
				t1f = get_1_rifl(opcode);
				set_rifl(opcode, tan(t1f));
				break;

			default:
				fatalerror("I960: %x: Unhandled 69.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x6c:
			switch((opcode >> 7) & 0xf) {
			case 0x0: // cvtri
				m_icount -= 33;
				t1f = get_1_rif(opcode);
				// apply rounding mode
				// we do this a little indirectly to avoid some odd GCC warnings
				t2f = 0.0;
				switch((m_AC>>30)&3)
				{
					case 0: t2f = floor(t1f+0.5); break;
					case 1: t2f = floor(t1f); break;
					case 2: t2f = ceil(t1f); break;
					case 3: t2f = t1f; break;
				}
				set_ri(opcode, (INT32)t2f);
				break;

			case 0x2: // cvtzri
				m_icount -= 43;
				t1f = get_1_rif(opcode);
				set_ri(opcode, (INT32)t1f);
				break;

			case 0x3: // cvtzril
				m_icount -= 44;
				t1f = get_1_rif(opcode);
				set_ri64(opcode, (INT64)t1f);
				break;

			case 0x9: // movr
				m_icount -= 5;
				t1f = get_1_rif(opcode);
				set_rif(opcode, t1f);
				break;

			default:
				fatalerror("I960: %x: Unhandled 6c.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x6d:
			switch((opcode >> 7) & 0xf) {
			case 0x9: // movrl
				m_icount -= 6;
				t1f = get_1_rifl(opcode);
				set_rifl(opcode, t1f);
				break;

			default:
				fatalerror("I960: %x: Unhandled 6d.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x6e:
			switch((opcode >> 7) & 0xf) {
			case 0x1: // movre
				{
					UINT32 *src=0, *dst=0;

					m_icount -= 8;

					if(!(opcode & 0x00000800)) {
						src = (UINT32 *)&m_r[opcode & 0x1e];
					} else {
						int idx = opcode & 0x1f;
						if(idx < 4)
							src = (UINT32 *)&m_fp[idx];
					}

					if(!(opcode & 0x00002000)) {
						dst = (UINT32 *)&m_r[(opcode>>19) & 0x1e];
					} else if(!(opcode & 0x00e00000))
						dst = (UINT32 *)&m_fp[(opcode>>19) & 3];

					dst[0] = src[0];
					dst[1] = src[1];
					dst[2] = src[2]&0xffff;
				}
				break;
			case 0x2: // cpysre
				m_icount -= 8;
				t1f = get_1_rifl(opcode);
				t2f = get_2_rifl(opcode);

				if (t2f >= 0.0)
					set_rifl(opcode, fabs(t1f));
				else
					set_rifl(opcode, -fabs(t1f));
				break;
			default:
				fatalerror("I960: %x: Unhandled 6e.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x70:
			switch((opcode >> 7) & 0xf) {
			case 0x1: // mulo
				m_icount -= 18;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2*t1);
				break;

			case 0x8: // remo
				m_icount -= 37;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, t2%t1);
				break;

			case 0xb: // divo
				m_icount -= 37;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				if (t1 == 0)    // HACK!
					set_ri(opcode, 0);
				else
					set_ri(opcode, t2/t1);
				break;

			default:
				fatalerror("I960: %x: Unhandled 70.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x74:
			switch((opcode >> 7) & 0xf) {
			case 0x1: // muli
				m_icount -= 18;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, ((INT32)t2)*((INT32)t1));
				break;

			case 0x8: // remi
				m_icount -= 37;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, ((INT32)t2)%((INT32)t1));
				break;

			case 0x9:{// modi
				INT32 src1, src2, dst;
				m_icount -= 37;
				src1 = (INT32)get_1_ri(opcode);
				src2 = (INT32)get_2_ri(opcode);
				dst = src2 - ((src2/src1)*src1);
				if(((src2*src1) < 0) && (dst != 0))
					dst += src1;
				set_ri(opcode, dst);
				break;
			}

			case 0xb: // divi
				m_icount -= 37;
				t1 = get_1_ri(opcode);
				t2 = get_2_ri(opcode);
				set_ri(opcode, ((INT32)t2)/((INT32)t1));
				break;

			default:
				fatalerror("I960: %x: Unhandled 74.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x78:
			switch((opcode >> 7) & 0xf) {
			case 0xb: // divr
				m_icount -= 35;
				t1f = get_1_rif(opcode);
				t2f = get_2_rif(opcode);
				set_rif(opcode, t2f/t1f);
				break;

			case 0xc: // mulr
				m_icount -= 18;
				t1f = get_1_rif(opcode);
				t2f = get_2_rif(opcode);
				set_rif(opcode, t2f*t1f);
				break;

			case 0xd: // subr
				m_icount -= 10;
				t1f = get_1_rif(opcode);
				t2f = get_2_rif(opcode);
				set_rif(opcode, t2f-t1f);
				break;

			case 0xf: // addr
				m_icount -= 10;
				t1f = get_1_rif(opcode);
				t2f = get_2_rif(opcode);
				set_rif(opcode, t2f+t1f);
				break;

			default:
				fatalerror("I960: %x: Unhandled 78.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x79:
			switch((opcode >> 7) & 0xf) {
			case 0xb: // divrl
				m_icount -= 77;
				t1f = get_1_rifl(opcode);
				t2f = get_2_rifl(opcode);
				set_rifl(opcode, t2f/t1f);
				break;

			case 0xc: // mulrl
				m_icount -= 36;
				t1f = get_1_rifl(opcode);
				t2f = get_2_rifl(opcode);
				set_rifl(opcode, t2f*t1f);
				break;

			case 0xd: // subrl
				m_icount -= 13;
				t1f = get_1_rifl(opcode);
				t2f = get_2_rifl(opcode);
				set_rifl(opcode, t2f-t1f);
				break;

			case 0xf: // addrl
				m_icount -= 13;
				t1f = get_1_rifl(opcode);
				t2f = get_2_rifl(opcode);
				set_rifl(opcode, t2f+t1f);
				break;

			default:
				fatalerror("I960: %x: Unhandled 79.%x\n", m_PIP, (opcode >> 7) & 0xf);
			}
			break;

		case 0x80: // ldob
			m_icount -= 4;
			m_r[(opcode>>19)&0x1f] = m_program->read_byte(get_ea(opcode));
			break;

		case 0x82: // stob
			m_icount -= 2;
			m_program->write_byte(get_ea(opcode), m_r[(opcode>>19)&0x1f]);
			break;

		case 0x84: // bx
			m_icount -= 3;
			m_IP = get_ea(opcode);
			break;

		case 0x85: // balx
			m_icount -= 5;
			t1 = get_ea(opcode);
			m_r[(opcode>>19)&0x1f] = m_IP;
			m_IP = t1;
			break;

		case 0x86: // callx
			t1 = get_ea(opcode);
			do_call(t1, 0, m_r[I960_SP]);
			break;

		case 0x88: // ldos
			m_icount -= 4;
			m_r[(opcode>>19)&0x1f] = i960_read_word_unaligned(get_ea(opcode));
			break;

		case 0x8a: // stos
			m_icount -= 2;
			i960_write_word_unaligned(get_ea(opcode), m_r[(opcode>>19)&0x1f]);
			break;

		case 0x8c: // lda
			m_icount--;
			m_r[(opcode>>19)&0x1f] = get_ea(opcode);
			break;

		case 0x90: // ld
			m_icount -= 4;
			m_r[(opcode>>19)&0x1f] = i960_read_dword_unaligned(get_ea(opcode));
			break;

		case 0x92: // st
			m_icount -= 2;
			i960_write_dword_unaligned(get_ea(opcode), m_r[(opcode>>19)&0x1f]);
			break;

		case 0x98:{// ldl
			int i;
			m_icount -= 5;
			t1 = get_ea(opcode);
			t2 = (opcode>>19)&0x1e;
			m_bursting = 1;
			for(i=0; i<2; i++) {
				m_r[t2+i] = i960_read_dword_unaligned(t1);
				if(m_bursting)
					t1 += 4;
			}
			break;
		}

		case 0x9a:{// stl
			int i;
			m_icount -= 3;
			t1 = get_ea(opcode);
			t2 = (opcode>>19)&0x1e;
			m_bursting = 1;
			for(i=0; i<2; i++) {
				i960_write_dword_unaligned(t1, m_r[t2+i]);
				if(m_bursting)
					t1 += 4;
			}
			break;
		}

		case 0xa0:{// ldt
			int i;
			m_icount -= 6;
			t1 = get_ea(opcode);
			t2 = (opcode>>19)&0x1c;
			m_bursting = 1;
			for(i=0; i<3; i++) {
				m_r[t2+i] = i960_read_dword_unaligned(t1);
				if(m_bursting)
					t1 += 4;
			}
			break;
		}

		case 0xa2:{// stt
			int i;
			m_icount -= 4;
			t1 = get_ea(opcode);
			t2 = (opcode>>19)&0x1c;
			m_bursting = 1;
			for(i=0; i<3; i++) {
				i960_write_dword_unaligned(t1, m_r[t2+i]);
				if(m_bursting)
					t1 += 4;
			}
			break;
		}

		case 0xb0:{// ldq
			int i;
			m_icount -= 7;
			t1 = get_ea(opcode);
			t2 = (opcode>>19)&0x1c;
			m_bursting = 1;
			for(i=0; i<4; i++) {
				m_r[t2+i] = i960_read_dword_unaligned(t1);
				if(m_bursting)
					t1 += 4;
			}
			break;
		}

		case 0xb2:{// stq
			int i;
			m_icount -= 5;
			t1 = get_ea(opcode);
			t2 = (opcode>>19)&0x1c;
			m_bursting = 1;
			for(i=0; i<4; i++) {
				i960_write_dword_unaligned(t1, m_r[t2+i]);
				if(m_bursting)
					t1 += 4;
			}
			break;
		}

		case 0xc0: // ldib
			m_icount -= 4;
			m_r[(opcode>>19)&0x1f] = (INT8)m_program->read_byte(get_ea(opcode));
			break;

		case 0xc2: // stib
			m_icount -= 2;
			m_program->write_byte(get_ea(opcode), m_r[(opcode>>19)&0x1f]);
			break;

		case 0xc8: // ldis
			m_icount -= 4;
			m_r[(opcode>>19)&0x1f] = (INT16)i960_read_word_unaligned(get_ea(opcode));
			break;

		case 0xca: // stis
			m_icount -= 2;
			i960_write_word_unaligned(get_ea(opcode), m_r[(opcode>>19)&0x1f]);
			break;

		default:
			fatalerror("I960: %x: Unhandled %02x\n", m_PIP, opcode >> 24);
	}

}

void i960_cpu_device::execute_run()
{
	UINT32 opcode;

	check_irqs();
	while(m_icount > 0) {
		m_PIP = m_IP;
		debugger_instruction_hook(this, m_IP);

		m_bursting = 0;

		opcode = m_direct->read_dword(m_IP);
		m_IP += 4;

		execute_op(opcode);
	}
}

void i960_cpu_device::execute_set_input(int irqline, int state)
{
	int int_tab =  m_program->read_dword(m_PRCB+20);    // interrupt table
	int cpu_pri = (m_PC>>16)&0x1f;
	int vector =0;
	int priority;
	UINT32 pend, word, wordofs;

	// We support the 4 external IRQ lines in "normal" mode only.
	// The i960's interrupt support is a bit more complete than that,
	// but Namco and Sega both went for the cheapest solution.

	switch (irqline)
	{
		case I960_IRQ0:
			vector = m_ICR & 0xff;
			break;

		case I960_IRQ1:
			vector = (m_ICR>>8)&0xff;
			break;

		case I960_IRQ2:
			vector = (m_ICR>>16)&0xff;
			break;

		case I960_IRQ3:
			vector = (m_ICR>>24)&0xff;
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
		if (((cpu_pri < priority) || (priority == 31)) && (m_immediate_irq == 0))
		{
			m_immediate_irq = 1;
			m_immediate_vector = vector;
			m_immediate_pri = priority;
		}
		else
		{
			// store the interrupt in the "pending" table
			pend = m_program->read_dword(int_tab);
			pend |= (1 << priority);
			m_program->write_dword(int_tab, pend);

			// now bitfield-ize the vector
			word = ((vector / 32) * 4) + 4;
			wordofs = vector % 32;
			pend = m_program->read_dword(int_tab + word);
			pend |= (1 << wordofs);
			m_program->write_dword(int_tab + word, pend);
		}

		// and ack it to the core now that it's queued
		standard_irq_callback(irqline);
	}
}


void i960_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	save_item(NAME(m_IP));
	save_item(NAME(m_PIP));
	save_item(NAME(m_SAT));
	save_item(NAME(m_PRCB));
	save_item(NAME(m_PC));
	save_item(NAME(m_AC));
	save_item(NAME(m_ICR));
	save_item(NAME(m_r));
	save_item(NAME(m_fp));
	save_item(NAME(m_rcache));
	save_item(NAME(m_rcache_frame_addr));
	save_item(NAME(m_rcache_pos));
	save_item(NAME(m_immediate_irq));
	save_item(NAME(m_immediate_vector));
	save_item(NAME(m_immediate_pri));

	state_add( I960_SAT,  "sat", m_SAT).formatstr("%08X");
	state_add( I960_PRCB, "prcb", m_PRCB).formatstr("%08X");
	state_add( I960_PC,   "pc", m_PC).formatstr("%08X");
	state_add( I960_AC,   "ac", m_AC).formatstr("%08X");
	state_add( I960_IP,   "ip", m_IP).formatstr("%08X");
	state_add( I960_PIP,  "pip", m_PIP).formatstr("%08X");
	state_add( I960_R0,   "pfp", m_r[ 0]).formatstr("%08X");
	state_add( I960_R1,   "sp", m_r[ 1]).formatstr("%08X");
	state_add( I960_R2,   "rip", m_r[ 2]).formatstr("%08X");
	state_add( I960_R3,   "r3", m_r[ 3]).formatstr("%08X");
	state_add( I960_R4,   "r4", m_r[ 4]).formatstr("%08X");
	state_add( I960_R5,   "r5", m_r[ 5]).formatstr("%08X");
	state_add( I960_R6,   "r6", m_r[ 6]).formatstr("%08X");
	state_add( I960_R7,   "r7", m_r[ 7]).formatstr("%08X");
	state_add( I960_R8,   "r8", m_r[ 8]).formatstr("%08X");
	state_add( I960_R9,   "r9", m_r[ 9]).formatstr("%08X");
	state_add( I960_R10,  "r10", m_r[10]).formatstr("%08X");
	state_add( I960_R11,  "r11", m_r[11]).formatstr("%08X");
	state_add( I960_R12,  "r12", m_r[12]).formatstr("%08X");
	state_add( I960_R13,  "r13", m_r[13]).formatstr("%08X");
	state_add( I960_R14,  "r14", m_r[14]).formatstr("%08X");
	state_add( I960_R15,  "r15", m_r[15]).formatstr("%08X");
	state_add( I960_G0,   "g0", m_r[16]).formatstr("%08X");
	state_add( I960_G1,   "g1", m_r[17]).formatstr("%08X");
	state_add( I960_G2,   "g2", m_r[18]).formatstr("%08X");
	state_add( I960_G3,   "g3", m_r[19]).formatstr("%08X");
	state_add( I960_G4,   "g4", m_r[20]).formatstr("%08X");
	state_add( I960_G5,   "g5", m_r[21]).formatstr("%08X");
	state_add( I960_G6,   "g6", m_r[22]).formatstr("%08X");
	state_add( I960_G7,   "g7", m_r[23]).formatstr("%08X");
	state_add( I960_G8,   "g8", m_r[24]).formatstr("%08X");
	state_add( I960_G9,   "g9", m_r[25]).formatstr("%08X");
	state_add( I960_G10,  "g10", m_r[26]).formatstr("%08X");
	state_add( I960_G11,  "g11", m_r[27]).formatstr("%08X");
	state_add( I960_G12,  "g12", m_r[28]).formatstr("%08X");
	state_add( I960_G13,  "g13", m_r[29]).formatstr("%08X");
	state_add( I960_G14,  "g14", m_r[30]).formatstr("%08X");
	state_add( I960_G15,  "fp", m_r[31]).formatstr("%08X");

	state_add( STATE_GENPC, "GENPC", m_IP).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_AC).noshow().formatstr("%2s");

	m_immediate_vector = 0;
	m_immediate_pri = 0;
	memset(m_rcache_frame_addr, 0, sizeof(m_rcache_frame_addr));
	memset(m_fp, 0, sizeof(m_fp));
	m_PIP = 0;

	m_icountptr = &m_icount;
}

void i960_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	static const char *const conditions[8] =
	{
		"no", "g", "e", "ge", "l", "ne", "le", "o"
	};

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%s", conditions[m_AC & 7]);
			break;
	}
}

void i960_cpu_device::device_reset()
{
	m_SAT        = m_program->read_dword(0);
	m_PRCB       = m_program->read_dword(4);
	m_IP         = m_program->read_dword(12);
	m_PC         = 0x001f2002;
	m_AC         = 0;
	m_ICR       = 0xff000000;
	m_bursting   = 0;
	m_immediate_irq = 0;

	memset(m_r, 0, sizeof(m_r));
	memset(m_rcache, 0, sizeof(m_rcache));

	m_r[I960_FP] = m_program->read_dword(m_PRCB+24);
	m_r[I960_SP] = m_r[I960_FP] + 64;
	m_rcache_pos = 0;
}


offs_t i960_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( i960 );
	return CPU_DISASSEMBLE_NAME(i960)(this, buffer, pc, oprom, opram, options);
}
