/*********************************************************************

    filemngr.c

    MESS's clunky built-in file manager

    TODO
        - Support image creation arguments
        - Restrict directory listing by file extension
        - Support file manager invocation from the main menu for
          required images
        - Restrict empty slot if image required

*********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "emu.h"
#include "image.h"
#include "ui.h"
#include "uimenu.h"
#include "zippath.h"
#include "unicode.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* conditional compilation to enable chosing of image formats - this is not
 * yet fully implemented */
#define ENABLE_FORMATS			0

/* time (in seconds) to display errors */
#define ERROR_MESSAGE_TIME		5

/* itemrefs for key menu items */
#define ITEMREF_NEW_IMAGE_NAME	((void *) 0x0001)
#define ITEMREF_CREATE			((void *) 0x0002)
#define ITEMREF_FORMAT			((void *) 0x0003)
#define ITEMREF_NO				((void *) 0x0004)
#define ITEMREF_YES				((void *) 0x0005)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* menu item type in the file selector */
enum _file_selector_entry_type
{
	SELECTOR_ENTRY_TYPE_EMPTY,
	SELECTOR_ENTRY_TYPE_CREATE,
	SELECTOR_ENTRY_TYPE_SOFTWARE_LIST,
	SELECTOR_ENTRY_TYPE_DRIVE,
	SELECTOR_ENTRY_TYPE_DIRECTORY,
	SELECTOR_ENTRY_TYPE_FILE
};
typedef enum _file_selector_entry_type file_selector_entry_type;



/* an entry within the file manager */
typedef struct _file_selector_entry file_selector_entry;
struct _file_selector_entry
{
	file_selector_entry *next;

	file_selector_entry_type type;
	const char *basename;
	const char *fullpath;
};



/* state of the file manager */
typedef struct _file_manager_menu_state file_manager_menu_state;
struct _file_manager_menu_state
{
	device_image_interface *selected_device;
	astring *current_directory;
	astring *current_file;
};



/* state of the file selector menu */
typedef struct _file_selector_menu_state file_selector_menu_state;
struct _file_selector_menu_state
{
	file_manager_menu_state *manager_menustate;
	file_selector_entry *entrylist;
};



/* state of the file creator menu */
typedef struct _file_create_menu_state file_create_menu_state;
struct _file_create_menu_state
{
	file_manager_menu_state *manager_menustate;
	const image_device_format *current_format;
	int confirm_save_as_yes;
	char filename_buffer[1024];
};


/* state of the confirm save as menu */
typedef struct _confirm_save_as_menu_state confirm_save_as_menu_state;
struct _confirm_save_as_menu_state
{
	int *yes;
};

/***************************************************************************
    MENU HELPERS
***************************************************************************/

/*-------------------------------------------------
    input_character - inputs a typed character
    into a buffer
-------------------------------------------------*/

static void input_character(char *buffer, size_t buffer_length, unicode_char unichar, int (*filter)(unicode_char))
{
	size_t buflen = strlen(buffer);

	if ((unichar == 8) && (buflen > 0))
	{
		*(char *)utf8_previous_char(&buffer[buflen]) = 0;
	}
	else if ((unichar > ' ') && ((filter == NULL) || (*filter)(unichar)))
	{
		buflen += utf8_from_uchar(&buffer[buflen], buffer_length - buflen, unichar);
		buffer[buflen] = 0;
	}
}



/*-------------------------------------------------
    extra_text_draw_box - generically adds header
    or footer text
-------------------------------------------------*/

