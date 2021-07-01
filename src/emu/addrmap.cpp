// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    addrmap.cpp

    Macros and helper functions for handling address map definitions.

***************************************************************************/

#include "emu.h"
#include "romload.h"
#include "validity.h"


//**************************************************************************
//  PARAMETERS
//**************************************************************************

#define DETECT_OVERLAPPING_MEMORY   (0)

//**************************************************************************
//  ADDRESS MAP ENTRY
//**************************************************************************

//-------------------------------------------------
//  address_map_entry - constructor
//-------------------------------------------------

address_map_entry::address_map_entry(device_t &device, address_map &map, offs_t start, offs_t end)
	: m_next(nullptr)
	, m_map(map)
	, m_devbase(device)
	, m_addrstart(start)
	, m_addrend(end)
	, m_addrmirror(0)
	, m_addrmask(0)
	, m_addrselect(0)
	, m_mask(0)
	, m_cswidth(0)
	, m_share(nullptr)
	, m_region(nullptr)
	, m_rgnoffs(0)
	, m_rproto8(device)
	, m_rproto16(device)
	, m_rproto32(device)
	, m_rproto64(device)
	, m_wproto8(device)
	, m_wproto16(device)
	, m_wproto32(device)
	, m_wproto64(device)
	, m_rproto8m(device)
	, m_rproto16m(device)
	, m_rproto32m(device)
	, m_rproto64m(device)
	, m_wproto8m(device)
	, m_wproto16m(device)
	, m_wproto32m(device)
	, m_wproto64m(device)
	, m_rproto8s(device)
	, m_rproto16s(device)
	, m_rproto32s(device)
	, m_rproto64s(device)
	, m_wproto8s(device)
	, m_wproto16s(device)
	, m_wproto32s(device)
	, m_wproto64s(device)
	, m_rproto8sm(device)
	, m_rproto16sm(device)
	, m_rproto32sm(device)
	, m_rproto64sm(device)
	, m_wproto8sm(device)
	, m_wproto16sm(device)
	, m_wproto32sm(device)
	, m_wproto64sm(device)
	, m_rproto8mo(device)
	, m_rproto16mo(device)
	, m_rproto32mo(device)
	, m_rproto64mo(device)
	, m_wproto8mo(device)
	, m_wproto16mo(device)
	, m_wproto32mo(device)
	, m_wproto64mo(device)
	, m_rproto8smo(device)
	, m_rproto16smo(device)
	, m_rproto32smo(device)
	, m_rproto64smo(device)
	, m_wproto8smo(device)
	, m_wproto16smo(device)
	, m_wproto32smo(device)
	, m_wproto64smo(device)
	, m_submap_device(nullptr)
	, m_memory(nullptr)
{
}


//-------------------------------------------------
//  mask - set the address mask value
//-------------------------------------------------

address_map_entry &address_map_entry::mask(offs_t _mask)
{
	m_addrmask = _mask;
	if (m_map.m_globalmask != 0)
		m_addrmask &= m_map.m_globalmask;
	return *this;
}


//-------------------------------------------------
//  umask16 - set a 16-bits unitmask value
//-------------------------------------------------

address_map_entry &address_map_entry::umask16(u16 _mask)
{
	m_mask = (u64(_mask) << 48) | (u64(_mask) << 32) | (u64(_mask) << 16) | _mask;
	return *this;
}


//-------------------------------------------------
//  umask32 - set a 32-bits unitmask value
//-------------------------------------------------

address_map_entry &address_map_entry::umask32(u32 _mask)
{
	m_mask = (u64(_mask) << 32) | _mask;
	return *this;
}


//-------------------------------------------------
//  umask64 - set a 64-bits unitmask value
//-------------------------------------------------

address_map_entry &address_map_entry::umask64(u64 _mask)
{
	m_mask = _mask;
	return *this;
}


//-------------------------------------------------
//  m - set up a handler for
//  retrieve a submap from a device
//-------------------------------------------------

address_map_entry &address_map_entry::m(const char *tag, address_map_constructor func)
{
	m_read.m_type = AMH_DEVICE_SUBMAP;
	m_read.m_tag = tag;
	m_write.m_type = AMH_DEVICE_SUBMAP;
	m_write.m_tag = tag;
	m_submap_device = nullptr;
	m_submap_delegate = func;
	return *this;
}

address_map_entry &address_map_entry::m(device_t *device, address_map_constructor func)
{
	m_read.m_type = AMH_DEVICE_SUBMAP;
	m_read.m_tag = nullptr;
	m_write.m_type = AMH_DEVICE_SUBMAP;
	m_write.m_tag = nullptr;
	m_submap_device = device;
	m_submap_delegate = func;
	return *this;
}

void address_map_entry::view(memory_view &mv)
{
	m_read.m_type = AMH_VIEW;
	m_read.m_tag = nullptr;
	m_write.m_type = AMH_VIEW;
	m_write.m_tag = nullptr;
	m_view = &mv;
	mv.initialize_from_address_map(m_addrstart, m_addrend, m_map.get_config());
}

//-------------------------------------------------
//  r/w/rw - handler setters for
//  8-bit read/write delegates
//-------------------------------------------------

address_map_entry &address_map_entry::r(read8_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE;
	m_read.m_bits = 8;
	m_read.m_name = func.name();
	m_rproto8 = func;
	return *this;
}

address_map_entry &address_map_entry::w(write8_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE;
	m_write.m_bits = 8;
	m_write.m_name = func.name();
	m_wproto8 = func;
	return *this;
}

