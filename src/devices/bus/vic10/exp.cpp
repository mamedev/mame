// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-10 Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"

#include "formats/cbm_crt.h"

#include <tuple>


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC10_EXPANSION_SLOT, vic10_expansion_slot_device, "vic10_expansion_slot", "VIC-10 expansion port")



//**************************************************************************
//  DEVICE VIC10_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vic10_expansion_card_interface - constructor
//-------------------------------------------------

device_vic10_expansion_card_interface::device_vic10_expansion_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "vic10exp")
{
	m_slot = dynamic_cast<vic10_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_vic10_expansion_card_interface - destructor
//-------------------------------------------------

device_vic10_expansion_card_interface::~device_vic10_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic10_expansion_slot_device - constructor
//-------------------------------------------------

vic10_expansion_slot_device::vic10_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIC10_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_vic10_expansion_card_interface>(mconfig, *this),
	device_cartrom_image_interface(mconfig, *this),
	m_write_irq(*this),
	m_write_res(*this),
	m_write_cnt(*this),
	m_write_sp(*this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic10_expansion_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

std::pair<std::error_condition, std::string> vic10_expansion_slot_device::call_load()
{
	std::error_condition err;

	if (m_card)
	{
		if (!loaded_through_softlist())
		{
			util::core_file &file = image_core_file();
			size_t const size = length();

			if (is_filetype("80"))
			{
				size_t actual;
				std::tie(err, m_card->m_lorom, actual) = read(file, 0x2000);
				if (!err && (actual != 0x2000))
					err = std::errc::io_error;

				if (!err && (size == 0x4000))
				{
					std::tie(err, m_card->m_uprom, actual) = read(file, 0x2000);
					if (!err && (actual != 0x2000))
						err = std::errc::io_error;
				}
			}
			else if (is_filetype("e0"))
			{
				size_t actual;
				std::tie(err, m_card->m_uprom, actual) = read(file, size);
				if (!err && (actual != size))
					err = std::errc::io_error;
			}
			else if (is_filetype("crt"))
			{
				size_t roml_size = 0;
				size_t romh_size = 0;
				int exrom = 1;
				int game = 1;

				if (cbm_crt_read_header(file, &roml_size, &romh_size, &exrom, &game))
				{
					uint8_t *roml = nullptr;
					uint8_t *romh = nullptr;

					m_card->m_lorom = std::make_unique<uint8_t[]>(roml_size);
					m_card->m_uprom = std::make_unique<uint8_t[]>(romh_size);

					if (roml_size) roml = m_card->m_lorom.get();
					if (romh_size) romh = m_card->m_lorom.get();

					cbm_crt_read_data(file, roml, romh);
				}
			}
			else
			{
				err = image_error::INVALIDIMAGE;
			}
		}
		else
		{
			load_software_region("lorom", m_card->m_lorom);
			load_software_region("exram", m_card->m_exram);
			load_software_region("uprom", m_card->m_uprom);
		}
	}

	return std::make_pair(err, std::string());
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string vic10_expansion_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		if (hook.is_filetype("crt"))
			return cbm_crt_get_card(*hook.image_file());
	}

	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

uint8_t vic10_expansion_slot_device::cd_r(offs_t offset, uint8_t data, int lorom, int uprom, int exram)
{
	if (m_card != nullptr)
	{
		data = m_card->vic10_cd_r(offset, data, lorom, uprom, exram);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void vic10_expansion_slot_device::cd_w(offs_t offset, uint8_t data, int lorom, int uprom, int exram)
{
	if (m_card != nullptr)
	{
		m_card->vic10_cd_w(offset, data, lorom, uprom, exram);
	}
}

int vic10_expansion_slot_device::p0_r() { int state = 0; if (m_card != nullptr) state = m_card->vic10_p0_r(); return state; }
void vic10_expansion_slot_device::p0_w(int state) { if (m_card != nullptr) m_card->vic10_p0_w(state); }


//-------------------------------------------------
//  SLOT_INTERFACE( vic10_expansion_cards )
//-------------------------------------------------

// slot devices
#include "std.h"
#include "multimax.h"

void vic10_expansion_cards(device_slot_interface &device)
{
	// the following need ROMs from the software list
	device.option_add_internal("standard", VIC10_STD);
	device.option_add_internal("multimax", VIC10_MULTIMAX);
}
