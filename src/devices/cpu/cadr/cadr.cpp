// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    MIT CADR microcode emulation

TODO:
- Instruction bit 46, statistics, not supported.
- Instruction bit 45, ilong, not supported.
- Not all ALU operations are supported.
- Most 'Misc functions' on instructions are not supported.
- Sequence break not supported.
- Most of the diagnostic/spy interface is not supported.

***************************************************************************/

#include "emu.h"
#include "cadr.h"
#include "cadr_dasm.h"


DEFINE_DEVICE_TYPE(CADR, cadr_cpu_device, "cadr_cpu", "MIT CADR")


#define LOG_TRACE (1 << 1)
#define LOG_DIAG (1 << 2)
#define LOG_VMA (1 << 3)
#define LOG_INT (1 << 4)
//#define VERBOSE (LOG_GENERAL | LOG_TRACE | LOG_DIAG | LOG_VMA)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

static const u8 dispatch_mask[0x08] =
{
	0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f
};


} // anonymous namespace

enum
{
	CADR_DISPATCH_CONSTANT,
	CADR_IC,
	CADR_LC,
	CADR_MD,
	CADR_N,
	CADR_OA_REG_LO,
	CADR_OA_REG_HI,
	CADR_Q,
	CADR_PDL_INDEX,
	CADR_PDL_POINTER,
	CADR_SPC_POINTER,
	CADR_VMA
};



cadr_cpu_device::cadr_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, CADR, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 64/*48*/, ADDRESS_BITS, -3, address_map_constructor(FUNC(cadr_cpu_device::program_map), this))
	, m_data_config("data", ENDIANNESS_BIG, 32, EXTERNAL_ADDRESS_BITS, -2)
	, m_inst_view(*this, "inst_view")
	, m_imem(*this, "imem")
{
}


device_memory_interface::space_config_vector cadr_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}


std::unique_ptr<util::disasm_interface> cadr_cpu_device::create_disassembler()
{
	return std::make_unique<cadr_disassembler>();
}


void cadr_cpu_device::device_start()
{
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_pc));
	save_item(NAME(m_next_pc));
	save_item(NAME(m_ir));
	save_item(NAME(m_n));
	save_item(NAME(m_a_mem));
	save_item(NAME(m_m_mem));
	save_item(NAME(m_a));
	save_item(NAME(m_m));
	save_item(NAME(m_q));
	save_item(NAME(m_pdl));
	save_item(NAME(m_pdl_index));
	save_item(NAME(m_pdl_pointer));
	save_item(NAME(m_md));
	save_item(NAME(m_read_delay));
	save_item(NAME(m_read_data));
	save_item(NAME(m_oa_reg_lo));
	save_item(NAME(m_oa_reg_hi));
	save_item(NAME(m_spc));
	save_item(NAME(m_spc_pointer));
	save_item(NAME(m_vma));
	save_item(NAME(m_vma_map_l1));
	save_item(NAME(m_vma_map_l2));
	save_item(NAME(m_dpc));
	save_item(NAME(m_dispatch_constant));
	save_item(NAME(m_ic));
	save_item(NAME(m_lc));
	save_item(NAME(m_diag_mode));
	save_item(NAME(m_access_fault));
	save_item(NAME(m_write_fault));
	save_item(NAME(m_page_fault));
	save_item(NAME(m_popj));
	save_item(NAME(m_interrupt_pending));

	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);

	state_add(STATE_GENPC, "PC", m_pc).formatstr("%5s");
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(CADR_DISPATCH_CONSTANT, "Dispatch Constant", m_dispatch_constant).formatstr("%4s");
	state_add(CADR_IC, "IC", m_ic).formatstr("%4s");
	state_add(CADR_LC, "LC", m_lc).formatstr("%11s");
	state_add(CADR_MD, "MD", m_md).formatstr("%11s");
	state_add(CADR_N, "N", m_n).formatstr("%5s");
	state_add(CADR_OA_REG_LO, "OA-Reg-lo", m_oa_reg_lo).formatstr("%9s");
	state_add(CADR_OA_REG_HI, "OA-Reg-hi", m_oa_reg_hi).formatstr("%8s");
	state_add(CADR_Q, "Q", m_q).formatstr("%11s");
	state_add(CADR_PDL_INDEX, "PDL-Index", m_pdl_index).formatstr("%4s");
	state_add(CADR_PDL_POINTER, "PDL-Pointer", m_pdl_pointer).formatstr("%4s");
	state_add(CADR_SPC_POINTER, "SPC-Pointer", m_spc_pointer).formatstr("%4s");
	state_add(CADR_VMA, "VMA", m_vma).formatstr("%11s");

	set_icountptr(m_icount);
}


