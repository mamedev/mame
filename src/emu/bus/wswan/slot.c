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
		m_rom(NULL),
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
	if (m_rom == NULL)
	{
		astring tempstring(tag);
		tempstring.cat(WSSLOT_ROM_REGION_TAG);
		m_rom = device().machine().memory().region_alloc(tempstring, size, 1, ENDIANNESS_LITTLE)->base();
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
						m_type(WS_STD)
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
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!core_stricmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return 0;
}

static const char *ws_get_slot(int type)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
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
		UINT32 size = (software_entry() == NULL) ? length() : get_software_region_length("rom");
		UINT32 nvram_size = 0;
		
		m_cart->rom_alloc(size, tag());
		ROM = m_cart->get_rom_base();
			
		if (software_entry() == NULL)
			fread(ROM, size);
		else
			memcpy(ROM, get_software_region("rom"), size);

		if (software_entry() == NULL)
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

		internal_header_logging(ROM, size);
	}

	return IMAGE_INIT_PASS;
}

/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void ws_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_nvram_size())
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
		case 0x01:	// SRAM 64Kbit
			type = WS_SRAM;
			nvram_len = 0x2000;
			break;
		case 0x02:	// SRAM 256Kbit
			type = WS_SRAM;
			nvram_len = 0x8000;
			break;
		case 0x05:	// SRAM 512Kbit
			type = WS_SRAM;
			nvram_len = 0x10000;
			break;
		case 0x03:	// SRAM 1Mbit
			type = WS_SRAM;
			nvram_len = 0x20000;
			break;
		case 0x04:	// SRAM 2Mbit
			type = WS_SRAM;
			nvram_len = 0x40000;
			break;
		case 0x10:	// EEPROM 1Kbit
			type = WS_EEPROM;
			nvram_len = 0x80;
			break;
		case 0x50:	// EEPROM 8Kbit
			type = WS_EEPROM;
			nvram_len = 0x400;
			break;
		case 0x20:	// EEPROM 16Kbit
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

void ws_cart_slot_device::get_default_card_software(astring &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "ws_rom";
		UINT32 size = core_fsize(m_file);
		dynamic_buffer rom(size);
		int type;
		UINT32 nvram;
		
		core_fread(m_file, rom, size);

		// nvram size is not really used here, but we set it up nevertheless
		type = get_cart_type(rom, size, nvram);
		slot_string = ws_get_slot(type);

		//printf("type: %s\n", slot_string);
		clear();

		result.cpy(slot_string);
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

void ws_cart_slot_device::internal_header_logging(UINT8 *ROM, UINT32 len)
{
#if 0
	enum enum_sram { SRAM_NONE=0, SRAM_64K, SRAM_256K, SRAM_512K, SRAM_1M, SRAM_2M, EEPROM_1K, EEPROM_16K, EEPROM_8K, SRAM_UNKNOWN };
	static const char *const wswan_sram_str[] = { "none", "64Kbit SRAM", "256Kbit SRAM", "512Kbit SRAM", "1Mbit SRAM", "2Mbit SRAM", "1Kbit EEPROM", "16Kbit EEPROM", "8Kbit EEPROM", "Unknown" };
	static const int wswan_sram_size[] = { 0, 64*1024/8, 256*1024/8, 512*1024/8, 1024*1024/8, 2*1024*1024/8,  1024/8, 16*1024/8, 8*1024/8, 0 };
	
	int sum = 0;
	/* Spit out some info */
	logerror("ROM DETAILS\n" );
	logerror("\tDeveloper ID: %X\n", m_ROMMap[m_ROMBanks - 1][0xfff6]);
	logerror("\tMinimum system: %s\n", m_ROMMap[m_ROMBanks - 1][0xfff7] ? "WonderSwan Color" : "WonderSwan");
	logerror("\tCart ID: %X\n", m_ROMMap[m_ROMBanks - 1][0xfff8]);
	logerror("\tROM size: %s\n", wswan_determine_romsize(m_ROMMap[m_ROMBanks - 1][0xfffa]));
	logerror("\tSRAM size: %s\n", sram_str);
	logerror("\tFeatures: %X\n", m_ROMMap[m_ROMBanks - 1][0xfffc]);
	logerror("\tRTC: %s\n", m_ROMMap[m_ROMBanks - 1][0xfffd] ? "yes" : "no");
	for (int i = 0; i < m_ROMBanks; i++)
	{
		int count;
		for (count = 0; count < 0x10000; count++)
		{
			sum += m_ROMMap[i][count];
		}
	}
	sum -= m_ROMMap[m_ROMBanks - 1][0xffff];
	sum -= m_ROMMap[m_ROMBanks - 1][0xfffe];
	sum &= 0xffff;
	logerror("\tChecksum: %X%X (calculated: %04X)\n", m_ROMMap[m_ROMBanks - 1][0xffff], m_ROMMap[m_ROMBanks - 1][0xfffe], sum);

	const char* wswan_state::wswan_determine_sram(UINT8 data )
	{
		m_eeprom.write_enabled = 0;
		m_eeprom.mode = SRAM_UNKNOWN;
		switch( data )
		{
			case 0x00: m_eeprom.mode = SRAM_NONE; break;
			case 0x01: m_eeprom.mode = SRAM_64K; break;
			case 0x02: m_eeprom.mode = SRAM_256K; break;
			case 0x03: m_eeprom.mode = SRAM_1M; break;
			case 0x04: m_eeprom.mode = SRAM_2M; break;
			case 0x05: m_eeprom.mode = SRAM_512K; break;
			case 0x10: m_eeprom.mode = EEPROM_1K; break;
			case 0x20: m_eeprom.mode = EEPROM_16K; break;
			case 0x50: m_eeprom.mode = EEPROM_8K; break;
		}
		m_eeprom.size = wswan_sram_size[ m_eeprom.mode ];
		return wswan_sram_str[ m_eeprom.mode ];
	}
	
	enum enum_romsize { ROM_4M=0, ROM_8M, ROM_16M, ROM_32M, ROM_64M, ROM_128M, ROM_UNKNOWN };
	static const char *const wswan_romsize_str[] = {
		"4Mbit", "8Mbit", "16Mbit", "32Mbit", "64Mbit", "128Mbit", "Unknown"
	};
	
	const char* wswan_state::wswan_determine_romsize( UINT8 data )
	{
		switch( data )
		{
			case 0x02:  return wswan_romsize_str[ ROM_4M ];
			case 0x03:  return wswan_romsize_str[ ROM_8M ];
			case 0x04:  return wswan_romsize_str[ ROM_16M ];
			case 0x06:  return wswan_romsize_str[ ROM_32M ];
			case 0x08:  return wswan_romsize_str[ ROM_64M ];
			case 0x09:  return wswan_romsize_str[ ROM_128M ];
		}
		return wswan_romsize_str[ ROM_UNKNOWN ];
	}
	
#endif
}
