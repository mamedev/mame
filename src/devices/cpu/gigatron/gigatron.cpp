// license:BSD-3-Clause
// copyright-holders:Sterophonick
/*****************************************************************************
 *
 * Skeleton device for Gigatron CPU Core
 *
 *****************************************************************************/

 //https://github.com/PhilThomas/gigatron/blob/master/src/gigatron.js

#include "emu.h"
#include "gigatron.h"
#include "gigatrondasm.h"


DEFINE_DEVICE_TYPE(GTRON, gigatron_cpu_device, "gigatron_cpu", "Gigatron CPU Device")


/* FLAGS */
#if 0
#define S  0x80
#define Z  0x40
#define OV 0x20
#define C  0x10
#endif


#define gigatron_readop(A) m_program->read_dword(A)
#define gigatron_readmem16(A) m_data->read_dword(A)
#define gigatron_writemem16(A,B) m_data->write_dword((A),B)


/***********************************
 *  illegal opcodes
 ***********************************/
void gigatron_cpu_device::gigatron_illegal()
{
	logerror("gigatron illegal opcode at 0x%04x\n", m_pc);
	m_icount -= 1;
}

/* Execute cycles */
void gigatron_cpu_device::execute_run()
{
	uint16_t opcode;

	do
	{
		debugger_instruction_hook(m_pc);

		opcode = gigatron_readop(m_pc);
		m_pc++;

		uint8_t op = (opcode >> 13) & 0x0007;
		uint8_t mode = (opcode >> 10) & 0x0007;
		uint8_t bus = (opcode >> 8) & 0x0003;
		uint8_t d = (opcode >> 0) & 0x00ff;

		switch( op)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				aluOp(op, mode, bus, d);
				break;
			case 6:
				storeOp(op, mode, bus, d);
				break;
			case 7:
				branchOp(op, mode, bus, d);
				break;
			default:
				gigatron_illegal();
				break;
		}

	} while( m_icount > 0 );
}


void gigatron_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	init();
}

void gigatron_cpu_device::init()
{
	m_ac = 0;
	m_x = 0;
	m_y = 0;
	m_pc = 0;
	state_add(GTRON_AC,        "AC",        m_ac);
	state_add(GTRON_X,         "X",         m_x);
	state_add(GTRON_Y,         "Y",         m_y);

	set_icountptr(m_icount);
}

void gigatron_cpu_device::branchOp(int op, int mode, int bus, int d)
{
}

void gigatron_cpu_device::aluOp(int op, int mode, int bus, int d)
{
	int b;
	(void)b;
	switch(bus) {
		case 0:
			b = d;
			break;
		case 1:
		case 2:
			b = m_ac;
			break;
		case 3:
			break;
	}
}

void gigatron_cpu_device::storeOp(int op, int mode, int bus, int d)
{
}

void gigatron_cpu_device::device_reset()
{
}

void gigatron_cpu_device::execute_set_input(int irqline, int state)
{
#if 0
	switch(irqline)
	{
		case GTRON_INT_INTRM: // level-sensitive
			m_intrm_pending = ((ASSERT_LINE == state) || (HOLD_LINE == state));
			m_intrm_state = (ASSERT_LINE == state);
			break;
		case GTRON_RESET: // edge-sensitive
			if (CLEAR_LINE != state)
				m_reset_pending = 1;
			m_reset_state = (ASSERT_LINE == state);
			break;
		case GTRON_INT_INTR: // edge-sensitive
			if (CLEAR_LINE != state)
				m_intr_pending = 1;
			m_intr_state = (ASSERT_LINE == state);
			break;
	}
#endif
}

gigatron_cpu_device::gigatron_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, GTRON, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, 14, -1)
	, m_data_config("data", ENDIANNESS_BIG, 8, 15, 0)
{
}


void gigatron_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = util::string_format("%c%c%c%c",
					m_flags & 0x80 ? 'S':'.',
					m_flags & 0x40 ? 'Z':'.',
					m_flags & 0x20 ? 'V':'.',
					m_flags & 0x10 ? 'C':'.');
			break;
	}
}


std::unique_ptr<util::disasm_interface> gigatron_cpu_device::create_disassembler()
{
	return std::make_unique<gigatron_disassembler>();
}


device_memory_interface::space_config_vector gigatron_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}
