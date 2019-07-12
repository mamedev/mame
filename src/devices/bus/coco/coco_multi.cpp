// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_multi.cpp

    Code for emulating CoCo's Multi-Pak Interface

     The Multi-Pak interface multiplexes all I/O lines from the Color
     Computer's expansion port to four identical ports. All I/O lines
     are continuously multiplexed except:

     Pin 36 - *SCS
     Pin 32 - *CTS
     Pin  8 - *CART

     These I/O lines are switched in one of two ways. First, is the front
     panel, four position switch. When adjusted the switch will direct the
     MPI to target these three I/O lines to the selected slot.

     Second, the MPI will listen to writes to $FF7F and respond accordingly:

     bit 0 --\___ Target *SCS to this slot number
     bit 1 --/
     bit 2 ------ Ignore
     bit 3 ------ Ignore
     bit 4 --\___ Target *CTS and *CART to this slot number
     bit 5 --/
     bit 6 ------ Ignore
     bit 7 ------ Ignore

     After writing to $FF7F, the position of the physical switch has no
     effect until reset.

     Reading is supported on $FF7F. It will reflect the position of the
     physical switch. Until data is written to $FF7F, then it will only
     reflect what has been written until a reset.

     A common modification users of the OS-9 operating system made was to
     tie all of the *CART pins together on the MPI motherboard. The *CART
     line is connected to the 6809's IRQ line. This allowed any hardware
     device in any slot to signal an IRQ to the CPU, no matter what the
     switch position was. OS-9 was designed from the very start to poll
     each device attached on every IRQ signal.

     Because of sloppy address decoding the original MPI also responds to
     $FF9F. No software is known to take advantage of this. After the
     introduction of the CoCo 3, which uses $FF9F internally, Tandy provided
     free upgrades to any MPI to fix this problem.  This behavior is not
     emulated (yet).

     Slots seem to be one-counted (i.e. - 1-4, not 0-3) by convention

***************************************************************************/

#include "emu.h"
#include "coco_multi.h"

#include "cococart.h"
#include "coco_dcmodem.h"
#include "coco_fdc.h"
#include "coco_gmc.h"
#include "coco_orch90.h"
#include "coco_pak.h"
#include "coco_rs232.h"
#include "coco_ssc.h"

#define SLOT1_TAG           "slot1"
#define SLOT2_TAG           "slot2"
#define SLOT3_TAG           "slot3"
#define SLOT4_TAG           "slot4"

#define SWITCH_CONFIG_TAG   "switch"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

static constexpr uint8_t MULTI_SLOT_LOOKUP[] = {0xcc, 0xdd, 0xee, 0xff};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_multipak_device

namespace
{
	class coco_multipak_device
		: public device_t
		, public device_cococart_interface
		, public device_cococart_host_interface
	{
	public:
		// construction/destruction
		coco_multipak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		INPUT_CHANGED_MEMBER( switch_changed );

	protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
		virtual READ8_MEMBER(cts_read) override;
		virtual WRITE8_MEMBER(cts_write) override;
		virtual READ8_MEMBER(scs_read) override;
		virtual WRITE8_MEMBER(scs_write) override;
		virtual void set_sound_enable(bool sound_enable) override;

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override;

		virtual uint8_t* get_cart_base() override;
		virtual uint32_t get_cart_size() override;

		virtual address_space &cartridge_space() override;
		virtual ioport_constructor device_input_ports() const override;

	private:
		// device references
		required_device_array<cococart_slot_device, 4> m_slots;

		// internal state
		uint8_t m_select;
		uint8_t m_block;

		// internal accessors
		int active_scs_slot_number() const;
		int active_cts_slot_number() const;
		cococart_slot_device &slot(int slot_number);
		cococart_slot_device &active_scs_slot();
		cococart_slot_device &active_cts_slot();

		// methods
		void set_select(uint8_t new_select);
		DECLARE_READ8_MEMBER(ff7f_read);
		DECLARE_WRITE8_MEMBER(ff7f_write);
		void update_line(int slot_number, line ln);

		DECLARE_WRITE_LINE_MEMBER(multi_slot1_cart_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot1_nmi_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot1_halt_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot2_cart_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot2_nmi_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot2_halt_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot3_cart_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot3_nmi_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot3_halt_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot4_cart_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot4_nmi_w);
		DECLARE_WRITE_LINE_MEMBER(multi_slot4_halt_w);
	};
};


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void coco_cart_slot1_3(device_slot_interface &device)
{
	device.option_add("rs232", COCO_RS232);
	device.option_add("dcmodem", COCO_DCMODEM);
	device.option_add("orch90", COCO_ORCH90);
	device.option_add("ssc", COCO_SSC);
	device.option_add("games_master", COCO_PAK_GMC);
	device.option_add("banked_16k", COCO_PAK_BANKED);
	device.option_add("pak", COCO_PAK);
}
static void coco_cart_slot4(device_slot_interface &device)
{
	device.option_add("cc2hdb1", COCO2_HDB1);
	device.option_add("cc3hdb1", COCO3_HDB1);
	device.option_add("fdcv11", COCO_FDC_V11);
	device.option_add("rs232", COCO_RS232);
	device.option_add("dcmodem", COCO_DCMODEM);
	device.option_add("orch90", COCO_ORCH90);
	device.option_add("ssc", COCO_SSC);
	device.option_add("games_master", COCO_PAK_GMC);
	device.option_add("banked_16k", COCO_PAK_BANKED);
	device.option_add("pak", COCO_PAK);
}


