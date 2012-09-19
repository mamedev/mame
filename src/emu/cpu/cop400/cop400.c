/***************************************************************************

    cop400.c

    National Semiconductor COP400 Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Type        ROM     RAM     G       D       IN

    COP410      512x8   32x4                    none
    COP411      512x8   32x4    0-2     0-1     none
    COP401      none    32x4                    none
    COP413?
    COP414?
    COP415?
    COP405?

    COP420      1024x8  64x4
    COP421      1024x8  64x4                    none
    COP422      1024x8  64x4    2-3     2-3     none
    COP402      none    64x4

    COP444      2048x8  128x4
    COP445      2048x8  128x4                   none
    COP424      1024x8  64x4
    COP425      1024x8  64x4                    none
    COP426      1024x8  64x4    2-3     2-3
    COP404      none    none

    COP440      2048x8  160x4
    COP441      2048x8  160x4
    COP442      2048x8  160x4

****************************************************************************

    Prefix      Temperature Range

    COP4xx      0C ... 70C
    COP3xx      -40C ... +85C
    COP2xx      -55C ... +125C

***************************************************************************/

/*

    TODO:

    - remove LBIOps
    - remove InstLen
    - run interrupt test suite
    - run production test suite
    - run microbus test suite
    - when is the microbus int cleared?
    - opcode support for 2048x8 and 128x4/160x4 memory sizes
    - CKO sync input
    - save internal RAM when CKO is RAM power supply pin
    - COP413/COP414/COP415/COP405
    - COP404 opcode map switching, dual timer, microbus enable
    - COP440/COP441/COP442 (new registers: 2-bit N, 4-bit H, 8-bit R; some new opcodes, 2Kx8 ROM, 160x4 RAM)

*/

#include "emu.h"
#include "debugger.h"
#include "cop400.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* feature masks */
#define COP410_FEATURE	0x01
#define COP420_FEATURE	0x02
#define COP444_FEATURE	0x04
#define COP440_FEATURE	0x08

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct cop400_opcode_map;

struct cop400_state
{
	const cop400_interface *intf;

    address_space *program;
    direct_read_data *direct;
    address_space *data;
    address_space *io;

    UINT8 featuremask;

	/* registers */
	UINT16	pc;				/* 9/10/11-bit ROM address program counter */
	UINT16	prevpc;			/* previous value of program counter */
	UINT8	a;				/* 4-bit accumulator */
	UINT8	b;				/* 5/6/7-bit RAM address register */
	int		c;				/* 1-bit carry register */
	UINT8	n;				/* 2-bit stack pointer (COP440 only) */
	UINT8	en;				/* 4-bit enable register */
	UINT8	g;				/* 4-bit general purpose I/O port */
	UINT8	q;				/* 8-bit latch for L port */
	UINT16	sa, sb, sc;		/* subroutine save registers (not present in COP440) */
	UINT8	sio;			/* 4-bit shift register and counter */
	int		skl;			/* 1-bit latch for SK output */
	UINT8	h;				/* 4-bit general purpose I/O port (COP440 only) */
	UINT8	r;				/* 8-bit general purpose I/O port (COP440 only) */
	UINT8	flags;			// used for I/O only

	/* counter */
	UINT8	t;				/* 8-bit timer */
	int		skt_latch;		/* timer overflow latch */

	/* input/output ports */
	UINT8	g_mask;			/* G port mask */
	UINT8	d_mask;			/* D port mask */
	UINT8	in_mask;		/* IN port mask */
	UINT8	il;				/* IN latch */
	UINT8	in[4];			/* IN port shift register */
	UINT8	si;				/* serial input */

	/* skipping logic */
	int skip;				/* skip next instruction */
	int skip_lbi;			/* skip until next non-LBI instruction */
	int last_skip;			/* last value of skip */
	int halt;				/* halt mode */
	int idle;				/* idle mode */

	/* microbus */
	int microbus_int;		/* microbus interrupt */

	/* execution logic */
	int InstLen[256];		/* instruction length in bytes */
	int LBIops[256];
	int LBIops33[256];
	int icount;				/* instruction counter */

	const cop400_opcode_map *opcode_map;

	/* timers */
	emu_timer *serial_timer;
	emu_timer *counter_timer;
	emu_timer *inil_timer;
	emu_timer *microbus_timer;
};

typedef void (*cop400_opcode_func) (cop400_state *cpustate, UINT8 opcode);

struct cop400_opcode_map {
	unsigned cycles;
	cop400_opcode_func function;
};

/***************************************************************************
    MACROS
***************************************************************************/

#define ROM(a)			cpustate->direct->read_decrypted_byte(a)
#define RAM_R(a)		cpustate->data->read_byte(a)
#define RAM_W(a, v)		cpustate->data->write_byte(a, v)
#define IN(a)			cpustate->io->read_byte(a)
#define OUT(a, v)		cpustate->io->write_byte(a, v)

#define IN_G()			(IN(COP400_PORT_G) & cpustate->g_mask)
#define IN_L()			IN(COP400_PORT_L)
#define IN_SI()			BIT(IN(COP400_PORT_SIO), 0)
#define IN_CKO()		BIT(IN(COP400_PORT_CKO), 0)
#define IN_IN()			(cpustate->in_mask ? IN(COP400_PORT_IN) : 0)

