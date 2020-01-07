// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
// thanks-to: Jeff Laughton
/*
  Diablo 1300 series Printer TTL CPU
  The work is based on the RE done by Jeff Laughton http://laughtonelectronics.com/Arcana/Diablo%20CPU/DiabloCPU.html
*/

#include "emu.h"
#include "debugger.h"
#include "diablo1300.h"
#include "diablo1300dasm.h"

//**************************************************************************
//  CONFIGURABLE LOGGING
//**************************************************************************
#define LOG_OP   (1U <<  1)
#define LOG_TABLE   (1U <<  2)

#define VERBOSE (LOG_GENERAL | LOG_OP | LOG_TABLE)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGOP(...)    LOGMASKED(LOG_OP,    __VA_ARGS__)
#define LOGTABLE(...) LOGMASKED(LOG_TABLE, __VA_ARGS__)

/*****************************************************************************/

inline uint16_t diablo1300_cpu_device::opcode_read(uint16_t address)
{
	return m_cache->read_word(address);
}

inline uint16_t diablo1300_cpu_device::program_read16(uint16_t address)
{
	return m_program->read_word(address);
}

inline void diablo1300_cpu_device::program_write16(uint16_t address, uint16_t data)
{
	m_program->write_word(address, data);
	return;
}

inline uint8_t diablo1300_cpu_device::data_read8(uint16_t address)
{
	return m_data->read_byte(address);
}

inline void diablo1300_cpu_device::data_write8(uint16_t address, uint8_t data)
{
	m_data->write_byte(address, data);
	return;
}

inline uint8_t diablo1300_cpu_device::read_reg(uint16_t reg)
{
	return data_read8(reg);
}

inline void diablo1300_cpu_device::write_reg(uint16_t reg, uint8_t data)
{
	data_write8(reg, data);
}

inline void diablo1300_cpu_device::write_port(uint16_t port, uint16_t data)
{
	// TODO: interact with mechanics/layout engine
}

inline uint8_t diablo1300_cpu_device::read_table(uint16_t offset)
{
	LOGTABLE("Read %02x from table ROM offset %04x[%04x]\n", m_table->base()[offset & 0x1ff], offset & 0x1ff, offset);
	return m_table->base()[offset & 0x1ff];
}

inline uint16_t diablo1300_cpu_device::read_ibus()
{
	// TODO: get signals from other boards
	return 0;
}

/*****************************************************************************/

DEFINE_DEVICE_TYPE(DIABLO1300, diablo1300_cpu_device, "diablo1300", "DIABLO 1300")

//-------------------------------------------------
//  diablo1300_cpu_device - constructor
//-------------------------------------------------

diablo1300_cpu_device::diablo1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, DIABLO1300, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 9, -1)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 5)
	, m_pc(0)
	, m_a(0)
	, m_b(0)
	, m_carry(0)
	, m_power_on(ASSERT_LINE)
	, m_program(nullptr)
	, m_data(nullptr)
	, m_cache(nullptr)
	, m_table(nullptr)
{
	// Allocate & setup
}


void diablo1300_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data    = &space(AS_DATA);
	m_cache   = m_program->cache<1, -1, ENDIANNESS_LITTLE>();
	m_table   = memregion("trom");

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).noshow();

	state_add(DIABLO_PC,         "PC",        m_pc).mask(0x1ff);
	state_add(DIABLO_A,          "A",         m_a).mask(0xff);
	state_add(DIABLO_B,          "B",         m_b).mask(0xff);
	state_add(DIABLO_CARRY,      "CARRY",     m_carry).formatstr("%1u");

	/* setup regtable */
	save_item(NAME(m_pc));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_carry));
	save_item(NAME(m_power_on));

	// set our instruction counter
	set_icountptr(m_icount);
}

void diablo1300_cpu_device::device_stop()
{
}

