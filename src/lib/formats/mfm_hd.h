// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Hard disk support
    See mfm_hd.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __MFMHDFMT__
#define __MFMHDFMT__

#include "emu.h"
#include "chd.h"

const chd_metadata_tag MFM_HARD_DISK_METADATA_TAG = CHD_MAKE_TAG('G','D','D','I');

extern const char *MFMHD_REC_METADATA_FORMAT;
extern const char *MFMHD_GAP_METADATA_FORMAT;

/*
    Determine how data are passed from the hard disk to the controller. We
    allow for different degrees of hardware emulation.
*/
enum mfmhd_enc_t
{
	MFM_BITS,               // One bit at a time
	MFM_BYTE,               // One data byte with interleaved clock bits
	SEPARATED,              // 8 clock bits (most sig byte), one data byte (least sig byte)
	SEPARATED_SIMPLE        // MSB: 00/FF (standard / mark) clock, LSB: one data byte
};

class mfmhd_image_format_t;

// Pointer to its alloc function
typedef mfmhd_image_format_t *(*mfmhd_format_type)();

template<class _FormatClass>
mfmhd_image_format_t *mfmhd_image_format_creator()
{
	return new _FormatClass();
}

/*
    Parameters for the track layout
*/
class mfmhd_layout_params
{
public:
	// Geometry params. These are fixed for the device. However, sector sizes
	// could be changed, but we do not support this (yet). These are defined
	// in the CHD and must match those of the device. They are stored by the GDDD tag.
	// The encoding is not stored in the CHD but is also supposed to be immutable.
	int     cylinders;
	int     heads;
	int     sectors_per_track;
	int     sector_size;
	mfmhd_enc_t     encoding;

	// Parameters like interleave, precompensation, write current can be changed
	// on every write operation. They are stored by the GDDI tag (first record).
	int     interleave;
	int     cylskew;
	int     headskew;
	int     write_precomp_cylinder;     // if -1, no wpcom on the disks
	int     reduced_wcurr_cylinder;     // if -1, no rwc on the disks

	// Parameters for the track layout that are supposed to be the same for
	// all tracks and that do not change (until the next reformat).
	// Also, they do not have any influence on the CHD file.
	// They are stored by the GDDI tag (second record).
	int     gap1;
	int     gap2;
	int     gap3;
	int     sync;
	int     headerlen;
	int     ecctype;        // -1 is CRC

	bool sane_rec()
	{
		return ((interleave > 0 && interleave < 32) && (cylskew >= 0 && cylskew < 32) && (headskew >= 0 && headskew < 32)
			&& (write_precomp_cylinder >= -1 && write_precomp_cylinder < 100000)
			&& (reduced_wcurr_cylinder >= -1 && reduced_wcurr_cylinder < 100000));
	}

	void reset_rec()
	{
		interleave = cylskew = headskew = 0;
		write_precomp_cylinder = reduced_wcurr_cylinder = -1;
	}

	bool sane_gap()
	{
		return ((gap1 >= 1 && gap1 < 1000) && (gap2 >= 1 && gap2 < 20) && (gap3 >= 1 && gap3 < 1000)
			&& (sync >= 10 && sync < 20)
			&& (headerlen >= 4 && headerlen<=5) && (ecctype>=-1 && ecctype < 10));
	}

	void reset_gap()
	{
		gap1 = gap2 = gap3 = sync = headerlen = ecctype = 0;
	}

	bool equals_rec(mfmhd_layout_params* other)
	{
		return ((interleave == other->interleave) &&
				(cylskew == other->cylskew) &&
				(headskew == other->headskew) &&
				(write_precomp_cylinder == other->write_precomp_cylinder) &&
				(reduced_wcurr_cylinder == other->reduced_wcurr_cylinder));
	}

	bool equals_gap(mfmhd_layout_params* other)
	{
		return ((gap1 == other->gap1) &&
				(gap2 == other->gap2) &&
				(gap3 == other->gap3) &&
				(sync == other->sync) &&
				(headerlen == other->headerlen) &&
				(ecctype == other->ecctype));
	}
};

enum mfmhd_param_t
{
	MFMHD_IL,
	MFMHD_HSKEW,
	MFMHD_CSKEW,
	MFMHD_WPCOM,
	MFMHD_RWC,
	MFMHD_GAP1,
	MFMHD_GAP2,
	MFMHD_GAP3,
	MFMHD_SYNC,
	MFMHD_HLEN,
	MFMHD_ECC
};

/*
    Hard disk format
*/
class mfmhd_image_format_t
{
public:
	mfmhd_image_format_t() { m_devtag = std::string("mfmhd_image_format_t"); };
	virtual ~mfmhd_image_format_t() {};

	// Load the image.
	virtual chd_error load(chd_file* chdfile, UINT16* trackimage, int tracksize, int cylinder, int head) = 0;

	// Save the image.
	virtual chd_error save(chd_file* chdfile, UINT16* trackimage, int tracksize, int cylinder, int head) = 0;

	// Return the original parameters of the image
	mfmhd_layout_params* get_initial_params() { return &m_param_old; }

	// Return the recent parameters of the image
	mfmhd_layout_params* get_current_params() { return &m_param; }

	// Set the track layout parameters (and reset the skew detection values)
	void set_layout_params(mfmhd_layout_params param);

	// Concrete format shall decide whether we want to save the retrieved parameters or not.
	virtual bool save_param(mfmhd_param_t type) =0;

	// Accept a tag for log output, since this is not a device instance
	void set_tag(std::string tag) { m_devtag = tag; }

protected:
	bool    m_lastbit;
	int     m_current_crc;
	int     m_secnumber[4];     // used to determine the skew values
	std::string m_devtag;

	mfmhd_layout_params m_param, m_param_old;

	void    mfm_encode(UINT16* trackimage, int& position, UINT8 byte, int count=1);
	void    mfm_encode_a1(UINT16* trackimage, int& position);
	void    mfm_encode_mask(UINT16* trackimage, int& position, UINT8 byte, int count, int mask);
	UINT8   mfm_decode(UINT16 raw);

	// Deliver defaults.
	virtual int get_default(mfmhd_param_t type) =0;

	// Debugging
	void    showtrack(UINT16* enctrack, int length);
	virtual const char* tag() { return m_devtag.c_str(); }
};

class mfmhd_generic_format : public mfmhd_image_format_t
{
public:
	mfmhd_generic_format() { m_devtag = std::string("mfmhd_generic_format"); };
	chd_error load(chd_file* chdfile, UINT16* trackimage, int tracksize, int cylinder, int head);
	chd_error save(chd_file* chdfile, UINT16* trackimage, int tracksize, int cylinder, int head);

	// Yes, we want to save all parameters
	virtual bool save_param(mfmhd_param_t type) { return true; }
	virtual int get_default(mfmhd_param_t type);

protected:
	virtual UINT8   cylinder_to_ident(int cylinder);
	virtual int     chs_to_lba(int cylinder, int head, int sector);
};

extern const mfmhd_format_type MFMHD_GEN_FORMAT;

#endif
