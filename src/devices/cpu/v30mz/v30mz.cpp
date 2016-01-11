// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Bryan McPhail
/****************************************************************************

    NEC V20/V30/V33 emulator modified to a v30mz emulator

    (Re)Written June-September 2000 by Bryan McPhail (mish@tendril.co.uk) based
    on code by Oliver Bergmann (Raul_Bloodworth@hotmail.com) who based code
    on the i286 emulator by Fabrice Frances which had initial work based on
    David Hedley's pcemu(!).

    This new core features 99% accurate cycle counts for each processor,
    there are still some complex situations where cycle counts are wrong,
    typically where a few instructions have differing counts for odd/even
    source and odd/even destination memory operands.

    Flag settings are also correct for the NEC processors rather than the
    I86 versions.

    Changelist:

    22/02/2003:
        Removed cycle counts from memory accesses - they are certainly wrong,
        and there is already a memory access cycle penalty in the opcodes
        using them.

        Fixed save states.

        Fixed ADJBA/ADJBS/ADJ4A/ADJ4S flags/return values for all situations.
        (Fixes bugs in Geostorm and Thunderblaster)

        Fixed carry flag on NEG (I thought this had been fixed circa Mame 0.58,
        but it seems I never actually submitted the fix).

        Fixed many cycle counts in instructions and bug in cycle count
        macros (odd word cases were testing for odd instruction word address
        not data address).

    Todo!
        Double check cycle timing is 100%.
        Fix memory interface (should be 16 bit).

****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "v30mz.h"


enum SREGS { ES=0, CS, SS, DS };
enum WREGS { AW=0, CW, DW, BW, SP, BP, IX, IY };

#define NEC_NMI_INT_VECTOR 2

enum BREGS {
	AL = NATIVE_ENDIAN_VALUE_LE_BE(0x0, 0x1),
	AH = NATIVE_ENDIAN_VALUE_LE_BE(0x1, 0x0),
	CL = NATIVE_ENDIAN_VALUE_LE_BE(0x2, 0x3),
	CH = NATIVE_ENDIAN_VALUE_LE_BE(0x3, 0x2),
	DL = NATIVE_ENDIAN_VALUE_LE_BE(0x4, 0x5),
	DH = NATIVE_ENDIAN_VALUE_LE_BE(0x5, 0x4),
	BL = NATIVE_ENDIAN_VALUE_LE_BE(0x6, 0x7),
	BH = NATIVE_ENDIAN_VALUE_LE_BE(0x7, 0x6),
	SPL = NATIVE_ENDIAN_VALUE_LE_BE(0x8, 0x9),
	SPH = NATIVE_ENDIAN_VALUE_LE_BE(0x9, 0x8),
	BPL = NATIVE_ENDIAN_VALUE_LE_BE(0xa, 0xb),
	BPH = NATIVE_ENDIAN_VALUE_LE_BE(0xb, 0xa),
	IXL = NATIVE_ENDIAN_VALUE_LE_BE(0xc, 0xd),
	IXH = NATIVE_ENDIAN_VALUE_LE_BE(0xd, 0xc),
	IYL = NATIVE_ENDIAN_VALUE_LE_BE(0xe, 0xf),
	IYH = NATIVE_ENDIAN_VALUE_LE_BE(0xf, 0xe)
};


#define CF      (m_CarryVal!=0)
#define SF      (m_SignVal<0)
#define ZF      (m_ZeroVal==0)
#define PF      m_parity_table[(UINT8)m_ParityVal]
#define AF      (m_AuxVal!=0)
#define OF      (m_OverVal!=0)
#define MD      (m_MF!=0)


/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/


/* The interrupt number of a pending external interrupt pending NMI is 2.   */
/* For INTR interrupts, the level is caught on the bus during an INTA cycle */

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

/***************************************************************************/

const device_type V30MZ = &device_creator<v30mz_cpu_device>;


v30mz_cpu_device::v30mz_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, V30MZ, "V30MZ", tag, owner, clock, "v30mz", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_ip(0)
	, m_TF(0)
	, m_int_vector(0)
	, m_pc(0)
{
	static const BREGS reg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	/* Set up parity lookup table. */
	for (UINT16 i = 0;i < 256; i++)
	{
		UINT16 c = 0;
		for (UINT16 j = i; j > 0; j >>= 1)
		{
			if (j & 1) c++;
		}
		m_parity_table[i] = !(c & 1);
	}

	for (UINT16 i = 0; i < 256; i++)
	{
		m_Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		m_Mod_RM.reg.w[i] = (WREGS) ( (i & 0x38) >> 3) ;
	}

	for (UINT16 i = 0xc0; i < 0x100; i++)
	{
		m_Mod_RM.RM.w[i] = (WREGS)( i & 7 );
		m_Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
	}

	memset(&m_regs, 0x00, sizeof(m_regs));
}


void v30mz_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	save_item(NAME(m_regs.w));
	save_item(NAME(m_sregs));
	save_item(NAME(m_ip));
	save_item(NAME(m_TF));
	save_item(NAME(m_IF));
	save_item(NAME(m_DF));
	save_item(NAME(m_MF));
	save_item(NAME(m_SignVal));
	save_item(NAME(m_int_vector));
	save_item(NAME(m_pending_irq));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_AuxVal));
	save_item(NAME(m_OverVal));
	save_item(NAME(m_ZeroVal));
	save_item(NAME(m_CarryVal));
	save_item(NAME(m_ParityVal));
	save_item(NAME(m_seg_prefix));
	save_item(NAME(m_seg_prefix_next));

	// Register state for debugger
//  state_add( NEC_PC, "PC", m_PC ).callimport().callexport().formatstr("%04X");
	state_add( NEC_IP, "IP", m_ip         ).callimport().callexport().formatstr("%04X");
	state_add( NEC_SP, "SP", m_regs.w[SP] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_AW, "AW", m_regs.w[AW] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_CW, "CW", m_regs.w[CS] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_DW, "DW", m_regs.w[DW] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_BW, "BW", m_regs.w[BW] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_BP, "BP", m_regs.w[BP] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_IX, "IX", m_regs.w[IX] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_IY, "IY", m_regs.w[IY] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_ES, "ES", m_sregs[ES] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_CS, "CS", m_sregs[CS] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_SS, "SS", m_sregs[SS] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_DS, "DS", m_sregs[DS] ).callimport().callexport().formatstr("%04X");
	state_add( NEC_VECTOR, "V", m_int_vector).callimport().callexport().formatstr("%02X");

	state_add(STATE_GENPC, "curpc", m_pc).callimport().callexport().formatstr("%05X");
	state_add(STATE_GENFLAGS, "GENFLAGS", m_TF).callimport().callexport().formatstr("%16s").noshow();

	m_icountptr = &m_icount;
}


void v30mz_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENPC:
			strprintf(str, "%08X", ( m_sregs[CS] << 4 ) + m_ip);
			break;

		case STATE_GENFLAGS:
			{
				UINT16 flags = CompressFlags();
				strprintf(str, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
					flags & 0x8000 ? 'M':'.',
					flags & 0x4000 ? '?':'.',
					flags & 0x2000 ? '?':'.',
					flags & 0x1000 ? '?':'.',
					flags & 0x0800 ? 'O':'.',
					flags & 0x0400 ? 'D':'.',
					flags & 0x0200 ? 'I':'.',
					flags & 0x0100 ? 'T':'.',
					flags & 0x0080 ? 'S':'.',
					flags & 0x0040 ? 'Z':'.',
					flags & 0x0020 ? '?':'.',
					flags & 0x0010 ? 'A':'.',
					flags & 0x0008 ? '?':'.',
					flags & 0x0004 ? 'P':'.',
					flags & 0x0002 ? 'N':'.',
					flags & 0x0001 ? 'C':'.');
			}
			break;
	}
}


void v30mz_cpu_device::device_reset()
{
	m_ZeroVal = 1;
	m_ParityVal = 1;
	m_regs.w[AW] = 0;
	m_regs.w[CW] = 0;
	m_regs.w[DW] = 0;
	m_regs.w[BW] = 0;
	m_regs.w[SP] = 0;
	m_regs.w[BP] = 0;
	m_regs.w[IX] = 0;
	m_regs.w[IY] = 0;
	m_sregs[ES] = 0;
	m_sregs[CS] = 0xffff;
	m_sregs[SS] = 0;
	m_sregs[DS] = 0;
	m_ip = 0;
	m_SignVal = 0;
	m_AuxVal = 0;
	m_OverVal = 0;
	m_CarryVal = 0;
	m_TF = 0;
	m_IF = 0;
	m_DF = 0;
	m_MF = 1;
	m_int_vector = 0;
	m_pending_irq = 0;
	m_nmi_state = 0;
	m_irq_state = 0;
	m_no_interrupt = 0;
	m_fire_trap = 0;
	m_prefix_base = 0;
	m_seg_prefix = false;
	m_seg_prefix_next = false;
	m_ea = 0;
	m_eo = 0;
	m_e16 = 0;
	m_modrm = 0;
	m_dst = 0;
	m_src = 0;
}


inline UINT32 v30mz_cpu_device::pc()
{
	m_pc = ( m_sregs[CS] << 4 ) + m_ip;
	return m_pc;
}


inline UINT8 v30mz_cpu_device::read_byte(UINT32 addr)
{
	return m_program->read_byte(addr);
}


inline UINT16 v30mz_cpu_device::read_word(UINT32 addr)
{
	return m_program->read_byte(addr) | ( m_program->read_byte(addr+1) << 8 );
}


inline void v30mz_cpu_device::write_byte(UINT32 addr, UINT8 data)
{
	m_program->write_byte(addr, data);
}


inline void v30mz_cpu_device::write_word(UINT32 addr, UINT16 data)
{
	m_program->write_byte( addr, data & 0xff );
	m_program->write_byte( addr + 1, data >> 8 );
}


inline UINT8 v30mz_cpu_device::read_port(UINT16 port)
{
	return m_io->read_byte(port);
}


inline void v30mz_cpu_device::write_port(UINT16 port, UINT8 data)
{
	m_io->write_byte(port, data);
}


inline UINT8 v30mz_cpu_device::fetch_op()
{
	UINT8 data = m_direct->read_byte( pc() );
	m_ip++;
	return data;
}


inline UINT8 v30mz_cpu_device::fetch()
{
	UINT8 data = m_direct->read_byte( pc() );
	m_ip++;
	return data;
}


inline UINT16 v30mz_cpu_device::fetch_word()
{
	UINT16 data = fetch();
	data |= ( fetch() << 8 );
	return data;
}


inline UINT8 v30mz_cpu_device::repx_op()
{
	UINT8 next = fetch_op();
	bool seg_prefix = false;
	int seg = 0;

	switch (next)
	{
	case 0x26:
		seg_prefix = true;
		seg = ES;
		break;
	case 0x2e:
		seg_prefix = true;
		seg = CS;
		break;
	case 0x36:
		seg_prefix = true;
		seg = SS;
		break;
	case 0x3e:
		seg_prefix = true;
		seg = DS;
		break;
	}

	if ( seg_prefix )
	{
		m_seg_prefix = true;
		m_seg_prefix_next = true;
		m_prefix_base = m_sregs[seg] << 4;
		next = fetch_op();
		CLK(2);
	}

	return next;
}


inline void v30mz_cpu_device::CLK(UINT32 cycles)
{
	m_icount -= cycles;
}


inline void v30mz_cpu_device::CLKM(UINT32 cycles_reg, UINT32 cycles_mem)
{
	m_icount -= ( m_modrm >= 0xc0 ) ? cycles_reg : cycles_mem;
}


inline UINT32 v30mz_cpu_device::default_base(int seg)
{
	if ( m_seg_prefix && (seg==DS || seg==SS) )
	{
		return m_prefix_base;
	}
	else
	{
		return m_sregs[seg] << 4;
	}
}


