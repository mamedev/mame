/***************************************************************************

    devcb.h

    Device callback interface helpers.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    These functions are used to adapt multiple read/write handler types
    to be used with device I/O. In general, a device is expected to
    declare its desired callback type, and these functions allow other
    callback types to be adapted appropriately.

    The desired callback types currently supported include:

        read_line_device_func:  (device)
        write_line_device_func: (device, data)
        read8_device_func:      (device, offset)
        write8_device_func:     (device, offset, data)

    The adapted callback types supported are:

        input port              (port)
        cpu input line          (cpu input line)
        read_line_device_func:  (device)
        write_line_device_func: (device, data)
        read8_device_func:      (device, offset)
        write8_device_func:     (device, offset, data)
        read8_space_func:       (space, offset)
        write8_space_func:      (space, offset, data)

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DEVCB_H__
#define __DEVCB_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define DEVCB_TYPE_NULL				(0)
#define DEVCB_TYPE_SELF				(1)
#define DEVCB_TYPE_INPUT			(2)
#define DEVCB_TYPE_DEVICE			(3)
#define DEVCB_TYPE_MEMORY(space)	(4 + (space))
#define DEVCB_TYPE_CPU_LINE(line)	(4 + ADDRESS_SPACES + (line))



/***************************************************************************
    MACROS
***************************************************************************/

#define DEVCB_NULL							{ DEVCB_TYPE_NULL }

/* standard line or read/write handlers with the calling device passed */
#define DEVCB_LINE(func)					{ DEVCB_TYPE_SELF, NULL, (func), NULL, NULL }
#define DEVCB_LINE_GND						{ DEVCB_TYPE_SELF, NULL, devcb_line_gnd_r, NULL, NULL }
#define DEVCB_LINE_VCC						{ DEVCB_TYPE_SELF, NULL, devcb_line_vcc_r, NULL, NULL }
#define DEVCB_HANDLER(func)					{ DEVCB_TYPE_SELF, NULL, NULL, (func), NULL }

/* line or read/write handlers for another device */
#define DEVCB_DEVICE_LINE(tag,func)			{ DEVCB_TYPE_DEVICE, tag, (func), NULL, NULL }
#define DEVCB_DEVICE_HANDLER(tag,func)		{ DEVCB_TYPE_DEVICE, tag, NULL, (func), NULL }

/* read/write handlers for a given CPU's address space */
#define DEVCB_MEMORY_HANDLER(cpu,space,func) { DEVCB_TYPE_MEMORY(ADDRESS_SPACE_##space), (cpu), NULL, NULL, (func) }

/* read handlers for an I/O port by tag */
#define DEVCB_INPUT_PORT(tag)				{ DEVCB_TYPE_INPUT, (tag), NULL, NULL, NULL }

/* write handlers for a CPU input line */
#define DEVCB_CPU_INPUT_LINE(tag,line)		{ DEVCB_TYPE_CPU_LINE(line), (tag), NULL, NULL, NULL }


/* macros for defining read_line/write_line functions */
#define READ_LINE_DEVICE_HANDLER(name)		int  name(ATTR_UNUSED const device_config *device)
#define WRITE_LINE_DEVICE_HANDLER(name) 	void name(ATTR_UNUSED const device_config *device, ATTR_UNUSED int state)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* forward declarations */
class device_config;


/* read/write types for I/O lines (similar to read/write handlers but no offset) */
typedef int (*read_line_device_func)(const device_config *device);
typedef void (*write_line_device_func)(const device_config *device, int state);


/* static structure used for device configuration when the desired callback type is a read_line_device_func */
typedef struct _devcb_read_line devcb_read_line;
struct _devcb_read_line
{
	UINT32					type;			/* one of the special DEVCB_TYPE values */
	const char *			tag;			/* tag of target, where appropriate */
	read_line_device_func	readline;		/* read line function */
	read8_device_func		readdevice;		/* read device function */
	read8_space_func		readspace;		/* read space function */
};

typedef struct _devcb_resolved_read_line devcb_resolved_read_line;
struct _devcb_resolved_read_line
{
	const void *			target;			/* target object */
	read_line_device_func	read;			/* read line function */
	const void *			realtarget;		/* real target object for stubs */
	union
	{
		read8_device_func	readdevice;
		read8_space_func	readspace;
	} real;									/* real read function for stubs */
};


/* static structure used for device configuration when the desired callback type is a write_line_device_func */
typedef struct _devcb_write_line devcb_write_line;
struct _devcb_write_line
{
	UINT32					type;			/* one of the special DEVCB_TYPE values */
	const char *			tag;			/* tag of target, where appropriate */
	write_line_device_func	writeline;		/* write line function */
	write8_device_func		writedevice;	/* write device function */
	write8_space_func		writespace;		/* write space function */
};

