// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  a2bus.c - Apple II slot bus and card emulation

  by R. Belmont

  Pinout (/ indicates an inverted signal, ie, one that would have a bar over it
          on a schematic diagram)

      (rear of computer)

     GND  26  25  +5V
  DMA IN  27  24  DMA OUT
  INT IN  28  23  INT OUT
    /NMI  29  22  /DMA
    /IRQ  30  21  RDY
    /RES  31  20  /IOSTB
    /INH  32  19  N.C.
    -12V  33  18  R/W
     -5V  34  17  A15
    CREF  35  16  A14
      7M  36  15  A13
      Q3  37  14  A12
     PH1  38  13  A11
  USER 1  39  12  A10
     PH0  40  11  A9
 /DEVSEL  41  10  A8
      D7  42   9  A7
      D6  43   8  A6
      D5  44   7  A5
      D4  45   6  A4
      D3  46   5  A3
      D2  47   4  A2
      D1  48   3  A1
      D0  49   2  A0
    -12V  50   1  /IOSEL

     (front of computer)

    Signal descriptions:
    GND - power supply ground
    DMA IN - daisy chain of DMA signal from higher priority devices.  usually connected to DMA OUT.
    INT IN - similar to DMA IN but for INT instead of DMA.
    /NMI - non-maskable interrupt input to the 6502
    /IRQ - maskable interrupt input to the 6502
    /RES - system reset signal
    /INH - On the II and II+, inhibits the motherboard ROMs, allowing a card to replace them.
           On the IIe, inhibits all motherboard RAM/ROM for both CPU and DMA accesses.
           On the IIgs, works like the IIe except for the address range 0x6000 to 0x9FFF where
           it will cause bus contention.
    -12V - negative 12 volts DC power
     -5V - negative 5 volts DC power
    CREF - 3.58 MHz color reference signal, only present on slot 7 of PAL models.
           IIgs redefines this exclusively for slot 3 as M2B0, Mega II bank 0 select.
      7M - 7 MHz clock (1/4th of the master clock on the IIgs, 1/2 on 8-bit IIs)
      Q3 - 2 MHz asymmetrical clock
     PH1 - 6502 phase 1 clock
  USER 1 - On the II and II+, this officially unspecified signal is tied to an unmarked jumper
           pad that, if connected, disables /DEVSEL and /IOSEL for all slots when pulled low.
           On the IIe, this is µPSYNC, output high by the 6502 during opcode fetch cycles.
           On the IIgs, this is /M2SEL, output low when the 128K Mega II RAM area is accessed.
     PH0 - 6502 phase 0 clock.  The phase 2 or E clock input required by 6800 or 6500 family
           peripherals must be regenerated from this signal using additional logic.
 /DEVSEL - asserted on an access to C0nX, where n = the slot number plus 8.
   D0-D7 - 8-bit data bus
     +5V - 5 volts DC power
 DMA OUT - see DMA IN
 INT OUT - see INT IN
    /DMA - pulling this low disconnects the 6502 from the bus and halts it
     RDY - 6502 RDY input.  Pulling this low when PH1 is active will halt the
           6502 and latch the current address bus value.
  /IOSTB - asserted on an access between C800 and CFFF.  To avoid contention,
           all cards should relinquish the use of this space upon any access
           to CFFF.
  A0-A15 - 16-bit address bus
  /IOSEL - asserted on accesses to CnXX where n is the slot number.
           Not present on slot 0.

  Though most card firmwares are programmed to work in any slot from 1 to 7,
  Apple II operating systems often associate slot numbers with particular
  interface types.  Some of the conventional patterns are as follows:

  - Slot 0 is reserved for a "language" RAM or firmware card (mapping its
    memory by asserting /INH rather than using /IOSEL and /IOSTB).
  - Slot 1 is standard for parallel or serial printer interfaces.
  - Slot 2 can be used for generic parallel or serial I/O.
  - Slot 3 is generally used by 80-column display interfaces.
  - Slot 4 or Slot 7 may be used by Z80 coprocessor cards.
  - Slot 5 may be a secondary disk controller.
  - Slot 6 is normally occupied by the primary bootable disk controller.

***************************************************************************/

#include "emu.h"
#include "a2bus.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_SLOT, a2bus_slot_device, "a2bus_slot", "Apple II Slot")

template class device_finder<device_a2bus_card_interface, false>;
template class device_finder<device_a2bus_card_interface, true>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2bus_slot_device - constructor
//-------------------------------------------------
a2bus_slot_device::a2bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a2bus_slot_device(mconfig, A2BUS_SLOT, tag, owner, clock)
{
}

