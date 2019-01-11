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

devcb_base::devcb_base(device_t &device, u64 defmask)
	: m_owner(device)
	, m_base(device)
	, m_type(CALLBACK_NONE)
	, m_target_tag(nullptr)
	, m_target_int(0)
	, m_space_tag(nullptr)
	, m_space_num(0)
	, m_space(nullptr)
	, m_rshift(0)
	, m_mask(defmask)
	, m_defmask(defmask)
	, m_xor(0)
{
	m_target.ptr = nullptr;
}


//-------------------------------------------------
//  devcb_base - destructor
//-------------------------------------------------

devcb_base::~devcb_base()
{
}


//-------------------------------------------------
//  reset - reset/initialize state
//-------------------------------------------------

void devcb_base::reset(callback_type type)
{
	reset(m_owner.mconfig().current_device(), type);
}

void devcb_base::reset(device_t &base, callback_type type)
{
	m_base = base;
	m_type = type;
	m_target_tag = nullptr;
	m_target_int = 0;
	m_space_tag = nullptr;
	m_space_num = 0;
	m_space = nullptr;
	m_target.ptr = nullptr;
	m_rshift = 0;
	m_mask = m_defmask;
	m_xor = 0;
	devcb_reset();
}


//-------------------------------------------------
//  resolve_ioport - resolve an I/O port or fatal
//  error if we can't find it
//-------------------------------------------------

void devcb_base::resolve_ioport()
{
	// attempt to resolve, fatal error if fail
	m_target.ioport = m_target_tag ? m_base.get().ioport(m_target_tag) : nullptr;
	if (!m_target.ioport)
		throw emu_fatalerror("Unable to resolve I/O port callback reference to '%s' in device '%s'\n", m_target_tag, m_base.get().tag());

	// adjust the mask to match the port bits
	u64 port_mask = 0;
	for (const ioport_field &field : m_target.ioport->fields())
		port_mask |= field.mask();
	m_mask = shift_mask(port_mask);
}


//-------------------------------------------------
//  resolve_membank - resolve a memory bank or
//  fatal error if we can't find it
//-------------------------------------------------

void devcb_base::resolve_membank()
{
	// attempt to resolve, fatal error if fail
	m_target.membank = m_target_tag ? m_base.get().membank(m_target_tag) : nullptr;
	if (!m_target.membank)
		throw emu_fatalerror("Unable to resolve memory bank callback reference to '%s' in device '%s'\n", m_target_tag, m_base.get().tag());
}


//-------------------------------------------------
//  resolve_output - resolve an output item
//-------------------------------------------------

void devcb_base::resolve_output()
{
	assert(m_target_tag);
	m_target.item = &m_base.get().machine().output().find_or_create_item(m_target_tag, 0);
}


//-------------------------------------------------
//  resolve_inputline - resolve a device and input
//  number or fatal error if we can't find it
//-------------------------------------------------

void devcb_base::resolve_inputline()
{
	// attempt to resolve, fatal error if fail
	m_target.device = m_target_tag ? m_base.get().subdevice(m_target_tag) : nullptr;
	if (!m_target.device)
		throw emu_fatalerror("Unable to resolve device reference to '%s' in device '%s'\n", m_target_tag, m_base.get().tag());

	// make sure we have an execute interface
	device_execute_interface *exec;
	if (!m_target.device->interface(exec))
		throw emu_fatalerror("No execute interface found for device reference to '%s' in device '%s'\n", m_target_tag, m_base.get().tag());
}


//-------------------------------------------------
//  resolve_space - resolve an address space or
//  fatal error if we can't find it
//-------------------------------------------------

