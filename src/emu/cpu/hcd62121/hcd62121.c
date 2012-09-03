/**********************************************************************

    Hitachi hcd62121 cpu core emulation.

The Hitachi hcd62121 is the custom cpu which was used in the Casio
CFX-9850 (and maybe some other things too).

This CPU core is based on the information provided by Martin Poupe.
Martin Poupe's site can be found at http://prg.rkk.cz/~mpoupe/

**********************************************************************/

#include "emu.h"
#include "debugger.h"
#include "hcd62121.h"


struct _hcd62121_state
{
	UINT32 prev_pc;
	UINT16 sp;
	UINT16 ip;
	UINT8 dsize;
	UINT8 cseg;
	UINT8 dseg;
	UINT8 sseg;
	UINT8 f;
	UINT16 lar;
	UINT8 reg[0x80];
	UINT8 temp1[0x10];
	UINT8 temp2[0x10];
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	address_space *io;
	int icount;
};

typedef struct _hcd62121_state hcd62121_state;


/* From the battery check routine at 20:e874 it looks like
   bit 3 of the flag register should be the Zero flag. */
#define _FLAG_Z		0x08
#define _FLAG_C		0x02
#define _FLAG_ZL	0x04
#define _FLAG_CL	0x01
#define _FLAG_ZH	0x10


#define mem_readbyte(cs,A)		((UINT8)(cs)->program->read_byte(A))
#define mem_writebyte(cs,A,V)	((cs)->program->write_byte(A,V))
#define io_readbyte(cs,A)		((UINT8)(cs)->io->read_byte(A))
#define io_writebyte(cs,A,V)	((cs)->io->write_byte(A,V))


INLINE UINT8 read_op(hcd62121_state *cpustate)
{
	UINT8 d = mem_readbyte(cpustate, ( cpustate->cseg << 16 ) | cpustate->ip );
	cpustate->ip++;
	return d;
}


INLINE UINT8 datasize( hcd62121_state *cpustate, UINT8 op )
{
	switch( op & 0x03 )
	{
	case 0:
		return 1;
	case 1:
		return 2;
	case 2:
		return ( cpustate->dsize >> 4 ) + 1;
	case 3:
		return ( cpustate->dsize & 0x0f ) + 1;
	}
	return 1;
}


INLINE void read_reg( hcd62121_state *cpustate, int size, UINT8 op1 )
{
	int i;

	if ( op1 & 0x80 )
	{
		for ( i = 0; i < size; i++ )
			cpustate->temp1[i] = cpustate->reg[ ( op1 - i ) & 0x7f ];
	}
	else
	{
		for ( i = 0; i < size; i++ )
			cpustate->temp1[i] = cpustate->reg[ ( op1 + i ) & 0x7f ];
	}
}


INLINE void write_reg( hcd62121_state *cpustate, int size, UINT8 op1 )
{
	int i;

	if ( op1 & 0x80 )
	{
		for ( i = 0; i < size; i++ )
			cpustate->reg[ ( op1 - i ) & 0x7f ] = cpustate->temp1[i];
	}
	else
	{
		for ( i = 0; i < size; i++ )
			cpustate->reg[ ( op1 + i ) & 0x7f ] = cpustate->temp1[i];
	}
}


INLINE void read_regreg( hcd62121_state *cpustate, int size, UINT8 op1, UINT8 op2, bool op_is_logical )
{
	int i;

	for ( i = 0; i < size; i++ )
		cpustate->temp1[i] = cpustate->reg[ (op1 + i) & 0x7f];

	if ( op1 & 0x80 )
	{
		/* Second operand is an immediate value */
		cpustate->temp2[0] = op2;
		for ( i = 1; i < size; i++ )
			cpustate->temp2[i] = op_is_logical ? op2 : 0;
	}
	else
	{
		/* Second operand is a register */
		for ( i = 0; i < size; i++ )
			cpustate->temp2[i] = cpustate->reg[ (op2 + i) & 0x7f ];
	}

	if ( ! ( op1 & 0x80 ) && ! ( op2 & 0x80 ) )
	{
		/* We need to swap parameters */
		for ( i = 0; i < size; i++ )
		{
			UINT8 v = cpustate->temp1[i];
			cpustate->temp1[i] = cpustate->temp2[i];
			cpustate->temp2[i] = v;
		}
	}
}


