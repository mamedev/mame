/*
 * Samsung SSP1601 DSP emulator
 *
 * Copyright 2008, Grazvydas Ignotas
 *
 * This code is released under the MAME license.
 *
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

CPU_DISASSEMBLE( ssp1601 );

/* detect ops with unimplemented/invalid fields.
 * Useful for homebrew or if a new VR revision pops up. */
//#define DO_CHECKS



struct ssp1601_state_t
{
	PAIR gr[8];		/* general regs, some are 16bit, some 32bit */
	union {
			unsigned char r[8];             /* pointer registers, 4 for earch bank */
			struct {
					unsigned char r0[4];
					unsigned char r1[4];
			};
	};
	union {
			unsigned short RAM[256*2];      /* 2 256-word internal RAM banks */
			struct {
					unsigned short RAM0[256];
					unsigned short RAM1[256];
			};
	};
	UINT16 stack[6]; /* 6-level hardware stack */
	PAIR ppc;

	int g_cycles;

	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
};

INLINE ssp1601_state_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SSP1601);
	return (ssp1601_state_t *)downcast<legacy_cpu_device *>(device)->token();
}



// 0
#define rX     ssp1601_state->gr[SSP_X].w.h
#define rY     ssp1601_state->gr[SSP_Y].w.h
#define rA     ssp1601_state->gr[SSP_A].w.h
#define rST    ssp1601_state->gr[SSP_ST].w.h	// 4
#define rSTACK ssp1601_state->gr[SSP_STACK].w.h
#define rPC    ssp1601_state->gr[SSP_PC].w.h
#define rP     ssp1601_state->gr[SSP_P]

#define rAL    ssp1601_state->gr[SSP_A].w.l
#define rA32   ssp1601_state->gr[SSP_A].d
#define rIJ    ssp1601_state->r

#define IJind  (((op>>6)&4)|(op&3))

#define PPC    ssp1601_state->ppc.w.h

#define FETCH() ssp1601_state->direct->read_decrypted_word(rPC++ << 1)
#define PROGRAM_WORD(a) ssp1601_state->program->read_word((a) << 1)
#define GET_PPC_OFFS() PPC

#define REG_READ(ssp1601_state,r) (((r) <= 4) ? ssp1601_state->gr[r].w.h : reg_read_handlers[r](ssp1601_state, r))
#define REG_WRITE(ssp1601_state,r,d) { \
	int r1 = r; \
	if (r1 >= 4) reg_write_handlers[r1](ssp1601_state, r1,d); \
	else if (r1 > 0) ssp1601_state->gr[r1].w.h = d; \
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
		update_P(ssp1601_state); \
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

#define CHIP_NAME "SSP1601"

// -----------------------------------------------------
// register i/o handlers

static void update_P(ssp1601_state_t *ssp1601_state)
{
	int m1 = (signed short)rX;
	int m2 = (signed short)rY;
	rP.d = (m1 * m2 * 2);
}

static UINT32 read_unknown(ssp1601_state_t *ssp1601_state, int reg)
{
	logerror("%s:%i FIXME\n", __FILE__, __LINE__);
	return 0;
}

static void write_unknown(ssp1601_state_t *ssp1601_state, int reg, UINT32 d)
{
	logerror("%s:%i FIXME\n", __FILE__, __LINE__);
}

/* map EXT regs to virtual I/O range of 0x00-0x0f */
static UINT32 read_ext(ssp1601_state_t *ssp1601_state, int reg)
{
	reg &= 7;
	return ssp1601_state->io->read_word((reg << 1));
}

static void write_ext(ssp1601_state_t *ssp1601_state, int reg, UINT32 d)
{
	reg &= 7;
	ssp1601_state->io->write_word((reg << 1), d);
}

// 4
static void write_ST(ssp1601_state_t *ssp1601_state, int reg, UINT32 d)
{
	CHECK_ST(d);
	rST = d;
}

