// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ccpu.c
    Core implementation for the portable Cinematronics CPU emulator.

    Written by Aaron Giles
    Special thanks to Zonn Moore for his detailed documentation.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "ccpu.h"


const device_type CCPU = &device_creator<ccpu_cpu_device>;


/***************************************************************************
    MACROS
***************************************************************************/

#define READOP(a)         (m_direct->read_byte(a))

#define RDMEM(a)          (m_data->read_word((a) * 2) & 0xfff)
#define WRMEM(a,v)        (m_data->write_word((a) * 2, (v)))

#define READPORT(a)       (m_io->read_byte(a))
#define WRITEPORT(a,v)    (m_io->write_byte((a), (v)))

#define SET_A0           do { m_a0flag = m_A; } while (0)
#define SET_CMP_VAL(x)    do { m_cmpacc = *m_acc; m_cmpval = (x) & 0xfff; } while (0)
#define SET_NC(a)         do { m_ncflag = ~(a); } while (0)
#define SET_MI(a)         do { m_nextnextmiflag = (a); } while (0)

#define TEST_A0          (m_a0flag & 1)
#define TEST_NC          ((m_ncflag >> 12) & 1)
#define TEST_MI          ((m_miflag >> 11) & 1)
#define TEST_LT          (m_cmpval < m_cmpacc)
#define TEST_EQ          (m_cmpval == m_cmpacc)
#define TEST_DR          (m_drflag != 0)

#define NEXT_ACC_A       do { SET_MI(*m_acc); m_acc = &m_A; } while (0)
#define NEXT_ACC_B       do { SET_MI(*m_acc); if (m_acc == &m_A) m_acc = &m_B; else m_acc = &m_A; } while (0)

#define CYCLES(x)         do { m_icount -= (x); } while (0)

#define STANDARD_ACC_OP(resexp,cmpval) \
do { \
	UINT16 result = resexp; \
	SET_A0;                      /* set the A0 bit based on the previous 'A' value */ \
	SET_CMP_VAL(cmpval);          /* set the compare values to the previous accumulator and the cmpval */ \
	SET_NC(result);               /* set the NC flag based on the unmasked result */ \
	*m_acc = result & 0xfff;     /* store the low 12 bits of the new value */ \
} while (0)



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

ccpu_cpu_device::ccpu_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, CCPU, "Cinematronics CPU", tag, owner, clock, "ccpu", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 8, 15, 0)
	, m_data_config("data", ENDIANNESS_BIG, 16, 32, -1)
	, m_io_config("io", ENDIANNESS_BIG, 8, 5, 0)
	, m_external_input(*this)
	, m_flags(0)
{
}


READ8_MEMBER( ccpu_cpu_device::read_jmi )
{
	/* this routine is called when there is no external input */
	/* and the JMI jumper is present */
	return TEST_MI;
}


void ccpu_cpu_device::wdt_timer_trigger()
{
	m_waiting = FALSE;
	m_watchdog++;
	if (m_watchdog >= 3)
		m_PC = 0;
}


void ccpu_cpu_device::device_start()
{
	/* copy input params */
	m_external_input.resolve_safe(0);
	m_vector_callback.bind_relative_to(*owner());
	assert(!m_vector_callback.isnull());

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_data = &space(AS_DATA);
	m_io = &space(AS_IO);

	save_item(NAME(m_PC));
	save_item(NAME(m_A));
	save_item(NAME(m_B));
	save_item(NAME(m_I));
	save_item(NAME(m_J));
	save_item(NAME(m_P));
	save_item(NAME(m_X));
	save_item(NAME(m_Y));
	save_item(NAME(m_T));
	save_item(NAME(m_a0flag));
	save_item(NAME(m_ncflag));
	save_item(NAME(m_cmpacc));
	save_item(NAME(m_cmpval));
	save_item(NAME(m_miflag));
	save_item(NAME(m_nextmiflag));
	save_item(NAME(m_nextnextmiflag));
	save_item(NAME(m_drflag));
	save_item(NAME(m_waiting));
	save_item(NAME(m_watchdog));

	// Register state for debugger
	state_add( CCPU_PC, "PC", m_PC).formatstr("%04X");
	state_add( CCPU_A,  "A",  m_A).mask(0xfff).formatstr("%03X");
	state_add( CCPU_B,  "B",  m_B).mask(0xfff).formatstr("%03X");
	state_add( CCPU_I,  "I",  m_I).mask(0xfff).formatstr("%03X");
	state_add( CCPU_J,  "J",  m_J).mask(0xfff).formatstr("%03X");
	state_add( CCPU_P,  "P",  m_P).mask(0xf).formatstr("%1X");
	state_add( CCPU_X,  "X",  m_X).mask(0xfff).formatstr("%03X");
	state_add( CCPU_Y,  "Y",  m_Y).mask(0xfff).formatstr("%03X");
	state_add( CCPU_T,  "T",  m_T).mask(0xfff).formatstr("%03X");
	state_add(STATE_GENPC, "curpc", m_PC).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).formatstr("%6s").noshow();

	m_icountptr = &m_icount;
}


