/***************************************************************************

    devcb.c

    Device callback interface helpers.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "devcb.h"
#include "driver.h"



/***************************************************************************
    STATIC-TO-LIVE CONVERSION
***************************************************************************/

/*-------------------------------------------------
    devcb_resolve_read_line - convert a static
    read line definition to a live definition
-------------------------------------------------*/

static READ_LINE_DEVICE_HANDLER( trampoline_read_port_to_read_line )
{
	return (input_port_read_direct((const input_port_config *)device) & 1) ? ASSERT_LINE : CLEAR_LINE;
}

static READ_LINE_DEVICE_HANDLER( trampoline_read8_to_read_line )
{
	const devcb_resolved_read_line *resolved = (const devcb_resolved_read_line *)device;
	return ((*resolved->real.readdevice)((const device_config *)resolved->realtarget, 0) & 1) ? ASSERT_LINE : CLEAR_LINE;
}

void devcb_resolve_read_line(devcb_resolved_read_line *resolved, const devcb_read_line *config, const device_config *device)
{
	/* reset the resolved structure */
	memset(resolved, 0, sizeof(*resolved));

	/* input port handlers */
	if (config->type == DEVCB_TYPE_INPUT)
	{
		resolved->target = input_port_by_tag(&device->machine->portlist, config->tag);
		if (resolved->target == NULL)
			fatalerror("devcb_resolve_read_line: unable to find input port '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);
		resolved->read = trampoline_read_port_to_read_line;
	}

	/* address space handlers */
	else if (config->type >= DEVCB_TYPE_MEMORY(ADDRESS_SPACE_PROGRAM) && config->type < DEVCB_TYPE_MEMORY(ADDRESS_SPACES) && config->readspace != NULL)
	{
		FPTR space = (FPTR)config->type - (FPTR)DEVCB_TYPE_MEMORY(ADDRESS_SPACE_PROGRAM);
		const device_config *cpu;

		if (device->owner != NULL)
			cpu = device_find_child_by_tag(device->owner, config->tag);
		else
			cpu = cputag_get_cpu(device->machine, config->tag);

		if (cpu == NULL)
			fatalerror("devcb_resolve_read_line: unable to find CPU '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);
		resolved->target = resolved;
		resolved->read = trampoline_read8_to_read_line;
		resolved->realtarget = memory_find_address_space(cpu, space);
		if (resolved->realtarget == NULL)
			fatalerror("devcb_resolve_read_line: unable to find CPU '%s' %s space (requested by %s '%s')", config->tag, address_space_names[space], device_get_name(device), device->tag);
		resolved->real.readspace = config->readspace;
	}

	/* device handlers */
	else if ((config->type == DEVCB_TYPE_DEVICE || config->type == DEVCB_TYPE_SELF) && (config->readline != NULL || config->readdevice != NULL))
	{
		/* locate the device */
		if (config->type == DEVCB_TYPE_SELF)
			resolved->target = device;
		else if (device->owner != NULL)
			resolved->target = device_find_child_by_tag(device->owner, config->tag);
		else
			resolved->target = devtag_get_device(device->machine, config->tag);

		if (resolved->target == NULL)
			fatalerror("devcb_resolve_read_line: unable to find device '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);

		/* read_line to read_line is direct */
		if (config->readline != NULL)
			resolved->read = config->readline;

		/* read_line to handler goes through a trampoline */
		else
		{
			resolved->realtarget = resolved->target;
			resolved->real.readdevice = config->readdevice;
			resolved->target = resolved;
			resolved->read = trampoline_read8_to_read_line;
		}
	}
}


/*-------------------------------------------------
    devcb_resolve_write_line - convert a static
    write line definition to a live definition
-------------------------------------------------*/

static WRITE_LINE_DEVICE_HANDLER( trampoline_write_port_to_write_line )
{
	input_port_write_direct((const input_port_config *)device, state, 0xffffffff);
}

static WRITE_LINE_DEVICE_HANDLER( trampoline_write8_to_write_line )
{
	const devcb_resolved_write_line *resolved = (const devcb_resolved_write_line *)device;
	(*resolved->real.writedevice)((const device_config *)resolved->realtarget, 0, state);
}

static WRITE_LINE_DEVICE_HANDLER( trampoline_writecpu_to_write_line )
{
	const devcb_resolved_write_line *resolved = (const devcb_resolved_write_line *)device;
	const device_config *cpu = (const device_config *)resolved->realtarget;
	cpu_set_input_line(cpu, resolved->real.writeline, state ? ASSERT_LINE : CLEAR_LINE);
}

void devcb_resolve_write_line(devcb_resolved_write_line *resolved, const devcb_write_line *config, const device_config *device)
{
	/* reset the resolved structure */
	memset(resolved, 0, sizeof(*resolved));

	if (config->type == DEVCB_TYPE_INPUT)
	{
		resolved->target = input_port_by_tag(&device->machine->portlist, config->tag);
		if (resolved->target == NULL)
			fatalerror("devcb_resolve_write_line: unable to find input port '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);
		resolved->write = trampoline_write_port_to_write_line;
	}

	/* address space handlers */
	else if (config->type >= DEVCB_TYPE_MEMORY(ADDRESS_SPACE_PROGRAM) && config->type < DEVCB_TYPE_MEMORY(ADDRESS_SPACES) && config->writespace != NULL)
	{
		FPTR space = (FPTR)config->type - (FPTR)DEVCB_TYPE_MEMORY(ADDRESS_SPACE_PROGRAM);
		const device_config *cpu;

		if (device->owner != NULL)
			cpu = device_find_child_by_tag(device->owner, config->tag);
		else
			cpu = cputag_get_cpu(device->machine, config->tag);

		if (cpu == NULL)
			fatalerror("devcb_resolve_write_line: unable to find CPU '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);
		resolved->target = resolved;
		resolved->write = trampoline_write8_to_write_line;
		resolved->realtarget = memory_find_address_space(cpu, space);
		if (resolved->realtarget == NULL)
			fatalerror("devcb_resolve_write_line: unable to find CPU '%s' %s space (requested by %s '%s')", config->tag, address_space_names[space], device_get_name(device), device->tag);
		resolved->real.writespace = config->writespace;
	}

	/* cpu line handlers */
	else if (config->type >= DEVCB_TYPE_CPU_LINE(0) && config->type < DEVCB_TYPE_CPU_LINE(MAX_INPUT_LINES))
	{
		FPTR line = (FPTR)config->type - (FPTR)DEVCB_TYPE_CPU_LINE(0);
		const device_config *cpu;

		if (device->owner != NULL)
			cpu = device_find_child_by_tag(device->owner, config->tag);
		else
			cpu = cputag_get_cpu(device->machine, config->tag);

		if (cpu == NULL)
			fatalerror("devcb_resolve_write_line: unable to find CPU '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);
		resolved->target = resolved;
		resolved->write = trampoline_writecpu_to_write_line;
		resolved->realtarget = cpu;
		resolved->real.writeline = (int) line;
	}

	/* device handlers */
	else if ((config->type == DEVCB_TYPE_DEVICE || config->type == DEVCB_TYPE_SELF) && (config->writeline != NULL || config->writedevice != NULL))
	{
		/* locate the device */
		if (config->type == DEVCB_TYPE_SELF)
			resolved->target = device;
		else if (device->owner != NULL)
			resolved->target = device_find_child_by_tag(device->owner, config->tag);
		else
			resolved->target = devtag_get_device(device->machine, config->tag);

		if (resolved->target == NULL)
			fatalerror("devcb_resolve_write_line: unable to find device '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);

		/* write_line to write_line is direct */
		if (config->writeline != NULL)
			resolved->write = config->writeline;

		/* write_line to handler goes through a trampoline */
		else
		{
			resolved->realtarget = resolved->target;
			resolved->real.writedevice = config->writedevice;
			resolved->target = resolved;
			resolved->write = trampoline_write8_to_write_line;
		}
	}
}


/*-------------------------------------------------
    devcb_resolve_read8 - convert a static
    8-bit read definition to a live definition
-------------------------------------------------*/

static READ8_DEVICE_HANDLER( trampoline_read_port_to_read8 )
{
	return input_port_read_direct((const input_port_config *)device);
}

static READ8_DEVICE_HANDLER( trampoline_read_line_to_read8 )
{
	const devcb_resolved_read8 *resolved = (const devcb_resolved_read8 *)device;
	return (*resolved->real.readline)((const device_config *)resolved->realtarget);
}

void devcb_resolve_read8(devcb_resolved_read8 *resolved, const devcb_read8 *config, const device_config *device)
{
	/* reset the resolved structure */
	memset(resolved, 0, sizeof(*resolved));

	/* input port handlers */
	if (config->type == DEVCB_TYPE_INPUT)
	{
		resolved->target = input_port_by_tag(&device->machine->portlist, config->tag);
		if (resolved->target == NULL)
			fatalerror("devcb_resolve_read8: unable to find input port '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);
		resolved->read = trampoline_read_port_to_read8;
	}

	/* address space handlers */
	else if (config->type >= DEVCB_TYPE_MEMORY(ADDRESS_SPACE_PROGRAM) && config->type < DEVCB_TYPE_MEMORY(ADDRESS_SPACES) && config->readspace != NULL)
	{
		FPTR space = (FPTR)config->type - (FPTR)DEVCB_TYPE_MEMORY(ADDRESS_SPACE_PROGRAM);
		const device_config *cpu;

		if (device->owner != NULL)
			cpu = device_find_child_by_tag(device->owner, config->tag);
		else
			cpu = cputag_get_cpu(device->machine, config->tag);

		if (cpu == NULL)
			fatalerror("devcb_resolve_read8: unable to find CPU '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);
		resolved->target = memory_find_address_space(cpu, space);
		if (resolved->target == NULL)
			fatalerror("devcb_resolve_read8: unable to find CPU '%s' %s space (requested by %s '%s')", config->tag, address_space_names[space], device_get_name(device), device->tag);
		resolved->read = (read8_device_func)config->readspace;
	}

	/* device handlers */
	else if ((config->type == DEVCB_TYPE_DEVICE || config->type == DEVCB_TYPE_SELF) && (config->readline != NULL || config->readdevice != NULL))
	{
		resolved->target = (config->type == DEVCB_TYPE_SELF) ? device : devtag_get_device(device->machine, config->tag);
		if (resolved->target == NULL)
			fatalerror("devcb_resolve_read8: unable to find device '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);

		/* read8 to read8 is direct */
		if (config->readdevice != NULL)
			resolved->read = config->readdevice;

		/* read8 to read_line goes through a trampoline */
		else
		{
			resolved->realtarget = resolved->target;
			resolved->real.readline = config->readline;
			resolved->target = resolved;
			resolved->read = trampoline_read_line_to_read8;
		}
	}
}


/*-------------------------------------------------
    devcb_resolve_write8 - convert a static
    8-bit write definition to a live definition
-------------------------------------------------*/

static WRITE8_DEVICE_HANDLER( trampoline_write_port_to_write8 )
{
	input_port_write_direct((const input_port_config *)device, data, 0xff);
}

static WRITE8_DEVICE_HANDLER( trampoline_write_line_to_write8 )
{
	const devcb_resolved_write8 *resolved = (const devcb_resolved_write8 *)device;
	(*resolved->real.writeline)((const device_config *)resolved->realtarget, (data & 1) ? ASSERT_LINE : CLEAR_LINE);
}

void devcb_resolve_write8(devcb_resolved_write8 *resolved, const devcb_write8 *config, const device_config *device)
{
	/* reset the resolved structure */
	memset(resolved, 0, sizeof(*resolved));

	if (config->type == DEVCB_TYPE_INPUT)
	{
		resolved->target = input_port_by_tag(&device->machine->portlist, config->tag);
		if (resolved->target == NULL)
			fatalerror("devcb_resolve_read_line: unable to find input port '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);
		resolved->write = trampoline_write_port_to_write8;
	}

	/* address space handlers */
	else if (config->type >= DEVCB_TYPE_MEMORY(ADDRESS_SPACE_PROGRAM) && config->type < DEVCB_TYPE_MEMORY(ADDRESS_SPACES) && config->writespace != NULL)
	{
		FPTR space = (FPTR)config->type - (FPTR)DEVCB_TYPE_MEMORY(ADDRESS_SPACE_PROGRAM);
		const device_config *cpu;

		if (device->owner != NULL)
			cpu = device_find_child_by_tag(device->owner, config->tag);
		else
			cpu = cputag_get_cpu(device->machine, config->tag);

		if (cpu == NULL)
			fatalerror("devcb_resolve_write8: unable to find CPU '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);
		resolved->target = memory_find_address_space(cpu, space);
		if (resolved->target == NULL)
			fatalerror("devcb_resolve_write8: unable to find CPU '%s' %s space (requested by %s '%s')", config->tag, address_space_names[space], device_get_name(device), device->tag);
		resolved->write = (write8_device_func)config->writespace;
	}

	/* device handlers */
	else if ((config->type == DEVCB_TYPE_DEVICE || config->type == DEVCB_TYPE_SELF) && (config->writeline != NULL || config->writedevice != NULL))
	{
		resolved->target = (config->type == DEVCB_TYPE_SELF) ? device : devtag_get_device(device->machine, config->tag);
		if (resolved->target == NULL)
			fatalerror("devcb_resolve_write8: unable to find device '%s' (requested by %s '%s')", config->tag, device_get_name(device), device->tag);

		/* write8 to write8 is direct */
		if (config->writedevice != NULL)
			resolved->write = config->writedevice;

		/* write8 to write_line goes through a trampoline */
		else
		{
			resolved->realtarget = resolved->target;
			resolved->real.writeline = config->writeline;
			resolved->target = resolved;
			resolved->write = trampoline_write_line_to_write8;
		}
	}
}
