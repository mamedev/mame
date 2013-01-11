/***************************************************************************

    i860dec.c

    Execution engine for the Intel i860 emulator.

    Copyright (C) 1995-present Jason Eckhardt (jle@rice.edu)
    Released for general non-commercial use under the MAME license
    with the additional requirement that you are free to use and
    redistribute this code in modified or unmodified form, provided
    you list me in the credits.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/*
 * References:
 *  `i860 Microprocessor Programmer's Reference Manual', Intel, 1990.
 *
 * This code was originally written by Jason Eckhardt as part of an
 * emulator for some i860-based Unix workstations (early 1990's) such
 * as the Stardent Vistra 800 series and the OkiStation/i860 7300 series.
 * The code you are reading now is the i860 CPU portion only, which has
 * been adapted to (and simplified for) MAME.
 * MAME-specific notes:
 * - i860XR emulation only (i860XP unnecessary for MAME).
 * - No emulation of data and instruction caches (unnecessary for MAME version).
 * - No emulation of DIM mode or CS8 mode (unnecessary for MAME version).
 * - No BL/IL/locked sequences (unnecessary for MAME).
 * - Emulate only the i860's LSB-first mode (BE = 0).
 * Generic notes:
 * - There is some amount of code duplication (e.g., see the
 *   various insn_* routines for the branches and FP routines) that
 *   could be eliminated.
 * - The host's floating point types are used to emulate the i860's
 *   floating point.  Should probably be made machine independent by
 *   using an IEEE FP emulation library.  On the other hand, most machines
 *   today also use IEEE FP.
 *
 */
#include "i860.h"
#include <math.h>


#undef HOST_MSB

#undef  TRACE_RDWR_MEM
#undef  TRACE_ADDR_TRANSLATION
#undef  TRACE_PAGE_FAULT
#define TRACE_UNDEFINED_I860
#undef  TRACE_EXT_INT
#define TRACE_UNALIGNED_MEM



#define i860s i860_state_t


/* Prototypes.  */
static void decode_exec (i860s *, UINT32, UINT32);
static UINT32 get_address_translation (i860s *, UINT32, int, int);
static UINT32 readmemi_emu (i860s *cpustate, UINT32, int);

//static void debugger (i860s *cpustate);
//static void disasm (i860s *cpustate, UINT32, int);
//static void dump_state (i860s *cpustate);




/* Defines for pending_trap.  */
enum {
	TRAP_NORMAL        = 0x01,
	TRAP_IN_DELAY_SLOT = 0x02,
	TRAP_WAS_EXTERNAL  = 0x04
};




/* Get/set general register value -- watch for r0 on writes.  */
#define get_iregval(gr)       (cpustate->iregs[(gr)])
#define set_iregval(gr, val)  (cpustate->iregs[(gr)] = ((gr) == 0 ? 0 : (val)))

INLINE float get_fregval_s (i860s *cpustate, int fr)
{
	float f;
	UINT32 x;
	UINT8 *tp;
	fr = 31 - fr;
	tp = (UINT8 *)(&cpustate->frg[fr * 4]);
	x = ((UINT32)tp[0] << 24) | ((UINT32)tp[1] << 16) |
		((UINT32)tp[2] << 8) | ((UINT32)tp[3]);
	f = *(float *)(&x);
	return f;
}

INLINE double get_fregval_d (i860s *cpustate, int fr)
{
	double d;
	UINT64 x;
	UINT8 *tp;
	fr = 31 - (fr + 1);
	tp = (UINT8 *)(&cpustate->frg[fr * 4]);
	x = ((UINT64)tp[0] << 56) | ((UINT64)tp[1] << 48) |
		((UINT64)tp[2] << 40) | ((UINT64)tp[3] << 32) |
		((UINT64)tp[4] << 24) | ((UINT64)tp[5] << 16) |
		((UINT64)tp[6] << 8) | ((UINT64)tp[7]);
	d = *(double *)(&x);
	return d;
}

INLINE void set_fregval_s (i860s *cpustate, int fr, float s)
{
	UINT8 *f = (UINT8 *)&s;
	UINT8 *tp;
	int newfr = 31 - fr;
	float jj = s;
	tp = (UINT8 *)(&cpustate->frg[newfr * 4]);

	f = (UINT8 *)(&jj);
	if (fr == 0 || fr == 1)
	{
		tp[0] = 0; tp[1] = 0; tp[2] = 0; tp[3] = 0;
	}
	else
	{
#ifndef HOST_MSB
		tp[0] = f[3]; tp[1] = f[2]; tp[2] = f[1]; tp[3] = f[0];
#else
		tp[0] = f[0]; tp[1] = f[1]; tp[2] = f[2]; tp[3] = f[3];
#endif
	}
}

INLINE void set_fregval_d (i860s *cpustate, int fr, double d)
{
	UINT8 *f = (UINT8 *)&d;
	UINT8 *tp;
	int newfr = 31 - (fr + 1);
	double jj = d;
	tp = (UINT8 *)(&cpustate->frg[newfr * 4]);

	f = (UINT8 *)(&jj);

	if (fr == 0)
	{
		tp[0] = 0; tp[1] = 0; tp[2] = 0; tp[3] = 0;
		tp[4] = 0; tp[5] = 0; tp[6] = 0; tp[7] = 0;
	}
	else
	{
#ifndef HOST_MSB
		tp[0] = f[7]; tp[1] = f[6]; tp[2] = f[5]; tp[3] = f[4];
		tp[4] = f[3]; tp[5] = f[2]; tp[6] = f[1]; tp[7] = f[0];
#else
		tp[0] = f[0]; tp[1] = f[1]; tp[2] = f[2]; tp[3] = f[3];
		tp[4] = f[4]; tp[5] = f[5]; tp[6] = f[6]; tp[7] = f[7];
#endif
	}
}


/* Macros for accessing register fields in instruction word.  */
#define get_isrc1(bits) (((bits) >> 11) & 0x1f)
#define get_isrc2(bits) (((bits) >> 21) & 0x1f)
#define get_idest(bits) (((bits) >> 16) & 0x1f)
#define get_fsrc1(bits) (((bits) >> 11) & 0x1f)
#define get_fsrc2(bits) (((bits) >> 21) & 0x1f)
#define get_fdest(bits) (((bits) >> 16) & 0x1f)
#define get_creg(bits) (((bits) >> 21) & 0x7)

/* Macros for accessing immediate fields.  */
/* 16-bit immediate.  */
#define get_imm16(insn) ((insn) & 0xffff)

/* Control register numbers.  */
enum {
	CR_FIR     = 0,
	CR_PSR     = 1,
	CR_DIRBASE = 2,
	CR_DB      = 3,
	CR_FSR     = 4,
	CR_EPSR    = 5
};

/* A mask for all the trap bits of the PSR (FT, DAT, IAT, IN, IT, or
   bits [12..8]).  */
#define PSR_ALL_TRAP_BITS_MASK 0x00001f00

/* A mask for PSR bits which can only be changed from supervisor level.  */
#define PSR_SUPERVISOR_ONLY_MASK 0x0000fff3


/* PSR: BR flag (PSR[0]):  set/get.  */
#define GET_PSR_BR()  ((cpustate->cregs[CR_PSR] >> 0) & 1)
#define SET_PSR_BR(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 0)) | (((val) & 1) << 0))

/* PSR: BW flag (PSR[1]):  set/get.  */
#define GET_PSR_BW()  ((cpustate->cregs[CR_PSR] >> 1) & 1)
#define SET_PSR_BW(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 1)) | (((val) & 1) << 1))

/* PSR: Shift count (PSR[21..17]):  set/get.  */
#define GET_PSR_SC()  ((cpustate->cregs[CR_PSR] >> 17) & 0x1f)
#define SET_PSR_SC(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~0x003e0000) | (((val) & 0x1f) << 17))

/* PSR: CC flag (PSR[2]):  set/get.  */
#define GET_PSR_CC()  ((cpustate->cregs[CR_PSR] >> 2) & 1)
#define SET_PSR_CC(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 2)) | (((val) & 1) << 2))

/* PSR: IT flag (PSR[8]):  set/get.  */
#define GET_PSR_IT()  ((cpustate->cregs[CR_PSR] >> 8) & 1)
#define SET_PSR_IT(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 8)) | (((val) & 1) << 8))

/* PSR: IN flag (PSR[9]):  set/get.  */
#define GET_PSR_IN()  ((cpustate->cregs[CR_PSR] >> 9) & 1)
#define SET_PSR_IN(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 9)) | (((val) & 1) << 9))

/* PSR: IAT flag (PSR[10]):  set/get.  */
#define GET_PSR_IAT()  ((cpustate->cregs[CR_PSR] >> 10) & 1)
#define SET_PSR_IAT(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 10)) | (((val) & 1) << 10))

/* PSR: DAT flag (PSR[11]):  set/get.  */
#define GET_PSR_DAT()  ((cpustate->cregs[CR_PSR] >> 11) & 1)
#define SET_PSR_DAT(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 11)) | (((val) & 1) << 11))

/* PSR: FT flag (PSR[12]):  set/get.  */
#define GET_PSR_FT()  ((cpustate->cregs[CR_PSR] >> 12) & 1)
#define SET_PSR_FT(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 12)) | (((val) & 1) << 12))

/* PSR: DS flag (PSR[13]):  set/get.  */
#define GET_PSR_DS()  ((cpustate->cregs[CR_PSR] >> 13) & 1)
#define SET_PSR_DS(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 13)) | (((val) & 1) << 13))

/* PSR: DIM flag (PSR[14]):  set/get.  */
#define GET_PSR_DIM()  ((cpustate->cregs[CR_PSR] >> 14) & 1)
#define SET_PSR_DIM(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 14)) | (((val) & 1) << 14))

/* PSR: LCC (PSR[3]):  set/get.  */
#define GET_PSR_LCC()  ((cpustate->cregs[CR_PSR] >> 3) & 1)
#define SET_PSR_LCC(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 3)) | (((val) & 1) << 3))

/* PSR: IM (PSR[4]):  set/get.  */
#define GET_PSR_IM()  ((cpustate->cregs[CR_PSR] >> 4) & 1)
#define SET_PSR_IM(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 4)) | (((val) & 1) << 4))

/* PSR: PIM (PSR[5]):  set/get.  */
#define GET_PSR_PIM()  ((cpustate->cregs[CR_PSR] >> 5) & 1)
#define SET_PSR_PIM(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 5)) | (((val) & 1) << 5))

/* PSR: U (PSR[6]):  set/get.  */
#define GET_PSR_U()  ((cpustate->cregs[CR_PSR] >> 6) & 1)
#define SET_PSR_U(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 6)) | (((val) & 1) << 6))

/* PSR: PU (PSR[7]):  set/get.  */
#define GET_PSR_PU()  ((cpustate->cregs[CR_PSR] >> 7) & 1)
#define SET_PSR_PU(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~(1 << 7)) | (((val) & 1) << 7))

/* PSR: Pixel size (PSR[23..22]):  set/get.  */
#define GET_PSR_PS()  ((cpustate->cregs[CR_PSR] >> 22) & 0x3)
#define SET_PSR_PS(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~0x00c00000) | (((val) & 0x3) << 22))

/* PSR: Pixel mask (PSR[31..24]):  set/get.  */
#define GET_PSR_PM()  ((cpustate->cregs[CR_PSR] >> 24) & 0xff)
#define SET_PSR_PM(val)  (cpustate->cregs[CR_PSR] = (cpustate->cregs[CR_PSR] & ~0xff000000) | (((val) & 0xff) << 24))

/* EPSR: WP bit (EPSR[14]):  set/get.  */
#define GET_EPSR_WP()  ((cpustate->cregs[CR_EPSR] >> 14) & 1)
#define SET_EPSR_WP(val)  (cpustate->cregs[CR_EPSR] = (cpustate->cregs[CR_EPSR] & ~(1 << 14)) | (((val) & 1) << 14))

/* EPSR: INT bit (EPSR[17]):  set/get.  */
#define GET_EPSR_INT()  ((cpustate->cregs[CR_EPSR] >> 17) & 1)
#define SET_EPSR_INT(val)  (cpustate->cregs[CR_EPSR] = (cpustate->cregs[CR_EPSR] & ~(1 << 17)) | (((val) & 1) << 17))


/* EPSR: OF flag (EPSR[24]):  set/get.  */
#define GET_EPSR_OF()  ((cpustate->cregs[CR_EPSR] >> 24) & 1)
#define SET_EPSR_OF(val)  (cpustate->cregs[CR_EPSR] = (cpustate->cregs[CR_EPSR] & ~(1 << 24)) | (((val) & 1) << 24))

/* EPSR: BE flag (EPSR[23]):  set/get.  */
#define GET_EPSR_BE()  ((cpustate->cregs[CR_EPSR] >> 23) & 1)
#define SET_EPSR_BE(val)  (cpustate->cregs[CR_EPSR] = (cpustate->cregs[CR_EPSR] & ~(1 << 23)) | (((val) & 1) << 23))

/* DIRBASE: ATE bit (DIRBASE[0]):  get.  */
#define GET_DIRBASE_ATE()  (cpustate->cregs[CR_DIRBASE] & 1)

/* DIRBASE: CS8 bit (DIRBASE[7]):  get.  */
#define GET_DIRBASE_CS8()  ((cpustate->cregs[CR_DIRBASE] >> 7) & 1)

/* FSR: FTE bit (FSR[5]):  set/get.  */
#define GET_FSR_FTE()  ((cpustate->cregs[CR_FSR] >> 5) & 1)
#define SET_FSR_FTE(val)  (cpustate->cregs[CR_FSR] = (cpustate->cregs[CR_FSR] & ~(1 << 5)) | (((val) & 1) << 5))

/* FSR: SE bit (FSR[8]):  set/get.  */
#define GET_FSR_SE()  ((cpustate->cregs[CR_FSR] >> 8) & 1)
#define SET_FSR_SE(val)  (cpustate->cregs[CR_FSR] = (cpustate->cregs[CR_FSR] & ~(1 << 8)) | (((val) & 1) << 8))


/*
static int has_delay_slot(UINT32 insn)
{
    int opc = (insn >> 26) & 0x3f;
    if (opc == 0x10 || opc == 0x1a || opc == 0x1b || opc == 0x1d ||
        opc == 0x1f || opc == 0x2d || (opc == 0x13 && (insn & 3) == 2))
    return 1;
    return 0;
}
*/

/* This is the external interface for asserting/deasserting pins on
   the i860.  */
void i860_set_pin (device_t *device, int pin, int val)
{
	i860s *cpustate = get_safe_token(device);
	if (pin == DEC_PIN_BUS_HOLD)
		cpustate->pin_bus_hold = val;
	else if (pin == DEC_PIN_RESET)
		cpustate->pin_reset = val;
	else
		assert (0);
}


/* This is the external interface for indicating an external interrupt
   to the i860.  */
void i860_gen_interrupt (i860s *cpustate)
{
	/* If interrupts are enabled, then set PSR.IN and prepare for trap.
	   Otherwise, the external interrupt is ignored.  We also set
	   bit EPSR.INT (which tracks the INT pin).  */
	if (GET_PSR_IM ())
	{
		SET_PSR_IN (1);
		SET_EPSR_INT (1);
		cpustate->pending_trap = TRAP_WAS_EXTERNAL;
	}

#ifdef TRACE_EXT_INT
	fprintf (stderr, "i860_gen_interrupt: External interrupt received ");
	if (GET_PSR_IM ())
		fprintf (stderr, "[PSR.IN set, preparing to trap]\n");
	else
		fprintf (stderr, "[ignored (interrupts disabled)]\n");
#endif
}


/* Fetch instructions from instruction cache.
   Note: The instruction cache is not implemented for MAME version,
   this just fetches and returns 1 instruction from memory.  */
static UINT32 ifetch (i860s *cpustate, UINT32 pc)
{
	UINT32 phys_pc = 0;
	UINT32 w1 = 0;

	/* If virtual mode, get translation.  */
	if (GET_DIRBASE_ATE ())
	{
		phys_pc = get_address_translation (cpustate, pc, 0  /* is_dataref */, 0 /* is_write */);
		cpustate->exiting_ifetch = 0;
		if (cpustate->pending_trap && (GET_PSR_DAT () || GET_PSR_IAT ()))
		{
			cpustate->exiting_ifetch = 1;
			return 0xffeeffee;
		}
	}
	else
		phys_pc = pc;

	/* Since i860 instructions are always stored LSB first (regardless of
	   the BE bit), we need to adjust the instruction below on MSB hosts.  */
	w1 = cpustate->program->read_dword(phys_pc);
#ifdef HOST_MSB
	BYTE_REV32 (w1);
#endif /* HOST_MSB.  */
	return w1;
}


/* Given a virtual address, perform the i860 address translation and
   return the corresponding physical address.
     vaddr:      virtual address
     is_dataref: 1 = load/store, 0 = instruction fetch.
     is_write:   1 = writing to vaddr, 0 = reading from vaddr
   The last two arguments are only used to determine what types
   of traps should be taken.

   Page tables must always be in memory (not cached).  So the routine
   here only accesses memory.  */
static UINT32 get_address_translation (i860s *cpustate, UINT32 vaddr, int is_dataref, int is_write)
{
	UINT32 vdir = (vaddr >> 22) & 0x3ff;
	UINT32 vpage = (vaddr >> 12) & 0x3ff;
	UINT32 voffset = vaddr & 0xfff;
	UINT32 dtb = (cpustate->cregs[CR_DIRBASE]) & 0xfffff000;
	UINT32 pg_dir_entry_a = 0;
	UINT32 pg_dir_entry = 0;
	UINT32 pg_tbl_entry_a = 0;
	UINT32 pg_tbl_entry = 0;
	UINT32 pfa1 = 0;
	UINT32 pfa2 = 0;
	UINT32 ret = 0;
	UINT32 ttpde = 0;
	UINT32 ttpte = 0;

	assert (GET_DIRBASE_ATE ());

	/* Get page directory entry at DTB:DIR:00.  */
	pg_dir_entry_a = dtb | (vdir << 2);
	pg_dir_entry = cpustate->program->read_dword(pg_dir_entry_a);
#ifdef HOST_MSB
	BYTE_REV32 (pg_dir_entry);
#endif

	/* Check for non-present PDE.  */
	if (!(pg_dir_entry & 1))
	{
		/* PDE is not present, generate DAT or IAT.  */
		if (is_dataref)
			SET_PSR_DAT (1);
		else
			SET_PSR_IAT (1);
		cpustate->pending_trap = 1;

		/* Dummy return.  */
		return 0;
	}

	/* PDE Check for write protection violations.  */
	if (is_write && is_dataref
		&& !(pg_dir_entry & 2)                  /* W = 0.  */
		&& (GET_PSR_U () || GET_EPSR_WP ()))   /* PSR_U = 1 or EPSR_WP = 1.  */
	{
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		/* Dummy return.  */
		return 0;
	}

	/* PDE Check for user-mode access to supervisor pages.  */
	if (GET_PSR_U ()
		&& !(pg_dir_entry & 4))                 /* U = 0.  */
	{
		if (is_dataref)
			SET_PSR_DAT (1);
		else
			SET_PSR_IAT (1);
		cpustate->pending_trap = 1;
		/* Dummy return.  */
		return 0;
	}

	/* FIXME: How exactly to handle A check/update?.  */

	/* Get page table entry at PFA1:PAGE:00.  */
	pfa1 = pg_dir_entry & 0xfffff000;
	pg_tbl_entry_a = pfa1 | (vpage << 2);
	pg_tbl_entry = cpustate->program->read_dword(pg_tbl_entry_a);
#ifdef HOST_MSB
	BYTE_REV32 (pg_tbl_entry);
#endif

	/* Check for non-present PTE.  */
	if (!(pg_tbl_entry & 1))
	{
		/* PTE is not present, generate DAT or IAT.  */
		if (is_dataref)
			SET_PSR_DAT (1);
		else
			SET_PSR_IAT (1);
		cpustate->pending_trap = 1;

		/* Dummy return.  */
		return 0;
	}

	/* PTE Check for write protection violations.  */
	if (is_write && is_dataref
		&& !(pg_tbl_entry & 2)                  /* W = 0.  */
		&& (GET_PSR_U () || GET_EPSR_WP ()))   /* PSR_U = 1 or EPSR_WP = 1.  */
	{
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		/* Dummy return.  */
		return 0;
	}

	/* PTE Check for user-mode access to supervisor pages.  */
	if (GET_PSR_U ()
		&& !(pg_tbl_entry & 4))                 /* U = 0.  */
	{
		if (is_dataref)
			SET_PSR_DAT (1);
		else
			SET_PSR_IAT (1);
		cpustate->pending_trap = 1;
		/* Dummy return.  */
		return 0;
	}

	/* Update A bit and check D bit.  */
	ttpde = pg_dir_entry | 0x20;
	ttpte = pg_tbl_entry | 0x20;
#ifdef HOST_MSB
	BYTE_REV32 (ttpde);
	BYTE_REV32 (ttpte);
#endif
	cpustate->program->write_dword(pg_dir_entry_a, ttpde);
	cpustate->program->write_dword(pg_tbl_entry_a, ttpte);

	if (is_write && is_dataref && (pg_tbl_entry & 0x40) == 0)
	{
		/* fprintf(stderr, "DAT trap on write without dirty bit v0x%08x/p0x%08x\n",
		   vaddr, (pg_tbl_entry & ~0xfff)|voffset); */
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		/* Dummy return.  */
		return 0;
	}

	pfa2 = (pg_tbl_entry & 0xfffff000);
	ret = pfa2 | voffset;

#ifdef TRACE_ADDR_TRANSLATION
	fprintf (stderr, "get_address_translation: virt(0x%08x) -> phys(0x%08x)\n",
				vaddr, ret);
#endif

	return ret;
}


