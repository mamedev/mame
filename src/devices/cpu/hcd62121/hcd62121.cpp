// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hitachi hcd62121 cpu core emulation.

The Hitachi hcd62121 is the custom cpu which was used in the Casio
CFX-9850 (and maybe some other things too).

This CPU core is based on the information provided by Martin Poupe.
Martin Poupe's site can be found at http://martin.poupe.org/casio/

**********************************************************************/

#include "emu.h"
#include "debugger.h"
#include "hcd62121.h"


/* From the battery check routine at 20:e874 it looks like
   bit 3 of the flag register should be the Zero flag. */
#define _FLAG_Z     0x08
#define _FLAG_C     0x02
#define _FLAG_ZL    0x04
#define _FLAG_CL    0x01
#define _FLAG_ZH    0x10


const device_type HCD62121 = &device_creator<hcd62121_cpu_device>;


hcd62121_cpu_device::hcd62121_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, HCD62121, "Hitachi HCD62121", tag, owner, clock, "hcd62121", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 8, 24, 0)
	, m_io_config("io", ENDIANNESS_BIG, 8, 8, 0), m_prev_pc(0)
		, m_sp(0)
	, m_ip(0)
	, m_dsize(0)
	, m_cseg(0)
	, m_dseg(0)
	, m_sseg(0)
	, m_f(0)
	, m_lar(0), m_program(nullptr), m_io(nullptr), m_icount(0)
{
}


UINT8 hcd62121_cpu_device::read_op()
{
	UINT8 d = m_program->read_byte( ( m_cseg << 16 ) | m_ip );
	m_ip++;
	return d;
}


UINT8 hcd62121_cpu_device::datasize( UINT8 op )
{
	switch( op & 0x03 )
	{
	case 0:
		return 1;
	case 1:
		return 2;
	case 2:
		return ( m_dsize >> 4 ) + 1;
	case 3:
		return ( m_dsize & 0x0f ) + 1;
	}
	return 1;
}


void hcd62121_cpu_device::read_reg( int size, UINT8 op1 )
{
	int i;

	if ( op1 & 0x80 )
	{
		for ( i = 0; i < size; i++ )
			m_temp1[i] = m_reg[ ( op1 - i ) & 0x7f ];
	}
	else
	{
		for ( i = 0; i < size; i++ )
			m_temp1[i] = m_reg[ ( op1 + i ) & 0x7f ];
	}
}


void hcd62121_cpu_device::write_reg( int size, UINT8 op1 )
{
	int i;

	if ( op1 & 0x80 )
	{
		for ( i = 0; i < size; i++ )
			m_reg[ ( op1 - i ) & 0x7f ] = m_temp1[i];
	}
	else
	{
		for ( i = 0; i < size; i++ )
			m_reg[ ( op1 + i ) & 0x7f ] = m_temp1[i];
	}
}


void hcd62121_cpu_device::read_regreg( int size, UINT8 op1, UINT8 op2, bool op_is_logical )
{
	int i;

	for ( i = 0; i < size; i++ )
		m_temp1[i] = m_reg[ (op1 + i) & 0x7f];

	if ( op1 & 0x80 )
	{
		/* Second operand is an immediate value */
		m_temp2[0] = op2;
		for ( i = 1; i < size; i++ )
			m_temp2[i] = op_is_logical ? op2 : 0;
	}
	else
	{
		/* Second operand is a register */
		for ( i = 0; i < size; i++ )
			m_temp2[i] = m_reg[ (op2 + i) & 0x7f ];
	}

	if ( ! ( op1 & 0x80 ) && ! ( op2 & 0x80 ) )
	{
		/* We need to swap parameters */
		for ( i = 0; i < size; i++ )
		{
			UINT8 v = m_temp1[i];
			m_temp1[i] = m_temp2[i];
			m_temp2[i] = v;
		}
	}
}


void hcd62121_cpu_device::write_regreg( int size, UINT8 op1, UINT8 op2 )
{
	int i;

	if ( ( op1 & 0x80 ) || ( op2 & 0x80 ) )
	{
		/* store in reg1 */
		for ( i = 0; i < size; i++ )
			m_reg[ (op1 + i) & 0x7f] = m_temp1[i];
	}
	else
	{
		/* store in reg2 */
		for ( i = 0; i < size; i++ )
			m_reg[ (op2 + i) & 0x7f] = m_temp1[i];
	}
}


