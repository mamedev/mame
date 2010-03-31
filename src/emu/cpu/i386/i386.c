/*
    Intel 386 emulator

    Written by Ville Linde

    Currently supports:
        Intel 386
        Intel 486
        Intel Pentium
        Cyrix MediaGX
*/

#include "emu.h"
#include "debugger.h"
#include "i386priv.h"
#include "i386.h"

#include "debug/debugcpu.h"

/* seems to be defined on mingw-gcc */
#undef i386

int i386_parity_table[256];
MODRM_TABLE i386_MODRM_table[256];

/*************************************************************************/

#define INT_DEBUG	1

static void i386_load_protected_mode_segment(i386_state *cpustate, I386_SREG *seg )
{
	UINT32 v1,v2;
	UINT32 base, limit;
	int entry;

	if ( seg->selector & 0x4 )
	{
		base = cpustate->ldtr.base;
		limit = cpustate->ldtr.limit;
	} else {
		base = cpustate->gdtr.base;
		limit = cpustate->gdtr.limit;
	}

	if (limit == 0 || seg->selector + 7 > limit)
		return;
	entry = seg->selector & ~0x7;

	v1 = READ32(cpustate, base + entry );
	v2 = READ32(cpustate, base + entry + 4 );

	seg->flags = (v2 >> 8) & 0xf0ff;
	seg->base = (v2 & 0xff000000) | ((v2 & 0xff) << 16) | ((v1 >> 16) & 0xffff);
	seg->limit = (v2 & 0xf0000) | (v1 & 0xffff);
	if (seg->flags & 0x8000)
		seg->limit = (seg->limit << 12) | 0xfff;
	seg->d = (seg->flags & 0x4000) ? 1 : 0;
}

static void i386_load_segment_descriptor(i386_state *cpustate, int segment )
{
	if (PROTECTED_MODE)
	{
		if (!V8086_MODE)
			i386_load_protected_mode_segment(cpustate, &cpustate->sreg[segment] );
		else
		{
			cpustate->sreg[segment].base = cpustate->sreg[segment].selector << 4;
			cpustate->sreg[segment].limit = 0xffff;
			cpustate->sreg[segment].flags = (segment == CS) ? 0x009a : 0x0092;
		}
	}
	else
	{
		cpustate->sreg[segment].base = cpustate->sreg[segment].selector << 4;

		if( segment == CS && !cpustate->performed_intersegment_jump )
			cpustate->sreg[segment].base |= 0xfff00000;
	}
}

static UINT32 get_flags(i386_state *cpustate)
{
	UINT32 f = 0x2;
	f |= cpustate->CF;
	f |= cpustate->PF << 2;
	f |= cpustate->AF << 4;
	f |= cpustate->ZF << 6;
	f |= cpustate->SF << 7;
	f |= cpustate->TF << 8;
	f |= cpustate->IF << 9;
	f |= cpustate->DF << 10;
	f |= cpustate->OF << 11;
	f |= cpustate->IOP1 << 12;
	f |= cpustate->IOP2 << 13;
	f |= cpustate->NT << 14;
	return (cpustate->eflags & cpustate->eflags_mask) | (f & 0xffff);
}

static void set_flags(i386_state *cpustate, UINT32 f )
{
	cpustate->CF = (f & 0x1) ? 1 : 0;
	cpustate->PF = (f & 0x4) ? 1 : 0;
	cpustate->AF = (f & 0x10) ? 1 : 0;
	cpustate->ZF = (f & 0x40) ? 1 : 0;
	cpustate->SF = (f & 0x80) ? 1 : 0;
	cpustate->TF = (f & 0x100) ? 1 : 0;
	cpustate->IF = (f & 0x200) ? 1 : 0;
	cpustate->DF = (f & 0x400) ? 1 : 0;
	cpustate->OF = (f & 0x800) ? 1 : 0;
	cpustate->IOP1 = (f & 0x1000) ? 1 : 0;
	cpustate->IOP2 = (f & 0x2000) ? 1 : 0;
	cpustate->NT = (f & 0x4000) ? 1 : 0;
	cpustate->eflags = f & cpustate->eflags_mask;
}

static void sib_byte(i386_state *cpustate,UINT8 mod, UINT32* out_ea, UINT8* out_segment)
{
	UINT32 ea = 0;
	UINT8 segment = 0;
	UINT8 scale, i, base;
	UINT8 sib = FETCH(cpustate);
	scale = (sib >> 6) & 0x3;
	i = (sib >> 3) & 0x7;
	base = sib & 0x7;

	switch( base )
	{
		case 0: ea = REG32(EAX); segment = DS; break;
		case 1: ea = REG32(ECX); segment = DS; break;
		case 2: ea = REG32(EDX); segment = DS; break;
		case 3: ea = REG32(EBX); segment = DS; break;
		case 4: ea = REG32(ESP); segment = SS; break;
		case 5:
			if( mod == 0 ) {
				ea = FETCH32(cpustate);
				segment = DS;
			} else if( mod == 1 ) {
				ea = REG32(EBP);
				segment = SS;
			} else if( mod == 2 ) {
				ea = REG32(EBP);
				segment = SS;
			}
			break;
		case 6: ea = REG32(ESI); segment = DS; break;
		case 7: ea = REG32(EDI); segment = DS; break;
	}
	switch( i )
	{
		case 0: ea += REG32(EAX) * (1 << scale); break;
		case 1: ea += REG32(ECX) * (1 << scale); break;
		case 2: ea += REG32(EDX) * (1 << scale); break;
		case 3: ea += REG32(EBX) * (1 << scale); break;
		case 4: break;
		case 5: ea += REG32(EBP) * (1 << scale); break;
		case 6: ea += REG32(ESI) * (1 << scale); break;
		case 7: ea += REG32(EDI) * (1 << scale); break;
	}
	*out_ea = ea;
	*out_segment = segment;
}

static void modrm_to_EA(i386_state *cpustate,UINT8 mod_rm, UINT32* out_ea, UINT8* out_segment)
{
	INT8 disp8;
	INT16 disp16;
	INT32 disp32;
	UINT8 mod = (mod_rm >> 6) & 0x3;
	UINT8 rm = mod_rm & 0x7;
	UINT32 ea;
	UINT8 segment;

	if( mod_rm >= 0xc0 )
		fatalerror("i386: Called modrm_to_EA with modrm value %02X !",mod_rm);

	if( cpustate->address_size ) {
		switch( rm )
		{
			default:
			case 0: ea = REG32(EAX); segment = DS; break;
			case 1: ea = REG32(ECX); segment = DS; break;
			case 2: ea = REG32(EDX); segment = DS; break;
			case 3: ea = REG32(EBX); segment = DS; break;
			case 4: sib_byte(cpustate, mod, &ea, &segment ); break;
			case 5:
				if( mod == 0 ) {
					ea = FETCH32(cpustate); segment = DS;
				} else {
					ea = REG32(EBP); segment = SS;
				}
				break;
			case 6: ea = REG32(ESI); segment = DS; break;
			case 7: ea = REG32(EDI); segment = DS; break;
		}
		if( mod == 1 ) {
			disp8 = FETCH(cpustate);
			ea += (INT32)disp8;
		} else if( mod == 2 ) {
			disp32 = FETCH32(cpustate);
			ea += disp32;
		}

		if( cpustate->segment_prefix )
			segment = cpustate->segment_override;

		*out_ea = ea;
		*out_segment = segment;

	} else {
		switch( rm )
		{
			default:
			case 0: ea = REG16(BX) + REG16(SI); segment = DS; break;
			case 1: ea = REG16(BX) + REG16(DI); segment = DS; break;
			case 2: ea = REG16(BP) + REG16(SI); segment = SS; break;
			case 3: ea = REG16(BP) + REG16(DI); segment = SS; break;
			case 4: ea = REG16(SI); segment = DS; break;
			case 5: ea = REG16(DI); segment = DS; break;
			case 6:
				if( mod == 0 ) {
					ea = FETCH16(cpustate); segment = DS;
				} else {
					ea = REG16(BP); segment = SS;
				}
				break;
			case 7: ea = REG16(BX); segment = DS; break;
		}
		if( mod == 1 ) {
			disp8 = FETCH(cpustate);
			ea += (INT32)disp8;
		} else if( mod == 2 ) {
			disp16 = FETCH16(cpustate);
			ea += (INT32)disp16;
		}

		if( cpustate->segment_prefix )
			segment = cpustate->segment_override;

		*out_ea = ea & 0xffff;
		*out_segment = segment;
	}
}

