// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************
*
*   Force Computer FGA-002 Force Gate Array
*
*   Documetation: http://bitsavers.informatik.uni-stuttgart.de/pdf/forceComputers/201559_FGA-002_Nov96.pdf
*
*   The FGA-002 gate array is a high speed CMOS device manufactured in 1.2 micron technology and containing 24,000 gates in a 281 pin PGA
*   package. It provides interfaces to the 68020/30 microprocessor as well as a VMEbus compatible interface.
*   The auxilary interface of the gate array is a high speed data channel used by the internal 32 bit DMA controller. The interface
*   allows data transfer rates of up to 6 MByte/second. The timing of the local I/O interface is programmable and provides
*   easy interfacing of local I/O devices. All control, address and data lines of the CPU and the VMEbus are either directly connected or
*   connected via buffers to the gate array allowing easy implementation and usage. The gate array registers are programmed by the local CPU.
*
*   FEATURES:
*   - Programmable decoding for CPU and VME access to the local main memory
*   - Interrupt management for internal and external interrupt sources
*   - 32 bit multi-port DMA Controller
*   - FORCE Message Broadcast slave interface with 2 message channels
*   - 8 interrupt capable MAILBOXES
*   - 8 bit TIMER with 16 selectable internal source clocks
*
*
*   CAUTION (from the documentation - no unducumented registers are currently emulated )
*   The FGA-002 gate array contains registers, which are used to configure the gate array for special external hardware
*   requirements. These registers are reserved and will be setup by the boot software according to the hardware environment in which the gate array is
*   implemented. These registers must not be changed by the user. Some of these hardware configuration registers also contain user selectable bits.
*   Programming the contents of these registers has to be done carefully without changing the bits initialized by the boot software.
*   Registers not described must not be programmed. Unqualified changes of register bits may have unpredictable consequences for the gate array and
*   external hardware. It is expressly forbidden to change register bits, except those defined for the user.
*
*/
#include "emu.h"
#include "fga002.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)
#define LOG_READ    (1U <<  2)
#define LOG_INT     (1U <<  3)
#define LOG_VEC     (1U <<  4)
#define LOG_LVL     (1U <<  5)
#define LOG_IACK    (1U <<  6)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,     __VA_ARGS__)
#define LOGVEC(...)   LOGMASKED(LOG_VEC,     __VA_ARGS__)
#define LOGLVL(...)   LOGMASKED(LOG_LVL,     __VA_ARGS__)
#define LOGIACK(...)  LOGMASKED(LOG_IACK,    __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
// device type definition
DEFINE_DEVICE_TYPE(FGA002, fga002_device, "fga002", "Force FGA-002")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  fga002_device - constructor
//-------------------------------------------------

fga002_device::fga002_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_out_int_cb(*this)
	, m_liack4_cb(*this)
	, m_liack5_cb(*this)
	, m_liack6_cb(*this)
	, m_liack7_cb(*this)
	, m_irq_level(uint8_t(0))
{
	for (auto & elem : m_int_state)
		elem = 0;
}

fga002_device::fga002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fga002_device(mconfig, FGA002, tag, owner, clock)
{
}

void fga002_device::device_start()
{
	LOG("%s\n", FUNCNAME);

	// resolve callbacks
	m_out_int_cb.resolve_safe();
	m_liack4_cb.resolve_safe(0);
	m_liack5_cb.resolve_safe(0);
	m_liack6_cb.resolve_safe(0);
	m_liack7_cb.resolve_safe(0);

	// Timers
	fga_timer = timer_alloc(TIMER_ID_FGA);

	save_pointer (NAME (m_fga002), sizeof(m_fga002));

	// Interrupts
	save_item(NAME(m_int_state));
}

void fga002_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
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
	m_fga002[FGA_ISLOCAL0]      = 0x80;
	m_fga002[FGA_ISLOCAL1]      = 0x80;
	m_fga002[FGA_ISLOCAL2]      = 0x80;
	m_fga002[FGA_ISLOCAL3]      = 0x80;
	m_fga002[FGA_ISLOCAL4]      = 0x80;
	m_fga002[FGA_ISLOCAL5]      = 0x80;
	m_fga002[FGA_ISLOCAL6]      = 0x80;
	m_fga002[FGA_ISLOCAL7]      = 0x80;
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
void fga002_device::device_timer (emu_timer &timer, device_timer_id id, int param)
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
		LOG("Unhandled Timer ID %d\n", id);
		break;
	}
}

/* The FGA002 Interrupts

    The FGA-002 Gate Array provides high end support for interrupt functionality. It manages interrupt sources
    within the gate array as well as external sources connected to the gate array. The FGA-002 Gate Array is an
    efficient interface for various interrupt sources to the local CPU, and supports up to 18 external interrupters.

    Interrupt inputs provided for external interrupt sources (exclusive of the VMEbus interrupt inputs) offer
    maximum flexibility as they may be configured to be level/edge sensitive or low/high active. The control of
    these features is performed by two bits contained in the extended interrupt control registers. The Interrupt
    Auto Clear bit in the extended interrupt control register determines, whether the interrupt of an edge sensitive
    input is cleared automatically during the interrupt acknowledge cycle or has to be cleared by the interrupt
    service routine. Each interrupt source is bound to an individual interrupt channel which has its own assigned
    vector number. The interrupt channels are configured by the Interrupt Control Registers where a 3 bit code for
    the level and a bit for enable/disable control are stored. Each interrupt channel may be programmed to interrupt
    the processor at any level.

    The vector table of the gate array is a group of 64 vectors. The two most significant bits of the 8 bit vector
    are programmable via register bits. The rest of the bits are assigned by gate array hardware. Not all of the
    64 vectors are used in the present gate array, and those not used are reserved for future extensions.

    The following groups of interrupt sources are supported:
    1. Internal Interrupt Sources
    - DMA CONTROLLER
    - TIMER
    - FORCE MESSAGE BROADCAST -FMB-
    - PARITY ERROR
    - 8 MAILBOXES
    2. External Interrupt Sources
    Onboard interrupts:
    - LOCAL 0-7 inputs
    - ABORT Key input
    - ACFAIL input
    - SYSFAIL input
    VMEbus interrupts:
    - 7 VMEbus interrupt inputs
*/

// TODO: Add more intrrupts sources in priority order, 18 in total.
const fga002_device::fga_irq_t fga002_device::s_irq_sources[] = {
		{INT_LOCAL0, FGA_ISLOCAL0, FGA_ICRLOCAL0 },
		{INT_LOCAL1, FGA_ISLOCAL1, FGA_ICRLOCAL1 },
		{INT_LOCAL2, FGA_ISLOCAL2, FGA_ICRLOCAL2 },
		{INT_LOCAL3, FGA_ISLOCAL3, FGA_ICRLOCAL3 },
		{INT_LOCAL4, FGA_ISLOCAL4, FGA_ICRLOCAL4 },
		{INT_LOCAL5, FGA_ISLOCAL5, FGA_ICRLOCAL5 },
		{INT_LOCAL6, FGA_ISLOCAL6, FGA_ICRLOCAL6 },
		{INT_LOCAL7, FGA_ISLOCAL7, FGA_ICRLOCAL7 }
};


