/***************************************************************************

    esrip.c

    Implementation of the Entertainment Sciences
    AM29116-based Real Time Image Processor

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "esrip.h"

CPU_DISASSEMBLE( esrip );


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define IPT_RAM_SIZE	(8192 * sizeof(UINT16))


/***************************************************************************
    MACROS
***************************************************************************/

#define RIP_PC		(cpustate->pc | ((cpustate->status_out & 1) << 8))
#define _BIT(x, n)	((x) & (1 << (n)))
#define RISING_EDGE(old_val, new_val, bit)	(!(old_val & (1 << bit)) && (new_val & (1 << bit)))

#define UNHANDLED	do {printf("%s:UNHANDLED (%x)\n", __FUNCTION__, inst); assert(0);} while (0)
#define INVALID		do {printf("%s:INVALID (%x)\n", __FUNCTION__, inst); assert(0);} while (0)

#define RAM_ADDR	(inst & 0x1f)
#define MODE		(inst & 0x8000)
#define WORD_MODE	(inst & 0x8000)
#define BYTE_MODE	(!WORD_MODE)
#define N			((inst >> 9) & 0xf)
#define OPCODE		((inst >> 5) & 0xf)
#define SRC			((inst >> 9) & 0xf)
#define	DST			(inst & 0x1f)	// TEST

#define BW_WORD		(1 << 15)
#define	BW_BYTE		(0 << 15)

#define FLAG_3		(1 << 7)
#define FLAG_2		(1 << 6)
#define FLAG_1		(1 << 5)
#define L_FLAG		(1 << 4)
#define	V_FLAG		(1 << 3)
#define	N_FLAG		(1 << 2)
#define	C_FLAG		(1 << 1)
#define	Z_FLAG		(1 << 0)

#define CLEAR_FLAGS(a)	(cpustate->new_status &= ~(a))
#define SET_FLAGS(a)	(cpustate->new_status |=  (a))


/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

struct esrip_state 
{
	UINT16	ram[32];
	UINT16	acc;
	UINT16	d_latch;
	UINT16	i_latch;
	UINT16	result;
	UINT8	new_status;
	UINT8	status;
	UINT16	inst;
	UINT8	immflag;
	UINT8	ct;
	UINT8	t;

	/* Instruction latches - current and previous values */
	UINT8	l1, pl1;
	UINT8	l2, pl2;
	UINT8	l3, pl3;
	UINT8	l4, pl4;
	UINT8	l5, pl5;
	UINT8	l6, pl6;
	UINT8	l7, pl7;

	UINT8	pc;
	UINT8	status_out;

	UINT8	x_scale;
	UINT8	y_scale;
	UINT8	img_bank;
	UINT8	line_latch;
	UINT16	fig_latch;
	UINT16	attr_latch;
	UINT16	adl_latch;
	UINT16	adr_latch;
	UINT16	iaddr_latch;
	UINT8	c_latch;

	UINT16	fdt_cnt;
	UINT16	ipt_cnt;

	UINT8	fig;
	UINT16	fig_cycles;

	UINT8	*optable;

	UINT16	*ipt_ram;
	UINT8	*lbrm;

	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	int		icount;

	read16_device_func	fdt_r;
	write16_device_func	fdt_w;
	UINT8 (*status_in)(running_machine &machine);
	int (*draw)(running_machine &machine, int l, int r, int fig, int attr, int addr, int col, int x_scale, int bank);
};


INLINE esrip_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == ESRIP);
	return (esrip_state *)downcast<legacy_cpu_device *>(device)->token();
}


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

UINT8 get_rip_status(device_t *cpu)
{
	esrip_state *cpustate = get_safe_token(cpu);
	return cpustate->status_out;
}


/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

enum ops
{
	ROTR1, TOR1, ROTR2, ROTC, ROTM, BOR2, CRCF, CRCR,
	SVSTR, PRT, SOR, TOR2, SHFTR, TEST, NOP, SETST, RSTST,
	ROTNR, BONR, BOR1, SONR, SHFTNR, PRTNR, TONR
};

static void make_ops(esrip_state *cpustate)
{
	int inst;

	for (inst = 0; inst < 65536; ++inst)
	{
		int quad = (inst >> 13) & 3;

		if (quad == 0)
		{
			if (((inst >> 5) & 0xc) == 0xc)
				cpustate->optable[inst] = ROTR1;
			else
				cpustate->optable[inst] = TOR1;
		}
		else if (quad == 1)
		{
			if (OPCODE < 2)
				cpustate->optable[inst] = ROTR2;
			else if (OPCODE < 6)
				cpustate->optable[inst] = ROTC;
			else
				cpustate->optable[inst] = ROTM;
		}
		else if (quad == 2)
		{
			if (OPCODE > 11)
				cpustate->optable[inst] = BOR2;
			else
			{
				int tmp = (inst >> 5) & 0xff;

				if (tmp == 0x63)
					cpustate->optable[inst] = CRCF;
				else if (tmp == 0x69)
					cpustate->optable[inst] = CRCR;
				else if (tmp == 0x7a)
					cpustate->optable[inst] = SVSTR;
				else
				{
					if ((SRC > 7) && (SRC < 12))
						cpustate->optable[inst] = PRT;
					else if (SRC > 11)
						cpustate->optable[inst] = SOR;
					else if (SRC < 6)
						cpustate->optable[inst] = TOR2;
					else
						cpustate->optable[inst] = SHFTR;
				}
			}
		}
		else
		{
			if (inst == 0x7140)
				cpustate->optable[inst] = NOP;
			else
			{
				int x = (inst & 0xffe0);
				if (x == 0x7340)
					cpustate->optable[inst] = TEST;
				else if (x == 0x7740)
					cpustate->optable[inst] = SETST;
				else if (x == 0x7540)
					cpustate->optable[inst] = RSTST;
				else
				{
					int op = OPCODE;
					if (op == 0xc)
					{
						if ((inst & 0x18) == 0x18)
							cpustate->optable[inst] = ROTNR;
						else
							cpustate->optable[inst] = BONR;
					}
					else if ((op & 0xc) == 0xc)
						cpustate->optable[inst] = BOR1;
					else
					{
						int src = SRC;

						if ((src & 0xc) == 0xc)
							cpustate->optable[inst] = SONR;
						else if ((src & 0x6) == 0x6)
							cpustate->optable[inst] = SHFTNR;
						else if (src & 0x8)
							cpustate->optable[inst] = PRTNR;
						else
							cpustate->optable[inst] = TONR;
					}
				}
			}
		}
	}
}

