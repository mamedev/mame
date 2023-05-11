// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact Core

 The following processors use the ARCompact instruction set

  - ARCtangent-A5
  - ARC 600
  - ARC 700

 (this is a skeleton core)

 ARCompact is a 32-bit CPU that freely mixes 32-bit and 16-bit instructions

 Various user customizations could be made as with the ARC A4 based processors
 these include custom instructions and registers.

\*********************************/

#include "emu.h"
#include "arcompact_helper.ipp"

#include "arcompactdasm.h"


DEFINE_DEVICE_TYPE(ARCA5, arcompact_device, "arc_a5", "Argonaut ARCtangent A5")

uint32_t arcompact_device::arcompact_auxreg002_LPSTART_r() { return m_LP_START & 0xfffffffe; }
void arcompact_device::arcompact_auxreg002_LPSTART_w(uint32_t data) { m_LP_START = data & 0xfffffffe; }
uint32_t arcompact_device::arcompact_auxreg003_LPEND_r() { return m_LP_END & 0xfffffffe; }
void arcompact_device::arcompact_auxreg003_LPEND_w(uint32_t data) { m_LP_END = data & 0xfffffffe; }

uint32_t arcompact_device::arcompact_auxreg00a_STATUS32_r() { return m_status32; }

uint32_t arcompact_device::arcompact_auxreg00b_STATUS32_L1_r() { return m_status32_l1; }
uint32_t arcompact_device::arcompact_auxreg00c_STATUS32_L2_r() { return m_status32_l2; }
void arcompact_device::arcompact_auxreg00b_STATUS32_L1_w(uint32_t data) { m_status32_l1 = data; }
void arcompact_device::arcompact_auxreg00c_STATUS32_L2_w(uint32_t data) { m_status32_l2 = data; }

void arcompact_device::arcompact_auxreg012_MULHI_w(uint32_t data)
{
	// The regular multiply result registers (r56 - MLO, r57 - MMID and r58 - MHI) are read-only
	// this optional extension allows the program to directly set the value of MHI
	// so that it's state can be restored
	//
	// (although the Leapster BIOS tries to write MMID directly when doing such a restoration
	// despite otherwise going through the recommended procedure of multiplying the lower result
	// by 1 to restore MLO and writing to this AUX address to restore MHI)
	m_regs[REG_MHI] = data;
}

uint32_t arcompact_device::arcompact_auxreg043_AUX_IRQ_LV12_r()
{
	logerror("%s: arcompact_auxreg043_AUX_IRQ_LV12_r\n", machine().describe_context());
	return m_AUX_IRQ_LV12;
}

void arcompact_device::arcompact_auxreg043_AUX_IRQ_LV12_w(uint32_t data)
{
	logerror("%s: arcompact_auxreg043_AUX_IRQ_LV12_w %08x\n", machine().describe_context(), data);
	if (data & 0x00000001) m_AUX_IRQ_LV12 &= ~0x00000001;
	if (data & 0x00000002) m_AUX_IRQ_LV12 &= ~0x00000002;
}


uint32_t arcompact_device::arcompact_auxreg025_INTVECTORBASE_r() { return m_INTVECTORBASE & 0xfffffc00; }

void arcompact_device::arcompact_auxreg025_INTVECTORBASE_w(uint32_t data)
{
	logerror("%s: m_INTVECTORBASE write %08x\n", machine().describe_context(), data);
	m_INTVECTORBASE = data & 0xfffffc00;
}

uint32_t arcompact_device::arcompact_auxreg012_TIMER0_r(offs_t offset)
{
	switch (offset)
	{
	case 0x00:
		logerror("%s: TIMER0 COUNT read\n", machine().describe_context());
		return m_timer[0][0];
	case 0x01:
		logerror("%s: TIMER0 CONTROL read\n", machine().describe_context());
		return m_timer[0][1];
	case 0x02:
		logerror("%s: TIMER0 LIMIT read\n", machine().describe_context());
		return m_timer[0][2];
	}
	return 0x00;
}

uint32_t arcompact_device::arcompact_auxreg100_TIMER1_r(offs_t offset)
{
	switch (offset)
	{
	case 0x00:
		logerror("%s: TIMER1 COUNT read\n", machine().describe_context());
		return m_timer[1][0];
	case 0x01:
		logerror("%s: TIMER1 CONTROL read\n", machine().describe_context());
		return m_timer[1][1];
	case 0x02:
		logerror("%s: TIMER1 LIMIT read\n", machine().describe_context());
		return m_timer[1][2];
	}
	return 0x00;
}

void arcompact_device::arcompact_auxreg012_TIMER0_w(offs_t offset, uint32_t data)
{
	switch (offset)
	{
	case 0x00:
		m_timer[0][0] = data;
		logerror("%s: TIMER0 COUNT write %08x\n", machine().describe_context(), data);
		break;
	case 0x01:
		m_timer[0][1] = data;
		logerror("%s: TIMER0 CONTROL write %08x\n", machine().describe_context(), data);
		break;
	case 0x02:
		m_timer[0][2] = data;
		logerror("%s: TIMER0 LIMIT write %08x\n", machine().describe_context(), data);
		break;
	}
}

void arcompact_device::arcompact_auxreg100_TIMER1_w(offs_t offset, uint32_t data)
{
	switch (offset)
	{
	case 0x00:
		m_timer[1][0] = data;
		logerror("%s: TIMER1 COUNT write %08x\n", machine().describe_context(), data);
		break;
	case 0x01:
		m_timer[1][1] = data;
		logerror("%s: TIMER1 CONTROL write %08x\n", machine().describe_context(), data);
		break;
	case 0x02:
		m_timer[1][2] = data;
		logerror("%s: TIMER1 LIMIT write %08x\n", machine().describe_context(), data);
		break;
	}
}

uint32_t arcompact_device::arcompact_auxreg200_AUX_IRQ_LVL_r()
{
	logerror("%s: arcompact_auxreg200_AUX_IRQ_LVL_r\n", machine().describe_context());
	return 0;
}

void arcompact_device::arcompact_auxreg200_AUX_IRQ_LVL_w(uint32_t data)
{
	logerror("%s: arcompact_auxreg200_AUX_IRQ_LVL_w %08x\n", machine().describe_context(), data);
	m_AUX_IRQ_LEV = data;
}



void arcompact_device::arcompact_auxreg_map(address_map& map)
{
	//map(0x000000000, 0x000000000) // STATUS register in ARCtangent-A4 format (legacy)
	//map(0x000000001, 0x000000001) // SEMAPHORE (for multi-cpu comms)
	map(0x000000002, 0x000000002).rw(FUNC(arcompact_device::arcompact_auxreg002_LPSTART_r), FUNC(arcompact_device::arcompact_auxreg002_LPSTART_w));
	map(0x000000003, 0x000000003).rw(FUNC(arcompact_device::arcompact_auxreg003_LPEND_r), FUNC(arcompact_device::arcompact_auxreg003_LPEND_w));
	//map(0x000000004, 0x000000004) // IDENTITY (processor ID register)
	//map(0x000000005, 0x000000005) // DEBUG (various flags, including SLEEP)
	//map(0x000000006, 0x000000006) // PC (alt way of reading current PC)
	map(0x00000000a, 0x00000000a).r(FUNC(arcompact_device::arcompact_auxreg00a_STATUS32_r)); // r/o
	map(0x00000000b, 0x00000000b).rw(FUNC(arcompact_device::arcompact_auxreg00b_STATUS32_L1_r), FUNC(arcompact_device::arcompact_auxreg00b_STATUS32_L1_w));
	map(0x00000000c, 0x00000000c).rw(FUNC(arcompact_device::arcompact_auxreg00c_STATUS32_L2_r), FUNC(arcompact_device::arcompact_auxreg00c_STATUS32_L2_w));

	map(0x000000012, 0x000000012).w(FUNC(arcompact_device::arcompact_auxreg012_MULHI_w));

	map(0x000000021, 0x000000023).rw(FUNC(arcompact_device::arcompact_auxreg012_TIMER0_r), FUNC(arcompact_device::arcompact_auxreg012_TIMER0_w));

	map(0x000000025, 0x000000025).rw(FUNC(arcompact_device::arcompact_auxreg025_INTVECTORBASE_r), FUNC(arcompact_device::arcompact_auxreg025_INTVECTORBASE_w));

	//map(0x000000041, 0x000000041) // AUX_MACMODE (used by optional maths extensions)

	map(0x000000043, 0x000000043).rw(FUNC(arcompact_device::arcompact_auxreg043_AUX_IRQ_LV12_r), FUNC(arcompact_device::arcompact_auxreg043_AUX_IRQ_LV12_w));

	// these registers can be used to check which optional capabilities any given CPU was built with
	//map(0x000000060, 0x000000060) // BCR_VER - Build Configuration Registers Version
	//map(0x000000063, 0x000000063) // BTA_LINK_BUILD - Build configuration for BTA Registers
	//map(0x000000065, 0x000000065) // EA_BUILD - Build configuration for Extended Arithmetic
	//map(0x000000068, 0x000000068) // VECBASE_AC_BUILD - Build configuration for Interrupts
	//map(0x00000006e, 0x00000006e) // RF_BUILD - Build configuration for Core Registers
	//map(0x000000075, 0x000000075) // TIMER_BUILD - Build configuration for Processor Timers
	//map(0x00000007b, 0x00000007b) // MULTIPLY_BUILD - Build configuration for Multiply
	//map(0x00000007c, 0x00000007c) // SWAP_BUILD - Build configuration for Swap
	//map(0x00000007d, 0x00000007d) // NORM_BUILD - Build configuration for Normalize
	//map(0x00000007e, 0x00000007e) // MINMAX_BUILD - Build configuration for Min/Max
	//map(0x00000007f, 0x00000007f) // BARREL_BUILD - Build configuration for Barrel Shift

	map(0x000000100, 0x000000102).rw(FUNC(arcompact_device::arcompact_auxreg100_TIMER1_r), FUNC(arcompact_device::arcompact_auxreg100_TIMER1_w));

	map(0x000000200, 0x000000200).rw(FUNC(arcompact_device::arcompact_auxreg200_AUX_IRQ_LVL_r), FUNC(arcompact_device::arcompact_auxreg200_AUX_IRQ_LVL_w));
	//map(0x000000201, 0x000000201) // AUX_IRQ_HINT

	// below are ARC700 registers
	//map(0x000000400, 0x000000400) // ERET
	//map(0x000000401, 0x000000401) // ERBTA
	//map(0x000000402, 0x000000402) // ERSTATUS
	//map(0x000000403, 0x000000403) // ECR
	//map(0x000000404, 0x000000404) // EFA
	//map(0x00000040a, 0x00000040a) // ICAUSE1
	//map(0x00000040b, 0x00000040b) // ICAUSE2
	//map(0x00000040c, 0x00000040c) // AUX_IENABLE
	//map(0x00000040d, 0x00000040d) // AUX_ITRIGGER
	//map(0x000000410, 0x000000410) // XPU
	//map(0x000000412, 0x000000412) // BTA
	//map(0x000000413, 0x000000413) // BTA_L1
	//map(0x000000414, 0x000000414) // BTA_L2
	//map(0x000000415, 0x000000415) // AUX_IRQ_PULSE_CANCEL
	//map(0x000000416, 0x000000416) // AUX_IRQ_PENDING
}

arcompact_device::arcompact_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: cpu_device(mconfig, ARCA5, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0) // some docs describe these as 'middle endian'
	, m_io_config("io", ENDIANNESS_LITTLE, 32, 32, -2, address_map_constructor(FUNC(arcompact_device::arcompact_auxreg_map), this))
	, m_default_vector_base(0)
{
}

device_memory_interface::space_config_vector arcompact_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

std::unique_ptr<util::disasm_interface> arcompact_device::create_disassembler()
{
	return std::make_unique<arcompact_disassembler>();
}


/*****************************************************************************/

/*****************************************************************************/

void arcompact_device::unimplemented_opcode(uint16_t op)
{
	fatalerror("ARCOMPACT: unknown opcode %04x at %04x\n", op, m_pc << 2);
}

/*****************************************************************************/


/*****************************************************************************/

void arcompact_device::device_start()
{
	m_pc = 0;

	m_debugger_temp = 0;

	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);

	state_add(ARCOMPACT_PC, "PC", m_debugger_temp).callimport().callexport().formatstr("%08X");

	state_add(ARCOMPACT_STATUS32, "STATUS32", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add(ARCOMPACT_LP_START, "LP_START", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add(ARCOMPACT_LP_END, "LP_END", m_debugger_temp).callimport().callexport().formatstr("%08X");

	state_add(STATE_GENPCBASE, "CURPC", m_debugger_temp).callimport().callexport().noshow();

	for (int i = 0x100; i < 0x140; i++)
	{
		state_add(i, arcompact_disassembler::regnames[i - 0x100], m_debugger_temp).callimport().callexport().formatstr("%08X");
	}

	m_irq_pending = false;

	set_icountptr(m_icount);

	save_item(NAME(m_pc));
	save_item(NAME(m_regs));
	save_item(NAME(m_delayactive));
	save_item(NAME(m_delaylinks));
	save_item(NAME(m_delayjump));
	save_item(NAME(m_allow_loop_check));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_status32));
	save_item(NAME(m_status32_l1));
	save_item(NAME(m_status32_l2));
	save_item(NAME(m_debug));
	save_item(NAME(m_timer));
	save_item(NAME(m_LP_START));
	save_item(NAME(m_LP_END));
	save_item(NAME(m_INTVECTORBASE));
	save_item(NAME(m_AUX_IRQ_LV12));
	save_item(NAME(m_AUX_IRQ_LEV));
}


//-------------------------------------------------
//  state_export - export state from the device,
//  to a known location where it can be read
//-------------------------------------------------

