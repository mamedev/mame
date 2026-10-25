// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Panasonic MN1880 series

    This is a cycle-by-cycle emulation of the "8-Bit Dual Microcomputer"
    architecture, which has two independent CPUs sharing the same memory
    spaces and execution core, with execution phases interleaved when both
    CPUs are operating. Pipelining permits operand fetches to overlap with
    data writes.

    Known issues:
    * Only a generic device type and two specific models have been
      provided. Though the broad functional descriptions of many actual
      models are known, more detailed documentation is not easy to find.
    * Few internal special function registers have been emulated, again
      owing to lack of documentation. The MN1880023 and MN18801A
      peripherals are emulated only as far as CR-504 and PSR-500 firmware
      uses them, and their register assignments are inferred from it.
    * Instruction timings are based mostly on those documented for the
      MN1870, which lacks a number of MN1880 instructions and differs in
      some other important ways. Many have been guessed at.
    * Some instruction behavior, especially for repeated cases, has been
      guessed at and may not be strictly correct. It seems possible that
      many of the more complicated instructions are not supposed to be
      repeatable.
    * Interrupt behavior, including vectoring, has also been guessed at,
      though the specific sources of each interrupt remain largely
      unidentified. Logical interrupt request inputs are provided.
      Separate interrupt enable registers have been provided for each CPU,
      though these seem to share the same internal addresses. The WAIT
      operand is treated as a temporary wakeup mask; only 02 00 has been
      seen. Repeated instructions are fully
      interruptible. Interrupt acknowledgment has been implemented as a
      special invocation of the PI instruction; however, PI usage is
      nonexistent in extant code.
    * The output queue has been implemented only for memory writes, even
      though the MN1870 documentation shows it as applicable for
      instructions that do none of those. It may also be used to
      pipeline updates to XP, YP, SP or FS.
    * Every cycle fetches a byte from the instruction space, whether it
      is needed for execution or not. This fetching may not happen quite
      so continuously on actual hardware (a few dummy fetches may be
      unavoidable), but it is more or less continuous on some other
      microcontrollers with Harvard-like architectures such as MCS-51.
    * Data writes occur simultaneously with program fetches. Harvard
      architecture makes instruction and data spaces independent from the
      code's perspective, but simultaneous access is obviously impossible
      when both addresses are external since there is at most one external
      address bus. Contention should slow prefetching and execution down.
    * Additional wait states for external memory, if any, are not emulated.
    * The LP register likely defines some sort of stack limit. This has not
      been implemented.
    * When execution is stopped in the debugger, IP already points to the
      byte following the opcode which has been loaded into IR. This at
      least seems consistent with the prefetch model and the handling of
      repeated instructions.
    * There is no way to focus on one of the two CPUs in the debugger when
      both are executing. This is not an issue on systems that simply
      disable CPUb from the start.
    * The debugger will not single-step through repeated instructions.
      Making MAME's context-insensitive disassembler produce any sensible
      output for these would be very difficult.
    * When the debugger is stopped after an instruction, its execution may
      have loaded the output queue but not emptied it yet. Examining the
      contents of locations about to be written to may show misleading
      values. Likewise, PCs at which watchpoint hits occur may be
      incorrectly reported for writes.
    * Due to the pipelining of writes, an interrupt may be accepted during
      an instruction which attempts to disable interrupts by setting the
      IEMASK bit, which may be cleared instead when control reaches the next
      sequential instruction from a RETI. This sequencing glitch is
      documented in the MN187XX23 user's manual, along with a failsafe way
      of setting IEMASK, and similar workarounds in extant program code
      suggest it is likewise present in the MN1880 series.
    * The optional MMU, which expands the memory spaces in certain models
      which contain neither internal ROM nor RAM, has been emulated only
      to the extent required by psr500, though it likely has a few other
      features and quirks.
    * In DF mode, MOV (XP),(da) apparently takes the high byte of the
      direct address from YP, unlike the MN1870 documentation. MOV (da),(YP)
      also uses YP, as documented for the MN1870.

***************************************************************************/

#include "emu.h"
#include "mn1880.h"
#include "mn1880d.h"

#include "multibyte.h"

#include <algorithm>
#include <bit>
#include <iterator>

// device type definitions
DEFINE_DEVICE_TYPE(MN1880, mn1880_device, "mn1880", "Panasonic MN1880")
DEFINE_DEVICE_TYPE(MN1880023, mn1880023_device, "mn1880023", "Panasonic MN1880023")
DEFINE_DEVICE_TYPE(MN18801A, mn18801a_device, "mn18801a", "Panasonic MN18801A")

ALLOW_SAVE_TYPE(mn1880_device::microstate)

mn1880_device::mn1880_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool has_mmu, u16 internal_io_end, address_map_constructor data_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, has_mmu ? 21 : 16, 0, 16, 14)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, has_mmu ? 21 : 16, 0, 16, 14, data_map)
	, m_has_mmu(has_mmu)
	, m_internal_io_end(internal_io_end)
	, m_cpum(0)
	, m_ustate(microstate::UNKNOWN)
	, m_da(0)
	, m_tmp1(0)
	, m_tmp2(0)
	, m_output_queue_state(0xff)
	, m_icount(0)
	, m_input_state(0)
	, m_if(0)
	, m_irq(0)
	, m_mmu_bank{0, 0, 0, 0}
	, m_mmu_enable(0)
{
}

mn1880_device::mn1880_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mn1880_device(mconfig, MN1880, tag, owner, clock, false, 0x16, address_map_constructor(FUNC(mn1880_device::internal_data_map), this))
{
}

// requests latch on assertion; deasserting the input does not clear them
void mn1880_device::execute_set_input(int inputnum, int state)
{
	if (inputnum >= 0 && inputnum < 12)
	{
		const u16 mask = 1U << inputnum;
		if (state != CLEAR_LINE)
		{
			if (!(m_input_state & mask))
				m_if |= mask;
			m_input_state |= mask;
		}
		else
		{
			m_input_state &= ~mask;
		}
	}
}

u8 mn1880_device::ie0_r()
{
	return (get_active_cpu().ie & 0x00f) | (m_cpu[0].iemask ? 0x10 : 0) | (m_cpu[1].iemask ? 0x20 : 0);
}

void mn1880_device::ie0_w(u8 data)
{
	cpu_registers &cpu = output_queued() ? m_cpu[m_output_queue_state] : get_active_cpu();
	cpu.ie = (cpu.ie & 0xff0) | (data & 0x0f);
	m_cpu[0].iemask = BIT(data, 4);
	m_cpu[1].iemask = BIT(data, 5);
}

u8 mn1880_device::ie1_r()
{
	return (get_active_cpu().ie & 0xff0) >> 4;
}

void mn1880_device::ie1_w(u8 data)
{
	cpu_registers &cpu = output_queued() ? m_cpu[m_output_queue_state] : get_active_cpu();
	cpu.ie = u16(data) << 4 | (cpu.ie & 0x00f);
}

u8 mn1880_device::cpum_r()
{
	return m_cpum;
}

void mn1880_device::cpum_w(u8 data)
{
	m_cpum = (data & 0xef) | (m_cpum & 0x10);

	// clearing a stop bit also ends WAIT
	for (int i = 0; i < 2; ++i)
	{
		if (!BIT(data, i))
			m_cpu[i].wait = 0;
	}
}

u8 mn1880_device::mmu_bank_r(offs_t offset)
{
	return m_mmu_bank[offset];
}

void mn1880_device::mmu_bank_w(offs_t offset, u8 data)
{
	m_mmu_bank[offset] = data & 0x7f;
}

u8 mn1880_device::mmu_enable_r()
{
	return m_mmu_enable;
}

void mn1880_device::mmu_enable_w(u8 data)
{
	m_mmu_enable = data & 0xc0;
}

void mn1880_device::internal_data_map(address_map &map)
{
	map(0x0012, 0x0012).rw(FUNC(mn1880_device::ie0_r), FUNC(mn1880_device::ie0_w));
	map(0x0015, 0x0015).rw(FUNC(mn1880_device::ie1_r), FUNC(mn1880_device::ie1_w));
	map(0x0016, 0x0016).rw(FUNC(mn1880_device::cpum_r), FUNC(mn1880_device::cpum_w));
	if (m_has_mmu)
	{
		map(0x0040, 0x0043).rw(FUNC(mn1880_device::mmu_bank_r), FUNC(mn1880_device::mmu_bank_w));
		map(0x0044, 0x0044).rw(FUNC(mn1880_device::mmu_enable_r), FUNC(mn1880_device::mmu_enable_w));
	}
}

std::unique_ptr<util::disasm_interface> mn1880_device::create_disassembler()
{
	return std::make_unique<mn1880_disassembler>();
}

