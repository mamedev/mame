// license:BSD-3-Clause
// copyright-holders:Carl
#include "i286.h"
#include "debugger.h"
#include "i86inline.h"

/*
 * Descriptor format
 * Data Segment
 * 48. . . . . . .40. . . . . . .32. . . . . . .24. . . . . . .16. . . . . . . 8. . . . . . . 0
 *  P|DPL|1|0|E|W|A|  BASE 23-16  |         BASE 15-0           |           LIMIT             |
 * Code Segment
 * 48. . . . . . .40. . . . . . .32. . . . . . .24. . . . . . .16. . . . . . . 8. . . . . . . 0
 *  P|DPL|1|1|C|R|A|  BASE 23-16  |         BASE 15-0           |           LIMIT             |
 * System
 * 48. . . . . . .40. . . . . . .32. . . . . . .24. . . . . . .16. . . . . . . 8. . . . . . . 0
 *  P|DPL|0| TYPE  |  BASE 23-16  |         BASE 15-0           |           LIMIT             |
 *
 *  P    == Present
 *  A    == Accessed
 *  E    == Expand Down
 *  W    == Read/Write
 *  C    == Conforming
 *  R    == Readable
 *  DPL  == Descriptor Privilege Level
 *
 *  Bits 48-64 are ignored but reserved for the 80386
 */

#define LIMIT(desc)  (desc[0]&0xffff)
#define BASE(desc)   ((desc[1]&0xffff)|((desc[2]&0xff)<<16))
#define RIGHTS(desc) ((desc[2]>>8)&0xff)

#define ACCESS(r)  (r&1)
#define SET_ACC(desc) (desc[2]|=0x100)
#define RW(r)      ((r>>1)&1)
#define READ(r)    ((r>>1)&1)
#define CONF(r)    ((r>>2)&1)
#define EXPDOWN(r) ((r>>2)&1)
#define CODE(r)    ((r>>3)&1)
#define SEGDESC(r) ((r>>4)&1)
#define DPL(r)     ((r>>5)&3)
#define PRES(r)    ((r>>7)&1)
#define GATE(r)    (r&31)
#define GATESEL(desc) (desc[1])
#define GATEOFF(desc) (desc[0])
#define GATECNT(desc) (desc[2]&31)

#define RPL(s)     (s&3)
#define IDX(s)     (s&(~7))
#define IDXTBL(s)  (s&(~3))
#define TBL(s)     (s&4)

#define TSSDESCIDLE 1
#define LDTDESC    2
#define TSSDESCBUSY 3
#define CALLGATE   4
#define TASKGATE   5
#define INTGATE    6
#define TRAPGATE   7

#define TSS_BACK 0
#define TSS_SP0  1
#define TSS_SS0  2
#define TSS_SP1  3
#define TSS_SS1  4
#define TSS_SP2  5
#define TSS_SS2  6
#define TSS_IP   7
#define TSS_FLAG 8
#define TSS_AX   9
#define TSS_CX   10
#define TSS_DX   11
#define TSS_BX   12
#define TSS_SP   13
#define TSS_BP   14
#define TSS_SI   15
#define TSS_DI   16
#define TSS_ES   17
#define TSS_CS   18
#define TSS_SS   19
#define TSS_DS   20
#define TSS_LDT  21

#define PMAX(a,b) ((a<b)?b:a)
#define CPL DPL(m_rights[SS])
#define PM (m_msw&1)

#define NT_IRET 0
#define NT_JMP  1
#define NT_CALL 2

/* these come from the 80286 timings in OPCODE.LST */
/* many of these numbers are suspect TODO: add protmode insns*/
const UINT8 i80286_cpu_device::m_i80286_timing[] =
{
	23,17,          /* exception, IRET */
		0, 2, 3, 1, /* INTs */
		2,              /* segment overrides */
		2, 2, 2,        /* flag operations */
		3, 3,16,14, /* arithmetic adjusts */
		3, 3,           /* decimal adjusts */
		2, 2,           /* sign extension */
		2, 7, 3, 3, 3, 5,   /* misc */

		7, 7,11,        /* direct JMPs */
		7,11,26,        /* indirect JMPs */
		7,13,           /* direct CALLs */
		7,11,29,        /* indirect CALLs */
	11,15,11,15,    /* returns */
		3, 7, 4, 8, /* conditional JMPs */
		4, 8, 4, 8, /* loops */

		5, 5, 5, 5, /* port reads */
		3, 3, 3, 3, /* port writes */

		2, 3, 3,        /* move, 8-bit */
		2, 3,           /* move, 8-bit immediate */
		2, 3, 3,        /* move, 16-bit */
		2, 3,           /* move, 16-bit immediate */
		5, 5, 3, 3, /* move, AL/AX memory */
		2, 5, 2, 3, /* move, segment registers */
		3, 5,           /* exchange, 8-bit */
		3, 5, 3,        /* exchange, 16-bit */

		5, 5, 3, 3, /* pushes */
		5, 5, 5, 5, /* pops */

		2, 7, 7,        /* ALU ops, 8-bit */
		3, 7, 7,        /* ALU ops, 8-bit immediate */
		2, 7, 7,        /* ALU ops, 16-bit */
		3, 7, 7,        /* ALU ops, 16-bit immediate */
		3, 7, 7,        /* ALU ops, 16-bit w/8-bit immediate */
	13,21,16,24,    /* MUL */
	13,21,16,24,    /* IMUL */
	14,22,17,25,    /* DIV */
	17,25,20,28,    /* IDIV */
		2, 2, 7, 7, /* INC/DEC */
		2, 2, 7, 7, /* NEG/NOT */

		2, 5, 1,        /* reg shift/rotate */
		7, 8, 1,        /* m8 shift/rotate */
		7, 8, 1,        /* m16 shift/rotate */

	13, 5,12,       /* CMPS 8-bit */
	13, 5,12,       /* CMPS 16-bit */
		9, 5, 8,        /* SCAS 8-bit */
		9, 5, 8,        /* SCAS 16-bit */
		5, 5, 4,        /* LODS 8-bit */
		5, 5, 4,        /* LODS 16-bit */
		4, 4, 3,        /* STOS 8-bit */
		4, 4, 3,        /* STOS 16-bit */
		5, 5, 4,        /* MOVS 8-bit */
		5, 5, 4,        /* MOVS 16-bit */

		5, 5, 4,        /* (80186) INS 8-bit */
		5, 5, 4,        /* (80186) INS 16-bit */
		5, 5, 4,        /* (80186) OUTS 8-bit */
		5, 5, 4,        /* (80186) OUTS 16-bit */
		3,17,19,        /* (80186) PUSH immediate, PUSHA/POPA */
	21,24,          /* (80186) IMUL immediate 8-bit */
	21,24,          /* (80186) IMUL immediate 16-bit */
	11,15,12, 4, 5, /* (80186) ENTER/LEAVE */
	13,             /* (80186) BOUND */
};

const device_type I80286 = &device_creator<i80286_cpu_device>;

i80286_cpu_device::i80286_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i8086_common_cpu_device(mconfig, I80286, "I80286", tag, owner, clock, "i80286", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 16, 0)
	, m_out_shutdown_func(*this)
{
	memcpy(m_timing, m_i80286_timing, sizeof(m_i80286_timing));
	m_amask = 0xffffff;
	m_fetch_xor = BYTE_XOR_LE(0);
	memset(m_sregs, 0x00, sizeof(m_sregs));
	m_sregs[CS] = 0xf000;
	memset(m_base, 0x00, sizeof(m_base));
	m_base[CS] = 0xff0000;
	memset(m_limit, 0x00, sizeof(m_limit));
	m_limit[CS] = m_limit[SS] = m_limit[DS] = m_limit[ES] = 0xffff;
	memset(m_rights, 0x00, sizeof(m_rights));
	m_rights[DS] = m_rights[SS] = m_rights[ES] = 0x93;
	memset(&m_gdtr, 0x00, sizeof(m_gdtr));
	memset(&m_idtr, 0x00, sizeof(m_idtr));
	m_idtr.limit = 0x3ff;
	memset(&m_ldtr, 0x00, sizeof(m_ldtr));
	memset(&m_tr, 0x00, sizeof(m_tr));
	m_msw = 0xfff0;
}

void i80286_cpu_device::device_reset()
{
	i8086_common_cpu_device::device_reset();
	m_MF = 0;
	m_NT = 0;
	m_IOPL = 0;
	m_msw = 0xfff0;
	m_limit[CS] = m_limit[SS] = m_limit[DS] = m_limit[ES] = 0xffff;
	m_sregs[DS] = m_sregs[SS] = m_sregs[ES] = 0;
	m_base[DS] = m_base[SS] = m_base[ES] = 0;
	m_rights[DS] = m_rights[SS] = m_rights[ES] = 0x93;
	m_rights[CS] = 0x9b;
	m_valid[CS] = m_valid[SS] = m_valid[DS] = m_valid[ES] = true;
	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;
	m_gdtr.base = m_ldtr.base = m_tr.base = 0;
	m_gdtr.limit = m_ldtr.limit = m_tr.limit = 0;
	m_ldtr.rights = m_tr.rights = 0;
	m_ldtr.sel = m_tr.sel = 0;
	m_sregs[CS] = 0xf000;
	m_base[CS] = 0xff0000;
	m_ip = 0xfff0;
	m_trap_level = 0;
	m_shutdown = false;
	m_out_shutdown_func(false);
}