void arcompact_device::state_export(const device_state_entry& entry)
{
	int index = entry.index();

	switch (index)
	{
	case ARCOMPACT_PC:
	case STATE_GENPCBASE:
		m_debugger_temp = m_pc;
		break;

	case ARCOMPACT_STATUS32:
		m_debugger_temp = m_status32;
		break;
	case ARCOMPACT_LP_START:
		m_debugger_temp = m_LP_START;
		break;
	case ARCOMPACT_LP_END:
		m_debugger_temp = m_LP_END;
		break;

	default:
		if ((index >= 0x100) && (index < 0x140))
		{
			m_debugger_temp = m_regs[index - 0x100];
		}
		break;
	}
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void arcompact_device::state_import(const device_state_entry& entry)
{
	int index = entry.index();

	switch (index)
	{
	case ARCOMPACT_PC:
	case STATE_GENPCBASE:
		m_pc = (m_debugger_temp & 0xfffffffe);
		break;

	case ARCOMPACT_STATUS32:
		m_status32 = m_debugger_temp;
		break;
	case ARCOMPACT_LP_START:
		m_LP_START = m_debugger_temp;
		break;
	case ARCOMPACT_LP_END:
		m_LP_END = m_debugger_temp;
		break;

	default:
		if ((index >= 0x100) && (index < 0x140))
		{
			m_regs[index - 0x100] = m_debugger_temp;
		}
		break;
	}
}

void arcompact_device::device_reset()
{
	m_pc = m_INTVECTORBASE = m_default_vector_base;

	m_delayactive = false;
	m_delayjump = 0x00000000;
	m_irq_pending = false;

	for (auto& elem : m_regs)
		elem = 0;

	m_status32 = 0;
	m_status32_l1 = 0;
	m_status32_l2 = 0;
	m_debug = 0;

	m_LP_START = 0;
	m_LP_END = 0;
	m_AUX_IRQ_LV12 = 0;
	m_AUX_IRQ_LEV = 0x000000c0; // only 2 interrupts are set to 'medium' priority (level 2) by default.

	m_allow_loop_check = true;

	for (int t = 0; t < 2; t++)
		for (int r = 0; r < 3; r++)
			m_timer[t][r] = 0x00;
}


/*****************************************************************************/

void arcompact_device::execute_set_input(int irqline, int state)
{
	if (state == ASSERT_LINE)
		m_irq_pending = true;
	else
		m_irq_pending = false;
}

/*****************************************************************************/

uint32_t arcompact_device::get_instruction(uint32_t op)
{
	uint8_t instruction = ((op & 0xf800) >> 11);

	if (instruction < 0x0c)
	{
		op <<= 16;
		op |= READ16((m_pc + 2));

		switch (instruction & 0x3f) // 32-bit instructions (with optional extra dword for immediate data)
		{
			default: return -1;
			case 0x00: // Bcc
			{
				uint8_t subinstr = (op & 0x00010000) >> 16;

				switch (subinstr & 0x01)
				{
					default: return -1;
					case 0x00:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// B<cc><.d> s21                   0000 0sss ssss sss0   SSSS SSSS SSNQ QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_B_cc_D_s21(op);  // Branch Conditionally
					}
					case 0x01:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// B<.d> s25                       0000 0sss ssss sss1   SSSS SSSS SSNR tttt
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_B_D_s25(op);  // Branch Unconditionally Far
					}
				}
			}
			case 0x01: // BLcc/BRcc
			{
				uint8_t subinstr = (op & 0x00010000) >> 16;
				switch (subinstr & 0x01)
				{
					default: return -1;
					case 0x00: // Branch & Link
					{
						uint8_t subinstr2 = (op & 0x00020000) >> 17;
						switch (subinstr2)
						{
							default: return -1;
							case 0x00:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BL<.cc><.d> s21                 0000 1sss ssss ss00   SSSS SSSS SSNQ QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_BL_cc_d_s21(op);  // Branch and Link Conditionally
							}
							case 0x01:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BL<.d> s25                      0000 1sss ssss ss10   SSSS SSSS SSNR tttt
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_BL_d_s25(op);  // Branch and Link Unconditional Far
							}
						}
					}
					case 0x01: // Branch on Compare
					{
						uint8_t subinstr2 = (op & 0x00000010) >> 4;

						switch (subinstr2)
						{
							default: return -1;
							case 0x00: // Branch on Compare Register-Register
							{
								uint8_t subinstr3 = op & 0x0000000f;
								switch (subinstr3)
								{
									case 0x00:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BREQ<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0000
// BREQ b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0000 (+ Limm)
// BREQ limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_reg(op, 0);  // BREQ (reg-reg)
									}
									case 0x01:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRNE<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0001
// BRNE b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0001 (+ Limm)
// BRNE limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_reg(op, 1); // BRNE (reg-reg)
									}
									case 0x02:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLT<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0010
// BRLT b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0010 (+ Limm)
// BRLT limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0010 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_reg(op, 2); // BRLT (reg-reg)
									}
									case 0x03:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRGE<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0011
// BRGE b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0011 (+ Limm)
// BRGE limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_reg(op, 3); // BRGE (reg-reg)
									}
									case 0x04:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLO<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0100
// BRLO b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0100 (+ Limm)
// BRLO limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_reg(op, 4); // BRLO (reg-reg)
									}
									case 0x05:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRHS b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0101 (+ Limm)
// BRHS limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0101 (+ Limm)
// BRHS<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0101
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_reg(op, 5); // BRHS (reg-reg)
									}
									case 0x0e:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT0<.d> b,c,s9                0000 1bbb ssss sss1   SBBB CCCC CCN0 1110
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_reg(op, 6); // BBIT0 (reg-reg)
									}
									case 0x0f:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT1<.d> b,c,s9                0000 1bbb ssss sss1   SBBB CCCC CCN0 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_reg(op, 7); // BBIT1 (reg-reg)
									}
									default:
									{
										// 0x06 - 0x0d
										return arcompact_handle_reserved(instruction, subinstr, subinstr2, subinstr3, op);  // illegal
									}
								}
							}
							case 0x01: // Branch on Compare/Bit Test Register-Immediate
							{
								uint8_t subinstr3 = op & 0x0000000f;
								switch (subinstr3)
								{
									case 0x00:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BREQ<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0000
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_imm(op, 0); // BREQ (reg-imm)
									}
									case 0x01:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRNE<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0001
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_imm(op, 1); // BRNE (reg-imm)
									}
									case 0x02:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLT<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0010
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_imm(op, 2); // BRLT (reg-imm)
									}
									case 0x03:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRGE<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0011
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_imm(op, 3); // BRGE (reg-imm)
									}
									case 0x04:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLO<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0100
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_imm(op, 4); // BRLO (reg-imm)
									}
									case 0x05:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRHS<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0101
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_imm(op, 5); // BRHS (reg-imm)
									}
									case 0x0e:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT0<.d> b,u6,s9               0000 1bbb ssss sss1   SBBB uuuu uuN1 1110
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_imm(op, 6); // BBIT0 (reg-imm)
									}
									case 0x0f:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT1<.d> b,u6,s9               0000 1bbb ssss sss1   SBBB uuuu uuN1 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRxx_reg_imm(op, 7); // BBIT1 (reg-imm)
									}
									default:
									{
										// 0x06 - 0x0d
										return arcompact_handle_reserved(instruction, subinstr, subinstr2, subinstr3, op);  // illegal
									}
								}
							}
						}
					}
				}
			}
			case 0x02:
			{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I
// LD<zz><.x><.aa><.di> a,[b,s9]   0001 0bbb ssss ssss   SBBB DaaZ ZXAA AAAA
// LD<zz><.x><.di> a,[limm]        0001 0110 0000 0000   0111 DRRZ ZXAA AAAA (+ Limm)
// LD<zz><.x><.aa><.di> 0,[b,s9]   0001 0bbb ssss ssss   SBBB DaaZ ZX11 1110
// LD<zz><.x><.di> 0,[limm]        0001 0110 0000 0000   0111 DRRZ ZX11 1110 (+ Limm)
//
// PREFETCH<.aa> [b,s9]            0001 0bbb ssss ssss   SBBB 0aa0 0011 1110
// PREFETCH [limm]                 0001 0110 0000 0000   0111 0RR0 0011 1110 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				return handleop32_LD_r_o(op);    // LD r+o
			}
			case 0x03:
			{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I
// ST<zz><.aa><.di> c,[b,s9]       0001 1bbb ssss ssss   SBBB CCCC CCDa aZZR
// ST<zz><.di> c,[limm]            0001 1110 0000 0000   0111 CCCC CCDR RZZR (+ Limm)
// ST<zz><.aa><.di> limm,[b,s9]    0001 1bbb ssss ssss   SBBB 1111 10Da aZZR (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				return handleop32_ST_r_o(op);    // ST r+o
			}
			case 0x04: // op a,b,c (basecase)
			{
				uint8_t subinstr = (op & 0x003f0000) >> 16;

				switch (subinstr & 0x3f)
				{
					case 0x00:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADD<.f> a,b,c                   0010 0bbb 0000 0000   FBBB CCCC CCAA AAAA
// ADD<.f> a,b,u6                  0010 0bbb 0100 0000   FBBB uuuu uuAA AAAA
// ADD<.f> b,b,s12                 0010 0bbb 1000 0000   FBBB ssss ssSS SSSS
// ADD<.cc><.f> b,b,c              0010 0bbb 1100 0000   FBBB CCCC CC0Q QQQQ
// ADD<.cc><.f> b,b,u6             0010 0bbb 1100 0000   FBBB uuuu uu1Q QQQQ
// ADD<.f> a,limm,c                0010 0110 0000 0000   F111 CCCC CCAA AAAA (+ Limm)
// ADD<.f> a,b,limm                0010 0bbb 0000 0000   FBBB 1111 10AA AAAA (+ Limm)
// ADD<.cc><.f> b,b,limm           0010 0bbb 1100 0000   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADD<.f> 0,b,c                   0010 0bbb 0000 0000   FBBB CCCC CC11 1110
// ADD<.f> 0,b,u6                  0010 0bbb 0100 0000   FBBB uuuu uu11 1110
// ADD<.f> 0,b,limm                0010 0bbb 0000 0000   FBBB 1111 1011 1110 (+ Limm)
// ADD<.cc><.f> 0,limm,c           0010 0110 1100 0000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ADD_do_op); // ADD
					}
					case 0x01:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADC<.f> a,b,c                   0010 0bbb 0000 0001   FBBB CCCC CCAA AAAA
// ADC<.f> a,b,u6                  0010 0bbb 0100 0001   FBBB uuuu uuAA AAAA
// ADC<.f> b,b,s12                 0010 0bbb 1000 0001   FBBB ssss ssSS SSSS
// ADC<.cc><.f> b,b,c              0010 0bbb 1100 0001   FBBB CCCC CC0Q QQQQ
// ADC<.cc><.f> b,b,u6             0010 0bbb 1100 0001   FBBB uuuu uu1Q QQQQ
// ADC<.f> a,limm,c                0010 0110 0000 0001   F111 CCCC CCAA AAAA (+ Limm)
// ADC<.f> a,b,limm                0010 0bbb 0000 0001   FBBB 1111 10AA AAAA (+ Limm)
// ADC<.cc><.f> b,b,limm           0010 0bbb 1100 0001   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADC<.f> 0,b,c                   0010 0bbb 0000 0001   FBBB CCCC CC11 1110
// ADC<.f> 0,b,u6                  0010 0bbb 0100 0001   FBBB uuuu uu11 1110
// ADC<.f> 0,b,limm                0010 0bbb 0000 0001   FBBB 1111 1011 1110 (+ Limm)
// ADC<.cc><.f> 0,limm,c           0010 0110 1100 0001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ADC_do_op);  // ADC
					}
					case 0x02:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SUB<.f> a,b,c                   0010 0bbb 0000 0010   FBBB CCCC CCAA AAAA
// SUB<.f> a,b,u6                  0010 0bbb 0100 0010   FBBB uuuu uuAA AAAA
// SUB<.f> b,b,s12                 0010 0bbb 1000 0010   FBBB ssss ssSS SSSS
// SUB<.cc><.f> b,b, c             0010 0bbb 1100 0010   FBBB CCCC CC0Q QQQQ
// SUB<.cc><.f> b,b,u6             0010 0bbb 1100 0010   FBBB uuuu uu1Q QQQQ
// SUB<.f> a,limm,c                0010 0110 0000 0010   F111 CCCC CCAA AAAA (+ Limm)
// SUB<.f> a,b,limm                0010 0bbb 0000 0010   FBBB 1111 10AA AAAA (+ Limm)
// SUB<.cc><.f> b,b,limm           0010 0bbb 1100 0010   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB <.f> 0,b,c                  0010 0bbb 0000 0010   FBBB CCCC CC11 1110
// SUB <.f> 0,b,u6                 0010 0bbb 0100 0010   FBBB uuuu uu11 1110
// SUB <.f> 0,b,limm               0010 0bbb 0000 0010   FBBB 1111 1011 1110 (+ Limm)
// SUB <.cc><.f> 0,limm,c          0010 0110 1100 0010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_SUB_do_op);  // SUB
					}
					case 0x03:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SBC<.f> a,b,c                   0010 0bbb 0000 0011   FBBB CCCC CCAA AAAA
// SBC<.f> a,b,u6                  0010 0bbb 0100 0011   FBBB uuuu uuAA AAAA
// SBC<.f> b,b,s12                 0010 0bbb 1000 0011   FBBB ssss ssSS SSSS
// SBC<.cc><.f> b,b,c              0010 0bbb 1100 0011   FBBB CCCC CC0Q QQQQ
// SBC<.cc><.f> b,b,u6             0010 0bbb 1100 0011   FBBB uuuu uu1Q QQQQ
// SBC<.f> a,limm,c                0010 0110 0000 0011   F111 CCCC CCAA AAAA (+ Limm)
// SBC<.f> a,b,limm                0010 0bbb 0000 0011   FBBB 1111 10AA AAAA (+ Limm)
// SBC<.cc><.f> b,b,limm           0010 0bbb 1100 0011   FBBB 1111 100Q QQQQ (+ Limm)
//
// SBC<.f> 0,b,c                   0010 0bbb 0000 0011   FBBB CCCC CC11 1110
// SBC<.f> 0,b,u6                  0010 0bbb 0100 0011   FBBB uuuu uu11 1110
// SBC<.f> 0,b,limm                0010 0bbb 0000 0011   FBBB 1111 1011 1110 (+ Limm)
// SBC<.cc><.f> 0,limm,c           0010 0110 1100 0011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_SBC_do_op);  // SBC
					}
					case 0x04:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AND<.f> a,b,c                   0010 0bbb 0000 0100   FBBB CCCC CCAA AAAA
// AND<.f> a,b,u6                  0010 0bbb 0100 0100   FBBB uuuu uuAA AAAA
// AND<.f> b,b,s12                 0010 0bbb 1000 0100   FBBB ssss ssSS SSSS
// AND<.cc><.f> b,b,c              0010 0bbb 1100 0100   FBBB CCCC CC0Q QQQQ
// AND<.cc><.f> b,b,u6             0010 0bbb 1100 0100   FBBB uuuu uu1Q QQQQ
// AND<.f> a,limm,c                0010 0110 0000 0100   F111 CCCC CCAA AAAA (+ Limm)
// AND<.f> a,b,limm                0010 0bbb 0000 0100   FBBB 1111 10AA AAAA (+ Limm)
// AND<.cc><.f> b,b,limm           0010 0bbb 1100 0100   FBBB 1111 100Q QQQQ (+ Limm)
//
// AND<.f> 0,b,c                   0010 0bbb 0000 0100   FBBB CCCC CC11 1110
// AND<.f> 0,b,u6                  0010 0bbb 0100 0100   FBBB uuuu uu11 1110
// AND<.f> 0,b,limm                0010 0bbb 0000 0100   FBBB 1111 1011 1110 (+ Limm)
// AND<.cc><.f> 0,limm,c           0010 0110 1100 0100   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_AND_do_op);  // AND
					}
					case 0x05:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OR<.f> a,b,c                    0010 0bbb 0000 0101   FBBB CCCC CCAA AAAA
