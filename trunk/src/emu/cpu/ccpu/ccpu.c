/***************************************************************************

    ccpu.c
    Core implementation for the portable Cinematronics CPU emulator.

    Written by Aaron Giles
    Special thanks to Zonn Moore for his detailed documentation.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "ccpu.h"


/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

struct ccpu_state
{
	UINT16              PC;
	UINT16              A;
	UINT16              B;
	UINT8               I;
	UINT16              J;
	UINT8               P;
	UINT16              X;
	UINT16              Y;
	UINT16              T;
	UINT16 *            acc;

	UINT16              a0flag, ncflag, cmpacc, cmpval;
	UINT16              miflag, nextmiflag, nextnextmiflag;
	UINT16              drflag;

	ccpu_input_func     external_input;
	ccpu_vector_func    vector_callback;

	UINT8               waiting;
	UINT8               watchdog;

	int                 icount;

	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *data;
	address_space *io;
};


INLINE ccpu_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CCPU);
	return (ccpu_state *)downcast<legacy_cpu_device *>(device)->token();
}


/***************************************************************************
    MACROS
***************************************************************************/

#define READOP(C,a)         ((C)->direct->read_decrypted_byte(a))

#define RDMEM(C,a)          ((C)->data->read_word((a) * 2) & 0xfff)
#define WRMEM(C,a,v)        ((C)->data->write_word((a) * 2, (v)))

#define READPORT(C,a)       ((C)->io->read_byte(a))
#define WRITEPORT(C,a,v)    ((C)->io->write_byte((a), (v)))

#define SET_A0(C)           do { (C)->a0flag = (C)->A; } while (0)
#define SET_CMP_VAL(C,x)    do { (C)->cmpacc = *(C)->acc; (C)->cmpval = (x) & 0xfff; } while (0)
#define SET_NC(C,a)         do { (C)->ncflag = ~(a); } while (0)
#define SET_MI(C,a)         do { (C)->nextnextmiflag = (a); } while (0)

#define TEST_A0(C)          ((C)->a0flag & 1)
#define TEST_NC(C)          (((C)->ncflag >> 12) & 1)
#define TEST_MI(C)          (((C)->miflag >> 11) & 1)
#define TEST_LT(C)          ((C)->cmpval < (C)->cmpacc)
#define TEST_EQ(C)          ((C)->cmpval == (C)->cmpacc)
#define TEST_DR(C)          ((C)->drflag != 0)

#define NEXT_ACC_A(C)       do { SET_MI(C, *(C)->acc); (C)->acc = &(C)->A; } while (0)
#define NEXT_ACC_B(C)       do { SET_MI(C, *(C)->acc); if ((C)->acc == &(C)->A) (C)->acc = &(C)->B; else (C)->acc = &(C)->A; } while (0)

#define CYCLES(C,x)         do { (C)->icount -= (x); } while (0)

#define STANDARD_ACC_OP(C,resexp,cmpval) \
do { \
	UINT16 result = resexp; \
	SET_A0(C);                      /* set the A0 bit based on the previous 'A' value */ \
	SET_CMP_VAL(C,cmpval);          /* set the compare values to the previous accumulator and the cmpval */ \
	SET_NC(C,result);               /* set the NC flag based on the unmasked result */ \
	*(C)->acc = result & 0xfff;     /* store the low 12 bits of the new value */ \
} while (0)



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static UINT8 read_jmi(device_t *device)
{
	/* this routine is called when there is no external input */
	/* and the JMI jumper is present */
	ccpu_state *cpustate = get_safe_token(device);
	return TEST_MI(cpustate);
}


void ccpu_wdt_timer_trigger(device_t *device)
{
	ccpu_state *cpustate = get_safe_token(device);
	cpustate->waiting = FALSE;
	cpustate->watchdog++;
	if (cpustate->watchdog >= 3)
		cpustate->PC = 0;
}