inline UINT32 v30mz_cpu_device::get_ea()
{
	switch( m_modrm & 0xc7 )
	{
	case 0x00:
		m_eo = m_regs.w[BW] + m_regs.w[IX];
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x01:
		m_eo = m_regs.w[BW] + m_regs.w[IY];
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x02:
		m_eo = m_regs.w[BP] + m_regs.w[IX];
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x03:
		m_eo = m_regs.w[BP] + m_regs.w[IY];
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x04:
		m_eo = m_regs.w[IX];
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x05:
		m_eo = m_regs.w[IY];
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x06:
		m_eo = fetch_word();
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x07:
		m_eo = m_regs.w[BW];
		m_ea = default_base(DS) + m_eo;
		break;

	case 0x40:
		m_eo = m_regs.w[BW] + m_regs.w[IX] + (INT8)fetch();
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x41:
		m_eo = m_regs.w[BW] + m_regs.w[IY] + (INT8)fetch();
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x42:
		m_eo = m_regs.w[BP] + m_regs.w[IX] + (INT8)fetch();
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x43:
		m_eo = m_regs.w[BP] + m_regs.w[IY] + (INT8)fetch();
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x44:
		m_eo = m_regs.w[IX] + (INT8)fetch();
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x45:
		m_eo = m_regs.w[IY] + (INT8)fetch();
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x46:
		m_eo = m_regs.w[BP] + (INT8)fetch();
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x47:
		m_eo = m_regs.w[BW] + (INT8)fetch();
		m_ea = default_base(DS) + m_eo;
		break;

	case 0x80:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BW] + m_regs.w[IX] + (INT16)m_e16;
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x81:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BW] + m_regs.w[IY] + (INT16)m_e16;
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x82:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BP] + m_regs.w[IX] + (INT16)m_e16;
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x83:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BP] + m_regs.w[IY] + (INT16)m_e16;
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x84:
		m_e16 = fetch_word();
		m_eo = m_regs.w[IX] + (INT16)m_e16;
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x85:
		m_e16 = fetch_word();
		m_eo = m_regs.w[IY] + (INT16)m_e16;
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x86:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BP] + (INT16)m_e16;
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x87:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BW] + (INT16)m_e16;
		m_ea = default_base(DS) + m_eo;
		break;
	}

	return m_ea;
}


inline void v30mz_cpu_device::PutbackRMByte(UINT8 data)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ] = data;
	}
	else
	{
		write_byte( m_ea, data );
	}
}


inline void v30mz_cpu_device::PutbackRMWord(UINT16 data)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ] = data;
	}
	else
	{
		write_word( m_ea, data );
	}
}

inline void v30mz_cpu_device::PutImmRMWord()
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ] = fetch_word();
	}
	else
	{
		UINT32 addr = get_ea();
		write_word( addr, fetch_word() );
	}
}

inline void v30mz_cpu_device::PutRMWord(UINT16 val)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ] = val;
	}
	else
	{
		write_word( get_ea(), val );
	}
}


inline void v30mz_cpu_device::PutRMByte(UINT8 val)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ] = val;
	}
	else
	{
		write_byte( get_ea(), val );
	}
}


inline void v30mz_cpu_device::PutImmRMByte()
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ] = fetch();
	}
	else
	{
		UINT32 addr = get_ea();
		write_byte( addr, fetch() );
	}
}


inline void v30mz_cpu_device::DEF_br8()
{
	m_modrm = fetch();
	m_src = RegByte();
	m_dst = GetRMByte();
}


inline void v30mz_cpu_device::DEF_wr16()
{
	m_modrm = fetch();
	m_src = RegWord();
	m_dst = GetRMWord();
}


inline void v30mz_cpu_device::DEF_r8b()
{
	m_modrm = fetch();
	m_dst = RegByte();
	m_src = GetRMByte();
}


inline void v30mz_cpu_device::DEF_r16w()
{
	m_modrm = fetch();
	m_dst = RegWord();
	m_src = GetRMWord();
}


inline void v30mz_cpu_device::DEF_ald8()
{
	m_src = fetch();
	m_dst = m_regs.b[AL];
}


inline void v30mz_cpu_device::DEF_axd16()
{
	m_src = fetch_word();
	m_dst = m_regs.w[AW];
}



inline void v30mz_cpu_device::RegByte(UINT8 data)
{
	m_regs.b[ m_Mod_RM.reg.b[ m_modrm ] ] = data;
}


inline void v30mz_cpu_device::RegWord(UINT16 data)
{
	m_regs.w[ m_Mod_RM.reg.w[ m_modrm ] ] = data;
}


inline UINT8 v30mz_cpu_device::RegByte()
{
	return m_regs.b[ m_Mod_RM.reg.b[ m_modrm ] ];
}


inline UINT16 v30mz_cpu_device::RegWord()
{
	return m_regs.w[ m_Mod_RM.reg.w[ m_modrm ] ];
}


inline UINT16 v30mz_cpu_device::GetRMWord()
{
	if ( m_modrm >= 0xc0 )
	{
		return m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ];
	}
	else
	{
		return read_word( get_ea() );
	}
}


inline UINT16 v30mz_cpu_device::GetnextRMWord()
{
	UINT32 addr = ( m_ea & 0xf0000 ) | ( ( m_ea + 2 ) & 0xffff );

	return read_word( addr );
}


inline UINT8 v30mz_cpu_device::GetRMByte()
{
	if ( m_modrm >= 0xc0 )
	{
		return m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ];
	}
	else
	{
		return read_byte( get_ea() );
	}
}


inline void v30mz_cpu_device::PutMemB(int seg, UINT16 offset, UINT8 data)
{
	write_byte( default_base( seg ) + offset, data);
}


inline void v30mz_cpu_device::PutMemW(int seg, UINT16 offset, UINT16 data)
{
	PutMemB( seg, offset, data & 0xff);
	PutMemB( seg, offset+1, data >> 8);
}


inline UINT8 v30mz_cpu_device::GetMemB(int seg, UINT16 offset)
{
	return read_byte( default_base(seg) + offset );
}


inline UINT16 v30mz_cpu_device::GetMemW(int seg, UINT16 offset)
{
	return GetMemB(seg, offset) | ( GetMemB(seg, offset + 1) << 8 );
}


// Setting flags

inline void v30mz_cpu_device::set_CFB(UINT32 x)
{
	m_CarryVal = x & 0x100;
}

inline void v30mz_cpu_device::set_CFW(UINT32 x)
{
	m_CarryVal = x & 0x10000;
}

inline void v30mz_cpu_device::set_AF(UINT32 x,UINT32 y,UINT32 z)
{
	m_AuxVal = (x ^ (y ^ z)) & 0x10;
}

inline void v30mz_cpu_device::set_SF(UINT32 x)
{
	m_SignVal = x;
}

inline void v30mz_cpu_device::set_ZF(UINT32 x)
{
	m_ZeroVal = x;
}

inline void v30mz_cpu_device::set_PF(UINT32 x)
{
	m_ParityVal = x;
}

inline void v30mz_cpu_device::set_SZPF_Byte(UINT32 x)
{
	m_SignVal = m_ZeroVal = m_ParityVal = (INT8)x;
}

inline void v30mz_cpu_device::set_SZPF_Word(UINT32 x)
{
	m_SignVal = m_ZeroVal = m_ParityVal = (INT16)x;
}

inline void v30mz_cpu_device::set_OFW_Add(UINT32 x,UINT32 y,UINT32 z)
{
	m_OverVal = (x ^ y) & (x ^ z) & 0x8000;
}

inline void v30mz_cpu_device::set_OFB_Add(UINT32 x,UINT32 y,UINT32 z)
{
	m_OverVal = (x ^ y) & (x ^ z) & 0x80;
}

inline void v30mz_cpu_device::set_OFW_Sub(UINT32 x,UINT32 y,UINT32 z)
{
	m_OverVal = (z ^ y) & (z ^ x) & 0x8000;
}

inline void v30mz_cpu_device::set_OFB_Sub(UINT32 x,UINT32 y,UINT32 z)
{
	m_OverVal = (z ^ y) & (z ^ x) & 0x80;
}


inline UINT16 v30mz_cpu_device::CompressFlags() const
{
	return (CF ? 1 : 0)
		| (PF ? 4 : 0)
		| (AF ? 0x10 : 0)
		| (ZF ? 0x40 : 0)
		| (SF ? 0x80 : 0)
		| (m_TF << 8)
		| (m_IF << 9)
		| (m_DF << 10)
		| (OF << 11)
		| (MD << 15);
}

inline void v30mz_cpu_device::ExpandFlags(UINT16 f)
{
	m_CarryVal = (f) & 1;
	m_ParityVal = !((f) & 4);
	m_AuxVal = (f) & 16;
	m_ZeroVal = !((f) & 64);
	m_SignVal = (f) & 128 ? -1 : 0;
	m_TF = ((f) & 256) == 256;
	m_IF = ((f) & 512) == 512;
	m_DF = ((f) & 1024) == 1024;
	m_OverVal = (f) & 2048;
	m_MF = ((f) & 0x8000) == 0x8000;
}

inline void v30mz_cpu_device::i_insb()
{
	PutMemB( ES, m_regs.w[IY], read_port( m_regs.w[DW] ) );
	m_regs.w[IY] += -2 * m_DF + 1;
	CLK(6);
}

inline void v30mz_cpu_device::i_insw()
{
	PutMemB( ES, m_regs.w[IY], read_port( m_regs.w[DW] ) );
	PutMemB( ES, (m_regs.w[IY] + 1) & 0xffff, read_port((m_regs.w[DW]+1)&0xffff));
	m_regs.w[IY] += -4 * m_DF + 2;
	CLK(6);
}

inline void v30mz_cpu_device::i_outsb()
{
	write_port( m_regs.w[DW], GetMemB( DS, m_regs.w[IX] ) );
	m_regs.w[IX] += -2 * m_DF + 1;
	CLK(7);
}

inline void v30mz_cpu_device::i_outsw()
{
	write_port( m_regs.w[DW], GetMemB( DS, m_regs.w[IX] ) );
	write_port( (m_regs.w[DW]+1)&0xffff, GetMemB( DS, (m_regs.w[IX]+1)&0xffff ) );
	m_regs.w[IX] += -4 * m_DF + 2;
	CLK(7);
}

inline void v30mz_cpu_device::i_movsb()
{
	UINT8 tmp = GetMemB( DS, m_regs.w[IX] );
	PutMemB( ES, m_regs.w[IY], tmp);
	m_regs.w[IY] += -2 * m_DF + 1;
	m_regs.w[IX] += -2 * m_DF + 1;
	CLK(5);
}

inline void v30mz_cpu_device::i_movsw()
{
	UINT16 tmp = GetMemW( DS, m_regs.w[IX] );
	PutMemW( ES, m_regs.w[IY], tmp );
	m_regs.w[IY] += -4 * m_DF + 2;
	m_regs.w[IX] += -4 * m_DF + 2;
	CLK(5);
}

inline void v30mz_cpu_device::i_cmpsb()
{
	m_src = GetMemB( ES, m_regs.w[IY] );
	m_dst = GetMemB( DS, m_regs.w[IX] );
	SUBB();
	m_regs.w[IY] += -2 * m_DF + 1;
	m_regs.w[IX] += -2 * m_DF + 1;
	CLK(6);
}

inline void v30mz_cpu_device::i_cmpsw()
{
	m_src = GetMemW( ES, m_regs.w[IY] );
	m_dst = GetMemW( DS, m_regs.w[IX] );
	SUBW();
	m_regs.w[IY] += -4 * m_DF + 2;
	m_regs.w[IX] += -4 * m_DF + 2;
	CLK(6);
}

inline void v30mz_cpu_device::i_stosb()
{
	PutMemB( ES, m_regs.w[IY], m_regs.b[AL] );
	m_regs.w[IY] += -2 * m_DF + 1;
	CLK(3);
}

inline void v30mz_cpu_device::i_stosw()
{
	PutMemW( ES, m_regs.w[IY], m_regs.w[AW] );
	m_regs.w[IY] += -4 * m_DF + 2;
	CLK(3);
}

inline void v30mz_cpu_device::i_lodsb()
{
	m_regs.b[AL] = GetMemB( DS, m_regs.w[IX] );
	m_regs.w[IX] += -2 * m_DF + 1;
	CLK(3);
}

inline void v30mz_cpu_device::i_lodsw()
{
	m_regs.w[AW] = GetMemW( DS, m_regs.w[IX] );
	m_regs.w[IX] += -4 * m_DF + 2;
	CLK(3);
}

inline void v30mz_cpu_device::i_scasb()
{
	m_src = GetMemB( ES, m_regs.w[IY] );
	m_dst = m_regs.b[AL];
	SUBB();
	m_regs.w[IY] += -2 * m_DF + 1;
	CLK(4);
}

inline void v30mz_cpu_device::i_scasw()
{
	m_src = GetMemW( ES, m_regs.w[IY] );
	m_dst = m_regs.w[AW];
	SUBW();
	m_regs.w[IY] += -4 * m_DF + 2;
	CLK(4);
}