static UINT32 GetNonTranslatedEA(i386_state *cpustate,UINT8 modrm)
{
	UINT8 segment;
	UINT32 ea;
	modrm_to_EA(cpustate, modrm, &ea, &segment );
	return ea;
}

static UINT32 GetEA(i386_state *cpustate,UINT8 modrm)
{
	UINT8 segment;
	UINT32 ea;
	modrm_to_EA(cpustate, modrm, &ea, &segment );
	return i386_translate(cpustate, segment, ea );
}

static void i386_trap(i386_state *cpustate,int irq, int irq_gate)
{
	/*  I386 Interrupts/Traps/Faults:
     *
     *  0x00    Divide by zero
     *  0x01    Debug exception
     *  0x02    NMI
     *  0x03    Int3
     *  0x04    Overflow
     *  0x05    Array bounds check
     *  0x06    Illegal Opcode
     *  0x07    FPU not available
     *  0x08    Double fault
     *  0x09    Coprocessor segment overrun
     *  0x0a    Invalid task state
     *  0x0b    Segment not present
     *  0x0c    Stack exception
     *  0x0d    General Protection Fault
     *  0x0e    Page fault
     *  0x0f    Reserved
     *  0x10    Coprocessor error
     */
	UINT32 v1, v2;
	UINT32 offset;
	UINT16 segment;
	int entry = irq * (PROTECTED_MODE ? 8 : 4);

	/* Check if IRQ is out of IDTR's bounds */
	if( entry > cpustate->idtr.limit ) {
		fatalerror("I386 Interrupt: IRQ out of IDTR bounds (IRQ: %d, IDTR Limit: %d)", irq, cpustate->idtr.limit);
	}

	if( !(PROTECTED_MODE) )
	{
		/* 16-bit */
		PUSH16(cpustate, get_flags(cpustate) & 0xffff );
		PUSH16(cpustate, cpustate->sreg[CS].selector );
		PUSH16(cpustate, cpustate->eip );

		cpustate->sreg[CS].selector = READ16(cpustate, cpustate->idtr.base + entry + 2 );
		cpustate->eip = READ16(cpustate, cpustate->idtr.base + entry );

		/* Interrupts that vector through either interrupt gates or trap gates cause TF */
		/* (the trap flag) to be reset after the current value of TF is saved on the stack as part of EFLAGS. */
		cpustate->TF = 0;

		if (irq_gate)
		{
			cpustate->IF = 0;
		}

	}
	else
	{
		int type;
		/* 32-bit */

		v1 = READ32(cpustate, cpustate->idtr.base + entry );
		v2 = READ32(cpustate, cpustate->idtr.base + entry + 4 );
		offset = (v2 & 0xffff0000) | (v1 & 0xffff);
		segment = (v1 >> 16) & 0xffff;
		type = (v2>>8) & 0x1F;

		if(type != 0x0e && type != 0x0f)  // if not 386 interrupt or trap gate
		{
			PUSH16(cpustate, get_flags(cpustate) & 0xffff );
			PUSH16(cpustate, cpustate->sreg[CS].selector );
			PUSH16(cpustate, cpustate->eip );
		}
		else
		{
			PUSH32(cpustate, get_flags(cpustate) & 0x00fcffff );
			PUSH32(cpustate, cpustate->sreg[CS].selector );
			PUSH32(cpustate, cpustate->eip );
		}

		cpustate->sreg[CS].selector = segment;
		cpustate->eip = offset;

		/* Interrupts that vector through either interrupt gates or trap gates cause TF */
		/* (the trap flag) to be reset after the current value of TF is saved on the stack as part of EFLAGS. */
		if ((type == 14) || (type==15))
			cpustate->TF = 0;

		if (type == 14)
		{
			cpustate->IF = 0;
		}

	}

	i386_load_segment_descriptor(cpustate,CS);
	CHANGE_PC(cpustate,cpustate->eip);

}

static void i386_check_irq_line(i386_state *cpustate)
{
	/* Check if the interrupts are enabled */
	if ( (cpustate->irq_state) && cpustate->IF )
	{
		cpustate->cycles -= 2;
		i386_trap(cpustate,cpustate->irq_callback(cpustate->device, 0), 1);
	}
}

#include "cycles.h"

static UINT8 *cycle_table_rm[X86_NUM_CPUS];
static UINT8 *cycle_table_pm[X86_NUM_CPUS];

#define CYCLES_NUM(x)	(cpustate->cycles -= (x))

INLINE void CYCLES(i386_state *cpustate,int x)
{
	if (PROTECTED_MODE)
	{
		cpustate->cycles -= cpustate->cycle_table_pm[x];
	}
	else
	{
		cpustate->cycles -= cpustate->cycle_table_rm[x];
	}
}

INLINE void CYCLES_RM(i386_state *cpustate,int modrm, int r, int m)
{
	if (modrm >= 0xc0)
	{
		if (PROTECTED_MODE)
		{
			cpustate->cycles -= cpustate->cycle_table_pm[r];
		}
		else
		{
			cpustate->cycles -= cpustate->cycle_table_rm[r];
		}
	}
	else
	{
		if (PROTECTED_MODE)
		{
			cpustate->cycles -= cpustate->cycle_table_pm[m];
		}
		else
		{
			cpustate->cycles -= cpustate->cycle_table_rm[m];
		}
	}
}

static void build_cycle_table(running_machine *machine)
{
	int i, j;
	for (j=0; j < X86_NUM_CPUS; j++)
	{
		cycle_table_rm[j] = auto_alloc_array(machine, UINT8, CYCLES_NUM_OPCODES);
		cycle_table_pm[j] = auto_alloc_array(machine, UINT8, CYCLES_NUM_OPCODES);

		for (i=0; i < sizeof(x86_cycle_table)/sizeof(X86_CYCLE_TABLE); i++)
		{
			int opcode = x86_cycle_table[i].op;
			cycle_table_rm[j][opcode] = x86_cycle_table[i].cpu_cycles[j][0];
			cycle_table_pm[j][opcode] = x86_cycle_table[i].cpu_cycles[j][1];
		}
	}
}

/* Forward declarations */
static void I386OP(decode_opcode)(i386_state *cpustate);
static void I386OP(decode_two_byte)(i386_state *cpustate);



#include "i386ops.c"
#include "i386op16.c"
#include "i386op32.c"
#include "i486ops.c"
#include "pentops.c"
#include "x87ops.c"
#include "i386ops.h"

static void I386OP(decode_opcode)(i386_state *cpustate)
{
	cpustate->opcode = FETCH(cpustate);
	if( cpustate->operand_size )
		cpustate->opcode_table1_32[cpustate->opcode](cpustate);
	else
		cpustate->opcode_table1_16[cpustate->opcode](cpustate);
}

/* Two-byte opcode prefix */
static void I386OP(decode_two_byte)(i386_state *cpustate)
{
	cpustate->opcode = FETCH(cpustate);
	if( cpustate->operand_size )
		cpustate->opcode_table2_32[cpustate->opcode](cpustate);
	else
		cpustate->opcode_table2_16[cpustate->opcode](cpustate);
}

/*************************************************************************/

static UINT64 i386_debug_segbase(void *globalref, void *ref, UINT32 params, const UINT64 *param)
{
	running_device *device = (running_device *)ref;
	i386_state *cpustate = get_safe_token(device);
	UINT32 result;
	I386_SREG seg;

	if (PROTECTED_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = (UINT16) param[0];
		i386_load_protected_mode_segment(cpustate,&seg);
		result = seg.base;
	}
	else
	{
		result = param[0] << 4;
	}
	return result;
}

static UINT64 i386_debug_seglimit(void *globalref, void *ref, UINT32 params, const UINT64 *param)
{
	running_device *device = (running_device *)ref;
	i386_state *cpustate = get_safe_token(device);
	UINT32 result = 0;
	I386_SREG seg;

	if (PROTECTED_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = (UINT16) param[0];
		i386_load_protected_mode_segment(cpustate,&seg);
		result = seg.limit;
	}
	return result;
}

static CPU_DEBUG_INIT( i386 )
{
	symtable_add_function(debug_cpu_get_symtable(device), "segbase", (void *)device, 1, 1, i386_debug_segbase);
	symtable_add_function(debug_cpu_get_symtable(device), "seglimit", (void *)device, 1, 1, i386_debug_seglimit);
}

/*************************************************************************/

static STATE_POSTLOAD( i386_postload )
{
	running_device *device = (running_device *)param;
	i386_state *cpustate = get_safe_token(device);
	int i;
	for (i = 0; i < 6; i++)
		i386_load_segment_descriptor(cpustate,i);
	CHANGE_PC(cpustate,cpustate->eip);
}

