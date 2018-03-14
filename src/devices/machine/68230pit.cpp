// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************
*
*   Motorola MC68230 PI/T Parallel Interface and Timer
*
* Port modes include :
* - bit i/o
* - unidirectional 8 bit and 16 bit
* - bidirectional 8 bit and 16 bit
* Programmable handshaking options
* 24-bit programmable timer modes
* Five separate interrupt vectors separate port and timer interrupt service requests
* Registers are read/write and directly addressable
* Registers are addressed for movep (move peripheral) and dmac compatibility
*
*  Todo
*  - Complete support for clock and timers
*  - Complete interrupt support
*  - Add DMA support
*  - Add appropriate buffering for each submode
**********************************************************************/

#include "emu.h"
#include "68230pit.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_READ    0x04
#define LOG_BIT     0x08
#define LOG_DR      0x10
#define LOG_INT     0x20

#define VERBOSE 0 //(LOG_SETUP | LOG_GENERAL | LOG_INT | LOG_BIT | LOG_DR)
#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,  __VA_ARGS__)
#define LOGBIT(...)   LOGMASKED(LOG_BIT,   __VA_ARGS__)
#define LOGDR(...)    LOGMASKED(LOG_DR,    __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PIT68230, pit68230_device, "pit68230", "MC68230 PI/T")

//-------------------------------------------------
//  pit68230_device - constructors
//-------------------------------------------------
pit68230_device::pit68230_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant)
	: device_t(mconfig, type, tag, owner, clock)
	, m_pa_out_cb(*this)
	, m_pa_in_cb(*this)
	, m_pb_out_cb(*this)
	, m_pb_in_cb(*this)
	, m_pc_out_cb(*this)
	, m_pc_in_cb(*this)
	, m_h1_out_cb(*this)
	, m_h2_out_cb(*this)
	, m_h3_out_cb(*this)
	, m_h4_out_cb(*this)
	, m_tirq_out_cb(*this)
	, m_pirq_out_cb(*this)
	, m_pgcr(0)
	, m_psrr(0)
	, m_paddr(0)
	, m_pbddr(0)
	, m_pcddr(0)
	, m_pivr(0)
	, m_pacr(0)
	, m_pbcr(0)
	, m_padr(0)
	, m_pbdr(0)
	, m_pcdr(0)
	, m_psr(0)
	, m_tcr(0)
	, m_tivr(0)
	, m_cpr(0)
	//  , m_cprh(0) // Collectivelly handled by m_cpr
	//  , m_cprm(0) // Collectivelly handled by m_cpr
	//  , m_cprl(0) // Collectivelly handled by m_cpr
	, m_cntr(0)
	, m_tsr(0)
{
	// FIXME: is the unused variant parameter supposed to be useful for something?
}


