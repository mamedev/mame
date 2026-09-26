// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PLUS4_EXPANSION_SLOT, plus4_expansion_slot_device, "plus4_expansion_slot", "Plus/4 Expansion Port")



//**************************************************************************
//  PLUS4 EXPANSION WINDOW
//**************************************************************************

plus4_expansion_window::plus4_expansion_window(device_t &device, const char *name, offs_t start, offs_t end)
	: m_view(device, name)
	, m_video_view(device, std::string(name) + "_video")
	, m_start(start)
	, m_end(end)
	, m_video_start(0)
	, m_hole_start(0)
	, m_hole_end(0)
	, m_has_video(false)
	, m_has_hole(false)
	, m_video_installed(false)
{
}

plus4_expansion_window::plus4_expansion_window(device_t &device, const char *name, offs_t start, offs_t end, offs_t video_start)
	: m_view(device, name)
	, m_video_view(device, std::string(name) + "_video")
	, m_start(start)
	, m_end(end)
	, m_video_start(video_start)
	, m_hole_start(0)
	, m_hole_end(0)
	, m_has_video(true)
	, m_has_hole(false)
	, m_video_installed(false)
{
}

plus4_expansion_window::plus4_expansion_window(device_t &device, const char *name, offs_t start, offs_t end, offs_t video_start, offs_t hole_start, offs_t hole_end)
	: m_view(device, name)
	, m_video_view(device, std::string(name) + "_video")
	, m_start(start)
	, m_end(end)
	, m_video_start(video_start)
	, m_hole_start(hole_start)
	, m_hole_end(hole_end)
	, m_has_video(true)
	, m_has_hole(true)
	, m_video_installed(false)
{
}

void plus4_expansion_window::install_views(address_space_installer &program, address_space_installer *video)
{
	program.install_view(m_start, m_end, m_view);
	m_view[0];

	if (m_has_video && video)
	{
		video->install_view(m_video_start, m_video_start + (m_end - m_start), m_video_view);
		m_video_view[0];
		m_video_installed = true;
	}
}

void plus4_expansion_window::select(int slot)
{
	m_view.select(slot);

	if (m_video_installed)
		m_video_view.select(slot);
}

void plus4_expansion_window::unmap()
{
	m_view.disable();

	if (m_video_installed)
		m_video_view.disable();
}

void plus4_expansion_window::variant::install_rom(offs_t start, offs_t end, offs_t mirror, void *baseptr)
{
	uint8_t *const base = reinterpret_cast<uint8_t *>(baseptr);

	for (offs_t m = 0; ; m = ((m | ~mirror) + 1) & mirror)
	{
		if (m_window.m_has_hole)
		{
			if (start < m_window.m_hole_start)
				install_rom_segment(start | m, std::min(end, m_window.m_hole_start - 1) | m, base);

			if (end > m_window.m_hole_end)
			{
				offs_t const first = std::max(start, m_window.m_hole_end + 1);
				install_rom_segment(first | m, end | m, base + (first - start));
			}
		}
		else
		{
			install_rom_segment(start | m, end | m, base);
		}

		if (m == mirror)
			break;
	}
}

void plus4_expansion_window::variant::install_rom_segment(offs_t start, offs_t end, uint8_t *base)
{
	m_window.m_view[m_slot].install_rom(m_window.m_start + start, m_window.m_start + end, base);

	if (m_window.m_video_installed)
		m_window.m_video_view[m_slot].install_rom(m_window.m_video_start + start, m_window.m_video_start + end, base);
}



//**************************************************************************
//  DEVICE PLUS4_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_plus4_expansion_card_interface - constructor
//-------------------------------------------------

device_plus4_expansion_card_interface::device_plus4_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "plus4exp")
	, m_slot(nullptr)
{
}


device_plus4_expansion_card_interface::~device_plus4_expansion_card_interface() = default;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  plus4_expansion_slot_device - constructor
//-------------------------------------------------

plus4_expansion_slot_device::plus4_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PLUS4_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_plus4_expansion_card_interface>(mconfig, *this),
	device_cartrom_image_interface(mconfig, *this),
	m_write_irq(*this),
	m_write_aec(*this),
	m_card(nullptr),
	m_root(find_root(owner)),
	m_c1l(*this, "c1l", 0x8000, 0xbfff, 0x18000),
	m_c1h(*this, "c1h", 0xc000, 0xffff, 0x1c000, 0x3c00, 0x3f1f),
	m_c2l(*this, "c2l", 0x8000, 0xbfff, 0x18000),
	m_c2h(*this, "c2h", 0xc000, 0xffff, 0x1c000, 0x3c00, 0x3f1f),
	m_io(*this, "io", 0xfd00, 0xfeff)
{
}


//-------------------------------------------------
//  add_passthrough - add a pass-through slot
//  on a card
//-------------------------------------------------

void plus4_expansion_slot_device::add_passthrough(machine_config &config, const char *tag)
{
	auto &slot = PLUS4_EXPANSION_SLOT(config, tag, DERIVED_CLOCK(1, 1), plus4_expansion_cards, nullptr);
	slot.irq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(plus4_expansion_slot_device::irq_w));
	slot.aec_wr_callback().set(DEVICE_SELF_OWNER, FUNC(plus4_expansion_slot_device::aec_w));
}


//-------------------------------------------------
//  find_root - locate the slot that owns the
//  memory views
//-------------------------------------------------

plus4_expansion_slot_device *plus4_expansion_slot_device::find_root(device_t *owner)
{
	plus4_expansion_slot_device *const parent = owner ? dynamic_cast<plus4_expansion_slot_device *>(owner->owner()) : nullptr;

	return parent ? parent->m_root : this;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_expansion_slot_device::device_start()
{
	m_card = get_card_device();

	if (m_card)
		m_card->set_slot(*this);
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

std::pair<std::error_condition, std::string> plus4_expansion_slot_device::call_load()
{
	if (m_card)
	{
		if (!loaded_through_softlist())
			return std::make_pair(image_error::UNSUPPORTED, "Plus/4 Expansion software must be loaded from the software list");

		for (const char *tag : { "c1l", "c1h", "c2l", "c2h" })
		{
			uint32_t const size = get_software_region_length(tag);

			if (size & (size - 1))
				return std::make_pair(image_error::INVALIDLENGTH, "All ROM sizes must be powers of 2");
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string plus4_expansion_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  SLOT_INTERFACE( plus4_expansion_cards )
//-------------------------------------------------

// slot devices
#include "c1551.h"
#include "sid.h"
#include "std.h"

void plus4_expansion_cards(device_slot_interface &device)
{
	device.option_add("c1551", C1551);
	device.option_add("sid", PLUS4_SID);

	// the following need ROMs from the software list
	device.option_add_internal("standard", PLUS4_STD);
}
