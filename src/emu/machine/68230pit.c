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
*  - Add all missing registers
*  - Add configuration
**********************************************************************/

#include "68230pit.h"

#define LOG(x) /* x */

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
        , m_write_pa (*this)
        , m_write_h2 (*this)
{
}


pit68230_device::pit68230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
        : device_t (mconfig, PIT68230, "PIT68230", tag, owner, clock, "pit68230", __FILE__),
        device_execute_interface (mconfig, *this)
        , m_icount (0)
        , m_write_pa (*this)
        , m_write_h2 (*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void pit68230_device::device_start ()
{
        LOG (logerror ("PIT68230 device started\n"));
        m_icountptr = &m_icount;

        // resolve callbacks
        m_write_pa.resolve_safe ();
        m_write_h2.resolve_safe ();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void pit68230_device::device_reset ()
{
        LOG (logerror ("PIT68230 device reseted\n"));
        m_pgcr = 0;
        m_psrr = 0;
        m_paddr = 0;
        m_pbddr = 0;
        m_pcddr = 0;
        m_pacr = 0; m_write_h2 (m_pacr);
        m_pbcr = 0;
        m_padr = 0; m_write_pa ((offs_t)0, m_padr); // TODO: check PADDR
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
        LOG (logerror ("h1_set %d @ m_psr %2x => ", state, m_psr));
        if (state) m_psr |= 1; else m_psr &= ~1;
        LOG (logerror ("%02x %lld\n", m_psr, machine ().firstcpu->total_cycles ()));
}

void pit68230_device::portb_setbit (UINT8 bit, UINT8 state)
{
        LOG (logerror ("portb_setbit %d/%d @ m_pbdr %2x => ", bit, state, m_pbdr));
        if (state) m_pbdr |= (1 << bit); else m_pbdr &= ~(1 << bit);
        LOG (logerror ("%02x %lld\n", m_pbdr, machine ().firstcpu->total_cycles ()));
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

LOG (static INT32 ow_cnt = 0);
LOG (static INT32 ow_data = 0);
LOG (static INT32 ow_ofs = 0);

WRITE8_MEMBER (pit68230_device::write){
        switch (offset) {
        case PIT_68230_PGCR:
                m_pgcr = data;
                break;

        case PIT_68230_PSRR:
                m_psrr = data;
                break;

        case PIT_68230_PADDR:
                m_paddr = data;
                break;

        case PIT_68230_PBDDR:
                m_pbddr = data;
                break;

        case PIT_68230_PCDDR:
                m_pcddr = data;
                break;

        case PIT_68230_PACR:
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
                m_write_h2 (m_pacr & 0x08 ? 1 : 0); // TODO: Check mode and submodes
                break;

        case PIT_68230_PBCR:
                m_pbcr = data;
                break;

        case PIT_68230_PADR:
                m_padr = data;
                // callbacks
                m_write_pa ((offs_t)0, m_padr); // TODO: check PADDR
                break;

        case PIT_68230_PSR:
                m_psr = data;
                break;

        default:
                LOG (logerror ("unhandled register %02x", offset));
        }

        LOG (if (offset != ow_ofs || data != ow_data || ow_cnt >= 1000) {
                logerror ("\npit68230_device::write: previous identical operation performed %02x times\n", ow_cnt);
                ow_cnt = 0;
                ow_data = data;
                ow_ofs = offset;
                logerror ("pit68230_device::write: offset=%02x data=%02x %lld\n", ow_ofs, ow_data, machine ().firstcpu->total_cycles ());
        }
                else
                        ow_cnt++; )
}

LOG (static INT32 or_cnt = 0);
LOG (static INT32 or_data = 0);
LOG (static INT32 or_ofs = 0);

READ8_MEMBER (pit68230_device::read){
        UINT8 data = 0;

        switch (offset) {
        case PIT_68230_PGCR:
                data = m_pgcr;
                break;

        case PIT_68230_PSRR:
                data = m_psrr;
                break;

        case PIT_68230_PADDR:
                data = m_paddr;
                break;

        case PIT_68230_PBDDR:
                data = m_pbddr;
                break;

        case PIT_68230_PCDDR:
                data = m_pcddr;
                break;

        case PIT_68230_PACR:
                data = m_pacr;
                break;

        case PIT_68230_PBCR:
                data = m_pbcr;
                break;

        case PIT_68230_PADR:
                data = m_padr;
                break;

        case PIT_68230_PBDR:
                /* 4.6.2. PORT B DATA REGISTER (PBDR). The port B data register is a holding
                 * register for moving data to and from port B pins. The port B data direction
                 * register determines whether each pin is an input (zero) or an output (one).
                 * This register is readable and writable at all times. Depending on the chosen
                 * mode/submode, reading or writing may affect the double-buffered handshake
                 * mechanism. The port B data register is not affected by the assertion of the
                 * RESET pin. PB0-PB7 sits on pins 17-24 on a 48 pin DIP package */
                data = m_pbdr;
                break;

        case PIT_68230_PSR:
                /* 4.8. PORT STATUS REGISTER (PSR) The port status register contains information about
                 * handshake pin activity. Bits 7-4 show the instantaneous level of the respective handshake
                 * pin, and are independent of the handshake pin sense bits in the port general control
                 * register. Bits 3-0 are the respective status bits referred to throughout this document.
                 * Their interpretation depends on the programmed mode/submode of the PI/T. For bits
                 * 3-0 a one is the active or asserted state. */
                data = m_psr;
                break;

        default:
                LOG (logerror ("unhandled register %02x", offset));
                data = 0;
        }

        LOG (if (offset != or_ofs || data != or_data || or_cnt >= 1000) {
                logerror ("\npit68230_device::read: previous identical operation performed %02x times\n", or_cnt);
                or_cnt = 0;
                or_data = data;
                or_ofs = offset;
                logerror ("pit68230_device::read: offset=%02x data=%02x %lld\n", or_ofs, or_data, machine ().firstcpu->total_cycles ());
        }
                else
                        or_cnt++; )

        return data;
}