static CPU_INIT( i386 )
{
	int i, j;
	static const int regs8[8] = {AL,CL,DL,BL,AH,CH,DH,BH};
	static const int regs16[8] = {AX,CX,DX,BX,SP,BP,SI,DI};
	static const int regs32[8] = {EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI};
	i386_state *cpustate = get_safe_token(device);

	build_cycle_table(device->machine);

	for( i=0; i < 256; i++ ) {
		int c=0;
		for( j=0; j < 8; j++ ) {
			if( i & (1 << j) )
				c++;
		}
		i386_parity_table[i] = ~(c & 0x1) & 0x1;
	}

	for( i=0; i < 256; i++ ) {
		i386_MODRM_table[i].reg.b = regs8[(i >> 3) & 0x7];
		i386_MODRM_table[i].reg.w = regs16[(i >> 3) & 0x7];
		i386_MODRM_table[i].reg.d = regs32[(i >> 3) & 0x7];

		i386_MODRM_table[i].rm.b = regs8[i & 0x7];
		i386_MODRM_table[i].rm.w = regs16[i & 0x7];
		i386_MODRM_table[i].rm.d = regs32[i & 0x7];
	}

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);

	state_save_register_device_item_array(device, 0,	cpustate->reg.d);
	state_save_register_device_item(device, 0, cpustate->sreg[ES].selector);
	state_save_register_device_item(device, 0, cpustate->sreg[ES].base);
	state_save_register_device_item(device, 0, cpustate->sreg[ES].limit);
	state_save_register_device_item(device, 0, cpustate->sreg[ES].flags);
	state_save_register_device_item(device, 0, cpustate->sreg[CS].selector);
	state_save_register_device_item(device, 0, cpustate->sreg[CS].base);
	state_save_register_device_item(device, 0, cpustate->sreg[CS].limit);
	state_save_register_device_item(device, 0, cpustate->sreg[CS].flags);
	state_save_register_device_item(device, 0, cpustate->sreg[SS].selector);
	state_save_register_device_item(device, 0, cpustate->sreg[SS].base);
	state_save_register_device_item(device, 0, cpustate->sreg[SS].limit);
	state_save_register_device_item(device, 0, cpustate->sreg[SS].flags);
	state_save_register_device_item(device, 0, cpustate->sreg[DS].selector);
	state_save_register_device_item(device, 0, cpustate->sreg[DS].base);
	state_save_register_device_item(device, 0, cpustate->sreg[DS].limit);
	state_save_register_device_item(device, 0, cpustate->sreg[DS].flags);
	state_save_register_device_item(device, 0, cpustate->sreg[FS].selector);
	state_save_register_device_item(device, 0, cpustate->sreg[FS].base);
	state_save_register_device_item(device, 0, cpustate->sreg[FS].limit);
	state_save_register_device_item(device, 0, cpustate->sreg[FS].flags);
	state_save_register_device_item(device, 0, cpustate->sreg[GS].selector);
	state_save_register_device_item(device, 0, cpustate->sreg[GS].base);
	state_save_register_device_item(device, 0, cpustate->sreg[GS].limit);
	state_save_register_device_item(device, 0, cpustate->sreg[GS].flags);
	state_save_register_device_item(device, 0, cpustate->eip);
	state_save_register_device_item(device, 0, cpustate->prev_eip);
	state_save_register_device_item(device, 0, cpustate->CF);
	state_save_register_device_item(device, 0, cpustate->DF);
	state_save_register_device_item(device, 0, cpustate->SF);
	state_save_register_device_item(device, 0, cpustate->OF);
	state_save_register_device_item(device, 0, cpustate->ZF);
	state_save_register_device_item(device, 0, cpustate->PF);
	state_save_register_device_item(device, 0, cpustate->AF);
	state_save_register_device_item(device, 0, cpustate->IF);
	state_save_register_device_item(device, 0, cpustate->TF);
	state_save_register_device_item_array(device, 0,	cpustate->cr);
	state_save_register_device_item_array(device, 0,	cpustate->dr);
	state_save_register_device_item_array(device, 0,	cpustate->tr);
	state_save_register_device_item(device, 0, cpustate->idtr.base);
	state_save_register_device_item(device, 0, cpustate->idtr.limit);
	state_save_register_device_item(device, 0, cpustate->gdtr.base);
	state_save_register_device_item(device, 0, cpustate->gdtr.limit);
	state_save_register_device_item(device, 0, cpustate->task.base);
	state_save_register_device_item(device, 0, cpustate->task.segment);
	state_save_register_device_item(device, 0, cpustate->task.limit);
	state_save_register_device_item(device, 0, cpustate->task.flags);
	state_save_register_device_item(device, 0, cpustate->ldtr.base);
	state_save_register_device_item(device, 0, cpustate->ldtr.segment);
	state_save_register_device_item(device, 0, cpustate->ldtr.limit);
	state_save_register_device_item(device, 0, cpustate->ldtr.flags);
	state_save_register_device_item(device, 0,  cpustate->irq_state);
	state_save_register_device_item(device, 0, cpustate->performed_intersegment_jump);
	state_save_register_postload(device->machine, i386_postload, (void *)device);
}

static void build_opcode_table(i386_state *cpustate, UINT32 features)
{
	int i;
	for (i=0; i < 256; i++)
	{
		cpustate->opcode_table1_16[i] = I386OP(invalid);
		cpustate->opcode_table1_32[i] = I386OP(invalid);
		cpustate->opcode_table2_16[i] = I386OP(invalid);
		cpustate->opcode_table2_32[i] = I386OP(invalid);
	}

	for (i=0; i < sizeof(x86_opcode_table)/sizeof(X86_OPCODE); i++)
	{
		const X86_OPCODE *op = &x86_opcode_table[i];

		if ((op->flags & features))
		{
			if (op->flags & OP_2BYTE)
			{
				cpustate->opcode_table2_32[op->opcode] = op->handler32;
				cpustate->opcode_table2_16[op->opcode] = op->handler16;
			}
			else
			{
				cpustate->opcode_table1_32[op->opcode] = op->handler32;
				cpustate->opcode_table1_16[op->opcode] = op->handler16;
			}
		}
	}
}

static CPU_RESET( i386 )
{
	i386_state *cpustate = get_safe_token(device);
	cpu_irq_callback save_irqcallback;

	save_irqcallback = cpustate->irq_callback;
	memset( cpustate, 0, sizeof(*cpustate) );
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base		= 0xffff0000;
	cpustate->sreg[CS].limit	= 0xffff;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x7ffffff0; // reserved bits set to 1
	cpustate->eflags = 0;
	cpustate->eflags_mask = 0x00030000;
	cpustate->eip = 0xfff0;

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 3 (386), Model 0 (DX), Stepping 8 (D1)
	REG32(EAX) = 0;
	REG32(EDX) = (3 << 8) | (0 << 4) | (8);

	build_opcode_table(cpustate, OP_I386);
	cpustate->cycle_table_rm = cycle_table_rm[CPU_CYCLES_I386];
	cpustate->cycle_table_pm = cycle_table_pm[CPU_CYCLES_I386];

	CHANGE_PC(cpustate,cpustate->eip);
}

static void i386_set_irq_line(i386_state *cpustate,int irqline, int state)
{
	if (state != CLEAR_LINE && cpustate->halted)
	{
		cpustate->halted = 0;
	}

	if ( irqline == INPUT_LINE_NMI )
	{
		/* NMI (I do not think that this is 100% right) */
		if ( state )
			i386_trap(cpustate,2, 1);
	}
	else
	{
		cpustate->irq_state = state;
	}
}

static void i386_set_a20_line(i386_state *cpustate,int state)
{
	if (state)
	{
		cpustate->a20_mask = ~0;
	}
	else
	{
		cpustate->a20_mask = ~(1 << 20);
	}
}

static CPU_EXECUTE( i386 )
{
	i386_state *cpustate = get_safe_token(device);

	cpustate->cycles = cycles;
	cpustate->base_cycles = cycles;
	CHANGE_PC(cpustate,cpustate->eip);

	if (cpustate->halted)
	{
		cpustate->tsc += cycles;
		return cycles;
	}

	while( cpustate->cycles > 0 )
	{
		cpustate->operand_size = cpustate->sreg[CS].d;
		cpustate->address_size = cpustate->sreg[CS].d;

		cpustate->segment_prefix = 0;
		cpustate->prev_eip = cpustate->eip;

		debugger_instruction_hook(device, cpustate->pc);

		i386_check_irq_line(cpustate);
		I386OP(decode_opcode)(cpustate);
	}
	cpustate->tsc += (cycles - cpustate->cycles);

	return cycles - cpustate->cycles;
}