/* Read memory emulation.
     addr = address to read.
     size = size of read in bytes.  */
static UINT32 readmemi_emu (i860s *cpustate, UINT32 addr, int size)
{
#ifdef TRACE_RDWR_MEM
	fprintf (stderr, "readmemi_emu: (ATE=%d) addr = 0x%08x, size = %d\n",
				GET_DIRBASE_ATE (), addr, size);
#endif

	/* If virtual mode, do translation.  */
	if (GET_DIRBASE_ATE ())
	{
		UINT32 phys = get_address_translation (cpustate, addr, 1 /* is_dataref */, 0 /* is_write */);
		if (cpustate->pending_trap && (GET_PSR_IAT () || GET_PSR_DAT ()))
		{
#ifdef TRACE_PAGE_FAULT
			fprintf (stderr, "0x%08x: ## Page fault (readmemi_emu).\n",
						cpustate->pc);
#endif
			cpustate->exiting_readmem = 1;
			return 0;
		}
		addr = phys;
	}

	/* First check for match to db register (before read).  */
	if (((addr & ~(size - 1)) == cpustate->cregs[CR_DB]) && GET_PSR_BR ())
	{
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		return 0;
	}

	/* Now do the actual read.  */
	if (size == 1)
	{
		UINT32 ret = cpustate->program->read_byte(addr);
		return ret & 0xff;
	}
	else if (size == 2)
	{
		UINT32 ret = cpustate->program->read_word(addr);
#ifdef HOST_MSB
		BYTE_REV16 (ret);
#endif
		return ret & 0xffff;
	}
	else if (size == 4)
	{
		UINT32 ret = cpustate->program->read_dword(addr);
#ifdef HOST_MSB
		BYTE_REV32 (ret);
#endif
		return ret;
	}
	else
		assert (0);

	return 0;
}


/* Write memory emulation.
     addr = address to write.
     size = size of write in bytes.
     data = data to write.  */
static void writememi_emu (i860s *cpustate, UINT32 addr, int size, UINT32 data)
{
#ifdef TRACE_RDWR_MEM
	fprintf (stderr, "writememi_emu: (ATE=%d) addr = 0x%08x, size = %d, data = 0x%08x\n",
				GET_DIRBASE_ATE (), addr, size, data);
#endif

	/* If virtual mode, do translation.  */
	if (GET_DIRBASE_ATE ())
	{
		UINT32 phys = get_address_translation (cpustate, addr, 1 /* is_dataref */, 1 /* is_write */);
		if (cpustate->pending_trap && (GET_PSR_IAT () || GET_PSR_DAT ()))
		{
#ifdef TRACE_PAGE_FAULT
			fprintf (stderr, "0x%08x: ## Page fault (writememi_emu).\n",
						cpustate->pc);
#endif
			cpustate->exiting_readmem = 2;
			return;
		}
		addr = phys;
	}

	/* First check for match to db register (before write).  */
	if (((addr & ~(size - 1)) == cpustate->cregs[CR_DB]) && GET_PSR_BW ())
	{
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		return;
	}

	/* Now do the actual write.  */
	if (size == 1)
		cpustate->program->write_byte(addr, data);
	else if (size == 2)
	{
#ifdef HOST_MSB
		BYTE_REV16 (data);
#endif
		cpustate->program->write_word(addr, data);
	}
	else if (size == 4)
	{
#ifdef HOST_MSB
		BYTE_REV32 (data);
#endif
		cpustate->program->write_dword(addr, data);
	}
	else
		assert (0);
}


/* Floating-point read mem routine.
     addr = address to read.
     size = size of read in bytes.
     dest = memory to put read data.  */
static void fp_readmem_emu (i860s *cpustate, UINT32 addr, int size, UINT8 *dest)
{
#ifdef TRACE_RDWR_MEM
	fprintf (stderr, "fp_readmem_emu: (ATE=%d) addr = 0x%08x, size = %d\n",
				GET_DIRBASE_ATE (), addr, size);
#endif

	assert (size == 4 || size == 8 || size == 16);

	/* If virtual mode, do translation.  */
	if (GET_DIRBASE_ATE ())
	{
		UINT32 phys = get_address_translation (cpustate, addr, 1 /* is_dataref */, 0 /* is_write */);
		if (cpustate->pending_trap && (GET_PSR_IAT () || GET_PSR_DAT ()))
		{
#ifdef TRACE_PAGE_FAULT
			fprintf (stderr, "0x%08x: ## Page fault (fp_readmem_emu).\n",
						cpustate->pc);
#endif
			cpustate->exiting_readmem = 3;
			return;
		}
		addr = phys;
	}

	/* First check for match to db register (before read).  */
	if (((addr & ~(size - 1)) == cpustate->cregs[CR_DB]) && GET_PSR_BR ())
	{
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		return;
	}

	if (size == 4)
	{
		dest[0] = cpustate->program->read_byte(addr+3);
		dest[1] = cpustate->program->read_byte(addr+2);
		dest[2] = cpustate->program->read_byte(addr+1);
		dest[3] = cpustate->program->read_byte(addr+0);
	}
	else if (size == 8)
	{
		dest[0] = cpustate->program->read_byte(addr+7);
		dest[1] = cpustate->program->read_byte(addr+6);
		dest[2] = cpustate->program->read_byte(addr+5);
		dest[3] = cpustate->program->read_byte(addr+4);
		dest[4] = cpustate->program->read_byte(addr+3);
		dest[5] = cpustate->program->read_byte(addr+2);
		dest[6] = cpustate->program->read_byte(addr+1);
		dest[7] = cpustate->program->read_byte(addr+0);
	}
	else if (size == 16)
	{
		int i;
		for (i = 0; i < 16; i++)
		{
			dest[i] = cpustate->program->read_byte(addr+15-i);
		}
	}
}


/* Floating-point write mem routine.
     addr = address to read.
     size = size of read in bytes.
     data = pointer to the data.
     wmask = bit mask of bytes to write (only for pst.d).  */
static void fp_writemem_emu (i860s *cpustate, UINT32 addr, int size, UINT8 *data, UINT32 wmask)
{
#ifdef TRACE_RDWR_MEM
	fprintf (stderr, "fp_writemem_emu: (ATE=%d) addr = 0x%08x, size = %d\n",
				GET_DIRBASE_ATE (), addr, size);
#endif

	assert (size == 4 || size == 8 || size == 16);

	/* If virtual mode, do translation.  */
	if (GET_DIRBASE_ATE ())
	{
		UINT32 phys = get_address_translation (cpustate, addr, 1 /* is_dataref */, 1 /* is_write */);
		if (cpustate->pending_trap && GET_PSR_DAT ())
		{
#ifdef TRACE_PAGE_FAULT
			fprintf (stderr, "0x%08x: ## Page fault (fp_writememi_emu).\n",
						cpustate->pc);
#endif
			cpustate->exiting_readmem = 4;
			return;
		}
		addr = phys;
	}

	/* First check for match to db register (before read).  */
	if (((addr & ~(size - 1)) == cpustate->cregs[CR_DB]) && GET_PSR_BW ())
	{
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		return;
	}

	if (size == 4)
	{
#if 1
		cpustate->program->write_byte(addr+3, data[0]);
		cpustate->program->write_byte(addr+2, data[1]);
		cpustate->program->write_byte(addr+1, data[2]);
		cpustate->program->write_byte(addr+0, data[3]);
#else
		UINT32 ddd = (data[3]) | (data[2] << 8) | (data[1] << 16) |(data[0] << 24);
		cpustate->program->write_dword(addr+0, ddd);
#endif
	}
	else if (size == 8)
	{
		/* Special: watch for wmask != 0xff, which means we're doing pst.d.  */
		if (wmask == 0xff)
		{
			cpustate->program->write_byte(addr+7, data[0]);
			cpustate->program->write_byte(addr+6, data[1]);
			cpustate->program->write_byte(addr+5, data[2]);
			cpustate->program->write_byte(addr+4, data[3]);
			cpustate->program->write_byte(addr+3, data[4]);
			cpustate->program->write_byte(addr+2, data[5]);
			cpustate->program->write_byte(addr+1, data[6]);
			cpustate->program->write_byte(addr+0, data[7]);
		}
		else
		{
			if (wmask & 0x80) cpustate->program->write_byte(addr+7, data[0]);
			if (wmask & 0x40) cpustate->program->write_byte(addr+6, data[1]);
			if (wmask & 0x20) cpustate->program->write_byte(addr+5, data[2]);
			if (wmask & 0x10) cpustate->program->write_byte(addr+4, data[3]);
			if (wmask & 0x08) cpustate->program->write_byte(addr+3, data[4]);
			if (wmask & 0x04) cpustate->program->write_byte(addr+2, data[5]);
			if (wmask & 0x02) cpustate->program->write_byte(addr+1, data[6]);
			if (wmask & 0x01) cpustate->program->write_byte(addr+0, data[7]);
		}
	}
	else if (size == 16)
	{
		int i;
		for (i = 0; i < 16; i++)
		{
			cpustate->program->write_byte(addr+15-i, data[i]);
		}
	}

}


#if 0
/* Do a pipeline dump.
    type: 0 (all), 1 (add), 2 (mul), 3 (load), 4 (graphics).  */
static void dump_pipe (i860s *cpustate, int type)
{
	int i = 0;

	fprintf (stderr, "pipeline state:\n");
	/* Dump the adder pipeline, if requested.  */
	if (type == 0 || type == 1)
	{
		fprintf (stderr, "  A: ");
		for (i = 0; i < 3; i++)
		{
			if (cpustate->A[i].stat.arp)
				fprintf (stderr, "[%dd] 0x%016llx ", i + 1,
							*(UINT64 *)(&cpustate->A[i].val.d));
			else
				fprintf (stderr, "[%ds] 0x%08x ", i + 1,
							*(UINT32 *)(&cpustate->A[i].val.s));
		}
		fprintf (stderr, "\n");
	}


	/* Dump the multiplier pipeline, if requested.  */
	if (type == 0 || type == 2)
	{
		fprintf (stderr, "  M: ");
		for (i = 0; i < 3; i++)
		{
			if (cpustate->M[i].stat.mrp)
				fprintf (stderr, "[%dd] 0x%016llx ", i + 1,
							*(UINT64 *)(&cpustate->M[i].val.d));
			else
				fprintf (stderr, "[%ds] 0x%08x ", i + 1,
							*(UINT32 *)(&cpustate->M[i].val.s));
		}
		fprintf (stderr, "\n");
	}

	/* Dump the load pipeline, if requested.  */
	if (type == 0 || type == 3)
	{
		fprintf (stderr, "  L: ");
		for (i = 0; i < 3; i++)
		{
			if (cpustate->L[i].stat.lrp)
				fprintf (stderr, "[%dd] 0x%016llx ", i + 1,
							*(UINT64 *)(&cpustate->L[i].val.d));
			else
				fprintf (stderr, "[%ds] 0x%08x ", i + 1,
							*(UINT32 *)(&cpustate->L[i].val.s));
		}
		fprintf (stderr, "\n");
	}

	/* Dump the graphics pipeline, if requested.  */
	if (type == 0 || type == 4)
	{
		fprintf (stderr, "  I: ");
		if (cpustate->G.stat.irp)
			fprintf (stderr, "[1d] 0x%016llx\n",
						*(UINT64 *)(&cpustate->G.val.d));
		else
			fprintf (stderr, "[1s] 0x%08x\n",
						*(UINT32 *)(&cpustate->G.val.s));
	}
}


/* Do a register/state dump.  */
static void dump_state (i860s *cpustate)
{
	int rn;

	/* GR's first, 4 per line.  */
	for (rn = 0; rn < 32; rn++)
	{
		if ((rn % 4) == 0)
			fprintf (stderr, "\n");
		fprintf (stderr, "%%r%-3d: 0x%08x  ", rn, get_iregval (rn));
	}
	fprintf (stderr, "\n");

	/* FR's (as 32-bits), 4 per line.  */
	for (rn = 0; rn < 32; rn++)
	{
		float ff = get_fregval_s (cpustate, rn);
		if ((rn % 4) == 0)
			fprintf (stderr, "\n");
		fprintf (stderr, "%%f%-3d: 0x%08x  ", rn, *(UINT32 *)&ff);
	}
	fprintf (stderr, "\n");

	fprintf (stderr, " psr: CC = %d, LCC = %d, SC = %d, IM = %d, U = %d\n",
				GET_PSR_CC (), GET_PSR_LCC (), GET_PSR_SC (), GET_PSR_IM (),
				GET_PSR_U ());
	fprintf (stderr, "      IT/FT/IAT/DAT/IN = %d/%d/%d/%d/%d\n",
				GET_PSR_IT (), GET_PSR_FT (), GET_PSR_IAT (),
				GET_PSR_DAT (), GET_PSR_IN ());
	fprintf (stderr, "epsr: INT = %d, OF = %d, BE = %d\n",
				GET_EPSR_INT (), GET_EPSR_OF (), GET_EPSR_BE ());
	fprintf (stderr, " fir: 0x%08x  dirbase: 0x%08x  fsr: 0x%08x\n",
				cpustate->cregs[CR_FIR], cpustate->cregs[CR_DIRBASE],
				cpustate->cregs[CR_FSR]);
	fprintf (stderr, "  pc: 0x%08x\n", cpustate->pc);
}
#endif

/* Sign extend N-bit number.  */
INLINE INT32 sign_ext (UINT32 x, int n)
{
	INT32 t;
	t = x >> (n - 1);
	t = ((-t) << n) | x;
	return t;
}


static void unrecog_opcode (UINT32 pc, UINT32 insn)
{
	fprintf (stderr, "0x%08x: 0x%08x   (unrecognized opcode)\n", pc, insn);
}


/* Execute "ld.c csrc2,idest" instruction.  */
static void insn_ld_ctrl (i860s *cpustate, UINT32 insn)
{
	UINT32 csrc2 = get_creg (insn);
	UINT32 idest = get_idest (insn);

#ifdef TRACE_UNDEFINED_I860
	if (csrc2 > 5)
	{
		/* Control register not between 0..5.  Undefined i860XR behavior.  */
		fprintf (stderr, "WARNING: insn_ld_from_ctrl (pc=0x%08x): bad creg in ld.c (ignored)\n", cpustate->pc);
		return;
	}
#endif

	/* If this is a load of the fir, then there are two cases:
	   1. First load of fir after a trap = usual value.
	   2. Not first load of fir after a trap = address of the ld.c insn.  */
	if (csrc2 == CR_FIR)
	{
		if (cpustate->fir_gets_trap_addr)
			set_iregval (idest, cpustate->cregs[csrc2]);
		else
		{
			cpustate->cregs[csrc2] = cpustate->pc;
			set_iregval (idest, cpustate->cregs[csrc2]);
		}
		cpustate->fir_gets_trap_addr = 0;
	}
	else
		set_iregval (idest, cpustate->cregs[csrc2]);
}


/* Execute "st.c isrc1,csrc2" instruction.  */
static void insn_st_ctrl (i860s *cpustate, UINT32 insn)
{
	UINT32 csrc2 = get_creg (insn);
	UINT32 isrc1 = get_isrc1 (insn);

#ifdef TRACE_UNDEFINED_I860
	if (csrc2 > 5)
	{
		/* Control register not between 0..5.  Undefined i860XR behavior.  */
		fprintf (stderr, "WARNING: insn_st_to_ctrl (pc=0x%08x): bad creg in st.c (ignored)\n", cpustate->pc);
		return;
	}
#endif

	/* Look for ITI bit turned on (but it never actually is written --
	   it always appears to be 0).  */
	if (csrc2 == CR_DIRBASE && (get_iregval (isrc1) & 0x20))
	{
		/* NOTE: The actual icache and TLB flush are unimplemented for
		   the MAME version.  */

		/* Make sure ITI isn't actually written.  */
		set_iregval (isrc1, (get_iregval (isrc1) & ~0x20));
	}

	if (csrc2 == CR_DIRBASE && (get_iregval (isrc1) & 1)
		&& GET_DIRBASE_ATE () == 0)
	{
		fprintf (stderr, "0x%08x: ** ATE going high!\n", cpustate->pc);
	}

	/* Update the register -- unless it is fir which cannot be updated.  */
	if (csrc2 == CR_EPSR)
	{
		UINT32 enew = 0, tmp = 0;
		/* Make sure unchangeable EPSR bits stay unchanged (DCS, stepping,
		   and type).  Also, some bits are only writeable in supervisor
		   mode.  */
		if (GET_PSR_U ())
		{
			enew = get_iregval (isrc1) & ~(0x003e1fff | 0x00c06000);
			tmp = cpustate->cregs[CR_EPSR] & (0x003e1fff | 0x00c06000);
		}
		else
		{
			enew = get_iregval (isrc1) & ~0x003e1fff;
			tmp = cpustate->cregs[CR_EPSR] & 0x003e1fff;
		}
		cpustate->cregs[CR_EPSR] = enew | tmp;
	}
	else if (csrc2 == CR_PSR)
	{
		/* Some PSR bits are only writeable in supervisor mode.  */
		if (GET_PSR_U ())
		{
			UINT32 enew = get_iregval (isrc1) & ~PSR_SUPERVISOR_ONLY_MASK;
			UINT32 tmp = cpustate->cregs[CR_PSR] & PSR_SUPERVISOR_ONLY_MASK;
			cpustate->cregs[CR_PSR] = enew | tmp;
		}
		else
			cpustate->cregs[CR_PSR] = get_iregval (isrc1);
	}
	else if (csrc2 == CR_FSR)
	{
		/* I believe that only 21..17, 8..5, and 3..0 should be updated.  */
		UINT32 enew = get_iregval (isrc1) & 0x003e01ef;
		UINT32 tmp = cpustate->cregs[CR_FSR] & ~0x003e01ef;
		cpustate->cregs[CR_FSR] = enew | tmp;
	}
	else if (csrc2 != CR_FIR)
		cpustate->cregs[csrc2] = get_iregval (isrc1);
}


/* Execute "ld.{s,b,l} isrc1(isrc2),idest" or
   "ld.{s,b,l} #const(isrc2),idest".  */
