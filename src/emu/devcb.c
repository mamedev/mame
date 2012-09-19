/***************************************************************************

    devcb.c

    Device callback interface helpers.

****************************************************************************

    Copyright Aaron Giles
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


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// special identifiers used to discover NULL functions
UINT8 devcb_resolved_read_line::s_null;
UINT8 devcb_resolved_write_line::s_null;
UINT8 devcb_resolved_read8::s_null;
UINT8 devcb_resolved_write8::s_null;
UINT8 devcb_resolved_read16::s_null;
UINT8 devcb_resolved_write16::s_null;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> devcb_resolver

class devcb_resolver
{
public:
	static ioport_port *resolve_port(const char *tag, device_t &current);
	static device_t *resolve_device(int index, const char *tag, device_t &current);
	static device_execute_interface *resolve_execute_interface(const char *tag, device_t &current);
	static address_space &resolve_space(int index, const char *tag, device_t &current);
};



//**************************************************************************
//  DEVCB RESOLVER
//**************************************************************************

//-------------------------------------------------
//  resolve_port - resolve to an input port object
//  based on the provided tag
//-------------------------------------------------

ioport_port *devcb_resolver::resolve_port(const char *tag, device_t &current)
{
	astring fullname;
	ioport_port *result = current.ioport(current.siblingtag(fullname, tag));
	if (result == NULL)
		throw emu_fatalerror("Unable to find input port '%s' (requested by %s '%s')", fullname.cstr(), current.name(), current.tag());
	return result;
}


//-------------------------------------------------
//  resolve_device - resolve to a device given
//  a tag and special index type
//-------------------------------------------------

device_t *devcb_resolver::resolve_device(int index, const char *tag, device_t &current)
{
	device_t *result = current.siblingdevice(tag);
	if (result == NULL)
		throw emu_fatalerror("Unable to resolve device '%s' (requested by callback to %s '%s')", tag, current.name(), current.tag());
	return result;
}


//-------------------------------------------------
//  resolve_execute_interface - resolve to an
//  execute interface on a device given a device
//  tag
//-------------------------------------------------

device_execute_interface *devcb_resolver::resolve_execute_interface(const char *tag, device_t &current)
{
	// find our target device
	device_t *targetdev = current.siblingdevice(tag);
	if (targetdev == NULL)
		throw emu_fatalerror("Unable to resolve device '%s' (requested by %s '%s')", tag, current.name(), current.tag());

	// make sure the target device has an execute interface
	device_execute_interface *exec;
	if (!targetdev->interface(exec))
		throw emu_fatalerror("Device '%s' (requested by %s '%s') has no execute interface", tag, current.name(), current.tag());

	return exec;
}


//-------------------------------------------------
//  resolve_space - resolve to an address space
//  given a device tag and a space index
//-------------------------------------------------

address_space &devcb_resolver::resolve_space(int index, const char *tag, device_t &current)
{
	// find our target device
	device_t *targetdev = current.siblingdevice(tag);
	if (targetdev == NULL)
		throw emu_fatalerror("Unable to resolve device '%s' (requested by %s '%s')", tag, current.name(), current.tag());

	// make sure the target device has a memory interface
	device_memory_interface *memory;
	if (!targetdev->interface(memory))
		throw emu_fatalerror("Device '%s' (requested by %s '%s') has no memory interface", tag, current.name(), current.tag());

	// set the real target and function, then prime a delegate
	address_space *result = memory->space(index);
	if (result == NULL)
		throw emu_fatalerror("Unable to find device '%s' space %d (requested by %s '%s')", tag, index, current.name(), current.tag());

	return *result;
}



//**************************************************************************
//  DEVCB RESOLVED READ LINE
//**************************************************************************

//-------------------------------------------------
//  devcb_resolved_read_line - empty constructor
//-------------------------------------------------

devcb_resolved_read_line::devcb_resolved_read_line()
{
	m_object.port = NULL;
	m_helper.read_line = NULL;
}


//-------------------------------------------------
//  resolve - resolve to a delegate from a static
//  structure definition
//-------------------------------------------------

void devcb_resolved_read_line::resolve(const devcb_read_line &desc, device_t &device)
{
	switch (desc.type)
	{
		default:
		case DEVCB_TYPE_NULL:
			m_object.constant = 0;
			m_helper.null_indicator = &s_null;
			*static_cast<devcb_read_line_delegate *>(this) = devcb_read_line_delegate(&devcb_resolved_read_line::from_constant, "(null)", this);
			break;

		case DEVCB_TYPE_IOPORT:
			m_object.port = devcb_resolver::resolve_port(desc.tag, device);
			*static_cast<devcb_read_line_delegate *>(this) = devcb_read_line_delegate(&devcb_resolved_read_line::from_port, desc.tag, this);
			break;

		case DEVCB_TYPE_DEVICE:
			m_object.device = devcb_resolver::resolve_device(desc.index, desc.tag, device);
			if (desc.readline != NULL)
				*static_cast<devcb_read_line_delegate *>(this) = devcb_read_line_delegate(desc.readline, desc.name, m_object.device);
			else
			{
				m_helper.read8_device = desc.readdevice;
				*static_cast<devcb_read_line_delegate *>(this) = devcb_read_line_delegate(&devcb_resolved_read_line::from_read8, desc.name, this);
			}
			break;

		case DEVCB_TYPE_LEGACY_SPACE:
			m_object.space = &devcb_resolver::resolve_space(desc.index, desc.tag, device);
			m_helper.read8_space = desc.readspace;
			*static_cast<devcb_read_line_delegate *>(this) = devcb_read_line_delegate(&devcb_resolved_read_line::from_read8, desc.name, this);
			break;

		case DEVCB_TYPE_CONSTANT:
			m_object.constant = desc.index;
			*static_cast<devcb_read_line_delegate *>(this) = devcb_read_line_delegate(&devcb_resolved_read_line::from_constant, "constant", this);
			break;
	}
}


//-------------------------------------------------
//  from_port - helper to convert from an I/O port
//  value to a line value
//-------------------------------------------------

int devcb_resolved_read_line::from_port()
{
	return (m_object.port->read() & 1) ? ASSERT_LINE : CLEAR_LINE;
}


//-------------------------------------------------
//  from_read8 - helper to convert from an 8-bit
//  memory read value to a line value
//-------------------------------------------------

int devcb_resolved_read_line::from_read8()
{
	return ((*m_helper.read8_device)(m_object.device, m_object.device->machine().driver_data()->generic_space(), 0, 0xff) & 1) ? ASSERT_LINE : CLEAR_LINE;
}


//-------------------------------------------------
//  from_constant - helper to convert from a
//  constant value to a line value
//-------------------------------------------------

int devcb_resolved_read_line::from_constant()
{
	return (m_object.constant & 1) ? ASSERT_LINE : CLEAR_LINE;
}



//**************************************************************************
//  DEVCB RESOLVED WRITE LINE
//**************************************************************************

//-------------------------------------------------
//  devcb_resolved_write_line - empty constructor
//-------------------------------------------------

devcb_resolved_write_line::devcb_resolved_write_line()
{
	m_object.port = NULL;
	m_helper.write_line = NULL;
}


//-------------------------------------------------
//  resolve - resolve to a delegate from a static
//  structure definition
//-------------------------------------------------

void devcb_resolved_write_line::resolve(const devcb_write_line &desc, device_t &device)
{
	switch (desc.type)
	{
		default:
		case DEVCB_TYPE_NULL:
			m_helper.null_indicator = &s_null;
			*static_cast<devcb_write_line_delegate *>(this) = devcb_write_line_delegate(&devcb_resolved_write_line::to_null, "(null)", this);
			break;

		case DEVCB_TYPE_IOPORT:
			m_object.port = devcb_resolver::resolve_port(desc.tag, device);
			*static_cast<devcb_write_line_delegate *>(this) = devcb_write_line_delegate(&devcb_resolved_write_line::to_port, desc.tag, this);
			break;

		case DEVCB_TYPE_DEVICE:
			m_object.device = devcb_resolver::resolve_device(desc.index, desc.tag, device);
			if (desc.writeline != NULL)
				*static_cast<devcb_write_line_delegate *>(this) = devcb_write_line_delegate(desc.writeline, desc.name, m_object.device);
			else
			{
				m_helper.write8_device = desc.writedevice;
				*static_cast<devcb_write_line_delegate *>(this) = devcb_write_line_delegate(&devcb_resolved_write_line::to_write8, desc.name, this);
			}
			break;

		case DEVCB_TYPE_LEGACY_SPACE:
			m_object.space = &devcb_resolver::resolve_space(desc.index, desc.tag, device);
			m_helper.write8_space = desc.writespace;
			*static_cast<devcb_write_line_delegate *>(this) = devcb_write_line_delegate(&devcb_resolved_write_line::to_write8, desc.name, this);
			break;

		case DEVCB_TYPE_INPUT_LINE:
			m_object.execute = devcb_resolver::resolve_execute_interface(desc.tag, device);
			m_helper.input_line = desc.index;
			*static_cast<devcb_write_line_delegate *>(this) = devcb_write_line_delegate(&devcb_resolved_write_line::to_input, desc.tag, this);
			break;
	}
}


//-------------------------------------------------
//  to_null - helper to handle a NULL write
//-------------------------------------------------

void devcb_resolved_write_line::to_null(int state)
{
}


//-------------------------------------------------
//  to_port - helper to convert to an I/O port
//  value from a line value
//-------------------------------------------------

void devcb_resolved_write_line::to_port(int state)
{
	m_object.port->write(state, 0xffffffff);
}


//-------------------------------------------------
//  to_write8 - helper to convert to an 8-bit
//  memory read value from a line value
//-------------------------------------------------

void devcb_resolved_write_line::to_write8(int state)
{
	(*m_helper.write8_device)(m_object.device, m_object.device->machine().driver_data()->generic_space(), 0, state, 0xff);
}


//-------------------------------------------------
//  to_input - helper to convert to a device input
//  value from a line value
//-------------------------------------------------

void devcb_resolved_write_line::to_input(int state)
{
	m_object.execute->set_input_line(m_helper.input_line, state);
}



//**************************************************************************
//  DEVCB RESOLVED READ8
//**************************************************************************

//-------------------------------------------------
//  devcb_resolved_read8 - empty constructor
//-------------------------------------------------

devcb_resolved_read8::devcb_resolved_read8()
{
	m_object.port = NULL;
	m_helper.read_line = NULL;
}


//-------------------------------------------------
//  resolve - resolve to a delegate from a static
//  structure definition
//-------------------------------------------------

void devcb_resolved_read8::resolve(const devcb_read8 &desc, device_t &device)
{
	switch (desc.type)
	{
		default:
		case DEVCB_TYPE_NULL:
			m_object.constant = 0;
			m_helper.null_indicator = &s_null;
			*static_cast<devcb_read8_delegate *>(this) = devcb_read8_delegate(&devcb_resolved_read8::from_constant, "(null)", this);
			break;

		case DEVCB_TYPE_IOPORT:
			m_object.port = devcb_resolver::resolve_port(desc.tag, device);
			*static_cast<devcb_read8_delegate *>(this) = devcb_read8_delegate(&devcb_resolved_read8::from_port, desc.tag, this);
			break;

		case DEVCB_TYPE_DEVICE:
			m_object.device = devcb_resolver::resolve_device(desc.index, desc.tag, device);
			if (desc.readdevice != NULL)
			{
			    m_helper.read8_device = desc.readdevice;
				*static_cast<devcb_read8_delegate *>(this) = devcb_read8_delegate(&devcb_resolved_read8::from_read8device, desc.name, this);
			}
			else
			{
				m_helper.read_line = desc.readline;
				*static_cast<devcb_read8_delegate *>(this) = devcb_read8_delegate(&devcb_resolved_read8::from_readline, desc.name, this);
			}
			break;

		case DEVCB_TYPE_LEGACY_SPACE:
			m_object.space = &devcb_resolver::resolve_space(desc.index, desc.tag, device);
		    m_helper.read8_space = desc.readspace;
			*static_cast<devcb_read8_delegate *>(this) = devcb_read8_delegate(&devcb_resolved_read8::from_read8space, desc.name, this);
			break;

		case DEVCB_TYPE_CONSTANT:
			m_object.constant = desc.index;
			*static_cast<devcb_read8_delegate *>(this) = devcb_read8_delegate(&devcb_resolved_read8::from_constant, "constant", this);
			break;
	}
}


//-------------------------------------------------
//  from_port - helper to convert from an I/O port
//  value to an 8-bit value
//-------------------------------------------------

UINT8 devcb_resolved_read8::from_port(offs_t offset, UINT8 mem_mask)
{
	return m_object.port->read();
}


//-------------------------------------------------
//  from_read8space - helper to convert from a device
//  line read value to an 8-bit value
//-------------------------------------------------

UINT8 devcb_resolved_read8::from_read8space(offs_t offset, UINT8 mem_mask)
{
	return (*m_helper.read8_space)(*m_object.space, offset, 0xff);
}


//-------------------------------------------------
//  from_read8device - helper to convert from a device
//  line read value to an 8-bit value
//-------------------------------------------------

UINT8 devcb_resolved_read8::from_read8device(offs_t offset, UINT8 mem_mask)
{
	return (*m_helper.read8_device)(m_object.device, m_object.device->machine().driver_data()->generic_space(), offset, mem_mask);
}


//-------------------------------------------------
//  from_readline - helper to convert from a device
//  line read value to an 8-bit value
//-------------------------------------------------

UINT8 devcb_resolved_read8::from_readline(offs_t offset, UINT8 mem_mask)
{
	return (*m_helper.read_line)(m_object.device);
}


//-------------------------------------------------
//  from_constant - helper to convert from a
//  constant value to an 8-bit value
//-------------------------------------------------

UINT8 devcb_resolved_read8::from_constant(offs_t offset, UINT8 mem_mask)
{
	return m_object.constant;
}



//**************************************************************************
//  DEVCB RESOLVED WRITE8
//**************************************************************************

//-------------------------------------------------
//  devcb_resolved_write8 - empty constructor
//-------------------------------------------------

devcb_resolved_write8::devcb_resolved_write8()
{
	m_object.port = NULL;
	m_helper.write_line = NULL;
}


//-------------------------------------------------
//  resolve - resolve to a delegate from a static
//  structure definition
//-------------------------------------------------

void devcb_resolved_write8::resolve(const devcb_write8 &desc, device_t &device)
{
	switch (desc.type)
	{
		default:
		case DEVCB_TYPE_NULL:
			m_helper.null_indicator = &s_null;
			*static_cast<devcb_write8_delegate *>(this) = devcb_write8_delegate(&devcb_resolved_write8::to_null, "(null)", this);
			break;

		case DEVCB_TYPE_IOPORT:
			m_object.port = devcb_resolver::resolve_port(desc.tag, device);
			*static_cast<devcb_write8_delegate *>(this) = devcb_write8_delegate(&devcb_resolved_write8::to_port, desc.tag, this);
			break;

		case DEVCB_TYPE_DEVICE:
			m_object.device = devcb_resolver::resolve_device(desc.index, desc.tag, device);
			if (desc.writedevice != NULL)
			{
				m_helper.write8_device = desc.writedevice;
				*static_cast<devcb_write8_delegate *>(this) = devcb_write8_delegate(&devcb_resolved_write8::to_write8device, desc.name, this);
			}
			else
			{
				m_helper.write_line = desc.writeline;
				*static_cast<devcb_write8_delegate *>(this) = devcb_write8_delegate(&devcb_resolved_write8::to_writeline, desc.name, this);
			}
			break;

		case DEVCB_TYPE_LEGACY_SPACE:
			m_object.space = &devcb_resolver::resolve_space(desc.index, desc.tag, device);
			m_helper.write8_space = desc.writespace;
			*static_cast<devcb_write8_delegate *>(this) = devcb_write8_delegate(&devcb_resolved_write8::to_write8space, desc.name, this);
			break;

		case DEVCB_TYPE_INPUT_LINE:
			m_object.execute = devcb_resolver::resolve_execute_interface(desc.tag, device);
			m_helper.input_line = desc.index;
			*static_cast<devcb_write8_delegate *>(this) = devcb_write8_delegate(&devcb_resolved_write8::to_input, desc.tag, this);
			break;
	}
}


//-------------------------------------------------
//  to_null - helper to handle a NULL write
//-------------------------------------------------

void devcb_resolved_write8::to_null(offs_t offset, UINT8 data, UINT8 mem_mask)
{
}


//-------------------------------------------------
//  to_port - helper to convert to an I/O port
//  value from a line value
//-------------------------------------------------

void devcb_resolved_write8::to_port(offs_t offset, UINT8 data, UINT8 mem_mask)
{
	m_object.port->write(data, mem_mask);
}


//-------------------------------------------------
//  to_write8space - helper to convert to an 8-bit
//  memory read value from a line value
//-------------------------------------------------

void devcb_resolved_write8::to_write8space(offs_t offset, UINT8 data, UINT8 mem_mask)
{
	(*m_helper.write8_space)(*m_object.space, offset, data, mem_mask);
}


//-------------------------------------------------
//  to_write8device - helper to convert to an 8-bit
//  memory read value from a line value
//-------------------------------------------------

void devcb_resolved_write8::to_write8device(offs_t offset, UINT8 data, UINT8 mem_mask)
{
	(*m_helper.write8_device)(m_object.device, m_object.device->machine().driver_data()->generic_space(), offset, data, mem_mask);
}


//-------------------------------------------------
//  to_write8 - helper to convert to an 8-bit
//  memory read value from a line value
//-------------------------------------------------

void devcb_resolved_write8::to_writeline(offs_t offset, UINT8 data, UINT8 mem_mask)
{
	(*m_helper.write_line)(m_object.device, (data & 1) ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  to_input - helper to convert to a device input
//  value from a line value
//-------------------------------------------------

void devcb_resolved_write8::to_input(offs_t offset, UINT8 data, UINT8 mem_mask)
{
	m_object.execute->set_input_line(m_helper.input_line, (data & 1) ? ASSERT_LINE : CLEAR_LINE);
}



//**************************************************************************
//  DEVCB RESOLVED READ16
//**************************************************************************

//-------------------------------------------------
//  devcb_resolved_read16 - empty constructor
//-------------------------------------------------

devcb_resolved_read16::devcb_resolved_read16()
{
	m_object.port = NULL;
	m_helper.read_line = NULL;
}


//-------------------------------------------------
//  resolve - resolve to a delegate from a static
//  structure definition
//-------------------------------------------------

void devcb_resolved_read16::resolve(const devcb_read16 &desc, device_t &device)
{
	switch (desc.type)
	{
		default:
		case DEVCB_TYPE_NULL:
			m_object.constant = 0;
			m_helper.null_indicator = &s_null;
			*static_cast<devcb_read16_delegate *>(this) = devcb_read16_delegate(&devcb_resolved_read16::from_constant, "(null)", this);
			break;

		case DEVCB_TYPE_IOPORT:
			m_object.port = devcb_resolver::resolve_port(desc.tag, device);
			*static_cast<devcb_read16_delegate *>(this) = devcb_read16_delegate(&devcb_resolved_read16::from_port, desc.tag, this);
			break;

		case DEVCB_TYPE_DEVICE:
			m_object.device = devcb_resolver::resolve_device(desc.index, desc.tag, device);
			if (desc.readdevice != NULL)
			{
				m_helper.read16_device = desc.readdevice;
				*static_cast<devcb_read16_delegate *>(this) = devcb_read16_delegate(&devcb_resolved_read16::from_read16, desc.name, this);
			}
			else
			{
				m_helper.read_line = desc.readline;
				*static_cast<devcb_read16_delegate *>(this) = devcb_read16_delegate(&devcb_resolved_read16::from_readline, desc.name, this);
			}
			break;

		case DEVCB_TYPE_LEGACY_SPACE:
			m_object.space = &devcb_resolver::resolve_space(desc.index, desc.tag, device);
			*static_cast<devcb_read16_delegate *>(this) = devcb_read16_delegate(desc.readspace, desc.name, m_object.space);
			break;

		case DEVCB_TYPE_CONSTANT:
			m_object.constant = desc.index;
			*static_cast<devcb_read16_delegate *>(this) = devcb_read16_delegate(&devcb_resolved_read16::from_constant, "constant", this);
			break;
	}
}


//-------------------------------------------------
//  from_port - helper to convert from an I/O port
//  value to a 16-bit value
//-------------------------------------------------

UINT16 devcb_resolved_read16::from_port(offs_t offset, UINT16 mask)
{
	return m_object.port->read();
}


//-------------------------------------------------
//  from_read16 - helper to convert from a device
//  line read value to a 16-bit value
//-------------------------------------------------

UINT16 devcb_resolved_read16::from_read16(offs_t offset, UINT16 mask)
{
	return (*m_helper.read16_device)(m_object.device, m_object.device->machine().driver_data()->generic_space(), offset, mask);
}


//-------------------------------------------------
//  from_read16 - helper to convert from a device
//  line read value to a 16-bit value
//-------------------------------------------------

UINT16 devcb_resolved_read16::from_readline(offs_t offset, UINT16 mask)
{
	return (*m_helper.read_line)(m_object.device);
}


//-------------------------------------------------
//  from_constant - helper to convert from a
//  constant value to a 16-bit value
//-------------------------------------------------

UINT16 devcb_resolved_read16::from_constant(offs_t offset, UINT16 mask)
{
	return m_object.constant;
}



//**************************************************************************
//  DEVCB RESOLVED WRITE16
//**************************************************************************

//-------------------------------------------------
//  devcb_resolved_write16 - empty constructor
//-------------------------------------------------

devcb_resolved_write16::devcb_resolved_write16()
{
	m_object.port = NULL;
	m_helper.write_line = NULL;
}


//-------------------------------------------------
//  resolve - resolve to a delegate from a static
//  structure definition
//-------------------------------------------------

void devcb_resolved_write16::resolve(const devcb_write16 &desc, device_t &device)
{
	switch (desc.type)
	{
		default:
		case DEVCB_TYPE_NULL:
			m_helper.null_indicator = &s_null;
			*static_cast<devcb_write16_delegate *>(this) = devcb_write16_delegate(&devcb_resolved_write16::to_null, "(null)", this);
			break;

		case DEVCB_TYPE_IOPORT:
			m_object.port = devcb_resolver::resolve_port(desc.tag, device);
			*static_cast<devcb_write16_delegate *>(this) = devcb_write16_delegate(&devcb_resolved_write16::to_port, desc.tag, this);
			break;

		case DEVCB_TYPE_DEVICE:
			m_object.device = devcb_resolver::resolve_device(desc.index, desc.tag, device);
			if (desc.writedevice != NULL)
			{
				m_helper.write16_device = desc.writedevice;
				*static_cast<devcb_write16_delegate *>(this) = devcb_write16_delegate(&devcb_resolved_write16::to_write16, desc.name, this);
			}
			else
			{
				m_helper.write_line = desc.writeline;
				*static_cast<devcb_write16_delegate *>(this) = devcb_write16_delegate(&devcb_resolved_write16::to_writeline, desc.name, this);
			}
			break;

		case DEVCB_TYPE_LEGACY_SPACE:
			m_object.space = &devcb_resolver::resolve_space(desc.index, desc.tag, device);
			*static_cast<devcb_write16_delegate *>(this) = devcb_write16_delegate(desc.writespace, desc.name, m_object.space);
			break;

		case DEVCB_TYPE_INPUT_LINE:
			m_object.execute = devcb_resolver::resolve_execute_interface(desc.tag, device);
			m_helper.input_line = desc.index;
			*static_cast<devcb_write16_delegate *>(this) = devcb_write16_delegate(&devcb_resolved_write16::to_input, desc.tag, this);
			break;
	}
}


//-------------------------------------------------
//  to_null - helper to handle a NULL write
//-------------------------------------------------

void devcb_resolved_write16::to_null(offs_t offset, UINT16 data, UINT16 mask)
{
}


//-------------------------------------------------
//  to_port - helper to convert to an I/O port
//  value from a line value
//-------------------------------------------------

void devcb_resolved_write16::to_port(offs_t offset, UINT16 data, UINT16 mask)
{
	m_object.port->write(data, mask);
}


//-------------------------------------------------
//  to_write16 - helper to convert to a 16-bit
//  memory read value from a line value
//-------------------------------------------------

void devcb_resolved_write16::to_write16(offs_t offset, UINT16 data, UINT16 mask)
{
	(*m_helper.write16_device)(m_object.device, m_object.device->machine().driver_data()->generic_space(), offset, data, mask);
}


//-------------------------------------------------
//  to_write16 - helper to convert to a 16-bit
//  memory read value from a line value
//-------------------------------------------------

void devcb_resolved_write16::to_writeline(offs_t offset, UINT16 data, UINT16 mask)
{
	(*m_helper.write_line)(m_object.device, (data & 1) ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  to_input - helper to convert to a device input
//  value from a line value
//-------------------------------------------------

void devcb_resolved_write16::to_input(offs_t offset, UINT16 data, UINT16 mask)
{
	m_object.execute->set_input_line(m_helper.input_line, (data & 1) ? ASSERT_LINE : CLEAR_LINE);
}
