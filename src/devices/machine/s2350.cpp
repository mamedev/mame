// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  American Microsystems, Inc. (AMI) S2350 Universal Synchronous Receiver/Transmitter(USRT)

  Chip Features
    - 500kHz Data Rates
    - Internal Sync Detection
    - Fill Character Register
    - Double Buffered Input/Output
    - Bus Oriented Outputs
    - 5-8 Bit Characters
    - Odd/Even or No Parity
    - Error Status Flags


  Current assumptions:
    - Data size == 8
    - Parity == NONE

  Not Implemented
    - parity
    - data size (other than 8)

 **********************************************************************
                            _____   _____
                   GND   1 |*    \_/     | 40  NDB2
                   Vcc   2 |             | 39  NDB1
                   NPB   3 |             | 38  TDS
                   POE   4 |             | 37  RCP
                    CS   5 |             | 36  TCP
                   TSO   6 |             | 35  RDE
                   FCT   7 |             | 34  SWE
                   SCR   8 |             | 33  RD0
                  TBMT   9 |     AMI     | 32  RD1
                   RPE  10 |    S2350    | 31  RD2
                   ROR  11 |             | 30  RD3
                   RDA  12 |             | 29  RD4
                    RR  13 |             | 28  RD5
                 RESET  14 |             | 27  RD6
                    D0  15 |             | 26  RD7
                    D1  16 |             | 25  RSI
                    D2  17 |             | 24  TFS
                    D3  18 |             | 23  RSS
                    D4  19 |             | 22  D7
                    D5  20 |_____________| 21  D6


Name             Pin No.     Function
----------------------------------------------------------------------

D0-D7            15-22       Data Inputs

RD7-RD70         26-33       Received Data Outputs

TDS               38         Transmit Data Strobe

TFS               24         Transmit Fill Strobe

RSS               23         Receiver Sync Strobe

TBMT               9         Transmit Buffer Empty

TSO                6         Transmitter Serial Output

TCP               36         Transmit Clock

RDE               35         Receive Data Enable

FCT                7         Fill Character Transmitted

RSI               25         Receiver Serial Input

RCP               37         Receiver Clock

RDA               12         Received Data Available

SCR                8         Sync Character Received

SWE               34         Status Word Enable

ROR               11         Receiver Overrun

RPE               10         Receiver Parity Error

RR                13         Receiver Restart

NDB1/NDB2        39-40       Number Data Bits

NPB                3         No Parity Bit

POE                4         Parity Odd/Even

CS                 5         Control Strobe

RESET             14         Master Reset

Vcc                2         +5.0V

GND                1         Ground
***************************************************************************/


#include "emu.h"
#include "s2350.h"

// Shows register setup
#define LOG_REG (1U << 1)
// Show control lines
#define LOG_LINE (1U << 2)
// Function calls
#define LOG_FUNC (1U << 3)
// Sync detect
#define LOG_SYNC (1U << 4)

//#define VERBOSE (0xff)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGLINE(...)       LOGMASKED(LOG_LINE, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)
#define LOGSYNC(...)       LOGMASKED(LOG_SYNC, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


DEFINE_DEVICE_TYPE(S2350, s2350_device, "s2350", "American Microsystems Inc. S2350 USRT")


s2350_device::s2350_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, S2350, tag, owner, clock),
	m_tbmt(*this),
	m_fct(*this),
	m_tx_cb(*this),
	m_rda(*this),
	m_scr(*this),
	m_ror(*this),
	m_rpe(*this)
{
}

s2350_device::~s2350_device()
{
}

void s2350_device::device_start()
{
	save_item(NAME(m_transmit_buffer_empty));
	save_item(NAME(m_fill_char_transmitted));
	save_item(NAME(m_transmitter_holding_reg));
	save_item(NAME(m_transmitter_fill_reg));
	save_item(NAME(m_transmitter_shift_reg));
	save_item(NAME(m_serial_tx_state));

	save_item(NAME(m_received_data_available));
	save_item(NAME(m_receiver_overrun));
	save_item(NAME(m_receiver_parity_error));
	save_item(NAME(m_sync_character_received));

	save_item(NAME(m_receiver_output_reg));
	save_item(NAME(m_receiver_sync_reg));
	save_item(NAME(m_receiver_shift_reg));
	save_item(NAME(m_serial_rx_state));

	save_item(NAME(m_in_sync));
}

void s2350_device::device_reset()
{
	// transmitter flags
	set_transmit_buffer_empty(true);
	set_fill_char_transmitted(false);

	// transmitter registers
	m_transmitter_shift_reg = 0xff;
	m_transmitter_holding_reg = 0xff;
	m_transmitter_fill_reg = 0x00;

	m_serial_tx_state = 0;

	set_received_data_available(false);
	set_receiver_overrun(false);
	set_receiver_parity_error(false);
	set_sync_character_received(false);

	m_sync_search_active = false;
	m_in_sync = false;

	m_serial_rx_state = 0;

	// receiver registers
	m_receiver_output_reg = 0x00;
	m_receiver_sync_reg = 0x00;
	m_receiver_shift_reg = 0x00;
}

void s2350_device::transmitter_holding_reg_w(u8 data)
{
	LOGREG("%s: 0x%02x\n", FUNCNAME, data);

	m_transmitter_holding_reg = data;
	set_transmit_buffer_empty(false);
}