/*************************************************************************/

static CPU_TRANSLATE( i386 )
{
	i386_state *cpustate = get_safe_token(device);
	int result = 1;
	if (space == ADDRESS_SPACE_PROGRAM)
	{
		if (cpustate->cr[0] & 0x80000000)
			result = translate_address(cpustate,address);
		*address &= cpustate->a20_mask;
	}
	return result;
}

static CPU_DISASSEMBLE( i386 )
{
	i386_state *cpustate = get_safe_token(device);
	return i386_dasm_one(buffer, pc, oprom, cpustate->sreg[CS].d ? 32 : 16);
}

static CPU_SET_INFO( i386 )
{
	i386_state *cpustate = get_safe_token(device);

	if (state == CPUINFO_INT_INPUT_STATE+INPUT_LINE_A20)
	{
		i386_set_a20_line(cpustate,info->i);
		return;
	}
	if (state >= CPUINFO_INT_INPUT_STATE && state <= CPUINFO_INT_INPUT_STATE + MAX_INPUT_LINES)
	{
		i386_set_irq_line(cpustate,state-CPUINFO_INT_INPUT_STATE, info->i);
		return;
	}

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I386_PC:			cpustate->pc = info->i;							break;
		case CPUINFO_INT_REGISTER + I386_EIP:			cpustate->eip = info->i; CHANGE_PC(cpustate,cpustate->eip);		break;
		case CPUINFO_INT_REGISTER + I386_AL:			REG8(AL) = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_AH:			REG8(AH) = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_BL:			REG8(BL) = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_BH:			REG8(BH) = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_CL:			REG8(CL) = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_CH:			REG8(CH) = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_DL:			REG8(DL) = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_DH:			REG8(DH) = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_AX:			REG16(AX) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_BX:			REG16(BX) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_CX:			REG16(CX) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_DX:			REG16(DX) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_SI:			REG16(SI) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_DI:			REG16(DI) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_BP:			REG16(BP) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_SP:			REG16(SP) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_IP:			cpustate->eip = (cpustate->eip & ~0xFFFF) | (info->i & 0xFFFF); CHANGE_PC(cpustate,cpustate->eip); break;
		case CPUINFO_INT_REGISTER + I386_EAX:			REG32(EAX) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_EBX:			REG32(EBX) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_ECX:			REG32(ECX) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_EDX:			REG32(EDX) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_EBP:			REG32(EBP) = info->i;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + I386_ESP:			REG32(ESP) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_ESI:			REG32(ESI) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_EDI:			REG32(EDI) = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_EFLAGS:		cpustate->eflags = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_CS:			cpustate->sreg[CS].selector = info->i & 0xffff; i386_load_segment_descriptor(cpustate,CS); break;
		case CPUINFO_INT_REGISTER + I386_CS_BASE:		cpustate->sreg[CS].base = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_CS_LIMIT:		cpustate->sreg[CS].limit = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_CS_FLAGS:		cpustate->sreg[CS].flags = info->i & 0xf0ff;	break;
		case CPUINFO_INT_REGISTER + I386_SS:			cpustate->sreg[SS].selector = info->i & 0xffff; i386_load_segment_descriptor(cpustate,SS); break;
		case CPUINFO_INT_REGISTER + I386_SS_BASE:		cpustate->sreg[SS].base = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_SS_LIMIT:		cpustate->sreg[SS].limit = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_SS_FLAGS:		cpustate->sreg[SS].flags = info->i & 0xf0ff;	break;
		case CPUINFO_INT_REGISTER + I386_DS:			cpustate->sreg[DS].selector = info->i & 0xffff; i386_load_segment_descriptor(cpustate,DS); break;
		case CPUINFO_INT_REGISTER + I386_DS_BASE:		cpustate->sreg[DS].base = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_DS_LIMIT:		cpustate->sreg[DS].limit = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_DS_FLAGS:		cpustate->sreg[DS].flags = info->i & 0xf0ff;	break;
		case CPUINFO_INT_REGISTER + I386_ES:			cpustate->sreg[ES].selector = info->i & 0xffff; i386_load_segment_descriptor(cpustate,ES); break;
		case CPUINFO_INT_REGISTER + I386_ES_BASE:		cpustate->sreg[ES].base = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_ES_LIMIT:		cpustate->sreg[ES].limit = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_ES_FLAGS:		cpustate->sreg[ES].flags = info->i & 0xf0ff;	break;
		case CPUINFO_INT_REGISTER + I386_FS:			cpustate->sreg[FS].selector = info->i & 0xffff; i386_load_segment_descriptor(cpustate,FS); break;
		case CPUINFO_INT_REGISTER + I386_FS_BASE:		cpustate->sreg[FS].base = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_FS_LIMIT:		cpustate->sreg[FS].limit = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_FS_FLAGS:		cpustate->sreg[FS].flags = info->i & 0xf0ff;	break;
		case CPUINFO_INT_REGISTER + I386_GS:			cpustate->sreg[GS].selector = info->i & 0xffff; i386_load_segment_descriptor(cpustate,GS); break;
		case CPUINFO_INT_REGISTER + I386_GS_BASE:		cpustate->sreg[GS].base = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_GS_LIMIT:		cpustate->sreg[GS].limit = info->i;				break;
		case CPUINFO_INT_REGISTER + I386_GS_FLAGS:		cpustate->sreg[GS].flags = info->i & 0xf0ff;	break;
		case CPUINFO_INT_REGISTER + I386_CR0:			cpustate->cr[0] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_CR1:			cpustate->cr[1] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_CR2:			cpustate->cr[2] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_CR3:			cpustate->cr[3] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_CR4:			cpustate->cr[4] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_DR0:			cpustate->dr[0] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_DR1:			cpustate->dr[1] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_DR2:			cpustate->dr[2] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_DR3:			cpustate->dr[3] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_DR4:			cpustate->dr[4] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_DR5:			cpustate->dr[5] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_DR6:			cpustate->dr[6] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_DR7:			cpustate->dr[7] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_TR6:			cpustate->tr[6] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_TR7:			cpustate->tr[7] = info->i;						break;
		case CPUINFO_INT_REGISTER + I386_GDTR_BASE:		cpustate->gdtr.base = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_GDTR_LIMIT:	cpustate->gdtr.limit = info->i & 0xffff;		break;
		case CPUINFO_INT_REGISTER + I386_IDTR_BASE:		cpustate->idtr.base = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_IDTR_LIMIT:	cpustate->idtr.limit = info->i & 0xffff;		break;
		case CPUINFO_INT_REGISTER + I386_TR:			cpustate->task.segment = info->i & 0xffff;		break;
		case CPUINFO_INT_REGISTER + I386_TR_BASE:		cpustate->task.base = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_TR_LIMIT:		cpustate->task.limit = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_TR_FLAGS:		cpustate->task.flags = info->i & 0xf0ff;		break;
		case CPUINFO_INT_REGISTER + I386_LDTR:			cpustate->ldtr.segment = info->i & 0xffff;		break;
		case CPUINFO_INT_REGISTER + I386_LDTR_BASE:		cpustate->ldtr.base = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_LDTR_LIMIT:	cpustate->ldtr.limit = info->i;					break;
		case CPUINFO_INT_REGISTER + I386_LDTR_FLAGS:	cpustate->ldtr.flags = info->i & 0xf0ff;		break;
	}
}

