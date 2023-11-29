// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Tensilica Xtensa

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "xtensa.h"
#include "xtensad.h"

#include "xtensa_tables.h"

#define LOG_UNHANDLED_OPS       (1U << 1)
#define LOG_HANDLED_OPS         (1U << 2)
#define LOG_UNHANDLED_CACHE_OPS (1U << 3)
#define LOG_UNHANDLED_SYNC_OPS  (1U << 4)
#define LOG_EXTREG_OPS          (1U << 8)


#define VERBOSE (LOG_UNHANDLED_OPS | LOG_EXTREG_OPS)
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(XTENSA, xtensa_device, "xtensa", "Tensilica Xtensa core")

xtensa_device::xtensa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, XTENSA, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_extregs_config("extregs", ENDIANNESS_LITTLE, 32, 8, -2, address_map_constructor(FUNC(xtensa_device::ext_regs), this))
	, m_pc(0)
{
}

std::unique_ptr<util::disasm_interface> xtensa_device::create_disassembler()
{
	return std::make_unique<xtensa_disassembler>();
}

device_memory_interface::space_config_vector xtensa_device::memory_space_config() const
{
	space_config_vector spaces = {
		std::make_pair(AS_PROGRAM, &m_space_config)
	};

	spaces.push_back(std::make_pair(AS_EXTREGS, &m_extregs_config));
	return spaces;
}

uint32_t xtensa_device::extreg_windowbase_r() { return m_extreg_windowbase; }
void xtensa_device::extreg_windowbase_w(u32 data) {	m_extreg_windowbase = data; logerror("m_extreg_windowbase set to %08x\n", data); }
uint32_t xtensa_device::extreg_windowstart_r() { return m_extreg_windowstart; }
void xtensa_device::extreg_windowstart_w(u32 data) { m_extreg_windowstart = data; logerror("m_extreg_windowstart set to %08x\n", data); }
uint32_t xtensa_device::extreg_lbeg_r() { return m_extreg_lbeg; }
void xtensa_device::extreg_lbeg_w(u32 data) { m_extreg_lbeg = data; logerror("m_extreg_lbeg set to %08x\n", data); }
uint32_t xtensa_device::extreg_lend_r() { return m_extreg_lend; }
void xtensa_device::extreg_lend_w(u32 data) { m_extreg_lend = data; logerror("m_extreg_lend set to %08x\n", data); }
uint32_t xtensa_device::extreg_lcount_r() { return m_extreg_lcount; }
void xtensa_device::extreg_lcount_w(u32 data) { m_extreg_lcount = data; logerror("m_extreg_lcount set to %08x\n", data); }


