// license: BSD-3-Clause
// copyright-holders: Vas Crabb
/***************************************************************************

    Mac 128k/512k/Plus keyboard interface (pre-ADB)

    Two-wire protocol designed to avoid the need for an accurate/stable
    clock source in the keyboard.  This is not a multi-drop bus.  In
    cases where peripherals are daisy-chained, the upstream peripheral
    acts as a host for the downstream peripheral.

    In the idle state, both signal lines are high, pulled up using
    resistors.  All transactions are initiated by the host, but the
    timing is controlled entirely by the peripheral.  The data signal
    must be valid on the rising edge of the clock signal.  The data is
    not inverted.

    Transaction steps (ignoring mandatory delays):
    * Host pulls down data line to initiate transfer
    * Peripheral reads one octet (eight bits) from host, MSB first:
      - Peripheral pulls clock line low
      - Host sets bit on data line
      - Peripheral releases clock line
      - Peripheral samples data line
      - Repeat for remaining bits of octet
    * Host releases data line
    * Peripheral sends one octet (eight bits) to host, MSB first:
      - Peripheral places bit on data line
      - Peripheral pulls clock line low
      - Peripheral releases clock line
      - Host samples data line
      - Repeat for remaining bits of octet
    * Peripheral releases data line

    The last bit (LSB) of the octet transferred from the host to the
    peripheral is always zero.  This allows the host to hold the data
    line low until it's ready to receive the response from the
    peripheral.  The last bit of the octet transferred from the
    peripheral to the host is always high.

    Some transactions expect an immediate response while others allow
    the peripheral to delay the response if it has no data to send
    immediately.  The host polls the peripheral continuously.  If no
    response is received in about 500ms, the host assumes the
    peripheral has crashed/reset or has been unplugged, and will
    attempt to re-establish communication.

    Transactions:
    * 0x10: Permission to send
      Peripheral responds immediately with data if avaiblable.  If no
      data is available, peripheral waits for data and responds when it
      becomes available.  If no data is available after 250ms, the
      peripheral responds with 0x7b to hand control back to the host.
    * 0x14: Request to send
      Peripheral responds immediately with data, or 0x7b if no data is
      available.
    * 0x16: Reset and identify
      Peripheral responds with identification code.
      - 0x03: M0110 compact keyboard
      - 0x0b: M0110A Mac Plus keyboard
      - 0x11: M0120 numeric keypad
      - 0x13: M0120 numeric keypad with M0110 compact keyboard
      - 0x1b: M0120 numeric keypad with M0110A Mac Plus keyboard
      - 0x27: Assimilation Process keypad with M0110 keyboard???
    * 0x36: Perform self test
      Peripheral responds with 0x7d (pass) or 0x77 (failure).

    Responses to polling commands 0x10 and 0x14 have the first bit clear
    for key down events or set for key up events.

    The keypad passes polling commands 0x10 and 0x14 on to the keyboard
    if it has no response to send.  If the keypad has a key transition
    to report, it responds with the octet 0x79.  After receiving this
    response, the host should send command 0x10 to receive the key
    transition event from the keypad.

    Additionally, if bit 5 of the octet sent to the peripheral is set
    (0x40), the octet is passed on to a daisy-chained peripheral with
    this bit cleared.  The response from the daisy-chained peripheral is
    passed back to the host.  If the transaction to the daisy-chained
    peripheral times out, the peripheral sends the response 0x77 to the
    host.

    TODO:
    * Dump/emulate newer versions of Apple keyboards (GI PIC1657 based)
    * Emulate Music Publisher music notation input pad
    * Emulate Assimilation Process keypad (and trackball)

***************************************************************************/

#include "emu.h"
#include "mackbd.h"


DEFINE_DEVICE_TYPE(MAC_KEYBOARD_PORT, mac_keyboard_port_device, "mackbd_port", "Macintosh 128k/512k/Plus Keyboard Port")


//**************************************************************************
//  HOST PORT
//**************************************************************************

mac_keyboard_port_device::mac_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MAC_KEYBOARD_PORT, tag, owner, clock)
	, device_single_card_slot_interface<device_mac_keyboard_interface>(mconfig, *this)
	, m_clock_cb{ *this }
	, m_data_cb{ *this }
	, m_peripheral{ nullptr }
{
}

mac_keyboard_port_device::~mac_keyboard_port_device()
{
}

void mac_keyboard_port_device::data_w(int state)
{
	if (m_peripheral)
		m_peripheral->data_w(state);
}

void mac_keyboard_port_device::device_start()
{
	m_peripheral = dynamic_cast<device_mac_keyboard_interface *>(get_card_device());
}


//**************************************************************************
//  PERIPHERAL INTERFACE
//**************************************************************************

device_mac_keyboard_interface::device_mac_keyboard_interface(machine_config const &mconfig, device_t &device)
	: device_interface(device, "mackbd")
	, m_host{ dynamic_cast<mac_keyboard_port_device *>(device.owner()) }
{
}

device_mac_keyboard_interface::~device_mac_keyboard_interface()
{
}

void device_mac_keyboard_interface::interface_validity_check(validity_checker &valid) const
{
	device_t *const owner(device().owner());
	if (owner && dynamic_cast<device_slot_interface *>(owner) && !dynamic_cast<mac_keyboard_port_device *>(owner))
		osd_printf_error("Device %s (%s) is not a Macintosh keyboard port\n", owner->tag(), owner->name());
}


//**************************************************************************
//  SUPPORTED DEVICES
//**************************************************************************

#include "keyboard.h"
#include "pluskbd.h"


// must come after including the headers that declare these extern
template class device_finder<device_mac_keyboard_interface, false>;
template class device_finder<device_mac_keyboard_interface, true>;


void mac_keyboard_devices(device_slot_interface &device)
{
	device.option_add("us",    MACKBD_M0110);
	device.option_add("gb",    MACKBD_M0110B);
	device.option_add("fr",    MACKBD_M0110F);
	device.option_add("it",    MACKBD_M0110T);
	device.option_add("jp",    MACKBD_M0110J);

	device.option_add("pad",   MACKBD_M0120);
	device.option_add("eupad", MACKBD_M0120P);

	device.option_add("usp",   MACKBD_M0110A);
	device.option_add("frp",   MACKBD_M0110A_F);
	device.option_add("jpp",   MACKBD_M0110A_J);
}