pit68230_device::pit68230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pit68230_device (mconfig, PIT68230, tag, owner, clock, 0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void pit68230_device::device_start ()
{
	LOGSETUP("%s\n", FUNCNAME);

	// NOTE:
	// Not using resolve_safe for the m_px_in_cb's is a temporary way to be able to check
	// if a handler is installed with isnull(). The safe function installs a dummy default
	// handler which disable the isnull() test. TODO: Need a better fix?

	// resolve callbacks Port A
	m_pa_out_cb.resolve_safe();
	m_pa_in_cb.resolve();

	// resolve callbacks Port B
	m_pb_out_cb.resolve_safe();
	m_pb_in_cb.resolve();

	// resolve callbacks Port C
	m_pc_out_cb.resolve_safe();
	m_pc_in_cb.resolve();

	m_h1_out_cb.resolve_safe();
	m_h2_out_cb.resolve_safe();
	m_h3_out_cb.resolve_safe();
	m_h4_out_cb.resolve_safe();

	m_tirq_out_cb.resolve_safe();
	m_pirq_out_cb.resolve_safe();

	// Timers
	pit_timer = timer_alloc(TIMER_ID_PIT);

	// state saving
	save_item(NAME(m_pgcr));
	save_item(NAME(m_psrr));
	save_item(NAME(m_paddr));
	save_item(NAME(m_pbddr));
	save_item(NAME(m_pcddr));
	save_item(NAME(m_pivr));
	save_item(NAME(m_pacr));
	save_item(NAME(m_pbcr));
	save_item(NAME(m_padr));
	save_item(NAME(m_pbdr));
	save_item(NAME(m_pcdr));
	save_item(NAME(m_psr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tivr));
	save_item(NAME(m_cpr));
	save_item(NAME(m_cntr));
	save_item(NAME(m_tsr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void pit68230_device::device_reset ()
{
	LOGSETUP("%s %s \n",tag(), FUNCNAME);

	m_pgcr = 0;
	m_psrr = 0;
	m_paddr = 0;
	m_pbddr = 0;
	m_pcddr = 0;
	m_pivr = 0x0f;  m_pirq_out_cb(CLEAR_LINE);
	m_pacr = 0; m_h2_out_cb(CLEAR_LINE);
	m_pbcr = 0; m_h4_out_cb(CLEAR_LINE);
	m_padr = 0; m_pa_out_cb((offs_t)0, m_padr);
	m_pbdr = 0; m_pb_out_cb((offs_t)0, m_pbdr);
	m_pcdr = 0; m_pc_out_cb((offs_t)0, m_pcdr);
	m_psr = 0;
	m_tcr = 0;
	m_tivr = 0x0f; m_tirq_out_cb(CLEAR_LINE);
	m_tsr = 0;
	LOGSETUP("%s %s DONE!\n",tag(), FUNCNAME);
}

/*
 * PIACK* provides the Port vector in an iack cycle modified by source H1-H4
 */
uint8_t pit68230_device::irq_piack()
{
	LOGINT("%s %s <- %02x\n",tag(), FUNCNAME, m_pivr);
	return m_pivr;
}

/*
 * TIACK* provides the Timer vector in an iack cycle
 */
uint8_t pit68230_device::irq_tiack()
{
	LOGINT("%s %s <- %02x\n",tag(), FUNCNAME, m_tivr);
	return m_tivr;
}

/*
 * trigger_interrupt - called when a potential interrupt condition occurs
 * but will only generate an interrupt when the PIT is programmed to do so.
 */
void pit68230_device::trigger_interrupt(int source)
{
	LOGINT("%s %s Source: %02x\n",tag(), FUNCNAME, source);

	if (source == INT_TIMER)
	{
		// TODO: implement priorities and support nested interrupts
		if ( (m_tcr & REG_TCR_TOUT_TIACK_MASK) == REG_TCR_TOUT_TIACK_INT ||
			 (m_tcr & REG_TCR_TOUT_TIACK_MASK) == REG_TCR_TOUT_PC7_INT )
			{
				m_tirq_out_cb(ASSERT_LINE);
			}
	}
	else
	{
		// TODO: implement priorities and support nested interrupts for the H1-H4 sources
		m_pirq_out_cb(ASSERT_LINE);
	}
}

void pit68230_device::tick_clock()
{
	if (m_tcr & REG_TCR_TIMER_ENABLE)
	{
		if (m_cntr-- == 0) // Zero detect
		{
			LOGINT("Timer reached zero!\n");
			if ((m_tcr & REG_TCR_ZD) == 0)
				m_cntr = m_cpr;
			else // mask off to 24 bit on rollover
				m_cntr &= 0xffffff;
			m_tsr = 1;
			trigger_interrupt(INT_TIMER);
		}
	}
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
void pit68230_device::device_timer (emu_timer &timer, device_timer_id id, int32_t param, void *ptr)
{
	switch(id)
	{
	case TIMER_ID_PIT:
		tick_clock();
		break;
	default:
		LOG("Unhandled Timer ID %d\n", id);
		break;
	}
}

WRITE_LINE_MEMBER( pit68230_device::h1_w )
{
	LOGBIT("%s %s H1 set to %d\n", tag(), FUNCNAME, state);

	// Set the direct level in PSR
	m_psr = ((state == 0) ? (m_psr & ~REG_PSR_H1L) : (m_psr | REG_PSR_H1L));

	// Set the programmed assert level in PSR
	if (m_pgcr & REG_PGCR_H1_SENSE)
		m_psr = ((state != 0) ? (m_psr & ~REG_PSR_H1S) : (m_psr | REG_PSR_H1S));
	else
		m_psr = ((state == 0) ? (m_psr & ~REG_PSR_H1S) : (m_psr | REG_PSR_H1S));
}

WRITE_LINE_MEMBER( pit68230_device::h2_w )
{
	LOGBIT("%s %s H2 set to %d\n", tag(), FUNCNAME, state);

	// Set the direct level in PSR
	m_psr = ((state == 0) ? (m_psr & ~REG_PSR_H2L) : (m_psr | REG_PSR_H2L));

	// Set the programmed assert level in PSR
	if (m_pgcr & REG_PGCR_H2_SENSE)
		m_psr = ((state != 0) ? (m_psr & ~REG_PSR_H2S) : (m_psr | REG_PSR_H2S));
	else
		m_psr = ((state == 0) ? (m_psr & ~REG_PSR_H2S) : (m_psr | REG_PSR_H2S));
}

WRITE_LINE_MEMBER( pit68230_device::h3_w )
{
	LOGBIT("%s %s H3 set to %d\n", tag(), FUNCNAME, state);

	// Set the direct level in PSR
	m_psr = ((state == 0) ? (m_psr & ~REG_PSR_H3L) : (m_psr | REG_PSR_H3L));

	// Set the programmed assert level in PSR
	if (m_pgcr & REG_PGCR_H3_SENSE)
		m_psr = ((state != 0) ? (m_psr & ~REG_PSR_H3S) : (m_psr | REG_PSR_H3S));
	else
		m_psr = ((state == 0) ? (m_psr & ~REG_PSR_H3S) : (m_psr | REG_PSR_H3S));
}

WRITE_LINE_MEMBER( pit68230_device::h4_w )
{
	LOGBIT("%s %s H4 set to %d\n", tag(), FUNCNAME, state);

	// Set the direct level in PSR
	m_psr = ((state == 0) ? (m_psr & ~REG_PSR_H4L) : (m_psr | REG_PSR_H4L));

	// Set the programmed assert level in PSR
	if (m_pgcr & REG_PGCR_H4_SENSE)
		m_psr = ((state != 0) ? (m_psr & ~REG_PSR_H4S) : (m_psr | REG_PSR_H4S));
	else
		m_psr = ((state == 0) ? (m_psr & ~REG_PSR_H4S) : (m_psr | REG_PSR_H4S));
}

// TODO: remove this method and replace it with a call to pb_update_bit() in force68k.cpp
void pit68230_device::portb_setbit(uint8_t bit, uint8_t state)
{
	if (state) m_pbdr |= (1 << bit); else m_pbdr &= ~(1 << bit);
}

void pit68230_device::pa_update_bit(uint8_t bit, uint8_t state)
{
	LOGBIT("%s %s bit %d to %d\n", tag(), FUNCNAME, bit, state);
	// Check if requested bit is an output bit and can't be affected
	if (m_paddr & (1 << bit))
	{
		LOG("- 68230 PIT: tried to set input bit at port A that is programmed as output!\n");
		return;
	}
	if (state)
	{
		m_padr |= (1 << bit);
		m_pail |= (1 << bit);
	}
	else
	{
		m_padr &= ~(1 << bit);
		m_pail &= ~(1 << bit);
	}
}

void pit68230_device::pb_update_bit(uint8_t bit, uint8_t state)
{
	LOGBIT("%s %s bit %d to %d\n",tag(), FUNCNAME, bit, state);
	// Check if requested bit is an output bit and can't be affected
	if (m_pbddr & (1 << bit))
	{
		LOG("- 68230 PIT: tried to set input bit at port B that is programmed as output!\n");
		return;
	}
	if (state)
	{
		m_pbdr |= (1 << bit);
		m_pbil |= (1 << bit);
	}
	else
	{
		m_pbdr &= ~(1 << bit);
		m_pbil &= ~(1 << bit);
	}
}

// TODO: Make sure port C is in the right alternate mode
void pit68230_device::pc_update_bit(uint8_t bit, uint8_t state)
{
	LOGBIT("%s %s bit %d to %d\n",tag(), FUNCNAME, bit, state);
	// Check if requested bit is an output bit and can't be affected
	if (m_pcddr & (1 << bit))
	{
		LOG("- 68230 PIT: tried to set input bit at port C that is programmed as output!\n");
		return;
	}
	if (state)
	{
		m_pcdr |= (1 << bit);
		m_pcil |= (1 << bit);
	}
	else
	{
		m_pcdr &= ~(1 << bit);
		m_pcil &= ~(1 << bit);
	}
}

void pit68230_device::update_tin(uint8_t state)
{
	// Tick clock on falling edge. TODO: check what flank is correct
	if (state == CLEAR_LINE)
	{
		tick_clock();
	}

	pc_update_bit(REG_PCDR_TIN_BIT, state == ASSERT_LINE ? 0 : 1);
}

#if VERBOSE > 2
static int32_t ow_cnt = 0;
static int32_t ow_data = 0;
static int32_t ow_ofs = 0;
#endif

void pit68230_device::wr_pitreg_pgcr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	LOGSETUP("PGCR  - Mode %d,", (data >> 6) & 3 );
	LOGSETUP(" H34:%s, H12:%s,", (data & 0x20) ? "enabled" : "disabled", (data & 0x10) ? "enabled" : "disabled" );
	LOGSETUP(" Sense assert H4:%s, H3:%s, H2:%s, H1:%s\n",
			  data & 0x04 ? "Hi" : "Lo", data & 0x03 ? "Hi" : "Lo",
			  data & 0x02 ? "Hi" : "Lo", data & 0x01 ? "Hi" : "Lo");
	m_pgcr = data;
}

void pit68230_device::wr_pitreg_psrr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	LOGSETUP("PSSR - %s pin activated,", data & 0x40 ? "DMA" : "PC4");
	LOGSETUP(" %s pin support %s interrupts,", data & 0x80 ? "PIRQ" : "PC5",
			  data & 0x08 ? "no" : (data & 0x10 ? "vectored" : "autovectored" ) );
	LOGSETUP(" H prio mode:%d\n", data & 0x03 );

	m_psrr = data;
}

void pit68230_device::wr_pitreg_paddr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	LOGSETUP("%s PADDR: %02x\n", tag(), data);
	m_paddr = data;
}

void pit68230_device::wr_pitreg_pbddr(uint8_t data)
{
	LOGSETUP("%s PBDDR: %02x\n", tag(), data);
	m_pbddr = data;
}

void pit68230_device::wr_pitreg_pcddr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	LOGSETUP("%s PCDDR: %02x\n", tag(), data);
	m_pcddr = data;
}

void pit68230_device::wr_pitreg_pivr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	LOGSETUP("%s PIVR: %02x\n", tag(), data);
	m_pivr = data & 0xfc; // lowest two bits are read as zero
}

