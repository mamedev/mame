// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlos_*.c - OS specific low level code
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
