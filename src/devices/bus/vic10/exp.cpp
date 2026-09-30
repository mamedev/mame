// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-10 Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"

#include "formats/cbm_crt.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC10_EXPANSION_SLOT, vic10_expansion_slot_device, "vic10_expansion_slot", "VIC-10 expansion port")



//**************************************************************************
//  VIC10 EXPANSION WINDOW
//**************************************************************************

vic10_expansion_window::vic10_expansion_window(device_t &device, const char *name, offs_t start, offs_t end)
	: m_view(device, name)
	, m_video_view(device, std::string(name) + "_video")
	, m_start(start)
	, m_end(end)
	, m_video_start(0)
	, m_video_end(0)
	, m_has_video(false)
	, m_video_installed(false)
{
}

vic10_expansion_window::vic10_expansion_window(device_t &device, const char *name, offs_t start, offs_t end, offs_t video_start, offs_t video_end)
	: m_view(device, name)
	, m_video_view(device, std::string(name) + "_video")
	, m_start(start)
	, m_end(end)
	, m_video_start(video_start)
	, m_video_end(video_end)
	, m_has_video(true)
	, m_video_installed(false)
{
}

void vic10_expansion_window::install_views(address_space &program, address_space *video)
{
	program.install_view(m_start, m_end, m_view);
	m_view[0];

	if (m_has_video && video)
	{
		video->install_view(m_video_start, m_video_end, m_video_view);
		m_video_view[0];
		m_video_installed = true;
	}
}

void vic10_expansion_window::select(int slot)
{
	m_view.select(slot);

	if (m_video_installed)
		m_video_view.select(slot);
}

void vic10_expansion_window::unmap()
{
	m_view.disable();

	if (m_video_installed)
		m_video_view.disable();
}

void vic10_expansion_window::variant::install(offs_t start, offs_t end, void *baseptr, bool writable)
{
	if (writable)
		m_window.m_view[m_slot].install_ram(m_window.m_start + start, m_window.m_start + end, baseptr);
	else
		m_window.m_view[m_slot].install_rom(m_window.m_start + start, m_window.m_start + end, baseptr);

	if (!m_window.m_video_installed)
		return;

	offs_t const video_offset = (m_window.m_end - m_window.m_start) - (m_window.m_video_end - m_window.m_video_start);
	offs_t const video_first = std::max(start, video_offset);

	if (video_first <= end)
		m_window.m_video_view[m_slot].install_rom(m_window.m_video_start + video_first - video_offset, m_window.m_video_start + end - video_offset, reinterpret_cast<uint8_t *>(baseptr) + (video_first - start));
}



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
	m_program(*this, finder_base::DUMMY_TAG, -1),
	m_video(*this, finder_base::DUMMY_TAG, -1),
	m_write_irq(*this),
	m_write_res(*this),
	m_write_cnt(*this),
	m_write_sp(*this),
	m_card(nullptr),
	m_exram(*this, "exram", 0x0800, 0x0fff, 0x0800, 0x0fff),
	m_lorom(*this, "lorom", 0x8000, 0x9fff),
	m_uprom(*this, "uprom", 0xe000, 0xffff, 0x3000, 0x3fff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic10_expansion_slot_device::device_start()
{
	m_card = get_card_device();

	if (m_program)
	{
		for (vic10_expansion_window *window : { &m_exram, &m_lorom, &m_uprom })
			window->install_views(*m_program.target(), m_video.target());
	}
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

std::pair<std::error_condition, std::string> vic10_expansion_slot_device::call_load()
{
	if (!m_card || loaded_through_softlist())
		return {};

	machine().memory().region_free(subtag("lorom"));
	machine().memory().region_free(subtag("uprom"));

	util::random_read &file = image_core_file();
	size_t const size = length();

	if (is_filetype("80"))
	{
		if ((size != 0x2000) && (size != 0x4000))
			return { image_error::INVALIDLENGTH, std::string() };

		std::error_condition err = load_region(file, "lorom", 0x0000, 0x2000);
		if (!err && (size == 0x4000))
			err = load_region(file, "uprom", 0x0000, 0x2000);

		return { err, std::string() };
	}
	else if (is_filetype("e0"))
	{
		if (!size || (size > 0x2000))
			return { image_error::INVALIDLENGTH, std::string() };

		return { load_region(file, "uprom", 0x2000 - size, size), std::string() };
	}
	else if (is_filetype("crt"))
	{
		size_t roml_size = 0;
		size_t romh_size = 0;
		int exrom = 1;
		int game = 1;

		if (!cbm_crt_read_header(file, &roml_size, &romh_size, &exrom, &game))
			return { image_error::INVALIDIMAGE, std::string() };

		if ((roml_size > 0x2000) || (romh_size > 0x2000))
			return { image_error::INVALIDLENGTH, std::string() };

		uint8_t *const roml = roml_size ? alloc_region("lorom") : nullptr;
		uint8_t *const romh = romh_size ? alloc_region("uprom") + 0x2000 - romh_size : nullptr;

		if (!cbm_crt_read_data(file, roml, romh))
			return { image_error::INVALIDIMAGE, std::string() };

		return {};
	}

	return { image_error::INVALIDIMAGE, std::string() };
}


//-------------------------------------------------
//  alloc_region - allocate a ROM block region
//  named like the software list data area
//-------------------------------------------------

uint8_t *vic10_expansion_slot_device::alloc_region(const char *tag)
{
	machine().memory().region_free(subtag(tag));
	memory_region *const region = machine().memory().region_alloc(subtag(tag), 0x2000, 1, ENDIANNESS_LITTLE);
	std::fill_n(region->base(), region->bytes(), 0);

	return region->base();
}


//-------------------------------------------------
//  load_region - load a ROM block into a region
//-------------------------------------------------

std::error_condition vic10_expansion_slot_device::load_region(util::random_read &file, const char *tag, offs_t offset, size_t length)
{
	auto const [err, actual] = read(file, alloc_region(tag) + offset, length);
	if (!err && (actual != length))
		return std::errc::io_error;

	return err;
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


int vic10_expansion_slot_device::p0_r() { int state = 0; if (m_card != nullptr) state = m_card->vic10_p0_r(); return state; }
void vic10_expansion_slot_device::p0_w(int state) { if (m_card != nullptr) m_card->vic10_p0_w(state); }


//-------------------------------------------------
//  SLOT_INTERFACE( vic10_expansion_cards )
//-------------------------------------------------

// slot devices
#include "basic.h"
#include "multimax.h"
#include "std.h"

void vic10_expansion_cards(device_slot_interface &device)
{
	// the following need ROMs from the software list
	device.option_add_internal("standard", VIC10_STD);
	device.option_add_internal("basic", VIC10_BASIC);
	device.option_add_internal("multimax", VIC10_MULTIMAX);
}
