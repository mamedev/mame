// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont
/*********************************************************************

    ap_dsk35.h

    Apple 3.5" disk images

*********************************************************************/
#ifndef MAME_FORMATS_AP_DSK35_H
#define MAME_FORMATS_AP_DSK35_H

#pragma once

#include "flopimg.h"
#include "flopimg_legacy.h"

void sony_filltrack(uint8_t *buffer, size_t buffer_len, size_t *pos, uint8_t data);
uint8_t sony_fetchtrack(const uint8_t *buffer, size_t buffer_len, size_t *pos);

int apple35_sectors_per_track(floppy_image_legacy *image, int track);

/**************************************************************************/

LEGACY_FLOPPY_OPTIONS_EXTERN(apple35_mac);
LEGACY_FLOPPY_OPTIONS_EXTERN(apple35_iigs);

class dc42_format : public floppy_image_format_t
{
public:
	dc42_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	void update_chk(const uint8_t *data, int size, uint32_t &chk);
};

extern const floppy_format_type FLOPPY_DC42_FORMAT;

class apple_gcr_format : public floppy_image_format_t
{
public:
	apple_gcr_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_APPLE_GCR_FORMAT;

class apple_2mg_format : public floppy_image_format_t
{
public:
	apple_2mg_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_APPLE_2MG_FORMAT;

#endif // MAME_FORMATS_AP_DSK35_H
