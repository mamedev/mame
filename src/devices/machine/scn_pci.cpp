// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Signetics SCN2651 Programmable Communications Interface (PCI)
    Signetics SCN2661A/B/C Enhanced Programmable Communications Interface
    Signetics SCN2641 Asynchronous Communications Interface

    These serial interface chips are loosely based on the Intel 8251
    Universal Synchronous/Asynchronous Receiver/Transmitter, though not
    software-compatible or plug-compatible with it. Signetics' PCI has
    several original features, including an optional internal baud rate
    generator, transparent synchronous mode and multiple loopback modes.
    The EPCI makes several further improvements to synchronous and
    asynchronous support, with the latter carried over to the
    asynchronous-only SCN2641.

    The baud rate generator is very similar to the COM8116 and its related
    family of devices, providing 16 fixed rates for a given input clock,
    with the available rates and standard clock varying between EPCI
    models. SCN2661C (also numbered 2661-3) provides the same divisors as
    SCN2651 and COM8116, but SCN2661A (2661-1) and SCN2661B (2661-2) take
    a different BRCLK and have unique divisors. SCN2641 is different again.

    There are four interrupt conditions, expressed as three status bits and
    three outputs (which the SCN2641 reduces to one): transmitter ready,
    receiver ready, transmitter empty and change in DSR/DCD state, the last
    (and only the last) of which is cleared when the status register is
    read. Unlike the corresponding outputs of the Intel 8251 USART, these
    are active-low, open-drain outputs, making them easily tied together
    (as the SCN2641 does internally).

    The polarity of the R/W bus control input is compatible with the
    Signetics 2650 microprocessor, but the opposite of most RAMs and many
    familiar microprocessors such as the Intel 8080A and Motorola MC68000,
    both of which Signetics incidentally second-sourced. (This is also
    true of the SCN68661/MC68661, which is nothing more than an alternate
    designation for the SCN2661.) Since this input requires the same setup
    requirement as the address lines, the PCI/EPCI's read and write
    registers are often mapped to separate addresses when placed on an
    Intel-type bus, like the MC6850 ACIA but with reads below writes.

    The clock outputs have not been tested. The documentation for these is
    not very descriptive, and the resynchronization of the RxC output in
    1X mode is more or less a conjecture.

***************************************************************************/

#include "emu.h"
#include "scn_pci.h"

#define LOG_INIT    (1 << 1U)
#define LOG_COMMAND (1 << 2U)
#define LOG_RCVR    (1 << 3U)
#define LOG_XMTR    (1 << 4U)
//#define VERBOSE (LOG_INIT | LOG_COMMAND | LOG_RCVR | LOG_XMTR)
#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SCN2651,  scn2651_device,  "scn2651",  "Signetics SCN2651 PCI")
DEFINE_DEVICE_TYPE(SCN2661A, scn2661a_device, "scn2661a", "Signetics SCN2661A EPCI")
DEFINE_DEVICE_TYPE(SCN2661B, scn2661b_device, "scn2661b", "Signetics SCN2661B EPCI")
DEFINE_DEVICE_TYPE(SCN2661C, scn2661c_device, "scn2661c", "Signetics SCN2661C EPCI")
DEFINE_DEVICE_TYPE(SCN2641,  scn2641_device,  "scn2641",  "Signetics SCN2641 ACI")

//**************************************************************************
//  BAUD RATE TABLES
//**************************************************************************

const u16 scn_pci_device::s_br_divisors_1[16] = {
	6144,   // 4.9152 MHz /    50 / 16
	4096,   // 4.9152 MHz /    75 / 16
	2793,   // 4.9152 MHz /   110 / 16
	2284,   // 4.9152 MHz / 134.5 / 16
	2048,   // 4.9152 MHz /   150 / 16
	1536,   // 4.9152 MHz /   200 / 16
	1024,   // 4.9152 MHz /   300 / 16
	512,    // 4.9152 MHz /   600 / 16
	292,    // 4.9152 MHz /  1050 / 16
	256,    // 4.9152 MHz /  1200 / 16
	171,    // 4.9152 MHz /  1800 / 16
	154,    // 4.9152 MHz /  2000 / 16
	128,    // 4.9152 MHz /  2400 / 16
	64,     // 4.9152 MHz /  4800 / 16
	32,     // 4.9152 MHz /  9600 / 16
	16      // 4.9152 MHz / 19200 / 16
};

const u16 scn_pci_device::s_br_divisors_2[16] = {
	6752,   // 4.9152 MHz /  45.5 / 16
	6144,   // 4.9152 MHz /    50 / 16
	4096,   // 4.9152 MHz /    75 / 16
	2793,   // 4.9152 MHz /   110 / 16
	2284,   // 4.9152 MHz / 134.5 / 16
	2048,   // 4.9152 MHz /   150 / 16
	1024,   // 4.9152 MHz /   300 / 16
	512,    // 4.9152 MHz /   600 / 16
	256,    // 4.9152 MHz /  1200 / 16
	171,    // 4.9152 MHz /  1800 / 16
	154,    // 4.9152 MHz /  2000 / 16
	128,    // 4.9152 MHz /  2400 / 16
	64,     // 4.9152 MHz /  4800 / 16
	32,     // 4.9152 MHz /  9600 / 16
	16,     // 4.9152 MHz / 19200 / 16
	8       // 4.9152 MHz / 38400 / 16
};

const u16 scn_pci_device::s_br_divisors_3[16] = {
	6336,   // 5.0688 MHz /    50 / 16
	4224,   // 5.0688 MHz /    75 / 16
	2880,   // 5.0688 MHz /   110 / 16
	2355,   // 5.0688 MHz / 134.5 / 16
	2112,   // 5.0688 MHz /   150 / 16
	1056,   // 5.0688 MHz /   300 / 16
	528,    // 5.0688 MHz /   600 / 16
	264,    // 5.0688 MHz /  1200 / 16
	176,    // 5.0688 MHz /  1800 / 16
	158,    // 5.0688 MHz /  2000 / 16
	132,    // 5.0688 MHz /  2400 / 16
	88,     // 5.0688 MHz /  3600 / 16
	66,     // 5.0688 MHz /  4800 / 16
	44,     // 5.0688 MHz /  7200 / 16
	33,     // 5.0688 MHz /  9600 / 16
	16      // 5.0688 MHz / 19200 / 16
};

