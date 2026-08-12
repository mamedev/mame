// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*********************************************************************

    NCR 53C700/53C710 SCSI I/O Processors

    TODO:
    * Low-level register accesses
    * Remove arbitrary delays
    * Add unimplemented SCRIPTS opcodes

*********************************************************************/

#include "emu.h"
#include "53c7xx.h"



//**************************************************************************
//  DEBUGGERY
//**************************************************************************

#define LOG_UNHANDLED       (1U << 1)
#define LOG_HOST            (1U << 2)
#define LOG_STATE           (1U << 3)
#define LOG_SCRIPTS         (1U << 4)
#define VERBOSE             (0)

#include "logmacro.h"


//**************************************************************************
//  REGISTER CONSTANTS (INCOMPLETE)
//**************************************************************************

namespace {

constexpr uint8_t SCNTL0_TRG       = 0x01;
[[maybe_unused]] constexpr uint8_t SCNTL0_AAP = 0x02;
[[maybe_unused]] constexpr uint8_t SCNTL0_EPG = 0x04;
[[maybe_unused]] constexpr uint8_t SCNTL0_EPC = 0x08;
constexpr uint8_t SCNTL0_WATN      = 0x10;
constexpr uint8_t SCNTL0_START     = 0x20;
constexpr uint8_t SCNTL0_ARB_MASK  = 0x03;
constexpr uint8_t SCNTL0_ARB_SHIFT = 6;

constexpr uint8_t DSTAT_OPC  = 0x01;
[[maybe_unused]] constexpr uint8_t DSTAT_WTD = 0x02;
constexpr uint8_t DSTAT_SIR  = 0x04;
[[maybe_unused]] constexpr uint8_t DSTAT_SSI  = 0x08;
[[maybe_unused]] constexpr uint8_t DSTAT_ABRT = 0x10;
constexpr uint8_t DSTAT_DFE  = 0x80;

[[maybe_unused]] constexpr uint8_t SSTAT0_PAR = 0x01;
[[maybe_unused]] constexpr uint8_t SSTAT0_RST = 0x02;
[[maybe_unused]] constexpr uint8_t SSTAT0_UDC = 0x04;
[[maybe_unused]] constexpr uint8_t SSTAT0_SGE = 0x08;
[[maybe_unused]] constexpr uint8_t SSTAT0_SEL = 0x10;
constexpr uint8_t SSTAT0_STO = 0x20;
constexpr uint8_t SSTAT0_CMP = 0x40;
[[maybe_unused]] constexpr uint8_t SSTAT0_MA = 0x80;

[[maybe_unused]] constexpr uint8_t SSTAT1_SDP = 0x01;
[[maybe_unused]] constexpr uint8_t SSTAT1_RST = 0x02;
constexpr uint8_t SSTAT1_WOA = 0x04;
constexpr uint8_t SSTAT1_LOA = 0x08;
[[maybe_unused]] constexpr uint8_t SSTAT1_AIP = 0x10;
constexpr uint8_t SSTAT1_ORF = 0x20;
constexpr uint8_t SSTAT1_OLF = 0x40;
constexpr uint8_t SSTAT1_ILF = 0x80;

constexpr uint8_t CTEST4_NOTIME = 0x10;

constexpr uint8_t ISTAT_DIP  = 0x01;
constexpr uint8_t ISTAT_SIP  = 0x02;
[[maybe_unused]] constexpr uint8_t ISTAT_PRE = 0x04;
constexpr uint8_t ISTAT_CON  = 0x08;
constexpr uint8_t ISTAT_ABRT = 0x80;

constexpr uint8_t DCNTL_STD = 0x04;
constexpr uint8_t DCNTL_LLM = 0x08;
constexpr uint8_t DCNTL_SSM = 0x10;

} // anonymous namespace


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(NCR53C700, ncr53c700_device, "ncr53c700", "NCR 53C700 SCSI")
DEFINE_DEVICE_TYPE(NCR53C710, ncr53c710_device, "ncr53c710", "NCR 53C710 SCSI")

ncr53c700_device::ncr53c700_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	nscsi_device_interface(mconfig, *this),
	device_execute_interface(mconfig, *this),
	m_icount(0),
	m_irq_handler(*this),
	m_host_write(*this),
	m_host_read(*this, 0)
{
}

ncr53c700_device::ncr53c700_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ncr53c700_device(mconfig, NCR53C700, tag, owner, clock)
{
}

ncr53c710_device::ncr53c710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ncr53c700_device(mconfig, NCR53C710, tag, owner, clock),
	m_big_lit_handler(*this, 0)
{
}

void ncr53c700_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	m_tm = timer_alloc(FUNC(ncr53c700_device::step_timer), this);

	// The SCRIPTS processor runs at ~2 MIPS so approximate this
	set_unscaled_clock(2000000);

	// savestate support
	save_item(NAME(m_scntl));
	save_item(NAME(m_sdid));
	save_item(NAME(m_sien));
	save_item(NAME(m_scid));
	save_item(NAME(m_sxfer));
	save_item(NAME(m_sodl));
	save_item(NAME(m_socl));
	save_item(NAME(m_sfbr));
	save_item(NAME(m_sidl));
	save_item(NAME(m_sbdl));
	save_item(NAME(m_sbcl));
	save_item(NAME(m_dstat));
	save_item(NAME(m_sstat));
	save_item(NAME(m_ctest));
	save_item(NAME(m_temp));
	save_item(NAME(m_dfifo));
	save_item(NAME(m_istat));
	save_item(NAME(m_dbc));
	save_item(NAME(m_dcmd));
	save_item(NAME(m_dnad));
	save_item(NAME(m_dsp));
	save_item(NAME(m_dsps));
	save_item(NAME(m_dmode));
	save_item(NAME(m_dien));
	save_item(NAME(m_dwt));
	save_item(NAME(m_dcntl));

	// other state
	save_item(NAME(m_scsi_state));
	save_item(NAME(m_connected));
	save_item(NAME(m_finished));
	save_item(NAME(m_first_byte_received));
	save_item(NAME(m_xfr_phase));

	save_item(NAME(m_scripts_state));
}

void ncr53c710_device::device_start()
{
	ncr53c700_device::device_start();

	save_item(NAME(m_dsa));
	save_item(NAME(m_ctest8));
	save_item(NAME(m_lcrc));
	save_item(NAME(m_scratch));
	save_item(NAME(m_carry));
}

