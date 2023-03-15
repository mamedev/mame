// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * National Semiconductor 32382 Memory Management Unit.
 *
 * Sources:
 *   - http://bitsavers.org/components/national/_dataBooks/1988_National_Series_32000_Microprocessors_Databook.pdf
 *
 * TODO:
 *   - tlb
 *   - breakpoints
 *   - cycles
 *   - fast status protocol
 */

#include "emu.h"
#include "ns32382.h"

#define LOG_GENERAL   (1U << 0)
#define LOG_TRANSLATE (1U << 1)

//#define VERBOSE (LOG_GENERAL|LOG_TRANSLATE)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NS32382, ns32382_device, "ns32382", "National Semiconductor 32382 Memory Management Unit")

enum state : unsigned
{
	IDLE      = 0,
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
	BAR   = 0x0, // breakpoint address register
	BMR   = 0x2, // breakpoint mask register
	BDR   = 0x3, // breakpoint data register
	BEAR  = 0x6, // bus error address register
	MCR   = 0x9, // memory management control register
	MSR   = 0xa, // memory management status register
	TEAR  = 0xb, // translation exception address register
	PTB0  = 0xc, // page table base register 0
	PTB1  = 0xd, // page table base register 1
	IVAR0 = 0xe, // invalidate virtual address register 0
	IVAR1 = 0xf, // invalidate virtual address register 1
};

enum mcr_mask : u32
{
	MCR_TU  = 0x00000001, // translate user-mode
	MCR_TS  = 0x00000002, // translate supervisor-mode
	MCR_DS  = 0x00000004, // dual-space translation
	MCR_AO  = 0x00000008, // access level override
	MCR_BR  = 0x00000010, // break on read
	MCR_BW  = 0x00000020, // break on write
	MCR_BE  = 0x00000040, // break on execution
	MCR_BAS = 0x00000080, // breakpoint address space

	MCR_WM  = 0x000000ff,
};

enum msr_mask : u32
{
	MSR_TEX = 0x00000003, // translation exception
	MSR_DDT = 0x00000004, // data direction (translate exception)
	MSR_UST = 0x00000008, // user/supervisor
	MSR_STT = 0x000000f0, // cpu status
	MSR_BP  = 0x00000200, // break
	MSR_CE  = 0x00000400, // cpu error
	MSR_ME  = 0x00000800, // mmu error
	MSR_DDE = 0x00001000, // data direction (bus error)
	MSR_USE = 0x00002000, // user/supervisor
	MSR_STE = 0x0003c000, // cpu status

	MSR_WM  = 0x0003feff,
};

enum msr_tex_mask : u32
{
	TEX_IL1 = 0x00000001, // first level pte invalid
	TEX_IL2 = 0x00000002, // second level pte invalid
	TEX_PL  = 0x00000003, // protection violation
};

enum ptb_mask : u32
{
	PTB_AB = 0xfffff000, // address bits
};

enum pte_mask : u32
{
	PTE_V   = 0x00000001, // valid
	PTE_PL  = 0x00000006, // protection level
	PTE_CI  = 0x00000040, // (level 2 only) cache inhibit
	PTE_R   = 0x00000080, // referenced
	PTE_M   = 0x00000100, // modified
	PTE_USR = 0x00000e00, // user
	PTE_PFN = 0xfffff000, // page frame number
};

enum pte_pl_mask : u32
{
	PL_SRO = 0x00000000, // supervisor read only
	PL_SRW = 0x00000002, // supervisor read write
	PL_URO = 0x00000004, // user read only
	PL_URW = 0x00000006, // user read write
};

enum va_mask : u32
{
	VA_INDEX1 = 0xffc00000,
	VA_INDEX2 = 0x003ff000,
	VA_OFFSET = 0x00000fff,
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

ns32382_device::ns32382_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NS32382, tag, owner, clock)
	, ns32000_fast_slave_interface(mconfig, *this)
	, ns32000_mmu_interface(mconfig, *this)
{
}