void xtensa_device::ext_regs(address_map &map)
{
	// Loop Option (0-2),
	map(0x00, 0x00).rw(FUNC(xtensa_device::extreg_lbeg_r), FUNC(xtensa_device::extreg_lbeg_w)); // "lbeg" LOOP BEGIN
	map(0x01, 0x01).rw(FUNC(xtensa_device::extreg_lend_r), FUNC(xtensa_device::extreg_lend_w)); // "lend" LOOP END
	map(0x02, 0x02).rw(FUNC(xtensa_device::extreg_lcount_r), FUNC(xtensa_device::extreg_lcount_w)); // "lcount" LOOP COUNT

	// Core Architecture (3)
	//map(0x03, 0x03) // "sar", 

	// Boolean Option (4)
	//map(0x04, 0x04) // "br", 

	// Extended L32R Option (5)
	//map(0x05, 0x05) // "litbase", 

	// Conditional Store Option (12)
	//map(0x0c, 0x0c) // "scompare1", 

	// MAC16 Option (16-17)
	//map(0x10, 0x10) // "acclo",  
	//map(0x11, 0x11) // "acchi",

	// MAC16 Option (32-35)
	//map(0x20, 0x20) // "m0", 
	//map(0x21, 0x21) // "m1",
	//map(0x22, 0x22) // "m2",
	//map(0x23, 0x23) // "m3", 

	 // Windowed Register Option (72-73)
	map(0x48, 0x48).rw(FUNC(xtensa_device::extreg_windowbase_r), FUNC(xtensa_device::extreg_windowbase_w)); // "WindowBase",
	map(0x49, 0x49).rw(FUNC(xtensa_device::extreg_windowstart_r), FUNC(xtensa_device::extreg_windowstart_w));// "WindowStart", 

	// MMU Option (83)
	//map(0x53, 0x53) // "ptevaddr", 

	// Trace Port Option (89)
	//map(0x59, 0x59) // "mmid", 

	// MMU Option (90-92)
	//map(0x5a, 0x5a) // "rasid", 
	//map(0x5b, 0x5b) // "itlbcfg",
	//map(0x5c, 0x5c) // "dtlbcfg", 

	// Debug Option (96)
	//map(0x60, 0x60) // "ibreakenable", 

	// XEA1 Only (98)
	//map(0x62, 0x62) // "cacheattr", 

	// Conditional Store Option (99)
	//map(0x63, 0x63) // "atomctl", 

	// Debug Option (104)
	//map(0x68, 0x68) // "ddr", 

	// Memory ECC/Parity Option (106-111)
	//map(0x6a, 0x6a) // "mepc",
	//map(0x6b, 0x6b) // "meps",
	//map(0x6c, 0x6c) // "mesave",
	//map(0x6d, 0x6d) // "mesr",
	//map(0x6e, 0x6e) // "mecr",
	//map(0x6f, 0x6f) // "mevaddr", 

	// Debug Option (128-129)
	//map(0x80, 0x80) // "ibreaka0", 
	//map(0x81, 0x81) // "ibreaka1", 

	// Debug Option (144-145)
	//map(0x90, 0x90) // "dbreaka0", 
	//map(0x91, 0x91) // "dbreaka1",

	// Debug Option (160-161)
	//map(0xa0, 0xa0) // "dbreakc0",
	//map(0xa1, 0xa1) // "dbreakc1", 

	// Exception Option (177)
	//map(0xb1, 0xb1) // "epc1", 

	// High-Priority Interrupt Option (178-183)
	//map(0xb2, 0xb2) // "epc2",
	//map(0xb3, 0xb3) // "epc3",
	//map(0xb4, 0xb4) // "epc4",
	//map(0xb5, 0xb5) // "epc5",
	//map(0xb6, 0xb6) // "epc6",
	//map(0xb7, 0xb7) // "epc7", 

	// Exception Option (192)
	//map(0xc0, 0xc0) // "depc", 

	// High-Priority Interrupt Option (194-199)
	//map(0xc2, 0xc2) // "eps2", 
	//map(0xc3, 0xc3) // "eps3",
	//map(0xc4, 0xc4) // "eps4",
	//map(0xc5, 0xc5) // "eps5",
	//map(0xc6, 0xc6) // "eps6",
	//map(0xc7, 0xc7) // "eps7", 

	// Exception Option (209)
	//map(0xd1, 0xd1) // "excsave1", 

	// High-Priority Interrupt Option (210-215)
	//map(0xd2, 0xd2) // "excsave2", 
	//map(0xd3, 0xd3) // "excsave3",
	//map(0xd4, 0xd4) // "excsave4",
	//map(0xd5, 0xd5) // "excsave5",
	//map(0xd6, 0xd6) // "excsave6",
	//map(0xd7, 0xd7) // "excsave7", 

	// Coprocessor Option (224)
	//map(0xe0, 0xe0) // "cpenable", 

	// Interrupt Option (226-228)
	//map(0xe2, 0xe2) // "intset", 
	//map(0xe3, 0xe3) // "intclr",
	//map(0xe4, 0xe4) // "intenable", 

	// various options (230)
	//map(0xe6, 0xe6) // "ps", 

	// Relocatable Vector Option (231)
	//map(0xe7, 0xe7) // "vecbase", 

	// Exception Option (232)
	//map(0xe8, 0xe8) // "exccause", 

	// Debug Option (233)
	//map(0xe9, 0xe9) // "debugcause", 

	// Timer Interrupt Option (234)
	//map(0xea, 0xea) // "ccount", 

	// Processor ID Option (235)
	//map(0xeb, 0xeb) // "prid", 

	// Debug Option (236-237)
	//map(0xec, 0xec) // "icount", 
	//map(0xed, 0xed) // "icountlevel",

	// Exception Option (238)
	//map(0xee, 0xee) // "excvaddr", 

	// Timer Interrupt Option (240-242)
	//map(0xf0, 0xf0) // "ccompare0", 
	//map(0xf1, 0xf1) // "ccompare1",
	//map(0xf2, 0xf2) // "ccompare2", 

	// Miscellaneous Special Registers Option (244-247)
	//map(0xf4, 0xf4) // "misc0", 
	//map(0xf5, 0xf5) // "misc1",
	//map(0xf6, 0xf6) // "misc2",
	//map(0xf7, 0xf7) // "misc3", 

}


void xtensa_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_space);

	std::fill(std::begin(m_a), std::end(m_a), 0);

	m_num_physical_regs = 512; // just set this higher than it should be for now, until we emulate window exceptions (marimba startup check suggests 32, with it wrapping around when exceptions are disabled)
	m_a.resize(m_num_physical_regs);

	set_icountptr(m_icount);

	state_add(XTENSA_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc);
	state_add(STATE_GENPCBASE, "CURPC", m_pc);
	state_add(XTENSA_WINDOW, "WinBase", m_extreg_windowbase);

	state_add(XTENSA_LOOPBEGIN, "LoopBegin", m_extreg_lbeg);
	state_add(XTENSA_LOOPEND, "LoopEnd", m_extreg_lend);
	state_add(XTENSA_LOOPCOUNT, "LoopCount", m_extreg_lcount);

	for (int i = 0; i < m_num_physical_regs; i++)
		state_add(XTENSA_A0 + i, string_format("a%d", i).c_str(), m_a[i]);

	state_add(XTENSA_PC, "PC", m_pc);

	save_item(NAME(m_pc));
	save_item(NAME(m_a));
}

void xtensa_device::device_reset()
{
	// TODO: Reset state

	m_extreg_windowbase = 0;
	m_extreg_windowstart = 0;

	m_extreg_sar = 0;

	m_extreg_lbeg = 0;
	m_extreg_lend = 0;
	m_extreg_lcount = 0;
}

