// license:BSD-3-Clause
// copyright-holders:A. Lenard
/***************************************************************************

    Zilog Z8010 Memory Management Unit

***************************************************************************/

#include "emu.h"
#include "z8010.h"
#include "cpu/z8000/z8000.h"    // For ST_* status codes

//#define VERBOSE 1
#include "logmacro.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define SDR_MAX_SEGMENTS 64
#define SDR_SIZE (sizeof(sdr_entry) * SDR_MAX_SEGMENTS)

#define SDR_ENTRY(x) ((reinterpret_cast<sdr_entry*>(m_sdr.get()))[x])
#define SDR_FIELD(x,y) SDR_ENTRY(x).b[y]
#define CUR_SDR_FIELD SDR_FIELD(m_sar, m_dsc)

// device type definitions
DEFINE_DEVICE_TYPE(Z8010, z8010_device, "z8010", "Zilog Z8010 MMU")

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  INC_SAR - increase m_sar
//-------------------------------------------------

inline void z8010_device::INC_SAR()
{
	if (m_sar < (SDR_MAX_SEGMENTS-1))
		++m_sar;
}

//-------------------------------------------------
//  INC_DSC - increase m_dsc
//-------------------------------------------------

inline void z8010_device::INC_DSC(const uint8_t max_dsc = DSC_SDR_ATTR)
{
	if (m_dsc < max_dsc)
		++m_dsc;
}

//-------------------------------------------------
//  INC_DSC_SAR - increase m_dsc, then m_sar
//-------------------------------------------------