static void insn_ldx (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	INT32 immsrc1 = sign_ext (get_imm16 (insn), 16);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 eff = 0;
	/* Operand size, in bytes.  */
	int sizes[4] = { 1, 1, 2, 4};
	int size = 0;
	int form_disp_reg = 0;

	/* Bits 28 and 0 determine the operand size.  */
	size = sizes[((insn >> 27) & 2) | (insn & 1)];

	/* Bit 26 determines the addressing mode (reg+reg or disp+reg).  */
	form_disp_reg = (insn & 0x04000000);

	/* Get effective address depending on disp+reg or reg+reg form.  */
	if (form_disp_reg)
	{
		/* Chop off lower bits of displacement.  */
		immsrc1 &= ~(size - 1);
		eff = (UINT32)(immsrc1 + (INT32)(get_iregval (isrc2)));
	}
	else
		eff = get_iregval (isrc1) + get_iregval (isrc2);

#ifdef TRACE_UNALIGNED_MEM
	if (eff & (size - 1))
	{
		fprintf (stderr, "0x%08x: Unaligned access detected (0x%08x).\n",
					cpustate->pc, eff);
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		return;
	}
#endif

	/* The i860 sign-extends 8- or 16-bit integer loads.

	   Below, the readmemi_emu() needs to happen outside of the
	   set_iregval macro (otherwise the readmem won't occur if r0
	   is the target register).  */
	if (size < 4)
	{
		UINT32 readval = sign_ext (readmemi_emu (cpustate, eff, size), size * 8);
		/* Do not update register on page fault.  */
		if (cpustate->exiting_readmem)
		{
			return;
		}
		set_iregval (idest, readval);
	}
	else
	{
		UINT32 readval = readmemi_emu (cpustate, eff, size);
		/* Do not update register on page fault.  */
		if (cpustate->exiting_readmem)
		{
			return;
		}
		set_iregval (idest, readval);
	}
}


/* Execute "st.x isrc1ni,#const(isrc2)" instruction (there is no
   (reg + reg form).  Store uses the split immediate, not the normal
   16-bit immediate as in ld.x.  */
static void insn_stx (i860s *cpustate, UINT32 insn)
{
	INT32 immsrc = sign_ext ((((insn >> 5) & 0xf800) | (insn & 0x07ff)), 16);
	UINT32 isrc1 = get_isrc1 (insn);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 eff = 0;
	/* Operand size, in bytes.  */
	int sizes[4] = { 1, 1, 2, 4};
	int size = 0;

	/* Bits 28 and 0 determine the operand size.  */
	size = sizes[((insn >> 27) & 2) | (insn & 1)];

	/* FIXME: Do any necessary traps.  */

	/* Get effective address.  Chop off lower bits of displacement.  */
	immsrc &= ~(size - 1);
	eff = (UINT32)(immsrc + (INT32)get_iregval (isrc2));

	/* Write data (value of reg isrc1) to memory at eff.  */
	writememi_emu (cpustate, eff, size, get_iregval (isrc1));
	if (cpustate->exiting_readmem)
		return;
}


/* Execute "fst.y fdest,isrc1(isrc2)", "fst.y fdest,isrc1(isrc2)++",
           "fst.y fdest,#const(isrc2)" or "fst.y fdest,#const(isrc2)++"
   instruction.  */
static void insn_fsty (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	INT32 immsrc1 = sign_ext (get_imm16 (insn), 16);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	UINT32 eff = 0;
	/* Operand size, in bytes.  */
	int sizes[4] = { 8, 4, 16, 4};
	int size = 0;
	int form_disp_reg = 0;
	int auto_inc = (insn & 1);

	/* Bits 2 and 1 determine the operand size.  */
	size = sizes[((insn >> 1) & 3)];

	/* Bit 26 determines the addressing mode (reg+reg or disp+reg).  */
	form_disp_reg = (insn & 0x04000000);

	/* FIXME: Check for undefined behavior, non-even or non-quad
	   register operands for fst.d and fst.q respectively.  */

	/* Get effective address depending on disp+reg or reg+reg form.  */
	if (form_disp_reg)
	{
		/* Chop off lower bits of displacement.  */
		immsrc1 &= ~(size - 1);
		eff = (UINT32)(immsrc1 + (INT32)(get_iregval (isrc2)));
	}
	else
		eff = get_iregval (isrc1) + get_iregval (isrc2);

#ifdef TRACE_UNALIGNED_MEM
	if (eff & (size - 1))
	{
		fprintf (stderr, "0x%08x: Unaligned access detected (0x%08x).\n",
					cpustate->pc, eff);
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		return;
	}
#endif

	/* Do (post) auto-increment.  */
	if (auto_inc)
	{
		set_iregval (isrc2, eff);
#ifdef TRACE_UNDEFINED_I860
		/* When auto-inc, isrc1 and isrc2 regs can't be the same.  */
		if (isrc1 == isrc2)
		{
			/* Undefined i860XR behavior.  */
			fprintf (stderr, "WARNING: insn_fsty (pc=0x%08x): isrc1 = isrc2 in fst with auto-inc (ignored)\n", cpustate->pc);
			return;
		}
#endif
	}

	/* Write data (value of freg fdest) to memory at eff.  */
	if (size == 4)
		fp_writemem_emu (cpustate, eff, size, (UINT8 *)(&cpustate->frg[4 * (31 - fdest)]), 0xff);
	else if (size == 8)
		fp_writemem_emu (cpustate, eff, size, (UINT8 *)(&cpustate->frg[4 * (31 - (fdest + 1))]), 0xff);
	else
		fp_writemem_emu (cpustate, eff, size, (UINT8 *)(&cpustate->frg[4 * (31 - (fdest + 3))]), 0xff);

}


/* Execute "fld.y isrc1(isrc2),fdest", "fld.y isrc1(isrc2)++,idest",
           "fld.y #const(isrc2),fdest" or "fld.y #const(isrc2)++,idest".
   Where y = {l,d,q}.  Note, there is no pfld.q, though.  */
static void insn_fldy (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	INT32 immsrc1 = sign_ext (get_imm16 (insn), 16);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	UINT32 eff = 0;
	/* Operand size, in bytes.  */
	int sizes[4] = { 8, 4, 16, 4};
	int size = 0;
	int form_disp_reg = 0;
	int auto_inc = (insn & 1);
	int piped = (insn & 0x40000000);

	/* Bits 2 and 1 determine the operand size.  */
	size = sizes[((insn >> 1) & 3)];

	/* Bit 26 determines the addressing mode (reg+reg or disp+reg).  */
	form_disp_reg = (insn & 0x04000000);

	/* There is no pipelined load quad.  */
	if (piped && size == 16)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	/* FIXME: Check for undefined behavior, non-even or non-quad
	   register operands for fld.d and fld.q respectively.  */

	/* Get effective address depending on disp+reg or reg+reg form.  */
	if (form_disp_reg)
	{
		/* Chop off lower bits of displacement.  */
		immsrc1 &= ~(size - 1);
		eff = (UINT32)(immsrc1 + (INT32)(get_iregval (isrc2)));
	}
	else
		eff = get_iregval (isrc1) + get_iregval (isrc2);

	/* Do (post) auto-increment.  */
	if (auto_inc)
	{
		set_iregval (isrc2, eff);
#ifdef TRACE_UNDEFINED_I860
		/* When auto-inc, isrc1 and isrc2 regs can't be the same.  */
		if (isrc1 == isrc2)
		{
			/* Undefined i860XR behavior.  */
			fprintf (stderr, "WARNING: insn_fldy (pc=0x%08x): isrc1 = isrc2 in fst with auto-inc (ignored)\n", cpustate->pc);
			return;
		}
#endif
	}

#ifdef TRACE_UNALIGNED_MEM
	if (eff & (size - 1))
	{
		fprintf (stderr, "0x%08x: Unaligned access detected (0x%08x).\n",
					cpustate->pc, eff);
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		return;
	}
#endif

	/* Update the load pipe if necessary.  */
	/* FIXME: Copy result-status bits to fsr from last stage.  */
	if (!piped)
	{
		/* Scalar version writes the current result to fdest.  */
		/* Read data at 'eff' into freg 'fdest' (reads to f0 or f1 are
		   thrown away).  */
		if (fdest > 1)
		{
			if (size == 4)
				fp_readmem_emu (cpustate, eff, size, (UINT8 *)&(cpustate->frg[4 * (31 - fdest)]));
			else if (size == 8)
				fp_readmem_emu (cpustate, eff, size, (UINT8 *)&(cpustate->frg[4 * (31 - (fdest + 1))]));
			else if (size == 16)
				fp_readmem_emu (cpustate, eff, size, (UINT8 *)&(cpustate->frg[4 * (31 - (fdest + 3))]));
		}
	}
	else
	{
		/* Read the data into a temp space first.  This way we can test
		   for any traps before updating the pipeline.  The pipeline must
		   stay unaffected after a trap so that the instruction can be
		   properly restarted.  */
		UINT8 bebuf[8];
		fp_readmem_emu (cpustate, eff, size, bebuf);
		if (cpustate->pending_trap && cpustate->exiting_readmem)
			goto ab_op;

		/* Pipelined version writes fdest with the result from the last
		   stage of the pipeline, with precision specified by the LRP
		   bit of the stage's result-status bits.  */
#if 1 /* FIXME: WIP on FSR update.  This may not be correct.  */
		/* Copy 3rd stage LRP to FSR.  */
		if (cpustate->L[1 /* 2 */].stat.lrp)
			cpustate->cregs[CR_FSR] |= 0x04000000;
		else
			cpustate->cregs[CR_FSR] &= ~0x04000000;
#endif
		if (cpustate->L[2].stat.lrp)  /* 3rd (last) stage.  */
			set_fregval_d (cpustate, fdest, cpustate->L[2].val.d);
		else
			set_fregval_s (cpustate, fdest, cpustate->L[2].val.s);

		/* Now advance pipeline and write loaded data to first stage.  */
		cpustate->L[2] = cpustate->L[1];
		cpustate->L[1] = cpustate->L[0];
		if (size == 8)
		{
			UINT8 *t = (UINT8 *)&(cpustate->L[0].val.d);
#ifndef HOST_MSB
			t[7] = bebuf[0]; t[6] = bebuf[1]; t[5] = bebuf[2]; t[4] = bebuf[3];
			t[3] = bebuf[4]; t[2] = bebuf[5]; t[1] = bebuf[6]; t[0] = bebuf[7];
#else
			t[0] = bebuf[0]; t[1] = bebuf[1]; t[2] = bebuf[2]; t[3] = bebuf[3];
			t[4] = bebuf[4]; t[5] = bebuf[5]; t[6] = bebuf[6]; t[7] = bebuf[7];
#endif
			cpustate->L[0].stat.lrp = 1;
		}
		else
		{
			UINT8 *t = (UINT8 *)&(cpustate->L[0].val.s);
#ifndef HOST_MSB
			t[3] = bebuf[0]; t[2] = bebuf[1]; t[1] = bebuf[2]; t[0] = bebuf[3];
#else
			t[0] = bebuf[0]; t[1] = bebuf[1]; t[2] = bebuf[2]; t[3] = bebuf[3];
#endif
			cpustate->L[0].stat.lrp = 0;
		}
	}

	ab_op:;
}


/* Execute "pst.d fdest,#const(isrc2)" or "fst.d fdest,#const(isrc2)++"
   instruction.  */
static void insn_pstd (i860s *cpustate, UINT32 insn)
{
	INT32 immsrc1 = sign_ext (get_imm16 (insn), 16);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	UINT32 eff = 0;
	int auto_inc = (insn & 1);
	UINT8 *bebuf = 0;
	int pm = GET_PSR_PM ();
	int i;
	UINT32 wmask;
	int orig_pm = pm;

	/* Get the pixel size, where:
	   PS: 0 = 8 bits, 1 = 16 bits, 2 = 32-bits.  */
	int ps = GET_PSR_PS ();

#ifdef TRACE_UNDEFINED_I860
	if (!(ps == 0 || ps == 1 || ps == 2))
		fprintf (stderr, "insn_pstd: Undefined i860XR behavior, invalid value %d for pixel size.\n", ps);
#endif

#ifdef TRACE_UNDEFINED_I860
	/* Bits 2 and 1 determine the operand size, which must always be
	   zero (indicating a 64-bit operand).  */
	if (insn & 0x6)
	{
		/* Undefined i860XR behavior.  */
		fprintf (stderr, "WARNING: insn_pstd (pc=0x%08x): bad operand size specifier\n", cpustate->pc);
	}
#endif

	/* FIXME: Check for undefined behavior, non-even register operands.  */

	/* Get effective address.  Chop off lower bits of displacement.  */
	immsrc1 &= ~(8 - 1);
	eff = (UINT32)(immsrc1 + (INT32)(get_iregval (isrc2)));

#ifdef TRACE_UNALIGNED_MEM
	if (eff & (8 - 1))
	{
		fprintf (stderr, "0x%08x: Unaligned access detected (0x%08x).\n",
					cpustate->pc, eff);
		SET_PSR_DAT (1);
		cpustate->pending_trap = 1;
		return;
	}
#endif

	/* Do (post) auto-increment.  */
	if (auto_inc)
		set_iregval (isrc2, eff);

	/* Update the the pixel mask depending on the pixel size.  Shift PM
	   right by 8/2^ps bits.  */
	if (ps == 0)
		pm = (pm >> 8) & 0x00;
	else if (ps == 1)
		pm = (pm >> 4) & 0x0f;
	else if (ps == 2)
		pm = (pm >> 2) & 0x3f;
	SET_PSR_PM (pm);

	/* Write data (value of freg fdest) to memory at eff-- but only those
	   bytes that are enabled by the bits in PSR.PM.  Bit 0 of PM selects
	   the pixel at the lowest address.  */
	wmask = 0;
	for (i = 0; i < 8; )
	{
		if (ps == 0)
		{
			if (orig_pm & 0x80)
				wmask |= 1 << (7-i);
			i += 1;
		}
		else if (ps == 1)
		{
			if (orig_pm & 0x08)
				wmask |= 0x3 << (6-i);
			i += 2;
		}
		else if (ps == 2)
		{
			if (orig_pm & 0x02)
				wmask |= 0xf << (4-i);
			i += 4;
		}
		else
		{
			wmask = 0xff;
			break;
		}
		orig_pm <<= 1;
	}
	bebuf = (UINT8 *)(&cpustate->frg[4 * (31 - (fdest + 1))]);
	fp_writemem_emu (cpustate, eff, 8, bebuf, wmask);
}


/* Execute "ixfr isrc1ni,fdest" instruction.  */
static void insn_ixfr (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	UINT32 fdest = get_fdest (insn);
	UINT32 iv = 0;

	/* This is a bit-pattern transfer, not a conversion.  */
	iv = get_iregval (isrc1);
	set_fregval_s (cpustate, fdest, *(float *)&iv);
}


/* Execute "addu isrc1,isrc2,idest".  */
static void insn_addu (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 tmp_dest_val = 0;
	UINT64 tmp = 0;

	src1val = get_iregval (get_isrc1 (insn));

	/* We don't update the actual idest register now because below we
	   need to test the original src1 and src2 if either happens to
	   be the destination register.  */
	tmp_dest_val = src1val + get_iregval (isrc2);

	/* Set OF and CC flags.
	   For unsigned:
	     OF = bit 31 carry
	     CC = bit 31 carry.
	 */
	tmp = (UINT64)src1val + (UINT64)(get_iregval (isrc2));
	if ((tmp >> 32) & 1)
	{
		SET_PSR_CC (1);
		SET_EPSR_OF (1);
	}
	else
	{
		SET_PSR_CC (0);
		SET_EPSR_OF (0);
	}

	/* Now update the destination register.  */
	set_iregval (idest, tmp_dest_val);
}


/* Execute "addu #const,isrc2,idest".  */
static void insn_addu_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 tmp_dest_val = 0;
	UINT64 tmp = 0;

	src1val = sign_ext (get_imm16 (insn), 16);

	/* We don't update the actual idest register now because below we
	   need to test the original src1 and src2 if either happens to
	   be the destination register.  */
	tmp_dest_val = src1val + get_iregval (isrc2);

	/* Set OF and CC flags.
	   For unsigned:
	     OF = bit 31 carry
	     CC = bit 31 carry.
	 */
	tmp = (UINT64)src1val + (UINT64)(get_iregval (isrc2));
	if ((tmp >> 32) & 1)
	{
		SET_PSR_CC (1);
		SET_EPSR_OF (1);
	}
	else
	{
		SET_PSR_CC (0);
		SET_EPSR_OF (0);
	}

	/* Now update the destination register.  */
	set_iregval (idest, tmp_dest_val);
}


/* Execute "adds isrc1,isrc2,idest".  */
static void insn_adds (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 tmp_dest_val = 0;
	int sa, sb, sres;

	src1val = get_iregval (get_isrc1 (insn));

	/* We don't update the actual idest register now because below we
	   need to test the original src1 and src2 if either happens to
	   be the destination register.  */
	tmp_dest_val = src1val + get_iregval (isrc2);

	/* Set OF and CC flags.
	   For signed:
	     OF = standard signed overflow.
	     CC set   if isrc2 < -isrc1
	     CC clear if isrc2 >= -isrc1
	 */
	sa = src1val & 0x80000000;
	sb = get_iregval (isrc2) & 0x80000000;
	sres = tmp_dest_val & 0x80000000;
	if (sa != sb && sa != sres)
		SET_EPSR_OF (1);
	else
		SET_EPSR_OF (0);

	if ((INT32)get_iregval (isrc2) < -(INT32)(src1val))
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	/* Now update the destination register.  */
	set_iregval (idest, tmp_dest_val);
}


/* Execute "adds #const,isrc2,idest".  */
static void insn_adds_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 tmp_dest_val = 0;
	int sa, sb, sres;

	src1val = sign_ext (get_imm16 (insn), 16);

	/* We don't update the actual idest register now because below we
	   need to test the original src1 and src2 if either happens to
	   be the destination register.  */
	tmp_dest_val = src1val + get_iregval (isrc2);

	/* Set OF and CC flags.
	   For signed:
	     OF = standard signed overflow.
	     CC set   if isrc2 < -isrc1
	     CC clear if isrc2 >= -isrc1
	 */
	sa = src1val & 0x80000000;
	sb = get_iregval (isrc2) & 0x80000000;
	sres = tmp_dest_val & 0x80000000;
	if (sa != sb && sa != sres)
		SET_EPSR_OF (1);
	else
		SET_EPSR_OF (0);

	if ((INT32)get_iregval (isrc2) < -(INT32)(src1val))
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	/* Now update the destination register.  */
	set_iregval (idest, tmp_dest_val);
}


/* Execute "subu isrc1,isrc2,idest".  */
static void insn_subu (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 tmp_dest_val = 0;

	src1val = get_iregval (get_isrc1 (insn));

	/* We don't update the actual idest register now because below we
	   need to test the original src1 and src2 if either happens to
	   be the destination register.  */
	tmp_dest_val = src1val - get_iregval (isrc2);

	/* Set OF and CC flags.
	   For unsigned:
	     OF = NOT(bit 31 carry)
	     CC = bit 31 carry.
	     (i.e. CC set   if isrc2 <= isrc1
	           CC clear if isrc2 > isrc1
	 */
	if ((UINT32)get_iregval (isrc2) <= (UINT32)src1val)
	{
		SET_PSR_CC (1);
		SET_EPSR_OF (0);
	}
	else
	{
		SET_PSR_CC (0);
		SET_EPSR_OF (1);
	}

	/* Now update the destination register.  */
	set_iregval (idest, tmp_dest_val);
}


/* Execute "subu #const,isrc2,idest".  */
static void insn_subu_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 tmp_dest_val = 0;

	src1val = sign_ext (get_imm16 (insn), 16);

	/* We don't update the actual idest register now because below we
	   need to test the original src1 and src2 if either happens to
	   be the destination register.  */
	tmp_dest_val = src1val - get_iregval (isrc2);

	/* Set OF and CC flags.
	   For unsigned:
	     OF = NOT(bit 31 carry)
	     CC = bit 31 carry.
	     (i.e. CC set   if isrc2 <= isrc1
	           CC clear if isrc2 > isrc1
	 */
	if ((UINT32)get_iregval (isrc2) <= (UINT32)src1val)
	{
		SET_PSR_CC (1);
		SET_EPSR_OF (0);
	}
	else
	{
		SET_PSR_CC (0);
		SET_EPSR_OF (1);
	}

	/* Now update the destination register.  */
	set_iregval (idest, tmp_dest_val);
}


