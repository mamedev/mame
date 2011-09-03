/*********************************************************************

    formats/st_dsk.h

    Atari ST generic 9/10/11 sector-per-track formats

*********************************************************************/

#ifndef ST_DSK_H_
#define ST_DSK_H_

#include "flopimg.h"

class st_gen_format : public floppy_image_format_t
{
public:
	st_gen_format();

	static const desc_e desc_fcp_9[];

	static const desc_e desc_fcp_10_0[];
	static const desc_e desc_fcp_10_1[];
	static const desc_e desc_fcp_10_2[];
	static const desc_e desc_fcp_10_3[];
	static const desc_e desc_fcp_10_4[];
	static const desc_e desc_fcp_10_5[];
	static const desc_e desc_fcp_10_6[];
	static const desc_e desc_fcp_10_7[];
	static const desc_e desc_fcp_10_8[];
	static const desc_e desc_fcp_10_9[];
	static const desc_e *const desc_fcp_10[];

	static const desc_e desc_fcp_11_0[];
	static const desc_e desc_fcp_11_1[];
	static const desc_e desc_fcp_11_2[];
	static const desc_e desc_fcp_11_3[];
	static const desc_e desc_fcp_11_4[];
	static const desc_e desc_fcp_11_5[];
	static const desc_e desc_fcp_11_6[];
	static const desc_e desc_fcp_11_7[];
	static const desc_e desc_fcp_11_8[];
	static const desc_e desc_fcp_11_9[];
	static const desc_e desc_fcp_11_10[];
	static const desc_e *const desc_fcp_11[];

	void generate(int track, int head, int track_count, int head_count, UINT8 *buffer, int sector_count, floppy_image *image);
};

class st_format : public st_gen_format
{
public:
	st_format();

	virtual int identify(floppy_image *image);
	virtual bool load(floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;

private:
	void find_size(floppy_image *image, int &track_count, int &head_count, int &sector_count);
};

class msa_format : public st_gen_format
{
public:
	msa_format();

	virtual int identify(floppy_image *image);
	virtual bool load(floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;

private:
	bool uncompress(UINT8 *buffer, int csize, int usize);
	void read_header(floppy_image *image, UINT16 &sign, UINT16 &sect, UINT16 &head, UINT16 &strack, UINT16 &etrack);
};

extern const floppy_format_type FLOPPY_ST_FORMAT;
extern const floppy_format_type FLOPPY_MSA_FORMAT;

#endif /*ST_DSK_H_*/
