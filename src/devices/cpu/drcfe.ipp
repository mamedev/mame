// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcfe.ipp

    Generic dynamic recompiler frontend structures and utilities.

****************************************************************************

    Future improvements/changes:

    * more aggressive handling of needed registers for conditional
        intrablock branches

***************************************************************************/
#ifndef MAME_CPU_DRCFE_IPP
#define MAME_CPU_DRCFE_IPP

#pragma once

#include "drcfe.h"

#include <algorithm>


//**************************************************************************
//  DRC FRONTEND
//**************************************************************************

//-------------------------------------------------
//  drc_frontend_base - constructor
//-------------------------------------------------

template <typename Desc>
drc_frontend_base<Desc>::drc_frontend_base(offs_t pageshift, u32 window_start, u32 window_end, u32 max_sequence)
	: m_window_start(window_start)
	, m_window_end(window_end)
	, m_max_sequence(max_sequence)
	, m_pageshift(pageshift)
	, m_desc_array(window_end + window_start + 2, nullptr)
{
}


//-------------------------------------------------
//  ~drc_frontend_base - destructor
//-------------------------------------------------

template <typename Desc>
drc_frontend_base<Desc>::~drc_frontend_base()
{
	// release any descriptions we've accumulated
	release_descriptions();
}


//-------------------------------------------------
//  describe_code - describe a sequence of code
//  that falls within the configured window
//  relative to the specified startpc
//-------------------------------------------------

template <typename Desc>
template <typename T>
const Desc *drc_frontend_base<Desc>::do_describe_code(T &&describe, offs_t startpc)
{
	constexpr u32 MAX_STACK_DEPTH = 100;

	// an entry that maps branches for our code walking
	struct pc_stack_entry
	{
		offs_t              targetpc;
		offs_t              srcpc;
	};

	// release any descriptions we've accumulated
	release_descriptions();

	// add the initial PC to the stack
	pc_stack_entry pcstack[MAX_STACK_DEPTH];
	pc_stack_entry *pcstackptr = &pcstack[0];
	pcstackptr->srcpc = 0;
	pcstackptr->targetpc = startpc;
	pcstackptr++;

	// loop while we still have a stack
	offs_t const minpc = startpc - (std::min)(m_window_start, startpc);
	offs_t const maxpc = startpc + (std::min)(m_window_end, 0xffffffff - startpc);
	while (pcstackptr != &pcstack[0])
	{
		// if we've already hit this PC, just mark it a branch target and continue
		pc_stack_entry *const curstack = --pcstackptr;
		Desc *curdesc = m_desc_array[curstack->targetpc - minpc];
		if (curdesc != nullptr)
		{
			curdesc->set_is_branch_target();

			// if the branch crosses a page boundary, mark the target as needing to revalidate
			if (m_pageshift != 0 && ((curstack->srcpc ^ curdesc->pc) >> m_pageshift) != 0)
			{
				curdesc->set_validate_tlb();
				curdesc->set_can_cause_exception();
			}

			// continue processing
			continue;
		}

		// loop until we exit the block
		for (offs_t curpc = curstack->targetpc; curpc >= minpc && curpc < maxpc && m_desc_array[curpc - minpc] == nullptr; curpc += m_desc_array[curpc - minpc]->length)
		{
			// allocate a new description and describe this instruction
			m_desc_array[curpc - minpc] = curdesc = describe_one(describe, curpc, curdesc);

			// first instruction in a sequence is always a branch target
			if (curpc == curstack->targetpc)
				curdesc->set_is_branch_target();

			// stop if we hit a page fault
			if (curdesc->compiler_page_fault())
				break;

			// if we are the first instruction in the whole window, we must validate the TLB
			if (curpc == startpc && m_pageshift != 0)
			{
				curdesc->set_validate_tlb();
				curdesc->set_can_cause_exception();
			}

			// if we are a branch within the block range, add the branch target to our stack
			if (curdesc->is_branch() && (curdesc->targetpc >= minpc) && (curdesc->targetpc < maxpc) && (pcstackptr < &pcstack[MAX_STACK_DEPTH]))
			{
				curdesc->set_intrablock_branch();
				pcstackptr->srcpc = curdesc->pc;
				pcstackptr->targetpc = curdesc->targetpc;
				pcstackptr++;
			}

			// if we're done, we're done
			if (curdesc->end_sequence())
				break;
		}
	}

	// now build the list of descriptions in order
	// first from startpc -> maxpc, then from minpc -> startpc
	build_sequence(startpc - minpc, maxpc - minpc, true);
	build_sequence(minpc - minpc, startpc - minpc, false);
	return m_desc_live_list.first();
}


//-------------------------------------------------
//  describe_one - describe a single instruction,
//  recursively describing opcodes in delay
//  slots of branches as well
//-------------------------------------------------