/* Execute "subs isrc1,isrc2,idest".  */
static void insn_subs (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 tmp_dest_val = 0;
	int sa, sb, sres;

	src1val = get_iregval (get_isrc1 (insn));

	/* We don't update the actual idest register now because below we
	   need to test the original src1 and src2 if either happens to
	   be the destination register.  */
	tmp_dest_val = src1val - get_iregval (isrc2);

	/* Set OF and CC flags.
	   For signed:
	     OF = standard signed overflow.
	     CC set   if isrc2 > isrc1
	     CC clear if isrc2 <= isrc1
	 */
	sa = src1val & 0x80000000;
	sb = get_iregval (isrc2) & 0x80000000;
	sres = tmp_dest_val & 0x80000000;
	if (sa != sb && sa != sres)
		SET_EPSR_OF (1);
	else
		SET_EPSR_OF (0);

	if ((INT32)get_iregval (isrc2) > (INT32)(src1val))
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	/* Now update the destination register.  */
	set_iregval (idest, tmp_dest_val);
}


/* Execute "subs #const,isrc2,idest".  */
static void insn_subs_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 tmp_dest_val = 0;
	int sa, sb, sres;

	src1val = sign_ext (get_imm16 (insn), 16);

	/* We don't update the actual idest register now because below we
	   need to test the original src1 and src2 if either happens to
	   be the destination register.  */
	tmp_dest_val = src1val - get_iregval (isrc2);

	/* Set OF and CC flags.
	   For signed:
	     OF = standard signed overflow.
	     CC set   if isrc2 > isrc1
	     CC clear if isrc2 <= isrc1
	 */
	sa = src1val & 0x80000000;
	sb = get_iregval (isrc2) & 0x80000000;
	sres = tmp_dest_val & 0x80000000;
	if (sa != sb && sa != sres)
		SET_EPSR_OF (1);
	else
		SET_EPSR_OF (0);

	if ((INT32)get_iregval (isrc2) > (INT32)(src1val))
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	/* Now update the destination register.  */
	set_iregval (idest, tmp_dest_val);
}


/* Execute "shl isrc1,isrc2,idest".  */
static void insn_shl (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);

	src1val = get_iregval (get_isrc1 (insn));
	set_iregval (idest, get_iregval (isrc2) << src1val);
}


/* Execute "shl #const,isrc2,idest".  */
static void insn_shl_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);

	src1val = sign_ext (get_imm16 (insn), 16);
	set_iregval (idest, get_iregval (isrc2) << src1val);
}


/* Execute "shr isrc1,isrc2,idest".  */
static void insn_shr (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);

	src1val = get_iregval (get_isrc1 (insn));

	/* The iregs array is UINT32, so this is a logical shift.  */
	set_iregval (idest, get_iregval (isrc2) >> src1val);

	/* shr also sets the SC in psr (shift count).  */
	SET_PSR_SC (src1val);
}


/* Execute "shr #const,isrc2,idest".  */
static void insn_shr_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);

	src1val = sign_ext (get_imm16 (insn), 16);

	/* The iregs array is UINT32, so this is a logical shift.  */
	set_iregval (idest, get_iregval (isrc2) >> src1val);

	/* shr also sets the SC in psr (shift count).  */
	SET_PSR_SC (src1val);
}


/* Execute "shra isrc1,isrc2,idest".  */
static void insn_shra (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);

	src1val = get_iregval (get_isrc1 (insn));

	/* The iregs array is UINT32, so cast isrc2 to get arithmetic shift.  */
	set_iregval (idest, (INT32)get_iregval (isrc2) >> src1val);
}


/* Execute "shra #const,isrc2,idest".  */
static void insn_shra_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);

	src1val = sign_ext (get_imm16 (insn), 16);

	/* The iregs array is UINT32, so cast isrc2 to get arithmetic shift.  */
	set_iregval (idest, (INT32)get_iregval (isrc2) >> src1val);
}


/* Execute "shrd isrc1ni,isrc2,idest" instruction.  */
static void insn_shrd (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 sc = GET_PSR_SC ();
	UINT32 tmp;

	/* Do the operation:
	   idest = low_32(isrc1ni:isrc2 >> sc).  */
	if (sc == 0)
		tmp = get_iregval (isrc2);
	else
	{
		tmp = get_iregval (isrc1) << (32 - sc);
		tmp |= (get_iregval (isrc2) >> sc);
	}
	set_iregval (idest, tmp);
}


/* Execute "and isrc1,isrc2,idest".  */
static void insn_and (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	res = get_iregval (isrc1) & get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "and #const,isrc2,idest".  */
static void insn_and_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	src1val = get_imm16 (insn);
	res = src1val & get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "andh #const,isrc2,idest".  */
static void insn_andh_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	src1val = get_imm16 (insn);
	res = (src1val << 16) & get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "andnot isrc1,isrc2,idest".  */
static void insn_andnot (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	res = (~get_iregval (isrc1)) & get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "andnot #const,isrc2,idest".  */
static void insn_andnot_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	src1val = get_imm16 (insn);
	res = (~src1val) & get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "andnoth #const,isrc2,idest".  */
static void insn_andnoth_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	src1val = get_imm16 (insn);
	res = (~(src1val << 16)) & get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "or isrc1,isrc2,idest".  */
static void insn_or (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	res = get_iregval (isrc1) | get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "or #const,isrc2,idest".  */
static void insn_or_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	src1val = get_imm16 (insn);
	res = src1val | get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "orh #const,isrc2,idest".  */
static void insn_orh_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	src1val = get_imm16 (insn);
	res = (src1val << 16) | get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "xor isrc1,isrc2,idest".  */
static void insn_xor (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	res = get_iregval (isrc1) ^ get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "xor #const,isrc2,idest".  */
static void insn_xor_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	src1val = get_imm16 (insn);
	res = src1val ^ get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "xorh #const,isrc2,idest".  */
static void insn_xorh_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 idest = get_idest (insn);
	UINT32 res = 0;

	/* Do the operation.  */
	src1val = get_imm16 (insn);
	res = (src1val << 16) ^ get_iregval (isrc2);

	/* Set flags.  */
	if (res == 0)
		SET_PSR_CC (1);
	else
		SET_PSR_CC (0);

	set_iregval (idest, res);
}


/* Execute "trap isrc1ni,isrc2,idest" instruction.  */
static void insn_trap (i860s *cpustate, UINT32 insn)
{
	SET_PSR_IT (1);
	cpustate->pending_trap = 1;
}


/* Execute "intovr" instruction.  */
static void insn_intovr (i860s *cpustate, UINT32 insn)
{
	if (GET_EPSR_OF ())
	{
		SET_PSR_IT (1);
		cpustate->pending_trap = 1;
	}
}


/* Execute "bte isrc1,isrc2,sbroff".  */
static void insn_bte (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 target_addr = 0;
	INT32 sbroff = 0;
	int res = 0;

	src1val = get_iregval (get_isrc1 (insn));

	/* Compute the target address from the sbroff field.  */
	sbroff = sign_ext ((((insn >> 5) & 0xf800) | (insn & 0x07ff)), 16);
	target_addr = (INT32)cpustate->pc + 4 + (sbroff << 2);

	/* Determine comparison result.  */
	res = (src1val == get_iregval (isrc2));

	/* Branch routines always update the PC.  */
	if (res)
		cpustate->pc = target_addr;
	else
		cpustate->pc += 4;

	cpustate->pc_updated = 1;
}


/* Execute "bte #const5,isrc2,sbroff".  */
static void insn_bte_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 target_addr = 0;
	INT32 sbroff = 0;
	int res = 0;

	src1val = (insn >> 11) & 0x1f;  /* 5-bit field, zero-extended.  */

	/* Compute the target address from the sbroff field.  */
	sbroff = sign_ext ((((insn >> 5) & 0xf800) | (insn & 0x07ff)), 16);
	target_addr = (INT32)cpustate->pc + 4 + (sbroff << 2);

	/* Determine comparison result.  */
	res = (src1val == get_iregval (isrc2));

	/* Branch routines always update the PC.  */
	if (res)
		cpustate->pc = target_addr;
	else
		cpustate->pc += 4;

	cpustate->pc_updated = 1;
}


/* Execute "btne isrc1,isrc2,sbroff".  */
static void insn_btne (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 target_addr = 0;
	INT32 sbroff = 0;
	int res = 0;

	src1val = get_iregval (get_isrc1 (insn));

	/* Compute the target address from the sbroff field.  */
	sbroff = sign_ext ((((insn >> 5) & 0xf800) | (insn & 0x07ff)), 16);
	target_addr = (INT32)cpustate->pc + 4 + (sbroff << 2);

	/* Determine comparison result.  */
	res = (src1val != get_iregval (isrc2));

	/* Branch routines always update the PC.  */
	if (res)
		cpustate->pc = target_addr;
	else
		cpustate->pc += 4;

	cpustate->pc_updated = 1;
}


/* Execute "btne #const5,isrc2,sbroff".  */
static void insn_btne_imm (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = 0;
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 target_addr = 0;
	INT32 sbroff = 0;
	int res = 0;

	src1val = (insn >> 11) & 0x1f;  /* 5-bit field, zero-extended.  */

	/* Compute the target address from the sbroff field.  */
	sbroff = sign_ext ((((insn >> 5) & 0xf800) | (insn & 0x07ff)), 16);
	target_addr = (INT32)cpustate->pc + 4 + (sbroff << 2);

	/* Determine comparison result.  */
	res = (src1val != get_iregval (isrc2));

	/* Branch routines always update the PC.  */
	if (res)
		cpustate->pc = target_addr;
	else
		cpustate->pc += 4;

	cpustate->pc_updated = 1;
}


/* Execute "bc lbroff" instruction.  */
static void insn_bc (i860s *cpustate, UINT32 insn)
{
	UINT32 target_addr = 0;
	INT32 lbroff = 0;
	int res = 0;

	/* Compute the target address from the lbroff field.  */
	lbroff = sign_ext ((insn & 0x03ffffff), 26);
	target_addr = (INT32)cpustate->pc + 4 + (lbroff << 2);

	/* Determine comparison result.  */
	res = (GET_PSR_CC () == 1);

	/* Branch routines always update the PC.  */
	if (res)
		cpustate->pc = target_addr;
	else
		cpustate->pc += 4;

	cpustate->pc_updated = 1;
}


/* Execute "bnc lbroff" instruction.  */
static void insn_bnc (i860s *cpustate, UINT32 insn)
{
	UINT32 target_addr = 0;
	INT32 lbroff = 0;
	int res = 0;

	/* Compute the target address from the lbroff field.  */
	lbroff = sign_ext ((insn & 0x03ffffff), 26);
	target_addr = (INT32)cpustate->pc + 4 + (lbroff << 2);

	/* Determine comparison result.  */
	res = (GET_PSR_CC () == 0);

	/* Branch routines always update the PC, since pc_updated is set
	   in the decode routine.  */
	if (res)
		cpustate->pc = target_addr;
	else
		cpustate->pc += 4;

	cpustate->pc_updated = 1;
}


/* Execute "bc.t lbroff" instruction.  */
static void insn_bct (i860s *cpustate, UINT32 insn)
{
	UINT32 target_addr = 0;
	INT32 lbroff = 0;
	int res = 0;
	UINT32 orig_pc = cpustate->pc;

	/* Compute the target address from the lbroff field.  */
	lbroff = sign_ext ((insn & 0x03ffffff), 26);
	target_addr = (INT32)cpustate->pc + 4 + (lbroff << 2);

	/* Determine comparison result.  */
	res = (GET_PSR_CC () == 1);

	/* Careful. Unlike bla, the delay slot instruction is only executed
	   if the branch is taken.  */
	if (res)
	{
		/* Execute delay slot instruction.  */
		cpustate->pc += 4;
		decode_exec (cpustate, ifetch (cpustate, orig_pc + 4), 0);
		cpustate->pc = orig_pc;
		if (cpustate->pending_trap)
		{
			cpustate->pending_trap |= TRAP_IN_DELAY_SLOT;
			goto ab_op;
		}
	}

	/* Since this branch is delayed, we must jump 2 instructions if
	   if isn't taken.  */
	if (res)
		cpustate->pc = target_addr;
	else
		cpustate->pc += 8;

	cpustate->pc_updated = 1;

	ab_op:
	;
}


/* Execute "bnc.t lbroff" instruction.  */
static void insn_bnct (i860s *cpustate, UINT32 insn)
{
	UINT32 target_addr = 0;
	INT32 lbroff = 0;
	int res = 0;
	UINT32 orig_pc = cpustate->pc;

	/* Compute the target address from the lbroff field.  */
	lbroff = sign_ext ((insn & 0x03ffffff), 26);
	target_addr = (INT32)cpustate->pc + 4 + (lbroff << 2);

	/* Determine comparison result.  */
	res = (GET_PSR_CC () == 0);

	/* Careful. Unlike bla, the delay slot instruction is only executed
	   if the branch is taken.  */
	if (res)
	{
		/* Execute delay slot instruction.  */
		cpustate->pc += 4;
		decode_exec (cpustate, ifetch (cpustate, orig_pc + 4), 0);
		cpustate->pc = orig_pc;
		if (cpustate->pending_trap)
		{
			cpustate->pending_trap |= TRAP_IN_DELAY_SLOT;
			goto ab_op;
		}
	}

	/* Since this branch is delayed, we must jump 2 instructions if
	   if isn't taken.  */
	if (res)
		cpustate->pc = target_addr;
	else
		cpustate->pc += 8;

	cpustate->pc_updated = 1;

	ab_op:
	;
}


/* Execute "call lbroff" instruction.  */
static void insn_call (i860s *cpustate, UINT32 insn)
{
	UINT32 target_addr = 0;
	INT32 lbroff = 0;
	UINT32 orig_pc = cpustate->pc;

	/* Compute the target address from the lbroff field.  */
	lbroff = sign_ext ((insn & 0x03ffffff), 26);
	target_addr = (INT32)cpustate->pc + 4 + (lbroff << 2);

	/* Execute the delay slot instruction.  */
	cpustate->pc += 4;
	decode_exec (cpustate, ifetch (cpustate, orig_pc + 4), 0);
	cpustate->pc = orig_pc;
	if (cpustate->pending_trap)
	{
		cpustate->pending_trap |= TRAP_IN_DELAY_SLOT;
		goto ab_op;
	}

	/* Sets the return pointer (r1).  */
	set_iregval (1, orig_pc + 8);

	/* New target.  */
	cpustate->pc = target_addr;
	cpustate->pc_updated = 1;

	ab_op:;
}


/* Execute "br lbroff".  */
static void insn_br (i860s *cpustate, UINT32 insn)
{
	UINT32 target_addr = 0;
	INT32 lbroff = 0;
	UINT32 orig_pc = cpustate->pc;

	/* Compute the target address from the lbroff field.  */
	lbroff = sign_ext ((insn & 0x03ffffff), 26);
	target_addr = (INT32)cpustate->pc + 4 + (lbroff << 2);

	/* Execute the delay slot instruction.  */
	cpustate->pc += 4;
	decode_exec (cpustate, ifetch (cpustate, orig_pc + 4), 0);
	cpustate->pc = orig_pc;
	if (cpustate->pending_trap)
	{
		cpustate->pending_trap |= TRAP_IN_DELAY_SLOT;
		goto ab_op;
	}

	/* New target.  */
	cpustate->pc = target_addr;
	cpustate->pc_updated = 1;

	ab_op:;
}


/* Execute "bri isrc1ni" instruction.
   Note: I didn't merge this code with calli because bri must do
   a lot of flag manipulation if any trap bits are set.  */
static void insn_bri (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	UINT32 orig_pc = cpustate->pc;
	UINT32 orig_psr = cpustate->cregs[CR_PSR];
	UINT32 orig_src1_val = get_iregval (isrc1);

#if 1 /* TURBO.  */
	cpustate->cregs[CR_PSR] &= ~PSR_ALL_TRAP_BITS_MASK;
#endif

	/* Execute the delay slot instruction.  */
	cpustate->pc += 4;
	decode_exec (cpustate, ifetch (cpustate, orig_pc + 4), 0);
	cpustate->pc = orig_pc;

	/* Delay slot insn caused a trap, abort operation.  */
	if (cpustate->pending_trap)
	{
		cpustate->pending_trap |= TRAP_IN_DELAY_SLOT;
		goto ab_op;
	}

	/* If any trap bits are set, we need to do the return from
	   trap work.  Note, we must use the PSR value that existed
	   before the delay slot instruction was executed since the
	   delay slot instruction might itself cause a trap bit to
	   be set.  */
	if (orig_psr & PSR_ALL_TRAP_BITS_MASK)
	{
		/* Restore U and IM from their previous copies.  */
		SET_PSR_U (GET_PSR_PU ());
		SET_PSR_IM (GET_PSR_PIM ());

		cpustate->fir_gets_trap_addr = 0;
	}

	/* Update PC.  */
	cpustate->pc = orig_src1_val;

	cpustate->pc_updated = 1;
	ab_op:;
}

/* Execute "calli isrc1ni" instruction.  */
static void insn_calli (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	UINT32 orig_pc = cpustate->pc;
	UINT32 orig_src1_val = get_iregval (isrc1);

#ifdef TRACE_UNDEFINED_I860
	/* Check for undefined behavior.  */
	if (isrc1 == 1)
	{
		/* Src1 must not be r1.  */
		fprintf (stderr, "WARNING: insn_calli (pc=0x%08x): isrc1 = r1 on a calli\n", cpustate->pc);
	}
#endif

	/* Set return pointer before executing delay slot instruction.  */
	set_iregval (1, cpustate->pc + 8);

	/* Execute the delay slot instruction.  */
	cpustate->pc += 4;
	decode_exec (cpustate, ifetch (cpustate, orig_pc + 4), 0);
	cpustate->pc = orig_pc;
	if (cpustate->pending_trap)
	{
		set_iregval (1, orig_src1_val);
		cpustate->pending_trap |= TRAP_IN_DELAY_SLOT;
		goto ab_op;
	}

	/* Set new PC.  */
	cpustate->pc = orig_src1_val;
	cpustate->pc_updated = 1;

	ab_op:;
}


/* Execute "bla isrc1ni,isrc2,sbroff" instruction.  */
static void insn_bla (i860s *cpustate, UINT32 insn)
{
	UINT32 isrc1 = get_isrc1 (insn);
	UINT32 isrc2 = get_isrc2 (insn);
	UINT32 target_addr = 0;
	INT32 sbroff = 0;
	int lcc_tmp = 0;
	UINT32 orig_pc = cpustate->pc;
	UINT32 orig_isrc2val = get_iregval (isrc2);

#ifdef TRACE_UNDEFINED_I860
	/* Check for undefined behavior.  */
	if (isrc1 == isrc2)
	{
		/* Src1 and src2 the same is undefined i860XR behavior.  */
		fprintf (stderr, "WARNING: insn_bla (pc=0x%08x): isrc1 and isrc2 are the same (ignored)\n", cpustate->pc);
		return;
	}
#endif

	/* Compute the target address from the sbroff field.  */
	sbroff = sign_ext ((((insn >> 5) & 0xf800) | (insn & 0x07ff)), 16);
	target_addr = (INT32)cpustate->pc + 4 + (sbroff << 2);

	/* Determine comparison result based on opcode.  */
	lcc_tmp = ((INT32)get_iregval (isrc2) >= -(INT32)get_iregval (isrc1));

	set_iregval (isrc2, get_iregval (isrc1) + orig_isrc2val);

	/* Execute the delay slot instruction.  */
	cpustate->pc += 4;
	decode_exec (cpustate, ifetch (cpustate, orig_pc + 4), 0);
	cpustate->pc = orig_pc;
	if (cpustate->pending_trap)
	{
		cpustate->pending_trap |= TRAP_IN_DELAY_SLOT;
		goto ab_op;
	}

	if (GET_PSR_LCC ())
		cpustate->pc = target_addr;
	else
	{
		/* Since this branch is delayed, we must jump 2 instructions if
		   if isn't taken.  */
		cpustate->pc += 8;
	}
	SET_PSR_LCC (lcc_tmp);

	cpustate->pc_updated = 1;
	ab_op:;
}


/* Execute "flush #const(isrc2)" or "flush #const(isrc2)++" instruction.  */
static void insn_flush (i860s *cpustate, UINT32 insn)
{
	UINT32 src1val = sign_ext (get_imm16 (insn), 16);
	UINT32 isrc2 = get_isrc2 (insn);
	int auto_inc = (insn & 1);
	UINT32 eff = 0;

	/* Technically, idest should be encoded as r0 because idest
	   is undefined after the instruction.  We don't currently
	   check for this.

	   Flush D$ block at address #const+isrc2.  Block is undefined
	   after.  The effective address must be 16-byte aligned.

	   FIXME: Need to examine RB and RC and do this right.
	  */

	/* Chop off lower bits of displacement to 16-byte alignment.  */
	src1val &= ~(16-1);
	eff = src1val + get_iregval (isrc2);
	if (auto_inc)
		set_iregval (isrc2, eff);

	/* In user mode, the flush is ignored.  */
	if (GET_PSR_U () == 0)
	{
		/* If line is dirty, write it to memory and invalidate.
		   NOTE: The actual dirty write is unimplemented in the MAME version
		   as we don't emulate the dcache.  */
	}
}


/* Execute "[p]fmul.{ss,sd,dd} fsrc1,fsrc2,fdest" instruction or
   pfmul3.dd fsrc1,fsrc2,fdest.

   The pfmul3.dd differs from pfmul.dd in that it treats the pipeline
   as 3 stages, even though it is a double precision multiply.  */
static void insn_fmul (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	int src_prec = insn & 0x100;     /* 1 = double, 0 = single.  */
	int res_prec = insn & 0x080;     /* 1 = double, 0 = single.  */
	int piped = insn & 0x400;        /* 1 = pipelined, 0 = scalar.  */
	double dbl_tmp_dest = 0.0;
	float sgl_tmp_dest = 0.0;
	double dbl_last_stage_contents = 0.0;
	float sgl_last_stage_contents = 0.0;
	int is_pfmul3 = insn & 0x4;
	int num_stages = (src_prec && !is_pfmul3) ? 2 : 3;

	/* Only .dd is valid for pfmul.  */
	if (is_pfmul3 && (insn & 0x180) != 0x180)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	/* Check for invalid .ds combination.  */
	if ((insn & 0x180) == 0x100)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	/* For pipelined version, retrieve the contents of the last stage
	   of the pipeline, whose precision is specified by the MRP bit
	   of the stage's result-status bits.  Note for pfmul, the number
	   of stages is determined by the source precision of the current
	   operation.  */
	if (piped)
	{
		if (cpustate->M[num_stages - 1].stat.mrp)
			dbl_last_stage_contents = cpustate->M[num_stages - 1].val.d;
		else
			sgl_last_stage_contents = cpustate->M[num_stages - 1].val.s;
	}

	/* Do the operation, being careful about source and result
	   precision.  */
	if (src_prec)
	{
		double v1 = get_fregval_d (cpustate, fsrc1);
		double v2 = get_fregval_d (cpustate, fsrc2);

		/* For pipelined mul, if fsrc2 is the same as fdest, then the last
		   stage is bypassed to fsrc2 (rather than using the value in fsrc2).
		   This bypass is not available for fsrc1, and is undefined behavior.  */
		if (0 && piped && fdest != 0 && fsrc1 == fdest)
			v1 = dbl_last_stage_contents;
		if (piped && fdest != 0 && fsrc2 == fdest)
			v2 = dbl_last_stage_contents;

		if (res_prec)
			dbl_tmp_dest = v1 * v2;
		else
			sgl_tmp_dest = (float)(v1 * v2);
	}
	else
	{
		float v1 = get_fregval_s (cpustate, fsrc1);
		float v2 = get_fregval_s (cpustate, fsrc2);

		/* For pipelined mul, if fsrc2 is the same as fdest, then the last
		   stage is bypassed to fsrc2 (rather than using the value in fsrc2).
		   This bypass is not available for fsrc1, and is undefined behavior.  */
		if (0 && piped && fdest != 0 && fsrc1 == fdest)
			v1 = sgl_last_stage_contents;
		if (piped && fdest != 0 && fsrc2 == fdest)
			v2 = sgl_last_stage_contents;

		if (res_prec)
			dbl_tmp_dest = (double)(v1 * v2);
		else
			sgl_tmp_dest = v1 * v2;
	}

	/* FIXME: Set result-status bits besides MRP. And copy to fsr from
	          last stage.  */
	/* FIXME: Scalar version flows through all stages.  */
	/* FIXME: Mixed precision (only weird for pfmul).  */
	if (!piped)
	{
		/* Scalar version writes the current calculation to the fdest
		   register, with precision specified by the R bit.  */
		if (res_prec)
			set_fregval_d (cpustate, fdest, dbl_tmp_dest);
		else
			set_fregval_s (cpustate, fdest, sgl_tmp_dest);
	}
	else
	{
		/* Pipelined version writes fdest with the result from the last
		   stage of the pipeline.  */
#if 1 /* FIXME: WIP on FSR update.  This may not be correct.  */
		/* Copy 3rd stage MRP to FSR.  */
		if (cpustate->M[num_stages - 2  /* 1 */].stat.mrp)
			cpustate->cregs[CR_FSR] |= 0x10000000;
		else
			cpustate->cregs[CR_FSR] &= ~0x10000000;
#endif

		if (cpustate->M[num_stages - 1].stat.mrp)
			set_fregval_d (cpustate, fdest, dbl_last_stage_contents);
		else
			set_fregval_s (cpustate, fdest, sgl_last_stage_contents);

		/* Now advance pipeline and write current calculation to
		   first stage.  */
		if (num_stages == 3)
		{
			cpustate->M[2] = cpustate->M[1];
			cpustate->M[1] = cpustate->M[0];
		}
		else
			cpustate->M[1]  = cpustate->M[0];

		if (res_prec)
		{
			cpustate->M[0].val.d = dbl_tmp_dest;
			cpustate->M[0].stat.mrp = 1;
		}
		else
		{
			cpustate->M[0].val.s = sgl_tmp_dest;
			cpustate->M[0].stat.mrp = 0;
		}
	}
}


/* Execute "fmlow.dd fsrc1,fsrc2,fdest" instruction.  */
static void insn_fmlow (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);

	double v1 = get_fregval_d (cpustate, fsrc1);
	double v2 = get_fregval_d (cpustate, fsrc2);
	INT64 i1 = *(UINT64 *)&v1;
	INT64 i2 = *(UINT64 *)&v2;
	INT64 tmp = 0;

	/* Only .dd is valid for fmlow.  */
	if ((insn & 0x180) != 0x180)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	/* The lower 32-bits are obvious.  What exactly goes in the upper
	   bits?
	   Technically, the upper-most 10 bits are undefined, but i'd like
	   to be undefined in the same way as the real i860 if possible.  */

	/* Keep lower 53 bits of multiply.  */
	tmp = i1 * i2;
	tmp &= 0x001fffffffffffffULL;
	tmp |= (i1 & 0x8000000000000000LL) ^ (i2 & 0x8000000000000000LL);
	set_fregval_d (cpustate, fdest, *(double *)&tmp);
}


/* Execute [p]fadd.{ss,sd,dd} fsrc1,fsrc2,fdest (.ds disallowed above).  */
static void insn_fadd_sub (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	int src_prec = insn & 0x100;     /* 1 = double, 0 = single.  */
	int res_prec = insn & 0x080;     /* 1 = double, 0 = single.  */
	int piped = insn & 0x400;        /* 1 = pipelined, 0 = scalar.  */
	int is_sub = insn & 1;           /* 1 = sub, 0 = add.  */
	double dbl_tmp_dest = 0.0;
	float sgl_tmp_dest = 0.0;
	double dbl_last_stage_contents = 0.0;
	float sgl_last_stage_contents = 0.0;

	/* Check for invalid .ds combination.  */
	if ((insn & 0x180) == 0x100)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	/* For pipelined version, retrieve the contents of the last stage
	   of the pipeline, whose precision is specified by the ARP bit
	   of the stage's result-status bits.  There are always three stages
	   for pfadd/pfsub.  */
	if (piped)
	{
		if (cpustate->A[2].stat.arp)
			dbl_last_stage_contents = cpustate->A[2].val.d;
		else
			sgl_last_stage_contents = cpustate->A[2].val.s;
	}

	/* Do the operation, being careful about source and result
	   precision.  */
	if (src_prec)
	{
		double v1 = get_fregval_d (cpustate, fsrc1);
		double v2 = get_fregval_d (cpustate, fsrc2);

		/* For pipelined add/sub, if fsrc1 is the same as fdest, then the last
		   stage is bypassed to fsrc1 (rather than using the value in fsrc1).
		   Likewise for fsrc2.  */
		if (piped && fdest != 0 && fsrc1 == fdest)
			v1 = dbl_last_stage_contents;
		if (piped && fdest != 0 && fsrc2 == fdest)
			v2 = dbl_last_stage_contents;

		if (res_prec)
			dbl_tmp_dest = is_sub ? v1 - v2 : v1 + v2;
		else
			sgl_tmp_dest = is_sub ? (float)(v1 - v2) : (float)(v1 + v2);
	}
	else
	{
		float v1 = get_fregval_s (cpustate, fsrc1);
		float v2 = get_fregval_s (cpustate, fsrc2);

		/* For pipelined add/sub, if fsrc1 is the same as fdest, then the last
		   stage is bypassed to fsrc1 (rather than using the value in fsrc1).
		   Likewise for fsrc2.  */
		if (piped && fdest != 0 && fsrc1 == fdest)
			v1 = sgl_last_stage_contents;
		if (piped && fdest != 0 && fsrc2 == fdest)
			v2 = sgl_last_stage_contents;

		if (res_prec)
			dbl_tmp_dest = is_sub ? (double)(v1 - v2) : (double)(v1 + v2);
		else
			sgl_tmp_dest = is_sub ? v1 - v2 : v1 + v2;
	}

	/* FIXME: Set result-status bits besides ARP. And copy to fsr from
	          last stage.  */
	/* FIXME: Scalar version flows through all stages.  */
	if (!piped)
	{
		/* Scalar version writes the current calculation to the fdest
		   register, with precision specified by the R bit.  */
		if (res_prec)
			set_fregval_d (cpustate, fdest, dbl_tmp_dest);
		else
			set_fregval_s (cpustate, fdest, sgl_tmp_dest);
	}
	else
	{
		/* Pipelined version writes fdest with the result from the last
		   stage of the pipeline, with precision specified by the ARP
		   bit of the stage's result-status bits.  */
#if 1 /* FIXME: WIP on FSR update.  This may not be correct.  */
		/* Copy 3rd stage ARP to FSR.  */
		if (cpustate->A[1 /* 2 */].stat.arp)
			cpustate->cregs[CR_FSR] |= 0x20000000;
		else
			cpustate->cregs[CR_FSR] &= ~0x20000000;
#endif
		if (cpustate->A[2].stat.arp)  /* 3rd (last) stage.  */
			set_fregval_d (cpustate, fdest, dbl_last_stage_contents);
		else
			set_fregval_s (cpustate, fdest, sgl_last_stage_contents);

		/* Now advance pipeline and write current calculation to
		   first stage.  */
		cpustate->A[2] = cpustate->A[1];
		cpustate->A[1] = cpustate->A[0];
		if (res_prec)
		{
			cpustate->A[0].val.d = dbl_tmp_dest;
			cpustate->A[0].stat.arp = 1;
		}
		else
		{
			cpustate->A[0].val.s = sgl_tmp_dest;
			cpustate->A[0].stat.arp = 0;
		}
	}
}


/* Operand types for PFAM/PFMAM routine below.  */
enum {
	OP_SRC1     = 0,
	OP_SRC2     = 1,
	OP_KI       = 2,
	OP_KR       = 4,
	OP_T        = 8,
	OP_MPIPE    = 16,
	OP_APIPE    = 32,
	FLAGM       = 64   /* Indicates PFMAM uses M rather than A pipe result.  */
};

/* A table to map DPC value to source operands.

   The PFAM and PFMAM tables are nearly identical, and the only differences
   are that every time PFAM uses the A pipe, PFMAM uses the M pipe instead.
   So we only represent the PFAM table and use a special flag on any entry
   where the PFMAM table would use the M pipe rather than the A pipe.
   Also, entry 16 is not valid for PFMAM.  */
static const struct
{
	int M_unit_op1;
	int M_unit_op2;
	int A_unit_op1;
	int A_unit_op2;
	int T_loaded;
	int K_loaded;
} src_opers[] = {
	/* 0000 */ { OP_KR,   OP_SRC2,        OP_SRC1,        OP_MPIPE,       0, 0},
	/* 0001 */ { OP_KR,   OP_SRC2,        OP_T,           OP_MPIPE,       0, 1},
	/* 0010 */ { OP_KR,   OP_SRC2,        OP_SRC1,        OP_APIPE|FLAGM, 1, 0},
	/* 0011 */ { OP_KR,   OP_SRC2,        OP_T,           OP_APIPE|FLAGM, 1, 1},
	/* 0100 */ { OP_KI,   OP_SRC2,        OP_SRC1,        OP_MPIPE,       0, 0},
	/* 0101 */ { OP_KI,   OP_SRC2,        OP_T,           OP_MPIPE,       0, 1},
	/* 0110 */ { OP_KI,   OP_SRC2,        OP_SRC1,        OP_APIPE|FLAGM, 1, 0},
	/* 0111 */ { OP_KI,   OP_SRC2,        OP_T,           OP_APIPE|FLAGM, 1, 1},
	/* 1000 */ { OP_KR,   OP_APIPE|FLAGM, OP_SRC1,        OP_SRC2,        1, 0},
	/* 1001 */ { OP_SRC1, OP_SRC2,        OP_APIPE|FLAGM, OP_MPIPE,       0, 0},
	/* 1010 */ { OP_KR,   OP_APIPE|FLAGM, OP_SRC1,        OP_SRC2,        0, 0},
	/* 1011 */ { OP_SRC1, OP_SRC2,        OP_T,           OP_APIPE|FLAGM, 1, 0},
	/* 1100 */ { OP_KI,   OP_APIPE|FLAGM, OP_SRC1,        OP_SRC2,        1, 0},
	/* 1101 */ { OP_SRC1, OP_SRC2,        OP_T,           OP_MPIPE,       0, 0},
	/* 1110 */ { OP_KI,   OP_APIPE|FLAGM, OP_SRC1,        OP_SRC2,        0, 0},
	/* 1111 */ { OP_SRC1, OP_SRC2,        OP_T,           OP_APIPE|FLAGM, 0, 0}
};

static float get_fval_from_optype_s (i860s *cpustate, UINT32 insn, int optype)
{
	float retval = 0.0;
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);

	optype &= ~FLAGM;
	switch (optype)
	{
	case OP_SRC1:
		retval = get_fregval_s (cpustate, fsrc1);
		break;
	case OP_SRC2:
		retval = get_fregval_s (cpustate, fsrc2);
		break;
	case OP_KI:
		retval = cpustate->KI.s;
		break;
	case OP_KR:
		retval = cpustate->KR.s;
		break;
	case OP_T:
		retval = cpustate->T.s;
		break;
	case OP_MPIPE:
		/* Last stage is 3rd stage for single precision input.  */
		retval = cpustate->M[2].val.s;
		break;
	case OP_APIPE:
		retval = cpustate->A[2].val.s;
		break;
	default:
		assert (0);
	}

	return retval;
}


