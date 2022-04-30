// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * National Semiconductor 32082 Memory Management Unit.
 *
 * Sources:
 *   - http://bitsavers.org/components/national/_dataBooks/1988_National_Series_32000_Microprocessors_Databook.pdf
 *
 * TODO:
 *   - tlb
 *   - breakpoints
 */

#include "emu.h"
#include "ns32082.h"

#define LOG_GENERAL   (1U << 0)
#define LOG_TRANSLATE (1U << 1)

//#define VERBOSE (LOG_GENERAL|LOG_TRANSLATE)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NS32082, ns32082_device, "ns32082", "National Semiconductor 32082 Memory Management Unit")

enum state : unsigned
{
	IDLE      = 0,
	OPERATION = 1, // awaiting operation word
	OPERAND   = 2, // awaiting operands
	RDVAL     = 3, // rdval pending
	WRVAL     = 4, // wrval pending
	STATUS    = 5, // status word available
	RESULT    = 6, // result word available
};

enum idbyte : u8
{
	FORMAT_14 = 0x1e,
};

enum reg_mask : unsigned
{
	BPR0 = 0x0, // breakpoint register 0
	BPR1 = 0x1, // breakpoint register 1
	PF0  = 0x4, // program flow register 0 (removed at rev L)
	PF1  = 0x5, // program flow register 1 (removed at rev L)
	SC   = 0x8, // sequential count register (removed at rev L)
	MSR  = 0xa, // memory management status register
	BCNT = 0xb, // breakpoint counter register
	PTB0 = 0xc, // page table base register 0
	PTB1 = 0xd, // page table base register 1
	EIA  = 0xf, // error/invalidate address register
};

enum msr_mask : u32
{
	MSR_TE  = 0x00000001, // translation error
	MSR_R   = 0x00000002, // reset
	MSR_B   = 0x00000004, // break
	MSR_TET = 0x00000038, // translation error type
	MSR_BN  = 0x00000040, // breakpoint number
	MSR_ED  = 0x00000100, // error direction
	MSR_BD  = 0x00000200, // break direction
	MSR_EST = 0x00001c00, // error status
	MSR_BST = 0x0000e000, // breakpoint status
	MSR_TU  = 0x00010000, // translate user-mode addresses
	MSR_TS  = 0x00020000, // translate supervisor-mode addresses
	MSR_DS  = 0x00040000, // dual-space translation
	MSR_AO  = 0x00080000, // access level override
	MSR_BEN = 0x00100000, // breakpoint enable
	MSR_UB  = 0x00200000, // user-only breakpointing
	MSR_AI  = 0x00400000, // abort/interrupt
	MSR_FT  = 0x00800000, // flow trace (removed at rev L)
	MSR_UT  = 0x01000000, // user trace (removed at rev L)
	MSR_NT  = 0x02000000, // nonsequential trace (removed at rev L)

	MSR_ERC = 0x00000007,
	MSR_WM  = 0x03ff0000,
};

enum msr_tet_mask : u32
{
	TET_PL  = 0x00000008,
	TET_IL1 = 0x00000010,
	TET_IL2 = 0x00000020,
};

enum ptb_mask : u32
{
	PTB_AB = 0x00fffc00, // address bits
	PTB_MS = 0x80000000, // memory system
};

enum va_mask : u32
{
	VA_INDEX1 = 0x00ff0000,
	VA_INDEX2 = 0x0000fe00,
	VA_OFFSET = 0x000001ff,
};

enum pte_mask : u32
{
	PTE_V   = 0x00000001, // valid
	PTE_PL  = 0x00000006, // protection level
	PTE_R   = 0x00000008, // referenced
	PTE_M   = 0x00000010, // modified
	PTE_NSC = 0x00000060, // reserved
	PTE_USR = 0x00000180, // user bits
	PTE_PFN = 0x00fffe00, // page frame number
	PTE_MS  = 0x80000000, // memory system
};

enum pte_pl_mask : u32
{
	PL_SRO = 0x00000000, // supervisor read only
	PL_SRW = 0x00000002, // supervisor read write
	PL_URO = 0x00000004, // user read only
	PL_URW = 0x00000006, // user read write
};

enum eia_mask : u32
{
	EIA_VA = 0x00ffffff, // virtual address
	EIA_AS = 0x80000000, // address space
};