// 5
static UINT32 read_STACK(ssp1601_state_t *ssp1601_state, int reg)
{
	--rSTACK;
	if ((signed short)rSTACK < 0) {
		rSTACK = 5;
		logerror(__FILE__ " FIXME: stack underflow! (%i) @ %04x\n", rSTACK, GET_PPC_OFFS());
	}
	return ssp1601_state->stack[rSTACK];
}

static void write_STACK(ssp1601_state_t *ssp1601_state, int reg, UINT32 d)
{
	if (rSTACK >= 6) {
		logerror(__FILE__ " FIXME: stack overflow! (%i) @ %04x\n", rSTACK, GET_PPC_OFFS());
		rSTACK = 0;
	}
	ssp1601_state->stack[rSTACK++] = d;
}

// 6
static UINT32 read_PC(ssp1601_state_t *ssp1601_state, int reg)
{
	return rPC;
}

static void write_PC(ssp1601_state_t *ssp1601_state, int reg, UINT32 d)
{
	rPC = d;
	ssp1601_state->g_cycles--;
}

// 7
static UINT32 read_P(ssp1601_state_t *ssp1601_state, int reg)
{
	update_P(ssp1601_state);
	return rP.w.h;
}

// 15
static UINT32 read_AL(ssp1601_state_t *ssp1601_state, int reg)
{
	/* apparently reading AL causes some effect on EXT bus, VR depends on that.. */
	read_ext(ssp1601_state, reg);
	return rAL;
}

static void write_AL(ssp1601_state_t *ssp1601_state, int reg, UINT32 d)
{
	write_ext(ssp1601_state, reg, d);
	rAL = d;
}


typedef UINT32 (*read_func_t)(ssp1601_state_t *ssp1601_state, int reg);
typedef void (*write_func_t)(ssp1601_state_t *ssp1601_state, int reg, UINT32 d);

static const read_func_t reg_read_handlers[16] =
{
	read_unknown, read_unknown, read_unknown, read_unknown, // -, X, Y, A
	read_unknown,	// 4 ST
	read_STACK,
	read_PC,
	read_P,
	read_ext,	// 8
	read_ext,
	read_ext,
	read_ext,
	read_ext,	// 12
	read_ext,
	read_ext,
	read_AL
};

static const write_func_t reg_write_handlers[16] =
{
	write_unknown, write_unknown, write_unknown, write_unknown, // -, X, Y, A
	write_ST,
	write_STACK,
	write_PC,
	write_unknown,	// 7 P (not writable)
	write_ext,	// 8
	write_ext,
	write_ext,
	write_ext,
	write_ext,	// 12
	write_ext,
	write_ext,
	write_AL
};

// -----------------------------------------------------
// pointer register handlers

//
#define ptr1_read(ssp1601_state, op) ptr1_read_(ssp1601_state, op&3,(op>>6)&4,(op<<1)&0x18)