static double get_fval_from_optype_d (i860s *cpustate, UINT32 insn, int optype)
{
	double retval = 0.0;
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);

	optype &= ~FLAGM;
	switch (optype)
	{
	case OP_SRC1:
		retval = get_fregval_d (cpustate, fsrc1);
		break;
	case OP_SRC2:
		retval = get_fregval_d (cpustate, fsrc2);
		break;
	case OP_KI:
		retval = cpustate->KI.d;
		break;
	case OP_KR:
		retval = cpustate->KR.d;
		break;
	case OP_T:
		retval = cpustate->T.d;
		break;
	case OP_MPIPE:
		/* Last stage is 2nd stage for double precision input.  */
		retval = cpustate->M[1].val.d;
		break;
	case OP_APIPE:
		retval = cpustate->A[2].val.d;
		break;
	default:
		assert (0);
	}

	return retval;
}


/* Execute pf[m]{a,s}m.{ss,sd,dd} fsrc1,fsrc2,fdest (FP dual ops).

   Since these are always pipelined, the P bit is used to distinguish
   family pfam (P=1) from family pfmam (P=0), and the lower 4 bits
   of the extended opcode is the DPC.

   Note also that the S and R bits are slightly different than normal
   floating point operations.  The S bit denotes the precision of the
   multiplication source, while the R bit denotes the precision of
   the addition source as well as precision of all results.  */
