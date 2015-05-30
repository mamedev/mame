// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII memory mapped I/O hardware
 *
 *****************************************************************************/
#include "alto2cpu.h"
#include "a2roms.h"

/**
 * @brief read printer paper ready bit
 * Paper ready bit. 0 when the printer is ready for a paper scrolling operation.
 */
READ16_MEMBER ( alto2_cpu_device::pprdy_r ) { return X_RDBITS(m_hw.utilin,16,0,0); }

/**
 * @brief read printer check bit
 * Printer check bit bit.
 * Should the printer find itself in an abnormal state, it sets this bit to 0
 */
READ16_MEMBER ( alto2_cpu_device::pcheck_r ) { return X_RDBITS(m_hw.utilin,16,1,1); }

/**
 * @brief read unused bit 2
 */
READ16_MEMBER ( alto2_cpu_device::unused2_r ) { return X_RDBITS(m_hw.utilin,16,2,2); }

/**
 * @brief read printer daisy ready bit
 * Daisy ready bit. 0 when the printer is ready to print a character.
 */
READ16_MEMBER ( alto2_cpu_device::pchrdy_r ) { return X_RDBITS(m_hw.utilin,16,3,3); }

/**
 * @brief read printer carriage ready bit
 * Carriage ready bit. 0 when the printer is ready for horizontal positioning.
 */
READ16_MEMBER ( alto2_cpu_device::parrdy_r ) { return X_RDBITS(m_hw.utilin,16,4,4); }

/**
 * @brief read printer ready bit
 * Ready bit. Both this bit and the appropriate other ready bit (carriage,
 * daisy, etc.) must be 0 before attempting any output operation.
 */
READ16_MEMBER ( alto2_cpu_device::pready_r ) { return X_RDBITS(m_hw.utilin,16,5,5); }

/**
 * @brief memory configuration switch
 */
READ16_MEMBER ( alto2_cpu_device::memconfig_r ) { return X_RDBITS(m_hw.utilin,16,6,6); }

/**
 * @brief get unused bit 7
 */
READ16_MEMBER ( alto2_cpu_device::unused7_r ) { return X_RDBITS(m_hw.utilin,16,7,7); }

/**
 * @brief get key set key 0
 */
READ16_MEMBER ( alto2_cpu_device::keyset_key0_r ) { return X_RDBITS(m_hw.utilin,16,8,8); }

/**
 * @brief get key set key 1
 */
READ16_MEMBER ( alto2_cpu_device::keyset_key1_r ) { return X_RDBITS(m_hw.utilin,16,9,9); }

/**
 * @brief get key set key 2
 */
READ16_MEMBER ( alto2_cpu_device::keyset_key2_r ) { return X_RDBITS(m_hw.utilin,16,10,10); }

/**
 * @brief get key set key 3
 */
READ16_MEMBER ( alto2_cpu_device::keyset_key3_r ) { return X_RDBITS(m_hw.utilin,16,11,11); }

/**
 * @brief get key set key 4
 */
READ16_MEMBER ( alto2_cpu_device::keyset_key4_r ) { return X_RDBITS(m_hw.utilin,16,12,12); }

/**
 * @brief get mouse red button bit
 */
READ16_MEMBER ( alto2_cpu_device::mouse_red_r ) { return X_RDBITS(m_hw.utilin,16,13,13); }

/**
 * @brief get mouse blue button bit
 */
READ16_MEMBER ( alto2_cpu_device::mouse_blue_r ) { return X_RDBITS(m_hw.utilin,16,14,14); }

/**
 * @brief get mouse yellow button bit
 */
READ16_MEMBER ( alto2_cpu_device::mouse_yellow_r ) { return X_RDBITS(m_hw.utilin,16,15,15); }

/**
 * @brief write printer paper ready bit
 */
WRITE16_MEMBER( alto2_cpu_device::pprdy_w ) { X_WRBITS(m_hw.utilin,16,0,0,data); }

/**
 * @brief write printer check bit
 */
WRITE16_MEMBER( alto2_cpu_device::pcheck_w ) { X_WRBITS(m_hw.utilin,16,1,1,data); }

/**
 * @brief read unused bit 2
 */
WRITE16_MEMBER( alto2_cpu_device::unused2_w ) { X_WRBITS(m_hw.utilin,16,2,2,data); }

/**
 * @brief write printer daisy ready bit
 */
WRITE16_MEMBER( alto2_cpu_device::pchrdy_w ) { X_WRBITS(m_hw.utilin,16,3,3,data); }

/**
 * @brief write printer carriage ready bit
 */
WRITE16_MEMBER( alto2_cpu_device::parrdy_w ) { X_WRBITS(m_hw.utilin,16,4,4,data); }

/**
 * @brief write printer ready bit
 */
WRITE16_MEMBER( alto2_cpu_device::pready_w ) { X_WRBITS(m_hw.utilin,16,5,5,data); }

/**
 * @brief write memory configuration switch
 */
