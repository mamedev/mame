// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "gen_fifo.h"

template<typename T> generic_fifo_device_base<T>::generic_fifo_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_empty_cb(*this),
	m_full_cb(*this),
	m_on_fifo_empty_pre_sync([](){}),
	m_on_fifo_empty_post_sync([](){}),
	m_on_fifo_unempty([](){}),
	m_on_fifo_full_post_sync([](){}),
	m_on_fifo_unfull([](){}),
	m_on_push([](){}),
	m_on_pop([](){}),
	m_sync_empty(nullptr),
	m_sync_full(nullptr),
	m_size(0),
	m_empty_triggered(false),
	m_full_triggered(false)
{
}

template<typename T> void generic_fifo_device_base<T>::clear()
{
	bool was_empty = is_empty();
	bool was_full = m_size && is_full();
	m_values.clear();
	m_extra_values.clear();

	if(m_sync_empty) {
		m_sync_empty->adjust(attotime::never);
		m_sync_full->adjust(attotime::never);
	}

	if(was_full)
		m_full_cb(false);

	if(m_full_triggered) {
		m_full_triggered = false;
		m_on_fifo_unfull();
	}

	if(!was_empty)
		m_empty_cb(true);
}

template<typename T> void generic_fifo_device_base<T>::device_start()
{
	m_empty_cb.resolve_safe();
	m_full_cb.resolve_safe();

	m_sync_empty = timer_alloc(FUNC(generic_fifo_device_base<T>::sync_empty), this);
	m_sync_full = timer_alloc(FUNC(generic_fifo_device_base<T>::sync_full), this);

	// This is not saving the fifo, let's hope it's empty...
	save_item(NAME(m_empty_triggered));
	save_item(NAME(m_full_triggered));
}

template<typename T> TIMER_CALLBACK_MEMBER(generic_fifo_device_base<T>::sync_empty)
{
	// Sync was called for a fifo empty, is it still empty?
	if(is_empty()) {
		// If yes, stop the destination if not done yet
		if(!m_empty_triggered) {
			m_empty_triggered = true;
			m_on_fifo_empty_post_sync();
		}
	}
}

template<typename T> TIMER_CALLBACK_MEMBER(generic_fifo_device_base<T>::sync_full)
{
	// Add the extra values in if there's space
	bool was_empty = is_empty();
	bool was_full = is_full();

	while(!m_extra_values.empty() && !is_full()) {
		T t(std::move(m_extra_values.front()));
		m_extra_values.erase(m_extra_values.begin());
		m_values.emplace_back(std::move(t));
	}

	// Adjust devcb lines as needed
	if(was_empty && !is_empty())
		m_empty_cb(false);
	if(!was_full && is_full())
		m_full_cb(true);

	// Are there still values that don't fit?
	if(!m_extra_values.empty()) {
		// If yes, stop the source if not done yet
		if(!m_full_triggered) {
			m_full_triggered = true;
			m_on_fifo_full_post_sync();
		}
	}
}

template<typename T> T generic_fifo_device_base<T>::pop()
{
	if(machine().side_effects_disabled()) {
		if(is_empty())
			return T();
		return m_values.front();
	}

	// Are we empty?
	if(is_empty()) {
		// First, trigger the sync
		m_sync_empty->adjust(attotime::zero);
		// Then fire the callback
		m_on_fifo_empty_pre_sync();

		return T();

	} else {
		// Prepare the value
		bool was_full = is_full();
		T t(std::move(m_values.front()));
		m_values.erase(m_values.begin());

		// If we have extra values and the source is stopped,
		// push one in, and possibly restart the source.
		if(!m_extra_values.empty()) {
			T t1(std::move(m_extra_values.front()));
			m_extra_values.erase(m_extra_values.begin());
			m_values.emplace_back(std::move(t1));

			if(m_extra_values.empty()) {
				// We don't have any extra values left, we can unblock
				// the source.
				if(m_full_triggered) {
					m_full_triggered = false;
					m_on_fifo_unfull();
				}
			}
		}

		// Update the devcb lines
		if(is_empty())
			m_empty_cb(true);
		if(was_full && !is_full())
			m_full_cb(false);

		// We did a pop
		m_on_pop();
		return std::move(t);
	}
}

template<typename T> void generic_fifo_device_base<T>::push(T t)
{
	// Are we already overflowed?
	if(!m_extra_values.empty())
		// If yes, just add to the overflow queue
		m_extra_values.emplace_back(std::move(t));

	else if(is_full()) {
		// Otherwise, are we full?

		// If yes start the overflow queue
		m_extra_values.emplace_back(std::move(t));

		// And trigger the sync
		m_sync_full->adjust(attotime::zero);

	} else {
		// Add the value to the fifo
		bool was_empty = is_empty();
		m_values.emplace_back(std::move(t));

		// Update the devcb lines
		if(was_empty)
			m_empty_cb(false);
		if(is_full())
			m_full_cb(true);

		// Unblock the destination if needed
		if(m_empty_triggered) {
			m_empty_triggered = false;
			m_on_fifo_unempty();
		}
	}

	// We did a push
	m_on_push();
}

template class generic_fifo_device_base<u32>;

generic_fifo_u32_device::generic_fifo_u32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	generic_fifo_device_base<u32>(mconfig, GENERIC_FIFO_U32, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(GENERIC_FIFO_U32, generic_fifo_u32_device, "generic_fifo_u32_device", "Generic fifo, u32 values")

