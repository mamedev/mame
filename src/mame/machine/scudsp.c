/******************************************************************************************
System Control Unit - DSP emulator version 0.09

Written by Angelo Salese & Mariusz Wojcieszek

Changelog:
110807: Angelo Salese
- Allow the Program Counter to be read-backable from SH-2, needed by Virtua Fighter to not
  get stuck on "round 1" announcement;

110806: Angelo Salese
- Allows reading from non-work ram h areas;
- Fixed DMA add values;
- Fixed a MVI condition shift flag bug, now Sega Saturn produces sound during splash screen;
- Removed left-over IRQ;

110722: Angelo Salese
- Added DSP IRQ command, tested with "The King of Boxing"

110527: Angelo Salese
- Fixed incorrectly setted execute flag clearance, allows animation of the Sega Saturn
  splash screen;

051129: Mariusz Wojcieszek
- Fixed parallel instructions which increment CT registers to update CT register only
  once, after dsp operation is finished. This fixes instructions like
  MOV MC0,X MOV MC0,Y used by vfremix
- Changed ALU 32bit instructions to not sign extend their result when loaded to ALU.
  This matches Sega's dspsim behaviour.
- Changed DMA addnumber handling to match Sega's dspsim.

050813: Mariusz Wojcieszek
- Fixed add number in DSP DMA

050412: Angelo Salese
- Fixed the T0F behaviour in the DMA operation,it was causing an hang in Treasure Hunt
  due of that.
- Removed the dsp.log file creation when you are not using the debug build

041114: Angelo Salese
- Finished flags in ALU opcodes
- SR opcode: MSB does not change.

040328: Mariusz Wojcieszek
- rewritten ALU and MUL operations using signed arithmetics
- improved DMA
- fixed MOV ALH,x

031211: Mariusz Wojcieszek
- result of ALU command is stored into ALU register
- X-Bus command: MOV [s],X can be executed in parallel to other X-Bus commands
- Y-Bus command: MOV [s],Y can be executed in parallel to other Y-Bus commands
- Jump and LPS/BTM support:
    jump addresses are absolute,
    prefetched instructions are executed before jump is taken
- after each instruction, X and Y is multiplied and contents are loaded into MUL register
- fixed RL8
- fixed MVI
- flags computation in MVI and JMP is partly guessed (because of errors in docs)
- added reading DSP mem from SH2 side
- overworked disassembler

 TODO:
- Convert this to cpu structure
- Disassembler: complete it
- Add control flags

******************************************************************************************/
#include "emu.h"
#include "machine/scudsp.h"
#include "sound/scsp.h"
#include "includes/stv.h"

/*DSP macros*/
#define PRF ((state->m_scu_regs[32] & 0x04000000) >> 26)
#define EPF ((state->m_scu_regs[32] & 0x02000000) >> 25)
#define T0F ((state->m_scu_regs[32] & 0x00800000) >> 23)
#define SF  ((state->m_scu_regs[32] & 0x00400000) >> 22)
#define ZF  ((state->m_scu_regs[32] & 0x00200000) >> 21)
#define CF  ((state->m_scu_regs[32] & 0x00100000) >> 20)
#define VF  ((state->m_scu_regs[32] & 0x00080000) >> 19)
#define EF  ((state->m_scu_regs[32] & 0x00040000) >> 18)
#define ESF ((state->m_scu_regs[32] & 0x00020000) >> 17)
#define EXF ((state->m_scu_regs[32] & 0x00010000) >> 16)
#define LEF ((state->m_scu_regs[32] & 0x00008000) >> 15)
#define T0F_1 state->m_scu_regs[32]|=0x00800000
#define T0F_0 state->m_scu_regs[32]&=~0x00800000
#define EXF_0 state->m_scu_regs[32]&=~0x00010000
#define EF_1  state->m_scu_regs[32]|=0x00040000

#define SET_C(_val) (state->m_scu_regs[32] = ((state->m_scu_regs[32] & ~0x00100000) | ((_val) ? 0x00100000 : 0)))
#define SET_S(_val) (state->m_scu_regs[32] = ((state->m_scu_regs[32] & ~0x00400000) | ((_val) ? 0x00400000 : 0)))
#define SET_Z(_val) (state->m_scu_regs[32] = ((state->m_scu_regs[32] & ~0x00200000) | ((_val) ? 0x00200000 : 0)))
#define SET_V(_val) (state->m_scu_regs[32] = ((state->m_scu_regs[32] & ~0x00080000) | ((_val) ? 0x00080000 : 0)))

union SCUDSPREG32 {
	INT32  si;
	UINT32 ui;
};

union SCUDSPREG16 {
	INT16  si;
	UINT16 ui;
};