void xtensa_device::handle_reserved(u32 inst)
{
	LOGMASKED(LOG_UNHANDLED_OPS, "%-8s0x%02X ; reserved\n", "db", inst & 0xff);
	m_nextpc = m_pc + 1;
}

u32 xtensa_device::get_reg(u8 reg)
{
	// TODO: much more complex than this with the Windowed Register Option!
	int realreg = reg + m_extreg_windowbase * 4;

	if (realreg >= 0 && realreg < m_num_physical_regs)
	{
		return m_a[realreg];
	}
	else
	{
		// exceptions?
		return m_a[realreg & (m_num_physical_regs-1)];
	}
}

void xtensa_device::set_reg(u8 reg, u32 value)
{
	// TODO: much more complex than this with the Windowed Register Option!
	int realreg = reg + m_extreg_windowbase * 4;

	if (realreg >= 0 && realreg < m_num_physical_regs)
	{
		m_a[realreg] = value;
	}
	else
	{
		// exceptions?
		m_a[realreg & (m_num_physical_regs-1)] = value;
	}
}

u32 xtensa_device::get_mem32(u32 addr)
{
	if (addr & 3)
		logerror("get_mem32 unaligned\n");

	return m_space.read_dword(addr & ~3);
}

void xtensa_device::set_mem32(u32 addr, u32 data)
{
	if (addr & 3)
		logerror("set_mem32 unaligned\n");

	return m_space.write_dword(addr &~ 3, data);
}