address_map_entry &address_map_entry::r(read8m_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_M;
	m_read.m_bits = 8;
	m_read.m_name = func.name();
	m_rproto8m = func;
	return *this;
}

address_map_entry &address_map_entry::w(write8m_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_M;
	m_write.m_bits = 8;
	m_write.m_name = func.name();
	m_wproto8m = func;
	return *this;
}

address_map_entry &address_map_entry::r(read8s_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_S;
	m_read.m_bits = 8;
	m_read.m_name = func.name();
	m_rproto8s = func;
	return *this;
}

address_map_entry &address_map_entry::w(write8s_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_S;
	m_write.m_bits = 8;
	m_write.m_name = func.name();
	m_wproto8s = func;
	return *this;
}

address_map_entry &address_map_entry::r(read8sm_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_SM;
	m_read.m_bits = 8;
	m_read.m_name = func.name();
	m_rproto8sm = func;
	return *this;
}

address_map_entry &address_map_entry::w(write8sm_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_SM;
	m_write.m_bits = 8;
	m_write.m_name = func.name();
	m_wproto8sm = func;
	return *this;
}

address_map_entry &address_map_entry::r(read8mo_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_MO;
	m_read.m_bits = 8;
	m_read.m_name = func.name();
	m_rproto8mo = func;
	return *this;
}

address_map_entry &address_map_entry::w(write8mo_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_MO;
	m_write.m_bits = 8;
	m_write.m_name = func.name();
	m_wproto8mo = func;
	return *this;
}

address_map_entry &address_map_entry::r(read8smo_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_SMO;
	m_read.m_bits = 8;
	m_read.m_name = func.name();
	m_rproto8smo = func;
	return *this;
}

address_map_entry &address_map_entry::w(write8smo_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_SMO;
	m_write.m_bits = 8;
	m_write.m_name = func.name();
	m_wproto8smo = func;
	return *this;
}


//-------------------------------------------------
//  r/w/rw - handler setters for
//  16-bit read/write delegates
//-------------------------------------------------

address_map_entry &address_map_entry::r(read16_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE;
	m_read.m_bits = 16;
	m_read.m_name = func.name();
	m_rproto16 = func;
	return *this;
}

address_map_entry &address_map_entry::w(write16_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE;
	m_write.m_bits = 16;
	m_write.m_name = func.name();
	m_wproto16 = func;
	return *this;
}

address_map_entry &address_map_entry::r(read16m_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_M;
	m_read.m_bits = 16;
	m_read.m_name = func.name();
	m_rproto16m = func;
	return *this;
}

address_map_entry &address_map_entry::w(write16m_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_M;
	m_write.m_bits = 16;
	m_write.m_name = func.name();
	m_wproto16m = func;
	return *this;
}

address_map_entry &address_map_entry::r(read16s_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_S;
	m_read.m_bits = 16;
	m_read.m_name = func.name();
	m_rproto16s = func;
	return *this;
}

address_map_entry &address_map_entry::w(write16s_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_S;
	m_write.m_bits = 16;
	m_write.m_name = func.name();
	m_wproto16s = func;
	return *this;
}

address_map_entry &address_map_entry::r(read16sm_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_SM;
	m_read.m_bits = 16;
	m_read.m_name = func.name();
	m_rproto16sm = func;
	return *this;
}

address_map_entry &address_map_entry::w(write16sm_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_SM;
	m_write.m_bits = 16;
	m_write.m_name = func.name();
	m_wproto16sm = func;
	return *this;
}

address_map_entry &address_map_entry::r(read16mo_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_MO;
	m_read.m_bits = 16;
	m_read.m_name = func.name();
	m_rproto16mo = func;
	return *this;
}

address_map_entry &address_map_entry::w(write16mo_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_MO;
	m_write.m_bits = 16;
	m_write.m_name = func.name();
	m_wproto16mo = func;
	return *this;
}

address_map_entry &address_map_entry::r(read16smo_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_SMO;
	m_read.m_bits = 16;
	m_read.m_name = func.name();
	m_rproto16smo = func;
	return *this;
}

address_map_entry &address_map_entry::w(write16smo_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_SMO;
	m_write.m_bits = 16;
	m_write.m_name = func.name();
	m_wproto16smo = func;
	return *this;
}


//-------------------------------------------------
//  r/w/rw - handler setters for
//  32-bit read/write delegates
//-------------------------------------------------

address_map_entry &address_map_entry::r(read32_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE;
	m_read.m_bits = 32;
	m_read.m_name = func.name();
	m_rproto32 = func;
	return *this;
}

address_map_entry &address_map_entry::w(write32_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE;
	m_write.m_bits = 32;
	m_write.m_name = func.name();
	m_wproto32 = func;
	return *this;
}

address_map_entry &address_map_entry::r(read32m_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_M;
	m_read.m_bits = 32;
	m_read.m_name = func.name();
	m_rproto32m = func;
	return *this;
}

address_map_entry &address_map_entry::w(write32m_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_M;
	m_write.m_bits = 32;
	m_write.m_name = func.name();
	m_wproto32m = func;
	return *this;
}

address_map_entry &address_map_entry::r(read32s_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_S;
	m_read.m_bits = 32;
	m_read.m_name = func.name();
	m_rproto32s = func;
	return *this;
}

address_map_entry &address_map_entry::w(write32s_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_S;
	m_write.m_bits = 32;
	m_write.m_name = func.name();
	m_wproto32s = func;
	return *this;
}

