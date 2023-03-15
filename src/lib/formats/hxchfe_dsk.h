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
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	static void generate_track_from_hfe_bitstream(int track, int head, int samplelength, const uint8_t *trackbuf, int track_end, floppy_image *image);

	// Header fields from the HFE format
	struct header_info {
		int m_cylinders = 0;                                // Number of track in the file
		int m_heads = 0;                                    // Number of valid side
		encoding_t m_track_encoding = UNKNOWN_ENCODING;     // Track Encoding mode
		int m_bit_rate = 0;                                 // Bitrate in Kbit/s (max: 500)
		int m_floppy_rpm = 0;                               // Rotation per minute
		floppymode_t m_interface_mode = DISABLE_FLOPPYMODE; // Floppy interface mode.

		bool m_write_allowed = true;
		bool m_single_step = true;
		bool m_track0s0_has_altencoding = false;
		encoding_t  m_track0s0_encoding = UNKNOWN_ENCODING; // alternate track_encoding for track 0 Side 0
		bool m_track0s1_has_altencoding = false;
		encoding_t  m_track0s1_encoding = UNKNOWN_ENCODING; // alternate track_encoding for track 0 Side 1

		int m_cyl_offset[256];
		int m_cyl_length[256];

		floppymode_t m_selected_mode = DISABLE_FLOPPYMODE;
		encoding_t m_selected_encoding = UNKNOWN_ENCODING;
	};

	static void set_floppy_mode(header_info &info, floppymode_t mode) { info.m_selected_mode = mode; }
	static void set_encoding(header_info &info, encoding_t enc) { info.m_selected_encoding = enc; }
};

extern const hfe_format FLOPPY_HFE_FORMAT;

#endif // MAME_FORMATS_HXCHFE_DSK_H
