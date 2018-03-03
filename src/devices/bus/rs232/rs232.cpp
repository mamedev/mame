// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Generic EIA RS-232/CCITT V.24 serial port emulation

****************************************************************************

    Source  DB-25  DB-9
             1          AA  Protective Ground [Shield]
             7      5   AB  Signal Ground [Common Return]
    DTE →    2      3   BA  Transmitted Data
    ← DCE    3      2   BB  Received Data
    DTE →    4      7   CA  Request to Send
    ← DCE    5      8   CB  Clear to Send
    ← DCE    6      6   CC  Data Set Ready [Data Mode]
    DTE →   20      4   CD  Data Terminal Ready
    ← DCE   22      9   CE  Ring Indicator
    ← DCE    8      1   CF  Received Line Signal Detector [Data Carrier Detect]
    ← DCE   21          CG  Signal Quality Detector
    DTE →   23          CH  Data Signal Rate Selector
    ← DCE   23          CI  Data Signal Rate Selector [Signaling Rate Indicator]
    DTE →   24          DA  Transmitter Signal Element Timing [External Tx Clock]
    ← DCE   15          DB  Transmitter Signal Element Timing
    ← DCE   17          DD  Receiver Signal Element Timing
    DTE →   14         SBA  Secondary Transmitted Data
    ← DCE   16         SBB  Secondary Received Data
    DTE →   19         SCA  Secondary Request to Send
    ← DCE   13         SCB  Secondary Clear to Send
    ← DCE   12         SCF  Secondary Received Line Signal Detector
    DTE →   18          LL  Local Loopback
    DTE →   21          RL  Remote Loopback
    ← DCE   25          TM  Test Mode

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
	m_cts(0),
	m_dce_rxc(0),
	m_dce_txc(0),
	m_rxd_handler(*this),
	m_dcd_handler(*this),
	m_dsr_handler(*this),
	m_ri_handler(*this),
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
	save_item(NAME(m_cts));
	save_item(NAME(m_dce_rxc));
	save_item(NAME(m_dce_txc));

	m_rxd = 1;
	m_dcd = 1;
	m_dsr = 1;
	m_ri = 1;
	m_cts = 1;

	m_rxd_handler(1);
	m_dcd_handler(1);
	m_dsr_handler(1);
	m_ri_handler(1);
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

SLOT_INTERFACE_START( default_rs232_devices )
	SLOT_INTERFACE("keyboard", SERIAL_KEYBOARD)
	SLOT_INTERFACE("loopback", RS232_LOOPBACK)
	SLOT_INTERFACE("null_modem", NULL_MODEM)
	SLOT_INTERFACE("printer", SERIAL_PRINTER)
	SLOT_INTERFACE("terminal", SERIAL_TERMINAL)
	SLOT_INTERFACE("pty", PSEUDO_TERMINAL)
	SLOT_INTERFACE("sunkbd", SUN_KBD_ADAPTOR)
	SLOT_INTERFACE("ie15", SERIAL_TERMINAL_IE15)
SLOT_INTERFACE_END