static void insn_dualop (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	int src_prec = insn & 0x100;     /* 1 = double, 0 = single.  */
	int res_prec = insn & 0x080;     /* 1 = double, 0 = single.  */
	int is_pfam = insn & 0x400;      /* 1 = pfam, 0 = pfmam.  */
	int is_sub = insn & 0x10;        /* 1 = pf[m]sm, 0 = pf[m]am.  */
	double dbl_tmp_dest_mul = 0.0;
	float sgl_tmp_dest_mul = 0.0;
	double dbl_tmp_dest_add = 0.0;
	float sgl_tmp_dest_add = 0.0;
	double dbl_last_Mstage_contents = 0.0;
	float sgl_last_Mstage_contents = 0.0;
	double dbl_last_Astage_contents = 0.0;
	float sgl_last_Astage_contents = 0.0;
	int num_mul_stages = src_prec ? 2 : 3;

	int dpc = insn & 0xf;
	int M_unit_op1 = src_opers[dpc].M_unit_op1;
	int M_unit_op2 = src_opers[dpc].M_unit_op2;
	int A_unit_op1 = src_opers[dpc].A_unit_op1;
	int A_unit_op2 = src_opers[dpc].A_unit_op2;
	int T_loaded = src_opers[dpc].T_loaded;
	int K_loaded = src_opers[dpc].K_loaded;

	/* Check for invalid .ds combination.  */
	if ((insn & 0x180) == 0x100)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	if (is_pfam == 0)
	{
		/* Check for invalid DPC combination 16 for PFMAM.  */
		if (dpc == 16)
		{
			unrecog_opcode (cpustate->pc, insn);
			return;
		}

		/* PFMAM table adjustments (M_unit_op1 is never a pipe stage,
		   so no adjustment made for it).   */
		M_unit_op2 = (M_unit_op2 & FLAGM) ? OP_MPIPE : M_unit_op2;
		A_unit_op1 = (A_unit_op1 & FLAGM) ? OP_MPIPE : A_unit_op1;
		A_unit_op2 = (A_unit_op2 & FLAGM) ? OP_MPIPE : A_unit_op2;
	}

	/* FIXME: Check for fsrc1/fdest overlap for some mul DPC combinations.  */

	/* Retrieve the contents of the last stage of the multiplier pipeline,
	   whose precision is specified by the MRP bit of the stage's result-
	   status bits.  Note for multiply, the number of stages is determined
	   by the source precision of the current operation.  */
	if (cpustate->M[num_mul_stages - 1].stat.mrp)
		dbl_last_Mstage_contents = cpustate->M[num_mul_stages - 1].val.d;
	else
		sgl_last_Mstage_contents = cpustate->M[num_mul_stages - 1].val.s;

	/* Similarly, retrieve the last stage of the adder pipe.  */
	if (cpustate->A[2].stat.arp)
		dbl_last_Astage_contents = cpustate->A[2].val.d;
	else
		sgl_last_Astage_contents = cpustate->A[2].val.s;

	/* Do the mul operation, being careful about source and result
	   precision.  */
	if (src_prec)
	{
		double v1 = get_fval_from_optype_d (cpustate, insn, M_unit_op1);
		double v2 = get_fval_from_optype_d (cpustate, insn, M_unit_op2);

		/* For mul, if fsrc2 is the same as fdest, then the last stage
		   is bypassed to fsrc2 (rather than using the value in fsrc2).
		   This bypass is not available for fsrc1, and is undefined behavior.  */
		if (0 && M_unit_op1 == OP_SRC1 && fdest != 0 && fsrc1 == fdest)
			v1 = is_pfam ? dbl_last_Astage_contents : dbl_last_Mstage_contents;
		if (M_unit_op2 == OP_SRC2 && fdest != 0 && fsrc2 == fdest)
			v2 = is_pfam ? dbl_last_Astage_contents : dbl_last_Mstage_contents;

		if (res_prec)
			dbl_tmp_dest_mul = v1 * v2;
		else
			sgl_tmp_dest_mul = (float)(v1 * v2);
	}
	else
	{
		float v1 = get_fval_from_optype_s (cpustate, insn, M_unit_op1);
		float v2 = get_fval_from_optype_s (cpustate, insn, M_unit_op2);

		/* For mul, if fsrc2 is the same as fdest, then the last stage
		   is bypassed to fsrc2 (rather than using the value in fsrc2).
		   This bypass is not available for fsrc1, and is undefined behavior.  */
		if (0 && M_unit_op1 == OP_SRC1 && fdest != 0 && fsrc1 == fdest)
			v1 = is_pfam ? sgl_last_Astage_contents : sgl_last_Mstage_contents;
		if (M_unit_op2 == OP_SRC2 && fdest != 0 && fsrc2 == fdest)
			v2 = is_pfam ? sgl_last_Astage_contents : sgl_last_Mstage_contents;

		if (res_prec)
			dbl_tmp_dest_mul = (double)(v1 * v2);
		else
			sgl_tmp_dest_mul = v1 * v2;
	}

	/* Do the add operation, being careful about source and result
	   precision.  Remember, the R bit indicates source and result precision
	   here.  */
	if (res_prec)
	{
		double v1 = get_fval_from_optype_d (cpustate, insn, A_unit_op1);
		double v2 = get_fval_from_optype_d (cpustate, insn, A_unit_op2);

		/* For add/sub, if fsrc1 is the same as fdest, then the last stage
		   is bypassed to fsrc1 (rather than using the value in fsrc1).
		   Likewise for fsrc2.  */
		if (A_unit_op1 == OP_SRC1 && fdest != 0 && fsrc1 == fdest)
			v1 = is_pfam ? dbl_last_Astage_contents : dbl_last_Mstage_contents;
		if (A_unit_op2 == OP_SRC2 && fdest != 0 && fsrc2 == fdest)
			v2 = is_pfam ? dbl_last_Astage_contents : dbl_last_Mstage_contents;

		if (res_prec)
			dbl_tmp_dest_add = is_sub ? v1 - v2 : v1 + v2;
		else
			sgl_tmp_dest_add = is_sub ? (float)(v1 - v2) : (float)(v1 + v2);
	}
	else
	{
		float v1 = get_fval_from_optype_s (cpustate, insn, A_unit_op1);
		float v2 = get_fval_from_optype_s (cpustate, insn, A_unit_op2);

		/* For add/sub, if fsrc1 is the same as fdest, then the last stage
		   is bypassed to fsrc1 (rather than using the value in fsrc1).
		   Likewise for fsrc2.  */
		if (A_unit_op1 == OP_SRC1 && fdest != 0 && fsrc1 == fdest)
			v1 = is_pfam ? sgl_last_Astage_contents : sgl_last_Mstage_contents;
		if (A_unit_op2 == OP_SRC2 && fdest != 0 && fsrc2 == fdest)
			v2 = is_pfam ? sgl_last_Astage_contents : sgl_last_Mstage_contents;

		if (res_prec)
			dbl_tmp_dest_add = is_sub ? (double)(v1 - v2) : (double)(v1 + v2);
		else
			sgl_tmp_dest_add = is_sub ? v1 - v2 : v1 + v2;
	}

	/* If necessary, load T.  */
	if (T_loaded)
	{
		/* T is loaded from the result of the last stage of the multiplier.  */
		if (cpustate->M[num_mul_stages - 1].stat.mrp)
			cpustate->T.d = dbl_last_Mstage_contents;
		else
			cpustate->T.s = sgl_last_Mstage_contents;
	}

	/* If necessary, load KR or KI.  */
	if (K_loaded)
	{
		/* KI or KR is loaded from the first register input.  */
		if (M_unit_op1 == OP_KI)
		{
			if (src_prec)
				cpustate->KI.d = get_fregval_d (cpustate, fsrc1);
			else
				cpustate->KI.s  = get_fregval_s (cpustate, fsrc1);
		}
		else if (M_unit_op1 == OP_KR)
		{
			if (src_prec)
				cpustate->KR.d = get_fregval_d (cpustate, fsrc1);
			else
				cpustate->KR.s  = get_fregval_s (cpustate, fsrc1);
		}
		else
			assert (0);
	}

	/* Now update fdest (either from adder pipe or multiplier pipe,
	   depending on whether the instruction is pfam or pfmam).  */
	if (is_pfam)
	{
		/* Update fdest with the result from the last stage of the
		   adder pipeline, with precision specified by the ARP
		   bit of the stage's result-status bits.  */
		if (cpustate->A[2].stat.arp)
			set_fregval_d (cpustate, fdest, dbl_last_Astage_contents);
		else
			set_fregval_s (cpustate, fdest, sgl_last_Astage_contents);
	}
	else
	{
		/* Update fdest with the result from the last stage of the
		   multiplier pipeline, with precision specified by the MRP
		   bit of the stage's result-status bits.  */
		if (cpustate->M[num_mul_stages - 1].stat.mrp)
			set_fregval_d (cpustate, fdest, dbl_last_Mstage_contents);
		else
			set_fregval_s (cpustate, fdest, sgl_last_Mstage_contents);
	}

	/* FIXME: Set result-status bits besides MRP. And copy to fsr from
	          last stage.  */
	/* FIXME: Mixed precision (only weird for pfmul).  */
#if 1 /* FIXME: WIP on FSR update.  This may not be correct.  */
	/* Copy 3rd stage MRP to FSR.  */
	if (cpustate->M[num_mul_stages - 2  /* 1 */].stat.mrp)
		cpustate->cregs[CR_FSR] |= 0x10000000;
	else
		cpustate->cregs[CR_FSR] &= ~0x10000000;
#endif

	/* Now advance multiplier pipeline and write current calculation to
	   first stage.  */
	if (num_mul_stages == 3)
	{
		cpustate->M[2] = cpustate->M[1];
		cpustate->M[1] = cpustate->M[0];
	}
	else
		cpustate->M[1]  = cpustate->M[0];

	if (res_prec)
	{
		cpustate->M[0].val.d = dbl_tmp_dest_mul;
		cpustate->M[0].stat.mrp = 1;
	}
	else
	{
		cpustate->M[0].val.s = sgl_tmp_dest_mul;
		cpustate->M[0].stat.mrp = 0;
	}

	/* FIXME: Set result-status bits besides ARP. And copy to fsr from
	          last stage.  */
#if 1 /* FIXME: WIP on FSR update.  This may not be correct.  */
	/* Copy 3rd stage ARP to FSR.  */
	if (cpustate->A[1 /* 2 */].stat.arp)
		cpustate->cregs[CR_FSR] |= 0x20000000;
	else
		cpustate->cregs[CR_FSR] &= ~0x20000000;
#endif

	/* Now advance adder pipeline and write current calculation to
	   first stage.  */
	cpustate->A[2] = cpustate->A[1];
	cpustate->A[1] = cpustate->A[0];
	if (res_prec)
	{
		cpustate->A[0].val.d = dbl_tmp_dest_add;
		cpustate->A[0].stat.arp = 1;
	}
	else
	{
		cpustate->A[0].val.s = sgl_tmp_dest_add;
		cpustate->A[0].stat.arp = 0;
	}
}


/* Execute frcp.{ss,sd,dd} fsrc2,fdest (.ds disallowed above).  */
static void insn_frcp (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	int src_prec = insn & 0x100;     /* 1 = double, 0 = single.  */
	int res_prec = insn & 0x080;     /* 1 = double, 0 = single.  */

	/* Do the operation, being careful about source and result
	   precision.  */
	if (src_prec)
	{
		double v = get_fregval_d (cpustate, fsrc2);
		double res;
		if (v == (double)0.0)
		{
			/* Generate source-exception trap if fsrc2 is 0.  */
			if (0 /* && GET_FSR_FTE () */)
			{
				SET_PSR_FT (1);
				SET_FSR_SE (1);
				cpustate->pending_trap = GET_FSR_FTE ();
			}
			/* Set fdest to INF or some other exceptional value here?  */
		}
		else
		{
			/* Real i860 isn't a precise as a real divide, but this should
			   be okay.  */
			SET_FSR_SE (0);
			*((UINT64 *)&v) &= 0xfffff00000000000ULL;
			res = (double)1.0/v;
			*((UINT64 *)&res) &= 0xfffff00000000000ULL;
			if (res_prec)
				set_fregval_d (cpustate, fdest, res);
			else
				set_fregval_s (cpustate, fdest, (float)res);
		}
	}
	else
	{
		float v = get_fregval_s (cpustate, fsrc2);
		float res;
		if (v == 0.0)
		{
			/* Generate source-exception trap if fsrc2 is 0.  */
			if (0 /* GET_FSR_FTE () */)
			{
				SET_PSR_FT (1);
				SET_FSR_SE (1);
				cpustate->pending_trap = GET_FSR_FTE ();
			}
			/* Set fdest to INF or some other exceptional value here?  */
		}
		else
		{
			/* Real i860 isn't a precise as a real divide, but this should
			   be okay.  */
			SET_FSR_SE (0);
			*((UINT32 *)&v) &= 0xffff8000;
			res = (float)1.0/v;
			*((UINT32 *)&res) &= 0xffff8000;
			if (res_prec)
				set_fregval_d (cpustate, fdest, (double)res);
			else
				set_fregval_s (cpustate, fdest, res);
		}
	}
}


/* Execute frsqr.{ss,sd,dd} fsrc2,fdest (.ds disallowed above).  */
static void insn_frsqr (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	int src_prec = insn & 0x100;     /* 1 = double, 0 = single.  */
	int res_prec = insn & 0x080;     /* 1 = double, 0 = single.  */

	/* Check for invalid .ds combination.  */
	if ((insn & 0x180) == 0x100)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	/* Check for invalid .ds combination.  */
	if ((insn & 0x180) == 0x100)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	/* Do the operation, being careful about source and result
	   precision.  */
	if (src_prec)
	{
		double v = get_fregval_d (cpustate, fsrc2);
		double res;
		if (v == 0.0 || v < 0.0)
		{
			/* Generate source-exception trap if fsrc2 is 0 or negative.  */
			if (0 /* GET_FSR_FTE () */)
			{
				SET_PSR_FT (1);
				SET_FSR_SE (1);
				cpustate->pending_trap = GET_FSR_FTE ();
			}
			/* Set fdest to INF or some other exceptional value here?  */
		}
		else
		{
			SET_FSR_SE (0);
			*((UINT64 *)&v) &= 0xfffff00000000000ULL;
			res = (double)1.0/sqrt (v);
			*((UINT64 *)&res) &= 0xfffff00000000000ULL;
			if (res_prec)
				set_fregval_d (cpustate, fdest, res);
			else
				set_fregval_s (cpustate, fdest, (float)res);
		}
	}
	else
	{
		float v = get_fregval_s (cpustate, fsrc2);
		float res;
		if (v == 0.0 || v < 0.0)
		{
			/* Generate source-exception trap if fsrc2 is 0 or negative.  */
			if (0 /* GET_FSR_FTE () */)
			{
				SET_PSR_FT (1);
				SET_FSR_SE (1);
				cpustate->pending_trap = GET_FSR_FTE ();
			}
			/* Set fdest to INF or some other exceptional value here?  */
		}
		else
		{
			SET_FSR_SE (0);
			*((UINT32 *)&v) &= 0xffff8000;
			res = (float)1.0/sqrt (v);
			*((UINT32 *)&res) &= 0xffff8000;
			if (res_prec)
				set_fregval_d (cpustate, fdest, (double)res);
			else
				set_fregval_s (cpustate, fdest, res);
		}
	}
}


/* Execute fxfr fsrc1,idest.  */
static void insn_fxfr (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 idest = get_idest (insn);
	float fv = 0;

	/* This is a bit-pattern transfer, not a conversion.  */
	fv = get_fregval_s (cpustate, fsrc1);
	set_iregval (idest, *(UINT32 *)&fv);
}


/* Execute [p]ftrunc.{ss,sd,dd} fsrc1,idest.  */
/* FIXME: Is .ss really a valid combination?  On the one hand,
   the programmer's reference (1990) lists ftrunc.p where .p
   is any of {ss,sd,dd}.  On the other hand, a paragraph on the
   same page states that [p]ftrunc must specify double-precision
   results.  Inconsistent.
   Update: The vendor SVR4 assembler does not accept .ss combination,
   so the latter sentence above appears to be the correct way.  */
static void insn_ftrunc (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fdest = get_fdest (insn);
	int src_prec = insn & 0x100;     /* 1 = double, 0 = single.  */
	int res_prec = insn & 0x080;     /* 1 = double, 0 = single.  */
	int piped = insn & 0x400;        /* 1 = pipelined, 0 = scalar.  */

	/* Check for invalid .ds or .ss combinations.  */
	if ((insn & 0x080) == 0)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	/* Do the operation, being careful about source and result
	   precision.  Operation: fdest = integer part of fsrc1 in
	   lower 32-bits.  */
	if (src_prec)
	{
		double v1 = get_fregval_d (cpustate, fsrc1);
		INT32 iv = (INT32)v1;
		/* We always write a single, since the lower 32-bits of fdest
		   get the result (and the even numbered reg is the lower).  */
		set_fregval_s (cpustate, fdest, *(float *)&iv);
	}
	else
	{
		float v1 = get_fregval_s (cpustate, fsrc1);
		INT32 iv = (INT32)v1;
		/* We always write a single, since the lower 32-bits of fdest
		   get the result (and the even numbered reg is the lower).  */
		set_fregval_s (cpustate, fdest, *(float *)&iv);
	}

	/* FIXME: Handle updating of pipestages for pftrunc.  */
	/* Includes looking at ARP (add result precision.) */
	if (piped)
	{
		fprintf (stderr, "insn_ftrunc: FIXME: pipelined not functional yet.\n");
		if (res_prec)
			set_fregval_d (cpustate, fdest, 0.0);
		else
			set_fregval_s (cpustate, fdest, 0.0);
	}
}


/* Execute [p]famov.{ss,sd,ds,dd} fsrc1,fdest.  */
static void insn_famov (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fdest = get_fdest (insn);
	int src_prec = insn & 0x100;     /* 1 = double, 0 = single.  */
	int res_prec = insn & 0x080;     /* 1 = double, 0 = single.  */
	int piped = insn & 0x400;        /* 1 = pipelined, 0 = scalar.  */
	double dbl_tmp_dest = 0.0;
	double sgl_tmp_dest = 0.0;

	/* Do the operation, being careful about source and result
	   precision.  */
	if (src_prec)
	{
		double v1 = get_fregval_d (cpustate, fsrc1);
		if (res_prec)
			dbl_tmp_dest = v1;
		else
			sgl_tmp_dest = (float)v1;
	}
	else
	{
		float v1 = get_fregval_s (cpustate, fsrc1);
		if (res_prec)
			dbl_tmp_dest = (double)v1;
		else
			sgl_tmp_dest = v1;
	}

	/* FIXME: Set result-status bits besides ARP. And copy to fsr from
	          last stage.  */
	/* FIXME: Scalar version flows through all stages.  */
	if (!piped)
	{
		/* Scalar version writes the current calculation to the fdest
		   register, with precision specified by the R bit.  */
		if (res_prec)
			set_fregval_d (cpustate, fdest, dbl_tmp_dest);
		else
			set_fregval_s (cpustate, fdest, sgl_tmp_dest);
	}
	else
	{
		/* Pipelined version writes fdest with the result from the last
		   stage of the pipeline, with precision specified by the ARP
		   bit of the stage's result-status bits.  */
#if 1 /* FIXME: WIP on FSR update.  This may not be correct.  */
		/* Copy 3rd stage ARP to FSR.  */
		if (cpustate->A[1 /* 2 */].stat.arp)
			cpustate->cregs[CR_FSR] |= 0x20000000;
		else
			cpustate->cregs[CR_FSR] &= ~0x20000000;
#endif
		if (cpustate->A[2].stat.arp)  /* 3rd (last) stage.  */
			set_fregval_d (cpustate, fdest, cpustate->A[2].val.d);
		else
			set_fregval_s (cpustate, fdest, cpustate->A[2].val.s);

		/* Now advance pipeline and write current calculation to
		   first stage.  */
		cpustate->A[2] = cpustate->A[1];
		cpustate->A[1] = cpustate->A[0];
		if (res_prec)
		{
			cpustate->A[0].val.d = dbl_tmp_dest;
			cpustate->A[0].stat.arp = 1;
		}
		else
		{
			cpustate->A[0].val.s = sgl_tmp_dest;
			cpustate->A[0].stat.arp = 0;
		}
	}
}


/* Execute [p]fiadd/sub.{ss,dd} fsrc1,fsrc2,fdest.  */
static void insn_fiadd_sub (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	int src_prec = insn & 0x100;     /* 1 = double, 0 = single.  */
	int res_prec = insn & 0x080;     /* 1 = double, 0 = single.  */
	int piped = insn & 0x400;        /* 1 = pipelined, 0 = scalar.  */
	int is_sub = insn & 0x4;         /* 1 = sub, 0 = add.  */
	double dbl_tmp_dest = 0.0;
	float sgl_tmp_dest = 0.0;

	/* Check for invalid .ds and .sd combinations.  */
	if ((insn & 0x180) == 0x100
		|| (insn & 0x180) == 0x080)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	/* Do the operation, being careful about source and result
	   precision.  */
	if (src_prec)
	{
		double v1 = get_fregval_d (cpustate, fsrc1);
		double v2 = get_fregval_d (cpustate, fsrc2);
		UINT64 iv1 = *(UINT64 *)&v1;
		UINT64 iv2 = *(UINT64 *)&v2;
		UINT64 r;
		if (is_sub)
			r = iv1 - iv2;
		else
			r = iv1 + iv2;
		if (res_prec)
			dbl_tmp_dest = *(double *)&r;
		else
			assert (0);    /* .ds not allowed.  */
	}
	else
	{
		float v1 = get_fregval_s (cpustate, fsrc1);
		float v2 = get_fregval_s (cpustate, fsrc2);
		UINT64 iv1 = (UINT64)(*(UINT32 *)&v1);
		UINT64 iv2 = (UINT64)(*(UINT32 *)&v2);
		UINT32 r;
		if (is_sub)
			r = (UINT32)(iv1 - iv2);
		else
			r = (UINT32)(iv1 + iv2);
		if (res_prec)
			assert (0);    /* .sd not allowed.  */
		else
			sgl_tmp_dest = *(float *)&r;
	}

	/* FIXME: Copy result-status bit IRP to fsr from last stage.  */
	/* FIXME: Scalar version flows through all stages.  */
	if (!piped)
	{
		/* Scalar version writes the current calculation to the fdest
		   register, with precision specified by the R bit.  */
		if (res_prec)
			set_fregval_d (cpustate, fdest, dbl_tmp_dest);
		else
			set_fregval_s (cpustate, fdest, sgl_tmp_dest);
	}
	else
	{
		/* Pipelined version writes fdest with the result from the last
		   stage of the pipeline, with precision specified by the IRP
		   bit of the stage's result-status bits.  */
#if 1 /* FIXME: WIP on FSR update.  This may not be correct.  */
		/* Copy stage IRP to FSR.  */
		if (res_prec)
			cpustate->cregs[CR_FSR] |= 0x08000000;
		else
			cpustate->cregs[CR_FSR] &= ~0x08000000;
#endif
		if (cpustate->G.stat.irp)   /* 1st (and last) stage.  */
			set_fregval_d (cpustate, fdest, cpustate->G.val.d);
		else
			set_fregval_s (cpustate, fdest, cpustate->G.val.s);

		/* Now write current calculation to first and only stage.  */
		if (res_prec)
		{
			cpustate->G.val.d = dbl_tmp_dest;
			cpustate->G.stat.irp = 1;
		}
		else
		{
			cpustate->G.val.s = sgl_tmp_dest;
			cpustate->G.stat.irp = 0;
		}
	}
}


