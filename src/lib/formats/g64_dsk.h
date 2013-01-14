/*********************************************************************

    formats/g64_dsk.h

    Commodore 1541 GCR disk image format

*********************************************************************/

#ifndef G64_DSK_H_
#define G64_DSK_H_

#include "flopimg.h"
#include "imageutl.h"

class g64_format : public floppy_image_format_t {
public:
	g64_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);
	virtual bool supports_save() const;

protected:
	static const UINT32 c1541_cell_size[];
};

extern const floppy_format_type FLOPPY_G64_FORMAT;


// legacy
#define G64_SYNC_MARK           0x3ff       /* 10 consecutive 1-bits */

#define G64_BUFFER_SIZE         16384
#define G64_SPEED_BLOCK_SIZE    1982

const int C2040_BITRATE[] =
{
	XTAL_16MHz/16,  /* tracks  1-17 */
	XTAL_16MHz/15,  /* tracks 18-24 */
	XTAL_16MHz/14,  /* tracks 25-30 */
	XTAL_16MHz/13   /* tracks 31-42 */
};

const int C8050_BITRATE[] =
{
	XTAL_12MHz/2/16,    /* tracks  1-39 */
	XTAL_12MHz/2/15,    /* tracks 40-53 */
	XTAL_12MHz/2/14,    /* tracks 54-65 */
	XTAL_12MHz/2/13     /* tracks 65-84 */
};

FLOPPY_IDENTIFY( g64_dsk_identify );
FLOPPY_CONSTRUCT( g64_dsk_construct );

#endif
