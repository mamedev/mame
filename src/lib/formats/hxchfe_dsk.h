// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*********************************************************************

    formats/hxchfe_dsk.h

    HxC Floppy Emulator HFE file format

*********************************************************************/
#ifndef MAME_FORMATS_HXCHFE_DSK_H
#define MAME_FORMATS_HXCHFE_DSK_H

#pragma once

#include "flopimg.h"

class hfe_format : public floppy_image_format_t
{
public:
	enum encoding_t
	{
		ISOIBM_MFM_ENCODING = 0x00,
		AMIGA_MFM_ENCODING,
		ISOIBM_FM_ENCODING,
		EMU_FM_ENCODING,
		UNKNOWN_ENCODING = 0xff
	};

	enum floppymode_t
	{
		IBMPC_DD_FLOPPYMODE = 00,
		IBMPC_HD_FLOPPYMODE,
		ATARIST_DD_FLOPPYMODE,
		ATARIST_HD_FLOPPYMODE,
		AMIGA_DD_FLOPPYMODE,
		AMIGA_HD_FLOPPYMODE,
		CPC_DD_FLOPPYMODE,
		GENERIC_SHUGART_DD_FLOPPYMODE,
		IBMPC_ED_FLOPPYMODE,
		MSX2_DD_FLOPPYMODE,
		C64_DD_FLOPPYMODE,
		EMU_SHUGART_FLOPPYMODE,
		S950_DD_FLOPPYMODE,
		S950_HD_FLOPPYMODE,
		DISABLE_FLOPPYMODE = 0xfe
	};

	hfe_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
	virtual bool supports_save() const noexcept override;

protected:
	// Format parameters that can be overridden by subclasses
	bool m_standard_track_count;
	int  m_samplerate;
	int  m_rpm;
	bool m_write_allowed;
	bool m_single_step;
	floppymode_t m_floppymode;
	encoding_t m_encoding;

private:
	// Header fields from the HFE format
	struct header_info
	{
		int cylinders = 0;                                // Number of track in the file
		int heads = 0;                                    // Number of valid side
		encoding_t track_encoding = UNKNOWN_ENCODING;     // Track Encoding mode
		int sample_rate = 0;                              // Sample rate in K/s (max: 500)
		floppymode_t interface_mode = DISABLE_FLOPPYMODE; // Floppy interface mode.
		int track_list_offset = 0;

		bool write_allowed = true;
		bool single_step = true;
		bool track0s0_has_altencoding = false;
		encoding_t  track0s0_encoding = UNKNOWN_ENCODING; // alternative track_encoding for track 0 Side 0
		bool track0s1_has_altencoding = false;
		encoding_t  track0s1_encoding = UNKNOWN_ENCODING; // alternative track_encoding for track 0 Side 1
	};

	struct lut_entry
	{
		lut_entry(int _off, int _len) { offset = _off; length = _len; }

		int offset;
		int length;
	};

	void generate_track_from_hfe_bitstream(int cyl, int head, int samplelength, const uint8_t *trackbuf, int track_end, floppy_image &image) const;
	void generate_hfe_bitstream_from_track(int cyl, int head, long cyltime, int samplelength, uint8_t *trackbuf, int track_end, const floppy_image &image) const;

	bool eval_header(header_info& header, uint8_t* headerbytes, int drive_cylinders) const;
	int determine_cell_size(const std::vector<uint32_t> &tbuf) const;
};

extern const hfe_format FLOPPY_HFE_FORMAT;

#endif // MAME_FORMATS_HXCHFE_DSK_H
