// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


    Atari 8 bit cart emulation
    (through slot devices)

    Emulation of the cartslot(s) for Atari 8bit series of home computers

    Accessors to ROM are typically given in the area 0xa000-0xbfff, but some
    carts (and the right slot in A800) maps ROM to 0x8000-0x9fff too
    Bankswitch typically happens by accessing addresses in 0xd500-0xd5ff

    Accordingly, this device offers the following handlers
    - read_80xx/write_80xx
    - read_d5xx/write_d5xx
    Notice that these are installed in different ranges at machine start by
    the drivers, so that it might well be that offs=0 for read_80xx is 0xa000!

 ***********************************************************************************************************/


#include "emu.h"
#include "hashfile.h"
#include "a800_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A800_CART_SLOT = &device_creator<a800_cart_slot_device>;
const device_type A5200_CART_SLOT = &device_creator<a5200_cart_slot_device>;
const device_type XEGS_CART_SLOT = &device_creator<xegs_cart_slot_device>;


//-------------------------------------------------
//  device_vcs_cart_interface - constructor
//-------------------------------------------------

device_a800_cart_interface::device_a800_cart_interface (const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0),
		m_bank_mask(0)
{
}


//-------------------------------------------------
//  ~device_a800_cart_interface  - destructor
//-------------------------------------------------

device_a800_cart_interface::~device_a800_cart_interface ()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_a800_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(A800SLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;

		// setup other helpers
		m_bank_mask = (size / 0x2000) - 1;  // code for XEGS carts makes use of this to simplify banking
	}
}

//-------------------------------------------------
//  ram_alloc - alloc the space for the on-cart RAM
//-------------------------------------------------

void device_a800_cart_interface::ram_alloc(UINT32 size)
{
	m_ram.resize(size);
	device().save_item(NAME(m_ram));
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the on-cart RAM
//-------------------------------------------------

void device_a800_cart_interface::nvram_alloc(UINT32 size)
{
	m_nvram.resize(size);
	device().save_item(NAME(m_nvram));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ****_cart_slot_device - constructor
//-------------------------------------------------
a800_cart_slot_device::a800_cart_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
						device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this), m_cart(nullptr), m_type(0)
{
}

a800_cart_slot_device::a800_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, A800_CART_SLOT, "Atari 8bit Cartridge Slot", tag, owner, clock, "a800_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this), m_cart(nullptr), m_type(0)
{
}


a5200_cart_slot_device::a5200_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						a800_cart_slot_device(mconfig, A5200_CART_SLOT, "Atari 5200 Cartridge Slot", tag, owner, clock, "a5200_cart_slot", __FILE__)
{
}


xegs_cart_slot_device::xegs_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						a800_cart_slot_device(mconfig, XEGS_CART_SLOT, "Atari XEGS Cartridge Slot", tag, owner, clock, "xegs_cart_slot", __FILE__)
{
}


//-------------------------------------------------
//  ****_cart_slot_device - destructor
//-------------------------------------------------

a800_cart_slot_device::~a800_cart_slot_device()
{
}

a5200_cart_slot_device::~a5200_cart_slot_device()
{
}

xegs_cart_slot_device::~xegs_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a800_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_a800_cart_interface  *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void a800_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}



/*-------------------------------------------------
 call load
 -------------------------------------------------*/

//-------------------------------------------------
//  A800 PCBs
//-------------------------------------------------