void pit68230_device::wr_pitreg_pacr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	LOGSETUP("%s PACR", tag());
	m_pacr = data;
	// callbacks
	/*PACR in Mode 0
	 * 5 43  H2 Control in Submode 00 && 01
	 * ------------------------------------
	 * 0 XX  Input pin  - edge-sensitive status input, H2S is set on an asserted edge.
	 * 1 00  Output pin - negated, H2S is always clear.
	 * 1 01  Output pin - asserted, H2S is always clear.
	 * 1 10  Output pin - interlocked input handshake protocol, H2S is always clear.
	 * 1 11  Output pin - pulsed input handshake protocol, H2S is always clear.
	 *
	 * 5 43  H2 Control in Submode 1x
	 * ------------------------------------
	 * 0 XX  Input pin  - edge-sensitive status input, H2S is set on an asserted edge.
	 * 1 X0  Output pin - negated, H2S is always cleared.
	 * 1 X1  Output pin - asserted, H2S is always cleared.
	 */
	if (m_pgcr & REG_PGCR_H12_ENABLE)
	{
		if (m_pacr & REG_PACR_H2_CTRL_IN_OUT)
		{
			switch(m_pacr & REG_PACR_H2_CTRL_MASK)
			{
			case REG_PACR_H2_CTRL_OUT_00:
				LOGSETUP(" - H2 cleared\n");
				m_h2_out_cb(CLEAR_LINE);
				break;
			case REG_PACR_H2_CTRL_OUT_01:
				LOGSETUP(" - H2 asserted\n");
				m_h2_out_cb(ASSERT_LINE);
				break;
			case REG_PACR_H2_CTRL_OUT_10:
				LOGSETUP(" - interlocked handshake not implemented\n");
				break;
			case REG_PACR_H2_CTRL_OUT_11:
				LOGSETUP(" - pulsed handshake not implemented\n");
				break;
			default: logerror(("Undefined H2 mode, broken driver - please report!\n"));
			}
		}
	}
	else
	{
		LOGSETUP(" - H2 cleared because being disabled in PGCR\n");
		m_h2_out_cb(CLEAR_LINE);
	}
}

