/*********************************************************************



*********************************************************************/

#include "emu.h"
#include "floppy.h"
#include "formats/imageutl.h"

// device type definition
const device_type FLOPPY = &device_creator<floppy_image_device>;

//-------------------------------------------------
//  floppy_image_device - constructor
//-------------------------------------------------

floppy_image_device::floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, FLOPPY, "Floppy drive", tag, owner, clock),
	  device_image_interface(mconfig, *this),
	  m_image(NULL)
{

}

//-------------------------------------------------
//  floppy_image_device - destructor
//-------------------------------------------------

floppy_image_device::~floppy_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void floppy_image_device::device_config_complete()
{
	// inherit a copy of the static data
	const floppy_interface *intf = reinterpret_cast<const floppy_interface *>(static_config());
	if (intf != NULL)
		*static_cast<floppy_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{    	
		memset(&m_formats, 0, sizeof(m_formats));
		memset(&m_interface, 0, sizeof(m_interface));
		memset(&m_device_displayinfo, 0, sizeof(m_device_displayinfo));
		memset(&m_load_func, 0, sizeof(m_load_func));
		memset(&m_unload_func, 0, sizeof(m_unload_func));
	}

	image_device_format **formatptr;
    image_device_format *format;
    formatptr = &m_formatlist;
	int cnt = 0;
	m_extension_list[0] = '\0';
	while (m_formats[cnt].name)
	{
		// allocate a new format
		format = global_alloc_clear(image_device_format);		
		format->m_index 	  = cnt;
		format->m_name        = m_formats[cnt].name;
		format->m_description = m_formats[cnt].description;
		format->m_extensions  = m_formats[cnt].extensions;
		format->m_optspec     = (m_formats[cnt].param_guidelines) ? m_formats[cnt].param_guidelines : "";

		image_specify_extension( m_extension_list, 256, m_formats[cnt].extensions );
		// and append it to the list
		*formatptr = format;
		formatptr = &format->m_next;
		cnt++;
	}
	
	// set brief and instance name
	update_names();
}

static TIMER_CALLBACK(floppy_drive_index_callback)
{
	floppy_image_device *image = (floppy_image_device *) ptr;
	image->index_func();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void floppy_image_device::device_start()
{
	// resolve callbacks
	m_out_idx_func.resolve(m_out_idx_cb, *this);
	
	m_idx = 0;
	
	/* motor off */
	m_mon = 1;

	m_rpm = 300.0f;
	
	m_index_timer = machine().scheduler().timer_alloc(FUNC(floppy_drive_index_callback), (void *)this);
}

bool floppy_image_device::call_load()
{
	device_image_interface *image = this;
	int best;
	m_image = global_alloc(floppy_image((void *) image, &image_ioprocs, m_formats));
	const struct floppy_format_def *format = m_image->identify(&best);
	if (format) {
		m_image->load(best);
		if (m_load_func)
			return m_load_func(*this);
	} else {
		return IMAGE_INIT_FAIL;
	}
	return IMAGE_INIT_PASS;
}

void floppy_image_device::call_unload()
{
	if (m_image)
		global_free(m_image);
	if (m_unload_func)
		m_unload_func(*this);
}

/* motor on, active low */
void floppy_image_device::mon_w(int state)
{
	/* force off if there is no attached image */
	if (!exists())
		state = 1;

	/* off -> on */
	if (m_mon && state == 0)
	{
		m_idx = 0;
		index_func();
	}

	/* on -> off */
	else if (m_mon == 0 && state)
		m_index_timer->adjust(attotime::zero);

	m_mon = state;
}

/* index pulses at rpm/60 Hz, and stays high 1/20th of time */
void floppy_image_device::index_func()
{
	double ms = 1000.0 / (m_rpm / 60.0);

	if (m_idx)
	{
		m_idx = 0;
		m_index_timer->adjust(attotime::from_double(ms*19/20/1000.0));
	}
	else
	{
		m_idx = 1;
		m_index_timer->adjust(attotime::from_double(ms/20/1000.0));
	}

	m_out_idx_func(m_idx);

	//if (drive->index_pulse_callback)
//		drive->index_pulse_callback(drive->controller, img, drive->idx);
}

int floppy_image_device::ready_r()
{
	if (exists())
	{
		if (m_mon == 0)
		{
			return 1;
		}
	}
	return 0;
}

double floppy_image_device::get_pos()
{
	return m_index_timer->elapsed().as_double();
}