#define OUT_G(v)		OUT(COP400_PORT_G, (v) & cpustate->g_mask)
#define OUT_L(v)		OUT(COP400_PORT_L, (v))
#define OUT_D(v)		OUT(COP400_PORT_D, (v) & cpustate->d_mask)
#define OUT_SK(v)		OUT(COP400_PORT_SK, (v))
#define OUT_SO(v)		OUT(COP400_PORT_SIO, (v))

#define PC				cpustate->pc
#define A				cpustate->a
#define B				cpustate->b
#define C				cpustate->c
#define G				cpustate->g
#define Q				cpustate->q
#define H				cpustate->h
#define R				cpustate->r
#define EN				cpustate->en
#define SA				cpustate->sa
#define SB				cpustate->sb
#define SC				cpustate->sc
#define SIO				cpustate->sio
#define SKL				cpustate->skl
#define T				cpustate->t
#define IL				cpustate->il

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE cop400_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == COP401 ||
		   device->type() == COP410 ||
		   device->type() == COP411 ||
		   device->type() == COP402 ||
		   device->type() == COP420 ||
		   device->type() == COP421 ||
		   device->type() == COP422 ||
		   device->type() == COP404 ||
		   device->type() == COP424 ||
		   device->type() == COP425 ||
		   device->type() == COP426 ||
		   device->type() == COP444 ||
		   device->type() == COP445);
	return (cop400_state *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE void PUSH(cop400_state *cpustate, UINT16 data)
{
	if (cpustate->featuremask != COP410_FEATURE)
	{
		SC = SB;
	}

	SB = SA;
	SA = data;
}

INLINE void POP(cop400_state *cpustate)
{
	PC = SA;
	SA = SB;

	if (cpustate->featuremask != COP410_FEATURE)
	{
		SB = SC;
	}
}

INLINE void WRITE_Q(cop400_state *cpustate, UINT8 data)
{
	Q = data;

	if (BIT(EN, 2))
	{
		OUT_L(Q);
	}
}

INLINE void WRITE_G(cop400_state *cpustate, UINT8 data)
{
	if (cpustate->intf->microbus == COP400_MICROBUS_ENABLED)
	{
		data = (data & 0x0e) | cpustate->microbus_int;
	}

	G = data;

	OUT_G(G);
}

/***************************************************************************
    OPCODE HANDLERS
***************************************************************************/

#define INSTRUCTION(mnemonic) INLINE void (mnemonic)(cop400_state *cpustate, UINT8 opcode)

INSTRUCTION(illegal)
{
	logerror("COP400: PC = %04x, Illegal opcode = %02x\n", PC-1, ROM(PC-1));
}

#include "cop400op.c"

/***************************************************************************
    OPCODE TABLES
***************************************************************************/

static const cop400_opcode_map COP410_OPCODE_23_MAP[] =
{
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, xad		},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	}
};

static void cop410_op23(cop400_state *cpustate, UINT8 opcode)
{
	UINT8 opcode23 = ROM(PC++);

	(*(COP410_OPCODE_23_MAP[opcode23].function))(cpustate, opcode23);
}

static const cop400_opcode_map COP410_OPCODE_33_MAP[] =
{
	{1, illegal 	},{1, skgbz0	},{1, illegal   },{1, skgbz2	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, skgbz1	},{1, illegal	},{1, skgbz3	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, skgz		},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, ing		},{1, illegal	},{1, illegal	},{1, illegal	},{1, inl		},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, halt		},{1, illegal	},{1, omg		},{1, illegal	},{1, camq		},{1, illegal	},{1, obd		},{1, illegal	},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, lei 		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},
	{1, lei 		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	}
};

static void cop410_op33(cop400_state *cpustate, UINT8 opcode)
{
	UINT8 opcode33 = ROM(PC++);

	(*(COP410_OPCODE_33_MAP[opcode33].function))(cpustate, opcode33);
}

static const cop400_opcode_map COP410_OPCODE_MAP[] =
{
	{1, clra		},{1, skmbz0	},{1, xor_		},{1, skmbz2		},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{0, illegal		},{1, skmbz1	},{0, illegal	},{1, skmbz3		},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, skc			},{1, ske		},{1, sc		},{1, cop410_op23	},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, asc			},{1, add		},{1, rc		},{1, cop410_op33	},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},

	{1, comp		},{0, illegal	},{1, rmb2		},{1, rmb3			},{1, nop		},{1, rmb1		},{1, smb2		},{1, smb1		},
	{1,	ret			},{1, retsk		},{0, illegal	},{1, smb3			},{1, rmb0		},{1, smb0		},{1, cba		},{1, xas		},
	{1, cab			},{1, aisc		},{1, aisc		},{1, aisc			},{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc		},
	{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc			},{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc		},
	{2, jmp			},{2, jmp		},{0, illegal	},{0, illegal		},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal   },
	{2, jsr			},{2, jsr		},{0, illegal	},{0, illegal		},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
	{1, stii		},{1, stii		},{1, stii		},{1, stii			},{1, stii		},{1, stii		},{1, stii		},{1, stii		},
	{1, stii		},{1, stii		},{1, stii		},{1, stii			},{1, stii		},{1, stii		},{1, stii		},{1, stii		},

	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{2, lqid		},

	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{2, jid		}
};

static const cop400_opcode_map COP420_OPCODE_23_MAP[] =
{
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},

	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	}
};

static void cop420_op23(cop400_state *cpustate, UINT8 opcode)
{
	UINT8 opcode23 = ROM(PC++);

	(*(COP420_OPCODE_23_MAP[opcode23].function))(cpustate, opcode23);
}