// TODO add support for sense status
void pit68230_device::wr_pitreg_pbcr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	m_pbcr = data;
	if ((m_pgcr & REG_PGCR_H34_ENABLE) || ((m_pbcr & REG_PBCR_SUBMODE_MASK) == REG_PBCR_SUBMODE_1X))
	{
		if (m_pbcr & REG_PBCR_H4_CTRL_IN_OUT)
		{
			switch(m_pbcr & REG_PBCR_H4_CTRL_MASK)
			{
			case REG_PBCR_H4_CTRL_OUT_00:
				LOG(" - H4 cleared\n");
				m_h4_out_cb(CLEAR_LINE);
				break;
			case REG_PBCR_H4_CTRL_OUT_01:
				LOG(" - H4 asserted\n");
				m_h4_out_cb(ASSERT_LINE);
				break;
			case REG_PBCR_H4_CTRL_OUT_10:
				LOGSETUP(" - interlocked handshake not implemented\n");
				break;
			case REG_PBCR_H4_CTRL_OUT_11:
				LOGSETUP(" - pulsed handshake not implemented\n");
				break;
			default: logerror(("Undefined H4 mode, broken driver - please report!\n"));
			}
		}
	}
	else
	{
		LOG(" - H4 cleared because being disabled in PGCR\n");
		m_h4_out_cb(CLEAR_LINE);
	}
}

void pit68230_device::wr_pitreg_padr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	m_padr = (data & m_paddr);

	// callback
	m_pa_out_cb ((offs_t)0, m_padr);
}

void pit68230_device::wr_pitreg_pbdr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	m_pbdr = (data & m_pbddr);

	// callback
	m_pb_out_cb ((offs_t)0, m_pbdr & m_pbddr);
}

void pit68230_device::wr_pitreg_pcdr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	m_pcdr = (data & m_pcddr);

	// callback
	m_pc_out_cb ((offs_t)0, m_pcdr);
}

void pit68230_device::wr_pitreg_psr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	m_psr = data;
}