INLINE void write_regreg( hcd62121_state *cpustate, int size, UINT8 op1, UINT8 op2 )
{
	int i;

	if ( ( op1 & 0x80 ) || ( op2 & 0x80 ) )
	{
		/* store in reg1 */
		for ( i = 0; i < size; i++ )
			cpustate->reg[ (op1 + i) & 0x7f] = cpustate->temp1[i];
	}
	else
	{
		/* store in reg2 */
		for ( i = 0; i < size; i++ )
			cpustate->reg[ (op2 + i) & 0x7f] = cpustate->temp1[i];
	}
}


INLINE void read_iregreg( hcd62121_state *cpustate, int size, UINT8 op1, UINT8 op2 )
{
	int i;
	UINT16 ad;

	ad = cpustate->reg[ ( 0x40 | op1 ) & 0x7f ] | ( cpustate->reg[ ( 0x40 | ( op1 + 1 ) ) & 0x7f ] << 8 );

	for ( i = 0; i < size; i++ )
	{
		cpustate->temp1[i] = mem_readbyte( cpustate, ( cpustate->dseg << 16 ) | ad );
		ad += ( op1 & 0x40 ) ? -1 : 1;
	}
	cpustate->lar = ad;

	if ( op1 & 0x80 )
	{
		cpustate->temp2[0] = op2;
		for ( i = 1; i < size; i++ )
			cpustate->temp2[i] = 0;
	}
	else
	{
		for ( i = 0; i < size; i++ )
			cpustate->temp2[i] = cpustate->reg[ (op2 + i) & 0x7f ];
	}

	if ( ! ( op1 & 0x80 ) && ! ( op2 & 0x80 ) )
	{
		/* We need to swap parameters */
		for ( i = 0; i < size; i++ )
		{
			UINT8 v = cpustate->temp1[i];
			cpustate->temp1[i] = cpustate->temp2[i];
			cpustate->temp2[i] = v;
		}
	}
}


INLINE void write_iregreg( hcd62121_state *cpustate, int size, UINT8 op1, UINT8 op2 )
{
	int i;

	if ( ( op1 & 0x80 ) || ( op2 & 0x80 ) )
	{
		/* store in (reg1) */
		UINT16 ad = cpustate->reg[ ( 0x40 | op1 ) & 0x7f ] | ( cpustate->reg[ ( 0x40 | ( op1 + 1 ) ) & 0x7f ] << 8 );

		for ( i = 0; i < size; i++ )
		{
			mem_writebyte( cpustate, ( cpustate->dseg << 16 ) | ad, cpustate->temp1[i] );
			ad += ( op1 & 0x40 ) ? -1 : 1;
		}
		cpustate->lar = ad;
	}
	else
	{
		/* store in reg2 */
		for ( i = 0; i < size; i++ )
			cpustate->reg[ (op2 + i) & 0x7f] = cpustate->temp1[i];
	}
}


INLINE void write_iregreg2( hcd62121_state *cpustate, int size, UINT8 op1, UINT8 op2 )
{
	int i;

	if ( ( op1 & 0x80 ) || ( op2 & 0x80 ) )
	{
		/* store in reg2 */
		for ( i = 0; i < size; i++ )
			cpustate->reg[ (op2 + i) & 0x7f] = cpustate->temp2[i];
	}
	else
	{
		/* store in (reg1) */
		UINT16 ad = cpustate->reg[ ( 0x40 | op1 ) & 0x7f ] | ( cpustate->reg[ ( 0x40 | ( op1 + 1 ) ) & 0x7f ] << 8 );

		for ( i = 0; i < size; i++ )
		{
			mem_writebyte( cpustate, ( cpustate->dseg << 16 ) | ad, cpustate->temp2[i] );
			ad += ( op1 & 0x40 ) ? -1 : 1;
		}
		cpustate->lar = ad;
	}
}


INLINE int check_cond( hcd62121_state *cpustate, UINT8 op )
{
	switch ( op & 0x07 )
	{
	case 0x00:	/* ZH set */
		if ( cpustate->f & _FLAG_ZH )
			return 1;
		break;

	case 0x01:	/* ZL set */
		if ( cpustate->f & _FLAG_ZL )
			return 1;
		break;

	case 0x02:	/* C set */
		if ( cpustate->f & _FLAG_C )
			return 1;
		break;

	case 0x03:	/* Z set */
		if ( cpustate->f & _FLAG_Z )
			return 1;
		break;

	case 0x04:	/* Z or C set */
		if ( cpustate->f & ( _FLAG_Z | _FLAG_C ) )
			return 1;
		break;

	case 0x05:	/* CL set */
		if ( cpustate->f & _FLAG_CL )
			return 1;
		break;

	case 0x06:	/* C clear */
		if ( ! ( cpustate->f & _FLAG_C ) )
			return 1;
		break;

	case 0x07:	/* Z clear */
		if ( ! ( cpustate->f & _FLAG_Z ) )
			return 1;
		break;
	}

	return 0;
}


