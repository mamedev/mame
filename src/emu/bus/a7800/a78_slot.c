/***********************************************************************************************************


    Atari 7800 cart emulation
    (through slot devices)

    Emulation of the cartslot for Atari 7800

    Quoting "ATARI 7800 BANKSWITCHING GUIDE" (by Eckhard Stolberg):
    7800 games can use the memory from $0400 to $047f, from $0500
    to $17ff and from $2800 to $ffff, but only the High-Score cart
    uses anything below $4000. It has 4KB of ROM at $3000-$3fff
    and 2KB of battery-backed RAM at $1000-$17ff.

    Accordingly, we use the following handlers:
    - read_04xx/write_04xx for accesses in the $0400 to $047f range
    - read_10xx/write_10xx for accesses in the $1000 to $17ff range
    - read_30xx/write_30xx for accesses in the $3000 to $3fff range
    - read_40xx/write_40xx for accesses in the $4000 to $ffff range
    even if not all carts use all of them (in particular no cart type
    seems to use access to the ranges $0500 to $0fff and $2800 to $2fff)
 

 ***********************************************************************************************************/


#include "emu.h"
#include "a78_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A78_CART_SLOT = &device_creator<a78_cart_slot_device>;


//-------------------------------------------------
//  device_vcs_cart_interface - constructor
//-------------------------------------------------

device_a78_cart_interface::device_a78_cart_interface (const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_a78_cart_interface  - destructor
//-------------------------------------------------

device_a78_cart_interface::~device_a78_cart_interface ()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_a78_cart_interface::rom_alloc(UINT32 size)
{
	if (m_rom == NULL)
	{
		// allocate rom
		m_rom.resize(size);

		// setup other helpers
		if ((size / 0x4000) & 1) // compensate for SuperGame carts with 9 x 16K banks (to my knowledge no other cart has m_bank_mask != power of 2)
			m_bank_mask = (size / 0x4000) - 2;
		else
			m_bank_mask = (size / 0x4000) - 1;

		// the rom is mapped to the top of the memory area
		// so we store the starting point of data to simplify 
		// the access handling
		m_base_rom = 0x10000 - size;
	}
}

//-------------------------------------------------
//  ram_alloc - alloc the space for the on-cart RAM
//-------------------------------------------------

void device_a78_cart_interface::ram_alloc(UINT32 size)
{
	if (m_ram == NULL)
	{
		m_ram.resize(size);
		device().save_item(NAME(m_ram));
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the on-cart RAM
//-------------------------------------------------

void device_a78_cart_interface::nvram_alloc(UINT32 size)
{
	if (m_nvram == NULL)
	{
		m_nvram.resize(size);
		device().save_item(NAME(m_nvram));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a78_cart_slot_device - constructor
//-------------------------------------------------
a78_cart_slot_device::a78_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, A78_CART_SLOT, "Atari 7800 Cartridge Slot", tag, owner, clock, "a78_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  a78_cart_slot_device - destructor
//-------------------------------------------------

a78_cart_slot_device::~a78_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a78_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_a78_cart_interface  *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void a78_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}



/*-------------------------------------------------
 call load
 -------------------------------------------------*/

//-------------------------------------------------
//  A78 PCBs
//-------------------------------------------------

struct a78_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const a78_slot slot_list[] =
{
	{ A78_TYPE0,    "a78_rom" },
	{ A78_TYPE1,    "a78_pokey" },
	{ A78_TYPE2,    "a78_sg" },
	{ A78_TYPE3,    "a78_sg_pokey" },
	{ A78_TYPE6,    "a78_sg_ram" },
	{ A78_TYPEA,    "a78_sg9" },
	{ A78_TYPEB,    "a78_xmc" },
	{ A78_ABSOLUTE, "a78_abs" },
	{ A78_ACTIVISION, "a78_act" },
	{ A78_HSC,      "a78_hsc" },
	{ A78_BANKRAM,  "a78_bankram" },
	{ A78_XB_BOARD, "a78_xboard" },
	{ A78_XM_BOARD, "a78_xm" },
};

static int a78_get_pcb_id(const char *slot)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!core_stricmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return 0;
}

static const char *a78_get_slot(int type)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
	}

	return "a78_rom";
}

bool a78_cart_slot_device::call_load()
{
	UINT8 *ROM;
	UINT32 len;

	if (software_entry() != NULL)
	{
		const char *pcb_name;
		len = get_software_region_length("rom");

		m_cart->rom_alloc(len);
		ROM = m_cart->get_rom_base();
		memcpy(ROM, get_software_region("rom"), len);

		if ((pcb_name = get_feature("slot")) != NULL)
			m_type = a78_get_pcb_id(pcb_name);
		else
			m_type = A78_TYPE0;
	}
	else
	{
		// Load and check the header
		char head[128];		
		fread(head, 128);
		
		if (verify_header((char *)head) == IMAGE_VERIFY_FAIL)
			return IMAGE_INIT_FAIL;

		len = (head[49] << 24) |(head[50] << 16) |(head[51] << 8) | head[52];
		if (len + 128 > length())
		{
			logerror("Invalid length in the header. The game might be corrupted.\n");
			len = length() - 128;
		}

		switch ((head[53] << 8) | head[54])
		{
			case 0x0000:
				m_type = A78_TYPE0;
				break;
			case 0x0001:
				m_type = A78_TYPE1;
				break;
			case 0x0002:
				m_type = A78_TYPE2;
				break;
			case 0x0003:
				m_type = A78_TYPE3;
				break;
			case 0x0006:
				m_type = A78_TYPE6;
				break;
			case 0x000a:
				m_type = A78_TYPEA;
				break;
			case 0x000b:
				m_type = A78_TYPEB;
				break;
			case 0x0020:
				m_type = A78_BANKRAM;
				break;
			case 0x0100:
				m_type = A78_ABSOLUTE;
				break;
			case 0x0200:
				m_type = A78_ACTIVISION;
				break;
		}
		logerror("Cart type: %x\n", m_type);
		
		// This field is currently only used for logging
		m_stick_type = head[55];
		
		m_cart->rom_alloc(len);
		ROM = m_cart->get_rom_base();
		fread(ROM, len);
	}

	//printf("Type: %s\n", a78_get_slot(m_type));

	if (m_type == A78_TYPE6)
		m_cart->ram_alloc(0x4000);
	if (m_type == A78_BANKRAM)
		m_cart->ram_alloc(0x8000);
	if (m_type == A78_XB_BOARD || m_type == A78_XM_BOARD)
		m_cart->ram_alloc(0x20000);
	if (m_type == A78_HSC || m_type == A78_XM_BOARD)
	{
		m_cart->nvram_alloc(0x800);
		battery_load(m_cart->get_nvram_base(), 0x800, 0xff);
	}

	return IMAGE_INIT_PASS;
}


void a78_partialhash(hash_collection &dest, const unsigned char *data,
					 unsigned long length, const char *functions)
{
	if (length <= 128)
		return;
	dest.compute(&data[128], length - 128, functions);
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void a78_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_nvram_size())
		battery_save(m_cart->get_nvram_base(), 0x800);
}



