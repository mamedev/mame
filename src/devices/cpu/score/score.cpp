// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Sunplus Technology S+core
    by Sandro Ronco

******************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "score.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const device_type SCORE7 = &device_creator<score7_cpu_device>;


//**************************************************************************
//  MACROS
//**************************************************************************

#include "scorem.h"


//**************************************************************************
//  Opcodes Tables
//**************************************************************************

const score7_cpu_device::op_handler score7_cpu_device::s_opcode32_table[4*8] =
{
	&score7_cpu_device::op_specialform, &score7_cpu_device::op_iform1, &score7_cpu_device::op_jump , &score7_cpu_device::op_rixform1, &score7_cpu_device::op_branch, &score7_cpu_device::op_iform2, &score7_cpu_device::op_crform, &score7_cpu_device::op_rixform2,
	&score7_cpu_device::op_addri      , &score7_cpu_device::op_undef , &score7_cpu_device::op_undef, &score7_cpu_device::op_undef   , &score7_cpu_device::op_andri , &score7_cpu_device::op_orri  , &score7_cpu_device::op_undef , &score7_cpu_device::op_undef   ,
	&score7_cpu_device::op_lw         , &score7_cpu_device::op_lh    , &score7_cpu_device::op_lhu  , &score7_cpu_device::op_lb      , &score7_cpu_device::op_sw    , &score7_cpu_device::op_sh    , &score7_cpu_device::op_lbu   , &score7_cpu_device::op_sb      ,
	&score7_cpu_device::op_cache      , &score7_cpu_device::op_undef , &score7_cpu_device::op_undef, &score7_cpu_device::op_undef   , &score7_cpu_device::op_cenew , &score7_cpu_device::op_undef , &score7_cpu_device::op_undef , &score7_cpu_device::op_undef
};

const score7_cpu_device::op_handler score7_cpu_device::s_opcode16_table[8] =
{
	&score7_cpu_device::op_rform1, &score7_cpu_device::op_undef, &score7_cpu_device::op_rform2, &score7_cpu_device::op_jform, &score7_cpu_device::op_branch16, &score7_cpu_device::op_ldiu, &score7_cpu_device::op_iform1a, &score7_cpu_device::op_iform1b
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  score7_cpu_device - constructor
//-------------------------------------------------

score7_cpu_device::score7_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SCORE7, "S+core 7", tag, owner, clock, "score7", __FILE__),
		m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0),
		m_pc(0),
		m_ppc(0)
{
	memset(m_gpr, 0x00, sizeof(m_gpr));
	memset(m_cr, 0x00, sizeof(m_cr));
	memset(m_sr, 0x00, sizeof(m_sr));
	memset(m_ce, 0x00, sizeof(m_ce));
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void score7_cpu_device::device_start()
{
	// find address spaces
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	// set our instruction counter
	m_icountptr = &m_icount;

	// register state for debugger
	state_add(SCORE_PC  , "PC"  , m_pc).callimport().callexport().formatstr("%08X");

	for(int i=0; i<0x20; i++)
		state_add(SCORE_GPR + i, string_format("r%d", i).c_str(), m_gpr[i]).callimport().callexport().formatstr("%08X");

	for(int i=0; i<0x20; i++)
		state_add(SCORE_CR + i, string_format("cr%d", i).c_str(), m_cr[i]).callimport().callexport().formatstr("%08X");

	for(int i=0; i<3; i++)
		state_add(SCORE_SR + i, string_format("sr%d", i).c_str(), m_sr[i]).callimport().callexport().formatstr("%08X");

	state_add(SCORE_CEH, "ceh", REG_CEH).callimport().callexport().formatstr("%08X");
	state_add(SCORE_CEL, "cel", REG_CEL).callimport().callexport().formatstr("%08X");

	state_add(STATE_GENPC, "curpc", m_pc).callimport().callexport().formatstr("%08X").noshow();
	state_add(STATE_GENPCBASE, "curpcbase", m_ppc).callimport().callexport().formatstr("%8X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_ppc).formatstr("%5s").noshow();

	// save state
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_op));
	save_item(NAME(m_gpr));
	save_item(NAME(m_cr));
	save_item(NAME(m_sr));
	save_item(NAME(m_ce));
	save_item(NAME(m_pending_interrupt));
}


//-------------------------------------------------
//  device_reset - reset up the device
//-------------------------------------------------

void score7_cpu_device::device_reset()
{
	// GPR are undefined at reset
	memset(m_gpr,0, sizeof(m_gpr));
	memset(m_cr, 0, sizeof(m_cr));
	memset(m_sr, 0, sizeof(m_sr));
	memset(m_ce, 0, sizeof(m_ce));
	memset(m_pending_interrupt, 0, sizeof(m_pending_interrupt));

	REG_EXCPVEC = m_pc = 0x9f000000;
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void score7_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%s%s%s%s%s",
				REG_CR & FLAG_V ? "V" : ".",
				REG_CR & FLAG_C ? "C" : ".",
				REG_CR & FLAG_Z ? "Z" : ".",
				REG_CR & FLAG_N ? "N" : ".",
				REG_CR & FLAG_T ? "T" : "."
			);
			break;
	}
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config * score7_cpu_device::memory_space_config(address_spacenum spacenum) const
{
	return  (spacenum == AS_PROGRAM) ? &m_program_config: nullptr;
}


