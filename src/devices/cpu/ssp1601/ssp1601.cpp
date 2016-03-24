// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli,Grazvydas Ignotas
/*
 * Samsung SSP1601 DSP emulator
 *
 * Copyright 2008, Grazvydas Ignotas
 *
 * notes:
 * not everything is implemented, but it is accurate enough to
 * properly emulate Virtua Racing for Genesis/MegaDrive:
 *
 *   only Z and N status flags are emulated (others unused by VR)
 *   so all condition checks except N and Z are ignored (not used by VR)
 *   modifiers for 'OP a, ri' and ((ri)) are ignored (not used by VR)
 *   loop repeat mode when destination is (ri) is ignored
 *   ops not used by VR are not implemented
 */

#include "emu.h"
#include "debugger.h"
#include "ssp1601.h"


/* detect ops with unimplemented/invalid fields.
 * Useful for homebrew or if a new VR revision pops up. */
//#define DO_CHECKS


// 0
#define rX     m_gr[SSP_X].w.h
#define rY     m_gr[SSP_Y].w.h
#define rA     m_gr[SSP_A].w.h
#define rST    m_gr[SSP_ST].w.h    // 4
#define rSTACK m_gr[SSP_STACK].w.h
#define rPC    m_gr[SSP_PC].w.h
#define rP     m_gr[SSP_P]

#define rAL    m_gr[SSP_A].w.l
#define rA32   m_gr[SSP_A].d
#define rIJ    m_r

#define IJind  (((op>>6)&4)|(op&3))

#define PPC    m_ppc.w.h

#define FETCH() m_direct->read_word(rPC++ << 1)
#define PROGRAM_WORD(a) m_program->read_word((a) << 1)
#define GET_PPC_OFFS() PPC

#define REG_READ(r) (((r) <= 4) ? m_gr[r].w.h : (this->*reg_read_handlers[r])(r))
#define REG_WRITE(r,d) { \
	int r1 = r; \
	if (r1 >= 4) (this->*reg_write_handlers[r1])(r1,d); \
	else if (r1 > 0) m_gr[r1].w.h = d; \
}

// flags
#define SSP_FLAG_L (1<<0xc)
#define SSP_FLAG_Z (1<<0xd)
#define SSP_FLAG_V (1<<0xe)
#define SSP_FLAG_N (1<<0xf)

// update ZN according to 32bit ACC.
#define UPD_ACC_ZN \
	rST &= ~(SSP_FLAG_Z|SSP_FLAG_N); \
	if (!rA32) rST |= SSP_FLAG_Z; \
	else rST |= (rA32>>16)&SSP_FLAG_N;

// it seems SVP code never checks for L and OV, so we leave them out.
#define UPD_LZVN \
	rST &= ~(SSP_FLAG_L|SSP_FLAG_Z|SSP_FLAG_V|SSP_FLAG_N); \
	if (!rA32) rST |= SSP_FLAG_Z; \
	else rST |= (rA32>>16)&SSP_FLAG_N;

// standard cond processing.
// again, only Z and N is checked, as VR doesn't seem to use any other conds.
#define COND_CHECK \
	switch (op&0xf0) { \
		case 0x00: cond = 1; break; /* always true */ \
		case 0x50: cond = !((rST ^ (op<<5)) & SSP_FLAG_Z); break; /* Z matches f(?) bit */ \
		case 0x70: cond = !((rST ^ (op<<7)) & SSP_FLAG_N); break; /* N matches f(?) bit */ \
		default:logerror(__FILE__ " FIXME: unimplemented cond @ %04x\n", GET_PPC_OFFS()); break; \
	}

// ops with accumulator.
// note that 'ld A' doesn't affect flags
#define OP_LDA(x) \
	rA = x

#define OP_LDA32(x) \
	rA32 = x

#define OP_SUBA(x) { \
	rA32 -= (x) << 16; \
	UPD_LZVN \
}

#define OP_SUBA32(x) { \
	rA32 -= (x); \
	UPD_LZVN \
}