void cadr_cpu_device::device_reset()
{
	m_pc = 0;
	m_next_pc = 0;
	m_ir = 0;
	m_q = 0;
	m_n = true;
	m_oa_reg_lo = 0;
	m_oa_reg_hi = 0;
	m_lc = 0;
	m_popj = false;
	m_diag_mode = 0;
	m_inst_view.select(0);
	m_interrupt_pending = false;
	m_spc_pointer = 0;
	m_read_delay = 0;
}


void cadr_cpu_device::program_map(address_map &map)
{
	map(0, 0xffff).ram().share(m_imem);
	map(0, 0x3ff).view(m_inst_view);
	m_inst_view[0](0, 0x3ff).rom();
	m_inst_view[1];
}


u16 cadr_cpu_device::diag_r(offs_t offset)
{
	LOGMASKED(LOG_TRACE, "diag_r %02x\n", offset);
	return 0;
}


void cadr_cpu_device::diag_w(offs_t offset, u16 data)
{
	LOGMASKED(LOG_TRACE, "daig_w: %02x, %04x\n", offset, data);

	switch (offset)
	{
	// x------- PROG.BOOT
	// -x------ PROG.RESET
	// --x----- PROMDISABLE
	// ---x---- TRAPENB
	// ----x--- STATHENB
	// -----x-- ERRSTOP
	// ------xx SPEED 00 - extra slow, 01 - slow, 10 - normal, 11 - fast
	case 0x05: // mode register
		m_diag_mode = data;
		m_inst_view.select(BIT(m_diag_mode, 5));
		break;
	case 0x00: // debug ir 15-0
	case 0x01: // debug ir 31-16
	case 0x02: // debug ir 47-32
	case 0x03: // clock control register
	case 0x04: // OPC control register
	default:
		fatalerror("%x(%o): diag_w: write to %02x not implemented", m_prev_pc, m_prev_pc, offset);
	}
}


void cadr_cpu_device::read()
{
	m_access_fault = false;
	m_write_fault = false;
	m_page_fault = false;
	const u8 l1 = m_vma_map_l1[(m_vma >> 13) & 0x7ff];
	const u16 l2_index = (l1 << 5) | ((m_vma >> 8) & 0x1f);
	const u32 l2 = m_vma_map_l2[l2_index] & 0xffffff;
	u32 address = (l2 << 8) | (m_vma & 0xff);
	LOGMASKED(LOG_VMA, "VMA address %08x(%o) mapped to %08x, l1_index=%d, l1=%o, l2_index=%d, l2=%o\n", m_vma, m_vma, address, (m_vma >> 13) & 0x7ff, l1, l2_index, l2);

	if (!BIT(address, 31))
	{
		m_access_fault = true;
		m_page_fault = true;
		LOGMASKED(LOG_VMA, "access fault, page fault\n");
	}
	else
	{
		m_read_data = m_data.read_dword(address & 0x00ffffff);
		m_read_delay = 2;
	}
}


void cadr_cpu_device::write()
{
	m_access_fault = false;
	m_write_fault = false;
	m_page_fault = false;
	const u16 l1_index = (m_vma >> 13) & 0x7ff;
	const u8 l1 = m_vma_map_l1[l1_index];
	const u16 l2_index = (l1 << 5) | ((m_vma >> 8) & 0x1f);
	const u32 l2 = m_vma_map_l2[l2_index] & 0xffffff;
	u32 address = (l2 << 8) | (m_vma & 0xff);
	LOGMASKED(LOG_VMA, "VMA address 0x%08x(0%o) mapped to 0x%08x, l1_index=0x%x, l1=0%o, l2_index=0x%x, l2=0%o\n", m_vma, m_vma, address, l1_index, l1, l2_index, l2);

	if (!BIT(address, 31))
	{
		m_access_fault = true;
		m_page_fault = true;
		LOGMASKED(LOG_VMA, "access fault, page fault\n");
	}
	if (!BIT(address, 30))
	{
		m_write_fault = true;
		m_page_fault = true;
		LOGMASKED(LOG_VMA, "write fault, page fault\n");
	}
	else
	{
		m_data.write_dword(address & 0x00ffffff, m_md);
	}
}


void cadr_cpu_device::push_spc(u16 data)
{
	m_spc_pointer = (m_spc_pointer + 1) & 0x1f;
	m_spc[m_spc_pointer] = data;
}


u16 cadr_cpu_device::pop_spc()
{
	const u16 data = m_spc[m_spc_pointer];
	m_spc_pointer = (m_spc_pointer - 1) & 0x1f;
	return data;
}