static const cop400_opcode_map COP420_OPCODE_33_MAP[] =
{
	{1, illegal		},{1, skgbz0	},{1, illegal	},{1, skgbz2	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, skgbz1	},{1, illegal	},{1, skgbz3	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, skgz		},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, inin		},{1, inil		},{1, ing		},{1, illegal	},{1, cqma		},{1, illegal	},{1, inl		},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, omg		},{1, illegal	},{1, camq		},{1, illegal	},{1, obd		},{1, illegal	},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, ogi			},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},
	{1, ogi			},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},
	{1, lei 		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},
	{1, lei 		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},

	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	}
};

static void cop420_op33(cop400_state *cpustate, UINT8 opcode)
{
	UINT8 opcode33 = ROM(PC++);

	(*(COP420_OPCODE_33_MAP[opcode33].function))(cpustate, opcode33);
}

static const cop400_opcode_map COP420_OPCODE_MAP[] =
{
	{1, clra		},{1, skmbz0	},{1, xor_		},{1, skmbz2		},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, casc		},{1, skmbz1	},{1, xabr		},{1, skmbz3		},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, skc			},{1, ske		},{1, sc		},{1, cop420_op23	},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, asc			},{1, add		},{1, rc		},{1, cop420_op33	},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},

	{1, comp		},{1, skt		},{1, rmb2		},{1, rmb3			},{1, nop		},{1, rmb1		},{1, smb2		},{1, smb1		},
	{1,	cop420_ret	},{1, retsk		},{1, adt		},{1, smb3			},{1, rmb0		},{1, smb0		},{1, cba		},{1, xas		},
	{1, cab			},{1, aisc		},{1, aisc		},{1, aisc			},{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc		},
	{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc			},{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc		},
	{2, jmp			},{2, jmp		},{2, jmp		},{2, jmp			},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal   },
	{2, jsr			},{2, jsr		},{2, jsr		},{2, jsr			},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
	{1, stii		},{1, stii		},{1, stii		},{1, stii			},{1, stii		},{1, stii		},{1, stii		},{1, stii		},
	{1, stii		},{1, stii		},{1, stii		},{1, stii			},{1, stii		},{1, stii		},{1, stii		},{1, stii		},

	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{2, lqid		},

	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{2, jid		}
};

static const cop400_opcode_map COP444_OPCODE_23_MAP[256] =
{
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},

	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},
	{1, ldd			},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},{1, ldd		},

	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},

	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
	{1, xad			},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},{1, xad		},
};

static void cop444_op23(cop400_state *cpustate, UINT8 opcode)
{
	UINT8 opcode23 = ROM(PC++);

	(*(COP444_OPCODE_23_MAP[opcode23].function))(cpustate, opcode23);
}

static const cop400_opcode_map COP444_OPCODE_33_MAP[256] =
{
	{1, illegal		},{1, skgbz0	},{1, illegal	},{1, skgbz2	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, skgbz1	},{1, illegal	},{1, skgbz3	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, skgz		},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, inin		},{1, inil		},{1, ing		},{1, illegal	},{1, cqma		},{1, illegal	},{1, inl		},{1, ctma		},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, it			},{1, illegal	},{1, omg		},{1, illegal	},{1, camq		},{1, illegal	},{1, obd		},{1, camt		},

	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, ogi			},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},
	{1, ogi			},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},{1, ogi		},
	{1, lei 		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},
	{1, lei 		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},{1, lei		},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},
	{1, illegal 	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},{1, illegal	},

	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},

	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
};

static void cop444_op33(cop400_state *cpustate, UINT8 opcode)
{
	UINT8 opcode33 = ROM(PC++);

	(*(COP444_OPCODE_33_MAP[opcode33].function))(cpustate, opcode33);
}

static const cop400_opcode_map COP444_OPCODE_MAP[256] =
{
	{1, clra		},{1, skmbz0	},{1, xor_		},{1, skmbz2		},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, casc		},{1, skmbz1	},{1, cop444_xabr},{1, skmbz3		},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, skc			},{1, ske		},{1, sc		},{1, cop444_op23	},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, asc			},{1, add		},{1, rc		},{1, cop444_op33	},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},

	{1, comp		},{1, skt		},{1, rmb2		},{1, rmb3			},{1, nop		},{1, rmb1		},{1, smb2		},{1, smb1		},
	{1,	cop420_ret	},{1, retsk		},{1, adt		},{1, smb3			},{1, rmb0		},{1, smb0		},{1, cba		},{1, xas		},
	{1, cab			},{1, aisc		},{1, aisc		},{1, aisc			},{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc		},
	{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc			},{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc		},
	{2, jmp			},{2, jmp		},{2, jmp		},{2, jmp			},{2, jmp		},{2, jmp		},{2, jmp		},{2, jmp		},
	{2, jsr			},{2, jsr		},{2, jsr		},{2, jsr			},{2, jsr		},{2, jsr		},{2, jsr		},{2, jsr		},
	{1, stii		},{1, stii		},{1, stii		},{1, stii			},{1, stii		},{1, stii		},{1, stii		},{1, stii		},
	{1, stii		},{1, stii		},{1, stii		},{1, stii			},{1, stii		},{1, stii		},{1, stii		},{1, stii		},

	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{2, lqid		},

	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{2, jid		}
};

/***************************************************************************
    TIMER CALLBACKS
***************************************************************************/

