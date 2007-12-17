/***************************************************************************

    mb88xx.c
    Core implementation for the portable Fujitsu MB88xx series MCU emulator.

    Written by Ernesto Corvi


    TODO:
    - Add support for the timer
    - Add support for the serial interface
    - Split the core to support multiple CPU types?

***************************************************************************/

#include "debugger.h"
#include "mb88xx.h"

/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

typedef struct
{
	UINT8	PC; 	/* Program Counter: 6 bits */
	UINT8	PA; 	/* Page Address: 4 bits */
	UINT16	SP[4];	/* Stack is 4*10 bit addresses deep, but we also use 3 top bits per address to store flags during irq */
	UINT8	SI;		/* Stack index: 2 bits */
	UINT8	A;		/* Accumulator: 4 bits */
	UINT8	X;		/* Index X: 4 bits */
	UINT8	Y;		/* Index Y: 4 bits */
	UINT8	st;		/* State flag: 1 bit */
	UINT8	zf;		/* Zero flag: 1 bit */
	UINT8	cf;		/* Carry flag: 1 bit */
	UINT8	vf;		/* Timer overflow flag: 1 bit */
	UINT8	sf;		/* Serial Full/Empty flag: 1 bit */
	UINT8	nf;		/* Interrupt flag: 1 bit */

    /* Peripheral Control */
    UINT8	pio; /* Peripheral enable bits: 8 bits */

    /* Timer registers */
    UINT8	TH;	/* Timer High: 4 bits */
    UINT8	TL;	/* Timer Low: 4 bits */

    /* Serial registers */
    UINT8	SB;	/* Serial buffer: 4 bits */

    /* PLA configuration */
    UINT8 *	PLA;

    /* IRQ handling */
    int	pending_interrupt;
    int (*irqcallback)(int);
} mb88Regs;

/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static mb88Regs mb88;
static int mb88_icount;

/***************************************************************************
    MACROS
***************************************************************************/

#define READOP(a) 			(cpu_readop(a))

#define RDMEM(a)			(data_read_byte_8(a))
#define WRMEM(a,v)			(data_write_byte_8((a), (v)))

#define READPORT(a)			(io_read_byte_8(a))
#define WRITEPORT(a,v)		(io_write_byte_8((a), (v)))

#define TEST_ST()			(mb88.st & 1)
#define TEST_ZF()			(mb88.zf & 1)
#define TEST_CF()			(mb88.cf & 1)
#define TEST_VF()			(mb88.vf & 1)
#define TEST_SF()			(mb88.sf & 1)
#define TEST_NF()			(mb88.nf & 1)

#define UPDATE_ST_C(v)		mb88.st=(v&0x10) ? 0 : 1
#define UPDATE_ST_Z(v)		mb88.st=(v==0) ? 0 : 1

#define UPDATE_CF(v)		mb88.cf=((v&0x10)==0) ? 0 : 1
#define UPDATE_ZF(v)		mb88.zf=(v!=0) ? 0 : 1

#define CYCLES(x)			do { mb88_icount -= (x); } while (0)

#define GETPC()				(((int)mb88.PA << 6)+mb88.PC)
#define GETEA()				((mb88.X << 4)+mb88.Y)

#define INCPC()				do { mb88.PC++; if ( mb88.PC >= 0x40 ) { mb88.PC = 0; mb88.PA++; } } while (0)


/***************************************************************************
    CONTEXT SWITCHING
***************************************************************************/

static void mb88_get_context(void *dst)
{
	/* copy the context */
	*(mb88Regs *)dst = mb88;
}


static void mb88_set_context(void *src)
{
	/* copy the context */
	if (src)
		mb88 = *(mb88Regs *)src;
	change_pc(GETPC());
}