void cadr_cpu_device::write_vma_map()
{
	// Write L1 map
	if (BIT(m_vma, 26))
	{
		m_vma_map_l1[(m_md >> 13) & 0x7ff] = (m_vma >> 27) & 0x1f;
		LOGMASKED(LOG_VMA, "write l1 %o %o\n", (m_md >> 13) & 0x7ff, (m_vma >> 27) & 0x1f);
	}
	// Write L2 map
	if (BIT(m_vma, 25))
	{
		const u16 index = (m_vma_map_l1[(m_md >> 13) & 0x7ff] << 5) | ((m_md >> 8) & 0x1f);
		m_vma_map_l2[index] = m_vma;
		LOGMASKED(LOG_VMA, "write l2 %o %o\n", index, m_vma);
	}
}


void cadr_cpu_device::get_m_source()
{
	if (BIT(m_ir, 31))
	{
		switch ((m_ir >> 26) & 0x1f)
		{
		case 0x00:
			m_m = m_dispatch_constant;
			break;
		case 0x01: // SPC pointer 28-24, SPC data 18-0
			m_m = ((m_spc_pointer & 0x1f) << 24) | (m_spc[m_spc_pointer] & 0x7ffff);
			break;
		case 0x02:
			m_m = m_pdl_pointer;
			break;
		case 0x03:
			m_m = m_pdl_index;
			break;
		case 0x05: 
			LOGMASKED(LOG_TRACE, "read pdl[%x(%o)] %x(%o)\n", m_pdl_index, m_pdl_index, m_pdl[m_pdl_index], m_pdl[m_pdl_index]);
			m_m = m_pdl[m_pdl_index];
			break;
		case 0x07:
			m_m = m_q;
			break;
		case 0x08:
			m_m = m_vma;
			break;
		case 0x09: // MAP[MD]
			{
				const u16 l1_index = (m_md >> 13) & 0x7ff;
				const u8 l1 = m_vma_map_l1[l1_index];
				const u16 l2_index = (l1 << 5) | ((m_md >> 8) & 0x1f);
				const u32 l2 = m_vma_map_l2[l2_index] & 0xffffff;
				LOGMASKED(LOG_VMA, "map[md]: md=%x(%o), l1_index=%x(%o), l1=%x(%o), l2_index=%x(%o), l2=%x(%o)\n", m_md, m_md, l1_index, l1_index, l1, l1, l2_index, l2_index, l2, l2);
				m_m = (l1 << 24) | l2;
			}
			break;
		case 0x0a:
			m_m = m_md;
			break;
		case 0x0b:
			m_m = m_lc;
			break;
		case 0x0c: // SPC pointer 28-24, SPC data 18-0, pop
			m_m = ((m_spc_pointer & 0x1f) << 24) | (m_spc[m_spc_pointer] & 0x7ffff);
			m_spc_pointer = (m_spc_pointer - 1) & 0x1f;
			break;
		case 0x14:
			m_m  = m_pdl[m_pdl_pointer];
			m_pdl_pointer = (m_pdl_pointer - 1) & 0x3ff;
			break;
		case 0x15:
			m_m = m_pdl[m_pdl_pointer];
			break;

		case 0x04: // PDL buffer indexed by PDL index, PDL pointer decremented
		case 0x06: // OPC registers 13-0
			fatalerror("%x(%o): get_m_source: functional m source %02x not implemented", m_prev_pc, m_prev_pc, (m_ir >> 26) & 0x1f);
		case 0x0d: // reserved
		case 0x0e: // reserved
		case 0x0f: // reserved
		case 0x16: // instruction at 21c5 / 20705
		default:
			// When no source is selected the MF lines will float
			logerror("%x(%o): get_m_source: illegal functional m source %02x selected", m_prev_pc, m_prev_pc, (m_ir >> 26) & 0x1f);
			break;
		}
	}
	else
	{
		m_m = m_m_mem[(m_ir >> 26) & 0x1f];
	}
}


void cadr_cpu_device::add32(u32 x, u32 y, u32 carry_in, u32 &res, u32 &carry_out)
{
	const u64 result = x + y + carry_in;
	res = u32(result);
	carry_out = BIT(result, 32);
}


void cadr_cpu_device::sub32(u32 x, u32 y, u32 carry_in, u32 &res, u32 &carry_out)
{
	const u64 result = y - x - (carry_in ? 0 : 1);
	res = u32(result);
	carry_out = BIT(result, 32);
}


