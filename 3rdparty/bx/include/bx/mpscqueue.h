/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_MPSCQUEUE_H_HEADER_GUARD
#define BX_MPSCQUEUE_H_HEADER_GUARD

#include "allocator.h"
#include "mutex.h"
#include "spscqueue.h"

namespace bx
{
	///
	template <typename Ty>
	class MpScUnboundedQueueT
	{
		BX_CLASS(MpScUnboundedQueueT
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		MpScUnboundedQueueT(AllocatorI* _allocator);

		///
		~MpScUnboundedQueueT();

		///
		void push(Ty* _ptr); // producer only

		///
		Ty* peek(); // consumer only

		///
		Ty* pop(); // consumer only

	private:
		Mutex m_write;
		SpScUnboundedQueueT<Ty> m_queue;
	};

	///
	template <typename Ty>
	class MpScUnboundedBlockingQueue
	{
		BX_CLASS(MpScUnboundedBlockingQueue
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		MpScUnboundedBlockingQueue(AllocatorI* _allocator);

		///
		~MpScUnboundedBlockingQueue();

		///
		void push(Ty* _ptr); // producer only

		///
		Ty* pop(); // consumer only

	private:
		MpScUnboundedQueueT<Ty> m_queue;
		Semaphore m_sem;
	};

} // namespace bx

#include "inline/mpscqueue.inl"

#endif // BX_MPSCQUEUE_H_HEADER_GUARD