void ncr53c700_device::device_reset()
{
	// Reset registers to defaults
	m_scntl[0]  = 3 << SCNTL0_ARB_SHIFT;
	m_scntl[1]  = 0;
	m_sdid      = 0;
	m_sien      = 0;
	m_scid      = 0;
	m_sxfer     = 0;
	m_sodl      = 0;
	m_socl      = 0;
	m_sfbr      = 0;
	m_sidl      = 0;
	m_sbdl      = 0;
	m_sbcl      = 0;
	m_dstat     = DSTAT_DFE;
	m_sstat[0]  = 0;
	m_sstat[1]  = 0;
	m_sstat[2]  = 0;
	m_ctest[0]  = 0;
	m_ctest[1]  = 0xf0;
	m_ctest[2]  = 0x21;
	m_ctest[3]  = 0;
	m_ctest[4]  = 0;
	m_ctest[5]  = 0;
	m_ctest[6]  = 0;
	m_ctest[7]  = 0;
	m_temp      = 0;
	m_dfifo     = 0;
	m_istat     = 0;//ISTAT_PRE;
	m_dbc       = 0;
	m_dcmd      = 0;
	m_dnad      = 0;
	m_dsp       = 0;
	m_dsps      = 0;
	m_dmode     = 0;
	m_dien      = 0;
	m_dwt       = 0;
	m_dcntl     = 0;

	m_finished = false;
	m_connected = false;
	m_first_byte_received = false;

	m_scsi_bus->ctrl_wait(m_scsi_refid, S_SEL | S_BSY | S_RST, S_ALL);
	set_scripts_state(SCRIPTS_IDLE);
	set_scsi_state(IDLE);

	m_irq_handler(CLEAR_LINE);
}

void ncr53c710_device::device_reset()
{
	ncr53c700_device::device_reset();

	m_dsa = 0;
	m_ctest8 = CTEST8_REVISION;
	m_lcrc = 0;
	m_scratch = 0;
	m_carry = false;
}


//**************************************************************************
//  MEMORY HANDLERS
//**************************************************************************

//-------------------------------------------------
//  read - Host read handler
//-------------------------------------------------

uint32_t ncr53c700_device::read(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_HOST, "%s: REG R: [%x] (%08X)\n", machine().describe_context(), offset, mem_mask);

	uint32_t ret = 0;

	switch (offset)
	{
		case 0x0:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_scntl[0];
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_scntl[1] << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_sdid << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_sien << 24;
			}

			break;
		}

		case 0x1:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_scid;
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_sxfer << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_sodl << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_socl << 24;
			}

			break;
		}

		case 0x2:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_sfbr;
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_sidl << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_sbdl << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_sbcl << 24;
			}

			break;
		}

		case 0x3:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_dstat;
				// DFE isn't cleared on read
				m_dstat &= DSTAT_DFE;
				update_irqs();
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_sstat[0] << 8;
				m_sstat[0] = 0;
				update_irqs();
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_sstat[1] << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_sstat[2] << 24;
			}

			break;
		}

		case 0x5:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_ctest[0];
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_ctest[1] << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_ctest[2] << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_ctest[3] << 24;
			}

			break;
		}

		case 0x6:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_ctest[4];
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_ctest[5] << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_ctest[6] << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_ctest[7] << 24;
			}

			break;
		}

		case 0x7:
		{
			ret = m_temp;

			break;
		}

		case 0x8:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_dfifo;
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_istat << 8;
			}

			break;
		}

		case 0x9:
		{
			if (ACCESSING_BITS_0_7 || ACCESSING_BITS_8_15 || ACCESSING_BITS_16_23)
			{
				ret = m_dbc;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_dcmd << 24;
			}

			break;
		}

		case 0xa:
		{
			ret = m_dnad;

			break;
		}

		case 0xb:
		{
			ret = m_dsp;

			break;
		}

		case 0xc:
		{
			ret = m_dsps;

			break;
		}

		case 0xd:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_dmode;
			}

			break;
		}

		case 0xe:
		{
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_dien << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_dwt << 16;

			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_dcntl << 24;
			}

			break;
		}

		default:
		{
			LOGMASKED(LOG_UNHANDLED, "%s: Unhandled register access", machine().describe_context());
		}
	}

	return ret;
}


//-------------------------------------------------
//  read - 53C710 host read handler
//-------------------------------------------------

uint32_t ncr53c710_device::read(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x4:
			LOGMASKED(LOG_HOST, "%s: REG R: [%x] (%08X)\n", machine().describe_context(), offset, mem_mask);
			return m_dsa;

		case 0x5:
		{
			uint32_t ret = ncr53c700_device::read(offset, mem_mask);

			if (ACCESSING_BITS_16_23)
			{
				uint8_t const value = (m_ctest[2] & ~CTEST2_SIGP) | (m_istat & ISTAT_SIGP ? CTEST2_SIGP : 0);
				ret = (ret & 0xff00ffff) | (uint32_t(value) << 16);
				m_istat &= ~ISTAT_SIGP;
			}

			return ret;
		}

		case 0x8:
		{
			uint32_t ret = ncr53c700_device::read(offset, mem_mask);

			if (ACCESSING_BITS_16_23)
				ret |= m_ctest8 << 16;
			if (ACCESSING_BITS_24_31)
				ret |= m_lcrc << 24;

			return ret;
		}

		case 0xd:
			LOGMASKED(LOG_HOST, "%s: REG R: [%x] (%08X)\n", machine().describe_context(), offset, mem_mask);
			return m_scratch;

		case 0xe:
		{
			uint32_t ret = ncr53c700_device::read(offset, mem_mask);

			if (ACCESSING_BITS_0_7)
				ret |= m_dmode;

			return ret;
		}

		default:
			return ncr53c700_device::read(offset, mem_mask);
	}
}


//-------------------------------------------------
//  write - Host write handler
//-------------------------------------------------

void ncr53c700_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_HOST, "%s: REG W: [%x] (%08X) %x\n", machine().describe_context(), offset, mem_mask, data);

	switch (offset)
	{
		case 0x0:
		{
			if (ACCESSING_BITS_0_7)
			{
				m_scntl[0] = data;

				if (data & SCNTL0_TRG)
					fatalerror("53c7xx: Target mode unsupported!");

				if (data & SCNTL0_START)
				{
					// Start arbitration
					set_scsi_state(ARBITRATE_WAIT_FREE);
					step(true);
				}
			}
			if (ACCESSING_BITS_8_15)
			{
				m_scntl[1] = data >> 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_sdid = data >> 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_sien = data >> 24;
			}

			break;
		}

		case 0x1:
		{
			if (ACCESSING_BITS_0_7)
			{
				m_scid = data;
			}
			if (ACCESSING_BITS_8_15)
			{
				m_sxfer = data >> 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_sodl = data >> 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_socl = data >> 24;
			}

			break;
		}

		case 0x5:
		{
			if (ACCESSING_BITS_0_7)
				m_ctest[0] = data;

			break;
		}

		case 0x6:
		{
			if (ACCESSING_BITS_0_7)
			{
				m_ctest[4] = data;
			}
			if (ACCESSING_BITS_8_15)
			{
				m_ctest[5] = data >> 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_ctest[6] = data >> 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_ctest[7] = data >> 24;
			}

			break;
		}

		case 0x7:
		{
			m_temp = data;

			break;
		}

		case 0x8:
		{
			if (ACCESSING_BITS_0_7)
			{
				m_dfifo = data;
			}
			if (ACCESSING_BITS_8_15)
			{
				istat_w(data >> 8);
			}
			break;
		}

		case 0x9:
		{
			if (ACCESSING_BITS_0_7 || ACCESSING_BITS_8_15 || ACCESSING_BITS_16_23)
			{
				m_dbc = data & 0xffffff;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_dcmd = data >> 24;
			}

			break;
		}

		case 0xa:
		{
			m_dnad = data;

			break;
		}

		case 0xb:
		{
			// Write to the upper byte starts the fetch
			m_dsp = data;

			if (m_dmode & 1)
			{
				set_scripts_state(SCRIPTS_WAIT_MANUAL_START);
			}
			else
			{
				set_scripts_state(SCRIPTS_FETCH);
			}

			break;
		}

		case 0xc:
		{
			m_dsps = data;

			break;
		}

		case 0xd:
		{
			if (ACCESSING_BITS_0_7)
			{
				m_dmode = data;

				if (m_dmode & DMODE_PIPE)
					fatalerror("53c700: DMA pipeline mode not supported!");
			}

			break;
		}

		case 0xe:
		{
			if (ACCESSING_BITS_8_15)
			{
				m_dien = data >> 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_dwt = data >> 16;

				if (m_dwt)
					fatalerror("53c7xx: DMA Watchdog Timer enabled!");
			}
			if (ACCESSING_BITS_24_31)
			{
				dcntl_w(data >> 24);
			}

			break;
		}

		default:
		{
			LOGMASKED(LOG_UNHANDLED, "%s: Unhandled register access", machine().describe_context());
		}
	}
}


