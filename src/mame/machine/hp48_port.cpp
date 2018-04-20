// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2008

   Hewlett Packard HP48 S/SX & G/GX/G+ and HP49 G

**********************************************************************/

#include "emu.h"
#include "machine/hp48_port.h"
#include "includes/hp48.h"

#include "logmacro.h"


/* ----- card images ------ */
DEFINE_DEVICE_TYPE(HP48_PORT, hp48_port_image_device, "hp48_port_image", "HP48 memory card")

/* helper for load and create */
void hp48_port_image_device::fill_port()
{
	int size = m_port_size;
	LOG("hp48_port_image_device::fill_port: %s module=%i size=%i rw=%i\n", tag(), m_module, size, m_port_write);
	m_port_data = make_unique_clear<uint8_t[]>(2 * size);
	m_hp48->m_modules[m_module].off_mask = 2 * (( size > 128 * 1024 ) ? 128 * 1024 : size) - 1;
	m_hp48->m_modules[m_module].read     = read8_delegate();
	m_hp48->m_modules[m_module].write    = write8_delegate();
	m_hp48->m_modules[m_module].isnop    = m_port_write ? 0 : 1;
	m_hp48->m_modules[m_module].data     = (void*)m_port_data.get();
	m_hp48->apply_modules();
}

/* helper for start and unload */
void hp48_port_image_device::unfill_port()
{
	LOG("hp48_port_image_device::unfill_port\n");
	m_hp48->m_modules[m_module].off_mask = 0x00fff;  /* 2 KB */
	m_hp48->m_modules[m_module].read     = read8_delegate();
	m_hp48->m_modules[m_module].write    = write8_delegate();
	m_hp48->m_modules[m_module].data     = nullptr;
	m_hp48->m_modules[m_module].isnop    = 1;
	m_port_size                          = 0;
}


image_init_result hp48_port_image_device::call_load()
{
	int size = length();
	if (size == 0) size = m_max_size; /* default size */
	LOG("hp48_port_image load: size=%i\n", size);

	/* check size */
	if ((size < 32*1024) || (size > m_max_size) || (size & (size-1)))
	{
		logerror("hp48: image size for %s should be a power of two between %i and %i\n", tag(), 32*1024, m_max_size);
		return image_init_result::FAIL;
	}

	m_port_size = size;
	m_port_write = !is_readonly();
	fill_port();
	fread(m_port_data.get(), m_port_size);
	m_hp48->decode_nibble(m_port_data.get(), m_port_data.get(), m_port_size);
	return image_init_result::PASS;
}

image_init_result hp48_port_image_device::call_create(int format_type, util::option_resolution *format_options)
{
	int size = m_max_size;
	LOG("hp48_port_image create: size=%i\n", size);
	/* XXX defaults to max_size; get user-specified size instead */

	/* check size */
	/* size must be a power of 2 between 32K and max_size */
	if ( (size < 32*1024) || (size > m_max_size) || (size & (size-1)) )
	{
		logerror( "hp48: image size for %s should be a power of two between %i and %i\n", tag(), 32*1024, m_max_size );
		return image_init_result::FAIL;
	}

	m_port_size = size;
	m_port_write = true;
	fill_port();
	return image_init_result::PASS;
}

void hp48_port_image_device::call_unload()
{
	LOG("hp48_port image unload: %s size=%i rw=%i\n", tag(), m_port_size, m_port_write);
	if (m_port_write)
	{
		m_hp48->encode_nibble(m_port_data.get(), m_port_data.get(), m_port_size);
		fseek(0, SEEK_SET);
		fwrite(m_port_data.get(), m_port_size);
	}
	m_port_data = nullptr;
	unfill_port();
	m_hp48->apply_modules();
}

void hp48_port_image_device::device_start()
{
	LOG("hp48_port_image start\n");
	unfill_port();
}

hp48_port_image_device::hp48_port_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP48_PORT, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_port_size(0)
	, m_port_write(false)
	, m_hp48(*this, DEVICE_SELF_OWNER)
{
}