device_memory_interface::space_config_vector mn1880_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void mn1880_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_DATA).specific(m_data);
	set_icountptr(m_icount);

	using namespace std::placeholders;
	state_add<u16>(MN1880_IP, "IP",
		[this] () { return get_active_cpu().ip; },
		[this] (u16 data) { get_active_cpu().ip = data; }
	).noshow();
	state_add<u16>(STATE_GENPC, "GENPC",
		[this] () { return get_active_cpu().ip; },
		[this] (u16 data) { get_active_cpu().ip = data; }
	).noshow();
	state_add<u16>(STATE_GENPCBASE, "CURPC",
		[this] () { return get_active_cpu().irp; },
		[this] (u16 data) { get_active_cpu().irp = data; }
	).noshow();
	state_add<u8>(MN1880_IR, "IR",
		[this] () { return get_active_cpu().ir; },
		[this] (u8 data) { get_active_cpu().ir = data; }
	).noshow();
	state_add<u8>(MN1880_FS, "FS",
		[this] () { return get_active_cpu().fs; },
		[this] (u8 data) { get_active_cpu().fs = data; }
	).noshow();
	state_add<u8>(STATE_GENFLAGS, "FLAGS",
		[this] () { return get_active_cpu().fs; },
		[this] (u8 data) { get_active_cpu().fs = data; }
	).formatstr("%10s").noshow();
	state_add<u16>(MN1880_XP, "XP",
		[this] () { return get_active_cpu().xp; },
		[this] (u16 data) { get_active_cpu().xp = data; }
	).noshow();
	state_add<u16>(MN1880_YP, "YP",
		[this] () { return get_active_cpu().yp; },
		[this] (u16 data) { get_active_cpu().yp = data; }
	).noshow();
	state_add<u8>(MN1880_XPL, "XPl",
		[this] () { return get_active_cpu().xp & 0x00ff; },
		[this] (u8 data) { setl(get_active_cpu().xp, data); }
	).noshow();
	state_add<u8>(MN1880_XPH, "XPh",
		[this] () { return (get_active_cpu().xp & 0xff00 >> 8); },
		[this] (u8 data) { seth(get_active_cpu().xp, data); }
	).noshow();
	state_add<u8>(MN1880_YPL, "YPl",
		[this] () { return get_active_cpu().yp & 0x00ff; },
		[this] (u8 data) { setl(get_active_cpu().yp, data); }
	).noshow();
	state_add<u8>(MN1880_YPH, "YPh",
		[this] () { return (get_active_cpu().yp & 0xff00 >> 8); },
		[this] (u8 data) { seth(get_active_cpu().yp, data); }
	).noshow();
	state_add<u16>(MN1880_SP, "SP",
		[this] () { return get_active_cpu().sp; },
		[this] (u16 data) { get_active_cpu().sp = data; }
	).noshow();
	state_add<u16>(MN1880_LP, "LP",
		[this] () { return get_active_cpu().lp; },
		[this] (u16 data) { get_active_cpu().lp = data; }
	).noshow();
	state_add<u16>(MN1880_IE, "IE",
		[this] () { return get_active_cpu().ie; },
		[this] (u16 data) { get_active_cpu().ie = data; }
	).mask(0xfff).noshow();
	state_add<bool>(MN1880_IE, "IEMASK",
		[this] () { return get_active_cpu().iemask; },
		[this] (bool data) { get_active_cpu().iemask = data; }
	).noshow();

	for (int i = 0; i < 2; i++)
	{
		state_add(MN1880_IPA + i, util::string_format("IP%c", 'a' + i), m_cpu[i].ip).formatstr("%5s");
		state_add(MN1880_IRA + i, util::string_format("IR%c", 'a' + i), m_cpu[i].ir);
		state_add(MN1880_FSA + i, util::string_format("FS%c", 'a' + i), m_cpu[i].fs);
		state_add(MN1880_XPA + i, util::string_format("XP%c", 'a' + i), m_cpu[i].xp);
		state_add(MN1880_YPA + i, util::string_format("YP%c", 'a' + i), m_cpu[i].yp);
		state_add(MN1880_SPA + i, util::string_format("SP%c", 'a' + i), m_cpu[i].sp);
		state_add(MN1880_LPA + i, util::string_format("LP%c", 'a' + i), m_cpu[i].lp);
		state_add(MN1880_IEA + i, util::string_format("IE%c", 'a' + i), m_cpu[i].ie).mask(0xfff);
		state_add(MN1880_IEMASKA + i, util::string_format("IEMASK%c", 'a' + i), m_cpu[i].iemask);
		state_add_divider(MN1880_DIVIDER1 + i);
	}

	state_add(MN1880_IF, "IF", m_if).mask(0xfff);
	state_add(MN1880_CPUM, "CPUM", m_cpum);

	save_item(STRUCT_MEMBER(m_cpu, ip));
	save_item(STRUCT_MEMBER(m_cpu, irp));
	save_item(STRUCT_MEMBER(m_cpu, ir));
	save_item(STRUCT_MEMBER(m_cpu, fs));
	save_item(STRUCT_MEMBER(m_cpu, xp));
	save_item(STRUCT_MEMBER(m_cpu, yp));
	save_item(STRUCT_MEMBER(m_cpu, sp));
	save_item(STRUCT_MEMBER(m_cpu, lp));
	save_item(STRUCT_MEMBER(m_cpu, wait));
	save_item(STRUCT_MEMBER(m_cpu, ie));
	save_item(STRUCT_MEMBER(m_cpu, iemask));
	save_item(NAME(m_input_state));
	save_item(NAME(m_if));
	save_item(NAME(m_irq));
	save_item(NAME(m_cpum));
	save_item(NAME(m_ustate));
	save_item(NAME(m_da));
	save_item(NAME(m_tmp1));
	save_item(NAME(m_tmp2));
	save_item(NAME(m_output_queue_state));
	if (m_has_mmu)
	{
		save_item(NAME(m_mmu_bank));
		save_item(NAME(m_mmu_enable));
	}
}

void mn1880_device::device_reset()
{
	for (cpu_registers &cpu : m_cpu)
	{
		cpu.fs &= 0xc0; // CF & ZF might or might not be cleared as well
		cpu.fs |= 0x10; // HACK: skip fake first instruction in debugger
		cpu.ir = 0xf6;
		cpu.wait = 0;
		cpu.ie = 0;
		cpu.iemask = true;
	}
	m_cpu[0].ip = 0x0000;
	m_cpu[1].ip = 0x0020;

	// TBD: exactly what are SP and LP initialized to for each CPU?
	m_cpu[0].sp = 0x0100;
	m_cpu[0].lp = 0x0060;
	m_cpu[1].sp = 0x0200;
	m_cpu[1].lp = 0x0160;

	m_if = 0;
	m_irq = 0;
	m_cpum = 0x0c;
	m_ustate = microstate::NEXT;
	m_output_queue_state = 0xff;

	m_mmu_enable = 0;
}

bool mn1880_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);
	switch (spacenum)
	{
	case AS_PROGRAM:
		address = mmu_psen_translate(address);
		break;

	case AS_DATA:
		address = mmu_data_translate(address);
		break;
	}

	return true;
}

const mn1880_device::microstate mn1880_device::s_decode_map[256] =
{
	microstate::NOP_1, microstate::REP_1, microstate::REP_1, microstate::REP_1, microstate::REP_1, microstate::REP_1, microstate::REP_1, microstate::REP_1,
	microstate::REP_1, microstate::REP_1, microstate::REP_1, microstate::REP_1, microstate::REP_1, microstate::REP_1, microstate::REP_1, microstate::REP_1,
	microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1,
	microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1, microstate::CLRSET_1,
	microstate::T1B_1, microstate::T1B_1, microstate::T1B_1, microstate::T1B_1, microstate::T1B_1, microstate::T1B_1, microstate::T1B_1, microstate::T1B_1,
	microstate::T1B_1, microstate::T1B_1, microstate::T1B_1, microstate::T1B_1, microstate::T1B_1, microstate::T1B_1, microstate::T1B_1, microstate::T1B_1,
	microstate::UNKNOWN, microstate::MOVL31_1, microstate::UNKNOWN, microstate::MOVL31_1, microstate::MOVL34_1, microstate::MOVL35_1, microstate::MOV36_1, microstate::MOV37_1,
	microstate::MOVL38_1, microstate::MOVL39_1, microstate::MOVL38_1, microstate::MOVL39_1, microstate::ASL_1, microstate::ASL_1, microstate::ASR_1, microstate::ASR_1,
	microstate::DEC40_1, microstate::DEC40_1, microstate::NOT_1, microstate::NOT_1, microstate::CMPM44_1, microstate::CMPM45_1, microstate::XCH4_1, microstate::XCH4_1,
	microstate::INC48_1, microstate::INC48_1, microstate::CLR4A_1, microstate::CLR4A_1, microstate::ROL_1, microstate::ROL_1, microstate::ROR_1, microstate::ROR_1,
	microstate::CMPM50_1, microstate::DIV51_1, microstate::CMPM52_1, microstate::MOVDA_1, microstate::MOV54_1, microstate::MOV55_1, microstate::MOV56_1, microstate::MOV56_1,
	microstate::XCH58_1, microstate::MUL59_1, microstate::XCH58_1, microstate::UNKNOWN, microstate::MOVL5C_1, microstate::MOVL5D_1, microstate::MOVL5E_1, microstate::MOVL5F_1,
	microstate::CMP_1, microstate::CMP_1, microstate::CMP_1, microstate::CMP_1, microstate::AND_1, microstate::AND_1, microstate::AND_1, microstate::AND_1,
	microstate::XOR_1, microstate::XOR_1, microstate::XOR_1, microstate::XOR_1, microstate::OR_1, microstate::OR_1, microstate::OR_1, microstate::OR_1,
	microstate::SUBC_1, microstate::SUBC_1, microstate::SUBC_1, microstate::SUBC_1, microstate::SUBD_1, microstate::SUBD_1, microstate::SUBD_1, microstate::SUBD_1,
	microstate::ADDC_1, microstate::ADDC_1, microstate::ADDC_1, microstate::ADDC_1, microstate::ADDD_1, microstate::ADDD_1, microstate::ADDD_1, microstate::ADDD_1,
	microstate::BR80_1, microstate::BR80_1, microstate::BR80_1, microstate::BR80_1, microstate::CMPL_1, microstate::CLRFS_1, microstate::CMPL_1, microstate::CLRFS_1,
	microstate::BR88_1, microstate::BR88_1, microstate::BR88_1, microstate::BR88_1, microstate::UNKNOWN, microstate::SETFS_1, microstate::UNKNOWN, microstate::SETFS_1,
	microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1,
	microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1, microstate::CALL90_1,
	microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1,
	microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1, microstate::BRA0_1,
	microstate::UNKNOWN, microstate::POPFS_1, microstate::UNKNOWN, microstate::POPFS_1, microstate::POPB4_1, microstate::POPB5_1, microstate::POPB4_1, microstate::POPB7_1,
	microstate::UNKNOWN, microstate::PUSHFS_1, microstate::UNKNOWN, microstate::PUSHFS_1, microstate::PUSHBC_1, microstate::PUSHBD_1, microstate::PUSHBC_1, microstate::PUSHBF_1,
	microstate::SUBCL_1, microstate::DIVC1_1, microstate::SUBCL_1, microstate::UNKNOWN, microstate::XCHC4_1, microstate::XCHC5_1, microstate::XCHC4_1, microstate::XCHC5_1,
	microstate::ADDCL_1, microstate::MULC9_1, microstate::ADDCL_1, microstate::UNKNOWN, microstate::MOVCC_1, microstate::MOVCD_1, microstate::MOVCC_1, microstate::MOVCD_1,
	microstate::CMPD0_1, microstate::CMPD1_1, microstate::CMPD0_1, microstate::CMPD1_1, microstate::XCHD4_1, microstate::XCHD4_1, microstate::UNKNOWN, microstate::XCHD7_1,
	microstate::MOVD8_1, microstate::MOVD9_1, microstate::MOVD8_1, microstate::MOVD9_1, microstate::MOVDC_1, microstate::MOVDD_1, microstate::MOVDC_1, microstate::MOVDD_1,
	microstate::LOOP_1, microstate::LOOP_1, microstate::LOOP_1, microstate::LOOP_1, microstate::DECE4_1, microstate::INCE5_1, microstate::DECE4_1, microstate::INCE5_1,
	microstate::ADDRE8_1, microstate::ADDRE9_1, microstate::ADDRE8_1, microstate::ADDRE9_1, microstate::ADDREC_1, microstate::ADDRED_1, microstate::ADDREC_1, microstate::ADDRED_1,
	microstate::CMPBF0_1, microstate::CMPBF1_1, microstate::MOV1_1, microstate::WAIT_1, microstate::RET_1, microstate::RETI_1, microstate::BR_1, microstate::CALL_1,
	microstate::CMPBF0_1, microstate::CMPBF1_1, microstate::MOV1_1, microstate::PUSHFB_1, microstate::BRFC_1, microstate::CALLFD_1, microstate::RDTBL_1, microstate::PI_1
};

const u16 mn1880_device::s_input_queue_map[16] =
{
	//FEDCBA9876543210
	0b0000000000000000, // 0x
	0b1111111111111111, // 1x
	0b1111111111111111, // 2x
	0b1010010101111010, // 3x
	0b1010101010111010, // 4x
	0b1110010011101101, // 5x
	0b1110111011101110, // 6x
	0b1110111011101110, // 7x
	0b0000111101001111, // 8x
	0b0000000000000000, // 9x
	0b1111111111111111, // Ax
	0b0000000000000000, // Bx
	0b1111111010101110, // Cx
	0b0000111100001111, // Dx
	0b1010111100000000, // Ex
	0b0000111101001111  // Fx
};

const u8 mn1880_device::s_branch_fs[4] =
{
	0x00, 0x80, 0x40, 0xc0
};