//-------------------------------------------------
//  write - 53C710 host write handler
//-------------------------------------------------

void ncr53c710_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x4:
			LOGMASKED(LOG_HOST, "%s: REG W: [%x] (%08X) %x\n", machine().describe_context(), offset, mem_mask, data);
			COMBINE_DATA(&m_dsa);
			break;

		case 0x8:
			ncr53c700_device::write(offset, data, mem_mask);

			if (ACCESSING_BITS_16_23)
			{
				uint8_t const value = data >> 16;
				m_ctest8 = CTEST8_REVISION | (value & CTEST8_WRITABLE);

				if (value & CTEST8_CLF)
				{
					m_dfifo = 0;
					m_sstat[1] &= ~(SSTAT1_ORF | SSTAT1_OLF | SSTAT1_ILF);
				}
			}
			if (ACCESSING_BITS_24_31)
				m_lcrc = data >> 24;
			break;

		case 0xd:
			LOGMASKED(LOG_HOST, "%s: REG W: [%x] (%08X) %x\n", machine().describe_context(), offset, mem_mask, data);
			COMBINE_DATA(&m_scratch);
			break;

		case 0xe:
			ncr53c700_device::write(offset, data, mem_mask);

			if (ACCESSING_BITS_0_7)
				m_dmode = data;
			break;

		default:
			ncr53c700_device::write(offset, data, mem_mask);
	}
}


//-------------------------------------------------
//  dcntl_w - handle 53C700 DMA control
//-------------------------------------------------

void ncr53c700_device::dcntl_w(uint8_t data)
{
	m_dcntl = data;

	if (m_dcntl & DCNTL_RST)
	{
		device_reset();
		return;
	}

	if (m_dcntl & DCNTL_SSM)
		fatalerror("53c7xx: DMA single-step mode not supported!");

	if ((m_dcntl & DCNTL_STD) && (m_scripts_state == SCRIPTS_WAIT_MANUAL_START))
		set_scripts_state(SCRIPTS_FETCH);

	if (m_dcntl & DCNTL_LLM)
		fatalerror("53c7xx: SCSI Low-Level Mode not supported!");

	// TODO: Update clocking
}


//-------------------------------------------------
//  dcntl_w - handle 53C710 DMA control
//-------------------------------------------------

void ncr53c710_device::dcntl_w(uint8_t data)
{
	m_dcntl = data;

	if (m_dcntl & DCNTL_SSM)
		fatalerror("53c7xx: DMA single-step mode not supported!");

	if ((m_dcntl & DCNTL_STD) && (m_scripts_state == SCRIPTS_WAIT_MANUAL_START))
		set_scripts_state(SCRIPTS_FETCH);

	if (m_dcntl & DCNTL_LLM)
		fatalerror("53c7xx: SCSI Low-Level Mode not supported!");

	// TODO: Update clocking
}


//-------------------------------------------------
//  istat_w - handle model-specific interrupt status
//-------------------------------------------------

void ncr53c700_device::istat_w(uint8_t data)
{
	m_istat = data;
}


//-------------------------------------------------
//  istat_w - handle 53C710 interrupt status
//-------------------------------------------------

void ncr53c710_device::istat_w(uint8_t data)
{
	m_istat = (m_istat & (ISTAT_CON | ISTAT_SIP | ISTAT_DIP)) | (data & (ISTAT_ABRT | ISTAT_RST | ISTAT_SIGP));

	if (m_istat & ISTAT_RST)
	{
		uint8_t const dcntl = m_dcntl & DCNTL_EA;
		uint8_t const dmode = m_dmode & DMODE_FC1;

		device_reset();

		m_dcntl = dcntl;
		m_dmode = dmode;
		m_istat = ISTAT_RST;
	}
}



//**************************************************************************
//  SCSI STATE MACHINE
//**************************************************************************

//-------------------------------------------------
//  update_irqs -
//-------------------------------------------------
void ncr53c700_device::update_irqs()
{
	if (m_sstat[0] & m_sien)
		m_istat |= ISTAT_SIP;
	else
		m_istat &= ~ISTAT_SIP;

	if (m_dstat & m_dien)
		m_istat |= ISTAT_DIP;
	else
		m_istat &= ~ISTAT_DIP;

	m_irq_handler(m_istat & (ISTAT_SIP | ISTAT_DIP) ? ASSERT_LINE : CLEAR_LINE);
}

//-------------------------------------------------
//  set_scsi_state - change SCSI state
//-------------------------------------------------

void ncr53c700_device::set_scsi_state(int state)
{
	LOGMASKED(LOG_STATE, "SCSI state change: %x to %x\n", m_scsi_state, state);

	m_scsi_state = state;
}


//-------------------------------------------------
//  delay - step the SCSI state machine following
//  a time delay
//-------------------------------------------------

void ncr53c700_device::delay(const attotime &delay)
{
	m_tm->adjust(delay);
}


//-------------------------------------------------
//  scsi_ctrl_changed - callback from nscsi_device
//-------------------------------------------------

void ncr53c700_device::scsi_ctrl_changed()
{
	step(false);
}


//-------------------------------------------------
//  host_byte_shift - locate a byte within a host
//  memory longword
//-------------------------------------------------

unsigned ncr53c700_device::host_byte_shift(offs_t address)
{
	return 8 * (address & 3);
}


//-------------------------------------------------
//  host_byte_shift - locate a byte within a host
//  memory longword with selectable endianness
//-------------------------------------------------

unsigned ncr53c710_device::host_byte_shift(offs_t address)
{
	return 8 * ((address & 3) ^ (m_big_lit_handler() ? 3 : 0));
}


//-------------------------------------------------
//  transfer_control_address - resolve an absolute
//  53C700 transfer-control address
//-------------------------------------------------

uint32_t ncr53c700_device::transfer_control_address()
{
	return m_dsps;
}


//-------------------------------------------------
//  transfer_control_address - resolve an absolute
//  or relative 53C710 transfer-control address
//-------------------------------------------------

