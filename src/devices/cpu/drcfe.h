// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcfe.h

    Generic dynamic recompiler frontend structures and utilities.

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
#ifndef MAME_CPU_DRCFE_H
#define MAME_CPU_DRCFE_H

#pragma once

#include <bitset>
#include <type_traits>
#include <vector>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// this defines a branch targetpc that is dynamic at runtime
constexpr offs_t BRANCH_TARGET_DYNAMIC = ~offs_t(0);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// description of a given opcode
template <typename Impl, size_t N>
class opcode_desc_base
{
public:
	using regmask = std::bitset<N>;

	Impl *next() const { return m_next; }

	void set_is_unconditional_branch()   { m_flags.set(IS_UNCONDITIONAL_BRANCH); }
	void set_is_conditional_branch()     { m_flags.set(IS_CONDITIONAL_BRANCH); }
	void set_is_branch_target()          { m_flags.set(IS_BRANCH_TARGET); }
	void set_in_delay_slot()             { m_flags.set(IN_DELAY_SLOT); }
	void set_intrablock_branch()         { m_flags.set(INTRABLOCK_BRANCH); }
	void set_can_cause_exception()       { m_flags.set(CAN_CAUSE_EXCEPTION); }
	void set_will_cause_exception()      { m_flags.set(WILL_CAUSE_EXCEPTION); }
	void set_validate_tlb()              { m_flags.set(VALIDATE_TLB); }
	void set_compiler_page_fault()       { m_flags.set(COMPILER_PAGE_FAULT); }
	void set_invalid_opcode()            { m_flags.set(INVALID_OPCODE); }
	void set_virtual_noop()              { m_flags.set(VIRTUAL_NOOP); }
	void set_redispatch()                { m_flags.set(REDISPATCH); }
	void set_return_to_start()           { m_flags.set(RETURN_TO_START); }
	void set_end_sequence()              { m_flags.set(END_SEQUENCE); }

	bool is_unconditional_branch() const { return m_flags[IS_UNCONDITIONAL_BRANCH]; }
	bool is_conditional_branch() const   { return m_flags[IS_CONDITIONAL_BRANCH]; }
	bool is_branch() const               { return is_unconditional_branch() || is_conditional_branch(); }
	bool is_branch_target() const        { return m_flags[IS_BRANCH_TARGET]; }
	bool in_delay_slot() const           { return m_flags[IN_DELAY_SLOT]; }
	bool intrablock_branch() const       { return m_flags[INTRABLOCK_BRANCH]; }
	bool can_cause_exception() const     { return m_flags[CAN_CAUSE_EXCEPTION]; }
	bool will_cause_exception() const    { return m_flags[WILL_CAUSE_EXCEPTION]; }
	bool validate_tlb() const            { return m_flags[VALIDATE_TLB]; }
	bool compiler_page_fault() const     { return m_flags[COMPILER_PAGE_FAULT]; }
	bool invalid_opcode() const          { return m_flags[INVALID_OPCODE]; }
	bool virtual_noop() const            { return m_flags[VIRTUAL_NOOP]; }
	bool redispatch() const              { return m_flags[REDISPATCH]; }
	bool return_to_start() const         { return m_flags[RETURN_TO_START]; }
	bool end_sequence() const            { return m_flags[END_SEQUENCE]; }

	// links to other descriptions
	Impl *          m_next;                 // pointer to next description
	Impl *          branch;                 // pointer back to branch description for delay slots
	simple_list<Impl> delay;                // pointer to delay slot description

	// information about the current PC
	offs_t          pc;                     // PC of this opcode
	offs_t          targetpc;               // target PC if we are a branch, or BRANCH_TARGET_DYNAMIC

	// information about this instruction's execution
	u8              length;                 // length in bytes of this opcode
	u8              delayslots;             // number of delay slots (for branches)
	u8              skipslots;              // number of skip slots (for branches)

