/*************************************************************/
/**                                                         **/
/**                        lr35902.c                        **/
/**                                                         **/
/** This file contains implementation for the GameBoy CPU.  **/
/** See lr35902.h for the relevant definitions. Please, note**/
/** that this code can not be used to emulate a generic Z80 **/
/** because the GameBoy version of it differs from Z80 in   **/
/** many ways.                                              **/
/**                                                         **/
/** Orginal cpu code (PlayBoy)  Carsten Sorensen    1998    **/
/** MESS modifications          Hans de Goede       1998    **/
/** Adapted to new cpuintrf     Juergen Buchmueller 2000    **/
/** Adapted to new cpuintrf     Anthony Kruize      2002    **/
/** Changed reset function to                               **/
/** reset all registers instead                             **/
/** of just AF.                            Wilbert Pol 2004 **/
/**                                                         **/
/** 1.1:                                                    **/
/**   Removed dependency on the mess gameboy driver         **/
/**                                                         **/
/** 1.2:                                                    **/
/**   Fixed cycle count for taking an interrupt             **/
/**   Fixed cycle count for BIT X,(HL) instructions         **/
/**   Fixed flags in RRCA instruction                       **/
/**   Fixed DAA instruction                                 **/
/**   Fixed flags in ADD SP,n8 instruction                  **/
/**   Fixed flags in LD HL,SP+n8 instruction                **/
/**                                                         **/
/** 1.3:                                                    **/
/**   Improved triggering of the HALT bug                   **/
/**   Added 4 cycle penalty when leaving HALT state for     **/
/**   newer versions of the cpu core                        **/
/**                                                         **/
/** 1.4:                                                    **/
/**   Split fetch and execute cycles.                       **/
/**                                                         **/
/*************************************************************/

#include "emu.h"
#include "debugger.h"
#include "lr35902.h"


#define IME     0x01
#define HALTED  0x02


//**************************************************************************
//  LR35902 DEVICE
//**************************************************************************

const device_type LR35902 = &device_creator<lr35902_cpu_device>;


lr35902_cpu_device::lr35902_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, LR35902, "LR35902", tag, owner, clock),
		m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
{
	c_regs = NULL;
	c_features = 0;
	c_timer_expired_func = NULL;
}


void lr35902_cpu_device::static_set_config(device_t &device, const lr35902_config &config)
{
	lr35902_cpu_device &conf = downcast<lr35902_cpu_device &>(device);
	static_cast<lr35902_config &>(conf) = config;
}


/****************************************************************************/
/* Memory functions                                                         */
/****************************************************************************/

inline void lr35902_cpu_device::cycles_passed(UINT8 cycles)
{
	m_icount -= cycles / m_gb_speed;
	if ( m_timer_expired_func )
	{
		m_timer_expired_func( this, cycles );
	}
}


inline UINT8 lr35902_cpu_device::mem_read_byte( UINT16 addr )
{
	UINT8 data = m_program->read_byte( addr );
	cycles_passed( 4 );
	return data;
}


inline void lr35902_cpu_device::mem_write_byte( UINT16 addr, UINT8 data )
{
	m_program->write_byte( addr, data );
	cycles_passed( 4 );
}


inline UINT16 lr35902_cpu_device::mem_read_word( UINT16 addr )
{
	UINT16 data = mem_read_byte( addr );
	data |= ( mem_read_byte( addr + 1 ) << 8 );
	return data;
}


inline void lr35902_cpu_device::mem_write_word( UINT16 addr, UINT16 data )
{
	mem_write_byte( addr, data & 0xFF );
	mem_write_byte( addr + 1, data >> 8 );
}


void lr35902_cpu_device::device_start()
{
	m_device = this;
	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_A));
	save_item(NAME(m_F));
	save_item(NAME(m_B));
	save_item(NAME(m_C));
	save_item(NAME(m_D));
	save_item(NAME(m_E));
	save_item(NAME(m_H));
	save_item(NAME(m_L));
	save_item(NAME(m_PC));
	save_item(NAME(m_SP));
	save_item(NAME(m_IE));
	save_item(NAME(m_IF));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_ei_delay));
	save_item(NAME(m_execution_state));
	save_item(NAME(m_op));
	save_item(NAME(m_gb_speed));
	save_item(NAME(m_gb_speed_change_pending));
	save_item(NAME(m_enable));
	save_item(NAME(m_doHALTbug));

	// Register state for debugger
	state_add( LR35902_PC, "PC", m_PC ).callimport().callexport().formatstr("%04X");
	state_add( LR35902_SP, "SP", m_SP ).callimport().callexport().formatstr("%04X");
	state_add( LR35902_A,  "A",  m_A  ).callimport().callexport().formatstr("%02X");
	state_add( LR35902_F,  "F",  m_F  ).callimport().callexport().formatstr("%02X");
	state_add( LR35902_B,  "B",  m_B  ).callimport().callexport().formatstr("%02X");
	state_add( LR35902_C,  "C",  m_C  ).callimport().callexport().formatstr("%02X");
	state_add( LR35902_D,  "D",  m_D  ).callimport().callexport().formatstr("%02X");
	state_add( LR35902_E,  "E",  m_E  ).callimport().callexport().formatstr("%02X");
	state_add( LR35902_H,  "H",  m_H  ).callimport().callexport().formatstr("%02X");
	state_add( LR35902_L,  "L",  m_L  ).callimport().callexport().formatstr("%02X");
	state_add( LR35902_IRQ_STATE, "IRQ", m_enable ).callimport().callexport().formatstr("%02X");
	state_add( LR35902_IE, "IE", m_IE ).callimport().callexport().formatstr("%02X");
	state_add( LR35902_IF, "IF", m_IF ).callimport().callexport().formatstr("%02X");

	state_add(STATE_GENPC, "curpc", m_PC).callimport().callexport().formatstr("%8s").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_F).mask(0xf0).formatstr("%8s").noshow();

	m_icountptr = &m_icount;
}