//-------------------------------------------------
//  execute - execute for the provided number of
//  cycles
//-------------------------------------------------

void score7_cpu_device::execute_run()
{
	do
	{
		debugger_instruction_hook(this, m_pc);

		m_ppc = m_pc;

		check_irq();

		UINT32 op = fetch();

		switch(((op>>30) & 2) | ((op>>15) & 1))
		{
			case 0: // 16-bit + 16-bit instruction
				m_op = ((m_pc & 0x02) ? (op >> 16) : op) & 0x7fff;
				m_pc += 2;
				(this->*s_opcode16_table[(m_op >> 12) & 0x07])();
				break;
			case 1: // undefined parity bit
				m_pc += 4;
				gen_exception(EXCEPTION_P_EL);
				break;
			case 2: // parallel conditional execution
				m_op = (GET_T ? op: (op >> 16)) & 0x7fff;
				m_pc += 4;
				(this->*s_opcode16_table[(m_op >> 12) & 0x07])();
				break;
			case 3: // 32-bit instruction
				m_op = (op & 0x7fff) | ((op >> 1) & 0x3fff8000);
				m_pc += 4;
				(this->*s_opcode32_table[(m_op >> 25) & 0x01f])();
				break;
		}

		m_icount -= 3;  // FIXME: if available use correct cycles per instructions
	}
	while (m_icount > 0);
}


//-------------------------------------------------
//  execute_set_input
//-------------------------------------------------

void score7_cpu_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case 0:
		if(state)
		{
			int vector = standard_irq_callback(0);
			if (vector > 0 && vector < 64)
			{
				if((REG_PSR & 0x01) && state)
					m_pending_interrupt[vector] = true;
			}
		}
		break;
	}
}

//**************************************************************************
//  HELPERS
//**************************************************************************


bool score7_cpu_device::check_condition_branch(UINT8 bc)
{
	if ((bc & 0x0f) == 14)          // CNT>0, CNT--
	{
		if (REG_CNT > 0)
		{
			REG_CNT--;
			return true;
		}
		return false;
	}
	else
		return check_condition(bc);
}

bool score7_cpu_device::check_condition(UINT8 bc)
{
	switch(bc & 0x0f)
	{
		case  0:    return GET_C;                      // carry set
		case  1:    return !GET_C;                     // carry clear
		case  2:    return GET_C && !GET_Z;            // C & ~Z
		case  3:    return !GET_C || GET_Z;            // ~C | Z
		case  4:    return GET_Z;                      // Z
		case  5:    return !GET_Z;                     // ~Z
		case  6:    return !GET_Z && (GET_N == GET_V); // (Z = 0) & (N = V)
		case  7:    return GET_Z || (GET_N != GET_V);  // (Z = 1) | (N != V)
		case  8:    return (GET_N == GET_V);           // N = V
		case  9:    return (GET_N != GET_V);           //  N != V
		case 10:    return GET_N;                      // N
		case 11:    return !GET_N;                     // ~N
		case 12:    return GET_V;                      // overflow V
		case 13:    return !GET_V;                     // no overflow ~V
		case 14:    return REG_CNT > 0;                // CNT>0
		case 15:    return true;                       // always
	}

	return false;
}

INT32 score7_cpu_device::sign_extend(UINT32 data, UINT8 len)
{
	data &= (1 << len) - 1;
	UINT32 sign = 1 << (len - 1);
	return (data ^ sign) - sign;
}

UINT32 score7_cpu_device::fetch()
{
	return m_direct->read_dword(m_pc & ~3);
}

UINT8 score7_cpu_device::read_byte(offs_t offset)
{
	return m_program->read_byte(offset);
}

UINT16 score7_cpu_device::read_word(offs_t offset)
{
	return m_program->read_word(offset & ~1);
}

UINT32 score7_cpu_device::read_dword(offs_t offset)
{
	return m_program->read_dword(offset & ~3);
}

void score7_cpu_device::write_byte(offs_t offset, UINT8 data)
{
	m_program->write_byte(offset, data);
}

void score7_cpu_device::write_word(offs_t offset, UINT16 data)
{
	m_program->write_word(offset & ~1, data);
}

void score7_cpu_device::write_dword(offs_t offset, UINT32 data)
{
	m_program->write_dword(offset & ~3, data);
}

void score7_cpu_device::check_irq()
{
	if(REG_PSR & 0x01)
	{
		for (int i=63; i>0; i--)
		{
			if (m_pending_interrupt[i])
			{
				m_pending_interrupt[i] = false;
				debugger_interrupt_hook(this, i);
				gen_exception(EXCEPTION_INTERRUPT, i);
				return;
			}
		}
	}
}

