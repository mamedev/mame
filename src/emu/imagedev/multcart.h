/*********************************************************************

    multcart.h

    Multi-cartridge handling code

*********************************************************************/

#ifndef __MULTCART_H__
#define __MULTCART_H__

#include "osdcore.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum _multicart_load_flags
{
	MULTICART_FLAGS_DONT_LOAD_RESOURCES	= 0x00,
	MULTICART_FLAGS_LOAD_RESOURCES		= 0x01
};
typedef enum _multicart_load_flags multicart_load_flags;

enum _multicart_resource_type
{
	MULTICART_RESOURCE_TYPE_INVALID,
	MULTICART_RESOURCE_TYPE_ROM,
	MULTICART_RESOURCE_TYPE_RAM
};
typedef enum _multicart_resource_type multicart_resource_type;

typedef struct _multicart_resource multicart_resource;
struct _multicart_resource
{
	const char *				id;
	const char *				filename;
	multicart_resource *		next;
	multicart_resource_type		type;
	UINT32						length;
	void *						ptr;
};


typedef struct _multicart_socket multicart_socket;
struct _multicart_socket
{
	const char *				id;
	multicart_socket *			next;
	const multicart_resource *	resource;
	void *						ptr;
};


typedef struct _multicart_private multicart_private;

typedef struct _multicart_t multicart_t;
struct _multicart_t
{
	const multicart_resource *	resources;
	const multicart_socket *	sockets;
	const char *				pcb_type;
	const char *				gamedrv_name; /* need this to find the path to the nvram files */
	multicart_private *			data;
};


enum _multicart_open_error
{
	MCERR_NONE,
	MCERR_NOT_MULTICART,
	MCERR_CORRUPT,
	MCERR_OUT_OF_MEMORY,
	MCERR_XML_ERROR,
	MCERR_INVALID_FILE_REF,
	MCERR_ZIP_ERROR,
	MCERR_MISSING_RAM_LENGTH,
	MCERR_INVALID_RAM_SPEC,
	MCERR_UNKNOWN_RESOURCE_TYPE,
	MCERR_INVALID_RESOURCE_REF,
	MCERR_INVALID_FILE_FORMAT,
	MCERR_MISSING_LAYOUT,
	MCERR_NO_PCB_OR_RESOURCES
};
typedef enum _multicart_open_error multicart_open_error;

const char *multicart_error_text(multicart_open_error error);

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* opens a multicart */
multicart_open_error multicart_open(const char *filename, const char *drvname, multicart_load_flags load_flags, multicart_t **cart);

/* closes a multicart */
void multicart_close(multicart_t *cart);

#endif /* __MULTCART_H__ */