static TIMER_CALLBACK( serial_tick )
{
	cop400_state *cpustate = (cop400_state *)ptr;

	if (BIT(EN, 0))
	{
		/*

            SIO is an asynchronous binary counter decrementing its value by one upon each low-going pulse ("1" to "0") occurring on the SI input.
            Each pulse must remain at each logic level at least two instruction cycles. SK outputs the value of the C upon the execution of an XAS
            and remains latched until the execution of another XAS instruction. The SO output is equal to the value of EN3.

        */

		// serial output

		OUT_SO(BIT(EN, 3));

		// serial clock

		OUT_SK(SKL);

		// serial input

		cpustate->si <<= 1;
		cpustate->si = (cpustate->si & 0x0e) | IN_SI();

		if ((cpustate->si & 0x0f) == 0x0c) // 1100
		{
			SIO--;
			SIO &= 0x0f;
		}
	}
	else
	{
		/*

            SIO is a serial shift register, shifting continuously left each instruction cycle time. The data present at SI goes into the least
            significant bit of SIO: SO can be enabled to output the most significant bit of SIO each cycle time. SK output becomes a logic-
            controlled clock, providing a SYNC signal each instruction time. It will start outputting a SYNC pulse upon the execution of an XAS
            instruction with C = "1," stopping upon the execution of a subsequent XAS with C = "0".

            If EN0 is changed from "1" to "0" ("0" to "1") the SK output will change from "1" to SYNC (SYNC to "1") without the execution of
            an XAS instruction.

        */

		// serial output

		if (BIT(EN, 3))
		{
			OUT_SO(BIT(SIO, 3));
		}
		else
		{
			OUT_SO(0);
		}

		// serial clock

		if (SKL)
		{
			OUT_SK(1); // SYNC
		}
		else
		{
			OUT_SK(0);
		}

		// serial input

		SIO = ((SIO << 1) | IN_SI()) & 0x0f;
	}
}

static TIMER_CALLBACK( counter_tick )
{
	cop400_state *cpustate = (cop400_state *)ptr;

	T++;

	if (T == 0)
	{
		cpustate->skt_latch = 1;

		if (cpustate->idle)
		{
			cpustate->idle = 0;
			cpustate->halt = 0;
		}
	}
}

static TIMER_CALLBACK( inil_tick )
{
	cop400_state *cpustate = (cop400_state *)ptr;
	UINT8 in;
	int i;

	in = IN_IN();

	for (i = 0; i < 4; i++)
	{
		cpustate->in[i] = (cpustate->in[i] << 1) | BIT(in, i);

		if ((cpustate->in[i] & 0x07) == 0x04) // 100
		{
			IL |= (1 << i);
		}
	}
}

static TIMER_CALLBACK( microbus_tick )
{
	cop400_state *cpustate = (cop400_state *)ptr;
	UINT8 in;

	in = IN_IN();

	if (!BIT(in, 2))
	{
		// chip select

		if (!BIT(in, 1))
		{
			// read strobe

			OUT_L(Q);

			cpustate->microbus_int = 1;
		}
		else if (!BIT(in, 3))
		{
			// write strobe

			Q = IN_L();

			cpustate->microbus_int = 0;
		}
	}
}

/***************************************************************************
    INITIALIZATION
***************************************************************************/

static void define_state_table(device_t *device)
{
	cop400_state *cpustate = get_safe_token(device);

	device_state_interface *state;
	device->interface(state);
	state->state_add(STATE_GENPC,     "GENPC",     cpustate->pc).mask(0xfff).noshow();
	state->state_add(STATE_GENPCBASE, "GENPCBASE", cpustate->prevpc).mask(0xfff).noshow();
	state->state_add(STATE_GENSP,     "GENSP",     cpustate->n).mask(0x3).noshow();
	state->state_add(STATE_GENFLAGS,  "GENFLAGS",  cpustate->flags).mask(0x3).callimport().callexport().noshow().formatstr("%2s");

	state->state_add(COP400_PC,       "PC",        cpustate->pc).mask(0xfff);

	if (cpustate->featuremask & (COP410_FEATURE | COP420_FEATURE | COP444_FEATURE))
	{
		state->state_add(COP400_SA,   "SA",        cpustate->sa).mask(0xfff);
		state->state_add(COP400_SB,   "SB",        cpustate->sb).mask(0xfff);
		if (cpustate->featuremask & (COP420_FEATURE | COP444_FEATURE))
			state->state_add(COP400_SC, "SC",      cpustate->sc).mask(0xfff);
	}
	if (cpustate->featuremask & COP440_FEATURE)
		state->state_add(COP400_N,    "N",         cpustate->n).mask(0x3);

	state->state_add(COP400_A,        "A",         cpustate->a).mask(0xf);
	state->state_add(COP400_B,        "B",         cpustate->b);
	state->state_add(COP400_C,        "C",         cpustate->c).mask(0x1);

	state->state_add(COP400_EN,       "EN",        cpustate->en).mask(0xf);
	state->state_add(COP400_G,        "G",         cpustate->g).mask(0xf);
	if (cpustate->featuremask & COP440_FEATURE)
		state->state_add(COP400_H,    "H",         cpustate->h).mask(0xf);
	state->state_add(COP400_Q,        "Q",         cpustate->q);
	if (cpustate->featuremask & COP440_FEATURE)
		state->state_add(COP400_R,    "R",         cpustate->r);

	state->state_add(COP400_SIO,      "SIO",       cpustate->sio).mask(0xf);
	state->state_add(COP400_SKL,      "SKL",       cpustate->skl).mask(0x1);

	if (cpustate->featuremask & (COP420_FEATURE | COP444_FEATURE | COP440_FEATURE))
		state->state_add(COP400_T,    "T",         cpustate->t);
}