void score7_cpu_device::gen_exception(int cause, UINT32 param)
{
	debugger_exception_hook(this, cause);

	REG_ECR = (REG_ECR & ~0x0000001f) | (cause & 0x1f);              // set exception cause
	REG_PSR = (REG_PSR & ~0x0000000f) | ((REG_PSR << 2) & 0x0c);     // push status bits
	REG_CR  = (REG_CR  & ~0x000003ff) | ((REG_CR << 5) & 0x3e0);     // push flag bits
	REG_EPC = m_ppc & 0xfffffffe;                                    // set return address

	switch(cause)
	{
		case EXCEPTION_P_EL:
			REG_EMA = REG_EPC;
			// intentional fallthrough
		case EXCEPTION_NMI:
		case EXCEPTION_CEE:
		case EXCEPTION_SYSCALL:
		case EXCEPTION_TRAP:
		case EXCEPTION_RI:
			m_pc = (REG_EXCPVEC & 0xffff0000) + 0x200;
			break;

		case EXCEPTION_SWI:
		case EXCEPTION_INTERRUPT:
			REG_ECR = (REG_ECR & ~0x00fc0000) | ((param & 0x3f) << 18);      // set irq source
			m_pc = (REG_EXCPVEC & 0xffff0000) + 0x200 + (param << (REG_EXCPVEC & 1 ? 4 : 2));
			break;

		case EXCEPTION_ADEL_INSTRUCTION:
		case EXCEPTION_BUSEL_INSTRUCTION:
		case EXCEPTION_CCU:
		case EXCEPTION_ADEL_DATA:
		case EXCEPTION_ADES_DATA:
		case EXCEPTION_CPE:
		case EXCEPTION_BUSEL_DATA:
			fatalerror("unhandled exception: %d 0x%08x (PC=0x%08x)\n", cause, param, m_ppc);
	}
}


//**************************************************************************
//  32-bit opcodes
//**************************************************************************

