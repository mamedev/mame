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
#define LOG_INT     (1U <<  4)

//#define VERBOSE  (LOG_SETUP|LOG_READ|LOG_SERIAL|LOG_INT)
#define LOG_OUTPUT_FUNC printf // Needs always to be enabled as the default value 'logerror' is not available here

#include "logmacro.h"

//#define LOG(...) LOGMASKED(LOG_GENERAL,   __VA_ARGS__) // Already defined in logmacro.h
#define LOGSETUP(...)  LOGMASKED(LOG_SETUP,  __VA_ARGS__)
#define LOGR(...)      LOGMASKED(LOG_READ,   __VA_ARGS__)
#define LOGSERIAL(...) LOGMASKED(LOG_SERIAL, __VA_ARGS__)
#define LOGINT(...)    LOGMASKED(LOG_INT,    __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

void mc68340_serial_module_device::device_start()
{
	m_cpu = downcast<m68340_cpu_device *>(owner());
	mc68340_duart_device::device_start();
}

uint8_t mc68340_serial_module_device::read(offs_t offset)
{
	LOG("%s\n", FUNCNAME);
	int val = 0;

	LOGR("%08x %s %08x\n", m_cpu->pcbase(), FUNCNAME, offset);

	/*Setting the STP bit stops all clocks within the serial module (including the crystal
	  or external clock and SCLK), except for the clock from the IMB. The clock from the IMB
	  remains active to allow CPU32 access to the MCR. The clock stops on the low phase of the
	  clock and remains stopped until the STP bit is cleared by the CPU32 or a hardware reset.
	  Accesses to serial module registers while in stop mode produce a bus error.   */
	if ( (m_mcrh & REG_MCRH_STP) && offset != REG_MCRH && offset != REG_MCRL)
	{
		logerror("Attempt to access timer registers while timer clocks are stopped, STP bit in MCR is set!");
		return val; // TODO: Should cause BUSERROR
	}

	switch (offset)
	{
	case REG_MCRH:
		val = m_mcrh;
		LOGSERIAL("- %08x %s %04x, %04x (MCRH - Module Configuration Register High byte)\n", m_cpu->pcbase(), FUNCNAME, offset, val);
		break;
	case REG_MCRL:
		val = m_mcrl;
		LOGSERIAL("- %08x %s %04x, %04x (MCRL - Module Configuration Register Low byte)\n", m_cpu->pcbase(), FUNCNAME, offset, val);
		break;
	case REG_ILR:
		val = m_ilr;
		LOGSERIAL("- %08x %s %04x, %04x (ILR - Interrupt Level Register)\n", m_cpu->pcbase(), FUNCNAME, offset, val);
		break;
	case REG_IVR:
		val = m_ivr;
		LOGSERIAL("- %08x %s %04x, %04x (IVR - Interrupt Vector Register)\n", m_cpu->pcbase(), FUNCNAME, offset, val);
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

	return offset >= 0x10 && offset < 0x22 ? mc68340_duart_device::read(offset - 0x10) : val;
}

void mc68340_serial_module_device::write(offs_t offset, uint8_t data)
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
	  Accesses to serial module registers while in stop mode produce a bus error.   */
	if ( (m_mcrh & REG_MCRH_STP) && offset != REG_MCRH && offset != REG_MCRL)
	{
		logerror("Attempt to access timer registers while timer clocks are stopped, STP bit in MCR is set!");
		return; // TODO: Should cause BUSERROR
	}

	switch (offset)
	{
	case REG_MCRH:
		m_mcrh = data;
		LOGSERIAL("PC: %08x %s %04x, %04x (MCRH - Module Configuration Register High byte)\n", m_cpu->pcbase(), FUNCNAME, offset, data);
		LOGSERIAL("- Clocks are %s\n", data & REG_MCRH_STP ? "stopped" : "running");
		LOGSERIAL("- Freeze signal %s - not implemented\n", data & REG_MCRH_FRZ1 ? "stops at character boundary" : "is ignored");
		LOGSERIAL("- CTS capture clock: %s - not implemented\n", data & REG_MCRH_ICCS ? "SCLK" : "Crystal");
		break;
	case REG_MCRL:
		m_mcrl = data;
		LOGSERIAL("PC: %08x %s %04x, %04x (MCRL - Module Configuration Register Low byte)\n", m_cpu->pcbase(), FUNCNAME, offset, data);
		LOGSERIAL("- Supervisor registers %s - not implemented\n", data & REG_MCRL_SUPV ? "requries supervisor privileges" : "can be accessed by user privileged software");
		LOGSERIAL("- Interrupt Arbitration level: %02x\n", data & REG_MCRL_ARBLV);
		break;
	case REG_ILR:
		m_ilr = data;
		LOGSERIAL("PC: %08x %s %04x, %04x (ILR - Interrupt Level Register)\n", m_cpu->pcbase(), FUNCNAME, offset, data);
		LOGSERIAL("- Interrupt Level: %02x\n", data & REG_ILR_MASK);
		m_cpu->update_ipl();
		break;
	case REG_IVR:
		m_ivr = data;
		LOGSERIAL("PC: %08x %s %04x, %04x (IVR - Interrupt Vector Register)\n", m_cpu->pcbase(), FUNCNAME, offset, data);
		LOGSERIAL("- Interrupt Vector: %02x\n", data);
		break;
	default:
		if (offset >= 0x10 && offset < 0x22) mc68340_duart_device::write(offset - 0x10, data);
	}

}

WRITE_LINE_MEMBER( mc68340_serial_module_device::irq_w )
{
	LOGINT("IRQ!\n%s\n", FUNCNAME);
	m_cpu->update_ipl();
}

mc68340_serial_module_device::mc68340_serial_module_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
  : mc68340_duart_device(mconfig, MC68340_SERIAL_MODULE, tag, owner, clock)
{
}

void mc68340_serial_module_device::module_reset()
{
	mc68340_duart_device::device_reset();
}

DEFINE_DEVICE_TYPE(MC68340_SERIAL_MODULE, mc68340_serial_module_device, "mc68340sermod", "MC68340 Serial Module")