// OR<.f> a,b,u6                   0010 0bbb 0100 0101   FBBB uuuu uuAA AAAA
// OR<.f> b,b,s12                  0010 0bbb 1000 0101   FBBB ssss ssSS SSSS
// OR<.cc><.f> b,b,c               0010 0bbb 1100 0101   FBBB CCCC CC0Q QQQQ
// OR<.cc><.f> b,b,u6              0010 0bbb 1100 0101   FBBB uuuu uu1Q QQQQ
// OR<.f> a,limm,c                 0010 0110 0000 0101   F111 CCCC CCAA AAAA (+ Limm)
// OR<.f> a,b,limm                 0010 0bbb 0000 0101   FBBB 1111 10AA AAAA (+ Limm)
// OR<.cc><.f> b,b,limm            0010 0bbb 1100 0101   FBBB 1111 100Q QQQQ (+ Limm)
//
// OR<.f> 0,b,c                    0010 0bbb 0000 0101   FBBB CCCC CC11 1110
// OR<.f> 0,b,u6                   0010 0bbb 0100 0101   FBBB uuuu uu11 1110
// OR<.f> 0,b,limm                 0010 0bbb 0000 0101   FBBB 1111 1011 1110 (+ Limm)
// OR<.cc><.f> 0,limm,c            0010 0110 1100 010  1 F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_OR_do_op);  // OR
					}
					case 0x06:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// BIC<.f> a,b,c                   0010 0bbb 0000 0110   FBBB CCCC CCAA AAAA
// BIC<.f> a,b,u6                  0010 0bbb 0100 0110   FBBB uuuu uuAA AAAA
// BIC<.f> b,b,s12                 0010 0bbb 1000 0110   FBBB ssss ssSS SSSS
// BIC<.cc><.f> b,b,c              0010 0bbb 1100 0110   FBBB CCCC CC0Q QQQQ
// BIC<.cc><.f> b,b,u6             0010 0bbb 1100 0110   FBBB uuuu uu1Q QQQQ
// BIC<.f> a,limm,c                0010 0110 0000 0110   F111 CCCC CCAA AAAA (+ Limm)
// BIC<.f> a,b,limm                0010 0bbb 0000 0110   FBBB 1111 10AA AAAA (+ Limm)
// BIC<.cc><.f> b,b,limm           0010 0bbb 1100 0110   FBBB 1111 100Q QQQQ (+ Limm)
//
// BIC<.f> 0,b,c                   0010 0bbb 0000 0110   FBBB CCCC CC11 1110
// BIC<.f> 0,b,u6                  0010 0bbb 0100 0110   FBBB uuuu uu11 1110
// BIC<.f> 0,b,limm                0010 0bbb 0000 0110   FBBB 1111 1011 1110 (+ Limm)
// BIC<.cc><.f> 0,limm,c           0010 0110 1100 0110   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_BIC_do_op);  // BIC
					}
					case 0x07:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// XOR<.f> a,b,c                   0010 0bbb 0000 0111   FBBB CCCC CCAA AAAA
// XOR<.f> a,b,u6                  0010 0bbb 0100 0111   FBBB uuuu uuAA AAAA
// XOR<.f> b,b,s12                 0010 0bbb 1000 0111   FBBB ssss ssSS SSSS
// XOR<.cc><.f> b,b,c              0010 0bbb 1100 0111   FBBB CCCC CC0Q QQQQ
// XOR<.cc><.f> b,b,u6             0010 0bbb 1100 0111   FBBB uuuu uu1Q QQQQ
// XOR<.f> a,limm,c                0010 0110 0000 0111   F111 CCCC CCAA AAAA (+ Limm)
// XOR<.f> a,b,limm                0010 0bbb 0000 0111   FBBB 1111 10AA AAAA (+ Limm)
// XOR<.cc><.f> b,b,limm           0010 0bbb 1100 0111   FBBB 1111 100Q QQQQ (+ Limm)
//
// XOR<.f> 0,b,c                   0010 0bbb 0000 0111   FBBB CCCC CC11 1110
// XOR<.f> 0,b,u6                  0010 0bbb 0100 0111   FBBB uuuu uu11 1110
// XOR<.f> 0,b,limm                0010 0bbb 0000 0111   FBBB 1111 1011 1110 (+ Limm)
// XOR<.cc><.f> 0,limm,c           0010 0110 1100 0111   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_XOR_do_op);  // XOR
					}
					case 0x08:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MAX<.f> a,b,c                   0010 0bbb 0000 1000   FBBB CCCC CCAA AAAA
// MAX<.f> a,b,u6                  0010 0bbb 0100 1000   FBBB uuuu uuAA AAAA
// MAX<.f> b,b,s12                 0010 0bbb 1000 1000   FBBB ssss ssSS SSSS
// MAX<.cc><.f> b,b,c              0010 0bbb 1100 1000   FBBB CCCC CC0Q QQQQ
// MAX<.cc><.f> b,b,u6             0010 0bbb 1100 1000   FBBB uuuu uu1Q QQQQ
// MAX<.f> a,limm,c                0010 0110 0000 1000   F111 CCCC CCAA AAAA (+ Limm)
// MAX<.f> a,b,limm                0010 0bbb 0000 1000   FBBB 1111 10AA AAAA (+ Limm)
// MAX<.cc><.f> b,b,limm           0010 0bbb 1100 1000   FBBB 1111 100Q QQQQ (+ Limm)
//
// MAX<.f> 0,b,c                   0010 0bbb 0000 1000   FBBB CCCC CC11 1110
// MAX<.f> 0,b,u6                  0010 0bbb 0100 1000   FBBB uuuu uu11 1110
// MAX<.f> 0,b,limm                0010 0bbb 0000 1000   FBBB 1111 1011 1110 (+ Limm)
// MAX<.cc><.f> 0,limm,c           0010 0110 1100 1000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_MAX_do_op);  // MAX
					}
					case 0x09:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MIN<.f> a,b,c                   0010 0bbb 0000 1001   FBBB CCCC CCAA AAAA
// MIN<.f> a,b,u6                  0010 0bbb 0100 1001   FBBB uuuu uuAA AAAA
// MIN<.f> b,b,s12                 0010 0bbb 1000 1001   FBBB ssss ssSS SSSS
// MIN<.cc><.f> b,b,c              0010 0bbb 1100 1001   FBBB CCCC CC0Q QQQQ
// MIN<.cc><.f> b,b,u6             0010 0bbb 1100 1001   FBBB uuuu uu1Q QQQQ
// MIN<.f> a,limm,c                0010 0110 0000 1001   F111 CCCC CCAA AAAA (+ Limm)
// MIN<.f> a,b,limm                0010 0bbb 0000 1001   FBBB 1111 10AA AAAA (+ Limm)
// MIN<.cc><.f> b,b,limm           0010 0bbb 1100 1001   FBBB 1111 100Q QQQQ (+ Limm)
//
// MIN<.f> 0,b,c                   0010 0bbb 0000 1001   FBBB CCCC CC11 1110
// MIN<.f> 0,b,u6                  0010 0bbb 0100 1001   FBBB uuuu uu11 1110
// MIN<.f> 0,b,limm                0010 0bbb 0000 1001   FBBB 1111 1011 1110 (+ Limm)
// MIN<.cc><.f> 0,limm,c           0010 0110 1100 1001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_MIN_do_op);  // MIN
					}
					case 0x0a:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MOV<.f> b,s12                   0010 0bbb 1000 1010   FBBB ssss ssSS SSSS
// MOV<.cc><.f> b,c                0010 0bbb 1100 1010   FBBB CCCC CC0Q QQQQ
// MOV<.cc><.f> b,u6               0010 0bbb 1100 1010   FBBB uuuu uu1Q QQQQ
// MOV<.cc><.f> b,limm             0010 0bbb 1100 1010   FBBB 1111 100Q QQQQ (+ Limm)
//
// MOV<.f> 0,s12                   0010 0110 1000 1010   F111 ssss ssSS SSSS
// MOV<.cc><.f> 0,c                0010 0110 1100 1010   F111 CCCC CC0Q QQQQ
// MOV<.cc><.f> 0,u6               0010 0110 1100 1010   F111 uuuu uu1Q QQQQ
// MOV<.cc><.f> 0,limm             0010 0110 1100 1010   F111 1111 100Q QQQQ (+ Limm)
//
// NOP                             0010 0110 0100 1010   0111 0000 0000 0000 (NOP is a custom encoded MOV?)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_MOV(op);  // MOV
					}
					case 0x0b:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// TST b,s12                       0010 0bbb 1000 1011   1BBB ssss ssSS SSSS
// TST<.cc> b,c                    0010 0bbb 1100 1011   1BBB CCCC CC0Q QQQQ
// TST<.cc> b,u6                   0010 0bbb 1100 1011   1BBB uuuu uu1Q QQQQ
// TST<.cc> b,limm                 0010 0bbb 1100 1011   1BBB 1111 100Q QQQQ (+ Limm)
// TST<.cc> limm,c                 0010 0110 1100 1011   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general_nowriteback_forced_flag(op, handleop32_TST_do_op);  // TST
					}
					case 0x0c:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// CMP b,s12                       0010 0bbb 1000 1100   1BBB ssss ssSS SSSS
// CMP<.cc> b,c                    0010 0bbb 1100 1100   1BBB CCCC CC0Q QQQQ
// CMP<.cc> b,u6                   0010 0bbb 1100 1100   1BBB uuuu uu1Q QQQQ
// CMP<.cc> b,limm                 0010 0bbb 1100 1100   1BBB 1111 100Q QQQQ (+ Limm)
// CMP<.cc> limm,c                 0010 0110 1100 1100   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general_nowriteback_forced_flag(op, handleop32_CMP_do_op);  // CMP
					}
					case 0x0d:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RCMP b,s12                      0010 0bbb 1000 1101   1BBB ssss ssSS SSSS
// RCMP<.cc> b,c                   0010 0bbb 1100 1101   1BBB CCCC CC0Q QQQQ
// RCMP<.cc> b,u6                  0010 0bbb 1100 1101   1BBB uuuu uu1Q QQQQ
// RCMP<.cc> b,limm                0010 0bbb 1100 1101   1BBB 1111 100Q QQQQ (+ Limm)
// RCMP<.cc> limm,c                0010 0110 1100 1101   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general_nowriteback_forced_flag(op, handleop32_RCMP_do_op);  // RCMP
					}
					case 0x0e:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RSUB<.f> a,b,c                  0010 0bbb 0000 1110   FBBB CCCC CCAA AAAA
// RSUB<.f> a,b,u6                 0010 0bbb 0100 1110   FBBB uuuu uuAA AAAA
// NEG<.f> a,b                     0010 0bbb 0100 1110   FBBB 0000 00AA AAAA (NEG is an alias)
//
// RSUB<.f> b,b,s12                0010 0bbb 1000 1110   FBBB ssss ssSS SSSS
// RSUB<.cc><.f> b,b,c             0010 0bbb 1100 1110   FBBB CCCC CC0Q QQQQ
// RSUB<.cc><.f> b,b,u6            0010 0bbb 1100 1110   FBBB uuuu uu1Q QQQQ
// NEG<.cc><.f> b,b                0010 0bbb 1100 1110   FBBB 0000 001Q QQQQ (NEG is an alias)
//
// RSUB<.f> a,limm,c               0010 0110 0000 1110   F111 CCCC CCAA AAAA (+ Limm)
// RSUB<.f> a,b,limm               0010 0bbb 0000 1110   FBBB 1111 10AA AAAA (+ Limm)
// RSUB<.cc><.f> b,b,limm          0010 0bbb 1100 1110   FBBB 1111 100Q QQQQ (+ Limm)
//
// RSUB<.f> 0,b,c                  0010 0bbb 0000 1110   FBBB CCCC CC11 1110
// RSUB<.f> 0,b,u6                 0010 0bbb 0100 1110   FBBB uuuu uu11 1110
// RSUB<.f> 0,b,limm               0010 0bbb 0000 1110   FBBB 1111 1011 1110 (+ Limm)
// RSUB<.cc><.f> 0,limm,c          0010 0110 1100 1110   F111 CCCC CC0Q QQQQ (+ Limm)
//
//                                 IIII I      SS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_RSUB_do_op);  // RSUB
					}
					case 0x0f:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BSET<.f> a,b,c                  0010 0bbb 0000 1111   FBBB CCCC CCAA AAAA
// BSET<.f> a,b,u6                 0010 0bbb 0100 1111   FBBB uuuu uuAA AAAA
// BSET<.cc><.f> b,b,c             0010 0bbb 1100 1111   FBBB CCCC CC0Q QQQQ
// BSET<.cc><.f> b,b,u6            0010 0bbb 1100 1111   FBBB uuuu uu1Q QQQQ
// BSET<.f> a,limm,c               0010 0110 0000 1111   F111 CCCC CCAA AAAA (+ Limm)
//
// BSET<.f> 0,b,c                  0010 0bbb 0000 1111   FBBB CCCC CC11 1110
// BSET<.f> 0,b,u6                 0010 0bbb 0100 1111   FBBB uuuu uu11 1110
// BSET<.cc><.f> 0,limm,c          0010 0110 1100 1111   F110 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_BSET_do_op);  // BSET
					}
					case 0x10:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BCLR<.f> a,b,c                  0010 0bbb 0001 0000   FBBB CCCC CCAA AAAA
// BCLR<.f> a,b,u6                 0010 0bbb 0101 0000   FBBB uuuu uuAA AAAA
// BCLR<.cc><.f> b,b,c             0010 0bbb 1101 0000   FBBB CCCC CC0Q QQQQ
// BCLR<.cc><.f> b,b,u6            0010 0bbb 1101 0000   FBBB uuuu uu1Q QQQQ
// BCLR<.f> a,limm,c               0010 0110 0001 0000   F111 CCCC CCAA AAAA (+ Limm)
//
// BCLR<.f> 0,b,c                  0010 0bbb 0001 0000   FBBB CCCC CC11 1110
// BCLR<.f> 0,b,u6                 0010 0bbb 0101 0000   FBBB uuuu uu11 1110
// BCLR<.cc><.f> 0,limm,c          0010 0110 1101 0000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_BCLR_do_op);  // BCLR
					}
					case 0x11:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BTST<.cc> b,c                   0010 0bbb 1101 0001   1BBB CCCC CC0Q QQQQ
// BTST<.cc> b,u6                  0010 0bbb 1101 0001   1BBB uuuu uu1Q QQQQ
// BTST<.cc> limm,c                0010 0110 1101 0001   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general_nowriteback_forced_flag(op, handleop32_BTST_do_op);  // BTST
					}
					case 0x12:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BXOR<.f> a,b,c                  0010 0bbb 0001 0010   FBBB CCCC CCAA AAAA
