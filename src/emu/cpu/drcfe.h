/***************************************************************************

    drcfe.h

    Generic dynamic recompiler frontend structures and utilities.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Concepts:

    Dynamic recompiling cores are generally broken into a platform-neutral
    "frontend", which performs some level of analysis on the code, and a
    platform-specific "backend", which generates the recompiled machine
    code.

    The frontend's job is generally to walk through the instruction stream,
    identifying basic blocks, or "sequences" of code that can be compiled
    and optimized as a unit. This scanning involves recursively walking
    the instruction stream, following branches, etc., within a specific
    "code window", relative to the current PC.

    As the frontend walks through the code, it generates a list of opcode
    "descriptions", one per visited opcode, providing information about
    code flow, exception handling, and other characteristics. Once the
    walkthrough is finished, these descriptions are assembled together into
    a linked list and returned for further processing by the backend.

***************************************************************************/

#pragma once

#ifndef __DRCFE_H__
#define __DRCFE_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* this defines a branch targetpc that is dynamic at runtime */
#define BRANCH_TARGET_DYNAMIC			(~0)


/* opcode branch flags */
#define OPFLAG_IS_UNCONDITIONAL_BRANCH	0x00000001		/* instruction is unconditional branch */
#define OPFLAG_IS_CONDITIONAL_BRANCH	0x00000002		/* instruction is conditional branch */
#define OPFLAG_IS_BRANCH				(OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_IS_CONDITIONAL_BRANCH)
#define OPFLAG_IS_BRANCH_TARGET			0x00000004		/* instruction is the target of a branch */
#define OPFLAG_IN_DELAY_SLOT			0x00000008		/* instruction is in the delay slot of a branch */
#define OPFLAG_INTRABLOCK_BRANCH		0x00000010		/* instruction branches within the block */

/* opcode exception flags */
#define OPFLAG_CAN_TRIGGER_SW_INT		0x00000020		/* instruction can trigger a software interrupt */
#define OPFLAG_CAN_EXPOSE_EXTERNAL_INT	0x00000040		/* instruction can expose an external interrupt */
#define OPFLAG_CAN_CAUSE_EXCEPTION		0x00000080		/* instruction may generate exception */
#define OPFLAG_WILL_CAUSE_EXCEPTION		0x00000100		/* instruction will generate exception */
#define OPFLAG_PRIVILEGED				0x00000200		/* instruction is privileged */

/* opcode virtual->physical translation flags */
#define OPFLAG_VALIDATE_TLB				0x00000400		/* instruction must validate TLB before execution */
#define OPFLAG_MODIFIES_TRANSLATION		0x00000800		/* instruction modifies the TLB */
#define OPFLAG_COMPILER_PAGE_FAULT		0x00001000		/* compiler hit a page fault when parsing */
#define OPFLAG_COMPILER_UNMAPPED		0x00002000		/* compiler hit unmapped memory when parsing */

/* opcode flags */
#define OPFLAG_INVALID_OPCODE			0x00004000		/* instruction is invalid */
#define OPFLAG_VIRTUAL_NOOP				0x00008000		/* instruction is a virtual no-op */

/* opcode sequence flow flags */
#define OPFLAG_REDISPATCH				0x00010000		/* instruction must redispatch after completion */
#define OPFLAG_RETURN_TO_START			0x00020000		/* instruction must jump back to the beginning after completion */
#define OPFLAG_END_SEQUENCE				0x00040000		/* this is the last instruction in a sequence */
#define OPFLAG_CAN_CHANGE_MODES			0x00080000		/* instruction can change modes */

/* execution semantics */
#define OPFLAG_READS_MEMORY				0x00100000		/* instruction reads memory */
#define OPFLAG_WRITES_MEMORY			0x00200000		/* instruction writes memory */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* opaque internal state */
typedef struct _drcfe_state drcfe_state;


/* description of a given opcode */
typedef struct _opcode_desc opcode_desc;
struct _opcode_desc
{
	/* links to other descriptions */
	opcode_desc *	next;					/* pointer to next description */
	opcode_desc *	prev;					/* pointer to previous description */
	opcode_desc *	branch;					/* pointer back to branch description for delay slots */
	opcode_desc *	delay;					/* pointer to delay slot description */

	/* information about the current PC */
	offs_t			pc;						/* PC of this opcode */
	offs_t			physpc;					/* physical PC of this opcode */
	offs_t			targetpc;				/* target PC if we are a branch, or BRANCH_TARGET_DYNAMIC */

	/* copy of up to 16 bytes of opcode */
	union
	{
		UINT8		b[16];
		UINT16		w[8];
		UINT32		l[4];
		UINT64		q[2];
	} opptr;								/* pointer to opcode memory */

	/* information about this instruction's execution */
	UINT8			length;					/* length in bytes of this opcode */
	UINT8			delayslots;				/* number of delay slots (for branches) */
	UINT8			skipslots;				/* number of skip slots (for branches) */
	UINT32			flags;					/* OPFLAG_* opcode flags */
	UINT32			cycles;					/* number of cycles needed to execute */

	/* register usage information */
	UINT32			regin[4];				/* input registers */
	UINT32			regout[4];				/* output registers */
	UINT32			regreq[4];				/* required output registers */
};


/* callback function that is used to describe a single opcode */
typedef int (*drcfe_describe_func)(void *param, opcode_desc *desc, const opcode_desc *prev);


/* description of a given opcode */
typedef struct _drcfe_config drcfe_config;
struct _drcfe_config
{
	UINT32			window_start;			/* code window start offset = startpc - window_start */
	UINT32			window_end;				/* code window end offset = startpc + window_end */
	UINT32			max_sequence;			/* maximum instructions to include in a sequence */
	drcfe_describe_func	describe;				/* callback to describe a single instruction */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initializate the drcfe state */
drcfe_state *drcfe_init(running_device *cpu, const drcfe_config *config, void *param);

/* clean up after ourselves */
void drcfe_exit(drcfe_state *drcfe);

/* describe a sequence of code that falls within the configured window relative to the specified startpc */
const opcode_desc *drcfe_describe_code(drcfe_state *drcfe, offs_t startpc);


#endif /* __DRCFE_H__ */
