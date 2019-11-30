// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    SWTPC SS-50 I/O port interface

    The SS-50 bus is actually two buses in one: a 50-pin master bus
    used by RAM boards and such, and a 30-pin I/O bus derived from it
    either on the motherboard or a dedicated interface board, which
    provides each of eight "interface" cards with its specific chip
    select line and only a few address lines for register selection.
    This 30-pin bus is what is emulated here.

    As originally introduced in the 6800 Computer System, the
    interface bus only included two register select lines, just enough
    to drive one MC6820 PIA. No defined signals were placed on the
    first two lines, which cards were permitted to use for any
    purpose. The original MP-B motherboard also quite wastefully
    reserved 8K (technically 4K) of address space to map a maximum of
    32 read/write registers. The later MC6809-oriented systems not
    only reduced the I/O segment to a selectable 1K, but appropriated
    the two formerly undefined lines for two additional register
    select lines, making it possible to use more complex interface
    cards but breaking compatibility with some earlier ones.

    An unusual feature of the SS-50 bus is the presence of five baud
    rate frequencies, selected from among the (inverted) outputs of
    a MC14411 Bit Rate Generator. The 6800 Computer System provided
    only rates between 110 and 1200 (multiplied by 16), and also used
    the MC14411's master frequency to generate the CPU clocks. The
    MP-09 CPU board retained the MC14411 at first, but provided an
    independent XTAL for the MC6809 and jumpers to select higher
    baud rates. Later the MC14411 was removed from the CPU board so
    the five baud rate lines on the 50-pin bus could be reused to
    provide 20-bit addressing and DMA. This was accomplished by
    adding a separate MP-ID Interface Driver Board to decode I/O
    accesses (and optionally slow them down to allow old 1 MHz
    peripherals to be used with a faster CPU), generate the baud
    rates (whose selection was changed yet again) from a dedicated
    MC14411, and provide a few I/O functions of its own.

***********************************************************************

                        +-+
                     30 |o| RS2 (originally UD3)
                     29 |o| RS3 (originally UD4)
                     28 |o| -16V (originally -12V)
                     27 |o| +16V (originally +12V)
                     26 |o| GND
                     25 |o| GND
                     24 |o| INDEX
                     23 |o| /FIRQ (6800: /NMI)
                     22 |o| /IRQ
                     21 |o| RS0
                     20 |o| RS1
                     19 |o| D0
                     18 |o| D1
                     17 |o| D2
                     16 |o| D3
                     15 |o| D4
                     14 |o| D5
                     13 |o| D6
                     12 |o| D7
                     11 |o| E (6800: ϕ2)
                     10 |o| R/W
                      9 |o| +8V (unregulated)
                      8 |o| +8V (unregulated)
                      7 |o| 600b/1200b (originally 1200b)
                      6 |o| 4800b (originally 600b)
                      5 |o| 300b
                      4 |o| 150b/9600b (originally 150b)
                      3 |o| 110b
                      2 |o| /RESET
                      1 |o| I/O #
                        +-+

**********************************************************************/

#include "emu.h"
#include "bus/ss50/interface.h"

#include "bus/ss50/dc5.h"
#include "bus/ss50/mpc.h"
//#include "bus/ss50/mpl.h"
//#include "bus/ss50/mpr.h"
#include "bus/ss50/mps.h"
#include "bus/ss50/mps2.h"
#include "bus/ss50/mpt.h"
#include "bus/ss50/piaide.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SS50_INTERFACE, ss50_interface_port_device, "ss50_interface", "SS-50 Interface Port")

//**************************************************************************
//  SS-50 INTERFACE PORT DEVICE
//**************************************************************************

//-------------------------------------------------
//  ss50_interface_port_device - construction
//-------------------------------------------------

ss50_interface_port_device::ss50_interface_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SS50_INTERFACE, tag, owner, clock),
	device_single_card_slot_interface<ss50_card_interface>(mconfig, *this),
	m_irq_cb(*this),
	m_firq_cb(*this),
	m_card(nullptr)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void ss50_interface_port_device::device_resolve_objects()
{
	m_irq_cb.resolve_safe();
	m_firq_cb.resolve_safe();

	m_card = get_card_device();
	if (m_card != nullptr)
		m_card->m_slot = this;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ss50_interface_port_device::device_start()
{
}

//-------------------------------------------------
//  read - interface read access (pre-decoded)
//-------------------------------------------------

u8 ss50_interface_port_device::read(offs_t offset)
{
	if (m_card == nullptr)
	{
		if (!machine().side_effects_disabled())
			logerror("%s: Read from unspecified interface (RS = %X)\n", machine().describe_context(), offset);
		return 0xff;
	}

	return m_card->register_read(offset);
}

//-------------------------------------------------
//  write - interface write access (pre-decoded)
//-------------------------------------------------

void ss50_interface_port_device::write(offs_t offset, u8 data)
{
	if (m_card == nullptr)
	{
		logerror("%s: Write to unspecified interface (RS = %X, D = %02X)\n", machine().describe_context(), offset, data);
		return;
	}

	m_card->register_write(offset, data);
}

//-------------------------------------------------
//  fN_w - baud rate clocks for serial interfaces
//-------------------------------------------------

WRITE_LINE_MEMBER(ss50_interface_port_device::f110_w)
{
	if (m_card != nullptr)
		m_card->f110_w(state);
}

WRITE_LINE_MEMBER(ss50_interface_port_device::f150_9600_w)
{
	if (m_card != nullptr)
		m_card->f150_9600_w(state);
}

WRITE_LINE_MEMBER(ss50_interface_port_device::f300_w)
{
	if (m_card != nullptr)
		m_card->f300_w(state);
}

WRITE_LINE_MEMBER(ss50_interface_port_device::f600_4800_w)
{
	if (m_card != nullptr)
		m_card->f600_4800_w(state);
}

WRITE_LINE_MEMBER(ss50_interface_port_device::f600_1200_w)
{
	if (m_card != nullptr)
		m_card->f600_1200_w(state);
}

//**************************************************************************
//  SS-50 CARD INTERFACE
//**************************************************************************

template class device_finder<ss50_card_interface, false>;
template class device_finder<ss50_card_interface, true>;

//-------------------------------------------------
//  ss50_card_interface - construction
//-------------------------------------------------

ss50_card_interface::ss50_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "ss50card"),
	m_slot(nullptr)
{
}

void ss50_card_interface::interface_pre_start()
{
	if (!m_slot)
		throw device_missing_dependencies();
}

void ss50_default_2rs_devices(device_slot_interface &device)
{
	device.option_add("dc5", SS50_DC5);
	device.option_add("mpc", SS50_MPC);
	//device.option_add("mpl", SS50_MPL);
	//device.option_add("mpn", SS50_MPN);
	device.option_add("mps", SS50_MPS);
	device.option_add("mps2", SS50_MPS2);
	device.option_add("mpt", SS50_MPT);
	device.option_add("piaide", SS50_PIAIDE);
}
