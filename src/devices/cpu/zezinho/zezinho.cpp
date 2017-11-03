// license:GPL-2.0+
// copyright-holders:Felipe Sanches

#include "emu.h"
#include "zezinho_cpu.h"
#include "debugger.h"
#include "includes/zezinho.h"

#define READ_PROG_BYTE_ZEZINHO(A) (m_program->read_byte(A))
#define WRITE_PROG_BYTE_ZEZINHO(A,V) (m_program->write_byte(A,V))

#define READ_DATA_BYTE_ZEZINHO(A) (m_data->read_byte(A))
#define WRITE_DATA_BYTE_ZEZINHO(A,V) (m_data->write_byte(A,V))

#define READ_DATA_WORD_ZEZINHO(A) ((int)((m_data->read_byte(2*A)<<8) | m_data->read_byte(2*A+1)))
#define WRITE_DATA_WORD_ZEZINHO(A,V) {m_data->write_byte(2*A,V>>8); m_data->write_byte(2*A+1,V&0xFF);}

#define ADDRESS_MASK_4K    0xFFF
#define INCREMENT_PC_4K    (m_pc = (m_pc+1) & ADDRESS_MASK_4K)

DEFINE_DEVICE_TYPE(ZEZINHO2_CPU, zezinho_cpu_device, "zezinho2_cpu", "Zezinho2 (ITA-II) CPU")

//Internal 4k-10bits of RAM
static ADDRESS_MAP_START(datamem_16bit, AS_DATA, /*10*/ 16, zezinho_cpu_device)
	AM_RANGE(0x0000, 0x0fff) AM_RAM
ADDRESS_MAP_END

//ROM
static ADDRESS_MAP_START(progmem_16bit, AS_PROGRAM, 16, zezinho_cpu_device)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

zezinho_cpu_device::zezinho_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, ZEZINHO2_CPU, tag, owner, clock)
	, m_data_config("data", ENDIANNESS_BIG, /*10*/ 16, 12, 0, ADDRESS_MAP_NAME(datamem_16bit))
	, m_program_config("program", ENDIANNESS_BIG, 16, 12, 0, ADDRESS_MAP_NAME(progmem_16bit))
	, m_icount(0)
{
}

device_memory_interface::space_config_vector zezinho_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void zezinho_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	save_item(NAME(m_pc));
	save_item(NAME(m_acc));
	save_item(NAME(m_addr));
	save_item(NAME(m_opcode));

	// Register state for debugger
	state_add( ZEZINHO_CI,         "CI",       m_pc         ).mask(0xFFF);
	state_add( ZEZINHO_ACC,        "ACC",      m_acc        ).mask(0xFFF);
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("0%06O").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).formatstr("0%06O").noshow();

	m_icountptr = &m_icount;
}

void zezinho_cpu_device::device_reset()
{
	m_pc = 0;
	m_acc = 0;
	m_run = true;
	m_addr = 0;
	m_opcode = 0;
}

/* execute instructions on this CPU until icount expires */
void zezinho_cpu_device::execute_run() {
	do {
		debugger_instruction_hook(this, m_pc);

		if (!m_run){
			m_icount = 0;   /* if processor is stopped, just burn cycles */
		} else {
			execute_instruction();
			m_icount --;
		}
	}
	while (m_icount > 0);
}

#define OPERAND_ADDRESS(operand) (~((m_opcode & 0x0F) << 8 | operand) & 0xFFF)

/* execute one instruction */
void zezinho_cpu_device::execute_instruction()
{
	int operand;
	m_opcode = READ_PROG_BYTE_ZEZINHO(m_pc); INCREMENT_PC_4K;
	operand = READ_PROG_BYTE_ZEZINHO(m_pc); INCREMENT_PC_4K;

	switch (m_opcode & 0xF0){
		case 0x20: // Sai
			printf("Sai: %d\n", READ_DATA_WORD_ZEZINHO(OPERAND_ADDRESS(operand)));
			return;
		case 0x30: // Armazena
			WRITE_DATA_WORD_ZEZINHO(OPERAND_ADDRESS(operand), m_acc);
			return;
		case 0xD0: // Limpa o acumulador e subtrai
			m_acc = -READ_DATA_WORD_ZEZINHO(OPERAND_ADDRESS(operand));
			return;
		case 0xE0: // Soma
			m_acc += READ_DATA_WORD_ZEZINHO(OPERAND_ADDRESS(operand));
			return;
		default:
			printf("unimplemented opcode: 0x%02X\n", m_opcode);
	}
}

offs_t zezinho_cpu_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE( zezinho );
	return CPU_DISASSEMBLE_NAME(zezinho)(this, stream, pc, oprom, opram, options);
}