#define OP_CMPA(x) { \
	UINT32 t = rA32 - ((x) << 16); \
	rST &= ~(SSP_FLAG_L|SSP_FLAG_Z|SSP_FLAG_V|SSP_FLAG_N); \
	if (!t) rST |= SSP_FLAG_Z; \
	else    rST |= (t>>16)&SSP_FLAG_N; \
}

#define OP_CMPA32(x) { \
	UINT32 t = rA32 - (x); \
	rST &= ~(SSP_FLAG_L|SSP_FLAG_Z|SSP_FLAG_V|SSP_FLAG_N); \
	if (!t) rST |= SSP_FLAG_Z; \
	else    rST |= (t>>16)&SSP_FLAG_N; \
}

#define OP_ADDA(x) { \
	rA32 += (x) << 16; \
	UPD_LZVN \
}

#define OP_ADDA32(x) { \
	rA32 += (x); \
	UPD_LZVN \
}

#define OP_ANDA(x) \
	rA32 &= (x) << 16; \
	UPD_ACC_ZN

#define OP_ANDA32(x) \
	rA32 &= (x); \
	UPD_ACC_ZN

#define OP_ORA(x) \
	rA32 |= (x) << 16; \
	UPD_ACC_ZN

#define OP_ORA32(x) \
	rA32 |= (x); \
	UPD_ACC_ZN

#define OP_EORA(x) \
	rA32 ^= (x) << 16; \
	UPD_ACC_ZN

#define OP_EORA32(x) \
	rA32 ^= (x); \
	UPD_ACC_ZN


#define OP_CHECK32(OP) { \
	if ((op & 0x0f) == SSP_P) { /* A <- P */ \
		update_P(); \
		OP(rP.d); \
		break; \
	} \
	if ((op & 0x0f) == SSP_A) { /* A <- A */ \
		OP(rA32); \
		break; \
	} \
}


#ifdef DO_CHECKS
#define CHECK_IMM16()   if (op&0x1ff)    logerror(__FILE__ " imm bits! %04x @ %04x\n", op,  GET_PPC_OFFS())
#define CHECK_B_SET()   if (op&0x100)    logerror(__FILE__ " b set!    %04x @ %04x\n", op,  GET_PPC_OFFS())
#define CHECK_B_CLEAR() if (!(op&0x100)) logerror(__FILE__ " b clear!  %04x @ %04x\n", op,  GET_PPC_OFFS())
#define CHECK_MOD()     if (op&0x00c)    logerror(__FILE__ " mod bits! %04x @ %04x\n", op,  GET_PPC_OFFS())
#define CHECK_10f()     if (op&0x10f)    logerror(__FILE__ " bits 10f! %04x @ %04x\n", op,  GET_PPC_OFFS())
#define CHECK_008()     if (op&0x008)    logerror(__FILE__ " bits 008! %04x @ %04x\n", op,  GET_PPC_OFFS())
#define CHECK_00f()     if (op&0x00f)    logerror(__FILE__ " bits 00f! %04x @ %04x\n", op,  GET_PPC_OFFS())
#define CHECK_0f0()     if (op&0x0f0)    logerror(__FILE__ " bits 0f0! %04x @ %04x\n", op,  GET_PPC_OFFS())
#define CHECK_1f0()     if (op&0x1f0)    logerror(__FILE__ " bits 1f0! %04x @ %04x\n", op,  GET_PPC_OFFS())
#define CHECK_RPL()     if (rST&7)       logerror(__FILE__ " unhandled RPL! %04x @ %04x\n", op,  GET_PPC_OFFS())
#define CHECK_ST(d)     if((rST^d)&0xf98)logerror(__FILE__ " ssp FIXME ST %04x -> %04x @ %04x\n", rST, d, GET_PPC_OFFS())
#else
#define CHECK_IMM16()
#define CHECK_B_SET()
#define CHECK_B_CLEAR()
#define CHECK_MOD()
#define CHECK_10f()
#define CHECK_008()
#define CHECK_00f()
#define CHECK_0f0()
#define CHECK_1f0()
#define CHECK_RPL()
#define CHECK_ST(d)
#endif


const device_type SSP1601 = &device_creator<ssp1601_device>;


