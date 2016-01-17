// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    NCR Decision mate slot bus and module emulation

    From the NCR System Technical manual:

    "The lower part of the controller board contains the I/0 bus together with
    seven user-accessible connectors. These connectors are identified on the board
    as J1 through J7 (reading from left to right), and correspond to the seven slots
    (numbers 1 through 7) at the rear of the cabinet.
    Position J1 is reserved for the connection of any one of the memory expansion modules
    (K200, K202, K208).
    Position J7 is reserved for the connection of either the diagnostic module (K220),
    or the customer-installable 16-bit processor module (K231).
    Positions J2 through J6 are the general purpose slots for the connection of the peripheral
    adapter and other interfaces to the computer. These five positions are identical, and Figure 2.2
    defines the type of connector, while the pin assignments are shown in Figure 2.3."

    Pinout (/ indicates an inverted signal, ie, one that would have a bar over it
              on a schematic diagram)

                                               a        c
                                       |-------------------------|
                                       |+5V        1    +5 V     |
                                       |                         |
                                       |           2    +12 V    |
                                       |                         |
        System Reset Output, act. low  |RESET/     3    RESET IN/|  active low, general system RESET
                                       |                         |
        I/O Write                      |IOW/       4    IOR/     |  I/O read    R/W lines are Processor control lines
                                       |                         |
        Memory Write                   |MEMW/      5    MEMR/    |  Memory read active low, Tri-State possible
                                       |                         |
        BD0-BD7: Switch by IFSEL       |BD1        6    BDO      |  BD0 - BD7: Data-Bus lines (8 bit)
        (I/O-Read). Normal Output.     |                         |  bidirectional, active high
                                       |BD3        7    BD2      |
                                       |                         |
        Direction can be changed       |BD5        8    BD4      |  Bus-Driver to peripheral Bus (LS245)
        by DIR/ signal.                |                         |  automatic detection
                                       |BD7        9    BD6      |
                                       |                         |
        Ready Signal from the          |READY DMA 10    ABTRI /  |  Address Bus Tri-State, active low signal
        mem. contr. (XACK), act. hi.   |                         |
        End of Process-EOP signals that|EOP/      11             |  cf. 8234A-6 spec.
        DMA service has been completed |                         |
                                       |INTACK/   12    IFSEL 4/ |
                                       |                         |
        Change peripheral from board   |AUTO/     13    DIR/     |  Direction of the databus driver. Low signal
        type 1 to 2.NC on type 1       |                         |  change to input.
        Test Hold. External request    |THOLD/    14    HLDA     |  Hold Acknowledge. A response from the Z80
        to set the Z80 in hold state.  |                         |  The Z80 CPU is in hold state, active high.
        Processor-Clock: inverse signal|PCLK/     15    CLK1     |  Clock Output 1 MHz
        of the CPU clock, freq. 4MHz   |                         |
        Logic Ground                   |LGRD      16    TRAMD/   |  Test RAM-Disable. For ext. ROM or RAM expansion.
                                       |                         |  Switching with the System RAM, RAM output disabled. act. low
                                       |BA19      17    BA18     |  BA0-BA19: Buffered 20bit Address Bus
                                       |                         |
                                       |BA17      18    BA16     |
                                       |                         |
                                       |BA15      19    BA14     |  Range to 1MB Normal output, active high
                                       |                         |
                                       |BA13      20    BA12     |
                                       |                         |
                                       |BA11      21    BA10     |  Tri-State possible with ABTRI/ signal
                                       |                         |
                                       |BA9       22    BA8      |
                                       |                         |
                                       |BA7       23    BA6      |
                                       |                         |
                                       |BA5       24    BA4      |
                                       |                         |
                                       |BA3       25    BA2      |
                                       |                         |
                                       |BA1       26    BAO      |
                                       |                         |
        IFSEL 0-4, active low          |IFSEL3/   27    IFSEL2/  |  The select of the I/O pprts in the peripherals is made
        The interface does not need    |                         |  by any IFSEL and BA3 (BA3/). (10 peripherals). Automatic
        own address decoder            |IFSEL1/   28    IFSELO/  |  change of the data bus direction. This change is not possible
                                       |                         |  while a dma function is performed
        DRQ0-DRQ1: DMA Request for     |DRQ1      29    DRQ0     |  Asynchr. channel requests are used by peripherals
        resp. channels, act. high      |                         |  to request DMA service
        DACK0-DACK1:                   |DACK1 /   30    DACK0/   |  DMA-Acknowledge Channels 0 and 1, active low
                                       |                         |  These lines indicate an active DMA channel
                                       |WAIT/     31    INT/     |
                                       |                         |
        Logic Ground                   |LGRD      32    LGRD     |  Logic Ground
                                       |-------------------------|

    Two additional plug/socket connections are possible on the bus. These are made on the solder side of the controller board
    and are designated J2A and J7 A. These connectors are not considered to be user accessible , rather for factory use ,
    or for use by field engineers and system integrators. Normally, these two connectors are used for:
    ??? J2A - The connection of the fixed disk interface board, or a custom design board.
    ??? J7A - The connection of the 16-bit processor board (factory option and kit K230).

    The processor and diagnostics module for Slots J7 and J7A have the following additional signals in addition to the ones present on Slots J2-J6.
    Note that the middle row of the connector (row "b") carries some signals too:

    A2 - OPT 2
    Signals A19-A26 and C19-C26 are called e.g. A7 instead of BA7 in the processor module schematic (fig. 2.14, page 2.21 System Technical Manual Hardware)
    Signals A6-A9 and C6-D9 are called e.g. D4 instead of BD4 in the processor module schematic
    B10 - READYP
    B11 - HOLD
    B12 - SWITCH 16/
    B13 - HLDA 16
    B14 - 16 BITAV
    B15 - STDMARQ
    B16 - LGRD
    B17 - 16 BITSET/
    B20 - MEMRQ/
    B32 - LGRD

    The memory modules on J1 share the same physical connector with the other expansion modules, but carry different signals:

    A1 - +5V            C1 - +5V
    A2                  C2
    A3 - AOUT7          C3
    A4 - AOUT6          C4 - AOUT5
    A5 - AOUT4          C5 - AOUT3
    A6                  C6
    A7                  C7
    A8                  C8
    A9                  C9
    A10                 C10 - AOUT2
    A11                 C11 - AOUT1
    A12 - OD0           C12 - AOUT0
    A13 - OD1           C13 - OD2
    A14 - OD3           C14
    A15                 C15
    A16 - LGRD          C16 - LGRD
    A17                 C17
    A18                 C18
    A19 - ID0           C19
    A20 - ID1           C20
    A21 - ID2           C21
    A22 - ID3           C22
    A23 - ID4           C23 - CAS7/
    A24 - ID5           C24 - CAS6/
    A25 - ID6           C25 - CAS5/
    A26 - ID7           C26 - CAS4/
    A27 - CAS2/         C27 - CAS1/
    A28 - CAS3/         C28
    A29 - OD4           C29 - OD5
    A30 - RAS/          C30 - WE/
    A31 - OD6           C31 - OD7
    A32 - LGRD          C32 - LGRD


