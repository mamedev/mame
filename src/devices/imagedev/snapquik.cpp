// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    snapquik.cpp

    Snapshots and quickloads

*********************************************************************/

#include "emu.h"
#include "snapquik.h"

// device type definition
DEFINE_DEVICE_TYPE(SNAPSHOT, snapshot_image_device, "snapsot_image", "Snapshot")

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
	/* invoke the load */
	m_load(*this);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snapshot_image_device::device_start()
{
	m_load.resolve();

	/* allocate a timer */
	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(snapshot_image_device::process_snapshot_or_quickload),this));
}

/*-------------------------------------------------
    call_load
-------------------------------------------------*/
image_init_result snapshot_image_device::call_load()
{
	/* adjust the timer */
	m_timer->adjust(m_delay, 0);
	return image_init_result::PASS;
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