void score7_cpu_device::op_specialform()
{
	UINT8 ra = GET_S_RA(m_op);
	UINT8 rb = GET_S_RB(m_op);
	UINT8 rd = GET_S_RD(m_op);
	UINT8 cu = GET_S_CU(m_op);
	UINT32 r;

	switch(GET_S_FUNC6(m_op))
	{
		case 0x00:  // nop
			break;
		case 0x01:  // syscall
			gen_exception(EXCEPTION_SYSCALL);
			break;
		case 0x02:  // trap
			if (check_condition(rb))
				gen_exception(EXCEPTION_TRAP);
			break;
		case 0x03:  // sdbbp
			unemulated_op("sdbbp");
			break;
		case 0x04:  // br
			if (check_condition_branch(rb))
			{
				if (GET_S_LK(m_op))
					REG_LNK = m_pc;

				m_pc = m_gpr[ra];
			}
			break;
		case 0x05: // pflush
			unemulated_op("pflush");
			break;
		case 0x08:  // add
			r = m_gpr[ra] + m_gpr[rb];
			if (cu)
			{
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_ADD(m_gpr[ra], m_gpr[rb], r);
				CHECK_C_ADD(m_gpr[ra], m_gpr[rb]);
			}
			m_gpr[rd] = r;
			break;
		case 0x09:  // addc
			r = m_gpr[ra] + m_gpr[rb] + GET_C;
			if (cu)
			{
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_ADD(m_gpr[ra] + GET_C, m_gpr[rb], r);
				CHECK_C_ADD(m_gpr[ra] + GET_C, m_gpr[rb]);
			}
			m_gpr[rd] = r;
			break;
		case 0x0a:  // sub
			r = m_gpr[ra] - m_gpr[rb];
			if (cu)
			{
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_SUB(m_gpr[ra], m_gpr[rb], r);
				CHECK_C_SUB(m_gpr[ra], m_gpr[rb]);
			}
			m_gpr[rd] = r;
			break;
		case 0x0b:  // subc
			r = m_gpr[ra] - m_gpr[rb] - (GET_C ^ 1);
			if (cu)
			{
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_SUB(m_gpr[ra] - (GET_C ^ 1), m_gpr[rb], r);
				CHECK_C_SUB(m_gpr[ra] - (GET_C ^ 1), m_gpr[rb]);
			}
			m_gpr[rd] = r;
			break;
		case 0x0c:  // cmp
			if (cu)
			{
				r = m_gpr[ra] - m_gpr[rb];
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_SUB(m_gpr[ra], m_gpr[rb], r);
				CHECK_C_SUB(m_gpr[ra], m_gpr[rb]);
				switch(rd & 0x03)
				{
					case 0: SET_T(GET_Z); break;
					case 1: SET_T(GET_N); break;
				}
			}
			break;
		case 0x0d:  // cmpz
			if (cu)
			{
				r = m_gpr[ra] - 0;
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_SUB(m_gpr[ra], 0, r);
				CHECK_C_SUB(m_gpr[ra], 0);
				switch(rd & 0x03)
				{
					case 0: SET_T(GET_Z); break;
					case 1: SET_T(GET_N); break;
				}
			}
			break;
		case 0x0f:  // neg
			r = 0 - m_gpr[rb];
			if (cu)
			{
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_SUB(0, m_gpr[rb], r);
				CHECK_C_SUB(0, m_gpr[rb]);
			}
			m_gpr[rd] = r;
			break;
		case 0x10:  // and
			m_gpr[rd] = m_gpr[ra] & m_gpr[rb];
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
			}
			break;
		case 0x11:  // or
			m_gpr[rd] = m_gpr[ra] | m_gpr[rb];
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
			}
			break;
		case 0x12:  // not
			r = ~m_gpr[ra];
			if (cu)
			{
				CHECK_Z(r);
				CHECK_N(r);
			}
			m_gpr[rd] = r;
			break;
		case 0x13:  // xor
			m_gpr[rd] = m_gpr[ra] ^ m_gpr[rb];
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
			}
			break;
		case 0x14:  // bitclr
			m_gpr[rd] = m_gpr[ra] & ~(1 << rb);
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
			}
			break;
		case 0x15:  // bitset
			m_gpr[rd] = m_gpr[ra] | (1 << rb);
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
			}
			break;
		case 0x16:  // bittst
			if (cu)
			{
				r = m_gpr[ra] & (1 << rb);
				CHECK_N(m_gpr[ra]);
				CHECK_Z(r);
			}
			break;
		case 0x17:  // bittgl
			m_gpr[rd] = m_gpr[ra] ^ (1 << rb);
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
			}
			break;
		case 0x18:  // sll
			m_gpr[rd] = m_gpr[ra] << (m_gpr[rb] & 0x1f);
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
				SET_C(BIT(m_gpr[ra], 32 - (m_gpr[rb] & 0x1f)));
			}
			break;
		case 0x1a:  // srl
			m_gpr[rd] = m_gpr[ra] >> (m_gpr[rb] & 0x1f);
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
				SET_C(BIT(m_gpr[ra], (m_gpr[rb] & 0x1f) - 1));
			}
			break;
		case 0x1b:  // sra
			m_gpr[rd] = sign_extend(m_gpr[ra] >> (m_gpr[rb] & 0x1f), 32 - (m_gpr[rb] & 0x1f));
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
				SET_C(BIT(m_gpr[ra], (m_gpr[rb] & 0x1f) - 1));
			}
			break;
		case 0x1c:  // ror
			unemulated_op("ror");
			break;
		case 0x1d:  // rorc
			unemulated_op("rorc");
			break;
		case 0x1e:  // rol
			unemulated_op("rol");
			break;
		case 0x1f:  // rolc
			unemulated_op("rolc");
			break;
		case 0x20:  // mul
		{
			INT64 a = (INT32)m_gpr[ra];
			INT64 b = (INT32)m_gpr[rb];
			UINT64 d = a * b;
			REG_CEL = d & 0xffffffff;
			REG_CEH = (d >> 32) & 0xffffffff;
			break;
		}
		case 0x21:  // mulu
		{
			UINT64 a = (UINT32)m_gpr[ra];
			UINT64 b = (UINT32)m_gpr[rb];
			UINT64 d = a * b;
			REG_CEL = d & 0xffffffff;
			REG_CEH = (d >> 32) & 0xffffffff;
			break;
		}
		case 0x22:  // div
			if (m_gpr[rb])
			{
				INT32 a = (INT32)m_gpr[ra];
				INT32 b = (INT32)m_gpr[rb];
				REG_CEL = a / b;
				REG_CEH = a % b;
			}
			else
			{
				gen_exception(EXCEPTION_CEE);   // divide by zero exception
			}
			break;
		case 0x23:  // divu
			if (m_gpr[rb])
			{
				UINT32 a = (UINT32)m_gpr[ra];
				UINT32 b = (UINT32)m_gpr[rb];
				REG_CEL = a / b;
				REG_CEH = a % b;
			}
			else
			{
				gen_exception(EXCEPTION_CEE);   // divide by zero exception
			}
			break;
		case 0x24:  // mfce
			switch(rb & 3)
			{
				case 1: m_gpr[rd] = REG_CEL;                        break;
				case 2: m_gpr[rd] = REG_CEH;                        break;
				case 3: m_gpr[rd] = REG_CEH; m_gpr[ra] = REG_CEL;   break;
			}
			break;
		case 0x25:  // mtce
			switch(rb & 3)
			{
				case 1: REG_CEL = m_gpr[rd];                        break;
				case 2: REG_CEH = m_gpr[rd];                        break;
				case 3: REG_CEH = m_gpr[rd]; REG_CEL = m_gpr[ra];   break;
			}
			break;
		case 0x28:  // mfsr
			if (rb < 3)
				m_gpr[rd] = m_sr[rb];
			break;
		case 0x29:  // mtsr
			if (rb < 3)
				m_sr[rb] = m_gpr[ra];
			break;
		case 0x2a:  // t
			SET_T(check_condition(rb));
			break;
		case 0x2b:  // mv
			if ((rb & 0x0f) != 14 && check_condition(rb))
				m_gpr[rd] = m_gpr[ra];
			break;
		case 0x2c:  // extsb
		case 0x2d:  // extsh
			m_gpr[rd] = sign_extend(m_gpr[ra], (GET_S_FUNC6(m_op) & 1) ? 16 : 8);
			if (cu)
			{
				CHECK_N(m_gpr[rd]);
				CHECK_Z(m_gpr[rd]);     // undefined
			}
			break;
		case 0x2e:  // extzb
		case 0x2f:  // extzh
			m_gpr[rd] = m_gpr[ra] & ((GET_S_FUNC6(m_op) & 1) ? 0xffff : 0x00ff);
			if (cu)
			{
				CHECK_N(m_gpr[rd]);
				CHECK_Z(m_gpr[rd]);     // undefined
			}
			break;
		case 0x30:  // lcb
			unemulated_op("lcb");
			break;
		case 0x31:  // lcw
			unemulated_op("lcw");
			break;
		case 0x33:  // lce
			unemulated_op("lce");
			break;
		case 0x34:  // scb
			unemulated_op("scb");
			break;
		case 0x35:  // scw
			unemulated_op("scw");
			break;
		case 0x37:  // sce
			unemulated_op("sce");
			break;
		case 0x38:  // slli
			m_gpr[rd] = m_gpr[ra] << rb;
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
				SET_C(BIT(m_gpr[ra], 32 - rb));
			}
			break;
		case 0x3a:  // srli
			m_gpr[rd] = m_gpr[ra] >> rb;
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
				SET_C(BIT(m_gpr[ra], rb - 1));
			}
			break;
		case 0x3b:  // srai
			m_gpr[rd] = sign_extend(m_gpr[ra] >> rb, 32 - rb);
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
				SET_C(BIT(m_gpr[ra], rb - 1));
			}
			break;
		case 0x3c:  // rori
			unemulated_op("rori");
			break;
		case 0x3d:  // roric
			unemulated_op("roric");
			break;
		case 0x3e:  // roli
			unemulated_op("roli");
			break;
		case 0x3f:  // rolic
			unemulated_op("rolic");
			break;
		default:
			op_undef();
	}
}

