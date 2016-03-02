// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
/*
    Intel 386 emulator

    Written by Ville Linde

    Currently supports:
        Intel 386
        Intel 486
        Intel Pentium
        Cyrix MediaGX
        Intel Pentium MMX
        Intel Pentium Pro
        Intel Pentium II
        Intel Pentium III
        Intel Pentium 4
*/

#include "emu.h"
#include "debugger.h"
#include "i386priv.h"
#include "i386.h"

#include "debug/debugcpu.h"

/* seems to be defined on mingw-gcc */
#undef i386

const device_type I386 = &device_creator<i386_device>;
const device_type I386SX = &device_creator<i386SX_device>;
const device_type I486 = &device_creator<i486_device>;
const device_type PENTIUM = &device_creator<pentium_device>;
const device_type MEDIAGX = &device_creator<mediagx_device>;
const device_type PENTIUM_PRO = &device_creator<pentium_pro_device>;
const device_type PENTIUM_MMX = &device_creator<pentium_mmx_device>;
const device_type PENTIUM2 = &device_creator<pentium2_device>;
const device_type PENTIUM3 = &device_creator<pentium3_device>;
const device_type PENTIUM4 = &device_creator<pentium4_device>;


i386_device::i386_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, I386, "I386", tag, owner, clock, "i386", __FILE__)
	, device_vtlb_interface(mconfig, *this, AS_PROGRAM)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 32, 16, 0)
	, m_smiact(*this)
{
	m_program_config.m_logaddr_width = 32;
	m_program_config.m_page_shift = 12;

	// 32 unified
	set_vtlb_dynamic_entries(32);
}


i386_device::i386_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int program_data_width, int program_addr_width, int io_data_width)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, device_vtlb_interface(mconfig, *this, AS_PROGRAM)
	, m_program_config("program", ENDIANNESS_LITTLE, program_data_width, program_addr_width, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, io_data_width, 16, 0)
	, m_smiact(*this)
{
	m_program_config.m_logaddr_width = 32;
	m_program_config.m_page_shift = 12;

	// 32 unified
	set_vtlb_dynamic_entries(32);
}

i386SX_device::i386SX_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i386_device(mconfig, I386SX, "I386SX", tag, owner, clock, "i386sx", __FILE__, 16, 24, 16)
{
}

i486_device::i486_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i386_device(mconfig, I486, "I486", tag, owner, clock, "i486", __FILE__)
{
}

pentium_device::pentium_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i386_device(mconfig, PENTIUM, "PENTIUM", tag, owner, clock, "pentium", __FILE__)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb
	set_vtlb_dynamic_entries(96);
}

pentium_device::pentium_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: i386_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb
	set_vtlb_dynamic_entries(96);
}

mediagx_device::mediagx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i386_device(mconfig, MEDIAGX, "MEDIAGX", tag, owner, clock, "mediagx", __FILE__)
{
}

pentium_pro_device::pentium_pro_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pentium_device(mconfig, PENTIUM_PRO, "Pentium Pro", tag, owner, clock, "pentium_pro", __FILE__)
{
}

pentium_mmx_device::pentium_mmx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pentium_device(mconfig, PENTIUM_MMX, "Pentium MMX", tag, owner, clock, "pentium_mmx", __FILE__)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	set_vtlb_dynamic_entries(96);
}

pentium2_device::pentium2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pentium_device(mconfig, PENTIUM2, "Pentium II", tag, owner, clock, "pentium2", __FILE__)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	set_vtlb_dynamic_entries(96);
}

pentium3_device::pentium3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pentium_device(mconfig, PENTIUM3, "Pentium III", tag, owner, clock, "pentium3", __FILE__)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	set_vtlb_dynamic_entries(96);
}

pentium4_device::pentium4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pentium_device(mconfig, PENTIUM4, "Pentium 4", tag, owner, clock, "pentium4", __FILE__)
{
	// 128 dtlb, 64 itlb
	set_vtlb_dynamic_entries(196);
}


int i386_parity_table[256];
MODRM_TABLE i386_MODRM_table[256];

#define FAULT(fault,error) {m_ext = 1; i386_trap_with_error(fault,0,0,error); return;}
#define FAULT_EXP(fault,error) {m_ext = 1; i386_trap_with_error(fault,0,trap_level+1,error); return;}

/*************************************************************************/

UINT32 i386_device::i386_load_protected_mode_segment(I386_SREG *seg, UINT64 *desc )
{
	UINT32 v1,v2;
	UINT32 base, limit;
	int entry;

	if(!seg->selector)
	{
		seg->flags = 0;
		seg->base = 0;
		seg->limit = 0;
		seg->d = 0;
		seg->valid = false;
		return 0;
	}

	if ( seg->selector & 0x4 )
	{
		base = m_ldtr.base;
		limit = m_ldtr.limit;
	} else {
		base = m_gdtr.base;
		limit = m_gdtr.limit;
	}

	entry = seg->selector & ~0x7;
	if (limit == 0 || entry + 7 > limit)
		return 0;

	v1 = READ32PL0(base + entry );
	v2 = READ32PL0(base + entry + 4 );

	seg->flags = (v2 >> 8) & 0xf0ff;
	seg->base = (v2 & 0xff000000) | ((v2 & 0xff) << 16) | ((v1 >> 16) & 0xffff);
	seg->limit = (v2 & 0xf0000) | (v1 & 0xffff);
	if (seg->flags & 0x8000)
		seg->limit = (seg->limit << 12) | 0xfff;
	seg->d = (seg->flags & 0x4000) ? 1 : 0;
	seg->valid = true;

	if(desc)
		*desc = ((UINT64)v2<<32)|v1;
	return 1;
}

void i386_device::i386_load_call_gate(I386_CALL_GATE *gate)
{
	UINT32 v1,v2;
	UINT32 base,limit;
	int entry;

	if ( gate->segment & 0x4 )
	{
		base = m_ldtr.base;
		limit = m_ldtr.limit;
	} else {
		base = m_gdtr.base;
		limit = m_gdtr.limit;
	}

	entry = gate->segment & ~0x7;
	if (limit == 0 || entry + 7 > limit)
		return;

	v1 = READ32PL0(base + entry );
	v2 = READ32PL0(base + entry + 4 );

	/* Note that for task gates, offset and dword_count are not used */
	gate->selector = (v1 >> 16) & 0xffff;
	gate->offset = (v1 & 0x0000ffff) | (v2 & 0xffff0000);
	gate->ar = (v2 >> 8) & 0xff;
	gate->dword_count = v2 & 0x001f;
	gate->present = (gate->ar >> 7) & 0x01;
	gate->dpl = (gate->ar >> 5) & 0x03;
}

void i386_device::i386_set_descriptor_accessed(UINT16 selector)
{
	// assume the selector is valid, we don't need to check it again
	UINT32 base, addr;
	UINT8 rights;
	if(!(selector & ~3))
		return;

	if ( selector & 0x4 )
		base = m_ldtr.base;
	else
		base = m_gdtr.base;

	addr = base + (selector & ~7) + 5;
	i386_translate_address(TRANSLATE_READ, &addr, nullptr);
	rights = m_program->read_byte(addr);
	// Should a fault be thrown if the table is read only?
	m_program->write_byte(addr, rights | 1);
}

void i386_device::i386_load_segment_descriptor(int segment )
{
	if (PROTECTED_MODE)
	{
		if (!V8086_MODE)
		{
			i386_load_protected_mode_segment(&m_sreg[segment], nullptr );
			if(m_sreg[segment].selector)
				i386_set_descriptor_accessed(m_sreg[segment].selector);
		}
		else
		{
			m_sreg[segment].base = m_sreg[segment].selector << 4;
			m_sreg[segment].limit = 0xffff;
			m_sreg[segment].flags = (segment == CS) ? 0x00fb : 0x00f3;
			m_sreg[segment].d = 0;
			m_sreg[segment].valid = true;
		}
	}
	else
	{
		m_sreg[segment].base = m_sreg[segment].selector << 4;
		m_sreg[segment].d = 0;
		m_sreg[segment].valid = true;

		if( segment == CS && !m_performed_intersegment_jump )
			m_sreg[segment].base |= 0xfff00000;
	}
}

/* Retrieves the stack selector located in the current TSS */
UINT32 i386_device::i386_get_stack_segment(UINT8 privilege)
{
	UINT32 ret;
	if(privilege >= 3)
		return 0;

	if(m_task.flags & 8)
		ret = READ32PL0((m_task.base+8) + (8*privilege));
	else
		ret = READ16PL0((m_task.base+4) + (4*privilege));

	return ret;
}

/* Retrieves the stack pointer located in the current TSS */
UINT32 i386_device::i386_get_stack_ptr(UINT8 privilege)
{
	UINT32 ret;
	if(privilege >= 3)
		return 0;

	if(m_task.flags & 8)
		ret = READ32PL0((m_task.base+4) + (8*privilege));
	else
		ret = READ16PL0((m_task.base+2) + (4*privilege));

	return ret;
}

UINT32 i386_device::get_flags() const
{
	UINT32 f = 0x2;
	f |= m_CF;
	f |= m_PF << 2;
	f |= m_AF << 4;
	f |= m_ZF << 6;
	f |= m_SF << 7;
	f |= m_TF << 8;
	f |= m_IF << 9;
	f |= m_DF << 10;
	f |= m_OF << 11;
	f |= m_IOP1 << 12;
	f |= m_IOP2 << 13;
	f |= m_NT << 14;
	f |= m_RF << 16;
	f |= m_VM << 17;
	f |= m_AC << 18;
	f |= m_VIF << 19;
	f |= m_VIP << 20;
	f |= m_ID << 21;
	return (m_eflags & ~m_eflags_mask) | (f & m_eflags_mask);
}

void i386_device::set_flags(UINT32 f )
{
	m_CF = (f & 0x1) ? 1 : 0;
	m_PF = (f & 0x4) ? 1 : 0;
	m_AF = (f & 0x10) ? 1 : 0;
	m_ZF = (f & 0x40) ? 1 : 0;
	m_SF = (f & 0x80) ? 1 : 0;
	m_TF = (f & 0x100) ? 1 : 0;
	m_IF = (f & 0x200) ? 1 : 0;
	m_DF = (f & 0x400) ? 1 : 0;
	m_OF = (f & 0x800) ? 1 : 0;
	m_IOP1 = (f & 0x1000) ? 1 : 0;
	m_IOP2 = (f & 0x2000) ? 1 : 0;
	m_NT = (f & 0x4000) ? 1 : 0;
	m_RF = (f & 0x10000) ? 1 : 0;
	m_VM = (f & 0x20000) ? 1 : 0;
	m_AC = (f & 0x40000) ? 1 : 0;
	m_VIF = (f & 0x80000) ? 1 : 0;
	m_VIP = (f & 0x100000) ? 1 : 0;
	m_ID = (f & 0x200000) ? 1 : 0;
	m_eflags = f & m_eflags_mask;
}

void i386_device::sib_byte(UINT8 mod, UINT32* out_ea, UINT8* out_segment)
{
	UINT32 ea = 0;
	UINT8 segment = 0;
	UINT8 scale, i, base;
	UINT8 sib = FETCH();
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
				ea = FETCH32();
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

void i386_device::modrm_to_EA(UINT8 mod_rm, UINT32* out_ea, UINT8* out_segment)
{
	INT8 disp8;
	INT16 disp16;
	INT32 disp32;
	UINT8 mod = (mod_rm >> 6) & 0x3;
	UINT8 rm = mod_rm & 0x7;
	UINT32 ea;
	UINT8 segment;

	if( mod_rm >= 0xc0 )
		fatalerror("i386: Called modrm_to_EA with modrm value %02X!\n",mod_rm);


	if( m_address_size ) {
		switch( rm )
		{
			default:
			case 0: ea = REG32(EAX); segment = DS; break;
			case 1: ea = REG32(ECX); segment = DS; break;
			case 2: ea = REG32(EDX); segment = DS; break;
			case 3: ea = REG32(EBX); segment = DS; break;
			case 4: sib_byte(mod, &ea, &segment ); break;
			case 5:
				if( mod == 0 ) {
					ea = FETCH32(); segment = DS;
				} else {
					ea = REG32(EBP); segment = SS;
				}
				break;
			case 6: ea = REG32(ESI); segment = DS; break;
			case 7: ea = REG32(EDI); segment = DS; break;
		}
		if( mod == 1 ) {
			disp8 = FETCH();
			ea += (INT32)disp8;
		} else if( mod == 2 ) {
			disp32 = FETCH32();
			ea += disp32;
		}

		if( m_segment_prefix )
			segment = m_segment_override;

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
					ea = FETCH16(); segment = DS;
				} else {
					ea = REG16(BP); segment = SS;
				}
				break;
			case 7: ea = REG16(BX); segment = DS; break;
		}
		if( mod == 1 ) {
			disp8 = FETCH();
			ea += (INT32)disp8;
		} else if( mod == 2 ) {
			disp16 = FETCH16();
			ea += (INT32)disp16;
		}

		if( m_segment_prefix )
			segment = m_segment_override;

		*out_ea = ea & 0xffff;
		*out_segment = segment;
	}
}

UINT32 i386_device::GetNonTranslatedEA(UINT8 modrm,UINT8 *seg)
{
	UINT8 segment;
	UINT32 ea;
	modrm_to_EA(modrm, &ea, &segment );
	if(seg) *seg = segment;
	return ea;
}

UINT32 i386_device::GetEA(UINT8 modrm, int rwn)
{
	UINT8 segment;
	UINT32 ea;
	modrm_to_EA(modrm, &ea, &segment );
	return i386_translate(segment, ea, rwn );
}

/* Check segment register for validity when changing privilege level after an RETF */
void i386_device::i386_check_sreg_validity(int reg)
{
	UINT16 selector = m_sreg[reg].selector;
	UINT8 CPL = m_CPL;
	UINT8 DPL,RPL;
	I386_SREG desc;
	int invalid;

	memset(&desc, 0, sizeof(desc));
	desc.selector = selector;
	i386_load_protected_mode_segment(&desc,nullptr);
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = selector & 0x03;

	/* Must be within the relevant descriptor table limits */
	if(selector & 0x04)
	{
		if((selector & ~0x07) > m_ldtr.limit)
			invalid = 1;
	}
	else
	{
		if((selector & ~0x07) > m_gdtr.limit)
			invalid = 1;
	}

	/* Must be either a data or readable code segment */
	if(((desc.flags & 0x0018) == 0x0018 && (desc.flags & 0x0002)) || (desc.flags & 0x0018) == 0x0010)
		invalid = 0;
	else
		invalid = 1;

	/* If a data segment or non-conforming code segment, then either DPL >= CPL or DPL >= RPL */
	if(((desc.flags & 0x0018) == 0x0018 && (desc.flags & 0x0004) == 0) || (desc.flags & 0x0018) == 0x0010)
	{
		if((DPL < CPL) || (DPL < RPL))
			invalid = 1;
	}

	/* if segment is invalid, then segment register is nulled */
	if(invalid != 0)
	{
		m_sreg[reg].selector = 0;
		i386_load_segment_descriptor(reg);
	}
}

int i386_device::i386_limit_check(int seg, UINT32 offset)
{
	if(PROTECTED_MODE && !V8086_MODE)
	{
		if((m_sreg[seg].flags & 0x0018) == 0x0010 && m_sreg[seg].flags & 0x0004) // if expand-down data segment
		{
			// compare if greater then 0xffffffff when we're passed the access size
			if((offset <= m_sreg[seg].limit) || ((m_sreg[seg].d)?0:(offset > 0xffff)))
			{
				logerror("Limit check at 0x%08x failed. Segment %04x, limit %08x, offset %08x (expand-down)\n",m_pc,m_sreg[seg].selector,m_sreg[seg].limit,offset);
				return 1;
			}
		}
		else
		{
			if(offset > m_sreg[seg].limit)
			{
				logerror("Limit check at 0x%08x failed. Segment %04x, limit %08x, offset %08x\n",m_pc,m_sreg[seg].selector,m_sreg[seg].limit,offset);
				return 1;
			}
		}
	}
	return 0;
}