/* The timer control register (TCR) determines all operations of the timer. Bits 7-5 configure the PC3/TOUT
and PC7/TIACKpins for port C, square wave, vectored interrupt, or autovectored interrupt operation bit
4 specifies whether the counter receives data from the counter preload register or continues counting when
zero detect is reached ; bit 3 is unused and is read as zero bits 2 and 1 configure the path from the CLK
and TINpins to the counter controller ; and bit 0 ena-bles the timer. This register is readable and writable
at all times. All bits are cleared to zero when the RESET pin is asserted.

TCR bits
 7 6 5   TOUT/TIACK Control
----------------------------
 0 0 X  The dual-function pins PC3/TOUT and PC7/TIACK carry the port C function.
 0 1 X  The dual-function pinPC3/TOUT carries the TOUT function. In the run state it is used as a squarewave
        output and is toggled on zero detect. The TOUT pin is high while in the halt state. The dualfunction
        pin PC7/TIACK carries the PC7 function.
 1 0 0  The dual-function pin PC3/TOUT carries the TOUT function. In the run or halt state it is used as
        a timer interrupt request output. The timer interrupt is disabled, thus, the pin is always three stated.
        The dual-function pin PC7/TIACK carries the TIACK function ; however, since interrupt request is
        negated, the PI/T produces no response (i.e., no data or DTACK) to an asserted TIACK. Refer to
        5.1.3. Timer Interrupt Acknowledge Cycles for details.
 1 0 1  The dual-function pin PC3/TOUT carries the TOUTfunction and is used as a timer interrupt request
        output. The timer interrupt is enabled ; thus, the pin is low when the timer ZDS status bit is one.
        The dual-function pin PC7/TIACK carries the TIACK function and is used as a timer interrupt acknowledge
        input. Refer to the5.1.3. Timer InterruptAcknowledge Cycles fordetails. Thiscombination
        supports vectored timer interrupts.
 1 1 0  The dual-function pin PC3/TOUT function. In the run or halt state it is used as a timer interrupt
        request output. The timer interrupt is disabled ; thus, the pin is always three-stated. The dual-function
        pin PC7/TIACK carries the PC7 function.
 1 1 1  The dual-function pin PC3/TOUT carries the TOUTfunction and is used as a timer interrupt request
        output. The timer interrupt is enabled ; thus, the pin is low when the timer ZDS status bit is one.
        The dual-function pin PC7/TIACK carries the PC7 function and autovectored interrupts are supported.

TCR bit 4 - Zero Detect Control
     0  The counter is loaded fromthe counter preload register on the first clock to the 24-bit counter after
        zero detect, then resumes counting.
     1  The counter rolls over on zero detect, then continues counting.

TCR bit 3 - Unused and is always read as zero.

TCR bits
   2 1  Clock Control
   0 0  The PC2/TIN input pin carries the port C function, and the CLK pin and prescaler are used. The
        prescaler is decremented on the falling transition of the CLKpin ; the 24-bit counter is decremented,
        rolls over, or is loaded from the counter preload registers when the prescaler rolls over from $OO
        to $1F. The timer enable bit determines whether the timer is in the run or halt state.
   0 1  The PC2/TIN pin serves as a timer input, and the CLK pin and prescaler are used. The prescaler
        is decremented on the falling transition of the CLK pin ; the 24-bit counter is decremented, rolls
        over, or is loaded from the counter preload registers when the prescaler rolls over from $00 to $1F.
        The timer is in the run state when the timer enable bit is one and the TIN pin is high ; otherwise,
        the timer is in the halt state.
   1 0  The PC2/TIN pin serves as a timer input and the prescaler is used. The prescaler is decremented
        following the rising transition of the TIN pin after being synchronized with the internal clock. The
        24-bit counter is decremented, rolls over, or is loaded from the counter preload registers when the
        prescaler rolls over from $00 to $1F. The timer enable bit determines whether the timer is in the
        run or halt state.
   1 1  The PC2/TIN pin serves as a timer input and the prescaler is not used. The 24-bit counter is decremented,
        rolls over, or is loaded from the counter preload registers following the rising edge of
        the TIN pin after being synchronized with the internal clock. The timer enable bit determines whether
        the timer is in the run or halt state.
TCR bit 0 - Timer Enable
     0  Disabled
     1  Enabled
*/
void pit68230_device::wr_pitreg_tcr(uint8_t data)
{
	//int tout  = 0;
	//int tiack = 0;
	//int irq   = 0;
	int psc   = 0;
	int clk   = 0;
	int pen   = 0;
	//int sqr   = 0;

	LOG("%s(%02x) %s\n", FUNCNAME, data, tag());
	m_tcr = data;
	switch (m_tcr & REG_TCR_TOUT_TIACK_MASK)
	{
	case REG_TCR_PC3_PC7:
	case REG_TCR_PC3_PC7_DC:        LOG("- PC3 and PC7 used as I/O pins\n"); break;
	case REG_TCR_TOUT_PC7_SQ:
	case REG_TCR_TOUT_PC7_SQ_DC:    LOG("- PC3 used as SQuare wave TOUT and PC7 used as I/O pin - not implemented yet\n");       /* sqr = 1; */ break;
	case REG_TCR_TOUT_TIACK:        LOG("- PC3 used as TOUT and PC7 used as TIACK - not implemented yet\n"); /*tout = 1; tiack = 1;*/          break;
	case REG_TCR_TOUT_TIACK_INT:    LOG("- PC3 used as TOUT and PC7 used as TIACK, Interrupts enabled\n");   /*tout = 1; tiack = 1; irq = 1;*/ break;
	case REG_TCR_TOUT_PC7:          LOG("- PC3 used as TOUT and PC7 used as I/O pin - not implemented yet\n");                             break;
	case REG_TCR_TOUT_PC7_INT:      LOG("- PC3 used as TOUT and PC7 used as I/O pin, Interrupts enabled\n"); /*tout = 1; irq = 1; */           break;
	}

	switch (m_tcr & REG_TCR_CC_MASK)
	{
	case REG_TCR_CC_PC2_CLK_PSC:    LOG("- PC2 used as I/O pin,CLK and x32 prescaler are used\n");                                       clk = 1; psc = 1; break;
	case REG_TCR_CC_TEN_CLK_PSC:    LOG("- PC2 used as Timer enable/disable, CLK and presacaler are used - not implemented\n"); pen = 1; clk = 1; psc = 1; break;
	case REG_TCR_CC_TIN_PSC:        LOG("- PC2 used as Timer clock and the presacaler is used - not implemented\n");                              psc = 1; break;
	case REG_TCR_CC_TIN_RAW:        LOG("- PC2 used as Timer clock and the presacaler is NOT used\n"); break;
	}
	LOG("%s", m_tcr & REG_TCR_ZR ? "- Spec violation, should always be 0!\n" : "");
	LOG("- Timer %s when reaching 0 (zero)\n", m_tcr & REG_TCR_ZD ? "rolls over" : "reload the preload values");
	LOG("- Timer is %s\n", m_tcr & REG_TCR_ENABLE ? "enabled" : "disabled");

	if (m_tcr & REG_TCR_ENABLE)
	{
		m_cntr = 0;
		if (pen == 1)
		{
			LOG("PC2 enable/disable TBD\n");
		}
		if (clk == 1)
		{
			int rate = clock() / (psc == 1 ? 32 : 1);
			pit_timer->adjust(attotime::from_hz(rate), TIMER_ID_PIT, attotime::from_hz(rate));
			LOG("PIT timer started @ rate: %d and CLK: %d,\n", rate, clock());
		}
	}
	else
	{
		pit_timer->adjust(attotime::never, TIMER_ID_PIT, attotime::never);
	}
}

