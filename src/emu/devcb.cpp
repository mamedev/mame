// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devcb.c

    Device callback interface helpers.

***************************************************************************/

#include "emu.h"



//**************************************************************************
//  DEVCB BASE CLASS
//**************************************************************************

//-------------------------------------------------
//  devcb_base - constructor
//-------------------------------------------------

devcb_base::devcb_base(device_t &device, UINT64 defmask)
	: m_device(device),
		m_rshift(0),
		m_mask(defmask),
		m_xor(0)
{
	reset();
}


//-------------------------------------------------
//  reset - reset/initialize state
//-------------------------------------------------

void devcb_base::reset(callback_type type)
{
	m_type = type;
	m_target_tag = NULL;
	m_target_int = 0;
	m_space_tag = NULL;
	m_space_num = AS_0;
	m_space = NULL;
	m_target.ptr = NULL;
	m_rshift = 0;
	m_mask = ~U64(0);
}


//-------------------------------------------------
//  resolve_ioport - resolve an I/O port or fatal
//  error if we can't find it
//-------------------------------------------------

void devcb_base::resolve_ioport()
{
	// attempt to resolve, fatal error if fail
	m_target.ioport = (m_target_tag != NULL) ? m_device.owner()->ioport(m_target_tag) : NULL;
	if (m_target.ioport == NULL)
		throw emu_fatalerror("Unable to resolve I/O port callback reference to '%s' in device '%s'\n", m_target_tag, m_device.tag());
}


//-------------------------------------------------
//  resolve_inputline - resolve a device and input
//  number or fatal error if we can't find it
//-------------------------------------------------

void devcb_base::resolve_inputline()
{
	// attempt to resolve, fatal error if fail
	m_target.device = (m_target_tag != NULL) ? m_device.owner()->subdevice(m_target_tag) : NULL;
	if (m_target.device == NULL)
		throw emu_fatalerror("Unable to resolve device reference to '%s' in device '%s'\n", m_target_tag, m_device.tag());

	// make sure we have an execute interface
	device_execute_interface *exec;
	if (!m_target.device->interface(exec))
		throw emu_fatalerror("No execute interface found for device reference to '%s' in device '%s'\n", m_target_tag, m_device.tag());
}


//-------------------------------------------------
//  resolve_space - resolve an address space or
//  fatal error if we can't find it
//-------------------------------------------------

void devcb_base::resolve_space()
{
	// attempt to resolve, fatal error if fail
	device_t *spacedev = (m_space_tag != NULL) ? m_device.owner()->subdevice(m_space_tag) : NULL;
	if (spacedev == NULL)
		throw emu_fatalerror("Unable to resolve device reference to '%s' in device '%s'\n", m_space_tag, m_device.tag());
	if (!spacedev->memory().has_space(m_space_num))
		throw emu_fatalerror("Unable to resolve device address space %d on '%s' in device '%s'\n", m_space_num, m_space_tag, m_device.tag());
	m_space = &spacedev->memory().space(m_space_num);
}



//**************************************************************************
//  DEVCB READ CLASS
//**************************************************************************

//-------------------------------------------------
//  devcb_read_base - constructor
//-------------------------------------------------

devcb_read_base::devcb_read_base(device_t &device, UINT64 defmask)
	: devcb_base(device, defmask)
{
}


//-------------------------------------------------
//  reset - reset/initialize state
//-------------------------------------------------

void devcb_read_base::reset(callback_type type)
{
	// parent first
	devcb_base::reset(type);

	// local stuff
	m_readline = read_line_delegate();
	m_read8 = read8_delegate();
	m_read16 = read16_delegate();
	m_read32 = read32_delegate();
	m_read64 = read64_delegate();
	m_adapter = &devcb_read_base::read_unresolved_adapter;
}


//-------------------------------------------------
//  resolve - resolve the specified callback to
//  its final form
//-------------------------------------------------

