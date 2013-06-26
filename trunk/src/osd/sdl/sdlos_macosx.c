//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard sdl header
#include <sys/stat.h>
#include <unistd.h>

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <Carbon/Carbon.h>

#include "sdlinc.h"

// MAME headers
#include "osdcore.h"

//============================================================
//  PROTOTYPES
//============================================================

static osd_ticks_t init_cycle_counter(void);
static osd_ticks_t mach_cycle_counter(void);

//============================================================
//  STATIC VARIABLES
//============================================================

static osd_ticks_t      (*cycle_counter)(void) = init_cycle_counter;
static osd_ticks_t      (*ticks_counter)(void) = init_cycle_counter;
static osd_ticks_t      ticks_per_second;

//============================================================
//  init_cycle_counter
//
//  to avoid total grossness, this function is split by subarch
//============================================================

static osd_ticks_t init_cycle_counter(void)
{
	osd_ticks_t start, end;
	osd_ticks_t a, b;

	cycle_counter = mach_cycle_counter;
	ticks_counter = mach_cycle_counter;

	// wait for an edge on the timeGetTime call
	a = SDL_GetTicks();
	do
	{
		b = SDL_GetTicks();
	} while (a == b);

	// get the starting cycle count
	start = (*cycle_counter)();

	// now wait for 1/4 second total
	do
	{
		a = SDL_GetTicks();
	} while (a - b < 250);

	// get the ending cycle count
	end = (*cycle_counter)();

	// compute ticks_per_sec
	ticks_per_second = (end - start) * 4;

	// return the current cycle count
	return (*cycle_counter)();
}

//============================================================
//  performance_cycle_counter
//============================================================

//============================================================
//  mach_cycle_counter
//============================================================
static osd_ticks_t mach_cycle_counter(void)
{
	return mach_absolute_time();
}

//============================================================
//   osd_cycles
//============================================================

osd_ticks_t osd_ticks(void)
{
	return (*cycle_counter)();
}


//============================================================
//  osd_ticks_per_second
//============================================================

osd_ticks_t osd_ticks_per_second(void)
{
	if (ticks_per_second == 0)
	{
		return 1;   // this isn't correct, but it prevents the crash
	}
	return ticks_per_second;
}



//============================================================
//  osd_sleep
//============================================================

void osd_sleep(osd_ticks_t duration)
{
	UINT32 msec;

	// make sure we've computed ticks_per_second
	if (ticks_per_second == 0)
		(void)osd_ticks();

	// convert to milliseconds, rounding down
	msec = (UINT32)(duration * 1000 / ticks_per_second);

	// only sleep if at least 2 full milliseconds
	if (msec >= 2)
	{
		// take a couple of msecs off the top for good measure
		msec -= 2;
		usleep(msec*1000);
	}
}

//============================================================
//  osd_num_processors
//============================================================

int osd_get_num_processors(void)
{
	int processors = 1;

	struct host_basic_info host_basic_info;
	unsigned int count;
	kern_return_t r;
	mach_port_t my_mach_host_self;

	count = HOST_BASIC_INFO_COUNT;
	my_mach_host_self = mach_host_self();
	if ( ( r = host_info(my_mach_host_self, HOST_BASIC_INFO, (host_info_t)(&host_basic_info), &count)) == KERN_SUCCESS )
	{
		processors = host_basic_info.avail_cpus;
	}
	mach_port_deallocate(mach_task_self(), my_mach_host_self);

	return processors;
}

//============================================================
//  osd_malloc
//============================================================

void *osd_malloc(size_t size)
{
#ifndef MALLOC_DEBUG
	return malloc(size);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}


//============================================================
//  osd_malloc_array
//============================================================

void *osd_malloc_array(size_t size)
{
#ifndef MALLOC_DEBUG
	return malloc(size);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}


//============================================================
//  osd_free
//============================================================

void osd_free(void *ptr)
{
#ifndef MALLOC_DEBUG
	free(ptr);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}

//============================================================
//  osd_getenv
//============================================================

char *osd_getenv(const char *name)
{
	return getenv(name);
}

//============================================================
//  osd_setenv
//============================================================

int osd_setenv(const char *name, const char *value, int overwrite)
{
	return setenv(name, value, overwrite);
}

//============================================================
//  osd_get_clipboard_text
//============================================================

