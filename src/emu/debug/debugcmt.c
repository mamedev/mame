/***************************************************************************

    debugcmt.c

    Debugger code-comment management functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Debugger comment file format:

    (from comment struct - assuming (MAX_COMMENT_LINE_LENGTH == 128))
    0           - valid byte
    1:4         - address
    5:133       - comment
    134:138     - color
    139:142     - instruction crc

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "xmlfile.h"
#include "debugcmt.h"
#include "debugcpu.h"
#include "debugvw.h"
#include "info.h"
#include <zlib.h>



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define COMMENT_VERSION	(1)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _debug_comment debug_comment;
struct _debug_comment
{
	UINT8			is_valid;
	UINT32			address;
	char			text[DEBUG_COMMENT_MAX_LINE_LENGTH];
	rgb_t			color;
	UINT32			crc;
};


class debug_cpu_comment_group
{
public:
	int 			comment_count;
	UINT32			change_count;
	debug_comment *	comment_info[DEBUG_COMMENT_MAX_NUM];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int debug_comment_load_xml(running_machine *machine, mame_file *file);
static void debug_comment_exit(running_machine &machine);



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------------------------------
    debug_comment_init - initializes the comment memory and
                         loads any existing comment file
-------------------------------------------------------------------------*/

int debug_comment_init(running_machine *machine)
{
	/* allocate memory for the comments */
	device_disasm_interface *disasm;
	for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
		if (device->interface(disasm))
			device->debug()->m_comments = auto_alloc_clear(machine, debug_cpu_comment_group);

	/* automatically load em up */
	debug_comment_load(machine);
	machine->add_notifier(MACHINE_NOTIFY_EXIT, debug_comment_exit);
	return 1;
}


/*-------------------------------------------------------------------------
    debug_comment_add - adds a comment to the list at the given address.
                        use debug_comment_get_opcode_crc32(addr) to get
                        the proper crc32
-------------------------------------------------------------------------*/

int debug_comment_add(device_t *device, offs_t addr, const char *comment, rgb_t color, UINT32 c_crc)
{
	debug_cpu_comment_group *comments = device->debug()->m_comments;
	int insert_point = comments->comment_count;
	int match = 0;
	int i = 0;

	/* Create a new item to insert into the list */
	debug_comment *insert_me = auto_alloc(device->machine, debug_comment);
	insert_me->color = color;
	insert_me->is_valid = 1;
	insert_me->address = addr;
	insert_me->crc = c_crc;
	strcpy(insert_me->text, comment);

	/* Find the insert point */
	for (i = 0; i < comments->comment_count; i++)
	{
		if (insert_me->address < comments->comment_info[i]->address)
		{
			insert_point = i;
			break;
		}
		else if (insert_me->address == comments->comment_info[i]->address &&
				 insert_me->crc == comments->comment_info[i]->crc)
		{
			insert_point = i;
			match = 1;
			break;
		}
	}

	/* Got an exact match?  Just replace */
	if (match == 1)
	{
		auto_free(device->machine, comments->comment_info[insert_point]);
		comments->comment_info[insert_point] = insert_me;
		comments->change_count++;

		/* force an update of disassembly views */
		device->machine->m_debug_view->update_all(DVT_DISASSEMBLY);
		return 1;
	}

	/* Otherwise insert */
	/* First, shift the list down */
	for (i = comments->comment_count; i >= insert_point; i--)
		comments->comment_info[i] = comments->comment_info[i-1];

	/* do the insertion */
	comments->comment_info[insert_point] = insert_me;
	comments->comment_count++;
	comments->change_count++;

	/* force an update of disassembly views */
	device->machine->m_debug_view->update_all(DVT_DISASSEMBLY);

	return 1;
}


/*-------------------------------------------------------------------------
    debug_comment_remove - removes a comment at a given address
                        use debug_comment_get_opcode_crc32(addr) to get
                        the proper crc32
-------------------------------------------------------------------------*/

int debug_comment_remove(device_t *device, offs_t addr, UINT32 c_crc)
{
	debug_cpu_comment_group *comments = device->debug()->m_comments;
	int remove_index = -1;
	int i;

	for (i = 0; i < comments->comment_count; i++)
		if (comments->comment_info[i]->address == addr)	/* got an address match */
			if (comments->comment_info[i]->crc == c_crc)
				remove_index = i;

	/* The comment doesn't exist? */
	if (remove_index == -1)
		return 0;

	/* Okay, it's there, now remove it */
	auto_free(device->machine, comments->comment_info[remove_index]);

	for (i = remove_index; i < comments->comment_count-1; i++)
		comments->comment_info[i] = comments->comment_info[i+1];

	comments->comment_count--;
	comments->change_count++;

	/* force an update of disassembly views */
	device->machine->m_debug_view->update_all(DVT_DISASSEMBLY);

	return 1;
}


