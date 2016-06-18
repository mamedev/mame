// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstr??m
/**********************************************************************
*
*   Motorola MC68230 PI/T Parallell Interface and Timer
*
*  Revisions
*  2015-07-15 JLE initial
*
*  Todo
*  - Add clock and timers
*  - Add double buffering for each submode
*  - Add all missing registers
*  - Add configuration
**********************************************************************/

#include "68230pit.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#define LOGR(x) LOG(x)
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

//#define LOG(x) x
//#define logerror printf

//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

const device_type PIT68230 = &device_creator<pit68230_device>;

//-------------------------------------------------
//  pit68230_device - constructors
//-------------------------------------------------
pit68230_device::pit68230_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: device_t (mconfig, type, name, tag, owner, clock, shortname, source),
	  device_execute_interface (mconfig, *this)
	, m_icount (0)
	, m_pa_out_cb(*this)
	, m_pa_in_cb(*this)
	, m_pb_out_cb(*this)
	, m_pb_in_cb(*this)
	, m_pc_out_cb(*this)
	, m_pc_in_cb(*this)
	, m_h1_out_cb (*this)
	, m_h2_out_cb (*this)
	, m_h3_out_cb (*this)
	, m_h4_out_cb (*this)
	, m_pgcr(0)
	, m_psrr(0)
	, m_paddr(0)
	, m_pbddr(0)
	, m_pcddr(0)
	, m_pacr(0)
	, m_pbcr(0)
	, m_padr(0)
	, m_pbdr(0)
	, m_psr(0)
	, m_tcr(0)
	, m_cpr(0)
	//	, m_cprh(0)
	//	, m_cprm(0)
	//	, m_cprl(0)
	, m_cntr(0)
{
}


