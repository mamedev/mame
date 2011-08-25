/*********************************************************************

    snapquik.h

    Snapshots and quickloads

*********************************************************************/

#include "emu.h"
#include "snapquik.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _snapquick_token snapquick_token;
struct _snapquick_token
{
	emu_timer *timer;
	snapquick_load_func load;
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    assert_is_snapshot_or_quickload - asserts/confirms
    that a given device is a snapshot or quickload
-------------------------------------------------*/

INLINE void assert_is_snapshot_or_quickload(device_t *device)
{
	assert(device != NULL);
	assert(downcast<const legacy_device_base *>(device)->inline_config() != NULL);
	assert((device->type() == SNAPSHOT) || (device->type() == QUICKLOAD)
		|| (device->type() == Z80BIN));
}



/*-------------------------------------------------
    get_token - safely gets the snapshot/quickload data
-------------------------------------------------*/

INLINE snapquick_token *get_token(device_t *device)
{
	assert_is_snapshot_or_quickload(device);
	return (snapquick_token *) downcast<legacy_device_base *>(device)->token();
}



/*-------------------------------------------------
    get_config - safely gets the quickload config
-------------------------------------------------*/

INLINE const snapquick_config *get_config(device_t *device)
{
	assert_is_snapshot_or_quickload(device);
	return (const snapquick_config *) downcast<const legacy_device_base *>(device)->inline_config();
}

INLINE const snapquick_config *get_config_dev(const device_t *device)
{
	assert(device != NULL);
	assert((device->type() == SNAPSHOT) || (device->type() == QUICKLOAD)
		|| (device->type() == Z80BIN));
	return (const snapquick_config *) downcast<const legacy_device_base *>(device)->inline_config();
}

/*-------------------------------------------------
    log_quickload - logs and displays useful
    data for the end user
-------------------------------------------------*/

void log_quickload(const char *type, UINT32 start, UINT32 length, UINT32 exec, const char *exec_format)
{
    astring tempstring;

    logerror("Loading %04X bytes of RAM at %04X\n", length, start);

    tempstring.catprintf("Quickload type: %s   Length: %d bytes\n", type, length);
    tempstring.catprintf("Start: 0x%04X   End: 0x%04X   Exec: ", start, start + length - 1);

    logerror("Quickload loaded.\n");
    if (!mame_stricmp(exec_format, EXEC_NA))
        tempstring.cat("N/A");
    else
    {
        logerror("Execution can resume with ");
        logerror(exec_format, exec);
        logerror("\n");
        tempstring.catprintf(exec_format, exec);
    }

    ui_popup_time(10, "%s", tempstring.cstr());
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    TIMER_CALLBACK(process_snapshot_or_quickload)
-------------------------------------------------*/

static TIMER_CALLBACK(process_snapshot_or_quickload)
{
	device_image_interface *image = (device_image_interface *) ptr;
	snapquick_token *token = get_token(&image->device());

	/* invoke the load */
	(*token->load)(*image,
		image->filetype(),
		image->length());

	/* unload the device */
	image->unload();
}



/*-------------------------------------------------
    DEVICE_START( snapquick )
-------------------------------------------------*/

static DEVICE_START( snapquick )
{
	snapquick_token *token = get_token(device);

	/* allocate a timer */
	token->timer = device->machine().scheduler().timer_alloc(FUNC(process_snapshot_or_quickload), (void *) dynamic_cast<device_image_interface *>(device));

}



/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( snapquick )
-------------------------------------------------*/

static DEVICE_IMAGE_LOAD( snapquick )
{
	const snapquick_config *config = get_config(image);
	snapquick_token *token = get_token(image);

	/* locate the load function */
	token->load = (snapquick_load_func) reinterpret_cast<snapquick_load_func>(image.get_device_specific_call());

	/* adjust the timer */

		token->timer->adjust(
		attotime(config->delay_seconds, config->delay_attoseconds),
		0);

	return IMAGE_INIT_PASS;
}



/*-------------------------------------------------
    DEVICE_GET_INFO(snapquick) - device getinfo
    function
-------------------------------------------------*/

static DEVICE_GET_INFO(snapquick)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(snapquick_token); break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = sizeof(snapquick_config); break;
		case DEVINFO_INT_IMAGE_READABLE:				info->i = 1; break;
		case DEVINFO_INT_IMAGE_WRITEABLE:				info->i = 0; break;
		case DEVINFO_INT_IMAGE_CREATABLE:				info->i = 0; break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(snapquick); break;
		case DEVINFO_FCT_IMAGE_LOAD:					info->f = (genf *) DEVICE_IMAGE_LOAD_NAME(snapquick); break;
		case DEVINFO_FCT_SNAPSHOT_QUICKLOAD_LOAD:		info->f = (genf *) get_config_dev(device)->load; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_IMAGE_FILE_EXTENSIONS:			strcpy(info->s, get_config_dev(device)->file_extensions); break;
	}
}



/*-------------------------------------------------
    DEVICE_GET_INFO(snapshot) - device getinfo
    function
-------------------------------------------------*/

DEVICE_GET_INFO(snapshot)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_IMAGE_TYPE:					info->i = IO_SNAPSHOT; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Snapshot"); break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Snapshot"); break;

		default: DEVICE_GET_INFO_CALL(snapquick); break;
	}
}