u8 mn1880_device::cpu_registers::addcz(u8 data1, u8 data2, bool carry, bool holdz)
{
	if ((data1 + data2 + (carry ? 1 : 0)) >= 0x100)
		fs |= 0x80;
	else
		fs &= 0x7f;
	data1 += data2 + (carry ? 1 : 0);
	if (u8(data1) != 0)
		fs &= 0xbf;
	else if (!holdz)
		fs |= 0x40;
	return data1;
}

// 4-bit decimal add with carry (without adjust)
u8 mn1880_device::cpu_registers::adddcz(u8 data1, u8 data2, bool carry)
{
	if (((data1 & 0x0f) + (data2 & 0x0f) + (carry ? 1 : 0)) >= 0x0a)
		fs |= 0x80;
	else
		fs &= 0x7f;
	data1 = (data1 & 0xf0) | ((data1 + data2 + (carry ? 1 : 0)) & 0x0f);
	if (u8(data1 & 0x0f) == 0)
		fs |= 0x40;
	else
		fs &= 0xbf;
	return data1;
}

u8 mn1880_device::cpu_registers::subcz(u8 data1, u8 data2, bool carry, bool holdz)
{
	if (data1 < data2 + (carry ? 1 : 0))
		fs |= 0x80;
	else
		fs &= 0x7f;
	data1 -= data2 + (carry ? 1 : 0);
	if (u8(data1) != 0)
		fs &= 0xbf;
	else if (!holdz)
		fs |= 0x40;
	return data1;
}

// 4-bit decimal subtract with carry (without adjust)
u8 mn1880_device::cpu_registers::subdcz(u8 data1, u8 data2, bool carry)
{
	if ((data1 & 0x0f) < (data2 & 0x0f) + (carry ? 1 : 0))
		fs |= 0x80;
	else
		fs &= 0x7f;
	data1 = (data1 & 0xf0) | ((data1 - data2 - (carry ? 1 : 0)) & 0x0f);
	if (u8(data1 & 0x0f) == 0)
		fs |= 0x40;
	else
		fs &= 0xbf;
	return data1;
}

u8 mn1880_device::cpu_registers::rolc(u8 data)
{
	if (BIT(fs, 7))
	{
		if (!BIT(data, 7))
			fs &= 0x7f;
		return (data << 1) | 0x01;
	}
	else
	{
		if (BIT(data, 7))
			fs |= 0x80;
		return data << 1;
	}
}

u8 mn1880_device::cpu_registers::rorc(u8 data)
{
	if (BIT(fs, 7))
	{
		if (!BIT(data, 0))
			fs &= 0x7f;
		return (data >> 1) | 0x80;
	}
	else
	{
		if (BIT(data, 0))
			fs |= 0x80;
		return data >> 1;
	}
}

u8 mn1880_device::cpu_registers::asrc(u8 data)
{
	if (BIT(data, 0))
		fs |= 0x80;
	else
		fs &= 0x7f;
	return (data >> 1) | (data & 0x80);
}

void mn1880_device::cpu_registers::branch(u16 label)
{
	ip = label;
	fs &= 0xe0;
}

void mn1880_device::swap_cpus()
{
	if (!BIT(m_cpum, BIT(m_cpum, 4) ^ 1))
		m_cpum ^= 0x10;
}

void mn1880_device::next_instruction(u8 input)
{
	cpu_registers &cpu = get_active_cpu();
	if ((cpu.fs & 0x0f) != 0)
		cpu.fs = (cpu.fs - 1) | 0x10;
	else
	{
		cpu.fs &= 0xe0;
		cpu.irp = cpu.ip++;
		cpu.ir = input;
	}

	m_ustate = microstate::NEXT;
	swap_cpus();
}