char *osd_get_clipboard_text(void)
{
	char *result = NULL; /* core expects a malloced C string of uft8 data */

	PasteboardRef pasteboard_ref;
	OSStatus err;
	PasteboardSyncFlags sync_flags;
	PasteboardItemID item_id;
	CFIndex flavor_count;
	CFArrayRef flavor_type_array;
	CFIndex flavor_index;
	ItemCount item_count;
	UInt32 item_index;
	Boolean success = false;

	err = PasteboardCreate(kPasteboardClipboard, &pasteboard_ref);

	if (!err)
	{
		sync_flags = PasteboardSynchronize( pasteboard_ref );

		err = PasteboardGetItemCount(pasteboard_ref, &item_count );

		for (item_index=1; item_index<=item_count; item_index++)
		{
			err = PasteboardGetItemIdentifier(pasteboard_ref, item_index, &item_id);

			if (!err)
			{
				err = PasteboardCopyItemFlavors(pasteboard_ref, item_id, &flavor_type_array);

				if (!err)
				{
					flavor_count = CFArrayGetCount(flavor_type_array);

					for (flavor_index = 0; flavor_index < flavor_count; flavor_index++)
					{
						CFStringRef flavor_type;
						CFDataRef flavor_data;
						CFStringEncoding encoding;
						CFStringRef string_ref;
						CFDataRef data_ref;
						CFIndex length;
						CFRange range;

						flavor_type = (CFStringRef)CFArrayGetValueAtIndex(flavor_type_array, flavor_index);

						if (UTTypeConformsTo (flavor_type, kUTTypeUTF16PlainText))
							encoding = kCFStringEncodingUTF16;
						else if (UTTypeConformsTo (flavor_type, kUTTypeUTF8PlainText))
							encoding = kCFStringEncodingUTF8;
						else if (UTTypeConformsTo (flavor_type, kUTTypePlainText))
							encoding = kCFStringEncodingMacRoman;
						else
							continue;

						err = PasteboardCopyItemFlavorData(pasteboard_ref, item_id, flavor_type, &flavor_data);

						if( !err )
						{
							string_ref = CFStringCreateFromExternalRepresentation (kCFAllocatorDefault, flavor_data, encoding);
							data_ref = CFStringCreateExternalRepresentation (kCFAllocatorDefault, string_ref, kCFStringEncodingUTF8, '?');

							length = CFDataGetLength (data_ref);
							range = CFRangeMake (0,length);

							result = (char *)osd_malloc_array (length+1);
							if (result != NULL)
							{
								CFDataGetBytes (data_ref, range, (unsigned char *)result);
								result[length] = 0;
								success = true;
								break;
							}

							CFRelease(data_ref);
							CFRelease(string_ref);
							CFRelease(flavor_data);
						}
					}

					CFRelease(flavor_type_array);
				}
			}

			if (success)
				break;
		}

		CFRelease(pasteboard_ref);
	}

	return result;
}

//============================================================
//  osd_stat
//============================================================

osd_directory_entry *osd_stat(const char *path)
{
	int err;
	osd_directory_entry *result = NULL;
	#if defined(SDLMAME_DARWIN) || defined(SDLMAME_NO64BITIO)
	struct stat st;
	#else
	struct stat64 st;
	#endif

	#if defined(SDLMAME_DARWIN) || defined(SDLMAME_NO64BITIO)
	err = stat(path, &st);
	#else
	err = stat64(path, &st);
	#endif

	if( err == -1) return NULL;

	// create an osd_directory_entry; be sure to make sure that the caller can
	// free all resources by just freeing the resulting osd_directory_entry
	result = (osd_directory_entry *) osd_malloc_array(sizeof(*result) + strlen(path) + 1);
	strcpy(((char *) result) + sizeof(*result), path);
	result->name = ((char *) result) + sizeof(*result);
	result->type = S_ISDIR(st.st_mode) ? ENTTYPE_DIR : ENTTYPE_FILE;
	result->size = (UINT64)st.st_size;

	return result;
}

//============================================================
//  osd_get_volume_name
//============================================================

const char *osd_get_volume_name(int idx)
{
	if (idx!=0) return NULL;
	return "/";
}

//============================================================
//  osd_get_slider_list
//============================================================

const void *osd_get_slider_list()
{
	return NULL;
}

//============================================================
//  osd_get_full_path
//============================================================

file_error osd_get_full_path(char **dst, const char *path)
{
	file_error err;
	char path_buffer[512];

	err = FILERR_NONE;

	if (getcwd(path_buffer, 511) == NULL)
	{
		printf("osd_get_full_path: failed!\n");
		err = FILERR_FAILURE;
	}
	else
	{
		*dst = (char *)osd_malloc_array(strlen(path_buffer)+strlen(path)+3);

		// if it's already a full path, just pass it through
		if (path[0] == '/')
		{
			strcpy(*dst, path);
		}
		else
		{
			sprintf(*dst, "%s%s%s", path_buffer, PATH_SEPARATOR, path);
		}
	}

	return err;
}
