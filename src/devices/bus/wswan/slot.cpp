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

const device_type WS_CART_SLOT = &device_creator<ws_cart_slot_device>;

//**************************************************************************
//    Wonderswan Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_ws_cart_interface - constructor
//-------------------------------------------------

device_ws_cart_interface::device_ws_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0),
		m_bank_mask(0),
		m_has_rtc(false),
		m_is_rotated(false)
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

void device_ws_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(WSSLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
		m_bank_mask = ((m_rom_size >> 16) - 1);
	}
}


//-------------------------------------------------
//  nvram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_ws_cart_interface::nvram_alloc(UINT32 size)
{
	m_nvram.resize(size);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ws_cart_slot_device - constructor
//-------------------------------------------------
ws_cart_slot_device::ws_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, WS_CART_SLOT, "Wonderswan Cartridge Slot", tag, owner, clock, "ws_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(WS_STD), m_cart(nullptr)
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
	m_cart = dynamic_cast<device_ws_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ws_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
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
	{ WS_EEPROM,   "ws_eeprom" }
};

static int ws_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
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

bool ws_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT8 *ROM;
		UINT32 size = (software_entry() == nullptr) ? length() : get_software_region_length("rom");
		UINT32 nvram_size = 0;

		m_cart->rom_alloc(size, tag());
		ROM = m_cart->get_rom_base();

		if (software_entry() == nullptr)
			fread(ROM, size);
		else
			memcpy(ROM, get_software_region("rom"), size);

		if (software_entry() == nullptr)
		{
			int chunks = size / 0x10000;
			// get cart type and nvram length
			m_type = get_cart_type(ROM, size, nvram_size);

			if (ROM[(chunks - 1) * 0x10000 + 0xfffd])
				m_cart->set_has_rtc(true);
			if (ROM[(chunks - 1) * 0x10000 + 0xfffc] & 0x01)
				m_cart->set_is_rotated(true);
		}
		else
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = ws_get_pcb_id(pcb_name);

			if (m_type == WS_SRAM)
				nvram_size = get_software_region_length("sram");
			if (m_type == WS_EEPROM)
				nvram_size = get_software_region_length("eeprom");

			if (get_feature("rtc"))
			{
				if (!core_stricmp(get_feature("rtc"), "yes"))
					m_cart->set_has_rtc(true);
			}
			if (get_feature("rotated"))
			{
				if (!core_stricmp(get_feature("rotated"), "yes"))
					m_cart->set_is_rotated(true);
			}
		}

		//printf("Type: %s\n", ws_get_slot(m_type));

		if (nvram_size)
		{
			// allocate NVRAM
			m_cart->nvram_alloc(nvram_size);
			// and load possible battery save
			battery_load(m_cart->get_nvram_base(), m_cart->get_nvram_size(), 0x00);
		}

		internal_header_logging(ROM, ((size >> 16) - 1) << 16, size);
	}

	return IMAGE_INIT_PASS;
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
 call softlist load
 -------------------------------------------------*/

bool ws_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}


/*-------------------------------------------------
 get cart type from cart file
 -------------------------------------------------*/

int ws_cart_slot_device::get_cart_type(UINT8 *ROM, UINT32 len, UINT32 &nvram_len)
{
	int chunks = len / 0x10000;
	int type = WS_STD;

	switch (ROM[(chunks - 1) * 0x10000 + 0xfffb])
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
			printf("Unknown RAM size [0x%X]\n", ROM[(chunks - 1) * 0x10000 + 0xfffb]);
			logerror("Unknown RAM size [0x%X]\n", ROM[(chunks - 1) * 0x10000 + 0xfffb]);
			break;
	}

	return type;
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

void ws_cart_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string;
		UINT32 size = core_fsize(m_file);
		dynamic_buffer rom(size);
		int type;
		UINT32 nvram;

		core_fread(m_file, &rom[0], size);

		// nvram size is not really used here, but we set it up nevertheless
		type = get_cart_type(&rom[0], size, nvram);
		slot_string = ws_get_slot(type);

		//printf("type: %s\n", slot_string);
		clear();

		result.assign(slot_string);
		return;
	}

	software_get_default_slot(result, "ws_rom");
}

