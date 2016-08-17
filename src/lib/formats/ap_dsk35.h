// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont
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

class dc42_format : public floppy_image_format_t
{
public:
	dc42_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	static const desc_e mac_gcr[];

	UINT8 gb(const UINT8 *buf, int ts, int &pos, int &wrap);
	void update_chk(const UINT8 *data, int size, UINT32 &chk);
};

extern const floppy_format_type FLOPPY_DC42_FORMAT;

#endif /* AP_DSK35_H */