pit68230_device::pit68230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t (mconfig, PIT68230, "PIT68230", tag, owner, clock, "pit68230", __FILE__),
	  device_execute_interface (mconfig, *this)
	, m_icount (0)
	, m_pa_out_cb (*this)
	, m_pa_in_cb(*this)
	, m_pb_out_cb(*this)
	, m_pb_in_cb(*this)
	, m_pc_out_cb(*this)
	, m_pc_in_cb(*this)
	, m_h1_out_cb(*this)
	, m_h2_out_cb(*this)
	, m_h3_out_cb(*this)
	, m_h4_out_cb(*this)
	, m_pgcr(0)
	, m_psrr(0)
	, m_paddr(0)
	, m_pbddr(0)
	, m_pcddr(0)
	, m_pacr(0)
	, m_pbcr(0)
	, m_padr(0)
	, m_pbdr(0)
	, m_psr(0)
	, m_tcr(0)
	, m_cpr(0)
	//	, m_cprh(0)
	//	, m_cprm(0)
	//	, m_cprl(0)
	, m_cntr(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void pit68230_device::device_start ()
{
	LOG(("%s\n", FUNCNAME));
	m_icountptr = &m_icount;

	// resolve callbacks
	m_pa_out_cb.resolve_safe();
	m_pa_in_cb.resolve_safe(0);
	m_pb_out_cb.resolve_safe();
	m_pb_in_cb.resolve_safe(0);
	m_h1_out_cb.resolve_safe();
	m_h2_out_cb.resolve_safe();
	m_h3_out_cb.resolve_safe();
	m_h4_out_cb.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void pit68230_device::device_reset ()
{
	LOG(("%s %s \n",tag(), FUNCNAME));

	m_pgcr = 0;
	m_psrr = 0;
	m_paddr = 0;
	m_pbddr = 0;
	m_pcddr = 0;
	m_pacr = 0; m_h2_out_cb(m_pacr);
	m_pbcr = 0;
	m_padr = 0; m_pa_out_cb((offs_t)0, m_padr); // TODO: check PADDR
	m_pbdr = 0;
	m_psr = 0;
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
void pit68230_device::device_timer (emu_timer &timer, device_timer_id id, INT32 param, void *ptr)
{
}

void pit68230_device::h1_set (UINT8 state)
{
	LOG(("%s %s %d @ m_psr %2x => ",tag(), FUNCNAME, state, m_psr));
	if (state) m_psr |= 1; else m_psr &= ~1;
	LOG(("%02x %lld\n", m_psr, machine ().firstcpu->total_cycles ()));
}

void pit68230_device::portb_setbit (UINT8 bit, UINT8 state)
{
	LOG(("%s %s %d/%d @ m_pbdr %2x => ", tag(), FUNCNAME, bit, state, m_pbdr));
	if (state) m_pbdr |= (1 << bit); else m_pbdr &= ~(1 << bit);
	LOG(("%02x %lld\n", m_pbdr, machine ().firstcpu->total_cycles ()));
}

//-------------------------------------------------
//  execute_run -
//-------------------------------------------------
void pit68230_device::execute_run ()
{
		do {
				synchronize ();

				m_icount--;
		} while (m_icount > 0);
}

#if VERBOSE > 2
static INT32 ow_cnt = 0;
static INT32 ow_data = 0;
static INT32 ow_ofs = 0;
#endif

void pit68230_device::wr_pitreg_pgcr(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
	m_pgcr = data;
}

void pit68230_device::wr_pitreg_psrr(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
	m_psrr = data;
}

void pit68230_device::wr_pitreg_paddr(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
	m_paddr = data;
}

void pit68230_device::wr_pitreg_pbddr(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
	m_pbddr = data;
}

void pit68230_device::wr_pitreg_pcddr(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
	m_pcddr = data;
}

void pit68230_device::wr_pitreg_pacr(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
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
	m_h2_out_cb (m_pacr & 0x08 ? 1 : 0); // TODO: Check mode and submodes
}

void pit68230_device::wr_pitreg_pbcr(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
	m_pbcr = data;
}

void pit68230_device::wr_pitreg_padr(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
	m_padr = data;
	// callbacks
	m_pa_out_cb ((offs_t)0, m_padr); // TODO: check PADDR
}

void pit68230_device::wr_pitreg_psr(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
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
 0 0 X	The dual-function pins PC3/TOUT and PC7/TIACK carry the port C function.
 0 1 X	The dual-function pinPC3/TOUT carries the TOUT function. In the run state it is used as a squarewave
		output and is toggled on zero detect. The TOUT pin is high while in the halt state. The dualfunction
		pin PC7/TIACK carries the PC7 function.
 1 0 0	The dual-function pin PC3/TOUT carries the TOUT function. In the run or halt state it is used as
		a timer interrupt request output. The timer interrupt is disabled, thus, the pin is always three stated.
		The dual-function pin PC7/TIACK carries the TIACK function ; however, since interrupt request is
		negated, the PI/T produces no response (i.e., no data or DTACK) to an asserted TIACK. Refer to
		5.1.3. Timer Interrupt Acknowledge Cycles for details.
 1 0 1	The dual-function pin PC3/TOUT carries the TOUTfunction and is used as a timer interrupt request
		output. The timer interrupt is enabled ; thus, the pin is low when the timer ZDS status bit is one.
		The dual-function pin PC7/TIACK carries the TIACK function and is used as a timer interrupt acknowledge
		input. Refer to the5.1.3. Timer InterruptAcknowledge Cycles fordetails. Thiscombination
		supports vectored timer interrupts.
 1 1 0	The dual-function pin PC3/TOUT function. In the run or halt state it is used as a timer interrupt
		request output. The timer interrupt is disabled ; thus, the pin is always three-stated. The dual-function
		pin PC7/TIACK carries the PC7 function.
 1 1 1	The dual-function pin PC3/TOUT carries the TOUTfunction and is used as a timer interrupt request
		output. The timer interrupt is enabled ; thus, the pin is low when the timer ZDS status bit is one.
		The dual-function pin PC7/TIACK carries the PC7 function and autovectored interrupts are supported.

TCR bit 4 - Zero Detect Control
	 0	The counter is loaded fromthe counter preload register on the first clock to the 24-bit counter after
		zero detect, then resumes counting.
	 1	The counter rolls over on zero detect, then continues counting.

TCR bit 3 - Unused and is always read as zero.

TCR bits
   2 1  Clock Control
   0 0  The PC2/TIN input pin carries the port C function, and the CLK pin and prescaler are used. The
		prescaler is decremented on the falling transition of the CLKpin ; the 24-bit counter is decremented,
		rolls over, or is loaded from the counter preload registers when the prescaler rolls over from $OO
		to $1F. The timer enable bit determines whether the timer is in the run or halt state.
   0 1	The PC2/TIN pin serves as a timer input, and the CLK pin and prescaler are used. The prescaler
		is decremented on the falling transition of the CLK pin ; the 24-bit counter is decremented, rolls
		over, or is loaded from the counter preload registers when the prescaler rolls over from $00 to $1F.
		The timer is in the run state when the timer enable bit is one and the TIN pin is high ; otherwise,
		the timer is in the halt state.
   1 0	The PC2/TIN pin serves as a timer input and the prescaler is used. The prescaler is decremented
		following the rising transition of the TIN pin after being synchronized with the internal clock. The
		24-bit counter is decremented, rolls over, or is loaded from the counter preload registers when the
		prescaler rolls over from $00 to $1F. The timer enable bit determines whether the timer is in the
		run or halt state.
   1 1	The PC2/TIN pin serves as a timer input and the prescaler is not used. The 24-bit counter is decremented,
		rolls over, or is loaded from the counter preload registers following the rising edge of
		the TIN pin after being synchronized with the internal clock. The timer enable bit determines whether
		the timer is in the run or halt state.
TCR bit 0 - Timer Enable
	 0	Disabled
	 1	Enabled
*/
void pit68230_device::wr_pitreg_tcr(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x Timer %s\n", 
		 FUNCNAME, data, m_owner->tag(), FUNCNAME, data, data & REG_TCR_ENABLE ? "enabled" : "disabled"));
	m_tcr = data;
}

void pit68230_device::wr_pitreg_cprh(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
	m_cpr &= ~0xff0000;
	m_cpr |= ((data << 16) & 0xff0000);
	//	m_cprh = data;
}

void pit68230_device::wr_pitreg_cprm(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
	m_cpr &= ~0x00ff00;
	m_cpr |= ((data << 8) & 0x00ff00);
	//	m_cprm = data;
}

void pit68230_device::wr_pitreg_cprl(UINT8 data)
{
	LOG(("%s(%02x) \"%s\": %s - %02x\n", FUNCNAME, data, m_owner->tag(), FUNCNAME, data));
	m_cpr &= ~0x0000ff;
	m_cpr |= ((data << 0) & 0x0000ff);
	//	m_cprl = data;
}

WRITE8_MEMBER (pit68230_device::write)
{
	LOG(("%s %s \n",tag(), FUNCNAME));
	switch (offset) {
	case PIT_68230_PGCR:	wr_pitreg_pgcr(data); break;
	case PIT_68230_PSRR:	wr_pitreg_psrr(data); break;
	case PIT_68230_PADDR:	wr_pitreg_paddr(data); break;
	case PIT_68230_PBDDR:	wr_pitreg_pbddr(data); break;
	case PIT_68230_PCDDR:	wr_pitreg_pcddr(data); break;
	case PIT_68230_PACR:	wr_pitreg_pacr(data); break;
	case PIT_68230_PBCR:	wr_pitreg_pbcr(data); break;
	case PIT_68230_PADR:	wr_pitreg_padr(data); break;
	case PIT_68230_PAAR:	break;	// RO register so ignored
	case PIT_68230_PBAR:	break;	// RO register so ignored
	case PIT_68230_PSR:		wr_pitreg_psr(data); break;
	case PIT_68230_TCR:		wr_pitreg_tcr(data); break;
	case PIT_68230_CPRH:	wr_pitreg_cprh(data); break;
	case PIT_68230_CPRM:	wr_pitreg_cprm(data); break;
	case PIT_68230_CPRL:	wr_pitreg_cprl(data); break;
	default:
		LOG (("Unhandled Write of %02x to register %02x", data, offset));
	}

#if VERBOSE > 2
	if (offset != ow_ofs || data != ow_data || ow_cnt >= 1000) {
		logerror ("\npit68230_device::write: previous identical operation performed %02x times\n", ow_cnt);
		ow_cnt = 0;
		ow_data = data;
		ow_ofs = offset;
		logerror ("pit68230_device::write: offset=%02x data=%02x %lld\n", ow_ofs, ow_data, machine ().firstcpu->total_cycles ());
	}
	else
		ow_cnt++;
#endif

}

#if VERBOSE > 2
static INT32 or_cnt = 0;
static INT32 or_data = 0;
static INT32 or_ofs = 0;
#endif

UINT8 pit68230_device::rr_pitreg_pgcr()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, m_pgcr));
	return m_pgcr;
}