static CPU_INIT( ccpu )
{
	const ccpu_config *configdata = (const ccpu_config *)device->static_config();
	ccpu_state *cpustate = get_safe_token(device);

	/* copy input params */
	cpustate->external_input = configdata->external_input ? configdata->external_input : read_jmi;
	cpustate->vector_callback = configdata->vector_callback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = &device->space(AS_DATA);
	cpustate->io = &device->space(AS_IO);

	device->save_item(NAME(cpustate->PC));
	device->save_item(NAME(cpustate->A));
	device->save_item(NAME(cpustate->B));
	device->save_item(NAME(cpustate->I));
	device->save_item(NAME(cpustate->J));
	device->save_item(NAME(cpustate->P));
	device->save_item(NAME(cpustate->X));
	device->save_item(NAME(cpustate->Y));
	device->save_item(NAME(cpustate->T));
	device->save_item(NAME(cpustate->a0flag));
	device->save_item(NAME(cpustate->ncflag));
	device->save_item(NAME(cpustate->cmpacc));
	device->save_item(NAME(cpustate->cmpval));
	device->save_item(NAME(cpustate->miflag));
	device->save_item(NAME(cpustate->nextmiflag));
	device->save_item(NAME(cpustate->nextnextmiflag));
	device->save_item(NAME(cpustate->drflag));
	device->save_item(NAME(cpustate->waiting));
	device->save_item(NAME(cpustate->watchdog));
}


static CPU_RESET( ccpu )
{
	ccpu_state *cpustate = get_safe_token(device);

	/* zero registers */
	cpustate->PC = 0;
	cpustate->A = 0;
	cpustate->B = 0;
	cpustate->I = 0;
	cpustate->J = 0;
	cpustate->P = 0;
	cpustate->X = 0;
	cpustate->Y = 0;
	cpustate->T = 0;
	cpustate->acc = &cpustate->A;

	/* zero flags */
	cpustate->a0flag = 0;
	cpustate->ncflag = 0;
	cpustate->cmpacc = 0;
	cpustate->cmpval = 1;
	cpustate->miflag = cpustate->nextmiflag = cpustate->nextnextmiflag = 0;
	cpustate->drflag = 0;

	cpustate->waiting = FALSE;
	cpustate->watchdog = 0;
}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

