// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
/**************************************************************************************************

Atari A800/A5200/XEGS cart slot emulation

<fill me>

References:
- Altirra HW reference manual 2022-07-07 edition, chapter 8;
- https://github.com/atari800/atari800/blob/master/DOC/cart.txt
- https://www.atarimax.com/jindroush.atari.org/acarts.html

**************************************************************************************************/

#include "emu.h"
#include "a800_slot.h"

#include "hashfile.h"
#include "multibyte.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A800_CART_SLOT,  a800_cart_slot_device,  "a800_cart_slot",  "Atari 8-bit Cartridge Slot")
DEFINE_DEVICE_TYPE(A5200_CART_SLOT, a5200_cart_slot_device, "a5200_cart_slot", "Atari 5200 Cartridge Slot")


//-------------------------------------------------
//  device_a800_cart_interface - constructor
//-------------------------------------------------

device_a800_cart_interface::device_a800_cart_interface (const machine_config &mconfig, device_t &device)
	: device_interface(device, "a800cart")
	, m_rom(nullptr)
	, m_rom_size(0)
	, m_bank_mask(0)
{
	m_slot = dynamic_cast<a800_cart_slot_device *>(device.owner());
}

device_a5200_cart_interface::device_a5200_cart_interface (const machine_config &mconfig, device_t &device)
	: device_interface(device, "a5200cart")
	, m_rom(nullptr)
	, m_rom_size(0)
	, m_bank_mask(0)
{
	m_slot = dynamic_cast<a5200_cart_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_a800_cart_interface  - destructor
//-------------------------------------------------

device_a800_cart_interface::~device_a800_cart_interface ()
{
}

device_a5200_cart_interface::~device_a5200_cart_interface ()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_a800_cart_interface::rom_alloc(uint32_t size)
{
	if (m_rom == nullptr)
	{
		// TODO: shouldn't really load from fixed tag
		// (particularly inconvenient for stuff like flash ROM hookups)
		m_rom = device().machine().memory().region_alloc(device().subtag("^cart:rom"), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;

		// setup other helpers
		// TODO: unusable for SIC! and any other mapping that maps over 0x4000 rather than 0x2000
		m_bank_mask = (size / 0x2000) - 1;  // code for XEGS carts makes use of this to simplify banking
	}
}

void device_a800_cart_interface::interface_pre_start()
{
	if (!m_slot->started())
		throw device_missing_dependencies();
}

void device_a800_cart_interface::interface_post_start()
{
	m_slot->m_space_mem->install_device(0x0000, 0x3fff, *this, &device_a800_cart_interface::cart_map);
	m_slot->m_space_io->install_device(0x0000, 0x00ff, *this, &device_a800_cart_interface::cctl_map);
}

void device_a800_cart_interface::cart_map(address_map &map)
{
	map(0x0000, 0x3fff).unmaprw();
}

void device_a800_cart_interface::cctl_map(address_map &map)
{
	map(0x0000, 0x00ff).unmaprw();
}

void device_a800_cart_interface::rd4_w(int state) { m_slot->m_rd4_cb(state); }
void device_a800_cart_interface::rd5_w(int state) { m_slot->m_rd5_cb(state); }
// helper to call both lines at same time, for anything banking on the full range.
void device_a800_cart_interface::rd_both_w(int state) { rd4_w(state); rd5_w(state); }


// a5200

void device_a5200_cart_interface::rom_alloc(uint32_t size)
{
	if (m_rom == nullptr)
	{
		// TODO: shouldn't really load from fixed tag
		// (particularly inconvenient for stuff like flash ROM hookups)
		m_rom = device().machine().memory().region_alloc(device().subtag("^cart:rom"), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;

		// setup other helpers
		// TODO: unusable for SIC! and any other mapping that maps over 0x4000 rather than 0x2000
		m_bank_mask = (size / 0x2000) - 1;  // code for XEGS carts makes use of this to simplify banking
	}
}

void device_a5200_cart_interface::cart_map(address_map &map)
{
	map(0x0000, 0x7fff).unmaprw();
}

void device_a5200_cart_interface::interface_pre_start()
{
	if (!m_slot->started())
		throw device_missing_dependencies();
}

void device_a5200_cart_interface::interface_post_start()
{
	m_slot->m_space_mem->install_device(0x0000, 0x7fff, *this, &device_a5200_cart_interface::cart_map);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ****_cart_slot_device - constructor
//-------------------------------------------------

a800_cart_slot_device::a800_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, A800_CART_SLOT, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_a800_cart_interface>(mconfig, *this)
	, m_cart(nullptr)
	, m_rd4_cb(*this)
	, m_rd5_cb(*this)
	, m_space_mem_config("cart_mem", ENDIANNESS_LITTLE, 8, 14, 0, address_map_constructor())
	, m_space_io_config("cart_io", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor())
	, m_is_xegs(false)
{
}

a5200_cart_slot_device::a5200_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, A5200_CART_SLOT, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_a5200_cart_interface>(mconfig, *this)
	, m_cart(nullptr)
	, m_space_mem_config("cart_mem", ENDIANNESS_LITTLE, 8, 15, 0, address_map_constructor())
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

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a800_cart_slot_device::device_start()
{
	m_cart = get_card_device();
	m_space_io = &space(AS_IO);
	m_space_mem = &space(AS_PROGRAM);
}

void a5200_cart_slot_device::device_start()
{
	m_cart = get_card_device();
	m_space_mem = &space(AS_PROGRAM);
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
	{ A800_BLIZZARD,  "a800_blizzard" },
	{ A800_XEGS,      "xegs" },
	{ A800_BBSB,      "a800_bbsb" },
	{ A800_DIAMOND,   "a800_diamond" },
	{ A800_WILLIAMS,  "a800_williams" },
	{ A800_EXPRESS,   "a800_express" },
	{ A800_SPARTADOS, "a800_sparta" },
	{ A800_SPARTADOS_128KB, "a800_sparta_128kb" },
	{ A800_TURBO,     "a800_turbo" },
	{ A800_BLIZZARD,  "a800_blizzard" },
	{ A800_TELELINK2, "a800_tlink2" },
	{ A800_ULTRACART, "a800_ultracart" },
	{ A800_ATRAX,     "a800_atrax" },
	{ A800_CORINA,    "a800_corina" },
	{ A800_CORINA_SRAM, "a800_corina_sram" },
	{ SIC_128KB,      "sic_128kb" },
	{ SIC_256KB,      "sic_256kb" },
	{ SIC_512KB,      "sic_512kb" },
	{ ATARIMAX_MAXFLASH_128KB, "maxflash_128kb" },
	{ ATARIMAX_MAXFLASH_1MB, "maxflash_1mb" },
	{ A800_ADAWLIAH,  "a800_adawliah" },
	{ A800_8K_RIGHT,  "a800_8k_right" },
	{ A5200_4K,       "a5200_rom" },
	{ A5200_8K,       "a5200_rom" },
	{ A5200_16K,      "a5200_rom" },
	{ A5200_32K,      "a5200_rom" },
	{ A5200_16K_2CHIPS, "a5200_2chips" },
	{ A5200_BBSB,     "a5200_bbsb" }
};


static int a800_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!strcmp(elem.slot_option, slot))
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

std::pair<std::error_condition, std::string> a800_cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint32_t len;

		if (loaded_through_softlist())
		{
			const char *pcb_name;
			len = get_software_region_length("rom");

			m_cart->rom_alloc(len);
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
				uint8_t header[16];
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
				// also make a try with .hsi file (for .a52 files)
				std::string info;
				if (hashfile_extrainfo(*this, info) && info.compare("A13MIRRORING")==0)
					m_type = A5200_16K_2CHIPS;
			}

			m_cart->rom_alloc(len);
			fread(m_cart->get_rom_base(), len);
		}

		logerror("%s loaded cartridge '%s' size %dK\n", machine().system().name, filename(), len/1024);
	}
	return std::make_pair(std::error_condition(), std::string());
}