enum st_mask : unsigned
{
	ST_ICI = 0x0, // bus idle (CPU busy)
	ST_ICW = 0x1, // bus idle (CPU wait)
	ST_ISE = 0x3, // bus idle (slave execution)
	ST_IAM = 0x4, // interrupt acknowledge, master
	ST_IAC = 0x5, // interrupt acknowledge, cascaded
	ST_EIM = 0x6, // end of interrupt, master
	ST_EIC = 0x7, // end of interrupt, cascaded
	ST_SIF = 0x8, // sequential instruction fetch
	ST_NIF = 0x9, // non-sequential instruction fetch
	ST_ODT = 0xa, // operand data transfer
	ST_RMW = 0xb, // read RMW operand
	ST_EAR = 0xc, // effective address read
	ST_SOP = 0xd, // slave operand
	ST_SST = 0xe, // slave status
	ST_SID = 0xf, // slave ID
};

ns32082_device::ns32082_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NS32082, tag, owner, clock)
	, ns32000_slow_slave_interface(mconfig, *this)
	, ns32000_mmu_interface(mconfig, *this)
	, m_bpr{}
	, m_pf{}
	, m_sc(0)
	, m_msr(0)
	, m_bcnt(0)
	, m_ptb{}
	, m_eia(0)
{
}

void ns32082_device::device_start()
{
	save_item(NAME(m_bpr));
	save_item(NAME(m_pf));
	save_item(NAME(m_sc));
	save_item(NAME(m_msr));
	save_item(NAME(m_bcnt));
	save_item(NAME(m_ptb));
	save_item(NAME(m_eia));

	save_item(NAME(m_idbyte));
	save_item(NAME(m_opword));
	save_item(STRUCT_MEMBER(m_op, expected));
	save_item(STRUCT_MEMBER(m_op, issued));
	save_item(STRUCT_MEMBER(m_op, value));
	save_item(NAME(m_status));

	save_item(NAME(m_state));
	save_item(NAME(m_tcy));
}

void ns32082_device::device_reset()
{
	m_msr = 0;

	m_state = IDLE;
}

void ns32082_device::state_add(device_state_interface &parent, int &index)
{
	parent.state_add(index++, "MSR", m_msr).formatstr("%08X");
}

u16 ns32082_device::read_st(int *icount)
{
	if (m_state == STATUS)
	{
		m_state = (m_op[2].issued == m_op[2].expected) ? IDLE : RESULT;

		if (icount)
			*icount -= m_tcy;

		LOG("read_st status 0x%04x tcy %d %s (%s)\n", m_status, m_tcy,
			(m_state == RESULT ? "results pending" : "complete"), machine().describe_context());

		return m_status;
	}

	logerror("read_st protocol error reading status word (%s)\n", machine().describe_context());
	return 0;
}

u16 ns32082_device::read_op()
{
	if (m_state == RESULT && m_op[2].issued < m_op[2].expected)
	{
		u16 const data = u16(m_op[2].value >> (m_op[2].issued * 8));
		LOG("read_op word %d data 0x%04x (%s)\n", m_op[2].issued >> 1, data, machine().describe_context());

		m_op[2].issued += 2;

		if (m_op[2].issued == m_op[2].expected)
		{
			LOG("read_op last result word issued\n");
			m_state = IDLE;
		}

		return data;
	}

	logerror("read_op protocol error reading result word (%s)\n", machine().describe_context());
	return 0;
}

void ns32082_device::write_id(u16 data)
{
	bool const match = (data == FORMAT_14);

	if (match)
	{
		LOG("write_id match 0x%04x (%s)\n", data, machine().describe_context());
		m_state = OPERATION;
	}
	else
	{
		LOG("write_id ignore 0x%04x (%s)\n", data, machine().describe_context());
		m_state = IDLE;
	}

	m_idbyte = u8(data);
}