static CPU_INIT( esrip )
{
	esrip_state *cpustate = get_safe_token(device);
	esrip_config* _config = (esrip_config*)device->static_config();

	memset(cpustate, 0, sizeof(*cpustate));

	/* Register configuration structure callbacks */
	cpustate->fdt_r = _config->fdt_r;
	cpustate->fdt_w = _config->fdt_w;
	cpustate->lbrm = (UINT8*)device->machine().root_device().memregion(_config->lbrm_prom)->base();
	cpustate->status_in = _config->status_in;
	cpustate->draw = _config->draw;

	/* Allocate image pointer table RAM */
	cpustate->ipt_ram = auto_alloc_array(device->machine(), UINT16, IPT_RAM_SIZE/2);

	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	/* Create the instruction decode lookup table */
	cpustate->optable = auto_alloc_array(device->machine(), UINT8, 65536);
	make_ops(cpustate);

	/* Register stuff for state saving */
	device->save_item(NAME(cpustate->acc));
	device->save_item(NAME(cpustate->ram));
	device->save_item(NAME(cpustate->d_latch));
	device->save_item(NAME(cpustate->i_latch));
	device->save_item(NAME(cpustate->result));
	device->save_item(NAME(cpustate->new_status));
	device->save_item(NAME(cpustate->status));
	device->save_item(NAME(cpustate->inst));
	device->save_item(NAME(cpustate->immflag));
	device->save_item(NAME(cpustate->ct));
	device->save_item(NAME(cpustate->t));
	device->save_item(NAME(cpustate->l1));
	device->save_item(NAME(cpustate->l2));
	device->save_item(NAME(cpustate->l3));
	device->save_item(NAME(cpustate->l4));
	device->save_item(NAME(cpustate->l5));
	device->save_item(NAME(cpustate->l6));
	device->save_item(NAME(cpustate->l7));
	device->save_item(NAME(cpustate->pl1));
	device->save_item(NAME(cpustate->pl2));
	device->save_item(NAME(cpustate->pl3));
	device->save_item(NAME(cpustate->pl4));
	device->save_item(NAME(cpustate->pl5));
	device->save_item(NAME(cpustate->pl6));
	device->save_item(NAME(cpustate->pl7));
	device->save_item(NAME(cpustate->pc));
	device->save_item(NAME(cpustate->status_out));
	device->save_item(NAME(cpustate->x_scale));
	device->save_item(NAME(cpustate->y_scale));
	device->save_item(NAME(cpustate->img_bank));
	device->save_item(NAME(cpustate->line_latch));
	device->save_item(NAME(cpustate->fig_latch));
	device->save_item(NAME(cpustate->attr_latch));
	device->save_item(NAME(cpustate->adl_latch));
	device->save_item(NAME(cpustate->adr_latch));
	device->save_item(NAME(cpustate->iaddr_latch));
	device->save_item(NAME(cpustate->c_latch));
	device->save_item(NAME(cpustate->fdt_cnt));
	device->save_item(NAME(cpustate->ipt_cnt));
	device->save_item(NAME(cpustate->fig));
	device->save_item(NAME(cpustate->fig_cycles));
	device->save_pointer(NAME(cpustate->ipt_ram), IPT_RAM_SIZE / sizeof(UINT16));
}


static CPU_RESET( esrip )
{
	esrip_state *cpustate = get_safe_token(device);

	cpustate->pc = 0;

	cpustate->pl1 = 0xff;
	cpustate->pl2 = 0xff;
	cpustate->pl3 = 0xff;
	cpustate->pl4 = 0xff;
	cpustate->pl5 = 0xff;
	cpustate->pl6 = 0xff;
	cpustate->pl7 = 0xff;

	cpustate->l1 = 0xff;
	cpustate->l2 = 0xff;
	cpustate->l3 = 0xff;
	cpustate->l4 = 0xff;
	cpustate->l5 = 0xff;
	cpustate->l6 = 0xff;
	cpustate->l7 = 0xff;

	cpustate->status_out = 0;
	cpustate->immflag = 0;
}


static CPU_EXIT( esrip )
{

}


/***************************************************************************
    PRIVATE FUNCTIONS
***************************************************************************/

static int get_hblank(running_machine &machine)
{
	return machine.primary_screen->hblank();
}

/* Return the state of the LBRM line (Y-scaling related) */
static int get_lbrm(esrip_state *cpustate)
{
	int addr = ((cpustate->y_scale & 0x3f) << 3) | ((cpustate->line_latch >> 3) & 7);
	int sel = (cpustate->line_latch & 7);

	UINT8 val = cpustate->lbrm[addr];

	return (val >> sel) & 1;
}

INLINE int check_jmp(esrip_state *cpustate, UINT8 jmp_ctrl)
{
	int ret = 0;

	if (~jmp_ctrl & 0x10)
	{
		switch (jmp_ctrl & 7)
		{
			/* CT */      case 0: ret = cpustate->ct;         break;
			/* T1 */      case 4: ret = BIT(cpustate->t, 0);  break;
			/* T2 */      case 2: ret = BIT(cpustate->t, 1);  break;
			/* T3 */      case 6: ret = BIT(cpustate->t, 2);  break;
			/* T4 */      case 1: ret = BIT(cpustate->t, 3);  break;
			/* /LBRM */   case 5: ret = !get_lbrm(cpustate);  break;
			/* /HBLANK */ case 3: ret = !get_hblank(cpustate->device->machine()); break;
			/* JMP */     case 7: ret = 0;                    break;
		}

		ret ^= 1;
	}
	else if (~jmp_ctrl & 0x08)
	{
		switch (jmp_ctrl & 7)
		{
			/* CT */      case 0: ret = cpustate->ct;        break;
			/* T1 */      case 4: ret = BIT(cpustate->t, 0); break;
			/* T2 */      case 2: ret = BIT(cpustate->t, 1); break;
			/* T3 */      case 6: ret = BIT(cpustate->t, 2); break;
			/* T4 */      case 1: ret = BIT(cpustate->t, 3); break;
			/* /LBRM */   case 5: ret = !get_lbrm(cpustate); break;
			/* /FIG */    case 3: ret = !cpustate->fig;      break;
			/* JMP */     case 7: ret = 1;                   break;
		}
	}
	else
		assert(!"RIP: Invalid jump control");

	return ret;
}


INLINE void calc_z_flag(esrip_state *cpustate, UINT16 res)
{
	cpustate->new_status &= ~Z_FLAG;
	cpustate->new_status |= (res == 0);
}

INLINE void calc_c_flag_add(esrip_state *cpustate, UINT16 a, UINT16 b)
{
	cpustate->new_status &= ~C_FLAG;
	cpustate->new_status |= ((UINT16)(b) > (UINT16)(~(a))) ? 2 : 0;
}

INLINE void calc_c_flag_sub(esrip_state *cpustate, UINT16 a, UINT16 b)
{
	cpustate->new_status &= ~C_FLAG;
	cpustate->new_status |= ((UINT16)(b) <= (UINT16)(a)) ? 2 : 0;
}

INLINE void calc_n_flag(esrip_state *cpustate, UINT16 res)
{
	cpustate->new_status &= ~N_FLAG;
	cpustate->new_status |= (res & 0x8000) ? 4 : 0;
}

INLINE void calc_v_flag_add(esrip_state *cpustate, UINT16 a, UINT16 b, UINT32 r)
{
	cpustate->new_status &= ~V_FLAG;
	cpustate->new_status |= ((a ^ r) & (b ^ r) & 0x8000) ? 8 : 0;
}