// BXOR<.f> a,b,u6                 0010 0bbb 0101 0010   FBBB uuuu uuAA AAAA
// BXOR<.cc><.f> b,b,c             0010 0bbb 1101 0010   FBBB CCCC CC0Q QQQQ
// BXOR<.cc><.f> b,b,u6            0010 0bbb 1101 0010   FBBB uuuu uu1Q QQQQ
// BXOR<.f> a,limm,c               0010 0110 0001 0010   F111 CCCC CCAA AAAA (+ Limm)
//
// BXOR<.f> 0,b,c                  0010 0bbb 0001 0010   FBBB CCCC CC11 1110
// BXOR<.f> 0,b,u6                 0010 0bbb 0101 0010   FBBB uuuu uu11 1110
// BXOR<.cc><.f> 0,limm,c          0010 0110 1101 0010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_BXOR_do_op);  // BXOR
					}
					case 0x13:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BMSK<.f> a,b,c                  0010 0bbb 0001 0011   FBBB CCCC CCAA AAAA
// BMSK<.f> a,b,u6                 0010 0bbb 0101 0011   FBBB uuuu uuAA AAAA
// BMSK<.cc><.f> b,b,c             0010 0bbb 1101 0011   FBBB CCCC CC0Q QQQQ
// BMSK<.cc><.f> b,b,u6            0010 0bbb 1101 0011   FBBB uuuu uu1Q QQQQ
// BMSK<.f> a,limm,c               0010 0110 0001 0011   F111 CCCC CCAA AAAA (+ Limm)
//
// BMSK<.f> 0,b,c                  0010 0bbb 0001 0011   FBBB CCCC CC11 1110
// BMSK<.f> 0,b,u6                 0010 0bbb 0101 0011   FBBB uuuu uu11 1110
// BMSK<.cc><.f> 0,limm,c          0010 0110 1101 0011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_BMSK_do_op);  // BMSK
					}
					case 0x14:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ADD1<.f> a,b,c                  0010 0bbb 0001 0100   FBBB CCCC CCAA AAAA
// ADD1<.f> a,b,u6                 0010 0bbb 0101 0100   FBBB uuuu uuAA AAAA
// ADD1<.f> b,b,s12                0010 0bbb 1001 0100   FBBB ssss ssSS SSSS
// ADD1<.cc><.f> b,b,c             0010 0bbb 1101 0100   FBBB CCCC CC0Q QQQQ
// ADD1<.cc><.f> b,b,u6            0010 0bbb 1101 0100   FBBB uuuu uu1Q QQQQ
// ADD1<.f> a,limm,c               0010 0110 0001 0100   F111 CCCC CCAA AAAA (+ Limm)
// ADD1<.f> a,b,limm               0010 0bbb 0001 0100   FBBB 1111 10AA AAAA (+ Limm)
// ADD1<.cc><.f> b,b,limm          0010 0bbb 1101 0100   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADD1<.f> 0,b,c                  0010 0bbb 0001 0100   FBBB CCCC CC11 1110
// ADD1<.f> 0,b,u6                 0010 0bbb 0101 0100   FBBB uuuu uu11 1110
// ADD1<.f> 0,b,limm               0010 0bbb 0001 0100   FBBB 1111 1011 1110 (+ Limm)
// ADD1<.cc><.f> 0,limm,c          0010 0110 1101 0100   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ADD1_do_op);  // ADD1
					}
					case 0x15:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ADD2<.f> a,b,c                  0010 0bbb 0001 0101   FBBB CCCC CCAA AAAA
// ADD2<.f> a,b,u6                 0010 0bbb 0101 0101   FBBB uuuu uuAA AAAA
// ADD2<.f> b,b,s12                0010 0bbb 1001 0101   FBBB ssss ssSS SSSS
// ADD2<.cc><.f> b,b,c             0010 0bbb 1101 0101   FBBB CCCC CC0Q QQQQ
// ADD2<.cc><.f> b,b,u6            0010 0bbb 1101 0101   FBBB uuuu uu1Q QQQQ
// ADD2<.f> a,limm,c               0010 0110 0001 0101   F111 CCCC CCAA AAAA (+ Limm)
// ADD2<.f> a,b,limm               0010 0bbb 0001 0101   FBBB 1111 10AA AAAA (+ Limm)
// ADD2<.cc><.f> b,b,limm          0010 0bbb 1101 0101   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADD2<.f> 0,b,c                  0010 0bbb 0001 0101   FBBB CCCC CC11 1110
// ADD2<.f> 0,b,u6                 0010 0bbb 0101 0101   FBBB uuuu uu11 1110
// ADD2<.f> 0,b,limm               0010 0bbb 0001 0101   FBBB 1111 1011 1110 (+ Limm)
// ADD2<.cc><.f> 0,limm,c          0010 0110 1101 0101   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ADD2_do_op);  // ADD2
					}
					case 0x16:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ADD3<.f> a,b,c                  0010 0bbb 0001 0110   FBBB CCCC CCAA AAAA
// ADD3<.f> a,b,u6                 0010 0bbb 0101 0110   FBBB uuuu uuAA AAAA
// ADD3<.f> b,b,s12                0010 0bbb 1001 0110   FBBB ssss ssSS SSSS
// ADD3<.cc><.f> b,b,c             0010 0bbb 1101 0110   FBBB CCCC CC0Q QQQQ
// ADD3<.cc><.f> b,b,u6            0010 0bbb 1101 0110   FBBB uuuu uu1Q QQQQ
// ADD3<.f> a,limm,c               0010 0110 0001 0110   F111 CCCC CCAA AAAA (+ Limm)
// ADD3<.f> a,b,limm               0010 0bbb 0001 0110   FBBB 1111 10AA AAAA (+ Limm)
// ADD3<.cc><.f> b,b,limm          0010 0bbb 1101 0110   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADD3<.f> 0,b,c                  0010 0bbb 0001 0110   FBBB CCCC CC11 1110
// ADD3<.f> 0,b,u6                 0010 0bbb 0101 0110   FBBB uuuu uu11 1110
// ADD3<.f> 0,b,limm               0010 0bbb 0001 0110   FBBB 1111 1011 1110 (+ Limm)
// ADD3<.cc><.f> 0,limm,c          0010 0110 1101 0110   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ADD3_do_op);  // ADD3
					}
					case 0x17:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SUB1<.f> a,b,c                  0010 0bbb 0001 0111   FBBB CCCC CCAA AAAA
// SUB1<.f> a,b,u6                 0010 0bbb 0101 0111   FBBB uuuu uuAA AAAA
// SUB1<.f> b,b,s12                0010 0bbb 1001 0111   FBBB ssss ssSS SSSS
// SUB1<.cc><.f> b,b,c             0010 0bbb 1101 0111   FBBB CCCC CC0Q QQQQ
// SUB1<.cc><.f> b,b,u6            0010 0bbb 1101 0111   FBBB uuuu uu1Q QQQQ
// SUB1<.f> a,limm,c               0010 0110 0001 0111   F111 CCCC CCAA AAAA (+ Limm)
// SUB1<.f> a,b,limm               0010 0bbb 0001 0111   FBBB 1111 10AA AAAA (+ Limm)
// SUB1<.cc><.f> b,b,limm          0010 0bbb 1101 0111   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB1<.f> 0,b,c                  0010 0bbb 0001 0111   FBBB CCCC CC11 1110
// SUB1<.f> 0,b,u6                 0010 0bbb 0101 0111   FBBB uuuu uu11 1110
// SUB1<.f> 0,b,limm               0010 0bbb 0001 0111   FBBB 1111 1011 1110 (+ Limm)
// SUB1<.cc><.f> 0,limm,c          0010 0110 1101 0111   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_SUB1_do_op);  // SUB1
					}
					case 0x18:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SUB2<.f> a,b,c                  0010 0bbb 0001 1000   FBBB CCCC CCAA AAAA
// SUB2<.f> a,b,u6                 0010 0bbb 0101 1000   FBBB uuuu uuAA AAAA
// SUB2<.f> b,b,s12                0010 0bbb 1001 1000   FBBB ssss ssSS SSSS
// SUB2<.cc><.f> b,b,c             0010 0bbb 1101 1000   FBBB CCCC CC0Q QQQQ
// SUB2<.cc><.f> b,b,u6            0010 0bbb 1101 1000   FBBB uuuu uu1Q QQQQ
// SUB2<.f> a,limm,c               0010 0110 0001 1000   F111 CCCC CCAA AAAA (+ Limm)
// SUB2<.f> a,b,limm               0010 0bbb 0001 1000   FBBB 1111 10AA AAAA (+ Limm)
// SUB2<.cc><.f> b,b,limm          0010 0bbb 1101 1000   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB2<.f> 0,b,c                  0010 0bbb 0001 1000   FBBB CCCC CC11 1110
// SUB2<.f> 0,b,u6                 0010 0bbb 0101 1000   FBBB uuuu uu11 1110
// SUB2<.f> 0,b,limm               0010 0bbb 0001 1000   FBBB 1111 1011 1110 (+ Limm)
// SUB2<.cc><.f> 0,limm,c          0010 0110 1101 1000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_SUB2_do_op);  // SUB2
					}
					case 0x19:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SUB3<.f> a,b,c                  0010 0bbb 0001 1001   FBBB CCCC CCAA AAAA
// SUB3<.f> a,b,u6                 0010 0bbb 0101 1001   FBBB uuuu uuAA AAAA
// SUB3<.f> b,b,s12                0010 0bbb 1001 1001   FBBB ssss ssSS SSSS
// SUB3<.cc><.f> b,b,c             0010 0bbb 1101 1001   FBBB CCCC CC0Q QQQQ
// SUB3<.cc><.f> b,b,u6            0010 0bbb 1101 1001   FBBB uuuu uu1Q QQQQ
// SUB3<.f> a,limm,c               0010 0110 0001 1001   F111 CCCC CCAA AAAA (+ Limm)
// SUB3<.f> a,b,limm               0010 0bbb 0001 1001   FBBB 1111 10AA AAAA (+ Limm)
// SUB3<.cc><.f> b,b,limm          0010 0bbb 1101 1001   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB3<.f> 0,b,c                  0010 0bbb 0001 1001   FBBB CCCC CC11 1110
// SUB3<.f> 0,b,u6                 0010 0bbb 0101 1001   FBBB uuuu uu11 1110
// SUB3<.f> 0,limm,c               0010 0110 0001 1001   F111 CCCC CC11 1110 (+ Limm)
// SUB3<.cc><.f> 0,limm,c          0010 0110 1101 1001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_SUB3_do_op);  // SUB3
					}
					case 0x1a:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MPY<.f> a,b,c                   0010 0bbb 0001 1010   FBBB CCCC CCAA AAAA
// MPY<.f> a,b,u6                  0010 0bbb 0101 1010   FBBB uuuu uuAA AAAA
// MPY<.f> b,b,s12                 0010 0bbb 1001 1010   FBBB ssss ssSS SSSS
// MPY<.cc><.f> b,b,c              0010 0bbb 1101 1010   FBBB CCCC CC0Q QQQQ
// MPY<.cc><.f> b,b,u6             0010 0bbb 1101 1010   FBBB uuuu uu1Q QQQQ
// MPY<.f> a,limm,c                0010 0110 0001 1010   F111 CCCC CCAA AAAA (+ Limm)
// MPY<.f> a,b,limm                0010 0bbb 0001 1010   FBBB 1111 10AA AAAA (+ Limm)
// MPY<.cc><.f> b,b,limm           0010 0bbb 1101 1010   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPY<.f> 0,b,c                   0010 0bbb 0001 1010   FBBB CCCC CC11 1110
// MPY<.f> 0,b,u6                  0010 0bbb 0101 1010   FBBB uuuu uu11 1110
// MPY<.cc><.f> 0,limm,c           0010 0110 1101 1010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_MPY_do_op);  // MPY *
					}
					case 0x1b:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MPYH<.f> a,b,c                  0010 0bbb 0001 1011   FBBB CCCC CCAA AAAA
// MPYH<.f> a,b,u6                 0010 0bbb 0101 1011   FBBB uuuu uuAA AAAA
// MPYH<.f> b,b,s12                0010 0bbb 1001 1011   FBBB ssss ssSS SSSS
// MPYH<.cc><.f> b,b,c             0010 0bbb 1101 1011   FBBB CCCC CC0Q QQQQ
// MPYH<.cc><.f> b,b,u6            0010 0bbb 1101 1011   FBBB uuuu uu1Q QQQQ
// MPYH<.f> a,limm,c               0010 0110 0001 1011   F111 CCCC CCAA AAAA (+ Limm)
// MPYH<.f> a,b,limm               0010 0bbb 0001 1010   FBBB 1111 10AA AAAA (+ Limm)
// MPYH<.cc><.f> b,b,limm          0010 0bbb 1101 1011   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPYH<.f> 0,b,c                  0010 0bbb 0001 1011   FBBB CCCC CC11 1110
// MPYH<.f> 0,b,u6                 0010 0bbb 0101 1011   FBBB uuuu uu11 1110
// MPYH<.cc><.f> 0,limm,c          0010 0110 1101 1011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_MPYH_do_op);  // MPYH *
					}
					case 0x1c:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MPYHU<.f> a,b,c                 0010 0bbb 0001 1100   FBBB CCCC CCAA AAAA
// MPYHU<.f> a,b,u6                0010 0bbb 0101 1100   FBBB uuuu uuAA AAAA
// MPYHU<.f> b,b,s12               0010 0bbb 1001 1100   FBBB ssss ssSS SSSS
// MPYHU<.cc><.f> b,b,c            0010 0bbb 1101 1100   FBBB CCCC CC0Q QQQQ
// MPYHU<.cc><.f> b,b,u6           0010 0bbb 1101 1100   FBBB uuuu uu1Q QQQQ
// MPYHU<.f> a,limm,c              0010 0110 0001 1100   F111 CCCC CCAA AAAA (+ Limm)
// MPYHU<.f> a,b,limm              0010 0bbb 0001 1100   FBBB 1111 10AA AAAA (+ Limm)
// MPYHU<.cc><.f> b,b,limm         0010 0bbb 1101 1100   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPYHU<.f> 0,b,c                 0010 0bbb 0001 1100   FBBB CCCC CC11 1110
// MPYHU<.f> 0,b,u6                0010 0bbb 0101 1100   FBBB uuuu uu11 1110
// MPYHU<.cc><.f> 0,limm,c         0010 0110 1101 1100   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_MPYHU_do_op);  // MPYHU *
					}
					case 0x1d:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MPYU<.f> a,b,c                  0010 0bbb 0001 1101   FBBB CCCC CCAA AAAA