static void extra_text_draw_box(float origx1, float origx2, float origy, float yspan, const char *text, int direction)
{
	float text_width, text_height;
	float width, maxwidth;
	float x1, y1, x2, y2, temp;

	/* get the size of the text */
	ui_draw_text_full( render_container_get_ui(),text, 0.0f, 0.0f, 1.0f, JUSTIFY_LEFT, WRAP_WORD,
		DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &text_width, &text_height);
	width = text_width + (2 * UI_BOX_LR_BORDER);
	maxwidth = MAX(width, origx2 - origx1);

	/* compute our bounds */
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy + (yspan * direction);
	y2 = origy + (UI_BOX_TB_BORDER * direction);

	if (y1 > y2)
	{
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	/* draw a box */
	ui_draw_outlined_box(render_container_get_ui(),x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	/* take off the borders */
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	/* draw the text within it */
	ui_draw_text_full(render_container_get_ui(),text, x1, y1, text_width, JUSTIFY_LEFT, WRAP_WORD,
					  DRAW_NORMAL, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
}



/*-------------------------------------------------
    extra_text_render - generically adds header
    and footer text
-------------------------------------------------*/

static void extra_text_render(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom,
	float origx1, float origy1, float origx2, float origy2,
	const char *header, const char *footer)
{
	header = ((header != NULL) && (header[0] != '\0')) ? header : NULL;
	footer = ((footer != NULL) && (footer[0] != '\0')) ? footer : NULL;

	if (header != NULL)
		extra_text_draw_box(origx1, origx2, origy1, top, header, -1);
	if (footer != NULL)
		extra_text_draw_box(origx1, origx2, origy2, bottom, footer, +1);
}



/***************************************************************************
    CONFIRM SAVE AS MENU
***************************************************************************/

/*-------------------------------------------------
    menu_confirm_save_as_populate - populates the
    confirm save as menu
-------------------------------------------------*/

static void menu_confirm_save_as_populate(running_machine *machine, ui_menu *menu, void *state)
{
	ui_menu_item_append(menu, "File Already Exists - Overide?", NULL, MENU_FLAG_DISABLE, NULL);
	ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, MENU_FLAG_DISABLE, NULL);
	ui_menu_item_append(menu, "No", NULL, 0, ITEMREF_NO);
	ui_menu_item_append(menu, "Yes", NULL, 0, ITEMREF_YES);
}



/*-------------------------------------------------
    menu_confirm_save_as - confirm save as menu
-------------------------------------------------*/

static void menu_confirm_save_as(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;
	confirm_save_as_menu_state *menustate = (confirm_save_as_menu_state *) state;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_confirm_save_as_populate(machine, menu, state);

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);

	/* process the event */
	if ((event != NULL) && (event->iptkey == IPT_UI_SELECT))
	{
		if (event->itemref == ITEMREF_YES)
			*menustate->yes = TRUE;

		/* no matter what, pop out */
		ui_menu_stack_pop(machine);
	}
}



/***************************************************************************
    FILE CREATE MENU
***************************************************************************/

/*-------------------------------------------------
    is_valid_filename_char - tests to see if a
    character is valid in a filename
-------------------------------------------------*/

static int is_valid_filename_char(unicode_char unichar)
{
	/* this should really be in the OSD layer */
	static const char valid_filename_char[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 	/* 00-0f */
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 	/* 10-1f */
		1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 	/*  !"#$%&'()*+,-./ */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 	/* 0123456789:;<=>? */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 	/* @ABCDEFGHIJKLMNO */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 	/* PQRSTUVWXYZ[\]^_ */
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 	/* `abcdefghijklmno */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 	/* pqrstuvwxyz{|}~  */
	};
	return (unichar < ARRAY_LENGTH(valid_filename_char)) && valid_filename_char[unichar];
}



/*-------------------------------------------------
    file_create_render_extra - perform our
    special rendering
-------------------------------------------------*/

static void file_create_render_extra(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	file_create_menu_state *menustate = (file_create_menu_state *) state;

	extra_text_render(machine, menu, state, selectedref, top, bottom, origx1, origy1, origx2, origy2,
		astring_c(menustate->manager_menustate->current_directory),
		NULL);
}



/*-------------------------------------------------
    menu_file_create_populate - populates the file
    creator menu
-------------------------------------------------*/

static void menu_file_create_populate(running_machine *machine, ui_menu *menu, void *state, void *selection)
{
	astring buffer;
	file_create_menu_state *menustate = (file_create_menu_state *) state;
	device_image_interface *device = menustate->manager_menustate->selected_device;
	const image_device_format *format;
	const char *new_image_name;

	/* append the "New Image Name" item */
	if (selection == ITEMREF_NEW_IMAGE_NAME)
	{
		astring_assemble_2(&buffer, menustate->filename_buffer, "_");
		new_image_name = astring_c(&buffer);
	}
	else
	{
		new_image_name = menustate->filename_buffer;
	}
	ui_menu_item_append(menu, "New Image Name:", new_image_name, 0, ITEMREF_NEW_IMAGE_NAME);

	/* do we support multiple formats? */
	format = device->device_get_creatable_formats();
	if (ENABLE_FORMATS && (format != NULL))
	{
		ui_menu_item_append(menu, "Image Format:", menustate->current_format->m_description, 0, ITEMREF_FORMAT);
		menustate->current_format = format;
	}

	/* finish up the menu */
	ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	ui_menu_item_append(menu, "Create", NULL, 0, ITEMREF_CREATE);

	/* set up custom render proc */
	ui_menu_set_custom_render(menu, file_create_render_extra, ui_get_line_height() + 3.0f * UI_BOX_TB_BORDER, 0);
}



/*-------------------------------------------------
    create_new_image - creates a new disk image
-------------------------------------------------*/

static int create_new_image(device_image_interface *image, const char *directory, const char *filename, int *yes)
{
	astring *path;
	osd_directory_entry *entry;
	osd_dir_entry_type file_type;
	int do_create, err;
	int result = FALSE;
	ui_menu *child_menu;
	confirm_save_as_menu_state *child_menustate;

	/* assemble the full path */
	path = zippath_combine(astring_alloc(), directory, filename);

	/* does a file or a directory exist at the path */
	entry = osd_stat(astring_c(path));
	file_type = (entry != NULL) ? entry->type : ENTTYPE_NONE;
	if (entry != NULL)
		free(entry);

	/* special case */
	if ((file_type == ENTTYPE_FILE) && *yes)
		file_type = ENTTYPE_NONE;

	switch(file_type)
	{
		case ENTTYPE_NONE:
			/* no file/dir here - always create */
			do_create = TRUE;
			break;

		case ENTTYPE_FILE:
			/* a file exists here - ask for permission from the user */
			child_menu = ui_menu_alloc(image->device().machine, render_container_get_ui(), menu_confirm_save_as, NULL);
			child_menustate = (confirm_save_as_menu_state*)ui_menu_alloc_state(child_menu, sizeof(*child_menustate), NULL);
			child_menustate->yes = yes;
			ui_menu_stack_push(child_menu);
			do_create = FALSE;
			break;

		case ENTTYPE_DIR:
			/* a directory exists here - we can't save over it */
			ui_popup_time(ERROR_MESSAGE_TIME, "Cannot save over directory");
			do_create = FALSE;
			break;

		default:
			fatalerror("Unexpected");
			do_create = FALSE;
			break;
	}

	/* create the image, if appropriate */
	if (do_create)
	{
		err = image->create(astring_c(path), 0, NULL);
		if (err != 0)
			popmessage("Error: %s", image->error());
		else
			result = TRUE;
	}

	/* free the path */
	astring_free(path);

	return result;
}



/*-------------------------------------------------
    menu_file_create - file creator menu
-------------------------------------------------*/

static void menu_file_create(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	void *selection;
	const ui_menu_event *event;
	ui_menu_event fake_event;
	file_create_menu_state *menustate = (file_create_menu_state *) state;

	/* identify the selection */
	selection = ui_menu_get_selection(menu);

	/* rebuild the menu */
	ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_POSITION);
	menu_file_create_populate(machine, menu, state, selection);

	if (menustate->confirm_save_as_yes)
	{
		/* we just returned from a "confirm save as" dialog and the user said "yes" - fake an event */
		memset(&fake_event, 0, sizeof(fake_event));
		fake_event.iptkey = IPT_UI_SELECT;
		fake_event.itemref = ITEMREF_CREATE;
		event = &fake_event;
	}
	else
	{
		/* process the menu */
		event = ui_menu_process(machine, menu, 0);
	}

	/* process the event */
	if (event != NULL)
	{
		/* handle selections */
		switch(event->iptkey)
		{
			case IPT_UI_SELECT:
				if ((event->itemref == ITEMREF_CREATE) || (event->itemref == ITEMREF_NEW_IMAGE_NAME))
				{
					if (create_new_image(
						menustate->manager_menustate->selected_device,
						astring_c(menustate->manager_menustate->current_directory),
						menustate->filename_buffer,
						&menustate->confirm_save_as_yes))
					{
						/* success - pop out twice to device view */
						ui_menu_stack_pop(machine);
						ui_menu_stack_pop(machine);
					}
				}
				break;

			case IPT_SPECIAL:
				if (ui_menu_get_selection(menu) == ITEMREF_NEW_IMAGE_NAME)
				{
					input_character(
						menustate->filename_buffer,
						ARRAY_LENGTH(menustate->filename_buffer),
						event->unichar,
						is_valid_filename_char);
				}
				break;
		}
	}
}



