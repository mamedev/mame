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

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	void set_floppy_mode(floppymode_t mode) { m_selected_mode = mode; }
	void set_encoding(encoding_t enc) { m_selected_encoding = enc; }

private:
	void generate_track_from_hfe_bitstream(int track, int head, int samplelength, const uint8_t *trackbuf, int track_end, floppy_image *image);
	void generate_hfe_bitstream_from_track(int track, int head, int& samplelength, encoding_t& encoding, uint8_t *trackbuf, int track_end, floppy_image *image);

	// Header fields from the HFE format
	int m_cylinders;                 // Number of track in the file
	int m_heads;                     // Number of valid side
	encoding_t m_track_encoding;     // Track Encoding mode
	int m_bit_rate;                  // Bitrate in Kbit/s (max: 500)
	int m_floppy_rpm;                // Rotation per minute
	floppymode_t m_interface_mode;   // Floppy interface mode.

	bool m_write_allowed;
	bool m_single_step;
	bool m_track0s0_has_altencoding;
	encoding_t  m_track0s0_encoding; // alternate track_encoding for track 0 Side 0
	bool m_track0s1_has_altencoding;
	encoding_t  m_track0s1_encoding; // alternate track_encoding for track 0 Side 1

	int m_cyl_offset[256];
	int m_cyl_length[256];

	floppymode_t m_selected_mode;
	encoding_t m_selected_encoding;
};

extern const floppy_format_type FLOPPY_HFE_FORMAT;

#endif // MAME_FORMATS_HXCHFE_DSK_H