CPU_GET_INFO( i386 )
{
	i386_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i386_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 32;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 15;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 40;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_LOGADDR_WIDTH_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_PAGE_SHIFT_PROGRAM:	info->i = 12;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE:					info->i = CLEAR_LINE;					break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I386_PC:			info->i = cpustate->pc;							break;
		case CPUINFO_INT_REGISTER + I386_EIP:			info->i = cpustate->eip;						break;
		case CPUINFO_INT_REGISTER + I386_AL:			info->i = REG8(AL);						break;
		case CPUINFO_INT_REGISTER + I386_AH:			info->i = REG8(AH);						break;
		case CPUINFO_INT_REGISTER + I386_BL:			info->i = REG8(BL);						break;
		case CPUINFO_INT_REGISTER + I386_BH:			info->i = REG8(BH);						break;
		case CPUINFO_INT_REGISTER + I386_CL:			info->i = REG8(CL);						break;
		case CPUINFO_INT_REGISTER + I386_CH:			info->i = REG8(CH);						break;
		case CPUINFO_INT_REGISTER + I386_DL:			info->i = REG8(DL);						break;
		case CPUINFO_INT_REGISTER + I386_DH:			info->i = REG8(DH);						break;
		case CPUINFO_INT_REGISTER + I386_AX:			info->i = REG16(AX);					break;
		case CPUINFO_INT_REGISTER + I386_BX:			info->i = REG16(BX);					break;
		case CPUINFO_INT_REGISTER + I386_CX:			info->i = REG16(CX);					break;
		case CPUINFO_INT_REGISTER + I386_DX:			info->i = REG16(DX);					break;
		case CPUINFO_INT_REGISTER + I386_SI:			info->i = REG16(SI);					break;
		case CPUINFO_INT_REGISTER + I386_DI:			info->i = REG16(DI);					break;
		case CPUINFO_INT_REGISTER + I386_BP:			info->i = REG16(BP);					break;
		case CPUINFO_INT_REGISTER + I386_SP:			info->i = REG16(SP);					break;
		case CPUINFO_INT_REGISTER + I386_IP:			info->i = cpustate->eip & 0xFFFF;				break;
		case CPUINFO_INT_REGISTER + I386_EAX:			info->i = REG32(EAX);					break;
		case CPUINFO_INT_REGISTER + I386_EBX:			info->i = REG32(EBX);					break;
		case CPUINFO_INT_REGISTER + I386_ECX:			info->i = REG32(ECX);					break;
		case CPUINFO_INT_REGISTER + I386_EDX:			info->i = REG32(EDX);					break;
		case CPUINFO_INT_REGISTER + I386_EBP:			info->i = REG32(EBP);					break;
		case CPUINFO_INT_REGISTER + I386_ESP:			info->i = REG32(ESP);					break;
		case CPUINFO_INT_REGISTER + I386_ESI:			info->i = REG32(ESI);					break;
		case CPUINFO_INT_REGISTER + I386_EDI:			info->i = REG32(EDI);					break;
		case CPUINFO_INT_REGISTER + I386_EFLAGS:		info->i = cpustate->eflags;						break;
		case CPUINFO_INT_REGISTER + I386_CS:			info->i = cpustate->sreg[CS].selector;			break;
		case CPUINFO_INT_REGISTER + I386_CS_BASE:		info->i = cpustate->sreg[CS].base;				break;
		case CPUINFO_INT_REGISTER + I386_CS_LIMIT:		info->i = cpustate->sreg[CS].limit;				break;
		case CPUINFO_INT_REGISTER + I386_CS_FLAGS:		info->i = cpustate->sreg[CS].flags;				break;
		case CPUINFO_INT_REGISTER + I386_SS:			info->i = cpustate->sreg[SS].selector;			break;
		case CPUINFO_INT_REGISTER + I386_SS_BASE:		info->i = cpustate->sreg[SS].base;				break;
		case CPUINFO_INT_REGISTER + I386_SS_LIMIT:		info->i = cpustate->sreg[SS].limit;				break;
		case CPUINFO_INT_REGISTER + I386_SS_FLAGS:		info->i = cpustate->sreg[SS].flags;				break;
		case CPUINFO_INT_REGISTER + I386_DS:			info->i = cpustate->sreg[DS].selector;			break;
		case CPUINFO_INT_REGISTER + I386_DS_BASE:		info->i = cpustate->sreg[DS].base;				break;
		case CPUINFO_INT_REGISTER + I386_DS_LIMIT:		info->i = cpustate->sreg[DS].limit;				break;
		case CPUINFO_INT_REGISTER + I386_DS_FLAGS:		info->i = cpustate->sreg[DS].flags;				break;
		case CPUINFO_INT_REGISTER + I386_ES:			info->i = cpustate->sreg[ES].selector;			break;
		case CPUINFO_INT_REGISTER + I386_ES_BASE:		info->i = cpustate->sreg[ES].base;				break;
		case CPUINFO_INT_REGISTER + I386_ES_LIMIT:		info->i = cpustate->sreg[ES].limit;				break;
		case CPUINFO_INT_REGISTER + I386_ES_FLAGS:		info->i = cpustate->sreg[ES].flags;				break;
		case CPUINFO_INT_REGISTER + I386_FS:			info->i = cpustate->sreg[FS].selector;			break;
		case CPUINFO_INT_REGISTER + I386_FS_BASE:		info->i = cpustate->sreg[FS].base;				break;
		case CPUINFO_INT_REGISTER + I386_FS_LIMIT:		info->i = cpustate->sreg[FS].limit;				break;
		case CPUINFO_INT_REGISTER + I386_FS_FLAGS:		info->i = cpustate->sreg[FS].flags;				break;
		case CPUINFO_INT_REGISTER + I386_GS:			info->i = cpustate->sreg[GS].selector;			break;
		case CPUINFO_INT_REGISTER + I386_GS_BASE:		info->i = cpustate->sreg[GS].base;				break;
		case CPUINFO_INT_REGISTER + I386_GS_LIMIT:		info->i = cpustate->sreg[GS].limit;				break;
		case CPUINFO_INT_REGISTER + I386_GS_FLAGS:		info->i = cpustate->sreg[GS].flags;				break;
		case CPUINFO_INT_REGISTER + I386_CR0:			info->i = cpustate->cr[0];						break;
		case CPUINFO_INT_REGISTER + I386_CR1:			info->i = cpustate->cr[1];						break;
		case CPUINFO_INT_REGISTER + I386_CR2:			info->i = cpustate->cr[2];						break;
		case CPUINFO_INT_REGISTER + I386_CR3:			info->i = cpustate->cr[3];						break;
		case CPUINFO_INT_REGISTER + I386_CR4:			info->i = cpustate->cr[4];						break;
		case CPUINFO_INT_REGISTER + I386_DR0:			info->i = cpustate->dr[0];						break;
		case CPUINFO_INT_REGISTER + I386_DR1:			info->i = cpustate->dr[1];						break;
		case CPUINFO_INT_REGISTER + I386_DR2:			info->i = cpustate->dr[2];						break;
		case CPUINFO_INT_REGISTER + I386_DR3:			info->i = cpustate->dr[3];						break;
		case CPUINFO_INT_REGISTER + I386_DR4:			info->i = cpustate->dr[4];						break;
		case CPUINFO_INT_REGISTER + I386_DR5:			info->i = cpustate->dr[5];						break;
		case CPUINFO_INT_REGISTER + I386_DR6:			info->i = cpustate->dr[6];						break;
		case CPUINFO_INT_REGISTER + I386_DR7:			info->i = cpustate->dr[7];						break;
		case CPUINFO_INT_REGISTER + I386_TR6:			info->i = cpustate->tr[6];						break;
		case CPUINFO_INT_REGISTER + I386_TR7:			info->i = cpustate->tr[7];						break;
		case CPUINFO_INT_REGISTER + I386_GDTR_BASE:		info->i = cpustate->gdtr.base;					break;
		case CPUINFO_INT_REGISTER + I386_GDTR_LIMIT:	info->i = cpustate->gdtr.limit;					break;
		case CPUINFO_INT_REGISTER + I386_IDTR_BASE:		info->i = cpustate->idtr.base;					break;
		case CPUINFO_INT_REGISTER + I386_IDTR_LIMIT:	info->i = cpustate->idtr.limit;					break;
		case CPUINFO_INT_REGISTER + I386_TR:			info->i = cpustate->task.segment;				break;
		case CPUINFO_INT_REGISTER + I386_TR_BASE:		info->i = cpustate->task.base;					break;
		case CPUINFO_INT_REGISTER + I386_TR_LIMIT:		info->i = cpustate->task.limit;					break;
		case CPUINFO_INT_REGISTER + I386_TR_FLAGS:		info->i = cpustate->task.flags;					break;
		case CPUINFO_INT_REGISTER + I386_LDTR:			info->i = cpustate->ldtr.segment;				break;
		case CPUINFO_INT_REGISTER + I386_LDTR_BASE:		info->i = cpustate->ldtr.base;					break;
		case CPUINFO_INT_REGISTER + I386_LDTR_LIMIT:	info->i = cpustate->ldtr.limit;					break;
		case CPUINFO_INT_REGISTER + I386_LDTR_FLAGS:	info->i = cpustate->ldtr.flags;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:	    				info->setinfo = CPU_SET_INFO_NAME(i386);			break;
		case CPUINFO_FCT_INIT:		    				info->init = CPU_INIT_NAME(i386);					break;
		case CPUINFO_FCT_RESET:		    				info->reset = CPU_RESET_NAME(i386);				break;
		case CPUINFO_FCT_EXECUTE:	    				info->execute = CPU_EXECUTE_NAME(i386);			break;
		case CPUINFO_FCT_BURN:		    				info->burn = NULL;						break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->cycles;				break;
		case CPUINFO_FCT_TRANSLATE:						info->translate = CPU_TRANSLATE_NAME(i386);	break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i386);			break;
		case CPUINFO_FCT_DEBUG_INIT:					info->debug_init = CPU_DEBUG_INIT_NAME(i386);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "I386");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Intel 386");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Ville Linde"); break;

		case CPUINFO_STR_FLAGS:							sprintf(info->s, "%08X", get_flags(cpustate));	break;

		case CPUINFO_STR_REGISTER + I386_PC:			sprintf(info->s, "PC: %08X", cpustate->pc);		break;
		case CPUINFO_STR_REGISTER + I386_EIP:			sprintf(info->s, "EIP: %08X", cpustate->eip);	break;
		case CPUINFO_STR_REGISTER + I386_AL:			sprintf(info->s, "~AL: %02X", REG8(AL)); break;
		case CPUINFO_STR_REGISTER + I386_AH:			sprintf(info->s, "~AH: %02X", REG8(AH)); break;
		case CPUINFO_STR_REGISTER + I386_BL:			sprintf(info->s, "~BL: %02X", REG8(BL)); break;
		case CPUINFO_STR_REGISTER + I386_BH:			sprintf(info->s, "~BH: %02X", REG8(BH)); break;
		case CPUINFO_STR_REGISTER + I386_CL:			sprintf(info->s, "~CL: %02X", REG8(CL)); break;
		case CPUINFO_STR_REGISTER + I386_CH:			sprintf(info->s, "~CH: %02X", REG8(CH)); break;
		case CPUINFO_STR_REGISTER + I386_DL:			sprintf(info->s, "~DL: %02X", REG8(DL)); break;
		case CPUINFO_STR_REGISTER + I386_DH:			sprintf(info->s, "~DH: %02X", REG8(DH)); break;
		case CPUINFO_STR_REGISTER + I386_AX:			sprintf(info->s, "~AX: %04X", REG16(AX)); break;
		case CPUINFO_STR_REGISTER + I386_BX:			sprintf(info->s, "~BX: %04X", REG16(BX)); break;
		case CPUINFO_STR_REGISTER + I386_CX:			sprintf(info->s, "~CX: %04X", REG16(CX)); break;
		case CPUINFO_STR_REGISTER + I386_DX:			sprintf(info->s, "~DX: %04X", REG16(DX)); break;
		case CPUINFO_STR_REGISTER + I386_SI:			sprintf(info->s, "~SI: %04X", REG16(SI)); break;
		case CPUINFO_STR_REGISTER + I386_DI:			sprintf(info->s, "~DI: %04X", REG16(DI)); break;
		case CPUINFO_STR_REGISTER + I386_BP:			sprintf(info->s, "~BP: %04X", REG16(BP)); break;
		case CPUINFO_STR_REGISTER + I386_SP:			sprintf(info->s, "~SP: %04X", REG16(SP)); break;
		case CPUINFO_STR_REGISTER + I386_IP:			sprintf(info->s, "~IP: %04X", cpustate->eip & 0xFFFF); break;
		case CPUINFO_STR_REGISTER + I386_EAX:			sprintf(info->s, "EAX: %08X", cpustate->reg.d[EAX]); break;
		case CPUINFO_STR_REGISTER + I386_EBX:			sprintf(info->s, "EBX: %08X", cpustate->reg.d[EBX]); break;
		case CPUINFO_STR_REGISTER + I386_ECX:			sprintf(info->s, "ECX: %08X", cpustate->reg.d[ECX]); break;
		case CPUINFO_STR_REGISTER + I386_EDX:			sprintf(info->s, "EDX: %08X", cpustate->reg.d[EDX]); break;
		case CPUINFO_STR_REGISTER + I386_EBP:			sprintf(info->s, "EBP: %08X", cpustate->reg.d[EBP]); break;
		case CPUINFO_STR_REGISTER + I386_ESP:			sprintf(info->s, "ESP: %08X", cpustate->reg.d[ESP]); break;
		case CPUINFO_STR_REGISTER + I386_ESI:			sprintf(info->s, "ESI: %08X", cpustate->reg.d[ESI]); break;
		case CPUINFO_STR_REGISTER + I386_EDI:			sprintf(info->s, "EDI: %08X", cpustate->reg.d[EDI]); break;
		case CPUINFO_STR_REGISTER + I386_EFLAGS:		sprintf(info->s, "EFLAGS: %08X", cpustate->eflags); break;
		case CPUINFO_STR_REGISTER + I386_CS:			sprintf(info->s, "CS: %04X", cpustate->sreg[CS].selector); break;
		case CPUINFO_STR_REGISTER + I386_CS_BASE:		sprintf(info->s, "CSBASE: %08X", cpustate->sreg[CS].base); break;
		case CPUINFO_STR_REGISTER + I386_CS_LIMIT:		sprintf(info->s, "CSLIMIT: %08X", cpustate->sreg[CS].limit); break;
		case CPUINFO_STR_REGISTER + I386_CS_FLAGS:		sprintf(info->s, "CSFLAGS: %04X", cpustate->sreg[CS].flags); break;
		case CPUINFO_STR_REGISTER + I386_SS:			sprintf(info->s, "SS: %04X", cpustate->sreg[SS].selector); break;
		case CPUINFO_STR_REGISTER + I386_SS_BASE:		sprintf(info->s, "SSBASE: %08X", cpustate->sreg[SS].base); break;
		case CPUINFO_STR_REGISTER + I386_SS_LIMIT:		sprintf(info->s, "SSLIMIT: %08X", cpustate->sreg[SS].limit); break;
		case CPUINFO_STR_REGISTER + I386_SS_FLAGS:		sprintf(info->s, "SSFLAGS: %04X", cpustate->sreg[SS].flags); break;
		case CPUINFO_STR_REGISTER + I386_DS:			sprintf(info->s, "DS: %04X", cpustate->sreg[DS].selector); break;
		case CPUINFO_STR_REGISTER + I386_DS_BASE:		sprintf(info->s, "DSBASE: %08X", cpustate->sreg[DS].base); break;
		case CPUINFO_STR_REGISTER + I386_DS_LIMIT:		sprintf(info->s, "DSLIMIT: %08X", cpustate->sreg[DS].limit); break;
		case CPUINFO_STR_REGISTER + I386_DS_FLAGS:		sprintf(info->s, "DSFLAGS: %04X", cpustate->sreg[DS].flags); break;
		case CPUINFO_STR_REGISTER + I386_ES:			sprintf(info->s, "ES: %04X", cpustate->sreg[ES].selector); break;
		case CPUINFO_STR_REGISTER + I386_ES_BASE:		sprintf(info->s, "ESBASE: %08X", cpustate->sreg[ES].base); break;
		case CPUINFO_STR_REGISTER + I386_ES_LIMIT:		sprintf(info->s, "ESLIMIT: %08X", cpustate->sreg[ES].limit); break;
		case CPUINFO_STR_REGISTER + I386_ES_FLAGS:		sprintf(info->s, "ESFLAGS: %04X", cpustate->sreg[ES].flags); break;
		case CPUINFO_STR_REGISTER + I386_FS:			sprintf(info->s, "FS: %04X", cpustate->sreg[FS].selector); break;
		case CPUINFO_STR_REGISTER + I386_FS_BASE:		sprintf(info->s, "FSBASE: %08X", cpustate->sreg[FS].base); break;
		case CPUINFO_STR_REGISTER + I386_FS_LIMIT:		sprintf(info->s, "FSLIMIT: %08X", cpustate->sreg[FS].limit); break;
		case CPUINFO_STR_REGISTER + I386_FS_FLAGS:		sprintf(info->s, "FSFLAGS: %04X", cpustate->sreg[FS].flags); break;
		case CPUINFO_STR_REGISTER + I386_GS:			sprintf(info->s, "GS: %04X", cpustate->sreg[GS].selector); break;
		case CPUINFO_STR_REGISTER + I386_GS_BASE:		sprintf(info->s, "GSBASE: %08X", cpustate->sreg[GS].base); break;
		case CPUINFO_STR_REGISTER + I386_GS_LIMIT:		sprintf(info->s, "GSLIMIT: %08X", cpustate->sreg[GS].limit); break;
		case CPUINFO_STR_REGISTER + I386_GS_FLAGS:		sprintf(info->s, "GSFLAGS: %04X", cpustate->sreg[GS].flags); break;
		case CPUINFO_STR_REGISTER + I386_CR0:			sprintf(info->s, "CR0: %08X", cpustate->cr[0]); break;
		case CPUINFO_STR_REGISTER + I386_CR1:			sprintf(info->s, "CR1: %08X", cpustate->cr[1]); break;
		case CPUINFO_STR_REGISTER + I386_CR2:			sprintf(info->s, "CR2: %08X", cpustate->cr[2]); break;
		case CPUINFO_STR_REGISTER + I386_CR3:			sprintf(info->s, "CR3: %08X", cpustate->cr[3]); break;
		case CPUINFO_STR_REGISTER + I386_CR4:			sprintf(info->s, "CR4: %08X", cpustate->cr[4]); break;
		case CPUINFO_STR_REGISTER + I386_DR0:			sprintf(info->s, "DR0: %08X", cpustate->dr[0]); break;
		case CPUINFO_STR_REGISTER + I386_DR1:			sprintf(info->s, "DR1: %08X", cpustate->dr[1]); break;
		case CPUINFO_STR_REGISTER + I386_DR2:			sprintf(info->s, "DR2: %08X", cpustate->dr[2]); break;
		case CPUINFO_STR_REGISTER + I386_DR3:			sprintf(info->s, "DR3: %08X", cpustate->dr[3]); break;
		case CPUINFO_STR_REGISTER + I386_DR4:			sprintf(info->s, "DR4: %08X", cpustate->dr[4]); break;
		case CPUINFO_STR_REGISTER + I386_DR5:			sprintf(info->s, "DR5: %08X", cpustate->dr[5]); break;
		case CPUINFO_STR_REGISTER + I386_DR6:			sprintf(info->s, "DR6: %08X", cpustate->dr[6]); break;
		case CPUINFO_STR_REGISTER + I386_DR7:			sprintf(info->s, "DR7: %08X", cpustate->dr[7]); break;
		case CPUINFO_STR_REGISTER + I386_TR6:			sprintf(info->s, "TR6: %08X", cpustate->tr[6]); break;
		case CPUINFO_STR_REGISTER + I386_TR7:			sprintf(info->s, "TR7: %08X", cpustate->tr[7]); break;
		case CPUINFO_STR_REGISTER + I386_GDTR_BASE:		sprintf(info->s, "GDTRBASE: %08X", cpustate->gdtr.base); break;
		case CPUINFO_STR_REGISTER + I386_GDTR_LIMIT:	sprintf(info->s, "GDTRLIMIT: %04X", cpustate->gdtr.limit); break;
		case CPUINFO_STR_REGISTER + I386_IDTR_BASE:		sprintf(info->s, "IDTRBASE: %08X", cpustate->idtr.base); break;
		case CPUINFO_STR_REGISTER + I386_IDTR_LIMIT:	sprintf(info->s, "IDTRLIMIT: %04X", cpustate->idtr.limit); break;
		case CPUINFO_STR_REGISTER + I386_LDTR:			sprintf(info->s, "LDTR: %04X", cpustate->ldtr.segment); break;
		case CPUINFO_STR_REGISTER + I386_LDTR_BASE:		sprintf(info->s, "LDTRBASE: %08X", cpustate->ldtr.base); break;
		case CPUINFO_STR_REGISTER + I386_LDTR_LIMIT:	sprintf(info->s, "LDTRLIMIT: %08X", cpustate->ldtr.limit); break;
		case CPUINFO_STR_REGISTER + I386_LDTR_FLAGS:	sprintf(info->s, "LDTRFLAGS: %04X", cpustate->ldtr.flags); break;
		case CPUINFO_STR_REGISTER + I386_TR:			sprintf(info->s, "TR: %04X", cpustate->task.segment); break;
		case CPUINFO_STR_REGISTER + I386_TR_BASE:		sprintf(info->s, "TRBASE: %08X", cpustate->task.base); break;
		case CPUINFO_STR_REGISTER + I386_TR_LIMIT:		sprintf(info->s, "TRLIMIT: %08X", cpustate->task.limit); break;
		case CPUINFO_STR_REGISTER + I386_TR_FLAGS:		sprintf(info->s, "TRFLAGS: %04X", cpustate->task.flags); break;
	}
}