	// register usage information
	regmask         regin;                  // input registers
	regmask         regout;                 // output registers
	regmask         regreq;                 // required output registers

protected:
	enum
	{
		// opcode branch flags
		IS_UNCONDITIONAL_BRANCH = 0,    // instruction is unconditional branch
		IS_CONDITIONAL_BRANCH,          // instruction is conditional branch
		IS_BRANCH_TARGET,               // instruction is the target of a branch
		IN_DELAY_SLOT,                  // instruction is in the delay slot of a branch
		INTRABLOCK_BRANCH,              // instruction branches within the block

		// opcode exception flags
		CAN_CAUSE_EXCEPTION,            // instruction may generate exception
		WILL_CAUSE_EXCEPTION,           // instruction will generate exception

		// opcode virtual->physical translation flags
		VALIDATE_TLB,                   // instruction must validate TLB before execution
		COMPILER_PAGE_FAULT,            // compiler hit a page fault when parsing

		// opcode flags
		INVALID_OPCODE,                 // instruction is invalid
		VIRTUAL_NOOP,                   // instruction is a virtual no-op

		// opcode sequence flow flags
		REDISPATCH,                     // instruction must redispatch after completion
		RETURN_TO_START,                // instruction must jump back to the beginning after completion
		END_SEQUENCE,                   // this is the last instruction in a sequence

		FLAG_COUNT
	};

	void reset(offs_t curpc, bool in_delay_slot)
	{
		m_next = nullptr;
		branch = nullptr;
		delay.reset();
		pc = curpc;
		targetpc = BRANCH_TARGET_DYNAMIC;
		length = 0;
		delayslots = 0;
		skipslots = 0;
		regin.reset();
		regout.reset();
		regreq.reset();
		m_flags.reset();

		// set the delay slot flag
		if (in_delay_slot)
			set_in_delay_slot();
	}

	// TODO: make these constexpr when GCC/GNU libstdc++ catch up
	template <size_t Start, size_t Width>
	static std::enable_if_t<Width <= (sizeof(unsigned long) * 8), unsigned long> regmask_field(regmask const &r)
	{
		static_assert((Width > 0) && ((Start + Width) <= N));
		return ((r << (N - Start - Width)) >> (N - Width)).to_ulong();
	}
	template <size_t Start, size_t Width>
	static std::enable_if_t<(Width > (sizeof(unsigned long) * 8)) && (Width <= (sizeof(unsigned long long) * 8)), unsigned long long> regmask_field(regmask const &r)
	{
		static_assert((Width > 0) && ((Start + Width) <= N));
		return ((r << (N - Start - Width)) >> (N - Width)).to_ullong();
	}

	std::bitset<FLAG_COUNT> m_flags;
};


// DRC frontend state
template <typename Desc>
class drc_frontend_base
{
public:
	// construction/destruction
	drc_frontend_base(offs_t pageshift, u32 window_start, u32 window_end, u32 max_sequence);
	~drc_frontend_base();

	// get last opcode of block
	Desc const *get_last() { return m_desc_live_list.last(); }

protected:
	// describe a block
	template <typename T>
	Desc const *do_describe_code(T && describe, offs_t startpc);

private:
	// internal helpers
	template <typename T>
	Desc *describe_one(T &&describe, offs_t curpc, Desc const *prevdesc, bool in_delay_slot = false);
	void build_sequence(int start, int end, bool redispatch);
	void accumulate_required_backwards(Desc &desc, typename Desc::regmask &reqmask);
	void release_descriptions();

	// configuration parameters
	u32 const           m_window_start;             // code window start offset = startpc - window_start
	u32 const           m_window_end;               // code window end offset = startpc + window_end
	u32 const           m_max_sequence;             // maximum instructions to include in a sequence

	// CPU parameters
	offs_t const        m_pageshift;                // shift to convert address to a page index

	// opcode descriptor arrays
	simple_list<Desc>   m_desc_live_list;           // list of live descriptions
	fixed_allocator<Desc> m_desc_allocator;         // fixed allocator for descriptions
	std::vector<Desc *> m_desc_array;               // array of descriptions in PC order
};

#endif // MAME_CPU_DRCFE_H
