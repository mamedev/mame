// license:BSD-3-Clause
// copyright-holders:David Haywood, Joakim Larsson Edstrom
/* 68340 SERIAL module */

#include "emu.h"
#include "68340.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//#define LOG_GENERAL (1U <<  0) // Already defined in logmacro.h
#define LOG_SETUP   (1U <<  1)
#define LOG_READ    (1U <<  2)
#define LOG_SERIAL  (1U <<  3)

#define VERBOSE  (LOG_SETUP|LOG_READ|LOG_SERIAL)
#define LOG_OUTPUT_FUNC printf // Needs always to be enabled as the default value 'logerror' is not available here

#include "logmacro.h"

//#define LOG(...) LOGMASKED(LOG_GENERAL,   __VA_ARGS__) // Already defined in logmacro.h
#define LOGSETUP(...)  LOGMASKED(LOG_SETUP,  __VA_ARGS__)
#define LOGR(...)      LOGMASKED(LOG_READ,   __VA_ARGS__)
#define LOGSERIAL(...) LOGMASKED(LOG_SERIAL, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

READ8_MEMBER( m68340_serial::read )
{
	LOG("%s\n", FUNCNAME);
	int val = 0;

	LOGR("%08x %s %08x, (%08x)\n", space.device().safe_pc(), FUNCNAME, offset, mem_mask);

	/*Setting the STP bit stops all clocks within the serial module (including the crystal 
	  or external clock and SCLK), except for the clock from the IMB. The clock from the IMB 
	  remains active to allow CPU32 access to the MCR. The clock stops on the low phase of the 
	  clock and remains stopped until the STP bit is cleared by the CPU32 or a hardware reset. 
	  Accesses to serial module registers while in stop mode produce a bus error.	*/
	if ( (m_mcrh & REG_MCRH_STP) && offset != REG_MCRH && offset != REG_MCRL)
	{
		logerror("Attempt to access timer registers while timer clocks are stopped, STP bit in MCR is set!");
		return val; // TODO: Should cause BUSERROR
	}

	switch (offset)
	{
	case REG_MCRH:
		val = m_mcrh;
		LOGSERIAL("- %08x %s %04x, %04x (%04x) (MCRH - Module Configuration Register High byte)\n", space.device().safe_pc(), FUNCNAME, offset, val, mem_mask);
		break;
	case REG_MCRL:
		val = m_mcrl;
		LOGSERIAL("- %08x %s %04x, %04x (%04x) (MCRL - Module Configuration Register Low byte)\n", space.device().safe_pc(), FUNCNAME, offset, val, mem_mask);
		break;
	case REG_ILR:
		val = m_ilr;
		LOGSERIAL("- %08x %s %04x, %04x (%04x) (ILR - Interrupt Level Register)\n", space.device().safe_pc(), FUNCNAME, offset, val, mem_mask);
		break;
	case REG_IVR:
		val = m_ivr;
		LOGSERIAL("- %08x %s %04x, %04x (%04x) (IVR - Interrupt Vector  Register)\n", space.device().safe_pc(), FUNCNAME, offset, val, mem_mask);
		break;
	}


	LOGR(" * Reg %02x -> %02x - %s\n", offset, val,
		(offset > 0x21) ? "Error - should not happen" :
	     std::array<char const *, 0x22>
	     {{
		 "MCRH", "MCRL", "n/a",  "n/a",  "ILR",  "IVR",  "hole", "hole", // 0x00 - 0x07
		 "hole", "hole", "hole", "hole", "hole", "hole", "hole", "hole", // 0x08 - 0x0f
		 "MR1A", "SRA",  "n/a",  "RBA",  "IPCR", "ISR",  "n/a",  "n/a",  // 0x10 - 0x17
		 "MR1B", "SRB",  "n/a",  "RBB",  "n/a",  "IP",   "n/a",  "n/a",  // 0x18 - 0x1f
		 "MR2A", "MR2B" }}[offset]);                                     // 0x20 - 0x21
	
	return offset >= 0x10 && offset < 0x22 ? m_duart->read(space, offset - 0x10, mem_mask) : val;
}