ssp1601_device::ssp1601_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SSP1601, "SSP1601", tag, owner, clock, "ssp1601", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 16, 16, -1)
	, m_io_config("io", ENDIANNESS_BIG, 16, 4, 0)
{
}


offs_t ssp1601_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( ssp1601 );
	return CPU_DISASSEMBLE_NAME(ssp1601)(this, buffer, pc, oprom, opram, options);
}


// -----------------------------------------------------
// register i/o handlers

void ssp1601_device::update_P()
{
	int m1 = (signed short)rX;
	int m2 = (signed short)rY;
	rP.d = (m1 * m2 * 2);
}

UINT32 ssp1601_device::read_unknown(int reg)
{
	logerror("%s:%i FIXME\n", __FILE__, __LINE__);
	return 0;
}

void ssp1601_device::write_unknown(int reg, UINT32 d)
{
	logerror("%s:%i FIXME\n", __FILE__, __LINE__);
}

/* map EXT regs to virtual I/O range of 0x00-0x0f */
UINT32 ssp1601_device::read_ext(int reg)
{
	reg &= 7;
	return m_io->read_word((reg << 1));
}

void ssp1601_device::write_ext(int reg, UINT32 d)
{
	reg &= 7;
	m_io->write_word((reg << 1), d);
}

// 4
void ssp1601_device::write_ST(int reg, UINT32 d)
{
	CHECK_ST(d);
	rST = d;
}

// 5
UINT32 ssp1601_device::read_STACK(int reg)
{
	--rSTACK;
	if ((signed short)rSTACK < 0) {
		rSTACK = 5;
		logerror(__FILE__ " FIXME: stack underflow! (%i) @ %04x\n", rSTACK, GET_PPC_OFFS());
	}
	return m_stack[rSTACK];
}

void ssp1601_device::write_STACK(int reg, UINT32 d)
{
	if (rSTACK >= 6) {
		logerror(__FILE__ " FIXME: stack overflow! (%i) @ %04x\n", rSTACK, GET_PPC_OFFS());
		rSTACK = 0;
	}
	m_stack[rSTACK++] = d;
}

// 6
UINT32 ssp1601_device::read_PC(int reg)
{
	return rPC;
}

void ssp1601_device::write_PC(int reg, UINT32 d)
{
	rPC = d;
	m_g_cycles--;
}

// 7
UINT32 ssp1601_device::read_P(int reg)
{
	update_P();
	return rP.w.h;
}

// 15
UINT32 ssp1601_device::read_AL(int reg)
{
	/* apparently reading AL causes some effect on EXT bus, VR depends on that.. */
	read_ext(reg);
	return rAL;
}

void ssp1601_device::write_AL(int reg, UINT32 d)
{
	write_ext(reg, d);
	rAL = d;
}



const ssp1601_device::read_func_t ssp1601_device::reg_read_handlers[16] =
{
	&ssp1601_device::read_unknown, &ssp1601_device::read_unknown, &ssp1601_device::read_unknown, &ssp1601_device::read_unknown, // -, X, Y, A
	&ssp1601_device::read_unknown,   // 4 ST
	&ssp1601_device::read_STACK,
	&ssp1601_device::read_PC,
	&ssp1601_device::read_P,
	&ssp1601_device::read_ext,   // 8
	&ssp1601_device::read_ext,
	&ssp1601_device::read_ext,
	&ssp1601_device::read_ext,
	&ssp1601_device::read_ext,   // 12
	&ssp1601_device::read_ext,
	&ssp1601_device::read_ext,
	&ssp1601_device::read_AL
};

const ssp1601_device::write_func_t ssp1601_device::reg_write_handlers[16] =
{
	&ssp1601_device::write_unknown, &ssp1601_device::write_unknown, &ssp1601_device::write_unknown, &ssp1601_device::write_unknown, // -, X, Y, A
	&ssp1601_device::write_ST,
	&ssp1601_device::write_STACK,
	&ssp1601_device::write_PC,
	&ssp1601_device::write_unknown,  // 7 P (not writable)
	&ssp1601_device::write_ext,  // 8
	&ssp1601_device::write_ext,
	&ssp1601_device::write_ext,
	&ssp1601_device::write_ext,
	&ssp1601_device::write_ext,  // 12
	&ssp1601_device::write_ext,
	&ssp1601_device::write_ext,
	&ssp1601_device::write_AL
};

