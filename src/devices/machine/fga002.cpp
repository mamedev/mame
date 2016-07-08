// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************
*
*   Force Computer FGA-002 Force Gate Array
*
*	Documetation: http://bitsavers.informatik.uni-stuttgart.de/pdf/forceComputers/201559_FGA-002_Nov96.pdf
*
*	The FGA-002 gate array is a high speed CMOS device manufactured in 1.2 micron technology and containing 24,000 gates in a 281 pin PGA
*	package. It provides interfaces to the 68020/30 microprocessor as well as a VMEbus compatible interface.
*	The auxilary interface of the gate array is a high speed data channel used by the internal 32 bit DMA controller. The interface
*	allows data transfer rates of up to 6 MByte/second. The timing of the local I/O interface is programmable and provides
*	easy interfacing of local I/O devices. All control, address and data lines of the CPU and the VMEbus are either directly connected or 
*	connected via buffers to the gate array allowing easy implementation and usage. The gate array registers are programmed by the local CPU.
*
*	FEATURES:
*	- Programmable decoding for CPU and VME access to the local main memory
*	- Interrupt management for internal and external interrupt sources
*	- 32 bit multi-port DMA Controller 
*	- FORCE Message Broadcast slave interface with 2 message channels
*	- 8 interrupt capable MAILBOXES
*	- 8 bit TIMER with 16 selectable internal source clocks
*
*
*	CAUTION (from the documentation - no unducumented registers are currently emulated )
*	The FGA-002 gate array contains registers, which are used to configure the gate array for special external hardware
*	requirements. These registers are reserved and will be setup by the boot software according to the hardware environment in which the gate array is
*	implemented. These registers must not be changed by the user. Some of these hardware configuration registers also contain user selectable bits.
*	Programming the contents of these registers has to be done carefully without changing the bits initialized by the boot software.
*	Registers not described must not be programmed. Unqualified changes of register bits may have unpredictable consequences for the gate array and 
*	external hardware. It is expressly forbidden to change register bits, except those defined for the user.
*
*/
#include "fga002.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#define LOGR(x)
#if VERBOSE == 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#define LLFORMAT "%I64%"
#else
#define FUNCNAME __PRETTY_FUNCTION__
#define LLFORMAT "%lld"
#endif

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
// device type definition
const device_type FGA002   = &device_creator<fga002_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  fga002_device - constructor
//-------------------------------------------------

fga002_device::fga002_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

fga002_device::fga002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FGA002, "FGA-002", tag, owner, clock, "fga002", __FILE__)
{
}

void fga002_device::device_start()
{
	LOG(("%s\n", FUNCNAME));

	// Timers
	fga_timer = timer_alloc(TIMER_ID_FGA);

	save_pointer (NAME (m_fga002), sizeof(m_fga002));
}

void fga002_device::device_reset()
{
	LOG(("%s\n", FUNCNAME));
	/* Reset values for the FGA-002 */
	memset(&m_fga002[0], 0, sizeof(m_fga002));
	m_fga002[FGA_RSVMECALL]     = 0x80;
	m_fga002[FGA_RSKEYRES]      = 0x80;
	m_fga002[FGA_RSCPUCALL]     = 0x80;
	m_fga002[FGA_RSLOCSW]       = 0x80;
	m_fga002[FGA_ISTIM0]        = 0x80;
	m_fga002[FGA_ISDMANORM]     = 0x80;
	m_fga002[FGA_ISDMAERR]      = 0x80;
	m_fga002[FGA_ISFMB0REF]     = 0x80;
	m_fga002[FGA_ISFMB1REF]     = 0x80;
	m_fga002[FGA_ISPARITY]      = 0x80;
	m_fga002[FGA_ISABORT]       = 0x80;
	m_fga002[FGA_ISACFAIL]      = 0x80;
	m_fga002[FGA_ISSYSFAIL]     = 0x80;
	m_fga002[FGA_ISFMB0MES]     = 0x80;
	m_fga002[FGA_ISFMB1MES]     = 0x80;
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
void fga002_device::device_timer (emu_timer &timer, device_timer_id id, INT32 param, void *ptr)
{
	switch(id)
	{
	case TIMER_ID_FGA: 
		if (m_tim0count-- == 0) // Zero detect
	    {
			if ((m_fga002[FGA_TIM0CTL] & REG_TIM0CTL_ZERO_STOP) == 0)
			{
				fga_timer->adjust(attotime::never, TIMER_ID_FGA, attotime::never);
			}
			else 
			{
				if ((m_fga002[FGA_TIM0CTL] & REG_TIM0CTL_AUTOPRELOAD) == 0)
					m_tim0count &= 0xff;
				else
					m_tim0count = m_fga002[FGA_TIM0PRELOAD];
			}
		}
		break;
	default: 
		LOG(("Unhandled Timer ID %d\n", id)); 
		break; 
	}
}



/*	The FGA002 Timer

	FEATURES
	- 8 bit Synchronous Counter
	- 16 selectable clocks with frequencies from 1MHz to 0.5 Hz
	- Autopreload and Zerostop operating modes
	- Watchdog Timer operation
	- SYSFAIL and/or interrupt generation
	- Vectored interrupt
	- Interrupt levels selectable by software
*/

/*	Timer Preload Register TIM0PRELOAD
	The Timer Preload Register TIM0PRELOAD contains the preset value which can be loaded into the counter circuit. The default
	value of this register after reset is $00. The TIM0PRELOAD register can be read at any time but must not be altered if the
	timer is running.
	[7:0] The Timer Preload register contains the 8 bit value that is loaded into the counter if the
	Autopreload option in the TIM0CTL register is selected and the counter reaches the value zero.
	Also, if a write access to the TIM0COUNT register is performed, the counter is loaded with the value
	stored in the Timer Preload Register.
*/
void fga002_device::do_fga002reg_tim0preload_w(UINT8 data)
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
	m_fga002[FGA_TIM0PRELOAD] = data;
}