typedef struct _devcb_resolved_write_line devcb_resolved_write_line;
struct _devcb_resolved_write_line
{
	const void *			target;			/* target object */
	write_line_device_func	write;			/* write line function */
	const void *			realtarget;		/* real target object for stubs */
	union
	{
		write8_device_func	writedevice;
		write8_space_func	writespace;
		int					writeline;
	} real;									/* real write function for stubs */
};


/* static structure used for device configuration when the desired callback type is a read8_device_func */
typedef struct _devcb_read8 devcb_read8;
struct _devcb_read8
{
	UINT32					type;			/* one of the special DEVCB_TYPE values */
	const char *			tag;			/* tag of target, where appropriate */
	read_line_device_func	readline;		/* read line function */
	read8_device_func		readdevice;		/* read device function */
	read8_space_func		readspace;		/* read space function */
};

typedef struct _devcb_resolved_read8 devcb_resolved_read8;
struct _devcb_resolved_read8
{
	const void *			target;			/* target object */
	read8_device_func		read;			/* read function */
	const void *			realtarget;		/* real target object for stubs */
	union
	{
		read8_device_func	readdevice;
		read8_space_func	readspace;
		read_line_device_func readline;
	} real;									/* real read function for stubs */
};


/* static structure used for device configuration when the desired callback type is a write8_device_func */
typedef struct _devcb_write8 devcb_write8;
struct _devcb_write8
{
	UINT32					type;			/* one of the special DEVCB_TYPE values */
	const char *			tag;			/* tag of target, where appropriate */
	write_line_device_func	writeline;		/* write line function */
	write8_device_func		writedevice;	/* write device function */
	write8_space_func		writespace;		/* write space function */
};

typedef struct _devcb_resolved_write8 devcb_resolved_write8;
struct _devcb_resolved_write8
{
	const void *			target;			/* target object */
	write8_device_func		write;			/* write function */
	const void *			realtarget;		/* real target object for stubs */
	union
	{
		write8_device_func	writedevice;
		write8_space_func	writespace;
		write_line_device_func writeline;
	} real;									/* real write function for stubs */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- static-to-live conversion ----- */

/* convert a static read line definition to a live definition */
void devcb_resolve_read_line(devcb_resolved_read_line *resolved, const devcb_read_line *config, const device_config *device);

/* convert a static write line definition to a live definition */
void devcb_resolve_write_line(devcb_resolved_write_line *resolved, const devcb_write_line *config, const device_config *device);

/* convert a static 8-bit read definition to a live definition */
void devcb_resolve_read8(devcb_resolved_read8 *resolved, const devcb_read8 *config, const device_config *device);

/* convert a static 8-bit write definition to a live definition */
void devcb_resolve_write8(devcb_resolved_write8 *resolved, const devcb_write8 *config, const device_config *device);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    devcb_call_read_line - call through a
    resolved read_line handler
-------------------------------------------------*/

INLINE int devcb_call_read_line(const devcb_resolved_read_line *resolved)
{
	return (resolved->read != NULL) ? (*resolved->read)((const device_config *)resolved->target) : 0;
}


/*-------------------------------------------------
    devcb_call_read8 - call through a
    resolved read8 handler
-------------------------------------------------*/

INLINE int devcb_call_read8(const devcb_resolved_read8 *resolved, offs_t offset)
{
	return (resolved->read != NULL) ? (*resolved->read)((const device_config *)resolved->target, offset) : 0;
}


/*-------------------------------------------------
    devcb_call_write_line - call through a
    resolved write_line handler
-------------------------------------------------*/

INLINE void devcb_call_write_line(const devcb_resolved_write_line *resolved, int state)
{
	if (resolved->write != NULL)
		(*resolved->write)((const device_config *)resolved->target, state);
}


/*-------------------------------------------------
    devcb_call_write8 - call through a
    resolved write8 handler
-------------------------------------------------*/

INLINE void devcb_call_write8(const devcb_resolved_write8 *resolved, offs_t offset, UINT8 data)
{
	if (resolved->write != NULL)
		(*resolved->write)((const device_config *)resolved->target, offset, data);
}

/*-------------------------------------------------
    devcb_line_gnd_r - input tied to GND
-------------------------------------------------*/

INLINE READ_LINE_DEVICE_HANDLER( devcb_line_gnd_r )
{
	return 0;
}

/*-------------------------------------------------
    devcb_line_vcc_r - input tied to Vcc
-------------------------------------------------*/

INLINE READ_LINE_DEVICE_HANDLER( devcb_line_vcc_r )
{
	return 1;
}

#endif	/* __DEVCB_H__ */