void devcb_base::resolve_space()
{
	// attempt to resolve, fatal error if fail
	device_t *const spacedev(m_space_tag ? m_base.get().subdevice(m_space_tag) : nullptr);
	if (!spacedev)
		throw emu_fatalerror("Unable to resolve device reference to '%s' in device '%s'\n", m_space_tag, m_base.get().tag());
	if (!spacedev->memory().has_space(m_space_num))
		throw emu_fatalerror("Unable to resolve device address space %d on '%s' in device '%s'\n", m_space_num, m_space_tag, m_base.get().tag());
	m_space = &spacedev->memory().space(m_space_num);
}



//**************************************************************************
//  DEVCB READ CLASS
//**************************************************************************

//-------------------------------------------------
//  devcb_read_base - constructor
//-------------------------------------------------

devcb_read_base::devcb_read_base(device_t &device, u64 defmask, bool chained)
	: devcb_base(device, defmask),
		m_adapter(&devcb_read_base::read_unresolved_adapter)
{
	if (!chained)
		device.m_input_callbacks.push_back(this);
}


//-------------------------------------------------
//  devcb_reset - reset/initialize local state
//-------------------------------------------------

void devcb_read_base::devcb_reset()
{
	m_readline = read_line_delegate();
	m_read8 = read8_delegate();
	m_read16 = read16_delegate();
	m_read32 = read32_delegate();
	m_read64 = read64_delegate();
	m_adapter = &devcb_read_base::read_unresolved_adapter;
	m_chain = nullptr;
}


//-------------------------------------------------
//  chain_alloc - add another callback to the
//  input chain
//-------------------------------------------------

devcb_read_base &devcb_read_base::chain_alloc()
{
	// set up the chained callback pointer
	m_chain.reset(new devcb_read_base(m_owner, m_defmask, true));
	return *m_chain;
}


//-------------------------------------------------
//  resolve - resolve the specified callback to
//  its final form
//-------------------------------------------------

void devcb_read_base::resolve()
{
	// first resolve any address spaces
	if (m_space_tag)
		resolve_space();
	else
		m_space = &m_owner.machine().dummy_space();

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
			m_readline.bind_relative_to(m_base);
			m_target_int = 0;
			m_adapter = m_readline.isnull() ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read_line_adapter;
			m_mask = shift_mask(1);
			break;

		case CALLBACK_8:
			name = m_read8.name();
			m_read8.bind_relative_to(m_base);
			m_target_int = 0;
			m_adapter = m_read8.isnull() ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read8_adapter;
			m_mask = shift_mask(0xff);
			break;

		case CALLBACK_16:
			name = m_read16.name();
			m_read16.bind_relative_to(m_base);
			m_target_int = 0;
			m_adapter = m_read16.isnull() ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read16_adapter;
			m_mask = shift_mask(0xffff);
			break;

		case CALLBACK_32:
			name = m_read32.name();
			m_read32.bind_relative_to(m_base);
			m_target_int = 0;
			m_adapter = m_read32.isnull() ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read32_adapter;
			m_mask = shift_mask(0xffffffff);
			break;

		case CALLBACK_64:
			name = m_read64.name();
			m_read64.bind_relative_to(m_base);
			m_target_int = 0;
			m_adapter = m_read64.isnull() ? &devcb_read_base::read_constant_adapter : &devcb_read_base::read64_adapter;
			break;

		case CALLBACK_IOPORT:
			name = m_target_tag;
			resolve_ioport();
			m_target_int = 0;
			m_adapter = &devcb_read_base::read_ioport_adapter;
			break;

		case CALLBACK_MEMBANK:
			throw emu_fatalerror("Device read callbacks can't be connected to bank switches\n");

		case CALLBACK_OUTPUT:
			throw emu_fatalerror("Device read callbacks can't be connected to output items\n");

		case CALLBACK_LOG:
			m_adapter = &devcb_read_base::read_logged_adapter;
			m_mask = 0;
			break;

		case CALLBACK_CONSTANT:
			if (m_xor != 0)
				throw emu_fatalerror("devcb_read: Attempt to invert constant value (%lX ^ %lX)\n", (unsigned long)shift_mask(m_target_int), (unsigned long)m_xor);
			m_adapter = &devcb_read_base::read_constant_adapter;
			m_mask = shift_mask(m_target_int);
			break;

		case CALLBACK_INPUTLINE:
		case CALLBACK_ASSERTLINE:
		case CALLBACK_CLEARLINE:
		case CALLBACK_HOLDLINE:
			throw emu_fatalerror("Device read callbacks can't be connected to input lines\n");
		}
	}
	catch (binding_type_exception &binderr)
	{
		throw emu_fatalerror("devcb_read: Error performing a late bind of type %s to %s (name=%s)\n", binderr.m_actual_type.name(), binderr.m_target_type.name(), name);
	}

	// resolve callback chain recursively
	if (m_chain != nullptr)
		m_chain->resolve();

	// protect against bus contention (the masks must not overlap)
	for (const devcb_read_base *chained_cb = m_chain.get(); chained_cb != nullptr; chained_cb = chained_cb->m_chain.get())
		if ((m_mask & chained_cb->m_mask) != 0)
			throw emu_fatalerror("Device %s read callback (name=%s) overlaps with chained callback (%lX & %lX)", m_owner.tag(), name, (unsigned long)m_mask, (unsigned long)chained_cb->m_mask);
}


