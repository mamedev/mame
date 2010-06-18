/***************************************************************************

    diimage.c

    Device image interfaces.

****************************************************************************

    Copyright Miodrag Milanovic
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "ui.h"


//**************************************************************************
//  DEVICE CONFIG IMAGE INTERFACE
//**************************************************************************
const image_device_type_info device_config_image_interface::m_device_info_array[] =
	{
		{ IO_CARTSLOT,	"cartridge",	"cart" }, /*  0 */
		{ IO_FLOPPY,	"floppydisk",	"flop" }, /*  1 */
		{ IO_HARDDISK,	"harddisk",		"hard" }, /*  2 */
		{ IO_CYLINDER,	"cylinder",		"cyln" }, /*  3 */
		{ IO_CASSETTE,	"cassette",		"cass" }, /*  4 */
		{ IO_PUNCHCARD,	"punchcard",	"pcrd" }, /*  5 */
		{ IO_PUNCHTAPE,	"punchtape",	"ptap" }, /*  6 */
		{ IO_PRINTER,	"printer",		"prin" }, /*  7 */
		{ IO_SERIAL,	"serial",		"serl" }, /*  8 */
		{ IO_PARALLEL,	"parallel",		"parl" }, /*  9 */
		{ IO_SNAPSHOT,	"snapshot",		"dump" }, /* 10 */
		{ IO_QUICKLOAD,	"quickload",	"quik" }, /* 11 */
		{ IO_MEMCARD,	"memcard",		"memc" }, /* 12 */
		{ IO_CDROM,     "cdrom",        "cdrm" }, /* 13 */
		{ IO_MAGTAPE,	"magtape",		"magt" }, /* 14 */
	};

//-------------------------------------------------
//  device_config_image_interface - constructor
//-------------------------------------------------

device_config_image_interface::device_config_image_interface(const machine_config &mconfig, device_config &devconfig)
	: device_config_interface(mconfig, devconfig)
{
}


//-------------------------------------------------
//  ~device_config_image_interface - destructor
//-------------------------------------------------

device_config_image_interface::~device_config_image_interface()
{
}


//-------------------------------------------------
//  find_device_type - search trough list of
//  device types to extact data
//-------------------------------------------------

const image_device_type_info *device_config_image_interface::find_device_type(iodevice_t type)
{
	int i;
	for (i = 0; i < ARRAY_LENGTH(device_config_image_interface::m_device_info_array); i++)
	{
		if (m_device_info_array[i].m_type == type)
			return &m_device_info_array[i];
	}
	return NULL;
}

//-------------------------------------------------
//  device_typename - retrieves device type name
//-------------------------------------------------

const char *device_config_image_interface::device_typename(iodevice_t type)
{
	const image_device_type_info *info = find_device_type(type);
	return (info != NULL) ? info->m_name : NULL;
}

//-------------------------------------------------
//  device_brieftypename - retrieves device
//  brief type name
//-------------------------------------------------

const char *device_config_image_interface::device_brieftypename(iodevice_t type)
{
	const image_device_type_info *info = find_device_type(type);
	return (info != NULL) ? info->m_shortname : NULL;
}

//**************************************************************************
//  DEVICE image INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_image_interface - constructor
//-------------------------------------------------

device_image_interface::device_image_interface(running_machine &machine, const device_config &config, device_t &device)
	: device_interface(machine, config, device),
	  m_image_config(dynamic_cast<const device_config_image_interface &>(config)),	  
	  m_file(NULL)
{
}


//-------------------------------------------------
//  ~device_image_interface - destructor
//-------------------------------------------------

device_image_interface::~device_image_interface()
{
}

/****************************************************************************
    CREATION FORMATS
****************************************************************************/

/*-------------------------------------------------
    device_get_indexed_creatable_format -
    accesses a specific image format available for
    image creation by index
-------------------------------------------------*/

const image_device_format *device_image_interface::device_get_indexed_creatable_format(int index)
{
    const image_device_format *format = device_get_creatable_formats();
    while(index-- && (format != NULL))
        format = format->m_next;
    return format;
}



/*-------------------------------------------------
    device_get_named_creatable_format -
    accesses a specific image format available for
    image creation by name
-------------------------------------------------*/

const image_device_format *device_image_interface::device_get_named_creatable_format(const char *format_name)
{
    const image_device_format *format = device_get_creatable_formats();
    while((format != NULL) && strcmp(format->m_name, format_name))
        format = format->m_next;
    return format;
}

/****************************************************************************
    ERROR HANDLING
****************************************************************************/

/*-------------------------------------------------
    image_clear_error - clear out any specified
    error
-------------------------------------------------*/

void device_image_interface::clear_error()
{
    m_err = IMAGE_ERROR_SUCCESS;
    if (m_err_message.len()==0)
    {
        //image_freeptr(image->dev, image->err_message);
        m_err_message.reset();
    }
}



/*-------------------------------------------------
    error - returns the error text for an image
    error
-------------------------------------------------*/

const char *device_image_interface::error()
{
    static const char *const messages[] =
    {
        NULL,
        "Internal error",
        "Unsupported operation",
        "Out of memory",
        "File not found",
        "Invalid image",
        "File already open",
        "Unspecified error"
    };

    return m_err_message.len()==0 ? m_err_message : astring(messages[m_err]);
}



/*-------------------------------------------------
    seterror - specifies an error on an image
-------------------------------------------------*/

void device_image_interface::seterror(image_error_t err, const char *message)
{
    clear_error();
    m_err = err;
    if (message != NULL)
    {
        m_err_message = message;
    }
}



/*-------------------------------------------------
    message - used to display a message while
    loading
-------------------------------------------------*/

void device_image_interface::message(const char *format, ...)
{
    va_list args;
    char buffer[256];	

    /* format the message */
    va_start(args, format);
    vsnprintf(buffer, ARRAY_LENGTH(buffer), format, args);
    va_end(args);

    /* display the popup for a standard amount of time */
    ui_popup_time(5, "%s: %s",
        basename(),
        buffer);
}


/****************************************************************************
  Accessor functions

  These provide information about the device; and about the mounted image
****************************************************************************/

/*-------------------------------------------------
    filename
-------------------------------------------------*/

const char *device_image_interface::filename()
{
    const char *name = m_name;
    return (name[0] != '\0') ? name : NULL;
}

/*-------------------------------------------------
    basename
-------------------------------------------------*/

const char *device_image_interface::basename()
{
    char *fname = (char*)filename();
	const char *c;

	// NULL begets NULL
	if (!fname)
		return NULL;

	// start at the end and return when we hit a slash or colon
	for (c = fname + strlen(fname) - 1; c >= fname; c--)
		if (*c == '\\' || *c == '/' || *c == ':')
			return c + 1;

	// otherwise, return the whole thing
	return fname;
}


/*-------------------------------------------------
    basename_noext
-------------------------------------------------*/

const char *device_image_interface::basename_noext()
{
    const char *s;
    char *ext;

    s = basename();
	if (s)
	{
		ext = strrchr(s, '.');
		if (ext)
			*ext = '\0';
	}
    return s;
}



/*-------------------------------------------------
    filetype
-------------------------------------------------*/

const char *device_image_interface::filetype()
{
    const char *s = filename();
    if (s != NULL)
        s = strrchr(s, '.');
    return s ? s+1 : NULL;
}