void ns32382_device::device_start()
{
	save_item(NAME(m_bar));
	save_item(NAME(m_bdr));
	save_item(NAME(m_bear));
	save_item(NAME(m_bmr));
	save_item(NAME(m_mcr));
	save_item(NAME(m_msr));
	save_item(NAME(m_ptb));
	save_item(NAME(m_tear));

	save_item(NAME(m_opword));
	save_item(STRUCT_MEMBER(m_op, expected));
	save_item(STRUCT_MEMBER(m_op, issued));
	save_item(STRUCT_MEMBER(m_op, value));
	save_item(NAME(m_status));

	save_item(NAME(m_state));
	save_item(NAME(m_tcy));
}

void ns32382_device::device_reset()
{
	m_mcr = 0;
	m_msr = 0;

	m_state = IDLE;
}

void ns32382_device::state_add(device_state_interface &parent, int &index)
{
	parent.state_add(index++, "MCR", m_mcr).formatstr("%02X");
	parent.state_add(index++, "MSR", m_msr).formatstr("%08X");
}

u32 ns32382_device::read_st(int *icount)
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

	logerror("read_st protocol error reading status (%s)\n", machine().describe_context());
	return 0;
}

u32 ns32382_device::read()
{
	if (m_state == RESULT && m_op[2].issued < m_op[2].expected)
	{
		u32 const data = u32(m_op[2].value >> (m_op[2].issued * 8));
		LOG("read_op dword %d data 0x%08x (%s)\n", m_op[2].issued >> 2, data, machine().describe_context());

		m_op[2].issued += 4;

		if (m_op[2].issued == m_op[2].expected)
		{
			LOG("read_op last result dword issued\n");
			m_state = IDLE;
		}

		return data;
	}

	logerror("read_op protocol error reading result dword (%s)\n", machine().describe_context());
	return 0;
}

void ns32382_device::write(u32 data)
{
	switch (m_state)
	{
	case IDLE:
		if (BIT(data, 24, 8) == FORMAT_14)
		{
			LOG("write_fast match 0x%08x (%s)\n", data, machine().describe_context());

			m_opword = swapendian_int16(data >> 8);
			m_tcy = 0;

			// initialize operands
			for (operand &op : m_op)
			{
				op.expected = 0;
				op.issued = 0;
				op.value = 0;
			}

			// format 14: xxxx xsss s0oo ooii 0001 1110
			unsigned const size = m_opword & 3;

			switch (BIT(m_opword, 2, 4))
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

			// insert dword into operand value
			op.value |= u64(data) << (op.issued * 8);
			op.issued += 4;
		}
		else
			logerror("write_fast protocol error unexpected operand 0x%08x (%s)\n", data, machine().describe_context());
		break;
	}

	// start execution when all operands are available
	if (m_state == OPERAND && m_op[0].issued >= m_op[0].expected && m_op[1].issued >= m_op[1].expected)
		execute();
}

void ns32382_device::execute()
{
	m_status = 0;

	// format 14: xxxx xsss s0oo ooii 0001 1110
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
		case BAR: m_bar = m_op[0].value; break;
		case BMR: m_bmr = m_op[0].value; break;
		case MCR: m_mcr = m_op[0].value; break;
		case MSR: m_msr = m_op[0].value; break;
		case TEAR: m_tear = m_op[0].value; break;
		case PTB0: m_ptb[0] = m_op[0].value & PTB_AB; break;
		case PTB1: m_ptb[1] = m_op[0].value & PTB_AB; break;
		case IVAR0: break; // TODO: invalidate tlb entry
		case IVAR1: break; // TODO: invalidate tlb entry
		default:
			logerror("lmr unknown register %d (%s)\n", quick, machine().describe_context());
			break;
		}
		m_tcy = 30;
		break;
	case 3: // smr
		switch (quick)
		{
		case BAR: m_op[2].value = m_bar; break;
		case BMR: m_op[2].value = m_bmr; break;
		case BDR: m_op[2].value = m_bdr; break;
		case BEAR: m_op[2].value = m_bear; break;
		case MCR: m_op[2].value = m_mcr & MCR_WM; break;
		case MSR: m_op[2].value = m_msr & MSR_WM; break;
		case TEAR: m_op[2].value = m_tear; break;
		case PTB0: m_op[2].value = m_ptb[0]; break;
		case PTB1: m_op[2].value = m_ptb[1]; break;
		default:
			logerror("smr unknown register %d (%s)\n", quick, machine().describe_context());
			break;
		}
		m_tcy = 25;
		break;
	}

	// exceptions suppress result issue
	if (m_status & SLAVE_Q)
		m_op[2].expected = 0;

	if (m_state == OPERAND)
		m_state = STATUS;
}