void lr35902_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case LR35902_SPEED:
			string.printf("%02X", 0x7E | ( ( m_gb_speed - 1 ) << 7 ) | m_gb_speed_change_pending );
			break;

		case STATE_GENFLAGS:
			string.printf("%c%c%c%c",
				m_F & FLAG_Z   ? 'Z' : '.',
				m_F & FLAG_N   ? 'N' : '.',
				m_F & FLAG_H   ? 'H' : '.',
				m_F & FLAG_C   ? 'C' : '.'
			);
			break;
	}
}

/*** Reset lr353902 registers: ******************************/
/*** This function can be used to reset the register      ***/
/*** file before starting execution with lr35902_execute(cpustate)***/
/*** It sets the registers to their initial values.       ***/
/************************************************************/
void lr35902_cpu_device::device_reset()
{
	m_A = 0x00;
	m_F = 0x00;
	m_B = 0x00;
	m_C = 0x00;
	m_D = 0x00;
	m_E = 0x00;
	m_H = 0x00;
	m_L = 0x00;
	m_SP = 0x0000;
	m_PC = 0x0000;
	if ( c_regs ) {
		m_A = c_regs[0] >> 8;
		m_F = c_regs[0] & 0xFF;
		m_B = c_regs[1] >> 8;
		m_C = c_regs[1] & 0xFF;
		m_D = c_regs[2] >> 8;
		m_E = c_regs[2] & 0xFF;
		m_H = c_regs[3] >> 8;
		m_L = c_regs[3] & 0xFF;
		m_SP = c_regs[4];
		m_PC = c_regs[5];
	}
	m_timer_expired_func = c_timer_expired_func;
	m_features = c_features;

	m_enable = 0;
	m_IE = 0;
	m_IF = 0;

	m_execution_state = 0;
	m_doHALTbug = 0;
	m_ei_delay = 0;
	m_gb_speed_change_pending = 0;
	m_gb_speed = 1;
}


offs_t lr35902_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( lr35902 );
	return CPU_DISASSEMBLE_NAME( lr35902 )(NULL, buffer, pc, oprom, opram, 0);
}


void lr35902_cpu_device::check_interrupts()
{
	UINT8 irq = m_IE & m_IF;

	/* Interrupts should be taken after the first instruction after an EI instruction */
	if (m_ei_delay) {
		m_ei_delay = 0;
		return;
	}

	/*
	   logerror("Attempting to process LR35902 Interrupt IRQ $%02X\n", irq);
	   logerror("Attempting to process LR35902 Interrupt IE $%02X\n", m_IE);
	   logerror("Attempting to process LR35902 Interrupt IF $%02X\n", m_IF);
	*/
	if (irq)
	{
		int irqline = 0;
		/*
		   logerror("LR35902 Interrupt IRQ $%02X\n", irq);
		*/

		for( ; irqline < 5; irqline++ )
		{
			if( irq & (1<<irqline) )
			{
				if (m_enable & HALTED)
				{
					m_enable &= ~HALTED;
					m_PC++;
					if ( m_features & LR35902_FEATURE_HALT_BUG ) {
						if ( ! ( m_enable & IME ) ) {
							/* Old cpu core (dmg/mgb/sgb) */
							m_doHALTbug = 1;
						}
					} else {
						/* New cpu core (cgb/agb/ags) */
						/* Adjust for internal syncing with video core */
						/* This feature needs more investigation */
						if ( irqline < 2 ) {
							cycles_passed( 4 );
						}
					}
				}
				if ( m_enable & IME ) {
					m_enable &= ~IME;
					m_IF &= ~(1 << irqline);
					cycles_passed( 12 );
					m_SP -= 2;
					mem_write_word( m_SP, m_PC );
					m_PC = 0x40 + irqline * 8;
					/*logerror("LR35902 Interrupt PC $%04X\n", m_PC );*/
					return;
				}
			}
		}
	}
}


/************************************************************/
/*** Execute lr35902 code for cycles cycles, return nr of ***/
/*** cycles actually executed.                            ***/
/************************************************************/
void lr35902_cpu_device::execute_run()
{
	do
	{
		if ( m_execution_state ) {
			UINT8   x;
			/* Execute instruction */
			switch( m_op ) {
#include "opc_main.h"
			}
		} else {
			/* Fetch and count cycles */
			check_interrupts();
			debugger_instruction_hook(this, m_PC);
			if ( m_enable & HALTED ) {
				cycles_passed( 4 );
				m_execution_state = 1;
			} else {
				m_op = mem_read_byte( m_PC++ );
				if ( m_doHALTbug ) {
					m_PC--;
					m_doHALTbug = 0;
				}
			}
		}
		m_execution_state ^= 1;
	} while (m_icount > 0);
}


void lr35902_cpu_device::execute_set_input( int inptnum, int state )
{
	m_irq_state = state;
	if( state == ASSERT_LINE )
	{
		m_IF |= (0x01 << inptnum);
	}
	else
	{
		m_IF &= ~(0x01 << inptnum);
	}
}


UINT8 lr35902_cpu_device::get_speed()
{
	return 0x7E | ( ( m_gb_speed - 1 ) << 7 ) | m_gb_speed_change_pending;
}


void lr35902_cpu_device::set_speed( UINT8 speed_request )
{
	m_gb_speed_change_pending = speed_request & 0x01;
}
