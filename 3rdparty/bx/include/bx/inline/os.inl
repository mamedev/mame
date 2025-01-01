/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_H_HEADER_GUARD
#	error "Must be included from bx/os.h!"
#endif // BX_H_HEADER_GUARD

namespace bx
{
	template<typename ProtoT>
	inline ProtoT dlsym(void* _handle, const StringView& _symbol)
	{
		return reinterpret_cast<ProtoT>(dlsym(_handle, _symbol) );
	}

} // namespace bx