void cadr_cpu_device::alu_operation(u32 &res, u32 &carry_out)
{
	if (BIT(m_ir, 8))
	{
		// div/mult
		switch ((m_ir >> 3) & 0x1f)
		{
		case 0x00: // mult step
			LOGMASKED(LOG_TRACE, "mult-step, a=0x%x(%o), m=0x%x(%o), q=%08x\n", m_a, m_a, m_m, m_m, m_q);
			if (BIT(m_q, 0))
			{
				add32(m_a, m_m, BIT(m_ir, 2), res, carry_out);
			}
			else
			{
				LOGMASKED(LOG_TRACE, "mult-step else m=%x\n", m_m);
				res = m_m;
				carry_out = BIT(m_m, 32);
			}
			break;
		case 0x09: // initial div step
			LOGMASKED(LOG_TRACE, "initial div %o/%o, %d/%d\n", m_q, m_a, m_q, m_a);
			sub32(m_a, m_m, BIT(~m_ir, 2), res, carry_out);
			break;
		case 0x01: // div step
			if (BIT(m_q, 0))
			{
				sub32(m_a, m_m, BIT(~m_ir, 2), res, carry_out);
			}
			else
			{
				add32(m_a, m_m, BIT(m_ir, 2), res, carry_out);
			}
			break;
		case 0x05: // remainder correction
			if (BIT(m_q, 0))
			{
				res = m_m;
			}
			else
			{
				add32(m_a, m_m, BIT(m_ir, 2), res, carry_out);
			}
			break;
		default:
			fatalerror("%x(%o): alu div/mult %02x operation not implemented", m_prev_pc, m_prev_pc, (m_ir >> 3) & 0x1f);
		}
	}
	else
	{
		switch ((m_ir >> 3) & 0x1f)
		{
		case 0x00: // SETZ - ZEROS
			res = 0;
			break;
		case 0x01: // AND - M&A
			res = m_m & m_a;
			break;
		case 0x02: // ANDCA - M&~A
			res = m_m & ~m_a;
			break;
		case 0x03: // SETM - M
			res = m_m;
			break;
		case 0x04: // ANDCM - ~M&A
			res = ~m_m & m_a;
			break;
		case 0x05: // SETA - A
			res = m_a;
			break;
		case 0x06: // XOR - M^A
			res = m_m ^ m_a;
			break;
		case 0x07: // IOR - M|A
			res = m_m | m_a;
			break;
		case 0x08: // ANDCB - ~A&~M
			res = ~m_a & ~m_m;
			break;
		case 0x0a: // SETCA - ~A
			res = ~m_a;
			break;
		case 0x0f: // SETO - ONES
			res = ~0;
			break;
		case 0x10: // -1 (C=0), 0 (C=1)
			res = u32(-1 + BIT(m_ir, 2));
			break;
		case 0x16: // M-A-1 (C=0), M-A (C=1)
			sub32(m_a, m_m, BIT(m_ir, 2), res, carry_out);
			break;
		case 0x19: // M+A (C=0), M+A+1 (C=1)
			add32(m_a, m_m, BIT(m_ir, 2), res, carry_out);
			break;
		case 0x1c: // M (C=0), M+1 (C=1)
			add32(0, m_m, BIT(m_ir, 2), res, carry_out);
			break;
		case 0x1f: // M+M (C=0), M+M+1 (C=1)
			add32(m_m, m_m, BIT(m_ir, 2), res, carry_out);
			break;

		case 0x09: // EQV - M=A
		case 0x0b: // ORCA - M&~A
		case 0x0c: // SETCM - ~M
		case 0x0d: // ORCM - ~M|A
		case 0x0e: // ORCB - ~M|~A
		case 0x11: // (M&A)-1 (C=0), (M&~A) (C=1)
		case 0x12: // (M&~A)-1 (C=0), (M&~A) (C=1)
		case 0x13: // M-1 (C=0), M (C=1)
		case 0x14: // M|~A (C=0), (M|~A)+1 (C=1)
		case 0x15: // (M|~A)+(M&A) (C=0), (M|~A)+(M&A)+1 (C=1)
		case 0x17: // (M|~A)+M (C=0), (M|~A)+M+1 (C=1)
		case 0x18: // M|~A (C=0), (M|~A)+1 (C=1)
		case 0x1a: // (M|A)+(M&~A) (C=0), (M|A)+(M&~A)+1 (C=1)
		case 0x1b: // (M|A)+M (C=0), (M|A)+M+1 (C=1)
		case 0x1d: // M+(M&~A) (C=0), M+(M&~A)+1 (C=1)
		case 0x1e: // M+(M|~A) (C=0), M+(M|~A)+1 (C=1)
			fatalerror("%x(%o): alu operation %02x not implemented", m_prev_pc, m_prev_pc, (m_ir >> 3) & 0x1f);
		}
	}
}