//-------------------------------------------------
//  resolve_safe - resolve the callback; if not
//  specified, resolve to a constant callback with
//  the given value
//-------------------------------------------------

void devcb_read_base::resolve_safe(u64 none_constant_value)
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

u64 devcb_read_base::read_unresolved_adapter(address_space &space, offs_t offset, u64 mask)
{
	throw emu_fatalerror("Attempted to read through an unresolved devcb item");
}


//-------------------------------------------------
//  read_line_adapter - read from a line delegate
//-------------------------------------------------

u64 devcb_read_base::read_line_adapter(address_space &space, offs_t offset, u64 mask)
{
	return shift_mask_xor(m_readline() & 1);
}


//-------------------------------------------------
//  read8_adapter - read from an 8-bit delegate
//-------------------------------------------------

u64 devcb_read_base::read8_adapter(address_space &space, offs_t offset, u64 mask)
{
	return shift_mask_xor(m_read8(space, offset, unshift_mask(mask)));
}


//-------------------------------------------------
//  read16_adapter - read from a 16-bit delegate
//-------------------------------------------------

u64 devcb_read_base::read16_adapter(address_space &space, offs_t offset, u64 mask)
{
	return shift_mask_xor(m_read16(space, offset, unshift_mask(mask)));
}


//-------------------------------------------------
//  read32_adapter - read from a 32-bit delegate
//-------------------------------------------------

u64 devcb_read_base::read32_adapter(address_space &space, offs_t offset, u64 mask)
{
	return shift_mask_xor(m_read32(space, offset, unshift_mask(mask)));
}


//-------------------------------------------------
//  read64_adapter - read from a 64-bit delegate
//-------------------------------------------------

u64 devcb_read_base::read64_adapter(address_space &space, offs_t offset, u64 mask)
{
	return shift_mask_xor(m_read64(space, offset, unshift_mask(mask)));
}


//-------------------------------------------------
//  read_ioport - read from an I/O port
//-------------------------------------------------

u64 devcb_read_base::read_ioport_adapter(address_space &space, offs_t offset, u64 mask)
{
	return shift_mask_xor(m_target.ioport->read());
}


//-------------------------------------------------
//  read_logged_adapter - log a read and return
//  zero
//-------------------------------------------------

u64 devcb_read_base::read_logged_adapter(address_space &space, offs_t offset, u64 mask)
{
	m_owner.logerror("%s: %s\n", m_base.get().machine().describe_context(), m_target_tag);
	return 0;
}


//-------------------------------------------------
//  read_constant - read from a constant
//-------------------------------------------------

u64 devcb_read_base::read_constant_adapter(address_space &space, offs_t offset, u64 mask)
{
	return shift_mask(m_target_int);
}