void i80286_cpu_device::device_start()
{
	i8086_common_cpu_device::device_start();
	save_item(NAME(m_trap_level));
	save_item(NAME(m_msw));
	save_item(NAME(m_base));
	save_item(NAME(m_limit));
	save_item(NAME(m_rights));
	save_item(NAME(m_valid));
	save_item(NAME(m_gdtr.base));
	save_item(NAME(m_gdtr.limit));
	save_item(NAME(m_idtr.base));
	save_item(NAME(m_idtr.limit));
	save_item(NAME(m_ldtr.sel));
	save_item(NAME(m_ldtr.base));
	save_item(NAME(m_ldtr.limit));
	save_item(NAME(m_ldtr.rights));
	save_item(NAME(m_tr.sel));
	save_item(NAME(m_tr.base));
	save_item(NAME(m_tr.limit));
	save_item(NAME(m_tr.rights));
	save_item(NAME(m_amask));
	save_item(NAME(m_shutdown));

	state_add( I286_ES, "ES", m_sregs[ES] ).callimport().callexport().formatstr("%04X");
	state_add( I286_ES_BASE, "ESBASE", m_base[ES]).callimport().callexport().formatstr("%06X");
	state_add( I286_ES_LIMIT, "ESLIMIT", m_limit[ES]).callimport().callexport().formatstr("%04X");
	state_add( I286_ES_FLAGS, "ESFLAGS", m_rights[ES]).callimport().callexport().formatstr("%02X");
	state_add( I286_CS, "CS", m_sregs[CS] ).callimport().callexport().formatstr("%04X");
	state_add( I286_CS_BASE, "CSBASE", m_base[CS]).callimport().callexport().formatstr("%06X");
	state_add( I286_CS_LIMIT, "CSLIMIT", m_limit[CS]).callimport().callexport().formatstr("%04X");
	state_add( I286_CS_FLAGS, "CSFLAGS", m_rights[CS]).callimport().callexport().formatstr("%02X");
	state_add( I286_SS, "SS", m_sregs[SS] ).callimport().callexport().formatstr("%04X");
	state_add( I286_SS_BASE, "SSBASE", m_base[SS]).callimport().callexport().formatstr("%06X");
	state_add( I286_SS_LIMIT, "SSLIMIT", m_limit[SS]).callimport().callexport().formatstr("%04X");
	state_add( I286_SS_FLAGS, "SSFLAGS", m_rights[SS]).callimport().callexport().formatstr("%02X");
	state_add( I286_DS, "DS", m_sregs[DS] ).callimport().callexport().formatstr("%04X");
	state_add( I286_DS_BASE, "DSBASE", m_base[DS]).callimport().callexport().formatstr("%06X");
	state_add( I286_DS_LIMIT, "DSLIMIT", m_limit[DS]).callimport().callexport().formatstr("%04X");
	state_add( I286_DS_FLAGS, "DSFLAGS", m_rights[DS]).callimport().callexport().formatstr("%02X");
	state_add( I286_GDTR_BASE, "GDTRBASE", m_gdtr.base).callimport().callexport().formatstr("%06X");
	state_add( I286_GDTR_LIMIT, "GDTRLIMIT", m_gdtr.limit).callimport().callexport().formatstr("%04X");
	state_add( I286_IDTR_BASE, "IDTRBASE", m_idtr.base).callimport().callexport().formatstr("%06X");
	state_add( I286_IDTR_LIMIT, "IDTRLIMIT", m_idtr.limit).callimport().callexport().formatstr("%04X");
	state_add( I286_LDTR, "LDTR", m_ldtr.sel ).callimport().callexport().formatstr("%04X");
	state_add( I286_LDTR_BASE, "LDTRBASE", m_ldtr.base).callimport().callexport().formatstr("%06X");
	state_add( I286_LDTR_LIMIT, "LDTRLIMIT", m_ldtr.limit).callimport().callexport().formatstr("%04X");
	state_add( I286_LDTR_FLAGS, "LDTRFLAGS", m_ldtr.rights).callimport().callexport().formatstr("%02X");
	state_add( I286_TR, "TR", m_tr.sel ).callimport().callexport().formatstr("%04X");
	state_add( I286_TR_BASE, "TRBASE", m_tr.base).callimport().callexport().formatstr("%06X");
	state_add( I286_TR_LIMIT, "TRLIMIT", m_tr.limit).callimport().callexport().formatstr("%04X");
	state_add( I286_TR_FLAGS, "TRFLAGS", m_tr.rights).callimport().callexport().formatstr("%02X");
	state_add( I286_MSW, "MSW", m_msw ).callimport().callexport().formatstr("%04X");
	state_add( I286_VECTOR, "V", m_int_vector).callimport().callexport().formatstr("%02X");

	state_add(STATE_GENPC, "curpc", m_pc).callimport().callexport().formatstr("%06X");
	m_out_shutdown_func.resolve_safe();
}

void i80286_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENPC:
			strprintf(str, "%08X", m_base[CS] + m_ip);
			break;

		case STATE_GENFLAGS:
			{
				UINT16 flags = CompressFlags();
				strprintf(str, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
					flags & 0x8000 ? '0':'.',
					flags & 0x4000 ? 'N':'.',
					flags & 0x2000 ? 'I':'.',
					flags & 0x1000 ? 'I':'.',
					flags & 0x0800 ? 'O':'.',
					flags & 0x0400 ? 'D':'.',
					flags & 0x0200 ? 'I':'.',
					flags & 0x0100 ? 'T':'.',
					flags & 0x0080 ? 'S':'.',
					flags & 0x0040 ? 'Z':'.',
					flags & 0x0020 ? '0':'.',
					flags & 0x0010 ? 'A':'.',
					flags & 0x0008 ? '0':'.',
					flags & 0x0004 ? 'P':'.',
					flags & 0x0002 ? '1':'.',
					flags & 0x0001 ? 'C':'.');
			}
			break;
	}
}

bool i80286_cpu_device::memory_translate(address_spacenum spacenum, int intention, offs_t &address)
{
	if(spacenum == AS_PROGRAM)
		address &= m_amask;

	return TRUE;
}

void i80286_cpu_device::execute_set_input(int inptnum, int state)
{
	if(inptnum == INPUT_LINE_NMI)
	{
		if(m_nmi_state == state)
		{
			return;
		}
		m_nmi_state = state;
		if(state != CLEAR_LINE)
		{
			m_pending_irq |= NMI_IRQ;
		}
	}
	else if(inptnum == INPUT_LINE_A20)
		m_amask = m_a20_callback.isnull() ? 0xffffff : m_a20_callback(state);
	else
	{
		m_irq_state = state;
		if(state == CLEAR_LINE)
		{
			m_pending_irq &= ~INT_IRQ;
		}
		else
		{
			m_pending_irq |= INT_IRQ;
		}
	}
}
// when a cpu reset happens on a AT the bios checks for 9 in byte 0xf
// of the nvram.  if yes, after init, it sets the stack pointer to the value in 0040:0067
// in the bios data segment then pops es and ds off that stack, does popa then a far ret.

void i80286_cpu_device::trap(UINT32 error)
{
	int error_code = error & 0xffff;
	UINT16 number = error >> 16;
	if(error_code == 0xffff)
		error_code = -1;
	m_ip = m_prev_ip;
	try
	{
		switch(number)
		{
			case FAULT_DE:
			case FAULT_TS:
			case FAULT_NP:
			case FAULT_SS:
			case FAULT_GP:
				m_trap_level++;
				if(m_trap_level == 2)
					throw TRAP(FAULT_DF,0);
				if(m_trap_level == 3)
					break;
				interrupt_descriptor(number,1,error_code);
				break;
			case FAULT_DF:
				interrupt_descriptor(number,1,0);
				break;
			default:
				interrupt_descriptor(number,1,-1);
		}
	}
	catch(UINT32 e)
	{
		trap(e);
	}
	if(m_trap_level == 3)
	{
		m_shutdown = true;
		m_out_shutdown_func(true);
	}
	m_trap_level = 0;
}

UINT32 i80286_cpu_device::selector_address(UINT16 sel)
{
	UINT32 base;
	UINT16 limit;
	if(TBL(sel))
	{
		base = m_ldtr.base;
		limit = m_ldtr.limit;
	}
	else
	{
		base = m_gdtr.base;
		limit = m_gdtr.limit;
	}
	return ((IDX(sel) >= limit) || !IDXTBL(sel) ? -1 : base + IDX(sel));
}