/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void mb88_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	if ( config )
	{
		const struct MB88Config *_config = (const struct MB88Config*)config;
		mb88.PLA = _config->PLA_config;
	}

	mb88.irqcallback = irqcallback;

	state_save_register_item("mb88", clock, mb88.PC);
	state_save_register_item("mb88", clock, mb88.PA);
	state_save_register_item("mb88", clock, mb88.SP[0]);
	state_save_register_item("mb88", clock, mb88.SP[1]);
	state_save_register_item("mb88", clock, mb88.SP[2]);
	state_save_register_item("mb88", clock, mb88.SP[3]);
	state_save_register_item("mb88", clock, mb88.SI);
	state_save_register_item("mb88", clock, mb88.A);
	state_save_register_item("mb88", clock, mb88.X);
	state_save_register_item("mb88", clock, mb88.Y);
	state_save_register_item("mb88", clock, mb88.st);
	state_save_register_item("mb88", clock, mb88.zf);
	state_save_register_item("mb88", clock, mb88.cf);
	state_save_register_item("mb88", clock, mb88.vf);
	state_save_register_item("mb88", clock, mb88.sf);
	state_save_register_item("mb88", clock, mb88.nf);
	state_save_register_item("mb88", clock, mb88.pio);
	state_save_register_item("mb88", clock, mb88.TH);
	state_save_register_item("mb88", clock, mb88.TL);
	state_save_register_item("mb88", clock, mb88.SB);
	state_save_register_item("mb88", clock, mb88.pending_interrupt);
}

static void mb88_reset(void)
{
	/* zero registers and flags */
	mb88.PC = 0;
	mb88.PA = 0;
	mb88.PA = 0;
	mb88.SP[0] = mb88.SP[1] = mb88.SP[2] = mb88.SP[3] = 0;
	mb88.SI = 0;
	mb88.A = 0;
	mb88.X = 0;
	mb88.Y = 0;
	mb88.st = 1;	/* start off with st=1 */
	mb88.zf = 0;
	mb88.cf = 0;
	mb88.vf = 0;
	mb88.sf = 0;
	mb88.nf = 0;
	mb88.pio = 0;
	mb88.TH = 0;
	mb88.TL = 0;
	mb88.SB = 0;
	mb88.pending_interrupt = 0;
}

/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

static int pla( int inA, int inB )
{
	int index = ((inB&1) << 4) | (inA&0x0f);

	if ( mb88.PLA )
		return mb88.PLA[index];

	return index;
}

static void set_irq_line(int state)
{
	/* on falling edge trigger interrupt */
	if ( (mb88.pio & 0x04) && mb88.nf && state == CLEAR_LINE )
	{
		mb88.pending_interrupt = 1;
	}

	mb88.nf = (state != CLEAR_LINE) ? 1 : 0;
}

static void update_pio( void )
{
	/* update interrupts, serial and timer flags */

	if ( (mb88.pio & 0x04) && mb88.pending_interrupt )
	{
		/* no vectors supported, just do the callback to clear irq_state if needed */
		if (mb88.irqcallback)
			(*mb88.irqcallback)(0);

		mb88.SP[mb88.SI] = GETPC();
		mb88.SP[mb88.SI] |= TEST_CF() << 15;
		mb88.SP[mb88.SI] |= TEST_ZF() << 14;
		mb88.SP[mb88.SI] |= TEST_ST() << 13;
		mb88.SI = ( mb88.SI + 1 ) & 3;
		mb88.PC = 0x02;
		mb88.PA = 0x00;
		mb88.st = 1;
		change_pc(GETPC());

		mb88.pending_interrupt = 0;

		CYCLES(3); /* ? */
	}

	/* TODO: add support for serial and timer */
}