static struct {
		UINT8 pc;                                      /*Program Counter*/
		UINT8 delay;                                       /* Delay */
		UINT8 top;                                     /*Jump Command memory*/
		UINT16 lop;                                    /*Counter Register*/   /*12-bits*/
		UINT8  ct0,ct1,ct2,ct3;                        /*Index for RAM*/      /*6-bits */
		UINT32 md0[0x40],md1[0x40],md2[0x40],md3[0x40]; /*RAM memory*/
		UINT8  ra;                                     /*RAM selector*/
		SCUDSPREG32 rx;                                /*X-Bus register*/
		INT64 mul;                                      /*Multiplier register*//*48-bits*/
		SCUDSPREG32 ry;                                /*Y-Bus register*/
		INT64  alu;                                     /*ALU register*/       /*48-bits*/
		SCUDSPREG16 ph;                                /*ALU high register*/
		SCUDSPREG32 pl;                                /*ALU low register*/
		SCUDSPREG16 ach;                                   /*ALU external high register*/
		SCUDSPREG32 acl;                                   /*ALU external low register*/
		UINT32 ra0,wa0;                                /*DSP DMA registers*/
		UINT32 internal_prg[0x100];
} dsp_reg;

static UINT32 opcode;
static UINT8 update_mul = 0;

#ifdef MAME_DEBUG
#define DEBUG_DSP 1
#else
#define DEBUG_DSP 0
#endif

static char dasm_buffer[100];
#if DEBUG_DSP
static FILE *log_file = NULL;
#endif
static void dsp_dasm_opcode( UINT32 op, char *buffer );



static UINT32 dsp_get_source_mem_value( UINT32 mode )
{
	UINT32 value = 0;

	switch( mode )
	{
		case 0x0:   /* M0 */
			value = dsp_reg.md0[ dsp_reg.ct0 ];
			break;
		case 0x1:   /* M1 */
			value = dsp_reg.md1[ dsp_reg.ct1 ];
			break;
		case 0x2:   /* M2 */
			value = dsp_reg.md2[ dsp_reg.ct2 ];
			break;
		case 0x3:   /* M3 */
			value = dsp_reg.md3[ dsp_reg.ct3 ];
			break;
		case 0x4:   /* MC0 */
			value = dsp_reg.md0[ dsp_reg.ct0++ ];
			dsp_reg.ct0 &= 0x3f;
			break;
		case 0x5:   /* MC1 */
			value = dsp_reg.md1[ dsp_reg.ct1++ ];
			dsp_reg.ct1 &= 0x3f;
			break;
		case 0x6:   /* MC2 */
			value = dsp_reg.md2[ dsp_reg.ct2++ ];
			dsp_reg.ct2 &= 0x3f;
			break;
		case 0x7:   /* MC3 */
			value = dsp_reg.md3[ dsp_reg.ct3++ ];
			dsp_reg.ct3 &= 0x3f;
			break;
	}
	return value;
}

static UINT32 dsp_get_source_mem_reg_value( UINT32 mode )
{
	if ( mode < 0x8 )
	{
		return dsp_get_source_mem_value( mode );
	}
	else
	{
		switch( mode )
		{
			case 0x9:
				return (UINT32)((dsp_reg.alu & U64(0x00000000ffffffff)) >> 0);
			case 0xA:
				return (UINT32)((dsp_reg.alu & U64(0x0000ffffffff0000)) >> 16);
		}
	}
	return 0;
}

static void dsp_set_dest_mem_reg( UINT32 mode, UINT32 value )
{
	switch( mode )
	{
		case 0x0:   /* MC0 */
			dsp_reg.md0[ dsp_reg.ct0++ ] = value;
			dsp_reg.ct0 &= 0x3f;
			break;
		case 0x1:   /* MC1 */
			dsp_reg.md1[ dsp_reg.ct1++ ] = value;
			dsp_reg.ct1 &= 0x3f;
			break;
		case 0x2:   /* MC2 */
			dsp_reg.md2[ dsp_reg.ct2++ ] = value;
			dsp_reg.ct2 &= 0x3f;
			break;
		case 0x3:   /* MC3 */
			dsp_reg.md3[ dsp_reg.ct3++ ] = value;
			dsp_reg.ct3 &= 0x3f;
			break;
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
	}
}

static void dsp_set_dest_mem_reg_2( UINT32 mode, UINT32 value )
{
	if ( mode < 0xb )
	{
		dsp_set_dest_mem_reg( mode, value );
	}
	else
	{
		switch( mode )
		{
			case 0xc:   /* PC */
				dsp_reg.delay = dsp_reg.pc;  /* address next after this command will be executed twice */
				dsp_reg.top = dsp_reg.pc;
				dsp_reg.pc = value;
				break;
		}
	}
}