int i80286_cpu_device::verify(UINT16 selector, int operation, UINT8 rights, bool valid)
{
	if(!IDXTBL(selector) && !valid)
		return FAULT_GP;
	if(!SEGDESC(rights))
		return FAULT_GP;

	switch (operation)
	{
		case I8086_READ:
			if(CODE(rights) && !READ(rights))
				return FAULT_GP;
			break;
		case I8086_WRITE:
			if(CODE(rights) || !RW(rights))
				return FAULT_GP;
			break;
		case I8086_FETCH:
			if(!CODE(rights))
				return FAULT_GP;
			break;
	}
	return 0;
}

void i80286_cpu_device::pop_seg(int reg)
{
	UINT16 sel;
	if(PM)
		check_permission(SS, m_regs.w[SP], 2, I8086_READ);
	sel = read_word(m_base[SS] + m_regs.w[SP]);
	data_descriptor(reg, sel);
	m_regs.w[SP] += 2;
}

void i80286_cpu_device::data_descriptor(int reg, UINT16 selector, int cpl, UINT32 trap, UINT16 offset, int size)
{
	if(PM)
	{
		UINT16 desc[3];
		UINT8 r;
		UINT32 addr;
		if((reg != SS) && !IDXTBL(selector))
		{
			m_sregs[reg] = 0;
			m_limit[reg] = 0;
			m_base[reg] = 0;
			m_rights[reg] = 0;
			m_valid[reg] = 0;
			return;
		}

		if((addr = selector_address(selector)) == -1)
			throw trap;

		desc[0] = read_word(addr);
		desc[1] = read_word(addr + 2);
		desc[2] = read_word(addr + 4);
		r = RIGHTS(desc);

		if(!SEGDESC(r))
			throw trap;

		if(reg == SS)
		{
			if(!IDXTBL(selector))
				throw trap;
			if(DPL(r) != cpl)
				throw trap;
			if(RPL(selector) != cpl)
				throw trap;
			if(!RW(r) || CODE(r))
				throw trap;
			if(!PRES(r))
				throw TRAP(FAULT_SS, (IDXTBL(selector) + (trap & 1)));
		}
		else
		{
			if((DPL(r) < PMAX(cpl, RPL(selector))) && (!CODE(r) || (CODE(r) && !CONF(r))))
				throw trap;
			if(CODE(r) && !READ(r))
				throw trap;
			if(!PRES(r))
				throw TRAP(FAULT_NP, (IDXTBL(selector) + (trap & 1)));
		}

		if(offset + size)
		{
			if((CODE(r) || !EXPDOWN(r)) && ((offset + size - 1) > LIMIT(desc)))
				throw (reg==SS) ? TRAP(FAULT_SS, (trap & 1)) : trap;

			if(!CODE(r) && EXPDOWN(r) && ((offset <= LIMIT(desc)) || ((offset + size - 1) > 0xffff)))
				throw (reg==SS) ? TRAP(FAULT_SS, (trap & 1)) : trap;
		}

		SET_ACC(desc);
		write_word(addr + 4, desc[2]);
		m_sregs[reg] = selector;
		m_limit[reg] = LIMIT(desc);
		m_base[reg] = BASE(desc);
		m_rights[reg] = RIGHTS(desc);
	}
	else
	{
		m_sregs[reg] = selector;
		m_base[reg] = selector << 4;
	}
	m_valid[reg] = 1;
}

void i80286_cpu_device::data_descriptor(int reg, UINT16 selector)
{
	data_descriptor(reg, selector, CPL, TRAP(FAULT_GP,IDXTBL(selector)));
}

void i80286_cpu_device::switch_task(UINT16 ntask, int type)
{
	UINT16 ndesc[3], desc[3], ntss[22], otss[22], flags;
	UINT8 r, lr;
	UINT32 naddr, oaddr, ldtaddr;
	int i;
	logerror("i286: %06x This program uses TSSs, how rare. Please report this to the developers.\n", pc());

	if(TBL(ntask))
		throw TRAP(FAULT_TS, IDXTBL(ntask));

	if((naddr = selector_address(ntask)) == -1)
		throw TRAP(FAULT_TS, IDXTBL(ntask));

	oaddr = selector_address(m_tr.sel);
	ndesc[0] = read_word(naddr);
	ndesc[1] = read_word(naddr + 2);
	ndesc[2] = read_word(naddr + 4);
	desc[2] = read_word(oaddr + 4);
	r = RIGHTS(ndesc);

	if(SEGDESC(r) || ((GATE(r) & ~2) != TSSDESCIDLE))
		throw TRAP(FAULT_GP, IDXTBL(ntask));

	if(!PRES(r))
		throw TRAP(FAULT_NP, IDXTBL(ntask));

	if(LIMIT(ndesc) < 43)
		throw TRAP(FAULT_TS, IDXTBL(ntask));

	flags = CompressFlags();

	if(type == NT_CALL)
		write_word(BASE(ndesc) + TSS_BACK * 2, m_tr.sel);

	if(type == NT_IRET)
		flags &= ~0x4000;

	otss[TSS_IP] = m_ip;
	otss[TSS_FLAG] = flags;
	otss[TSS_AX] = m_regs.w[AX];
	otss[TSS_CX] = m_regs.w[CX];
	otss[TSS_DX] = m_regs.w[DX];
	otss[TSS_BX] = m_regs.w[BX];
	otss[TSS_SP] = m_regs.w[SP];
	otss[TSS_BP] = m_regs.w[BP];
	otss[TSS_SI] = m_regs.w[SI];
	otss[TSS_DI] = m_regs.w[DI];
	otss[TSS_ES] = m_sregs[ES];
	otss[TSS_CS] = m_sregs[CS];
	otss[TSS_SS] = m_sregs[SS];
	otss[TSS_DS] = m_sregs[DS];

	for (i = 14; i < 42; i += 2)
		write_word(m_tr.base + i, otss[i / 2]);

	for (i = 0; i < 44; i += 2)
		ntss[i / 2] = read_word(BASE(ndesc) + i);

	// jmp does both
	if(type != NT_CALL)
	{
		desc[2] &= ~0x200; // mark idle
		write_word(oaddr + 4, desc[2]);
	}

	if(type != NT_IRET)
	{
		ndesc[2] |= 0x200;
		write_word(naddr + 4, ndesc[2]);
	}

	m_tr.sel = ntask;
	m_tr.limit = LIMIT(ndesc);
	m_tr.base = BASE(ndesc);
	m_tr.rights = RIGHTS(ndesc);

	load_flags(ntss[TSS_FLAG], 0);
	m_regs.w[AX] = ntss[TSS_AX];
	m_regs.w[CX] = ntss[TSS_CX];
	m_regs.w[DX] = ntss[TSS_DX];
	m_regs.w[BX] = ntss[TSS_BX];
	m_regs.w[SP] = ntss[TSS_SP];
	m_regs.w[BP] = ntss[TSS_BP];
	m_regs.w[SI] = ntss[TSS_SI];
	m_regs.w[DI] = ntss[TSS_DI];

	if(TBL(ntss[TSS_LDT]))
		throw TRAP(FAULT_TS, IDXTBL(ntss[TSS_LDT]));

	if(IDXTBL(ntss[TSS_LDT]))
	{
		if((ldtaddr = selector_address(ntss[TSS_LDT])) == -1)
			throw TRAP(FAULT_TS, IDXTBL(ntss[TSS_LDT]));

		desc[0] = read_word(ldtaddr);
		desc[1] = read_word(ldtaddr + 2);
		desc[2] = read_word(ldtaddr + 4);
		lr = RIGHTS(desc);

		if(SEGDESC(lr) || (GATE(lr) != LDTDESC))
			throw TRAP(FAULT_TS, IDXTBL(ntss[TSS_LDT]));

		if(!PRES(lr))
			throw TRAP(FAULT_TS, IDXTBL(ntss[TSS_LDT]));

		m_ldtr.sel = ntss[TSS_LDT];
		m_ldtr.limit = LIMIT(desc);
		m_ldtr.base = BASE(desc);
		m_ldtr.rights = RIGHTS(desc);
	}
	else
	{
		m_ldtr.sel = 0;
		m_ldtr.limit = 0;
		m_ldtr.base = 0;
		m_ldtr.rights = 0;
	}

	if(type == NT_CALL)
		m_NT = 1;

	m_msw |= 8;
	// Docs explicitly say SS is loaded first.  Why?  Because the DPL
	// of the TSS is compared to the DPL of SS which is CPL
	data_descriptor(SS, ntss[TSS_SS], RPL(ntss[TSS_CS]), TRAP(FAULT_TS, IDXTBL(ntss[TSS_SS])));

	try
	{
		code_descriptor(ntss[TSS_CS], ntss[TSS_IP], 0);
	}
	catch (UINT32 e)
	{
		int error_code = e & 0xffff;
		if(error_code == FAULT_GP)
			e = TRAP(FAULT_TS, (e >> 16)); // #NP fault is correct
		throw e;
	}

	data_descriptor(ES, ntss[TSS_ES], CPL, TRAP(FAULT_TS, IDXTBL(ntss[TSS_ES])));
	data_descriptor(DS, ntss[TSS_DS], CPL, TRAP(FAULT_TS, IDXTBL(ntss[TSS_DS])));
}

