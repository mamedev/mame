// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/// \file
/// \brief Simple broadcast notifier
///
/// Classes for managing subscriptions and calling multiple listener
/// functions for notifications.
#ifndef MAME_LIB_UTIL_NOTIFIER_H
#define MAME_LIB_UTIL_NOTIFIER_H

#pragma once

#include "delegate.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>


namespace util {

/// \brief A subscription to a notification source
///
/// Class for representing a subscription to a notifier.  Automatically
/// unsubscribes on destruction, if explicitly reset, if assigned to, or
/// when taking ownership of a different subscription by move
/// assignment.
/// \sa notifier
class notifier_subscription
{
public:
	/// \brief Create an empty subscription
	///
	/// Initialises an instance not referring to a subscription.
	notifier_subscription() noexcept : m_token(), m_live(nullptr), m_index(0U) { }

	/// \brief Transfer ownership of a subscription
	///
	/// Transfers ownership of a subscription to a new instance.
	/// \param [in,out] that The subscription to transfer ownership
	///   from.  Will no longer refer to a subscription after ownership
	///   is transferred away.
	notifier_subscription(notifier_subscription &&that) noexcept :
		m_token(std::move(that.m_token)),
		m_live(that.m_live),
		m_index(that.m_index)
	{
		that.m_token.reset();
	}

	/// \brief Unsubscribe and destroy a subscription
	///
	/// Unsubscribes if the subscription is active and cleans up the
	/// subscription instance.
	~notifier_subscription() noexcept
	{
		auto token(m_token.lock());
		if (token)
			(*m_live)[m_index] = false;
	}

	/// \brief Swap two subscriptions
	///
	/// Exchanges ownership of subscriptions between two instances.
	/// \param [in,out] that The subscription to exchange ownership
	///   with.
	void swap(notifier_subscription &that) noexcept
	{
		using std::swap;
		swap(m_token, that.m_token);
		swap(m_live, that.m_live);
		swap(m_index, that.m_index);
	}

	/// \brief Unsubscribe from notifications
	///
	/// If the instance refers to an active subscription, cancel it so
	/// no future notifications will be received.
	void reset() noexcept
	{
		auto token(m_token.lock());
		if (token)
			(*m_live)[m_index] = false;
		m_token.reset();
	}

	/// \brief Test whether a subscription is active
	///
	/// Tests whether a subscription is active.  A subscription will be
	/// inactive if it is default constructed, reset, transferred away,
	/// or if the underlying notifier is destructed.
	/// \return True if the subscription is active, false otherwise.
	explicit operator bool() const noexcept { return !m_token.expired(); }

	/// \brief Transfer ownership of a subscription
	///
	/// Transfers ownership of a subscription to an existing instance.
	/// If the subscription is active, it will be cancelled before it
	/// takes ownership of the other subscription.
	/// \param [in,out] that The subscription to transfer ownership
	///   from.  Will no longer refer to a subscription after ownership
	///   is transferred away.
	/// \return A reference to the instance that ownership was
	///   transferred to.
	notifier_subscription &operator=(notifier_subscription &&that) noexcept
	{
		{
			auto token(m_token.lock());
			if (token)
				(*m_live)[m_index] = false;
		}
		m_token = std::move(that.m_token);
		m_live = that.m_live;
		m_index = that.m_index;
		that.m_token.reset();
		return *this;
	}

protected:
	notifier_subscription(
			std::shared_ptr<int> const &token,
			std::vector<bool> &live,
			std::vector<bool>::size_type index) :
		m_token(token),
		m_live(&live),
		m_index(index)
	{
	}

private:
	notifier_subscription(notifier_subscription const &) = delete;
	notifier_subscription &operator=(notifier_subscription const &) = delete;

	std::weak_ptr<int> m_token;
	std::vector<bool> *m_live;
	std::vector<bool>::size_type m_index;
};


/// \brief Broadcast notifier
///
/// Calls multiple listener functions.  Allows listeners to subscribe
/// and unsubscribe.  Subscriptions are managed using
/// \c notifier_subscription instances.
/// \tparam Params Argument types for the listener functions.
/// \sa notifier_subscription
template <typename... Params>
class notifier
{
public:
	/// \brief Listener delegate type
	///
	/// The delegate type used to represent listener functions.
	using delegate_type = delegate<void (Params...)>;

	/// \brief Create a new notifier
	///
	/// Creates a new notifier with no initial subscribers.
	notifier() : m_token(std::make_shared<int>(0)) { }

	/// \brief Destroy a notifier
	///
	/// Destroys a notifier, causing any subscriptions to become
	/// inactive.
	~notifier() noexcept { m_token.reset(); }

	/// \brief Add a listener
	///
	/// Adds a listener function subscription, returning an object for
	/// managing the subscription.
	/// \param [in] listener The function to be called on notifications.
	/// \return A subscription object.  Destroy the object or call its
	///   \c reset member function to unsubscribe.
	notifier_subscription subscribe(delegate_type &&listener)
	{
		struct subscription_impl : notifier_subscription
		{
			subscription_impl(notifier &host, std::size_t index) noexcept :
				notifier_subscription(host.m_token, host.m_live, index)
			{
			}
		};
		for (std::size_t i = 0U; m_listeners.size() > i; ++i)
		{
			if (!m_live[i])
			{
				m_live[i] = true;
				m_listeners[i] = std::move(listener);
				return subscription_impl(*this, i);
			}
		}
		m_live.emplace_back(true);
		try
		{
			m_listeners.emplace_back(std::move(listener));
		}
		catch (...)
		{
			m_live.pop_back();
			throw;
		}
		return subscription_impl(*this, m_listeners.size() - 1);
	}

	/// \brief Call listeners
	///
	/// Calls all active listener functions.
	/// \param [in] args Arguments to pass to the listener functions.
	void operator()(Params... args) const
	{
		for (std::size_t i = 0U; m_listeners.size() > i; ++i)
		{
			if (m_live[i])
				m_listeners[i](args...);
		}
	}

private:
	notifier(notifier const &) = delete;
	notifier &operator=(notifier const &) = delete;

	std::shared_ptr<int> m_token;
	std::vector<bool> m_live;
	std::vector<delegate_type> m_listeners;
};


/// \brief Swap two notifier subscriptions
///
/// Exchanges ownership of two notifier subscriptions.  Allows the
/// swappable idiom to be used with notifier subscriptions.
/// \param [in,out] x Takes ownership of the subscription from \p y.
/// \param [in,out] y Takes ownership of the subscription from \p x.
/// \sa notifier_subscription
inline void swap(notifier_subscription &x, notifier_subscription &y) noexcept { x.swap(y); }

} // namespace util

#endif // MAME_LIB_UTIL_NOTIFIER_H