uint32_t ncr53c710_device::transfer_control_address()
{
	if (BIT(m_dbc, 23))
	{
		int32_t const displacement = util::sext(m_dsps, 24);
		return m_dsp + displacement;
	}

	return m_dsps;
}


//-------------------------------------------------
//  host_memory_read - read host memory on behalf
//  of the DMA engine
//-------------------------------------------------

uint32_t ncr53c700_device::host_memory_read(offs_t address, uint32_t mem_mask)
{
	return m_host_read(address, mem_mask);
}


//-------------------------------------------------
//  host_memory_write - write host memory on behalf
//  of the DMA engine
//-------------------------------------------------

void ncr53c700_device::host_memory_write(offs_t address, uint32_t data, uint32_t mem_mask)
{
	m_host_write(address, data, mem_mask);
}


//-------------------------------------------------
//  send_byte - send data to a SCSI device
//-------------------------------------------------

void ncr53c700_device::send_byte()
{
	if (m_dbc == 0)
		fatalerror("53C7XX: send_byte() - DBC should not be 0\n");

	set_scsi_state( (m_scsi_state & STATE_MASK) | (SEND_WAIT_SETTLE << SUB_SHIFT) );

	uint32_t data = host_memory_read(m_dnad & ~3, 0xffffffff);
	data = data >> host_byte_shift(m_dnad) & 0xff;

	++m_dnad;
	--m_dbc;

	m_scsi_bus->data_w(m_scsi_refid, data);
	m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_REQ, S_REQ);
	delay(attotime::from_nsec(5));
}


//-------------------------------------------------
//  recv_byte - receive data from a SCSI device
//-------------------------------------------------

void ncr53c700_device::recv_byte()
{
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_REQ, S_REQ);
	set_scsi_state( (m_scsi_state & STATE_MASK) | (RECV_WAIT_REQ_1 << SUB_SHIFT) );
	step(false);
}


//-------------------------------------------------
//  step_timer - callback to step the SCSI
//  state machine
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(ncr53c700_device::step_timer)
{
	step(true);
}


//-------------------------------------------------
//  step - advance the SCSI state machine
//-------------------------------------------------

void ncr53c700_device::step(bool timeout)
{
	uint32_t ctrl = m_scsi_bus->ctrl_r();
	uint32_t data = m_scsi_bus->data_r();

	LOGMASKED(LOG_STATE, "Step: CTRL:%x DATA:%x (%d.%d) Timeout:%d\n", ctrl, data, m_scsi_state & STATE_MASK, m_scsi_state >> SUB_SHIFT, timeout);

	// Check for disconnect from target
	if (!(m_scntl[0] & SCNTL0_TRG) && m_connected && !(ctrl & S_BSY))
	{
		set_scsi_state(FINISHED);
		m_connected = false;
//      istatus |= I_DISCONNECT;
//      reset_disconnect();
//      check_irq();
	}

	switch (m_scsi_state & SUB_MASK ? m_scsi_state & SUB_MASK : m_scsi_state & STATE_MASK)
	{
		case IDLE:
		{
			break;
		}

		case FINISHED:
		{
			m_finished = true;
			set_scsi_state(IDLE);
			step(true);

			break;
		}

		case ARBITRATE_WAIT_FREE:
		{
			if (!timeout)
				break;

			// Is the bus free?
			if (ctrl & (S_BSY | S_SEL))
			{
				// Keep trying until it is
				delay(attotime::from_nsec(800));
			}
			else
			{
				// Bus is free; next phase
				delay(attotime::from_nsec(800));
				set_scsi_state(ARBITRATE_CHECK_FREE);
			}

			break;
		}

		case ARBITRATE_CHECK_FREE:
		{
			if ((ctrl & (S_BSY | S_SEL)) == 0)
			{
				// Bus is free - assert the controller SCSI ID and BUSY
				m_scsi_bus->ctrl_w(m_scsi_refid, S_BSY, S_BSY);

				if (((m_scntl[0] >> SCNTL0_ARB_SHIFT) & SCNTL0_ARB_MASK) == 3)
				{
					// Full arbitration
					m_scsi_bus->data_w(m_scsi_refid, m_scid);
				}
				else
				{
					// Simple arbitration
					m_scsi_bus->data_w(m_scsi_refid, m_sodl);
				}

				set_scsi_state(ARBITRATE_EXAMINE_BUS);
				delay(attotime::from_nsec(2400));
			}

			break;
		}

		case ARBITRATE_EXAMINE_BUS:
		{
			if (!timeout)
				break;

			if (ctrl & S_SEL)
			{
				m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_BSY);
				m_scsi_bus->data_w(m_scsi_refid, 0);

				if (((m_scntl[0] >> SCNTL0_ARB_SHIFT) & SCNTL0_ARB_MASK) == 3)
				{
					// Try again
					set_scsi_state(ARBITRATE_WAIT_FREE);
					delay(attotime::from_nsec(2400));
				}
				else
				{
					// TODO: Is this right?
					m_sstat[1] |= SSTAT1_LOA;
					m_scntl[0] &= ~SCNTL0_START;
					m_sstat[0] |= SSTAT0_CMP;
					update_irqs();

					set_scsi_state(FINISHED);
					step(true);
				}

				break;
			}

			// Full arbitration?
			if (((m_scntl[0] >> SCNTL0_ARB_SHIFT) & SCNTL0_ARB_MASK) == 3)
			{
				int win;
				for (win = 7; win >=0 && !(data & (1 << win)); win--) {};

				if ((1 << win) != m_scid)
				{
					m_scsi_bus->data_w(m_scsi_refid, 0);
					m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);

					delay(attotime::from_nsec(2400));
					break;
				}

				// Begin the select phase; assert SEL
				m_sstat[1] |= SSTAT1_WOA;
				m_scsi_bus->ctrl_w(m_scsi_refid, S_SEL, S_SEL);
				set_scsi_state(ARBITRATE_ASSERT_SEL);
				delay(attotime::from_nsec(1200));
			}
			else
			{
				// TODO: Worth adding another state here?
				m_sstat[0] |= SSTAT0_CMP;
				m_scntl[0] &= ~SCNTL0_START;
				update_irqs();
				set_scsi_state(FINISHED);
				step(true);
			}

			break;
		}

		case ARBITRATE_ASSERT_SEL:
		{
			if (!timeout)
				break;

			// selection identifies both the initiator and the target
			m_scsi_bus->data_w(m_scsi_refid, m_scid | m_sdid);

			set_scsi_state(ARBITRATE_SELECT_DEST);
			delay(attotime::from_nsec(2));

			break;
		}

		case ARBITRATE_SELECT_DEST:
		{
			if (!timeout)
				break;

			m_scsi_bus->ctrl_w(m_scsi_refid, m_scntl[0] & SCNTL0_WATN ? S_ATN : 0, S_ATN | S_BSY);

			set_scsi_state(ARBITRATE_RELEASE_BSY);
			delay(attotime::from_nsec(20));

			break;
		}

		case ARBITRATE_RELEASE_BSY:
		{
			if (!timeout)
				break;

			if (ctrl & S_BSY)
			{
				set_scsi_state(ARBITRATE_DESKEW_WAIT);
				delay(attotime::from_nsec(500));
			}
			else
			{
				set_scsi_state(ARBITRATE_WAIT_BSY);

				if (!(m_ctest[4] & CTEST4_NOTIME))
					delay(attotime::from_msec(250));
			}

			break;
		}

		case ARBITRATE_WAIT_BSY:
		{
			if (ctrl & S_BSY)
			{
				set_scsi_state(ARBITRATE_DESKEW_WAIT);
				delay(attotime::from_nsec(500));
			}
			else if (timeout)
			{
				LOGMASKED(LOG_STATE, "Selection timeout selecting ID %02x\n", m_sdid);

				m_scntl[0] &= ~SCNTL0_START;
				m_sstat[0] |= SSTAT0_STO;
				m_connected = false;
				set_scsi_state(IDLE);
				set_scripts_state(SCRIPTS_IDLE);

				m_scsi_bus->data_w(m_scsi_refid, 0);
				m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_SEL | S_ATN | S_BSY);
				update_irqs();
			}

			break;
		}

		case ARBITRATE_DESKEW_WAIT:
		{
			if (!timeout)
				break;

			// Clear everything
			m_scsi_bus->data_w(m_scsi_refid, 0);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_SEL);

			// Done?
			m_sstat[0] |= SSTAT0_CMP;
			m_scntl[0] &= ~SCNTL0_START;
			//update_irqs();
			set_scsi_state(FINISHED);
			m_connected = true;

			step(true);
			break;
		}


		// Note this is actually block transfers
		case INIT_XFER:
		{
			if (ctrl & S_INP)
			{
				set_scsi_state(m_dbc ? INIT_XFER_RECV_BYTE_ACK : INIT_XFER_RECV_BYTE_NACK);
				recv_byte();
			}
			else
			{
				if (m_dbc == 1)
					m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ATN);

				set_scsi_state(INIT_XFER_SEND_BYTE);
				send_byte();
			}

			break;
		}

		case INIT_XFER_SEND_BYTE:
		{
			if (m_dbc == 0)
			{
				set_scsi_state(FINISHED);
				step(true);
			}
			else
			{
				set_scsi_state(INIT_XFER_WAIT_REQ);
			}

			break;
		}

		case INIT_XFER_RECV_BYTE_ACK:
		{
			set_scsi_state(INIT_XFER_WAIT_REQ);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK);

			break;
		}

		case INIT_XFER_RECV_BYTE_NACK:
		{
			set_scsi_state(FINISHED);
			step(true);

			break;
		}

		case INIT_XFER_WAIT_REQ:
		{
			if (!(ctrl & S_REQ))
				break;

			if ((ctrl & S_PHASE_MASK) != m_xfr_phase)
			{
				set_scsi_state(FINISHED);
				step(true);
			}
			else
			{
				set_scsi_state(INIT_XFER);
				step(false);
			}

			break;
		}

		case SEND_WAIT_SETTLE << SUB_SHIFT:
		{
			if (!timeout)
				break;

			set_scsi_state( (m_scsi_state & STATE_MASK) | (SEND_WAIT_REQ_0 << SUB_SHIFT) );
			step(false);

			break;
		}

		case SEND_WAIT_REQ_0 << SUB_SHIFT:
		{
			if (ctrl & S_REQ)
				break;

			set_scsi_state(m_scsi_state & STATE_MASK);
			m_scsi_bus->data_w(m_scsi_refid, 0);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK);

			step(false);

			break;
		}

		case RECV_WAIT_REQ_1 << SUB_SHIFT:
		{
			if (!(ctrl & S_REQ))
				break;

			set_scsi_state( (m_scsi_state & STATE_MASK) | (RECV_WAIT_SETTLE << SUB_SHIFT) );
			delay(attotime::from_nsec(5));

			break;
		}

		case RECV_WAIT_SETTLE << SUB_SHIFT:
		{
			if (!timeout)
				break;

			if ((m_scsi_state & STATE_MASK) != INIT_XFER_RECV_PAD)
			{
				if (!m_first_byte_received)
				{
					m_sfbr = m_scsi_bus->data_r();
					m_first_byte_received = true;
				}

				uint32_t shift = host_byte_shift(m_dnad);
				uint32_t mem_mask = 0xff << shift;
				host_memory_write(m_dnad & ~3, data << shift, mem_mask);

				++m_dnad;
				--m_dbc;
			}

			m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
			set_scsi_state( (m_scsi_state & STATE_MASK) | (RECV_WAIT_REQ_0 << SUB_SHIFT) );
			step(false);

			break;
		}

		case RECV_WAIT_REQ_0 << SUB_SHIFT:
		{
			if (ctrl & S_REQ)
				break;

			set_scsi_state(m_scsi_state & STATE_MASK);
			step(false);

			break;
		}

		default:
			fatalerror("Unknown state! (%x)\n", m_scsi_state);

	}
}