static UINT32 dsp_compute_condition( address_space &space, UINT32 condition )
{
	saturn_state *state = space.machine().driver_data<saturn_state>();
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

static void dsp_set_dest_dma_mem( UINT32 memcode, UINT32 value, UINT32 counter )
{
	if ( memcode < 4 )
	{
		dsp_set_dest_mem_reg( memcode, value );
	}
	else if ( memcode == 4 )
	{
		dsp_reg.internal_prg[ counter & 0x100 ] = value;
	}
}

static UINT32 dsp_get_mem_source_dma( UINT32 memcode, UINT32 counter )
{
	switch( memcode & 0x3 )
	{
		case 0x0:
			return dsp_reg.md0[ (dsp_reg.ct0 + counter) & 0x3f ];
		case 0x1:
			return dsp_reg.md1[ (dsp_reg.ct1 + counter) & 0x3f ];
		case 0x2:
			return dsp_reg.md2[ (dsp_reg.ct2 + counter) & 0x3f ];
		case 0x3:
			return dsp_reg.md3[ (dsp_reg.ct3 + counter) & 0x3f ];
	}
	return 0;
}

UINT32 dsp_prg_ctrl_r(address_space &space)
{
	saturn_state *state = space.machine().driver_data<saturn_state>();

	return (state->m_scu_regs[0x80/4] & 0x06ff8000) | (dsp_reg.pc & 0xff);
}

void dsp_prg_ctrl_w(address_space &space, UINT32 data)
{
	saturn_state *state = space.machine().driver_data<saturn_state>();

	if(LEF) dsp_reg.pc = (data & 0xff);
	if(EXF) dsp_execute_program(space);
}

void dsp_prg_data(UINT32 data)
{
	dsp_reg.internal_prg[dsp_reg.pc] = data;
	dsp_reg.pc++;
}

void dsp_ram_addr_ctrl(UINT32 data)
{
	dsp_reg.ra = data & 0xff;

	switch((dsp_reg.ra & 0xc0) >> 6)
	{
		case 0: dsp_reg.ct0 = (dsp_reg.ra & 0x3f); break;
		case 1: dsp_reg.ct1 = (dsp_reg.ra & 0x3f); break;
		case 2: dsp_reg.ct2 = (dsp_reg.ra & 0x3f); break;
		case 3: dsp_reg.ct3 = (dsp_reg.ra & 0x3f); break;
	}
}

void dsp_ram_addr_w(UINT32 data)
{
#if DEBUG_DSP
	if ( log_file == NULL )
	{
		log_file = fopen( "dsp.log", "a" );
	}
	fprintf( log_file, "DSP: Writing mem %02X %04X\n", dsp_reg.ra & 0xff, data );
#endif
	dsp_set_dest_mem_reg( (dsp_reg.ra & 0xc0) >> 6, data );
}

UINT32 dsp_ram_addr_r()
{
	UINT32 data;

	data = dsp_get_source_mem_value( ((dsp_reg.ra & 0xc0) >> 6) + 4 );
#if DEBUG_DSP
	if ( log_file == NULL )
	{
		log_file = fopen( "dsp.log", "a" );
	}
	fprintf( log_file, "DSP: Reading mem at %02X %04X\n", dsp_reg.ra & 0xff, data );
#endif
	return data;
}

static void dsp_operation(address_space &space)
{
	saturn_state *state = space.machine().driver_data<saturn_state>();
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
			i3 = dsp_reg.acl.si & dsp_reg.pl.si;
			dsp_reg.alu = (UINT64)(UINT32)i3;
			SET_Z(i3 == 0);
			SET_C(0);
			SET_S(i3 < 0);
			break;
		case 0x2:   /* OR */
			i3 = dsp_reg.acl.si | dsp_reg.pl.si;
			dsp_reg.alu = (UINT64)(UINT32)i3;
			SET_Z(i3 == 0);
			SET_C(0);
			SET_S(i3 < 0);
			break;
		case 0x3:   /* XOR */
			i3 = dsp_reg.acl.si ^ dsp_reg.pl.si;
			dsp_reg.alu = (UINT64)(UINT32)i3;
			SET_Z(i3 == 0);
			SET_C(0);
			SET_S(i3 < 0);
			break;
		case 0x4:   /* ADD */
			i3 = dsp_reg.acl.si + dsp_reg.pl.si;
			dsp_reg.alu = (UINT64)(UINT32)i3;
			//SET_Z(i3 == 0);
			SET_Z( (i3 & S64(0xffffffffffff)) == 0 );
			//SET_S(i3 < 0);
			SET_S( i3 & S64(0x1000000000000));
			SET_C(i3 & S64(0x100000000));
			SET_V(((i3) ^ (dsp_reg.acl.si)) & ((i3) ^ (dsp_reg.pl.si)) & 0x80000000);
			break;
		case 0x5:   /* SUB */
			i3 = dsp_reg.acl.si - dsp_reg.pl.si;
			dsp_reg.alu = (UINT64)(UINT32)i3;
			SET_Z(i3 == 0);
			SET_C(i3 & S64(0x100000000));
			SET_S(i3 < 0);
			SET_V(((dsp_reg.pl.si) ^ (dsp_reg.acl.si)) & ((dsp_reg.pl.si) ^ (i3)) & 0x80000000);
			break;
		case 0x6:   /* AD2 */
			i1 = CONCAT_64((INT32)dsp_reg.ph.si,dsp_reg.pl.si);
			i2 = CONCAT_64((INT32)dsp_reg.ach.si,dsp_reg.acl.si);
			dsp_reg.alu = i1 + i2;
			SET_Z((dsp_reg.alu & S64(0xffffffffffff)) == 0);
			SET_S((dsp_reg.alu & S64(0x800000000000)) > 0);
			SET_C((dsp_reg.alu) & S64(0x1000000000000));
			SET_V(((dsp_reg.alu) ^ (i1)) & ((dsp_reg.alu) ^ (i2)) & S64(0x800000000000));
			break;
		case 0x7:   /* ??? */
			/* Unrecognized opcode */
			break;
		case 0x8:   /* SR */
			i3 = (dsp_reg.acl.si >> 1) | (dsp_reg.acl.si & 0x80000000);/*MSB does not change*/
			dsp_reg.alu = (UINT64)(UINT32)i3;
			SET_Z(i3 == 0);
			SET_S(i3 < 0);
			SET_C(dsp_reg.acl.ui & 0x80000000);
			break;
		case 0x9:   /* RR */
			i3 = ((dsp_reg.acl.ui >> 1) & 0x7fffffff) | ((dsp_reg.acl.ui << 31) & 0x80000000);
			dsp_reg.alu = (UINT64)(UINT32)i3;
			SET_Z( i3 == 0 );
			SET_S( i3 < 0 );
			SET_C( dsp_reg.acl.ui & 0x1 );
			break;
		case 0xa:   /* SL */
			i3 = dsp_reg.acl.si << 1;
			dsp_reg.alu = (UINT64)(UINT32)i3;
			SET_Z( i3 == 0 );
			SET_S( i3 < 0 );
			SET_C( dsp_reg.acl.ui & 0x80000000 );
			break;
		case 0xB:   /* RL */
			i3 = ((dsp_reg.acl.si << 1) & 0xfffffffe) | ((dsp_reg.acl.si >> 31) & 0x1);
			dsp_reg.alu = (UINT64)(UINT32)i3;
			SET_Z( i3 == 0 );
			SET_S( i3 < 0 );
			SET_C( dsp_reg.acl.ui & 0x80000000 );
			break;
		case 0xc:
		case 0xd:
		case 0xe:
			/* Unrecognized opcode */
			break;
		case 0xF:   /* RL8 */
			i3 = ((dsp_reg.acl.si << 8) & 0xffffff00) | ((dsp_reg.acl.si >> 24) & 0xff);
			dsp_reg.alu = i3;
			SET_Z( i3 == 0 );
			SET_S( i3 < 0 );
			SET_C( dsp_reg.acl.si & 0x01000000 );
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
		dsp_reg.rx.ui = dsp_get_source_mem_value( dsp_mem );
		update_mul = 1;
	}
	switch( (opcode & 0x1800000) >> 23 )
	{
		case 0x0:   /* NOP */
		case 0x1:   /* NOP ? */
			break;
		case 0x2:   /* MOV MUL,P */
			dsp_reg.ph.ui = (UINT16)((dsp_reg.mul & U64(0x0000ffff00000000)) >> 32);
			dsp_reg.pl.ui = (UINT32)((dsp_reg.mul & U64(0x00000000ffffffff)) >> 0);
			break;
		case 0x3:   /* MOV [s],P */
			dsp_mem = (opcode & 0x700000) >> 20;
			if ( dsp_mem & 4 )
			{
				dsp_mem &= 3;
				update_ct[dsp_mem] = 1;
			}
			dsp_reg.pl.ui = dsp_get_source_mem_value(  dsp_mem );
			dsp_reg.ph.si = (dsp_reg.pl.si < 0) ? -1 : 0;
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
		dsp_reg.ry.ui = dsp_get_source_mem_value( dsp_mem );
		update_mul = 1;
	}
	switch( (opcode & 0x60000) >> 17 )
	{
		case 0x0:   /* NOP */
			break;
		case 0x1:   /* CLR A */
			dsp_reg.acl.ui = 0;
			dsp_reg.ach.ui = 0;
			break;
		case 0x2:   /* MOV ALU,A */
			dsp_reg.ach.ui = (UINT16)((dsp_reg.alu & U64(0x0000ffff00000000)) >> 32);
			dsp_reg.acl.ui = (UINT32)((dsp_reg.alu & U64(0x00000000ffffffff)) >> 0);
			break;
		case 0x3:   /* MOV [s], A */
			dsp_mem = (opcode & 0x1C000 ) >> 14;
			if (dsp_mem & 4)
			{
				dsp_mem &= 3;
				update_ct[dsp_mem] = 1;
			}
			dsp_reg.acl.ui = dsp_get_source_mem_value( dsp_mem );
			dsp_reg.ach.si = ((dsp_reg.acl.si < 0) ? -1 : 0);
			break;
	}

	/* update CT registers */
	if ( update_ct[0] ) { dsp_reg.ct0++; dsp_reg.ct0 &= 0x3f; };
	if ( update_ct[1] ) { dsp_reg.ct1++; dsp_reg.ct1 &= 0x3f; };
	if ( update_ct[2] ) { dsp_reg.ct2++; dsp_reg.ct2 &= 0x3f; };
	if ( update_ct[3] ) { dsp_reg.ct3++; dsp_reg.ct3 &= 0x3f; };


	/* D1-Bus */
	switch( (opcode & 0x3000) >> 12 )
	{
		case 0x0:   /* NOP */
			break;
		case 0x1:   /* MOV SImm,[d] */
			dsp_set_dest_mem_reg( (opcode & 0xf00) >> 8, (INT32)(INT8)(opcode & 0xff) );
			break;
		case 0x2:
			/* ??? */
			break;
		case 0x3:   /* MOV [s],[d] */
			dsp_set_dest_mem_reg( (opcode & 0xf00) >> 8, dsp_get_source_mem_reg_value( opcode & 0xf ) );
			break;
	}

}

