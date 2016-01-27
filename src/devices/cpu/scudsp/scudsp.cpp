// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Mariusz Wojcieszek
/*****************************************************************************
 *
 * scudsp.c
 * Sega SCUDSP emulator version 1.00
 *
 * copyright Angelo Salese & Mariusz Wojcieszek, all rights reserved
 *
 * Changelog:
 * 131010: Angelo Salese
 * - Converted to CPU structure
 *
 * 110807: Angelo Salese
 * - Allow the Program Counter to be read-backable from SH-2, needed by Virtua Fighter to not
 *   get stuck on "round 1" announcement;
 *
 * 110806: Angelo Salese
 * - Allows reading from non-work ram h areas;
 * - Fixed DMA add values;
 * - Fixed a MVI condition shift flag bug, now Sega Saturn produces sound during splash screen;
 * - Removed left-over IRQ;
 *
 * 110722: Angelo Salese
 * - Added DSP IRQ command, tested with "The King of Boxing"
 *
 * 110527: Angelo Salese
 * - Fixed incorrectly setted execute flag clearance, allows animation of the Sega Saturn
 *   splash screen;
 *
 * 051129: Mariusz Wojcieszek
 * - Fixed parallel instructions which increment CT registers to update CT register only
 *   once, after dsp operation is finished. This fixes instructions like
 *   MOV MC0,X MOV MC0,Y used by vfremix
 * - Changed ALU 32bit instructions to not sign extend their result when loaded to ALU.
 *   This matches Sega's dspsim behaviour.
 * - Changed DMA addnumber handling to match Sega's dspsim.
 *
 * 050813: Mariusz Wojcieszek
 * - Fixed add number in DSP DMA
 *
 * 050412: Angelo Salese
 * - Fixed the T0F behaviour in the DMA operation,it was causing an hang in Treasure Hunt
 *   due of that.
 * - Removed the dsp.log file creation when you are not using the debug build
 *
 * 041114: Angelo Salese
 * - Finished flags in ALU opcodes
 * - SR opcode: MSB does not change.
 *
 * 040328: Mariusz Wojcieszek
 * - rewritten ALU and MUL operations using signed arithmetics
 * - improved DMA
 * - fixed MOV ALH,x
 *
 * 031211: Mariusz Wojcieszek
 * - result of ALU command is stored into ALU register
 * - X-Bus command: MOV [s],X can be executed in parallel to other X-Bus commands
 * - Y-Bus command: MOV [s],Y can be executed in parallel to other Y-Bus commands
 * - Jump and LPS/BTM support:
 *   jump addresses are absolute,
 *   prefetched instructions are executed before jump is taken
 * - after each instruction, X and Y is multiplied and contents are loaded into MUL register
 * - fixed RL8
 * - fixed MVI
 * - flags computation in MVI and JMP is partly guessed (because of errors in docs)
 * - added reading DSP mem from SH2 side
 * - overworked disassembler
 *
 *  TODO:
 * - Fix INSTA_DMA hack
 * - Fix disassembler
 * - Fix timings (no info available so far)
 * - Add control flags
 * - Croc: has a bug somewhere that never allows it to trip the ENDI opcode.
 *   Snippet of interest is:
 *   08    00823500                                            CLR A     MOV M0,PL
 *   09    08040000    OR                                      MOV ALU,A
 *   0A    D208000D    JMP NZ,$D
 *   0B    00000000    NOP
 *   0C    F8000000    ENDI
 *
 *   40    00863502                                            MOV M0,A  MOV M2,PL
 *   41    10003009    ADD                                               MOV ALL,MC0
 *   42    D3400042    JMP T0,$42
 *   43    00000000    NOP
 *   44    D0000007    JMP $7
 *
 *
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
#define SF  (m_flags & 0x00400000)
#define ZF  (m_flags & 0x00200000)
#define CF  m_flags & 0x00100000
#define VF  m_flags & 0x00080000
#define EF  m_flags & 0x00040000
#define ESF m_flags & 0x00020000
#define EXF m_flags & 0x00010000 // execute flag (basically tied to RESET pin)
#define LEF m_flags & 0x00008000 // change PC value
#define T0F_1 m_flags|=0x00800000
#define T0F_0 m_flags&=~0x00800000
#define EXF_0 m_flags&=~0x00010000
#define EF_1  m_flags|=0x00040000

#define SET_C(_val) (m_flags = ((m_flags & ~0x00100000) | ((_val) ? 0x00100000 : 0)))
#define SET_S(_val) (m_flags = ((m_flags & ~0x00400000) | ((_val) ? 0x00400000 : 0)))
#define SET_Z(_val) (m_flags = ((m_flags & ~0x00200000) | ((_val) ? 0x00200000 : 0)))
#define SET_V(_val) (m_flags = ((m_flags & ~0x00080000) | ((_val) ? 0x00080000 : 0)))


#define FLAGS_MASK 0x06ff8000
#define INSTA_DMA 1

#define scudsp_readop(A) m_program->read_dword(A << 2)
#define scudsp_writeop(A, B) m_program->write_dword(A << 2, B)
#define scudsp_readmem(A,MD) m_data->read_dword((A | (MD << 6)) << 2)
#define scudsp_writemem(A,MD,B) m_data->write_dword((A | (MD << 6)) << 2, B)

UINT32 scudsp_cpu_device::scudsp_get_source_mem_reg_value( UINT32 mode )
{
	if ( mode < 0x8 )
	{
		return scudsp_get_source_mem_value( mode );
	}
	else
	{
		switch( mode )
		{
			case 0x9:
				return (UINT32)((m_alu & U64(0x00000000ffffffff)) >> 0);
			case 0xA:
				return (UINT32)((m_alu & U64(0x0000ffffffff0000)) >> 16);
		}
	}
	return 0;
}

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
		case 0x4:   /* RX */
			m_rx.ui = value;
			break;
		case 0x5:   /* PL */
			m_pl.ui = value;
			m_ph.si = (m_pl.si < 0) ? -1 : 0;
			break;
		case 0x6:   /* RA0 */
			m_ra0 = value;
			break;
		case 0x7:   /* WA0 */
			m_wa0 = value;
			break;
		case 0x8:
		case 0x9:
			/* ??? */
			break;
		case 0xa:   /* LOP */
			m_lop = value;
			break;
		case 0xb:   /* TOP */
			m_top = value;
			break;
		case 0xc:   /* CT0 */
			m_ct0 = value & 0x3f;
			break;
		case 0xd:   /* CT1 */
			m_ct1 = value & 0x3f;
			break;
		case 0xe:   /* CT2 */
			m_ct2 = value & 0x3f;
			break;
		case 0xf:   /* CT3 */
			m_ct3 = value & 0x3f;
			break;
	}
}