/***************************************************************************
    FILE SELECTOR MENU
***************************************************************************/

/*-------------------------------------------------
    file_selector_render_extra - perform our
    special rendering
-------------------------------------------------*/

static void file_selector_render_extra(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	file_selector_menu_state *menustate = (file_selector_menu_state *) state;

	extra_text_render(machine, menu, state, selectedref, top, bottom,
		origx1, origy1, origx2, origy2,
		astring_c(menustate->manager_menustate->current_directory),
		NULL);
}



/*-------------------------------------------------
    compare_file_selector_entries - sorting proc
    for file selector entries
-------------------------------------------------*/

static int compare_file_selector_entries(const file_selector_entry *e1, const file_selector_entry *e2)
{
	int result;
	const char *e1_basename = (e1->basename != NULL) ? e1->basename : "";
	const char *e2_basename = (e2->basename != NULL) ? e2->basename : "";

	if (e1->type < e2->type)
	{
		result = -1;
	}
	else if (e1->type > e2->type)
	{
		result = 1;
	}
	else
	{
		result = mame_stricmp(e1_basename, e2_basename);
		if (result == 0)
		{
			result = strcmp(e1_basename, e2_basename);
			if (result == 0)
			{
				if (e1 < e2)
					result = -1;
				else if (e1 > e2)
					result = 1;
			}
		}
	}

	return result;
}



