/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SPSCQUEUE_H_HEADER_GUARD
#define BX_SPSCQUEUE_H_HEADER_GUARD

#include "allocator.h"
#include "cpu.h"
#include "semaphore.h"

namespace bx
{
	///
	class SpScUnboundedQueue
	{
		BX_CLASS(SpScUnboundedQueue
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		SpScUnboundedQueue(AllocatorI* _allocator);

		///
		~SpScUnboundedQueue();

		///
		void push(void* _ptr);

		///
		void* peek();

		///
		void* pop();

	private:
		struct Node
		{
			///
			Node(void* _ptr);

			void* m_ptr;
			Node* m_next;
		};

		AllocatorI* m_allocator;
		Node* m_first;
		Node* m_divider;
		Node* m_last;
	};

	///
	template<typename Ty>
	class SpScUnboundedQueueT
	{
		BX_CLASS(SpScUnboundedQueueT
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		SpScUnboundedQueueT(AllocatorI* _allocator);

		///
		~SpScUnboundedQueueT();

		///
		void push(Ty* _ptr);

		///
		Ty* peek();

		///
		Ty* pop();

	private:
		SpScUnboundedQueue m_queue;
	};

#if BX_CONFIG_SUPPORTS_THREADING
	///
	class SpScBlockingUnboundedQueue
	{
		BX_CLASS(SpScBlockingUnboundedQueue
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		SpScBlockingUnboundedQueue(AllocatorI* _allocator);

		///
		~SpScBlockingUnboundedQueue();

		///
		void push(void* _ptr); // producer only

		///
		void* peek(); // consumer only

		///
		void* pop(int32_t _msecs = -1); // consumer only

	private:
		Semaphore m_count;
		SpScUnboundedQueue m_queue;
	};

	///
	template<typename Ty>
	class SpScBlockingUnboundedQueueT
	{
		BX_CLASS(SpScBlockingUnboundedQueueT
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		SpScBlockingUnboundedQueueT(AllocatorI* _allocator);

		///
		~SpScBlockingUnboundedQueueT();

		///
		void push(Ty* _ptr); // producer only

		///
		Ty* peek(); // consumer only

		///
		Ty* pop(int32_t _msecs = -1); // consumer only

	private:
		SpScBlockingUnboundedQueue m_queue;
	};
#endif // BX_CONFIG_SUPPORTS_THREADING

} // namespace bx

#include "inline/spscqueue.inl"

#endif // BX_SPSCQUEUE_H_HEADER_GUARD