WRITE16_MEMBER( alto2_cpu_device::memconfig_w ) { X_WRBITS(m_hw.utilin,16,6,6,data); }

/**
 * @brief write unused bit 7
 */
WRITE16_MEMBER( alto2_cpu_device::unused7_w ) { X_WRBITS(m_hw.utilin,16,7,7,data); }

/**
 * @brief write key set key 0
 */
WRITE16_MEMBER( alto2_cpu_device::keyset_key0_w ) { X_WRBITS(m_hw.utilin,16,8,8,data); }

/**
 * @brief write key set key 1
 */
WRITE16_MEMBER( alto2_cpu_device::keyset_key1_w ) { X_WRBITS(m_hw.utilin,16,9,9,data); }

/**
 * @brief write key set key 2
 */
WRITE16_MEMBER( alto2_cpu_device::keyset_key2_w ) { X_WRBITS(m_hw.utilin,16,10,10,data); }

/**
 * @brief write key set key 3
 */
WRITE16_MEMBER( alto2_cpu_device::keyset_key3_w ) { X_WRBITS(m_hw.utilin,16,11,11,data); }

/**
 * @brief write key set key 4
 */
WRITE16_MEMBER( alto2_cpu_device::keyset_key4_w ) { X_WRBITS(m_hw.utilin,16,12,12,data); }

/**
 * @brief write mouse red button bit
 */
WRITE16_MEMBER( alto2_cpu_device::mouse_red_w ) { X_WRBITS(m_hw.utilin,16,13,13,data); }

/**
 * @brief write mouse blue button bit
 */
WRITE16_MEMBER( alto2_cpu_device::mouse_blue_w ) { X_WRBITS(m_hw.utilin,16,14,14,data); }

/**
 * @brief write mouse yellow button bit
 */
WRITE16_MEMBER( alto2_cpu_device::mouse_yellow_w ) { X_WRBITS(m_hw.utilin,16,15,15,data); }

/**
 * @brief write mouse buttons bits
 */
WRITE16_MEMBER( alto2_cpu_device::mouse_buttons_w ) { X_WRBITS(m_hw.utilin,16,13,15,data); }

/**
 * @brief printer paper strobe bit
 * Paper strobe bit. Toggling this bit causes a paper scrolling operation.
 */
//static inline UINT16 GET_PPPSTR(UINT16 utilout) { return X_RDBITS(utilout,16,0,0); }

/**
 * @brief printer retstore bit
 * Restore bit. Toggling this bit resets the printer (including clearing
 * the "check" condition if present) and moves the carriage to the
 * left margin.
 */
//static inline UINT16 GET_PREST(UINT16 utilout) { return X_RDBITS(utilout,16,1,1); }

/**
 * @brief printer ribbon bit
 * Ribbon bit. When this bit is 1 the ribbon is up (in printing
 * position); when 0, it is down.
 */
//static inline UINT16 GET_PRIB(UINT16 utilout) { return X_RDBITS(utilout,16,2,2); }

/**
 * @brief printer daisy strobe bit
 * Daisy strobe bit. Toggling this bit causes a character to be printed.
 */
//static inline UINT16 GET_PCHSTR(UINT16 utilout) { return X_RDBITS(utilout,16,3,3); }

/**
 * @brief printer carriage strobe bit
 * Carriage strobe bit. Toggling this bit causes a horizontal position operation.
 */
//static inline UINT16 GET_PCARSTR(UINT16 utilout) { return X_RDBITS(utilout,16,4,4); }

/**
 * @brief printer data
 * Argument to various output operations:
 * 1. Printing characters. When the daisy bit is toggled bits 9-15 of this field
 * are interpreted as an ASCII character code to be printed (it should be noted
 * that all codes less than 040 print as lower case "w").
 * 2. For paper and carriage operations the field is interpreted as a displacement
 * (-1024 to +1023), in units of 1/48 inch for paper and 1/60 inch for carriage.
 * Positive is down or to the right, negative up or to the left. The value is
 * represented as sign-magnitude (i.e., bit 5 is 1 for negative numbers, 0 for
 * positive; bits 6-15 are the absolute value of the number).
 */
//static inline UINT16 GET_PDATA(UINT16 utilout) { return X_RDBITS(utilout,16,5,15); }

/**
 * @brief read the UTILIN port
 *
 * @param addr memory mapped I/O address to be read
 * @return current value on the UTILIN port
 */
READ16_MEMBER( alto2_cpu_device::utilin_r )
{
	UINT16  data;
	// FIXME: update the printer status
	// printer_read();

	data = m_hw.utilin;

	if (!space.debugger_access()) {
		LOG((LOG_HW,2," UTILIN rd %#o (%#o)\n", offset, data));
	}
	return data;
}

/**
 * @brief read the XBUS port
 *
 * @param addr memory mapped I/O address to be read
 * @return current value on the XBUS port latch
 */
