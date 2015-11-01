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
	OSStatus err;

	PasteboardRef pasteboard_ref;
	err = PasteboardCreate(kPasteboardClipboard, &pasteboard_ref);
	if (err)
		return NULL;

	PasteboardSynchronize(pasteboard_ref);

	ItemCount item_count;
	err = PasteboardGetItemCount(pasteboard_ref, &item_count);

	char *result = NULL; // core expects a malloced C string of uft8 data
	for (UInt32 item_index = 1; (item_index <= item_count) && !result; item_index++)
	{
		PasteboardItemID item_id;
		err = PasteboardGetItemIdentifier(pasteboard_ref, item_index, &item_id);
		if (err)
			continue;

		CFArrayRef flavor_type_array;
		err = PasteboardCopyItemFlavors(pasteboard_ref, item_id, &flavor_type_array);
		if (err)
			continue;

		CFIndex const flavor_count = CFArrayGetCount(flavor_type_array);
		for (CFIndex flavor_index = 0; (flavor_index < flavor_count) && !result; flavor_index++)
		{
			CFStringRef const flavor_type = (CFStringRef)CFArrayGetValueAtIndex(flavor_type_array, flavor_index);

			CFStringEncoding encoding;
			if (UTTypeConformsTo(flavor_type, kUTTypeUTF16PlainText))
				encoding = kCFStringEncodingUTF16;
			else if (UTTypeConformsTo (flavor_type, kUTTypeUTF8PlainText))
				encoding = kCFStringEncodingUTF8;
			else if (UTTypeConformsTo (flavor_type, kUTTypePlainText))
				encoding = kCFStringEncodingMacRoman;
			else
				continue;

			CFDataRef flavor_data;
			err = PasteboardCopyItemFlavorData(pasteboard_ref, item_id, flavor_type, &flavor_data);

			if (!err)
			{
				CFStringRef string_ref = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, flavor_data, encoding);
				CFDataRef data_ref = CFStringCreateExternalRepresentation (kCFAllocatorDefault, string_ref, kCFStringEncodingUTF8, '?');
				CFRelease(string_ref);
				CFRelease(flavor_data);

				CFIndex const length = CFDataGetLength(data_ref);
				CFRange const range = CFRangeMake(0, length);

				result = reinterpret_cast<char *>(osd_malloc_array(length + 1));
				if (result)
				{
					CFDataGetBytes(data_ref, range, reinterpret_cast<unsigned char *>(result));
					result[length] = 0;
				}

				CFRelease(data_ref);
			}
		}

		CFRelease(flavor_type_array);
	}

	CFRelease(pasteboard_ref);

	return result;
}