address_map_entry &address_map_entry::r(read32sm_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_SM;
	m_read.m_bits = 32;
	m_read.m_name = func.name();
	m_rproto32sm = func;
	return *this;
}

address_map_entry &address_map_entry::w(write32sm_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_SM;
	m_write.m_bits = 32;
	m_write.m_name = func.name();
	m_wproto32sm = func;
	return *this;
}

address_map_entry &address_map_entry::r(read32mo_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_MO;
	m_read.m_bits = 32;
	m_read.m_name = func.name();
	m_rproto32mo = func;
	return *this;
}

address_map_entry &address_map_entry::w(write32mo_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_MO;
	m_write.m_bits = 32;
	m_write.m_name = func.name();
	m_wproto32mo = func;
	return *this;
}

address_map_entry &address_map_entry::r(read32smo_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_SMO;
	m_read.m_bits = 32;
	m_read.m_name = func.name();
	m_rproto32smo = func;
	return *this;
}

address_map_entry &address_map_entry::w(write32smo_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_SMO;
	m_write.m_bits = 32;
	m_write.m_name = func.name();
	m_wproto32smo = func;
	return *this;
}

//-------------------------------------------------
//  r/w/rw - handler setters for
//  64-bit read/write delegates
//-------------------------------------------------

address_map_entry &address_map_entry::r(read64_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE;
	m_read.m_bits = 64;
	m_read.m_name = func.name();
	m_rproto64 = func;
	return *this;
}

address_map_entry &address_map_entry::w(write64_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE;
	m_write.m_bits = 64;
	m_write.m_name = func.name();
	m_wproto64 = func;
	return *this;
}

address_map_entry &address_map_entry::r(read64m_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_M;
	m_read.m_bits = 64;
	m_read.m_name = func.name();
	m_rproto64m = func;
	return *this;
}

address_map_entry &address_map_entry::w(write64m_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_M;
	m_write.m_bits = 64;
	m_write.m_name = func.name();
	m_wproto64m = func;
	return *this;
}

address_map_entry &address_map_entry::r(read64s_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_S;
	m_read.m_bits = 64;
	m_read.m_name = func.name();
	m_rproto64s = func;
	return *this;
}

address_map_entry &address_map_entry::w(write64s_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_S;
	m_write.m_bits = 64;
	m_write.m_name = func.name();
	m_wproto64s = func;
	return *this;
}

address_map_entry &address_map_entry::r(read64sm_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_SM;
	m_read.m_bits = 64;
	m_read.m_name = func.name();
	m_rproto64sm = func;
	return *this;
}

address_map_entry &address_map_entry::w(write64sm_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_SM;
	m_write.m_bits = 64;
	m_write.m_name = func.name();
	m_wproto64sm = func;
	return *this;
}

address_map_entry &address_map_entry::r(read64mo_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_MO;
	m_read.m_bits = 64;
	m_read.m_name = func.name();
	m_rproto64mo = func;
	return *this;
}

address_map_entry &address_map_entry::w(write64mo_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_MO;
	m_write.m_bits = 64;
	m_write.m_name = func.name();
	m_wproto64mo = func;
	return *this;
}

address_map_entry &address_map_entry::r(read64smo_delegate func)
{
	assert(!func.isnull());
	m_read.m_type = AMH_DEVICE_DELEGATE_SMO;
	m_read.m_bits = 64;
	m_read.m_name = func.name();
	m_rproto64smo = func;
	return *this;
}

address_map_entry &address_map_entry::w(write64smo_delegate func)
{
	assert(!func.isnull());
	m_write.m_type = AMH_DEVICE_DELEGATE_SMO;
	m_write.m_bits = 64;
	m_write.m_name = func.name();
	m_wproto64smo = func;
	return *this;
}


//-------------------------------------------------
//  unitmask_is_appropriate - verify that the
//  provided unitmask is valid and expected
//-------------------------------------------------

bool address_map_entry::unitmask_is_appropriate(u8 width, u64 unitmask, const char *string) const
{
#if 0
	// if no mask, this must match the default width of the map
	if (unitmask == 0)
	{
		if (m_map.m_databits != width)
		{
			osd_printf_error("Handler %s is a %d-bit handler but was specified in a %d-bit address map\n", string, width, m_map.m_databits);
			return false;
		}
		return true;
	}

	// if we have a mask, we must be smaller than the default width of the map
	if (m_map.m_databits < width)
	{
		osd_printf_error("Handler %s is a %d-bit handler and is too wide to be used in a %d-bit address map\n", string, width, m_map.m_databits);
		return false;
	}

	// if map is narrower than 64 bits, check the mask width as well
	if (m_map.m_databits < 64 && (unitmask >> m_map.m_databits) != 0)
	{
		osd_printf_error("Handler %s specified a mask of %016X, too wide to be used in a %d-bit address map\n", string, unitmask, m_map.m_databits);
		return false;
	}

	// the mask must represent whole units of width
	u32 basemask = (width == 8) ? 0xff : (width == 16) ? 0xffff : 0xffffffff;
	u64 singlemask = basemask;
	int count = 0;
	while (singlemask != 0)
	{
		if ((unitmask & singlemask) == singlemask)
			count++;
		else if ((unitmask & singlemask) != 0)
		{
			osd_printf_error("Handler %s specified a mask of %016X; needs to be in even chunks of %X\n", string, unitmask, basemask);
			return false;
		}
		singlemask <<= width;
	}

#if 0
	// the mask must be symmetrical
	u64 unitmask_bh = unitmask >> 8 & 0x00ff00ff00ff00ffU;
	u64 unitmask_bl = unitmask & 0x00ff00ff00ff00ffU;
	u64 unitmask_wh = unitmask >> 16 & 0x0000ffff0000ffffU;
	u64 unitmask_wl = unitmask & 0x0000ffff0000ffffU;
	u64 unitmask_dh = unitmask >> 32 & 0x00000000ffffffffU;
	u64 unitmask_dl = unitmask & 0x00000000ffffffffU;
	if ((unitmask_bh != 0 && unitmask_bl != 0 && unitmask_bh != unitmask_bl)
		|| (unitmask_wh != 0 && unitmask_wl != 0 && unitmask_wh != unitmask_wl)
		|| (unitmask_dh != 0 && unitmask_dl != 0 && unitmask_dh != unitmask_dl))
	{
		osd_printf_error("Handler %s specified an asymmetrical mask of %016X\n", string, unitmask);
		return false;
	}
#endif
#endif

	return true;
}