template <typename Desc>
template <typename T>
Desc *drc_frontend_base<Desc>::describe_one(T &&describe, offs_t curpc, Desc const *prevdesc, bool in_delay_slot)
{
	// initialize the description
	Desc *const desc = m_desc_allocator.alloc();
	desc->reset(curpc, in_delay_slot);

	// call the callback to describe an instruction
	if (!describe(*desc, prevdesc))
	{
		desc->set_will_cause_exception();
		desc->set_invalid_opcode();
		return desc;
	}

	// validate the TLB if we are exactly at the start of a page, or if we cross a page boundary
	if (m_pageshift != 0 && (((curpc - 1) ^ (curpc + desc->length - 1)) >> m_pageshift) != 0)
	{
		desc->set_validate_tlb();
		desc->set_can_cause_exception();
	}

	// validate stuff
	assert((desc->length > 0) || desc->virtual_noop());

	// if we are a branch with delay slots, recursively walk those
	if (desc->is_branch())
	{
		// iterate over slots and describe them
		offs_t delaypc = curpc + desc->length;
		// If this is a delay slot it is the true branch fork and the pc should be the previous branch target
		if (desc->in_delay_slot())
		{
			if (prevdesc->targetpc != BRANCH_TARGET_DYNAMIC)
			{
				delaypc = prevdesc->targetpc;
				//printf("drc_frontend_base::describe_one Branch in delay slot. curpc=%08X delaypc=%08X\n", curpc, delaypc);
			}
			else
			{
				//printf("drc_frontend_base::describe_one Warning! Branch in delay slot of dynamic target. curpc=%08X\n", curpc);
			}
		}
		Desc *prev = desc;
		for (u8 slotnum = 0; slotnum < desc->delayslots; slotnum++)
		{
			// recursively describe the next instruction
			Desc *delaydesc = describe_one(describe, delaypc, prev, true);
			if (delaydesc == nullptr)
				break;
			desc->delay.append(*delaydesc);
			prev = desc;

			// set a pointer back to the original branch
			delaydesc->branch = desc;

			// stop if we hit a page fault
			if (delaydesc->compiler_page_fault())
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

template <typename Desc>
void drc_frontend_base<Desc>::build_sequence(int start, int end, bool redispatch)
{
	// iterate in order from start to end, picking up all non-NULL instructions
	int consecutive = 0;
	int seqstart = -1;
	int skipsleft = 0;
	for (int descnum = start; descnum < end; descnum++)
	{
		if (m_desc_array[descnum] != nullptr)
		{
			// determine the next instruction, taking skips into account
			Desc *curdesc = m_desc_array[descnum];
			int nextdescnum = descnum + curdesc->length;
			Desc *nextdesc = (nextdescnum < end) ? m_desc_array[nextdescnum] : nullptr;
			for (u8 skipnum = 0; skipnum < curdesc->skipslots && nextdesc != nullptr; skipnum++)
			{
				nextdescnum = nextdescnum + nextdesc->length;
				nextdesc = (nextdescnum < end) ? m_desc_array[nextdescnum] : nullptr;
			}

			// start a new sequence if we aren't already in the middle of one
			if (seqstart == -1 && skipsleft == 0)
			{
				// tag all start-of-sequence instructions as needing TLB verification
				curdesc->set_validate_tlb();
				curdesc->set_can_cause_exception();
				seqstart = descnum;
			}

			// if we are the last instruction, indicate end-of-sequence and redispatch
			if (nextdesc == nullptr)
			{
				curdesc->set_end_sequence();
				if (redispatch)
					curdesc->set_redispatch();
				else if (nextdescnum == end)
					curdesc->set_return_to_start();
			}

			// otherwise, do some analysis based on the next instruction
			else
			{
				// if there are instructions between us and the next instruction, we must end our sequence here
				int scandescnum;
				Desc *scandesc = nullptr;
				for (scandescnum = descnum + 1; scandescnum < end; scandescnum++)
				{
					scandesc = m_desc_array[scandescnum];
					if (scandesc != nullptr || scandesc == nextdesc)
						break;
				}
				if (scandesc != nextdesc)
					curdesc->set_end_sequence();

				// if the next instruction is a branch target, mark this instruction as end of sequence
				if (nextdesc->is_branch_target())
					curdesc->set_end_sequence();
			}

			// if we exceed the maximum consecutive count, cut off the sequence
			if (++consecutive >= m_max_sequence)
				curdesc->set_end_sequence();
			if (curdesc->end_sequence())
				consecutive = 0;

			// if this is the end of a sequence, work backwards
			if (curdesc->end_sequence())
			{
				// figure out which registers we *must* generate, assuming at the end all must be
				typename Desc::regmask reqmask;
				reqmask.set();
				if (seqstart != -1)
				{
					for (int backdesc = descnum; backdesc != seqstart - 1; backdesc--)
					{
						if (m_desc_array[backdesc] != nullptr)
							accumulate_required_backwards(*m_desc_array[backdesc], reqmask);
					}
				}

				// reset the register states
				seqstart = -1;
			}

			// if we have instructions remaining to be skipped, and this instruction is a branch target
			// belay the skip order
			if ((skipsleft > 0) && curdesc->is_branch_target())
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
	}

	// zap the array
	std::fill_n(&m_desc_array[start], end - start, nullptr);
}


//-------------------------------------------------
//  accumulate_required_backwards - recursively
//  accumulate live register liveness information
//  walking in a backwards direction
//-------------------------------------------------

template <typename Desc>
void drc_frontend_base<Desc>::accumulate_required_backwards(Desc &desc, typename Desc::regmask &reqmask)
{
	// recursively handle delay slots
	if (desc.delay.first() != nullptr)
		accumulate_required_backwards(*desc.delay.first(), reqmask);

	// if this is a branch, we have to reset our requests
	if (desc.is_branch())
		reqmask.set();

	// determine the required registers
	desc.regreq = desc.regout & reqmask;

	// any registers modified by this instruction aren't required upstream until referenced
	reqmask &= ~desc.regout;

	// any registers required by this instruction now get marked required
	reqmask |= desc.regin;
}


//-------------------------------------------------
//  release_descriptions - release any
//  descriptions we've allocated back to the
//  free list
//------------------------------------------------

template <typename Desc>
void drc_frontend_base<Desc>::release_descriptions()
{
	// release all delay slots first
	for (Desc *curdesc = m_desc_live_list.first(); curdesc != nullptr; curdesc = curdesc->next())
		m_desc_allocator.reclaim_all(curdesc->delay);

	// reclaim all the descriptors
	m_desc_allocator.reclaim_all(m_desc_live_list);
}

#endif // MAME_CPU_DRCFE_IPP