ns32382_device::translate_result ns32382_device::translate(address_space &space, unsigned st, u32 &address, bool user, bool write, bool pfs, bool debug)
{
	// check translation required
	if ((!(m_mcr & MCR_TU) && user) || (!(m_mcr & MCR_TS) && !user))
		return COMPLETE;

	// treat WRVAL as write
	write |= m_state == WRVAL;

	bool const address_space = (m_mcr & MCR_DS) && user;
	unsigned const access_level = (user && !(m_mcr & MCR_AO))
		? ((write || st == ST_RMW) ? PL_URW : PL_URO) : ((write || st == ST_RMW) ? PL_SRW : PL_SRO);

	if (m_state == IDLE && !debug)
		m_msr &= ~(MSR_STT | MSR_UST | MSR_DDT | MSR_TEX);

	// read level 1 page table entry
	u32 const pte1_address = m_ptb[address_space] | ((address & VA_INDEX1) >> 20);
	u32 const pte1 = space.read_dword(pte1_address);
	LOGMASKED(LOG_TRANSLATE, "translate level 1 page table address 0x%06x entry 0x%08x\n", pte1_address, pte1);

	if (access_level > (pte1 & PTE_PL) || !(pte1 & PTE_V))
	{
		if (m_state == IDLE && !debug)
		{
			m_msr |= ((st & 15) << 4) | (user ? MSR_UST : 0) | (write ? MSR_DDT : 0);
			if (access_level > (pte1 & PTE_PL))
				m_msr |= TEX_PL;
			else if (!(pte1 & PTE_V))
				m_msr |= TEX_IL1;

			m_tear = address;
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

		LOGMASKED(LOG_TRANSLATE, "translate level 1 abort address 0x%08x\n", address);
		return ABORT;
	}

	// set referenced
	if (!(pte1 & PTE_R) && !debug)
		space.write_dword(pte1_address, pte1 | PTE_R);

	// read level 2 page table entry
	u32 const pte2_address = (pte1 & PTE_PFN) | ((address & VA_INDEX2) >> 10);
	u32 const pte2 = space.read_dword(pte2_address);
	LOGMASKED(LOG_TRANSLATE, "translate level 2 page table address 0x%06x entry 0x%08x\n", pte2_address, pte2);

	if (access_level > (pte2 & PTE_PL) || !(pte2 & PTE_V))
	{
		if (m_state == IDLE && !debug)
		{
			m_msr |= ((st & 15) << 4) | (user ? MSR_UST : 0) | (write ? MSR_DDT : 0);
			if (access_level > (pte2 & PTE_PL))
				m_msr |= TEX_PL;
			else if (!(pte2 & PTE_V))
				m_msr |= TEX_IL2;

			m_tear = address;
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
			LOGMASKED(LOG_TRANSLATE, "translate level 2 abort address 0x%08x\n", address);
			return ABORT;
		}
	}

	// set modified and referenced
	if ((!(pte2 & PTE_R) || (write && !(pte2 & PTE_M))) && !debug)
		space.write_dword(pte2_address, pte2 | (write ? PTE_M : 0) | PTE_R);

	address = (pte2 & PTE_PFN) | (address & VA_OFFSET);
	LOGMASKED(LOG_TRANSLATE, "translate complete 0x%08x\n", address);

	return COMPLETE;
}