void i80286_cpu_device::code_descriptor(UINT16 selector, UINT16 offset, int gate)
{
	if(PM)
	{
		UINT16 desc[3];
		UINT8 r;
		UINT32 addr;
		if((addr = selector_address(selector)) == -1)
			throw TRAP(FAULT_GP, IDXTBL(selector));

		desc[0] = read_word(addr);
		desc[1] = read_word(addr + 2);
		desc[2] = read_word(addr + 4);
		r = RIGHTS(desc);

		if(SEGDESC(r))
		{
			if(!CODE(r))
				throw TRAP(FAULT_GP, IDXTBL(selector));

			if(CONF(r))
			{
				if(DPL(r) > CPL)
					throw TRAP(FAULT_GP, IDXTBL(selector));
			}
			else
				if((RPL(selector) > CPL) || (DPL(r) != CPL))
					throw TRAP(FAULT_GP, IDXTBL(selector));

			if(!PRES(r))
				throw TRAP(FAULT_NP, IDXTBL(selector));  // this order is important

			if(offset > LIMIT(desc))
				throw TRAP(FAULT_GP, 0);

			SET_ACC(desc);
			write_word(addr + 4, desc[2]);
			m_sregs[CS] = IDXTBL(selector) | CPL;
			m_limit[CS] = LIMIT(desc);
			m_base[CS] = BASE(desc);
			m_rights[CS] = RIGHTS(desc);
			m_ip = offset;
		}
		else
		{ // systemdescriptor
			UINT16 gatesel = GATESEL(desc);

			if(!gate)
				throw TRAP(FAULT_GP, IDXTBL(selector)); // tss cs must be segment
			if(DPL(r) < PMAX(CPL,RPL(selector)))
				throw TRAP(FAULT_GP, IDXTBL(selector));
			if(!PRES(r))
				throw TRAP(FAULT_NP, IDXTBL(selector));

			switch (GATE(r))
			{
				case CALLGATE:
				{
					UINT16 gatedesc[3];
					if((addr = selector_address(gatesel)) == -1)
						throw TRAP(FAULT_GP, IDXTBL(gatesel));

					gatedesc[0] = read_word(addr);
					gatedesc[1] = read_word(addr + 2);
					gatedesc[2] = read_word(addr + 4);
					r = RIGHTS(gatedesc);

					if(!CODE(r) || !SEGDESC(r))
						throw TRAP(FAULT_GP, IDXTBL(gatesel));
					if(DPL(r) > CPL)
						throw TRAP(FAULT_GP, IDXTBL(gatesel));
					if(!PRES(r))
						throw TRAP(FAULT_NP, IDXTBL(gatesel));

					if(GATEOFF(desc) > LIMIT(gatedesc))
						throw TRAP(FAULT_GP,0);

					if (!CONF(r) && (DPL(r) < CPL))
					{
						// inner call
						UINT16 tss_ss, tss_sp, oldss, oldsp;
						UINT32 oldstk;
						int i;
						if(gate == NT_JMP)
							throw TRAP(FAULT_GP, IDXTBL(gatesel)); // can't jmp to inner

						tss_ss = read_word(m_tr.base+TSS_SS0*2+(DPL(r)*4));
						tss_sp = read_word(m_tr.base+TSS_SP0*2+(DPL(r)*4));

						oldss = m_sregs[SS];
						oldsp = m_regs.w[SP];
						oldstk = m_base[SS] + oldsp;
						data_descriptor(SS, tss_ss, DPL(r), TRAP(FAULT_TS,IDXTBL(tss_ss)), tss_sp-8-(GATECNT(desc)*2), 8+(GATECNT(desc)*2));
						m_regs.w[SP] = tss_sp;
						PUSH(oldss);
						PUSH(oldsp);
						for(i = GATECNT(desc)-1; i >= 0; i--)
							PUSH(read_word(oldstk+(i*2)));
					}
					else
						check_permission(SS, m_regs.w[SP]-4, 4, I8086_READ);

					SET_ACC(gatedesc);
					write_word(addr+4, gatedesc[2]);
					m_sregs[CS]=IDXTBL(gatesel) | DPL(r);
					m_limit[CS]=LIMIT(gatedesc);
					m_base[CS]=BASE(gatedesc);
					m_rights[CS]=RIGHTS(gatedesc);
					m_ip=GATEOFF(desc);
					break;
				}

				case TASKGATE:
					selector = gatesel;
					if((addr = selector_address(selector)) == -1)
						throw TRAP(FAULT_GP, IDXTBL(selector));

					desc[2] = read_word(addr+4);
					r = RIGHTS(desc);
					if (SEGDESC(r) || (GATE(r) != TSSDESCIDLE))
						throw TRAP(FAULT_GP,IDXTBL(selector));

				case TSSDESCIDLE:
					switch_task(selector, gate);
					load_flags(CompressFlags(), CPL);
					break;

				default:
					throw TRAP(FAULT_GP,IDXTBL(selector));
			}
		}
	}
	else
	{
		m_ip = offset;
		m_sregs[CS]=selector;
		m_base[CS]=selector<<4;
		m_rights[CS]=0x9b;
		m_limit[CS]=0xffff;
	}
}

void i80286_cpu_device::interrupt_descriptor(int number, int hwint, int error)
{
	UINT16 desc[3], gatesel, flags = CompressFlags();
	UINT8 r;
	hwint = hwint ? 1 : 0;

	if(number == -1)
	{
		number = standard_irq_callback(0);

		m_irq_state = CLEAR_LINE;
		m_pending_irq &= ~INT_IRQ;
		hwint = 1;
	}

	if(!PM)
	{
		PUSH(flags & ~0xf000);
		m_TF = m_IF = 0;

		UINT16 dest_off = read_word(number * 4 + 0);
		UINT16 dest_seg = read_word(number * 4 + 2);

		PUSH(m_sregs[CS]);
		PUSH(m_ip);
		code_descriptor(dest_seg, dest_off, 0);
		return;
	}

	if((number << 3) >= m_idtr.limit)
		throw TRAP(FAULT_GP, (number * 8 + 2 + hwint));

	desc[0] = read_word(m_idtr.base + (number << 3));
	desc[1] = read_word(m_idtr.base + (number << 3) + 2);
	desc[2] = read_word(m_idtr.base + (number << 3) + 4);

	r = RIGHTS(desc);
	if(!hwint && (DPL(r) < CPL))
		throw TRAP(FAULT_GP, (number * 8 + 2 + hwint));
	if(!PRES(r))
		throw TRAP(FAULT_NP, (number * 8 + 2 + hwint));
	gatesel = GATESEL(desc);

	switch (GATE(r))
	{
		case TASKGATE:
			try
			{
				switch_task(gatesel, NT_CALL);
			}
			catch (UINT32 e)
			{
				throw e + hwint;
			}
			if((hwint == 1) && (error != -1))
				PUSH(error);
			load_flags(CompressFlags(), CPL); // new flags
			break;

		case INTGATE:
		case TRAPGATE:
		{
			UINT16 gatedesc[3];
			UINT32 addr;

			if((addr = selector_address(gatesel)) == -1)
				throw TRAP(FAULT_GP, (IDXTBL(gatesel) + hwint));

			gatedesc[0] = read_word(addr);
			gatedesc[1] = read_word(addr + 2);
			gatedesc[2] = read_word(addr + 4);
			r = RIGHTS(gatedesc);
			if(!CODE(r) || !SEGDESC(r))
				throw TRAP(FAULT_GP, (IDXTBL(gatesel) + hwint));
			if(DPL(r) > CPL)
				throw TRAP(FAULT_GP, (IDXTBL(gatesel) + hwint));
			if(!PRES(r))
				throw TRAP(FAULT_NP, (IDXTBL(gatesel) + hwint));
			if(GATEOFF(desc) > LIMIT(gatedesc))
				throw TRAP(FAULT_GP, hwint);

			if(!CONF(r) && (DPL(r) < CPL))
			{
				// inner call
				UINT16 tss_ss, tss_sp, oldss, oldsp;
				tss_ss = read_word(m_tr.base + TSS_SS0 * 2 + (DPL(r) * 4));
				tss_sp = read_word(m_tr.base + TSS_SP0 * 2 + (DPL(r) * 4));

				oldss = m_sregs[SS];
				oldsp = m_regs.w[SP];
				data_descriptor(SS, tss_ss, DPL(r), TRAP(FAULT_TS, (IDXTBL(tss_ss) + hwint)), tss_sp - ((error != -1) ? 12 : 10), (error != -1) ? 12 : 10);
				m_regs.w[SP] = tss_sp;
				PUSH(oldss);
				PUSH(oldsp);
			}
			else
				check_permission(SS, m_regs.w[SP] - ((error != -1) ? 8 : 6), (error != -1) ? 8 : 6, I8086_READ);

			SET_ACC(gatedesc);
			write_word(addr + 4, gatedesc[2]);
			PUSH(flags);
			PUSH(m_sregs[CS]);
			PUSH(m_ip);
			if((hwint == 1) && (error != -1))
				PUSH(error);
			m_sregs[CS] = IDXTBL(gatesel) | DPL(r);
			m_limit[CS] = LIMIT(gatedesc);
			m_base[CS] = BASE(gatedesc);
			m_rights[CS] = RIGHTS(gatedesc);
			m_ip = GATEOFF(desc);
			m_TF = 0;
			m_NT = 0;
			if(GATE(RIGHTS(desc)) == INTGATE)
				m_IF = 0;
			break;
		}
		default:
			throw TRAP(FAULT_GP, (number * 8 + 2 + hwint));
	}
}