// -----------------------------------------------------
// pointer register handlers

//
#define ptr1_read(op) ptr1_read_(op&3,(op>>6)&4,(op<<1)&0x18)

UINT32 ssp1601_device::ptr1_read_(int ri, int isj2, int modi3)
{
	//int t = (op&3) | ((op>>6)&4) | ((op<<1)&0x18);
	UINT32 mask, add = 0, t = ri | isj2 | modi3;
	unsigned char *rp = nullptr;
	switch (t)
	{
		// mod=0 (00)
		case 0x00:
		case 0x01:
		case 0x02: return mem.m_RAM0[regs.m_r0[t&3]];
		case 0x03: return mem.m_RAM0[0];
		case 0x04:
		case 0x05:
		case 0x06: return mem.m_RAM1[regs.m_r1[t&3]];
		case 0x07: return mem.m_RAM1[0];
		// mod=1 (01), "+!"
		case 0x08:
		case 0x09:
		case 0x0a: return mem.m_RAM0[regs.m_r0[t&3]++];
		case 0x0b: return mem.m_RAM0[1];
		case 0x0c:
		case 0x0d:
		case 0x0e: return mem.m_RAM1[regs.m_r1[t&3]++];
		case 0x0f: return mem.m_RAM1[1];
		// mod=2 (10), "-"
		case 0x10:
		case 0x11:
		case 0x12: rp = &regs.m_r0[t&3]; t = mem.m_RAM0[*rp];
					if (!(rST&7)) { (*rp)--; return t; }
					add = -1; goto modulo;
		case 0x13: return mem.m_RAM0[2];
		case 0x14:
		case 0x15:
		case 0x16: rp = &regs.m_r1[t&3]; t = mem.m_RAM1[*rp];
					if (!(rST&7)) { (*rp)--; return t; }
					add = -1; goto modulo;
		case 0x17: return mem.m_RAM1[2];
		// mod=3 (11), "+"
		case 0x18:
		case 0x19:
		case 0x1a: rp = &regs.m_r0[t&3]; t = mem.m_RAM0[*rp];
					if (!(rST&7)) { (*rp)++; return t; }
					add = 1; goto modulo;
		case 0x1b: return mem.m_RAM0[3];
		case 0x1c:
		case 0x1d:
		case 0x1e: rp = &regs.m_r1[t&3]; t = mem.m_RAM1[*rp];
					if (!(rST&7)) { (*rp)++; return t; }
					add = 1; goto modulo;
		case 0x1f: return mem.m_RAM1[3];
	}

	return 0;

modulo:
	mask = (1 << (rST&7)) - 1;
	*rp = (*rp & ~mask) | ((*rp + add) & mask);
	return t;
}

void ssp1601_device::ptr1_write(int op, UINT32 d)
{
	int t = (op&3) | ((op>>6)&4) | ((op<<1)&0x18);
	switch (t)
	{
		// mod=0 (00)
		case 0x00:
		case 0x01:
		case 0x02: mem.m_RAM0[regs.m_r0[t&3]] = d; return;
		case 0x03: mem.m_RAM0[0] = d; return;
		case 0x04:
		case 0x05:
		case 0x06: mem.m_RAM1[regs.m_r1[t&3]] = d; return;
		case 0x07: mem.m_RAM1[0] = d; return;
		// mod=1 (01), "+!"
		// mod=3,      "+"
		case 0x08:
		case 0x09:
		case 0x0a: mem.m_RAM0[regs.m_r0[t&3]++] = d; return;
		case 0x0b: mem.m_RAM0[1] = d; return;
		case 0x0c:
		case 0x0d:
		case 0x0e: mem.m_RAM1[regs.m_r1[t&3]++] = d; return;
		case 0x0f: mem.m_RAM1[1] = d; return;
		// mod=2 (10), "-"
		case 0x10:
		case 0x11:
		case 0x12: mem.m_RAM0[regs.m_r0[t&3]--] = d; CHECK_RPL(); return;
		case 0x13: mem.m_RAM0[2] = d; return;
		case 0x14:
		case 0x15:
		case 0x16: mem.m_RAM1[regs.m_r1[t&3]--] = d; CHECK_RPL(); return;
		case 0x17: mem.m_RAM1[2] = d; return;
		// mod=3 (11), "+"
		case 0x18:
		case 0x19:
		case 0x1a: mem.m_RAM0[regs.m_r0[t&3]++] = d; CHECK_RPL(); return;
		case 0x1b: mem.m_RAM0[3] = d; return;
		case 0x1c:
		case 0x1d:
		case 0x1e: mem.m_RAM1[regs.m_r1[t&3]++] = d; CHECK_RPL(); return;
		case 0x1f: mem.m_RAM1[3] = d; return;
	}
}