void score7_cpu_device::op_iform1()
{
	UINT8 rd = GET_I_RD(m_op);
	UINT32 imm16 = GET_I_IMM16(m_op);
	INT32 simm16 = sign_extend(imm16, 16);
	UINT8 cu = GET_I_CU(m_op);
	UINT32 r;

	switch(GET_I_FUNC3(m_op))
	{
		case 0: // addi
			r = m_gpr[rd] + simm16;
			if (cu)
			{
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_ADD(m_gpr[rd], (UINT32)simm16, r);
				CHECK_C_ADD(m_gpr[rd], (UINT32)simm16);
			}
			m_gpr[rd] = r;
			break;
		case 2: // cmpi
			if (cu)
			{
				r = m_gpr[rd] - simm16;
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_SUB(m_gpr[rd], (UINT32)simm16, r);
				CHECK_C_SUB(m_gpr[rd], (UINT32)simm16);
			}
			break;
		case 4: // andi
			m_gpr[rd] = m_gpr[rd] & imm16;
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
			}
			break;
		case 5: // ori
			m_gpr[rd] = m_gpr[rd] | imm16;
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
			}
			break;
		case 6: // ldi
			m_gpr[rd] = simm16;
			break;
		default:
			op_undef();
	}
}

void score7_cpu_device::op_jump()
{
	if(GET_J_LK(m_op))
		REG_LNK = m_pc;

	m_pc = (m_ppc & 0xfe000000) | (GET_J_DISP24(m_op) << 1);
}

void score7_cpu_device::op_rixform1()
{
	UINT8 ra = GET_RIX_RA(m_op);
	UINT8 rd = GET_RIX_RD(m_op);

	// pre-increment
	m_gpr[ra] += sign_extend(GET_RIX_IMM12(m_op), 12);

	switch(GET_RIX_FUNC3(m_op))
	{
		case 0: // lw
			m_gpr[rd] = read_dword(m_gpr[ra]);
			break;
		case 1: // lh
			m_gpr[rd] = sign_extend(read_word(m_gpr[ra]), 16);
			break;
		case 2: // lhu
			m_gpr[rd] = read_word(m_gpr[ra]);
			break;
		case 3: // lb
			m_gpr[rd] = sign_extend(read_byte(m_gpr[ra]), 8);
			break;
		case 4: // sw
			write_dword(m_gpr[ra], m_gpr[rd]);
			break;
		case 5: // sh
			write_word(m_gpr[ra], m_gpr[rd] & 0xffff);
			break;
		case 6: // lbu
			m_gpr[rd] = read_byte(m_gpr[ra]);
			break;
		case 7: // sb
			write_byte(m_gpr[ra], m_gpr[rd] & 0xff);
			break;
	}
}

void score7_cpu_device::op_branch()
{
	if (check_condition_branch(GET_BC_BC(m_op)))
	{
		INT32 disp = sign_extend(GET_BC_DISP19(m_op), 19) << 1;
		if (GET_BC_LK(m_op))
			REG_LNK = m_pc;

		m_pc = m_ppc + disp;
	}
}

