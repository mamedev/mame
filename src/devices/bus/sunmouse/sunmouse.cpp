// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Sun mouse port

    Before USB, Sun used an asynchronous serial mouse protocol.  Data is
    transmitted at TTL levels using an asynchronous serial protocol.
    The mouse RxD line is pulled up by the host and driven low by an
    open collector driver in the mouse for the mark (1) condition.  This
    means that if no mouse is connected, the host will see a break
    condition.  The protocol is mostly compatible with the Mouse Systems
    "non-rotatable" protocol.

    Most Sun mouse port devices transmit data at 1,200 Baud.  However,
    some devices transmit at 4,800 Baud, and it's possible to modify any
    Sun Type 5 mouse for 4,800 Baud operation.  Solaris 2.3 or later is
    required to support a 4,800 Baud mouse.

    From the Sun 2/50 onwards, the keyboard and mouse share a single
    connector, either using a splitter adapter, or by plugging the mouse
    into a pass-through connector on the keyboard.

    Type 3 female DA-15 connector on host (introduced on Sun 2/50):
     1: keyboard RxD         9: GND
     2: GND                 10: +5V
     3: keyboard TxD        11: +5V
     4: GND                 12: +5V
     5: mouse RxD           13: +5V
     6: GND                 14: +5V
     7: mouse TxD           15: +5V
     8: GND

    Type 4 female Mini-DIN-8 connector on host (introduced on Sun 3/80):
    1: GND          3: +5V              6: keyboard RxD
    2: GND          4: mouse RxD        7: mouse TxD
                    5: keyboard TxD     8: +5V

    Type 5 female Mini-DIN-8 connector on host:
    1: GND          3: +5V              6: keyboard RxD
    2: GND          4: mouse RxD        7: power key
                    5: keyboard TxD     8: +5V

    Type 5 and later sacrifice the TxD line from the host to the mouse
    in favour of a dedicated line for the soft power key.  It was never
    used for anything useful with standard Sun peripherals anyway.

    Sun 2 and Sun 3 used an optical mouse made by Mouse Systems.  The
    colour varied (usually black for Sun 2 and white for Sun 3).  They
    require a Mouse Systems optical mouse pad to work correctly.  The
    later Sun optical mouse for Sun 4 requires a different pad with
    thinner stripes.

    The Logitech Trackman T-CB1 is a compatible replacement for a Sun
    Type 4 or Type 5 mouse, made at Sun's request.

    Known Sun mouse part numbers:
    * 370-1169: Type 4 optical mouse (manufactured by Mouse Systems)
    * 370-1398: Type 5 optical mouse (bridge JP2 for 4,800 Baud)
    * 370-1586-01: SPARCstation Voyager 4,800 Baud mouse
    * 370-1586-02: Type 5 opto-mechanical mouse
    * 370-1586-03: Type 5 opto-mechanical mouse

    TODO:
    * Confirm polarity of TxD signal from host to mouse.
    * Dump mouse microcontrollers.
*/

#include "emu.h"
#include "sunmouse.h"


DEFINE_DEVICE_TYPE(SUNMOUSE_PORT, sun_mouse_port_device, "sunmouse", "Sun Mouse Port")


int const device_sun_mouse_port_interface::START_BIT_COUNT;
int const device_sun_mouse_port_interface::DATA_BIT_COUNT;
device_serial_interface::parity_t const device_sun_mouse_port_interface::PARITY;
device_serial_interface::stop_bits_t const device_sun_mouse_port_interface::STOP_BITS;
int const device_sun_mouse_port_interface::BAUD;



sun_mouse_port_device::sun_mouse_port_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: sun_mouse_port_device(mconfig, SUNMOUSE_PORT, tag, owner, clock)
{
}


sun_mouse_port_device::sun_mouse_port_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface<device_sun_mouse_port_interface>(mconfig, *this)
	, m_rxd(0)
	, m_rxd_handler(*this)
	, m_dev(nullptr)
{
}


sun_mouse_port_device::~sun_mouse_port_device()
{
}


void sun_mouse_port_device::device_config_complete()
{
	m_dev = get_card_device();
}


void sun_mouse_port_device::device_resolve_objects()
{
	m_rxd = 1;

	m_rxd_handler.resolve_safe();
}


void sun_mouse_port_device::device_start()
{
	save_item(NAME(m_rxd));

	if (!m_dev)
		m_rxd_handler(m_rxd = 0);
}


WRITE_LINE_MEMBER( sun_mouse_port_device::write_txd )
{
	if (m_dev)
		m_dev->input_txd(state ? 0 : 1);
}



device_sun_mouse_port_interface::device_sun_mouse_port_interface(machine_config const &mconfig, device_t &device)
	: device_interface(device, "sunmouse")
	, m_port(dynamic_cast<sun_mouse_port_device *>(device.owner()))
{
}


device_sun_mouse_port_interface::~device_sun_mouse_port_interface()
{
}



#include "hlemouse.h"

void default_sun_mouse_devices(device_slot_interface &device)
{
	device.option_add("hle1200", SUN_1200BAUD_HLE_MOUSE);
	device.option_add("hle4800", SUN_4800BAUD_HLE_MOUSE);
}
