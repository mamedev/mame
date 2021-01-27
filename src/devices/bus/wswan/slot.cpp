// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************************************

    Bandai Wonderswan / Wonderswan Color cart emulation
    (through slot devices)

 ***********************************************************************************************************/

#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(WS_CART_SLOT, ws_cart_slot_device, "ws_cart_slot", "Wonderswan Cartridge Slot")

//**************************************************************************
//    Wonderswan Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_ws_cart_interface - constructor
//-------------------------------------------------

device_ws_cart_interface::device_ws_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "wswancart"),
	m_rom(nullptr),
	m_rom_size(0),
	m_nvram_size(0),
	m_bank_mask(0),
	m_has_rtc(false)
{
}


//-------------------------------------------------
//  ~device_ws_cart_interface - destructor
//-------------------------------------------------

device_ws_cart_interface::~device_ws_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_ws_cart_interface::rom_alloc(u32 size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = (u16 *)device().machine().memory().region_alloc(std::string(tag).append(WSSLOT_ROM_REGION_TAG).c_str(), size, 2, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
		m_bank_mask = ((m_rom_size >> 16) - 1);
	}
}


//-------------------------------------------------
//  nvram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_ws_cart_interface::nvram_alloc(u32 size)
{
	m_nvram_size = size;
	m_nvram.resize(size);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ws_cart_slot_device - constructor
//-------------------------------------------------
ws_cart_slot_device::ws_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, WS_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_ws_cart_interface>(mconfig, *this),
	m_type(WS_STD),
	m_cart(nullptr)
{
}


//-------------------------------------------------
//  ws_cart_slot_device - destructor
//-------------------------------------------------

ws_cart_slot_device::~ws_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ws_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}


//-------------------------------------------------
//  WSWAN PCB
//-------------------------------------------------

struct ws_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const ws_slot slot_list[] =
{
	{ WS_STD,      "ws_rom" },
	{ WS_SRAM,     "ws_sram" },
	{ WS_EEPROM,   "ws_eeprom" },
	{ WWITCH,      "wwitch"}
};

static int ws_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!strcmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *ws_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "std";
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result ws_cart_slot_device::call_load()
{
	if (m_cart)
	{
		u16 *ROM;
		u32 size = !loaded_through_softlist() ? length() : get_software_region_length("rom");
		u32 nvram_size = 0;

		m_cart->rom_alloc(size, tag());
		ROM = m_cart->get_rom_base();

		if (!loaded_through_softlist())
			fread(ROM, size);
		else
			memcpy(ROM, get_software_region("rom"), size);

		if (!loaded_through_softlist())
		{
			// get cart type and nvram length
			m_type = get_cart_type(ROM, size, nvram_size);

			if (ROM[(size >> 1) - 2] & 0xff00)
				m_cart->set_has_rtc(true);
		}
		else
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = ws_get_pcb_id(pcb_name);

			if (m_type == WS_SRAM || m_type == WWITCH)
				nvram_size = get_software_region_length("sram");
			if (m_type == WS_EEPROM)
				nvram_size = get_software_region_length("eeprom");

			if (get_feature("rtc"))
			{
				if (!strcmp(get_feature("rtc"), "yes"))
					m_cart->set_has_rtc(true);
			}
		}

		if (nvram_size)
		{
			// allocate NVRAM
			m_cart->nvram_alloc(nvram_size);
			// and load possible battery save
			battery_load(m_cart->get_nvram_base(), m_cart->get_nvram_size(), 0x00);
		}

		internal_header_logging(ROM, ((size >> 16) - 1) << 16, size);
	}

	return image_init_result::PASS;
}

/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void ws_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_nvram_base() && m_cart->get_nvram_size())
		battery_save(m_cart->get_nvram_base(), m_cart->get_nvram_size());
}


/*-------------------------------------------------
 get cart type from cart file
 -------------------------------------------------*/

int ws_cart_slot_device::get_cart_type(const u16 *ROM, u32 len, u32 &nvram_len) const
{
	int type = WS_STD;

	switch (ROM[(len >> 1) - 3] >> 8)
	{
		case 0x00:
			break;
		case 0x01:  // SRAM 64Kbit
			type = WS_SRAM;
			nvram_len = 0x2000;
			break;
		case 0x02:  // SRAM 256Kbit
			type = WS_SRAM;
			nvram_len = 0x8000;
			break;
		case 0x05:  // SRAM 512Kbit
			type = WS_SRAM;
			nvram_len = 0x10000;
			break;
		case 0x03:  // SRAM 1Mbit
			type = WS_SRAM;
			nvram_len = 0x20000;
			break;
		case 0x04:  // SRAM 2Mbit
			type = WS_SRAM;
			nvram_len = 0x40000;
			break;
		case 0x10:  // EEPROM 1Kbit
			type = WS_EEPROM;
			nvram_len = 0x80;
			break;
		case 0x50:  // EEPROM 8Kbit
			type = WS_EEPROM;
			nvram_len = 0x400;
			break;
		case 0x20:  // EEPROM 16Kbit
			type = WS_EEPROM;
			nvram_len = 0x800;
			break;
		default:
			logerror("Unknown RAM size [0x%X]\n", ROM[(len >> 1) - 3] >> 8);
			break;
	}

	return type;
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string ws_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		const char *slot_string;
		u32 size = hook.image_file()->size();
		std::vector<u16> rom(size);
		int type;
		u32 nvram;

		hook.image_file()->read(&rom[0], size);

		// nvram size is not really used here, but we set it up nevertheless
		type = get_cart_type(&rom[0], size, nvram);
		slot_string = ws_get_slot(type);

		return std::string(slot_string);
	}

	return software_get_default_slot("ws_rom");
}