//**************************************************************************
//  ADDRESS MAP
//**************************************************************************

//-------------------------------------------------
//  address_map - constructor
//-------------------------------------------------

address_map::address_map(device_t &device, int spacenum)
	: m_spacenum(spacenum),
		m_device(&device),
		m_view(nullptr),
		m_unmapval(0),
		m_globalmask(0)
{
	// get our memory interface
	const device_memory_interface *memintf;
	if (!m_device->interface(memintf))
		throw emu_fatalerror("No memory interface defined for device '%s'\n", m_device->tag());

	// and then the configuration for the current address space
	m_config = memintf->space_config(spacenum);
	if (m_config == nullptr)
		throw emu_fatalerror("No memory address space configuration found for device '%s', space %d\n", m_device->tag(), spacenum);

	// append the map provided by the owner
	if (!memintf->get_addrmap(spacenum).isnull())
	{
		m_device = device.owner();
		memintf->get_addrmap(spacenum)(*this);
		m_device = &device;
	}

	// construct the internal device map (last so it takes priority)
	if (!m_config->m_internal_map.isnull())
		m_config->m_internal_map(*this);
}



//-------------------------------------------------
//  address_map - constructor in the submap case
//-------------------------------------------------

address_map::address_map(device_t &device, address_map_entry *entry)
	: m_spacenum(AS_PROGRAM),
		m_device(&device),
		m_view(nullptr),
		m_config(entry->m_map.m_config),
		m_unmapval(0),
		m_globalmask(0)
{
	// Retrieve the submap
	entry->m_submap_delegate.late_bind(*m_device);
	entry->m_submap_delegate(*this);
}



//----------------------------------------------------------
//  address_map - constructor dynamic device mapping case
//----------------------------------------------------------

address_map::address_map(const address_space &space, offs_t start, offs_t end, u64 unitmask, int cswidth, device_t &device, address_map_constructor submap_delegate)
	: m_spacenum(space.spacenum()),
		m_device(&device),
		m_view(nullptr),
		m_config(&space.space_config()),
		m_unmapval(space.unmap()),
		m_globalmask(space.addrmask())
{
	(*this)(start, end).m(DEVICE_SELF, submap_delegate).umask64(unitmask).cswidth(cswidth);
}


//----------------------------------------------------------
//  address_map - constructor memory view mapping case
//----------------------------------------------------------

address_map::address_map(memory_view &view)
	: m_spacenum(-1),
	  m_device(&view.m_device),
	  m_view(&view),
	  m_config(view.m_config),
	  m_unmapval(0),
	  m_globalmask(0)
{
}


//-------------------------------------------------
//  ~address_map - destructor
//-------------------------------------------------

address_map::~address_map()
{
}


//-------------------------------------------------
//  append - append an entry to the end of the
//  list
//-------------------------------------------------

void address_map::global_mask(offs_t mask)
{
	m_globalmask = mask;
}



//-------------------------------------------------
//  add - add a new entry
//-------------------------------------------------

address_map_entry &address_map::operator()(offs_t start, offs_t end)
{
	address_map_entry *ptr = new address_map_entry(*m_device, *this, start, end);
	m_entrylist.append(*ptr);
	return *ptr;
}


//-------------------------------------------------
//  import_submaps - propagate in the device submaps
//-------------------------------------------------

