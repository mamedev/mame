// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*

    Intelligent peripheral bus controller (IBC)

    This is a simple circuit used to connect to the Hexbus infrastructure.

    Two variants are known, with the only difference being the voltage pins
    and a split of incoming/outgoing lines from/to the Hexbus.

    The 22-pin version is the earlier one, but it is used in the HX5102
    floppy drive.

                  22-pin                         28-pin
               +-----------+                  +-----------+
        I/O-0  | 1   \/  22|  O-2      I/O-0  | 1   \/  28|  Vcc
        I/O-1  | 2       21|  O-1      I/O-1  | 2       27|  O-0
        I/O-2  | 3       20|  O-0      I/O-2  | 3       26|  O-1
        I/O-3  | 4       19|  Vdd      I/O-3  | 4       25|  O-2
          BAV  | 5       18|  MS        BAVI  | 5       24|  O-3,R/W
          HSK  | 6       17|  D-3        BAV  | 6       23|  D-3I
          IRQ  | 7       16|  D-2       HSKI  | 7       22|  D-3
            E  | 8       15|  D-1        HSK  | 8       21|  D-2I
          Vss  | 9       14|  D-0        IRQ  | 9       20|  D-2
          RES  |10       13|  RS           E  |10       19|  D-1I
      R/W,O-3  |11       12|  CS          CS  |11       18|  D-1
               +-----------+             RES  |12       17|  D-0I
                  1052911                 MS  |13       16|  D-0
                                         GND  |14       15|  RS
                                              +-----------+
                                                  TP0370

        Pins   Dir  Meaning (MS=0)          Dir Meaning (MS=1)
        -------------------------------------------------------------------
        I/O-0  i/o  Data lines (Host)       in  Data lines (Host)
        I/O-1  i/o  Data lines (Host)       in  Data lines (Host)
        I/O-2  i/o  Data lines (Host)       in  Data lines (Host)
        I/O-3  i/o  Data lines (Host)       in  Data lines (Host)
          BAV  i/o  Bus available (Hexbus)  i/o Bus available (Hexbus)
          HSK  i/o  Handshake line (Hexbus) i/o Handshake line (Hexbus)
          IRQ  out  Interrupt               out Interrupt
            E  in   See below               in  Write strobe
           MS  in   Mode select=0           in  Mode select=1
           RS  in   Register select         in  Register select
           CS  in   Chip select             in  Chip select
          RES  in   Reset                   in  Reset
          O-0  in   Option select           out Data output (Host)
          O-1  in   Option select           out Data output (Host)
          O-2  in   Option select           out Data output (Host)
          R/W  in   Read/write select       out Data output (Host, O-3)
          D-0  i/o  Data lines (Hexbus)     i/o  Data lines (Hexbus)
          D-1  i/o  Data lines (Hexbus)     i/o  Data lines (Hexbus)
          D-2  i/o  Data lines (Hexbus)     i/o  Data lines (Hexbus)
          D-3  i/o  Data lines (Hexbus)     i/o  Data lines (Hexbus)
          Vss
          Vdd
          Vcc  +5V
          GND  0V

          Options for MS=0
          O-2 O-1 O-0
          -----------
           0   0   0         TMS7000 mode (E active low)
           0   0   1          6500 mode (E active high, using R/W)
           0   1   0          8048/8085 mode (WR*->R/W, RD*->E)
           1   *   *          Multiplexed A/D Buses (ALATCH/ALE->RS)

           MS=1 is used for 4-bit MPUs which do not have a bidirectional bus
           (e.g. TMS1000).


         All registers are 4-bit wide

         RS   Dir    Access
         ---------------------------------
          0   Read   Receive data register
          0   Write  Transmit data register
          1   Read   Status register
          1   Write  Control register

         Control register
          MSB 3   DISABLE: 1->Disable clearing of Inhibit by new BAV
              2   INHIBIT: 1->Inhibit latching of HSK and IRQ until new BAV
              1   BAV: 1-> Assert BAV* line; 0->Release BAV* when HSK* low
          LSB 0   HSK: 1-> Assert HSK* line, clear IRQ; 0->Release HSK*

          Special: 1100 -> Hardware reset


         Status register
          MSB 3   \_ 00 = Enable state; 01 = Inhibit/disable state;
              2   /  10 = Active IRQ; 11 = Start of message
              1   BAV* asserted (1=low)
          LSB 0   HSK* asserted (1=low)

          Special: 01** after hardware reset

    [1] Texas Instruments Consumer Products Group: Hexbus(TM) specifications,
        Sep 1983
