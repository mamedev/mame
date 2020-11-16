// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    nanosvg.h

    NanoSVG helpers.

***************************************************************************/
#ifndef MAME_LIB_UTIL_NANOSVG_H
#define MAME_LIB_UTIL_NANOSVG_H

#include <nanosvg/src/nanosvg.h>
#include <nanosvg/src/nanosvgrast.h>

#include <memory>


namespace util {

struct nsvg_deleter
{
	void operator()(NSVGimage *ptr) const { nsvgDelete(ptr); }
	void operator()(NSVGrasterizer *ptr) const { nsvgDeleteRasterizer(ptr); }
};


using nsvg_image_ptr = std::unique_ptr<NSVGimage, nsvg_deleter>;
using nsvg_rasterizer_ptr = std::unique_ptr<NSVGrasterizer, nsvg_deleter>;

} // namespace util

#endif // MAME_LIB_UTIL_NANOSVG_H