std::pair<std::error_condition, std::string> a5200_cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint32_t len;

		if (loaded_through_softlist())
		{
			const char *pcb_name;
			len = get_software_region_length("rom");

			m_cart->rom_alloc(len);
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
				uint8_t header[16];
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
				// also make a try with .hsi file (for .a52 files)
				std::string info;
				if (hashfile_extrainfo(*this, info) && info.compare("A13MIRRORING")==0)
					m_type = A5200_16K_2CHIPS;
			}

			m_cart->rom_alloc(len);
			fread(m_cart->get_rom_base(), len);
		}

		logerror("%s loaded cartridge '%s' size %dK\n", machine().system().name, filename(), len/1024);
	}
	return std::make_pair(std::error_condition(), std::string());
}

void a800_cart_slot_device::call_unload()
{
}

void a5200_cart_slot_device::call_unload()
{
}


/*-------------------------------------------------
 identify_cart_type - code to detect cart type from
 fullpath
 -------------------------------------------------*/

int a800_cart_slot_device::identify_cart_type(const uint8_t *header) const
{
	int type = A800_8K;

	// TODO: canonically applies to .car extension only, have 10 bytes extra header on top

	// check CART format
	if (strncmp((const char *)header, "CART", 4))
		fatalerror("Invalid header detected!\n");

	switch (get_u32be(&header[4]))
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
		case 17:
			type = A800_ATRAX;
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
		case 41:
			type = ATARIMAX_MAXFLASH_128KB;
			break;
		case 42:
			type = ATARIMAX_MAXFLASH_1MB;
			break;
		case 43:
			type = A800_SPARTADOS_128KB;
			break;
		case 44:
			type = A800_OSS8K;
			break;
		case 50:
			type = A800_TURBO;
			break;
		case 51:
			type = A800_TURBO;
			break;
		case 52:
			type = A800_ULTRACART;
			break;
		case 54:
			type = SIC_128KB;
			break;
		case 55:
			type = SIC_256KB;
			break;
		case 56:
			type = SIC_512KB;
			break;
		case 69:
			type = A800_ADAWLIAH;
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
			osd_printf_info("Cart type \"%d\" is currently unsupported.\n", get_u32be(&header[4]));
			break;
	}

	return type;
}