const u16 scn2641_device::s_br_divisors[16] = {
	4608,   // 3.6864 MHz /    50 / 16
	3072,   // 3.6864 MHz /    75 / 16
	2095,   // 3.6864 MHz /   110 / 16
	1713,   // 3.6864 MHz / 134.5 / 16
	1536,   // 3.6864 MHz /   150 / 16
	768,    // 3.6864 MHz /   300 / 16
	384,    // 3.6864 MHz /   600 / 16
	192,    // 3.6864 MHz /  1200 / 16
	128,    // 3.6864 MHz /  1800 / 16
	115,    // 3.6864 MHz /  2000 / 16
	96,     // 3.6864 MHz /  2400 / 16
	64,     // 3.6864 MHz /  3600 / 16
	48,     // 3.6864 MHz /  4800 / 16
	32,     // 3.6864 MHz /  7200 / 16
	24,     // 3.6864 MHz /  9600 / 16
	12      // 3.6864 MHz / 19200 / 16
};

//**************************************************************************
//  DEVICE CONSTRUCTION & INITIALIZATION
//**************************************************************************

ALLOW_SAVE_TYPE(scn_pci_device::rcvr_state);
ALLOW_SAVE_TYPE(scn_pci_device::xmtr_state);

//-------------------------------------------------
//  scn_pci_device - constructor
//-------------------------------------------------

scn_pci_device::scn_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, const u16 *br_div, bool is_enhanced, bool is_aci)
	: device_t(mconfig, type, tag, owner, clock)
	, m_dtr_callback(*this)
	, m_rts_callback(*this)
	, m_txemt_dschg_callback(*this)
	, m_txc_callback(*this)
	, m_rxc_callback(*this)
	, m_txd_callback(*this)
	, m_txrdy_callback(*this)
	, m_rxrdy_callback(*this)
	, m_br_div(br_div)
	, m_is_enhanced(is_enhanced)
	, m_is_aci(is_aci)
	, m_brg_timer(nullptr)
	, m_rhr(0)
	, m_thr(0)
	, m_status(0xff)
	, m_syn{0, 0, 0}
	, m_mode{0, 0}
	, m_command(0)
	, m_mode_pointer(0)
	, m_syn_pointer(0)
	, m_rxd(true)
	, m_cts(true)
	, m_txc_input(true)
	, m_rxc_input(true)
	, m_rsr(0)
	, m_rbits(0)
	, m_rparity(false)
	, m_rcvr_state(rcvr_state::DISABLED)
	, m_rcvr_clocks(0)
	, m_null_frame_received(false)
	, m_pre_syndet(false)
	, m_tsr(0)
	, m_tbits(0)
	, m_tparity(false)
	, m_xmtr_state(xmtr_state::MARKING)
	, m_xmtr_clocks(0)
	, m_txd(false)
	, m_thr_loaded(false)
	, m_txemt(true)
	, m_dschg(true)
	, m_dtr(true)
	, m_rts(true)
	, m_syn1_parity(false)
	, m_brg_output(true)
{
}

//-------------------------------------------------
//  scn2651_device - constructor
//-------------------------------------------------

scn2651_device::scn2651_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: scn_pci_device(mconfig, SCN2651, tag, owner, clock, s_br_divisors_3, false, false)
{
}

//-------------------------------------------------
//  scn2661a_device - constructor
//-------------------------------------------------

scn2661a_device::scn2661a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: scn_pci_device(mconfig, SCN2661A, tag, owner, clock, s_br_divisors_1, true, false)
{
}

//-------------------------------------------------
//  scn2661b_device - constructor
//-------------------------------------------------

scn2661b_device::scn2661b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: scn_pci_device(mconfig, SCN2661B, tag, owner, clock, s_br_divisors_2, true, false)
{
}

//-------------------------------------------------
//  scn2661c_device - constructor
//-------------------------------------------------

scn2661c_device::scn2661c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: scn_pci_device(mconfig, SCN2661C, tag, owner, clock, s_br_divisors_3, true, false)
{
}

//-------------------------------------------------
//  scn2641_device - constructor
//-------------------------------------------------

scn2641_device::scn2641_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: scn_pci_device(mconfig, SCN2641, tag, owner, clock, s_br_divisors, true, true)
	, m_intr_callback(*this)
{
}


//-------------------------------------------------
//  device_validity_check - validate a device after
//  the configuration has been constructed
//-------------------------------------------------