***************************************************************************/


#include "emu.h"
#include "dmvbus.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type DMVCART_SLOT = &device_creator<dmvcart_slot_device>;

//**************************************************************************
//    DMV cartridge interface
//**************************************************************************

//-------------------------------------------------
//  device_dmvslot_interface - constructor
//-------------------------------------------------

device_dmvslot_interface::device_dmvslot_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_dmvslot_interface - destructor
//-------------------------------------------------

device_dmvslot_interface::~device_dmvslot_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmvcart_slot_device - constructor
//-------------------------------------------------
dmvcart_slot_device::dmvcart_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, DMVCART_SLOT, "Decision Mate V cartridge slot", tag, owner, clock, "dmvcart_slot", __FILE__),
		device_slot_interface(mconfig, *this),
		m_prog_read_cb(*this),
		m_prog_write_cb(*this),
		m_out_int_cb(*this),
		m_out_irq_cb(*this),
		m_out_thold_cb(*this), m_cart(nullptr)
{
}


//-------------------------------------------------
//  dmvcart_slot_device - destructor
//-------------------------------------------------

dmvcart_slot_device::~dmvcart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmvcart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_dmvslot_interface *>(get_card_device());

	// resolve callbacks
	m_prog_read_cb.resolve_safe(0);
	m_prog_write_cb.resolve_safe();
	m_out_int_cb.resolve_safe();
	m_out_irq_cb.resolve_safe();
	m_out_thold_cb.resolve_safe();
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