UINT8 pit68230_device::rr_pitreg_psrr()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, m_psrr));
	return m_psrr;
}

UINT8 pit68230_device::rr_pitreg_paddr()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, m_paddr));
	return m_paddr;
}

UINT8 pit68230_device::rr_pitreg_pbddr()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, m_pbddr));
	return m_pbddr;
}

UINT8 pit68230_device::rr_pitreg_pcddr()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, m_pcddr));
	return m_pcddr;
}

UINT8 pit68230_device::rr_pitreg_pacr()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, m_pacr));
	return m_pacr;
}

UINT8 pit68230_device::rr_pitreg_pbcr()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, m_pbcr));
	return m_pbcr;
}

UINT8 pit68230_device::rr_pitreg_padr()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, m_padr));
	return m_padr;
}

/* 4.6.2. PORT B DATA REGISTER (PBDR). The port B data register is a holding
 * register for moving data to and from port B pins. The port B data direction
 * register determines whether each pin is an input (zero) or an output (one).
 * This register is readable and writable at all times. Depending on the chosen
 * mode/submode, reading or writing may affect the double-buffered handshake
 * mechanism. The port B data register is not affected by the assertion of the
 * RESET pin. PB0-PB7 sits on pins 17-24 on a 48 pin DIP package */
UINT8 pit68230_device::rr_pitreg_pbdr()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, m_pbdr));
	return m_pbdr;
}

