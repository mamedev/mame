// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************

  Mr Gluk Reset Service
  KR512VI1 clone MC146818

  Refs:
  https://zxart.ee/spa/software/prikladnoe-po/electronics/pzu/mr-gluk-reset-service-663/mr-gluk-reset-service-663/action:viewFile/id:250389/fileId:814961/

****************************************************************************/

#include "emu.h"
#include "glukrs.h"

glukrs_device::glukrs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: mc146818_device(mconfig, GLUKRS, tag, owner, clock)
{
	switch (clock)
	{
	case 4'194'304:
	case 1'048'576:
		m_tuc = 248;
		break;
	case 32'768:
		m_tuc = 1984;
		break;
	}
	set_24hrs(true);
}

void glukrs_device::device_start()
{
	mc146818_device::device_start();

	save_item(NAME(m_glukrs_active));
}

void glukrs_device::device_reset()
{
	mc146818_device::device_reset();

	m_glukrs_active = false;
}

// device type definition
DEFINE_DEVICE_TYPE(GLUKRS, glukrs_device, "glukrs", "Mr Gluk Reset Service")