inline void v30mz_cpu_device::i_popf()
{
	UINT32 tmp = POP();

	ExpandFlags(tmp);
	CLK(3);
	if (m_TF)
	{
		m_fire_trap = 1;
	}
}


inline void v30mz_cpu_device::ADDB()
{
	UINT32 res = m_dst + m_src;

	set_CFB(res);
	set_OFB_Add(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Byte(res);
	m_dst = res & 0xff;
}


inline void v30mz_cpu_device::ADDW()
{
	UINT32 res = m_dst + m_src;

	set_CFW(res);
	set_OFW_Add(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Word(res);
	m_dst = res & 0xffff;
}


inline void v30mz_cpu_device::SUBB()
{
	UINT32 res = m_dst - m_src;

	set_CFB(res);
	set_OFB_Sub(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Byte(res);
	m_dst = res & 0xff;
}


inline void v30mz_cpu_device::SUBW()
{
	UINT32 res = m_dst - m_src;

	set_CFW(res);
	set_OFW_Sub(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Word(res);
	m_dst = res & 0xffff;
}


inline void v30mz_cpu_device::ORB()
{
	m_dst |= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Byte(m_dst);
}


inline void v30mz_cpu_device::ORW()
{
	m_dst |= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Word(m_dst);
}


inline void v30mz_cpu_device::ANDB()
{
	m_dst &= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Byte(m_dst);
}


inline void v30mz_cpu_device::ANDW()
{
	m_dst &= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Word(m_dst);
}


inline void v30mz_cpu_device::XORB()
{
	m_dst ^= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Byte(m_dst);
}


inline void v30mz_cpu_device::XORW()
{
	m_dst ^= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Word(m_dst);
}


inline void v30mz_cpu_device::ROL_BYTE()
{
	m_CarryVal = m_dst & 0x80;
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
}

inline void v30mz_cpu_device::ROL_WORD()
{
	m_CarryVal = m_dst & 0x8000;
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
}

inline void v30mz_cpu_device::ROR_BYTE()
{
	m_CarryVal = m_dst & 0x1;
	m_dst = (m_dst >> 1) | (CF ? 0x80 : 0x00);
}

inline void v30mz_cpu_device::ROR_WORD()
{
	m_CarryVal = m_dst & 0x1;
	m_dst = (m_dst >> 1) + (CF ? 0x8000 : 0x0000);
}

inline void v30mz_cpu_device::ROLC_BYTE()
{
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
	set_CFB(m_dst);
}

inline void v30mz_cpu_device::ROLC_WORD()
{
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
	set_CFW(m_dst);
}

inline void v30mz_cpu_device::RORC_BYTE()
{
	m_dst |= ( CF ? 0x100 : 0x00);
	m_CarryVal = m_dst & 0x01;
	m_dst >>= 1;
}

inline void v30mz_cpu_device::RORC_WORD()
{
	m_dst |= ( CF ? 0x10000 : 0);
	m_CarryVal = m_dst & 0x01;
	m_dst >>= 1;
}

inline void v30mz_cpu_device::SHL_BYTE(UINT8 c)
{
	m_icount -= c;
	m_dst <<= c;
	set_CFB(m_dst);
	set_SZPF_Byte(m_dst);
	PutbackRMByte(m_dst);
}

inline void v30mz_cpu_device::SHL_WORD(UINT8 c)
{
	m_icount -= c;
	m_dst <<= c;
	set_CFW(m_dst);
	set_SZPF_Word(m_dst);
	PutbackRMWord(m_dst);
}

inline void v30mz_cpu_device::SHR_BYTE(UINT8 c)
{
	m_icount -= c;
	m_dst >>= c-1;
	m_CarryVal = m_dst & 0x1;
	m_dst >>= 1;
	set_SZPF_Byte(m_dst);
	PutbackRMByte(m_dst);
}

inline void v30mz_cpu_device::SHR_WORD(UINT8 c)
{
	m_icount -= c;
	m_dst >>= c-1;
	m_CarryVal = m_dst & 0x1;
	m_dst >>= 1;
	set_SZPF_Word(m_dst);
	PutbackRMWord(m_dst);
}

inline void v30mz_cpu_device::SHRA_BYTE(UINT8 c)
{
	m_icount -= c;
	m_dst = ((INT8)m_dst) >> (c-1);
	m_CarryVal = m_dst & 0x1;
	m_dst = m_dst >> 1;
	set_SZPF_Byte(m_dst);
	PutbackRMByte(m_dst);
}

inline void v30mz_cpu_device::SHRA_WORD(UINT8 c)
{
	m_icount -= c;
	m_dst = ((INT16)m_dst) >> (c-1);
	m_CarryVal = m_dst & 0x1;
	m_dst = m_dst >> 1;
	set_SZPF_Word(m_dst);
	PutbackRMWord(m_dst);
}


inline void v30mz_cpu_device::XchgAWReg(UINT8 reg)
{
	UINT16 tmp = m_regs.w[reg];

	m_regs.w[reg] = m_regs.w[AW];
	m_regs.w[AW] = tmp;
}


inline void v30mz_cpu_device::IncWordReg(UINT8 reg)
{
	UINT32 tmp = m_regs.w[reg];
	UINT32 tmp1 = tmp+1;

	m_OverVal = (tmp == 0x7fff);
	set_AF(tmp1,tmp,1);
	set_SZPF_Word(tmp1);
	m_regs.w[reg] = tmp1;
}


inline void v30mz_cpu_device::DecWordReg(UINT8 reg)
{
	UINT32 tmp = m_regs.w[reg];
	UINT32 tmp1 = tmp-1;

	m_OverVal = (tmp == 0x8000);
	set_AF(tmp1,tmp,1);
	set_SZPF_Word(tmp1);
	m_regs.w[reg] = tmp1;
}


inline void v30mz_cpu_device::PUSH(UINT16 data)
{
	m_regs.w[SP] -= 2;
	write_word( ( m_sregs[SS] << 4 ) + m_regs.w[SP], data );
}


inline UINT16 v30mz_cpu_device::POP()
{
	UINT16 data = read_word( ( m_sregs[SS] << 4 ) + m_regs.w[SP] );

	m_regs.w[SP] += 2;
	return data;
}


inline void v30mz_cpu_device::JMP(bool cond)
{
	int rel  = (int)((INT8)fetch());

	if (cond)
	{
		m_ip += rel;
		CLK(9);
	}
	CLK(1);
}


inline void v30mz_cpu_device::ADJ4(INT8 param1,INT8 param2)
{
	if (AF || ((m_regs.b[AL] & 0xf) > 9))
	{
		UINT16 tmp;
		tmp = m_regs.b[AL] + param1;
		m_regs.b[AL] = tmp;
		m_AuxVal = 1;
		m_CarryVal |= tmp & 0x100;
	}
	if (CF || (m_regs.b[AL]>0x9f))
	{
		m_regs.b[AL] += param2;
		m_CarryVal = 1;
	}
	set_SZPF_Byte(m_regs.b[AL]);
}


inline void v30mz_cpu_device::ADJB(INT8 param1, INT8 param2)
{
	if (AF || ((m_regs.b[AL] & 0xf) > 9))
	{
		m_regs.b[AL] += param1;
		m_regs.b[AH] += param2;
		m_AuxVal = 1;
		m_CarryVal = 1;
	}
	else
	{
		m_AuxVal = 0;
		m_CarryVal = 0;
	}
	m_regs.b[AL] &= 0x0F;
}


void v30mz_cpu_device::interrupt(int int_num)
{
	PUSH( CompressFlags() );
	CLK(2);
	m_TF = m_IF = 0;

	if (int_num == -1)
	{
		int_num = standard_irq_callback(0);

		m_irq_state = CLEAR_LINE;
		m_pending_irq &= ~INT_IRQ;
	}

	UINT16 dest_off = read_word( int_num * 4 + 0 );
	UINT16 dest_seg = read_word( int_num * 4 + 2 );

	PUSH(m_sregs[CS]);
	PUSH(m_ip);
	m_ip = dest_off;
	m_sregs[CS] = dest_seg;
}


void v30mz_cpu_device::execute_set_input( int inptnum, int state )
{
	if (inptnum == INPUT_LINE_NMI)
	{
		if ( m_nmi_state == state )
		{
			return;
		}
		m_nmi_state = state;
		if (state != CLEAR_LINE)
		{
			m_pending_irq |= NMI_IRQ;
		}
	}
	else
	{
		m_irq_state = state;
		if (state == CLEAR_LINE)
		{
			m_pending_irq &= ~INT_IRQ;
		}
		else
		{
			m_pending_irq |= INT_IRQ;
		}
	}
}


offs_t v30mz_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( nec );
	return CPU_DISASSEMBLE_NAME(nec)(this, buffer, pc, oprom, opram, options);
}


void v30mz_cpu_device::execute_run()
{
	while(m_icount > 0 )
	{
		if ( m_seg_prefix_next )
		{
			m_seg_prefix = true;
			m_seg_prefix_next = false;
		}
		else
		{
			m_seg_prefix = false;

				/* Dispatch IRQ */
			if ( m_pending_irq && m_no_interrupt == 0 )
			{
				if ( m_pending_irq & NMI_IRQ )
				{
					interrupt(NEC_NMI_INT_VECTOR);
					m_pending_irq &= ~NMI_IRQ;
				}
				else if ( m_IF )
				{
					/* the actual vector is retrieved after pushing flags */
					/* and clearing the IF */
					interrupt(-1);
				}
			}

			/* No interrupt allowed between last instruction and this one */
			if ( m_no_interrupt )
			{
				m_no_interrupt--;
			}

			/* trap should allow one instruction to be executed */
			if ( m_fire_trap )
			{
				if ( m_fire_trap >= 2 )
				{
					interrupt(1);
					m_fire_trap = 0;
				}
				else
				{
					m_fire_trap++;
				}
			}
		}

		debugger_instruction_hook( this, pc() );

		UINT8 op = fetch_op();

		switch(op)
		{
			case 0x00: // i_add_br8
				DEF_br8();
				ADDB();
				PutbackRMByte(m_dst);
				CLKM(1,3);
				break;

			case 0x01: // i_add_wr16
				DEF_wr16();
				ADDW();
				PutbackRMWord(m_dst);
				CLKM(1,3);
				break;

			case 0x02: // i_add_r8b
				DEF_r8b();
				ADDB();
				RegByte(m_dst);
				CLKM(1,2);
				break;

			case 0x03: // i_add_r16w
				DEF_r16w();
				ADDW();
				RegWord(m_dst);
				CLKM(1,2);
				break;

			case 0x04: // i_add_ald8
				DEF_ald8();
				ADDB();
				m_regs.b[AL] = m_dst;
				CLK(1);
				break;

			case 0x05: // i_add_axd16
				DEF_axd16();
				ADDW();
				m_regs.w[AW] = m_dst;
				CLK(1);
				break;

			case 0x06: // i_push_es
				PUSH(m_sregs[ES]);
				CLK(2);
				break;

			case 0x07: // i_pop_es
				m_sregs[ES] = POP();
				CLK(3);
				break;

			case 0x08: // i_or_br8
				DEF_br8();
				ORB();
				PutbackRMByte(m_dst);
				CLKM(1,3);
				break;

			case 0x09: // i_or_wr16
				DEF_wr16();
				ORW();
				PutbackRMWord(m_dst);
				CLKM(1,3);
				break;

			case 0x0a: // i_or_r8b
				DEF_r8b();
				ORB();
				RegByte(m_dst);
				CLKM(1,2);
				break;

			case 0x0b: // i_or_r16w
				DEF_r16w();
				ORW();
				RegWord(m_dst);
				CLKM(1,2);
				break;

			case 0x0c: // i_or_ald8
				DEF_ald8();
				ORB();
				m_regs.b[AL] = m_dst;
				CLK(1);
				break;

			case 0x0d: // i_or_axd16
				DEF_axd16();
				ORW();
				m_regs.w[AW] = m_dst;
				CLK(1);
				break;

			case 0x0e: // i_push_cs
				PUSH(m_sregs[CS]);
				CLK(2);
				break;

			case 0x0f: // i_pre_nec
				{
					UINT32 tmp, tmp2;

					switch ( fetch() )
					{
					case 0x10:  /* Test */
						m_modrm = fetch();
						tmp = GetRMByte();
						tmp2 = m_regs.b[CL] & 0x7;
						m_ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;
						m_CarryVal = m_OverVal = 0;
						break;
					case 0x11:  /* Test */
						m_modrm = fetch();
						tmp = GetRMWord();
						tmp2 = m_regs.b[CL] & 0xf;
						m_ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;
						m_CarryVal = m_OverVal = 0;
						break;
					case 0x12:  /* Clr */
						m_modrm = fetch();
						tmp = GetRMByte();
						tmp2 = m_regs.b[CL] & 0x7;
						tmp &= ~(1<<tmp2);
						PutbackRMByte(tmp);
						break;
					case 0x13:  /* Clr */
						m_modrm = fetch();
						tmp = GetRMWord();
						tmp2 = m_regs.b[CL] & 0xf;
						tmp &= ~(1<<tmp2);
						PutbackRMWord(tmp);
						break;
					case 0x14:  /* Set */
						m_modrm = fetch();
						tmp = GetRMByte();
						tmp2 = m_regs.b[CL] & 0x7;
						tmp |= (1<<tmp2);
						PutbackRMByte(tmp);
						break;
					case 0x15:  /* Set */
						m_modrm = fetch();
						tmp = GetRMWord();
						tmp2 = m_regs.b[CL] & 0xf;
						tmp |= (1<<tmp2);
						PutbackRMWord(tmp);
						break;
					case 0x16:  /* Not */
						m_modrm = fetch();
						tmp = GetRMByte();
						tmp2 = m_regs.b[CL] & 0x7;
						tmp ^= (1 << tmp2);
						PutbackRMByte(tmp);
						break;
					case 0x17:  /* Not */
						m_modrm = fetch();
						tmp = GetRMWord();
						tmp2 = m_regs.b[CL] & 0xf;
						tmp ^= (1 << tmp2);
						PutbackRMWord(tmp);
						break;

					case 0x18:  /* Test */
						m_modrm = fetch();
						tmp = GetRMByte();
						tmp2 = fetch() & 0x7;
						m_ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;
						m_CarryVal = m_OverVal = 0;
						break;
					case 0x19:  /* Test */
						m_modrm = fetch();
						tmp = GetRMWord();
						tmp2 = fetch() & 0xf;
						m_ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0;
						m_CarryVal = m_OverVal = 0;
						break;
					case 0x1a:  /* Clr */
						m_modrm = fetch();
						tmp = GetRMByte();
						tmp2 = fetch() & 0x7;
						tmp &= ~(1<<tmp2);
						PutbackRMByte(tmp);
						break;
					case 0x1b:  /* Clr */
						m_modrm = fetch();
						tmp = GetRMWord();
						tmp2 = fetch() & 0xf;
						tmp &= ~(1<<tmp2);
						PutbackRMWord(tmp);
						break;
					case 0x1c:  /* Set */
						m_modrm = fetch();
						tmp = GetRMByte();
						tmp2 = fetch() & 0x7;
						tmp |= (1<<tmp2);
						PutbackRMByte(tmp);
						break;
					case 0x1d:  /* Set */
						m_modrm = fetch();
						tmp = GetRMWord();
						tmp2 = fetch() & 0xf;
						tmp |= (1<<tmp2);
						PutbackRMWord(tmp);
						break;
					case 0x1e:  /* Not */
						m_modrm = fetch();
						tmp = GetRMByte();
						tmp2 = fetch() & 0x7;
						tmp ^= (1 << tmp2);
						PutbackRMByte(tmp);
						break;
					case 0x1f:  /* Not */
						m_modrm = fetch();
						tmp = GetRMWord();
						tmp2 = fetch() & 0xf;
						tmp ^= (1 << tmp2);
						PutbackRMWord(tmp);
						break;

					case 0x20:
						{
							int count = (m_regs.b[CL]+1)/2;
							UINT16 di = m_regs.w[IY];
							UINT16 si = m_regs.w[IX];
							if (m_seg_prefix)
							{
								logerror("%s: %06x: Warning: seg_prefix defined for add4s\n", tag(), pc());
							}
							m_ZeroVal = m_CarryVal = 0;
							for (int i=0;i<count;i++)
							{
								CLK(19);
								tmp = GetMemB(DS, si);
								tmp2 = GetMemB(ES, di);
								int v1 = (tmp>>4)*10 + (tmp&0xf);
								int v2 = (tmp2>>4)*10 + (tmp2&0xf);
								int result = v1 + v2 + m_CarryVal;
								m_CarryVal = result > 99 ? 1 : 0;
								result = result % 100;
								v1 = ((result/10)<<4) | (result % 10);
								PutMemB(ES, di,v1);
								if (v1)
								{
									m_ZeroVal = 1;
								}
								si++;
								di++;
							}
						}
						break;
					case 0x22:
						{
							int count = (m_regs.b[CL]+1)/2;
							UINT16 di = m_regs.w[IY];
							UINT16 si = m_regs.w[IX];
							if (m_seg_prefix)
							{
								logerror("%s: %06x: Warning: seg_prefix defined for sub4s\n", tag(), pc());
							}
							m_ZeroVal = m_CarryVal = 0;
							for (int i=0; i<count; i++)
							{
								int result;
								CLK(19);
								tmp = GetMemB(ES, di);
								tmp2 = GetMemB(DS, si);
								int v1 = (tmp>>4)*10 + (tmp&0xf);
								int v2 = (tmp2>>4)*10 + (tmp2&0xf);
								if (v1 < (v2+m_CarryVal))
								{
									v1+=100;
									result = v1-(v2+m_CarryVal);
									m_CarryVal = 1;
								}
								else
								{
									result = v1-(v2+m_CarryVal);
									m_CarryVal = 0;
								}
								v1 = ((result/10)<<4) | (result % 10);
								PutMemB(ES, di,v1);
								if (v1)
								{
									m_ZeroVal = 1;
								}
								si++;
								di++;
							}
						}
						break;
					case 0x26:
						{
							int count = (m_regs.b[CL]+1)/2;
							UINT16 di = m_regs.w[IY];
							UINT16 si = m_regs.w[IX];
							if (m_seg_prefix)
							{
								logerror("%s: %06x: Warning: seg_prefix defined for cmp4s\n", tag(), pc());
							}
							m_ZeroVal = m_CarryVal = 0;
							for (int i=0; i<count; i++)
							{
								int result;
								CLK(19);
								tmp = GetMemB(ES, di);
								tmp2 = GetMemB(DS, si);
								int v1 = (tmp>>4)*10 + (tmp&0xf);
								int v2 = (tmp2>>4)*10 + (tmp2&0xf);
								if (v1 < (v2+m_CarryVal))
								{
									v1+=100;
									result = v1-(v2+m_CarryVal);
									m_CarryVal = 1;
								}
								else
								{
									result = v1-(v2+m_CarryVal);
									m_CarryVal = 0;
								}
								v1 = ((result/10)<<4) | (result % 10);
								if (v1)
								{
									m_ZeroVal = 1;
								}
								si++;
								di++;
							}
						}
						break;
					case 0x28:
						m_modrm = fetch();
						tmp = GetRMByte();
						tmp <<= 4;
						tmp |= m_regs.b[AL] & 0xf;
						m_regs.b[AL] = (m_regs.b[AL] & 0xf0) | ((tmp>>8)&0xf);
						tmp &= 0xff;
						PutbackRMByte(tmp);
						CLKM(9,15);
						break;
					case 0x2a:
						m_modrm = fetch();
						tmp = GetRMByte();
						tmp2 = (m_regs.b[AL] & 0xf)<<4;
						m_regs.b[AL] = (m_regs.b[AL] & 0xf0) | (tmp&0xf);
						tmp = tmp2 | (tmp>>4);
						PutbackRMByte(tmp);
						CLKM(13,19);
						break;
					case 0x31:
						m_modrm = fetch(); m_modrm = 0; logerror("%s: %06x: Unimplemented bitfield INS\n", tag(), pc()); break;
					case 0x33:
						m_modrm = fetch(); m_modrm = 0; logerror("%s: %06x: Unimplemented bitfield EXT\n", tag(), pc()); break;
					case 0x92:  /* V25/35 FINT */
						CLK(2);
						break;
					case 0xe0:
						m_modrm = fetch();
						m_modrm = 0;
						logerror("%s: %06x: V33 unimplemented BRKXA (break to expansion address)\n", tag(), pc());
						break;
					case 0xf0:
						m_modrm = fetch();
						m_modrm = 0;
						logerror("%s: %06x: V33 unimplemented RETXA (return from expansion address)\n", tag(), pc());
						break;
					case 0xff:
						m_modrm = fetch();
						m_modrm = 0;
						logerror("%s: %06x: unimplemented BRKEM (break to 8080 emulation mode)\n", tag(), pc());
						break;
					default:
						logerror("%s: %06x: Unknown V20 instruction\n", tag(), pc());
						break;
					}
				}
				break;


			case 0x10: // i_adc_br8
				DEF_br8();
				m_src += CF ? 1 : 0;
				ADDB();
				PutbackRMByte(m_dst);
				CLKM(1,3);
				break;

			case 0x11: // i_adc_wr16
				DEF_wr16();
				m_src += CF ? 1 : 0;
				ADDW();
				PutbackRMWord(m_dst);
				CLKM(1,3);
				break;

			case 0x12: // i_adc_r8b
				DEF_r8b();
				m_src += CF ? 1 : 0;
				ADDB();
				RegByte(m_dst);
				CLKM(1,2);
				break;

			case 0x13: // i_adc_r16w
				DEF_r16w();
				m_src += CF ? 1 : 0;
				ADDW();
				RegWord(m_dst);
				CLKM(1,2);
				break;

			case 0x14: // i_adc_ald8
				DEF_ald8();
				m_src += CF ? 1 : 0;
				ADDB();
				m_regs.b[AL] = m_dst;
				CLK(1);
				break;

			case 0x15: // i_adc_axd16
				DEF_axd16();
				m_src += CF ? 1 : 0;
				ADDW();
				m_regs.w[AW] = m_dst;
				CLK(1);
				break;

			case 0x16: // i_push_ss
				PUSH(m_sregs[SS]);
				CLK(2);
				break;

			case 0x17: // i_pop_ss
				m_sregs[SS] = POP();
				CLK(3);
				m_no_interrupt = 1;
				break;


			case 0x18: // i_sbb_br8
				DEF_br8();
				m_src += CF ? 1 : 0;
				SUBB();
				PutbackRMByte(m_dst);
				CLKM(1,3);
				break;

			case 0x19: // i_sbb_wr16
				DEF_wr16();
				m_src += CF ? 1 : 0;
				SUBW();
				PutbackRMWord(m_dst);
				CLKM(1,3);
				break;

			case 0x1a: // i_sbb_r8b
				DEF_r8b();
				m_src += CF ? 1 : 0;
				SUBB();
				RegByte(m_dst);
				CLKM(1,2);
				break;

			case 0x1b: // i_sbb_r16w
				DEF_r16w();
				m_src += CF ? 1 : 0;
				SUBW();
				RegWord(m_dst);
				CLKM(1,2);
				break;

			case 0x1c: // i_sbb_ald8
				DEF_ald8();
				m_src += CF ? 1 : 0;
				SUBB();
				m_regs.b[AL] = m_dst;
				CLK(1);
				break;

			case 0x1d: // i_sbb_axd16
				DEF_axd16();
				m_src += CF ? 1 : 0;
				SUBW();
				m_regs.w[AW] = m_dst;
				CLK(1);
				break;

			case 0x1e: // i_push_ds
				PUSH(m_sregs[DS]);
				CLK(2);
				break;

			case 0x1f: // i_pop_ds
				m_sregs[DS] = POP();
				CLK(3);
				break;


			case 0x20: // i_and_br8
				DEF_br8();
				ANDB();
				PutbackRMByte(m_dst);
				CLKM(1,3);
				break;

			case 0x21: // i_and_wr16
				DEF_wr16();
				ANDW();
				PutbackRMWord(m_dst);
				CLKM(1,3);
				break;

			case 0x22: // i_and_r8b
				DEF_r8b();
				ANDB();
				RegByte(m_dst);
				CLKM(1,2);
				break;

			case 0x23: // i_and_r16w
				DEF_r16w();
				ANDW();
				RegWord(m_dst);
				CLKM(1,2);
				break;

			case 0x24: // i_and_ald8
				DEF_ald8();
				ANDB();
				m_regs.b[AL] = m_dst;
				CLK(1);
				break;

			case 0x25: // i_and_axd16
				DEF_axd16();
				ANDW();
				m_regs.w[AW] = m_dst;
				CLK(1);
				break;

			case 0x26: // i_es
				m_seg_prefix_next = true;
				m_prefix_base = m_sregs[ES]<<4;
				CLK(1);
				break;

			case 0x27: // i_daa
				ADJ4(6,0x60);
				CLK(10);
				break;


			case 0x28: // i_sub_br8
				DEF_br8();
				SUBB();
				PutbackRMByte(m_dst);
				CLKM(1,3);
				break;

			case 0x29: // i_sub_wr16
				DEF_wr16();
				SUBW();
				PutbackRMWord(m_dst);
				CLKM(1,3);
				break;

			case 0x2a: // i_sub_r8b
				DEF_r8b();
				SUBB();
				RegByte(m_dst);
				CLKM(1,2);
				break;

			case 0x2b: // i_sub_r16w
				DEF_r16w();
				SUBW();
				RegWord(m_dst);
				CLKM(1,2);
				break;

			case 0x2c: // i_sub_ald8
				DEF_ald8();
				SUBB();
				m_regs.b[AL] = m_dst;
				CLK(1);
				break;

			case 0x2d: // i_sub_axd16
				DEF_axd16();
				SUBW();
				m_regs.w[AW] = m_dst;
				CLK(1);
				break;

			case 0x2e: // i_cs
				m_seg_prefix_next = true;
				m_prefix_base = m_sregs[CS]<<4;
				CLK(1);
				break;

			case 0x2f: // i_das
				ADJ4(-6,-0x60);
				CLK(10);
				break;


			case 0x30: // i_xor_br8
				DEF_br8();
				XORB();
				PutbackRMByte(m_dst);
				CLKM(1,3);
				break;

			case 0x31: // i_xor_wr16
				DEF_wr16();
				XORW();
				PutbackRMWord(m_dst);
				CLKM(1,3);
				break;

			case 0x32: // i_xor_r8b
				DEF_r8b();
				XORB();
				RegByte(m_dst);
				CLKM(1,2);
				break;

			case 0x33: // i_xor_r16w
				DEF_r16w();
				XORW();
				RegWord(m_dst);
				CLKM(1,2);
				break;

			case 0x34: // i_xor_ald8
				DEF_ald8();
				XORB();
				m_regs.b[AL] = m_dst;
				CLK(1);
				break;

			case 0x35: // i_xor_axd16
				DEF_axd16();
				XORW();
				m_regs.w[AW] = m_dst;
				CLK(1);
				break;

			case 0x36: // i_ss
				m_seg_prefix_next = true;
				m_prefix_base = m_sregs[SS]<<4;
				CLK(1);
				break;

			case 0x37: // i_aaa
				ADJB(6, (m_regs.b[AL] > 0xf9) ? 2 : 1);
				CLK(9);
				break;


			case 0x38: // i_cmp_br8
				DEF_br8();
				SUBB();
				CLKM(1,2);
				break;

			case 0x39: // i_cmp_wr16
				DEF_wr16();
				SUBW();
				CLKM(1,2);
				break;

			case 0x3a: // i_cmp_r8b
				DEF_r8b();
				SUBB();
				CLKM(1,2);
				break;

			case 0x3b: // i_cmp_r16w
				DEF_r16w();
				SUBW();
				CLKM(1,2);
				break;

			case 0x3c: // i_cmp_ald8
				DEF_ald8();
				SUBB();
				CLK(1);
				break;

			case 0x3d: // i_cmp_axd16
				DEF_axd16();
				SUBW();
				CLK(1);
				break;

			case 0x3e: // i_ds
				m_seg_prefix_next = true;
				m_prefix_base = m_sregs[DS]<<4;
				CLK(1);
				break;

			case 0x3f: // i_aas
				ADJB(-6, (m_regs.b[AL] < 6) ? -2 : -1);
				CLK(9);
				break;


			case 0x40: // i_inc_ax
				IncWordReg(AW);
				CLK(1);
				break;

			case 0x41: // i_inc_cx
				IncWordReg(CW);
				CLK(1);
				break;

			case 0x42: // i_inc_dx
				IncWordReg(DW);
				CLK(1);
				break;

			case 0x43: // i_inc_bx
				IncWordReg(BW);
				CLK(1);
				break;

			case 0x44: // i_inc_sp
				IncWordReg(SP);
				CLK(1);
				break;

			case 0x45: // i_inc_bp
				IncWordReg(BP);
				CLK(1);
				break;

			case 0x46: // i_inc_si
				IncWordReg(IX);
				CLK(1);
				break;

			case 0x47: // i_inc_di
				IncWordReg(IY);
				CLK(1);
				break;


			case 0x48: // i_dec_ax
				DecWordReg(AW);
				CLK(1);
				break;

			case 0x49: // i_dec_cx
				DecWordReg(CW);
				CLK(1);
				break;

			case 0x4a: // i_dec_dx
				DecWordReg(DW);
				CLK(1);
				break;

			case 0x4b: // i_dec_bx
				DecWordReg(BW);
				CLK(1);
				break;

			case 0x4c: // i_dec_sp
				DecWordReg(SP);
				CLK(1);
				break;

			case 0x4d: // i_dec_bp
				DecWordReg(BP);
				CLK(1);
				break;

			case 0x4e: // i_dec_si
				DecWordReg(IX);
				CLK(1);
				break;

			case 0x4f: // i_dec_di
				DecWordReg(IY);
				CLK(1);
				break;


			case 0x50: // i_push_ax
				PUSH(m_regs.w[AW]);
				CLK(1);
				break;

			case 0x51: // i_push_cx
				PUSH(m_regs.w[CW]);
				CLK(1);
				break;

			case 0x52: // i_push_dx
				PUSH(m_regs.w[DW]);
				CLK(1);
				break;

			case 0x53: // i_push_bx
				PUSH(m_regs.w[BW]);
				CLK(1);
				break;

			case 0x54: // i_push_sp
				PUSH(m_regs.w[SP]);
				CLK(1);
				break;

			case 0x55: // i_push_bp
				PUSH(m_regs.w[BP]);
				CLK(1);
				break;

			case 0x56: // i_push_si
				PUSH(m_regs.w[IX]);
				CLK(1);
				break;

			case 0x57: // i_push_di
				PUSH(m_regs.w[IY]);
				CLK(1);
				break;


			case 0x58: // i_pop_ax
				m_regs.w[AW] = POP();
				CLK(1);
				break;

			case 0x59: // i_pop_cx
				m_regs.w[CW] = POP();
				CLK(1);
				break;

			case 0x5a: // i_pop_dx
				m_regs.w[DW] = POP();
				CLK(1);
				break;

			case 0x5b: // i_pop_bx
				m_regs.w[BW] = POP();
				CLK(1);
				break;

			case 0x5c: // i_pop_sp
				m_regs.w[SP] = POP();
				CLK(1);
				break;

			case 0x5d: // i_pop_bp
				m_regs.w[BP] = POP();
				CLK(1);
				break;

			case 0x5e: // i_pop_si
				m_regs.w[IX] = POP();
				CLK(1);
				break;

			case 0x5f: // i_pop_di
				m_regs.w[IY] = POP();
				CLK(1);
				break;


			case 0x60: // i_pusha
				{
					UINT32 tmp = m_regs.w[SP];

					PUSH(m_regs.w[AW]);
					PUSH(m_regs.w[CW]);
					PUSH(m_regs.w[DW]);
					PUSH(m_regs.w[BW]);
					PUSH(tmp);
					PUSH(m_regs.w[BP]);
					PUSH(m_regs.w[IX]);
					PUSH(m_regs.w[IY]);
					CLK(9);
				}
				break;

			case 0x61: // i_popa
				m_regs.w[IY] = POP();
				m_regs.w[IX] = POP();
				m_regs.w[BP] = POP();
								POP();
				m_regs.w[BW] = POP();
				m_regs.w[DW] = POP();
				m_regs.w[CW] = POP();
				m_regs.w[AW] = POP();
				CLK(8);
				break;

			case 0x62: // i_chkind
				{
					UINT32 low,high,tmp;
					m_modrm = fetch();
					low = GetRMWord();
					high = GetnextRMWord();
					tmp = RegWord();
					if (tmp<low || tmp>high)
					{
						interrupt(5);
						CLK(20);
					}
					else
					{
						CLK(13);
					}
					logerror("%s: %06x: bound %04x high %04x low %04x tmp\n", tag(), pc(), high, low, tmp);
				}
				break;

			case 0x64: // i_repnc
				{
					UINT8 next = repx_op();
					UINT16 c = m_regs.w[CW];

					switch (next)
					{
					case 0x6c:  CLK(2); if (c) do { i_insb();  c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6d:  CLK(2); if (c) do { i_insw();  c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6e:  CLK(2); if (c) do { i_outsb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6f:  CLK(2); if (c) do { i_outsw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa4:  CLK(2); if (c) do { i_movsb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa5:  CLK(2); if (c) do { i_movsw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa6:  CLK(2); if (c) do { i_cmpsb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa7:  CLK(2); if (c) do { i_cmpsw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaa:  CLK(2); if (c) do { i_stosb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xab:  CLK(2); if (c) do { i_stosw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xac:  CLK(2); if (c) do { i_lodsb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xad:  CLK(2); if (c) do { i_lodsw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xae:  CLK(2); if (c) do { i_scasb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaf:  CLK(2); if (c) do { i_scasw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					default:
						logerror("%s: %06x: REPNC invalid\n", tag(), pc() );
						// Decrement IP so the normal instruction will be executed next
						m_ip--;
						break;
					}
				}
				break;


			case 0x65: // i_repc
				{
					UINT8 next = repx_op();
					UINT16 c = m_regs.w[CW];

					switch (next)
					{
					case 0x6c:  CLK(2); if (c) do { i_insb();  c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6d:  CLK(2); if (c) do { i_insw();  c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6e:  CLK(2); if (c) do { i_outsb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6f:  CLK(2); if (c) do { i_outsw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa4:  CLK(2); if (c) do { i_movsb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa5:  CLK(2); if (c) do { i_movsw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa6:  CLK(2); if (c) do { i_cmpsb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa7:  CLK(2); if (c) do { i_cmpsw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaa:  CLK(2); if (c) do { i_stosb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xab:  CLK(2); if (c) do { i_stosw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xac:  CLK(2); if (c) do { i_lodsb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xad:  CLK(2); if (c) do { i_lodsw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xae:  CLK(2); if (c) do { i_scasb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaf:  CLK(2); if (c) do { i_scasw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					default:
						logerror("%s: %06x: REPC invalid\n", tag(), pc());
						// Decrement IP so the normal instruction will be executed next
						m_ip--;
						break;
					}
				}
				break;


			case 0x68: // i_push_d16
				PUSH( fetch_word() );
				CLK(1);
				break;

			case 0x69: // i_imul_d16
				{
					UINT32 tmp;
					DEF_r16w();
					tmp = fetch_word();
					m_dst = (INT32)((INT16)m_src)*(INT32)((INT16)tmp);
					m_CarryVal = m_OverVal = (((INT32)m_dst) >> 15 != 0) && (((INT32)m_dst) >> 15 != -1);
					RegWord(m_dst);
					CLKM(3,4);
				}
				break;

			case 0x6a: // i_push_d8
				PUSH( (UINT16)((INT16)((INT8)fetch())) );
				CLK(1);
				break;

			case 0x6b: // i_imul_d8
				{
					UINT32 src2;
					DEF_r16w();
					src2= (UINT16)((INT16)((INT8)fetch()));
					m_dst = (INT32)((INT16)m_src)*(INT32)((INT16)src2);
					m_CarryVal = m_OverVal = (((INT32)m_dst) >> 15 != 0) && (((INT32)m_dst) >> 15 != -1);
					RegWord(m_dst);
					CLKM(3,4);
				}
				break;

			case 0x6c: // i_insb
				i_insb();
				break;

			case 0x6d: // i_insw
				i_insw();
				break;

			case 0x6e: // i_outsb
				i_outsb();
				break;

			case 0x6f: // i_outsw
				i_outsw();
				break;


			case 0x70: // i_jo
				JMP( OF);
				break;

			case 0x71: // i_jno
				JMP(!OF);
				break;

			case 0x72: // i_jc
				JMP( CF);
				break;

			case 0x73: // i_jnc
				JMP(!CF);
				break;

			case 0x74: // i_jz
				JMP( ZF);
				break;

			case 0x75: // i_jnz
				JMP(!ZF);
				break;

			case 0x76: // i_jce
				JMP(CF || ZF);
				break;

			case 0x77: // i_jnce
				JMP(!(CF || ZF));
				break;

			case 0x78: // i_js
				JMP( SF);
				break;

			case 0x79: // i_jns
				JMP(!SF);
				break;

			case 0x7a: // i_jp
				JMP( PF);
				break;

			case 0x7b: // i_jnp
				JMP(!PF);
				break;

			case 0x7c: // i_jl
				JMP((SF!=OF)&&(!ZF));
				break;

			case 0x7d: // i_jnl
				JMP((ZF)||(SF==OF));
				break;

			case 0x7e: // i_jle
				JMP((ZF)||(SF!=OF));
				break;

			case 0x7f: // i_jnle
				JMP((SF==OF)&&(!ZF));
				break;


			case 0x80: // i_80pre
				m_modrm = fetch();
				m_dst = GetRMByte();
				m_src = fetch();
				if (m_modrm >=0xc0 )             { CLK(1); }
				else if ((m_modrm & 0x38)==0x38) { CLK(2); }
				else                             { CLK(3); }
				switch (m_modrm & 0x38)
				{
				case 0x00:                      ADDB(); PutbackRMByte(m_dst);   break;
				case 0x08:                      ORB();  PutbackRMByte(m_dst);   break;
				case 0x10: m_src += CF ? 1 : 0; ADDB(); PutbackRMByte(m_dst);   break;
				case 0x18: m_src += CF ? 1 : 0; SUBB(); PutbackRMByte(m_dst);   break;
				case 0x20:                      ANDB(); PutbackRMByte(m_dst);   break;
				case 0x28:                      SUBB(); PutbackRMByte(m_dst);   break;
				case 0x30:                      XORB(); PutbackRMByte(m_dst);   break;
				case 0x38:                      SUBB();                         break;  /* CMP */
				}
				break;


			case 0x81: // i_81pre
				m_modrm = fetch();
				m_dst = GetRMWord();
				m_src = fetch_word();
				if (m_modrm >=0xc0 )             { CLK(1); }
				else if ((m_modrm & 0x38)==0x38) { CLK(2); }
				else                             { CLK(3); }
				switch (m_modrm & 0x38)
				{
				case 0x00:                      ADDW(); PutbackRMWord(m_dst);   break;
				case 0x08:                      ORW();  PutbackRMWord(m_dst);   break;
				case 0x10: m_src += CF ? 1 : 0; ADDW(); PutbackRMWord(m_dst);   break;
				case 0x18: m_src += CF ? 1 : 0; SUBW(); PutbackRMWord(m_dst);   break;
				case 0x20:                      ANDW(); PutbackRMWord(m_dst);   break;
				case 0x28:                      SUBW(); PutbackRMWord(m_dst);   break;
				case 0x30:                      XORW(); PutbackRMWord(m_dst);   break;
				case 0x38:                      SUBW();                         break;  /* CMP */
				}
				break;


			case 0x82: // i_82pre
				m_modrm = fetch();
				m_dst = GetRMByte();
				m_src = (INT8)fetch();
				if (m_modrm >=0xc0 )             { CLK(1); }
				else if ((m_modrm & 0x38)==0x38) { CLK(2); }
				else                             { CLK(3); }
				switch (m_modrm & 0x38)
				{
				case 0x00:                      ADDB(); PutbackRMByte(m_dst);   break;
				case 0x08:                      ORB();  PutbackRMByte(m_dst);   break;
				case 0x10: m_src += CF ? 1 : 0; ADDB(); PutbackRMByte(m_dst);   break;
				case 0x18: m_src += CF ? 1 : 0; SUBB(); PutbackRMByte(m_dst);   break;
				case 0x20:                      ANDB(); PutbackRMByte(m_dst);   break;
				case 0x28:                      SUBB(); PutbackRMByte(m_dst);   break;
				case 0x30:                      XORB(); PutbackRMByte(m_dst);   break;
				case 0x38:                      SUBB();                         break; /* CMP */
				}
				break;


			case 0x83: // i_83pre
				m_modrm = fetch();
				m_dst = GetRMWord();
				m_src = ((INT16)((INT8)fetch()));
				if ( m_modrm >= 0xc0 )               { CLK(1); }
				else if (( m_modrm & 0x38 ) == 0x38) { CLK(2); }
				else                                 { CLK(3); }
				switch (m_modrm & 0x38)
				{
				case 0x00:                      ADDW(); PutbackRMWord(m_dst); break;
				case 0x08:                      ORW();  PutbackRMWord(m_dst); break;
				case 0x10: m_src += CF ? 1 : 0; ADDW(); PutbackRMWord(m_dst); break;
				case 0x18: m_src += CF ? 1 : 0; SUBW(); PutbackRMWord(m_dst); break;
				case 0x20:                      ANDW(); PutbackRMWord(m_dst); break;
				case 0x28:                      SUBW(); PutbackRMWord(m_dst); break;
				case 0x30:                      XORW(); PutbackRMWord(m_dst); break;
				case 0x38:                      SUBW();                       break; /* CMP */
				}
				break;


			case 0x84: // i_test_br8
				DEF_br8();
				ANDB();
				CLKM(1,2);
				break;

			case 0x85: // i_test_wr16
				DEF_wr16();
				ANDW();
				CLKM(1,2);
				break;

			case 0x86: // i_xchg_br8
				DEF_br8();
				RegByte(m_dst);
				PutbackRMByte(m_src);
				CLKM(3,5);
				break;

			case 0x87: // i_xchg_wr16
				DEF_wr16();
				RegWord(m_dst);
				PutbackRMWord(m_src);
				CLKM(3,5);
				break;


			case 0x88: // i_mov_br8
				m_modrm = fetch();
				m_src = RegByte();
				PutRMByte(m_src);
				CLK(1);
				break;

			case 0x89: // i_mov_wr16
				m_modrm = fetch();
				m_src = RegWord();
				PutRMWord(m_src);
				CLK(1);
				break;

			case 0x8a: // i_mov_r8b
				m_modrm = fetch();
				m_src = GetRMByte();
				RegByte(m_src);
				CLK(1);
				break;

			case 0x8b: // i_mov_r16w
				m_modrm = fetch();
				m_src = GetRMWord();
				RegWord(m_src);
				CLK(1);
				break;

			case 0x8c: // i_mov_wsreg
				m_modrm = fetch();
				PutRMWord(m_sregs[(m_modrm & 0x38) >> 3]);
				CLKM(1,3);
				break;

			case 0x8d: // i_lea
				m_modrm = fetch();
				get_ea();
				RegWord(m_eo);
				CLK(1);
				break;

			case 0x8e: // i_mov_sregw
				m_modrm = fetch();
				m_src = GetRMWord();
				CLKM(2,3);
				switch (m_modrm & 0x38)
				{
				case 0x00:  /* mov es,ew */
					m_sregs[ES] = m_src;
					break;
				case 0x08:  /* mov cs,ew */
					m_sregs[CS] = m_src;
					break;
				case 0x10:  /* mov ss,ew */
					m_sregs[SS] = m_src;
					break;
				case 0x18:  /* mov ds,ew */
					m_sregs[DS] = m_src;
					break;
				default:
					logerror("%s: %06x: Mov Sreg - Invalid register\n", tag(), pc());
				}
				m_no_interrupt = 1;
				break;

			case 0x8f: // i_popw
				m_modrm = fetch();
				PutRMWord( POP() );
				CLKM(1,3);
				break;

			case 0x90: // i_nop
				CLK(1);
				break;

			case 0x91: // i_xchg_axcx
				XchgAWReg(CW);
				CLK(3);
				break;

			case 0x92: // i_xchg_axdx
				XchgAWReg(DW);
				CLK(3);
				break;

			case 0x93: // i_xchg_axbx
				XchgAWReg(BW);
				CLK(3);
				break;

			case 0x94: // i_xchg_axsp
				XchgAWReg(SP);
				CLK(3);
				break;

			case 0x95: // i_xchg_axbp
				XchgAWReg(BP);
				CLK(3);
				break;

			case 0x96: // i_xchg_axsi
				XchgAWReg(IX);
				CLK(3);
				break;

			case 0x97: // i_xchg_axdi
				XchgAWReg(IY);
				CLK(3);
				break;


			case 0x98: // i_cbw
				m_regs.b[AH] = (m_regs.b[AL] & 0x80) ? 0xff : 0;
				CLK(1);
				break;

			case 0x99: // i_cwd
				m_regs.w[DW] = (m_regs.b[AH] & 0x80) ? 0xffff : 0;
				CLK(1);
				break;

			case 0x9a: // i_call_far
				{
					UINT16 tmp = fetch_word();
					UINT16 tmp2 = fetch_word();
					PUSH(m_sregs[CS]);
					PUSH(m_ip);
					m_ip = tmp;
					m_sregs[CS] = tmp2;
					CLK(10);
				}
				break;

			case 0x9b: // i_wait
				logerror("%s: %06x: Hardware POLL\n", tag(), pc());
				break;

			case 0x9c: // i_pushf
				PUSH( CompressFlags() );
				CLK(2);
				break;

			case 0x9d: // i_popf
				i_popf();
				break;

			case 0x9e: // i_sahf
				{
					UINT32 tmp = (CompressFlags() & 0xff00) | (m_regs.b[AH] & 0xd5);
					ExpandFlags(tmp);
					CLK(4);
				}
				break;

			case 0x9f: // i_lahf
				m_regs.b[AH] = CompressFlags();
				CLK(2);
				break;


			case 0xa0: // i_mov_aldisp
				{
					UINT32 addr = fetch_word();
					m_regs.b[AL] = GetMemB(DS, addr);
					CLK(1);
				}
				break;

			case 0xa1: // i_mov_axdisp
				{
					UINT32 addr = fetch_word();
					m_regs.b[AL] = GetMemB(DS, addr);
					m_regs.b[AH] = GetMemB(DS, addr+1);
					CLK(1);
				}
				break;

			case 0xa2: // i_mov_dispal
				{
					UINT32 addr = fetch_word();
					PutMemB(DS, addr, m_regs.b[AL]);
					CLK(1);
				}
				break;

			case 0xa3: // i_mov_dispax
				{
					UINT32 addr = fetch_word();
					PutMemB(DS, addr, m_regs.b[AL]);
					PutMemB(DS, addr+1, m_regs.b[AH]);
					CLK(1);
				}
				break;

			case 0xa4: // i_movsb
				i_movsb();
				break;

			case 0xa5: // i_movsw
				i_movsw();
				break;

			case 0xa6: // i_cmpsb
				i_cmpsb();
				break;

			case 0xa7: // i_cmpsw
				i_cmpsw();
				break;


			case 0xa8: // i_test_ald8
				DEF_ald8();
				ANDB();
				CLK(1);
				break;

			case 0xa9: // i_test_axd16
				DEF_axd16();
				ANDW();
				CLK(1);
				break;

			case 0xaa: // i_stosb
				i_stosb();
				break;

			case 0xab: // i_stosw
				i_stosw();
				break;

			case 0xac: // i_lodsb
				i_lodsb();
				break;

			case 0xad: // i_lodsw
				i_lodsw();
				break;

			case 0xae: // i_scasb
				i_scasb();
				break;

			case 0xaf: // i_scasw
				i_scasw();
				break;


			case 0xb0: // i_mov_ald8
				m_regs.b[AL] = fetch();
				CLK(1);
				break;

			case 0xb1: // i_mov_cld8
				m_regs.b[CL] = fetch();
				CLK(1);
				break;

			case 0xb2: // i_mov_dld8
				m_regs.b[DL] = fetch();
				CLK(1);
				break;

			case 0xb3: // i_mov_bld8
				m_regs.b[BL] = fetch();
				CLK(1);
				break;

			case 0xb4: // i_mov_ahd8
				m_regs.b[AH] = fetch();
				CLK(1);
				break;

			case 0xb5: // i_mov_chd8
				m_regs.b[CH] = fetch();
				CLK(1);
				break;

			case 0xb6: // i_mov_dhd8
				m_regs.b[DH] = fetch();
				CLK(1);
				break;

			case 0xb7: // i_mov_bhd8
				m_regs.b[BH] = fetch();
				CLK(1);
				break;


			case 0xb8: // i_mov_axd16
				m_regs.b[AL] = fetch();
				m_regs.b[AH] = fetch();
				CLK(1);
				break;

			case 0xb9: // i_mov_cxd16
				m_regs.b[CL] = fetch();
				m_regs.b[CH] = fetch();
				CLK(1);
				break;

			case 0xba: // i_mov_dxd16
				m_regs.b[DL] = fetch();
				m_regs.b[DH] = fetch();
				CLK(1);
				break;

			case 0xbb: // i_mov_bxd16
				m_regs.b[BL] = fetch();
				m_regs.b[BH] = fetch();
				CLK(1);
				break;

			case 0xbc: // i_mov_spd16
				m_regs.b[SPL] = fetch();
				m_regs.b[SPH] = fetch();
				CLK(1);
				break;

			case 0xbd: // i_mov_bpd16
				m_regs.b[BPL] = fetch();
				m_regs.b[BPH] = fetch();
				CLK(1);
				break;

			case 0xbe: // i_mov_sid16
				m_regs.b[IXL] = fetch();
				m_regs.b[IXH] = fetch();
				CLK(1);
				break;

			case 0xbf: // i_mov_did16
				m_regs.b[IYL] = fetch();
				m_regs.b[IYH] = fetch();
				CLK(1);
				break;


			case 0xc0: // i_rotshft_bd8
				{
					UINT8 c;
					m_modrm = fetch();
					m_src = GetRMByte();
					m_dst = m_src;
					c = fetch();
					CLKM(3,5);
					if (c)
					{
						switch ( m_modrm & 0x38 )
						{
						case 0x00: do { ROL_BYTE();  c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x08: do { ROR_BYTE();  c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x10: do { ROLC_BYTE(); c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x18: do { RORC_BYTE(); c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x20: SHL_BYTE(c); break;
						case 0x28: SHR_BYTE(c); break;
						case 0x30: logerror("%s: %06x: Undefined opcode 0xc0 0x30 (SHLA)\n", tag(), pc()); break;
						case 0x38: SHRA_BYTE(c); break;
						}
					}
				}
				break;

			case 0xc1: // i_rotshft_wd8
				{
					UINT8 c;
					m_modrm = fetch();
					m_src = GetRMWord();
					m_dst = m_src;
					c = fetch();
					CLKM(3,5);
					if (c)
					{
						switch ( m_modrm & 0x38 )
						{
						case 0x00: do { ROL_WORD();  c--; } while (c>0); PutbackRMWord(m_dst); break;
						case 0x08: do { ROR_WORD();  c--; } while (c>0); PutbackRMWord(m_dst); break;
						case 0x10: do { ROLC_WORD(); c--; } while (c>0); PutbackRMWord(m_dst); break;
						case 0x18: do { RORC_WORD(); c--; } while (c>0); PutbackRMWord(m_dst); break;
						case 0x20: SHL_WORD(c); break;
						case 0x28: SHR_WORD(c); break;
						case 0x30: logerror("%s: %06x: Undefined opcode 0xc1 0x30 (SHLA)\n", tag(), pc()); break;
						case 0x38: SHRA_WORD(c); break;
						}
					}
				}
				break;


			case 0xc2: // i_ret_d16
				{
					UINT32 count = fetch_word();
					m_ip = POP();
					m_regs.w[SP] += count;
					CLK(6);
				}
				break;

			case 0xc3: // i_ret
				m_ip = POP();
				CLK(6);
				break;

			case 0xc4: // i_les_dw
				m_modrm = fetch();
				RegWord( GetRMWord() );
				m_sregs[ES] = GetnextRMWord();
				CLK(6);
				break;

			case 0xc5: // i_lds_dw
				m_modrm = fetch();
				RegWord( GetRMWord() );
				m_sregs[DS] = GetnextRMWord();
				CLK(6);
				break;

			case 0xc6: // i_mov_bd8
				m_modrm = fetch();
				PutImmRMByte();
				CLK(1);
				break;

			case 0xc7: // i_mov_wd16
				m_modrm = fetch();
				PutImmRMWord();
				CLK(1);
				break;


			case 0xc8: // i_enter
				{
					UINT16 nb = fetch();
					UINT32 level;

					CLK(8);
					nb |= fetch() << 8;
					level = fetch();
					PUSH(m_regs.w[BP]);
					m_regs.w[BP] = m_regs.w[SP];
					m_regs.w[SP] -= nb;
					for (int i=1; i<level; i++)
					{
						PUSH( GetMemW(SS,m_regs.w[BP] - i*2) );
						CLK(4);
					}
					if (level)
					{
						PUSH(m_regs.w[BP]);
						CLK( ( level == 1 ) ? 2 : 3 );
					}
				}
				break;

			case 0xc9: // i_leave
				m_regs.w[SP] = m_regs.w[BP];
				m_regs.w[BP] = POP();
				CLK(2);
				break;

			case 0xca: // i_retf_d16
				{
					UINT32 count = fetch_word();
					m_ip = POP();
					m_sregs[CS] = POP();
					m_regs.w[SP] += count;
					CLK(9);
				}
				break;

			case 0xcb: // i_retf
				m_ip = POP();
				m_sregs[CS] = POP();
				CLK(8);
				break;

			case 0xcc: // i_int3
				interrupt(3);
				CLK(9);
				break;

			case 0xcd: // i_int
				interrupt(fetch());
				CLK(10);
				break;

			case 0xce: // i_into
				if (OF)
				{
					interrupt(4);
					CLK(7);
				}
				CLK(6);
				break;

			case 0xcf: // i_iret
				m_ip = POP();
				m_sregs[CS] = POP();
				i_popf();
				CLK(10);
				break;


			case 0xd0: // i_rotshft_b
				m_modrm = fetch();
				m_src = GetRMByte();
				m_dst = m_src;
				CLKM(1,3);
				switch ( m_modrm & 0x38 )
				{
				case 0x00: ROL_BYTE();  PutbackRMByte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x08: ROR_BYTE();  PutbackRMByte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x10: ROLC_BYTE(); PutbackRMByte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x18: RORC_BYTE(); PutbackRMByte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x20: SHL_BYTE(1); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x28: SHR_BYTE(1); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x30: logerror("%s: %06x: Undefined opcode 0xd0 0x30 (SHLA)\n", tag(), pc()); break;
				case 0x38: SHRA_BYTE(1); m_OverVal = 0; break;
				}
				break;

			case 0xd1: // i_rotshft_w
				m_modrm = fetch();
				m_src = GetRMWord();
				m_dst = m_src;
				CLKM(1,3);
				switch ( m_modrm & 0x38 )
				{
				case 0x00: ROL_WORD();  PutbackRMWord(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
				case 0x08: ROR_WORD();  PutbackRMWord(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
				case 0x10: ROLC_WORD(); PutbackRMWord(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
				case 0x18: RORC_WORD(); PutbackRMWord(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
				case 0x20: SHL_WORD(1); m_OverVal = (m_src ^ m_dst) & 0x8000;  break;
				case 0x28: SHR_WORD(1); m_OverVal = (m_src ^ m_dst) & 0x8000;  break;
				case 0x30: logerror("%s: %06x: Undefined opcode 0xd1 0x30 (SHLA)\n", tag(), pc()); break;
				case 0x38: SHRA_WORD(1); m_OverVal = 0; break;
				}
				break;

			case 0xd2: // i_rotshft_bcl
				{
					UINT8 c;

					m_modrm = fetch();
					m_src = GetRMByte();
					m_dst = m_src;
					c = m_regs.b[CL];
					CLKM(3,5);
					if (c)
					{
						switch ( m_modrm & 0x38 )
						{
						case 0x00: do { ROL_BYTE();  c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x08: do { ROR_BYTE();  c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x10: do { ROLC_BYTE(); c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x18: do { RORC_BYTE(); c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x20: SHL_BYTE(c); break;
						case 0x28: SHR_BYTE(c); break;
						case 0x30: logerror("%s: %06x: Undefined opcode 0xd2 0x30 (SHLA)\n", tag(), pc()); break;
						case 0x38: SHRA_BYTE(c); break;
						}
					}
				}
				break;

			case 0xd3: // i_rotshft_wcl
				{
					UINT8 c;

					m_modrm = fetch();
					m_src = GetRMWord();
					m_dst = m_src;
					c = m_regs.b[CL];
					CLKM(3,5);
					if (c)
					{
						switch ( m_modrm & 0x38 )
						{
							case 0x00: do { ROL_WORD();  c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x08: do { ROR_WORD();  c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x10: do { ROLC_WORD(); c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x18: do { RORC_WORD(); c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x20: SHL_WORD(c); break;
							case 0x28: SHR_WORD(c); break;
							case 0x30: logerror("%s: %06x: Undefined opcode 0xd3 0x30 (SHLA)\n", tag(), pc()); break;
							case 0x38: SHRA_WORD(c); break;
						}
					}
				}
				break;

			case 0xd4: // i_aam
				fetch();
				m_regs.b[AH] = m_regs.b[AL] / 10;
				m_regs.b[AL] %= 10;
				set_SZPF_Word(m_regs.w[AW]);
				CLK(17);
				break;

			case 0xd5: // i_aad
				fetch();
				m_regs.b[AL] = m_regs.b[AH] * 10 + m_regs.b[AL];
				m_regs.b[AH] = 0;
				set_SZPF_Byte(m_regs.b[AL]);
				CLK(5);
				break;

			case 0xd6: // i_setalc
				m_regs.b[AL] = (CF) ? 0xff : 0x00;
				CLK(3);
				logerror("%s: %06x: Undefined opcode (SETALC)\n", tag(), pc() );
				break;

			case 0xd7: // i_trans
				m_regs.b[AL] = GetMemB( DS, m_regs.w[BW] + m_regs.b[AL] );
				CLK(5);
				break;

			case 0xd8: // i_fpo
				m_modrm = fetch();
				CLK(1);
				logerror("%s: %06x: Unimplemented floating point control %04x\n", tag(), pc(), m_modrm);
				break;


			case 0xe0: // i_loopne
				{
					INT8 disp = (INT8)fetch();

					m_regs.w[CW]--;
					if (!ZF && m_regs.w[CW])
					{
						m_ip = m_ip + disp;
						CLK(3);
					}
					CLK(3);
				}
				break;

			case 0xe1: // i_loope
				{
					INT8 disp = (INT8)fetch();

					m_regs.w[CW]--;
					if (ZF && m_regs.w[CW])
					{
						m_ip = m_ip + disp;
						CLK(3);
					}
					CLK(3);
				}
				break;

			case 0xe2: // i_loop
				{
					INT8 disp = (INT8)fetch();

					m_regs.w[CW]--;
					if (m_regs.w[CW])
					{
						m_ip = m_ip + disp;
						CLK(3);
					}
					CLK(2);
				}
				break;

			case 0xe3: // i_jcxz
				{
					INT8 disp = (INT8)fetch();

					if (m_regs.w[CW] == 0)
					{
						m_ip = m_ip + disp;
						CLK(3);
					}
					CLK(1);
				}
				break;

			case 0xe4: // i_inal
				m_regs.b[AL] = read_port( fetch() );
				CLK(6);
				break;

			case 0xe5: // i_inax
				{
					UINT8 port = fetch();

					m_regs.b[AL] = read_port(port);
					m_regs.b[AH] = read_port(port+1);
					CLK(6);
				}
				break;

			case 0xe6: // i_outal
				write_port( fetch(), m_regs.b[AL]);
				CLK(6);
				break;

			case 0xe7: // i_outax
				{
					UINT8 port = fetch();

					write_port(port, m_regs.b[AL]);
					write_port(port+1, m_regs.b[AH]);
					CLK(6);
				}
				break;


			case 0xe8: // i_call_d16
				{
					INT16 tmp = (INT16)fetch_word();

					PUSH(m_ip);
					m_ip = m_ip + tmp;
					CLK(5);
				}
				break;

			case 0xe9: // i_jmp_d16
				{
					INT16 offset = (INT16)fetch_word();
					m_ip += offset;
					CLK(4);
				}
				break;

			case 0xea: // i_jmp_far
				{
					UINT16 tmp = fetch_word();
					UINT16 tmp1 = fetch_word();

					m_sregs[CS] = tmp1;
					m_ip = tmp;
					CLK(7);
				}
				break;

			case 0xeb: // i_jmp_d8
				{
					int tmp = (int)((INT8)fetch());

					CLK(4);
					if (tmp==-2 && m_no_interrupt==0 && (m_pending_irq==0) && m_icount>0)
					{
						m_icount%=12; /* cycle skip */
					}
					m_ip = (UINT16)(m_ip+tmp);
				}
				break;

			case 0xec: // i_inaldx
				m_regs.b[AL] = read_port(m_regs.w[DW]);
				CLK(6);
				break;

			case 0xed: // i_inaxdx
				{
					UINT32 port = m_regs.w[DW];

					m_regs.b[AL] = read_port(port);
					m_regs.b[AH] = read_port(port+1);
					CLK(6);
				}
				break;

			case 0xee: // i_outdxal
				write_port(m_regs.w[DW], m_regs.b[AL]);
				CLK(6);
				break;

			case 0xef: // i_outdxax
				{
					UINT32 port = m_regs.w[DW];

					write_port(port, m_regs.b[AL]);
					write_port(port+1, m_regs.b[AH]);
					CLK(6);
				}
				break;


			case 0xf0: // i_lock
				logerror("%s: %06x: Warning - BUSLOCK\n", tag(), pc());
				m_no_interrupt = 1;
				CLK(1);
				break;

			case 0xf2: // i_repne
				{
					UINT8 next = repx_op();
					UINT16 c = m_regs.w[CW];

					switch (next)
					{
					case 0x6c:  CLK(3); if (c) do { i_insb();  c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6d:  CLK(3); if (c) do { i_insw();  c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6e:  CLK(3); if (c) do { i_outsb(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6f:  CLK(3); if (c) do { i_outsw(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa4:  CLK(3); if (c) do { i_movsb(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa5:  CLK(3); if (c) do { i_movsw(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa6:  CLK(3); if (c) do { i_cmpsb(); c--; } while (c>0 && !ZF);   m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa7:  CLK(3); if (c) do { i_cmpsw(); c--; } while (c>0 && !ZF);   m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaa:  CLK(3); if (c) do { i_stosb(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xab:  CLK(3); if (c) do { i_stosw(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xac:  CLK(3); if (c) do { i_lodsb(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xad:  CLK(3); if (c) do { i_lodsw(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xae:  CLK(3); if (c) do { i_scasb(); c--; } while (c>0 && !ZF);   m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaf:  CLK(3); if (c) do { i_scasw(); c--; } while (c>0 && !ZF);   m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					default:
						logerror("%s: %06x: REPNE invalid\n", tag(), pc());
						// Decrement IP so the normal instruction will be executed next
						m_ip--;
						break;
					}
				}
				break;

			case 0xf3: // i_repe
				{
					UINT8 next = repx_op();
					UINT16 c = m_regs.w[CW];

					switch (next)
					{
					case 0x6c:  CLK(3); if (c) do { i_insb();  c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6d:  CLK(3); if (c) do { i_insw();  c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6e:  CLK(3); if (c) do { i_outsb(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6f:  CLK(3); if (c) do { i_outsw(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa4:  CLK(3); if (c) do { i_movsb(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa5:  CLK(3); if (c) do { i_movsw(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa6:  CLK(3); if (c) do { i_cmpsb(); c--; } while (c>0 && ZF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa7:  CLK(3); if (c) do { i_cmpsw(); c--; } while (c>0 && ZF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaa:  CLK(3); if (c) do { i_stosb(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xab:  CLK(3); if (c) do { i_stosw(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xac:  CLK(3); if (c) do { i_lodsb(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xad:  CLK(3); if (c) do { i_lodsw(); c--; } while (c>0);          m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xae:  CLK(3); if (c) do { i_scasb(); c--; } while (c>0 && ZF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaf:  CLK(3); if (c) do { i_scasw(); c--; } while (c>0 && ZF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					default:
						logerror("%s: %06x: REPE invalid\n", tag(), pc());
						// Decrement IP so the normal instruction will be executed next
						m_ip--;
						break;
					}
				}
				break;

			case 0xf4: // i_hlt
				logerror("%s: %06x: HALT\n", tag(), pc());
				m_icount = 0;
				break;

			case 0xf5: // i_cmc
				m_CarryVal ^= 1;
				CLK(4);
				break;

			case 0xf6: // i_f6pre
				{
					UINT32 tmp;
					UINT32 uresult,uresult2;
					INT32 result,result2;

					m_modrm = fetch();
					tmp = GetRMByte();
					switch ( m_modrm & 0x38 )
					{
					case 0x00:  /* TEST */
						tmp &= fetch();
						m_CarryVal = m_OverVal = 0;
						set_SZPF_Byte(tmp);
						CLKM(1,2);
						break;
					case 0x08:
						logerror("%s: %06x: Undefined opcode 0xf6 0x08\n", tag(), pc());
						break;
					case 0x10:  /* NOT */
						PutbackRMByte(~tmp);
						CLKM(1,3);
						break;
					case 0x18:  /* NEG */
						m_CarryVal = (tmp!=0) ? 1 : 0;
						tmp = (~tmp)+1;
						set_SZPF_Byte(tmp);
						PutbackRMByte(tmp&0xff);
						CLKM(1,3);
						break;
					case 0x20:  /* MULU */
						uresult = m_regs.b[AL] * tmp;
						m_regs.w[AW] = (UINT16)uresult;
						m_CarryVal = m_OverVal = (m_regs.b[AH]!=0) ? 1 : 0;
						CLKM(3,4);
						break;
					case 0x28:  /* MUL */
						result = (INT16)((INT8)m_regs.b[AL])*(INT16)((INT8)tmp);
						m_regs.w[AW] = (UINT16)result;
						m_CarryVal = m_OverVal = (m_regs.b[AH]!=0) ? 1 : 0;
						CLKM(3,4);
						break;
					case 0x30:  /* DIVU */
						if (tmp)
						{
							uresult = m_regs.w[AW];
							uresult2 = uresult % tmp;
							if ((uresult /= tmp) > 0xff)
							{
								interrupt(0);
							}
							else
							{
								m_regs.b[AL] = uresult;
								m_regs.b[AH] = uresult2;
							}
						}
						else
						{
							interrupt(0);
						}
						CLKM(15,16);
						break;
					case 0x38:  /* DIV */
						if (tmp)
						{
							result = (INT16)m_regs.w[AW];
							result2 = result % (INT16)((INT8)tmp);
							if ((result /= (INT16)((INT8)tmp)) > 0xff)
							{
								interrupt(0);
							}
							else
							{
								m_regs.b[AL] = result;
								m_regs.b[AH] = result2;
							}
						}
						else
						{
							interrupt(0);
						}
						CLKM(17,18);
						break;
					}
				}
				break;


			case 0xf7: // i_f7pre
				{
					UINT32 tmp,tmp2;
					UINT32 uresult,uresult2;
					INT32 result,result2;

					m_modrm = fetch();
					tmp = GetRMWord();
					switch ( m_modrm & 0x38 )
					{
					case 0x00:  /* TEST */
						tmp2 = fetch_word();
						tmp &= tmp2;
						m_CarryVal = m_OverVal = 0;
						set_SZPF_Word(tmp);
						CLKM(1,2);
						break;
					case 0x08:
						logerror("%s: %06x: Undefined opcode 0xf7 0x08\n", tag(), pc());
						break;
					case 0x10:  /* NOT */
						PutbackRMWord(~tmp);
						CLKM(1,3);
						break;
					case 0x18:  /* NEG */
						m_CarryVal = (tmp!=0) ? 1 : 0;
						tmp = (~tmp) + 1;
						set_SZPF_Word(tmp);
						PutbackRMWord(tmp);
						CLKM(1,3);
						break;
					case 0x20:  /* MULU */
						uresult = m_regs.w[AW]*tmp;
						m_regs.w[AW] = uresult & 0xffff;
						m_regs.w[DW] = ((UINT32)uresult)>>16;
						m_CarryVal = m_OverVal = (m_regs.w[DW] != 0) ? 1 : 0;
						CLKM(3,4);
						break;
					case 0x28:  /* MUL */
						result = (INT32)((INT16)m_regs.w[AW]) * (INT32)((INT16)tmp);
						m_regs.w[AW] = result & 0xffff;
						m_regs.w[DW] = result >> 16;
						m_CarryVal = m_OverVal = (m_regs.w[DW] != 0) ? 1 : 0;
						CLKM(3,4);
						break;
					case 0x30:  /* DIVU */
						if (tmp)
						{
							uresult = (((UINT32)m_regs.w[DW]) << 16) | m_regs.w[AW];
							uresult2 = uresult % tmp;
							if ((uresult /= tmp) > 0xffff)
							{
								interrupt(0);
							}
							else
							{
								m_regs.w[AW] = uresult;
								m_regs.w[DW] = uresult2;
							}
						}
						else
						{
							interrupt(0);
						}
						CLKM(23,24);
						break;
					case 0x38:  /* DIV */
						if (tmp)
						{
							result = ((UINT32)m_regs.w[DW] << 16) + m_regs.w[AW];
							result2 = result % (INT32)((INT16)tmp);
							if ((result /= (INT32)((INT16)tmp)) > 0xffff)
							{
								interrupt(0);
							}
							else
							{
								m_regs.w[AW] = result;
								m_regs.w[DW] = result2;
							}
						}
						else
						{
							interrupt(0);
						}
						CLKM(24,25);
						break;
					}
				}
				break;


			case 0xf8: // i_clc
				m_CarryVal = 0;
				CLK(4);
				break;

			case 0xf9: // i_stc
				m_CarryVal = 1;
				CLK(4);
				break;

			case 0xfa: // i_di
				m_IF = 0;
				CLK(4);
				break;

			case 0xfb: // i_ei
				m_IF = 1;
				CLK(4);
				break;

			case 0xfc: // i_cld
				m_DF = 0;
				CLK(4);
				break;

			case 0xfd: // i_std
				m_DF = 1;
				CLK(4);
				break;

			case 0xfe: // i_fepre
				{
					UINT32 tmp, tmp1;
					m_modrm = fetch();
					tmp = GetRMByte();
					switch ( m_modrm & 0x38 )
					{
					case 0x00:  /* INC */
						tmp1 = tmp+1;
						m_OverVal = (tmp==0x7f);
						set_AF(tmp1,tmp,1);
						set_SZPF_Byte(tmp1);
						PutbackRMByte(tmp1);
						CLKM(1,3);
						break;
					case 0x08:  /* DEC */
						tmp1 = tmp-1;
						m_OverVal = (tmp==0x80);
						set_AF(tmp1,tmp,1);
						set_SZPF_Byte(tmp1);
						PutbackRMByte(tmp1);
						CLKM(1,3);
						break;
					default:
						logerror("%s: %06x: FE Pre with unimplemented mod\n", tag(), pc());
						break;
					}
				}
				break;

			case 0xff: // i_ffpre
				{
					UINT32 tmp, tmp1;
					m_modrm = fetch();
					tmp = GetRMWord();
					switch ( m_modrm & 0x38 )
					{
					case 0x00:  /* INC */
						tmp1 = tmp+1;
						m_OverVal = (tmp==0x7fff);
						set_AF(tmp1,tmp,1);
						set_SZPF_Word(tmp1);
						PutbackRMWord(tmp1);
						CLKM(1,3);
						break;
					case 0x08:  /* DEC */
						tmp1 = tmp-1;
						m_OverVal = (tmp==0x8000);
						set_AF(tmp1,tmp,1);
						set_SZPF_Word(tmp1);
						PutbackRMWord(tmp1);
						CLKM(1,3);
						break;
					case 0x10:  /* CALL */
						PUSH(m_ip);
						m_ip = tmp;
						CLKM(5,6);
						break;
					case 0x18:  /* CALL FAR */
						tmp1 = m_sregs[CS];
						m_sregs[CS] = GetnextRMWord();
						PUSH(tmp1);
						PUSH(m_ip);
						m_ip = tmp;
						CLKM(5,12);
						break;
					case 0x20:  /* JMP */
						m_ip = tmp;
						CLKM(4,5);
						break;
					case 0x28:  /* JMP FAR */
						m_ip = tmp;
						m_sregs[CS] = GetnextRMWord();
						CLK(10);
						break;
					case 0x30:
						PUSH(tmp);
						CLK(1);
						break;
					default:
						logerror("%s: %06x: FF Pre with unimplemented mod\n", tag(), pc());
						break;
					}
				}
				break;

		default:
			m_icount -= 10;
			logerror("%s: %06x: Invalid Opcode %02x\n", tag(), pc(), op);
			break;

		}
	}
}
