// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


    Saturn cart emulation
    (through slot devices)

    Despite the system having a single cart slot, 3 different kinds of cart can be inserted and
    different memory areas are exposed to each of them
    * ROM carts are accessed in range 0x02000000-0x023fffff and 0x22000000-0x24ffffff of both CPUs
    * Data RAM carts are accessed in range 0x02400000-0x027fffff of both CPUs (each DRAM chip is
      mapped independently, the 1st at 0x2400000, the second at 0x2600000)
    * Battery RAM carts are accessed in range 0x04000000-0x047fffff of both CPUs

    It is not clear what happens to accesses beyond the cart size (open bus? mirror of cart data?),
    e.g. if you have a 16Mbit battery cart inserted and the system tries to read/write above 0x04400000,
    so for the moment the whole range is mapped and an error message is printed for out-of-bounds
    accesses


 ***********************************************************************************************************/


#include "emu.h"
#include "sat_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SATURN_CART_SLOT = &device_creator<sat_cart_slot_device>;


//-------------------------------------------------
//  device_sat_cart_interface - constructor
//-------------------------------------------------

device_sat_cart_interface::device_sat_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device), m_cart_type(0),
		m_rom(nullptr),
		m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_sat_cart_interface - destructor
//-------------------------------------------------

device_sat_cart_interface::~device_sat_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_sat_cart_interface::rom_alloc(UINT32 size, std::string tag)
{
	if (m_rom == nullptr)
	{
		m_rom = (UINT32 *)device().machine().memory().region_alloc(std::string(tag).append(SATSLOT_ROM_REGION_TAG).c_str(), size, 4, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  bram_alloc - alloc the space for the Backup RAM
//-------------------------------------------------

void device_sat_cart_interface::bram_alloc(UINT32 size)
{
	m_ext_bram.resize(size);
	device().save_item(NAME(m_ext_bram));
}


//-------------------------------------------------
//  dram*_alloc - alloc the space for the DRAM
//-------------------------------------------------

void device_sat_cart_interface::dram0_alloc(UINT32 size)
{
	m_ext_dram0.resize(size/sizeof(UINT32));
	device().save_item(NAME(m_ext_dram0));
}

void device_sat_cart_interface::dram1_alloc(UINT32 size)
{
	m_ext_dram1.resize(size/sizeof(UINT32));
	device().save_item(NAME(m_ext_dram1));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sat_cart_slot_device - constructor
//-------------------------------------------------
sat_cart_slot_device::sat_cart_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, SATURN_CART_SLOT, "Saturn Cartridge Slot", tag, owner, clock, "sat_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this), m_cart(nullptr)
{
}


//-------------------------------------------------
//  sat_cart_slot_device - destructor
//-------------------------------------------------

sat_cart_slot_device::~sat_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sat_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_sat_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void sat_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}



/*-------------------------------------------------
 call load
 -------------------------------------------------*/


bool sat_cart_slot_device::call_load()
{
	if (m_cart)
	{
		bool is_rom = ((software_entry() == nullptr) || ((software_entry() != nullptr) && get_software_region("rom")));

		if (is_rom)
		{
			// from fullpath, only ROM carts
			UINT32 len = (software_entry() != nullptr) ? get_software_region_length("rom") : length();
			UINT32 *ROM;

			m_cart->rom_alloc(len, tag());
			ROM = m_cart->get_rom_base();

			if (software_entry() != nullptr)
				memcpy(ROM, get_software_region("rom"), len);
			else
				fread(ROM, len);

			// fix endianness....
			for (int i = 0; i < len/4; i ++)
				ROM[i] = BITSWAP32(ROM[i],7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,23,22,21,20,19,18,17,16,31,30,29,28,27,26,25,24);
//          {
//              UINT8 tempa = ROM[i+0];
//              UINT8 tempb = ROM[i+1];
//              ROM[i+1] = ROM[i+2];
//              ROM[i+0] = ROM[i+3];
//              ROM[i+3] = tempa;
//              ROM[i+2] = tempb;
//          }
		}
		else
		{
			// DRAM or BRAM carts from softlist
			if (get_software_region("bram"))
				m_cart->bram_alloc(get_software_region_length("bram"));
			if (get_software_region("dram0"))
				m_cart->dram0_alloc(get_software_region_length("dram0"));
			if (get_software_region("dram1"))
				m_cart->dram1_alloc(get_software_region_length("dram1"));
		}
		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void sat_cart_slot_device::call_unload()
{
}



/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool sat_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	machine().rom_load().load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string sat_cart_slot_device::get_default_card_software()
{
	return software_get_default_slot("rom");
}



int sat_cart_slot_device::get_cart_type()
{
	if (m_cart)
		return m_cart->get_cart_type();

	return 0xff;
}



/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ32_MEMBER(sat_cart_slot_device::read_rom)
{
	if (m_cart)
		return m_cart->read_rom(space, offset, mem_mask);
	else
		return 0xffffffff;
}

READ32_MEMBER(sat_cart_slot_device::read_ext_dram0)
{
	if (m_cart)
		return m_cart->read_ext_dram0(space, offset, mem_mask);
	else
		return 0xffffffff;
}

READ32_MEMBER(sat_cart_slot_device::read_ext_dram1)
{
	if (m_cart)
		return m_cart->read_ext_dram1(space, offset, mem_mask);
	else
		return 0xffffffff;
}

READ32_MEMBER(sat_cart_slot_device::read_ext_bram)
{
	if (m_cart)
		return m_cart->read_ext_bram(space, offset, mem_mask);
	else
		return 0xffffffff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE32_MEMBER(sat_cart_slot_device::write_ext_dram0)
{
	if (m_cart)
		m_cart->write_ext_dram0(space, offset, data, mem_mask);
}

WRITE32_MEMBER(sat_cart_slot_device::write_ext_dram1)
{
	if (m_cart)
		m_cart->write_ext_dram1(space, offset, data, mem_mask);
}

WRITE32_MEMBER(sat_cart_slot_device::write_ext_bram)
{
	if (m_cart)
		m_cart->write_ext_bram(space, offset, data, mem_mask);
}
