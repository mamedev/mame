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

#pragma once

#ifndef __DRCFE_H__
#define __DRCFE_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// this defines a branch targetpc that is dynamic at runtime
const offs_t BRANCH_TARGET_DYNAMIC = ~0;


// opcode branch flags
const UINT32 OPFLAG_IS_UNCONDITIONAL_BRANCH = 0x00000001;       // instruction is unconditional branch
const UINT32 OPFLAG_IS_CONDITIONAL_BRANCH   = 0x00000002;       // instruction is conditional branch
const UINT32 OPFLAG_IS_BRANCH               = (OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_IS_CONDITIONAL_BRANCH);
const UINT32 OPFLAG_IS_BRANCH_TARGET        = 0x00000004;       // instruction is the target of a branch
const UINT32 OPFLAG_IN_DELAY_SLOT           = 0x00000008;       // instruction is in the delay slot of a branch
const UINT32 OPFLAG_INTRABLOCK_BRANCH       = 0x00000010;       // instruction branches within the block

// opcode exception flags
const UINT32 OPFLAG_CAN_TRIGGER_SW_INT      = 0x00000020;       // instruction can trigger a software interrupt
const UINT32 OPFLAG_CAN_EXPOSE_EXTERNAL_INT = 0x00000040;       // instruction can expose an external interrupt
const UINT32 OPFLAG_CAN_CAUSE_EXCEPTION     = 0x00000080;       // instruction may generate exception
const UINT32 OPFLAG_WILL_CAUSE_EXCEPTION    = 0x00000100;       // instruction will generate exception
const UINT32 OPFLAG_PRIVILEGED              = 0x00000200;       // instruction is privileged

// opcode virtual->physical translation flags
const UINT32 OPFLAG_VALIDATE_TLB            = 0x00000400;       // instruction must validate TLB before execution
const UINT32 OPFLAG_MODIFIES_TRANSLATION    = 0x00000800;       // instruction modifies the TLB
const UINT32 OPFLAG_COMPILER_PAGE_FAULT     = 0x00001000;       // compiler hit a page fault when parsing
const UINT32 OPFLAG_COMPILER_UNMAPPED       = 0x00002000;       // compiler hit unmapped memory when parsing

// opcode flags
const UINT32 OPFLAG_INVALID_OPCODE          = 0x00004000;       // instruction is invalid
const UINT32 OPFLAG_VIRTUAL_NOOP            = 0x00008000;       // instruction is a virtual no-op

// opcode sequence flow flags
const UINT32 OPFLAG_REDISPATCH              = 0x00010000;       // instruction must redispatch after completion
const UINT32 OPFLAG_RETURN_TO_START         = 0x00020000;       // instruction must jump back to the beginning after completion
const UINT32 OPFLAG_END_SEQUENCE            = 0x00040000;       // this is the last instruction in a sequence
const UINT32 OPFLAG_CAN_CHANGE_MODES        = 0x00080000;       // instruction can change modes

// execution semantics
const UINT32 OPFLAG_READS_MEMORY            = 0x00100000;       // instruction reads memory
const UINT32 OPFLAG_WRITES_MEMORY           = 0x00200000;       // instruction writes memory



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// description of a given opcode
struct opcode_desc
{
	opcode_desc *next() const { return m_next; }

	// links to other descriptions
	opcode_desc *   m_next;                 // pointer to next description
	opcode_desc *   branch;                 // pointer back to branch description for delay slots
	simple_list<opcode_desc> delay;         // pointer to delay slot description

	// information about the current PC
	offs_t          pc;                     // PC of this opcode
	offs_t          physpc;                 // physical PC of this opcode
	offs_t          targetpc;               // target PC if we are a branch, or BRANCH_TARGET_DYNAMIC

	// copy of up to 16 bytes of opcode
	union
	{
		UINT8       b[16];
		UINT16      w[8];
		UINT32      l[4];
		UINT64      q[2];
	} opptr;                                // pointer to opcode memory

	// information about this instruction's execution
	UINT8           length;                 // length in bytes of this opcode
	UINT8           delayslots;             // number of delay slots (for branches)
	UINT8           skipslots;              // number of skip slots (for branches)
	UINT32          flags;                  // OPFLAG_* opcode flags
	UINT32          cycles;                 // number of cycles needed to execute

	// register usage information
	UINT32          regin[4];               // input registers
	UINT32          regout[4];              // output registers
	UINT32          regreq[4];              // required output registers
};


// DRC frontend state
class drc_frontend
{
public:
	// construction/destruction
	drc_frontend(device_t &cpu, UINT32 window_start, UINT32 window_end, UINT32 max_sequence);
	virtual ~drc_frontend();

	// describe a block
	const opcode_desc *describe_code(offs_t startpc);

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) = 0;

private:
	// internal helpers
	opcode_desc *describe_one(offs_t curpc, const opcode_desc *prevdesc);
	void build_sequence(int start, int end, UINT32 endflag);
	void accumulate_required_backwards(opcode_desc &desc, UINT32 *reqmask);
	void release_descriptions();

	// configuration parameters
	UINT32              m_window_start;             // code window start offset = startpc - window_start
	UINT32              m_window_end;               // code window end offset = startpc + window_end
	UINT32              m_max_sequence;             // maximum instructions to include in a sequence

	// CPU parameters
	cpu_device &        m_cpudevice;                // CPU device object
	address_space &     m_program;                  // program address space for this CPU
	offs_t              m_pageshift;                // shift to convert address to a page index

	// opcode descriptor arrays
	simple_list<opcode_desc> m_desc_live_list;      // list of live descriptions
	fixed_allocator<opcode_desc> m_desc_allocator;  // fixed allocator for descriptions
	std::vector<opcode_desc *> m_desc_array;      // array of descriptions in PC order
};


#endif /* __DRCFE_H__ */