/* Execute pf{gt,le,eq}.{ss,dd} fsrc1,fsrc2,fdest.
   Opcode pfgt has R bit cleared; pfle has R bit set.  */
static void insn_fcmp (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	int src_prec = insn & 0x100;     /* 1 = double, 0 = single.  */
	double dbl_tmp_dest = 0.0;
	double sgl_tmp_dest = 0.0;
	/* int is_eq = insn & 1; */
	int is_gt = ((insn & 0x81) == 0x00);
	int is_le = ((insn & 0x81) == 0x80);

	/* Do the operation.  Source and result precision must be the same.
	     pfgt: CC set     if fsrc1 > fsrc2, else cleared.
	     pfle: CC cleared if fsrc1 <= fsrc2, else set.
	     pfeq: CC set     if fsrc1 = fsrc2, else cleared.

	   Note that the compares write an undefined (but non-exceptional)
	   result into the first stage of the adder pipeline.  We'll model
	   this by just pushing in dbl_ or sgl_tmp_dest which equal 0.0.  */
	if (src_prec)
	{
		double v1 = get_fregval_d (cpustate, fsrc1);
		double v2 = get_fregval_d (cpustate, fsrc2);
		if (is_gt)                /* gt.  */
			SET_PSR_CC (v1 > v2 ? 1 : 0);
		else if (is_le)           /* le.  */
			SET_PSR_CC (v1 <= v2 ? 0 : 1);
		else                      /* eq.  */
			SET_PSR_CC (v1 == v2 ? 1 : 0);
	}
	else
	{
		float v1 = get_fregval_s (cpustate, fsrc1);
		float v2 = get_fregval_s (cpustate, fsrc2);
		if (is_gt)                /* gt.  */
			SET_PSR_CC (v1 > v2 ? 1 : 0);
		else if (is_le)           /* le.  */
			SET_PSR_CC (v1 <= v2 ? 0 : 1);
		else                      /* eq.  */
			SET_PSR_CC (v1 == v2 ? 1 : 0);
	}

	/* FIXME: Set result-status bits besides ARP. And copy to fsr from
	          last stage.  */
	/* These write fdest with the result from the last
	   stage of the pipeline, with precision specified by the ARP
	   bit of the stage's result-status bits.  */
#if 1 /* FIXME: WIP on FSR update.  This may not be correct.  */
	/* Copy 3rd stage ARP to FSR.  */
	if (cpustate->A[1 /* 2 */].stat.arp)
		cpustate->cregs[CR_FSR] |= 0x20000000;
	else
		cpustate->cregs[CR_FSR] &= ~0x20000000;
#endif
	if (cpustate->A[2].stat.arp)  /* 3rd (last) stage.  */
		set_fregval_d (cpustate, fdest, cpustate->A[2].val.d);
	else
		set_fregval_s (cpustate, fdest, cpustate->A[2].val.s);

	/* Now advance pipeline and write current calculation to
	   first stage.  */
	cpustate->A[2] = cpustate->A[1];
	cpustate->A[1] = cpustate->A[0];
	if (src_prec)
	{
		cpustate->A[0].val.d = dbl_tmp_dest;
		cpustate->A[0].stat.arp = 1;
	}
	else
	{
		cpustate->A[0].val.s = sgl_tmp_dest;
		cpustate->A[0].stat.arp = 0;
	}
}


/* Execute [p]fzchk{l,s} fsrc1,fsrc2,fdest.
   The fzchk instructions have S and R bits set.  */
static void insn_fzchk (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	int piped = insn & 0x400;        /* 1 = pipelined, 0 = scalar.  */
	int is_fzchks = insn & 8;        /* 1 = fzchks, 0 = fzchkl.  */
	double dbl_tmp_dest = 0.0;
	int i;
	double v1 = get_fregval_d (cpustate, fsrc1);
	double v2 = get_fregval_d (cpustate, fsrc2);
	UINT64 iv1 = *(UINT64 *)&v1;
	UINT64 iv2 = *(UINT64 *)&v2;
	UINT64 r = 0;
	char pm = GET_PSR_PM ();

	/* Check for S and R bits set.  */
	if ((insn & 0x180) != 0x180)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	/* Do the operation.  The fzchks version operates in parallel on
	   four 16-bit pixels, while the fzchkl operates on two 32-bit
	   pixels (pixels are unsigned ordinals in this context).  */
	if (is_fzchks)
	{
		pm = (pm >> 4) & 0x0f;
		for (i = 3; i >= 0; i--)
		{
			UINT16 ps1 = (iv1 >> (i * 16)) & 0xffff;
			UINT16 ps2 = (iv2 >> (i * 16)) & 0xffff;
			if (ps2 <= ps1)
			{
				r |= ((UINT64)ps2 << (i * 16));
				pm |= (1 << (7 - (3 - i)));
			}
			else
			{
				r |= ((UINT64)ps1 << (i * 16));
				pm &= ~(1 << (7 - (3 - i)));
			}
		}
	}
	else
	{
		pm = (pm >> 2) & 0x3f;
		for (i = 1; i >= 0; i--)
		{
			UINT32 ps1 = (iv1 >> (i * 32)) & 0xffffffff;
			UINT32 ps2 = (iv2 >> (i * 32)) & 0xffffffff;
			if (ps2 <= ps1)
			{
				r |= ((UINT64)ps2 << (i * 32));
				pm |= (1 << (7 - (1 - i)));
			}
			else
			{
				r |= ((UINT64)ps1 << (i * 32));
				pm &= ~(1 << (7 - (1 - i)));
			}
		}
	}

	dbl_tmp_dest = *(double *)&r;
	SET_PSR_PM (pm);
	cpustate->merge = 0;

	/* FIXME: Copy result-status bit IRP to fsr from last stage.  */
	/* FIXME: Scalar version flows through all stages.  */
	if (!piped)
	{
		/* Scalar version writes the current calculation to the fdest
		   register, always with double precision.  */
		set_fregval_d (cpustate, fdest, dbl_tmp_dest);
	}
	else
	{
		/* Pipelined version writes fdest with the result from the last
		   stage of the pipeline, with precision specified by the IRP
		   bit of the stage's result-status bits.  */
		if (cpustate->G.stat.irp)   /* 1st (and last) stage.  */
			set_fregval_d (cpustate, fdest, cpustate->G.val.d);
		else
			set_fregval_s (cpustate, fdest, cpustate->G.val.s);

		/* Now write current calculation to first and only stage.  */
		cpustate->G.val.d = dbl_tmp_dest;
		cpustate->G.stat.irp = 1;
	}
}


/* Execute [p]form.dd fsrc1,fdest.
   The form.dd instructions have S and R bits set.  */
static void insn_form (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fdest = get_fdest (insn);
	int piped = insn & 0x400;        /* 1 = pipelined, 0 = scalar.  */
	double dbl_tmp_dest = 0.0;
	double v1 = get_fregval_d (cpustate, fsrc1);
	UINT64 iv1 = *(UINT64 *)&v1;

	/* Check for S and R bits set.  */
	if ((insn & 0x180) != 0x180)
	{
		unrecog_opcode (cpustate->pc, insn);
		return;
	}

	iv1 |= cpustate->merge;
	dbl_tmp_dest = *(double *)&iv1;
	cpustate->merge = 0;

	/* FIXME: Copy result-status bit IRP to fsr from last stage.  */
	/* FIXME: Scalar version flows through all stages.  */
	if (!piped)
	{
		/* Scalar version writes the current calculation to the fdest
		   register, always with double precision.  */
		set_fregval_d (cpustate, fdest, dbl_tmp_dest);
	}
	else
	{
		/* Pipelined version writes fdest with the result from the last
		   stage of the pipeline, with precision specified by the IRP
		   bit of the stage's result-status bits.  */
		if (cpustate->G.stat.irp)   /* 1st (and last) stage.  */
			set_fregval_d (cpustate, fdest, cpustate->G.val.d);
		else
			set_fregval_s (cpustate, fdest, cpustate->G.val.s);

		/* Now write current calculation to first and only stage.  */
		cpustate->G.val.d = dbl_tmp_dest;
		cpustate->G.stat.irp = 1;
	}
}


/* Execute [p]faddp fsrc1,fsrc2,fdest.  */
static void insn_faddp (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	int piped = insn & 0x400;        /* 1 = pipelined, 0 = scalar.  */
	double dbl_tmp_dest = 0.0;
	double v1 = get_fregval_d (cpustate, fsrc1);
	double v2 = get_fregval_d (cpustate, fsrc2);
	UINT64 iv1 = *(UINT64 *)&v1;
	UINT64 iv2 = *(UINT64 *)&v2;
	UINT64 r = 0;
	int ps = GET_PSR_PS ();

	r = iv1 + iv2;
	dbl_tmp_dest = *(double *)&r;

	/* Update the merge register depending on the pixel size.
	   PS: 0 = 8 bits, 1 = 16 bits, 2 = 32-bits.  */
	if (ps == 0)
	{
		cpustate->merge = ((cpustate->merge >> 8) & ~0xff00ff00ff00ff00ULL);
		cpustate->merge |= (r & 0xff00ff00ff00ff00ULL);
	}
	else if (ps == 1)
	{
		cpustate->merge = ((cpustate->merge >> 6) & ~0xfc00fc00fc00fc00ULL);
		cpustate->merge |= (r & 0xfc00fc00fc00fc00ULL);
	}
	else if (ps == 2)
	{
		cpustate->merge = ((cpustate->merge >> 8) & ~0xff000000ff000000ULL);
		cpustate->merge |= (r & 0xff000000ff000000ULL);
	}
#ifdef TRACE_UNDEFINED_I860
	else
		fprintf (stderr, "insn_faddp: Undefined i860XR behavior, invalid value %d for pixel size.\n", ps);
#endif

	/* FIXME: Copy result-status bit IRP to fsr from last stage.  */
	/* FIXME: Scalar version flows through all stages.  */
	if (!piped)
	{
		/* Scalar version writes the current calculation to the fdest
		   register, always with double precision.  */
		set_fregval_d (cpustate, fdest, dbl_tmp_dest);
	}
	else
	{
		/* Pipelined version writes fdest with the result from the last
		   stage of the pipeline, with precision specified by the IRP
		   bit of the stage's result-status bits.  */
		if (cpustate->G.stat.irp)   /* 1st (and last) stage.  */
			set_fregval_d (cpustate, fdest, cpustate->G.val.d);
		else
			set_fregval_s (cpustate, fdest, cpustate->G.val.s);

		/* Now write current calculation to first and only stage.  */
		cpustate->G.val.d = dbl_tmp_dest;
		cpustate->G.stat.irp = 1;
	}
}


/* Execute [p]faddz fsrc1,fsrc2,fdest.  */
static void insn_faddz (i860s *cpustate, UINT32 insn)
{
	UINT32 fsrc1 = get_fsrc1 (insn);
	UINT32 fsrc2 = get_fsrc2 (insn);
	UINT32 fdest = get_fdest (insn);
	int piped = insn & 0x400;        /* 1 = pipelined, 0 = scalar.  */
	double dbl_tmp_dest = 0.0;
	double v1 = get_fregval_d (cpustate, fsrc1);
	double v2 = get_fregval_d (cpustate, fsrc2);
	UINT64 iv1 = *(UINT64 *)&v1;
	UINT64 iv2 = *(UINT64 *)&v2;
	UINT64 r = 0;

	r = iv1 + iv2;
	dbl_tmp_dest = *(double *)&r;

	/* Update the merge register depending on the pixel size.  */
	cpustate->merge = ((cpustate->merge >> 16) & ~0xffff0000ffff0000ULL);
	cpustate->merge |= (r & 0xffff0000ffff0000ULL);

	/* FIXME: Copy result-status bit IRP to fsr from last stage.  */
	/* FIXME: Scalar version flows through all stages.  */
	if (!piped)
	{
		/* Scalar version writes the current calculation to the fdest
		   register, always with double precision.  */
		set_fregval_d (cpustate, fdest, dbl_tmp_dest);
	}
	else
	{
		/* Pipelined version writes fdest with the result from the last
		   stage of the pipeline, with precision specified by the IRP
		   bit of the stage's result-status bits.  */
		if (cpustate->G.stat.irp)   /* 1st (and last) stage.  */
			set_fregval_d (cpustate, fdest, cpustate->G.val.d);
		else
			set_fregval_s (cpustate, fdest, cpustate->G.val.s);

		/* Now write current calculation to first and only stage.  */
		cpustate->G.val.d = dbl_tmp_dest;
		cpustate->G.stat.irp = 1;
	}
}


/* Flags for the decode table.  */
enum {
	DEC_MORE    = 1,    /* More decoding necessary.  */
	DEC_DECODED = 2     /* Fully decoded, go.  */
};


struct decode_tbl_t {
	/* Execute function for this opcode.  */
	void (*insn_exec)(i860s *, UINT32);

	/* Flags for this opcode.  */
	char flags;
};


/* First-level decode table (i.e., for the 6 primary opcode bits).  */
static const decode_tbl_t decode_tbl[64] = {
	/* A slight bit of decoding for loads and stores is done in the
	   execution routines (operand size and addressing mode), which
	   is why their respective entries are identical.  */
	{ insn_ldx,         DEC_DECODED}, /* ld.b isrc1(isrc2),idest.  */
	{ insn_ldx,         DEC_DECODED}, /* ld.b #const(isrc2),idest.  */
	{ insn_ixfr,        DEC_DECODED}, /* ixfr isrc1ni,fdest.  */
	{ insn_stx,         DEC_DECODED}, /* st.b isrc1ni,#const(isrc2).  */
	{ insn_ldx,         DEC_DECODED}, /* ld.{s,l} isrc1(isrc2),idest.  */
	{ insn_ldx,         DEC_DECODED}, /* ld.{s,l} #const(isrc2),idest.  */
	{ 0,                0},
	{ insn_stx,         DEC_DECODED}, /* st.{s,l} isrc1ni,#const(isrc2),idest.*/
	{ insn_fldy,        DEC_DECODED}, /* fld.{l,d,q} isrc1(isrc2)[++],fdest. */
	{ insn_fldy,        DEC_DECODED}, /* fld.{l,d,q} #const(isrc2)[++],fdest. */
	{ insn_fsty,        DEC_DECODED}, /* fst.{l,d,q} fdest,isrc1(isrc2)[++] */
	{ insn_fsty,        DEC_DECODED}, /* fst.{l,d,q} fdest,#const(isrc2)[++] */
	{ insn_ld_ctrl,     DEC_DECODED}, /* ld.c csrc2,idest.  */
	{ insn_flush,       DEC_DECODED}, /* flush #const(isrc2) (or autoinc).  */
	{ insn_st_ctrl,     DEC_DECODED}, /* st.c isrc1,csrc2.  */
	{ insn_pstd,        DEC_DECODED}, /* pst.d fdest,#const(isrc2)[++].  */
	{ insn_bri,         DEC_DECODED}, /* bri isrc1ni.  */
	{ insn_trap,        DEC_DECODED}, /* trap isrc1ni,isrc2,idest.   */
	{ 0,                DEC_MORE}, /* FP ESCAPE FORMAT, more decode.  */
	{ 0,                DEC_MORE}, /* CORE ESCAPE FORMAT, more decode.  */
	{ insn_btne,        DEC_DECODED}, /* btne isrc1,isrc2,sbroff.  */
	{ insn_btne_imm,    DEC_DECODED}, /* btne #const,isrc2,sbroff.  */
	{ insn_bte,         DEC_DECODED}, /* bte isrc1,isrc2,sbroff.  */
	{ insn_bte_imm,     DEC_DECODED}, /* bte #const5,isrc2,idest.  */
	{ insn_fldy,        DEC_DECODED}, /* pfld.{l,d,q} isrc1(isrc2)[++],fdest.*/
	{ insn_fldy,        DEC_DECODED}, /* pfld.{l,d,q} #const(isrc2)[++],fdest.*/
	{ insn_br,          DEC_DECODED}, /* br lbroff.  */
	{ insn_call,        DEC_DECODED}, /* call lbroff .  */
	{ insn_bc,          DEC_DECODED}, /* bc lbroff.  */
	{ insn_bct,         DEC_DECODED}, /* bc.t lbroff.  */
	{ insn_bnc,         DEC_DECODED}, /* bnc lbroff.  */
	{ insn_bnct,        DEC_DECODED}, /* bnc.t lbroff.  */
	{ insn_addu,        DEC_DECODED}, /* addu isrc1,isrc2,idest.  */
	{ insn_addu_imm,    DEC_DECODED}, /* addu #const,isrc2,idest.  */
	{ insn_subu,        DEC_DECODED}, /* subu isrc1,isrc2,idest.  */
	{ insn_subu_imm,    DEC_DECODED}, /* subu #const,isrc2,idest.  */
	{ insn_adds,        DEC_DECODED}, /* adds isrc1,isrc2,idest.  */
	{ insn_adds_imm,    DEC_DECODED}, /* adds #const,isrc2,idest.  */
	{ insn_subs,        DEC_DECODED}, /* subs isrc1,isrc2,idest.  */
	{ insn_subs_imm,    DEC_DECODED}, /* subs #const,isrc2,idest.  */
	{ insn_shl,         DEC_DECODED}, /* shl isrc1,isrc2,idest.  */
	{ insn_shl_imm,     DEC_DECODED}, /* shl #const,isrc2,idest.  */
	{ insn_shr,         DEC_DECODED}, /* shr isrc1,isrc2,idest.  */
	{ insn_shr_imm,     DEC_DECODED}, /* shr #const,isrc2,idest.  */
	{ insn_shrd,        DEC_DECODED}, /* shrd isrc1ni,isrc2,idest.  */
	{ insn_bla,         DEC_DECODED}, /* bla isrc1ni,isrc2,sbroff.  */
	{ insn_shra,        DEC_DECODED}, /* shra isrc1,isrc2,idest.  */
	{ insn_shra_imm,    DEC_DECODED}, /* shra #const,isrc2,idest.  */
	{ insn_and,         DEC_DECODED}, /* and isrc1,isrc2,idest.  */
	{ insn_and_imm,     DEC_DECODED}, /* and #const,isrc2,idest.  */
	{ 0,                0},
	{ insn_andh_imm,    DEC_DECODED}, /* andh #const,isrc2,idest.  */
	{ insn_andnot,      DEC_DECODED}, /* andnot isrc1,isrc2,idest.  */
	{ insn_andnot_imm,  DEC_DECODED}, /* andnot #const,isrc2,idest.  */
	{ 0,                0},
	{ insn_andnoth_imm, DEC_DECODED}, /* andnoth #const,isrc2,idest.  */
	{ insn_or,          DEC_DECODED}, /* or isrc1,isrc2,idest.  */
	{ insn_or_imm,      DEC_DECODED}, /* or #const,isrc2,idest.  */
	{ 0,                0},
	{ insn_orh_imm,     DEC_DECODED}, /* orh #const,isrc2,idest.  */
	{ insn_xor,         DEC_DECODED}, /* xor isrc1,isrc2,idest.  */
	{ insn_xor_imm,     DEC_DECODED}, /* xor #const,isrc2,idest.  */
	{ 0,                0},
	{ insn_xorh_imm,    DEC_DECODED}, /* xorh #const,isrc2,idest.  */
};


/* Second-level decode table (i.e., for the 3 core escape opcode bits).  */
static const decode_tbl_t core_esc_decode_tbl[8] = {
	{ 0,                0},
	{ 0,                0}, /* lock  (FIXME: unimplemented).  */
	{ insn_calli,       DEC_DECODED}, /* calli isrc1ni.                 */
	{ 0,                0},
	{ insn_intovr,      DEC_DECODED}, /* intovr.                        */
	{ 0,                0},
	{ 0,                0},
	{ 0,                0}, /* unlock (FIXME: unimplemented). */
};