//**************************************************************************
//  SCSI SCRIPTS
//**************************************************************************

//-------------------------------------------------
//  set_scripts_state -
//-------------------------------------------------

void ncr53c700_device::set_scripts_state(scripts_state state)
{
	m_scripts_state = state;
}


//-------------------------------------------------
//  scripts_yield - suspend execution
//-------------------------------------------------
void ncr53c700_device::scripts_yield()
{
	m_icount = 0;
}


//-------------------------------------------------
//  execute_run - SCRIPTS execution loop
//-------------------------------------------------

void ncr53c700_device::execute_run()
{
	// Not processing anything so bail
	if (m_scripts_state < SCRIPTS_FETCH)
	{
		m_icount = 0;
		return;
	}

	do
	{
		switch (m_scripts_state)
		{
			case SCRIPTS_FETCH:
			{
				m_finished = false;

				// Fetch the instruction
				uint32_t inst = host_memory_read(m_dsp, 0xffffffff);

				LOGMASKED(LOG_SCRIPTS, "FETCH dsp=%08x inst=%08x istat=%02x dstat=%02x\n", m_dsp, inst, m_istat, m_dstat);

				m_dcmd = inst >> 24;
				m_dbc = inst & 0xffffff;

				// Unless we encounter an illegal instruction...
				set_scripts_state(SCRIPTS_EXECUTE);

				// Decode the relevant group
				switch ((m_dcmd >> 6) & 3)
				{
					case 0:
						scripts_decode_bm();
						break;

					case 1:
						scripts_decode_io();
						break;

					case 2:
						scripts_decode_tc();
						break;

					case 3:
						scripts_decode_memory_move();
				}

				LOGMASKED(LOG_SCRIPTS, "%s", disassemble_scripts());
				break;
			}

			case SCRIPTS_EXECUTE:
			{
				(*this.*m_scripts_op)();
				break;
			}
		}

		m_icount--;
	} while (m_icount > 0);
}


//-------------------------------------------------
//  scripts_decode_bm - decode block move
//-------------------------------------------------

void ncr53c700_device::scripts_decode_bm(void)
{
	// Decode our instruction
	if (m_scntl[0] & SCNTL0_TRG)
	{
		// Target mode
		switch (block_move_opcode())
		{
			case 0:
				m_scripts_op = &ncr53c700_device::bm_t_move;
				break;

			default:
				illegal();
				return;
		}
	}
	else
	{
		// Initiator mode
		switch (block_move_opcode())
		{
			case 1:
				m_scripts_op = &ncr53c700_device::bm_i_wmov;
				break;

			default:
				illegal();
				return;
		}
	}

	m_dnad = host_memory_read(m_dsp + 4, 0xffffffff);
	m_dsp += 8;
	configure_block_move();
}