void hcd62121_cpu_device::read_iregreg( int size, UINT8 op1, UINT8 op2 )
{
	int i;
	UINT16 ad;

	ad = m_reg[ ( 0x40 | op1 ) & 0x7f ] | ( m_reg[ ( 0x40 | ( op1 + 1 ) ) & 0x7f ] << 8 );

	for ( i = 0; i < size; i++ )
	{
		m_temp1[i] = m_program->read_byte( ( m_dseg << 16 ) | ad );
		ad += ( op1 & 0x40 ) ? -1 : 1;
	}
	m_lar = ad;

	if ( op1 & 0x80 )
	{
		m_temp2[0] = op2;
		for ( i = 1; i < size; i++ )
			m_temp2[i] = 0;
	}
	else
	{
		for ( i = 0; i < size; i++ )
			m_temp2[i] = m_reg[ (op2 + i) & 0x7f ];
	}

	if ( ! ( op1 & 0x80 ) && ! ( op2 & 0x80 ) )
	{
		/* We need to swap parameters */
		for ( i = 0; i < size; i++ )
		{
			UINT8 v = m_temp1[i];
			m_temp1[i] = m_temp2[i];
			m_temp2[i] = v;
		}
	}
}


void hcd62121_cpu_device::write_iregreg( int size, UINT8 op1, UINT8 op2 )
{
	int i;

	if ( ( op1 & 0x80 ) || ( op2 & 0x80 ) )
	{
		/* store in (reg1) */
		UINT16 ad = m_reg[ ( 0x40 | op1 ) & 0x7f ] | ( m_reg[ ( 0x40 | ( op1 + 1 ) ) & 0x7f ] << 8 );

		for ( i = 0; i < size; i++ )
		{
			m_program->write_byte( ( m_dseg << 16 ) | ad, m_temp1[i] );
			ad += ( op1 & 0x40 ) ? -1 : 1;
		}
		m_lar = ad;
	}
	else
	{
		/* store in reg2 */
		for ( i = 0; i < size; i++ )
			m_reg[ (op2 + i) & 0x7f] = m_temp1[i];
	}
}


void hcd62121_cpu_device::write_iregreg2( int size, UINT8 op1, UINT8 op2 )
{
	int i;

	if ( ( op1 & 0x80 ) || ( op2 & 0x80 ) )
	{
		/* store in reg2 */
		for ( i = 0; i < size; i++ )
			m_reg[ (op2 + i) & 0x7f] = m_temp2[i];
	}
	else
	{
		/* store in (reg1) */
		UINT16 ad = m_reg[ ( 0x40 | op1 ) & 0x7f ] | ( m_reg[ ( 0x40 | ( op1 + 1 ) ) & 0x7f ] << 8 );

		for ( i = 0; i < size; i++ )
		{
			m_program->write_byte( ( m_dseg << 16 ) | ad, m_temp2[i] );
			ad += ( op1 & 0x40 ) ? -1 : 1;
		}
		m_lar = ad;
	}
}


int hcd62121_cpu_device::check_cond( UINT8 op )
{
	switch ( op & 0x07 )
	{
	case 0x00:  /* ZH set */
		if ( m_f & _FLAG_ZH )
			return 1;
		break;

	case 0x01:  /* ZL set */
		if ( m_f & _FLAG_ZL )
			return 1;
		break;

	case 0x02:  /* C set */
		if ( m_f & _FLAG_C )
			return 1;
		break;

	case 0x03:  /* Z set */
		if ( m_f & _FLAG_Z )
			return 1;
		break;

	case 0x04:  /* Z or C set */
		if ( m_f & ( _FLAG_Z | _FLAG_C ) )
			return 1;
		break;

	case 0x05:  /* CL set */
		if ( m_f & _FLAG_CL )
			return 1;
		break;

	case 0x06:  /* C clear */
		if ( ! ( m_f & _FLAG_C ) )
			return 1;
		break;

	case 0x07:  /* Z clear */
		if ( ! ( m_f & _FLAG_Z ) )
			return 1;
		break;
	}

	return 0;
}