static int mb88_execute(int cycles)
{
	mb88_icount = cycles;

	while (mb88_icount >= 0)
	{
		UINT8 opcode, arg, oc;

		/* fetch the opcode */
		CALL_MAME_DEBUG;
		opcode = READOP(GETPC());

		/* increment the PC */
		INCPC();

		/* start with instruction doing 1 cycle */
		oc = 1;

		switch (opcode)
		{
			case 0x00: /* nop ZCS:...*/
				mb88.st = 1;
				break;

			case 0x01: /* outO ZCS:...*/
				WRITEPORT( MB88_PORTO, pla(mb88.A,TEST_CF()) );
				mb88.st = 1;
				break;

			case 0x02: /* outP ZCS:... */
				WRITEPORT( MB88_PORTP, mb88.A );
				mb88.st = 1;
				break;

			case 0x03: /* outR ZCS:... */
				arg = mb88.Y;
				WRITEPORT( MB88_PORTR0+(arg&3), mb88.A );
				mb88.st = 1;
				break;

			case 0x04: /* tay ZCS:... */
				mb88.Y = mb88.A;
				mb88.st = 1;
				break;

			case 0x05: /* tath ZCS:... */
				mb88.TH = mb88.A;
				mb88.st = 1;
				break;

			case 0x06: /* tatl ZCS:... */
				mb88.TL = mb88.A;
				mb88.st = 1;
				break;

			case 0x07: /* tas ZCS:... */
				mb88.SB = mb88.A;
				mb88.st = 1;
				break;

			case 0x08: /* icy ZCS:x.x */
				mb88.Y++;
				UPDATE_ST_C(mb88.Y);
				mb88.Y &= 0x0f;
				UPDATE_ZF(mb88.Y);
				break;

			case 0x09: /* icm ZCS:x.x */
				arg=RDMEM(GETEA());
				arg++;
				UPDATE_ST_C(arg);
				arg &= 0x0f;
				UPDATE_ZF(arg);
				WRMEM(GETEA(),arg);
				break;

			case 0x0a: /* stic ZCS:x.x */
				WRMEM(GETEA(),mb88.A);
				mb88.Y++;
				UPDATE_ST_C(mb88.Y);
				mb88.Y &= 0x0f;
				UPDATE_ZF(mb88.Y);
				break;

			case 0x0b: /* x ZCS:x.. */
				arg = RDMEM(GETEA());
				WRMEM(GETEA(),mb88.A);
				mb88.A = arg;
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0x0c: /* rol ZCS:xxx */
				mb88.A <<= 1;
				mb88.A |= TEST_CF();
				UPDATE_ST_C(mb88.A);
				mb88.cf = mb88.st ^ 1;
				mb88.A &= 0x0f;
				UPDATE_ZF(mb88.A);
				break;

			case 0x0d: /* l ZCS:x.. */
				mb88.A = RDMEM(GETEA());
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0x0e: /* adc ZCS:xxx */
				arg = RDMEM(GETEA());
				arg += mb88.A;
				arg += TEST_CF();
				UPDATE_ST_C(arg);
				mb88.cf = mb88.st ^ 1;
				mb88.A = arg & 0x0f;
				UPDATE_ZF(mb88.A);
				break;

			case 0x0f: /* and ZCS:x.x */
				mb88.A &= RDMEM(GETEA());
				UPDATE_ZF(mb88.A);
				mb88.st = mb88.zf ^ 1;
				break;

			case 0x10: /* daa ZCS:.xx */
				if ( TEST_CF() || mb88.A > 9 ) mb88.A += 6;
				UPDATE_ST_C(mb88.A);
				mb88.cf = mb88.st ^ 1;
				mb88.A &= 0x0f;
				break;

			case 0x11: /* das ZCS:.xx */
				if ( TEST_CF() || mb88.A > 9 ) mb88.A += 10;
				UPDATE_ST_C(mb88.A);
				mb88.cf = mb88.st ^ 1;
				mb88.A &= 0x0f;
				break;

			case 0x12: /* inK ZCS:x.. */
				mb88.A = READPORT( MB88_PORTK ) & 0x0f;
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0x13: /* inR ZCS:x.. */
				arg = mb88.Y;
				mb88.A = READPORT( MB88_PORTR0+(arg&3) ) & 0x0f;
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0x14: /* tya ZCS:x.. */
				mb88.A = mb88.Y;
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0x15: /* ttha ZCS:x.. */
				mb88.A = mb88.TH;
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0x16: /* ttla ZCS:x.. */
				mb88.A = mb88.TL;
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0x17: /* tsa ZCS:x.. */
				mb88.A = mb88.SB;
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0x18: /* dcy ZCS:..x */
				mb88.Y--;
				UPDATE_ST_C(mb88.Y);
				mb88.Y &= 0x0f;
				break;

			case 0x19: /* dcm ZCS:x.x */
				arg=RDMEM(GETEA());
				arg--;
				UPDATE_ST_C(arg);
				arg &= 0x0f;
				UPDATE_ZF(arg);
				WRMEM(GETEA(),arg);
				break;

			case 0x1a: /* stdc ZCS:x.x */
				WRMEM(GETEA(),mb88.A);
				mb88.Y--;
				UPDATE_ST_C(mb88.Y);
				mb88.Y &= 0x0f;
				UPDATE_ZF(mb88.Y);
				break;

			case 0x1b: /* xx ZCS:x.. */
				arg = mb88.X;
				mb88.X = mb88.A;
				mb88.A = arg;
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0x1c: /* ror ZCS:xxx */
				mb88.A |= TEST_CF() << 4;
				UPDATE_ST_C(mb88.A << 4);
				mb88.cf = mb88.st ^ 1;
				mb88.A >>= 1;
				mb88.A &= 0x0f;
				UPDATE_ZF(mb88.A);
				break;

			case 0x1d: /* st ZCS:x.. */
				WRMEM(GETEA(),mb88.A);
				mb88.st = 1;
				break;

			case 0x1e: /* sbc ZCS:xxx */
				arg = RDMEM(GETEA());
				arg -= mb88.A;
				arg -= TEST_CF();
				UPDATE_ST_C(arg);
				mb88.cf = mb88.st ^ 1;
				mb88.A = arg & 0x0f;
				UPDATE_ZF(mb88.A);
				break;

			case 0x1f: /* or ZCS:x.x */
				mb88.A |= RDMEM(GETEA());
				UPDATE_ZF(mb88.A);
				mb88.st = mb88.zf ^ 1;
				break;

			case 0x20: /* setR ZCS:... */
				arg = READPORT( MB88_PORTR0+(mb88.Y/4) );
				WRITEPORT( MB88_PORTR0+(mb88.Y/4), arg | ( 1 << (mb88.Y%4) ) );
				mb88.st = 1;
				break;

			case 0x21: /* setc ZCS:.xx */
				mb88.cf = 1;
				mb88.st = 1;
				break;

			case 0x22: /* rstR ZCS:... */
				arg = READPORT( MB88_PORTR0+(mb88.Y/4) );
				WRITEPORT( MB88_PORTR0+(mb88.Y/4), arg & ~( 1 << (mb88.Y%4) ) );
				mb88.st = 1;
				break;

			case 0x23: /* rstc ZCS:.xx */
				mb88.cf = 0;
				mb88.st = 1;
				break;

			case 0x24: /* tstr ZCS:..x */
				arg = READPORT( MB88_PORTR0+(mb88.Y/4) );
				mb88.st = ( arg & ( 1 << (mb88.Y%4) ) ) ? 1 : 0;
				break;

			case 0x25: /* tsti ZCS:..x */
				mb88.st = mb88.nf ^ 1;
				break;

			case 0x26: /* tstv ZCS:..x */
				mb88.st = mb88.vf ^ 1;
				break;

			case 0x27: /* tsts ZCS:..x */
				mb88.st = mb88.sf ^ 1;
				break;

			case 0x28: /* tstc ZCS:..x */
				mb88.st = mb88.cf ^ 1;
				break;

			case 0x29: /* tstz ZCS:..x */
				mb88.st = mb88.zf ^ 1;
				break;

			case 0x2a: /* sts ZCS:x.. */
				WRMEM(GETEA(),mb88.SB);
				UPDATE_ZF(mb88.SB);
				mb88.st = 1;
				break;

			case 0x2b: /* ls ZCS:x.. */
				mb88.SB = RDMEM(GETEA());
				UPDATE_ZF(mb88.SB);
				mb88.st = 1;
				break;

			case 0x2c: /* rts ZCS:... */
				mb88.SI = ( mb88.SI - 1 ) & 3;
				mb88.PC = mb88.SP[mb88.SI] & 0x3f;
				mb88.PA = mb88.SP[mb88.SI] >> 6;
				change_pc(GETPC());
				mb88.st = 1;
				break;

			case 0x2d: /* neg ZCS: ..x */
				mb88.A = (~mb88.A)+1;
				mb88.A &= 0x0f;
				UPDATE_ST_Z(mb88.A);
				break;

			case 0x2e: /* c ZCS:xxx */
				arg = RDMEM(GETEA());
				arg -= mb88.A;
				UPDATE_CF(arg);
				arg &= 0x0f;
				UPDATE_ST_Z(arg);
				mb88.zf = mb88.st ^ 1;
				break;

			case 0x2f: /* eor ZCS:x.x */
				mb88.A ^= RDMEM(GETEA());
				UPDATE_ST_Z(mb88.A);
				mb88.zf = mb88.st ^ 1;
				break;

			case 0x30: case 0x31: case 0x32: case 0x33: /* sbit ZCS:... */
				arg = RDMEM(GETEA());
				WRMEM(GETEA(), arg | (1 << (opcode&3)));
				mb88.st = 1;
				break;

			case 0x34: case 0x35: case 0x36: case 0x37: /* rbit ZCS:... */
				arg = RDMEM(GETEA());
				WRMEM(GETEA(), arg & ~(1 << (opcode&3)));
				mb88.st = 1;
				break;

			case 0x38: case 0x39: case 0x3a: case 0x3b: /* tbit ZCS:... */
				arg = RDMEM(GETEA());
				mb88.st = ( arg & (1 << (opcode&3) ) ) ? 1 : 0;
				break;

			case 0x3c: /* rti ZCS:... */
				/* restore address and saved state flags on the top bits of the stack */
				mb88.SI = ( mb88.SI - 1 ) & 3;
				mb88.PC = mb88.SP[mb88.SI] & 0x3f;
				mb88.PA = mb88.SP[mb88.SI] >> 6;
				mb88.st = (mb88.SP[mb88.SI] >> 13)&1;
				mb88.zf = (mb88.SP[mb88.SI] >> 14)&1;
				mb88.cf = (mb88.SP[mb88.SI] >> 15)&1;
				change_pc(GETPC());
				break;

			case 0x3d: /* jpa imm ZCS:..x */
				mb88.PA = READOP(GETPC()) & 0x1f;
				mb88.PC = mb88.A * 4;
				oc = 2;
				mb88.st = 1;
				break;

			case 0x3e: /* en imm ZCS:... */
				mb88.pio |= READOP(GETPC());
				INCPC();
				oc = 2;
				mb88.st = 1;
				break;

			case 0x3f: /* dis imm ZCS:... */
				mb88.pio &= ~(READOP(GETPC()));
				INCPC();
				oc = 2;
				mb88.st = 1;
				break;

			case 0x40:	case 0x41:	case 0x42:	case 0x43: /* setD ZCS:... */
				arg = READPORT(MB88_PORTR0);
				arg |= (1 << (opcode&3));
				WRITEPORT(MB88_PORTR0,arg);
				mb88.st = 1;
				break;

			case 0x44:	case 0x45:	case 0x46:	case 0x47: /* rstD ZCS:... */
				arg = READPORT(MB88_PORTR0);
				arg &= ~(1 << (opcode&3));
				WRITEPORT(MB88_PORTR0,arg);
				mb88.st = 1;
				break;

			case 0x48:	case 0x49:	case 0x4a:	case 0x4b: /* tstD ZCS:..x */
				arg = READPORT(MB88_PORTR2);
				mb88.st = (arg & (1 << (opcode&3))) ? 1 : 0;
				break;

			case 0x4c:	case 0x4d:	case 0x4e:	case 0x4f: /* tba ZCS:..x */
				mb88.st = (mb88.A & (1 << (opcode&3))) ? 1 : 0;
				break;

			case 0x50:	case 0x51:	case 0x52:	case 0x53: /* xd ZCS:x.. */
				arg = RDMEM(opcode&3);
				WRMEM((opcode&3),mb88.A);
				mb88.A = arg;
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0x54:	case 0x55:	case 0x56:	case 0x57: /* xyd ZCS:x.. */
				arg = RDMEM((opcode&3)+4);
				WRMEM((opcode&3)+4,mb88.Y);
				mb88.Y = arg;
				UPDATE_ZF(mb88.Y);
				mb88.st = 1;
				break;

			case 0x58:	case 0x59:	case 0x5a:	case 0x5b:
			case 0x5c:	case 0x5d:	case 0x5e:	case 0x5f: /* lxi ZCS:x.. */
				mb88.X = opcode & 7;
				UPDATE_ZF(mb88.X);
				mb88.st = 1;
				break;

			case 0x60:	case 0x61:	case 0x62:	case 0x63:
			case 0x64:	case 0x65:	case 0x66:	case 0x67: /* call imm ZCS:..x */
				arg = READOP(GETPC());
				INCPC();
				oc = 2;
				if ( TEST_ST() )
				{
					mb88.SP[mb88.SI] = GETPC();
					mb88.SI = ( mb88.SI + 1 ) & 3;
					mb88.PC = arg & 0x3f;
					mb88.PA = ( ( opcode & 7 ) << 2 ) | ( arg >> 6 );
					change_pc(GETPC());

				}
				mb88.st = 1;
				break;

			case 0x68:	case 0x69:	case 0x6a:	case 0x6b:
			case 0x6c:	case 0x6d:	case 0x6e:	case 0x6f: /* jpl imm ZCS:..x */
				arg = READOP(GETPC());
				INCPC();
				oc = 2;
				if ( TEST_ST() )
				{
					mb88.PC = arg & 0x3f;
					mb88.PA = ( ( opcode & 7 ) << 2 ) | ( arg >> 6 );
					change_pc(GETPC());
				}
				mb88.st = 1;
				break;

			case 0x70:	case 0x71:	case 0x72:	case 0x73:
			case 0x74:	case 0x75:	case 0x76:	case 0x77:
			case 0x78:	case 0x79:	case 0x7a:	case 0x7b:
			case 0x7c:	case 0x7d:	case 0x7e:	case 0x7f: /* ai ZCS:xxx */
				arg = opcode & 0x0f;
				arg += mb88.A;
				UPDATE_ST_C(arg);
				mb88.cf = mb88.st ^ 1;
				mb88.A = arg & 0x0f;
				UPDATE_ZF(mb88.A);
				break;

			case 0x80:	case 0x81:	case 0x82:	case 0x83:
			case 0x84:	case 0x85:	case 0x86:	case 0x87:
			case 0x88:	case 0x89:	case 0x8a:	case 0x8b:
			case 0x8c:	case 0x8d:	case 0x8e:	case 0x8f: /* lxi ZCS:x.. */
				mb88.Y = opcode & 0x0f;
				UPDATE_ZF(mb88.Y);
				mb88.st = 1;
				break;

			case 0x90:	case 0x91:	case 0x92:	case 0x93:
			case 0x94:	case 0x95:	case 0x96:	case 0x97:
			case 0x98:	case 0x99:	case 0x9a:	case 0x9b:
			case 0x9c:	case 0x9d:	case 0x9e:	case 0x9f: /* li ZCS:x.. */
				mb88.A = opcode & 0x0f;
				UPDATE_ZF(mb88.A);
				mb88.st = 1;
				break;

			case 0xa0:	case 0xa1:	case 0xa2:	case 0xa3:
			case 0xa4:	case 0xa5:	case 0xa6:	case 0xa7:
			case 0xa8:	case 0xa9:	case 0xaa:	case 0xab:
			case 0xac:	case 0xad:	case 0xae:	case 0xaf: /* cyi ZCS:xxx */
				arg = mb88.Y - (opcode & 0x0f);
				UPDATE_CF(arg);
				arg &= 0x0f;
				UPDATE_ST_Z(arg);
				mb88.zf = mb88.st ^ 1;
				break;

			case 0xb0:	case 0xb1:	case 0xb2:	case 0xb3:
			case 0xb4:	case 0xb5:	case 0xb6:	case 0xb7:
			case 0xb8:	case 0xb9:	case 0xba:	case 0xbb:
			case 0xbc:	case 0xbd:	case 0xbe:	case 0xbf: /* ci ZCS:xxx */
				arg = mb88.A - (opcode & 0x0f);
				UPDATE_CF(arg);
				arg &= 0x0f;
				UPDATE_ST_Z(arg);
				mb88.zf = mb88.st ^ 1;
				break;

			default: /* jmp ZCS:..x */
				if ( TEST_ST() )
				{
					mb88.PC = opcode & 0x3f;
					change_pc(GETPC());
				}
				mb88.st = 1;
				break;
		}

		/* update cycle counts */
		CYCLES( oc );

		/* update interrupts, serial and timer flags */
		update_pio();
	}

	return cycles - mb88_icount;
}