/*-------------------------------------------------------------------------
    debug_comment_get_text - returns the comment for a given addresses
                        use debug_comment_get_opcode_crc32(addr) to get
                        the proper crc32
-------------------------------------------------------------------------*/

const char *debug_comment_get_text(device_t *device, offs_t addr, UINT32 c_crc)
{
	debug_cpu_comment_group *comments = device->debug()->m_comments;
	int i;

	/* inefficient - should use bsearch - but will be a little tricky with multiple comments per addr */
	for (i = 0; i < comments->comment_count; i++)
		if (comments->comment_info[i]->address == addr)	/* got an address match */
		{
			/* now check the bank information to be sure */
			if (comments->comment_info[i]->crc == c_crc)
				return comments->comment_info[i]->text;
		}

	return 0x00;
}


/*-------------------------------------------------------------------------
    debug_comment_get_count - returns the number of comments
    for a given cpu number
-------------------------------------------------------------------------*/

int debug_comment_get_count(device_t *device)
{
	debug_cpu_comment_group *comments = device->debug()->m_comments;
	return comments->comment_count;
}


/*-------------------------------------------------------------------------
    debug_comment_get_change_count - returns the change counter
    for a given cpu number
-------------------------------------------------------------------------*/

UINT32 debug_comment_get_change_count(device_t *device)
{
	debug_cpu_comment_group *comments = device->debug()->m_comments;
	return comments->change_count;
}

/*-------------------------------------------------------------------------
    debug_comment_all_change_count - returns the change counter
    for all cpu's
-------------------------------------------------------------------------*/

UINT32 debug_comment_all_change_count(running_machine *machine)
{
	UINT32 retVal = 0;

	for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
		if (device->debug()->m_comments != NULL)
			retVal += device->debug()->m_comments->change_count;

	return retVal;
}

/*-------------------------------------------------------------------------
    debug_comment_get_opcode_crc32 - magic function that takes all the
                        current state of the debugger and returns a crc32
                        for the opcode at the requested address.
-------------------------------------------------------------------------*/

UINT32 debug_comment_get_opcode_crc32(device_t *device, offs_t address)
{
	const address_space *space = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	int i;
	UINT32 crc;
	UINT8 opbuf[64], argbuf[64];
	char buff[256];
	offs_t numbytes;
	cpu_device *cpudevice = downcast<cpu_device *>(device);
	int maxbytes = cpudevice->max_opcode_bytes();
	UINT32 addrmask = space->logaddrmask;

	memset(opbuf, 0x00, sizeof(opbuf));
	memset(argbuf, 0x00, sizeof(argbuf));

	// fetch the bytes up to the maximum
	for (i = 0; i < maxbytes; i++)
	{
		opbuf[i] = debug_read_opcode(space, address + i, 1, FALSE);
		argbuf[i] = debug_read_opcode(space, address + i, 1, TRUE);
	}

	numbytes = device->debug()->disassemble(buff, address & addrmask, opbuf, argbuf) & DASMFLAG_LENGTHMASK;
	numbytes = memory_address_to_byte(space, numbytes);

	crc = crc32(0, argbuf, numbytes);

	return crc;
}


/*-------------------------------------------------------------------------
    debug_comment_dump - debugging function to dump junk to the command line
-------------------------------------------------------------------------*/

void debug_comment_dump(device_t *device, offs_t addr)
{
	debug_cpu_comment_group *comments = device->debug()->m_comments;
	int i;
	int ff = 0;

	if (addr == -1)
	{
		for (i = 0; i < comments->comment_count; i++)
			if (comments->comment_info[i]->is_valid)
				logerror("%d : %s (%d %d)\n", i, comments->comment_info[i]->text,
												 comments->comment_info[i]->address,
												 comments->comment_info[i]->crc);
	}
	else
	{
		UINT32 c_crc = debug_comment_get_opcode_crc32(device, addr);

		for (i = 0; i < comments->comment_count; i++)
			if (comments->comment_info[i]->address == addr)	/* got an address match */
			{
				/* now check the bank information to be sure */
				if (comments->comment_info[i]->crc == c_crc)
				{
					logerror("%d : %s (%d %d)\n", addr,
												  comments->comment_info[addr]->text,
												  comments->comment_info[addr]->address,
												  comments->comment_info[addr]->crc);
					ff = 1;
				}
			}

		if (!ff) logerror("No comment exists for address : 0x%x\n", addr);
	}
}


/*-------------------------------------------------------------------------
    debug_comment_save - comment file saving
-------------------------------------------------------------------------*/