void pit68230_device::wr_pitreg_tivr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": \n", FUNCNAME, data, tag());
	m_tivr = data;
}

void pit68230_device::wr_pitreg_cprh(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	m_cpr &= ~0xff0000;
	m_cpr |= ((data << 16) & 0xff0000);
}

void pit68230_device::wr_pitreg_cprm(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	m_cpr &= ~0x00ff00;
	m_cpr |= ((data << 8) & 0x00ff00);
}

void pit68230_device::wr_pitreg_cprl(uint8_t data)
{
	LOG("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, tag(), FUNCNAME, data);
	m_cpr &= ~0x0000ff;
	m_cpr |= ((data << 0) & 0x0000ff);
}

void pit68230_device::wr_pitreg_tsr(uint8_t data)
{
	LOG("%s(%02x) \"%s\": \n", FUNCNAME, data, tag());
	if (data & 1)
	{
		m_tsr = 0; // A write resets the TSR;
		m_tirq_out_cb(CLEAR_LINE);
	}
}

WRITE8_MEMBER (pit68230_device::write)
{
	LOG("\"%s\" %s: Register write '%02x' -> [%02x]\n", tag(), FUNCNAME, data, offset );
	LOGSETUP(" * %s Reg %02x <- %02x  \n", tag(), offset, data);
	switch (offset) {
	case PIT_68230_PGCR:    wr_pitreg_pgcr(data); break;
	case PIT_68230_PSRR:    wr_pitreg_psrr(data); break;
	case PIT_68230_PADDR:   wr_pitreg_paddr(data); break;
	case PIT_68230_PBDDR:   wr_pitreg_pbddr(data); break;
	case PIT_68230_PCDDR:   wr_pitreg_pcddr(data); break;
	case PIT_68230_PIVR:    wr_pitreg_pivr(data); break;
	case PIT_68230_PACR:    wr_pitreg_pacr(data); break;
	case PIT_68230_PBCR:    wr_pitreg_pbcr(data); break;
	case PIT_68230_PADR:    wr_pitreg_padr(data); break;
	case PIT_68230_PBDR:    wr_pitreg_pbdr(data); break;
	case PIT_68230_PAAR:    break; // Ignores write per spec, read only register
	case PIT_68230_PBAR:    break; // Ignores write per spec, read only register
	case PIT_68230_PCDR:    wr_pitreg_pcdr(data); break;
	case PIT_68230_PSR:     wr_pitreg_psr(data); break;
	case PIT_68230_TCR:     wr_pitreg_tcr(data); break;
	case PIT_68230_TIVR:    wr_pitreg_tivr(data); break;
	case PIT_68230_CPRH:    wr_pitreg_cprh(data); break;
	case PIT_68230_CPRM:    wr_pitreg_cprm(data); break;
	case PIT_68230_CPRL:    wr_pitreg_cprl(data); break;
	case PIT_68230_CNTRH:   break; // Ignores write per spec, read only register
	case PIT_68230_CNTRM:   break; // Ignores write per spec, read only register
	case PIT_68230_CNTRL:   break; // Ignores write per spec, read only register
	case PIT_68230_TSR:     wr_pitreg_tsr(data); break;
	default:
		LOG("Unhandled Write of %02x to register %02x", data, offset);
	}

#if VERBOSE > 2
	if (offset != ow_ofs || data != ow_data || ow_cnt >= 1000) {
		if (ow_cnt > 1)
		{
			logerror ("\npit68230_device::write: previous identical operation performed %02x times\n", ow_cnt);
			logerror ("pit68230_device::write: offset=%02x data=%02x %s\n", offset, data, machine().describe_context());
		}
		ow_cnt = 0;
		ow_data = data;
		ow_ofs = offset;
	}
	else
		ow_cnt++;
#endif

}

#if VERBOSE > 2
static int32_t or_cnt = 0;
static int32_t or_data = 0;
static int32_t or_ofs = 0;
#endif

uint8_t pit68230_device::rr_pitreg_pgcr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_pgcr);
	return m_pgcr;
}

