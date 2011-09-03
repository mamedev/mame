#ifndef MFI_DSK_H
#define MFI_DSK_H

#include "flopimg.h"

class mfi_format : public floppy_image_format_t
{
public:
	mfi_format();

	virtual int identify(floppy_image *image);
	virtual bool load(floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;

private:
	enum {
		TIME_MASK = 0x0fffffff,
		BIT_MASK  = 0xf0000000,
		BIT_SHIFT = 28,

		BIT_0     = (0 << BIT_SHIFT),
		BIT_1     = (1 << BIT_SHIFT),
		BIT_WEAK  = (2 << BIT_SHIFT)
	};

	static const char sign[16];

	struct header {
		char sign[16];
		unsigned int cyl_count, head_count;
	};

	struct entry {
		unsigned int offset, compressed_size, uncompressed_size;
	};

	void advance(const UINT32 *trackbuf, UINT32 &cur_cell, UINT32 cell_count, UINT32 time);
	UINT32 get_next_edge(const UINT32 *trackbuf, UINT32 cur_cell, UINT32 cell_count);	
};

extern const floppy_format_type FLOPPY_MFI_FORMAT;

#endif /* MFI_DSK_H */
