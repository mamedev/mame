// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    snapquik.cpp

    Snapshots and quickloads

*********************************************************************/

#include "emu.h"
#include "snapquik.h"

#include "softlist_dev.h"
#include "ui/uimain.h"

// device type definition
DEFINE_DEVICE_TYPE(SNAPSHOT, snapshot_image_device, "snapshot_image", "Snapshot")

//-------------------------------------------------
//  snapshot_image_device - constructor
//-------------------------------------------------

snapshot_image_device::snapshot_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: snapshot_image_device(mconfig, SNAPSHOT, tag, owner, clock)
{
}

snapshot_image_device::snapshot_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_load(*this)
	, m_file_extensions(nullptr)
	, m_interface(nullptr)
	, m_delay(attotime::zero)
	, m_timer(nullptr)
{
}

//-------------------------------------------------
//  snapshot_image_device - destructor
//-------------------------------------------------

snapshot_image_device::~snapshot_image_device()
{
}

/*-------------------------------------------------
    TIMER_CALLBACK_MEMBER(process_snapshot_or_quickload)
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER(snapshot_image_device::process_snapshot_or_quickload)
{
	check_for_file();

	// invoke the load (FIXME: better way to report errors?)
	auto [err, message] = m_load(*this);
	if (err)
	{
		osd_printf_error(
				!message.empty()
					? "%1$s: Error loading '%2$s': %3$s (%4$s:%5$d %6$s)\n"
					: "%1$s: Error loading '%2$s': %6$s (%4$s:%5$d)\n",
				tag(),
				basename(),
				message,
				err.category().name(),
				err.value(),
				err.message());
		popmessage(
				!message.empty() ? "Error loading '%1$s': %2$s" : "Error loading '%1$s': %3$s",
				basename(),
				message,
				err.message());
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snapshot_image_device::device_start()
{
	m_load.resolve();

	// allocate a timer
	m_timer = timer_alloc(FUNC(snapshot_image_device::process_snapshot_or_quickload), this);
}

/*-------------------------------------------------
    call_load
-------------------------------------------------*/
std::pair<std::error_condition, std::string> snapshot_image_device::call_load()
{
	// adjust the timer
	m_timer->adjust(m_delay, 0);
	return std::make_pair(std::error_condition(), std::string());
}

const software_list_loader &snapshot_image_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}


//-------------------------------------------------
//  show_message - used to display a message while
//  loading
//-------------------------------------------------

void snapshot_image_device::show_message(util::format_argument_pack<char> const &args)
{
	// display the popup for a standard amount of time
	machine().ui().popup_time(5, "%s: %s", basename(), util::string_format(args));
}


// device type definition
DEFINE_DEVICE_TYPE(QUICKLOAD, quickload_image_device, "quickload", "Quickload")

//-------------------------------------------------
//  quickload_image_device - constructor
//-------------------------------------------------

quickload_image_device::quickload_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: snapshot_image_device(mconfig, QUICKLOAD, tag, owner, clock)
{
}