READ16_MEMBER( alto2_cpu_device::xbus_r )
{
	UINT16 data = m_hw.xbus[offset & 3];

	if (!space.debugger_access()) {
		LOG((LOG_HW,2," XBUS[%d] rd %#o (%#o)\n", offset & 3, offset, data));
	}
	return data;
}

/**
 * @brief write the XBUS port
 *
 * The actual outputs are active-low.
 *
 * @param addr memory mapped I/O address to be read
 * @param data value to write to the XBUS port latch
 */
WRITE16_MEMBER( alto2_cpu_device::xbus_w )
{
	if (!space.debugger_access()) {
		LOG((LOG_HW,2," XBUS[%d] wr %#o (%#o)\n", offset & 3, offset, data));
	}
	m_hw.xbus[offset&3] = data;
}

/**
 * @brief read the UTILOUT port
 *
 * @param addr memory mapped I/O address to be read
 * @return current value on the UTILOUT port latch
 */
READ16_MEMBER( alto2_cpu_device::utilout_r )
{
	UINT16 data = m_hw.utilout ^ 0177777;
	if (!space.debugger_access()) {
		LOG((0,2,"  UTILOUT rd %#o (%#o)\n", offset, data));
	}
	return data;
}

/**
 * @brief write the UTILOUT port
 *
 * The actual outputs are active-low.
 *
 * @param addr memory mapped I/O address to be read
 * @param data value to write to the UTILOUT port latch
 */
WRITE16_MEMBER( alto2_cpu_device::utilout_w )
{
	if (!space.debugger_access()) {
		LOG((LOG_HW,2," UTILOUT wr %#o (%#o)\n", offset, data));
	}
	m_hw.utilout = data ^ 0177777;

	// FIXME: write printer data
	// printer_write();
}

/**
 * <PRE>
 * TODO: use madr.a65 and madr.a64 to determine the actual I/O address ranges
 *
 * madr.a65
 *  address line    connected to
 *  -------------------------------
 *  A0      MAR[11]
 *  A1      KEYSEL
 *  A2      MAR[7-10] == 0
 *  A3      MAR[12]
 *  A4      MAR[13]
 *  A5      MAR[14]
 *  A6      MAR[15]
 *  A7      IOREF (MAR[0-6] == 1)
 *
 *  output data connected to
 *  -------------------------------
 *  D0      IOSEL0
 *  D1      IOSEL1
 *  D2      IOSEL2
 *  D3      INTIO
 *
 * madr.a64
 *  address line    connected to
 *  -------------------------------
 *  A0      STORE
 *  A1      MAR[11]
 *  A2      MAR[7-10] == 0
 *  A3      MAR[12]
 *  A4      MAR[13]
 *  A5      MAR[14]
 *  A6      MAR[15]
 *  A7      IOREF (MAR[0-6] == 1)
 *
 *  output data connected to
 *  -------------------------------
 *  D0      & MISYSCLK -> SELP
 *  D1      ^ INTIO -> INTIOX
 *  "       ^ 1 -> NERRSEL
 *  "       & WRTCLK -> NRSTE
 *  D2      XREG'
 *  D3      & MISYSCLK -> LOADERC
 * </PRE>
 */

static const prom_load_t pl_madr_a64 =
{
	"madr.a64",
	0,
	"a66b0eda",
	"4d9088f592caa3299e90966b17765be74e523144",
	/* size */  0400,
	/* amap */  AMAP_DEFAULT,
	/* axor */  0,
	/* dxor */  017,                        // invert D0-D3
	/* width */ 4,
	/* shift */ 0,
	/* dmap */  DMAP_DEFAULT,
	/* dand */  ZERO,
	/* type */  sizeof(UINT8)
};

static const prom_load_t pl_madr_a65 =
{
	"madr.a65",
	0,
	"ba37febd",
	"82e9db1cb65f451755295f0d179e6f8fe3349d4d",
	/* size */  0400,
	/* amap */  AMAP_DEFAULT,
	/* axor */  0,
	/* dxor */  017,                        // invert D0-D3
	/* width */ 4,
	/* shift */ 0,
	/* dmap */  DMAP_DEFAULT,
	/* dand */  ZERO,
	/* type */  sizeof(UINT8)
};

/**
 * @brief clear all keys and install the mmio handler for KBDAD to KBDAD+3
 */
void alto2_cpu_device::init_hw()
{
	memset(&m_hw, 0, sizeof(m_hw));
	m_madr_a64 = prom_load(machine(), &pl_madr_a64, memregion("madr_a64")->base());
	m_madr_a65 = prom_load(machine(), &pl_madr_a65, memregion("madr_a65")->base());
}

void alto2_cpu_device::exit_hw()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_hw()
{
	m_hw.eia = 0;
	m_hw.utilout = 0;
	// open inputs on the XBUS (?)
	m_hw.xbus[0] = 0177777;
	m_hw.xbus[1] = 0177777;
	m_hw.xbus[2] = 0177777;
	m_hw.xbus[3] = 0177777;
	// open inputs on UTILIN
	m_hw.utilin = 0177777;
}