void devcb_read_base::resolve()
{
	// first resolve any address spaces
	if (m_space_tag != NULL)
		resolve_space();
	else
		m_space = &downcast<driver_device &>(m_device.machine().root_device()).generic_space();

	// then handle the various types
	const char *name = "unknown";
	try
	{
		switch (m_type)
		{
			case CALLBACK_NONE:
				break;

			case CALLBACK_LINE:
				name = m_readline.name();
				m_readline.bind_relative_to(*m_device.owner());
				m_target_int = 0;
				m_adapter = m_readline.isnull() ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read_line_adapter;
				break;

			case CALLBACK_8:
				name = m_read8.name();
				m_read8.bind_relative_to(*m_device.owner());
				m_target_int = 0;
				m_adapter = m_read8.isnull() ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read8_adapter;
				break;

			case CALLBACK_16:
				name = m_read16.name();
				m_read16.bind_relative_to(*m_device.owner());
				m_target_int = 0;
				m_adapter = m_read16.isnull() ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read16_adapter;
				break;

			case CALLBACK_32:
				name = m_read32.name();
				m_read32.bind_relative_to(*m_device.owner());
				m_target_int = 0;
				m_adapter = m_read32.isnull() ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read32_adapter;
				break;

			case CALLBACK_64:
				name = m_read64.name();
				m_read64.bind_relative_to(*m_device.owner());
				m_target_int = 0;
				m_adapter = m_read64.isnull() ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read64_adapter;
				break;

			case CALLBACK_IOPORT:
				resolve_ioport();
				m_target_int = 0;
				m_adapter = (m_target.ioport == NULL) ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read_ioport_adapter;
				break;

			case CALLBACK_LOG:
				m_adapter = &devcb_read_base::read_logged_adapter;
				break;

			case CALLBACK_CONSTANT:
				m_adapter = &devcb_read_base::read_constant_adapter;
				break;

			case CALLBACK_INPUTLINE:
				throw emu_fatalerror("Device read callbacks can't be connected to input lines\n");
		}
	}
	catch (binding_type_exception &binderr)
	{
		throw emu_fatalerror("devcb_read: Error performing a late bind of type %s to %s (name=%s)\n", binderr.m_actual_type.name(), binderr.m_target_type.name(), name);
	}
}


//-------------------------------------------------
//  resolve_safe - resolve the callback; if not
//  specified, resolve to a constant callback with
//  the given value
//-------------------------------------------------

void devcb_read_base::resolve_safe(UINT64 none_constant_value)
{
	// convert to a constant if none specified
	if (m_type == CALLBACK_NONE)
	{
		m_target_int = none_constant_value;
		m_type = CALLBACK_CONSTANT;
	}
	resolve();
}


//-------------------------------------------------
//  read_unresolved_adapter - error-generating
//  unresolved adapter
//-------------------------------------------------

UINT64 devcb_read_base::read_unresolved_adapter(address_space &space, offs_t offset, UINT64 mask)
{
	throw emu_fatalerror("Attempted to read through an unresolved devcb item");
}


//-------------------------------------------------
//  read_line_adapter - read from a line delegate
//-------------------------------------------------

UINT64 devcb_read_base::read_line_adapter(address_space &space, offs_t offset, UINT64 mask)
{
	return shift_mask_xor(m_readline() & 1);
}


//-------------------------------------------------
//  read8_adapter - read from an 8-bit delegate
//-------------------------------------------------

UINT64 devcb_read_base::read8_adapter(address_space &space, offs_t offset, UINT64 mask)
{
	return shift_mask_xor(m_read8(space, offset, unshift_mask(mask)));
}


//-------------------------------------------------
//  read16_adapter - read from a 16-bit delegate
//-------------------------------------------------

UINT64 devcb_read_base::read16_adapter(address_space &space, offs_t offset, UINT64 mask)
{
	return shift_mask_xor(m_read16(space, offset, unshift_mask(mask)));
}


//-------------------------------------------------
//  read32_adapter - read from a 32-bit delegate
//-------------------------------------------------

UINT64 devcb_read_base::read32_adapter(address_space &space, offs_t offset, UINT64 mask)
{
	return shift_mask_xor(m_read32(space, offset, unshift_mask(mask)));
}


//-------------------------------------------------
//  read64_adapter - read from a 64-bit delegate
//-------------------------------------------------

UINT64 devcb_read_base::read64_adapter(address_space &space, offs_t offset, UINT64 mask)
{
	return shift_mask_xor(m_read64(space, offset, unshift_mask(mask)));
}


//-------------------------------------------------
//  read_ioport - read from an I/O port
//-------------------------------------------------

UINT64 devcb_read_base::read_ioport_adapter(address_space &space, offs_t offset, UINT64 mask)
{
	return shift_mask_xor(m_target.ioport->read());
}


//-------------------------------------------------
//  read_logged_adapter - log a read and return a
//  constant
//-------------------------------------------------

UINT64 devcb_read_base::read_logged_adapter(address_space &space, offs_t offset, UINT64 mask)
{
	m_device.logerror("%s: read %s\n", m_device.machine().describe_context(), m_target_tag);
	return shift_mask_xor(m_target_int);
}


//-------------------------------------------------
//  read_constant - read from a constant
//-------------------------------------------------

UINT64 devcb_read_base::read_constant_adapter(address_space &space, offs_t offset, UINT64 mask)
{
	return shift_mask_xor(m_target_int);
}



//**************************************************************************
//  DEVCB WRITE CLASS
//**************************************************************************

//-------------------------------------------------
//  devcb_write_base - constructor
//-------------------------------------------------

devcb_write_base::devcb_write_base(device_t &device, UINT64 defmask)
	: devcb_base(device, defmask)
{
}


//-------------------------------------------------
//  reset - reset/initialize state
//-------------------------------------------------

void devcb_write_base::reset(callback_type type)
{
	// parent first
	devcb_base::reset(type);

	// local stuff
	m_writeline = write_line_delegate();
	m_write8 = write8_delegate();
	m_write16 = write16_delegate();
	m_write32 = write32_delegate();
	m_write64 = write64_delegate();
	m_adapter = &devcb_write_base::write_unresolved_adapter;
}