void coco_multipak_device::device_add_mconfig(machine_config &config)
{
	COCOCART_SLOT(config, m_slots[0], DERIVED_CLOCK(1, 1), coco_cart_slot1_3, nullptr);
	m_slots[0]->cart_callback().set(FUNC(coco_multipak_device::multi_slot1_cart_w));
	m_slots[0]->nmi_callback().set(FUNC(coco_multipak_device::multi_slot1_nmi_w));
	m_slots[0]->halt_callback().set(FUNC(coco_multipak_device::multi_slot1_halt_w));
	COCOCART_SLOT(config, m_slots[1], DERIVED_CLOCK(1, 1), coco_cart_slot1_3, nullptr);
	m_slots[1]->cart_callback().set(FUNC(coco_multipak_device::multi_slot2_cart_w));
	m_slots[1]->nmi_callback().set(FUNC(coco_multipak_device::multi_slot2_nmi_w));
	m_slots[1]->halt_callback().set(FUNC(coco_multipak_device::multi_slot2_halt_w));
	COCOCART_SLOT(config, m_slots[2], DERIVED_CLOCK(1, 1), coco_cart_slot1_3, nullptr);
	m_slots[2]->cart_callback().set(FUNC(coco_multipak_device::multi_slot3_cart_w));
	m_slots[2]->nmi_callback().set(FUNC(coco_multipak_device::multi_slot3_nmi_w));
	m_slots[2]->halt_callback().set(FUNC(coco_multipak_device::multi_slot3_halt_w));
	COCOCART_SLOT(config, m_slots[3], DERIVED_CLOCK(1, 1), coco_cart_slot4, "fdcv11");
	m_slots[3]->cart_callback().set(FUNC(coco_multipak_device::multi_slot4_cart_w));
	m_slots[3]->nmi_callback().set(FUNC(coco_multipak_device::multi_slot4_nmi_w));
	m_slots[3]->halt_callback().set(FUNC(coco_multipak_device::multi_slot4_halt_w));
}

INPUT_PORTS_START( coco_multipack )
	PORT_START( SWITCH_CONFIG_TAG )
	PORT_CONFNAME( 0x03, 0x03, "Multi-Pak Slot Switch" ) PORT_CHANGED_MEMBER(DEVICE_SELF, coco_multipak_device, switch_changed, nullptr)
		PORT_CONFSETTING( 0x00, "Slot 1" )
		PORT_CONFSETTING( 0x01, "Slot 2" )
		PORT_CONFSETTING( 0x02, "Slot 3" )
		PORT_CONFSETTING( 0x03, "Slot 4" )
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_MULTIPAK, device_cococart_interface, coco_multipak_device, "coco_multipack", "CoCo Multi-Pak Interface")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor coco_multipak_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( coco_multipack );
}

//-------------------------------------------------
//  coco_multipak_device - constructor
//-------------------------------------------------

