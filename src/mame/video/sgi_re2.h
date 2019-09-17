// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_VIDEO_SGI_RE2_H
#define MAME_VIDEO_SGI_RE2_H

#pragma once

class sgi_re2_device : public device_t
{
public:
	sgi_re2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	enum register_number : unsigned
	{
		// buffered registers (write only unless noted)
		RE2_ENABRGB   = 0x04, // enable 8 bit rgb (1)
		RE2_BIGENDIAN = 0x05, // enable big endian mode (1)
		RE2_FUNC      = 0x06, // raster op function (4)
		RE2_HADDR     = 0x07, // starting pixel location (2)
		RE2_NOPUP     = 0x08, // size of uaux (1)
		RE2_XYFRAC    = 0x09, // initial xyfrac (4)
		RE2_RGB       = 0x0a, // initial color values (27)
		RE2_YX        = 0x0b, // initial y and x (22)
		RE2_PUPDATA   = 0x0c, // pup data (2)
		RE2_PATL      = 0x0d, // pattern mask low (16)
		RE2_PATH      = 0x0e, // pattern mask high (16)
		RE2_DZI       = 0x0f, // delta z integer (24)
		RE2_DZF       = 0x10, // delta z fraction (14)
		RE2_DR        = 0x11, // delta red (24)
		RE2_DG        = 0x12, // delta green (20)
		RE2_DB        = 0x13, // delta blue (20)
		RE2_Z         = 0x14, // initial z integer (24)
		RE2_R         = 0x15, // initial red (23)
		RE2_G         = 0x16, // initial green (19)
		RE2_B         = 0x17, // initial blue (19)
		RE2_STIP      = 0x18, // stipple pattern (16, rw)
		RE2_STIPCOUNT = 0x19, // stipple repeat (8, rw)
		RE2_DX        = 0x1a, // delta x (16)
		RE2_DY        = 0x1b, // delta y (16)
		RE2_NUMPIX    = 0x1c, // pixel count (11)
		RE2_X         = 0x1d, // initial x (12)
		RE2_Y         = 0x1e, // initial y (11)
		RE2_IR        = 0x1f, // instruction (3)

		// unbuffered registers (write only unless noted)
		RE2_RWDATA    = 0x20, // read/write data (32, rw)
		RE2_PIXMASK   = 0x21, // pixel mask (24)
		RE2_AUXMASK   = 0x22, // auxiliary mask (9)
		RE2_WIDDATA   = 0x23, // window id (4)
		RE2_UAUXDATA  = 0x24, // uaux data (4)
		RE2_RWMODE    = 0x25, // read/write mode (3)
		RE2_READBUF   = 0x26, // buffer select (1)
		RE2_PIXTYPE   = 0x27, // pixel type (2)
		RE2_ASELECT   = 0x28, // antialias select (6)
		RE2_ALIGNPAT  = 0x29, // pattern alignment (1)
		RE2_ENABPAT   = 0x2a, // enable pattern mask (1)
		RE2_ENABSTIP  = 0x2b, // enable stipple (1)
		RE2_ENABDITH  = 0x2c, // enable dithering (1)
		RE2_ENABWID   = 0x2d, // enable wid check (1)
		RE2_CURWID    = 0x2e, // current wid (4)
		RE2_DEPTHFN   = 0x2f, // depth function (4)
		RE2_REPSTIP   = 0x30, // stipple repeat (8)
		RE2_ENABLWID  = 0x31, // enable line wid (1)
		RE2_FBOPTION  = 0x32, // frame buffer option (2)
		RE2_TOPSCAN   = 0x33, // first row/column (18)
		RE2_ZBOPTION  = 0x36, // z buffer option (1)
		RE2_XZOOM     = 0x37, // x zoom factor (8)
		RE2_UPACMODE  = 0x38, // packing mode (2)
		RE2_YMIN      = 0x39, // bottom screen mask (11)
		RE2_YMAX      = 0x3a, // top screen mask (11)
		RE2_XMIN      = 0x3b, // left screen mask (12)
		RE2_XMAX      = 0x3c, // right screen mask (12)
		RE2_COLORCMP  = 0x3d, // z compare source (1)
		RE2_MEGOPTION = 0x3e, // vram size (1)
	};

	u32 reg_r(offs_t offset);
	void reg_w(offs_t offset, u32 data);

protected:

private:
	u16 m_stip;
	u8 m_stipcount;

	u32 m_rwdata;
};

DECLARE_DEVICE_TYPE(SGI_RE2, sgi_re2_device)

#endif // MAME_VIDEO_SGI_RE2_H