void ns32082_device::write_op(u16 data)
{
	switch (m_state)
	{
	case OPERATION:
		m_opword = swapendian_int16(data);
		LOG("write_op opword 0x%04x (%s)\n", m_opword, machine().describe_context());

		m_tcy = 0;

		// initialize operands
		for (operand &op : m_op)
		{
			op.expected = 0;
			op.issued = 0;
			op.value = 0;
		}

		// decode operands
		if (m_idbyte == FORMAT_14)
		{
			// format 14: xxxx xsss s0oo ooii 0001 1110
			unsigned const size = m_opword & 3;

			switch ((m_opword >> 2) & 15)
			{
			case 0: // rdval
				break;
			case 1: // wrval
				break;
			case 2: // lmr
				m_op[0].expected = size + 1;
				break;
			case 3: // smr
				m_op[2].expected = size + 1;
				break;
			}

			m_state = OPERAND;
		}
		break;

	case OPERAND:
		// check awaiting operand word
		if (m_op[0].issued < m_op[0].expected || m_op[1].issued < m_op[1].expected)
		{
			unsigned const n = (m_op[0].issued < m_op[0].expected) ? 0 : 1;
			operand &op = m_op[n];

			LOG("write_op op%d data 0x%04x (%s)\n", n, data, machine().describe_context());

			// insert word into operand value
			op.value |= u64(data) << (op.issued * 8);
			op.issued += 2;
		}
		else
			logerror("write_op protocol error unexpected operand word 0x%04x (%s)\n", data, machine().describe_context());
		break;
	}

	// start execution when all operands are available
	if (m_state == OPERAND && m_op[0].issued >= m_op[0].expected && m_op[1].issued >= m_op[1].expected)
		execute();
}

void ns32082_device::execute()
{
	m_status = 0;

	switch (m_idbyte)
	{
	case FORMAT_14:
		// format 14: xxxx xsss s0oo ooii 0001 1110
		{
			unsigned const quick = BIT(m_opword, 7, 4);

			switch (BIT(m_opword, 2, 4))
			{
			case 0: // rdval
				m_tcy = 21;
				m_state = RDVAL;
				break;
			case 1: // wrval
				m_tcy = 21;
				m_state = WRVAL;
				break;
			case 2: // lmr
				switch (quick)
				{
				case BPR0: m_bpr[0] = m_op[0].value & u32(0xfcffffff); break;
				case BPR1: m_bpr[1] = m_op[0].value & u32(0xf8ffffff); break;
				case PF0: m_pf[0] = m_op[0].value & u32(0x00ffffff); break;
				case PF1: m_pf[1] = m_op[0].value & u32(0x00ffffff); break;
				case SC: m_sc = m_op[0].value; break;
				case MSR: set_msr(m_op[0].value); break;
				case BCNT: m_bcnt = m_op[0].value & u32(0x00ffffff); break;
				case PTB0: m_ptb[0] = m_op[0].value & u32(0xfffffc00); break;
				case PTB1: m_ptb[1] = m_op[0].value & u32(0xfffffc00); break;
				case EIA: set_eia(m_op[0].value); break;
				default:
					logerror("lmr unknown register %d (%s)\n", quick, machine().describe_context());
					break;
				}
				m_tcy = 30;
				break;
			case 3: // smr
				switch (quick)
				{
				case BPR0: m_op[2].value = m_bpr[0]; break;
				case BPR1: m_op[2].value = m_bpr[1]; break;
				case PF0: m_op[2].value = m_pf[0]; break;
				case PF1: m_op[2].value = m_pf[1]; break;
				case SC: m_op[2].value = m_sc; break;
				case MSR: m_op[2].value = m_msr; break;
				case BCNT: m_op[2].value = m_bcnt; break;
				case PTB0: m_op[2].value = m_ptb[0]; break;
				case PTB1: m_op[2].value = m_ptb[1]; break;
				case EIA: m_op[2].value = m_eia; break;
				default:
					logerror("smr unknown register %d (%s)\n", quick, machine().describe_context());
					break;
				}
				m_tcy = 25;
				break;
			}
		}
		break;
	}

	// exceptions suppress result issue
	if (m_status & SLAVE_Q)
		m_op[2].expected = 0;

	if (m_state == OPERAND)
		m_state = STATUS;
}

void ns32082_device::set_msr(u32 data)
{
	if (data & MSR_R)
		m_msr &= ~(MSR_TE | MSR_B | MSR_TET | MSR_ED | MSR_BD | MSR_EST | MSR_BST);

	if ((m_msr ^ data) & (MSR_TS | MSR_TU))
		LOG("supervisor translation %s user translation %s (%s)\n",
			data & MSR_TS ? "enabled" : "disabled",
			data & MSR_TU ? "enabled" : "disabled", machine().describe_context());

	m_msr = (m_msr & ~MSR_WM) | (data & MSR_WM);
}