void score7_cpu_device::op_iform2()
{
	UINT8 rd = GET_I_RD(m_op);
	UINT32 imm16 = GET_I_IMM16(m_op) << 16;
	INT32 simm16 = (INT32)imm16;
	UINT8 cu = GET_I_CU(m_op);
	UINT32 r;

	switch(GET_I_FUNC3(m_op))
	{
		case 0: // addis
			r = m_gpr[rd] + simm16;
			if (cu)
			{
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_ADD(m_gpr[rd], imm16, r);
				CHECK_C_ADD(m_gpr[rd], imm16);
			}
			m_gpr[rd] = r;
			break;
		case 2: // cmpis
			if (cu)
			{
				r = m_gpr[rd] - simm16;
				CHECK_Z(r);
				CHECK_N(r);
				CHECK_V_SUB(m_gpr[rd], imm16, r);
				CHECK_C_SUB(m_gpr[rd], imm16);
			}
			break;
		case 4: // andis
			m_gpr[rd] = m_gpr[rd] & imm16;
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
			}
			break;
		case 5: // oris
			m_gpr[rd] = m_gpr[rd] | imm16;
			if (cu)
			{
				CHECK_Z(m_gpr[rd]);
				CHECK_N(m_gpr[rd]);
			}
			break;
		case 6: // ldis
			m_gpr[rd] = imm16;
			break;
		default:
			op_undef();
	}

}

void score7_cpu_device::op_crform()
{
	if ((REG_PSR & 0x08) && !(REG_PSR & 0x10000000))
		return;

	UINT8 cr = GET_CR_CR(m_op);
	UINT8 rd = GET_CR_RD(m_op);

	switch(GET_CR_OP(m_op))
	{
		case 0x00:  // mtcr
			m_cr[cr] = m_gpr[rd];
			break;
		case 0x01:  // mfcr
			m_gpr[rd] = m_cr[cr];
			break;
		case 0x84:  // rte
			REG_PSR = (REG_PSR & ~ 0x03) | ((REG_PSR >> 2) & 0x03);
			REG_CR = (REG_CR & ~0x1f) | ((REG_CR >> 5) & 0x1f);
			m_pc = REG_EPC;
			break;
		default:
			if ((GET_CR_OP(m_op) & 0xc0) == 0)
				fatalerror("%s: unemulated Coprocessor 0x%x (PC=0x%08x)\n", tag(), GET_CR_OP(m_op) & 0x07, m_ppc);
			else
				op_undef();
	}
}

void score7_cpu_device::op_rixform2()
{
	UINT8 ra = GET_RIX_RA(m_op);
	UINT8 rd = GET_RIX_RD(m_op);

	switch(GET_RIX_FUNC3(m_op))
	{
		case 0: // lw
			m_gpr[rd] = read_dword(m_gpr[ra]);
			break;
		case 1: // lh
			m_gpr[rd] = sign_extend(read_word(m_gpr[ra]), 16);
			break;
		case 2: // lhu
			m_gpr[rd] = read_word(m_gpr[ra]);
			break;
		case 3: // lb
			m_gpr[rd] = sign_extend(read_byte(m_gpr[ra]), 8);
			break;
		case 4: // sw
			write_dword(m_gpr[ra], m_gpr[rd]);
			break;
		case 5: // sh
			write_word(m_gpr[ra], m_gpr[rd] & 0xffff);
			break;
		case 6: // lbu
			m_gpr[rd] = read_byte(m_gpr[ra]);
			break;
		case 7: // sb
			write_byte(m_gpr[ra], m_gpr[rd] & 0xff);
			break;
	}

	// post-increment
	m_gpr[ra] += sign_extend(GET_RIX_IMM12(m_op), 12);
}

void score7_cpu_device::op_addri()
{
	UINT8 ra = GET_RI_RA(m_op);
	UINT8 rd = GET_RI_RD(m_op);
	INT32 simm14 = sign_extend(GET_RI_IMM14(m_op), 14);
	UINT8 cu = GET_RI_CU(m_op);

	UINT32 r = m_gpr[ra] + simm14;
	if (cu)
	{
		CHECK_Z(r);
		CHECK_N(r);
		CHECK_V_ADD(m_gpr[ra], (UINT32)simm14, r);
		CHECK_C_ADD(m_gpr[ra], (UINT32)simm14);
	}
	m_gpr[rd] = r;
}

void score7_cpu_device::op_andri()
{
	UINT8 ra = GET_RI_RA(m_op);
	UINT8 rd = GET_RI_RD(m_op);
	UINT32 imm14 = GET_RI_IMM14(m_op);

	m_gpr[rd] = m_gpr[ra] & imm14;

	if (GET_RI_CU(m_op))
	{
		CHECK_Z(m_gpr[rd]);
		CHECK_N(m_gpr[rd]);
	}
}

void score7_cpu_device::op_orri()
{
	UINT8 ra = GET_RI_RA(m_op);
	UINT8 rd = GET_RI_RD(m_op);
	UINT32 imm14 = GET_RI_IMM14(m_op);

	m_gpr[rd] = m_gpr[ra] | imm14;

	if (GET_RI_CU(m_op))
	{
		CHECK_Z(m_gpr[rd]);
		CHECK_N(m_gpr[rd]);
	}
}

void score7_cpu_device::op_lw()
{
	UINT8 rd = GET_LS_RD(m_op);
	UINT8 ra = GET_LS_RA(m_op);
	INT32 simm15 = sign_extend(GET_LS_IMM15(m_op), 15);

	m_gpr[rd] = read_dword(m_gpr[ra] + simm15);
}

void score7_cpu_device::op_lh()
{
	UINT8 rd = GET_LS_RD(m_op);
	UINT8 ra = GET_LS_RA(m_op);
	INT32 simm15 = sign_extend(GET_LS_IMM15(m_op), 15);

	m_gpr[rd] = sign_extend(read_word(m_gpr[ra] + simm15), 16);
}

