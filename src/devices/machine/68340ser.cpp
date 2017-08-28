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

#define VERBOSE  (LOG_SETUP|LOG_READ)
#define LOG_OUTPUT_FUNC printf // Needs always to be enabled as the default value 'logerror' is not available here

#include "logmacro.h"

//#define LOG(...) LOGMASKED(LOG_GENERAL,   __VA_ARGS__) // Already defined in logmacro.h
#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,  __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

READ8_MEMBER( m68340_cpu_device::m68340_internal_serial_r )
{	LOG("%s\n", FUNCNAME);
  //	assert(m68340SERIAL);
	//m68340_serial &serial = *m68340SERIAL;
	int val = 0;
	//int pc = space.device().safe_pc();
	
	//LOGSETUP("%08x m68340_internal_serial_r %08x, (%08x)\n", pc, offset, mem_mask);

	LOGR(" * Reg %02x -> %02x - %s\n", offset, val,
		(offset > 0x21) ? "Error - should not happen" :
	     std::array<char const *, 0x22>
	     {{
		 "MCRH", "MCRL", "n/a",  "n/a",  "ILR",  "IVR",  "hole", "hole", // 0x00 - 0x07
		 "hole", "hole", "hole", "hole", "hole", "hole", "hole", "hole", // 0x08 - 0x0f
		 "MR1A", "SRA",  "n/a",  "RBA",  "IPCR", "ISR",  "n/a",  "n/a",  // 0x10 - 0x17
		 "MR1B", "SRB",  "n/a",  "RBB",  "n/a",  "IP",   "n/a",  "n/a",  // 0x18 - 0x1f
		 "MR2A", "MR2B" }}[offset]);                                     // 0x20 - 0x21

	return offset >= 0x10 && offset < 0x20 ? m_serial->m_duart->read(space, offset - 0x10, mem_mask) : 0;
}

WRITE8_MEMBER( m68340_cpu_device::m68340_internal_serial_w )
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

	if (offset >= 0x10 && offset < 0x20) m_serial->m_duart->write(space, offset - 0x10, data, mem_mask);

	//	assert(m68340SERIAL);
	//m68340_serial &serial = *m68340SERIAL;

	//int pc = space.device().safe_pc();
	//LOGSETUP("%08x m68340_internal_serial_w %08x, %08x (%08x)\n", pc, offset, data, mem_mask);
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
	// state saving
	//	save_item(NAME(m_duart));
}

void m68340_serial::device_reset()
{
	LOGSETUP("%s %s \n",tag(), FUNCNAME);

	// Do channel reset
	m_duart->reset();
}

DEFINE_DEVICE_TYPE(M68340_SERIAL_MODULE, m68340_serial,   "m68340 serial module",         "Motorola 68340 Serial Module")

//void m68340_serial::reset()
//{
//}