ns32082_device::translate_result ns32082_device::translate(address_space &space, unsigned st, u32 &address, bool user, bool write, bool pfs, bool debug)
{
	// update program flow trace state
	if (pfs && (m_msr & MSR_FT))
	{
		if (st == ST_NIF)
		{
			m_pf[1] = m_pf[0];
			m_pf[0] = address;

			m_sc = m_sc << 16;
		}

		m_sc++;
	}

	// check translation required
	if ((!(m_msr & MSR_TU) && user) || (!(m_msr & MSR_TS) && !user))
		return COMPLETE;

	// treat WRVAL as write
	write |= m_state == WRVAL;

	bool const address_space = (m_msr & MSR_DS) && user;
	unsigned const access_level = (user && !(m_msr & MSR_AO))
		? ((write || st == ST_RMW) ? PL_URW : PL_URO) : ((write || st == ST_RMW) ? PL_SRW : PL_SRO);

	u32 const ptb = ((m_ptb[address_space] & PTB_MS) >> 7) | (m_ptb[address_space] & PTB_AB);

	LOGMASKED(LOG_TRANSLATE, "translate address_space %d access_level %d page table 0x%08x address 0x%08x\n", address_space, access_level, ptb, address);

	if (m_state == IDLE && !debug)
		m_msr &= ~(MSR_EST | MSR_ED | MSR_TET | MSR_TE);

	// read level 1 page table entry
	u32 const pte1_address = ptb | ((address & VA_INDEX1) >> 14);
	u32 const pte1 = space.read_dword(pte1_address);
	LOGMASKED(LOG_TRANSLATE, "translate level 1 page table address 0x%06x entry 0x%08x\n", pte1_address, pte1);

	if (access_level > (pte1 & PTE_PL) || !(pte1 & PTE_V))
	{
		if (m_state == IDLE && !debug)
		{
			m_msr |= (write ? 0 : MSR_ED) | ((st & 7) << 10) | MSR_TE;
			if (access_level > (pte1 & PTE_PL))
				m_msr |= TET_PL;
			if (!(pte1 & PTE_V))
				m_msr |= TET_IL1;

			m_eia = (address_space ? EIA_AS : 0) | (address & EIA_VA);
		}

		if (m_state == RDVAL || m_state == WRVAL)
		{
			if (pte1 & PTE_V)
			{
				m_state = STATUS;
				m_status |= SLAVE_F;

				return CANCEL;
			}
			else
				m_state = IDLE;
		}

		LOGMASKED(LOG_TRANSLATE, "translate level 1 abort eia 0x%08x\n", m_eia);
		return ABORT;
	}

	// set referenced
	if (!(pte1 & PTE_R) && !debug)
		space.write_word(pte1_address, u16(pte1 | PTE_R));

	// read level 2 page table entry
	u32 const pte2_address = ((pte1 & PTE_MS) >> 7) | (pte1 & PTE_PFN) | ((address & VA_INDEX2) >> 7);
	u32 const pte2 = space.read_dword(pte2_address);
	LOGMASKED(LOG_TRANSLATE, "translate level 2 page table address 0x%06x entry 0x%08x\n", pte2_address, pte2);

	if (access_level > (pte2 & PTE_PL) || !(pte2 & PTE_V))
	{
		if (m_state == IDLE && !debug)
		{
			m_msr |= (write ? 0 : MSR_ED) | ((st & 7) << 10) | MSR_TE;
			if (access_level > (pte2 & PTE_PL))
				m_msr |= TET_PL;
			if (!(pte2 & PTE_V))
				m_msr |= TET_IL2;

			m_eia = (address_space ? EIA_AS : 0) | (address & EIA_VA);
		}

		if (m_state == RDVAL || m_state == WRVAL)
		{
			m_state = STATUS;
			if (pte1 & PTE_V)
				m_status |= SLAVE_F;

			return CANCEL;
		}
		else
		{
			LOGMASKED(LOG_TRANSLATE, "translate level 2 abort eia 0x%08x\n", m_eia);
			return ABORT;
		}
	}

	// set modified and referenced
	if ((!(pte2 & PTE_R) || (write && !(pte2 & PTE_M))) && !debug)
		space.write_word(pte2_address, u16(pte2 | (write ? PTE_M : 0) | PTE_R));

	address = ((pte1 & PTE_MS) >> 7) | (pte2 & PTE_PFN) | (address & VA_OFFSET);
	LOGMASKED(LOG_TRANSLATE, "translate complete 0x%08x\n", address);

	return COMPLETE;
}
