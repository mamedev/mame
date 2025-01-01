/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_SEM_H_HEADER_GUARD
#define BX_SEM_H_HEADER_GUARD

#include "bx.h"

namespace bx
{
	///
	class Semaphore
	{
		BX_CLASS(Semaphore
			, NO_COPY
			);

	public:
		///
		Semaphore();

		///
		~Semaphore();

		///
		void post(uint32_t _count = 1);

		///
		bool wait(int32_t _msecs = -1);

	private:
		BX_ALIGN_DECL(16, uint8_t) m_internal[128];
	};

} // namespace bx

#endif // BX_SEM_H_HEADER_GUARD
