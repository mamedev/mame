// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_multi.c

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

     Second, the MPI will listen to writes to 0xff7f and respond accordingly:

     bit 0 --\___ Target *SCS to this slot number
     bit 1 --/
     bit 2 ------ Ignore
     bit 3 ------ Ignore
     bit 4 --\___ Target *CTS and *CART to this slot number
     bit 5 --/
     bit 6 ------ Ignore
     bit 7 ------ Ignore

     After writing to 0xff7f, the position of the physical switch has no
     effect until reset.

     Reading is supported on 0xff7f. It will reflect the position of the
     physical switch. Until data is written to 0xff7f, then it will only
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
     free upgrades to any MPI to fix this problem.


***************************************************************************/

#include "emu.h"
#include "includes/coco.h"
#include "coco_multi.h"
#include "coco_232.h"
#include "coco_orch90.h"
#include "coco_pak.h"
#include "coco_fdc.h"

#define SLOT1_TAG           "slot1"
#define SLOT2_TAG           "slot2"
#define SLOT3_TAG           "slot3"
#define SLOT4_TAG           "slot4"



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static SLOT_INTERFACE_START(coco_cart_slot1_3)
	SLOT_INTERFACE("rs232", COCO_232)
	SLOT_INTERFACE("orch90", COCO_ORCH90)
	SLOT_INTERFACE("banked_16k", COCO_PAK_BANKED)
	SLOT_INTERFACE("pak", COCO_PAK)
SLOT_INTERFACE_END
static SLOT_INTERFACE_START(coco_cart_slot4)
	SLOT_INTERFACE("cc3hdb1", COCO3_HDB1)
	SLOT_INTERFACE("fdcv11", COCO_FDC_V11)
	SLOT_INTERFACE("rs232", COCO_232)
	SLOT_INTERFACE("orch90", COCO_ORCH90)
	SLOT_INTERFACE("banked_16k", COCO_PAK_BANKED)
	SLOT_INTERFACE("pak", COCO_PAK)
SLOT_INTERFACE_END

WRITE_LINE_MEMBER(coco_multipak_device::multi_cart_w)
{
	cococart_slot_device *cart = dynamic_cast<cococart_slot_device *>(owner());
	cart->m_cart_callback(state);
}

WRITE_LINE_MEMBER(coco_multipak_device::multi_nmi_w)
{
	cococart_slot_device *cart = dynamic_cast<cococart_slot_device *>(owner());
	cart->m_nmi_callback(state);
}

WRITE_LINE_MEMBER(coco_multipak_device::multi_halt_w)
{
	cococart_slot_device *cart = dynamic_cast<cococart_slot_device *>(owner());
	cart->m_halt_callback(state);
}

static MACHINE_CONFIG_FRAGMENT(coco_multi)
	MCFG_COCO_CARTRIDGE_ADD(SLOT1_TAG, coco_cart_slot1_3, NULL)
	MCFG_COCO_CARTRIDGE_CART_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_nmi_w))
	MCFG_COCO_CARTRIDGE_HALT_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_halt_w))
	MCFG_COCO_CARTRIDGE_ADD(SLOT2_TAG, coco_cart_slot1_3, NULL)
	MCFG_COCO_CARTRIDGE_CART_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_nmi_w))
	MCFG_COCO_CARTRIDGE_HALT_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_halt_w))
	MCFG_COCO_CARTRIDGE_ADD(SLOT3_TAG, coco_cart_slot1_3, NULL)
	MCFG_COCO_CARTRIDGE_CART_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_nmi_w))
	MCFG_COCO_CARTRIDGE_HALT_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_halt_w))
	MCFG_COCO_CARTRIDGE_ADD(SLOT4_TAG, coco_cart_slot4, "fdcv11")
	MCFG_COCO_CARTRIDGE_CART_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_nmi_w))
	MCFG_COCO_CARTRIDGE_HALT_CB(DEVWRITELINE(DEVICE_SELF, coco_multipak_device, multi_halt_w))
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type COCO_MULTIPAK = &device_creator<coco_multipak_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_multipak_device - constructor
//-------------------------------------------------

coco_multipak_device::coco_multipak_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, COCO_MULTIPAK, "CoCo Multi-Pak Interface", tag, owner, clock, "coco_multipak", __FILE__),
		device_cococart_interface( mconfig, *this )
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_multipak_device::device_start()
{
	// identify slots
	m_slots[0] = dynamic_cast<cococart_slot_device *>(subdevice(SLOT1_TAG));
	m_slots[1] = dynamic_cast<cococart_slot_device *>(subdevice(SLOT2_TAG));
	m_slots[2] = dynamic_cast<cococart_slot_device *>(subdevice(SLOT3_TAG));
	m_slots[3] = dynamic_cast<cococart_slot_device *>(subdevice(SLOT4_TAG));
	m_owner = dynamic_cast<cococart_slot_device *>(owner());

	// install $FF7F handler
	write8_delegate wh = write8_delegate(FUNC(coco_multipak_device::ff7f_write), this);
	machine().device(MAINCPU_TAG)->memory().space(AS_PROGRAM).install_write_handler(0xFF7F, 0xFF7F, wh);

	// initial state
	m_select = 0xFF;

	// save state
	save_item(NAME(m_select));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_multipak_device::device_reset()
{
	m_select = 0xFF;
}



//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor coco_multipak_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( coco_multi );
}



//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

UINT8* coco_multipak_device::get_cart_base()
{
	return active_cts_slot()->get_cart_base();
}



//-------------------------------------------------
//  read
//-------------------------------------------------

READ8_MEMBER(coco_multipak_device::read)
{
	return active_scs_slot()->read(space,offset);
}



//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER(coco_multipak_device::write)
{
	active_scs_slot()->write(space,offset,data);
}



//-------------------------------------------------
//  ff7f_write
//-------------------------------------------------

WRITE8_MEMBER(coco_multipak_device::ff7f_write)
{
	set_select(data);
}



//-------------------------------------------------
//  set_select
//-------------------------------------------------

void coco_multipak_device::set_select(UINT8 new_select)
{
	UINT8 xorval = m_select ^ new_select;
	m_select = new_select;
	if (xorval & 0x30)
		cart_base_changed();
}



//-------------------------------------------------
//  active_scs_slot
//-------------------------------------------------

cococart_slot_device *coco_multipak_device::active_scs_slot(void)
{
	return m_slots[(m_select >> 0) & 0x03];
}



//-------------------------------------------------
//  active_cts_slot
//-------------------------------------------------

cococart_slot_device *coco_multipak_device::active_cts_slot(void)
{
	return m_slots[(m_select >> 4) & 0x03];
}
