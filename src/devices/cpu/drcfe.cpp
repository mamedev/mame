// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcfe.c

    Generic dynamic recompiler frontend structures and utilities.

****************************************************************************

    Future improvements/changes:

    * more aggressive handling of needed registers for conditional
        intrablock branches

***************************************************************************/

#include "emu.h"
#include "drcfe.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const UINT32 MAX_STACK_DEPTH = 100;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// an entry that maps branches for our code walking
struct pc_stack_entry
{
	offs_t              targetpc;
	offs_t              srcpc;
};



//**************************************************************************
//  DRC FRONTEND
//**************************************************************************

//-------------------------------------------------
//  drc_frontend - constructor
//-------------------------------------------------

drc_frontend::drc_frontend(device_t &cpu, UINT32 window_start, UINT32 window_end, UINT32 max_sequence)
	: m_window_start(window_start),
		m_window_end(window_end),
		m_max_sequence(max_sequence),
		m_cpudevice(downcast<cpu_device &>(cpu)),
		m_program(m_cpudevice.space(AS_PROGRAM)),
		m_pageshift(m_cpudevice.space_config(AS_PROGRAM)->m_page_shift),
		m_desc_array(window_end + window_start + 2, nullptr)
{
}


//-------------------------------------------------
//  ~drc_frontend - destructor
//-------------------------------------------------

drc_frontend::~drc_frontend()
{
	// release any descriptions we've accumulated
	release_descriptions();
}


//-------------------------------------------------
//  describe_code - describe a sequence of code
//  that falls within the configured window
//  relative to the specified startpc
//-------------------------------------------------

const opcode_desc *drc_frontend::describe_code(offs_t startpc)
{
	// release any descriptions we've accumulated
	release_descriptions();

	// add the initial PC to the stack
	pc_stack_entry pcstack[MAX_STACK_DEPTH];
	pc_stack_entry *pcstackptr = &pcstack[0];
	pcstackptr->srcpc = 0;
	pcstackptr->targetpc = startpc;
	pcstackptr++;

	// loop while we still have a stack
	offs_t minpc = startpc - MIN(m_window_start, startpc);
	offs_t maxpc = startpc + MIN(m_window_end, 0xffffffff - startpc);
	while (pcstackptr != &pcstack[0])
	{
		// if we've already hit this PC, just mark it a branch target and continue
		pc_stack_entry *curstack = --pcstackptr;
		opcode_desc *curdesc = m_desc_array[curstack->targetpc - minpc];
		if (curdesc != nullptr)
		{
			curdesc->flags |= OPFLAG_IS_BRANCH_TARGET;

			// if the branch crosses a page boundary, mark the target as needing to revalidate
			if (m_pageshift != 0 && ((curstack->srcpc ^ curdesc->pc) >> m_pageshift) != 0)
				curdesc->flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION;

			// continue processing
			continue;
		}

		// loop until we exit the block
		for (offs_t curpc = curstack->targetpc; curpc >= minpc && curpc < maxpc && m_desc_array[curpc - minpc] == nullptr; curpc += m_desc_array[curpc - minpc]->length)
		{
			// allocate a new description and describe this instruction
			m_desc_array[curpc - minpc] = curdesc = describe_one(curpc, curdesc);

			// first instruction in a sequence is always a branch target
			if (curpc == curstack->targetpc)
				curdesc->flags |= OPFLAG_IS_BRANCH_TARGET;

			// stop if we hit a page fault
			if (curdesc->flags & OPFLAG_COMPILER_PAGE_FAULT)
				break;

			// if we are the first instruction in the whole window, we must validate the TLB
			if (curpc == startpc && m_pageshift != 0)
				curdesc->flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION;

			// if we are a branch within the block range, add the branch target to our stack
			if ((curdesc->flags & OPFLAG_IS_BRANCH) && curdesc->targetpc >= minpc && curdesc->targetpc < maxpc && pcstackptr < &pcstack[MAX_STACK_DEPTH])
			{
				curdesc->flags |= OPFLAG_INTRABLOCK_BRANCH;
				pcstackptr->srcpc = curdesc->pc;
				pcstackptr->targetpc = curdesc->targetpc;
				pcstackptr++;
			}

			// if we're done, we're done
			if (curdesc->flags & OPFLAG_END_SEQUENCE)
				break;
		}
	}

	// now build the list of descriptions in order
	// first from startpc -> maxpc, then from minpc -> startpc
	build_sequence(startpc - minpc, maxpc - minpc, OPFLAG_REDISPATCH);
	build_sequence(minpc - minpc, startpc - minpc, OPFLAG_RETURN_TO_START);
	return m_desc_live_list.first();
}