void fga002_device::trigger_interrupt(uint8_t data)
{
	uint8_t icr = 0;

	LOGINT("%s(%02x)\n", FUNCNAME, data);

	/* The Interrupt Control Register (ICR*) bit, must be set for the correspondning channel */
	// TODO: Support programmable assert level for interrupt source
	switch(data)
	{
	case INT_LOCAL0: icr = m_fga002[FGA_ICRLOCAL0]; m_fga002[FGA_ISLOCAL0] = 0x00; break;
	case INT_LOCAL1: icr = m_fga002[FGA_ICRLOCAL1]; m_fga002[FGA_ISLOCAL1] = 0x00; break;
	case INT_LOCAL2: icr = m_fga002[FGA_ICRLOCAL2]; m_fga002[FGA_ISLOCAL2] = 0x00; break;
	case INT_LOCAL3: icr = m_fga002[FGA_ICRLOCAL3]; m_fga002[FGA_ISLOCAL3] = 0x00; break;
	case INT_LOCAL4: icr = m_fga002[FGA_ICRLOCAL4]; m_fga002[FGA_ISLOCAL4] = 0x00; break;
	case INT_LOCAL5: icr = m_fga002[FGA_ICRLOCAL5]; m_fga002[FGA_ISLOCAL5] = 0x00; break;
	case INT_LOCAL6: icr = m_fga002[FGA_ICRLOCAL6]; m_fga002[FGA_ISLOCAL6] = 0x00; break;
	case INT_LOCAL7: icr = m_fga002[FGA_ICRLOCAL7]; m_fga002[FGA_ISLOCAL7] = 0x00; break;
	default: LOGINT(" - interrupt source %d - not supported", data); return;
	}
	if ((icr & REG_ICR_ENABLE) == 0 || (icr & REG_ICR_LVL_MSK) == 0)
	{
		LOGINT(" - The Interrupt Control Register bit for channel %02x is not set or level is 0, blocking attempt to interrupt\n", data);
		return;
	}
	m_irq_level = icr & REG_ICR_LVL_MSK;
	LOGINT(" - Interrupt Level %d, caused by ICR %02x with vector %02x\n", m_irq_level, icr, data );

	// trigger intrrupt to CPU through board driver callback.
	m_out_int_cb(ASSERT_LINE);
}

u16 fga002_device::iack()
{
	int vec = 0x18; // Spurious interrupt
	int vec_found = 0;
	int level;

	LOGIACK("%s %s()\n", tag(), FUNCNAME);
	for (level = 7; level > 0; level--)
	{
		LOGLVL("\n LEVEL %d\n", level);
		// Find first interrupt on this level to acknowledge
		LOGLVL("Vec Status[val] Control[val]\n");
		for (auto & elem : s_irq_sources)
		{
			LOGLVL(" %02x    %02x[%02x]      %02x[%02x]\n",
					elem.vector,
					elem.status, m_fga002[elem.status],
					elem.control, m_fga002[elem.control]);
			// Right level?
			LOGLVL("Level %02x == ICR %02x is %s\n", level, m_fga002[elem.control] & REG_ICR_LVL_MSK, ((m_fga002[elem.control] & REG_ICR_LVL_MSK) == level) ? "true!" : "false..");
			if ((m_fga002[elem.control] & REG_ICR_LVL_MSK) == level)
			{
				// Pending interrupt?
				LOGLVL("ISx %02x interrupt is %s\n", m_fga002[elem.status], (m_fga002[elem.status] & REG_ISLOCAL_IRQ) ? "cleared though.." : "pending!");
				if ((m_fga002[elem.status] & REG_ISLOCAL_IRQ) == 0)
				{
					if (vec_found == 0)
					{
						vec = elem.vector; // Assume internal vector
						LOGVEC(" - org vec:%02x ", vec);
						switch (vec)
						{
							// Assuming that the attached device is returning -1 (INT_ACK_AUTOVECTOR) if no vector is provided, then we keep internal vector
							// TODO: Add device specific parameter that maps devices no-vector response to INT_ACK_AUTOVECTOR (and preferably INT_ACK_NOVECTOR)
						case INT_LOCAL4:  if (( (m_fga002[FGA_LOCALIACK] & REG_LIACK_LOCAL4_MSK) >> 0) != REG_LIACK_INT_IACK ) vec = m_liack4_cb(); break;
						case INT_LOCAL5:  if (( (m_fga002[FGA_LOCALIACK] & REG_LIACK_LOCAL5_MSK) >> 2) != REG_LIACK_INT_IACK ) vec = m_liack5_cb(); break;
						case INT_LOCAL6:  if (( (m_fga002[FGA_LOCALIACK] & REG_LIACK_LOCAL6_MSK) >> 4) != REG_LIACK_INT_IACK ) vec = m_liack6_cb(); break;
						case INT_LOCAL7:  if (( (m_fga002[FGA_LOCALIACK] & REG_LIACK_LOCAL7_MSK) >> 6) != REG_LIACK_INT_IACK ) vec = m_liack7_cb(); break;
							// All other devices uses the vector provided by the FGA.
						default: break; /* Since we need the vector for the switch statement the default job is already done */
						}
						LOGVEC("dev:%02x ", vec);
						if (vec == 0x18) vec = INT_EMPTY;
						LOGVEC("avec:%02x ", vec);

						// Add vector page bits and return vector
						vec = (vec & 0x3f) | ((m_fga002[FGA_CTL3] & REG_CTL3_VECTORBITS7_6) << 4);
						LOGVEC("pvec:%02x\n", vec);

						LOGVEC(" - Interrupt Acknowledge Vector %02x\n", vec);
						/* TODO:
						   - Support auto clear of interrupt source and level triggered
						*/
						vec_found = 1;
					}
					else{
						m_irq_level = level;
						LOGIACK(" - Interrupt Acknowledge Vector %02x, next interrupt has level %02x\n", vec, m_irq_level);
						m_out_int_cb(CLEAR_LINE);
						return vec;
					}
				}
			}
		}
	}
	LOGIACK(" - Interrupt Acknowledge Vector %02x, next interrupt is off %02x\n", vec, m_irq_level);
	m_out_int_cb(CLEAR_LINE);
	return vec;
}

int fga002_device::get_irq_level()
{
	LOGINT("%s %s() - %02x\n", tag(), FUNCNAME, m_irq_level);
	return m_irq_level;
}

/*  The FGA002 Timer

    FEATURES
    - 8 bit Synchronous Counter
    - 16 selectable clocks with frequencies from 1MHz to 0.5 Hz
    - Autopreload and Zerostop operating modes
    - Watchdog Timer operation
    - SYSFAIL and/or interrupt generation
    - Vectored interrupt
    - Interrupt levels selectable by software
*/

/*  Timer Preload Register TIM0PRELOAD
    The Timer Preload Register TIM0PRELOAD contains the preset value which can be loaded into the counter circuit. The default
    value of this register after reset is $00. The TIM0PRELOAD register can be read at any time but must not be altered if the
    timer is running.
    [7:0] The Timer Preload register contains the 8 bit value that is loaded into the counter if the
    Autopreload option in the TIM0CTL register is selected and the counter reaches the value zero.
    Also, if a write access to the TIM0COUNT register is performed, the counter is loaded with the value
    stored in the Timer Preload Register.
*/
void fga002_device::do_fga002reg_tim0preload_w(uint8_t data)
{
	LOG("%s(%02x)\n", FUNCNAME, data);
	m_fga002[FGA_TIM0PRELOAD] = data;
}