UINT8 fga002_device::do_fga002reg_tim0preload_r()
{
	LOG(("%s() %02x\n", FUNCNAME, m_fga002[FGA_TIM0PRELOAD]));
	return m_fga002[FGA_TIM0PRELOAD];
}

/*	Timer Control Register TIM0CTL
	In the Timer Control Register TIM0CTL the operating mode and the clock source of the timer can be selected. The Timer
	Control Register is grouped into two major fields. Bits 7-4 define the operating mode of the timer and the sysfail option.
	Bits 3-0 select the source clock applied to the timer. The TIM0CTL register is cleared to $00 after any reset operation.
	[7] Zerostop This bit selects whether the counter stops when reaching zero count or continues counting down. The value the
	counter will decrement to next depends on the setting of bit 6 of this register, which is the Autopreload bit.
	1 = The counter continues counting down.
	0 = The counter stops on zero count.
	[6] Autopreload This bit selects whether the counter rolls over from $00 to the value $FF and continues counting down or is
	preset by the contents of the timer preload register after reaching the zero count. The Autopreload option may be
	ignored if the counter is programmed to stop on zero count.
	1 The Autopreload option is enabled. When the counter has passed from $01 to $00, the value stored in the Preload
	register will be transferred to the counter on the first clock edge following the zero count clock. After
	that transfer the counter continues decrementing from the new value. 
	0 The Autopreload option is disabled. After the counter has reached zero it will roll over to the value $FF and
	continue counting down.
	[5] Sysfail This bit enables/disables the sysfail generation by the timer. If this option is enabled, the SFAILO output pin of the FGA-002
	gate array will be asserted low when the timer triggers the timer interrupt. The sysfail signal is negated when the timer
	interrupt is cleared. 1 = enabled 0 = disabled
	[4] Start/Stop: This bit controls the timer start and stop operation. 1 = start 0 = stop
	[3:0] Clock select This bitfield provides selection of the source clock for timer operation.
	3..0 source clock period
	0000   1 microsecond
	0001   2 microseconds
	0010   4 microseconds
	0011   8 microseconds
	0100  16 microseconds
	0101  32 microseconds
	0110  64 microseconds
	0111 128 microseconds
	1000 256 microseconds
	1001 512 microseconds
	1010   2 milliseconds
	1011   8 milliseconds
	1100  32 milliseconds
	1101 125 milliseconds
	1110 500 milliseconds
	1111   2 seconds
*/

void fga002_device::do_fga002reg_tim0ctl_w(UINT8 data)
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
	if ((data  & REG_TIM0CTL_START_STOP) != (m_fga002[FGA_TIM0CTL] & REG_TIM0CTL_START_STOP))
	{
		if ((data & REG_TIM0CTL_START_STOP) == 0)
			fga_timer->adjust(attotime::never, TIMER_ID_FGA, attotime::never);
		else
		{
			switch (data & REG_TIM0CTL_CLK_MSK)
			{
			case REG_TIM0CTL_CLK_1_MIC: fga_timer->adjust(attotime::from_usec(1), TIMER_ID_FGA, attotime::from_usec(1)); break;
			case REG_TIM0CTL_CLK_2_MIC: fga_timer->adjust(attotime::from_usec(2), TIMER_ID_FGA, attotime::from_usec(2)); break;
			case REG_TIM0CTL_CLK_4_MIC: fga_timer->adjust(attotime::from_usec(4), TIMER_ID_FGA, attotime::from_usec(4)); break;
			case REG_TIM0CTL_CLK_8_MIC: fga_timer->adjust(attotime::from_usec(8), TIMER_ID_FGA, attotime::from_usec(8)); break;
			case REG_TIM0CTL_CLK_16_MIC: fga_timer->adjust(attotime::from_usec(16), TIMER_ID_FGA, attotime::from_usec(16)); break;
			case REG_TIM0CTL_CLK_32_MIC: fga_timer->adjust(attotime::from_usec(32), TIMER_ID_FGA, attotime::from_usec(32)); break;
			case REG_TIM0CTL_CLK_64_MIC: fga_timer->adjust(attotime::from_usec(64), TIMER_ID_FGA, attotime::from_usec(64)); break;
			case REG_TIM0CTL_CLK_128_MIC: fga_timer->adjust(attotime::from_usec(128), TIMER_ID_FGA, attotime::from_usec(128)); break;
			case REG_TIM0CTL_CLK_256_MIC: fga_timer->adjust(attotime::from_usec(256), TIMER_ID_FGA, attotime::from_usec(256)); break;
			case REG_TIM0CTL_CLK_512_MIC: fga_timer->adjust(attotime::from_usec(512), TIMER_ID_FGA, attotime::from_usec(512)); break;
			case REG_TIM0CTL_CLK_2_MIL: fga_timer->adjust(attotime::from_msec(2), TIMER_ID_FGA, attotime::from_msec(2)); break;
			case REG_TIM0CTL_CLK_8_MIL: fga_timer->adjust(attotime::from_msec(8), TIMER_ID_FGA, attotime::from_msec(8)); break;
			case REG_TIM0CTL_CLK_32_MIL: fga_timer->adjust(attotime::from_msec(32), TIMER_ID_FGA, attotime::from_msec(32)); break;
			case REG_TIM0CTL_CLK_125_MIL: fga_timer->adjust(attotime::from_msec(125), TIMER_ID_FGA, attotime::from_msec(125)); break;
			case REG_TIM0CTL_CLK_500_MIL: fga_timer->adjust(attotime::from_msec(500), TIMER_ID_FGA, attotime::from_msec(500)); break;
			case REG_TIM0CTL_CLK_2_SEC: fga_timer->adjust(attotime::from_seconds(2), TIMER_ID_FGA, attotime::from_seconds(2)); break;
			default: logerror("REG_TIM0CTL programmer error, please report\n"); break; // Should never happen
			}
		}
	}
	// TODO: Support SYSFAIL flag on interrupt
	m_fga002[FGA_TIM0CTL] = data;
}