void hcd62121_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);

	save_item( NAME(m_prev_pc) );
	save_item( NAME(m_sp) );
	save_item( NAME(m_ip) );
	save_item( NAME(m_dsize) );
	save_item( NAME(m_cseg) );
	save_item( NAME(m_dseg) );
	save_item( NAME(m_sseg) );
	save_item( NAME(m_f) );
	save_item( NAME(m_lar) );
	save_item( NAME(m_reg) );
	save_item( NAME(m_temp1) );
	save_item( NAME(m_temp2) );

	// Register state for debugger
	state_add( STATE_GENPC,    "curpc",    m_ip ).callimport().callexport().formatstr("%8s");
	state_add( STATE_GENFLAGS, "GENFLAGS", m_f  ).callimport().callexport().formatstr("%12s").noshow();

	state_add( HCD62121_IP,    "IP",    m_ip    ).callimport().callexport().formatstr("%04X");
	state_add( HCD62121_SP,    "SP",    m_sp    ).callimport().callexport().formatstr("%04X");
	state_add( HCD62121_LAR,   "LAR",   m_lar   ).callimport().callexport().formatstr("%04X");
	state_add( HCD62121_CS,    "CS",    m_cseg  ).callimport().callexport().formatstr("%02X");
	state_add( HCD62121_DS,    "DS",    m_dseg  ).callimport().callexport().formatstr("%02X");
	state_add( HCD62121_SS,    "SS",    m_sseg  ).callimport().callexport().formatstr("%02X");
	state_add( HCD62121_DSIZE, "DSIZE", m_dsize ).callimport().callexport().formatstr("%02X");
	state_add( HCD62121_F,     "F",     m_f     ).callimport().callexport().formatstr("%02X");

	state_add( HCD62121_R00, "R00", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R04, "R04", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R08, "R08", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R0C, "R0C", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R10, "R10", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R14, "R14", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R18, "R18", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R1C, "R1C", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R20, "R20", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R24, "R24", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R28, "R28", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R2C, "R2C", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R30, "R30", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R34, "R34", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R38, "R38", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R3C, "R3C", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R40, "R40", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R44, "R44", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R48, "R48", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R4C, "R4C", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R50, "R50", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R54, "R54", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R58, "R58", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R5C, "R5C", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R60, "R60", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R64, "R64", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R68, "R68", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R6C, "R6C", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R70, "R70", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R74, "R74", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R78, "R78", m_reg[0x00] ).callimport().callexport().formatstr("%8s");
	state_add( HCD62121_R7C, "R7C", m_reg[0x00] ).callimport().callexport().formatstr("%8s");

	m_icountptr = &m_icount;
}


