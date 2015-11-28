// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*
    CPU emulation for Patinho Feio, the first computer designed and manufactured in Brazil
*/

#include "emu.h"
#include "debugger.h"
#include "patinho_feio.h"

#define CI       m_ci //The program counter is called "contador de instrucoes" in portuguese
#define ACC      m_acc

#define ADDRESS_MASK_4K    0xFFF
#define INCREMENT_CI_4K    (CI = (CI+1) & ADDRESS_MASK_4K)

const device_type PATINHO_FEIO  = &device_creator<patinho_feio_cpu_device>;

//Internal 4kbytes of RAM
static ADDRESS_MAP_START(prog_8bit, AS_PROGRAM, 8, patinho_feio_cpu_device)
    AM_RANGE(0x0000, 0x0fff) AM_RAM
ADDRESS_MAP_END

patinho_feio_cpu_device::patinho_feio_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, PATINHO_FEIO, "PATINHO FEIO", tag, owner, clock, "patinho_feio_cpu", __FILE__),
        m_program_config("program", ENDIANNESS_LITTLE, 8, 12, 0, ADDRESS_MAP_NAME(prog_8bit)),
        m_icount(0)
{
}

void patinho_feio_cpu_device::device_start()
{
	m_ci = 0;
	m_acc = 0;

    m_program = &space(AS_PROGRAM);

	save_item(NAME(m_ci));
	save_item(NAME(m_acc));

	// Register state for debugger
	state_add( PATINHO_FEIO_CI,         "CI",       m_ci         ).mask(0xFFF);
	state_add( PATINHO_FEIO_ACC,        "ACC",      m_acc        ).mask(0xFF);
    
    m_icountptr = &m_icount;
    m_run = true;
}


void patinho_feio_cpu_device::device_reset()
{
}

/* execute instructions on this CPU until icount expires */
void patinho_feio_cpu_device::execute_run()
{
	do
	{
		if ((! m_run)){
			m_icount = 0;   /* if processor is stopped, just burn cycles */
        } else {
            debugger_instruction_hook(this, CI);

            execute_instruction();
			m_icount --;
		}
	}
	while (m_icount > 0);
}


/* execute one instruction */
void patinho_feio_cpu_device::execute_instruction()
{
//    char opcode = patinho_feio_read(CI);
    INCREMENT_CI_4K;
}

offs_t patinho_feio_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( patinho_feio );
	return CPU_DISASSEMBLE_NAME(patinho_feio)(this, buffer, pc, oprom, opram, options);
}

