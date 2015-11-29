// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*
    CPU emulation for Patinho Feio, the first computer designed and manufactured in Brazil
*/

#include "emu.h"
#include "debugger.h"
#include "patinho_feio.h"

#define PC       m_pc //The program counter is called "contador de instrucoes" in portuguese
#define ACC      m_acc

#define READ_BYTE_PATINHO(A) ((signed)m_program->read_byte(A))
#define WRITE_BYTE_PATINHO(A,V) (m_program->write_byte(A,V))

#define READ_INDEX_REG() READ_BYTE_PATINHO(0x000)
#define WRITE_INDEX_REG(V) WRITE_BYTE_PATINHO(0x000, V)

#define ADDRESS_MASK_4K    0xFFF
#define INCREMENT_PC_4K    (PC = (PC+1) & ADDRESS_MASK_4K)

const device_type PATINHO_FEIO  = &device_creator<patinho_feio_cpu_device>;


//Internal 4kbytes of RAM
static ADDRESS_MAP_START(prog_8bit, AS_PROGRAM, 8, patinho_feio_cpu_device)
    AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("internalram")
ADDRESS_MAP_END

patinho_feio_cpu_device::patinho_feio_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, PATINHO_FEIO, "PATINHO FEIO", tag, owner, clock, "patinho_feio_cpu", __FILE__),
        m_program_config("program", ENDIANNESS_LITTLE, 8, 12, 0, ADDRESS_MAP_NAME(prog_8bit)),
        m_icount(0)
{
}

void patinho_feio_cpu_device::device_start()
{
    m_program = &space(AS_PROGRAM);

	save_item(NAME(m_pc));
	save_item(NAME(m_acc));

	// Register state for debugger
	state_add( PATINHO_FEIO_CI,         "CI",       m_pc         ).mask(0xFFF);
    state_add( PATINHO_FEIO_ACC,        "ACC",      m_acc        ).mask(0xFF);
    state_add(STATE_GENPC, "GENPC", m_pc).formatstr("0%06O").noshow();

    m_icountptr = &m_icount;
}


void patinho_feio_cpu_device::device_reset()
{
    m_pc = 0xE00;
    m_acc = 0;
    m_run = true;
}

/* execute instructions on this CPU until icount expires */
void patinho_feio_cpu_device::execute_run()
{
	do
	{
		if ((! m_run)){
			m_icount = 0;   /* if processor is stopped, just burn cycles */
        } else {
            execute_instruction();
			m_icount --;
		}
	}
	while (m_icount > 0);
}

/* execute one instruction */
void patinho_feio_cpu_device::execute_instruction()
{
    debugger_instruction_hook(this, PC);
    offs_t addr;
    unsigned char tmp;
    unsigned char opcode = READ_BYTE_PATINHO(PC);
    INCREMENT_PC_4K;

    switch (opcode){
        case 0xDA:
            //CARI="Carrega Imediato":
            //     Load an immediate into the accumulator
            ACC = READ_BYTE_PATINHO(PC);
            INCREMENT_PC_4K;
            return;
        case 0x9A:
            //INIB="Inibe"
            //     disables interrupts
            m_interrupts_enabled = false;
            return;
        case 0x9B:
            //PERM="Permite"
            //     enables interrupts
            m_interrupts_enabled = true;
            return;
        case 0x9C:
            //ESP="Espera":
            //    Holds execution and waits for an interrupt to occur.
            m_run = false;
            m_wait_for_interrupt = true;
            return;
        case 0x9D:
            //PARE="Pare":
            //    Holds execution. This can only be recovered by
            //    manually triggering execution again by
            //    pressing the "Partida" (start) button in the panel
            m_run = false;
            m_wait_for_interrupt = false;
            return;
        case 0x9E:
            //TRI="Troca com Indexador":
            //     Exchange the value of the accumulator with the index register
            tmp = ACC;
            ACC = READ_INDEX_REG();
            WRITE_INDEX_REG(tmp);
            return;
    }

    switch (opcode & 0xF0){
        case 0x00:
            //PLA = "Pula": Jump to address
            addr = READ_BYTE_PATINHO(PC) & 0xFFF;
            INCREMENT_PC_4K;
            PC = addr;
            return;
    }
    printf("unimplemented opcode: 0x%02X\n", opcode);
}

offs_t patinho_feio_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( patinho_feio );
	return CPU_DISASSEMBLE_NAME(patinho_feio)(this, buffer, pc, oprom, opram, options);
}