void score7_cpu_device::op_lhu()
{
	UINT8 rd = GET_LS_RD(m_op);
	UINT8 ra = GET_LS_RA(m_op);
	INT32 simm15 = sign_extend(GET_LS_IMM15(m_op), 15);

	m_gpr[rd] = read_word(m_gpr[ra] + simm15);
}

void score7_cpu_device::op_lb()
{
	UINT8 rd = GET_LS_RD(m_op);
	UINT8 ra = GET_LS_RA(m_op);
	INT32 simm15 = sign_extend(GET_LS_IMM15(m_op), 15);

	m_gpr[rd] = sign_extend(read_byte(m_gpr[ra] + simm15), 8);
}

void score7_cpu_device::op_sw()
{
	UINT8 rd = GET_LS_RD(m_op);
	UINT8 ra = GET_LS_RA(m_op);
	INT32 simm15 = sign_extend(GET_LS_IMM15(m_op), 15);

	write_dword(m_gpr[ra] + simm15, m_gpr[rd]);
}

void score7_cpu_device::op_sh()
{
	UINT8 rd = GET_LS_RD(m_op);
	UINT8 ra = GET_LS_RA(m_op);
	INT32 simm15 = sign_extend(GET_LS_IMM15(m_op), 15);

	write_word(m_gpr[ra] + simm15, m_gpr[rd] & 0xffff);
}

void score7_cpu_device::op_lbu()
{
	UINT8 rd = GET_LS_RD(m_op);
	UINT8 ra = GET_LS_RA(m_op);
	INT32 simm15 = sign_extend(GET_LS_IMM15(m_op), 15);

	m_gpr[rd] = read_byte(m_gpr[ra] + simm15);
}

void score7_cpu_device::op_sb()
{
	UINT8 rd = GET_LS_RD(m_op);
	UINT8 ra = GET_LS_RA(m_op);
	INT32 simm15 = sign_extend(GET_LS_IMM15(m_op), 15);

	write_byte(m_gpr[ra] + simm15, m_gpr[rd] & 0xff);
}

void score7_cpu_device::op_cache()
{
	//unemulated_op("CACHE");
}

void score7_cpu_device::op_cenew()
{
	unemulated_op("CENew");
}


//**************************************************************************
//  16-bit opcodes
//**************************************************************************

void score7_cpu_device::op_rform1()
{
	UINT8 rd = GET_R_RD(m_op);
	UINT8 ra = GET_R_RA(m_op);

	switch(GET_R_FUNC4(m_op))
	{
		case 0x00:  // nop!
			break;
		case 0x01:  // mlfh!
			m_gpr[rd] = m_gpr[0x10 + ra];
			break;
		case 0x02:  // mhfl!
			m_gpr[0x10 + rd] = m_gpr[ra];
			break;
		case 0x03:  // mv!
			m_gpr[rd] = m_gpr[ra];
			break;
		case 0x04:  // br!
			if (check_condition_branch(rd))
				m_pc = m_gpr[ra];
			break;
		case 0x05:  // t!
			SET_T(check_condition(rd));
			break;
		case 0x0c:  // brl!
			if (check_condition_branch(rd))
			{
				REG_LNK = m_pc;
				m_pc = m_gpr[ra];
			}
			break;
		default:
			op_undef();
	}
}

void score7_cpu_device::op_rform2()
{
	UINT8 rd = GET_R_RD(m_op);
	UINT8 ra = GET_R_RA(m_op);
	UINT32 r;

	switch(GET_R_FUNC4(m_op))
	{
		case 0x00:  // add!
			r = m_gpr[rd] + m_gpr[ra];
			CHECK_Z(r);
			CHECK_N(r);
			CHECK_V_ADD(m_gpr[rd], m_gpr[ra], r);
			CHECK_C_ADD(m_gpr[rd], m_gpr[ra]);
			m_gpr[rd] = r;
			break;
		case 0x01:  // sub!
			r = m_gpr[rd] - m_gpr[ra];
			CHECK_Z(r);
			CHECK_N(r);
			CHECK_V_SUB(m_gpr[rd], m_gpr[ra], r);
			CHECK_C_SUB(m_gpr[rd], m_gpr[ra]);
			m_gpr[rd] = r;
			break;
		case 0x02:  // neg!
			m_gpr[rd] = 0 - m_gpr[ra];
			CHECK_Z(m_gpr[rd]);
			CHECK_N(m_gpr[rd]);
			CHECK_V_SUB(0, m_gpr[ra], m_gpr[rd]);
			CHECK_C_SUB(0, m_gpr[ra]);
			break;
		case 0x03:  // cmp!
			r = m_gpr[rd] - m_gpr[ra];
			CHECK_Z(r);
			CHECK_N(r);
			CHECK_V_SUB(m_gpr[rd], m_gpr[ra], r);
			CHECK_C_SUB(m_gpr[rd], m_gpr[ra]);
			break;
		case 0x04:  // and!
			m_gpr[rd] &= m_gpr[ra];
			CHECK_Z(m_gpr[rd]);
			CHECK_N(m_gpr[rd]);
			break;
		case 0x05:  // or!
			m_gpr[rd] |= m_gpr[ra];
			CHECK_Z(m_gpr[rd]);
			CHECK_N(m_gpr[rd]);
			break;
		case 0x06:  // not!
			m_gpr[rd] = ~m_gpr[ra];
			CHECK_Z(m_gpr[rd]);
			CHECK_N(m_gpr[rd]);
			break;
		case 0x07:  // xor!
			m_gpr[rd] ^= m_gpr[ra];
			CHECK_Z(m_gpr[rd]);
			CHECK_N(m_gpr[rd]);
			break;
		case 0x08:  // lw!
			m_gpr[rd] = read_dword(m_gpr[ra]);
			break;
		case 0x09:  // lh!
			m_gpr[rd] = sign_extend(read_word(m_gpr[ra]), 16);
			break;
		case 0x0a:  // pop!
			m_gpr[GET_P_RDG(m_op)] = read_dword(m_gpr[GET_P_RAG(m_op)]);
			m_gpr[GET_P_RAG(m_op)] += 0x04;
			break;
		case 0x0b:  // lbu!
			m_gpr[rd] = read_byte(m_gpr[ra]);
			break;
		case 0x0c:  // sw!
			write_dword(m_gpr[ra], m_gpr[rd]);
			break;
		case 0x0d:  // sh!
			write_word(m_gpr[ra], m_gpr[rd] & 0xffff);
			break;
		case 0x0e:  // push
			m_gpr[GET_P_RAG(m_op)] -= 0x04;
			write_dword(m_gpr[GET_P_RAG(m_op)], m_gpr[GET_P_RDG(m_op)]);
			break;
		case 0x0f:  // sb!
			write_byte(m_gpr[ra], m_gpr[rd] & 0xff);
			break;
	}
}