void s2350_device::transmit_fill_reg_w(u8 data)
{
	LOGREG("%s: 0x%02x\n", FUNCNAME, data);

	m_transmitter_fill_reg = data;
}

void s2350_device::receiver_sync_reg_w(u8 data)
{
	LOGREG("%s: 0x%02x\n", FUNCNAME, data);

	m_receiver_sync_reg = data;
}

u8 s2350_device::receiver_sync_search()
{
	set_sync_character_received(false);
	m_sync_search_active = true;

	LOGREG("%s: 0x%02x\n", FUNCNAME, m_receiver_sync_reg);

	return m_receiver_sync_reg;
}

u8 s2350_device::receiver_output_reg_r()
{
	set_received_data_available(false);

	LOGREG("%s: 0x%02x\n", FUNCNAME, m_receiver_output_reg);

	return m_receiver_output_reg;
}

u8 s2350_device::status_word_r()
{
	u8 status = 0x00;

	status |= m_received_data_available ? 0x01 : 0x00;
	status |= m_receiver_overrun        ? 0x02 : 0x00;
	status |= m_receiver_parity_error   ? 0x04 : 0x00;
	status |= m_sync_character_received ? 0x08 : 0x00;
	status |= m_fill_char_transmitted   ? 0x40 : 0x00;
	status |= m_transmit_buffer_empty   ? 0x80 : 0x00;

	if (!machine().side_effects_disabled())
	{
		set_fill_char_transmitted(false);
		set_receiver_overrun(false);
		set_receiver_parity_error(false);
		set_sync_character_received(false);
	}

	LOGREG("%s: 0x%02x\n", FUNCNAME, status);

	return status;
}

void s2350_device::rx_w(int state)
{
	m_serial_rx_line = state;
}

void s2350_device::receiver_restart()
{
	LOGFUNC("%s\n", FUNCNAME);

	set_received_data_available(false);
	set_sync_character_received(false);
	set_receiver_overrun(false);
	set_receiver_parity_error(false);

	m_sync_search_active = true;
	m_serial_rx_state = 0;
	m_serial_tx_state = 0;
}

void s2350_device::receive_byte(u8 data)
{
	if (m_received_data_available)
	{
		set_receiver_overrun(true);
	}

	m_receiver_output_reg = data;
	set_received_data_available(true);
}

void s2350_device::set_transmit_buffer_empty(bool val)
{
	LOGLINE("%s - val: %d\n", FUNCNAME, val);

	m_transmit_buffer_empty = val;

	m_tbmt(val);
}

void s2350_device::set_fill_char_transmitted(bool val)
{
	LOGLINE("%s - val: %d\n", FUNCNAME, val);

	m_fill_char_transmitted = val;

	m_fct(val);
}

void s2350_device::set_received_data_available(bool val)
{
	LOGLINE("%s - val: %d\n", FUNCNAME, val);

	m_received_data_available = val;

	m_rda(val);
}

void s2350_device::set_receiver_overrun(bool val)
{
	LOGLINE("%s - val: %d\n", FUNCNAME, val);

	m_receiver_overrun = val;

	m_ror(val);
}

void s2350_device::set_receiver_parity_error(bool val)
{
	LOGLINE("%s - val: %d\n", FUNCNAME, val);

	m_receiver_parity_error = val;

	m_rpe(val);
}

void s2350_device::set_sync_character_received(bool val)
{
	LOGLINE("%s - val: %d\n", FUNCNAME, val);

	m_sync_character_received = val;

	m_scr(val);
}

void s2350_device::update_receiver_shift()
{
	m_receiver_sync_reg >>= 1;

	m_receiver_shift_reg |= m_serial_rx_line ? 0x80 : 0x00;
}

void s2350_device::rcp_w()
{
	if (m_in_sync)
	{
		if (m_serial_rx_state++ == 0)
		{
			m_receiver_shift_reg = 0;
		}

		update_receiver_shift();

		if (m_serial_rx_state == 8)
		{
			receive_byte(m_receiver_shift_reg);

			m_serial_rx_state = 0;
		}
	}
	else if (m_sync_search_active)
	{
		LOGSYNC("%s - searching for sync\n", FUNCNAME);

		update_receiver_shift();

		// check for a match
		if (m_receiver_sync_reg == m_receiver_shift_reg)
		{
			LOGSYNC("%s - sync found\n", FUNCNAME);

			m_serial_rx_line = 0;
			m_in_sync = true;
		}
	}
	else
	{
		LOGSYNC("%s - ignoring data\n", FUNCNAME);
	}
}

void s2350_device::tcp_w()
{
	if (m_serial_tx_state == 0)
	{
		if (m_transmit_buffer_empty)
		{
			set_fill_char_transmitted(true);
			m_transmitter_shift_reg = m_transmitter_fill_reg;
		}
		else
		{
			set_transmit_buffer_empty(false);
			set_fill_char_transmitted(false);
			m_transmitter_shift_reg = m_transmitter_holding_reg;
		}
	}

	m_tx_cb(BIT(m_transmitter_shift_reg, m_serial_tx_state));

	m_serial_tx_state = (m_serial_tx_state + 1) & 0x7;
}