void i386_device::i386_sreg_load(UINT16 selector, UINT8 reg, bool *fault)
{
	// Checks done when MOV changes a segment register in protected mode
	UINT8 CPL,RPL,DPL;

	CPL = m_CPL;
	RPL = selector & 0x0003;

	if(!PROTECTED_MODE || V8086_MODE)
	{
		m_sreg[reg].selector = selector;
		i386_load_segment_descriptor(reg);
		if(fault) *fault = false;
		return;
	}

	if(fault) *fault = true;
	if(reg == SS)
	{
		I386_SREG stack;

		memset(&stack, 0, sizeof(stack));
		stack.selector = selector;
		i386_load_protected_mode_segment(&stack,nullptr);
		DPL = (stack.flags >> 5) & 0x03;

		if((selector & ~0x0003) == 0)
		{
			logerror("SReg Load (%08x): Selector is null.\n",m_pc);
			FAULT(FAULT_GP,0)
		}
		if(selector & 0x0004)  // LDT
		{
			if((selector & ~0x0007) > m_ldtr.limit)
			{
				logerror("SReg Load (%08x): Selector is out of LDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		else  // GDT
		{
			if((selector & ~0x0007) > m_gdtr.limit)
			{
				logerror("SReg Load (%08x): Selector is out of GDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if (RPL != CPL)
		{
			logerror("SReg Load (%08x): Selector RPL does not equal CPL.\n",m_pc);
			FAULT(FAULT_GP,selector & ~0x03)
		}
		if(((stack.flags & 0x0018) != 0x10) && (stack.flags & 0x0002) != 0)
		{
			logerror("SReg Load (%08x): Segment is not a writable data segment.\n",m_pc);
			FAULT(FAULT_GP,selector & ~0x03)
		}
		if(DPL != CPL)
		{
			logerror("SReg Load (%08x): Segment DPL does not equal CPL.\n",m_pc);
			FAULT(FAULT_GP,selector & ~0x03)
		}
		if(!(stack.flags & 0x0080))
		{
			logerror("SReg Load (%08x): Segment is not present.\n",m_pc);
			FAULT(FAULT_SS,selector & ~0x03)
		}
	}
	if(reg == DS || reg == ES || reg == FS || reg == GS)
	{
		I386_SREG desc;

		if((selector & ~0x0003) == 0)
		{
			m_sreg[reg].selector = selector;
			i386_load_segment_descriptor(reg );
			if(fault) *fault = false;
			return;
		}

		memset(&desc, 0, sizeof(desc));
		desc.selector = selector;
		i386_load_protected_mode_segment(&desc,nullptr);
		DPL = (desc.flags >> 5) & 0x03;

		if(selector & 0x0004)  // LDT
		{
			if((selector & ~0x0007) > m_ldtr.limit)
			{
				logerror("SReg Load (%08x): Selector is out of LDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		else  // GDT
		{
			if((selector & ~0x0007) > m_gdtr.limit)
			{
				logerror("SReg Load (%08x): Selector is out of GDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if((desc.flags & 0x0018) != 0x10)
		{
			if((((desc.flags & 0x0002) != 0) && ((desc.flags & 0x0018) != 0x18)) || !(desc.flags & 0x10))
			{
				logerror("SReg Load (%08x): Segment is not a data segment or readable code segment.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if(((desc.flags & 0x0018) == 0x10) || ((!(desc.flags & 0x0004)) && ((desc.flags & 0x0018) == 0x18)))
		{
			// if data or non-conforming code segment
			if((RPL > DPL) || (CPL > DPL))
			{
				logerror("SReg Load (%08x): Selector RPL or CPL is not less or equal to segment DPL.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if(!(desc.flags & 0x0080))
		{
			logerror("SReg Load (%08x): Segment is not present.\n",m_pc);
			FAULT(FAULT_NP,selector & ~0x03)
		}
	}

	m_sreg[reg].selector = selector;
	i386_load_segment_descriptor(reg );
	if(fault) *fault = false;
}

void i386_device::i386_trap(int irq, int irq_gate, int trap_level)
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
	UINT32 offset, oldflags = get_flags();
	UINT16 segment;
	int entry = irq * (PROTECTED_MODE ? 8 : 4);
	int SetRPL;
	m_lock = false;

	if( !(PROTECTED_MODE) )
	{
		/* 16-bit */
		PUSH16(oldflags & 0xffff );
		PUSH16(m_sreg[CS].selector );
		if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
			PUSH16(m_eip );
		else
			PUSH16(m_prev_eip );

		m_sreg[CS].selector = READ16(m_idtr.base + entry + 2 );
		m_eip = READ16(m_idtr.base + entry );

		m_TF = 0;
		m_IF = 0;
	}
	else
	{
		int type;
		UINT16 flags;
		I386_SREG desc;
		UINT8 CPL = m_CPL, DPL; //, RPL = 0;

		/* 32-bit */
		v1 = READ32PL0(m_idtr.base + entry );
		v2 = READ32PL0(m_idtr.base + entry + 4 );
		offset = (v2 & 0xffff0000) | (v1 & 0xffff);
		segment = (v1 >> 16) & 0xffff;
		type = (v2>>8) & 0x1F;
		flags = (v2>>8) & 0xf0ff;

		if(trap_level == 2)
		{
			logerror("IRQ: Double fault.\n");
			FAULT_EXP(FAULT_DF,0);
		}
		if(trap_level >= 3)
		{
			logerror("IRQ: Triple fault. CPU reset.\n");
			set_input_line(INPUT_LINE_RESET, PULSE_LINE);
			return;
		}

		/* segment privilege checks */
		if(entry >= m_idtr.limit)
		{
			logerror("IRQ (%08x): Vector %02xh is past IDT limit.\n",m_pc,entry);
			FAULT_EXP(FAULT_GP,entry+2)
		}
		/* segment must be interrupt gate, trap gate, or task gate */
		if(type != 0x05 && type != 0x06 && type != 0x07 && type != 0x0e && type != 0x0f)
		{
			logerror("IRQ#%02x (%08x): Vector segment %04x is not an interrupt, trap or task gate.\n",irq,m_pc,segment);
			FAULT_EXP(FAULT_GP,entry+2)
		}

		if(m_ext == 0) // if software interrupt (caused by INT/INTO/INT3)
		{
			if(((flags >> 5) & 0x03) < CPL)
			{
				logerror("IRQ (%08x): Software IRQ - gate DPL is less than CPL.\n",m_pc);
				FAULT_EXP(FAULT_GP,entry+2)
			}
			if(V8086_MODE)
			{
				if((!m_IOP1 || !m_IOP2) && (m_opcode != 0xcc))
				{
					logerror("IRQ (%08x): Is in Virtual 8086 mode and IOPL != 3.\n",m_pc);
					FAULT(FAULT_GP,0)
				}

			}
		}

		if((flags & 0x0080) == 0)
		{
			logerror("IRQ: Vector segment is not present.\n");
			FAULT_EXP(FAULT_NP,entry+2)
		}

		if(type == 0x05)
		{
			/* Task gate */
			memset(&desc, 0, sizeof(desc));
			desc.selector = segment;
			i386_load_protected_mode_segment(&desc,nullptr);
			if(segment & 0x04)
			{
				logerror("IRQ: Task gate: TSS is not in the GDT.\n");
				FAULT_EXP(FAULT_TS,segment & ~0x03);
			}
			else
			{
				if(segment > m_gdtr.limit)
				{
					logerror("IRQ: Task gate: TSS is past GDT limit.\n");
					FAULT_EXP(FAULT_TS,segment & ~0x03);
				}
			}
			if((desc.flags & 0x000f) != 0x09 && (desc.flags & 0x000f) != 0x01)
			{
				logerror("IRQ: Task gate: TSS is not an available TSS.\n");
				FAULT_EXP(FAULT_TS,segment & ~0x03);
			}
			if((desc.flags & 0x0080) == 0)
			{
				logerror("IRQ: Task gate: TSS is not present.\n");
				FAULT_EXP(FAULT_NP,segment & ~0x03);
			}
			if(!(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1))
				m_eip = m_prev_eip;
			if(desc.flags & 0x08)
				i386_task_switch(desc.selector,1);
			else
				i286_task_switch(desc.selector,1);
			return;
		}
		else
		{
			/* Interrupt or Trap gate */
			memset(&desc, 0, sizeof(desc));
			desc.selector = segment;
			i386_load_protected_mode_segment(&desc,nullptr);
			CPL = m_CPL;  // current privilege level
			DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
//          RPL = segment & 0x03;  // requested privilege level

			if((segment & ~0x03) == 0)
			{
				logerror("IRQ: Gate segment is null.\n");
				FAULT_EXP(FAULT_GP,m_ext)
			}
			if(segment & 0x04)
			{
				if((segment & ~0x07) > m_ldtr.limit)
				{
					logerror("IRQ: Gate segment is past LDT limit.\n");
					FAULT_EXP(FAULT_GP,(segment & 0x03)+m_ext)
				}
			}
			else
			{
				if((segment & ~0x07) > m_gdtr.limit)
				{
					logerror("IRQ: Gate segment is past GDT limit.\n");
					FAULT_EXP(FAULT_GP,(segment & 0x03)+m_ext)
				}
			}
			if((desc.flags & 0x0018) != 0x18)
			{
				logerror("IRQ: Gate descriptor is not a code segment.\n");
				FAULT_EXP(FAULT_GP,(segment & 0x03)+m_ext)
			}
			if((desc.flags & 0x0080) == 0)
			{
				logerror("IRQ: Gate segment is not present.\n");
				FAULT_EXP(FAULT_NP,(segment & 0x03)+m_ext)
			}
			if((desc.flags & 0x0004) == 0 && (DPL < CPL))
			{
				/* IRQ to inner privilege */
				I386_SREG stack;
				UINT32 newESP,oldSS,oldESP;

				if(V8086_MODE && DPL)
				{
					logerror("IRQ: Gate to CPL>0 from VM86 mode.\n");
					FAULT_EXP(FAULT_GP,segment & ~0x03);
				}
				/* Check new stack segment in TSS */
				memset(&stack, 0, sizeof(stack));
				stack.selector = i386_get_stack_segment(DPL);
				i386_load_protected_mode_segment(&stack,nullptr);
				oldSS = m_sreg[SS].selector;
				if(flags & 0x0008)
					oldESP = REG32(ESP);
				else
					oldESP = REG16(SP);
				if((stack.selector & ~0x03) == 0)
				{
					logerror("IRQ: New stack selector is null.\n");
					FAULT_EXP(FAULT_GP,m_ext)
				}
				if(stack.selector & 0x04)
				{
					if((stack.selector & ~0x07) > m_ldtr.base)
					{
						logerror("IRQ: New stack selector is past LDT limit.\n");
						FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
					}
				}
				else
				{
					if((stack.selector & ~0x07) > m_gdtr.base)
					{
						logerror("IRQ: New stack selector is past GDT limit.\n");
						FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
					}
				}
				if((stack.selector & 0x03) != DPL)
				{
					logerror("IRQ: New stack selector RPL is not equal to code segment DPL.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
				}
				if(((stack.flags >> 5) & 0x03) != DPL)
				{
					logerror("IRQ: New stack segment DPL is not equal to code segment DPL.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
				}
				if(((stack.flags & 0x0018) != 0x10) && (stack.flags & 0x0002) != 0)
				{
					logerror("IRQ: New stack segment is not a writable data segment.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext) // #TS(stack selector + EXT)
				}
				if((stack.flags & 0x0080) == 0)
				{
					logerror("IRQ: New stack segment is not present.\n");
					FAULT_EXP(FAULT_SS,(stack.selector & ~0x03)+m_ext) // #TS(stack selector + EXT)
				}
				newESP = i386_get_stack_ptr(DPL);
				if(type & 0x08) // 32-bit gate
				{
					if(((newESP < (V8086_MODE?36:20)) && !(stack.flags & 0x4)) || ((~stack.limit < (~(newESP - 1) + (V8086_MODE?36:20))) && (stack.flags & 0x4)))
					{
						logerror("IRQ: New stack has no space for return addresses.\n");
						FAULT_EXP(FAULT_SS,0)
					}
				}
				else // 16-bit gate
				{
					newESP &= 0xffff;
					if(((newESP < (V8086_MODE?18:10)) && !(stack.flags & 0x4)) || ((~stack.limit < (~(newESP - 1) + (V8086_MODE?18:10))) && (stack.flags & 0x4)))
					{
						logerror("IRQ: New stack has no space for return addresses.\n");
						FAULT_EXP(FAULT_SS,0)
					}
				}
				if(offset > desc.limit)
				{
					logerror("IRQ: New EIP is past code segment limit.\n");
					FAULT_EXP(FAULT_GP,0)
				}
				/* change CPL before accessing the stack */
				m_CPL = DPL;
				/* check for page fault at new stack TODO: check if stack frame crosses page boundary */
				WRITE_TEST(stack.base+newESP-1);
				/* Load new stack segment descriptor */
				m_sreg[SS].selector = stack.selector;
				i386_load_protected_mode_segment(&m_sreg[SS],nullptr);
				i386_set_descriptor_accessed(stack.selector);
				REG32(ESP) = newESP;
				if(V8086_MODE)
				{
					//logerror("IRQ (%08x): Interrupt during V8086 task\n",m_pc);
					if(type & 0x08)
					{
						PUSH32SEG(m_sreg[GS].selector & 0xffff);
						PUSH32SEG(m_sreg[FS].selector & 0xffff);
						PUSH32SEG(m_sreg[DS].selector & 0xffff);
						PUSH32SEG(m_sreg[ES].selector & 0xffff);
					}
					else
					{
						PUSH16(m_sreg[GS].selector);
						PUSH16(m_sreg[FS].selector);
						PUSH16(m_sreg[DS].selector);
						PUSH16(m_sreg[ES].selector);
					}
					m_sreg[GS].selector = 0;
					m_sreg[FS].selector = 0;
					m_sreg[DS].selector = 0;
					m_sreg[ES].selector = 0;
					m_VM = 0;
					i386_load_segment_descriptor(GS);
					i386_load_segment_descriptor(FS);
					i386_load_segment_descriptor(DS);
					i386_load_segment_descriptor(ES);
				}
				if(type & 0x08)
				{
					// 32-bit gate
					PUSH32SEG(oldSS);
					PUSH32(oldESP);
				}
				else
				{
					// 16-bit gate
					PUSH16(oldSS);
					PUSH16(oldESP);
				}
				SetRPL = 1;
			}
			else
			{
				int stack_limit;
				if((desc.flags & 0x0004) || (DPL == CPL))
				{
					/* IRQ to same privilege */
					if(V8086_MODE && !m_ext)
					{
						logerror("IRQ: Gate to same privilege from VM86 mode.\n");
						FAULT_EXP(FAULT_GP,segment & ~0x03);
					}
					if(type == 0x0e || type == 0x0f)  // 32-bit gate
						stack_limit = 10;
					else
						stack_limit = 6;
					// TODO: Add check for error code (2 extra bytes)
					if(REG32(ESP) < stack_limit)
					{
						logerror("IRQ: Stack has no space left (needs %i bytes).\n",stack_limit);
						FAULT_EXP(FAULT_SS,0)
					}
					if(offset > desc.limit)
					{
						logerror("IRQ: Gate segment offset is past segment limit.\n");
						FAULT_EXP(FAULT_GP,0)
					}
					SetRPL = 1;
				}
				else
				{
					logerror("IRQ: Gate descriptor is non-conforming, and DPL does not equal CPL.\n");
					FAULT_EXP(FAULT_GP,segment)
				}
			}
		}
		UINT32 tempSP = REG32(ESP);
		try
		{
			// this is ugly but the alternative is worse
			if(type != 0x0e && type != 0x0f)  // if not 386 interrupt or trap gate
			{
				PUSH16(oldflags & 0xffff );
				PUSH16(m_sreg[CS].selector );
				if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
					PUSH16(m_eip );
				else
					PUSH16(m_prev_eip );
			}
			else
			{
				PUSH32(oldflags & 0x00ffffff );
				PUSH32SEG(m_sreg[CS].selector );
				if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
					PUSH32(m_eip );
				else
					PUSH32(m_prev_eip );
			}
		}
		catch(UINT64 e)
		{
			REG32(ESP) = tempSP;
			throw e;
		}
		if(SetRPL != 0)
			segment = (segment & ~0x03) | m_CPL;
		m_sreg[CS].selector = segment;
		m_eip = offset;

		if(type == 0x0e || type == 0x06)
			m_IF = 0;
		m_TF = 0;
		m_NT = 0;
	}

	i386_load_segment_descriptor(CS);
	CHANGE_PC(m_eip);

}

void i386_device::i386_trap_with_error(int irq, int irq_gate, int trap_level, UINT32 error)
{
	i386_trap(irq,irq_gate,trap_level);
	if(irq == 8 || irq == 10 || irq == 11 || irq == 12 || irq == 13 || irq == 14)
	{
		// for these exceptions, an error code is pushed onto the stack by the processor.
		// no error code is pushed for software interrupts, either.
		if(PROTECTED_MODE)
		{
			UINT32 entry = irq * 8;
			UINT32 v2,type;
			v2 = READ32PL0(m_idtr.base + entry + 4 );
			type = (v2>>8) & 0x1F;
			if(type == 5)
			{
				v2 = READ32PL0(m_idtr.base + entry);
				v2 = READ32PL0(m_gdtr.base + ((v2 >> 16) & 0xfff8) + 4);
				type = (v2>>8) & 0x1F;
			}
			if(type >= 9)
				PUSH32(error);
			else
				PUSH16(error);
		}
		else
			PUSH16(error);
	}
}


void i386_device::i286_task_switch(UINT16 selector, UINT8 nested)
{
	UINT32 tss;
	I386_SREG seg;
	UINT16 old_task;
	UINT8 ar_byte;  // access rights byte

	/* TODO: Task State Segment privilege checks */

	/* For tasks that aren't nested, clear the busy bit in the task's descriptor */
	if(nested == 0)
	{
		if(m_task.segment & 0x0004)
		{
			ar_byte = READ8(m_ldtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_ldtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
		else
		{
			ar_byte = READ8(m_gdtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_gdtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
	}

	/* Save the state of the current task in the current TSS (TR register base) */
	tss = m_task.base;
	WRITE16(tss+0x0e,m_eip & 0x0000ffff);
	WRITE16(tss+0x10,get_flags() & 0x0000ffff);
	WRITE16(tss+0x12,REG16(AX));
	WRITE16(tss+0x14,REG16(CX));
	WRITE16(tss+0x16,REG16(DX));
	WRITE16(tss+0x18,REG16(BX));
	WRITE16(tss+0x1a,REG16(SP));
	WRITE16(tss+0x1c,REG16(BP));
	WRITE16(tss+0x1e,REG16(SI));
	WRITE16(tss+0x20,REG16(DI));
	WRITE16(tss+0x22,m_sreg[ES].selector);
	WRITE16(tss+0x24,m_sreg[CS].selector);
	WRITE16(tss+0x26,m_sreg[SS].selector);
	WRITE16(tss+0x28,m_sreg[DS].selector);

	old_task = m_task.segment;

	/* Load task register with the selector of the incoming task */
	m_task.segment = selector;
	memset(&seg, 0, sizeof(seg));
	seg.selector = m_task.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_task.limit = seg.limit;
	m_task.base = seg.base;
	m_task.flags = seg.flags;

	/* Set TS bit in CR0 */
	m_cr[0] |= 0x08;

	/* Load incoming task state from the new task's TSS */
	tss = m_task.base;
	m_ldtr.segment = READ16(tss+0x2a) & 0xffff;
	seg.selector = m_ldtr.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_ldtr.limit = seg.limit;
	m_ldtr.base = seg.base;
	m_ldtr.flags = seg.flags;
	m_eip = READ16(tss+0x0e);
	set_flags(READ16(tss+0x10));
	REG16(AX) = READ16(tss+0x12);
	REG16(CX) = READ16(tss+0x14);
	REG16(DX) = READ16(tss+0x16);
	REG16(BX) = READ16(tss+0x18);
	REG16(SP) = READ16(tss+0x1a);
	REG16(BP) = READ16(tss+0x1c);
	REG16(SI) = READ16(tss+0x1e);
	REG16(DI) = READ16(tss+0x20);
	m_sreg[ES].selector = READ16(tss+0x22) & 0xffff;
	i386_load_segment_descriptor(ES);
	m_sreg[CS].selector = READ16(tss+0x24) & 0xffff;
	i386_load_segment_descriptor(CS);
	m_sreg[SS].selector = READ16(tss+0x26) & 0xffff;
	i386_load_segment_descriptor(SS);
	m_sreg[DS].selector = READ16(tss+0x28) & 0xffff;
	i386_load_segment_descriptor(DS);

	/* Set the busy bit in the new task's descriptor */
	if(selector & 0x0004)
	{
		ar_byte = READ8(m_ldtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_ldtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}
	else
	{
		ar_byte = READ8(m_gdtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_gdtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}

	/* For nested tasks, we write the outgoing task's selector to the back-link field of the new TSS,
	   and set the NT flag in the EFLAGS register */
	if(nested != 0)
	{
		WRITE16(tss+0,old_task);
		m_NT = 1;
	}
	CHANGE_PC(m_eip);

	m_CPL = (m_sreg[SS].flags >> 5) & 3;
//  printf("286 Task Switch from selector %04x to %04x\n",old_task,selector);
}

void i386_device::i386_task_switch(UINT16 selector, UINT8 nested)
{
	UINT32 tss;
	I386_SREG seg;
	UINT16 old_task;
	UINT8 ar_byte;  // access rights byte
	UINT32 oldcr3 = m_cr[3];

	/* TODO: Task State Segment privilege checks */

	/* For tasks that aren't nested, clear the busy bit in the task's descriptor */
	if(nested == 0)
	{
		if(m_task.segment & 0x0004)
		{
			ar_byte = READ8(m_ldtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_ldtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
		else
		{
			ar_byte = READ8(m_gdtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_gdtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
	}

	/* Save the state of the current task in the current TSS (TR register base) */
	tss = m_task.base;
	WRITE32(tss+0x1c,m_cr[3]);  // correct?
	WRITE32(tss+0x20,m_eip);
	WRITE32(tss+0x24,get_flags());
	WRITE32(tss+0x28,REG32(EAX));
	WRITE32(tss+0x2c,REG32(ECX));
	WRITE32(tss+0x30,REG32(EDX));
	WRITE32(tss+0x34,REG32(EBX));
	WRITE32(tss+0x38,REG32(ESP));
	WRITE32(tss+0x3c,REG32(EBP));
	WRITE32(tss+0x40,REG32(ESI));
	WRITE32(tss+0x44,REG32(EDI));
	WRITE32(tss+0x48,m_sreg[ES].selector);
	WRITE32(tss+0x4c,m_sreg[CS].selector);
	WRITE32(tss+0x50,m_sreg[SS].selector);
	WRITE32(tss+0x54,m_sreg[DS].selector);
	WRITE32(tss+0x58,m_sreg[FS].selector);
	WRITE32(tss+0x5c,m_sreg[GS].selector);

	old_task = m_task.segment;

	/* Load task register with the selector of the incoming task */
	m_task.segment = selector;
	memset(&seg, 0, sizeof(seg));
	seg.selector = m_task.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_task.limit = seg.limit;
	m_task.base = seg.base;
	m_task.flags = seg.flags;

	/* Set TS bit in CR0 */
	m_cr[0] |= 0x08;

	/* Load incoming task state from the new task's TSS */
	tss = m_task.base;
	m_ldtr.segment = READ32(tss+0x60) & 0xffff;
	seg.selector = m_ldtr.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_ldtr.limit = seg.limit;
	m_ldtr.base = seg.base;
	m_ldtr.flags = seg.flags;
	m_eip = READ32(tss+0x20);
	set_flags(READ32(tss+0x24));
	REG32(EAX) = READ32(tss+0x28);
	REG32(ECX) = READ32(tss+0x2c);
	REG32(EDX) = READ32(tss+0x30);
	REG32(EBX) = READ32(tss+0x34);
	REG32(ESP) = READ32(tss+0x38);
	REG32(EBP) = READ32(tss+0x3c);
	REG32(ESI) = READ32(tss+0x40);
	REG32(EDI) = READ32(tss+0x44);
	m_sreg[ES].selector = READ32(tss+0x48) & 0xffff;
	i386_load_segment_descriptor(ES);
	m_sreg[CS].selector = READ32(tss+0x4c) & 0xffff;
	i386_load_segment_descriptor(CS);
	m_sreg[SS].selector = READ32(tss+0x50) & 0xffff;
	i386_load_segment_descriptor(SS);
	m_sreg[DS].selector = READ32(tss+0x54) & 0xffff;
	i386_load_segment_descriptor(DS);
	m_sreg[FS].selector = READ32(tss+0x58) & 0xffff;
	i386_load_segment_descriptor(FS);
	m_sreg[GS].selector = READ32(tss+0x5c) & 0xffff;
	i386_load_segment_descriptor(GS);
	/* For nested tasks, we write the outgoing task's selector to the back-link field of the new TSS,
	   and set the NT flag in the EFLAGS register before setting cr3 as the old tss address might be gone */
	if(nested != 0)
	{
		WRITE32(tss+0,old_task);
		m_NT = 1;
	}
	m_cr[3] = READ32(tss+0x1c);  // CR3 (PDBR)
	if(oldcr3 != m_cr[3])
		vtlb_flush_dynamic();

	/* Set the busy bit in the new task's descriptor */
	if(selector & 0x0004)
	{
		ar_byte = READ8(m_ldtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_ldtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}
	else
	{
		ar_byte = READ8(m_gdtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_gdtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}

	CHANGE_PC(m_eip);

	m_CPL = (m_sreg[SS].flags >> 5) & 3;
//  printf("386 Task Switch from selector %04x to %04x\n",old_task,selector);
}

void i386_device::i386_check_irq_line()
{
	if(!m_smm && m_smi)
	{
		pentium_smi();
		return;
	}

	/* Check if the interrupts are enabled */
	if ( (m_irq_state) && m_IF )
	{
		m_cycles -= 2;
		i386_trap(standard_irq_callback(0), 1, 0);
	}
}

void i386_device::i386_protected_mode_jump(UINT16 seg, UINT32 off, int indirect, int operand32)
{
	I386_SREG desc;
	I386_CALL_GATE call_gate;
	UINT8 CPL,DPL,RPL;
	UINT8 SetRPL;
	UINT16 segment = seg;
	UINT32 offset = off;

	/* Check selector is not null */
	if((segment & ~0x03) == 0)
	{
		logerror("JMP: Segment is null.\n");
		FAULT(FAULT_GP,0)
	}
	/* Selector is within descriptor table limit */
	if((segment & 0x04) == 0)
	{
		/* check GDT limit */
		if((segment & ~0x07) > (m_gdtr.limit))
		{
			logerror("JMP: Segment is past GDT limit.\n");
			FAULT(FAULT_GP,segment & 0xfffc)
		}
	}
	else
	{
		/* check LDT limit */
		if((segment & ~0x07) > (m_ldtr.limit))
		{
			logerror("JMP: Segment is past LDT limit.\n");
			FAULT(FAULT_GP,segment & 0xfffc)
		}
	}
	/* Determine segment type */
	memset(&desc, 0, sizeof(desc));
	desc.selector = segment;
	i386_load_protected_mode_segment(&desc,nullptr);
	CPL = m_CPL;  // current privilege level
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = segment & 0x03;  // requested privilege level
	if((desc.flags & 0x0018) == 0x0018)
	{
		/* code segment */
		if((desc.flags & 0x0004) == 0)
		{
			/* non-conforming */
			if(RPL > CPL)
			{
				logerror("JMP: RPL %i is less than CPL %i\n",RPL,CPL);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
			if(DPL != CPL)
			{
				logerror("JMP: DPL %i is not equal CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
		}
		else
		{
			/* conforming */
			if(DPL > CPL)
			{
				logerror("JMP: DPL %i is less than CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
		}
		SetRPL = 1;
		if((desc.flags & 0x0080) == 0)
		{
			logerror("JMP: Segment is not present\n");
			FAULT(FAULT_NP,segment & 0xfffc)
		}
		if(offset > desc.limit)
		{
			logerror("JMP: Offset is past segment limit\n");
			FAULT(FAULT_GP,0)
		}
	}
	else
	{
		if((desc.flags & 0x0010) != 0)
		{
			logerror("JMP: Segment is a data segment\n");
			FAULT(FAULT_GP,segment & 0xfffc)  // #GP (cannot execute code in a data segment)
		}
		else
		{
			switch(desc.flags & 0x000f)
			{
			case 0x01:  // 286 Available TSS
			case 0x09:  // 386 Available TSS
				logerror("JMP: Available 386 TSS at %08x\n",m_pc);
				memset(&desc, 0, sizeof(desc));
				desc.selector = segment;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				if(DPL < CPL)
				{
					logerror("JMP: TSS: DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(DPL < RPL)
				{
					logerror("JMP: TSS: DPL %i is less than TSS RPL %i\n",DPL,RPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("JMP: TSS: Segment is not present\n");
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(desc.flags & 0x0008)
					i386_task_switch(desc.selector,0);
				else
					i286_task_switch(desc.selector,0);
				return;
			case 0x04:  // 286 Call Gate
			case 0x0c:  // 386 Call Gate
				//logerror("JMP: Call gate at %08x\n",m_pc);
				SetRPL = 1;
				memset(&call_gate, 0, sizeof(call_gate));
				call_gate.segment = segment;
				i386_load_call_gate(&call_gate);
				DPL = call_gate.dpl;
				if(DPL < CPL)
				{
					logerror("JMP: Call Gate: DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(DPL < RPL)
				{
					logerror("JMP: Call Gate: DPL %i is less than RPL %i\n",DPL,RPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("JMP: Call Gate: Segment is not present\n");
					FAULT(FAULT_NP,segment & 0xfffc)
				}
				/* Now we examine the segment that the call gate refers to */
				if(call_gate.selector == 0)
				{
					logerror("JMP: Call Gate: Gate selector is null\n");
					FAULT(FAULT_GP,0)
				}
				if(call_gate.selector & 0x04)
				{
					if((call_gate.selector & ~0x07) > m_ldtr.limit)
					{
						logerror("JMP: Call Gate: Gate Selector is past LDT segment limit\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				else
				{
					if((call_gate.selector & ~0x07) > m_gdtr.limit)
					{
						logerror("JMP: Call Gate: Gate Selector is past GDT segment limit\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				desc.selector = call_gate.selector;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;
				if((desc.flags & 0x0018) != 0x18)
				{
					logerror("JMP: Call Gate: Gate does not point to a code segment\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				if((desc.flags & 0x0004) == 0)
				{  // non-conforming
					if(DPL != CPL)
					{
						logerror("JMP: Call Gate: Gate DPL does not equal CPL\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				else
				{  // conforming
					if(DPL > CPL)
					{
						logerror("JMP: Call Gate: Gate DPL is greater than CPL\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("JMP: Call Gate: Gate Segment is not present\n");
					FAULT(FAULT_NP,call_gate.selector & 0xfffc)
				}
				if(call_gate.offset > desc.limit)
				{
					logerror("JMP: Call Gate: Gate offset is past Gate segment limit\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				segment = call_gate.selector;
				offset = call_gate.offset;
				break;
			case 0x05:  // Task Gate
				logerror("JMP: Task gate at %08x\n",m_pc);
				memset(&call_gate, 0, sizeof(call_gate));
				call_gate.segment = segment;
				i386_load_call_gate(&call_gate);
				DPL = call_gate.dpl;
				if(DPL < CPL)
				{
					logerror("JMP: Task Gate: Gate DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(DPL < RPL)
				{
					logerror("JMP: Task Gate: Gate DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(call_gate.present == 0)
				{
					logerror("JMP: Task Gate: Gate is not present.\n");
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				/* Check the TSS that the task gate points to */
				desc.selector = call_gate.selector;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				RPL = call_gate.selector & 0x03;  // requested privilege level
				if(call_gate.selector & 0x04)
				{
					logerror("JMP: Task Gate TSS: TSS must be global.\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				else
				{
					if((call_gate.selector & ~0x07) > m_gdtr.limit)
					{
						logerror("JMP: Task Gate TSS: TSS is past GDT limit.\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				if((call_gate.ar & 0x000f) == 0x0009 || (call_gate.ar & 0x000f) == 0x0001)
				{
					logerror("JMP: Task Gate TSS: Segment is not an available TSS.\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				if(call_gate.present == 0)
				{
					logerror("JMP: Task Gate TSS: TSS is not present.\n");
					FAULT(FAULT_NP,call_gate.selector & 0xfffc)
				}
				if(call_gate.ar & 0x08)
					i386_task_switch(call_gate.selector,0);
				else
					i286_task_switch(call_gate.selector,0);
				return;
			default:  // invalid segment type
				logerror("JMP: Invalid segment type (%i) to jump to.\n",desc.flags & 0x000f);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
		}
	}

	if(SetRPL != 0)
		segment = (segment & ~0x03) | m_CPL;
	if(operand32 == 0)
		m_eip = offset & 0x0000ffff;
	else
		m_eip = offset;
	m_sreg[CS].selector = segment;
	m_performed_intersegment_jump = 1;
	i386_load_segment_descriptor(CS);
	CHANGE_PC(m_eip);
}

void i386_device::i386_protected_mode_call(UINT16 seg, UINT32 off, int indirect, int operand32)
{
	I386_SREG desc;
	I386_CALL_GATE gate;
	UINT8 SetRPL;
	UINT8 CPL, DPL, RPL;
	UINT16 selector = seg;
	UINT32 offset = off;
	int x;

	if((selector & ~0x03) == 0)
	{
		logerror("CALL (%08x): Selector is null.\n",m_pc);
		FAULT(FAULT_GP,0)  // #GP(0)
	}
	if(selector & 0x04)
	{
		if((selector & ~0x07) > m_ldtr.limit)
		{
			logerror("CALL: Selector is past LDT limit.\n");
			FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
		}
	}
	else
	{
		if((selector & ~0x07) > m_gdtr.limit)
		{
			logerror("CALL: Selector is past GDT limit.\n");
			FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
		}
	}

	/* Determine segment type */
	memset(&desc, 0, sizeof(desc));
	desc.selector = selector;
	i386_load_protected_mode_segment(&desc,nullptr);
	CPL = m_CPL;  // current privilege level
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = selector & 0x03;  // requested privilege level
	if((desc.flags & 0x0018) == 0x18)  // is a code segment
	{
		if(desc.flags & 0x0004)
		{
			/* conforming */
			if(DPL > CPL)
			{
				logerror("CALL: Code segment DPL %i is greater than CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
			}
		}
		else
		{
			/* non-conforming */
			if(RPL > CPL)
			{
				logerror("CALL: RPL %i is greater than CPL %i\n",RPL,CPL);
				FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
			}
			if(DPL != CPL)
			{
				logerror("CALL: Code segment DPL %i is not equal to CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
			}
		}
		SetRPL = 1;
		if((desc.flags & 0x0080) == 0)
		{
			logerror("CALL (%08x): Code segment is not present.\n",m_pc);
			FAULT(FAULT_NP,selector & ~0x03)  // #NP(selector)
		}
		if (operand32 != 0)  // if 32-bit
		{
			if(i386_limit_check(SS, REG32(ESP) - 8))
			{
				logerror("CALL (%08x): Stack has no room for return address.\n",m_pc);
				FAULT(FAULT_SS,0)  // #SS(0)
			}
		}
		else
		{
			if(i386_limit_check(SS, (REG16(SP) - 4) & 0xffff))
			{
				logerror("CALL (%08x): Stack has no room for return address.\n",m_pc);
				FAULT(FAULT_SS,0)  // #SS(0)
			}
		}
		if(offset > desc.limit)
		{
			logerror("CALL: EIP is past segment limit.\n");
			FAULT(FAULT_GP,0)  // #GP(0)
		}
	}
	else
	{
		/* special segment type */
		if(desc.flags & 0x0010)
		{
			logerror("CALL: Segment is a data segment.\n");
			FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
		}
		else
		{
			switch(desc.flags & 0x000f)
			{
			case 0x01:  // Available 286 TSS
			case 0x09:  // Available 386 TSS
				logerror("CALL: Available TSS at %08x\n",m_pc);
				if(DPL < CPL)
				{
					logerror("CALL: TSS: DPL is less than CPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if(DPL < RPL)
				{
					logerror("CALL: TSS: DPL is less than RPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if(desc.flags & 0x0002)
				{
					logerror("CALL: TSS: TSS is busy.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("CALL: TSS: Segment %02x is not present.\n",selector);
					FAULT(FAULT_NP,selector & ~0x03) // #NP(selector)
				}
				if(desc.flags & 0x08)
					i386_task_switch(desc.selector,1);
				else
					i286_task_switch(desc.selector,1);
				return;
			case 0x04:  // 286 call gate
			case 0x0c:  // 386 call gate
				if((desc.flags & 0x000f) == 0x04)
					operand32 = 0;
				else
					operand32 = 1;
				memset(&gate, 0, sizeof(gate));
				gate.segment = selector;
				i386_load_call_gate(&gate);
				DPL = gate.dpl;
				//logerror("CALL: Call gate at %08x (%i parameters)\n",m_pc,gate.dword_count);
				if(DPL < CPL)
				{
					logerror("CALL: Call gate DPL %i is less than CPL %i.\n",DPL,CPL);
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				if(DPL < RPL)
				{
					logerror("CALL: Call gate DPL %i is less than RPL %i.\n",DPL,RPL);
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				if(gate.present == 0)
				{
					logerror("CALL: Call gate is not present.\n");
					FAULT(FAULT_NP,desc.selector & ~0x03)  // #GP(selector)
				}
				desc.selector = gate.selector;
				if((gate.selector & ~0x03) == 0)
				{
					logerror("CALL: Call gate: Segment is null.\n");
					FAULT(FAULT_GP,0)  // #GP(0)
				}
				if(desc.selector & 0x04)
				{
					if((desc.selector & ~0x07) > m_ldtr.limit)
					{
						logerror("CALL: Call gate: Segment is past LDT limit\n");
						FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
					}
				}
				else
				{
					if((desc.selector & ~0x07) > m_gdtr.limit)
					{
						logerror("CALL: Call gate: Segment is past GDT limit\n");
						FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
					}
				}
				i386_load_protected_mode_segment(&desc,nullptr);
				if((desc.flags & 0x0018) != 0x18)
				{
					logerror("CALL: Call gate: Segment is not a code segment.\n");
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				DPL = ((desc.flags >> 5) & 0x03);
				if(DPL > CPL)
				{
					logerror("CALL: Call gate: Segment DPL %i is greater than CPL %i.\n",DPL,CPL);
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("CALL (%08x): Code segment is not present.\n",m_pc);
					FAULT(FAULT_NP,desc.selector & ~0x03)  // #NP(selector)
				}
				if(DPL < CPL && (desc.flags & 0x0004) == 0)
				{
					I386_SREG stack;
					I386_SREG temp;
					UINT32 oldSS,oldESP;
					/* more privilege */
					/* Check new SS segment for privilege level from TSS */
					memset(&stack, 0, sizeof(stack));
					stack.selector = i386_get_stack_segment(DPL);
					i386_load_protected_mode_segment(&stack,nullptr);
					if((stack.selector & ~0x03) == 0)
					{
						logerror("CALL: Call gate: TSS selector is null\n");
						FAULT(FAULT_TS,0)  // #TS(0)
					}
					if(stack.selector & 0x04)
					{
						if((stack.selector & ~0x07) > m_ldtr.limit)
						{
							logerror("CALL: Call gate: TSS selector is past LDT limit\n");
							FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
						}
					}
					else
					{
						if((stack.selector & ~0x07) > m_gdtr.limit)
						{
							logerror("CALL: Call gate: TSS selector is past GDT limit\n");
							FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
						}
					}
					if((stack.selector & 0x03) != DPL)
					{
						logerror("CALL: Call gate: Stack selector RPL does not equal code segment DPL %i\n",DPL);
						FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
					}
					if(((stack.flags >> 5) & 0x03) != DPL)
					{
						logerror("CALL: Call gate: Stack DPL does not equal code segment DPL %i\n",DPL);
						FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
					}
					if((stack.flags & 0x0018) != 0x10 && (stack.flags & 0x0002))
					{
						logerror("CALL: Call gate: Stack segment is not a writable data segment\n");
						FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
					}
					if((stack.flags & 0x0080) == 0)
					{
						logerror("CALL: Call gate: Stack segment is not present\n");
						FAULT(FAULT_SS,stack.selector)  // #SS(SS selector)
					}
					UINT32 newESP = i386_get_stack_ptr(DPL);
					if(!stack.d)
					{
						newESP &= 0xffff;
					}
					if(operand32 != 0)
					{
						if(newESP < ((gate.dword_count & 0x1f) + 16))
						{
							logerror("CALL: Call gate: New stack has no room for 32-bit return address and parameters.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						if(gate.offset > desc.limit)
						{
							logerror("CALL: Call gate: EIP is past segment limit.\n");
							FAULT(FAULT_GP,0) // #GP(0)
						}
					}
					else
					{
						if(newESP < ((gate.dword_count & 0x1f) + 8))
						{
							logerror("CALL: Call gate: New stack has no room for 16-bit return address and parameters.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						if((gate.offset & 0xffff) > desc.limit)
						{
							logerror("CALL: Call gate: IP is past segment limit.\n");
							FAULT(FAULT_GP,0) // #GP(0)
						}
					}
					selector = gate.selector;
					offset = gate.offset;

					m_CPL = (stack.flags >> 5) & 0x03;
					/* check for page fault at new stack */
					WRITE_TEST(stack.base+newESP-1);
					/* switch to new stack */
					oldSS = m_sreg[SS].selector;
					m_sreg[SS].selector = i386_get_stack_segment(m_CPL);
					if(operand32 != 0)
					{
						oldESP = REG32(ESP);
					}
					else
					{
						oldESP = REG16(SP);
					}
					i386_load_segment_descriptor(SS );
					REG32(ESP) = newESP;

					if(operand32 != 0)
					{
						PUSH32SEG(oldSS);
						PUSH32(oldESP);
					}
					else
					{
						PUSH16(oldSS);
						PUSH16(oldESP & 0xffff);
					}

					memset(&temp, 0, sizeof(temp));
					temp.selector = oldSS;
					i386_load_protected_mode_segment(&temp,nullptr);
					/* copy parameters from old stack to new stack */
					for(x=(gate.dword_count & 0x1f)-1;x>=0;x--)
					{
						UINT32 addr = oldESP + (operand32?(x*4):(x*2));
						addr = temp.base + (temp.d?addr:(addr&0xffff));
						if(operand32)
							PUSH32(READ32(addr));
						else
							PUSH16(READ16(addr));
					}
					SetRPL = 1;
				}
				else
				{
					/* same privilege */
					if (operand32 != 0)  // if 32-bit
					{
						if(i386_limit_check(SS, REG32(ESP) - 8))
						{
							logerror("CALL: Stack has no room for return address.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						selector = gate.selector;
						offset = gate.offset;
					}
					else
					{
						if(i386_limit_check(SS, (REG16(SP) - 4) & 0xffff))
						{
							logerror("CALL: Stack has no room for return address.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						selector = gate.selector;
						offset = gate.offset & 0xffff;
					}
					if(offset > desc.limit)
					{
						logerror("CALL: EIP is past segment limit.\n");
						FAULT(FAULT_GP,0) // #GP(0)
					}
					SetRPL = 1;
				}
				break;
			case 0x05:  // task gate
				logerror("CALL: Task gate at %08x\n",m_pc);
				memset(&gate, 0, sizeof(gate));
				gate.segment = selector;
				i386_load_call_gate(&gate);
				DPL = gate.dpl;
				if(DPL < CPL)
				{
					logerror("CALL: Task Gate: Gate DPL is less than CPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if(DPL < RPL)
				{
					logerror("CALL: Task Gate: Gate DPL is less than RPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if((gate.ar & 0x0080) == 0)
				{
					logerror("CALL: Task Gate: Gate is not present.\n");
					FAULT(FAULT_NP,selector & ~0x03) // #NP(selector)
				}
				/* Check the TSS that the task gate points to */
				desc.selector = gate.selector;
				i386_load_protected_mode_segment(&desc,nullptr);
				if(gate.selector & 0x04)
				{
					logerror("CALL: Task Gate: TSS is not global.\n");
					FAULT(FAULT_TS,gate.selector & ~0x03) // #TS(selector)
				}
				else
				{
					if((gate.selector & ~0x07) > m_gdtr.limit)
					{
						logerror("CALL: Task Gate: TSS is past GDT limit.\n");
						FAULT(FAULT_TS,gate.selector & ~0x03) // #TS(selector)
					}
				}
				if(desc.flags & 0x0002)
				{
					logerror("CALL: Task Gate: TSS is busy.\n");
					FAULT(FAULT_TS,gate.selector & ~0x03) // #TS(selector)
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("CALL: Task Gate: TSS is not present.\n");
					FAULT(FAULT_NP,gate.selector & ~0x03) // #TS(selector)
				}
				if(desc.flags & 0x08)
					i386_task_switch(desc.selector,1);  // with nesting
				else
					i286_task_switch(desc.selector,1);
				return;
			default:
				logerror("CALL: Invalid special segment type (%i) to jump to.\n",desc.flags & 0x000f);
				FAULT(FAULT_GP,selector & ~0x07)  // #GP(selector)
			}
		}
	}

	if(SetRPL != 0)
		selector = (selector & ~0x03) | m_CPL;

	UINT32 tempSP = REG32(ESP);
	try
	{
		// this is ugly but the alternative is worse
		if(operand32 == 0)
		{
			/* 16-bit operand size */
			PUSH16(m_sreg[CS].selector );
			PUSH16(m_eip & 0x0000ffff );
			m_sreg[CS].selector = selector;
			m_performed_intersegment_jump = 1;
			m_eip = offset;
			i386_load_segment_descriptor(CS);
		}
		else
		{
			/* 32-bit operand size */
			PUSH32SEG(m_sreg[CS].selector );
			PUSH32(m_eip );
			m_sreg[CS].selector = selector;
			m_performed_intersegment_jump = 1;
			m_eip = offset;
			i386_load_segment_descriptor(CS );
		}
	}
	catch(UINT64 e)
	{
		REG32(ESP) = tempSP;
		throw e;
	}

	CHANGE_PC(m_eip);
}

void i386_device::i386_protected_mode_retf(UINT8 count, UINT8 operand32)
{
	UINT32 newCS, newEIP;
	I386_SREG desc;
	UINT8 CPL, RPL, DPL;

	UINT32 ea = i386_translate(SS, (STACK_32BIT)?REG32(ESP):REG16(SP), 0);

	if(operand32 == 0)
	{
		newEIP = READ16(ea) & 0xffff;
		newCS = READ16(ea+2) & 0xffff;
	}
	else
	{
		newEIP = READ32(ea);
		newCS = READ32(ea+4) & 0xffff;
	}

	memset(&desc, 0, sizeof(desc));
	desc.selector = newCS;
	i386_load_protected_mode_segment(&desc,nullptr);
	CPL = m_CPL;  // current privilege level
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = newCS & 0x03;

	if(RPL < CPL)
	{
		logerror("RETF (%08x): Return segment RPL is less than CPL.\n",m_pc);
		FAULT(FAULT_GP,newCS & ~0x03)
	}

	if(RPL == CPL)
	{
		/* same privilege level */
		if((newCS & ~0x03) == 0)
		{
			logerror("RETF: Return segment is null.\n");
			FAULT(FAULT_GP,0)
		}
		if(newCS & 0x04)
		{
			if((newCS & ~0x07) >= m_ldtr.limit)
			{
				logerror("RETF: Return segment is past LDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if((newCS & ~0x07) >= m_gdtr.limit)
			{
				logerror("RETF: Return segment is past GDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0018) != 0x0018)
		{
			logerror("RETF: Return segment is not a code segment.\n");
			FAULT(FAULT_GP,newCS & ~0x03)
		}
		if(desc.flags & 0x0004)
		{
			if(DPL > RPL)
			{
				logerror("RETF: Conforming code segment DPL is greater than CS RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if(DPL != RPL)
			{
				logerror("RETF: Non-conforming code segment DPL does not equal CS RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0080) == 0)
		{
			logerror("RETF (%08x): Code segment is not present.\n",m_pc);
			FAULT(FAULT_NP,newCS & ~0x03)
		}
		if(newEIP > desc.limit)
		{
			logerror("RETF: EIP is past code segment limit.\n");
			FAULT(FAULT_GP,0)
		}
		if(operand32 == 0)
		{
			UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+3) != 0)
			{
				logerror("RETF (%08x): SP is past stack segment limit.\n",m_pc);
				FAULT(FAULT_SS,0)
			}
		}
		else
		{
			UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+7) != 0)
			{
				logerror("RETF: ESP is past stack segment limit.\n");
				FAULT(FAULT_SS,0)
			}
		}
		if(operand32 == 0)
			REG16(SP) += (4+count);
		else
			REG32(ESP) += (8+count);
	}
	else if(RPL > CPL)
	{
		UINT32 newSS, newESP;  // when changing privilege
		/* outer privilege level */
		if(operand32 == 0)
		{
			UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+7) != 0)
			{
				logerror("RETF (%08x): SP is past stack segment limit.\n",m_pc);
				FAULT(FAULT_SS,0)
			}
		}
		else
		{
			UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+15) != 0)
			{
				logerror("RETF: ESP is past stack segment limit.\n");
				FAULT(FAULT_SS,0)
			}
		}
		/* Check CS selector and descriptor */
		if((newCS & ~0x03) == 0)
		{
			logerror("RETF: CS segment is null.\n");
			FAULT(FAULT_GP,0)
		}
		if(newCS & 0x04)
		{
			if((newCS & ~0x07) >= m_ldtr.limit)
			{
				logerror("RETF: CS segment selector is past LDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if((newCS & ~0x07) >= m_gdtr.limit)
			{
				logerror("RETF: CS segment selector is past GDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0018) != 0x0018)
		{
			logerror("RETF: CS segment is not a code segment.\n");
			FAULT(FAULT_GP,newCS & ~0x03)
		}
		if(desc.flags & 0x0004)
		{
			if(DPL > RPL)
			{
				logerror("RETF: Conforming CS segment DPL is greater than return selector RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if(DPL != RPL)
			{
				logerror("RETF: Non-conforming CS segment DPL is not equal to return selector RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0080) == 0)
		{
			logerror("RETF: CS segment is not present.\n");
			FAULT(FAULT_NP,newCS & ~0x03)
		}
		if(newEIP > desc.limit)
		{
			logerror("RETF: EIP is past return CS segment limit.\n");
			FAULT(FAULT_GP,0)
		}

		if(operand32 == 0)
		{
			ea += count+4;
			newESP = READ16(ea) & 0xffff;
			newSS = READ16(ea+2) & 0xffff;
		}
		else
		{
			ea += count+8;
			newESP = READ32(ea);
			newSS = READ32(ea+4) & 0xffff;
		}

		/* Check SS selector and descriptor */
		desc.selector = newSS;
		i386_load_protected_mode_segment(&desc,nullptr);
		DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
		if((newSS & ~0x07) == 0)
		{
			logerror("RETF: SS segment is null.\n");
			FAULT(FAULT_GP,0)
		}
		if(newSS & 0x04)
		{
			if((newSS & ~0x07) > m_ldtr.limit)
			{
				logerror("RETF (%08x): SS segment selector is past LDT limit.\n",m_pc);
				FAULT(FAULT_GP,newSS & ~0x03)
			}
		}
		else
		{
			if((newSS & ~0x07) > m_gdtr.limit)
			{
				logerror("RETF (%08x): SS segment selector is past GDT limit.\n",m_pc);
				FAULT(FAULT_GP,newSS & ~0x03)
			}
		}
		if((newSS & 0x03) != RPL)
		{
			logerror("RETF: SS segment RPL is not equal to CS segment RPL.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		if((desc.flags & 0x0018) != 0x0010 || (desc.flags & 0x0002) == 0)
		{
			logerror("RETF: SS segment is not a writable data segment.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		if(((desc.flags >> 5) & 0x03) != RPL)
		{
			logerror("RETF: SS DPL is not equal to CS segment RPL.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		if((desc.flags & 0x0080) == 0)
		{
			logerror("RETF: SS segment is not present.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		m_CPL = newCS & 0x03;

		/* Load new SS:(E)SP */
		if(operand32 == 0)
			REG16(SP) = (newESP+count) & 0xffff;
		else
			REG32(ESP) = newESP+count;
		m_sreg[SS].selector = newSS;
		i386_load_segment_descriptor(SS );

		/* Check that DS, ES, FS and GS are valid for the new privilege level */
		i386_check_sreg_validity(DS);
		i386_check_sreg_validity(ES);
		i386_check_sreg_validity(FS);
		i386_check_sreg_validity(GS);
	}

	/* Load new CS:(E)IP */
	if(operand32 == 0)
		m_eip = newEIP & 0xffff;
	else
		m_eip = newEIP;
	m_sreg[CS].selector = newCS;
	i386_load_segment_descriptor(CS );
	CHANGE_PC(m_eip);
}

void i386_device::i386_protected_mode_iret(int operand32)
{
	UINT32 newCS, newEIP;
	UINT32 newSS, newESP;  // when changing privilege
	I386_SREG desc,stack;
	UINT8 CPL, RPL, DPL;
	UINT32 newflags;
	UINT8 IOPL = m_IOP1 | (m_IOP2 << 1);

	CPL = m_CPL;
	UINT32 ea = i386_translate(SS, (STACK_32BIT)?REG32(ESP):REG16(SP), 0);
	if(operand32 == 0)
	{
		newEIP = READ16(ea) & 0xffff;
		newCS = READ16(ea+2) & 0xffff;
		newflags = READ16(ea+4) & 0xffff;
	}
	else
	{
		newEIP = READ32(ea);
		newCS = READ32(ea+4) & 0xffff;
		newflags = READ32(ea+8);
	}

	if(V8086_MODE)
	{
		UINT32 oldflags = get_flags();
		if(IOPL != 3)
		{
			logerror("IRET (%08x): Is in Virtual 8086 mode and IOPL != 3.\n",m_pc);
			FAULT(FAULT_GP,0)
		}
		if(operand32 == 0)
		{
			m_eip = newEIP & 0xffff;
			m_sreg[CS].selector = newCS & 0xffff;
			newflags &= ~(3<<12);
			newflags |= (((oldflags>>12)&3)<<12);  // IOPL cannot be changed in V86 mode
			set_flags((newflags & 0xffff) | (oldflags & ~0xffff));
			REG16(SP) += 6;
		}
		else
		{
			m_eip = newEIP;
			m_sreg[CS].selector = newCS & 0xffff;
			newflags &= ~(3<<12);
			newflags |= 0x20000 | (((oldflags>>12)&3)<<12);  // IOPL and VM cannot be changed in V86 mode
			set_flags(newflags);
			REG32(ESP) += 12;
		}
	}
	else if(NESTED_TASK)
	{
		UINT32 task = READ32(m_task.base);
		/* Task Return */
		logerror("IRET (%08x): Nested task return.\n",m_pc);
		/* Check back-link selector in TSS */
		if(task & 0x04)
		{
			logerror("IRET: Task return: Back-linked TSS is not in GDT.\n");
			FAULT(FAULT_TS,task & ~0x03)
		}
		if((task & ~0x07) >= m_gdtr.limit)
		{
			logerror("IRET: Task return: Back-linked TSS is not in GDT.\n");
			FAULT(FAULT_TS,task & ~0x03)
		}
		memset(&desc, 0, sizeof(desc));
		desc.selector = task;
		i386_load_protected_mode_segment(&desc,nullptr);
		if((desc.flags & 0x001f) != 0x000b)
		{
			logerror("IRET (%08x): Task return: Back-linked TSS is not a busy TSS.\n",m_pc);
			FAULT(FAULT_TS,task & ~0x03)
		}
		if((desc.flags & 0x0080) == 0)
		{
			logerror("IRET: Task return: Back-linked TSS is not present.\n");
			FAULT(FAULT_NP,task & ~0x03)
		}
		if(desc.flags & 0x08)
			i386_task_switch(desc.selector,0);
		else
			i286_task_switch(desc.selector,0);
		return;
	}
	else
	{
		if(newflags & 0x00020000) // if returning to virtual 8086 mode
		{
			// 16-bit iret can't reach here
			newESP = READ32(ea+12);
			newSS = READ32(ea+16) & 0xffff;
			/* Return to v86 mode */
			//logerror("IRET (%08x): Returning to Virtual 8086 mode.\n",m_pc);
			if(CPL != 0)
			{
				UINT32 oldflags = get_flags();
				newflags = (newflags & ~0x00003000) | (oldflags & 0x00003000);
				if(CPL > IOPL)
					newflags = (newflags & ~0x200 ) | (oldflags & 0x200);
			}
			set_flags(newflags);
			m_eip = POP32() & 0xffff;  // high 16 bits are ignored
			m_sreg[CS].selector = POP32() & 0xffff;
			POP32();  // already set flags
			newESP = POP32();
			newSS = POP32() & 0xffff;
			m_sreg[ES].selector = POP32() & 0xffff;
			m_sreg[DS].selector = POP32() & 0xffff;
			m_sreg[FS].selector = POP32() & 0xffff;
			m_sreg[GS].selector = POP32() & 0xffff;
			REG32(ESP) = newESP;  // all 32 bits are loaded
			m_sreg[SS].selector = newSS;
			i386_load_segment_descriptor(ES);
			i386_load_segment_descriptor(DS);
			i386_load_segment_descriptor(FS);
			i386_load_segment_descriptor(GS);
			i386_load_segment_descriptor(SS);
			m_CPL = 3;  // Virtual 8086 tasks are always run at CPL 3
		}
		else
		{
			if(operand32 == 0)
			{
				UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
				if(i386_limit_check(SS,offset+3) != 0)
				{
					logerror("IRET: Data on stack is past SS limit.\n");
					FAULT(FAULT_SS,0)
				}
			}
			else
			{
				UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
				if(i386_limit_check(SS,offset+7) != 0)
				{
					logerror("IRET: Data on stack is past SS limit.\n");
					FAULT(FAULT_SS,0)
				}
			}
			RPL = newCS & 0x03;
			if(RPL < CPL)
			{
				logerror("IRET (%08x): Return CS RPL is less than CPL.\n",m_pc);
				FAULT(FAULT_GP,newCS & ~0x03)
			}
			if(RPL == CPL)
			{
				/* return to same privilege level */
				if(operand32 == 0)
				{
					UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+5) != 0)
					{
						logerror("IRET (%08x): Data on stack is past SS limit.\n",m_pc);
						FAULT(FAULT_SS,0)
					}
				}
				else
				{
					UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+11) != 0)
					{
						logerror("IRET (%08x): Data on stack is past SS limit.\n",m_pc);
						FAULT(FAULT_SS,0)
					}
				}
				if((newCS & ~0x03) == 0)
				{
					logerror("IRET: Return CS selector is null.\n");
					FAULT(FAULT_GP,0)
				}
				if(newCS & 0x04)
				{
					if((newCS & ~0x07) >= m_ldtr.limit)
					{
						logerror("IRET: Return CS selector (%04x) is past LDT limit.\n",newCS);
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				else
				{
					if((newCS & ~0x07) >= m_gdtr.limit)
					{
						logerror("IRET: Return CS selector is past GDT limit.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				memset(&desc, 0, sizeof(desc));
				desc.selector = newCS;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				RPL = newCS & 0x03;
				if((desc.flags & 0x0018) != 0x0018)
				{
					logerror("IRET (%08x): Return CS segment is not a code segment.\n",m_pc);
					FAULT(FAULT_GP,newCS & ~0x07)
				}
				if(desc.flags & 0x0004)
				{
					if(DPL > RPL)
					{
						logerror("IRET: Conforming return CS DPL is greater than CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				else
				{
					if(DPL != RPL)
					{
						logerror("IRET: Non-conforming return CS DPL is not equal to CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("IRET: (%08x) Return CS segment is not present.\n", m_pc);
					FAULT(FAULT_NP,newCS & ~0x03)
				}
				if(newEIP > desc.limit)
				{
					logerror("IRET: Return EIP is past return CS limit.\n");
					FAULT(FAULT_GP,0)
				}

				if(CPL != 0)
				{
					UINT32 oldflags = get_flags();
					newflags = (newflags & ~0x00003000) | (oldflags & 0x00003000);
					if(CPL > IOPL)
						newflags = (newflags & ~0x200 ) | (oldflags & 0x200);
				}

				if(operand32 == 0)
				{
					m_eip = newEIP;
					m_sreg[CS].selector = newCS;
					set_flags(newflags);
					REG16(SP) += 6;
				}
				else
				{
					m_eip = newEIP;
					m_sreg[CS].selector = newCS & 0xffff;
					set_flags(newflags);
					REG32(ESP) += 12;
				}
			}
			else if(RPL > CPL)
			{
				/* return to outer privilege level */
				memset(&desc, 0, sizeof(desc));
				desc.selector = newCS;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				RPL = newCS & 0x03;
				if(operand32 == 0)
				{
					UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+9) != 0)
					{
						logerror("IRET: SP is past SS limit.\n");
						FAULT(FAULT_SS,0)
					}
				}
				else
				{
					UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+19) != 0)
					{
						logerror("IRET: ESP is past SS limit.\n");
						FAULT(FAULT_SS,0)
					}
				}
				/* Check CS selector and descriptor */
				if((newCS & ~0x03) == 0)
				{
					logerror("IRET: Return CS selector is null.\n");
					FAULT(FAULT_GP,0)
				}
				if(newCS & 0x04)
				{
					if((newCS & ~0x07) >= m_ldtr.limit)
					{
						logerror("IRET: Return CS selector is past LDT limit.\n");
						FAULT(FAULT_GP,newCS & ~0x03);
					}
				}
				else
				{
					if((newCS & ~0x07) >= m_gdtr.limit)
					{
						logerror("IRET: Return CS selector is past GDT limit.\n");
						FAULT(FAULT_GP,newCS & ~0x03);
					}
				}
				if((desc.flags & 0x0018) != 0x0018)
				{
					logerror("IRET: Return CS segment is not a code segment.\n");
					FAULT(FAULT_GP,newCS & ~0x03)
				}
				if(desc.flags & 0x0004)
				{
					if(DPL > RPL)
					{
						logerror("IRET: Conforming return CS DPL is greater than CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				else
				{
					if(DPL != RPL)
					{
						logerror("IRET: Non-conforming return CS DPL does not equal CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("IRET: Return CS segment is not present.\n");
					FAULT(FAULT_NP,newCS & ~0x03)
				}

				/* Check SS selector and descriptor */
				if(operand32 == 0)
				{
					newESP = READ16(ea+6) & 0xffff;
					newSS = READ16(ea+8) & 0xffff;
				}
				else
				{
					newESP = READ32(ea+12);
					newSS = READ32(ea+16) & 0xffff;
				}
				memset(&stack, 0, sizeof(stack));
				stack.selector = newSS;
				i386_load_protected_mode_segment(&stack,nullptr);
				DPL = (stack.flags >> 5) & 0x03;
				if((newSS & ~0x03) == 0)
				{
					logerror("IRET: Return SS selector is null.\n");
					FAULT(FAULT_GP,0)
				}
				if(newSS & 0x04)
				{
					if((newSS & ~0x07) >= m_ldtr.limit)
					{
						logerror("IRET: Return SS selector is past LDT limit.\n");
						FAULT(FAULT_GP,newSS & ~0x03);
					}
				}
				else
				{
					if((newSS & ~0x07) >= m_gdtr.limit)
					{
						logerror("IRET: Return SS selector is past GDT limit.\n");
						FAULT(FAULT_GP,newSS & ~0x03);
					}
				}
				if((newSS & 0x03) != RPL)
				{
					logerror("IRET: Return SS RPL is not equal to return CS RPL.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if((stack.flags & 0x0018) != 0x0010)
				{
					logerror("IRET: Return SS segment is not a data segment.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if((stack.flags & 0x0002) == 0)
				{
					logerror("IRET: Return SS segment is not writable.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if(DPL != RPL)
				{
					logerror("IRET: Return SS DPL does not equal SS RPL.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if((stack.flags & 0x0080) == 0)
				{
					logerror("IRET: Return SS segment is not present.\n");
					FAULT(FAULT_NP,newSS & ~0x03)
				}
				if(newEIP > desc.limit)
				{
					logerror("IRET: EIP is past return CS limit.\n");
					FAULT(FAULT_GP,0)
				}

//              if(operand32 == 0)
//                  REG16(SP) += 10;
//              else
//                  REG32(ESP) += 20;

				// IOPL can only change if CPL is zero
				if(CPL != 0)
				{
					UINT32 oldflags = get_flags();
					newflags = (newflags & ~0x00003000) | (oldflags & 0x00003000);
					if(CPL > IOPL)
						newflags = (newflags & ~0x200 ) | (oldflags & 0x200);
				}

				if(operand32 == 0)
				{
					m_eip = newEIP & 0xffff;
					m_sreg[CS].selector = newCS;
					set_flags(newflags);
					REG16(SP) = newESP & 0xffff;
					m_sreg[SS].selector = newSS;
				}
				else
				{
					m_eip = newEIP;
					m_sreg[CS].selector = newCS & 0xffff;
					set_flags(newflags);
					REG32(ESP) = newESP;
					m_sreg[SS].selector = newSS & 0xffff;
				}
				m_CPL = newCS & 0x03;
				i386_load_segment_descriptor(SS);

				/* Check that DS, ES, FS and GS are valid for the new privilege level */
				i386_check_sreg_validity(DS);
				i386_check_sreg_validity(ES);
				i386_check_sreg_validity(FS);
				i386_check_sreg_validity(GS);
			}
		}
	}

	i386_load_segment_descriptor(CS);
	CHANGE_PC(m_eip);
}

#include "cycles.h"

#define CYCLES_NUM(x)   (m_cycles -= (x))

void i386_device::CYCLES(int x)
{
	if (PROTECTED_MODE)
	{
		m_cycles -= m_cycle_table_pm[x];
	}
	else
	{
		m_cycles -= m_cycle_table_rm[x];
	}
}

void i386_device::CYCLES_RM(int modrm, int r, int m)
{
	if (modrm >= 0xc0)
	{
		if (PROTECTED_MODE)
		{
			m_cycles -= m_cycle_table_pm[r];
		}
		else
		{
			m_cycles -= m_cycle_table_rm[r];
		}
	}
	else
	{
		if (PROTECTED_MODE)
		{
			m_cycles -= m_cycle_table_pm[m];
		}
		else
		{
			m_cycles -= m_cycle_table_rm[m];
		}
	}
}

void i386_device::build_cycle_table()
{
	int i, j;
	for (j=0; j < X86_NUM_CPUS; j++)
	{
		cycle_table_rm[j] = std::make_unique<UINT8[]>(CYCLES_NUM_OPCODES);
		cycle_table_pm[j] = std::make_unique<UINT8[]>(CYCLES_NUM_OPCODES);

		for (i=0; i < sizeof(x86_cycle_table)/sizeof(X86_CYCLE_TABLE); i++)
		{
			int opcode = x86_cycle_table[i].op;
			cycle_table_rm[j][opcode] = x86_cycle_table[i].cpu_cycles[j][0];
			cycle_table_pm[j][opcode] = x86_cycle_table[i].cpu_cycles[j][1];
		}
	}
}

void i386_device::report_invalid_opcode()
{
#ifndef DEBUG_MISSING_OPCODE
	logerror("i386: Invalid opcode %02X at %08X %s\n", m_opcode, m_pc - 1, m_lock ? "with lock" : "");
#else
	logerror("i386: Invalid opcode");
	for (int a = 0; a < m_opcode_bytes_length; a++)
		logerror(" %02X", m_opcode_bytes[a]);
	logerror(" at %08X\n", m_opcode_pc);
#endif
}

void i386_device::report_invalid_modrm(const char* opcode, UINT8 modrm)
{
#ifndef DEBUG_MISSING_OPCODE
	logerror("i386: Invalid %s modrm %01X at %08X\n", opcode, modrm, m_pc - 2);
#else
	logerror("i386: Invalid %s modrm %01X", opcode, modrm);
	for (int a = 0; a < m_opcode_bytes_length; a++)
		logerror(" %02X", m_opcode_bytes[a]);
	logerror(" at %08X\n", m_opcode_pc);
#endif
	i386_trap(6, 0, 0);
}


#include "i386ops.inc"
#include "i386op16.inc"
#include "i386op32.inc"
#include "i486ops.inc"
#include "pentops.inc"
#include "x87ops.inc"
#include "i386ops.h"

void i386_device::i386_decode_opcode()
{
	m_opcode = FETCH();

	if(m_lock && !m_lock_table[0][m_opcode])
		return i386_invalid();

	if( m_operand_size )
		(this->*m_opcode_table1_32[m_opcode])();
	else
		(this->*m_opcode_table1_16[m_opcode])();
}

/* Two-byte opcode 0f xx */
void i386_device::i386_decode_two_byte()
{
	m_opcode = FETCH();

	if(m_lock && !m_lock_table[1][m_opcode])
		return i386_invalid();

	if( m_operand_size )
		(this->*m_opcode_table2_32[m_opcode])();
	else
		(this->*m_opcode_table2_16[m_opcode])();
}

/* Three-byte opcode 0f 38 xx */
void i386_device::i386_decode_three_byte38()
{
	m_opcode = FETCH();

	if (m_operand_size)
		(this->*m_opcode_table338_32[m_opcode])();
	else
		(this->*m_opcode_table338_16[m_opcode])();
}

/* Three-byte opcode 0f 3a xx */
void i386_device::i386_decode_three_byte3a()
{
	m_opcode = FETCH();

	if (m_operand_size)
		(this->*m_opcode_table33a_32[m_opcode])();
	else
		(this->*m_opcode_table33a_16[m_opcode])();
}

/* Three-byte opcode prefix 66 0f xx */
void i386_device::i386_decode_three_byte66()
{
	m_opcode = FETCH();
	if( m_operand_size )
		(this->*m_opcode_table366_32[m_opcode])();
	else
		(this->*m_opcode_table366_16[m_opcode])();
}

/* Three-byte opcode prefix f2 0f xx */
void i386_device::i386_decode_three_bytef2()
{
	m_opcode = FETCH();
	if( m_operand_size )
		(this->*m_opcode_table3f2_32[m_opcode])();
	else
		(this->*m_opcode_table3f2_16[m_opcode])();
}

/* Three-byte opcode prefix f3 0f */
void i386_device::i386_decode_three_bytef3()
{
	m_opcode = FETCH();
	if( m_operand_size )
		(this->*m_opcode_table3f3_32[m_opcode])();
	else
		(this->*m_opcode_table3f3_16[m_opcode])();
}

/* Four-byte opcode prefix 66 0f 38 xx */
void i386_device::i386_decode_four_byte3866()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table46638_32[m_opcode])();
	else
		(this->*m_opcode_table46638_16[m_opcode])();
}

/* Four-byte opcode prefix 66 0f 3a xx */
void i386_device::i386_decode_four_byte3a66()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4663a_32[m_opcode])();
	else
		(this->*m_opcode_table4663a_16[m_opcode])();
}

/* Four-byte opcode prefix f2 0f 38 xx */
void i386_device::i386_decode_four_byte38f2()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4f238_32[m_opcode])();
	else
		(this->*m_opcode_table4f238_16[m_opcode])();
}

/* Four-byte opcode prefix f2 0f 3a xx */
void i386_device::i386_decode_four_byte3af2()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4f23a_32[m_opcode])();
	else
		(this->*m_opcode_table4f23a_16[m_opcode])();
}

/* Four-byte opcode prefix f3 0f 38 xx */
void i386_device::i386_decode_four_byte38f3()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4f338_32[m_opcode])();
	else
		(this->*m_opcode_table4f338_16[m_opcode])();
}


/*************************************************************************/

UINT8 i386_device::read8_debug(UINT32 ea, UINT8 *data)
{
	UINT32 address = ea;

	if(!i386_translate_address(TRANSLATE_DEBUG_MASK,&address,nullptr))
		return 0;

	address &= m_a20_mask;
	*data = m_program->read_byte(address);
	return 1;
}

UINT32 i386_device::i386_get_debug_desc(I386_SREG *seg)
{
	UINT32 base, limit, address;
	union { UINT8 b[8]; UINT32 w[2]; } data;
	UINT8 ret;
	int entry;

	if ( seg->selector & 0x4 )
	{
		base = m_ldtr.base;
		limit = m_ldtr.limit;
	} else {
		base = m_gdtr.base;
		limit = m_gdtr.limit;
	}

	entry = seg->selector & ~0x7;
	if (limit == 0 || entry + 7 > limit)
		return 0;

	address = entry + base;

	// todo: bigendian
	ret = read8_debug( address+0, &data.b[0] );
	ret += read8_debug( address+1, &data.b[1] );
	ret += read8_debug( address+2, &data.b[2] );
	ret += read8_debug( address+3, &data.b[3] );
	ret += read8_debug( address+4, &data.b[4] );
	ret += read8_debug( address+5, &data.b[5] );
	ret += read8_debug( address+6, &data.b[6] );
	ret += read8_debug( address+7, &data.b[7] );

	if(ret != 8)
		return 0;

	seg->flags = (data.w[1] >> 8) & 0xf0ff;
	seg->base = (data.w[1] & 0xff000000) | ((data.w[1] & 0xff) << 16) | ((data.w[0] >> 16) & 0xffff);
	seg->limit = (data.w[1] & 0xf0000) | (data.w[0] & 0xffff);
	if (seg->flags & 0x8000)
		seg->limit = (seg->limit << 12) | 0xfff;
	seg->d = (seg->flags & 0x4000) ? 1 : 0;
	seg->valid = (seg->selector & ~3)?(true):(false);

	return seg->valid;
}

UINT64 i386_device::debug_segbase(symbol_table &table, int params, const UINT64 *param)
{
	UINT32 result;
	I386_SREG seg;

	if(param[0] > 65535)
		return 0;

	if (PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = param[0];
		if(!i386_get_debug_desc(&seg))
			return 0;
		result = seg.base;
	}
	else
	{
		result = param[0] << 4;
	}
	return result;
}

UINT64 i386_device::debug_seglimit(symbol_table &table, int params, const UINT64 *param)
{
	UINT32 result = 0;
	I386_SREG seg;

	if (PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = param[0];
		if(!i386_get_debug_desc(&seg))
			return 0;
		result = seg.limit;
	}
	return result;
}

UINT64 i386_device::debug_segofftovirt(symbol_table &table, int params, const UINT64 *param)
{
	UINT32 result;
	I386_SREG seg;

	if(param[0] > 65535)
		return 0;

	if (PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = param[0];
		if(!i386_get_debug_desc(&seg))
			return 0;
		if((seg.flags & 0x0090) != 0x0090) // not system and present
			return 0;
		if((seg.flags & 0x0018) == 0x0010 && seg.flags & 0x0004) // expand down
		{
			if(param[1] <= seg.limit)
				return 0;
		}
		else
		{
			if(param[1] > seg.limit)
				return 0;
		}
		result = seg.base+param[1];
	}
	else
	{
		if(param[1] > 65535)
			return 0;

		result = (param[0] << 4) + param[1];
	}
	return result;
}

UINT64 i386_device::debug_virttophys(symbol_table &table, int params, const UINT64 *param)
{
	UINT32 result = param[0];

	if(!i386_translate_address(TRANSLATE_DEBUG_MASK,&result,nullptr))
		return 0;
	return result;
}

UINT64 i386_debug_segbase(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	i386_device *i386 = (i386_device *)(ref);
	return i386->debug_segbase(table, params, param);
}

UINT64 i386_debug_seglimit(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	i386_device *i386 = (i386_device *)(ref);
	return i386->debug_seglimit(table, params, param);
}

UINT64 i386_debug_segofftovirt(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	i386_device *i386 = (i386_device *)(ref);
	return i386->debug_segofftovirt(table, params, param);
}

static UINT64 i386_debug_virttophys(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	i386_device *i386 = (i386_device *)(ref);
	return i386->debug_virttophys(table, params, param);
}

void i386_device::device_debug_setup()
{
	debug()->symtable().add("segbase", (void *)this, 1, 1, i386_debug_segbase);
	debug()->symtable().add("seglimit", (void *)this, 1, 1, i386_debug_seglimit);
	debug()->symtable().add("segofftovirt", (void *)this, 2, 2, i386_debug_segofftovirt);
	debug()->symtable().add("virttophys", (void *)this, 1, 1, i386_debug_virttophys);
}

/*************************************************************************/

void i386_device::i386_postload()
{
	int i;
	for (i = 0; i < 6; i++)
		i386_load_segment_descriptor(i);
	CHANGE_PC(m_eip);
}

void i386_device::i386_common_init()
{
	int i, j;
	static const int regs8[8] = {AL,CL,DL,BL,AH,CH,DH,BH};
	static const int regs16[8] = {AX,CX,DX,BX,SP,BP,SI,DI};
	static const int regs32[8] = {EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI};

	assert((sizeof(XMM_REG)/sizeof(double)) == 2);

	build_cycle_table();

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

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);
	m_smi = false;
	m_debugger_temp = 0;
	m_lock = false;

	zero_state();

	save_item(NAME(m_reg.d));
	save_item(NAME(m_sreg[ES].selector));
	save_item(NAME(m_sreg[ES].base));
	save_item(NAME(m_sreg[ES].limit));
	save_item(NAME(m_sreg[ES].flags));
	save_item(NAME(m_sreg[ES].d));
	save_item(NAME(m_sreg[CS].selector));
	save_item(NAME(m_sreg[CS].base));
	save_item(NAME(m_sreg[CS].limit));
	save_item(NAME(m_sreg[CS].flags));
	save_item(NAME(m_sreg[CS].d));
	save_item(NAME(m_sreg[SS].selector));
	save_item(NAME(m_sreg[SS].base));
	save_item(NAME(m_sreg[SS].limit));
	save_item(NAME(m_sreg[SS].flags));
	save_item(NAME(m_sreg[SS].d));
	save_item(NAME(m_sreg[DS].selector));
	save_item(NAME(m_sreg[DS].base));
	save_item(NAME(m_sreg[DS].limit));
	save_item(NAME(m_sreg[DS].flags));
	save_item(NAME(m_sreg[DS].d));
	save_item(NAME(m_sreg[FS].selector));
	save_item(NAME(m_sreg[FS].base));
	save_item(NAME(m_sreg[FS].limit));
	save_item(NAME(m_sreg[FS].flags));
	save_item(NAME(m_sreg[FS].d));
	save_item(NAME(m_sreg[GS].selector));
	save_item(NAME(m_sreg[GS].base));
	save_item(NAME(m_sreg[GS].limit));
	save_item(NAME(m_sreg[GS].flags));
	save_item(NAME(m_sreg[GS].d));
	save_item(NAME(m_eip));
	save_item(NAME(m_prev_eip));
	save_item(NAME(m_CF));
	save_item(NAME(m_DF));
	save_item(NAME(m_SF));
	save_item(NAME(m_OF));
	save_item(NAME(m_ZF));
	save_item(NAME(m_PF));
	save_item(NAME(m_AF));
	save_item(NAME(m_IF));
	save_item(NAME(m_TF));
	save_item(NAME(m_cr));
	save_item(NAME(m_dr));
	save_item(NAME(m_tr));
	save_item(NAME(m_idtr.base));
	save_item(NAME(m_idtr.limit));
	save_item(NAME(m_gdtr.base));
	save_item(NAME(m_gdtr.limit));
	save_item(NAME(m_task.base));
	save_item(NAME(m_task.segment));
	save_item(NAME(m_task.limit));
	save_item(NAME(m_task.flags));
	save_item(NAME(m_ldtr.base));
	save_item(NAME(m_ldtr.segment));
	save_item(NAME(m_ldtr.limit));
	save_item(NAME(m_ldtr.flags));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_performed_intersegment_jump));
	save_item(NAME(m_mxcsr));
	save_item(NAME(m_smm));
	save_item(NAME(m_smi_latched));
	save_item(NAME(m_smi));
	save_item(NAME(m_nmi_masked));
	save_item(NAME(m_nmi_latched));
	save_item(NAME(m_smbase));
	save_item(NAME(m_lock));
	machine().save().register_postload(save_prepost_delegate(FUNC(i386_device::i386_postload), this));

	m_smiact.resolve_safe();

	m_icountptr = &m_cycles;
}

void i386_device::device_start()
{
	i386_common_init();

	build_opcode_table(OP_I386);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_I386].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_I386].get();

	register_state_i386();
}

void i386_device::register_state_i386()
{
	state_add( I386_PC,         "PC", m_pc).formatstr("%08X");
	state_add( I386_EIP,        "EIP", m_eip).callimport().formatstr("%08X");
	state_add( I386_AL,         "~AL", REG8(AL)).formatstr("%02X");
	state_add( I386_AH,         "~AH", REG8(AH)).formatstr("%02X");
	state_add( I386_BL,         "~BL", REG8(BL)).formatstr("%02X");
	state_add( I386_BH,         "~BH", REG8(BH)).formatstr("%02X");
	state_add( I386_CL,         "~CL", REG8(CL)).formatstr("%02X");
	state_add( I386_CH,         "~CH", REG8(CH)).formatstr("%02X");
	state_add( I386_DL,         "~DL", REG8(DL)).formatstr("%02X");
	state_add( I386_DH,         "~DH", REG8(DH)).formatstr("%02X");
	state_add( I386_AX,         "~AX", REG16(AX)).formatstr("%04X");
	state_add( I386_BX,         "~BX", REG16(BX)).formatstr("%04X");
	state_add( I386_CX,         "~CX", REG16(CX)).formatstr("%04X");
	state_add( I386_DX,         "~DX", REG16(DX)).formatstr("%04X");
	state_add( I386_SI,         "~SI", REG16(SI)).formatstr("%04X");
	state_add( I386_DI,         "~DI", REG16(DI)).formatstr("%04X");
	state_add( I386_BP,         "~BP", REG16(BP)).formatstr("%04X");
	state_add( I386_SP,         "~SP", REG16(SP)).formatstr("%04X");
	state_add( I386_IP,         "~IP", m_debugger_temp).mask(0xffff).callimport().callexport().formatstr("%04X");
	state_add( I386_EAX,        "EAX", m_reg.d[EAX]).formatstr("%08X");
	state_add( I386_EBX,        "EBX", m_reg.d[EBX]).formatstr("%08X");
	state_add( I386_ECX,        "ECX", m_reg.d[ECX]).formatstr("%08X");
	state_add( I386_EDX,        "EDX", m_reg.d[EDX]).formatstr("%08X");
	state_add( I386_EBP,        "EBP", m_reg.d[EBP]).formatstr("%08X");
	state_add( I386_ESP,        "ESP", m_reg.d[ESP]).formatstr("%08X");
	state_add( I386_ESI,        "ESI", m_reg.d[ESI]).formatstr("%08X");
	state_add( I386_EDI,        "EDI", m_reg.d[EDI]).formatstr("%08X");
	state_add( I386_EFLAGS,     "EFLAGS", m_eflags).formatstr("%08X");
	state_add( I386_CS,         "CS", m_sreg[CS].selector).callimport().formatstr("%04X");
	state_add( I386_CS_BASE,    "CSBASE", m_sreg[CS].base).formatstr("%08X");
	state_add( I386_CS_LIMIT,   "CSLIMIT", m_sreg[CS].limit).formatstr("%08X");
	state_add( I386_CS_FLAGS,   "CSFLAGS", m_sreg[CS].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_SS,         "SS", m_sreg[SS].selector).callimport().formatstr("%04X");
	state_add( I386_SS_BASE,    "SSBASE", m_sreg[SS].base).formatstr("%08X");
	state_add( I386_SS_LIMIT,   "SSLIMIT", m_sreg[SS].limit).formatstr("%08X");
	state_add( I386_SS_FLAGS,   "SSFLAGS", m_sreg[SS].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_DS,         "DS", m_sreg[DS].selector).callimport().formatstr("%04X");
	state_add( I386_DS_BASE,    "DSBASE", m_sreg[DS].base).formatstr("%08X");
	state_add( I386_DS_LIMIT,   "DSLIMIT", m_sreg[DS].limit).formatstr("%08X");
	state_add( I386_DS_FLAGS,   "DSFLAGS", m_sreg[DS].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_ES,         "ES", m_sreg[ES].selector).callimport().formatstr("%04X");
	state_add( I386_ES_BASE,    "ESBASE", m_sreg[ES].base).formatstr("%08X");
	state_add( I386_ES_LIMIT,   "ESLIMIT", m_sreg[ES].limit).formatstr("%08X");
	state_add( I386_ES_FLAGS,   "ESFLAGS", m_sreg[ES].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_FS,         "FS", m_sreg[FS].selector).callimport().formatstr("%04X");
	state_add( I386_FS_BASE,    "FSBASE", m_sreg[FS].base).formatstr("%08X");
	state_add( I386_FS_LIMIT,   "FSLIMIT", m_sreg[FS].limit).formatstr("%08X");
	state_add( I386_FS_FLAGS,   "FSFLAGS", m_sreg[FS].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_GS,         "GS", m_sreg[GS].selector).callimport().formatstr("%04X");
	state_add( I386_GS_BASE,    "GSBASE", m_sreg[GS].base).formatstr("%08X");
	state_add( I386_GS_LIMIT,   "GSLIMIT", m_sreg[GS].limit).formatstr("%08X");
	state_add( I386_GS_FLAGS,   "GSFLAGS", m_sreg[GS].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_CR0,        "CR0", m_cr[0]).formatstr("%08X");
	state_add( I386_CR1,        "CR1", m_cr[1]).formatstr("%08X");
	state_add( I386_CR2,        "CR2", m_cr[2]).formatstr("%08X");
	state_add( I386_CR3,        "CR3", m_cr[3]).formatstr("%08X");
	state_add( I386_CR4,        "CR4", m_cr[4]).formatstr("%08X");
	state_add( I386_DR0,        "DR0", m_dr[0]).formatstr("%08X");
	state_add( I386_DR1,        "DR1", m_dr[1]).formatstr("%08X");
	state_add( I386_DR2,        "DR2", m_dr[2]).formatstr("%08X");
	state_add( I386_DR3,        "DR3", m_dr[3]).formatstr("%08X");
	state_add( I386_DR4,        "DR4", m_dr[4]).formatstr("%08X");
	state_add( I386_DR5,        "DR5", m_dr[5]).formatstr("%08X");
	state_add( I386_DR6,        "DR6", m_dr[6]).formatstr("%08X");
	state_add( I386_DR7,        "DR7", m_dr[7]).formatstr("%08X");
	state_add( I386_TR6,        "TR6", m_tr[6]).formatstr("%08X");
	state_add( I386_TR7,        "TR7", m_tr[7]).formatstr("%08X");
	state_add( I386_GDTR_BASE,  "GDTRBASE", m_gdtr.base).formatstr("%08X");
	state_add( I386_GDTR_LIMIT, "GDTRLIMIT", m_gdtr.limit).formatstr("%04X");
	state_add( I386_IDTR_BASE,  "IDTRBASE", m_idtr.base).formatstr("%08X");
	state_add( I386_IDTR_LIMIT, "IDTRLIMIT", m_idtr.limit).formatstr("%04X");
	state_add( I386_LDTR,       "LDTR", m_ldtr.segment).formatstr("%04X");
	state_add( I386_LDTR_BASE,  "LDTRBASE", m_ldtr.base).formatstr("%08X");
	state_add( I386_LDTR_LIMIT, "LDTRLIMIT", m_ldtr.limit).formatstr("%08X");
	state_add( I386_LDTR_FLAGS, "LDTRFLAGS", m_ldtr.flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_TR,         "TR", m_task.segment).formatstr("%04X");
	state_add( I386_TR_BASE,    "TRBASE", m_task.base).formatstr("%08X");
	state_add( I386_TR_LIMIT,   "TRLIMIT", m_task.limit).formatstr("%08X");
	state_add( I386_TR_FLAGS,   "TRFLAGS", m_task.flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_CPL,        "CPL", m_CPL).formatstr("%01X");

	state_add( STATE_GENPC, "GENPC", m_pc).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).formatstr("%8s").noshow();
	state_add( STATE_GENSP, "GENSP", REG32(ESP)).noshow();
}

void i386_device::register_state_i386_x87()
{
	register_state_i386();

	state_add( X87_CTRL,   "x87_CW", m_x87_cw).formatstr("%04X");
	state_add( X87_STATUS, "x87_SW", m_x87_sw).formatstr("%04X");
	state_add( X87_TAG,    "x87_TAG", m_x87_tw).formatstr("%04X");
	state_add( X87_ST0,    "ST0", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST1,    "ST1", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST2,    "ST2", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST3,    "ST3", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST4,    "ST4", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST5,    "ST5", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST6,    "ST6", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST7,    "ST7", m_debugger_temp ).formatstr("%15s");
}

void i386_device::register_state_i386_x87_xmm()
{
	register_state_i386_x87();

	state_add( SSE_XMM0, "XMM0", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM1, "XMM1", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM2, "XMM2", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM3, "XMM3", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM4, "XMM4", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM5, "XMM5", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM6, "XMM6", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM7, "XMM7", m_debugger_temp ).formatstr("%32s");

}

void i386_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case I386_EIP:
			CHANGE_PC(m_eip);
			break;
		case I386_IP:
			m_eip = ( m_eip & ~0xffff ) | ( m_debugger_temp & 0xffff);
			CHANGE_PC(m_eip);
			break;
		case I386_CS:
			i386_load_segment_descriptor(CS);
			break;
		case I386_SS:
			i386_load_segment_descriptor(SS);
			break;
		case I386_DS:
			i386_load_segment_descriptor(DS);
			break;
		case I386_ES:
			i386_load_segment_descriptor(ES);
			break;
		case I386_FS:
			i386_load_segment_descriptor(FS);
			break;
		case I386_GS:
			i386_load_segment_descriptor(GS);
			break;
	}
}

void i386_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case I386_IP:
			m_debugger_temp = m_eip & 0xffff;
			break;
	}
}

void i386_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%08X", get_flags());
			break;
		case X87_ST0:
			str = string_format("%f", fx80_to_double(ST(0)));
			break;
		case X87_ST1:
			str = string_format("%f", fx80_to_double(ST(1)));
			break;
		case X87_ST2:
			str = string_format("%f", fx80_to_double(ST(2)));
			break;
		case X87_ST3:
			str = string_format("%f", fx80_to_double(ST(3)));
			break;
		case X87_ST4:
			str = string_format("%f", fx80_to_double(ST(4)));
			break;
		case X87_ST5:
			str = string_format("%f", fx80_to_double(ST(5)));
			break;
		case X87_ST6:
			str = string_format("%f", fx80_to_double(ST(6)));
			break;
		case X87_ST7:
			str = string_format("%f", fx80_to_double(ST(7)));
			break;
		case SSE_XMM0:
			str = string_format("%08x%08x%08x%08x", XMM(0).d[3], XMM(0).d[2], XMM(0).d[1], XMM(0).d[0]);
			break;
		case SSE_XMM1:
			str = string_format("%08x%08x%08x%08x", XMM(1).d[3], XMM(1).d[2], XMM(1).d[1], XMM(1).d[0]);
			break;
		case SSE_XMM2:
			str = string_format("%08x%08x%08x%08x", XMM(2).d[3], XMM(2).d[2], XMM(2).d[1], XMM(2).d[0]);
			break;
		case SSE_XMM3:
			str = string_format("%08x%08x%08x%08x", XMM(3).d[3], XMM(3).d[2], XMM(3).d[1], XMM(3).d[0]);
			break;
		case SSE_XMM4:
			str = string_format("%08x%08x%08x%08x", XMM(4).d[3], XMM(4).d[2], XMM(4).d[1], XMM(4).d[0]);
			break;
		case SSE_XMM5:
			str = string_format("%08x%08x%08x%08x", XMM(5).d[3], XMM(5).d[2], XMM(5).d[1], XMM(5).d[0]);
			break;
		case SSE_XMM6:
			str = string_format("%08x%08x%08x%08x", XMM(6).d[3], XMM(6).d[2], XMM(6).d[1], XMM(6).d[0]);
			break;
		case SSE_XMM7:
			str = string_format("%08x%08x%08x%08x", XMM(7).d[3], XMM(7).d[2], XMM(7).d[1], XMM(7).d[0]);
			break;
	}
}

void i386_device::build_opcode_table(UINT32 features)
{
	int i;
	for (i=0; i < 256; i++)
	{
		m_opcode_table1_16[i] = &i386_device::i386_invalid;
		m_opcode_table1_32[i] = &i386_device::i386_invalid;
		m_opcode_table2_16[i] = &i386_device::i386_invalid;
		m_opcode_table2_32[i] = &i386_device::i386_invalid;
		m_opcode_table366_16[i] = &i386_device::i386_invalid;
		m_opcode_table366_32[i] = &i386_device::i386_invalid;
		m_opcode_table3f2_16[i] = &i386_device::i386_invalid;
		m_opcode_table3f2_32[i] = &i386_device::i386_invalid;
		m_opcode_table3f3_16[i] = &i386_device::i386_invalid;
		m_opcode_table3f3_32[i] = &i386_device::i386_invalid;
		m_lock_table[0][i] = false;
		m_lock_table[1][i] = false;
	}

	for (i=0; i < sizeof(s_x86_opcode_table)/sizeof(X86_OPCODE); i++)
	{
		const X86_OPCODE *op = &s_x86_opcode_table[i];

		if ((op->flags & features))
		{
			if (op->flags & OP_2BYTE)
			{
				m_opcode_table2_32[op->opcode] = op->handler32;
				m_opcode_table2_16[op->opcode] = op->handler16;
				m_opcode_table366_32[op->opcode] = op->handler32;
				m_opcode_table366_16[op->opcode] = op->handler16;
				m_lock_table[1][op->opcode] = op->lockable;
			}
			else if (op->flags & OP_3BYTE66)
			{
				m_opcode_table366_32[op->opcode] = op->handler32;
				m_opcode_table366_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTEF2)
			{
				m_opcode_table3f2_32[op->opcode] = op->handler32;
				m_opcode_table3f2_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTEF3)
			{
				m_opcode_table3f3_32[op->opcode] = op->handler32;
				m_opcode_table3f3_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTE38)
			{
				m_opcode_table338_32[op->opcode] = op->handler32;
				m_opcode_table338_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTE3A)
			{
				m_opcode_table33a_32[op->opcode] = op->handler32;
				m_opcode_table33a_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE3866)
			{
				m_opcode_table46638_32[op->opcode] = op->handler32;
				m_opcode_table46638_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE3A66)
			{
				m_opcode_table4663a_32[op->opcode] = op->handler32;
				m_opcode_table4663a_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE38F2)
			{
				m_opcode_table4f238_32[op->opcode] = op->handler32;
				m_opcode_table4f238_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE3AF2)
			{
				m_opcode_table4f23a_32[op->opcode] = op->handler32;
				m_opcode_table4f23a_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE38F3)
			{
				m_opcode_table4f338_32[op->opcode] = op->handler32;
				m_opcode_table4f338_16[op->opcode] = op->handler16;
			}
			else
			{
				m_opcode_table1_32[op->opcode] = op->handler32;
				m_opcode_table1_16[op->opcode] = op->handler16;
				m_lock_table[0][op->opcode] = op->lockable;
			}
		}
	}
}

void i386_device::zero_state()
{
	memset( &m_reg, 0, sizeof(m_reg) );
	memset( m_sreg, 0, sizeof(m_sreg) );
	m_eip = 0;
	m_pc = 0;
	m_prev_eip = 0;
	m_eflags = 0;
	m_eflags_mask = 0;
	m_CF = 0;
	m_DF = 0;
	m_SF = 0;
	m_OF = 0;
	m_ZF = 0;
	m_PF = 0;
	m_AF = 0;
	m_IF = 0;
	m_TF = 0;
	m_IOP1 = 0;
	m_IOP2 = 0;
	m_NT = 0;
	m_RF = 0;
	m_VM = 0;
	m_AC = 0;
	m_VIF = 0;
	m_VIP = 0;
	m_ID = 0;
	m_CPL = 0;
	m_performed_intersegment_jump = 0;
	m_delayed_interrupt_enable = 0;
	memset( m_cr, 0, sizeof(m_cr) );
	memset( m_dr, 0, sizeof(m_dr) );
	memset( m_tr, 0, sizeof(m_tr) );
	memset( &m_gdtr, 0, sizeof(m_gdtr) );
	memset( &m_idtr, 0, sizeof(m_idtr) );
	memset( &m_task, 0, sizeof(m_task) );
	memset( &m_ldtr, 0, sizeof(m_ldtr) );
	m_ext = 0;
	m_halted = 0;
	m_operand_size = 0;
	m_xmm_operand_size = 0;
	m_address_size = 0;
	m_operand_prefix = 0;
	m_address_prefix = 0;
	m_segment_prefix = 0;
	m_segment_override = 0;
	m_cycles = 0;
	m_base_cycles = 0;
	m_opcode = 0;
	m_irq_state = 0;
	m_a20_mask = 0;
	m_cpuid_max_input_value_eax = 0;
	m_cpuid_id0 = 0;
	m_cpuid_id1 = 0;
	m_cpuid_id2 = 0;
	m_cpu_version = 0;
	m_feature_flags = 0;
	m_tsc = 0;
	m_perfctr[0] = m_perfctr[1] = 0;
	memset( m_x87_reg, 0, sizeof(m_x87_reg) );
	m_x87_cw = 0;
	m_x87_sw = 0;
	m_x87_tw = 0;
	m_x87_data_ptr = 0;
	m_x87_inst_ptr = 0;
	m_x87_opcode = 0;
	memset( m_sse_reg, 0, sizeof(m_sse_reg) );
	m_mxcsr = 0;
	m_smm = false;
	m_smi = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;
	m_smbase = 0;
	memset( m_opcode_bytes, 0, sizeof(m_opcode_bytes) );
	m_opcode_pc = 0;
	m_opcode_bytes_length = 0;
}

void i386_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x9b;
	m_sreg[CS].valid    = true;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0092;
	m_sreg[DS].valid = m_sreg[ES].valid = m_sreg[FS].valid = m_sreg[GS].valid = m_sreg[SS].valid =true;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;
	m_smm = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;

	m_a20_mask = ~0;

	m_cr[0] = 0x7fffffe0; // reserved bits set to 1
	m_eflags = 0;
	m_eflags_mask = 0x00037fd7;
	m_eip = 0xfff0;

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 3 (386), Model 0 (DX), Stepping 8 (D1)
	REG32(EAX) = 0;
	REG32(EDX) = (3 << 8) | (0 << 4) | (8);
	m_cpu_version = REG32(EDX);

	m_CPL = 0;

	CHANGE_PC(m_eip);
}

void i386_device::pentium_smi()
{
	UINT32 smram_state = m_smbase + 0xfe00;
	UINT32 old_cr0 = m_cr[0];
	UINT32 old_flags = get_flags();

	if(m_smm)
		return;

	m_cr[0] &= ~(0x8000000d);
	set_flags(2);
	if(!m_smiact.isnull())
		m_smiact(true);
	m_smm = true;
	m_smi_latched = false;

	// save state
	WRITE32(m_cr[4], smram_state+SMRAM_IP5_CR4);
	WRITE32(m_sreg[ES].limit, smram_state+SMRAM_IP5_ESLIM);
	WRITE32(m_sreg[ES].base, smram_state+SMRAM_IP5_ESBASE);
	WRITE32(m_sreg[ES].flags, smram_state+SMRAM_IP5_ESACC);
	WRITE32(m_sreg[CS].limit, smram_state+SMRAM_IP5_CSLIM);
	WRITE32(m_sreg[CS].base, smram_state+SMRAM_IP5_CSBASE);
	WRITE32(m_sreg[CS].flags, smram_state+SMRAM_IP5_CSACC);
	WRITE32(m_sreg[SS].limit, smram_state+SMRAM_IP5_SSLIM);
	WRITE32(m_sreg[SS].base, smram_state+SMRAM_IP5_SSBASE);
	WRITE32(m_sreg[SS].flags, smram_state+SMRAM_IP5_SSACC);
	WRITE32(m_sreg[DS].limit, smram_state+SMRAM_IP5_DSLIM);
	WRITE32(m_sreg[DS].base, smram_state+SMRAM_IP5_DSBASE);
	WRITE32(m_sreg[DS].flags, smram_state+SMRAM_IP5_DSACC);
	WRITE32(m_sreg[FS].limit, smram_state+SMRAM_IP5_FSLIM);
	WRITE32(m_sreg[FS].base, smram_state+SMRAM_IP5_FSBASE);
	WRITE32(m_sreg[FS].flags, smram_state+SMRAM_IP5_FSACC);
	WRITE32(m_sreg[GS].limit, smram_state+SMRAM_IP5_GSLIM);
	WRITE32(m_sreg[GS].base, smram_state+SMRAM_IP5_GSBASE);
	WRITE32(m_sreg[GS].flags, smram_state+SMRAM_IP5_GSACC);
	WRITE32(m_ldtr.flags, smram_state+SMRAM_IP5_LDTACC);
	WRITE32(m_ldtr.limit, smram_state+SMRAM_IP5_LDTLIM);
	WRITE32(m_ldtr.base, smram_state+SMRAM_IP5_LDTBASE);
	WRITE32(m_gdtr.limit, smram_state+SMRAM_IP5_GDTLIM);
	WRITE32(m_gdtr.base, smram_state+SMRAM_IP5_GDTBASE);
	WRITE32(m_idtr.limit, smram_state+SMRAM_IP5_IDTLIM);
	WRITE32(m_idtr.base, smram_state+SMRAM_IP5_IDTBASE);
	WRITE32(m_task.limit, smram_state+SMRAM_IP5_TRLIM);
	WRITE32(m_task.base, smram_state+SMRAM_IP5_TRBASE);
	WRITE32(m_task.flags, smram_state+SMRAM_IP5_TRACC);

	WRITE32(m_sreg[ES].selector, smram_state+SMRAM_ES);
	WRITE32(m_sreg[CS].selector, smram_state+SMRAM_CS);
	WRITE32(m_sreg[SS].selector, smram_state+SMRAM_SS);
	WRITE32(m_sreg[DS].selector, smram_state+SMRAM_DS);
	WRITE32(m_sreg[FS].selector, smram_state+SMRAM_FS);
	WRITE32(m_sreg[GS].selector, smram_state+SMRAM_GS);
	WRITE32(m_ldtr.segment, smram_state+SMRAM_LDTR);
	WRITE32(m_task.segment, smram_state+SMRAM_TR);

	WRITE32(m_dr[7], smram_state+SMRAM_DR7);
	WRITE32(m_dr[6], smram_state+SMRAM_DR6);
	WRITE32(REG32(EAX), smram_state+SMRAM_EAX);
	WRITE32(REG32(ECX), smram_state+SMRAM_ECX);
	WRITE32(REG32(EDX), smram_state+SMRAM_EDX);
	WRITE32(REG32(EBX), smram_state+SMRAM_EBX);
	WRITE32(REG32(ESP), smram_state+SMRAM_ESP);
	WRITE32(REG32(EBP), smram_state+SMRAM_EBP);
	WRITE32(REG32(ESI), smram_state+SMRAM_ESI);
	WRITE32(REG32(EDI), smram_state+SMRAM_EDI);
	WRITE32(m_eip, smram_state+SMRAM_EIP);
	WRITE32(old_flags, smram_state+SMRAM_EFLAGS);
	WRITE32(m_cr[3], smram_state+SMRAM_CR3);
	WRITE32(old_cr0, smram_state+SMRAM_CR0);

	m_sreg[DS].selector = m_sreg[ES].selector = m_sreg[FS].selector = m_sreg[GS].selector = m_sreg[SS].selector = 0;
	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffffffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x8093;
	m_sreg[DS].valid = m_sreg[ES].valid = m_sreg[FS].valid = m_sreg[GS].valid = m_sreg[SS].valid =true;
	m_sreg[CS].selector = 0x3000; // pentium only, ppro sel = smbase >> 4
	m_sreg[CS].base = m_smbase;
	m_sreg[CS].limit = 0xffffffff;
	m_sreg[CS].flags = 0x809b;
	m_sreg[CS].valid = true;
	m_cr[4] = 0;
	m_dr[7] = 0x400;
	m_eip = 0x8000;

	m_nmi_masked = true;
	CHANGE_PC(m_eip);
}

void i386_device::execute_set_input(int irqline, int state)
{
	if ( irqline == INPUT_LINE_A20 )
	{
		i386_set_a20_line( state );
		return;
	}
	if ( irqline == INPUT_LINE_NMI )
	{
		if ( state != CLEAR_LINE && m_halted)
		{
			m_halted = 0;
		}

		/* NMI (I do not think that this is 100% right) */
		if(m_nmi_masked)
		{
			m_nmi_latched = true;
			return;
		}
		if ( state )
			i386_trap(2, 1, 0);
	}
	else
	{
		if (irqline >= 0 && irqline <= MAX_INPUT_LINES)
		{
			if ( state != CLEAR_LINE && m_halted )
			{
				m_halted = 0;
			}

			m_irq_state = state;
		}
	}
}

void pentium_device::execute_set_input(int irqline, int state)
{
	if ( irqline == INPUT_LINE_SMI )
	{
		if ( !m_smi && state && m_smm )
		{
			m_smi_latched = true;
		}
		m_smi = state;
	}
	else
	{
		i386_device::execute_set_input(irqline, state);
	}
}

void i386_device::i386_set_a20_line(int state)
{
	if (state)
	{
		m_a20_mask = ~0;
	}
	else
	{
		m_a20_mask = ~(1 << 20);
	}
	// TODO: how does A20M and the tlb interact
	vtlb_flush_dynamic();
}

void i386_device::execute_run()
{
	int cycles = m_cycles;
	m_base_cycles = cycles;
	CHANGE_PC(m_eip);

	if (m_halted)
	{
		m_tsc += cycles;
		m_cycles = 0;
		return;
	}

	while( m_cycles > 0 )
	{
		i386_check_irq_line();
		m_operand_size = m_sreg[CS].d;
		m_xmm_operand_size = 0;
		m_address_size = m_sreg[CS].d;
		m_operand_prefix = 0;
		m_address_prefix = 0;

		m_ext = 1;
		int old_tf = m_TF;

		m_segment_prefix = 0;
		m_prev_eip = m_eip;

		debugger_instruction_hook(this, m_pc);

		if(m_delayed_interrupt_enable != 0)
		{
			m_IF = 1;
			m_delayed_interrupt_enable = 0;
		}
#ifdef DEBUG_MISSING_OPCODE
		m_opcode_bytes_length = 0;
		m_opcode_pc = m_pc;
#endif
		try
		{
			i386_decode_opcode();
			if(m_TF && old_tf)
			{
				m_prev_eip = m_eip;
				m_ext = 1;
				i386_trap(1,0,0);
			}
			if(m_lock && (m_opcode != 0xf0))
				m_lock = false;
		}
		catch(UINT64 e)
		{
			m_ext = 1;
			i386_trap_with_error(e&0xffffffff,0,0,e>>32);
		}
	}
	m_tsc += (cycles - m_cycles);
}

/*************************************************************************/

bool i386_device::memory_translate(address_spacenum spacenum, int intention, offs_t &address)
{
	bool ret = true;
	if(spacenum == AS_PROGRAM)
		ret = i386_translate_address(intention, &address, nullptr);
	address &= m_a20_mask;
	return ret;
}

offs_t i386_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return i386_dasm_one(buffer, pc, oprom, m_sreg[CS].d ? 32 : 16);
}


/*****************************************************************************/
/* Intel 486 */


void i486_device::device_start()
{
	i386_common_init();

	build_opcode_table(OP_I386 | OP_FPU | OP_I486);
	build_x87_opcode_table();
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_I486].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_I486].get();

	register_state_i386_x87();
}

void i486_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x009b;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0092;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x00000010;
	m_eflags = 0;
	m_eflags_mask = 0x00077fd7;
	m_eip = 0xfff0;
	m_smm = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 4 (486), Model 0/1 (DX), Stepping 3
	REG32(EAX) = 0;
	REG32(EDX) = (4 << 8) | (0 << 4) | (3);
	m_cpu_version = REG32(EDX);

	CHANGE_PC(m_eip);
}


/*****************************************************************************/
/* Pentium */


void pentium_device::device_start()
{
	i386_common_init();
	register_state_i386_x87();

	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM);
	build_x87_opcode_table();
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();
}

void pentium_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x009b;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0092;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x00000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x003f7fd7;
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 5 (Pentium), Model 2 (75 - 200MHz), Stepping 5
	REG32(EAX) = 0;
	REG32(EDX) = (5 << 8) | (2 << 4) | (5);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	m_feature_flags = 0x000001bf;

	CHANGE_PC(m_eip);
}


/*****************************************************************************/
/* Cyrix MediaGX */


void mediagx_device::device_start()
{
	i386_common_init();
	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_CYRIX);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_MEDIAGX].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_MEDIAGX].get();
}

void mediagx_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x009b;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0092;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x00000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_smm = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 4, Model 4 (MediaGX)
	REG32(EAX) = 0;
	REG32(EDX) = (4 << 8) | (4 << 4) | (1); /* TODO: is this correct? */

	m_cpuid_id0 = 0x69727943;   // Cyri
	m_cpuid_id1 = 0x736e4978;   // xIns
	m_cpuid_id2 = 0x6d616574;   // tead

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	m_feature_flags = 0x00000001;

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium Pro */

void pentium_pro_device::device_start()
{
	i386_common_init();
	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium_pro_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x009b;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0092;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 1 (Pentium Pro)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (1 << 4) | (1); /* TODO: is this correct? */

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x02;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	// [15:15] CMOV and FCMOV
	// No MMX
	m_feature_flags = 0x000081bf;

	CHANGE_PC(m_eip);
}


/*****************************************************************************/
/* Intel Pentium MMX */

void pentium_mmx_device::device_start()
{
	i386_common_init();
	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_MMX);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium_mmx_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x009b;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0092;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 5, Model 4 (P55C)
	REG32(EAX) = 0;
	REG32(EDX) = (5 << 8) | (4 << 4) | (1);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	// [23:23] MMX instructions
	m_feature_flags = 0x008001bf;

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium II */

void pentium2_device::device_start()
{
	i386_common_init();
	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium2_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x009b;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0092;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 3 (Pentium II / Klamath)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (3 << 4) | (1); /* TODO: is this correct? */

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x02;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	m_feature_flags = 0x008081bf;       // TODO: enable relevant flags here

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium III */

void pentium3_device::device_start()
{
	i386_common_init();
	register_state_i386_x87_xmm();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX | OP_SSE);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium3_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x009b;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0092;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 8 (Pentium III / Coppermine)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (8 << 4) | (10);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x03;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 4:4] Time Stamp Counter
	// [ D:D] PTE Global Bit
	m_feature_flags = 0x00002011;       // TODO: enable relevant flags here

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium 4 */

void pentium4_device::device_start()
{
	i386_common_init();
	register_state_i386_x87_xmm();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX | OP_SSE | OP_SSE2);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium4_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x009b;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0092;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [27:20] Extended family
	// [19:16] Extended model
	// [13:12] Type
	// [11: 8] Family
	// [ 7: 4] Model
	// [ 3: 0] Stepping ID
	// Family 15, Model 0 (Pentium 4 / Willamette)
	REG32(EAX) = 0;
	REG32(EDX) = (0 << 20) | (0xf << 8) | (0 << 4) | (1);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x02;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	m_feature_flags = 0x00000001;       // TODO: enable relevant flags here

	CHANGE_PC(m_eip);
}