static UINT32 ptr1_read_(ssp1601_state_t *ssp1601_state, int ri, int isj2, int modi3)
{
	//int t = (op&3) | ((op>>6)&4) | ((op<<1)&0x18);
	UINT32 mask, add = 0, t = ri | isj2 | modi3;
	unsigned char *rp = NULL;
	switch (t)
	{
		// mod=0 (00)
		case 0x00:
		case 0x01:
		case 0x02: return ssp1601_state->RAM0[ssp1601_state->r0[t&3]];
		case 0x03: return ssp1601_state->RAM0[0];
		case 0x04:
		case 0x05:
		case 0x06: return ssp1601_state->RAM1[ssp1601_state->r1[t&3]];
		case 0x07: return ssp1601_state->RAM1[0];
		// mod=1 (01), "+!"
		case 0x08:
		case 0x09:
		case 0x0a: return ssp1601_state->RAM0[ssp1601_state->r0[t&3]++];
		case 0x0b: return ssp1601_state->RAM0[1];
		case 0x0c:
		case 0x0d:
		case 0x0e: return ssp1601_state->RAM1[ssp1601_state->r1[t&3]++];
		case 0x0f: return ssp1601_state->RAM1[1];
		// mod=2 (10), "-"
		case 0x10:
		case 0x11:
		case 0x12: rp = &ssp1601_state->r0[t&3]; t = ssp1601_state->RAM0[*rp];
		           if (!(rST&7)) { (*rp)--; return t; }
		           add = -1; goto modulo;
		case 0x13: return ssp1601_state->RAM0[2];
		case 0x14:
		case 0x15:
		case 0x16: rp = &ssp1601_state->r1[t&3]; t = ssp1601_state->RAM1[*rp];
		           if (!(rST&7)) { (*rp)--; return t; }
		           add = -1; goto modulo;
		case 0x17: return ssp1601_state->RAM1[2];
		// mod=3 (11), "+"
		case 0x18:
		case 0x19:
		case 0x1a: rp = &ssp1601_state->r0[t&3]; t = ssp1601_state->RAM0[*rp];
		           if (!(rST&7)) { (*rp)++; return t; }
		           add = 1; goto modulo;
		case 0x1b: return ssp1601_state->RAM0[3];
		case 0x1c:
		case 0x1d:
		case 0x1e: rp = &ssp1601_state->r1[t&3]; t = ssp1601_state->RAM1[*rp];
		           if (!(rST&7)) { (*rp)++; return t; }
		           add = 1; goto modulo;
		case 0x1f: return ssp1601_state->RAM1[3];
	}

	return 0;

modulo:
	mask = (1 << (rST&7)) - 1;
	*rp = (*rp & ~mask) | ((*rp + add) & mask);
	return t;
}

static void ptr1_write(ssp1601_state_t *ssp1601_state, int op, UINT32 d)
{
	int t = (op&3) | ((op>>6)&4) | ((op<<1)&0x18);
	switch (t)
	{
		// mod=0 (00)
		case 0x00:
		case 0x01:
		case 0x02: ssp1601_state->RAM0[ssp1601_state->r0[t&3]] = d; return;
		case 0x03: ssp1601_state->RAM0[0] = d; return;
		case 0x04:
		case 0x05:
		case 0x06: ssp1601_state->RAM1[ssp1601_state->r1[t&3]] = d; return;
		case 0x07: ssp1601_state->RAM1[0] = d; return;
		// mod=1 (01), "+!"
		// mod=3,      "+"
		case 0x08:
		case 0x09:
		case 0x0a: ssp1601_state->RAM0[ssp1601_state->r0[t&3]++] = d; return;
		case 0x0b: ssp1601_state->RAM0[1] = d; return;
		case 0x0c:
		case 0x0d:
		case 0x0e: ssp1601_state->RAM1[ssp1601_state->r1[t&3]++] = d; return;
		case 0x0f: ssp1601_state->RAM1[1] = d; return;
		// mod=2 (10), "-"
		case 0x10:
		case 0x11:
		case 0x12: ssp1601_state->RAM0[ssp1601_state->r0[t&3]--] = d; CHECK_RPL(); return;
		case 0x13: ssp1601_state->RAM0[2] = d; return;
		case 0x14:
		case 0x15:
		case 0x16: ssp1601_state->RAM1[ssp1601_state->r1[t&3]--] = d; CHECK_RPL(); return;
		case 0x17: ssp1601_state->RAM1[2] = d; return;
		// mod=3 (11), "+"
		case 0x18:
		case 0x19:
		case 0x1a: ssp1601_state->RAM0[ssp1601_state->r0[t&3]++] = d; CHECK_RPL(); return;
		case 0x1b: ssp1601_state->RAM0[3] = d; return;
		case 0x1c:
		case 0x1d:
		case 0x1e: ssp1601_state->RAM1[ssp1601_state->r1[t&3]++] = d; CHECK_RPL(); return;
		case 0x1f: ssp1601_state->RAM1[3] = d; return;
	}
}