a2bus_slot_device::a2bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface<device_a2bus_card_interface>(mconfig, *this)
	, m_a2bus(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_slot_device::device_resolve_objects()
{
	device_a2bus_card_interface *const a2bus_card = get_card_device();
	if (a2bus_card)
		a2bus_card->set_a2bus(m_a2bus, tag());
}

void a2bus_slot_device::device_start()
{
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS, a2bus_device, "a2bus", "Apple II Bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2bus_device - constructor
//-------------------------------------------------

a2bus_device::a2bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a2bus_device(mconfig, A2BUS, tag, owner, clock)
{
}

a2bus_device::a2bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_maincpu_space(*this, finder_base::DUMMY_TAG, -1)
	, m_out_irq_cb(*this)
	, m_out_nmi_cb(*this)
	, m_out_inh_cb(*this)
	, m_out_dma_cb(*this)
	, m_slot_irq_mask(0), m_slot_nmi_mask(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_device::device_start()
{
	// clear slots
	std::fill(std::begin(m_device_list), std::end(m_device_list), nullptr);

	m_slot_irq_mask = m_slot_nmi_mask = 0;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void a2bus_device::device_reset()
{
}

device_a2bus_card_interface *a2bus_device::get_a2bus_card(int slot)
{
	if (slot < 0)
	{
		return nullptr;
	}

	if (m_device_list[slot])
	{
		return m_device_list[slot];
	}

	return nullptr;
}

void a2bus_device::add_a2bus_card(int slot, device_a2bus_card_interface *card)
{
	m_device_list[slot] = card;
}

void a2bus_device::reset_bus()
{
	for (int slot = 0; slot <= 7; slot++)
	{
		auto card = get_a2bus_card(slot);
		if (card != nullptr)
		{
			card->reset_from_bus();
		}
	}
}

uint8_t a2bus_device::get_a2bus_irq_mask()
{
	return m_slot_irq_mask;
}

uint8_t a2bus_device::get_a2bus_nmi_mask()
{
	return m_slot_nmi_mask;
}

void a2bus_device::set_irq_line(int state, int slot)
{
	if (state == CLEAR_LINE)
	{
		m_slot_irq_mask &= ~(1<<slot);
	}
	else if (state == ASSERT_LINE)
	{
		m_slot_irq_mask |= (1<<slot);
	}

	m_out_irq_cb(state);
}

void a2bus_device::set_nmi_line(int state, int slot)
{
	m_out_nmi_cb(state);

	if (state == CLEAR_LINE)
	{
		m_slot_nmi_mask &= ~(1<<slot);
	}
	else if (state == ASSERT_LINE)
	{
		m_slot_nmi_mask |= (1<<slot);
	}
}

void a2bus_device::set_dma_line(int state)
{
	m_out_dma_cb(state);
}

uint8_t a2bus_device::dma_r(uint16_t offset)
{
	return m_maincpu_space->read_byte(offset);
}

void a2bus_device::dma_w(uint16_t offset, uint8_t data)
{
	m_maincpu_space->write_byte(offset, data);
}

void a2bus_device::recalc_inh(int slot)
{
	m_out_inh_cb(ASSERT_LINE);
	m_out_inh_cb(CLEAR_LINE);
}

// interrupt request from a2bus card
void a2bus_device::irq_w(int state) { m_out_irq_cb(state); }
void a2bus_device::nmi_w(int state) { m_out_nmi_cb(state); }

//**************************************************************************
//  DEVICE CONFIG A2BUS CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE A2BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_a2bus_card_interface - constructor
//-------------------------------------------------

device_a2bus_card_interface::device_a2bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "a2bus")
	, m_a2bus_finder(device, finder_base::DUMMY_TAG), m_a2bus(nullptr)
	, m_a2bus_slottag(nullptr), m_slot(-1), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_a2bus_card_interface - destructor
//-------------------------------------------------

device_a2bus_card_interface::~device_a2bus_card_interface()
{
}

void device_a2bus_card_interface::interface_validity_check(validity_checker &valid) const
{
	if (m_a2bus_finder && m_a2bus && (m_a2bus != m_a2bus_finder))
		osd_printf_error("Contradictory buses configured (%s and %s)\n", m_a2bus_finder->tag(), m_a2bus->tag());
}

void device_a2bus_card_interface::interface_pre_start()
{
	if (!m_a2bus)
	{
		m_a2bus = m_a2bus_finder;
		if (!m_a2bus)
			fatalerror("Can't find Apple II Bus device %s\n", m_a2bus_finder.finder_tag());
	}

	if (0 > m_slot)
	{
		if (!m_a2bus->started())
			throw device_missing_dependencies();

		// extract the slot number from the last digit of the slot tag
		size_t const tlen = strlen(m_a2bus_slottag);

		m_slot = (m_a2bus_slottag[tlen - 1] - '0');
		if (m_slot < 0 || m_slot > 7)
			fatalerror("Slot %x out of range for Apple II Bus\n", m_slot);

		m_a2bus->add_a2bus_card(m_slot, this);
	}
}