void ccpu_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c",
					TEST_A0 ? '0' : 'o',
					TEST_NC ? 'N' : 'n',
					TEST_LT ? 'L' : 'l',
					TEST_EQ ? 'E' : 'e',
					m_external_input() ? 'M' : 'm',
					TEST_DR ? 'D' : 'd');
			break;
	}
}


void ccpu_cpu_device::device_reset()
{
	/* zero registers */
	m_PC = 0;
	m_A = 0;
	m_B = 0;
	m_I = 0;
	m_J = 0;
	m_P = 0;
	m_X = 0;
	m_Y = 0;
	m_T = 0;
	m_acc = &m_A;

	/* zero flags */
	m_a0flag = 0;
	m_ncflag = 0;
	m_cmpacc = 0;
	m_cmpval = 1;
	m_miflag = m_nextmiflag = m_nextnextmiflag = 0;
	m_drflag = 0;

	m_waiting = FALSE;
	m_watchdog = 0;
}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

void ccpu_cpu_device::execute_run()
{
	if (m_waiting)
	{
		m_icount = 0;
		return;
	}

	do
	{
		UINT16 tempval;
		UINT8 opcode;

		/* update the delayed MI flag */
		m_miflag = m_nextmiflag;
		m_nextmiflag = m_nextnextmiflag;

		/* fetch the opcode */
		debugger_instruction_hook(this, m_PC);
		opcode = READOP(m_PC++);

		switch (opcode)
		{
			/* LDAI */
			case 0x00:  case 0x01:  case 0x02:  case 0x03:
			case 0x04:  case 0x05:  case 0x06:  case 0x07:
			case 0x08:  case 0x09:  case 0x0a:  case 0x0b:
			case 0x0c:  case 0x0d:  case 0x0e:  case 0x0f:
				tempval = (opcode & 0x0f) << 8;
				STANDARD_ACC_OP(tempval, tempval);
				NEXT_ACC_A; CYCLES(1);
				break;

			/* INP */
			case 0x10:  case 0x11:  case 0x12:  case 0x13:
			case 0x14:  case 0x15:  case 0x16:  case 0x17:
			case 0x18:  case 0x19:  case 0x1a:  case 0x1b:
			case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
				if (m_acc == &m_A)
					tempval = READPORT(opcode & 0x0f) & 1;
				else
					tempval = READPORT(16 + (opcode & 0x07)) & 1;
				STANDARD_ACC_OP(tempval, tempval);
				NEXT_ACC_A; CYCLES(1);
				break;

			/* A8I */
			case 0x20:
				tempval = READOP(m_PC++);
				STANDARD_ACC_OP(*m_acc + tempval, tempval);
				NEXT_ACC_A; CYCLES(3);
				break;

			/* A4I */
			case 0x21:  case 0x22:  case 0x23:
			case 0x24:  case 0x25:  case 0x26:  case 0x27:
			case 0x28:  case 0x29:  case 0x2a:  case 0x2b:
			case 0x2c:  case 0x2d:  case 0x2e:  case 0x2f:
				tempval = opcode & 0x0f;
				STANDARD_ACC_OP(*m_acc + tempval, tempval);
				NEXT_ACC_A; CYCLES(1);
				break;

			/* S8I */
			case 0x30:
				tempval = READOP(m_PC++);
				STANDARD_ACC_OP(*m_acc + (tempval ^ 0xfff) + 1, tempval);
				NEXT_ACC_A; CYCLES(3);
				break;

			/* S4I */
			case 0x31:  case 0x32:  case 0x33:
			case 0x34:  case 0x35:  case 0x36:  case 0x37:
			case 0x38:  case 0x39:  case 0x3a:  case 0x3b:
			case 0x3c:  case 0x3d:  case 0x3e:  case 0x3f:
				tempval = opcode & 0x0f;
				STANDARD_ACC_OP(*m_acc + (tempval ^ 0xfff) + 1, tempval);
				NEXT_ACC_A; CYCLES(1);
				break;

			/* LPAI */
			case 0x40:  case 0x41:  case 0x42:  case 0x43:
			case 0x44:  case 0x45:  case 0x46:  case 0x47:
			case 0x48:  case 0x49:  case 0x4a:  case 0x4b:
			case 0x4c:  case 0x4d:  case 0x4e:  case 0x4f:
				tempval = READOP(m_PC++);
				m_J = (opcode & 0x0f) + (tempval & 0xf0) + ((tempval & 0x0f) << 8);
				NEXT_ACC_A; CYCLES(3);
				break;

			/* T4K */
			case 0x50:
				m_PC = (m_P << 12) + m_J;
				NEXT_ACC_B; CYCLES(4);
				break;

			/* JMIB/JEHB */
			case 0x51:
				if (m_external_input()) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_B; CYCLES(2);
				break;

			/* JVNB */
			case 0x52:
				if (TEST_DR) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_B; CYCLES(2);
				break;

			/* JLTB */
			case 0x53:
				if (TEST_LT) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_B; CYCLES(2);
				break;

			/* JEQB */
			case 0x54:
				if (TEST_EQ) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_B; CYCLES(2);
				break;

			/* JCZB */
			case 0x55:
				if (TEST_NC) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_B; CYCLES(2);
				break;

			/* JOSB */
			case 0x56:
				if (TEST_A0) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_B; CYCLES(2);
				break;

			/* SSA */
			case 0x57:
				NEXT_ACC_B; CYCLES(2);
				break;

			/* JMP */
			case 0x58:
				m_PC = ((m_PC - 1) & 0xf000) + m_J;
				NEXT_ACC_A; CYCLES(4);
				break;

			/* JMI/JEH */
			case 0x59:
				if (m_external_input()) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_A; CYCLES(2);
				break;

			/* JVN */
			case 0x5a:
				if (TEST_DR) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_A; CYCLES(2);
				break;

			/* JLT */
			case 0x5b:
				if (TEST_LT) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_A; CYCLES(2);
				break;

			/* JEQ */
			case 0x5c:
				if (TEST_EQ) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_A; CYCLES(2);
				break;

			/* JCZ */
			case 0x5d:
				if (TEST_NC) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_A; CYCLES(2);
				break;

			/* JOS */
			case 0x5e:
				if (TEST_A0) { m_PC = ((m_PC - 1) & 0xf000) + m_J; CYCLES(2); }
				NEXT_ACC_A; CYCLES(2);
				break;

			/* NOP */
			case 0x5f:
				NEXT_ACC_A; CYCLES(2);
				break;

			/* ADD */
			case 0x60:  case 0x61:  case 0x62:  case 0x63:
			case 0x64:  case 0x65:  case 0x66:  case 0x67:
			case 0x68:  case 0x69:  case 0x6a:  case 0x6b:
			case 0x6c:  case 0x6d:  case 0x6e:  case 0x6f:
				m_I = (m_P << 4) + (opcode & 0x0f);
				tempval = RDMEM(m_I);
				STANDARD_ACC_OP(*m_acc + tempval, tempval);
				NEXT_ACC_A; CYCLES(3);
				break;

			/* SUB */
			case 0x70:  case 0x71:  case 0x72:  case 0x73:
			case 0x74:  case 0x75:  case 0x76:  case 0x77:
			case 0x78:  case 0x79:  case 0x7a:  case 0x7b:
			case 0x7c:  case 0x7d:  case 0x7e:  case 0x7f:
				m_I = (m_P << 4) + (opcode & 0x0f);
				tempval = RDMEM(m_I);
				STANDARD_ACC_OP(*m_acc + (tempval ^ 0xfff) + 1, tempval);
				NEXT_ACC_A; CYCLES(3);
				break;

			/* SETP */
			case 0x80:  case 0x81:  case 0x82:  case 0x83:
			case 0x84:  case 0x85:  case 0x86:  case 0x87:
			case 0x88:  case 0x89:  case 0x8a:  case 0x8b:
			case 0x8c:  case 0x8d:  case 0x8e:  case 0x8f:
				m_P = opcode & 0x0f;
				NEXT_ACC_A; CYCLES(1);
				break;

			/* OUT */
			case 0x90:  case 0x91:  case 0x92:  case 0x93:
			case 0x94:  case 0x95:  case 0x96:  case 0x97:
			case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
			case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:
				if (m_acc == &m_A)
					WRITEPORT(opcode & 0x07, ~*m_acc & 1);
				NEXT_ACC_A; CYCLES(1);
				break;

			/* LDA */
			case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:
			case 0xa4:  case 0xa5:  case 0xa6:  case 0xa7:
			case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:
			case 0xac:  case 0xad:  case 0xae:  case 0xaf:
				m_I = (m_P << 4) + (opcode & 0x0f);
				tempval = RDMEM(m_I);
				STANDARD_ACC_OP(tempval, tempval);
				NEXT_ACC_A; CYCLES(3);
				break;

			/* TST */
			case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:
			case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
			case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:
			case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
				m_I = (m_P << 4) + (opcode & 0x0f);
				tempval = RDMEM(m_I);
				{
					UINT16 result = *m_acc + (tempval ^ 0xfff) + 1;
					SET_A0;
					SET_CMP_VAL(tempval);
					SET_NC(result);
					SET_MI(result);
				}
				NEXT_ACC_A; CYCLES(3);
				break;

			/* WS */
			case 0xc0:  case 0xc1:  case 0xc2:  case 0xc3:
			case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
			case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
			case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:
				m_I = (m_P << 4) + (opcode & 0x0f);
				m_I = RDMEM(m_I) & 0xff;
				NEXT_ACC_A; CYCLES(3);
				break;

			/* STA */
			case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:
			case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
			case 0xd8:  case 0xd9:  case 0xda:  case 0xdb:
			case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:
				m_I = (m_P << 4) + (opcode & 0x0f);
				WRMEM(m_I, *m_acc);
				NEXT_ACC_A; CYCLES(3);
				break;

			/* DV */
			case 0xe0:
				{
					INT16 stopX = (INT16)(m_A << 4) >> 4;
					INT16 stopY = (INT16)(m_B << 4) >> 4;

					stopX = ((INT16)(stopX - m_X) >> m_T) + m_X;
					stopY = ((INT16)(stopY - m_Y) >> m_T) + m_Y;

					m_vector_callback(m_X, m_Y, stopX, stopY, m_T);

					/* hack to make QB3 display semi-correctly during explosions */
					m_A = m_X & 0xfff;
					m_B = m_Y & 0xfff;
				}
				NEXT_ACC_A; CYCLES(1);
				break;

			/* LPAP */
			case 0xe1:
				m_J = RDMEM(m_I);
				NEXT_ACC_A; CYCLES(3);
				break;

			/* WSP */
			case 0xf1:
				m_I = RDMEM(m_I) & 0xff;
				NEXT_ACC_A; CYCLES(3);
				break;

			/* LKP */
			case 0xe2:
			case 0xf2:
				tempval = READOP(((m_PC - 1) & 0xf000) + *m_acc);
				STANDARD_ACC_OP(tempval, tempval);
				NEXT_ACC_A; CYCLES(7);
				m_PC++;
				break;

			/* MUL */
			case 0xe3:
			case 0xf3:
				tempval = RDMEM(m_I);
				SET_A0;
				m_cmpval = tempval & 0xfff;
				if (m_acc == &m_A)
				{
					if (m_A & 1)
					{
						UINT16 result;
						m_cmpacc = m_B;
						m_A = (m_A >> 1) | ((m_B << 11) & 0x800);
						m_B = ((INT16)(m_B << 4) >> 5) & 0xfff;
						result = m_B + tempval;
						SET_NC(result);
						SET_MI(result);
						m_B = result & 0xfff;
					}
					else
					{
						UINT16 result;
						m_cmpacc = m_A;
						result = m_A + tempval;
						m_A = (m_A >> 1) | ((m_B << 11) & 0x800);
						m_B = ((INT16)(m_B << 4) >> 5) & 0xfff;
						SET_NC(result);
						SET_MI(result);
					}
				}
				else
				{
					UINT16 result;
					m_cmpacc = m_B;
					m_B = ((INT16)(m_B << 4) >> 5) & 0xfff;
					result = m_B + tempval;
					SET_NC(result);
					SET_MI(result);
					if (m_A & 1)
						m_B = result & 0xfff;
				}
				NEXT_ACC_A; CYCLES(2);
				break;

			/* NV */
			case 0xe4:
			case 0xf4:
				m_T = 0;
				while (((m_A & 0xa00) == 0x000 || (m_A & 0xa00) == 0xa00) &&
						((m_B & 0xa00) == 0x000 || (m_B & 0xa00) == 0xa00) &&
						m_T < 16)
				{
					m_A = (m_A << 1) & 0xfff;
					m_B = (m_B << 1) & 0xfff;
					m_T++;
					CYCLES(1);
				}
				NEXT_ACC_A; CYCLES(1);
				break;

			/* FRM */
			case 0xe5:
			case 0xf5:
				m_waiting = TRUE;
				NEXT_ACC_A;
				m_icount = -1;

				/* some games repeat the FRM opcode twice; it apparently does not cause
				   a second wait, so we make sure we skip any duplicate opcode at this
				   point */
				if (READOP(m_PC) == opcode)
					m_PC++;
				break;

			/* STAP */
			case 0xe6:
			case 0xf6:
				WRMEM(m_I, *m_acc);
				NEXT_ACC_A; CYCLES(2);
				break;

			/* CST */
			case 0xf7:
				m_watchdog = 0;
			/* ADDP */
			case 0xe7:
				tempval = RDMEM(m_I);
				STANDARD_ACC_OP(*m_acc + tempval, tempval);
				NEXT_ACC_A; CYCLES(2);
				break;

			/* SUBP */
			case 0xe8:
			case 0xf8:
				tempval = RDMEM(m_I);
				STANDARD_ACC_OP(*m_acc + (tempval ^ 0xfff) + 1, tempval);
				NEXT_ACC_A; CYCLES(3);
				break;

			/* ANDP */
			case 0xe9:
			case 0xf9:
				tempval = RDMEM(m_I);
				STANDARD_ACC_OP(*m_acc & tempval, tempval);
				NEXT_ACC_A; CYCLES(2);
				break;

			/* LDAP */
			case 0xea:
			case 0xfa:
				tempval = RDMEM(m_I);
				STANDARD_ACC_OP(tempval, tempval);
				NEXT_ACC_A; CYCLES(2);
				break;

			/* SHR */
			case 0xeb:
			case 0xfb:
				tempval = ((m_acc == &m_A) ? (m_A >> 1) : ((INT16)(m_B << 4) >> 5)) & 0xfff;
				tempval |= (*m_acc + (0xb0b | (opcode & 0xf0))) & 0x1000;
				STANDARD_ACC_OP(tempval, 0xb0b | (opcode & 0xf0));
				NEXT_ACC_A; CYCLES(1);
				break;

			/* SHL */
			case 0xec:
			case 0xfc:
				tempval = (*m_acc << 1) & 0xfff;
				tempval |= (*m_acc + (0xc0c | (opcode & 0xf0))) & 0x1000;
				STANDARD_ACC_OP(tempval, 0xc0c | (opcode & 0xf0));
				NEXT_ACC_A; CYCLES(1);
				break;

			/* ASR */
			case 0xed:
			case 0xfd:
				tempval = ((INT16)(*m_acc << 4) >> 5) & 0xfff;
				STANDARD_ACC_OP(tempval, 0xd0d | (opcode & 0xf0));
				NEXT_ACC_A; CYCLES(1);
				break;

			/* SHRB */
			case 0xee:
			case 0xfe:
				if (m_acc == &m_A)
				{
					tempval = (m_A >> 1) | ((m_B << 11) & 0x800);
					m_B = ((INT16)(m_B << 4) >> 5) & 0xfff;
				}
				else
					tempval = ((INT16)(m_B << 4) >> 5) & 0xfff;
				tempval |= (*m_acc + (0xe0e | (opcode & 0xf0))) & 0x1000;
				STANDARD_ACC_OP(tempval, 0xe0e | (opcode & 0xf0));
				NEXT_ACC_A; CYCLES(1);
				break;

			/* SHLB */
			case 0xef:
			case 0xff:
				if (m_acc == &m_A)
				{
					tempval = (m_A << 1) & 0xfff;
					m_B = (m_B << 1) & 0xfff;
				}
				else
					tempval = (m_B << 1) & 0xfff;
				tempval |= (*m_acc + (0xf0f | (opcode & 0xf0))) & 0x1000;
				STANDARD_ACC_OP(tempval, 0xf0f | (opcode & 0xf0));
				NEXT_ACC_A; CYCLES(1);
				break;

			/* IV */
			case 0xf0:
				m_X = (INT16)(m_A << 4) >> 4;
				m_Y = (INT16)(m_B << 4) >> 4;
				NEXT_ACC_A; CYCLES(1);
				break;
		}
	} while (m_icount > 0);
}


offs_t ccpu_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( ccpu );
	return CPU_DISASSEMBLE_NAME(ccpu)(this, buffer, pc, oprom, opram, options);
}