/*-------------------------------------------------
    append_file_selector_entry - appends a new
    file selector entry to an entry list
-------------------------------------------------*/

static file_selector_entry *append_file_selector_entry(ui_menu *menu, file_selector_menu_state *menustate,
	file_selector_entry_type entry_type, const char *entry_basename, const char *entry_fullpath)
{
	file_selector_entry *entry;
	file_selector_entry **entryptr;

	/* allocate a new entry */
	entry = (file_selector_entry *) ui_menu_pool_alloc(menu, sizeof(*entry));
	memset(entry, 0, sizeof(*entry));
	entry->type = entry_type;
	entry->basename = (entry_basename != NULL) ? ui_menu_pool_strdup(menu, entry_basename) : entry_basename;
	entry->fullpath = (entry_fullpath != NULL) ? ui_menu_pool_strdup(menu, entry_fullpath) : entry_fullpath;

	/* find the end of the list */
	entryptr = &menustate->entrylist;
	while ((*entryptr != NULL) && (compare_file_selector_entries(entry, *entryptr) >= 0))
		entryptr = &(*entryptr)->next;

	/* insert the entry */
	entry->next = *entryptr;
	*entryptr = entry;
	return entry;
}



/*-------------------------------------------------
    append_file_selector_entry_menu_item - appends
    a menu item for a file selector entry
-------------------------------------------------*/