uint8_t pit68230_device::rr_pitreg_psrr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_psrr);
	return m_psrr & 0x7f; // mask out unused bits
}

uint8_t pit68230_device::rr_pitreg_paddr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_paddr);
	return m_paddr;
}

uint8_t pit68230_device::rr_pitreg_pbddr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_pbddr);
	return m_pbddr;
}

uint8_t pit68230_device::rr_pitreg_pcddr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_pcddr);
	return m_pcddr;
}

uint8_t pit68230_device::rr_pitreg_pivr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_pivr);
	return m_pivr;
}

uint8_t pit68230_device::rr_pitreg_pacr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_pacr);
	return m_pacr;
}

uint8_t pit68230_device::rr_pitreg_pbcr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_pbcr);
	return m_pbcr;
}

uint8_t pit68230_device::rr_pitreg_padr()
{
	m_padr &= m_paddr;
	if (!m_pa_in_cb.isnull())
	{
		m_padr |= (m_pa_in_cb() & ~m_paddr);
	}
	else
	{
		m_padr |= (m_pail & ~m_paddr);
	}
	LOGDR("%s %s <- %02x\n",tag(), FUNCNAME, m_padr);
	return m_padr;
}

/* 4.6.2. PORT B DATA REGISTER (PBDR). The port B data register is a holding
 * register for moving data to and from port B pins. The port B data direction
 * register determines whether each pin is an input (zero) or an output (one).
 * This register is readable and writable at all times. Depending on the chosen
 * mode/submode, reading or writing may affect the double-buffered handshake
 * mechanism. The port B data register is not affected by the assertion of the
 * RESET pin. PB0-PB7 sits on pins 17-24 on a 48 pin DIP package */
uint8_t pit68230_device::rr_pitreg_pbdr()
{
	m_pbdr &= m_pbddr;
	if (!m_pb_in_cb.isnull())
	{
		m_pbdr |= (m_pb_in_cb() & ~m_pbddr);
	}
	else
	{
		m_pbdr |= (m_pbil & ~m_pbddr);
	}

	LOGDR("%s %s <- %02x\n",tag(), FUNCNAME, m_pbdr);

	return m_pbdr;
}

uint8_t pit68230_device::rr_pitreg_pcdr()
{
	m_pcdr &= m_pcddr;
	if (!m_pc_in_cb.isnull()) // Port C has alternate functions that may set bits apart from callback
	{
		m_pcdr |= (m_pc_in_cb() & ~m_pcddr);
	}
	else
	{
		m_pcdr |= (m_pcil & ~m_pcddr);
	}

	if (m_pcdr != 0) { LOGDR("%s %s <- %02x\n",tag(), FUNCNAME, m_pcdr); }

	return m_pcdr;
}

/* The port A alternate register is an alternate register for reading the port A pins.
It is a read-only address and no other PI/T condition is affected. In all modes,
the instantaneous pin level is read and no input latching is performed except at the
data bus interface. Writes to this address are answered with DTACK, but the data is ignored.*/
uint8_t pit68230_device::rr_pitreg_paar()
{
	uint8_t ret;
	ret = m_pa_in_cb.isnull() ? 0 : m_pa_in_cb();
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, ret);
	return ret;
}