static void dsp_move_immediate( address_space &space )
{
	UINT32 value;

	if ( opcode & 0x2000000 )
	{
		if ( dsp_compute_condition( space, (opcode & 0x3F80000 ) >> 19 ) )
		{
			value = opcode & 0x7ffff;
			if ( value & 0x40000 ) value |= 0xfff80000;
			dsp_set_dest_mem_reg_2( (opcode & 0x3C000000) >> 26, value );
		}
	}
	else
	{
		value = opcode & 0x1ffffff;
		if ( value & 0x1000000 ) value |= 0xfe000000;
		dsp_set_dest_mem_reg_2( (opcode & 0x3C000000) >> 26, value );
	}
}


static void dsp_dma( address_space &space )
{
	saturn_state *state = space.machine().driver_data<saturn_state>();

	UINT8 hold = (opcode &  0x4000) >> 14;
	UINT32 add = (opcode & 0x38000) >> 15;
	UINT32 dir_from_D0 = (opcode & 0x1000 ) >> 12;
	UINT32 transfer_cnt = 0;
	UINT32 source = 0, dest = 0;
	UINT32 dsp_mem = (opcode & 0x300) >> 8;
	UINT32 counter = 0;
	UINT32 data;


	T0F_1;

	if ( opcode & 0x2000 )
	{
		transfer_cnt = dsp_get_source_mem_value( opcode & 0xf );
		switch ( add & 0x7 )
		{
			case 0: add = 0; break;
			case 1: add = 4; break;
			default: add = 4; break;
		}
	}
	else
	{
		transfer_cnt = opcode & 0xff;
		switch( add )
		{
			case 0: add = 0; break;  /* 0 */
			case 1: add = 4; break;  /* 1 */
			case 2: add = 4; break;  /* 2 */
			case 3: add = 16; break; /* 4 */
			case 4: add = 16; break;  /* 8 */
			case 5: add = 64; break; /* 16 */
			case 6: add = 128; break; /* 32 */
			case 7: add = 256; break; /* 64 */
		}
	}

	if ( dir_from_D0 == 0 )
	{
		/* DMA D0,[RAM] */
		source = dsp_reg.ra0 << 2;
		source &= 0x07ffffff;
		dest &= 0x07ffffff;
		transfer_cnt &= 0xff;

#if DEBUG_DSP
		fprintf( log_file, "/*DSP DMA D0,[RAM%d],%d add=%d*/\n", dsp_mem, transfer_cnt, add );
#endif

		for ( counter = 0; counter < transfer_cnt ; counter++ )
		{
#if DEBUG_DSP
			fprintf( log_file, "%08X, ", source ); fflush( log_file );
#endif

			if ( source >= 0x06000000 && source <= 0x060fffff )
			{
				data = space.read_dword(source );
			}
			else
			{
				data = (space.read_word(source)<<16) | space.read_word(source+2);
				//popmessage( "Bad DSP DMA mem read = %08X", source );
#if DEBUG_DSP
				//fprintf( log_file, "/*Bad DSP DMA mem read = %08X*/\n", source );
#endif
			}

#if DEBUG_DSP
			fprintf( log_file, "%08X,\n", data );
#endif
			dsp_set_dest_dma_mem( dsp_mem, data, counter );
			source += add;
		}

		if ( hold == 0 )
		{
			dsp_reg.ra0 += ((counter * add) >> 2);
		}
	}
	else
	{
		/* DMA [RAM],D0 */
		dest = dsp_reg.wa0 << 2;
		source &= 0x07ffffff;
		dest &= 0x07ffffff;
		transfer_cnt &= 0xff;
		//logerror("[DSP DMA] SRC = %08x | DEST = %08x | SIZE = %08x | ADD VALUE = %08x\n",source,dest,transfer_cnt,add);

#if DEBUG_DSP
		fprintf( log_file, "/*DSP DMA [RAM%d],D0,%d\tadd=%d,source=%08X*/\n", dsp_mem, transfer_cnt, add, source );
#endif
		for ( counter = 0; counter < transfer_cnt; counter++ )
		{
			space.write_dword(dest, dsp_get_mem_source_dma( dsp_mem, counter ) );
			dest += add;
		}

		if ( hold == 0 )
		{
			dsp_reg.wa0 += ((counter * add) >> 2);
		}
	}

	/* TODO: move this behind a timer */
	T0F_0;
}