struct a800_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const a800_slot slot_list[] =
{
	{ A800_8K,        "a800_8k" },
	{ A800_16K,       "a800_16k" },
	{ A800_OSS034M,   "a800_oss034m" },
	{ A800_OSS043M,   "a800_oss043m" },
	{ A800_OSSM091,   "a800_ossm091" },
	{ A800_OSS8K,     "a800_oss8k" },
	{ A800_PHOENIX,   "a800_phoenix" },
	{ A800_XEGS,      "xegs" },
	{ A800_BBSB,      "a800_bbsb" },
	{ A800_DIAMOND,   "a800_diamond" },
	{ A800_WILLIAMS,  "a800_williams" },
	{ A800_EXPRESS,   "a800_express" },
	{ A800_SPARTADOS, "a800_sparta" },
	{ A800_TURBO64,   "a800_turbo64" },
	{ A800_TURBO128,  "a800_turbo128" },
	{ A800_BLIZZARD,  "a800_blizzard" },
	{ A800_TELELINK2, "a800_tlink2" },
	{ A800_MICROCALC, "a800_sitsa" },
	{ A800_CORINA,    "a800_corina" },
	{ A800_8K_RIGHT,  "a800_8k_right" },
	{ A5200_4K,       "a5200" },
	{ A5200_8K,       "a5200" },
	{ A5200_16K,      "a5200" },
	{ A5200_32K,      "a5200" },
	{ A5200_16K_2CHIPS, "a5200_2chips" },
	{ A5200_32K,      "a5200" },
	{ A5200_BBSB,     "a5200_bbsb" }
};


static int a800_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *a800_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "a800_8k";
}

bool a800_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT32 len;

		if (software_entry() != nullptr)
		{
			const char *pcb_name;
			len = get_software_region_length("rom");

			m_cart->rom_alloc(len, tag());
			memcpy(m_cart->get_rom_base(), get_software_region("rom"), len);

			if ((pcb_name = get_feature("slot")) != nullptr)
				m_type = a800_get_pcb_id(pcb_name);
			else
				m_type = A800_8K;
		}
		else
		{
			len = length();

			// check whether there is an header, to identify the cart type
			if ((len % 0x1000) == 0x10)
			{
				UINT8 header[16];
				fread(header, 0x10);
				m_type = identify_cart_type(header);
				len -= 0x10;    // in identify_cart_type the first 0x10 bytes are read, so we need to adjust here
			}
			else    // otherwise try to guess based on size
			{
				if (len == 0x8000)
					m_type = A5200_32K;
				if (len == 0x4000)
					m_type = A800_16K;
				if (len == 0x2000)
					m_type = A800_8K;
				if (len == 0x1000)
					m_type = A5200_4K;
			}

			m_cart->rom_alloc(len, tag());
			fread(m_cart->get_rom_base(), len);
		}
		if (m_type == A800_TELELINK2)
			m_cart->nvram_alloc(0x100);

		printf("%s loaded cartridge '%s' size %dK\n", machine().system().name, filename(), len/1024);
	}
	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void a800_cart_slot_device::call_unload()
{
}

/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool a800_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	machine().rom_load().load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}

/*-------------------------------------------------
 identify_cart_type - code to detect cart type from
 fullpath
 -------------------------------------------------*/

