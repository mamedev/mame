/*********************************************************************

    ap_dsk35.h

    Apple 3.5" disk images

*********************************************************************/

#ifndef AP_DSK35_H
#define AP_DSK35_H

#include "flopimg.h"

void sony_filltrack(UINT8 *buffer, size_t buffer_len, size_t *pos, UINT8 data);
UINT8 sony_fetchtrack(const UINT8 *buffer, size_t buffer_len, size_t *pos);

int apple35_sectors_per_track(floppy_image_legacy *image, int track);

/**************************************************************************/

LEGACY_FLOPPY_OPTIONS_EXTERN(apple35_mac);
LEGACY_FLOPPY_OPTIONS_EXTERN(apple35_iigs);


#endif /* AP_DSK35_H */