u32 cadr_cpu_device::get_output(u32 m, u32 alu_out, u32 alu_carry)
{
	switch ((m_ir >> 12) & 0x03)
	{
	case 0x00:
		return rotl_32(m, m_ir & 0x1f); /* illegal, output of byte extractor */
	case 0x01:
		return alu_out;
	case 0x02:
		return (alu_out >> 1) | (alu_carry ? 0x80000000 : 0);
	case 0x03:
		return (alu_out << 1) | BIT(m_q, 31);
	}
	return 0;
}


void cadr_cpu_device::write_destination(u32 output)
{
	if (BIT(m_ir, 25))
	{
		const u16 index = (m_ir >> 14) & 0x3ff;
		m_a_mem[index] = output;
		LOGMASKED(LOG_TRACE, "a[%x(%o)] <- %x(%o)\n", index, index, output, output);
	}
	else
	{
		switch ((m_ir >> 19) & 0x1f)
		{
		case 0x00: break;
		case 0x01:
			m_lc = (1 << 31) | (m_ic & 0x3c000000) | (output & 0x3ffffff);
			if (!BIT(m_ic, 29))
			{
				m_lc &= ~1;
			}
			LOGMASKED(LOG_TRACE, "LC <- %x(%o)\n", output, output);
			break;
		case 0x02: // IC
			m_ic = output;
			m_lc = (m_lc & 0x83ffffff) | (m_ic & 0x3c000000);
			LOGMASKED(LOG_INT, "sequence break %d\n", BIT(m_ic, 26));
			LOGMASKED(LOG_INT, "interrupt enable %d\n", BIT(m_ic, 27));
			LOGMASKED(LOG_INT, "bus reset %d\n", BIT(m_ic, 28));
			LOGMASKED(LOG_INT, "lc mode %d\n", BIT(m_ic, 29));
			break;
		case 0x08: // PDL[ptr]
			m_pdl[m_pdl_pointer] = output;
			LOGMASKED(LOG_TRACE, "PDL[%x(%o)] <- %x(%o)\n", m_pdl_pointer, m_pdl_pointer, output, output);
			break;
		case 0x09: // PDL[ptr], push
			m_pdl_pointer = (m_pdl_pointer + 1) & 0x3ff;
			LOGMASKED(LOG_TRACE, "PDL[%x(%o)] <- %x(%o)\n", m_pdl_pointer, m_pdl_pointer, output, output);
			m_pdl[m_pdl_pointer] = output;
			break;
		case 0x0a:
			LOGMASKED(LOG_TRACE, "PDL[%x(%o)] <- %x(%o)\n", m_pdl_index, m_pdl_index, output, output);
			m_pdl[m_pdl_index] = output;
			break;
		case 0x0b:
			m_pdl_index = output & 0x3ff;
			LOGMASKED(LOG_TRACE, "PDL-Index <- %x(%o)\n", m_pdl_index, m_pdl_index);
			break;
		case 0x0c:
			m_pdl_pointer = output & 0x3ff;
			LOGMASKED(LOG_TRACE, "PDL-ptr <- %x(%o)\n", m_pdl_pointer, m_pdl_pointer);
			break;
		case 0x0d: push_spc(output); break;
		case 0x0e:
			m_oa_reg_lo = output & 0x3ffffff;
			LOGMASKED(LOG_TRACE, "OA-reg-lo <- %x(%o)\n", m_oa_reg_lo, m_oa_reg_lo);
			break;
		case 0x0f:
			m_oa_reg_hi = output & 0x3fffff;
			LOGMASKED(LOG_TRACE, "OA-reg-hi <- %x(%o)\n", m_oa_reg_hi, m_oa_reg_hi);
			break;
		case 0x10:
			m_vma = output;
			LOGMASKED(LOG_TRACE, "VMA <- %x(%o)\n", m_vma, m_vma);
			break;
		case 0x11: // VMA, start-read
			m_vma = output;
			read();
			LOGMASKED(LOG_TRACE, "VMA <- %x(%o), start-read, MD=%x(%o)\n", m_vma, m_vma, m_md, m_md);
			break;
		case 0x12: // VMA, start-write
			m_vma = output;
			write();
			LOGMASKED(LOG_TRACE, "VMA <- %x(%o), start-write, MD=%x(%o)\n", m_vma, m_vma, m_md, m_md);
			break;
		case 0x13: // VMA, write map
			m_vma = output;
			write_vma_map();
			break;
		case 0x18:
			m_md = output;
			LOGMASKED(LOG_TRACE, "MD <- %x(%o)\n", m_md, m_md);
			break;
		case 0x1a: // MD, start-write
			m_md = output;
			write();
			LOGMASKED(LOG_TRACE, "MD <- %x(%o), start-write, MD=%x(%o)\n", m_md, m_md, m_md, m_md);
			break;
		case 0x1b: // MD, write map
			m_md = output;
			write_vma_map();
			break;

		case 0x19: // MD, start-read
		default:
		   fatalerror("%x(%o): output %02x not implemented", m_prev_pc, m_prev_pc, (m_ir >> 19) & 0x1f); break;
		}
		const u16 index = (m_ir >> 14) & 0x1f;
		m_m_mem[index] = output;
		m_a_mem[index] = output;
		LOGMASKED(LOG_TRACE, "m/a[%x(%o)] <- %x(%o)\n", index, index, output, output);
	}
}