void scudsp_cpu_device::scudsp_set_dest_mem_reg_2( UINT32 mode, UINT32 value )
{
	if ( mode < 0xb )
	{
		scudsp_set_dest_mem_reg( mode, value );
	}
	else
	{
		switch( mode )
		{
			case 0xc:   /* PC */
				m_delay = m_pc;  /* address next after this command will be executed twice */
				m_top = m_pc;
				m_pc = value;
				break;
		}
	}
}

UINT32 scudsp_cpu_device::scudsp_compute_condition( UINT32 condition )
{
	UINT32 result = 0;

	switch( condition & 0xf )
	{
		case 0x1:   /* Z */
			result = ZF;
			break;
		case 0x2:   /* S */
			result = SF;
			break;
		case 0x3:   /* ZS */
			result = ZF | SF;
			break;
		case  0x4:  /* C */
			result = CF;
			break;
		case 0x8:   /* T0 */
			result = T0F;
			break;
	}
	if ( !(condition & 0x20) )
	{
		result = !result;
	}

	return result;
}

void scudsp_cpu_device::scudsp_set_dest_dma_mem( UINT32 memcode, UINT32 value, UINT32 counter )
{
	if ( memcode < 4 )
	{
		switch(memcode)
		{
			case 0x0:   /* MC0 */
				scudsp_writemem(((m_ct0 + counter) & 0x3f),0,value);
				break;
			case 0x1:   /* MC1 */
				scudsp_writemem(((m_ct1 + counter) & 0x3f),1,value);
				break;
			case 0x2:   /* MC2 */
				scudsp_writemem(((m_ct2 + counter) & 0x3f),2,value);
				break;
			case 0x3:   /* MC3 */
				scudsp_writemem(((m_ct3 + counter) & 0x3f),3,value);
				break;
		}
	}
	else if ( memcode == 4 )
	{
		fatalerror("scudsp_set_dest_dma_mem == 4");
		/* caused a stack overflow for sure ... */
		//dsp_reg.internal_prg[ counter & 0x100 ] = value;
	}
}

