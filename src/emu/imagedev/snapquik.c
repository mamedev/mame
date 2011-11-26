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
    : device_t(mconfig, SNAPSHOT, "Snapshot", tag, owner, clock),
	  device_image_interface(mconfig, *this)
{
}

snapshot_image_device::snapshot_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, type, name, tag, owner, clock),
		device_image_interface(mconfig, *this)
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
    TIMER_CALLBACK(process_snapshot_or_quickload)
-------------------------------------------------*/

static TIMER_CALLBACK(process_snapshot_or_quickload)
{
	reinterpret_cast<snapshot_image_device *>(ptr)->timer_callback();
}

void snapshot_image_device::timer_callback()
{
	/* invoke the load */
	(*m_load)(*this, filetype(), length());

	/* unload the device */
	unload();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snapshot_image_device::device_start()
{
	/* allocate a timer */
	m_timer = machine().scheduler().timer_alloc(FUNC(process_snapshot_or_quickload), (void *)this);
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
    : snapshot_image_device(mconfig, QUICKLOAD, "Quickload", tag, owner, clock)
{
}

