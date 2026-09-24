// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-20 Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"

#include "multibyte.h"




//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC20_EXPANSION_SLOT, vic20_expansion_slot_device, "vic20_expansion_slot", "VIC-20 expansion port")



//**************************************************************************
//  VIC20 EXPANSION WINDOW
//**************************************************************************

vic20_expansion_window::vic20_expansion_window(device_t &device, const char *name, offs_t start, offs_t end)
	: m_view(device, name)
	, m_video_view(device, std::string(name) + "_video")
	, m_start(start)
	, m_end(end)
	, m_video_start(0)
	, m_has_video(false)
	, m_video_installed(false)
{
}

vic20_expansion_window::vic20_expansion_window(device_t &device, const char *name, offs_t start, offs_t end, offs_t video_start)
	: m_view(device, name)
	, m_video_view(device, std::string(name) + "_video")
	, m_start(start)
	, m_end(end)
	, m_video_start(video_start)
	, m_has_video(true)
	, m_video_installed(false)
{
}

void vic20_expansion_window::install_views(address_space &program, address_space *video)
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

void vic20_expansion_window::select(int slot)
{
	m_view.select(slot);

	if (m_video_installed)
		m_video_view.select(slot);
}

void vic20_expansion_window::unmap()
{
	m_view.disable();

	if (m_video_installed)
		m_video_view.disable();
}



//**************************************************************************
//  DEVICE VIC20_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vic20_expansion_card_interface - constructor
//-------------------------------------------------

device_vic20_expansion_card_interface::device_vic20_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "vic20exp")
	, m_slot(nullptr)
{
}


device_vic20_expansion_card_interface::~device_vic20_expansion_card_interface() = default;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_expansion_slot_device - constructor
//-------------------------------------------------

vic20_expansion_slot_device::vic20_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIC20_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_vic20_expansion_card_interface>(mconfig, *this),
	device_cartrom_image_interface(mconfig, *this),
	m_program(*this, finder_base::DUMMY_TAG, -1),
	m_video(*this, finder_base::DUMMY_TAG, -1),
	m_write_irq(*this),
	m_write_nmi(*this),
	m_write_res(*this),
	m_card(nullptr),
	m_root(find_root(owner)),
	m_ram1(*this, "ram1", 0x0400, 0x07ff, 0x2400),
	m_ram2(*this, "ram2", 0x0800, 0x0bff, 0x2800),
	m_ram3(*this, "ram3", 0x0c00, 0x0fff, 0x2c00),
	m_blk1(*this, "blk1", 0x2000, 0x3fff),
	m_blk2(*this, "blk2", 0x4000, 0x5fff),
	m_blk3(*this, "blk3", 0x6000, 0x7fff),
	m_blk5(*this, "blk5", 0xa000, 0xbfff),
	m_io2(*this, "io2", 0x9800, 0x9bff),
	m_io3(*this, "io3", 0x9c00, 0x9fff)
{
}


//-------------------------------------------------
//  find_root - locate the slot that owns the
//  memory views
//-------------------------------------------------

vic20_expansion_slot_device *vic20_expansion_slot_device::find_root(device_t *owner)
{
	vic20_expansion_slot_device *const parent = owner ? dynamic_cast<vic20_expansion_slot_device *>(owner->owner()) : nullptr;

	return parent ? parent->m_root : this;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_expansion_slot_device::device_start()
{
	m_card = get_card_device();

	if (m_card)
		m_card->set_slot(*this);

	if ((m_root == this) && m_program)
	{
		for (vic20_expansion_window *window : { &m_ram1, &m_ram2, &m_ram3, &m_blk1, &m_blk2, &m_blk3, &m_blk5, &m_io2, &m_io3 })
			window->install_views(*m_program.target(), m_video.target());
	}
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

std::pair<std::error_condition, std::string> vic20_expansion_slot_device::call_load()
{
	if (!m_card || loaded_through_softlist())
		return {};

	struct layout
	{
		char const *extension;
		uint16_t address;
		char const *region;
		offs_t offset;
		size_t length;
	};

	static constexpr layout layouts[] =
	{
		{ "20", 0x2000, "blk1", 0x0000, 0x2000 },
		{ "40", 0x4000, "blk2", 0x0000, 0x2000 },
		{ "60", 0x6000, "blk3", 0x0000, 0x2000 },
		{ "70", 0x7000, "blk3", 0x1000, 0x1000 },
		{ "a0", 0xa000, "blk5", 0x0000, 0x2000 },
		{ "b0", 0xb000, "blk5", 0x1000, 0x1000 }
	};

	util::random_read &file = image_core_file();
	bool const crt = is_filetype("crt");
	uint16_t address = 0;

	if (crt)
	{
		uint8_t header[2];
		auto const [err, actual] = read(file, header, sizeof(header));

		if (err)
			return { err, std::string() };
		if (actual != sizeof(header))
			return { std::errc::io_error, std::string() };

		address = get_u16le(header);
	}

	for (layout const &entry : layouts)
	{
		if ((crt && (address == entry.address)) || (!crt && is_filetype(entry.extension)))
			return { load_region(file, entry.region, entry.offset, entry.length), std::string() };
	}

	return { image_error::INVALIDIMAGE, crt ? "Unsupported address in CRT file header" : std::string() };
}

//-------------------------------------------------
//  load_region - load a ROM block into a region
//  named like the software list data area
//-------------------------------------------------

std::error_condition vic20_expansion_slot_device::load_region(util::random_read &file, const char *tag, offs_t offset, size_t length)
{
	machine().memory().region_free(subtag(tag));
	memory_region *const region = machine().memory().region_alloc(subtag(tag), 0x2000, 1, ENDIANNESS_LITTLE);
	std::fill_n(region->base(), region->bytes(), 0);

	auto const [err, actual] = read(file, region->base() + offset, length);
	if (!err && (actual != length))
		return std::errc::io_error;

	return err;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string vic20_expansion_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("standard");
}


void vic20_expansion_slot_device::add_passthrough(machine_config &config, const char *tag)
{
	auto &slot = VIC20_EXPANSION_SLOT(config, tag, DERIVED_CLOCK(1, 1), vic20_expansion_cards, nullptr);
	slot.irq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(vic20_expansion_slot_device::irq_w));
	slot.nmi_wr_callback().set(DEVICE_SELF_OWNER, FUNC(vic20_expansion_slot_device::nmi_w));
	slot.res_wr_callback().set(DEVICE_SELF_OWNER, FUNC(vic20_expansion_slot_device::res_w));
}


//-------------------------------------------------
//  SLOT_INTERFACE( vic20_expansion_cards )
//-------------------------------------------------

// slot devices
#include "fe3.h"
#include "megacart.h"
#include "std.h"
#include "vfp.h"
#include "vic1010.h"
#include "vic1110.h"
#include "vic1111.h"
#include "vic1112.h"
#include "vic1210.h"
#include "videopak.h"
#include "speakeasy.h"

void vic20_expansion_cards(device_slot_interface &device)
{
	device.option_add("exp", VIC1010);
	device.option_add("3k", VIC1210);
	device.option_add("8k", VIC1110);
	device.option_add("16k", VIC1111);
	device.option_add("fe3", VIC20_FE3);
	device.option_add("vfp", VIC20_VFP);
	device.option_add("speakez", VIC20_SPEAKEASY);
	device.option_add("videopak", VIC20_VIDEO_PAK);

	// the following need ROMs from the software list
	device.option_add_internal("standard", VIC20_STD);
	device.option_add_internal("ieee488", VIC1112);
	device.option_add_internal("megacart", VIC20_MEGACART);
}
