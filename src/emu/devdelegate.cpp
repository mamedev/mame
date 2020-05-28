// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    devdelegate.c

    Delegates that are late-bound to MAME devices.

***************************************************************************/

#include "emu.h"
#include "devdelegate.h"


namespace emu { namespace detail {

delegate_late_bind &device_delegate_helper::bound_object() const
{
	if (!m_tag)
		return m_base.get();
	device_t *const device(m_base.get().subdevice(m_tag));
	if (!device)
		throw emu_fatalerror("Unable to locate device '%s' relative to '%s'\n", m_tag, m_base.get().tag());
	return *device;
}


void device_delegate_helper::set_tag(char const *tag)
{
	m_base = m_base.get().mconfig().current_device();
	m_tag = tag;
}

} } // namespace emu::detail
