// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************

  Mr Gluk Reset Service
  KR512VI1 clone MC146818

  Refs:
  https://zxart.ee/spa/software/prikladnoe-po/electronics/pzu/mr-gluk-reset-service-663/mr-gluk-reset-service-663/action:viewFile/id:250389/fileId:814961/

****************************************************************************/

#include "emu.h"
#include "osdcore.h"
#include "glukrs.h"

glukrs_device::glukrs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: mc146818_device(mconfig, GLUKRS, tag, owner, clock)
{
	m_tuc = 1984;
	set_24hrs(true);
}

void glukrs_device::device_validity_check(validity_checker &valid) const
{
	mc146818_device::device_validity_check(valid);

	if (clock() != 32'768)
		osd_printf_warning("Clock %u is different from expected 32'768\n", clock());
}

void glukrs_device::device_start()
{
	mc146818_device::device_start();

	save_item(NAME(m_glukrs_active));
}

void glukrs_device::device_reset()
{
	m_data[REG_A] &= ~(REG_A_DV2 | REG_A_DV1 | REG_A_DV2);
	m_data[REG_A] |= REG_A_DV1;
	mc146818_device::device_reset();

	update_timer();

	m_glukrs_active = false;
}

// device type definition
DEFINE_DEVICE_TYPE(GLUKRS, glukrs_device, "glukrs", "Mr Gluk Reset Service")
