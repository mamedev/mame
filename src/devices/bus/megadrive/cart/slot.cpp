// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "slot.h"

#include "options.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>


DEFINE_DEVICE_TYPE(MEGADRIVE_CART_SLOT, megadrive_cart_slot_device, "megadrive_cart_slot", "MegaDrive Cartridge Slot")

device_megadrive_cart_interface::device_megadrive_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "megadrive_cart")
	, m_slot(dynamic_cast<megadrive_cart_slot_device *>(device.owner()))
{
}

device_megadrive_cart_interface::~device_megadrive_cart_interface()
{
}

void device_megadrive_cart_interface::interface_pre_start()
{
	if (!m_slot->started())
		throw device_missing_dependencies();
}

void device_megadrive_cart_interface::interface_post_start()
{
	m_slot->m_space_mem->install_device(0x000000, 0xdfffff, *this, &device_megadrive_cart_interface::cart_map);
	m_slot->m_space_io->install_device(0x00, 0xff, *this, &device_megadrive_cart_interface::time_io_map);
}

void device_megadrive_cart_interface::cart_map(address_map &map)
{
	// NOTE: is host responsibility to handle open bus access
//	map(0x000000, 0x3fffff).unmaprw();
}

// $a13000 base
void device_megadrive_cart_interface::time_io_map(address_map &map)
{
//	map(0x00, 0xff).unmaprw();
}


//void device_megadrive_cart_interface::rom_alloc(size_t size)
//{
//	if (m_rom == nullptr)
//	{
//		m_rom = (uint16_t *)device().machine().memory().region_alloc(device().subtag("^cart:rom"), size, 2, ENDIANNESS_BIG)->base();
//		m_rom_size = size;
//		m_rom_mask = size - 1;
//	}
//}


megadrive_cart_slot_device::megadrive_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_CART_SLOT, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_megadrive_cart_interface>(mconfig, *this)
	, m_cart(nullptr)
	, m_space_mem_config("cart_mem", ENDIANNESS_BIG, 16, 24, 0, address_map_constructor())
	, m_space_io_config("time_io", ENDIANNESS_BIG, 16, 8, 0, address_map_constructor())
{
}

megadrive_cart_slot_device::~megadrive_cart_slot_device()
{
}

device_memory_interface::space_config_vector megadrive_cart_slot_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_mem_config),
		std::make_pair(AS_IO, &m_space_io_config)
	};
}

void megadrive_cart_slot_device::device_start()
{
	m_cart = get_card_device();
	m_space_mem = &space(AS_PROGRAM);
	m_space_io = &space(AS_IO);
}

const char *megadrive_cart_slot_device::get_cart_type(const uint8_t *ROM, uint32_t len)
{
	using namespace bus::megadrive;

//	int type = slotoptions::MD_STD;
	// TODO: not caring about loose detection for now
	return slotoptions::MD_STD;
}

//static int md_get_pcb_id(const char *slot)
//{
//	for (auto & elem : slot_list)
//	{
//		if (!strcmp(elem.slot_option, slot))
//			return elem.pcb_id;
//	}
//
//	return MD_STD;
//}

//static const char *md_get_slot(int type)
//{
//	for (auto & elem : slot_list)
//	{
//		if (elem.pcb_id == type)
//			return elem.slot_option;
//	}
//
//	return "rom";
//}

std::pair<std::error_condition, std::string> megadrive_cart_slot_device::call_load()
{
	if (m_cart)
	{
		std::error_condition err;

		if (loaded_through_softlist())
		{
			err = load_swlist();
		}
		else
		{
			err = load_loose();
		}

		if (err)
			return std::make_pair(err ? err : std::errc::io_error, "Error loading detection");
	}

	return std::make_pair(std::error_condition(), std::string());
}

std::error_condition megadrive_cart_slot_device::load_swlist()
{
	using namespace bus::megadrive;

//	uint16_t *ROM;
//	uint32_t length = get_software_region_length("rom");
	const char  *slot_name;
//
//	// if cart size is not (2^n * 64K), the system will see anyway that size so we need to alloc a bit more space
//	length = m_cart->get_padded_size(length);
//
//	memory_region *const romregion = machine().memory().region_alloc(subtag("rom"), length, 2, ENDIANNESS_BIG);
//	auto const [err, actual] = read_at(file, offset, romregion->base(), len);
//	if (err || (len != actual))
//		return std::make_pair(err ? err : std::errc::io_error, "Error reading ROM data from cartridge file");
//
////	m_cart->rom_alloc(length);
////	ROM = m_cart->get_rom_base();
//	memcpy(romregion, get_software_region("rom"), get_software_region_length("rom"));
//
//	// if we allocated a ROM larger that the file (e.g. due to uneven cart size), set remaining space to 0xff
//	if (length > get_software_region_length("rom"))
//		memset(romregion + get_software_region_length("rom")/2, 0xffff, (length - get_software_region_length("rom"))/2);
//
	slot_name = get_feature("slot");
	if (slot_name == nullptr)
		slot_name = slotoptions::MD_STD;

//	printf("%s\n", slot_name);
	return std::error_condition(m_cart->load());
}

