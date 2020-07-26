// license:BSD-2-Clause
// copyright-holders:Sterophonick, Phil Thomas
/*****************************************************************************
 *
 * Skeleton device for Gigatron CPU Core
 *
 *****************************************************************************/

 //https://github.com/PhilThomas/gigatron/blob/master/src/gigatron.js

#include "emu.h"
#include "gigatron.h"
#include "gigatrondasm.h"


DEFINE_DEVICE_TYPE(GTRON, gigatron_cpu_device, "gigatron_cpu", "Gigatron CPU")


/* FLAGS */
#if 0
#define S  0x80
#define Z  0x40
#define OV 0x20
#define C  0x10
#endif

#define gigatron_readop(A) m_program->read_word(A)
#define gigatron_readmem16(A) m_data->read_dword(A)
#define gigatron_readmem8(A) m_data->read_byte(A)
#define gigatron_writemem16(A,B) m_data->write_dword((A),B)
#define gigatron_writemem8(A,B) m_data->write_byte((A),B)


/***********************************
 *  illegal opcodes
 ***********************************/
void gigatron_cpu_device::gigatron_illegal()
{
	logerror("gigatron illegal opcode at 0x%04x\n", m_ppc);
	m_icount -= 1;
}

/* Execute cycles */
void gigatron_cpu_device::execute_run()
{
	uint16_t opcode;

	do
	{
		m_ppc = m_pc;
		debugger_instruction_hook(m_pc);

		opcode = gigatron_readop(m_pc);
		m_pc = m_npc;
		m_npc = (m_pc + 1) & m_romMask;

		uint8_t op = (opcode >> 13) & 0x0007;
		uint8_t mode = (opcode >> 10) & 0x0007;
		uint8_t bus = (opcode >> 8) & 0x0003;
		uint8_t d = (opcode >> 0) & 0x00ff;

		switch (op)
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
		m_icount--;
	} while (m_icount > 0);
}


void gigatron_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	state_add(GTRON_PC,        "PC",        m_pc);
	state_add(GTRON_NPC,       "NPC",       m_npc);
	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_ppc).noshow();
	state_add(GTRON_AC,        "AC",        m_ac);
	state_add(GTRON_X,         "X",         m_x);
	state_add(GTRON_Y,         "Y",         m_y);
	state_add(GTRON_IREG,      "IREG",      m_inReg);
	state_add(GTRON_OUTX,      "OUTX",      m_outx);
	state_add(GTRON_OUT,       "OUT",       m_out);

	set_icountptr(m_icount);

	save_item(NAME(m_ac));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_npc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_inReg));
	save_item(NAME(m_pc));
	save_item(NAME(m_outx));
	save_item(NAME(m_out));

	m_outx_cb.resolve_safe();
	m_out_cb.resolve_safe();
	m_ir_cb.resolve_safe(0);

	reset_cpu();
}

void gigatron_cpu_device::reset_cpu()
{
	m_ac = 0;
	m_x = 0;
	m_y = 0;
	m_pc = 0;
	m_npc = (m_pc + 1) & m_romMask;
	m_ppc = 0;
	m_inReg = 0xFF;
	m_outx = 0;
	m_out = 0;
}

void gigatron_cpu_device::branchOp(uint8_t op, uint8_t mode, uint8_t bus, uint8_t d)
{
	const uint8_t ZERO = 0x80;
	bool c = false;
	uint8_t ac2 = m_ac ^ ZERO;
	uint16_t base = m_pc & 0xff00;
	switch (mode)
	{
	case 0: //jmp
		c = true;
		base = m_y << 8;
		break;
	case 1: //bgt
		c = (ac2 > ZERO);
		break;
	case 2: //blt
		c = (ac2 < ZERO);
		break;
	case 3: //bne
		c = (ac2 != ZERO);
		break;
	case 4: //beq
		c = (ac2 == ZERO);
		break;
	case 5: //bge
		c = (ac2 >= ZERO);
		break;
	case 6: //ble
		c = (ac2 <= ZERO);
		break;
	case 7: //bra
		c = true;
		break;
	}

	if (c)
	{
		uint8_t b = offset(bus, d);
		m_npc = base | b;
	}
}

void gigatron_cpu_device::aluOp(uint8_t op, uint8_t mode, uint8_t bus, uint8_t d)
{
	uint8_t b = 0;
	switch (bus)
	{
	case 0:
		b = d;
		break;
	case 1:
		b = gigatron_readmem8((uint16_t)(addr(mode, d) & m_ramMask));
		break;
	case 2:
		b = m_ac;
		break;
	case 3:
		b = m_inReg;
		break;
	}
	switch (op)
	{
	case 1:
		b = (m_ac & b);
		break;
	case 2:
		b = (m_ac | b);
		break;
	case 3:
		b = (m_ac ^ b);
		break;
	case 4:
		b = (m_ac + b);
		break;
	case 5:
		b = (m_ac - b);
		break;
	}
	switch (mode)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		m_ac = b;
		break;
	case 4:
		m_x = b;
		break;
	case 5:
		m_y = b;
		break;
	case 6:
	case 7:
		uint16_t rising = ~(m_out & b);
		m_out = b;
		m_out_cb(0, m_out, 0xFF);

		// rising edge of out[6] registers outx from ac
		if (rising & 0x40)
		{
			m_outx = m_ac;
			m_outx_cb(0, m_outx, 0xFF);
		}
		break;
	}
}

uint16_t gigatron_cpu_device::addr(uint8_t mode, uint8_t d)
{
	switch (mode)
	{
	case 0:
	case 4:
	case 5:
	case 6:
		return d;
	case 1:
		return m_x;
	case 2:
		return (m_y << 8) | d;
	case 3:
		return (m_y << 8) | m_x;
	case 7:
		uint16_t addr2 = (m_y << 8) | m_x;
		m_x = (m_x + 1) & 0xff;
		return addr2;
	}
	return 0;
}

uint8_t gigatron_cpu_device::offset(uint8_t bus, uint8_t d)
{
	switch (bus)
	{
	case 0:
		return d;
	case 1:
		return gigatron_readmem8(d);
	case 2:
		return m_ac;
	case 3:
		return m_inReg;
	}
	return 0;
}

void gigatron_cpu_device::storeOp(uint8_t op, uint8_t mode, uint8_t bus, uint8_t d)
{
	uint8_t b = 0;
	switch (bus)
	{
	case 0:
		b = d;
		break;
	case 1:
		b = 0;
		break;
	case 2:
		b = m_ac;
		break;
	case 3:
		b = m_inReg;
		break;
	}

	u16 address = addr(mode, d);
	if (bus == 1)
		logerror("%04x: ctrl = 0x%04x\n", m_ppc, address);
	else
		gigatron_writemem8(address & m_ramMask, b);

	switch (mode)
	{
	case 4: // XXX not clear whether x++ mode takes priority
		m_x = b;
		break;
	case 5:
		m_y = b;
		break;
	}
}

void gigatron_cpu_device::device_reset()
{
	reset_cpu();
}

void gigatron_cpu_device::execute_set_input(int irqline, int state)
{
#if 0
	switch (irqline)
	{
	default:
		break;
	}
#endif
}

gigatron_cpu_device::gigatron_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, GTRON, tag, owner, clock)
	, m_ramMask(0x7FFF)
	, m_romMask(0xFFFF)
	, m_program_config("program", ENDIANNESS_BIG, 16, 14, -1)
	, m_data_config("data", ENDIANNESS_BIG, 8, 15, 0)
	, m_outx_cb(*this)
	, m_out_cb(*this)
	, m_ir_cb(*this)
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