int a800_cart_slot_device::identify_cart_type(UINT8 *header)
{
	int type = A800_8K;

	// check CART format
	if (strncmp((const char *)header, "CART", 4))
		fatalerror("Invalid header detected!\n");

	switch ((header[4] << 24) + (header[5] << 16) +  (header[6] << 8) + (header[7] << 0))
	{
		case 1:
			type = A800_8K;
			break;
		case 2:
			type = A800_16K;
			break;
		case 3:
			type = A800_OSS034M;
			break;
		case 8:
			type = A800_WILLIAMS;
			break;
		case 9:
			type = A800_DIAMOND;
			break;
		case 10:
			type = A800_EXPRESS;
			break;
		case 11:
			type = A800_SPARTADOS;
			break;
		case 12:
			type = A800_XEGS;
			break;
		case 15:
			type = A800_OSSM091;
			break;
		case 18:
			type = A800_BBSB;
			break;
		case 21:
			type = A800_8K_RIGHT;
			break;
		case 39:
			type = A800_PHOENIX;
			break;
		case 40:
			type = A800_BLIZZARD;
			break;
		case 44:
			type = A800_OSS8K;
			break;
		case 50:
			type = A800_TURBO64;
			break;
		case 51:
			type = A800_TURBO128;
			break;
		case 52:
			type = A800_MICROCALC;
			break;
			// Atari 5200 CART files
		case 4:
			type = A5200_32K;
			break;
		case 16:
			type = A5200_16K;
			break;
		case 19:
			type = A5200_8K;
			break;
		case 20:
			type = A5200_4K;
			break;
		case 6:
			type = A5200_16K_2CHIPS;
			break;
		case 7:
			type = A5200_BBSB;
			break;
		default:
			osd_printf_info("Cart type \"%d\" is currently unsupported.\n", (header[4] << 24) + (header[5] << 16) +  (header[6] << 8) + (header[7] << 0));
			break;
	}

	return type;
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string a800_cart_slot_device::get_default_card_software()
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string;
		dynamic_buffer head(0x10);
		UINT32 len = core_fsize(m_file);
		int type = A800_8K;

		// check whether there is an header, to identify the cart type
		if ((len % 0x1000) == 0x10)
		{
			core_fread(m_file, &head[0], 0x10);
			type = identify_cart_type(&head[0]);
		}
		else    // otherwise try to guess based on size
		{
			if (len == 0x4000)
				type = A800_16K;
			if (len == 0x2000)
				type = A800_8K;
		}

		if (type >= A5200_4K)
			osd_printf_info("This game is not designed for A800. You might want to run it in A5200.\n");

		slot_string = a800_get_slot(type);

		clear();

		return std::string(slot_string);
	}
	else
		return software_get_default_slot("a800_8k");
}


std::string a5200_cart_slot_device::get_default_card_software()
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string;
		dynamic_buffer head(0x10);
		UINT32 len = core_fsize(m_file);
		int type = A5200_8K;

		// check whether there is an header, to identify the cart type
		if ((len % 0x1000) == 0x10)
		{
			core_fread(m_file, &head[0], 0x10);
			type = identify_cart_type(&head[0]);

			std::string info;
			if (hashfile_extrainfo(*this, info) && info.compare("A13MIRRORING")==0)
				type = A5200_16K_2CHIPS;
		}
		if (type < A5200_4K)
			osd_printf_info("This game is not designed for A5200. You might want to run it in A800 or A800XL.\n");

		slot_string = a800_get_slot(type);

		clear();

		return std::string(slot_string);
	}
	else
		return software_get_default_slot("a5200");
}


std::string xegs_cart_slot_device::get_default_card_software()
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string;
		dynamic_buffer head(0x10);
		UINT32 len = core_fsize(m_file);
		int type = A800_8K;

		// check whether there is an header, to identify the cart type
		if ((len % 0x1000) == 0x10)
		{
			core_fread(m_file, &head[0], 0x10);
			type = identify_cart_type(&head[0]);
		}
		if (type != A800_XEGS)
		{
			osd_printf_info("This game is not designed for XEGS. ");
			if (type >= A5200_4K)
				osd_printf_info("You might want to run it in A5200.\n");
			else
				osd_printf_info("You might want to run it in A800 or A800XL.\n");
		}

		slot_string = a800_get_slot(type);

		clear();

		return std::string(slot_string);
	}
	else
		return software_get_default_slot("xegs");
}


/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(a800_cart_slot_device::read_80xx)
{
	if (m_cart)
		return m_cart->read_80xx(space, offset, mem_mask);
	else
		return 0xff;
}

READ8_MEMBER(a800_cart_slot_device::read_d5xx)
{
	if (m_cart)
		return m_cart->read_d5xx(space, offset, mem_mask);
	else
		return 0xff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(a800_cart_slot_device::write_80xx)
{
	if (m_cart)
		m_cart->write_80xx(space, offset, data, mem_mask);
}

WRITE8_MEMBER(a800_cart_slot_device::write_d5xx)
{
	if (m_cart)
		m_cart->write_d5xx(space, offset, data, mem_mask);
}