/*****************************************************************************/
/* Intel 486 */


static CPU_INIT( i486 )
{
	CPU_INIT_CALL(i386);
}

static CPU_RESET( i486 )
{
	i386_state *cpustate = get_safe_token(device);
	cpu_irq_callback save_irqcallback;

	save_irqcallback = cpustate->irq_callback;
	memset( cpustate, 0, sizeof(*cpustate) );
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base		= 0xffff0000;
	cpustate->sreg[CS].limit	= 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x00000010;
	cpustate->eflags = 0;
	cpustate->eflags_mask = 0x00070000;
	cpustate->eip = 0xfff0;

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 4 (486), Model 0/1 (DX), Stepping 3
	REG32(EAX) = 0;
	REG32(EDX) = (4 << 8) | (0 << 4) | (3);

	build_opcode_table(cpustate, OP_I386 | OP_FPU | OP_I486);
	cpustate->cycle_table_rm = cycle_table_rm[CPU_CYCLES_I486];
	cpustate->cycle_table_pm = cycle_table_pm[CPU_CYCLES_I486];

	CHANGE_PC(cpustate,cpustate->eip);
}

static CPU_EXIT( i486 )
{
}

static CPU_SET_INFO( i486 )
{
	i386_state *cpustate = get_safe_token(device);
	switch (state)
	{
		case CPUINFO_INT_REGISTER + X87_CTRL:			cpustate->fpu_control_word = info->i;			break;
		case CPUINFO_INT_REGISTER + X87_STATUS:			cpustate->fpu_status_word = info->i;			break;
		case CPUINFO_INT_REGISTER + X87_ST0:			ST(0).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST1:			ST(1).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST2:			ST(2).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST3:			ST(3).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST4:			ST(4).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST5:			ST(5).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST6:			ST(6).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST7:			ST(7).f = info->i;						break;

		default:										CPU_SET_INFO_CALL(i386);				break;
	}
}