void diablo1300_cpu_device::device_reset()
{
	m_pc    = 0;
	m_a     = 0;
	m_b     = 0;
	m_carry = 0;

	m_power_on = ASSERT_LINE;  // should be CLEAR_LINE when card can detect power up
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector diablo1300_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> diablo1300_cpu_device::create_disassembler()
{
	return std::make_unique<diablo1300_disassembler>();
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t diablo1300_cpu_device::execute_min_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t diablo1300_cpu_device::execute_max_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------
void diablo1300_cpu_device::execute_run()
{
	uint32_t op;

	m_pc &= 0x1f;

	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		if( m_power_on == ASSERT_LINE )
		{
			op = opcode_read(m_pc);
			m_pc++;
			switch (op & 0x0007)
			{
			case 0:
				/* OUTPUT Dport, Sreg: Output register SSSS via reg A to port DDD, reg B and carry are cleared
				   111A SSSS 0DDD RIII
				      A                = 0: register is ORed into reg A, = 1: register is copied into reg A
				        SSSS           = Source register
				              DDD      = Destination port address
				                  R    = RAM bank select
				                   III = 000 (opcode)
				*/
				LOGOP("OUTPUT dv%d, r%02X\n",
					(op & 0x0070) >> 4,
					((op & 0x0f00) >> 8) + ((op & 0x0008) ? 0x10 : 0));
				m_a = read_reg(((op & 0x0f00) >> 8) + ((op & 0x0008) ? 0x10 : 0));
				m_b = 0;
				m_carry = 0;
				write_port((op & 0x0070) >> 4, m_a);
				break;
			case 1:
				/* JNC Addr: If carry not set: set PC to address H AAAA AAAA, reg B and carry are cleared
				   AAAA AAAA 0000 HIII
				   AAAA AAAA           = 8 low bits in Destination Address
				                  H    = The 9th hi address bit
				                   III = 001 (opcode)
				*/
				LOGOP("JNC    %03X\n", ((op & 0xff00) >> 8) + ((op & 0x0008) ? 0x100 : 0));
				m_a = (op & 0xff00) >> 8;
				m_b = 0;
				if (m_carry == 0)
				{
					m_pc = ((op & 0x0008) + m_a);
				}
				m_carry = 0;
				break;
			case 2:
				/* RST Dport : Reset Port
				   1111 0AAA BBBB RIII
				         AAA           = Device address
				             BBBB      = I8-I5 signals
				                  R    = RAM bank select
				                   III = 010 (opcode)
				*/
				LOGOP("RST    dv%d\n", (op & 0x0700) >> 8);
				m_b = read_ibus();
				m_a = read_port((op & 0x0700) >> 8);
				m_carry = (m_carry + m_a + m_b) > 0xff ? 1 : 0;
				break;
			case 3:
				/* LDBBIT Sreg, #value: Load AAAA AAAA #value into reg A, register BBBB reg B and set carry if #value != 0
				   AAAA AAAA BBBB RIII
				   AAAA AAAA           = bits to load immediate into A
				             BBBB      = register to load into B
				                  R    = RAM bank select
				                   III = 011 (opcode)
				*/
				LOGOP("LDBBIT r%02X, %02X\n",
					((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
					(op & 0xff00) >> 8);
				m_a = (op & 0xff00) >> 8;
				m_b = read_reg(((op & 0x00f0) >> 4));
				m_carry = (m_a & m_b) != 0 ? 1 : 0;
				break;
			case 4:
				switch(op & 0xc000)
				{
				case 0x4000:
					/* XLAT Dreg: Load table data into A and reg, 0 into B
					   II10 0000 AAAA RIII
					             AAAA      = Register
					              R    = RAM bank select
					   II              III = 01xx xxxx xxxx x100 (opcode)
					*/
					LOGOP("XLAT   r%02X\n",
						((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0));
					m_a = read_table(m_b + (m_carry != 0 ? 0x100 : 0x000));
					m_b = 0;
					m_carry = 0;
					write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
					break;

				case 0xc000:
					/* MOVCPL Dreg, Sreg: register to register within RAM bank, acc B and carry is cleared
					   II11 SSSS DDDD RIII
					            SSSS           = Source Register
					                 DDDD      = Destination register
					                      R    = RAM bank select
					       II              III = 11xx xxxx xxxx x100 (opcode)
					*/
					LOGOP("MOVCPL r%02X, r%02X\n",
						((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
						((op & 0x0f00) >> 8) + ((op & 0x0008) ? 0x10 : 0));
					m_a = read_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x0f00) >> 8));
					m_b = 0;
					m_carry = 0;
					write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
					break;

				case 0x8000:
					/* INPUT Dreg, Sport: port to register, acc B and carry is cleared
					   II10 SSSS DDDD RIII
					        SSSS           = Source Port
					         DDDD      = Destination register
					              R    = RAM bank select
					   II              III = 01xx xxxx xxxx x100 (opcode)
					*/
					LOGOP("INPUT  r%02X, dv%X\n",
						((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
						((op & 0x0f00) >> 8));
					m_a = read_port((op & 0x0f00) >> 8);
					m_b = 0;
					m_carry = 0;
					write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
					break;
				default:
					break;
				}
				break;
			case 5:
				/* LOAD# Dreg,#val: Load value AAAA AAAA into register DDDD, acc B and carry is cleared
				   AAAA AAAA DDDD RIII
				   AAAA AAAA           = bits to load into A
				             DDDD      = register put A into
				                  R    = RAM bank select
				                   III = 101 (opcode)
				*/
				LOGOP("LOAD#  r%02X, %02X\n",
					((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
					(op & 0xff00) >> 8);
				m_a = (op & 0xff00) >> 8;
				m_b = 0;
				m_carry = 0;
				write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
				break;
			case 6:
				/* ADCCPL S/Dreg, Sreg
				   1111 AAAA BBBB RIII
				        AAAA           = Load register AAAA into reg A
				             BBBB      = Load register into reg B
				                  R    = RAM bank select
				                   III = 110 (opcode)
				*/
				LOGOP("ADCCPL r%02X, r%02X\n",
					((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
					((op & 0x0f00) >> 8) + ((op & 0x0008) ? 0x10 : 0));
				m_a = read_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x0f00) >> 8));
				m_b = read_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4));
				m_carry = (m_a + m_b + m_carry) > 255 ? 1 : 0;
				write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
				break;
			case 7:
				/* ADC# S/Dreg, #val
				   AAAA AAAA BBBB RIII
				   AAAA AAAA           = Load bits AAAA AAAA into A
				             BBBB      = Load register BBBB into B
				                     R = RAM bank select
				                   III = 100 (opcode)
				*/
				LOGOP("ADC#   r%02X, %02X\n",
					((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
					(op & 0xff00) >> 8);
				m_a = (op & 0xff00) >> 8;
				m_b = read_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4));
				m_carry = (m_a + m_b + m_carry) > 255 ? 1 : 0;
				write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
				break;
			default:
				break;
			}
		}
		--m_icount;
	}
}