/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool a78_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry );
	return TRUE;
}

/*-------------------------------------------------
 identify_cart_type - code to detect cart type from
 fullpath
 -------------------------------------------------*/

int a78_cart_slot_device::verify_header(char *header)
{
	const char *magic = "ATARI7800";

	if (strncmp(magic, header + 1, 9))
	{
		logerror("Not a valid A7800 image\n");
		return IMAGE_VERIFY_FAIL;
	}

	logerror("returning ID_OK\n");
	return IMAGE_VERIFY_PASS;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

void a78_cart_slot_device::get_default_card_software(astring &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "a78_rom";
		dynamic_buffer head(128);
		int type = A78_TYPE0;
		
		// Load and check the header
		core_fread(m_file, head, 128);		
		switch ((head[53] << 8) | head[54])
		{
			case 0x0000:
				type = A78_TYPE0;
				break;
			case 0x0001:
				type = A78_TYPE1;
				break;
			case 0x0002:
				type = A78_TYPE2;
				break;
			case 0x0003:
				type = A78_TYPE3;
				break;
			case 0x0004:
				type = A78_TYPE6;
				break;
			case 0x000a:
				type = A78_TYPEA;
				break;
			case 0x000b:
				type = A78_TYPEB;
				break;
			case 0x0020:
				m_type = A78_BANKRAM;
				break;
			case 0x0100:
				type = A78_ABSOLUTE;
				break;
			case 0x0200:
				type = A78_ACTIVISION;
				break;
		}
		logerror("Cart type: %x\n", type);
		slot_string = a78_get_slot(type);
		
		clear();
		
		result.cpy(slot_string);
	}
	else
		software_get_default_slot(result, "a78_rom");
}


/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(a78_cart_slot_device::read_04xx)
{
	if (m_cart)
		return m_cart->read_04xx(space, offset, mem_mask);
	else
		return 0xff;
}

READ8_MEMBER(a78_cart_slot_device::read_10xx)
{
	if (m_cart)
		return m_cart->read_10xx(space, offset, mem_mask);
	else
		return 0xff;
}

READ8_MEMBER(a78_cart_slot_device::read_30xx)
{
	if (m_cart)
		return m_cart->read_30xx(space, offset, mem_mask);
	else
		return 0xff;
}

READ8_MEMBER(a78_cart_slot_device::read_40xx)
{
	if (m_cart)
		return m_cart->read_40xx(space, offset, mem_mask);
	else
		return 0xff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(a78_cart_slot_device::write_04xx)
{
	if (m_cart)
		m_cart->write_04xx(space, offset, data, mem_mask);
}

WRITE8_MEMBER(a78_cart_slot_device::write_10xx)
{
	if (m_cart)
		m_cart->write_10xx(space, offset, data, mem_mask);
}

WRITE8_MEMBER(a78_cart_slot_device::write_30xx)
{
	if (m_cart)
		m_cart->write_30xx(space, offset, data, mem_mask);
}

WRITE8_MEMBER(a78_cart_slot_device::write_40xx)
{
	if (m_cart)
		m_cart->write_40xx(space, offset, data, mem_mask);
}


/*  Header format
 0     Header version     - 1 byte
 1..16  "ATARI7800     "  - 16 bytes
 17..48 Cart title        - 32 bytes
 49..52 data length      - 4 bytes
 53..54 cart type          - 2 bytes
 bit 0 0x01 - pokey cart
 bit 1 0x02 - supercart bank switched
 bit 2 0x04 - supercart RAM at $4000
 bit 3 0x08 - additional ROM at $4000
 bit 4 0x10 - bank 6 at $4000
 bit 5 0x20 - supercart banked RAM
 
 bit 8-15 - Special
 0 = Normal cart
 1 = Absolute (F18 Hornet)
 2 = Activision
 
 55   controller 1 type  - 1 byte
 56   controller 2 type  - 1 byte
 0 = None
 1 = Joystick
 2 = Light Gun
 57  0 = NTSC/1 = PAL
 
 100..127 "ACTUAL CART DATA STARTS HERE" - 28 bytes
 
 Versions:
 Version 0: Initial release
 Version 1: Added PAL/NTSC bit. Added Special cart byte.
 Changed 53 bit 2, added bit 3
 
 */

// TODO: log all properties from the header