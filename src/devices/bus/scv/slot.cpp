// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    Epoch Super Cassette Vision cart emulation
    (through slot devices)

 ***********************************************************************************************************/

#include "emu.h"
#include "slot.h"


DEFINE_DEVICE_TYPE(SCV_CART_SLOT, scv_cart_slot_device, "scv_cart_slot", "SCV Cartridge Slot")


enum
{
	SCV_8K = 0,
	SCV_16K,
	SCV_32K,
	SCV_32K_RAM,
	SCV_64K,
	SCV_128K,
	SCV_128K_RAM
};

//**************************************************************************
//    SCV cartridges Interface
//**************************************************************************

device_scv_cart_interface::device_scv_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "scvcart"),
	m_slot(dynamic_cast<scv_cart_slot_device *>(device.owner()))
{
}


device_scv_cart_interface::~device_scv_cart_interface()
{
}


void device_scv_cart_interface::savestate_ram()
{
	if (cart_ram_region())
	{
		u8 *rambase(&cart_ram_region()->as_u8());
		device().save_pointer(NAME(rambase), cart_ram_region()->bytes());
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

scv_cart_slot_device::scv_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SCV_CART_SLOT, tag, owner, clock),
	device_cartrom_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_scv_cart_interface>(mconfig, *this),
	m_type(SCV_8K),
	m_cart(nullptr),
	m_address_space(*this, finder_base::DUMMY_TAG, -1, 8)
{
}


scv_cart_slot_device::~scv_cart_slot_device()
{
}


void scv_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}


//-------------------------------------------------
//  SCV PCB
//-------------------------------------------------

struct scv_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const scv_slot slot_list[] =
{
	{ SCV_8K,       "rom8k" },
	{ SCV_16K,      "rom16k" },
	{ SCV_32K,      "rom32k" },
	{ SCV_32K_RAM,  "rom32k_ram" },
	{ SCV_64K,      "rom64k" },
	{ SCV_128K,     "rom128k" },
	{ SCV_128K_RAM, "rom128k_ram" }
};

static int scv_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!strcmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return slot_list[0].pcb_id;
}

static const char *scv_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return slot_list[0].slot_option;
}


std::pair<std::error_condition, std::string> scv_cart_slot_device::call_load()
{
	if (m_cart)
	{
		const u32 len = loaded_through_softlist() ? get_software_region_length("rom") : length();
		const bool has_ram = loaded_through_softlist() && get_software_region("ram");

		if (len > 0x20000)
			return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be no more than 128K)");

		if (!loaded_through_softlist())
		{
			u32 length_aligned = 0x2000;
			while (length_aligned < len)
				length_aligned *= 2;

			memory_region *const romregion = machine().memory().region_alloc(subtag("rom"), length_aligned, 1, ENDIANNESS_LITTLE);
			if (fread(romregion->base(), len) != len)
				return std::make_pair(image_error::UNSPECIFIED, "Unable to fully read file");

			m_type = get_cart_type(len);
		}
		else
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = scv_get_pcb_id(pcb_name);
		}

		// for the moment we only support RAM from softlist and in the following configurations
		// 1) 32K ROM + 8K RAM; 2) 128K ROM + 4K RAM
		if (m_type == SCV_32K && has_ram)
			m_type = SCV_32K_RAM;
		if (m_type == SCV_128K && has_ram)
			m_type = SCV_128K_RAM;

		m_cart->install_memory_handlers(m_address_space.target());
		m_cart->savestate_ram();
	}

	return std::make_pair(std::error_condition(), std::string());
}


int scv_cart_slot_device::get_cart_type(u32 len)
{
	int type = SCV_8K;

	// TODO: is there any way to identify carts with RAM?!?
	switch (len)
	{
		case 0x2000:
			type = SCV_8K;
			break;
		case 0x4000:
			type = SCV_16K;
			break;
		case 0x8000:
			type = SCV_32K;
			break;
		case 0x10000:
			type = SCV_64K;
			break;
		case 0x20000:
			type = SCV_128K;
			break;
	}

	return type;
}


std::string scv_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		u64 len = 0;
		hook.image_file()->length(len); // FIXME: check error return, guard against excessively large files

		const int type = get_cart_type(len);
		const char *const slot_string = scv_get_slot(type);

		return std::string(slot_string);
	}

	return software_get_default_slot(slot_list[0].slot_option);
}


void scv_cart_slot_device::write_bank(u8 data)
{
	if (m_cart)
		m_cart->write_bank(data);
}