uint8_t fga002_device::do_fga002reg_tim0preload_r()
{
	LOG("%s() %02x\n", FUNCNAME, m_fga002[FGA_TIM0PRELOAD]);
	return m_fga002[FGA_TIM0PRELOAD];
}

/*  Timer Control Register TIM0CTL
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

void fga002_device::do_fga002reg_tim0ctl_w(uint8_t data)
{
	LOG("%s(%02x)\n", FUNCNAME, data);
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

uint8_t fga002_device::do_fga002reg_tim0ctl_r()
{
	LOG("%s() %02x\n", FUNCNAME, m_fga002[FGA_TIM0CTL]);
	return m_fga002[FGA_TIM0CTL];
}

/*  Timer Count Register TIM0COUNT
    The Timer Count Register TIM0COUNT contains the current value of the timer/counter. A write access to this register will
    load the counter with the value stored in the Timer Preload Register. The written data will be ignored.
    It is permitted to perform read/write accesses to the Timer Count Register when the timer is running.
    The Timer Count Register is initialized to the value $FF after reset.
    [7:0] Timer Count Value
*/
void fga002_device::do_fga002reg_tim0count_w(uint8_t data)
{
	LOG("%s(%02x)\n", FUNCNAME, data);
	m_tim0count = m_fga002[FGA_TIM0PRELOAD];
}

uint8_t fga002_device::do_fga002reg_tim0count_r()
{
	LOG("%s() %02x\n", FUNCNAME, m_tim0count);
	return m_tim0count;
}

/*  Timer Interrupt Control Register ICRTIM0
    Timer Interrupt Control is performed by the Timer Interrupt Control Register ICRTIM0 which enables/disables the interrupt
    and selects the interrupt level.
    [3] IRQ enable, 1 = timer interrupt channel enabled, 0 = disabled
    [2:0] IRQ level 000 = interrupt disabled 001-111 = Level 1 to 7  interrupt
*/
void fga002_device::do_fga002reg_icrtim0_w(uint8_t data)
{
	LOGINT("%s(%02x)\n", FUNCNAME, data);
	m_fga002[FGA_ICRTIM0] = data;
}

uint8_t fga002_device::do_fga002reg_icrtim0_r()
{
	LOGINT("%s() %02x\n", FUNCNAME, m_fga002[FGA_ICRTIM0]);
	return m_fga002[FGA_ICRTIM0];
}

/*  Timer Interrupt Status Register ISTIM0
    ISTIM0 displays a pending timer interrupt. This bit is always readable and indicates 0 if the timer interrupt has been triggered. A write access to the
    ISTIM0 register clears the timer interrupt. The data written to this register will be ignored.
    [7] The IRQ Status register bit displays if a timer interrupt request is pending. 1 = no interrupt is pending. 0 = interrupt is pending
    [6:0] not used
*/
void fga002_device::do_fga002reg_istim0_w(uint8_t data)
{
	LOGINT("%s(%02x)\n", FUNCNAME, data);
	m_fga002[FGA_ISTIM0] &= ~REG_ISTIM0_TIM_INT; // Clear timer interrupt status
}

uint8_t fga002_device::do_fga002reg_istim0_r()
{
	LOGINT("%s() %02x\n", FUNCNAME, m_fga002[FGA_ISTIM0]);
	return m_fga002[FGA_ISTIM0];
}


/*
 * FGA-002 interrupt support
 */
void fga002_device::do_fga002reg_localiack_w(uint8_t data)
{
	m_fga002[FGA_LOCALIACK] = data;

#if VERBOSE
	const char *liack[] = {"internal", "no", "external 1us", "external 500ns"};

	LOGINT("%s(%02x)\n", FUNCNAME, data);
	LOGINT("- LOCAL7: %s vector\n", liack[(data >> 6) & 0x03]);
	LOGINT("- LOCAL6: %s vector\n", liack[(data >> 4) & 0x03]);
	LOGINT("- LOCAL5: %s vector\n", liack[(data >> 2) & 0x03]);
	LOGINT("- LOCAL4: %s vector\n", liack[(data >> 0) & 0x03]);
#endif
}

uint8_t fga002_device::do_fga002reg_localiack_r()
{
	uint8_t ret = m_fga002[FGA_LOCALIACK];
	LOGINT("%s() <- %02x\n", FUNCNAME, ret);
	return ret;
}

uint8_t fga002_device::do_fga002reg_ctl3_r()
{
	uint8_t ret = m_fga002[FGA_CTL3];
	LOGINT("%s() <- %02x\n", FUNCNAME, ret);
	return ret;
}

void fga002_device::do_fga002reg_ctl3_w(uint8_t data)
{
	m_fga002[FGA_CTL3] = data;
	LOGINT("%s(%02x)\n", FUNCNAME, data);
}

