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
	device_memory_interface(mconfig, *this),
	m_card(nullptr),
	m_space_config("cart", ENDIANNESS_LITTLE, 8, 15, 0, address_map_constructor(FUNC(cbm2_expansion_slot_device::cart_map), this)),
	m_bank1(0x2000),
	m_bank2(0x4000),
	m_bank3(0x6000),
	m_data(0xff)
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector cbm2_expansion_slot_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


//-------------------------------------------------
//  cart_map -
//-------------------------------------------------

void cbm2_expansion_slot_device::cart_map(address_map &map)
{
	map(0x0000, 0x7fff).lr8(NAME([this] () { return m_data; })).nopw();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_expansion_slot_device::device_start()
{
	m_card = get_card_device();

	for (cbm2_expansion_window *window : { &m_bank1, &m_bank2, &m_bank3 })
		window->m_space = &space(0);
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
//  read - cartridge data read
//-------------------------------------------------

uint8_t cbm2_expansion_slot_device::read(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3)
{
	int const bank = !csbank1 ? 1 : !csbank2 ? 2 : !csbank3 ? 3 : 0;

	if (!bank)
		return data;

	m_data = data;

	return space(0).read_byte((bank << 13) | (offset & 0x1fff));
}


//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void cbm2_expansion_slot_device::write(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3)
{
	int const bank = !csbank1 ? 1 : !csbank2 ? 2 : !csbank3 ? 3 : 0;

	if (bank)
		space(0).write_byte((bank << 13) | (offset & 0x1fff), data);
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