static void cop400_init(legacy_cpu_device *device, UINT8 g_mask, UINT8 d_mask, UINT8 in_mask, int has_counter, int has_inil)
{
	cop400_state *cpustate = get_safe_token(device);

	cpustate->intf = (cop400_interface *) device->static_config();

	/* find address spaces */

	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = &device->space(AS_DATA);
	cpustate->io = &device->space(AS_IO);

	/* set output pin masks */

	cpustate->g_mask = g_mask;
	cpustate->d_mask = d_mask;
	cpustate->in_mask = in_mask;

	/* allocate serial timer */

	cpustate->serial_timer = device->machine().scheduler().timer_alloc(FUNC(serial_tick), cpustate);
	cpustate->serial_timer->adjust(attotime::zero, 0, attotime::from_hz(device->clock() / 16));

	/* allocate counter timer */

	if (has_counter)
	{
		cpustate->counter_timer = device->machine().scheduler().timer_alloc(FUNC(counter_tick), cpustate);
		cpustate->counter_timer->adjust(attotime::zero, 0, attotime::from_hz(device->clock() / 16 / 4));
	}

	/* allocate IN latch timer */

	if (has_inil)
	{
		cpustate->inil_timer = device->machine().scheduler().timer_alloc(FUNC(inil_tick), cpustate);
		cpustate->inil_timer->adjust(attotime::zero, 0, attotime::from_hz(device->clock() / 16));
	}

	/* allocate Microbus timer */

	if (cpustate->intf->microbus == COP400_MICROBUS_ENABLED)
	{
		cpustate->microbus_timer = device->machine().scheduler().timer_alloc(FUNC(microbus_tick), cpustate);
		cpustate->microbus_timer->adjust(attotime::zero, 0, attotime::from_hz(device->clock() / 16));
	}

	/* register for state saving */

	device->save_item(NAME(cpustate->pc));
	device->save_item(NAME(cpustate->prevpc));
	device->save_item(NAME(cpustate->n));
	device->save_item(NAME(cpustate->sa));
	device->save_item(NAME(cpustate->sb));
	device->save_item(NAME(cpustate->sc));
	device->save_item(NAME(cpustate->a));
	device->save_item(NAME(cpustate->b));
	device->save_item(NAME(cpustate->c));
	device->save_item(NAME(cpustate->g));
	device->save_item(NAME(cpustate->h));
	device->save_item(NAME(cpustate->q));
	device->save_item(NAME(cpustate->r));
	device->save_item(NAME(cpustate->en));
	device->save_item(NAME(cpustate->sio));
	device->save_item(NAME(cpustate->skl));
	device->save_item(NAME(cpustate->t));
	device->save_item(NAME(cpustate->skip));
	device->save_item(NAME(cpustate->skip_lbi));
	device->save_item(NAME(cpustate->skt_latch));
	device->save_item(NAME(cpustate->si));
	device->save_item(NAME(cpustate->last_skip));
	device->save_item(NAME(cpustate->in));
	device->save_item(NAME(cpustate->microbus_int));
	device->save_item(NAME(cpustate->halt));
	device->save_item(NAME(cpustate->idle));
}

static void cop410_init_opcodes(device_t *device)
{
	cop400_state *cpustate = get_safe_token(device);
	int i;

	/* set up the state table */

	cpustate->featuremask = COP410_FEATURE;
	define_state_table(device);

	/* initialize instruction length array */

	for (i=0; i<256; i++) cpustate->InstLen[i]=1;

	cpustate->InstLen[0x60] = cpustate->InstLen[0x61] = cpustate->InstLen[0x68] =
	cpustate->InstLen[0x69] = cpustate->InstLen[0x33] = cpustate->InstLen[0x23] = 2;

	/* initialize LBI opcode array */

	for (i=0x00; i<0x100; i++) cpustate->LBIops[i] = 0;
	for (i=0x08; i<0x10; i++) cpustate->LBIops[i] = 1;
	for (i=0x18; i<0x20; i++) cpustate->LBIops[i] = 1;
	for (i=0x28; i<0x30; i++) cpustate->LBIops[i] = 1;
	for (i=0x38; i<0x40; i++) cpustate->LBIops[i] = 1;

	/* select opcode map */

	cpustate->opcode_map = COP410_OPCODE_MAP;
}

static void cop420_init_opcodes(device_t *device)
{
	cop400_state *cpustate = get_safe_token(device);
	int i;

	/* set up the state table */

	cpustate->featuremask = COP420_FEATURE;
	define_state_table(device);

	/* initialize instruction length array */

	for (i=0; i<256; i++) cpustate->InstLen[i]=1;

	cpustate->InstLen[0x60] = cpustate->InstLen[0x61] = cpustate->InstLen[0x62] = cpustate->InstLen[0x63] =
	cpustate->InstLen[0x68] = cpustate->InstLen[0x69] = cpustate->InstLen[0x6a] = cpustate->InstLen[0x6b] =
	cpustate->InstLen[0x33] = cpustate->InstLen[0x23] = 2;

	/* initialize LBI opcode array */

	for (i=0x00; i<0x100; i++) cpustate->LBIops[i] = 0;
	for (i=0x08; i<0x10; i++) cpustate->LBIops[i] = 1;
	for (i=0x18; i<0x20; i++) cpustate->LBIops[i] = 1;
	for (i=0x28; i<0x30; i++) cpustate->LBIops[i] = 1;
	for (i=0x38; i<0x40; i++) cpustate->LBIops[i] = 1;

	for (i=0; i<256; i++) cpustate->LBIops33[i] = 0;
	for (i=0x80; i<0xc0; i++) cpustate->LBIops33[i] = 1;

	/* select opcode map */

	cpustate->opcode_map = COP420_OPCODE_MAP;
}