void xtensa_device::getop_and_execute()
{
	m_nextpc = m_pc + 2;
	u32 inst = m_cache.read_byte(m_pc);
	inst |= m_cache.read_byte(m_pc+1)<<8;

	const u8 op0 = BIT(inst, 0, 4);
	if (op0 < 0b1000)
	{
		inst |= u32(m_cache.read_byte(m_pc+2)) << 16;
		m_nextpc = m_pc + 3;
	}

	switch (op0)
	{
	case 0b0000: // QRST
		switch (BIT(inst, 16, 4))
		{
		case 0b0000: // RST0
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // ST0
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // SNM0
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // ILL
						LOGMASKED(LOG_UNHANDLED_OPS, "ill\n");
						break;

					case 0b1000: // RET
						LOGMASKED(LOG_UNHANDLED_OPS, "ret\n");
						break;

					case 0b1001: // RETW (with Windowed Register Option)
						LOGMASKED(LOG_UNHANDLED_OPS, "retw\n");
						break;

					case 0b1010: // JX
					{
						u8 reg = BIT(inst, 8, 4);
						LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d\n", "jx", reg);
						m_nextpc = get_reg(reg);
						break;
					}

					case 0b1100: // CALLX0
					case 0b1101: case 0b1110: case 0b1111: // CALLX4, CALLX8, CALLX12 (with Windowed Register Option)
						LOGMASKED(LOG_UNHANDLED_OPS, "callx%-3da%d\n", BIT(inst, 4, 2) * 4, BIT(inst, 8, 4));
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0001: // MOVSP (with Windowed Register Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d", "movsp\n", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0010: // SYNC
					switch (BIT(inst, 4, 8))
					{
					case 0b00000000: // ISYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "isync\n");
						break;

					case 0b00000001: // RSYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "rsync\n");
						break;

					case 0b00000010: // ESYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "esync\n");
						break;

					case 0b00000011: // DSYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "dsync\n");
						break;

					case 0b00001000: // EXCW (with Exception Option)
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "excw\n");
						break;

					case 0b00001100: // MEMW
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "memw\n");
						break;

					case 0b00001101: // EXTW (added in RA-2004.1)
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "extw\n");
						break;

					case 0b00001111: // NOP (added in RA-2004.1; was assembly macro previously)
						LOGMASKED(LOG_HANDLED_OPS, "nop\n");
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0011: // RFEI
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // RFET
						switch (BIT(inst, 8, 4))
						{
						case 0b0000: // RFE (with Exception Option)
							LOGMASKED(LOG_UNHANDLED_OPS, "rfe\n");
							break;

						case 0b0001: // RFUE (with Exception Option; XEA1 only)
							LOGMASKED(LOG_UNHANDLED_OPS, "rfue\n");
							break;

						case 0b0010: // RFDE (with Exception Option)
							LOGMASKED(LOG_UNHANDLED_OPS, "rfde\n");
							break;

						case 0b0100: // RFWO (with Windowed Register option)
							LOGMASKED(LOG_UNHANDLED_OPS, "rfwo\n");
							break;

						case 0b0101: // RFWU (with Windowed Register option)
							LOGMASKED(LOG_UNHANDLED_OPS, "rfwu\n");
							break;

						default:
							handle_reserved(inst);
							break;
						}
						break;

					case 0b0001: // RFI (with High-Priority Interrupt Option)
						LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d\n", "rfi", BIT(inst, 8, 4));
						break;

					case 0b0010: // RFME (with Memory ECC/Parity Option)
						LOGMASKED(LOG_UNHANDLED_OPS, "rfme\n");
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0100: // BREAK (with Debug Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d, %d\n", "break", BIT(inst, 8, 4), BIT(inst, 4, 4));
					break;

				case 0b0101: // SYSCALL (with Exception Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "syscall\n");
					break;

				case 0b0110: // RSIL (with Interrupt Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, %d\n", "rsil", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0111: // WAITI (with Interrupt Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d\n", "waiti", BIT(inst, 8, 4));
					break;

				case 0b1000: // ANY4 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "any4", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1001: // ALL4 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "all4", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1010: // ANY8 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "any8", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1011: // ALL8 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "all8", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0001: // AND
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", "and", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0010: // OR
				if (BIT(inst, 8, 4) == BIT(inst, 4, 4))
				{
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "mov", BIT(inst, 12, 4), BIT(inst, 8, 4));
				}
				else
				{
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", "or", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				}
				break;

			case 0b0011: // XOR
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", "xor", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0100: // ST1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: case 0b0001: case 0b0010: case 0b0011: // SSR, SSL, SSA8L, SSA8B
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d\n", s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4));
					break;

				case 0b0100: // SSAI
				{
					u8 imm = BIT(inst, 8, 4) + (inst & 0x000010);
					LOGMASKED(LOG_HANDLED_OPS, "%-8s%d\n", "ssai", imm);
					m_extreg_sar = imm;
					break;
				}

				case 0b0110: case 0b0111: // RER, WER
				case 0b1110: case 0b1111: // NSA, NSAU (with Miscellaneous Operations Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // ROTW (with Windowed Register Option)
				{
					s8 imm = util::sext(inst >> 4, 4);
					LOGMASKED(LOG_HANDLED_OPS, "%-8s%d\n", "rotw", imm);
					m_extreg_windowbase += imm;
					break;
				}

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0101: // TLB (with Region Translation Option or MMU Option)
				switch (BIT(inst, 12, 4))
				{
				case 0b0011: case 0b0101: case 0b0110: case 0b0111: // RITLB0, PITLB, WITLB, RITLB1
				case 0b1011: case 0b1101: case 0b1110: case 0b1111: // RDTLB0, PDTLB, WDTLB, RDTLB1
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", s_tlb_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0100: case 0b1100: // IITLB, IDTLB
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d\n", s_tlb_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0110: // RT0
				switch (BIT(inst, 8, 4))
				{
				case 0b0000: // NEG
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "neg", BIT(inst, 12, 4), BIT(inst, 4, 4));
					break;

				case 0b0001: // ABS
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "abs", BIT(inst, 12, 4), BIT(inst, 4, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b1000: case 0b1100: // ADD, SUB
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", BIT(inst, 22) ? "sub" : "add", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1001: case 0b1010: case 0b1011: // ADDX2, ADDX4, ADDX8
			case 0b1101: case 0b1110: case 0b1111: // SUBX2, SUBX4, SUBX8
				LOGMASKED(LOG_UNHANDLED_OPS, "%sx%-4da%d, a%d, a%d\n", BIT(inst, 22) ? "sub" : "add", 1 << BIT(inst, 20, 2), BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0001: // RST1
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: // SLLI (shift count is 0..31)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %d\n", "slli", BIT(inst, 12, 4), BIT(inst, 4, 4), BIT(inst, 8, 4) + (BIT(inst, 20) ? 16 : 0));
				break;

			case 0b0010: case 0b0011: // SRAI (shift count is 0..31)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %d\n", "srai", BIT(inst, 12, 4), BIT(inst, 4, 4), BIT(inst, 8, 4) + (BIT(inst, 20) ? 16 : 0));
				break;

			case 0b0100: // SRLI (shift count is 0..15)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %d\n", "srli", BIT(inst, 12, 4), BIT(inst, 4, 4), BIT(inst, 8, 4));
				break;

			case 0b0110: // XSR (added in T1040)
				LOGMASKED(LOG_UNHANDLED_OPS, "xsr.%-3s a%d\n", special_reg(BIT(inst, 8, 8), true), BIT(inst, 4, 4));
				break;

			case 0b0111: // ACCER (added in RC-2009.0)
				switch (BIT(inst, 20, 4))
				{
				case 0b0000: // RER
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "rer", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // WER
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "wer", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b1000: // SRC
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", "src", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1001: // SRL
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "srl", BIT(inst, 12, 4), BIT(inst, 4, 4));
				break;

			case 0b1010: // SLL
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sll", BIT(inst, 12, 4), BIT(inst, 8, 4));
				break;

			case 0b1011: // SRA
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sra", BIT(inst, 12, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: // MUL16U (with 16-bit Integer Multiply Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", "mul16u", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1101: // MUL16S (with 16-bit Integer Multiply Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", "mul16s", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1111: // IMP (Implementation-Specific)
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // LICT (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "lict", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0001: // SICT (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sict", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0010: // LICW (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "licw", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0011: // SICW (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sicw", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // LDCT (with Data Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "ldct", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1001: // SDCT (with Data Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sdct", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1110: // RFDX (with On-Chip Debug)
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // RFDO
						LOGMASKED(LOG_UNHANDLED_OPS, "rfdo\n");
						break;

					case 0b0001: // RFDD
						LOGMASKED(LOG_UNHANDLED_OPS, "rfdd\n");
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0010: // RST2
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011: case 0b0100: // ANDB, ANDBC, ORB, ORBC, XORB (with Boolean Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d, b%d\n", s_rst2_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1010: case 0b1011: // MULL, MULUH, MULSH (with 32-bit Integer Multiply Option)
			case 0b1100: case 0b1101: case 0b1110: case 0b1111: // QUOU, QUOS, REMU, REMS (with 32-bit Integer Divide Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", s_rst2_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0011: // RST3
			switch (BIT(inst, 20, 4))
			{
			case 0b0000:// RSR
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.%-3d a%d\n", "rsr", special_reg(BIT(inst, 8, 8), BIT(inst, 20)), BIT(inst, 4, 4));
				break;

			case 0b0001:// WSR
			{
				u8 spcreg = BIT(inst, 8, 8);
				u8 reg = BIT(inst, 4, 4);
				LOGMASKED(LOG_EXTREG_OPS, "%s.%-3d a%d\n", "wsr", special_reg(spcreg, BIT(inst, 20)), reg);
				space(AS_EXTREGS).write_dword(spcreg, get_reg(reg));
				break;
			}

			case 0b0010: case 0b0011: // SEXT, CLAMPS (with Miscellaneous Operations Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %d\n", s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4) + 7);
				break;

			case 0b0100: case 0b0101: case 0b0110: case 0b0111: // MIN, MAX, MINU, MAXU (with Miscellaneous Operations Option)
			case 0b1000: case 0b1001: case 0b1010: case 0b1011: // MOVEQZ, MOVNEZ, MOVLTZ, MOVGEZ
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // MOVF, MOVT (with Boolean Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, b%d\n", s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1110: case 0b1111: // RUR, WUR (TODO: TIE user_register names)
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.u%-2d a%d\n", s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 4, 8), BIT(inst, 12, 4));
				break;
			}
			break;

		case 0b0100: case 0b0101: // EXTUI
		{
			u8 dstreg = BIT(inst, 12, 4);
			u8 srcreg = BIT(inst, 4, 4);
			u8 shift = BIT(inst, 8, 4) + (BIT(inst, 16) ? 16 : 0);
			u8 numbits = BIT(inst, 20, 4) + 1;
			LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, a%d, %d, %d\n", "extui", dstreg, srcreg, shift, numbits);
			set_reg(dstreg, (get_reg(srcreg) >> shift) & ((1 << numbits)-1));
			break;
		}

		case 0b0110: case 0b0111: // CUST0, CUST1
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8s0x%02X ; cust%d?\n", "db", inst & 0xff, BIT(inst, 16));
			m_nextpc = m_pc + 1;
			break;

		case 0b1000: // LSCX (with Floating-Point Coprocessor Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // LSX
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "lsx", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0001: // LSXU
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "lsxu", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0100: // SSX
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "ssx", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0101: // SSXU
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "ssxu", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1001: // LSC4 (with Windowed Register Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // L32E
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", "l32e", BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm(int(BIT(inst, 12, 4)) * 4 - 64));
				break;

			case 0b0100: // S32E
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", "s32e", BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm(int(BIT(inst, 12, 4)) * 4 - 64));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1010: // FP0 (with Floating-Point Coprocessor Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0100: case 0b0101: // ADD.S, SUB.S, MUL.S, MADD.S, MSUB.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d, f%d\n", s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1001: case 0b1010: case 0b1011: case 0b1110: // ROUND.S, TRUNC.S, FLOOR.S, CEIL.S, UTRUNC.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-7s a%d, f%d, %d\n", s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // FLOAT.S, UFLOAT.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-7s f%d, a%d, %d\n", s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1111: // FP1OP
				switch (BIT(inst, 4, 4))
				{
				case 0b0000: // MOV.S
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d\n", "mov.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0001: // ABS.S
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d\n", "abs.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0100: // RFR
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, f%d\n", "rfr", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0101: // WFR
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d\n", "wfr", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0110: // NEG.S
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d\n", "neg.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1011: // FP1 (with Floating-Point Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0001: case 0b0010: case 0b0011: case 0b0100: case 0b0101: case 0b0110: case 0b0111: // UN.S, OEQ.S, UEQ.S, OLT.S, ULT.S, OLE.S, ULE.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, f%d, f%d\n", s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1001: case 0b1010: case 0b1011: // MOVEQZ.S, MOVNEZ.S, MOVLTZ.S, MOVGEZ.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d, a%d\n", s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // MOVF.S, MOVT.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d, b%d\n", s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0001: // L32R (virtual address is always aligned)
	{
		u8 reg = BIT(inst, 4, 4);
		u32 addr = (m_pc + 3 - 0x40000 + (inst >> 8) * 4) & 0xfffffffc;
		LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, 0x%08X\n", "l32r", reg, addr);
		set_reg(reg, get_mem32(addr));
		break;
	}

	case 0b0010: // LSAI
		switch (BIT(inst, 12, 4))
		{
		case 0b0000: case 0b0100: // L8UI, S8I
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm(inst >> 16));
			break;

		case 0b0001: case 0b0101: case 0b1001: // L16UI, S16I, L16SI
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm((inst >> 16) * 2));
			break;

		case 0b0010: // L32I
		{
			u8 dstreg = BIT(inst, 4, 4);
			u8 basereg = BIT(inst, 8, 4);
			u32 imm = (inst >> 16) * 4;
			LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, a%d, %s\n", "l32i", BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm((inst >> 16) * 4));
			set_reg(dstreg, get_mem32(get_reg(basereg) + imm));
			break;
		}

		case 0b0110: // S32I
		{
			u8 srcreg = BIT(inst, 4, 4);
			u8 basereg = BIT(inst, 8, 4);
			u32 imm = (inst >> 16) * 4;
			LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, a%d, %s\n", "s32i", srcreg, basereg, format_imm(imm));
			set_mem32(get_reg(basereg) + imm, get_reg(srcreg));
			break;
		}

		case 0b1011: // L32AI
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm((inst >> 16) * 4));
			break;

		case 0b1111: // S32RI (with Multiprocessor Synchronization Option)
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm((inst >> 16) * 4));
			break;

		case 0b1110: // S32C1I (with Conditional Store Option)
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm((inst >> 16) * 4));
			break;

		case 0b0111: // CACHE
			switch (BIT(inst, 4, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011: // DPFR, DPFW, DPFRO, DPFWO (with Data Cache Option)
			case 0b0100: case 0b0101: case 0b0110: case 0b0111: // DHWB, DHWBI, DHI, DII (with Data Cache Option)
			case 0b1100: case 0b1110: case 0b1111: // IPF, IHI, III (with Instruction Cache Option)
				LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", s_cache_ops[BIT(inst, 4, 4)], BIT(inst, 8, 4), format_imm((inst >> 16) * 4));
				break;

			case 0b1000: // DCE (with Data Cache Option)
				switch (BIT(inst, 16, 4))
				{
				case 0b0000: // DPFL (with Data Cache Index Lock Option)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "dpfl", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0010: // DHU (with Data Cache Index Lock Option)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "dhu", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0011: // DIU (with Data Cache Index Lock Option)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "diu", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0100: // DIWB (added in T1050)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "diwb", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0101: // DIWBI (added in T1050)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "diwbi", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;
				}
				break;

			case 0b1101: // ICE (with Instruction Cache Index Lock Option)
				switch (BIT(inst, 16, 4))
				{
				case 0b0000: // IPFL
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "ipfl", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0010: // IHU
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "ihu", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0011: // IIU
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "iiu", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1010: // MOVI
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, %s\n", "movi", BIT(inst, 4, 4), format_imm(util::sext((inst & 0x000f00) + (inst >> 16), 12)));
			break;

		case 0b1100: // ADDI
		{
			u8 dstreg = BIT(inst, 8, 4);
			u8 srcreg = BIT(inst, 4, 4);
			s32 imm = s8(u8(inst >> 16)) * (BIT(inst, 12) ? 256 : 1);
			LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], dstreg, srcreg, format_imm(imm));
			set_reg(dstreg, get_reg(srcreg)+imm);
			break;
		}

		case 0b1101: // ADDMI
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4), BIT(inst, 4, 4), format_imm(s8(u8(inst >> 16)) * (BIT(inst, 12) ? 256 : 1)));
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0011: // LSCI (with Floating-Point Coprocessor Option)
		if (BIT(inst, 12, 2) == 0)
		{
			// LSI, SSI, LSIU, SSIU
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, %s\n", s_lsci_ops[BIT(inst, 14, 2)], BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm(BIT(inst, 16, 8) * 4));
			break;
		}
		else
		{
			handle_reserved(inst);
			break;
		}

	case 0b0100: // MAC16 (with MAC16 Option)
		switch (BIT(inst, 20, 4))
		{
		case 0b0000: case 0b0001: // MACID, MACCD
			if (BIT(inst, 18, 2) == 0b10)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.dd.%s.%s m%d, a%d, m%d, m%d\n", s_mac16_ops[BIT(inst, 18, 2)],
											s_mac16_half[BIT(inst, 16, 2)],
											BIT(inst, 20) ? "lddec" : "ldinc",
											BIT(inst, 12, 2), BIT(inst, 8, 4),
											BIT(inst, 14), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0100: case 0b0101: // MACIA, MACCA
			if (BIT(inst, 18, 2) == 0b10)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.da.%s.%s m%d, a%d, m%d, a%d\n", s_mac16_ops[BIT(inst, 18, 2)],
											s_mac16_half[BIT(inst, 16, 2)],
											BIT(inst, 20) ? "lddec" : "ldinc",
											BIT(inst, 12, 2), BIT(inst, 8, 4),
											BIT(inst, 14), BIT(inst, 4, 4));
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0010: // MACDD
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.dd.%s m%d, m%d\n", s_mac16_ops[BIT(inst, 18, 2)], s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 14), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0011: // MACAD
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.ad.%s a%d, m%d\n", s_mac16_ops[BIT(inst, 18, 2)], s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 8, 4), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0110: // MACDA
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.da.%s m%d, a%d\n", s_mac16_ops[BIT(inst, 18, 2)], s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 14), BIT(inst, 4, 4));
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0111: // MACAA
			LOGMASKED(LOG_UNHANDLED_OPS, "%s.aa.%s a%d, a%d\n", s_mac16_ops[BIT(inst, 18, 2)], s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 8, 4), BIT(inst, 4, 4));
			break;

		case 0b1000: case 0b1001: // MACI, MACC
			switch (BIT(inst, 16, 4))
			{
			case 0b0000: // LDINC, LDDEC
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sm%d, a%d\n", BIT(inst, 20) ? "lddec" : "ldinc", BIT(inst, 12, 2), BIT(inst, 8, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0101: // CALLN (target address is always aligned)
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // CALL0
		case 0b01: case 0b10: case 0b11: // CALL4, CALL8, CALL12 (with Windowed Register Option)
			LOGMASKED(LOG_UNHANDLED_OPS, "call%-4d0x%08X\n", BIT(inst, 4, 2) * 4, (m_pc & 0xfffffffc) + 4 + util::sext(inst >> 6, 18) * 4);
			break;
		}
		break;

	case 0b0110: // SI
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // J
		{
			u32 newpc = m_pc + 4 + util::sext(inst >> 6, 18);
			LOGMASKED(LOG_HANDLED_OPS, "%-8s0x%08X\n", "j", newpc);
			m_nextpc = newpc;
			break;
		}

		case 0b01: // BZ
		{
			u8 reg = BIT(inst, 8, 4);
			u32 addr = m_pc + 4 + util::sext(inst >> 12, 12);

			switch (BIT(inst, 6, 2))
			{
			case 0b00:// beqz
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, 0x%08X\n", "beqz", reg, addr);
				break;
			case 0b01:// bnez
			{
				LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, 0x%08X\n", "bnez", reg, addr);
				if (get_reg(reg) !=0)
					m_nextpc = addr;
				break;
			}
			case 0b10:// bltz
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, 0x%08X\n", "bltz", reg, addr);
				break;
			case 0b11:// bgez
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, 0x%08X\n", "bgez", reg, addr);
				break;
			}
			break;
		}

		case 0b10: // BI0
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, %s, 0x%08X\n", s_bi0_ops[BIT(inst, 6, 2)], BIT(inst, 8, 4), format_imm(s_b4const[BIT(inst, 12, 4)]),  m_pc + 4 + s8(u8(inst >> 16)));
			break;

		case 0b11: // BI1
			switch (BIT(inst, 6, 2))
			{
			case 0b00: // ENTRY
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, %s\n", "entry", BIT(inst, 8, 4), format_imm((inst >> 12) * 4));
				break;

			case 0b01: // B1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: case 0b0001: // BF, BT (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, 0x%08X\n", BIT(inst, 12) ? "bt" : "bf", BIT(inst, 8, 4), m_pc + 4 + s8(u8(inst >> 16)));
					break;

				case 0b1000: // LOOP (with Loop Option)
				{
					u8 reg = BIT(inst, 8, 4);
					u32 addr = m_pc + 4 + s8(u8(inst >> 16));
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, 0x%08X\n", "loop",  reg, addr);
					m_extreg_lcount = get_reg(reg)-1;
					m_extreg_lbeg = m_nextpc;
					m_extreg_lend = addr;
					break;
				}

				case 0b1001: // LOOPNEZ (with Loop Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, 0x%08X\n", "loopnez", BIT(inst, 8, 4), m_pc + 4 + s8(u8(inst >> 16)));
					break;

				case 0b1010: // LOOPGTZ (with Loop Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, 0x%08X\n", "loopgtz", BIT(inst, 8, 4), m_pc + 4 + s8(u8(inst >> 16)));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b10: case 0b11: // BLTUI, BGEUI
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, %s, 0x%08X\n", BIT(inst, 6) ? "bgeui" : "bltui", BIT(inst, 8, 4), format_imm(s_b4constu[BIT(inst, 4, 4)]), m_pc + 4 + s8(u8(inst >> 16)));
				break;
			}
			break;
		}
		break;

	case 0b0111: // B
		if (BIT(inst, 13, 2) == 0b11)
		{
			// BBCI, BBSI
			u8 reg = BIT(inst, 8, 4);
			u8 imm =  BIT(inst, 4, 4) + (BIT(inst, 12) ? 16 : 0);
			u32 addr = m_pc + 4 + s8(u8(inst >> 16));
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, %d, 0x%08X\n", BIT(inst, 15) ? "bbsi" : "bbci", reg, imm, addr);
			break;
		}
		else
		{
			// BNONE, BEQ, BLT, BLTU, BALL, BBC, BBCI, BANY, BNE, BGE, BGEU, BNALL, BBS
			u8 as = BIT(inst, 8, 4);
			u8 at = BIT(inst, 4, 4);
			u32 addr = m_pc + 4 + s8(u8(inst >> 16));

			switch (BIT(inst, 12, 4))
			{
			case 0b0000:// bnone
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bnone", as, at, addr);
				break;
			case 0b0001:// beq
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "beq", as, at, addr);
				break;
			case 0b0010:// blt
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "blt", as, at, addr);
				break;
			case 0b0011:// bltu
				LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bltu", as, at, addr);
				if (get_reg(as) < get_reg(at))
					m_nextpc = addr;
				break;
			case 0b0100:// ball
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "ball", as, at, addr);
				break;
			case 0b0101:// bbc
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbc", as, at, addr);
				break;
			//case 0b0110:// bbci
			//	LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbci", as, at, addr);
			//	break;
			//case 0b0111:// bbci
			//	LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbci", as, at, addr);
			//	break;
			case 0b1000:// bany
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bany", as, at, addr);
				break;
			case 0b1001:// bne
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bne", as, at, addr);
				break;
			case 0b1010:// bge
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bge", as, at, addr);
				break;
			case 0b1011:// bgeu,
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bgeu", as, at, addr);
				break;
			case 0b1100:// bnall
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bnall", as, at, addr);
				break;
			case 0b1101:// bbs
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbs", as, at, addr);
				break;
			//case 0b1110:// bbsi
			//	LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbsi", as, at, addr);
			//	break;
			//case 0b1111:// bbsih
			//	LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbsih", as, at, addr);
			//	break;
			default:
				break;
			}
		}
		break;

	case 0b1000: // L32I.N (with Code Density Option)
	{
		u8 dstreg = BIT(inst, 4, 4);
		u8 basereg = BIT(inst, 8, 4);
		u32 imm = BIT(inst, 12, 4) * 4;
		LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, a%d, %s\n", "l32i.n", dstreg, basereg, format_imm(imm));
		set_reg(dstreg, get_mem32(get_reg(basereg) + imm));
		break;
	}

	case 0b1001: // S32I.N (with Code Density Option)
	{
		u8 srcreg = BIT(inst, 4, 4);
		u8 basereg = BIT(inst, 8, 4);
		u32 imm = BIT(inst, 12, 4) * 4;
		LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, a%d, %s\n", "s32i.n", srcreg, basereg, format_imm(imm));
		set_mem32(get_reg(basereg) + imm, get_reg(srcreg));
		break;
	}

	case 0b1010: // ADD.N (with Code Density Option)
		LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", "add.n", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
		break;

	case 0b1011: // ADDI.N (with Code Density Option)
	{
		u8 dstreg = BIT(inst, 12, 4);
		u8 srcreg = BIT(inst, 8, 4);
		s32 imm = BIT(inst, 4, 4) == 0 ? -1 : int(BIT(inst, 4, 4));
		LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, a%d, %d\n", "addi.n", dstreg, srcreg, imm);
		set_reg(dstreg, get_reg(srcreg)+imm);
		break;
	}

	case 0b1100: // ST2 (with Code Density Option)
		if (!BIT(inst, 7))
		{
			// 7-bit immediate field uses asymmetric sign extension (range is -32..95)
			u8 dstreg = BIT(inst, 8, 4);
			s32 imm = int((inst & 0x0070) + BIT(inst, 12, 4) - (BIT(inst, 5, 2) == 0b11 ? 128 : 0));
			LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, %s\n", "movi.n", dstreg, format_imm(imm));
			set_reg(dstreg,imm);
			break;
		}
		else
		{
			if (BIT(inst, 6))
			{
				// 6-bit immediate field is zero-extended (these forms can branch forward only)
				u8 reg = BIT(inst, 8, 4);
				u8 addr = m_pc + 4 + (inst & 0x0030) + BIT(inst, 12, 4);
				LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, 0x%08X\n", "bnez.n", reg, addr);
				if (get_reg(reg) != 0)
					m_nextpc = addr;
				break;
			}
			else
			{
				// 6-bit immediate field is zero-extended (these forms can branch forward only)
				u8 reg = BIT(inst, 8, 4);
				u8 addr = m_pc + 4 + (inst & 0x0030) + BIT(inst, 12, 4);
				LOGMASKED(LOG_HANDLED_OPS, "%-8sa%d, 0x%08X\n", "beqz.n", reg, addr);
				if (get_reg(reg) == 0)
					m_nextpc = addr;
				break;
			}
			break;
		}

	case 0b1101: // ST3 (with Code Density Option)
		switch (BIT(inst, 12, 4))
		{
		case 0b0000: // MOV.N
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "mov.n", BIT(inst, 4, 4), BIT(inst, 8, 4));
			break;

		case 0b1111: // S3
			switch (BIT(inst, 4, 4))
			{
			case 0b0000: // RET.N
				LOGMASKED(LOG_UNHANDLED_OPS, "ret.n\n");
				break;

			case 0b0001: // RETW.N (with Windowed Register Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "retw.n\n");
				break;

			case 0b0010: // BREAK.N (with Debug Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d\n", "break.n", BIT(inst, 8, 4));
				break;

			case 0b0011: // NOP.N
				LOGMASKED(LOG_HANDLED_OPS, "nop.n\n");
				break;

			case 0b0110: // ILL.N
				LOGMASKED(LOG_UNHANDLED_OPS, "ill.n\n");
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	default:
		handle_reserved(inst);
		break;
	}

	// handle zero overhead loops
	if (m_nextpc == m_extreg_lend)
	{
		if (m_extreg_lcount != 0)
		{
			m_extreg_lcount--;
			m_nextpc = m_extreg_lbeg;
		}
	}

	m_pc = m_nextpc;
}


void xtensa_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);
		getop_and_execute();
		m_icount--;
	}
}

void xtensa_device::execute_set_input(int inputnum, int state)
{
	// TODO
}