static file_selector_entry *append_dirent_file_selector_entry(ui_menu *menu, file_selector_menu_state *menustate,
	const osd_directory_entry *dirent)
{
	astring *buffer;
	file_selector_entry_type entry_type;
	file_selector_entry *entry;

	switch(dirent->type)
	{
		case ENTTYPE_FILE:
			entry_type = SELECTOR_ENTRY_TYPE_FILE;
			break;

		case ENTTYPE_DIR:
			entry_type = SELECTOR_ENTRY_TYPE_DIRECTORY;
			break;

		default:
			/* exceptional case; do not add a menu item */
			return NULL;
	}

	/* determine the full path */
	buffer = zippath_combine(
		astring_alloc(),
		astring_c(menustate->manager_menustate->current_directory),
		dirent->name);

	/* create the file selector entry */
	entry = append_file_selector_entry(
		menu,
		menustate,
		entry_type,
		dirent->name,
		astring_c(buffer));

	astring_free(buffer);
	return entry;
}



/*-------------------------------------------------
    append_file_selector_entry_menu_item - appends
    a menu item for a file selector entry
-------------------------------------------------*/

static void append_file_selector_entry_menu_item(ui_menu *menu, const file_selector_entry *entry)
{
	const char *text = NULL;
	const char *subtext = NULL;

	switch(entry->type)
	{
		case SELECTOR_ENTRY_TYPE_EMPTY:
			text = "[empty slot]";
			break;

		case SELECTOR_ENTRY_TYPE_CREATE:
			text = "[create]";
			break;

		case SELECTOR_ENTRY_TYPE_SOFTWARE_LIST:
			text = "[software list]";
			break;

		case SELECTOR_ENTRY_TYPE_DRIVE:
			text = entry->basename;
			subtext = "[DRIVE]";
			break;

		case SELECTOR_ENTRY_TYPE_DIRECTORY:
			text = entry->basename;
			subtext = "[DIR]";
			break;

		case SELECTOR_ENTRY_TYPE_FILE:
			text = entry->basename;
			subtext = "[FILE]";
			break;
	}
	ui_menu_item_append(menu, text, subtext, 0, (void *) entry);
}



/*-------------------------------------------------
    menu_file_selector_populate - creates and
    allocates all menu items for a directory
-------------------------------------------------*/

static file_error menu_file_selector_populate(running_machine *machine, ui_menu *menu, file_selector_menu_state *menustate)
{
	zippath_directory *directory = NULL;
	file_error err = FILERR_NONE;
	const osd_directory_entry *dirent;
	const file_selector_entry *entry;
	const file_selector_entry *selected_entry = NULL;
	int i;
	const char *volume_name;
	device_image_interface *device = menustate->manager_menustate->selected_device;
	const char *path = astring_c(menustate->manager_menustate->current_directory);

	/* open the directory */
	err = zippath_opendir(path, &directory);
	if (err != FILERR_NONE)
		goto done;

	/* clear out the menu entries */
	menustate->entrylist = NULL;

	/* add the "[empty slot]" entry */
	append_file_selector_entry(menu, menustate, SELECTOR_ENTRY_TYPE_EMPTY, NULL, NULL);

	if (device->image_config().is_creatable() && !zippath_is_zip(directory))
	{
		/* add the "[create]" entry */
		append_file_selector_entry(menu, menustate, SELECTOR_ENTRY_TYPE_CREATE, NULL, NULL);
	}

	/* add the "[software list]" entry */
	append_file_selector_entry(menu, menustate, SELECTOR_ENTRY_TYPE_SOFTWARE_LIST, NULL, NULL);

	/* add the drives */
	i = 0;
	while((volume_name = osd_get_volume_name(i))!=NULL)
	{
		append_file_selector_entry(menu, menustate, SELECTOR_ENTRY_TYPE_DRIVE,
			volume_name, volume_name);
		i++;
	}

	/* build the menu for each item */
	while((dirent = zippath_readdir(directory)) != NULL)
	{
		/* append a dirent entry */
		entry = append_dirent_file_selector_entry(menu, menustate, dirent);

		if (entry != NULL)
		{
			/* set the selected item to be the first non-parent directory or file */
			if ((selected_entry == NULL) && strcmp(dirent->name, ".."))
				selected_entry = entry;

			/* do we have to select this file? */
			if (!mame_stricmp(astring_c(menustate->manager_menustate->current_file), dirent->name))
				selected_entry = entry;
		}
	}

	/* append all of the menu entries */
	for (entry = menustate->entrylist; entry != NULL; entry = entry->next)
		append_file_selector_entry_menu_item(menu, entry);

	/* set the selection (if we have one) */
	if (selected_entry != NULL)
		ui_menu_set_selection(menu, (void *) selected_entry);

	/* set up custom render proc */
	ui_menu_set_custom_render(menu, file_selector_render_extra, ui_get_line_height() + 3.0f * UI_BOX_TB_BORDER, 0);

done:
	if (directory != NULL)
		zippath_closedir(directory);
	return err;
}