/* The port B alternate register is an alternate register for reading the port B pins.
It is a read-only address and no other PI/T condition is affected. In all modes,
the instantaneous pin level is read and no input latching is performed except at the
data bus interface.Writes to this address are answered with DTACK, but the data is ignored.*/
uint8_t pit68230_device::rr_pitreg_pbar()
{
	uint8_t ret;
	ret = m_pb_in_cb.isnull() ? 0 : m_pb_in_cb();
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, ret);
	return ret;
}

/* 4.8. PORT STATUS REGISTER (PSR) The port status register contains information about
 * handshake pin activity. Bits 7-4 show the instantaneous level of the respective handshake
 * pin, and are independent of the handshake pin sense bits in the port general control
 * register. Bits 3-0 are the respective status bits referred to throughout this document.
 * Their interpretation depends on the programmed mode/submode of the PI/T. For bits
 * 3-0 a one is the active or asserted state. */
uint8_t pit68230_device::rr_pitreg_psr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_psr);
	return m_psr;
}

uint8_t pit68230_device::rr_pitreg_tcr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_tcr);
	return m_tcr;
}

uint8_t pit68230_device::rr_pitreg_tivr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_tivr);
	return m_tivr;
}

uint8_t pit68230_device::rr_pitreg_cprh()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, (m_cpr >> 16) & 0xff);
	return (m_cpr >> 16) & 0xff;
}

uint8_t pit68230_device::rr_pitreg_cprm()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, (m_cpr >> 8) & 0xff);
	return (m_cpr >> 8) & 0xff;
}

uint8_t pit68230_device::rr_pitreg_cprl()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, (m_cpr >> 0) & 0xff);
	return (m_cpr >> 0) & 0xff;
}

uint8_t pit68230_device::rr_pitreg_cntrh()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, (m_cntr >> 16) & 0xff);
	return (m_cntr >> 16) & 0xff;
}

uint8_t pit68230_device::rr_pitreg_cntrm()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, (m_cntr >> 8) & 0xff);
	return (m_cntr >> 8) & 0xff;
}

uint8_t pit68230_device::rr_pitreg_cntrl()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, (m_cntr >> 0) & 0xff);
	return (m_cntr >> 0) & 0xff;
}

uint8_t pit68230_device::rr_pitreg_tsr()
{
	LOGR("%s %s <- %02x\n",tag(), FUNCNAME, m_tsr);
	return m_tsr;
}

READ8_MEMBER (pit68230_device::read){
	uint8_t data;

	switch (offset) {
	case PIT_68230_PGCR:    data = rr_pitreg_pgcr(); break;
	case PIT_68230_PSRR:    data = rr_pitreg_psrr(); break;
	case PIT_68230_PADDR:   data = rr_pitreg_paddr(); break;
	case PIT_68230_PBDDR:   data = rr_pitreg_pbddr(); break;
	case PIT_68230_PCDDR:   data = rr_pitreg_pcddr(); break;
	case PIT_68230_PIVR:    data = rr_pitreg_pivr(); break;
	case PIT_68230_PACR:    data = rr_pitreg_pacr(); break;
	case PIT_68230_PBCR:    data = rr_pitreg_pbcr(); break;
	case PIT_68230_PADR:    data = rr_pitreg_padr(); break;
	case PIT_68230_PBDR:    data = rr_pitreg_pbdr(); break;
	case PIT_68230_PAAR:    data = rr_pitreg_paar(); break;
	case PIT_68230_PBAR:    data = rr_pitreg_pbar(); break;
	case PIT_68230_PCDR:    data = rr_pitreg_pcdr(); break;
	case PIT_68230_PSR:     data = rr_pitreg_psr(); break;
	case PIT_68230_TCR:     data = rr_pitreg_tcr(); break;
	case PIT_68230_TIVR:    data = rr_pitreg_tivr(); break;
	case PIT_68230_CPRH:    data = rr_pitreg_cprh(); break;
	case PIT_68230_CPRM:    data = rr_pitreg_cprm(); break;
	case PIT_68230_CPRL:    data = rr_pitreg_cprl(); break;
	case PIT_68230_CNTRH:   data = rr_pitreg_cntrh(); break;
	case PIT_68230_CNTRM:   data = rr_pitreg_cntrm(); break;
	case PIT_68230_CNTRL:   data = rr_pitreg_cntrl(); break;
	case PIT_68230_TSR:     data = rr_pitreg_tsr(); break;
	default:
		LOG("Unhandled read register %02x returning 0x00\n", offset);
		data = 0;
	}

#if VERBOSE > 2
	if (offset != or_ofs || data != or_data || or_cnt >= 1000) {
		LOGSETUP(" * %s Reg %02x -> %02x  \n", tag(), offset, data);
		if (or_cnt > 1)
		{
			logerror ("\npit68230_device::read: previous identical operation performed %02x times\n", or_cnt);
			logerror (" - pit68230_device::read: offset=%02x data=%02x %s\n", offset, data, machine().describe_context());
		}
		or_cnt = 0;
		or_data = data;
		or_ofs = offset;
	}
	else
	{
		or_cnt++;
	}
#endif

	return data;
}
