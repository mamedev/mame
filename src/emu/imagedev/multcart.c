/*********************************************************************

    multcart.c

    Multi-cartridge handling code

*********************************************************************/

#include "emu.h"
#include "multcart.h"
#include "pool.h"
#include "unzip.h"
#include "corestr.h"
#include "xmlfile.h"
#include "hash.h"
#include "machine/ram.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _multicart_private
{
	object_pool *	pool;
	zip_file *		zip;
};


typedef struct _multicart_load_state multicart_load_state;
struct _multicart_load_state
{
	multicart_t *			multicart;
	zip_file *			zip;
	xml_data_node *			layout_xml;
	xml_data_node *			resources_node;
	xml_data_node *			pcb_node;
	multicart_resource *		resources;
	multicart_socket *		sockets;
};

static const char error_text[14][30] =
{
	"no error",
	"not a multicart",
	"module definition corrupt",
	"out of memory",
	"xml format error",
	"invalid file reference",
	"zip file error",
	"missing ram length",
	"invalid ram specification",
	"unknown resource type",
	"invalid resource reference",
	"invalid file format",
	"missing layout",
	"no pcb or resource found"
};

const char *multicart_error_text(multicart_open_error error)
{
	return error_text[(int)error];
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    find_file
-------------------------------------------------*/

static const zip_file_header *find_file(zip_file *zip, const char *filename)
{
	const zip_file_header *header;
	for (header = zip_file_first_file(zip); header != NULL; header = zip_file_next_file(zip))
	{
		if (!core_stricmp(header->filename, filename))
			return header;
	}
	return NULL;
}

static const zip_file_header *find_file_crc(zip_file *zip, const char *filename, UINT32 crc)
{
	const zip_file_header *header;
	for (header = zip_file_first_file(zip); header != NULL; header = zip_file_next_file(zip))
	{
		// if the CRC and name both match, we're good
		// if the CRC matches and the name doesn't, we're still good
		if ((!core_stricmp(header->filename, filename)) && (header->crc == crc))
		{
			return header;
		}
		else if (header->crc == crc)
		{
			return header;
		}
	}
	return NULL;
}

/*-------------------------------------------------
    find_pcb_and_resource_nodes
-------------------------------------------------*/

static int find_pcb_and_resource_nodes(xml_data_node *layout_xml,
	xml_data_node **pcb_node, xml_data_node **resource_node)
{
	xml_data_node *romset_node;
	xml_data_node *configuration_node;

	*pcb_node = NULL;
	*resource_node = NULL;

	romset_node = xml_get_sibling(layout_xml->child, "romset");
	if (romset_node == NULL)
		return FALSE;

	configuration_node = xml_get_sibling(romset_node->child, "configuration");
	if (configuration_node == NULL)
		return FALSE;

	*pcb_node = xml_get_sibling(configuration_node->child, "pcb");
	if (*pcb_node == NULL)
		return FALSE;

	*resource_node = xml_get_sibling(romset_node->child, "resources");
	if (*resource_node == NULL)
		return FALSE;

	return TRUE;
}


/*-------------------------------------------------
    get_resource_type
-------------------------------------------------*/

static multicart_resource_type get_resource_type(const char *s)
{
	multicart_resource_type result;
	if (!strcmp(s, "rom"))
		result = MULTICART_RESOURCE_TYPE_ROM;
	else if (!strcmp(s, "ram"))
		result = MULTICART_RESOURCE_TYPE_RAM;
	else
		result = MULTICART_RESOURCE_TYPE_INVALID;
	return result;
}


/*-------------------------------------------------
    load_rom_resource
-------------------------------------------------*/

static multicart_open_error load_rom_resource(multicart_load_state *state, xml_data_node *resource_node,
	multicart_resource *resource)
{
	const char *file, *crcstr, *sha1;
	const zip_file_header *header;
	zip_error ziperr;
	UINT32 crc;

	/* locate the 'file' attribute */
	file = xml_get_attribute_string(resource_node, "file", NULL);
	if (file == NULL)
		return MCERR_XML_ERROR;

	if (!(crcstr = xml_get_attribute_string(resource_node, "crc", NULL)))
	{
		/* locate the file in the ZIP file */
		header = find_file(state->zip, file);
	}
	else	/* CRC tag is present, use it */
	{
		crc = strtoul(crcstr, NULL, 16);
		header = find_file_crc(state->zip, file, crc);
	}
	if (header == NULL)
		return MCERR_INVALID_FILE_REF;
	resource->length = header->uncompressed_length;

	/* allocate bytes for this resource */
	resource->ptr = pool_malloc_lib(state->multicart->data->pool, resource->length);
	if (resource->ptr == NULL)
		return MCERR_OUT_OF_MEMORY;

	/* and decompress it */
	ziperr = zip_file_decompress(state->zip, resource->ptr, resource->length);
	if (ziperr != ZIPERR_NONE)
		return MCERR_ZIP_ERROR;

	/* check SHA1 now */
	if ((sha1 = xml_get_attribute_string(resource_node, "sha1", NULL)))
	{
		hash_collection actual_hashes;
		actual_hashes.compute((const UINT8 *)resource->ptr, resource->length, hash_collection::HASH_TYPES_CRC_SHA1);

		hash_collection expected_hashes;
		expected_hashes.add_from_string(hash_collection::HASH_SHA1, sha1, strlen(sha1));

		if (actual_hashes != expected_hashes)
		{
			return MCERR_INVALID_FILE_REF;
		}
	}

	return MCERR_NONE;
}


/*-------------------------------------------------
    load_ram_resource
-------------------------------------------------*/

static multicart_open_error load_ram_resource(core_options &options, multicart_load_state *state, xml_data_node *resource_node,
	multicart_resource *resource)
{
	const char *length_string;
	const char *ram_type;
	const char *ram_filename;

	astring *ram_pathname;

	/* locate the 'length' attribute */
	length_string = xml_get_attribute_string(resource_node, "length", NULL);
	if (length_string == NULL)
		return MCERR_MISSING_RAM_LENGTH;

	/* ...and parse it */
	resource->length = ram_parse_string(length_string);
	if (resource->length <= 0)
		return MCERR_INVALID_RAM_SPEC;

	/* allocate bytes for this resource */
	resource->ptr = pool_malloc_lib(state->multicart->data->pool, resource->length);
	if (resource->ptr == NULL)
		return MCERR_OUT_OF_MEMORY;

	/* Is this a persistent RAM resource? Then try to load it. */
	ram_type = xml_get_attribute_string(resource_node, "type", NULL);
	if (ram_type != NULL)
	{
		if (strcmp(ram_type, "persistent")==0)
		{
			astring tmp;

			/* Get the file name. */
			ram_filename = xml_get_attribute_string(resource_node, "file", NULL);
			if (ram_filename==NULL)
				return MCERR_XML_ERROR;

			ram_pathname = astring_assemble_3(&tmp, state->multicart->gamedrv_name, PATH_SEPARATOR, ram_filename);

			/* Save the file name so that we can write the contents on unloading.
               If the RAM resource has no filename, we know that it was volatile only. */
			resource->filename = pool_strdup_lib(state->multicart->data->pool, astring_c(ram_pathname));

			if (resource->filename == NULL)
				return MCERR_OUT_OF_MEMORY;

			image_battery_load_by_name(options, resource->filename, resource->ptr, resource->length, 0x00);
		}
		/* else this type is volatile, in which case we just have
            a memory expansion */
	}
	return MCERR_NONE;
}


/*-------------------------------------------------
    load_resource
-------------------------------------------------*/

static multicart_open_error load_resource(core_options &options, multicart_load_state *state, xml_data_node *resource_node,
	multicart_resource_type resource_type)
{
	const char *id;
	multicart_open_error err;
	multicart_resource *resource;
	multicart_resource **next_resource;

	/* get the 'id' attribute; error if not present */
	id = xml_get_attribute_string(resource_node, "id", NULL);
	if (id == NULL)
		return MCERR_XML_ERROR;

	/* allocate memory for the resource */
	resource = (multicart_resource *)pool_malloc_lib(state->multicart->data->pool, sizeof(*resource));
	if (resource == NULL)
		return MCERR_OUT_OF_MEMORY;
	memset(resource, 0, sizeof(*resource));
	resource->type = resource_type;

	/* copy id */
	resource->id = pool_strdup_lib(state->multicart->data->pool, id);
	if (resource->id == NULL)
		return MCERR_OUT_OF_MEMORY;

	switch(resource->type)
	{
		case MULTICART_RESOURCE_TYPE_ROM:
			err = load_rom_resource(state, resource_node, resource);
			if (err != MCERR_NONE)
				return err;
			break;

		case MULTICART_RESOURCE_TYPE_RAM:
			err = load_ram_resource(options, state, resource_node, resource);
			if (err != MCERR_NONE)
				return err;
			break;

		default:
			return MCERR_UNKNOWN_RESOURCE_TYPE;
	}

	/* append the resource */
	for (next_resource = &state->resources; *next_resource; next_resource = &(*next_resource)->next)
		;
	*next_resource = resource;

	return MCERR_NONE;
}


/*-------------------------------------------------
    load_all_resources
-------------------------------------------------*/

static multicart_open_error load_all_resources(core_options &options, multicart_load_state *state)
{
	multicart_open_error err;
	xml_data_node *resource_node;
	multicart_resource_type resource_type;

	for (resource_node = state->resources_node->child; resource_node != NULL; resource_node = resource_node->next)
	{
		resource_type = get_resource_type(resource_node->name);
		if (resource_type != MULTICART_RESOURCE_TYPE_INVALID)
		{
			err = load_resource(options, state, resource_node, resource_type);
			if (err != MCERR_NONE)
				return err;
		}
	}

	state->multicart->resources = state->resources;
	return MCERR_NONE;
}

/*-------------------------------------------------
    save_ram_resources. This is important for persistent RAM. All
    resources were allocated within the memory pool of this device and will
    be freed on multicart_close.
-------------------------------------------------*/

static multicart_open_error save_ram_resources(core_options &options, multicart_t *cart)
{
	const multicart_resource *resource;

	for (resource = cart->resources; resource != NULL; resource = resource->next)
	{
		if ((resource->type == MULTICART_RESOURCE_TYPE_RAM) && (resource->filename != NULL))
		{
			image_battery_save_by_name(options, resource->filename, resource->ptr, resource->length);
		}
	}
	return MCERR_NONE;
}

/*-------------------------------------------------
    load_socket
-------------------------------------------------*/

static multicart_open_error load_socket(multicart_load_state *state, xml_data_node *socket_node)
{
	const char *id;
	const char *uses;
	const multicart_resource *resource;
	multicart_socket *socket;
	multicart_socket **next_socket;

	/* get the 'id' and 'uses' attributes; error if not present */
	id = xml_get_attribute_string(socket_node, "id", NULL);
	uses = xml_get_attribute_string(socket_node, "uses", NULL);
	if ((id == NULL) || (uses == NULL))
		return MCERR_XML_ERROR;

	/* find the resource */
	for (resource = state->multicart->resources; resource != NULL; resource = resource->next)
	{
		if (!strcmp(uses, resource->id))
			break;
	}
	if (resource == NULL)
		return MCERR_INVALID_RESOURCE_REF;

	/* create the socket */
	socket = (multicart_socket *)pool_malloc_lib(state->multicart->data->pool, sizeof(*socket));
	if (socket == NULL)
		return MCERR_OUT_OF_MEMORY;
	memset(socket, 0, sizeof(*socket));
	socket->resource = resource;
	socket->ptr = resource->ptr;

	/* copy id */
	socket->id = pool_strdup_lib(state->multicart->data->pool, id);
	if (socket->id == NULL)
		return MCERR_OUT_OF_MEMORY;

	/* which pointer should I use? */
	if (resource->ptr != NULL)
	{
		/* use the resource's ptr */
		socket->ptr = resource->ptr;
	}
	else
	{
		/* allocate bytes for this socket */
		socket->ptr = pool_malloc_lib(state->multicart->data->pool, resource->length);
		if (socket->ptr == NULL)
			return MCERR_OUT_OF_MEMORY;

		/* ...and clear it */
		memset(socket->ptr, 0xCD, resource->length);
	}

	/* append the resource */
	for (next_socket = &state->sockets; *next_socket; next_socket = &(*next_socket)->next)
		;
	*next_socket = socket;

	return MCERR_NONE;
}


/*-------------------------------------------------
    load_all_sockets
-------------------------------------------------*/

static multicart_open_error load_all_sockets(multicart_load_state *state)
{
	multicart_open_error err;
	xml_data_node *socket_node;

	for (socket_node = xml_get_sibling(state->pcb_node->child, "socket"); socket_node != NULL;
		socket_node = xml_get_sibling(socket_node->next, "socket"))
	{
		err = load_socket(state, socket_node);
		if (err != MCERR_NONE)
			return err;
	}

	state->multicart->sockets = state->sockets;
	return MCERR_NONE;
}


/*-------------------------------------------------
    multicart_open - opens a multicart
-------------------------------------------------*/

multicart_open_error multicart_open(core_options &options, const char *filename, const char *gamedrv, multicart_load_flags load_flags, multicart_t **cart)
{
	multicart_open_error err;
	zip_error ziperr;
	object_pool *pool;
	multicart_load_state state = {0, };
	const zip_file_header *header;
	const char *pcb_type;
	char *layout_text = NULL;

	/* allocate an object pool */
	pool = pool_alloc_lib(NULL);
	if (pool == NULL)
	{
		err = MCERR_OUT_OF_MEMORY;
		goto done;
	}

	/* allocate the multicart */
	state.multicart = (multicart_t*)pool_malloc_lib(pool, sizeof(*state.multicart));
	if (state.multicart == NULL)
	{
		err = MCERR_OUT_OF_MEMORY;
		goto done;
	}
	memset(state.multicart, 0, sizeof(*state.multicart));

	/* allocate the multicart's private data */
	state.multicart->data = (multicart_private*)pool_malloc_lib(pool, sizeof(*state.multicart->data));
	if (state.multicart->data == NULL)
	{
		err = MCERR_OUT_OF_MEMORY;
		goto done;
	}
	memset(state.multicart->data, 0, sizeof(*state.multicart->data));
	state.multicart->data->pool = pool;
	pool = NULL;

	/* open the ZIP file */
	ziperr = zip_file_open(filename, &state.zip);
	if (ziperr != ZIPERR_NONE)
	{
		err = MCERR_NOT_MULTICART;
		goto done;
	}

	/* find the layout.xml file */
	header = find_file(state.zip, "layout.xml");
	if (header == NULL)
	{
		err = MCERR_MISSING_LAYOUT;
		goto done;
	}

	/* reserve space for the layout text */
	layout_text = (char*)malloc(header->uncompressed_length + 1);
	if (layout_text == NULL)
	{
		err = MCERR_OUT_OF_MEMORY;
		goto done;
	}

	/* uncompress the layout text */
	ziperr = zip_file_decompress(state.zip, layout_text, header->uncompressed_length);
	if (ziperr != ZIPERR_NONE)
	{
		err = MCERR_ZIP_ERROR;
		goto done;
	}
	layout_text[header->uncompressed_length] = '\0';

	/* parse the layout text */
	state.layout_xml = xml_string_read(layout_text, NULL);
	if (state.layout_xml == NULL)
	{
		err = MCERR_XML_ERROR;
		goto done;
	}

	/* locate the PCB node */
	if (!find_pcb_and_resource_nodes(state.layout_xml, &state.pcb_node, &state.resources_node))
	{
		err = MCERR_NO_PCB_OR_RESOURCES;
		goto done;
	}

	/* get the PCB resource_type */
	pcb_type = xml_get_attribute_string(state.pcb_node, "type", "");
	state.multicart->pcb_type = pool_strdup_lib(state.multicart->data->pool, pcb_type);
	if (state.multicart->pcb_type == NULL)
	{
		err = MCERR_OUT_OF_MEMORY;
		goto done;
	}

	state.multicart->gamedrv_name = pool_strdup_lib(state.multicart->data->pool, gamedrv);
	if (state.multicart->gamedrv_name == NULL)
	{
		err = MCERR_OUT_OF_MEMORY;
		goto done;
	}

	/* do we have to load resources? */
	if (load_flags & MULTICART_FLAGS_LOAD_RESOURCES)
	{
		err = load_all_resources(options, &state);
		if (err != MCERR_NONE)
			goto done;

		err = load_all_sockets(&state);
		if (err != MCERR_NONE)
			goto done;
	}

	err = MCERR_NONE;

done:
	if (pool != NULL)
		pool_free_lib(pool);
	if (state.zip != NULL)
		zip_file_close(state.zip);
	if (layout_text != NULL)
		free(layout_text);
	if (state.layout_xml != NULL)
		xml_file_free(state.layout_xml);

	if ((err != MCERR_NONE) && (state.multicart != NULL))
	{
		multicart_close(options, state.multicart);
		state.multicart = NULL;
	}
	*cart = state.multicart;
	return err;
}


/*-------------------------------------------------
    multicart_close - closes a multicart
-------------------------------------------------*/

void multicart_close(core_options &options, multicart_t *cart)
{
	save_ram_resources(options, cart);
	pool_free_lib(cart->data->pool);
}