void score7_cpu_device::op_jform()
{
	if(GET_J_LK(m_op))
		REG_LNK = m_pc;

	m_pc = (m_ppc & 0xfffff000) | (GET_J_DISP11(m_op) << 1);
}

void score7_cpu_device::op_branch16()
{
	if(check_condition_branch(GET_BX_EC(m_op)))
		m_pc = m_ppc + (sign_extend(GET_BX_DISP8(m_op), 8) << 1);
}

void score7_cpu_device::op_ldiu()
{
	m_gpr[GET_I2_RD(m_op)] = GET_I2_IMM8(m_op);
}

void score7_cpu_device::op_iform1a()
{
	UINT8 rd = GET_I16_RD(m_op);
	UINT8 imm5 = GET_I16_IMM5(m_op);

	switch(GET_I16_FUNC3(m_op))
	{
		case 0x00:  // addei!
			unemulated_op("addei!");
			break;
		case 0x01:  // slli!
			m_gpr[rd] <<= imm5;
			CHECK_Z(m_gpr[rd]);
			CHECK_N(m_gpr[rd]);
			break;
		case 0x02:  // sdbbp!
			unemulated_op("sdbbp!");
			break;
		case 0x03:  // srli!
			m_gpr[rd] >>= imm5;
			CHECK_Z(m_gpr[rd]);
			CHECK_N(m_gpr[rd]);
			break;
		case 0x04:  // bitclr!
			m_gpr[rd] &= ~(1 << imm5);
			CHECK_Z(m_gpr[rd]);
			CHECK_N(m_gpr[rd]);
			break;
		case 0x05:  // bitset!
			m_gpr[rd] |= (1 << imm5);
			CHECK_Z(m_gpr[rd]);
			CHECK_N(m_gpr[rd]);
			break;
		case 0x06:  // bittst!
			CHECK_N(m_gpr[rd]);
			CHECK_Z(m_gpr[rd] & (1 << imm5));
			break;
		default:
			op_undef();
	}
}

void score7_cpu_device::op_iform1b()
{
	UINT8 rd = GET_I16_RD(m_op);
	UINT16 imm5 = GET_I16_IMM5(m_op);

	switch(GET_I16_FUNC3(m_op))
	{
		case 0x00:  // lwp!
			m_gpr[rd] = read_dword(REG_BP + (imm5<<2));
			break;
		case 0x01:  // lhp!
			m_gpr[rd] = sign_extend(read_word(REG_BP + (imm5<<1)), 16);
			break;
		case 0x03:  // lbup!
			m_gpr[rd] = read_byte(REG_BP + imm5);
			break;
		case 0x04:  // swp!
			write_dword(REG_BP + (imm5<<2), m_gpr[rd]);
			break;
		case 0x05:  // shp!
			write_word(REG_BP + (imm5<<1), m_gpr[rd] & 0xffff);
			break;
		case 0x07:  // sbp!
			write_byte(REG_BP + imm5, m_gpr[rd] & 0xff);
			break;
		default:
			op_undef();
	}
}

void score7_cpu_device::op_undef()
{
	logerror("%s: undefined instruction 0x%x (PC=0x%08x)\n", tag(), m_op, m_ppc);
	gen_exception(EXCEPTION_RI);
}

void score7_cpu_device::unemulated_op(const char * op)
{
	fatalerror("%s: unemulated %s (PC=0x%08x)\n", tag(), op, m_ppc);
}