void address_map::import_submaps(running_machine &machine, device_t &owner, int data_width, endianness_t endian, int addr_shift)
{
	address_map_entry *prev = nullptr;
	address_map_entry *entry = m_entrylist.first();
	u64 base_unitmask = (~u64(0)) >> (64 - data_width);

	while (entry)
	{
		if (entry->m_read.m_type == AMH_DEVICE_SUBMAP)
		{
			device_t *mapdevice = entry->m_submap_device;
			if (!mapdevice)
			{
				mapdevice = owner.subdevice(entry->m_read.m_tag);
				if (mapdevice == nullptr)
					throw emu_fatalerror("Attempted to submap a non-existent device '%s' in space %d of device '%s'\n", owner.subtag(entry->m_read.m_tag), m_spacenum, m_device->basetag());
			}

			// Grab the submap
			address_map submap(*mapdevice, entry);

			// Recursively import if needed
			submap.import_submaps(machine, *mapdevice, data_width, endian, addr_shift);

			offs_t max_end = entry->m_addrend - entry->m_addrstart;

			if(!entry->m_mask || (entry->m_mask & base_unitmask) == base_unitmask)
			{
				// Easy case, no unitmask at mapping level - Merge in all the map contents in order
				while (submap.m_entrylist.count())
				{
					address_map_entry *subentry = submap.m_entrylist.detach_head();

					if (addr_shift > 0)
					{
						subentry->m_addrstart <<= addr_shift;
						subentry->m_addrend = ((subentry->m_addrend + 1) << addr_shift) - 1;
						subentry->m_addrmirror <<= addr_shift;
						subentry->m_addrmask <<= addr_shift;
						subentry->m_addrselect <<= addr_shift;
					}
					else if (addr_shift < 0)
					{
						subentry->m_addrstart >>= -addr_shift;
						subentry->m_addrend >>= -addr_shift;
						subentry->m_addrmirror >>= -addr_shift;
						subentry->m_addrmask >>= -addr_shift;
						subentry->m_addrselect >>= -addr_shift;
					}

					if (subentry->m_addrend > max_end)
						subentry->m_addrend = max_end;

					subentry->m_addrstart += entry->m_addrstart;
					subentry->m_addrend += entry->m_addrstart;
					subentry->m_addrmirror |= entry->m_addrmirror;
					subentry->m_addrmask |= entry->m_addrmask;
					subentry->m_addrselect |= entry->m_addrselect;

					if (subentry->m_addrstart > entry->m_addrend)
					{
						delete subentry;
						continue;
					}

					// Insert the entry in the map
					m_entrylist.insert_after(*subentry, prev);
					prev = subentry;
				}
			}
			else
			{
				// There is a unitmask, calculate its ratio
				int ratio = 0;
				for (int i=0; i != data_width; i++)
					if ((entry->m_mask >> i) & 1)
						ratio ++;
				ratio = data_width / ratio;
				if (addr_shift > 0)
					ratio <<= addr_shift;
				else if (addr_shift < 0)
					max_end = ((max_end + 1) << -addr_shift) - 1;
				max_end = (max_end + 1) / ratio - 1;

				// Then merge the contents taking the ratio into account
				while (submap.m_entrylist.count())
				{
					address_map_entry *subentry = submap.m_entrylist.detach_head();

					if (subentry->m_mask && (subentry->m_mask != 0xffffffffffffffffU))
					{
						// Check if the mask can actually fit
						int subentry_ratio = 0;
						for (int i=0; i != data_width; i++)
							if ((subentry->m_mask >> i) & 1)
								subentry_ratio ++;
						subentry_ratio = data_width / subentry_ratio;
						if (ratio * subentry_ratio > data_width / 8)
							fatalerror("import_submap: In range %x-%x mask %x mirror %x select %x of device %s, the import unitmask of %0*x combined with an entry unitmask of %0*x does not fit in %d bits.\n", subentry->m_addrstart, subentry->m_addrend, subentry->m_addrmask, subentry->m_addrmirror, subentry->m_addrselect, entry->m_read.m_tag, data_width/4, entry->m_mask, data_width/4, subentry->m_mask, data_width);

						// Regenerate the unitmask
						u64 newmask = 0;
						int bit_in_subentry = 0;
						for (int i=0; i != data_width; i++)
							if ((entry->m_mask >> i) & 1)
							{
								if ((subentry->m_mask >> bit_in_subentry) & 1)
									newmask |= u64(1) << i;
								bit_in_subentry ++;
							}
						subentry->m_mask = newmask;
					}
					else
						subentry->m_mask = entry->m_mask;

					subentry->m_cswidth = std::max(subentry->m_cswidth, entry->m_cswidth);

					if (subentry->m_addrend > max_end)
						subentry->m_addrend = max_end;

					if (addr_shift < 0)
					{
						subentry->m_addrstart = ((subentry->m_addrstart * ratio) >> -addr_shift) + entry->m_addrstart;
						subentry->m_addrend = (((subentry->m_addrend + 1) * ratio - 1) >> -addr_shift) + entry->m_addrstart;
						subentry->m_addrmirror = ((subentry->m_addrmirror / ratio) << -addr_shift) | entry->m_addrmirror;
						subentry->m_addrmask = ((subentry->m_addrmask / ratio) << -addr_shift) | entry->m_addrmask;
						subentry->m_addrselect = ((subentry->m_addrselect / ratio) << -addr_shift) | entry->m_addrselect;
					}
					else
					{
						subentry->m_addrstart = subentry->m_addrstart * ratio + entry->m_addrstart;
						subentry->m_addrend = (subentry->m_addrend + 1) * ratio - 1 + entry->m_addrstart;
						subentry->m_addrmirror = (subentry->m_addrmirror / ratio) | entry->m_addrmirror;
						subentry->m_addrmask = (subentry->m_addrmask / ratio) | entry->m_addrmask;
						subentry->m_addrselect = (subentry->m_addrselect / ratio) | entry->m_addrselect;
					}

					if (subentry->m_addrstart > entry->m_addrend)
					{
						delete subentry;
						continue;
					}

					// Insert the entry in the map
					m_entrylist.insert_after(*subentry, prev);
					prev = subentry;
				}
			}

			address_map_entry *to_delete = entry;
			entry = entry->next();
			m_entrylist.remove(*to_delete);
		}
		else
		{
			prev = entry;
			entry = entry->next();
		}
	}
}


//-------------------------------------------------
//  map_validity_check - perform validity checks on
//  one of the device's address maps
//-------------------------------------------------

