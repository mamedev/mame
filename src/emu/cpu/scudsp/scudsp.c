/*****************************************************************************
 *
 * SCUDSP CPU core
 *
 * skeleton for now ...
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "scudsp.h"


const device_type SCUDSP = &device_creator<scudsp_cpu_device>;

/* FLAGS */
#define PRF m_flags & 0x04000000
#define EPF m_flags & 0x02000000
#define T0F m_flags & 0x00800000
#define SF  m_flags & 0x00400000
#define ZF  m_flags & 0x00200000
#define CF  m_flags & 0x00100000
#define VF  m_flags & 0x00080000
#define EF  m_flags & 0x00040000
#define ESF m_flags & 0x00020000
#define EXF m_flags & 0x00010000 // execute flag (basically tied to RESET pin)
#define LEF m_flags & 0x00008000 // change PC value

#define FLAGS_MASK 0x06ff8000

#define scudsp_readop(A) m_program->read_dword(A << 2)
#define scudsp_writeop(A, B) m_program->write_dword(A << 2, B)
#define scudsp_readmem(A,MD) m_data->read_dword((A | MD << 6) << 2)
#define scudsp_writemem(A,MD,B) m_data->write_dword((A | MD << 6) << 2, B)

UINT32 scudsp_cpu_device::scudsp_get_source_mem_value(UINT8 mode)
{
	UINT32 value = 0;

	switch( mode )
	{
		case 0x0:   /* M0 */
			value = scudsp_readmem(m_ct0,0);
			break;
		case 0x1:   /* M1 */
			value = scudsp_readmem(m_ct1,1);
			break;
		case 0x2:   /* M2 */
			value = scudsp_readmem(m_ct2,2);
			break;
		case 0x3:   /* M3 */
			value = scudsp_readmem(m_ct3,3);
			break;
		case 0x4:   /* MC0 */
			value = scudsp_readmem(m_ct0++,0);
			m_ct0 &= 0x3f;
			break;
		case 0x5:   /* MC1 */
			value = scudsp_readmem(m_ct1++,1);
			m_ct1 &= 0x3f;
			break;
		case 0x6:   /* MC2 */
			value = scudsp_readmem(m_ct2++,2);
			m_ct2 &= 0x3f;
			break;
		case 0x7:   /* MC3 */
			value = scudsp_readmem(m_ct3++,3);
			m_ct3 &= 0x3f;
			break;
	}

	return value;
}

void scudsp_cpu_device::scudsp_set_dest_mem_reg( UINT32 mode, UINT32 value )
{
	switch( mode )
	{
		case 0x0:   /* MC0 */
			scudsp_writemem(m_ct0++,0,value);
			m_ct0 &= 0x3f;
			break;
		case 0x1:   /* MC1 */
			scudsp_writemem(m_ct1++,1,value);
			m_ct1 &= 0x3f;
			break;
		case 0x2:   /* MC2 */
			scudsp_writemem(m_ct2++,2,value);
			m_ct2 &= 0x3f;
			break;
		case 0x3:   /* MC3 */
			scudsp_writemem(m_ct3++,3,value);
			m_ct3 &= 0x3f;
			break;
#if 0
		case 0x4:   /* RX */
			dsp_reg.rx.ui = value;
			update_mul = 1;
			break;
		case 0x5:   /* PL */
			dsp_reg.pl.ui = value;
			dsp_reg.ph.si = (dsp_reg.pl.si < 0) ? -1 : 0;
			break;
		case 0x6:   /* RA0 */
			dsp_reg.ra0 = value;
			break;
		case 0x7:   /* WA0 */
			dsp_reg.wa0 = value;
			break;
		case 0x8:
		case 0x9:
			/* ??? */
			break;
		case 0xa:   /* LOP */
			dsp_reg.lop = value;
			break;
		case 0xb:   /* TOP */
			dsp_reg.top = value;
			break;
		case 0xc:   /* CT0 */
			dsp_reg.ct0 = value & 0x3f;
			break;
		case 0xd:   /* CT1 */
			dsp_reg.ct1 = value & 0x3f;
			break;
		case 0xe:   /* CT2 */
			dsp_reg.ct2 = value & 0x3f;
			break;
		case 0xf:   /* CT3 */
			dsp_reg.ct3 = value & 0x3f;
			break;
#endif
	}
}


READ32_MEMBER( scudsp_cpu_device::program_control_r )
{
	return (m_pc & 0xff) | (m_flags & FLAGS_MASK);
}