int a5200_cart_slot_device::identify_cart_type(const uint8_t *header) const
{
	int type = A800_8K;

	// TODO: canonically applies to .car extension only, have 10 bytes extra header on top

	// check CART format
	if (strncmp((const char *)header, "CART", 4))
		fatalerror("Invalid header detected!\n");

	switch (get_u32be(&header[4]))
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
		case 17:
			type = A800_ATRAX;
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
		case 41:
			type = ATARIMAX_MAXFLASH_128KB;
			break;
		case 42:
			type = ATARIMAX_MAXFLASH_1MB;
			break;
		case 43:
			type = A800_SPARTADOS_128KB;
			break;
		case 44:
			type = A800_OSS8K;
			break;
		case 50:
			type = A800_TURBO;
			break;
		case 51:
			type = A800_TURBO;
			break;
		case 52:
			type = A800_ULTRACART;
			break;
		case 54:
			type = SIC_128KB;
			break;
		case 55:
			type = SIC_256KB;
			break;
		case 56:
			type = SIC_512KB;
			break;
		case 69:
			type = A800_ADAWLIAH;
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
			osd_printf_info("Cart type \"%d\" is currently unsupported.\n", get_u32be(&header[4]));
			break;
	}

	return type;
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string a800_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	// Nope, will crash when mounting the SDX subslot
//  std::string slot_default_option = default_option();
	const bool is_xegs = m_is_xegs;

	if (hook.image_file())
	{
		uint64_t len;
		hook.image_file()->length(len); // FIXME: check error return

		int type = is_xegs ? A800_XEGS : A800_8K;
		// check whether there is an header, to identify the cart type
		if ((len % 0x1000) == 0x10)
		{
			uint8_t head[0x10];
			/*auto const [err, actual] =*/ read(*hook.image_file(), &head[0], 0x10); // FIXME: check error return or read returning short
			type = identify_cart_type(&head[0]);
		}
		else    // otherwise try to guess based on size
		{
			if (is_xegs)
				return std::string("xegs");
			// TODO: very incomplete, is it also worth it?
			// Altirra sports a very complex scoring analysis (including code binary patterns),
			// and it still grants to the user multiple options
			if (len == 0x4000)
				type = A800_16K;
			if (len == 0x2000)
				type = A800_8K;
		}

		if (is_xegs && type != A800_XEGS)
		{
			osd_printf_info("This game is not designed for XEGS.\n");
			osd_printf_info("You might want to run it in %s.\n", type >= A5200_4K ? "A5200" : "A800");
		}
		else if (type >= A5200_4K)
			osd_printf_info("This game is not designed for A800. You might want to run it in A5200.\n");

		char const *const slot_string = a800_get_slot(type);

		return std::string(slot_string);
	}

	return software_get_default_slot(is_xegs ? "xegs" : "a800_8k");
}