INLINE hcd62121_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == HCD62121);
	return (hcd62121_state *)downcast<legacy_cpu_device *>(device)->token();
}


static CPU_INIT( hcd62121 )
{
	hcd62121_state *cpustate = get_safe_token(device);

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);
}


static CPU_RESET( hcd62121 )
{
	hcd62121_state *cpustate = get_safe_token(device);

	cpustate->sp = 0x0000;
	cpustate->ip = 0x0000;
	cpustate->cseg = 0;
	cpustate->dseg = 0;
	cpustate->sseg = 0;
	cpustate->lar = 0;
	cpustate->f = 0;
	cpustate->dsize = 0;

	for( int i = 0; i < 0x80; i++ )
	{
		cpustate->reg[i] = 0;
	}
}


static CPU_EXECUTE( hcd62121 )
{
	hcd62121_state *cpustate = get_safe_token(device);

	do
	{
		UINT32 pc = ( cpustate->cseg << 16 ) | cpustate->ip;
		UINT8 op;

		debugger_instruction_hook(device, pc);
		cpustate->prev_pc = pc;

		op = read_op( cpustate );

		cpustate->icount -= 4;

		switch ( op )
		{
#include "hcd62121_ops.h"
		};

	} while (cpustate->icount > 0);
}


static CPU_SET_INFO( hcd62121 )
{
	hcd62121_state *cpustate = get_safe_token(device);

	switch (state)
	{
	/* --- the following bits of info are set as 64-bit signed integers --- */
	case CPUINFO_INT_INPUT_STATE + 0:
	case CPUINFO_INT_INPUT_STATE + 1:
												break;

	case CPUINFO_INT_SP:							cpustate->sp = info->i;							break;
	case CPUINFO_INT_PC:							cpustate->ip = info->i;							break;

	case CPUINFO_INT_REGISTER + HCD62121_IP:		cpustate->ip = info->i;							break;
	case CPUINFO_INT_REGISTER + HCD62121_SP:		cpustate->sp = info->i;							break;
	case CPUINFO_INT_REGISTER + HCD62121_LAR:		cpustate->lar = info->i;						break;
	case CPUINFO_INT_REGISTER + HCD62121_CS:		cpustate->cseg = info->i;						break;
	case CPUINFO_INT_REGISTER + HCD62121_DS:		cpustate->dseg = info->i;						break;
	case CPUINFO_INT_REGISTER + HCD62121_SS:		cpustate->sseg = info->i;						break;
	case CPUINFO_INT_REGISTER + HCD62121_DSIZE:		cpustate->dsize = info->i;						break;
//  case CPUINFO_INT_REGISTER + HCD62121_R00:   break;
//  case CPUINFO_INT_REGISTER + HCD62121_R02:   break;
	}
}