/* Second-level decode table (i.e., for the 7 FP extended opcode bits).  */
static const decode_tbl_t fp_decode_tbl[128] = {
	/* Floating point instructions.  The least significant 7 bits are
	   the (extended) opcode and bits 10:7 are P,D,S,R respectively
	   ([p]ipelined, [d]ual, [s]ource prec., [r]esult prec.).
	   For some operations, I defer decoding the P,S,R bits to the
	   emulation routine for them.  */
	{ insn_dualop,      DEC_DECODED}, /* 0x00 pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x01 pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x02 pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x03 pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x04 pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x05 pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x06 pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x07 pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x08 pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x09 pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x0A pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x0B pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x0C pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x0D pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x0E pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x0F pf[m]am */
	{ insn_dualop,      DEC_DECODED}, /* 0x10 pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x11 pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x12 pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x13 pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x14 pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x15 pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x16 pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x17 pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x18 pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x19 pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x1A pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x1B pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x1C pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x1D pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x1E pf[m]sm */
	{ insn_dualop,      DEC_DECODED}, /* 0x1F pf[m]sm */
	{ insn_fmul,        DEC_DECODED}, /* 0x20 [p]fmul */
	{ insn_fmlow,       DEC_DECODED}, /* 0x21 fmlow.dd */
	{ insn_frcp,        DEC_DECODED}, /* 0x22 frcp.{ss,sd,dd} */
	{ insn_frsqr,       DEC_DECODED}, /* 0x23 frsqr.{ss,sd,dd} */
	{ insn_fmul,        DEC_DECODED}, /* 0x24 pfmul3.dd */
	{ 0,                0}, /* 0x25 */
	{ 0,                0}, /* 0x26 */
	{ 0,                0}, /* 0x27 */
	{ 0,                0}, /* 0x28 */
	{ 0,                0}, /* 0x29 */
	{ 0,                0}, /* 0x2A */
	{ 0,                0}, /* 0x2B */
	{ 0,                0}, /* 0x2C */
	{ 0,                0}, /* 0x2D */
	{ 0,                0}, /* 0x2E */
	{ 0,                0}, /* 0x2F */
	{ insn_fadd_sub,    DEC_DECODED}, /* 0x30, [p]fadd.{ss,sd,dd} */
	{ insn_fadd_sub,    DEC_DECODED}, /* 0x31, [p]fsub.{ss,sd,dd} */
	{ 0,                0}, /* 0x32, [p]fix.{ss,sd,dd}  FIXME: nyi. */
	{ insn_famov,       DEC_DECODED}, /* 0x33, [p]famov.{ss,sd,ds,dd} */
	{ insn_fcmp,        DEC_DECODED}, /* 0x34, pf{gt,le}.{ss,dd} */
	{ insn_fcmp,        DEC_DECODED}, /* 0x35, pfeq.{ss,dd} */
	{ 0,                0}, /* 0x36 */
	{ 0,                0}, /* 0x37 */
	{ 0,                0}, /* 0x38 */
	{ 0,                0}, /* 0x39 */
	{ insn_ftrunc,      DEC_DECODED}, /* 0x3A, [p]ftrunc.{ss,sd,dd} */
	{ 0,                0}, /* 0x3B */
	{ 0,                0}, /* 0x3C */
	{ 0,                0}, /* 0x3D */
	{ 0,                0}, /* 0x3E */
	{ 0,                0}, /* 0x3F */
	{ insn_fxfr,        DEC_DECODED}, /* 0x40, fxfr */
	{ 0,                0}, /* 0x41 */
	{ 0,                0}, /* 0x42 */
	{ 0,                0}, /* 0x43 */
	{ 0,                0}, /* 0x44 */
	{ 0,                0}, /* 0x45 */
	{ 0,                0}, /* 0x46 */
	{ 0,                0}, /* 0x47 */
	{ 0,                0}, /* 0x48 */
	{ insn_fiadd_sub,   DEC_DECODED}, /* 0x49, [p]fiadd.{ss,dd} */
	{ 0,                0}, /* 0x4A */
	{ 0,                0}, /* 0x4B */
	{ 0,                0}, /* 0x4C */
	{ insn_fiadd_sub,   DEC_DECODED}, /* 0x4D, [p]fisub.{ss,dd} */
	{ 0,                0}, /* 0x4E */
	{ 0,                0}, /* 0x4F */
	{ insn_faddp,       DEC_DECODED}, /* 0x50, [p]faddp */
	{ insn_faddz,       DEC_DECODED}, /* 0x51, [p]faddz */
	{ 0,                0}, /* 0x52 */
	{ 0,                0}, /* 0x53 */
	{ 0,                0}, /* 0x54 */
	{ 0,                0}, /* 0x55 */
	{ 0,                0}, /* 0x56 */
	{ insn_fzchk,       DEC_DECODED}, /* 0x57, [p]fzchkl */
	{ 0,                0}, /* 0x58 */
	{ 0,                0}, /* 0x59 */
	{ insn_form,        DEC_DECODED}, /* 0x5A, [p]form.dd */
	{ 0,                0}, /* 0x5B */
	{ 0,                0}, /* 0x5C */
	{ 0,                0}, /* 0x5D */
	{ 0,                0}, /* 0x5E */
	{ insn_fzchk,       DEC_DECODED}, /* 0x5F, [p]fzchks */
	{ 0,                0}, /* 0x60 */
	{ 0,                0}, /* 0x61 */
	{ 0,                0}, /* 0x62 */
	{ 0,                0}, /* 0x63 */
	{ 0,                0}, /* 0x64 */
	{ 0,                0}, /* 0x65 */
	{ 0,                0}, /* 0x66 */
	{ 0,                0}, /* 0x67 */
	{ 0,                0}, /* 0x68 */
	{ 0,                0}, /* 0x69 */
	{ 0,                0}, /* 0x6A */
	{ 0,                0}, /* 0x6B */
	{ 0,                0}, /* 0x6C */
	{ 0,                0}, /* 0x6D */
	{ 0,                0}, /* 0x6E */
	{ 0,                0}, /* 0x6F */
	{ 0,                0}, /* 0x70 */
	{ 0,                0}, /* 0x71 */
	{ 0,                0}, /* 0x72 */
	{ 0,                0}, /* 0x73 */
	{ 0,                0}, /* 0x74 */
	{ 0,                0}, /* 0x75 */
	{ 0,                0}, /* 0x76 */
	{ 0,                0}, /* 0x77 */
	{ 0,                0}, /* 0x78 */
	{ 0,                0}, /* 0x79 */
	{ 0,                0}, /* 0x7A */
	{ 0,                0}, /* 0x7B */
	{ 0,                0}, /* 0x7C */
	{ 0,                0}, /* 0x7D */
	{ 0,                0}, /* 0x7E */
	{ 0,                0}, /* 0x7F */
};


/*
 * Main decoder driver.
 *  insn = instruction at the current PC to execute.
 *  non_shadow = This insn is not in the shadow of a delayed branch).
 */
static void decode_exec (i860s *cpustate, UINT32 insn, UINT32 non_shadow)
{
	int upper_6bits = (insn >> 26) & 0x3f;
	char flags = 0;
	int unrecognized = 1;

	if (cpustate->exiting_ifetch)
		return;

	if ((upper_6bits == 0x12 || upper_6bits == 0x2c) && insn & 0x0200)
		logerror("D-bit seen.\n");
	if (GET_EPSR_BE ())
		logerror("BE-bit high.\n");
	if (GET_DIRBASE_CS8 ())
		logerror("CS8-bit high.\n");

	flags = decode_tbl[upper_6bits].flags;
	if (flags & DEC_DECODED)
	{
		decode_tbl[upper_6bits].insn_exec (cpustate, insn);
		unrecognized = 0;
	}
	else if (flags & DEC_MORE)
	{
		if (upper_6bits == 0x12)
		{
			/* FP instruction format handled here.  */
			char fp_flags = fp_decode_tbl[insn & 0x7f].flags;
			if (fp_flags & DEC_DECODED)
			{
				fp_decode_tbl[insn & 0x7f].insn_exec (cpustate, insn);
				unrecognized = 0;
			}
		}
		else if (upper_6bits == 0x13)
		{
			/* Core escape instruction format handled here.  */
			char esc_flags = core_esc_decode_tbl[insn & 0x3].flags;
			if (esc_flags & DEC_DECODED)
			{
				core_esc_decode_tbl[insn & 0x3].insn_exec (cpustate, insn);
				unrecognized = 0;
			}
		}
	}

	if (unrecognized)
		unrecog_opcode (cpustate->pc, insn);

	/* For now, just treat every instruction as taking the same number of
	   clocks-- a major oversimplification.  */
	cpustate->icount -= 9;
}


/* Set-up all the default power-on/reset values.  */
void reset_i860 (i860s *cpustate)
{
	int i;
	/* On power-up/reset, i860 has values:
	     PC = 0xffffff00.
	     Integer registers: r0 = 0, others = undefined.
	     FP registers:      f0:f1 = 0, others undefined.
	     psr: U = IM = BR = BW = 0; others = undefined.
	     epsr: IL = WP = PBM = BE = 0; processor type, stepping, and
	           DCS are proper and read-only; others = undefined.
	     db: undefined.
	     dirbase: DPS, BL, ATE = 0
	     fir, fsr, KR, KI, MERGE: undefined. (what about T?)

	     I$: flushed.
	     D$: undefined (all modified bits = 0).
	     TLB: flushed.

	   Note that any undefined values are set to 0x55aa55aa patterns to
	   try to detect defective i860 software.  */

	/* PC is at trap address after reset.  */
	cpustate->pc = 0xffffff00;

	/* Set grs and frs to undefined/nonsense values, except r0.  */
	for (i = 0; i < 32; i++)
	{
		set_iregval (i, 0x55aa55aa);
		set_fregval_s (cpustate, i, 0.0);
	}
	set_iregval (0, 0);
	set_fregval_s (cpustate, 0, 0.0);
	set_fregval_s (cpustate, 1, 0.0);

	/* Set whole psr to 0.  This sets the proper bits to 0 as specified
	   above, and zeroes the undefined bits.  */
	cpustate->cregs[CR_PSR] = 0;

	/* Set most of the epsr bits to 0 (as specified above), leaving
	   undefined as zero as well.  Then properly set processor type,
	   step, and DCS. Type = EPSR[7..0], step = EPSR[12..8],
	   DCS = EPSR[21..18] (2^[12+dcs] = cache size).
	   We'll pretend to be stepping D0, since it has the fewest bugs
	   (and I don't want to emulate the many defects in the earlier
	   steppings).
	   Proc type: 1 = XR, 2 = XP   (XR has 8KB data cache -> DCS = 1).
	   Steppings (XR): 3,4,5,6,7 = (B2, C0, B3, C1, D0 respectively).
	   Steppings (XP): 0, 2, 3, 4 = (A0, B0, B1, B2) (any others?).  */
	cpustate->cregs[CR_EPSR] = 0x00040701;

	/* Set DPS, BL, ATE = 0 and the undefined parts also to 0.  */
	cpustate->cregs[CR_DIRBASE] = 0x00000000;

	/* Set fir, fsr, KR, KI, MERGE, T to undefined.  */
	cpustate->cregs[CR_FIR] = 0xaa55aa55;
	cpustate->cregs[CR_FSR] = /* 0xaa55aa55; */ 0;
	cpustate->KR.d = 0.0;
	cpustate->KI.d = 0.0;
	cpustate->T.d = 0.0;
	cpustate->merge = 0xaa55aa55;

	cpustate->fir_gets_trap_addr = 0;
}




/*=================================================================*/
/* MAME execution hook for i860 emulator. */
/*=================================================================*/

#include "emu.h"

static CPU_EXECUTE( i860 )
{
	i860_state_t *cpustate = get_safe_token(device);

	/* Check if the data bus is held by another device, and bail if so.
	   Also check for reset.  */
	if (cpustate->pin_reset)
		reset_i860 (cpustate);
	if (cpustate->pin_bus_hold)
	{
		cpustate->icount = 0;
		return;
	}

	cpustate->exiting_readmem = 0;
	cpustate->exiting_ifetch = 0;

	/* Decode and execute loop.  */
	while (cpustate->icount > 0)
	{
		UINT32 savepc = cpustate->pc;
		cpustate->pc_updated = 0;
		cpustate->pending_trap = 0;

#if 1 /* Delete me soon, for debugging VC inter-processor synch.  */
		if (cpustate->pc == 0xfffc0370 ||
			cpustate->pc == 0xfffc03a4)
		{
			fprintf(stderr, "(%s) 0x%08x: snag 0x20000000\n", cpustate->device->tag(), cpustate->pc);
			cpustate->single_stepping = 0;
		}
		else if (cpustate->pc == 0xfffc0384 ||
					cpustate->pc == 0xfffc03b8)
		{
			fprintf(stderr, "(%s) 0x%08x: passed 0x20000000\n", cpustate->device->tag(), cpustate->pc);
			cpustate->single_stepping = 0;
		}
#endif

		savepc = cpustate->pc;
		debugger_instruction_hook(cpustate->device, cpustate->pc);
		decode_exec (cpustate, ifetch (cpustate, cpustate->pc), 1);

		cpustate->exiting_ifetch = 0;
		cpustate->exiting_readmem = 0;

		if (cpustate->pending_trap)
		{
			/* If we need to trap, change PC to trap address.
			   Also set supervisor mode, copy U and IM to their
			   previous versions, clear IM.  */
			if ((cpustate->pending_trap & TRAP_WAS_EXTERNAL) || (GET_EPSR_INT () && GET_PSR_IN ()))
			{
				if (!cpustate->pc_updated)
					cpustate->cregs[CR_FIR] = savepc + 4;
				else
					cpustate->cregs[CR_FIR] = cpustate->pc;
			}
			else if (cpustate->pending_trap & TRAP_IN_DELAY_SLOT)
			{
				cpustate->cregs[CR_FIR] = savepc + 4;
			}
			else
				cpustate->cregs[CR_FIR] = savepc;

			cpustate->fir_gets_trap_addr = 1;
			SET_PSR_PU (GET_PSR_U ());
			SET_PSR_PIM (GET_PSR_IM ());
			SET_PSR_U (0);
			SET_PSR_IM (0);
			SET_PSR_DIM (0);
			SET_PSR_DS (0);
			cpustate->pc = 0xffffff00;
			cpustate->pending_trap = 0;
		}
		else if (!cpustate->pc_updated)
		{
			/* If the PC wasn't updated by a control flow instruction, just
			   bump to next sequential instruction.  */
			cpustate->pc += 4;
		}

		/*if (cpustate->single_stepping)
		    debugger (cpustate); */
	}
}
/*=================================================================*/




#if 0
/*=================================================================*/
/* Internal debugger-related stuff.  */

extern unsigned disasm_i860 (char *buf, unsigned int pc, unsigned int insn);


/* Disassemble `len' instructions starting at `addr'.  */
static void disasm (i860s *cpustate, UINT32 addr, int len)
{
	UINT32 insn;
	int j;
	for (j = 0; j < len; j++)
	{
		char buf[256];
		UINT32 phys_addr = addr;
		if (GET_DIRBASE_ATE ())
			phys_addr = get_address_translation (cpustate, addr, 1  /* is_dataref */, 0 /* is_write */);

		/* Note that we print the incoming (possibly virtual) address as the
		   PC rather than the translated address.  */
		fprintf (stderr, "  (%s) 0x%08x: ", cpustate->device->tag(), addr);
		insn = cpustate->program->read_dword(phys_addr);
#ifdef HOST_MSB
		BYTE_REV32 (insn);
#endif /* HOST_MSB.  */
		disasm_i860 (buf, addr, insn); fprintf (stderr, "%s", buf);
		fprintf (stderr, "\n");
		addr += 4;
#if 1
		if (cpustate->single_stepping == 1 && has_delay_slot (insn))
			len += 1;
#endif
	}
}


/* Dump `len' bytes starting at `addr'.  */
static void dbg_db (i860s *cpustate, UINT32 addr, int len)
{
	UINT8 b[16];
	int i;
	/* This will always dump a multiple of 16 bytes, even if 'len' isn't.  */
	while (len > 0)
	{
		/* Note that we print the incoming (possibly virtual) address
		   rather than the translated address.  */
		fprintf (stderr, "0x%08x: ", addr);
		for (i = 0; i < 16; i++)
		{
			UINT32 phys_addr = addr;
			if (GET_DIRBASE_ATE ())
				phys_addr = get_address_translation (cpustate, addr, 1  /* is_dataref */, 0 /* is_write */);

			b[i] = cpustate->program->read_byte(phys_addr);
			fprintf (stderr, "%02x ", b[i]);
			addr++;
		}
		fprintf (stderr, "| ");
		for (i = 0; i < 16; i++)
		{
			if (isprint (b[i]))
				fprintf (stderr, "%c", b[i]);
			else
				fprintf (stderr, ".");
		}
		fprintf (stderr, "\n");
		len -= 16;
	}
}


/* A simple internal debugger.  */
void debugger (i860s *cpustate)
{
	char buf[256];
	UINT32 curr_disasm = cpustate->pc;
	UINT32 curr_dumpdb = 0;
	int c = 0;

	if (cpustate->single_stepping > 1 && cpustate->single_stepping != cpustate->pc)
		return;

	buf[0] = 0;

	/* Always disassemble the upcoming instruction when single-stepping.  */
	if (cpustate->single_stepping)
	{
		disasm (cpustate, cpustate->pc, 1);
		if (has_delay_slot (2))
			disasm (cpustate, cpustate->pc + 4, 1);
	}
	else
		fprintf (stderr, "\nEmulator: internal debugger started (? for help).\n");

	fflush (stdin);

	cpustate->single_stepping = 0;
	while (!cpustate->single_stepping)
	{
		fprintf (stderr, "- ");
#if 0  /* Doesn't work on MacOSX BSD flavor.  */
		fscanf (stdin, "%s", buf);
#else
		while (1)
		{
			char it = 0;
			if (read(STDIN_FILENO, &it, 1) == 1)
			{
				if (it == '\n')
				{
					buf[c] = 0;
					c = 0;
					break;
				}
				buf[c++] = it;
			}
		}
#endif
		if (buf[0] == 'g')
		{
			if (buf[1] == '0')
				sscanf (buf + 1, "%x", &cpustate->single_stepping);
			else
				break;
			buf[1] = 0;
			fprintf (stderr, "go until pc = 0x%08x.\n",
						cpustate->single_stepping);
			cpustate->single_stepping = 0;    /* HACK */
		}
		else if (buf[0] == 'r')
			dump_state (cpustate);
		else if (buf[0] == 'u')
		{
			if (buf[1] == '0')
				sscanf (buf + 1, "%x", &curr_disasm);
			disasm (cpustate, curr_disasm, 10);
			curr_disasm += 10 * 4;
			buf[1] = 0;
		}
		else if (buf[0] == 'p')
		{
			if (buf[1] >= '0' && buf[1] <= '4')
				dump_pipe (cpustate, buf[1] - 0x30);
			buf[1] = 0;
		}
		else if (buf[0] == 's')
			cpustate->single_stepping = 1;
		else if (buf[0] == 'l')
			; //cpustate->pc = elf_load(buf + 1);
		else if (buf[0] == 'd' && buf[1] == 'b')
		{
			if (buf[2] == '0')
				sscanf (buf + 2, "%x", &curr_dumpdb);
			dbg_db (cpustate, curr_dumpdb, 32);
			curr_dumpdb += 32;
		}
		else if (buf[0] == 'x' && buf[1] == '0')
		{
			UINT32 v;
			sscanf (buf + 1, "%x", &v);
			if (GET_DIRBASE_ATE ())
				fprintf (stderr, "vma 0x%08x ==> phys 0x%08x\n", v,
							get_address_translation (cpustate, v, 1, 0));
			else
				fprintf (stderr, "not in virtual address mode.\n");
		}
		else if (buf[0] == 'B')
		{
			;//cpustate->pc = elf_load("bins/bsd");
			break;
		}
		else if (buf[0] == '?')
		{
			fprintf (stderr, "  db: dump bytes (db[0xaddress])\n   r: dump registers\n   s: single-step\n   g: go back to emulator (g[0xaddress])\n   u: disassemble (u[0xaddress])\n   p: dump pipelines (p{0-4} for all, add, mul, load, graphics)\n   l: load an ELF binary (lpath)\n   x: give virt->phys translation (x{0xaddress})\n");
		}
		else
			fprintf (stderr, "Bad command '%s'.\n", buf);
	}

	/* Less noise when single-stepping.  */
	if (cpustate->single_stepping != 1)
		fprintf (stderr, "Debugger done, continuing emulation.\n");
}

#endif