int debug_comment_save(running_machine *machine)
{
	int j;
	char crc_buf[20];
	xml_data_node *root = xml_file_create();
	xml_data_node *commentnode, *systemnode;
	int total_comments = 0;

	/* if we don't have a root, bail */
	if (root == NULL)
		return 0;

	/* create a comment node */
	commentnode = xml_add_child(root, "mamecommentfile", NULL);
	if (commentnode == NULL)
		goto error;
	xml_set_attribute_int(commentnode, "version", COMMENT_VERSION);

	/* create a system node */
	systemnode = xml_add_child(commentnode, "system", NULL);
	if (systemnode == NULL)
		goto error;
	xml_set_attribute(systemnode, "name", machine->gamedrv->name);

	/* for each cpu */
	for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
	{
		debug_cpu_comment_group *comments = device->debug()->m_comments;
		if (comments != NULL)
		{
			xml_data_node *curnode = xml_add_child(systemnode, "cpu", NULL);
			if (curnode == NULL)
				goto error;
			xml_set_attribute(curnode, "tag", device->tag());

			for (j = 0; j < comments->comment_count; j++)
			{
				xml_data_node *datanode = xml_add_child(curnode, "comment", xml_normalize_string(comments->comment_info[j]->text));
				if (datanode == NULL)
					goto error;
				xml_set_attribute_int(datanode, "address", comments->comment_info[j]->address);
				xml_set_attribute_int(datanode, "color", comments->comment_info[j]->color);
				sprintf(crc_buf, "%08X", comments->comment_info[j]->crc);
				xml_set_attribute(datanode, "crc", crc_buf);
				total_comments++;
			}
		}
	}

	/* flush the file */
	if (total_comments > 0)
	{
		file_error filerr;
		mame_file *fp;

		astring fname(machine->basename(), ".cmt");
		filerr = mame_fopen(SEARCHPATH_COMMENT, fname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &fp);

		if (filerr == FILERR_NONE)
		{
			xml_file_write(root, mame_core_file(fp));
			mame_fclose(fp);
		}
	}

	/* free and get out of here */
	xml_file_free(root);
	return 1;

error:
	xml_file_free(root);
	return 0;
}


/*-------------------------------------------------------------------------
    debug_comment_load(_xml) - comment file loading
-------------------------------------------------------------------------*/

int debug_comment_load(running_machine *machine)
{
	file_error filerr;
	mame_file *fp;

	astring fname(machine->basename(), ".cmt");
	filerr = mame_fopen(SEARCHPATH_COMMENT, fname, OPEN_FLAG_READ, &fp);

	if (filerr != FILERR_NONE) return 0;
	debug_comment_load_xml(machine, fp);
	mame_fclose(fp);

	return 1;
}

static int debug_comment_load_xml(running_machine *machine, mame_file *fp)
{
	int j;
	xml_data_node *root, *commentnode, *systemnode, *cpunode, *datanode;
	const char *name;
	int version;

	/* read the file */
	root = xml_file_read(mame_core_file(fp), NULL);
	if (root == NULL)
		goto error;

	/* find the config node */
	commentnode = xml_get_sibling(root->child, "mamecommentfile");
	if (commentnode == NULL)
		goto error;

	/* validate the config data version */
	version = xml_get_attribute_int(commentnode, "version", 0);
	if (version != COMMENT_VERSION)
		goto error;

	/* check to make sure the file is applicable */
	systemnode = xml_get_sibling(commentnode->child, "system");
	name = xml_get_attribute_string(systemnode, "name", "");
	if (strcmp(name, machine->gamedrv->name) != 0)
		goto error;

	for (cpunode = xml_get_sibling(systemnode->child, "cpu"); cpunode; cpunode = xml_get_sibling(cpunode->next, "cpu"))
	{
		device_t *cpu = machine->device(xml_get_attribute_string(cpunode, "tag", ""));
		if (cpu != NULL)
		{
			debug_cpu_comment_group *comments = cpu->debug()->m_comments;
			j = 0;

			for (datanode = xml_get_sibling(cpunode->child, "comment"); datanode; datanode = xml_get_sibling(datanode->next, "comment"))
			{
				/* Malloc the comment */
				comments->comment_info[j] = auto_alloc(machine, debug_comment);

				comments->comment_info[j]->address = xml_get_attribute_int(datanode, "address", 0);
				comments->comment_info[j]->color = xml_get_attribute_int(datanode, "color", 0);
				sscanf(xml_get_attribute_string(datanode, "crc", 0), "%08X", &comments->comment_info[j]->crc);
				strcpy(comments->comment_info[j]->text, datanode->value);
				comments->comment_info[j]->is_valid = 1;

				j++;
			}

			comments->comment_count = j;
		}
	}

	/* free the parser */
	xml_file_free(root);

	return 1;

error:
	if (root)
		xml_file_free(root);
	return 0;
}


/*-------------------------------------------------------------------------
    debug_comment_exit - saves the comments and frees memory
-------------------------------------------------------------------------*/

static void debug_comment_exit(running_machine &machine)
{
	debug_comment_save(&machine);
}