//-------------------------------------------------
//  validity_check - check the validity of the
//  callback object
//-------------------------------------------------

void devcb_read_base::validity_check(validity_checker &valid) const
{
	switch (m_type)
	{
	case CALLBACK_NONE:
	case CALLBACK_CONSTANT:
	case CALLBACK_LOG:
	case CALLBACK_IOPORT:
		break;

	case CALLBACK_LINE:
		if (!m_base.get().subdevice(m_readline.device_name()))
			osd_printf_error("Device %s not found for READLINE callback (%s)\n", m_readline.device_name(), m_readline.name());
		break;

	case CALLBACK_8:
		if (!m_base.get().subdevice(m_read8.device_name()))
			osd_printf_error("Device %s not found for READ8 callback (%s)\n", m_read8.device_name(), m_read8.name());
		break;

	case CALLBACK_16:
		if (!m_base.get().subdevice(m_read16.device_name()))
			osd_printf_error("Device %s not found for READ16 callback (%s)\n", m_read16.device_name(), m_read16.name());
		break;

	case CALLBACK_32:
		if (!m_base.get().subdevice(m_read32.device_name()))
			osd_printf_error("Device %s not found for READ32 callback (%s)\n", m_read32.device_name(), m_read32.name());
		break;

	case CALLBACK_64:
		if (!m_base.get().subdevice(m_read64.device_name()))
			osd_printf_error("Device %s not found for READ64 callback (%s)\n", m_read64.device_name(), m_read64.name());
		break;

	case CALLBACK_MEMBANK:
		osd_printf_error("Device read callbacks can't be connected to bank switches\n");
		break;

	case CALLBACK_OUTPUT:
		osd_printf_error("Device read callbacks can't be connected to output items\n");
		break;

	case CALLBACK_INPUTLINE:
	case CALLBACK_ASSERTLINE:
	case CALLBACK_CLEARLINE:
	case CALLBACK_HOLDLINE:
		throw emu_fatalerror("Device read callbacks can't be connected to input lines\n");
	}

	if (m_chain)
		m_chain->validity_check(valid);
}



//**************************************************************************
//  DEVCB WRITE CLASS
//**************************************************************************

//-------------------------------------------------
//  devcb_write_base - constructor
//-------------------------------------------------

devcb_write_base::devcb_write_base(device_t &device, u64 defmask, bool chained)
	: devcb_base(device, defmask),
		m_adapter(&devcb_write_base::write_unresolved_adapter)
{
	if (!chained)
		device.m_output_callbacks.push_back(this);
}


//-------------------------------------------------
//  devcb_reset - reset/initialize local state
//-------------------------------------------------

void devcb_write_base::devcb_reset()
{
	m_writeline = write_line_delegate();
	m_write8 = write8_delegate();
	m_write16 = write16_delegate();
	m_write32 = write32_delegate();
	m_write64 = write64_delegate();
	m_adapter = &devcb_write_base::write_unresolved_adapter;
	m_chain = nullptr;
}


//-------------------------------------------------
//  chain_alloc - add another callback to the
//  output chain
//-------------------------------------------------

devcb_write_base &devcb_write_base::chain_alloc()
{
	// set up the chained callback pointer
	m_chain.reset(new devcb_write_base(m_owner, m_defmask, true));
	return *m_chain;
}


//-------------------------------------------------
//  resolve - resolve the specified callback to
//  its final form
//-------------------------------------------------