UINT8 i80286_cpu_device::read_port_byte(UINT16 port)
{
	if(PM && (CPL > m_IOPL))
		throw TRAP(FAULT_GP, 0);
	return m_io->read_byte(port);
}

UINT16 i80286_cpu_device::read_port_word(UINT16 port)
{
	if(PM && (CPL > m_IOPL))
		throw TRAP(FAULT_GP, 0);
	return m_io->read_word_unaligned(port);
}

void i80286_cpu_device::write_port_byte(UINT16 port, UINT8 data)
{
	if(PM && (CPL > m_IOPL))
		throw TRAP(FAULT_GP, 0);
	m_io->write_byte(port, data);
}

void i80286_cpu_device::write_port_word(UINT16 port, UINT16 data)
{
	if(PM && (CPL > m_IOPL))
		throw TRAP(FAULT_GP, 0);
	m_io->write_word_unaligned(port, data);
}

UINT8 i80286_cpu_device::fetch_op()
{
	UINT8 data;
	if(m_ip > m_limit[CS])
		throw TRAP(FAULT_GP, 0);

	data = m_direct->read_byte( pc() & m_amask, m_fetch_xor );
	m_ip++;
	return data;
}

UINT8 i80286_cpu_device::fetch()
{
	UINT8 data;
	if(m_ip > m_limit[CS])
		throw TRAP(FAULT_GP, 0);

	data = m_direct->read_byte( pc() & m_amask, m_fetch_xor );
	m_ip++;
	return data;
}

UINT32 i80286_cpu_device::calc_addr(int seg, UINT16 offset, int size, int op, bool override)
{
	seg = (m_seg_prefix && (seg==DS || seg==SS) && override) ? m_prefix_seg : seg;
	if(op != I8086_NONE)
		check_permission(seg, offset, size, op);

	return (m_base[seg] + offset) & (op != I8086_NONE ? m_amask : 0xffffff);
}