static UINT32 ptr2_read(ssp1601_state_t *ssp1601_state, int op)
{
	int mv = 0, t = (op&3) | ((op>>6)&4) | ((op<<1)&0x18);
	switch (t)
	{
		// mod=0 (00)
		case 0x00:
		case 0x01:
		case 0x02: mv = ssp1601_state->RAM0[ssp1601_state->r0[t&3]]++; break;
		case 0x03: mv = ssp1601_state->RAM0[0]++; break;
		case 0x04:
		case 0x05:
		case 0x06: mv = ssp1601_state->RAM1[ssp1601_state->r1[t&3]]++; break;
		case 0x07: mv = ssp1601_state->RAM1[0]++; break;
		// mod=1 (01)
		case 0x0b: mv = ssp1601_state->RAM0[1]++; break;
		case 0x0f: mv = ssp1601_state->RAM1[1]++; break;
		// mod=2 (10)
		case 0x13: mv = ssp1601_state->RAM0[2]++; break;
		case 0x17: mv = ssp1601_state->RAM1[2]++; break;
		// mod=3 (11)
		case 0x1b: mv = ssp1601_state->RAM0[3]++; break;
		case 0x1f: mv = ssp1601_state->RAM1[3]++; break;
		default:   logerror(__FILE__ " FIXME: unimplemented mod in ((rX)) @ %04x\n", GET_PPC_OFFS());
		           return 0;
	}

	return PROGRAM_WORD(mv);
}


// -----------------------------------------------------


static CPU_INIT( ssp1601 )
{
	ssp1601_state_t *ssp1601_state = get_safe_token(device);

	device->save_item(NAME(rX));
	device->save_item(NAME(rY));
	device->save_item(NAME(rA32));
	device->save_item(NAME(rST));
	device->save_item(NAME(rSTACK));
	device->save_item(NAME(rPC));
	device->save_item(NAME(rP.d));
	device->save_item(NAME(PPC));
	device->save_item(NAME(ssp1601_state->stack));
	device->save_item(NAME(ssp1601_state->r));
	device->save_item(NAME(ssp1601_state->RAM));

	/* clear the state */
	memset(ssp1601_state, 0, sizeof(ssp1601_state_t));
	ssp1601_state->gr[0].w.h = 0xffff; // constant reg
	ssp1601_state->device = device;
	ssp1601_state->program = &device->space(AS_PROGRAM);
	ssp1601_state->direct = &ssp1601_state->program->direct();
	ssp1601_state->io = &device->space(AS_IO);

}

static CPU_EXIT( ssp1601 )
{
	/* nothing to do */
}

static CPU_RESET( ssp1601 )
{
	ssp1601_state_t *ssp1601_state = get_safe_token(device);

	rPC = 0x400;
	rSTACK = 0; // ? using ascending stack
	rST = 0;
}