// MPYU<.f> a,b,u6                 0010 0bbb 0101 1101   FBBB uuuu uuAA AAAA
// MPYU<.f> b,b,s12                0010 0bbb 1001 1101   FBBB ssss ssSS SSSS
// MPYU<.cc><.f> b,b,c             0010 0bbb 1101 1101   FBBB CCCC CC0Q QQQQ
// MPYU<.cc><.f> b,b,u6            0010 0bbb 1101 1101   FBBB uuuu uu1Q QQQQ
// MPYU<.f> a,limm,c               0010 0110 0001 1101   F111 CCCC CCAA AAAA (+ Limm)
// MPYU<.f> a,b,limm               0010 0bbb 0001 1101   FBBB 1111 10AA AAAA (+ Limm)
// MPYU<.cc><.f> b,b,limm          0010 0bbb 1101 1101   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPYU<.f> 0,b,c                  0010 0bbb 0001 1101   FBBB CCCC CC11 1110
// MPYU<.f> 0,b,u6                 0010 0bbb 0101 1101   FBBB uuuu uu11 1110
// MPYU<.cc><.f> 0,limm,c          0010 0110 1101 1101   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_MPYU_do_op);  // MPYU *
					}
					case 0x20:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// Jcc [c]                         0010 0RRR 1110 0000   0RRR CCCC CC0Q QQQQ
// Jcc limm                        0010 0RRR 1110 0000   0RRR 1111 100Q QQQQ (+ Limm)
// Jcc u6                          0010 0RRR 1110 0000   0RRR uuuu uu1Q QQQQ
// Jcc.F [ilink1]                  0010 0RRR 1110 0000   1RRR 0111 010Q QQQQ
// Jcc.F [ilink2]                  0010 0RRR 1110 0000   1RRR 0111 100Q QQQQ
//                                 IIII I      SS SSSS
// J [c]                           0010 0RRR 0010 0000   0RRR CCCC CCRR RRRR
// J.F [ilink1]                    0010 0RRR 0010 0000   1RRR 0111 01RR RRRR
// J.F [ilink2]                    0010 0RRR 0010 0000   1RRR 0111 10RR RRRR
// J limm                          0010 0RRR 0010 0000   0RRR 1111 10RR RRRR (+ Limm)
// J u6                            0010 0RRR 0110 0000   0RRR uuuu uuRR RRRR
// J s12                           0010 0RRR 1010 0000   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_J(op, false, false);  // Jcc
					}
					case 0x21:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// Jcc.D u6                        0010 0RRR 1110 0001   0RRR uuuu uu1Q QQQQ
// Jcc.D [c]                       0010 0RRR 1110 0001   0RRR CCCC CC0Q QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// J.D [c]                         0010 0RRR 0010 0001   0RRR CCCC CCRR RRRR
// J.D u6                          0010 0RRR 0110 0001   0RRR uuuu uuRR RRRR
// J.D s12                         0010 0RRR 1010 0001   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

						return handleop32_J(op, true, false);  // Jcc.D
					}
					case 0x22:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// JLcc [c]                        0010 0RRR 1110 0010   0RRR CCCC CC0Q QQQQ
// JLcc limm                       0010 0RRR 1110 0010   0RRR 1111 100Q QQQQ (+ Limm)
// JLcc u6                         0010 0RRR 1110 0010   0RRR uuuu uu1Q QQQQ
// JL [c]                          0010 0RRR 0010 0010   0RRR CCCC CCRR RRRR
// JL limm                         0010 0RRR 0010 0010   0RRR 1111 10RR RRRR (+ Limm)
// JL u6                           0010 0RRR 0110 0010   0RRR uuuu uuRR RRRR
// JL s12                          0010 0RRR 1010 0010   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_J(op, false, true);  // JLcc
					}
					case 0x23:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// JLcc.D u6                       0010 0RRR 1110 0011   0RRR uuuu uu1Q QQQQ
// JLcc.D [c]                      0010 0RRR 1110 0011   0RRR CCCC CC0Q QQQQ
// JL.D [c]                        0010 0RRR 0010 0011   0RRR CCCC CCRR RRRR
// JL.D u6                         0010 0RRR 0110 0011   0RRR uuuu uuRR RRRR
// JL.D s12                        0010 0RRR 1010 0011   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_J(op, true, true);  // JLcc.D
					}
					case 0x28:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LP<cc> u7                       0010 0RRR 1110 1000   0RRR uuuu uu1Q QQQQ
// LP s13                          0010 0RRR 1010 1000   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_LP(op);  // LPcc
					}
					case 0x29:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// FLAG<.cc> c                     0010 0rrr 1110 1001   0RRR CCCC CC0Q QQQQ
// FLAG<.cc> u6                    0010 0rrr 1110 1001   0RRR uuuu uu1Q QQQQ
// FLAG<.cc> limm                  0010 0rrr 1110 1001   0RRR 1111 100Q QQQQ (+ Limm)
// FLAG s12                        0010 0rrr 1010 1001   0RRR ssss ssSS SSSS
// FLAG c (unknown format)         0010 0000 0010 1001   0000 0000 0100 0000 (Leapster BIOS encodes like this? unless it's customized and isn't FLAG?)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_FLAG(op);  // FLAG
					}
					case 0x2a:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LR b,[c]                        0010 0bbb 0010 1010   0BBB CCCC CCRR RRRR
// LR b,[limm]                     0010 0bbb 0010 1010   0BBB 1111 10RR RRRR (+ Limm)
// LR b,[u6]                       0010 0bbb 0110 1010   0BBB uuuu uu00 0000
// LR b,[s12]                      0010 0bbb 1010 1010   0BBB ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_LR(op);  // LR
					}
					case 0x2b:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SR b,[c]                        0010 0bbb 0010 1011   0BBB CCCC CCRR RRRR
// SR b,[limm]                     0010 0bbb 0010 1011   0BBB 1111 10RR RRRR (+ Limm)
// SR b,[u6]                       0010 0bbb 0110 1011   0BBB uuuu uu00 0000
// SR b,[s12]                      0010 0bbb 1010 1011   0BBB ssss ssSS SSSS
// SR limm,[c]                     0010 0110 0010 1011   0111 CCCC CCRR RRRR (+ Limm)
// SR limm,[u6]                    0010 0110 0110 1011   0111 uuuu uu00 0000
// SR limm,[s12]                   0010 0110 1010 1011   0111 ssss ssSS SSSS (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_SR(op);  // SR
					}
					case 0x2f: // Sub Opcode
					{
						uint8_t subinstr2 = op & 0x0000003f;
						switch (subinstr2 & 0x3f)
						{
							case 0x00:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ASL<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0000
// ASL<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0000
// ASL<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0000 (+ Limm)
//
// ASL<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0000
// ASL<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0000
// ASL<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_ASL_single_do_op); // ASL
							}
							case 0x01:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ASR<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0001
// ASR<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0001
// ASR<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0001 (+ Limm)
//
// ASR<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0001
// ASR<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0001
// ASR<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_ASR_single_do_op);  // ASR
							}
							case 0x02:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// LSR<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0010
// LSR<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0010
// LSR<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0010 (+ Limm)
//
// LSR<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0010
// LSR<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0010
// LSR<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0010 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_LSR_single_do_op);  // LSR
							}
							case 0x03:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ROR<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0011
// ROR<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0011
// ROR<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0011 (+ Limm)
//
// ROR<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0011
// ROR<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0011
// ROR<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_ROR_do_op);  // ROR
							}
							case 0x04:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// RRC<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0100
// RRC<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0100
// RRC<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0100 (+ Limm)
//
// RRC<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0100
// RRC<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0100
// RRC<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_RRC_do_op);  // RRC
							}
							case 0x05:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// SEXB<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 0101
// SEXB<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 0101
// SEXB<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 0101 (+ Limm)
//
// SEXB<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 0101
// SEXB<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 0101
// SEXB<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 0101 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_SEXB_do_op);  // SEXB
							}
							case 0x06:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// SEXW<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 0110
// SEXW<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 0110
// SEXW<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 0110 (+ Limm)
//
// SEXW<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 0110
// SEXW<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 0110
// SEXW<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 0110 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_SEXW_do_op);  // SEXW
							}
							case 0x07:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// EXTB<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 0111
// EXTB<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 0111
// EXTB<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 0111 (+ Limm)
//
// EXTB<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 0111
// EXTB<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 0111
// EXTB<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 0111 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_EXTB_do_op);  // EXTB
							}
							case 0x08:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// EXTW<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 1000
// EXTW<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 1000
// EXTW<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 1000 (+ Limm)
//
// EXTW<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 1000
// EXTW<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 1000
// EXTW<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 1000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_EXTW_do_op);  // EXTW
							}
							case 0x09:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ABS<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 1001
// ABS<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 1001
// ABS<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 1001 (+ Limm)
//
// ABS<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 1001
// ABS<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 1001
// ABS<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 1001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_ABS_do_op);  // ABS
							}
							case 0x0a:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// NOT<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 1010
// NOT<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 1010
// NOT<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 1010 (+ Limm)
//
// NOT<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 1010
// NOT<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 1010
// NOT<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 1010 (+ Limm)
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_NOT_do_op);  // NOT
							}
							case 0x0b:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// RLC<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 1011
// RLC<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 1011
// RLC<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 1011 (+ Limm)
//
// RLC<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 1011
// RLC<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 1011
// RLC<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 1011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_RLC_do_op);  // RLC
							}
							case 0x0c:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// EX<.di> b,[c]                   0010 0bbb 0010 1111   DBBB CCCC CC00 1100
// EX<.di> b,[u6]                  0010 0bbb 0110 1111   DBBB uuuu uu00 1100
// EX<.di> b,[limm]                0010 0bbb 0010 1111   DBBB 1111 1000 1100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_EX(op);  // EX
							}
							case 0x3f: // ZOPs (Zero Operand Opcodes)
							{
								uint8_t subinstr3 = (op & 0x07000000) >> 24;
								subinstr3 |= ((op & 0x00007000) >> 12) << 3;

								switch (subinstr3 & 0x3f)
								{
									case 0x01:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I$$$   SS SSSS    $$$        ss ssss
// SLEEP <u6>                      0010 0001 0110 1111   0000 uuuu uu11 1111
// SLEEP c                         0010 0001 0010 1111   0000 CCCC CC11 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_SLEEP(op);  // SLEEP
									}
									case 0x02:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I$$$   SS SSSS    $$$        ss ssss
// SWI/TRAP0                       0010 0010 0110 1111   0000 0000 0011 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_SWI(op);  // SWI / TRAP0
									}
									case 0x03:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I$$$   SS SSSS    $$$        ss ssss
// SYNC                            0010 0011 0110 1111   0000 0000 0011 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_SYNC(op);  // SYNC
									}
									case 0x04:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I$$$   SS SSSS    $$$        ss ssss
// RTIE                            0010 0100 0110 1111   0000 0000 0011 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_RTIE(op);  // RTIE
									}
									case 0x05:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I$$$   SS SSSS    $$$        ss ssss
// BRK                             0010 0101 0110 1111   0000 0000 0011 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										return handleop32_BRK(op);  // BRK
									}

									default:
									{
										// 0x00, 0x06-0x3f
										return arcompact_handle_illegal(instruction, subinstr, subinstr2, subinstr3, op);  // illegal
									}
								}
							}
							default:
							{
								// 0x0d - 0x3e
								return arcompact_handle_illegal(instruction, subinstr, subinstr2, op);  // illegal
							}
						}
					}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LD<zz><.x><.aa><.di> a,[b,c]    0010 0bbb aa11 0ZZX   DBBB CCCC CCAA AAAA
// LD<zz><.x><.aa><.di> 0,[b,c]    0010 0bbb aa11 0ZZX   DBBB CCCC CC11 1110
// PREFETCH<.aa> [b,c]             0010 0bbb aa11 0000   0BBB CCCC CC11 1110    (prefetch is an alias)
//
// LD<zz><.x><.aa><.di> a,[b,limm] 0010 0bbb aa11 0ZZX   DBBB 1111 10AA AAAA (+ Limm)
// LD<zz><.x><.aa><.di> 0,[b,limm] 0010 0bbb aa11 0ZZX   DBBB 1111 1011 1110 (+ Limm)
// PREFETCH<.aa> [b,limm]          0010 0bbb aa11 0000   0BBB 1111 1011 1110 (+ Limm) (prefetch is an alias)
//
// LD<zz><.x><.di> a,[limm,c]      0010 0110 RR11 0ZZX   D111 CCCC CCAA AAAA (+ Limm)
// LD<zz><.x><.di> 0,[limm,c]      0010 0110 RR11 0ZZX   D111 CCCC CC11 1110 (+ Limm)
// PREFETCH [limm,c]               0010 0110 RR11 0000   0111 CCCC CC11 1110 (+ Limm) (prefetch is an alias)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
					{
						return handleop32_LDrr(op, (op >> 17) & 3, (op >> 16) & 1);  // LD r-r
					}
					default:
					{
						return arcompact_handle_illegal(instruction, subinstr, op);  // illegal
					}
				}
			}
			case 0x05: // op a,b,c (05 ARC ext)
			{
				uint8_t subinstr = (op & 0x003f0000) >> 16;

				switch (subinstr)
				{
					case 0x00:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ASL<.f> a,b,c                   0010 1bbb 0000 0000   FBBB CCCC CCAA AAAA
// ASL<.f> a,b,u6                  0010 1bbb 0100 0000   FBBB uuuu uuAA AAAA
// ASL<.f> b,b,s12                 0010 1bbb 1000 0000   FBBB ssss ssSS SSSS
// ASL<.cc><.f> b,b,c              0010 1bbb 1100 0000   FBBB CCCC CC0Q QQQQ
// ASL<.cc><.f> b,b,u6             0010 1bbb 1100 0000   FBBB uuuu uu1Q QQQQ
// ASL<.f> a,limm,c                0010 1110 0000 0000   F111 CCCC CCAA AAAA (+ Limm)
// ASL<.f> a,b,limm                0010 1bbb 0000 0000   FBBB 1111 10AA AAAA (+ Limm)
// ASL<.cc><.f> b,b,limm           0010 1bbb 1100 0000   FBBB 1111 100Q QQQQ (+ Limm)
//
// ASL<.f> 0,b,c                   0010 1bbb 0000 0000   FBBB CCCC CC11 1110
// ASL<.f> 0,b,u6                  0010 1bbb 0100 0000   FBBB uuuu uu11 1110
// ASL<.cc><.f> 0,limm,c           0010 1110 1100 0000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ASL_multiple_do_op);  // ASL
					}
					case 0x01:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LSR<.f> a,b,c                   0010 1bbb 0000 0001   FBBB CCCC CCAA AAAA
// LSR<.f> a,b,u6                  0010 1bbb 0100 0001   FBBB uuuu uuAA AAAA
// LSR<.f> b,b,s12                 0010 1bbb 1000 0001   FBBB ssss ssSS SSSS
// LSR<.cc><.f> b,b,c              0010 1bbb 1100 0001   FBBB CCCC CC0Q QQQQ
// LSR<.cc><.f> b,b,u6             0010 1bbb 1100 0001   FBBB uuuu uu1Q QQQQ
// LSR<.f> a,limm,c                0010 1110 0000 0001   F111 CCCC CCAA AAAA (+ Limm)
// LSR<.f> a,b,limm                0010 1bbb 0000 0001   FBBB 1111 10AA AAAA (+ Limm)
// LSR<.cc><.f> b,b,limm           0010 1bbb 1100 0001   FBBB 1111 100Q QQQQ (+ Limm)
//
// LSR<.f> 0,b,c                   0010 1bbb 0000 0001   FBBB CCCC CC11 1110
// LSR<.f> 0,b,u6                  0010 1bbb 0100 0001   FBBB uuuu uu11 1110
// LSR<.cc><.f> 0,limm,c           0010 1110 1100 0001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_LSR_multiple_do_op);  // LSR
					}
					case 0x02:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ASR<.f> a,b,c                   0010 1bbb 0000 0010   FBBB CCCC CCAA AAAA