CPU_GET_INFO( hcd62121 )
{
	hcd62121_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(hcd62121_state);					break;
	case CPUINFO_INT_INPUT_LINES:						info->i = 2;							break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
	case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
	case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 18;							break;
	case CPUINFO_INT_MIN_CYCLES:					info->i = 4;	/* right? */			break;
	case CPUINFO_INT_MAX_CYCLES:					info->i = 48;	/* right? */			break;

	case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 24;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 8;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

	case CPUINFO_INT_SP:							info->i = cpustate->sp;					break;
	case CPUINFO_INT_PC:							info->i = ( cpustate->cseg << 16 ) | cpustate->ip; break;
	case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->prev_pc;			break;

	case CPUINFO_INT_INPUT_STATE + 0:
	case CPUINFO_INT_INPUT_STATE + 1:
													/* TODO */									break;

	case CPUINFO_INT_REGISTER + HCD62121_IP:			info->i = cpustate->ip;					break;
	case CPUINFO_INT_REGISTER + HCD62121_SP:			info->i = cpustate->sp;					break;
	case CPUINFO_INT_REGISTER + HCD62121_LAR:			info->i = cpustate->lar;				break;
	case CPUINFO_INT_REGISTER + HCD62121_CS:			info->i = cpustate->cseg;				break;
	case CPUINFO_INT_REGISTER + HCD62121_DS:			info->i = cpustate->dseg;				break;
	case CPUINFO_INT_REGISTER + HCD62121_SS:			info->i = cpustate->sseg;				break;
	case CPUINFO_INT_REGISTER + HCD62121_DSIZE:			info->i = cpustate->dsize;				break;
	case CPUINFO_INT_REGISTER + HCD62121_R00:			info->i = ( cpustate->reg[0x00] << 24 ) | ( cpustate->reg[0x01] << 16 ) | ( cpustate->reg[0x02] << 8 ) | cpustate->reg[0x03]; break;
//  case CPUINFO_INT_REGISTER + HCD62121_R02:           info->i = cpustate->;                   break;

	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(hcd62121);		break;
	case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(hcd62121);				break;
	case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(hcd62121);			break;
	case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(hcd62121);		break;
	case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(hcd62121);		break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME:							strcpy(info->s, "HCD62121"); break;
	case CPUINFO_STR_FAMILY:						strcpy(info->s, "Hitachi HCD62121"); break;
	case CPUINFO_STR_VERSION:						strcpy(info->s, "0.1"); break;
	case CPUINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__); break;
	case CPUINFO_STR_CREDITS:						strcpy(info->s, "Copyright The MESS Team."); break;

	case CPUINFO_STR_FLAGS:
		sprintf(info->s, "%s-%s-%s-%c-%c",
			cpustate->f & _FLAG_ZH ? "ZH":"__",
			cpustate->f & _FLAG_CL ? "CL":"__",
			cpustate->f & _FLAG_ZL ? "ZL":"__",
			cpustate->f & _FLAG_C ? 'C':'_',
			cpustate->f & _FLAG_Z ? 'Z':'_'
			);
		break;

	case CPUINFO_STR_REGISTER + HCD62121_IP: sprintf(info->s, "IP:%04X", cpustate->ip); break;
	case CPUINFO_STR_REGISTER + HCD62121_SP: sprintf(info->s, "SP:%04X", cpustate->sp); break;
	case CPUINFO_STR_REGISTER + HCD62121_LAR: sprintf(info->s, "LAR:%04X", cpustate->lar); break;
	case CPUINFO_STR_REGISTER + HCD62121_CS: sprintf(info->s, "CS:%02X", cpustate->cseg); break;
	case CPUINFO_STR_REGISTER + HCD62121_DS: sprintf(info->s, "DS:%02X", cpustate->dseg); break;
	case CPUINFO_STR_REGISTER + HCD62121_SS: sprintf(info->s, "SS:%02X", cpustate->sseg); break;
	case CPUINFO_STR_REGISTER + HCD62121_DSIZE: sprintf(info->s, "DSIZE:%02X", cpustate->dsize); break;
	case CPUINFO_STR_REGISTER + HCD62121_R00: sprintf(info->s, "R00:%02X%02X%02X%02X", cpustate->reg[0x00], cpustate->reg[0x01], cpustate->reg[0x02], cpustate->reg[0x03]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R04: sprintf(info->s, "R04:%02X%02X%02X%02X", cpustate->reg[0x04], cpustate->reg[0x05], cpustate->reg[0x06], cpustate->reg[0x07]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R08: sprintf(info->s, "R08:%02X%02X%02X%02X", cpustate->reg[0x08], cpustate->reg[0x09], cpustate->reg[0x0a], cpustate->reg[0x0b]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R0C: sprintf(info->s, "R0C:%02X%02X%02X%02X", cpustate->reg[0x0c], cpustate->reg[0x0d], cpustate->reg[0x0e], cpustate->reg[0x0f]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R10: sprintf(info->s, "R10:%02X%02X%02X%02X", cpustate->reg[0x10], cpustate->reg[0x11], cpustate->reg[0x12], cpustate->reg[0x13]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R14: sprintf(info->s, "R14:%02X%02X%02X%02X", cpustate->reg[0x14], cpustate->reg[0x15], cpustate->reg[0x16], cpustate->reg[0x17]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R18: sprintf(info->s, "R18:%02X%02X%02X%02X", cpustate->reg[0x18], cpustate->reg[0x19], cpustate->reg[0x1a], cpustate->reg[0x1b]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R1C: sprintf(info->s, "R1C:%02X%02X%02X%02X", cpustate->reg[0x1c], cpustate->reg[0x1d], cpustate->reg[0x1e], cpustate->reg[0x1f]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R20: sprintf(info->s, "R20:%02X%02X%02X%02X", cpustate->reg[0x20], cpustate->reg[0x21], cpustate->reg[0x22], cpustate->reg[0x23]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R24: sprintf(info->s, "R24:%02X%02X%02X%02X", cpustate->reg[0x24], cpustate->reg[0x25], cpustate->reg[0x26], cpustate->reg[0x27]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R28: sprintf(info->s, "R28:%02X%02X%02X%02X", cpustate->reg[0x28], cpustate->reg[0x29], cpustate->reg[0x2a], cpustate->reg[0x2b]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R2C: sprintf(info->s, "R2C:%02X%02X%02X%02X", cpustate->reg[0x2c], cpustate->reg[0x2d], cpustate->reg[0x2e], cpustate->reg[0x2f]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R30: sprintf(info->s, "R30:%02X%02X%02X%02X", cpustate->reg[0x30], cpustate->reg[0x31], cpustate->reg[0x32], cpustate->reg[0x33]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R34: sprintf(info->s, "R34:%02X%02X%02X%02X", cpustate->reg[0x34], cpustate->reg[0x35], cpustate->reg[0x36], cpustate->reg[0x37]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R38: sprintf(info->s, "R38:%02X%02X%02X%02X", cpustate->reg[0x38], cpustate->reg[0x39], cpustate->reg[0x3a], cpustate->reg[0x3b]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R3C: sprintf(info->s, "R3C:%02X%02X%02X%02X", cpustate->reg[0x3c], cpustate->reg[0x3d], cpustate->reg[0x3e], cpustate->reg[0x3f]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R40: sprintf(info->s, "R40:%02X%02X%02X%02X", cpustate->reg[0x40], cpustate->reg[0x41], cpustate->reg[0x42], cpustate->reg[0x43]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R44: sprintf(info->s, "R44:%02X%02X%02X%02X", cpustate->reg[0x44], cpustate->reg[0x45], cpustate->reg[0x46], cpustate->reg[0x47]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R48: sprintf(info->s, "R48:%02X%02X%02X%02X", cpustate->reg[0x48], cpustate->reg[0x49], cpustate->reg[0x4a], cpustate->reg[0x4b]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R4C: sprintf(info->s, "R4C:%02X%02X%02X%02X", cpustate->reg[0x4c], cpustate->reg[0x4d], cpustate->reg[0x4e], cpustate->reg[0x4f]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R50: sprintf(info->s, "R50:%02X%02X%02X%02X", cpustate->reg[0x50], cpustate->reg[0x51], cpustate->reg[0x52], cpustate->reg[0x53]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R54: sprintf(info->s, "R54:%02X%02X%02X%02X", cpustate->reg[0x54], cpustate->reg[0x55], cpustate->reg[0x56], cpustate->reg[0x57]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R58: sprintf(info->s, "R58:%02X%02X%02X%02X", cpustate->reg[0x58], cpustate->reg[0x59], cpustate->reg[0x5a], cpustate->reg[0x5b]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R5C: sprintf(info->s, "R5C:%02X%02X%02X%02X", cpustate->reg[0x5c], cpustate->reg[0x5d], cpustate->reg[0x5e], cpustate->reg[0x5f]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R60: sprintf(info->s, "R60:%02X%02X%02X%02X", cpustate->reg[0x60], cpustate->reg[0x61], cpustate->reg[0x62], cpustate->reg[0x63]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R64: sprintf(info->s, "R64:%02X%02X%02X%02X", cpustate->reg[0x64], cpustate->reg[0x65], cpustate->reg[0x66], cpustate->reg[0x67]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R68: sprintf(info->s, "R68:%02X%02X%02X%02X", cpustate->reg[0x68], cpustate->reg[0x69], cpustate->reg[0x6a], cpustate->reg[0x6b]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R6C: sprintf(info->s, "R6C:%02X%02X%02X%02X", cpustate->reg[0x6c], cpustate->reg[0x6d], cpustate->reg[0x6e], cpustate->reg[0x6f]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R70: sprintf(info->s, "R70:%02X%02X%02X%02X", cpustate->reg[0x70], cpustate->reg[0x71], cpustate->reg[0x72], cpustate->reg[0x73]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R74: sprintf(info->s, "R74:%02X%02X%02X%02X", cpustate->reg[0x74], cpustate->reg[0x75], cpustate->reg[0x76], cpustate->reg[0x77]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R78: sprintf(info->s, "R78:%02X%02X%02X%02X", cpustate->reg[0x78], cpustate->reg[0x79], cpustate->reg[0x7a], cpustate->reg[0x7b]); break;
	case CPUINFO_STR_REGISTER + HCD62121_R7C: sprintf(info->s, "R7C:%02X%02X%02X%02X", cpustate->reg[0x7c], cpustate->reg[0x7d], cpustate->reg[0x7e], cpustate->reg[0x7f]); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(HCD62121, hcd62121);

