/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SPSCQUEUE_H_HEADER_GUARD
#	error "Must be included from bx/spscqueue.h!"
#endif // BX_SPSCQUEUE_H_HEADER_GUARD

namespace bx
{
	// Reference(s):
	// - Writing Lock-Free Code: A Corrected Queue
	//   https://web.archive.org/web/20190207230604/http://www.drdobbs.com/parallel/writing-lock-free-code-a-corrected-queue/210604448
	//
	inline SpScUnboundedQueue::SpScUnboundedQueue(AllocatorI* _allocator)
		: m_allocator(_allocator)
		, m_first(BX_NEW(m_allocator, Node)(NULL) )
		, m_divider(m_first)
		, m_last(m_first)
	{
	}

	inline SpScUnboundedQueue::~SpScUnboundedQueue()
	{
		while (NULL != m_first)
		{
			Node* node = m_first;
			m_first = node->m_next;
			BX_DELETE(m_allocator, node);
		}
	}

	inline void SpScUnboundedQueue::push(void* _ptr)
	{
		m_last->m_next = BX_NEW(m_allocator, Node)(_ptr);
		atomicExchangePtr( (void**)&m_last, m_last->m_next);
		while (m_first != m_divider)
		{
			Node* node = m_first;
			m_first = m_first->m_next;
			BX_DELETE(m_allocator, node);
		}
	}

	inline void* SpScUnboundedQueue::peek()
	{
		if (m_divider != m_last)
		{
			return m_divider->m_next->m_ptr;
		}

		return NULL;
	}

	inline void* SpScUnboundedQueue::pop()
	{
		if (m_divider != m_last)
		{
			void* ptr = m_divider->m_next->m_ptr;
			atomicExchangePtr( (void**)&m_divider, m_divider->m_next);
			return ptr;
		}

		return NULL;
	}

	inline SpScUnboundedQueue::Node::Node(void* _ptr)
		: m_ptr(_ptr)
		, m_next(NULL)
	{
	}

	template<typename Ty>
	inline SpScUnboundedQueueT<Ty>::SpScUnboundedQueueT(AllocatorI* _allocator)
		: m_queue(_allocator)
	{
	}

	template<typename Ty>
	inline SpScUnboundedQueueT<Ty>::~SpScUnboundedQueueT()
	{
	}

	template<typename Ty>
	inline void SpScUnboundedQueueT<Ty>::push(Ty* _ptr)
	{
		m_queue.push(_ptr);
	}

	template<typename Ty>
	inline Ty* SpScUnboundedQueueT<Ty>::peek()
	{
		return (Ty*)m_queue.peek();
	}

	template<typename Ty>
	inline Ty* SpScUnboundedQueueT<Ty>::pop()
	{
		return (Ty*)m_queue.pop();
	}

#if BX_CONFIG_SUPPORTS_THREADING
	inline SpScBlockingUnboundedQueue::SpScBlockingUnboundedQueue(AllocatorI* _allocator)
		: m_queue(_allocator)
	{
	}

	inline SpScBlockingUnboundedQueue::~SpScBlockingUnboundedQueue()
	{
	}

	inline void SpScBlockingUnboundedQueue::push(void* _ptr)
	{
		m_queue.push(_ptr);
		m_count.post();
	}

	inline void* SpScBlockingUnboundedQueue::peek()
	{
		return m_queue.peek();
	}

	inline void* SpScBlockingUnboundedQueue::pop(int32_t _msecs)
	{
		if (m_count.wait(_msecs) )
		{
			return m_queue.pop();
		}

		return NULL;
	}

	template<typename Ty>
	inline SpScBlockingUnboundedQueueT<Ty>::SpScBlockingUnboundedQueueT(AllocatorI* _allocator)
		: m_queue(_allocator)
	{
	}

	template<typename Ty>
	inline SpScBlockingUnboundedQueueT<Ty>::~SpScBlockingUnboundedQueueT()
	{
	}

	template<typename Ty>
	inline void SpScBlockingUnboundedQueueT<Ty>::push(Ty* _ptr)
	{
		m_queue.push(_ptr);
	}

	template<typename Ty>
	inline Ty* SpScBlockingUnboundedQueueT<Ty>::peek()
	{
		return (Ty*)m_queue.peek();
	}

	template<typename Ty>
	inline Ty* SpScBlockingUnboundedQueueT<Ty>::pop(int32_t _msecs)
	{
		return (Ty*)m_queue.pop(_msecs);
	}

#endif // BX_CONFIG_SUPPORTS_THREADING

} // namespace bx