//-------------------------------------------------
//  describe_one - describe a single instruction,
//  recursively describing opcodes in delay
//  slots of branches as well
//-------------------------------------------------

opcode_desc *drc_frontend::describe_one(offs_t curpc, const opcode_desc *prevdesc)
{
	// initialize the description
	opcode_desc *desc = m_desc_allocator.alloc();
	desc->m_next = nullptr;
	desc->branch = nullptr;
	desc->delay.reset();
	desc->pc = curpc;
	desc->physpc = curpc;
	desc->targetpc = BRANCH_TARGET_DYNAMIC;
	memset(&desc->opptr, 0x00, sizeof(desc->opptr));
	desc->length = 0;
	desc->delayslots = 0;
	desc->skipslots = 0;
	desc->flags = 0;
	desc->cycles = 0;
	memset(desc->regin, 0x00, sizeof(desc->regin));
	memset(desc->regout, 0x00, sizeof(desc->regout));
	memset(desc->regreq, 0x00, sizeof(desc->regreq));

	// call the callback to describe an instruction
	if (!describe(*desc, prevdesc))
	{
		desc->flags |= OPFLAG_WILL_CAUSE_EXCEPTION | OPFLAG_INVALID_OPCODE;
		return desc;
	}

	// validate the TLB if we are exactly at the start of a page, or if we cross a page boundary
	if (m_pageshift != 0 && (((curpc - 1) ^ (curpc + desc->length - 1)) >> m_pageshift) != 0)
		desc->flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION;

	// validate stuff
	assert(desc->length > 0 || (desc->flags & OPFLAG_VIRTUAL_NOOP) != 0);

	// if we are a branch with delay slots, recursively walk those
	if (desc->flags & OPFLAG_IS_BRANCH)
	{
		// iterate over slots and describe them
		offs_t delaypc = curpc + desc->length;
		opcode_desc *prev = desc;
		for (UINT8 slotnum = 0; slotnum < desc->delayslots; slotnum++)
		{
			// recursively describe the next instruction
			opcode_desc *delaydesc = describe_one(delaypc, prev);
			if (delaydesc == nullptr)
				break;
			desc->delay.append(*delaydesc);
			prev = desc;

			// set the delay slot flag and a pointer back to the original branch
			delaydesc->flags |= OPFLAG_IN_DELAY_SLOT;
			delaydesc->branch = desc;

			// stop if we hit a page fault
			if (delaydesc->flags & OPFLAG_COMPILER_PAGE_FAULT)
				break;

			// otherwise, advance
			delaypc += delaydesc->length;
		}
	}
	return desc;
}


//-------------------------------------------------
//  build_sequence - build an ordered sequence
//  of instructions
//-------------------------------------------------

