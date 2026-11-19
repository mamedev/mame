// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CBM2_EXPANSION_SLOT, cbm2_expansion_slot_device, "cbm2_expansion_slot", "CBM-II expansion port")

void cbm2_expansion_window::install_view(address_space_installer &program)
{
	program.install_view(m_start, m_start + 0x1fff, m_view);
	m_view[0];
}

void cbm2_expansion_window::install_rom(offs_t start, offs_t end, void *baseptr)
{
	m_view[0].install_rom(m_start + start, m_start + end, baseptr);
	m_view.select(0);
}

void cbm2_expansion_window::install_ram(offs_t start, offs_t end, void *baseptr)
{
	m_view[0].install_ram(m_start + start, m_start + end, baseptr);
	m_view.select(0);
}


//**************************************************************************
//  DEVICE CBM2_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_cbm2_expansion_card_interface - constructor
//-------------------------------------------------

device_cbm2_expansion_card_interface::device_cbm2_expansion_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "cbm2exp")
{
	m_slot = dynamic_cast<cbm2_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_cbm2_expansion_card_interface - destructor
//-------------------------------------------------

device_cbm2_expansion_card_interface::~device_cbm2_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_expansion_slot_device - constructor
//-------------------------------------------------

cbm2_expansion_slot_device::cbm2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CBM2_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_cbm2_expansion_card_interface>(mconfig, *this),
	device_cartrom_image_interface(mconfig, *this),
	m_program(*this, finder_base::DUMMY_TAG, -1),
	m_card(nullptr),
	m_bank1(*this, "bank1", 0xf2000),
	m_bank2(*this, "bank2", 0xf4000),
	m_bank3(*this, "bank3", 0xf6000)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_expansion_slot_device::device_start()
{
	m_card = get_card_device();

	if (m_program)
		install_program_views(*m_program.target());
}

void cbm2_expansion_slot_device::install_program_views(address_space_installer &program)
{
	for (cbm2_expansion_window *window : { &m_bank1, &m_bank2, &m_bank3 })
		window->install_view(program);
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

std::pair<std::error_condition, std::string> cbm2_expansion_slot_device::call_load()
{
	if (!m_card || loaded_through_softlist())
		return {};

	static char const *const banks[] = { "bank1", "bank2", "bank3" };

	for (char const *bank : banks)
		machine().memory().region_free(subtag(bank));

	int first;
	if (is_filetype("20"))
		first = 0;
	else if (is_filetype("40"))
		first = 1;
	else if (is_filetype("60"))
		first = 2;
	else
		return { image_error::INVALIDIMAGE, std::string() };

	util::random_read &file = image_core_file();
	size_t const size = length();

	if (!size || (size > (3 - first) * 0x2000))
		return { image_error::INVALIDLENGTH, std::string() };

	for (size_t offset = 0; offset < size; offset += 0x2000)
	{
		size_t const block = std::min<size_t>(size - offset, 0x2000);
		auto const [err, actual] = util::read(file, alloc_region(banks[first + offset / 0x2000]), block);
		if (err)
			return { err, std::string() };
		if (actual != block)
			return { std::errc::io_error, std::string() };
	}

	return {};
}


//-------------------------------------------------
//  alloc_region - allocate a ROM block region
//  named like the software list data area
//-------------------------------------------------

uint8_t *cbm2_expansion_slot_device::alloc_region(const char *tag)
{
	machine().memory().region_free(subtag(tag));
	memory_region *const region = machine().memory().region_alloc(subtag(tag), 0x2000, 1, ENDIANNESS_LITTLE);
	std::fill_n(region->base(), region->bytes(), 0);

	return region->base();
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string cbm2_expansion_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  SLOT_INTERFACE( cbm2_expansion_cards )
//-------------------------------------------------

// slot devices
#include "24k.h"
#include "hrg.h"
#include "std.h"

void cbm2_expansion_cards(device_slot_interface &device)
{
	device.option_add("24k", CBM2_24K);
	device.option_add("hrga", CBM2_HRG_A);
	device.option_add("hrgb", CBM2_HRG_B);
	device.option_add_internal("standard", CBM2_STD);
}