//-------------------------------------------------
//  block_move_opcode - decode the 53C700 two-bit
//  Block Move opcode
//-------------------------------------------------

unsigned ncr53c700_device::block_move_opcode() const
{
	return (m_dcmd >> 3) & 3;
}


//-------------------------------------------------
//  block_move_opcode - decode the 53C710 one-bit
//  Block Move opcode
//-------------------------------------------------

unsigned ncr53c710_device::block_move_opcode() const
{
	return BIT(m_dcmd, 3);
}


//-------------------------------------------------
//  configure_block_move - no additional 53C700
//  Block Move operands
//-------------------------------------------------

void ncr53c700_device::configure_block_move()
{
}


//-------------------------------------------------
//  configure_block_move - resolve a 53C710 table
//  indirect byte count and data address
//-------------------------------------------------

void ncr53c710_device::configure_block_move()
{
	if (!BIT(m_dcmd, 4))
		return;

	// load from table indirect
	uint32_t const table_address = m_dsa + util::sext(m_dnad, 24);
	m_dbc = host_memory_read(table_address, 0xffffffff) & 0xffffff;
	m_dnad = host_memory_read(table_address + 4, 0xffffffff);

	LOGMASKED(LOG_SCRIPTS, "BM table [%08x]: count=%06x address=%08x\n", table_address, m_dbc, m_dnad);
}


//-------------------------------------------------
//  scripts_decode_io - decode IO
//-------------------------------------------------

void ncr53c700_device::scripts_decode_io(void)
{
	// exit if scripts_decode_read_write() already handled this instruction
	if (scripts_decode_read_write())
		return;

	// Set Target Mode?
	if (m_dbc & (1 << 9))
		m_scntl[0] |= SCNTL0_TRG;

	// Decode our instruction
	if (m_scntl[0] & SCNTL0_TRG)
	{
		// Initiator mode
		switch ((m_dcmd >> 3) & 7)
		{
			case 0:
				m_scripts_op = &ncr53c700_device::io_t_reselect;
				break;

			case 1:
				m_scripts_op = &ncr53c700_device::io_t_disconnect;
				break;

			case 2:
				m_scripts_op = &ncr53c700_device::io_t_waitselect;
				break;

			case 3:
				m_scripts_op = &ncr53c700_device::io_t_set;
				break;

			case 4:
				m_scripts_op = &ncr53c700_device::io_t_clear;
				break;

			default:
				illegal();
				return;
		}
	}
	else
	{
		// Initiator mode
		switch ((m_dcmd >> 3) & 7)
		{
			case 0:
				m_scripts_op = &ncr53c700_device::io_i_select;
				break;

			case 1:
				m_scripts_op = &ncr53c700_device::io_i_waitdisconnect;
				break;

			case 2:
				m_scripts_op = &ncr53c700_device::io_i_waitreselect;
				break;

			case 3:
				m_scripts_op = &ncr53c700_device::io_i_set;
				break;

			case 4:
				m_scripts_op = &ncr53c700_device::io_i_clear;
				break;

			default:
				illegal();
				return;
		}
	}

	// Set some additional registers
	m_dnad = m_dsps = host_memory_read(m_dsp + 4, 0xffffffff);
	m_dsp += 8;
	load_io_operands();
}


//-------------------------------------------------
//  load_io_operands - load a direct 53C700 SCSI ID
//-------------------------------------------------

void ncr53c700_device::load_io_operands()
{
	m_sdid = m_dbc >> 16;
}


//-------------------------------------------------
//  load_io_operands - load direct or table indirect
//  53C710 selection parameters
//-------------------------------------------------

void ncr53c710_device::load_io_operands()
{
	m_sdid = m_dbc >> 16;

	if (!BIT(m_dcmd, 1))
		return;

	// load from table indirect
	uint32_t const table_address = m_dsa + util::sext(m_dbc, 24);
	uint32_t const table_data = host_memory_read(table_address, 0xffffffff);
	m_sdid = table_data >> host_byte_shift(table_address + 1);
	m_sxfer = table_data >> host_byte_shift(table_address + 2);

	LOGMASKED(LOG_SCRIPTS, "IO table [%08x]: data=%08x id=%02x sxfer=%02x\n", table_address, table_data, m_sdid, m_sxfer);
}


//-------------------------------------------------
//  scripts_decode_read_write - read/write SCRIPTS
//  instructions are not available on the 53C700
//-------------------------------------------------

bool ncr53c700_device::scripts_decode_read_write()
{
	return false;
}


//-------------------------------------------------
//  scripts_register_read - read a byte using the
//  fixed little-endian SCRIPTS register addresses
//-------------------------------------------------

uint8_t ncr53c710_device::scripts_register_read(uint8_t address)
{
	unsigned const shift = 8 * (address & 3);
	uint32_t const mem_mask = 0xffU << shift;
	return read(address >> 2, mem_mask) >> shift;
}


//-------------------------------------------------
//  scripts_register_write - write a byte using the
//  fixed little-endian SCRIPTS register addresses
//-------------------------------------------------

void ncr53c710_device::scripts_register_write(uint8_t address, uint8_t data)
{
	unsigned const shift = 8 * (address & 3);
	uint32_t const mem_mask = 0xffU << shift;
	write(address >> 2, uint32_t(data) << shift, mem_mask);
}


//-------------------------------------------------
//  scripts_decode_read_write - execute a 53C710
//  register read/write instruction
//-------------------------------------------------

bool ncr53c710_device::scripts_decode_read_write()
{
	unsigned const opcode = (m_dcmd >> 3) & 7;

	if (opcode < 5)
		return false;

	m_dsps = host_memory_read(m_dsp + 4, 0xffffffff);
	m_dsp += 8;

	uint8_t const address = (m_dbc >> 16) & 0x3f;
	uint8_t const immediate = m_dbc >> 8;

	// DBC bits 23-22 and 7-0 and the entire second word are reserved
	// ISTAT cannot be accessed by a read/write instruction
	if ((m_dbc & 0xc000ff) || m_dsps || (address == 0x21))
	{
		illegal();
		return true;
	}

	uint8_t value = immediate;

	if (((m_dcmd >> 1) & 3) != 0)
	{
		uint8_t const operand = (opcode == 5) ? m_sfbr : scripts_register_read(address);

		switch ((m_dcmd >> 1) & 3)
		{
			case 1:
				value = operand | immediate;
				break;

			case 2:
				value = operand & immediate;
				break;

			case 3:
			{
				uint16_t const result = uint16_t(operand) + immediate + (BIT(m_dcmd, 0) && m_carry ? 1 : 0);
				value = result;
				m_carry = BIT(result, 8);
				break;
			}
		}
	}

	if (opcode == 6)
		m_sfbr = value;
	else
		scripts_register_write(address, value);

	set_scripts_state(SCRIPTS_FETCH);
	return true;
}