void mn1880_device::execute_run()
{
	while (m_icount > 0)
	{
		--m_icount;

		// retire the queued write before sampling interrupts and CPUM
		const bool output_cycle = m_ustate == microstate::NEXT && output_queued();
		if (output_cycle)
		{
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1 & 0x00ff);
			m_output_queue_state = 0xff;
		}

		// an enabled interrupt wakes a stopped context
		for (int i = 0; i < 2; ++i)
		{
			if (!m_cpu[i].iemask && ((m_cpu[i].ie | m_cpu[i].wait) & m_if))
				m_cpum &= ~(1U << i);
		}

		if (m_ustate == microstate::NEXT && BIT(m_cpum, BIT(m_cpum, 4)))
		{
			swap_cpus();
			if (BIT(m_cpum, BIT(m_cpum, 4)))
				continue;
		}

		cpu_registers &cpu = get_active_cpu();

		if (m_ustate == microstate::NEXT)
		{
			m_irq = cpu.iemask ? 0 : (cpu.ie | cpu.wait) & m_if;
			if (m_irq != 0)
			{
				cpu.wait = 0;
				// REP has already fetched its opcode, so IP stays on the operand
				if (!(cpu.fs & 0x1f))
					--cpu.ip;
				if (!output_cycle)
					m_ustate = microstate::PI_1;
			}
			else if (cpu.wait == 0)
			{
				if (!BIT(cpu.fs, 4))
					debugger_instruction_hook(cpu.irp);
				if (!output_cycle && !BIT(s_input_queue_map[cpu.ir >> 4], cpu.ir & 0x0f))
					m_ustate = s_decode_map[cpu.ir];
			}
		}

		u8 input = m_cache.read_byte(mmu_psen_translate(cpu.ip));
		switch (m_ustate)
		{
		case microstate::NEXT:
			if (cpu.wait != 0)
			{
				swap_cpus();
			}
			else
			{
				if (m_irq != 0)
					m_ustate = microstate::PI_1;
				else
				{
					if (BIT(s_input_queue_map[cpu.ir >> 4], cpu.ir & 0x0f))
					{
						++cpu.ip;
						m_da = input;
					}
					m_ustate = s_decode_map[cpu.ir];
				}
			}
			break;

		case microstate::NOP_1:
			next_instruction(input);
			break;

		case microstate::REP_1:
			cpu.fs = (cpu.fs & 0xe0) | (cpu.ir & 0x0f);
			cpu.irp = cpu.ip++;
			cpu.ir = input;
			m_ustate = microstate::NEXT;
			break;

		case microstate::CLRSET_1:
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			if (m_da > m_internal_io_end)
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_ustate = microstate::CLRSET_2;
			break;

		case microstate::CLRSET_2:
			// on-chip registers are read and written in the same cycle (inferred)
			if (m_da <= m_internal_io_end)
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			if (BIT(cpu.ir, 3))
				m_tmp1 |= 1 << (cpu.ir & 0x07);
			else
				m_tmp1 &= ~(1 << (cpu.ir & 0x07));
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			next_instruction(input);
			break;

		case microstate::T1B_1:
			++cpu.ip;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da)) & (1 << (cpu.ir & 0x07));
			m_tmp2 = cpu.ip + s8(input);
			m_ustate = microstate::CMPBF1_3;
			break;

		case microstate::MOVL31_1:
			++cpu.ip;
			m_tmp1 = (m_da & 0xff) | u16(input) << 8;
			m_ustate = microstate::MOVL31_2;
			break;

		case microstate::MOVL31_2:
			if (BIT(cpu.ir, 1))
				cpu.yp = m_tmp1;
			else
				cpu.xp = m_tmp1;
			next_instruction(input);
			break;

		case microstate::MOVL34_1:
			if (BIT(cpu.fs, 5))
				m_da |= cpu.yp & 0xff00;
			m_ustate = microstate::MOVL34_2;
			break;

		case microstate::MOVL34_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_tmp2 = m_da + 1;
			m_da = cpu.xp;
			m_ustate = microstate::MOVL34_3;
			break;

		case microstate::MOVL34_3:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			m_da = m_tmp2;
			++cpu.xp;
			m_ustate = microstate::MOVL34_4;
			break;

		case microstate::MOVL34_4:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = cpu.xp;
			if ((cpu.fs & 0x1f) != 0)
				++cpu.xp;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::MOVL35_1:
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			m_tmp2 = m_da;
			m_da = cpu.yp;
			m_ustate = microstate::MOVL35_2;
			break;

		case microstate::MOVL35_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = m_tmp2;
			++cpu.yp;
			m_ustate = microstate::MOVL35_3;
			break;

		case microstate::MOVL35_3:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			m_da = cpu.yp;
			m_ustate = microstate::MOVL35_4;
			break;

		case microstate::MOVL35_4:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = m_tmp2 + 1;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::MOV36_1:
			if (BIT(cpu.fs, 5))
				m_da |= cpu.yp & 0xff00;
			m_ustate = microstate::MOV36_2;
			break;

		case microstate::MOV36_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = cpu.xp;
			if ((cpu.fs & 0x1f) != 0)
				++cpu.xp;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::MOV37_1:
			++cpu.ip;
			m_da = cpu.yp;
			if ((cpu.fs & 0x1f) != 0)
				++cpu.yp;
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = input;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.yp & 0xff00;
			m_ustate = microstate::MOV56_2;
			break;

		case microstate::MOVL38_1:
			if (BIT(cpu.fs, 5))
				m_da |= (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0xff00;
			m_ustate = microstate::MOVL38_2;
			break;

		case microstate::MOVL38_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			++m_da;
			m_ustate = microstate::MOVL38_3;
			break;

		case microstate::MOVL38_3:
			m_tmp1 |= m_data.read_byte(mmu_data_translate(m_da)) << 8;
			m_ustate = microstate::MOVL31_2; // TODO: output queue
			break;

		case microstate::MOVL39_1:
			++cpu.ip;
			m_da = input;
			if (BIT(cpu.fs, 5))
				m_da |= (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0xff00;
			m_ustate = microstate::MOVL39_2;
			break;

		case microstate::MOVL39_2:
			m_tmp1 = BIT(cpu.ir, 1) ? cpu.yp : cpu.xp;
			m_ustate = microstate::MOVL39_3;
			break;

		case microstate::MOVL39_3:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1 & 0x00ff);
			++m_da;
			m_tmp1 >>= 8;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::ASL_1:
			if (BIT(cpu.ir, 0))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
				m_da = cpu.xp;
			m_ustate = microstate::ASL_2;
			break;

		case microstate::ASL_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			cpu.fs = (m_tmp1 & 0x80) | (cpu.fs & 0x7f);
			m_tmp1 <<= 1;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::ASR_1:
			if (BIT(cpu.ir, 0))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
				m_da = cpu.xp;
			m_ustate = microstate::ASR_2;
			break;

		case microstate::ASR_2:
			m_tmp1 = cpu.asrc(m_data.read_byte(mmu_data_translate(m_da))); // TODO: read latch instead of terminal
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::DEC40_1:
			if (BIT(cpu.ir, 0))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_tmp1 = 0;
			if (!BIT(cpu.fs, 4))
				cpu.fs |= 0x80;
			m_ustate = microstate::SUBC_3;
			break;

		case microstate::NOT_1:
			if (BIT(cpu.ir, 0))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
				m_da = cpu.xp;
			m_tmp1 = 0xff;
			m_ustate = microstate::XOR_3;
			break;

		case microstate::CMPM44_1:
			m_tmp1 = m_da & 0x00ff;
			m_da = cpu.xp;
			m_ustate = microstate::CMPM44_2;
			break;

		case microstate::CMPM44_2:
			++cpu.ip;
			m_tmp1 &= m_data.read_byte(mmu_data_translate(m_da));
			m_tmp2 = input;
			m_ustate = microstate::CMPM44_3;
			break;

		case microstate::CMPM44_3:
			(void)cpu.subcz(m_tmp1, m_tmp2, false, false);
			m_ustate = microstate::NOP_1; // TODO: output queue (but what?)
			break;

		case microstate::CMPM45_1:
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			m_ustate = microstate::CMPM45_2;
			break;

		case microstate::CMPM45_2:
			++cpu.ip;
			m_tmp1 = input;
			m_ustate = microstate::CMPM44_2;
			break;

		case microstate::CMPM50_1:
			m_tmp2 = m_data.read_byte(mmu_data_translate(cpu.yp));
			m_tmp1 = m_da & 0x00ff;
			m_da = cpu.xp;
			m_ustate = microstate::CMPM50_2;
			break;

		case microstate::CMPM50_2:
			m_tmp2 &= m_tmp1;
			m_ustate = microstate::CMPM50_3;
			break;

		case microstate::CMPM50_3:
			m_tmp1 &= m_data.read_byte(mmu_data_translate(m_da));
			m_ustate = microstate::CMPM44_3;
			break;

		case microstate::CMPM52_1:
			++cpu.ip;
			m_tmp1 = input;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.yp & 0xff00;
			m_ustate = microstate::CMPM50_2;
			break;

		case microstate::CMPM52_2:
			++cpu.ip;
			m_tmp2 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = input;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			m_ustate = microstate::CMPM50_2;
			break;

		case microstate::XCH4_1:
			if (BIT(cpu.ir, 0))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::XCH4_2;
			break;

		case microstate::XCH4_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			m_tmp1 = (m_tmp1 << 4) | (m_tmp1 >> 4);
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::INC48_1:
			if (BIT(cpu.ir, 0))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_tmp1 = 0;
			if (!BIT(cpu.fs, 4))
				cpu.fs |= 0x80;
			m_ustate = microstate::ADDC_3;
			break;

		case microstate::CLR4A_1:
			if (BIT(cpu.ir, 0))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_tmp1 = 0;
			m_ustate = microstate::AND_3;
			break;

		case microstate::ROL_1:
			if (BIT(cpu.ir, 0))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::ROL_2;
			break;

		case microstate::ROL_2:
			m_tmp1 = cpu.rolc(m_data.read_byte(mmu_data_translate(m_da))); // TODO: read latch instead of terminal
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::ROR_1:
			if (BIT(cpu.ir, 0))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::ROR_2;
			break;

		case microstate::ROR_2:
			m_tmp1 = cpu.rorc(m_data.read_byte(mmu_data_translate(m_da))); // TODO: read latch instead of terminal
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::DIV51_1:
			m_da = cpu.yp;
			m_ustate = microstate::DIV51_2;
			break;

		case microstate::DIV51_2:
			m_tmp2 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = cpu.xp;
			m_ustate = microstate::DIV51_3;
			break;

		case microstate::DIV51_3:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			++m_da;
			m_ustate = microstate::DIV51_4;
			break;

		case microstate::DIV51_4:
			m_tmp1 |= m_data.read_byte(mmu_data_translate(m_da)) << 8;
			--m_da;
			m_ustate = microstate::DIV51_5;
			break;

		case microstate::DIV51_5:
			m_ustate = microstate::DIV51_6;
			break;

		case microstate::DIV51_6:
			m_ustate = microstate::DIV51_7;
			break;

		case microstate::DIV51_7:
			m_ustate = microstate::DIV51_8;
			break;

		case microstate::DIV51_8:
			m_ustate = microstate::DIV51_9;
			break;

		case microstate::DIV51_9:
			if (m_tmp2 == 0)
			{
				// TBD: documentation doesn't define what happens when the divisor is zero
				logerror("%04X: %04X divided by zero (XP = %04X, YP = %04X)\n", cpu.irp, m_tmp1, cpu.xp, cpu.yp);
				m_tmp1 = 0xff;
				m_tmp2 = 0xff;
			}
			else
				m_tmp1 /= std::exchange(m_tmp2, m_tmp1 % m_tmp2);
			m_ustate = microstate::DIV51_10;
			break;

		case microstate::DIV51_10:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1 & 0x00ff);
			m_tmp1 = m_tmp2; // remainder follows the quotient
			++m_da;
			cpu.xp = m_da;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::MOVDA_1:
			++cpu.ip;
			m_da = (m_da << 8) | input;
			m_ustate = microstate::MOVDA_2;
			break;

		case microstate::MOVDA_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			++cpu.ip;
			m_da = input;
			m_ustate = microstate::MOVDA_3;
			break;

		case microstate::MOVDA_3:
			++cpu.ip;
			m_da = (m_da << 8) | input;
			m_ustate = microstate::MOV56_2;
			break;

		case microstate::MOV54_1:
			m_da = cpu.yp;
			if ((cpu.fs & 0x1f) != 0)
				++cpu.yp;
			m_ustate = microstate::MOV36_2;
			break;

		case microstate::MOV55_1:
			m_data.write_byte(mmu_data_translate(cpu.xp), m_da & 0x00ff);
			if ((cpu.fs & 0x1f) != 0)
				++cpu.xp;
			next_instruction(input);
			break;

		case microstate::MOV56_1:
			++cpu.ip;
			if (BIT(cpu.ir, 0))
				m_tmp1 = m_da & 0x00ff;
			else
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.yp & 0xff00;
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			}
			m_da = input;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			m_ustate = microstate::MOV56_2;
			break;

		case microstate::MOV56_2:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			next_instruction(input);
			break;

		case microstate::XCH58_1:
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
				m_tmp2 = input;
			}
			else
			{
				m_da = cpu.yp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.yp;
			}
			m_ustate = microstate::XCH58_2;
			break;

		case microstate::XCH58_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			if (BIT(cpu.ir, 1))
			{
				std::swap(m_da, m_tmp2);
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_tmp2 = cpu.yp;
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::XCH58_3;
			break;

		case microstate::XCH58_3:
			m_tmp1 |= u16(m_data.read_byte(mmu_data_translate(m_da))) << 8; // TODO: read latch instead of terminal
			m_ustate = microstate::XCH58_4;
			break;

		case microstate::XCH58_4:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1 & 0x00ff);
			m_tmp1 >>= 8;
			m_da = m_tmp2;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::MUL59_1:
			m_da = cpu.xp;
			m_ustate = microstate::MUL59_2;
			break;

		case microstate::MUL59_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			++m_da;
			m_ustate = microstate::MUL59_3;
			break;

		case microstate::MUL59_3:
			m_tmp2 = m_data.read_byte(mmu_data_translate(m_da));
			m_ustate = microstate::MUL59_4;
			break;

		case microstate::MUL59_4:
			m_ustate = microstate::MUL59_5;
			break;

		case microstate::MUL59_5:
			m_ustate = microstate::MUL59_6;
			break;

		case microstate::MUL59_6:
			m_ustate = microstate::MUL59_7;
			break;

		case microstate::MUL59_7:
			m_tmp1 *= m_tmp2;
			m_da = cpu.xp;
			++cpu.xp;
			m_ustate = microstate::MUL59_8;
			break;

		case microstate::MUL59_8:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1 & 0x00ff);
			m_da = cpu.xp;
			if ((cpu.fs & 0x1f) != 0)
				++cpu.xp;
			m_tmp1 >>= 8;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::MOVL5C_1:
			m_da = cpu.yp;
			++cpu.yp;
			m_ustate = microstate::MOVL5C_2;
			break;

		case microstate::MOVL5C_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = cpu.xp;
			++cpu.xp;
			m_ustate = microstate::MOVL5C_3;
			break;

		case microstate::MOVL5C_3:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			m_da = cpu.yp;
			if ((cpu.fs & 0x1f) != 0)
				++cpu.yp;
			m_ustate = microstate::MOVL34_4;
			break;

		case microstate::MOVL5D_1:
			++cpu.ip;
			m_data.write_byte(mmu_data_translate(cpu.xp), m_da & 0x00ff);
			++cpu.xp;
			m_tmp1 = input;
			m_ustate = microstate::MOVL5D_2;
			break;

		case microstate::MOVL5D_2:
			m_data.write_byte(mmu_data_translate(cpu.xp), m_tmp1);
			if ((cpu.fs & 0x1f) != 0)
				++cpu.xp;
			next_instruction(input);
			break;

		case microstate::MOVL5E_1:
			++cpu.ip;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.yp & 0xff00;
			m_tmp2 = input;
			if (BIT(cpu.fs, 5))
				m_tmp2 |= cpu.xp & 0xff00;
			m_ustate = microstate::MOVL5E_2;
			break;

		case microstate::MOVL5E_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			std::swap(m_da, m_tmp2);
			m_ustate = microstate::MOVL5E_3;
			break;

		case microstate::MOVL5E_3:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			std::swap(m_da, m_tmp2);
			++m_da;
			m_ustate = microstate::MOVL5E_4;
			break;

		case microstate::MOVL5E_4:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = m_tmp2 + 1;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::MOVL5F_1:
			++cpu.ip;
			m_tmp1 = m_da & 0x00ff;
			m_da = input;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			m_ustate = microstate::MOVL5F_2;
			break;

		case microstate::MOVL5F_2:
			++cpu.ip;
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			m_tmp1 = input;
			++m_da;
			m_ustate = microstate::MOV56_2;
			break;

		case microstate::CMP_1:
			if (BIT(cpu.ir, 0))
				m_tmp1 = m_da & 0x00ff;
			else
			{
				if (BIT(cpu.ir, 1))
				{
					if (BIT(cpu.fs, 5))
						m_da |= cpu.yp & 0xff00;
				}
				else
				{
					m_da = cpu.yp;
					if ((cpu.fs & 0x1f) != 0)
						++cpu.yp;
				}
			}
			m_ustate = microstate::CMP_2;
			break;

		case microstate::CMP_2:
			if (!BIT(cpu.ir, 0))
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
				m_da = cpu.xp;
			}
			m_ustate = microstate::CMP_3;
			if (!BIT(cpu.fs, 4))
				cpu.fs &= 0x7f;
			break;

		case microstate::CMP_3:
			(void)cpu.subcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7), BIT(cpu.fs, 4));
			m_ustate = microstate::NOP_1; // TODO: output queue (but just what is the output?)
			break;

		case microstate::AND_1:
			if (BIT(cpu.ir, 0))
				m_tmp1 = m_da & 0x00ff;
			else
			{
				if (BIT(cpu.ir, 1))
				{
					if (BIT(cpu.fs, 5))
						m_da |= cpu.yp & 0xff00;
				}
				else
				{
					m_da = cpu.yp;
					if ((cpu.fs & 0x1f) != 0)
						++cpu.yp;
				}
			}
			m_ustate = microstate::AND_2;
			break;

		case microstate::AND_2:
			if (!BIT(cpu.ir, 0))
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::AND_3;
			break;

		case microstate::AND_3:
			m_tmp1 &= m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			if (u8(m_tmp1) == 0)
				cpu.fs |= 0x40;
			else
				cpu.fs &= 0xbf;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::XOR_1:
			if (BIT(cpu.ir, 0))
				m_tmp1 = m_da & 0x00ff;
			else
			{
				if (BIT(cpu.ir, 1))
				{
					if (BIT(cpu.fs, 5))
						m_da |= cpu.yp & 0xff00;
				}
				else
				{
					m_da = cpu.yp;
					if ((cpu.fs & 0x1f) != 0)
						++cpu.yp;
				}
			}
			m_ustate = microstate::XOR_2;
			break;

		case microstate::XOR_2:
			if (!BIT(cpu.ir, 0))
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::XOR_3;
			break;

		case microstate::XOR_3:
			m_tmp1 ^= m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			if (u8(m_tmp1) == 0)
				cpu.fs |= 0x40;
			else
				cpu.fs &= 0xbf;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::OR_1:
			if (BIT(cpu.ir, 0))
				m_tmp1 = m_da & 0x00ff;
			else
			{
				if (BIT(cpu.ir, 1))
				{
					if (BIT(cpu.fs, 5))
						m_da |= cpu.yp & 0xff00;
				}
				else
				{
					m_da = cpu.yp;
					if ((cpu.fs & 0x1f) != 0)
						++cpu.yp;
				}
			}
			m_ustate = microstate::OR_2;
			break;

		case microstate::OR_2:
			if (!BIT(cpu.ir, 0))
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::OR_3;
			break;

		case microstate::OR_3:
			m_tmp1 |= m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			if (u8(m_tmp1) == 0)
				cpu.fs |= 0x40;
			else
				cpu.fs &= 0xbf;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::SUBC_1:
			if (BIT(cpu.ir, 0))
				m_tmp1 = m_da & 0x00ff;
			else
			{
				if (BIT(cpu.ir, 1))
				{
					if (BIT(cpu.fs, 5))
						m_da |= cpu.yp & 0xff00;
				}
				else
				{
					m_da = cpu.yp;
					if ((cpu.fs & 0x1f) != 0)
						++cpu.yp;
				}
			}
			m_ustate = microstate::SUBC_2;
			break;

		case microstate::SUBC_2:
			if (!BIT(cpu.ir, 0))
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::SUBC_3;
			break;

		case microstate::SUBC_3:
			m_tmp1 = cpu.subcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7), BIT(cpu.fs, 4)); // TODO: read latch instead of terminal
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::SUBD_1:
			if (BIT(cpu.ir, 0))
				m_tmp1 = m_da & 0x00ff;
			else
			{
				if (BIT(cpu.ir, 1))
				{
					if (BIT(cpu.fs, 5))
						m_da |= cpu.yp & 0xff00;
				}
				else
				{
					m_da = cpu.yp;
					if ((cpu.fs & 0x1f) != 0)
						++cpu.yp;
				}
			}
			m_ustate = microstate::SUBD_2;
			break;

		case microstate::SUBD_2:
			if (!BIT(cpu.ir, 0))
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::SUBD_3;
			break;

		case microstate::SUBD_3:
			m_tmp1 = cpu.subdcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7)); // TODO: read latch instead of terminal
			m_ustate = microstate::SUBD_4;
			break;

		case microstate::SUBD_4:
			// Decimal adjust
			if (BIT(cpu.fs, 7))
			{
				m_tmp1 = (m_tmp1 & 0xf0) | ((m_tmp1 - 0x06) & 0x0f);
				if ((m_tmp1 & 0x0f) == 0)
					cpu.fs |= 0x40;
				else
					cpu.fs &= 0xbf;
			}
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::ADDC_1:
			if (BIT(cpu.ir, 0))
				m_tmp1 = m_da & 0x00ff;
			else
			{
				if (BIT(cpu.ir, 1))
				{
					if (BIT(cpu.fs, 5))
						m_da |= cpu.yp & 0xff00;
				}
				else
				{
					m_da = cpu.yp;
					if ((cpu.fs & 0x1f) != 0)
						++cpu.yp;
				}
			}
			m_ustate = microstate::ADDC_2;
			break;

		case microstate::ADDC_2:
			if (!BIT(cpu.ir, 0))
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::ADDC_3;
			break;

		case microstate::ADDC_3:
			m_tmp1 = cpu.addcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7), BIT(cpu.fs, 4)); // TODO: read latch instead of terminal
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::ADDD_1:
			if (BIT(cpu.ir, 0))
				m_tmp1 = m_da & 0x00ff;
			else
			{
				if (BIT(cpu.ir, 1))
				{
					if (BIT(cpu.fs, 5))
						m_da |= cpu.yp & 0xff00;
				}
				else
				{
					m_da = cpu.yp;
					if ((cpu.fs & 0x1f) != 0)
						++cpu.yp;
				}
			}
			m_ustate = microstate::ADDD_2;
			break;

		case microstate::ADDD_2:
			if (!BIT(cpu.ir, 0))
				m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
			{
				m_da = cpu.xp;
				if ((cpu.fs & 0x1f) != 0)
					++cpu.xp;
			}
			m_ustate = microstate::ADDD_3;
			break;

		case microstate::ADDD_3:
			m_tmp1 = cpu.adddcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7)); // TODO: read latch instead of terminal
			m_ustate = microstate::ADDD_4;
			break;

		case microstate::ADDD_4:
			// Decimal adjust
			if (BIT(cpu.fs, 7))
			{
				m_tmp1 = (m_tmp1 & 0xf0) | ((m_tmp1 + 0x06) & 0x0f);
				if ((m_tmp1 & 0x0f) == 0)
					cpu.fs |= 0x40;
				else
					cpu.fs &= 0xbf;
			}
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::BR80_1:
			if ((cpu.fs & s_branch_fs[cpu.ir & 0x03]) != 0)
				cpu.branch(cpu.ip + s8(m_da & 0x00ff));
			m_ustate = microstate::BR_2;
			break;

		case microstate::CMPL_1:
			if (BIT(cpu.ir, 1))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.yp & 0xff00;
			}
			else
				m_da = cpu.yp;
			m_ustate = microstate::CMPL_2;
			break;

		case microstate::CMPL_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_tmp2 = m_da;
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
				m_da = cpu.xp;
			m_ustate = microstate::CMPL_3;
			if (!BIT(cpu.fs, 4))
				cpu.fs &= 0x7f;
			break;

		case microstate::CMPL_3:
			(void)cpu.subcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7), BIT(cpu.fs, 4));
			std::swap(m_da, m_tmp2);
			++m_da;
			m_ustate = microstate::CMPL_4;
			break;

		case microstate::CMPL_4:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (!BIT(cpu.ir, 1))
				cpu.yp = m_da + 1;
			m_da = m_tmp2 + 1;
			m_ustate = microstate::CMPL_5;
			break;

		case microstate::CMPL_5:
			(void)cpu.subcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7), true);
			if (!BIT(cpu.ir, 1))
				cpu.xp = m_da + 1;
			m_ustate = microstate::NOP_1; // TODO: output queue (if any...)
			break;

		case microstate::CLRFS_1:
			cpu.fs &= ~(1 << (cpu.ir & 0x07));
			next_instruction(input);
			break;

		case microstate::BR88_1:
			if ((cpu.fs & s_branch_fs[cpu.ir & 0x03]) == 0)
				cpu.branch(cpu.ip + s8(m_da & 0x00ff));
			m_ustate = microstate::BR_2;
			break;

		case microstate::SETFS_1:
			cpu.fs |= 1 << (cpu.ir & 0x07);
			next_instruction(input);
			break;

		case microstate::SUBCL_1:
			if (BIT(cpu.ir, 1))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.yp & 0xff00;
			}
			else
				m_da = cpu.yp;
			m_ustate = microstate::SUBCL_2;
			break;

		case microstate::SUBCL_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_tmp2 = m_da;
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
				m_da = cpu.xp;
			m_ustate = microstate::SUBCL_3;
			break;

		case microstate::SUBCL_3:
			m_tmp1 = cpu.subcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7), BIT(cpu.fs, 4)); // TODO: read latch instead of terminal
			++m_tmp2;
			m_ustate = microstate::SUBCL_4;
			break;

		case microstate::SUBCL_4:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			std::swap(m_da, m_tmp2);
			m_ustate = microstate::SUBCL_5;
			break;

		case microstate::SUBCL_5:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (!BIT(cpu.ir, 1))
				cpu.yp = m_da + 1;
			m_da = m_tmp2 + 1;
			m_ustate = microstate::SUBCL_6;
			break;

		case microstate::SUBCL_6:
			m_tmp1 = cpu.subcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7), true); // TODO: read latch instead of terminal
			if (!BIT(cpu.ir, 1))
				cpu.xp = m_da + 1;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::ADDCL_1:
			if (BIT(cpu.ir, 1))
			{
				if (BIT(cpu.fs, 5))
					m_da |= cpu.yp & 0xff00;
			}
			else
				m_da = cpu.yp;
			m_ustate = microstate::ADDCL_2;
			break;

		case microstate::ADDCL_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_tmp2 = m_da;
			if (BIT(cpu.ir, 1))
			{
				++cpu.ip;
				m_da = input;
				if (BIT(cpu.fs, 5))
					m_da |= cpu.xp & 0xff00;
			}
			else
				m_da = cpu.xp;
			m_ustate = microstate::ADDCL_3;
			break;

		case microstate::ADDCL_3:
			m_tmp1 = cpu.addcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7), BIT(cpu.fs, 4)); // TODO: read latch instead of terminal
			++m_tmp2;
			m_ustate = microstate::ADDCL_4;
			break;

		case microstate::ADDCL_4:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			std::swap(m_da, m_tmp2);
			m_ustate = microstate::ADDCL_5;
			break;

		case microstate::ADDCL_5:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			if (!BIT(cpu.ir, 1))
				cpu.yp = m_da + 1;
			m_da = m_tmp2 + 1;
			m_ustate = microstate::ADDCL_6;
			break;

		case microstate::ADDCL_6:
			m_tmp1 = cpu.addcz(m_data.read_byte(mmu_data_translate(m_da)), m_tmp1, BIT(cpu.fs, 7), true); // TODO: read latch instead of terminal
			if (!BIT(cpu.ir, 1))
				cpu.xp = m_da + 1;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::CALL90_1:
			++cpu.ip;
			m_tmp1 = (cpu.ip & 0xf000) | u16(cpu.ir & 0x0f) << 8 | input;
			m_tmp2 = cpu.ip;
			m_da = cpu.sp - 1;
			m_ustate = microstate::CALL90_2;
			break;

		case microstate::CALL90_2:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp2 >> 8);
			--m_da;
			cpu.branch(m_tmp1);
			m_ustate = microstate::CALL_3;
			break;

		case microstate::BRA0_1:
			cpu.branch((cpu.ip & 0xf000) | u16(cpu.ir & 0x0f) << 8 | (m_da & 0x00ff));
			m_ustate = microstate::BR_2;
			break;

		case microstate::POPFS_1:
			m_da = cpu.sp;
			m_ustate = microstate::POPFS_2;
			break;

		case microstate::POPFS_2:
			cpu.fs = m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			cpu.sp = m_da + 1;
			m_ustate = microstate::NOP_1; // TODO: output queue (only FS?)
			break;

		case microstate::POPB4_1:
			m_da = cpu.sp;
			m_ustate = microstate::POPB4_2;
			break;

		case microstate::POPB4_2:
			(BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) = m_data.read_byte(mmu_data_translate(m_da));
			++m_da;
			m_ustate = microstate::POPB4_3;
			break;

		case microstate::POPB4_3:
			(BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) |= m_data.read_byte(mmu_data_translate(m_da)) << 8;
			cpu.sp = m_da + 1;
			m_ustate = microstate::NOP_1; // TODO: output queue (only XP/YP?)
			break;

		case microstate::POPB5_1:
			++cpu.ip;
			m_da = cpu.sp;
			m_tmp2 = input;
			if (BIT(cpu.fs, 5))
				m_tmp2 |= cpu.xp & 0xff00;
			m_ustate = microstate::POPB5_2;
			break;

		case microstate::POPB5_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			cpu.sp = m_da + 1;
			m_da = m_tmp2;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::POPB7_1:
			m_da = cpu.sp;
			m_tmp2 = cpu.xp;
			if ((cpu.fs & 0x1f) != 0)
				++cpu.xp;
			m_ustate = microstate::POPB5_2;
			break;

		case microstate::PUSHFS_1:
			m_da = cpu.sp - 1;
			m_tmp1 = cpu.fs;
			m_ustate = microstate::PUSHBC_3;
			break;

		case microstate::PUSHBC_1:
			m_da = cpu.sp - 1;
			m_tmp1 = BIT(cpu.ir, 1) ? cpu.yp : cpu.xp;
			m_ustate = microstate::PUSHBC_2;
			break;

		case microstate::PUSHBC_2:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1 >> 8);
			--m_da;
			m_ustate = microstate::PUSHBC_3;
			break;

		case microstate::PUSHBC_3:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1 & 0x00ff);
			cpu.sp = m_da;
			m_ustate = microstate::NOP_1; // TODO: output queue (only SP?)
			break;

		case microstate::PUSHBD_1:
			++cpu.ip;
			m_da = input;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			m_ustate = microstate::PUSHBD_2;
			break;

		case microstate::PUSHBD_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = cpu.sp - 1;
			m_ustate = microstate::PUSHBD_3;
			break;

		case microstate::PUSHBD_3:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1 & 0x00ff);
			cpu.sp = m_da;
			m_ustate = microstate::NOP_1; // TODO: output queue (only SP?)
			break;

		case microstate::PUSHBF_1:
			m_da = cpu.xp;
			if ((cpu.fs & 0x1f) != 0)
				--cpu.xp;
			m_ustate = microstate::PUSHBD_2;
			break;

		case microstate::DIVC1_1:
			m_tmp2 = m_da & 0x00ff;
			m_ustate = microstate::DIVC1_2;
			break;

		case microstate::DIVC1_2:
			m_da = cpu.xp;
			m_ustate = microstate::DIV51_3;
			break;

		case microstate::XCHC4_1:
			if (BIT(cpu.ir, 1))
				cpu.yp = (cpu.yp >> 8) | (cpu.yp << 8);
			else
				cpu.xp = (cpu.xp >> 8) | (cpu.xp << 8);
			m_ustate = microstate::XCHC4_2;
			break;

		case microstate::XCHC4_2:
			m_ustate = microstate::NOP_1; // TODO: output queue (only XPl/YPl?)
			break;

		case microstate::XCHC5_1:
			if (BIT(cpu.fs, 5))
				m_da |= (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0xff00;
			m_tmp2 = m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			m_tmp1 = (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0x00ff;
			m_ustate = microstate::XCHC5_2;
			break;

		case microstate::XCHC5_2:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			setl(BIT(cpu.ir, 1) ? cpu.yp : cpu.xp, m_tmp2);
			next_instruction(input);
			break;

		case microstate::MOVCC_1:
			if (BIT(cpu.fs, 5))
				m_da |= (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0xff00;
			m_tmp1 = (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0x00ff;
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::MOVCD_1:
			if (BIT(cpu.fs, 5))
				m_da |= (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0xff00;
			m_ustate = microstate::MOVCD_2;
			break;

		case microstate::MOVCD_2:
			setl(BIT(cpu.ir, 1) ? cpu.yp : cpu.xp, m_data.read_byte(mmu_data_translate(m_da)));
			next_instruction(input); // TODO: output queue
			break;

		case microstate::MULC9_1:
			m_tmp2 = m_da & 0x00ff;
			m_da = cpu.xp;
			m_ustate = microstate::MULC9_2;
			break;

		case microstate::MULC9_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_ustate = microstate::MULC9_3;
			break;

		case microstate::MULC9_3:
			m_ustate = microstate::MUL59_4;
			break;

		case microstate::CMPD0_1:
			if (BIT(cpu.fs, 5))
				m_da |= (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0xff00;
			m_ustate = microstate::CMPD0_2;
			break;

		case microstate::CMPD0_2:
			(void)cpu.subcz((BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0x00ff, m_data.read_byte(mmu_data_translate(m_da)), false, false);
			m_ustate = microstate::CMPD1_2;
			break;

		case microstate::CMPD1_1:
			(void)cpu.subcz((BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0x00ff, m_da & 0x00ff, false, false);
			m_ustate = microstate::CMPD1_2;
			break;

		case microstate::CMPD1_2:
			next_instruction(input);
			break;

		case microstate::XCHD4_1:
			m_tmp1 = std::exchange(BIT(cpu.ir, 0) ? cpu.sp : cpu.lp, cpu.xp);
			m_ustate = microstate::XCHD4_2;
			break;

		case microstate::XCHD4_2:
			cpu.xp = m_tmp1;
			next_instruction(input);
			break;

		case microstate::XCHD7_1:
			m_tmp1 = std::exchange(cpu.yp, cpu.xp);
			m_ustate = microstate::XCHD4_2;
			break;

		case microstate::MOVD8_1:
			setl(BIT(cpu.ir, 1) ? cpu.yp : cpu.xp, m_da & 0x00ff);
			next_instruction(input);
			break;

		case microstate::MOVD9_1:
			seth(BIT(cpu.ir, 1) ? cpu.yp : cpu.xp, m_da & 0x00ff);
			next_instruction(input);
			break;

		case microstate::MOVDC_1:
			if (BIT(cpu.ir, 1))
				cpu.yp = cpu.xp;
			else
				cpu.sp = cpu.xp;
			next_instruction(input);
			break;

		case microstate::MOVDD_1:
			if (BIT(cpu.ir, 1))
				cpu.xp = cpu.yp;
			else
				cpu.xp = cpu.sp;
			next_instruction(input);
			break;

		case microstate::LOOP_1:
			++cpu.ip;
			if (BIT(cpu.ir, 0))
			{
				if (BIT(cpu.ir, 1))
					++cpu.yp;
				else
					++cpu.xp;
			}
			m_da = cpu.sp;
			m_tmp2 = cpu.ip + s8(input);
			m_ustate = microstate::LOOP_2;
			break;

		case microstate::LOOP_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da)) - 1; // TODO: read latch instead of terminal
			if (m_tmp1 != 0)
				cpu.branch(m_tmp2);
			else
				cpu.sp = m_da + 1;
			m_ustate = microstate::MOV56_2;
			break;

		case microstate::DECE4_1:
			if (BIT(cpu.ir, 1))
				--cpu.yp;
			else
				--cpu.xp;
			next_instruction(input);
			break;

		case microstate::INCE5_1:
			if (BIT(cpu.ir, 1))
				++cpu.yp;
			else
				++cpu.xp;
			next_instruction(input);
			break;

		case microstate::ADDRE8_1:
			++cpu.ip;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.yp & 0xff00;
			m_tmp2 = input;
			m_ustate = microstate::ADDRE8_2;
			break;

		case microstate::ADDRE8_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_da = m_tmp2;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			m_ustate = microstate::ADDRE8_3;
			break;

		case microstate::ADDRE8_3:
			if (BIT(cpu.ir, 1))
				setl(cpu.yp, cpu.addcz(m_tmp1, m_data.read_byte(mmu_data_translate(m_da)), false, false));
			else
				setl(cpu.xp, cpu.addcz(m_tmp1, m_data.read_byte(mmu_data_translate(m_da)), false, false));
			m_ustate = microstate::NOP_1; // TODO: output queue (XPl only?)
			break;

		case microstate::ADDRE9_1:
			++cpu.ip;
			m_tmp2 = m_da & 0x00ff;
			m_da = input;
			if (BIT(cpu.fs, 5))
				m_da |= (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0xff00;
			m_ustate = microstate::ADDRE9_2;
			break;

		case microstate::ADDRE9_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			m_ustate = microstate::ADDRE9_3;
			break;

		case microstate::ADDRE9_3:
			setl(BIT(cpu.ir, 1) ? cpu.yp : cpu.xp, cpu.addcz(m_tmp1, m_tmp2, false, false));
			m_ustate = microstate::NOP_1; // TODO: output queue (XPl/YPl only?)
			break;

		case microstate::ADDREC_1:
			m_tmp1 = (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0x00ff;
			m_tmp2 = (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) >> 8;
			m_ustate = microstate::ADDRE9_3;
			break;

		case microstate::ADDRED_1:
			m_tmp1 = (BIT(cpu.ir, 1) ? cpu.yp : cpu.xp) & 0x00ff;
			m_tmp2 = m_da & 0x00ff;
			m_ustate = microstate::ADDRE9_3;
			break;

		case microstate::CMPBF0_1:
			++cpu.ip;
			m_tmp1 = m_data.read_byte(mmu_data_translate(cpu.xp)) ^ (m_da & 0x00ff);
			m_tmp2 = cpu.ip + s8(input);
			m_ustate = microstate::CMPBF1_3;
			break;

		case microstate::CMPBF1_1:
			++cpu.ip;
			m_tmp1 = m_da & 0x00ff;
			m_da = input;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			m_ustate = microstate::CMPBF1_2;
			break;

		case microstate::CMPBF1_2:
			++cpu.ip;
			m_tmp1 ^= m_data.read_byte(mmu_data_translate(m_da));
			m_tmp2 = cpu.ip + s8(input);
			m_ustate = microstate::CMPBF1_3;
			break;

		case microstate::CMPBF1_3:
			if (BIT(cpu.ir, 3) ? m_tmp1 == 0 : m_tmp1 != 0)
				cpu.branch(m_tmp2);
			m_ustate = microstate::BR_2;
			break;

		case microstate::MOV1_1:
			++cpu.ip;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.yp & 0xff00;
			m_tmp2 = input;
			m_tmp1 = 1 << (m_tmp2 & 0x07);
			m_ustate = microstate::MOV1_2;
			break;

		case microstate::MOV1_2:
			++cpu.ip;
			m_tmp1 &= m_data.read_byte(mmu_data_translate(m_da));
			m_da = input;
			if (BIT(cpu.fs, 5))
				m_da |= cpu.xp & 0xff00;
			m_ustate = microstate::MOV1_3;
			break;

		case microstate::MOV1_3:
			if (BIT(cpu.ir, 3) ? m_tmp1 == 0 : m_tmp1 != 0)
				m_ustate = microstate::MOV1_4;
			else
				m_ustate = microstate::MOV1N_4;
			m_tmp1 = 1 << ((m_tmp2 >> 4) & 0x07);
			break;

		case microstate::MOV1_4:
			m_tmp1 |= m_data.read_byte(mmu_data_translate(m_da)); // TODO: read latch instead of terminal
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::MOV1N_4:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da)) & ~m_tmp1; // TODO: read latch instead of terminal
			set_output_queued();
			next_instruction(input);
			break;

		case microstate::WAIT_1:
			++cpu.ip;
			// TODO: operand treated as a wakeup interrupt mask; byte order and layout unverified
			m_tmp1 = m_da | (input << 8);
			m_ustate = microstate::WAIT_2;
			break;

		case microstate::WAIT_2:
			cpu.wait = m_tmp1;
			m_cpum |= 1U << BIT(m_cpum, 4);
			next_instruction(input);
			break;

		case microstate::RET_1:
			m_da = cpu.sp;
			m_ustate = microstate::RET_2;
			break;

		case microstate::RET_2:
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			++m_da;
			m_ustate = microstate::RET_3;
			break;

		case microstate::RET_3:
			cpu.branch(u16(m_data.read_byte(mmu_data_translate(m_da))) << 8 | m_tmp1);
			cpu.sp = m_da + 1;
			m_ustate = microstate::BR_2;
			break;

		case microstate::RETI_1:
			m_da = cpu.sp;
			m_ustate = microstate::RETI_2;
			break;

		case microstate::RETI_2:
			cpu.fs = m_data.read_byte(mmu_data_translate(m_da));
			++m_da;
			m_ustate = microstate::RETI_3;
			break;

		case microstate::RETI_3:
			cpu.iemask = false;
			m_tmp1 = m_data.read_byte(mmu_data_translate(m_da));
			++m_da;
			m_ustate = microstate::RETI_4;
			break;

		case microstate::RETI_4:
			cpu.ip = u16(m_data.read_byte(mmu_data_translate(m_da))) << 8 | m_tmp1;
			if ((cpu.fs & 0x1f) != 0)
			{
				++m_da;
				m_ustate = microstate::RETI_5;
			}
			else
			{
				cpu.sp = m_da + 1;
				m_ustate = microstate::BR_2;
			}
			break;

		case microstate::RETI_5:
			cpu.ir = m_data.read_byte(mmu_data_translate(m_da));
			cpu.sp = m_da + 1;
			m_ustate = microstate::NEXT;
			swap_cpus();
			break;

		case microstate::BR_1:
			cpu.branch((m_da & 0x00ff) << 8 | input);
			m_ustate = microstate::BR_2;
			break;

		case microstate::BR_2:
			next_instruction(input);
			break;

		case microstate::CALL_1:
			++cpu.ip;
			m_tmp1 = input;
			m_tmp2 = cpu.ip + 1;
			m_da = cpu.sp - 1;
			m_ustate = microstate::CALL_2;
			break;

		case microstate::CALL_2:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp2 >> 8);
			--m_da;
			cpu.branch(m_tmp1 << 8 | input);
			m_ustate = microstate::CALL_3;
			break;

		case microstate::CALL_3:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp2 & 0x00ff);
			cpu.sp = m_da;
			next_instruction(input);
			break;

		case microstate::PUSHFB_1:
			m_tmp1 = m_da & 0x00ff;
			m_da = cpu.sp - 1;
			m_ustate = microstate::PUSHBC_3;
			break;

		case microstate::BRFC_1:
			cpu.branch(cpu.xp);
			m_ustate = microstate::BR_2;
			break;

		case microstate::CALLFD_1:
			m_tmp1 = cpu.xp;
			m_tmp2 = cpu.ip;
			m_da = cpu.sp - 1;
			m_ustate = microstate::CALL90_2;
			break;

		case microstate::RDTBL_1:
			m_ustate = microstate::RDTBL_2;
			break;

		case microstate::RDTBL_2:
			m_tmp2 = std::exchange(cpu.ip, cpu.yp);
			if ((cpu.fs & 0x1f) != 0)
				++cpu.yp;
			m_ustate = microstate::RDTBL_3;
			break;

		case microstate::RDTBL_3:
			m_tmp1 = input;
			m_da = cpu.xp;
			if ((cpu.fs & 0x1f) != 0)
				++cpu.xp;
			cpu.ip = m_tmp2;
			m_ustate = microstate::MOV56_2;
			break;

		case microstate::PI_1:
			m_tmp2 = cpu.ip;
			if (m_irq != 0)
			{
				// IRQ0 (first of four external edge inputs?) has the highest priority (after RESET)
				unsigned level = std::bit_width((m_irq - 1U) & ~m_irq);
				(void)standard_irq_callback(level, cpu.ip);
				cpu.ie &= ~(1 << level); // No separate in-service lockout; handler must re-enable specific interrupt
				m_if &= ~(1 << level);
				cpu.iemask = true;
				cpu.ip = 0x0004 + (level << 1);
			}
			else
				cpu.ip = 0x0002; // PI vector unconfirmed
			if (BIT(m_cpum, 4))
				cpu.ip |= 0x0020;
			m_da = cpu.sp - 1;
			if ((cpu.fs & 0x1f) != 0)
			{
				m_tmp1 = cpu.ir;
				m_ustate = microstate::PI_2;
			}
			else
				m_ustate = microstate::PI_3;
			break;

		case microstate::PI_2:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp1);
			--m_da;
			m_ustate = microstate::PI_3;
			break;

		case microstate::PI_3:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp2 >> 8);
			--m_da;
			m_tmp1 = u16(input) << 8;
			++cpu.ip;
			m_ustate = microstate::PI_4;
			break;

		case microstate::PI_4:
			m_data.write_byte(mmu_data_translate(m_da), m_tmp2 & 0x00ff);
			--m_da;
			m_tmp2 = cpu.fs;
			cpu.branch(m_tmp1 | input); // FS repeat bits are cleared here
			m_ustate = microstate::CALL_3;
			break;

		case microstate::UNKNOWN:
			logerror("%04X: Unknown or unemulated instruction %02X encountered\n", cpu.irp, cpu.ir);
			next_instruction(input);
			break;
		}
	}
}

