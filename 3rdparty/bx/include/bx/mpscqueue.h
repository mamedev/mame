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
			LwMutexScope $(m_write);
			m_queue.push(_ptr);
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

	template <typename Ty>
	class MpScUnboundedBlockingQueue
	{
		BX_CLASS(MpScUnboundedBlockingQueue
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		MpScUnboundedBlockingQueue()
		{
		}

		~MpScUnboundedBlockingQueue()
		{
		}

		void push(Ty* _ptr) // producer only
		{
			m_queue.push(_ptr);
			m_sem.post();
		}

		Ty* pop() // consumer only
		{
			m_sem.wait();
			return m_queue.pop();
		}

	private:
		MpScUnboundedQueue<Ty> m_queue;
		Semaphore m_sem;
	};

} // namespace bx

#endif // BX_MPSCQUEUE_H_HEADER_GUARD