bool cadr_cpu_device::jump_condition(s32 a, s32 m)
{
	bool condition = true;

	if (BIT(m_ir, 5))
	{
		switch (m_ir & 0x1f)
		{
		case 0x01: condition = m < a; break;
		case 0x02: condition = m <= a; break;
		case 0x03: condition = m == a; break;
		case 0x04: condition = m_page_fault; break;
		case 0x05: condition = m_page_fault || m_interrupt_pending; break;
		case 0x06: condition = m_page_fault || m_interrupt_pending /* || m_sequence_break */; break;
		case 0x07: condition = true; break;
		default:
			fatalerror("%x(%o): jump condition %02x not implemented", m_prev_pc, m_prev_pc, m_ir & 0x1f);
		}
	}
	else
	{
		condition = BIT(rotl_32(m, m_ir & 0x1f), 0);
	}

	if (BIT(m_ir, 6))
	{
		condition = !condition;
	}

	return condition;
}


void cadr_cpu_device::instruction_stream()
{
	u32 lc = m_lc & 0x3ffffff;
	bool byte_mode = BIT(m_ic, 29);
	m_lc = (m_lc & 0xbc000000) | ((lc + (byte_mode ? 1 : 2)) & 0x3ffffff);

	if (BIT(m_lc, 31))
	{
		m_lc &= 0x7fffffff;
		m_vma = lc >> 2;
		read();
		LOGMASKED(LOG_TRACE, "instruction_stream: read lc=%x(%o), vma=%x(%o), md=%x(%o)\n", lc, lc, m_vma, m_vma, m_md, m_md);
	}
	else
	{
		LOGMASKED(LOG_TRACE, "instruction_stream: no fetch\n");
		m_next_pc |= 2;
	}

	LOGMASKED(LOG_TRACE, "lc=%x(%o)\n", m_lc, m_lc);
	if (!(BIT(m_lc, 1) || (byte_mode && BIT(m_lc, 0))))
	{
		LOGMASKED(LOG_TRACE, "instruction_stream: lc=%x(%o), last byte in word\n", m_lc, m_lc);
		m_lc |= (1 << 31);
	}
	else
	{
		LOGMASKED(LOG_TRACE, "instruction_stream: lc=%x(%o), not last byte in word\n", m_lc, m_lc);
	}
}


void cadr_cpu_device::execute_alu()
{
	// TODO Misc functions
	if ((m_ir >> 10) & 0x03) {
		fatalerror("%x(%o): alu misc function %d not implemented", m_prev_pc, m_prev_pc, (m_ir >> 10) & 0x03);
	}

	u32 alu_out = 0;
	u32 carry_out = 0;
	alu_operation(alu_out, carry_out);

	u32 output = get_output(m_m, alu_out, carry_out);
	switch (m_ir & 0x03)
	{
	case 0x01: m_q = (m_q << 1) | (BIT(alu_out, 31) ^ 0x01); break;
	case 0x02: m_q = (m_q >> 1) | (BIT(alu_out, 0) << 31); break;
	case 0x03: m_q = alu_out; break;
	}
	write_destination(output);
}


void cadr_cpu_device::execute_jump()
{
	// TODO Misc functions
	if (((m_ir >> 10) & 0x03) > 0x01) {
		fatalerror("%x(%o): jump misc function %d not implemented", m_prev_pc, m_prev_pc, (m_ir >> 10) & 0x03);
	}

	if (jump_condition(m_a, m_m))
	{
		m_n = BIT(m_ir, 7);

		switch ((m_ir >> 8) & 0x03)
		{
		case 0x00: // jump
			m_next_pc = (m_ir >> 12) & 0x3fff;
			m_popj = false;
			break;
		case 0x01: // push pc on spc stack
			push_spc(m_n ? m_pc : m_next_pc);
			m_next_pc = (m_ir >> 12) & 0x3fff;
			m_popj = false;
			break;
		case 0x02: // pop new pc from spc stack
			m_next_pc = pop_spc();
			if (BIT(m_next_pc, 14))
			{
				instruction_stream();
			}
			m_next_pc &= 0x3fff;
			m_popj = false;
			break;
		case 0x03: // write i-mem
			LOGMASKED(LOG_TRACE, "write imem a[%x] = %08x , m = %08x\n", (m_ir >> 12) & 0x3fff, m_a, m_m);
			m_imem[(m_ir >> 12) & 0x3fff] = (u64(m_a & 0xffff) << 32) | m_m;
			m_icount--;
			m_n = false;
			break;
		}
	}

}