void mn1880_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
	{
		u8 fs = get_active_cpu().fs;
		str = string_format("%c%c%c%c RC=%-2d",
				BIT(fs, 7) ? 'C' : '.', // Carry flag
				BIT(fs, 6) ? 'Z' : '.', // Zero flag
				BIT(fs, 5) ? 'D' : '.', // Direct flag
				BIT(fs, 4) ? 'A' : '.', // Auto-repeat flag
				fs & 0x0f);
		break;
	}

	case MN1880_IPA:
		str = string_format("%04X%c", m_cpu[0].ip, BIT(m_cpum, 4) ? ' ' : '*');
		break;

	case MN1880_IPB:
		str = string_format("%04X%c", m_cpu[1].ip, BIT(m_cpum, 4) ? '*' : ' ');
		break;
	}
}


//**************************************************************************
//  MN1880023
//**************************************************************************

mn1880023_device::mn1880023_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mn1880_device(mconfig, MN1880023, tag, owner, clock, false, 0x5f, address_map_constructor(FUNC(mn1880023_device::internal_map), this))
	, m_port_in_cb(*this, 0xff)
	, m_port_out_cb(*this)
	, m_ad_in_cb(*this, 0)
	, m_sio_in_cb(*this, 0xff)
	, m_sio_out_cb(*this)
	, m_pwm_out_cb(*this)
	, m_timer0(nullptr)
	, m_timer3(nullptr)
	, m_sio_timer{nullptr, nullptr}
	, m_adc_timer(nullptr)
	, m_pwm_timer(nullptr)
	, m_port_input_mask{}
	, m_regs{}
	, m_timer2_start(attotime::zero)
	, m_timer4_count(0)
	, m_timer4_prescale(0)
	, m_timer2_capture_input(false)
	, m_timer4_count_input(false)
{
}