*/

#include "emu.h"
#include "hexbus.h"

// Devices
#include "tp0370.h"

#define LOG_DATA        (1U<<1)   // Data transfer
#define LOG_DETAIL      (1U<<2)
#define LOG_WRITE       (1U<<3)
#define LOG_LINES       (1U<<4)
#define LOG_STATUS      (1U<<5)
#define LOG_MODE        (1U<<6)

// Minimum log should be config and warnings
#define VERBOSE ( LOG_GENERAL )

enum
{
	CTRL_DISABLE=0x80,
	CTRL_INHIBIT=0x40,
	CTRL_BAV=0x20,
	CTRL_HSK=0x10
};


#include "logmacro.h"

// Hexbus instance
DEFINE_DEVICE_TYPE_NS(IBC, bus::hexbus, ibc_device,  "ibc",  "Intelligent Peripheral Bus Controller")

namespace bus { namespace hexbus {

ibc_device::ibc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IBC, tag, owner, clock),
	m_int(*this),
	m_hexout(*this),
	m_latch(*this),
	m_inhibit(false),
	m_disable(true),
	m_bavold(true),
	m_hskold(true),
	m_int_pending(false),
	m_incoming_message(false),
	m_message_started(false),
	m_latch_inhibit(false),
	m_data(0),
	m_transmit(0xff)
{
}

/*
    Reading from host
*/
READ8_MEMBER( ibc_device::read )
{
	uint8_t status = 0;
	switch (offset)
	{
	case 0:
		LOGMASKED(LOG_DATA, "Data reg -> %01x\n", m_data&0x0f);
		return (m_data<<4)&0xf0;  // returned in the first 4 bits
	case 1:
		if (m_message_started)
			status |= 0xc0;
		else
		{
			if (m_int_pending)
				status |= 0x80;
			else
			{
				if (m_disable || m_inhibit)
					status |= 0x40;
			}
		}
		if (m_bav) status |= 0x20;
		if (m_hsk) status |= 0x10;

		LOGMASKED(LOG_STATUS, "Status -> %02x\n", status);

		// Reset flag
		m_message_started = false;
		return status;

	default:
		LOG("Unknown read address: %02x\n", offset);
		break;
	}
	return 0;
}

/*
    Writing from host
*/
WRITE8_MEMBER( ibc_device::write )
{
	switch (offset)
	{
	case 0:
		LOGMASKED(LOG_WRITE, "Writing transmit data %02x\n", data);
		m_transmit = data;
		break;
	case 1:
		set_disable_inhibit((data & CTRL_DISABLE)!=0, (data & CTRL_INHIBIT)!=0);
		if ((data & CTRL_HSK)!=0 && m_int_pending)
		{
			// Reset INT
			LOGMASKED(LOG_DETAIL, "Reset Interrupt\n");
			m_int_pending = false;
			m_int(CLEAR_LINE);
		}
		else
		{
			set_lines((data & CTRL_BAV)!=0, (data & CTRL_HSK)!=0);
		}

		break;
	default:
		LOG("Unknown write address: %02x\n", offset);
		break;
	}
}

void ibc_device::set_disable_inhibit(bool dis, bool inh)
{
	LOGMASKED(LOG_MODE, "Setting disable=%s, inhibit=%s\n", dis? "on" : "off", inh? "on" : "off");
	m_disable = dis;
	m_inhibit = inh;
}

/*
    Called by a command, not automatically.
*/
void ibc_device::set_lines(bool bav, bool hsk)
{
	LOGMASKED(LOG_LINES, "%s BAV*, %s HSK*\n", bav? "Pull down" : "Release",  hsk? "Pull down" : "Release");

	// We're in the response phase.
	if (hsk) m_latch_inhibit = true;

	// Assert HSK*  (110 0 0111)
	// Release HSK* (110 1 0111)
	// Assert BAV*  (11010 0 11)
	// Release BAv* (11010 1 11)
	uint8_t val = (m_transmit & 0xc0)|((m_transmit & 0x30)>>4);
	if (!bav) val |= 0x04;
	if (!hsk) val |= 0x10;
	if (m_transmit != 0xff) LOGMASKED(LOG_LINES, "Data = %01x\n", m_transmit>>4);

	m_hexout(val);
}

/*
    Line state received via the Hexbus
    +------+------+------+------+------+------+------+------+
    | ADB3 | ADB2 |  -   | HSK* |  0   | BAV* | ADB1 | ADB0 |
    +------+------+------+------+------+------+------+------+
*/
void ibc_device::from_hexbus(uint8_t val)
{
	uint8_t data = ((val >> 4)&0x0c) | (val & 0x03);

	m_bavold = m_bav;
	m_hskold = m_hsk;

	m_hsk = ((val & 0x10)==0);
	m_bav = ((val & 0x04)==0);
	LOGMASKED(LOG_LINES, "Hexbus -> BAV*=%x, HSK*=%x, HSKold*=%x, DATA=%1x\n", m_bav? 0:1, m_hsk? 0:1, m_hskold? 0:1, data);

	if (m_disable)
	{
		// When disabled, do not latch HSK
		LOGMASKED(LOG_MODE, "Disabled, not latching HSK*\n");
	}
	else
	{
		// Falling edge of BAV*
		// Reset inhibit flag
		if (m_bav && !m_bavold)
		{
			LOGMASKED(LOG_LINES, "Bus acquired\n");
			m_inhibit = false;
			m_incoming_message = true;
		}
		else
		{
			if (!m_bav && m_bavold)
			{
				LOGMASKED(LOG_LINES, "Bus released\n");
				m_latch_inhibit = false;
			}
		}

		// The message may combine a change of BAV* and of HSK*.
		if (!m_inhibit)
		{
			// Falling edge of HSK*
			if (m_hsk && !m_hskold)
			{
				if (m_latch_inhibit)
				{
					LOGMASKED(LOG_LINES, "Not latching HSK* in response phase\n");
				}
				else
				{
					// On this falling edge, the nibble is supposed to be stable,
					// so keep it
					m_data = data;
					if (m_incoming_message && !m_message_started)
					{
						// Set flag for new message
						m_incoming_message = false;
						m_message_started = true;
						LOGMASKED(LOG_DETAIL, "New message started\n", data);
					}
					else
						m_message_started = false;

					LOGMASKED(LOG_DETAIL, "Data reg <- %1x\n", data);

					// set the latch
					m_latch(ASSERT_LINE);

					// and set interrupt
					m_int_pending = true;
					m_int(ASSERT_LINE);
				}
			}
		}
	}
}

/*
    Called from the hexbus_chained_device.
*/
void ibc_device::update_lines(bool bav, bool hsk)
{
	LOGMASKED(LOG_LINES, "Actual Hexbus line states: BAV*=%d, HSK*=%d\n", bav? 0:1, hsk? 0:1);
	m_bav = bav;
	m_hsk = hsk;
}

void ibc_device::device_start()
{
	m_int.resolve_safe();
	m_hexout.resolve_safe();
	m_latch.resolve_safe();
}

void ibc_device::device_reset()
{
	m_inhibit = true;
	m_disable = true;
}

}   }