void cadr_cpu_device::execute_dispatch()
{
	m_dispatch_constant = (m_ir >> 32) & 0x3ff;

	if (((m_ir >> 10) & 0x03) == 0x02)
	{
		// Write dispatch memory
		const u16 addr = (m_ir >> 12) & 0x7ff;
		const u32 a = m_a_mem[m_dispatch_constant];
		m_dpc[addr] = a & 0x3ffff;
		return;
	}
	u8 rotation = m_ir & 0x1f;

	// TODO Misc functions
	if (((m_ir >> 10) & 0x03) == 0x03) {
		if (BIT(m_ic, 29))
		{
			rotation = rotation ^ ((BIT(m_lc, 1) ^ BIT(m_lc, 0)) << 4);
			rotation = rotation ^ (BIT(m_lc, 0) << 3);
		}
		else
		{
			rotation = (rotation ^ (BIT(m_lc, 1) << 4)) ^ 0x10;
		}
		LOGMASKED(LOG_TRACE, "byte: mode 0x03, LC=%x(%o), rotation=%d\n", m_lc, m_lc, rotation);
	}
	else if ((m_ir >> 10) & 0x03)
	{
		fatalerror("%x(%o): dispatch misc function %d not implemented", m_prev_pc, m_prev_pc, (m_ir >> 10) & 0x03);
	}

	const u32 m = rotl_32(m_m, rotation) & dispatch_mask[(m_ir >> 5) & 0x07];
	u32 index = ((m_ir >> 12) & 0x7ff) | m;

	if ((m_ir >> 8) & 0x03) {
		const u8 l1 = m_vma_map_l1[(m_md >> 13) & 0x7ff];
		const u16 l2_index = (l1 << 5) | ((m_md >> 8) & 0x1f);
		const u32 l2 = m_vma_map_l2[l2_index] & 0xffffff;
		LOGMASKED(LOG_TRACE, "l2_index=%x(%o), l2=%x(%o)\n", l2_index, l2_index, l2, l2);

		if (BIT(m_ir, 8))
		{
			index |= BIT(l2, 18);
		}
		if (BIT(m_ir, 9))
		{
			index |= BIT(l2, 19);
		}
	}

	const u32 dispatch = m_dpc[index];
	m_n = BIT(dispatch, 14);

	LOGMASKED(LOG_TRACE, "dispatch m_source=%x(%o), m=%x(%o), index=%x(%o), dispatch=%x(%o) m_n=%s, rotation=%d\n", m_m, m_m, m, m, index, index, dispatch, dispatch, m_n ? "true" : "false", rotation);
	switch ((dispatch >> 15) & 0x03)
	{
	case 0x00: // jump
		m_next_pc = dispatch & 0x3fff;
		m_popj = false;
		break;
	case 0x01: // push pc on spc stack
		push_spc(m_n ? (BIT(m_ir, 25) ? m_pc - 1 : m_pc) : m_next_pc);
		m_next_pc = dispatch & 0x3fff;
		m_popj = false;
		break;
	case 0x02: // pop new pc from spc stack
		m_next_pc = pop_spc();
		if (BIT(m_next_pc, 14))
		{
			instruction_stream();
		}
		m_next_pc &= 0x3fff;
		m_popj = false;
		break;
	case 0x03: // fall-through
		break;
	}

	if (BIT(m_ir, 24))
	{
		u32 save_next_pc = m_next_pc;
		instruction_stream();
		m_next_pc = save_next_pc;
	}
}


