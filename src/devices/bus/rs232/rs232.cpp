// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Generic EIA RS-232/CCITT V.24 serial port emulation

****************************************************************************

    Source  DB-25  DB-9 EIA V.24
             1          AA  101  Protective Ground [Shield]
             7      5   AB  102  Signal Ground [Common Return]
    DTE →    2      3   BA  103  Transmitted Data
    ← DCE    3      2   BB  104  Received Data
    DTE →    4      7   CA  105  Request to Send
    ← DCE    5      8   CB  106  Clear to Send
    ← DCE    6      6   CC  107  Data Set Ready [Data Mode]
    DTE →   20             108.1 Connect Data Set to Line
    DTE →   20      4   CD 108.2 Data Terminal Ready
    ← DCE   22      9   CE  125  Ring Indicator
    ← DCE    8      1   CF  109  Received Line Signal Detector [Data Carrier Detect]
    ← DCE   21          CG  110  Signal Quality Detector
    DTE →   23          CH  111  Data Signal Rate Selector
    ← DCE   23          CI  112  Data Signal Rate Selector [Signaling Rate Indicator]
    ← DCE   12                   High Speed Indicator (Bell 212A)
    DTE →   24          DA  113  Transmitter Signal Element Timing [External Tx Clock]
    ← DCE   15          DB  114  Transmitter Signal Element Timing
    ← DCE   17          DD  115  Receiver Signal Element Timing
    DTE →   14         SBA  118  Secondary Transmitted Data
    ← DCE   16         SBB  119  Secondary Received Data
    DTE →   19         SCA  120  Secondary Request to Send
    ← DCE   13         SCB  121  Secondary Clear to Send
    ← DCE   12         SCF  122  Secondary Received Line Signal Detector
    DTE →   18          LL  141  Local Loopback
    DTE →   21          RL  140  Remote Loopback
    ← DCE   25          TM  142  Test Mode

    Mark (logic 1) = typically -12V (-3V maximum, -15V minimum)
    Space (logic 0) = typically +12V (+3V minimum, +15V maximum)

***************************************************************************/

#include "emu.h"
#include "rs232.h"

DEFINE_DEVICE_TYPE(RS232_PORT, rs232_port_device, "rs232", "RS232 Port")

rs232_port_device::rs232_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	rs232_port_device(mconfig, RS232_PORT, tag, owner, clock)
{
}

rs232_port_device::rs232_port_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_rxd(0),
	m_dcd(0),
	m_dsr(0),
	m_ri(0),
	m_si(0),
	m_cts(0),
	m_dce_rxc(0),
	m_dce_txc(0),
	m_rxd_handler(*this),
	m_dcd_handler(*this),
	m_dsr_handler(*this),
	m_ri_handler(*this),
	m_si_handler(*this),
	m_cts_handler(*this),
	m_rxc_handler(*this),
	m_txc_handler(*this),
	m_dev(nullptr)
{
}

rs232_port_device::~rs232_port_device()
{
}

void rs232_port_device::device_config_complete()
{
	m_dev = dynamic_cast<device_rs232_port_interface *>(get_card_device());
}

void rs232_port_device::device_resolve_objects()
{
	m_rxd_handler.resolve_safe();
	m_dcd_handler.resolve_safe();
	m_dsr_handler.resolve_safe();
	m_ri_handler.resolve_safe();
	m_si_handler.resolve_safe();
	m_cts_handler.resolve_safe();
	m_rxc_handler.resolve_safe();
	m_txc_handler.resolve_safe();
}

void rs232_port_device::device_start()
{
	save_item(NAME(m_rxd));
	save_item(NAME(m_dcd));
	save_item(NAME(m_dsr));
	save_item(NAME(m_ri));
	save_item(NAME(m_si));
	save_item(NAME(m_cts));
	save_item(NAME(m_dce_rxc));
	save_item(NAME(m_dce_txc));

	m_rxd = 1;
	m_dcd = 1;
	m_dsr = 1;
	m_ri = 1;
	m_si = 1;
	m_cts = 1;

	m_rxd_handler(1);
	m_dcd_handler(1);
	m_dsr_handler(1);
	m_ri_handler(1);
	m_si_handler(1);
	m_cts_handler(1);
}

WRITE_LINE_MEMBER( rs232_port_device::write_txd )
{
	if (m_dev)
		m_dev->input_txd(state);
}

WRITE_LINE_MEMBER( rs232_port_device::write_dtr )
{
	if (m_dev)
		m_dev->input_dtr(state);
}

WRITE_LINE_MEMBER( rs232_port_device::write_rts )
{
	if (m_dev)
		m_dev->input_rts(state);
}

WRITE_LINE_MEMBER( rs232_port_device::write_etc )
{
	if (m_dev)
		m_dev->input_etc(state);
}

WRITE_LINE_MEMBER( rs232_port_device::write_spds )
{
	if (m_dev)
		m_dev->input_spds(state);
}

device_rs232_port_interface::device_rs232_port_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_port = dynamic_cast<rs232_port_device *>(device.owner());
}

device_rs232_port_interface::~device_rs232_port_interface()
{
}

#include "keyboard.h"
#include "loopback.h"
#include "null_modem.h"
#include "printer.h"
#include "pty.h"
#include "sun_kbd.h"
#include "terminal.h"
#include "ie15.h"

void default_rs232_devices(device_slot_interface &device)
{
	device.option_add("keyboard", SERIAL_KEYBOARD);
	device.option_add("loopback", RS232_LOOPBACK);
	device.option_add("dec_loopback", DEC_RS232_LOOPBACK);
	device.option_add("null_modem", NULL_MODEM);
	device.option_add("printer", SERIAL_PRINTER);
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("pty", PSEUDO_TERMINAL);
	device.option_add("sunkbd", SUN_KBD_ADAPTOR);
	device.option_add("ie15", SERIAL_TERMINAL_IE15);
}