/*-------------------------------------------------
    DEVICE_GET_INFO(quickload) - device getinfo
    function
-------------------------------------------------*/

DEVICE_GET_INFO(quickload)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_IMAGE_TYPE:					info->i = IO_QUICKLOAD; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Quickload"); break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Quickload"); break;

		default: DEVICE_GET_INFO_CALL(snapquick); break;
	}
}
/*-------------------------------------------------
    z80bin_load_file - load a z80bin file into
    memory
-------------------------------------------------*/

static int z80bin_load_file(device_image_interface *image, const char *file_type, UINT16 *exec_addr, UINT16 *start_addr, UINT16 *end_addr )
{
	int ch;
	UINT16 args[3];
	UINT16 i=0, j, size;
	UINT8 data;
	char pgmname[256];
	char message[256];

	image->fseek(7, SEEK_SET);

	while((ch = image->fgetc()) != 0x1A)
	{
		if (ch == EOF)
		{
			image->seterror(IMAGE_ERROR_INVALIDIMAGE, "Unexpected EOF while getting file name");
			image->message(" Unexpected EOF while getting file name");
			return IMAGE_INIT_FAIL;
		}

		if (ch != '\0')
		{
			if (i >= (ARRAY_LENGTH(pgmname) - 1))
			{
				image->seterror(IMAGE_ERROR_INVALIDIMAGE, "File name too long");
				image->message(" File name too long");
				return IMAGE_INIT_FAIL;
			}

			pgmname[i] = ch;	/* build program name */
			i++;
		}
	}

	pgmname[i] = '\0';	/* terminate string with a null */

	if (image->fread(args, sizeof(args)) != sizeof(args))
	{
		image->seterror(IMAGE_ERROR_INVALIDIMAGE, "Unexpected EOF while getting file size");
		image->message(" Unexpected EOF while getting file size");
		return IMAGE_INIT_FAIL;
	}

	exec_addr[0] = LITTLE_ENDIANIZE_INT16(args[0]);
	start_addr[0] = LITTLE_ENDIANIZE_INT16(args[1]);
	end_addr[0]	= LITTLE_ENDIANIZE_INT16(args[2]);

	size = (end_addr[0] - start_addr[0] + 1) & 0xffff;

	/* display a message about the loaded quickload */
	image->message(" %s\nsize=%04X : start=%04X : end=%04X : exec=%04X",pgmname,size,start_addr[0],end_addr[0],exec_addr[0]);

	for (i = 0; i < size; i++)
	{
		j = (start_addr[0] + i) & 0xffff;
		if (image->fread(&data, 1) != 1)
		{
			snprintf(message, ARRAY_LENGTH(message), "%s: Unexpected EOF while writing byte to %04X", pgmname, (unsigned) j);
			image->seterror(IMAGE_ERROR_INVALIDIMAGE, message);
			image->message("%s: Unexpected EOF while writing byte to %04X", pgmname, (unsigned) j);
			return IMAGE_INIT_FAIL;
		}
		image->device().machine().device("maincpu")->memory().space(AS_PROGRAM)->write_byte(j, data);
	}

	return IMAGE_INIT_PASS;
}



/*-------------------------------------------------
    QUICKLOAD_LOAD( z80bin )
-------------------------------------------------*/

static QUICKLOAD_LOAD( z80bin )
{
	const z80bin_config *config;
	UINT16 exec_addr, start_addr, end_addr;
	int autorun;

	/* load the binary into memory */
	if (z80bin_load_file(&image, file_type, &exec_addr, &start_addr, &end_addr) == IMAGE_INIT_FAIL)
		return IMAGE_INIT_FAIL;

	/* is this file executable? */
	if (exec_addr != 0xffff)
	{
		config = (const z80bin_config *)downcast<const legacy_device_base &>(image.device()).inline_config();

		/* check to see if autorun is on (I hate how this works) */
		autorun = input_port_read_safe(image.device().machine(), "CONFIG", 0xFF) & 1;

		/* start program */
		if (config->execute != NULL)
		{
			(*config->execute)(image.device().machine(), start_addr, end_addr, exec_addr, autorun);
		}
		else
		{
			if (autorun)
				cpu_set_reg(image.device().machine().device("maincpu"), STATE_GENPC, exec_addr);
		}
	}

	return IMAGE_INIT_PASS;
}



/*-------------------------------------------------
    DEVICE_GET_INFO(z80bin)
-------------------------------------------------*/

DEVICE_GET_INFO(z80bin)
{
	/* quickload */
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = sizeof(z80bin_config); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_IMAGE_FILE_EXTENSIONS:			strcpy(info->s, "bin"); break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case DEVINFO_FCT_SNAPSHOT_QUICKLOAD_LOAD:		info->f = (genf *) quickload_load_z80bin; break;

		default:	DEVICE_GET_INFO_CALL(quickload); break;
	}
}

DEFINE_LEGACY_IMAGE_DEVICE(Z80BIN, z80bin);
DEFINE_LEGACY_IMAGE_DEVICE(SNAPSHOT, snapshot);
DEFINE_LEGACY_IMAGE_DEVICE(QUICKLOAD, quickload);
