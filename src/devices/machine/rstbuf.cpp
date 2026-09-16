// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    RST interrupt vector buffer

    The hardware circuit emulated by this device, which is generally
    made of various TTL gates and buffers, combines three independent
    interrupt request lines into a single level trigger and also
    generates a 8080-compatible RST opcode vector when the CPU
    acknowledges the interrupt, reflecting the current state of all
    three inputs.

    The positive version of this device treats all of its inputs and
    outputs as active high, in accordance with 8080 convention and
    some actual practice. An active-low IRQ would need to be supplied
    for a Z80, which may be used with this type of circuit in IM 0,
    though the negative polarity of the Z80 IRQ is not currently
    accounted for by MAME.

    Table of interrupt vectors for positive version:

        Active    Vector      Instruction    Address of
        inputs    fetched     executed       service routine
        -------   -------     -----------    ---------------
        1 only    CF          RST 1          0008
        2 only    D7          RST 2          0010
        4 only    E7          RST 4          0020
        1 and 2   DF          RST 3          0018
        1 and 4   EF          RST 5          0028
        2 and 4   F7          RST 6          0030
        1, 2, 4   FF          RST 7          0038

    An alternate version of this device buffers the logical negative
    of the request lines. This variant is primarily associated with
    Z80-based sound systems. Here the inputs, asserted singly, will
    produce a RST 18H, RST 28H or RST 30H interrupt (using Zilog
    rather than Intel syntax). When multiple request lines are active
    at once, the result is equivalent to the logical AND of the
    corresponding base vectors.

    This type of circuit features no built-in edge triggers or
    prioritization. Since its inputs are not latched, a spurious
    interrupt may occur if one or more lines are asserted just long
    enough for the CPU to accept the interrupt, but deasserted before
    the CPU's interrupt acknowledge cycle completes. In the positive
    version, the vector placed on the bus (C7) is a RST 0 instruction,
    which (after the return address is pushed onto the stack) will
    transfer control to 0000 as if the CPU had just been reset. The
    negative version provides a more useful vector (RST 7/RST 38H)
    for spurious interrupts, but will still generate RST 0 if all
    three inputs are active at once, though this situation cannot
    occur if the buffer only admits two request lines (as is
    generally the case with this version).

**********************************************************************/

#include "emu.h"
#include "rstbuf.h"

// device type definition
DEFINE_DEVICE_TYPE(RST_POS_BUFFER, rst_pos_buffer_device, "rst_pos_buffer", "RST Interrupt Buffer (positive modification)")
DEFINE_DEVICE_TYPE(RST_NEG_BUFFER, rst_neg_buffer_device, "rst_neg_buffer", "RST Interrupt Buffer (negative modification)")


//**************************************************************************
//  RST INTERRUPT BUFFER DEVICE
//**************************************************************************

//-------------------------------------------------
//  rst_buffer_device - constructor
//-------------------------------------------------

rst_buffer_device::rst_buffer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_int_cb(*this)
	, m_input_buffer(0)
{
}


//-------------------------------------------------
//  rst_pos_buffer_device - constructor
//-------------------------------------------------

rst_pos_buffer_device::rst_pos_buffer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: rst_buffer_device(mconfig, RST_POS_BUFFER, tag, owner, clock)
{
}


//-------------------------------------------------
//  rst_neg_buffer_device - constructor
//-------------------------------------------------

rst_neg_buffer_device::rst_neg_buffer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: rst_buffer_device(mconfig, RST_NEG_BUFFER, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rst_buffer_device::device_start()
{
	// save input state
	save_item(NAME(m_input_buffer));
}


//-------------------------------------------------
//  sync_set_input - synchronize asserted input
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(rst_buffer_device::sync_set_input)
{
	if (m_input_buffer == 0)
	{
		// assert interrupt when one line becomes active
		m_input_buffer = param;
		m_int_cb(1);
	}
	else
		m_input_buffer |= param;
}


//-------------------------------------------------
//  sync_clear_input - synchronize cleared input
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(rst_buffer_device::sync_clear_input)
{
	if (m_input_buffer == param)
	{
		// deassert interrupt when no more lines are active
		m_input_buffer = 0;
		m_int_cb(0);
	}
	else
		m_input_buffer &= ~param;
}


//-------------------------------------------------
//  sync_input - helper to synchronize input lines
//-------------------------------------------------

void rst_buffer_device::sync_input(bool state, u8 mask)
{
	if (state)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(rst_pos_buffer_device::sync_set_input), this), mask);
	else
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(rst_pos_buffer_device::sync_clear_input), this), mask);
}


//-------------------------------------------------
//  inta_cb - provide vector when interrupt is
//  acknowledged by CPU
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(rst_buffer_device::inta_cb)
{
	return get_vector();
}