static CPU_EXECUTE( ccpu )
{
	ccpu_state *cpustate = get_safe_token(device);

	if (cpustate->waiting)
	{
		cpustate->icount = 0;
		return;
	}

	do
	{
		UINT16 tempval;
		UINT8 opcode;

		/* update the delayed MI flag */
		cpustate->miflag = cpustate->nextmiflag;
		cpustate->nextmiflag = cpustate->nextnextmiflag;

		/* fetch the opcode */
		debugger_instruction_hook(device, cpustate->PC);
		opcode = READOP(cpustate, cpustate->PC++);

		switch (opcode)
		{
			/* LDAI */
			case 0x00:  case 0x01:  case 0x02:  case 0x03:
			case 0x04:  case 0x05:  case 0x06:  case 0x07:
			case 0x08:  case 0x09:  case 0x0a:  case 0x0b:
			case 0x0c:  case 0x0d:  case 0x0e:  case 0x0f:
				tempval = (opcode & 0x0f) << 8;
				STANDARD_ACC_OP(cpustate, tempval, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* INP */
			case 0x10:  case 0x11:  case 0x12:  case 0x13:
			case 0x14:  case 0x15:  case 0x16:  case 0x17:
			case 0x18:  case 0x19:  case 0x1a:  case 0x1b:
			case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
				if (cpustate->acc == &cpustate->A)
					tempval = READPORT(cpustate, opcode & 0x0f) & 1;
				else
					tempval = READPORT(cpustate, 16 + (opcode & 0x07)) & 1;
				STANDARD_ACC_OP(cpustate, tempval, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* A8I */
			case 0x20:
				tempval = READOP(cpustate, cpustate->PC++);
				STANDARD_ACC_OP(cpustate, *cpustate->acc + tempval, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* A4I */
			case 0x21:  case 0x22:  case 0x23:
			case 0x24:  case 0x25:  case 0x26:  case 0x27:
			case 0x28:  case 0x29:  case 0x2a:  case 0x2b:
			case 0x2c:  case 0x2d:  case 0x2e:  case 0x2f:
				tempval = opcode & 0x0f;
				STANDARD_ACC_OP(cpustate, *cpustate->acc + tempval, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* S8I */
			case 0x30:
				tempval = READOP(cpustate, cpustate->PC++);
				STANDARD_ACC_OP(cpustate, *cpustate->acc + (tempval ^ 0xfff) + 1, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* S4I */
			case 0x31:  case 0x32:  case 0x33:
			case 0x34:  case 0x35:  case 0x36:  case 0x37:
			case 0x38:  case 0x39:  case 0x3a:  case 0x3b:
			case 0x3c:  case 0x3d:  case 0x3e:  case 0x3f:
				tempval = opcode & 0x0f;
				STANDARD_ACC_OP(cpustate, *cpustate->acc + (tempval ^ 0xfff) + 1, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* LPAI */
			case 0x40:  case 0x41:  case 0x42:  case 0x43:
			case 0x44:  case 0x45:  case 0x46:  case 0x47:
			case 0x48:  case 0x49:  case 0x4a:  case 0x4b:
			case 0x4c:  case 0x4d:  case 0x4e:  case 0x4f:
				tempval = READOP(cpustate, cpustate->PC++);
				cpustate->J = (opcode & 0x0f) + (tempval & 0xf0) + ((tempval & 0x0f) << 8);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* T4K */
			case 0x50:
				cpustate->PC = (cpustate->P << 12) + cpustate->J;
				NEXT_ACC_B(cpustate); CYCLES(cpustate, 4);
				break;

			/* JMIB/JEHB */
			case 0x51:
				if ((*cpustate->external_input)(device)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_B(cpustate); CYCLES(cpustate, 2);
				break;

			/* JVNB */
			case 0x52:
				if (TEST_DR(cpustate)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_B(cpustate); CYCLES(cpustate, 2);
				break;

			/* JLTB */
			case 0x53:
				if (TEST_LT(cpustate)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_B(cpustate); CYCLES(cpustate, 2);
				break;

			/* JEQB */
			case 0x54:
				if (TEST_EQ(cpustate)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_B(cpustate); CYCLES(cpustate, 2);
				break;

			/* JCZB */
			case 0x55:
				if (TEST_NC(cpustate)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_B(cpustate); CYCLES(cpustate, 2);
				break;

			/* JOSB */
			case 0x56:
				if (TEST_A0(cpustate)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_B(cpustate); CYCLES(cpustate, 2);
				break;

			/* SSA */
			case 0x57:
				NEXT_ACC_B(cpustate); CYCLES(cpustate, 2);
				break;

			/* JMP */
			case 0x58:
				cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J;
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 4);
				break;

			/* JMI/JEH */
			case 0x59:
				if ((*cpustate->external_input)(device)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* JVN */
			case 0x5a:
				if (TEST_DR(cpustate)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* JLT */
			case 0x5b:
				if (TEST_LT(cpustate)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* JEQ */
			case 0x5c:
				if (TEST_EQ(cpustate)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* JCZ */
			case 0x5d:
				if (TEST_NC(cpustate)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* JOS */
			case 0x5e:
				if (TEST_A0(cpustate)) { cpustate->PC = ((cpustate->PC - 1) & 0xf000) + cpustate->J; CYCLES(cpustate, 2); }
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* NOP */
			case 0x5f:
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* ADD */
			case 0x60:  case 0x61:  case 0x62:  case 0x63:
			case 0x64:  case 0x65:  case 0x66:  case 0x67:
			case 0x68:  case 0x69:  case 0x6a:  case 0x6b:
			case 0x6c:  case 0x6d:  case 0x6e:  case 0x6f:
				cpustate->I = (cpustate->P << 4) + (opcode & 0x0f);
				tempval = RDMEM(cpustate, cpustate->I);
				STANDARD_ACC_OP(cpustate, *cpustate->acc + tempval, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* SUB */
			case 0x70:  case 0x71:  case 0x72:  case 0x73:
			case 0x74:  case 0x75:  case 0x76:  case 0x77:
			case 0x78:  case 0x79:  case 0x7a:  case 0x7b:
			case 0x7c:  case 0x7d:  case 0x7e:  case 0x7f:
				cpustate->I = (cpustate->P << 4) + (opcode & 0x0f);
				tempval = RDMEM(cpustate, cpustate->I);
				STANDARD_ACC_OP(cpustate, *cpustate->acc + (tempval ^ 0xfff) + 1, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* SETP */
			case 0x80:  case 0x81:  case 0x82:  case 0x83:
			case 0x84:  case 0x85:  case 0x86:  case 0x87:
			case 0x88:  case 0x89:  case 0x8a:  case 0x8b:
			case 0x8c:  case 0x8d:  case 0x8e:  case 0x8f:
				cpustate->P = opcode & 0x0f;
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* OUT */
			case 0x90:  case 0x91:  case 0x92:  case 0x93:
			case 0x94:  case 0x95:  case 0x96:  case 0x97:
			case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
			case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:
				if (cpustate->acc == &cpustate->A)
					WRITEPORT(cpustate, opcode & 0x07, ~*cpustate->acc & 1);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* LDA */
			case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:
			case 0xa4:  case 0xa5:  case 0xa6:  case 0xa7:
			case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:
			case 0xac:  case 0xad:  case 0xae:  case 0xaf:
				cpustate->I = (cpustate->P << 4) + (opcode & 0x0f);
				tempval = RDMEM(cpustate, cpustate->I);
				STANDARD_ACC_OP(cpustate, tempval, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* TST */
			case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:
			case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
			case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:
			case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
				cpustate->I = (cpustate->P << 4) + (opcode & 0x0f);
				tempval = RDMEM(cpustate, cpustate->I);
				{
					UINT16 result = *cpustate->acc + (tempval ^ 0xfff) + 1;
					SET_A0(cpustate);
					SET_CMP_VAL(cpustate, tempval);
					SET_NC(cpustate, result);
					SET_MI(cpustate, result);
				}
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* WS */
			case 0xc0:  case 0xc1:  case 0xc2:  case 0xc3:
			case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
			case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
			case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:
				cpustate->I = (cpustate->P << 4) + (opcode & 0x0f);
				cpustate->I = RDMEM(cpustate, cpustate->I) & 0xff;
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* STA */
			case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:
			case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
			case 0xd8:  case 0xd9:  case 0xda:  case 0xdb:
			case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:
				cpustate->I = (cpustate->P << 4) + (opcode & 0x0f);
				WRMEM(cpustate, cpustate->I, *cpustate->acc);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* DV */
			case 0xe0:
				{
					INT16 stopX = (INT16)(cpustate->A << 4) >> 4;
					INT16 stopY = (INT16)(cpustate->B << 4) >> 4;

					stopX = ((INT16)(stopX - cpustate->X) >> cpustate->T) + cpustate->X;
					stopY = ((INT16)(stopY - cpustate->Y) >> cpustate->T) + cpustate->Y;

					(*cpustate->vector_callback)(device, cpustate->X, cpustate->Y, stopX, stopY, cpustate->T);

					/* hack to make QB3 display semi-correctly during explosions */
					cpustate->A = cpustate->X & 0xfff;
					cpustate->B = cpustate->Y & 0xfff;
				}
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* LPAP */
			case 0xe1:
				cpustate->J = RDMEM(cpustate, cpustate->I);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* WSP */
			case 0xf1:
				cpustate->I = RDMEM(cpustate, cpustate->I) & 0xff;
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* LKP */
			case 0xe2:
			case 0xf2:
				tempval = READOP(cpustate, ((cpustate->PC - 1) & 0xf000) + *cpustate->acc);
				STANDARD_ACC_OP(cpustate, tempval, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 7);
				cpustate->PC++;
				break;

			/* MUL */
			case 0xe3:
			case 0xf3:
				tempval = RDMEM(cpustate, cpustate->I);
				SET_A0(cpustate);
				cpustate->cmpval = tempval & 0xfff;
				if (cpustate->acc == &cpustate->A)
				{
					if (cpustate->A & 1)
					{
						UINT16 result;
						cpustate->cmpacc = cpustate->B;
						cpustate->A = (cpustate->A >> 1) | ((cpustate->B << 11) & 0x800);
						cpustate->B = ((INT16)(cpustate->B << 4) >> 5) & 0xfff;
						result = cpustate->B + tempval;
						SET_NC(cpustate, result);
						SET_MI(cpustate, result);
						cpustate->B = result & 0xfff;
					}
					else
					{
						UINT16 result;
						cpustate->cmpacc = cpustate->A;
						result = cpustate->A + tempval;
						cpustate->A = (cpustate->A >> 1) | ((cpustate->B << 11) & 0x800);
						cpustate->B = ((INT16)(cpustate->B << 4) >> 5) & 0xfff;
						SET_NC(cpustate, result);
						SET_MI(cpustate, result);
					}
				}
				else
				{
					UINT16 result;
					cpustate->cmpacc = cpustate->B;
					cpustate->B = ((INT16)(cpustate->B << 4) >> 5) & 0xfff;
					result = cpustate->B + tempval;
					SET_NC(cpustate, result);
					SET_MI(cpustate, result);
					if (cpustate->A & 1)
						cpustate->B = result & 0xfff;
				}
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* NV */
			case 0xe4:
			case 0xf4:
				cpustate->T = 0;
				while (((cpustate->A & 0xa00) == 0x000 || (cpustate->A & 0xa00) == 0xa00) &&
						((cpustate->B & 0xa00) == 0x000 || (cpustate->B & 0xa00) == 0xa00) &&
						cpustate->T < 16)
				{
					cpustate->A = (cpustate->A << 1) & 0xfff;
					cpustate->B = (cpustate->B << 1) & 0xfff;
					cpustate->T++;
					CYCLES(cpustate, 1);
				}
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* FRM */
			case 0xe5:
			case 0xf5:
				cpustate->waiting = TRUE;
				NEXT_ACC_A(cpustate);
				cpustate->icount = -1;

				/* some games repeat the FRM opcode twice; it apparently does not cause
				   a second wait, so we make sure we skip any duplicate opcode at this
				   point */
				if (READOP(cpustate, cpustate->PC) == opcode)
					cpustate->PC++;
				break;

			/* STAP */
			case 0xe6:
			case 0xf6:
				WRMEM(cpustate, cpustate->I, *cpustate->acc);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* CST */
			case 0xf7:
				cpustate->watchdog = 0;
			/* ADDP */
			case 0xe7:
				tempval = RDMEM(cpustate, cpustate->I);
				STANDARD_ACC_OP(cpustate, *cpustate->acc + tempval, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* SUBP */
			case 0xe8:
			case 0xf8:
				tempval = RDMEM(cpustate, cpustate->I);
				STANDARD_ACC_OP(cpustate, *cpustate->acc + (tempval ^ 0xfff) + 1, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 3);
				break;

			/* ANDP */
			case 0xe9:
			case 0xf9:
				tempval = RDMEM(cpustate, cpustate->I);
				STANDARD_ACC_OP(cpustate, *cpustate->acc & tempval, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* LDAP */
			case 0xea:
			case 0xfa:
				tempval = RDMEM(cpustate, cpustate->I);
				STANDARD_ACC_OP(cpustate, tempval, tempval);
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 2);
				break;

			/* SHR */
			case 0xeb:
			case 0xfb:
				tempval = ((cpustate->acc == &cpustate->A) ? (cpustate->A >> 1) : ((INT16)(cpustate->B << 4) >> 5)) & 0xfff;
				tempval |= (*cpustate->acc + (0xb0b | (opcode & 0xf0))) & 0x1000;
				STANDARD_ACC_OP(cpustate, tempval, 0xb0b | (opcode & 0xf0));
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* SHL */
			case 0xec:
			case 0xfc:
				tempval = (*cpustate->acc << 1) & 0xfff;
				tempval |= (*cpustate->acc + (0xc0c | (opcode & 0xf0))) & 0x1000;
				STANDARD_ACC_OP(cpustate, tempval, 0xc0c | (opcode & 0xf0));
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* ASR */
			case 0xed:
			case 0xfd:
				tempval = ((INT16)(*cpustate->acc << 4) >> 5) & 0xfff;
				STANDARD_ACC_OP(cpustate, tempval, 0xd0d | (opcode & 0xf0));
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* SHRB */
			case 0xee:
			case 0xfe:
				if (cpustate->acc == &cpustate->A)
				{
					tempval = (cpustate->A >> 1) | ((cpustate->B << 11) & 0x800);
					cpustate->B = ((INT16)(cpustate->B << 4) >> 5) & 0xfff;
				}
				else
					tempval = ((INT16)(cpustate->B << 4) >> 5) & 0xfff;
				tempval |= (*cpustate->acc + (0xe0e | (opcode & 0xf0))) & 0x1000;
				STANDARD_ACC_OP(cpustate, tempval, 0xe0e | (opcode & 0xf0));
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* SHLB */
			case 0xef:
			case 0xff:
				if (cpustate->acc == &cpustate->A)
				{
					tempval = (cpustate->A << 1) & 0xfff;
					cpustate->B = (cpustate->B << 1) & 0xfff;
				}
				else
					tempval = (cpustate->B << 1) & 0xfff;
				tempval |= (*cpustate->acc + (0xf0f | (opcode & 0xf0))) & 0x1000;
				STANDARD_ACC_OP(cpustate, tempval, 0xf0f | (opcode & 0xf0));
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;

			/* IV */
			case 0xf0:
				cpustate->X = (INT16)(cpustate->A << 4) >> 4;
				cpustate->Y = (INT16)(cpustate->B << 4) >> 4;
				NEXT_ACC_A(cpustate); CYCLES(cpustate, 1);
				break;
		}
	} while (cpustate->icount > 0);
}



/***************************************************************************
    INFORMATION SETTERS
***************************************************************************/

static CPU_SET_INFO( ccpu )
{
	ccpu_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CCPU_PC:            cpustate->PC = info->i;                     break;
		case CPUINFO_INT_REGISTER + CCPU_FLAGS:
				cpustate->a0flag = (info->i & 0x01) ? 1 : 0;
				cpustate->ncflag = (info->i & 0x02) ? 0x0000 : 0x1000;
				cpustate->cmpacc = 1;
				cpustate->cmpval = (info->i & 0x04) ? 0 : (info->i & 0x08) ? 1 : 2;
				cpustate->miflag = (info->i & 0x10) ? 1 : 0;
				cpustate->drflag = (info->i & 0x20) ? 1 : 0;
				break;
		case CPUINFO_INT_REGISTER + CCPU_A:             cpustate->A = info->i & 0xfff;              break;
		case CPUINFO_INT_REGISTER + CCPU_B:             cpustate->B = info->i & 0xfff;              break;
		case CPUINFO_INT_REGISTER + CCPU_I:             cpustate->I = info->i & 0xff;               break;
		case CPUINFO_INT_REGISTER + CCPU_J:             cpustate->J = info->i & 0xfff;              break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + CCPU_P:             cpustate->P = info->i & 0x0f;               break;
		case CPUINFO_INT_REGISTER + CCPU_X:             cpustate->X = info->i & 0xfff;              break;
		case CPUINFO_INT_REGISTER + CCPU_Y:             cpustate->Y = info->i & 0xfff;              break;
		case CPUINFO_INT_REGISTER + CCPU_T:             cpustate->T = info->i & 0xfff;              break;
	}
}



/***************************************************************************
    INFORMATION GETTERS
***************************************************************************/

CPU_GET_INFO( ccpu )
{
	ccpu_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:                  info->i = sizeof(ccpu_state);                   break;
		case CPUINFO_INT_INPUT_LINES:                   info->i = 0;                                    break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:            info->i = 0;                                    break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;                           break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:              info->i = 1;                                    break;
		case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 1;                                    break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:         info->i = 1;                                    break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:         info->i = 3;                                    break;
		case CPUINFO_INT_MIN_CYCLES:                    info->i = 1;                                    break;
		case CPUINFO_INT_MAX_CYCLES:                    info->i = 1;                                    break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 8;                            break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 15;                          break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;                           break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:   info->i = 16;                           break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:   info->i = 8;                            break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:   info->i = -1;                           break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:     info->i = 8;                            break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:     info->i = 5;                            break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:     info->i = 0;                            break;

		case CPUINFO_INT_PREVIOUSPC:                    /* not implemented */                           break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CCPU_PC:            info->i = cpustate->PC;                         break;
		case CPUINFO_INT_REGISTER + CCPU_FLAGS:         info->i = 0;
				if (TEST_A0(cpustate)) info->i |= 0x01;
				if (TEST_NC(cpustate)) info->i |= 0x02;
				if (TEST_LT(cpustate)) info->i |= 0x04;
				if (TEST_EQ(cpustate)) info->i |= 0x08;
				if ((*cpustate->external_input)(cpustate->device)) info->i |= 0x10;
				if (TEST_DR(cpustate)) info->i |= 0x20;
				break;
		case CPUINFO_INT_REGISTER + CCPU_A:             info->i = cpustate->A;                          break;
		case CPUINFO_INT_REGISTER + CCPU_B:             info->i = cpustate->B;                          break;
		case CPUINFO_INT_REGISTER + CCPU_I:             info->i = cpustate->I;                          break;
		case CPUINFO_INT_REGISTER + CCPU_J:             info->i = cpustate->J;                          break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + CCPU_P:             info->i = cpustate->P;                          break;
		case CPUINFO_INT_REGISTER + CCPU_X:             info->i = cpustate->X;                          break;
		case CPUINFO_INT_REGISTER + CCPU_Y:             info->i = cpustate->Y;                          break;
		case CPUINFO_INT_REGISTER + CCPU_T:             info->i = cpustate->T;                          break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(ccpu);        break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(ccpu);               break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(ccpu);             break;
		case CPUINFO_FCT_EXIT:                          info->exit = NULL;                              break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(ccpu);         break;
		case CPUINFO_FCT_BURN:                          info->burn = NULL;                              break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(ccpu); break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:           info->icount = &cpustate->icount;               break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "CCPU");                        break;
		case CPUINFO_STR_FAMILY:                    strcpy(info->s, "Cinematronics CPU");           break;
		case CPUINFO_STR_VERSION:                   strcpy(info->s, "1.0");                         break;
		case CPUINFO_STR_SOURCE_FILE:                       strcpy(info->s, __FILE__);                      break;
		case CPUINFO_STR_CREDITS:                   strcpy(info->s, "Copyright Aaron Giles & Zonn Moore"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c",
					TEST_A0(cpustate) ? '0' : 'o',
					TEST_NC(cpustate) ? 'N' : 'n',
					TEST_LT(cpustate) ? 'L' : 'l',
					TEST_EQ(cpustate) ? 'E' : 'e',
					(*cpustate->external_input)(cpustate->device) ? 'M' : 'm',
					TEST_DR(cpustate) ? 'D' : 'd');
			break;

		case CPUINFO_STR_REGISTER + CCPU_FLAGS:
			sprintf(info->s, "FL:%c%c%c%c%c%c",
					TEST_A0(cpustate) ? '0' : 'o',
					TEST_NC(cpustate) ? 'N' : 'n',
					TEST_LT(cpustate) ? 'L' : 'l',
					TEST_EQ(cpustate) ? 'E' : 'e',
					(*cpustate->external_input)(cpustate->device) ? 'M' : 'm',
					TEST_DR(cpustate) ? 'D' : 'd');
			break;

		case CPUINFO_STR_REGISTER + CCPU_PC:            sprintf(info->s, "PC:%04X", cpustate->PC);      break;
		case CPUINFO_STR_REGISTER + CCPU_A:             sprintf(info->s, "A:%03X",  cpustate->A);       break;
		case CPUINFO_STR_REGISTER + CCPU_B:             sprintf(info->s, "B:%03X",  cpustate->B);       break;
		case CPUINFO_STR_REGISTER + CCPU_I:             sprintf(info->s, "I:%03X",  cpustate->I);       break;
		case CPUINFO_STR_REGISTER + CCPU_J:             sprintf(info->s, "J:%03X",  cpustate->J);       break;
		case CPUINFO_STR_REGISTER + CCPU_P:             sprintf(info->s, "P:%X",    cpustate->P);       break;
		case CPUINFO_STR_REGISTER + CCPU_X:             sprintf(info->s, "X:%03X",  cpustate->X);       break;
		case CPUINFO_STR_REGISTER + CCPU_Y:             sprintf(info->s, "Y:%03X",  cpustate->Y);       break;
		case CPUINFO_STR_REGISTER + CCPU_T:             sprintf(info->s, "T:%03X",  cpustate->T);       break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(CCPU, ccpu);