// ASR<.f> a,b,u6                  0010 1bbb 0100 0010   FBBB uuuu uuAA AAAA
// ASR<.f> b,b,s12                 0010 1bbb 1000 0010   FBBB ssss ssSS SSSS
// ASR<.cc><.f> b,b,c              0010 1bbb 1100 0010   FBBB CCCC CC0Q QQQQ
// ASR<.cc><.f> b,b,u6             0010 1bbb 1100 0010   FBBB uuuu uu1Q QQQQ
// ASR<.f> a,limm,c                0010 1110 0000 0010   F111 CCCC CCAA AAAA (+ Limm)
// ASR<.f> a,b,limm                0010 1bbb 0000 0010   FBBB 1111 10AA AAAA (+ Limm)
// ASR<.cc><.f> b,b,limm           0010 1bbb 1100 0010   FBBB 1111 100Q QQQQ (+ Limm)
//
// ASR<.f> 0,b,c                   0010 1bbb 0000 0010   FBBB CCCC CC11 1110
// ASR<.f> 0,b,u6                  0010 1bbb 0100 0010   FBBB uuuu uu11 1110
// ASR<.cc><.f> 0,limm,c           0010 1110 1100 0010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ASR_multiple_do_op);  // ASR
					}
					case 0x03:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ROR<.f> a,b,c                   0010 1bbb 0000 0011   FBBB CCCC CCAA AAAA
// ROR<.f> a,b,u6                  0010 1bbb 0100 0011   FBBB uuuu uuAA AAAA
// ROR<.f> b,b,s12                 0010 1bbb 1000 0011   FBBB ssss ssSS SSSS
// ROR<.cc><.f> b,b,c              0010 1bbb 1100 0011   FBBB CCCC CC0Q QQQQ
// ROR<.cc><.f> b,b,u6             0010 1bbb 1100 0011   FBBB uuuu uu1Q QQQQ
// ROR<.f> a,limm,c                0010 1110 0000 0011   F111 CCCC CCAA AAAA (+ Limm)
// ROR<.f> a,b,limm                0010 1bbb 0000 0011   FBBB 1111 10AA AAAA (+ Limm)
// ROR<.cc><.f> b,b,limm           0010 1bbb 1100 0011   FBBB 1111 100Q QQQQ (+ Limm)
//
// ROR<.f> 0,b,c                   0010 1bbb 0000 0011   FBBB CCCC CC11 1110
// ROR<.f> 0,b,u6                  0010 1bbb 0100 0011   FBBB uuuu uu11 1110
// ROR<.cc><.f> 0,limm,c           0010 1110 1100 0011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ROR_multiple_do_op);  // ROR
					}
					case 0x04:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MUL64 <0,>b,c                   0010 1bbb 0000 0100   0BBB CCCC CC11 1110
// MUL64 <0,>b,u6                  0010 1bbb 0100 0100   0BBB uuuu uu11 1110
// MUL64 <0,>b,s12                 0010 1bbb 1000 0100   0BBB ssss ssSS SSSS
// MUL64 <0,>limm,c                0010 1110 0000 0100   0111 CCCC CC11 1110 (+ Limm)
//
// MUL64<.cc> <0,>b,c              0010 1bbb 1100 0100   0BBB CCCC CC0Q QQQQ
// MUL64<.cc> <0,>b,u6             0010 1bbb 1100 0100   0BBB uuuu uu1Q QQQQ
// MUL64<.cc> <0,>limm,c           0010 1110 1100 0100   0111 CCCC CC0Q QQQQ (+ Limm)
// MUL64<.cc> <0,>b,limm           0010 1bbb 1100 0100   0BBB 1111 100Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general_MULx64(op, handleop32_MUL64_do_op);  // MUL64
					}
					case 0x05:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MULU64 <0,>b,c                  0010 1bbb 0000 0101   0BBB CCCC CC11 1110
// MULU64 <0,>b,u6                 0010 1bbb 0100 0101   0BBB uuuu uu11 1110
// MULU64 <0,>b,s12                0010 1bbb 1000 0101   0BBB ssss ssSS SSSS
// MULU64 <0,>limm,c               0010 1110 0000 0101   0111 CCCC CC11 1110 (+ Limm)
//
// MULU64<.cc> <0,>b,c             0010 1bbb 1100 0101   0BBB CCCC CC0Q QQQQ
// MULU64<.cc> <0,>b,u6            0010 1bbb 1100 0101   0BBB uuuu uu1Q QQQQ
// MULU64<.cc> <0,>limm,c          0010 1110 1100 0101   0111 CCCC CC0Q QQQQ (+ Limm)
// MULU64<.cc> <0,>b,limm          0010 1bbb 1100 0101   0BBB 1111 100Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general_MULx64(op, handleop32_MULU64_do_op);  // MULU64
					}
					case 0x06:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Extended Arithmetic (optional on ARCtangent-A5, ARC600, built in on ARC700)
// ADDS - Add and Saturate
//
// ADDS<.f> a,b,c                  0010 1bbb 0000 0110   FBBB CCCC CCAA AAAA
// ADDS<.f> a,b,u6                 0010 1bbb 0100 0110   FBBB uuuu uuAA AAAA
// ADDS<.f> b,b,s12                0010 1bbb 1000 0110   FBBB ssss ssSS SSSS
// ADDS<.cc><.f> b,b,c             0010 1bbb 1100 0110   FBBB CCCC CC0Q QQQQ
// ADDS<.cc><.f> b,b,u6            0010 1bbb 1100 0110   FBBB uuuu uu1Q QQQQ
// ADDS<.f> a,limm,c               0010 1110 0000 0110   F111 CCCC CCAA AAAA (+ Limm)
// ADDS<.f> a,b,limm               0010 1bbb 0000 0110   FBBB 1111 10AA AAAA (+ Limm)
// ADDS<.cc><.f> b,b,limm          0010 1bbb 1100 0110   FBBB 1111 10QQ QQQQ (+ Limm)
//
// ADDS<.f> 0,b,c                  0010 1bbb 0000 0110   FBBB CCCC CC11 1110
// ADDS<.f> 0,b,u6                 0010 1bbb 0100 0110   FBBB uuuu uu11 1110
// ADDS<.f> 0,b,limm               0010 1bbb 0000 0110   FBBB 1111 1011 1110 (+ Limm)
// ADDS<.cc><.f> 0,limm,c          0010 1110 1100 0110   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ADDS_do_op); // ADDS
					}
					case 0x07:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Extended Arithmetic (optional on ARCtangent-A5, ARC600, built in on ARC700)
// SUBS - Subtract and Saturate
//
// SUBS<.f> a,b,c                  0010 1bbb 0000 0111   FBBB CCCC CCAA AAAA
// SUBS<.f> a,b,u6                 0010 1bbb 0100 0111   FBBB uuuu uuAA AAAA
// SUBS<.f> b,b,s12                0010 1bbb 1000 0111   FBBB ssss ssSS SSSS
// SUBS<.cc><.f> b,b,c             0010 1bbb 1100 0111   FBBB CCCC CC0Q QQQQ
// SUBS<.cc><.f> b,b,u6            0010 1bbb 1100 0111   FBBB uuuu uu1Q QQQQ
// SUBS<.f> a,limm,c               0010 1110 0000 0111   F111 CCCC CCAA AAAA (+ Limm)
// SUBS<.f> a,b,limm               0010 1bbb 0000 0111   FBBB 1111 10AA AAAA (+ Limm)
// SUBS<.cc><.f> b,b,limm          0010 1bbb 1100 0111   FBBB 1111 10QQ QQQQ (+ Limm)
//
// SUBS<.f> 0,b,c                  0010 1bbb 0000 0111   FBBB CCCC CC11 1110
// SUBS<.f> 0,b,u6                 0010 1bbb 0100 0111   FBBB uuuu uu11 1110
// SUBS<.f> 0,b,limm               0010 1bbb 0000 0111   FBBB 1111 1011 1110 (+ Limm)
// SUBS<.cc><.f> 0,limm,c          0010 1110 1100 0111   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_SUBS_do_op); // SUBS
					}
					case 0x08:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Extended Arithmetic (optional on ARCtangent-A5, ARC600, built in on ARC700)
// DIVAW - Division Assist
//
// DIVAW a,b,c                     0010 1bbb 0000 1000   0BBB CCCC CCAA AAAA
// DIVAW a,b,u6                    0010 1bbb 0100 1000   0BBB uuuu uuAA AAAA
// DIVAW b,b,s12                   0010 1bbb 1000 1000   0BBB ssss ssSS SSSS
// DIVAW<.cc> b,b,c                0010 1bbb 1100 1000   0BBB CCCC CC0Q QQQQ
// DIVAW<.cc> b,b,u6               0010 1bbb 1100 1000   0BBB uuuu uu1Q QQQQ
// DIVAW a,limm,c                  0010 1110 0000 1000   0111 CCCC CCAA AAAA (+ Limm)
// DIVAW a,b,limm                  0010 1bbb 0000 1000   0BBB 1111 10AA AAAA (+ Limm)
// DIVAW<.cc> b,b,limm             0010 1bbb 1100 1000   0BBB 1111 10QQ QQQQ (+ Limm)
//
// DIVAW 0,b,c                     0010 1bbb 0000 1000   0BBB CCCC CC11 1110
// DIVAW 0,b,u6                    0010 1bbb 0100 1000   0BBB uuuu uu11 1110
// DIVAW<.cc> 0,limm,c             0010 1110 1100 1000   0111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_DIVAW_do_op); // DIVAW
					}
					case 0x0a:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Extended Arithmetic (optional ARCtangent-A5, ARC600, built in on ARC700)
// ASLS - Arithmetic Shift Left and Saturate (with negative shift value support)
//
//                                 IIII I      SS SSSS
// ASLS<.f> a,b,c                  0010 1bbb 0000 1010   FBBB CCCC CCAA AAAA
// ASLS<.f> a,b,u6                 0010 1bbb 0100 1010   FBBB uuuu uuAA AAAA
// ASLS<.f> b,b,s12                0010 1bbb 1000 1010   FBBB ssss ssSS SSSS
// ASLS<.cc><.f> b,b,c             0010 1bbb 1100 1010   FBBB CCCC CC0Q QQQQ
// ASLS<.cc><.f> b,b,u6            0010 1bbb 1100 1010   FBBB uuuu uu1Q QQQQ
// ASLS<.f> a,limm,c               0010 1110 0000 1010   F111 CCCC CCAA AAAA (+ Limm)
// ASLS<.f> a,b,limm               0010 1bbb 0000 1010   FBBB 1111 10AA AAAA (+ Limm)
// ASLS<.cc><.f> b,b,limm          0010 1bbb 1100 1010   FBBB 1111 10QQ QQQQ (+ Limm)
// ASLS<.f> 0,b,c                  0010 1bbb 0000 1010   FBBB CCCC CC11 1110
// ASLS<.f> 0,b,u6                 0010 1bbb 0100 1010   FBBB uuuu uu11 1110
// ASLS<.cc><.f> 0,limm,c          0010 1110 1100 1010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ASLS_do_op); // ASLS
					}
					case 0x0b:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Extended Arithmetic (optional ARCtangent-A5, ARC600, built in on ARC700)
// ASRS - Arithmetic Shift Right and Saturate (with negative shift value support)
//
// ASRS<.f> a,b,c                  0010 1bbb 0000 1011   FBBB CCCC CCAA AAAA
// ASRS<.f> a,b,u6                 0010 1bbb 0100 1011   FBBB uuuu uuAA AAAA
// ASRS<.f> b,b,s12                0010 1bbb 1000 1011   FBBB ssss ssSS SSSS
// ASRS<.cc><.f> b,b,c             0010 1bbb 1100 1011   FBBB CCCC CC0Q QQQQ
// ASRS<.cc><.f> b,b,u6            0010 1bbb 1100 1011   FBBB uuuu uu1Q QQQQ
// ASRS<.f> a,limm,c               0010 1110 0000 1011   F111 CCCC CCAA AAAA (+ Limm)
// ASRS<.f> a,b,limm               0010 1bbb 0000 1011   FBBB 1111 10AA AAAA (+ Limm)
// ASRS<.cc><.f> b,b,limm          0010 1bbb 1100 1011   FBBB 1111 10QQ QQQQ (+ Limm)
//
// ASRS<.f> 0,b,c                  0010 1bbb 0000 1011   FBBB CCCC CC11 1110
// ASRS<.f> 0,b,u6                 0010 1bbb 0100 1011   FBBB uuuu uu11 1110
// ASRS<.cc><.f> 0,limm,c          0010 1110 1100 1011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ASRS_do_op); // ASRS
					}
					case 0x0c:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Unknown Extension Op
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_UNKNOWN_05_0c_do_op);
					}
					case 0x10:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Unknown Extension Op
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_UNKNOWN_05_10_do_op);
					}
					case 0x14:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Unknown Extension Op
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_UNKNOWN_05_14_do_op);
					}
					case 0x28:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Extended Arithmetic (optional ARCtangent-A5, ARC600, built in on ARC700)
// ADDSDW - Dual 16-bit Add and Saturate
//
// ADDSDW<.f> a,b,c                0010 1bbb 0010 1000   FBBB CCCC CCAA AAAA
// ADDSDW<.f> a,b,u6               0010 1bbb 0110 1000   FBBB uuuu uuAA AAAA
// ADDSDW<.f> b,b,s12              0010 1bbb 1010 1000   FBBB ssss ssSS SSSS
// ADDSDW<.cc><.f> b,b,c           0010 1bbb 1110 1000   FBBB CCCC CC0Q QQQQ
// ADDSDW<.cc><.f> b,b,u6          0010 1bbb 1110 1000   FBBB uuuu uu1Q QQQQ
// ADDSDW<.f> a,limm,c             0010 1110 0010 1000   F111 CCCC CCAA AAAA (+ Limm)
// ADDSDW<.f> a,b,limm             0010 1bbb 0010 1000   FBBB 1111 10AA AAAA (+ Limm)
// ADDSDW<.cc><.f> b,b,limm        0010 1bbb 1110 1000   FBBB 1111 10QQ QQQQ (+ Limm)
//
// ADDSDW<.f> 0,b,c                0010 1bbb 0010 1000   FBBB CCCC CC11 1110
// ADDSDW<.f> 0,b,u6               0010 1bbb 0110 1000   FBBB uuuu uu11 1110
// ADDSDW<.cc><.f> 0,limm,c        0010 1110 1110 1000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_ADDSDW_do_op); // ADDSDW
					}
					case 0x29:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Extended Arithmetic (optional ARCtangent-A5, ARC600, built in on ARC700)