coco_multipak_device::coco_multipak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, COCO_MULTIPAK, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_slots(*this, "slot%u", 1), m_select(0), m_block(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_multipak_device::device_start()
{
	// install $FF7F handler
	install_readwrite_handler(0xFF7F, 0xFF7F, read8_delegate(FUNC(coco_multipak_device::ff7f_read), this),write8_delegate(FUNC(coco_multipak_device::ff7f_write), this));

	// initial state
	m_select = 0xFF;
	m_block = 0;

	// save state
	save_item(NAME(m_select));
	save_item(NAME(m_block));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_multipak_device::device_reset()
{
	set_select(MULTI_SLOT_LOOKUP[ioport(SWITCH_CONFIG_TAG)->read()]);
	m_block = 0;
}


//-------------------------------------------------
//  switch_changed - panel switch changed
//-------------------------------------------------

INPUT_CHANGED_MEMBER( coco_multipak_device::switch_changed )
{
	if (m_block == 0)
		set_select(MULTI_SLOT_LOOKUP[newval]);
}


//**************************************************************************
//  INTERNAL ACCESSORS
//**************************************************************************

//-------------------------------------------------
//  active_scs_slot_number
//-------------------------------------------------

int coco_multipak_device::active_scs_slot_number() const
{
	return ((m_select >> 0) & 0x03) + 1;
}


//-------------------------------------------------
//  active_cts_slot_number
//-------------------------------------------------

int coco_multipak_device::active_cts_slot_number() const
{
	return ((m_select >> 4) & 0x03) + 1;
}


//-------------------------------------------------
//  slot
//-------------------------------------------------

cococart_slot_device &coco_multipak_device::slot(int slot_number)
{
	assert(slot_number >= 1 && slot_number <= 4);
	return *m_slots[slot_number - 1];
}


//-------------------------------------------------
//  active_scs_slot
//-------------------------------------------------

cococart_slot_device &coco_multipak_device::active_scs_slot()
{
	int slot_number = active_scs_slot_number();
	return slot(slot_number);
}


//-------------------------------------------------
//  active_cts_slot
//-------------------------------------------------

cococart_slot_device &coco_multipak_device::active_cts_slot()
{
	int slot_number = active_cts_slot_number();
	return slot(slot_number);
}


//**************************************************************************
//  METHODS
//**************************************************************************

//-------------------------------------------------
//  set_select
//-------------------------------------------------

void coco_multipak_device::set_select(uint8_t new_select)
{
	// identify old value for CART, in case this needs to change
	cococart_slot_device::line_value old_cart = active_cts_slot().get_line_value(line::CART);

	// change value
	m_select = new_select;

	// did the CART line change?
	line_value new_cart = active_cts_slot().get_line_value(line::CART);
	if (new_cart != old_cart)
		update_line(active_cts_slot_number(), line::CART);

	cart_base_changed();
}


//-------------------------------------------------
//  ff7f_read
//-------------------------------------------------

READ8_MEMBER(coco_multipak_device::ff7f_read)
{
	return m_select | 0xcc;
}

//-------------------------------------------------
//  ff7f_write
//-------------------------------------------------

WRITE8_MEMBER(coco_multipak_device::ff7f_write)
{
	m_block = 0xff;
	set_select(data);
}


//-------------------------------------------------
//  update_line
//-------------------------------------------------

void coco_multipak_device::update_line(int slot_number, line ln)
{
	bool propagate;

	// one of our child devices set a line; we may need to propagate it upwards
	switch (ln)
	{
	case line::CART:
		// only propagate if this is coming from the slot specified
		propagate = slot_number == active_cts_slot_number();
		break;

	case line::NMI:
	case line::HALT:
		// always propagate these
		propagate = true;
		break;

	default:
		// do nothing
		propagate = false;
		break;
	}

	if (propagate)
		owning_slot().set_line_value(ln, slot(slot_number).get_line_value(ln));
}


//-------------------------------------------------
//  set_sound_enable
//-------------------------------------------------

void coco_multipak_device::set_sound_enable(bool sound_enable)
{
	// the SOUND_ENABLE (SNDEN) line is different; it is controlled by the CPU
	for (cococart_slot_device *slot : m_slots)
		slot->set_line_value(line::SOUND_ENABLE, sound_enable ? line_value::ASSERT : line_value::CLEAR);
}


//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

uint8_t* coco_multipak_device::get_cart_base()
{
	return active_cts_slot().get_cart_base();
}


//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

uint32_t coco_multipak_device::get_cart_size()
{
	return active_cts_slot().get_cart_size();
}


//-------------------------------------------------
//  cts_read
//-------------------------------------------------

READ8_MEMBER(coco_multipak_device::cts_read)
{
	return active_cts_slot().cts_read(space, offset);
}


//-------------------------------------------------
//  cts_write
//-------------------------------------------------

WRITE8_MEMBER(coco_multipak_device::cts_write)
{
	active_cts_slot().cts_write(space, offset, data);
}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

READ8_MEMBER(coco_multipak_device::scs_read)
{
	return active_scs_slot().scs_read(space, offset);
}


//-------------------------------------------------
//  scs_write
//-------------------------------------------------

WRITE8_MEMBER(coco_multipak_device::scs_write)
{
	active_scs_slot().scs_write(space, offset, data);
}


//-------------------------------------------------
//  multiX_slotX_[cart|nmi|halt] trampolines
//-------------------------------------------------

WRITE_LINE_MEMBER(coco_multipak_device::multi_slot1_cart_w) { update_line(1, line::CART); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot1_nmi_w)  { update_line(1, line::NMI); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot1_halt_w) { update_line(1, line::HALT); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot2_cart_w) { update_line(2, line::CART); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot2_nmi_w)  { update_line(2, line::NMI); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot2_halt_w) { update_line(2, line::HALT); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot3_cart_w) { update_line(3, line::CART); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot3_nmi_w)  { update_line(3, line::NMI); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot3_halt_w) { update_line(3, line::HALT); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot4_cart_w) { update_line(4, line::CART); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot4_nmi_w)  { update_line(4, line::NMI); }
WRITE_LINE_MEMBER(coco_multipak_device::multi_slot4_halt_w) { update_line(4, line::HALT); }


//-------------------------------------------------
//  cartridge_space
//-------------------------------------------------

address_space &coco_multipak_device::cartridge_space()
{
	return device_cococart_interface::cartridge_space();
}