WRITE8_MEMBER( m68340_serial::write )
{
	LOG("\n%s\n", FUNCNAME);
	LOGSETUP(" * Reg %02x <- %02x - %s\n", offset, data,
		 (offset > 0x21) ? "Error - should not happen" :
		 std::array<char const *, 0x22>
		 {{
		     "MCRH", "MCRL", "n/a",  "n/a",  "ILR",  "IVR",  "hole", "hole", // 0x00 - 0x07
		     "hole", "hole", "hole", "hole", "hole", "hole", "hole", "hole", // 0x08 - 0x0f
		     "MR1A", "CSRA", "CRA",  "TBA",  "ACR",  "IER",  "n/a",  "n/a",  // 0x10 - 0x17
		     "MR1B", "CSRB", "CRB",  "TBB",  "n/a",  "OPCR", "OPS",  "OPR",  // 0x18 - 0x1f
		     "MR2A", "MR2B" }}[offset]);                                     // 0x20 - 0x21

	/*Setting the STP bit stops all clocks within the serial module (including the crystal 
	  or external clock and SCLK), except for the clock from the IMB. The clock from the IMB 
	  remains active to allow CPU32 access to the MCR. The clock stops on the low phase of the 
	  clock and remains stopped until the STP bit is cleared by the CPU32 or a hardware reset. 
	  Accesses to serial module registers while in stop mode produce a bus error.	*/
	if ( (m_mcrh & REG_MCRH_STP) && offset != REG_MCRH && offset != REG_MCRL)
	{
		logerror("Attempt to access timer registers while timer clocks are stopped, STP bit in MCR is set!");
		return; // TODO: Should cause BUSERROR
	}

	switch (offset)
	{
	case REG_MCRH:
		m_mcrh = data;
		LOGSERIAL("PC: %08x %s %04x, %04x (%04x) (MCRH - Module Configuration Register High byte)\n", space.device().safe_pc(), FUNCNAME, offset, data, mem_mask);
		LOGSERIAL("- Clocks are %s\n", data & REG_MCRH_STP ? "stopped" : "running");
		LOGSERIAL("- Freeze signal %s - not implemented\n", data & REG_MCRH_FRZ1 ? "stops at character boundary" : "is ignored");
		LOGSERIAL("- CTS capture clock: %s - not implemented\n", data & REG_MCRH_ICCS ? "SCLK" : "Crystal");
		break;
	case REG_MCRL:
		m_mcrl = data;
		LOGSERIAL("PC: %08x %s %04x, %04x (%04x) (MCRL - Module Configuration Register Low byte)\n", space.device().safe_pc(), FUNCNAME, offset, data, mem_mask);
		LOGSERIAL("- Supervisor registers %s - not implemented\n", data & REG_MCRL_SUPV ? "requries supervisor privileges" : "can be accessed by user privileged software");
		LOGSERIAL("- Interrupt Arbitration level: %02x - not implemented\n", data & REG_MCRL_ARBLV);
		break;
	case REG_ILR:
		m_ilr = data;
		LOGSERIAL("PC: %08x %s %04x, %04x (%04x) (ILR - Interrupt Level Register)\n", space.device().safe_pc(), FUNCNAME, offset, data, mem_mask);
		LOGSERIAL("- Interrupt Level: %02x - not implemented\n", data & REG_ILR_MASK);
		break;
	case REG_IVR:
		m_ivr = data;
		LOGSERIAL("PC: %08x %s %04x, %04x (%04x) (IVR - Interrupt Vector Register)\n", space.device().safe_pc(), FUNCNAME, offset, data, mem_mask);
		LOGSERIAL("- Interrupt Vector: %02x - not implemented\n", data);
		break;
	default:
		if (offset >= 0x10 && offset < 0x22) m_duart->write(space, offset - 0x10, data, mem_mask);
	}

}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
MACHINE_CONFIG_MEMBER( m68340_serial::device_add_mconfig )
	MCFG_DEVICE_ADD("duart", M68340SERIAL, 0)
MACHINE_CONFIG_END

m68340_serial::m68340_serial(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, M68340_SERIAL_MODULE, tag, owner, clock),
	m_duart(*this, "duart")
{
}

void m68340_serial::device_start()
{
	LOGSETUP("%s\n", FUNCNAME);
}

void m68340_serial::device_reset()
{
	LOGSETUP("%s %s \n",tag(), FUNCNAME);

	// Do channel reset
	m_duart->reset();
}

DEFINE_DEVICE_TYPE(M68340_SERIAL_MODULE, m68340_serial,   "m68340 serial module",         "Motorola 68340 Serial Module")