void i80286_cpu_device::execute_run()
{
	while(m_icount > 0 )
	{
		try
		{
			if ( m_seg_prefix_next )
			{
				m_seg_prefix = true;
				m_seg_prefix_next = false;
			}
			else
			{
				m_prev_ip = m_ip;
				m_seg_prefix = false;

				/* Dispatch IRQ */
				if ( m_pending_irq && m_no_interrupt == 0 )
				{
					if ( m_pending_irq & NMI_IRQ )
					{
						interrupt_descriptor(NMI, 1, -1);
						m_pending_irq &= ~NMI_IRQ;
						m_halt = false;
						m_shutdown = false;
						m_out_shutdown_func(false);
					}
					else if ( m_IF )
					{
						interrupt_descriptor(-1, 1, -1);
						m_halt = false;
					}
				}

				if(m_halt || m_shutdown)
				{
					m_icount = 0;
					return;
				}

				/* No interrupt allowed between last instruction and this one */
				if ( m_no_interrupt )
				{
					m_no_interrupt--;
				}

				/* trap should allow one instruction to be executed */
				if ( m_fire_trap )
				{
					if ( m_fire_trap >= 2 )
					{
						interrupt(1);
						m_fire_trap = 0;
					}
					else
					{
						m_fire_trap++;
					}
				}
			}

			debugger_instruction_hook( this, pc() & m_amask );

			UINT8 op = fetch_op();

			switch(op)
			{
				case 0x07: // i_pop_es
					pop_seg(ES);
					CLK(POP_SEG);
					break;

				case 0x0f:
				{
					unsigned next = fetch_op();
					UINT16 desc[3], tmp, msw, sel;
					UINT8 r;
					UINT32 addr;

					switch (next)
					{
						case 0:
							if(!PM)
								throw TRAP(FAULT_UD, (UINT16)-1);
							m_modrm = fetch();
							switch (m_modrm & 0x38)
							{
								case 0: /* sldt */
									PutRMWord(m_ldtr.sel);
									break;

								case 8: /* str */
									PutRMWord(m_tr.sel);
									break;

								case 0x10: /* lldt */
									if(CPL != 0)
										throw TRAP(FAULT_GP, 0);
									sel = GetRMWord();
									if(TBL(sel))
										throw TRAP(FAULT_GP, IDXTBL(sel));
									if(IDXTBL(sel))
									{
										if(IDX(sel) >= m_gdtr.limit)
											throw TRAP(FAULT_GP, IDXTBL(sel));
										addr = m_gdtr.base + IDX(sel);
										desc[0] = read_word(addr);
										desc[1] = read_word(addr + 2);
										desc[2] = read_word(addr + 4);
										r = RIGHTS(desc);
										if(SEGDESC(r) || (GATE(r) != LDTDESC))
											throw TRAP(FAULT_GP, IDXTBL(sel));
										if(!PRES(r))
											throw TRAP(FAULT_NP, IDXTBL(sel));
									}
									else
									{
										desc[0] = 0;
										desc[1] = 0;
										desc[2] = 0;
									}
									m_ldtr.sel = sel;
									m_ldtr.limit = LIMIT(desc);
									m_ldtr.base = BASE(desc);
									m_ldtr.rights = RIGHTS(desc);
									break;

								case 0x18: /* ltr */
									if(CPL != 0)
										throw TRAP(FAULT_GP, 0);
									sel = GetRMWord();
									if((addr = selector_address(sel)) == -1)
										throw TRAP(FAULT_GP, IDXTBL(sel));
									desc[0] = read_word(addr);
									desc[1] = read_word(addr + 2);
									desc[2] = read_word(addr + 4);
									r = RIGHTS(desc);
									if(SEGDESC(r) || (GATE(r) != TSSDESCIDLE))
										throw TRAP(FAULT_GP, IDXTBL(sel));
									if(!PRES(r))
										throw TRAP(FAULT_NP, IDXTBL(sel));
									desc[2] |= 0x200; // mark busy
									write_word(addr + 4, desc[2]);
									m_tr.sel = sel;
									m_tr.limit = LIMIT(desc);
									m_tr.base = BASE(desc);
									m_tr.rights = RIGHTS(desc);
									break;

								case 0x20: /* verr */
									tmp = GetRMWord();
									if((addr = selector_address(tmp)) == -1)
										m_ZeroVal = 1;
									else
									{
										desc[2] = read_word(addr + 4);
										r = RIGHTS(desc);
										m_ZeroVal = verify(tmp, I8086_READ, RIGHTS(desc), 0);
										m_ZeroVal = m_ZeroVal || (CODE(r) && CONF(r) ? 0 : (DPL(r) < PMAX(RPL(tmp),CPL)));
									}
									break;

								case 0x28: /* verw */
									tmp = GetRMWord();
									if((addr = selector_address(tmp)) == -1)
										m_ZeroVal = 1;
									else
									{
										desc[2] = read_word(addr + 4);
										r = RIGHTS(desc);
										m_ZeroVal = verify(tmp, I8086_WRITE, RIGHTS(desc), 0);
										m_ZeroVal = m_ZeroVal || (DPL(r) < PMAX(RPL(tmp),CPL));
									}
									break;

								default:
									throw TRAP(FAULT_UD, (UINT16)-1);
							}
							break;
						case 1:
						{
							UINT32 ea;
							m_modrm = fetch();
							if((m_modrm >= 0xc0) && (m_modrm < 0xe0))
								throw TRAP(FAULT_UD, (UINT16)-1);
							switch (m_modrm & 0x38)
							{
								case 0: /* sgdt */
									ea = get_ea(6, I8086_WRITE);
									write_word(ea, m_gdtr.limit);
									write_word(ea + 2, m_gdtr.base & 0xffff);
									write_word(ea + 4, 0xff00 | m_gdtr.base >> 16);
									break;
								case 8: /* sidt */
									ea = get_ea(6, I8086_WRITE);
									write_word(ea, m_idtr.limit);
									write_word(ea + 2, m_idtr.base & 0xffff);
									write_word(ea + 4, 0xff00 | m_idtr.base >> 16);
									break;
								case 0x10: /* lgdt */
									if(PM && (CPL != 0))
										throw TRAP(FAULT_GP, 0);
									ea = get_ea(6, I8086_READ);
									m_gdtr.limit = read_word(ea);
									m_gdtr.base = read_word(ea + 2) | (read_byte(ea + 4) << 16);
									break;
								case 0x18: /* lidt */
									if(PM && (CPL != 0))
										throw TRAP(FAULT_GP, 0);
									ea = get_ea(6, I8086_READ);
									m_idtr.limit = read_word(ea);
									m_idtr.base = read_word(ea + 2) | (read_byte(ea + 4) << 16);
									break;
								case 0x20: /* smsw */
									PutRMWord(m_msw);
									break;
								case 0x30: /* lmsw */
									if(PM && (CPL != 0))
										throw TRAP(FAULT_GP, 0);
									msw = GetRMWord();
									m_msw = (m_msw & 1) | msw;
									break;
								default:
									throw TRAP(FAULT_UD, (UINT16)-1);
							}
							break;
						}
						case 2: /* LAR */
							if(!PM)
								throw TRAP(FAULT_UD, (UINT16)-1);
							m_modrm = fetch_op();
							tmp = GetRMWord();
							if((addr = selector_address(tmp)) == -1)
								m_ZeroVal = 1;
							else
							{
								desc[2] = read_word(addr + 4);
								r = RIGHTS(desc);
								if(!SEGDESC(r) && ((GATE(r) > TRAPGATE) || !GATE(r)))
									m_ZeroVal = 1;
								else if(DPL(r) >= PMAX(RPL(tmp),CPL) || (SEGDESC(r) && CODE(r) && CONF(r)))
								{
									m_ZeroVal = 0;
									// rights are expected to be in upper byte
									RegWord(r << 8);
								}
								else
									m_ZeroVal = 1;
							}
							break;
						case 3: /* LSL */
							if(!PM)
								throw TRAP(FAULT_UD, (UINT16)-1);
							m_modrm = fetch_op();
							tmp = GetRMWord();
							if((addr = selector_address(tmp)) == -1)
								m_ZeroVal = 1;
							else
							{
								desc[2] = read_word(addr + 4);
								r = RIGHTS(desc);
								if(!SEGDESC(r) && ((GATE(r) >= CALLGATE) || !GATE(r)))
									m_ZeroVal = 1; // not valid for gates
								else if(DPL(r) >= PMAX(RPL(tmp),CPL) || (SEGDESC(r) && CODE(r) && CONF(r)))
								{
									m_ZeroVal = 0;
									RegWord(read_word(addr));
								}
								else
									m_ZeroVal = 1;
							}
							break;
						case 5: /* loadall */
							if(PM && (CPL != 0))
								throw TRAP(FAULT_GP, 0);
							m_msw = (m_msw & 1) | read_word(0x806);
							m_tr.sel = read_word(0x816);
							ExpandFlags(read_word(0x818));
							m_ip = read_word(0x81a);
							m_ldtr.sel = read_word(0x81c);
							m_sregs[DS] = read_word(0x81e);
							m_sregs[SS] = read_word(0x820);
							m_sregs[CS] = read_word(0x822);
							m_sregs[ES] = read_word(0x824);
							m_regs.w[DI] = read_word(0x826);
							m_regs.w[SI] = read_word(0x828);
							m_regs.w[BP] = read_word(0x82a);
							m_regs.w[SP] = read_word(0x82c);
							m_regs.w[BX] = read_word(0x82e);
							m_regs.w[DX] = read_word(0x830);
							m_regs.w[CX] = read_word(0x832);
							m_regs.w[AX] = read_word(0x834);
							// loadall uses base-rights-limit order
#define LOADDESC(addr, sreg) {  desc[1] = read_word(addr); desc[2] = read_word(addr+2); desc[0] = read_word(addr+4); \
m_base[sreg] = BASE(desc); m_rights[sreg] = RIGHTS(desc); \
m_limit[sreg] = LIMIT(desc); }
							LOADDESC(0x836, ES);
							LOADDESC(0x83C, CS);
							LOADDESC(0x842, SS);
							LOADDESC(0x848, DS);
#undef LOADDESC
							// void cast supresses warning
#define LOADDESC(addr, reg, r) { desc[1] = read_word(addr); desc[2] = read_word(addr + 2); desc[0] = read_word(addr + 4); \
reg.base = BASE(desc); (void)(r); reg.limit = LIMIT(desc); }
							LOADDESC(0x84e, m_gdtr, 1);
							LOADDESC(0x854, m_ldtr, m_ldtr.rights = RIGHTS(desc));
							LOADDESC(0x85a, m_idtr, 1);
							LOADDESC(0x860, m_tr, m_tr.rights = RIGHTS(desc));
#undef LOADDESC
							break;

						case 6: /* clts */
							if(PM && (CPL != 0))
								throw TRAP(FAULT_GP, 0);
							m_msw &= ~8;
							break;
						default:
							throw TRAP(FAULT_UD, (UINT16)-1);
					}
					break;
				}

				case 0x17: // i_pop_ss
					pop_seg(SS);
					CLK(POP_SEG);
					m_no_interrupt = 1;
					break;

				case 0x1f: // i_pop_ds
					pop_seg(DS);
					CLK(POP_SEG);
					break;

				case 0x54: // i_push_sp
					PUSH(m_regs.w[SP]);
					CLK(PUSH_R16);
					break;

				case 0x60: // i_pusha
				{
					check_permission(SS, m_regs.w[SP]-16, 16, I8086_WRITE);
					UINT32 tmp = m_regs.w[SP];
					PUSH(m_regs.w[AX]);
					PUSH(m_regs.w[CX]);
					PUSH(m_regs.w[DX]);
					PUSH(m_regs.w[BX]);
					PUSH(tmp);
					PUSH(m_regs.w[BP]);
					PUSH(m_regs.w[SI]);
					PUSH(m_regs.w[DI]);
					CLK(PUSHA);
				}
				break;

				case 0x61: // i_popa
					check_permission(SS, m_regs.w[SP], 16, I8086_READ);
					m_regs.w[DI] = POP();
					m_regs.w[SI] = POP();
					m_regs.w[BP] = POP();
					POP();
					m_regs.w[BX] = POP();
					m_regs.w[DX] = POP();
					m_regs.w[CX] = POP();
					m_regs.w[AX] = POP();
					CLK(POPA);
					break;

				case 0x62: // i_bound
				{
					UINT32 low,high,tmp;
					m_modrm = fetch();
					low = GetRMWord();
					high = GetnextRMWord();
					tmp = RegWord();
					if (tmp<low || tmp>high)
						interrupt(5);
					CLK(BOUND);
					logerror("%s: %06x: bound %04x high %04x low %04x tmp\n", tag(), pc(), high, low, tmp);
				}
				break;

				case 0x63: // arpl
				{
					UINT16 tmp, source;
					if (!PM) throw TRAP(FAULT_UD,(UINT16)-1);

					m_modrm=fetch_op();
					tmp=GetRMWord();
					source=RegWord();

					if (RPL(tmp)<RPL(source))
					{
						m_ZeroVal = 0;
						PutbackRMWord(IDXTBL(tmp)|RPL(source));
					}
					else
						m_ZeroVal = 1;
					break;
				}

				case 0x68: // i_push_d16
					PUSH( fetch_word() );
					CLK(PUSH_IMM);
					break;

				case 0x69: // i_imul_d16
				{
					UINT32 tmp;
					DEF_r16w();
					tmp = fetch_word();
					m_dst = (INT32)((INT16)m_src)*(INT32)((INT16)tmp);
					m_CarryVal = m_OverVal = (((INT32)m_dst) >> 15 != 0) && (((INT32)m_dst) >> 15 != -1);
					RegWord(m_dst);
					CLKM(IMUL_RRI16, IMUL_RMI16);
				}
				break;

				case 0x6a: // i_push_d8
					PUSH( (UINT16)((INT16)((INT8)fetch())) );
					CLK(PUSH_IMM);
					break;

				case 0x6b: // i_imul_d8
				{
					UINT32 src2;
					DEF_r16w();
					src2= (UINT16)((INT16)((INT8)fetch()));
					m_dst = (INT32)((INT16)m_src)*(INT32)((INT16)src2);
					m_CarryVal = m_OverVal = (((INT32)m_dst) >> 15 != 0) && (((INT32)m_dst) >> 15 != -1);
					RegWord(m_dst);
					CLKM(IMUL_RRI8, IMUL_RMI8);
				}
				break;

				case 0x6c: // i_insb
					i_insb();
					break;

				case 0x6d: // i_insw
					i_insw();
					break;

				case 0x6e: // i_outsb
					i_outsb();
					break;

				case 0x6f: // i_outsw
					i_outsw();
					break;

				case 0x8c: // i_mov_wsreg
					m_modrm = fetch();
					if((m_modrm & 0x38) > 0x18)
					{
						logerror("%s: %06x: Mov Sreg - Invalid register\n", tag(), pc());
						throw TRAP(FAULT_UD, (UINT16)-1);
					}
					PutRMWord(m_sregs[(m_modrm & 0x38) >> 3]);
					CLKM(MOV_RS,MOV_MS);
					break;

				case 0x8e: // i_mov_sregw
					m_modrm = fetch();
					m_src = GetRMWord();
					CLKM(MOV_SR,MOV_SM);
					switch (m_modrm & 0x38)
					{
						case 0x00:  /* mov es,ew */
							data_descriptor(ES, m_src);
							break;
						case 0x10:  /* mov ss,ew */
							data_descriptor(SS, m_src);
							m_no_interrupt = 1;
							break;
						case 0x18:  /* mov ds,ew */
							data_descriptor(DS, m_src);
							break;
						default:
							logerror("%s: %06x: Mov Sreg - Invalid register\n", tag(), pc());
							throw TRAP(FAULT_UD, (UINT16)-1);
					}
					break;

				case 0x8f: // i_popw
				{
					m_modrm = fetch();
					UINT16 tmp = read_word(calc_addr(SS, m_regs.w[SP], 2, I8086_READ, false));
					PutRMWord( tmp );
					m_regs.w[SP] += 2;
					CLKM(POP_R16,POP_M16);
					break;
				}

				case 0x9a: // i_call_far
				{
					UINT16 cs = m_sregs[CS];
					UINT16 tmp = fetch_word();
					UINT16 tmp2 = fetch_word();
					UINT16 ip = m_ip;
					code_descriptor(tmp2, tmp, NT_CALL);
					PUSH(cs);
					PUSH(ip);
					CLK(CALL_FAR);
				}
				break;

				case 0x9b: // i_wait
					if((m_msw & 0x0a) == 0x0a)
						throw TRAP(FAULT_NM, (UINT16)-1);
					CLK(WAIT);
					break;

				case 0x9c: // pushf
				{
					UINT16 flags = CompressFlags();
					if(!PM)
						flags &= ~0xf000;
					PUSH(flags);
					CLK(PUSHF);
					break;
				}

				case 0x9d: // popf
				{
					UINT16 flags;
					flags = POP();
					CLK(POPF);
					load_flags(flags, CPL);
					break;
				}

				case 0xc0: // i_rotshft_bd8
				{
					UINT8 c;
					m_modrm = fetch();
					m_src = GetRMByte();
					m_dst = m_src;
					c = fetch() & 0x1f;
					CLKM(ROT_REG_BASE,ROT_M8_BASE);
					m_icount -= m_timing[ROT_REG_BIT] * c;
					if (c)
					{
						switch ( m_modrm & 0x38 )
						{
							case 0x00: do { ROL_BYTE();  c--; } while (c>0); PutbackRMByte(m_dst); break;
							case 0x08: do { ROR_BYTE();  c--; } while (c>0); PutbackRMByte(m_dst); break;
							case 0x10: do { ROLC_BYTE(); c--; } while (c>0); PutbackRMByte(m_dst); break;
							case 0x18: do { RORC_BYTE(); c--; } while (c>0); PutbackRMByte(m_dst); break;
							case 0x30:
							case 0x20: SHL_BYTE(c); break;
							case 0x28: SHR_BYTE(c); break;
							case 0x38: SHRA_BYTE(c); break;
						}
					}
				}
				break;

				case 0xc1: // i_rotshft_wd8
				{
					UINT8 c;
					m_modrm = fetch();
					m_src = GetRMWord();
					m_dst = m_src;
					c = fetch() & 0x1f;
					CLKM(ROT_REG_BASE,ROT_M16_BASE);
					m_icount -= m_timing[ROT_REG_BIT] * c;
					if (c)
					{
						switch ( m_modrm & 0x38 )
						{
							case 0x00: do { ROL_WORD();  c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x08: do { ROR_WORD();  c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x10: do { ROLC_WORD(); c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x18: do { RORC_WORD(); c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x30:
							case 0x20: SHL_WORD(c); break;
							case 0x28: SHR_WORD(c); break;
							case 0x38: SHRA_WORD(c); break;
						}
					}
				}
				break;

				case 0xc4: // i_les_dw
				{
					m_modrm = fetch();
					if(m_modrm >= 0xc0)
						throw TRAP(FAULT_UD, (UINT16)-1);
					UINT16 tmp = GetRMWord();
					data_descriptor(ES, GetnextRMWord());
					RegWord(tmp);
					CLK(LOAD_PTR);
					break;
				}

				case 0xc5: // i_lds_dw
				{
					m_modrm = fetch();
					if(m_modrm >= 0xc0)
						throw TRAP(FAULT_UD, (UINT16)-1);
					UINT16 tmp = GetRMWord();
					data_descriptor(DS, GetnextRMWord());
					RegWord(tmp);
					CLK(LOAD_PTR);
					break;
				}

				case 0xc8: // i_enter
				{
					UINT16 nb = fetch();
					UINT32 level;

					nb |= fetch() << 8;
					level = fetch();
					CLK(!level ? ENTER0 : (level == 1) ? ENTER1 : ENTER_BASE);
					if(level > 1)
						m_icount -= level * m_timing[ENTER_COUNT];
					PUSH(m_regs.w[BP]);
					m_regs.w[BP] = m_regs.w[SP];
					m_regs.w[SP] -= nb;
					for (int i=1; i<level; i++)
					{
						PUSH( GetMemW(SS,m_regs.w[BP] - i*2) );
					}
					if (level)
					{
						PUSH(m_regs.w[BP]);
					}
				}
				break;

				case 0xc9: // i_leave
					m_regs.w[SP] = m_regs.w[BP];
					m_regs.w[BP] = POP();
					CLK(LEAVE);
					break;

				case 0xca: // ret far imm
				{
					unsigned count = fetch_word();
					far_return(0, count);
					CLK(RET_FAR_IMM);
					break;
				}
				case 0xcb: // ret far
					far_return(0, 0);
					CLK(RET_FAR);
					break;

				case 0xcf: // iret
				{
					int oldcpl = (PM) ? CPL : 0;
					UINT16 flags = far_return(1, 0);
					CLK(IRET);
					load_flags(flags, oldcpl);
					break;
				}

				case 0xd2: // i_rotshft_bcl
				{
					UINT8 c;

					m_modrm = fetch();
					m_src = GetRMByte();
					m_dst = m_src;
					c = m_regs.b[CL] & 0x1f;
					CLKM(ROT_REG_BASE,ROT_M16_BASE);
					m_icount -= m_timing[ROT_REG_BIT] * c;
					if (c)
					{
						switch ( m_modrm & 0x38 )
						{
							case 0x00: do { ROL_BYTE();  c--; } while (c>0); PutbackRMByte(m_dst); break;
							case 0x08: do { ROR_BYTE();  c--; } while (c>0); PutbackRMByte(m_dst); break;
							case 0x10: do { ROLC_BYTE(); c--; } while (c>0); PutbackRMByte(m_dst); break;
							case 0x18: do { RORC_BYTE(); c--; } while (c>0); PutbackRMByte(m_dst); break;
							case 0x30:
							case 0x20: SHL_BYTE(c); break;
							case 0x28: SHR_BYTE(c); break;
							case 0x38: SHRA_BYTE(c); break;
						}
					}
				}
				break;

				case 0xd3: // i_rotshft_wcl
				{
					UINT8 c;

					m_modrm = fetch();
					m_src = GetRMWord();
					m_dst = m_src;
					c = m_regs.b[CL] & 0x1f;
					CLKM(ROT_REG_BASE,ROT_M16_BASE);
					m_icount -= m_timing[ROT_REG_BIT] * c;
					if (c)
					{
						switch ( m_modrm & 0x38 )
						{
							case 0x00: do { ROL_WORD();  c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x08: do { ROR_WORD();  c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x10: do { ROLC_WORD(); c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x18: do { RORC_WORD(); c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x30:
							case 0x20: SHL_WORD(c); break;
							case 0x28: SHR_WORD(c); break;
							case 0x38: SHRA_WORD(c); break;
						}
					}
				}
				break;

				case 0xd8: // i_esc
				case 0xd9:
				case 0xda:
				case 0xdb:
				case 0xdc:
				case 0xdd:
				case 0xde:
				case 0xdf:
					if((m_msw & 8) || (m_msw & 4))
						throw TRAP(FAULT_NM, (UINT16)-1);
					m_modrm = fetch();
					GetRMByte();
					CLK(NOP);
					if((m_modrm == 0xe0) && (op == 0xdf))
						m_regs.w[AX] = 0xffff;  // FPU not present
					break;

				case 0xea: // i_jmp_far
				{
					UINT16 tmp = fetch_word();
					UINT16 tmp1 = fetch_word();
					code_descriptor(tmp1, tmp, NT_JMP);
					CLK(JMP_FAR);
					break;
				}

				case 0xf0: // i_lock
					if(PM && (CPL > m_IOPL))
						throw TRAP(FAULT_GP, 0);
					logerror("%s: %06x: Warning - BUSLOCK\n", tag(), pc());
					m_no_interrupt = 1;
					CLK(NOP);
					break;

				case 0xf4: // i_hlt
					if(PM && (CPL > m_IOPL))
						throw TRAP(FAULT_GP, 0);
					m_icount = 0;
					m_halt = true;
					break;

				case 0xfa: // i_cli
					if(PM && (CPL > m_IOPL))
						throw TRAP(FAULT_GP, 0);
					m_IF = 0;
					CLK(FLAG_OPS);
					break;

				case 0xfb: // i_sti
					if(PM && (CPL > m_IOPL))
						throw TRAP(FAULT_GP, 0);
					m_IF = 1;
					CLK(FLAG_OPS);
					break;

				case 0xff: // i_ffpre
					{
						UINT32 tmp, tmp1;
						m_modrm = fetch();
						tmp = GetRMWord();
						switch ( m_modrm & 0x38 )
						{
						case 0x00:  /* INC */
							tmp1 = tmp+1;
							m_OverVal = (tmp==0x7fff);
							set_AF(tmp1,tmp,1);
							set_SZPF_Word(tmp1);
							PutbackRMWord(tmp1);
							CLKM(INCDEC_R16,INCDEC_M16);
							break;
						case 0x08:  /* DEC */
							tmp1 = tmp-1;
							m_OverVal = (tmp==0x8000);
							set_AF(tmp1,tmp,1);
							set_SZPF_Word(tmp1);
							PutbackRMWord(tmp1);
							CLKM(INCDEC_R16,INCDEC_M16);
							break;
						case 0x10:  /* CALL */
							PUSH(m_ip);
							m_ip = tmp;
							CLKM(CALL_R16,CALL_M16);
							break;
						case 0x18:  /* CALL FAR */
						{
							UINT16 ip = m_ip;
							tmp1 = m_sregs[CS];
							code_descriptor(GetnextRMWord(), tmp, NT_CALL);
							PUSH(tmp1);
							PUSH(ip);
							CLK(CALL_M32);
							break;
						}
						case 0x20:  /* JMP */
							m_ip = tmp;
							CLKM(JMP_R16,JMP_M16);
							break;
						case 0x28:  /* JMP FAR */
							code_descriptor(GetnextRMWord(), tmp, NT_JMP);
							CLK(JMP_M32);
							break;
						case 0x30:
							PUSH(tmp);
							CLKM(PUSH_R16,PUSH_M16);
							break;
						default:
							logerror("%s: %06x: FF Pre with unimplemented mod\n", tag(), pc());
							throw TRAP(FAULT_UD,(UINT16)-1);
						}
					}
					break;

				case 0xf2: // i_repne
				case 0xf3:
				{
					bool pass = false;
					UINT8 next = repx_op();
					UINT16 c = m_regs.w[CX];

					switch (next)
					{
						case 0x6c:  CLK(OVERRIDE); if (c) do { i_insb();  c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
						case 0x6d:  CLK(OVERRIDE); if (c) do { i_insw();  c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
						case 0x6e:  CLK(OVERRIDE); if (c) do { i_outsb(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
						case 0x6f:  CLK(OVERRIDE); if (c) do { i_outsw(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
						default:
							// Decrement IP and pass on
							m_ip -= 1 + (m_seg_prefix_next ? 1 : 0);
							pass = true;
					}
					if(!pass)
					{
						if(c)
							m_ip = m_prev_ip;
						break;
					}
				}

				default:
					if(!common_op(op))
					{
						m_icount -= 10; // UD fault timing?
						logerror("%s: %06x: Invalid Opcode %02x\n", tag(), pc(), op);
						m_ip = m_prev_ip;
						throw TRAP(FAULT_UD, (UINT16)-1);
					}
					break;
			}
		}
		catch(UINT32 e)
		{
			trap(e);
		}
	}
}


void i80286_cpu_device::load_flags(UINT16 flags, int cpl)
{
	UINT16 oldflags = CompressFlags();
	flags &= ~0x8000;
	if(PM && cpl)
	{
		UINT16 mask = 0x3000;
		if(cpl > m_IOPL)
			mask |= 0x200;
		flags &= ~mask;
		flags |= (oldflags & mask);
	}
	else if(!PM)
		(flags &= ~0xf000);
	ExpandFlags(flags);

	if(m_TF)
		m_fire_trap = 1;
}

UINT16 i80286_cpu_device::far_return(int iret, int bytes)
{
	UINT16 sel, off, flags = 0;
	int spaddr;

	if(PM && m_NT && iret)
	{
		switch_task(read_word(m_tr.base + TSS_BACK * 2), NT_IRET);
		return CompressFlags();
	}

	// must be restartable
	if(PM)
		check_permission(SS, m_regs.w[SP], (iret ? 6 : 4), I8086_READ);

	spaddr = (m_base[SS] + m_regs.w[SP]) & m_amask;
	off = read_word(spaddr);
	sel = read_word(spaddr + 2);
	if(iret)
		flags = read_word(spaddr + 4);

	if(PM)
	{
		UINT16 desc[3], newsp, newss;
		int addr, r;

		if((addr = selector_address(sel)) == -1)
			throw TRAP(FAULT_GP, IDXTBL(sel));

		if(RPL(sel) < CPL)
			throw TRAP(FAULT_GP, IDXTBL(sel));
		desc[0] = read_word(addr);
		desc[1] = read_word(addr + 2);
		desc[2] = read_word(addr + 4);
		r = RIGHTS(desc);

		if(!CODE(r) || !SEGDESC(r))
			throw TRAP(FAULT_GP, IDXTBL(sel));
		if(CONF(r))
		{
			if(DPL(r) > RPL(sel))
				throw TRAP(FAULT_GP, IDXTBL(sel));
		}
		else if(DPL(r) != RPL(sel))
			throw TRAP(FAULT_GP, IDXTBL(sel));

		if(!PRES(r))
			throw TRAP(FAULT_NP, IDXTBL(sel));
		if(off > LIMIT(desc))
			throw TRAP(FAULT_GP, 0);
		if(CPL < RPL(sel))
		{
			check_permission(SS, m_regs.w[SP] + (iret ? 6 : 4) + bytes, 4, I8086_READ);
			newsp = read_word(spaddr + ((iret ? 6 : 4) + bytes));
			newss = read_word(spaddr + ((iret ? 8 : 6) + bytes));
			data_descriptor(SS, newss, RPL(sel), TRAP(FAULT_GP, IDXTBL(newss)));
			m_regs.w[SP] = newsp + bytes;
		}
		else
			m_regs.w[SP] += (iret ? 6 : 4) + bytes;
		SET_ACC(desc);
		write_word(addr + 4, desc[2]);
		m_sregs[CS] = sel;
		m_limit[CS] = LIMIT(desc);
		m_base[CS] = BASE(desc);
		m_rights[CS] = RIGHTS(desc);
		m_ip = off;

		// docs say check rpl but windows doesn't like it
		r = m_rights[DS];
		if(verify(m_sregs[DS], I8086_READ, r, 0) || (CODE(r) && CONF(r) ? 0 : (DPL(r) < CPL)))
			data_descriptor(DS, 0);
		r = m_rights[ES];
		if(verify(m_sregs[ES], I8086_READ, r, 0) || (CODE(r) && CONF(r) ? 0 : (DPL(r) < CPL)))
			data_descriptor(ES, 0);
	}
	else
	{
		m_regs.w[SP] += (iret ? 6 : 4) + bytes;
		m_sregs[CS] = sel;
		m_base[CS] = sel << 4;
		m_rights[CS] = 0x9b;
		m_limit[CS] = 0xffff;
		m_ip = off;
	}

	return flags;
}

void i80286_cpu_device::check_permission(UINT8 check_seg, UINT32 offset, UINT16 size, int operation)
{
	int trap;
	UINT8 rights;
	if(PM)
	{
		rights = m_rights[check_seg];
		trap = verify(m_sregs[check_seg], operation, rights, m_valid[check_seg]);
		if((CODE(rights) || !EXPDOWN(rights)) && ((offset+size-1) > m_limit[check_seg]))
			trap = FAULT_GP;
		if(!CODE(rights) && EXPDOWN(rights) && ((offset <= m_limit[check_seg]) || ((offset + size - 1) > 0xffff)))
			trap = FAULT_GP;

		if((trap == FAULT_GP) && (check_seg == SS))
			trap = FAULT_SS;
		if(trap)
			throw TRAP(trap, 0);
	}
}