static void dsp_jump( address_space &space )
{
	if ( opcode & 0x3f80000 )
	{
		if ( dsp_compute_condition( space, (opcode & 0x3f80000) >> 19 ) )
		{
			dsp_reg.delay = dsp_reg.pc;
			dsp_reg.pc = opcode & 0xff;
		}
	}
	else
	{
		dsp_reg.delay = dsp_reg.pc;
		dsp_reg.pc = opcode & 0xff;
	}
}

static TIMER_CALLBACK( dsp_ended )
{
	saturn_state *state = machine.driver_data<saturn_state>();

	if(!(state->m_scu.ism & IRQ_DSP_END))
		state->m_maincpu->set_input_line_and_vector(0xa, HOLD_LINE, 0x45);
	else
		state->m_scu.ist |= (IRQ_DSP_END);

	EF_1;
}

static void dsp_end( address_space &dmaspace )
{
	saturn_state *state = dmaspace.machine().driver_data<saturn_state>();

	if(opcode & 0x08000000)
	{
		/*ENDI*/
		dmaspace.machine().scheduler().timer_set(attotime::from_usec(300), FUNC(dsp_ended));
	}

	EXF_0; /* END / ENDI */
}

static void dsp_loop( void )
{
	if ( opcode & 0x8000000 )
	{
		/* LPS */
		if ( dsp_reg.lop != 0 )
		{
			dsp_reg.lop--;
			dsp_reg.delay = dsp_reg.pc;
			dsp_reg.pc--;
		}
	}
	else
	{
		/* BTM */
		if ( dsp_reg.lop != 0 )
		{
			dsp_reg.lop--;
			dsp_reg.delay = dsp_reg.pc;
			dsp_reg.pc = dsp_reg.top;
		}
	}
}