// SUBSDW - Dual 16-bit Subtract and Saturate
//
// SUBSDW<.f> a,b,c                0010 1bbb 0010 1001   FBBB CCCC CCAA AAAA
// SUBSDW<.f> a,b,u6               0010 1bbb 0110 1001   FBBB uuuu uuAA AAAA
// SUBSDW<.f> b,b,s12              0010 1bbb 1010 1001   FBBB ssss ssSS SSSS
// SUBSDW<.cc><.f> b,b,c           0010 1bbb 1110 1001   FBBB CCCC CC0Q QQQQ
// SUBSDW<.cc><.f> b,b,u6          0010 1bbb 1110 1001   FBBB uuuu uu1Q QQQQ
// SUBSDW<.f> a,limm,c             0010 1110 0010 1001   F111 CCCC CCAA AAAA (+ Limm)
// SUBSDW<.f> a,b,limm             0010 1bbb 0010 1001   FBBB 1111 10AA AAAA (+ Limm)
// SUBSDW<.cc><.f> b,b,limm        0010 1bbb 1110 1001   FBBB 1111 10QQ QQQQ (+ Limm)
//
// SUBSDW<.f> 0,b,c                0010 1bbb 0010 1001   FBBB CCCC CC11 1110
// SUBSDW<.f> 0,b,u6               0010 1bbb 0110 1001   FBBB uuuu uu11 1110
// SUBSDW<.cc><.f> 0,limm,c        0010 1110 1110 1001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						return handleop32_general(op, handleop32_SUBSDW_do_op); // SUBSDW
					}
					case 0x2f: // SOPs
					{
						uint8_t subinstr2 = op & 0x0000003f;
						switch (subinstr2)
						{
							case 0x00:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SWAP<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0000
// SWAP<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0000
// SWAP<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0000 (+ Limm)
//
// SWAP<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0000
// SWAP<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0000
// SWAP<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_SWAP_do_op);  // SWAP
							}
							case 0x01:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NORM<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0001
// NORM<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0001
// NORM<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0001 (+ Limm)
//
// NORM<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0001
// NORM<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0001
// NORM<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_NORM_do_op);  // NORM
							}
							case 0x02:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SAT16<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0010
// SAT16<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0010
// SAT16<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0010 (+ Limm)
//
// SAT16<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0010
// SAT16<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0010
// SAT16<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0010 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_SAT16_do_op);  // SAT16
							}
							case 0x03:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RND16<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0011
// RND16<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0011
// RND16<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0011 (+ Limm)
//
// RND16<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0011
// RND16<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0011
// RND16<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_RND16_do_op); // RND16
							}
							case 0x04:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ABSSW<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0100
// ABSSW<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0100
// ABSSW<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0100 (+ Limm)
//
// ABSSW<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0100
// ABSSW<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0100
// ABSSW<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_ABSSW_do_op); // ABSSW
							}
							case 0x05:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ABSS<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0101
// ABSS<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0101
// ABSS<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0101 (+ Limm)
//
// ABSS<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0101
// ABSS<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0101
// ABSS<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0101 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_ABSS_do_op); // ABSS
							}
							case 0x06:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NEGSW<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0110
// NEGSW<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0110
// NEGSW<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0110 (+ Limm)
//
// NEGSW<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0110
// NEGSW<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0110
// NEGSW<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0110 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_NEGSW_do_op); // NEGSW
							}
							case 0x07:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NEGS<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0111
// NEGS<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0111
// NEGS<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0111 (+ Limm)
//
// NEGS<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0111
// NEGS<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0111
// NEGS<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0111 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_NEGS_do_op); // NEGS
							}
							case 0x08:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NORMW<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 1000
// NORMW<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 1000
// NORMW<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 1000 (+ Limm)
//
// NORMW<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 1000
// NORMW<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 1000
// NORMW<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 1000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								return handleop32_general_SOP_group(op, handleop32_NORMW_do_op);  // NORMW
							}
							case 0x3f: // ZOPs (Zero Operand Opcodes)
							{
								uint8_t subinstr3 = (op & 0x07000000) >> 24;
								subinstr3 |= ((op & 0x00007000) >> 12) << 3;

								switch (subinstr3)
								{
									default:
									{
										return arcompact_handle_illegal(instruction, subinstr, subinstr2, subinstr3, op);  // illegal
									}
								}
							}
							default:
							{
								return arcompact_handle_illegal(instruction, subinstr, subinstr2, op);  // illegal
							}
						}
					}
					default:
					{
						return arcompact_handle_illegal(instruction, subinstr, op);  // illegal
					}

				}
			}
			case 0x06: return handleop32_ARC_EXT06(op);    // op a,b,c (06 ARC ext)
			case 0x07: return handleop32_USER_EXT07(op);    // op a,b,c (07 User ext)
			case 0x08: return handleop32_USER_EXT08(op);    // op a,b,c (08 User ext)
			case 0x09: return handleop32_MARKET_EXT09(op);    // op a,b,c (09 Market ext)
			case 0x0a: return handleop32_MARKET_EXT0a(op);    // op a,b,c (0a Market ext)
			case 0x0b: return handleop32_MARKET_EXT0b(op);    // op a,b,c (0b Market ext)
		}
	}
	else
	{
		switch (instruction) // 16-bit instructions
		{
			default: return -1;
			case 0x0c: // Load/Add reg-reg
			{
				uint8_t subinstr = (op & 0x0018) >> 3;
				switch (subinstr)
				{
					default: return -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I       S S
// LD_S a,[b,c]                    0110 0bbb ccc0 0aaa
// #######################################################################################################################
						return handleop_LD_S_a_b_c(op);  // LD_S a,[b,c]
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I       S S
// LDB_S a,[b,c]                   0110 0bbb ccc0 1aaa
// #######################################################################################################################
						return handleop_LDB_S_a_b_c(op);  // LDB_S a,[b,c]
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I       S S
// LDW_S a,[b,c]                   0110 0bbb ccc1 0aaa
// #######################################################################################################################
						return handleop_LDW_S_a_b_c(op);  // LDW_S a,[b,c]
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII I       S S
// ADD_S a,b,c                     0110 0bbb ccc1 1aaa
// #######################################################################################################################
						return handleop_ADD_S_a_b_c(op);  // ADD_S a,b,c
					}
				}
			}
			case 0x0d: // Add/Sub/Shft imm
			{
				uint8_t subinstr = (op & 0x0018) >> 3;
				switch (subinstr)
				{
					default: return -1;
					case 0x00:
					{
// #######################################################################################################################
// ADD_S c,b,u3                    0110 1bbb ccc0 0uuu
// #######################################################################################################################
						return handleop_ADD_S_c_b_u3(op);  // ADD_S c,b,u3
					}
					case 0x01:
					{
// #######################################################################################################################
// SUB_S c,b,u3                    0110 1bbb ccc0 1uuu
// #######################################################################################################################
						return handleop_SUB_S_c_b_u3(op);  // SUB_S c,b,u3
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I       S S
// ASL_S c,b,u3                    0110 1bbb ccc1 0uuu
// #######################################################################################################################
						return handleop_ASL_S_c_b_u3(op);  // ASL_S c,b,u3
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII I       S S
// ASR_S c,b,u3                    0110 1bbb ccc1 1uuu
// #######################################################################################################################
						return handleop_ASR_S_c_b_u3(op);  // ASR_S c,b,u3
					}
				}
			}
			case 0x0e: // Mov/Cmp/Add
			{
				uint8_t subinstr = (op & 0x0018) >> 3;

				switch (subinstr)
				{
					default: return -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I       S S
// ADD_S b,b,h                     0111 0bbb hhh0 0HHH
// ADD_S b,b,limm                  0111 0bbb 1100 0111 (+ Limm)
// #######################################################################################################################
						return handleop_ADD_S_b_b_h_or_limm(op);  // ADD_S b,b,h  or  ADD_S b,b,limm
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I       S S
// MOV_S b,h                       0111 0bbb hhh0 1HHH
// MOV_S b,limm                    0111 0bbb 1100 1111 (+ Limm)
// #######################################################################################################################
						return handleop_MOV_S_b_h_or_limm(op);  // MOV_S b,h  or  MOV_S b,limm
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I       S S
// CMP_S b,h                       0111 0bbb hhh1 0HHH
// CMP_S b,limm                    0111 0bbb 1101 0111 (+ Limm)
// #######################################################################################################################
						return handleop_CMP_S_b_h_or_limm(op);  // CMP_S b,h  or  CMP_S b,limm
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII I       S S
// MOV_S h,b                       0111 0bbb hhh1 1HHH
// #######################################################################################################################
						return handleop_MOV_S_h_b(op);  // MOV_S h,b
					}
				}
			}
			case 0x0f: // op_S b,b,c (single 16-bit ops)
			{
				uint8_t subinstr = op & 0x01f;

				switch (subinstr)
				{
					case 0x00: // SOPs
					{
						uint8_t subinstr2 = (op & 0x00e0) >> 5;

						switch (subinstr2)
						{
							case 0x00:
							{
// #######################################################################################################################
//                                 IIII I    sssS SSSS
// J_S [b]                         0111 1bbb 0000 0000
// #######################################################################################################################
								return handleop_J_S_b(op);  // J_S [b]
							}
							case 0x01:
							{
// #######################################################################################################################
//                                 IIII I    sssS SSSS
// J_S.D [b]                       0111 1bbb 0010 0000
// #######################################################################################################################
								return handleop_J_S_D_b(op);  // J_S.D [b]
							}
							case 0x02:
							{
// #######################################################################################################################
//                                 IIII I    sssS SSSS
// JL_S [b]                        0111 1bbb 0100 0000
// #######################################################################################################################
								return handleop_JL_S_b(op);  // JL_S [b]
							}
							case 0x03:
							{
// #######################################################################################################################
//                                 IIII I    sssS SSSS
// JL_S.D [b]                      0111 1bbb 0110 0000
// #######################################################################################################################
								return handleop_JL_S_D_b(op);  // JL_S.D [b]
							}
							case 0x06:
							{
// #######################################################################################################################
// SUB_S.NE b,b,b                  0111 1bbb 1100 0000
//                                 IIII I    sssS SSSS
// #######################################################################################################################
								return handleop_SUB_S_NE_b_b_b(op);  // SUB_S.NE b,b,b
							}
							case 0x07: // ZOPs
							{
								uint8_t subinstr3 = (op & 0x0700) >> 8;

								switch (subinstr3)
								{
									case 0x00:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// NOP_S                           0111 1000 1110 0000
// #######################################################################################################################
										return handleop_NOP_S(op);  // NOP_S
									}
									case 0x01:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// UNIMP_S                         0111 1001 1110 0000
// #######################################################################################################################
										return handleop_UNIMP_S(op);  // UNIMP_S
									}
									case 0x04:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// JEQ_S [blink]                   0111 1100 1110 0000
// #######################################################################################################################
										return handleop_JEQ_S_blink(op);  // JEQ_S [BLINK]
									}
									case 0x05:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// JNE_S [blink]                   0111 1101 1110 0000
// #######################################################################################################################
										return handleop_JNE_S_blink(op);  // JNE_S [BLINK]
									}
									case 0x06:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// J_S [blink]                     0111 1110 1110 0000
// #######################################################################################################################
										return handleop_J_S_blink(op);  // J_S [BLINK]
									}
									case 0x07:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// J_S.D [blink]                   0111 1111 1110 0000
// #######################################################################################################################
										return handleop_J_S_D_blink(op);  // J_S.D [BLINK]
									}

									default: // 0x02, 0x03
									{
										 return arcompact_handle_illegal(instruction,subinstr, subinstr2, subinstr3, op);
									}
								}
							}
							default: // 0x04, 0x05
							{
								 return arcompact_handle_illegal(instruction,subinstr, subinstr2, op);
							}
						}
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// SUB_S b,b,c                     0111 1bbb ccc0 0010
// #######################################################################################################################
						return handleop_SUB_S_b_b_c(op);  // SUB_S b,b,c
					}
					case 0x04:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// AND_S b,b,c                     0111 1bbb ccc0 0100
// #######################################################################################################################
						return handleop_AND_S_b_b_c(op);  // AND_S b,b,c
					}
					case 0x05:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// OR_S b,b,c                      0111 1bbb ccc0 0101
// #######################################################################################################################
						return handleop_OR_S_b_b_c(op);  // OR_S b,b,c
					}
					case 0x06:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// BIC_S b,b,c                     0111 1bbb ccc0 0110
// #######################################################################################################################
						return handleop_BIC_S_b_b_c(op);  // BIC_S b,b,c
					}
					case 0x07:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// XOR_S b,b,c                     0111 1bbb ccc0 0111
// #######################################################################################################################
						return handleop_XOR_S_b_b_c(op);  // XOR_S b,b,c
					}
					case 0x0b:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// TST_S b,c                       0111 1bbb ccc0 1011
// #######################################################################################################################
						return handleop_TST_S_b_c(op);  // TST_S b,c
					}
					case 0x0c:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// MUL64_S <0,>b,c                 0111 1bbb ccc0 1100
// #######################################################################################################################
						return handleop_MUL64_S_0_b_c(op);  // MUL64_S <0,>b,c
					}
					case 0x0d:
					{

// #######################################################################################################################
//                                 IIII I       S SSSS
// SEXB_S b,c                      0111 1bbb ccc0 1101
// #######################################################################################################################
						return handleop_SEXB_S_b_c(op);  // SEXB_S b,c
					}
					case 0x0e:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// SEXW_S b,c                      0111 1bbb ccc0 1110
// #######################################################################################################################
						return handleop_SEXW_S_b_c(op);  // SEXW_S b,c
					}
					case 0x0f:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// EXTB_S b,c                      0111 1bbb ccc0 1111
// #######################################################################################################################
						return handleop_EXTB_S_b_c(op);  // EXTB_S b,c
					}
					case 0x10:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// EXTW_S b,c                      0111 1bbb ccc1 0000
// #######################################################################################################################
						return handleop_EXTW_S_b_c(op);  // EXTW_S b,c
					}
					case 0x11:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ABS_S b,c                       0111 1bbb ccc1 0001
// #######################################################################################################################
						return handleop_ABS_S_b_c(op);  // ABS_S b,c
					}
					case 0x12:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// NOT_S b,c                       0111 1bbb ccc1 0010
// #######################################################################################################################
						return handleop_NOT_S_b_c(op);  // NOT_S b,c
					}
					case 0x13:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// NEG_S b,c                       0111 1bbb ccc1 0011
// #######################################################################################################################
						return handleop_NEG_S_b_c(op);  // NEG_S b,c
					}
					case 0x14:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ADD1_S b,b,c                    0111 1bbb ccc1 0100
