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

DEFINE_DEVICE_TYPE(SATURN_CART_SLOT, sat_cart_slot_device, "sat_cart_slot", "Saturn Cartridge Slot")


//-------------------------------------------------
//  device_sat_cart_interface - constructor
//-------------------------------------------------

device_sat_cart_interface::device_sat_cart_interface(const machine_config &mconfig, device_t &device, int cart_type) :
	device_interface(device, "saturncart"),
	m_cart_type(cart_type),
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

void device_sat_cart_interface::rom_alloc(uint32_t size)
{
	if (m_rom == nullptr)
	{
		m_rom = (uint32_t *)device().machine().memory().region_alloc(device().subtag("^cart:rom"), size, 4, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  bram_alloc - alloc the space for the Backup RAM
//-------------------------------------------------

void device_sat_cart_interface::bram_alloc(uint32_t size)
{
	m_ext_bram.resize(size);
	device().save_item(NAME(m_ext_bram));
}


//-------------------------------------------------
//  dram*_alloc - alloc the space for the DRAM
//-------------------------------------------------

void device_sat_cart_interface::dram0_alloc(uint32_t size)
{
	m_ext_dram0.resize(size/sizeof(uint32_t));
	device().save_item(NAME(m_ext_dram0));
}

void device_sat_cart_interface::dram1_alloc(uint32_t size)
{
	m_ext_dram1.resize(size/sizeof(uint32_t));
	device().save_item(NAME(m_ext_dram1));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sat_cart_slot_device - constructor
//-------------------------------------------------
sat_cart_slot_device::sat_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SATURN_CART_SLOT, tag, owner, clock),
	device_cartrom_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_sat_cart_interface>(mconfig, *this),
	m_cart(nullptr)
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
	m_cart = get_card_device();
}



/*-------------------------------------------------
 call load
 -------------------------------------------------*/


image_init_result sat_cart_slot_device::call_load()
{
	if (m_cart)
	{
		bool is_rom = (!loaded_through_softlist() || (loaded_through_softlist() && get_software_region("rom")));

		if (is_rom)
		{
			// from fullpath, only ROM carts
			uint32_t len = loaded_through_softlist() ? get_software_region_length("rom") : length();
			uint32_t *ROM;

			m_cart->rom_alloc(len);
			ROM = m_cart->get_rom_base();

			if (loaded_through_softlist())
				memcpy(ROM, get_software_region("rom"), len);
			else
				fread(ROM, len);

			// fix endianness....
			for (int i = 0; i < len/4; i ++)
				ROM[i] = bitswap<32>(ROM[i],7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,23,22,21,20,19,18,17,16,31,30,29,28,27,26,25,24);
//          {
//              uint8_t tempa = ROM[i+0];
//              uint8_t tempb = ROM[i+1];
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
		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void sat_cart_slot_device::call_unload()
{
}



/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string sat_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
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

uint32_t sat_cart_slot_device::read_rom(offs_t offset)
{
	if (m_cart)
		return m_cart->read_rom(offset);
	else
		return 0xffffffff;
}

uint32_t sat_cart_slot_device::read_ext_dram0(offs_t offset)
{
	if (m_cart)
		return m_cart->read_ext_dram0(offset);
	else
		return 0xffffffff;
}

uint32_t sat_cart_slot_device::read_ext_dram1(offs_t offset)
{
	if (m_cart)
		return m_cart->read_ext_dram1(offset);
	else
		return 0xffffffff;
}

uint32_t sat_cart_slot_device::read_ext_bram(offs_t offset)
{
	if (m_cart)
		return m_cart->read_ext_bram(offset);
	else
		return 0xffffffff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

void sat_cart_slot_device::write_ext_dram0(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_cart)
		m_cart->write_ext_dram0(offset, data, mem_mask);
}

void sat_cart_slot_device::write_ext_dram1(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_cart)
		m_cart->write_ext_dram1(offset, data, mem_mask);
}

void sat_cart_slot_device::write_ext_bram(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_cart)
		m_cart->write_ext_bram(offset, data, mem_mask);
}