std::string a5200_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		uint64_t len;
		hook.image_file()->length(len); // FIXME: check error return

		// check whether there is an header, to identify the cart type
		int type = A5200_8K;
		if ((len % 0x1000) == 0x10)
		{
			uint8_t head[0x10];
			/*auto const [err, actual] =*/ read(*hook.image_file(), &head[0], 0x10); // FIXME: check error return or read returning short
			type = identify_cart_type(&head[0]);
		}
		else
		{
			std::string info;
			if (hook.hashfile_extrainfo(info) && info.compare("A13MIRRORING")==0)
				type = A5200_16K_2CHIPS;
		}
		if (type < A5200_4K)
			osd_printf_info("This game is not designed for A5200. You might want to run it in A800 or A800XL.\n");

		char const *const slot_string = a800_get_slot(type);

		return std::string(slot_string);
	}

	return software_get_default_slot("a5200_rom");
}


/*-------------------------------------------------
 read
 -------------------------------------------------*/

device_memory_interface::space_config_vector a800_cart_slot_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_mem_config),
		std::make_pair(AS_IO, &m_space_io_config)
	};
}

template <unsigned Bank> uint8_t a800_cart_slot_device::read_cart(offs_t offset)
{
	return m_space_mem->read_byte(offset | Bank << 13);
}

template <unsigned Bank> void a800_cart_slot_device::write_cart(offs_t offset, uint8_t data)
{
	return m_space_mem->write_byte(offset | Bank << 13, data);
}

// Instantiate maps
template uint8_t a800_cart_slot_device::read_cart<0>(offs_t offset);
template uint8_t a800_cart_slot_device::read_cart<1>(offs_t offset);
template void a800_cart_slot_device::write_cart<0>(offs_t offset, uint8_t data);
template void a800_cart_slot_device::write_cart<1>(offs_t offset, uint8_t data);

uint8_t a800_cart_slot_device::read_cctl(offs_t offset)
{
	return m_space_io->read_byte(offset);
}

void a800_cart_slot_device::write_cctl(offs_t offset, uint8_t data)
{
	m_space_io->write_byte(offset, data);
}

device_memory_interface::space_config_vector a5200_cart_slot_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_mem_config)
	};
}

uint8_t a5200_cart_slot_device::read_cart(offs_t offset)
{
	return m_space_mem->read_byte(offset);
}

void a5200_cart_slot_device::write_cart(offs_t offset, uint8_t data)
{
	return m_space_mem->write_byte(offset, data);
}
