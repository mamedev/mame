// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PMULTI_THREADING_H_
#define PMULTI_THREADING_H_

///
/// \file pmulti_threading.h
///

#include "pconfig.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <type_traits>
#include <utility>

namespace plib {

	template<bool enabled_ = true>
	struct pspin_mutex
	{
	public:
		inline pspin_mutex() noexcept = default;
		inline void lock() noexcept{ while (m_lock.test_and_set(std::memory_order_acquire)) { } }
		inline void unlock() noexcept { m_lock.clear(std::memory_order_release); }
	private:
		PALIGNAS_CACHELINE()
		std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
	};

	template<>
	struct pspin_mutex<false>
	{
	public:
		inline pspin_mutex() noexcept = default;
		static inline void lock() noexcept { }
		static inline void unlock() noexcept { }
	};

	class psemaphore
	{
	public:

		psemaphore(long count = 0) noexcept: m_count(count) { }

		psemaphore(const psemaphore& other) = delete;
		psemaphore& operator=(const psemaphore& other) = delete;
		psemaphore(psemaphore&& other) = delete;
		psemaphore& operator=(psemaphore&& other) = delete;
		~psemaphore() = default;

		void release(long update = 1)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_count += update;
			m_cv.notify_one();
		}

		void acquire()
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			while (m_count == 0)
				m_cv.wait(lock);
			--m_count;
		}

		bool try_acquire()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_count)
			{
				--m_count;
				return true;
			}
			return false;
		}
	private:
		std::mutex m_mutex;
		std::condition_variable m_cv;
		long m_count;
	};


} // namespace plib

#endif // PMULTI_THREADING_H_