//-------------------------------------------------
//  scripts_decode_tc - decode transfer control
//-------------------------------------------------

void ncr53c700_device::scripts_decode_tc(void)
{
	// Decode our instruction
	switch ((m_dcmd >> 3) & 7)
	{
		case 0:
			m_scripts_op = &ncr53c700_device::tc_jump;
			break;

		case 1:
			m_scripts_op = &ncr53c700_device::tc_call;
			break;

		case 2:
			m_scripts_op = &ncr53c700_device::tc_return;
			break;

		case 3:
			m_scripts_op = &ncr53c700_device::tc_int;
			break;

		default:
			illegal();
			break;
	}

	m_dnad = m_dsps = host_memory_read(m_dsp + 4, 0xffffffff);
	m_dsp += 8;
}


//-------------------------------------------------
//  scripts_decode_memory_move - memory move is not
//  available on the 53C700
//-------------------------------------------------

void ncr53c700_device::scripts_decode_memory_move()
{
	illegal();
}


//-------------------------------------------------
//  scripts_decode_memory_move - execute a 53C710
//  memory-to-memory move
//-------------------------------------------------

void ncr53c710_device::scripts_decode_memory_move()
{
	// bits 29-24 are reserved for this instruction
	if (m_dcmd & 0x3f)
	{
		illegal();
		return;
	}

	m_dsps = host_memory_read(m_dsp + 4, 0xffffffff);
	m_temp = host_memory_read(m_dsp + 8, 0xffffffff);
	m_dsp += 12;

	uint32_t source = m_dsps;
	uint32_t destination = m_temp;

	// make sure we are aligned
	if ((source ^ destination) & 3)
	{
		illegal();
		return;
	}

	// DSA is overwritten before the source is read, but its exact use for the move isn't known
	m_dsa = destination;

	// moves a single byte for the non-aligned cases
	auto move_byte = [this, &source, &destination]()
	{
		unsigned const source_shift = host_byte_shift(source);
		uint32_t const source_mask = 0xffU << source_shift;
		uint8_t const data = host_memory_read(source & ~3U, source_mask) >> source_shift;
		unsigned const destination_shift = host_byte_shift(destination);
		uint32_t const destination_mask = 0xffU << destination_shift;

		host_memory_write(destination & ~3U, uint32_t(data) << destination_shift, destination_mask);
		++source;
		++destination;
		--m_dbc;
	};

	// copy the leading bytes until we are 32-bit aligned
	while (m_dbc && (source & 3))
		move_byte();

	// move as many 32-bit values as possible
	while (m_dbc >= 4)
	{
		host_memory_write(destination, host_memory_read(source, 0xffffffff), 0xffffffff);
		source += 4;
		destination += 4;
		m_dbc -= 4;
	}

	// move the trailing bytes if necessary
	while (m_dbc)
		move_byte();

	set_scripts_state(SCRIPTS_FETCH);
}

//**************************************************************************
//  SCSI SCRIPTS INSTRUCTIONS
//**************************************************************************

//-------------------------------------------------
//  illegal - illegal instruction
//-------------------------------------------------

void ncr53c700_device::illegal()
{
	LOGMASKED(LOG_UNHANDLED, "Illegal SCRIPT: dcmd=%02x dbc=%06x dsp=%08x dsps=%08x\n", m_dcmd, m_dbc, m_dsp, m_dsps);

	m_dstat |= DSTAT_OPC;
	update_irqs();
	set_scripts_state(SCRIPTS_IDLE);
}


//-------------------------------------------------
//  unimplemented - report an unimplemented valid
//  SCRIPTS operation
//-------------------------------------------------

[[noreturn]] void ncr53c700_device::unimplemented(char const *operation) const
{
	fatalerror("%s is unimplemented: dcmd=%02x dbc=%06x dsp=%08x dsps=%08x\n", operation, m_dcmd, m_dbc, m_dsp, m_dsps);
}


//-------------------------------------------------
//  bm_t_move - block move (target)
//-------------------------------------------------

void ncr53c700_device::bm_t_move()
{
	unimplemented(__func__);
}


//-------------------------------------------------
//  bm_i_wmov - wait block move (initiator)
//-------------------------------------------------

void ncr53c700_device::bm_i_wmov()
{
	if (!m_finished)
	{
		if (m_scsi_state == IDLE)
		{
			m_first_byte_received = false;

			if (m_dbc == 0)
			{
				LOGMASKED(LOG_UNHANDLED, "DBC should not be 0\n");
				illegal();
			}

			// Indirect addressing
			if (m_dcmd & (1 << 5))
				m_dnad = host_memory_read(m_dnad, 0xffffffff);

			// Compare the phase bits
			if ((m_scsi_bus->ctrl_r() & 7) == (m_dcmd & 7))
			{
				// Transfer bytes
				set_scsi_state(INIT_XFER);
				m_xfr_phase = m_dcmd & 7;
				step(false);
			}
			else
			{
				fatalerror("Phase mismatch\n");
			}
		}
		else
		{
			scripts_yield();
		}
	}
	else
	{
		// TODO: We should see what happened here; different behaviour
		// depending on whether or not we won arbitration
		set_scripts_state(SCRIPTS_FETCH);
	}
}


//-------------------------------------------------
//  io_t_reselect -
//-------------------------------------------------

void ncr53c700_device::io_t_reselect()
{
	unimplemented(__func__);
}


//-------------------------------------------------
//  io_t_disconnect -
//-------------------------------------------------

void ncr53c700_device::io_t_disconnect()
{
	unimplemented(__func__);
}


//-------------------------------------------------
//  io_t_waitselect -
//-------------------------------------------------

void ncr53c700_device::io_t_waitselect()
{
	unimplemented(__func__);
}


//-------------------------------------------------
//  io_t_set -
//-------------------------------------------------

void ncr53c700_device::io_t_set()
{
	unimplemented(__func__);
}


//-------------------------------------------------
//  io_t_clear -
//-------------------------------------------------

void ncr53c700_device::io_t_clear()
{
	unimplemented(__func__);
}


//-------------------------------------------------
//  io_i_select -
//-------------------------------------------------

void ncr53c700_device::io_i_select()
{
	if (!m_finished)
	{
		if (m_scsi_state == IDLE)
		{
			m_scntl[0] |= (3 << SCNTL0_ARB_SHIFT) | SCNTL0_START;

			// Set select with ATN bit
			if (m_dcmd & 1)
				m_scntl[0] |= SCNTL0_WATN;

			// Start the arbitration
			set_scsi_state(ARBITRATE_WAIT_FREE);
			step(true);
		}

		scripts_yield();
	}
	else
	{
		// TODO: We should see what happened here; different behaviour
		// depending on whether or not we won arbitration
		set_scripts_state(SCRIPTS_FETCH);
	}
}


//-------------------------------------------------
//  io_i_waitdisconnect -
//-------------------------------------------------