UINT32 ssp1601_device::ptr2_read(int op)
{
	int mv, t = (op&3) | ((op>>6)&4) | ((op<<1)&0x18);
	switch (t)
	{
		// mod=0 (00)
		case 0x00:
		case 0x01:
		case 0x02: mv = mem.m_RAM0[regs.m_r0[t&3]]++; break;
		case 0x03: mv = mem.m_RAM0[0]++; break;
		case 0x04:
		case 0x05:
		case 0x06: mv = mem.m_RAM1[regs.m_r1[t&3]]++; break;
		case 0x07: mv = mem.m_RAM1[0]++; break;
		// mod=1 (01)
		case 0x0b: mv = mem.m_RAM0[1]++; break;
		case 0x0f: mv = mem.m_RAM1[1]++; break;
		// mod=2 (10)
		case 0x13: mv = mem.m_RAM0[2]++; break;
		case 0x17: mv = mem.m_RAM1[2]++; break;
		// mod=3 (11)
		case 0x1b: mv = mem.m_RAM0[3]++; break;
		case 0x1f: mv = mem.m_RAM1[3]++; break;
		default:   logerror(__FILE__ " FIXME: unimplemented mod in ((rX)) @ %04x\n", GET_PPC_OFFS());
					return 0;
	}

	return PROGRAM_WORD(mv);
}


// -----------------------------------------------------


void ssp1601_device::device_start()
{
	save_item(NAME(rX));
	save_item(NAME(rY));
	save_item(NAME(rA32));
	save_item(NAME(rST));
	save_item(NAME(rSTACK));
	save_item(NAME(rPC));
	save_item(NAME(rP.d));
	save_item(NAME(PPC));
	save_item(NAME(m_stack));
	save_item(NAME(m_r));
	save_item(NAME(m_RAM));

	/* clear the state */
	for ( int i = 0; i < 8; i++ )
	{
		m_gr[i].d = 0;
		m_r[i] = 0;
	}
	memset( m_RAM, 0, sizeof(m_RAM));
	for (auto & elem : m_stack)
	{
		elem = 0;
	}
	m_ppc.d = 0;
	m_g_cycles = 0;

	m_gr[0].w.h = 0xffff; // constant reg
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	state_add( SSP_R0,     "REG0",   m_gr[0].w.h).formatstr("%04X");
	state_add( SSP_X,      "X",      rX).formatstr("%04X");
	state_add( SSP_Y,      "Y",      rY).formatstr("%04X");
	state_add( SSP_A,      "A",      rA32).formatstr("%08X");
	state_add( SSP_ST,     "ST",     rST).formatstr("%04X");
	state_add( SSP_STACK,  "STACK",  rSTACK).formatstr("%04X");
	state_add( SSP_PC,     "PC",     rPC).formatstr("%04X");
	state_add( SSP_P,      "P",      rP.d).formatstr("%08X");
	state_add( SSP_STACK0, "STACK0", m_stack[0]).formatstr("%04X");
	state_add( SSP_STACK1, "STACK1", m_stack[1]).formatstr("%04X");
	state_add( SSP_STACK2, "STACK2", m_stack[2]).formatstr("%04X");
	state_add( SSP_STACK3, "STACK3", m_stack[3]).formatstr("%04X");
	state_add( SSP_STACK4, "STACK4", m_stack[4]).formatstr("%04X");
	state_add( SSP_STACK5, "STACK5", m_stack[5]).formatstr("%04X");
	state_add( SSP_PR0,    "R0",     m_r[0]).formatstr("%02X");
	state_add( SSP_PR1,    "R1",     m_r[1]).formatstr("%02X");
	state_add( SSP_PR2,    "R2",     m_r[2]).formatstr("%02X");
	state_add( SSP_PR3,    "R3",     m_r[3]).formatstr("%02X");
	state_add( SSP_PR4,    "R4",     m_r[4]).formatstr("%02X");
	state_add( SSP_PR5,    "R5",     m_r[5]).formatstr("%02X");
	state_add( SSP_PR6,    "R6",     m_r[6]).formatstr("%02X");
	state_add( SSP_PR7,    "R7",     m_r[7]).formatstr("%02X");

	state_add(STATE_GENPC, "GENPC", rPC).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", rST).formatstr("%4s").noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", PPC).noshow();

	m_icountptr = &m_g_cycles;
}