/*-------------------------------------------------
 read_rom20
 -------------------------------------------------*/

u16 ws_cart_slot_device::read_rom20(offs_t offset, u16 mem_mask)
{
	if (m_cart)
		return m_cart->read_rom20(offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 read_rom30
 -------------------------------------------------*/

u16 ws_cart_slot_device::read_rom30(offs_t offset, u16 mem_mask)
{
	if (m_cart)
		return m_cart->read_rom30(offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 read_rom40
 -------------------------------------------------*/

u16 ws_cart_slot_device::read_rom40(offs_t offset, u16 mem_mask)
{
	if (m_cart)
		return m_cart->read_rom40(offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 read_ram
 -------------------------------------------------*/

u16 ws_cart_slot_device::read_ram(offs_t offset, u16 mem_mask)
{
	if (m_cart)
		return m_cart->read_ram(offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 write_ram
 -------------------------------------------------*/

void ws_cart_slot_device::write_ram(offs_t offset, u16 data, u16 mem_mask)
{
	if (m_cart)
		m_cart->write_ram(offset, data, mem_mask);
}

/*-------------------------------------------------
 read_io
 -------------------------------------------------*/

u16 ws_cart_slot_device::read_io(offs_t offset, u16 mem_mask)
{
	if (m_cart)
		return m_cart->read_io(offset & 0x07, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 write_io
 -------------------------------------------------*/

void ws_cart_slot_device::write_io(offs_t offset, u16 data, u16 mem_mask)
{
	if (m_cart)
		m_cart->write_io(offset & 0x07, data, mem_mask);
}



/*-------------------------------------------------
 Internal header logging
 -------------------------------------------------*/

static const char *const sram_str[] = { "none", "64Kbit SRAM", "256Kbit SRAM", "512Kbit SRAM", "1Mbit SRAM", "2Mbit SRAM" };
static const char *const eeprom_str[] = { "none", "1Kbit EEPROM", "16Kbit EEPROM", "Unknown", "Unknown", "8Kbit EEPROM" };
static const char *const romsize_str[] = { "Unknown", "Unknown", "4Mbit", "8Mbit", "16Mbit", "Unknown", "32Mbit", "Unknown", "64Mbit", "128Mbit" };

void ws_cart_slot_device::internal_header_logging(const u16 *ROM, u32 offs, u32 len)
{
	const int banks = len / 0x10000;
	const int words = len >> 1;
	const u8 romsize = ROM[words - 3] & 0xff;
	const u8 ramtype = (ROM[words - 3] & 0xf000) ? 1 : 0;  // 1 = EEPROM, 0 = SRAM
	const u8 ramsize = ramtype ? (((ROM[words - 3] >> 8) & 0xf0) >> 4) : ((ROM[words - 3] >> 8) & 0x0f);
	u16 sum = 0;

	logerror( "ROM DETAILS\n" );
	logerror( "===========\n\n" );
	logerror("\tDeveloper ID: %X\n", ROM[words - 5] & 0xff);
	logerror("\tMinimum system: %s\n", ROM[words - 5] & 0xff00 ? "WonderSwan Color" : "WonderSwan");
	logerror("\tCart ID: %X\n", ROM[words - 4] & 0xff);
	logerror("\tROM size: %s\n", romsize_str[romsize]);
	if (ramtype)
		logerror("\tEEPROM size: %s\n", (ramsize < 6) ? eeprom_str[ramsize] : "Unknown");
	else
		logerror("\tSRAM size: %s\n", (ramsize < 6) ? sram_str[ramsize] : "Unknown");
	logerror("\tFeatures: %X\n", ROM[words - 2] & 0xff);
	logerror("\tRTC: %s\n", (ROM[words - 2] & 0xff00) ? "yes" : "no");
	for (int i = 0; i < banks; i++)
	{
		for (int count = 0; count < 0x8000; count++)
		{
			sum += ROM[i * 0x8000 + count] & 0xff;
			sum += ROM[i * 0x8000 + count] >> 8;
		}
	}
	sum -= ROM[words - 1] & 0xff;
	sum -= ROM[words - 1] >> 8;
	sum &= 0xffff;
	logerror("\tChecksum: %04X (calculated: %04X)\n", ROM[words - 1], sum);
}