void cadr_cpu_device::execute_byte()
{
	// TODO Misc functions
	u8 rotation = m_ir & 0x1f;
	if (((m_ir >> 10) & 0x03) == 0x03) {
		if (BIT(m_ic, 29))
		{
			rotation = rotation ^ ((BIT(m_lc, 1) ^ BIT(m_lc, 0)) << 4);
			rotation = rotation ^ (BIT(m_lc, 0) << 3);
		}
		else
		{
			rotation = (rotation ^ (BIT(m_lc, 1) << 4)) ^ 0x10;
		}
		LOGMASKED(LOG_TRACE, "byte: mode 0x03, LC=%x(%o), rotation=%d\n", m_lc, m_lc, rotation);
	}
	else if ((m_ir >> 10) & 0x03)
	{
		fatalerror("%x(%o): byte misc function %d not implemented", m_prev_pc, m_prev_pc, (m_ir >> 10) & 0x03);
	}

	u32 output = 0;
	if (m_ir & (3 << 12))
	{
		const u32 length = (m_ir >> 5) & 0x1f;
		const u32 r = BIT(m_ir, 12) ? rotl_32(m_m, rotation) : m_m;
		const u32 shift_right = BIT(m_ir, 13) ? rotation : 0;
		const u32 right_mask = 0xffffffff << shift_right;
		const u32 left_mask = 0xffffffff >> (31 - ((shift_right + length) & 0x1f));
		const u32 mask = right_mask & left_mask;

		output = (m_a & ~mask) | (r & mask);
		LOGMASKED(LOG_TRACE, "byte: a=%x(%o), m=%x(%o) left_mask=%x(%o), right_mask=%x(%o), mask=%x(%o), rotation=%x(%o), length=%x(%o), shift_right=%x(%o), r=%x(%o), output=%x(%o)\n", m_a, m_a, m_m, m_m, left_mask, left_mask, right_mask, right_mask, mask, mask, rotation, rotation, length, length, shift_right, shift_right, r, r, output, output);
	}

	write_destination(output);
}


void cadr_cpu_device::execute_run()
{
	do
	{
		if (m_read_delay)
		{
			if (!--m_read_delay)
			{
				m_md = m_read_data;
			}
		}

		if (!m_n)
		{
			debugger_instruction_hook(m_pc);
		}
		m_prev_pc = m_pc;
		m_pc = m_next_pc;
		u64 next_op = m_program.read_qword(m_next_pc++);

		m_ir |= m_oa_reg_lo;
		m_oa_reg_lo = 0;
		m_ir |= (u64(m_oa_reg_hi) << 26);
		m_oa_reg_hi = 0;

		if (!m_n)
		{
			LOGMASKED(LOG_TRACE, "%x(%o): IR: 0x%x(0%o), LC=0%o, MD=0%o\n", m_prev_pc, m_prev_pc, m_ir, m_ir, m_lc, m_md);

			m_popj = BIT(m_ir, 42);
			m_a = m_a_mem[(m_ir >> 32) & 0x3ff];
			get_m_source();

			switch (m_ir & (u64(3) << 43))
			{
			case u64(0) << 43: execute_alu(); break;
			case u64(1) << 43: execute_jump(); break;
			case u64(2) << 43: execute_dispatch(); break;
			case u64(3) << 43: execute_byte(); break;
			}
		}
		else
		{
			m_n = false;
		}

		m_ir = next_op;

		if (m_popj)
		{
			m_next_pc = pop_spc();
			if (BIT(m_next_pc, 14))
			{
				instruction_stream();
			}
			m_next_pc &= 0x3fff;
		}

		m_icount--;
	} while (m_icount > 0);
}


void cadr_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENPC:
		str = string_format("%05o" , m_pc);
		break;
	case CADR_DISPATCH_CONSTANT:
		str = string_format("%04o", m_dispatch_constant);
		break;
	case CADR_IC:
		str = string_format("%c%c%c%c",
			BIT(m_ic, 29) ? "B" : "W",
			BIT(m_ic, 28) ? "R" : "-",
			BIT(m_ic, 27) ? "I" : "-",
			BIT(m_ic, 26) ? "S" : "-"
		);
		break;
	case CADR_LC:
		str = string_format("%011o", m_lc);
		break;
	case CADR_MD:
		str = string_format("%011o", m_md);
		break;
	case CADR_N:
		str = m_n ? "true" : "false";
		break;
	case CADR_OA_REG_HI:
		str = string_format("%08o", m_oa_reg_hi);
		break;
	case CADR_OA_REG_LO:
		str = string_format("%09o", m_oa_reg_lo);
		break;
	case CADR_PDL_INDEX:
		str = string_format("%04o", m_pdl_index);
		break;
	case CADR_PDL_POINTER:
		str = string_format("%04o", m_pdl_pointer);
		break;
	case CADR_Q:
		str = string_format("%011o", m_q);
		break;
	case CADR_SPC_POINTER:
		str = string_format("%04o", m_spc_pointer);
		break;
	case CADR_VMA:
		str = string_format("%011o", m_vma);
		break;
	}
}


void cadr_cpu_device::execute_set_input(int inputnum, int state)
{
	if (inputnum == 0)
	{
		m_interrupt_pending = (state == ASSERT_LINE);
	}
}