void devcb_write_base::resolve()
{
	// first resolve any address spaces
	if (m_space_tag)
		resolve_space();
	else
		m_space = &m_owner.machine().dummy_space();

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
			m_writeline.bind_relative_to(m_base);
			m_adapter = m_writeline.isnull() ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write_line_adapter;
			break;

		case CALLBACK_8:
			name = m_write8.name();
			m_write8.bind_relative_to(m_base);
			m_adapter = m_write8.isnull() ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write8_adapter;
			break;

		case CALLBACK_16:
			name = m_write16.name();
			m_write16.bind_relative_to(m_base);
			m_adapter = m_write16.isnull() ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write16_adapter;
			break;

		case CALLBACK_32:
			name = m_write32.name();
			m_write32.bind_relative_to(m_base);
			m_adapter = m_write32.isnull() ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write32_adapter;
			break;

		case CALLBACK_64:
			name = m_write64.name();
			m_write64.bind_relative_to(m_base);
			m_adapter = m_write64.isnull() ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write64_adapter;
			break;

		case CALLBACK_IOPORT:
			resolve_ioport();
			m_adapter = !m_target.ioport ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write_ioport_adapter;
			break;

		case CALLBACK_MEMBANK:
			resolve_membank();
			m_adapter = !m_target.membank ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write_membank_adapter;
			break;

		case CALLBACK_OUTPUT:
			resolve_output();
			m_adapter = !m_target.item ? &devcb_write_base::write_noop_adapter : &devcb_write_base::write_output_adapter;
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

		case CALLBACK_ASSERTLINE:
			resolve_inputline();
			m_adapter = &devcb_write_base::write_assertline_adapter;
			break;

		case CALLBACK_CLEARLINE:
			resolve_inputline();
			m_adapter = &devcb_write_base::write_clearline_adapter;
			break;

		case CALLBACK_HOLDLINE:
			resolve_inputline();
			m_adapter = &devcb_write_base::write_holdline_adapter;
			break;
		}
	}
	catch (binding_type_exception &binderr)
	{
		throw emu_fatalerror("devcb_write: Error performing a late bind of type %s to %s (name=%s)\n", binderr.m_actual_type.name(), binderr.m_target_type.name(), name);
	}

	// resolve callback chain recursively
	if (m_chain)
		m_chain->resolve();
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
//  validity_check - check the validity of the
//  callback object
//-------------------------------------------------

void devcb_write_base::validity_check(validity_checker &valid) const
{
	switch (m_type)
	{
	case CALLBACK_NONE:
		break;

	case CALLBACK_CONSTANT:
	case CALLBACK_LOG:
	case CALLBACK_IOPORT:
	case CALLBACK_MEMBANK:
	case CALLBACK_OUTPUT:
		break;

	case CALLBACK_LINE:
		if (!m_base.get().subdevice(m_writeline.device_name()))
			osd_printf_error("Device %s not found for WRITELINE callback (%s)\n", m_writeline.device_name(), m_writeline.name());
		break;

	case CALLBACK_8:
		if (!m_base.get().subdevice(m_write8.device_name()))
			osd_printf_error("Device %s not found for WRITE8 callback (%s)\n", m_write8.device_name(), m_write8.name());
		break;

	case CALLBACK_16:
		if (!m_base.get().subdevice(m_write16.device_name()))
			osd_printf_error("Device %s not found for WRITE16 callback (%s)\n", m_write16.device_name(), m_write16.name());
		break;

	case CALLBACK_32:
		if (!m_base.get().subdevice(m_write32.device_name()))
			osd_printf_error("Device %s not found for WRITE32 callback (%s)\n", m_write32.device_name(), m_write32.name());
		break;

	case CALLBACK_64:
		if (!m_base.get().subdevice(m_write64.device_name()))
			osd_printf_error("Device %s not found for WRITE64 callback (%s)\n", m_write64.device_name(), m_write64.name());
		break;

	case CALLBACK_INPUTLINE:
	case CALLBACK_ASSERTLINE:
	case CALLBACK_CLEARLINE:
	case CALLBACK_HOLDLINE:
		{
			device_t *const device = m_base.get().subdevice(m_target_tag);
			device_execute_interface *execute;
			if (!device)
				osd_printf_error("Device %s not found for INPUTLINE callback (%d)\n", m_target_tag, int(m_target_int));
			else if (!device->interface(execute))
				osd_printf_error("Device %s has no interface for INPUTLINE callback (%d)\n", m_target_tag, int(m_target_int));
		}
		break;
	}

	if (m_chain)
		m_chain->validity_check(valid);
}