void mn1880023_device::internal_map(address_map &map)
{
	map(0x0000, 0x005f).rw(FUNC(mn1880023_device::reg_r), FUNC(mn1880023_device::reg_w));
	map(0x0000, 0x0009).rw(FUNC(mn1880023_device::port_r), FUNC(mn1880023_device::port_w));
	map(0x0014, 0x0014).w(FUNC(mn1880023_device::irq_flags_w));
	map(0x001a, 0x001a).w(FUNC(mn1880023_device::sio0_data_w));
	map(0x001e, 0x001e).w(FUNC(mn1880023_device::adc_control_w));
	map(0x0020, 0x0021).rw(FUNC(mn1880023_device::timer0_r), FUNC(mn1880023_device::timer0_w));
	map(0x0024, 0x0025).rw(FUNC(mn1880023_device::timer2_r), FUNC(mn1880023_device::timer2_w));
	map(0x0026, 0x0027).w(FUNC(mn1880023_device::timer3_w));
	map(0x0031, 0x0031).w(FUNC(mn1880023_device::timer_run_w));
	map(0x0033, 0x0033).w(FUNC(mn1880023_device::timer_control_w));
	map(0x003a, 0x003a).w(FUNC(mn1880023_device::sio1_data_w));
	map(0x004e, 0x004f).r(FUNC(mn1880023_device::timer4_r));
	map(0x0050, 0x0051).w(FUNC(mn1880023_device::pwm_reload_w));
	map(0x0055, 0x0055).w(FUNC(mn1880023_device::pwm_control_w));
	map(0x0060, 0x031f).ram();
	mn1880_device::internal_data_map(map);
}