UINT8 fga002_device::do_fga002reg_tim0ctl_r()
{
	LOG(("%s() %02x\n", FUNCNAME, m_fga002[FGA_TIM0CTL]));
	return m_fga002[FGA_TIM0CTL];
}

/*	Timer Count Register TIM0COUNT
	The Timer Count Register TIM0COUNT contains the current value of the timer/counter. A write access to this register will
	load the counter with the value stored in the Timer Preload Register. The written data will be ignored.
	It is permitted to perform read/write accesses to the Timer Count Register when the timer is running.
	The Timer Count Register is initialized to the value $FF after reset.
	[7:0] Timer Count Value
*/
void fga002_device::do_fga002reg_tim0count_w(UINT8 data)
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
	m_tim0count = m_fga002[FGA_TIM0PRELOAD];
}

UINT8 fga002_device::do_fga002reg_tim0count_r()
{
	LOG(("%s() %02x\n", FUNCNAME, m_tim0count));
	return m_tim0count;
}

/*	Timer Interrupt Control Register ICRTIM0
	Timer Interrupt Control is performed by the Timer Interrupt Control Register ICRTIM0 which enables/disables the interrupt
	and selects the interrupt level.
	[3] IRQ enable, 1 = timer interrupt channel enabled, 0 = disabled
	[2:0] IRQ level 000 = interrupt disabled 001-111 = Level 1 to 7  interrupt
*/
void fga002_device::do_fga002reg_icrtim0_w(UINT8 data)
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
	m_fga002[FGA_ICRTIM0] = data;
}

UINT8 fga002_device::do_fga002reg_icrtim0_r()
{
	LOG(("%s() %02x\n", FUNCNAME, m_fga002[FGA_ICRTIM0]));
	return m_fga002[FGA_ICRTIM0];
}

/*	Timer Interrupt Status Register ISTIM0
	ISTIM0 displays a pending timer interrupt. This bit is always readable and indicates 0 if the timer interrupt has been triggered. A write access to the
	ISTIM0 register clears the timer interrupt. The data written to this register will be ignored.
	[7] The IRQ Status register bit displays if a timer interrupt request is pending. 1 = no interrupt is pending. 0 = interrupt is pending
    [6:0] not used
*/
void fga002_device::do_fga002reg_istim0_w(UINT8 data)
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
	m_fga002[FGA_ISTIM0] &= ~REG_ISTIM0_TIM_INT; // Clear timer interrupt status
}

UINT8 fga002_device::do_fga002reg_istim0_r()
{
	LOG(("%s() %02x\n", FUNCNAME, m_fga002[FGA_ISTIM0]));
	return m_fga002[FGA_ISTIM0];
}