static void cop444_init_opcodes(device_t *device)
{
	cop400_state *cpustate = get_safe_token(device);
	int i;

	/* set up the state table */

	cpustate->featuremask = COP444_FEATURE;
	define_state_table(device);

	/* initialize instruction length array */

	for (i=0; i<256; i++) cpustate->InstLen[i]=1;

	cpustate->InstLen[0x60] = cpustate->InstLen[0x61] = cpustate->InstLen[0x62] = cpustate->InstLen[0x63] =
	cpustate->InstLen[0x68] = cpustate->InstLen[0x69] = cpustate->InstLen[0x6a] = cpustate->InstLen[0x6b] =
	cpustate->InstLen[0x33] = cpustate->InstLen[0x23] = 2;

	/* initialize LBI opcode array */

	for (i=0x00; i<0x100; i++) cpustate->LBIops[i] = 0;
	for (i=0x08; i<0x10; i++) cpustate->LBIops[i] = 1;
	for (i=0x18; i<0x20; i++) cpustate->LBIops[i] = 1;
	for (i=0x28; i<0x30; i++) cpustate->LBIops[i] = 1;
	for (i=0x38; i<0x40; i++) cpustate->LBIops[i] = 1;

	for (i=0; i<256; i++) cpustate->LBIops33[i] = 0;
	for (i=0x80; i<0xc0; i++) cpustate->LBIops33[i] = 1;

	/* select opcode map */

	cpustate->opcode_map = COP444_OPCODE_MAP;
}

static CPU_INIT( cop410 )
{
	cop410_init_opcodes(device);
	cop400_init(device, 0xf, 0xf, 0, 0, 0);
}

static CPU_INIT( cop411 )
{
	cop410_init_opcodes(device);
	cop400_init(device, 0x7, 0x3, 0, 0, 0);
}

static CPU_INIT( cop420 )
{
	cop420_init_opcodes(device);
	cop400_init(device, 0xf, 0xf, 0xf, 1, 1);
}

static CPU_INIT( cop421 )
{
	cop420_init_opcodes(device);
	cop400_init(device, 0xf, 0xf, 0, 1, 0);
}

static CPU_INIT( cop422 )
{
	cop420_init_opcodes(device);
	cop400_init(device, 0xe, 0xe, 0, 1, 0);
}

static CPU_INIT( cop444 )
{
	cop444_init_opcodes(device);
	cop400_init(device, 0xf, 0xf, 0xf, 1, 1);
}

static CPU_INIT( cop425 )
{
	cop444_init_opcodes(device);
	cop400_init(device, 0xf, 0xf, 0, 1, 0);
}

static CPU_INIT( cop426 )
{
	cop444_init_opcodes(device);
	cop400_init(device, 0xe, 0xe, 0xf, 1, 1);
}

static CPU_INIT( cop445 )
{
	cop444_init_opcodes(device);
	cop400_init(device, 0x7, 0x3, 0, 1, 0);
}

/***************************************************************************
    RESET
***************************************************************************/

static CPU_RESET( cop400 )
{
	cop400_state *cpustate = get_safe_token(device);

	PC = 0;
	A = 0;
	B = 0;
	C = 0;
	OUT_D(0);
	EN = 0;
	WRITE_G(cpustate, 0);
	SKL = 1;

	T = 0;
	cpustate->skt_latch = 1;

	cpustate->halt = 0;
	cpustate->idle = 0;
}

/***************************************************************************
    EXECUTION
***************************************************************************/