/***************************************************************************
    INFORMATION SETTERS
***************************************************************************/

static void mb88_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + MB88_IRQ_LINE:	set_irq_line(info->i);					break;

		case CPUINFO_INT_PC:
				mb88.PC = info->i & 0x3f;
				mb88.PA = (info->i >> 6) & 0x1f;
				change_pc(GETPC());
				break;
		case CPUINFO_INT_REGISTER + MB88_PC:			mb88.PC = info->i;						break;
		case CPUINFO_INT_REGISTER + MB88_PA:			mb88.PA = info->i;						break;
		case CPUINFO_INT_REGISTER + MB88_FLAGS:
				mb88.st = (info->i & 0x01) ? 1 : 0;
				mb88.zf = (info->i & 0x02) ? 1 : 0;
				mb88.cf = (info->i & 0x04) ? 1 : 0;
				mb88.vf = (info->i & 0x08) ? 1 : 0;
				mb88.sf = (info->i & 0x10) ? 1 : 0;
				mb88.nf = (info->i & 0x20) ? 1 : 0;
				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + MB88_SI:			mb88.SI = info->i & 0x03;				break;
		case CPUINFO_INT_REGISTER + MB88_A:				mb88.A = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + MB88_X:				mb88.X = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + MB88_Y:				mb88.Y = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + MB88_PIO:			mb88.pio = info->i & 0xff;				break;
		case CPUINFO_INT_REGISTER + MB88_TH:			mb88.TH = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + MB88_TL:			mb88.TL = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + MB88_SB:			mb88.SB = info->i & 0x0f;				break;
	}
}