inline void z8010_device::INC_DSC_SAR(const uint8_t max_dsc = DSC_SDR_ATTR)
{
	if (m_dsc < max_dsc)
	{
		++m_dsc;
	}
	else if (m_sar < (SDR_MAX_SEGMENTS-1))
	{
		m_dsc = DSC_SDR_BAH;
		++m_sar;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z8010_device - constructor
//-------------------------------------------------

z8010_device::z8010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, Z8010, tag, owner, clock),
	m_out_segt(*this),
	m_mode(0),
	m_sar(0),
	m_dsc(0),
	m_vtype(0),
	m_vseg(0),
	m_vhoffs(0),
	m_bcs(0),
	m_iseg(0),
	m_ihoffs(0),
	m_if1_seg(0),
	m_if1_hoffs(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z8010_device::device_start()
{
	m_sdr = std::make_unique<uint8_t[]>(SDR_SIZE);
	save_pointer(NAME(m_sdr), SDR_SIZE);

	save_item(NAME(m_mode));
	save_item(NAME(m_sar));
	save_item(NAME(m_dsc));
	save_item(NAME(m_vtype));
	save_item(NAME(m_vseg));
	save_item(NAME(m_vhoffs));
	save_item(NAME(m_bcs));
	save_item(NAME(m_iseg));
	save_item(NAME(m_ihoffs));
	save_item(NAME(m_if1_seg));
	save_item(NAME(m_if1_hoffs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z8010_device::device_reset()
{
	memset(&m_sdr[0], 0, SDR_SIZE);

	m_mode = 0;
	m_sar = 0;
	m_dsc = 0;
	m_vtype = 0;
	m_vseg = 0;
	m_vhoffs = 0;
	m_bcs = 0;
	m_iseg = 0;
	m_ihoffs = 0;
	m_if1_seg = 0;
	m_if1_hoffs = 0;
}

//-------------------------------------------------
//  ifetch1_observed - bus snoop of an Instruction
//  Fetch 1 cycle
//-------------------------------------------------

// The MMU sits on the CPU's multiplexed address/data bus and sees the
// segment number, offset and status of every transaction, whether or not
// it qualifies to translate it.  It keeps a running latch of the last
// IFETCH1 (first instruction word fetch) address it observed; a violation
// freezes a copy into the instruction seg/offset status registers.
void z8010_device::ifetch1_observed(offs_t offset)
{
	m_if1_seg = (uint8_t)(offset >> 16) & 0x7f;
	m_if1_hoffs = (uint8_t)(offset >> 8);
}

//-------------------------------------------------
//  segtack_r - segment trap acknowledge
//-------------------------------------------------

uint16_t z8010_device::segtack_r()
{
	if (m_vtype)
	{
		m_out_segt(CLEAR_LINE);
		return (uint16_t)(1 << (8 + (m_mode & MODE_ID_MASK)));
	}
	else
		return 0;
}

//-------------------------------------------------
//  read - read register command
//-------------------------------------------------

uint8_t z8010_device::read(offs_t offset)
{
	LOG("%s read cmd: %02x\n", machine().describe_context(), offset);

	switch (offset)
	{
	/* control registers */
	case 0x00: return m_mode;
	case 0x01: return m_sar;
	case 0x20: return m_dsc;

	/* segment descriptor registers */
	case 0x08:
		{
		uint8_t value;
		if (m_dsc > DSC_SDR_BAL)
			m_dsc = DSC_SDR_BAH;
		value = CUR_SDR_FIELD;
		INC_DSC(DSC_SDR_BAL);
		return value;
		}
	case 0x09:
		m_dsc = DSC_SDR_LIMIT;
		return CUR_SDR_FIELD;
	case 0x0a:
		m_dsc = DSC_SDR_ATTR;
		return CUR_SDR_FIELD;
	case 0x0b:
		{
		uint8_t value = CUR_SDR_FIELD;
		INC_DSC();
		return value;
		}
	case 0x0c:
		{
		uint8_t value;
		if (m_dsc > DSC_SDR_BAL)
			m_dsc = DSC_SDR_BAH;
		value = CUR_SDR_FIELD;
		INC_DSC_SAR(DSC_SDR_BAL);
		return value;
		}
	case 0x0d:
		{
		uint8_t value;
		m_dsc = DSC_SDR_LIMIT;
		value = CUR_SDR_FIELD;
		INC_SAR();
		return value;
		}
	case 0x0e:
		{
		uint8_t value;
		m_dsc = DSC_SDR_ATTR;
		value = CUR_SDR_FIELD;
		INC_SAR();
		return value;
		}
	case 0x0f:
		{
		uint8_t value = CUR_SDR_FIELD;
		INC_DSC_SAR();
		return value;
		}

	/* status registers */
	case 0x02: return m_vtype;
	case 0x03: return m_vseg;
	case 0x04: return m_vhoffs;
	case 0x05: return m_bcs;
	case 0x06: return m_iseg;
	case 0x07: return m_ihoffs;
	default:
		logerror("%s ERROR invalid read cmd %02x\n", machine().describe_context(), offset);
		return 0xff;
	}
}

//-------------------------------------------------
//  write - write register command
//-------------------------------------------------

void z8010_device::write(offs_t offset, uint8_t data)
{
	LOG("%s write cmd: %02x = %02x\n", machine().describe_context(), offset, data);

	switch (offset)
	{
	/* control registers */
	case 0x00: m_mode = data; break;
	case 0x01: m_sar = data & 0x3f; m_dsc = 0; break;
	case 0x20: m_dsc = data & 0x03; break;

	/* segment descriptor registers */
	case 0x08:
		if (m_dsc > DSC_SDR_BAL)
			m_dsc = DSC_SDR_BAH;
		CUR_SDR_FIELD = data;
		INC_DSC(DSC_SDR_BAL);
		break;
	case 0x09:
		m_dsc = DSC_SDR_LIMIT;
		CUR_SDR_FIELD = data;
		break;
	case 0x0a:
		m_dsc = DSC_SDR_ATTR;
		CUR_SDR_FIELD = data;
		break;
	case 0x0b:
		CUR_SDR_FIELD = data;
		INC_DSC();
		break;
	case 0x0c:
		if (m_dsc > DSC_SDR_BAL)
			m_dsc = DSC_SDR_BAH;
		CUR_SDR_FIELD = data;
		INC_DSC_SAR(DSC_SDR_BAL);
		break;
	case 0x0d:
		m_dsc = DSC_SDR_LIMIT;
		CUR_SDR_FIELD = data;
		INC_SAR();
		break;
	case 0x0e:
		m_dsc = DSC_SDR_ATTR;
		CUR_SDR_FIELD = data;
		INC_SAR();
		break;
	case 0x0f:
		CUR_SDR_FIELD = data;
		INC_DSC_SAR();
		break;
	case 0x15:
		{
		for (int i = 0; i < SDR_MAX_SEGMENTS; i++)
		{
			uint8_t &value = SDR_ENTRY(i).attr;
			value = (data ? (value | SDR_ATTR_CPUI) :
							(value & ~SDR_ATTR_CPUI));
		}
		}
		break;
	case 0x16:
		{
		for (int i = 0; i < SDR_MAX_SEGMENTS; i++)
		{
			uint8_t &value = SDR_ENTRY(i).attr;
			value = (data ? (value | SDR_ATTR_DMAI) :
							(value & ~SDR_ATTR_DMAI));
		}
		}
		break;

	/* status registers */
	case 0x11: m_vtype = 0; break;
	case 0x13: m_vtype &= ~VTYPE_SWW; break;
	case 0x14: m_vtype &= ~VTYPE_FATL; break;
	default:
		logerror("%s ERROR invalid write cmd %02x = %02x\n", machine().describe_context(), offset, data);
		break;
	}
}

//-------------------------------------------------
//  translate - translate memory address
//-------------------------------------------------

bool z8010_device::translate(offs_t &offset, bool write, bool sys, bool dma, int st)
{
	bool nsup = true;
	bool exec;
	bool const side_effects = !machine().side_effects_disabled();

	st &= BCS_CPU_MASK;
	exec = (st == z8002_device::ST_IFETCH_N) || (st == z8002_device::ST_IFETCH_1);

	// The BCS register latches the Z-Bus pin levels: N/S~ is LOW in system mode and
	// R/W~ is LOW for a write, so both bits read back 0 for a system-mode write.
	// The violation status registers (VTR/seg/offset/BCS/instr) FREEZE once a
	// violation is latched (until VTR is cleared via reg 0x11) — that is their
	// diagnostic purpose.  So BCS tracks the bus live only while no violation is
	// pending; the violating cycle's own status is the last one stored.  (UC3003
	// TST01 validates BCS == 0x08 = ST_REQ_DATA after a provoked write violation.)
	if (side_effects && !m_vtype)
		m_bcs = ((!sys) * BCS_N_S) | ((!write) * BCS_R_W) | (st);

	if (!(m_mode & MODE_MSEN) ||                                        // Master enable?
		(((m_mode & MODE_URS) != 0) != BIT(offset, 22)) ||              // Upper range select?
		((m_mode & MODE_MST) && (((m_mode & MODE_NMS) == 0) != sys)))   // System/normal mode?
	{
		offset = 0;
		LOG("%s: INVALID MMU STATE! offset: %06x, mode: %02x, sys: %01x\n", machine().describe_context(), offset, m_mode, sys);
		return false;
	}

	if (m_mode & MODE_TRNS) // Is translation on?
	{
		uint8_t sn = ((uint8_t)(offset >> 16)) & 0x3f;
		uint8_t so = (uint8_t)(offset >> 8);
		sdr_entry &s = SDR_ENTRY(sn);
		bool is_stack = s.attr & SDR_ATTR_DIRW;

		// Check access violations.  The attribute bits and the VTR bits are NOT
		// 1:1 (VTR bit 2 is the length violation, which has no attribute), so map
		// each attribute to its violation-type bit explicitly.
		uint8_t viol = 0;
		if (write && (s.attr & SDR_ATTR_RD))   viol |= VTYPE_RDV;   // Read-only violation?
		if (!sys  && (s.attr & SDR_ATTR_SYS))  viol |= VTYPE_SYSV;  // System violation?
		if (!dma  && (s.attr & SDR_ATTR_CPUI)) viol |= VTYPE_CPUIV; // CPU-inhibit violation?
		if (!exec && (s.attr & SDR_ATTR_EXC))  viol |= VTYPE_EXCV;  // Execute-only violation?

		// Length violation?
		if (is_stack)
		{
			if (so < s.limit)
			{
				viol |= VTYPE_SLV;
			}
		}
		else if (so > s.limit)
		{
			viol |= VTYPE_SLV;
		}

		if (side_effects && (viol ||
			(dma && (s.attr & SDR_ATTR_DMAI)))) // DMA-inhibit violation?
		{
			nsup = false;
		}

		// Check write warnings
		if (side_effects && write && is_stack && (so == s.limit))
		{
			if (m_vtype == 0)   // Primary write warning?
				viol |= VTYPE_PWW;
			else if ((m_vtype < VTYPE_SWW) && sys)  // Secondary write warning?
				viol |= VTYPE_SWW;
			else if (!sys)  // Fatal?
			{
				nsup = false;
				viol |= VTYPE_FATL;
			}
		}

		if (side_effects && viol)
		{
			if (m_vtype && !(viol & VTYPE_SWW)) // Fatal?
			{
				// A violation while one is already latched records ONLY the fatal
				// condition — the new violation's type bits are not accumulated
				// (UC3003 TST01: RDV latched, then SYSV provoked -> VTR must show
				// RDV|FATL with the SYSV bit clear).
				viol = VTYPE_FATL;
			}

			m_vseg = sn;
			m_vhoffs = so;
			// Freeze the last IFETCH1 address observed on the bus into the
			// instruction seg/offset status registers (UC3003 TST01 validates
			// these against the PC of its deliberate violating write).
			m_iseg = m_if1_seg;
			m_ihoffs = m_if1_hoffs;

			// No trap if in DMA mode or repeated SWW and FATL violations
			if ((!dma) && ((m_vtype & viol) & (VTYPE_SWW | VTYPE_FATL)) == 0)
			{
				// Generate Segment Trap
				m_out_segt(ASSERT_LINE);
			}

			m_vtype |= viol;
		}

		if (side_effects && (!m_vtype || ((m_vtype >= VTYPE_PWW) && !(m_vtype & VTYPE_FATL))))
		{
			if (write)
			{
				s.attr |= SDR_ATTR_CHG;
			}
			else if (st == z8002_device::ST_IFETCH_1)
			{
				// Save high address of successfully fetched instruction
				m_iseg = sn;
				m_ihoffs = so;
			}

			s.attr |= SDR_ATTR_REF;
		}

		// Perform address translation
		offset &= 0xffff;
		offset += ((offs_t)(s.bah) << 16) | ((offs_t)(s.bal) << 8);

		LOG("MMU TRNS: new offs %06x, seg: %02x, viol: %02x, sup: %01x\n", offset, sn, viol, !nsup);
	}

	return nsup;
}