void ssp1601_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c",
					(rST&SSP_FLAG_N) ? 'N' : '.',
					(rST&SSP_FLAG_V) ? 'V' : '.',
					(rST&SSP_FLAG_Z) ? 'Z' : '.',
					(rST&SSP_FLAG_L) ? 'L' : '.'
			);
			break;
	}
}


void ssp1601_device::device_reset()
{
	rPC = 0x400;
	rSTACK = 0; // ? using ascending stack
	rST = 0;
}


void ssp1601_device::execute_run()
{
	while (m_g_cycles > 0)
	{
		int op;
		UINT32 tmpv;

		PPC = rPC;

		debugger_instruction_hook(this, rPC);

		op = FETCH();

		switch (op >> 9)
		{
			// ld d, s
			case 0x00:
				CHECK_B_SET();
				if (op == 0) break; // nop
				if (op == ((SSP_A<<4)|SSP_P)) { // A <- P
					update_P();
					rA32 = rP.d;
				}
				else
				{
					tmpv = REG_READ(op & 0x0f);
					REG_WRITE((op & 0xf0) >> 4, tmpv);
				}
				break;

			// ld d, (ri)
			case 0x01: tmpv = ptr1_read(op); REG_WRITE((op & 0xf0) >> 4, tmpv); break;

			// ld (ri), s
			case 0x02: tmpv = REG_READ((op & 0xf0) >> 4); ptr1_write(op, tmpv); break;

			// ldi d, imm
			case 0x04: CHECK_10f(); tmpv = FETCH(); REG_WRITE((op & 0xf0) >> 4, tmpv);m_g_cycles--; break;

			// ld d, ((ri))
			case 0x05: CHECK_MOD(); tmpv = ptr2_read(op); REG_WRITE((op & 0xf0) >> 4, tmpv); m_g_cycles -= 2; break;

			// ldi (ri), imm
			case 0x06: tmpv = FETCH(); ptr1_write(op, tmpv); m_g_cycles--; break;

			// ld adr, a
			case 0x07: m_RAM[op & 0x1ff] = rA; break;

			// ld d, ri
			case 0x09: CHECK_MOD(); tmpv = rIJ[(op&3)|((op>>6)&4)]; REG_WRITE((op & 0xf0) >> 4, tmpv); break;

			// ld ri, s
			case 0x0a: CHECK_MOD(); rIJ[(op&3)|((op>>6)&4)] = REG_READ((op & 0xf0) >> 4); break;

			// ldi ri, simm
			case 0x0c:
			case 0x0d:
			case 0x0e:
			case 0x0f: rIJ[(op>>8)&7] = op; break;

			// call cond, addr
			case 0x24: {
				int cond = 0;
				CHECK_00f();
				COND_CHECK
				if (cond) { int new_PC = FETCH(); write_STACK(SSP_STACK, rPC); rPC = new_PC; }
				else rPC++;
				m_g_cycles--; // always 2 cycles
				break;
			}

			// ld d, (a)
			case 0x25:
				CHECK_10f();
				tmpv = PROGRAM_WORD(rA);
				REG_WRITE((op & 0xf0) >> 4, tmpv);
				m_g_cycles -= 2; // 3 cycles total
				break;

			// bra cond, addr
			case 0x26: {
				int cond = 0;
				CHECK_00f();
				COND_CHECK
				if (cond) { rPC = FETCH(); }
				else rPC++;
				m_g_cycles--;
				break;
			}

			// mod cond, op
			case 0x48: {
				int cond = 0;
				CHECK_008();
				COND_CHECK
				if (cond) {
					switch (op & 7) {
						case 2: rA32 = (signed int)rA32 >> 1; break; // shr (arithmetic)
						case 3: rA32 <<= 1; break; // shl
						case 6: rA32 = -(signed int)rA32; break; // neg
						case 7: if ((int)rA32 < 0) rA32 = -(signed int)rA32; break; // abs
						default: logerror(__FILE__ " FIXME: unhandled mod %i @ %04x\n",
								op&7, GET_PPC_OFFS());
					}
					UPD_ACC_ZN
				}
				break;
			}

			// mpys?
			case 0x1b:
				CHECK_B_CLEAR();
				update_P();
				rA32 -= rP.d;
				UPD_ACC_ZN
				rX = ptr1_read_(op&3, 0, (op<<1)&0x18);
				rY = ptr1_read_((op>>4)&3, 4, (op>>3)&0x18);
				break;

			// mpya (rj), (ri), b
			case 0x4b:
				CHECK_B_CLEAR();
				update_P();
				rA32 += rP.d;
				UPD_ACC_ZN
				rX = ptr1_read_(op&3, 0, (op<<1)&0x18);
				rY = ptr1_read_((op>>4)&3, 4, (op>>3)&0x18);
				break;

			// mld (rj), (ri), b
			case 0x5b:
				CHECK_B_CLEAR();
				rA32 = 0;
				rST &= 0x0fff;
				rST |= SSP_FLAG_Z;
				rX = ptr1_read_(op&3, 0, (op<<1)&0x18);
				rY = ptr1_read_((op>>4)&3, 4, (op>>3)&0x18);
				break;

			// OP a, s
			case 0x10: CHECK_1f0(); OP_CHECK32(OP_SUBA32); tmpv = REG_READ(op & 0x0f); OP_SUBA(tmpv); break;
			case 0x30: CHECK_1f0(); OP_CHECK32(OP_CMPA32); tmpv = REG_READ(op & 0x0f); OP_CMPA(tmpv); break;
			case 0x40: CHECK_1f0(); OP_CHECK32(OP_ADDA32); tmpv = REG_READ(op & 0x0f); OP_ADDA(tmpv); break;
			case 0x50: CHECK_1f0(); OP_CHECK32(OP_ANDA32); tmpv = REG_READ(op & 0x0f); OP_ANDA(tmpv); break;
			case 0x60: CHECK_1f0(); OP_CHECK32(OP_ORA32 ); tmpv = REG_READ(op & 0x0f); OP_ORA (tmpv); break;
			case 0x70: CHECK_1f0(); OP_CHECK32(OP_EORA32); tmpv = REG_READ(op & 0x0f); OP_EORA(tmpv); break;

			// OP a, (ri)
			case 0x11: CHECK_0f0(); tmpv = ptr1_read(op); OP_SUBA(tmpv); break;
			case 0x31: CHECK_0f0(); tmpv = ptr1_read(op); OP_CMPA(tmpv); break;
			case 0x41: CHECK_0f0(); tmpv = ptr1_read(op); OP_ADDA(tmpv); break;
			case 0x51: CHECK_0f0(); tmpv = ptr1_read(op); OP_ANDA(tmpv); break;
			case 0x61: CHECK_0f0(); tmpv = ptr1_read(op); OP_ORA (tmpv); break;
			case 0x71: CHECK_0f0(); tmpv = ptr1_read(op); OP_EORA(tmpv); break;

			// OP a, adr
			case 0x03: tmpv = m_RAM[op & 0x1ff]; OP_LDA (tmpv); break;
			case 0x13: tmpv = m_RAM[op & 0x1ff]; OP_SUBA(tmpv); break;
			case 0x33: tmpv = m_RAM[op & 0x1ff]; OP_CMPA(tmpv); break;
			case 0x43: tmpv = m_RAM[op & 0x1ff]; OP_ADDA(tmpv); break;
			case 0x53: tmpv = m_RAM[op & 0x1ff]; OP_ANDA(tmpv); break;
			case 0x63: tmpv = m_RAM[op & 0x1ff]; OP_ORA (tmpv); break;
			case 0x73: tmpv = m_RAM[op & 0x1ff]; OP_EORA(tmpv); break;

			// OP a, imm
			case 0x14: CHECK_IMM16(); tmpv = FETCH(); OP_SUBA(tmpv); m_g_cycles--; break;
			case 0x34: CHECK_IMM16(); tmpv = FETCH(); OP_CMPA(tmpv); m_g_cycles--; break;
			case 0x44: CHECK_IMM16(); tmpv = FETCH(); OP_ADDA(tmpv); m_g_cycles--; break;
			case 0x54: CHECK_IMM16(); tmpv = FETCH(); OP_ANDA(tmpv); m_g_cycles--; break;
			case 0x64: CHECK_IMM16(); tmpv = FETCH(); OP_ORA (tmpv); m_g_cycles--; break;
			case 0x74: CHECK_IMM16(); tmpv = FETCH(); OP_EORA(tmpv); m_g_cycles--; break;

			// OP a, ((ri))
			case 0x15: CHECK_MOD(); tmpv = ptr2_read(op); OP_SUBA(tmpv); m_g_cycles -= 2; break;
			case 0x35: CHECK_MOD(); tmpv = ptr2_read(op); OP_CMPA(tmpv); m_g_cycles -= 2; break;
			case 0x45: CHECK_MOD(); tmpv = ptr2_read(op); OP_ADDA(tmpv); m_g_cycles -= 2; break;
			case 0x55: CHECK_MOD(); tmpv = ptr2_read(op); OP_ANDA(tmpv); m_g_cycles -= 2; break;
			case 0x65: CHECK_MOD(); tmpv = ptr2_read(op); OP_ORA (tmpv); m_g_cycles -= 2; break;
			case 0x75: CHECK_MOD(); tmpv = ptr2_read(op); OP_EORA(tmpv); m_g_cycles -= 2; break;

			// OP a, ri
			case 0x19: CHECK_MOD(); tmpv = rIJ[IJind]; OP_SUBA(tmpv); break;
			case 0x39: CHECK_MOD(); tmpv = rIJ[IJind]; OP_CMPA(tmpv); break;
			case 0x49: CHECK_MOD(); tmpv = rIJ[IJind]; OP_ADDA(tmpv); break;
			case 0x59: CHECK_MOD(); tmpv = rIJ[IJind]; OP_ANDA(tmpv); break;
			case 0x69: CHECK_MOD(); tmpv = rIJ[IJind]; OP_ORA (tmpv); break;
			case 0x79: CHECK_MOD(); tmpv = rIJ[IJind]; OP_EORA(tmpv); break;

			// OP simm
			case 0x1c: CHECK_B_SET(); OP_SUBA(op & 0xff); break;
			case 0x3c: CHECK_B_SET(); OP_CMPA(op & 0xff); break;
			case 0x4c: CHECK_B_SET(); OP_ADDA(op & 0xff); break;
			case 0x5c: CHECK_B_SET(); OP_ANDA(op & 0xff); break;
			case 0x6c: CHECK_B_SET(); OP_ORA (op & 0xff); break;
			case 0x7c: CHECK_B_SET(); OP_EORA(op & 0xff); break;

			default:
				logerror(__FILE__ " FIXME unhandled op %04x @ %04x\n", op, GET_PPC_OFFS());
				break;
		}
		m_g_cycles--;
	}

	update_P();
}


void ssp1601_device::execute_set_input( int inputnum, int state )
{
	fatalerror("ssp1610: execute_set_input not implemented yet!\n");
}
