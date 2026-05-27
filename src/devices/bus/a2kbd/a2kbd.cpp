// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Apple II Keyboard Connector

    The Apple II receives parallel input from its keyboard through a
    16-pin DIP socket, much like the Apple I and several other early
    8-bit microcomputer systems (whose specific connector pinouts
    were almost never compatible with each other).

    Keypresses are translated into 7-bit ASCII codes placed on B1–B7
    (positive-true TTL levels). The setting of an eighth bit is
    triggered by the rising edge of the STROBE output, which
    notifies the host that a new keycode is present.

    The keyboard is also connected to the system's master RESET
    signal (active low), and should assert it when one or more
    special keys is pressed. In theory this line is bidirectional,
    but the motherboard and slot cards normally drive it only
    through power-up reset circuits (which were missing in the
    earliest Apple IIs).

    The state of the keyboard's Shift key(s) is not normally exposed
    on any of the connector pins. However, a widely installed
    hardware modification ran a wire from it to one of the Game I/O-
    related inputs of the LS251 multiplexer on the motherboard, most
    commonly SW2 (which became the standard location for polling the
    shift key on the Apple IIe and later models). Software that
    expects this modification to be installed (including some
    80-column card firmware) will convert uppercase letters to
    lowercase when they sense that Shift is not pressed and convert
    ], ^ and @ back to (uppercase) M, N and P when Shift is pressed.
    Some Apple II word processors also expect the Control key to be
    connected to another LS251 input.

    With the dubious exception of RESET, there are no outputs from
    the main unit to the keyboard... as standard. Videx's Enhancer ][
    appropriated two unused pins to carry the positive output of the
    key strobe flip-flop and one of the annunciators back to the
    keyboard to improve its efficiency and functionality.

    This parallel keyboard interface is specific to the Apple II,
    Apple II+ and some clones. Though the keyboard encoders used in
    the Apple III and in the Apple IIe & IIc are from the same
    family as that used in the newer II/II+ keyboard, they reside on
    the motherboard and cannot be easily replaced, and the actual
    keyboard connector on those systems carries only strobes and
    return lines.

                             ┌─────────┐
                      +5V  1 │•        │ 16  N.C.
                   STROBE  2 │         │ 15  −12V
                    RESET  3 │         │ 14  N.C.
                    (AN3)  4 │         │ 13  B2
                       B6  5 │         │ 12  B1
                       B5  6 │         │ 11  B4
                       B7  7 │         │ 10  B3
                      GND  8 │         │ 9   (ACK)
                             └─────────┘

*********************************************************************/

#include "emu.h"
#include "a2kbd.h"

#include "am100kbd.h"
#include "ivelultrkb.h"
#include "kb200.h"
#include "nkbd.h"
#include "tk10.h"
#include "videnh2.h"


//**************************************************************************
//  KEYBOARD CONNECTOR DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(A2KBD_CONNECTOR, a2kbd_connector_device, "a2kbd_connector", "Apple II Keyboard Connector")

a2kbd_connector_device::a2kbd_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A2KBD_CONNECTOR, tag, owner, clock)
	, device_single_card_slot_interface<device_a2kbd_interface>(mconfig, *this)
	, m_b_callback(*this)
	, m_strobe_callback(*this)
	, m_reset_callback(*this)
	, m_mode_callback(*this)
{
}

void a2kbd_connector_device::default_options(device_slot_interface &slot)
{
	//slot.option_add("okbd", APPLE2_OKBD);
	slot.option_add("nkbd", APPLE2_NKBD);
	slot.option_add("am100", AM100_KEYBOARD);
	slot.option_add("ivelultr", IVEL_ULTRA_KEYBOARD);
	slot.option_add("kb200", A2KBD_KB200);
	slot.option_add("tk10", A2KBD_TK10);
	slot.option_add("videnh2", A2KBD_VIDENH2);
	slot.option_add("uniap2ti", A2KBD_UNIAP2TI);
}

void a2kbd_connector_device::device_config_complete()
{
	m_intf = get_card_device();
}

void a2kbd_connector_device::device_resolve_objects()
{
	if (m_intf)
		m_intf->m_connector = this;
}

void a2kbd_connector_device::device_start()
{
}


//**************************************************************************
//  KEYBOARD DEVICE INTERFACE
//**************************************************************************

device_a2kbd_interface::device_a2kbd_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "a2kbd")
	, m_connector(nullptr)
{
}

device_a2kbd_interface::~device_a2kbd_interface()
{
}
