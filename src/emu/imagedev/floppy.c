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

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void floppy_image_device::device_start()
{
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