std::error_condition megadrive_cart_slot_device::load_loose()
{
	return std::error_condition();

//	unsigned char *ROM;
//	bool is_smd, is_md;
//	uint32_t tmplen = length(), offset, len;
//	std::vector<uint8_t> tmpROM(tmplen);
//
//	// STEP 1: store a (possibly headered) copy of the file and determine its type (SMD? MD? BIN?)
//	fread(&tmpROM[0], tmplen);
//	is_smd = false; //genesis_is_SMD(&tmpROM[0x200], tmplen - 0x200);
//	is_md = (tmpROM[0x80] == 'E') && (tmpROM[0x81] == 'A') && (tmpROM[0x82] == 'M' || tmpROM[0x82] == 'G');
//
//	// take header into account, if any
//	offset = is_smd ? 0x200 : 0;
//
//	// STEP 2: allocate space for the real copy of the game
//	// if cart size is not (2^n * 64K), the system will see anyway that size so we need to alloc a bit more space
//	len = m_cart->get_padded_size(tmplen - offset);
//
//	// this contains an hack for SSF2: its current bankswitch code needs larger ROM space to work
//	// m_cart->rom_alloc((len == 0x500000) ? 0x900000 : len);
//
//	// STEP 3: copy the game data in the appropriate way
//	ROM = (unsigned char *)m_cart->get_rom_base();
//
//	if (is_smd)
//	{
//		osd_printf_debug("SMD!\n");
//
//		for (int ptr = 0; ptr < (tmplen - 0x200) / 0x2000; ptr += 2)
//		{
//			for (int x = 0; x < 0x2000; x++)
//			{
//				ROM[ptr * 0x2000 + x * 2 + 0] = tmpROM[0x200 + ((ptr + 1) * 0x2000) + x];
//				ROM[ptr * 0x2000 + x * 2 + 1] = tmpROM[0x200 + ((ptr + 0) * 0x2000) + x];
//			}
//		}
//	}
//	else if (is_md)
//	{
//		osd_printf_debug("MD!\n");
//
//		for (int ptr = 0; ptr < tmplen; ptr += 2)
//		{
//			ROM[ptr] = tmpROM[(tmplen >> 1) + (ptr >> 1)];
//			ROM[ptr + 1] = tmpROM[(ptr >> 1)];
//		}
//	}
//	else
//	{
//		osd_printf_debug("BIN!\n");
//
//		fseek(0, SEEK_SET);
//		fread(ROM, len);
//	}
//
//	// if we allocated a ROM larger that the file (e.g. due to uneven cart size), set remaining space to 0xff
//	//if (len > (tmplen - offset))
//	//	memset(m_cart->get_rom_base() + (tmplen - offset)/2, 0xffff, (len - tmplen + offset)/2);
//
//	// STEP 4: determine the cart type (to deal with sram/eeprom & pirate mappers)
//	// m_type = get_cart_type(ROM, tmplen - offset);
//
//// CPU needs to access ROM as a ROM_REGION16_BE, so we need to compensate on LE machines
//#ifdef LSB_FIRST
//	unsigned char fliptemp;
//	for (int ptr = 0; ptr < len; ptr += 2)
//	{
//		fliptemp = ROM[ptr];
//		ROM[ptr] = ROM[ptr+1];
//		ROM[ptr+1] = fliptemp;
//	}
//#endif
//
//	return std::error_condition();
}


std::string megadrive_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	using namespace bus::megadrive;

	if (hook.image_file())
	{
		uint64_t len;

		if (hook.image_file()->length(len))
		{
			osd_printf_warning("[%s] Error getting cartridge ROM length - defaulting to linear ROM type\n", tag());
			return slotoptions::MD_STD;
		}

		std::vector<uint8_t> rom(len);

		// FIXME: check error return or read returning short
		util::read(*hook.image_file(), &rom[0], len);

		uint32_t const offset = 0; // genesis_is_SMD(&rom[0x200], len - 0x200) ? 0x200 : 0;

//		int const type = get_cart_type(&rom[offset], len - offset);
		char const *const slot_string = get_cart_type(&rom[offset], len - offset);

		return std::string(slot_string);
	}
	else
	{
		return software_get_default_slot(slotoptions::MD_STD);
	}
}


uint32_t device_megadrive_cart_interface::get_padded_size(uint32_t size)
{
	uint32_t pad_size = 0x10000;
	while (size > pad_size)
		pad_size <<= 1;

	if (pad_size < 0x800000 && size < pad_size)
		return pad_size;
	else
		return size;
}


void megadrive_cart_slot_device::call_unload()
{
	if (m_cart)
		m_cart->unload();
//	if (m_cart && m_cart->get_nvram_base() && m_cart->get_nvram_size())
//		battery_save(m_cart->get_nvram_base(), m_cart->get_nvram_size());
}

// 0x000000-0x3fffff
// shift << 1 here and not in memory constructor for two reasons:
// 1. cart has no A0 connected
// 2. memory_views would get confused by the shift with
//    "A memory_view must be installed at its configuration address."
u16 megadrive_cart_slot_device::base_r(offs_t offset, u16 mem_mask)
{
	return m_space_mem->read_word(offset << 1, mem_mask);
}

void megadrive_cart_slot_device::base_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_space_mem->write_word(offset << 1, data, mem_mask);
}

u16 megadrive_cart_slot_device::time_r(offs_t offset, u16 mem_mask)
{
	return m_space_io->read_word(offset << 1, mem_mask);
}

void megadrive_cart_slot_device::time_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_space_io->write_word(offset << 1, data, mem_mask);
}