/***************************************************************************
    INFORMATION GETTERS
***************************************************************************/

void mb88_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(mb88Regs);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 3;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 11;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 7;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 3;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + MB88_IRQ_LINE:	info->i = mb88.pending_interrupt ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_PC:							info->i = GETPC();						break;
		case CPUINFO_INT_REGISTER + MB88_PC: 			info->i = mb88.PC;						break;
		case CPUINFO_INT_REGISTER + MB88_PA: 			info->i = mb88.PA;						break;
		case CPUINFO_INT_REGISTER + MB88_FLAGS:			info->i = 0;
				if (TEST_ST()) info->i |= 0x01;
				if (TEST_ZF()) info->i |= 0x02;
				if (TEST_CF()) info->i |= 0x04;
				if (TEST_VF()) info->i |= 0x08;
				if (TEST_SF()) info->i |= 0x10;
				if (TEST_NF()) info->i |= 0x20;
				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + MB88_SI: 			info->i = mb88.SI;						break;
		case CPUINFO_INT_REGISTER + MB88_A: 			info->i = mb88.A;						break;
		case CPUINFO_INT_REGISTER + MB88_X: 			info->i = mb88.X;						break;
		case CPUINFO_INT_REGISTER + MB88_Y: 			info->i = mb88.Y;						break;
		case CPUINFO_INT_REGISTER + MB88_PIO: 			info->i = mb88.pio;						break;
		case CPUINFO_INT_REGISTER + MB88_TH: 			info->i = mb88.TH;						break;
		case CPUINFO_INT_REGISTER + MB88_TL: 			info->i = mb88.TL;						break;
		case CPUINFO_INT_REGISTER + MB88_SB: 			info->i = mb88.SB;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = mb88_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = mb88_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = mb88_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = mb88_init;					break;
		case CPUINFO_PTR_RESET:							info->reset = mb88_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = NULL;						break;
		case CPUINFO_PTR_EXECUTE:						info->execute = mb88_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = mb88_dasm;			break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &mb88_icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB88xx");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Fujitsu MB88xx");	break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright 2007 Ernesto Corvi"); break;

		case CPUINFO_STR_FLAGS:
    		sprintf(info->s, "%c%c%c%c%c%c",
	        		TEST_ST() ? 'T' : 't',
	        		TEST_ZF() ? 'Z' : 'z',
	        		TEST_CF() ? 'C' : 'c',
	        		TEST_VF() ? 'V' : 'v',
	        		TEST_SF() ? 'S' : 's',
	        		TEST_NF() ? 'I' : 'i');
	        break;

        case CPUINFO_STR_REGISTER + MB88_FLAGS:
    		sprintf(info->s, "FL:%c%c%c%c%c%c",
	        		TEST_ST() ? 'T' : 't',
	        		TEST_ZF() ? 'Z' : 'z',
	        		TEST_CF() ? 'C' : 'c',
	        		TEST_VF() ? 'V' : 'v',
	        		TEST_SF() ? 'S' : 's',
	        		TEST_NF() ? 'I' : 'i');
	        break;

        case CPUINFO_STR_REGISTER + MB88_PC:			sprintf(info->s, "PC:%02X", mb88.PC);	break;
        case CPUINFO_STR_REGISTER + MB88_PA:			sprintf(info->s, "PA:%02X", mb88.PA);	break;
        case CPUINFO_STR_REGISTER + MB88_SI:			sprintf(info->s, "SI:%1X", mb88.SI);	break;
		case CPUINFO_STR_REGISTER + MB88_A:				sprintf(info->s, "A:%1X", mb88.A);		break;
		case CPUINFO_STR_REGISTER + MB88_X:				sprintf(info->s, "X:%1X", mb88.X);		break;
		case CPUINFO_STR_REGISTER + MB88_Y:				sprintf(info->s, "Y:%1X", mb88.Y);		break;
		case CPUINFO_STR_REGISTER + MB88_PIO:			sprintf(info->s, "PIO:%02X", mb88.pio);	break;
        case CPUINFO_STR_REGISTER + MB88_TH:			sprintf(info->s, "TH:%1X", mb88.TH);	break;
		case CPUINFO_STR_REGISTER + MB88_TL:			sprintf(info->s, "TL:%1X", mb88.TL);	break;
		case CPUINFO_STR_REGISTER + MB88_SB:			sprintf(info->s, "SB:%1X", mb88.SB);	break;
	}
}

void mb8841_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 11;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 7;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB8841");				break;

		default:										mb88_get_info(state, info);			break;
	}
}

void mb8842_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 11;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 7;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB8842");				break;

		default:										mb88_get_info(state, info);			break;
	}
}

void mb8843_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 10;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 6;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB8843");				break;

		default:										mb88_get_info(state, info);			break;
	}
}

void mb8844_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 10;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 6;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB8844");				break;

		default:										mb88_get_info(state, info);			break;
	}
}
