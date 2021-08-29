/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg#license-bsd-2-clause
 */

#include "bimg_p.h"

namespace bimg
{
	bool imageParseGnf(ImageContainer& _imageContainer, bx::ReaderSeekerI* _reader, bx::Error* _err)
	{
		BX_UNUSED(_imageContainer, _reader, _err);
		BX_ERROR_SET(_err, BIMG_ERROR, "GNF: not supported.");
		return false;
	}

	ImageContainer* imageParseGnf(bx::AllocatorI* _allocator, const void* _src, uint32_t _size, bx::Error* _err)
	{
		BX_UNUSED(_allocator);

		bx::MemoryReader reader(_src, _size);

		uint32_t magic;
		bx::read(&reader, magic);

		ImageContainer imageContainer;
		if (BIMG_CHUNK_MAGIC_GNF != magic
		|| !imageParseGnf(imageContainer, &reader, _err) )
		{
			return NULL;
		}

		BX_ERROR_SET(_err, BIMG_ERROR, "GNF: not supported.");
		return NULL;
	}

} // namespace bimg