WRITE8_MEMBER (fga002_device::write){
	LOG(("%s[%04x] <- %02x    - ", FUNCNAME, offset, data));
	switch(offset)
	{
	case FGA_SPECIALENA     : LOG(("FGA_SPECIALENA - not implemented\n")); m_fga002[FGA_SPECIALENA] = data; break;
	case FGA_RSVMECALL      : LOG(("FGA_RSVMECALL - not implemented\n")); m_fga002[FGA_RSVMECALL] = data; break;
	case FGA_RSKEYRES       : LOG(("FGA_RSKEYRES - not implemented\n")); m_fga002[FGA_RSKEYRES] = data; break;
	case FGA_RSCPUCALL      : LOG(("FGA_RSCPUCALL - not implemented\n")); m_fga002[FGA_RSCPUCALL] = data; break;
	case FGA_RSLOCSW        : LOG(("FGA_RSLOCSW - not implemented\n")); m_fga002[FGA_RSLOCSW] = data; break;
	case FGA_ICRMBOX0       : LOG(("FGA_ICRMBOX0 - not implemented\n")); m_fga002[FGA_ICRMBOX0] = data; break;
	case FGA_ICRMBOX1       : LOG(("FGA_ICRMBOX1 - not implemented\n")); m_fga002[FGA_ICRMBOX1] = data; break;
	case FGA_ICRMBOX2       : LOG(("FGA_ICRMBOX2 - not implemented\n")); m_fga002[FGA_ICRMBOX2] = data; break;
	case FGA_ICRMBOX3       : LOG(("FGA_ICRMBOX3 - not implemented\n")); m_fga002[FGA_ICRMBOX3] = data; break;
	case FGA_ICRMBOX4       : LOG(("FGA_ICRMBOX4 - not implemented\n")); m_fga002[FGA_ICRMBOX4] = data; break;
	case FGA_ICRMBOX5       : LOG(("FGA_ICRMBOX5 - not implemented\n")); m_fga002[FGA_ICRMBOX5] = data; break;
	case FGA_ICRMBOX6       : LOG(("FGA_ICRMBOX6 - not implemented\n")); m_fga002[FGA_ICRMBOX6] = data; break;
	case FGA_ICRMBOX7       : LOG(("FGA_ICRMBOX7 - not implemented\n")); m_fga002[FGA_ICRMBOX7] = data; break;
	case FGA_VMEPAGE        : LOG(("FGA_VMEPAGE  - not implemented\n")); m_fga002[FGA_VMEPAGE ] = data; break;
	case FGA_ICRVME1        : LOG(("FGA_ICRVME1 - not implemented\n")); m_fga002[FGA_ICRVME1] = data; break;
	case FGA_ICRVME2        : LOG(("FGA_ICRVME2 - not implemented\n")); m_fga002[FGA_ICRVME2] = data; break;
	case FGA_ICRVME3        : LOG(("FGA_ICRVME3 - not implemented\n")); m_fga002[FGA_ICRVME3] = data; break;
	case FGA_ICRVME4        : LOG(("FGA_ICRVME4 - not implemented\n")); m_fga002[FGA_ICRVME4] = data; break;
	case FGA_ICRVME5        : LOG(("FGA_ICRVME5 - not implemented\n")); m_fga002[FGA_ICRVME5] = data; break;
	case FGA_ICRVME6        : LOG(("FGA_ICRVME6 - not implemented\n")); m_fga002[FGA_ICRVME6] = data; break;
	case FGA_ICRVME7        : LOG(("FGA_ICRVME7 - not implemented\n")); m_fga002[FGA_ICRVME7] = data; break;
	case FGA_ICRTIM0        : do_fga002reg_icrtim0_w(data); break;
	case FGA_ICRDMANORM     : LOG(("FGA_ICRDMANORM - not implemented\n")); m_fga002[FGA_ICRDMANORM] = data; break;
	case FGA_ICRDMAERR      : LOG(("FGA_ICRDMAERR - not implemented\n")); m_fga002[FGA_ICRDMAERR] = data; break;
	case FGA_CTL1           : LOG(("FGA_CTL1 - not implemented\n")); m_fga002[FGA_CTL1] = data; break;
	case FGA_CTL2           : LOG(("FGA_CTL2 - not implemented\n")); m_fga002[FGA_CTL2] = data; break;
	case FGA_ICRFMB0REF     : LOG(("FGA_ICRFMB0REF - not implemented\n")); m_fga002[FGA_ICRFMB0REF] = data; break;
	case FGA_ICRFMB1REF     : LOG(("FGA_ICRFMB1REF - not implemented\n")); m_fga002[FGA_ICRFMB1REF] = data; break;
	case FGA_ICRFMB0MES     : LOG(("FGA_ICRFMB0MES - not implemented\n")); m_fga002[FGA_ICRFMB0MES] = data; break;
	case FGA_ICRFMB1MES     : LOG(("FGA_ICRFMB1MES - not implemented\n")); m_fga002[FGA_ICRFMB1MES] = data; break;
	case FGA_CTL3           : LOG(("FGA_CTL3 - not implemented\n")); m_fga002[FGA_CTL3] = data; break;
	case FGA_CTL4           : LOG(("FGA_CTL4 - not implemented\n")); m_fga002[FGA_CTL4] = data; break;
	case FGA_ICRPARITY      : LOG(("FGA_ICRPARITY - not implemented\n")); m_fga002[FGA_ICRPARITY] = data; break;
	case FGA_AUXPINCTL      : LOG(("FGA_AUXPINCTL - not implemented\n")); m_fga002[FGA_AUXPINCTL] = data; break;
	case FGA_CTL5           : LOG(("FGA_CTL5 - not implemented\n")); m_fga002[FGA_CTL5] = data; break;
	case FGA_AUXFIFWEX      : LOG(("FGA_AUXFIFWEX - not implemented\n")); m_fga002[FGA_AUXFIFWEX] = data; break;
	case FGA_AUXFIFREX      : LOG(("FGA_AUXFIFREX - not implemented\n")); m_fga002[FGA_AUXFIFREX] = data; break;
	case FGA_CTL6           : LOG(("FGA_CTL6 - not implemented\n")); m_fga002[FGA_CTL6] = data; break;
	case FGA_CTL7           : LOG(("FGA_CTL7 - not implemented\n")); m_fga002[FGA_CTL7] = data; break;
	case FGA_CTL8           : LOG(("FGA_CTL8 - not implemented\n")); m_fga002[FGA_CTL8] = data; break;
	case FGA_CTL9           : LOG(("FGA_CTL9 - not implemented\n")); m_fga002[FGA_CTL9] = data; break;
	case FGA_ICRABORT       : LOG(("FGA_ICRABORT - not implemented\n")); m_fga002[FGA_ICRABORT] = data; break;
	case FGA_ICRACFAIL      : LOG(("FGA_ICRACFAIL - not implemented\n")); m_fga002[FGA_ICRACFAIL] = data; break;
	case FGA_ICRSYSFAIL     : LOG(("FGA_ICRSYSFAIL - not implemented\n")); m_fga002[FGA_ICRSYSFAIL] = data; break;
	case FGA_ICRLOCAL0      : LOG(("FGA_ICRLOCAL0 - not implemented\n")); m_fga002[FGA_ICRLOCAL0] = data; break;
	case FGA_ICRLOCAL1      : LOG(("FGA_ICRLOCAL1 - not implemented\n")); m_fga002[FGA_ICRLOCAL1] = data; break;
	case FGA_ICRLOCAL2      : LOG(("FGA_ICRLOCAL2 - not implemented\n")); m_fga002[FGA_ICRLOCAL2] = data; break;
	case FGA_ICRLOCAL3      : LOG(("FGA_ICRLOCAL3 - not implemented\n")); m_fga002[FGA_ICRLOCAL3] = data; break;
	case FGA_ICRLOCAL4      : LOG(("FGA_ICRLOCAL4 - not implemented\n")); m_fga002[FGA_ICRLOCAL4] = data; break;
	case FGA_ICRLOCAL5      : LOG(("FGA_ICRLOCAL5 - not implemented\n")); m_fga002[FGA_ICRLOCAL5] = data; break;
	case FGA_ICRLOCAL6      : LOG(("FGA_ICRLOCAL6 - not implemented\n")); m_fga002[FGA_ICRLOCAL6] = data; break;
	case FGA_ICRLOCAL7      : LOG(("FGA_ICRLOCAL7 - not implemented\n")); m_fga002[FGA_ICRLOCAL7] = data; break;
	case FGA_ENAMCODE       : LOG(("FGA_ENAMCODE - not implemented\n")); m_fga002[FGA_ENAMCODE] = data; break;
	case FGA_CTL10          : LOG(("FGA_CTL10 - not implemented\n")); m_fga002[FGA_CTL10] = data; break;
	case FGA_CTL11          : LOG(("FGA_CTL11 - not implemented\n")); m_fga002[FGA_CTL11] = data; break;
	case FGA_MAINUM         : LOG(("FGA_MAINUM - not implemented\n")); m_fga002[FGA_MAINUM] = data; break;
	case FGA_MAINUU         : LOG(("FGA_MAINUU - not implemented\n")); m_fga002[FGA_MAINUU] = data; break;
	case FGA_BOTTOMPAGEU    : LOG(("FGA_BOTTOMPAGEU - not implemented\n")); m_fga002[FGA_BOTTOMPAGEU] = data; break;
	case FGA_BOTTOMPAGEL    : LOG(("FGA_BOTTOMPAGEL - not implemented\n")); m_fga002[FGA_BOTTOMPAGEL] = data; break;
	case FGA_TOPPAGEU       : LOG(("FGA_TOPPAGEU - not implemented\n")); m_fga002[FGA_TOPPAGEU] = data; break;
	case FGA_TOPPAGEL       : LOG(("FGA_TOPPAGEL - not implemented\n")); m_fga002[FGA_TOPPAGEL] = data; break;
	case FGA_MYVMEPAGE      : LOG(("FGA_MYVMEPAGE - not implemented\n")); m_fga002[FGA_MYVMEPAGE] = data; break;
	case FGA_TIM0PRELOAD    : do_fga002reg_tim0preload_w(data); break;
	case FGA_TIM0CTL        : do_fga002reg_tim0ctl_w(data); break;
	case FGA_DMASRCATT      : LOG(("FGA_DMASRCATT - not implemented\n")); m_fga002[FGA_DMASRCATT] = data; break;
	case FGA_DMADSTATT      : LOG(("FGA_DMADSTATT - not implemented\n")); m_fga002[FGA_DMADSTATT] = data; break;
	case FGA_DMA_GENERAL    : LOG(("FGA_DMA_GENERAL - not implemented\n")); m_fga002[FGA_DMA_GENERAL] = data; break;
	case FGA_CTL12          : LOG(("FGA_CTL12 - not implemented\n")); m_fga002[FGA_CTL12] = data; break;
	case FGA_LIOTIMING      : LOG(("FGA_LIOTIMING - not implemented\n")); m_fga002[FGA_LIOTIMING] = data; break;
	case FGA_LOCALIACK      : LOG(("FGA_LOCALIACK - not implemented\n")); m_fga002[FGA_LOCALIACK] = data; break;
	case FGA_FMBCTL         : LOG(("FGA_FMBCTL - not implemented\n")); m_fga002[FGA_FMBCTL] = data; break;
	case FGA_FMBAREA        : LOG(("FGA_FMBAREA - not implemented\n")); m_fga002[FGA_FMBAREA] = data; break;
	case FGA_AUXSRCSTART    : LOG(("FGA_AUXSRCSTART - not implemented\n")); m_fga002[FGA_AUXSRCSTART] = data; break;
	case FGA_AUXDSTSTART    : LOG(("FGA_AUXDSTSTART - not implemented\n")); m_fga002[FGA_AUXDSTSTART] = data; break;
	case FGA_AUXSRCTERM     : LOG(("FGA_AUXSRCTERM - not implemented\n")); m_fga002[FGA_AUXSRCTERM] = data; break;
	case FGA_AUXDSTTERM     : LOG(("FGA_AUXDSTTERM - not implemented\n")); m_fga002[FGA_AUXDSTTERM] = data; break;
	case FGA_CTL13          : LOG(("FGA_CTL13 - not implemented\n")); m_fga002[FGA_CTL13] = data; break;
	case FGA_CTL14          : LOG(("FGA_CTL14 - not implemented\n")); m_fga002[FGA_CTL14] = data; break;
	case FGA_CTL15          : LOG(("FGA_CTL15 - not implemented\n")); m_fga002[FGA_CTL15] = data; break;
	case FGA_CTL16          : LOG(("FGA_CTL16 - not implemented\n")); m_fga002[FGA_CTL16] = data; break;
	case FGA_ISTIM0			: do_fga002reg_istim0_w(data); break;
	case FGA_ISDMANORM      : LOG(("FGA_ISDMANORM - not implemented\n")); m_fga002[FGA_ISDMANORM] = data; break;
	case FGA_ISDMAERR       : LOG(("FGA_ISDMAERR - not implemented\n")); m_fga002[FGA_ISDMAERR] = data; break;
	case FGA_ISFMB0REF      : LOG(("FGA_ISFMB0REF - not implemented\n")); m_fga002[FGA_ISFMB0REF] = data; break;
	case FGA_ISFMB1REF      : LOG(("FGA_ISFMB1REF - not implemented\n")); m_fga002[FGA_ISFMB1REF] = data; break;
	case FGA_ISPARITY       : LOG(("FGA_ISPARITY - not implemented\n")); m_fga002[FGA_ISPARITY] = data; break;
	case FGA_DMARUNCTL      : LOG(("FGA_DMARUNCTL - not implemented\n")); m_fga002[FGA_DMARUNCTL] = data; break;
	case FGA_ISABORT        : LOG(("FGA_ISABORT - not implemented\n")); m_fga002[FGA_ISABORT] = data; break;
	case FGA_ISFMB0MES      : LOG(("FGA_ISFMB0MES - not implemented\n")); m_fga002[FGA_ISFMB0MES] = data; break;
	case FGA_ISFMB1MES      : LOG(("FGA_ISFMB1MES - not implemented\n")); m_fga002[FGA_ISFMB1MES] = data; break;
	case FGA_ABORTPIN       : LOG(("FGA_ABORTPIN - not implemented\n")); m_fga002[FGA_ABORTPIN] = data; break;
	case FGA_TIM0COUNT		: do_fga002reg_tim0count_w(data); break;
	default:
		LOG(("Unsupported register %04x\n", offset));
	}
}