static CPU_EXECUTE( ssp1601 )
{
	ssp1601_state_t *ssp1601_state = get_safe_token(device);

	while (ssp1601_state->g_cycles > 0)
	{
		int op;
		UINT32 tmpv;

		PPC = rPC;

		debugger_instruction_hook(device, rPC);

		op = FETCH();

		switch (op >> 9)
		{
			// ld d, s
			case 0x00:
				CHECK_B_SET();
				if (op == 0) break; // nop
				if (op == ((SSP_A<<4)|SSP_P)) { // A <- P
					update_P(ssp1601_state);
					rA32 = rP.d;
				}
				else
				{
					tmpv = REG_READ(ssp1601_state,op & 0x0f);
					REG_WRITE(ssp1601_state,(op & 0xf0) >> 4, tmpv);
				}
				break;

			// ld d, (ri)
			case 0x01: tmpv = ptr1_read(ssp1601_state, op); REG_WRITE(ssp1601_state, (op & 0xf0) >> 4, tmpv); break;

			// ld (ri), s
			case 0x02: tmpv = REG_READ(ssp1601_state,(op & 0xf0) >> 4); ptr1_write(ssp1601_state, op, tmpv); break;

			// ldi d, imm
			case 0x04: CHECK_10f(); tmpv = FETCH(); REG_WRITE(ssp1601_state, (op & 0xf0) >> 4, tmpv);ssp1601_state->g_cycles--; break;

			// ld d, ((ri))
			case 0x05: CHECK_MOD(); tmpv = ptr2_read(ssp1601_state, op); REG_WRITE(ssp1601_state, (op & 0xf0) >> 4, tmpv); ssp1601_state->g_cycles -= 2; break;

			// ldi (ri), imm
			case 0x06: tmpv = FETCH(); ptr1_write(ssp1601_state, op, tmpv); ssp1601_state->g_cycles--; break;

			// ld adr, a
			case 0x07: ssp1601_state->RAM[op & 0x1ff] = rA; break;

			// ld d, ri
			case 0x09: CHECK_MOD(); tmpv = rIJ[(op&3)|((op>>6)&4)]; REG_WRITE(ssp1601_state,(op & 0xf0) >> 4, tmpv); break;

			// ld ri, s
			case 0x0a: CHECK_MOD(); rIJ[(op&3)|((op>>6)&4)] = REG_READ(ssp1601_state,(op & 0xf0) >> 4); break;

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
				if (cond) { int new_PC = FETCH(); write_STACK(ssp1601_state, SSP_STACK, rPC); rPC = new_PC; }
				else rPC++;
				ssp1601_state->g_cycles--; // always 2 cycles
				break;
			}

			// ld d, (a)
			case 0x25:
				CHECK_10f();
				tmpv = PROGRAM_WORD(rA);
				REG_WRITE(ssp1601_state,(op & 0xf0) >> 4, tmpv);
				ssp1601_state->g_cycles -= 2; // 3 cycles total
				break;

			// bra cond, addr
			case 0x26: {
				int cond = 0;
				CHECK_00f();
				COND_CHECK
				if (cond) { rPC = FETCH(); }
				else rPC++;
				ssp1601_state->g_cycles--;
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
				update_P(ssp1601_state);
				rA32 -= rP.d;
				UPD_ACC_ZN
				rX = ptr1_read_(ssp1601_state, op&3, 0, (op<<1)&0x18);
				rY = ptr1_read_(ssp1601_state, (op>>4)&3, 4, (op>>3)&0x18);
				break;

			// mpya (rj), (ri), b
			case 0x4b:
				CHECK_B_CLEAR();
				update_P(ssp1601_state);
				rA32 += rP.d;
				UPD_ACC_ZN
				rX = ptr1_read_(ssp1601_state, op&3, 0, (op<<1)&0x18);
				rY = ptr1_read_(ssp1601_state, (op>>4)&3, 4, (op>>3)&0x18);
				break;

			// mld (rj), (ri), b
			case 0x5b:
				CHECK_B_CLEAR();
				rA32 = 0;
				rST &= 0x0fff;
				rST |= SSP_FLAG_Z;
				rX = ptr1_read_(ssp1601_state, op&3, 0, (op<<1)&0x18);
				rY = ptr1_read_(ssp1601_state, (op>>4)&3, 4, (op>>3)&0x18);
				break;

			// OP a, s
			case 0x10: CHECK_1f0(); OP_CHECK32(OP_SUBA32); tmpv = REG_READ(ssp1601_state,op & 0x0f); OP_SUBA(tmpv); break;
			case 0x30: CHECK_1f0(); OP_CHECK32(OP_CMPA32); tmpv = REG_READ(ssp1601_state,op & 0x0f); OP_CMPA(tmpv); break;
			case 0x40: CHECK_1f0(); OP_CHECK32(OP_ADDA32); tmpv = REG_READ(ssp1601_state,op & 0x0f); OP_ADDA(tmpv); break;
			case 0x50: CHECK_1f0(); OP_CHECK32(OP_ANDA32); tmpv = REG_READ(ssp1601_state,op & 0x0f); OP_ANDA(tmpv); break;
			case 0x60: CHECK_1f0(); OP_CHECK32(OP_ORA32 ); tmpv = REG_READ(ssp1601_state,op & 0x0f); OP_ORA (tmpv); break;
			case 0x70: CHECK_1f0(); OP_CHECK32(OP_EORA32); tmpv = REG_READ(ssp1601_state,op & 0x0f); OP_EORA(tmpv); break;

			// OP a, (ri)
			case 0x11: CHECK_0f0(); tmpv = ptr1_read(ssp1601_state, op); OP_SUBA(tmpv); break;
			case 0x31: CHECK_0f0(); tmpv = ptr1_read(ssp1601_state, op); OP_CMPA(tmpv); break;
			case 0x41: CHECK_0f0(); tmpv = ptr1_read(ssp1601_state, op); OP_ADDA(tmpv); break;
			case 0x51: CHECK_0f0(); tmpv = ptr1_read(ssp1601_state, op); OP_ANDA(tmpv); break;
			case 0x61: CHECK_0f0(); tmpv = ptr1_read(ssp1601_state, op); OP_ORA (tmpv); break;
			case 0x71: CHECK_0f0(); tmpv = ptr1_read(ssp1601_state, op); OP_EORA(tmpv); break;

			// OP a, adr
			case 0x03: tmpv = ssp1601_state->RAM[op & 0x1ff]; OP_LDA (tmpv); break;
			case 0x13: tmpv = ssp1601_state->RAM[op & 0x1ff]; OP_SUBA(tmpv); break;
			case 0x33: tmpv = ssp1601_state->RAM[op & 0x1ff]; OP_CMPA(tmpv); break;
			case 0x43: tmpv = ssp1601_state->RAM[op & 0x1ff]; OP_ADDA(tmpv); break;
			case 0x53: tmpv = ssp1601_state->RAM[op & 0x1ff]; OP_ANDA(tmpv); break;
			case 0x63: tmpv = ssp1601_state->RAM[op & 0x1ff]; OP_ORA (tmpv); break;
			case 0x73: tmpv = ssp1601_state->RAM[op & 0x1ff]; OP_EORA(tmpv); break;

			// OP a, imm
			case 0x14: CHECK_IMM16(); tmpv = FETCH(); OP_SUBA(tmpv); ssp1601_state->g_cycles--; break;
			case 0x34: CHECK_IMM16(); tmpv = FETCH(); OP_CMPA(tmpv); ssp1601_state->g_cycles--; break;
			case 0x44: CHECK_IMM16(); tmpv = FETCH(); OP_ADDA(tmpv); ssp1601_state->g_cycles--; break;
			case 0x54: CHECK_IMM16(); tmpv = FETCH(); OP_ANDA(tmpv); ssp1601_state->g_cycles--; break;
			case 0x64: CHECK_IMM16(); tmpv = FETCH(); OP_ORA (tmpv); ssp1601_state->g_cycles--; break;
			case 0x74: CHECK_IMM16(); tmpv = FETCH(); OP_EORA(tmpv); ssp1601_state->g_cycles--; break;

			// OP a, ((ri))
			case 0x15: CHECK_MOD(); tmpv = ptr2_read(ssp1601_state, op); OP_SUBA(tmpv); ssp1601_state->g_cycles -= 2; break;
			case 0x35: CHECK_MOD(); tmpv = ptr2_read(ssp1601_state, op); OP_CMPA(tmpv); ssp1601_state->g_cycles -= 2; break;
			case 0x45: CHECK_MOD(); tmpv = ptr2_read(ssp1601_state, op); OP_ADDA(tmpv); ssp1601_state->g_cycles -= 2; break;
			case 0x55: CHECK_MOD(); tmpv = ptr2_read(ssp1601_state, op); OP_ANDA(tmpv); ssp1601_state->g_cycles -= 2; break;
			case 0x65: CHECK_MOD(); tmpv = ptr2_read(ssp1601_state, op); OP_ORA (tmpv); ssp1601_state->g_cycles -= 2; break;
			case 0x75: CHECK_MOD(); tmpv = ptr2_read(ssp1601_state, op); OP_EORA(tmpv); ssp1601_state->g_cycles -= 2; break;

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
		ssp1601_state->g_cycles--;
	}

	update_P(ssp1601_state);
}


/**************************************************************************
 * MAME interface
 **************************************************************************/

static CPU_SET_INFO( ssp1601 )
{
	ssp1601_state_t *ssp1601_state = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */

		case CPUINFO_INT_REGISTER + SSP_X:				rX = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP_Y:				rY = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP_A:				rA32 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP_ST:				rST = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP_STACK:			rSTACK = info->i;						break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SSP_PC:				rPC = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP_P:				rP.d = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP_STACK0:			ssp1601_state->stack[0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SSP_STACK1:			ssp1601_state->stack[1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SSP_STACK2:			ssp1601_state->stack[2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SSP_STACK3:			ssp1601_state->stack[3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SSP_STACK4:			ssp1601_state->stack[4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SSP_STACK5:			ssp1601_state->stack[5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SSP_PR0:			ssp1601_state->r[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + SSP_PR1:			ssp1601_state->r[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + SSP_PR2:			ssp1601_state->r[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + SSP_PR3:			ssp1601_state->r[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + SSP_PR4:			ssp1601_state->r[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + SSP_PR5:			ssp1601_state->r[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + SSP_PR6:			ssp1601_state->r[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + SSP_PR7:			ssp1601_state->r[7] = info->i;					break;

//      case CPUINFO_INT_INPUT_STATE + 0:               set_irq_line(0, info->i);               break;
//      case CPUINFO_INT_INPUT_STATE + 1:               set_irq_line(1, info->i);               break;
//      case CPUINFO_INT_INPUT_STATE + 2:               set_irq_line(2, info->i);               break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( ssp1601 )
{
	ssp1601_state_t *ssp1601_state = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(ssp1601_state_t);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 3;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 4;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:					/* not implemented */				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_REGISTER + SSP_R0:				info->i = ssp1601_state->gr[0].w.h;			break;
		case CPUINFO_INT_REGISTER + SSP_X:				info->i = rX;							break;
		case CPUINFO_INT_REGISTER + SSP_Y:				info->i = rY;							break;
		case CPUINFO_INT_REGISTER + SSP_A:				info->i = rA32;							break;
		case CPUINFO_INT_REGISTER + SSP_ST:				info->i = rST;							break;
		case CPUINFO_INT_REGISTER + SSP_STACK:			info->i = rSTACK;						break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SSP_PC:				info->i = rPC;							break;
		case CPUINFO_INT_REGISTER + SSP_P:				info->i = rP.d;							break;
		case CPUINFO_INT_REGISTER + SSP_STACK0:			info->i = ssp1601_state->stack[0];				break;
		case CPUINFO_INT_REGISTER + SSP_STACK1:			info->i = ssp1601_state->stack[1];				break;
		case CPUINFO_INT_REGISTER + SSP_STACK2:			info->i = ssp1601_state->stack[2];				break;
		case CPUINFO_INT_REGISTER + SSP_STACK3:			info->i = ssp1601_state->stack[3];				break;
		case CPUINFO_INT_REGISTER + SSP_STACK4:			info->i = ssp1601_state->stack[4];				break;
		case CPUINFO_INT_REGISTER + SSP_STACK5:			info->i = ssp1601_state->stack[5];				break;
		case CPUINFO_INT_REGISTER + SSP_PR0:			info->i = ssp1601_state->r[0];					break;
		case CPUINFO_INT_REGISTER + SSP_PR1:			info->i = ssp1601_state->r[1];					break;
		case CPUINFO_INT_REGISTER + SSP_PR2:			info->i = ssp1601_state->r[2];					break;
		case CPUINFO_INT_REGISTER + SSP_PR3:			info->i = ssp1601_state->r[3];					break;
		case CPUINFO_INT_REGISTER + SSP_PR4:			info->i = ssp1601_state->r[4];					break;
		case CPUINFO_INT_REGISTER + SSP_PR5:			info->i = ssp1601_state->r[5];					break;
		case CPUINFO_INT_REGISTER + SSP_PR6:			info->i = ssp1601_state->r[6];					break;
		case CPUINFO_INT_REGISTER + SSP_PR7:			info->i = ssp1601_state->r[7];					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(ssp1601);	break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(ssp1601);			break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(ssp1601);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(ssp1601);			break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(ssp1601);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(ssp1601);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &ssp1601_state->g_cycles;			break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map16 = NULL;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_IO:      info->internal_map16 = NULL;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:						strcpy(info->s, CHIP_NAME);									break;
		case CPUINFO_STR_FAMILY:				strcpy(info->s, "SSP1601 DSP");								break;
		case CPUINFO_STR_VERSION:				strcpy(info->s, "1.0");										break;
		case CPUINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);									break;
		case CPUINFO_STR_CREDITS:				strcpy(info->s, "Copyright Grazvydas Ignotas");				break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c", (rST&SSP_FLAG_N)?'N':'.', (rST&SSP_FLAG_V)?'V':'.',
				(rST&SSP_FLAG_Z)?'Z':'.', (rST&SSP_FLAG_L)?'L':'.');
			break;

		case CPUINFO_STR_REGISTER + SSP_R0:			sprintf(info->s, "REG0  :%04X", ssp1601_state->gr[0].w.h);			break;
		case CPUINFO_STR_REGISTER + SSP_X:			sprintf(info->s, "X  :%04X", rX);							break;
		case CPUINFO_STR_REGISTER + SSP_Y:			sprintf(info->s, "Y  :%04X", rY);							break;
		case CPUINFO_STR_REGISTER + SSP_A:			sprintf(info->s, "A  :%08X", rA32);							break;
		case CPUINFO_STR_REGISTER + SSP_ST:			sprintf(info->s, "ST  :%04X", rST);							break;
		case CPUINFO_STR_REGISTER + SSP_STACK:		sprintf(info->s, "STACK  :%04X", rSTACK);					break;
		case CPUINFO_STR_REGISTER + SSP_PC: 		sprintf(info->s, "PC  :%04X", rPC);							break;
		case CPUINFO_STR_REGISTER + SSP_P:			sprintf(info->s, "P  :%08X", rP.d);							break;
		case CPUINFO_STR_REGISTER + SSP_STACK0:		sprintf(info->s, "STACK0  :%04X", ssp1601_state->stack[0]);		break;
		case CPUINFO_STR_REGISTER + SSP_STACK1:		sprintf(info->s, "STACK1  :%04X", ssp1601_state->stack[1]);		break;
		case CPUINFO_STR_REGISTER + SSP_STACK2:		sprintf(info->s, "STACK2  :%04X", ssp1601_state->stack[2]);		break;
		case CPUINFO_STR_REGISTER + SSP_STACK3:		sprintf(info->s, "STACK3  :%04X", ssp1601_state->stack[3]);		break;
		case CPUINFO_STR_REGISTER + SSP_STACK4:		sprintf(info->s, "STACK4  :%04X", ssp1601_state->stack[4]);		break;
		case CPUINFO_STR_REGISTER + SSP_STACK5:		sprintf(info->s, "STACK5  :%04X", ssp1601_state->stack[5]);		break;
		case CPUINFO_STR_REGISTER + SSP_PR0:		sprintf(info->s, "R0  :%02X", ssp1601_state->r[0]);				break;
		case CPUINFO_STR_REGISTER + SSP_PR1:		sprintf(info->s, "R1  :%02X", ssp1601_state->r[1]);				break;
		case CPUINFO_STR_REGISTER + SSP_PR2:		sprintf(info->s, "R2  :%02X", ssp1601_state->r[2]);				break;
		case CPUINFO_STR_REGISTER + SSP_PR3:		sprintf(info->s, "R3  :%02X", ssp1601_state->r[3]);				break;
		case CPUINFO_STR_REGISTER + SSP_PR4:		sprintf(info->s, "R4  :%02X", ssp1601_state->r[4]);				break;
		case CPUINFO_STR_REGISTER + SSP_PR5:		sprintf(info->s, "R5  :%02X", ssp1601_state->r[5]);				break;
		case CPUINFO_STR_REGISTER + SSP_PR6:		sprintf(info->s, "R6  :%02X", ssp1601_state->r[6]);				break;
		case CPUINFO_STR_REGISTER + SSP_PR7:		sprintf(info->s, "R7  :%02X", ssp1601_state->r[7]);				break;
	}
}

// vim:ts=4


DEFINE_LEGACY_CPU_DEVICE(SSP1601, ssp1601);
