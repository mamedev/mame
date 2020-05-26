// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PMULTI_THREADING_H_
#define PMULTI_THREADING_H_

///
/// \file pmulti_threading.h
///

#include "pconfig.h"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <type_traits>
#include <utility>

namespace plib {

	template<bool enabled_ = true>
	struct pspin_mutex
	{
	public:
		pspin_mutex() noexcept = default;
		void lock() noexcept{ while (m_lock.test_and_set(std::memory_order_acquire)) { } }
		void unlock() noexcept { m_lock.clear(std::memory_order_release); }
	private:
		PALIGNAS_CACHELINE()
		std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
	};

	template<>
	struct pspin_mutex<false>
	{
	public:
		void lock() const noexcept { }
		void unlock() const noexcept { }
	};


} // namespace plib

#endif // PMULTI_THREADING_H_