READ8_MEMBER (fga002_device::read){
	UINT8 ret = 0;

	LOG(("%s[%04x]      ", FUNCNAME, offset));
	switch(offset)
	{
	case FGA_SPECIALENA     : ret = m_fga002[FGA_SPECIALENA]; LOG(("FGA_SPECIALENA returns %02x - not implemented\n", ret)); break;
	case FGA_RSVMECALL      : ret = m_fga002[FGA_RSVMECALL]; LOG(("FGA_RSVMECALL returns %02x - not implemented\n", ret)); break;
	case FGA_RSKEYRES       : ret = m_fga002[FGA_RSKEYRES]; LOG(("FGA_RSKEYRES returns %02x - not implemented\n", ret)); break;
	case FGA_RSCPUCALL      : ret = m_fga002[FGA_RSCPUCALL]; LOG(("FGA_RSCPUCALL returns %02x - not implemented\n", ret)); break;
	case FGA_RSLOCSW        : ret = m_fga002[FGA_RSLOCSW]; LOG(("FGA_RSLOCSW returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX0       : ret = m_fga002[FGA_ICRMBOX0]; LOG(("FGA_ICRMBOX0 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX1       : ret = m_fga002[FGA_ICRMBOX1]; LOG(("FGA_ICRMBOX1 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX2       : ret = m_fga002[FGA_ICRMBOX2]; LOG(("FGA_ICRMBOX2 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX3       : ret = m_fga002[FGA_ICRMBOX3]; LOG(("FGA_ICRMBOX3 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX4       : ret = m_fga002[FGA_ICRMBOX4]; LOG(("FGA_ICRMBOX4 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX5       : ret = m_fga002[FGA_ICRMBOX5]; LOG(("FGA_ICRMBOX5 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX6       : ret = m_fga002[FGA_ICRMBOX6]; LOG(("FGA_ICRMBOX6 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX7       : ret = m_fga002[FGA_ICRMBOX7]; LOG(("FGA_ICRMBOX7 returns %02x - not implemented\n", ret)); break;
	case FGA_VMEPAGE        : ret = m_fga002[FGA_VMEPAGE]; LOG(("FGA_VMEPAGE  returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME1        : ret = m_fga002[FGA_ICRVME1]; LOG(("FGA_ICRVME1 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME2        : ret = m_fga002[FGA_ICRVME2]; LOG(("FGA_ICRVME2 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME3        : ret = m_fga002[FGA_ICRVME3]; LOG(("FGA_ICRVME3 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME4        : ret = m_fga002[FGA_ICRVME4]; LOG(("FGA_ICRVME4 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME5        : ret = m_fga002[FGA_ICRVME5]; LOG(("FGA_ICRVME5 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME6        : ret = m_fga002[FGA_ICRVME6]; LOG(("FGA_ICRVME6 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME7        : ret = m_fga002[FGA_ICRVME7]; LOG(("FGA_ICRVME7 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRTIM0        : ret = do_fga002reg_icrtim0_r(); break;
	case FGA_ICRDMANORM     : ret = m_fga002[FGA_ICRDMANORM]; LOG(("FGA_ICRDMANORM returns %02x - not implemented\n", ret)); break;
	case FGA_ICRDMAERR      : ret = m_fga002[FGA_ICRDMAERR]; LOG(("FGA_ICRDMAERR returns %02x - not implemented\n", ret)); break;
	case FGA_CTL1           : ret = m_fga002[FGA_CTL1]; LOG(("FGA_CTL1 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL2           : ret = m_fga002[FGA_CTL2]; LOG(("FGA_CTL2 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRFMB0REF     : ret = m_fga002[FGA_ICRFMB0REF]; LOG(("FGA_ICRFMB0REF returns %02x - not implemented\n", ret)); break;
	case FGA_ICRFMB1REF     : ret = m_fga002[FGA_ICRFMB1REF]; LOG(("FGA_ICRFMB1REF returns %02x - not implemented\n", ret)); break;
	case FGA_ICRFMB0MES     : ret = m_fga002[FGA_ICRFMB0MES]; LOG(("FGA_ICRFMB0MES returns %02x - not implemented\n", ret)); break;
	case FGA_ICRFMB1MES     : ret = m_fga002[FGA_ICRFMB1MES]; LOG(("FGA_ICRFMB1MES returns %02x - not implemented\n", ret)); break;
	case FGA_CTL3           : ret = m_fga002[FGA_CTL3]; LOG(("FGA_CTL3 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL4           : ret = m_fga002[FGA_CTL4]; LOG(("FGA_CTL4 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRPARITY      : ret = m_fga002[FGA_ICRPARITY]; LOG(("FGA_ICRPARITY returns %02x - not implemented\n", ret)); break;
	case FGA_AUXPINCTL      : ret = m_fga002[FGA_AUXPINCTL]; LOG(("FGA_AUXPINCTL returns %02x - not implemented\n", ret)); break;
	case FGA_CTL5           : ret = m_fga002[FGA_CTL5]; LOG(("FGA_CTL5 returns %02x - not implemented\n", ret)); break;
	case FGA_AUXFIFWEX      : ret = m_fga002[FGA_AUXFIFWEX]; LOG(("FGA_AUXFIFWEX returns %02x - not implemented\n", ret)); break;
	case FGA_AUXFIFREX      : ret = m_fga002[FGA_AUXFIFREX]; LOG(("FGA_AUXFIFREX returns %02x - not implemented\n", ret)); break;
	case FGA_CTL6           : ret = m_fga002[FGA_CTL6]; LOG(("FGA_CTL6 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL7           : ret = m_fga002[FGA_CTL7]; LOG(("FGA_CTL7 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL8           : ret = m_fga002[FGA_CTL8]; LOG(("FGA_CTL8 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL9           : ret = m_fga002[FGA_CTL9]; LOG(("FGA_CTL9 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRABORT       : ret = m_fga002[FGA_ICRABORT]; LOG(("FGA_ICRABORT returns %02x - not implemented\n", ret)); break;
	case FGA_ICRACFAIL      : ret = m_fga002[FGA_ICRACFAIL]; LOG(("FGA_ICRACFAIL returns %02x - not implemented\n", ret)); break;
	case FGA_ICRSYSFAIL     : ret = m_fga002[FGA_ICRSYSFAIL]; LOG(("FGA_ICRSYSFAIL returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL0      : ret = m_fga002[FGA_ICRLOCAL0]; LOG(("FGA_ICRLOCAL0 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL1      : ret = m_fga002[FGA_ICRLOCAL1]; LOG(("FGA_ICRLOCAL1 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL2      : ret = m_fga002[FGA_ICRLOCAL2]; LOG(("FGA_ICRLOCAL2 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL3      : ret = m_fga002[FGA_ICRLOCAL3]; LOG(("FGA_ICRLOCAL3 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL4      : ret = m_fga002[FGA_ICRLOCAL4]; LOG(("FGA_ICRLOCAL4 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL5      : ret = m_fga002[FGA_ICRLOCAL5]; LOG(("FGA_ICRLOCAL5 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL6      : ret = m_fga002[FGA_ICRLOCAL6]; LOG(("FGA_ICRLOCAL6 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL7      : ret = m_fga002[FGA_ICRLOCAL7]; LOG(("FGA_ICRLOCAL7 returns %02x - not implemented\n", ret)); break;
	case FGA_ENAMCODE       : ret = m_fga002[FGA_ENAMCODE]; LOG(("FGA_ENAMCODE returns %02x - not implemented\n", ret)); break;
	case FGA_CTL10          : ret = m_fga002[FGA_CTL10]; LOG(("FGA_CTL10 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL11          : ret = m_fga002[FGA_CTL11]; LOG(("FGA_CTL11 returns %02x - not implemented\n", ret)); break;
	case FGA_MAINUM         : ret = m_fga002[FGA_MAINUM]; LOG(("FGA_MAINUM returns %02x - not implemented\n", ret)); break;
	case FGA_MAINUU         : ret = m_fga002[FGA_MAINUU]; LOG(("FGA_MAINUU returns %02x - not implemented\n", ret)); break;
	case FGA_BOTTOMPAGEU    : ret = m_fga002[FGA_BOTTOMPAGEU]; LOG(("FGA_BOTTOMPAGEU returns %02x - not implemented\n", ret)); break;
	case FGA_BOTTOMPAGEL    : ret = m_fga002[FGA_BOTTOMPAGEL]; LOG(("FGA_BOTTOMPAGEL returns %02x - not implemented\n", ret)); break;
	case FGA_TOPPAGEU       : ret = m_fga002[FGA_TOPPAGEU]; LOG(("FGA_TOPPAGEU returns %02x - not implemented\n", ret)); break;
	case FGA_TOPPAGEL       : ret = m_fga002[FGA_TOPPAGEL]; LOG(("FGA_TOPPAGEL returns %02x - not implemented\n", ret)); break;
	case FGA_MYVMEPAGE      : ret = m_fga002[FGA_MYVMEPAGE]; LOG(("FGA_MYVMEPAGE returns %02x - not implemented\n", ret)); break;
	case FGA_TIM0PRELOAD    : ret = do_fga002reg_tim0preload_r(); break;
	case FGA_TIM0CTL        : ret = do_fga002reg_tim0ctl_r(); break;
	case FGA_DMASRCATT      : ret = m_fga002[FGA_DMASRCATT]; LOG(("FGA_DMASRCATT returns %02x - not implemented\n", ret)); break;
	case FGA_DMADSTATT      : ret = m_fga002[FGA_DMADSTATT]; LOG(("FGA_DMADSTATT returns %02x - not implemented\n", ret)); break;
	case FGA_DMA_GENERAL    : ret = m_fga002[FGA_DMA_GENERAL]; LOG(("FGA_DMA_GENERAL returns %02x - not implemented\n", ret)); break;
	case FGA_CTL12          : ret = m_fga002[FGA_CTL12]; LOG(("FGA_CTL12 returns %02x - not implemented\n", ret)); break;
	case FGA_LIOTIMING      : ret = m_fga002[FGA_LIOTIMING]; LOG(("FGA_LIOTIMING returns %02x - not implemented\n", ret)); break;
	case FGA_LOCALIACK      : ret = m_fga002[FGA_LOCALIACK]; LOG(("FGA_LOCALIACK returns %02x - not implemented\n", ret)); break;
	case FGA_FMBCTL         : ret = m_fga002[FGA_FMBCTL]; LOG(("FGA_FMBCTL returns %02x - not implemented\n", ret)); break;
	case FGA_FMBAREA        : ret = m_fga002[FGA_FMBAREA]; LOG(("FGA_FMBAREA returns %02x - not implemented\n", ret)); break;
	case FGA_AUXSRCSTART    : ret = m_fga002[FGA_AUXSRCSTART]; LOG(("FGA_AUXSRCSTART returns %02x - not implemented\n", ret)); break;
	case FGA_AUXDSTSTART    : ret = m_fga002[FGA_AUXDSTSTART]; LOG(("FGA_AUXDSTSTART returns %02x - not implemented\n", ret)); break;
	case FGA_AUXSRCTERM     : ret = m_fga002[FGA_AUXSRCTERM]; LOG(("FGA_AUXSRCTERM returns %02x - not implemented\n", ret)); break;
	case FGA_AUXDSTTERM     : ret = m_fga002[FGA_AUXDSTTERM]; LOG(("FGA_AUXDSTTERM returns %02x - not implemented\n", ret)); break;
	case FGA_CTL13          : ret = m_fga002[FGA_CTL13]; LOG(("FGA_CTL13 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL14          : ret = m_fga002[FGA_CTL14]; LOG(("FGA_CTL14 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL15          : ret = m_fga002[FGA_CTL15]; LOG(("FGA_CTL15 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL16          : ret = m_fga002[FGA_CTL16]; LOG(("FGA_CTL16 returns %02x - not implemented\n", ret)); break;
	case FGA_ISTIM0			: ret = do_fga002reg_istim0_r(); break;
	case FGA_ISDMANORM      : ret = m_fga002[FGA_ISDMANORM]; LOG(("FGA_ISDMANORM returns %02x - not implemented\n", ret)); break;
	case FGA_ISDMAERR       : ret = m_fga002[FGA_ISDMAERR]; LOG(("FGA_ISDMAERR returns %02x - not implemented\n", ret)); break;
	case FGA_ISFMB0REF      : ret = m_fga002[FGA_ISFMB0REF]; LOG(("FGA_ISFMB0REF returns %02x - not implemented\n", ret)); break;
	case FGA_ISFMB1REF      : ret = m_fga002[FGA_ISFMB1REF]; LOG(("FGA_ISFMB1REF returns %02x - not implemented\n", ret)); break;
	case FGA_ISPARITY       : ret = m_fga002[FGA_ISPARITY]; LOG(("FGA_ISPARITY returns %02x - not implemented\n", ret)); break;
	case FGA_DMARUNCTL      : ret = m_fga002[FGA_DMARUNCTL]; LOG(("FGA_DMARUNCTL returns %02x - not implemented\n", ret)); break;
	case FGA_ISABORT        : ret = m_fga002[FGA_ISABORT]; LOG(("FGA_ISABORT returns %02x - not implemented\n", ret)); break;
	case FGA_ISFMB0MES      : ret = m_fga002[FGA_ISFMB0MES]; LOG(("FGA_ISFMB0MES returns %02x - not implemented\n", ret)); break;
	case FGA_ISFMB1MES      : ret = m_fga002[FGA_ISFMB1MES]; LOG(("FGA_ISFMB1MES returns %02x - not implemented\n", ret)); break;
	case FGA_ABORTPIN       : ret = m_fga002[FGA_ABORTPIN]; LOG(("FGA_ABORTPIN returns %02x - not implemented\n", ret)); break;
	case FGA_TIM0COUNT		: ret = do_fga002reg_tim0count_r(); break;
	default:
		LOG(("Unsupported register %04x\n", offset));
	}
	return ret;
}