static CPU_EXECUTE( cop400 )
{
	cop400_state *cpustate = get_safe_token(device);

	UINT8 opcode;

	do
	{
		cpustate->prevpc = PC;

		debugger_instruction_hook(device, PC);

		if (cpustate->intf->cko == COP400_CKO_HALT_IO_PORT)
		{
			cpustate->halt = IN_CKO();
		}

		if (cpustate->halt)
		{
			cpustate->icount -= 1;
			continue;
		}

		opcode = ROM(PC);

		if (cpustate->skip_lbi)
		{
			int is_lbi = 0;

			if (opcode == 0x33)
			{
				is_lbi = cpustate->LBIops33[ROM(PC+1)];
			}
			else
			{
				is_lbi = cpustate->LBIops[opcode];
			}

			if (is_lbi)
			{
				cpustate->icount -= cpustate->opcode_map[opcode].cycles;

				PC += cpustate->InstLen[opcode];
			}
			else
			{
				cpustate->skip_lbi = 0;
			}
		}

		if (!cpustate->skip_lbi)
		{
			int inst_cycles = cpustate->opcode_map[opcode].cycles;

			PC++;

			(*(cpustate->opcode_map[opcode].function))(cpustate, opcode);
			cpustate->icount -= inst_cycles;

			// check for interrupt

			if (BIT(EN, 1) && BIT(IL, 1))
			{
				cop400_opcode_func function = cpustate->opcode_map[ROM(PC)].function;

				if ((function != jp) &&	(function != jmp) && (function != jsr))
				{
					// store skip logic
					cpustate->last_skip = cpustate->skip;
					cpustate->skip = 0;

					// push next PC
					PUSH(cpustate, PC);

					// jump to interrupt service routine
					PC = 0x0ff;

					// disable interrupt
					EN &= ~0x02;
				}

				IL &= ~2;
			}

			// skip next instruction?

			if (cpustate->skip)
			{
				cop400_opcode_func function = cpustate->opcode_map[ROM(PC)].function;

				opcode = ROM(PC);

				if ((function == lqid) || (function == jid))
				{
					cpustate->icount -= 1;
				}
				else
				{
					cpustate->icount -= cpustate->opcode_map[opcode].cycles;
				}

				PC += cpustate->InstLen[opcode];

				cpustate->skip = 0;
			}
		}
	} while (cpustate->icount > 0);
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( program_512b, AS_PROGRAM, 8, legacy_cpu_device )
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( program_1kb, AS_PROGRAM, 8, legacy_cpu_device )
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( program_2kb, AS_PROGRAM, 8, legacy_cpu_device )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( data_32b, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x1f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( data_64b, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( data_128b, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END

#ifdef UNUSED_CODE
static ADDRESS_MAP_START( data_160b, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x9f) AM_RAM
ADDRESS_MAP_END
#endif

/***************************************************************************
    VALIDITY CHECKS
***************************************************************************/

#if 0
static CPU_VALIDITY_CHECK( cop410 )
{
	int error = FALSE;
	const cop400_interface *intf = (const cop400_interface *) config;

	if ((intf == NULL) || (intf->cki <= 0))
	{
		mame_printf_error("%s: %s has an invalid CPU configuration\n", driver->source_file, driver->name);
		error = TRUE;
	}

	return error;
}

static CPU_VALIDITY_CHECK( cop420 )
{
	int error = FALSE;
	const cop400_interface *intf = (const cop400_interface *) config;

	if ((intf == NULL) || (intf->cki <= 0))
	{
		mame_printf_error("%s: %s has an invalid CPU configuration\n", driver->source_file, driver->name);
		error = TRUE;
	}

	return error;
}

static CPU_VALIDITY_CHECK( cop421 )
{
	int error = FALSE;
	const cop400_interface *intf = (const cop400_interface *) config;

	if ((intf == NULL) || (intf->cki <= 0) || (intf->microbus == COP400_MICROBUS_ENABLED))
	{
		mame_printf_error("%s: %s has an invalid CPU configuration\n", driver->source_file, driver->name);
		error = TRUE;
	}

	return error;
}

static CPU_VALIDITY_CHECK( cop444 )
{
	int error = FALSE;
	const cop400_interface *intf = (const cop400_interface *) config;

	if ((intf == NULL) || (intf->cki <= 0))
	{
		mame_printf_error("%s: %s has an invalid CPU configuration\n", driver->source_file, driver->name);
		error = TRUE;
	}

	return error;
}
#endif

/***************************************************************************
    GENERAL CONTEXT ACCESS
***************************************************************************/

static CPU_IMPORT_STATE( cop400 )
{
	cop400_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			cpustate->c = (cpustate->flags >> 1) & 1;
			cpustate->skl = (cpustate->flags >> 0) & 1;
			break;
	}
}

static CPU_EXPORT_STATE( cop400 )
{
	cop400_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			cpustate->flags = (cpustate->c ? 0x02 : 0x00) |
							  (cpustate->skl ? 0x01 : 0x00);
			break;
	}
}

static CPU_EXPORT_STRING( cop400 )
{
	cop400_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c",
						 cpustate->c ? 'C' : '.',
						 cpustate->skl ? 'S' : '.');
			break;
	}
}

static CPU_SET_INFO( cop400 )
{
	/* nothing to set */
}

static CPU_GET_INFO( cop400 )
{
	cop400_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	cop400_interface *intf = (device->static_config() != NULL) ? (cop400_interface *)device->static_config() : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(cop400_state);							break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;											break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;											break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;							break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;											break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = (intf != NULL) ? intf->cki : 16;				break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;											break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;											break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;											break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;											break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 8;											break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			/* set per-core */										break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:			info->i = 0;											break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:			info->i = 8; /* really 4 */								break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			/* set per-core */										break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:			info->i = 0;											break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:				info->i = 8;											break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:				info->i = 9;											break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:				info->i = 0;											break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(cop400);				break;
		case CPUINFO_FCT_INIT:							/* set per-core */										break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(cop400);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(cop400);				break;
		case CPUINFO_FCT_DISASSEMBLE:					/* set per-core */										break;
//      case CPUINFO_FCT_VALIDITY_CHECK:                /* set per-core */                                      break;
		case CPUINFO_FCT_IMPORT_STATE:					info->import_state = CPU_IMPORT_STATE_NAME(cop400);		break;
		case CPUINFO_FCT_EXPORT_STATE:					info->export_state = CPU_EXPORT_STATE_NAME(cop400);		break;
		case CPUINFO_FCT_EXPORT_STRING: 				info->export_string = CPU_EXPORT_STRING_NAME(cop400);	break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;						break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	/* set per-core */										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		/* set per-core */										break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP400");								break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "National Semiconductor COPS");			break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0");									break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);								break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright MAME Team");					break;
	}
}

/***************************************************************************
    CPU-SPECIFIC CONTEXT ACCESS
***************************************************************************/