void scn2641_device::device_validity_check(validity_checker &valid) const
{
	if (!m_dtr_callback.isnull())
		osd_printf_error("Nonexistent DTR output configured on SCN2641\n");
	if (!m_txemt_dschg_callback.isnull())
		osd_printf_error("Nonexistent TxEMT/DSCHG output configured on SCN2641 (use INTR instead)\n");
	if (!m_txrdy_callback.isnull())
		osd_printf_error("Nonexistent TxRDY output configured on SCN2641 (use INTR instead)\n");
	if (!m_rxrdy_callback.isnull())
		osd_printf_error("Nonexistent RxRDY output configured on SCN2641 (use INTR instead)\n");
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void scn_pci_device::device_resolve_objects()
{
	m_dtr_callback.resolve_safe();
	m_rts_callback.resolve_safe();
	m_txemt_dschg_callback.resolve_safe();
	m_txc_callback.resolve_safe();
	m_rxc_callback.resolve_safe();
	m_txd_callback.resolve_safe();
	m_txrdy_callback.resolve_safe();
	m_rxrdy_callback.resolve_safe();
}

void scn2641_device::device_resolve_objects()
{
	scn_pci_device::device_resolve_objects();
	m_intr_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void scn_pci_device::device_start()
{
	// Create timer for baud rate generator
	m_brg_timer = timer_alloc(FUNC(scn_pci_device::brg_tick), this);

	// Save state
	save_item(NAME(m_rhr));
	save_item(NAME(m_thr));
	save_item(NAME(m_status));
	save_item(NAME(m_mode));
	save_item(NAME(m_command));
	save_item(NAME(m_mode_pointer));
	save_item(NAME(m_rxd));
	save_item(NAME(m_cts));
	save_item(NAME(m_txc_input));
	save_item(NAME(m_rxc_input));
	save_item(NAME(m_rsr));
	save_item(NAME(m_rbits));
	save_item(NAME(m_rparity));
	save_item(NAME(m_rcvr_state));
	save_item(NAME(m_rcvr_clocks));
	save_item(NAME(m_null_frame_received));
	save_item(NAME(m_pre_syndet));
	save_item(NAME(m_tsr));
	save_item(NAME(m_tbits));
	save_item(NAME(m_tparity));
	save_item(NAME(m_xmtr_state));
	save_item(NAME(m_xmtr_clocks));
	save_item(NAME(m_txd));
	save_item(NAME(m_thr_loaded));
	save_item(NAME(m_txemt));
	save_item(NAME(m_dschg));
	save_item(NAME(m_dtr));
	save_item(NAME(m_rts));
	save_item(NAME(m_brg_output));
	if (!m_is_aci)
	{
		save_item(NAME(m_syn));
		save_item(NAME(m_syn_pointer));
		save_item(NAME(m_syn1_parity));
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void scn_pci_device::device_reset()
{
	// Clear command register
	m_command = 0;
	set_dtr(false);
	set_rts(false);

	// Reset transmitter
	m_xmtr_state = xmtr_state::MARKING;
	m_tbits = 0;
	m_txemt = false;
	set_txd(true);

	// Reset receiver
	m_rcvr_state = rcvr_state::DISABLED;
	m_null_frame_received = false;

	// Disable baud rate generator
	m_mode[1] &= 0xcf;
	m_brg_timer->enable(false);

	// Reset init pointers
	m_mode_pointer = 0;
	m_syn_pointer = 0;

	// Clear status (except for DCD and DSR input flags)
	set_rxrdy(false);
	set_txrdy(false);
	set_txemt(false);
	set_dschg(false);
	m_status &= 0xc0;
	m_dschg = false;
}

//**************************************************************************
//  RECEIVER
//**************************************************************************

//-------------------------------------------------
//  set_rxrdy - set RxRDY flag and line output
//-------------------------------------------------

void scn_pci_device::set_rxrdy(bool state)
{
	if (BIT(m_status, 1) == state)
		return;

	m_status = (m_status & 0xfd) | (state ? 0x02 : 0);
	m_rxrdy_callback(state ? ASSERT_LINE : CLEAR_LINE);
}

void scn2641_device::set_rxrdy(bool state)
{
	if (BIT(m_status, 1) == state)
		return;

	m_status = (m_status & 0xfd) | (state ? 0x02 : 0);
	if ((m_status & 0x05) == 0)
		m_intr_callback(state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  rx_load_async - load a completed asynchronous
//  data frame into the receiver holding register
//-------------------------------------------------

void scn_pci_device::rx_load_async(u8 data, bool pe, bool fe)
{
	assert((m_mode[0] & 0x03) != 0);
	LOGMASKED(LOG_RCVR, "rx_load_async(%02X, PE=%d, FE=%d)\n", data, pe, fe);

	// Parity error
	if (pe)
		m_status |= 0x08;

	// Framing error
	if (fe)
	{
		m_status |= 0x20;

		if (data == 0 && m_null_frame_received)
		{
			m_rcvr_state = rcvr_state::BREAK_DETECT;
			if (m_is_enhanced && (m_mode[1] & 0x90) == 0x90)
				m_rxc_callback(1);
		}
		else
			m_rcvr_state = rcvr_state::ASYNC_START;

		m_null_frame_received = data == 0;
	}
	else
	{
		m_rcvr_state = rcvr_state::ASYNC_WAIT;
		m_null_frame_received = false;
	}

	if (m_rcvr_state == rcvr_state::BREAK_DETECT)
		return;

	// Retransmit data in remote loopback and automatic echo modes
	if (BIT(m_command, 6))
		loopback_retransmit(data);

	if ((m_command & 0xc0) != 0xc0)
	{
		m_rhr = data;

		// Overrun checking
		if (BIT(m_status, 1))
			m_status |= 0x10;
		else
			set_rxrdy(true);
	}
}


//-------------------------------------------------
//  rx_load_sync - load a completed synchronous
//  data frame into the receiver holding register
//-------------------------------------------------

void scn_pci_device::rx_load_sync(u8 data, bool pe)
{
	assert((m_mode[0] & 0x03) == 0);
	LOGMASKED(LOG_RCVR, "rx_load_sync(%02X, PE=%d)\n", data, pe);

	bool syndet = false;
	if (pe)
	{
		// Parity error
		m_status |= 0x08;
		m_pre_syndet = false;
	}
	else if (BIT(m_mode[0], 6))
	{
		// Check for DLE followed by SYN1 in transparent sync mode
		if (m_pre_syndet)
		{
			// Set DLE received flag (only applies when no parity)
			if (!BIT(m_mode[0], 4))
			{
				if (!m_is_enhanced || (data != m_syn[0] && data != m_syn[2]))
					m_status |= 0x08;
				else if (m_is_enhanced)
					m_status &= 0xf7;
			}
			syndet = data == m_syn[0];
			m_pre_syndet = false;
		}
		else
		{
			m_pre_syndet = data == m_syn[2];
			if (m_is_enhanced && !BIT(m_mode[0], 4))
				m_status &= 0xf7;
		}
	}
	else if (!BIT(m_mode[0], 7))
	{
		// Check for SYN1 followed by SYN2 in double sync mode
		if (m_pre_syndet)
		{
			syndet = data == m_syn[1];
			if (!m_is_enhanced || data != m_syn[0])
				m_pre_syndet = false;
		}
		else
			m_pre_syndet = data == m_syn[0];
	}
	else
	{
		syndet = data == m_syn[0];
		m_pre_syndet = false;
	}
	if (syndet)
		m_status |= 0x20;

	// Retransmit data in remote loopback mode
	if ((m_command & 0xc0) == 0xc0)
	{
		loopback_retransmit(data);
		return;
	}

	// Automatic SYN/DLE stripping mode
	if ((m_command & 0xc0) == 0x40 && (syndet || m_pre_syndet))
		return;

	m_rhr = data;

	// Overrun checking
	if (BIT(m_status, 1))
		m_status |= 0x10;
	else
		set_rxrdy(true);
}


//-------------------------------------------------
//  rxd_w - set serial data input for receiver
//-------------------------------------------------

WRITE_LINE_MEMBER(scn_pci_device::rxd_w)
{
	m_rxd = state;
}


//-------------------------------------------------
//  read_rhr - read receive holding register
//-------------------------------------------------

u8 scn_pci_device::read_rhr()
{
	if (!machine().side_effects_disabled())
		set_rxrdy(false);

	return m_rhr;
}


//-------------------------------------------------
//  rcvr_sync - synchronize the receiver for a
//  new data frame
//-------------------------------------------------

void scn_pci_device::rcvr_sync()
{
	m_rbits = ((m_mode[0] & 0x0c) >> 2) + (BIT(m_mode[0], 4) ? 6 : 5);
	m_rparity = !BIT(m_mode[0], 5);
	m_rcvr_state = rcvr_state::SYNCED;
}


//-------------------------------------------------
//  rcvr_update - shift in serial data
//-------------------------------------------------

void scn_pci_device::rcvr_update(bool internal, bool output_1x)
{
	bool local_loopback = (m_command & 0xc0) == 0x80;
	bool rxd = local_loopback ? m_txd : m_rxd;
	bool sync_mode = (m_mode[0] & 0x03) == 0;

	if (m_rcvr_state == rcvr_state::DISABLED && (local_loopback ? (m_command & 0x03) == 0x03 : BIT(m_command, 2) && BIT(m_status, 6)))
	{
		LOGMASKED(LOG_RCVR, "Receiver enabled\n");
		m_rcvr_state = sync_mode ? rcvr_state::HUNT_MODE : rcvr_state::ASYNC_WAIT;
		m_null_frame_received = false;
		m_pre_syndet = false;
	}

	if (m_rcvr_state == rcvr_state::ASYNC_WAIT && !rxd)
	{
		if ((internal && !sync_mode) || (m_mode[0] & 0x03) == 0x02)
			m_rcvr_clocks = 8;
		else if ((m_mode[0] & 0x03) == 0x03)
			m_rcvr_clocks = 32;
		else
			m_rcvr_clocks = 0;

		if (output_1x)
		{
			if (!BIT(m_mode[1], 7))
				m_rxc_callback(0);

			bool tx_loopback = sync_mode ? (m_command & 0xc0) == 0xc0 : BIT(m_command, 6);
			if (tx_loopback && BIT(m_mode[1], 5))
				m_txc_callback(0);
		}
	}
	else if ((internal && !sync_mode) || BIT(m_mode[0], 1))
	{
		m_rcvr_clocks++;
		if (output_1x && (m_rcvr_clocks & 7) == 0)
		{
			if (!BIT(m_mode[1], 7))
				m_rxc_callback(!BIT(m_rcvr_clocks, 3));

			bool tx_loopback = sync_mode ? (m_command & 0xc0) == 0xc0 : BIT(m_command, 6);
			if (tx_loopback && BIT(m_mode[1], 5))
				m_txc_callback(!BIT(m_rcvr_clocks, 3));
		}

		u8 mask = internal || !BIT(m_mode[0], 0) ? 15 : 63;
		if ((m_rcvr_clocks & mask) != 0)
		{
			if (m_rcvr_state == rcvr_state::ASYNC_START && rxd && (m_rcvr_clocks & (mask >> 1)) == 0)
			{
				m_rcvr_state = rcvr_state::ASYNC_WAIT;
				m_null_frame_received = false;
			}
			return;
		}
	}

	switch (m_rcvr_state)
	{
	case rcvr_state::DISABLED:
		break;

	case rcvr_state::ASYNC_WAIT:
		if (!rxd)
		{
			LOGMASKED(LOG_RCVR, "Start bit observed\n");
			m_rcvr_state = rcvr_state::ASYNC_START;
		}
		break;

	case rcvr_state::ASYNC_START:
		if (rxd)
		{
			m_rcvr_state = rcvr_state::ASYNC_WAIT;
			m_null_frame_received = false;
		}
		else
		{
			LOGMASKED(LOG_RCVR, "Start bit verified\n");
			rcvr_sync();
			m_rbits++;
		}
		break;

	case rcvr_state::HUNT_MODE:
		if ((m_mode[1] & 0x90) != 0x80)
		{
			u8 dbits = ((m_mode[0] & 0x0c) >> 2) + 5;
			if (BIT(m_mode[0], 4))
			{
				// Check for SYN1 with parity
				if ((m_rsr >> (15 - dbits)) == (m_syn[0] | (BIT(m_mode[0], 5) ? m_syn1_parity : !m_syn1_parity) << dbits))
				{
					rcvr_sync();
					if (BIT(m_mode[0], 7))
						m_status |= 0x20;
					else
						m_rcvr_state = rcvr_state::HUNT_SYN2;
				}
			}
			else
			{
				// Check for SYN1 without parity
				if ((m_rsr >> (16 - dbits)) == m_syn[0])
				{
					rcvr_sync();
					if (BIT(m_mode[0], 7))
						m_status |= 0x20;
					else
						m_rcvr_state = rcvr_state::HUNT_SYN2;
				}
			}
		}
		break;

	case rcvr_state::HUNT_SYN2:
		if (--m_rbits == 0)
		{
			bool pe = false;
			if (BIT(m_mode[0], 4))
			{
				pe = m_rparity;
				m_rsr <<= 1;
			}

			// Check for SYN2
			if (!pe && (m_rsr >> (11 - ((m_mode[0] & 0x0c) >> 2))) == m_syn[1])
			{
				rcvr_sync();
				m_status |= 0x20;
			}
			else
				m_rcvr_state = rcvr_state::HUNT_MODE;
		}
		break;

	case rcvr_state::SYNCED:
		if (--m_rbits == 0)
		{
			bool pe = false;
			if (BIT(m_mode[0], 4))
			{
				pe = m_rparity;
				m_rsr <<= 1;
			}
			if (sync_mode)
			{
				rx_load_sync(m_rsr >> (11 - (((m_mode[0] & 0x0c) >> 2))), pe);
				rcvr_sync();
			}
			else
			{
				// FIXME: stop bit handling is not exactly correct for SCN2651
				rx_load_async(m_rsr >> (11 - (((m_mode[0] & 0x0c) >> 2))), pe, !rxd);
			}
		}
		break;

	case rcvr_state::BREAK_DETECT:
		if (rxd)
		{
			m_rcvr_state = rcvr_state::ASYNC_WAIT;
			m_null_frame_received = false;
			if (m_is_enhanced && (m_mode[1] & 0x90) == 0x90)
				m_rxc_callback(0);
		}
		break;
	}

	// Shift in one bit
	// N.B. At this point, the async start bit is counted as a data bit. Fortunately, this has no effect on parity calculation.
	if (m_rcvr_state == rcvr_state::SYNCED)
	{
		if (BIT(m_mode[0], 4) && m_rbits == 1)
			LOGMASKED(LOG_RCVR, "Shifting in parity bit %d (%s)\n", rxd, rxd == m_rparity ? "good" : "bad");
		else
			LOGMASKED(LOG_RCVR, "Shifting in data bit %d (%d left)\n", rxd, m_rbits - 1);
	}
	m_rsr >>= 1;
	if (rxd)
	{
		m_rsr |= 0x8000;
		m_rparity = !m_rparity;
	}
}

//**************************************************************************
//  TRANSMITTER
//**************************************************************************

//-------------------------------------------------
//  set_txd - set TxD output (if enabled)
//-------------------------------------------------

void scn_pci_device::set_txd(bool state)
{
	if (m_txd == state)
		return;

	m_txd = state;
	if ((m_command & 0xc0) != 0x80)
		m_txd_callback(state);
}


//-------------------------------------------------
//  set_txrdy - set TxRDY flag and line output
//-------------------------------------------------

void scn_pci_device::set_txrdy(bool state)
{
	if (BIT(m_status, 0) == state)
		return;

	m_status = (m_status & 0xfe) | (state ? 0x01 : 0);
	m_txrdy_callback(state ? ASSERT_LINE : CLEAR_LINE);
}

void scn2641_device::set_txrdy(bool state)
{
	if (BIT(m_status, 0) == state)
		return;

	m_status = (m_status & 0xfe) | (state ? 0x01 : 0);
	if ((m_status & 0x06) == 0)
		m_intr_callback(state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  set_txemt - set TxEMT flag and line output
//  (combined with DSCHG)
//-------------------------------------------------

void scn_pci_device::set_txemt(bool state)
{
	if (m_txemt == state)
		return;

	m_txemt = state;
	if (!m_dschg)
	{
		m_status = (m_status & 0xfb) | (state ? 0x04 : 0);
		m_txemt_dschg_callback(state ? ASSERT_LINE : CLEAR_LINE);
	}
}

void scn2641_device::set_txemt(bool state)
{
	if (m_txemt == state)
		return;

	m_txemt = state;
	if (!m_dschg)
	{
		m_status = (m_status & 0xfb) | (state ? 0x04 : 0);
		if ((m_status & 0x03) == 0)
			m_intr_callback(state ? ASSERT_LINE : CLEAR_LINE);
	}
}


//-------------------------------------------------
//  write_thr - write transmit holding register
//-------------------------------------------------

void scn_pci_device::write_thr(u8 data)
{
	m_thr = data;
	m_thr_loaded = true;

	// Automatically stuff DLE in transparent sync mode (EPCI only)
	if (m_is_enhanced && (m_mode[0] & 0x43) == 0x40 && data == m_syn[2])
		m_command |= 0x08;

	set_txrdy(false);
	set_txemt(false);
}


//-------------------------------------------------
//  loopback_retransmit - set transmit holding
//  register without affecting interrupts
//-------------------------------------------------

void scn_pci_device::loopback_retransmit(u8 data)
{
	m_thr = data;
	m_thr_loaded = true;
}


//-------------------------------------------------
//  tx_load - load transmit shift register with
//  next character
//-------------------------------------------------

void scn_pci_device::tx_load(u8 data)
{
	m_tsr = data;
	m_tbits = ((m_mode[0] & 0x0c) >> 2) + 5 - (BIT(m_mode[0], 4) ? 0 : 1);
	m_tparity = !BIT(m_mode[0], 5);
}


//-------------------------------------------------
//  xmtr_update - shift out and reload data in
//  transmit shift register
//-------------------------------------------------

void scn_pci_device::xmtr_update(bool internal, bool output_1x)
{
	bool sync_mode = (m_mode[0] & 0x03) == 0;
	bool local_loopback = (m_command & 0xc0) == 0x80;
	if ((internal && !sync_mode) || BIT(m_mode[0], 1))
	{
		m_xmtr_clocks++;
		if (output_1x && (m_xmtr_clocks & 7) == 0)
		{
			if (BIT(m_mode[1], 5))
				m_txc_callback(BIT(m_xmtr_clocks, 3));

			if (local_loopback && (m_mode[1] & 0x90) == 0x10)
				m_rxc_callback(BIT(m_xmtr_clocks, 3));
		}

		u8 mask = internal || !BIT(m_mode[0], 0) ? 15 : 63;
		if ((m_xmtr_clocks & mask) != 0)
			return;

		// Adjust for 1½ stop bits
		if ((m_mode[0] & 0xc0) == 0x80 && m_xmtr_state == xmtr_state::STOP_BITS && m_tbits == 2)
			m_xmtr_clocks += internal || !BIT(m_mode[0], 0) ? 8 : 32;
	}

	if (m_xmtr_state == xmtr_state::MARKING)
	{
		// Delayed RTS drop (EPCI only)
		if (m_is_enhanced && !BIT(m_command, 5))
			set_rts(false);

		m_tbits = 0;
		if (!sync_mode && BIT(m_command, 3))
		{
			// Send break
			set_txd(false);
			return;
		}
	}

	if (m_tbits == 0)
	{
		bool tx_enabled = (sync_mode ? (m_command & 0xc0) == 0xc0 : BIT(m_command, 6)) ? BIT(m_command, 2) : BIT(m_command, 0);
		bool cts = local_loopback ? BIT(m_command, 5) : m_cts;

		if (m_xmtr_state == xmtr_state::DATA_BITS || m_xmtr_state == xmtr_state::SYN1 || m_xmtr_state == xmtr_state::SYN2)
		{
			if (BIT(m_mode[0], 4))
			{
				LOGMASKED(LOG_XMTR, "Sending parity bit %d\n", m_tparity);
				set_txd(m_tparity);
			}
			else
			{
				LOGMASKED(LOG_XMTR, "Shifting out final data bit %d\n", BIT(m_tsr, 0));
				set_txd(BIT(m_tsr, 0));
			}

			if (!sync_mode)
			{
				// Send stop bits
				m_tbits = BIT(m_mode[0], 7) ? 2 : 1;
				m_xmtr_state = xmtr_state::STOP_BITS;
				m_tsr = 0xff;
				return;
			}
		}
		else if (sync_mode || !(m_thr_loaded && tx_enabled && cts))
			set_txd(true);

		if (sync_mode && !BIT(m_mode[0], 7) && m_xmtr_state == xmtr_state::SYN1 && tx_enabled)
		{
			// Send SYN2 after SYN1
			tx_load(m_syn[1]);
			m_xmtr_state = xmtr_state::SYN2;
		}
		else if (sync_mode && BIT(m_command, 3) && tx_enabled && cts)
		{
			// Send DLE before anything in the holding register
			tx_load(m_thr);
			m_xmtr_state = xmtr_state::SYN2;
			if (m_is_enhanced)
				m_command &= 0xf7;
		}
		else if (m_thr_loaded && tx_enabled && cts)
		{
			// Start bit
			if (!sync_mode)
			{
				LOGMASKED(LOG_XMTR, "Sending start bit\n");
				set_txd(false);
			}
			tx_load(m_thr);
			m_thr_loaded = false;
			bool tx_loopback = sync_mode ? (m_command & 0xc0) == 0xc0 : BIT(m_command, 6);
			if (!tx_loopback)
				set_txrdy(true);
			m_xmtr_state = xmtr_state::DATA_BITS;
		}
		else
		{
			bool tx_loopback = sync_mode ? (m_command & 0xc0) == 0xc0 : BIT(m_command, 6);
			if ((m_xmtr_state == xmtr_state::DATA_BITS || m_xmtr_state == xmtr_state::STOP_BITS) && !tx_loopback)
				set_txemt(true);
			if (sync_mode && tx_enabled)
			{
				// Send SYN1
				tx_load(m_syn[0]);
				m_xmtr_state = xmtr_state::SYN1;
			}
			else
				m_xmtr_state = xmtr_state::MARKING;
		}
	}
	else
	{
		// Shift out one bit
		if (BIT(m_tsr, 0))
			m_tparity = !m_tparity;
		LOGMASKED(LOG_XMTR, "Shifting out %sdata bit %d\n", m_xmtr_state == xmtr_state::DATA_BITS ? "" : "non-", BIT(m_tsr, 0));
		set_txd(BIT(m_tsr, 0));
		m_tsr >>= 1;
		m_tbits--;
	}
}

//**************************************************************************
//  MODEM CONTROL
//**************************************************************************

//-------------------------------------------------
//  set_dschg - set DSCHG flag and line output
//  (combined with TxEMT)
//-------------------------------------------------

void scn_pci_device::set_dschg(bool state)
{
	if (m_dschg == state)
		return;

	m_dschg = state;
	if (!m_txemt)
	{
		m_status = (m_status & 0xfb) | (state ? 0x04 : 0);
		m_txemt_dschg_callback(state ? ASSERT_LINE : CLEAR_LINE);
	}
}

void scn2641_device::set_dschg(bool state)
{
	if (m_dschg == state)
		return;

	m_dschg = state;
	if (!m_txemt)
	{
		m_status = (m_status & 0xfb) | (state ? 0x04 : 0);
		if ((m_status & 0x03) == 0)
			m_intr_callback(state ? ASSERT_LINE : CLEAR_LINE);
	}
}


//-------------------------------------------------
//  set_dtr - set data terminal ready output
//  (active low)
//-------------------------------------------------

void scn_pci_device::set_dtr(bool state)
{
	if (m_dtr != state)
	{
		m_dtr = state;
		m_dtr_callback(state ? 0 : 1);
	}
}


//-------------------------------------------------
//  set_rts - set request to send output (active
//  low)
//-------------------------------------------------

void scn_pci_device::set_rts(bool state)
{
	if (m_rts != state)
	{
		m_rts = state;
		m_rts_callback(state ? 0 : 1);
	}
}


//-------------------------------------------------
//  dsr_w - write general purpose data set ready
//  or ring indicator input (active low)
//-------------------------------------------------

WRITE_LINE_MEMBER(scn_pci_device::dsr_w)
{
	assert(!m_is_aci);

	if (BIT(m_status, 7) == !state)
		return;

	if (state)
		m_status &= 0x7f;
	else
		m_status |= 0x80;

	if ((m_command & 0x05) != 0 && (m_command & 0xc0) != 0x80)
		set_dschg(true);
}


//-------------------------------------------------
//  dcd_w - write data carrier detect input to
//  enable receiver (active low)
//-------------------------------------------------

WRITE_LINE_MEMBER(scn_pci_device::dcd_w)
{
	if (BIT(m_status, 6) == !state)
		return;

	if (state)
		m_status &= 0xbf;
	else
		m_status |= 0x40;

	if ((m_command & 0x05) != 0 && (m_command & 0xc0) != 0x80)
		set_dschg(true);

	// Disable receiver when DCD goes inactive (except in local loopback mode)
	if (state && (m_command & 0xc0) != 0x80 && m_rcvr_state != rcvr_state::DISABLED)
	{
		m_rcvr_state = rcvr_state::DISABLED;
		set_rxrdy(false);
	}
}


//-------------------------------------------------
//  cts_w - write clear to send input to enable
//  transmitter (active low)
//-------------------------------------------------

WRITE_LINE_MEMBER(scn_pci_device::cts_w)
{
	m_cts = !state;
}

//**************************************************************************
//  MODE SETTING
//**************************************************************************

const char *const scn_pci_device::s_stop_bit_desc[4] = {
	"invalid",
	"1",
	"1 1/2",
	"2"
};


//-------------------------------------------------
//  write_syn - write SYN1/SYN2/DLE registers
//-------------------------------------------------

void scn_pci_device::write_syn(u8 data)
{
	if (m_is_aci)
	{
		logerror("%s: Invalid write to SYN/DLE register\n", machine().describe_context());
		return;
	}

	assert(m_syn_pointer < 3);
	m_syn[m_syn_pointer] = data;
	if (m_syn_pointer == 2)
		LOGMASKED(LOG_INIT, "DLE character = %02X\n", data);
	else
		LOGMASKED(LOG_INIT, "SYN%d character = %02X\n", m_syn_pointer + 1, data);
	if (m_syn_pointer == 0)
		m_syn1_parity = BIT(population_count_32(data), 0);

	// Write SYN1/SYN2/DLE in sequence
	if (m_syn_pointer++ == 2)
		m_syn_pointer = 0;
}


//-------------------------------------------------
//  read_mode - read mode registers
//-------------------------------------------------

u8 scn_pci_device::read_mode()
{
	assert(m_mode_pointer < 2);
	u8 mode = m_mode[m_mode_pointer];

	if (!machine().side_effects_disabled())
		m_mode_pointer = (m_mode_pointer == 0) ? 1 : 0;

	return mode;
}


//-------------------------------------------------
//  write_mode - write mode registers
//-------------------------------------------------

void scn_pci_device::write_mode(u8 data)
{
	if (m_mode_pointer == 0)
	{
		if ((data & 0x03) != 0)
		{
			LOGMASKED(LOG_INIT, "Asynchronous %dX mode: %d data bits, %s parity, %s stop bit%s\n",
								BIT(data, 1) ? (BIT(data, 0) ? 64 : 16) : 1,
								((data & 0x0c) >> 2) + 5,
								BIT(data, 4) ? (BIT(data, 5) ? "even" : "odd") : "no",
								s_stop_bit_desc[(data & 0xc0) >> 6], (data & 0xc0) != 0x40 ? "s" : "");
		}
		else
		{
			LOGMASKED(LOG_INIT, "Synchronous 1X mode: %s%s SYN, %d data bits, %s parity\n",
								BIT(data, 6) ? "transparent " : "",
								BIT(data, 7) ? "single" : "double",
								((data & 0x0c) >> 2) + 5,
								BIT(data, 4) ? (BIT(data, 5) ? "even" : "odd") : "no");
			if (m_is_aci)
				logerror("%s: Invalid synchronous mode configured\n", machine().describe_context());
		}

		m_mode[0] = data;

		// Write MR2 after MR1
		m_mode_pointer = 1;
	}
	else
	{
		assert(m_mode_pointer == 1);

		if (!m_is_enhanced)
		{
			if (data >= 0x50)
				logerror("Warning: incompatible EPCI TxC/RxC mode selected (MR2 = %02X)\n", data);
			data &= 0x3f;
		}

		if (m_mode[1] != data)
		{
			bool sync_mode = (m_mode[0] & 0x03) == 0;
			double baud_rate = clocks_to_attotime(m_br_div[data & 0x0f] * (sync_mode ? 1 : 16)).as_hz();
			if ((data & 0x90) == 0x80)
			{
				if (BIT(data, 5))
					LOGMASKED(LOG_INIT, "Internal TxC (%.1f baud), external RxC with XSYNC input\n", baud_rate);
				else
					LOGMASKED(LOG_INIT, "External RxC/TxC (combined) with XSYNC input\n");
			}
			else if (BIT(data, 4))
			{
				LOGMASKED(LOG_INIT, "%sRxC (%.1f baud with %s output)\n",
									BIT(data, 5) ? "Internal TxC/" : "External TxC, internal ",
									baud_rate,
									BIT(data, 7) ? "BKDET" : BIT(data, 6) ? "16X" : "1X");

				// BKDET output
				if (BIT(data, 7))
					m_rxc_callback(m_rcvr_state == rcvr_state::BREAK_DETECT ? 1 : 0);
			}
			else if (BIT(data, 5))
			{
				LOGMASKED(LOG_INIT, "Internal TxC (%.1f baud with %s output), external RxC\n",
									baud_rate,
									BIT(data, 6) ? "16X" : "1X");
			}
			else
				LOGMASKED(LOG_INIT, "External TxC, external RxC (may be indepedent)\n");

			if ((data & 0x30) == 0)
				m_brg_timer->enable(false);
			else
			{
				attotime brg_period = clocks_to_attotime(m_br_div[data & 0x0f]);
				m_brg_timer->adjust(brg_period / 2, 0, brg_period / 2);
			}
		}

		m_mode[1] = data;

		// Write MR1 after MR2
		m_mode_pointer = 0;
	}
}

//**************************************************************************
//  STATUS & CONTROL REGISTERS
//**************************************************************************

//-------------------------------------------------
//  read_status - read status register
//-------------------------------------------------

u8 scn_pci_device::read_status()
{
	u8 status = m_status;

	// Local loopback connects DTR to DCD internally
	if ((m_command & 0xc0) == 0x80)
		status = (status & 0xbf) | (BIT(m_command, 1) ? 0x40 : 0);

	if (!machine().side_effects_disabled())
	{
		// Reset DSCHG
		set_dschg(false);

		// Reset SYN detect in synchronous mode
		if ((m_mode[0] & 0x03) == 0)
			m_status &= 0xdf;
	}

	return status;
}


//-------------------------------------------------
//  read_command - read command register
//-------------------------------------------------

u8 scn_pci_device::read_command()
{
	if (!machine().side_effects_disabled())
	{
		// Reset pointers
		m_mode_pointer = 0;
		m_syn_pointer = 0;
	}

	return m_command;
}


//-------------------------------------------------
//  write_command - write command register
//-------------------------------------------------

void scn_pci_device::write_command(u8 data)
{
	u8 last_command = std::exchange(m_command, data);
	bool sync_mode = (m_mode[0] & 0x03) == 0;
	bool tx_loopback = sync_mode ? (data & 0xc0) == 0xc0 : BIT(data, 6);
	bool tx_enabled = tx_loopback ? BIT(data, 2) : BIT(data, 0);
	bool local_loopback = (data & 0xc0) == 0x80;
	bool rx_enabled = local_loopback ? BIT(data, 0) && BIT(data, 1) : BIT(data, 2) && BIT(m_status, 6);

	if ((last_command & 0x06) != (data & 0x06))
		LOGMASKED(LOG_COMMAND, "Rx %sabled, DTR %s\n", rx_enabled ? "en" : "dis", BIT(data, 1) ? "on" : "off");
	if ((last_command & 0x21) != (data & 0x21))
		LOGMASKED(LOG_COMMAND, "Tx %sabled, RTS %s\n", tx_enabled ? "en" : "dis", BIT(data, 5) ? "on" : "off");

	if (!BIT(data, 0) || tx_loopback)
	{
		set_txrdy(false);
		set_txemt(false);
	}
	else if (tx_enabled && !m_thr_loaded)
		set_txrdy(true);

	// Error flags are reset by instantaneous command or when receiver is disabled
	if (BIT(data, 4) || !rx_enabled)
	{
		u8 error_flags = sync_mode && rx_enabled ? 0x18 : 0x38;
		if ((m_status & error_flags) != 0)
		{
			m_status &= ~error_flags;
			LOGMASKED(LOG_COMMAND, "Rx error flags reset\n");
		}

		// Error reset command is not latched
		data &= 0xef;
	}

	if (!rx_enabled)
		m_rcvr_state = rcvr_state::DISABLED;

	set_dtr(BIT(data, 1) && !local_loopback);
	if (local_loopback)
		m_txd_callback(1);
	else
		m_txd_callback(m_txd);

	if (BIT(data, 5) && !local_loopback)
		set_rts(true);
	else if (!m_is_enhanced || !tx_enabled || sync_mode || m_xmtr_state == xmtr_state::MARKING)
		set_rts(false);

	if (!rx_enabled || (data & 0xc0) == 0xc0)
	{
		set_rxrdy(false);
		if (!tx_enabled)
			set_dschg(false);
	}

	if (BIT(data, 3))
	{
		if (!sync_mode)
			LOGMASKED(LOG_COMMAND, "Force break\n");
		else
			LOGMASKED(LOG_COMMAND, "Send DLE\n");
	}
}

//**************************************************************************
//  MICROPROCESSOR INTERFACE
//**************************************************************************

//-------------------------------------------------
//  read - read data from addressed register
//-------------------------------------------------

u8 scn_pci_device::read(offs_t offset)
{
	switch (offset & 3)
	{
	case 0:
		return read_rhr();

	case 1:
		return read_status();

	case 2:
		return read_mode();

	case 3:
		return read_command();

	default: // should never happen, but compilers are dumb
		return 0;
	}
}


//-------------------------------------------------
//  write - write data to addressed register
//-------------------------------------------------

void scn_pci_device::write(offs_t offset, u8 data)
{
	switch (offset & 3)
	{
	case 0:
		write_thr(data);
		break;

	case 1:
		write_syn(data);
		break;

	case 2:
		write_mode(data);
		break;

	case 3:
		write_command(data);
		break;
	}
}

//**************************************************************************
//  CLOCK INPUTS & BAUD RATE GENERATION
//**************************************************************************

//-------------------------------------------------
//  txc_w - external clock input for transmitter
//  (or jam sync for EPCI receiver)
//-------------------------------------------------

WRITE_LINE_MEMBER(scn_pci_device::txc_w)
{
	if (state == m_txc_input)
		return;

	m_txc_input = state;

	if ((m_mode[1] & 0x90) == 0x80)
	{
		// XSYNC input active on rising edge
		if (state && m_rcvr_state == rcvr_state::HUNT_MODE)
			rcvr_sync();
	}
	else if (!BIT(m_mode[1], 5))
	{
		if (state)
		{
			// Receiver is clocked by the transmit clock in local loopback mode
			if ((m_command & 0xc0) == 0x80)
				rcvr_update(false, false);
		}
		else
		{
			bool tx_loopback = (m_mode[0] & 0x03) == 0 ? (m_command & 0xc0) == 0xc0 : BIT(m_command, 6);
			if (!tx_loopback)
				xmtr_update(false, false);
		}
	}
}


//-------------------------------------------------
//  rxc_w - external clock input for receiver
//-------------------------------------------------

WRITE_LINE_MEMBER(scn_pci_device::rxc_w)
{
	if (state == m_rxc_input)
		return;

	m_rxc_input = state;

	if (!BIT(m_mode[1], 4))
	{
		bool rxc_txc = (m_mode[1] & 0xa0) == 0x80;
		if (state)
		{
			if ((m_command & 0xc0) != 0x80 || rxc_txc)
				rcvr_update(false, false);
		}
		else
		{
			bool tx_loopback = (m_mode[0] & 0x03) == 0 ? (m_command & 0xc0) == 0xc0 : BIT(m_command, 6);
			if (rxc_txc || tx_loopback)
				xmtr_update(false, false);
		}
	}
}


//-------------------------------------------------
//  brg_tick - handle one edge of the internally
//  generated baud rate clock
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(scn_pci_device::brg_tick)
{
	bool txc_internal = BIT(m_mode[1], 5);
	bool rxc_internal = BIT(m_mode[1], 4);

	if (!txc_internal && !rxc_internal)
	{
		m_brg_timer->enable(false);
		return;
	}

	m_brg_output = !m_brg_output;

	bool sync_mode = (m_mode[0] & 0x03) == 0;
	if (sync_mode || BIT(m_mode[1], 6))
	{
		// 16X clock outputs
		if (txc_internal && (m_mode[1] & 0x90) != 0x80)
			m_txc_callback(m_brg_output);
		if (rxc_internal && !BIT(m_mode[1], 7))
			m_rxc_callback(m_brg_output);
	}

	if (m_brg_output)
	{
		bool local_loopback = (m_command & 0xc0) == 0x80;
		if (local_loopback ? txc_internal : rxc_internal)
			rcvr_update(true, !local_loopback && !sync_mode && (m_mode[1] < 0x40 || (m_mode[1] & 0xf0) == 0xb0));
	}
	else
	{
		bool tx_loopback = sync_mode ? (m_command & 0xc0) == 0xc0 : BIT(m_command, 6);
		if (tx_loopback ? rxc_internal : txc_internal)
			xmtr_update(true, !tx_loopback && !sync_mode && (m_mode[1] < 0x40 || (m_mode[1] & 0xf0) == 0xb0));
	}
}