void drc_frontend::build_sequence(int start, int end, UINT32 endflag)
{
	// iterate in order from start to end, picking up all non-NULL instructions
	int consecutive = 0;
	int seqstart = -1;
	int skipsleft = 0;
	for (int descnum = start; descnum < end; descnum++)
		if (m_desc_array[descnum] != nullptr)
		{
			// determine the next instruction, taking skips into account
			opcode_desc *curdesc = m_desc_array[descnum];
			int nextdescnum = descnum + curdesc->length;
			opcode_desc *nextdesc = (nextdescnum < end) ? m_desc_array[nextdescnum] : nullptr;
			for (UINT8 skipnum = 0; skipnum < curdesc->skipslots && nextdesc != nullptr; skipnum++)
			{
				nextdescnum = nextdescnum + nextdesc->length;
				nextdesc = (nextdescnum < end) ? m_desc_array[nextdescnum] : nullptr;
			}

			// start a new sequence if we aren't already in the middle of one
			if (seqstart == -1 && skipsleft == 0)
			{
				// tag all start-of-sequence instructions as needing TLB verification
				curdesc->flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION;
				seqstart = descnum;
			}

			// if we are the last instruction, indicate end-of-sequence and redispatch
			if (nextdesc == nullptr)
			{
				curdesc->flags |= OPFLAG_END_SEQUENCE;
				if (endflag != OPFLAG_RETURN_TO_START || nextdescnum == end)
					curdesc->flags |= endflag;
			}

			// otherwise, do some analysis based on the next instruction
			else
			{
				// if there are instructions between us and the next instruction, we must end our sequence here
				int scandescnum;
				opcode_desc *scandesc = nullptr;
				for (scandescnum = descnum + 1; scandescnum < end; scandescnum++)
				{
					scandesc = m_desc_array[scandescnum];
					if (scandesc != nullptr || scandesc == nextdesc)
						break;
				}
				if (scandesc != nextdesc)
					curdesc->flags |= OPFLAG_END_SEQUENCE;

				// if the next instruction is a branch target, mark this instruction as end of sequence
				if (nextdesc->flags & OPFLAG_IS_BRANCH_TARGET)
					curdesc->flags |= OPFLAG_END_SEQUENCE;
			}

			// if we exceed the maximum consecutive count, cut off the sequence
			if (++consecutive >= m_max_sequence)
				curdesc->flags |= OPFLAG_END_SEQUENCE;
			if (curdesc->flags & OPFLAG_END_SEQUENCE)
				consecutive = 0;

			// if this is the end of a sequence, work backwards
			if (curdesc->flags & OPFLAG_END_SEQUENCE)
			{
				// figure out which registers we *must* generate, assuming at the end all must be
				UINT32 reqmask[4] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
				if (seqstart != -1)
					for (int backdesc = descnum; backdesc != seqstart - 1; backdesc--)
						if (m_desc_array[backdesc] != nullptr)
							accumulate_required_backwards(*m_desc_array[backdesc], reqmask);

				// reset the register states
				seqstart = -1;
			}

			// if we have instructions remaining to be skipped, and this instruction is a branch target
			// belay the skip order
			if (skipsleft > 0 && (curdesc->flags & OPFLAG_IS_BRANCH_TARGET))
				skipsleft = 0;

			// if we're not getting skipped, add us to the end of the list and clear our array slot
			if (skipsleft == 0)
				m_desc_live_list.append(*curdesc);
			else
				m_desc_allocator.reclaim(*curdesc);

			// if the current instruction starts skipping, reset our skip count
			// otherwise, just decrement
			if (curdesc->skipslots > 0)
				skipsleft = curdesc->skipslots;
			else if (skipsleft > 0)
				skipsleft--;
		}

	// zap the array
	memset(&m_desc_array[start], 0, (end - start) * sizeof(m_desc_array[0]));
}


//-------------------------------------------------
//  accumulate_required_backwards - recursively
//  accumulate live register liveness information
//  walking in a backwards direction
//-------------------------------------------------

void drc_frontend::accumulate_required_backwards(opcode_desc &desc, UINT32 *reqmask)
{
	// recursively handle delay slots
	if (desc.delay.first() != nullptr)
		accumulate_required_backwards(*desc.delay.first(), reqmask);

	// if this is a branch, we have to reset our requests
	if (desc.flags & OPFLAG_IS_BRANCH)
		reqmask[0] = reqmask[1] = reqmask[2] = reqmask[3] = 0xffffffff;

	// determine the required registers
	desc.regreq[0] = desc.regout[0] & reqmask[0];
	desc.regreq[1] = desc.regout[1] & reqmask[1];
	desc.regreq[2] = desc.regout[2] & reqmask[2];
	desc.regreq[3] = desc.regout[3] & reqmask[3];

	// any registers modified by this instruction aren't required upstream until referenced
	reqmask[0] &= ~desc.regout[0];
	reqmask[1] &= ~desc.regout[1];
	reqmask[2] &= ~desc.regout[2];
	reqmask[3] &= ~desc.regout[3];

	// any registers required by this instruction now get marked required
	reqmask[0] |= desc.regin[0];
	reqmask[1] |= desc.regin[1];
	reqmask[2] |= desc.regin[2];
	reqmask[3] |= desc.regin[3];
}


//-------------------------------------------------
//  release_descriptions - release any
//  descriptions we've allocated back to the
//  free list
//------------------------------------------------

void drc_frontend::release_descriptions()
{
	// release all delay slots first
	for (opcode_desc *curdesc = m_desc_live_list.first(); curdesc != nullptr; curdesc = curdesc->next())
		m_desc_allocator.reclaim_all(curdesc->delay);

	// reclaim all the descriptors
	m_desc_allocator.reclaim_all(m_desc_live_list);
}