/* The port A alternate register is an alternate register for reading the port A pins. 
It is a read-only address and no other PI/T condition is affected. In all modes,
the instantaneous pin level is read and no input latching is performed except at the 
data bus interface. Writes to this address are answered with DTACK, but the data is ignored.*/
UINT8 pit68230_device::rr_pitreg_paar()
{
	// NOTE: no side effect emulated so using ..padr 
	UINT8 ret;
	ret = m_pa_in_cb();
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, ret));
	return ret;
}

/* The port B alternate register is an alternate register for reading the port B pins. 
It is a read-only address and no other PI/T condition is affected. In all modes,
the instantaneous pin level is read and no input latching is performed except at the 
data bus interface.Writes to this address are answered with DTACK, but the data is ignored.*/
UINT8 pit68230_device::rr_pitreg_pbar()
{
	// NOTE: no side effect emulated so using ..pbdr
	UINT8 ret;
	ret = m_pb_in_cb();
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, ret));
	return ret;
}

/* 4.8. PORT STATUS REGISTER (PSR) The port status register contains information about
 * handshake pin activity. Bits 7-4 show the instantaneous level of the respective handshake
 * pin, and are independent of the handshake pin sense bits in the port general control
 * register. Bits 3-0 are the respective status bits referred to throughout this document.
 * Their interpretation depends on the programmed mode/submode of the PI/T. For bits
 * 3-0 a one is the active or asserted state. */
UINT8 pit68230_device::rr_pitreg_psr()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, m_psr));
	return m_psr;
}

UINT8 pit68230_device::rr_pitreg_cntrh()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, (m_cntr >> 16) & 0xff));
	return (m_cntr >> 16) & 0xff;
}

UINT8 pit68230_device::rr_pitreg_cntrm()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, (m_cntr >> 8) & 0xff));
	return (m_cntr >> 8) & 0xff;
}

UINT8 pit68230_device::rr_pitreg_cntrl()
{
	LOGR(("%s %s <- %02x\n",tag(), FUNCNAME, (m_cntr >> 0) & 0xff));
	return (m_cntr >> 0) & 0xff;
}

READ8_MEMBER (pit68230_device::read){
	UINT8 data;

	switch (offset) {
	case PIT_68230_PGCR:	data = rr_pitreg_pgcr(); break;
	case PIT_68230_PSRR:	data = rr_pitreg_psrr(); break;
	case PIT_68230_PADDR:	data = rr_pitreg_paddr(); break;
	case PIT_68230_PBDDR:	data = rr_pitreg_pbddr(); break;
	case PIT_68230_PCDDR:	data = rr_pitreg_pcddr(); break;
	case PIT_68230_PACR:	data = rr_pitreg_pacr(); break;
	case PIT_68230_PBCR:	data = rr_pitreg_pbcr(); break;
	case PIT_68230_PADR:	data = rr_pitreg_padr(); break;
	case PIT_68230_PBDR:	data = rr_pitreg_pbdr(); break;
	case PIT_68230_PAAR:	data = rr_pitreg_paar(); break;
	case PIT_68230_PBAR:	data = rr_pitreg_pbar(); break;
	case PIT_68230_PSR:		data = rr_pitreg_psr(); break;
	case PIT_68230_CNTRH:	data = rr_pitreg_cntrh(); break;
	case PIT_68230_CNTRM:	data = rr_pitreg_cntrm(); break;
	case PIT_68230_CNTRL:	data = rr_pitreg_cntrl(); break;
	default:
		LOG (("Unhandled read register %02x\n", offset));
		data = 0;
	}

#if VERBOSE > 2
	if (offset != or_ofs || data != or_data || or_cnt >= 1000) {
		logerror ("\npit68230_device::read: previous identical operation performed %02x times\n", or_cnt);
		or_cnt = 0;
		or_data = data;
		or_ofs = offset;
		logerror ("pit68230_device::read: offset=%02x data=%02x %lld\n", or_ofs, or_data, machine ().firstcpu->total_cycles ());
	}
	else
		or_cnt++;
#endif

	return data;
}