//-------------------------------------------------
//  resolve - resolve the specified callback to
//  its final form
//-------------------------------------------------

void devcb_write_base::resolve()
{
	// first resolve any address spaces
	if (m_space_tag != NULL)
		resolve_space();
	else
		m_space = &downcast<driver_device &>(m_device.machine().root_device()).generic_space();

	// then handle the various types
	const char *name = "unknown";
	try
	{
		switch (m_type)
		{
			case CALLBACK_NONE:
				break;

			case CALLBACK_LINE:
				name = m_writeline.name();
				m_writeline.bind_relative_to(*m_device.owner());
				m_adapter = m_writeline.isnull() ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write_line_adapter;
				break;

			case CALLBACK_8:
				name = m_write8.name();
				m_write8.bind_relative_to(*m_device.owner());
				m_adapter = m_write8.isnull() ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write8_adapter;
				break;

			case CALLBACK_16:
				name = m_write16.name();
				m_write16.bind_relative_to(*m_device.owner());
				m_adapter = m_write16.isnull() ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write16_adapter;
				break;

			case CALLBACK_32:
				name = m_write32.name();
				m_write32.bind_relative_to(*m_device.owner());
				m_adapter = m_write32.isnull() ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write32_adapter;
				break;

			case CALLBACK_64:
				name = m_write64.name();
				m_write64.bind_relative_to(*m_device.owner());
				m_adapter = m_write64.isnull() ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write64_adapter;
				break;

			case CALLBACK_IOPORT:
				resolve_ioport();
				m_adapter = (m_target.ioport == NULL) ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write_ioport_adapter;
				break;

			case CALLBACK_LOG:
				m_adapter = &devcb_write_base::write_logged_adapter;
				break;

			case CALLBACK_CONSTANT:
				m_adapter = &devcb_write_base::write_noop_adapter;
				break;

			case CALLBACK_INPUTLINE:
				resolve_inputline();
				m_adapter = &devcb_write_base::write_inputline_adapter;
				break;
		}
	}
	catch (binding_type_exception &binderr)
	{
		throw emu_fatalerror("devcb_write: Error performing a late bind of type %s to %s (name=%s)\n", binderr.m_actual_type.name(), binderr.m_target_type.name(), name);
	}
}


//-------------------------------------------------
//  resolve_safe - resolve the callback; if not
//  specified, resolve to a no-op
//-------------------------------------------------

void devcb_write_base::resolve_safe()
{
	// convert to a constant if none specified
	if (m_type == CALLBACK_NONE)
		m_type = CALLBACK_CONSTANT;
	resolve();
}


//-------------------------------------------------
//  write_unresolved_adapter - error-generating
//  unresolved adapter
//-------------------------------------------------

void devcb_write_base::write_unresolved_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	throw emu_fatalerror("Attempted to write through an unresolved devcb item");
}


//-------------------------------------------------
//  write_line_adapter - write from a line delegate
//-------------------------------------------------

void devcb_write_base::write_line_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	m_writeline(unshift_mask_xor(data) & 1);
}


//-------------------------------------------------
//  write8_adapter - write from an 8-bit delegate
//-------------------------------------------------

void devcb_write_base::write8_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	m_write8(space, offset, unshift_mask_xor(data), unshift_mask(mask));
}


//-------------------------------------------------
//  write16_adapter - write from a 16-bit delegate
//-------------------------------------------------

void devcb_write_base::write16_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	m_write16(space, offset, unshift_mask_xor(data), unshift_mask(mask));
}


//-------------------------------------------------
//  write32_adapter - write from a 32-bit delegate
//-------------------------------------------------

void devcb_write_base::write32_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	m_write32(space, offset, unshift_mask_xor(data), unshift_mask(mask));
}


//-------------------------------------------------
//  write64_adapter - write from a 64-bit delegate
//-------------------------------------------------

void devcb_write_base::write64_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	m_write64(space, offset, unshift_mask_xor(data), unshift_mask(mask));
}


//-------------------------------------------------
//  write_ioport - write from an I/O port
//-------------------------------------------------

void devcb_write_base::write_ioport_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	if (m_target.ioport)
		m_target.ioport->write(unshift_mask_xor(data));
}


//-------------------------------------------------
//  write_logged_adapter - logging unresolved
//  adapter
//-------------------------------------------------

void devcb_write_base::write_logged_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	m_device.logerror("%s: unresolved devcb write\n", m_device.machine().describe_context());
}


//-------------------------------------------------
//  write_constant - write from a constant
//-------------------------------------------------

void devcb_write_base::write_noop_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	// constant for writes is a no-op
}


//-------------------------------------------------
//  write_inputline_adapter - write to an device's
//  input line
//-------------------------------------------------

void devcb_write_base::write_inputline_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	m_target.device->execute().set_input_line(m_target_int, unshift_mask_xor(data) & 1);
}