void address_map::map_validity_check(validity_checker &valid, int spacenum) const
{
	// it's safe to assume here that the device has a memory interface and a config for this space
	const address_space_config &spaceconfig = *m_device->memory().space_config(spacenum);
	int default_alignunit = spaceconfig.alignment();

	bool detected_overlap = DETECT_OVERLAPPING_MEMORY ? false : true;

	// if this is an empty map, just ignore it
	if (m_entrylist.first() == nullptr)
		return;

	// validate the global map parameters
	if (m_spacenum != spacenum)
		osd_printf_error("Space %d has address space %d handlers!\n", spacenum, m_spacenum);

	offs_t globalmask = 0xffffffffUL >> (32 - spaceconfig.m_addr_width);
	if (m_globalmask != 0)
		globalmask = m_globalmask;

	// loop over entries and look for errors
	for (address_map_entry &entry : m_entrylist)
	{
		// look for overlapping entries
		if (!detected_overlap)
		{
			for (address_map_entry &scan : m_entrylist)
			{
				if (&scan == &entry)
					break;
				if (entry.m_addrstart <= scan.m_addrend && entry.m_addrend >= scan.m_addrstart &&
					((entry.m_read.m_type != AMH_NONE && scan.m_read.m_type != AMH_NONE) ||
						(entry.m_write.m_type != AMH_NONE && scan.m_write.m_type != AMH_NONE)))
				{
					osd_printf_warning("%s space has overlapping memory (%X-%X,%d,%d) vs (%X-%X,%d,%d)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_read.m_type, entry.m_write.m_type, scan.m_addrstart, scan.m_addrend, scan.m_read.m_type, scan.m_write.m_type);
					detected_overlap = true;
					break;
				}
			}
		}

		// look for inverted start/end pairs
		if (entry.m_addrend < entry.m_addrstart)
			osd_printf_error("Wrong %s memory read handler start = %08x > end = %08x\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend);

		// look for ranges outside the global mask
		if (entry.m_addrstart & ~globalmask)
			osd_printf_error("In %s memory range %x-%x, start address is outside of the global address mask %x\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, globalmask);
		if (entry.m_addrend & ~globalmask)
			osd_printf_error("In %s memory range %x-%x, end address is outside of the global address mask %x\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, globalmask);
		if (entry.m_addrmask & ~globalmask)
			osd_printf_error("In %s range %x-%x mask %x mirror %x select %x, mask is outside of the global address mask %x, did you mean %x ?\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, globalmask, entry.m_addrmask & globalmask);
		if ((entry.m_addrmirror & ~globalmask) && (entry.m_addrmirror | globalmask) != 0xffffffff >> (32 - spaceconfig.m_addr_width))
			osd_printf_error("In %s range %x-%x mask %x mirror %x select %x, mirror is outside of the global address mask %x, did you mean %x ?\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, globalmask, entry.m_addrmirror & globalmask);
		if (entry.m_addrselect & ~globalmask)
			osd_printf_error("In %s range %x-%x mask %x mirror %x select %x, select is outside of the global address mask %x, did you mean %x ?\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, globalmask, entry.m_addrselect & globalmask);

		// look for misaligned entries
		if (entry.m_read.m_type != AMH_NONE)
		{
			int alignunit = spaceconfig.byte2addr(entry.m_read.m_bits / 8);
			if (!alignunit)
				alignunit = default_alignunit;
			if ((entry.m_addrstart & (alignunit - 1)) != 0 || (entry.m_addrend & (alignunit - 1)) != (alignunit - 1))
				osd_printf_error("Wrong %s memory read handler start = %08x, end = %08x ALIGN = %d\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, alignunit);
		}
		if (entry.m_write.m_type != AMH_NONE)
		{
			int alignunit = spaceconfig.byte2addr(entry.m_write.m_bits / 8);
			if (!alignunit)
				alignunit = default_alignunit;
			if ((entry.m_addrstart & (alignunit - 1)) != 0 || (entry.m_addrend & (alignunit - 1)) != (alignunit - 1))
				osd_printf_error("Wrong %s memory write handler start = %08x, end = %08x ALIGN = %d\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, alignunit);
		}

		// verify mask/mirror/select
		offs_t set_bits = entry.m_addrstart | entry.m_addrend;
		offs_t changing_bits = entry.m_addrstart ^ entry.m_addrend;
		changing_bits |= changing_bits >> 1;
		changing_bits |= changing_bits >> 2;
		changing_bits |= changing_bits >> 4;
		changing_bits |= changing_bits >> 8;
		changing_bits |= changing_bits >> 16;

		if (entry.m_addrmask & ~changing_bits)
			osd_printf_error("In %s memory range %x-%x, mask %x is trying to unmask an unchanging address bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmask & ~changing_bits);
		if (entry.m_addrmirror & changing_bits)
			osd_printf_error("In %s memory range %x-%x, mirror %x touches a changing address bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_addrmirror & changing_bits);
		if (entry.m_addrselect & changing_bits)
			osd_printf_error("In %s memory range %x-%x, select %x touches a changing address bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrselect, entry.m_addrselect & changing_bits);
		if (entry.m_addrmirror & set_bits)
			osd_printf_error("In %s memory range %x-%x, mirror %x touches a set address bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_addrmirror & set_bits);
		if (entry.m_addrselect & set_bits)
			osd_printf_error("In %s memory range %x-%x, select %x touches a set address bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrselect, entry.m_addrselect & set_bits);
		if (entry.m_addrmirror & entry.m_addrselect)
			osd_printf_error("In %s memory range %x-%x, mirror %x touches a select bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_addrmirror & entry.m_addrselect);

		// if this is a program space, auto-assign implicit ROM entries
		if (entry.m_read.m_type == AMH_ROM && entry.m_region == nullptr && spacenum == AS_PROGRAM && entry.m_share == nullptr)
		{
			entry.m_region = m_device->tag();
			entry.m_rgnoffs = entry.m_addrstart;
		}

		// detect conflicts between region and share
		if (entry.m_region && entry.m_share)
			osd_printf_error("%s space memory map entry %X-%X has both .region() and .share()\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend);


		// Detect inaccesible memory.  Region automapping is handled before
		if (entry.m_read.m_type == AMH_ROM && entry.m_write.m_type != AMH_RAM &&
			!entry.m_share && !entry.m_region)
			osd_printf_error("%s space memory map entry %X-%X has .rom() but no .region() or .share()\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend);
		if (entry.m_read.m_type != AMH_RAM && entry.m_write.m_type == AMH_RAM &&
			!entry.m_share)
			osd_printf_error("%s space memory map entry %X-%X has .writeonly() but no .share()\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend);

		// if this entry references a memory region, validate it
		if (entry.m_region != nullptr)
		{
			// address map entries that reference regions but are NOPs are pointless
			if (entry.m_read.m_type != AMH_ROM && entry.m_read.m_type != AMH_RAM)
				osd_printf_error("%s space memory map entry %X-%X references memory region %s, but can't read it\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_region);
			if (entry.m_write.m_type == AMH_RAM)
				osd_printf_error("%s space memory map entry %X-%X references memory region %s, but wants to write to it\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_region);

			// make sure we can resolve the full path to the region
			bool found = false;
			std::string entry_region = entry.m_devbase.subtag(entry.m_region);

			// look for the region
			for (device_t &dev : device_enumerator(m_device->mconfig().root_device()))
			{
				for (romload::region const &region : romload::entries(dev.rom_region()).get_regions())
				{
					if (dev.subtag(region.get_tag()) == entry_region)
					{
						// verify the address range is within the region's bounds
						offs_t const length = region.get_length();
						if (entry.m_rgnoffs + spaceconfig.addr2byte(entry.m_addrend - entry.m_addrstart + 1) > length)
							osd_printf_error("%s space memory map entry %X-%X extends beyond region '%s' size (%X)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_region, length);

						device_t *rgndev = dev.subdevice(region.get_tag());
						if (rgndev != m_device)
						{
							int rgnwidth = region.get_width();
							bool rgnisbe = region.is_bigendian();

							// memory interface sets region width when tags are the same
							device_memory_interface *memintf;
							if (rgndev != nullptr && rgndev->interface(memintf))
							{
								const address_space_config *rgncfg = memintf->space_config(0);
								if (rgncfg != nullptr)
								{
									rgnwidth = rgncfg->data_width();
									rgnisbe = rgncfg->endianness() == ENDIANNESS_BIG;
								}
							}

							// verify data width
							if (rgnwidth != spaceconfig.data_width())
								osd_printf_error("%s space is %d-bit but references %d-bit region '%s'\n", spaceconfig.m_name, spaceconfig.data_width(), rgnwidth, dev.subtag(region.get_tag()));

							// verify endianness
							if (rgnwidth != 8)
							{
								if (!rgnisbe && spaceconfig.endianness() == ENDIANNESS_BIG)
									osd_printf_error("%s space is big-endian but references little-endian region '%s'\n", spaceconfig.m_name, dev.subtag(region.get_tag()));
								else if (rgnisbe && spaceconfig.endianness() == ENDIANNESS_LITTLE)
									osd_printf_error("%s space is little-endian but references big-endian region '%s'\n", spaceconfig.m_name, dev.subtag(region.get_tag()));
							}
						}

						found = true;
					}
				}
			}

			// error if not found
			if (!found)
				osd_printf_error("%s space memory map entry %X-%X references nonexistent region '%s'\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_region);
		}

		// make sure all devices exist
		if (entry.m_read.m_type == AMH_DEVICE_DELEGATE || entry.m_read.m_type == AMH_DEVICE_DELEGATE_M || entry.m_read.m_type == AMH_DEVICE_DELEGATE_S || entry.m_read.m_type == AMH_DEVICE_DELEGATE_SM || entry.m_read.m_type == AMH_DEVICE_DELEGATE_MO || entry.m_read.m_type == AMH_DEVICE_DELEGATE_SMO)
		{
			// extract the device tag from the proto-delegate
			std::pair<std::reference_wrapper<device_t>, const char *> devtag(entry.m_devbase, nullptr);
			switch (entry.m_read.m_bits)
			{
				case 8:
					if (entry.m_read.m_type == AMH_DEVICE_DELEGATE)
						devtag = entry.m_rproto8.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_M)
						devtag = entry.m_rproto8m.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_S)
						devtag = entry.m_rproto8s.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_SM)
						devtag = entry.m_rproto8sm.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_MO)
						devtag = entry.m_rproto8mo.finder_target();
					else
						devtag = entry.m_rproto8smo.finder_target();
					break;

				case 16:
					if (entry.m_read.m_type == AMH_DEVICE_DELEGATE)
						devtag = entry.m_rproto16.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_M)
						devtag = entry.m_rproto16m.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_S)
						devtag = entry.m_rproto16s.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_SM)
						devtag = entry.m_rproto16sm.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_MO)
						devtag = entry.m_rproto16mo.finder_target();
					else
						devtag = entry.m_rproto16smo.finder_target();
					break;

				case 32:
					if (entry.m_read.m_type == AMH_DEVICE_DELEGATE)
						devtag = entry.m_rproto32.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_M)
						devtag = entry.m_rproto32m.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_S)
						devtag = entry.m_rproto32s.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_SM)
						devtag = entry.m_rproto32sm.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_MO)
						devtag = entry.m_rproto32mo.finder_target();
					else
						devtag = entry.m_rproto32smo.finder_target();
					break;

				case 64:
					if (entry.m_read.m_type == AMH_DEVICE_DELEGATE)
						devtag = entry.m_rproto64.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_M)
						devtag = entry.m_rproto64m.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_S)
						devtag = entry.m_rproto64s.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_SM)
						devtag = entry.m_rproto64sm.finder_target();
					else if (entry.m_read.m_type == AMH_DEVICE_DELEGATE_MO)
						devtag = entry.m_rproto64mo.finder_target();
					else
						devtag = entry.m_rproto64smo.finder_target();
					break;
			}
			if (devtag.second && !devtag.first.get().subdevice(devtag.second))
				osd_printf_error("%s space memory map entry reads from nonexistent device '%s'\n", spaceconfig.m_name, devtag.first.get().subtag(devtag.second));
#ifndef MAME_DEBUG // assert will catch this earlier
			(void)entry.unitmask_is_appropriate(entry.m_read.m_bits, entry.m_mask, entry.m_read.m_name);
#endif
		}
		if (entry.m_write.m_type == AMH_DEVICE_DELEGATE || entry.m_read.m_type == AMH_DEVICE_DELEGATE_M || entry.m_write.m_type == AMH_DEVICE_DELEGATE_S || entry.m_write.m_type == AMH_DEVICE_DELEGATE_SM || entry.m_read.m_type == AMH_DEVICE_DELEGATE_MO || entry.m_write.m_type == AMH_DEVICE_DELEGATE_SMO)
		{
			// extract the device tag from the proto-delegate
			std::pair<std::reference_wrapper<device_t>, const char *> devtag(entry.m_devbase, nullptr);
			switch (entry.m_write.m_bits)
			{
				case 8:
					if (entry.m_write.m_type == AMH_DEVICE_DELEGATE)
						devtag = entry.m_wproto8.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_M)
						devtag = entry.m_wproto8m.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_S)
						devtag = entry.m_wproto8s.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_SM)
						devtag = entry.m_wproto8sm.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_MO)
						devtag = entry.m_wproto8mo.finder_target();
					else
						devtag = entry.m_wproto8smo.finder_target();
					break;

				case 16:
					if (entry.m_write.m_type == AMH_DEVICE_DELEGATE)
						devtag = entry.m_wproto16.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_M)
						devtag = entry.m_wproto16m.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_S)
						devtag = entry.m_wproto16s.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_SM)
						devtag = entry.m_wproto16sm.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_MO)
						devtag = entry.m_wproto16mo.finder_target();
					else
						devtag = entry.m_wproto16smo.finder_target();
					break;

				case 32:
					if (entry.m_write.m_type == AMH_DEVICE_DELEGATE)
						devtag = entry.m_wproto32.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_M)
						devtag = entry.m_wproto32m.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_S)
						devtag = entry.m_wproto32s.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_SM)
						devtag = entry.m_wproto32sm.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_MO)
						devtag = entry.m_wproto32mo.finder_target();
					else
						devtag = entry.m_wproto32smo.finder_target();
					break;

				case 64:
					if (entry.m_write.m_type == AMH_DEVICE_DELEGATE)
						devtag = entry.m_wproto64.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_M)
						devtag = entry.m_wproto64m.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_S)
						devtag = entry.m_wproto64s.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_SM)
						devtag = entry.m_wproto64sm.finder_target();
					else if (entry.m_write.m_type == AMH_DEVICE_DELEGATE_MO)
						devtag = entry.m_wproto64mo.finder_target();
					else
						devtag = entry.m_wproto64smo.finder_target();
					break;
			}
			if (devtag.second && !devtag.first.get().subdevice(devtag.second))
				osd_printf_error("%s space memory map entry writes to nonexistent device '%s'\n", spaceconfig.m_name, devtag.first.get().subtag(devtag.second));
#ifndef MAME_DEBUG // assert will catch this earlier
			(void)entry.unitmask_is_appropriate(entry.m_write.m_bits, entry.m_mask, entry.m_write.m_name);
#endif
		}

		// make sure ports exist
//      if ((entry.m_read.m_type == AMH_PORT && entry.m_read.m_tag != nullptr && portlist.find(entry.m_read.m_tag) == nullptr) ||
//          (entry.m_write.m_type == AMH_PORT && entry.m_write.m_tag != nullptr && portlist.find(entry.m_write.m_tag) == nullptr))
//          osd_printf_error("%s space memory map entry references nonexistent port tag '%s'\n", spaceconfig.m_name, entry.m_read.m_tag);

		// validate bank and share tags
		if (entry.m_read.m_type == AMH_BANK)
			valid.validate_tag(entry.m_read.m_tag);
		if (entry.m_write.m_type == AMH_BANK)
			valid.validate_tag(entry.m_write.m_tag);
		if (entry.m_share != nullptr)
			valid.validate_tag(entry.m_share);
	}
}

const address_space_config &address_map::get_config() const
{
	return *m_config;
}