CPU_GET_INFO( i486 )
{
	i386_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		case CPUINFO_FCT_SET_INFO:	    				info->setinfo = CPU_SET_INFO_NAME(i486);break;
		case CPUINFO_FCT_INIT:		    				info->init = CPU_INIT_NAME(i486);		break;
		case CPUINFO_FCT_RESET:		    				info->reset = CPU_RESET_NAME(i486);		break;
		case CPUINFO_FCT_EXIT:		    				info->exit = CPU_EXIT_NAME(i486);		break;

		case CPUINFO_INT_REGISTER + X87_CTRL:			info->i = cpustate->fpu_control_word;			break;
		case CPUINFO_INT_REGISTER + X87_STATUS:			info->i = cpustate->fpu_status_word;			break;
		case CPUINFO_INT_REGISTER + X87_ST0:			info->i = ST(0).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST1:			info->i = ST(1).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST2:			info->i = ST(2).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST3:			info->i = ST(3).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST4:			info->i = ST(4).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST5:			info->i = ST(5).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST6:			info->i = ST(6).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST7:			info->i = ST(7).f;						break;

		case DEVINFO_STR_NAME:							strcpy(info->s, "I486");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Intel 486");			break;
		case CPUINFO_STR_REGISTER + X87_CTRL:			sprintf(info->s, "FPU_CW: %04X", cpustate->fpu_control_word); break;
		case CPUINFO_STR_REGISTER + X87_STATUS:			sprintf(info->s, "FPU_SW: %04X", cpustate->fpu_status_word); break;
		case CPUINFO_STR_REGISTER + X87_ST0:			sprintf(info->s, "ST0: %f", ST(0).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST1:			sprintf(info->s, "ST1: %f", ST(1).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST2:			sprintf(info->s, "ST2: %f", ST(2).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST3:			sprintf(info->s, "ST3: %f", ST(3).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST4:			sprintf(info->s, "ST4: %f", ST(4).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST5:			sprintf(info->s, "ST5: %f", ST(5).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST6:			sprintf(info->s, "ST6: %f", ST(6).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST7:			sprintf(info->s, "ST7: %f", ST(7).f);	break;

		default:										CPU_GET_INFO_CALL(i386);				break;
	}
}

/*****************************************************************************/
/* Pentium */


static CPU_INIT( pentium )
{
	CPU_INIT_CALL(i386);
}

static CPU_RESET( pentium )
{
	i386_state *cpustate = get_safe_token(device);
	cpu_irq_callback save_irqcallback;

	save_irqcallback = cpustate->irq_callback;
	memset( cpustate, 0, sizeof(*cpustate) );
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base		= 0xffff0000;
	cpustate->sreg[CS].limit	= 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x00000010;
	cpustate->eflags = 0;
	cpustate->eflags_mask = 0x003b0000;
	cpustate->eip = 0xfff0;

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 5 (Pentium), Model 2 (75 - 200MHz), Stepping 5
	REG32(EAX) = 0;
	REG32(EDX) = (5 << 8) | (2 << 4) | (5);

	build_opcode_table(cpustate, OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM);
	cpustate->cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM];
	cpustate->cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM];

	cpustate->cpuid_id0 = 0x756e6547;	// Genu
	cpustate->cpuid_id1 = 0x49656e69;	// ineI
	cpustate->cpuid_id2 = 0x6c65746e;	// ntel

	cpustate->cpuid_max_input_value_eax = 0x01;
	cpustate->cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	cpustate->feature_flags = 0x000001bf;

	CHANGE_PC(cpustate,cpustate->eip);
}

static CPU_EXIT( pentium )
{
}

static CPU_SET_INFO( pentium )
{
	i386_state *cpustate = get_safe_token(device);
	switch (state)
	{
		case CPUINFO_INT_REGISTER + X87_CTRL:			cpustate->fpu_control_word = info->i;			break;
		case CPUINFO_INT_REGISTER + X87_STATUS:			cpustate->fpu_status_word = info->i;			break;
		case CPUINFO_INT_REGISTER + X87_ST0:			ST(0).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST1:			ST(1).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST2:			ST(2).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST3:			ST(3).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST4:			ST(4).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST5:			ST(5).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST6:			ST(6).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST7:			ST(7).f = info->i;						break;

		default:										CPU_SET_INFO_CALL(i386);				break;
	}
}

CPU_GET_INFO( pentium )
{
	i386_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		case CPUINFO_FCT_SET_INFO:	    				info->setinfo = CPU_SET_INFO_NAME(pentium);	break;
		case CPUINFO_FCT_INIT:		    				info->init = CPU_INIT_NAME(pentium);	break;
		case CPUINFO_FCT_RESET:		    				info->reset = CPU_RESET_NAME(pentium);	break;
		case CPUINFO_FCT_EXIT:		    				info->exit = CPU_EXIT_NAME(pentium);	break;

		case CPUINFO_INT_REGISTER + X87_CTRL:			info->i = cpustate->fpu_control_word;			break;
		case CPUINFO_INT_REGISTER + X87_STATUS:			info->i = cpustate->fpu_status_word;			break;
		case CPUINFO_INT_REGISTER + X87_ST0:			info->i = ST(0).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST1:			info->i = ST(1).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST2:			info->i = ST(2).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST3:			info->i = ST(3).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST4:			info->i = ST(4).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST5:			info->i = ST(5).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST6:			info->i = ST(6).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST7:			info->i = ST(7).f;						break;

		case DEVINFO_STR_NAME:							strcpy(info->s, "PENTIUM");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Intel Pentium");		break;
		case CPUINFO_STR_REGISTER + X87_CTRL:			sprintf(info->s, "FPU_CW: %04X", cpustate->fpu_control_word); break;
		case CPUINFO_STR_REGISTER + X87_STATUS:			sprintf(info->s, "FPU_SW: %04X", cpustate->fpu_status_word); break;
		case CPUINFO_STR_REGISTER + X87_ST0:			sprintf(info->s, "ST0: %f", ST(0).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST1:			sprintf(info->s, "ST1: %f", ST(1).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST2:			sprintf(info->s, "ST2: %f", ST(2).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST3:			sprintf(info->s, "ST3: %f", ST(3).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST4:			sprintf(info->s, "ST4: %f", ST(4).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST5:			sprintf(info->s, "ST5: %f", ST(5).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST6:			sprintf(info->s, "ST6: %f", ST(6).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST7:			sprintf(info->s, "ST7: %f", ST(7).f);	break;

		default:										CPU_GET_INFO_CALL(i386);				break;
	}
}

/*****************************************************************************/
/* Cyrix MediaGX */


static CPU_INIT( mediagx )
{
	CPU_INIT_CALL(i386);
}

static CPU_RESET( mediagx )
{
	i386_state *cpustate = get_safe_token(device);
	cpu_irq_callback save_irqcallback;

	save_irqcallback = cpustate->irq_callback;
	memset( cpustate, 0, sizeof(*cpustate) );
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base		= 0xffff0000;
	cpustate->sreg[CS].limit	= 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x00000010;
	cpustate->eflags = 0;
	cpustate->eflags_mask = 0x00270000; /* TODO: is this correct? */
	cpustate->eip = 0xfff0;

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 4, Model 4 (MediaGX)
	REG32(EAX) = 0;
	REG32(EDX) = (4 << 8) | (4 << 4) | (1);	/* TODO: is this correct? */

	build_opcode_table(cpustate, OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_CYRIX);
	cpustate->cycle_table_rm = cycle_table_rm[CPU_CYCLES_MEDIAGX];
	cpustate->cycle_table_pm = cycle_table_pm[CPU_CYCLES_MEDIAGX];

	cpustate->cpuid_id0 = 0x69727943;	// Cyri
	cpustate->cpuid_id1 = 0x736e4978;	// xIns
	cpustate->cpuid_id2 = 0x6d616574;	// tead

	cpustate->cpuid_max_input_value_eax = 0x01;
	cpustate->cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	cpustate->feature_flags = 0x00000001;

	CHANGE_PC(cpustate,cpustate->eip);
}

static CPU_EXIT( mediagx )
{
}

static CPU_SET_INFO( mediagx )
{
	i386_state *cpustate = get_safe_token(device);
	switch (state)
	{
		case CPUINFO_INT_REGISTER + X87_CTRL:			cpustate->fpu_control_word = info->i;			break;
		case CPUINFO_INT_REGISTER + X87_STATUS:			cpustate->fpu_status_word = info->i;			break;
		case CPUINFO_INT_REGISTER + X87_ST0:			ST(0).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST1:			ST(1).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST2:			ST(2).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST3:			ST(3).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST4:			ST(4).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST5:			ST(5).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST6:			ST(6).f = info->i;						break;
		case CPUINFO_INT_REGISTER + X87_ST7:			ST(7).f = info->i;						break;

		default:										CPU_SET_INFO_CALL(i386);				break;
	}
}

CPU_GET_INFO( mediagx )
{
	i386_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		case CPUINFO_FCT_SET_INFO:	    				info->setinfo = CPU_SET_INFO_NAME(mediagx);	break;
		case CPUINFO_FCT_INIT:		    				info->init = CPU_INIT_NAME(mediagx);	break;
		case CPUINFO_FCT_RESET:		    				info->reset = CPU_RESET_NAME(mediagx);	break;
		case CPUINFO_FCT_EXIT:		    				info->exit = CPU_EXIT_NAME(mediagx);	break;

		case CPUINFO_INT_REGISTER + X87_CTRL:			info->i = cpustate->fpu_control_word;			break;
		case CPUINFO_INT_REGISTER + X87_STATUS:			info->i = cpustate->fpu_status_word;			break;
		case CPUINFO_INT_REGISTER + X87_ST0:			info->i = ST(0).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST1:			info->i = ST(1).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST2:			info->i = ST(2).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST3:			info->i = ST(3).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST4:			info->i = ST(4).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST5:			info->i = ST(5).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST6:			info->i = ST(6).f;						break;
		case CPUINFO_INT_REGISTER + X87_ST7:			info->i = ST(7).f;						break;

		case DEVINFO_STR_NAME:							strcpy(info->s, "MEDIAGX");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Cyrix MediaGX");		break;
		case CPUINFO_STR_REGISTER + X87_CTRL:			sprintf(info->s, "FPU_CW: %04X", cpustate->fpu_control_word); break;
		case CPUINFO_STR_REGISTER + X87_STATUS:			sprintf(info->s, "FPU_SW: %04X", cpustate->fpu_status_word); break;
		case CPUINFO_STR_REGISTER + X87_ST0:			sprintf(info->s, "ST0: %f", ST(0).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST1:			sprintf(info->s, "ST1: %f", ST(1).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST2:			sprintf(info->s, "ST2: %f", ST(2).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST3:			sprintf(info->s, "ST3: %f", ST(3).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST4:			sprintf(info->s, "ST4: %f", ST(4).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST5:			sprintf(info->s, "ST5: %f", ST(5).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST6:			sprintf(info->s, "ST6: %f", ST(6).f);	break;
		case CPUINFO_STR_REGISTER + X87_ST7:			sprintf(info->s, "ST7: %f", ST(7).f);	break;

		default:										CPU_GET_INFO_CALL(i386);				break;
	}
}