/*-------------------------------------------------
    check_path - performs a quick check to see if
    a path exists
-------------------------------------------------*/

static file_error check_path(const char *path)
{
	return zippath_opendir(path, NULL);
}



/*-------------------------------------------------
    menu_file_selector - file selector menu
-------------------------------------------------*/

static void menu_file_selector(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	file_error err;
	const ui_menu_event *event;
	ui_menu *child_menu;
	file_selector_menu_state *menustate;
	file_create_menu_state *child_menustate;
	const file_selector_entry *entry;

	/* get menu state */
	menustate = (file_selector_menu_state *) state;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
	{
		err = menu_file_selector_populate(machine, menu, menustate);

		/* pop out if there was an error */
		if (err != FILERR_NONE)
		{
			ui_menu_stack_pop(machine);
			return;
		}
	}

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);
	if (event != NULL && event->itemref != NULL)
	{
		/* handle selections */
		if (event->iptkey == IPT_UI_SELECT)
		{
			entry = (const file_selector_entry *) event->itemref;
			switch(entry->type)
			{
				case SELECTOR_ENTRY_TYPE_EMPTY:
					/* empty slot - unload */
					menustate->manager_menustate->selected_device->unload();
					ui_menu_stack_pop(machine);
					break;

				case SELECTOR_ENTRY_TYPE_CREATE:
					/* create */
					child_menu = ui_menu_alloc(machine, render_container_get_ui(), menu_file_create, NULL);
					child_menustate = (file_create_menu_state*)ui_menu_alloc_state(child_menu, sizeof(*child_menustate), NULL);
					child_menustate->manager_menustate = menustate->manager_menustate;
					ui_menu_stack_push(child_menu);
					break;
				case SELECTOR_ENTRY_TYPE_SOFTWARE_LIST:
					child_menu = ui_menu_alloc(machine, render_container_get_ui(), ui_image_menu_software, menustate->manager_menustate->selected_device);
					ui_menu_stack_push(child_menu);
					break;
				case SELECTOR_ENTRY_TYPE_DRIVE:
				case SELECTOR_ENTRY_TYPE_DIRECTORY:
					/* drive/directory - first check the path */
					err = check_path(entry->fullpath);
					if (err != FILERR_NONE)
					{
						/* this path is problematic; present the user with an error and bail */
						ui_popup_time(1, "Error accessing %s", entry->fullpath);
						break;
					}
					astring_cpyc(menustate->manager_menustate->current_directory, entry->fullpath);
					ui_menu_reset(menu, (ui_menu_reset_options)0);
					break;

				case SELECTOR_ENTRY_TYPE_FILE:
					/* file */
					menustate->manager_menustate->selected_device->load(entry->fullpath);
					ui_menu_stack_pop(machine);
					break;
			}
		}
	}
}



/***************************************************************************
    FILE MANAGER
***************************************************************************/

/*-------------------------------------------------
    fix_working_directory - checks the working
    directory for this device to ensure that it
    "makes sense"
-------------------------------------------------*/

static void fix_working_directory(device_image_interface *image)
{
	/* if the image exists, set the working directory to the parent directory */
	if (image->exists())
	{
		astring *astr = astring_alloc();
		zippath_parent(astr, image->filename());
		image->set_working_directory(astring_c(astr));
		astring_free(astr);
	}

	/* check to see if the path exists; if not clear it */
	if (check_path(image->working_directory()) != FILERR_NONE)
		image->set_working_directory("");
}



/*-------------------------------------------------
    file_manager_render_extra - perform our
    special rendering
-------------------------------------------------*/