/*-------------------------------------------------
 read_rom20
 -------------------------------------------------*/

READ8_MEMBER(ws_cart_slot_device::read_rom20)
{
	if (m_cart)
		return m_cart->read_rom20(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 read_rom30
 -------------------------------------------------*/

READ8_MEMBER(ws_cart_slot_device::read_rom30)
{
	if (m_cart)
		return m_cart->read_rom30(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 read_rom40
 -------------------------------------------------*/

READ8_MEMBER(ws_cart_slot_device::read_rom40)
{
	if (m_cart)
		return m_cart->read_rom40(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 read_ram
 -------------------------------------------------*/

READ8_MEMBER(ws_cart_slot_device::read_ram)
{
	if (m_cart)
		return m_cart->read_ram(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write_ram
 -------------------------------------------------*/

WRITE8_MEMBER(ws_cart_slot_device::write_ram)
{
	if (m_cart)
		m_cart->write_ram(space, offset, data);
}

/*-------------------------------------------------
 read_io
 -------------------------------------------------*/

READ8_MEMBER(ws_cart_slot_device::read_io)
{
	if (m_cart)
		return m_cart->read_io(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write_io
 -------------------------------------------------*/

WRITE8_MEMBER(ws_cart_slot_device::write_io)
{
	if (m_cart)
		m_cart->write_io(space, offset, data);
}



/*-------------------------------------------------
 Internal header logging
 -------------------------------------------------*/

static const char *const sram_str[] = { "none", "64Kbit SRAM", "256Kbit SRAM", "512Kbit SRAM", "1Mbit SRAM", "2Mbit SRAM" };
static const char *const eeprom_str[] = { "none", "1Kbit EEPROM", "16Kbit EEPROM", "Unknown", "Unknown", "8Kbit EEPROM" };
static const char *const romsize_str[] = { "Unknown", "Unknown", "4Mbit", "8Mbit", "16Mbit", "Unknown", "32Mbit", "Unknown", "64Mbit", "128Mbit" };

void ws_cart_slot_device::internal_header_logging(UINT8 *ROM, UINT32 offs, UINT32 len)
{
	int sum = 0, banks = len / 0x10000;
	UINT8 romsize, ramtype, ramsize;
	romsize = ROM[offs + 0xfffa];
	ramtype = (ROM[offs + 0xfffb] & 0xf0) ? 1 : 0;  // 1 = EEPROM, 0 = SRAM
	ramsize = ramtype ? ((ROM[offs + 0xfffb] & 0xf0) >> 4) : (ROM[offs + 0xfffb] & 0x0f);


	logerror( "ROM DETAILS\n" );
	logerror( "===========\n\n" );
	logerror("\tDeveloper ID: %X\n", ROM[offs + 0xfff6]);
	logerror("\tMinimum system: %s\n", ROM[offs + 0xfff7] ? "WonderSwan Color" : "WonderSwan");
	logerror("\tCart ID: %X\n", ROM[offs + 0xfff8]);
	logerror("\tROM size: %s\n", romsize_str[romsize]);
	if (ramtype)
		logerror("\tEEPROM size: %s\n", (ramsize < 6) ? eeprom_str[ramsize] : "Unknown");
	else
		logerror("\tSRAM size: %s\n", (ramsize < 6) ? sram_str[ramsize] : "Unknown");
	logerror("\tFeatures: %X\n", ROM[offs + 0xfffc]);
	logerror("\tRTC: %s\n", ROM[offs + 0xfffd] ? "yes" : "no");
	for (int i = 0; i < banks; i++)
	{
		for (int count = 0; count < 0x10000; count++)
		{
			sum += ROM[(i * 0x10000) + count];
		}
	}
	sum -= ROM[offs + 0xffff];
	sum -= ROM[offs + 0xfffe];
	sum &= 0xffff;
	logerror("\tChecksum: %.2X%.2X (calculated: %04X)\n", ROM[offs + 0xffff], ROM[offs + 0xfffe], sum);
}
