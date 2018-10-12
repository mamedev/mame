// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Sun keyboard port

    Pre-USB Sun keyboards use an asynchronous serial protocol.  Data is
    transmitted at TTL levels using an asynchronous serial protocol at
    1,200 Baud.  The protocol remained compatible from at least the
    Type 2 keyboard to the Type 6 keyboard, although key layout and key
    cap labels varied, some scancodes were repurposed, and a variety of
    connectors were used.

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

    Scancodes (U.S. caps except as noted):
    00:                 20: 3               40: [ {             60: Page Up
    01: L1/Stop         21: 5               41: ] }             61: L10/Cut
    02: Volume Down     22: 5               42: Delete          62: Num Lock
    03: L2/Again        23: 6               43: Compose         63: Left Shift
    04: Volume Up       24: 7               44: R7/KP 7         64: Z
    05: F1              25: 8               45: R8/KP 8         65: X
    06: F2              26: 9               46: R9/KP 9         66: C
    07: F10             27: 0               47: KP -            67: V
    08: F3              28: - _             48: L7/Open         68: B
    09: F11             29: = +             49: L8/Paste        69: N
    0a: F4              2a: ` ~             4a: End             6a: M
    0b: F12             2b: Backspace       4b:                 6b: , <
    0c: F5              2c: Insert          4c: Control         6c: . >
    0d: Alt Graph       2d: R4/KP =/Mute    4d: A               6d: / ?
    0e: F6              2e: R5/KP /         4e: S               6e: Right Shift
    0f: Blank           2f: R6/KP *         4f: D               6f: Line Feed/\ _
    10: F7              30: Power           50: F               70: R13/KP 1
    11: F8              31: L5/Front        51: G               71: R14/KP 2
    12: F9              32: KP .            52: H               72: R15/KP 3
    13: Alt             33: L6/Copy         53: J               73: Kakutei
    14: Cursor Up       34: Home            54: K               74: Henkan
    15: R1/Pause        35: Tab             55: L               75: Nihongo On-Off
    16: R2/Print Screen 36: Q               56: ; :             76: Help
    17: R3/Scroll Lock  37: W               57: ' "             77: Caps Lock
    18: Cursor Left     38: E               58: \ |             78: Left Meta
    19: L3/Props        39: R               59: Return          79: Space
    1a: L4/Undo         3a: T               5a: Enter           7a: Right Meta
    1b: Cursor Down     3b: Y               5b: R10/KP 4        7b: Page Down
    1c: Cursor Right    3c: U               5c: R11/KP 5        7c: < >
    1d: Escape          3d: I               5d: R12/KP 6        7d: KP +
    1e: 1               3e: O               5e: KP 0            7e:
    1f: 2               3f: P               5f: L9/Find         7f:

    7e and 7f are reserved for special messages.
    L function group and R function group repurposed on Type 4 keyboard.
    F10, F11, F12, Alt Graph, Compose, and Help added on Type 4 keyboard.
    Num Lock, KP -, KP +, KP 0, KP ., and Enter added on Type 4 keyboard.
    Line Feed removed from Type 5 keyboard.
    Cursor Up, Cursor Down, Cursor Left, and Cursor Right added on Type 5 keyboard.
    Insert, Home, End, Page Up, and Page Down added on Type 5 keyboard.
    Volume Down, Volume Up, and Power added on Type 5 keyboard.
    KP = repurposed for Mute on Type 5 keyboard.
    Blank key only present on Type 5 UNIX keyboard.
    < > only present on Type 5 International (ISO) keyboard.
    Line Feed repurposed for backslash/underscore on Type 5 Japanese (JIS) keyboard.
    Kakutei, Henkan, and Nihongo On-Off only present on Type 5 Japanese (JIS) keyboard.

    TODO:
    * Add power key line for soft power sun4c and later systems.
    * Confirm actual logic levels.
    * Dump keyboard microcontrollers.
*/

#include "emu.h"
#include "sunkbd.h"


DEFINE_DEVICE_TYPE(SUNKBD_PORT, sun_keyboard_port_device, "sunkbd", "Sun Keyboard Port")


int const device_sun_keyboard_port_interface::START_BIT_COUNT;
int const device_sun_keyboard_port_interface::DATA_BIT_COUNT;
device_serial_interface::parity_t const device_sun_keyboard_port_interface::PARITY;
device_serial_interface::stop_bits_t const device_sun_keyboard_port_interface::STOP_BITS;
int const device_sun_keyboard_port_interface::BAUD;



sun_keyboard_port_device::sun_keyboard_port_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: sun_keyboard_port_device(mconfig, SUNKBD_PORT, tag, owner, clock)
{
}


sun_keyboard_port_device::sun_keyboard_port_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_rxd(0)
	, m_rxd_handler(*this)
	, m_dev(nullptr)
{
}


sun_keyboard_port_device::~sun_keyboard_port_device()
{
}


void sun_keyboard_port_device::device_config_complete()
{
	m_dev = dynamic_cast<device_sun_keyboard_port_interface *>(get_card_device());
}


void sun_keyboard_port_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_sun_keyboard_port_interface *>(card))
	{
		osd_printf_error(
				"Card device %s (%s) does not implement device_sun_keyboard_port_interface\n",
				card->tag(),
				card->name());
	}
}


void sun_keyboard_port_device::device_resolve_objects()
{
	m_rxd_handler.resolve_safe();
}


void sun_keyboard_port_device::device_start()
{
	if (get_card_device() && !m_dev)
	{
		throw emu_fatalerror(
				"Card device %s (%s) does not implement device_sun_keyboard_port_interface\n",
				get_card_device()->tag(),
				get_card_device()->name());
	}

	save_item(NAME(m_rxd));

	m_rxd = 1;

	m_rxd_handler(m_rxd);
}


WRITE_LINE_MEMBER( sun_keyboard_port_device::write_txd )
{
	if (m_dev)
		m_dev->input_txd(state);
}



device_sun_keyboard_port_interface::device_sun_keyboard_port_interface(machine_config const &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_port(dynamic_cast<sun_keyboard_port_device *>(device.owner()))
{
}


device_sun_keyboard_port_interface::~device_sun_keyboard_port_interface()
{
}



#include "hlekbd.h"

void default_sun_keyboard_devices(device_slot_interface &device)
{
	device.option_add("type3hle",   SUN_TYPE3_HLE_KEYBOARD);
	device.option_add("type4hle",   SUN_TYPE4_HLE_KEYBOARD);
	device.option_add("type5hle",   SUN_TYPE5_HLE_KEYBOARD);
	device.option_add("type5gbhle", SUN_TYPE5_GB_HLE_KEYBOARD);
	device.option_add("type5sehle", SUN_TYPE5_SE_HLE_KEYBOARD);
	device.option_add("type5jphle", SUN_TYPE5_JP_HLE_KEYBOARD);
}
