// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    device.h

    Device interface functions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DEVICE_IPP
#define MAME_EMU_DEVICE_IPP

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<void (u32)> clock_update_delegate;


//**************************************************************************
//  MEMBER TEMPLATES
//**************************************************************************

namespace emu { namespace detail {

template <class DeviceClass> template <typename... Params>
inline DeviceClass &device_type_impl<DeviceClass>::operator()(machine_config &mconfig, char const *tag, Params &&... args) const
{
	return dynamic_cast<DeviceClass &>(*mconfig.device_add(tag, *this, std::forward<Params>(args)...));
}

template <class DeviceClass> template <typename Exposed, bool Required, typename... Params>
inline DeviceClass &device_type_impl<DeviceClass>::operator()(machine_config &mconfig, device_finder<Exposed, Required> &finder, Params &&... args) const
{
	std::pair<device_t &, char const *> const target(finder.finder_target());
	assert(&mconfig.current_device() == &target.first);
	DeviceClass &result(dynamic_cast<DeviceClass &>(*mconfig.device_add(target.second, *this, std::forward<Params>(args)...)));
	return finder = result;
}

template <class DeviceClass> template <typename... Params>
inline DeviceClass &device_type_impl<DeviceClass>::operator()(machine_config_replace replace, char const *tag, Params &&... args) const
{
	return dynamic_cast<DeviceClass &>(*replace.config.device_replace(tag, *this, std::forward<Params>(args)...));
}

template <class DeviceClass> template <typename Exposed, bool Required, typename... Params>
inline DeviceClass &device_type_impl<DeviceClass>::operator()(machine_config_replace replace, device_finder<Exposed, Required> &finder, Params &&... args) const
{
	std::pair<device_t &, char const *> const target(finder.finder_target());
	assert(&replace.config.current_device() == &target.first);
	DeviceClass &result(dynamic_cast<DeviceClass &>(*replace.config.device_replace(target.second, *this, std::forward<Params>(args)...)));
	return finder = result;
}

} } // namespace emu::detail


template <typename Format, typename... Params>
inline void device_t::popmessage(Format &&fmt, Params &&... args) const
{
	if (m_machine != nullptr)
		m_machine->popmessage(std::forward<Format>(fmt), std::forward<Params>(args)...);
}

template <typename Format, typename... Params>
inline void device_t::logerror(Format &&fmt, Params &&... args) const
{
	if (m_machine != nullptr && m_machine->allow_logging())
	{
		g_profiler.start(PROFILER_LOGERROR);

		// dump to the buffer
		m_string_buffer.clear();
		m_string_buffer.seekp(0);
		util::stream_format(m_string_buffer, "[%s] ", tag());
		util::stream_format(m_string_buffer, std::forward<Format>(fmt), std::forward<Params>(args)...);
		m_string_buffer.put('\0');

		m_machine->strlog(&m_string_buffer.vec()[0]);

		g_profiler.stop();
	}
}

template <typename T, typename Ret, typename... Params>
inline std::enable_if_t<device_memory_interface::is_related_class<device_t, T>::value> device_memory_interface::set_addrmap(int spacenum, Ret (T::*func)(Params... args))
{
	device_t &dev(device().mconfig().current_device());
	set_addrmap(spacenum, address_map_constructor(func, dev.tag(), &downcast<T &>(dev)));
}

template <typename T, typename Ret, typename... Params>
inline std::enable_if_t<!device_memory_interface::is_related_class<device_t, T>::value> device_memory_interface::set_addrmap(int spacenum, Ret (T::*func)(Params... args))
{
	device_t &dev(device().mconfig().current_device());
	set_addrmap(spacenum, address_map_constructor(func, dev.tag(), &dynamic_cast<T &>(dev)));
}

#endif // MAME_EMU_DEVICE_IPP
