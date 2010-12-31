/***************************************************************************

    drcfe.c

    Generic dynamic recompiler frontend structures and utilities.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Future improvements/changes:

    * more aggressive handling of needed registers for conditional
        intrablock branches

***************************************************************************/

#include "emu.h"
#include "drcfe.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_STACK_DEPTH		100



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* an entry that maps branches for our code walking */
typedef struct _pc_stack_entry pc_stack_entry;
struct _pc_stack_entry
{
	offs_t				targetpc;
	offs_t				srcpc;
};


/* internal state */
struct _drcfe_state
{
	/* configuration parameters */
	UINT32				window_start;				/* code window start offset = startpc - window_start */
	UINT32				window_end;					/* code window end offset = startpc + window_end */
	UINT32				max_sequence;				/* maximum instructions to include in a sequence */

	drcfe_describe_func	describe;					/* callback to describe a single instruction */
	void *				param;						/* parameter for the callback */

	/* CPU parameters */
	cpu_device *		cpudevice;					/* CPU device object */
	address_space *program;					/* program address space for this CPU */
	offs_t				pageshift;					/* shift to convert address to a page index */

	/* opcode descriptor arrays */
	opcode_desc *		desc_live_list;				/* head of list of live descriptions */
	opcode_desc *		desc_free_list;				/* head of list of free descriptions */
	opcode_desc **		desc_array;					/* array of descriptions in PC order */
	UINT32				desc_array_size;			/* size of the array */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static opcode_desc *describe_one(drcfe_state *drcfe, offs_t curpc, const opcode_desc *prevdesc);
static opcode_desc **build_sequence(drcfe_state *drcfe, opcode_desc **tailptr, int start, int end, UINT32 endflag);
static void accumulate_required_backwards(opcode_desc *desc, UINT32 *reqmask);
static void release_descriptions(drcfe_state *drcfe, opcode_desc *desc);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    desc_alloc - allocate a new opcode description
-------------------------------------------------*/

INLINE opcode_desc *desc_alloc(drcfe_state *drcfe)
{
	opcode_desc *desc = drcfe->desc_free_list;

	/* pull a description off of the free list or allocate a new one */
	if (desc != NULL)
		drcfe->desc_free_list = desc->next;
	else
		desc = auto_alloc(drcfe->cpudevice->machine, opcode_desc);
	return desc;
}


/*-------------------------------------------------
    desc_free - free an opcode description
-------------------------------------------------*/

INLINE void desc_free(drcfe_state *drcfe, opcode_desc *desc)
{
	/* just put ourselves on the free list */
	desc->next = drcfe->desc_free_list;
	drcfe->desc_free_list = desc;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    drcfe_init - initializate the drcfe state
-------------------------------------------------*/

drcfe_state *drcfe_init(device_t *cpu, const drcfe_config *config, void *param)
{
	drcfe_state *drcfe;

	/* allocate some memory to hold the state */
	drcfe = auto_alloc_clear(cpu->machine, drcfe_state);

	/* allocate the description array */
	drcfe->desc_array = auto_alloc_array_clear(cpu->machine, opcode_desc *, config->window_end + config->window_start + 2);

	/* copy in configuration information */
	drcfe->window_start = config->window_start;
	drcfe->window_end = config->window_end;
	drcfe->max_sequence = config->max_sequence;
	drcfe->describe = config->describe;
	drcfe->param = param;

	/* initialize the state */
	drcfe->cpudevice = downcast<cpu_device *>(cpu);
	drcfe->program = drcfe->cpudevice->space(AS_PROGRAM);
	drcfe->pageshift = drcfe->cpudevice->space_config(AS_PROGRAM)->m_page_shift;

	return drcfe;
}


/*-------------------------------------------------
    drcfe_exit - clean up after ourselves
-------------------------------------------------*/

void drcfe_exit(drcfe_state *drcfe)
{
	/* release any descriptions we've accumulated */
	release_descriptions(drcfe, drcfe->desc_live_list);

	/* free our free list of descriptions */
	while (drcfe->desc_free_list != NULL)
	{
		opcode_desc *freeme = drcfe->desc_free_list;
		drcfe->desc_free_list = drcfe->desc_free_list->next;
		auto_free(drcfe->cpudevice->machine, freeme);
	}

	/* free the description array */
	auto_free(drcfe->cpudevice->machine, drcfe->desc_array);

	/* free the object itself */
	auto_free(drcfe->cpudevice->machine, drcfe);
}


/*-------------------------------------------------
    drcfe_describe_code - describe a sequence of
    code that falls within the configured window
    relative to the specified startpc
-------------------------------------------------*/

const opcode_desc *drcfe_describe_code(drcfe_state *drcfe, offs_t startpc)
{
	offs_t minpc = startpc - MIN(drcfe->window_start, startpc);
	offs_t maxpc = startpc + MIN(drcfe->window_end, 0xffffffff - startpc);
	pc_stack_entry pcstack[MAX_STACK_DEPTH];
	pc_stack_entry *pcstackptr = &pcstack[0];
	opcode_desc **tailptr;

	/* release any descriptions we've accumulated */
	release_descriptions(drcfe, drcfe->desc_live_list);
	drcfe->desc_live_list = NULL;

	/* add the initial PC to the stack */
	pcstackptr->srcpc = 0;
	pcstackptr->targetpc = startpc;
	pcstackptr++;

	/* loop while we still have a stack */
	while (pcstackptr != &pcstack[0])
	{
		pc_stack_entry *curstack = --pcstackptr;
		opcode_desc *curdesc;
		offs_t curpc;

		/* if we've already hit this PC, just mark it a branch target and continue */
		curdesc = drcfe->desc_array[curstack->targetpc - minpc];
		if (curdesc != NULL)
		{
			curdesc->flags |= OPFLAG_IS_BRANCH_TARGET;

			/* if the branch crosses a page boundary, mark the target as needing to revalidate */
			if (drcfe->pageshift != 0 && ((curstack->srcpc ^ curdesc->pc) >> drcfe->pageshift) != 0)
				curdesc->flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION;

			/* continue processing */
			continue;
		}

		/* loop until we exit the block */
		for (curpc = curstack->targetpc; curpc >= minpc && curpc < maxpc && drcfe->desc_array[curpc - minpc] == NULL; curpc += drcfe->desc_array[curpc - minpc]->length)
		{
			/* allocate a new description and describe this instruction */
			drcfe->desc_array[curpc - minpc] = curdesc = describe_one(drcfe, curpc, curdesc);

			/* first instruction in a sequence is always a branch target */
			if (curpc == curstack->targetpc)
				curdesc->flags |= OPFLAG_IS_BRANCH_TARGET;

			/* stop if we hit a page fault */
			if (curdesc->flags & OPFLAG_COMPILER_PAGE_FAULT)
				break;

			/* if we are the first instruction in the whole window, we must validate the TLB */
			if (curpc == startpc && drcfe->pageshift != 0)
				curdesc->flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION;

			/* if we are a branch within the block range, add the branch target to our stack */
			if ((curdesc->flags & OPFLAG_IS_BRANCH) && curdesc->targetpc >= minpc && curdesc->targetpc < maxpc && pcstackptr < &pcstack[MAX_STACK_DEPTH])
			{
				curdesc->flags |= OPFLAG_INTRABLOCK_BRANCH;
				pcstackptr->srcpc = curdesc->pc;
				pcstackptr->targetpc = curdesc->targetpc;
				pcstackptr++;
			}

			/* if we're done, we're done */
			if (curdesc->flags & OPFLAG_END_SEQUENCE)
				break;
		}
	}

	/* now build the list of descriptions in order */
	/* first from startpc -> maxpc, then from minpc -> startpc */
	tailptr = build_sequence(drcfe, &drcfe->desc_live_list, startpc - minpc, maxpc - minpc, OPFLAG_REDISPATCH);
	tailptr = build_sequence(drcfe, tailptr, minpc - minpc, startpc - minpc, OPFLAG_RETURN_TO_START);
	return drcfe->desc_live_list;
}



/***************************************************************************
    INTERNAL HELPERS
***************************************************************************/

/*-------------------------------------------------
    describe_one - describe a single instruction,
    recursively describing opcodes in delay
    slots of branches as well
-------------------------------------------------*/

static opcode_desc *describe_one(drcfe_state *drcfe, offs_t curpc, const opcode_desc *prevdesc)
{
	opcode_desc *desc = desc_alloc(drcfe);

	/* initialize the description */
	memset(desc, 0, sizeof(*desc));
	desc->pc = curpc;
	desc->physpc = curpc;
	desc->targetpc = BRANCH_TARGET_DYNAMIC;

	/* call the callback to describe an instruction */
	if (!(*drcfe->describe)(drcfe->param, desc, prevdesc))
	{
		desc->flags |= OPFLAG_WILL_CAUSE_EXCEPTION | OPFLAG_INVALID_OPCODE;
		return desc;
	}

	/* validate the TLB if we are exactly at the start of a page, or if we cross a page boundary */
	if (drcfe->pageshift != 0 && (((curpc - 1) ^ (curpc + desc->length - 1)) >> drcfe->pageshift) != 0)
		desc->flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION;

	/* validate stuff */
	assert(desc->length > 0 || (desc->flags & OPFLAG_VIRTUAL_NOOP) != 0);

	/* if we are a branch with delay slots, recursively walk those */
	if (desc->flags & OPFLAG_IS_BRANCH)
	{
		opcode_desc **tailptr = &desc->delay;
		offs_t delaypc = curpc + desc->length;
		opcode_desc *prev = desc;
		UINT8 slotnum;

		/* iterate over slots and describe them */
		for (slotnum = 0; slotnum < desc->delayslots; slotnum++)
		{
			/* recursively describe the next instruction */
			*tailptr = describe_one(drcfe, delaypc, prev);
			if (*tailptr == NULL)
				break;

			/* set the delay slot flag and a pointer back to the original branch */
			(*tailptr)->flags |= OPFLAG_IN_DELAY_SLOT;
			(*tailptr)->branch = desc;
			(*tailptr)->prev = prev;
			prev = *tailptr;

			/* stop if we hit a page fault */
			if ((*tailptr)->flags & OPFLAG_COMPILER_PAGE_FAULT)
				break;

			/* otherwise, advance */
			delaypc += (*tailptr)->length;
			tailptr = &(*tailptr)->next;
		}
	}

	return desc;
}


/*-------------------------------------------------
    build_sequence - build an ordered sequence
    of instructions
-------------------------------------------------*/

static opcode_desc **build_sequence(drcfe_state *drcfe, opcode_desc **tailptr, int start, int end, UINT32 endflag)
{
	opcode_desc *prev = NULL;
	int consecutive = 0;
	int seqstart = -1;
	int skipsleft = 0;
	int descnum;

	/* iterate in order from start to end, picking up all non-NULL instructions */
	for (descnum = start; descnum < end; descnum++)
		if (drcfe->desc_array[descnum] != NULL)
		{
			opcode_desc *curdesc = drcfe->desc_array[descnum];
			opcode_desc *nextdesc = NULL;
			int nextdescnum;
			UINT8 skipnum;

			/* determine the next instruction, taking skips into account */
			nextdescnum = descnum + curdesc->length;
			nextdesc = (nextdescnum < end) ? drcfe->desc_array[nextdescnum] : NULL;
			for (skipnum = 0; skipnum < curdesc->skipslots && nextdesc != NULL; skipnum++)
			{
				nextdescnum = nextdescnum + nextdesc->length;
				nextdesc = (nextdescnum < end) ? drcfe->desc_array[nextdescnum] : NULL;
			}

			/* start a new sequence if we aren't already in the middle of one */
			if (seqstart == -1 && skipsleft == 0)
			{
				/* tag all start-of-sequence instructions as needing TLB verification */
				curdesc->flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION;
				seqstart = descnum;
			}

			/* if we are the last instruction, indicate end-of-sequence and redispatch */
			if (nextdesc == NULL)
			{
				curdesc->flags |= OPFLAG_END_SEQUENCE;
				if (endflag != OPFLAG_RETURN_TO_START || nextdescnum == end)
					curdesc->flags |= endflag;
			}

			/* otherwise, do some analysis based on the next instruction */
			else
			{
				opcode_desc *scandesc = NULL;
				int scandescnum;

				/* if there are instructions between us and the next instruction, we must end our sequence here */
				for (scandescnum = descnum + 1; scandescnum < end; scandescnum++)
				{
					scandesc = drcfe->desc_array[scandescnum];
					if (scandesc != NULL || scandesc == nextdesc)
						break;
				}
				if (scandesc != nextdesc)
					curdesc->flags |= OPFLAG_END_SEQUENCE;

				/* if the next instruction is a branch target, mark this instruction as end of sequence */
				if (nextdesc->flags & OPFLAG_IS_BRANCH_TARGET)
					curdesc->flags |= OPFLAG_END_SEQUENCE;
			}

			/* if we exceed the maximum consecutive count, cut off the sequence */
			if (++consecutive >= drcfe->max_sequence)
				curdesc->flags |= OPFLAG_END_SEQUENCE;
			if (curdesc->flags & OPFLAG_END_SEQUENCE)
				consecutive = 0;

			/* if this is the end of a sequence, work backwards */
			if (curdesc->flags & OPFLAG_END_SEQUENCE)
			{
				UINT32 reqmask[4] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
				int backdesc;

				/* figure out which registers we *must* generate, assuming at the end all must be */
				if (seqstart != -1)
					for (backdesc = descnum; backdesc != seqstart - 1; backdesc--)
						if (drcfe->desc_array[backdesc] != NULL)
							accumulate_required_backwards(drcfe->desc_array[backdesc], reqmask);

				/* reset the register states */
				seqstart = -1;
			}

			/* if we have instructions remaining to be skipped, and this instruction is a branch target */
			/* belay the skip order */
			if (skipsleft > 0 && (curdesc->flags & OPFLAG_IS_BRANCH_TARGET))
				skipsleft = 0;

			/* if we're not getting skipped, add us to the end of the list and clear our array slot */
			if (skipsleft == 0)
			{
				*tailptr = curdesc;
				tailptr = &curdesc->next;
				curdesc->prev = prev;
				prev = curdesc;
			}
			else
				desc_free(drcfe, curdesc);

			/* if the current instruction starts skipping, reset our skip count */
			/* otherwise, just decrement */
			if (curdesc->skipslots > 0)
				skipsleft = curdesc->skipslots;
			else if (skipsleft > 0)
				skipsleft--;
		}

	/* zap the array */
	memset(&drcfe->desc_array[start], 0, (end - start) * sizeof(drcfe->desc_array[0]));

	/* return the final tailptr */
	return tailptr;
}


/*-------------------------------------------------
    accumulate_required_backwards - recursively
    accumulate live register liveness information
    walking in a backwards direction
-------------------------------------------------*/

static void accumulate_required_backwards(opcode_desc *desc, UINT32 *reqmask)
{
	/* recursively handle delay slots */
	if (desc->delay != NULL)
		accumulate_required_backwards(desc->delay, reqmask);

	/* if this is a branch, we have to reset our requests */
	if (desc->flags & OPFLAG_IS_BRANCH)
		reqmask[0] = reqmask[1] = reqmask[2] = reqmask[3] = 0xffffffff;

	/* determine the required registers */
	desc->regreq[0] = desc->regout[0] & reqmask[0];
	desc->regreq[1] = desc->regout[1] & reqmask[1];
	desc->regreq[2] = desc->regout[2] & reqmask[2];
	desc->regreq[3] = desc->regout[3] & reqmask[3];

	/* any registers modified by this instruction aren't required upstream until referenced */
	reqmask[0] &= ~desc->regout[0];
	reqmask[1] &= ~desc->regout[1];
	reqmask[2] &= ~desc->regout[2];
	reqmask[3] &= ~desc->regout[3];

	/* any registers required by this instruction now get marked required */
	reqmask[0] |= desc->regin[0];
	reqmask[1] |= desc->regin[1];
	reqmask[2] |= desc->regin[2];
	reqmask[3] |= desc->regin[3];
}


/*-------------------------------------------------
    release_descriptions - release any
    descriptions we've allocated back to the
    free list
------------------------------------------------*/

static void release_descriptions(drcfe_state *drcfe, opcode_desc *desc)
{
	/* loop while we still have valid entries */
	while (desc != NULL)
	{
		opcode_desc *freeme = desc;

		/* recursively release delay slots */
		if (desc->delay != NULL)
			release_descriptions(drcfe, desc->delay);
		desc = desc->next;
		desc_free(drcfe, freeme);
	}
}