// #######################################################################################################################
						return handleop_ADD1_S_b_b_c(op);  // ADD1_S b,b,c
					}
					case 0x15:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ADD2_S b,b,c                    0111 1bbb ccc1 0101
// #######################################################################################################################
						return handleop_ADD2_S_b_b_c(op);  // ADD2_S b,b,c
					}
					case 0x16:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ADD3_S b,b,c                    0111 1bbb ccc1 0110
// #######################################################################################################################
						return handleop_ADD3_S_b_b_c(op);  // ADD3_S b,b,c
					}
					case 0x18:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ASL_S b,b,c                     0111 1bbb ccc1 1000
// #######################################################################################################################
						return handleop_ASL_S_b_b_c_multiple(op);  // ASL_S b,b,c (multiple)
					}
					case 0x19:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// LSR_S b,b,c                     0111 1bbb ccc1 1001
// #######################################################################################################################
						return handleop_LSR_S_b_b_c_multiple(op);  // LSR_S b,b,c (multiple)
					}
					case 0x1a:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ASR_S b,b,c                     0111 1bbb ccc1 1010
// #######################################################################################################################
						return handleop_ASR_S_b_b_c_multiple(op);  // ASR_S b,b,c (multiple)
					}
					case 0x1b:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ASL_S b,c                       0111 1bbb ccc1 1011
// #######################################################################################################################
						return handleop_ASL_S_b_c_single(op);  // ASL_S b,c (single)
					}
					case 0x1c:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ASR_S b,c                       0111 1bbb ccc1 1100
// #######################################################################################################################
						return handleop_ASR_S_b_c_single(op);  // ASR_S b,c (single)
					}
					case 0x1d:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// LSR_S b,c                       0111 1bbb ccc1 1101
// #######################################################################################################################
						return handleop_LSR_S_b_c_single(op);  // LSR_S b,c (single)
					}
					case 0x1e:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// TRAP_S u6                       0111 1uuu uuu1 1110
// #######################################################################################################################
						return handleop_TRAP_S_u6(op);  // TRAP_S u6 (not a5?)
					}
					case 0x1f:
					{
						uint8_t subinstr2 = (op & 0x07e0) >> 5;
						if (subinstr2 == 0x3f)
						{
// #######################################################################################################################
//                                 IIII Isss sssS SSSS
// BRK_S                           0111 1111 1111 1111
// #######################################################################################################################
							return handleop_BRK_S(op);  // BRK_S ( 0x7fff only? )
						}
						else
						{
							return arcompact_handle_illegal(instruction,subinstr,subinstr2, op);
						}
					}
					default: // 0x01, 0x03, 0x08, 0x09, 0x0a, 0x17
					{
						return arcompact_handle_illegal(instruction, subinstr, op);
					}
				}
			}
			case 0x10:
			{
// #######################################################################################################################
//                                 IIII I
// LD_S c,[b,u7]                   1000 0bbb cccu uuuu
// #######################################################################################################################
				return handleop_LD_S_c_b_u7(op);    // LD_S c,[b,u7]
			}
			case 0x11:
			{
// #######################################################################################################################
//                                 IIII I
// LDB_S c,[b,u5]                  1000 1bbb cccu uuuu
// #######################################################################################################################
				return handleop_LDB_S_c_b_u5(op);    // LDB_S c,[b,u5]
			}
			case 0x12:
			{
// #######################################################################################################################
//                                 IIII I
// LDW_S c,[b,u6]                  1001 0bbb cccu uuuu
// #######################################################################################################################
				return handleop_LDW_S_c_b_u6(op);    // LDW_S c,[b,u6]
			}
			case 0x13:
			{
// #######################################################################################################################
//                                 IIII I
// LDW_S.X c,[b,u6]                1001 1bbb cccu uuuu
// #######################################################################################################################
				return handleop_LDW_S_X_c_b_u6(op);    // LDW_S.X c,[b,u6]
			}
			case 0x14:
			{
// #######################################################################################################################
//                                 IIII I
// ST_S c,[b,u7]                   1010 0bbb cccu uuuu
// #######################################################################################################################
				return handleop_ST_S_c_b_u7(op);    // ST_S c,[b,u7]
			}
			case 0x15:
			{
// #######################################################################################################################
//                                 IIII I
// STB_S c,[b,u5]                  1010 1bbb cccu uuuu
// #######################################################################################################################
				return handleop_STB_S_c_b_u5(op);    // STB_S
			}
			case 0x16:
			{
// #######################################################################################################################
//                                 IIII I
// STW_S c,[b,u6]                  1011 0bbb cccu uuuu
// #######################################################################################################################
				return handleop_STW_S_c_b_u6(op);    // STW_S
			}
			case 0x17: // Shift/Sub/Bit
			{
				uint8_t subinstr = (op & 0x00e0) >> 5;
				switch (subinstr)
				{
					default: return -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// ASL_S b,b,u5                    1011 1bbb 000u uuuu
// #######################################################################################################################
						return handleop_ASL_S_b_b_u5(op);  // ASL_S b,b,u5
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// LSR_S b,b,u5                    1011 1bbb 001u uuuu
// #######################################################################################################################
						return handleop_LSR_S_b_b_u5(op);  // LSR_S b,b,u5
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// ASR_S b,b,u5                    1011 1bbb 010u uuuu
// #######################################################################################################################
						return handleop_ASR_S_b_b_u5(op);  // ASR_S b,b,u5
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// SUB_S b,b,u5                    1011 1bbb 011u uuuu
// #######################################################################################################################
						return handleop_SUB_S_b_b_u5(op);  // SUB_S b,b,u5
					}
					case 0x04:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// BSET_S b,b,u5                   1011 1bbb 100u uuuu
// #######################################################################################################################
						return handleop_BSET_S_b_b_u5(op);  // BSET_S b,b,u5
					}
					case 0x05:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// BCLR_S b,b,u5                   1011 1bbb 101u uuuu
// #######################################################################################################################
						return handleop_BCLR_S_b_b_u5(op);  // BCLR_S b,b,u5
					}
					case 0x06:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// BMSK_S b,b,u5                   1011 1bbb 110u uuuu
// #######################################################################################################################
						return handleop_BMSK_S_b_b_u5(op);  // BMSK_S b,b,u5
					}
					case 0x07:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// BTST_S b,u5                     1011 1bbb 111u uuuu
// #######################################################################################################################
						return handleop_BTST_S_b_u5(op);  // BTST_S b,u5
					}
				}
			}
			case 0x18: // Stack Instr
			{
				uint8_t subinstr = (op & 0x00e0) >> 5;
				switch (subinstr)
				{
					default: return -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// LD_S b,[sp,u7]                  1100 0bbb 000u uuuu
// #######################################################################################################################
						return handleop_LD_S_b_sp_u7(op);  // LD_S b,[sp,u7]
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// LDB_S b,[sp,u7]                 1100 0bbb 001u uuuu
// #######################################################################################################################
						return handleop_LDB_S_b_sp_u7(op);  // LDB_S b,[sp,u7]
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// ST_S b,[sp,u7]                  1100 0bbb 010u uuuu
// #######################################################################################################################
						return handleop_ST_S_b_sp_u7(op);  // ST_S b,[sp,u7]
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// STB_S b,[sp,u7]                 1100 0bbb 011u uuuu
// #######################################################################################################################
						return handleop_STB_S_b_sp_u7(op);  // STB_S b,[sp,u7]
					}
					case 0x04:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// ADD_S b,sp,u7                   1100 0bbb 100u uuuu
// #######################################################################################################################
						return handleop_ADD_S_b_sp_u7(op);  // ADD_S b,sp,u7
					}

					case 0x05: // subtable 18_05
					{
						uint8_t subinstr2 = (op & 0x0700) >> 8;
						switch (subinstr2)
						{
							case 0x00:
							{
// #######################################################################################################################
//                                 IIII Isss SSS
// ADD_S sp,sp,u7                  1100 0000 101u uuuu
// #######################################################################################################################
								return handleop_ADD_S_sp_sp_u7(op);  // ADD_S sp,sp,u7
							}
							case 0x01:
							{
// #######################################################################################################################
//                                 IIII Isss SSS
// SUB_S sp,sp,u7                  1100 0001 101u uuuu
// #######################################################################################################################
								return handleop_SUB_S_sp_sp_u7(op);  // SUB_S sp,sp,u7
							}
							default: return arcompact_handle_illegal(instruction,subinstr,subinstr2, op);
						}
					}
					case 0x06: // subtable 18_06
					{
						uint8_t subinstr2 = op & 0x001f;
						switch (subinstr2)
						{
							case 0x01:
							{
// #######################################################################################################################
//                                 IIII I    SSSs ssss
// POP_S b                         1100 0bbb 1100 0001
// #######################################################################################################################
								return handleop_POP_S_b(op);  // POP_S b
							}
							case 0x11:
							{
// #######################################################################################################################
//                                 IIII I    SSSs ssss
// POP_S blink                     1100 0RRR 1101 0001
// #######################################################################################################################
								return handleop_POP_S_blink(op);  // POP_S blink
							}
							default: return arcompact_handle_illegal(instruction,subinstr,subinstr2, op);
						}
					}
					case 0x07: // subtable 18_07
					{
						uint8_t subinstr2 = op & 0x001f;

						switch (subinstr2)
						{
							case 0x01:
							{
// #######################################################################################################################
//                                 IIII I    SSSs ssss
// PUSH_S b                        1100 0bbb 1110 0001
// #######################################################################################################################
								return handleop_PUSH_S_b(op);  // PUSH_S b
							}
							case 0x11:
							{
// #######################################################################################################################
//                                 IIII I    SSSs ssss
// PUSH_S blink                    1100 0RRR 1111 0001
// #######################################################################################################################
								return handleop_PUSH_S_blink(op);  // PUSH_S blink
							}
							default: return arcompact_handle_illegal(instruction,subinstr,subinstr2, op);
						}
					}
				}
			}
			case 0x19: // GP Instr
			{
				uint8_t subinstr = (op & 0x0600) >> 9;

				switch (subinstr)
				{
					default: return -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII ISS
// LD_S r0,[gp,s11]                1100 100s ssss ssss
// #######################################################################################################################
						return handleop_LD_S_r0_gp_s11(op);  // LD_S r0,[gp,s11]
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII ISS
// LDB_S r0,[gp,s9]                1100 101s ssss ssss
// #######################################################################################################################
						return handleop_LDB_S_r0_gp_s9(op);  // LDB_S r0,[gp,s9]
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII ISS
// LDW_S r0,[gp,s10]               1100 110s ssss ssss
// #######################################################################################################################
						return handleop_LDW_S_r0_gp_s10(op);  // LDW_S r0,[gp,s10]
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII ISS
// ADD_S r0,gp,s11                 1100 111s ssss ssss
// #######################################################################################################################
						return handleop_ADD_S_r0_gp_s11(op);  // ADD_S r0,gp,s11
					}
				}
			}
			case 0x1a:
			{
// #######################################################################################################################
//                                 IIII I
// LD_S b,[pcl,u10]                1101 0bbb uuuu uuuu
// #######################################################################################################################
				return handleop_LD_S_b_pcl_u10(op);     // LD_S b,[pcl,u10]
			}

			case 0x1b:
			{
// #######################################################################################################################
//                                 IIII I
// MOV_S b,u8                      1101 1bbb uuuu uuuu
// #######################################################################################################################
				return handleop_MOV_S_b_u8(op);    // MOV_S b, u8
			}

			case 0x1c: // ADD_S/CMP_S
			{
				uint8_t subinstr = (op & 0x0080) >> 7;
				switch (subinstr)
				{
					default: return -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I    s
// ADD_S b,b,u7                    1110 0bbb 0uuu uuuu
// #######################################################################################################################
						return handleop_ADD_S_b_b_u7(op);  // ADD_S b, b, u7
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I    s
// CMP_S b,u7                      1110 0bbb 1uuu uuuu
// #######################################################################################################################
						return handleop_CMP_S_b_u7(op);  // CMP_S b, u7
					}
				}
			}
			case 0x1d: // BRcc_S
			{
				uint8_t subinstr = (op & 0x0080) >> 7;
				switch (subinstr)
				{
					default: return -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I    s
// BREQ_S b,0,s8                   1110 1bbb 0sss ssss
// #######################################################################################################################
						return handleop_BREQ_S_b_0_s8(op);  // BREQ_S b,0,s8
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I    s
// BRNE_S b,0,s8                   1110 1bbb 1sss ssss
// #######################################################################################################################
						return handleop_BRNE_S_b_0_s8(op);  // BRNE_S b,0,s8
					}
				}
			}
			case 0x1e: // Bcc_S
			{
				uint8_t subinstr = (op & 0x0600) >> 9;
				switch (subinstr)
				{
					default: return -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII ISS
// B_S s10                         1111 000s ssss ssss
// #######################################################################################################################
						return handleop_B_S_s10(op);  // B_S s10
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII ISS
// BEQ_S s10                       1111 001s ssss ssss
// #######################################################################################################################
						return handleop_BEQ_S_s10(op);  // BEQ_S s10
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII ISS
// BNE_S s10                       1111 010s ssss ssss
// #######################################################################################################################
						return handleop_BNE_S_s10(op);  // BNE_S s10
					}
					case 0x03: // Bcc_S
					{
						uint8_t subinstr2 = (op & 0x01c0) >> 6;
						switch (subinstr2)
						{
							default: return -1;
							case 0x00:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BGT_S s7                        1111 0110 00ss ssss
// #######################################################################################################################
								return handleop_BGT_S_s7(op);  // BGT_S s7
							}
							case 0x01:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BGE_S s7                        1111 0110 01ss ssss
// #######################################################################################################################
								return handleop_BGE_S_s7(op);  // BGE_S s7
							}
							case 0x02:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BLT_S s7                        1111 0110 10ss ssss
// #######################################################################################################################
								return handleop_BLT_S_s7(op);  // BLT_S s7
							}
							case 0x03:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BLE_S s7                        1111 0110 11ss ssss
// #######################################################################################################################
								return handleop_BLE_S_s7(op);  // BLE_S s7
							}
							case 0x04:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BHI_S s7                        1111 0111 00ss ssss
// #######################################################################################################################
								return handleop_BHI_S_s7(op);  // BHI_S s7
							}
							case 0x05:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BHS_S s7                        1111 0111 01ss ssss
// #######################################################################################################################
								return handleop_BHS_S_s7(op);  // BHS_S s7
							}
							case 0x06:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BLO_S s7                        1111 0111 10ss ssss
// #######################################################################################################################
								return handleop_BLO_S_s7(op);  // BLO_S s7
							}

							case 0x07:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BLS_S s7                        1111 0111 11ss ssss
// #######################################################################################################################
								return handleop_BLS_S_s7(op);  // BLS_S s7
							}
						}
					}
				}
			}
			case 0x1f:
			{
// #######################################################################################################################
//                                 IIII I
// BL_S s13                        1111 1sss ssss ssss
// #######################################################################################################################
				return handleop_BL_S_s13(op);    // BL_S s13
			}
		}
	}
	return 0;
}