#if DEBUG_DSP
static void dsp_dump_mem( FILE *f )
{
	UINT16 i;

	fprintf( f, "\n/*MEM 0*/\n{" );
	for ( i = 0; i < 0x40; i++ )
	{
		fprintf( f, "%08X, ", dsp_reg.md0[ i ] );
	}
	fprintf( f, "}\n/*MEM 1*/\n{" );
	for ( i = 0; i < 0x40; i++ )
	{
		fprintf( f, "%08X,", dsp_reg.md1[ i ] );
	}
	fprintf( f, "}\n/*MEM 2*/\n{" );
	for ( i = 0; i < 0x40; i++ )
	{
		fprintf( f, "%08X,", dsp_reg.md2[ i ] );
	}
	fprintf( f, "}\n/*MEM 3*/\n{" );
	for ( i = 0; i < 0x40; i++ )
	{
		fprintf( f, "%08X,", dsp_reg.md3[ i ] );
	}
	fprintf( f, "}\n" );
}
#endif

void dsp_execute_program(address_space &dmaspace)
{
	UINT32 cycles_run = 0;
	UINT8 cont = 1;
#if DEBUG_DSP
	UINT16 i;

	if ( log_file == NULL )
	{
		log_file = fopen("dsp.log", "a");
	}
	for ( i = 0; i < 0x100; i++ )
	{
		dsp_dasm_opcode( dsp_reg.internal_prg[ i ], dasm_buffer );
		fprintf( log_file, "%02X\t%08X\t%s\n", i, dsp_reg.internal_prg[ i ], dasm_buffer );
	}
	dsp_dump_mem( log_file );
#endif

	update_mul = 0;
	do
	{
		if ( dsp_reg.delay )
		{
			opcode = dsp_reg.internal_prg[ dsp_reg.delay ];
			dsp_reg.delay = 0;
		}
		else
		{
			opcode = dsp_reg.internal_prg[ dsp_reg.pc ];
			dsp_reg.pc++;
		}

		dsp_dasm_opcode( opcode, dasm_buffer );

		switch( (opcode & 0xc0000000) >> 30 )
		{
		case 0x00: /* 00 */
			dsp_operation( dmaspace );
			break;
		case 0x01: /* 01 */
			/* unrecognized opcode */
			break;
		case 0x02: /* 10 */
			dsp_move_immediate( dmaspace );
			break;
		case 0x03: /* 11 */
			switch( (opcode & 0x30000000) >> 28 )
			{
				case 0x00:
					dsp_dma(dmaspace);
					break;
				case 0x01:
					dsp_jump(dmaspace);
					break;
				case 0x02:
					dsp_loop();
					break;
				case 0x03:
					dsp_end(dmaspace);
					cont = 0;
					break;
			}
			break;
		}
		if ( update_mul == 1 )
		{
			dsp_reg.mul = (INT64)dsp_reg.rx.si * (INT64)dsp_reg.ry.si;
			update_mul = 0;
		}

		cycles_run++;

	} while( cont );
#if DEBUG_DSP
	dsp_dump_mem( log_file );
	fprintf( log_file, "\nRun %d cycles\n\n", cycles_run );
	fclose( log_file );
	log_file = NULL;
#endif
}



/***********************************************************************

    SCU DSP Disassembler

************************************************************************/
static const char *const ALU_Commands[] =
{
	"",     /* 0000 */
	"AND",  /* 0001 */
	"OR",   /* 0010 */
	"XOR",  /* 0011 */
	"ADD",  /* 0100 */
	"SUB",  /* 0101 */
	"AD2",  /* 0110 */
	"???",  /* 0111 */
	"SR",   /* 1000 */
	"RR",   /* 1001 */
	"SL",   /* 1010 */
	"RL",   /* 1011 */
	"???",  /* 1100 */
	"???",  /* 1101 */
	"???",  /* 1110 */
	"RL8",  /* 1111 */
};

