/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_MPSCQUEUE_H_HEADER_GUARD
#define BX_MPSCQUEUE_H_HEADER_GUARD

#include "spscqueue.h"

namespace bx
{
	template <typename Ty>
	class MpScUnboundedQueue
	{
		BX_CLASS(MpScUnboundedQueue
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		MpScUnboundedQueue()
		{
		}

		~MpScUnboundedQueue()
		{
		}

		void push(Ty* _ptr) // producer only
		{
			m_write.lock();
			m_queue.push(_ptr);
			m_write.unlock();
		}

		Ty* peek() // consumer only
		{
			return m_queue.peek();
		}

		Ty* pop() // consumer only
		{
			return m_queue.pop();
		}

	private:
		LwMutex m_write;
		SpScUnboundedQueue<Ty> m_queue;
	};

} // namespace bx

#endif // BX_MPSCQUEUE_H_HEADER_GUARD