//-------------------------------------------------
//  write_unresolved_adapter - error-generating
//  unresolved adapter
//-------------------------------------------------

void devcb_write_base::write_unresolved_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	throw emu_fatalerror("Attempted to write through an unresolved devcb item");
}


//-------------------------------------------------
//  write_line_adapter - write from a line delegate
//-------------------------------------------------

void devcb_write_base::write_line_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	m_writeline(unshift_mask_xor(data) & 1);
}


//-------------------------------------------------
//  write8_adapter - write from an 8-bit delegate
//-------------------------------------------------

void devcb_write_base::write8_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	m_write8(space, offset, unshift_mask_xor(data), unshift_mask(mask));
}


//-------------------------------------------------
//  write16_adapter - write from a 16-bit delegate
//-------------------------------------------------

void devcb_write_base::write16_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	m_write16(space, offset, unshift_mask_xor(data), unshift_mask(mask));
}


//-------------------------------------------------
//  write32_adapter - write from a 32-bit delegate
//-------------------------------------------------

void devcb_write_base::write32_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	m_write32(space, offset, unshift_mask_xor(data), unshift_mask(mask));
}


//-------------------------------------------------
//  write64_adapter - write from a 64-bit delegate
//-------------------------------------------------

void devcb_write_base::write64_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	m_write64(space, offset, unshift_mask_xor(data), unshift_mask(mask));
}


//-------------------------------------------------
//  write_ioport_adapter - write to an I/O port
//-------------------------------------------------

void devcb_write_base::write_ioport_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	if (m_target.ioport)
		m_target.ioport->write(unshift_mask_xor(data));
}


//-------------------------------------------------
//  write_membank_adapter - switch a memory bank
//-------------------------------------------------

void devcb_write_base::write_membank_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	if (m_target.membank)
		m_target.membank->set_entry(unshift_mask_xor(data));
}


//-------------------------------------------------
//  write_output_adapter - set an output item
//-------------------------------------------------

void devcb_write_base::write_output_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	if (m_target.item)
		m_target.item->set(unshift_mask_xor(data));
}


//-------------------------------------------------
//  write_logged_adapter - log write if masked
//  value is nonzero
//-------------------------------------------------

void devcb_write_base::write_logged_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	if (unshift_mask_xor(data) != 0)
		m_owner.logerror("%s: %s\n", m_owner.machine().describe_context(), m_base.get().subtag(m_target_tag));
}


//-------------------------------------------------
//  write_constant - write from a constant
//-------------------------------------------------

void devcb_write_base::write_noop_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	// constant for writes is a no-op
}


//-------------------------------------------------
//  write_inputline_adapter - write to a device's
//  input line
//-------------------------------------------------

void devcb_write_base::write_inputline_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	m_target.device->execute().set_input_line(m_target_int, unshift_mask_xor(data) & 1);
}


//-------------------------------------------------
//  write_assertline_adapter - write to a device's
//  input line
//-------------------------------------------------

void devcb_write_base::write_assertline_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	if (unshift_mask_xor(data) & 1)
		m_target.device->execute().set_input_line(m_target_int, ASSERT_LINE);
}


//-------------------------------------------------
//  write_clearline_adapter - write to a device's
//  input line
//-------------------------------------------------

void devcb_write_base::write_clearline_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	if (unshift_mask_xor(data) & 1)
		m_target.device->execute().set_input_line(m_target_int, CLEAR_LINE);
}

//-------------------------------------------------
//  write_clearline_adapter - write to a device's
//  input line
//-------------------------------------------------

void devcb_write_base::write_holdline_adapter(address_space &space, offs_t offset, u64 data, u64 mask)
{
	if (unshift_mask_xor(data) & 1)
		m_target.device->execute().set_input_line(m_target_int, HOLD_LINE);
}
