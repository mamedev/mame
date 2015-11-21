// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    snapquik.h

    Snapshots and quickloads

*********************************************************************/

#include "emu.h"
#include "snapquik.h"

// device type definition
const device_type SNAPSHOT = &device_creator<snapshot_image_device>;

//-------------------------------------------------
//  snapshot_image_device - constructor
//-------------------------------------------------

snapshot_image_device::snapshot_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SNAPSHOT, "Snapshot", tag, owner, clock, "snapshot_image", __FILE__),
	  device_image_interface(mconfig, *this),
	  m_file_extensions(NULL),
	  m_interface(NULL),
	  m_delay_seconds(0),
	  m_delay_attoseconds(0),
	  m_timer(NULL)
{
}

snapshot_image_device::snapshot_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_image_interface(mconfig, *this),
	m_file_extensions(NULL),
	m_interface(NULL),
	m_delay_seconds(0),
	m_delay_attoseconds(0),
	m_timer(NULL)
{
}
//-------------------------------------------------
//  snapshot_image_device - destructor
//-------------------------------------------------

snapshot_image_device::~snapshot_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void snapshot_image_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}

/*-------------------------------------------------
    TIMER_CALLBACK_MEMBER(process_snapshot_or_quickload)
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER(snapshot_image_device::process_snapshot_or_quickload)
{
	/* invoke the load */
	m_load(*this, filetype(), length());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snapshot_image_device::device_start()
{
	/* allocate a timer */
	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(snapshot_image_device::process_snapshot_or_quickload),this));
}

/*-------------------------------------------------
    call_load
-------------------------------------------------*/
bool snapshot_image_device::call_load()
{
	/* adjust the timer */
	m_timer->adjust(attotime(m_delay_seconds, m_delay_attoseconds),0);
	return IMAGE_INIT_PASS;
}

// device type definition
const device_type QUICKLOAD = &device_creator<quickload_image_device>;

//-------------------------------------------------
//  quickload_image_device - constructor
//-------------------------------------------------

quickload_image_device::quickload_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: snapshot_image_device(mconfig, QUICKLOAD, "Quickload", tag, owner, clock, "quickload", __FILE__)
{
}
