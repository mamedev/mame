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
#include "machine/sat_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SATURN_CART_SLOT = &device_creator<sat_cart_slot_device>;


//-------------------------------------------------
//  device_sat_cart_interface - constructor
//-------------------------------------------------

device_sat_cart_interface::device_sat_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(NULL),
		m_ext_dram0(NULL),
		m_ext_dram1(NULL),
		m_ext_bram(NULL),
		m_rom_size(0),
		m_ext_dram0_size(0),
		m_ext_dram1_size(0),
		m_ext_bram_size(0)
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

void device_sat_cart_interface::rom_alloc(running_machine &machine, UINT32 size)
{
	if (m_rom == NULL)
	{
		m_rom = auto_alloc_array_clear(machine, UINT32, size/4);
		m_rom_size = size;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sat_cart_slot_device - constructor
//-------------------------------------------------
sat_cart_slot_device::sat_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, SATURN_CART_SLOT, "Saturn Cartridge Slot", tag, owner, clock),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this)
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
		UINT32 *ROM;
		UINT32 len;

		if (software_entry() != NULL)
			len = get_software_region_length("cart");
		else
			len = length();

		m_cart->rom_alloc(machine(), len);
		ROM = m_cart->get_rom_base();

		if (software_entry() != NULL)
			memcpy(ROM, get_software_region("cart"), len);
		else
			fread(ROM, len);

		// fix endianness....
//      for (int i = 0; i < len; i += 4)
//      {
//          UINT8 tempa = ROM[i+0];
//          UINT8 tempb = ROM[i+1];
//          ROM[i+1] = ROM[i+2];
//          ROM[i+0] = ROM[i+3];
//          ROM[i+3] = tempa;
//          ROM[i+2] = tempb;
//      }
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

bool sat_cart_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry );
	return TRUE;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

const char * sat_cart_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
{
	return software_get_default_slot(config, options, this, "rom");
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
		return m_cart->read_rom(space, offset);
	else
		return 0xffffffff;
}

READ32_MEMBER(sat_cart_slot_device::read_ext_dram0)
{
	if (m_cart)
		return m_cart->read_ext_dram0(space, offset);
	else
		return 0xffffffff;
}

READ32_MEMBER(sat_cart_slot_device::read_ext_dram1)
{
	if (m_cart)
		return m_cart->read_ext_dram1(space, offset);
	else
		return 0xffffffff;
}

READ32_MEMBER(sat_cart_slot_device::read_ext_bram)
{
	if (m_cart)
		return m_cart->read_ext_bram(space, offset);
	else
		return 0xffffffff;
}

#if 0
READ32_MEMBER(sat_cart_slot_device::read_ext_bram)
{
	if (m_cart)
	{
		return (m_cart->read_ext_bram(space, offset * 2) << 16)
					| m_cart->read_ext_bram(space, offset * 2 + 1);
	}
	else
		return 0xffffffff;
}
#endif

/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE32_MEMBER(sat_cart_slot_device::write_ext_dram0)
{
	if (m_cart)
		m_cart->write_ext_dram0(space, offset, data);
}

WRITE32_MEMBER(sat_cart_slot_device::write_ext_dram1)
{
	if (m_cart)
		m_cart->write_ext_dram1(space, offset, data);
}

WRITE32_MEMBER(sat_cart_slot_device::write_ext_bram)
{
	if (m_cart)
		m_cart->write_ext_bram(space, offset, data);
}

#if 0
WRITE32_MEMBER(sat_cart_slot_device::write_ext_bram)
{
	if (m_cart)
	{
		if (ACCESSING_BITS_16_23)
			m_cart->write_ext_bram(space, offset * 2 + 0, (data & 0x00ff0000) >> 16);
		if (ACCESSING_BITS_0_7)
			m_cart->write_ext_bram(space, offset * 2 + 1, (data & 0x000000ff) >> 0);
	}
}
#endif