static void file_manager_render_extra(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	file_manager_menu_state *menustate = (file_manager_menu_state *) state;
	const char *path;

	/* access the path */
	path = (menustate->selected_device != NULL) ? menustate->selected_device->filename() : NULL;
	extra_text_render(machine, menu, state, selectedref, top, bottom,
		origx1, origy1, origx2, origy2, NULL, path);
}



/*-------------------------------------------------
    menu_file_manager_populate - populates the main
    file manager menu
-------------------------------------------------*/

static void menu_file_manager_populate(running_machine *machine, ui_menu *menu, void *state)
{
	char buffer[2048];
	device_image_interface *image = NULL;
	const char *entry_basename;

	/* cycle through all devices for this system */
	for (bool gotone = machine->m_devicelist.first(image); gotone; gotone = image->next(image))
	{
		/* get the image type/id */
		snprintf(buffer, ARRAY_LENGTH(buffer),
			"%s",
			image->image_config().devconfig().name());

		/* get the base name */
		entry_basename = image->basename();

		/* record the menu item */
		ui_menu_item_append(menu, buffer, (entry_basename != NULL) ? entry_basename : "---", 0, (void *) image);
	}

	/* set up custom render proc */
	ui_menu_set_custom_render(menu, file_manager_render_extra, 0, ui_get_line_height() + 3.0f * UI_BOX_TB_BORDER);
}



/*-------------------------------------------------
    file_manager_destroy_state - state destructor
-------------------------------------------------*/

static void file_manager_destroy_state(ui_menu *menu, void *state)
{
	file_manager_menu_state *menustate = (file_manager_menu_state *) state;

	if (menustate->current_directory != NULL)
		astring_free(menustate->current_directory);

	if (menustate->current_file != NULL)
		astring_free(menustate->current_file);
}



/*-------------------------------------------------
    menu_file_manager - main file manager menu
-------------------------------------------------*/

void ui_image_menu_file_manager(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;
	file_manager_menu_state *menustate;
	ui_menu *child_menu;
	file_selector_menu_state *child_menustate;

	/* if no state, allocate now */
	if (state == NULL)
	{
		state = ui_menu_alloc_state(menu, sizeof(*menustate), file_manager_destroy_state);
		menustate = (file_manager_menu_state *) state;

		menustate->current_directory = astring_alloc();
		menustate->current_file = astring_alloc();
	}
	menustate = (file_manager_menu_state *) state;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_file_manager_populate(machine, menu, state);

	/* update the selected device */
	menustate->selected_device = (device_image_interface *) ui_menu_get_selection(menu);

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);
	if (event != NULL && event->iptkey == IPT_UI_SELECT)
	{
		menustate->selected_device = (device_image_interface *) event->itemref;
		if (menustate->selected_device != NULL)
		{
			/* ensure that the working directory for this device exists */
			fix_working_directory(menustate->selected_device);

			/* set up current_directory and current_file - depends on whether we have an image */
			astring_cpyc(menustate->current_directory, menustate->selected_device->working_directory());
			astring_cpyc(menustate->current_file, menustate->selected_device->exists() ? menustate->selected_device->basename() : "");

			/* reset the existing menu */
			ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_POSITION);

			/* push the menu */
			child_menu = ui_menu_alloc(machine, render_container_get_ui(), menu_file_selector, NULL);
			child_menustate = (file_selector_menu_state *)ui_menu_alloc_state(child_menu, sizeof(*child_menustate), NULL);
			child_menustate->manager_menustate = menustate;
			ui_menu_stack_push(child_menu);
		}
	}
}

/*-------------------------------------------------
    ui_image_menu_image_info - menu that shows info
    on all loaded images
-------------------------------------------------*/

void ui_image_menu_image_info(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
	{
		astring *tempstring = image_info_astring(machine, astring_alloc());
		ui_menu_item_append(menu, astring_c(tempstring), NULL, MENU_FLAG_MULTILINE, NULL);
		astring_free(tempstring);
	}

	/* process the menu */
	ui_menu_process(machine, menu, 0);
}