// Local Interrupt control register methods
uint8_t fga002_device::do_fga002reg_icrlocal0_r(){    uint8_t ret = m_fga002[FGA_ICRLOCAL0]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_icrlocal1_r(){    uint8_t ret = m_fga002[FGA_ICRLOCAL1]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_icrlocal2_r(){    uint8_t ret = m_fga002[FGA_ICRLOCAL2]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_icrlocal3_r(){    uint8_t ret = m_fga002[FGA_ICRLOCAL3]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_icrlocal4_r(){    uint8_t ret = m_fga002[FGA_ICRLOCAL4]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_icrlocal5_r(){    uint8_t ret = m_fga002[FGA_ICRLOCAL5]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_icrlocal6_r(){    uint8_t ret = m_fga002[FGA_ICRLOCAL6]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_icrlocal7_r(){    uint8_t ret = m_fga002[FGA_ICRLOCAL7]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }

void  fga002_device::do_fga002reg_icrlocal0_w(uint8_t data){ m_fga002[FGA_ICRLOCAL0] = data; LOGINT("%s(%02x)\n", FUNCNAME, data); }
void  fga002_device::do_fga002reg_icrlocal1_w(uint8_t data){ m_fga002[FGA_ICRLOCAL1] = data; LOGINT("%s(%02x)\n", FUNCNAME, data); }
void  fga002_device::do_fga002reg_icrlocal2_w(uint8_t data){ m_fga002[FGA_ICRLOCAL2] = data; LOGINT("%s(%02x)\n", FUNCNAME, data); }
void  fga002_device::do_fga002reg_icrlocal3_w(uint8_t data){ m_fga002[FGA_ICRLOCAL3] = data; LOGINT("%s(%02x)\n", FUNCNAME, data); }
void  fga002_device::do_fga002reg_icrlocal4_w(uint8_t data){ m_fga002[FGA_ICRLOCAL4] = data; LOGINT("%s(%02x)\n", FUNCNAME, data); }
void  fga002_device::do_fga002reg_icrlocal5_w(uint8_t data){ m_fga002[FGA_ICRLOCAL5] = data; LOGINT("%s(%02x)\n", FUNCNAME, data); }
void  fga002_device::do_fga002reg_icrlocal6_w(uint8_t data){ m_fga002[FGA_ICRLOCAL6] = data; LOGINT("%s(%02x)\n", FUNCNAME, data); }
void  fga002_device::do_fga002reg_icrlocal7_w(uint8_t data){ m_fga002[FGA_ICRLOCAL7] = data; LOGINT("%s(%02x)\n", FUNCNAME, data); }

// Local Interrupt Status Register methods
uint8_t fga002_device::do_fga002reg_islocal0_r(){ uint8_t ret = m_fga002[FGA_ISLOCAL0]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_islocal1_r(){ uint8_t ret = m_fga002[FGA_ISLOCAL1]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_islocal2_r(){ uint8_t ret = m_fga002[FGA_ISLOCAL2]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_islocal3_r(){ uint8_t ret = m_fga002[FGA_ISLOCAL3]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_islocal4_r(){ uint8_t ret = m_fga002[FGA_ISLOCAL4]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_islocal5_r(){ uint8_t ret = m_fga002[FGA_ISLOCAL5]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_islocal6_r(){ uint8_t ret = m_fga002[FGA_ISLOCAL6]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }
uint8_t fga002_device::do_fga002reg_islocal7_r(){ uint8_t ret = m_fga002[FGA_ISLOCAL7]; LOGINT("%s() <- %02x\n", FUNCNAME, ret); return ret; }

void fga002_device::islocal_w(int status, int vector, int control, int data)
{
	m_fga002[status] = m_fga002[control] & REG_ICR_EDGE ? 0x80 : 0x00;
	LOGINT("%s(%02x)\n", FUNCNAME, m_fga002[status]);
}

//  TODO: support level triggered interrupts, only edge triggered interrupts are supported atm
void  fga002_device::do_fga002reg_islocal0_w(uint8_t data){ LOGINT("%s\n", FUNCNAME); islocal_w( FGA_ISLOCAL0, INT_LOCAL0, FGA_ICRLOCAL0, data ); }
void  fga002_device::do_fga002reg_islocal1_w(uint8_t data){ LOGINT("%s\n", FUNCNAME); islocal_w( FGA_ISLOCAL1, INT_LOCAL1, FGA_ICRLOCAL1, data ); }
void  fga002_device::do_fga002reg_islocal2_w(uint8_t data){ LOGINT("%s\n", FUNCNAME); islocal_w( FGA_ISLOCAL2, INT_LOCAL2, FGA_ICRLOCAL2, data ); }
void  fga002_device::do_fga002reg_islocal3_w(uint8_t data){ LOGINT("%s\n", FUNCNAME); islocal_w( FGA_ISLOCAL3, INT_LOCAL3, FGA_ICRLOCAL3, data ); }
void  fga002_device::do_fga002reg_islocal4_w(uint8_t data){ LOGINT("%s\n", FUNCNAME); islocal_w( FGA_ISLOCAL4, INT_LOCAL4, FGA_ICRLOCAL4, data ); m_liack4_cb(); } /* terminate device IRQ */
void  fga002_device::do_fga002reg_islocal5_w(uint8_t data){ LOGINT("%s\n", FUNCNAME); islocal_w( FGA_ISLOCAL5, INT_LOCAL5, FGA_ICRLOCAL5, data ); m_liack5_cb(); }
void  fga002_device::do_fga002reg_islocal6_w(uint8_t data){ LOGINT("%s\n", FUNCNAME); islocal_w( FGA_ISLOCAL6, INT_LOCAL6, FGA_ICRLOCAL6, data ); m_liack6_cb(); }
void  fga002_device::do_fga002reg_islocal7_w(uint8_t data){ LOGINT("%s\n", FUNCNAME); islocal_w( FGA_ISLOCAL6, INT_LOCAL7, FGA_ICRLOCAL7, data ); m_liack7_cb(); }

// Local IRQ callbacks
// TODO: support level triggered interrupts, ICR bit 6, only edge triggered interrupts are supported atm
// TODO: support programmable assertion levels ICR bit 5, a call to this function assumes assertion atm
void fga002_device::lirq_w(int status, int vector, int control, int state)
{
	LOGINT(" - %s\n", state == ASSERT_LINE ? "Asserted" : "Cleared");
	if (state == ASSERT_LINE)
	{
		m_fga002[status] &= ~REG_ISLOCAL_IRQ;
		trigger_interrupt(vector);
	}
	else
	{
		m_fga002[status] |= REG_ISLOCAL_IRQ;
	}
}

WRITE_LINE_MEMBER (fga002_device::lirq0_w) { LOGINT("%s\n", FUNCNAME); lirq_w( FGA_ISLOCAL0, INT_LOCAL0, FGA_ICRLOCAL0, state ); }
WRITE_LINE_MEMBER (fga002_device::lirq1_w) { LOGINT("%s\n", FUNCNAME); lirq_w( FGA_ISLOCAL1, INT_LOCAL1, FGA_ICRLOCAL1, state ); }
WRITE_LINE_MEMBER (fga002_device::lirq2_w) { LOGINT("%s\n", FUNCNAME); lirq_w( FGA_ISLOCAL2, INT_LOCAL2, FGA_ICRLOCAL2, state ); }
WRITE_LINE_MEMBER (fga002_device::lirq3_w) { LOGINT("%s\n", FUNCNAME); lirq_w( FGA_ISLOCAL3, INT_LOCAL3, FGA_ICRLOCAL3, state ); }
WRITE_LINE_MEMBER (fga002_device::lirq4_w) { LOGINT("%s\n", FUNCNAME); lirq_w( FGA_ISLOCAL4, INT_LOCAL4, FGA_ICRLOCAL4, state ); }
WRITE_LINE_MEMBER (fga002_device::lirq5_w) { LOGINT("%s\n", FUNCNAME); lirq_w( FGA_ISLOCAL5, INT_LOCAL5, FGA_ICRLOCAL5, state ); }
WRITE_LINE_MEMBER (fga002_device::lirq6_w) { LOGINT("%s\n", FUNCNAME); lirq_w( FGA_ISLOCAL6, INT_LOCAL6, FGA_ICRLOCAL6, state ); }
WRITE_LINE_MEMBER (fga002_device::lirq7_w) { LOGINT("%s\n", FUNCNAME); lirq_w( FGA_ISLOCAL7, INT_LOCAL7, FGA_ICRLOCAL7, state ); }

void fga002_device::write(offs_t offset, uint8_t data){
	LOG("%s[%04x] <- %02x    - ", FUNCNAME, offset, data);
	LOGSETUP(" * %s Reg %04x <- %02x\n", tag(), offset, data);
	switch(offset)
	{
	case FGA_SPECIALENA     : LOG("FGA_SPECIALENA - not implemented\n"); m_fga002[FGA_SPECIALENA] = data; break;
	case FGA_RSVMECALL      : LOG("FGA_RSVMECALL - not implemented\n"); m_fga002[FGA_RSVMECALL] = data; break;
	case FGA_RSKEYRES       : LOG("FGA_RSKEYRES - not implemented\n"); m_fga002[FGA_RSKEYRES] = data; break;
	case FGA_RSCPUCALL      : LOG("FGA_RSCPUCALL - not implemented\n"); m_fga002[FGA_RSCPUCALL] = data; break;
	case FGA_RSLOCSW        : LOG("FGA_RSLOCSW - not implemented\n"); m_fga002[FGA_RSLOCSW] = data; break;
	case FGA_ICRMBOX0       : LOG("FGA_ICRMBOX0 - not implemented\n"); m_fga002[FGA_ICRMBOX0] = data; break;
	case FGA_ICRMBOX1       : LOG("FGA_ICRMBOX1 - not implemented\n"); m_fga002[FGA_ICRMBOX1] = data; break;
	case FGA_ICRMBOX2       : LOG("FGA_ICRMBOX2 - not implemented\n"); m_fga002[FGA_ICRMBOX2] = data; break;
	case FGA_ICRMBOX3       : LOG("FGA_ICRMBOX3 - not implemented\n"); m_fga002[FGA_ICRMBOX3] = data; break;
	case FGA_ICRMBOX4       : LOG("FGA_ICRMBOX4 - not implemented\n"); m_fga002[FGA_ICRMBOX4] = data; break;
	case FGA_ICRMBOX5       : LOG("FGA_ICRMBOX5 - not implemented\n"); m_fga002[FGA_ICRMBOX5] = data; break;
	case FGA_ICRMBOX6       : LOG("FGA_ICRMBOX6 - not implemented\n"); m_fga002[FGA_ICRMBOX6] = data; break;
	case FGA_ICRMBOX7       : LOG("FGA_ICRMBOX7 - not implemented\n"); m_fga002[FGA_ICRMBOX7] = data; break;
	case FGA_VMEPAGE        : LOG("FGA_VMEPAGE  - not implemented\n"); m_fga002[FGA_VMEPAGE ] = data; break;
	case FGA_ICRVME1        : LOG("FGA_ICRVME1 - not implemented\n"); m_fga002[FGA_ICRVME1] = data; break;
	case FGA_ICRVME2        : LOG("FGA_ICRVME2 - not implemented\n"); m_fga002[FGA_ICRVME2] = data; break;
	case FGA_ICRVME3        : LOG("FGA_ICRVME3 - not implemented\n"); m_fga002[FGA_ICRVME3] = data; break;
	case FGA_ICRVME4        : LOG("FGA_ICRVME4 - not implemented\n"); m_fga002[FGA_ICRVME4] = data; break;
	case FGA_ICRVME5        : LOG("FGA_ICRVME5 - not implemented\n"); m_fga002[FGA_ICRVME5] = data; break;
	case FGA_ICRVME6        : LOG("FGA_ICRVME6 - not implemented\n"); m_fga002[FGA_ICRVME6] = data; break;
	case FGA_ICRVME7        : LOG("FGA_ICRVME7 - not implemented\n"); m_fga002[FGA_ICRVME7] = data; break;
	case FGA_ICRTIM0        : do_fga002reg_icrtim0_w(data); break;
	case FGA_ICRDMANORM     : LOG("FGA_ICRDMANORM - not implemented\n"); m_fga002[FGA_ICRDMANORM] = data; break;
	case FGA_ICRDMAERR      : LOG("FGA_ICRDMAERR - not implemented\n"); m_fga002[FGA_ICRDMAERR] = data; break;
	case FGA_CTL1           : LOG("FGA_CTL1 - not implemented\n"); m_fga002[FGA_CTL1] = data; break;
	case FGA_CTL2           : LOG("FGA_CTL2 - not implemented\n"); m_fga002[FGA_CTL2] = data; break;
	case FGA_ICRFMB0REF     : LOG("FGA_ICRFMB0REF - not implemented\n"); m_fga002[FGA_ICRFMB0REF] = data; break;
	case FGA_ICRFMB1REF     : LOG("FGA_ICRFMB1REF - not implemented\n"); m_fga002[FGA_ICRFMB1REF] = data; break;
	case FGA_ICRFMB0MES     : LOG("FGA_ICRFMB0MES - not implemented\n"); m_fga002[FGA_ICRFMB0MES] = data; break;
	case FGA_ICRFMB1MES     : LOG("FGA_ICRFMB1MES - not implemented\n"); m_fga002[FGA_ICRFMB1MES] = data; break;
	case FGA_CTL3           : do_fga002reg_ctl3_w(data); break;
	case FGA_CTL4           : LOG("FGA_CTL4 - not implemented\n"); m_fga002[FGA_CTL4] = data; break;
	case FGA_ICRPARITY      : LOG("FGA_ICRPARITY - not implemented\n"); m_fga002[FGA_ICRPARITY] = data; break;
	case FGA_AUXPINCTL      : LOG("FGA_AUXPINCTL - not implemented\n"); m_fga002[FGA_AUXPINCTL] = data; break;
	case FGA_CTL5           : LOG("FGA_CTL5 - not implemented\n"); m_fga002[FGA_CTL5] = data; break;
	case FGA_AUXFIFWEX      : LOG("FGA_AUXFIFWEX - not implemented\n"); m_fga002[FGA_AUXFIFWEX] = data; break;
	case FGA_AUXFIFREX      : LOG("FGA_AUXFIFREX - not implemented\n"); m_fga002[FGA_AUXFIFREX] = data; break;
	case FGA_CTL6           : LOG("FGA_CTL6 - not implemented\n"); m_fga002[FGA_CTL6] = data; break;
	case FGA_CTL7           : LOG("FGA_CTL7 - not implemented\n"); m_fga002[FGA_CTL7] = data; break;
	case FGA_CTL8           : LOG("FGA_CTL8 - not implemented\n"); m_fga002[FGA_CTL8] = data; break;
	case FGA_CTL9           : LOG("FGA_CTL9 - not implemented\n"); m_fga002[FGA_CTL9] = data; break;
	case FGA_ICRABORT       : LOG("FGA_ICRABORT - not implemented\n"); m_fga002[FGA_ICRABORT] = data; break;
	case FGA_ICRACFAIL      : LOG("FGA_ICRACFAIL - not implemented\n"); m_fga002[FGA_ICRACFAIL] = data; break;
	case FGA_ICRSYSFAIL     : LOG("FGA_ICRSYSFAIL - not implemented\n"); m_fga002[FGA_ICRSYSFAIL] = data; break;
	case FGA_ICRLOCAL0      : do_fga002reg_icrlocal0_w(data); break;
	case FGA_ICRLOCAL1      : do_fga002reg_icrlocal1_w(data); break;
	case FGA_ICRLOCAL2      : do_fga002reg_icrlocal2_w(data); break;
	case FGA_ICRLOCAL3      : do_fga002reg_icrlocal3_w(data); break;
	case FGA_ICRLOCAL4      : do_fga002reg_icrlocal4_w(data); break;
	case FGA_ICRLOCAL5      : do_fga002reg_icrlocal5_w(data); break;
	case FGA_ICRLOCAL6      : do_fga002reg_icrlocal6_w(data); break;
	case FGA_ICRLOCAL7      : do_fga002reg_icrlocal7_w(data); break;
	case FGA_ENAMCODE       : LOG("FGA_ENAMCODE - not implemented\n"); m_fga002[FGA_ENAMCODE] = data; break;
	case FGA_CTL10          : LOG("FGA_CTL10 - not implemented\n"); m_fga002[FGA_CTL10] = data; break;
	case FGA_CTL11          : LOG("FGA_CTL11 - not implemented\n"); m_fga002[FGA_CTL11] = data; break;
	case FGA_MAINUM         : LOG("FGA_MAINUM - not implemented\n"); m_fga002[FGA_MAINUM] = data; break;
	case FGA_MAINUU         : LOG("FGA_MAINUU - not implemented\n"); m_fga002[FGA_MAINUU] = data; break;
	case FGA_BOTTOMPAGEU    : LOG("FGA_BOTTOMPAGEU - not implemented\n"); m_fga002[FGA_BOTTOMPAGEU] = data; break;
	case FGA_BOTTOMPAGEL    : LOG("FGA_BOTTOMPAGEL - not implemented\n"); m_fga002[FGA_BOTTOMPAGEL] = data; break;
	case FGA_TOPPAGEU       : LOG("FGA_TOPPAGEU - not implemented\n"); m_fga002[FGA_TOPPAGEU] = data; break;
	case FGA_TOPPAGEL       : LOG("FGA_TOPPAGEL - not implemented\n"); m_fga002[FGA_TOPPAGEL] = data; break;
	case FGA_MYVMEPAGE      : LOG("FGA_MYVMEPAGE - not implemented\n"); m_fga002[FGA_MYVMEPAGE] = data; break;
	case FGA_TIM0PRELOAD    : do_fga002reg_tim0preload_w(data); break;
	case FGA_TIM0CTL        : do_fga002reg_tim0ctl_w(data); break;
	case FGA_DMASRCATT      : LOG("FGA_DMASRCATT - not implemented\n"); m_fga002[FGA_DMASRCATT] = data; break;
	case FGA_DMADSTATT      : LOG("FGA_DMADSTATT - not implemented\n"); m_fga002[FGA_DMADSTATT] = data; break;
	case FGA_DMA_GENERAL    : LOG("FGA_DMA_GENERAL - not implemented\n"); m_fga002[FGA_DMA_GENERAL] = data; break;
	case FGA_CTL12          : LOG("FGA_CTL12 - not implemented\n"); m_fga002[FGA_CTL12] = data; break;
	case FGA_LIOTIMING      : LOG("FGA_LIOTIMING - not implemented\n"); m_fga002[FGA_LIOTIMING] = data; break;
	case FGA_LOCALIACK      : do_fga002reg_localiack_w(data); break;
	case FGA_FMBCTL         : LOG("FGA_FMBCTL - not implemented\n"); m_fga002[FGA_FMBCTL] = data; break;
	case FGA_FMBAREA        : LOG("FGA_FMBAREA - not implemented\n"); m_fga002[FGA_FMBAREA] = data; break;
	case FGA_AUXSRCSTART    : LOG("FGA_AUXSRCSTART - not implemented\n"); m_fga002[FGA_AUXSRCSTART] = data; break;
	case FGA_AUXDSTSTART    : LOG("FGA_AUXDSTSTART - not implemented\n"); m_fga002[FGA_AUXDSTSTART] = data; break;
	case FGA_AUXSRCTERM     : LOG("FGA_AUXSRCTERM - not implemented\n"); m_fga002[FGA_AUXSRCTERM] = data; break;
	case FGA_AUXDSTTERM     : LOG("FGA_AUXDSTTERM - not implemented\n"); m_fga002[FGA_AUXDSTTERM] = data; break;
	case FGA_CTL13          : LOG("FGA_CTL13 - not implemented\n"); m_fga002[FGA_CTL13] = data; break;
	case FGA_CTL14          : LOG("FGA_CTL14 - not implemented\n"); m_fga002[FGA_CTL14] = data; break;
	case FGA_CTL15          : LOG("FGA_CTL15 - not implemented\n"); m_fga002[FGA_CTL15] = data; break;
	case FGA_CTL16          : LOG("FGA_CTL16 - not implemented\n"); m_fga002[FGA_CTL16] = data; break;
	case FGA_ISTIM0         : do_fga002reg_istim0_w(data); break;
	case FGA_ISLOCAL0       : do_fga002reg_islocal0_w(data); break;
	case FGA_ISLOCAL1       : do_fga002reg_islocal1_w(data); break;
	case FGA_ISLOCAL2       : do_fga002reg_islocal2_w(data); break;
	case FGA_ISLOCAL3       : do_fga002reg_islocal3_w(data); break;
	case FGA_ISLOCAL4       : do_fga002reg_islocal4_w(data); break;
	case FGA_ISLOCAL5       : do_fga002reg_islocal5_w(data); break;
	case FGA_ISLOCAL6       : do_fga002reg_islocal6_w(data); break;
	case FGA_ISLOCAL7       : do_fga002reg_islocal7_w(data); break;
	case FGA_ISDMANORM      : LOG("FGA_ISDMANORM - not implemented\n"); m_fga002[FGA_ISDMANORM] = data; break;
	case FGA_ISDMAERR       : LOG("FGA_ISDMAERR - not implemented\n"); m_fga002[FGA_ISDMAERR] = data; break;
	case FGA_ISFMB0REF      : LOG("FGA_ISFMB0REF - not implemented\n"); m_fga002[FGA_ISFMB0REF] = data; break;
	case FGA_ISFMB1REF      : LOG("FGA_ISFMB1REF - not implemented\n"); m_fga002[FGA_ISFMB1REF] = data; break;
	case FGA_ISPARITY       : LOG("FGA_ISPARITY - not implemented\n"); m_fga002[FGA_ISPARITY] = data; break;
	case FGA_DMARUNCTL      : LOG("FGA_DMARUNCTL - not implemented\n"); m_fga002[FGA_DMARUNCTL] = data; break;
	case FGA_ISABORT        : LOG("FGA_ISABORT - not implemented\n"); m_fga002[FGA_ISABORT] = data; break;
	case FGA_ISFMB0MES      : LOG("FGA_ISFMB0MES - not implemented\n"); m_fga002[FGA_ISFMB0MES] = data; break;
	case FGA_ISFMB1MES      : LOG("FGA_ISFMB1MES - not implemented\n"); m_fga002[FGA_ISFMB1MES] = data; break;
	case FGA_ABORTPIN       : LOG("FGA_ABORTPIN - not implemented\n"); m_fga002[FGA_ABORTPIN] = data; break;
	case FGA_TIM0COUNT      : do_fga002reg_tim0count_w(data); break;
	default:
		LOG("Unsupported register %04x\n", offset);
	}
}

uint8_t fga002_device::read(offs_t offset){
	uint8_t ret = 0;

	LOG("%s[%04x]      ", FUNCNAME, offset);
	switch(offset)
	{
	case FGA_SPECIALENA     : ret = m_fga002[FGA_SPECIALENA]; LOG("FGA_SPECIALENA returns %02x - not implemented\n", ret); break;
	case FGA_RSVMECALL      : ret = m_fga002[FGA_RSVMECALL]; LOG("FGA_RSVMECALL returns %02x - not implemented\n", ret); break;
	case FGA_RSKEYRES       : ret = m_fga002[FGA_RSKEYRES]; LOG("FGA_RSKEYRES returns %02x - not implemented\n", ret); break;
	case FGA_RSCPUCALL      : ret = m_fga002[FGA_RSCPUCALL]; LOG("FGA_RSCPUCALL returns %02x - not implemented\n", ret); break;
	case FGA_RSLOCSW        : ret = m_fga002[FGA_RSLOCSW]; LOG("FGA_RSLOCSW returns %02x - not implemented\n", ret); break;
	case FGA_ICRMBOX0       : ret = m_fga002[FGA_ICRMBOX0]; LOG("FGA_ICRMBOX0 returns %02x - not implemented\n", ret); break;
	case FGA_ICRMBOX1       : ret = m_fga002[FGA_ICRMBOX1]; LOG("FGA_ICRMBOX1 returns %02x - not implemented\n", ret); break;
	case FGA_ICRMBOX2       : ret = m_fga002[FGA_ICRMBOX2]; LOG("FGA_ICRMBOX2 returns %02x - not implemented\n", ret); break;
	case FGA_ICRMBOX3       : ret = m_fga002[FGA_ICRMBOX3]; LOG("FGA_ICRMBOX3 returns %02x - not implemented\n", ret); break;
	case FGA_ICRMBOX4       : ret = m_fga002[FGA_ICRMBOX4]; LOG("FGA_ICRMBOX4 returns %02x - not implemented\n", ret); break;
	case FGA_ICRMBOX5       : ret = m_fga002[FGA_ICRMBOX5]; LOG("FGA_ICRMBOX5 returns %02x - not implemented\n", ret); break;
	case FGA_ICRMBOX6       : ret = m_fga002[FGA_ICRMBOX6]; LOG("FGA_ICRMBOX6 returns %02x - not implemented\n", ret); break;
	case FGA_ICRMBOX7       : ret = m_fga002[FGA_ICRMBOX7]; LOG("FGA_ICRMBOX7 returns %02x - not implemented\n", ret); break;
	case FGA_VMEPAGE        : ret = m_fga002[FGA_VMEPAGE]; LOG("FGA_VMEPAGE  returns %02x - not implemented\n", ret); break;
	case FGA_ICRVME1        : ret = m_fga002[FGA_ICRVME1]; LOG("FGA_ICRVME1 returns %02x - not implemented\n", ret); break;
	case FGA_ICRVME2        : ret = m_fga002[FGA_ICRVME2]; LOG("FGA_ICRVME2 returns %02x - not implemented\n", ret); break;
	case FGA_ICRVME3        : ret = m_fga002[FGA_ICRVME3]; LOG("FGA_ICRVME3 returns %02x - not implemented\n", ret); break;
	case FGA_ICRVME4        : ret = m_fga002[FGA_ICRVME4]; LOG("FGA_ICRVME4 returns %02x - not implemented\n", ret); break;
	case FGA_ICRVME5        : ret = m_fga002[FGA_ICRVME5]; LOG("FGA_ICRVME5 returns %02x - not implemented\n", ret); break;
	case FGA_ICRVME6        : ret = m_fga002[FGA_ICRVME6]; LOG("FGA_ICRVME6 returns %02x - not implemented\n", ret); break;
	case FGA_ICRVME7        : ret = m_fga002[FGA_ICRVME7]; LOG("FGA_ICRVME7 returns %02x - not implemented\n", ret); break;
	case FGA_ICRTIM0        : ret = do_fga002reg_icrtim0_r(); break;
	case FGA_ICRDMANORM     : ret = m_fga002[FGA_ICRDMANORM]; LOG("FGA_ICRDMANORM returns %02x - not implemented\n", ret); break;
	case FGA_ICRDMAERR      : ret = m_fga002[FGA_ICRDMAERR]; LOG("FGA_ICRDMAERR returns %02x - not implemented\n", ret); break;
	case FGA_CTL1           : ret = m_fga002[FGA_CTL1]; LOG("FGA_CTL1 returns %02x - not implemented\n", ret); break;
	case FGA_CTL2           : ret = m_fga002[FGA_CTL2]; LOG("FGA_CTL2 returns %02x - not implemented\n", ret); break;
	case FGA_ICRFMB0REF     : ret = m_fga002[FGA_ICRFMB0REF]; LOG("FGA_ICRFMB0REF returns %02x - not implemented\n", ret); break;
	case FGA_ICRFMB1REF     : ret = m_fga002[FGA_ICRFMB1REF]; LOG("FGA_ICRFMB1REF returns %02x - not implemented\n", ret); break;
	case FGA_ICRFMB0MES     : ret = m_fga002[FGA_ICRFMB0MES]; LOG("FGA_ICRFMB0MES returns %02x - not implemented\n", ret); break;
	case FGA_ICRFMB1MES     : ret = m_fga002[FGA_ICRFMB1MES]; LOG("FGA_ICRFMB1MES returns %02x - not implemented\n", ret); break;
	case FGA_CTL3           : ret = do_fga002reg_ctl3_r(); break;
	case FGA_CTL4           : ret = m_fga002[FGA_CTL4]; LOG("FGA_CTL4 returns %02x - not implemented\n", ret); break;
	case FGA_ICRPARITY      : ret = m_fga002[FGA_ICRPARITY]; LOG("FGA_ICRPARITY returns %02x - not implemented\n", ret); break;
	case FGA_AUXPINCTL      : ret = m_fga002[FGA_AUXPINCTL]; LOG("FGA_AUXPINCTL returns %02x - not implemented\n", ret); break;
	case FGA_CTL5           : ret = m_fga002[FGA_CTL5]; LOG("FGA_CTL5 returns %02x - not implemented\n", ret); break;
	case FGA_AUXFIFWEX      : ret = m_fga002[FGA_AUXFIFWEX]; LOG("FGA_AUXFIFWEX returns %02x - not implemented\n", ret); break;
	case FGA_AUXFIFREX      : ret = m_fga002[FGA_AUXFIFREX]; LOG("FGA_AUXFIFREX returns %02x - not implemented\n", ret); break;
	case FGA_CTL6           : ret = m_fga002[FGA_CTL6]; LOG("FGA_CTL6 returns %02x - not implemented\n", ret); break;
	case FGA_CTL7           : ret = m_fga002[FGA_CTL7]; LOG("FGA_CTL7 returns %02x - not implemented\n", ret); break;
	case FGA_CTL8           : ret = m_fga002[FGA_CTL8]; LOG("FGA_CTL8 returns %02x - not implemented\n", ret); break;
	case FGA_CTL9           : ret = m_fga002[FGA_CTL9]; LOG("FGA_CTL9 returns %02x - not implemented\n", ret); break;
	case FGA_ICRABORT       : ret = m_fga002[FGA_ICRABORT]; LOG("FGA_ICRABORT returns %02x - not implemented\n", ret); break;
	case FGA_ICRACFAIL      : ret = m_fga002[FGA_ICRACFAIL]; LOG("FGA_ICRACFAIL returns %02x - not implemented\n", ret); break;
	case FGA_ICRSYSFAIL     : ret = m_fga002[FGA_ICRSYSFAIL]; LOG("FGA_ICRSYSFAIL returns %02x - not implemented\n", ret); break;
	case FGA_ICRLOCAL0      : ret = do_fga002reg_icrlocal0_r(); break;
	case FGA_ICRLOCAL1      : ret = do_fga002reg_icrlocal1_r(); break;
	case FGA_ICRLOCAL2      : ret = do_fga002reg_icrlocal2_r(); break;
	case FGA_ICRLOCAL3      : ret = do_fga002reg_icrlocal3_r(); break;
	case FGA_ICRLOCAL4      : ret = do_fga002reg_icrlocal4_r(); break;
	case FGA_ICRLOCAL5      : ret = do_fga002reg_icrlocal5_r(); break;
	case FGA_ICRLOCAL6      : ret = do_fga002reg_icrlocal6_r(); break;
	case FGA_ICRLOCAL7      : ret = do_fga002reg_icrlocal7_r(); break;
	case FGA_ENAMCODE       : ret = m_fga002[FGA_ENAMCODE]; LOG("FGA_ENAMCODE returns %02x - not implemented\n", ret); break;
	case FGA_CTL10          : ret = m_fga002[FGA_CTL10]; LOG("FGA_CTL10 returns %02x - not implemented\n", ret); break;
	case FGA_CTL11          : ret = m_fga002[FGA_CTL11]; LOG("FGA_CTL11 returns %02x - not implemented\n", ret); break;
	case FGA_MAINUM         : ret = m_fga002[FGA_MAINUM]; LOG("FGA_MAINUM returns %02x - not implemented\n", ret); break;
	case FGA_MAINUU         : ret = m_fga002[FGA_MAINUU]; LOG("FGA_MAINUU returns %02x - not implemented\n", ret); break;
	case FGA_BOTTOMPAGEU    : ret = m_fga002[FGA_BOTTOMPAGEU]; LOG("FGA_BOTTOMPAGEU returns %02x - not implemented\n", ret); break;
	case FGA_BOTTOMPAGEL    : ret = m_fga002[FGA_BOTTOMPAGEL]; LOG("FGA_BOTTOMPAGEL returns %02x - not implemented\n", ret); break;
	case FGA_TOPPAGEU       : ret = m_fga002[FGA_TOPPAGEU]; LOG("FGA_TOPPAGEU returns %02x - not implemented\n", ret); break;
	case FGA_TOPPAGEL       : ret = m_fga002[FGA_TOPPAGEL]; LOG("FGA_TOPPAGEL returns %02x - not implemented\n", ret); break;
	case FGA_MYVMEPAGE      : ret = m_fga002[FGA_MYVMEPAGE]; LOG("FGA_MYVMEPAGE returns %02x - not implemented\n", ret); break;
	case FGA_TIM0PRELOAD    : ret = do_fga002reg_tim0preload_r(); break;
	case FGA_TIM0CTL        : ret = do_fga002reg_tim0ctl_r(); break;
	case FGA_DMASRCATT      : ret = m_fga002[FGA_DMASRCATT]; LOG("FGA_DMASRCATT returns %02x - not implemented\n", ret); break;
	case FGA_DMADSTATT      : ret = m_fga002[FGA_DMADSTATT]; LOG("FGA_DMADSTATT returns %02x - not implemented\n", ret); break;
	case FGA_DMA_GENERAL    : ret = m_fga002[FGA_DMA_GENERAL]; LOG("FGA_DMA_GENERAL returns %02x - not implemented\n", ret); break;
	case FGA_CTL12          : ret = m_fga002[FGA_CTL12]; LOG("FGA_CTL12 returns %02x - not implemented\n", ret); break;
	case FGA_LIOTIMING      : ret = m_fga002[FGA_LIOTIMING]; LOG("FGA_LIOTIMING returns %02x - not implemented\n", ret); break;
	case FGA_LOCALIACK      : ret = do_fga002reg_localiack_r(); break;
	case FGA_FMBCTL         : ret = m_fga002[FGA_FMBCTL]; LOG("FGA_FMBCTL returns %02x - not implemented\n", ret); break;
	case FGA_FMBAREA        : ret = m_fga002[FGA_FMBAREA]; LOG("FGA_FMBAREA returns %02x - not implemented\n", ret); break;
	case FGA_AUXSRCSTART    : ret = m_fga002[FGA_AUXSRCSTART]; LOG("FGA_AUXSRCSTART returns %02x - not implemented\n", ret); break;
	case FGA_AUXDSTSTART    : ret = m_fga002[FGA_AUXDSTSTART]; LOG("FGA_AUXDSTSTART returns %02x - not implemented\n", ret); break;
	case FGA_AUXSRCTERM     : ret = m_fga002[FGA_AUXSRCTERM]; LOG("FGA_AUXSRCTERM returns %02x - not implemented\n", ret); break;
	case FGA_AUXDSTTERM     : ret = m_fga002[FGA_AUXDSTTERM]; LOG("FGA_AUXDSTTERM returns %02x - not implemented\n", ret); break;
	case FGA_CTL13          : ret = m_fga002[FGA_CTL13]; LOG("FGA_CTL13 returns %02x - not implemented\n", ret); break;
	case FGA_CTL14          : ret = m_fga002[FGA_CTL14]; LOG("FGA_CTL14 returns %02x - not implemented\n", ret); break;
	case FGA_CTL15          : ret = m_fga002[FGA_CTL15]; LOG("FGA_CTL15 returns %02x - not implemented\n", ret); break;
	case FGA_CTL16          : ret = m_fga002[FGA_CTL16]; LOG("FGA_CTL16 returns %02x - not implemented\n", ret); break;
	case FGA_ISTIM0         : ret = do_fga002reg_istim0_r(); break;
	case FGA_ISLOCAL0       : ret = do_fga002reg_islocal0_r(); break;
	case FGA_ISLOCAL1       : ret = do_fga002reg_islocal1_r(); break;
	case FGA_ISLOCAL2       : ret = do_fga002reg_islocal2_r(); break;
	case FGA_ISLOCAL3       : ret = do_fga002reg_islocal3_r(); break;
	case FGA_ISLOCAL4       : ret = do_fga002reg_islocal4_r(); break;
	case FGA_ISLOCAL5       : ret = do_fga002reg_islocal5_r(); break;
	case FGA_ISLOCAL6       : ret = do_fga002reg_islocal6_r(); break;
	case FGA_ISLOCAL7       : ret = do_fga002reg_islocal7_r(); break;
	case FGA_ISDMANORM      : ret = m_fga002[FGA_ISDMANORM]; LOG("FGA_ISDMANORM returns %02x - not implemented\n", ret); break;
	case FGA_ISDMAERR       : ret = m_fga002[FGA_ISDMAERR]; LOG("FGA_ISDMAERR returns %02x - not implemented\n", ret); break;
	case FGA_ISFMB0REF      : ret = m_fga002[FGA_ISFMB0REF]; LOG("FGA_ISFMB0REF returns %02x - not implemented\n", ret); break;
	case FGA_ISFMB1REF      : ret = m_fga002[FGA_ISFMB1REF]; LOG("FGA_ISFMB1REF returns %02x - not implemented\n", ret); break;
	case FGA_ISPARITY       : ret = m_fga002[FGA_ISPARITY]; LOG("FGA_ISPARITY returns %02x - not implemented\n", ret); break;
	case FGA_DMARUNCTL      : ret = m_fga002[FGA_DMARUNCTL]; LOG("FGA_DMARUNCTL returns %02x - not implemented\n", ret); break;
	case FGA_ISABORT        : ret = m_fga002[FGA_ISABORT]; LOG("FGA_ISABORT returns %02x - not implemented\n", ret); break;
	case FGA_ISFMB0MES      : ret = m_fga002[FGA_ISFMB0MES]; LOG("FGA_ISFMB0MES returns %02x - not implemented\n", ret); break;
	case FGA_ISFMB1MES      : ret = m_fga002[FGA_ISFMB1MES]; LOG("FGA_ISFMB1MES returns %02x - not implemented\n", ret); break;
	case FGA_ABORTPIN       : ret = m_fga002[FGA_ABORTPIN]; LOG("FGA_ABORTPIN returns %02x - not implemented\n", ret); break;
	case FGA_TIM0COUNT      : ret = do_fga002reg_tim0count_r(); break;
	default:
		LOG("Unsupported register %04x\n", offset);
	}
	return ret;
}