INLINE void calc_v_flag_sub(esrip_state *cpustate, UINT16 a, UINT16 b, UINT32 r)
{
	cpustate->new_status &= ~V_FLAG;
	cpustate->new_status |= ((a ^ b) & (r ^ b) & 0x8000) ? 8 : 0;
}


/***************************************************************************
    INSTRUCTIONS
***************************************************************************/

enum
{
	ACC,
	Y_BUS,
	STATUS,
	RAM,
};

/*************************************
 *
 *  Single operand
 *
 *************************************/
enum
{
	MOVE = 0xc,
	COMP = 0xd,
	INC  = 0xe,
	NEG  = 0xf
};

enum
{
	SORA  = 0x0,
	SORY  = 0x2,
	SORS  = 0x3,
	SOAR  = 0x4,
	SODR  = 0x6,
	SOIR  = 0x7,
	SOZR  = 0x8,
	SOZER = 0x9,
	SOSER = 0xa,
	SORR  = 0xb
};

static UINT16 sor_op(esrip_state *cpustate, UINT16 r, UINT16 opcode)
{
	UINT32 res = 0;

	switch (opcode)
	{
		case MOVE:
		{
			res = r;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case COMP:
		{
			res = r ^ 0xffff;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case INC:
		{
			res = r + 1;
			calc_v_flag_add(cpustate, r, 1, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_add(cpustate, r, 1);
			calc_z_flag(cpustate, res);
			break;
		}
		case NEG:
		{
			res = (r ^ 0xffff) + 1;
			calc_v_flag_sub(cpustate, 0, r, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_sub(cpustate, 0, r);
			calc_z_flag(cpustate, res);
			break;
		}
		default: assert(0);
	}

	return res & 0xffff;
}

static void sor(esrip_state *cpustate, UINT16 inst)
{
	UINT16	r = 0;
	UINT16	dst = 0;
	UINT16	res = 0;

	if (BYTE_MODE)
	{
		printf("Byte Mode! %.4x\n", inst);
		UNHANDLED;
	}

	switch ((inst >> 5) & 0xf)
	{
		case SORA: r = cpustate->ram[RAM_ADDR];		dst = ACC;		break;
		case SORY: r = cpustate->ram[RAM_ADDR];		dst = Y_BUS;	break;
		case SORS: r = cpustate->ram[RAM_ADDR];		dst = STATUS;	break;
		case SOAR: r = cpustate->acc;				dst = RAM;		break;
		case SODR: r = cpustate->d_latch;			dst = RAM;		break;
		case SOIR:
		{
			if (cpustate->immflag == 0)		// Macrofiy this?
			{
				cpustate->i_latch = inst;
				cpustate->immflag = 1;
				return;
			}
			else
			{
				r = cpustate->inst;
				dst = RAM;
				cpustate->immflag = 0;
			}
			break;
		}
		case SOZR: r = 0;						dst = RAM;		break;
		case SORR: r = cpustate->ram[RAM_ADDR];	dst = RAM;		break;
		default: UNHANDLED;
	}

	/* Operation */
	res = sor_op(cpustate, r, (inst >> 9) & 0xf);

	switch (dst)
	{
		case Y_BUS: break;
		case ACC: cpustate->acc = res; break;
		case RAM: cpustate->ram[RAM_ADDR] = res; break;
		default: UNHANDLED;
	}

	cpustate->result = res;
}

enum
{
	SOA  = 0x4,
	SOD  = 0x6,
	SOI  = 0x7,
	SOZ  = 0x8,
	SOZE = 0x9,
	SOSE = 0xa,
};

enum
{
	NRY  = 0,
	NRA  = 1,
	NRS  = 4,
	NRAS = 5,
};

static void sonr(esrip_state *cpustate, UINT16 inst)
{
	UINT16	r = 0;
	UINT16	res = 0;

	switch ((inst >> 5) & 0xf)
	{
		case SOA:	r = cpustate->acc;		break;
		case SOD:	r = cpustate->d_latch;	break;
		case SOI:
		{
			if (cpustate->immflag == 0)
			{
				cpustate->i_latch = inst;
				cpustate->immflag = 1;
				return;
			}
			else
			{
				r = cpustate->inst;
				cpustate->immflag = 0;
			}
			break;
		}
		case SOZ:	r = 0; break;
		default: INVALID;
	}

	/* Operation */
	res = sor_op(cpustate, r, (inst >> 9) & 0xf);

	/* Destination */
	switch (inst & 0x1f)
	{
		case NRY: break;
		case NRA: cpustate->acc = res; break;
		default: UNHANDLED;
	}

	cpustate->result = res;
}

/*************************************
 *
 *  Two operand
 *
 *************************************/

enum
{
	SUBR  = 0x0,
	SUBRC = 0x1,
	SUBS  = 0x2,
	SUBSC = 0x3,
	ADD   = 0x4,
	ADDC  = 0x5,
	AND   = 0x6,
	NAND  = 0x7,
	EXOR  = 0x8,
	NOR   = 0x9,
	OR    = 0xa,
	EXNOR = 0xb
};

static UINT16 tor_op(esrip_state *cpustate, UINT16 r, UINT16 s, int opcode)
{
	UINT32 res = 0;

	switch (opcode)
	{
		case SUBR:
		{
			res = s - r;
			calc_v_flag_sub(cpustate, s, r, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_sub(cpustate, s, r);
			calc_z_flag(cpustate, res);
			break;
		}
		case SUBRC: assert(0); break;
		case SUBS:
		{
			res = r - s;
			calc_v_flag_sub(cpustate, r, s, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_sub(cpustate, r, s);
			calc_z_flag(cpustate, res);
			break;
		}
		case SUBSC: assert(0); break;
		case ADD:
		{
			res = r + s;
			calc_v_flag_add(cpustate, r, s, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_add(cpustate, r, s);
			calc_z_flag(cpustate, res);
			break;
		}
		case ADDC:
		{
			// TODO TODO CHECK ME ETC
			res = r + s + ((cpustate->status >> 1) & 1);
			calc_v_flag_add(cpustate, r, s, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_add(cpustate, r, s);
			calc_z_flag(cpustate, res);
			break;
		}
		case AND:
		{
			res = r & s;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case NAND:
		{
			res = (r & s) ^ 0xffff;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case EXOR:
		{
			res = r ^ s;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case NOR:
		{
			res = (r | s) ^ 0xffff;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case OR:
		{
			res = r | s;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case EXNOR:
		{
			res = (r ^ s) ^ 0xffff;
			CLEAR_FLAGS(V_FLAG | N_FLAG | C_FLAG);
			calc_z_flag(cpustate, res);
			break;
		}
		default: assert(0);
	}

	return res & 0xffff;
}

static void tor1(esrip_state *cpustate, UINT16 inst)
{
	UINT16 r = 0;
	UINT16 s = 0;
	UINT16 dst = 0;
	UINT16	res = 0;

	enum
	{
		TORAA = 0x0,
		TORIA = 0x2,
		TODRA = 0x3,
		TORAY = 0x8,
		TORIY = 0xa,
		TODRY = 0xb,
		TORAR = 0xc,
		TORIR = 0xe,
		TODRR = 0xf,
	};

	switch (SRC)
	{
		case TORAA: r = cpustate->ram[RAM_ADDR];	s = cpustate->acc;	dst = ACC;	break;
		case TORIA:
		{
			if (cpustate->immflag == 0)
			{
				cpustate->i_latch = inst;
				cpustate->immflag = 1;
				return;
			}
			else
			{
				r = cpustate->ram[RAM_ADDR];
				s = cpustate->inst;
				dst = ACC;
				cpustate->immflag = 0;
			}
			break;
		}
		case TODRA: r = cpustate->d_latch;		s = cpustate->ram[RAM_ADDR];	dst = ACC;	break;
		case TORAY: r = cpustate->ram[RAM_ADDR];	s = cpustate->acc;			dst = Y_BUS;break;
		case TORIY:
		{
			if (cpustate->immflag == 0)
			{
				cpustate->i_latch = inst;
				cpustate->immflag = 1;
				return;
			}
			else
			{
				r = cpustate->ram[RAM_ADDR];
				s = cpustate->inst;
				dst = Y_BUS;
				cpustate->immflag = 0;
			}
			break;
		}
		case TODRY: r = cpustate->d_latch;		s = cpustate->ram[RAM_ADDR];	dst = Y_BUS;break;
		case TORAR: r = cpustate->ram[RAM_ADDR];	s = cpustate->acc;			dst = RAM;	break;
		case TORIR:
		{
			if (cpustate->immflag == 0)
			{
				cpustate->i_latch = inst;
				cpustate->immflag = 1;
				return;
			}
			else
			{
				r = cpustate->ram[RAM_ADDR];
				s = cpustate->inst;
				dst = RAM;
				cpustate->immflag = 0;
			}
			break;
		}
		case TODRR: r = cpustate->d_latch;		s = cpustate->ram[RAM_ADDR];	dst = RAM;	break;
		default: INVALID;
	}

	/* Operation */
	res = tor_op(cpustate, r, s, (inst >> 5) & 0xf);

	/* Destination */
	switch (dst)
	{
		case ACC:	cpustate->acc = res; break;
		case Y_BUS:	break;
		case RAM:	cpustate->ram[RAM_ADDR] = res; break;
		default:	INVALID;
	}

	cpustate->result = res;
}

static void tor2(esrip_state *cpustate, UINT16 inst)
{
	UINT16 r = 0;
	UINT16 s = 0;
	UINT32 res = 0;

	enum
	{
		TODAR = 0x1,
		TOAIR = 0x2,
		TODIR = 0x5,
	};

	switch (SRC)
	{
		case TODAR: r = cpustate->d_latch;	s = cpustate->acc;	break;
		case TOAIR:
		{
			if (cpustate->immflag == 0)
			{
				cpustate->i_latch = inst;
				cpustate->immflag = 1;
				return;
			}
			else
			{
				r = cpustate->acc;
				s = cpustate->inst;
				cpustate->immflag = 0;
			}
			break;
		}
		case TODIR:
		{
			if (cpustate->immflag == 0)
			{
				cpustate->i_latch = inst;
				cpustate->immflag = 1;
				return;
			}
			else
			{
				r = cpustate->d_latch;
				s = cpustate->inst;
				cpustate->immflag = 0;
			}
			break;
		}
		default: INVALID;
	}

	/* Operation */
	res = tor_op(cpustate, r, s, (inst >> 5) & 0xf);

	/* Destination is always RAM */
	cpustate->ram[RAM_ADDR] = res;

	cpustate->result = res;
}

static void tonr(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		TODA  = 0x1,
		TOAI  = 0x2,
		TODI  = 0x5
	};

	enum
	{
		NRY = 0,
		NRA = 1,
		NRS = 4,
		NRAS = 5,
	};

	UINT16 r = 0;
	UINT16 s = 0;
	UINT16	res = 0;

	switch (SRC)
	{
		case TODA:
		{
			r = cpustate->d_latch;
			s = cpustate->acc;
			break;
		}
		case TOAI:
		{
			break;
		}
		case TODI:
		{
			if (cpustate->immflag == 0)
			{
				cpustate->i_latch = inst;
				cpustate->immflag = 1;
				return;
			}
			else
			{
				r = cpustate->d_latch;
				s = cpustate->inst;
				cpustate->immflag = 0;
			}
			break;
		}
		default: INVALID;
	}

	/* Operation */
	res = tor_op(cpustate, r, s, (inst >> 5) & 0xf);

	/* Destination */
	switch (DST)
	{
		case NRY:
			break;
		case NRA:
			cpustate->acc = res;
			break;
		case NRS:
			UNHANDLED;
			break;
		case NRAS:
			UNHANDLED;
			break;
		default:
			INVALID;
	}
	cpustate->result = res;
}

/*************************************
 *
 *  Bit operation
 *
 *************************************/

static void bonr(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		TSTNA  = 0x00,
		RSTNA  = 0x01,
		SETNA  = 0x02,
		A2NA   = 0x04,
		S2NA   = 0x05,
		LD2NA  = 0x06,
		LDC2NA = 0x07,
		TSTND  = 0x10,
		RSTND  = 0x11,
		SETND  = 0x12,
		A2NDY  = 0x14,
		S2NDY  = 0x15,
		LD2NY  = 0x16,
		LDC2NY = 0x17,
	};

	UINT16	res = 0;

	switch (inst & 0x1f)
	{
		case TSTNA:
		{
			res = cpustate->acc & (1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case RSTNA:
		{
			res = cpustate->acc & ~(1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			cpustate->acc = res;
			break;
		}
		case SETNA:
		{
			res = cpustate->acc | (1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(cpustate, res);
			cpustate->acc = res;
			break;
		}
		case A2NA:
		{
			UINT16 r = cpustate->acc;
			UINT16 s = 1 << N;
			res = r + s;
			calc_z_flag(cpustate, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_add(cpustate, r, s);
			calc_v_flag_add(cpustate, r, s, res);
			cpustate->acc = res;
			break;
		}
		case S2NA:
		{
			UINT16 r = cpustate->acc;
			UINT16 s = 1 << N;
			res = r - s;
			calc_z_flag(cpustate, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_sub(cpustate, r, s);
			calc_v_flag_sub(cpustate, r, s, res);
			cpustate->acc = res;
			break;
		}

		case TSTND:
		{
			res = cpustate->d_latch & (1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}

		case SETND:
		{
			UINT16 r = cpustate->d_latch;
			res = r | (1 << N);
			cpustate->d_latch = res;

			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(cpustate, res);
			break;
		}
		case LD2NY:
		{
			res = (1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(cpustate, res);
			break;
		}
		case LDC2NY:
		{
			res = (1 << N) ^ 0xffff;
			CLEAR_FLAGS(Z_FLAG | C_FLAG | V_FLAG);
			calc_n_flag(cpustate, res);
			break;
		}

		case A2NDY:
		{
			UINT16 r = cpustate->d_latch;
			UINT16 s = 1 << N;
			res = r + s;

			calc_z_flag(cpustate, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_add(cpustate, r, s);
			calc_v_flag_add(cpustate, r, s, res);
			break;
		}

		default:
			UNHANDLED;
	}

	cpustate->result = res;
}

static void bor1(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		SETNR  = 0xd,
		RSTNR  = 0xe,
		TSTNR  = 0xf,
	};

	UINT16	res = 0;

	switch ((inst >> 5) & 0xf)
	{
		case SETNR:
		{
			res = cpustate->ram[RAM_ADDR] | (1 << N);
			cpustate->ram[RAM_ADDR] = res;
			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(cpustate, res);
			break;
		}
		case RSTNR:
		{
			res = cpustate->ram[RAM_ADDR] & ~(1 << N);
			cpustate->ram[RAM_ADDR] = res;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case TSTNR:
		{
			res = cpustate->ram[RAM_ADDR] & (1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		default: INVALID;
	}

	cpustate->result = res;
}



static void bor2(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		LD2NR  = 0xc,
		LDC2NR = 0xd,
		A2NR   = 0xe,
		S2NR   = 0xf,
	};

	UINT32 res = 0;

	switch ((inst >> 5) & 0xf)
	{
		case LD2NR:
		{
			res = 1 << N;
			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(cpustate, res);
			break;
		}
		case LDC2NR:
		{
			res = (1 << N) ^ 0xffff;
			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(cpustate, res);
			break;
		}
		case A2NR:
		{
			UINT16 r = cpustate->ram[RAM_ADDR];
			UINT16 s = 1 << N;

			res = r + s;
			calc_v_flag_add(cpustate, r, s, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_add(cpustate, r, s);
			calc_z_flag(cpustate, res);
			break;
		}
		case S2NR:
		{
			UINT16 r = cpustate->ram[RAM_ADDR];
			UINT16 s = 1 << N;

			res = r - s;
			calc_v_flag_sub(cpustate, r, s, res);
			calc_n_flag(cpustate, res);
			calc_c_flag_sub(cpustate, r, s);
			calc_z_flag(cpustate, res);
			break;
		}
		default: INVALID;
	}

	/* Destination is RAM */
	cpustate->ram[RAM_ADDR] = res;
	cpustate->result = res;
}

/*************************************
 *
 *  Rotate
 *
 *************************************/

/* TODO Combine these */
static void rotr1(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		RTRA = 0xc,
		RTRY = 0xd,
		RTRR = 0xf,
	};

	UINT16	u = 0;
	UINT16	dst = 0;
	UINT16	res = 0;
	int		n = N;

	switch ((inst >> 5) & 0xf)
	{
		case RTRA: u = cpustate->ram[RAM_ADDR];	dst = ACC;		break;
		case RTRY: u = cpustate->ram[RAM_ADDR];	dst = Y_BUS;	break;
		case RTRR: u = cpustate->ram[RAM_ADDR];	dst = RAM;		break;
		default: INVALID;
	}

	res = (u << n) | (u >> (16 - n));
	CLEAR_FLAGS(V_FLAG | C_FLAG);
	calc_n_flag(cpustate, res);
	calc_z_flag(cpustate, res);

	switch (dst)
	{
		case ACC: cpustate->acc = res; break;
		case RAM: cpustate->ram[RAM_ADDR] = res; break;
	}

	cpustate->result = res;
}

static void rotr2(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		RTAR = 0,
		RTDR = 1,
	};

	UINT16	u = 0;
	UINT16	res = 0;

	switch ((inst >> 5) & 0xf)
	{
		case RTAR: u = cpustate->acc;		break;
		case RTDR: u = cpustate->d_latch;	break;
		default: INVALID;
	}

	res = (u << N) | (u >> (16 - N));
	CLEAR_FLAGS(V_FLAG | C_FLAG);
	calc_n_flag(cpustate, res);
	calc_z_flag(cpustate, res);
	cpustate->ram[RAM_ADDR] = res;

	cpustate->result = res;
}

static void rotnr(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		RTDY = 0x18,
		RTDA = 0x19,
		RTAY = 0x1c,
		RTAA = 0x1d,
	};

	UINT16	u = 0;
	UINT16	res = 0;
	UINT16	dst = 0;

	switch (inst & 0x1f)
	{
		case RTDY: u = cpustate->d_latch;	dst = Y_BUS;	break;
		case RTDA: u = cpustate->d_latch;	dst = ACC;		break;
		case RTAY: u = cpustate->acc;		dst = Y_BUS;	break;
		case RTAA: u = cpustate->acc;		dst = ACC;		break;
		default: INVALID;
	}

	res = (u << N) | (u >> (16 - N));
	CLEAR_FLAGS(V_FLAG | C_FLAG);
	calc_n_flag(cpustate, res);
	calc_z_flag(cpustate, res);

	switch (dst)
	{
		case Y_BUS: break;
		case ACC: cpustate->acc = res; break;
		case RAM: cpustate->ram[RAM_ADDR] = res; break;
		default: UNHANDLED;
	}

	cpustate->result = res;
}

/*************************************
 *
 *  Rotate and compare
 *
 *************************************/

static void rotc(esrip_state *cpustate, UINT16 inst)
{
	UNHANDLED;
}

/*************************************
 *
 *  Rotate and merge
 *
 *************************************/

static void rotm(esrip_state *cpustate, UINT16 inst)
{
	UNHANDLED;
}

/*************************************
 *
 *  Prioritize
 *
 *************************************/

static void prt(esrip_state *cpustate, UINT16 inst)
{
	UNHANDLED;
}

static void prtnr(esrip_state *cpustate, UINT16 inst)
{
	UNHANDLED;
}


/*************************************
 *
 *  CRC
 *
 *************************************/

static void crcf(esrip_state *cpustate, UINT16 inst)
{
	UNHANDLED;
}

static void crcr(esrip_state *cpustate, UINT16 inst)
{
	UNHANDLED;
}

/*************************************
 *
 *  Single bit shift
 *
 *************************************/

enum
{
	SHUPZ  = 0,
	SHUP1  = 1,
	SHUPL  = 2,
	SHDNZ  = 4,
	SHDN1  = 5,
	SHDNL  = 6,
	SHDNC  = 7,
	SHDNOV = 8,
};

#define	SET_LINK_flag(cpustate, x)	(cpustate->new_status &= ~L_FLAG); \
							(cpustate->new_status |= x ? L_FLAG : 0)

static UINT16 shift_op(esrip_state *cpustate, UINT16 u, int opcode)
{
	UINT32 res = 0;

	switch (opcode)
	{
		case SHUPZ:
		{
			res = (u << 1);
			SET_LINK_flag(cpustate, u & 0x8000);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case SHUP1:
		{
			res = (u << 1) | 1;
			SET_LINK_flag(cpustate, u & 0x8000);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}
		case SHUPL:
		{
			res = (u << 1) | ((cpustate->status & L_FLAG) ? 1 : 0);
			SET_LINK_flag(cpustate, u & 0x8000);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(cpustate, res);
			calc_z_flag(cpustate, res);
			break;
		}

		case SHDNZ:
		case SHDN1:
		case SHDNL:
		case SHDNC:
		case SHDNOV:
		default: assert(0);
	}

	return res;
}

static void shftr(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		SHRR = 6,
		SHDR = 7,
	};

	UINT16	u = 0;
	UINT16	res = 0;

	switch ((inst >> 9) & 0xf)
	{
		case SHRR: u = cpustate->ram[RAM_ADDR];	break;
		case SHDR: u = cpustate->d_latch;			break;
		default: INVALID;
	}

	res = shift_op(cpustate, u, (inst >> 5) & 0xf);

	/* Destination is always RAM */
	cpustate->ram[RAM_ADDR] = res;
	cpustate->result = res;
}

static void shftnr(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		SHA = 6,
		SHD = 7,
	};

	UINT16	u = 0;
	UINT16	res = 0;

	switch ((inst >> 9) & 0xf)
	{
		case SHA: u = cpustate->acc;			break;
		case SHD: u = cpustate->d_latch;		break;
		default: INVALID;
	}

	res = shift_op(cpustate, u, (inst >> 5) & 0xf);

	switch (DST)
	{
		case NRY:	break;
		case NRA:	cpustate->acc = res; break;
		default:	INVALID;
	}
	cpustate->result = res;
}


/*************************************
 *
 *  Status
 *
 *************************************/

static void svstr(esrip_state *cpustate, UINT16 inst)
{
	UNHANDLED;
}

static void rstst(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		RONCZ = 0x3,
		RL    = 0x5,
		RF1   = 0x6,
		RF2   = 0x9,
		RF3   = 0xa,
	};

	switch (inst & 0x1f)
	{
		case RONCZ: CLEAR_FLAGS(V_FLAG | N_FLAG | C_FLAG | Z_FLAG); break;
		case RL:	CLEAR_FLAGS(L_FLAG); break;
		case RF1:	CLEAR_FLAGS(FLAG_1); break;
		case RF2:	CLEAR_FLAGS(FLAG_2); break;
		case RF3:	CLEAR_FLAGS(FLAG_3); break;
	}

	cpustate->result = 0;
}

static void setst(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		SONCZ = 0x3,
		SL    = 0x5,
		SF1   = 0x6,
		SF2   = 0x9,
		SF3   = 0xa,
	};

	switch (inst & 0x1f)
	{
		case SONCZ: SET_FLAGS(V_FLAG | N_FLAG | C_FLAG | Z_FLAG); break;
		case SL:	SET_FLAGS(L_FLAG); break;
		case SF1:	SET_FLAGS(FLAG_1); break;
		case SF2:	SET_FLAGS(FLAG_2); break;
		case SF3:	SET_FLAGS(FLAG_3); break;
	}

	cpustate->result = 0xffff;
}

static void test(esrip_state *cpustate, UINT16 inst)
{
	enum
	{
		TNOZ = 0x00,
		TNO  = 0x02,
		TZ   = 0x04,
		TOVR = 0x06,
		TLOW = 0x08,
		TC   = 0x0a,
		TZC  = 0x0c,
		TN   = 0x0e,
		TL   = 0x10,
		TF1  = 0x12,
		TF2  = 0x14,
		TF3  = 0x16
	};

	UINT32 res = 0;

	switch (inst & 0x1f)
	{
		case TNOZ: UNHANDLED; break;
		case TNO:  UNHANDLED; break;
		case TZ:   res = cpustate->status & (Z_FLAG); break;
		case TOVR: res = cpustate->status & (V_FLAG); break;
		case TLOW: UNHANDLED; break;
		case TC:   res = cpustate->status & (C_FLAG); break;
		case TZC:  UNHANDLED; break;
		case TN:   res = cpustate->status & (N_FLAG); break;
		case TL:   res = cpustate->status & (L_FLAG); break;
		case TF1:  res = cpustate->status & (FLAG_1); break;
		case TF2:  res = cpustate->status & (FLAG_2); break;
		case TF3:  res = cpustate->status & (FLAG_3); break;
		default:   INVALID;
	}

	cpustate->ct = res ? 1 : 0;
}


/*************************************
 *
 *  No operation
 *
 *************************************/

static void nop(esrip_state *cpustate, UINT16 inst)
{
	cpustate->result = 0xff;	// Undefined
}

static void (*const operations[24])(esrip_state *cpustate, UINT16 inst) =
{
	rotr1, tor1, rotr2, rotc, rotm, bor2, crcf, crcr,
	svstr, prt, sor, tor2, shftr, test, nop, setst, rstst,
	rotnr, bonr, bor1, sonr, shftnr, prtnr, tonr
};

INLINE void am29116_execute(esrip_state *cpustate, UINT16 inst, int _sre)
{
	/* Status register shadow */
	cpustate->new_status = cpustate->status;

	/* Required for immediate source instructions */
	cpustate->inst = inst;

	if (cpustate->immflag == 1)
		inst = cpustate->i_latch;

	(*operations[cpustate->optable[inst]])(cpustate, inst);

	if (!_sre)
	{
		cpustate->status = cpustate->new_status;
		cpustate->t = cpustate->status;
	}
}


static CPU_EXECUTE( esrip )
{
	esrip_state *cpustate = get_safe_token(device);
	int calldebugger = (device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0;
	UINT8 status;

	/* I think we can get away with placing this outside of the loop */
	status = cpustate->status_in(device->machine());

	/* Core execution loop */
	do
	{
		UINT64	inst;
		UINT8	next_pc;
		UINT16	x_bus = 0;
		UINT16	ipt_bus = 0;
		UINT16	y_bus = 0;

		int yoe = _BIT(cpustate->l5, 1);
		int bl46 = BIT(cpustate->l4, 6);
		int bl44 = BIT(cpustate->l4, 4);

		UINT32 in_h;
		UINT32 in_l;

		if (cpustate->fig_cycles)
		{
			if (--cpustate->fig_cycles == 0)
				cpustate->fig = 0;
		}

		/* /OEY = 1 : Y-bus is high imped */
		if (yoe)
		{
			/* Status In */
			if (!_BIT(cpustate->l2, 0))
				y_bus = status | (!cpustate->fig << 3);

			/* FDT RAM: /Enable, Direction and /RAM OE */
			else if (!bl44 && !_BIT(cpustate->l2, 3) && bl46)
				y_bus = cpustate->fdt_r(device, cpustate->fdt_cnt, 0);

			/* IPT RAM: /Enable and /READ */
			else if (!_BIT(cpustate->l2, 6) && !_BIT(cpustate->l4, 5))
				y_bus = cpustate->ipt_ram[cpustate->ipt_cnt];

			/* DLE  - latch the value on the Y-BUS into the data latch */
			if (_BIT(cpustate->l5, 0))
				cpustate->d_latch = y_bus;

			/* Now execute the AM29116 instruction */
			am29116_execute(cpustate, (cpustate->l7 << 8) | cpustate->l6, BIT(cpustate->l5, 2));
		}
		else
		{
			am29116_execute(cpustate, (cpustate->l7 << 8) | cpustate->l6, BIT(cpustate->l5, 2));

			y_bus = cpustate->result;

			if (BIT(cpustate->l5, 0))
				cpustate->d_latch = y_bus;
		}

		/* Determine what value is on the X-Bus */

		/* FDT RAM */
		if (!bl44)
			x_bus = cpustate->fdt_r(device, cpustate->fdt_cnt, 0);

		/* Buffer is enabled - write direction */
		else if (!BIT(cpustate->l2, 3) && !bl46)
		{
			if (!yoe)
				x_bus = y_bus;
			else if ( !BIT(cpustate->l2, 6) && !BIT(cpustate->l4, 5) )
				x_bus = cpustate->ipt_ram[cpustate->ipt_cnt];
		}

		/* IPT BUS */
		if (!BIT(cpustate->l2, 6))
			ipt_bus = cpustate->ipt_ram[cpustate->ipt_cnt];
		else if (!BIT(cpustate->l4, 5))
		{
			if (!BIT(cpustate->l5, 1))
				ipt_bus = y_bus;
			else
				ipt_bus = x_bus;
		}


		/* Write FDT RAM: /Enable, Direction and WRITE */
		if (!BIT(cpustate->l2, 3) && !bl46 && !BIT(cpustate->l4, 3))
			cpustate->fdt_w(device, cpustate->fdt_cnt, x_bus, 0);

		/* Write IPT RAM: /Enable and /WR */
		if (!BIT(cpustate->l2, 7) && !BIT(cpustate->l4, 5))
			cpustate->ipt_ram[cpustate->ipt_cnt] = ipt_bus;


		if ((((cpustate->l5 >> 3) & 0x1f) & 0x18) != 0x18)
		{
			if ( check_jmp(cpustate, (cpustate->l5 >> 3) & 0x1f) )
				next_pc = cpustate->l1;
			else
				next_pc = cpustate->pc + 1;
		}
		else
			next_pc = cpustate->pc + 1;

		cpustate->pl1 = cpustate->l1;
		cpustate->pl2 = cpustate->l2;
		cpustate->pl3 = cpustate->l3;
		cpustate->pl4 = cpustate->l4;
		cpustate->pl5 = cpustate->l5;
		cpustate->pl6 = cpustate->l6;
		cpustate->pl7 = cpustate->l7;

		/* Latch instruction */
		inst = cpustate->direct->read_decrypted_qword(RIP_PC << 3);

		in_h = inst >> 32;
		in_l = inst & 0xffffffff;

		cpustate->l1 = (in_l >> 8);
		cpustate->l2 = (in_l >> 16);
		cpustate->l3 = (in_l >> 24);

		cpustate->l4 = (in_h >> 0);
		cpustate->l5 = (in_h >> 8);
		cpustate->l6 = (in_h >> 16);
		cpustate->l7 = (in_h >> 24);

		/* Colour latch */
		if (RISING_EDGE(cpustate->pl3, cpustate->l3, 0))
			cpustate->c_latch = (x_bus >> 12) & 0xf;

		/* Right pixel line buffer address */
		if (RISING_EDGE(cpustate->pl3, cpustate->l3, 1))
			cpustate->adr_latch = x_bus & 0xfff;

		/* Left pixel line buffer address */
		if (RISING_EDGE(cpustate->pl3, cpustate->l3, 2))
			cpustate->adl_latch = x_bus & 0xfff;

		/* FIGLD: Start the DMA */
		if (RISING_EDGE(cpustate->pl3, cpustate->l3, 3))
		{
			cpustate->attr_latch = x_bus;

			cpustate->fig = 1;
			cpustate->fig_cycles = cpustate->draw(device->machine(), cpustate->adl_latch, cpustate->adr_latch, cpustate->fig_latch, cpustate->attr_latch, cpustate->iaddr_latch, cpustate->c_latch, cpustate->x_scale, cpustate->img_bank);
		}

		/* X-scale */
		if (RISING_EDGE(cpustate->pl3, cpustate->l3, 4))
			cpustate->x_scale = x_bus >> 8;

		/* Y-scale and image bank */
		if (RISING_EDGE(cpustate->pl4, cpustate->l4, 2))
		{
			cpustate->y_scale = x_bus & 0xff;
			cpustate->img_bank = (y_bus >> 14) & 3;
		}

		/* Image ROM address */
		if (RISING_EDGE(cpustate->pl3, cpustate->l3, 5))
			cpustate->iaddr_latch = y_bus;

		/* IXLLD */
		if (RISING_EDGE(cpustate->pl3, cpustate->l3, 6))
		{
			cpustate->line_latch = ipt_bus >> 10;
			cpustate->fig_latch  = ipt_bus & 0x3ff;
		}

		/* Status write */
		if (RISING_EDGE(cpustate->pl3, cpustate->l3, 7))
			cpustate->status_out = y_bus & 0xff;

		/* FDT address counter */
		if (!BIT(cpustate->pl2, 1))
			cpustate->fdt_cnt = y_bus & 0xfff;
		else if (BIT(cpustate->pl2, 2))
			cpustate->fdt_cnt = (cpustate->fdt_cnt + 1) & 0xfff;

		/* Now we can alter the IPT address counter */
		if (!BIT(cpustate->pl2, 4))
			cpustate->ipt_cnt = y_bus & 0x1fff;
		else if (BIT(cpustate->pl2, 5))
			cpustate->ipt_cnt = (cpustate->ipt_cnt + 1) & 0x1fff;

		if (calldebugger)
			debugger_instruction_hook(device, RIP_PC);

		cpustate->pc = next_pc;

		cpustate->icount--;
	} while (cpustate->icount > 0);
}


/**************************************************************************
 * set_info
 **************************************************************************/

static CPU_SET_INFO( esrip )
{
	esrip_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ESRIP_PC:	cpustate->pc = info->i & 0xff;
												cpustate->status_out &= ~1;
												cpustate->status_out |= ((info->i >> 8) & 1);
												break;
	}
}

/**************************************************************************
 * get_info
 **************************************************************************/

CPU_GET_INFO( esrip )
{
	esrip_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(esrip_state);	break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;		break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;					break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 8;					break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;					break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;					break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;					break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 64;			break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 9;			break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -3;			break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;			break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;			break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;			break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;			break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;			break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;			break;

		case CPUINFO_INT_REGISTER:
		case CPUINFO_INT_PC:							info->i = RIP_PC;	break;
		case CPUINFO_INT_REGISTER + ESRIP_STATW:		info->i = cpustate->status_out; break;
		case CPUINFO_INT_REGISTER + ESRIP_FDTC:			info->i = cpustate->fdt_cnt; break;
		case CPUINFO_INT_REGISTER + ESRIP_IPTC:			info->i = cpustate->ipt_cnt; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(esrip);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(esrip);			break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(esrip);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(esrip);			break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(esrip);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(esrip);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Real Time Image Processor");		break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Entertainment Sciences");			break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Philip J Bennett"); break;

		case CPUINFO_STR_FLAGS:							sprintf(info->s, "%c%c%c%c%c%c%c%c%c",
																		cpustate->status & 0x80 ? '3' : '.',
																		cpustate->status & 0x40 ? '2' : '.',
																		cpustate->status & 0x20 ? '1' : '.',
																		cpustate->status & 0x10 ? 'L' : '.',
																		cpustate->status & 0x08 ? 'V' : '.',
																		cpustate->status & 0x04 ? 'N' : '.',
																		cpustate->status & 0x02 ? 'C' : '.',
																		cpustate->status & 0x01 ? 'Z' : '.',
																		get_hblank(device->machine()) ? 'H' : '.'); break;

		case CPUINFO_STR_REGISTER + ESRIP_PC:			sprintf(info->s, "PC: %04X", RIP_PC); break;
		case CPUINFO_STR_REGISTER + ESRIP_ACC:			sprintf(info->s, "ACC: %04X", cpustate->acc); break;
		case CPUINFO_STR_REGISTER + ESRIP_DLATCH:		sprintf(info->s, "DLATCH: %04X", cpustate->d_latch); break;
		case CPUINFO_STR_REGISTER + ESRIP_ILATCH:		sprintf(info->s, "ILATCH: %04X", cpustate->i_latch); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM00:		sprintf(info->s, "RAM[00]: %04X", cpustate->ram[0x00]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM01:		sprintf(info->s, "RAM[01]: %04X", cpustate->ram[0x01]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM02:		sprintf(info->s, "RAM[02]: %04X", cpustate->ram[0x02]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM03:		sprintf(info->s, "RAM[03]: %04X", cpustate->ram[0x03]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM04:		sprintf(info->s, "RAM[04]: %04X", cpustate->ram[0x04]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM05:		sprintf(info->s, "RAM[05]: %04X", cpustate->ram[0x05]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM06:		sprintf(info->s, "RAM[06]: %04X", cpustate->ram[0x06]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM07:		sprintf(info->s, "RAM[07]: %04X", cpustate->ram[0x07]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM08:		sprintf(info->s, "RAM[08]: %04X", cpustate->ram[0x08]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM09:		sprintf(info->s, "RAM[09]: %04X", cpustate->ram[0x09]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM0A:		sprintf(info->s, "RAM[0A]: %04X", cpustate->ram[0x0a]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM0B:		sprintf(info->s, "RAM[0B]: %04X", cpustate->ram[0x0b]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM0C:		sprintf(info->s, "RAM[0C]: %04X", cpustate->ram[0x0c]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM0D:		sprintf(info->s, "RAM[0D]: %04X", cpustate->ram[0x0d]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM0E:		sprintf(info->s, "RAM[0E]: %04X", cpustate->ram[0x0e]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM0F:		sprintf(info->s, "RAM[0F]: %04X", cpustate->ram[0x0f]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM10:		sprintf(info->s, "RAM[10]: %04X", cpustate->ram[0x10]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM11:		sprintf(info->s, "RAM[11]: %04X", cpustate->ram[0x11]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM12:		sprintf(info->s, "RAM[12]: %04X", cpustate->ram[0x12]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM13:		sprintf(info->s, "RAM[13]: %04X", cpustate->ram[0x13]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM14:		sprintf(info->s, "RAM[14]: %04X", cpustate->ram[0x14]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM15:		sprintf(info->s, "RAM[15]: %04X", cpustate->ram[0x15]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM16:		sprintf(info->s, "RAM[16]: %04X", cpustate->ram[0x16]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM17:		sprintf(info->s, "RAM[17]: %04X", cpustate->ram[0x17]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM18:		sprintf(info->s, "RAM[18]: %04X", cpustate->ram[0x18]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM19:		sprintf(info->s, "RAM[19]: %04X", cpustate->ram[0x19]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM1A:		sprintf(info->s, "RAM[1A]: %04X", cpustate->ram[0x1a]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM1B:		sprintf(info->s, "RAM[1B]: %04X", cpustate->ram[0x1b]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM1C:		sprintf(info->s, "RAM[1C]: %04X", cpustate->ram[0x1c]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM1D:		sprintf(info->s, "RAM[1D]: %04X", cpustate->ram[0x1d]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM1E:		sprintf(info->s, "RAM[1E]: %04X", cpustate->ram[0x1e]); break;
		case CPUINFO_STR_REGISTER + ESRIP_RAM1F:		sprintf(info->s, "RAM[1F]: %04X", cpustate->ram[0x1f]); break;
		case CPUINFO_STR_REGISTER + ESRIP_STATW:		sprintf(info->s, "STAT: %04X", cpustate->status_out); break;
		case CPUINFO_STR_REGISTER + ESRIP_FDTC:			sprintf(info->s, "FDTC: %04X", cpustate->fdt_cnt); break;
		case CPUINFO_STR_REGISTER + ESRIP_IPTC:			sprintf(info->s, "IPTC: %04X", cpustate->ipt_cnt); break;
		case CPUINFO_STR_REGISTER + ESRIP_XSCALE:		sprintf(info->s, "XSCL: %04X", cpustate->x_scale); break;
		case CPUINFO_STR_REGISTER + ESRIP_YSCALE:		sprintf(info->s, "YSCL: %04X", cpustate->y_scale); break;
		case CPUINFO_STR_REGISTER + ESRIP_BANK:			sprintf(info->s, "BANK: %04X", cpustate->img_bank); break;
		case CPUINFO_STR_REGISTER + ESRIP_LINE:			sprintf(info->s, "LINE: %04X", cpustate->line_latch); break;
		case CPUINFO_STR_REGISTER + ESRIP_FIG:			sprintf(info->s, "FIG: %04X", cpustate->fig_latch); break;
		case CPUINFO_STR_REGISTER + ESRIP_ATTR:			sprintf(info->s, "ATTR: %04X", cpustate->attr_latch); break;
		case CPUINFO_STR_REGISTER + ESRIP_ADRL:			sprintf(info->s, "ADRL: %04X", cpustate->adl_latch); break;
		case CPUINFO_STR_REGISTER + ESRIP_ADRR:			sprintf(info->s, "ADRR: %04X", cpustate->adr_latch); break;
		case CPUINFO_STR_REGISTER + ESRIP_COLR:			sprintf(info->s, "COLR: %04X", cpustate->c_latch); break;
		case CPUINFO_STR_REGISTER + ESRIP_IADDR:		sprintf(info->s, "IADR: %04X", cpustate->iaddr_latch); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(ESRIP, esrip);