void mn1880023_device::device_start()
{
	mn1880_device::device_start();

	m_timer0 = timer_alloc(FUNC(mn1880023_device::timer0_tick), this);
	m_timer3 = timer_alloc(FUNC(mn1880023_device::timer3_tick), this);
	m_sio_timer[0] = timer_alloc(FUNC(mn1880023_device::sio0_tick), this);
	m_sio_timer[1] = timer_alloc(FUNC(mn1880023_device::sio1_tick), this);
	m_adc_timer = timer_alloc(FUNC(mn1880023_device::adc_tick), this);
	m_pwm_timer = timer_alloc(FUNC(mn1880023_device::pwm_tick), this);

	save_item(NAME(m_regs));
	save_item(NAME(m_timer2_start));
	save_item(NAME(m_timer4_count));
	save_item(NAME(m_timer4_prescale));
	save_item(NAME(m_timer2_capture_input));
	save_item(NAME(m_timer4_count_input));
}

void mn1880023_device::device_reset()
{
	mn1880_device::device_reset();

	// port pin levels at reset are unknown, so no outputs are driven here
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_timer0->adjust(attotime::never);
	m_timer3->adjust(attotime::never);
	m_sio_timer[0]->adjust(attotime::never);
	m_sio_timer[1]->adjust(attotime::never);
	m_adc_timer->adjust(attotime::never);
	m_pwm_timer->adjust(attotime::never);
	m_timer2_start = machine().time();
	m_timer4_count = 0;
	m_timer4_prescale = 0;
}

u8 mn1880023_device::reg_r(offs_t offset)
{
	return m_regs[offset];
}

void mn1880023_device::reg_w(offs_t offset, u8 data)
{
	m_regs[offset] = data;
}

u8 mn1880023_device::port_r(offs_t offset)
{
	const u8 mask = m_port_input_mask[offset];
	if (!mask)
		return m_regs[offset];

	return (m_regs[offset] & ~mask) | (m_port_in_cb[offset]() & mask);
}

void mn1880023_device::port_w(offs_t offset, u8 data)
{
	m_regs[offset] = data;
	m_port_out_cb[offset](data);
}

void mn1880023_device::irq_flags_w(u8 data)
{
	// writing 0 to flag bit 0, 2 or 4 also clears request 4, 6 or 8
	m_regs[0x14] = data;
	clear_interrupt_requests((~data & 0x15) << 4);
}

void mn1880023_device::sio0_data_w(u8 data)
{
	m_regs[0x1a] = data;
	m_regs[0x18] |= 0x80;
	m_regs[0x14] &= ~0x20;
	m_sio_timer[0]->adjust(clocks_to_attotime(64));
}

TIMER_CALLBACK_MEMBER(mn1880023_device::sio0_tick)
{
	m_regs[0x18] &= ~0x80;
	m_regs[0x14] |= 0x20;

	// only the transmit (a6) and receive (af) modes used by CR-504 firmware are known
	if (m_regs[0x19] == 0xa6)
		m_sio_out_cb[0](m_regs[0x1a]);
	else if (m_regs[0x19] == 0xaf)
		m_regs[0x1b] = m_sio_in_cb[0]();
}

