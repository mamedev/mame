// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/imd_dsk.h

    IMD disk images

*********************************************************************/
#ifndef MAME_FORMATS_IMD_DSK_H
#define MAME_FORMATS_IMD_DSK_H

#include "flopimg.h"

#include <string>
#include <vector>

class imd_format : public floppy_image_format_t
{
public:
	imd_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
	virtual bool supports_save() const noexcept override;

private:
	// Rich per-sector record produced by extract_track().
	// Captures everything IMD's per-sector type byte can encode.
	struct extracted_sector
	{
		uint8_t  idam_track  = 0;
		uint8_t  idam_head   = 0;
		uint8_t  idam_sector = 0;
		uint8_t  idam_size   = 0;
		bool     addr_crc_ok = false;
		bool     data_crc_ok = false;
		bool     deleted_dam = false;
		bool     has_data    = false;
		std::vector<uint8_t> data;
	};

	struct track_info
	{
		bool                          is_mfm    = false;
		int                           cell_size = 0;   // ns
		uint8_t                       mode_byte = 5;   // IMD mode (0..5)
		std::vector<extracted_sector> sectors;
	};

	// Rich extractor: walks bitstream once, captures physical sector
	// order, IDAM contents, DAM-deleted flag, and CRC validity for
	// both address and data records.  Self-contained so the shared
	// flopimg.cpp extractors do not need to change.
	void extract_track_rich(const std::vector<bool> &bitstream, bool is_mfm,
	                        std::vector<extracted_sector> &out) const;

	// Per-track encoding/cell-size detection.  Tries the four common
	// combinations (MFM @ 1000/2000ns, FM @ 2000/4000ns) and picks
	// the one with the most cleanly-decoded sectors.  Returns false
	// if no encoding produced any sectors (track is unformatted or
	// unrecognised).
	bool detect_track(const floppy_image &image, int cyl, int head,
	                  track_info &out) const;
};

extern const imd_format FLOPPY_IMD_FORMAT;

#endif // MAME_FORMATS_IMD_DSK_H
