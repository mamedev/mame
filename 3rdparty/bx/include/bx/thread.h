/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_THREAD_H_HEADER_GUARD
#define BX_THREAD_H_HEADER_GUARD

#include "allocator.h"
#include "mpscqueue.h"

namespace bx
{
	///
	typedef int32_t (*ThreadFn)(class Thread* _self, void* _userData);

	///
	class Thread
	{
		BX_CLASS(Thread
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		Thread();

		///
		virtual ~Thread();

		///
		void init(ThreadFn _fn, void* _userData = NULL, uint32_t _stackSize = 0, const char* _name = NULL);

		///
		void shutdown();

		///
		bool isRunning() const;

		///
		int32_t getExitCode() const;

		///
		void setThreadName(const char* _name);

		///
		void push(void* _ptr);

		///
		void* pop();

	private:
		friend struct ThreadInternal;
		int32_t entry();

		BX_ALIGN_DECL(16, uint8_t) m_internal[64];

		ThreadFn  m_fn;
		void*     m_userData;
		MpScUnboundedBlockingQueue<void> m_queue;
		Semaphore m_sem;
		uint32_t  m_stackSize;
		int32_t   m_exitCode;
		bool      m_running;
	};

	///
	class TlsData
	{
	public:
		///
		TlsData();

		///
		~TlsData();

		///
		void* get() const;

		///
		void set(void* _ptr);

	private:
		BX_ALIGN_DECL(16, uint8_t) m_internal[64];
	};

} // namespace bx

#endif // BX_THREAD_H_HEADER_GUARD