CPU_GET_INFO( cop410 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 9;											break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 5;											break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop410);						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(cop410);		break;
//      case CPUINFO_FCT_VALIDITY_CHECK:                info->validity_check = CPU_VALIDITY_CHECK_NAME(cop410); break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(program_512b);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map8 = ADDRESS_MAP_NAME(data_32b);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP410");								break;

		default:										CPU_GET_INFO_CALL(cop400);								break;
	}
}

CPU_GET_INFO( cop411 )
{
	// COP411 is a 20-pin package version of the COP410, missing D2/D3/G3/CKO

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop411);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP411");								break;

		default:										CPU_GET_INFO_CALL(cop410);								break;
	}
}

CPU_GET_INFO( cop401 )
{
	// COP401 is a ROMless version of the COP410

	switch (state)
	{
		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = NULL;								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP401");								break;

		default:										CPU_GET_INFO_CALL(cop410);								break;
	}
}

CPU_GET_INFO( cop420 )
{
	cop400_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 10;											break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 6;											break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop420);						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(cop420);		break;
//      case CPUINFO_FCT_VALIDITY_CHECK:                info->validity_check = CPU_VALIDITY_CHECK_NAME(cop420); break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(program_1kb);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map8 = ADDRESS_MAP_NAME(data_64b);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP420");								break;

		case CPUINFO_STR_FLAGS: sprintf(info->s,
									"%c%c%c",
									 cpustate->c ? 'C' : '.',
									 cpustate->skl ? 'S' : '.',
									 cpustate->skt_latch ? 'T' : '.'); break;

		default:										CPU_GET_INFO_CALL(cop400);								break;
	}
}

CPU_GET_INFO( cop421 )
{
	// COP421 is a 24-pin package version of the COP420, lacking the IN ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop421);						break;
//      case CPUINFO_FCT_VALIDITY_CHECK:                info->validity_check = CPU_VALIDITY_CHECK_NAME(cop421); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP421");								break;

		default:										CPU_GET_INFO_CALL(cop420);								break;
	}
}

CPU_GET_INFO( cop422 )
{
	// COP422 is a 20-pin package version of the COP420, lacking G0/G1, D0/D1, and the IN ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop422);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP422");								break;

		default:										CPU_GET_INFO_CALL(cop421);								break;
	}
}

CPU_GET_INFO( cop402 )
{
	// COP402 is a ROMless version of the COP420

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = NULL;								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP402");								break;

		default:										CPU_GET_INFO_CALL(cop420);								break;
	}
}

CPU_GET_INFO( cop444 )
{
	cop400_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 11;											break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 7;											break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop444);						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(cop444);		break;
//      case CPUINFO_FCT_VALIDITY_CHECK:                info->validity_check = CPU_VALIDITY_CHECK_NAME(cop444); break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(program_2kb);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map8 = ADDRESS_MAP_NAME(data_128b);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP444");								break;

		case CPUINFO_STR_FLAGS: sprintf(info->s,
									"%c%c%c",
									 cpustate->c ? 'C' : '.',
									 cpustate->skl ? 'S' : '.',
									 cpustate->skt_latch ? 'T' : '.'); break;

		default:										CPU_GET_INFO_CALL(cop400);								break;
	}
}

CPU_GET_INFO( cop445 )
{
	// COP445 is a 24-pin package version of the COP444, lacking the IN ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop445);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP445");								break;

		default:										CPU_GET_INFO_CALL(cop444);								break;
	}
}

CPU_GET_INFO( cop424 )
{
	// COP424 is functionally equivalent to COP444, with only 1K ROM and 64x4 bytes RAM

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 10;											break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 6;											break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(program_1kb);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map8 = ADDRESS_MAP_NAME(data_64b);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP424");								break;

		default:										CPU_GET_INFO_CALL(cop444);								break;
	}
}

CPU_GET_INFO( cop425 )
{
	// COP425 is a 24-pin package version of the COP424, lacking the IN ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop425);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP425");								break;

		default:										CPU_GET_INFO_CALL(cop424);								break;
	}
}

CPU_GET_INFO( cop426 )
{
	// COP426 is a 20-pin package version of the COP424, with only L0-L7, G2-G3, D2-D3 ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop426);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP426");								break;

		default:										CPU_GET_INFO_CALL(cop424);								break;
	}
}

CPU_GET_INFO( cop404 )
{
	// COP404 is a ROMless version of the COP444, which can emulate a COP410C/COP411C, COP424C/COP425C, or a COP444C/COP445C

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = NULL;								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP404");								break;

		default:										CPU_GET_INFO_CALL(cop444);								break;
	}
}

/* COP410 family */
DEFINE_LEGACY_CPU_DEVICE(COP401, cop401);
DEFINE_LEGACY_CPU_DEVICE(COP410, cop410);
DEFINE_LEGACY_CPU_DEVICE(COP411, cop411);

/* COP420 family */
DEFINE_LEGACY_CPU_DEVICE(COP402, cop402);
DEFINE_LEGACY_CPU_DEVICE(COP420, cop420);
DEFINE_LEGACY_CPU_DEVICE(COP421, cop421);
DEFINE_LEGACY_CPU_DEVICE(COP422, cop422);

/* COP444 family */
DEFINE_LEGACY_CPU_DEVICE(COP404, cop404);
DEFINE_LEGACY_CPU_DEVICE(COP424, cop424);
DEFINE_LEGACY_CPU_DEVICE(COP425, cop425);
DEFINE_LEGACY_CPU_DEVICE(COP426, cop426);
DEFINE_LEGACY_CPU_DEVICE(COP444, cop444);
DEFINE_LEGACY_CPU_DEVICE(COP445, cop445);