UINT32 scudsp_cpu_device::scudsp_get_mem_source_dma( UINT32 memcode, UINT32 counter )
{
	switch( memcode & 0x3 )
	{
		case 0x0:
			return scudsp_readmem(((m_ct0 + counter) & 0x3f),0);
		case 0x1:
			return scudsp_readmem(((m_ct1 + counter) & 0x3f),1);
		case 0x2:
			return scudsp_readmem(((m_ct2 + counter) & 0x3f),2);
		case 0x3:
			return scudsp_readmem(((m_ct3 + counter) & 0x3f),3);
	}
	return 0;
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

void scudsp_cpu_device::scudsp_operation(UINT32 opcode)
{
	INT64 i1,i2;
	INT32 i3;
	int update_ct[4] = {0,0,0,0};
	int dsp_mem;


	/* ALU */
	switch( (opcode & 0x3c000000) >> 26 )
	{
		case 0x0:   /* NOP */
			break;
		case 0x1:   /* AND */
			i3 = m_acl.si & m_pl.si;
			m_alu = (UINT64)(UINT32)i3;
			SET_Z(i3 == 0);
			SET_C(0);
			SET_S(i3 < 0);
			break;
		case 0x2:   /* OR */
			i3 = m_acl.si | m_pl.si;
			m_alu = (UINT64)(UINT32)i3;
			SET_C(0);
			SET_S(i3 < 0);
			/* TODO: Croc and some early Psygnosis games wants Z to be 1 when the result of this one is negative.
			         Needs HW tests ... */
			if(i3 < 0)
				i3 = 0;
			SET_Z(i3 == 0);
			break;
		case 0x3:   /* XOR */
			i3 = m_acl.si ^ m_pl.si;
			m_alu = (UINT64)(UINT32)i3;
			SET_Z(i3 == 0);
			SET_C(0);
			SET_S(i3 < 0);
			break;
		case 0x4:   /* ADD */
			i3 = m_acl.si + m_pl.si;
			m_alu = (UINT64)(UINT32)i3;
			//SET_Z(i3 == 0);
			SET_Z( (i3 & S64(0xffffffffffff)) == 0 );
			//SET_S(i3 < 0);
			SET_S( i3 & S64(0x1000000000000));
			SET_C(i3 & S64(0x100000000));
			SET_V(((i3) ^ (m_acl.si)) & ((i3) ^ (m_pl.si)) & 0x80000000);
			break;
		case 0x5:   /* SUB */
			i3 = m_acl.si - m_pl.si;
			m_alu = (UINT64)(UINT32)i3;
			SET_Z(i3 == 0);
			SET_C(i3 & S64(0x100000000));
			SET_S(i3 < 0);
			SET_V(((m_pl.si) ^ (m_acl.si)) & ((m_pl.si) ^ (i3)) & 0x80000000);
			break;
		case 0x6:   /* AD2 */
			i1 = CONCAT_64((INT32)m_ph.si,m_pl.si);
			i2 = CONCAT_64((INT32)m_ach.si,m_acl.si);
			m_alu = i1 + i2;
			SET_Z((m_alu & S64(0xffffffffffff)) == 0);
			SET_S((m_alu & S64(0x800000000000)) > 0);
			SET_C((m_alu) & S64(0x1000000000000));
			SET_V(((m_alu) ^ (i1)) & ((m_alu) ^ (i2)) & S64(0x800000000000));
			break;
		case 0x7:   /* ??? */
			/* Unrecognized opcode */
			break;
		case 0x8:   /* SR */
			i3 = (m_acl.si >> 1) | (m_acl.si & 0x80000000);/*MSB does not change*/
			m_alu = (UINT64)(UINT32)i3;
			SET_Z(i3 == 0);
			SET_S(i3 < 0);
			SET_C(m_acl.ui & 0x80000000);
			break;
		case 0x9:   /* RR */
			i3 = ((m_acl.ui >> 1) & 0x7fffffff) | ((m_acl.ui << 31) & 0x80000000);
			m_alu = (UINT64)(UINT32)i3;
			SET_Z( i3 == 0 );
			SET_S( i3 < 0 );
			SET_C( m_acl.ui & 0x1 );
			break;
		case 0xa:   /* SL */
			i3 = m_acl.si << 1;
			m_alu = (UINT64)(UINT32)i3;
			SET_Z( i3 == 0 );
			SET_S( i3 < 0 );
			SET_C( m_acl.ui & 0x80000000 );
			break;
		case 0xB:   /* RL */
			i3 = ((m_acl.si << 1) & 0xfffffffe) | ((m_acl.si >> 31) & 0x1);
			m_alu = (UINT64)(UINT32)i3;
			SET_Z( i3 == 0 );
			SET_S( i3 < 0 );
			SET_C( m_acl.ui & 0x80000000 );
			break;
		case 0xc:
		case 0xd:
		case 0xe:
			/* Unrecognized opcode */
			break;
		case 0xF:   /* RL8 */
			i3 = ((m_acl.si << 8) & 0xffffff00) | ((m_acl.si >> 24) & 0xff);
			m_alu = i3;
			SET_Z( i3 == 0 );
			SET_S( i3 < 0 );
			SET_C( m_acl.si & 0x01000000 );
			break;
	}

	/* X-Bus */
	if ( opcode & 0x2000000 )
	{
		/* MOV [s],X */
		dsp_mem = (opcode & 0x700000) >> 20;
		if ( dsp_mem & 4 )
		{
			dsp_mem &= 3;
			update_ct[dsp_mem] = 1;
		}
		m_rx.ui = scudsp_get_source_mem_value( dsp_mem );
		m_update_mul = 1;
	}
	switch( (opcode & 0x1800000) >> 23 )
	{
		case 0x0:   /* NOP */
		case 0x1:   /* NOP ? */
			break;
		case 0x2:   /* MOV MUL,P */
			m_ph.ui = (UINT16)((m_mul & U64(0x0000ffff00000000)) >> 32);
			m_pl.ui = (UINT32)((m_mul & U64(0x00000000ffffffff)) >> 0);
			break;
		case 0x3:   /* MOV [s],P */
			dsp_mem = (opcode & 0x700000) >> 20;
			if ( dsp_mem & 4 )
			{
				dsp_mem &= 3;
				update_ct[dsp_mem] = 1;
			}
			m_pl.ui = scudsp_get_source_mem_value(  dsp_mem );
			m_ph.si = (m_pl.si < 0) ? -1 : 0;
			break;
	}

	/* Y-Bus */
	if ( opcode & 0x80000 )
	{
		/* MOV [s],Y */
		dsp_mem = (opcode & 0x1C000 ) >> 14;
		if (dsp_mem & 4)
		{
			dsp_mem &= 3;
			update_ct[dsp_mem] = 1;
		}
		m_ry.ui = scudsp_get_source_mem_value( dsp_mem );
		m_update_mul = 1;
	}
	switch( (opcode & 0x60000) >> 17 )
	{
		case 0x0:   /* NOP */
			break;
		case 0x1:   /* CLR A */
			m_acl.ui = 0;
			m_ach.ui = 0;
			break;
		case 0x2:   /* MOV ALU,A */
			m_ach.ui = (UINT16)((m_alu & U64(0x0000ffff00000000)) >> 32);
			m_acl.ui = (UINT32)((m_alu & U64(0x00000000ffffffff)) >> 0);
			break;
		case 0x3:   /* MOV [s], A */
			dsp_mem = (opcode & 0x1C000 ) >> 14;
			if (dsp_mem & 4)
			{
				dsp_mem &= 3;
				update_ct[dsp_mem] = 1;
			}
			m_acl.ui = scudsp_get_source_mem_value( dsp_mem );
			m_ach.si = ((m_acl.si < 0) ? -1 : 0);
			break;
	}

	/* update CT registers */
	if ( update_ct[0] ) { m_ct0++; m_ct0 &= 0x3f; };
	if ( update_ct[1] ) { m_ct1++; m_ct1 &= 0x3f; };
	if ( update_ct[2] ) { m_ct2++; m_ct2 &= 0x3f; };
	if ( update_ct[3] ) { m_ct3++; m_ct3 &= 0x3f; };


	/* D1-Bus */
	switch( (opcode & 0x3000) >> 12 )
	{
		case 0x0:   /* NOP */
			break;
		case 0x1:   /* MOV SImm,[d] */
			scudsp_set_dest_mem_reg( (opcode & 0xf00) >> 8, (INT32)(INT8)(opcode & 0xff) );
			break;
		case 0x2:
			/* ??? */
			break;
		case 0x3:   /* MOV [s],[d] */
			scudsp_set_dest_mem_reg( (opcode & 0xf00) >> 8, scudsp_get_source_mem_reg_value( opcode & 0xf ) );
			break;
	}

	m_icount -= 1;
}

void scudsp_cpu_device::scudsp_move_immediate( UINT32 opcode )
{
	UINT32 value;

	if ( opcode & 0x2000000 )
	{
		if ( scudsp_compute_condition( (opcode & 0x3F80000 ) >> 19 ) )
		{
			value = opcode & 0x7ffff;
			if ( value & 0x40000 ) value |= 0xfff80000;
			scudsp_set_dest_mem_reg_2( (opcode & 0x3C000000) >> 26, value );
		}
	}
	else
	{
		value = opcode & 0x1ffffff;
		if ( value & 0x1000000 ) value |= 0xfe000000;
		scudsp_set_dest_mem_reg_2( (opcode & 0x3C000000) >> 26, value );
	}
	m_icount -= 1;
}

void scudsp_cpu_device::scudsp_dma( UINT32 opcode )
{
	UINT8 hold = (opcode &  0x4000) >> 14;
	UINT32 add = (opcode & 0x38000) >> 15;
	UINT32 dir_from_D0 = (opcode & 0x1000 ) >> 12;
	UINT32 dsp_mem = (opcode & 0x300) >> 8;

	T0F_1;

	if ( opcode & 0x2000 )
	{
		m_dma.size = scudsp_get_source_mem_value( opcode & 0xf );
		switch ( add & 0x7 )
		{
			case 0: m_dma.add = 0; break;
			case 1: m_dma.add = 4; break;
			default: m_dma.add = 4; break;
		}
	}
	else
	{
		m_dma.size = opcode & 0xff;
		switch( add )
		{
			case 0: m_dma.add = 0; break;  /* 0 */
			case 1: m_dma.add = 4; break;  /* 1 */
			case 2: m_dma.add = 4; break;  /* 2 */
			case 3: m_dma.add = 16; break; /* 4 */
			case 4: m_dma.add = 16; break;  /* 8 */
			case 5: m_dma.add = 64; break; /* 16 */
			case 6: m_dma.add = 128; break; /* 32 */
			case 7: m_dma.add = 256; break; /* 64 */
		}
	}

	m_dma.dir = dir_from_D0;
	if ( m_dma.dir == 0 )
	{
		m_dma.src = (m_ra0 << 2) & 0x07ffffff;
		m_dma.dst = dsp_mem;
	}
	else
	{
		m_dma.src = dsp_mem;
		m_dma.dst = (m_wa0 << 2) & 0x07ffffff;
	}

	m_dma.update = ( hold == 0 );
	m_dma.ex = 1;
	m_dma.count = 0;
	/* HACK ALERT: It looks like that scheduling craps out the m_dma parameters, why this happens I don't know ... */
	#if INSTA_DMA
	{
		UINT32 data;
		if ( m_dma.dir == 0 )
		{
			for(m_dma.count = 0;m_dma.count < m_dma.size; m_dma.count++)
			{
				data = (m_in_dma_cb(m_dma.src)<<16) | m_in_dma_cb(m_dma.src+2);
				scudsp_set_dest_dma_mem( m_dma.dst, data, m_dma.count );

				m_dma.src += m_dma.add;

				if ( m_dma.update )
				{
					m_ra0 += ((1 * m_dma.add) >> 2);
				}
			}
		}
		else
		{
			for(m_dma.count = 0;m_dma.count < m_dma.size; m_dma.count++)
			{
				data = scudsp_get_mem_source_dma( m_dma.src, m_dma.count );

				m_out_dma_cb(m_dma.dst, data >> 16 );
				m_out_dma_cb(m_dma.dst+2, data & 0xffff );

				m_dma.dst += m_dma.add;

				if ( m_dma.update )
				{
					m_wa0 += ((1 * m_dma.add) >> 2);
				}
			}
		}

		//if(m_dma.count >= m_dma.size)
		{
			m_dma.ex = 0;
			T0F_0;
		}

		m_icount -= m_dma.size;
	}
	#endif


	//printf("SRC %08x DST %08x SIZE %08x UPDATE %08x DIR %08x ADD %08x\n",m_dma.src,m_dma.dst,m_dma.size,m_dma.update,m_dma.dir,m_dma.add);

	m_icount -= 1;
}

void scudsp_cpu_device::scudsp_jump( UINT32 opcode )
{
	if ( opcode & 0x3f80000 )
	{
		if ( scudsp_compute_condition( (opcode & 0x3f80000) >> 19 ) )
		{
			m_delay = m_pc;
			m_pc = opcode & 0xff;
		}
	}
	else
	{
		m_delay = m_pc;
		m_pc = opcode & 0xff;
	}

	m_icount -= 1;
}

void scudsp_cpu_device::scudsp_loop(UINT32 opcode)
{
	if ( opcode & 0x8000000 )
	{
		/* LPS */
		if ( m_lop != 0 )
		{
			m_lop--;
			m_delay = m_pc;
			m_pc--;
		}
	}
	else
	{
		/* BTM */
		if ( m_lop != 0 )
		{
			m_lop--;
			m_delay = m_pc;
			m_pc = m_top;
		}
	}
	m_icount -= 1;
}

void scudsp_cpu_device::scudsp_end(UINT32 opcode)
{
	if(opcode & 0x08000000)
	{
		/*ENDI*/
		EF_1;
		m_out_irq_cb(1);
	}

	EXF_0; /* END / ENDI */
	set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_icount -= 1;
}

void scudsp_cpu_device::scudsp_illegal(UINT32 opcode)
{
	fatalerror("scudsp illegal opcode at 0x%04x\n", m_pc);
	m_icount -= 1;
}

void scudsp_cpu_device::scudsp_exec_dma()
{
	UINT32 data;
	if ( m_dma.dir == 0 )
	{
		data = (m_in_dma_cb(m_dma.src)<<16) | m_in_dma_cb(m_dma.src+2);
		scudsp_set_dest_dma_mem( m_dma.dst, data, m_dma.count );

		m_dma.src += m_dma.add;

		if ( m_dma.update )
		{
			m_ra0 += ((1 * m_dma.add) >> 2);
		}
	}
	else
	{
		data = scudsp_get_mem_source_dma( m_dma.src, m_dma.count );

		m_out_dma_cb(m_dma.dst, data >> 16 );
		m_out_dma_cb(m_dma.dst+2, data & 0xffff );

		m_dma.dst += m_dma.add;

		if ( m_dma.update )
		{
			m_wa0 += ((1 * m_dma.add) >> 2);
		}
	}

	m_dma.count++;
	if(m_dma.count >= m_dma.size)
	{
		m_dma.ex = 0;
		T0F_0;
	}

	m_icount -= 1;
}

/* Execute cycles */
void scudsp_cpu_device::execute_run()
{
	UINT32 opcode;

	do
	{
		m_update_mul = 0;

		debugger_instruction_hook(this, m_pc);

		if ( m_delay )
		{
			opcode = scudsp_readop(m_delay);
			m_delay = 0;
		}
		else
		{
			opcode = scudsp_readop(m_pc);
			m_pc++;
		}

		switch( (opcode & 0xc0000000) >> 30 )
		{
			case 0x00: /* 00 */
				scudsp_operation(opcode);
				break;
			case 0x01: /* 01 */
				scudsp_illegal(opcode);
				break;
			case 0x02: /* 10 */
				scudsp_move_immediate(opcode);
				break;
			case 0x03: /* 11 */
				switch( (opcode & 0x30000000) >> 28 )
				{
					case 0x00:
						scudsp_dma(opcode);
						break;
					case 0x01:
						scudsp_jump(opcode);
						break;
					case 0x02:
						scudsp_loop(opcode);
						break;
					case 0x03:
						scudsp_end(opcode);
						break;
				}
				break;
		}

		if ( m_update_mul == 1 )
		{
			m_mul = (INT64)m_rx.si * (INT64)m_ry.si;
			m_update_mul = 0;
		}

		if (m_dma.ex == 1)
		{
			scudsp_exec_dma();
		}

	} while( m_icount > 0 );
}

void scudsp_cpu_device::device_start()
{
	m_pc = 0;
	m_flags = 0;
	m_delay = 0;
	m_top = 0;
	m_lop = 0;
	memset(&m_rx, 0x00, sizeof(m_rx));
	m_mul = 0;
	memset(&m_ry, 0x00, sizeof(m_ry));
	m_alu = 0;
	memset(&m_ph, 0x00, sizeof(m_ph));
	memset(&m_pl, 0x00, sizeof(m_pl));
	memset(&m_ach, 0x00, sizeof(m_ach));
	memset(&m_acl, 0x00, sizeof(m_acl));
	m_ra0 = 0;
	m_wa0 = 0;
	m_ra = 0;
	m_ct0 = 0;
	m_ct1 = 0;
	m_ct2 = 0;
	m_ct3 = 0;
	memset(&m_dma, 0x00, sizeof(m_dma));

	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	save_item(NAME(m_pc));
	save_item(NAME(m_ra));

	save_item(NAME(m_ct0));
	save_item(NAME(m_ct1));
	save_item(NAME(m_ct2));
	save_item(NAME(m_ct3));

	save_item(NAME(m_flags));
	save_item(NAME(m_delay));

	save_item(NAME(m_top));
	save_item(NAME(m_lop));
	save_item(NAME(m_rx.ui));

	save_item(NAME(m_mul));

	save_item(NAME(m_ry.ui));

	save_item(NAME(m_alu));
	save_item(NAME(m_ph.ui));
	save_item(NAME(m_pl.ui));
	save_item(NAME(m_ach.ui));
	save_item(NAME(m_acl.ui));
	save_item(NAME(m_ra0));
	save_item(NAME(m_wa0));

	save_item(NAME(m_dma.src));
	save_item(NAME(m_dma.dst));
	save_item(NAME(m_dma.size));

	// Register state for debugger
	state_add( SCUDSP_PC, "PC", m_pc ).formatstr("%02X");
	state_add( SCUDSP_FLAGS, "SR", m_flags ).formatstr("%08X");
	state_add( SCUDSP_DELAY, "DELAY", m_delay ).formatstr("%02X").noshow();
	state_add( SCUDSP_TOP, "TOP", m_top).formatstr("%02X");
	state_add( SCUDSP_LOP, "LOP", m_lop).formatstr("%03X");
	state_add( SCUDSP_RX, "RX", m_rx).formatstr("%08X");
	state_add( SCUDSP_MUL, "MUL", m_mul).formatstr("%012X");
	state_add( SCUDSP_RY, "RY", m_ry).formatstr("%08X");
	state_add( SCUDSP_ALU, "ALU", m_alu).formatstr("%012X");
	state_add( SCUDSP_PH, "PH", m_ph).formatstr("%04X");
	state_add( SCUDSP_PL, "PL", m_pl).formatstr("%08X");
	state_add( SCUDSP_ACH, "ACH", m_ach).formatstr("%04X");
	state_add( SCUDSP_ACL, "ACL", m_acl).formatstr("%08X");
	state_add( SCUDSP_RA0, "RA0", m_ra0).formatstr("%08X");
	state_add( SCUDSP_WA0, "WA0", m_wa0).formatstr("%08X");
	state_add( SCUDSP_RA, "RA", m_ra ).formatstr("%02X");
	state_add( SCUDSP_CT0, "CT0", m_ct0 ).formatstr("%02X");
	state_add( SCUDSP_CT1, "CT1", m_ct1 ).formatstr("%02X");
	state_add( SCUDSP_CT2, "CT2", m_ct2 ).formatstr("%02X");
	state_add( SCUDSP_CT3, "CT3", m_ct3 ).formatstr("%02X");
	state_add( STATE_GENPC, "curpc", m_pc ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_flags ).formatstr("%17s").noshow();

	m_out_irq_cb.resolve_safe();
	m_in_dma_cb.resolve_safe(0);
	m_out_dma_cb.resolve_safe();

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
			//m_reset_state = state;
			break;
	}
}

scudsp_cpu_device::scudsp_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SCUDSP, "SCUDSP", tag, owner, clock, "scudsp", __FILE__)
	, m_out_irq_cb(*this)
	, m_in_dma_cb(*this)
	, m_out_dma_cb(*this)
	, m_program_config("program", ENDIANNESS_BIG, 32, 8, -2)
	, m_data_config("data", ENDIANNESS_BIG, 32, 8, -2)
{
}


void scudsp_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%s%s%s%c%c%c%c%c%s%s%s",
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