void ncr53c700_device::io_i_waitdisconnect()
{
	if (m_scsi_bus->ctrl_r() & (S_BSY | S_SEL))
		scripts_yield();
	else
		set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  io_i_waitreselect -
//-------------------------------------------------

void ncr53c700_device::io_i_waitreselect()
{
	unimplemented(__func__);
}


//-------------------------------------------------
//  io_i_waitreselect - 53C710 signal-process wakeup
//-------------------------------------------------

void ncr53c710_device::io_i_waitreselect()
{
	// kickstart 3.1 on the a4000t uses waitreselect to idle SCRIPTS
	// and will wake it up with SIGP once more work is ready
	// actual reselection is unimplemented like for the base 53C700

	if (!(m_istat & ISTAT_SIGP))
	{
		m_icount = 0;
		return;
	}

	if (BIT(m_dcmd, 2))
	{
		int32_t const displacement = util::sext(m_dnad, 24);
		m_dsp += displacement;
	}
	else
	{
		m_dsp = m_dnad;
	}

	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  io_i_set -
//-------------------------------------------------

void ncr53c700_device::io_i_set()
{
	uint32_t mask = 0;

	if (m_dbc & (1 << 3))
		mask |= S_ATN;

	if (m_dbc & (1 << 6))
		mask |= S_ACK;

	m_scsi_bus->ctrl_w(m_scsi_refid, mask, mask);

	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  io_i_clear -
//-------------------------------------------------

void ncr53c700_device::io_i_clear()
{
	uint32_t mask = 0;

	if (m_dbc & (1 << 3))
		mask |= S_ATN;

	if (m_dbc & (1 << 6))
		mask |= S_ACK;

	m_scsi_bus->ctrl_w(m_scsi_refid, 0, mask);

	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  scripts_data_compare - compare SFBR with the
//  masked Transfer Control data value
//-------------------------------------------------

bool ncr53c700_device::scripts_data_compare() const
{
	uint8_t const mask = m_dbc >> 8;
	return ((m_sfbr ^ uint8_t(m_dbc)) & uint8_t(~mask)) == 0;
}


//-------------------------------------------------
//  tc_jump -
//-------------------------------------------------

void ncr53c700_device::tc_jump()
{
//  if (m_dbc & (1 << 16))
//      printf("Must wait for valid phase?\n");

	bool jump = true;

	if (m_dbc & (1 << 17))
	{
		// Phase
		jump &= (m_dcmd & 7) == (m_scsi_bus->ctrl_r() & 7);
	}
	if (m_dbc & (1 << 18))
	{
		// Data
		jump &= scripts_data_compare();
	}

	if (!(m_dbc & (1 << 19)))
		jump = !jump;

	if (jump)
	{
		m_dsp = transfer_control_address();
	}
	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  tc_call -
//-------------------------------------------------

void ncr53c700_device::tc_call()
{
	bool jump = true;

	if (m_dbc & (1 << 17))
	{
		// Phase
		jump &= (m_dcmd & 7) == (m_scsi_bus->ctrl_r() & 7);
	}
	if (m_dbc & (1 << 18))
	{
		// Data
		jump &= scripts_data_compare();
	}

	if (!(m_dbc & (1 << 19)))
		jump = !jump;

	if (jump)
	{
		m_temp = m_dsp;
		m_dsp = transfer_control_address();
	}
	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  tc_return -
//-------------------------------------------------

void ncr53c700_device::tc_return()
{
	bool jump = true;

	if (m_dbc & (1 << 17))
	{
		// Phase
		jump &= (m_dcmd & 7) == (m_scsi_bus->ctrl_r() & 7);
	}
	if (m_dbc & (1 << 18))
	{
		// Data
		jump &= scripts_data_compare();
	}

	if (!(m_dbc & (1 << 19)))
		jump = !jump;

	if (jump)
	{
		m_dsp = m_temp;
	}
	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  tc_int -
//-------------------------------------------------

void ncr53c700_device::tc_int()
{
	bool jump = true;

	if (m_dbc & (1 << 17))
	{
		// Phase
		jump &= (m_dcmd & 7) == (m_scsi_bus->ctrl_r() & 7);
	}
	if (m_dbc & (1 << 18))
	{
		// Data
		jump &= scripts_data_compare();
	}

	if (!(m_dbc & (1 << 19)))
		jump = !jump;

	if (jump)
	{
		m_dstat |= DSTAT_SIR;
		update_irqs();
		set_scripts_state(SCRIPTS_IDLE);
	}
	else
	{
		set_scripts_state(SCRIPTS_FETCH);
	}
}


//**************************************************************************
//  SCSI SCRIPTS DISASSEMBLY
//**************************************************************************

//-------------------------------------------------
//  disassemble_scripts -
//-------------------------------------------------

std::string ncr53c700_device::disassemble_scripts()
{
	static char const *const phases[] =
	{
		"Data Out",
		"Data In",
		"Command",
		"Status",
		"Reserved",
		"Reserved",
		"Message Out",
		"Message In"
	};

	std::string opstring;

	switch ((m_dcmd >> 6) & 3)
	{
		case 0:
		{
			opstring = util::string_format("BMOV: %s [%x] %d bytes\n", phases[m_dcmd & 7], m_dnad, m_dbc);
			break;
		}
		case 1:
		{
			static char const *const ops[] =
			{
				"SELECT",
				"DISCONNECT",
				"RESELECT",
				"SET",
				"CLEAR",
				"ILLEGAL",
				"ILLEGAL",
				"ILLEGAL",
			};

			opstring = util::string_format("IO: %s (%x)\n", ops[(m_dcmd >> 3) & 7], m_dnad);
			break;
		}
		case 2:
		{
			static char const *const ops[] =
			{
				"JUMP",
				"CALL",
				"RETURN",
				"INT",
				"ILLEGAL",
				"ILLEGAL",
				"ILLEGAL",
				"ILLEGAL",
			};

			opstring = util::string_format("TC: %s %c (%s) (%x)\n", ops[(m_dcmd >> 3) & 7], m_dbc & (1 << 19) ? 'T' : 'F', phases[m_dcmd & 7], m_dnad);
			break;
		}
		case 3:
		{
			opstring = "ILLEGAL";
			break;
		}
	}

	return util::string_format("SCRIPTS [%08x]: %s", m_dsp - 8, opstring);
}


//-------------------------------------------------
//  disassemble_scripts - add 53C710-only SCRIPTS
//  instruction groups
//-------------------------------------------------

std::string ncr53c710_device::disassemble_scripts()
{
	if (((m_dcmd >> 6) & 3) == 3)
		return util::string_format("SCRIPTS [%08x]: MMOV: [%08x] -> [%08x]\n", m_dsp - 12, m_dsps, m_temp);

	if ((((m_dcmd >> 6) & 3) == 1) && (((m_dcmd >> 3) & 7) >= 5))
	{
		static char const *const opcodes[] = { "SFBR->REG", "REG->SFBR", "RMW" };
		static char const *const operators[] = { "MOV", "OR", "AND", "ADD" };

		return util::string_format(
			"SCRIPTS [%08x]: RW: %s %s reg=%02x data=%02x\n",
			m_dsp - 8,
			opcodes[((m_dcmd >> 3) & 7) - 5],
			operators[(m_dcmd >> 1) & 3],
			(m_dbc >> 16) & 0x3f,
			(m_dbc >> 8) & 0xff);
	}

	return ncr53c700_device::disassemble_scripts();
}