void hcd62121_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENPC:
			strprintf(str, "%06X", (m_cseg << 16) | m_ip);
			break;

		case STATE_GENFLAGS:
			strprintf(str, "%s-%s-%s-%c-%c",
				m_f & _FLAG_ZH ? "ZH":"__",
				m_f & _FLAG_CL ? "CL":"__",
				m_f & _FLAG_ZL ? "ZL":"__",
				m_f & _FLAG_C ? 'C':'_',
				m_f & _FLAG_Z ? 'Z':'_'
			);

			break;

		case HCD62121_R00:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x00], m_reg[0x01], m_reg[0x02], m_reg[0x03]);
			break;
		case HCD62121_R04:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x04], m_reg[0x05], m_reg[0x06], m_reg[0x07]);
			break;
		case HCD62121_R08:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x08], m_reg[0x09], m_reg[0x0A], m_reg[0x0B]);
			break;
		case HCD62121_R0C:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x0C], m_reg[0x0D], m_reg[0x0E], m_reg[0x0F]);
			break;
		case HCD62121_R10:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x10], m_reg[0x11], m_reg[0x12], m_reg[0x13]);
			break;
		case HCD62121_R14:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x14], m_reg[0x15], m_reg[0x16], m_reg[0x17]);
			break;
		case HCD62121_R18:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x18], m_reg[0x19], m_reg[0x1A], m_reg[0x1B]);
			break;
		case HCD62121_R1C:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x1C], m_reg[0x1D], m_reg[0x1E], m_reg[0x1F]);
			break;
		case HCD62121_R20:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x20], m_reg[0x21], m_reg[0x22], m_reg[0x23]);
			break;
		case HCD62121_R24:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x24], m_reg[0x25], m_reg[0x26], m_reg[0x27]);
			break;
		case HCD62121_R28:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x28], m_reg[0x29], m_reg[0x2A], m_reg[0x2B]);
			break;
		case HCD62121_R2C:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x2C], m_reg[0x2D], m_reg[0x2E], m_reg[0x2F]);
			break;
		case HCD62121_R30:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x30], m_reg[0x31], m_reg[0x32], m_reg[0x33]);
			break;
		case HCD62121_R34:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x34], m_reg[0x35], m_reg[0x36], m_reg[0x37]);
			break;
		case HCD62121_R38:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x38], m_reg[0x39], m_reg[0x3A], m_reg[0x3B]);
			break;
		case HCD62121_R3C:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x3C], m_reg[0x3D], m_reg[0x3E], m_reg[0x3F]);
			break;
		case HCD62121_R40:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x40], m_reg[0x41], m_reg[0x42], m_reg[0x43]);
			break;
		case HCD62121_R44:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x44], m_reg[0x45], m_reg[0x46], m_reg[0x47]);
			break;
		case HCD62121_R48:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x48], m_reg[0x49], m_reg[0x4A], m_reg[0x4B]);
			break;
		case HCD62121_R4C:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x4C], m_reg[0x4D], m_reg[0x4E], m_reg[0x4F]);
			break;
		case HCD62121_R50:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x50], m_reg[0x51], m_reg[0x52], m_reg[0x53]);
			break;
		case HCD62121_R54:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x54], m_reg[0x55], m_reg[0x56], m_reg[0x57]);
			break;
		case HCD62121_R58:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x58], m_reg[0x59], m_reg[0x5A], m_reg[0x5B]);
			break;
		case HCD62121_R5C:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x5C], m_reg[0x5D], m_reg[0x5E], m_reg[0x5F]);
			break;
		case HCD62121_R60:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x60], m_reg[0x61], m_reg[0x62], m_reg[0x63]);
			break;
		case HCD62121_R64:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x64], m_reg[0x65], m_reg[0x66], m_reg[0x67]);
			break;
		case HCD62121_R68:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x68], m_reg[0x69], m_reg[0x6A], m_reg[0x6B]);
			break;
		case HCD62121_R6C:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x6C], m_reg[0x6D], m_reg[0x6E], m_reg[0x6F]);
			break;
		case HCD62121_R70:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x70], m_reg[0x71], m_reg[0x72], m_reg[0x73]);
			break;
		case HCD62121_R74:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x74], m_reg[0x75], m_reg[0x76], m_reg[0x77]);
			break;
		case HCD62121_R78:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x78], m_reg[0x79], m_reg[0x7A], m_reg[0x7B]);
			break;
		case HCD62121_R7C:
			strprintf(str, "%02X%02X%02X%02X", m_reg[0x7C], m_reg[0x7D], m_reg[0x7E], m_reg[0x7F]);
			break;
	}
}


void hcd62121_cpu_device::device_reset()
{
	m_sp = 0x0000;
	m_ip = 0x0000;
	m_cseg = 0;
	m_dseg = 0;
	m_sseg = 0;
	m_lar = 0;
	m_f = 0;
	m_dsize = 0;

	for(auto & elem : m_reg)
	{
		elem = 0;
	}
}


void hcd62121_cpu_device::execute_run()
{
	do
	{
		UINT32 pc = ( m_cseg << 16 ) | m_ip;
		UINT8 op;

		debugger_instruction_hook(this, pc);
		m_prev_pc = pc;

		op = read_op();

		m_icount -= 4;

		switch ( op )
		{
#include "hcd62121_ops.h"
		};

	} while (m_icount > 0);
}


offs_t hcd62121_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( hcd62121 );
	return CPU_DISASSEMBLE_NAME(hcd62121)(this, buffer, pc, oprom, opram, options);
}