void mn1880023_device::sio1_data_w(u8 data)
{
	m_regs[0x3a] = data;
	m_regs[0x37] &= ~0x10;
	m_sio_timer[1]->adjust(clocks_to_attotime(64));
}

TIMER_CALLBACK_MEMBER(mn1880023_device::sio1_tick)
{
	m_sio_out_cb[1](m_regs[0x3a]);
	m_regs[0x3b] = m_sio_in_cb[1]();
	m_regs[0x37] |= 0x10;
}

void mn1880023_device::adc_control_w(u8 data)
{
	m_regs[0x1e] = data;
	if (BIT(data, 3))
		m_adc_timer->adjust(clocks_to_attotime(64));
}

TIMER_CALLBACK_MEMBER(mn1880023_device::adc_tick)
{
	m_regs[0x1c] = m_ad_in_cb[m_regs[0x1d] & 7]();
	m_regs[0x37] |= 0x08;
}

u8 mn1880023_device::timer0_r(offs_t offset)
{
	if (!m_timer0->enabled())
		return m_regs[0x20 + offset];

	const u16 count = 0x10000 - (m_timer0->remaining().as_ticks(clock()) + 15) / 16;
	return offset ? (count >> 8) : count;
}

void mn1880023_device::timer0_w(offs_t offset, u8 data)
{
	m_regs[0x20 + offset] = data;
	update_timer0();
}

void mn1880023_device::update_timer0()
{
	// only the periodic mode (control 8a) used by CR-504 firmware is emulated
	if (BIT(m_regs[0x31], 4) && (m_regs[0x33] & 0x9f) == 0x8a)
	{
		const attotime period = clocks_to_attotime(16 * (0x10000 - get_u16le(&m_regs[0x20])));
		m_timer0->adjust(period, 0, period);
	}
	else
	{
		m_timer0->adjust(attotime::never);
	}
}

TIMER_CALLBACK_MEMBER(mn1880023_device::timer0_tick)
{
	request_interrupt(IRQ_TIMER0);
}

u16 mn1880023_device::timer2_count() const
{
	const u16 count = get_u16le(&m_regs[0x24]);
	if (!BIT(m_regs[0x31], 6))
		return count;

	return count + (machine().time() - m_timer2_start).as_ticks(clock() / 16);
}

u8 mn1880023_device::timer2_r(offs_t offset)
{
	const u16 count = timer2_count();
	return offset ? (count >> 8) : count;
}

void mn1880023_device::timer2_w(offs_t offset, u8 data)
{
	m_regs[0x24 + offset] = data;
	m_timer2_start = machine().time();
}

void mn1880023_device::timer2_capture_w(int state)
{
	if (state && !m_timer2_capture_input)
		put_u16le(&m_regs[0x2c], timer2_count());
	m_timer2_capture_input = state;
}

void mn1880023_device::timer3_w(offs_t offset, u8 data)
{
	m_regs[0x26 + offset] = data;
	update_timer3();
}

void mn1880023_device::update_timer3()
{
	// TODO: only the first overflow of a programmed delay is emulated
	if (BIT(m_regs[0x31], 7))
		m_timer3->adjust(clocks_to_attotime(16 * (0x10000 - get_u16le(&m_regs[0x26]))));
	else
		m_timer3->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(mn1880023_device::timer3_tick)
{
	m_regs[0x14] |= 0x04;
	m_regs[0x37] |= 0x02;
	if (BIT(m_regs[0x36], 1))
		request_interrupt(IRQ_TIMER3);
}

void mn1880023_device::timer_run_w(u8 data)
{
	const u8 changed = m_regs[0x31] ^ data;
	m_regs[0x31] = data;
	if (BIT(changed, 6))
		m_timer2_start = machine().time();
	if (BIT(changed, 4))
		update_timer0();
	if (BIT(changed, 7))
		update_timer3();
}

void mn1880023_device::timer_control_w(u8 data)
{
	const u8 changed = m_regs[0x33] ^ data;
	m_regs[0x33] = data;
	if (changed & 0x9f)
		update_timer0();
	if (BIT(changed & data, 6))
	{
		m_timer4_count = get_u16le(&m_regs[0x4c]);
		m_timer4_prescale = 0;
	}
}

u8 mn1880023_device::timer4_r(offs_t offset)
{
	return offset ? (m_timer4_count >> 8) : m_timer4_count;
}

void mn1880023_device::timer4_count_w(int state)
{
	// counts every second input edge (inferred)
	if (state && !m_timer4_count_input && BIT(m_regs[0x33], 6) && ++m_timer4_prescale == 2)
	{
		m_timer4_prescale = 0;
		if (!++m_timer4_count)
		{
			m_regs[0x14] |= 0x10;
			request_interrupt(IRQ_TIMER4);
		}
	}
	m_timer4_count_input = state;
}

u16 mn1880023_device::pwm_reload() const
{
	return get_u16le(&m_regs[0x50]) & 0x3ff;
}

u16 mn1880023_device::pwm_compare() const
{
	return get_u16le(&m_regs[0x52]) & 0x3ff;
}

void mn1880023_device::pwm_reload_w(offs_t offset, u8 data)
{
	m_regs[0x50 + offset] = data;
	update_pwm();
}

void mn1880023_device::pwm_control_w(u8 data)
{
	const u8 changed = m_regs[0x55] ^ data;
	m_regs[0x55] = data;
	if (BIT(changed, 0))
		update_pwm();
}

void mn1880023_device::update_pwm()
{
	// 10-bit counter clocked at oscillator/4
	if (BIT(m_regs[0x55], 0))
	{
		const attotime period = clocks_to_attotime(4 * (0x400 - pwm_reload()));
		m_pwm_timer->adjust(period, 0, period);
	}
	else
	{
		m_pwm_timer->adjust(attotime::never);
	}
}

TIMER_CALLBACK_MEMBER(mn1880023_device::pwm_tick)
{
	// only the rising edge at the start of each period is emulated
	m_pwm_out_cb(1);
	m_pwm_out_cb(0);

	m_regs[0x37] |= 0x04;
	m_regs[0x14] |= 0x01;
	if (BIT(m_regs[0x36], 2))
		request_interrupt(IRQ_PWM);
}


//**************************************************************************
//  MN18801A
//**************************************************************************

mn18801a_device::mn18801a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mn1880_device(mconfig, MN18801A, tag, owner, clock, true, 0x44, address_map_constructor(FUNC(mn18801a_device::internal_map), this))
	, m_so_out_cb(*this)
	, m_hs_out_cb(*this)
	, m_to_out_cb(*this)
	, m_tcio_in_cb(*this, 0xff)
	, m_timer{nullptr, nullptr, nullptr}
	, m_timer_reload{0, 0, 0}
	, m_timer_enable(0)
	, m_timer_mask(0)
	, m_timer_status(0)
	, m_so(0)
	, m_hs(0)
	, m_to(0xff)
{
}

void mn18801a_device::internal_map(address_map &map)
{
	map(0x005a, 0x005a).lrw8(NAME([this] () -> u8 { return m_timer_mask; }), NAME([this] (u8 data) { m_timer_mask = data; }));
	map(0x005c, 0x005c).lr8(NAME([this] () -> u8 { return m_timer_enable; })).w(FUNC(mn18801a_device::timer_enable_w));
	map(0x005d, 0x005d).noprw(); // peripheral mode control?
	map(0x005e, 0x005e).nopr().w(FUNC(mn18801a_device::timer_restart_w));
	map(0x0060, 0x0065).w(FUNC(mn18801a_device::timer_reload_w));
	map(0x3fe3, 0x3fe3).lr8(NAME([this] () -> u8 { return m_so; })).w(FUNC(mn18801a_device::so_w));
	map(0x3fe7, 0x3fe7).lr8(NAME([this] () -> u8 { return m_to; })).w(FUNC(mn18801a_device::to_w));
	map(0x3fe9, 0x3fe9).lr8(NAME([this] () -> u8 { return m_tcio_in_cb(); }));
	map(0x3fee, 0x3fee).lr8(NAME([this] () -> u8 { return m_timer_status; })).w(FUNC(mn18801a_device::timer_status_w));
	map(0x3ff3, 0x3ff3).lr8(NAME([this] () -> u8 { return m_hs; })).w(FUNC(mn18801a_device::hs_w));
	mn1880_device::internal_data_map(map);
}

void mn18801a_device::device_start()
{
	mn1880_device::device_start();

	for (auto &timer : m_timer)
		timer = timer_alloc(FUNC(mn18801a_device::timer_tick), this);

	save_item(NAME(m_timer_reload));
	save_item(NAME(m_timer_enable));
	save_item(NAME(m_timer_mask));
	save_item(NAME(m_timer_status));
	save_item(NAME(m_so));
	save_item(NAME(m_hs));
	save_item(NAME(m_to));
}

void mn18801a_device::device_reset()
{
	mn1880_device::device_reset();

	std::fill(std::begin(m_timer_reload), std::end(m_timer_reload), 0);
	m_timer_enable = 0;
	m_timer_mask = 0;
	m_timer_status = 0;
	for (auto &timer : m_timer)
		timer->adjust(attotime::never);

	// port pin levels at reset are unknown, so no outputs are driven here
	m_so = 0;
	m_hs = 0;
	m_to = 0xff;
}

void mn18801a_device::timer_adjust(int timer)
{
	if (BIT(m_timer_enable, timer))
	{
		const attotime period = attotime::from_ticks(0x10000 - m_timer_reload[timer], clock() / 16);
		m_timer[timer]->adjust(period, timer, period);
	}
	else
	{
		m_timer[timer]->adjust(attotime::never);
	}
}

void mn18801a_device::timer_reload_w(offs_t offset, u8 data)
{
	const int timer = offset >> 1;
	if (BIT(offset, 0))
	{
		m_timer_reload[timer] = (m_timer_reload[timer] & 0x00ff) | (u16(data) << 8);
		timer_adjust(timer);
	}
	else
	{
		m_timer_reload[timer] = (m_timer_reload[timer] & 0xff00) | data;
	}
}

void mn18801a_device::timer_enable_w(u8 data)
{
	const u8 changed = m_timer_enable ^ data;
	m_timer_enable = data;
	for (int timer = 0; timer < 3; ++timer)
	{
		if (BIT(changed, timer))
			timer_adjust(timer);
	}
}

void mn18801a_device::timer_restart_w(u8 data)
{
	// TODO: other modes, live counter reads and exact reload timing
	for (int timer = 0; timer < 3; ++timer)
	{
		if (BIT(data, timer))
			timer_adjust(timer);
	}
}

void mn18801a_device::timer_status_w(u8 data)
{
	m_timer_status &= data;
	if (!(m_timer_status & m_timer_mask & 7))
		clear_interrupt_requests(1U << IRQ_TIMER);
}

TIMER_CALLBACK_MEMBER(mn18801a_device::timer_tick)
{
	m_timer_status |= 1 << param;
	if (BIT(m_timer_mask, param))
		request_interrupt(IRQ_TIMER);
}

void mn18801a_device::so_w(u8 data)
{
	m_so = data;
	m_so_out_cb(data);
}

void mn18801a_device::hs_w(u8 data)
{
	m_hs = data;
	m_hs_out_cb(data);
}

void mn18801a_device::to_w(u8 data)
{
	m_to = data;
	m_to_out_cb(data);
}