WRITE32_MEMBER( scudsp_cpu_device::program_control_w )
{
	UINT32 oldval, newval;

	oldval = (m_flags & 0xffffff00) | (m_pc & 0xff);
	newval = oldval;
	COMBINE_DATA(&newval);

	m_flags = newval & FLAGS_MASK;

	if(LEF)
		m_pc = newval & 0xff;

	//printf("%08x PRG CTRL\n",data);
	set_input_line(INPUT_LINE_RESET, (EXF) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE32_MEMBER( scudsp_cpu_device::program_w )
{
	//printf("%02x %08x PRG\n",m_pc,data);
	scudsp_writeop(m_pc++, data);
}

WRITE32_MEMBER( scudsp_cpu_device::ram_address_control_w )
{
	//printf("%02x %08x PRG\n",m_pc,data);
	m_ra = data & 0xff;

	switch((m_ra & 0xc0) >> 6)
	{
		case 0: m_ct0 = (m_ra & 0x3f); break;
		case 1: m_ct1 = (m_ra & 0x3f); break;
		case 2: m_ct2 = (m_ra & 0x3f); break;
		case 3: m_ct3 = (m_ra & 0x3f); break;
	}
}

READ32_MEMBER( scudsp_cpu_device::ram_address_r )
{
	UINT32 data;

	data = scudsp_get_source_mem_value( ((m_ra & 0xc0) >> 6) + 4 );

	return data;
}

WRITE32_MEMBER( scudsp_cpu_device::ram_address_w )
{
	scudsp_set_dest_mem_reg( (m_ra & 0xc0) >> 6, data );
}

/***********************************
 *  illegal opcodes
 ***********************************/
void scudsp_cpu_device::scudsp_illegal()
{
	//logerror("scudsp illegal opcode at 0x%04x\n", m_pc);
	m_icount -= 1;
}

/* Execute cycles */
void scudsp_cpu_device::execute_run()
{
	UINT32 opcode;

	do
	{
		debugger_instruction_hook(this, m_pc);

		opcode = scudsp_readop(m_pc);
		m_pc++;

		switch( opcode )
		{
			default:
				scudsp_illegal();
				break;
		}

	} while( m_icount > 0 );
}


void scudsp_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	save_item(NAME(m_pc));
	save_item(NAME(m_ra));
	save_item(NAME(m_ct0));
	save_item(NAME(m_ct1));
	save_item(NAME(m_ct2));
	save_item(NAME(m_ct3));
	save_item(NAME(m_flags));
	save_item(NAME(m_reset_state));

	// Register state for debugger
	state_add( SCUDSP_PC, "PC", m_pc ).formatstr("%02X");
	state_add( SCUDSP_FLAGS, "SR", m_flags ).formatstr("%08X");
	state_add( SCUDSP_RA, "RA", m_ra ).formatstr("%02X");
	state_add( SCUDSP_CT0, "CT0", m_ct0 ).formatstr("%02X");
	state_add( SCUDSP_CT1, "CT1", m_ct1 ).formatstr("%02X");
	state_add( SCUDSP_CT2, "CT2", m_ct2 ).formatstr("%02X");
	state_add( SCUDSP_CT3, "CT3", m_ct3 ).formatstr("%02X");
	state_add( STATE_GENPC, "curpc", m_pc ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_flags ).formatstr("%17s").noshow();

	m_icountptr = &m_icount;
}

void scudsp_cpu_device::device_reset()
{

}

void scudsp_cpu_device::execute_set_input(int irqline, int state)
{
	switch(irqline)
	{
		case SCUDSP_RESET:
			m_reset_state = state;
			break;
	}
}

scudsp_cpu_device::scudsp_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SCUDSP, "SCUDSP", tag, owner, clock, "scudsp", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 8, -2)
	, m_data_config("data", ENDIANNESS_BIG, 32, 8, -2)
{
}


void scudsp_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%s%s%s%c%c%c%c%c%s%s%s",
				m_flags & 0x4000000 ? "PR":"..",
				m_flags & 0x2000000 ? "EP":"..",
				m_flags & 0x800000 ? "T0":"..",
				m_flags & 0x400000 ? 'S':'.',
				m_flags & 0x200000 ? 'Z':'.',
				m_flags & 0x100000 ? 'C':'.',
				m_flags & 0x80000 ? 'V':'.',
				m_flags & 0x40000 ? 'E':'.',
				m_flags & 0x20000 ? "ES":"..",
				m_flags & 0x10000 ? "EX":"..",
				m_flags & 0x8000 ? "LE":"..");
			break;
	}
}


offs_t scudsp_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( scudsp );
	return CPU_DISASSEMBLE_NAME(scudsp)(this, buffer, pc, oprom, opram, options);
}