bool dmvcart_slot_device::read(offs_t offset, UINT8 &data)
{
	if (m_cart)
		return m_cart->read(offset, data);
	return false;
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

bool dmvcart_slot_device::write(offs_t offset, UINT8 data)
{
	if (m_cart)
		return m_cart->write(offset, data);
	return false;
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void dmvcart_slot_device::ram_read(UINT8 cas, offs_t offset, UINT8 &data)
{
	if (m_cart)
		m_cart->ram_read(cas, offset, data);
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void dmvcart_slot_device::ram_write(UINT8 cas, offs_t offset, UINT8 data)
{
	if (m_cart)
		return m_cart->ram_write(cas, offset, data);
}

/*-------------------------------------------------
    IO read
-------------------------------------------------*/

void dmvcart_slot_device::io_read(address_space &space, int ifsel, offs_t offset, UINT8 &data)
{
	if (m_cart)
		m_cart->io_read(space, ifsel, offset, data);
}


/*-------------------------------------------------
   IO write
-------------------------------------------------*/

void dmvcart_slot_device::io_write(address_space &space, int ifsel, offs_t offset, UINT8 data)
{
	if (m_cart)
		m_cart->io_write(space, ifsel, offset, data);
}

/*-------------------------------------------------
    av16bit
-------------------------------------------------*/

bool dmvcart_slot_device::av16bit()
{
	if (m_cart)
		return m_cart->av16bit();
	return  false;
}

/*-------------------------------------------------
    hold_w
-------------------------------------------------*/

void dmvcart_slot_device::hold_w(int state)
{
	if (m_cart)
		m_cart->hold_w(state);
}

void dmvcart_slot_device::switch16_w(int state)
{
	if (m_cart)
		m_cart->switch16_w(state);
}

void dmvcart_slot_device::timint_w(int state)
{
	if (m_cart)
		m_cart->timint_w(state);
}

void dmvcart_slot_device::keyint_w(int state)
{
	if (m_cart)
		m_cart->keyint_w(state);
}

void dmvcart_slot_device::busint_w(int state)
{
	if (m_cart)
		m_cart->busint_w(state);
}

void dmvcart_slot_device::flexint_w(int state)
{
	if (m_cart)
		m_cart->flexint_w(state);
}

void dmvcart_slot_device::irq2_w(int state)
{
	if (m_cart)
		m_cart->irq2_w(state);
}

void dmvcart_slot_device::irq2a_w(int state)
{
	if (m_cart)
		m_cart->irq2a_w(state);
}

void dmvcart_slot_device::irq3_w(int state)
{
	if (m_cart)
		m_cart->irq3_w(state);
}

void dmvcart_slot_device::irq4_w(int state)
{
	if (m_cart)
		m_cart->irq4_w(state);
}

void dmvcart_slot_device::irq5_w(int state)
{
	if (m_cart)
		m_cart->irq5_w(state);
}

void dmvcart_slot_device::irq6_w(int state)
{
	if (m_cart)
		m_cart->irq6_w(state);
}