static const char *const X_Commands[] =
{
	"",             /* 000 */
	"",             /* 001 */   /* NOP? check instruction @ 0x0B */
	"MOV MUL,P",    /* 010 */
	"MOV %s,P",     /* 011 */
	"MOV %s,X",     /* 100 */
};

static const char *const Y_Commands[] =
{
	"",             /* 000 */
	"CLR A",        /* 001 */
	"MOV ALU,A",    /* 010 */
	"MOV %s,A",     /* 011 */
	"MOV %s,Y",     /* 100 */
};

static const char *const D1_Commands[] =
{
	"",                 /* 00 */
	"MOV %I8,%d",       /* 01 */
	"???",              /* 10 */
	"MOV %S,%d",        /* 11 */
};

static const char *const SourceMemory[] =
{
	"M0",           /* 000 */
	"M1",           /* 001 */
	"M2",           /* 010 */
	"M3",           /* 011 */
	"MC0",          /* 100 */
	"MC1",          /* 101 */
	"MC2",          /* 110 */
	"MC3",          /* 111 */
};

static const char *const SourceMemory2[] =
{
	"M0",           /* 0000 */
	"M1",           /* 0001 */
	"M2",           /* 0010 */
	"M3",           /* 0011 */
	"MC0",          /* 0100 */
	"MC1",          /* 0101 */
	"MC2",          /* 0110 */
	"MC3",          /* 0111 */
	"???",          /* 1000 */
	"ALL",          /* 1001 */
	"ALH",          /* 1010 */
	"???",          /* 1011 */
	"???",          /* 1100 */
	"???",          /* 1101 */
	"???",          /* 1110 */
	"???",          /* 1111 */
};

static const char *const DestMemory[] =
{
	"MC0",          /* 0000 */
	"MC1",          /* 0001 */
	"MC2",          /* 0010 */
	"MC3",          /* 0011 */
	"RX",           /* 0100 */
	"PL",           /* 0101 */
	"RA0",          /* 0110 */
	"WA0",          /* 0111 */
	"???",          /* 1000 */
	"???",          /* 1001 */
	"LOP",          /* 1010 */
	"TOP",          /* 1011 */
	"CT0",          /* 1100 */
	"CT1",          /* 1101 */
	"CT2",          /* 1110 */
	"CT3",          /* 1111 */
};

static const char *const DestDMAMemory[] =
{
	"M0",           /* 000 */
	"M1",           /* 001 */
	"M2",           /* 010 */
	"M3",           /* 011 */
	"PRG",          /* 100 */
	"???",          /* 101 */
	"???",          /* 110 */
	"???",          /* 111 */
};

static const char *const MVI_Command[] =
{
	"MVI %I,%d",    /* 0 */
	"MVI %I,%d,%f", /* 1 */
};

static const char *const JMP_Command[] =
{
	"JMP %IA",
	"JMP %f,%IA",
};

static const char *const DMA_Command[] =
{
	"DMA%H%A D0,%M,%I",
	"DMA%H%A %s,D0,%I",
	"DMA%H%A D0,%M,%s",
	"DMA%H%A %s,D0,%s",
};


static void dsp_dasm_prefix( const char* format, char* buffer, UINT32 *data )
{

	for ( ; *format; format++ )
	{
		if ( *format == '%' )
		{
			switch( *++format )
			{
				case 'H':
					if ( *data )
					{
						strcpy( buffer, "H" );
					}
					else
					{
						*buffer = 0;
					}
					break;
				case 'A':
					if ( *data == 0 )
					{
						strcpy( buffer, "0" );
					}
					else if ( *data == 1 )
					{
						*buffer = 0;
					}
					else
					{
						sprintf( buffer, "%d", 1 << (*data - 1) );
					}
					break;
				case 's':
					strcpy( buffer, SourceMemory[ *data & 0x7 ] );
					break;
				case 'd':
					strcpy( buffer, DestMemory[ *data & 0xf ] );
					break;
				case 'S':
					strcpy( buffer, SourceMemory2[ *data & 0xf ] );
					break;
				case 'I':
					++format;
					if ( *format == '8' )
					{
						sprintf( buffer, "#$%x", *data );
					}
					else if ( *format == 'A' )
					{
						sprintf( buffer, "$%X", *data );
					}
					else
					{
						--format;
						sprintf( buffer, "#$%X", *data );
					}
					break;
				case 'f':
					if ( !(*data & 0x20) )
					{
						strcpy( buffer, "N" );
						buffer++;
					}
					switch( *data & 0xf )
					{
						case 0x3:
							strcpy( buffer, "ZS" );
							break;
						case 0x2:
							strcpy( buffer, "S" );
							break;
						case 0x4:
							strcpy( buffer, "C" );
							break;
						case 0x8:
							strcpy( buffer, "T0" );
							break;
						case 0x1:
							strcpy( buffer, "Z" );
							break;
						default:
							strcpy( buffer, "?" );
							break;
					}
					break;
				case 'M':
					strcpy( buffer, DestDMAMemory[ *data ] );
					break;

			}
			data++;
			buffer += strlen( buffer );
		}
		else
		{
			*buffer++ = *format;
		}
	}
	*buffer = 0;
}
static void dsp_dasm_operation( UINT32 op, char *buffer )
{
	char *my_buffer = buffer;
	char temp_buffer[64];
	UINT32 data[2 ];

	/* NOP */
	if ( (op & 0x3F8E3000) == 0 )
	{
		sprintf( buffer, "%-10s", "NOP" );
		return;
	}
	/* ALU */
	sprintf( my_buffer, "%-10s", ALU_Commands[ (op & 0x3c000000) >> 26] );
	my_buffer += strlen( my_buffer );

	/* X-Bus */
	data[0] = (op & 0x700000) >> 20;
	if ( op & 0x2000000 )
	{
		dsp_dasm_prefix( X_Commands[ 4 ], temp_buffer, data );
	}
	else
	{
		*temp_buffer = 0;
	}
	sprintf( my_buffer, "%-10s", temp_buffer );
	my_buffer += strlen( my_buffer );

	dsp_dasm_prefix( X_Commands[ (op & 0x1800000) >> 23 ], temp_buffer,  data );
	sprintf( my_buffer, "%-10s", temp_buffer );
	my_buffer += strlen( my_buffer );

	/* Y-Bus */
	data[0] = (op & 0x1C000 ) >> 14 ;
	if ( op & 0x80000 )
	{
		dsp_dasm_prefix( Y_Commands[4], temp_buffer, data );
	}
	else
	{
		*temp_buffer = 0;
	}
	sprintf( my_buffer, "%-10s", temp_buffer );
	my_buffer += strlen( my_buffer );

	dsp_dasm_prefix( Y_Commands[ (op & 0x60000) >> 17 ], temp_buffer,  data );
	sprintf( my_buffer, "%-10s", temp_buffer );
	my_buffer += strlen( my_buffer );

	/* D1-Bus */
	switch( (op & 0x3000) >> 12 )
	{
	case 0x1:
		data[0] = (op & 0xFF);
		data[1] = ((op & 0xF00) >> 8);
		break;
	case 0x3:
		data[0] = (op & 0xF);
		data[1] = ((op & 0xF00) >> 8);
		break;
	};
	dsp_dasm_prefix( D1_Commands[ (op & 0x3000) >> 12 ], temp_buffer, data );
	sprintf( my_buffer, "%-10s", temp_buffer );
}

static void dsp_dasm_move_immediate( UINT32 op, char *buffer )
{
	UINT32 data[3];
	if ( (op & 0x2000000) )
	{
		data[0] = op & 0x7FFFF;
		data[1] = (op & 0x3C000000) >> 26;
		data[2] = (op & 0x3F80000 ) >> 19;
		dsp_dasm_prefix( MVI_Command[1], buffer, data ); /* TODO: bad mem*/
	}
	else
	{
		data[0] = op & 0x1FFFFFF;
		data[1] = (op & 0x3C000000) >> 26;
		dsp_dasm_prefix( MVI_Command[0], buffer, data ); /* TODO: bad mem*/
	}
}

static void dsp_dasm_jump( UINT32 op, char *buffer )
{
	UINT32 data[2];
	if ( op & 0x3F80000 )
	{
		data[0] = (op & 0x3F80000) >> 19;
		data[1] = op & 0xff;
		dsp_dasm_prefix( JMP_Command[1], buffer, data );
	}
	else
	{
		data[0] = op & 0xff;
		dsp_dasm_prefix( JMP_Command[0], buffer, data );
	}
}

static void dsp_dasm_loop( UINT32 op, char* buffer )
{
	if ( op & 0x8000000 )
	{
		strcpy( buffer, "LPS" );
	}
	else
	{
		strcpy( buffer, "BTM" );
	}
}

static void dsp_dasm_end( UINT32 op, char* buffer )
{
	if ( op & 0x8000000 )
	{
		strcpy( buffer, "ENDI" );
	}
	else
	{
		strcpy( buffer, "END" );
	}
}

static void dsp_dasm_dma( UINT32 op, char* buffer )
{
	UINT32 data[4];
	data[0] = (op &  0x4000) >> 14; /* H */
	data[1] = (op & 0x38000) >> 15; /* A */
	data[2] = (op & 0x700) >> 8; /* Mem */
	data[3] = (op & 0xff);
	dsp_dasm_prefix( DMA_Command[(op & 0x3000) >> 12], buffer, data );
}

static void dsp_dasm_opcode( UINT32 op, char *buffer )
{
	switch( (op & 0xc0000000) >> 30 )
	{
	case 0x00: /* 00 */
		dsp_dasm_operation( op, buffer );
		break;
	case 0x01: /* 01 */
		strcpy( buffer, "???"  );
		break;
	case 0x02: /* 10 */
		dsp_dasm_move_immediate( op, buffer );
		break;
	case 0x03: /* 11 */
		switch( (op & 0x30000000) >> 28 )
		{
			case 0x00:
				dsp_dasm_dma( op, buffer );
				break;
			case 0x01:
				dsp_dasm_jump( op, buffer );
				break;
			case 0x02:
				dsp_dasm_loop( op, buffer );
				break;
			case 0x03:
				dsp_dasm_end( op, buffer );
				break;
		}
		break;
	}
}
